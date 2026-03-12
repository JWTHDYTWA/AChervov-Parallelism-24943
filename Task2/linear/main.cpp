#include <cstdint>
#include <memory>
#include <array>

#define N 20000
#define SIZE (20000 * 20000)

void multiply(double A[], uint32_t m, uint32_t n, double V[], double out[]);

int main(int argc, char const *argv[])
{
    //
    return 0;
}

void multiply(double A[], uint32_t m, uint32_t n, double V[], double out[])
{
    #pragma omp parallel for
    for (size_t i = 0; i < N; i++)
    {
        /* code */
    }
    
}
