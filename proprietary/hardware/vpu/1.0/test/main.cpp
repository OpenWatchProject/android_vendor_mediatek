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

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream
#include "test/vpu_data_wpp.h"

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_TAG
#define LOG_TAG "MtkCam.vpu_test"

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

static char const *vpu_algo_name[] = {"ipu_flo_d2d_k3", "ipu_flo_d2d_k3", "ipu_flo_d2d_k7", "ipu_flo_d2d_k5"};
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

int ion_drv_handle = 0;

// image width, height, size, data length
static UINT32 gTestImgWidth   = 0;
static UINT32 gTestImgHeight  = 0;
static UINT32 gTestImgSize    = 0;
static UINT32 gTestDataLength = 0;

// save test image data pointer into global variables due to create threads
static UINT8 *pgTest_img1 = NULL, *pgTest_img2 = NULL, *pgTest_data1 = NULL;
static UINT32 multi_user_que_cnt = 2, flush_que_cnt = 0;
static UINT32 gDumpImage = 0;

enum {
  VPU_TEST_SUCCESS = 1 << 0,
  VPU_TEST_FAIL    = 1 << 1,
} ;

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
    VpuPort    *port1, *port2, *port3;
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
    vpu = VpuStream::createInstance();
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
int vpu_enque_test(UINT32 test_pattern,UINT32 test_count)
{
    UINT32      i;
    int         result = 0, img1_fd = 0, img2_fd = 0, data1_fd = 0;
    UINT8      *img1_va = NULL, *img2_va = NULL, *data1_va = NULL;
    VpuStream  *vpu  = NULL;
    VpuAlgo    *algo = NULL;
    VpuPort    *port1, *port2, *port3;
    VpuRequest *request = NULL;
    VpuBuffer   buf_img1, buf_img2, buf_data1;

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
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        goto EXIT;
    }

#if 1 // test setPower and MayPhyAddr
    uint32_t ret;
    bool bRet;
    VpuPower pwr;
    pwr.mode = ePowerModeOn;
    pwr.opp  = ePowerOppUnrequest;
    bRet = vpu->setPower(pwr);
    MY_LOGD("setPower bRet = %d", bRet);
    ret = vpuMapPhyAddr(img1_fd);
    MY_LOGD("MapPhyAddr for img1_fd,  addr = 0x%x", ret);
    ret = vpuMapPhyAddr(img2_fd);
    MY_LOGD("MapPhyAddr for img2_fd,  addr = 0x%x", ret);
    ret = vpuMapPhyAddr(data1_fd);
    MY_LOGD("MapPhyAddr for data1_fd, addr = 0x%x", ret);
