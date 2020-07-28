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

#define LOG_TAG "lsc_mgr2_misc"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "lsc_mgr2.h"

#include <cutils/properties.h>
#include <aaa_log.h>

#include <isp_mgr.h>
#include <mtkcam/featureio/capturenr.h>

using namespace std;
using namespace NSIspTuning;

namespace NSIspTuningv3 {

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))


#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define LSC_LOG_BEGIN(fmt, arg...)  MY_LOG("[%s +] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG_END(fmt, arg...)    MY_LOG("[%s -] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG(fmt, arg...)        MY_LOG("[%s] " fmt, __FUNCTION__, ##arg)
#define LSC_ERR(fmt, arg...)        MY_ERR("[%s] " fmt, __FUNCTION__, ##arg)

MRESULT
LscMgr2::
CCTOPSetBypass123(MBOOL fgBypass)
{
    LSC_LOG_BEGIN("fgBypass(%d)", fgBypass);

    if (m_fgBypass1to3 != fgBypass)
    {
        m_fgBypass1to3 = fgBypass;
        loadTableFlow(MTRUE);
    }
    
    LSC_LOG_END("m_fgBypass1to3(%d)", m_fgBypass1to3);    
    
    return 0;
}

MRESULT
LscMgr2::
CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename)
{
    m_bDumpSdblk = fgSave;
    m_strSdblkFile = filename;
    return 0;
}

MINT32 
LscMgr2::
dumpSdblk(const char* table_name, const ISP_NVRAM_LSC_T& LscConfig, const MUINT32 *ShadingTbl)
{
    string strTblName(table_name);
    string strFilename;
    FILE *fhwtbl,*fsdblk;

    if (ShadingTbl == NULL)
    {
        LSC_ERR("NULL table");
        return -1;
    }

    LSC_LOG_BEGIN("ShadingTbl(%p)", ShadingTbl);
    
    strFilename = strTblName + ".sdblk";
    fsdblk = fopen(strFilename.c_str(), "w");
    if ( fsdblk == NULL )
    {
        LSC_ERR("Can't open: %s", (const char*) strFilename.c_str());
        return -1;
    }

    strFilename = strTblName + ".hwtbl";
    fhwtbl = fopen(strFilename.c_str(), "w"); 
    if ( fhwtbl == NULL )
    {
        LSC_ERR("Can't open: %s", (const char*) strFilename.c_str());
        return -1;
    }

    fprintf(fsdblk," %8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
            LscConfig.ctl1.bits.SDBLK_XOFST,
            LscConfig.ctl1.bits.SDBLK_YOFST,
            LscConfig.ctl2.bits.LSC_SDBLK_WIDTH,
            LscConfig.ctl3.bits.LSC_SDBLK_HEIGHT,
            LscConfig.ctl2.bits.LSC_SDBLK_XNUM,
            LscConfig.ctl3.bits.LSC_SDBLK_YNUM,
            LscConfig.lblock.bits.LSC_SDBLK_lWIDTH,
            LscConfig.lblock.bits.LSC_SDBLK_lHEIGHT);

    MINT32 x_num = LscConfig.ctl2.bits.LSC_SDBLK_XNUM + 1;
    MINT32 y_num = LscConfig.ctl3.bits.LSC_SDBLK_YNUM + 1;

    MINT32 numCoef = x_num * y_num * 4 * 4;
    MINT32 i, c = 0;

    for (i = numCoef-1; i >= 0; i--)
    {
        MUINT32 coef1, coef2, coef3;
        MUINT32 val = *ShadingTbl++;
        coef3 = (val& 0x3FF00000) >> 20;
        coef2 = (val& 0x000FFC00) >> 10;
        coef1 = val& 0x000003FF;
        fprintf(fsdblk, " %8d %8d %8d", coef1, coef2, coef3);
        fprintf(fhwtbl,"0x%08x, ", val);
        c ++;

        if (c == 4)
        {
            c = 0;
            fprintf(fhwtbl,"\n");
            fprintf(fsdblk,"\n");
        }
    }

    fclose(fhwtbl);
    fclose(fsdblk);

    LSC_LOG_END();

    return 0;
}

