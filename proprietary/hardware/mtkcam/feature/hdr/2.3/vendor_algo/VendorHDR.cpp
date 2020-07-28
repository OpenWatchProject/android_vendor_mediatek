/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#define DEBUG_LOG_TAG "VendorUTLS"

#include "VendorHDR.h"

#include <thread>
#include <vector>

#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>

#include <mtkcam/drv/iopipe/SImager/ISImager.h>

#include <mtkcam/feature/hdr/utils/Debug.h>
#include <mtkcam/feature/utils/ImageBufferUtils.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/feature/FaceDetection/fd_hal_base.h>


// Android UTILS
#include <utils/ThreadDefs.h>

// CUSTOM
#include <aaa/ae_param.h>

// STD
#include <math.h>

using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NS3Av3;
using namespace NSCam::Utils;
using namespace NSCamHW;


// round-robin time-sharing policy
#define THREAD_POLICY     SCHED_OTHER
// most threads run at normal priority
#define THREAD_PRIORITY   ANDROID_PRIORITY_NORMAL

// 0: 8bits; 1: 12 bits
#define HDR_AE_12BIT_FLARE 1

#define L1_CACHE_BYTES 32

//#define HDR_DEBUG_SKIP_MODIFY_POLICY

#define ALIGN_FLOOR(x,a)  ((x) & ~((a) - 1L))
#define ALIGN_CEIL(x,a)   (((x) + (a) - 1L) & ~((a) - 1L))

//VPU define and include
//MT6771
#ifdef FAKEHDR_ENABLE

//Dng Header
#include <MTKDngOp.h>
#include <mtkcam/aaa/IDngInfo.h>


#include <sys/mman.h>
#include <vpu.h>                        // interface for vpu stream
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#define AUTO_TEST_TASK_PARTITION_NUM (25)
#define MY_GET_TIME(...)
#define MY_DIFF_TIME(...)
#define MY_DISPLAY_TIME(...)
#define MY_RESET_TIME(...)
#define MY_ADD_TIME(...)
typedef struct vpu_st_sett_histogram {
    int S32_Scale;
    int S32_Offset;
};
static char const *vpu_algo_name[] = {"vpu_lib_d2d_histogram"};

