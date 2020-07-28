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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONNECT_LMV_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONNECT_LMV_H_

#include "P1Common.h"
//
#include <mtkcam/feature/eis/lmv_hal.h>
#include <mtkcam/feature/eis/gis_calibration.h>
#include <camera_custom_eis.h>
//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/

class P1ConnectLMV
    : public virtual android::RefBase
{
    public:
                        P1ConnectLMV(
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI,
                            MINT32 nSysLevel
                        )
                        : mLock()
                        , mOpenId(nOpenId)
                        , mLogLevel(nLogLevel)
                        , mLogLevelI(nLogLevelI)
                        , mSysLevel(nSysLevel)
                        , mpLMV(NULL)
                        , mpGISCalibration(NULL)
                        , mEisMode(0)
                        , mIsConfig3A(MFALSE)
                        , mIsCalibration(MFALSE)
                        , mConfigData()
                        {
                            //MY_LOGD0("+++");
                            //MY_LOGD0("---");
                        };

        virtual         ~P1ConnectLMV()
                        {
                            //MY_LOGD0("+++");
                            //MY_LOGD0("---");
                        };

        MBOOL           support(void);

        MINT32          getOpenId(void);

        MBOOL           init(
                            sp<IImageBuffer> & rEISOBuf,
                            MUINT32 eisMode,
                            const MUINT32 eisFactor
                        );

        MBOOL           uninit(void);

        MVOID           config(void);

        MVOID           enableSensor(void);

        MVOID           enableOIS(IHal3A_T * p3A);

        MVOID           getBuf(
                            sp<IImageBuffer> & rEISOBuf
                        );

        MBOOL           isEISOn(
                            IMetadata* const inApp
                        );

        MBOOL           is3DNROn(
                            IMetadata* const inApp,
                            IMetadata* const inHal
                        );

        MBOOL           checkSwitchOut(
                            IMetadata* const inHAL
                        );

        MVOID           adjustCropInfo(
                            IMetadata* pAppMetadata,
                            IMetadata* pHalMetadata,
                            MRect& cropRect_control,
                            MSize sensorParam_Size,
                            MBOOL bEnableFrameSync,
                            MBOOL bIsStereoCamMode
                        );

        MVOID           processDequeFrame(
                            NSCam::NSIoPipe::NSCamIOPipe::QBufInfo&  pBufInfo
                        );

        MVOID           processDropFrame(
                            android::sp<NSCam::IImageBuffer>& spBuf
                        );

        MVOID           processResult(
                            MBOOL isBinEn,
                            MBOOL isConfigEis,
                            MBOOL isConfigRrz,
                            IMetadata* pInAPP,//inAPP
                            IMetadata* pInHAL,//inHAL
                            MetaSet_T & result3A,
                            IHal3A_T* p3A,
                            MINT32 const currMagicNum,
                            MUINT32 const currSofIdx,
                            MUINT32 const lastSofIdx,
                            MUINT32 const uniSwitchState,//UNI_SWITCH_STATE
                            QBufInfo const &deqBuf,
                            MUINT32 const bufIdxEis,
                            MUINT32 const bufIdxRrz,
                            IMetadata & rOutputLMV
                        );

    private:
        MVOID           processLMV(
                            MBOOL isBinEn,
                            IHal3A_T* p3A,
                            MINT32 const currMagicNum,
                            MUINT32 const currSofIdx,
                            MUINT32 const lastSofIdx,
                            QBufInfo const &deqBuf,
                            MUINT32 const deqBufIdx,
                            MUINT8 captureIntent,
                            MINT64 exposureTime,
                            IMetadata & rOutputLMV
                        );

    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MINT32          mSysLevel;
        LMVHal*         mpLMV;
        GisCalibration* mpGISCalibration;
        MUINT32         mEisMode;
        MBOOL           mIsConfig3A;
        MBOOL           mIsCalibration;
        LMV_HAL_CONFIG_DATA
                        mConfigData;


};

#ifdef IS_LMV
#undef IS_LMV
#endif
#define IS_LMV(ConnectLMV_Ptr)\
    ((ConnectLMV_Ptr != NULL) && (ConnectLMV_Ptr->support()))


};//namespace NSP1Node
};//namespace v3
};//namespace NSCam


#endif//_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONNECT_LMV_H_

