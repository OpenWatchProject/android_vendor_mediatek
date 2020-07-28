/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 * @file BMDeNoisePipeNode.h
 * @brief This is the definition of Bayer+White de-noise pipe nodes
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_BMDeNoise_PIPE_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_BMDeNoise_PIPE_NODE_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <core/include/CamThreadNode.h>
#include <core/include/ImageBufferPool.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
//for image format convertion
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <mtkcam/feature/utils/ImageBufferUtils.h>

// Module header file
#include <featurePipe/core/include/Timer.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

// Local header file
#include "BMDeNoisePipe_Common.h"
#include "bufferPool/BMBufferPool.h"

#if MET_USER_EVENT_SUPPORT
#include <met_tag.h>
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef android::sp<EffectRequest> EffectRequestPtr;
typedef android::sp<EffectFrameInfo> FrameInfoPtr;
typedef android::sp<PipeRequest> PipeRequestPtr;

/*******************************************************************************
* Enum Define
********************************************************************************/
enum BMDeNoiseDataId {
    ID_INVALID,
    //0
    ROOT_ENQUE,
    ROOT_TO_PREPROCESS,
    ROOT_TO_QUICK_PREPROCESS,
    ROOT_TO_DENOISE,
    P2AFM_TO_N3D_FEFM_CCin,
    //5
    N3D_TO_WPE_WARP_DATA,
    WPE_TO_DPE_WARP_RESULT,
    DPE_TO_DENOISE_DMP_RESULT,
    PREPROCESS_TO_DENOISE,
    BMDENOISE_RESULT_TO_MNR,
    //10
    BMDENOISE_RESULT_TO_SWNR,
    BMDENOISE_RESULT,
    DENOISE_TO_POSTPROCESS,
    MNR_RESULT,
    SWNR_RESULT,
    //15
    BM3RDPARTY_RESULT,
    DENOISE_FINAL_RESULT_OUT,
    ROOT_TO_P2AFM,
    TO_DUMP_RAWS,
    ROOT_HR_TO_BSS,
    //20
    SWNR_TO_HELPER,
    P2_DONE,
    SHUTTER,
    ERROR_OCCUR_NOTIFY,
    ROOT_HR_TO_BAYER_PREPROCESS,
    //25
    BAYER_TO_MONO_PREPROCESS,
    ROOT_HR_TO_FUSION,
    ENTER_HIGH_PERF,
    EXIT_HIGH_PERF,
    BSS_TO_P2AFM,
    //30
    MONO_PREPROCESS_TO_FUSION,
    FUSION_TO_MFNR,
    BSS_RESULT,
};

enum BMDeNoiseProcessId {
    UNKNOWN,
    BAYER_TO_YUV,
    MONO_TO_YUV,
    BAYER_PREPROCESS,
    MONO_PREPROCESS,
    P2AFM,
    DPE,
    DO_GGM,
    YUV_MERGE,
    FINAL_PROCESS,
};

/*******************************************************************************
* Structure Define
********************************************************************************/
class BMDeNoisePipeNode;
struct EnquedBufPool
    :public Timer
{
public:
    BMDeNoisePipeNode*                                  mpNode;
    PipeRequestPtr                                      mPipeRequest;
    ImgInfoMapPtr                                       mPImgInfo;
    BMDeNoiseProcessId                                  mProcessId;

    Vector<SmartTuningBuffer>                           mEnqueTuningBuffer;
    KeyedVector<BMDeNoiseBufferID, SmartImageBuffer>    mEnquedSmartImgBufMap;
    KeyedVector<BMDeNoiseBufferID, SmartGraphicBuffer>  mEnquedSmartGBufMap;
    KeyedVector<BMDeNoiseBufferID, sp<IImageBuffer> >   mEnquedIImgBufMap;

    EnquedBufPool(BMDeNoisePipeNode* node, PipeRequestPtr pipeReq, ImgInfoMapPtr imgInfo, BMDeNoiseProcessId eProcessId)
    : mpNode(node)
    , mPipeRequest(pipeReq)
    , mPImgInfo(imgInfo)
    , mProcessId(eProcessId)
    {
    }

    EnquedBufPool(EnquedBufPool* pEnqueData, BMDeNoiseProcessId eProcessId)
    : mpNode(pEnqueData->mpNode)
    , mPipeRequest(pEnqueData->mPipeRequest)
    , mPImgInfo(pEnqueData->mPImgInfo)
    , mProcessId(eProcessId)
    {
    }

    MVOID addBuffData(BMDeNoiseBufferID bufID, SmartImageBuffer pSmBuf)
    { mEnquedSmartImgBufMap.add(bufID, pSmBuf); }

    MVOID addGBuffData(BMDeNoiseBufferID bufID, SmartGraphicBuffer pSmgBuf)
    { mEnquedSmartGBufMap.add(bufID, pSmgBuf); }

    MVOID addTuningData(SmartTuningBuffer pSmBuf)
    { mEnqueTuningBuffer.add(pSmBuf); }

    MVOID addIImageBuffData(BMDeNoiseBufferID bufID, sp<IImageBuffer> pBuf)
    { mEnquedIImgBufMap.add(bufID, pBuf); }
};

