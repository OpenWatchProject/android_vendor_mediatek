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

#define LOG_TAG "lsc_mgr_dft"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include "LscMgrDefault.h"
#include <mtkcam/drv/IHalSensor.h>
#include <LscUtil.h>

#include "shading_tuning_custom.h"
#include "tsf_tuning_custom.h"

#include <ae_param.h>
#include <mtkcam/def/common.h>
#include <aaa_sensor_mgr.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <ResultPool4LSCConfig.h>

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NSIspTuning;
using namespace NSCam;

LscMgrDefault::
LscMgrDefault(ESensorDev_T eSensorDev, MINT32 i4SensorIdx)
    : m_eSensorDev(eSensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorID(0)
    , m_eSensorMode(ESensorMode_Preview)
    , m_ePrevSensorMode(ESensorMode_NUM)
    , m_u4NumSensorModes(0)
    , m_fgOnOff(MTRUE)
    , m_fgOnOffP2(MFALSE)
    , m_binOnOff(MFALSE)
    , m_fgInit(MFALSE)
    , m_fgDynamicShading(MTRUE)
    , m_fgSensorCropInfoNull(MFALSE)
    , m_fgBypassAlign(MFALSE)
    , m_eAlignFlag(E_LSC_ALIGN_USE_CCT)
    , m_eTrfmFlag(E_LSC_TRFM_OK)
    , m_u4CTIdx(0)
    , m_u4Rto(32)
    , m_u4BufIdx(0)
    , m_u4CurrentBufIdx(0)
    , m_u4BufIdxP2(0)
    , m_i4LogEn(0)
    , m_u4P1BinWidth(0)
    , m_u4P1BinHeight(0)
    , m_pNvramOtp(NULL)
    , m_pTsf(NULL)
    , m_Lock()
    , m_Thread_s(0)
    , m_hadInit(MFALSE)
{
    LSC_LOG("Enter LscMgrDefault: sensorDev(%d), sensorIdx(%d)", eSensorDev, i4SensorIdx);

    // Polling sensor power on
#define MAX_POLLING_COUNT 100

    AAA_TRACE_D("LSC EEPROM");

    MINT32 i = MAX_POLLING_COUNT;
    MUINT32 u4PowerOn = 0;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor(LOG_TAG, m_i4SensorIdx);
    //LSC_LOG("waiting for sensor(%d) power on", m_eSensorDev);
    pIHalSensor->sendCommand(m_eSensorDev, NSCam::SENSOR_CMD_GET_SENSOR_POWER_ON_STETE, (MUINTPTR)&u4PowerOn, 0, 0);
    while (0 == (m_eSensorDev & u4PowerOn) && i != 0)
    {
        usleep(1000 * 10);
        i --;
        pIHalSensor->sendCommand(m_eSensorDev, NSCam::SENSOR_CMD_GET_SENSOR_POWER_ON_STETE, (MUINTPTR)&u4PowerOn, 0, 0);
    }
    LSC_LOG("Sensor(%d) power on done, count(%d)", m_eSensorDev, (MAX_POLLING_COUNT - i));
    pIHalSensor->destroyInstance(LOG_TAG);

    AAA_TRACE_END_D;

    m_i4SensorID=getSensorId();

    // get nvram, and read otp after sensor power on
    m_pNvramOtp = ILscNvram::getInstance(eSensorDev);
}

LscMgrDefault::
~LscMgrDefault()
{
    m_pTsf->destroyInstance();
    LSC_LOG("Exit LscMgrDefault");
}

MBOOL
LscMgrDefault::
init()
{
    LSC_LOG_BEGIN("SensorDev(%d), byp123(%d)", m_eSensorDev, m_fgBypassAlign);
    MINT32 i4Type = 0;
    GET_PROP("vendor.debug.lsc_mgr.log", 0, m_i4LogEn);
    GET_PROP("vendor.debug.lsc_mgr.type", 0, i4Type);

    // create buffers
    MINT32 i = 0;
    for (i = 0; i < SHADING_SUPPORT_CT_NUM; i++)
    {
        char strName[32];
        sprintf(strName, "CT_%d", i);
        ILscBuf* pBuf = createBuf(strName, ILscBuf::E_LSC_IMEM);
        m_pLscBuf[i] = pBuf;
        if (!pBuf)
        {
            LSC_ERR("Fail to create buffer %s", strName);
        }
    }
    for (i = 0; i < RING_BUF_NUM; i++)
    {
        char strName[32];
        sprintf(strName, "RingBuf_%d", i);
        ILscBuf* pBuf = createBuf(strName, ILscBuf::E_LSC_IMEM);
        m_prLscBufRing[i] = pBuf;
        if (!pBuf)
        {
            LSC_ERR("Fail to create buffer %s", strName);
        }
    }
    for (i = 0; i < RING_BUF_NUM; i++)
    {
        char strName[32];
        sprintf(strName, "P2_%d", i);
        ILscBuf* pBuf = createBuf(strName, ILscBuf::E_LSC_IMGBUF);
        m_prLscBufP2[i] = pBuf;
        if (!pBuf)
        {
            LSC_ERR("Fail to create buffer %s", strName);
        }
    }

    // resolution and reset tables
    if (0 == m_u4NumSensorModes)
    {
        // get resolution at the 1st time
        LSC_LOG("The 1st time, getSensorResolution");
        getSensorResolution();
        resetLscTbl();
    }
    else
    {
        LSC_LOG("Not the 1st time, show info only");
        showResolutionInfo();
    }

    // transformation flow
    loadTableFlow(MFALSE);

    // TSF
    m_pTsf = ILscTsf::createInstance(m_eSensorDev, static_cast<ILscTsf::E_LSC_TSF_TYPE_T>(i4Type));

    m_fgInit = MFALSE;

    LSC_LOG_END();

    m_hadInit=MTRUE;

    return MTRUE;
}

MBOOL
LscMgrDefault::
uninit()
{
    LSC_LOG_BEGIN("SensorDev(%d)", m_eSensorDev);

    m_mutex_s.lock();
    for(int i=0;i<m_Thread_s.size();i++)
    {
        LSC_LOG("[%s] wait sending shading table thread %d for sensor +", __func__, m_Thread_s[i]);
        int s = pthread_join(m_Thread_s[i], NULL);
        m_Thread_s[i]=0;
        LSC_LOG("[%s] wait sending shading table thread %d for sensor -", __func__, m_Thread_s[i]);
        if( s != 0 )
        {
            LSC_ERR("[%s] pthread join error: %d", __func__, s);
            return false;
        }
    }
    m_Thread_s.clear();
    m_mutex_s.unlock();

    if(m_pTsf->isInit())
        m_pTsf->uninit();

    MINT32 i = 0;

    // destroy buffer pool.
    std::list<ILscBuf*>::iterator iter;
    for (iter = m_rBufPool.begin(); iter != m_rBufPool.end(); iter++)
    {
        if (*iter)
        {
            delete (*iter);
            *iter = NULL;
        }
    }
    m_rBufPool.clear();

    LSC_LOG_END("SensorDev(%d)", m_eSensorDev);

    m_hadInit=MFALSE;

    return MTRUE;
}

MBOOL
LscMgrDefault::
resetLscTbl()
{
    // load table from nvram to tbl obj
    MUINT32 i;
    MINT32 i4W = m_rSensorCropWin[ESensorMode_Capture].u4W;
    MINT32 i4H = m_rSensorCropWin[ESensorMode_Capture].u4H;
    MINT32 i4GridX = m_pNvramOtp->getLscNvram()->GridXNum;
    MINT32 i4GridY = m_pNvramOtp->getLscNvram()->GridYNum;

    LSC_LOG("Grid(%dx%d), WH(%dx%d)", i4GridX, i4GridY, i4W, i4H);

    for (i = 0; i < SHADING_SUPPORT_CT_NUM; i++)
    {
        ILscTbl& rTbl = m_rCapLscTbl[i];
        rTbl.setConfig(i4W, i4H, i4GridX, i4GridY);
        rTbl.setData(m_pNvramOtp->getLut(ESensorMode_Capture, i), rTbl.getSize());

        char dbgFile[256];
        sprintf(dbgFile, "/sdcard/shading/sensor_%d_cap_%d_%dx%d_%dx%d.log", m_eSensorDev, i, i4GridX, i4GridY, i4W, i4H);
        rTbl.dump(dbgFile);

        //sprintf(dbgFile, "/sdcard/shading/sensor_%d_cap_%d_gain.log", m_eSensorDev, i);
        //ILscTbl rToGain(ILscTable::GAIN_FLOAT);
        //rTbl.convert(rToGain);
        //rToGain.dump(dbgFile);
    }

    return MTRUE;
}

const ILscTbl*
LscMgrDefault::
getCapLut(MUINT32 u4CtIdx) const
{
    if (u4CtIdx < SHADING_SUPPORT_CT_NUM)
    {
        return &m_rCapLscTbl[u4CtIdx];
    }
    else
    {
        return NULL;
    }
}

const ILscTbl*
LscMgrDefault::
getLut(MUINT32 u4CtIdx) const
{
    if (u4CtIdx < SHADING_SUPPORT_CT_NUM)
    {
        return &m_rCurLscTbl[u4CtIdx];
    }
    else
    {
        return NULL;
    }
}

MBOOL
LscMgrDefault::
loadTableFlow(MBOOL fgForceRestore)
{
    MINT32 i4Opt123 = 0;

    GET_PROP("vendor.debug.lsc_mgr.opt123", 0, i4Opt123);

    LSC_LOG_BEGIN("SensorDev(%d), byp123(%d), i4Opt123(%d), force_restore(%d)", m_eSensorDev, m_fgBypassAlign, i4Opt123, fgForceRestore);

    if (i4Opt123 & 0x1 || fgForceRestore || m_fgBypassAlign)
    {
        resetLscTbl();
        m_eAlignFlag = E_LSC_ALIGN_USE_CCT;
    }

    MBOOL fgBypass123 = (i4Opt123 & 0x2) ? MTRUE : MFALSE;
    if (m_eAlignFlag != E_LSC_ALIGN_MTK_OTP_ALIGN_OK)
    {
        if (fgBypass123 || m_fgBypassAlign)
        {
            LSC_LOG("LSC alignment is bypassed for debug purpose");
            m_eAlignFlag = E_LSC_ALIGN_USE_CCT;
        }
        else
        {
            switch (m_pNvramOtp->getOtpState())
            {
            case ILscNvram::E_LSC_NO_OTP:
                m_eAlignFlag = E_LSC_ALIGN_NO_OTP;
                #if 0
                LSC_LOG("No OTP: Do 1-to-3 from capture.");
                // new path: only calibration in capture mode, and transform to the others.
                if (doShadingTrfm())
                {
                    //copyRawLscToNvram(CAL_DATA_LOAD+1);
                    m_e1to3Flag = E_LSC_123_NO_OTP_OK;
                }
                else
                {
                    m_e1to3Flag = E_LSC_123_NO_OTP_ERR;
                }
                #endif
                break;
            case ILscNvram::E_LSC_WITH_MTK_OTP:
                LSC_LOG("MTK OTP: Do shading align");
                if (doShadingAlign())
                {
                    LSC_LOG("MTK OTP: Do shading align ok");
                    m_eAlignFlag = E_LSC_ALIGN_MTK_OTP_ALIGN_OK;
                }
                else
                {
                    LSC_LOG("MTK OTP: Do shading align fail");
                    m_eAlignFlag = E_LSC_ALIGN_MTK_OTP_ALIGN_ERR;
                }
                break;
            default:
            case ILscNvram::E_LSC_OTP_ERROR:
                m_eAlignFlag = E_LSC_ALIGN_OTP_ERR;
                break;
            }

            //setSensorTblFlow();
            m_mutex_s.lock();
            pthread_t threadID;
            ::pthread_create(&threadID, NULL, threadSendSensor, this);
            m_Thread_s.push_back(threadID);
            m_mutex_s.unlock();
        }
    }
    else
    {
        LSC_LOG("Aligned Tables already exist in LscMgrDefault (0x%08x)", m_eAlignFlag);

        //setSensorTblFlow();
        m_mutex_s.lock();
        pthread_t threadID;
        ::pthread_create(&threadID, NULL, threadSendSensor, this);
        m_Thread_s.push_back(threadID);
        m_mutex_s.unlock();
    }

    LSC_LOG_END("(%d)", m_eAlignFlag);

    return MTRUE;
}

MBOOL
LscMgrDefault::
getSensorResolution()
{
    IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();

    SensorStaticInfo rSensorStaticInfo;
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("lsc_mgr", m_i4SensorIdx);
    pIHalSensorList->querySensorStaticInfo(m_eSensorDev, &rSensorStaticInfo);

#ifdef USING_MTK_LDVT
    MINT32 u4NumSensorModes = 2;
#else
    MINT32 u4NumSensorModes = MIN(rSensorStaticInfo.SensorModeNum, (MINT32)ESensorMode_NUM);
#endif
    m_rSensorResolution.u4SensorPreviewWidth  = rSensorStaticInfo.previewWidth;
    m_rSensorResolution.u4SensorPreviewHeight = rSensorStaticInfo.previewHeight;
    m_rSensorResolution.u4SensorCaptureWidth  = rSensorStaticInfo.captureWidth;
    m_rSensorResolution.u4SensorCaptureHeight = rSensorStaticInfo.captureHeight;
    m_rSensorResolution.u4SensorVideoWidth    = rSensorStaticInfo.videoWidth;
    m_rSensorResolution.u4SensorVideoHeight   = rSensorStaticInfo.videoHeight;
    m_rSensorResolution.u4SensorVideo1Width   = rSensorStaticInfo.video1Width;
    m_rSensorResolution.u4SensorVideo1Height  = rSensorStaticInfo.video1Height;
    m_rSensorResolution.u4SensorVideo2Width   = rSensorStaticInfo.video2Width;
    m_rSensorResolution.u4SensorVideo2Height  = rSensorStaticInfo.video2Height;
    m_rSensorResolution.u4SensorCustom1Width  = rSensorStaticInfo.SensorCustom1Width;
    m_rSensorResolution.u4SensorCustom1Height = rSensorStaticInfo.SensorCustom1Height;
    m_rSensorResolution.u4SensorCustom2Width  = rSensorStaticInfo.SensorCustom2Width;
    m_rSensorResolution.u4SensorCustom2Height = rSensorStaticInfo.SensorCustom2Height;
    m_rSensorResolution.u4SensorCustom3Width  = rSensorStaticInfo.SensorCustom3Width;
    m_rSensorResolution.u4SensorCustom3Height = rSensorStaticInfo.SensorCustom3Height;
    m_rSensorResolution.u4SensorCustom4Width  = rSensorStaticInfo.SensorCustom4Width;
    m_rSensorResolution.u4SensorCustom4Height = rSensorStaticInfo.SensorCustom4Height;
    m_rSensorResolution.u4SensorCustom5Width  = rSensorStaticInfo.SensorCustom5Width;
    m_rSensorResolution.u4SensorCustom5Height = rSensorStaticInfo.SensorCustom5Height;

    LSC_LOG("u4NumSensorModes +++ (%d)", u4NumSensorModes);
    LSC_LOG("i4SensorPreviewWidth (%d)", m_rSensorResolution.u4SensorPreviewWidth );
    LSC_LOG("i4SensorPreviewHeight(%d)", m_rSensorResolution.u4SensorPreviewHeight);
    LSC_LOG("i4SensorCaptureWidth (%d)", m_rSensorResolution.u4SensorCaptureWidth );
    LSC_LOG("i4SensorCaptureHeight(%d)", m_rSensorResolution.u4SensorCaptureHeight);
    LSC_LOG("i4SensorVideoWidth   (%d)", m_rSensorResolution.u4SensorVideoWidth   );
    LSC_LOG("i4SensorVideoHeight  (%d)", m_rSensorResolution.u4SensorVideoHeight  );
    LSC_LOG("i4SensorVideo1Width  (%d)", m_rSensorResolution.u4SensorVideo1Width  );
    LSC_LOG("i4SensorVideo1Height (%d)", m_rSensorResolution.u4SensorVideo1Height );
    LSC_LOG("i4SensorVideo2Width  (%d)", m_rSensorResolution.u4SensorVideo2Width  );
    LSC_LOG("i4SensorVideo2Height (%d)", m_rSensorResolution.u4SensorVideo2Height );
    LSC_LOG("i4SensorCustom1Width (%d)", m_rSensorResolution.u4SensorCustom1Width );
    LSC_LOG("i4SensorCustom1Height(%d)", m_rSensorResolution.u4SensorCustom1Height);
    LSC_LOG("i4SensorCustom2Width (%d)", m_rSensorResolution.u4SensorCustom2Width );
    LSC_LOG("i4SensorCustom2Height(%d)", m_rSensorResolution.u4SensorCustom2Height);
    LSC_LOG("i4SensorCustom3Width (%d)", m_rSensorResolution.u4SensorCustom3Width );
    LSC_LOG("i4SensorCustom3Height(%d)", m_rSensorResolution.u4SensorCustom3Height);
    LSC_LOG("i4SensorCustom4Width (%d)", m_rSensorResolution.u4SensorCustom4Width );
    LSC_LOG("i4SensorCustom4Height(%d)", m_rSensorResolution.u4SensorCustom4Height);
    LSC_LOG("i4SensorCustom5Width (%d)", m_rSensorResolution.u4SensorCustom5Width );
    LSC_LOG("i4SensorCustom5Height(%d)", m_rSensorResolution.u4SensorCustom5Height);

    MINT32 i;
    for (i = 0; i < u4NumSensorModes; i++)
    {
        MUINT32 scenario = i;
        SensorCropWinInfo rSensorCropInfo;

        ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));

        MINT32 err = pIHalSensor->sendCommand(m_eSensorDev,
                SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                (MUINTPTR)&scenario,
                (MUINTPTR)&rSensorCropInfo,
                0);

        if (err != 0 || rSensorCropInfo.full_w == 0)
        {
            LSC_ERR("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO is not implemented well (%d), use predefined rule.", err);
            m_fgSensorCropInfoNull = MTRUE;
            break;
        }

        SensorCropInfo_T rCrop;
        rCrop.w0 = rSensorCropInfo.full_w;
        rCrop.h0 = rSensorCropInfo.full_h;
        rCrop.x1 = rSensorCropInfo.x0_offset;
        rCrop.y1 = rSensorCropInfo.y0_offset;
        rCrop.w1 = rSensorCropInfo.w0_size;
        rCrop.h1 = rSensorCropInfo.h0_size;
        rCrop.w1r= rSensorCropInfo.scale_w;
        rCrop.h1r= rSensorCropInfo.scale_h;
        rCrop.x2 = rSensorCropInfo.x1_offset + rSensorCropInfo.x2_tg_offset;
        rCrop.y2 = rSensorCropInfo.y1_offset + rSensorCropInfo.y2_tg_offset;
        rCrop.w2 = rSensorCropInfo.w2_tg_size;
        rCrop.h2 = rSensorCropInfo.h2_tg_size;
        rCrop.u4W= rSensorCropInfo.w2_tg_size;
        rCrop.u4H= rSensorCropInfo.h2_tg_size;
        LSC_LOG("SensorMode(%d), full_0(%d,%d) crop1(%d,%d,%d,%d) resize(%d,%d) crop2(%d,%d,%d,%d) final size(%d,%d)",
            scenario, rCrop.w0, rCrop.h0, rCrop.x1, rCrop.y1, rCrop.w1, rCrop.h1, rCrop.w1r, rCrop.h1r,
            rCrop.x2, rCrop.y2, rCrop.w2, rCrop.h2, rCrop.u4W, rCrop.u4H);
        m_rSensorCropWin[i] = rCrop;
    }

    pIHalSensor->destroyInstance("lsc_mgr");

    m_rSensorCropWin[ESensorMode_Preview].u4W    = rSensorStaticInfo.previewWidth;
    m_rSensorCropWin[ESensorMode_Preview].u4H    = rSensorStaticInfo.previewHeight;
    m_rSensorCropWin[ESensorMode_Capture].u4W    = rSensorStaticInfo.captureWidth;
    m_rSensorCropWin[ESensorMode_Capture].u4H    = rSensorStaticInfo.captureHeight;
    m_rSensorCropWin[ESensorMode_Video].u4W      = rSensorStaticInfo.videoWidth;
    m_rSensorCropWin[ESensorMode_Video].u4H      = rSensorStaticInfo.videoHeight;
    m_rSensorCropWin[ESensorMode_SlimVideo1].u4W = rSensorStaticInfo.video1Width;
    m_rSensorCropWin[ESensorMode_SlimVideo1].u4H = rSensorStaticInfo.video1Height;
    m_rSensorCropWin[ESensorMode_SlimVideo2].u4W = rSensorStaticInfo.video2Width;
    m_rSensorCropWin[ESensorMode_SlimVideo2].u4H = rSensorStaticInfo.video2Height;
    m_rSensorCropWin[ESensorMode_Custom1].u4W    = rSensorStaticInfo.SensorCustom1Width;
    m_rSensorCropWin[ESensorMode_Custom1].u4H    = rSensorStaticInfo.SensorCustom1Height;
    m_rSensorCropWin[ESensorMode_Custom2].u4W    = rSensorStaticInfo.SensorCustom2Width;
    m_rSensorCropWin[ESensorMode_Custom2].u4H    = rSensorStaticInfo.SensorCustom2Height;
    m_rSensorCropWin[ESensorMode_Custom3].u4W    = rSensorStaticInfo.SensorCustom3Width;
    m_rSensorCropWin[ESensorMode_Custom3].u4H    = rSensorStaticInfo.SensorCustom3Height;
    m_rSensorCropWin[ESensorMode_Custom4].u4W    = rSensorStaticInfo.SensorCustom4Width;
    m_rSensorCropWin[ESensorMode_Custom4].u4H    = rSensorStaticInfo.SensorCustom4Height;
    m_rSensorCropWin[ESensorMode_Custom5].u4W    = rSensorStaticInfo.SensorCustom5Width;
    m_rSensorCropWin[ESensorMode_Custom5].u4H    = rSensorStaticInfo.SensorCustom5Height;

    //Check Rule #3
    if(m_rSensorCropWin[ESensorMode_Capture].w1 !=  m_rSensorCropWin[ESensorMode_Capture].w1r || m_rSensorCropWin[ESensorMode_Capture].h1!=  m_rSensorCropWin[ESensorMode_Capture].h1r)
        LSC_ERR("Capture sensor mode applied resize");

    MUINT32 u4GridX = m_pNvramOtp->getLscNvram()->GridXNum;
    MUINT32 u4GridY = m_pNvramOtp->getLscNvram()->GridYNum;
    for (i = 0; i < u4NumSensorModes; i++)
    {
        //Check Rule #1
        if(m_rSensorCropWin[i].w0 !=  m_rSensorCropWin[ESensorMode_Capture].w0 || m_rSensorCropWin[i].h0 !=  m_rSensorCropWin[ESensorMode_Capture].h0)
            LSC_ERR("Original size (full_0) of sensor mode(%d) is not equal to original size (full_0) of capture sensor mode", i);
        //Check Rule #2
        if(m_rSensorCropWin[i].w2 !=  m_rSensorCropWin[i].u4W || m_rSensorCropWin[i].h2 !=  m_rSensorCropWin[i].u4H)
            LSC_ERR("crop2 size of sensor mode (%d) is not equal to its final size", i);
        //Check Rule #4
        if(m_rSensorCropWin[i].w2  % 2 !=0 ||
           m_rSensorCropWin[i].h2  % 2 !=0 ||
           m_rSensorCropWin[i].w1r % 2 !=0 ||
           m_rSensorCropWin[i].h1r % 2 !=0 )
            LSC_ERR("resize or final size of sensor mode (%d) is not even number", i);

        convertSensorCrop(!m_fgSensorCropInfoNull, m_rSensorCropWin[ESensorMode_Capture], m_rSensorCropWin[i], m_rTransformCfg[i]);
        m_rTransformCfg[i].u4GridX = u4GridX;
        m_rTransformCfg[i].u4GridY = u4GridY;
    }

    //init dynamic bin size
    ILscTbl::Config rawConfig=ILscTbl::makeConfig( m_rSensorCropWin[ESensorMode_Preview].u4W, m_rSensorCropWin[ESensorMode_Preview].u4H, u4GridX, u4GridY);
    ILscTbl::Config binConfig=ILscTbl::makeConfig( m_rSensorCropWin[ESensorMode_Preview].u4W/2, m_rSensorCropWin[ESensorMode_Preview].u4H/2, u4GridX, u4GridY);

    LSC_LOG("[Generate bin size after getSensorResolution] Raw size ( %d x %d), Bin size ( %d x %d )",
        m_rSensorCropWin[ESensorMode_Preview].u4W, m_rSensorCropWin[ESensorMode_Preview].u4H,
        m_rSensorCropWin[ESensorMode_Preview].u4W/2, m_rSensorCropWin[ESensorMode_Preview].u4H/2);

    m_u4NumSensorModes = u4NumSensorModes;

    return MTRUE;
}

