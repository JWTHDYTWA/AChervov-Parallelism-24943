#include <iostream>
#include <cmath>
#include <numbers>
#include <memory>
#include "types.h"

#ifdef EN_PARALLELISM
#include <omp.h>
#endif

#define PRECISION 10'000'000

const fp_type period = std::numbers::pi * 2;

int main(int argc, char const *argv[])
{
    using std::unique_ptr;
    unique_ptr<fp_type[]> sin_values = std::make_unique<fp_type[]>(PRECISION);
    fp_type sum = 0;

    #ifdef EN_PARALLELISM
    #pragma omp parallel for reduction(+:sum)
    #endif
    for (int i = 0; i < PRECISION; i++)
    {
        fp_type temp = (period * i) / PRECISION;

        #ifdef USE_DOUBLE
        sin_values[i] = std::sin(temp);
        #else
        sin_values[i] = std::sinf(temp);
        #endif

        sum += sin_values[i];
    }

    std::cout << "Sum: " << sum << std::endl;
    
    return 0;
}