inline void setDebugTag(SHADING_DEBUG_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_SHADING_MODULE_ID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MRESULT 
LscMgr2::
getDebugInfo(SHADING_DEBUG_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;

    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).get(debug);
    MUINT32 u4Addr = ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).getAddr();

    MINT32* pTsfExif = NULL;
    if (m_pTsfResultInfo)
    {
        pTsfExif = reinterpret_cast<MINT32*>(m_pTsfResultInfo->ExifData);
        LSC_LOG("pTsfExif(%p), ExifData(%p)", pTsfExif, &m_pTsfResultInfo->ExifData[0]);
    }

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)m_fg1to3);
    // setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)(m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)m_eLscScenario);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)u4Addr);
    //setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)debug.ratio.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)0/*debug.gain_th.val*/);

    //V2
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
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_BORDER,     (MUINT32)m_rOtp.LscTable.MtkLcsData.PixId);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GX,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 28) & 0x0000000F)+2));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GY,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 12) & 0x0000000F)+2));

    LSC_RESVERED_STRUCT resvered_buffer;

    MTKLscUtil::LscVirtualChannel(&resvered_buffer);

    for( int i = SHAD_TAG_CAM_LSC_CP_0; i <= SHAD_TAG_CAM_LSC_CP_7; i++ )
    {
        setDebugTag(rShadingDbgInfo, i, resvered_buffer.buffer[ i - SHAD_TAG_CAM_LSC_CP_0 ] );
    }

    // TSF related
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, m_bTSF);
    if (pTsfExif && m_bTSF)
    {
        MINT32 i;
        #ifdef TSF_BUILD
        ::pthread_mutex_lock(&m_Mutex);
        #endif
        for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
        {
            setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
        }
        #ifdef TSF_BUILD
        ::pthread_mutex_unlock(&m_Mutex);
        #endif
    }

    // SDBLK dump
    if (m_bDumpSdblk)
    {
        if (dumpSdblk(m_strSdblkFile.c_str(), debug, m_pBaseAddr) != 0)
            LSC_ERR("dumpSdblk error\n");
    }

    LSC_LOG_END();
    
    return 0;
}

MRESULT
LscMgr2Rto::
getDebugInfo(SHADING_DEBUG_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;

    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).get(debug);
    MUINT32 u4Addr = ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).getAddr();

    MINT32* pTsfExif = NULL;
    if (m_pTsfResultInfo)
    {
        pTsfExif = reinterpret_cast<MINT32*>(m_pTsfResultInfo->ExifData);
        LSC_LOG("pTsfExif(%p), ExifData(%p)", pTsfExif, &m_pTsfResultInfo->ExifData[0]);
    }

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)m_fg1to3);
    // setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)(m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, ((MUINT32)m_eLscScenario|0x80000000));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)u4Addr);
    //setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)m_i4Rto);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)0/*debug.gain_th.val*/);

    //V2
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
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_BORDER,     (MUINT32)m_rOtp.LscTable.MtkLcsData.PixId);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GX,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 28) & 0x0000000F)+2));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GY,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 12) & 0x0000000F)+2));

    LSC_RESVERED_STRUCT resvered_buffer;

    MTKLscUtil::LscVirtualChannel(&resvered_buffer);

    for( int i = SHAD_TAG_CAM_LSC_CP_0; i <= SHAD_TAG_CAM_LSC_CP_7; i++ )
    {
        setDebugTag(rShadingDbgInfo, i, resvered_buffer.buffer[ i - SHAD_TAG_CAM_LSC_CP_0 ] );
    }

    // TSF related
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, m_bTSF);
    if (pTsfExif && m_bTSF)
    {
        MINT32 i;
        #ifdef TSF_BUILD
        ::pthread_mutex_lock(&m_Mutex);
        #endif
        for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
        {
            setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
        }
        #ifdef TSF_BUILD
        ::pthread_mutex_unlock(&m_Mutex);
        #endif
    }

    // SDBLK dump
    if (m_bDumpSdblk)
    {
        if (dumpSdblk(m_strSdblkFile.c_str(), debug, m_pBaseAddr) != 0)
            LSC_ERR("dumpSdblk error\n");
    }

    LSC_LOG_END();
    
    return 0;
}

