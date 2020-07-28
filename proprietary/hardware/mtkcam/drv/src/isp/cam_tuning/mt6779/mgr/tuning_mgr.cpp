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
            LOG_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)",
                mpszName, pszInfo, mIdx++,
                (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};


#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 1   // Use CameraProfile API
        // static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        // static unsigned int G_emLocalEventId = 0;  // Used within each function.
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
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_START(Event_TdriMgr);  // Profiling Start.
    this->mSensorIndex       = 0;
    this->mBQNum             = 0;
    this->mpVerifyTuningNode = NULL;
    this->mCtlEn_CAM         = 0;
    this->mCtlEn2_CAM        = 0;
    this->mCtlEn3_CAM        = 0;
    this->mCtlEn4_CAM        = 0;
    this->mCtlEnDMA_CAM      = 0;
    this->mCtlEnDMA2_CAM     = 0;
    this->mCtlEn_UNI         = 0;
    this->mCtlEnDMA_UNI      = 0;
    this->pTuningDrv         = NULL;
    this->mTuningDrvUser     = eTuningDrvUser_TuningUserNum;
    DBG_LOG_CONFIG(imageio, tuning_mgr);
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
        LOG_ERR("Unsupported sensorIndex: %d (Must in 0 ~ %d, Reset it to be 0)",
            sensorIndex, NTUNING_MAX_SENSOR_CNT);

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
        // Fix tuning user to eTuningDrvUser_3A currently.
        // If need support other user, need to modify tuning manager API.
        this->mTuningDrvUser = eTuningDrvUser_3A;
        this->pTuningDrv->init(userName, this->mTuningDrvUser, this->mBQNum);

        this->mvTuningNodes.clear();

        this->mCtlEn_CAM      = 0;
        this->mCtlEn2_CAM     = 0;
        this->mCtlEn3_CAM     = 0;
        this->mCtlEn4_CAM     = 0;
        this->mCtlEnDMA_CAM   = 0;
        this->mCtlEnDMA2_CAM  = 0;
        this->mCtlEn_UNI      = 0;
        this->mCtlEnDMA_UNI   = 0;

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

    this->mCtlEn_CAM     = 0;
    this->mCtlEn2_CAM    = 0;
    this->mCtlEn3_CAM    = 0;
    this->mCtlEn4_CAM    = 0;
    this->mCtlEnDMA_CAM  = 0;
    this->mCtlEnDMA2_CAM = 0;
    this->mCtlEn_UNI     = 0;
    this->mCtlEnDMA_UNI  = 0;

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

    this->mvTuningNodes[BQIdx]->eUpdateFuncBit =
        ((EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) |
                             (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit));

    this->mQueueInfoLock.unlock();

    switch(engine)
    {
        case eTuningMgrFunc_LSC_R1:
            uTuningBit = eIspTuningMgrFuncBit_LSC;
            break;
        case eTuningMgrFunc_BPCI_R1:                    // BNR rename to BPC
            uTuningBit = eIspTuningMgrFuncBit_BPCI;      // BNR rename to BPC
            break;
        case eTuningMgrFunc_PDE_R1:
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

    LOG_DBG("Idx(%d), engine(0x%02x), memID(%08d), va(0x%08x)",
        BQIdx, engine, memID, va);

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

    this->mvTuningNodes[BQIdx]->eUpdateFuncBit =
        ((EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) |
                             (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit));

    this->mQueueInfoLock.unlock();

    // Enable engine and DMA according to ctlEn
    switch(engine)
    {
        case eTuningMgrFunc_OBC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | OBC_R1_EN_)  : (this->mCtlEn_CAM    & ~OBC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_BPC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | BPC_R1_EN_)  : (this->mCtlEn_CAM    & ~BPC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_LSC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LSC_R1_EN_)  : (this->mCtlEn_CAM    & ~LSC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | LSCI_R1_EN_) : (this->mCtlEnDMA_CAM & ~LSCI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AA_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | AA_R1_EN_)   : (this->mCtlEn_CAM    & ~AA_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AAO_R1_EN_)  : (this->mCtlEnDMA_CAM & ~AAO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AF_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | AF_R1_EN_)   : (this->mCtlEn2_CAM   & ~AF_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AFO_R1_EN_)  : (this->mCtlEnDMA_CAM & ~AFO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_FLK_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | FLK_R1_EN_)  : (this->mCtlEn_CAM    & ~FLK_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | FLKO_R1_EN_) : (this->mCtlEnDMA_CAM & ~FLKO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PBN_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | PBN_R1_EN_)  : (this->mCtlEn2_CAM   & ~PBN_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_PDE_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | PDE_R1_EN_)  : (this->mCtlEn2_CAM   & ~PDE_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | PDI_R1_EN_)  : (this->mCtlEnDMA_CAM & ~PDI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_MOBC_R2:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | MOBC_R2_EN_) : (this->mCtlEn_CAM    & ~MOBC_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_WB_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | WB_R1_EN_)   : (this->mCtlEn_CAM    & ~WB_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_DGN_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | DGN_R1_EN_)  : (this->mCtlEn_CAM    & ~DGN_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_TSFS_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | TSFS_R1_EN_) : (this->mCtlEn2_CAM   & ~TSFS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | TSFSO_R1_EN_): (this->mCtlEnDMA_CAM & ~TSFSO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_DM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | DM_R1_EN_)   : (this->mCtlEn2_CAM   & ~DM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_FLC_R1:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | FLC_R1_EN_)  : (this->mCtlEn3_CAM   & ~FLC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_CCM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | CCM_R1_EN_)  : (this->mCtlEn2_CAM   & ~CCM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_GGM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | GGM_R1_EN_)  : (this->mCtlEn2_CAM   & ~GGM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_GGM_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | GGM_R2_EN_)  : (this->mCtlEn3_CAM   & ~GGM_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_G2C_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | G2C_R1_EN_)  : (this->mCtlEn2_CAM   & ~G2C_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_G2C_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | G2C_R2_EN_)  : (this->mCtlEn3_CAM   & ~G2C_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_YNRS_R1:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | YNRS_R1_EN_) : (this->mCtlEn3_CAM   & ~YNRS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_RSS_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | RSS_R2_EN_)  : (this->mCtlEn3_CAM   & ~RSS_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_SLK_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | SLK_R1_EN_)  : (this->mCtlEn_CAM    & ~SLK_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_SLK_R2:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | SLK_R2_EN_)  : (this->mCtlEn2_CAM   & ~SLK_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_BPCI_R1:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | BPCI_R1_EN_) : (this->mCtlEnDMA_CAM & ~BPCI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_LCES_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LCES_R1_EN_) : (this->mCtlEn_CAM    & ~LCES_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_LTMS_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LTMS_R1_EN_) : (this->mCtlEn_CAM    & ~LTMS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_HLR_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | HLR_R1_EN_ ) : (this->mCtlEn2_CAM   & ~HLR_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,    BQIdx);
            break;
        default:
            LOG_ERR("Unspoorted engine(%x)!!", engine);
            break;
    }


    this->mvTuningNodes[BQIdx]->ctlEn_CAM     = this->mCtlEn_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn2_CAM    = this->mCtlEn2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn3_CAM    = this->mCtlEn3_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn4_CAM    = this->mCtlEn4_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM  = this->mCtlEnDMA_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM = this->mCtlEnDMA2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn_UNI     = this->mCtlEn_UNI;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI  = this->mCtlEnDMA_UNI;

    LOG_DBG("Idx(%08d), engine(0x%02x), ctlEn(%d)  "
            "eTuningMgrUpdateFunc(0x%08x), "
            "ctlEn_CAM(0x%08x), ctlEn2_CAM(0x%08x), "
            "ctlEn3_CAM(0x%08x), ctlEn4_CAM(0x%08x), "
            "ctlEnDMA_CAM(0x%08x), ctlEnDMA2_CAM(0x%08x), "
            "ctlEn_UNI(0x%08x), ctlEnDMA_UNI(0x%08x) ",
            BQIdx, engine, ctlEn,
            this->mvTuningNodes[BQIdx]->eUpdateFuncBit,
            this->mvTuningNodes[BQIdx]->ctlEn_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn3_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn4_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn_UNI,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI);

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
        case eTuningMgrFunc_OBC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | OBC_R1_EN_)  : (this->mCtlEn_CAM    & ~OBC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_BPC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | BPC_R1_EN_)  : (this->mCtlEn_CAM    & ~BPC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_LSC_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LSC_R1_EN_)  : (this->mCtlEn_CAM    & ~LSC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | LSCI_R1_EN_) : (this->mCtlEnDMA_CAM & ~LSCI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AA_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | AA_R1_EN_)   : (this->mCtlEn_CAM    & ~AA_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AAO_R1_EN_)  : (this->mCtlEnDMA_CAM & ~AAO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_AF_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | AF_R1_EN_)   : (this->mCtlEn2_CAM   & ~AF_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | AFO_R1_EN_)  : (this->mCtlEnDMA_CAM & ~AFO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_FLK_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | FLK_R1_EN_)  : (this->mCtlEn_CAM    & ~FLK_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | FLKO_R1_EN_) : (this->mCtlEnDMA_CAM & ~FLKO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_PBN_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | PBN_R1_EN_)  : (this->mCtlEn2_CAM   & ~PBN_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;

        case eTuningMgrFunc_PDE_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | PDE_R1_EN_)  : (this->mCtlEn2_CAM   & ~PDE_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | PDI_R1_EN_)  : (this->mCtlEnDMA_CAM & ~PDI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_MOBC_R2:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | MOBC_R2_EN_) : (this->mCtlEn_CAM    & ~MOBC_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_WB_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | WB_R1_EN_)   : (this->mCtlEn_CAM    & ~WB_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_DGN_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | DGN_R1_EN_)  : (this->mCtlEn_CAM    & ~DGN_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_TSFS_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | TSFS_R1_EN_) : (this->mCtlEn2_CAM   & ~TSFS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | TSFSO_R1_EN_)  : (this->mCtlEnDMA_CAM & ~TSFSO_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_DM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | DM_R1_EN_)   : (this->mCtlEn2_CAM   & ~DM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_FLC_R1:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | FLC_R1_EN_)  : (this->mCtlEn3_CAM   & ~FLC_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_CCM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | CCM_R1_EN_)  : (this->mCtlEn2_CAM   & ~CCM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_GGM_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | GGM_R1_EN_)  : (this->mCtlEn2_CAM   & ~GGM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_GGM_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | GGM_R2_EN_)  : (this->mCtlEn3_CAM   & ~GGM_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_G2C_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | G2C_R1_EN_)  : (this->mCtlEn2_CAM   & ~G2C_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_G2C_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | G2C_R2_EN_)  : (this->mCtlEn3_CAM   & ~G2C_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_YNRS_R1:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | YNRS_R1_EN_) : (this->mCtlEn3_CAM   & ~YNRS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_RSS_R2:
            this->mCtlEn3_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn3_CAM   | RSS_R2_EN_)  : (this->mCtlEn3_CAM   & ~RSS_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN3,    this->mCtlEn3_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_SLK_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | SLK_R1_EN_)  : (this->mCtlEn_CAM    & ~SLK_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_SLK_R2:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | SLK_R2_EN_)  : (this->mCtlEn2_CAM   & ~SLK_R2_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,    this->mCtlEn2_CAM,   BQIdx);
            break;
        case eTuningMgrFunc_BPCI_R1:
            this->mCtlEnDMA_CAM = (ctlEn == MTRUE) ? (this->mCtlEnDMA_CAM | BPCI_R1_EN_) : (this->mCtlEnDMA_CAM & ~BPCI_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_DMA_EN, this->mCtlEnDMA_CAM, BQIdx);
            break;
        case eTuningMgrFunc_LCES_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LCES_R1_EN_) : (this->mCtlEn_CAM    & ~LCES_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_LTMS_R1:
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LTMS_R1_EN_) : (this->mCtlEn_CAM    & ~LTMS_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_LTM_R1: // discuss with tuning. only open in enalbeEngine function
            this->mCtlEn_CAM    = (ctlEn == MTRUE) ? (this->mCtlEn_CAM    | LTM_R1_EN_ ) : (this->mCtlEn_CAM    & ~LTM_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN,     this->mCtlEn_CAM,    BQIdx);
            break;
        case eTuningMgrFunc_HLR_R1:
            this->mCtlEn2_CAM   = (ctlEn == MTRUE) ? (this->mCtlEn2_CAM   | HLR_R1_EN_ ) : (this->mCtlEn2_CAM   & ~HLR_R1_EN_);
            TUNING_MGR_WRITE_REG_CAM(this, CAMCTL_R1_CAMCTL_EN2,     this->mCtlEn2_CAM,    BQIdx);
            break;
        default:
            LOG_ERR("Unspoorted engine(%x)!!", engine);
            break;
    }


    this->mvTuningNodes[BQIdx]->ctlEn_CAM     = this->mCtlEn_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn2_CAM    = this->mCtlEn2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn3_CAM    = this->mCtlEn3_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn4_CAM    = this->mCtlEn4_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM  = this->mCtlEnDMA_CAM;
    this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM = this->mCtlEnDMA2_CAM;
    this->mvTuningNodes[BQIdx]->ctlEn_UNI     = this->mCtlEn_UNI;
    this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI  = this->mCtlEnDMA_UNI;

    LOG_DBG("Idx(%08d), engine(0x%02x), ctlEn(%d)  "
            "eTuningMgrUpdateFunc(0x%08x), "
            "ctlEn_CAM(0x%08x), ctlEn2_CAM(0x%08x), "
            "ctlEn3_CAM(0x%08x), ctlEn4_CAM(0x%08x), "
            "ctlEnDMA_CAM(0x%08x), ctlEnDMA2_CAM(0x%08x), "
            "ctlEn_UNI(0x%08x), ctlEnDMA_UNI(0x%08x)",
            BQIdx, engine, ctlEn,
            this->mvTuningNodes[BQIdx]->eUpdateFuncBit,
            this->mvTuningNodes[BQIdx]->ctlEn_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn3_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn4_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn_UNI,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI);

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

        this->mvTuningNodes[BQIdx]->eUpdateFuncBit =
            ((EIspTuningMgrFunc)((MUINT32)getIspTuningMgrFunc(engine) |
                                 (MUINT32)this->mvTuningNodes[BQIdx]->eUpdateFuncBit));

        this->mQueueInfoLock.unlock();
    }


    LOG_DBG("Idx(%08d), engine(0x%02x), update(%d) "
            "eTuningMgrUpdateFunc(0x%08x), "
            "ctlEn_CAM(0x%08x), ctlEn2_CAM(0x%08x), "
            "ctlEn3_CAM(0x%08x), ctlEn4_CAM(0x%08x), "
            "ctlEnDMA_CAM(0x%08x), ctlEnDMA2_CAM(0x%08x), "
            "ctlEn_UNI(0x%08x), ctlEnDMA_UNI(0x%08x)",
            BQIdx, engine, update,
            this->mvTuningNodes[BQIdx]->eUpdateFuncBit,
            this->mvTuningNodes[BQIdx]->ctlEn_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn3_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn4_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn_UNI,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI);


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

    for(MUINT32 i = 0; i < this->mBQNum; i++)
    {
        if(NULL == isVerify)
            LOG_DBG("MagicNum[%d]: 0x%x", i, magicNum[i]);
        else
            LOG_INF("MagicNum[%d]: 0x%x, verified: %d", i, magicNum[i], isVerify[i]);
    }

    ret = this->pTuningDrv->deTuningQue(this->mTuningDrvUser,
                magicNum, isVerify, this->mvTuningNodes, (type == TUNING_MGR_TYPE_SETTUNING)?MTRUE:MFALSE);

    if(ret == MFALSE){
        LOG_ERR("deTuningQue fail");
        ret = MFALSE;
        goto EXIT;
    }

    if(this->mvTuningNodes.size() != this->mBQNum)
    {
        LOG_ERR("Size of this->mvTuningNodes.size(%zu) is not identical to mBQNum(%u)!!!",
                this->mvTuningNodes.size(), this->mBQNum);

        ret = MFALSE;
        goto EXIT;
    }

    LOG_DBG("-. dequeBuffer");

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::enqueBuffer(TUNING_MGR_TYPE_ENUM type)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mQueueInfoLock);

    ret = this->pTuningDrv->enTuningQue(this->mTuningDrvUser, (type == TUNING_MGR_TYPE_SETTUNING)?MTRUE:MFALSE);

    //this->mvTuningNodes.clear(); // No need to clear it

    LOG_INF("magicNum(0x%x), type:%d", this->mvTuningNodes[0]->magicNum, type);

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

    if(this->pTuningDrv->searchVerifyTuningQue(this->mTuningDrvUser,
        magicNum, this->mpVerifyTuningNode)){
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
    tuningMgrReadRegs(
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
        pRegIo[i].Data =
            this->mvTuningNodes[BQIdx]->pTuningRegBuf[(pRegIo[i].Addr + shift) >> 2];

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
        LOG_ERR("over range(0x%08x), legal_range(0x%08x)\n", addr, legal_range);
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
        LOG_ERR("over range(0x%08x), legal_range(0x%08x)\n", addr, legal_range);
      return MFALSE;
    }

    value = this->mvTuningNodes[BQIdx]->pTuningRegBuf[(addr + shift) >> 2];

    LOG_DBG("BQIdx(%d),addr(0x%08x),data(0x%08x)",BQIdx, addr, value);

EXIT:
    return value;

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
        this->mvTuningNodes[BQIdx]->pTuningRegBuf[(pRegIo[i].Addr + shift) >> 2] =
            pRegIo[i].Data;

        LOG_DBG("i(%d),(0x%08x,0x%08x)",i,pRegIo[i].Addr,pRegIo[i].Data);
    }

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteRegs_Uni(
        TUNING_MGR_REG_IO_STRUCT*  pRegIo,
        MINT32 cnt,
        MUINT32 BQIdx)
{
    (void)pRegIo;
    (void)cnt;
    (void)BQIdx;

    LOG_ERR("No Support tuningMgrWriteRegs_Uni()!!!\n");
    return MFALSE;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    tuningMgrWriteReg_Uni(
        MUINT32 addr,
        MUINT32 data,
        MUINT32 BQIdx)
{
    (void)addr;
    (void)data;
    (void)BQIdx;

    LOG_ERR("No Support tuningMgrWriteReg_Uni()!!!\n");
    return MFALSE;
}

//-----------------------------------------------------------------------------
MUINT32 TuningMgrImp::
    tuningMgrReadReg_Uni(
        MUINT32 addr,
        MUINT32 BQIdx)
{
    (void)addr;
    (void)BQIdx;

    LOG_ERR("No Support tuningMgrReadReg_Uni()!!!\n");
    return MFALSE;
}

//-----------------------------------------------------------------------------
MBOOL TuningMgrImp::
    queryTopControlStatus(
        ETuningMgrFunc engine,
        MBOOL *enableStatus,
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

    switch(engine)
    {
        case eTuningMgrFunc_OBC_R1:
            if((this->mCtlEn_CAM & OBC_R1_EN_) == OBC_R1_EN_ ) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_BPC_R1:
            if((this->mCtlEn_CAM & BPC_R1_EN_) == BPC_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_LSC_R1:
            if((this->mCtlEn_CAM & LSC_R1_EN_) == LSC_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_AA_R1:
            if((this->mCtlEn_CAM & AA_R1_EN_) == AA_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_AF_R1:
            if((this->mCtlEn2_CAM & AF_R1_EN_) == AF_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_FLK_R1:
            if((this->mCtlEn_CAM & FLK_R1_EN_) == FLK_R1_EN_){
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_PBN_R1:
            if((this->mCtlEn2_CAM & PBN_R1_EN_) == PBN_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_PDE_R1:
            if((this->mCtlEn2_CAM & PDE_R1_EN_) == PDE_R1_EN_){
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_MOBC_R2:
            if((this->mCtlEn_CAM & MOBC_R2_EN_) == MOBC_R2_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_WB_R1:
            if((this->mCtlEn_CAM & WB_R1_EN_) == WB_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_DGN_R1:
            if((this->mCtlEn_CAM & DGN_R1_EN_) == DGN_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_TSFS_R1:
            if((this->mCtlEn2_CAM & TSFS_R1_EN_) == TSFS_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_DM_R1:
            if((this->mCtlEn2_CAM & DM_R1_EN_) == DM_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_FLC_R1:
            if((this->mCtlEn3_CAM & FLC_R1_EN_) == FLC_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_CCM_R1:
            if((this->mCtlEn2_CAM & CCM_R1_EN_) == CCM_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_GGM_R1:
            if((this->mCtlEn2_CAM & GGM_R1_EN_) == GGM_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_GGM_R2:
            if((this->mCtlEn3_CAM & GGM_R2_EN_) == GGM_R2_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_G2C_R1:
            if((this->mCtlEn2_CAM & G2C_R1_EN_) == G2C_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_G2C_R2:
            if((this->mCtlEn3_CAM & G2C_R2_EN_) == G2C_R2_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_YNRS_R1:
            if((this->mCtlEn3_CAM & YNRS_R1_EN_) == YNRS_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_RSS_R2:
            if((this->mCtlEn3_CAM & RSS_R2_EN_) == RSS_R2_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_SLK_R1:
            if((this->mCtlEn_CAM & SLK_R1_EN_) == SLK_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_SLK_R2:
            if((this->mCtlEn2_CAM & SLK_R2_EN_) == SLK_R2_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_BPCI_R1:
            if((this->mCtlEnDMA_CAM & BPCI_R1_EN_) == BPCI_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_LCES_R1:
            if((this->mCtlEn_CAM & LCES_R1_EN_) == LCES_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_LTMS_R1:
            if((this->mCtlEn_CAM & LTMS_R1_EN_) == LTMS_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_LTM_R1: // discuss with tuning. only open in enalbeEngine function
            if((this->mCtlEn_CAM & LTM_R1_EN_ ) == LTM_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        case eTuningMgrFunc_HLR_R1:
            if((this->mCtlEn2_CAM & HLR_R1_EN_) == HLR_R1_EN_) {
                *enableStatus = MTRUE;
            } else {
                *enableStatus = MFALSE;
            }
            break;
        default:
            LOG_ERR("Unspoorted engine(%x)!!", engine);
            ret = MFALSE;
            break;
    }

    LOG_DBG("Idx(%08d) engine(0x%02x) "
            "eTuningMgrUpdateFunc(0x%08x), "
            "ctlEn_CAM(0x%08x_0x%08x), ctlEn2_CAM(0x%08x_0x%08x), "
            "ctlEn3_CAM(0x%08x_0x%08x), ctlEn4_CAM(0x%08x_0x%08x), "
            "ctlEnDMA_CAM(0x%08x_0x%08x), ctlEnDMA2_CAM(0x%08x_0x%08x), "
            "ctlEn_UNI(0x%08x_0x%08x), ctlEnDMA_UNI(0x%08x_0x%08x) ",
            BQIdx, engine,
            this->mvTuningNodes[BQIdx]->eUpdateFuncBit,
            this->mvTuningNodes[BQIdx]->ctlEn_CAM,
            this->mCtlEn_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn2_CAM,
            this->mCtlEn2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn3_CAM,
            this->mCtlEn3_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn4_CAM,
            this->mCtlEn4_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_CAM,
            this->mCtlEnDMA_CAM,
            this->mvTuningNodes[BQIdx]->ctlEnDMA2_CAM,
            this->mCtlEnDMA2_CAM,
            this->mvTuningNodes[BQIdx]->ctlEn_UNI,
            this->mCtlEn_UNI,
            this->mvTuningNodes[BQIdx]->ctlEnDMA_UNI,
            this->mCtlEnDMA_UNI);
EXIT:
    return ret;

}

