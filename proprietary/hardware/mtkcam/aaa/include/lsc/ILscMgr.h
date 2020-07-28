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
#ifndef _I_LSC_MGR_H_
#define _I_LSC_MGR_H_

#include <aaa_types.h>
#include <isp_tuning.h>
#include <Local.h>
#include <utils/Vector.h>
#include <vector>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <camera_custom_nvram.h>

#include "ILscBuf.h"
#include "ILscTbl.h"

#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
#include <tuning/shading_flow_custom.h>
#endif

namespace NSIspTuning {
#define EN_LSC_LOG_UPDATE        (0x1 << 0)
#define EN_LSC_LOG_GET_CT        (0x1 << 1)
#define EN_LSC_LOG_SET_CT        (0x1 << 2)
#define EN_LSC_LOG_SET_TABLE     (0x1 << 3)
#define EN_LSC_LOG_THREAD        (0x1 << 4)
#define EN_LSC_LOG_TSF_SET_PROC  (0x1 << 5)
#define EN_LSC_LOG_TSF_RUN       (0x1 << 6)
#define EN_LSC_LOG_TSF_BATCH     (0x1 << 7)
#define EN_LSC_LOG_TSF_DUMP      (0x1 << 8)
#define EN_LSC_LOG_TSF_REINIT    (0x1 << 9)
#define EN_LSC_LOG_TRFM_DUMP     (0x1 << 10)
#define EN_LSC_LOG_TSF_PER_FRAME (0x1 << 11)

/*******************************************************************************
 * Interface of LSC Manager
 *******************************************************************************/
class ILscMgr
{
public:
    typedef enum
    {
        E_TSF_CMD_IDLE      = 0,
        E_TSF_CMD_INIT      = 1,
        E_TSF_CMD_RUN       = 2,
        E_TSF_CMD_BATCH     = 3,
        E_TSF_CMD_BATCH_CAP = 4,
        E_TSF_CMD_CHG       = 5,
        E_TSF_CMD_KEEP      = 6,
    } E_TSF_CMD_T;

    typedef struct
    {
        MUINT32 m_u4CCT;
        MINT32  m_i4LV;
        MINT32  m_RGAIN;
        MINT32  m_GGAIN;
        MINT32  m_BGAIN;
        MINT32  m_FLUO_IDX;
        MINT32  m_DAY_FLUO_IDX;
        MBOOL   m_FLASH_ON;
    } TSF_AWB_INFO;

    typedef struct
    {
        E_TSF_CMD_T     eCmd;
        MUINT32         u4FrmId;
        MUINT32         u4FrmReq;
        TSF_AWB_INFO    rAwbInfo;
        MUINT32         u4SizeAwbStat;
        MUINT8*         prAwbStat;
        MUINT8*         prTSFStat;
    } TSF_INPUT_INFO_T;

    typedef enum
    {
        LSC_SCENARIO_PRV   = 0,    //     ESensorMode_Preview,
        LSC_SCENARIO_CAP   = 1,    //     ESensorMode_Capture,
        LSC_SCENARIO_VDO   = 2,    //     ESensorMode_Video,
        LSC_SCENARIO_SLIM1 = 3,    //     ESensorMode_SlimVideo1,
        LSC_SCENARIO_SLIM2 = 4,    //     ESensorMode_SlimVideo2,
        LSC_SCENARIO_CUST1 = 5,    //     ESensorMode_Custom1,
        LSC_SCENARIO_CUST2 = 6,    //     ESensorMode_Custom2,
        LSC_SCENARIO_CUST3 = 7,    //     ESensorMode_Custom3,
        LSC_SCENARIO_CUST4 = 8,    //     ESensorMode_Custom4,
        LSC_SCENARIO_CUST5 = 9,    //     ESensorMode_Custom5,
        LSC_SCENARIO_NUM
    } ELscScenario_T;

    typedef enum
    {
        LSC_P1 = 0,
        LSC_P2 = 1
    } E_LSC_MODULE_T;

