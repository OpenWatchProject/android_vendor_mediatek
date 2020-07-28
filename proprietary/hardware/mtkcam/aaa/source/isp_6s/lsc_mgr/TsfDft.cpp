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
put(const ILscMgr::TSF_INPUT_INFO_T& rInStat, MUINT32 StatFullSize, MUINT8 /*fgOpt*/)
{
    MBOOL fgRet = MTRUE;

    // AWB info
    u4FrmId = rInStat.u4FrmId;
    u4FrmReq = rInStat.u4FrmReq;
    rAwbInfo = rInStat.rAwbInfo;
    //u4Ratio = m_pLsc->getRatio();

    if(rInStat.prTSFStat)
    {
        vecStat.resize(StatFullSize/sizeof(MUINT8));
        ::memcpy(vecStat.data(), rInStat.prTSFStat, StatFullSize);
    }
    return fgRet;
}

/*******************************************************************************
 * TsfDft
 *******************************************************************************/
void
TsfDft::
destroyInstance()
{}

TsfDft::
TsfDft(MUINT32 u4SensorDev)
    : m_u4SensorDev(u4SensorDev)
    , m_eSensorMode(ESensorMode_Capture)
    , m_i4LogEn(0)
    , m_bTSF(MFALSE)
    , m_bTSFInstanced(MFALSE)
    , m_i4CfgOpt(0)
    , m_fgStatOpt(0x7)
    , m_u4TblIdx(0)
    , m_u4TblIdxRear(0)
    , m_u4ProcCnt(0)
    , m_pLsc(NULL)
    , m_bDump(MFALSE)
    , m_bReInitTsf(MTRUE)
    , m_bForceApply(MFALSE)
    , m_u1OtpPixId(0)
    , m_u1OtpGridX(0)
    , m_u1OtpGridY(0)
{
    LSC_LOG("Enter Type 0: Default Cycle");
    for (MINT32 i = 0; i < RING_TBL_NUM; i++)
        m_rStat[i].u4Ratio = 32;

    for (MINT32 i = 0; i < RING_TBL_NUM; i++)
    {
        char strName[32];
        sprintf(strName, "TSF_RingBuf_%d", i);

        MUINT32 id = 0xFFFFFFFD - i;           /*m_pLsc->m_rBufPool.size()*/
        ILscBuf* pBuf = new ILscBuf(m_u4SensorDev, id, MTRUE, strName, ILscBuf::E_LSC_IMEM);
        if (!pBuf)
        {
            LSC_ERR("Fail to create buffer %s", strName);
        } else {
            LSC_LOG("OK: pBuf(%p, %s), virtual addr %p", pBuf, pBuf->getName(), pBuf->editTable());
            m_pTsfRingBuf[i] = pBuf;
        }
    }
}

TsfDft::
~TsfDft()
{
    LSC_LOG("Exit Type 0: Default Cycle");
}

MBOOL
TsfDft::
init()
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

    GET_PROP("vendor.debug.lsc_mgr.log", 0, m_i4LogEn);
    // Default is 0. Change to 1 => Because Aspect ratio change, it need to reform table.
    GET_PROP("vendor.debug.lsc_mgr.cfg", 1, m_i4CfgOpt);

    m_pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_u4SensorDev));
    m_pLsc->getRawSize(ESensorMode_Capture, m_u4FullW, m_u4FullH);

    createTsf();
    createThread();

    // Start to create tsf_in / tsf_out

    MUINT32 id = 0xFFFFFFFF;           /*m_pLsc->m_rBufPool.size()*/
    char strName[32];
    sprintf(strName, "TSF_in");
    m_pTsfInDump = new ILscBuf(m_u4SensorDev, id, MFALSE, strName, ILscBuf::E_LSC_IMEM);
    if (!m_pTsfInDump)
    {
        LSC_ERR("Fail to create m_pTsfInDump buffer.");
    }

#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
    LSC_LOG("CT_3 Shading Table will be used in casees when Flash is ON.");
    const ILscTbl* pFlashOnTbl = m_pLsc->getCapLut(3);
    m_rFlashOnTbl = *pFlashOnTbl;
    m_rFlashOnTbl.editRsvdData().u4HwRto = 32;
#endif

    m_bReInitTsf=MTRUE;
    m_bForceApply =MFALSE;

    // End to create tsf_in / tsf_out

    /*LSC_ERR("Tbl addr");
    for(int i=0;i<4;i++)
    {
        LSC_ERR("i: %d, %p", i, m_pTsfRingBuf[i]);
    }

    LSC_ERR("Tbl ptr addr");
    for(int i=0;i<4;i++)
    {
        LSC_ERR("i: %d, %p", i, m_ptrTsfRingBuf[i]);
    }*/

    m_isTsfInit = MTRUE;

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);
    return MTRUE;
}

