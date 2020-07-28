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

/**
 * @file AdvCamSettingMgr.h
 *
 * Manage advanced camera setting from app.
 *
 */

#ifndef _MTKCAM_ADV_CAM_SETTING_MGR_H_
#define _MTKCAM_ADV_CAM_SETTING_MGR_H_

#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/hdrDetection/Defs.h>
#include <mtkcam/drv/IHalSensor.h>

namespace NSCam {

class IMetadata;

/* Store advanced camera setting, such as vhdr mode, eis mode...*/
// NOTE : This class should be simple enough so that Basic Package should not build fail due to this classs
class AdvCamSetting : public android::RefBase {

public:
    // HDR variable
    MUINT32 vhdrMode;

    // LMV variable
    MBOOL needLMV;
    // 3DNR variable, refer to 3dnr_defs.h E3DNR_MODE_MASK
    MUINT32 nr3dMode;

    // EIS variable
    MUINT32 eisMode;
    MUINT32 eisFactor;
    MUINT32 eisExtraBufNum; // Only valid if EIS using queue request solution rather than timestamp solution

    MBOOL useTSQ;

    // Streaming Feature Use
    MBOOL isTkApp; // True only if App has ever called AdvCamService to setConfigParam() before pipeline config
    MBOOL useAdvP2; // If true, use Adv P2Node to create pipeline.

    AdvCamSetting()
    : vhdrMode(SENSOR_VHDR_MODE_NONE)
    , needLMV(MFALSE)
    , nr3dMode(0)
    , eisMode(0)
    , eisFactor(100)
    , eisExtraBufNum(0)
    , useTSQ(MFALSE)
    , isTkApp(MFALSE)
    , useAdvP2(MFALSE)
    {}
};

/* Manage advanced camera setting */
class AdvCamSettingMgr
{
public:
    struct PipelineParam
    {
        PipelineParam()
            :mResizedRawSize(0,0)
            ,mSensorMode(0)
            ,mSensorSize(0,0)
            ,mVideoSize(0,0)
            ,mMaxStreamSize(0,0)
            ,currentAdvSetting(NULL)
            {
            }

        MSize   mResizedRawSize;
        MUINT32 mSensorMode;
        MSize   mSensorSize;
        MSize   mVideoSize;
        MSize   mMaxStreamSize;
        android::sp<AdvCamSetting> currentAdvSetting;
    };

    struct AdvCamInputParam
    {
        MBOOL isRecordPipe;
        MSize videoSize;
        MBOOL is4K2K;
        android::sp<AdvCamSetting> lastAdvCamSetting;
        MUINT32 operationMode;

        AdvCamInputParam()
            : isRecordPipe(MFALSE)
            , videoSize()
            , is4K2K(MFALSE)
            , lastAdvCamSetting(NULL)
            , operationMode(0)
        {}

        AdvCamInputParam(MBOOL _isRec, MSize _vSize, MBOOL _is4K
                ,android::sp<AdvCamSetting> _lastAdvCamSetting, MUINT32 _operationMode)
            : isRecordPipe(_isRec)
            , videoSize(_vSize)
            , is4K2K(_is4K)
            , lastAdvCamSetting(_lastAdvCamSetting)
            , operationMode(_operationMode)
        {}
    };

    struct RequestParam
    {
        MBOOL   mHasEncodeBuf   = MFALSE;
        MBOOL   mIsRepeatingReq = MFALSE;
    };

    enum EUSER_PARAM_MASK
    {
        EUSER_PARAM_MASK_NONE                   = 0,
        EUSER_PARAM_MASK_FIRST_DEV              = 1 << 0,
        EUSER_PARAM_MASK_MULTI_CAM              = 1 << 1,
    };

    struct UserParam
    {
        MUINT32 mUserParamMask    = EUSER_PARAM_MASK_NONE;
    };

    static AdvCamSettingMgr* getInstance(const MUINT32 sensorIdx);

    /**
     * Initial Function.
     */
    virtual MBOOL init();

    /**
     * Notify device ready to close. Confirm to  clear app's config meta in this function
     */
    virtual MBOOL notifyDeviceClose();

    /**
     * Notify device received 1st request after pipeline configured. Confirm to clear app's config meta in this function
     */
    virtual MBOOL notifyFirstReq();

    /**
     * This function should be called in Hal3 configure().
     * When this function is called, it will generate AdvCamSetting from Metadata which app previously set,
     * and then clear the Metadata to avoid load repeatly.
     * @param[in] inputPara : some advcam setting need to reference pipeline param, such as isRecording or is4K for EIS.
     * @param[in] inputPara : some advcam setting need to reference pipeline param, such as isRecording or is4K for EIS.
     * @return : new AdvCamSetting, or null if config param not exist.
     */
    virtual android::sp<AdvCamSetting> generateAdvSetting(const AdvCamInputParam &inputParam, const UserParam &userParam);

    /**
     * This function should be called in Hal3 submitRequest() if needReconfigPipeline() return true.
     * When this function is called, it will generate AdvCamSetting from current App Metadata.
     * @param[in] appMetadata : it will use this metadata to generate new AdvCamSetting
     * @param[in] inputPara : some advcam setting need to reference pipeline param, such as isRecording or is4K for EIS.
     * @return : new AdvCamSetting, or null if config param not exist.
     */
    virtual android::sp<AdvCamSetting> regenAdvSetting(IMetadata*  appMetadata, const AdvCamInputParam &inputParam, const UserParam &userParam);

    /**
     * This function should be called after generateAdvSetting().
     * You can use this function to know need to reconfig pipeline or not. Such as due to vhdr on/off switch.
     */
    virtual MBOOL needReconfigPipeline(IMetadata*  appMetadata , const PipelineParam &pipelineParam,
                                       const AdvCamInputParam &inputParam);

    /**
     * Before new request submitted to pipeline, it may need to modify metadata for feature.
     * Such as Isp Profile.
     *@param[in/out] halMetadata : it will update some value in hal metadata
     *@param[in/out] appMetadata : it may update some value in app metadata for Hal1. In Hal3, this maybe NULL.
     *@param[in] pipelineParam : it's some pipeline param to decide metadata setting
     *@param[in] reqParam : per-frame request related parameter, such as has record buf in this req or not.
     *@param[in] inpuParam : Additional pipeline params for generate AdvCamSetting
     */
    virtual MBOOL updateRequestMeta( IMetadata*  halMetadata,
                                     IMetadata*  appMetadata,
                                     const PipelineParam    &pipelineParam,
                                     const RequestParam     &reqParam,
                                     const AdvCamInputParam &inputParam);

protected:

    /**
     *@brief AdvCamSettingMgr constructor
     */
    AdvCamSettingMgr() {};

    /**
     *@brief AdvCamSettingMgr destructor
     */
    virtual ~AdvCamSettingMgr() {};

};

}; // NSCam namespace
#endif // _MTKCAM_ADV_CAM_SETTING_MGR_H_

