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
#include <map>
#include <thread>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <core/include/CamThreadNode.h>
#include <core/include/ImageBufferPool.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
// MDP/PQ
#include <DpIspStream.h>
//for image format convertion
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <mtkcam/feature/utils/ImageBufferUtils.h>

// Module header file
#include <featurePipe/core/include/Timer.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

// Local header file
#include "BMDeNoisePipe_Common.h"
#include "bufferPool/BMBufferPool.h"

// Module header file

#define PIPE_MODULE_TAG "BMDeNoisePipe"
#define PIPE_CLASS_TAG "Node"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#if MET_USER_EVENT_SUPPORT
#include <met_tag.h>
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")

#define BMDN_FEAUTRE_UNIQUE_ID 918
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
    MFHR_EARLY_RELEASE,
    POSTVIEW_DONE
};

enum BMDeNoiseProcessId {
    UNKNOWN,
    BAYER_TO_YUV,
    BAYER_COLOR_EFFECT,
    MONO_TO_YUV,
    BAYER_PREPROCESS,
    MONO_PREPROCESS,
    P2AFM,
    DPE,
    DO_GGM,
    YUV_MERGE,
    FINAL_PROCESS
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
    PQParam*                                            mpPQParam;

    Vector<SmartTuningBuffer>                           mEnqueTuningBuffer;
    KeyedVector<BMDeNoiseBufferID, SmartImageBuffer>    mEnquedSmartImgBufMap;
    KeyedVector<BMDeNoiseBufferID, SmartGraphicBuffer>  mEnquedSmartGBufMap;
    std::map< BMDeNoiseBufferID, sp<IImageBuffer> >     mEnquedIImgBufMap;
    std::map< BMDeNoiseBufferID, sp<mfll::IMfllImageBuffer> > mEnquedIMfllImageBufMap;

    EnquedBufPool(BMDeNoisePipeNode* node, PipeRequestPtr pipeReq, ImgInfoMapPtr imgInfo, BMDeNoiseProcessId eProcessId)
    : mpNode(node)
    , mPipeRequest(pipeReq)
    , mPImgInfo(imgInfo)
    , mProcessId(eProcessId)
    , mpPQParam(nullptr)
    {
    }

    EnquedBufPool(EnquedBufPool* pEnqueData, BMDeNoiseProcessId eProcessId)
    : mpNode(pEnqueData->mpNode)
    , mPipeRequest(pEnqueData->mPipeRequest)
    , mPImgInfo(pEnqueData->mPImgInfo)
    , mProcessId(eProcessId)
    {
    }

    ~EnquedBufPool()
    {
        if(mpPQParam != nullptr){
            MY_LOGD("delete mpPQParam");
            if(mpPQParam->WDMAPQParam != nullptr){
                DpPqParam* ptr = static_cast<DpPqParam*>(mpPQParam->WDMAPQParam);
                delete ptr;
            }
            if(mpPQParam->WROTPQParam != nullptr){
                DpPqParam* ptr = static_cast<DpPqParam*>(mpPQParam->WROTPQParam);
                delete ptr;
            }
            delete mpPQParam;
        }
    }

    MVOID addBuffData(BMDeNoiseBufferID bufID, SmartImageBuffer pSmBuf)
    { mEnquedSmartImgBufMap.add(bufID, pSmBuf); }

    MVOID addGBuffData(BMDeNoiseBufferID bufID, SmartGraphicBuffer pSmgBuf)
    { mEnquedSmartGBufMap.add(bufID, pSmgBuf); }

    MVOID addTuningData(SmartTuningBuffer pSmBuf)
    { mEnqueTuningBuffer.add(pSmBuf); }

    MVOID addIImageBuffData(BMDeNoiseBufferID bufID, sp<IImageBuffer> pBuf)
    { mEnquedIImgBufMap[bufID] = pBuf; }

    MVOID addIMfllImageBuffData(BMDeNoiseBufferID bufID, sp<mfll::IMfllImageBuffer> pBuf)
    { mEnquedIMfllImageBufMap[bufID] = pBuf; }

    MVOID addPQData(PQParam* pPQParam)
    { mpPQParam = pPQParam;}
};

struct ISPTuningConfig
{
    IMetadata* pInAppMeta;
    IMetadata* pInHalMeta;
    NS3Av3::IHal3A* p3AHAL;
    MBOOL bInputResizeRaw;
    MINT32 reqNo;
    IImageBuffer* pLcsBuf = nullptr;
};

