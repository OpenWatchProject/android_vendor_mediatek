/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "MtkCam/VsdofCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

//
#include <sys/prctl.h> // prctl(PR_SET_NAME)
#include <sys/resource.h>
#include <system/thread_defs.h>

//
#include "VsdofCtrler.h"
#include <mtkcam/def/common.h>

// mtkcam
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include "../utils/VendorUtils.h"
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>

// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>

#ifdef MY_LOGE
#undef MY_LOGE
#endif
#ifdef MY_LOGW
#undef MY_LOGW
#endif

#define MY_LOGV(id, fmt, arg...)     CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)     CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)     CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)     CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)     CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)     CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)     CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

#define MY__LOGV(fmt, arg...)        CAM_LOGV("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGD(fmt, arg...)        CAM_LOGD("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGI(fmt, arg...)        CAM_LOGI("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGW(fmt, arg...)        CAM_LOGW("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGE(fmt, arg...)        CAM_LOGE("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGA(fmt, arg...)        CAM_LOGA("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)
#define MY__LOGF(fmt, arg...)        CAM_LOGF("[%d][%s] " fmt, mRefCount, __FUNCTION__, ##arg)

#define MY___LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __FUNCTION__, ##arg)
#define MY___LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __FUNCTION__, ##arg)

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY___LOGE("Null %s Object", #x); return MFALSE;}  \
} while(0)

#define CHECK_OBJECT_AND_MAKE_ERROR(x,frame,RequestImg)  do{            \
        if (x == nullptr) {                                             \
            MY__LOGE("Null %s Object", #x);                             \
            frame->markError(RequestImg, true);                         \
            return MFALSE;}                                             \
        } while(0)

using namespace std;
using namespace android;
using namespace NSCam::plugin;
//	using namespace NS3Av3;
using namespace NSCam;

#define create_dump_folder() \
{ \
    if(mVsdofCtrlerDump) { \
        string msFilename = "/sdcard/vsdofctrl//0"; \
        NSCam::Utils::makePath(msFilename.c_str(), 0660); \
        MY__LOGD("create folder:%s",msFilename.c_str()); \
    } \
}

#define dumpFrameReq(frame) \
for (int i = 0 ; i < RequestFrame::eRequestImg_Size ; i++) \
{ \
    IImageBuffer* ptr = frame->getImageBuffer((RequestFrame::eRequestImg)i); \
    if (ptr) \
    { \
        MSize size  = ptr->getImgSize(); \
        MINT format = ptr->getImgFormat(); \
        MY_LOGD(frame->getOpenId(), " [%02d] VA(%p) (%dx%d) f(0x%x)", \
            i, (void *)ptr->getBufVA(0), size.w, size.h, format); \
    } \
}

#define saveImage(str, id, img) \
{ \
    if((mVsdofCtrlerDump)&&(img != nullptr)) \
    { \
        MSize size = img->getImgSize(); \
        string filename = "/sdcard/vsdofctrl/"+to_string(id)+"_" str "_"+to_string(size.w)+"x"+to_string(size.h)+".raw"; \
        img->saveToFile(filename.c_str()); \
        MY__LOGD("save file to path:%s",filename.c_str()); \
    } \
}

#define VSDOF_CTRL_NAME       ("Cam@pluginVSDOF")
#define VSDOF_CTRL_POLICY     (SCHED_OTHER)
#define VSDOF_CTRL_PRIORITY   (0)
#define VSDOF_WAIT_THREAD_TIME_NS   2000000000      // wait 2 sec for MDP process


// 0: use 3rd party algo
// 1: copy main1 to processed image out
// 2: copy main2 to processed image out
#define VSDOF_CTRLER_COPY_TEST          "debug.vsdofctrler.copy"
#define VSDOF_CTRLER_COPY_TEST_DEFAULT  eVSDOF_CTRLER_COPY_MAIN2_TO_OUTPUT

#define VSDOF_CTRLER_DUMP               "debug.vsdofctrler.dump"

