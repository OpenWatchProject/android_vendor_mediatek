#ifndef __CCU_AE_SYNC_UTILITY_H__
#define __CCU_AE_SYNC_UTILITY_H__

#if WIN32
#include <windows.h>
#pragma warning (error: 4013)
#endif // WIN32


#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/*
 * AE sync algo version = Year + Week + Day + Counter
 */
#define AE_SYNC_ALGO_VER_YEAR    (8)
#define AE_SYNC_ALGO_VER_WEEK    (48)
#define AE_SYNC_ALGO_VER_DAY     (4)
#define AE_SYNC_ALGO_VER_COUNTER (001)
#define AE_SYNC_ALGO_VER_FULL    ((AE_SYNC_ALGO_VER_YEAR) * 1000000 + (AE_SYNC_ALGO_VER_WEEK) * 10000 + (AE_SYNC_ALGO_VER_DAY) * 1000 + (AE_SYNC_ALGO_VER_COUNTER))

/*
 * AE sync algo interface version = Year + Week + Day + Counter
 */
#define AE_SYNC_ALGO_IF_VER_YEAR    (8)
#define AE_SYNC_ALGO_IF_VER_WEEK    (48)
#define AE_SYNC_ALGO_IF_VER_DAY     (4)
#define AE_SYNC_ALGO_IF_VER_COUNTER (001)
#define AE_SYNC_ALGO_IF_VER_FULL    ((AE_SYNC_ALGO_IF_VER_YEAR) * 1000000 + (AE_SYNC_ALGO_IF_VER_WEEK) * 10000 + (AE_SYNC_ALGO_IF_VER_DAY) * 1000 + (AE_SYNC_ALGO_IF_VER_COUNTER))

#endif
