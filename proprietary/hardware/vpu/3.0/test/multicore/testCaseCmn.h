#ifndef __VPU_TEST_3_0_CMN_H__
#define __VPU_TEST_3_0_CMN_H__

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream

#include <cutils/properties.h>  // For property_get().
#include <ctime>

#include <sys/types.h>
#include <unistd.h>

#undef LOG_TAG
#define LOG_TAG "[vpuUT_multicore]"

#define MY_LOGV(fmt, arg...)        printf(LOG_TAG "[%d/%d][VER] %s: " fmt "\n",getpid(), gettid(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        printf(LOG_TAG "[%d/%d][DEBUG] %s: " fmt "\n",getpid(), gettid(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        printf(LOG_TAG "[%d/%d][INFO] %s: " fmt "\n",getpid(), gettid(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        printf(LOG_TAG "[%d/%d][WARN] %s: " fmt "\n",getpid(), gettid(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        printf(LOG_TAG "[%d/%d][ERROR] %s: " fmt "\n",getpid(), gettid(), __func__, ##arg)

typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned int        UINT32;
typedef long                INT32;
typedef unsigned char       UINT8;
typedef char                INT8;
typedef bool                BOOL;

#endif
