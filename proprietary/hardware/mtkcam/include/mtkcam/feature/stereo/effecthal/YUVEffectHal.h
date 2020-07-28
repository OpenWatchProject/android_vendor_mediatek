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
#ifndef _MTK_HARDWARE_MTKCAM_FEATURE_YUV_EFFECTHAL_H_
#define _MTK_HARDWARE_MTKCAM_FEATURE_YUV_EFFECTHAL_H_


#include <string>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>

#define SENSOR_IDX_MAIN1 "sensorIdx_main1"
#define SENSOR_IDX_MAIN2 "sensorIdx_main2"
#define DUAL_YUV_REQUEST_STATE_KEY "ReqStatus"
#define DUALYUV_DONE_KEY "Done"
#define DUALYUV_FAILED_KEY "Failed"
#define DUAL_YUV_REQUEST_IN_APP_META "InAppMeta"
#define DUAL_YUV_REQUEST_IN_HAL_META "InHalMeta"
#define EFFECT_PARAMS_KEY "Metadata"

//
//using namespace android;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;
using namespace NS3Av3;

namespace NSCam{

//class EffectRequest;
typedef android::sp<EffectRequest> EffectRequestPtr;
typedef android::sp<EffectFrameInfo> FrameInfoPtr;

class YUVEffectHal : public EffectHalBase
{
    public:
        typedef enum eDualYUVNodeOpState {
            STATE_NORMAL = 1,
            STATE_CAPTURE = 2,
            STATE_THUMBNAIL = 3
        } DualYUVNodeOpState;

        typedef enum DualYUV_BufferDataTypes{
            BID_DUALYUV_INVALID = 0,
            BID_REQUEST_STATE,

            BID_DualYUV_IN_FSRAW1,
            BID_DualYUV_IN_FSRAW2,
            BID_DualYUV_IN_RSRAW1,
            BID_DualYUV_IN_RSRAW2,

            BID_OUT_MV_F_CAP_MAIN1,
            BID_OUT_MV_F_CAP_MAIN2,
            BID_OUT_FD,
            BID_OUT_MV_F,
            BID_OUT_THUMBNAIL_CAP,

            // metadata
            BID_META_IN_APP,
            BID_META_IN_HAL,
            BID_META_IN_HAL_MAIN2,
            //BID_META_OUT_APP,
            //BID_META_OUT_HAL,
        } DualYUVBufferID;

        enum eCropGroup
        {
            eCROP_CRZ=1,
            eCROP_WDMA=2,
            eCROP_WROT=3
        };

        enum tuning_tag
        {
            tuning_tag_G2G = 0,
            tuning_tag_G2C,
            tuning_tag_GGM,
            tuning_tag_UDM,
        };
    public:
        YUVEffectHal();
        virtual ~YUVEffectHal();
        MBOOL onFlush();
    protected:
        virtual status_t   initImpl();
        virtual status_t   uninitImpl();
        //non-blocking
        virtual status_t   prepareImpl();
        virtual status_t   releaseImpl();
        virtual status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const;
        virtual status_t   startImpl(uint64_t *uid=NULL);
        virtual status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL);
        //non-blocking
        virtual status_t   updateEffectRequestImpl(const EffectRequestPtr request);
        // Currently, these funcs will not used.
        virtual bool       allParameterConfigured();
        virtual status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
        virtual status_t   setParameterImpl(android::String8 &key, android::String8 &object);
        virtual status_t   setParametersImpl(android::sp<EffectParameter> parameter);
    private:
        struct ISPTuningConfig
        {
            FrameInfoPtr& pInAppMetaFrame;
            FrameInfoPtr& pInHalMetaFrame;
            IHal3A* p3AHAL;
            MBOOL bInputResizeRaw;
        };
        struct EnquedData
        {
        public:
            EffectRequestPtr mRequest;
            YUVEffectHal* mpYUVEffectHal;

            EnquedData(EffectRequestPtr req, YUVEffectHal* pYUVEffectHal)
            : mRequest(req), mpYUVEffectHal(pYUVEffectHal)
            {
            }

        };
        struct eis_region
        {
            MUINT32 x_int;
            MUINT32 x_float;
            MUINT32 y_int;
            MUINT32 y_float;
            MSize   s;
        #if SUPPORT_EIS_MV
            MUINT32 x_mv_int;
            MUINT32 x_mv_float;
            MUINT32 y_mv_int;
            MUINT32 y_mv_float;
            MUINT32 is_from_zzr;
        #endif
        };
        MBOOL              buildQParams_NORMAL(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam);
        MBOOL              buildQParams_CAP(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam);
        MBOOL              buildQParams_THUMBNAIL(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam);
        // isp tuning operations
        TuningParam        applyISPTuning(MVOID* pVATuningBuffer, const ISPTuningConfig& ispConfig);
        MVOID              cleanUp();
        MBOOL              calCropForScreen(FrameInfoPtr& pFrameInfo, MPoint &rCropStartPt, MSize& rCropSize );
        IMetadata*         getMetadataFromFrameInfoPtr(sp<EffectFrameInfo> pFrameInfo);
        MBOOL              isEISOn(IMetadata* const inApp);
        MBOOL              queryEisRegion(IMetadata* const inHal, eis_region& region, EffectRequestPtr request);
        MVOID              debugQParams(const QParams& rInputQParam);
        MVOID              ReleaseQParam(QParams& rParams);
        MVOID              ReleaseMain2HalMeta(const EffectRequestPtr request);
        MINT32             DegreeToeTransform(MINT32 degree);
        MINT32             getJpegRotation(IMetadata* pMeta);

        // P2 callbacks
        static MVOID onP2Callback(QParams& rParams);
        static MVOID onP2FailedCallback(QParams& rParams);
        MVOID handleP2Done(QParams& rParams, EnquedData* pEnqueData);
        MVOID handleP2Failed(QParams& rParams, EnquedData* pEnqueData);



    private:
        mutable Mutex      mLock;
        MINT32             miDump = MFALSE;
        MINT32             mbDebugLog = MFALSE;
        std::string        msFilename = "";
        MINT32             mLogLevel;
        MINT32             miOff3AInfo;
        MINT32             miSensorIdx_Main1;
        MINT32             miSensorIdx_Main2;
        // P2 NormalStream
        INormalStream*     mpINormalStream;
        // 3A hal
        IHal3A*            mp3AHal_Main1;
        IHal3A*            mp3AHal_Main2;
};
};
#endif //_MTK_HARDWARE_MTKCAM_FEATURE_YUV_EFFECTHAL_H_