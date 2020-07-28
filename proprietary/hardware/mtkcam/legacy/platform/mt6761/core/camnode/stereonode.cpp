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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/SNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;

#include <semaphore.h>
#include <vector>
#include <list>
using namespace std;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
using namespace NSCam::NSIoPipe::NSPostProc;
#include <core/iopipe/CamIO/PortMap.h>
using namespace NSCam::NSIoPipe;
//
#include "mtkcam/drv/imem_drv.h"
//
#include <mtkcam/imageio/ispio_utility.h>
//
#include <aee.h>
//
#include <sys/prctl.h>
//
#include <mtkcam/featureio/stereo_hal_base.h>
//
#include "./inc/stereonodeImpl.h"
#include "./inc/IspSyncControlHw.h"
#include <mtkcam/camnode/stereonode.h>
//
#include <cutils/properties.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)

#define MY_LOGV2(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD2(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI2(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW2(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE2(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA2(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF2(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START          MY_LOGD("+")
#define FUNC_END            MY_LOGD("-")

#define ENABLE_CAMIOCONTROL_LOG (0)
#define ENABLE_BUFCONTROL_LOG   (1)
#define BUFFER_RETURN_CHECK     (1)


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_PRV_NAME    "PrvStereo"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*
********************************************************************************/
StereoNode*
StereoNode::
createInstance(SNodeInitCfg const initCfg)
{
    // NOTE:
    // Preview : mpStereoNode,mpStereoNode_Main2      stereoNodeType = STEREO_PASS2_FEATURE_PRV( MtkCamAdapter.preview.cpp )
    // Capture : mpStereoNode,mpStereoNode_Main2      stereoNodeType = STEREO_PASS2_CAPTURE( StereoShot.cpp )
    //           mpStereoCapNode                      stereoNodeType = PASS2_CAPTURE ( Set in pass2node.capture.cpp)
    switch(initCfg.mScenarioType)
    {
        case STEREO_PASS2_PREVIEW:
        case STEREO_PASS2_CAPTURE:
            return new PrvStereo(initCfg);
            break;
        case STEREO_PASS2_FEATURE_PRV:
        case STEREO_PASS2_FEATURE_CAP:
            return new FeatureStereo(initCfg);
            break;
        default:
            break;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
StereoNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
StereoNode::
StereoNode(SNodeInitCfg const initCfg)
    : ICamThreadNode(
            MODULE_PRV_NAME,
            SingleTrigger,
            SCHED_POLICY,
            SCHED_PRIORITY
            )
{
}

/*******************************************************************************
 *
 ********************************************************************************/
StereoNode::
~StereoNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoNodeImpl::
    StereoNodeImpl(SNodeInitCfg const initCfg)
    : StereoNode(initCfg)
    , mInitCfg(initCfg)
    , mSensorDevIdx(0)
    , muFrameCnt(0)
    , mbCfgImgo(MFALSE)
    , mbCfgFeo(MFALSE)
    , mbCfgRgb(MFALSE)
    , mTargetSize()
    , mAlgoImgSize()
    , mRgbSize()
    , mFeImgSize()
    , mHwFeSize()
    , mHwFeBlock(0)
    , mpStereoHal(NULL)
    , mpPostProcPipe(NULL)
    , mpIspSyncCtrlHw(NULL)
    , mbAllocDone(MFALSE)
    , mbIsStopped(MTRUE)
    , mpImgTransform(NULL)
{
    mpImgTransform = IImageTransform::createInstance();
    if( !mpImgTransform )
    {
        MY_LOGE("imageTransform create failed");
    }
    mBufCount = (isPreviewPath()) ? BUF_COUNT_PREVUEW : BUF_COUNT_CAPTURE;
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoNodeImpl::
~StereoNodeImpl()
{
    if( mpImgTransform )
    {
        mpImgTransform->destroyInstance();
        mpImgTransform = NULL;
    }
}


/******************************************************************************
*
 ******************************************************************************/
bool
StereoNodeImpl::
waitThreadDone()
{
    void* threadRet = NULL;
    if( pthread_join(mThreadAlloc, &threadRet) != 0 )
    {
        MY_LOGE("pthread join fail");
    }
    mbAllocDone = MTRUE;
    return MTRUE;
}


/******************************************************************************
*
 ******************************************************************************/
bool
StereoNodeImpl::
alloc()
{
    MBOOL ret = MFALSE;
    HW_DATA_STEREO_T hwData;
    if( !mpStereoHal || !mpStereoHal->STEREOGetInfo(hwData) )
    {
        MY_LOGE("STEREOGetInfo fail");
    }
    mTargetSize     = MSize(mInitCfg.mAlgoTargetWidth, mInitCfg.mAlgoTargetHeight);
    mAlgoImgSize    = mpStereoHal->getAlgoInputSize(getSensorIdx());
    mFeImgSize      = mpStereoHal->getFEImgSize();
    mHwFeBlock      = hwData.hwfe_block_size;
    mHwFeSize.w     = hwData.fefm_image_width;
    mHwFeSize.h     = hwData.fefm_imgae_height;
    mRgbSize.w      = hwData.rgba_image_width;
    mRgbSize.h      = hwData.rgba_image_height;
    MY_LOGD("FE(%dx%d):block(%d)Size(%dx%d);RGB(%dx%d);ALGOImg(%dx%d);TargetImg(%dx%d)",
        getHWFESize().w, getHWFESize().h, getHWFEBlock(),
        getFEImgSize().w, getFEImgSize().h,
        getRgbImgSize().w, getRgbImgSize().h,
        getAlgoImgSize().w, getAlgoImgSize().h,
        getTargetImgSize().w, getTargetImgSize().h);
    //
    MSize rgbSize = getRgbImgSize();
    MSize algoSize  = getAlgoImgSize() ;
    vector<HwPortConfig_t> lHwPortCfg;
    HwPortConfig_t cfgImg = {
        mapToPortID(STEREO_IMG),
        eImgFmt_YV12,
        algoSize,
        MRect( MPoint(0, 0), algoSize )
    };

    //
    setupPort(mbCfgImgo, mbCfgFeo, mbCfgRgb);
    if ( mbCfgImgo )
        lHwPortCfg.push_back(cfgImg);

    // we alloc STEREO_DST in capture mode only
    if(!isPreviewPath())
    {
        MSize dstSize = MSize(960,544);
        HwPortConfig_t cfgDst = {
            mapToPortID(STEREO_DST),
            eImgFmt_YV12,
            dstSize,
            MRect( MPoint(0, 0), dstSize )
        };
        lHwPortCfg.push_back(cfgDst);
    }

    //
    CAM_TRACE_BEGIN("alloc");
    if( !allocBuffers(lHwPortCfg) )
    {
        MY_LOGE("alloc buffers failed");
        goto lbExit;
    }
    CAM_TRACE_END();
    ret = MTRUE;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void*
StereoNodeImpl::
doThreadAlloc(void* arg)
{
    ::prctl(PR_SET_NAME,"allocThread", 0, 0, 0);
    StereoNodeImpl* pSelf = reinterpret_cast<StereoNodeImpl*>(arg);
    return (void*)pSelf->alloc();
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;

    //
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    mSensorDevIdx   = pHalSensorList->querySensorDevIdx(getSensorIdx());
    //
    mpStereoHal = StereoHalBase::createInstance();
    //
    mpIspSyncCtrlHw = IspSyncControlHw::createInstance(getSensorIdx());
    if( !mpIspSyncCtrlHw )
    {
        MY_LOGE("create IspSyncControlHw failed");
        goto lbExit;
    }
    //
    mlPostBufData.clear();
    mDataDstSet.clear();
    //
    mpPostProcPipe = createPipe();
    if( mpPostProcPipe == NULL )
    {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }
    if( !mpPostProcPipe->init() )
    {
        MY_LOGE("postproc pipe init failed");
        goto lbExit;
    }
    //
    for (MUINT32 i = 0; i < getBufCount(); ++i)
    {
        mpStaParam[i]   = new StaData;
        mpSrzParam[i]   = new SrzSize;
        mpFeParam[i]    = new FE;
    }

    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    if( mpIspSyncCtrlHw )
    {
        mpIspSyncCtrlHw->destroyInstance();
        mpIspSyncCtrlHw = NULL;
    }

    if ( mpPostProcPipe )
    {
        ret = mpPostProcPipe->uninit();
        mpPostProcPipe->destroyInstance(getName());
        mpPostProcPipe = NULL;
    }

    if ( mpStereoHal )
    {
        mpStereoHal->destroyInstance();
        mpStereoHal = NULL;
    }

    for (MUINT32 i = 0; i < getBufCount(); ++i) {
        if ( mpStaParam[i] )
            delete mpStaParam[i];
        if ( mpSrzParam[i] )
            delete mpSrzParam[i];
        if ( mpFeParam[i] )
            delete mpFeParam[i];
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    if( pthread_create(&mThreadAlloc, NULL, doThreadAlloc, this) != 0 )
    {
        MY_LOGE("pthread create failed");
        goto lbExit;
    }
    //
    muPostFrameCnt = 0;
    muEnqFrameCnt = 0;
    muDeqFrameCnt = 0;
    //
    ret = MTRUE;
    mbIsStopped = MFALSE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done

    Mutex::Autolock lock(mLock);
    {
        list<PostBufInfo>::iterator iter;
        for(iter = mlPostBufData.begin(); iter != mlPostBufData.end(); iter++)
        {
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)",
                    (*iter).data,
                    (*iter).buf);
            handleReturnBuffer(
                (*iter).data,
                (MUINTPTR)((*iter).buf),
                0);
        }
    }
    while(muEnqFrameCnt > muDeqFrameCnt)
    {
        MY_LOGD("wait lock enq %d > deq %d", muEnqFrameCnt, muDeqFrameCnt);
        mCondDeque.wait(mLock);
        MY_LOGD("wait done");
    }
    //
    if ( !mbAllocDone )
    {
        waitThreadDone();
    }
    freeBuffers();
    //
    mbIsStopped = MTRUE;
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    if(!isPreviewPath())
    {
        MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: datatype: %d buf: 0x%08X", data, buf);
    }
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;
    CAM_TRACE_CALL();
    if ( data != STEREO_FEO )
    {
        ICamBufHandler* pBufHdl = getBufferHandler(data);
        IImageBuffer* pBuf = (IImageBuffer*)buf;
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
            return MFALSE;
        }
        MY_LOGD("SNode::onReturnBuffer, data %d, buf(%d:0x%x)", data, pBuf->getFD(), buf);
        ret = pBufHdl->enqueBuffer(data, pBuf);
        if( !ret )
        {
            MY_LOGE("enque fail: data %d, buf 0x%x", data, buf);
        }
    }
    else
    {
        Mutex::Autolock lock(mLock);
        IMEM_BUF_INFO feo = *(IMEM_BUF_INFO*)buf;
        mlFeBufQueue.push_back(feo);
        MY_LOGD("SNode::onReturnBuffer, data %d, buf(0x%x) ID(%d) VA(0x%x) size(%d)",
                data, buf, feo.memID, feo.virtAddr, mlFeBufQueue.size());

        if(mbIsStopped){
        // free this feo buffer
        IMemDrv* pIMemDrv =  IMemDrv::createInstance();
        IMEM_BUF_INFO feo = *(IMEM_BUF_INFO*)buf;
        if(0 == feo.virtAddr)
        {
            MY_LOGE("Buffer doesn't exist");
            return MTRUE;
        }
        if(pIMemDrv->unmapPhyAddr(&feo) < 0)
        {
            MY_LOGE("pIMemDrv->unmapPhyAddr() error");
            return MTRUE;
        }
        if (pIMemDrv->freeVirtBuf(MODULE_PRV_NAME, &feo) < 0)
        {
            MY_LOGE("pIMemDrv->freeVirtBuf() error");
            return MTRUE;
        }
        // uninit IMemDrv
        if (getBufCount() == mlFeBufQueue.size()){
            if ( pIMemDrv ) {
                MY_LOGD("pIMemDrv->uninit()");
                pIMemDrv->uninit(MODULE_PRV_NAME);
                pIMemDrv->destroyInstance();
            }
        }
    }
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
StereoNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
        char dumppath[256];
        sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)               \
        do{                                                        \
            IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
            char filename[256];                                    \
            sprintf(filename, "%s%s_%d_%dx%d_%d.%s",               \
                    dumppath,                                      \
                    #type,                                         \
                    getSensorIdx(),                                \
                    buffer->getImgSize().w,buffer->getImgSize().h, \
                    cnt,                                           \
                    fileExt                                        \
                   );                                              \
            buffer->saveToFile(filename);                          \
        }while(0)

        if(!makePath(dumppath,0660))
        {
            MY_LOGE("makePath [%s] fail",dumppath);
            return;
        }

        switch( data )
        {
            case STEREO_SRC:
                DUMP_IImageBuffer( STEREO_SRC, buf, "raw", muPostFrameCnt );
                break;
            case STEREO_DST:
                DUMP_IImageBuffer( STEREO_DST, buf, "yuv", muDeqFrameCnt  );
                break;
            case STEREO_IMG:
                DUMP_IImageBuffer( STEREO_IMG, buf, "yuv", muDeqFrameCnt  );
                break;
            default:
                MY_LOGE("not handle this yet: data %d", data);
                break;
        }
#undef DUMP_IImageBuffer
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
threadLoopUpdate()
{
    PostBufInfo postBufData;
    {
        Mutex::Autolock lock(mLock);

        if( mlPostBufData.size() == 0 ) {
            MY_LOGE("no posted buf");
            return MFALSE;
        }

        postBufData = mlPostBufData.front();
        mlPostBufData.pop_front();
    }

    //
    if ( !mbAllocDone )
    {
        waitThreadDone();
    }

    if((getSensorIdx() == 2) && (StereoSettingProvider::IsMain2YuvSensor()))
    {
        // If main2 sensor is yuv, use mdp instead of P2
        MY_LOGD("enter MDP : data=%d, buf=%p (format=%d, %dx%d), ext=%d",
                postBufData.data,
                postBufData.buf, postBufData.buf->getImgFormat(),
                postBufData.buf->getImgSize().w,postBufData.buf->getImgSize().h,
                postBufData.ext);

        return processMDP(postBufData.data, postBufData.buf, postBufData.ext);
    }
    else
    {
        return enquePass2(postBufData.data, postBufData.buf, postBufData.ext);
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
setupPort(MBOOL& cfgImgo, MBOOL& cfgFeo, MBOOL& cfgRgb)
{
    cfgImgo     = isDataConnected(STEREO_IMG);
    cfgFeo = cfgRgb = MFALSE;
    mDataDstSet = getDataConnected();
    if(isPreviewPath())
    {
        // Force to connect STEREO_DST for get buf handle when do enquePass2
        mDataDstSet.push_back(STEREO_DST);
    }

    MY_LOGD("mDataDstSet.size()=%d", mDataDstSet.size());
    MY_LOGD("imgo %d, feo %d, rgb %d", cfgImgo, cfgFeo, cfgRgb);
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
allocBuffers(vector<HwPortConfig_t> & lPortCfg)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    vector< HwPortConfig_t >::const_iterator pConfig = lPortCfg.begin();
    IMemDrv* pIMemDrv =  IMemDrv::createInstance();
    if ( !pIMemDrv || !pIMemDrv->init(MODULE_PRV_NAME) ) {
        MY_LOGE("pIMemDrv->init() error");
        goto lbExit;
    }
    for(MUINT32 i = 0; i < getBufCount(); ++i)
    {
        IMEM_BUF_INFO bufInfo;
        bufInfo.size = getHWFESize().size();
        if(pIMemDrv->allocVirtBuf(MODULE_PRV_NAME, &bufInfo) < 0)
        {
            MY_LOGE("pIMemDrv->allocVirtBuf() error, i(%d)",i);
            goto lbExit;
        }
        if(pIMemDrv->mapPhyAddr(&bufInfo) < 0)
        {
            MY_LOGE("pIMemDrv->mapPhyAddr() error, i(%d)",i);
            goto lbExit;
        }
        pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &bufInfo);
        mlFeBufQueue.push_back(bufInfo);
    }
    //
    while( pConfig != lPortCfg.end() )
    {
        MUINT32 nodedatatype = mapToNodeDataType(pConfig->mPortID);
        ICamBufHandler* pBufHdl = getBufferHandler(nodedatatype);
        MY_LOGD("handle(%p) data(%d) S(%dx%d)F(0x%x)", pBufHdl, nodedatatype, pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", nodedatatype);
            goto lbExit;
        }
        //alloc buffer
        AllocInfo allocinfo(pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt,
                eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);

        for(MUINT32 i = 0; i < getBufCount() ; i++ )
        {
            if(nodedatatype == STEREO_IMG)
            {
                pBufHdl->updateRequest(nodedatatype,
                                        mpStereoHal->getAlgoInputTransform(),
                                        isPreviewPath()?
                                        (NSImageio::NSIspio::EPortCapbility_Disp):
                                        (NSImageio::NSIspio::EPortCapbility_Cap));
                if( mpStereoHal->getAlgoInputTransform() == eTransform_ROT_90  ||
                    mpStereoHal->getAlgoInputTransform() == eTransform_ROT_270 ||
                    mpStereoHal->getAlgoInputTransform() == (eTransform_FLIP_V |eTransform_ROT_90)||
                    mpStereoHal->getAlgoInputTransform() == (eTransform_FLIP_H |eTransform_ROT_90))
                {
                    allocinfo.w = pConfig->mSize.h;
                    allocinfo.h = pConfig->mSize.w;
                }
            }
            if( !pBufHdl->requestBuffer(nodedatatype, allocinfo) )
            {
                MY_LOGE("request buffer failed: data %d", nodedatatype);
                goto lbExit;
            }
            //
            if ( nodedatatype == STEREO_IMG)
            {
                ImgRequest outRequest;
                IImageBuffer* pBuf = NULL;
                if( !pBufHdl->dequeBuffer(nodedatatype, &outRequest) )
                {
                    MY_LOGE("deque buffer failed: data %d", nodedatatype);
                    goto lbExit;
                }
                pBuf = const_cast<IImageBuffer*>(outRequest.mBuffer);
                MSize AlgoSize = MSize(allocinfo.w,allocinfo.h);
                pBuf->setExtParam(AlgoSize);        // why??
                if( !pBufHdl->enqueBuffer(nodedatatype, outRequest.mBuffer) )
                {
                    MY_LOGE("enque buffer failed: data %d", nodedatatype);
                    goto lbExit;
                }
            }
        }
        //
        pConfig++;
    }
    //
    ret = MTRUE;
lbExit:
    MY_LOGD("MemLeakFix");
    // if ( !pIMemDrv ) {
    //     pIMemDrv->uninit();
    //     pIMemDrv->destroyInstance();
    // }
    if (pIMemDrv){
        pIMemDrv->destroyInstance();
    }
    if( !ret ) {
        MY_LOGE("allocBuffers failed");
    }
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
freeBuffers()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    list<IMEM_BUF_INFO>::iterator iter;
    IMEM_BUF_INFO feo;
    IMemDrv* pIMemDrv =  IMemDrv::createInstance();
    // if ( !pIMemDrv || !pIMemDrv->init() ) {
    //     MY_LOGE("pIMemDrv->init() error");
    //     goto lbExit;
    // }
    MY_LOGD("check buffer size(%d)", mlFeBufQueue.size());
    for(iter = mlFeBufQueue.begin(); iter != mlFeBufQueue.end(); iter++)
    {
        MY_LOGD("Buffer addr(0x%x)", (*iter).virtAddr);
        if(0 == (*iter).virtAddr)
        {
            MY_LOGE("Buffer doesn't exist");
            continue;
        }
        if(pIMemDrv->unmapPhyAddr(&(*iter)) < 0)
        {
            MY_LOGE("pIMemDrv->unmapPhyAddr() error");
            goto lbExit;
        }
        if (pIMemDrv->freeVirtBuf(MODULE_PRV_NAME, &(*iter)) < 0)
        {
            MY_LOGE("pIMemDrv->freeVirtBuf() error");
            goto lbExit;
        }
    }
    //
    ret = MTRUE;
lbExit:
    // if ( !pIMemDrv ) {
    //     pIMemDrv->uninit();
    //     pIMemDrv->destroyInstance();
    // }
    MY_LOGD("MemLeakFix");

    if (getBufCount() == mlFeBufQueue.size()){
        if ( pIMemDrv ) {
            MY_LOGD("pIMemDrv->uninit()");
            pIMemDrv->uninit(MODULE_PRV_NAME);
            pIMemDrv->destroyInstance();
        }
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
enquePass2(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    CAM_TRACE_CALL();
    // get output bufffers
    MUINT32 dstCount = 0, dstDataType[MAX_DST_PORT_NUM];
    ImgRequest outRequest[MAX_DST_PORT_NUM];
    IMEM_BUF_INFO feoRequest;
    vector<MUINT32> vDataDst = mDataDstSet;
    MUINT32 const dataDstCount = vDataDst.size()-1; // STEREO_DST is optional
    MSize const srcImgSize = buf->getImgSize();
    MSize const dstImgSize = getAlgoImgSize();
    MSize FDSize = MSize(640,360);              // give default FD buffer size

    //
    for(MUINT32 i = 0; i < vDataDst.size(); i++)
    {
        ICamBufHandler* pBufHdl = getBufferHandler( vDataDst[i] );
        if( pBufHdl && pBufHdl->dequeBuffer( vDataDst[i], &outRequest[dstCount]) )
        {
            MY_LOGD("vDataDst[%d]=%d: get pBufHdl", i, vDataDst[i]);
            dstDataType[dstCount] = vDataDst[i];
            dstCount++;
        }
        else
        {
            MY_LOGD("vDataDst[%d]=%d: no pBufHdl", i, vDataDst[i]);
            if ( STEREO_DST != vDataDst[i] )
            {   // ignore fd/ot dst buffer
                MY_LOGE("no dst buf (%d)", vDataDst[i]);
            }
        }
    }
    if( dstCount < dataDstCount )
    {
        MY_LOGW("not enough dst buffer, skip data(%d), buf(0x%x)", data, buf);
        handleReturnBuffer(data, (MUINTPTR)buf);

        //
        for( MUINT32 i = 0 ; i < dstCount ; i++ )
        {
            MUINT32 const dstData   = dstDataType[i];
            ICamBufHandler* pBufHdl = getBufferHandler(dstData);
            IImageBuffer const* pDstBuf = outRequest[i].mBuffer;
            if( pBufHdl && pBufHdl->enqueBuffer(dstData, pDstBuf) )
            {
                MY_LOGD("data %d, buf(%d:0x%x)", dstData, pDstBuf->getFD(), pDstBuf);
            }
            else
            {
                MY_LOGE("enqueBuf fail: data %d, buf(%d:0x%x)", dstData, pDstBuf->getFD(), pDstBuf);
            }
        }
        return MTRUE;
    }

    QParams enqueParams;

    MUINT32 magicNum = 0;
    MVOID*  pPrivateData = NULL;
    MUINT32 privateDataSize = 0;
    MCropRect p2InCrop;

    MINT64 timestamp = 0;
    //input
    {
        IImageBuffer const*  pSrcBuf = buf;
        Input src;
        //
        src.mPortID = mapToPortID(data);
        src.mBuffer = const_cast<IImageBuffer*>(pSrcBuf);
        //
        mpIspSyncCtrlHw->getPass2Info(
                            src.mBuffer,
                            srcImgSize,
                            magicNum,
                            pPrivateData,
                            privateDataSize,
                            p2InCrop);
        //
        enqueParams.mpPrivaData = pPrivateData;
        enqueParams.mFrameNo = magicNum;
        //
        timestamp = pSrcBuf->getTimestamp();

        MY_LOGD("P2 input : mpPrivaData=%p, mFrameNo=%d, PortID(type=%d, index=%d, inout=%d), buf_size(%dx%d), format=%d",
                enqueParams.mpPrivaData,
                enqueParams.mFrameNo,
                src.mPortID.type, src.mPortID.index, src.mPortID.inout,
                pSrcBuf->getImgSize().w,pSrcBuf->getImgSize().h,pSrcBuf->getImgFormat());

        enqueParams.mvIn.push_back(src);
    }

    IMAGE_RESOLUTION_INFO_STRUCT    algo_resolution = StereoSettingProvider::imageResolution();
    MUINT32 uRatioDenomerator = algo_resolution.uRatioDenomerator;  //16 or 4
    MUINT32 uRatioNumerator = algo_resolution.uRatioNumerator;    //9 or 3
    MSize bufSz = buf->getImgSize();
    int p2_out_h = (bufSz.w*uRatioNumerator)/uRatioDenomerator;
    int32_t bBypassRatioCheck = 0;

    if(getSensorIdx() == 2) // only check for main2
    {
        bBypassRatioCheck = ::property_get_int32("debug.camera.stereo.ratiom2", 0);
    }
    // Crop1 => imgi size
    // crop2 => WROT
    // crop3 => WDMA (FD buffer)

    MCrpRsInfo crop1;
    crop1.mGroupID    = 1;
    crop1.mCropRect = MCropRect(MPoint(0,0),bufSz);
    crop1.mResizeDst  = bufSz;

    MCrpRsInfo crop2;
    crop2.mGroupID      = 2;
    if(( p2_out_h == bufSz.h)|| (bBypassRatioCheck))
    {
        crop2.mCropRect = MCropRect(MPoint(0,0),bufSz);
    }
    else
    {
        crop2.mCropRect.p_fractional.x = 0;
        crop2.mCropRect.p_fractional.y = 0;

        crop2.mCropRect.s.w = bufSz.w;
        crop2.mCropRect.s.h = p2_out_h & ~1; //Image size must be even

        crop2.mCropRect.p_integral.x = 0;
        crop2.mCropRect.p_integral.y = ((bufSz.h - crop2.mCropRect.s.h)>>1);
    }
    crop2.mResizeDst    = dstImgSize;

    //
    // output
    MBOOL  bOutputFDBuf = MFALSE;
    for( MUINT32 i = 0 ; i < dstCount ; i++ )
    {
        IImageBuffer const* pDstBuf = outRequest[i].mBuffer;
        if(STEREO_DST == dstDataType[i])
        {
            FDSize = pDstBuf->getImgSize();
            bOutputFDBuf = MTRUE;
        }

        Output dst;
        dst.mPortID     = mapToPortID(dstDataType[i]);
        dst.mBuffer     = const_cast<IImageBuffer*>(pDstBuf);
        dst.mTransform  = outRequest[i].mTransform;
        dst.mBuffer->setTimestamp(timestamp);

        MY_LOGD("Out data %d, buf 0x%x, va/pa(0x%x/0x%x), size(%dx%d), tans %d; Output PortID: type=%d, index=%d, inout=%d",
                dstDataType[i], pDstBuf,
                pDstBuf->getBufVA(0), pDstBuf->getBufPA(0),
                pDstBuf->getImgSize().w, pDstBuf->getImgSize().h,
                outRequest[i].mTransform,
                dst.mPortID.type,
                dst.mPortID.index,
                dst.mPortID.inout);
        //
        enqueParams.mvOut.push_back(dst);
    }

    MCrpRsInfo crop3;
    if(bOutputFDBuf)
    {
        crop3.mGroupID      = 3;
        crop3.mCropRect     = MCropRect(MPoint(0,0),bufSz);
        crop3.mResizeDst    = FDSize;
    }
    else
    {
        crop3.mCropRect     = MCropRect(MPoint(0,0),MSize(0,0));
        crop3.mResizeDst    = MSize(0,0);
    }
    MY_LOGD("In data %d, buf 0x%x,va/pa(0x%x/0x%x),size(%dx%d) # 0x%X, cnt %d; crop2(%d,%d,%d,%d)(%dx%d), crop3(%d,%d,%d,%d)(%dx%d)",
            data, buf, buf->getBufVA(0), buf->getBufPA(0), buf->getImgSize().w, buf->getImgSize().h,magicNum, muEnqFrameCnt,
            crop2.mCropRect.p_integral.x, crop2.mCropRect.p_integral.y, crop2.mCropRect.s.w, crop2.mCropRect.s.h, crop2.mResizeDst.w, crop2.mResizeDst.h,
            crop3.mCropRect.p_integral.x, crop3.mCropRect.p_integral.y, crop3.mCropRect.s.w, crop3.mCropRect.s.h, crop3.mResizeDst.w, crop3.mResizeDst.h);

    // update p2 crop parameter
    enqueParams.mvCropRsInfo.push_back( crop1 );
    enqueParams.mvCropRsInfo.push_back( crop2 );
    if(bOutputFDBuf)
    {
        enqueParams.mvCropRsInfo.push_back( crop3 );
    }
    //
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie = this;
    //
    configFeature();
    //
    if( !mpIspSyncCtrlHw->lockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync lock pass2 failed");
        return MFALSE;
    }
    //
    CAM_TRACE_FMT_BEGIN("enqP2:%d", muEnqFrameCnt);
    if( !mpPostProcPipe->enque(enqueParams) )
    {
        CAM_TRACE_FMT_END();
        MY_LOGE("enque pass2 failed");
        AEE_ASSERT("ISP pass2 enque fail");

        if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS2) )
        {
            MY_LOGE("isp sync unlock pass2 failed");
        }
        return MFALSE;
    }
    CAM_TRACE_FMT_END();

    muEnqFrameCnt++;

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
processMDP(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    MUINT32 dstCount = 0, dstDataType[MAX_DST_PORT_NUM];
    ImgRequest outRequest[MAX_DST_PORT_NUM];
    vector<MUINT32> vDataDst = mDataDstSet;
    MUINT32 const dataDstCount = vDataDst.size()-1; // STEREO_DST is optional
    MSize const srcImgSize = buf->getImgSize();
    MSize const dstImgSize = getAlgoImgSize();
    MINT64 timestamp = buf->getTimestamp();

    // 0. get output buffer
    for(MUINT32 i = 0; i < vDataDst.size(); i++)
    {
        ICamBufHandler* pBufHdl = getBufferHandler( vDataDst[i] );
        MY_LOGD("vDataDst[%d]=%d", i, vDataDst[i]);
        if( pBufHdl && pBufHdl->dequeBuffer( vDataDst[i], &outRequest[dstCount]) )
        {
            MY_LOGD("get pBufHdl");
            dstDataType[dstCount] = vDataDst[i];
            dstCount++;
        }
        else
        {
            MY_LOGD("no pBufHdl");
            if ( STEREO_DST != vDataDst[i] )
            {   // ignore fd/ot dst buffer
                MY_LOGE("no dst buf (%d)", vDataDst[i]);
            }
        }
    }
    if( dstCount < dataDstCount )
    {
        MY_LOGW("not enough dst buffer, skip data(%d), buf(0x%x)", data, buf);
        handleReturnBuffer(data, (MUINTPTR)buf);

        //
        for( MUINT32 i = 0 ; i < dstCount ; i++ )
        {
            MUINT32 const dstData   = dstDataType[i];
            ICamBufHandler* pBufHdl = getBufferHandler(dstData);
            IImageBuffer const* pDstBuf = outRequest[i].mBuffer;
            if( pBufHdl && pBufHdl->enqueBuffer(dstData, pDstBuf) )
            {
                MY_LOGD("data %d, buf(%d:0x%x)", dstData, pDstBuf->getFD(), pDstBuf);
            }
            else
            {
                MY_LOGE("enqueBuf fail: data %d, buf(%d:0x%x)", dstData, pDstBuf->getFD(), pDstBuf);
            }
        }
        return MTRUE;
    }

    // 1. caculate crop
    IMAGE_RESOLUTION_INFO_STRUCT    algo_resolution = StereoSettingProvider::imageResolution();
    MUINT32 uRatioDenomerator = algo_resolution.uRatioDenomerator;  //16 or 4
    MUINT32 uRatioNumerator = algo_resolution.uRatioNumerator;    //9 or 3
    MSize bufSz = buf->getImgSize();
    int mdp_out_h = (bufSz.w*uRatioNumerator)/uRatioDenomerator;
    int32_t bBypassRatioCheck = 0;

    bBypassRatioCheck = ::property_get_int32("debug.camera.stereo.ratiom2", 0);

    MRect mdp_crop;
    if(( mdp_out_h == bufSz.h) || (bBypassRatioCheck))
    {
        mdp_crop = MRect(MPoint(0,0),bufSz);
    }
    else
    {
        mdp_crop.s.w = bufSz.w;
        mdp_crop.s.h = mdp_out_h & ~1;
        mdp_crop.p.x = 0;
        mdp_crop.p.y = ((bufSz.h - mdp_crop.s.h)>>1);
    }

    // 2. do mdp & post buffer
    for( MUINT32 i = 0 ; i < dstCount ; i++ )
    {
        IImageBuffer* pDstBuf = const_cast<IImageBuffer*>(outRequest[i].mBuffer);
        MY_LOGD("mdp dst: data %d, buf 0x%x, va/pa(0x%x/0x%x), size(%dx%d), tans %d",
                dstDataType[i], pDstBuf,
                pDstBuf->getBufVA(0), pDstBuf->getBufPA(0),
                pDstBuf->getImgSize().w, pDstBuf->getImgSize().h,
                outRequest[i].mTransform);

        pDstBuf->setTimestamp(timestamp);
        MBOOL bResult = mpImgTransform->execute(buf,
                                          pDstBuf, NULL,
                                          mdp_crop,
                                          outRequest[i].mTransform,
                                          0xFFFFFFFF);
        handlePostBuffer( dstDataType[i], (MUINTPTR)pDstBuf, 0 );
    }

    // 3. return
    handleReturnBuffer(data, (MUINTPTR)buf);    // return buffer

    FUNC_END;
    return MTRUE;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    PostBufInfo postBufData = {data, buf, ext};
    mlPostBufData.push_back(postBufData);

    muPostFrameCnt++;

    if( isReadyToEnque() )
    {
        triggerLoop();
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
handleP2Done(QParams& rParams)
{
    CAM_TRACE_CALL();
    CAM_TRACE_FMT_BEGIN("deqP2:%d", muDeqFrameCnt);
    MBOOL ret = MFALSE;
    MBOOL isZoom        = (rParams.mvCropRsInfo[0].mCropRect.p_integral.x != 0) ? MTRUE : MFALSE;
    MUINT32 magicNum    = rParams.mFrameNo;
    Vector<Input>::const_iterator iterIn;
    Vector<Output>::const_iterator iterOut;
    Vector<ModuleInfo>::const_iterator iterMInfo;
    //
    MY_LOGD("cnt %d #(0x%x) in(%d) out(%d) moduleData(%d) isZoom(%d)",
            muDeqFrameCnt,
            magicNum,
            rParams.mvIn.size(),
            rParams.mvOut.size(),
            rParams.mvModuleData.size(),
            isZoom);
    //
    if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync unlock pass2 failed");
        goto lbExit;
    }

    for( iterIn = rParams.mvIn.begin(); iterIn != rParams.mvIn.end(); iterIn++ )
    {
        PortID portId = iterIn->mPortID;
        portId.group = 0;
        MUINT32 nodeDataType = mapToNodeDataType( portId );
        MY_LOGD("In PortID(0x%08X), nodeData(%d)",portId, nodeDataType);
        handleReturnBuffer( nodeDataType, (MUINTPTR)iterIn->mBuffer);
    }
    //
    for( iterOut = rParams.mvOut.begin(); iterOut != rParams.mvOut.end(); iterOut++ )
    {
        PortID portId = iterOut->mPortID;
        portId.group = 0;
        MUINT32 nodeDataType = mapToNodeDataType( portId );
        MY_LOGD("Out PortID(0x%08X), nodeData(%d), bufInfo(0x%x), va/pa(0x%x/0x%x), ID(%d); SNode:handlePostBuffer sensor=%d, data=%d, buf=%p",
                portId, nodeDataType, iterOut->mBuffer, iterOut->mBuffer->getBufVA(0), iterOut->mBuffer->getBufPA(0), iterOut->mBuffer->getFD(),
                getSensorIdx(), nodeDataType, iterOut->mBuffer);

        Mutex::Autolock lock(mLock);
        handlePostBuffer( nodeDataType, (MUINTPTR)iterOut->mBuffer, 0 );
    }
    //
    {
        Mutex::Autolock lock(mLock);
        muDeqFrameCnt++;
        mCondDeque.broadcast();
    }
    //

    ret = MTRUE;
lbExit:
    CAM_TRACE_FMT_END();
    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MVOID
StereoNodeImpl::
pass2CbFunc(QParams& rParams)
{
     StereoNodeImpl* pNodeImpl = (StereoNodeImpl*)(rParams.mpCookie);
     pNodeImpl->handleP2Done(rParams);
}


/*******************************************************************************
*
********************************************************************************/
PortID
StereoNodeImpl::
mapToPortID(MUINT32 const nodeDataType)
{
    //hard-coded mapping
    switch(nodeDataType)
    {
        case STEREO_SRC:
            return IMGI;
            break;
        case STEREO_IMG:
            return WROTO;
            break;
        case STEREO_DST:
            return WDMAO;
        default:
            MY_LOGE("NoteType(%d) can not map to P2 port", nodeDataType);
            break;
    }
    return PortID();
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
StereoNodeImpl::
mapToNodeDataType(PortID const portId)
{
    //hard-coded mapping
    if( portId == IMGI )
        return STEREO_SRC;
    else if( portId == WDMAO )
        return STEREO_DST;
    else if ( portId == WROTO )
        return STEREO_IMG;

    MY_LOGE("portId(type=%d, index=%d, inout=%d) can not map to NodeDataType",
        portId.type, portId.index , portId.index );
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