MUINT32 VsdofCtrler::mShotCnt = 0;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MBOOL
doCleanImage_and_Thumbnail(
    MRect rectSrc,
    MUINT32 outOrientaion,
    IImageBuffer* pInputImg,
    IImageBuffer* pCleanImg,
    IImageBuffer* pCleanThumbImg,
    IImageBuffer* pProcessedThumbImg )
{
    CHECK_OBJECT(pInputImg);
    CHECK_OBJECT(pCleanImg);

    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>>
        transform(IImageTransform::createInstance()
                , [](IImageTransform * p){ if (p) { p->destroyInstance();}});
    CHECK_OBJECT(transform.get());

    MBOOL ret;
    MRect rectCleanimg;
    MSize CleanImageSize = pCleanImg->getImgSize();
    rectCleanimg.p = MPoint(0, 0);
    if(outOrientaion == eTransform_ROT_90 || outOrientaion == eTransform_ROT_270)
        rectCleanimg.s = MSize(CleanImageSize.h,CleanImageSize.w);
    else
        rectCleanimg.s = CleanImageSize;

    MRect rectCleanThumb(MPoint(0, 0), pCleanThumbImg->getImgSize());
    if(pCleanThumbImg)
    {
        // do clean image & clean image thumbnail
        ret = transform->execute(
                  pInputImg,
                  pCleanImg,
                  pCleanThumbImg,
                  calCrop(rectSrc, rectCleanimg),
                  calCrop(rectSrc, rectCleanThumb),
                  outOrientaion,
                  0,
                  3000);
    }
    else
    {
        // do clean image only
        ret = transform->execute(
                  pInputImg,
                  pCleanImg,
                  NULL,
                  calCrop(rectSrc, rectCleanimg),
                  outOrientaion,
                  3000);
    }

    if(pProcessedThumbImg && pCleanThumbImg)
    {
        // do processed image thumbnail (copy from clean image thumbnail)
        MRect rectProcessedThumb(MPoint(0, 0), pProcessedThumbImg->getImgSize());
        ret = transform->execute(
                  pCleanThumbImg,
                  pProcessedThumbImg,
                  NULL,
                  calCrop(rectCleanThumb, rectProcessedThumb),
                  0,
                  3000);
    }
    return ret;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
VsdofCtrler::
VsdofCtrler(
    MINT32 mode
) : BaseVsdofCtrler(mode)
  , mVsdofCtrlerDump(0)
{
    setCaptureNum(1);
    setDelayFrameNum(0);
    MY__LOGD("CapNum(%d) DelayNum(%d)", getCaptureNum(), getDelayFrameNum());

    // get main1 & main2 openId
    if (!StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId))
    {
        MY__LOGE("get sensor id fail");
    }

    mVsdofCtrlerCopyTest  = ::property_get_int32(VSDOF_CTRLER_COPY_TEST,
                                                VSDOF_CTRLER_COPY_TEST_DEFAULT);
    if( mVsdofCtrlerCopyTest ) {
        MY__LOGD("mVsdofCtrlerCopyTest flag %d", mVsdofCtrlerCopyTest);
    }

    mVsdofCtrlerDump  = ::property_get_int32(VSDOF_CTRLER_DUMP, 0);

    // start thread
    mVsdofCtrlerThread = new VsdofCtrlerThread();
    if(mVsdofCtrlerThread->run(VSDOF_CTRL_NAME)!=OK)
    {
        MY__LOGE("run thread fail");
    }

}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
VsdofCtrler::
~VsdofCtrler()
{
    MY__LOGD("+");

    // stop thread
    if(mVsdofCtrlerThread.get())
    {
        mVsdofCtrlerThread->requestExit();
        mVsdofCtrlerThread->mThreadCondStart.signal();
        MY__LOGD("+ wait thread process thumb & clean image done -3");
        mVsdofCtrlerThread->join();
        MY__LOGD("- exit wait condition -3");
        mVsdofCtrlerThread = NULL;
    }

    MY__LOGD("-");
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------

IImageBuffer*
VsdofCtrler::
createEmptyImageBuffer(MSize ImgSize)
{
    const char* name = "VsdofVendorWoringkBuffer";
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

    // query format
    const MSize MAIN_SIZE = ImgSize;
    const enum EImageFormat format = eImgFmt_NV21;
    const MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(format);
    const MUINT32 IMG_BOUNDARY[3] = {0, 0, 0};
    MUINT32 IMG_STRIDES[3] = {0, 0, 0};
    for (MUINT32 i = 0; i < plane; i++)
    {
        IMG_STRIDES[i] = (NSCam::Utils::Format::queryPlaneWidthInPixels(format, i, MAIN_SIZE.w) *
                         NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i)) >>3;
    }
    const IImageBufferAllocator::ImgParam imgParam(
        format, MAIN_SIZE, IMG_STRIDES, IMG_BOUNDARY, 3);

    const MINT usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN |
                       eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_WRITE_RARELY;

    MY__LOGD("(%dx%d) +", ImgSize.w, ImgSize.h);
    IImageBuffer* pImgBufWorking = allocator->alloc(name, imgParam);
    pImgBufWorking->lockBuf(name, usage);

    return pImgBufWorking;
}

