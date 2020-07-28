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
#include <liblsctrans/ShadingTblTransform.h>

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <isp_mgr.h>

#include <cmath>

using namespace NSIspTuning;

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
 * TsfDft::TSF_INPUT_STAT_T
 *******************************************************************************/
MBOOL
TsfDft::TSF_INPUT_STAT_T::
dump(const char* filename) const
{
    FILE* fptr = fopen(filename, "wb");

    if (fptr)
    {
    #if STAT16BIT
        fwrite(vecStat.data(), vecStat.size(), 2, fptr);
    #else
        fwrite(vecStat.data(), vecStat.size(), 1, fptr);
    #endif
        fwrite(&rAwbInfo, sizeof(ILscMgr::TSF_AWB_INFO), 1, fptr);
        fclose(fptr);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
TsfDft::TSF_INPUT_STAT_T::
put(const ILscMgr::TSF_INPUT_INFO_T& rInStat, MUINT8 /*fgOpt*/)
{
    MBOOL fgRet = MTRUE;

    // AWB info
    u4FrmId = rInStat.u4FrmId;
    u4FrmReq = rInStat.u4FrmReq;
    rAwbInfo = rInStat.rAwbInfo;
    //u4Ratio = m_pLsc->getRatio();


    if(rInStat.prAwbStat)
    {
        vecStat.resize(rInStat.u4SizeAwbStat/sizeof(MUINT16));
        ::memcpy(vecStat.data(), rInStat.prAwbStat, rInStat.u4SizeAwbStat);
    }
    return fgRet;
}

/*******************************************************************************
 * TsfDft
 *******************************************************************************/
ILscTsf*
TsfDft::
createInstance(MUINT32 u4SensorDev)
{
    switch (u4SensorDev)
    {
    default:
    case ESensorDev_Main:
        static TsfDft singleton_main(static_cast<MUINT32>(ESensorDev_Main));
        return &singleton_main;
    case ESensorDev_MainSecond:
        static TsfDft singleton_main2(static_cast<MUINT32>(ESensorDev_MainSecond));
        return &singleton_main2;
    case ESensorDev_Sub:
        static TsfDft singleton_sub(static_cast<MUINT32>(ESensorDev_Sub));
        return &singleton_sub;
    case ESensorDev_SubSecond:
        static TsfDft singleton_sub2(static_cast<MUINT32>(ESensorDev_SubSecond));
        return &singleton_sub2;
    }
}

void
TsfDft::
destroyInstance()
{}

TsfDft::
TsfDft(MUINT32 u4SensorDev)
    : m_u4SensorDev(u4SensorDev)
    , m_eSensorMode(ESensorMode_Capture)
    , m_u4LogEn(0)
    , m_bTSF(MFALSE)
    , m_bTSFInstanced(MFALSE)
    , m_u1CfgOpt(0)
    , m_fgStatOpt(0x7)
    , m_u4TblIdx(0)
    , m_u4ProcCnt(0)
    , m_pLsc(NULL)
    , m_bDump(MFALSE)
    , m_u1OtpPixId(0)
    , m_u1OtpGridX(0)
    , m_u1OtpGridY(0)
{
    LSC_LOG("Enter Type 0: Default Cycle");
    for (MINT32 i = 0; i < RING_TBL_NUM; i++)
        m_rStat[i].u4Ratio = 32;
}

TsfDft::
~TsfDft()
{
    LSC_LOG("Exit Type 0: Default Cycle");
}

MBOOL
TsfDft::
init(ESensorMode_T eFullSensorMode)
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    GET_PROP("vendor.debug.lsc_mgr.log", "0", m_u4LogEn);
    // Default is 0. Change to 1 => Because Aspect ratio change, it need to reform table.
    GET_PROP("vendor.debug.lsc_mgr.cfg", "1", m_u1CfgOpt);

    m_pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));
    m_pLsc->getRawSize(eFullSensorMode, m_u4FullW, m_u4FullH);

    createTsf();
    createThread();

    // Start to create tsf_in / tsf_out

    MUINT32 id = 0xFFFFFFFF;           /*m_pLsc->m_rBufPool.size()*/
    char strName[32];
    sprintf(strName, "TSF_in");
    m_pTsfIn = new ILscBuf(m_u4SensorDev, id, MFALSE, strName, ILscBuf::E_LSC_IMEM);

     if (!m_pTsfIn)
    {
        LSC_ERR("Fail to create TsfIn buffer.");
    }

    m_pTsfInDump = new ILscBuf(m_u4SensorDev, id, MFALSE, strName, ILscBuf::E_LSC_IMEM);
    if (!m_pTsfInDump)
    {
        LSC_ERR("Fail to create m_pTsfInDump buffer.");
    }

    id = 0xFFFFFFFE;                  /*m_pLsc->m_rBufPool.size()*/
    sprintf(strName, "TSF_out");
    m_pTsfOut = new ILscBuf(m_u4SensorDev, id, MFALSE, strName, ILscBuf::E_LSC_IMEM);

    if (!m_pTsfOut)
    {
        LSC_ERR("Fail to create TsfOut buffer.");
    }

#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
    LSC_LOG("CT_3 Shading Table will be used in casees when Flash is ON.");
    const ILscTbl* pFlashOnTbl = m_pLsc->getCapLut(3);
    m_rFlashOnTbl = *pFlashOnTbl;
    m_rFlashOnTbl.editRsvdData().u4HwRto = 32;
#endif

    // End to create tsf_in / tsf_out

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);
    return MTRUE;
}

MBOOL
TsfDft::
uninit()
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);
    m_bDump = MFALSE;
    if (m_pTsfIn)
    {
        delete m_pTsfIn;
        m_pTsfIn = NULL;
    }
    if (m_pTsfInDump)
    {
        delete m_pTsfInDump;
        m_pTsfInDump = NULL;
    }
    if (m_pTsfOut)
    {
        delete m_pTsfOut;
        m_pTsfOut = NULL;
    }
    m_rTsfEnvInfo.pCb = NULL;
    destroyThread();
    destroyTsf();
    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);
    return MTRUE;
}

