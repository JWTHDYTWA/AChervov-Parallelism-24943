#pragma once

#include <cstddef>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "utilities.hpp"
#include "threading.hpp"

class matrix
{

private:
    std::unique_ptr<double[]> _data;
    size_t _M;
    size_t _N;

    static std::weak_ptr<threading::ThreadPool> matrix_threadpool;
    static std::mutex pool_mutex;

public:
    matrix() = default;
    matrix(size_t M, size_t N)
    {
        _data = std::make_unique_for_overwrite<double[]>(M*N);
        _M = M;
        _N = N;
    }

    matrix(matrix&& other) noexcept : _M(other._M), _N(other._N), _data(std::move(other._data))
    {
        other._M = 0;
        other._N = 0;
    }
    ~matrix() = default;

    inline double& operator[] (size_t i)
    {
        return _data[i];
    }
    inline double operator[] (size_t i) const
    {
        return _data[i];
    }
    inline size_t getM() const
    {
        return _M;
    }
    inline size_t getN() const
    {
        return _N;
    }

    inline double get(size_t m, size_t n) const
    {
        return _data[m*_N + n];
    }
    inline double& get(size_t m, size_t n)
    {
        return _data[m*_N + n];
    }

    matrix& operator=(matrix &&other) noexcept
    {
        if (this != &other) {
            _M = std::exchange(other._M, 0);
            _N = std::exchange(other._N, 0);
            _data = std::move(other._data);
        }
        return *this;
    }
    matrix& operator=(const matrix& other)
    {
        if (this == &other) {
            return *this;
        }
        size_t new_size = other._M * other._N;
        size_t old_size = _M * _N;

        if (new_size != old_size) {
            _data = std::make_unique_for_overwrite<double[]>(new_size);
        }
        _M = other._M;
        _N = other._N;

        if (new_size > 0) {
            std::copy(other._data.get(), other._data.get() + new_size, _data.get());
        }
        return *this;
    }

    // Контрольная сумма матрицы, выполняющая сразу две задачи:
    // 1. Проверка инварианта входных данных
    // 2. Избежание нежелательных оптимизаций компилятора
    uint64_t check_sum()
    {
        uint64_t sum = 0;
        for (size_t i = 0; i < _M*_N; i++)
        {
            sum += static_cast<uint64_t>(_data[i]);
        }
        return sum;
    }

    static void set_threadpool(std::weak_ptr<threading::ThreadPool> pool) {
        std::lock_guard<std::mutex> lock(pool_mutex);
        matrix_threadpool = std::move(pool);
    }
    static std::shared_ptr<threading::ThreadPool> get_threadpool()
    {
        return matrix_threadpool.lock();
    }

    friend matrix operator* (const matrix&, const matrix&);
    friend void matrix_multiply(const matrix&, const matrix&, matrix&, std::shared_ptr<threading::ThreadPool>&, size_t threads_count);
};

matrix operator* (const matrix &A, const matrix &B)
{
    if (A._N != B._M)
    {
        throw std::invalid_argument("Wrong matrix dimensions: An != Bm");
    }

    std::shared_ptr<threading::ThreadPool> pool;
    {
        std::lock_guard<std::mutex> lock(matrix::pool_mutex);
        pool = matrix::matrix_threadpool.lock(); 
    }

    if (!pool) {
        throw std::runtime_error("ThreadPool is not set. Set it using matrix::set_threadpool().");
    }

    matrix C(A._M, B._N);
    matrix_multiply(A, B, C, pool, std::thread::hardware_concurrency());
    
    return C;
}

void matrix_multiply(const matrix &A, const matrix &B, matrix& C, std::shared_ptr<threading::ThreadPool>& threadpool, size_t threads_count = std::thread::hardware_concurrency())
{
    if (A.getN() != B.getM())
    {
        throw std::invalid_argument("Wrong matrix dimensions: An != Bm");
    }
    if (A.getM() != C.getM() || B.getN() != C.getN())
    {
        throw std::invalid_argument("Wrong matrix dimensions: Am != Cm or Bn != Cn");
    }

    const ptrdiff_t local_M = A.getM();
    const ptrdiff_t local_N = B.getN();
    const ptrdiff_t local_K = A.getN();

    // Сценарий 1: Умножение матрицы на вектор
    if (local_N == 1)
    {
        threading::parallel_for_with_pool<ptrdiff_t>(0, local_M, *threadpool, threads_count, [&](ptrdiff_t start, ptrdiff_t end) {
            for (ptrdiff_t m = start; m < end; m++)
            {
                double sum = 0;
                ptrdiff_t local_m_A = m * local_K;
                for (ptrdiff_t k = 0; k < local_K; k++)
                {
                    sum += A[local_m_A + k] * B[k];
                }
                C[m] = sum;
            }
        });
    }
    // Сценарий 2: Умножение матрицы на матрицу
    else
    {
        // Создаем временную транспонированную матрицу B_T размера N x K
        matrix B_T(local_N, local_K);

        threading::parallel_for_with_pool<ptrdiff_t>(0, local_K, *threadpool, threads_count, [&](ptrdiff_t start, ptrdiff_t end) {
            for (ptrdiff_t k = start; k < end; k++)
            {
                for (ptrdiff_t n = 0; n < local_N; n++)
                {
                    B_T[n * local_K + k] = B[k * local_N + n];
                }
            }
        });

        threading::parallel_for_with_pool<ptrdiff_t>(0, local_M, *threadpool, threads_count, [&](ptrdiff_t start, ptrdiff_t end) {
            for (ptrdiff_t m = start; m < end; m++)
            {
                ptrdiff_t local_m_A = m * local_K;
                ptrdiff_t local_m_C = m * local_N;

                for (ptrdiff_t n = 0; n < local_N; n++)
                {
                    double sum = 0;
                    ptrdiff_t local_n_B = n * local_K;

                    for (ptrdiff_t k = 0; k < local_K; k++)
                    {
                        sum += A[local_m_A + k] * B_T[local_n_B + k];
                    }
                    C[local_m_C + n] = sum;
                }
            }
        });
    }
}

std::weak_ptr<threading::ThreadPool> matrix::matrix_threadpool;
std::mutex matrix::pool_mutex;