MRESULT 
LscMgr2RtoCycle::
getDebugInfo(SHADING_DEBUG_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;

    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).get(debug);
    MUINT32 u4Addr = ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).getAddr();

    MINT32* pTsfExif = NULL;
    if (m_pTsfResultInfo)
    {
        pTsfExif = reinterpret_cast<MINT32*>(m_pTsfResultInfo->ExifData);
        LSC_LOG("pTsfExif(%p), ExifData(%p)", pTsfExif, &m_pTsfResultInfo->ExifData[0]);
    }

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)m_fg1to3);
    // setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)(m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, ((MUINT32)m_eLscScenario|0xC0000000));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)u4Addr);
    //setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)m_i4Rto);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)0/*debug.gain_th.val*/);

    //V2
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
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_BORDER,     (MUINT32)m_rOtp.LscTable.MtkLcsData.PixId);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GX,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 28) & 0x0000000F)+2));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_OTP_GY,         (MUINT32)(((m_rOtp.LscTable.MtkLcsData.CapIspReg[1] >> 12) & 0x0000000F)+2));
    
    LSC_RESVERED_STRUCT resvered_buffer;

    MTKLscUtil::LscVirtualChannel(&resvered_buffer);

    for( int i = SHAD_TAG_CAM_LSC_CP_0; i <= SHAD_TAG_CAM_LSC_CP_7; i++ )
    {
        setDebugTag(rShadingDbgInfo, i, resvered_buffer.buffer[ i - SHAD_TAG_CAM_LSC_CP_0 ] );
    }

    // TSF related
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, m_bTSF);
    if (pTsfExif && m_bTSF)
    {
        MINT32 i;
        #ifdef TSF_BUILD
        ::pthread_mutex_lock(&m_Mutex);
        #endif
        for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
        {
            setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
        }
        #ifdef TSF_BUILD
        ::pthread_mutex_unlock(&m_Mutex);
        #endif
    }

    // SDBLK dump
    if (m_bDumpSdblk)
    {
        if (dumpSdblk(m_strSdblkFile.c_str(), debug, m_pBaseAddr) != 0)
            LSC_ERR("dumpSdblk error\n");
    }

    LSC_LOG_END();
    
    return 0;
}

MRESULT
LscMgr2::
getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl, DEBUG_SHAD_ARRAY_2_T& rShadRestTbl)
{
    ISP_NVRAM_LSC_T debug;
    
    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).get(debug);

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

    MUINT32 u4RestBlocks = 0;

    if (u4Blocks >= APPN_SHAD_BLOCK_NUM_MAX)
    {
        u4RestBlocks = u4Blocks - APPN_SHAD_BLOCK_NUM_MAX;
        u4Blocks = APPN_SHAD_BLOCK_NUM_MAX;
    }

    rShadingDbgTbl.rDbgSHADArray.u4CountU32 = u4Blocks*4*4;
    MUINT32* pu4Addr = m_pBaseAddr;
    ::memcpy(rShadingDbgTbl.rDbgSHADArray.u4Array, pu4Addr, u4Blocks*4*4*sizeof(MUINT32));

    rShadRestTbl.u4CountU32 = u4RestBlocks*4*4;
    if (u4RestBlocks)
    {
        ::memcpy(rShadRestTbl.u4Array, (MUINT32*) pu4Addr+SHAD_ARRAY_VALUE_SIZE, u4RestBlocks*4*4*sizeof(MUINT32));
    }

    LSC_LOG_END("X(%d),Y(%d),Cnt(%d),Cnt2(%d)",
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumX,
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumY,
        rShadingDbgTbl.rDbgSHADArray.u4CountU32,
        rShadRestTbl.u4CountU32);
    
    return 0;
}