MBOOL
LscMgrDefault::
convertSensorCrop(MBOOL fgWithSensorCropInfo, const SensorCropInfo_T& rFullInfo, const SensorCropInfo_T& rCropInfo, ILscTable::TransformCfg_T& rCropCfg)
{
    if (!fgWithSensorCropInfo)
    {
        MFLOAT fScale = 1.0f;

        // fit horizontal
        if (rFullInfo.u4W < rFullInfo.u4H || rCropInfo.u4W < rCropInfo.u4H)
        {
            LSC_ERR("Impossible Case.");
        }

        LSC_LOG("No sensor crop info, use magic rule");
        fScale = (MFLOAT) rCropInfo.u4W / rFullInfo.u4W;
        if (rCropInfo.u4W*10 > rFullInfo.u4W*7)
        {
            // crop case
            LSC_LOG("Crop case, width scale(%3.3f)", fScale);
            rCropCfg.u4ResizeW  = rFullInfo.u4W;
            rCropCfg.u4ResizeH  = rFullInfo.u4H;
            rCropCfg.u4W        = rCropInfo.u4W;
            rCropCfg.u4H        = rCropInfo.u4H;
            rCropCfg.u4X        = (rCropCfg.u4ResizeW >= rCropCfg.u4W) ? ((rCropCfg.u4ResizeW - rCropCfg.u4W) / 2) : 0;
            rCropCfg.u4Y        = (rCropCfg.u4ResizeH >= rCropCfg.u4H) ? ((rCropCfg.u4ResizeH - rCropCfg.u4H) / 2) : 0;
        }
        else
        {
            // resize case
            MFLOAT fScale_Height = (MFLOAT) rCropInfo.u4H / rFullInfo.u4H;
            if (fScale_Height > fScale)
                fScale = fScale_Height;
            LSC_LOG("Resize case, scale(%3.3f)", fScale);
            rCropCfg.u4ResizeW = rFullInfo.u4W*fScale;
            rCropCfg.u4ResizeH = rFullInfo.u4H*fScale;
            rCropCfg.u4W       = rCropInfo.u4W;
            rCropCfg.u4H       = rCropInfo.u4H;
            rCropCfg.u4X       = (rCropCfg.u4ResizeW >= rCropCfg.u4W) ? ((rCropCfg.u4ResizeW - rCropCfg.u4W) / 2) : 0;
            rCropCfg.u4Y       = (rCropCfg.u4ResizeH >= rCropCfg.u4H) ? ((rCropCfg.u4ResizeH - rCropCfg.u4H) / 2) : 0;
        }
    }
    else
    {
        // calculate crop with respect to capture
        // rw = w1r/w1, rh = h1r/h1,
        // x2'= x1*r+x2
        // x2_cap'' = x2c' * rw_scn / rw_cap
        MFLOAT rwc = (MFLOAT) rFullInfo.w1r / rFullInfo.w1;
        MFLOAT rhc = (MFLOAT) rFullInfo.h1r / rFullInfo.h1;
        MFLOAT x2c_ = (MFLOAT) rFullInfo.x1*rwc + rFullInfo.x2;
        MFLOAT y2c_ = (MFLOAT) rFullInfo.y1*rhc + rFullInfo.y2;
        LSC_LOG("rwc(%3.3f), rhc(%3.3f), x2c_(%3.3f), y2c_(%3.3f)", rwc, rhc, x2c_, y2c_);

        MFLOAT rws = (MFLOAT) rCropInfo.w1r / rCropInfo.w1;
        MFLOAT rhs = (MFLOAT) rCropInfo.h1r / rCropInfo.h1;
        MFLOAT x2s_ = (MFLOAT) rCropInfo.x1*rws + rCropInfo.x2;
        MFLOAT y2s_ = (MFLOAT) rCropInfo.y1*rhs + rCropInfo.y2;
        LSC_LOG("rws(%3.3f), rhs(%3.3f), x2s_(%3.3f), y2s_(%3.3f)", rws, rhs, x2s_, y2s_);

        MFLOAT x2c__ = x2c_ * rws / rwc;
        MFLOAT y2c__ = y2c_ * rhs / rhc;

        MFLOAT x_sc = x2s_ - x2c__;
        MFLOAT y_sc = y2s_ - y2c__;
        MFLOAT out_w = (MFLOAT) rFullInfo.w2 * rws / rwc;
        MFLOAT out_h = (MFLOAT) rFullInfo.h2 * rhs / rhc;
        MFLOAT w2s = rCropInfo.w2;
        MFLOAT h2s = rCropInfo.h2;

        LSC_LOG("x2c__(%3.3f), y2c__(%3.3f), x_sc(%3.3f), y_sc(%3.3f), out_w(%3.3f), out_h(%3.3f)",
            x2c__, y2c__, x_sc, y_sc, out_w, out_h);

        if (x_sc < 0.0f)
        {
            LSC_LOG("Negative x_sc(%3.3f)", x_sc);
            x_sc = 0.0f;
        }

        if ((MFLOAT)x_sc + w2s > out_w)
        {
            LSC_LOG("Exceed right side, x_sc(%3.3f), w2s(%3.3f), out_w(%3.3f)", x_sc, w2s, out_w);
            //w2s = out_w - x_sc;
            x_sc = 0.0f;
            w2s = out_w;
        }

        if (y_sc < 0.0f)
        {
            LSC_LOG("Negative y_sc(%3.3f)", y_sc);
            y_sc = 0.0f;
        }

        if ((MFLOAT)y_sc + h2s > out_h)
        {
            LSC_LOG("Exceed bottom side, y_sc(%3.3f), h2s(%3.3f), out_h(%3.3f)", y_sc, h2s, out_h);
            //h2s = out_h - y_sc;
            y_sc = 0.0f;
            h2s = out_h;
        }

        rCropCfg.u4ResizeW = ROUND(out_w);
        rCropCfg.u4ResizeH = ROUND(out_h);
        rCropCfg.u4W       = ROUND(w2s);
        rCropCfg.u4H       = ROUND(h2s);
        rCropCfg.u4X       = x_sc;
        rCropCfg.u4Y       = y_sc;
    }

    LSC_LOG("Crop CFG for lib ImgWH(%d,%d), outWH(%d,%d), Crop(%d,%d,%d,%d)",
        rFullInfo.w2, rFullInfo.h2, rCropCfg.u4ResizeW, rCropCfg.u4ResizeH,
        rCropCfg.u4X, rCropCfg.u4Y, rCropCfg.u4W, rCropCfg.u4H);

    return MTRUE;
}

