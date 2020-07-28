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
 * @file IBMDeNoisePipe.h
 * @brief This is the interface of bayer+mono de-noise feature pipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_IBMDENOISE_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_IBMDENOISE_PIPE_H_

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

#define BMDENOISE_COMPLETE_KEY "onComplete"
#define BMDENOISE_FLUSH_KEY "onFlush"
#define BMDENOISE_ERROR_KEY "onError"
#define BMDENOISE_COMPOSITION_KEY "Composition"
#define BMDENOISE_DENOISE_TYPE_KEY "DeNoiseType"

using namespace android;

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
/*******************************************************************************
* Enum Define
********************************************************************************/
enum BMDeNoiseFeatureType
{
    TYPE_BMDN,
    TYPE_MFHR,
    TYPE_BMDN_3rdParty,
    TYPE_MFHR_3rdParty
};

enum BMDeNoiseBufferID{
    BID_BMDENOISEPIPE_INVALID = 0,
    //0
    BID_REQUEST_STATE,
    BID_PRE_PROCESS_OUT_YUV,
    BID_PRE_PROCESS_OUT_MFBO,
    BID_PRE_PROCESS_OUT_W_1,
    BID_PRE_PROCESS_OUT_W_2,
    //5
    BID_PRE_PROCESS_OUT_MFBO_FINAL_1,
    BID_PRE_PROCESS_OUT_MFBO_FINAL_2,
    BID_DENOISE_IN_MFBO_FINAL_1,
    BID_DENOISE_IN_MFBO_FINAL_2,
    BID_DENOISE_IN_W_1,
    //10
    BID_DENOISE_IN_SCENE_INFO,
    BID_DENOISE_IN_DISPARITY_MAP_1,
    BID_DENOISE_IN_DISPARITY_MAP_2,
    BID_DENOISE_IN_WARPING_MATRIX,
    BID_DENOISE_HAL_OUT,
    //15
    BID_DENOISE_HAL_OUT_ROT_BACK,
    BID_DENOISE_FINAL_RESULT,
    BID_DENOISE_AND_SWNR_OUT,
    BID_DENOISE_AND_MNR_OUT,
    BID_DENOISE_3RD_PARTY_OUT,
    //20
    BID_PRE_PROCESS_OUT_1,
    BID_PRE_PROCESS_OUT_2,
    BID_PRE_PROCESS_OUT_RRZO,
    BID_INPUT_FSRAW_1,
    BID_INPUT_FSRAW_2,
    //25
    BID_INPUT_RSRAW_1,
    BID_INPUT_RSRAW_2,
    BID_P2AFM_FE1B_INPUT,
    BID_P2AFM_FE2B_INPUT,
    BID_P2AFM_FE1C_INPUT,
    //30
    BID_P2AFM_FE2C_INPUT,
    BID_P2AFM_OUT_FDIMG,
    BID_P2AFM_OUT_FE1BO,
    BID_P2AFM_OUT_FE2BO,
    BID_P2AFM_OUT_FE1CO,
    //35
    BID_P2AFM_OUT_FE2CO,
    BID_P2AFM_OUT_RECT_IN1,
    BID_P2AFM_OUT_RECT_IN2,
    BID_P2AFM_OUT_RECT_IN1_CAP,
    BID_P2AFM_OUT_RECT_IN2_CAP,
    //40
    BID_P2AFM_OUT_CC_IN1,
    BID_P2AFM_OUT_CC_IN2,
    BID_P2AFM_OUT_FMBO_LR,
    BID_P2AFM_OUT_FMBO_RL,
    BID_P2AFM_OUT_FMCO_LR,
    //45
    BID_P2AFM_OUT_FMCO_RL,
    BID_P2AFM_OUT_MV_F,
    BID_P2AFM_OUT_MV_F_CAP,
    BID_N3D_OUT_MASK_M,
    BID_N3D_OUT_PRE_MASK_S,
    //50
    BID_N3D_OUT_WARP_GRID_1_X,
    BID_N3D_OUT_WARP_GRID_1_Y,
    BID_N3D_OUT_WARP_GRID_1_Z,
    BID_N3D_OUT_WARP_GRID_2_X,
    BID_N3D_OUT_WARP_GRID_2_Y,
    //55
    BID_N3D_OUT_WARP_GRID_2_Z,
    BID_N3D_OUT_SV_CC_Y,
    BID_N3D_OUT_WARPING_MATRIX,
    BID_N3D_OUT_MV_Y,
    BID_N3D_OUT_SV_Y,
    //60
    BID_N3D_OUT_MASK_S,
    BID_N3D_OUT_SCENEINFO,
    BID_DPE_OUT_CFM_L,
    BID_DPE_OUT_CFM_R,
    BID_DPE_OUT_DMP_L,
    //65
    BID_DPE_OUT_DMP_R,
    BID_DPE_OUT_RESPO_L,
    BID_DPE_OUT_RESPO_R,
    BID_DPE_OUT_LAST_DMP_L,
    BID_DPE_OUT_LAST_DMP_R,
    //70
    BID_WPE_OUT_MV_Y,
    BID_WPE_OUT_SV_Y,
    BID_WPE_OUT_MASK_S,
    BID_JPEG_YUV,
    BID_THUMB_YUV,
    //75
    BID_META_IN_APP,
    BID_META_IN_HAL,
    BID_META_IN_HAL_MAIN2,
    BID_META_OUT_APP,
    BID_META_OUT_HAL,
    //80
    BID_IMG3O,
    BID_SWNR_IN_YV12,
    BID_SWNR_OUT_YV12,
    BID_TEMP_BUFFER,
    BID_UV_B,
    //85
    BID_UV_M,
    BID_LCS_1,
    BID_LCS_2,
    BID_POST_PROCESS_TEMP_YV12,
    BID_POST_PROCESS_TEMP_I422,
    //90
    BID_POSTVIEW,
    BID_PRE_PROCESS_OUT_W_1_IMG3O,
    BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O,
    BID_PRE_PROCESS_OUT_YUV_IMG3O,
    BID_BAYER_PREPROCESS_YUV,
    //95
    BID_MONO_PREPROCESS_YUV,
    BID_MONO_PREPROCESS_SMALL_YUV,
    BID_WARP_OUT,
    BID_FUSION_OUT,
    BID_MFNR_OUT,
    //100
    BID_MFNR_OUT_FINAL,
    BID_LCSO_1_BUF,
    BID_LCSO_2_BUF,
    BID_LCSO_ROT_BUF
} ;