MBOOL
TsfDft::
loadOtpDataForTsf()
{
    ILscNvram* pNvram = ILscNvram::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));
    const ILscTbl* pGolden = pNvram->getGolden();
    const ILscTbl* pUnit = pNvram->getUnit();

    // Golden/Unit
    // for TSF
    GAIN_TBL& rTsfGainGolden    = m_rTsfEnvInfo.ShadingTbl.Golden;
    GAIN_TBL& rTsfGainUnit      = m_rTsfEnvInfo.ShadingTbl.Unit;
    const ILscTable::Config& rUnitCfg = pUnit->getConfig();
    rTsfGainUnit.bayer        = (MTK_BAYER_ORDER_ENUM)pUnit->getBayer();
    rTsfGainUnit.offset_x     = 0;
    rTsfGainUnit.offset_y     = 0;
    rTsfGainUnit.crop_width   = rUnitCfg.i4ImgWd;
    rTsfGainUnit.crop_height  = rUnitCfg.i4ImgHt;
    rTsfGainUnit.grid_x       = rUnitCfg.i4GridX;
    rTsfGainUnit.grid_y       = rUnitCfg.i4GridY;
    rTsfGainGolden = rTsfGainUnit;
    m_u1OtpPixId = rTsfGainUnit.bayer;
    m_u1OtpGridX = rTsfGainUnit.grid_x;
    m_u1OtpGridY = rTsfGainUnit.grid_y;

    MUINT32 u4GoldenGainTblSize = pGolden->getSize();
    MUINT32 u4UnitGainTblSize = pUnit->getSize();
    LSC_LOG("u4GoldenGainTblSize(%d), u4UnitGainTblSize(%d)", u4GoldenGainTblSize, u4UnitGainTblSize);
    if (u4GoldenGainTblSize != u4UnitGainTblSize)
    {
        LSC_ERR("Size of the golden gain table and unit gain table is not the same.");
    }

    rTsfGainGolden.Tbl          = new MUINT32[u4GoldenGainTblSize/4];
    rTsfGainUnit.Tbl            = new MUINT32[u4UnitGainTblSize/4];
    ::memcpy(rTsfGainGolden.Tbl, pGolden->getData(), u4GoldenGainTblSize);
    ::memcpy(rTsfGainUnit.Tbl, pUnit->getData(), u4UnitGainTblSize);

    LSC_LOG("Golden(%p), Unit(%p)", rTsfGainGolden.Tbl, rTsfGainUnit.Tbl);
    LSC_LOG("Bayer(%d), Crop(%d,%d,%d,%d), Grid(%d,%d)",
        rTsfGainUnit.bayer, rTsfGainGolden.offset_x, rTsfGainGolden.offset_y, rTsfGainGolden.crop_width, rTsfGainGolden.crop_height,
        rTsfGainGolden.grid_x, rTsfGainGolden.grid_y);
#if 1
    switch( pNvram->getOtpData()->TableRotation )
    {
        default:
        case 0:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_R0D;
        break;
        case 1:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_R180D;
        break;
        case 2:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_MIRROR;
        break;
        case 3:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_FLIP;
        break;
    }
#endif
    return MTRUE;
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
        const AE_PARAM_T* pAeParam;

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
        }

        MTK_TSF_GET_ENV_INFO_STRUCT rTsfGetEnvInfo;
        m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &rTsfGetEnvInfo);
        LSC_LOG("MTKTSF_FEATURE_GET_ENV_INFO, buffer size(%d)", rTsfGetEnvInfo.WorkingBuffSize);

        // allocate working buffer
        MUINT8* gWorkinBuffer = new MUINT8[rTsfGetEnvInfo.WorkingBuffSize];
        ::memset(gWorkinBuffer, 0, rTsfGetEnvInfo.WorkingBuffSize);

        m_fgStatOpt = STAT_OPT(
            pAeParam->strAEParasetting.bEnableHDRLSB,
            pAeParam->strAEParasetting.bEnableAEOVERCNTconfig,
            pAeParam->strAEParasetting.bEnableTSFSTATconfig);
        m_rTsfEnvInfo.ImgWidth    = pAeParam->strAEParasetting.u4AEWinodwNumX; //rAwbStatParma.i4WindowNumX;
        m_rTsfEnvInfo.ImgHeight   = pAeParam->strAEParasetting.u4AEWinodwNumY; //rAwbStatParma.i4WindowNumY;
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
        const AWB_ALGO_CAL_T& rAlgoCalParam = pNvram3A->rAWBNVRAM[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam;
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

MVOID
TsfDft::
destroyTsf()
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    m_prTsf->TsfExit();
    m_prTsf->destroyInstance(m_prTsf);

    delete [] (MUINT8*)m_rTsfEnvInfo.WorkingBufAddr;

    if (m_rTsfEnvInfo.ShadingTbl.Golden.Tbl)
    {
        delete [] m_rTsfEnvInfo.ShadingTbl.Golden.Tbl;
        m_rTsfEnvInfo.ShadingTbl.Golden.Tbl = NULL;
    }

    if (m_rTsfEnvInfo.ShadingTbl.Unit.Tbl)
    {
        delete [] m_rTsfEnvInfo.ShadingTbl.Unit.Tbl;
        m_rTsfEnvInfo.ShadingTbl.Unit.Tbl = NULL;
    }

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);
}

MBOOL
TsfDft::
tsfResetTbl(ESensorMode_T /*eLscScn*/)
{
    // reset proc shading table and result shading table
    const ILscTbl* pBaseLsc = m_pLsc->getLut(2);
    if (pBaseLsc)
    {
        m_rLscBaseTbl = *pBaseLsc;

        MINT32 i;
        for (i = 0; i < RING_TBL_NUM; i++)
        {
            m_rLscRingTbl[i] = m_rLscBaseTbl;
            m_rStat[i].u4Ratio = 32;
        }
        return MTRUE;
    }

    return MFALSE;
}

MBOOL
TsfDft::
tsfReformTbl()
{
    ILscTable::TransformCfg_T rTrfmA = m_rTransformCfg;
    ILscTable::TransformCfg_T rTrfmB = m_pLsc->getCurTrfmCfg();
    m_rTransformCfg = rTrfmB;

    // FOV of A
    rTrfmA.u4X = rTrfmA.u4X * m_u4FullW / rTrfmA.u4ResizeW;
    rTrfmA.u4Y = rTrfmA.u4Y * m_u4FullH / rTrfmA.u4ResizeH;
    rTrfmA.u4W = rTrfmA.u4W * m_u4FullW / rTrfmA.u4ResizeW;
    rTrfmA.u4H = rTrfmA.u4H * m_u4FullH / rTrfmA.u4ResizeH;
    rTrfmA.u4ResizeW = m_u4FullW;
    rTrfmA.u4ResizeH = m_u4FullH;

    // FOV of B
    rTrfmB.u4X = rTrfmB.u4X * m_u4FullW / rTrfmB.u4ResizeW;
    rTrfmB.u4Y = rTrfmB.u4Y * m_u4FullH / rTrfmB.u4ResizeH;
    rTrfmB.u4W = rTrfmB.u4W * m_u4FullW / rTrfmB.u4ResizeW;
    rTrfmB.u4H = rTrfmB.u4H * m_u4FullH / rTrfmB.u4ResizeH;
    rTrfmB.u4ResizeW = m_u4FullW;
    rTrfmB.u4ResizeH = m_u4FullH;

    if ((rTrfmB.u4X >= rTrfmA.u4X) && (rTrfmB.u4Y >= rTrfmA.u4Y) &&
        ((rTrfmA.u4X + rTrfmA.u4W) >= (rTrfmB.u4X + rTrfmB.u4W)) &&
        ((rTrfmA.u4Y + rTrfmA.u4H) >= (rTrfmB.u4Y + rTrfmB.u4H)))
    {
        // FOVA >= FOVB
        ILscTable::TransformCfg_T rTrfmRel = rTrfmB;
        rTrfmRel.u4X = rTrfmB.u4X - rTrfmA.u4X;
        rTrfmRel.u4Y = rTrfmB.u4Y - rTrfmA.u4Y;

        MINT32 i;
        for (i = 0; i < RING_TBL_NUM; i++)
        {
            ILscTbl rTbl = m_rLscRingTbl[i];
            rTbl.setConfig(rTrfmA.u4W, rTrfmA.u4H, rTbl.getConfig().i4GridX, rTbl.getConfig().i4GridY);
            if (!rTbl.cropOut(rTrfmRel, m_rLscRingTbl[i])) //Fix Coverity
            {
                LSC_ERR("Fail to crop matching LSC Gain Table.");
                return MFALSE;
            }
        }
        return MTRUE;
    }
    else
    {
        // just use the current table for initialing, at lease it will be similar.
        return MFALSE; //tsfResetTbl(m_eSensorMode);
    }
}

