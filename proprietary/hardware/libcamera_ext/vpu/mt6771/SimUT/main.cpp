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

#include <log/log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream

#include <cutils/properties.h>  // For property_get().

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_TAG
#define LOG_TAG "MtkCam.vpu_test"

typedef uint64_t            MUINT64;
typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef int64_t             MINT64;
typedef signed int          MINT32;
typedef signed short        MINT16;
typedef signed char         MINT8;

typedef int                 MBOOL;
typedef float               MFLOAT;

// *****************
// data partition setting
// *****************
#define AUTO_TEST_TASK_PARTITION_NUM (5)
#define MAX_TASK_NUM (10)
// *****************
// profile
// *****************
#define TIME_PROF
#if defined(TIME_PROF)

#include <sys/time.h>
typedef timeval MY_TIME;

#define MY_GET_TIME gettimeofday
#define MY_DIFF_TIME time_diff
#define MY_DISPLAY_TIME time_display
#define MY_RESET_TIME(a) memset(a, 0, sizeof(a))
#define MY_ADD_TIME(a, b) (a += b)
static void time_diff(MINT32 *elapse_t, timeval start_t, timeval end_t)
{
    *elapse_t = (end_t.tv_sec - start_t.tv_sec)*1000000 + (end_t.tv_usec - start_t.tv_usec);
}
static void time_display(MINT32 elapse_t, const char *string)
{
    printf("%s: %8d(us) = %5.4f(ms)\n", string, elapse_t, elapse_t/1000.0f);
}

MINT32 elapse_time[12];
MINT32 temp_time;
MY_TIME start_time, end_time, sub_start_time, sub_end_time;
#endif

static void readImage(char* path, void* data, int size)
{
    FILE * pFile;
    int read_size;

    pFile = fopen ( path , "rb" );
    if (pFile == NULL)
    {
        printf("[Error] open %s fail\n", path);
    }

    // copy the file into the buffer:
    read_size = fread (data, 1, size, pFile);
    if (read_size != size)
    {
        printf("[Error] read error %d %d\n", read_size, size);
    }

    // terminate
    fclose (pFile);
}

static void writeImage(char* path, void* data, int size)
{
    FILE * pFile;
    int read_size;

    pFile = fopen ( path , "wb" );
    if (pFile == NULL)
    {
        printf("[Error] open %s fail\n", path);
    }

    // copy the file into the buffer:
    read_size = fwrite (data, 1, size, pFile);
    if (read_size != size)
    {
        printf("[Error] write error %d %d\n", read_size, size);
    }

    // terminate
    fclose (pFile);
}

struct vpu_st_sett_harris {
    int S32_Scale;
    int S32_Offset;
};

struct vpu_st_sett_histogram {
    int S32_Scale;
    int S32_Offset;
};

struct vpu_st_sett_canny {
    int S32_Scale;
    int S32_Offset;
};

struct vpu_st_sett_k15 {
    float F32_Scale;
    int S32_RX;
    int S32_RY;
    int S32_Width;
    int S32_Height;
    int S32_Stride;
    int S32_BlkSize;
};
struct vpu_st_sett_k14 {
    int S32_Width;
    int S32_Height;
    int S32_Stride;
};
struct vpu_st_sett_swnr {
    int S32_Width;
    int S32_Height;
    int S32_Stride;
    int S32_Stage;
    int S32_NeedBlend;
    int S32_NeedDither;
    int S32_NRSmooth;
};

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

#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "[%s] " fmt "\n",  __FUNCTION__, ##arg)

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
//static char gpSrcName[]    = "/data/vpu_test_src.raw";
//static char gpSrcParam[]   = "/data/vpu_test_src_param.raw";
//static char gpGoldenName[] = "/data/vpu_test_golden.raw";
//static unsigned char *g_datasrc_wpp = NULL;
//static unsigned char *g_datadst_golden_wpp = NULL;
//static unsigned char *g_procParam = NULL;

typedef enum {
    E_VPU_ALGO_NONE = 0,
    E_VPU_ALGO_WARP,
    E_VPU_ALGO_MAX_PW,
    E_VPU_ALGO_SHIFT,
    E_VPU_ALGO_MAX,
} VPU_ALGO_E ;

//static char const *vpu_algo_name[] = {"vpu_flo_d2d_k3", "vpu_flo_d2d_k3", "vpu_flo_d2d_k7", "vpu_flo_d2d_k5", "vpu_pil_d2d_k5"};

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
//static sem_t     mSem;
//static bool g_non_stop;
//static int g_time_exec;
//static int g_time_sleep;

int ion_drv_handle = 0;

// image width, height, size, data length
//static UINT32 gTestImgWidth   = 0;
//static UINT32 gTestImgHeight  = 0;
//static UINT32 gTestImgSize    = 0;
//static UINT32 gTestDataLength = 0;

// save test image data pointer into global variables due to create threads
//static UINT8 *pgTest_img1 = NULL, *pgTest_img2 = NULL, *pgTest_data1 = NULL;
//static UINT32 multi_user_que_cnt = 2, flush_que_cnt = 0; //old usage
//static UINT32 coreA_multi_user_que_cnt = 2, coreA_flush_que_cnt = 0;
//static UINT32 coreB_multi_user_que_cnt = 2, coreB_flush_que_cnt = 0;
//static UINT32 common_multi_user_que_cnt = 2, common_flush_que_cnt = 0;
//static UINT32 gDumpImage = 0;
//static bool coreA_thread_need_wait = false;
//static bool coreB_thread_need_wait = false;
//static bool common_thread_need_wait = false;

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
/*
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
}*/
#if 0
static int _vpu_flush_buf(int mIonDrv, int IonBufFd)
{
    ion_user_handle_t pIonHandle;

    if(ion_import(mIonDrv, IonBufFd, &pIonHandle))
    {
        MY_LOGE("ion_import fail,memID(0x%x)",IonBufFd);
        return -1;
    }

    //b. cache sync by range

    struct ion_sys_data sys_data;

    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;

    sys_data.cache_sync_param.handle=(ion_user_handle_t)pIonHandle;

    //sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;

    sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_ALL;

    if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
    {
        MY_LOGE("CAN NOT DO SYNC");
    }

    if(ion_free(mIonDrv,pIonHandle))
    {
        MY_LOGE("ion_free fail");
        return -1;
    }
    return -1;
}
#endif

static int _vpu_flush_buf_by_range(int mIonDrv, size_t buf_size, int fd, void* buffer_va)
{
    ion_user_handle_t pIonHandle;

    if(ion_import(mIonDrv, fd, &pIonHandle))
    {
        MY_LOGE("ion_import fail,memID(0x%x)",fd);
        return -1;
    }

    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle= (ion_user_handle_t)pIonHandle;
    
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    sys_data.cache_sync_param.va = buffer_va;                       //please input correct kernel/user va of this buffer.
    sys_data.cache_sync_param.size = buf_size;                      // if you have special usage, can not do all ion buffer flush ,please contact me.
    
    if (ion_custom_ioctl(fd, ION_CMD_SYSTEM, (void*)&sys_data))
    {
        MY_LOGE("Cache flush by range error: fd %d, va:0x%x, size : %d\n", fd, buffer_va, buf_size);
        return -1;
    }

    return 0;
}