    typedef struct
    {
        int i4CenterX;
        int i4CenterY;
        int i4R0;
        int i4R1;
        int i4R2;
        int i4Gain0;
        int i4Gain1;
        int i4Gain2;
        int i4Gain3;
        int i4Gain4;
        int i4SetZero;
    } SL2_CFG_T;

#if (CAM3_3A_ISP_30_EN)
    static ILscMgr*                 createInstance(
                                        MUINT32 const eSensorDev,
                                        MINT32 i4SensorIdx,
                                        ESensorTG_T const eSensorTG,
                                        NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram);
#else
    static ILscMgr*                 createInstance(
                                        MUINT32 const eSensorDev,
                                        MINT32 i4SensorIdx);
#endif
    static ILscMgr*                 getInstance(MUINT32 sensor);

    virtual MBOOL                   destroyInstance() = 0;

    virtual MBOOL                   init() = 0;
    virtual MBOOL                   uninit() = 0;

    virtual MBOOL                   setTgInfo(MINT32 const /*i4TGInfo*/) {return MFALSE;};
    virtual MBOOL                   setSensorMode(ESensorMode_T eSensorMode, MUINT32 u4Width, MUINT32 u4Height, MBOOL fgForce=MFALSE) = 0;
    virtual ESensorMode_T           getSensorMode() const = 0;
    virtual MBOOL                   getIsOtpOn() const = 0;
    virtual const ILscTbl::TransformCfg_T& getTransformCfg(ESensorMode_T eSensorMode) const = 0;
    virtual const ILscTbl::TransformCfg_T& getCurTrfmCfg() const = 0;
    virtual MBOOL                   getP1BinSize(MUINT32& u4OutW, MUINT32& u4OutH) const = 0;
    virtual MBOOL                   getRawSize(ESensorMode_T eSensorMode, MUINT32& u4OutW, MUINT32& u4OutH) const = 0;
    virtual SL2_CFG_T               getSl2() const = 0;

    virtual MUINT32                 getRegIdx() const {return 32767;};
    virtual MUINT32                 getCTIdx() = 0;
    virtual MBOOL                   setCTIdx(MUINT32 const u4CTIdx) = 0;
    virtual MVOID                   setOnOff(MBOOL fgOnOff) = 0;
    virtual MBOOL                   getOnOff() const = 0;
    virtual MVOID                   setOnOffP2(MBOOL fgOnOff) {};
    virtual MBOOL                   getOnOffP2() const {return MFALSE;};
#if (CAM3_3A_IP_BASE)
    virtual MVOID                   setFactoryMode(MBOOL fgOnOff) = 0;
#endif
    virtual MBOOL                   setRatio(MUINT32 u4Ratio) = 0;
    virtual MUINT32                 getRatio() const = 0;
    virtual MBOOL                   lockRatio(MBOOL bLock){return MFALSE;};

    virtual ILscBuf*                createBuf(const char* name, ILscBuf::E_BUF_MODE_T eBufMode) = 0;
    virtual MBOOL                   destroyBuf(ILscBuf* pBuf) = 0;
    virtual MBOOL                   syncBuf(ILscBuf* /*pBuf*/, E_LSC_MODULE_T /*eModu=LSC_P1*/) { return MFALSE; }
    virtual MBOOL                   syncBufAndRatio(ILscBuf** pBuf, MUINT32 ratio, E_LSC_MODULE_T eModu=LSC_P1) = 0;
    virtual MBOOL                   syncTbl(const ILscTbl& rTbl, E_LSC_MODULE_T eModu=LSC_P1) = 0;
    virtual MBOOL                   syncTSFO(const std::vector<MUINT8> &inputTSFO){return MFALSE;};
    virtual ILscBuf*                getP2Buf() const = 0;
#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
    virtual MBOOL                   evaluate_cus_Ratio(LSC_INPUT_INFO_T &rLscInputInfo, MINT32 i4IsLockRto) = 0;
    virtual MVOID                   enableDynamicShading(MBOOL fgEnable, MUINT32 eOperMode) = 0;
    virtual MVOID                   enableSuperNightMode(MBOOL fgEnable) {};
    virtual MVOID                   setAppInfo(MINT32 appParam) {};
    virtual MBOOL                   getLSCResultPool(MVOID *outputLSC_Result) = 0;
#endif
//    virtual MUINT32            getSensorDev() const = 0;