MBOOL
VsdofCtrler::
doVsdof3rdParty(
    IImageBuffer* pMain1Img,
    IImageBuffer* pMain2Img,
    IImageBuffer* pProcessedOutputImg,
    IImageBuffer* pDepthmap)
{
    char* main1_yuv_ptr = (char*)pMain1Img->getBufVA(0);
    char* main2_yuv_ptr = (char*)pMain2Img->getBufVA(0);
    char* processed_yuv_ptr = (char*)pProcessedOutputImg->getBufVA(0);

    MSize main1_size = pMain1Img->getImgSize();
    MSize main2_size = pMain2Img->getImgSize();
    MSize ProcessedOut_size = pProcessedOutputImg->getImgSize();
    MSize Depthmap_size = pDepthmap->getImgSize();  // format: Y8

// ************** ToDo - Vsdof algorithm Start ***************

// ************** ToDo - Vsdof algorithm End ***************
    return MTRUE;
}

MBOOL
VsdofCtrler::doCopy(
    MRect rectInput,
    IImageBuffer* pInputImg,
    IImageBuffer* pOutputImg,
    IImageBuffer* pDepthmap)
{
    CHECK_OBJECT(pInputImg);
    CHECK_OBJECT(pOutputImg);
    CHECK_OBJECT(pDepthmap);
    // get input/output size, and crop size
    MSize dstOutputImgSize = pOutputImg->getImgSize();
    MRect rectDstOutputImgImg(MPoint(0, 0), dstOutputImgSize);

    // copy output frame using MDP (IImageTransform)
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>>
        transform (IImageTransform::createInstance()
                , [](IImageTransform * p){ if (p) { p->destroyInstance();}});
    CHECK_OBJECT(transform.get());
    MBOOL ret = transform->execute(
                  pInputImg,
                  pOutputImg,
                  nullptr,
                  calCrop(rectInput, rectDstOutputImgImg),
                  0,
                  3000);
    if (ret != MTRUE)
    {
        MY__LOGE("IImageTransform::execute for processed image returns fail");
    }

    // create dummy depthmap data
    unsigned char* pDepthmapPtr = (unsigned char*)pDepthmap->getBufVA(0); // format: Y8
    size_t Depthmap_size = pDepthmap->getBufSizeInBytes(0);
    unsigned char cnt=0;
    for(size_t i=0;i<Depthmap_size;i++)
    {
        pDepthmapPtr[i] = cnt++;
        if(cnt == 0xff) cnt++;
        // 0x00, 0x01, 0x02, ..., 0xfe, 0x00, 0x01 ...
    }
    return ret;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
VsdofCtrler::
doDualCamProcess(
    ReqFrameSP main1Frame,
    ReqFrameSP main2Frame)
{
    MBOOL ret = MFALSE;

    MY__LOGD("main1(%p) main2(%p)", (void *)main1Frame.get(), (void *)main2Frame.get());
    if (main1Frame.get() != NULL && main2Frame.get() != NULL)
    {
        dumpFrameReq(main1Frame);
        dumpFrameReq(main2Frame);

        IMetadata* appMetaIn  = main1Frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
        MUINT32 outOrientaion  = main1Frame->getOrientation(RequestFrame::eRequestImg_FullOut);
        IImageBuffer* pMain1Img  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        IImageBuffer* pMain2Img  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);

        IImageBuffer* pOutputImg  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
        IImageBuffer* pOutputThumb  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
        IImageBuffer* pPostviewImg = main1Frame->getImageBuffer(RequestFrame::eRequestImg_PostviewOut);

        IImageBuffer* pOutCleanImg  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
        IImageBuffer* pOutCleanThumb  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);

        IImageBuffer* pDepthmap = main1Frame->getImageBuffer(RequestFrame::eRequestImg_ReservedOut1);

        IImageBuffer* pProcessedWorkingBuffer = NULL;
        IImageBuffer* pProcessedBuffer = pOutputImg;

        CHECK_OBJECT(appMetaIn);
        CHECK_OBJECT(pMain1Img);
        CHECK_OBJECT(pMain2Img);
        CHECK_OBJECT(mVsdofCtrlerThread.get());
        CHECK_OBJECT_AND_MAKE_ERROR(pOutputImg,main1Frame,RequestFrame::eRequestImg_FullOut);
        CHECK_OBJECT_AND_MAKE_ERROR(pOutCleanImg,main2Frame,RequestFrame::eRequestImg_FullOut);
        CHECK_OBJECT_AND_MAKE_ERROR(pDepthmap,main1Frame,RequestFrame::eRequestImg_ReservedOut1);

        // dump input frame
        create_dump_folder();
        saveImage("1.main1",mShotCnt,pMain1Img);
        saveImage("2.main2",mShotCnt,pMain2Img);

        // 1. do clean image and thumbnail
        MRect rectSrc = calCropRegin(appMetaIn, pMain1Img->getImgSize(), mMain1OpenId);
        {
            AutoMutex _l(mVsdofCtrlerThread->mThreadLock);
            if (mVsdofCtrlerThread->mThreadProcessDone == MFALSE)
            {
                MY__LOGD("+ wait thread process thumb & clean image done -1");
                mVsdofCtrlerThread->mThreadCondEnd.wait(mVsdofCtrlerThread->mThreadLock);
                MY__LOGD("- exit wait condition -1");
            }
            mVsdofCtrlerThread->mThreadProcessDone = MFALSE;
            mVsdofCtrlerThread->mRectSrc = rectSrc;
            mVsdofCtrlerThread->mMain1FrameForThread = main1Frame;
            mVsdofCtrlerThread->mMain2FrameForThread = main2Frame;
            // wake up thread to process thumbnail & clean image
            mVsdofCtrlerThread->mThreadCondStart.signal();
        }

        // 2. create working buffer for rotate
        pProcessedWorkingBuffer = createEmptyImageBuffer(pMain1Img->getImgSize());

        // 3. get output crop size
        MSize dstOutputImgSize = pOutputImg->getImgSize();
        if(outOrientaion == eTransform_ROT_90 || outOrientaion == eTransform_ROT_270)
        {
            pProcessedBuffer = pProcessedWorkingBuffer;
            dstOutputImgSize = MSize(pOutputImg->getImgSize().h, pOutputImg->getImgSize().w);
        }
        MRect rectDstOutputImg(MPoint(0, 0), dstOutputImgSize);

        // 4. do image process (3rd party algo or just copy from main1/main2)
        if(mVsdofCtrlerCopyTest == eVSDOF_CTRLER_DO_3RD_PARTY_ALGO)
            ret = doVsdof3rdParty(pMain1Img, pMain2Img, pProcessedBuffer,pDepthmap);
        else if(mVsdofCtrlerCopyTest == eVSDOF_CTRLER_COPY_MAIN1_TO_OUTPUT)
            ret = doCopy(rectSrc, pMain1Img, pProcessedBuffer,pDepthmap);
        else
        {
            IMetadata* appMeta2In  = main2Frame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
            if(appMetaIn != NULL)
            {
                MRect rectSrc2 = calCropRegin(appMeta2In, pMain2Img->getImgSize(), mMain2OpenId);
                ret = doCopy(rectSrc2, pMain2Img, pProcessedBuffer,pDepthmap);
            }
        }

        // 5. use MDP to generate output yuv image & post view
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>>
            transform(IImageTransform::createInstance()
                , [](IImageTransform * p){ if (p) { p->destroyInstance();}});
        CHECK_OBJECT(transform.get());
        MBOOL ret;
        if(pPostviewImg)
        {
            MRect rectDstPostView;
            rectDstPostView.p = MPoint(0, 0);
            rectDstPostView.s = pPostviewImg->getImgSize();
            ret = transform->execute(pProcessedBuffer,
                                      pOutputImg,
                                      pPostviewImg,
                                      calCrop(rectSrc, rectDstOutputImg),
                                      calCrop(rectSrc, rectDstPostView),
                                      outOrientaion,
                                      0,
                                      3000);
        }
        else
        {
            ret = transform->execute(pProcessedBuffer,
                                      pOutputImg,
                                      NULL,
                                      calCrop(rectSrc, rectDstOutputImg),
                                      outOrientaion,
                                      3000);
        }
        if (ret != MTRUE)
        {
            MY__LOGE("IImageTransform::execute for processed image returns fail");
            return ret;
        }

        // wait thread process thumb & clean image done
        if (mVsdofCtrlerThread->mThreadProcessDone == MFALSE)
        {
            AutoMutex _l(mVsdofCtrlerThread->mThreadLock);
            MY__LOGD("+ wait thread process thumb & clean image done -2");
            mVsdofCtrlerThread->mThreadCondEnd.wait(mVsdofCtrlerThread->mThreadLock);
            MY__LOGD("- exit wait condition -2");
        }

        // 6. free working buffer
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if (pProcessedWorkingBuffer && allocator)
        {
            MY__LOGD("free vsdof working buffer");
            allocator->free(pProcessedWorkingBuffer);
        }

        // dump result
        saveImage("3.outimg",mShotCnt,pOutputImg);
        saveImage("4.outimg_thumb",mShotCnt,pOutputThumb);
        saveImage("5.cleanimg",mShotCnt,pOutCleanImg);
        saveImage("6.cleanimg_thumb",mShotCnt,pOutCleanThumb);
        saveImage("7.postview",mShotCnt,pPostviewImg);
        saveImage("8.depthmap",mShotCnt,pDepthmap);
        mShotCnt ++;

        // no need to process post view for main2
        main2Frame->markError(RequestFrame::eRequestImg_PostviewOut, true);
        onShutter();
        onNextCaptureReady();
    }

    MY__LOGD("-");
    return (ret==MTRUE)? true : false;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VsdofCtrler::VsdofCtrlerThread::