MBOOL
TsfDft::
uninit()
{
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);
    m_bDump = MFALSE;
    if (m_pTsfInDump)
    {
        delete m_pTsfInDump;
        m_pTsfInDump = NULL;
    }
    //m_rTsfEnvInfo.pCb = NULL;
    destroyThread();
    destroyTsf();

    m_isTsfInit = MFALSE;

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
    const ILscTbl* pBaseLsc = (!m_bIsFactory)?m_pLsc->getLut(2):m_pLsc->getLut(3);
    if (pBaseLsc)
    {
        m_rLscBaseTbl = *pBaseLsc;

        MINT32 i;
        for (i = 0; i < RING_TBL_NUM; i++)
        {
            //m_rLscRingTbl[i] = m_rLscBaseTbl;
            //m_ptrLscRingTbl[i] = &m_rLscRingTbl[0];
            m_rLscBaseTbl.toBuf(*m_pTsfRingBuf[i]);
            m_ptrTsfRingBuf[i]=m_pTsfRingBuf[0];
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
        // reset base table
        const ILscTbl* pBaseLsc = (!m_bIsFactory)?m_pLsc->getLut(2):m_pLsc->getLut(3);
        if(!pBaseLsc)
        {
            LSC_ERR("Base Tbl is NULL");
            return MFALSE;
        }
        m_rLscBaseTbl = *pBaseLsc;

        // FOVA >= FOVB
        ILscTable::TransformCfg_T rTrfmRel = rTrfmB;
        rTrfmRel.u4X = rTrfmB.u4X - rTrfmA.u4X;
        rTrfmRel.u4Y = rTrfmB.u4Y - rTrfmA.u4Y;

        MINT32 i;
        for (i = 0; i < RING_TBL_NUM; i++)
        {
            //ILscTbl rTbl = m_rLscRingTbl[i];
            ILscTbl rTbl;
            ILscTbl rTblOut;
            rTbl.setConfig(rTrfmA.u4W, rTrfmA.u4H, rTrfmA.u4GridX, rTrfmA.u4GridY);
            rTbl.setData(m_pTsfRingBuf[i]->getTable(),m_pTsfRingBuf[i]->getSize());
            if (!rTbl.cropOut(rTrfmRel, rTblOut)) //Fix Coverity
            {
                LSC_ERR("Fail to crop matching LSC Gain Table.");
                return MFALSE;
            }
            rTblOut.toBuf(*m_pTsfRingBuf[i]);
        }
        return MTRUE;
    }
    else
    {
        m_bForceApply=MTRUE;
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
tsfSetBuf(ILscBuf* rBuf)
{
    return m_pLsc->syncBuf(rBuf, ILscMgr::LSC_P1);
}

MBOOL
TsfDft::
tsfSetBufAndRatio(ILscBuf** rBuf, const TSF_INPUT_STAT_T &rStat)
{
    MBOOL ret =
        (m_pLsc->syncBufAndRatio(rBuf, rStat.u4Ratio, ILscMgr::LSC_P1)) &&
        (m_pLsc->syncTSFO(rStat.vecStat))
        ;

    return ret;
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
        // Chang SL2 to frontal bining
        u4TgW=1;u4TgH=1;u4BinW=1;u4BinH=1;
        rSl2Cfg.i4CenterX = u4Rx * u4TgW / u4BinW;
        rSl2Cfg.i4CenterY = u4Ry * u4TgH / u4BinH;
        rSl2Cfg.i4R0      = u4R * (3*u4TgW) / (10*u4BinW);
        rSl2Cfg.i4R1      = u4R * (6*u4TgW) / (10*u4BinW);
        rSl2Cfg.i4R2      = u4R * (8*u4TgW) / (10*u4BinW);


        if(rSL2.SL2_GAIN_1 && rSL2.SL2_GAIN_2 && rSL2.SL2_GAIN_3 && rSL2.SL2_GAIN_4)
        {
            rSl2Cfg.i4Gain0   = rSL2.SL2_GAIN_0;
            rSl2Cfg.i4Gain1   = rSL2.SL2_GAIN_1;
            rSl2Cfg.i4Gain2   = rSL2.SL2_GAIN_2;
            rSl2Cfg.i4Gain3   = rSL2.SL2_GAIN_3;
            rSl2Cfg.i4Gain4   = rSL2.SL2_GAIN_4;
            rSl2Cfg.i4SetZero = rSL2.SL2_SET_ZERO;

            CAM_LOGD("SLK from ALG is (Gain0, 1, 2, 3, 4), z = (%d, %d, %d, %d, %d), %d",
            rSL2.SL2_GAIN_0, rSL2.SL2_GAIN_1, rSL2.SL2_GAIN_2, rSL2.SL2_GAIN_3, rSL2.SL2_GAIN_4, rSL2.SL2_SET_ZERO);
        } else {
            rSl2Cfg.i4Gain0   = 0;
            rSl2Cfg.i4Gain1   = 64;
            rSl2Cfg.i4Gain2   = 128;
            rSl2Cfg.i4Gain3   = 192;
            rSl2Cfg.i4Gain4   = 255;
            rSl2Cfg.i4SetZero = 0;

            CAM_LOGD("set SLK from default value");
        }

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
    m_rTsfEnvInfo.Bitdepth = 16;
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

    m_rTsfEnvInfo.StatStrideSize = m_blockW*12;
    m_rTsfEnvInfo.StatFullSize = m_blockW*12*m_blockH;

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

    MINT32 i4TsfSL2En = 0;
    GET_PROP("vendor.debug.lsc_mgr.sl2", -1, i4TsfSL2En);

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

    LSC_LOG("BaseTbl, ImgWd(%d), ImgHt(%d), Size(%d), GridX(%d), GridY(%d), BlkX(%d), BlkY(%d), BlkW(%d), BlkH(%d), LastW(%d), LastH(%d), Type(%d), Bayer(%d)\n",
                      m_rLscBaseTbl.getConfig().i4ImgWd, m_rLscBaseTbl.getConfig().i4ImgHt, m_rLscBaseTbl.getSize(), m_rLscBaseTbl.getConfig().i4GridX, m_rLscBaseTbl.getConfig().i4GridY,
                      m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkX, m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkY, m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkW, m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkH,
                      m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkLastW, m_rLscBaseTbl.getConfig().rCfgBlk.i4BlkLastH, m_rLscBaseTbl.getType(), m_rLscBaseTbl.getBayer());

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
convert(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo) const
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_SET_PROC) ? MTRUE : MFALSE;
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
    rProcInfo.ShadingTbl = (MINT32 *)rInputBuf->getTable();
    rProcInfo.ImgAddr = (MUINT8*)rInputStat.vecStat.data();
    rProcInfo.LscRA = rInputBuf->getRatio();

    if(fgLogEn) {
        ILscTbl rInputTbl;
        rInputTbl.setConfig(m_rLscBaseTbl.getConfig());
        rInputTbl.setRsvdData(rInputBuf->getRatio());
        rInputTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());

        LSC_LOG("[%s] InputTbl, ImgWd(%d), ImgHt(%d), Size(%d), GridX(%d), GridY(%d), BlkX(%d), BlkY(%d), BlkW(%d), BlkH(%d), LastW(%d), LastH(%d), Type(%d), Bayer(%d)\n",
            __FUNCTION__, rInputTbl.getConfig().i4ImgWd, rInputTbl.getConfig().i4ImgHt, rInputTbl.getSize(), rInputTbl.getConfig().i4GridX, rInputTbl.getConfig().i4GridY,
                      rInputTbl.getConfig().rCfgBlk.i4BlkX, rInputTbl.getConfig().rCfgBlk.i4BlkY, rInputTbl.getConfig().rCfgBlk.i4BlkW, rInputTbl.getConfig().rCfgBlk.i4BlkH,
                      rInputTbl.getConfig().rCfgBlk.i4BlkLastW, rInputTbl.getConfig().rCfgBlk.i4BlkLastH, rInputTbl.getType(), rInputTbl.getBayer());
    }
    CAM_LOGD_IF(fgLogEn, "[%s] L(%d), C(%d), F(%d), DF(%d), R(%d), G(%d), B(%d) Rto(%d)\n",
        __FUNCTION__, rProcInfo.ParaL, rProcInfo.ParaC, rProcInfo.FLUO_IDX, rProcInfo.DAY_FLUO_IDX, rProcInfo.Gain.i4R, rProcInfo.Gain.i4G, rProcInfo.Gain.i4B, rProcInfo.LscRA);

    return MTRUE;
}

