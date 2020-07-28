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

#ifndef _MTK_CAMERA_FEATURE_PIPE_IDUALCAM_MULTIFRAME_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_IDUALCAM_MULTIFRAME_PIPE_H_

// Standard C header file
#include <map>

// Android system/core header file
#include <utils/RefBase.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/utils/metadata/IMetadata.h>

// Module header file

// Local header file

using namespace android;

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF{
/*******************************************************************************
* Enum Define
********************************************************************************/
enum class DualCamMFType
{
    TYPE_DCMF_unknown,
    TYPE_DCMF_3rdParty,
    TYPE_DCMF_3rdParty_HDR
};

enum class DualCamMFBufferID{
    BID_INVALID = 0,
    // 1
    BID_REQUEST_STATE,
    BID_INPUT_FSRAW_1,
    BID_INPUT_FSRAW_2,
    BID_INPUT_RSRAW_1,
    BID_INPUT_RSRAW_2,
    // 6
    BID_POSTVIEW,
    BID_FS_YUV_1,
    BID_FS_YUV_2,
    BID_RS_YUV_1,
    BID_RS_YUV_2,
    // 11
    BID_LCS_1,
    BID_LCS_2,
    BID_META_IN_APP,
    BID_META_IN_HAL,
    BID_META_IN_HAL_2,
    // 16
    BID_META_OUT_APP,
    BID_META_OUT_HAL,
    BID_SM_YUV,
    BID_META_OUT_HAL_2,
} ;

enum class DualCamMFParamID{
    // 1
    PID_IS_MAIN,
    PID_IS_LAST,
    PID_LCSO_CONTENT_WIDTH,
    PID_LCSO_CONTENT_HEIGHT,
    PID_FRAME_NUM,
    // 6
    PID_ENABLE_MFB,
    PID_ENABLE_HDR,
};

enum class eStopWatchType
{
    eSTOPWATCHTYPE_MANUAL,
    eSTOPWATCHTYPE_AUTO
};

/*******************************************************************************
* Structure Define
********************************************************************************/


/*******************************************************************************
* Class Define
********************************************************************************/


/******************************************************************************
 *  StopWatch Interface.
 ******************************************************************************/
class IStopWatch
    : public android::RefBase
{
public:
    virtual void                Start()             = 0;
    virtual void                Stop()              = 0;
    virtual                     ~IStopWatch(){};

    IStopWatch&                 operator=(const IStopWatch&)    = delete;
    IStopWatch(const IStopWatch&)                               = delete;

protected:
    IStopWatch(){};
};
/******************************************************************************
 *  StopWatchCollection Interface.
 *****************************************************************************/
class IStopWatchCollection
    : public android::RefBase
{
public:

    virtual std::string                 GetName()                                           const   = 0;
    virtual android::sp<IStopWatch>     GetStopWatch(eStopWatchType type, const std::string name)   = 0;
    virtual void                        BeginStopWatch(const std::string name, const void* cookie)  = 0;
    virtual void                        EndStopWatch(const void* cookie)                            = 0;
    virtual void                        PrintReport()                                               = 0;

public:
    virtual ~IStopWatchCollection(){};

public:
    IStopWatchCollection& operator= (const IStopWatchCollection& other) = delete;
    IStopWatchCollection& operator= (IStopWatchCollection&& other)      = delete;
    IStopWatchCollection(const IStopWatchCollection& other)             = delete;
    IStopWatchCollection(IStopWatchCollection&& other)                  = delete;

protected:
    IStopWatchCollection(){};
};

class PipeRequest;
/**
 * @class abstract class IDualCamMFPipeCB
 * @brief This is the interface of bayer+mono de-noise feature pipe CB.
 */
class IDualCamMFPipeCB
    : public virtual android::RefBase
{

public:
    enum EventId{
        eCompleted,
        eFailed,
        eShutter,
        eP2Done,
        eFlush,
        ePostview,
        eYuvDone
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual ~IDualCamMFPipeCB() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual android::status_t                          onEvent(EventId eventType, sp<PipeRequest>& request, void* param1 = NULL, void* param2 = NULL) = 0;
};

/**
 * @class PipeRequest
 * @brief This is the enque data stricture of bayer+mono de-noise feature pipe
 */
class PipeRequest
    : public virtual android::RefBase
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  RefBase Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MVOID           onLastStrongRef(const void* /*id*/){mvImageBufferSet.clear();mvMetadataSet.clear();mvParams.clear();}
public:
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Public Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    typedef MVOID           (*PFN_CALLBACK_T)(MVOID* tag, String8 status, sp<PipeRequest>& request);//completed, failed
    PipeRequest(
        MUINT32             _reqNo = 0,
        PFN_CALLBACK_T      _cb = nullptr,
        MVOID*              _tag = nullptr
    )
        : mpTag(_tag)
        , mpOnRequestProcessed(_cb)
        , mReqNo(_reqNo)
        {}
    PipeRequest(
        MUINT32                 _reqNo,
        sp<IDualCamMFPipeCB>    _cb
    )
        : mwpCB(_cb)
        , mReqNo(_reqNo)
        {}
    virtual ~PipeRequest(){
        mvImageBufferSet.clear();
        mvMetadataSet.clear();
        mvParams.clear();
    }

    ssize_t                 addImageBuffer(const DualCamMFBufferID& key, const sp<IImageBuffer>& value){
                                sp<IImageBuffer> pBuf = value;
                                mvImageBufferSet[key] = pBuf;
                                return mvImageBufferSet.size();

                            }
    ssize_t                 addMetadata(const DualCamMFBufferID& key, IMetadata* const value){return mvMetadataSet.add(key, value);}
    ssize_t                 addParam(const DualCamMFParamID& key, const MINT32& value){return mvParams.add(key, value);}

    ssize_t                 removeImageBuffer(const DualCamMFBufferID& key){return mvImageBufferSet.erase(key);}
    ssize_t                 removeMetadata(const DualCamMFBufferID& key){return mvMetadataSet.removeItem(key);}
    ssize_t                 removeParam(const DualCamMFParamID& key){return mvParams.removeItem(key);}

    sp<IImageBuffer>        getImageBuffer(const DualCamMFBufferID& key){
                                    return mvImageBufferSet.count(key)>0? mvImageBufferSet[key] : nullptr;}
    std::map< DualCamMFBufferID, sp<IImageBuffer> >
                            getImageBufferSet(){return mvImageBufferSet;}
    IMetadata*              getMetadata(const DualCamMFBufferID& key){
                                    return mvMetadataSet.indexOfKey(key)>=0? mvMetadataSet.valueFor(key) : nullptr;}
    MINT32                  getParam(const DualCamMFParamID& key){
                                    return mvParams.indexOfKey(key)>=0? mvParams.valueFor(key) : -1;}
    MINT32                  getRequestNo() const {return mReqNo;};

    sp<IStopWatchCollection> getStopWatchCollection() {return mpStopWatchCollection;};
    void                     setStopWatchCollection(const sp<IStopWatchCollection> pStopWatchCollection) {mpStopWatchCollection =  pStopWatchCollection;};
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Public Data Members
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MVOID*                                              mpTag = nullptr;
    PFN_CALLBACK_T                                      mpOnRequestProcessed = nullptr;
    wp<IDualCamMFPipeCB>                                mwpCB = nullptr;
    sp<IStopWatchCollection>                            mpStopWatchCollection = nullptr;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Private Data Members
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32                                              mReqNo  = -1;

    std::map< DualCamMFBufferID, sp<IImageBuffer> >     mvImageBufferSet;
    KeyedVector<DualCamMFBufferID, IMetadata*>          mvMetadataSet;
    KeyedVector<DualCamMFParamID, MINT32>               mvParams;
};

/**
 * @class IDualCamMFPipe
 * @brief This is the interface of bayer+mono de-noise feature pipe
 */
class IDualCamMFPipe
    : public virtual android::RefBase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
// Copy constructor
// Create instance
static sp<IDualCamMFPipe> createInstance(MINT32 iSensorIdx_Main1, MINT32 iSensorIdx_Main2, DualCamMFType type);
// Destr instance
MBOOL destroyInstance();
// destructor
virtual ~IDualCamMFPipe() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief initalization
     * @param [in] szCallerame :  caller name
     * @return
     *-true indicates success, otherwise indicates fail
     */
    virtual MBOOL init() = 0;
    virtual MBOOL uninit() = 0;
    virtual MBOOL enque(android::sp<PipeRequest>& request) = 0;
    virtual MVOID flush() = 0;
    virtual MVOID sync() = 0;
    virtual MVOID setFlushOnStop(MBOOL flushOnStop) = 0;
    virtual DualCamMFType getPipeFeatureType() = 0;
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
* Namespace end.
********************************************************************************/
};
};
};
};

#endif