MBOOL
LscMgrDefault::
showResolutionInfo()
{
    UINT32 i, u4NumSensorModes;
    u4NumSensorModes = m_u4NumSensorModes;

    LSC_LOG("u4NumSensorModes(%d)", u4NumSensorModes);
    LSC_LOG("i4SensorPreviewWidth (%d)", m_rSensorResolution.u4SensorPreviewWidth);
    LSC_LOG("i4SensorPreviewHeight(%d)", m_rSensorResolution.u4SensorPreviewHeight);
    LSC_LOG("i4SensorCaptureWidth (%d)", m_rSensorResolution.u4SensorCaptureWidth);
    LSC_LOG("i4SensorCaptureHeight(%d)", m_rSensorResolution.u4SensorCaptureHeight);
    LSC_LOG("i4SensorVideoWidth   (%d)", m_rSensorResolution.u4SensorVideoWidth);
    LSC_LOG("i4SensorVideoHeight  (%d)", m_rSensorResolution.u4SensorVideoHeight);
    LSC_LOG("i4SensorVideo1Width  (%d)", m_rSensorResolution.u4SensorVideo1Width);
    LSC_LOG("i4SensorVideo1Height (%d)", m_rSensorResolution.u4SensorVideo1Height);
    LSC_LOG("i4SensorVideo2Width  (%d)", m_rSensorResolution.u4SensorVideo2Width);
    LSC_LOG("i4SensorVideo2Height (%d)", m_rSensorResolution.u4SensorVideo2Height);
    LSC_LOG("i4SensorCustom1Width (%d)", m_rSensorResolution.u4SensorCustom1Width );
    LSC_LOG("i4SensorCustom1Height(%d)", m_rSensorResolution.u4SensorCustom1Height);
    LSC_LOG("i4SensorCustom2Width (%d)", m_rSensorResolution.u4SensorCustom2Width );
    LSC_LOG("i4SensorCustom2Height(%d)", m_rSensorResolution.u4SensorCustom2Height);
    LSC_LOG("i4SensorCustom3Width (%d)", m_rSensorResolution.u4SensorCustom3Width );
    LSC_LOG("i4SensorCustom3Height(%d)", m_rSensorResolution.u4SensorCustom3Height);
    LSC_LOG("i4SensorCustom4Width (%d)", m_rSensorResolution.u4SensorCustom4Width );
    LSC_LOG("i4SensorCustom4Height(%d)", m_rSensorResolution.u4SensorCustom4Height);
    LSC_LOG("i4SensorCustom5Width (%d)", m_rSensorResolution.u4SensorCustom5Width );
    LSC_LOG("i4SensorCustom5Height(%d)", m_rSensorResolution.u4SensorCustom5Height);

    if (!m_fgSensorCropInfoNull)
    {
        for (i = 0; i < u4NumSensorModes; i++)
        {
            const SensorCropInfo_T& rCrop = m_rSensorCropWin[i];
            LSC_LOG("SensorMode(%d), full_0(%d,%d) crop1(%d,%d,%d,%d) resize(%d,%d) crop2(%d,%d,%d,%d) final size(%d,%d)",
                i, rCrop.w0, rCrop.h0, rCrop.x1, rCrop.y1, rCrop.w1, rCrop.h1, rCrop.w1r, rCrop.h1r,
                rCrop.x2, rCrop.y2, rCrop.w2, rCrop.h2, rCrop.u4W, rCrop.u4H);
        }
    }

    for (i = 0; i < u4NumSensorModes; i++)
    {
        const ILscTable::TransformCfg_T& rCropCfg = m_rTransformCfg[i];
        LSC_LOG("Crop CFG for lib ImgWH(%d,%d), outWH(%d,%d), Crop(%d,%d,%d,%d)",
            m_rSensorCropWin[ESensorMode_Capture].w2, m_rSensorCropWin[ESensorMode_Capture].h2, rCropCfg.u4ResizeW, rCropCfg.u4ResizeH,
        rCropCfg.u4X, rCropCfg.u4Y, rCropCfg.u4W, rCropCfg.u4H);
    }


    return MTRUE;
}

