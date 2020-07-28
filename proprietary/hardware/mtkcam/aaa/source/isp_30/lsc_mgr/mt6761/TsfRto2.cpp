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
#define LOG_TAG "tsf_rto2"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "TsfRto2.h"
#include <LscUtil.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<TsfRto2>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

ILscTsf*
TsfRto2::
createInstance(MUINT32 u4SensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(u4SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<TsfRto2>(u4SensorDev);
    } );

    return rSingleton.instance.get();
}

TsfRto2::
TsfRto2(MUINT32 u4SensorDev)
    : TsfDft(u4SensorDev)
{
    LSC_LOG("Enter Type 2: Ratio Cycle");

    for (MINT32 i = 0; i < RING_TBL_NUM; i++)
        m_rStat[i].u4Ratio = 32;

    LSC_LOG("alloc ring buf for type 2");
    for (MINT32 i = 0; i < RING_TBL_NUM; i++)
    {
        LSC_LOG("alloc ring %d", i);
        char strName[32];
        sprintf(strName, "TSF_RingBuf_%d", i);

        MUINT32 id = 0xFFFFFFFD - i;           /*m_pLsc->m_rBufPool.size()*/
        ILscBuf* pBuf = new ILscBuf(m_u4SensorDev, id, MTRUE, strName, ILscBuf::E_LSC_IMEM);
        if (!pBuf)
        {
            LSC_ERR("Fail to create buffer %s", strName);
        } else {
            LSC_LOG("OK: pBuf(%p, %s), poolsize(%d)", pBuf, pBuf->getName(), id);
            m_pTsfRingBuf[i] = pBuf;
        }
    }
}

TsfRto2::
~TsfRto2()
{
    LSC_LOG("Exit Type 2: Ratio Cycle");
}

MBOOL
TsfRto2::
tsfCfgChg()
{
    MRESULT ret = S_TSF_OK;

    // this is for identical sized tables, but block numbers are different.
    // no cropping support.
    // ex. 10x10 -> 16x16
    LSC_LOG_BEGIN("Sensor(%d)", m_u4SensorDev);

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

    tsfSetBufAndRatio(&m_pTsfRingBuf[(m_u4TblIdx & RING_TBL_MSK)], m_rStat[(m_u4TblIdx & RING_TBL_MSK)].u4Ratio);

    LSC_LOG_END("Sensor(%d)", m_u4SensorDev);

    return ret == S_TSF_OK;
}


MBOOL
TsfRto2::
tsfSetRatoTblChg(MUINT32 u4Ratio, const ILscTbl& rBaseTbl)
{
    ILscTbl rRtoTbl(ILscTable::HWTBL);
    if (rBaseTbl.getRaTbl(u4Ratio, rRtoTbl))
    {
        MTK_TSF_TBL_STRUCT rTsfTbl;
        MTK_TSF_LSC_PARAM_STRUCT rLscCfg;
        ILscTable::Config rTblCfg = rRtoTbl.getConfig();
        rLscCfg.raw_wd        = rTblCfg.i4ImgWd;
        rLscCfg.raw_ht        = rTblCfg.i4ImgHt;
        rLscCfg.x_offset      = 0;
        rLscCfg.y_offset      = 0;
        rLscCfg.block_wd      = rTblCfg.rCfgBlk.i4BlkW;
        rLscCfg.block_ht      = rTblCfg.rCfgBlk.i4BlkH;
        rLscCfg.x_grid_num    = rTblCfg.i4GridX;
        rLscCfg.y_grid_num    = rTblCfg.i4GridY;
        rLscCfg.block_wd_last = rTblCfg.rCfgBlk.i4BlkLastW;
        rLscCfg.block_ht_last = rTblCfg.rCfgBlk.i4BlkLastH;
        rTsfTbl.pLscConfig = &rLscCfg;
        rTsfTbl.ShadingTbl = (MINT32*)rRtoTbl.getData();
        MRESULT ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &rTsfTbl, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
            m_prTsf->TsfReset();
            return MFALSE;
        }
        return MTRUE;
    }
    LSC_ERR("Fail to gen ratio(%d) table", u4Ratio);
    return MFALSE;
}