static void _vpu_setup_buf(VpuBuffer &buf, int buf_fd, VpuBufferFormat format,
                           unsigned int width,  unsigned int height, unsigned int buf_offset,
                           unsigned int stride, unsigned int length)
{
    MY_LOGD("_vpu_setup_buf:%d,%d,%d,%d,%d,%d",buf_fd,width,height,buf_offset,stride,length);
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
/*
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
}*/

static MBOOL VPU_Get_IonBuffer(int drv_h, int len, int ByteAlign, int cache,int *buf_share_fd, void **buf_va)
{
    ion_user_handle_t buf_handle;

    int ion_prot_flag = cache ? (ION_FLAG_CACHED|ION_FLAG_CACHED_NEEDS_SYNC):0;
    // allocate ion buffer handle
    if(ion_alloc_mm(drv_h, (size_t)len, ByteAlign, ion_prot_flag, &buf_handle))        // no alignment, non-cache
    {
        printf("fail to get ion buffer handle (drv_h=0x%x, len=%d)", drv_h, len);
        return 0;
    }
    // get ion buffer share handle
    if(ion_share(drv_h, buf_handle, buf_share_fd))
    {
        printf("fail to get ion buffer share handle");
        if(ion_free(drv_h,buf_handle))
            printf("ion free fail");
        return 0;
    }
    // get buffer virtual address
    *buf_va = ion_mmap(drv_h, NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, *buf_share_fd, 0);
    printf("alloc ion: ion_buf_handle %d, share_fd %d", buf_handle, *buf_share_fd);
    if(*buf_va != NULL)
        return 1;
    else
    {
        printf("fail to get buffer virtual address");
        return 0;
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

static BOOL _vpu_free_buffer(int drv_h, int len,int buf_share_fd, UINT8 *buf_va)
{
    ion_user_handle_t buf_handle;
    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        MY_LOGW("fail to get import share buffer fd");
        return 0;
    }
    //MY_LOGV("import ion: ion_buf_handle %d, share_fd %d", buf_handle, buf_share_fd);
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

void padding_dup(MUINT8 *p_u8Src, MUINT8 *p_u8Dst, MUINT32 width, MUINT32 height, int pad_size)
{
    MINT32 i;

    for (i=0; i<pad_size+1; i++)
    {
        memset(p_u8Dst, p_u8Src[0], pad_size);
        p_u8Dst += pad_size;

        memcpy(p_u8Dst, p_u8Src, width);
        p_u8Dst += width;

        memset(p_u8Dst, p_u8Src[width-1], pad_size);
        p_u8Dst += pad_size;
    }
    p_u8Src += width;

    for (i=1; i<(int)height-1; i++)
    {
        memset(p_u8Dst, p_u8Src[0], pad_size);
        p_u8Dst += pad_size;

        memcpy(p_u8Dst, p_u8Src, width);
        p_u8Dst += width;

        memset(p_u8Dst, p_u8Src[width-1], pad_size);
        p_u8Dst += pad_size;

        p_u8Src += width;
    }

    for (i=0; i<pad_size+1; i++)
    {
        memset(p_u8Dst, p_u8Src[0], pad_size);
        p_u8Dst += pad_size;

        memcpy(p_u8Dst, p_u8Src, width);
        p_u8Dst += width;

        memset(p_u8Dst, p_u8Src[width-1], pad_size);
        p_u8Dst += pad_size;
    }
}
int vpu_enque_harris(int i4ImgInFD,int i4ImgOutFD,int  width,int height)
{
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    //VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2;
    VpuRequest *request = NULL;
    VpuExtraParam extra_param;
    VpuRequest *deque_req = NULL;
	char algoName[] = "vpu_lib_d2d_harris";

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }
/*
    VpuPower pwr;
    pwr.mode = ePowerModeOn;
    pwr.opp  = ePowerOppUnrequest;
    if (!vpu->setPower(pwr))
    {
        MY_LOGW("fail to setPower!!");
        goto EXIT;
    }*/

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo = vpu->getAlgo(algoName);
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    // 3. setup request
    // 3.1 acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    if ((request = vpu->acquire(algo)) == NULL)
    {
        MY_LOGW("fail to get vpu request !!!");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3.2 add buffers to request
    MY_GET_TIME(&sub_start_time, NULL);

    _vpu_setup_buf(buf_img1, i4ImgInFD, eFormatImageY8, width + 6, height + 6, 0, width + 6, (width + 6) * (height + 6));
    _vpu_setup_buf(buf_img2, i4ImgOutFD, eFormatImageY8, width, height, 0, width, width*height);

    buf_img1.port_id = 0;
    request->addBuffer( buf_img1);
    buf_img2.port_id = 1;
    request->addBuffer( buf_img2);
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    //VpuProperty prop;

    //int param1;

    vpu_st_sett_harris m_property;
    MY_GET_TIME(&sub_start_time, NULL);
    m_property.S32_Scale = width;
    m_property.S32_Offset = height;

    res = request->setProperty((void*)&m_property, sizeof(vpu_st_sett_harris));
    if(res == false)
    {
        MY_LOGD("Error: set property fail!");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu_status!");
        goto EXIT;
    }

    // 4. enqueue & dequeue

    MY_GET_TIME(&sub_start_time, NULL);

    extra_param.opp_step = 0;
    //extra_param.bw = 0;
    request-> setExtraParam(extra_param);
    MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, 0);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[6], temp_time);
    MY_GET_TIME(&sub_start_time, NULL);

    vpu->enque(request, eIndex_A);
    if(res == false)
    {
        MY_LOGD("Error: enque fail!");
        goto EXIT;
    }

    deque_req = vpu->deque();
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);
    MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

    // 5. check result
    MY_LOGD("test finished !!!, (req=%p, deque_req=%p)",request, deque_req);

EXIT:
    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    if (request)
        vpu->release(request);

    if (vpu)
        delete vpu;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("test finish");

    return 0;
}

int vpu_enque_histogram(int i4ImgInFD,int i4ImgOutFD,int  width,int height)
{
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    //VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2;
    VpuRequest *request = NULL;
    //VpuProperty prop;
    vpu_st_sett_histogram m_property;
    VpuRequest *deque_req = NULL;
    VpuExtraParam extra_param;
	char algoName[] = "vpu_lib_d2d_histogram";

    //int param1;

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo = vpu->getAlgo(algoName);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);

    //2. acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    if ((request = vpu->acquire(algo)) == NULL)
    {
        MY_LOGW("fail to get vpu request !!!");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);
    buf_img1.port_id = 0;
    _vpu_setup_buf(buf_img1, i4ImgInFD, eFormatImageY8, width, height, 0, width, width*height);
    request->addBuffer( buf_img1);
    buf_img2.port_id = 1;
    _vpu_setup_buf(buf_img2, i4ImgOutFD, eFormatImageY8, width, height, 0, width, width*height);
    request->addBuffer( buf_img2);
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    extra_param.opp_step = 0;
    extra_param.bw = 0;
    request-> setExtraParam(extra_param);

    m_property.S32_Scale = 0;
    m_property.S32_Offset = height;
    res = request->setProperty((void*)&m_property, sizeof(vpu_st_sett_histogram));
    if(res == false)
    {
        MY_LOGD("Error: set property fail!");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu_status!");
        goto EXIT;
    }

    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);

    vpu->enque(request);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[6], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);

    deque_req = vpu->deque();
    MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);

    // 5. check result
    MY_LOGD("test finished !!!, (req=%p, deque_req=%p)",request, deque_req);
EXIT:
    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    if (request)
        vpu->release(request);

    if (vpu)
        delete vpu;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("test finish");

    return 0;
}
int vpu_enque_canny(int i4ImgInFD,int i4ImgWkFD,int i4ImgOutFD,int  width,int height, int pad_size)
{
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    //VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2,buf_img3;
    VpuRequest *request = NULL;
    //VpuProperty prop;
    vpu_st_sett_canny m_property;
    VpuRequest *deque_req = NULL;
    char algoName[] = "vpu_lib_d2d_canny";
    //int param1;

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo = vpu->getAlgo(algoName);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);

    //2. acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    if ((request = vpu->acquire(algo)) == NULL)
    {
        MY_LOGW("fail to get vpu request !!!");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);

    _vpu_setup_buf(buf_img1, i4ImgInFD, eFormatImageY8, width+2*pad_size, height+2*pad_size, 0, width+2*pad_size, (width+2*pad_size)*(height+2*pad_size));
    _vpu_setup_buf(buf_img2, i4ImgWkFD, eFormatImageY8, width+pad_size, height+pad_size, 0, width+pad_size, (width+pad_size)*(height+pad_size));
    _vpu_setup_buf(buf_img3, i4ImgOutFD, eFormatImageY8, width, height, 0, width, width*height);

    buf_img1.port_id = 0;
    request->addBuffer( buf_img1);
    buf_img2.port_id = 1;
    request->addBuffer( buf_img2);
    buf_img3.port_id = 2;
    request->addBuffer( buf_img3);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    m_property.S32_Scale = width;
    m_property.S32_Offset = height;
    res = request->setProperty((void*)&m_property, sizeof(vpu_st_sett_canny));
    if(res == false)
    {
        MY_LOGD("Error: set property fail!");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu_status!");
        goto EXIT;
    }

    MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, 0);
    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);
    vpu->enque(request, eIndex_A);
    if(res == false)
    {
        MY_LOGD("Error: enque fail!");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[6], temp_time);

    MY_LOGD("enque done");

    MY_GET_TIME(&sub_start_time, NULL);
    deque_req = vpu->deque();
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);
    MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

    // 5. check result
    MY_LOGD("test finished !!!, (req=%p, deque_req=%p)",request, deque_req);