MBOOL
TsfDft::
tsfSetProcInfo(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf)
{
    MRESULT ret = S_TSF_OK;
    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputBuf, rProcInfo);
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
tsfGetResult(ILscBuf* rOutputBuf)
{
    MRESULT ret = S_TSF_OK;

    m_rTsfResult.ShadingTbl = rOutputBuf->editTable();
    m_rTsfResult.u4TblSize = rOutputBuf->getSize();
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, NULL, &m_rTsfResult);
    return (ret == S_TSF_OK);
}


MBOOL
TsfDft::
tsfCfgChg()
{
    MRESULT ret = S_TSF_OK;

    //MRESULT ret = S_TSF_OK;

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
        //m_rLscRingTbl[i].setConfig(m_rLscBaseTbl.getConfig());

        ILscTbl tmpTbl;
        tmpTbl.setConfig(m_rLscBaseTbl.getConfig());
        tmpTbl.setRsvdData(m_pTsfRingBuf[i]->getRatio());
        tmpTbl.setData(m_pTsfRingBuf[i]->getTable(),m_pTsfRingBuf[i]->getSize());
        tmpTbl.toBuf(*m_pTsfRingBuf[i]);
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

    //tsfSetTbl(m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]);
    //tsfSetTbl(*m_ptrLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]);


    if(m_bReInitTsf) {
        LSC_LOG("m_bReInitTsf=TRUE, tsfSetBufAndRatio");
        tsfSetBufAndRatio(&m_pTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)], m_rStat[(m_u4TblIdx & RING_TBL_MSK)]);
    }


lbExit:
    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);

    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfSetCfgChgToHW()
{
    MRESULT ret = S_TSF_OK;
    // this is for identical sized tables, but block numbers are different.
    // no cropping support.
    // ex. 10x10 -> 16x16
    LSC_LOG_BEGIN("tsfSetCfgChgToHW Sensor(%d)", m_u4SensorDev);

    ILscTbl tmpTbl;
    tmpTbl.setConfig(m_rTsfLscParam.raw_wd, m_rTsfLscParam.raw_ht,
                     m_rTsfLscParam.x_grid_num, m_rTsfLscParam.y_grid_num);
    tmpTbl.setRsvdData(m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getRatio());
    tmpTbl.setData(m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getTable(),m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getSize());
    tmpTbl.toBuf(*m_pTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]);

    tsfSetBufAndRatio(&m_pTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)], m_rStat[(m_u4TblIdx & RING_TBL_MSK)]);