MBOOL
LscMgrDefault::
getResolution(ESensorMode_T eSensorMode, SensorCropInfo_T& rInfo) const
{
    if (eSensorMode >= ESensorMode_NUM)
    {
        LSC_ERR("Sensor Mode(%d) doesn't exist!", eSensorMode);
        rInfo = m_rSensorCropWin[ESensorMode_Preview];
        return MFALSE;
    }
    rInfo = m_rSensorCropWin[eSensorMode];
    //LSC_LOG("eSensorMode(%d), W(%d), H(%d)", eSensorMode, rInfo.u4W, rInfo.u4H);
    return MTRUE;
}

MBOOL
LscMgrDefault::
getResolution(ELscScenario_T eScn, SensorCropInfo_T& rInfo) const
{
    if (eScn >= LSC_SCENARIO_NUM)
    {
        LSC_ERR("eScn(%d) >= %d", eScn, LSC_SCENARIO_NUM);
        eScn = LSC_SCENARIO_PRV;
    }
    rInfo = m_rSensorCropWin[eScn];
    //LSC_LOG("eScn(%d), W(%d), H(%d)", eScn, rInfo.u4W, rInfo.u4H);
    return MTRUE;
}

MBOOL
LscMgrDefault::
getP1BinSize(MUINT32& u4OutW, MUINT32& u4OutH) const
{
    u4OutW = m_u4P1BinWidth;
    u4OutH = m_u4P1BinHeight;
    return MTRUE;
}