EXIT:
    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    if (request)
        vpu->release(request);

    if (vpu)
        delete vpu;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("test finish");

    return 0;
}

int vpu_enque_histogram_data_partition(int i4ImgInFD,int i4ImgOutFD,int  width,int height,int loop)
{
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    //VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2;
    VpuRequest *request[AUTO_TEST_TASK_PARTITION_NUM];
    //VpuProperty prop;
    vpu_st_sett_histogram m_property;
    VpuRequest *deque_req = NULL;
    VpuExtraParam extra_param;
	char algoName[] = "vpu_lib_d2d_histogram";

    //int param1;
    int i, enq_count = 0, deq_count = 0, cnt = 0;

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        request[i]  = NULL;
    }

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo = vpu->getAlgo(algoName);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);

    //2. acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        if ((request[i] = vpu->acquire(algo)) == NULL)
        {
            MY_LOGW("fail to get vpu request[%d] !!!",i);
            goto EXIT;
        }
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);
    buf_img1.port_id = 0;
    _vpu_setup_buf(buf_img1, i4ImgInFD, eFormatImageY8, width, height, 0, width, width*height);

    buf_img2.port_id = 1;
    _vpu_setup_buf(buf_img2, i4ImgOutFD, eFormatImageY8, width, height, 0, width, width*height);
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++){
        request[i]->addBuffer( buf_img1);
        request[i]->addBuffer( buf_img2);
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    extra_param.opp_step = 0;
    extra_param.bw = 0;
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        int working_height = ((height/AUTO_TEST_TASK_PARTITION_NUM)/30)*30;
        m_property.S32_Scale = i*working_height;
        m_property.S32_Offset = (i+1)*working_height;
        res = request[i]->setProperty((void*)&m_property, sizeof(vpu_st_sett_histogram));
        if(res == false)
        {
            MY_LOGD("Error: set property fail!");
            goto EXIT;
        }
        request[i]-> setExtraParam(extra_param);
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu_status!");
        goto EXIT;
    }
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        MY_LOGD("vpu->enque[%d](0x%lx): number %d",i, (unsigned long)request[i], 0);
    }
    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);

    while(cnt < loop)
    {
        enq_count = 0;
        deq_count = 0;
        for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
        {
            if (vpu->enque(request[i])) enq_count++;
        }
        MY_LOGD("enque_count = %d",enq_count);
        while(1)
        {
            deque_req = vpu->deque();
            MY_LOGD("deque_req = (0x%lx) %lu",(unsigned long)deque_req,(unsigned long)deque_req);

            deq_count++;
            if(deq_count == enq_count)
                goto EXIT;
        }
        cnt++;
    }

EXIT:
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);
    MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

    // 5. check result
    //MY_LOGD("test finished !!!, (req=%p, deque_req=%p)",request, deque_req);

    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        if (request[i])
            vpu->release(request[i]);
    }

    if (vpu)
        delete vpu;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("test finish");

    return 0;
}
int vpu_enque_pregf_task_partition(int i4DownInFD[], int i4DownOutFD[], int i4DownTileFD[],
                                   int i4Part1In0FD[], int i4Part1In1FD[], int i4Part1Out0FD[], int i4Part1Out1FD[], int i4Part1TileFD[],
                                   int width, int height, int bitResolution, int tileDataSize, int runCnt)

