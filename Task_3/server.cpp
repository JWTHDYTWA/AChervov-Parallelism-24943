#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>
#include <thread>
#include <random>
#include <numbers>
#include <queue>
#include <latch>

#include <boost/program_options.hpp>

#include "matrix.hpp"
#include "utilities.hpp"
#include "threading.hpp"

#define USAGE "./server -s SIZE [-t THREADS]"

namespace po = boost::program_options;


/// TYPE DEFINITIONS ///

template <typename T>
class Server {
    private:
        std::shared_ptr<threading::ThreadPool> threadpool;
        std::vector<std::shared_future<T>> results;
        bool running = false;
        std::mutex state_mutex;
        std::condition_variable cv;
    public:
        Server(size_t size, int threads) {
            threadpool = std::make_shared<threading::ThreadPool>(threads);
        }
        Server(std::shared_ptr<threading::ThreadPool> threadpool):
            threadpool(threadpool) {}
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        void start() {
            std::lock_guard<std::mutex> lock(state_mutex);
            running = true;
        }

        void stop() {
            std::lock_guard<std::mutex> lock(state_mutex);
            running = false;
        }

        template <typename... Args>
        size_t add_task(std::type_identity_t<std::function<T(Args...)>> task, Args... args) {
            if (!running) {
                throw std::runtime_error("Server is not running");
            }
            std::lock_guard<std::mutex> lock(state_mutex);
            std::future<T> future = threadpool->enqueue(task, args...);
            results.push_back(future.share());
            return results.size() - 1;
        }

        T request_result(size_t task_id) {
            std::shared_future<T> fut;
            {
                std::lock_guard<std::mutex> lock(state_mutex);
                if (task_id >= results.size()) {
                    throw std::out_of_range("Invalid task ID");
                }
                fut = results[task_id];
            }
            return fut.get();
        }
        
        std::optional<T> try_request_result(size_t task_id) {
            std::lock_guard<std::mutex> lock(state_mutex);
            if (task_id >= results.size()) {
                return std::nullopt;
            }
            auto& future = results[task_id];
            if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                return future.get();
            } else {
                return std::nullopt;
            }
        }

};


/// FUNCTION DECLARATIONS ///

template <typename T>
T fun_sin(T arg);

template <typename T>
T fun_sqrt(T arg);

template <typename T>
T fun_pow(T x, T y);


/// GLOBAL VARIABLES ///

// std::shared_ptr<threading::ThreadPool> threadpool;


int main(int argc, char const *argv[])
{
    // Using standard streams
    using std::cout, std::cin, std::cerr, std::endl;
    
    try
    {
        /// ARGUMENT PARSING ///

        int threads = 1;

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
        
        /// THREADPOOL CREATION ///

        std::shared_ptr<threading::ThreadPool> threadpool = std::make_shared<threading::ThreadPool>(threads);

        /// LOGIC ///

        Server<double> server(threadpool);
        server.start();

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution dis(6, 9999);

        size_t client_tasks[3];
        for (int i = 0; i < 3; i++)
        {
            client_tasks[i] = dis(gen);
        }

        std::vector<std::pair<size_t, double>> sin_outputs;
        std::vector<std::pair<size_t, double>> sqrt_outputs;
        std::vector<std::pair<size_t, double>> pow_outputs;
        sin_outputs.reserve(client_tasks[0]);
        sqrt_outputs.reserve(client_tasks[1]);
        pow_outputs.reserve(client_tasks[2]);

        {
            std::vector<std::jthread> clients(3);
            std::latch start_latch(3);

            clients[0] = std::jthread([&server, &sin_outputs, &client_tasks, &start_latch](std::stop_token stoken) mutable {
                try
                {
                    std::random_device lrd;
                    std::mt19937_64 lgen(lrd());
                    std::uniform_real_distribution ldis(-std::numbers::pi, std::numbers::pi);

                    size_t tasks = client_tasks[0];
                    std::vector<size_t> idx;
                    idx.reserve(tasks);

                    start_latch.arrive_and_wait();

                    for (ptrdiff_t i = 0; i < tasks; i++)
                    {
                        auto id = server.add_task(fun_sin<double>, ldis(lgen));
                        idx.push_back(id);
                        std::this_thread::yield();
                    }
                    
                    for (auto &&i : idx)
                    {
                        auto result = server.request_result(i);
                        sin_outputs.push_back({i, result});
                    }
                }
                catch (const std::exception &e)
                {
                    cerr << "Unexpected error in Sin thread: " << e.what() << endl;
                }
            });

            clients[1] = std::jthread([&server, &sqrt_outputs, &client_tasks, &start_latch](std::stop_token stoken) mutable {
                try
                {
                    std::random_device lrd;
                    std::mt19937_64 lgen(lrd());
                    std::uniform_real_distribution ldis(0.0, 1000000.0);

                    size_t tasks = client_tasks[1];
                    std::vector<size_t> idx;
                    idx.reserve(tasks);

                    start_latch.arrive_and_wait();

                    for (ptrdiff_t i = 0; i < tasks; i++)
                    {
                        auto id = server.add_task(fun_sqrt<double>, ldis(lgen));
                        idx.push_back(id);
                        std::this_thread::yield();
                    }
                    
                    for (auto &&i : idx)
                    {
                        auto result = server.request_result(i);
                        sqrt_outputs.push_back({i, result});
                    }
                }
                catch (const std::exception &e)
                {
                    cerr << "Unexpected error in Sqrt thread: " << e.what() << endl;
                }
            });

            clients[2] = std::jthread([&server, &pow_outputs, &client_tasks, &start_latch](std::stop_token stoken) mutable {
                try
                {
                    std::random_device lrd;
                    std::mt19937_64 lgen(lrd());
                    std::uniform_real_distribution ldis1(0.0, 1000.0);
                    std::uniform_real_distribution ldis2(0.0, 10.0);

                    size_t tasks = client_tasks[2];
                    std::vector<size_t> idx;
                    idx.reserve(tasks);

                    start_latch.arrive_and_wait();

                    for (ptrdiff_t i = 0; i < tasks; i++)
                    {
                        auto id = server.add_task(fun_pow<double>, ldis1(lgen), ldis2(lgen));
                        idx.push_back(id);
                        std::this_thread::yield();
                    }
                    
                    for (auto &&i : idx)
                    {
                        auto result = server.request_result(i);
                        pow_outputs.push_back({i, result});
                    }
                }
                catch (const std::exception &e)
                {
                    cerr << "Unexpected error in Pow thread: " << e.what() << endl;
                }
            });
        }

        /// RESULTS HANDLING ///
        
        std::ofstream file(
            std::format("./results_server_{}T.csv", threads),
            std::ios::out | std::ios::app | std::ios::ate
        );

        if (!file.is_open()) {
            cerr << "Couldn't open file." << endl;
            return 1;
        }

        if (file.tellp() == 0) {
            file << "Task;Id;Result" << endl;
        }
        for (auto &&res : sin_outputs)
        {
            file << "sin" << ';' << res.first << ';' << res.second << endl;
        }
        for (auto &&res : sqrt_outputs)
        {
            file << "sqrt" << ';' << res.first << ';' << res.second << endl;
        }
        for (auto &&res : pow_outputs)
        {
            file << "pow" << ';' << res.first << ';' << res.second << endl;
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

template <typename T>
T fun_sin(T arg)
{
    return std::sin(arg);
}

template <typename T>
T fun_sqrt(T arg)
{
    return std::sqrt(arg);
}

template <typename T>
T fun_pow(T x, T y)
{
    return std::pow(x, y);
}
