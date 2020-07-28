/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <cutils/log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream
#include "test/vpu_data_wpp.h"
#include "vpu_lib_sett_ksample.h"     // algo. header


#include <cutils/properties.h>  // For property_get().
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_TAG
#define LOG_TAG "MtkCam.vpu_test"


typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned int        UINT32;
typedef long                INT32;
typedef unsigned char       UINT8;
typedef char                INT8;
typedef bool                BOOL;


#define TIME_PROF

#if defined(TIME_PROF)

#include <sys/time.h>

//#define LOG_TAG "VPU_UT"
#define LOGD printf
#define LOGE printf



typedef timeval MY_TIME;

#define MY_GET_TIME gettimeofday
#define MY_DIFF_TIME time_diff
#define MY_DISPLAY_TIME time_display
#define MY_RESET_TIME(a) memset(a, 0, sizeof(a))
#define MY_ADD_TIME(a, b) (a += b)

static void time_diff(INT32 *elapse_t, timeval start_t, timeval end_t)
{
    *elapse_t = (end_t.tv_sec - start_t.tv_sec)*1000000 + (end_t.tv_usec - start_t.tv_usec);
}
static void time_display(INT32 elapse_t, const char *string)
{
    printf("%s: %8d(us) = %5.4f(ms)\n", string, elapse_t, elapse_t/1000.0f);
}

INT32 elapse_time[12];
INT32 temp_time;
MY_TIME start_time, end_time, sub_start_time, sub_end_time;
#endif


#if 0
#define CAM_LOGV ALOGV
#define CAM_LOGD ALOGD
#define CAM_LOGI ALOGI
#define CAM_LOGW ALOGW
#define CAM_LOGE ALOGE
#define CAM_LOGA ALOGA
#define CAM_LOGF ALOGF
#else
#define CAM_LOGV printf
#define CAM_LOGD printf
#define CAM_LOGI printf
#define CAM_LOGW printf
#define CAM_LOGE printf
#define CAM_LOGA printf
#define CAM_LOGF printf
#endif

#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");

typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned int        UINT32;
typedef long                INT32;
typedef unsigned char       UINT8;
typedef char                INT8;
typedef bool                BOOL;

/******************************************************************************
 *
 ******************************************************************************/
static char gpSrcName[]    = "/data/vpu_test_src.raw";
static char gpSrcParam[]   = "/data/vpu_test_src_param.raw";
static char gpGoldenName[] = "/data/vpu_test_golden.raw";
static unsigned char *g_datasrc_wpp = NULL;
static unsigned char *g_datadst_golden_wpp = NULL;
static unsigned char *g_procParam = NULL;

typedef enum {
    E_VPU_ALGO_NONE = 0,
    E_VPU_ALGO_WARP,
    E_VPU_ALGO_MAX_PW,
    E_VPU_ALGO_SHIFT,
    E_VPU_ALGO_MAX,
} VPU_ALGO_E ;


static char const *vpu_algo_name[] = {"vpu_flo_d2d_k3", "vpu_flo_d2d_k3", "vpu_flo_d2d_k7", "vpu_flo_d2d_k5", "vpu_pil_d2d_k5"};

//#define TEST_ALGO_MAX_PW        "ipu_flo_d2d_k7"      // max. power pattern, no external mem. IO
//#define TEST_ALGO               "ipu_flo_d2d_k3"      // warping
#define TEST_ALGO_PORT1         "d2dk3ip0"
#define TEST_ALGO_PORT2         "d2dk3op1"
#define TEST_ALGO_PORT3         "d2dk3ip2"

#define TEST_SLT_ALGO_PORT1     "d2dk5ip0"
#define TEST_SLT_ALGO_PORT2     "d2dk5op1"
#define TEST_SLT_ALGO_PORT3     "d2dk5ip2"

#define TEST_SLT_IMG_WIDTH          64
#define TEST_SLT_IMG_HEIGHT         64
#define TEST_SLT_IMG_SIZE           (TEST_SLT_IMG_WIDTH*TEST_SLT_IMG_HEIGHT)

#define TEST_IMG_WIDTH0         1920    // image from binary file
#define TEST_IMG_WIDTH1         640     // image from header file

#define MAX_MULTI_USER_CNT      20
#define MAX_MULTI_USER_QUE_CNT  20

#define AUTO_TEST_CNT           2
#define AUTO_TEST_USER_CNT      3
#define AUTO_TEST_QUEUE_CNT     3
#define AUTO_TEST_FLUSH_CNT     2

#define AUTO_TEST_PW_CNT          100
#define AUTO_TEST_PW_PRE_ENQ_CNT  5
char *algo_name   =  NULL; //(char *)vpu_algo_name[0];
char port1_name[] = TEST_ALGO_PORT1;
char port2_name[] = TEST_ALGO_PORT2;
char port3_name[] = TEST_ALGO_PORT3;
VPU_ALGO_E g_coreA_test_pattern = E_VPU_ALGO_WARP;
VPU_ALGO_E g_coreB_test_pattern = E_VPU_ALGO_WARP;
VPU_ALGO_E g_common_test_pattern = E_VPU_ALGO_WARP;
static sem_t     mSem;
static bool g_non_stop;
static int g_time_exec;
static int g_time_sleep;

int ion_drv_handle = 0;

// image width, height, size, data length
static UINT32 gTestImgWidth   = 0;
static UINT32 gTestImgHeight  = 0;
static UINT32 gTestImgSize    = 0;
static UINT32 gTestDataLength = 0;

// save test image data pointer into global variables due to create threads
static UINT8 *pgTest_img1 = NULL, *pgTest_img2 = NULL, *pgTest_data1 = NULL;
static UINT32 multi_user_que_cnt = 2, flush_que_cnt = 0; //old usage
static UINT32 coreA_multi_user_que_cnt = 2, coreA_flush_que_cnt = 0;
static UINT32 coreB_multi_user_que_cnt = 2, coreB_flush_que_cnt = 0;
static UINT32 common_multi_user_que_cnt = 2;
static UINT32 gDumpImage = 0;
static bool coreA_thread_need_wait = false;
static bool coreB_thread_need_wait = false;
static bool common_thread_need_wait = false;
static UINT32 gVcoreOpp=4;
//static UINT32 gFreqOpp=0;
static bool coreA_adjust_opp = false, coreB_adjust_opp = false;
static bool coreA_random_opp = false, coreB_random_opp = false;
#define MaxUINT 65535


enum {
  VPU_TEST_SUCCESS = 1 << 0,
  VPU_TEST_FAIL    = 1 << 1,
} ;

typedef struct d2d_k3_property_structure {
	int           param1;
	int           param2;
	int           param3;
	int           param4;
	int           param5;
} d2d_k3_property_structure_t;


static void save_to_file(char *filename, char *pBuf, int size)
{
   FILE *fp;
   MY_LOGD("================================");
   MY_LOGD("save file: %s", filename);
   if ((fp = fopen(filename, "wb")) == (FILE *)NULL)
  {
    MY_LOGE("fail to open file : %s", filename);
    return;
  }
  fwrite(pBuf, size, 1, fp);
  fclose(fp);
}

static void _vpu_setup_buf(VpuBuffer &buf, int buf_fd, VpuBufferFormat format,
                           unsigned int width,  unsigned int height, unsigned int buf_offset,
                           unsigned int stride, unsigned int length)
{
    buf.format      = format;
    buf.width       = width;
    buf.height      = height;
    buf.planeCount  = 1;

    /* ion case */
    buf.planes[0].fd      = (unsigned int)buf_fd;
    buf.planes[0].offset  = buf_offset;
    buf.planes[0].stride  = stride;
    buf.planes[0].length  = length;
}

static void _vpu_setup_test_pattern(UINT32 test_pattern, UINT8 **test_img1,
                                    UINT8   **test_img2, UINT8 **test_data1)
{
    switch(test_pattern)
    {
        case 0:
        default:
            *test_img1  = g_datasrc_wpp;
            *test_img2  = g_datadst_golden_wpp;
            *test_data1 = g_procParam;
            break;
    }
}

static BOOL _vpu_get_buffer(int drv_h, int len, int *buf_share_fd, UINT8 **buf_va)
{
    ion_user_handle_t buf_handle;
    // allocate ion buffer handle
    if(ion_alloc_mm(drv_h, (size_t)len, 0, 0, &buf_handle))        // no alignment, non-cache
    {
        MY_LOGW("fail to get ion buffer handle (drv_h=0x%x, len=%d)", drv_h, len);
        return 0;
    }
    // get ion buffer share handle
    if(ion_share(drv_h, buf_handle, buf_share_fd))
    {
        MY_LOGW("fail to get ion buffer share handle");
        if(ion_free(drv_h,buf_handle))
            MY_LOGW("ion free fail");
        return 0;
    }
    // get buffer virtual address
    *buf_va = (UINT8 *)ion_mmap(drv_h, NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, *buf_share_fd, 0);
    MY_LOGV("alloc ion: ion_buf_handle %d, share_fd %d", buf_handle, *buf_share_fd);
    if(*buf_va != NULL)
        return 1;
    else
    {
        MY_LOGW("fail to get buffer virtual address");
        return 0;
    }
}

static BOOL _vpu_sync_buffer(int drv_h, int len, int buf_share_fd, UINT8 *buf_va)
{
    struct ion_sys_data sys_data;
    ion_user_handle_t buf_handle;
    BOOL ret = false;

    //pthread_mutex_lock(&mutex);

    /* check arguments */
    if(drv_h == 0 || len <= 0 || buf_share_fd == 0 || buf_va == nullptr)
    {
        MY_LOGE("[vputest][sync] invalid argument (%d/%d/%d/%p)\n", drv_h, len, buf_share_fd, buf_va);

        //pthread_mutex_unlock(&mutex);
        return false;
    }

    /* get handle from shared fd */
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        MY_LOGW("fail to get import share buffer fd");
        //pthread_mutex_unlock(&mutex);
        return false;
    }

    /* sync ion memory */
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = buf_handle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    sys_data.cache_sync_param.va = (void *)buf_va;
    sys_data.cache_sync_param.size = len;
    if(ion_custom_ioctl(drv_h, ION_CMD_SYSTEM, &sys_data))
    {
        MY_LOGE("[vputest][sync] %s ion custom fail\n", __func__);
        ret = false;
        goto out;
    }

    ret = true;

out:
    /* free ref count cause by ion import */
    if(ion_free(drv_h, buf_handle))
    {
        MY_LOGE("[vputest][sync] %s ion free cnt\n", __func__);
        ret = false;
    }
    //pthread_mutex_unlock(&mutex);
    return ret;
}

static BOOL _vpu_free_buffer(int drv_h, int len,int buf_share_fd, UINT8 *buf_va)
{
    ion_user_handle_t buf_handle;
    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        MY_LOGW("fail to get import share buffer fd");
        return 0;
    }
    MY_LOGV("import ion: ion_buf_handle %d, share_fd %d", buf_handle, buf_share_fd);
    // 2. free for IMPORT ref cnt
    if(ion_free(drv_h, buf_handle))
    {
        MY_LOGW("fail to free ion buffer (free ion_import ref cnt)");
        return 0;
    }
    // 3. unmap virtual memory address
    if(ion_munmap(drv_h, (void *)buf_va, (size_t)len))
    {
        MY_LOGW("fail to get unmap virtual memory");
        return 0;
    }
    // 4. close share buffer fd
    if(ion_share_close(drv_h, buf_share_fd))
    {
        MY_LOGW("fail to close share buffer fd");
        return 0;
    }
    // 5. pair of ion_alloc_mm
    if(ion_free(drv_h, buf_handle))
    {
        MY_LOGW("fail to free ion buffer (free ion_alloc_mm ref cnt)");
        return 0;
    }

    return 1;
}

