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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RscDrv"


#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include <mtkcam/def/common.h>
#include <rsc_drv.h>
#include <rscunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(rsc_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (rsc_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (rsc_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (rsc_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (rsc_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (rsc_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (rsc_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class RscDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    RscDbgTimer(char const*const pszTitle)
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
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

MINT32              RscDrv::m_Fd = -1;
volatile MINT32     RscDrvImp::m_UserCnt = 0;
RSC_DRV_RW_MODE     RscDrv::m_regRWMode = RSC_DRV_R_ONLY;
MUINT32*            RscDrvImp::m_pRscHwRegAddr = NULL;
android::Mutex      RscDrvImp::m_RscInitMutex;


char                RscDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

RscDrvImp::RscDrvImp()
{
    DBG_LOG_CONFIG(drv, rsc_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pRscHwRegAddr = NULL;
}

RscDrv* RscDrv::createInstance()
{
    return RscDrvImp::getInstance();
}

static RscDrvImp singleton;

RscDrv* RscDrvImp::getInstance()
{
    LOG_WRN("dummy sigleton[0x%p].",(void*)&singleton);

    return &singleton;
}


MBOOL RscDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_RscInitMutex);
    //
    if( (userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to use rsc driver\n");
        return MFALSE;
    }
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    if(this->m_UserCnt > 0)
    {
        if(this->m_UserCnt < MAX_USER_NUMBER){
            strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
            old_cnt = android_atomic_inc(&m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    //
    strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
    old_cnt = android_atomic_inc(&m_UserCnt);

    LOG_DBG("-,ret: %d. mInitCount:(%d)", Result, this->m_UserCnt);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL RscDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_RscInitMutex);
    //
    if((userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to uninit rsc driver\n");
        return MFALSE;
    }
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in RscDrv , curUser(%s)",userName);
        goto EXIT;
    }

    for(MUINT32 i=0;i<MAX_USER_NUMBER;i++){
        if(strcmp((const char*)this->m_UserName[i],userName) == 0){
            bMatch = i+1;   //avoid match at the very 1st
            break;
        }
    }

    if(!bMatch){
        LOG_ERR("no matching username:%s\n",userName);
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=4)
           LOG_ERR("current user: %s, %s, %s, %s\n",this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]);
        return MFALSE;
    }
    else
        this->m_UserName[bMatch-1][0] = '\0';

    // More than one user
    old_cnt = android_atomic_dec(&m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

    if(this->m_UserCnt!= 0){
        LOG_INF("current working user:\n");
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=8)
            LOG_INF("current user: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
            ,this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]  \
            ,this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
    }
    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no rsc device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL RscDrvImp::waitIrq(RSC_WAIT_IRQ_STRUCT* pWaitIrq)
{
    (void)pWaitIrq;
    LOG_ERR("waitIrq Error!!, This is RSC Dummy Driver!!");

    return MTRUE;
}

MBOOL RscDrvImp::clearIrq(RSC_CLEAR_IRQ_STRUCT* pClearIrq)
{
    (void)pClearIrq;
    LOG_ERR("clearIrq Error!!, This is RSC Dummy Driver!!");

    return MTRUE;
}



MBOOL RscDrvImp::waitRSCFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    (void)Status;
    (void)timeoutMs;
    LOG_ERR("waitFrameDone Error!!, This is RSC Dummy Driver!!");

    return MTRUE;
}

MBOOL RscDrvImp::enqueRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec)
{
    (void) RscConfigVec;
    LOG_ERR("enque Error!!, This is RSC Dummy Driver!!");

    return MTRUE;
}

MBOOL RscDrvImp::dequeRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec)
{
    (void) RscConfigVec;
    LOG_ERR("deque Error!!, This is RSC Dummy Driver!!");

    return MTRUE;
}


MUINT32 RscDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    (void)Addr;
    (void)caller;
    LOG_ERR("readReg Error!!, This is RSC Dummy Driver!!");

    return 0;
}

//-----------------------------------------------------------------------------
MBOOL RscDrvImp::readRegs(RSC_DRV_REG_IO_STRUCT* pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_ERR("readRegs Error!!, This is RSC Dummy Driver!!");
    return MTRUE;
}


MBOOL RscDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    (void)Addr;
    (void)Data;
    (void)caller;
    LOG_WRN("Dummy RSC driver used.");
    //
    return MTRUE;
}

MBOOL RscDrvImp::writeRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_WRN("Dummy RSC driver used.");
    return MTRUE;

}

MUINT32 RscDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL RscDrvImp::setRWMode(RSC_DRV_RW_MODE rwMode)
{
    LOG_WRN("Dummy RSC driver used.");
    this->m_regRWMode = rwMode;
    return MTRUE;
}