MBOOL
TsfDft::
tsfSetTbl(const ILscTbl& rTbl)
{
    return m_pLsc->syncTbl(rTbl, ILscMgr::LSC_P1);
}

MBOOL
TsfDft::
tsfSetSL2(const MTK_TSF_SL2_PARAM_STRUCT& rSL2)
{
    ILscMgr::SL2_CFG_T rSl2Cfg;

    // frontal binnig size
    MUINT32 u4BinW = m_rTsfLscParam.raw_wd;
    MUINT32 u4BinH = m_rTsfLscParam.raw_ht;
    // TG size
    MUINT32 u4TgW, u4TgH;
    if (m_pLsc->getRawSize(m_eSensorMode, u4TgW, u4TgH) && u4BinW != 0 && u4BinH != 0)
    {
        MUINT32 u4Rx = rSL2.SL2_CENTR_X;
        MUINT32 u4Ry = rSL2.SL2_CENTR_Y;
        MUINT32 u4R  = ((u4Rx + u4Ry) *1448) >> 11;
        // TG coordinate
        rSl2Cfg.i4CenterX = u4Rx * u4TgW / u4BinW;
        rSl2Cfg.i4CenterY = u4Ry * u4TgH / u4BinH;
        rSl2Cfg.i4R0      = u4R * (3*u4TgW) / (10*u4BinW);
        rSl2Cfg.i4R1      = u4R * (6*u4TgW) / (10*u4BinW);
        rSl2Cfg.i4R2      = u4R * (8*u4TgW) / (10*u4BinW);
        m_pLsc->updateSl2(rSl2Cfg);
        return MTRUE;
    }

    return MFALSE;
}

MBOOL
TsfDft::
tsfInit()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    m_prTsf->TsfExit();

    // SensorCrop
    // 0: full 4:3 FOV
    // 1: full 16:9 FOV (full horizontal FOV, cropped vertical FOV)
    // 2: general cropping case
    m_rTsfEnvInfo.Raw16_9Mode = 2;
    #if 1
    ILscTable::TransformCfg_T rCropCfg = m_pLsc->getCurTrfmCfg();
    m_rTsfEnvInfo.SensorCrop.full_width        = m_u4FullW;
    m_rTsfEnvInfo.SensorCrop.full_height    = m_u4FullH;
    m_rTsfEnvInfo.SensorCrop.resize_width    = rCropCfg.u4ResizeW;
    m_rTsfEnvInfo.SensorCrop.resize_height    = rCropCfg.u4ResizeH;
    m_rTsfEnvInfo.SensorCrop.crop_width        = rCropCfg.u4W;
    m_rTsfEnvInfo.SensorCrop.crop_height    = rCropCfg.u4H;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs    = rCropCfg.u4X;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs    = rCropCfg.u4Y;
    #else
    m_rTsfEnvInfo.SensorCrop.full_width        = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.full_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.resize_width    = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.resize_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_width        = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.crop_height    = m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs    = 0;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs    = 0;
    #endif
    LSC_LOG("SensorCrop(%d): Full(%d,%d), Resize(%d,%d), Crop(%d,%d,%d,%d)",
        m_eSensorMode,
        m_rTsfEnvInfo.SensorCrop.full_width      ,
        m_rTsfEnvInfo.SensorCrop.full_height  ,
        m_rTsfEnvInfo.SensorCrop.resize_width ,
        m_rTsfEnvInfo.SensorCrop.resize_height,
        m_rTsfEnvInfo.SensorCrop.crop_hor_offs,
        m_rTsfEnvInfo.SensorCrop.crop_ver_offs,
        m_rTsfEnvInfo.SensorCrop.crop_width   ,
        m_rTsfEnvInfo.SensorCrop.crop_height  );

    if (m_pTsfIn)
    {
        m_rTsfEnvInfo.buf_tsfi.VA = (MUINTPTR)(m_pTsfIn->editTable());
        m_rTsfEnvInfo.buf_tsfi.PA = (MUINTPTR)(m_pTsfIn->getPhyAddr());
        m_rTsfEnvInfo.buf_tsfi.size = m_pTsfIn->getSize();
    }
    else
    {
        LSC_ERR("Failed to setup m_rTsfEnvInfo.buf_tsfi.");
        m_rTsfEnvInfo.buf_tsfi.VA = 0;
        m_rTsfEnvInfo.buf_tsfi.PA = 0;
        m_rTsfEnvInfo.buf_tsfi.size = 0;
    }

    if (m_pTsfOut)
    {
        m_rTsfEnvInfo.buf_tsfo.VA = (MUINTPTR)(m_pTsfOut->getTable());
        m_rTsfEnvInfo.buf_tsfo.PA = (MUINTPTR)(m_pTsfOut->getPhyAddr());
        m_rTsfEnvInfo.buf_tsfo.size = m_pTsfOut->getSize();
    }
    else
    {
        LSC_ERR("Failed to setup m_rTsfEnvInfo.buf_tsfo.");
        m_rTsfEnvInfo.buf_tsfo.VA = 0;
        m_rTsfEnvInfo.buf_tsfo.PA = 0;
        m_rTsfEnvInfo.buf_tsfo.size = 0;
    }

    if (m_pTsfIn && m_pTsfOut)
    {
        m_rTsfEnvInfo.pCb = this;
    }
    else
    {
        LSC_ERR("m_rTsfEnvInfo.pCb is NULL. Failed to allocate memory for TSF buffers.");
        m_rTsfEnvInfo.pCb = NULL;
    }

    MINT32 i4TsfSL2En = 0;
    GET_PROP("vendor.debug.lsc_mgr.sl2", "-1", i4TsfSL2En);

    if (i4TsfSL2En == -1)
    {
        // 0:disable, 1:TSF's SL2, 2:NVRAM default
        i4TsfSL2En = isEnableSL2(m_u4SensorDev);
        LSC_LOG("TSF set SL2 default mode(%d)", i4TsfSL2En);
    }
    else
    {
        LSC_LOG("TSF set SL2 mode(%d)", i4TsfSL2En);
    }

    m_rTsfEnvInfo.EnableSL2      = i4TsfSL2En;
    m_rTsfEnvInfo.pLscConfig     = &m_rTsfLscParam;
    m_rTsfEnvInfo.BaseShadingTbl = (MINT32*) m_rLscBaseTbl.getData();   // base luma table

    LSC_LOG("StatOpt(0x%x), ImgWidth(%d), ImgHeight(%d), BayerOrder(%d), BaseShadingTbl(%p), Raw16_9Mode(%d), EnableSL2(%d), pLscConfig(%p)",
        m_fgStatOpt, m_rTsfEnvInfo.ImgWidth, m_rTsfEnvInfo.ImgHeight, m_rTsfEnvInfo.BayerOrder,
        m_rTsfEnvInfo.BaseShadingTbl, m_rTsfEnvInfo.Raw16_9Mode, m_rTsfEnvInfo.EnableSL2, m_rTsfEnvInfo.pLscConfig);

    // init
    ret = m_prTsf->TsfInit(&m_rTsfEnvInfo, NULL);

    m_prTsf->TsfReset();

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);

    return MTRUE;
}

