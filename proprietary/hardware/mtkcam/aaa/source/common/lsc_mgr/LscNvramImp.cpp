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
#define LOG_TAG "lsc_nvram"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include "ILscNvram.h"
#include <LscUtil.h>
#include <mtkcam/drv/IHalSensor.h>
#include <nvbuf_util.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>

using namespace NSIspTuning;

static MVOID dumpOtpFmt(const char* filename, const ILscTbl& rTbl)
{
//    sprintf(strFile, "%s.goldenFmt.txt", filename);
    FILE* pGolden = fopen(filename, "w");
    if (pGolden)
    {
        ILscTable::ConfigBlk rCfgBlk = rTbl.getConfig().rCfgBlk;
        MUINT32 u4Reg1=0, u4Reg3=0;
        u4Reg1 = ((rCfgBlk.i4BlkX << 28) & 0xF0000000) |
                 ((rCfgBlk.i4BlkW << 16) & 0x0FFF0000) |
                 ((rCfgBlk.i4BlkY << 12) & 0x0000F000) |
                 ((rCfgBlk.i4BlkH) & 0x00000FFF);
        u4Reg3 = ((rCfgBlk.i4BlkLastW << 16) & 0x0FFF0000) |
                 ((rCfgBlk.i4BlkLastH) & 0x00000FFF);
        fprintf(pGolden,
        "SensorGoldenCalTable:{   // SensorGoldenCalTable\n"
        "       PixId:    3,\n"
        "       SlimLscType:    0,\n"
        "       Width:    0,\n"
        "       Height:    0,\n"
        "       OffsetX:    0,\n"
        "       OffsetY:    0,\n"
        "       TblSize:    0,\n");
        fprintf(pGolden,
        "       IspLSCReg:    {0, 0x%08x, 0, 0x%08x, 0},\n", u4Reg1, u4Reg3);
        fprintf(pGolden,
        "       GainTable:    {\n");
        MINT32 i = 0;
        MINT32 i4Size = rTbl.getSize();
        const MUINT8* pData = static_cast<const MUINT8*>(rTbl.getData());
        for (i = 0; i < i4Size; i++)
        {
            if ((i & 0xF) == 0)
                fprintf(pGolden, "\n            ");
            fprintf(pGolden, "0x%02x, ", *pData++);
        }
        fprintf(pGolden,
        "\n       },\n"
        "},");
        fclose(pGolden);
    }
}

ILscNvram*
ILscNvram::
getInstance(MUINT32 sensor)
{
    return LscNvramImp::getInstance(sensor);
}