MINT32
LscMgr2::
getGainTable(MUINT32 u4Bayer, MUINT32 u4GridNumX, MUINT32 u4GridNumY, MFLOAT* pGainTbl)
{
    LSC_LOG_BEGIN("bayer(%d), X(%d), Y(%d), tbl(%p)", u4Bayer, u4GridNumX, u4GridNumY, pGainTbl);

    MINT32 i4Ret = 0;
    ELscScenario_T eLscScn;
    eLscScn = (ELscScenario_T) m_eLscScenario;
    ShadingTblInput_T rShadingInput;
    ShadingTblCropCfg_T rShadingCrop;

    if (u4GridNumX == 1 && u4GridNumY == 1)
    {
        *pGainTbl++ = 1.0f;
        *pGainTbl++ = 1.0f;
        *pGainTbl++ = 1.0f;
        *pGainTbl++ = 1.0f;
        return 0;
    }
  
    rShadingInput.u1CoefXNum    = m_rIspLscCfg[eLscScn].ctl2.bits.LSC_SDBLK_XNUM + 2;
    rShadingInput.u1CoefYNum    = m_rIspLscCfg[eLscScn].ctl3.bits.LSC_SDBLK_YNUM + 2;

    rShadingCrop.u4Width        = m_rSensorCropCfg[eLscScn].u4Width;
    rShadingCrop.u4Height       = m_rSensorCropCfg[eLscScn].u4Height;
    rShadingCrop.u4OutputWidth  = rShadingCrop.u4Width;
    rShadingCrop.u4OutputHeight = rShadingCrop.u4Height;
    rShadingCrop.rCrop.u4X      = 0;
    rShadingCrop.rCrop.u4Y      = 0;
    rShadingCrop.rCrop.u4W      = rShadingCrop.u4Width;
    rShadingCrop.rCrop.u4H      = rShadingCrop.u4Height;

    LSC_LOG("Input (%d x %d) -> Output (%d x %d) -> X(%d), Y(%d), (%d x %d)",
        rShadingCrop.u4Width, rShadingCrop.u4Height, rShadingCrop.u4OutputWidth, rShadingCrop.u4OutputHeight,
        rShadingCrop.rCrop.u4X, rShadingCrop.rCrop.u4Y, rShadingCrop.rCrop.u4W, rShadingCrop.rCrop.u4H);
    
    // NVRAM as input
    MUINT8* pAddr = ((MUINT8*)stRawLscInfo[eLscScn].virtAddr) + 2*getPerLutSize(eLscScn);
    // output gain
    MUINT32 u4OutSize = u4GridNumX * u4GridNumY * 4 * 2;
    MUINT8* pOutput = new MUINT8[u4OutSize];
    if (pOutput == NULL)
    {
        i4Ret = -2;
        LSC_ERR("Fail to allocate buffer! (%dx%d)", u4GridNumX, u4GridNumY);
    }
    
    rShadingInput.pu4CoefTbl = (MUINT32*) pAddr;
    if (0 == shadingTblCvtGainR(rShadingCrop, rShadingInput, u4GridNumX, u4GridNumY, (MUINT32*)pOutput))
    {
        LSC_LOG("Crop OK: eLscScn(%d), Grid(%dx%d), Input(%p) => (%dx%d), Output(%p)",
            eLscScn, rShadingInput.u1CoefXNum, rShadingInput.u1CoefYNum, pAddr, u4GridNumX, u4GridNumY, pOutput);
    }
    else
    {
        i4Ret = -1;
        LSC_ERR("Crop NG: eLscScn(%d), Grid(%dx%d), Input(%p) => (%dx%d), Output(%p)",
            eLscScn, rShadingInput.u1CoefXNum, rShadingInput.u1CoefYNum, pAddr, u4GridNumX, u4GridNumY, pOutput);
    }

    //
    if (i4Ret == 0)
    {
        MINT32 i4Count = u4OutSize / sizeof(MUINT16);
        MUINT16* pSrc = reinterpret_cast<MUINT16*>(pOutput);
        MFLOAT* pDest = pGainTbl;
        while (i4Count--)
        {
            *pDest++ = (MFLOAT) *pSrc++ / 8192.0f;
        }
    }

    delete pOutput;

    LSC_LOG_END();
    return i4Ret;
}

}