readyToRun()
{
    MY___LOGD("+");
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)VSDOF_CTRL_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, VSDOF_CTRL_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, VSDOF_CTRL_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY___LOGD("tid(%d) policy(%d) priority(%d)"
            , ::gettid(), ::sched_getscheduler(0)
            , sched_p.sched_priority);

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
VsdofCtrler::VsdofCtrlerThread::
threadLoop()
{
    MY___LOGD("+");
    MBOOL ret = MFALSE;

    while(!exitPending())
    {
        // 1. check wait event
        AutoMutex _l(mThreadLock);
        MY___LOGD("wait doDualCamProcess to notify");
        mThreadCondStart.wait(mThreadLock);

        ReqFrameSP& main1Frame = mMain1FrameForThread;
        ReqFrameSP& main2Frame = mMain2FrameForThread;

        // 2. check if thread exit
        if(exitPending())
        {
            main1Frame = NULL;
            main2Frame = NULL;
            mThreadProcessDone = MTRUE;
            mThreadCondEnd.broadcast();
            MY___LOGD("exit thread");
            break;
        }

        // 3. check input
        if((main1Frame == NULL)||(main2Frame == NULL))
        {
            MY___LOGD("wrong input, mMain1FrameForThread=NULL, mMain2FrameForThread=NULL");
            continue;
        }

        MY___LOGD("start to process clean image and thumbnail");

        MUINT32 outOrientaion  = main1Frame->getOrientation(RequestFrame::eRequestImg_FullOut);
        IImageBuffer* pMain1Img  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        IImageBuffer* pMain2Img  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);
        IImageBuffer* pOutputThumb  = main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
        IImageBuffer* pOutCleanImg  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
        IImageBuffer* pOutCleanThumb  = main2Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);

        // do clean image and thumbnail
        ret = doCleanImage_and_Thumbnail(mRectSrc,
                                       outOrientaion,
                                       pMain1Img,
                                       pOutCleanImg,
                                       pOutCleanThumb,
                                       pOutputThumb);
        if (ret != MTRUE)
        {
            MY___LOGE("fail to do clean image and thumbnail");
        }
        else
        {
            MY___LOGD("process clean image & thumb nail ok");
        }

        main1Frame = NULL;
        main2Frame = NULL;
        mThreadProcessDone = MTRUE;
        mThreadCondEnd.broadcast();
    }

    MY___LOGD("-");
    return  false;
}