MBOOL
TsfDft::
convert(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rInputTbl, MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo) const
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_SET_PROC) ? MTRUE : MFALSE;
#if 0
    const MINT32* pAwbForceParam = m_rTsfCfgTbl.TSF_CFG.rAWBInput;

    if (m_bTsfForceAwb && pAwbForceParam)
    {
        rProcInfo.ParaL           = pAwbForceParam[0];
        rProcInfo.ParaC           = pAwbForceParam[1];
        rProcInfo.FLUO_IDX        = pAwbForceParam[2];
        rProcInfo.DAY_FLUO_IDX    = pAwbForceParam[3];
    }
    else
#endif
    {
        rProcInfo.ParaL           = rInputStat.rAwbInfo.m_i4LV;
        rProcInfo.ParaC           = rInputStat.rAwbInfo.m_u4CCT;
        rProcInfo.FLUO_IDX        = rInputStat.rAwbInfo.m_FLUO_IDX;
        rProcInfo.DAY_FLUO_IDX    = rInputStat.rAwbInfo.m_DAY_FLUO_IDX;
    }
    rProcInfo.Gain.i4R = rInputStat.rAwbInfo.m_RGAIN;
    rProcInfo.Gain.i4G = rInputStat.rAwbInfo.m_GGAIN;
    rProcInfo.Gain.i4B = rInputStat.rAwbInfo.m_BGAIN;

    // AWB stat
    rProcInfo.ShadingTbl = (MINT32*)rInputTbl.getData();
#if STAT16BIT
    rProcInfo.ImgAddr = (MUINT16*)rInputStat.vecStat.data();
#else
    rProcInfo.ImgAddr = (MUINT8*)rInputStat.vecStat.data();
#endif
    rProcInfo.LscRA = rInputTbl.getRsvdData().u4HwRto;

    CAM_LOGD_IF(fgLogEn, "[%s] L(%d), C(%d), F(%d), DF(%d), R(%d), G(%d), B(%d) Rto(%d)\n",
        __FUNCTION__, rProcInfo.ParaL, rProcInfo.ParaC, rProcInfo.FLUO_IDX, rProcInfo.DAY_FLUO_IDX, rProcInfo.Gain.i4R, rProcInfo.Gain.i4G, rProcInfo.Gain.i4B, rProcInfo.LscRA);

    if( rProcInfo.Gain.i4R == 0 && rProcInfo.Gain.i4G == 0 && rProcInfo.Gain.i4B == 0 )
    {
        LSC_ERR("AWB info incorrect!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
TsfDft::
tsfSetProcInfo(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rInputTbl)
{
    MRESULT ret = S_TSF_OK;
    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputTbl, rProcInfo);
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &rProcInfo, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_PROC_INFO", ret);
        m_prTsf->TsfReset();
        m_u4PerFrameStep = 0;
    }

    return (ret == S_TSF_OK);
}

TsfDft::E_TSF_STATE_T
TsfDft::
tsfMain()
{
    MRESULT ret = S_TSF_OK;
    E_TSF_STATE_T eState = E_TSF_NOT_READY;

    ret = m_prTsf->TsfMain();
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): TsfMain", ret);
        m_prTsf->TsfReset();
        m_u4PerFrameStep = 0;
        eState = E_TSF_FAIL;
        goto lbExit;
    }

    MTK_TSF_GET_PROC_INFO_STRUCT rTsfGetProc;
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &rTsfGetProc);
    if (rTsfGetProc.TsfState == MTKTSF_STATE_READY)
    {
        eState = E_TSF_READY;
    }
    else if (rTsfGetProc.TsfState == MTKTSF_STATE_OPT_DONE)
    {
        eState = E_TSF_OPT_DONE;
    }

lbExit:
    return eState;
}

MBOOL
TsfDft::
tsfGetResult(ILscTbl& rOutputTbl)
{
    MRESULT ret = S_TSF_OK;

    m_rTsfResult.ShadingTbl = static_cast<MUINT32*>(rOutputTbl.editData());
    m_rTsfResult.u4TblSize = rOutputTbl.getSize();
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, NULL, &m_rTsfResult);
    return (ret == S_TSF_OK);
}

MBOOL
TsfDft::
tsfCfgChg()
{
    MRESULT ret = S_TSF_OK;

    // this is for identical sized tables, but block numbers are different.
    // no cropping support.
    // ex. 10x10 -> 16x16
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    MTK_TSF_TBL_STRUCT rTsfTbl;
    rTsfTbl.pLscConfig = &m_rTsfLscParam;
    rTsfTbl.ShadingTbl = (MINT32*)m_rLscBaseTbl.getData();
    // convert current gain table from nxn to mxm (ex. 10x10 to 16x16)
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &rTsfTbl, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    m_rLscBaseTbl.setConfig(
        m_rTsfLscParam.raw_wd, m_rTsfLscParam.raw_ht,
        m_rTsfLscParam.x_grid_num, m_rTsfLscParam.y_grid_num);

    MINT32 i;
    for (i = 0; i < RING_TBL_NUM; i++)
    {
        m_rLscRingTbl[i].setConfig(m_rLscBaseTbl.getConfig());
    }
#if 0
    // convert transformed table to HW coef table.
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &m_rTsfTbl, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_GEN_CAP_TBL", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        //tsfSetSL2(m_rTsfResult.SL2Para);
    }
#endif
    //tsfSetTbl(m_rLscBaseTbl);
    tsfSetTbl(m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]);

