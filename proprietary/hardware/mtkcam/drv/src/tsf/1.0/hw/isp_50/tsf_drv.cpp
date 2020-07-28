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
#define LOG_TAG "TsfDrv"
#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_tsf.h"
#include <mtkcam/def/common.h>
#include <tsf_drv.h>


#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
#include "tsf_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(tsf_drv);



// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (tsf_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (tsf_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (tsf_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (tsf_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (tsf_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (tsf_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class TsfDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    TsfDbgTimer(char const*const pszTitle)
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

MINT32              TsfDrv::m_Fd = -1;
volatile MINT32     TsfDrvImp::m_UserCnt = 0;
TSF_DRV_RW_MODE     TsfDrv::m_regRWMode = TSF_DRV_R_ONLY;
MUINT32*            TsfDrvImp::m_pTsfHwRegAddr = NULL;
android::Mutex      TsfDrvImp::m_TsfInitMutex;


char                TsfDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

TsfDrvImp::TsfDrvImp()
{
    DBG_LOG_CONFIG(drv, tsf_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pTsfHwRegAddr = NULL;
}

TsfDrv* TsfDrv::createInstance()
{
    return TsfDrvImp::getInstance();
}

static TsfDrvImp singleton;

TsfDrv* TsfDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%lx].", (unsigned long)&singleton);

    return &singleton;
}


