/*
 * simu_arm_math.h
 *
 *  Created on: 2015/09/07
 *      Author: MTK09639
 *  2015/12/02 (fixed_v1) by PS
 */

#ifndef _SIMU_ARM_MATH_H_
#define _SIMU_ARM_MATH_H_

#include <stdint.h>

/*  temporary typedef  */
typedef float float32_t;
typedef double float64_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

/* simulated arm math lib */
q31_t clip_q63_to_q31(q63_t); /* map q63 to q31 format. if q31 is overflow, it will be 2^31 -1 */

void fixed_timefeature(int32_t *pSrc, int32_t blockSize, int32_t *result);
void fixed_max(int32_t *pSrc, uint32_t blockSize, int32_t *pResult);
void fixed_min(int32_t * pSrc, uint32_t blockSize, int32_t * pResult);
void fixed_mean(int32_t * pSrc, int32_t blockSize, int32_t * pResult);
void fixed_std(int32_t *pSrc, uint32_t blockSize, int32_t *pResult);
void fixed_rms(int32_t *pSrc, uint32_t blockSize, int32_t *pResult);

#endif /* _LIB_SIMU_ARM_MATH_H_ */