lbExit:
    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);

    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfBatch(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rInputTbl, ILscTbl& rOutputTbl)
{
    MRESULT ret = S_TSF_OK;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d)", m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio);

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    if (convert(rInputStat, rInputTbl, rProcInfo) == MFALSE)
    {
        LSC_ERR("convert fail (return MFALSE)");
        return MFALSE;
    }

    m_rTsfResult.ShadingTbl = static_cast<MUINT32*>(rOutputTbl.editData());
    m_rTsfResult.u4TblSize = rOutputTbl.getSize();

    m_prTsf->TsfReset();
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &rProcInfo, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        //tsfSetTbl(rOutputTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
        if (fgDump)
        {
            char strFile[512] = {'\0'};
            MUINT32 u4ProcCnt = m_u4ProcCnt;
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_bat.bin", u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);
            rInputTbl.dump(strFile);
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);
            rOutputTbl.dump(strFile);
        }
    }

    LSC_LOG_END("Sensor(%d), #(%d)", m_u4SensorDev, rInputStat.u4FrmId);

    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfBatchCap(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rInputTbl, ILscTbl& rOutputTbl)
{
    MRESULT ret = S_TSF_OK;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d), m_bDump(%d)", m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio, m_bDump);

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputTbl, rProcInfo);

    m_rTsfResult.ShadingTbl = static_cast<MUINT32*>(rOutputTbl.editData());
    m_rTsfResult.u4TblSize = rOutputTbl.getSize();

    m_prTsf->TsfReset();
    // disable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)0, 0);
    // batch
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &rProcInfo, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        //tsfSetTbl(rOutputTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
        if (fgDump)
        {
            char strFile[512] = {'\0'};
            MUINT32 u4ProcCnt = m_u4ProcCnt;
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_cap.bin", u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);
            rInputTbl.dump(strFile);
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);
            rOutputTbl.dump(strFile);
        }
        if (m_bDump)
        {
            char strFile[512] = {'\0'};
            MUINT32 u4ProcCnt = m_u4ProcCnt;
            sprintf(strFile, "%s.t0_%06d_%04d_tsfInStat_cap.bin", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);
            sprintf(strFile, "%s.t0_%06d_%04d_tsfInput_cap.tbl", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            rInputTbl.dump(strFile);
            sprintf(strFile, "%s.t0_%06d_%04d_tsfOutput_cap.tbl", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            rOutputTbl.dump(strFile);

            sprintf(strFile, "%s.t0_%06d_%04d_tsfInput_cap", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);

            rInputTbl.toBuf(*m_pTsfInDump);
            // SDBLK dump
            if (m_pTsfInDump)
            {
                if (!m_pTsfInDump->dump(strFile))
                {
                    LSC_ERR("Fail to dump %s", strFile);
                }
            }
        }
    }
    // enable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)1, 0);

    LSC_LOG_END("Sensor(%d), #(%d)", m_u4SensorDev, rInputStat.u4FrmId);

    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfRun(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rInputTbl, const ILscTbl& rTblPrior, ILscTbl& rOutputTbl)
{
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;
    MINT32 i4Case = 0;
    MUINT32 u4Step = m_u4PerFrameStep;

    CAM_LOGD_IF(fgLogEn, "[%s +] Sensor(%d), #(%d), ratio(%d), step(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio, u4Step);

    if (m_u4PerFrameStep == 0)
    {
        // only set proc info at the 1st frame.
        if (!tsfSetProcInfo(rInputStat, rInputTbl))
            return MFALSE;
    }
    m_u4PerFrameStep ++;

    E_TSF_STATE_T eState = tsfMain();
    if (eState == E_TSF_FAIL)
    {
        i4Case = -1;
    }
    else if (eState == E_TSF_READY)
    {
        if (tsfGetResult(rOutputTbl))
        {
            m_u4PerFrameStep = 0;
            rOutputTbl.editRsvdData().u4HwRto = rInputStat.u4Ratio;
            tsfSetTbl(rOutputTbl);
            tsfSetSL2(m_rTsfResult.SL2Para);
            i4Case = 1;
        }
        else
        {
            i4Case = -2;
        }
    }
    else
    {
        i4Case = 0;
        rOutputTbl = rTblPrior;
    }

    if (fgDump)
    {
        char strFile[512] = {'\0'};
        MUINT32 u4ProcCnt = m_u4ProcCnt;
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_%d.bin", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        rInputStat.dump(strFile);
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInput_%d.tbl", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        rInputTbl.dump(strFile);
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_%d.tbl", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        rOutputTbl.dump(strFile);
    }

    CAM_LOGD_IF(fgLogEn, "[%s -] Sensor(%d), #(%d), step(%d), case(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, u4Step, i4Case);

    return (i4Case >= 0);
}

MBOOL
TsfDft::
tsfKeep(const TSF_INPUT_STAT_T& rInputStat, const ILscTbl& rTblPrior, ILscTbl& rOutputTbl)
{
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
    MUINT32 u4Ratio = rInputStat.u4Ratio;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d)", m_u4SensorDev, rInputStat.u4FrmId, u4Ratio);

    //keep table
    rOutputTbl = rTblPrior;//input update to Queue

    if (fgDump)
    {
        char strFile[512] = {'\0'};
        MUINT32 u4ProcCnt = m_u4ProcCnt;
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_keep.bin", u4ProcCnt, rInputStat.u4FrmId);
        rInputStat.dump(strFile);
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfPrior_keep.tbl", u4ProcCnt, rInputStat.u4FrmId);
        rTblPrior.dump(strFile);
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_keep.tbl", u4ProcCnt, rInputStat.u4FrmId);
        rOutputTbl.dump(strFile);
    }

    LSC_LOG_END("Sensor(%d), #(%d)", m_u4SensorDev, rInputStat.u4FrmId);

    return MTRUE;
}

MBOOL
TsfDft::
createThread()
{
#ifdef TSF_THREAD_BUILD
    LSC_LOG_BEGIN();
    m_fgThreadLoop = MTRUE;
    m_fgThreadExec = MFALSE;
    ::pthread_mutex_init(&m_Mutex, NULL);
    ::sem_init(&m_Sema, 0, 0);
    ::sem_init(&m_SemaExecDone, 0, 0);
    ::pthread_create(&m_Thread, NULL, threadLoop, this);
    LSC_LOG_END("Create TSF m_Thread(0x%08x)\n", (MUINT32) m_Thread);
#endif
    return MTRUE;
}

MBOOL
TsfDft::
destroyThread()
{
#ifdef TSF_THREAD_BUILD
    LSC_LOG_BEGIN("sensor(%d)", m_u4SensorDev);

    TSF_LOCK();
    m_fgThreadLoop = MFALSE;
    if (m_fgThreadExec)
    {
        LSC_LOG("Post m_SemaExecDone");
        ::sem_post(&m_SemaExecDone);
        m_fgThreadExec = MFALSE;
    }
    TSF_UNLOCK();
    ::sem_post(&m_Sema);
    ::pthread_join(m_Thread, NULL);
    ::sem_destroy(&m_Sema);
    ::sem_destroy(&m_SemaExecDone);
    ::pthread_mutex_destroy(&m_Mutex);

    LSC_LOG_END();
#endif
    return MTRUE;
}

MVOID
TsfDft::
changeThreadSetting()
{
#ifdef TSF_THREAD_BUILD
    // (1) set name
    ::prctl(PR_SET_NAME, "F858THREAD", 0, 0, 0);

    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_F858_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_TSF;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    LSC_LOG(
        "sensor(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)"
        , m_u4SensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );

#endif
}

MBOOL
TsfDft::
waitDone()
{
#ifdef TSF_THREAD_BUILD
    if (m_fgThreadExec)
    {
        if(m_u4LogEn)
        {
            LSC_LOG_BEGIN();
        }
        ::sem_wait(&m_SemaExecDone);
        m_fgThreadExec = MFALSE;
        if(m_u4LogEn)
        {
            LSC_LOG_END();
        }
    }
#endif
    return MTRUE;
}

MVOID
TsfDft::
doThreadFunc()
{
    LSC_LOG_BEGIN();

    while (m_fgThreadLoop)
    {
        ::sem_wait(&m_Sema);
        if (!m_fgThreadLoop)
            break;
        TSF_LOCK();
        MBOOL fgOK = MFALSE;
        MUINT32 u4CurStep = m_u4PerFrameStep;
        MUINT32 u4TblIdx = (m_u4TblIdxCmd & RING_TBL_MSK);
        const TSF_INPUT_STAT_T& rStat = m_rStat[u4TblIdx];
        const ILscTbl& rInputTbl = m_rLscRingTbl[u4TblIdx];
        const ILscTbl& rPriorTbl = m_rLscRingTbl[(u4TblIdx+2)&RING_TBL_MSK];
        ILscTbl& rOutputTbl = m_rLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK];
        AAA_TRACE_D("TSF_RUN(#%d,%d)", rStat.u4FrmId, u4TblIdx);
        if (E_LSC_TSF_TYPE_0 == getType())
        {
            rOutputTbl.editRsvdData().u4HwRto = rStat.u4Ratio;
            NSIspTuningv3::ISP_MGR_LSC_T::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev)).putRatio(rStat.u4Ratio);
        }
        if (u4CurStep < TSF_STEP_COUNT)
        {
            ::sem_post(&m_SemaExecDone);
        }
        fgOK = tsfRun(rStat, rInputTbl, rPriorTbl, rOutputTbl);
        m_u4ProcCnt++;
        if (u4CurStep >= TSF_STEP_COUNT)
        {
            ::sem_post(&m_SemaExecDone);
        }
        AAA_TRACE_END_D;
        TSF_UNLOCK();
    }

    LSC_LOG_END();
}

