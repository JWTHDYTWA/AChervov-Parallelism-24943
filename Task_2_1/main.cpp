#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>
#include <omp.h>

#include <boost/program_options.hpp>

#include "matrix.hpp"
#include "utilities.hpp"

#define USAGE "./matrix_mult -s SIZE [-t THREADS]"

namespace po = boost::program_options;

template <typename RES_T>
struct BenchResult
{
    std::optional<std::exception> status;
    double duration;
    RES_T result;
};


void initialize_matrix(matrix &A);
void initialize_vector(matrix &V);
BenchResult<matrix> benchmark(size_t, size_t);

int main(int argc, char const *argv[])
{
    // Using standard streams
    using std::cout, std::cin, std::cerr, std::endl;
    
    try
    {
        size_t size;
        int threads = 0;

        po::options_description desc("Options");
        desc.add_options()
        ("help,h", "Displays this message")
        ("threads,t", po::value<int>(&threads), "Number of parallel threads")
        ("size,s", po::value<size_t>(&size)->required(), "Matrix dimension span - M=N");

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

        auto [status, duration, res] = benchmark(100, size);
        
        if (status.has_value())
        {
            cerr << "Benchmark encountered a problem:" << endl;
            cerr << status.value().what() << endl;
        }
        else
        {
            std::ofstream file("./results.csv", std::ios::out | std::ios::app | std::ios::ate);

            if (!file.is_open()) {
                cerr << "Couldn't open file." << endl;
                return 1;
            }

            if (file.tellp() == 0) {
                file << "Size;Threads;Duration" << endl;
            }
            file << size << ';' << omp_get_max_threads() << ';' << duration << endl;
        }
    }
    catch (const po::error &e)
    {
        cerr << "Argument error:" << e.what() << endl;
        cerr << "Use --help to see syntax." << endl;
    }
    return 0;
}

void initialize_matrix(matrix &A)
{
    #pragma omp parallel
    {
        ptrdiff_t M = A.getM();
        ptrdiff_t N = A.getN();

        #pragma omp for schedule(static)
        for (ptrdiff_t m = 0; m < M; m++)
        {
            for (ptrdiff_t n = 0; n < N; n++)
            {
                if (m==n) A[m*N + n] = 2.0;
                else A[m*N + n] = 1.0;
            }
        }
    }
}

void initialize_vector(matrix &V)
{
    #pragma omp parallel
    {
        double val = V.getM() + 1;
        ptrdiff_t M = V.getM();

        #pragma omp for schedule(static)
        for (ptrdiff_t m = 0; m < M; m++)
        {
            V[m] = val;
        }
    }
}

BenchResult<matrix> benchmark(size_t test_n, size_t size)
{
    BenchResult<matrix> res;
    std::vector<double> times(test_n);

    try
    {
        matrix A(size, size);
        initialize_matrix(A);
        matrix V(size, 1);
        initialize_vector(V);
        matrix C(size, 1);
        initialize_vector(C);

        for (size_t i = 0; i < test_n; i++)
        {
            const auto start{std::chrono::steady_clock::now()};
            C = A * V;
            const auto end{std::chrono::steady_clock::now()};
            const std::chrono::duration<double> elapsed_seconds{end - start};
            times[i] = elapsed_seconds.count();

            if (i == test_n-1) res.result = C;
        }
        res.result = C;
        res.duration = get_mean(times, 2);
    }
    catch(const std::exception& e)
    {
        res.status = e;
        return res;
    }
    
    res.status = std::nullopt;
    return res;
}
