#ifndef __CCU_HLR_UTILITY_H__
#define __CCU_HLR_UTILITY_H__
#include <stdint.h>
#include <string.h>
#ifndef WIN32
#include <stdbool.h>
#endif

typedef float float32_t;
typedef double float64_t;
typedef bool bool_t;

int32_t div_round(int32_t const x, int32_t const y);
int32_t clamp(int32_t const x, int32_t const a, int32_t const b);
int32_t interpolate(int32_t const x, int32_t const x0, int32_t const x1, int32_t const y0, int32_t const y1);


#endif