LscNvramImp::
LscNvramImp(MUINT32 sensor)
    : m_eSensorDev(sensor)
    , m_bIsEEPROMImported(MFALSE)
    , m_eOtpState(E_LSC_NO_OTP)
    , m_pNvram_Isp(NULL)
    , m_prNvram3A(NULL)
    , m_prShadingLut(NULL)
    , m_pGolden(NULL)
    , m_pUnit(NULL)
    , m_Thread(0)
{
    LSC_LOG("Enter LscNvramImp");
    getNvramData();
    getTsfCfgTbl();
    m_eOtpState = importEEPromData();

    if (m_eOtpState == E_LSC_WITH_MTK_OTP)
    {
        ILscTable::TBL_BAYER_T eBayerGain = ILscTable::BAYER_B;
        const CAM_CAL_LSC_MTK_TYPE& rMtkLsc = m_rOtp.LscTable.MtkLcsData;
        MUINT32 u4XNum  = ((rMtkLsc.CapIspReg[1] >> 28) & 0x0000000F);
        MUINT32 u4YNum  = ((rMtkLsc.CapIspReg[1] >> 12) & 0x0000000F);
        MUINT32 u4BlkW  = ((rMtkLsc.CapIspReg[1] >> 16) & 0x00000FFF);
        MUINT32 u4BlkH  = ( rMtkLsc.CapIspReg[1]        & 0x00000FFF);
        MUINT32 u4LastW = ((rMtkLsc.CapIspReg[3] >> 16) & 0x00000FFF);
        MUINT32 u4LastH = ( rMtkLsc.CapIspReg[3]        & 0x00000FFF);
        LSC_LOG("Unit table: u4XNum(%d), u4YNum(%d), u4BlkW(%d), u4BlkH(%d), u4LastW(%d), u4LastH(%d)", u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH);

        // XNum, YNum, BlkW, BlkH must be greater than zero. LastW,LastH can be zero.
        if((u4XNum<1) || (u4YNum<1) || (u4BlkW<1) || (u4BlkH<1)/* || (u4LastW<0) || (u4LastH<0)*/)
        {
            LSC_ERR("Wrong unit table configuration loaded from the OTP memory: CapIspReg[1] = 0x%08x and CapIspReg[3] = 0x%08x",
                rMtkLsc.CapIspReg[1], rMtkLsc.CapIspReg[3]);
            m_eOtpState = E_LSC_OTP_ERROR;
            goto lbExit;
        }
        else
        {
            LSC_LOG("Correct unit table configuration loaded from the OTP memory: CapIspReg[1] = 0x%08x and CapIspReg[3] = 0x%08x",
                rMtkLsc.CapIspReg[1], rMtkLsc.CapIspReg[3]);
        }
#if 0
        switch (rMtkLsc.PixId)
        {
        case 0: eBayerGain = ILscTable::BAYER_B;    break;
        case 1: eBayerGain = ILscTable::BAYER_GB;   break;
        case 2: eBayerGain = ILscTable::BAYER_GR;   break;
        case 3: eBayerGain = ILscTable::BAYER_R;    break;
        }
#endif
        eBayerGain = ILscTable::BAYER_B;
        switch (m_prShadingLut->SensorGoldenCalTable.PixId)
        {
        case 0: eBayerGain = ILscTable::BAYER_B;    break;
        case 1: eBayerGain = ILscTable::BAYER_GB;   break;
        case 2: eBayerGain = ILscTable::BAYER_GR;   break;
        case 3: eBayerGain = ILscTable::BAYER_R;    break;
        }
#if 0   // already removed
        // write unit gain table to NVRAM
        LSC_LOG("Write Unit Gain to NVRAM buffer");
        MUINT32 u4GainTblSize = (u4XNum + 2)*(u4YNum + 2)*4*2;  // in byte (x*y*4ch*2byte)
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] = rMtkLsc.CapIspReg[1];
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3] = rMtkLsc.CapIspReg[3];
        m_prShadingLut->SensorGoldenCalTable.IspLSCReg[4] = rMtkLsc.CapIspReg[4];
        m_prShadingLut->SensorGoldenCalTable.TblSize = u4GainTblSize;
        ::memcpy((void*)m_prShadingLut->SensorGoldenCalTable.UnitGainTable,
                (void*)rMtkLsc.CapTable, u4GainTblSize);
#endif
        // init unit gain
        m_pUnit = new ILscTbl(ILscTable::GAIN_FIXED);
        m_pUnit->setBayer(eBayerGain);
        m_pUnit->setConfig(ILscTable::ConfigBlk(u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH));
        m_pUnit->setData(rMtkLsc.CapTable, m_pUnit->getSize());
        // init golden gain
        u4XNum  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 28) & 0x0000000F);
        u4YNum  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 12) & 0x0000000F);
        u4BlkW  = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1] >> 16) & 0x00000FFF);
        u4BlkH  = ( m_prShadingLut->SensorGoldenCalTable.IspLSCReg[1]        & 0x00000FFF);
        u4LastW = ((m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3] >> 16) & 0x00000FFF);
        u4LastH = ( m_prShadingLut->SensorGoldenCalTable.IspLSCReg[3]        & 0x00000FFF);
        LSC_LOG("Golden table: u4XNum(%d), u4YNum(%d), u4BlkW(%d), u4BlkH(%d), u4LastW(%d), u4LastH(%d)", u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH);
        m_pGolden = new ILscTbl(ILscTable::GAIN_FIXED);
        m_pGolden->setBayer(eBayerGain);
        m_pGolden->setConfig(ILscTable::ConfigBlk(u4XNum, u4YNum, u4BlkW, u4BlkH, u4LastW, u4LastH));
        m_pGolden->setData(m_prShadingLut->SensorGoldenCalTable.GainTable, m_pGolden->getSize());

        //create dump thread
        m_mutex.lock();
        if(!m_Thread)
            ::pthread_create(&m_Thread, NULL, threadLoop, this);
        m_mutex.unlock();

    }
lbExit:
    LSC_LOG("Exit LscNvramImp");
}

LscNvramImp::
~LscNvramImp()
{
    LSC_LOG("Delete LscNvramImp");

    if (m_pGolden)
        delete m_pGolden;
    if (m_pUnit)
        delete m_pUnit;
}