MBOOL
LscMgrDefault::
getRawSize(ESensorMode_T eSensorMode, MUINT32& u4OutW, MUINT32& u4OutH) const
{
    SensorCropInfo_T rCropInfo;
    if (getResolution(eSensorMode, rCropInfo))
    {
        u4OutW = rCropInfo.u4W;
        u4OutH = rCropInfo.u4H;
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
LscMgrDefault::
calSl2Config(MUINT32 u4Wd, MUINT32 u4Ht, SL2_CFG_T& rSl2Cfg) const
{
    // default setting
    MUINT32 u4Rx, u4Ry, u4R;
    u4Rx = u4Wd / 2;
    u4Ry = u4Ht / 2;
    u4R = ((u4Rx + u4Ry) *1448) >> 11;

    rSl2Cfg.i4CenterX = u4Rx;
    rSl2Cfg.i4CenterY = u4Ry;
    rSl2Cfg.i4R0 = u4R*3/10;
    rSl2Cfg.i4R1 = u4R*6/10;
    rSl2Cfg.i4R2 = u4R*8/10;

    //SLK gain default value
    rSl2Cfg.i4Gain0=0;
    rSl2Cfg.i4Gain1=64;
    rSl2Cfg.i4Gain2=128;
    rSl2Cfg.i4Gain3=192;
    rSl2Cfg.i4Gain4=255;
    rSl2Cfg.i4SetZero = 0;

    return MTRUE;
}

#if 0
ILscMgr::ELscScenario_T
LscMgrDefault::
getLscScenario(ESensorMode_T eSensorMode)
{
    if (eSensorMode >= ESensorMode_NUM)
    {
        LSC_ERR("eSensorMode(%d) >= %d", eSensorMode, ESensorMode_NUM);
        eSensorMode = ESensorMode_Preview;
    }

    return static_cast<ILscMgr::ELscScenario_T>(eSensorMode);
}
#endif

MBOOL
LscMgrDefault::
doShadingAlign()
{
    LSC_LOG_BEGIN("sensorDev(%d): MTK 1-to-3 Unit/Golden Align", m_eSensorDev);

    MBOOL fgRet = MTRUE;
    MINT32 j = 0;
    const ILscTbl* pGolden = m_pNvramOtp->getGolden();
    const ILscTbl* pUnit = m_pNvramOtp->getUnit();
    MUINT8 u1Rot = m_pNvramOtp->getOtpData()->TableRotation;

    for (j = 0; j < SHADING_SUPPORT_CT_NUM; j++)
    {
        MINT32 i4Ret = 0;
        ILscTbl rInput(m_rCapLscTbl[j]); // make a copy in case of error
        i4Ret = ILscTbl::shadingAlign(*pGolden, *pUnit, rInput, m_rCapLscTbl[j], u1Rot);
        if (0 != i4Ret)
        {
            // fgRet = MFALSE;
            LSC_ERR("Fail to do shading align (%d), ct(%d), restore table", i4Ret, j);
            m_rCapLscTbl[j] = rInput;
        }
        else
        {
            // debug dump file
            char dbgFile[256];
            ILscTable::Config rCfg = m_rCapLscTbl[j].getConfig();
            sprintf(dbgFile, "/sdcard/shading/sensor_%d_align_cap_%d_%dx%d_%dx%d.log", m_eSensorDev, j, rCfg.i4GridX, rCfg.i4GridY, rCfg.i4ImgWd, rCfg.i4ImgHt);
            if (do_mkdir("/sdcard/shading", S_IRWXU | S_IRWXG | S_IRWXO))
                m_rCapLscTbl[j].dump(dbgFile);
        }
    }

    LSC_LOG_END("ret(%d), rot(%d)", fgRet, u1Rot);

    return fgRet;
}
#if 0
MBOOL
LscMgrDefault::
doShadingTrfm()
{
    LSC_LOG_BEGIN("MTK 1-to-3 Crop Transform");

    ELscScenario_T eLscScn;
    for (MINT32 i4Scn = 0; i4Scn < (MINT32)m_u4NumSensorModes; i4Scn++)
    {
        eLscScn = (ELscScenario_T) i4Scn;
        if (eLscScn == LSC_SCENARIO_CAP)
            continue;

        const ILscTable::TransformCfg_T& rTrfm = m_rTransformCfg[i4Scn];
        for (MUINT32 j = 0; j < SHADING_SUPPORT_CT_NUM; j++)
        {
            ILscTbl& rCapTbl = m_rLscTbl[LSC_SCENARIO_CAP][j];
            ILscTbl& rTarget = m_rLscTbl[i4Scn][j];
            ILscTbl rTmpTbl = m_rLscTbl[i4Scn][j];  // make a copy in case of error

            LSC_LOG("Input (%d x %d) -> Output (%d x %d) -> X(%d), Y(%d), (%d x %d)",
            rCapTbl.getConfig().i4ImgWd, rCapTbl.getConfig().i4ImgHt, rTrfm.u4ResizeW, rTrfm.u4ResizeH,
            rTrfm.u4X, rTrfm.u4Y, rTrfm.u4W, rTrfm.u4H);

            MBOOL fgSuccess = rCapTbl.cropOut(rTrfm, rTarget);
            if (!fgSuccess)
            {
                LSC_ERR("Fail to do shading transform eScn(%d) ct(%d), restore table", i4Scn, j);
                rTarget = rTmpTbl;
            }
            else
            {
                // debug dump file
                char dbgFile[256];
                sprintf(dbgFile, "/sdcard/shading/sensor_%d_trfm_%d_%d.log", m_eSensorDev, i4Scn, j);
                rTarget.dump(dbgFile);
            }
        }
    }

    LSC_LOG_END();

    return MTRUE;
}
#else
MBOOL
LscMgrDefault::
doShadingTrfm()
{
    LSC_LOG_BEGIN("MTK 1-to-3 Crop Transform");

    MINT32 iFailCnt = 0;
    MINT32 i4Scn = getSensorMode();
    const ILscTable::TransformCfg_T& rTrfm = m_rCurTrfmCfg;
    for (MUINT32 j = 0; j < SHADING_SUPPORT_CT_NUM; j++)
    {
        const ILscTbl& rCapTbl = m_rCapLscTbl[j];   // cap table after alignment
        ILscTbl& rTarget = m_rCurLscTbl[j];

        LSC_LOG("Input (%d x %d) -> Output (%d x %d) -> Grid(%dx%d), Crop(%d,%d,%d,%d)",
            rCapTbl.getConfig().i4ImgWd, rCapTbl.getConfig().i4ImgHt, rTrfm.u4ResizeW, rTrfm.u4ResizeH,
            rTrfm.u4GridX, rTrfm.u4GridY, rTrfm.u4X, rTrfm.u4Y, rTrfm.u4W, rTrfm.u4H);

        MBOOL fgSuccess = rCapTbl.cropOut(rTrfm, rTarget);
        if (!fgSuccess)
        {
            LSC_ERR("Fail to do shading transform eScn(%d) ct(%d), restore table", i4Scn, j);
            rTarget = rCapTbl;
            rTarget.setConfig(rTrfm.u4W, rTrfm.u4H, rTrfm.u4GridX, rTrfm.u4GridY);
            iFailCnt ++;
        }
        else
        {
            if (m_i4LogEn & EN_LSC_LOG_TRFM_DUMP)
            {
                // debug dump file
                char dbgFile[256];
                sprintf(dbgFile, "/sdcard/shading/sensor_%d_trfm_%d_%d.log", m_eSensorDev, i4Scn, j);
                rTarget.dump(dbgFile);
            }
        }
    }

    m_eTrfmFlag = iFailCnt != 0 ? E_LSC_TRFM_ERR : E_LSC_TRFM_OK;
    LSC_LOG_END("flg(%d), failcnt(%d)", m_eTrfmFlag, iFailCnt);
    return MTRUE;
}
#endif

MBOOL
LscMgrDefault::
doShadingToSensorGain(MUINT32 u4GridX, MUINT32 u4GridY, std::vector<MUINT16*> &vecGain)
{
    LSC_LOG_BEGIN("[%s] MTK crop to sensor gain", __func__);

    ILscTbl output(ILscTable::GAIN_FIXED);

    MBOOL fgRet = MFALSE;

    MUINT32 width = m_rCapLscTbl[2].getConfig().i4ImgWd;
    MUINT32 height = m_rCapLscTbl[2].getConfig().i4ImgHt;
    if (m_rCapLscTbl[2].cropToGain(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, 0, 0, width, height),
        output, vecGain, 1))
        fgRet = MTRUE;
    else
        LSC_ERR("Fail to crop matching LSC Gain Table.");

    LSC_LOG_END();

    return MTRUE;
}


MBOOL
LscMgrDefault::
setSensorMode(ESensorMode_T eSensorMode, MUINT32 u4Width, MUINT32 u4Height, MBOOL fgForce)
{
    if(eSensorMode >= ESensorMode_NUM)
    {
        LSC_ERR("Wrong sensor config, sensor Mode(%d) doesn't exist!", eSensorMode);
        return MFALSE;
    }

    if(!m_hadInit)
        LSC_ERR("[%s] config before init", __func__);

    ESensorMode_T ePrevSensorMode;

    ePrevSensorMode = m_eSensorMode;

    if (eSensorMode != ePrevSensorMode ||
        m_fgInit == MFALSE ||
        m_u4P1BinWidth != u4Width ||
        m_u4P1BinHeight != u4Height ||
        fgForce)
    {
        // update config
        LSC_LOG("update config for eLscScn(%d -> %d): m_fgInit(%d), fgForce(%d), P1BinWxH(%dx%d -> %dx%d)",
            m_eSensorMode, eSensorMode, m_fgInit, fgForce, m_u4P1BinWidth, m_u4P1BinHeight, u4Width, u4Height);

        m_fgInit = MTRUE;
        m_eSensorMode = eSensorMode;
        m_ePrevSensorMode = ePrevSensorMode;
        m_u4P1BinWidth = u4Width;
        m_u4P1BinHeight = u4Height;

        // modify trfm cfg due to frontal bin
        MUINT32 u4CapW = 0, u4CapH = 0;
        MUINT32 u4TgW = 0, u4TgH = 0;
        if (getRawSize(ESensorMode_Capture, u4CapW, u4CapH) && (u4CapW != 0) && (u4CapH != 0) &&
            getRawSize(eSensorMode, u4TgW, u4TgH) && (u4TgW != 0) && (u4TgH != 0))
        {
            m_rCurTrfmCfg.u4ResizeW = m_rTransformCfg[eSensorMode].u4ResizeW * u4Width  / u4TgW;
            m_rCurTrfmCfg.u4ResizeH = m_rTransformCfg[eSensorMode].u4ResizeH * u4Height / u4TgH;
            m_rCurTrfmCfg.u4GridX   = m_rTransformCfg[eSensorMode].u4GridX;
            m_rCurTrfmCfg.u4GridY   = m_rTransformCfg[eSensorMode].u4GridY;
            m_rCurTrfmCfg.u4X       = m_rTransformCfg[eSensorMode].u4X * u4Width  / u4TgW;
            m_rCurTrfmCfg.u4Y       = m_rTransformCfg[eSensorMode].u4Y * u4Height / u4TgH;
            m_rCurTrfmCfg.u4W       = m_rTransformCfg[eSensorMode].u4W * u4Width  / u4TgW;
            m_rCurTrfmCfg.u4H       = m_rTransformCfg[eSensorMode].u4H * u4Height / u4TgH;
            LSC_LOG("Trfm(%d)[%dx%d -> %dx%d, crop(%d,%d,%d,%d)]", eSensorMode, u4CapW, u4CapH,
                    m_rCurTrfmCfg.u4ResizeW, m_rCurTrfmCfg.u4ResizeH, m_rCurTrfmCfg.u4X, m_rCurTrfmCfg.u4Y, m_rCurTrfmCfg.u4W, m_rCurTrfmCfg.u4H);
            doShadingTrfm();

            if(u4Width != u4TgW  || u4Height!= u4TgH) // re-gen bin size
            {
                ILscTbl::Config rawConfig=ILscTbl::makeConfig(   u4TgW,    u4TgH,
                    m_rCurTrfmCfg.u4GridX, m_rCurTrfmCfg.u4GridY);
                ILscTbl::Config binConfig=ILscTbl::makeConfig( u4Width, u4Height,
                    m_rCurTrfmCfg.u4GridX, m_rCurTrfmCfg.u4GridY);


                LSC_LOG("[Re-gen bin info] Raw size ( %d x %d), Bin size ( %d x %d )", u4TgW, u4TgH, u4Width, u4Height);
            } else {
                m_binOnOff=MFALSE;
            }
        }
        else
        {
            LSC_ERR("Wrong size: Cap size(%dx%d), TG size scn(%d): (%dx%d)", u4CapW, u4CapH, eSensorMode, u4TgW, u4TgH);
        }

        // load table to dma buffer
        MINT32 j;
        for (j = 0; j < SHADING_SUPPORT_CT_NUM; j++)
        {
            ILscTbl& rTbl = m_rCurLscTbl[j];
            rTbl.toBuf(*m_pLscBuf[j]);
        }

        // TSF
        if(!m_pTsf->isInit())
        {
            m_pTsf->config(m_rConfig);
            m_pTsf->init();
            m_pTsf->setOnOff(m_pNvramOtp->getTsfNvram()->TSF_CFG.isTsfEn ? MTRUE : MFALSE);
        }

        m_pTsf->setConfig(eSensorMode, u4Width, u4Height, fgForce);
        calSl2Config(u4Width, u4Height, m_rSl2Cfg);
    }
    return MTRUE;
}

ESensorMode_T
LscMgrDefault::
getSensorMode() const
{
    return m_eSensorMode;
}

const ILscTable::TransformCfg_T&
LscMgrDefault::
getTransformCfg(ESensorMode_T eSensorMode) const
{
    return m_rTransformCfg[eSensorMode];
}

const ILscTable::TransformCfg_T&
LscMgrDefault::
getCurTrfmCfg() const
{
    return m_rCurTrfmCfg;
}

MUINT32
LscMgrDefault::
getCTIdx()
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_GET_CT) ? MTRUE : MFALSE;
    if (SHADING_SUPPORT_CT_NUM <= m_u4CTIdx)
    {
        LSC_ERR("m_u4CTIdx(%d) exceeds", m_u4CTIdx);
    }

    CAM_LOGD_IF(fgLogEn, "[%s] CT(%d)\n", __FUNCTION__, m_u4CTIdx);
    return m_u4CTIdx;
}