lbExit:
    LSC_LOG_END("tsfSetCfgChgToHW Sensor(%d)", m_u4SensorDev);
    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfBatch(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf* rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MRESULT ret = S_TSF_OK;
    MBOOL fgDump = m_i4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d)", m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio);

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputBuf, rProcInfo);

    m_rTsfResult.ShadingTbl = rOutputBuf->editTable();
    m_rTsfResult.u4TblSize = rOutputBuf->getSize();

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
        if(m_bForceApply)
        {
            LSC_LOG("apply shading table");
            tsfSetBufAndRatio(&rOutputBuf, rInputStat);
            m_bForceApply=MFALSE;
        }

        tsfSetSL2(m_rTsfResult.SL2Para);
        if (fgDump)
        {
            char strFile[512] = {'\0'};
            MUINT32 u4ProcCnt = m_u4ProcCnt;
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfInStat_bat.bin", u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);

            ILscTbl inTbl;
            inTbl.setConfig(m_rLscBaseTbl.getConfig());
            inTbl.setRsvdData(rInputBuf->getRatio());
            inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfInput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);
            inTbl.dump(strFile);

            ILscTbl outTbl;
            outTbl.setConfig(m_rLscBaseTbl.getConfig());
            outTbl.setRsvdData(rOutputBuf->getRatio());
            outTbl.setData(rOutputBuf->getTable(),rOutputBuf->getSize());
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfOutput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);

            char buff[16];
            snprintf(buff, sizeof(buff), "%06d_%04d", u4ProcCnt, rInputStat.u4FrmId);
            m_strDumpFileNormal = buff;

            outTbl.dump(strFile);
        }
    }

    LSC_LOG_END("Sensor(%d), #(%d)", m_u4SensorDev, rInputStat.u4FrmId);

    return ret == S_TSF_OK;
}