void vpu_slt_test(UINT32 test_count)
{
    int     result = VPU_TEST_SUCCESS, img1_fd = 0, img2_fd = 0;
    UINT8  *img1_va = NULL, *img2_va = NULL;
    UINT32 i, j;
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    VpuPort    *port1, *port2;
    VpuBuffer   buf_img1, buf_img2;
    VpuRequest *request = NULL;

    // setup ion buffer (image1, image2, data1)
    if(!_vpu_get_buffer(ion_drv_handle, TEST_SLT_IMG_SIZE, &img1_fd, &img1_va))
    {
        MY_LOGW("fail to get img1 ion buffer");
        goto EXIT;
    }
    if(!_vpu_get_buffer(ion_drv_handle, TEST_SLT_IMG_SIZE, &img2_fd, &img2_va))
    {
        MY_LOGW("fail to get img2 ion buffer");
        goto EXIT;
    }

    // 1. init vpu
    vpu = VpuStream::createInstance("vpu_slt_test");
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }

    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    // 2. get port
    port1 = algo->getPort(TEST_SLT_ALGO_PORT1);
    port2 = algo->getPort(TEST_SLT_ALGO_PORT2);
    if (!port1 || !port2)
    {
        MY_LOGW("port1 0x%lx, port2 0x%lx", (unsigned long)port1, (unsigned long)port2);
        MY_LOGW("some Port is NULL");
        goto EXIT;
    }
    // 2.1 prepare buffer for port1, port2
    _vpu_setup_buf(buf_img1,img1_fd,eFormatImageY8,TEST_SLT_IMG_WIDTH, TEST_SLT_IMG_HEIGHT, 0,TEST_SLT_IMG_WIDTH,TEST_SLT_IMG_SIZE);
    _vpu_setup_buf(buf_img2,img2_fd,eFormatImageY8,TEST_SLT_IMG_WIDTH, TEST_SLT_IMG_HEIGHT, 0,TEST_SLT_IMG_WIDTH,TEST_SLT_IMG_SIZE);

    // 3. setup request
    // 3.1 acquire request
    request = vpu->acquire(algo);
    if ((request = vpu->acquire(algo)) == NULL)
    {
        MY_LOGW("fail to get vpu request !!!");
        goto EXIT;
    }

    // 3.2 add buffers to request
    request->addBuffer(port1, buf_img1);
    request->addBuffer(port2, buf_img2);

    MY_LOGD("start to test: test_count %d", test_count);
    for(i = 0 ; i < test_count ; i++)
    {
        VpuRequest *deque_req;

        // init source image and reset destination image
        for (j = 0 ; j < TEST_SLT_IMG_SIZE ; j++)
        {
            img1_va[j] = j;
        }
        memset((void *)img2_va , 0, TEST_SLT_IMG_SIZE); // clean output image buffer
        MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, i);
        // 4. enqueue & dequeue
        vpu->enque(request);
        deque_req = vpu->deque();

        MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);
        // 5. check result
        if(deque_req == request)
        {
            for (j = 0 ; j < TEST_SLT_IMG_SIZE ; j++)
            {
                if (img2_va[j] != (img1_va[j] << 1))
                {
                  MY_LOGE("compare test result failed (iterm[%d]: %d != %d)", j, img2_va[j], (img1_va[j] << 1));
                  result |= VPU_TEST_FAIL;
                  break;
                }
            }
        }
        else
        {
            result |= VPU_TEST_FAIL;
            MY_LOGW("test failed !!!, get wrong deque request (req=%p, deque_req=%p)",request, deque_req);
        }
    }

    if (result == VPU_TEST_SUCCESS)
    {
        MY_LOGW("Test Succeeded");
    }
    else
    {
        MY_LOGW("test failed !!!");
    }

EXIT:
    // 6. release resource & exit

    if (request)
        vpu->release(request);

    if (vpu)
        delete vpu;
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, TEST_SLT_IMG_SIZE, img1_fd, img1_va))
        MY_LOGW("fail to free img1 ion buffer");

    if(img2_va && !_vpu_free_buffer(ion_drv_handle, TEST_SLT_IMG_SIZE, img2_fd, img2_va))
        MY_LOGW("fail to free img2 ion buffer");

    MY_LOGI("test finish");
}
int vpu_enque_test(UINT32 test_pattern,UINT32 test_count, UINT32 core)
{
    int      i;
    int         result = 0, img1_fd = 0, img2_fd = 0, data1_fd = 0;
    UINT8      *img1_va = NULL, *img2_va = NULL, *data1_va = NULL;
    VpuStream  *vpu  = NULL;
    VpuAlgo    *algo = NULL;
    VpuRequest *request = NULL;
    VpuBuffer   buf_img1, buf_img2, buf_data1;
	std::vector<VpuStatus> vpu_status;
	bool res=false;
	VpuCoreIndex coreIndex = eIndex_NONE;
	VpuExtraParam extraP;

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        goto EXIT;
    }
    // setup ion buffer (image1, image2, data1)
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("fail to get img1 ion buffer");
        goto EXIT;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd, &img2_va))
    {
        MY_LOGW("fail to get img2 ion buffer");
        goto EXIT;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd, &data1_va))
    {
        MY_LOGW("fail to get data1 ion buffer");
        goto EXIT;
    }

    // init test image
    memcpy((void *)img1_va, (const void *)pgTest_img1, gTestImgSize);

    // 1. init vpu
    vpu = VpuStream::createInstance("vpu_enque_test");
    if (vpu == NULL)
    {
    	printf("fail to get vpu handle\n");
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }

#if 0 // test setPower and MayPhyAddr
    bool bRet;
    VpuPower pwr;
    pwr.mode = ePowerModeOn;
    pwr.opp  = 0;
    bRet = vpu->setPower(pwr, eIndex_A);
    MY_LOGD("setPower bRet = %d", bRet);
#endif


	//
	switch(core)
	{
		case 0:
		default:
			coreIndex = eIndex_NONE;
			break;
		case 1:
			algo_name = (char *)vpu_algo_name[test_pattern];
			coreIndex = eIndex_A;
			break;
		case 2:
			algo_name = (char *)vpu_algo_name[test_pattern]; //should be the same with vpu0
			coreIndex = eIndex_B;
			break;
		case 3:
			algo_name = (char *)vpu_algo_name[test_pattern]; //should be the same with vpu0
			coreIndex = eIndex_C;
			break;
	}
	//
    // 1.1 init algo of vpu
    MY_LOGW("algo_name(%s)\n",algo_name);
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    // 2. get port
	MY_LOGD("flag 1");

    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1,img1_fd,eFormatImageY8,gTestImgWidth,gTestImgHeight,0,gTestImgWidth,gTestImgSize);
    _vpu_setup_buf(buf_img2,img2_fd,eFormatImageY8,gTestImgWidth,gTestImgHeight,0,gTestImgWidth,gTestImgSize);
    _vpu_setup_buf(buf_data1,data1_fd,eFormatData,0,0,0,0,gTestDataLength);
	MY_LOGD("flag 2");
    // 3. setup request
    // 3.1 acquire request
    request = vpu->acquire(algo);
	MY_LOGD("flag 3");
    // 3.2 add buffers to request
	if(test_pattern == 0 || test_pattern ==1)
	{//k3
		//fill id in buffer structure,
		buf_img1.port_id = 0;
		request->addBuffer(buf_img1);

		buf_img2.port_id = 1;
		request->addBuffer(buf_img2);

		buf_data1.port_id = 2;
		request->addBuffer(buf_data1);
	}

	MY_LOGD("flag 4");
    // 3.3 set Property to request
	if(test_pattern == 0 || test_pattern ==1)
	{//k3
		d2d_k3_property_structure_t property;
		property.param1 = 1;
		property.param2 = 2;
		property.param3 = 3;
		property.param4 = 4;
		property.param5 = 1;
		request->setProperty((void*)&property, sizeof(d2d_k3_property_structure_t));
	}
	//
	MY_LOGD("flag4.5");

	extraP.opp_step = gVcoreOpp;
	//extraP.freq_step = gFreqOpp;
	request->setExtraParam(extraP);

	MY_LOGD("flag 5");
    // get vpu status
	res = vpu->getVpuStatus(vpu_status);
	for (i = 0 ; i < (int)vpu_status.size() ; i++)
	{
		printf("(%d)->idx(%d), ava(%d), waitNum(%d)\n", i,
			vpu_status[i].core_index, vpu_status[i].available,
			vpu_status[i].waiting_requests_num);
	}


    MY_LOGD("start to test: test_count %d", test_count);
    for(i = 0 ; i < (int)test_count ; i++)
    {
    	VpuRequest *deque_req;
		char        buffer[50];
    	/* harris corner only do enque/deque*/
    	if(test_pattern != 2)
		{
	        memset((void *)img2_va , 0, gTestImgSize);                             // clean output image buffer
	        memcpy((void *)data1_va, (const void *)pgTest_data1, gTestDataLength); // init test data, MUST copy again because vpu will modify params after executing d2d
	    }
		MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, i);
        // 4. enqueue & dequeue
        vpu->enque(request, coreIndex);
        deque_req = vpu->deque();

        // 5. check result
        if(test_pattern == 2)
        {
        	if(deque_req == request)
	        {
	        	result |= VPU_TEST_SUCCESS;
        	}
			else
	        {
	            result |= VPU_TEST_FAIL;
	            MY_LOGW("test failed !!!, get wrong deque request (req=%p, deque_req=%p)",request, deque_req);
	        }
        }
		else
		{
	        if(deque_req == request)
	        {
	            int result_cmp = 0;
				if(test_pattern == 0 || test_pattern ==1)
				{//k3
					result_cmp = memcmp((unsigned char*)img2_va,(unsigned char*)pgTest_img2,gTestImgSize);
				}
	            if(result_cmp == 0)
	            {
	                MY_LOGI("test success");
	                result |= VPU_TEST_SUCCESS;
	            }
	            else
	            {
	                result |= VPU_TEST_FAIL;
	                MY_LOGW("compare test result failed (%d)",result_cmp);
	            }
	        }
	        else
	        {
	            result |= VPU_TEST_FAIL;
	            MY_LOGW("test failed !!!, get wrong deque request (req=%p, deque_req=%p)",request, deque_req);
	        }

	        if (i == test_count - 1 && gDumpImage)
	        {
	            snprintf(buffer, 50, "/data/vpu_test_dst%d.raw", i);
	            save_to_file(buffer, (char *)img2_va, gTestImgSize);
	        }
        }
    }

EXIT:
    // 6. release resource & exit
    //free((void*)property_data);
	//free((void*)port1);
	//free((void*)port2);
	//free((void*)port3);

    if (request)
        vpu->release(request);
    if (vpu)
        delete vpu;

    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
        MY_LOGW("fail to free img1 ion buffer");

    if(img2_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd, img2_va))
        MY_LOGW("fail to free img2 ion buffer");

    if(data1_va && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd, data1_va))
        MY_LOGW("fail to free data1 ion buffer");

    MY_LOGI("test finish");
    return result;
}