enum BMDeNoiseParamID{
    PID_MFHR_REQ_ID,
    PID_MFHR_IS_MAIN,
    PID_EXIF_UNIQUE_ID,
    PID_SYNC_FAILED,
    PID_MFHR_IS_ON,
    PID_LCSO_CONTENT_WIDTH,
    PID_LCSO_CONTENT_HEIGHT,
};

enum BMDeNoisePipeRequestComp{
    COMP_NONE = 0,
    COMP_BM_DENOISE,
    COMP_BM_DENOISE_AND_MNR,
    COMP_BM_DENOISE_AND_SWNR
};

enum BMDeNoisePipeDeNoiseType{
    DN_TYPE_NONE = 0,
    DN_TYPE_NORMAL,
    DN_TYPE_QUICK,
    DN_TYPE_UNKNOWN
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
    virtual android::sp<IStopWatch>     GetStopWatch(MINT32 type, const std::string name)           = 0;
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
 * @class abstract class IBMDeNoisePipeCB
 * @brief This is the interface of bayer+mono de-noise feature pipe CB.
 */
class IBMDeNoisePipeCB
    : public virtual android::RefBase
{

public:
    enum EventId{
        eCompleted,
        eFailed,
        eShutter,
        eP2Done,
        eFlush,
        ePostview
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual ~IBMDeNoisePipeCB() {};
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
        sp<IBMDeNoisePipeCB>    _cb
    )
        : mwpCB(_cb)
        , mReqNo(_reqNo)
        {}
    virtual ~PipeRequest(){
        mvImageBufferSet.clear();
        mvMetadataSet.clear();
        mvParams.clear();
    }

    ssize_t                 addImageBuffer(const BMDeNoiseBufferID& key, const sp<IImageBuffer>& value){
                                sp<IImageBuffer> pBuf = value;
                                mvImageBufferSet[key] = pBuf;
                                return mvImageBufferSet.size();

                            }
    ssize_t                 addMetadata(const MINT32& key, IMetadata* const value){return mvMetadataSet.add(key, value);}
    ssize_t                 addParam(const MINT32& key, const MINT32& value){return mvParams.add(key, value);}

    ssize_t                 removeImageBuffer(const BMDeNoiseBufferID& key){return mvImageBufferSet.erase(key);}
    ssize_t                 removeMetadata(const MINT32& key){return mvMetadataSet.removeItem(key);}
    ssize_t                 removeParam(const MINT32& key){return mvParams.removeItem(key);}

    sp<IImageBuffer>        getImageBuffer(const BMDeNoiseBufferID& key){
                                    return mvImageBufferSet.count(key)>0? mvImageBufferSet[key] : nullptr;}
    IMetadata*              getMetadata(const MINT32& key){
                                    return mvMetadataSet.indexOfKey(key)>=0? mvMetadataSet.valueFor(key) : nullptr;}
    MINT32                  getParam(const MINT32& key){
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
    wp<IBMDeNoisePipeCB>                                mwpCB = nullptr;
    sp<IStopWatchCollection>                            mpStopWatchCollection = nullptr;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Private Data Members
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32                                              mReqNo  = -1;

    std::map< BMDeNoiseBufferID, sp<IImageBuffer> >     mvImageBufferSet;
    KeyedVector<MINT32, IMetadata*>                     mvMetadataSet;
    KeyedVector<MINT32, MINT32>                         mvParams;
};

/**
 * @class IBMDeNoisePipe
 * @brief This is the interface of bayer+mono de-noise feature pipe
 */
class IBMDeNoisePipe
    : public virtual android::RefBase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
// Copy constructor
// Create instance
static sp<IBMDeNoisePipe> createInstance(MINT32 iSensorIdx_Main1, MINT32 iSensorIdx_Main2, MUINT32 type);
// Destr instance
MBOOL destroyInstance();
// destructor
virtual ~IBMDeNoisePipe() {};

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
    virtual BMDeNoiseFeatureType getPipeFeatureType() = 0;
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


#endif
