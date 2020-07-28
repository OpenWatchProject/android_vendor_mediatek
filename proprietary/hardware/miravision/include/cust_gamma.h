#ifndef __GAMMA_H__
#define __GAMMA_H__

#include "ddp_gamma.h"

#define GAMMA_INDEX_PROPERTY_NAME "persist.vendor.sys.pq.gamma.index"
#define GAMMA_DEBUG_PROPERTY_NAME "persist.vendor.sys.pq.gamma.debug"

// Number of LCM of this project supports
#define GAMMA_LCM_MAX 1

// Maximum number of gamma table per LCM
#define GAMMA_INDEX_MAX 15

// The index of default gamma table
#define GAMMA_INDEX_DEFAULT 7
#define PQ_GAMMA_INDEX_DEFAULT "7"

typedef unsigned short gamma_entry_t[3][DISP_GAMMA_LUT_SIZE];

extern const gamma_entry_t cust_gamma[GAMMA_LCM_MAX][GAMMA_INDEX_MAX];

#endif

