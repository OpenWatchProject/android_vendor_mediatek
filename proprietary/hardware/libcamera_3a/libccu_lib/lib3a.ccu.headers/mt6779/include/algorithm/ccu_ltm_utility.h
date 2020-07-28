#ifndef __CCU_LTM_UTILITY_H__
#define __CCU_LTM_UTILITY_H__
#include <stdint.h>
#include <string.h>
#ifndef WIN32
#include <stdbool.h>
#else
#define ccu_memcpy	memcpy
#define ccu_memset	memset
#endif

typedef float float32_t;
typedef double float64_t;
typedef bool bool_t;

int32_t ccu_abs(int32_t x);
int32_t ccu_min(int32_t a, int32_t b);
int32_t ccu_max(int32_t a, int32_t b);
float32_t ccu_min_f(float32_t a, float32_t b);
float32_t ccu_max_f(float32_t a, float32_t b);
int32_t ccu_sign(int32_t x);
int32_t get_cdf_hist(uint32_t *src, uint32_t *des, uint32_t bins, uint32_t* total_count);
bool_t int64_mult(uint64_t x, uint64_t y, uint64_t *result);
uint32_t cal_hist_sts(uint32_t *p_hist, uint32_t hist_bins, uint32_t perc, bool_t b_is_bright);
uint32_t cal_muti_seg_interp(uint32_t *p_x, uint32_t *p_y, uint32_t xi, uint32_t bins, uint32_t* start_idx);
uint32_t cal_interp(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, uint32_t xi);

#endif