{
    VpuStream  *vpu0     = NULL;
    VpuStream  *vpu1     = NULL;

    VpuAlgo    *algo0    = NULL;
    VpuAlgo    *algo1    = NULL;

    //VpuPort    *port0_1, *port0_2, *port0_3;
    //VpuPort    *port1_1, *port1_2, *port1_3, *port1_4, *port1_5;

    VpuBuffer   buf_img0_1, buf_img0_2, buf_data0;
    VpuBuffer   buf_img1_1, buf_img1_2,buf_img1_3, buf_img1_4, buf_data1;

    VpuRequest *request0[MAX_TASK_NUM];
    VpuRequest *request1[MAX_TASK_NUM];

    char algoName0[] = "vpu_flo_d2d_k14";
	char algoName1[] = "vpu_flo_d2d_k15";
	
    vpu_st_sett_k14 m_property_k14;
    vpu_st_sett_k15 m_property_k15;

    VpuRequest *deque_req = NULL;

    //int loop = 1;

    //int param1;
    int i, enq_count0 = 0, deq_count0 = 0, cnt = 0;
    int enq_count1 = 0, deq_count1 = 0;

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    for (i = 0 ; i < runCnt ; i++)
    {
        request0[i]  = NULL;
        request1[i]  = NULL;
    }

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu0 = VpuStream::createInstance();
    if (vpu0 == NULL)
    {
        MY_LOGW("fail to get vpu0 handle");
        goto EXIT;
    }

    vpu1 = VpuStream::createInstance();
    if (vpu1 == NULL)
    {
        MY_LOGW("fail to get vpu1 handle");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo0 = vpu0->getAlgo(algoName0);
    if (algo0 == NULL)
    {
        MY_LOGW("fail to get vpu algo0 handle");
        goto EXIT;
    }

    algo1 = vpu1->getAlgo(algoName1);
    if (algo1 == NULL)
    {
        MY_LOGW("fail to get vpu algo1 handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);

    //2. acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < runCnt ; i++)
    {
        if ((request0[i] = vpu0->acquire(algo0)) == NULL)
        {
            MY_LOGW("fail to get vpu request0[%d] !!!",i);
            goto EXIT;
        }
        if ((request1[i] = vpu1->acquire(algo1)) == NULL)
        {
            MY_LOGW("fail to get vpu request0[%d] !!!",i);
            goto EXIT;
        }
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);

    for (i = 0 ; i < runCnt ; i++){
        buf_img0_1.port_id = 0;
        _vpu_setup_buf(buf_img0_1, i4DownInFD[/*i*/0], eFormatImageY8, width, height, 0, width*bitResolution, width*height*bitResolution);

        buf_img0_2.port_id = 1;
        _vpu_setup_buf(buf_img0_2, i4DownOutFD[i], eFormatImageY8, width, height, 0, width*bitResolution, width*height*bitResolution);

        buf_data0.port_id = 2;
        _vpu_setup_buf(buf_data0, i4DownTileFD[/*i*/0], eFormatData, tileDataSize, 1, 0, tileDataSize, tileDataSize);

        request0[i]->addBuffer( buf_img0_1);
        request0[i]->addBuffer( buf_img0_2);
        request0[i]->addBuffer( buf_data0);

        buf_img1_1.port_id = 0;
        _vpu_setup_buf(buf_img1_1, i4Part1In0FD[i], eFormatImageY8, width/2, height/2, 0, (width/2)*bitResolution, (width/2)*(height/2)*bitResolution);

        buf_img1_2.port_id = 1;
        _vpu_setup_buf(buf_img1_2, i4Part1In1FD[i], eFormatImageY8, width/2, height/2, (width/2*8+8)*bitResolution, (width/2)*bitResolution, (width/2)*(height/2)*bitResolution);

        buf_img1_3.port_id = 2;
        _vpu_setup_buf(buf_img1_3, i4Part1Out0FD[i], eFormatImageY8, width/2, height/2, (width/2*8+8)*bitResolution, (width/2)*bitResolution, (width/2)*(height/2)*bitResolution);

        buf_img1_4.port_id = 3;
        _vpu_setup_buf(buf_img1_4, i4Part1Out1FD[i], eFormatImageY8, width/2, height/2, (width/2*8+8)*bitResolution, (width/2)*bitResolution, (width/2)*(height/2)*bitResolution);

        buf_data1.port_id = 4;
        _vpu_setup_buf(buf_data1, i4Part1TileFD[i], eFormatData, tileDataSize, 1, 0, tileDataSize, tileDataSize);

        request1[i]->addBuffer( buf_img1_1);
        request1[i]->addBuffer( buf_img1_2);
        request1[i]->addBuffer( buf_img1_3);
        request1[i]->addBuffer( buf_img1_4);
        request1[i]->addBuffer( buf_data1);
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < runCnt ; i++)
    {
        m_property_k14.S32_Width = 128;
        m_property_k14.S32_Height = 32;
        m_property_k14.S32_Stride = 128;
        res = request0[i]->setProperty((void*)&m_property_k14, sizeof(vpu_st_sett_k14));
        if(res == false)
        {
            MY_LOGD("Error: set k14 property fail (%d)!",i);
            goto EXIT;
        }

        m_property_k15.F32_Scale = 16.0;
        m_property_k15.S32_RX = 4;
        m_property_k15.S32_RY = 4;
        m_property_k15.S32_Width = 128;
        m_property_k15.S32_Height = 32;
        m_property_k15.S32_Stride = 128;
        m_property_k15.S32_BlkSize = 128;
        res = request1[i]->setProperty((void*)&m_property_k15, sizeof(vpu_st_sett_k15));
        if(res == false)
        {
            MY_LOGD("Error: set k15 property fail (%d)!",i);
            goto EXIT;
        }
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu0->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu0_status!");
        goto EXIT;
    }
    res = vpu1->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu1_status!");
        goto EXIT;
    }
    for (i = 0 ; i < runCnt ; i++)
    {
        MY_LOGD("vpu0->enque[%d](0x%lx): number %d",i, (unsigned long)request0[i], 0);
        MY_LOGD("vpu1->enque[%d](0x%lx): number %d",i, (unsigned long)request1[i], 0);
    }
    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);
    enq_count1 = enq_count0 = 0;
    deq_count1 = deq_count0 = 0;

#if 1
    {
        for (i = 0 ; i < runCnt ; i++)
        {
            if (vpu0->enque(request0[i],eIndex_B)) enq_count0++;
            if (vpu1->enque(request1[i],eIndex_A)) enq_count1++;
        }
        MY_LOGD("enque_count0 = %d",enq_count0);
        MY_LOGD("enque_count1 = %d",enq_count1);
        while(1)
        {
            deque_req = vpu0->deque();
            MY_LOGD("vpu0 deque_req = (0x%lx) %d",(unsigned long)deque_req,deq_count0);
            if (deque_req == request0[deq_count0]) deq_count0++;

            deque_req = vpu1->deque();
            MY_LOGD("vpu1 deque_req = (0x%lx) %d",(unsigned long)deque_req,deq_count1);
            if (deque_req == request1[deq_count1]) deq_count1++;

            if(deq_count0 == enq_count0 && deq_count1 == enq_count1)
                goto EXIT;
        }

        cnt++;
    }
#else
    vpu0->enque(request0[8],1);
    deque_req = vpu0->deque();
    MY_LOGD("vpu0 deque_req = (0x%lx) %d",(unsigned long)deque_req,deq_count0);
#endif

EXIT:
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);

    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < runCnt ; i++)
    {
        if (request0[i])
            vpu0->release(request0[i]);
        if (request1[i])
            vpu1->release(request1[i]);
    }

    if (vpu0)
        delete vpu0;
    if (vpu1)
        delete vpu1;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("task partition test finish");

    return 0;
}
int vpu_enque_swnr(int i4InOutFD,int i4InFD,int i4OutFD,int i4WeightFD,int width, int height, int i4Run, UINT8 *prVaInOut, int ion_drv_handle)
{
   VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    //VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2,buf_img3,buf_img4,buf_img5;
    VpuRequest *request = NULL;
    //VpuProperty prop;
    vpu_st_sett_swnr m_property;
    VpuRequest *deque_req = NULL;
    VpuExtraParam extra_param;
    int i,j;
    char algoName[] = "vpu_lib_d2d_swnr";

    std::vector<VpuStatus> vpu_status;

    bool res=false;

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo = vpu->getAlgo(algoName);
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        goto EXIT;
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[2], temp_time);

    //2. acquire request
    MY_GET_TIME(&sub_start_time, NULL);
    if ((request = vpu->acquire(algo)) == NULL)
    {
        MY_LOGW("fail to get vpu request !!!");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);

    _vpu_setup_buf(buf_img1, i4InOutFD, eFormatImageY8, width, height, 0, width, width*height);
    _vpu_setup_buf(buf_img2, i4InFD, eFormatImageY8, width, height, 0, width, width*height);
    _vpu_setup_buf(buf_img3, i4InOutFD, eFormatImageY8, width, height, 0, width, width*height);
    _vpu_setup_buf(buf_img4, i4OutFD, eFormatImageY8, width, height, 0, width, width*height);
    _vpu_setup_buf(buf_img5, i4WeightFD, eFormatImageY8, 256, 1, 0, 256, 256);

    buf_img1.port_id = 0;
    request->addBuffer( buf_img1);
    buf_img2.port_id = 1;
    request->addBuffer( buf_img2);
    buf_img3.port_id = 2;
    request->addBuffer( buf_img3);
    buf_img4.port_id = 3;
    request->addBuffer( buf_img4);
    buf_img5.port_id = 4;
    request->addBuffer( buf_img5);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    m_property.S32_Width = width;
    m_property.S32_Height = height;
    m_property.S32_Stride = width;
    m_property.S32_Stage = 1;
    m_property.S32_NeedBlend = 1;
    m_property.S32_NeedDither = 1;
    m_property.S32_NRSmooth = 256;

    res = request->setProperty((void*)&m_property, sizeof(vpu_st_sett_swnr));
    if(res == false)
    {
        MY_LOGD("Error: set property fail!");
        goto EXIT;
    }

    extra_param.opp_step = 0;
    request-> setExtraParam(extra_param);

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        MY_LOGD("Error: vpu_status!");
        goto EXIT;
    }

    MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, 0);

    //Dirty image
    if(i4Run > 1){
        printf("Do dirty image!\n");
        for(i=0;i<512;i++)
            for(j=0;j<512;j++)
            {
                if(prVaInOut[i*width + j] > 127)
                    prVaInOut[i*width + j] = 255;
                else
                    prVaInOut[i*width + j] = 0;
            }
    }

    if(i4Run == 3){
        printf("Do cache sync!\n");
        //ion_cache_sync_flush_all(ion_drv_handle);
    }

    // 4. enqueue & dequeue
    if(i4Run>4)
        i4Run = i4Run-3;
    else
        i4Run = 1;
    for(i=0;i<i4Run;i++)
    {
        MY_GET_TIME(&sub_start_time, NULL);
        vpu->enque(request, eIndex_A);
        if(res == false)
        {
            MY_LOGD("Error: enque fail!");
            goto EXIT;
        }

        deque_req = vpu->deque();
        MY_GET_TIME(&sub_end_time, NULL);
        MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
        MY_ADD_TIME(elapse_time[7], temp_time);
        MY_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

        MY_DISPLAY_TIME(temp_time , "[swnr] ENQUE+DEQUE   ");
    }
    // 5. check result
    MY_LOGD("test finished !!!, (req=%p, deque_req=%p)",request, deque_req);
