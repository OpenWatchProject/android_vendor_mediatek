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
#define LOG_TAG "lsc_mgr_dft_misc"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include "LscMgrDefault.h"
#include <LscUtil.h>
#include <vector>

#include <isp_mgr.h>

using namespace NSIspTuning;
using namespace NSIspTuningv3;

MRESULT
LscMgrDefault::
CCTOPSetBypass123(MBOOL fgBypass)
{
    LSC_LOG_BEGIN("fgBypass(%d)", fgBypass);

    if (m_fgBypassAlign != fgBypass)
    {
        m_fgBypassAlign = fgBypass;
        loadTableFlow(MTRUE);
    }

    LSC_LOG_END("m_fgBypassAlign(%d)", m_fgBypassAlign);

    return 0;
}

MINT32
LscMgrDefault::
CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename)
{
    m_bDumpSdblk = fgSave;
    m_strSdblkFile = filename;
    m_pTsf->setDumpFileCfg(fgSave, filename);
    return 0;
}

MVOID
LscMgrDefault::
CCTGetSdblkFileCfg(char **outStr)
{
    strncpy(*outStr, m_strSdblkFile.c_str(), strlen(m_strSdblkFile.c_str()));
}

MBOOL
LscMgrDefault::
CCTOPReset()
{
    LSC_LOG("Reset by CCT");
    loadTableFlow(MTRUE);
    setSensorMode(m_eSensorMode, m_u4P1BinWidth, m_u4P1BinHeight, MTRUE);
    return MTRUE;
}

inline static void setDebugTag(SHADING_DEBUG_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_SHADING_MODULE_ID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MINT32
LscMgrDefault::
getDebugInfo(SHADING_DEBUG_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;

    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;
    CAM_LOGD_IF(fgLogEn, "[%s +]", __FUNCTION__);

    ISP_MGR_LSC_T::getInstance(m_eSensorDev).get(debug);
    MUINT32 u4Addr = ISP_MGR_LSC_T::getInstance(m_eSensorDev).getAddr();

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)((m_eAlignFlag<<16) | m_eTrfmFlag));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)m_eSensorMode);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)(m_u4Rto<<16) | m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)u4Addr);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)debug.ratio_0.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)0/*debug.gain_th.val*/);

    // Tsf
    const MINT32* pTsfExif = NULL;
    MBOOL fgTsfOnOff = MFALSE;
    if (m_pTsf)
    {
        MUINT32 u4Type = m_pTsf->getType();
        if (ILscTsf::E_LSC_TSF_TYPE_OpenShading != u4Type)
        {
            fgTsfOnOff = m_pTsf->getOnOff();
            pTsfExif = static_cast<const MINT32*>(m_pTsf->getRsvdData());
            if (fgTsfOnOff)
                m_pTsf->setDebugInfo(static_cast<MVOID*>(&rShadingDbgInfo));
        }
        setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)(u4Type<<16)|m_eSensorMode);
    }
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, fgTsfOnOff);

    // SDBLK dump
    if (m_bDumpSdblk && m_pCurrentBuf)
    {
        if (!m_pCurrentBuf->dump(m_strSdblkFile.c_str()))
        {
            LSC_ERR("Fail to dump %s", m_strSdblkFile.c_str());
        }
    }

    CAM_LOGD_IF(fgLogEn, "[%s -]", __FUNCTION__);

    return 0;
}

