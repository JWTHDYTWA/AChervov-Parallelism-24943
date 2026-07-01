#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <optional>
#include <omp.h>
#include <cmath>
#include <variant>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "utilities.hpp"
#include "matrix.hpp"

#define USAGE "./integrate [-t THREADS]"

namespace po = boost::program_options;

template <typename RES_T>
using BenchResult = std::vector<std::pair<double, RES_T>>;
// {
//     std::optional<std::exception> status;
//     std::vector<std::pair<double, RES_T>> runs;
// };

double integrate(double (*func)(double), double, double, int);
double integrate_omp(double (*func)(double), double, double, int);

void initialize_vector_with_value(matrix &v, const double value);
void initialize_matrix(matrix &a);

BenchResult<double> benchmark(size_t, size_t, bool);
void solve_blas_sections(const matrix &A, const matrix &b, matrix &x, matrix &r_buf, const double tau, const double eps);
void solve_blas_single(const matrix &A, const matrix &b, matrix &x, matrix &r_buf, const double tau, const double eps);


int main(int argc, char const *argv[])
{
    // Using standard streams
    using std::cout, std::cin, std::cerr, std::endl;

    CursorGuard cg();
    
    try
    {
        /// ARGUMENT PARSING ///

        // size_t size;
        int threads = 0;
        size_t benchmarks = 1;
        bool single_section = false;
        size_t size;

        po::options_description desc("Options");
        desc.add_options()
        ("help,h", "Displays this message")
        ("threads,t", po::value<int>(&threads), "Number of parallel threads")
        ("size,s", po::value<size_t>(&size)->required(), "Matrix dimension span - M=N")
        ("bench,b", po::value<size_t>(&benchmarks), "Benchmark iterations")
        ("mode,m", po::bool_switch(&single_section), "Single section mode");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cout << "Usage:" << endl << USAGE << endl << endl;
            cout << desc << endl;
            return 0;
        }
        po::notify(vm);

        if (vm.count("threads"))
        {
            omp_set_num_threads(threads);
        }

        
        /// MAIN LOGIC ///

        auto runs = benchmark(benchmarks, size, single_section);
        
        std::ofstream file(
            boost::str(boost::format("./results_%dT_%dS.csv") % omp_get_max_threads() % size),
            std::ios::out | std::ios::app | std::ios::ate
        );

        if (!file.is_open()) {
            cerr << "Couldn't open file." << endl;
            return 1;
        }

        if (file.tellp() == 0) {
            file << "Duration;Result" << endl;
        }
        for (auto &&run : runs)
        {
            file << run.first << ';' << run.second << endl;
        }

    }
    catch (const po::error &e)
    {
        cerr << "Argument error:" << e.what() << endl;
        cerr << "Use --help to see syntax." << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "Unexpected exception: " << e.what() << endl;
    }

    return 0;
}


// Запуск бенчмарка на решение СЛАУ методом простых итераций.
// @param test_n Число независимых тестов
// @param size Размер матрицы и вектора.
// @param ss Режим Single Section.
BenchResult<double> benchmark(size_t test_n, size_t size, bool ss)
{
    std::vector<std::pair<double, double>> runs(test_n);
    auto solve_blas = ss ? solve_blas_single : solve_blas_sections;

    matrix A(size, size);
    initialize_matrix(A);
    matrix b(size, 1);
    initialize_vector_with_value(b, (double)b.getM() + 1.0);

    matrix x(size, 1);
    matrix r(size, 1);

    for (size_t i = 0; i < test_n; i++)
    {
        std::cout << clear_line << "Progress: " << i+1 << "/" << test_n << std::flush;
        initialize_vector_with_value(x, 0);
        const auto start{std::chrono::steady_clock::now()};
        // Load Begin

        solve_blas(A, b, x, r, 2.0 / (size + 2.0), 1e-6);

        // Load End
        const auto end{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds{end - start};

        runs[i] = {elapsed_seconds.count(), x[0]};
    }
    std::cout << std::endl;
    
    return runs;
}

void initialize_vector_with_value(matrix &v, const double value)
{
    if (v.getN() != 1) throw std::invalid_argument("Vector width (N) must be 1.");

    const ptrdiff_t M = v.getM();

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (ptrdiff_t m = 0; m < M; m++)
        {
            v[m] = value;
        }
    }
}

