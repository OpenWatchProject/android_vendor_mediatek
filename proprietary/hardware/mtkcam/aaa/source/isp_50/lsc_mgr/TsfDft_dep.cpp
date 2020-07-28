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
#define LOG_TAG "tsf_dft"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "TsfDft.h"
#include <LscUtil.h>

#include <ILscNvram.h>

#include <ae_param.h>
#include <ae_tuning_custom.h>
#include <shading_tuning_custom.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <mtkcam/def/PriorityDefs.h>
#include <ShadingTblTransform.h>

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <isp_mgr.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<TsfDft>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

#define STAT_OPT(hdr, over, tsf) (((hdr)?4:0)|((over)?2:0)|((tsf)?1:0))
#define TSF_SCN_DFT ESensorMode_Preview

#if 1 //(CAM3_3ATESTLVL > CAM3_3ASTTUT)
#define TSF_BUILD
#endif
#define TSF_THREAD_BUILD

#ifdef TSF_THREAD_BUILD
#define TSF_LOCK() ::pthread_mutex_lock(&m_Mutex)
#define TSF_UNLOCK() ::pthread_mutex_unlock(&m_Mutex)
#else
#define TSF_LOCK() /*::pthread_mutex_lock(&m_Mutex)*/
#define TSF_UNLOCK() /*::pthread_mutex_unlock(&m_Mutex)*/
#endif

#define TSF_ASP_CHG_OPT 2
#define TSF_STEP_COUNT 3  /*Cycle = 4*/


/*******************************************************************************
 * TsfDft
 *******************************************************************************/
ILscTsf*
TsfDft::
createInstance(MUINT32 u4SensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(u4SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<TsfDft>(u4SensorDev);
    } );

    return rSingleton.instance.get();
}

MVOID
TsfDft::
createTsf()
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    m_u4PerFrameStep = 0;
    m_prTsf = MTKTsf::createInstance();

    // create tsf instance
    if (!m_prTsf)
    {
        LSC_ERR("NULL TSF instance");
        m_fgThreadLoop = 0;
    }
    else
    {
        /*const AE_PARAM_TEMP_T* pAeParam;

        switch (m_u4SensorDev)
        {
        default:
        case ESensorDev_Main:
            pAeParam = &getAEParam<ESensorDev_Main>();
            break;
        case ESensorDev_Sub:
            pAeParam = &getAEParam<ESensorDev_Sub>();
            break;
        case ESensorDev_MainSecond:
            pAeParam = &getAEParam<ESensorDev_MainSecond>();
            break;
        case ESensorDev_SubSecond:
            pAeParam = &getAEParam<ESensorDev_SubSecond>();
            break;
        case ESensorDev_MainThird:
            pAeParam = &getAEParam<ESensorDev_MainThird>();
            break;
        }*/

        MTK_TSF_GET_ENV_INFO_STRUCT rTsfGetEnvInfo;
        m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &rTsfGetEnvInfo);
        LSC_LOG("MTKTSF_FEATURE_GET_ENV_INFO, buffer size(%d)", rTsfGetEnvInfo.WorkingBuffSize);

        // allocate working buffer
        MUINT8* gWorkinBuffer = new MUINT8[rTsfGetEnvInfo.WorkingBuffSize];
        ::memset(gWorkinBuffer, 0, rTsfGetEnvInfo.WorkingBuffSize);

        //this function is depreciated
        //m_fgStatOpt = STAT_OPT(1, TRUE, TRUE);

        m_rTsfEnvInfo.ImgWidth    = m_blockW;//pAeParam->strAEParasetting.u4AEWinodwNumX; //rAwbStatParma.i4WindowNumX;
        m_rTsfEnvInfo.ImgHeight   = m_blockH;//pAeParam->strAEParasetting.u4AEWinodwNumY; //rAwbStatParma.i4WindowNumY;
        m_rTsfEnvInfo.BayerOrder  = MTK_BAYER_B;
        m_rTsfEnvInfo.WorkingBufAddr = (MUINT32*)gWorkinBuffer;

        ILscNvram* pNvram = ILscNvram::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));

        // tsf tuning para/data
        const CAMERA_TSF_TBL_STRUCT* pTsfCfg = pNvram->getTsfNvram();
        m_rTsfEnvInfo.Para        = (MUINT32*)pTsfCfg->TSF_DATA;
        m_rTsfEnvInfo.pTuningPara = (MINT32*)pTsfCfg->TSF_PARA;
        m_rTsfEnvInfo.TS_TS       = 1;
        m_rTsfEnvInfo.MA_NUM      = 5;

        // golden/unit alignment
        m_rTsfEnvInfo.WithOTP = m_pLsc->getIsOtpOn();

        if (m_rTsfEnvInfo.WithOTP)
        {
            loadOtpDataForTsf();
        }
        else
        {
            LSC_LOG("No OTP Data");
            m_rTsfEnvInfo.afn = MTKTSF_AFN_R0D;
            m_rTsfEnvInfo.ShadingTbl.Golden.Tbl = NULL;
            m_rTsfEnvInfo.ShadingTbl.Unit.Tbl = NULL;
            m_u1OtpPixId = 0;
            m_u1OtpGridX = 0;
            m_u1OtpGridY = 0;
        }
        LSC_LOG("WithOTP(%d)", m_rTsfEnvInfo.WithOTP);

        // AWB NVRAM
        m_rTsfEnvInfo.EnableORCorrection = 0;

        const NVRAM_CAMERA_3A_STRUCT* pNvram3A = pNvram->get3ANvram();
        const AWB_ALGO_CAL_T& rAlgoCalParam = pNvram3A->AWB[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam;
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R   = rAlgoCalParam.rCalData.rUnitGain.i4R; // 512;
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G   = rAlgoCalParam.rCalData.rUnitGain.i4G; // 512;
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B   = rAlgoCalParam.rCalData.rUnitGain.i4B; // 512;
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R = rAlgoCalParam.rCalData.rGoldenGain.i4R; //512;
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G = rAlgoCalParam.rCalData.rGoldenGain.i4G; //512;
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B = rAlgoCalParam.rCalData.rGoldenGain.i4B; //512;
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R    = rAlgoCalParam.rCalData.rD65Gain.i4R; //809;
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G    = rAlgoCalParam.rCalData.rD65Gain.i4G; //512;
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B    = rAlgoCalParam.rCalData.rD65Gain.i4B; //608;
        LSC_LOG("AwbNvramInfo: UnitGain(%d, %d, %d), GoldenGain(%d, %d, %d), D65Gain(%d, %d, %d)",
            m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R  ,
            m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G  ,
            m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B  ,
            m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R,
            m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G,
            m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B,
            m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R   ,
            m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G   ,
            m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B   );

        m_bTSFInit = MFALSE;
    }

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);
}

