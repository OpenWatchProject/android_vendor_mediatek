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
#define LOG_TAG "OweDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <owe_drv.h>
#include <oweunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "engine_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(owe_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (owe_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (owe_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (owe_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (owe_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (owe_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (owe_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class OweDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    OweDbgTimer(char const*const pszTitle)
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

MINT32              OweDrv::m_Fd = -1;
volatile MINT32     OweDrvImp::m_UserCnt = 0;
OWE_DRV_RW_MODE     OweDrv::m_regRWMode = OWE_DRV_R_ONLY;
MUINT32*            OweDrvImp::m_pOweHwRegAddr = NULL;
android::Mutex      OweDrvImp::m_OweInitMutex;


char                OweDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

OweDrvImp::OweDrvImp()
{
    DBG_LOG_CONFIG(drv, owe_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pOweHwRegAddr = NULL;
}

OweDrv* OweDrv::createInstance()
{
    return OweDrvImp::getInstance();
}

static OweDrvImp singleton;

OweDrv* OweDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%lx].", (unsigned long)&singleton);

    return &singleton;
}


MBOOL OweDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_OweInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to use owe driver\n");
        return MFALSE;
    }
    //
    if(m_UserCnt > 0)
    {
        MUINT32 idx;
        if(m_UserCnt < MAX_USER_NUMBER){

            for(idx=0;idx<MAX_USER_NUMBER;idx++){
                if(this->m_UserName[idx][0] == '\0'){
                    break;
                }
            }
            if (idx<MAX_USER_NUMBER){
                strncpy((char*)this->m_UserName[idx],userName, strlen(userName));
                this->m_UserName[idx][strlen(userName)] = '\0';
            }
            else
            {
                LOG_ERR("can't find any empty userName:%d!!", idx);
            }
            tmp = android_atomic_inc(&m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d.", m_UserCnt);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }
    //
    strncpy((char*)this->m_UserName[m_UserCnt],userName, strlen(userName));
    this->m_UserName[m_UserCnt][strlen(userName)] = '\0';
    tmp = android_atomic_inc(&m_UserCnt);

EXIT:

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pOweHwRegAddr(0x%lx)", Result, m_UserCnt, (unsigned long)m_pOweHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL OweDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_OweInitMutex);
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to uninit owe driver\n");
        return MFALSE;
    }

    //
    if(m_UserCnt <= 0)
    {
        LOG_ERR("no more user in OweDrv , curUser(%s)",userName);
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
    tmp = android_atomic_dec(&m_UserCnt);

    if(m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    if(m_pOweHwRegAddr != MAP_FAILED){
         munmap(m_pOweHwRegAddr, OWE_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        this->m_Fd = -1;
        this->m_regRWMode=OWE_DRV_R_ONLY;
    }

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, m_UserCnt);

    if(m_UserCnt!= 0){
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
        LOG_ERR("no owe device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL OweDrvImp::waitIrq(OWE_WAIT_IRQ_STRUCT* pWaitIrq)
{
    (void)pWaitIrq;
    LOG_ERR("waitIrq Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}

MBOOL OweDrvImp::clearIrq(OWE_CLEAR_IRQ_STRUCT* pClearIrq)
{
    (void)pClearIrq;
    LOG_ERR("clearIrq Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}



MBOOL OweDrvImp::waitOWEFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    (void)Status;
    (void)timeoutMs;
    LOG_ERR("waitOWEFrameDone Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}

MBOOL OweDrvImp::checkOCCSetting(NSCam::NSIoPipe::OCCConfig* pOccCfg)
{
    (void)pOccCfg;
    LOG_ERR("checkOCCSetting Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}


MBOOL OweDrvImp::enqueOCC(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec)
{
    (void)OccConfigVec;
    LOG_ERR("enqueOCC Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}

MBOOL OweDrvImp::dequeOCC(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec)
{
    (void)OccConfigVec;
    LOG_ERR("dequeOCC Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}

MBOOL OweDrvImp::checkWMFESetting(NSCam::NSIoPipe::OWMFECtrl* pWmfeCtrl)
{
    (void)pWmfeCtrl;
    LOG_ERR("checkWMFESetting Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}

MBOOL OweDrvImp::enqueWMFE(vector<NSCam::NSIoPipe::OWMFEConfig>& WmfeConfigVec)
{
    (void)WmfeConfigVec;
    LOG_ERR("enqueWMFE Error!!, This is OWE Dummy Driver!!");

    return MTRUE;


}

MBOOL OweDrvImp::dequeWMFE(vector<NSCam::NSIoPipe::OWMFEConfig>& WmfeConfigVec)
{
    (void)WmfeConfigVec;
    LOG_ERR("dequeWMFE Error!!, This is OWE Dummy Driver!!");

    return MTRUE;
}


MUINT32 OweDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    (void)Addr;
    (void)caller;
    LOG_ERR("readReg Error!!, This is OWE Dummy Driver!!");

    return 0;
}

//-----------------------------------------------------------------------------
MBOOL OweDrvImp::readRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_ERR("readRegs Error!!, This is OWE Dummy Driver!!");
    return MTRUE;
}


MBOOL OweDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    (void)Addr;
    (void)Data;
    (void)caller;
    LOG_ERR("writeReg Error!!, This is OWE Dummy Driver!!");
    //
    return MTRUE;
}

MBOOL OweDrvImp::writeRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_ERR("writeRegs Error!!, This is OWE Dummy Driver!!");
    return MTRUE;

}

MUINT32 OweDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL OweDrvImp::setRWMode(OWE_DRV_RW_MODE rwMode)
{
    (void)rwMode;
    return MTRUE;
}