MVOID
LscNvramImp::
getNvramData(void)
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)", (MINT32)m_eSensorDev);
    NVRAM_CAMERA_ISP_PARAM_STRUCT* pNvram_Isp = NULL;
    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
    NVRAM_CAMERA_3A_STRUCT *pNvram3A = NULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_eSensorDev, (void*&)pNvram_Isp);
    if(err!=0)
    {
        LSC_ERR("Fail to getBufAndRead(CAMERA_NVRAM_DATA_ISP)!");
        goto lbExit;
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev, (void*&)pNvram_Shading);
    if(err!=0)
    {
        LSC_ERR("Fail to getBufAndRead(CAMERA_NVRAM_DATA_SHADING)!");
        goto lbExit;
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)pNvram3A);
    if(err!=0)
    {
        LSC_ERR("Fail to getBufAndRead(CAMERA_NVRAM_DATA_3A)!");
        goto lbExit;
    }

    m_pNvram_Isp = pNvram_Isp;
    LSC_LOG("m_pNvram_Isp(%p)", m_pNvram_Isp);

    m_prShadingLut = &pNvram_Shading->Shading;
#ifdef USING_MTK_LDVT
    m_prShadingLut->GridXNum = 17;
    m_prShadingLut->GridYNum = 17;
    m_prShadingLut->Width = 1600;
    m_prShadingLut->Height = 1200;
#endif
    LSC_LOG("m_prShadingLut(%p), Version(%d), SensorID(0x%08x), Grid(%dx%d)", m_prShadingLut,
        m_prShadingLut->Version, m_prShadingLut->SensorId,
        m_prShadingLut->GridXNum, m_prShadingLut->GridYNum);

    m_prNvram3A = pNvram3A;
    LSC_LOG("m_prNvram3A(%p)", m_prNvram3A);

lbExit:
    LSC_LOG_END();
}

MVOID
LscNvramImp::
getTsfCfgTbl(void)
{
    LSC_LOG_BEGIN("m_eSensorDev(0x%02x)", (MINT32)m_eSensorDev);

    CAMERA_TSF_TBL_STRUCT*      pDftTsf     = NULL;
    MBOOL           fgLoadOK                = MFALSE;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_TSF_TABLE, m_eSensorDev, (void*&)pDftTsf);
    if(err!=0)
    {
        LSC_ERR("Fail to getBufAndRead(CAMERA_DATA_TSF_TABLE)!");
    }
    else
    {
        fgLoadOK = MTRUE;
    }

    if (fgLoadOK)
    {
        ::memcpy(&m_rTsfCfgTbl, pDftTsf, sizeof(CAMERA_TSF_TBL_STRUCT));
        LSC_LOG("Load TSF table OK, TSF(%d), CtIdx(%d), data(%p), para(%p)",
            m_rTsfCfgTbl.TSF_CFG.isTsfEn, m_rTsfCfgTbl.TSF_CFG.tsfCtIdx,
            m_rTsfCfgTbl.TSF_DATA,
            m_rTsfCfgTbl.TSF_PARA);
    }
    else
    {
        LSC_ERR("Read fail on both sides (NVRAM data and image sensor data)");
        assert (0);
    }

    LSC_LOG_END();
}

MUINT32*
LscNvramImp::
getLut(ESensorMode_T /*eLscScn*/) const
{
#if USING_BUILTIN_LSC
    if (eLscScn == LSC_SCENARIO_CAP)
        return def_coef_cap;
    else
        return def_coef;
#else
    #if 0
    switch (eLscScn)
    {
    case ESensorMode_Preview:
        return &m_prShadingLut->PrvTable[0][0];
    case ESensorMode_Capture:
        return &m_prShadingLut->CapTable[0][0];
    case ESensorMode_Video:
        return &m_prShadingLut->VdoTable[0][0];
    case ESensorMode_SlimVideo1:
        return &m_prShadingLut->Sv1Table[0][0];
    case ESensorMode_SlimVideo2:
        return &m_prShadingLut->Sv2Table[0][0];
    case ESensorMode_Custom1:
        return &m_prShadingLut->Cs1Table[0][0];
    case ESensorMode_Custom2:
        return &m_prShadingLut->Cs2Table[0][0];
    case ESensorMode_Custom3:
        return &m_prShadingLut->Cs3Table[0][0];
    case ESensorMode_Custom4:
        return &m_prShadingLut->Cs4Table[0][0];
    case ESensorMode_Custom5:
        return &m_prShadingLut->Cs5Table[0][0];
    default:
        LSC_ERR("Wrong eLscScn(%d)", eLscScn);
        break;
    }
    return NULL;
    #else
    return &m_prShadingLut->CapTable[0][0];
    #endif
#endif
}