MVOID*
TsfDft::
threadLoop(void* arg)
{
#ifdef TSF_THREAD_BUILD
    TsfDft* _this = reinterpret_cast<TsfDft*>(arg);

    _this->changeThreadSetting();

    _this->doThreadFunc();

#endif
    return NULL;
}

const MVOID*
TsfDft::
getRsvdData() const
{
    return &m_rTsfResult.ExifData[0];
}

inline static void setDebugTag(SHADING_DEBUG_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_SHADING_MODULE_ID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MBOOL
TsfDft::
setDumpFileCfg(MBOOL fgSave, const char* pathname)
{
    m_bDump = fgSave;
    m_strDumpFile = pathname;
    LSC_LOG("dump(%d), path(%s)", fgSave, pathname);
    return MTRUE;
}

MBOOL
TsfDft::
dumpData()
{
    if (m_bDump)
    {
        MINT32 i4Type = getType();
        char strFile[512] = {'\0'};
        MUINT32 u4ProcCnt = m_u4ProcCntLatest;
        MUINT32 u4TblIdx = m_u4TblIdxCmd & RING_TBL_MSK;
        LSC_LOG("Type(%d) procCnt(%d), idx(%d), path(%s)", i4Type, u4ProcCnt, u4TblIdx, m_strDumpFile.c_str());
        sprintf(strFile, "%s.t%d_%06d_tsfInStat.bin", m_strDumpFile.c_str(), i4Type, u4ProcCnt);
        m_rStat[u4TblIdx].dump(strFile);
        sprintf(strFile, "%s.t%d_%06d_tsfInput.tbl", m_strDumpFile.c_str(), i4Type, u4ProcCnt);
        m_rLscRingTbl[u4TblIdx].dump(strFile);
        sprintf(strFile, "%s.t%d_%06d_tsfOutput.tbl", m_strDumpFile.c_str(), i4Type, u4ProcCnt);
        m_rLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK].dump(strFile);
    }
    return MTRUE;
}

MBOOL
TsfDft::
setDebugInfo(MVOID* pDbgInfo)
{
    if (pDbgInfo == NULL)
    {
        LSC_ERR("NULL pDbgInfo!");
        return MFALSE;
    }

    //V2
    SHADING_DEBUG_INFO_T& rShadingDbgInfo = *reinterpret_cast<SHADING_DEBUG_INFO_T*>(pDbgInfo);
    MUINT32 rInputTblRto = m_rLscRingTbl[(m_u4TblIdx + RING_TBL_MSK) & RING_TBL_MSK].getRsvdData().u4HwRto;
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN,       (MUINT32)(rInputTblRto<<16) | m_pLsc->getOnOff());
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_STATW,        (MUINT32)m_rTsfEnvInfo.ImgWidth);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_STATH,        (MUINT32)m_rTsfEnvInfo.ImgHeight);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_BORDER,       (MUINT32)m_rTsfEnvInfo.BayerOrder);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_ORIEN,        (MUINT32)m_rTsfEnvInfo.afn);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_FULLW,        (MUINT32)m_rTsfEnvInfo.SensorCrop.full_width);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_FULLH,        (MUINT32)m_rTsfEnvInfo.SensorCrop.full_height);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_REW,          (MUINT32)m_rTsfEnvInfo.SensorCrop.resize_width);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_REH,          (MUINT32)m_rTsfEnvInfo.SensorCrop.resize_height);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CRW,          (MUINT32)m_rTsfEnvInfo.SensorCrop.crop_width);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CRH,          (MUINT32)m_rTsfEnvInfo.SensorCrop.crop_height);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OFFW,         (MUINT32)m_rTsfEnvInfo.SensorCrop.crop_hor_offs);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OFFH,         (MUINT32)m_rTsfEnvInfo.SensorCrop.crop_ver_offs);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_BORDER,   (MUINT32)m_u1OtpPixId);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GX,       (MUINT32)m_u1OtpGridX);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GY,       (MUINT32)m_u1OtpGridY);

    LSC_RESVERED_STRUCT resvered_buffer;
    MTKLscUtil::LscVirtualChannel(&resvered_buffer);
    MINT32 i;
    for (i = SHAD_TAG_CAM_LSC_CP_0; i <= SHAD_TAG_CAM_LSC_CP_7; i++)
    {
        setDebugTag(rShadingDbgInfo, i, resvered_buffer.buffer[ i - SHAD_TAG_CAM_LSC_CP_0 ] );
    }

    const MINT32* pTsfExif = &m_rTsfResult.ExifData[0];
    for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
    {
        setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
    }

    dumpData();

    return MTRUE;
}

MBOOL
TsfDft::
setOnOff(MBOOL fgOnOff)
{
#if defined(TSF_BUILD)
    LSC_LOG("(%d)", fgOnOff);
    m_bTSF = fgOnOff;
    return MTRUE;
#else
    m_bTSF = MFALSE;
    return MFALSE;
#endif
}

MBOOL
TsfDft::
getOnOff() const
{
    return m_bTSF;
}