static void* _vpu_test_thread(void *arg)
{
    VpuStream  *vpu;
    VpuAlgo    *algo;
    VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2[MAX_MULTI_USER_QUE_CNT], buf_data1[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *request[MAX_MULTI_USER_QUE_CNT];
    int         result = VPU_TEST_SUCCESS;

    int         img1_fd,  img2_fd[MAX_MULTI_USER_QUE_CNT],  data1_fd[MAX_MULTI_USER_QUE_CNT];
    UINT8      *img1_va, *img2_va[MAX_MULTI_USER_QUE_CNT], *data1_va[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *deque_req = NULL;
    UINT32 que_cnt = 2, i, j, flushed = 0;

    vpu = (VpuStream *)arg;
    MY_LOGD("thread: user: 0x%lx", (unsigned long)vpu);
    // initialize
    img1_va  = NULL;
    img1_fd  = 0 ;
    for (i = 0 ; i < MAX_MULTI_USER_QUE_CNT ; i++)
    {
        request[i]  = NULL;
        img2_va[i]  = NULL;
        data1_va[i] = NULL;
        img2_fd[i]  = 0;
        data1_fd[i] = 0;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("fail to get img1 ion buffer");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }

    // init test image data
    memcpy((void *)img1_va ,(const void *)pgTest_img1 , gTestImgSize);

    if(multi_user_que_cnt > MAX_MULTI_USER_QUE_CNT)
        que_cnt = MAX_MULTI_USER_QUE_CNT;
    else
        que_cnt = multi_user_que_cnt;

    for(i = 0 ; i < que_cnt ; i++)
    {
        if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd[i], &img2_va[i]))
        {
            MY_LOGW("fail to get img2 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd[i], &data1_va[i]))
        {
            MY_LOGW("fail to get data1 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
    }
    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
    // 2. get port
    port1 = algo->getPort(port1_name);
    port2 = algo->getPort(port2_name);
    port3 = algo->getPort(port3_name);
    if (!port1 || !port2 || !port3)
    {
        MY_LOGW("port1 0x%lx, port2 0x%lx, port3 0x%lx", (unsigned long)port1, (unsigned long)port2, (unsigned long)port3);
        MY_LOGW("some Port is NULL");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1, img1_fd, eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    for(i = 0 ; i < que_cnt ; i++)
    {
        _vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        _vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);

    }
    // 3. setup request & enque
    for(i = 0 ; i < que_cnt ; i++)
    {
        request[i] = vpu->acquire(algo);
        request[i]->addBuffer(port1, buf_img1);
        request[i]->addBuffer(port2, buf_img2[i]);
        request[i]->addBuffer(port3, buf_data1[i]);
        {
            int param1 = 1;
            int param2 = 2;
            int param3 = 3;
            int param4 = 4;

            VpuProperty prop;
            prop.type = eTypeInt32;
            prop.count = 1;

            prop.data.i32 = &param1;
            request[i]->setProperty("param1", prop);

            prop.data.i32 = &param2;
            request[i]->setProperty("param2", prop);

            prop.data.i32 = &param3;
            request[i]->setProperty("param3", prop);

            prop.data.i32 = &param4;
            request[i]->setProperty("param4", prop);
        }
        vpu->enque(request[i]);
    }

    // 4. dequeue
    for(i = 0 ; i < que_cnt ; i++)
    {
        if(i == flush_que_cnt)
        {
            MY_LOGI("flush queue from queue cnt=%d",flush_que_cnt);
            vpu->flush();
            flushed = 1;
        }
        deque_req = vpu->deque();
        // 5. check result
        if(deque_req != request[i])
        {
            MY_LOGW("get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
            for(j = 0 ; j < que_cnt ; j++)
            {
                if(deque_req == request[j])
                    MY_LOGW("deque_req is same as queue %d", j);
            }
            if(j >= que_cnt)
                MY_LOGW("deque_req is different with any request");
            result |= VPU_TEST_FAIL;
        }
        if (!flushed)
        {
            int result_cmp = memcmp(img2_va[i], (unsigned char*)pgTest_img2, gTestImgSize);
            if(result_cmp == 0)
            {
                MY_LOGI("queue %d compare ok", i);
            }
            else
            {
                result |= VPU_TEST_FAIL;
                MY_LOGW("queue %d compare failed", i);
            }
        }
        if (gDumpImage)
        {
          char buffer[50];
          snprintf(buffer, 50, "/data/vpu_multi_dst_%d_%d.raw", gettid(), i);
          save_to_file(buffer, (char *)img2_va[i], gTestImgSize);
        }
    }

EXIT:
    // 6. release resource
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
        MY_LOGW("fail to free img1 ion buffer");
    for(i = 0 ; i < que_cnt ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
        if(data1_va[i] && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd[i], data1_va[i]))
            MY_LOGW("fail to free data1 ion buffer %d", i);
        if(img2_va[i]  && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd[i], img2_va[i]))
            MY_LOGW("fail to free img2 ion buffer for queue %d", i);
    }
    if(result == VPU_TEST_SUCCESS)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);

    return  NULL;
}


int vpu_multi_user_test(UINT32 test_pattern, UINT32 multi_user_cnt, UINT32 que_cnt, UINT32 flush_que)
{
    int result = 0;
    void *thread_ret;

    VpuStream *vpu[MAX_MULTI_USER_CNT];
    pthread_t  vpu_thread[MAX_MULTI_USER_CNT];
    UINT32     i, j, test_user_cnt;

    if(multi_user_cnt > MAX_MULTI_USER_CNT)
        test_user_cnt = MAX_MULTI_USER_CNT;
    else
        test_user_cnt = multi_user_cnt;

    MY_LOGD("multi user test: user_cnt: %d, que_cnt %d", test_user_cnt, que_cnt);

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        return result;
    }

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        // 1. init vpu
        vpu[i] = VpuStream::createInstance("vpu_multi_user_test");
        if (vpu[i] == NULL)
        {
            MY_LOGW("fail to get vpu handle");
            for(j = 0 ; j < i ; j++)
                delete vpu[j];
            return result;
        }
    }

    multi_user_que_cnt = que_cnt;
    flush_que_cnt      = flush_que;

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        MY_LOGD("creat thread: user: 0x%lx", (unsigned long)vpu[i]);
        pthread_create(&vpu_thread[i], NULL, _vpu_test_thread, (void *)vpu[i]);
    }

    result = VPU_TEST_SUCCESS;
    for(i = 0 ; i < test_user_cnt ; i++)
    {
        pthread_join(vpu_thread[i], &thread_ret);
        if(thread_ret == 0)
        {
            MY_LOGW("vpu multi-thread test fail (thread = %d, result=0x%lx)", i, (unsigned long)thread_ret);
            result |= VPU_TEST_FAIL;
        }
        delete vpu[i];
    }
    return result;
}