EXIT:
    // 6. release resource & exit
    MY_GET_TIME(&sub_start_time, NULL);
    if (request)
        vpu->release(request);

    if (vpu)
        delete vpu;

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[8], temp_time);

    MY_LOGI("test finish");

    return 0;
}
/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    UINT8 *prVaIn1, *prVaIn2, *prVaOut1;
    //UINT8 *prVaIn3, *prVaIn4,*prVaIn5;
    char * pi1WorkingFolder;
    char i1FilePath[300];

    //char value[PROPERTY_VALUE_MAX] = {'\0'};

    int i4Case = 0, i4Run = 1;
    int i;

    for( i = 1; i < argc; i++)
    {
        char* key = argv[i];

        switch(key[1]){
        case 'i':  // [must]
            i++;
            pi1WorkingFolder = argv[i];
            printf("Working Path: %s\n", pi1WorkingFolder);
            break;
        case 'c':   // [must]
            i++;
            i4Case = atoi(argv[i]);
            printf("run Case: %d\n", i4Case);
            break;
        case 'r':   // [optional]
            i++;
            i4Run = atoi(argv[i]);
            printf("run Times: %d\n", i4Run);
            break;
        case 'm':   // [optional]

            break;
        }
    }

    //check i4Run
    if(i4Run < 0 || i4Run > 100)
    {
        MY_LOGW("i4Run out of range, reset to 1");
        i4Run = 1;
    }

    // open ion
    ion_drv_handle = mt_ion_open(__FILE__);
    if (!ion_drv_handle)
    {
      MY_LOGW("fail to get ion driver handle");
      goto EXIT;
    }

    if(i4Case == 0)
    {
#define TEST_PATTERN_WD (1920)
#define TEST_PATTERN_HT (1080)
#define SIZE_IMAGE (TEST_PATTERN_WD*TEST_PATTERN_HT)
#define TEST_PATTERN "data/input_1920x1080_400.yuv"
#define BITTRUE_PATTERN  "data/out_bt_1920x1080"
        int i4ImgInFD;
        int i4ImgOutFD;

        //load file
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD+6)*(TEST_PATTERN_HT+6)* sizeof(char), &i4ImgInFD, &prVaIn1);
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), &i4ImgOutFD, &prVaOut1);

        sprintf(i1FilePath, TEST_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaOut1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));
        padding_dup(prVaOut1, prVaIn1, TEST_PATTERN_WD, TEST_PATTERN_HT, 3);

        //reset output
        memset(prVaOut1,0x00,TEST_PATTERN_WD*TEST_PATTERN_HT);

        // flush cache
        //ion_cache_sync_flush_range( ion_drv_handle);
        //ion_cache_sync_flush_range( ion_drv_handle);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD+6)*(TEST_PATTERN_HT+6)* sizeof(char), i4ImgInFD, (void*)prVaIn1);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char),     i4ImgOutFD,(void*)prVaOut1);

        for( i=0; i<i4Run; i++)
        {
            MY_RESET_TIME(elapse_time);

            MY_GET_TIME(&start_time, NULL);

            vpu_enque_harris(i4ImgInFD, i4ImgOutFD, TEST_PATTERN_WD, TEST_PATTERN_HT);

            MY_GET_TIME(&end_time, NULL);
            MY_DIFF_TIME(&temp_time, start_time, end_time);
            MY_ADD_TIME(elapse_time[0], temp_time);

            MY_DISPLAY_TIME(elapse_time[0] , "[HARRIS] vpu_enque_harris   ");
            MY_DISPLAY_TIME(elapse_time[1] , "[HARRIS] - createInstance");
            MY_DISPLAY_TIME(elapse_time[2] , "[HARRIS] - getAlgo       ");
            MY_DISPLAY_TIME(elapse_time[3] , "[HARRIS] - Acquire       ");
            MY_DISPLAY_TIME(elapse_time[4] , "[HARRIS] - add buffer    ");
            MY_DISPLAY_TIME(elapse_time[5] , "[HARRIS] - set property  ");
            MY_DISPLAY_TIME(elapse_time[6] , "[HARRIS] - set extra para");
            MY_DISPLAY_TIME(elapse_time[7] , "[HARRIS] - enque+deque   ");
            MY_DISPLAY_TIME(elapse_time[8] , "[HARRIS] - release       ");
        }

        //verify
        char* pBuffer = (char*)malloc((TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));
        sprintf(i1FilePath, BITTRUE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBuffer, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));

        if (memcmp(pBuffer, prVaOut1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char)) == 0)
        {
            printf("imgOut: Pass\n");
        }
        else
        {
            printf("imgOut: Fail\n");

            sprintf(i1FilePath, "data/imgOut_error_out.bin");
            printf("Write imgOut_error_out.bin: %s\n", i1FilePath);
            writeImage(i1FilePath, prVaOut1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));
        }
        free(pBuffer);

        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD+6)*(TEST_PATTERN_HT+6)* sizeof(char), i4ImgInFD, prVaIn1);
        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgOutFD, prVaOut1);

#undef TEST_PATTERN_WD
#undef TEST_PATTERN_HT
#undef SIZE_IMAGE
#undef TEST_PATTERN
#undef BITTRUE_PATTERN
    }
    else if(i4Case == 1)
    {
#define TEST_PATTERN_WD (1920)
#define TEST_PATTERN_HT (1080)
#define SIZE_IMAGE (TEST_PATTERN_WD*TEST_PATTERN_HT)
#define TEST_PATTERN "data/input_1920x1080_400.yuv"

        int i4ImgInFD;
        int i4ImgOutFD;

        //load file
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), &i4ImgInFD, &prVaIn1);
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), &i4ImgOutFD, &prVaOut1);

        sprintf(i1FilePath, TEST_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaIn1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));

        //reset output
        memset(prVaOut1,0x00,TEST_PATTERN_WD*TEST_PATTERN_HT);

        // flush cache
        //ion_cache_sync_flush_range( ion_drv_handle);
        //ion_cache_sync_flush_range( ion_drv_handle);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgInFD, (void*)prVaIn1);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgOutFD, (void*)prVaOut1);

        MY_RESET_TIME(elapse_time);

        MY_GET_TIME(&start_time, NULL);

        vpu_enque_histogram(i4ImgInFD, i4ImgOutFD, TEST_PATTERN_WD, TEST_PATTERN_HT);

        MY_GET_TIME(&end_time, NULL);
        MY_DIFF_TIME(&temp_time, start_time, end_time);
        MY_ADD_TIME(elapse_time[0], temp_time);
        MY_DISPLAY_TIME(elapse_time[0] , "[HIST] vpu_enque_hist   ");
        MY_DISPLAY_TIME(elapse_time[1] , "[HIST] - createInstance");
        MY_DISPLAY_TIME(elapse_time[2] , "[HIST] - getAlgo       ");
        MY_DISPLAY_TIME(elapse_time[3] , "[HIST] - Acquire       ");
        MY_DISPLAY_TIME(elapse_time[4] , "[HIST] - add buffer    ");
        MY_DISPLAY_TIME(elapse_time[5] , "[HIST] - set property  ");
        MY_DISPLAY_TIME(elapse_time[6] , "[HIST] - enque         ");
        MY_DISPLAY_TIME(elapse_time[7] , "[HIST] - deque         ");
        MY_DISPLAY_TIME(elapse_time[8] , "[HIST] - release       ");

        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgInFD, prVaIn1);
        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgOutFD, prVaOut1);

