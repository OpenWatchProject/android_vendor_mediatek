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
 * @file ae_mgr.h
 * @brief AE manager
 */

#ifndef _AE_MGR_IF_H_
#define _AE_MGR_IF_H_

#include <private/aaa_hal_private.h>
#include <custom/aaa/AEPlinetable.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <dbg_aaa_param.h>
#include <dbg_ae_param.h>
#include "camera_custom_hdr.h"
#include <ae_pline_custom.h>
#include <isp_config/ae_config.h>
#include <tuning/ae_flow_custom.h>
#include <vector>
#include "algorithm/ccu_ae_param.h"


/**
 * @brief AE exposure mode define
 */
typedef enum {
    eAE_EXPO_TIME = 0,     /*!< AE exposure by time */
    eAE_EXPO_LINE             /*!< AE exposure by line */
}eAE_EXPO_BASE;

typedef struct {
    strAEPLineInfomation AEPLineInfomation;
    strAEPLineMapping AEPLineMapping[30];
    strAFPlineInfo normalAFPlineInfo;    // normal mode
    strAFPlineInfo ZSDAFPlineInfo;
    strAEPLineGainList AEPLineGainList;
}AE_EASYTUNING_PLINE_INFO_T;

namespace NS3Av3
{
/*******************************************************************************
*
*******************************************************************************/
/**
 * @brief AE manager interface
 */

class IAeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IAeMgr(IAeMgr const&);
    //  Copy-assignment operator is disallowed.
    IAeMgr& operator=(IAeMgr const&);

public:  ////
    IAeMgr() {}
    ~IAeMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief AE get instance
     */
    static IAeMgr& getInstance();
    /**
     * @brief camera preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_if.h
     */
    MRESULT cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam);
    /**
     * @brief camera start
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_if.h
     */
    MRESULT Start(MINT32 i4SensorDev);
    /**
     * @brief uninit
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT uninit(MINT32 i4SensorDev);
    /**
     * @brief uninit
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT Stop(MINT32 i4SensorDev);
    /**
     * @brief set AE meter area window and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] sNewAEMeteringArea meter area; please refer to aaa_hal_if.h
     */
    MRESULT setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea);
    /**
     * @brief set AE EV compensation index and step
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewEVIndex EV index value;
     * @param [in] fStep Step value; The EV compensation value is equal i4NewEVIndex*fStep
     */
    MRESULT setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep);
    /**
     * @brief set AE metering mode for AE control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEMeteringMode metering mode value; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode);
    /**
     * @brief set AE ISO speed
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewAEISOSpeed ISO value; "0" means "Auto"
     */
    MRESULT setAEISOSpeed(MINT32 i4SensorDev, MUINT32 i4NewAEISOSpeed);
    /**
     * @brief set AE min / max frame rate value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewAEMinFps minimun frame rate value
     * @param [in] i4NewAEMaxFps maximun frame rate value
     */
    MRESULT setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    /**
     * @brief set Anti-banding mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEFLKMode flicker mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode);
    /**
    * @brief set Flicker frame rate active being adjustable
    * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
    * @param [in] a_bFlickerFPSAvtive Booling;
    */
    MRESULT setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive);
    /**
     * @brief set camera mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAECamMode camera mode; please refer to CamDefs.h
     */
    MRESULT setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode);
    /**
     * @brief enable AE manual pline
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eAEManualPline enum for AE manual pline mode; please refer to EAEManualPline_T in ae_pline_custom.h
     * @param [in] bEnable booling for enable AE manual pline;
     */
    MRESULT enableAEManualPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualPline, MBOOL bEnable);
    /**
     * @brief enable AE manual capture pline
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eAEManualCapPline enum for AE manual capture pline mode; please refer to EAEManualPline_T in ae_pline_custom.h
     * @param [in] bEnable booling for enable AE manual capture pline;
     */
    MRESULT enableAEManualCapPline(MINT32 i4SensorDev, EAEManualPline_T eAEManualCapPline, MBOOL bEnable);
    /**
     * @brief set capture mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEShotMode capture mode; please refer to CamDefs.h
     */
    MRESULT setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode);
    /**
     * @brief set AEHDRmode control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4AEHDRmode 0:OFF 1:ON 2:AUTO
     */
    MRESULT setAEHDRMode(MINT32 i4SensorDev, MUINT32 u4AEHDRmode);
    /**
     * @brief set getAEHDROnOff control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return HDR detection result value; please refer to HDRDetectionResult in Defs.h
     */
    MINT32 getAEHDROnOff(MINT32 i4SensorDev);
    /**
     * @brief set Scene Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewScene Scene mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewAEScene);
    /**
     * @brief get AE scene
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE scene value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEScene(MINT32 i4SensorDev);
    /**
     * @brief set AE Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEmode AE mode; please refer to AEPlinetable.h
     */
    MRESULT setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEMode);
    /**
     * @brief get AE mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE mode value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEMode(MINT32 i4SensorDev);
    /**
     * @brief get AE state
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE state value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEState(MINT32 i4SensorDev);
    /**
     * @brief AP lock or unlock AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAPAELock lock AE (MTRUE) or unlock AE (MFALSE)
     */
    MRESULT setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock);
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
     * @brief post capture AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] bIsStrobe On or OFF strobe
     */
    MRESULT doPostCapAE(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe);
    /**
     * @brief Auto focus AE calculate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doAFAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief AE calculate the capture parameters
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bIsStrobeFired On or OFF strobe
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doPreCapAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief send the capture paramters to sensor and isp
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT doCapAE(MINT32 i4SensorDev);
    /**
     * @brief Calculate the preview AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doPvAE(MINT32 i4SensorDev, MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief get AE luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE luminance value
     */
    MINT32 getLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn);
    /**
     * @brief get over exposure area AE luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE luminance value
     */
    MINT32 getAOECompLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn);
    /**
     * @brief get AE capture luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE capture luminance value
     */
    MINT32 getCaptureLVvalue(MINT32 i4SensorDev);
    /**
     * @brief the strobe trigger threshold is bigger than brightness value or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return Strobe trigger threshold is bigger than brightness value (MTRUE) or strobe trigger threshold is smaller than brightness value (MFALSE)
     */
    MBOOL IsStrobeBVTrigger(MINT32 i4SensorDev);
    /**
     * @brief get AE meter area luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
    */
    MRESULT getAEMeteringYvalue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    /**
     * @brief get AE meter block area luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
     * @param [out] u2YCnt total counter value
    */
    MRESULT getAEMeteringBlockAreaValue(MINT32 i4SensorDev, CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt);
    /**
     * @brief get High dynamic range capture information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] strHDROutputInfo capture information; please refer to aaa_hal_if.h
    */
    MRESULT getHDRCapInfo(MINT32 i4SensorDev, Hal3A_HDROutputParam_T & strHDROutputInfo);
    /**
     * @brief set phone rotate degree
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4RotateDegree rotate degree. The value is 0, 90, 180 or 270 only.
     */
    MRESULT setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree);
    /**
     * @brief get AE algorithm condition result
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] i4AECondition AE condition value. please refer to Ae_param.h
     */
    MBOOL getAECondition(MINT32 i4SensorDev, MUINT32 i4AECondition);
    /**
     * @brief get LCE AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rLCEInfo LCE AE information; please refer to aaa_hal_if.h
    */
    MRESULT getLCEPlineInfo(MINT32 i4SensorDev, LCEInfo_T &a_rLCEInfo);
    /**
     * @brief get Face AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the luminance value change
    */
    MINT16 getAEFaceDiffIndex(MINT32 i4SensorDev);
    /**
     * @brief update the sensor delay information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4SutterDelay sensor shutter delay information
     * @param [in] i4SensorGainDelay sensor gain delay information
     * @param [in] i4IspGainDelay isp gain delay information
    */
    MRESULT updateSensorDelayInfo(MINT32 i4SensorDev, MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    /**
     * @brief get brightness value by frame
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] bFrameUpdate frame update (MTRUE) or no update (MFALSE)
     * @param [out] iYvalue luminance value
    */
    MRESULT getBrightnessValue(MINT32 i4SensorDev, MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    /**
     * @brief get AE NVRAM data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAENVRAM AE NVRAM structure
    */
    MINT32 getAENvramData(MINT32 i4SensorDev, AE_NVRAM_T &rAENVRAM);
    /**
     * @brief get NVRAM data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
    */
    MRESULT getNvramData(MINT32 i4SensorDev);
    /**
     * @brief get AE block value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] pYvalues pointer Y value
     * @param [in] size block numbers
    */
    MRESULT getAEBlockYvalues(MINT32 i4SensorDev, MUINT8 *pYvalues, MUINT8 size);
    /**
     * @brief get AE sensor active cycle
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] i4ActiveCycle pointer cycle value
    */
    MRESULT getAESensorActiveCycle(MINT32 i4SensorDev, MINT32* i4ActiveCycle);
    /**
     * @brief set Video dynamic frame rate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bVdoDynamicFps enable or disable video dynamic frame rate
    */
    MRESULT setVideoDynamicFrameRate(MINT32 i4SensorDev, MBOOL bVdoDynamicFps);
    /**
     * @brief provide the SGG1Gain for AF used
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] pSGG1Gain output the SGG1 gain value
    */
    MRESULT getAESGG1Gain(MINT32 i4SensorDev, MUINT32 *pSGG1Gain);
    /**
     * @brief get AE Pline table
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eTableID input the table ID; please refer to AEPlinetable.h
     * @param [out] a_AEPlineTable output the Pline table pointer
    */
    MRESULT getAEPlineTable(MINT32 i4SensorDev, eAETableID eTableID, strAETable &a_AEPlineTable);
    /**
     * @brief is Need Update sensor by I2C
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
    */
    MBOOL IsNeedUpdateSensor(MINT32 i4SensorDev);
    /**
     * @brief get AeMgr autoflicker state
     *
    */
    MINT32 getAEAutoFlickerState(MINT32 i4SensorDev);
    /**
     * @brief get TG interrupt AE information
    */
    MRESULT getTgIntAEInfo(MINT32 i4SensorDev, MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio);
    /**
     * @brief is multi capture mode
     * @param [in] bMultiCap input the multi capture mode (MTRUE) or not (FALSE)
    */
    MRESULT IsMultiCapture(MINT32 i4SensorDev,MBOOL bMultiCap);
    /**
     * @brief is AE more OB2 on@ B+M MFHR
     * @param [in] AE denoise mode 1:BMDN 2:MFHR
    */
    MRESULT enableStereoDenoiseRatio(MINT32 i4SensorDev, MUINT32 u4enableStereoDenoise);
    /**
     * @brief set TG info
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4TGInfo TG info
     */
    MBOOL setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo);
    /**
     * @brief is check the brightness change
     * @param [out] output the change over or not
    */
    MBOOL isLVChangeTooMuch(MINT32 i4SensorDev);
    /**
     * @brief lock the black level
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bLockBlackLevel MTRUE : lock, MFALSE : unlock
    */
    MRESULT bBlackLevelLock(MINT32 i4SensorDev, MBOOL bLockBlackLevel);
    /**
     * @brief get the sensor params
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rSensorInfo output the frame durationtable, exposure time, and ISO senstivity
    */
    MRESULT getSensorParams(MINT32 i4SensorDev, AE_SENSOR_PARAM_T &a_rSensorInfo);
    /**
     * @brief get the AE config params
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] bHDRen outputs that it enables HDR LSB 4bits or not
     * @param [out] bOverCnten outputs that it enables Overexposure count or not
     * @param [out] bTSFen outputs that it enables TSF statistic or not
    */
    MRESULT getAEConfigParams(MINT32 i4SensorDev, MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen);
    /**
     * @brief get AAO size
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] u4BlockNumW/u4BlockNumH are AAO width and height
     */
    MRESULT getAAOSize(MINT32 i4SensorDev, MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH);
    /**
     * @brief get AE to AF info
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] rAEInfo output AE to AF info: please refer to Aaa_hal_private.h
     */
    MRESULT getAE2AFInfo(MINT32 i4SensorDev, CCU_AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo);
    /**
     * @brief reconfig AE ISP statistic
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pDBinInfo input point to BinInfo; please refer to Cam_Notify.h
     * @param [out] pOutRegCfg output Register config: please refer to Cam_Notify.h
     */
    MRESULT reconfig(MINT32 i4SensorDev, MVOID *pDBinInfo, MVOID *pOutRegCfg);
    /**
     * @brief set AE digital zoom ratio information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] zoom ratio information
     */
    MRESULT setDigZoomRatio(MINT32 i4SensorDev, MINT32 i4ZoomRatio);
    /**
     * @brief reset mvhdr ratio
     * @param [in] bReset 0: not reset, 1: reset
     */
    MRESULT resetMvhdrRatio(MINT32 i4SensorDev, MBOOL bReset);
    /**
     * @brief get AE ISO thres status
     * @param [out] a_ISOIdx1Status ISO1 status value
     * @param [out] a_ISOIdx2Status ISO2 status value
     */
	MRESULT getISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    /**
     * @brief reset AE ISO thres status
     * @param [out] a_ISOIdx1Status reset ISO1 status value
     * @param [out] a_ISOIdx2Status reset ISO2 status value
     */
    MRESULT resetGetISOThresStatus(MINT32 i4SensorDev, MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    /**
     * @brief Notify AE Mgr when 3A hal init CCU
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bInit: true when CCU init
     */
    MBOOL IsCCUAEInit(MINT32 i4SensorDev,MBOOL bInit);
    /**
     * @brief set request number
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4ReqNum for request number
     */
    MRESULT setRequestNum(MINT32 i4SensorDev, MUINT32 u4ReqNum);
    /**
     * @brief get AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAEPerframeInfo: AE per-frame information
     */
    MRESULT getAEInfo(MINT32 i4SensorDev, AE_PERFRAME_INFO_T& rAEPerframeInfo);
    /**
     * @brief get AE ISP config register
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] pResultConfig: AE isp register pointer
     */
    MRESULT configReg(MINT32 i4SensorDev, AEResultConfig_T *pResultConfig);
    /**
     * @brief send AE ctrl
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eAECtrl AE enum; please refer to EAECtrl_T in ae_flow_custom.h
     * @param [in/out] iArg1/iArg2/iArg3: AE data pointer
     */
	MRESULT sendAECtrl(MINT32 i4SensorDev, EAECtrl_T eAECtrl, MINTPTR iArg1, MINTPTR iArg2, MINTPTR iArg3, MINTPTR iArg4);
    /**
     * @brief set AE Params
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] AE_PARAM_SET_INFO; please refer to AE_PARAM_SET_INFO in aaa_hal_private.h
     */
	MRESULT setAEParams(MINT32 i4SensorDev, AE_PARAM_SET_INFO const &rNewParam);
    // CCT feature APIs.
    /**
     * @brief Enable AE
    */
    MINT32 CCTOPAEEnable(MINT32 i4SensorDev);
    /**
     * @brief Disable AE
    */
    MINT32 CCTOPAEDisable(MINT32 i4SensorDev);
    /**
     * @brief get AE enable/disable information
     * @param [out] a_pEnableAE pointer for enable/disable information
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    /**
     * @brief set AE scene mode information
     * @param [out] a_AEScene value
    */
    MINT32 CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene);
    /**
     * @brief get AE scene mode information
     * @param [out] a_pAEScene pointer for change scene mode information
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetAEScene(MINT32 i4SensorDev, MINT32 *a_pAEScene, MUINT32 *a_pOutLen);
    /**
     * @brief set AE metering mode information
     * @param [in] a_AEMeteringMode AE metering mode
    */
    MINT32 CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode);
    /**
     * @brief apply AE exposure relative parameters
     * @param [in] a_pAEExpParam pointer to input exposure relative parameters
    */
    MINT32 CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief set AE flicker mode information
     * @param [in] a_AEFlickerMode AE flicker mode value Auto,60Hz,50Hz,OFF
    */
    MINT32 CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode);
    /**
     * @brief get AE exposure relative parameters
     * @param [in] a_pAEExpParamIn pointer don't input anything
     * @param [out] a_pAEExpParamOut pointer to output exposure relative parameters
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    /**
     * @brief get AE flicker mode information
     * @param [out] a_pAEFlickerMode pointer for flicker mode
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetFlickerMode(MINT32 i4SensorDev, MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    /**
     * @brief get AE metering mode information
     * @param [out] a_pAEMEteringMode pointer for output metering mode value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetMeteringMode(MINT32 i4SensorDev, MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    /**
     * @brief apply AE NVRAM paramters
     * @param [in] a_pAENVRAM pointer for NVRAM data
    */
    MINT32 CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 u4AENVRAMIdx = 0);
    /**
     * @brief get AE NVRAM parameter value
     * @param [in] a_pAENVRAM pointer to output NVRAM data
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    /**
     * @brief save AE NVRAM parameters to NVRAM
    */
    MINT32 CCTOPAESaveNVRAMParam(MINT32 i4SensorDev);
    /**
     * @brief get current EV value
     * @param [out] a_pAECurrentEV pointer to output EV value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetCurrentEV(MINT32 i4SensorDev, MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    /**
     * @brief lock AE exposure setting
    */
    MINT32 CCTOPAELockExpSetting(MINT32 i4SensorDev);
    /**
     * @brief unlock AE exposure setting
    */
    MINT32 CCTOPAEUnLockExpSetting(MINT32 i4SensorDev);
    /**
     * @brief set AE capture parameters value
     * @param [in] a_pAEExpParam pointer to set capture parameters
    */
    MINT32 CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief get AE capture parameters value
     * @param [in] a_pAEExpParam pointer to output capture parameters
    */
    MINT32 CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief set AE target value
     * @param [in] u4AETargetValue set target value for AE reference
    */
    MINT32 CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue);
    /**
     * @brief apply AE Pline NVRAM paramters
     * @param [in] a_pAEPlineNVRAM pointer for AE Pline NVRAM data
    */
    MINT32 CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM);
    /**
     * @brief get AE Pline parameter value
     * @param [in] a_pAEPlineNVRAM pointer to output AE Pline NVRAM data
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM,MUINT32 *a_pOutLen);
    /**
     * @brief save AE Pline parameters to NVRAM
    */
    MINT32 CCTOPAESavePlineNVRAM(MINT32 i4SensorDev);
};

};  //  namespace NS3Av3
#endif // _AE_MGR_IF_H_