/*
VPU Stream example code
*/
static void _vpu_setup_buf(VpuBuffer &buf, MINT32 buf_fd, VpuBufferFormat format,
                           MUINT32 width,  MUINT32 height, MUINT32 buf_offset,
                           MUINT32 stride, MUINT32 length)
{
    HDR_LOGD("_vpu_setup_buf:%d,%d,%d,%d,%d,%d,%d",buf_fd,width,height,buf_offset,stride,length,format);
    buf.format      = format;
    buf.width       = width;
    buf.height      = height;
    buf.planeCount  = 1;

    /* ion case */
    buf.planes[0].fd      = (MUINT32)buf_fd;
    buf.planes[0].offset  = buf_offset;
    buf.planes[0].stride  = stride;
    buf.planes[0].length  = length;
}
static MINT32 vpu_enque_histogram_data_partition(
    MINT32 i4ImgInFD,MINT32 i4ImgOutFD,MINT32  width,MINT32 height,MINT32 loop,
    MUINT32  w0,MUINT32 h0,MUINT32 s0, MUINT32 size0,MINT32 f0,
    MUINT32  w1,MUINT32 h1,MUINT32 s1, MUINT32 size1,MINT32 f1)
{
    VpuStream  *vpu     = NULL;
    VpuAlgo    *algo    = NULL;
    VpuPort    *port1, *port2, *port3;
    VpuBuffer   buf_img1, buf_img2;
    VpuRequest *request[AUTO_TEST_TASK_PARTITION_NUM];
    VpuProperty prop;
    vpu_st_sett_histogram m_property;
    VpuRequest *deque_req = NULL;
    VpuExtraParam extra_param;
    char *algo_name   =  "vpu_lib_d2d_histogram";

    MINT32 param1;
    MINT32 result, i, j, enq_count = 0, deq_count = 0, cnt = 0;

    std::vector<VpuStatus> vpu_status;

    MBOOL res=false;

    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        request[i]  = NULL;
    }

    // 1. init vpu
    MY_GET_TIME(&sub_start_time, NULL);
    vpu = VpuStream::createInstance();
    if (vpu == NULL)
    {
        HDR_LOGW("fail to get vpu handle");
        goto EXIT;
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[1], temp_time);

    // 1.1 init algo of vpu
    MY_GET_TIME(&sub_start_time, NULL);
    algo_name = (char *)vpu_algo_name[0];
    algo = vpu->getAlgo(algo_name);
    if (algo == NULL)
    {
        HDR_LOGW("fail to get vpu algo handle");
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
            HDR_LOGW("fail to get vpu request[%d] !!!",i);
            goto EXIT;
        }
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[3], temp_time);

    // 3. setup request
    MY_GET_TIME(&sub_start_time, NULL);
    buf_img1.port_id = 0;
    _vpu_setup_buf(buf_img1, i4ImgInFD, eFormatImageY8, (MUINT32)w0, (MUINT32)h0, 0, (MUINT32)s0, size0);

    buf_img2.port_id = 1;
    _vpu_setup_buf(buf_img2, i4ImgOutFD, eFormatImageY8, (MUINT32)w1, (MUINT32)h1, 0, (MUINT32)s1, size1);
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++){
        request[i]->addBuffer( buf_img1);
        request[i]->addBuffer( buf_img2);
    }

    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[4], temp_time);

    MY_GET_TIME(&sub_start_time, NULL);
    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        int working_height = ((height/AUTO_TEST_TASK_PARTITION_NUM)/30)*30;
        m_property.S32_Scale = i*working_height;
        m_property.S32_Offset = (i+1)*working_height;
        res = request[i]->setProperty((void*)&m_property, sizeof(vpu_st_sett_histogram));
        if(res == false)
        {
            HDR_LOGD("Error: set property fail!");
            goto EXIT;
        }
    }
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[5], temp_time);

    res = vpu->getVpuStatus(vpu_status);
    if(res == false)
    {
        HDR_LOGD("Error: vpu_status!");
        goto EXIT;
    }

    extra_param.opp_step = 0;
    extra_param.bw = 45; //Mbytes/sec

    for (i = 0 ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
    {
        request[i]-> setExtraParam(extra_param);
        HDR_LOGD("vpu->enque[%d](0x%lx): number %d",i, (unsigned long)request[i], 0);
    }
    // 4. enqueue & dequeue
    MY_GET_TIME(&sub_start_time, NULL);

    enq_count = 0;
    deq_count = 0;

    while(1)
    {
        for (i = enq_count ; i < AUTO_TEST_TASK_PARTITION_NUM ; i++)
        {
            if (vpu->enque(request[i])) enq_count++;
        }
        HDR_LOGD("enque_count = %d",enq_count);

        while(1)
        {
            deque_req = vpu->deque();
            HDR_LOGD("deque_req = (0x%lx) %d",(unsigned long)deque_req,deq_count);

            if (!deque_req || deque_req != request[deq_count])
            {
                HDR_LOGD("fail to dequeue deque_req(0x%lx) != request[%d] (0x%lx)", (unsigned long)deque_req, deq_count, (unsigned long)request[deq_count]);
                break;
            }

            deq_count++;
            if(deq_count == enq_count)
                break;
        }

        //Check if every task is enqued successfully
        if(enq_count == AUTO_TEST_TASK_PARTITION_NUM)
            break;
    }

EXIT:
    MY_GET_TIME(&sub_end_time, NULL);
    MY_DIFF_TIME(&temp_time, sub_start_time, sub_end_time);
    MY_ADD_TIME(elapse_time[7], temp_time);
    HDR_LOGD("vpu->deque(): request:(0x%lx)", (unsigned long)deque_req);

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

    HDR_LOGI("test finish");

    return 0;
}

#endif //MTK_FAKEHDR_ENABLE
// ---------------------------------------------------------------------------

sp<IHDR>
createInstance_VendorHDR(char const * const pszShotName,
            uint32_t const u4ShotMode, int32_t const i4OpenId)
{
    return new NSCam::VendorHDR(pszShotName, u4ShotMode, i4OpenId);
}