struct ISPTuningConfig
{
    IMetadata* pInAppMeta;
    IMetadata* pInHalMeta;
    NS3Av3::IHal3A* p3AHAL;
    MBOOL bInputResizeRaw;
    MINT32 reqNo;
};

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class BMDeNoisePipeDataHandler
 * @brief BMDeNoisePipeDataHandler
 */
class BMDeNoisePipeDataHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
typedef BMDeNoiseDataId DataID;

// Constructor
// Copy constructor
// Create instance
// Destr instance

protected:
// Destructor
virtual ~BMDeNoisePipeDataHandler(){};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: PipeRequestPtr
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, PipeRequestPtr &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: EffectRequestPtr
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, EffectRequestPtr &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: FrameInfoPtr
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, FrameInfoPtr &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: SmartImageBuffer
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, SmartImageBuffer &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: SmartImageBuffer
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, ImgInfoMapPtr &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: MINT32
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, MINT32 &data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: Vector<MINT32>
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, android::Vector<MINT32>& data)
    {
        UNUSED(id);
        UNUSED(data);
        return MFALSE;
    }
    /**
     * @brief ID2Name
     * @param [in] id: dataID
     * @return
     *-the string name of the DataID
     */
    static const char* ID2Name(DataID id);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Public Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
};




/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class BMDeNoisePipeNode
 * @brief BMDeNoisePipeNode
 */
class BMDeNoisePipeNode:
    public BMDeNoisePipeDataHandler,
    public CamThreadNode<BMDeNoisePipeDataHandler>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
typedef CamGraph<BMDeNoisePipeNode> Graph_T;
typedef BMDeNoisePipeDataHandler Handler_T;
// Constructor
BMDeNoisePipeNode(
            const char *name,
            Graph_T *graph);
// Copy constructor
// Create instance
// Destr instance

protected:
// Destructor
virtual ~BMDeNoisePipeNode(){};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief enableDumpImage
     * @param [in] flag: indicate whether to dump or not
     * @return
     *-void
     */
    void enableDumpImage(MBOOL flag);

    virtual MBOOL onDump(DataID id, ImgInfoMapPtr &data, const char* fileName=NULL, const char* postfix=NULL);
    virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);

    MBOOL handleDump(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx);
    MBOOL handleData(DataID id, PipeRequestPtr pReq);
    MBOOL handleData(DataID id, EffectRequestPtr pReq);
    MBOOL handleData(DataID id, ImgInfoMapPtr pReq);
    MBOOL handleData(DataID id, MINT32 data);
    MBOOL handleData(DataID id, android::Vector<MINT32>& data);
    MVOID dumpRequestData(BMDeNoiseBufferID BID, PipeRequestPtr pReq);

    MVOID setP2Operator(sp<P2Operator> spP2Op);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    /**
     * @brief onInit
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL onInit()          override;
    /**
     * @brief onUninit
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL onUninit()        override    { return MTRUE; }
    /**
     * @brief onThreadStart
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL onThreadStart()   override    { return MTRUE; }
    /**
     * @brief onThreadStop
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL onThreadStop()    override    { return MTRUE; }
    /**
     * @brief onThreadLoop
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onThreadLoop() = 0;
    /**
     * @brief getBIDName
     * @return the name string
     */
    const char* getBIDName(BMDeNoiseBufferID BID);
    /**
     * @brief getBIDPostfix
     * @return the postfix string
     */
    const char* getBIDPostfix(BMDeNoiseBufferID BID);
    /**
     * @brief getBIDName
     * @return the name string
     */
    const char* getProcessName(BMDeNoiseProcessId PID);
    /**
     * @brief to convert image buffer format
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL formatConverter(IImageBuffer *imgSrc, IImageBuffer *imgDst);
    /**
     * @brief to handle release buffer
     * @return
     *-true indicates success, otherwise indicates fail
     */
     template<class T, class K>
     MBOOL handleReleaseBufferPool(MINT32 reqId, T& pool, K& config, const char* userName);
     /**
     * @brief to handle allocate buffer
     * @return
     *-true indicates success, otherwise indicates fail
     */
     template<class T, class K>
     MBOOL handleAllocateBufferPool(MINT32 reqId, T& pool, K& config, const char* userName, MBOOL pureInit = MFALSE);
     /**
     * @brief to print MET tags
     * @return
     */
     MVOID MET_START(const char* tag_name);
     MVOID MET_END(const char* tag_name);
     /**
     * @brief to init buffer pool configs
     * @return
     *-none
     */
     MVOID initBufferPool();
     /**
     * @brief to init module orientation setting
     * @return
     *-none
     */
     MBOOL prepareModuleSettings();
     /**
     * @brief to transform eTransform to degree
     * @return
     *-none
     */
     MINT32 eTransformToDegree(MINT32 eTrans);
     /**
     * @brief to transform degree to eTransform
     * @return
     *-none
     */
     MINT32 DegreeToeTransform(MINT32 degree);

     /**
     * @brief to apply ISP tuning
     * @return TuningParam
     *-none
     */
     NS3Av3::TuningParam applyISPTuning(
        const char* name,
        SmartTuningBuffer& targetTuningBuf,
        const ISPTuningConfig& ispConfig,
        MBOOL useDefault = MFALSE,
        MBOOL updateHalMeta = MFALSE
    );

     /**
     * @brief to acquire jpeg transfom from app meta
     * @return the eTransform for jpeg
     *-none
     */
     MINT32 getJpegRotation(IMetadata* pMeta);
     /**
     * @brief check the buffer of DataID will dump or not
     * @return
     */
     virtual MBOOL checkToDump(DataID id);
     /**
     * @brief check wheter we should do data dump for this reqeust
     * @return
     *-true indicates shoud dump, otherwise not
     */
     MBOOL shouldDumpRequest(PipeRequestPtr pReq);
     /**
     * @brief check wheter we should do data dump for this reqeust
     * @return
     *-true indicates shoud dump, otherwise not
     */
     MBOOL shouldDumpForTuning(BMDeNoiseBufferID BID);
     /**
     * @brief to separate a floating number to integer and fractional part.
     * both parts will be presented in integer format
     * @return
     *-none
     */
     MVOID separateFloat(float src, MINT32& integer, MINT32& fraction);

     /**
     * @brief to do system call
     * @return
     *-none
     */
     static MVOID systemCall(const char* cmd);

     /**
     * @brief to create a image buffer heap in specified format. Only support formats with 1 plane.
     * @return
     *-sp<ImageBufferHeap> the image buffer heap with new format
     */
     sp<ImageBufferHeap> createImageBufferHeapInFormat(sp<IImageBuffer> pImgBuf, EImageFormat eFmt, MSize ROI);

     /**
     * @brief to update src crop to fit into the aspect ratio of output image buffer
     * @return
     *-none
     */
     MVOID updateSrcCropByOutputRatio(MRect& rSrcCrop, sp<IImageBuffer> src, sp<IImageBuffer> dst);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Public Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    MBOOL                               mbDumpImageBuffer = MFALSE;
    MBOOL                               mbProfileLog = MFALSE;
    MBOOL                               mbDebugLog = MFALSE;

    MINT32                              mSensorIdx_Main1 = -1;
    MINT32                              mSensorIdx_Main2 = -1;

    MUINT                               miDumpBufSize = 0;
    MUINT                               miDumpStartIdx= 0;
    MUINT                               miTuningDump = 0;

    mutable Mutex                       mLock;
    MBOOL                               mbRequstReleaseBuffer = MFALSE;
    std::set<MINT32>                    mProcessingRequestSet;

    wp<P2Operator>                      mwpP2Operator = nullptr;

    MINT32                              mModuleTrans = -1;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
};