MBOOL
TsfDft::
tsfBatchCap(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf* rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MRESULT ret = S_TSF_OK;
    MBOOL fgDump = m_i4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d), m_bDump(%d)", m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio, m_bDump);

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputBuf, rProcInfo);

    m_rTsfResult.ShadingTbl = rOutputBuf->editTable();
    m_rTsfResult.u4TblSize = rOutputBuf->getSize();

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
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfInStat_cap.bin", u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);

            ILscTbl inTbl;
            inTbl.setConfig(m_rLscBaseTbl.getConfig());
            inTbl.setRsvdData(rInputBuf->getRatio());
            inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfInput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);
            inTbl.dump(strFile);

            ILscTbl outTbl;
            outTbl.setConfig(m_rLscBaseTbl.getConfig());
            outTbl.setRsvdData(rOutputBuf->getRatio());
            outTbl.setData(rOutputBuf->getTable(),rOutputBuf->getSize());
            sprintf(strFile, "/sdcard/camera_dump/%06d_%04d_tsfOutput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);
            outTbl.dump(strFile);

            char buff[16];
            snprintf(buff, sizeof(buff), "%06d_%04d", u4ProcCnt, rInputStat.u4FrmId);
            m_strDumpFileNormal = buff;

        }
        if (m_bDump)
        {
            char strFile[512] = {'\0'};
            MUINT32 u4ProcCnt = m_u4ProcCnt;
            sprintf(strFile, "%s.t0_%06d_%04d_tsfInStat_cap.bin", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);

            ILscTbl inTbl;
            inTbl.setConfig(m_rLscBaseTbl.getConfig());
            inTbl.setRsvdData(rInputBuf->getRatio());
            inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
            sprintf(strFile, "%s.t0_%06d_%04d_tsfInput_cap.tbl", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            inTbl.dump(strFile);

            ILscTbl outTbl;
            outTbl.setConfig(m_rLscBaseTbl.getConfig());
            outTbl.setRsvdData(rOutputBuf->getRatio());
            outTbl.setData(rOutputBuf->getTable(),rOutputBuf->getSize());
            sprintf(strFile, "%s.t0_%06d_%04d_tsfOutput_cap.tbl", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);
            outTbl.dump(strFile);

            sprintf(strFile, "%s.t0_%06d_%04d_tsfInput_cap", m_strDumpFile.c_str(), u4ProcCnt, rInputStat.u4FrmId);

            //rInputTbl.toBuf(*m_pTsfInDump);
            // SDBLK dump
            if (rInputBuf)
            {
                if (!rInputBuf->dump(strFile))
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
tsfRun(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf** rBufPrior, ILscBuf** rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MBOOL fgDump = m_i4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;
    MINT32 i4Case = 0;
    MUINT32 u4Step = m_u4PerFrameStep;

    CAM_LOGD_IF(fgLogEn, "[%s +] Sensor(%d), #(%d), ratio(%d), step(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, rInputStat.u4Ratio, u4Step);

    if (m_u4PerFrameStep == 0)
    {
        // only set proc info at the 1st frame.
        AAA_TRACE_D("TSF_SET_INFO");
        if (!tsfSetProcInfo(rInputStat, rInputBuf)) {
            AAA_TRACE_END_D;
            return MFALSE;
        }
        AAA_TRACE_END_D;
    }
    m_u4PerFrameStep ++;

    //AAA_TRACE_D("TSF_RUN_M");
    E_TSF_STATE_T eState = tsfMain();
    //AAA_TRACE_END_D;
    if (eState == E_TSF_FAIL)
    {
        i4Case = -1;
    }
    else if (eState == E_TSF_READY)
    {
        //AAA_TRACE_D("TSF_RUN2");
        //LSC_ERR("----------------TSF_READY---------------- ");
        if (tsfGetResult(*rOutputBuf))
        {
            //LSC_ERR("have a new data %d", (((MUINT32*)((*rOutputBuf)->getTable()))[3000]));
            m_u4PerFrameStep = 0;
            (*rOutputBuf)->setRatio(rInputStat.u4Ratio);
            //tsfSetBuf(*rOutputBuf);
            AAA_TRACE_D("TSF_SET_BUF");
            tsfSetBufAndRatio(rOutputBuf, rInputStat);
            AAA_TRACE_END_D;
            tsfSetSL2(m_rTsfResult.SL2Para);
            i4Case = 1;

            m_u4TblIdxRear=((m_u4TblIdxRear+1)&RING_TBL_MSK);
            //LSC_ERR("real inc m_u4TblIdxRear to: %d", m_u4TblIdxRear);
        }
        else
        {
            i4Case = -2;
        }
        //AAA_TRACE_END_D;
    }
    else
    {
        i4Case = 0;
        AAA_TRACE_D("TSF_RING_OP");
        *rOutputBuf = *rBufPrior;
        AAA_TRACE_END_D;
    }

    if (fgDump)
    {
        char strFile[512] = {'\0'};
        MUINT32 u4ProcCnt = m_u4ProcCnt;
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_%d.bin", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        rInputStat.dump(strFile);

        ILscTbl inTbl;
        inTbl.setConfig(m_rLscBaseTbl.getConfig());
        inTbl.setRsvdData(rInputBuf->getRatio());
        inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInput_%d.tbl", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        inTbl.dump(strFile);

        ILscTbl outTbl;
        outTbl.setConfig(m_rLscBaseTbl.getConfig());
        outTbl.setRsvdData((*rOutputBuf)->getRatio());
        outTbl.setData((*rOutputBuf)->getTable(),(*rOutputBuf)->getSize());
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_%d.tbl", u4ProcCnt, rInputStat.u4FrmId, u4Step);
        outTbl.dump(strFile);

        char buff[16];
        snprintf(buff, sizeof(buff), "%06d_%04d", u4ProcCnt, rInputStat.u4FrmId);
        m_strDumpFileNormal = buff;
    }

    CAM_LOGD_IF(fgLogEn, "[%s -] Sensor(%d), #(%d), step(%d), case(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, u4Step, i4Case);

    return (i4Case >= 0);
}

MBOOL
TsfDft::
tsfKeep(const TSF_INPUT_STAT_T& rInputStat, ILscBuf** rBufPrior, ILscBuf** rOutputBuf)
{
    if(rOutputBuf==NULL)
        return MFALSE;
    MBOOL fgDump = m_i4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
    MUINT32 u4Ratio = rInputStat.u4Ratio;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d)", m_u4SensorDev, rInputStat.u4FrmId, u4Ratio);

    //keep table
    *rOutputBuf = *rBufPrior;//input update to Queue

    if (fgDump)
    {
        char strFile[512] = {'\0'};
        MUINT32 u4ProcCnt = m_u4ProcCnt;
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInStat_keep.bin", u4ProcCnt, rInputStat.u4FrmId);
        rInputStat.dump(strFile);
        ILscTbl inTbl;
        inTbl.setConfig(m_rLscBaseTbl.getConfig());
        inTbl.setRsvdData((*rBufPrior)->getRatio());
        inTbl.setData((*rBufPrior)->getTable(),(*rBufPrior)->getSize());
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfPrior_keep.tbl", u4ProcCnt, rInputStat.u4FrmId);
        inTbl.dump(strFile);

        ILscTbl outTbl;
        outTbl.setConfig(m_rLscBaseTbl.getConfig());
        outTbl.setRsvdData((*rOutputBuf)->getRatio());
        outTbl.setData((*rOutputBuf)->getTable(),(*rOutputBuf)->getSize());
        sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_keep.tbl", u4ProcCnt, rInputStat.u4FrmId);
        outTbl.dump(strFile);
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
        if(m_i4LogEn)
        {
            LSC_LOG_BEGIN();
        }
        ::sem_wait(&m_SemaExecDone);
        m_fgThreadExec = MFALSE;
        if(m_i4LogEn)
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
        //const ILscTbl& rInputTbl = m_rLscRingTbl[u4TblIdx];
        //const ILscTbl& rPriorTbl = m_rLscRingTbl[(u4TblIdx+2)&RING_TBL_MSK];
        //ILscTbl& rOutputTbl = m_rLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK];

        //const ILscTbl* rInputTbl = m_ptrLscRingTbl[u4TblIdx];
        //ILscTbl* rPriorTbl = m_ptrLscRingTbl[(u4TblIdx+2)&RING_TBL_MSK];
        //int tmpRead=((m_u4TblIdxRear+1)&RING_TBL_MSK);
        //m_ptrLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK]=&m_rLscRingTbl[tmpRead];

        ILscBuf* rInputBuf = m_ptrTsfRingBuf[u4TblIdx];
        ILscBuf* rPriorBuf = m_ptrTsfRingBuf[(u4TblIdx+2)&RING_TBL_MSK];
        int tmpRead=((m_u4TblIdxRear+1)&RING_TBL_MSK);
        m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]=m_pTsfRingBuf[tmpRead];


        ILscBuf* rOutputBuf = m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK];

        //LSC_ERR("table addr in %p, pri %p, out %p", rInputBuf, rPriorBuf, rOutputBuf);

        AAA_TRACE_D("TSF_RUN");
        //AAA_TRACE_D("TSF_RUN_F");
        if (E_LSC_TSF_TYPE_0 == getType())
        {
            rOutputBuf->setRatio(rStat.u4Ratio);
            m_pLsc->setRatio(rStat.u4Ratio);
        }
        if (u4CurStep < TSF_STEP_COUNT)
        {
            ::sem_post(&m_SemaExecDone);
        }
        //AAA_TRACE_END_D;
        fgOK = tsfRun(rStat, rInputBuf, &rPriorBuf, &m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]);
        //AAA_TRACE_D("TSF_RUN_R");
        m_u4ProcCnt++;
        if (u4CurStep >= TSF_STEP_COUNT)
        {
            ::sem_post(&m_SemaExecDone);
        }
        //AAA_TRACE_END_D;
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

MVOID
TsfDft::
getDumpFileCfg(char *output)
{
    LSC_LOG("[%s] dump(%d), path(%s)", __func__, m_bDump, m_strDumpFileNormal.c_str());

    int t;
    char *ptr = output;

    if(ptr!=NULL) {
        t = snprintf(ptr, m_strDumpFileNormal.size(), "%s", m_strDumpFileNormal.c_str());
        ptr += t;
    } else {
        LSC_ERR("[%s] null string", __func__);
    }
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


        ILscTbl inTbl;
        inTbl.setConfig(m_rLscBaseTbl.getConfig());
        inTbl.setRsvdData(m_ptrTsfRingBuf[u4TblIdx]->getRatio());
        inTbl.setData(m_ptrTsfRingBuf[u4TblIdx]->getTable(),m_ptrTsfRingBuf[u4TblIdx]->getSize());
        sprintf(strFile, "%s.t%d_%06d_tsfInput.tbl", m_strDumpFile.c_str(), i4Type, u4ProcCnt);
        //m_rLscRingTbl[u4TblIdx].dump(strFile);
        inTbl.dump(strFile);

        ILscTbl outTbl;
        outTbl.setConfig(m_rLscBaseTbl.getConfig());
        outTbl.setRsvdData(m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]->getRatio());
        outTbl.setData(m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]->getTable(),m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]->getSize());
        sprintf(strFile, "%s.t%d_%06d_tsfOutput.tbl", m_strDumpFile.c_str(), i4Type, u4ProcCnt);
        //m_rLscRingTbl[(u4TblIdx+3)&RING_TBL_MSK].dump(strFile);
        outTbl.dump(strFile);
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
    //MUINT32 rInputTblRto = m_rLscRingTbl[(m_u4TblIdx + RING_TBL_MSK) & RING_TBL_MSK].getRsvdData().u4HwRto;
    //MUINT32 rInputTblRto = (m_ptrLscRingTbl[(m_u4TblIdx + RING_TBL_MSK) & RING_TBL_MSK]->getRsvdData()).u4HwRto;
    MUINT32 rInputTblRto = m_ptrTsfRingBuf[(m_u4TblIdx + RING_TBL_MSK) & RING_TBL_MSK]->getRatio();
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
setIsFactory(MBOOL fgOnOff)
{
    m_bIsFactory=fgOnOff;
    return MTRUE;
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

    ILscTable::Config rTblCfg = (!m_bIsFactory)?m_pLsc->getLut(2)->getConfig():m_pLsc->getLut(3)->getConfig();
    LSC_LOG("Sensor(%d), Scn(%d), WxH(%dx%d), Grid(%dx%d), force(%d), m_bReInitTsf(%d)",
            m_u4SensorDev, eSensorMode, rTblCfg.i4ImgWd, rTblCfg.i4ImgHt, rTblCfg.i4GridX, rTblCfg.i4GridY, fgForce, m_bReInitTsf);

    MBOOL fgBlkChg =
        (m_rTsfLscParam.x_grid_num != rTblCfg.i4GridX)||
        (m_rTsfLscParam.y_grid_num != rTblCfg.i4GridY);

    float fRto = (((float)m_rTsfLscParam.raw_wd*rTblCfg.i4ImgHt) / ((float)m_rTsfLscParam.raw_ht*rTblCfg.i4ImgWd));
    float fMax = 1.0f + (1.0f/80.0f);
    float fMin = 1.0f - (1.0f/80.0f);
    MBOOL fgAspectChg = fRto > fMax || fRto < fMin;
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

    switch (m_i4CfgOpt)
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

    if(!m_bReInitTsf)
      tsfSetCfgChgToHW();

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
        m_bTSFInstanced = MTRUE;
        m_bTSFInit = MTRUE;

        if(fgCrop==MFALSE)
        {
            MUINT32 u4TblIdx = (m_u4TblIdx & RING_TBL_MSK);
            TSF_INPUT_STAT_T& rStat = m_rStat[u4TblIdx];

            ILscBuf* rInputBuf = m_ptrTsfRingBuf[u4TblIdx];
            ILscBuf* rPriorBuf = m_ptrTsfRingBuf[(u4TblIdx+2)&RING_TBL_MSK];

            m_u4TblIdxRear=((m_u4TblIdxRear+1)&RING_TBL_MSK);
            //LSC_ERR("m_u4TblIdxRear %d", m_u4TblIdxRear);

            m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]=m_pTsfRingBuf[m_u4TblIdxRear];
            ILscBuf* rOutputBuf = m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK];

            tsfBatch(rStat, rInputBuf, rOutputBuf);
        }

        m_bTSFInstanced = MTRUE;
        m_bTSFInit = MTRUE;
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
                //m_rLscRingTbl[i].editRsvdData().u4HwRto = u4Rto;
                //(m_ptrLscRingTbl[i]->editRsvdData()).u4HwRto = u4Rto;

                //TO-DO update ratio to shading table
                m_rStat[i].u4Ratio=u4Rto;
                break;
            }
        }
    }
    TSF_UNLOCK();
    return MTRUE;
}

