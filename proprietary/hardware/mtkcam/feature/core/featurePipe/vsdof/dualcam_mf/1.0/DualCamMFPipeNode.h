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

#ifndef _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_PIPE_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_PIPE_NODE_H_

#define LOG_TAG "DualCamMF/BaseNode"

// Standard C header file
#include <map>
#include <thread>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <DpIspStream.h>
#include <core/include/CamThreadNode.h>
#include <core/include/ImageBufferPool.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
//for image format convertion
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <mtkcam/feature/utils/ImageBufferUtils.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/IScenarioControl.h>
// isp5.0 srz setting
#include <mtkcam/drv/def/Dip_Notify_datatype.h>

// Module header file
#include <featurePipe/core/include/Timer.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

// Local header file
#include "DualCamMFPipe_Common.h"
#include "bufferPool/DCBufferPool.h"

// Module header file

#if MET_USER_EVENT_SUPPORT
#include <met_tag.h>
#endif

#define DCMF_FEAUTRE_UNIQUE_ID 910
#define UNUSED(expr) do { (void)(expr); } while (0)
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {

typedef android::sp<PipeRequest> PipeRequestPtr;

/*******************************************************************************
* Enum Define
********************************************************************************/
enum DualCamMFDataId {
    ID_INVALID,
    // 1
    ROOT_ENQUE,
    NORMAL_POSTVIEW,
    P2_DONE,
    SHUTTER,
    ERROR_OCCUR_NOTIFY,
    // 6
    FINAL_RESULT,
    PREPROCESS_TO_VENDOR,
    QUICK_POSTVIEW,
    BSS_RESULT_ORIGIN,
    BSS_RESULT_ORDERED,
    // 11
    PREPROCESS_YUV_DONE
};

enum DualCamMFProcessId {
    UNKNOWN,
    // 1
    FSRAW_TO_YUV_1,
    FSRAW_TO_YUV_2,
    RSRAW_TO_YUV_1,
    RSRAW_TO_YUV_2,
    FINAL_PROCESS,
    // 6
    VENDOR_JOB
};

/*******************************************************************************
* Structure Define
********************************************************************************/
class DualCamMFPipeNode;
struct EnquedBufPool
    :public Timer
{
public:
    DualCamMFPipeNode*                                  mpNode;
    PipeRequestPtr                                      mPipeRequest;
    PipeRequestPtr                                      mPipeRequest_BssSrc;
    ImgInfoMapPtr                                       mPImgInfo;
    DualCamMFProcessId                                  mProcessId;
    PQParam*                                            mpPQParam;
    ModuleInfo*                                         mpModuleInfo;

    Vector<SmartTuningBuffer>                           mEnqueTuningBuffer;
    KeyedVector<DualCamMFBufferID, SmartImageBuffer>    mEnquedSmartImgBufMap;
    KeyedVector<DualCamMFBufferID, SmartGraphicBuffer>  mEnquedSmartGBufMap;
    std::map< DualCamMFBufferID, sp<IImageBuffer> >     mEnquedIImgBufMap;
    std::map< DualCamMFBufferID, sp<mfll::IMfllImageBuffer> > mEnquedIMfllImageBufMap;

    EnquedBufPool(DualCamMFPipeNode* node, PipeRequestPtr pipeReq, ImgInfoMapPtr imgInfo, DualCamMFProcessId eProcessId)
    : mpNode(node)
    , mPipeRequest(pipeReq)
    , mPipeRequest_BssSrc(nullptr)
    , mPImgInfo(imgInfo)
    , mProcessId(eProcessId)
    , mpPQParam(nullptr)
    , mpModuleInfo(nullptr)
    {
    }

    EnquedBufPool(EnquedBufPool* pEnqueData, DualCamMFProcessId eProcessId)
    : mpNode(pEnqueData->mpNode)
    , mPipeRequest(pEnqueData->mPipeRequest)
    , mPipeRequest_BssSrc(pEnqueData->mPipeRequest_BssSrc)
    , mPImgInfo(pEnqueData->mPImgInfo)
    , mProcessId(eProcessId)
    , mpPQParam(nullptr)
    , mpModuleInfo(nullptr)
    {
    }

    ~EnquedBufPool()
    {
        if(mpPQParam != nullptr){
            CAM_LOGD("delete mpPQParam");
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
        if(mpModuleInfo != nullptr){
            CAM_LOGD("delete mpModuleInfo");
            if(mpModuleInfo->moduleStruct != nullptr){
                _SRZ_SIZE_INFO_* ptr = static_cast<_SRZ_SIZE_INFO_*>(mpModuleInfo->moduleStruct);
                delete ptr;
            }
            delete mpModuleInfo;
        }
    }

    MVOID addBuffData(DualCamMFBufferID bufID, SmartImageBuffer pSmBuf)
    { mEnquedSmartImgBufMap.add(bufID, pSmBuf); }

    MVOID addGBuffData(DualCamMFBufferID bufID, SmartGraphicBuffer pSmgBuf)
    { mEnquedSmartGBufMap.add(bufID, pSmgBuf); }

    MVOID addTuningData(SmartTuningBuffer pSmBuf)
    { mEnqueTuningBuffer.add(pSmBuf); }

    MVOID addIImageBuffData(DualCamMFBufferID bufID, sp<IImageBuffer> pBuf)
    { mEnquedIImgBufMap[bufID] = pBuf; }

    MVOID addIMfllImageBuffData(DualCamMFBufferID bufID, sp<mfll::IMfllImageBuffer> pBuf)
    { mEnquedIMfllImageBufMap[bufID] = pBuf; }

    MINT32 getReqNo() const
    {
        if(mPipeRequest != nullptr){
            return mPipeRequest->getRequestNo();
        }
        if(mPImgInfo != nullptr){
            return mPImgInfo->getRequestPtr()->getRequestNo();
        }
        CAM_LOGW("no available data!");
        return -1;
    }

    MVOID addPQData(PQParam* pPQParam)
    { mpPQParam = pPQParam;}

    MVOID addModuleInfo(ModuleInfo* pModuleInfo)
    { mpModuleInfo = pModuleInfo;}
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
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class DualCamMFPipeDataHandler
 * @brief DualCamMFPipeDataHandler
 */
class DualCamMFPipeDataHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
typedef DualCamMFDataId DataID;

// Constructor
// Copy constructor
// Create instance
// Destr instance

protected:
// Destructor
virtual ~DualCamMFPipeDataHandler(){};


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
 * @class DualCamMFPipeNode
 * @brief DualCamMFPipeNode
 */
class DualCamMFPipeNode:
    public DualCamMFPipeDataHandler,
    public CamThreadNode<DualCamMFPipeDataHandler>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
typedef CamGraph<DualCamMFPipeNode> Graph_T;
typedef DualCamMFPipeDataHandler Handler_T;
// Constructor
DualCamMFPipeNode(
            const char *name,
            Graph_T *graph);
// Copy constructor
// Create instance
// Destr instance

protected:
// Destructor
virtual ~DualCamMFPipeNode(){};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onDump(DataID id, ImgInfoMapPtr &data, const char* fileName=NULL, const char* postfix=NULL);
    virtual MBOOL onDump(DataID id, PipeRequestPtr &data, const char* fileName=NULL, const char* postfix=NULL);
    virtual MBOOL doBufferPoolAllocation(MUINT32 count = 1);

    MBOOL handleDump(IImageBuffer* pBuf, DualCamMFBufferID BID, MUINT32 iReqIdx);
    MBOOL handleDumpWithExtName(IImageBuffer* pBuf, DualCamMFBufferID BID, MUINT32 iReqIdx, string& extName);
    MBOOL handleData(DataID id, PipeRequestPtr pReq);
    MBOOL handleData(DataID id, ImgInfoMapPtr pReq);
    MBOOL handleData(DataID id, MINT32 data);
    MBOOL handleData(DataID id, android::Vector<MINT32>& data);
    MVOID dumpRequestData(DualCamMFBufferID BID, PipeRequestPtr pReq);

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
        IMetadata* pMeta_hal_out = nullptr);
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
    MBOOL shouldDumpForTuning(DualCamMFBufferID BID);
    /**
    * @brief check wheter is customer tuning mode
    * @return
    *-true indicates shoud yes, otherwise not
    */
    MBOOL isCustomerTuning();
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
    static const char* getBIDName(DualCamMFBufferID BID);
    /**
    * @brief getBIDPostfix
    * @return the postfix string
    */
    static const char* getBIDPostfix(DualCamMFBufferID BID);
    /**
    * @brief getBIDName
    * @return the name string
    */
    static const char* getProcessName(DualCamMFProcessId PID);
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
    * @brief to update src crop to fit into the aspect ratio of output image buffer
    * @return
    *-none
    */
    static MVOID updateSrcCropByOutputRatio(MRect& rSrcCrop, sp<IImageBuffer> src, sp<IImageBuffer> dst, MINT32 dstTrans = 0);
    /**
    * @brief to produce pass1 raw dump formal name string
    * @return
    *-none
    */
    MVOID setRawDumpExtNames(ImgInfoMapPtr imgInfoMap, const DualCamMFBufferID& bid);
    /**
    * @brief to fill in PQ params
    * @return
    *-none
    */
    static MVOID fillInPQParam(PQParam* const pPQParam, MUINT32 processId);
    /**
    * @brief to fill in Srz params
    * @return
    *-none
    */
    MBOOL fillInSrzParam(ModuleInfo* const pModuleInfo, MUINT32 processId, sp<IImageBuffer> pBuf_lcso, sp<IImageBuffer> pBuf_raw);
    /**
    * @brief to get the fov ratio crop
    * @return
    *-none
    */
    static MRect getFovRatioCrop(const MSize srcImgSize, const MSize dstImgSize, MBOOL isCropMain);
    /**
    * @brief to add an exlusive job to this node,
    * itblocks of there is another exclusive job ru
    * @return
    *-true indicates wait and add job success, otherwise failed
    */
    MBOOL waitAndAddExclusiveJob(const MUINT32 reqNo);
    /**
    * @brief wait for previous exlusive job is finished
    * @return
    *-true indicates wait done, otherwise timeout
    */
    MBOOL finishExclusiveJob(const MUINT32 reqNo);
    /**
    * @brief get iso vaule from metadata
    * @return
    *-none
    */
    static MINT32 getISOFromMeta(const IMetadata* const pHalMeta);
    /**
    * @brief get exp vaule from metadata
    * @return
    *-none
    */
    static MINT32 getEXPFromMeta(const IMetadata* const pHalMeta);
    /**
    * @brief img format string
    * @return
    *-none
    */
    static const char* getFormatString(const IImageBuffer* const pBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Public Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MBOOL                               mbProfileLog = MFALSE;
    MBOOL                               mbDebugLog = MFALSE;

    MINT32                              mSensorIdx_Main1 = -1;
    MINT32                              mSensorIdx_Main2 = -1;

    MINT32                              mBayerOrder_main1 = -1;
    MINT32                              mBayerOrder_main2 = -1;

    MUINT                               miDumpBufSize     = 0;
    MUINT                               miDumpStartIdx    = 0;
    MUINT                               miTuningDump      = 0;
    MUINT                               miCustomerTuning  = 0;

    mutable Mutex                       mLock;
    mutable Condition                   mCond;
    std::set<MINT32>                    mProcessingRequestSet;

    wp<P2Operator>                      mwpP2Operator = nullptr;

    MINT32                              mModuleTrans = -1;

    map<EDIPInfoEnum, MUINT32>          mDipInfo;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
};

/*
* @class AutoScenCtrl
* @brief AutoScenCtrl
+ */
class AutoScenCtrl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Constructor
    AutoScenCtrl(
        const char *userName,
        MINT32 openId
    );
    // Copy constructor
    // Create instance
    // Destr instance
    // Destructor
    ~AutoScenCtrl();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const char*          mUserName;
    sp<IScenarioControl> mpScenarioCtrl = nullptr;
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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        CAM_LOGD("name: %s (0x%x)", mName.c_str(), this);

        if(taskTable.empty())
            CAM_LOGE("empty task table.");
    }

    ~TaskQueue()
    {
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        CAM_LOGD("name: %s (0x%x)", mName.c_str(), this);
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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        MINT32 reqNo = data->getReqNo();

        std::lock_guard<std::mutex> _l(mLocker);
        mTaskQueue.push(data);
        CAM_LOGD("add data, reqID: %d, Data: 0x%x, ProcessId: %d", reqNo, data, data->mProcessId);
        mCondition.notify_one();
    }
    /**
    * @brief to run the task queue
    * @return
    *-none
    */
    inline MVOID run()
    {
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        std::lock_guard<std::mutex> _l(mLocker);

        if(mHadRun)
        {
            CAM_LOGE("just can call run one time during the objec life-cycle, name: %s (0x%x)", mName.c_str(), this);
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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        if(mpWorker->joinable())
        {
            CAM_LOGD("joinable and wait worker finish task.");
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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        while(true)
        {
            CAM_LOGD("loop: +");
            EnquedBufPool* data;
            {
                std::unique_lock<std::mutex> _l(mLocker);
                if(mExitPending)
                {
                    CAM_LOGD("exit pendind (before wait)");
                    break;
                }

                if(mTaskQueue.empty())
                {
                    CAM_LOGD("wait: +");
                    // we must check the wait-up flags to advoid the spurious wakeup
                    mCondition.wait(_l, [this] { return (mTaskQueue.size() > 0) || mExitPending; });
                    CAM_LOGD("wait: -");
                }

                if(mExitPending)
                {
                    CAM_LOGD("exit pendind (before wait)");
                    break;
                }

                data = mTaskQueue.front();
                mTaskQueue.pop();

                MINT32 reqNo = data->getReqNo();
                CAM_LOGD("get data, reqID: %d, Data: 0x%x, ProcessId: %d", reqNo, data, data->mProcessId);
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
        ScopeLogger logger(LOG_TAG, __FUNCTION__);

        auto iter = mTaskTable.find(data->mProcessId);
        if(iter == mTaskTable.end())
        {
            CAM_LOGE("process id is not found: %d (0x%x)", data->mProcessId, data);
            return;
        }
        Task task = mTaskTable.at(data->mProcessId);

        CAM_LOGD("handle data (%x) +", data);
        (mOwnerPtr->*task)(data);
        CAM_LOGD("handle data (%x) -", data);
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
* Namespace end.
********************************************************************************/
};
};
};
};

#endif
