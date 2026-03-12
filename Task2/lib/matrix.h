#pragma once

#include <array>
#include <cstddef>

void _multiply(const double *A, const double *V, double *out, size_t m, size_t n)
{
    #pragma omp parallel
    for (size_t i = 0; i < n; i++)
    {
        /* code */
    }
    
}

template <size_t M, size_t N>
class matrix
{
private:
    std::array<double, M*N> data;
public:
    matrix();
    ~matrix();

    T& operator[] (size_t m, size_t n)
    {
        return data[m*N + n];
    }

};

template <size_t M, size_t N>
std::array<double, N> operator* (matrix<M,N> A, std::array<double,N>)
{

}