/*MBOOL
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
            //MUINT32 width = m_rLscRingTbl[i].getConfig().i4ImgWd;
            //MUINT32 height = m_rLscRingTbl[i].getConfig().i4ImgHt;
            MUINT32 width = (m_ptrLscRingTbl[i]->getConfig()).i4ImgWd;
            MUINT32 height = (m_ptrLscRingTbl[i]->getConfig()).i4ImgHt;
            MUINT32 blk_width  = width / (u4GridX-1);
            MUINT32 blk_height = height / (u4GridY-1);
            //if (m_rLscRingTbl[i].cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, (blk_width>>1), (blk_height>>1), width - blk_width, height - blk_height), output))
            if (m_ptrLscRingTbl[i]->cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, (blk_width>>1), (blk_height>>1), width - blk_width, height - blk_height), output))
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
        //MUINT32 width = m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].getConfig().i4ImgWd;
        //MUINT32 height = m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].getConfig().i4ImgHt;
        MUINT32 width = (m_ptrLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]->getConfig()).i4ImgWd;
        MUINT32 height = (m_ptrLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]->getConfig()).i4ImgHt;
        //if (m_rLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)].cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, 0, 0, width, height), output))
        if (m_ptrLscRingTbl[(m_u4TblIdx & RING_TBL_MSK)]->cropOut(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, 0, 0, width, height), output))
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
}*/

