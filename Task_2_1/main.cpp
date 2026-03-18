#include <cstdint>
#include <iostream>
#include <memory>
#include <chrono>
#include <omp.h>
#include "matrix.h"

void initialize_matrix(matrix &A);
void initialize_vector(matrix &V);

int main(int argc, char const *argv[])
{
    using std::cout, std::endl;

    size_t size = 20000;
    if (argc >= 2)
    {
        omp_set_num_threads(atoi(argv[1]));
    }
    if (argc >= 3)
    {
        size = atoll(argv[2]);
    }

    matrix A(size, size);
    initialize_matrix(A);
    matrix V(size, 1);
    initialize_vector(V);

    // cout << A[0] << endl;
    // cout << A[1] << endl;
    // cout << A[20000*5 + 5] << endl;
    // cout << A[20000 * 15000] << endl;

    // cout << V[15000] << endl;
    // cout << V[11000] << endl;

    const auto start{std::chrono::steady_clock::now()};
    matrix C = A * V;
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    cout << "Matrix multiplication took "
         << elapsed_seconds.count()
         << " seconds on "
         << omp_get_max_threads()
         << " threads." << endl;
    
    return 0;
}

void initialize_matrix(matrix &A)
{
    #pragma omp parallel
    {
        size_t M = A.getM();
        size_t N = A.getN();

        int thread = omp_get_thread_num();
        bool first_i = true;

        #pragma omp for schedule(static)
        for (size_t m = 0; m < A.getM(); m++)
        {
            if (first_i)
            {
                #pragma omp critical
                {
                    std::cout << "Thread " << thread << " started from row " << m << std::endl;
                    first_i = false;
                }
            }
            for (size_t n = 0; n < A.getN(); n++)
            {
                if (m==n) A[m*A.getN() + n] = 2.0;
                else A[m*A.getN() + n] = 1.0;
            }
        }
    }
}

void initialize_vector(matrix &V)
{
    #pragma omp parallel
    {
        double val = V.getM() + 1;

        #pragma omp for schedule(static)
        for (size_t m = 0; m < V.getM(); m++)
        {
            V[m] = val;
        }
    }
}