static MBOOL setThreadProp(int policy, int priority)
{
#ifndef HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    //int const policy    = pthreadAttr_ptr->sched_policy;
    //int const priority  = pthreadAttr_ptr->sched_priority;
    //HDR_LOGD("policy=%d, priority=%d", policy, priority);

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    switch (policy)
    {
        // non-real-time
        case SCHED_OTHER:
            sched_p.sched_priority = 0;
            sched_setscheduler(0, policy, &sched_p);
            // -20(high)~19(low)
            setpriority(PRIO_PROCESS, 0, priority);
            break;

        // real-time
        case SCHED_FIFO:
        default:
            // -20(high)~19(low)
            sched_p.sched_priority = priority;
            sched_setscheduler(0, policy, &sched_p);
    }
#endif

    return MTRUE;
}

static MBOOL getThreadProp(int* policy, int* priority)
{
#ifndef HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    struct sched_param sched_p;
    *policy = ::sched_getscheduler(0);

    switch (*policy)
    {
        // non-real-time
        case SCHED_OTHER:
            // -20(high)~19(low)
            *priority = getpriority(PRIO_PROCESS, 0);
            break;

        // real-time
        case SCHED_FIFO:
        default:
            struct sched_param sched_p;
            ::sched_getparam(0, &sched_p);
            *priority = sched_p.sched_priority;
    }
#endif

    return MTRUE;
}

// ---------------------------------------------------------------------------

VendorHDR::VendorHDR(char const * const /*pszShotName*/,
        uint32_t const u4ShotMode, int32_t const i4OpenId)
  : mInputFramesNum(HDR_MAX_INPUT_FRAME),
    mSourceW(0),
    mSourceH(0),
    mOpenID(i4OpenId),
    mHDRShotMode(u4ShotMode),
    mSensorType(SENSOR_TYPE_UNKNOWN),
    mCompleteCB(NULL),
    mpCompleteCBUser(NULL),
    mCapturePolicy(SCHED_OTHER),
    mCapturePriority(ANDROID_PRIORITY_NORMAL),
    mforceBreak(MFALSE),
    mDebugMode(0),
    mLuxIndex(0),
    mScene(0)
{
    //init shotParam
    memset(&mShotParam, 0, sizeof(HDRProc_ShotParam));

    // Hal3A
    mHal3A = std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
    (
        MAKE_Hal3A(i4OpenId, DEBUG_LOG_TAG),
        [](IHal3A* p){ if (p) p->destroyInstance(DEBUG_LOG_TAG); }
    );

    mDebugMode = property_get_int32("vendor.mediatek.hdr.debug", 0);

    // initialize semaphores
    for (MINT32 i = 0; i < HDR_MAX_INPUT_FRAME; i++)
    {
        memset(&mSourceImgBufSem[i], 0, sizeof(sem_t));
        memset(&mSourceImgBufRaw16Sem[i], 0, sizeof(sem_t));
        sem_init(&mSourceImgBufSem[i], 0, 0);
        sem_init(&mSourceImgBufRaw16Sem[i], 0, 0);
    }
}

VendorHDR::~VendorHDR()
{
    uninit();
}

MBOOL VendorHDR::uninit()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    FUNCTION_LOG_END;
    return  ret;
}

// ---------------------------------------------------------------------------

