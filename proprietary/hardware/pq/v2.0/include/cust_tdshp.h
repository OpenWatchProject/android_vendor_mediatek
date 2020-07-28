#ifndef __CUST_TDSHP_H__
#define __CUST_TDSHP_H__

#include "ddp_pq.h"
#include "PQServiceCommon.h"

#define SUPPORT_PQ_PATH0 "/sdcard/SUPPORT_PQ"
#define SUPPORT_PQ_PATH1 "/storage/sdcard1/SUPPORT_PQ"

// for debug
#define PQ_DBG_SHP_TUNING_DEFAULT                   "0" // 0: disable, 1: enable, 2: default
#define PQ_DBG_SHP_TUNING_STR                       "debug.pq.shp.tuning"

extern const DISPLAY_TDSHP_T tdshpindex;

enum TUNING_FLAG_ENUM
{
    TDSHP_FLAG_NORMAL = (0),
    TDSHP_FLAG_TUNING = (1 << 0),
    TDSHP_FLAG_DC_TUNING = (1 << 1),
    TDSHP_FLAG_NCS_SHP_TUNING = (1 << 2),
    TDSHP_FLAG_DS_TUNING = (1 << 3),
    RSZ_FLAG_ULTRARES_TUNING = (1 << 4),
    RSZ_FLAG_NCS_RSZ_TUNING = (1 << 5)
};

#define PQ_DBG_MDP_CZ_ISP_TUNING_DEFAULT    "0"
#define PQ_DBG_MDP_CZ_ISP_TUNING_STR        "vendor.debug.pq.cz.isp.tuning"

#define GLOBAL_PQ_SUPPORT_STR "ro.vendor.globalpq.support"
#define GLOBAL_PQ_SUPPORT_DEFAULT "0"
#define GLOBAL_PQ_ENABLE_STR "persist.vendor.sys.globalpq.enable"
#define GLOBAL_PQ_ENABLE_DEFAULT "0"

#define GLOBAL_PQ_STRENGTH_STR "persist.vendor.sys.globalpq.strength"
#define GLOBAL_PQ_STRENGTH_DEFAULT "0"

#define GLOBAL_PQ_VIDEO_SHARPNESS_STRENGTH_RANGE 12
#define GLOBAL_PQ_VIDEO_DC_STRENGTH_RANGE 11
#define GLOBAL_PQ_UI_SHARPNESS_STRENGTH_RANGE 12
#define GLOBAL_PQ_UI_DC_STRENGTH_RANGE 11

#define GLOBAL_PQ_DC_INDEX_MAX 12

typedef struct{
    unsigned int entry[GLOBAL_PQ_DC_INDEX_MAX][PQDC_INDEX_MAX];
} DISPLAY_DC_T;

extern DISPLAY_DC_T dcindex;

typedef struct GlobalPQIndex {
    DISPLAY_DC_T dcindex;
} GLOBAL_PQ_INDEX_T;

#endif