static void* _vpu_coreA_test_thread(void *arg)
{
    VpuStream  *vpu;
    VpuAlgo    *algo;
    VpuBuffer   buf_img1, buf_img2[MAX_MULTI_USER_QUE_CNT], buf_data1[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *request[MAX_MULTI_USER_QUE_CNT];
    int         result = VPU_TEST_SUCCESS;

    int         img1_fd,  img2_fd[MAX_MULTI_USER_QUE_CNT],  data1_fd[MAX_MULTI_USER_QUE_CNT];
    UINT8      *img1_va, *img2_va[MAX_MULTI_USER_QUE_CNT], *data1_va[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *deque_req = NULL;
    UINT32 que_cnt = 2, i, j, flushed = 0;
	int original_cnt = 2;
	VpuCoreIndex coreIndex = eIndex_NONE;
	srand( time(NULL) );
	int rand_value=0;
	VpuExtraParam extraP;
	bool first_time = true;

    vpu = (VpuStream *)arg;
    MY_LOGD("[coreA] thread: user: 0x%lx", (unsigned long)vpu);
    // initialize
    img1_va  = NULL;
    img1_fd  = 0 ;
    for (i = 0 ; i < MAX_MULTI_USER_QUE_CNT ; i++)
    {
        request[i]  = NULL;
        img2_va[i]  = NULL;
        data1_va[i] = NULL;
        img2_fd[i]  = 0;
        data1_fd[i] = 0;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("[coreA] fail to get img1 ion buffer");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }

    // init test image data
    memcpy((void *)img1_va ,(const void *)pgTest_img1 , gTestImgSize);

	original_cnt = coreA_multi_user_que_cnt;
    if(coreA_multi_user_que_cnt > MAX_MULTI_USER_QUE_CNT)
        que_cnt = MAX_MULTI_USER_QUE_CNT;
    else
        que_cnt = coreA_multi_user_que_cnt;

    for(i = 0 ; i < que_cnt ; i++)
    {
        if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd[i], &img2_va[i]))
        {
            MY_LOGW("[coreA] fail to get img2 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd[i], &data1_va[i]))
        {
            MY_LOGW("[coreA] fail to get data1 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
    }

	//
	algo_name = (char *)vpu_algo_name[g_coreA_test_pattern];
	coreIndex = eIndex_A;


	if(coreA_thread_need_wait)
	{

::sem_wait(&mSem);
	}
	else
	{
		::sem_post(&mSem);
	}

    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("[coreA] fail to get vpu algo handle");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
	MY_LOGD("[coreA] flag 1");
    // 2. get port

    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1, img1_fd, eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    for(i = 0 ; i < que_cnt ; i++)
    {
        _vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        _vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);

    }
	MY_LOGD("[coreA] flag 2");
    // 3. setup request & enque
    first_time = true;
    while(original_cnt > 0)
    {
	    for(i = 0 ; i < que_cnt ; i++)
	    {
	    	/* harris corner only do enque/deque*/
	    	if(g_coreA_test_pattern != 2)
			{
		        memset((void *)img2_va[i] , 0, gTestImgSize);                             // clean output image buffer
		        memcpy((void *)data1_va[i], (const void *)pgTest_data1, gTestDataLength); // init test data, MUST copy again because vpu will modify params after executing d2d
		    }

	    	if(first_time)
	    	{
		        request[i] = vpu->acquire(algo);
				if(g_coreA_test_pattern == 0 || g_coreA_test_pattern ==1)
				{//k3
					buf_img1.port_id = 0;
			        request[i]->addBuffer(buf_img1);
					buf_img2[i].port_id = 1;
			        request[i]->addBuffer(buf_img2[i]);
					buf_data1[i].port_id = 2;
			        request[i]->addBuffer(buf_data1[i]);
			        {
						d2d_k3_property_structure_t property;
						property.param1 = 1;
						property.param2 = 2;
						property.param3 = 3;
						property.param4 = 4;
						property.param5 = 1;
						request[i]->setProperty((void*)&property, sizeof(d2d_k3_property_structure_t));
			        }
				}
	    	}

			if(coreA_adjust_opp)
			{
				if(!coreA_random_opp)
				{ //sequentially set opp/freq
					#if 1
					extraP.opp_step = i%8;
					#else
					extraP.opp_step = (i/4)%2;
					extraP.freq_step = i%4;
					#endif
				}
				else
				{
					rand_value = rand() % 3;
					#if 1
					extraP.opp_step = rand() % 8;
					#else
					extraP.opp_step = rand() % 2;
					extraP.freq_step = rand() % 4;
					#endif
				}
				MY_LOGW("[coreA] value(%d/%d/%d)", rand_value, extraP.opp_step, extraP.freq_step);
				usleep(rand_value * 1000);
				request[i]->setExtraParam(extraP);
			}
	        vpu->enque(request[i], coreIndex);
	    }

	    // 4. dequeue
	    for(i = 0 ; i < que_cnt ; i++)
	    {
	    	#if 0 //todo..
	        if(i == flush_que_cnt)
	        {
	            MY_LOGI("flush queue from queue cnt=%d",flush_que_cnt);
	            vpu->flush();
	            flushed = 1;
	        }
			#endif
	        deque_req = vpu->deque();
	        // 5. check result
	        if(deque_req != request[i])
	        {
	            MY_LOGW("[coreA] get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
	            for(j = 0 ; j < que_cnt ; j++)
	            {
	                if(deque_req == request[j])
	                    MY_LOGW("[coreA] deque_req is same as queue %d", j);
	            }
	            if(j >= que_cnt)
	                MY_LOGW("[coreA] deque_req is different with any request");
	            result |= VPU_TEST_FAIL;
	        }
			//non_stop for HQA
			if(g_non_stop)
			{	//while loop
				vpu->enque(request[i], coreIndex);
				deque_req = vpu->deque();
		        // 5. check result
		        if(deque_req != request[i])
		        {
		            MY_LOGW("[coreA] get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
		            result |= VPU_TEST_FAIL;
					break;
		        }
			}
			//

	        if (!flushed)
	        {
	            int result_cmp = 0;
				if(g_coreA_test_pattern == 0 || g_coreA_test_pattern ==1)
				{//k3
					result_cmp = memcmp(img2_va[i], (unsigned char*)pgTest_img2, gTestImgSize);
				}
	            if(result_cmp == 0)
	            {
	                MY_LOGI("[coreA] queue %d compare ok", i);
	            }
	            else
	            {
	                result |= VPU_TEST_FAIL;
	                MY_LOGW("[coreA] queue %d compare failed, %d", i, result_cmp);
	            }
	        }
	        if (gDumpImage)
	        {
	          char buffer[50];
	          snprintf(buffer, 50, "/data/vpu_coreA_dst_%d_%d.raw", gettid(), i);
	          save_to_file(buffer, (char *)img2_va[i], gTestImgSize);
	        }
	    }

		original_cnt = original_cnt - (int)que_cnt;
		first_time = false;
		MY_LOGW("[coreA] original_cnt, que_cnt(%d, %d)\n", original_cnt, que_cnt);
		//
		for(i = 0 ; i < que_cnt ; i++)
	    {
	        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
	        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);

			_vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        	_vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
	    }
    }


EXIT:
    // 6. release resource
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
        MY_LOGW("[coreA] fail to free img1 ion buffer");
    for(i = 0 ; i < que_cnt ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
        if(data1_va[i] && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd[i], data1_va[i]))
            MY_LOGW("[coreA] fail to free data1 ion buffer %d", i);
        if(img2_va[i]  && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd[i], img2_va[i]))
            MY_LOGW("[coreA] fail to free img2 ion buffer for queue %d", i);
    }
    if(result == VPU_TEST_SUCCESS)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);

    return  NULL;
}

static void* _vpu_coreB_test_thread(void *arg)
{
    VpuStream  *vpu;
    VpuAlgo    *algo;
    VpuBuffer   buf_img1, buf_img2[MAX_MULTI_USER_QUE_CNT], buf_data1[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *request[MAX_MULTI_USER_QUE_CNT];
    int         result = VPU_TEST_SUCCESS;

    int         img1_fd,  img2_fd[MAX_MULTI_USER_QUE_CNT],  data1_fd[MAX_MULTI_USER_QUE_CNT];
    UINT8      *img1_va, *img2_va[MAX_MULTI_USER_QUE_CNT], *data1_va[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *deque_req = NULL;
    UINT32 que_cnt = 2, i, j, flushed = 0;
	int original_cnt = 2;
	VpuCoreIndex coreIndex = eIndex_NONE;
	srand( time(NULL) );
	int rand_value=0;
	VpuExtraParam extraP;
	bool first_time = true;

    vpu = (VpuStream *)arg;
    MY_LOGD("[coreB] thread: user: 0x%lx", (unsigned long)vpu);
    // initialize
    img1_va  = NULL;
    img1_fd  = 0 ;
    for (i = 0 ; i < MAX_MULTI_USER_QUE_CNT ; i++)
    {
        request[i]  = NULL;
        img2_va[i]  = NULL;
        data1_va[i] = NULL;
        img2_fd[i]  = 0;
        data1_fd[i] = 0;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("[coreB] fail to get img1 ion buffer");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }

    // init test image data
    memcpy((void *)img1_va ,(const void *)pgTest_img1 , gTestImgSize);

	original_cnt = coreB_multi_user_que_cnt;
    if(coreB_multi_user_que_cnt > MAX_MULTI_USER_QUE_CNT)
        que_cnt = MAX_MULTI_USER_QUE_CNT;
    else
        que_cnt = coreB_multi_user_que_cnt;

    for(i = 0 ; i < que_cnt ; i++)
    {
        if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd[i], &img2_va[i]))
        {
            MY_LOGW("[coreB] fail to get img2 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd[i], &data1_va[i]))
        {
            MY_LOGW("[coreB] fail to get data1 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
    }

	//
	algo_name = (char *)vpu_algo_name[g_coreB_test_pattern];
	coreIndex = eIndex_B;

    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("[coreB] fail to get vpu algo handle");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
	MY_LOGD("[coreB] flag 1");
    // 2. get port

    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1, img1_fd, eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    for(i = 0 ; i < que_cnt ; i++)
    {
        _vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        _vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);

    }
	MY_LOGD("[coreB] flag 2");
    // 3. setup request & enque
    first_time = true;
    while(original_cnt > 0)
    {
	    for(i = 0 ; i < que_cnt ; i++)
	    {
	    	/* harris corner only do enque/deque*/
	    	if(g_coreB_test_pattern != 2)
			{
		        memset((void *)img2_va[i] , 0, gTestImgSize);                             // clean output image buffer
		        memcpy((void *)data1_va[i], (const void *)pgTest_data1, gTestDataLength); // init test data, MUST copy again because vpu will modify params after executing d2d
		    }

	    	if(first_time)
	    	{
		    	request[i] = vpu->acquire(algo);
		    	if(g_coreB_test_pattern == 0 || g_coreB_test_pattern == 1)
				{//k3
					buf_img1.port_id = 0;
			        request[i]->addBuffer(buf_img1);
					buf_img2[i].port_id = 1;
			        request[i]->addBuffer(buf_img2[i]);
					buf_data1[i].port_id = 2;
			        request[i]->addBuffer(buf_data1[i]);
			        {
						d2d_k3_property_structure_t property;
						property.param1 = 1;
						property.param2 = 2;
						property.param3 = 3;
						property.param4 = 4;
						property.param5 = 1;
						request[i]->setProperty((void*)&property, sizeof(d2d_k3_property_structure_t));
			        }
		    	}
	    	}

			if(coreB_adjust_opp)
			{
				if(!coreB_random_opp)
				{	//sequentially set opp/freq
					#if 1
					extraP.opp_step = i%8;
					#else
					extraP.opp_step = (i/4)%2;
					extraP.freq_step = i%4;
					#endif
				}
				else
				{
					rand_value = rand() % 5;
					#if 1
					extraP.opp_step = rand() % 8;
					#else
					extraP.opp_step = rand() % 2;
					extraP.freq_step = rand() % 4;
					#endif
				}
				MY_LOGW("[coreB] value(%d/%d/%d)", rand_value, extraP.opp_step, extraP.freq_step);
				usleep(rand_value * 1000);
				request[i]->setExtraParam(extraP);
			}

	        vpu->enque(request[i], coreIndex);
	    }

	    // 4. dequeue
	    for(i = 0 ; i < que_cnt ; i++)
	    {
	    	#if 0 //todo..
	        if(i == flush_que_cnt)
	        {
	            MY_LOGI("flush queue from queue cnt=%d",flush_que_cnt);
	            vpu->flush();
	            flushed = 1;
	        }
			#endif
	        deque_req = vpu->deque();
	        // 5. check result
	        if(deque_req != request[i])
	        {
	            MY_LOGW("[coreB] get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
	            for(j = 0 ; j < que_cnt ; j++)
	            {
	                if(deque_req == request[j])
	                    MY_LOGW("[coreB] deque_req is same as queue %d", j);
	            }
	            if(j >= que_cnt)
	                MY_LOGW("[coreB] deque_req is different with any request");
	            result |= VPU_TEST_FAIL;
	        }

			//non_stop for HQA
			if(g_non_stop)
			{	//while loop
				vpu->enque(request[i], coreIndex);
				deque_req = vpu->deque();
		        // 5. check result
		        if(deque_req != request[i])
		        {
		            MY_LOGW("[coreA] get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
		            result |= VPU_TEST_FAIL;
					break;
		        }
			}
			//

	        if (!flushed)
	        {
	            int result_cmp = 0;
				if(g_coreB_test_pattern == 0 || g_coreB_test_pattern ==1)
				{//k3
					result_cmp = memcmp(img2_va[i], (unsigned char*)pgTest_img2, gTestImgSize);
				}
	            if(result_cmp == 0)
	            {
	                MY_LOGI("[coreB] queue %d compare ok", i);
	            }
	            else
	            {
	                result |= VPU_TEST_FAIL;
	                MY_LOGW("[coreB] queue %d compare failed, %d", i, result_cmp);
	            }
	        }
	        if (gDumpImage)
	        {
	          char buffer[50];
	          snprintf(buffer, 50, "/data/vpu_coreB_dst_%d_%d.raw", gettid(), i);
	          save_to_file(buffer, (char *)img2_va[i], gTestImgSize);
	        }
	    }

		original_cnt = original_cnt - (int)que_cnt;
		MY_LOGW("[coreB] original_cnt, que_cnt(%d, %d)\n", original_cnt, que_cnt);
		first_time = false;

		//
		for(i = 0 ; i < que_cnt ; i++)
	    {
	        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
	        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);

			_vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        	_vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
	    }
    }

EXIT:
    // 6. release resource
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
        MY_LOGW("[coreB] fail to free img1 ion buffer");
    for(i = 0 ; i < que_cnt ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
        if(data1_va[i] && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd[i], data1_va[i]))
            MY_LOGW("[coreB] fail to free data1 ion buffer %d", i);
        if(img2_va[i]  && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd[i], img2_va[i]))
            MY_LOGW("[coreB] fail to free img2 ion buffer for queue %d", i);
    }
    if(result == VPU_TEST_SUCCESS)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);

    return  NULL;
}

static void* _vpu_common_test_thread(void *arg)
{
    VpuStream  *vpu;
    VpuAlgo    *algo;
    VpuBuffer   buf_img1, buf_img2[MAX_MULTI_USER_QUE_CNT], buf_data1[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *request[MAX_MULTI_USER_QUE_CNT];
    int         result = VPU_TEST_SUCCESS;

    int         img1_fd,  img2_fd[MAX_MULTI_USER_QUE_CNT],  data1_fd[MAX_MULTI_USER_QUE_CNT];
    UINT8      *img1_va, *img2_va[MAX_MULTI_USER_QUE_CNT], *data1_va[MAX_MULTI_USER_QUE_CNT];
    VpuRequest *deque_req = NULL;
    UINT32 que_cnt = 2, i, j, flushed = 0;
	VpuCoreIndex coreIndex = eIndex_NONE;

    vpu = (VpuStream *)arg;
    MY_LOGD("[common] thread: user: 0x%lx", (unsigned long)vpu);
    // initialize
    img1_va  = NULL;
    img1_fd  = 0 ;
    for (i = 0 ; i < MAX_MULTI_USER_QUE_CNT ; i++)
    {
        request[i]  = NULL;
        img2_va[i]  = NULL;
        data1_va[i] = NULL;
        img2_fd[i]  = 0;
        data1_fd[i] = 0;
    }
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("[common] fail to get img1 ion buffer");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }

    // init test image data
    memcpy((void *)img1_va ,(const void *)pgTest_img1 , gTestImgSize);

    if(common_multi_user_que_cnt > MAX_MULTI_USER_QUE_CNT)
        que_cnt = MAX_MULTI_USER_QUE_CNT;
    else
        que_cnt = common_multi_user_que_cnt;

    for(i = 0 ; i < que_cnt ; i++)
    {
        if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd[i], &img2_va[i]))
        {
            MY_LOGW("[common] fail to get img2 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd[i], &data1_va[i]))
        {
            MY_LOGW("[common] fail to get data1 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
    }

	//
	algo_name = (char *)vpu_algo_name[g_common_test_pattern];

	if(common_thread_need_wait)
	{
		::sem_wait(&mSem);
	}
	else
	{
		::sem_post(&mSem);
	}

    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("[common] fail to get vpu algo handle");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
	MY_LOGD("[common] flag 1");
    // 2. get port

    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1, img1_fd, eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    for(i = 0 ; i < que_cnt ; i++)
    {
        _vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        _vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);

    }
	MY_LOGD("[common] flag 2");
    // 3. setup request & enque
    for(i = 0 ; i < que_cnt ; i++)
    {
        request[i] = vpu->acquire(algo);
		if(g_common_test_pattern == 0 || g_common_test_pattern ==1)
		{//k3
			buf_img1.port_id = 0;
	        request[i]->addBuffer(buf_img1);
			buf_img2[i].port_id = 1;
	        request[i]->addBuffer(buf_img2[i]);
			buf_data1[i].port_id = 2;
	        request[i]->addBuffer(buf_data1[i]);
	        {
				d2d_k3_property_structure_t property;
				property.param1 = 1;
				property.param2 = 2;
				property.param3 = 3;
				property.param4 = 4;
				property.param5 = 1;
				request[i]->setProperty((void*)&property, sizeof(d2d_k3_property_structure_t));
	        }
		}
        vpu->enque(request[i], coreIndex);
    }

    // 4. dequeue
    for(i = 0 ; i < que_cnt ; i++)
    {
    	#if 0 //todo..
        if(i == flush_que_cnt)
        {
            MY_LOGI("flush queue from queue cnt=%d",flush_que_cnt);
            vpu->flush();
            flushed = 1;
        }
		#endif
        deque_req = vpu->deque();
        // 5. check result
        if(deque_req != request[i])
        {
            MY_LOGW("[common] get wrong queue (deque_req = 0x%lx, request[%d] = 0x%lx)", (unsigned long)deque_req, i, (unsigned long)request[i]);
            for(j = 0 ; j < que_cnt ; j++)
            {
                if(deque_req == request[j])
                    MY_LOGW("[common] deque_req is same as queue %d", j);
            }
            if(j >= que_cnt)
                MY_LOGW("[common] deque_req is different with any request");
            result |= VPU_TEST_FAIL;
        }
        if (!flushed)
        {
            int result_cmp = 0;
			if(g_common_test_pattern == 0 || g_common_test_pattern ==1)
			{//k3
				result_cmp = memcmp(img2_va[i], (unsigned char*)pgTest_img2, gTestImgSize);
			}
            if(result_cmp == 0)
            {
                MY_LOGI("[common] queue %d compare ok", i);
            }
            else
            {
                result |= VPU_TEST_FAIL;
                MY_LOGW("[common] queue %d compare failed", i);
            }
        }
        if (gDumpImage)
        {
          char buffer[50];
          snprintf(buffer, 50, "/data/vpu_common_dst_%d_%d.raw", gettid(), i);
          save_to_file(buffer, (char *)img2_va[i], gTestImgSize);
        }
    }

EXIT:
    // 6. release resource
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
        MY_LOGW("[common] fail to free img1 ion buffer");
    for(i = 0 ; i < que_cnt ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
        if(data1_va[i] && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd[i], data1_va[i]))
            MY_LOGW("[common] fail to free data1 ion buffer %d", i);
        if(img2_va[i]  && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd[i], img2_va[i]))
            MY_LOGW("[common] fail to free img2 ion buffer for queue %d", i);
    }
    if(result == VPU_TEST_SUCCESS)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);

    return  NULL;
}