MBOOL
LscMgrDefault::
setCTIdx(MUINT32 const u4CTIdx)
{
    MUINT32 _u4CtIdx;
    MINT32 dbg_ct = 0;
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_SET_CT) ? MTRUE : MFALSE;
    GET_PROP("vendor.debug.lsc_mgr.ct", -1, dbg_ct);

    if (dbg_ct != -1)
    {
        LSC_LOG("DEBUG CT(%d)", dbg_ct);
        _u4CtIdx = dbg_ct;
    }
    else
    {
        _u4CtIdx = u4CTIdx;
    }

    if (SHADING_SUPPORT_CT_NUM <= _u4CtIdx)
    {
        LSC_LOG("_u4CtIdx(%d) exceeds", _u4CtIdx);
        return MFALSE;
    }

    m_u4CTIdx = _u4CtIdx;
    CAM_LOGD_IF(fgLogEn, "[%s] CT(%d)", __FUNCTION__, _u4CtIdx);

    return MTRUE;
}

MBOOL
LscMgrDefault::
setRatio(MUINT32 u4Ratio)
{
    m_u4Rto = u4Ratio > 32 ? 32 : u4Ratio;
    return MTRUE;
}

MUINT32
LscMgrDefault::
getRatio() const
{
    return m_u4Rto;
}