    virtual const ILscTbl*          getCapLut(MUINT32 u4CtIdx) const = 0;
    virtual const ILscTbl*          getLut(MUINT32 u4CtIdx) const = 0;
    virtual MINT32                  getGainTable(MUINT32 u4Bayer, MUINT32 u4GridNumX, MUINT32 u4GridNumY, MFLOAT* pGainTbl) = 0;
    virtual MBOOL                   getGainTable(MUINT32 gridPointNum, MINT32 i4MagicNum, ILscTbl& output) const = 0;
    virtual MBOOL                   getGainTable(MUINT32 gridPointNum, MINT32 i4MagicNum, std::vector<MUINT16*> &vecGain) const = 0;
    virtual MINT32                  getCorresTsfo(MUINT32 u4CurrentBufIdx, android::Vector<MUINT8>& rData) { return -1; }
    virtual MINT32                  getCorresTsfo(MUINT32 u4CurrentBufIdx, std::vector<MUINT8>& rData) { return -1; }
    virtual MINT32                  getCurrTbl(ILscTbl& output) const = 0;
    virtual MINT32                  getCurrTbl(android::Vector<MUINT8>& rData) const { return -1; }
    virtual MINT32                  getCurrTbl(std::vector<MUINT8>& rData) const { return -1; }
#if (CAM3_3A_IP_BASE)
    // Use Index to get shading table. Because LSC has 4 buffer of shading table.
    virtual MINT32                  getCurrTbl(MUINT32 u4CurrentBufIdx, android::Vector<MUINT8>& rData) const { return -1; }
    virtual MINT32                  getCurrTbl(MUINT32 u4CurrentBufIdx, std::vector<MUINT8>& rData) const { return -1; }
    virtual MUINT32                 getCurrTblIndex() const = 0;
    virtual MUINT32                 getCurTsfoIndex() const { return 0; }
    virtual MBOOL                   config(NS3Av3::shadingConfig_T rConfig) = 0;
    virtual MVOID                   setCapAAOMagicNum(MINT32 i4CapAAOMagicNum){};
    virtual MINT32                  getCapAAOMagicNum(){return -2;};
#endif
    virtual MINT32                  getDebugInfo(SHADING_DEBUG_INFO_T &rShadingDbgInfo) = 0;
    virtual MINT32                  getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl, DEBUG_SHAD_ARRAY_2_T& rShadRestTbl) = 0;

    virtual MINT32                  CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename) = 0;
#if (CAM3_3A_IP_BASE)
    virtual MVOID                   CCTOPGetSdblkFileCfg(char * /*output*/) {};
#endif
    virtual MINT32                  CCTOPSetBypass123(MBOOL fgBypass) = 0;
    virtual MBOOL                   CCTOPReset() = 0;

    virtual MVOID                   CCTGetSdblkFileCfg(char ** /*outStr*/){};
//    virtual MINT32                  setTsfForceAwb(MBOOL fgForce) = 0;

    virtual MVOID                   updateSl2(const SL2_CFG_T& rCfgSl2) = 0;
    virtual MVOID                   updateLsc() = 0;
    virtual MVOID                   updateTsf(const TSF_INPUT_INFO_T& rInputInfo) = 0;
#if (CAM3_3A_IP_BASE)
    virtual MBOOL                   updateRatio(MINT32 i4MagicNum, MUINT32 u4Rto) = 0;
#else
    virtual MBOOL                   updateRatio(MUINT32 u4MagicNum, MUINT32 u4Rto) = 0;
#endif

    virtual MBOOL                   waitTsfExecDone() = 0;
    virtual MVOID                   setTsfOnOff(MBOOL fgOnOff) = 0;
    virtual MBOOL                   getTsfOnOff() const = 0;

    virtual MBOOL                   setDumbFilePrefix(const std::string filename, E_LSC_MODULE_T eModu=LSC_P1) { return MFALSE; }

protected:
    virtual                         ~ILscMgr(){}
};

};
#endif // _I_LSC_MGR_H_