int vpu_fixcoreA_fixcoreB_test(UINT32 test_pattern, UINT32 multi_user_cnt, UINT32 que_cnt, UINT32 flush_que)
{
    int result = 0;
    void *thread_ret;

    VpuStream *vpu[MAX_MULTI_USER_CNT];
    pthread_t  vpu_thread[MAX_MULTI_USER_CNT];
    UINT32     i, j, test_user_cnt;

	sem_init(&mSem, 0, 0);

    if(multi_user_cnt > MAX_MULTI_USER_CNT)
        test_user_cnt = MAX_MULTI_USER_CNT;
    else
        test_user_cnt = multi_user_cnt;

    MY_LOGD("multi user test: user_cnt: %d, que_cnt %d", test_user_cnt, que_cnt);

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
	g_coreA_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
	g_coreB_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        return result;
    }

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        // 1. init vpu
        vpu[i] = VpuStream::createInstance("vpu_fixcoreA_fixcoreB_test");
        if (vpu[i] == NULL)
        {
            MY_LOGW("fail to get vpu handle");
            for(j = 0 ; j < i ; j++)
                delete vpu[j];
            return result;
        }
    }

    coreA_multi_user_que_cnt = que_cnt;
	coreB_multi_user_que_cnt = que_cnt;
    coreA_flush_que_cnt      = flush_que;
	coreB_flush_que_cnt      = flush_que;

	coreA_thread_need_wait = false;
	coreB_thread_need_wait = false;

	g_non_stop = false;
	g_time_exec = 0;
	g_time_sleep = 0;

	//coreA
	MY_LOGD("coreA creat thread: user: 0x%lx", (unsigned long)vpu[0]);
	pthread_create(&vpu_thread[0], NULL, _vpu_coreA_test_thread, (void *)vpu[0]);

	//coreB
	MY_LOGD("coreB creat thread: user: 0x%lx", (unsigned long)vpu[1]);
	pthread_create(&vpu_thread[1], NULL, _vpu_coreB_test_thread, (void *)vpu[1]);

    result = VPU_TEST_SUCCESS;
    for(i = 0 ; i < test_user_cnt ; i++)
    {
        pthread_join(vpu_thread[i], &thread_ret);
        if(thread_ret == 0)
        {
            MY_LOGW("vpu multi-thread test fail (thread = %d, result=0x%lx)", i, (unsigned long)thread_ret);
            result |= VPU_TEST_FAIL;
        }
        delete vpu[i];
    }
    return result;
}

int vpu_fixcoreA_nospecify_test(UINT32 test_pattern, UINT32 multi_user_cnt, UINT32 que_cnt_1, UINT32 que_cnt_2)
{
    int result = 0;
    void *thread_ret;

    VpuStream *vpu[MAX_MULTI_USER_CNT];
    pthread_t  vpu_thread[MAX_MULTI_USER_CNT];
    UINT32     i, j, test_user_cnt;

	sem_init(&mSem, 0, 0);

    if(multi_user_cnt > MAX_MULTI_USER_CNT)
        test_user_cnt = MAX_MULTI_USER_CNT;
    else
        test_user_cnt = multi_user_cnt;

    MY_LOGD("multi user test: user_cnt: %d, que_cnt %d/%d", test_user_cnt, que_cnt_1, que_cnt_2);

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
	g_coreA_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
	g_common_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        return result;
    }

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        // 1. init vpu
        vpu[i] = VpuStream::createInstance("vpu_fixcoreA_nospecify_test");
        if (vpu[i] == NULL)
        {
            MY_LOGW("fail to get vpu handle");
            for(j = 0 ; j < i ; j++)
                delete vpu[j];
            return result;
        }
    }

    coreA_multi_user_que_cnt = que_cnt_1;
    common_multi_user_que_cnt = que_cnt_2;
	coreA_thread_need_wait = false;
	common_thread_need_wait = false;

	if(coreA_multi_user_que_cnt > common_multi_user_que_cnt)
	{
		coreA_thread_need_wait = false;
		common_thread_need_wait = true;
	}
	else if (coreA_multi_user_que_cnt < common_multi_user_que_cnt)
	{
		coreA_thread_need_wait = true;
		common_thread_need_wait = false;
	}
	g_non_stop = false;
	g_time_exec = 0;
	g_time_sleep = 0;

	//coreA
	MY_LOGD("coreA creat thread: user: 0x%lx", (unsigned long)vpu[0]);
	pthread_create(&vpu_thread[0], NULL, _vpu_coreA_test_thread, (void *)vpu[0]);

	//common
	MY_LOGD("common creat thread: user: 0x%lx", (unsigned long)vpu[1]);
	pthread_create(&vpu_thread[1], NULL, _vpu_common_test_thread, (void *)vpu[1]);

    result = VPU_TEST_SUCCESS;
    for(i = 0 ; i < test_user_cnt ; i++)
    {
        pthread_join(vpu_thread[i], &thread_ret);
        if(thread_ret == 0)
        {
            MY_LOGW("vpu multi-thread test fail (thread = %d, result=0x%lx)", i, (unsigned long)thread_ret);
            result |= VPU_TEST_FAIL;
        }
        delete vpu[i];
    }
    return result;
}

int vpu_Imax_test(UINT32 test_pattern, UINT32 maxCoreNum, UINT32 que_cnt, bool non_stop, int time_exec, int time_sleep)
{
    int result = 0;
    void *thread_ret;
	g_non_stop = false;

    VpuStream *vpu[MAX_MULTI_USER_CNT];
    pthread_t  vpu_thread[MAX_MULTI_USER_CNT];
    UINT32     i, j, test_user_cnt;

	sem_init(&mSem, 0, 0);

    if(maxCoreNum > MAX_MULTI_USER_CNT)
        test_user_cnt = MAX_MULTI_USER_CNT;
    else
        test_user_cnt = maxCoreNum;

    MY_LOGD("multi user test: user_cnt: %d, que_cnt %d", test_user_cnt, que_cnt);

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
	g_coreA_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
	g_coreB_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        return result;
    }

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        // 1. init vpu
        vpu[i] = VpuStream::createInstance("vpu_Imax_test");
        if (vpu[i] == NULL)
        {
            MY_LOGW("fail to get vpu handle");
            for(j = 0 ; j < i ; j++)
                delete vpu[j];
            return result;
        }
    }

    coreA_multi_user_que_cnt = que_cnt;
    coreA_flush_que_cnt      = 0;
	coreA_thread_need_wait = false;
	if(test_user_cnt>1)
	{
		coreB_multi_user_que_cnt = que_cnt;
		coreB_flush_que_cnt      = 0;
		coreB_thread_need_wait = false;
	}
	if(test_user_cnt>2)
	{ //add coreC
	}

	g_non_stop = non_stop;
	g_time_exec = time_exec;
	g_time_sleep = time_sleep;
	gDumpImage = 0;
	if(g_non_stop)
	{
		coreA_multi_user_que_cnt = 1;
		coreB_multi_user_que_cnt = 1;
	}

	//coreA
	MY_LOGD("coreA creat thread: user: 0x%lx", (unsigned long)vpu[0]);
	pthread_create(&vpu_thread[0], NULL, _vpu_coreA_test_thread, (void *)vpu[0]);

	//coreB
	if(test_user_cnt>1)
	{
		MY_LOGD("coreB creat thread: user: 0x%lx", (unsigned long)vpu[1]);
		pthread_create(&vpu_thread[1], NULL, _vpu_coreB_test_thread, (void *)vpu[1]);
	}

    result = VPU_TEST_SUCCESS;
    for(i = 0 ; i < test_user_cnt ; i++)
    {
        pthread_join(vpu_thread[i], &thread_ret);
        if(thread_ret == 0)
        {
            MY_LOGW("vpu multi-thread test fail (thread = %d, result=0x%lx)", i, (unsigned long)thread_ret);
            result |= VPU_TEST_FAIL;
        }
        delete vpu[i];
    }
    return result;
}

int vpu_voltage_freq_test(UINT32 test_pattern, UINT32 maxCoreNum, UINT32 que_cnt, bool random, int time_exec, int time_sleep)
{
    int result = 0;
    void *thread_ret;
	g_non_stop = false;

    VpuStream *vpu[MAX_MULTI_USER_CNT];
    pthread_t  vpu_thread[MAX_MULTI_USER_CNT];
    UINT32     i, j, test_user_cnt;

	sem_init(&mSem, 0, 0);

    if(maxCoreNum > MAX_MULTI_USER_CNT)
        test_user_cnt = MAX_MULTI_USER_CNT;
    else
        test_user_cnt = maxCoreNum;

    MY_LOGD("multi user test: user_cnt: %d, que_cnt %d", test_user_cnt, que_cnt);

    // select test pattern
    _vpu_setup_test_pattern(test_pattern, &pgTest_img1, &pgTest_img2, &pgTest_data1);
	g_coreA_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
	g_coreB_test_pattern = static_cast<VPU_ALGO_E>(test_pattern);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        return result;
    }

    for(i = 0 ; i < test_user_cnt ; i++)
    {
        // 1. init vpu
        vpu[i] = VpuStream::createInstance("vpu_voltage_freq_test");
        if (vpu[i] == NULL)
        {
            MY_LOGW("fail to get vpu handle");
            for(j = 0 ; j < i ; j++)
                delete vpu[j];
            return result;
        }
    }

    coreA_multi_user_que_cnt = que_cnt;
    coreA_flush_que_cnt      = 0;
	coreA_thread_need_wait = false;
	if(test_user_cnt>1)
	{
		coreB_multi_user_que_cnt = que_cnt;
		coreB_flush_que_cnt      = 0;
		coreB_thread_need_wait = false;
	}
	if(test_user_cnt>2)
	{ //add coreC
	}

	g_non_stop = false;
	g_time_exec = time_exec;
	g_time_sleep = time_sleep;
	gDumpImage = 0;
	if(g_non_stop)
	{
		coreA_multi_user_que_cnt = 1;
		coreB_multi_user_que_cnt = 1;
	}

	coreA_adjust_opp=true;
	coreB_adjust_opp=true;
	coreA_random_opp=random;
	coreB_random_opp=random;

	//coreA
	MY_LOGD("coreA creat thread: user: 0x%lx", (unsigned long)vpu[0]);
	pthread_create(&vpu_thread[0], NULL, _vpu_coreA_test_thread, (void *)vpu[0]);

	//coreB
	if(test_user_cnt>1)
	{
		MY_LOGD("coreB creat thread: user: 0x%lx", (unsigned long)vpu[1]);
		pthread_create(&vpu_thread[1], NULL, _vpu_coreB_test_thread, (void *)vpu[1]);
	}

    result = VPU_TEST_SUCCESS;
    for(i = 0 ; i < test_user_cnt ; i++)
    {
        pthread_join(vpu_thread[i], &thread_ret);
        if(thread_ret == 0)
        {
            MY_LOGW("vpu multi-thread test fail (thread = %d, result=0x%lx)", i, (unsigned long)thread_ret);
            result |= VPU_TEST_FAIL;
        }
        delete vpu[i];
    }
    return result;
}


void vpu_cleartime(void)
{

   int i = 0;
   for (i=0;i<12;i++)
   	elapse_time[i] = 0;

}

#define TEST_WIDTH 256
#define TEST_HEIGHT 256