MINT32
LscMgrDefault::
getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl, DEBUG_SHAD_ARRAY_2_T& rShadRestTbl)
{
#if 0
    ISP_NVRAM_LSC_T debug;

    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;
    CAM_LOGD_IF(fgLogEn, "[%s +]", __FUNCTION__);

    ISP_MGR_LSC_T::getInstance(m_eSensorDev).get(debug);

    ::memset(&rShadingDbgTbl, 0, sizeof(DEBUG_SHAD_ARRAY_INFO_T));
    ::memset(&rShadRestTbl, 0, sizeof(DEBUG_SHAD_ARRAY_2_T));

    rShadingDbgTbl.hdr.u4KeyID = DEBUG_SHAD_TABLE_KEYID;
    rShadingDbgTbl.hdr.u4ModuleCount = ModuleNum<1, 0>::val;
    rShadingDbgTbl.hdr.u4DbgSHADArrayOffset = sizeof(DEBUG_SHAD_ARRAY_INFO_S::Header);

    rShadingDbgTbl.rDbgSHADArray.u4BlockNumX = debug.ctl2.bits.LSC_SDBLK_XNUM + 1;
    rShadingDbgTbl.rDbgSHADArray.u4BlockNumY = debug.ctl3.bits.LSC_SDBLK_YNUM + 1;

    MUINT32 u4Blocks =
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumX *
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumY;

    rShadingDbgTbl.rDbgSHADArray.u4CountU32 = u4Blocks*4*6;
    MUINT32 u4Size = u4Blocks*4*6*sizeof(MUINT32);
    if (u4Size > SHAD_ARRAY_VALUE_SIZE*sizeof(MUINT32))
    {
        LSC_ERR("Size(%d) exceeds max", u4Size);
        rShadingDbgTbl.rDbgSHADArray.u4CountU32 = SHAD_ARRAY_VALUE_SIZE;
        u4Size = SHAD_ARRAY_VALUE_SIZE*sizeof(MUINT32);
    }
    const MUINT32* pu4Addr = m_pCurrentBuf->getTable();
    ::memcpy(rShadingDbgTbl.rDbgSHADArray.u4Array, pu4Addr, u4Size);

    CAM_LOGD_IF(fgLogEn, "[%s -] X(%d),Y(%d),Cnt(%d)", __FUNCTION__,
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumX,
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumY,
        rShadingDbgTbl.rDbgSHADArray.u4CountU32);
#endif
    return 0;
}
/*
MBOOL
LscMgrDefault::
setSwNr()
{
    ISP_NVRAM_SL2_T rSl2Cfg;
    ISP_MGR_SL2_T::getInstance(m_eSensorDev).get(rSl2Cfg);

    LSC_LOG("SL2 cen(0x%08x), rr0(0x%08x), rr1(0x%08x), rr2(0x%08x)",
        rSl2Cfg.cen, rSl2Cfg.max0_rr, rSl2Cfg.max1_rr, rSl2Cfg.max2_rr);
#if 0
    SwNRParam::getInstance(m_i4SensorIdx)->setSL2B(
        rSl2Cfg.cen.bits.SL2_CENTR_X,
        rSl2Cfg.cen.bits.SL2_CENTR_Y,
        rSl2Cfg.max0_rr.val,
        rSl2Cfg.max1_rr.val,
        rSl2Cfg.max2_rr.val);
#endif
    return MTRUE;
}
*/
MINT32
LscMgrDefault::
getGainTable(MUINT32 /*u4Bayer*/, MUINT32 /*u4GridNumX*/, MUINT32 /*u4GridNumY*/, MFLOAT* pGainTbl)
{
    // #warning "hardcode table"
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    return 0;
}

MBOOL
LscMgrDefault::
getGainTable(MUINT32 gridPointNum, MINT32 i4MagicNum, ILscTbl& output) const
{
    MBOOL fgRet = MFALSE;
    if (m_pTsf->getOnOff() == MFALSE)
    {
        // LSC OnOff
        if (getOnOff() == MTRUE)
        {
            LSC_LOG("TSF is turned OFF. Use current LSC table: m_rCurLscTbl[u4CTIdx].");

            ILscTbl currTbl;

            if (getCurrTbl(currTbl) == MFALSE)
            {
                LSC_ERR("Failed to get current shading table.");
                return fgRet;
            }

            MUINT32 width = currTbl.getConfig().i4ImgWd;
            MUINT32 height = currTbl.getConfig().i4ImgHt;
            MUINT32 blk_width  = width / (gridPointNum-1);
            MUINT32 blk_height = height / (gridPointNum-1);

            if (currTbl.cropOut(ILscTable::TransformCfg_T(width, height, gridPointNum, gridPointNum, (blk_width>>1), (blk_height>>1), width - blk_width, height - blk_height), output))
            {
                fgRet = MTRUE;
            }
            else
            {
                LSC_ERR("Fail to crop matching LSC Gain Table.");
            }
        }
        else
        {
            LSC_ERR("LSC is turned OFF. Zero gain will be used.");
        }
    }
    else if (m_pTsf->getTbl(gridPointNum, gridPointNum, static_cast<MUINT32>(i4MagicNum), output) == MFALSE)
    {
        LSC_ERR("Failed to get Gain Table.");
    }
    else
    {
        fgRet = MTRUE;
    }

    return fgRet;
}

