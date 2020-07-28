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
#ifndef _HAL_3A_RAWIMP_H_
#define _HAL_3A_RAWIMP_H_

#include <cutils/properties.h>
#include <string.h>
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/std/Log.h>
#include <IThreadRaw.h>
#include <mtkcam/drv/IHalSensor.h>
#include <Hal3ACbHub.h>

#include "Hal3ARaw.h"
#include <ae_mgr/ae_mgr.h>
#include <awb_mgr/awb_mgr_if.h>
#include <sensor_mgr/aaa_sensor_mgr.h>

#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/sys/IFileCache.h>

// Task header
#include <task/ITaskMgr.h>
#include <IThread.h>

#include <private/IopipeUtils.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <dbg_aaa_param.h>
#include <ISync3A.h>
#include <aaa_hal_sttCtrl.h>
#include <debug/DebugUtil.h>
#include <aaa_common_custom.h>
#include <ResultPool4Module.h>
#include <Hal3APolicy.h>
#if CAM3_AF_FEATURE_EN
#include <dbg_af_param.h>
#include <af_feature.h>
extern "C"{
#include <af_algo_if.h>
}
#include <af_define.h>
#include <af_mgr/af_mgr_if.h>
#include <lens/mcu_drv.h>
#include <pd_mgr_if.h>
#endif

#if CAM3_FLASH_FEATURE_EN
#include <flash_hal.h>
#include <flash_mgr.h>
#endif
#include <flash_feature.h>

#if CAM3_FLICKER_FEATURE_EN
#include <flicker_hal_if.h>
#endif

#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

#include "cct_server.h"
#include "cctsvr_entry.h"

#include <iccu_ctrl_3actrl.h>
#include <ccu_ext_interface/ccu_ae_extif.h>
#include <ccu_ext_interface/ccu_ltm_extif.h>

//CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
#include <list>
#include <drv/cq_tuning_mgr.h>
#endif

#include "private/PDTblGen.h"

#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener

// ResultPool and config header
#include <IResultPool.h>
#include <isp_config/af_config.h>
#include <isp_config/isp_pdo_config.h>
#include <isp_config/ae_config.h>
#include <isp_config/awb_config.h>
#include <isp_config/isp_flicker_config.h>
#include <ResultPool4LSCConfig.h>

#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// Index manager
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <mtkcam/utils/mapping_mgr/idx_cache.h>
#include <nvbuf_util.h>
#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>
#include "camera_custom_msdk.h"

//Thread Use
#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include "iccu_mgr.h"
using namespace NSCcuIf;
#include "SttBufQ.h"
#include <faces.h>

// DP5.0 check sum
#include "aaa_exif_tag_chksum.h"
#include <isp_tuning_custom.h>
#include <tuning/shading_flow_custom.h>

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define MY_FORCEINLINE inline __attribute__((always_inline))

// define g/gyro info
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

using namespace std;
using namespace NSIspExifDebug;
using namespace NSCam::TuningUtils;

class Hal3ARawImp : virtual public Hal3ARaw
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID       destroyInstance();
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause();
    virtual MVOID       resume(MINT32 MagicNum = 0);
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario);
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    virtual MBOOL       setStereoParams(Stereo_Param_T const &rNewParam);
    virtual MBOOL       autoFocus();
    virtual MBOOL       cancelAutoFocus();
    virtual MVOID       setFDEnable(MBOOL fgEnable);
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/, MINT32 i4P1DoneSttNum = -1);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    virtual MBOOL       chkLampFlashOnCond() const;
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       isFlashOnCapture() const;
    virtual MBOOL       chkCapFlash() const {return m_bIsHighQualityCaptureOn;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId) const;
    virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MINT32      queryMagicNumber() const {return m_i4SttMagicNumber;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       notifyP1PwrOn(); //Open CCU power.
    virtual MBOOL       notifyP1PwrOff(); //Close CCU power.
    virtual MBOOL       dumpP1Params(MINT32 i4MagicNum);
    virtual MBOOL       preset(Param_T const &rNewParam);
    virtual MVOID       querySensorStaticInfo();
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum);
    virtual MBOOL       notify4CCU();
    virtual MVOID       notifyPreStop();
    virtual MINT32      taskEnumToAeState(MINT32 i4TaskEnum);
    virtual MBOOL       setCCUParams();
    static  MVOID*      ThreadAEStart(MVOID*);
    static  MVOID*      ThreadAFStart(MVOID*);