MBOOL
TsfDft::
getGainTbl(MUINT32 u4GridX, MUINT32 u4GridY, MINT32 i4MagicNum, std::vector<MUINT16*> &vecGain)
{
    TSF_LOCK();

    ILscTbl output(ILscTable::GAIN_FIXED);

    MBOOL fgRet = MFALSE;

    ILscTbl inTbl;
    inTbl.setConfig(m_rLscBaseTbl.getConfig());
    inTbl.setRsvdData(m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getRatio());
    inTbl.setData(m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getTable(),m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getSize());

    MUINT32 width = m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getConfig().i4ImgWd;
    MUINT32 height = m_ptrTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)]->getConfig().i4ImgHt;
    if (inTbl.cropToGain(ILscTable::TransformCfg_T(width, height, u4GridX, u4GridY, 0, 0, width, height),
        output, vecGain, 0))
        fgRet = MTRUE;
    else
        LSC_ERR("Fail to crop matching LSC Gain Table.");

    TSF_UNLOCK();
    return fgRet;
}

MBOOL
TsfDft::
update(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo)
{
    MBOOL fgLogEn = (m_i4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;

    TSF_LOCK();
    if (m_bTSFInit && m_bTSF)
    {
        MUINT32 u4TblIdx = (m_u4TblIdx & RING_TBL_MSK);
        TSF_INPUT_STAT_T& rStat = m_rStat[u4TblIdx];

        MUINT32 u4DesiredRto = m_pLsc->getRatio();

        // AWB stat
        AAA_TRACE_D("TSF_AAO(%d)", rInputInfo.u4FrmId);
        AAA_TRACE_LSC(TSF_AAO);

        rStat.u4FrmId = rInputInfo.u4FrmId;
        rStat.u4FrmReq = rInputInfo.u4FrmReq;
        rStat.rAwbInfo = rInputInfo.rAwbInfo;
        rStat.u4BlkNumX = m_rTsfEnvInfo.ImgWidth;
        rStat.u4BlkNumY = m_rTsfEnvInfo.ImgHeight;
        rStat.u4Ratio = u4DesiredRto;
        rStat.put(rInputInfo, m_blockW*12*m_blockH, m_fgStatOpt);

        AAA_TRACE_END_LSC;
        AAA_TRACE_END_D;

        m_u4ProcCntLatest = m_u4ProcCnt;
        m_u4TblIdxCmd = u4TblIdx;

        if(fgLogEn) {
            CAM_LOGD_IF(fgLogEn, "Tbl ptr addr, index %d", u4TblIdx);
            for(int i=0;i<4;i++)
            {
                CAM_LOGD_IF(fgLogEn, "i: %d, %p, %d", i, m_ptrTsfRingBuf[i], ((MINT32*)(m_ptrTsfRingBuf[i]->getTable()))[m_ptrTsfRingBuf[i]->getSize()/8]);
            }

            CAM_LOGD_IF(fgLogEn, "Tbl addr");
            for(int i=0;i<4;i++)
            {
                CAM_LOGD_IF(fgLogEn, "i: %d, %p, %d", i, m_pTsfRingBuf[i], ((MINT32*)(m_pTsfRingBuf[i]->getTable()))[m_pTsfRingBuf[i]->getSize()/8]);
            }
        }

    #ifdef TSF_THREAD_BUILD
        if (ILscMgr::E_TSF_CMD_RUN == rInputInfo.eCmd)
        {
            m_fgThreadExec = MTRUE;
            ::sem_post(&m_Sema);
        }
    #endif
        else
        {
            //AAA_TRACE_D("TSF(%d,#%d)", rInputInfo.eCmd, rInputInfo.u4FrmId);
            MBOOL fgOK = MFALSE;

            ILscBuf* rInputBuf = m_ptrTsfRingBuf[u4TblIdx];
            ILscBuf* rPriorBuf = m_ptrTsfRingBuf[(u4TblIdx+2)&RING_TBL_MSK];

            //Keep will not output a new shading table, we should not increase index number
            if(rInputInfo.eCmd!=ILscMgr::E_TSF_CMD_KEEP)
            {
            m_u4TblIdxRear=((m_u4TblIdxRear+1)&RING_TBL_MSK);
            m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]=m_pTsfRingBuf[m_u4TblIdxRear];
            }
            ILscBuf* rOutputBuf = m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK];

            switch (rInputInfo.eCmd)
            {
            default:
            case ILscMgr::E_TSF_CMD_BATCH:
                fgOK = tsfBatch(rStat, rInputBuf, rOutputBuf);
                //LSC_ERR("have a new data %d", ((MUINT32*)rOutputBuf->getTable())[3000]);
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
                    fgOK = tsfBatchCap(rStat, rInputBuf, rOutputBuf);
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
                    fgOK = tsfBatchCap(rStat, rInputBuf, rOutputBuf);
                }
#else
                fgOK = tsfBatchCap(rStat, rInputBuf, rOutputBuf);
#endif
                break;
            case ILscMgr::E_TSF_CMD_RUN:
                fgOK = tsfRun(rStat, rInputBuf, &rPriorBuf, &rOutputBuf);
                break;
            case ILscMgr::E_TSF_CMD_KEEP:
                fgOK = tsfKeep(rStat, &rPriorBuf, &m_ptrTsfRingBuf[(u4TblIdx+3)&RING_TBL_MSK]);
                rOutputBuf=rPriorBuf;
                break;
            }
            if (E_LSC_TSF_TYPE_0 == getType())
            {
                //rOutputBuf->editRsvdData().u4HwRto = u4DesiredRto;
                rOutputBuf->setRatio(u4DesiredRto);
                if (fgOK)
                {
                    tsfSetBufAndRatio(&rOutputBuf, rStat);
                }
                else
                {
                    LSC_ERR("TSF Error(%d,%d,%d,#%d)", rInputInfo.eCmd, rStat.u4Ratio, u4DesiredRto, rInputInfo.u4FrmId);
                }
            }
            m_u4ProcCnt++;
            //AAA_TRACE_END_D;
        }
        // increment table index
        m_u4TblIdx = ((m_u4TblIdx+1) & RING_TBL_MSK);
        //LSC_ERR("m_u4TblIdx %d", m_u4TblIdx);
    }
    TSF_UNLOCK();
    return MTRUE;
}

MBOOL
TsfDft::
config(NS3Av3::shadingConfig_T rConfig)
{
    m_srideSize=rConfig.AAOstrideSize;
    m_blockH=rConfig.AAOBlockH;
    m_blockW=rConfig.AAOBlockW;

    LSC_LOG("[%s] tsf AAO config (strideSize, blockW, blockH)=(%d, %d, %d)",
        __func__, rConfig.AAOstrideSize, rConfig.AAOBlockW, rConfig.AAOBlockH);

    return MTRUE;
}

MBOOL
TsfDft::
isInit()
{
    return m_isTsfInit;
}

MBOOL
TsfDft::
getTsfsConfig(TSFS_INFO_STRUCT &rConfig)
{
    MRESULT ret = S_TSF_OK;

    MTK_TSF_TSFS_INFO_STRUCT *rAlgConfig = reinterpret_cast<MTK_TSF_TSFS_INFO_STRUCT *>(&rConfig);

    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_TSFS_REG, rAlgConfig, NULL);

    return (ret == S_TSF_OK);
}

