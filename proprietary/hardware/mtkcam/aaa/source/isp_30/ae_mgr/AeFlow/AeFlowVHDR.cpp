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
#define LOG_TAG "AeFlowVHDR"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeFlowVHDR.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"


#define AE_FLOW_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_FLOW_LOG_IF(cond, fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE_IF(cond, fmt, ##arg); \
        } else { \
            CAM_LOGD_IF(cond, fmt, ##arg); \
        } \
    }while(0)


        
#define EN_3A_SCHEDULE_LOG    2

using namespace NS3Av3;
using namespace NSIspTuning;


IAeFlow*
AeFlowVHDR::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeFlowVHDR singleton_main(ESensorDev_Main);
        AE_FLOW_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeFlowVHDR singleton_main2(ESensorDev_MainSecond);
        AE_FLOW_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_MainThird:  //  Main Third Sensor
        static AeFlowVHDR singleton_main3(ESensorDev_MainThird);
        AE_FLOW_LOG("ESensorDev_MainThird(%p)", &singleton_main3);
        return &singleton_main3;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowVHDR singleton_sub(ESensorDev_Sub);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeFlowVHDR singleton_sub2(ESensorDev_SubSecond);
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeFlowVHDR::
AeFlowVHDR(ESensorDev_T sensor)
    : AeFlowDefault(sensor)
    , m_u4AEISOSpeed(0)
    , m_bRealISOSpeed(MFALSE)
{
    AE_FLOW_LOG("Enter AeFlowVHDR DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeFlowVHDR::
schedulePvAE(MVOID* input, MVOID* output)
{
    AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;
    AE_FLOW_SCHEDULE_OUTPUT_T* poutputinfo = (AE_FLOW_SCHEDULE_OUTPUT_T*) output;
    
    MBOOL bIsHDRflow = (pinputinfo->eAETargetMode != AE_MODE_NORMAL);
    MINT32 i4ActiveItem = 0;
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    m_u4HwMagicNum = pinputinfo->u4HwMagicNum;
    m_i4WaitVDNum = pinputinfo->i4WaitVDNum;
    m_bPerframeAEFlag = pinputinfo->bPerframeAEFlag;
    m_bTouchAEAreaChange = pinputinfo->bTouchAEAreaChange;
    m_eAETargetMode = pinputinfo->eAETargetMode;
    m_bAAASchedule  = pinputinfo->bAAASchedule;
    m_bAElimitor = pinputinfo->bAElimitor;

    if(m_bPerframeAEFlag){
        switch(pinputinfo->eAEFlowType){
            case E_AE_FLOW_CPU_DOPVAE:
                if (m_eAETargetMode == AE_MODE_MVHDR_TARGET){
                    m_bPerframeAEFlag = MFALSE;
                    monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is non-Perframe mVHDR AE flow  \n",__FUNCTION__);
                } else if (m_bTouchAEAreaChange){
                    if (m_pAEParam->strAEParasetting.bEnableTouchSmooth){
                        bCalculateAE  = MTRUE ;
                        bApplyAE = MTRUE ;
                        AE_FLOW_LOG( "[monitorAndReschedule:Perframe touch HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d \n",
                          bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                    }else{
                        m_bPerframeAEFlag = MFALSE;
                        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    }
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Touch HDR AE flow, bTouchPerframe:%d \n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
                } else {
                    bCalculateAE  = MTRUE ;
                    bApplyAE = MTRUE ;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Perframe HDR AE flow \n",__FUNCTION__);
                    AE_FLOW_LOG( "[monitorAndReschedule:Perframe HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d \n",
                      bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                }
            break;
        case E_AE_FLOW_CPU_DOAFAE:
            if (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag &&(pinputinfo->bAEReadyCapture == MFALSE)){
                bCalculateAE  = MTRUE ;
                bApplyAE = MTRUE ;
                AE_FLOW_LOG( "[monitorAndReschedule:Perframe Touch HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d \n",
                      bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
            }else{
                monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
            }
            break;
        case E_AE_FLOW_CPU_DOPRECAPAE:
            monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
            break;
        case E_AE_FLOW_CPU_DOCAPAE:
        case E_AE_FLOW_CPU_DOAFASSIST:
            break;
        }
    }else{
        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
    }
    poutputinfo->pModifiedApply = &bApplyAE;
    poutputinfo->pModifiedCalc = &bCalculateAE;
    poutputinfo->pModifiedPerframeFlag = &m_bPerframeAEFlag;
    poutputinfo->pWaitVDNum = &m_i4WaitVDNum;
}

MVOID
AeFlowVHDR::
calculateAE(MVOID* input, MVOID* output)
{
    AE_FLOW_CALCULATE_INFO_T* pinputinfo = (AE_FLOW_CALCULATE_INFO_T*) input;
    strAEOutput* poutputinfo = (strAEOutput*) output;
    strAEInput rAEInput;
    memset(&rAEInput , 0, sizeof(strAEInput ));
    m_u4AEISOSpeed = pinputinfo->u4AEISOSpeed;
    m_bRealISOSpeed = pinputinfo->bRealISOSpeed;
    switch(pinputinfo->eAEFlowType){
        case E_AE_FLOW_CPU_DOPVAE:
            if(pinputinfo->bAElock|| m_bAdbAELock) {
                rAEInput.eAeState = AE_STATE_AELOCK;
                AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] AE lock state \n", __FUNCTION__);
            } else if(pinputinfo->bAEOneShotControl){
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG( "[%s()] AE_STATE_ONE_SHOT", __FUNCTION__);
            } else if(m_bTouchAEAreaChange) {
                if (m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag )
                    rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                else
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Enable per-frame AE for touch:%d\n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
            } else if (m_bAElimitor) {
                rAEInput.eAeState = AE_STATE_PANORAMA_MODE;
                AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] PANORAMA \n", __FUNCTION__);
            } else {
                if ((m_eAETargetMode == AE_MODE_MVHDR_TARGET)||(!m_bPerframeAEFlag)){
                    rAEInput.eAeState = AE_STATE_SLOW_MOTION;
                    AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] VHDR non-perframe flow\n", __FUNCTION__);
                } else {
                    rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
                }
            }
            break;
        case E_AE_FLOW_CPU_DOAFAE:
            if(pinputinfo->bAElock || m_bAdbAELock) {
                rAEInput.eAeState = AE_STATE_AELOCK;
            }else if (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag &&(!pinputinfo->bAEReadyCapture)){
                rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                AE_FLOW_LOG("[doAFAE] AE_STATE_TOUCH_PERFRAME :%d\n",m_bPerframeAEFlag);
            }else{
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG( "[doAFAE] AE_STATE_ONE_SHOT\n");
            }
            break;
        case E_AE_FLOW_CPU_DOPRECAPAE:
            if(pinputinfo->bAElock || m_bAdbAELock) {
                rAEInput.eAeState = AE_STATE_AELOCK;
            }else{
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
            }
            break;
        case E_AE_FLOW_CPU_DOCAPAE:
            rAEInput.eAeState = AE_STATE_POST_CAPTURE;
            break;
        case E_AE_FLOW_CPU_DOAFASSIST:
            rAEInput.eAeState = AE_STATE_AFASSIST;
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: FlowType = %d \n", __FUNCTION__, m_eSensorDev, pinputinfo->eAEFlowType);
    }
    // Prepare param for algorithm
    rAEInput.pAESatisticBuffer = pinputinfo->pAEStatisticBuf;
    rAEInput.eAeTargetMode = m_eAETargetMode;
    rAEInput.i8TimeStamp = pinputinfo->i8TimeStamp;
    m_strAECycleInfo.m_u4HwMagicNum = m_u4HwMagicNum;
    // Prepare perframe AE info to algorithm
    if (m_bPerframeAEFlag){
        rAEInput.CycleInfo = m_strAECycleInfo;
        for (int i=0; i< MAX_AE_PRE_EVSETTING; i++) {
            rAEInput.PreEvSetting[i] = m_PreEvSettingQueue[i];
            rAEInput.u4PreAEidx[i] = m_u4PreAEidxQueue[i];
            rAEInput.u4PreTgIntAEidx[i] = m_u4PreTgIntAEidxQueue[i];
        }
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s():PassToAlgo] EvSetting/AEidx/Tgidx [N-1]:%d/%d/%d/%d/%d [N-2]:%d/%d/%d/%d/%d [N-3]:%d/%d/%d/%d/%d FrameCnt/Tgcnt/Magic(%d/%d/%d)\n", __FUNCTION__,
                rAEInput.PreEvSetting[0].u4Eposuretime, rAEInput.PreEvSetting[0].u4AfeGain, rAEInput.PreEvSetting[0].u4IspGain, rAEInput.u4PreAEidx[0], rAEInput.u4PreTgIntAEidx[0],
                rAEInput.PreEvSetting[1].u4Eposuretime, rAEInput.PreEvSetting[1].u4AfeGain, rAEInput.PreEvSetting[1].u4IspGain, rAEInput.u4PreAEidx[1], rAEInput.u4PreTgIntAEidx[1],
                rAEInput.PreEvSetting[2].u4Eposuretime, rAEInput.PreEvSetting[2].u4AfeGain, rAEInput.PreEvSetting[2].u4IspGain, rAEInput.u4PreAEidx[2], rAEInput.u4PreTgIntAEidx[2],
                rAEInput.CycleInfo.m_i4FrameCnt, rAEInput.CycleInfo.m_i4FrameTgCnt, rAEInput.CycleInfo.m_u4HwMagicNum);

    }

    // Handle Full-frame AE
    AaaTimer localTimer("handleAE", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
    AAA_TRACE_G(handleAE);
    AAA_TRACE_NL(handleAE);
    m_pIAeAlgo->handleAE(&rAEInput, poutputinfo);
    AAA_TRACE_END_NL;
    AAA_TRACE_END_G;
    localTimer.End();

    // i/m/zVHDR params modification
    if (m_eAETargetMode == AE_MODE_IVHDR_TARGET){
        updatePreviewParamsByiVHDR(poutputinfo);
    } else if (m_eAETargetMode == AE_MODE_MVHDR_TARGET){
        updatePreviewParamsBymVHDR(poutputinfo);
    } else if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET){
        updatePreviewParamsByzVHDR(poutputinfo,pinputinfo->u4VHDRratio);
    }

    //Update AE algorihm cycle info
    updatePreEvSettingQueue(poutputinfo->EvSetting.u4Eposuretime, poutputinfo->EvSetting.u4AfeGain, poutputinfo->EvSetting.u4IspGain, poutputinfo->i4AEidxNext);
    output = poutputinfo;
    // AF assist avoid frame count maintain
    if (pinputinfo->eAEFlowType == E_AE_FLOW_CPU_DOAFASSIST)
        return;
    // Frame count maintain
    if((rAEInput.CycleInfo.m_i4FrameCnt != m_strAECycleInfo.m_i4FrameCnt)&&(m_bPerframeAEFlag))
    {
      if(rAEInput.CycleInfo.m_i4FrameCnt>0)
          m_strAECycleInfo.m_i4FrameCnt = rAEInput.CycleInfo.m_i4FrameCnt;
      else
          AE_FLOW_LOG("Wrong frameCnt:%d %d\n",m_strAECycleInfo.m_i4FrameCnt,rAEInput.CycleInfo.m_i4FrameCnt);
    }
}

AeFlowVHDR::
~AeFlowVHDR()
{

    AE_FLOW_LOG("Delete ~AeFlowVHDR - DeviceId:%d",(MUINT32)m_eSensorDev);

}

MVOID
AeFlowVHDR::
updatePreviewParamsByiVHDR(strAEOutput *sAEInfo)
{
}

MVOID
AeFlowVHDR::
updatePreviewParamsBymVHDR(strAEOutput *sAEInfo)
{
}

MRESULT
AeFlowVHDR::
updatePreviewParamsByzVHDR(strAEOutput *sAEInfo,MUINT32 u4VHDRratio)
{
   return S_AE_OK;
}