MBOOL
TsfDft::
setConfig(ESensorMode_T eSensorMode, MUINT32 /*u4W*/, MUINT32 /*u4H*/, MBOOL fgForce)
{
#if defined(TSF_BUILD) && !defined(LSC_DBG)
    LSC_LOG_BEGIN();

    TSF_LOCK();

    ESensorMode_T ePrevSensorMode = m_eSensorMode;

    ILscTable::Config rTblCfg = m_pLsc->getLut(2)->getConfig();
    LSC_LOG("Sensor(%d), Scn(%d), WxH(%dx%d), Grid(%dx%d), force(%d)",
        m_u4SensorDev, eSensorMode, rTblCfg.i4ImgWd, rTblCfg.i4ImgHt, rTblCfg.i4GridX, rTblCfg.i4GridY, fgForce);

    ILscTable::TransformCfg_T rTransCfg = m_pLsc->getCurTrfmCfg();

    float fCurCropRtoX = (float)m_rTransformCfg.u4X / (float)m_rTransformCfg.u4ResizeW;
    float fCurCropRtoY = (float)m_rTransformCfg.u4Y / (float)m_rTransformCfg.u4ResizeH;

    float fCropRtoX = (float)rTransCfg.u4X / (float)rTransCfg.u4ResizeW;
    float fCropRtoY = (float)rTransCfg.u4Y / (float)rTransCfg.u4ResizeH;

    MBOOL fgBlkChg =
        (m_rTsfLscParam.x_grid_num != rTblCfg.i4GridX)||
        (m_rTsfLscParam.y_grid_num != rTblCfg.i4GridY);

    float fRto = (((float)m_rTsfLscParam.raw_wd*rTblCfg.i4ImgHt) / ((float)m_rTsfLscParam.raw_ht*rTblCfg.i4ImgWd));
    float fMax = 1.0f + (1.0f/80.0f);
    float fMin = 1.0f - (1.0f/80.0f);

    LSC_LOG("cur crop ratio (x, y): (%.2f, %.2f), to crop ratio (x, y): (%.2f, %.2f)", fCurCropRtoX, fCurCropRtoY, fCropRtoX, fCropRtoY);

    MBOOL fgAspectChg = fRto > fMax || fRto < fMin || std::fabs(float(fCurCropRtoX-fCropRtoX))>0.1 || std::fabs(float(fCurCropRtoY-fCropRtoY))>0.1;
    //MBOOL fgAspectChg = (m_rTsfLscParam.raw_wd*u4H != m_rTsfLscParam.raw_ht*u4W);

    MINT32 i = 0;
    // config table
    m_rTsfLscParam.raw_wd        = rTblCfg.i4ImgWd;
    m_rTsfLscParam.raw_ht        = rTblCfg.i4ImgHt;
    m_rTsfLscParam.x_offset      = 0;
    m_rTsfLscParam.y_offset      = 0;
    m_rTsfLscParam.block_wd      = rTblCfg.rCfgBlk.i4BlkW;
    m_rTsfLscParam.block_ht      = rTblCfg.rCfgBlk.i4BlkH;
    m_rTsfLscParam.x_grid_num    = rTblCfg.i4GridX;
    m_rTsfLscParam.y_grid_num    = rTblCfg.i4GridY;
    m_rTsfLscParam.block_wd_last = rTblCfg.rCfgBlk.i4BlkLastW;
    m_rTsfLscParam.block_ht_last = rTblCfg.rCfgBlk.i4BlkLastH;

    LSC_LOG("raw_wd(%d), raw_ht(%d), block_wd(%d), block_ht(%d), xgrid(%d), ygrid(%d), wd_last(%d), ht_last(%d)",
        m_rTsfLscParam.raw_wd,
        m_rTsfLscParam.raw_ht,
        m_rTsfLscParam.block_wd,
        m_rTsfLscParam.block_ht,
        m_rTsfLscParam.x_grid_num,
        m_rTsfLscParam.y_grid_num,
        m_rTsfLscParam.block_wd_last,
        m_rTsfLscParam.block_ht_last);
    m_eSensorMode = eSensorMode;

    MBOOL fgCfgForceReset = 0;  /* reset table with high ct table, i.e. use high ct as initial */
    MBOOL fgCfgKeep = 0;        /* keep using previous table as initial one to continue */
    MBOOL fgCfgReform = 0;      /* crop previous table to the target sensor mode as initial (big to small) */

    switch (m_u1CfgOpt)
    {
    default:
    case 0:
        fgCfgForceReset = (!m_bTSFInstanced) || fgBlkChg || fgForce;
        fgCfgKeep       = !m_bTSFInit || fgAspectChg;
        fgCfgReform     = 0;
        break;
    case 1:
        fgCfgForceReset = (!m_bTSFInstanced) || fgBlkChg || fgForce;
        fgCfgKeep       = !m_bTSFInit;
        fgCfgReform     = fgAspectChg;
        break;
    case 2:
        fgCfgForceReset = (!m_bTSFInstanced) || fgBlkChg || fgAspectChg || fgForce;
        fgCfgKeep       = !m_bTSFInit;
        fgCfgReform     = 0;
        break;
    }
    //
    if (fgCfgForceReset)
    {
        LSC_LOG("Need to reset table, eSensorMode(%d), instance(%d), init(%d), fgBlkChg(%d), fgAspectChg(%d), fgForce(%d)",
            eSensorMode, m_bTSFInstanced, m_bTSFInit, fgBlkChg, fgAspectChg, fgForce);
        m_rTransformCfg = m_pLsc->getCurTrfmCfg();
        tsfResetTbl(eSensorMode);
        tsfInit();
        tsfCfgChg();
        m_bTSFInstanced = MTRUE;
        m_bTSFInit = MTRUE;
    }
    else if (fgCfgReform) // reorder fgCfgReform/fgCfgKeep priority
    {
        MBOOL fgCrop = tsfReformTbl();
        LSC_LOG("FOV change, eSensorMode(%d->%d), instance(%d), init(%d), fgBlkChg(%d), fgAspectChg(%d,%d), fgForce(%d)",
            ePrevSensorMode, eSensorMode, m_bTSFInstanced, m_bTSFInit, fgBlkChg, fgAspectChg, fgCrop, fgForce);
        if(fgCrop == MFALSE)
            tsfResetTbl(eSensorMode);
        tsfInit();
        tsfCfgChg();
    }
    else if (fgCfgKeep)
    {
        LSC_LOG("Reinit but no reset table, eSensorMode(%d), instance(%d), init(%d), fgBlkChg(%d), fgAspectChg(%d), fgForce(%d)",
            eSensorMode, m_bTSFInstanced, m_bTSFInit, fgBlkChg, fgAspectChg, fgForce);
        m_rTransformCfg = m_pLsc->getCurTrfmCfg();
        tsfInit();
        tsfCfgChg();
        m_bTSFInstanced = MTRUE;
        m_bTSFInit = MTRUE;
    }
    else
    {
        tsfCfgChg();
        LSC_LOG("No need to reset table");
    }

    m_u4PerFrameStep = 0;

    TSF_UNLOCK();
    LSC_LOG_END();
#endif

    return MTRUE;
}