void initialize_matrix(matrix &a)
{
    const ptrdiff_t M = a.getM();
    const ptrdiff_t N = a.getN();
    if (M != N) throw std::invalid_argument("Matrix A must be square.");

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (ptrdiff_t i = 0; i < M; i++)
        {
            for (ptrdiff_t j = 0; j < N; j++)
            {
                a[i*N + j] = 1.0;
            }
        }

        #pragma omp for schedule(static)
        for (ptrdiff_t i = 0; i < M; i++)
        {
            a[i*N + i] = 2.0;
        }
    }
}

void solve_blas_sections(const matrix &A, const matrix &b, matrix &x, matrix &r_buf, const double tau, const double eps)
{
    if (b.getN() != 1 || r_buf.getN() != 1)
        throw std::invalid_argument("Some of vectors have N>1.");
    if (A.getN() != b.getM())
        throw std::invalid_argument("Matrix A and vector b are incompatible.");
    if (b.getM() != r_buf.getM())
        throw std::invalid_argument("Some of vectors are not the same size");

    const ptrdiff_t N = A.getM();
    double b_norm = vector_norm(b);

    while (true)
    {
        #pragma omp parallel for schedule(static)
        for (ptrdiff_t i = 0; i < N; i++) {
            double ax_i = 0.0;
            for (ptrdiff_t j = 0; j < N; j++) {
                ax_i += A[i*N+j] * x[j];
            }
            r_buf[i] = ax_i - b[i]; // Ax_n - b
        }
        
        double r_sqr_sum = 0;
        #pragma omp parallel for schedule(static) reduction(+:r_sqr_sum)
        for (ptrdiff_t i = 0; i < N; i++)
        {
            r_sqr_sum += r_buf[i] * r_buf[i];
        }

        double err = std::sqrt(r_sqr_sum) / b_norm;
        if (err < eps) break;
        
        #pragma omp parallel for schedule(static)
        for (ptrdiff_t i = 0; i < N; i++) {
            x[i] -= tau * r_buf[i];
        }
    }
}

void solve_blas_single(const matrix &A, const matrix &b, matrix &x, matrix &r_buf, const double tau, const double eps)
{
    if (b.getN() != 1 || r_buf.getN() != 1)
        throw std::invalid_argument("Some of vectors have N>1.");
    if (A.getN() != b.getM())
        throw std::invalid_argument("Matrix A and vector b are incompatible.");
    if (b.getM() != r_buf.getM())
        throw std::invalid_argument("Some of vectors are not the same size");

    const ptrdiff_t N = A.getM();
    double b_norm = vector_norm(b);

    bool stop = false;
    double r_sqr_sum = 0.0;

    #pragma omp parallel shared(A, b, x, r_buf, stop, r_sqr_sum)
    {
        while (true)
        {
            #pragma omp single
            {
                r_sqr_sum = 0.0;
            }

            #pragma omp for schedule(static) reduction(+:r_sqr_sum)
            for (ptrdiff_t i = 0; i < N; i++) {
                double ax_i = 0.0;
                for (ptrdiff_t j = 0; j < N; j++) {
                    ax_i += A[i*N+j] * x[j];
                }
                double r_val = ax_i - b[i];
                r_buf[i] = r_val;
                r_sqr_sum += r_val * r_val;
            }

            #pragma omp single
            {
                double err = std::sqrt(r_sqr_sum) / b_norm;
                if (err < eps) {
                    stop = true;
                }
            }

            if (stop) {
                break;
            }

            #pragma omp for schedule(static)
            for (ptrdiff_t i = 0; i < N; i++) {
                x[i] -= tau * r_buf[i];
            }
        }
    }
}
