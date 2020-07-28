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

#ifndef _MTKCAM_ADV_CAM_SETTING_MGR_IMP_H_
#define _MTKCAM_ADV_CAM_SETTING_MGR_IMP_H_

#include <mtkcam/feature/advCamSetting/AdvCamSettingMgr.h>
#include <mtkcam/feature/advCamSetting/CamCfgSetter.h> // currently called AdvCamService
#include <utils/Mutex.h>

namespace NSCam {

class IMetadata;

class AdvCamSettingMgr_Imp : public AdvCamSettingMgr, public CamCfgSetter
{
public:

// ====== CamCfgSetter Implemetation =================

    /**
    *@brief App can set advance config data before Hal3 configure() called.
    *@param[in]  configMeta : advanced config metadata set by app.
    *@return : True if success.
    */
    virtual bool setConfigureParam(const camera_metadata_t* configMeta);

// ====== AdvCamSettingMgr Implemetation =================

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
     * @param[in] userParam : How user could use this pipeline. for example: PIP, Dual-cam
     * @return : new AdvCamSetting, or null if config param not exist.
     */
    virtual android::sp<AdvCamSetting> generateAdvSetting(const AdvCamInputParam &inputParam, const UserParam &userParam);

    /**
     * This function should be called in Hal3 submitRequest() if needReconfigPipeline() return true.
     * When this function is called, it will generate AdvCamSetting from current App Metadata.
     * @param[in] appMetadata : it will use this metadata to generate new AdvCamSetting
     * @param[in] inputPara : some advcam setting need to reference pipeline param, such as isRecording or is4K for EIS.
     * @param[in] userParam : How user could use this pipeline. for example: PIP, Dual-cam
     * @return : new AdvCamSetting, or null if config param not exist.
     */
    virtual android::sp<AdvCamSetting> regenAdvSetting(IMetadata  *appMetadata, const AdvCamInputParam &inputParam, const UserParam &userParam);

    /**
     * This function should be called after generateAdvSetting().
     * You can use this function to know need to reconfig pipeline or not. Such as due to vhdr on/off switch.
     */
    virtual MBOOL needReconfigPipeline(IMetadata  *appMetadata , const PipelineParam &pipelineParam,
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
    virtual MBOOL updateRequestMeta( IMetadata  *halMetadata,
                                     IMetadata  *appMetadata,
                                     const PipelineParam    &pipelineParam,
                                     const RequestParam     &reqParam,
                                     const AdvCamInputParam &inputParam);

// ===================

public:

    AdvCamSettingMgr_Imp(const MUINT32 sensorIdx);

    virtual ~AdvCamSettingMgr_Imp();

private:

    struct LastFrameInfo
    {
        const MINT32 UNKNOWN = -1;
        MINT32   recordState   = UNKNOWN;
        MINT32   appMode       = UNKNOWN;
        MINT32   eisOn         = UNKNOWN; // -1 : unknown, 0 : off, 1 : on

        MVOID reset()
        {
            recordState = UNKNOWN;
            appMode = UNKNOWN;
            eisOn = UNKNOWN;
        }
    };

    class ConfigMetaHolder
    {
    public:
        /*Return true if extract sucess*/
        MBOOL extractConfigMeta(IMetadata &outMeta);
        /*Return true if setting config meta success */
        MBOOL setConfigMeta(const camera_metadata_t *inputAndroidMeta);
        /*Return true if previous meta exist*/
        MBOOL clear();
    private:
        IMetadata                   mConfigMeta;
        android::Mutex              mConfigLock;
        MBOOL                       mIsConfigMetaValid = MFALSE;

    };

    MUINT32                     mSensorIdx = 0;
    ConfigMetaHolder            mConfigHolder;

    MINT32                      mLogLevel = 0;
    MBOOL                       mEnableTSQ = MTRUE;
    MBOOL                       mDefaultAdvP2 = MFALSE;

    // State Members
    /*
    * Streaming state such as preview, videoPreview, videoRecord, ....
    * Only calculate it if app set metadta or it is REPEATING request
    * Initialize in constructor.
    */
    MINT32                      mAppMode;
    LastFrameInfo               mLastInfo;
    MBOOL                       mIsTkApp = MFALSE;

private:

    android::sp<AdvCamSetting> convertMetaToSetting(      MBOOL             isTkApp,
                                                          MBOOL             useAdvP2,
                                                    const IMetadata        &configMeta,
                                                    const AdvCamInputParam &inputPara,
                                                    const UserParam &userParam);


    MBOOL genDefaultConfig(const AdvCamInputParam  &inputParam,
                                 IMetadata         &outMeta);

    MBOOL updateIspProfile( HDRMode     hdrMode,
                            IMetadata  *halMetadata,
                            IMetadata  *appMetadata,
                      const PipelineParam &pipelineParam);

    MBOOL updateStreamingState(const IMetadata          *appMetadata,
                               const RequestParam       &reqParam,
                               const AdvCamInputParam   &inputParam,
                                     IMetadata          *halMetadata);

    MBOOL updateCropRegion( IMetadata  *halMetadata,
                            IMetadata  *appMetadata,
                      const PipelineParam &pipelineParam);
// ===== Customize =====
// Such as
//1. default running P2_Node or not
//2. custom operation mode convert to metadata

    MBOOL isCustomerApp(MUINT32 operationMode);

    MBOOL needAdvP2(MBOOL isTkApp, const AdvCamInputParam &inputParam);

    MBOOL customConfigMeta(const AdvCamInputParam  &inputParam,
                                 IMetadata         &outMeta);

// ===== 3DNR =====
    MUINT32 get3DNRMode(const IMetadata &configMeta, const AdvCamInputParam &inputPara, const UserParam &userParam);

// ===== HDR =====
    MUINT32 getSupportedVHDRMode(const MUINT32 sensorId);

    HDRMode getHDRMode(const IMetadata  *appMetadata);

    MUINT32 getVHDRMode(const HDRMode &hdrMode,
                        const android::sp<AdvCamSetting> &curSet);

    MBOOL needReconfigByHDR(const IMetadata *appMetadata,
                            const android::sp<AdvCamSetting> &curSet);
// ===== EIS =====
    MVOID calEisSettings(const IMetadata          *appMetadata,
                         const AdvCamInputParam   &inputPara,
                               MUINT32             vhdrMode,
                               MBOOL               isTkApp,
                               MBOOL               useAdvP2,
                               MUINT32            &outEisMode,
                               MUINT32            &outEisFactor,
                               MUINT32            &outEisExtraBufNum);
    MBOOL needReconfigByEIS(const IMetadata          *appMetadata,
                            const AdvCamInputParam   &inputPara,
                            const android::sp<AdvCamSetting> &curSet);
    MBOOL updateEISRequestMeta(const IMetadata          *appMetadata,
                               const PipelineParam      &pipelineParam,
                                     IMetadata          *halMetadata);
    MBOOL isEisEnabled(const android::sp<AdvCamSetting> &curSet);
    MBOOL isAdvEisEnabled(const android::sp<AdvCamSetting> &curSet);
    MBOOL isEisQEnabled(const android::sp<AdvCamSetting> &curSet);

};

}; // NSCam namespace
#endif // _MTKCAM_ADV_CAM_SETTING_MGR_IMP_H_

