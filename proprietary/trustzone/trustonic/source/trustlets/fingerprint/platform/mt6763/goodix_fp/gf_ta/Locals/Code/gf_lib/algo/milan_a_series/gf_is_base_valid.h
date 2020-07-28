/***************** (C) COPYRIGHT 2016 GOODIX *************************
 * File Name          : is_base_valid.h
 * Author             : goodix.fpd2
 * Version            : v1.7.1
 * Date               : 2016-12-18 22:00
 * Description        : The image base updating file
 Updating base when power on or temprature has changed
 * History            : v1.7.1 -- 1.remove  BASE_INVALID_MAX(15)
 2.add (((s16)uwBaseMean - (s16)uwRawDataMean) > 400) judgement
 3.AllSaturationNumber judgement to 60%
 ********************************************************************/
#ifndef __GF_IS_BASE_VALID_H__
#define __GF_IS_BASE_VALID_H__
#define  PLATFORM_TYPE          MTK
#define  QSEE                   0
#define  MTK                    1
#define  VS2013                 2
#define  BASE_VALID_VERSION     "V1.7.1"

// for VS2013
#if(PLATFORM_TYPE == VS2013)
#define LOGD       printf
typedef unsigned char u8;
typedef unsigned short u16;  // NOLINT
typedef unsigned long u32;  // NOLINT
typedef signed char s8;
typedef signed short s16;  // NOLINT
typedef signed long s32;  // NOLINT
#define tee_malloc  malloc
#define tee_free    free
#endif

// for QSEE
#if(PLATFORM_TYPE == QSEE)
#include <typedef.h>
#include "gx_memory.h"
#include "gx_log.h"
#include "gf_update_base.h"

#define tee_malloc  gx_malloc
#define tee_free    gx_free
#endif

#if(PLATFORM_TYPE == MTK)
#include <stdint.h>
#include "gf_error.h"
#include "gf_tee_internal_api.h"
#include "string.h"
#include "cpl_memory.h"

#define tee_malloc(X)     CPL_MEM_MALLOC(X)
#define tee_free(X)    CPL_MEM_FREE(X)

typedef unsigned char u8;
typedef unsigned short u16;  // NOLINT
typedef unsigned long u32;  // NOLINT
typedef signed char s8;
typedef signed short s16;  // NOLINT
typedef signed long s32;  // NOLINT
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    BASE_INVALID = 0, BASE_VALID = 1, PARA_ERR = 2,
} EN_RET_TYPE;

typedef enum {
    BASE_ERR = 0, SEL_BASE = 1, SEL_RAWDATA = 2,
} BASE_RET_TYPE;

typedef struct {
    u16 uwTemperature;
    u16 uwRowNum;
    u16 uwColNum;
    u16 *puwData;
    u16 uwIsPassBaseValid;
    u16 uwIsAdjustedSlowly;
} ST_BASE_DATA;

extern u16 gbMicroAdjustDac;
EN_RET_TYPE IsBaseValid(const ST_BASE_DATA *pBase, const ST_BASE_DATA *pRawData);
u16 GetRawDataMean(const u16 uwRawData[], const u16 uwRowNum, const u16 uwColNum);
u16 GetRawDataConsistency(const u16 uwRawData[], const u16 uwRowNum, const u16 uwColNum);
BASE_RET_TYPE ChooseValidBase(const ST_BASE_DATA *pBase, const ST_BASE_DATA *pRawData);
void getRawDataMeanAndConsistency(const ST_BASE_DATA *pRawData, u32* pRawDataMean,
        u16* pRawDataConsistency);

#if defined(__cplusplus)
}
#endif
#endif  // __GF_IS_BASE_VALID_H__