protected:  //    Ctor/Dtor.
                        Hal3ARawImp(MINT32 const i4SensorDevId);
    virtual             ~Hal3ARawImp(){}

                        Hal3ARawImp(const Hal3ARawImp&);
                        Hal3ARawImp& operator=(const Hal3ARawImp&);

    MBOOL               init(MINT32 const i4SensorOpenIndex);
    MBOOL               uninit();
    MRESULT             updateTGInfo();
    MINT32              config(const ConfigInfo_T& rConfigInfo);
    MBOOL               get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    MBOOL               getASDInfo(ASDInfo_T &a_rASDInfo) const;
    MBOOL               getP1DbgInfo(AAA_DEBUG_INFO1_T& rDbg3AInfo1, DEBUG_SHAD_ARRAY_INFO_T& rDbgShadTbl, AAA_DEBUG_INFO2_T& rDbg3AInfo2) const;
    // Flash flow
    MBOOL               checkAndOpenFlash(MINT32 i4Type);
    MBOOL               checkAndCloseFlash();
    MVOID               doBackup2A();
    MVOID               doRestore2A();
    MVOID               handleBadPicture(MINT32 i4ReqMagic, MINT32 i4P1DoneSttNum = -1);
    MUINT32             getNvramIndex(CAM_IDX_QRY_COMB& rMappingInfo, EModule_T module, MUINT32& index);
    MINT32              getIsDoSync3A(MINT32 i4Sync3ASwitchOn);

    MINT32              m_3ALogEnable;
    MINT32              m_i4IdxCacheLogEnable;
    MINT32              m_i4CopyLscP1En;
    MINT32              m_3ACctEnable;
    MINT32              m_DebugLogWEn;
    volatile std::atomic<int>        m_Users;
    mutable std::mutex  m_Lock;
    std::mutex          mP2Mtx;
    std::mutex          m3AOperMtx1;
    std::mutex          m3AOperMtx2;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorDev;
    MINT32              m_i4sensorId;
    MUINT32             m_u4SensorMode;
    MUINT               m_u4TgInfo;
    MBOOL               m_bAAOMode;
    MBOOL               m_bEnable3ASetParams;
    MBOOL               m_bFaceDetectEnable;
    MINT32              m_i4TgWidth;
    MINT32              m_i4TgHeight;
    MBOOL               mbIsHDRShot;
    MINT32              m_i4HbinWidth;
    MINT32              m_i4HbinHeight;
    MBOOL               m_fgEnableShadingMeta;
    MBOOL               m_bIsRecordingFlash;
    MINT32              m_i4EVCap;
    MINT32              m_i4SensorPreviewDelay;
    MINT32              m_i4AeShutDelayFrame;
    MINT32              m_i4AeISPGainDelayFrame;
    MBOOL               m_bPreStop;

    MINT32              m_i4OverrideMinFrameRate;
    MINT32              m_i4OverrideMaxFrameRate;
    MINT32              m_i4AETargetMode = AE_MODE_NORMAL;
    MINT32              m_u4LastRequestNumber;
    MBOOL               m_bFlashOpenedByTask;
    MINT32              m_i4FlashType;
    MBOOL               m_bFrontalBin;
    MUINT8              m_u1LastCaptureIntent;
    pthread_t           m_ThreadAE;
    pthread_t           m_ThreadAF;

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IThreadRaw*         m_pThreadRaw;
    ITaskMgr*           m_pTaskMgr;
    NSCcuIf::ICcuMgrExt* m_pICcuMgr;
    Param_T             m_rParam;
    AF_Param_T          m_rAfParam;
    ScenarioParam       m_sParam;
    ScenarioParam       m_sPreParam;
    I3ACallBack*        m_pCbSet;
    MINT32              m_i4SttMagicNumber;
    MBOOL               m_bIsHighQualityCaptureOn;
    MBOOL               m_bIsCapEnd;
    MBOOL               m_bIsFlashOpened;
    MBOOL               m_bIsSkipSync3A;
    MINT32              m_i4StereoWarning;
    std::vector<MINT32> m_vShortExpFrame;
    MINT32              m_i4ShortExpCount;

    NSCcuIf::ICcuCtrl3ACtrl* m_pCcuCtrl3ACtrl;
#if CAM3_STEREO_FEATURE_EN
    Stereo_Param_T m_rLastStereoParam;
#endif

    //CmdQ
#if defined(MTKCAM_CMDQ_AA_SUPPORT)
    CqTuningMgr*        m_pCqTuningMgr;
#endif
    IResultPool*        m_pResultPoolObj;
    Hal3ACbHub*         m_pCallbackHub;
    IdxMgr*             m_pIdxMgr;
    CAM_IDX_QRY_COMB    m_Mapping_Info_3A;
    Query_3A_Index_T    m_3A_Index;
    MINT32              m_faceNum;
    MBOOL               m_bIsFirstSetParams;
    MINT32              m_i4CamModeEnable;
    Hal3ASttCtrl*       m_p3ASttCtrl;
    MINT32              m_i4OperMode;
    AFStaticInfo_T      m_rAFStaticInfo;
    MUINT32             m_i4EffectiveFrame;
    MINT32              m_i4SubsampleCount;
    MUINT32             m_u4PrePresetKey;
    SMVR_GAIN_DELAY_T   m_rGainDelay;
    SLOW_MOTION_3A_INFO_T m_rNew3AInfo;
    IThreadRaw::AFParam_T m_rThreadRawAFPAram;
    std::mutex          mPostCommandMtx;
    IdxCache            m_rIdxCache;
    MINT32              m_i4BypassStt;
};

#endif