/*******************************************************************************
* Template Function Implementation
********************************************************************************/
template<class T, class K>
MBOOL
BMDeNoisePipeNode::
handleReleaseBufferPool(MINT32 reqId, T& pool, K& config, const char* userName)
{
    MY_LOGD("[%s]%s +", userName, __FUNCTION__);

    Timer localTimer;
    localTimer.start();

    Mutex::Autolock _l(mLock);

    if(reqId == -1){
        MY_LOGD("request release buffer");
        mbRequstReleaseBuffer = MTRUE;
    }else{
        MY_LOGD("remove procesing request");
        if(mProcessingRequestSet.count(reqId) == 0){
            MY_LOGD("cant find req:%d in mProcessingRequestSet!", reqId);
            return MFALSE;
        }else{
            mProcessingRequestSet.erase(reqId);
        }
    }

    if(mProcessingRequestSet.empty() && mbRequstReleaseBuffer){
        pool.uninit();
        mbRequstReleaseBuffer = MFALSE;
    }

    localTimer.stop();

    MY_LOGD("[%s]%s (%d)ms -", userName, __FUNCTION__, localTimer.getElapsed());
    return MTRUE;
}

template<class T, class K>
MBOOL
BMDeNoisePipeNode::
handleAllocateBufferPool(MINT32 reqId, T& pool, K& config, const char* userName, MBOOL pureInit)
{
    MY_LOGD("[%s]%s pureInit:%d +", userName, __FUNCTION__, pureInit);

    Timer localTimer;
    localTimer.start();

    Mutex::Autolock _l(mLock);

    if(!pool.mbInit){
        // BufferPoolSet init
        MY_LOGD("[%s]=>BufferPoolSet::init", userName);
        pool.init(config);
    }

    if(!pureInit){
        if(mProcessingRequestSet.count(reqId) == 0){
            mProcessingRequestSet.insert(reqId);
        }else{
            MY_LOGE("reqId:%d already in mProcessingRequestSet, should not happened!", reqId);
            return MFALSE;
        }
    }

    localTimer.stop();

    MY_LOGD("[%s]%s (%d)ms -", userName, __FUNCTION__, localTimer.getElapsed());
    return MTRUE;
}

/*******************************************************************************
* Namespace end.
********************************************************************************/
};
};
};


#endif