void vpu_performance_test (INT64 totalLength)
{
#if defined(__ANDROID__) || defined(ANDROID)
    //UTIL_TRACE_CALL();

    VpuStream *prVPU = NULL;
    VpuAlgo *prAlgo = NULL;
    VpuRequest *prRequest = NULL;
    VpuRequest *prDequeReq = NULL;
    VpuBuffer rInBuffer, rOutBuffer;
	int i4In1FD = NULL,i4OutFD = NULL;
	int i4PadWidth = TEST_WIDTH, i4PadHeight = TEST_HEIGHT;
    UINT8  *prVaIn1 = NULL, *prVaOut1 = NULL;
	int ion_total_szie = totalLength;

    if (totalLength > (i4PadWidth*i4PadHeight))
    {
      i4PadHeight = (totalLength/i4PadWidth)+1;
	}
	else
		ion_total_szie = i4PadWidth * i4PadHeight;

	LOGE("vpu_performance_test ion_total_szie = \n", ion_total_szie);


    if(!_vpu_get_buffer(ion_drv_handle, ion_total_szie, &i4In1FD, &prVaIn1))
    {
        MY_LOGW("[common] fail to get img1 ion buffer");
        return;
    }
    memset(prVaIn1, 0x01, ion_total_szie*sizeof(UINT8));


    // flush cache
    //ion_cache_sync_flush_all(ion_drv_handle);
    _vpu_sync_buffer(ion_drv_handle, ion_total_szie, i4In1FD, prVaIn1); // new api

    vpu_cleartime();

    // 1. init_vpu
    MY_GET_TIME(&sub_start_time, NULL);
    prVPU = VpuStream::createInstance();
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);
    if (prVPU == NULL)
    {
        LOGE("[%s] fail to get vpu handle\n", LOG_TAG);
        goto EXIT;
    }

#if 0
    // force vpu power on
    // This control can force VPU power-on before using VPU so that it can reduce VPU boot up time.
    VpuPower pwr;
    pwr.mode = ePowerModeOn; //always on
    pwr.opp  = 0x0; // Max freq.

    if (!prVPU->setPower(pwr, eIndex_A))
    {
        LOGE("[%s] fail to force VPU on\n", LOG_TAG);
        goto EXIT;
    }
#endif

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
        LOGE("[%s] Start %s\n", LOG_TAG, "Algo1");
    //prAlgo = prVPU->getAlgo("vpu_lib_d2d_ksample");
		prAlgo = prVPU->getAlgo("vpu_lib_jack_ksample");
        LOGE("[%s] Start %s, load vpu_lib_jack_ksample \n", LOG_TAG, "Algo2");
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);
    if (prAlgo == NULL)
    {
		LOGE("[%s] fail to get vpu algo handle %s, re-loading !!\n", LOG_TAG, "vpu_lib_jack_ksample");
        goto EXIT;

    }

    // 1.2 create request
    MY_GET_TIME(&sub_start_time, NULL);
    prRequest = prVPU->acquire(prAlgo);
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);
    if (prRequest == NULL)
    {
        LOGE("[%s] fail to get vpu algo request \n", LOG_TAG);
        goto EXIT;
    }


    // 2. get/set  in/out port info
    MY_GET_TIME(&sub_start_time, NULL);
    //input1
    //     buf: pad_w*pad_h
    //     src: pad_w*pad_h offset: 0
    rInBuffer.planeCount = 1;
    rInBuffer.planes[0].fd = i4In1FD;
    rInBuffer.planes[0].offset = 0;
    rInBuffer.planes[0].stride = i4PadWidth*sizeof(UINT8);
    rInBuffer.planes[0].length = i4PadWidth*i4PadHeight*sizeof(UINT8);
    rInBuffer.format = eFormatData;
    rInBuffer.width = i4PadWidth;
    rInBuffer.height = i4PadHeight;
    rInBuffer.port_id = 0; // please check with port_prop_descs of algo. header
    if (prRequest->addBuffer(rInBuffer) == false)
    {
    	  LOGE("[%s] fail to add input buffer \n", LOG_TAG);
        goto EXIT;
    }

    //output
    //     buf: pad_w*pad_h
    //     src: pad_w*pad_h offset: 0
    if (i4OutFD != NULL)
    {
	    rOutBuffer.planeCount = 1;
	    rOutBuffer.planes[0].fd = i4OutFD;
	    rOutBuffer.planes[0].offset = 0;
	    rOutBuffer.planes[0].stride = i4PadWidth*sizeof(UINT8);
	    rOutBuffer.planes[0].length = i4PadWidth*i4PadHeight*sizeof(UINT8);
	    rOutBuffer.format = eFormatData;
	    rOutBuffer.width = i4PadWidth;
	    rOutBuffer.height = i4PadHeight;
	    rOutBuffer.port_id = 1; // please check with port_prop_descs of algo. header
	    if (prRequest->addBuffer(rOutBuffer) == false)
	    {
	    	  LOGE("[%s] fail to add output buffer \n", LOG_TAG);
	        goto EXIT;
	    }

    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    // 3.  set Property
    MY_GET_TIME(&sub_start_time, NULL);

    vpu_st_sett_ksample rAlgoParam; // please check with algo. header
    rAlgoParam.S32_Wight = i4PadWidth;
    rAlgoParam.S32_Height = i4PadHeight;
	rAlgoParam.S32_Op = 0x77;
	rAlgoParam.U64_TotalLength = totalLength;
    if (prRequest->setProperty((void *)&rAlgoParam, sizeof(vpu_st_sett_ksample)) == false)
    {
    	  LOGE("[%s] fail to set property \n", LOG_TAG);
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

#if 0
    // this request prefer VPU freq. and bandwidth setting
    VpuExtraParam rParam;
    rParam.opp_step = 0;
    rParam.bw = 2500; // it means this request need 2500MB/s bandwidth
    if (prRequest->setExtraParam(rParam) == FALSE)
    {
    	  LOGE("[%s] fail to set extra parameter %s\n", LOG_TAG);
        goto EXIT;
    }
#endif

    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);
    if ( prVPU->enque(prRequest) == false)
    {
    	  LOGE("[%s] fail to enque\n", LOG_TAG);
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[6], temp_time);

	MY_DISPLAY_TIME(elapse_time[6],"jack enque process time ...");

    MY_GET_TIME(&sub_start_time, NULL);
    prDequeReq = prVPU->deque();
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);

	MY_DISPLAY_TIME(elapse_time[7],"jack deque process time... ");

    if (prDequeReq == NULL)
    {
    	  LOGE("[%s] fail to deque \n", LOG_TAG);
        goto EXIT;
    }

    // 5. check result
    if(prDequeReq != prRequest)
    {
        LOGE("[%s] Fail: get wrong deque request (req=%p, deque_req=%p) \n", LOG_TAG, prRequest, prDequeReq);
    }

#if 0
    // disable force VPU power on mode
    VpuPower pwr;
    pwr.mode = ePowerModeDynamic; //dynamically control by driver
    pwr.opp  = ePowerOppUnrequest; // default freq. (min)

    if (!prVPU->setPower(pwr, eIndex_A))
    {
        LOGE("[%s] fail to default power mode %s\n", LOG_TAG);
        goto EXIT;
    }
#endif

EXIT:
    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    if (prRequest)
    {
        prVPU->release(prRequest);
    }

    if (prVPU != NULL)
    {
        delete prVPU;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);


#endif
}

void vpu_test_menu()
{
    MY_LOGD("##############################");
    MY_LOGD("1: single core enque test");
    MY_LOGD("2: multi-user test (legacy, not maintain)");
    MY_LOGD("3: flush queue test(legacy, not maintain)");
    MY_LOGD("4: Imax test");
	MY_LOGD("5: multi-core(fix_coreA + fix_coreB) test");
	MY_LOGD("6: multi-core(fix_coreA + no_specify) test");
	MY_LOGD("7: performance test");
    MY_LOGD("9: test exit");
    MY_LOGD("##############################");
    MY_LOGD("select item:");
}

int vpu_load_file()
{
    FILE *fpSrc = NULL, *fpGolden = NULL, *fpParam = NULL;
    int   sizeSrc, sizeGolden, sizeParam, ret = 1, rdSize;
    MY_LOGD("load file: %s, %s, %s", gpSrcName, gpSrcParam, gpGoldenName);

#define VPU_OPEN_FILE(fp, filename)                       \
    if ((fp = fopen(filename, "rb")) == (FILE *)NULL)     \
    {                                                     \
        MY_LOGE("fail to open file : %s", filename);      \
        goto EXIT;                                        \
    }
    VPU_OPEN_FILE(fpSrc,    gpSrcName);
    VPU_OPEN_FILE(fpGolden, gpGoldenName);
    VPU_OPEN_FILE(fpParam,  gpSrcParam);
#undef VPU_OPEN_FILE

#define VPU_GET_SIZE(fp, size)    \
    fseek (fp, 0, SEEK_END);      \
    size = ftell(fp);             \
    rewind (fp);

    VPU_GET_SIZE(fpSrc,    sizeSrc);
    VPU_GET_SIZE(fpGolden, sizeGolden);
    VPU_GET_SIZE(fpParam,  sizeParam);
#undef VPU_GET_SIZE

    MY_LOGD("sizeSrc %d, sizeGolden %d, sizeParam %d", sizeSrc, sizeGolden, sizeParam);

#define VPU_ALLOC_BUF(pData, size)                          \
    if ((pData = (unsigned char *)malloc(size)) == NULL)    \
    {                                                       \
        MY_LOGE("fail to alloc buffer size : %d", size);    \
        goto EXIT;                                          \
    }

    VPU_ALLOC_BUF(g_datasrc_wpp,        sizeSrc);
    VPU_ALLOC_BUF(g_datadst_golden_wpp, sizeGolden);
    VPU_ALLOC_BUF(g_procParam,          sizeParam);
#undef VPU_ALLOC_BUF

#define VPU_READ_DATA(pData, size, fp)                                  \
    rdSize = fread(pData, 1, size, fp);                                 \
    if (rdSize != size)                                                 \
    {                                                                   \
        MY_LOGE("fail to read size: %d, file size: %d", rdSize, size);  \
        goto EXIT;                                                      \
    }

    VPU_READ_DATA(g_datasrc_wpp,        sizeSrc,    fpSrc);
    VPU_READ_DATA(g_datadst_golden_wpp, sizeGolden, fpGolden);
    VPU_READ_DATA(g_procParam,          sizeParam,  fpParam);
#undef VPU_READ_DATA

    ret = 0;

    gTestImgSize    = sizeSrc;
    gTestImgWidth   = TEST_IMG_WIDTH0;
    gTestImgHeight  = gTestImgSize/TEST_IMG_WIDTH0;
    gTestDataLength = sizeParam;

EXIT:
    if (fpSrc)    fclose(fpSrc);
    if (fpGolden) fclose(fpGolden);
    if (fpParam)  fclose(fpParam);
    if (ret)
    {
        if (g_datasrc_wpp)        free((void *) g_datasrc_wpp);
        if (g_datadst_golden_wpp) free((void *) g_datadst_golden_wpp);
        if (g_procParam)          free((void *) g_procParam);
        g_datasrc_wpp        = NULL;
        g_datadst_golden_wpp = NULL;
        g_procParam          = NULL;
    }
    return ret;
}

int vpu_hqa_maxPower_test(int time_exec, int time_sleep, int loop)
{
    int result, i, enq_count = 0, deq_count = 0, cnt = 0;
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    VpuRequest *request[AUTO_TEST_PW_PRE_ENQ_CNT];
    timeval t0, t1, t2;
    MY_LOGD("Start to test maxPower: time_exec: %d (secs), time_sleep %d (secs), loop: %d", time_exec, time_sleep, loop);
    result = VPU_TEST_FAIL;

	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vpu.hqapdn_test_para.en", value, "0");
    int vup_hqapdn_test_para_en=atoi(value);



    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        request[i]  = NULL;
    }
    gettimeofday(&t0,NULL);
    // 1. init vpu
    vpu = VpuStream::createInstance("vpu_hqa_maxPower_test");
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }
	{
        VpuPower pwr;
        pwr.mode = ePowerModeOn;
        pwr.opp  = ePowerOppUnrequest;
        if (!vpu->setPower(pwr))
        {
            MY_LOGW("fail to setPower!!");
            result = VPU_TEST_FAIL;
            goto EXIT;
        }
    }
    // 1.1 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    // 2. setup request
    // 2.1 acquire request
    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        if ((request[i] = vpu->acquire(algo)) == NULL)
        {
            MY_LOGW("fail to get vpu request[%d]", i);
            goto EXIT;
        }
    }

    while(cnt < loop)
    {
      enq_count = 0;
      deq_count = 0;
      gettimeofday(&t1,NULL);
      // 3. enqueue & dequeue
      for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
      {
        if (vpu->enque(request[i])) enq_count++;
      }
      while(1)
      {
          VpuRequest *deque_req;
          deque_req = vpu->deque();
          if (!deque_req)
          {
          	MY_LOGE("[Vpu] MAXPower deque fail, loop cnt = %d, deq_count %d\n", cnt, deq_count);
            goto EXIT;
          }
          deq_count++;
		  MY_LOGD("[Vpu] keep doing, loop cnt = %d, deq_count %d\n", cnt, deq_count);
          if (vpu->enque(deque_req))
            enq_count++;

		  if(!vup_hqapdn_test_para_en)
		  {
          	gettimeofday(&t2,NULL);
          	if ((int)(t2.tv_sec - t1.tv_sec) >= time_exec) break;
		  }
      }

      while(deq_count != enq_count)
      {
          VpuRequest *deque_req;
          deque_req = vpu->deque();
          if (!deque_req)
          {
            break;
          }
          deq_count++;
      }
      cnt++;
      gettimeofday(&t2,NULL);
      MY_LOGD("elapsed time = %d (secs), loop cnt = %d, deq_count %d\n", (int)(t2.tv_sec - t0.tv_sec), cnt, deq_count);
      sleep(time_sleep);
    }
    result = VPU_TEST_SUCCESS;

