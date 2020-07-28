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
#define LOG_TAG "DpeDrv"


#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_dpe.h"
#include <mtkcam/def/common.h>
#include <dpe_drv.h>
#include <dpeunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(dpe_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (dpe_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class DpeDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    DpeDbgTimer(char const*const pszTitle)
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

MINT32              DpeDrv::m_Fd = -1;
volatile MINT32     DpeDrvImp::m_UserCnt = 0;
DPE_DRV_RW_MODE     DpeDrv::m_regRWMode = DPE_DRV_R_ONLY;
MUINT32*            DpeDrvImp::m_pDpeHwRegAddr = NULL;
android::Mutex      DpeDrvImp::m_DpeInitMutex;


char                DpeDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

DpeDrvImp::DpeDrvImp()
{
    DBG_LOG_CONFIG(drv, dpe_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pDpeHwRegAddr = NULL;
}

DpeDrv* DpeDrv::createInstance()
{
    return DpeDrvImp::getInstance();
}

static DpeDrvImp singleton;

DpeDrv* DpeDrvImp::getInstance()
{
    LOG_WRN("dummy sigleton[0x%p].",(void*)&singleton);

    return &singleton;
}


MBOOL DpeDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_DpeInitMutex);
    //
    if( (userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to use dpe driver\n");
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

    // Open dpe device
    this->m_Fd = open(DPE_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("DPE kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(DPE_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("DPE kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=DPE_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pDpeHwRegAddr = (MUINT32 *) mmap(0, DPE_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, DPE_BASE_HW);

            if(m_pDpeHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("DPE mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=DPE_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, DPE_RESET, NULL) < 0){
        LOG_ERR("DPE Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
    old_cnt = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            munmap(m_pDpeHwRegAddr, DPE_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close dpe device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pDpeHwRegAddr(0x%p)", Result, this->m_UserCnt, (void*)m_pDpeHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL DpeDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_DpeInitMutex);
    //
    if((userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to uninit dpe driver\n");
        return MFALSE;
    }
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in DpeDrv , curUser(%s)",userName);
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

    if(m_pDpeHwRegAddr != MAP_FAILED){
         munmap(m_pDpeHwRegAddr, DPE_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=DPE_DRV_R_ONLY;
    }

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
        LOG_ERR("no dpe device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL DpeDrvImp::waitIrq(DPE_WAIT_IRQ_STRUCT* pWaitIrq)
{
    (void)pWaitIrq;
    LOG_ERR("waitIrq Error!!, This is DPE Dummy Driver!!");

    return MTRUE;
}

MBOOL DpeDrvImp::clearIrq(DPE_CLEAR_IRQ_STRUCT* pClearIrq)
{
    (void)pClearIrq;
    LOG_ERR("clearIrq Error!!, This is DPE Dummy Driver!!");

    return MTRUE;
}



MBOOL DpeDrvImp::waitDPEFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    (void)Status;
    (void)timeoutMs;
    LOG_ERR("waitFrameDone Error!!, This is DPE Dummy Driver!!");

    return MTRUE;
}

MBOOL DpeDrvImp::enqueDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec)
{
    (void) DpeConfigVec;
    LOG_ERR("enque Error!!, This is DPE Dummy Driver!!");

    return MTRUE;
}

MBOOL DpeDrvImp::dequeDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec)
{
    (void) DpeConfigVec;
    LOG_ERR("deque Error!!, This is DPE Dummy Driver!!");

    return MTRUE;
}


MUINT32 DpeDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    (void)Addr;
    (void)caller;
    LOG_ERR("readReg Error!!, This is DPE Dummy Driver!!");

    return 0;
}

//-----------------------------------------------------------------------------
MBOOL DpeDrvImp::readRegs(DPE_DRV_REG_IO_STRUCT* pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_ERR("readRegs Error!!, This is DPE Dummy Driver!!");
    return MTRUE;
}


MBOOL DpeDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    (void)Addr;
    (void)Data;
    (void)caller;
    LOG_WRN("Dummy DPE driver used.");
    //
    return MTRUE;
}

MBOOL DpeDrvImp::writeRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)pRegIo;
    (void)Count;
    (void)caller;
    LOG_WRN("Dummy DPE driver used.");
    return MTRUE;

}

MUINT32 DpeDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL DpeDrvImp::setRWMode(DPE_DRV_RW_MODE rwMode)
{
    LOG_WRN("Dummy DPE driver used.");
    this->m_regRWMode = rwMode;
    return MTRUE;
}

