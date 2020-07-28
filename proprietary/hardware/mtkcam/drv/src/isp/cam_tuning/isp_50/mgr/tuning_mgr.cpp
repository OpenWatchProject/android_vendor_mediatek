/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "TuningMgr"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <semaphore.h>

#include <tuning_mgr_imp.h>
  // For CPTLog*() CameraProfile APIS.


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define TUNING_DUMMY_MAGIC_NUM -1
#define UNUSED(expr) do { (void)(expr); } while (0)

class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            LOG_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 1   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #elif 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT

#define TUNING_MGR_ISP_CLEAN_TOP_NUM    6
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
//void*  TuningMgr::mpIspReferenceRegMap;
pthread_mutex_t mQueueTopAccessLock;

static TuningMgrImp gTuningMgrObj[NTUNING_MAX_SENSOR_CNT];



// Jessy TODO: Modify this
//MUINT32 tuningMgrCleanSetting[TUNING_MGR_ISP_CLEAN_TOP_NUM] = {0x4004, 0x400F8, 0x4010, 0x4014, 0x4018, 0x401C};


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TuningMgrImp::TuningMgrImp()
                : mLock()
                , mQueueInfoLock()
                , mInitCount(0)
{
    int i;
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_START(Event_TdriMgr);  // Profiling Start.
    mInitCount = 0;
    //
}

//-----------------------------------------------------------------------------
TuningMgrImp::~TuningMgrImp()
{
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_END();     // Profiling End.
}

//-----------------------------------------------------------------------------
TuningMgr* TuningMgr::getInstance(MUINT32 sensorIndex)
{
    if(sensorIndex >= NTUNING_MAX_SENSOR_CNT)
    {
        LOG_ERR("Unsupported sensorIndex: %d (Must in 0 ~ %d, Reset it to be 0)", sensorIndex, NTUNING_MAX_SENSOR_CNT);
        sensorIndex = 0;
    }

    LOG_INF("+getInstance, sensorIndex: %d", sensorIndex);

    gTuningMgrObj[sensorIndex].mSensorIndex = sensorIndex;
    return  (TuningMgr*)&gTuningMgrObj[sensorIndex];
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::init(const char* userName, MUINT32 BQNum)
{
    MBOOL ret = MTRUE;
    int i;
    MINT32 tmp=0;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("init TuningMgr");

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }

    LOG_INF("userName: %s, mInitCount: %d, BQNum: %d", userName, this->mInitCount, BQNum);

    if(this->mInitCount == 0)
    {
        LOCAL_PROFILING_LOG_PRINT("run tuning mgr init");

        this->mBQNum = BQNum;

        // Get tuning drv Instance
        this->pTuningDrv = (TuningDrv*)TuningDrv::getInstance(this->mSensorIndex);
        if (!this->pTuningDrv) {
            LOG_ERR("TuningDrvImp::getInstance() fail \n");
            ret = MFALSE;
            goto EXIT;
        }

        // Init
        this->mTuningDrvUser = eTuningDrvUser_3A; // Fix tuning user to eTuningDrvUser_3A currently. If need support other user, need to modify tuning manager API.
        this->pTuningDrv->init(userName, this->mTuningDrvUser, this->mBQNum);

        this->mvTuningNodes.clear();

        this->mCtlEn_CAM = 0;
        this->mCtlEn2_CAM = 0;
        this->mCtlEnDMA_CAM = 0;
        this->mCtlEn_UNI = 0;
        this->mCtlEnDMA_UNI = 0;

        //map reg map for R/W Macro, in order to calculate addr-offset by reg name
        this->mpIspReferenceRegMap = (void*)malloc(REG_SIZE);

    }

    tmp = android_atomic_inc(&this->mInitCount);

EXIT:
    LOG_INF("X:\n");
    return ret;

}



