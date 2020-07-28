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
 * @file awb_mgr_if.h
 * @brief AWB manager interface
 */

#ifndef _AWB_MGR_IF_H_
#define _AWB_MGR_IF_H_

#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <flash_awb_param.h>
#include <private/aaa_hal_private.h>
#include <dbg_aaa_param.h>
#include <dbg_awb_param.h>
#include <isp_config/awb_config.h>
#include <tuning/awb_flow_custom.h>



namespace NS3Av3
{
class IAWBInitPara
{
  int aaoW;
  int aaoH;
};
/**
 * @brief AWB manager interface
 */
class IAwbMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IAwbMgr(IAwbMgr const&);
    //  Copy-assignment operator is disallowed.
    IAwbMgr& operator=(IAwbMgr const&);

public:  ////
    IAwbMgr() {}
    ~IAwbMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief get instance of IAwbMgr
     */
    static IAwbMgr& getInstance();

    MBOOL start(MINT32 i4SensorDev);
    MBOOL stop(MINT32 i4SensorDev);

    MBOOL init(MINT32 i4SensorDev, MINT32 i4SensorIdx, IAWBInitPara* initPara=0);
    /**
     * @brief uninit
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MBOOL uninit(MINT32 i4SensorDev);

    /**
     * @brief check if AWB lock is supported or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return always return TRUE
     */
    inline MBOOL isAWBLockSupported()
    {
        return MTRUE;
    }

    /**
     * @brief check if AWB is enabled or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return TRUE if AWB is enabled
     */
    MBOOL isAWBEnable(MINT32 i4SensorDev);

    /**
     * @brief set AWB mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewAWBMode AWB mode; please refer to awb_feature.h
     */
    MBOOL setAWBMode(MINT32 i4SensorDev, MINT32 i4NewAWBMode);

