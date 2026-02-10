
#ifndef FPLEN
#define FPLEN 4
#endif

#if (FPLEN == 4)
typedef float fp_type;
#elif (FPLEN == 8)
typedef double fp_type;
#else
#error FPLEN should be either 4 or 8.
#endif