#undef TEST_PATTERN_WD
#undef TEST_PATTERN_HT
#undef SIZE_IMAGE
#undef TEST_PATTERN
    }
    else if(i4Case == 2)
    {
#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080
#define GAUSSIAN_PAD 2
#define SOBEL_PAD 1
#define NMS_PAD 1
#define SIZE_IMAGE (IMG_WIDTH*IMG_HEIGHT)
#define PAD_WIDTH (GAUSSIAN_PAD+SOBEL_PAD+NMS_PAD)
#define TEST_PATTERN "data/input_1920x1080_400.yuv"
        int i4ImgInFD;
        int i4ImgWkFD;
        int i4ImgOutFD;

        //load file
        _vpu_get_buffer(ion_drv_handle, (IMG_WIDTH + 2*PAD_WIDTH)*(IMG_HEIGHT + 2*PAD_WIDTH)* sizeof(char), &i4ImgInFD, &prVaIn1);
        _vpu_get_buffer(ion_drv_handle, (IMG_WIDTH + PAD_WIDTH)*(IMG_HEIGHT + PAD_WIDTH)* sizeof(char), &i4ImgWkFD, &prVaIn2);
        _vpu_get_buffer(ion_drv_handle, (IMG_WIDTH)*(IMG_HEIGHT)* sizeof(char), &i4ImgOutFD, &prVaOut1);

        sprintf(i1FilePath, TEST_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaOut1, (IMG_WIDTH)*(IMG_HEIGHT)*sizeof(char));
        padding_dup(prVaOut1, prVaIn1, IMG_WIDTH, IMG_HEIGHT, PAD_WIDTH);

        //reset output
        memset(prVaIn2,0x00,(IMG_WIDTH + PAD_WIDTH)*(IMG_HEIGHT + PAD_WIDTH));
        memset(prVaOut1,0x00,(IMG_WIDTH)*(IMG_HEIGHT));

        // flush cache
        //ion_cache_sync_flush_range( ion_drv_handle);
        //ion_cache_sync_flush_range( ion_drv_handle);
        //ion_cache_sync_flush_range( ion_drv_handle);
        _vpu_flush_buf_by_range(ion_drv_handle, (IMG_WIDTH + 2*PAD_WIDTH)*(IMG_HEIGHT + 2*PAD_WIDTH)* sizeof(char), i4ImgInFD, (void*)prVaIn1);
        _vpu_flush_buf_by_range(ion_drv_handle, (IMG_WIDTH + PAD_WIDTH)*(IMG_HEIGHT + PAD_WIDTH)* sizeof(char),     i4ImgWkFD, (void*)prVaIn2);
        _vpu_flush_buf_by_range(ion_drv_handle, (IMG_WIDTH)*(IMG_HEIGHT)* sizeof(char),                             i4ImgOutFD,(void*)prVaOut1);

        for( i=0; i<i4Run; i++)
        {
            MY_RESET_TIME(elapse_time);

            MY_GET_TIME(&start_time, NULL);

            vpu_enque_canny(i4ImgInFD, i4ImgWkFD, i4ImgOutFD, IMG_WIDTH, IMG_HEIGHT,PAD_WIDTH);

            MY_GET_TIME(&end_time, NULL);
            MY_DIFF_TIME(&temp_time, start_time, end_time);
            MY_ADD_TIME(elapse_time[0], temp_time);

            MY_DISPLAY_TIME(elapse_time[0] , "[CANNY] vpu_enque_canny   ");
            MY_DISPLAY_TIME(elapse_time[1] , "[CANNY] - createInstance");
            MY_DISPLAY_TIME(elapse_time[2] , "[CANNY] - getAlgo       ");
            MY_DISPLAY_TIME(elapse_time[3] , "[CANNY] - Acquire       ");
            MY_DISPLAY_TIME(elapse_time[4] , "[CANNY] - add buffer    ");
            MY_DISPLAY_TIME(elapse_time[5] , "[CANNY] - set property  ");
            MY_DISPLAY_TIME(elapse_time[6] , "[CANNY] - enque         ");
            MY_DISPLAY_TIME(elapse_time[7] , "[CANNY] - deque         ");
            MY_DISPLAY_TIME(elapse_time[8] , "[CANNY] - release       ");
        }

        printf("imgWK: write file\n");

        sprintf(i1FilePath, "data/imgWK_test_out.bin");
        printf("Write imgWK_test_out.bin: %s\n", i1FilePath);
        writeImage(i1FilePath, prVaIn2, (IMG_WIDTH + PAD_WIDTH)*(IMG_HEIGHT + PAD_WIDTH)*sizeof(char));

        printf("imgOut: write file\n");

        sprintf(i1FilePath, "data/imgOut_test_out.bin");
        printf("Write imgOut_test_out.bin: %s\n", i1FilePath);
        writeImage(i1FilePath, prVaOut1, (IMG_WIDTH )*(IMG_HEIGHT )*sizeof(char));

        _vpu_free_buffer(ion_drv_handle, (IMG_WIDTH + 2*PAD_WIDTH)*(IMG_HEIGHT + 2*PAD_WIDTH)* sizeof(char), i4ImgInFD, prVaIn1);
        _vpu_free_buffer(ion_drv_handle, (IMG_WIDTH + PAD_WIDTH)*(IMG_HEIGHT + PAD_WIDTH)* sizeof(char), i4ImgWkFD, prVaIn2);
        _vpu_free_buffer(ion_drv_handle, (IMG_WIDTH)*(IMG_HEIGHT)* sizeof(char), i4ImgOutFD, prVaOut1);

#undef IMG_WIDTH
#undef IMG_HEIGHT
#undef GAUSSIAN_PAD
#undef SOBEL_PAD
#undef NMS_PAD
#undef SIZE_IMAGE
#undef PAD_WIDTH
#undef TEST_PATTERN
    }
    else if(i4Case == 3)
    {
#define TEST_PATTERN_WD (1920)
#define TEST_PATTERN_HT (1080)
#define SIZE_IMAGE (TEST_PATTERN_WD*TEST_PATTERN_HT)
#define TEST_PATTERN "data/input_1920x1080_400.yuv"
#define BITTRUE_PATTERN "data/Output_1920x1080__5.raw"

        int i4ImgInFD;
        int i4ImgOutFD;

        //load file
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), &i4ImgInFD, &prVaIn1);
        _vpu_get_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), &i4ImgOutFD, &prVaOut1);

        sprintf(i1FilePath, TEST_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaIn1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));

        //reset output
        memset(prVaOut1,0x00,TEST_PATTERN_WD*TEST_PATTERN_HT);

        // flush cache
        //ion_cache_sync_flush_range( ion_drv_handle);
        //ion_cache_sync_flush_range( ion_drv_handle);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgInFD,   (void*)prVaIn1);
        _vpu_flush_buf_by_range(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgOutFD, (void*)prVaOut1);

        //_vpu_flush_buf(ion_drv_handle, i4ImgInFD);
        //_vpu_flush_buf(ion_drv_handle, i4ImgOutFD);

        for( i=0; i<i4Run; i++)
        {
            MY_RESET_TIME(elapse_time);

            MY_GET_TIME(&start_time, NULL);

            vpu_enque_histogram_data_partition(i4ImgInFD, i4ImgOutFD, TEST_PATTERN_WD, TEST_PATTERN_HT, 1);

            MY_GET_TIME(&end_time, NULL);
            MY_DIFF_TIME(&temp_time, start_time, end_time);
            MY_ADD_TIME(elapse_time[0], temp_time);

            MY_DISPLAY_TIME(elapse_time[0] , "[HIST] vpu_enque_histogram_data_partition ");
            MY_DISPLAY_TIME(elapse_time[1] , "[HIST] - createInstance");
            MY_DISPLAY_TIME(elapse_time[2] , "[HIST] - getAlgo       ");
            MY_DISPLAY_TIME(elapse_time[3] , "[HIST] - Acquire       ");
            MY_DISPLAY_TIME(elapse_time[4] , "[HIST] - add buffer    ");
            MY_DISPLAY_TIME(elapse_time[5] , "[HIST] - set property  ");
            MY_DISPLAY_TIME(elapse_time[6] , "[HIST] - XXXXX         ");
            MY_DISPLAY_TIME(elapse_time[7] , "[HIST] - enque + deque ");
            MY_DISPLAY_TIME(elapse_time[8] , "[HIST] - release       ");
        }

        //verify
        char* pBuffer = (char*)malloc((TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));
        sprintf(i1FilePath, BITTRUE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBuffer, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));

        if (memcmp(pBuffer, prVaOut1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char)) == 0)
        {
            printf("hitogram: Pass\n");
        }
        else
        {
            printf("hitogram: Fail\n");

            sprintf(i1FilePath, "data/imgOut_hist_out.bin");
            printf("Write imgOut_hist_out.bin: %s\n", i1FilePath);
            writeImage(i1FilePath, prVaOut1, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)*sizeof(char));
        }
        free(pBuffer);

        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgInFD, prVaIn1);
        _vpu_free_buffer(ion_drv_handle, (TEST_PATTERN_WD)*(TEST_PATTERN_HT)* sizeof(char), i4ImgOutFD, prVaOut1);
        printf("free buffer done\n");