#endif

    // 1.1 init algo of vpu
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
        goto EXIT;
    }

    // 2.1 prepare buffer for port1
    _vpu_setup_buf(buf_img1,img1_fd,eFormatImageY8,gTestImgWidth,gTestImgHeight,0,gTestImgWidth,gTestImgSize);
    _vpu_setup_buf(buf_img2,img2_fd,eFormatImageY8,gTestImgWidth,gTestImgHeight,0,gTestImgWidth,gTestImgSize);
    _vpu_setup_buf(buf_data1,data1_fd,eFormatData,0,0,0,0,gTestDataLength);

    // 3. setup request
    // 3.1 acquire request
    request = vpu->acquire(algo);

    // 3.2 add buffers to request
    request->addBuffer(port1, buf_img1);
    request->addBuffer(port2, buf_img2);
    request->addBuffer(port3, buf_data1);

    // 3.3 set Property to request
    {
        int param1 = 1;
        int param2 = 2;
        int param3 = 3;
        int param4 = 4;
        int param5 = 1;

        VpuProperty prop;
        prop.type = eTypeInt32;
        prop.count = 1;

        prop.data.i32 = &param1;
        request->setProperty("param1", prop);

        prop.data.i32 = &param2;
        request->setProperty("param2", prop);

        prop.data.i32 = &param3;
        request->setProperty("param3", prop);

        prop.data.i32 = &param4;
        request->setProperty("param4", prop);
    }

    MY_LOGD("start to test: test_count %d", test_count);
    for(i = 0 ; i < test_count ; i++)
    {
        char        buffer[50];
        VpuProperty prop;
        int         param5;
        VpuRequest *deque_req;
        memset((void *)img2_va , 0, gTestImgSize);                             // clean output image buffer
        memcpy((void *)data1_va, (const void *)pgTest_data1, gTestDataLength); // init test data, MUST copy again because vpu will modify params after executing d2d
        MY_LOGD("vpu->enque(0x%lx): number %d", (unsigned long)request, i);
        // 4. enqueue & dequeue
        vpu->enque(request);
        deque_req = vpu->deque();
        deque_req->getProperty("param5", prop);
        param5 = *prop.data.i32;
        MY_LOGD("vpu->deque(): request:(0x%lx), result:%d", (unsigned long)deque_req, param5);
        // 5. check result
        if(deque_req == request)
        {
            int result_cmp = memcmp((unsigned char*)img2_va,(unsigned char*)pgTest_img2,gTestImgSize);
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

EXIT:
    // 6. release resource & exit
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
            int param5 = 1;

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
        vpu[i] = VpuStream::createInstance();
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

void vpu_test_menu()
{
    MY_LOGD("##############################");
    MY_LOGD("1: enque test");
    MY_LOGD("2: multi-user test");
    MY_LOGD("3: flush queue test");
    MY_LOGD("4: HQA test");
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

    for (i = 0 ; i < AUTO_TEST_PW_PRE_ENQ_CNT ; i++)
    {
        request[i]  = NULL;
    }
    gettimeofday(&t0,NULL);
    // 1. init vpu
    vpu = VpuStream::createInstance();
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
            goto EXIT;
          }
          deq_count++;
          if (vpu->enque(deque_req))
            enq_count++;

          gettimeofday(&t2,NULL);
          if ((int)(t2.tv_sec - t1.tv_sec) >= time_exec) break;
      }

      while(deq_count != enq_count)
      {
          VpuRequest *deque_req;
          deque_req = vpu->deque();
          if (!deque_req)
            break;
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
        result != VPU_TEST_FAIL;
        goto EXIT;
    }
    // 1. init vpu
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        MY_LOGW("fail to get vpu handle");
        result != VPU_TEST_FAIL;
        goto EXIT;
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
            int param5 = 1;

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
    UINT32 testcase = 0, test_pattern = 0, test_cnt = 0, user_cnt = 2, queue_cnt =2, flush_que=0;
    int input;
    UINT32 enter_test = 1;
    int autoTestCase  = 0;
    VPU_ALGO_E test_algo = E_VPU_ALGO_WARP;
    int argc_cnt ;

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
        MY_LOGD("sizeSrc %d, sizeGolden %d, sizeParam %d", sizeof(g_datasrc_640x360_wpp), sizeof(g_datadst_640x360_golden_wpp), sizeof(g_datapp_640x360_wpp));
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

    switch(autoTestCase)
    {
        case 1:
            ret = vpu_enque_test(test_pattern, AUTO_TEST_CNT);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("test OK !!!");
            else
                MY_LOGD("test failed !!!");
            break;
        case 2:
            ret = vpu_multi_user_test(test_pattern, AUTO_TEST_USER_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_QUEUE_CNT);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("test OK !!!");
            else
                MY_LOGD("test failed !!!");
            break;
        case 3:
            ret = vpu_multi_user_test(test_pattern, AUTO_TEST_USER_CNT, AUTO_TEST_QUEUE_CNT, AUTO_TEST_FLUSH_CNT);
            if(ret == VPU_TEST_SUCCESS)
                MY_LOGD("test OK !!!");
            else
                MY_LOGD("test failed !!!");
            break;
        case 0:
            while(enter_test)
            {
                vpu_test_menu();
                scanf("%d", &input);
                testcase = input;
                switch(testcase)
                {
                    case 1:
                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
                        MY_LOGD("enque/deque test pattern:");
                        scanf("%d", &input);
                        test_pattern = input;
                        MY_LOGD("enque/deque test count:");
                        scanf("%d", &input);
                        test_cnt = input;
                        MY_LOGD("dumpImage(default is %d):", gDumpImage);
                        scanf("%d", &input);
                        gDumpImage = input;
                        ret = vpu_enque_test(test_pattern, test_cnt);
                        if(ret == VPU_TEST_SUCCESS)
                            MY_LOGD("test OK !!!");
                        else
                            MY_LOGD("test failed !!!");

                        break;

                    case 2:
                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
                        MY_LOGD("enque/deque test pattern:");
                        scanf("%d", &input);
                        test_pattern = input;
                        MY_LOGD("user count (max user cnt = %d):", MAX_MULTI_USER_CNT);
                        scanf("%d", &input);
                        user_cnt = input;
                        MY_LOGD("queue count (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
                        scanf("%d", &input);
                        queue_cnt = input;
                        MY_LOGD("dumpImage(default is %d):", gDumpImage);
                        scanf("%d", &input);
                        gDumpImage = input;
                        ret = vpu_multi_user_test(test_pattern ,user_cnt, queue_cnt, queue_cnt);
                        if(ret == VPU_TEST_SUCCESS)
                            MY_LOGD("test OK !!!");
                        else
                            MY_LOGD("test failed !!!");

                        break;

                    case 3:
                        algo_name = (char *)vpu_algo_name[E_VPU_ALGO_WARP];
                        MY_LOGD("enque/deque test pattern:");
                        scanf("%d", &input);
                        test_pattern = input;
                        MY_LOGD("user count (max user cnt = %d):", MAX_MULTI_USER_CNT);
                        scanf("%d", &input);
                        user_cnt = input;
                        MY_LOGD("queue count (max queue cnt = %d):", MAX_MULTI_USER_QUE_CNT);
                        scanf("%d", &input);
                        queue_cnt = input;
                        MY_LOGD("flush queue count (must less than queue cnt = %d):", queue_cnt);
                        scanf("%d", &input);
                        flush_que = input;
                        MY_LOGD("dumpImage(default is %d):", gDumpImage);
                        scanf("%d", &input);
                        gDumpImage = input;
                        if(flush_que >= queue_cnt)
                            flush_que = queue_cnt - 1;
                        ret = vpu_multi_user_test(test_pattern, user_cnt, queue_cnt, flush_que);
                        if(ret == VPU_TEST_SUCCESS)
                            MY_LOGD("test OK !!!");
                        else
                            MY_LOGD("test failed !!!");

                        break;
                    case 4:
                    {
                        int time_exec, time_sleep, loop;

                        MY_LOGD("HQA test pattern: (0: MaxPower, 1: Warping)");
                        scanf("%d", &input);
                        test_algo = input == 0 ? E_VPU_ALGO_MAX_PW : E_VPU_ALGO_WARP ;
                        algo_name   = (char *)vpu_algo_name[test_algo];
                        MY_LOGD("time to exec (secs)(max: 12 hr, min: 10 secs):");
                        scanf("%d", &input);
                        time_exec = input;
                        MY_LOGD("time to sleep after executing (secs)(max: 1 hr, min: 1 secs):");
                        scanf("%d", &input);
                        time_sleep = input;
                        MY_LOGD("loop count (max: 10000, min: 1):");
                        scanf("%d", &input);
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
                        if(ret == VPU_TEST_SUCCESS)
                            MY_LOGD("test OK !!!");
                        else
                            MY_LOGD("test failed !!!");
                        break;
                    }

                    case 9:
                        enter_test = 0;
                        break;

                    default:
                        break;
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
