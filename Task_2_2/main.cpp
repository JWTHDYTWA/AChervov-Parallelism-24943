#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <optional>
#include <omp.h>
#include <cmath>

#include <boost/program_options.hpp>

#include "utilities.hpp"

#define USAGE "./integrate [-t THREADS]"
#define nsteps 40'000'000

const double a = -4.0;
const double b = 4.0;

template <typename RES_T>
struct BenchResult
{
    std::optional<std::exception> status;
    double duration;
    RES_T result;
};

double integrate(double (*func)(double), double, double, int);
double integrate_omp(double (*func)(double), double, double, int);
void initialize_array(double* arr, size_t size);

namespace po = boost::program_options;


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
        ("threads,t", po::value<int>(&threads), "Number of parallel threads");

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

        // Main logic

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

double func(double x)
{
    return std::exp(-x * x);
}

double integrate(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; i++)
        sum += func(a + h * (i + 0.5));

    sum *= h;

    return sum;
}

double integrate_omp(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++)
            sum += func(a + h * (i + 0.5));
    }
    sum *= h;

    return sum;
}

BenchResult<double> benchmark(size_t test_n, size_t size)
{
    BenchResult<double> res;
    std::vector<double> times(test_n);

    try
    {
        auto array = std::make_unique<double[]>(size);
        initialize_array(array.get(), size);

        for (size_t i = 0; i < test_n; i++)
        {


            const auto start{std::chrono::steady_clock::now()};
            // Load Begin
            res.result = integrate_omp(func, a, b, nsteps);
            // Load End
            const auto end{std::chrono::steady_clock::now()};

            const std::chrono::duration<double> elapsed_seconds{end - start};
            times[i] = elapsed_seconds.count();
        }
        res.duration = get_mean(times, 2);
    }
    catch(const std::exception& e)
    {
        res.status = e;
    }
    
    res.status = std::nullopt;
    return res;
}

void initialize_array(double* arr, size_t size)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (size_t i = 0; i < size; i++)
        {
            arr[i] = 0.0;
        }
    }
}