MBOOL
LscMgrDefault::
getGainTable(MUINT32 gridPointNum, MINT32 i4MagicNum, std::vector<MUINT16*> &vecGain) const
{
    MBOOL fgRet = MFALSE;
    if (m_pTsf->getOnOff() == MFALSE)
    {
        return MFALSE;
    }
    else if (m_pTsf->getGainTbl(gridPointNum, gridPointNum, i4MagicNum, vecGain) == MFALSE)
    {
        LSC_ERR("No need to update Gain Table.");
        fgRet = MTRUE;
    }
    else
    {
        fgRet = MTRUE;
    }

    return fgRet;

}

MINT32
LscMgrDefault::
getCurrTbl(ILscTbl& output) const
{
    Mutex::Autolock lock(m_Lock);
    if (m_pCurrentBuf)
    {
        ILscTable::TBL_TYPE_T eType = output.getType();
        ILscBuf::Config rCfg = m_pCurrentBuf->getConfig();
        if (eType == ILscTable::HWTBL)
        {
            MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;

            CAM_LOGD_IF(fgLogEn, "[%s] LSCI addr %p", __func__, m_pCurrentBuf);

            output.setConfig(rCfg.i4ImgWd, rCfg.i4ImgHt, rCfg.i4BlkX+2, rCfg.i4BlkY+2);
            output.setData(m_pCurrentBuf->getTable(), m_pCurrentBuf->getSize());
        }
        else
        {
            ILscTbl rTmp(ILscTable::HWTBL);
            rTmp.setConfig(rCfg.i4ImgWd, rCfg.i4ImgHt, rCfg.i4BlkX+2, rCfg.i4BlkY+2);
            rTmp.setData(m_pCurrentBuf->getTable(), m_pCurrentBuf->getSize());
            rTmp.convert(output);
        }
        output.editRsvdData().u4HwRto = m_pCurrentBuf->getRatio();
    }
    return 0;
}

MINT32
LscMgrDefault::
getCurrTbl(android::Vector<MUINT8>& rData) const
{
    Mutex::Autolock lock(m_Lock);
    if (m_pCurrentBuf)
    {
        MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_UPDATE) ? MTRUE : MFALSE;

        CAM_LOGD_IF(fgLogEn, "[%s] LSCI addr %p", __func__, m_pCurrentBuf);

        rData.resize(m_pCurrentBuf->getSize()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData));
        ILscBuf::Config rCfg = m_pCurrentBuf->getConfig();
        ILscTable::Config rTblCfg = ILscTbl::makeConfig(rCfg.i4ImgWd, rCfg.i4ImgHt, rCfg.i4BlkX+2, rCfg.i4BlkY+2);
        ILscTbl::RsvdData m_rRsvdData;
        m_rRsvdData.u4HwRto = m_pCurrentBuf->getRatio();
        ::memcpy(rData.editArray(), &rTblCfg, sizeof(ILscTable::Config));
        ::memcpy(rData.editArray()+sizeof(ILscTable::Config), &m_rRsvdData, sizeof(ILscTbl::RsvdData));
        ::memcpy(rData.editArray()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData), m_pCurrentBuf->getTable(), m_pCurrentBuf->getSize());
    }
    return 0;
}

MUINT32
LscMgrDefault::
getCurrTblIndex() const
{
    return m_u4CurrentBufIdx;
}

MINT32
LscMgrDefault::
getCurrTbl(MUINT32 u4CurrentBufIdx, android::Vector<MUINT8>& rData) const
{
    Mutex::Autolock lock(m_Lock);
    ILscBuf* pCurrentBuf;

    if (!getTsfOnOff())
        pCurrentBuf = m_prLscBufRing[u4CurrentBufIdx];
    else
        pCurrentBuf = m_prTsfBufRing[u4CurrentBufIdx];

    if (pCurrentBuf)
    {
        rData.resize(pCurrentBuf->getSize()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData));
        ILscBuf::Config rCfg = pCurrentBuf->getConfig();
        ILscTable::Config rTblCfg = ILscTbl::makeConfig(rCfg.i4ImgWd, rCfg.i4ImgHt, rCfg.i4BlkX+2, rCfg.i4BlkY+2);
        ILscTbl::RsvdData m_rRsvdData;
        m_rRsvdData.u4HwRto = pCurrentBuf->getRatio();
        ::memcpy(rData.editArray(), &rTblCfg, sizeof(ILscTable::Config));
        ::memcpy(rData.editArray()+sizeof(ILscTable::Config), &m_rRsvdData, sizeof(ILscTbl::RsvdData));
        ::memcpy(rData.editArray()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData), pCurrentBuf->getTable(), pCurrentBuf->getSize());
    }
    return 0;
}
