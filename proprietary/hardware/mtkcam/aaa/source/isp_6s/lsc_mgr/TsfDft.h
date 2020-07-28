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
#ifndef _TSF_DFT_H_
#define _TSF_DFT_H_

#include <pthread.h>
#include <semaphore.h>

#include <ILscTsf.h>
#include <lsc/ILscTbl.h>
#include <lsc/ILscMgr.h>
#include <vector>
#include <string>
#include <MTKTsf.h>

#define STAT16BIT 0

namespace NSIspTuning
{
#define EN_LSC_LOG_THREAD       (0x1 << 4)
#define EN_LSC_LOG_TSF_SET_PROC (0x1 << 5)
#define EN_LSC_LOG_TSF_RUN      (0x1 << 6)
#define EN_LSC_LOG_TSF_BATCH    (0x1 << 7)
#define EN_LSC_LOG_TSF_DUMP     (0x1 << 8)
#define EN_LSC_LOG_TSF_REINIT   (0x1 << 9)

class TsfDft : public ILscTsf
{
public:
    static ILscTsf*     createInstance(MUINT32 u4SensorDev);
    virtual void        destroyInstance();
    virtual MBOOL       init();
    virtual MBOOL       uninit();
    virtual MBOOL       setOnOff(MBOOL fgOnOff);
    virtual MBOOL       getOnOff() const;
    virtual MBOOL       setIsFactory(MBOOL fgOnOff);
    virtual MBOOL       setConfig(ESensorMode_T eSensorMode, MUINT32 u4W, MUINT32 u4H, MBOOL fgForce);
    virtual MBOOL       update(const ILscMgr::TSF_INPUT_INFO_T& rInputInfo);
    virtual MBOOL       updateRatio(MUINT32 u4MagicNum, MUINT32 u4Rto);
    //virtual MBOOL       getTbl(MUINT32 u4GridX, MUINT32 u4GridY, MUINT32 u4MagicNum, ILscTbl& output);
    virtual MBOOL       getGainTbl(MUINT32 u4GridX, MUINT32 u4GridY, MINT32 i4MagicNum, std::vector<MUINT16*> &vecGain);
    virtual E_LSC_TSF_TYPE_T    getType() const {return E_LSC_TSF_TYPE_0;}
    virtual MBOOL               setDumpFileCfg(MBOOL fgSave, const char* pathname);
    virtual MVOID               getDumpFileCfg(char *output);
    virtual MBOOL               setDebugInfo(MVOID* pDbgInfo);
    virtual const MVOID*        getRsvdData() const;
    virtual MBOOL               waitDone();

    //virtual void        doNotifyCb (MINT32  _msgType);
    virtual MBOOL       config(NS3Av3::shadingConfig_T rConfig);
    virtual MBOOL       isInit();
    virtual MBOOL       getTsfsConfig(TSFS_INFO_STRUCT &rConfig);

protected:
    #define RING_BUF_NUM 3
    #define RING_TBL_NUM 4
    #define RING_TBL_MSK 0x3

    typedef enum
    {
        E_TSF_NOT_READY = 0,
        E_TSF_READY     = 1,
        E_TSF_OPT_DONE  = 2,
        E_TSF_FAIL      = 255
    } E_TSF_STATE_T;

    struct TSF_INPUT_STAT_T
    {
        MUINT32                 u4FrmId;
        MUINT32                 u4FrmReq;
        MUINT32                 u4Ratio;
        MUINT32                 u4BlkNumX;
        MUINT32                 u4BlkNumY;
        ILscMgr::TSF_AWB_INFO   rAwbInfo;
    #if STAT16BIT
        std::vector<MUINT16>    vecStat;
    #else
        std::vector<MUINT8>     vecStat;
    #endif
        MBOOL dump(const char* filename) const;
        MBOOL put(const ILscMgr::TSF_INPUT_INFO_T& rInStat, MUINT32 StatFullSize, MUINT8 fgOpt = 0x7);
    };

                            TsfDft(MUINT32 u4SensorDev);
    virtual                 ~TsfDft();

    virtual MBOOL           createThread();
    virtual MBOOL           destroyThread();
    virtual MVOID           changeThreadSetting();
    virtual MVOID           doThreadFunc();
    static MVOID *          threadLoop(void *arg);

    virtual MVOID           createTsf();
    virtual MVOID           destroyTsf();
    virtual MBOOL           loadOtpDataForTsf();