struct BM_BSS_RESULT
{
    MINT32 frameIdx = -1;
    MINT32 x = -1;
    MINT32 y = -1;
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
     * @brief onData
     * @param [in] id: dataID
     * @param [in] data: Vector<BM_BSS_RESULT>
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL onData(DataID id, android::Vector<BM_BSS_RESULT>& data)
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
    MBOOL handleDumpWithExtName(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx, string& extName);
    MBOOL handleData(DataID id, PipeRequestPtr pReq);
    MBOOL handleData(DataID id, EffectRequestPtr pReq);
    MBOOL handleData(DataID id, ImgInfoMapPtr pReq);
    MBOOL handleData(DataID id, MINT32 data);
    MBOOL handleData(DataID id, android::Vector<MINT32>& data);
    MBOOL handleData(DataID id, android::Vector<BM_BSS_RESULT>& data);
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
    * @brief to apply ISP tuning
    * @return TuningParam
    *-none
    */
    NS3Av3::TuningParam applyISPTuning(
        const char* name,
        SmartTuningBuffer& targetTuningBuf,
        const ISPTuningConfig& ispConfig,
        MBOOL useDefault = MFALSE,
        MBOOL updateHalMeta = MFALSE);
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
    * @brief to print MET tags
    * @return
    */
    static MVOID MET_START(const char* tag_name);
    static MVOID MET_END(const char* tag_name);
    /**
    * @brief getBIDName
    * @return the name string
    */
    static const char* getBIDName(BMDeNoiseBufferID BID);
    /**
    * @brief getBIDPostfix
    * @return the postfix string
    */
    static const char* getBIDPostfix(BMDeNoiseBufferID BID);
    /**
    * @brief getBIDName
    * @return the name string
    */
    static const char* getProcessName(BMDeNoiseProcessId PID);
    /**
    * @brief to transform ENUM_ROTATION to eTransform
    * @return
    *-none
    */
    static MINT32 eRotationToeTransform(ENUM_ROTATION rotation);
    /**
    * @brief to transform eTransform to ENUM_ROTATION
    * @return
    *-none
    */
    static ENUM_ROTATION eTransformToRotation(MINT32 eTrans);
    /**
    * @brief to transform eTransform to degree
    * @return
    *-none
    */
    static MINT32 eTransformToDegree(MINT32 eTrans);
    /**
    * @brief to transform degree to eTransform
    * @return
    *-none
    */
    static MINT32 DegreeToeTransform(MINT32 degree);
    /**
    * @brief to convert image buffer format
    * @return
    *-true indicates success, otherwise indicates fail
    */
    static MBOOL formatConverter(IImageBuffer *imgSrc, IImageBuffer *imgDst);
    /**
    * @brief to acquire jpeg transfom from app meta
    * @return the eTransform for jpeg
    *-none
    */
    static MINT32 getJpegRotation(IMetadata* pMeta);
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
    static sp<ImageBufferHeap> createImageBufferHeapInFormat(sp<IImageBuffer> pImgBuf, EImageFormat eFmt, MSize ROI);
    /**
    * @brief to update src crop to fit into the aspect ratio of output image buffer
    * @return
    *-none
    */
    static MVOID updateSrcCropByOutputRatio(MRect& rSrcCrop, sp<IImageBuffer> src, sp<IImageBuffer> dst);
    /**
    * @brief to produce pass1 raw dump formal name string
    * @return
    *-none
    */
    MVOID setRawDumpExtNames(ImgInfoMapPtr imgInfoMap, const BMDeNoiseBufferID& bid);
    /**
    * @brief to fill in PQ params
    * @return
    *-none
    */
    static MVOID fillInPQParam(PQParam* const pPQParam, MUINT32 processId);
    /**
    * @brief to get the fov ratio crop
    * @return
    *-none
    */
    static MRect getFovRatioCrop(const MSize srcImgSize, const MSize dstImgSize, MBOOL isCropMain);

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

    MINT32                              mBayerOrder_main1 = -1;
    MINT32                              mBayerOrder_main2 = -1;

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

/**
 * @class TaskQueue
 * @brief TaskQueue
 */
template<typename TOwner>
class TaskQueue final
{
public:
    typedef MVOID (TOwner::*Task)(EnquedBufPool*);
    typedef std::map<MINT32, Task> TaskTable;
    typedef std::pair<MINT32, Task> TaskPair;

public:
    TaskQueue(TaskQueue& other) = delete;
    TaskQueue(TaskQueue&& other) = delete;
    TaskQueue& operator=(const TaskQueue& other) = delete;
    TaskQueue& operator=(const TaskQueue&& other) = delete;

public:
    TaskQueue(TOwner* owner, const string name, const TaskTable& taskTable)
    : mOwnerPtr(owner)
    , mName(name)
    , mTaskTable(taskTable)
    , mHadRun(false)
    , mIsRunning(false)
    , mExitPending(false)
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        MY_LOGD("name: %s (0x%x)", mName.c_str(), this);