//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::uninit(const char* userName)
{
    MBOOL ret = MTRUE;
    MINT32 tmp;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("Uninit TuningMgr");

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use tuning manager\n");
        return MFALSE;
    }

    LOG_INF("userName: %s, mInitCount: %d", userName, this->mInitCount);

    tmp = android_atomic_dec(&this->mInitCount);

    // Not the last one.
    if(this->mInitCount > 0) {
        goto EXIT;
    }

    this->mvTuningNodes.clear();

    this->mCtlEn_CAM = 0;
    this->mCtlEn2_CAM = 0;
    this->mCtlEnDMA_CAM = 0;
    this->mCtlEn_UNI = 0;
    this->mCtlEnDMA_UNI = 0;

    if(this->mpIspReferenceRegMap != NULL)
    {
        free((void*)this->mpIspReferenceRegMap);
        this->mpIspReferenceRegMap = NULL;
    }


    // uninit tuning_drv
    this->pTuningDrv->uninit(userName, this->mTuningDrvUser);


    LOG_INF("Release\n");

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    updateEngineFD(
        ETuningMgrFunc engine,
        MUINT32 BQIdx,
        MINT32 memID,
        MUINTPTR va
)
{
    MBOOL ret = MTRUE;
    MUINT32 uTuningBit = 0;

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    this->mQueueInfoLock.lock();
    this->mvTuningNodes[BQIdx]->eUpdateFuncBit = (EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) | (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit);
    this->mQueueInfoLock.unlock();

    switch(engine)
    {
        case eTuningMgrFunc_LSC:
            uTuningBit = eIspTuningMgrFuncBit_LSC;
            break;
        case eTuningMgrFunc_BNR:
            uTuningBit = eIspTuningMgrFuncBit_BNR;
            break;
        case eTuningMgrFunc_PDE:
            uTuningBit = eIspTuningMgrFuncBit_PDE;
            break;
        default:
            LOG_ERR("updateFD: Unspoorted engine(%x)!!", engine);
            ret = MFALSE;
            goto EXIT;
    }

    this->mvTuningNodes[BQIdx]->memID = memID;
    this->mvTuningNodes[BQIdx]->Dmai_bufinfo[uTuningBit].VA_addr = va;
    this->mvTuningNodes[BQIdx]->Dmai_bufinfo[uTuningBit].FD = memID;

    LOG_DBG("Idx(%d), engine(0x%x), memID(%d)", BQIdx, engine, memID);

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    updateEngine(
        ETuningMgrFunc engine,
        MBOOL ctlEn,
        MUINT32 BQIdx
)
{
    MBOOL ret = MTRUE;

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    this->mQueueInfoLock.lock();
    this->mvTuningNodes[BQIdx]->eUpdateFuncBit = (EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) | (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit);
    this->mQueueInfoLock.unlock();

    // Enable engine and DMA according to ctlEn
    switch(engine)
    {
        case eTuningMgrFunc_DBS:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | DBS_EN_) : (this->mCtlEn_CAM & ~DBS_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_OBC:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | OBC_EN_) : (this->mCtlEn_CAM & ~OBC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RMG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RMG_EN_) : (this->mCtlEn_CAM & ~RMG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_BNR:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | BNR_EN_) : (this->mCtlEn_CAM & ~BNR_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | BPCI_EN_) : (this->mCtlEnDMA_CAM & ~BPCI_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RMM:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RMM_EN_) : (this->mCtlEn_CAM & ~RMM_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CAC:
#if 0 /*No CAC for Olympus*/
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | CAC_EN_) : (this->mCtlEn_CAM & ~CAC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | CACI_EN) : (this->mCtlEnDMA_CAM & ~CACI_EN);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
#endif
            break;
        case eTuningMgrFunc_LSC:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | LSC_EN_) : (this->mCtlEn_CAM & ~LSC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | LSCI_EN_) : (this->mCtlEnDMA_CAM & ~LSCI_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RCP:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RCP_EN_) : (this->mCtlEn_CAM & ~RCP_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RPG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RPG_EN_) : (this->mCtlEn_CAM & ~RPG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_UFE:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | UFE_EN_) : (this->mCtlEn_CAM & ~UFE_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | UFEO_EN_) : (this->mCtlEnDMA_CAM & ~UFEO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RCP3:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RCP3_EN_) : (this->mCtlEn_CAM & ~RCP3_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CPG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | CPG_EN_) : (this->mCtlEn_CAM & ~CPG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AA:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | AA_EN_) : (this->mCtlEn_CAM & ~AA_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AAO_EN_) : (this->mCtlEnDMA_CAM & ~AAO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SGG1:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | SGG1_EN_) : (this->mCtlEn_CAM & ~SGG1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);

            break;
        case eTuningMgrFunc_AF:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | AF_EN_) : (this->mCtlEn_CAM & ~AF_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AFO_EN_) : (this->mCtlEnDMA_CAM & ~AFO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_UNP2:
            this->mCtlEn_UNI= (ctlEn == MTRUE) ? (this->mCtlEn_UNI | UNP2_A_EN_) : (this->mCtlEn_UNI & ~UNP2_A_EN_);
            TUNING_MGR_WRITE_REG_UNI(this, CAM_UNI_TOP_MOD_EN, this->mCtlEn_UNI, BQIdx);
            break;
        case eTuningMgrFunc_SGG3:
            this->mCtlEn2_CAM= (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SGG3_EN_) : (this->mCtlEn2_CAM & ~SGG3_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_FLK:
            this->mCtlEn2_CAM= (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | FLK_EN_) : (this->mCtlEn2_CAM & ~FLK_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | FLKO_EN_) : (this->mCtlEnDMA_CAM & ~FLKO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SL2F:
	    this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SL2F_EN_) : (this->mCtlEn2_CAM & ~SL2F_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PBN:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | PBN_EN_) : (this->mCtlEn_CAM & ~PBN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PDE:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | (PDE_EN_|PDI_EN_)) : (this->mCtlEnDMA_CAM & ~(PDE_EN_|PDI_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | (PDE_EN_)) : (this->mCtlEn2_CAM & ~(PDE_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PS:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | (PSO_EN_)) : (this->mCtlEnDMA_CAM & ~(PSO_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | (PS_EN_)) : (this->mCtlEn2_CAM & ~(PS_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SGG5:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SGG5_EN_) : (this->mCtlEn2_CAM & ~SGG5_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_HLR:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | HLR_EN_) : (this->mCtlEn2_CAM & ~HLR_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SL2J:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SL2J_EN_) : (this->mCtlEn2_CAM & ~SL2J_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CPN:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | CPN_EN_) : (this->mCtlEn2_CAM & ~CPN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_DCPN:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | DCPN_EN_) : (this->mCtlEn2_CAM & ~DCPN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_ADBS:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | ADBS_EN_) : (this->mCtlEn2_CAM & ~ADBS_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SCM:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SCM_EN_) : (this->mCtlEn2_CAM & ~SCM_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        default:
            LOG_ERR("Unspoorted engine(%x)!!", engine);
            break;
    }


    this->mvTuningNodes[BQIdx]->ctlEn_CAM = this->mCtlEn_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn2_CAM = this->mCtlEn2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM= this->mCtlEnDMA_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn_UNI = this->mCtlEn_UNI;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI = this->mCtlEnDMA_UNI;

    LOG_DBG("Idx(%d), engine(0x%x), ctlEn(%d)", BQIdx, engine, ctlEn);
    LOG_DBG("eTuningMgrUpdateFunc(0x%x), ctlEn_CAM(0x%x), ctlEnDMA_CAM(0x%x), ctlEn_UNI(0x%x), ctlEnDMA_UNI(0x%x)", \
        this->mvTuningNodes[BQIdx]->eUpdateFuncBit, this->mvTuningNodes[BQIdx]->ctlEn_CAM, \
        this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM, this->mvTuningNodes[BQIdx]->ctlEn_UNI, this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI);

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    enableEngine(
        ETuningMgrFunc engine,
        MBOOL ctlEn,
        MUINT32 BQIdx
)
{
    MBOOL ret = MTRUE;

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    // Enable engine and DMA according to ctlEn
    switch(engine)
    {
        case eTuningMgrFunc_DBS:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | DBS_EN_) : (this->mCtlEn_CAM & ~DBS_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_OBC:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | OBC_EN_) : (this->mCtlEn_CAM & ~OBC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RMG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RMG_EN_) : (this->mCtlEn_CAM & ~RMG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_BNR:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | BNR_EN_) : (this->mCtlEn_CAM & ~BNR_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | BPCI_EN_) : (this->mCtlEnDMA_CAM & ~BPCI_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RMM:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RMM_EN_) : (this->mCtlEn_CAM & ~RMM_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CAC:
#if 0 /*No CAC for Olympus*/
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | CAC_EN_) : (this->mCtlEn_CAM & ~CAC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | CACI_EN) : (this->mCtlEnDMA_CAM & ~CACI_EN);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
#endif
            break;
        case eTuningMgrFunc_LSC:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | LSC_EN_) : (this->mCtlEn_CAM & ~LSC_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | LSCI_EN_) : (this->mCtlEnDMA_CAM & ~LSCI_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RCP:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RCP_EN_) : (this->mCtlEn_CAM & ~RCP_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RPG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RPG_EN_) : (this->mCtlEn_CAM & ~RPG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_UFE:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | UFE_EN_) : (this->mCtlEn_CAM & ~UFE_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | UFEO_EN_) : (this->mCtlEnDMA_CAM & ~UFEO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_RCP3:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | RCP3_EN_) : (this->mCtlEn_CAM & ~RCP3_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CPG:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | CPG_EN_) : (this->mCtlEn_CAM & ~CPG_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AA:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | AA_EN_) : (this->mCtlEn_CAM & ~AA_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AAO_EN_) : (this->mCtlEnDMA_CAM & ~AAO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SGG1:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | SGG1_EN_) : (this->mCtlEn_CAM & ~SGG1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);

            break;
        case eTuningMgrFunc_AF:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | AF_EN_) : (this->mCtlEn_CAM & ~AF_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AFO_EN_) : (this->mCtlEnDMA_CAM & ~AFO_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_UNP2:
            this->mCtlEn_UNI= (ctlEn == MTRUE) ? (this->mCtlEn_UNI | UNP2_A_EN_) : (this->mCtlEn_UNI & ~UNP2_A_EN_);
            TUNING_MGR_WRITE_REG_UNI(this, CAM_UNI_TOP_MOD_EN, this->mCtlEn_UNI, BQIdx);
            break;
        case eTuningMgrFunc_SGG3:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SGG3_EN_) : (this->mCtlEn2_CAM & ~SGG3_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_FLK:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | FLK_EN_) : (this->mCtlEn2_CAM & ~FLK_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SL2F:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SL2F_EN_) : (this->mCtlEn2_CAM & ~SL2F_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PBN:
            this->mCtlEn_CAM = (ctlEn == MTRUE) ? (this->mCtlEn_CAM | PBN_EN_) : (this->mCtlEn_CAM & ~PBN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN, this->mCtlEn_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PDE:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | (PDE_EN_|PDI_EN_)) : (this->mCtlEnDMA_CAM & ~(PDE_EN_|PDI_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | (PDE_EN_)) : (this->mCtlEn2_CAM & ~(PDE_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PS:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | (PSO_EN_)) : (this->mCtlEnDMA_CAM & ~(PSO_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | (PS_EN_)) : (this->mCtlEn2_CAM & ~(PS_EN_));
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SGG5:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SGG5_EN_) : (this->mCtlEn2_CAM & ~SGG5_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_HLR:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | HLR_EN_) : (this->mCtlEn2_CAM & ~HLR_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SL2J:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SL2J_EN_) : (this->mCtlEn2_CAM & ~SL2J_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_CPN:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | CPN_EN_) : (this->mCtlEn2_CAM & ~CPN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_DCPN:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | DCPN_EN_) : (this->mCtlEn2_CAM & ~DCPN_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_ADBS:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | ADBS_EN_) : (this->mCtlEn2_CAM & ~ADBS_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        case eTuningMgrFunc_SCM:
            this->mCtlEn2_CAM = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM | SCM_EN_) : (this->mCtlEn2_CAM & ~SCM_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAM_CTL_EN2, this->mCtlEn2_CAM, BQIdx);
            break;
        default:
            LOG_ERR("Unspoorted engine(%x)!!", engine);
            break;
    }


    this->mvTuningNodes[BQIdx]->ctlEn_CAM = this->mCtlEn_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn2_CAM = this->mCtlEn2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM= this->mCtlEnDMA_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn_UNI = this->mCtlEn_UNI;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI = this->mCtlEnDMA_UNI;

    LOG_DBG("Idx(%d), engine(0x%x), ctlEn(%d)", BQIdx, engine, ctlEn);
    LOG_DBG("eTuningMgrUpdateFunc(0x%x), ctlEn_CAM(0x%x), ctlEnDMA_CAM(0x%x), ctlEn_UNI(0x%x), ctlEnDMA_UNI(0x%x)", \
        this->mvTuningNodes[BQIdx]->eUpdateFuncBit, this->mvTuningNodes[BQIdx]->ctlEn_CAM, \
        this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM, this->mvTuningNodes[BQIdx]->ctlEn_UNI, this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI);

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningEngine(
        ETuningMgrFunc engine,
        MBOOL update,
        MUINT32 BQIdx
)
{
    MBOOL ret = MTRUE;

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if (update == MTRUE)
    {
        this->mQueueInfoLock.lock();
        this->mvTuningNodes[BQIdx]->eUpdateFuncBit = (EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) | (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit);
        this->mQueueInfoLock.unlock();
    }

    LOG_DBG("eTuningMgrUpdateFunc(0x%x):update(%d) ", this->mvTuningNodes[BQIdx]->eUpdateFuncBit, update);

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    dequeBuffer(
        MINT32* magicNum,
        MBOOL* isVerify,
        TUNING_MGR_TYPE_ENUM type
)
{
    Mutex::Autolock lock(this->mQueueInfoLock);
    //
    MBOOL ret = MTRUE;
   (void)type;

    for(MUINT32 i = 0; i < this->mBQNum; i++)
    {
        if(NULL == isVerify)
            LOG_DBG("MagicNum[%d]: 0x%x", i, magicNum[i]);
        else
            LOG_INF("MagicNum[%d]: 0x%x, verified: %d", i, magicNum[i], isVerify[i]);
    }

    ret = this->pTuningDrv->deTuningQue(this->mTuningDrvUser, magicNum, isVerify, this->mvTuningNodes);

    if(ret == MFALSE){
        LOG_ERR("deTuningQue fail");
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes.size() != this->mBQNum)
    {
        LOG_ERR("Size of this->mvTuningNodes.size(%zu) is not identical to mBQNum(%u)!!!", this->mvTuningNodes.size(), this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    LOG_DBG("-. dequeBuffer");

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    enqueBuffer(TUNING_MGR_TYPE_ENUM type
)
{
    MBOOL ret = MTRUE;
    (void)type;
    Mutex::Autolock lock(this->mQueueInfoLock);

    ret = this->pTuningDrv->enTuningQue(this->mTuningDrvUser);

    //this->mvTuningNodes.clear(); // No need to clear it

    LOG_INF("magicNum(0x%x)", this->mvTuningNodes[0]->magicNum);

    return ret;
}

MBOOL TuningMgrImp::
    dumpTuningInfo(
        MINT32 magicNum,
        MINT32 uniqueKey
)
{
    MBOOL ret = MTRUE;
    LOG_DBG("+, magic#(%d)", magicNum);
    UNUSED(uniqueKey);

    if(this->pTuningDrv->searchVerifyTuningQue(this->mTuningDrvUser, magicNum, this->mpVerifyTuningNode)){
        LOG_INF("TODO!!! dump tuning info to file, magic#(%d)", magicNum);
    }
    else{
        LOG_WRN("Cannot find the tuning info");
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    LOG_DBG("-");
    return ret;
}


//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteRegs(
        TUNING_MGR_REG_IO_STRUCT*  pRegIo,
        MINT32 cnt,
        MUINT32 BQIdx)
{
    MBOOL ret = MFALSE;
    MUINT32 shift;
    MUINT32 legal_range;
    int i = 0;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    LOG_DBG("BQIdx(%d), startAddr(0x%08x), Cnt(%d)", BQIdx, pRegIo[0].Addr, cnt);

    shift = 0x0;
    // check wether addr is < upper bound
    legal_range = CAM_BASE_RANGE;

    if(pRegIo[i].Addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", pRegIo[i].Addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    for(i = 0; i< cnt; i++){
        this->mvTuningNodes[BQIdx]->pTuningRegBuf[(pRegIo[i].Addr + shift) >> 2] = pRegIo[i].Data;
        LOG_DBG("i(%d),(0x%08x,0x%08x)",i,pRegIo[i].Addr,pRegIo[i].Data);
    }

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteReg(
        MUINT32 addr,
        MUINT32 data,
        MUINT32 BQIdx)
{
    MBOOL ret = MFALSE;
    MUINT32 shift;
    MUINT32 legal_range;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    LOG_DBG("BQIdx(%d), addr:0x%08X, data:0x%08x", BQIdx, addr, data);

    shift = 0x0;
    // check wether addr is < upper bound
    legal_range = CAM_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    this->mvTuningNodes[BQIdx]->pTuningRegBuf[(addr + shift) >> 2] = data;

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MUINT32 TuningMgrImp::
    tuningMgrReadReg(
        MUINT32 addr,
        MUINT32 BQIdx)
{
    MUINT32 value = 0x0;
    MUINT32 shift;
    MUINT32 legal_range;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    shift = 0x0;
    // check wether addr is < upper bound
    legal_range = CAM_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
      return MFALSE;
    }

    value = this->mvTuningNodes[BQIdx]->pTuningRegBuf[(addr + shift) >> 2];

    LOG_DBG("BQIdx(%d),addr(0x%08x),data(0x%08x)",BQIdx, addr, value);

EXIT:
    return value;

}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteRegs_Uni(
        TUNING_MGR_REG_IO_STRUCT*  pRegIo,
        MINT32 cnt,
        MUINT32 BQIdx)
{
    MBOOL ret = MFALSE;
    MUINT32 shift;
    MUINT32 legal_range;
    int i = 0;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    LOG_DBG("BQIdx(%d), startAddr(0x%08x), Cnt(%d)", BQIdx, pRegIo[0].Addr, cnt);

    shift = 0x1000;
    // check wether addr is < upper bound
    legal_range = UNI_BASE_RANGE;

    if(pRegIo[i].Addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", pRegIo[i].Addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    for(i = 0; i< cnt; i++){
        this->mvTuningNodes[BQIdx]->pTuningRegBuf[(pRegIo[i].Addr + shift) >> 2] = pRegIo[i].Data;
        LOG_DBG("i(%d),(0x%08x,0x%08x)",i,pRegIo[i].Addr,pRegIo[i].Data);
    }

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteReg_Uni(
        MUINT32 addr,
        MUINT32 data,
        MUINT32 BQIdx)
{
    MBOOL ret = MFALSE;
    MUINT32 shift;
    MUINT32 legal_range;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    LOG_DBG("BQIdx(%d), addr:0x%08X", BQIdx, addr);

    shift = 0x1000;
    // check wether addr is < upper bound
    legal_range = UNI_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    this->mvTuningNodes[BQIdx]->pTuningRegBuf[(addr + shift) >> 2] = data;

EXIT:
    return ret;


}

//-----------------------------------------------------------------------------
MUINT32 TuningMgrImp::
    tuningMgrReadReg_Uni(
        MUINT32 addr,
        MUINT32 BQIdx)
{
    MUINT32 value = 0x0;
    MUINT32 shift;
    MUINT32 legal_range;

    Mutex::Autolock lock(this->mQueueInfoLock);

    if(BQIdx >= this->mBQNum)
    {
        LOG_ERR("Index(%d) is over size(%d)!!", BQIdx, this->mBQNum);
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx] == NULL)
    {
        LOG_ERR("mvTuningNodes[%d] is null point", BQIdx);
        goto EXIT;
    }

    if(this->mvTuningNodes[BQIdx]->pTuningRegBuf == NULL)
    {
        LOG_ERR("This->mvTuningNodes[%d]->pTuningRegBuf is null point", BQIdx);
        goto EXIT;
    }

    shift = 0x1000;
    // check wether addr is < upper bound
    legal_range = UNI_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
        return MFALSE;
    }

    value = this->mvTuningNodes[BQIdx]->pTuningRegBuf[(addr + shift) >> 2];

    LOG_DBG("BQIdx(%d),addr(0x%08x),data(0x%08x)",BQIdx, addr, value);

EXIT:
    return value;


}


//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    queryTopControlStatus(
        ETuningMgrFunc engine,
        MBOOL *enableStatus,
        MUINT32 BQIdx)
{
    (void)engine;
    (void)enableStatus;
    (void)BQIdx;
    LOG_ERR("ISP_50 no spported yet!!!\n");
    return MFALSE;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrReadRegs(
        TUNING_MGR_REG_IO_STRUCT*  pRegIo,
        MINT32 cnt,
        MUINT32 BQIdx)
{
    (void)pRegIo;
    (void)cnt;
    (void)BQIdx;
    LOG_ERR("ISP_50 no spported yet!!!\n");
    return MFALSE;

}

