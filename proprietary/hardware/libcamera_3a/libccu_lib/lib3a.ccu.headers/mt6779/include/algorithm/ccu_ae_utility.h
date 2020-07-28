#ifndef __CCU_AE_UTILITY_H__
#define __CCU_AE_UTILITY_H__

#if WIN32
#include <windows.h>
#pragma warning (error: 4013)
#endif // WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define AE_ALGO_REVISION    (9181014)
#define AE_ALGO_IF_REVISION (9105008)

/*
 * AE algo version = Year + Week + Day + Counter
 */
#define AE_ALGO_VER_YEAR    (8)
#define AE_ALGO_VER_WEEK    (44)
#define AE_ALGO_VER_DAY     (4)
#define AE_ALGO_VER_COUNTER (002)
#define AE_ALGO_VER_FULL    ((AE_ALGO_VER_YEAR) * 1000000 + (AE_ALGO_VER_WEEK) * 10000 + (AE_ALGO_VER_DAY) * 1000 + (AE_ALGO_VER_COUNTER))

/*Control*/
#define HIGH_BOUND_CWM_CHECK ((MUINT32) 6)
#define LOW_BOUND_CWM_CHECK ((MUINT32) 6)
#define AE_GAIN_SHIFT ((MUINT32)8)
#define AE_GAIN_BASE ((MUINT32)1<<AE_GAIN_SHIFT)

/*Log*/
#define CHECK_LOG_LEVEL_START(level) if (g_pAEAlgoData->m_AlgoCtrl.u4LogLevel & (level)) {
#define CHECK_LOG_LEVEL_END }

/*Memory Check*/
//switch for memory check monitor
//change to 1 to turn on memory check monitor
//change to 0 to make all macro API empty and will not be built into binary
//#define MEMCHK_ENABLE 1

#define MEMCHK_5x5_LIMIT 25
#define MEMCHK_15x15_LIMIT 225
#define MEMCHK_AAO_LIMIT 128
#define MEMCHK_HIST_LIMIT AE_SW_HISTOGRAM_BIN

#if MEMCHK_ENABLE
//initialization of this module, just do once at system boot
#define MEMCHK_ARRAY(INDEX, LIMIT) if((INDEX) < 0 || (INDEX) >= (LIMIT)) { ccu_log(CCU_MUST_LOGTAG, 2, "[MEMCHK_ARRAY] ERROR, IDX:%d/%d\n", (INDEX), (LIMIT), 0, 0); CCU_ERROR("ARRAY INDEX MISMATCH"); }
#define MEMCHK_5x5(INDEX) MEMCHK_ARRAY(INDEX, MEMCHK_5x5_LIMIT)
#define MEMCHK_15x15(INDEX) MEMCHK_ARRAY(INDEX, MEMCHK_15x15_LIMIT)
#define MEMCHK_AAO(INDEX) MEMCHK_ARRAY(INDEX, MEMCHK_AAO_LIMIT)
#define MEMCHK_HIST(INDEX) MEMCHK_ARRAY(INDEX, MEMCHK_HIST_LIMIT)
#define MEMCHK_LUM(INDEX) MEMCHK_ARRAY(INDEX, LumLog2x1000_TABLE_SIZE)
#else
#define MEMCHK_ARRAY(INDEX, LIMIT)
#define MEMCHK_5x5(INDEX)
#define MEMCHK_15x15(INDEX)
#define MEMCHK_AAO(INDEX)
#define MEMCHK_HIST(INDEX)
#define MEMCHK_LUM(INDEX)
#endif


/*Statistics*/
#define STAT_Y_MAX  ((MUINT32)(255))
#define STAT_RGB_MAX ((MUINT32)(255))
#define STAT_MAX ((MUINT32)(4095))
#define HIST_BINS ((MUINT32)(256))
#define SHIFT_BIT ((MUINT32)(4)) //8->12
#define AE_HIST_BINMODE_128 0
#define AE_HIST_BINMODE_256 1
#define AE_HIST_COLORMODE_R 0
#define AE_HIST_COLORMODE_G 1
#define AE_HIST_COLORMODE_B 2
#define AE_HIST_COLORMODE_RGB 3
#define AE_HIST_COLORMODE_Y 4
#define AE_HIST_COLORMODE_FLARE 4

#define LTM_STAT_Y_MAX ((MUINT32)(4095))
#define LTM_STAT_SHIFT_BIT ((MUINT32)(4)) //8->12

/*Face*/
#define FD_BV_L 0
#define FD_BV_H 4000
#define FD_LOST_MAXCNT (3)
#define FD_FOUND_MAXCNT (1)

#endif
