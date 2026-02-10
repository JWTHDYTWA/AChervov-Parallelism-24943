#include <iostream>
#include <cmath>
#include <numbers>
#include <omp.h>
#include "types.h"

#define PRECISION 10000000

fp_type sin_values[PRECISION];
const fp_type period = std::numbers::pi * 2;

int main(int argc, char const *argv[])
{
    fp_type sum = 0;

    // #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < PRECISION; i++)
    {
        fp_type temp = (period * i) / PRECISION;

        #if (FPLEN == 8)
        sin_values[i] = std::sin(temp);
        #elif (FPLEN == 4)
        sin_values[i] = std::sinf(temp);
        #endif
    }

    for (int i = 0; i < PRECISION; i++)
    {
        sum += sin_values[i];
    }

    std::cout << "Sum: " << sum << std::endl;
    
    return 0;
}