MBOOL
LscMgrDefault::
getOnOff(void) const
{

    return m_fgOnOff;
}

MVOID
LscMgrDefault::
setOnOff(MBOOL fgOnOff)
{
    m_fgOnOff = fgOnOff;
}

MBOOL
LscMgrDefault::
getOnOffP2(void) const
{
    return m_fgOnOffP2;
}

MVOID
LscMgrDefault::
setOnOffP2(MBOOL fgOnOff)
{
    m_fgOnOffP2 = fgOnOff;
}

MVOID
LscMgrDefault::
setFactoryMode(MBOOL fgOnOff)
{
    m_fgIsFactory=fgOnOff;
    //m_pTsf->setIsFactory(fgOnOff);
}

MVOID
LscMgrDefault::
updateLsc()
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;
    MUINT32 u4CTIdx;

    u4CTIdx = getCTIdx();
    u4CTIdx = u4CTIdx >= 3 ? 3 : u4CTIdx;

    CAM_LOGD_IF(fgLogEn, "[%s] +", __func__);

    m_rCurLscTbl[u4CTIdx].editRsvdData().u4HwRto = m_u4Rto;

    // put to isp_mgr buf
    syncTbl(m_rCurLscTbl[u4CTIdx]);

    CAM_LOGD_IF(fgLogEn, "[%s] -", __func__);
}

MVOID
LscMgrDefault::
setTsfOnOff(MBOOL fgOnOff)
{
    m_pTsf->setOnOff(fgOnOff);
}

MBOOL
LscMgrDefault::
getTsfOnOff() const
{
    return m_pTsf->getOnOff();
}

MBOOL
LscMgrDefault::
config(NS3Av3::shadingConfig_T rConfig)
{
    m_rConfig=rConfig;
    m_rConfig.AAOBlockW=40;
    m_rConfig.AAOBlockH=30;

    LSC_LOG("[%s] save HbinWeight/HbinHeight= (%d, %d), blockH/blockW = (%d, %d)",
        __func__,
        m_rConfig.u4HBinWidth, m_rConfig.u4HBinHeight,
        m_rConfig.AAOBlockH, m_rConfig.AAOBlockW);
    return MTRUE;
}

MBOOL
LscMgrDefault::
setDumbFilePrefix(const std::string filename, E_LSC_MODULE_T eModu)
{
    return MFALSE;
}

MVOID
LscMgrDefault::
updateTsf(const TSF_INPUT_INFO_T& rInputInfo)
{
    m_pTsf->update(rInputInfo);
}

MBOOL
LscMgrDefault::
updateRatio(MINT32 i4MagicNum,MUINT32 u4Rto)
{
    return m_pTsf->updateRatio(i4MagicNum,u4Rto);
}

MBOOL
LscMgrDefault::
waitTsfExecDone()
{
    return m_pTsf->waitDone();
}

ILscBuf*
LscMgrDefault::
createBuf(const char* name, ILscBuf::E_BUF_MODE_T eBufMode)
{
    MUINT32 u4BufCount = m_rBufPool.size();
    ILscBuf* pBuf = new ILscBuf(m_eSensorDev, u4BufCount, MFALSE, name, eBufMode);
    if (pBuf)
    {
        m_rBufPool.push_back(pBuf);
        LSC_LOG("OK: pBuf(%p, %s), poolsize(%d)", pBuf, pBuf->getName(), (MINT32)m_rBufPool.size());
    }
    else
    {
        LSC_ERR("NG: poolsize(%d)", u4BufCount);
    }
    return pBuf;
}

MBOOL
LscMgrDefault::
destroyBuf(ILscBuf* pBuf)
{
    if (pBuf)
    {
        std::list<ILscBuf*>::iterator iter;
        for (iter = m_rBufPool.begin(); iter != m_rBufPool.end(); iter++)
        {
            if (pBuf == *iter)
            {
                LSC_LOG("OK: pBuf(%p, %s), poolsize(%d)", pBuf, pBuf->getName(), (MINT32)m_rBufPool.size());
                delete pBuf;
                *iter = NULL;
                m_rBufPool.erase(iter);
                return MTRUE;
            }
        }
    }
    return MFALSE;
}

MBOOL
LscMgrDefault::
syncBuf(ILscBuf* pBuf, E_LSC_MODULE_T eModu)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    if (LSC_P1 == eModu)
    {
        m_pCurrentBuf = pBuf;
    }
    else
    {
        LSC_LOG("sync P2 shading buf");
        m_pCurrBufP2 = pBuf;
    }
    return MTRUE;
}

MBOOL
LscMgrDefault::
syncTbl(const ILscTbl& rTbl, E_LSC_MODULE_T eModu)
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    std::lock_guard<std::mutex> lock(m_Lock);
    if (LSC_P1 == eModu)
    {
        //this function only support on TSF disable mode
        ILscBuf* pBuf = m_prLscBufRing[m_u4BufIdx];

        CAM_LOGD_IF(fgLogEn, "[%d] sync tbl p1 (va, pa) = (%p, %p)",
            __func__, m_prLscBufRing[m_u4BufIdx], m_prLscBufRing[m_u4BufIdx]->getPhyAddr());

        rTbl.toBuf(*pBuf);
        pBuf->setRatio(rTbl.getRsvdData().u4HwRto);
        //Get shading table index for Hal
        m_u4CurrentBufIdx = m_u4BufIdx;

        m_u4BufIdx ++;
        m_u4BufIdx = m_u4BufIdx >= RING_BUF_NUM ? 0 : m_u4BufIdx;
        m_pCurrentBuf = pBuf;
    }
    else
    {
        LSC_LOG("sync P2 shading tbl to buf");
        ILscBuf* pBuf = m_prLscBufP2[m_u4BufIdxP2];
        rTbl.toBuf(*pBuf);
        pBuf->setRatio(rTbl.getRsvdData().u4HwRto);
        //begin test
        //MUINT8* pDest = (MUINT8*)pBuf->getTable();
        //::memset(pDest, 0x1, pBuf->getSize());
        //end test
        m_u4BufIdxP2 ++;
        m_u4BufIdxP2 = m_u4BufIdxP2 >= RING_BUF_NUM ? 0 : m_u4BufIdxP2;
        m_pCurrBufP2 = pBuf;
    }
    return MTRUE;
}