#undef TEST_PATTERN_WD
#undef TEST_PATTERN_HT
#undef SIZE_IMAGE
#undef TEST_PATTERN
#undef BITTRUE_PATTERN
    }
    else if(i4Case == 4)
    {
        //task partition
        //Core0: PreGFDown, Core1: PreGFPart1

#define SIZE_PROC_PARAM (38224)
#define PREGF_DOWN_WD (4240)
#define PREGF_DOWN_HT (2398)
#define BIT_RESOLUTION (2)
#define TEST_PATTERN "data/BayerW.bin"
#define TILE_PATTERN "data/pregfastds_out_procParam.bin"
#define BIT_TRUE_PATTERN "data/out_wdown.raw"

#define PART1_TEST_PATTERN_A "data/BayerW_Down_In.bin"
#define PART1_TEST_PATTERN_B "data/BayerWVar.bin"
#define PART1_TILE_PATTERN "data/pregfastp1_out_procParam.bin"
#define PART1_BIT_TRUE_PATTERN_A "data/out_A.raw"
#define PART1_BIT_TRUE_PATTERN_B "data/out_B.raw"

        int i4DownInFD[MAX_TASK_NUM];
        int i4DownTileFD[MAX_TASK_NUM];
        int i4DownOutFD[MAX_TASK_NUM];
        int i4Part1In0FD[MAX_TASK_NUM];
        int i4Part1In1FD[MAX_TASK_NUM];
        int i4Part1TileFD[MAX_TASK_NUM];
        int i4Part1Out0FD[MAX_TASK_NUM];
        int i4Part1Out1FD[MAX_TASK_NUM];

        UINT8 * prVaI[MAX_TASK_NUM];
        UINT8 * prVaO[MAX_TASK_NUM];
        UINT8 * prVaT[MAX_TASK_NUM];
        UINT8 * prPart1VaIn0[MAX_TASK_NUM];
        UINT8 * prPart1VaIn1[MAX_TASK_NUM];
        UINT8 * prPart1VaOut0[MAX_TASK_NUM];
        UINT8 * prPart1VaOut1[MAX_TASK_NUM];
        UINT8 * prPart1VaT[MAX_TASK_NUM];

        if(i4Run > MAX_TASK_NUM) i4Run = MAX_TASK_NUM;
        if(i4Run < 1 ) i4Run = 1;

        printf("i4Run = %d\n",i4Run);

        //load but-true file and input data
        for(i=0;i<i4Run;i++)
        {
            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD)*(PREGF_DOWN_HT)*BIT_RESOLUTION* sizeof(char), &(i4DownInFD[i]), &(prVaI[i]));
            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), &(i4DownOutFD[i]), &(prVaO[i]));
            _vpu_get_buffer(ion_drv_handle, SIZE_PROC_PARAM* sizeof(char), &(i4DownTileFD[i]), &(prVaT[i]));

            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), &(i4Part1In0FD[i]), &(prPart1VaIn0[i]));
            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), &(i4Part1In1FD[i]), &(prPart1VaIn1[i]));
            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), &(i4Part1Out0FD[i]), &(prPart1VaOut0[i]));
            _vpu_get_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), &(i4Part1Out1FD[i]), &(prPart1VaOut1[i]));
            _vpu_get_buffer(ion_drv_handle, SIZE_PROC_PARAM* sizeof(char), &(i4Part1TileFD[i]), &(prPart1VaT[i]));
        }

        sprintf(i1FilePath, TEST_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaI[0], (PREGF_DOWN_WD)*(PREGF_DOWN_HT)*BIT_RESOLUTION*sizeof(char));
        printf("Check input data: %d,%d,%d,%d", *((unsigned short*)prVaI[0]),*(((unsigned short*)prVaI[0])+1),*(((unsigned short*)prVaI[0])+2),*(((unsigned short*)prVaI[0])+3));

        sprintf(i1FilePath, TILE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaT[0], SIZE_PROC_PARAM*sizeof(char));

        sprintf(i1FilePath, PART1_TEST_PATTERN_A);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prPart1VaIn0[0],(PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));

        sprintf(i1FilePath, PART1_TEST_PATTERN_B);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prPart1VaIn1[0],(PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));

        sprintf(i1FilePath, PART1_TILE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prPart1VaT[0], SIZE_PROC_PARAM*sizeof(char));

        for(i=1;i<i4Run;i++)
        {
            memcpy(prVaI[i], prVaI[0], (PREGF_DOWN_WD)*(PREGF_DOWN_HT)*BIT_RESOLUTION*sizeof(char));
            memcpy(prVaT[i], prVaT[0], SIZE_PROC_PARAM*sizeof(char));

            memcpy(prPart1VaIn0[i], prPart1VaIn0[0], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
            memcpy(prPart1VaIn1[i], prPart1VaIn1[0], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
            memcpy(prPart1VaT[i], prPart1VaT[0], SIZE_PROC_PARAM*sizeof(char));
        }

        //debug
        sprintf(i1FilePath, "data/test_out_8.bin");
        printf("Debug OUT Img: %s\n", i1FilePath);
        writeImage(i1FilePath, prVaI[8], (PREGF_DOWN_WD)*(PREGF_DOWN_HT)* BIT_RESOLUTION*sizeof(char));

        // flush cache
        for(i=0;i<i4Run;i++)
        {
            //ion_cache_sync_flush_range( i4DownInFD[i]);
            //skip
            //_vpu_flush_buf(ion_drv_handle,i4DownInFD[i]);
            //_vpu_flush_buf(ion_drv_handle,i4DownOutFD[i]);
            _vpu_flush_buf_by_range(ion_drv_handle, (PREGF_DOWN_WD)*(PREGF_DOWN_HT)*BIT_RESOLUTION* sizeof(char),     (i4DownInFD[i]),  (void*)(prVaI[i]));
            _vpu_flush_buf_by_range(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4DownOutFD[i]), (void*)(prVaO[i]));

        }

        MY_RESET_TIME(elapse_time);

        MY_GET_TIME(&start_time, NULL);

        vpu_enque_pregf_task_partition(i4DownInFD, i4DownOutFD, i4DownTileFD,
                                        i4Part1In0FD, i4Part1In1FD, i4Part1Out0FD, i4Part1Out1FD, i4Part1TileFD,
                                        PREGF_DOWN_WD, PREGF_DOWN_HT, BIT_RESOLUTION, SIZE_PROC_PARAM, i4Run);

        MY_GET_TIME(&end_time, NULL);
        MY_DIFF_TIME(&temp_time, start_time, end_time);
        MY_ADD_TIME(elapse_time[0], temp_time);

        MY_DISPLAY_TIME(elapse_time[0] , "[PreGF] vpu_enque_PreGF_task_partition ");
        MY_DISPLAY_TIME(elapse_time[1] , "[PreGF] - createInstance");
        MY_DISPLAY_TIME(elapse_time[2] , "[PreGF] - getAlgo       ");
        MY_DISPLAY_TIME(elapse_time[3] , "[PreGF] - Acquire       ");
        MY_DISPLAY_TIME(elapse_time[4] , "[PreGF] - add buffer    ");
        MY_DISPLAY_TIME(elapse_time[5] , "[PreGF] - set property  ");
        MY_DISPLAY_TIME(elapse_time[6] , "[PreGF] - XXXXX         ");
        MY_DISPLAY_TIME(elapse_time[7] , "[PreGF] - enque + deque ");
        MY_DISPLAY_TIME(elapse_time[8] , "[PreGF] - release       ");

        //verify
        char* pBuffer = (char*)malloc((PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
        sprintf(i1FilePath, BIT_TRUE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBuffer, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));

        for(i=0;i<i4Run;i++)
        {
            if (memcmp(pBuffer, prVaO[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char)) == 0)
            {
                printf("PreGFDown(%d): Pass\n",i);
            }
            else
            {
                printf("PreGFDown(%d): Fail\n",i);

                sprintf(i1FilePath, "data/imgPreGFDown_error_out_%d.bin",i);
                printf("Write imgPreGFDown_error_out.bin: %s\n", i1FilePath);
                writeImage(i1FilePath, prVaO[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
            }
        }
        free(pBuffer);

        //verify
        char* pBufferA= (char*)malloc((PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
        char* pBufferB= (char*)malloc((PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
        sprintf(i1FilePath, PART1_BIT_TRUE_PATTERN_A);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBufferA, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
        sprintf(i1FilePath, PART1_BIT_TRUE_PATTERN_B);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBufferB, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));

        for(i=0;i<i4Run;i++)
        {
            if (memcmp(pBufferA, prPart1VaOut0[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char)) == 0)
            {
                printf("PreGFPart1A(%d): Pass\n",i);
            }
            else
            {
                printf("PreGFPart1A(%d): Fail\n",i);

                sprintf(i1FilePath, "data/imgPreGFPart1A_error_out_%d.bin",i);
                printf("Write imgPreGFPart1_error_out.bin: %s\n", i1FilePath);
                writeImage(i1FilePath, prPart1VaOut0[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
            }
            if (memcmp(pBufferB, prPart1VaOut1[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char)) == 0)
            {
                printf("PreGFPart1B(%d): Pass\n",i);
            }
            else
            {
                printf("PreGFPart1B(%d): Fail\n",i);

                sprintf(i1FilePath, "data/imgPreGFPart1B_error_out_%d.bin",i);
                printf("Write imgPreGFPart1_error_out.bin: %s\n", i1FilePath);
                writeImage(i1FilePath, prPart1VaOut1[i], (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char));
            }
        }
        free(pBufferA);
        free(pBufferB);

        for(i=0;i<i4Run;i++)
        {
            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD)*(PREGF_DOWN_HT)*BIT_RESOLUTION* sizeof(char), (i4DownInFD[i]), (prVaI[i]));
            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4DownOutFD[i]), (prVaO[i]));
            _vpu_free_buffer(ion_drv_handle, SIZE_PROC_PARAM* sizeof(char), (i4DownTileFD[i]), (prVaT[i]));

            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4Part1In0FD[i]), (prPart1VaIn0[i]));
            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4Part1In1FD[i]), (prPart1VaIn1[i]));
            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4Part1Out0FD[i]), (prPart1VaOut0[i]));
            _vpu_free_buffer(ion_drv_handle, (PREGF_DOWN_WD/2)*(PREGF_DOWN_HT/2)* BIT_RESOLUTION*sizeof(char), (i4Part1Out1FD[i]), (prPart1VaOut1[i]));
            _vpu_free_buffer(ion_drv_handle, SIZE_PROC_PARAM* sizeof(char), (i4Part1TileFD[i]), (prPart1VaT[i]));
        }

        printf("free buffer done\n");