        if(taskTable.empty())
            MY_LOGE("empty task table.");
    }

    ~TaskQueue()
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        MY_LOGD("name: %s (0x%x)", mName.c_str(), this);
        requestExit();
        join();
    }
    /**
    * @brief to know the TaskQueue is running or not
    * @return
    *-none
    */
    inline MBOOL isRunning()
    {
        std::lock_guard<std::mutex> _l(mLocker);
        return mIsRunning;
    }
    /**
    * @brief to add task data
    * @return
    *-none
    */
    MVOID addTaskData(EnquedBufPool* data)
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        MINT32 reqNo = data->mPImgInfo->getRequestPtr()->getRequestNo();

        std::lock_guard<std::mutex> _l(mLocker);
        mTaskQueue.push(data);
        MY_LOGD("add data, reqID: %d, Data: 0x%x, ProcessId: %d", reqNo, data, data->mProcessId);
        mCondition.notify_one();
    }
    /**
    * @brief to run the task queue
    * @return
    *-none
    */
    inline MVOID run()
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        std::lock_guard<std::mutex> _l(mLocker);

        if(mHadRun)
        {
            MY_LOGE("just can call run one time during the objec life-cycle, name: %s (0x%x)", mName.c_str(), this);
            return;
        }

        mHadRun = true;
        mIsRunning = true;
        mExitPending = false;
        mpWorker = make_unique<std::thread>(std::bind(&TaskQueue::workerLoop, this));
    }
    /**
    * @brief to request the TaskQueue exit, non-blocking
    * @return
    *-none
    */
    inline MVOID requestExit()
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        std::lock_guard<std::mutex> _l(mLocker);
        mExitPending = true;
        mCondition.notify_one();
    }
    /**
    * @brief to wait (blocking) until the TaskQueue exit
    * @return
    *-none
    */
    inline MVOID join()
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        if(mpWorker->joinable())
        {
            MY_LOGD("joinable and wait worker finish task.");
            mpWorker->join();
        }
    }
private:
    /**
    * @brief to do work for each task
    * @return
    *-none
    */
    void workerLoop()
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        while(true)
        {
            MY_LOGD("loop: +");
            EnquedBufPool* data;
            {
                std::unique_lock<std::mutex> _l(mLocker);
                if(mExitPending)
                {
                    MY_LOGD("exit pendind (before wait)");
                    break;
                }

                if(mTaskQueue.empty())
                {
                    MY_LOGD("wait: +");
                    // we must check the wait-up flags to advoid the spurious wakeup
                    mCondition.wait(_l, [this] { return (mTaskQueue.size() > 0) || mExitPending; });
                    MY_LOGD("wait: -");
                }

                if(mExitPending)
                {
                    MY_LOGD("exit pendind (before wait)");
                    break;
                }

                data = mTaskQueue.front();
                mTaskQueue.pop();

                MINT32 reqNo = data->mPImgInfo->getRequestPtr()->getRequestNo();
                MY_LOGD("get data, reqID: %d, Data: 0x%x, ProcessId: %d", reqNo, data, data->mProcessId);
            }
            handleTask(data);
        }
    }
    /**
    * @brief to handle the task by argument
    * @return
    *-none
    */
    MVOID handleTask(EnquedBufPool* data)
    {
        ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

        auto iter = mTaskTable.find(data->mProcessId);
        if(iter == mTaskTable.end())
        {
            MY_LOGE("process id is not found: %d (0x%x)", data->mProcessId, data);
            return;
        }
        Task task = mTaskTable.at(data->mProcessId);

        MY_LOGD("handle data (%x) +", data);
        (mOwnerPtr->*task)(data);
        MY_LOGD("handle data (%x) -", data);
    }
private:
    TOwner* const       mOwnerPtr;
    const std::string   mName;
    const TaskTable     mTaskTable;

    MBOOL mHadRun;
    MBOOL mIsRunning;
    MBOOL mExitPending;

    std::unique_ptr<std::thread>    mpWorker;
    std::mutex                      mLocker;
    std::condition_variable         mCondition;

    std::queue<EnquedBufPool*>      mTaskQueue;
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