MBOOL
LscMgrDefault::
syncTSFO(const std::vector<MUINT8> &inputTSFO)
{
    m_TsfoBufRing[m_u4TsfBufIdx] = inputTSFO;

    m_u4CurrTsfBufIdx = m_u4TsfBufIdx;
    m_u4TsfBufIdx++;
    m_u4TsfBufIdx = m_u4TsfBufIdx >= RING_BUF_NUM ? 0 : m_u4TsfBufIdx;

    return MTRUE;
}

NSIspTuning::ILscMgr::
SL2_CFG_T
LscMgrDefault::
getSl2() const
{
    SL2_CFG_T tmpSL2Cfg=m_rSl2Cfg;
    //LSC_ERR("X:%d Y:%d R:%d", m_rSl2Cfg.i4CenterX, m_rSl2Cfg.i4CenterY, m_rSl2Cfg.i4R2);

    return m_rSl2Cfg;
}

MBOOL
LscMgrDefault::
syncBufAndRatio(ILscBuf** pBuf, MUINT32 ratio, E_LSC_MODULE_T eModu)
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;
    std::lock_guard<std::mutex> lock(m_Lock);
    if (LSC_P1 == eModu)
    {

        m_pCurrentBuf = *pBuf;
        m_prTsfBufRing[m_u4BufIdx] = *pBuf;

        m_u4CurrentBufIdx = m_u4BufIdx;
        m_u4BufIdx ++;
        m_u4BufIdx = m_u4BufIdx >= RING_BUF_NUM ? 0 : m_u4BufIdx;
        m_u4Rto = ratio;

        CAM_LOGD_IF(fgLogEn, "[%s] m_pCurrentBuf address %p", __func__, m_pCurrentBuf);
    }
    else
    {
        LSC_ERR("This API (syncBufAndRatio) only supported in P1");
    }
    return MTRUE;
}

ILscBuf*
LscMgrDefault::
getP2Buf() const
{
    if(getOnOffP2())
    {
        LSC_LOG("[%s] LSC is on return pointer %p", __func__, m_pCurrBufP2);
        return m_pCurrBufP2;
    }
    else
    {
        LSC_LOG("[%s] LSC is off return null pointer", __func__);
        return NULL;
    }
}

MBOOL
LscMgrDefault::
evaluate_cus_Ratio(LSC_INPUT_INFO_T &rLscInputInfo, MINT32 i4IsLockRto)
{
    // Notice : This need update to ISP caminfo
    //rIspCamInfo.eIdx_Shading_CCT=(NSIspTuning::EIndex_Shading_CCT_T)getCTIdx();

    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;

    MINT32 i4Rto;
    getPropInt("vendor.debug.lsc_mgr.ratio", &i4Rto, -1);

    MBOOL fgLockRto = rLscInputInfo.bAELock || rLscInputInfo.bAELimiter || i4IsLockRto;
    MBOOL fgAutoRto = (i4Rto == -1);

    IspTuningCustom* m_pIspTuningCustom = IspTuningCustom::createInstance(m_eSensorDev, m_i4SensorID);

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(rLscInputInfo);
        setCTIdx(new_cct_idx);

        if (fgAutoRto && !fgLockRto)
        {
            //AAA_TRACE_LSC(evaluate_Shading_Ratio);
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(rLscInputInfo);
            //AAA_TRACE_END_LSC;
            setRatio(i4Rto);
        }
        else if (!fgAutoRto)
        {
            setRatio(i4Rto);
        }
        // else: lock, do not update ratio
    }
    else
    {
        if (fgAutoRto) i4Rto = 32;
        setRatio(i4Rto);
    }

    CAM_LOGD_IF(fgLogEn, "[%s] (flash, iso, rto)=(%d, %d, %d), auto(%d), lock(%d,%d), dyn(%d), PreCapFlashIsLockRto(%d)", __FUNCTION__,
        rLscInputInfo.isFlash, rLscInputInfo.u4RealISOValue, i4Rto, fgAutoRto,
        rLscInputInfo.bAELock, rLscInputInfo.bAELimiter, m_fgDynamicShading, i4IsLockRto);

    return  MTRUE;
}

MVOID
LscMgrDefault::
enableDynamicShading(MBOOL fgEnable, MUINT32 u4iOperMode)
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;
    CAM_LOGD_IF(fgLogEn, "[%s] fgEnable(%d) m_eOperMode(%d), (old, new)=(%d, %d)", __FUNCTION__, fgEnable, u4iOperMode, m_fgDynamicShading, fgEnable);

    std::lock_guard<std::mutex> lock(m_Lock);

    EOperMode_T const eOperMode=(EOperMode_T)u4iOperMode;
    if (eOperMode == EOperMode_Meta)
    {
        fgEnable = MFALSE;
    }

    m_fgDynamicShading = fgEnable;
}

MBOOL
LscMgrDefault::
getLSCResultPool(MVOID *outputLSC_Result)
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    CAM_LOGD_IF(fgLogEn, "[%s] +", __FUNCTION__);
    MINT32 defaultLSCValue = -1;
#if CAM3_DISABLE_SHADING
    defaultLSCValue = 0;
#endif
    //////////////////////////////////////
    MINT32 i4Tsf;
    getPropInt("vendor.debug.lsc_mgr.manual_tsf", &i4Tsf, defaultLSCValue);
    MINT32 i4OnOff;
    getPropInt("vendor.debug.lsc_mgr.enable", &i4OnOff, defaultLSCValue);

    if (i4Tsf != -1){
        setTsfOnOff(i4Tsf ? MTRUE : MFALSE);
    }

    if (i4OnOff != -1){
        if(m_fgOnOff!=i4OnOff)
        {
            m_fgOnOff=(i4OnOff ? MTRUE : MFALSE);
            CAM_LOGD_IF(fgLogEn, "[%s] LSC on off change %d", __func__, m_fgOnOff);
        }
    }

    if (!getTsfOnOff())
        updateLsc();
    else
        waitTsfExecDone();


    //get result of LSC
    MBOOL ret=MFALSE;
    NS3Av3::LSCConfigResult_T *output=(NS3Av3::LSCConfigResult_T *)outputLSC_Result;

    if(m_pCurrentBuf && m_rConfig.u4HBinWidth!=0 && m_rConfig.u4HBinHeight!=0)
    {
        output->u4RawWd=m_rConfig.u4HBinWidth;
        output->u4RawHt=m_rConfig.u4HBinHeight;

        output->rResultBuf=m_pCurrentBuf;
        output->ratio     = m_u4Rto;

        output->fgOnOff=m_fgOnOff;

        output->eIdx_Shading_CCT=m_u4CTIdx;

        CAM_LOGD_IF(fgLogEn, "[%s] tbl (va, pa): (%p, %p), ratio %d, OnOff %d", __func__, m_pCurrentBuf, m_pCurrentBuf->getPhyAddr(), m_u4Rto, m_fgOnOff);

        //Get TSFS size from ALG
        TSFS_INFO_STRUCT rTsfsConfig;
        rTsfsConfig.IMG_WD = m_rConfig.u4HBinWidth;
        rTsfsConfig.IMG_HT = m_rConfig.u4HBinHeight;

        rTsfsConfig.TSFS_W_HNUM = (m_rConfig.AAOBlockW!=0)?m_rConfig.AAOBlockW:40;
        rTsfsConfig.TSFS_W_VNUM = (m_rConfig.AAOBlockH!=0)?m_rConfig.AAOBlockH:30;

        m_pTsf->getTsfsConfig(rTsfsConfig);

        output->tsfsConfig = rTsfsConfig;

        ret = MTRUE;
    }

    //set slk params
    SL2_CFG_T rSl2Cfg = getSl2();
    output->i4CenterX = rSl2Cfg.i4CenterX;
    output->i4CenterY = rSl2Cfg.i4CenterY;
    output->i4R0      = rSl2Cfg.i4R0;
    output->i4R1      = rSl2Cfg.i4R1;
    output->i4R2      = rSl2Cfg.i4R2;
    output->i4Gain0   = rSl2Cfg.i4Gain0;
    output->i4Gain1   = rSl2Cfg.i4Gain1;
    output->i4Gain2   = rSl2Cfg.i4Gain2;
    output->i4Gain3   = rSl2Cfg.i4Gain3;
    output->i4Gain4   = rSl2Cfg.i4Gain4;
    output->i4SetZero = rSl2Cfg.i4SetZero;

    CAM_LOGD_IF(fgLogEn, "[%s] -", __FUNCTION__);

    return ret;
}