    /**
     * @brief set sensor mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewSensorMode sensor mode; please refer to isp_tuning.h
     * @param [in] i4BinWidth: BIN width
     * @param [in] i4BinHeight: BIN height
     * @param [in] i4QbinWidth: QBIN width
     * @param [in] i4QbinHeight: QBIN height
     */
    MBOOL setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MINT32 i4BinWidth, MINT32 i4BinHeight, MINT32 i4QbinWidth, MINT32 i4QbinHeight);

    /**
     * @brief get AWB mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return current AWB mode
     */
    MINT32 getAWBMode(MINT32 i4SensorDev);

    /**
     * @brief set strobe mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewStrobeMode strobe mode; please refer to AWB_STROBE_MODE_T in awb_param.h
     */
    MBOOL setStrobeMode(MINT32 i4SensorDev, MINT32 i4NewStrobeMode);

    /**
     * @brief get strobe mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return current strobe mode
     */
    MINT32 getStrobeMode(MINT32 i4SensorDev);

    /**
     * @brief set flash AWB data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rFlashAwbData flash AWB data
     */
    MBOOL setFlashAWBData(MINT32 i4SensorDev, FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData);

    /**
     * @brief set AWB lock
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAWBLock TRUE: lock, FALSE: unlock
     */
    MBOOL setAWBLock(MINT32 i4SensorDev, MBOOL bAWBLock);

    /**
     * @brief enable AWB
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MBOOL enableAWB(MINT32 i4SensorDev);

    /**
     * @brief disable AWB
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MBOOL disableAWB(MINT32 i4SensorDev);

    /**
     * @brief set AWB Statistics crop ratio
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4SensorMode: sensor mode
     * @param [in] i4CropOffsetX: crop offset X
     * @param [in] i4CropOffsetY: crop offset Y
     * @param [in] i4CropRegionWidth: crop region width
     * @param [in] i4CropRegionHeight: crop region height
     */
    MBOOL setAWBStatCropRegion(MINT32 i4SensorDev,
                               MINT32 i4SensorMode,
                               MINT32 i4CropOffsetX,
                               MINT32 i4CropOffsetY,
                               MINT32 i4CropRegionWidth,
                               MINT32 i4CropRegionHeight
                              );

    /**
     * @brief preview AWB main function
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4FrameCount current frame count
     * @param [in] bAEStable TRUE: AE is stable, FALSE: AE is unstable
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     * @param [in] bApplyToHW TURE: upadte AWB gain to HW, FALSE: do not apply AWB gain to HW
     */
    MBOOL doPvAWB(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MUINT32 u4ExposureTime, MBOOL bApplyToHW = MTRUE);

    MBOOL setMaxFPS(MINT32 i4SensorDev, MINT32 i4MaxFPS);

     /**
     * @brief touch focus AWB main function
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     * @param [in] bApplyToHW TURE: upadte AWB gain to HW, FALSE: do not apply AWB gain to HW
     */
    MBOOL doAFAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW = MTRUE);

    /**
     * @brief pre-capture AWB main function
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     * @param [in] bApplyToHW TURE: upadte AWB gain to HW, FALSE: do not apply AWB gain to HW
     */
    MBOOL doPreCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW = MTRUE);

    /**
     * @brief capture AWB main function
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     * @param [in] bApplyToHW TURE: upadte AWB gain to HW, FALSE: do not apply AWB gain to HW
     */
    MBOOL doCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW = MTRUE);

    /**
     * @brief capture flash AWB display function
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bApplyToHW TURE: upadte AWB gain to HW, FALSE: do not apply AWB gain to HW
     */
    MBOOL doCapFlashAWBDisplay(MINT32 i4SensorDev, MBOOL bApplyToHW = MTRUE);

    /**
     * @brief save N3D AWB data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] AWB_SYNC_OUTPUT_N3D_T N3D AWB sync output data
     */
    MBOOL applyAWB(MINT32 i4SensorDev, AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput);

    /**
     * @brief get EXIF debug info
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAWBDebugInfo AWB debug info; please refer to dbg_awb_param.h
     * @param [out] rAWBDebugData AWB debug data; please refer to dbg_awb_param.h
     */
    MBOOL getDebugInfo(MINT32 i4SensorDev, AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData);

    /**
     * @brief get correlated color temperature
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return correlated color temperature
     */
    MINT32 getAWBCCT(MINT32 i4SensorDev);

    /**
     * @brief get ASD info
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rAWBASDInfo ASD info; please refer to awb_param.h
     */
    MBOOL getASDInfo(MINT32 i4SensorDev, AWB_ASD_INFO_T &a_rAWBASDInfo);

    /**
     * @brief get AWB output
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rAWBOutput AWB algorithm output; please refer to awb_param.h
     */
    MBOOL getAWBOutput(MINT32 i4SensorDev, AWB_OUTPUT_T &a_rAWBOutput);

    /**
     * @brief get AWB gain
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAwbGain AWB gain; please refer to awb_param.h
     */
    MBOOL getAWBGain(MINT32 i4SensorDev, AWB_GAIN_T &rAwbGain, MINT32& i4ScaleUnit) const;
	MRESULT getAWBParentStat(MINT32 i4SensorDev, AWB_PARENT_BLK_STAT_T *a_rAWBParentState, MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY);

    /**
     * @brief set AF LV
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] i4AFLV scene LV for touch AF
     */
    MVOID setAFLV(MINT32 i4SensorDev, MINT32 i4AFLV);

    /**
     * @brief get AF LV
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return scene LV for touch AF
     */
    MINT32 getAFLV(MINT32 i4SensorDev);


    /**
     * @brief get N3D init parameter
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] CCTInitParam CCT init parameter
     * @param [in] NoramlGainInitParam normal AWB gain init parameter
     */
    //MBOOL getInitInputParam(MINT32 i4SensorDev, SYNC_CTC_INIT_PARAM &CCTInitParam, NORMAL_GAIN_INIT_INPUT_PARAM &NoramlGainInitParam);

    /**
     * @brief set TG info
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4TGInfo TG info
     */
    MBOOL setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo);

    /**
     * @brief set AWB NVRAM index
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4AWBNvramIdx AWB NVRAM index
     */
    MBOOL setAWBNvramIdx(MINT32 const i4SensorDev, MINT32 const i4AWBNvramIdx);

    /**
     * @brief set Strobe NVRAM index
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4StrobeNvramIdx Strobe NVRAM index
     */
    MBOOL setStrobeNvramIdx(MINT32 const i4SensorDev, MINT32 const i4StrobeNvramIdx);

    /**
     * @brief set FlashCali NVRAM index
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4FlashCaliNvramIdx FlashCali NVRAM index
     */
    MBOOL setFlashCaliNvramIdx(MINT32 const i4SensorDev, MINT32 const i4FlashCaliNvramIdx);

    /**
     * @brief set CamScenarioMode control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4CamScenarioMode camera scenario mode; please refer to CAM_SCENARIO_T in camera_custom_nvram.h
     */
    MRESULT setCamScenarioMode(MINT32 i4SensorDev, MUINT32 u4CamScenarioMode);

    /**
     * @brief set CamScenarioMode control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_eNVRAMIndex camera NVRAM index;
     */
	MRESULT setNVRAMIndex(MINT32 i4SensorDev, MUINT32 a_eNVRAMIndex);

    MBOOL setIsMono(MINT32 const i4SensorDev, int bMono, MUINT32 i4SensorType);

    /**
     * @brief set AWB face detection enable
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bFDenable face detection enable flag
     */
    MRESULT setFDenable(MINT32 i4SensorDev, MBOOL bFDenable);

    /**
     * @brief set AWB face detection area and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     * @param [in] i4tgwidth QBIN image width
     * @param [in] i4tgheight QBIN image height
     */
    MRESULT setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight);

    /**
     * @brief set Zoom window information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4XOffset horizontal offset value
     * @param [in] u4YOffset vertical offset value
     * @param [in] u4Width horizontal width value
     * @param [in] u4Height vertical height value
     */
    MRESULT setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);

    /**
     * @brief get focus area
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] vecOut including type, number of ROI, left of ROI1, top of ROI1, right of ROI1, bottom of ROI1, Result of ROI1, left of ROI2, top of ROI2, right of ROI2, bottom of ROI2, Result of ROI2, etc.
     * @param [in] i4tgwidth QBIN image width
     * @param [in] i4tgheight QBIN image height
     */
    MRESULT getFocusArea(MINT32 i4SensorDev, std::vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight);

    //__________________________Camera HAL3.2__________________________

  MBOOL backup(MINT32 const i4SensorDev);
    MBOOL restore(MINT32 const i4SensorDev);

    MBOOL getColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT& fGain_R, MFLOAT& fGain_G, MFLOAT& fGain_B);
    MBOOL setColorCorrectionGain(MINT32 const i4SensorDev, MFLOAT fGain_R, MFLOAT fGain_G, MFLOAT fGain_B);
    MBOOL getAWBState(MINT32 const i4SensorDev, mtk_camera_metadata_enum_android_control_awb_state_t& eAWBState);
    MBOOL setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode);


    MBOOL SetAETargetMode(MINT32 const i4SensorDev, eAETargetMODE mode); //for hdr
     /* @brief set AEAAOmode control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4AEAAOmode 0:8/12bits 1:14bits
     */
    MBOOL setAAOMode(MINT32 i4SensorDev, MUINT32 u4AWBAAOmode);

    MBOOL getAAOConfig(MINT32 i4SensorDev, AWB_AAO_CONFIG_Param_T &rAWBConfig);

   MBOOL setMWBColorTemperature(MINT32 i4SensorDev, int colorTemperature);

   MBOOL getSupportMWBColorTemperature(MINT32 i4SensorDev, MUINT32 &max, MUINT32 &min);

   MBOOL getAWBColorTemperature(MINT32 i4SensorDev, MUINT32 &colorTemperature);

   MBOOL SetTorchMode(MINT32 i4SensorDev, MBOOL is_torch_now);

   MBOOL SetMainFlashInfo(MINT32 i4SensorDev, MBOOL is_main_flash_on);

   MBOOL CallBackAwb(MINT32 i4SensorDev, MVOID* pIn, MVOID* pOut);

   MBOOL SetAWBFlare(MINT32 i4SensorDev, MUINT32 Flare, MUINT32 FlareOffset);

   MRESULT configReg(MINT32 i4SensorDev, AWBResultConfig_T *pResultConfig);

   MBOOL getAWBInfo(MINT32 i4SensorDev, AWB_ISP_INFO_T& rAWBInfo);

   MBOOL getPostgain(MINT32 i4SensorDev, AWB_GAIN_T& rPostgain);

   MBOOL setPostgain(MINT32 i4SensorDev, AWB_GAIN_T rPostgain);

   MBOOL setPostgainLock(MINT32 i4SensorDev, MBOOL bPostgainLock);

   MBOOL getAWBStatInfo(MINT32 i4SensorDev, AWB_STAT_INFO_T& rAWBStatInfo);
   
   MBOOL setAWBStatInfo(MINT32 i4SensorDev, AWB_STAT_INFO_T rAWBStatInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MRESULT CCTOPAWBEnable(MINT32 i4SensorDev);
    MRESULT CCTOPAWBDisable(MINT32 i4SensorDev);
    MRESULT CCTOPAWBGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBSetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain);
    MRESULT CCTOPAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 u4CamScenarioMode = CAM_SCENARIO_PREVIEW);
    MRESULT CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, CAM_SCENARIO_T eIdx, MVOID *a_pAWBNVRAM);
    MRESULT CCTOPAWBSaveNVRAMParam(MINT32 i4SensorDev);
    MRESULT CCTOPAWBSetAWBMode(MINT32 i4SensorDev, MINT32 a_AWBMode);
    MRESULT CCTOPAWBGetAWBMode(MINT32 i4SensorDev, MINT32 *a_pAWBMode, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetLightProb(MINT32 i4SensorDev, MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBBypassCalibration(MINT32 i4SensorDev, MBOOL bBypassCalibration);
    MRESULT CCTOPFlashAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM);
    MRESULT CCTOPFlashAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM, MUINT32 *a_pOutLen);
    MRESULT CCTOPFlashAWBSaveNVRAMParam(MINT32 i4SensorDev);
    MRESULT CCTOPFlashCalibrationSaveNVRAMParam(MINT32 i4SensorDev);
    MRESULT CCTOPAWBApplyK71NVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM);
};

};  //  namespace NS3Av3
#endif // _AWB_MGR_IF_H_

