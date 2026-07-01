#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
// #include <concepts>
#include <latch>

namespace threading {

class ThreadPool {
public:

    // @brief Конструктор пула потоков
    // @param threads_count Количество потоков в пуле
    explicit ThreadPool(size_t threads_count) : stop(false), threads_count(threads_count) {
        workers.reserve(threads_count);
        for (size_t i = 0; i < threads_count; ++i) {
            workers.emplace_back([this](std::stop_token stop_token) {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        
                        this->condition.wait(lock, [this, stop_token] {
                            return this->stop || !this->tasks.empty() || stop_token.stop_requested();
                        });

                        if ((this->stop || stop_token.stop_requested()) && this->tasks.empty()) {
                            return;
                        }

                        // Забираем задачу из очереди
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    // @brief Добавляет задачу в очередь и возвращает future для получения результата
    // @param f Функция или callable объект, который будет выполнен в пуле потоков
    // @param args Аргументы для функции f
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> 
    {
        using return_type = std::invoke_result_t<F, Args...>;

        // Оборачиваем задачу в packaged_task для связывания с future
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable
            {
                return std::invoke(std::move(f), std::move(args)...);
            }
        );
        
        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("Добавление задачи в остановленный ThreadPool невозможно");
            }

            tasks.emplace([task]() { (*task)(); });
        }
        
        condition.notify_one();
        return res;
    }

    size_t get_thread_count() const {
        return threads_count;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        // std::jthread автоматически сделают join() при выходе из области видимости
    }

private:
    std::vector<std::jthread> workers;          // Вектор рабочих потоков
    std::queue<std::function<void()>> tasks;    // Очередь задач
    std::mutex queue_mutex;                     // Мьютекс для защиты очереди задач
    std::condition_variable condition;          // Условная переменная для синхронизации
    bool stop;                                  // Флаг остановки пула
    size_t threads_count;                       // Количество потоков в пуле
};

template <typename Index, typename F>
void parallel_for_with_pool(Index start, Index end, ThreadPool& pool, size_t threads_count, F&& f)
{
    Index total = end - start;
    if (threads_count <= 1 || total < static_cast<Index>(threads_count)) {
        f(start, end);
        return;
    }

    std::latch completion_latch(threads_count);

    Index chunk = total / threads_count;
    Index remainder = total % threads_count;

    Index current_start = start;
    for (size_t t = 0; t < threads_count; ++t) {
        Index current_end = current_start + chunk + (t < remainder ? 1 : 0);
        
        pool.enqueue([current_start, current_end, &f, &completion_latch]() {
            f(current_start, current_end);
            completion_latch.count_down();
        });
        
        current_start = current_end;
    }

    completion_latch.wait(); 
}

} // namespace threading