MBOOL
TsfDft::
updateRatio(MUINT32 u4MagicNum, MUINT32 u4Rto)
{
    TSF_LOCK();
    MUINT32 i = 0;
    if (E_LSC_TSF_TYPE_0 == getType())
    {
        for (i = 0; i < RING_TBL_NUM; i++)
        {
            if (m_rStat[i].u4FrmReq == u4MagicNum)//result magic
            {
                m_rLscRingTbl[i].editRsvdData().u4HwRto = u4Rto;
                break;
            }
        }
    }
    TSF_UNLOCK();
    return MTRUE;
}

MBOOL
TsfDft::
getTbl(MUINT32 u4GridX, MUINT32 u4GridY, MUINT32 u4MagicNum, ILscTbl& output)
{
    TSF_LOCK();
    MBOOL fgRet = MFALSE;
    MUINT32 i = 0;
    for (i = 0; i < RING_TBL_NUM; i++)
    {
        if (m_rStat[i].u4FrmReq == u4MagicNum)//result magic
        {
            MUINT32 width = m_rLscRingTbl[i].getConfig().i4ImgWd;
            MUINT32 height = m_rLscRingTbl[i].getConfig().i4ImgHt;
            MUINT32 blk_width  = width / (u4GridX-1);
            MUINT32 blk_height = height / (u4GridY-1);
            if (m_rLscRingTbl[i].cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, (blk_width>>1), (blk_height>>1), width - blk_width, height - blk_height), output))
            {
                fgRet = MTRUE;
            }
            else
            {
                LSC_ERR("Fail to crop matching LSC Gain Table.");
            }

            break;
        }
    }
    if (!fgRet) // Matching table was not found
    {
        MUINT32 width = m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].getConfig().i4ImgWd;
        MUINT32 height = m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].getConfig().i4ImgHt;
        if (m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, 0, 0, width, height), output))
        {
            fgRet = MTRUE;
        }
        else
        {
            LSC_ERR("Fail to crop matching LSC Gain Table.");
        }
    }
    TSF_UNLOCK();
    return fgRet;
}

MBOOL
TsfDft::
update(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo)
{
    TSF_LOCK();
    if (m_bTSFInit && m_bTSF)
    {
        MUINT32 u4TblIdx = (m_u4TblIdx & RING_TBL_MSK);
        TSF_INPUT_STAT_T& rStat = m_rStat[u4TblIdx];
        AAA_TRACE_G(TSF_AAO);
        AAA_TRACE_NL(TSF_AAO);
        MUINT32 u4DesiredRto = m_pLsc->getRatio();
        AAA_TRACE_END_NL;
        AAA_TRACE_END_G;

        // AWB stat
        AAA_TRACE_D("TSF_AAO(%d)", rInputInfo.u4FrmId);
        rStat.u4FrmId = rInputInfo.u4FrmId;
        rStat.u4FrmReq = rInputInfo.u4FrmReq;
        rStat.rAwbInfo = rInputInfo.rAwbInfo;
        rStat.u4BlkNumX = m_rTsfEnvInfo.ImgWidth;
        rStat.u4BlkNumY = m_rTsfEnvInfo.ImgHeight;
        rStat.u4Ratio = u4DesiredRto;
        rStat.put(rInputInfo, m_fgStatOpt);
        AAA_TRACE_END_D;

        m_u4ProcCntLatest = m_u4ProcCnt;
        m_u4TblIdxCmd = u4TblIdx;

    #ifdef TSF_THREAD_BUILD
        if (ILscMgr::E_TSF_CMD_RUN == rInputInfo.eCmd)
        {
            m_fgThreadExec = MTRUE;
            ::sem_post(&m_Sema);
        }
    #endif
        else
        {
            AAA_TRACE_D("TSF(%d,#%d)", rInputInfo.eCmd, rInputInfo.u4FrmId);
            MBOOL fgOK = MFALSE;
            const ILscTbl& rInputTbl = m_rLscRingTbl[u4TblIdx];
            const ILscTbl& rPriorTbl = m_rLscRingTbl[(u4TblIdx+2)&RING_TBL_MSK];
            ILscTbl& rOutputTbl = m_rLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK];
            switch (rInputInfo.eCmd)
            {
            default:
            case ILscMgr::E_TSF_CMD_BATCH:
                fgOK = tsfBatch(rStat, rInputTbl, rOutputTbl);
                break;
            case ILscMgr::E_TSF_CMD_BATCH_CAP:
#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
                if (rStat.rAwbInfo.m_FLASH_ON)
                {
                    MRESULT ret = S_TSF_OK;
                    MTK_TSF_TBL_STRUCT rTsfTbl;
                    rTsfTbl.pLscConfig = &m_rTsfLscParam;
                    rTsfTbl.ShadingTbl = (MINT32*)m_rFlashOnTbl.getData();
                    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &rTsfTbl, NULL);
                    if (ret != S_TSF_OK)
                    {
                        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
                        m_prTsf->TsfReset();
                    }
                    fgOK = tsfBatchCap(rStat, rInputTbl, rOutputTbl);
                    rTsfTbl.ShadingTbl = (MINT32*)m_rLscBaseTbl.getData();
                    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &rTsfTbl, NULL);
                    if (ret != S_TSF_OK)
                    {
                        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
                        m_prTsf->TsfReset();
                    }
                }
                else
                {
                    fgOK = tsfBatchCap(rStat, rInputTbl, rOutputTbl);
                }
#else
                fgOK = tsfBatchCap(rStat, rInputTbl, rOutputTbl);
#endif
                break;
            case ILscMgr::E_TSF_CMD_RUN:
                fgOK = tsfRun(rStat, rInputTbl, rPriorTbl, rOutputTbl);
                break;
            case ILscMgr::E_TSF_CMD_KEEP:
                fgOK = tsfKeep(rStat, rPriorTbl, rOutputTbl);
                break;
            }
            if (E_LSC_TSF_TYPE_0 == getType())
            {
                rOutputTbl.editRsvdData().u4HwRto = u4DesiredRto;
                if (fgOK)
                {
                    tsfSetTbl(rOutputTbl);
                }
                else
                {
                    LSC_ERR("TSF Error(%d,%d,%d,#%d)", rInputInfo.eCmd, rStat.u4Ratio, u4DesiredRto, rInputInfo.u4FrmId);
                }
            }
            m_u4ProcCnt++;
            AAA_TRACE_END_D;
        }
        // increment table index
        m_u4TblIdx = ((m_u4TblIdx+1) & RING_TBL_MSK);
    }
    TSF_UNLOCK();
    return MTRUE;
}


/*******************************************************************************
* doNotifyCb
*******************************************************************************/

void TsfDft::doNotifyCb(MINT32  _msgType)
{
    switch (_msgType)
    {
        case eID_NOTIFY_TSF_IN:
            if (m_pTsfIn)
            {
                m_pTsfIn->validate();
            }
            else
            {
                LSC_ERR("Cache memory flush failed. TSF_IN buffer is Null.");
            }
            break;
        case eID_NOTIFY_TSF_OUT:
            if (m_pTsfOut)
            {
                m_pTsfOut->validate();
            }
            else
            {
                LSC_ERR("Cache memory flush failed. TSF_OUT buffer is Null.");
            }
            break;
        default:
            LSC_ERR("Cache memory flush failed. Incorrect _msgType: %d.", _msgType);
            break;
    }
}

