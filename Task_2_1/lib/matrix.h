#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <omp.h>

class matrix
{

private:
    std::unique_ptr<double[]> _data;
    size_t _M;
    size_t _N;

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

    friend matrix operator* (const matrix &A, const matrix &V);

};

matrix operator* (const matrix &A, const matrix &B)
{
    if (A._N != B._M)
    {
        throw std::invalid_argument("Wrong matrix dimensions: An != Bm");
    }

    matrix C(A._M, B._N);
    #pragma omp parallel
    {
        size_t local_M = A._M;
        size_t local_N = B._N;
        size_t local_K = A._N;

        size_t local_m_A;

        #pragma omp for schedule(static)
        for (size_t n = 0; n < B._N; n++)
        {
            for (size_t m = 0; m < A._M; m++)
            {
                local_m_A = m * local_K;
                double sum = 0;
                for (size_t k = 0; k < A._N; k++)
                {
                    sum += (A[local_m_A + k] * B[k * local_N + n]);
                }
                C[m * B._N + n] = sum;
            }
        }
        
    }
    return C;
}