EXIT:
    // 6. release resource & exit
    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
      if (request[i])
          vpu->release(request[i]);
    }
    if (vpu)
        delete vpu;
    MY_LOGI("test finish");
    gettimeofday(&t2,NULL);
    MY_LOGD("elapsed time = %d (secs), loop cnt = %d\n", (int)(t2.tv_sec - t0.tv_sec), cnt);
  return result;
}
int vpu_hqa_warping_test(int time_exec, int time_sleep, int loop)
{
    int result, i, j, enq_count = 0, deq_count = 0, cnt = 0;
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2[AUTO_TEST_PW_PRE_ENQ_CNT], buf_data1[AUTO_TEST_PW_PRE_ENQ_CNT];
    int         img1_fd,  img2_fd[AUTO_TEST_PW_PRE_ENQ_CNT],  data1_fd[AUTO_TEST_PW_PRE_ENQ_CNT];
    UINT8      *img1_va, *img2_va[AUTO_TEST_PW_PRE_ENQ_CNT], *data1_va[AUTO_TEST_PW_PRE_ENQ_CNT];
    VpuRequest *request[AUTO_TEST_PW_PRE_ENQ_CNT];
    timeval t0, t1, t2;
    MY_LOGD("Start to test Warping: time_exec: %d (secs), time_sleep %d (secs), loop: %d", time_exec, time_sleep, loop);
    result = 0;

    gettimeofday(&t0,NULL);

    // initialize
    img1_va  = NULL;
    img1_fd  = 0 ;
    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        request[i]  = NULL;
        img2_va[i]  = NULL;
        data1_va[i] = NULL;
        img2_fd[i]  = 0;
        data1_fd[i] = 0;
    }
    // select test pattern
    _vpu_setup_test_pattern(0, &pgTest_img1, &pgTest_img2, &pgTest_data1);
    if (pgTest_img1 == NULL)
    {
        MY_LOGW("fail to load test image");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
    // 1. init vpu
    vpu = VpuStream::createInstance("vpu_hqa_warping_test");
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
	{
        VpuPower pwr;
        pwr.mode = ePowerModeOn;
        pwr.opp  = ePowerOppUnrequest;
        if (!vpu->setPower(pwr))
        {
            MY_LOGW("fail to setPower!!");
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
    }
    // 1.1 init test image data
    if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img1_fd, &img1_va))
    {
        MY_LOGW("fail to get img1 ion buffer");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
    memcpy((void *)img1_va ,(const void *)pgTest_img1 , gTestImgSize);
    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        if(!_vpu_get_buffer(ion_drv_handle, gTestImgSize, &img2_fd[i], &img2_va[i]))
        {
            MY_LOGW("fail to get img2 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        if(!_vpu_get_buffer(ion_drv_handle, gTestDataLength, &data1_fd[i], &data1_va[i]))
        {
            MY_LOGW("fail to get data1 ion buffer for que%d", i);
            result |= VPU_TEST_FAIL;
            goto EXIT;
        }
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
    }
    // 1.2 init algo of vpu
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }
    // 2. get port
    port1 = algo->getPort(port1_name);
    port2 = algo->getPort(port2_name);
    port3 = algo->getPort(port3_name);
    if (!port1 || !port2 || !port3)
    {
        MY_LOGW("port1 0x%lx, port2 0x%lx, port3 0x%lx", (unsigned long)port1, (unsigned long)port2, (unsigned long)port3);
        MY_LOGW("some Port is NULL");
        result |= VPU_TEST_FAIL;
        goto EXIT;
    }
    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1, img1_fd, eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    for(i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        _vpu_setup_buf(buf_data1[i], data1_fd[i], eFormatData, 0, 0, 0, 0, gTestDataLength);
        _vpu_setup_buf(buf_img2[i] , img2_fd[i] , eFormatImageY8, gTestImgWidth, gTestImgHeight, 0, gTestImgWidth, gTestImgSize);
    }
    // 2. setup request
    // 2.1 acquire request
    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        if ((request[i] = vpu->acquire(algo)) == NULL)
        {
            MY_LOGW("fail to get vpu request[%d]", i);
            goto EXIT;
        }
        request[i]->addBuffer(port1, buf_img1);
        request[i]->addBuffer(port2, buf_img2[i]);
        request[i]->addBuffer(port3, buf_data1[i]);
        {
            int param1 = 1;
            int param2 = 2;
            int param3 = 3;
            int param4 = 4;

            VpuProperty prop;
            prop.type = eTypeInt32;
            prop.count = 1;

            prop.data.i32 = &param1;
            request[i]->setProperty("param1", prop);

            prop.data.i32 = &param2;
            request[i]->setProperty("param2", prop);

            prop.data.i32 = &param3;
            request[i]->setProperty("param3", prop);

            prop.data.i32 = &param4;
            request[i]->setProperty("param4", prop);
        }
    }
    while(cnt < loop)
    {
      enq_count = 0;
      deq_count = 0;
      gettimeofday(&t1,NULL);

      // 3. enqueue & dequeue
      for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
      {
        memset((void *)img2_va[i], 0, gTestImgSize);        // clean output image buffer
        memcpy((void *)data1_va[i],(const void *)pgTest_data1, gTestDataLength);
        if (vpu->enque(request[i])) enq_count++;
      }
      while(1)
      {
          VpuRequest *deque_req;
          deque_req = vpu->deque();
          j = deq_count%AUTO_TEST_PW_PRE_ENQ_CNT;
          deq_count++;
          if (!deque_req || deque_req != request[j])
          {
            MY_LOGW("fail to dequeue deque_req(0x%lx) != request[%d] (0x%lx)", (unsigned long)deque_req, j, (unsigned long)request[j]);
            goto EXIT;
          }
          int result_cmp = memcmp(img2_va[j], (unsigned char*)pgTest_img2, gTestImgSize);
          if(result_cmp == 0)
          {
              MY_LOGI("queue %d compare ok", deq_count);
          }
          else
          {
              result |= VPU_TEST_FAIL;
              MY_LOGW("queue %d compare failed", deq_count);
              goto EXIT;
          }
          memset((void *)img2_va[j], 0, gTestImgSize);        // clean output image buffer
          memcpy((void *)data1_va[j],(const void *)pgTest_data1, gTestDataLength);

          if (vpu->enque(deque_req))
            enq_count++;

          gettimeofday(&t2,NULL);
          if ((int)(t2.tv_sec - t1.tv_sec) >= time_exec) break;
      }

      while(deq_count != enq_count)
      {
          VpuRequest *deque_req;
          deque_req = vpu->deque();
          j = deq_count%AUTO_TEST_PW_PRE_ENQ_CNT;
          deq_count++;
          if (!deque_req || deque_req != request[j])
          {
            MY_LOGW("fail to dequeue deque_req(0x%lx) != request[%d] (0x%lx)", (unsigned long)deque_req, j, (unsigned long)request[j]);
            goto EXIT;
          }
          int result_cmp = memcmp(img2_va[j], (unsigned char*)pgTest_img2, gTestImgSize);
          if(result_cmp == 0)
          {
              MY_LOGI("queue %d compare ok", deq_count);
          }
          else
          {
              result |= VPU_TEST_FAIL;
              MY_LOGW("queue %d compare failed", deq_count);
              goto EXIT;
          }
      }
      cnt++;
      gettimeofday(&t2,NULL);
      MY_LOGD("elapsed time = %d (secs), loop cnt = %d, deq_count %d\n", (int)(t2.tv_sec - t0.tv_sec), cnt, deq_count);
      sleep(time_sleep);
    }
    result = VPU_TEST_SUCCESS;