#undef MAX_TASK_NUM
#undef SIZE_PROC_PARAM

#undef PREGF_DOWN_WD
#undef PREGF_DOWN_HT
#undef BIT_RESOLUTION
#undef TEST_PATTERN
#undef TILE_PATTERN
#undef BIT_TRUE_PATTERN
#undef PART1_TEST_PATTERN_A
#undef PART1_TEST_PATTERN_B
#undef PART1_TILE_PATTERN
#undef PART1_BIT_TRUE_PATTERN_A
#undef PART1_BIT_TRUE_PATTERN_B
    }
    else if(i4Case == 5)
    {
        //swnr stage 2
#define IMG_WD (1560)
#define IMG_HT (2104)
#define IMG_SZ (1560*2104)
#define TEST_PATTERN0 "data/Pass1out_0.raw"
#define TEST_PATTERN1 "data/Pass1out_2.raw"
#define TEST_PATTERN2 "data/_FSWNR_weight_tbl"

#define BIT_TRUE_PATTERN "data/Pass2out_0.raw"
#define TEST_CACHE_PATTERN "data/TestCache_0.raw"

        int i4InOutFD;
        int i4InFD;
        int i4OutFD;
        int i4WeightFD;

        UINT8 * prVaInOut;
        UINT8 * prVaIn;
        UINT8 * prVaOut;
        UINT8 * prVaWeight;

        //load but-true file and input data

        VPU_Get_IonBuffer(ion_drv_handle, IMG_SZ* sizeof(char),0,0, &i4InOutFD, (void **)&prVaInOut);
        VPU_Get_IonBuffer(ion_drv_handle, IMG_SZ* sizeof(char),0,0, &i4InFD, (void **)&prVaIn);
        VPU_Get_IonBuffer(ion_drv_handle, IMG_SZ* sizeof(char),0,0, &i4OutFD, (void **)&prVaOut);
        VPU_Get_IonBuffer(ion_drv_handle, 256* sizeof(char),0,0, &i4WeightFD, (void **)&prVaWeight);

        //_vpu_get_buffer(ion_drv_handle, IMG_SZ* sizeof(char), &i4InOutFD, &prVaInOut);
        //_vpu_get_buffer(ion_drv_handle, IMG_SZ* sizeof(char), &i4InFD, &prVaIn);
        //_vpu_get_buffer(ion_drv_handle, IMG_SZ* sizeof(char), &i4OutFD, &prVaOut);
        //_vpu_get_buffer(ion_drv_handle, 256* sizeof(char), &i4WeightFD, &prVaWeight);

        sprintf(i1FilePath, TEST_PATTERN0);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaInOut, IMG_SZ*sizeof(char));
        sprintf(i1FilePath, TEST_PATTERN1);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaIn, IMG_SZ*sizeof(char));
        sprintf(i1FilePath, TEST_PATTERN2);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, prVaWeight, 255*sizeof(char));

        // flush cache
        if(i4Run == 3){
            printf("Do cache sync!\n");
            //ion_cache_sync_flush_all(ion_drv_handle);
        }
        //_vpu_flush_buf(ion_drv_handle,i4InOutFD);
        //_vpu_flush_buf(ion_drv_handle,i4InFD);
        //_vpu_flush_buf(ion_drv_handle,i4WeightFD);

        MY_RESET_TIME(elapse_time);

        MY_GET_TIME(&start_time, NULL);

        vpu_enque_swnr(i4InOutFD, i4InFD, i4OutFD, i4WeightFD, IMG_WD, IMG_HT, i4Run, prVaInOut,ion_drv_handle);

        MY_GET_TIME(&end_time, NULL);
        MY_DIFF_TIME(&temp_time, start_time, end_time);
        MY_ADD_TIME(elapse_time[0], temp_time);

        MY_DISPLAY_TIME(elapse_time[0] , "[swnr] vpu_enque_swnr ");
        MY_DISPLAY_TIME(elapse_time[1] , "[swnr] - createInstance");
        MY_DISPLAY_TIME(elapse_time[2] , "[swnr] - getAlgo       ");
        MY_DISPLAY_TIME(elapse_time[3] , "[swnr] - Acquire       ");
        MY_DISPLAY_TIME(elapse_time[4] , "[swnr] - add buffer    ");
        MY_DISPLAY_TIME(elapse_time[5] , "[swnr] - set property  ");
        MY_DISPLAY_TIME(elapse_time[6] , "[swnr] - enque         ");
        MY_DISPLAY_TIME(elapse_time[7] , "[swnr] - deque         ");
        MY_DISPLAY_TIME(elapse_time[8] , "[swnr] - release       ");

        //verify
        char* pBuffer = (char*)malloc(IMG_SZ*sizeof(char));
        sprintf(i1FilePath, BIT_TRUE_PATTERN);
        if(i4Run == 2 || i4Run == 3)
            sprintf(i1FilePath, TEST_CACHE_PATTERN);
        printf("READing Img: %s\n", i1FilePath);
        readImage(i1FilePath, pBuffer, IMG_SZ*sizeof(char));

        if (memcmp(pBuffer,prVaInOut, IMG_SZ*sizeof(char)) == 0)
        {
            printf("swnr(%d): Pass\n",i);
        }
        else
        {
            printf("swnr(%d): Fail\n",i);

            sprintf(i1FilePath, "data/imgSwnr_error_out_%d.bin",i);
            printf("Write imgSwnr_error_out_.bin: %s\n", i1FilePath);
            writeImage(i1FilePath, prVaInOut, IMG_SZ*sizeof(char));
        }

        free(pBuffer);

        _vpu_free_buffer(ion_drv_handle, IMG_SZ* sizeof(char), i4InOutFD, prVaInOut);
        _vpu_free_buffer(ion_drv_handle, IMG_SZ* sizeof(char), i4InFD, prVaIn);
        _vpu_free_buffer(ion_drv_handle, IMG_SZ* sizeof(char), i4OutFD, prVaOut);
        _vpu_free_buffer(ion_drv_handle, 256* sizeof(char), i4WeightFD, prVaWeight);

        printf("free buffer done\n");
    }

EXIT:
    if(ion_drv_handle)
        ion_close(ion_drv_handle);

    return 1;
}