    virtual MBOOL           dumpData();
    virtual MBOOL           convert(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo) const;
    virtual MBOOL           tsfResetTbl(ESensorMode_T eLscScn);
    virtual MBOOL           tsfReformTbl();
    virtual MBOOL           tsfSetTbl(const ILscTbl& rTbl);
    virtual MBOOL           tsfSetBuf(ILscBuf* rBuf);
    virtual MBOOL           tsfSetBufAndRatio(ILscBuf** rBuf, const TSF_INPUT_STAT_T &rStat);
    virtual MBOOL           tsfSetSL2(const MTK_TSF_SL2_PARAM_STRUCT& rSL2);

    virtual MBOOL           tsfInit();
    virtual MBOOL           tsfCfgChg();
            MBOOL           tsfSetCfgChgToHW();

    virtual MBOOL           tsfBatch(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf* rOutputBuf);
    virtual MBOOL           tsfBatchCap(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputTbl, ILscBuf* rOutputBuf);
    virtual MBOOL           tsfRun(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf, ILscBuf** rBufPrior, ILscBuf** rOutputBuf);
    virtual MBOOL           tsfKeep(const TSF_INPUT_STAT_T& rInputStat, ILscBuf** rBufPrior, ILscBuf** rOutputBuf);
    virtual MBOOL           tsfSetProcInfo(const TSF_INPUT_STAT_T& rInputStat, ILscBuf* rInputBuf);
    virtual E_TSF_STATE_T   tsfMain();
    virtual MBOOL           tsfGetResult(ILscBuf* rOutputTbl);

    // tsf
    MUINT32                         m_u4SensorDev;
    ESensorMode_T                   m_eSensorMode;
    MINT32                          m_i4LogEn;
    MUINT32                         m_u4FullW;
    MUINT32                         m_u4FullH;
    MBOOL                           m_bTSF;
    MBOOL                           m_bIsFactory;
    MBOOL                           m_bTSFInit;
    MBOOL                           m_bTSFInstanced;
    MBOOL                           m_bTsfForceAwb;
    MBOOL                           m_bReInitTsf;
    MBOOL                           m_bForceApply;
    MINT32                          m_i4CfgOpt;
    MUINT8                          m_fgStatOpt;
    MUINT8*                         m_prAwbStat;
    MUINT32                         m_u4PerFrameStep;
    MUINT32                         m_u4TblIdxCmd;
    MUINT32                         m_u4TblIdx;
    MUINT32                         m_u4TblIdxRear;
    MUINT32                         m_u4ProcCnt;
    MUINT32                         m_u4ProcCntLatest;

    ILscTbl                         m_rLscBaseTbl;
    //ILscTbl*                        m_ptrLscRingTbl[RING_TBL_NUM];
    //ILscTbl                         m_rLscRingTbl[RING_TBL_NUM];
#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
    ILscTbl                         m_rFlashOnTbl;
#endif
    ILscMgr*                        m_pLsc;
    TSF_INPUT_STAT_T                m_rStat[RING_TBL_NUM];
    ILscTable::TransformCfg_T       m_rTransformCfg;
    ILscBuf*                        m_pTsfInDump;
    ILscBuf*                        m_ptrTsfRingBuf[RING_TBL_NUM];
    ILscBuf*                        m_pTsfRingBuf[RING_TBL_NUM];

    MTKTsf*                         m_prTsf;
    MTK_TSF_ENV_INFO_STRUCT         m_rTsfEnvInfo;
    MTK_TSF_LSC_PARAM_STRUCT        m_rTsfLscParam;
    MTK_TSF_RESULT_INFO_STRUCT      m_rTsfResult;

    // thread
    MBOOL                           m_fgThreadLoop;
    MBOOL                           m_fgThreadExec;
    pthread_t                       m_Thread;
    pthread_mutex_t                 m_Mutex;
    sem_t                           m_Sema;
    sem_t                           m_SemaExecDone;

    // reserved data for debug
    MBOOL                           m_bDump;
    std::string                     m_strDumpFile;
    std::string                     m_strDumpFileNormal;

    std::vector<MINT32>             m_vecRsvd;

    // otp
    MUINT8                          m_u1OtpPixId;
    MUINT8                          m_u1OtpGridX;
    MUINT8                          m_u1OtpGridY;

    MUINT32                         m_srideSize;
    MUINT32                         m_blockW;
    MUINT32                         m_blockH;

    MBOOL                           m_isTsfInit;
};
};
#endif //_TSF_DFT_H_