MBOOL TsfDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_TsfInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to use tsf driver\n");
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
                this->m_UserName[idx][strlen(userName)]='\0';
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

    // Open tsf device
    this->m_Fd = open(TSF_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("TSF kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(TSF_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("TSF kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=TSF_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pTsfHwRegAddr = (MUINT32 *) mmap(0, TSF_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, TSF_BASE_HW);

            if(m_pTsfHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("TSF mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }

            this->m_regRWMode=TSF_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, TSF_RESET, NULL) < 0){
        LOG_ERR("TSF Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strncpy((char*)this->m_UserName[m_UserCnt],userName, strlen(userName));
    this->m_UserName[m_UserCnt][strlen(userName)]='\0';
    tmp = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            if(m_pTsfHwRegAddr != MAP_FAILED){
                 munmap(m_pTsfHwRegAddr, TSF_REG_RANGE);
            }

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close tsf device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pTsfHwRegAddr(%lx)", Result, m_UserCnt, (unsigned long)m_pTsfHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL TsfDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_TsfInitMutex);
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to uninit tsf driver\n");
        return MFALSE;
    }

    //
    if(m_UserCnt <= 0)
    {
        LOG_ERR("no more user in TsfDrv , curUser(%s)",userName);
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

    if(m_pTsfHwRegAddr != MAP_FAILED){
         munmap(m_pTsfHwRegAddr, TSF_REG_RANGE);
    }

    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=TSF_DRV_R_ONLY;
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
        LOG_ERR("no tsf device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL TsfDrvImp::waitIrq(TSF_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    TSF_IRQ_CLEAR_ENUM OrgClr;
    TsfDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    TSF_WAIT_IRQ_STRUCT waitirq;
    LOG_DBG(" - E. Status(0x%08x),Timeout(%d).\n",pWaitIrq->Status, pWaitIrq->Timeout);
    if(-1 == FD_CHK()){
        return MFALSE;
    }
    waitirq.Clear=pWaitIrq->Clear;
    waitirq.Type=pWaitIrq->Type;
    waitirq.Status=pWaitIrq->Status;
    waitirq.Timeout=pWaitIrq->Timeout;
    waitirq.UserKey=pWaitIrq->UserKey;
    waitirq.ProcessID=pWaitIrq->ProcessID;
    waitirq.bDumpReg=pWaitIrq->bDumpReg;

    while( waitirq.Timeout > 0 )//receive restart system call again
    {
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,TSF_WAIT_IRQ,&waitirq);
        Tb=dbgTmr.getUs();
        if( Ret== (-SIG_ERESTARTSYS) )
        {
            waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
            LOG_INF("ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d us)\n",waitirq.Type, waitirq.Status, waitirq.Timeout);
        }
        else
        {
            break;
        }
    }

    if(Ret < 0) {
        LOG_ERR("TSF_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL TsfDrvImp::clearIrq(TSF_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    TSF_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(TSF_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,TSF_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("TSF_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL TsfDrvImp::waitTSFFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    TSF_WAIT_IRQ_STRUCT WaitIrq;
    MBOOL waitIrqRes = MFALSE;
    WaitIrq.Clear = TSF_IRQ_WAIT_CLEAR;
    WaitIrq.Type = TSF_IRQ_TYPE_INT_TSF_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    DRV_TRACE_BEGIN("TSF waitIrq");

    waitIrqRes = waitIrq(&WaitIrq);

    if (MTRUE == waitIrqRes)
    {
        LOG_INF("TSF Wait Interupt Frame Done Success!!\n");
        DRV_TRACE_END();
        return MTRUE;
    }
    else
    {
        LOG_INF("TSF Wait Interupt Frame Done Fail!!\n");
        DRV_TRACE_END();
        return MFALSE;
    }
}

MUINT32 TsfDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = TSF_REG_RANGE;
    LOG_DBG("+,Tsf_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->TsfRegMutex);
    (void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==TSF_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pTsfHwRegAddr[(Addr>>2)];
    }
    else{
        TSF_REG_IO_STRUCT TsfRegIo;
        TSF_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        TsfRegIo.pData = (TSF_REG_STRUCT*)&RegIo;
        TsfRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, TSF_READ_REGISTER, &TsfRegIo);
        if(Ret < 0)
        {
            LOG_ERR("TSF_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Tsf_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL TsfDrvImp::readRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = TSF_REG_RANGE;
    android::Mutex::Autolock lock(this->TsfRegMutex);
    (void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == TSF_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pTsfHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        TSF_REG_IO_STRUCT TsfRegIo;
        //pRegIo->module = this->m_HWmodule;
        TsfRegIo.pData = (TSF_REG_STRUCT*)pRegIo;
        TsfRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, TSF_READ_REGISTER, &TsfRegIo);
        if(Ret < 0)
        {
            LOG_ERR("TSF_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Tsf_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL TsfDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = TSF_REG_RANGE;
    LOG_DBG("Tsf_write:m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->TsfRegMutex);
    (void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case TSF_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pTsfHwRegAddr[(Addr>>2)] = Data;
            break;
        case TSF_DRV_RW_IOCTL:
            TSF_REG_IO_STRUCT TsfRegIo;
            TSF_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            TsfRegIo.pData = (TSF_REG_STRUCT*)&RegIo;
            TsfRegIo.Count = 1;
            ret = ioctl(this->m_Fd, TSF_WRITE_REGISTER, &TsfRegIo);
            if(ret < 0){
                LOG_ERR("TSF_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case TSF_DRV_R_ONLY:
            LOG_ERR("TSF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->TsfRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL TsfDrvImp::writeRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = TSF_REG_RANGE;
    android::Mutex::Autolock lock(this->TsfRegMutex);
    (void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case TSF_DRV_RW_IOCTL:
            TSF_REG_IO_STRUCT TsfRegIo;
            //pRegIo->module = this->m_HWmodule;
            TsfRegIo.pData = (TSF_REG_STRUCT*)pRegIo;
            TsfRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, TSF_WRITE_REGISTER, &TsfRegIo);
            if(Ret < 0){
                LOG_ERR("TSF_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case TSF_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                {
                    this->m_pTsfHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
                }
            }while(++i<Count);
            break;
        case TSF_DRV_R_ONLY:
            LOG_ERR("TSF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Tsf_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}



MBOOL TsfDrvImp::enqueTSF(TSFConfig const& rTSFConfig)
{
    TsfDbgTimer dbgTmr("enqueTSF");
    MUINT32 Ta=0,Tb=0;
    if (((TSF_MEMALIGNED_MINUS1 & rTSFConfig.DMA_Tsfo.u4BufPA) > 0) || ((TSF_MEMALIGNED_MINUS1 & rTSFConfig.DMA_Tsfi.u4BufPA) > 0) )
    {
        LOG_ERR("Mem addr is not 16x algined InPa(0x%lx), OutPa(0x%lx)\n",(unsigned long)rTSFConfig.DMA_Tsfi.u4BufPA, (unsigned long)rTSFConfig.DMA_Tsfo.u4BufPA);
        return MFALSE;
    }
    /* Reset TSF flow */
    m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] = m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] | 0x10000;
    Ta=dbgTmr.getUs();
    while (((m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] >> 24) & 0x01) != 0x0){
        Tb=dbgTmr.getUs();
        if ( ((Tb-Ta)/1000) >= TSF_RESET_TIMEOUT)
        {
            LOG_ERR("Fail!!, TSF Reset Exceed 2ms !!\n");
            return MFALSE;
        }
    }
    m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] = m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] | 0x00100000;
    m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] = m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] & (0xffeeffee);
    m_pTsfHwRegAddr[(TSF_COEFF_1_OFFSET>>2)] = rTSFConfig.TSF_COEFF_1;
    m_pTsfHwRegAddr[(TSF_COEFF_2_OFFSET>>2)] = rTSFConfig.TSF_COEFF_2;
    m_pTsfHwRegAddr[(TSF_COEFF_3_OFFSET>>2)] = rTSFConfig.TSF_COEFF_3;
    m_pTsfHwRegAddr[(TSF_COEFF_4_OFFSET>>2)] = rTSFConfig.TSF_COEFF_4;

    m_pTsfHwRegAddr[(TSFO_BASE_ADDR_OFFSET>>2)] = rTSFConfig.DMA_Tsfo.u4BufPA;
    m_pTsfHwRegAddr[(TSFO_XSIZE_OFFSET>>2)] = (rTSFConfig.DMA_Tsfo.u4Width -1);
    m_pTsfHwRegAddr[(TSFO_YSIZE_OFFSET>>2)] = (rTSFConfig.DMA_Tsfo.u4Height -1);
    m_pTsfHwRegAddr[(TSFO_STRIDE_OFFSET>>2)] = (TSF_DMA_BUS_SIZE_EN | TSF_DMA_BUS_SIZE_16BIT | rTSFConfig.DMA_Tsfo.u4Stride);

    m_pTsfHwRegAddr[(TSFI_BASE_ADDR_OFFSET>>2)] = rTSFConfig.DMA_Tsfi.u4BufPA;
    m_pTsfHwRegAddr[(TSFI_XSIZE_OFFSET>>2)] = (rTSFConfig.DMA_Tsfi.u4Width -1);
    m_pTsfHwRegAddr[(TSFI_YSIZE_OFFSET>>2)] = (rTSFConfig.DMA_Tsfi.u4Height -1);
    m_pTsfHwRegAddr[(TSFI_STRIDE_OFFSET>>2)] = (TSF_DMA_BUS_SIZE_EN | TSF_DMA_BUS_SIZE_32BIT | rTSFConfig.DMA_Tsfi.u4Stride);

    m_pTsfHwRegAddr[(TSF_INT_EN_OFFSET>>2)] = 0x1; //TSF Interrupt Enable
    m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] = m_pTsfHwRegAddr[(TSF_START_OFFSET>>2)] | 0x11;

    return MTRUE;
}


MUINT32 TsfDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL TsfDrvImp::setRWMode(TSF_DRV_RW_MODE rwMode)
{
    if(rwMode > TSF_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}