MUINT32*
LscNvramImp::
getLut(ESensorMode_T eLscScn, MUINT32 u4CtIdx) const
{
#if USING_BUILTIN_LSC
    if (eLscScn == LSC_SCENARIO_CAP)
        return def_coef_cap;
    else
        return def_coef;
#else
    if (u4CtIdx < SHADING_SUPPORT_CT_NUM)
    {
        #if 0
        switch (eLscScn)
        {
        case ESensorMode_Preview:
            return &m_prShadingLut->PrvTable[u4CtIdx][0];
        case ESensorMode_Capture:
            return &m_prShadingLut->CapTable[u4CtIdx][0];
        case ESensorMode_Video:
            return &m_prShadingLut->VdoTable[u4CtIdx][0];
        case ESensorMode_SlimVideo1:
            return &m_prShadingLut->Sv1Table[u4CtIdx][0];
        case ESensorMode_SlimVideo2:
            return &m_prShadingLut->Sv2Table[u4CtIdx][0];
        case ESensorMode_Custom1:
            return &m_prShadingLut->Cs1Table[u4CtIdx][0];
        case ESensorMode_Custom2:
            return &m_prShadingLut->Cs2Table[u4CtIdx][0];
        case ESensorMode_Custom3:
            return &m_prShadingLut->Cs3Table[u4CtIdx][0];
        case ESensorMode_Custom4:
            return &m_prShadingLut->Cs4Table[u4CtIdx][0];
        case ESensorMode_Custom5:
            return &m_prShadingLut->Cs5Table[u4CtIdx][0];
        default:
            LSC_ERR("Wrong eLscScn(%d)", eLscScn);
            break;
        }
        #else
        return &m_prShadingLut->CapTable[u4CtIdx][0];
        #endif
    }

    LSC_ERR("Wrong eLscScn(%d), CT(%d)", eLscScn, u4CtIdx);
    return NULL;
#endif
}

MBOOL
LscNvramImp::
check123InNvram() const
{
    MBOOL fgRet =
        (m_pNvram_Isp->ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD) ||
        (m_pNvram_Isp->ISPComm.CommReg[CAL_INFO_IN_COMM_LOAD] == (CAL_DATA_LOAD+1));
    return fgRet;
}

MBOOL
LscNvramImp::
readNvramTbl(MBOOL fgForce)
{
    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev, (void*&)pNvram_Shading, fgForce);
    if(err!=0)
    {
        LSC_ERR("Fail to init NvramDrvMgr, use default LSC table");
        return MFALSE;
    }

    ISP_SHADING_STRUCT* prShadingLut = &pNvram_Shading->Shading;
    if (m_prShadingLut != prShadingLut)
    {
        LSC_LOG("m_prShadingLut(%p) <= prShadingLut(%p)", m_prShadingLut, prShadingLut);
        m_prShadingLut = prShadingLut;
    }

    LSC_LOG("OK");
    return MTRUE;
}

MBOOL
LscNvramImp::
writeNvramTbl(void)
{
    if (0 != NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_SHADING, m_eSensorDev))
    {
        LSC_ERR("Fail to write shading table to NVRAM");
        return MFALSE;
    }
    return MTRUE;

}

MVOID*
LscNvramImp::
threadLoop(void* arg)
{
    LscNvramImp* _this = reinterpret_cast<LscNvramImp*>(arg);

    _this->doThreadFunc();

    return NULL;
}

MVOID
LscNvramImp::
doThreadFunc()
{
    pthread_t threadID=pthread_self();
    //do something
    LSC_LOG("[LSC waiting thread] thread id %d +", threadID);

    if (access("/data/vendor/shading_otp", 6) != -1)
    {
        char strFileName[512] = {'\0'};
        // dump golden/unit gain tables
        sprintf(strFileName, "/data/vendor/shading_otp/%d_golden_%dx%d_%d.otp", m_eSensorDev, m_pGolden->getConfig().i4GridX, m_pGolden->getConfig().i4GridY, m_pGolden->getBayer());
        m_pGolden->dump(strFileName);
        sprintf(strFileName, "/data/vendor/shading_otp/%d_unit_%dx%d_%d.otp", m_eSensorDev, m_pUnit->getConfig().i4GridX, m_pUnit->getConfig().i4GridY, m_pUnit->getBayer());
        m_pUnit->dump(strFileName);
        sprintf(strFileName, "/data/vendor/shading_otp/%d_unit_%dx%d_%d_goldenFmt.txt", m_eSensorDev, m_pUnit->getConfig().i4GridX, m_pUnit->getConfig().i4GridY, m_pUnit->getBayer());
        dumpOtpFmt(strFileName, *m_pUnit);
    }
    else
        LSC_ERR("can't access /data/vendor/shading_otp, cancel dump debug file");

    pthread_detach(pthread_self());

    LSC_LOG("[LSC waiting thread] thread id %d -", threadID);
}