EXIT:
    while(deq_count != enq_count)
    {
       vpu->deque();
       deq_count++;
    }
    // 6. release resource & exit
    if(img1_va && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img1_fd, img1_va))
    {
        MY_LOGW("fail to free img1 ion buffer");
    }
    for(i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
        if(data1_va[i] && !_vpu_free_buffer(ion_drv_handle, gTestDataLength, data1_fd[i], data1_va[i]))
            MY_LOGW("fail to free data1 ion buffer %d", i);
        if(img2_va[i]  && !_vpu_free_buffer(ion_drv_handle, gTestImgSize, img2_fd[i], img2_va[i]))
            MY_LOGW("fail to free img2 ion buffer for queue %d", i);
    }
    if (vpu)
        delete vpu;
    MY_LOGI("test finish");
    gettimeofday(&t2,NULL);
    MY_LOGD("elapsed time = %d (secs), loop cnt = %d\n", (int)(t2.tv_sec - t0.tv_sec), cnt);
  return result;
}
/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0, bLoad = 1;
    UINT32 testcase = 0, test_pattern = 0, test_cnt = 0, user_cnt = 2, queue_cnt =2, queue_cnt_2 =2, flush_que=0;
    UINT32 input;
    UINT32 enter_test = 1;
    int autoTestCase  = 0;
    VPU_ALGO_E test_algo = E_VPU_ALGO_WARP;
	UINT32 coreIndex = 0;

	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.vpu.hqapdn_test_para.en", value, "0");
    int vup_hqapdn_test_para_en=atoi(value);

	char special_case_value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.vpu.special_test_en.case_num", special_case_value, "0");
    int vup_special_test_en_case=atoi(special_case_value);

	MY_LOGD("%d/%d", vup_hqapdn_test_para_en, vup_special_test_en_case);
	if((vup_hqapdn_test_para_en) || (vup_special_test_en_case>0))
	{
		autoTestCase = 0;
	}
	else
	{
	    if (argc < 5)
	    {
	        MY_LOGD("Usage: vpu_test <testAlgo> <testCase> <testPattern> <dumpImage>");
	        MY_LOGD("<testAlgo> 0: don't care, 1: D2D warping");
	        MY_LOGD("<testCase> 0: test manu, 1:enque test, 2: multi-user test, 3:flush queue test");
	        MY_LOGD("1: enque test:      auto d2d_test, queue count: %d ", AUTO_TEST_CNT);
	        MY_LOGD("2: multi-user test: auto multi_user d2d_test, user count: %d, queue count: %d, flush count: %d", AUTO_TEST_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_FLUSH_CNT);
	        MY_LOGD("3: flush queu test: auto multi_user d2d_test, user count: %d, queue count: %d, flush count: %d", AUTO_TEST_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_FLUSH_CNT);
	        autoTestCase = 0;
	    }
	    else
	    {
	        test_algo    = (VPU_ALGO_E)atoi(argv[1]);
	        autoTestCase = atoi(argv[2]);
	        test_pattern = atoi(argv[3]);
	        gDumpImage   = atoi(argv[4]);
	    }
	}

    if (vpu_load_file())
    {
        MY_LOGW("fail to load test file, load from header file !!");
        // load from header file
        gTestImgSize    = sizeof(g_datasrc_640x360_wpp);
        gTestImgWidth   = TEST_IMG_WIDTH1;
        gTestImgHeight  = gTestImgSize/TEST_IMG_WIDTH1;
        gTestDataLength = sizeof(g_datapp_640x360_wpp);

        g_datasrc_wpp        = (unsigned char *)g_datasrc_640x360_wpp;
        g_datadst_golden_wpp = (unsigned char *)g_datadst_640x360_golden_wpp;
        g_procParam          = (unsigned char *)g_datapp_640x360_wpp;
        bLoad = 0;
        MY_LOGD("sizeSrc 0x%lx, sizeGolden 0x%lx, sizeParam 0x%lx", sizeof(g_datasrc_640x360_wpp), sizeof(g_datadst_640x360_golden_wpp), sizeof(g_datapp_640x360_wpp));
    }

    ion_drv_handle = mt_ion_open(__FILE__);
    if (!ion_drv_handle)
    {
      MY_LOGW("fail to get ion driver handle");
      goto EXIT;
    }

    if (test_algo >= E_VPU_ALGO_MAX)
    {
      MY_LOGW("number of vpu algo(%d) is larger than max(%d)", test_algo, E_VPU_ALGO_MAX);
      goto EXIT;
    }

    algo_name   = (char *)vpu_algo_name[test_algo];
    MY_LOGD("test case: %d, test pattern %d, gDumpImage %d, algo_name %s", autoTestCase, test_pattern, gDumpImage, algo_name);

    if (test_algo == E_VPU_ALGO_SHIFT)
    {
      vpu_slt_test(AUTO_TEST_CNT);
      goto EXIT;
    }

	coreA_adjust_opp = false;
	coreB_adjust_opp = false;
    switch(autoTestCase)
    {
        case 1:
            ret = vpu_enque_test(test_pattern, AUTO_TEST_CNT, 0);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("VPU Test PASS !!!");
            else
                MY_LOGD("VPU Test FAIL !!!");
            break;
        case 2:
            ret = vpu_multi_user_test(test_pattern, AUTO_TEST_USER_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_QUEUE_CNT);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("VPU Test PASS !!!");
            else
                MY_LOGD("VPU Test FAIL !!!");
            break;
        case 3:
            ret = vpu_multi_user_test(test_pattern, AUTO_TEST_USER_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_FLUSH_CNT);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("VPU Test PASS !!!");
            else
                MY_LOGD("VPU Test FAIL !!!");
            break;
        case 0:
			if(vup_hqapdn_test_para_en)
			{
				if (argc < 7)
	    		{
	    			MY_LOGE("need more parameters..., current(%d)",argc);
				}
				else
				{
					int time_exec, time_sleep, coreNum, loop, non_stop;
					test_algo = (VPU_ALGO_E)atoi(argv[1]);
					algo_name = (char *)vpu_algo_name[test_algo];
					coreNum = atoi(argv[2]);
					loop = atoi(argv[3]);
					non_stop = atoi(argv[4]);
					time_exec = atoi(argv[5]);
			        time_sleep = atoi(argv[6]);
					MY_LOGI("parameters, test_algo(%d_%s)/coreNum(%d/%d/%d)/time_exec(%d)\time_sleep(%d)",\
						test_algo,algo_name,coreNum,loop,non_stop,time_exec,time_sleep);
					ret = vpu_Imax_test((int)test_algo, coreNum, loop, non_stop, time_exec, time_sleep);
					if(ret == VPU_TEST_SUCCESS)
						MY_LOGD("VPU Test PASS !!!");
					else
						MY_LOGD("VPU Test FAIL !!!");
					break;

				}
			}
			else if (vup_special_test_en_case > 0)
			{
				int time_exec, time_sleep, coreNum, loop, non_stop;
				bool random=false;
				switch(vup_special_test_en_case)
				{
					default:
						MY_LOGE("wrong value(%d)", vup_special_test_en_case);
						break;
					/* random voltage/freq adjustment test, fix pattern */
					/* vpu_test64 coreNum loop */
					case 1:
						algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
						if (argc < 4)
			    		{
			    			MY_LOGE("need more parameters..., current(%d)",argc);
						}
						else
						{
							coreNum = atoi(argv[1]);
							random = atoi(argv[2]);
							loop = atoi(argv[3]);
							non_stop = 0;
							time_exec = 0;
					        time_sleep = 0;
							MY_LOGI("parameters, test_algo(%d_%s)/coreNum(%d/%d/%d)/time_exec(%d)\time_sleep(%d)",\
								E_VPU_ALGO_WARP,algo_name,coreNum,loop,random,time_exec,time_sleep);
							ret = vpu_voltage_freq_test((int)test_algo, coreNum, loop, random, time_exec, time_sleep);
							if(ret == VPU_TEST_SUCCESS)
								MY_LOGD("VPU Test PASS !!!");
							else
								MY_LOGD("VPU Test FAIL !!!");
							break;
						}
						break;
				}
			}
			else
			{
	            while(enter_test)
	            {
	                vpu_test_menu();
	                scanf("%d", &input);
	                testcase = input;
	                switch(testcase)
	                {
	                    case 1: //single core enque test
	                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
							MY_LOGD("specify core(0:do not specify, 1: coreA, 2: coreB, 3: coreC):");
	                        scanf("%u", &input);
							coreIndex = input;
	                        MY_LOGD("enque/deque test pattern:");
	                        scanf("%u", &input);
	                        test_pattern = input;
	                        MY_LOGD("enque/deque test count:");
	                        scanf("%u", &input);
	                        test_cnt = input;
	                        MY_LOGD("Opp(0~7, default is %d):", gVcoreOpp);
	                        scanf("%u", &input);
							gVcoreOpp = input;
	                        //MY_LOGD("DSP Freq Opp(default is %d):", gFreqOpp);
	                        //scanf("%u", &input);
							//gFreqOpp = input;
							if(test_cnt < MaxUINT && test_cnt > 0)
	                            ret = vpu_enque_test(test_pattern, test_cnt, coreIndex);
	                        else
	                            MY_LOGD("VPU Test Count Input FAIL !!!");
	                        //ret = vpu_enque_test(test_pattern, test_cnt, coreIndex);
	                        if(ret == VPU_TEST_SUCCESS)
	                            MY_LOGD("VPU Test PASS !!!");
	                        else
	                            MY_LOGD("VPU Test FAIL !!!");

	                        break;

	                    case 2: // multi-user test (legacy, not maintain...)
	                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
	                        MY_LOGD("enque/deque test pattern:");
	                        scanf("%u", &input);
	                        test_pattern = input;
	                        MY_LOGD("user count (max user cnt = %d):", MAX_MULTI_USER_CNT);
	                        scanf("%u", &input);
	                        user_cnt = input;
	                        MY_LOGD("queue count (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
	                        scanf("%u", &input);
	                        queue_cnt = input;
	                        MY_LOGD("dumpImage(default is %d):", gDumpImage);
	                        scanf("%u", &input);
	                        gDumpImage = input;
	                        ret = vpu_multi_user_test(test_pattern ,user_cnt, queue_cnt, queue_cnt);
	                        if(ret == VPU_TEST_SUCCESS)
	                            MY_LOGD("VPU Test PASS !!!");
	                        else
	                            MY_LOGD("VPU Test FAIL !!!");

	                        break;

	                    case 3:
	                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
	                        MY_LOGD("enque/deque test pattern:");
	                        scanf("%u", &input);
	                        test_pattern = input;
	                        MY_LOGD("user count (max user cnt = %d):", MAX_MULTI_USER_CNT);
	                        scanf("%u", &input);
	                        user_cnt = input;
	                        MY_LOGD("queue count (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
	                        scanf("%u", &input);
	                        queue_cnt = input;
	                        MY_LOGD("flush queue count (must less than queue cnt = %d):", queue_cnt);
	                        scanf("%u", &input);
	                        flush_que = input;
	                        MY_LOGD("dumpImage(default is %d):", gDumpImage);
	                        scanf("%u", &input);
	                        gDumpImage = input;
	                        if(flush_que >= queue_cnt)
	                            flush_que = queue_cnt - 1;
	                        ret = vpu_multi_user_test(test_pattern, user_cnt, queue_cnt, flush_que);
	                        if(ret == VPU_TEST_SUCCESS)
	                            MY_LOGD("VPU Test PASS !!!");
	                        else
	                            MY_LOGD("VPU Test FAIL !!!");

	                        break;
	                    case 4:
	                    {
	                        int time_exec, time_sleep, coreNum, loop, non_stop;
							#if 0
	                        MY_LOGD("HQA test pattern: (0: MaxPower, 1: Warping)");
	                        scanf("%u", &input);
	                        test_algo = input == 0 ? E_VPU_ALGO_MAX_PW : E_VPU_ALGO_WARP ;
	                        algo_name   = (char *)vpu_algo_name[test_algo];
	                        MY_LOGD("time to exec (secs)(max: 12 hr, min: 10 secs):");
	                        scanf("%u", &input);
	                        time_exec = input;
	                        MY_LOGD("time to sleep after executing (secs)(max: 1 hr, min: 1 secs):");
	                        scanf("%u", &input);
	                        time_sleep = input;
	                        MY_LOGD("loop count (max: 10000, min: 1):");
	                        scanf("%u", &input);
	                        loop = input;
	                        MY_LOGD("algo: %s, time_exec: %d (secs), time_sleep: %d(secs), loop: %d", test_algo == E_VPU_ALGO_MAX_PW ? "maxPower" : "warping", time_exec, time_sleep, loop);
	                        if (test_algo == E_VPU_ALGO_MAX_PW)
	                        {
	                          ret = vpu_hqa_maxPower_test(time_exec, time_sleep, loop);
	                        }
	                        else
	                        {
	                          ret = vpu_hqa_warping_test(time_exec, time_sleep, loop);
	                        }
							#else
							MY_LOGD("IMax test pattern: (2: harris corner, others: not support)");
							scanf("%u", &input);
							if(input != 2)
							{//currently not support other imax pattern
								input = 2;
							}
							algo_name = (char *)vpu_algo_name[input];
							MY_LOGD("IMax core Num: (1: single(coreA), 2: multi(coreA+coreB)");
							scanf("%u", &input);
							coreNum = input;
							MY_LOGD("IMax loop Num:");
							scanf("%u", &input);
							loop = input;
							non_stop = false;
							time_exec = 0;
					        time_sleep = 0;
							MY_LOGI("parameters, test_algo(%d_%s)/coreNum(%d/%d/%d)/time_exec(%d)\time_sleep(%d)",\
								test_algo,algo_name,coreNum,loop,non_stop,time_exec,time_sleep);
							ret = vpu_Imax_test((int)test_algo, coreNum, loop, non_stop, time_exec, time_sleep);
							#endif
	                        if(ret == VPU_TEST_SUCCESS)
	                            MY_LOGD("VPU Test PASS !!!");
	                        else
	                            MY_LOGD("VPU Test FAIL !!!");
	                        break;
	                    }
						case 5: //multi-core(fix_coreA + fix_coreB) test
						{
							algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
							MY_LOGD("enque/deque test pattern:");
							scanf("%u", &input);
							test_pattern = input;
							MY_LOGD("queue count (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
							scanf("%u", &input);
							queue_cnt = input;
							MY_LOGD("dumpImage(default is %d):", gDumpImage);
							scanf("%u", &input);
							gDumpImage = input;
							ret = vpu_fixcoreA_fixcoreB_test(test_pattern ,2 /*coreA, coreB*/, queue_cnt, queue_cnt);
							if(ret == VPU_TEST_SUCCESS)
								MY_LOGD("VPU Test PASS !!!");
							else
								MY_LOGD("VPU Test FAIL !!!");
							break;
						}
						case 6: //multi-core(fix_coreA + no specify) test
						{
							algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
							MY_LOGD("enque/deque test pattern:");
							scanf("%u", &input);
							test_pattern = input;
							MY_LOGD("queue count for coreA (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
							scanf("%u", &input);
							queue_cnt = input;
							MY_LOGD("queue count for no specify (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
							scanf("%u", &input);
							queue_cnt_2 = input;
							MY_LOGD("dumpImage(default is %d):", gDumpImage);
							scanf("%u", &input);
							gDumpImage = input;
							ret = vpu_fixcoreA_nospecify_test(test_pattern ,2 /*coreA, common*/, queue_cnt, queue_cnt_2);
							if(ret == VPU_TEST_SUCCESS)
								MY_LOGD("VPU Test PASS !!!");
							else
								MY_LOGD("VPU Test FAIL !!!");
							break;
						}

						case 7:
						{
							MY_LOGD("VPU performance test !!!:");

 						    vpu_performance_test(256*256);

							break;
						}


	                    case 9:
	                        enter_test = 0;
	                        break;

	                    default:
	                        break;
	                }

				}
			}
            break;
        default:
            break;
    }

EXIT:
    if(ion_drv_handle)
        ion_close(ion_drv_handle);

    if (bLoad)
    {
        if (g_datasrc_wpp)
            free((void *) g_datasrc_wpp);
        if (g_datadst_golden_wpp)
            free((void *) g_datadst_golden_wpp);
        if (g_procParam)
            free((void *) g_procParam);
    }

    return 1;
}