MBOOL VendorHDR::updateInfo()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.mediatek.hdr.debug", value, "0");
    mDebugMode = atoi(value);
    HDR_LOGD_IF(mDebugMode == 1, "[updateInfo] debug mode ON");

    // make debug dump path
    if (mDebugMode && !makePath(HDR_DUMP_PATH, 0660))
    {
        HDR_LOGW("make debug dump path %s failed", HDR_DUMP_PATH);
    }

    // show capture policy & priority
    getThreadProp(&mCapturePolicy, &mCapturePriority);
    HDR_LOGD("[updateInfo] capture policy(%d) priority(%d)",
            mCapturePolicy, mCapturePriority);

    // set YUV buffer size
    mSourceW = mShotParam.pictureSize.w;
    mSourceH = mShotParam.pictureSize.h;
    HDR_LOGD("[updateInfo] HDR picture size(%dx%d)", mSourceW, mSourceH);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::prepare()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if (mforceBreak)
    {
        HDR_LOGD("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    ret = init();

    // Get Lux index
    {
        ASDInfo_T ASDInfo;
        MINT32    ae_lv = 0;
        double    temp = 0;

        mHal3A->send3ACtrl( NS3Av3::E3ACtrl_GetAsdInfo,
            reinterpret_cast<MINTPTR>(&ASDInfo), 0);

        ae_lv = ASDInfo.i4AELv_x10;
        temp = ae_lv/10 - 3;

        // BV convert to lux index, lux index = 2^ (i4AELv_x10/10 -3)
        mLuxIndex = pow(2, temp);

        if(mLuxIndex > 128) {
            mScene = HDR_Bright;
        }
        else if(mLuxIndex > 4) {
            mScene = HDR_Normal;
        }
        else {
            mScene = HDR_Dark;
        }

        HDR_LOGD("Lux index(%f), Scene(%d)", mLuxIndex, mScene);


    }

    //Get Black level (OB offest) nvram range is 0~8192
    {
        MINT32 tmp[4] = {0, 0, 0, 0};
        mHal3A->send3ACtrl(E3ACtrl_GetOBOffset, reinterpret_cast<MINTPTR>(tmp), 0);

        for(int i = 0; i < 4; i++)
        {
            mBlackLevel[i] = (float)(8192 - tmp[i]) / 4;
        }

        HDR_LOGD("Black level(%d, %d, %d, %d)", mBlackLevel[0], mBlackLevel[1], mBlackLevel[2], mBlackLevel[3]);
    }

    //Face rect range
 #if 0
    {
        halFDBase* fdObject = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
        MtkCameraFaceMetadata *fd_info_result = new MtkCameraFaceMetadata;
        MtkCameraFace *faces = new MtkCameraFace[15];
        MtkFaceInfo *info = new MtkFaceInfo[15];

        fd_info_result->faces = faces;
        fd_info_result->posInfo = info;

        if(fdObject) {
            fdObject->halFDGetFaceInfo(fd_info_result);
            HDR_LOGD("Face number(%d)", fd_info_result->number_of_faces);

            for(int i=0; i < fd_info_result->number_of_faces;i++)
            {
                memcpy(mRect, fd_info_result->faces[i].rect, sizeof(mRect));
                HDR_LOGD("Face rect(%d, %d, %d, %d)", mRect[0]
                                                    , mRect[1]
                                                    , mRect[2]
                                                    , mRect[3]);
            }
            fdObject->destroyInstance();
            fdObject = nullptr;
            delete faces;
            delete info;
        }
        else
        HDR_LOGE("fdObject is null");
    }
#endif
    // create HDR working buffer asynchronously
    allocateProcessMemory();

    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::addInputFrame(
        MINT32 frameIndex, const android::sp<IImageBuffer>& inBuffer)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if ((frameIndex < 0) || (frameIndex > mInputFramesNum))
    {
        HDR_LOGE("[addInputFrame] invalid frameIndex(%d)", frameIndex);
        return MFALSE;
    }

    if (inBuffer == nullptr)
    {
        HDR_LOGE("[addInputFrame] inBuffer is NULL");
        return MFALSE;
    }

    mpSourceImgBuf[frameIndex] = inBuffer;
    sem_post(&mSourceImgBufSem[frameIndex]);

    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::process()
{
    // a workter thread that process HDR post-processing
    mProcessThread = std::async(std::launch::async, VendorHDR::hdrProcess, this);

    return MTRUE;
}

MVOID VendorHDR::hdrProcess(MVOID* arg)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    VendorHDR *self = static_cast<VendorHDR *>(arg);

    // set thread's name
    ::prctl(PR_SET_NAME, "HDRWorker", 0, 0, 0);
    HDR_LOGD("[hdrProcess] setThreadProp");
    setThreadProp(SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND);

    // wait for HDR input buffers
    self->waitInputFrame();
#if 0
    //Insert VPU Code here
    MUINT8 *prVaIn1, *prVaOut1;
    MINT32 i4ImgInFD,i4ShareInFD;
    MINT32 i4ImgOutFD,i4ShareOutFD;
    MSize  imgSize0,imgSize1;
    MUINT32 imgStride0, imgStride1;
    MUINT32 imgSizeBytes0, imgSizeBytes1;
    MINT32 imgFormat0,imgFormat1;
    MUINT32 imgPlaneCnt0, imgPlaneCnt1;
    MUINT32 imgFDCnt0, imgFDCnt1;
    MUINT32 imgOffset0, imgOffset1;

    prVaIn1 = (MUINT8 *)self->mpSourceImgBuf[0]->getBufVA(0);
    prVaOut1 = (MUINT8 *)self->mpSourceImgBuf[2]->getBufVA(0);
    i4ImgInFD = (MINT32)self->mpSourceImgBuf[0]->getFD(0);
    i4ImgOutFD = (MINT32)self->mpSourceImgBuf[2]->getFD(0);
    imgSize0 = self->mpSourceImgBuf[0]->getImgSize();
    imgSize1 = self->mpSourceImgBuf[2]->getImgSize();
    imgStride0 = (MUINT32)self->mpSourceImgBuf[0]->getBufStridesInBytes(0);
    imgStride1 = (MUINT32)self->mpSourceImgBuf[2]->getBufStridesInBytes(0);
    imgSizeBytes0 = (MUINT32)self->mpSourceImgBuf[0]->getBufSizeInBytes(0);
    imgSizeBytes1 = (MUINT32)self->mpSourceImgBuf[2]->getBufSizeInBytes(0);
    imgFormat0 = self->mpSourceImgBuf[0]->getImgFormat() ;
    imgFormat1 = self->mpSourceImgBuf[2]->getImgFormat() ;
    imgPlaneCnt0 = (MUINT32)self->mpSourceImgBuf[0]->getPlaneCount();
    imgPlaneCnt1 = (MUINT32)self->mpSourceImgBuf[2]->getPlaneCount();
    imgFDCnt0 = (MUINT32)self->mpSourceImgBuf[0]->getFDCount();
    imgFDCnt1 = (MUINT32)self->mpSourceImgBuf[2]->getFDCount();
    imgOffset0 = (MUINT32)self->mpSourceImgBuf[0]->getBufOffsetInBytes(0);
    imgOffset1 = (MUINT32)self->mpSourceImgBuf[2]->getBufOffsetInBytes(0);

    //Check image information
    HDR_LOGD("Img0, FD = %d, width = %d, height = %d, stride = %d, size = %d, format = %d, planeCnt = %d, FDCnt  = %d, offset = %d",i4ImgInFD,imgSize0.w,imgSize0.h,imgStride0,imgSizeBytes0,imgFormat0,imgPlaneCnt0,imgFDCnt0,imgOffset0);
    HDR_LOGD("Img1, FD = %d, width = %d, height = %d, stride = %d, size = %d, format = %d, planeCnt = %d, FDCnt  = %d, offset = %d",i4ImgOutFD,imgSize1.w,imgSize1.h,imgStride1,imgSizeBytes1,imgFormat1,imgPlaneCnt1,imgFDCnt1,imgOffset1);

    //FD from getFD can be used directly to VPU stream
    i4ShareInFD = i4ImgInFD;
    i4ShareOutFD = i4ImgOutFD;

    //Check log
    HDR_LOGD("VPU Pre-check: %d,%d,%d,%d",*(prVaOut1),*(prVaOut1+8),*(prVaOut1+16),*(prVaOut1+24));
    HDR_LOGD("VPU Fake Process Start, FD = %d,%d, Shared FD = %d,%d",i4ImgInFD,i4ImgOutFD,i4ShareInFD,i4ShareOutFD);

    //flush cache, this step must exist
    if(!(self->mpSourceImgBuf[0]->syncCache(eCACHECTRL_FLUSH )))
        HDR_LOGW("buf0 syncCache Fail");
    if(!(self->mpSourceImgBuf[2]->syncCache(eCACHECTRL_FLUSH )))
        HDR_LOGW("buf1 syncCache Fail");

    //FakeHDR VPU processing
    //Customer should implement their algorithm here and replace this example.
    vpu_enque_histogram_data_partition(i4ShareInFD, i4ShareOutFD, 1920, 1080, 1,
        imgSize0.w,imgSize0.h,imgStride0,imgSizeBytes0,imgFormat0,
        imgSize1.w,imgSize1.h,imgStride1,imgSizeBytes1,imgFormat1);

    HDR_LOGD("VPU Fake Process End");
    //Check output buffer, these 4 numbers must be increasing
    HDR_LOGD("VPU check: %d,%d,%d,%d",*(prVaOut1),*(prVaOut1+8),*(prVaOut1+16),*(prVaOut1+24));
#endif

    ret = self->writeHDROutputFrame();
    HDR_LOGE_IF(MTRUE != ret, "[hdrProcess] writeHDROutputFrame failed");

    // notify the caller that HDR post-processing is done
    self->notify(ret);
}

MVOID VendorHDR::notify(MBOOL ret) const
{
    AutoMutex l(mCompleteCBLock);

    if (mCompleteCB && mpCompleteCBUser)
    {
        mCompleteCB(mpCompleteCBUser, mHdrResult, ret);
        HDR_LOGD("hdrProcess mCompleteCB(%d)", ret);
    }
}

MBOOL VendorHDR::release()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    FUNCTION_LOG_END;
    return ret;
}