MBOOL
TsfRto2::
tsfBatch(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf* rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MRESULT ret = S_TSF_OK;
    MUINT32 u4Ratio = rInputStat.u4Ratio;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d), m_bDump(%d)", m_u4SensorDev, rInputStat.u4FrmId, u4Ratio, m_bDump);


    if (!tsfSetRatoTblChg(u4Ratio, m_rLscBaseTbl))
    {
        LSC_ERR("Fail to set table change");
    }

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputBuf, rProcInfo);

    //m_rTsfResult.ShadingTbl = rOutputBuf->editTable();
    //m_rTsfResult.u4TblSize = rOutputBuf->getSize();

    m_prTsf->TsfReset();
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &rProcInfo, &m_rTsfResult);

    memcpy(rOutputBuf->editTable(), m_rTsfResult.ShadingTbl, MAX_SHADING_CapTil_SIZE);

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

            ILscTbl inTbl;
            inTbl.setConfig(m_rLscBaseTbl.getConfig());
            inTbl.setRsvdData(rInputBuf->getRatio());
            inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfInput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);
            inTbl.dump(strFile);

            ILscTbl outTbl;
            outTbl.setConfig(m_rLscBaseTbl.getConfig());
            outTbl.setRsvdData(rOutputBuf->getRatio());
            outTbl.setData(rOutputBuf->getTable(),rOutputBuf->getSize());
            sprintf(strFile, "/sdcard/tsf/%06d_%04d_tsfOutput_bat.tbl", u4ProcCnt, rInputStat.u4FrmId);

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
TsfRto2::
tsfBatchCap(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf* rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MRESULT ret = S_TSF_OK;
    MUINT32 u4Ratio = rInputStat.u4Ratio;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;

    LSC_LOG_BEGIN("Sensor(%d), #(%d), ratio(%d), m_bDump(%d)", m_u4SensorDev, rInputStat.u4FrmId, u4Ratio, m_bDump);

    if (!tsfSetRatoTblChg(u4Ratio, m_rLscBaseTbl))
    {
        LSC_ERR("Fail to set table change");
    }

    MTK_TSF_SET_PROC_INFO_STRUCT rProcInfo;
    convert(rInputStat, rInputBuf, rProcInfo);

    //m_rTsfResult.ShadingTbl = rOutputBuf->editTable();
    //m_rTsfResult.u4TblSize = rOutputBuf->getSize();

    m_prTsf->TsfReset();
    // disable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)0, 0);
    // batch
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &rProcInfo, &m_rTsfResult);

    memcpy(rOutputBuf->editTable(), m_rTsfResult.ShadingTbl, MAX_SHADING_CapTil_SIZE);

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
            sprintf(strFile, "/data/vendor/camera_dump/%06d_%04d_tsfInStat_cap.bin", u4ProcCnt, rInputStat.u4FrmId);
            rInputStat.dump(strFile);

            ILscTbl inTbl;
            inTbl.setConfig(m_rLscBaseTbl.getConfig());
            inTbl.setRsvdData(rInputBuf->getRatio());
            inTbl.setData(rInputBuf->getTable(),rInputBuf->getSize());
            sprintf(strFile, "/data/vendor/camera_dump/%06d_%04d_tsfInput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);
            inTbl.dump(strFile);

            ILscTbl outTbl;
            outTbl.setConfig(m_rLscBaseTbl.getConfig());
            outTbl.setRsvdData(rOutputBuf->getRatio());
            outTbl.setData(rOutputBuf->getTable(),rOutputBuf->getSize());
            sprintf(strFile, "/data/vendor/camera_dump/%06d_%04d_tsfOutput_cap.tbl", u4ProcCnt, rInputStat.u4FrmId);

            char buff[16];
            snprintf(buff, sizeof(buff), "%06d_%04d", u4ProcCnt, rInputStat.u4FrmId);
            m_strDumpFileNormal = buff;

            outTbl.dump(strFile);
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
TsfRto2::
tsfRun(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf** rBufPrior, ILscBuf** rOutputBuf)
{
    if(rInputBuf==NULL || rOutputBuf==NULL)
        return MFALSE;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;
    MINT32 i4Case = 0;
    MUINT32 u4Step = m_u4PerFrameStep;
    MUINT32 u4Ratio = rInputStat.u4Ratio;

    CAM_LOGD_IF(fgLogEn, "[%s +] Sensor(%d), #(%d), ratio(%d), step(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, u4Ratio, u4Step);

    if (m_u4PerFrameStep == 0)
    {
        // only set proc info at the 1st frame.
        if (!tsfSetProcInfo(rInputStat, rInputBuf))
            return MFALSE;
    }
    m_u4PerFrameStep ++;

    // ratio
    if (m_eState == E_TSF_OPT_DONE)
    {
        if (!tsfSetRatoTblChg(u4Ratio, m_rLscBaseTbl))
        {
            LSC_ERR("Fail to set table change");
        }
    }

    E_TSF_STATE_T eState = tsfMain();
    m_eState = eState;
    if (eState == E_TSF_FAIL)
    {
        i4Case = -1;
    }
    else if (eState == E_TSF_READY)
    {
        if (tsfGetResult(*rOutputBuf))
        {
            m_u4PerFrameStep = 0;
            (*rOutputBuf)->setRatio(rInputStat.u4Ratio);
            //tsfSetBuf(*rOutputBuf);
            tsfSetBufAndRatio(rOutputBuf, rInputStat.u4Ratio);
            tsfSetSL2(m_rTsfResult.SL2Para);
            i4Case = 1;

            m_u4TblIdxRear=((m_u4TblIdxRear+1)&RING_TBL_MSK);
        }
        else
        {
            i4Case = -2;
        }
    }
    else
    {
        i4Case = 0;
        *rOutputBuf = *rBufPrior;
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
    }

    CAM_LOGD_IF(fgLogEn, "[%s -] Sensor(%d), #(%d), step(%d), case(%d)", __FUNCTION__, m_u4SensorDev, rInputStat.u4FrmId, u4Step, i4Case);

    return (i4Case >= 0);
}

MBOOL
TsfRto2::
tsfKeep(const TSF_INPUT_STAT_T& rInputStat, ILscBuf** rBufPrior, ILscBuf** rOutputBuf)
{
    if(rOutputBuf==NULL)
        return MFALSE;
    MBOOL fgDump = m_u4LogEn & EN_LSC_LOG_TSF_DUMP ? MTRUE : MFALSE;
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