MVOID VendorHDR::waitInputFrame()
{
    HDR_TRACE_CALL();

    for (MINT32 i = 0; i < mInputFramesNum; i++)
    {
        // TODO: use sem_timedwait() to have informative messages
        sem_wait(&mSourceImgBufSem[i]);
    }

    // save normalized small image for debugging
    if(mDebugMode)
    {
        for (MUINT32 i = 0; i < mInputFramesNum; i++)
        {
            //For debug MDP result
            String8 fileResultName;
            char    pre_filename[512];

            genFileName_TUNING(pre_filename, sizeof(pre_filename), &m_dumpNamingHint);

            fileResultName = String8::format("%s_%s_%d_%dx%d_.raw"
                , pre_filename
                , "Input"
                , i
                , mpSourceImgBufRaw16[i]->getImgSize().w
                , mpSourceImgBufRaw16[i]->getImgSize().h);
            mpSourceImgBufRaw16[i]->saveToFile(fileResultName);
        }
    }
}


MBOOL VendorHDR::writeHDROutputFrame()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    if (mforceBreak)
    {
        HDR_LOGW("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    MBOOL ret = MTRUE;

    mHdrResult = mpSourceImgBuf[0];
#if 0
    sp<IImageBuffer>& hdrResult(mHdrResult);

    ImageBufferUtils::getInstance().createBuffer(hdrResult, mpSourceImgBufRaw16[0]);
    if (hdrResult == nullptr)
    {
        HDR_LOGE("image buffer is NULL");
        return false;
    }

    // get buffer size and copy hdr result to image buffer
    size_t hdrResultSize = 0;
    // placed result at src ptr. and do memory copy
    // For demo example, we placed third input frame as our result
    for (size_t i = 0; i < hdrResult->getPlaneCount(); i++)
    {
        size_t planeBufSize = hdrResult->getBufSizeInBytes(i);
        MUINT8 *srcPtr = (MUINT8 *)mpSourceImgBufRaw16[1]->getBufVA(i);
        void *dstPtr = (void *)hdrResult->getBufVA(i);
        memcpy(dstPtr, srcPtr, planeBufSize);

    }
#endif
lbExit:
    // release Working buffer here
    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::getCaptureInfo(
        Vector<NS3Av3::CaptureParam_T>& vCapExpParam, MINT32& hdrFrameNum)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::getCaptureInfo(std::vector<HDRCaptureParam>& vCapParam)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    // Resizes the container so that it contains n elements
    vCapParam.resize(mInputFramesNum);

    MINT32 iso;
    MINT64 shutterUs;
    NS3Av3::FrameOutputParam_T _paramT;
    int EvSetting[3] = {0, -1, 1};

    for(int i = 0; i < mInputFramesNum; i++)
    {
        CaptureParam_T tmpCap3AParam;
        mHal3A->send3ACtrl( NS3Av3::E3ACtrl_GetExposureParam,
            reinterpret_cast<MINTPTR>(&tmpCap3AParam), 0);

        CaptureParam_T modifiedCap3AParam = tmpCap3AParam;

        // get the current AE PLINE index
        NS3Av3::FrameOutputParam_T _paramT;
        mHal3A->send3ACtrl(E3ACtrl_GetRTParamsInfo, (MINTPTR)&_paramT, 0);
        int _plineIdx = static_cast<int>(_paramT.u4AEIndex);
        HDR_LOGD("E3ACtrl_GetRTParamsInfo: index(%d)", _plineIdx);
        strAEOutput _aeOutput;
        mHal3A->send3ACtrl(
                E3ACtrl_GetEvCapture,
                EvSetting[i] * 10,
                (MINTPTR)&_aeOutput);
        // get ISO from the certain index

        // debug
        HDR_LOGD("E3ACtrl_GetEvCapture: ev(%d), shutter(%" PRId64 "), AfeGain(%d), IspGain(%d)",
                EvSetting[i],
                _aeOutput.EvSetting.u4Eposuretime,
                _aeOutput.EvSetting.u4AfeGain,
                _aeOutput.EvSetting.u4IspGain
               );

        // start to modify AE
        HDRCaptureParam& tmpCapParam(vCapParam.at(i));
        CaptureParam_T& tmpExpParam(tmpCapParam.exposureParam);
        tmpExpParam.u4Eposuretime = _aeOutput.EvSetting.u4Eposuretime;
        tmpExpParam.u4AfeGain     = _aeOutput.EvSetting.u4AfeGain;
        tmpExpParam.u4IspGain     = _aeOutput.EvSetting.u4IspGain;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::init()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    // Init VPU here

lbExit:
    if (!ret)
    {
        uninit();
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL VendorHDR::allocateProcessMemory()
{
    // a worker thread that allocates HDR working buffers asynchronously
    mMemAllocThread = std::async(std::launch::async, VendorHDR::allocateProcessMemoryTask, this);

    return MTRUE;
}

MVOID VendorHDR::allocateProcessMemoryTask(MVOID* arg)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    VendorHDR *self = static_cast<VendorHDR *>(arg);

    // allocate XXX working buffer
    // ret = self->requestXXXImageBuffer(HDR_BUFFER_SE);
    // if (ret != MTRUE)
    // {
    //     HDR_LOGE("can't alloc XXX working buffer");
    // }
    //sem_post(&self->mWorkingImgBufSem);

    FUNCTION_LOG_END;
}

MBOOL VendorHDR::setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
    switch (paramId)
    {
        case HDRProcParam_Set_AWB_Gain:
        {
            HDR_AWB_Gain *getAW = (HDR_AWB_Gain *)iArg1;

            if(getAW) {
                mAWB[0] = (float)getAW->rGain / 512;
                mAWB[1] = (float)getAW->gGain / 512; //G is 1
                mAWB[2] = (float)getAW->bGain / 512;
                HDR_LOGD("Derek AWB gain(%f, %f, %f)" , mAWB[0]
                                                      , mAWB[1]
                                                      , mAWB[2]);

            }
            else {
                HDR_LOGW("Get awb info fail");
            }
        }
        break;
        case HDRProcParam_Set_Debug_Name_Info:
        {
            FILE_DUMP_NAMING_HINT *getInfo = (FILE_DUMP_NAMING_HINT*)iArg1;

            if(getInfo) {
                m_dumpNamingHint = *getInfo;
            }
            else {
                HDR_LOGW("Get debug info fail");
            }
        }
        break;
        default:
            //HDR_LOGE("[setParam] undefined paramId(%u)", paramId);
            return MFALSE;

    }
    return MTRUE;
}

MBOOL VendorHDR::setShotParam(void const* pParam)
{
    if (nullptr == pParam)
    {
        HDR_LOGE("[setShotParam] NULL pParam");
        return MFALSE;
    }

    // TODO: need to use mutex to protect

    HDRProc_ShotParam *in_param = (HDRProc_ShotParam *)pParam;

    memcpy(&mShotParam, in_param, sizeof(HDRProc_ShotParam));

    return MTRUE;
}

MBOOL VendorHDR::getParam(MUINT32 paramId, MUINT32& rArg1, MUINT32& rArg2)
{
    switch (paramId)
    {
        case HDRProcParam_Get_src_main_format:
            rArg1 = eImgFmt_I420;
            break;
        case HDRProcParam_Get_src_main_size:
            rArg1 = mSourceW;
            rArg2 = mSourceH;
            break;
        case HDRProcParam_Get_src_small_format:
            rArg1 = eImgFmt_Y8;
            break;
        case HDRProcParam_Get_src_small_size:
            // rArg1 = mu4W_small;
            // rArg2 = mu4H_small;
            break;
        default:
            HDR_LOGE("[getParam] undefined paramId(%u)", paramId);
            return MFALSE;
    }
    return MTRUE;
}

MVOID VendorHDR::setCompleteCallback(
        HDRProcCompleteCallback_t completeCB, MVOID* user)
{
    AutoMutex l(mCompleteCBLock);

    if (completeCB)
    {
        mCompleteCB = completeCB;
        mpCompleteCBUser = user;
        HDR_LOGD("HDRProc callback is set(%#" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(completeCB));
    }
}

MINT32 VendorHDR::queryHDRAlgoType(void)
{
    return RawDomainHDR;
}

#ifdef FAKEHDR_ENABLE
MBOOL VendorHDR::
transformToUnpackraw16(int index)
{
    MTKDngOp *MyDngop = NULL;
    DngOpResultInfo MyDngopResultInfo;
    DngOpImageInfo MyDngopImgInfo;
    const MINT32 pOpenID = mOpenID;

    MINT32 rawBitDepth = 10;
    HwInfoHelper helper(pOpenID);
    if(!helper.updateInfos()) {
        HDR_LOGE("cannot properly update infos");
        rawBitDepth = 10;
    }
    else
    {
        helper.getRecommendRawBitDepth(rawBitDepth);
    }
    // Initialize //
    HDR_LOGD("Start to simulate pack...\n");
    //
    MUINT32 bitDstNum = MAKE_DngInfo(LOG_TAG, pOpenID)->getRawBitDepth();
    MUINT8 const*  puRawImgBuf = (MUINT8*)mpSourceImgBuf[index]->getBufVA(0);

    if (CC_UNLIKELY( puRawImgBuf == nullptr )) {
        HDR_LOGE("input buffer getBufVA is nullptr");
        assert(0);
    }
    //
    MyDngop = MyDngop->createInstance(DRV_DNGOP_UNPACK_OBJ_SW);
    std::shared_ptr<MTKDngOp> dngop_resource(MyDngop , [](MTKDngOp* p){
        HDR_LOGD("Destroy dngop instance");
        p->destroyInstance(p);
    });

    MyDngopImgInfo.Width = mpSourceImgBuf[index]->getImgSize().w;
    MyDngopImgInfo.Height = mpSourceImgBuf[index]->getImgSize().h;
    MyDngopImgInfo.Stride_src = mpSourceImgBuf[index]->getBufStridesInBytes(0);
    MyDngopImgInfo.Stride_dst = mpSourceImgBufRaw16[index]->getBufStridesInBytes(0);
    MyDngopImgInfo.BIT_NUM =  rawBitDepth;
    MyDngopImgInfo.BIT_NUM_DST = ((rawBitDepth == 12) && (bitDstNum == 12))?12:10;
    MyDngopImgInfo.Bit_Depth = 10;//for pack
    //
    // query buffer size
    MUINT32 buf_size = MyDngopImgInfo.Stride_dst * MyDngopImgInfo.Height;
    MyDngopImgInfo.Buff_size = buf_size;

    // set buffer address
    //unsigned char *p_buffer = new unsigned char[buf_size];
    void* p_buffer = (void*)mpSourceImgBufRaw16[index]->getBufVA(0);

    if (CC_UNLIKELY( p_buffer == nullptr )) {
        HDR_LOGE("input write buffer is nullptr");
        assert(0);
    }

    MyDngopResultInfo.ResultAddr = p_buffer;

    // assign raw data
    MyDngopImgInfo.srcAddr = reinterpret_cast<void*>(const_cast<MUINT8*>(puRawImgBuf));
    HDR_LOGD("transformToUnpackraw16+");
    MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);
    HDR_LOGD("transformToUnpackraw16-");

    // save normalized small image for debugging
    if(mDebugMode)
    {
        String8 filename = String8::format("/sdcard/DCIM/Camera/InputUnpacked_%d_%dx%d.raw", index
                                         , mpSourceImgBuf[index]->getImgSize().w, mpSourceImgBuf[index]->getImgSize().h);
        HDR_LOGD("save input raw[%d] %s", index, filename.string());
        //mpSourceImgBufRaw16[index]->saveToFile(filename);
        bool ret = NSCam::Utils::saveBufToFile(filename.c_str(), (unsigned char *)p_buffer, buf_size);
    }

    return MTRUE;
}
#endif
