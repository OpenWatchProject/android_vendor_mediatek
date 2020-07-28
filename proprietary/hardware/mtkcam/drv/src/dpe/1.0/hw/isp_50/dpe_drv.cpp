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
#include <mtkcam/utils/std/Trace.h>
#include <dpe_drv.h>
#include <dpeunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "dpe_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(dpe_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_DEBUG  ) { printf(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_INFO   ) { printf(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (dpe_drv_DbgLogEnable_ERROR  ) { printf(fmt, ##arg); } } while(0)
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
    LOG_DBG("singleton[0x%lx].", (unsigned long)&singleton);

    return &singleton;
}


MBOOL DpeDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_DpeInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to use dpe driver\n");
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
                strncpy((char*)this->m_UserName[idx],userName,strlen(userName));
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
    strncpy((char*)this->m_UserName[m_UserCnt],userName,strlen(userName));
    this->m_UserName[m_UserCnt][strlen(userName)]='\0';
    tmp = android_atomic_inc(&m_UserCnt);

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

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pDpeHwRegAddr(0x%lx)", Result, m_UserCnt, (unsigned long)m_pDpeHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL DpeDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_DpeInitMutex);
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to uninit dpe driver\n");
        return MFALSE;
    }

    //
    if(m_UserCnt <= 0)
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
    tmp = android_atomic_dec(&m_UserCnt);

    if(m_UserCnt > 0)    // If there are still users, exit.
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
        LOG_ERR("no dpe device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL DpeDrvImp::waitIrq(DPE_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    DPE_IRQ_CLEAR_ENUM OrgClr;
    DpeDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    DPE_WAIT_IRQ_STRUCT waitirq;
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
        Ret = ioctl(this->m_Fd,DPE_WAIT_IRQ,&waitirq);
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
        LOG_ERR("DPE_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL DpeDrvImp::clearIrq(DPE_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    DPE_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(DPE_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,DPE_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("DPE_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL DpeDrvImp::waitDPEFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    DPE_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = DPE_IRQ_WAIT_CLEAR;
    WaitIrq.Type = DPE_IRQ_TYPE_INT_DPE_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    CAM_TRACE_BEGIN("DPE waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & DPE_DVE_INT_ST)
        {
            LOG_DBG("DPE Wait DVE Interupt Frame Done Success!!\n");
        }
        else
        {
            LOG_DBG("DPE Wait WMFE Interupt Frame Done Success!!\n");
        }
        CAM_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & DPE_DVE_INT_ST)
        {
            LOG_DBG("DPE Wait DVE Interupt Frame Done Fail!!\n");
        }
        else
        {
            LOG_DBG("DPE Wait WMFE Interupt Frame Done Fail!!\n");
        }
        CAM_TRACE_END();
        return MFALSE;
    }


}

MBOOL DpeDrvImp::checkDVESetting(NSCam::NSIoPipe::DVEConfig* pDveCfg)
{
    if (pDveCfg->Dve_Horz_Ds_Mode == 0)
    {
        //IMGI
        if (pDveCfg->Dve_Imgi_l_Fmt != DPE_IMGI_Y_FMT)
        {
            if (pDveCfg->Dve_Imgi_l.u4Stride < (((pDveCfg->Dve_Org_Width+7)/8)*2))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+7)/8)*2. horz_ds(0x%x),imgi_l_fmt(0x%x), imgi_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l_Fmt, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        else
        {
            if (pDveCfg->Dve_Imgi_l.u4Stride < ((pDveCfg->Dve_Org_Width+7)/8))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+7)/8). horz_ds(0x%x),imgi_l_fmt(0x%x), imgi_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l_Fmt, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        if (pDveCfg->Dve_Imgi_r_Fmt != DPE_IMGI_Y_FMT)
        {
            if (pDveCfg->Dve_Imgi_r.u4Stride < (((pDveCfg->Dve_Org_Width+7)/8)*2))
            {
                LOG_ERR("imgi stride >= ((DVE_ORG_WIDTH+7)/8)*2. horz_ds(0x%x),imgi_r_fmt(0x%x), imgi_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r_Fmt, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        else
        {
            if (pDveCfg->Dve_Imgi_r.u4Stride < ((pDveCfg->Dve_Org_Width+7)/8))
            {
                LOG_ERR("imgi stride >= ((DVE_ORG_WIDTH+7)/8). horz_ds(0x%x),imgi_r_fmt(0x%x), imgi_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r_Fmt, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        //Check IMGI Buffer Size
        if (pDveCfg->Dve_Imgi_l.u4BufSize < (pDveCfg->Dve_Imgi_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("imgi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),imgi_l_size(0x%x), imgi_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l.u4BufSize, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Imgi_r.u4BufSize < (pDveCfg->Dve_Imgi_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("imgi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),imgi_r_size(0x%x), imgi_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r.u4BufSize, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //DVI
        if (pDveCfg->Dve_Dvi_l.u4Stride < (((pDveCfg->Dve_Org_Width+7)/8)*2))
        {
            LOG_ERR("dvi stride must >= ((DVE_ORG_WIDTH+7)/8)*2. horz_ds(0x%x),dvi_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvi_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvi_r.u4Stride < (((pDveCfg->Dve_Org_Width+7)/8)*2))
        {
            LOG_ERR("dvi stride must >= ((DVE_ORG_WIDTH+7)/8)*2. horz_ds(0x%x),dvi_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvi_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check DVI Buffer Size
        if (pDveCfg->Dve_Dvi_l.u4BufSize < (pDveCfg->Dve_Dvi_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvi_l_size(0x%x), dvi_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvi_l.u4BufSize, pDveCfg->Dve_Dvi_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvi_r.u4BufSize < (pDveCfg->Dve_Dvi_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvi_r_size(0x%x), dvi_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvi_r.u4BufSize, pDveCfg->Dve_Dvi_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }



        //MASKI
        if (pDveCfg->Dve_Mask_En == true)
        {
            if (pDveCfg->Dve_Maski_l.u4Stride < ((pDveCfg->Dve_Org_Width+7)/8))
            {
                LOG_ERR("maski stride must >= ((DVE_ORG_WIDTH+7)/8). horz_ds(0x%x),maski_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Maski_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }

            if (pDveCfg->Dve_Maski_r.u4Stride < ((pDveCfg->Dve_Org_Width+7)/8))
            {
                LOG_ERR("maski stride must >= ((DVE_ORG_WIDTH+7)/8). horz_ds(0x%x),maski_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Maski_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
            //Check MASKI Buffer Size
            if (pDveCfg->Dve_Maski_l.u4BufSize < (pDveCfg->Dve_Maski_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
            {
                LOG_ERR("maski buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),maski_l_size(0x%x), maski_l_stride(0x%x), ord_h(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Maski_l.u4BufSize, pDveCfg->Dve_Maski_l.u4Stride, pDveCfg->Dve_Org_Height);
                return MFALSE;
            }
            if (pDveCfg->Dve_Maski_r.u4BufSize < (pDveCfg->Dve_Maski_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
            {
                LOG_ERR("maski buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),maski_r_size(0x%x), maski_r_stride(0x%x), ord_h(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Maski_r.u4BufSize, pDveCfg->Dve_Maski_r.u4Stride, pDveCfg->Dve_Org_Height);
                return MFALSE;
            }
        }

        //DVO
        if (pDveCfg->Dve_Dvo_l.u4Stride < ((((((pDveCfg->Dve_Org_Width+7)/8)*2)+15)>>4)<<4))
        {
            LOG_ERR("dvo stride must >= ((((DVE_ORG_WIDTH+7)/8)*2+15)>>4)<<4. horz_ds(0x%x),dvo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvo_r.u4Stride < ((((((pDveCfg->Dve_Org_Width+7)/8)*2)+15)>>4)<<4))
        {
            LOG_ERR("dvo stride must >= ((((DVE_ORG_WIDTH+7)/8)*2+15)>>4)<<4. horz_ds(0x%x),dvo_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvo_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check DVO Buffer Size
        if (pDveCfg->Dve_Dvo_l.u4BufSize < (pDveCfg->Dve_Dvo_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvo_l_size(0x%x), dvo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvo_l.u4BufSize, pDveCfg->Dve_Dvo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvo_r.u4BufSize < (pDveCfg->Dve_Dvo_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvo_r_size(0x%x), dvo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvo_r.u4BufSize, pDveCfg->Dve_Dvo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //CONFO
        if (pDveCfg->Dve_Confo_l.u4Stride < (((((pDveCfg->Dve_Org_Width+7)/8)+15)>>4)<<4))
        {
            LOG_ERR("confo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),confo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Confo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Confo_r.u4Stride < (((((pDveCfg->Dve_Org_Width+7)/8)+15)>>4)<<4))
        {
            LOG_ERR("confo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),confo_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Confo_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check CONFO Buffer Size
        if (pDveCfg->Dve_Confo_l.u4BufSize < (pDveCfg->Dve_Confo_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("confo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),confo_l_size(0x%x), confo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Confo_l.u4BufSize, pDveCfg->Dve_Confo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Confo_r.u4BufSize < (pDveCfg->Dve_Confo_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("confo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),confo_r_size(0x%x), confo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Confo_r.u4BufSize, pDveCfg->Dve_Confo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //RESPO
        if (pDveCfg->Dve_Respo_l.u4Stride < (((((pDveCfg->Dve_Org_Width+7)/8)+15)>>4)<<4))
        {
            LOG_ERR("respo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),respo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Respo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Respo_r.u4Stride < (((((pDveCfg->Dve_Org_Width+7)/8)+15)>>4)<<4))
        {
            LOG_ERR("respo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),respo_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Respo_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check RESPO Buffer Size
        if (pDveCfg->Dve_Respo_l.u4BufSize < (pDveCfg->Dve_Respo_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("respo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),respo_l_size(0x%x), respo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Respo_l.u4BufSize, pDveCfg->Dve_Respo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Respo_r.u4BufSize < (pDveCfg->Dve_Respo_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("respo buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),respo_r_size(0x%x), respo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Respo_r.u4BufSize, pDveCfg->Dve_Respo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
    }
    else
    {
        //IMGI
        if (pDveCfg->Dve_Imgi_l_Fmt != DPE_IMGI_Y_FMT)
        {
            if (pDveCfg->Dve_Imgi_l.u4Stride < (((pDveCfg->Dve_Org_Width+3)/4)*2))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+3)/4)*2. horz_ds(0x%x),imgi_l_fmt(0x%x), imgi_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l_Fmt, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        else
        {
            if (pDveCfg->Dve_Imgi_l.u4Stride < ((pDveCfg->Dve_Org_Width+3)/4))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+3)/4). horz_ds(0x%x),imgi_l_fmt(0x%x), imgi_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l_Fmt, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }

        }
        if (pDveCfg->Dve_Imgi_r_Fmt != DPE_IMGI_Y_FMT)
        {
            if (pDveCfg->Dve_Imgi_r.u4Stride < (((pDveCfg->Dve_Org_Width+3)/4)*2))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+3)/4)*2. horz_ds(0x%x),imgi_r_fmt(0x%x), imgi_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r_Fmt, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
        }
        else
        {
            if (pDveCfg->Dve_Imgi_r.u4Stride < ((pDveCfg->Dve_Org_Width+3)/4))
            {
                LOG_ERR("imgi stride must >= ((DVE_ORG_WIDTH+3)/4). horz_ds(0x%x),imgi_r_fmt(0x%x), imgi_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r_Fmt, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }

        }
        //Check IMGI Buffer Size
        if (pDveCfg->Dve_Imgi_l.u4BufSize < (pDveCfg->Dve_Imgi_l.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("imgi buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),imgi_l_size(0x%x), imgi_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_l.u4BufSize, pDveCfg->Dve_Imgi_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Imgi_r.u4BufSize < (pDveCfg->Dve_Imgi_r.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("imgi buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),imgi_r_size(0x%x), imgi_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Imgi_r.u4BufSize, pDveCfg->Dve_Imgi_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //DVI
        if (pDveCfg->Dve_Dvi_l.u4Stride < (((pDveCfg->Dve_Org_Width+3)/4)*2))
        {
            LOG_ERR("dvi stride must >= ((DVE_ORG_WIDTH+3)/4)*2. horz_ds(0x%x),dvi_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvi_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvi_r.u4Stride < (((pDveCfg->Dve_Org_Width+3)/4)*2))
        {
            LOG_ERR("dvi stride must >= ((DVE_ORG_WIDTH+3)/4)*2. horz_ds(0x%x),dvi_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvi_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check DVI Buffer Size
        if (pDveCfg->Dve_Dvi_l.u4BufSize < (pDveCfg->Dve_Dvi_l.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvi_l_size(0x%x), dvi_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvi_l.u4BufSize, pDveCfg->Dve_Dvi_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvi_r.u4BufSize < (pDveCfg->Dve_Dvi_r.u4Stride*((pDveCfg->Dve_Org_Height+7)/8)))
        {
            LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+7)/8). horz_ds(0x%x),dvi_r_size(0x%x), dvi_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvi_r.u4BufSize, pDveCfg->Dve_Dvi_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //MASKI
        if (pDveCfg->Dve_Mask_En == true)
        {
            if (pDveCfg->Dve_Maski_l.u4Stride < ((pDveCfg->Dve_Org_Width+3)/4))
            {
                LOG_ERR("maski stride must >= ((DVE_ORG_WIDTH+3)/4). horz_ds(0x%x),maski_l_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Maski_l.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
            
            if (pDveCfg->Dve_Maski_r.u4Stride < ((pDveCfg->Dve_Org_Width+3)/4))
            {
                LOG_ERR("maski stride must >= ((DVE_ORG_WIDTH+3)/4). horz_ds(0x%x),maski_r_stride(0x%x), ord_w(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Maski_r.u4Stride, pDveCfg->Dve_Org_Width);
                return MFALSE;
            }
            //Check MASKI Buffer Size
            if (pDveCfg->Dve_Maski_l.u4BufSize < (pDveCfg->Dve_Maski_l.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
            {
                LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),maski_l_size(0x%x), maski_l_stride(0x%x), ord_h(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Maski_l.u4BufSize, pDveCfg->Dve_Maski_l.u4Stride, pDveCfg->Dve_Org_Height);
                return MFALSE;
            }
            if (pDveCfg->Dve_Maski_r.u4BufSize < (pDveCfg->Dve_Maski_r.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
            {
                LOG_ERR("dvi buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),maski_r_size(0x%x), maski_r_stride(0x%x), ord_h(0x%x)", 
                    pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Maski_r.u4BufSize, pDveCfg->Dve_Maski_r.u4Stride, pDveCfg->Dve_Org_Height);
                return MFALSE;
            }

        }
        //DVO
        if (pDveCfg->Dve_Dvo_l.u4Stride < ((((((pDveCfg->Dve_Org_Width+3)/4)*2)+15)>>4)<<4))
        {
            LOG_ERR("dvo stride must >= ((((DVE_ORG_WIDTH+3)/4)*2+15)>>4)<<4. horz_ds(0x%x),dvo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvo_r.u4Stride < ((((((pDveCfg->Dve_Org_Width+3)/4)*2)+15)>>4)<<4))
        {
           LOG_ERR("dvo stride must >= ((((DVE_ORG_WIDTH+3)/4)*2+15)>>4)<<4. horz_ds(0x%x),dvo_r_stride(0x%x), ord_w(0x%x)", 
               pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Dvo_r.u4Stride, pDveCfg->Dve_Org_Width);
           return MFALSE;
        }
        //Check DVO Buffer Size
        if (pDveCfg->Dve_Dvo_l.u4BufSize < (pDveCfg->Dve_Dvo_l.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),dvo_l_size(0x%x), dvo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvo_l.u4BufSize, pDveCfg->Dve_Dvo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Dvo_r.u4BufSize < (pDveCfg->Dve_Dvo_r.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),dvo_r_size(0x%x), dvo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Dvo_r.u4BufSize, pDveCfg->Dve_Dvo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //CONFO
        if (pDveCfg->Dve_Confo_l.u4Stride < (((((pDveCfg->Dve_Org_Width+3)/4)+15)>>4)<<4))
        {
            LOG_ERR("stride must >= ((((DVE_ORG_WIDTH+3)/4)+15)>>4)<<4. horz_ds(0x%x),confo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Confo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Confo_r.u4Stride < (((((pDveCfg->Dve_Org_Width+3)/4)+15)>>4)<<4))
        {
            LOG_ERR("stride must >= ((((DVE_ORG_WIDTH+3)/4)+15)>>4)<<4. horz_ds(0x%x),confo_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Confo_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }

        //Check CONFO Buffer Size
        if (pDveCfg->Dve_Confo_l.u4BufSize < (pDveCfg->Dve_Confo_l.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),confo_l_size(0x%x), confo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Confo_l.u4BufSize, pDveCfg->Dve_Confo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Confo_r.u4BufSize < (pDveCfg->Dve_Confo_r.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("dvo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),confo_r_size(0x%x), confo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Confo_r.u4BufSize, pDveCfg->Dve_Confo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

        //RESPO
        if (pDveCfg->Dve_Respo_l.u4Stride < (((((pDveCfg->Dve_Org_Width+3)/4)+15)>>4)<<4))
        {
            LOG_ERR("respo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),respo_l_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Respo_l.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        if (pDveCfg->Dve_Respo_r.u4Stride < (((((pDveCfg->Dve_Org_Width+3)/4)+15)>>4)<<4))
        {
            LOG_ERR("respo stride must >= ((((DVE_ORG_WIDTH+7)/8)+15)>>4)<<4. horz_ds(0x%x),respo_r_stride(0x%x), ord_w(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode, pDveCfg->Dve_Respo_r.u4Stride, pDveCfg->Dve_Org_Width);
            return MFALSE;
        }
        //Check RESPO Buffer Size
        if (pDveCfg->Dve_Respo_l.u4BufSize < (pDveCfg->Dve_Respo_l.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("respo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),respo_l_size(0x%x), respo_l_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Respo_l.u4BufSize, pDveCfg->Dve_Respo_l.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }
        if (pDveCfg->Dve_Respo_r.u4BufSize < (pDveCfg->Dve_Respo_r.u4Stride*((pDveCfg->Dve_Org_Height+3)/4)))
        {
            LOG_ERR("respo buf size must >= stride * ((DVE_ORG_HEIGHT+3)/4). horz_ds(0x%x),respo_r_size(0x%x), respo_r_stride(0x%x), ord_h(0x%x)", 
                pDveCfg->Dve_Horz_Ds_Mode,pDveCfg->Dve_Respo_r.u4BufSize, pDveCfg->Dve_Respo_r.u4Stride, pDveCfg->Dve_Org_Height);
            return MFALSE;
        }

    }

    return MTRUE;
}


MBOOL DpeDrvImp::enqueDVE(vector<NSCam::NSIoPipe::DVEConfig>& DveConfigVec)
{
    MINT32 Ret;
    unsigned int num = DveConfigVec.size();
    NSCam::NSIoPipe::DVEConfig* pDveConfig;
    DPE_DVERequest dpe_dveRequest;
    DPE_DVEConfig dpe_DveConfig[_SUPPORT_MAX_DPE_FRAME_REQUEST_];

    for (unsigned int i=0; i<DveConfigVec.size(); i++)
    {
        pDveConfig = &(DveConfigVec.at(i));
        if (MFALSE == checkDVESetting(pDveConfig))
        {
            LOG_ERR("DVE Enque setting is wrong!!idx(%d), numb(%d)\n", i, num);
            return MFALSE;
        }

        dpe_DveConfig[i].DPE_DVE_CTRL = (pDveConfig->DVE_RESPO_SEL<<29) | (pDveConfig->DVE_CONFO_SEL<<28) | (0x3920000) | (pDveConfig->Dve_Imgi_r_Fmt<<10) | (pDveConfig->Dve_Imgi_l_Fmt<<8) | (pDveConfig->Dve_Mask_En <<6) | (pDveConfig->Dve_r_Bbox_En <<5) | (pDveConfig->Dve_l_Bbox_En <<4) | (pDveConfig->Dve_Skp_Pre_Dv <<3) \
        | ((pDveConfig->Dve_Vert_Ds_Mode & 0x01)<< 2) | ((pDveConfig->Dve_Horz_Ds_Mode & 0x01)<< 1);

        dpe_DveConfig[i].DPE_DVE_ORG_L_HORZ_BBOX = (pDveConfig->Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT << 16) | pDveConfig->Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT;
        dpe_DveConfig[i].DPE_DVE_ORG_L_VERT_BBOX = (pDveConfig->Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM << 16) | pDveConfig->Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP;
        dpe_DveConfig[i].DPE_DVE_ORG_R_HORZ_BBOX = (pDveConfig->Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT << 16) | pDveConfig->Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT;
        dpe_DveConfig[i].DPE_DVE_ORG_R_VERT_BBOX = (pDveConfig->Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM << 16) | pDveConfig->Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP;
        dpe_DveConfig[i].DPE_DVE_ORG_SIZE = (pDveConfig->Dve_Org_Height <<16) | pDveConfig->Dve_Org_Width;
        dpe_DveConfig[i].DPE_DVE_ORG_SR_0 = (pDveConfig->Dve_Org_Horz_Sr_1 <<16) | pDveConfig->Dve_Org_Horz_Sr_0;
        dpe_DveConfig[i].DPE_DVE_ORG_SR_1 = pDveConfig->Dve_Org_Vert_Sr_0;
        dpe_DveConfig[i].DPE_DVE_ORG_SV = (pDveConfig->Dve_Org_Start_Vert_Sv <<16) | pDveConfig->Dve_Org_Start_Horz_Sv;
        dpe_DveConfig[i].DPE_DVE_CAND_NUM = pDveConfig->Dve_Cand_Num;
        dpe_DveConfig[i].DPE_DVE_CAND_SEL_0 = (pDveConfig->Dve_Cand_3.DVE_CAND_SEL << 24) | (pDveConfig->Dve_Cand_2.DVE_CAND_SEL << 16) | (pDveConfig->Dve_Cand_1.DVE_CAND_SEL << 8) | (pDveConfig->Dve_Cand_0.DVE_CAND_SEL);
        dpe_DveConfig[i].DPE_DVE_CAND_SEL_1 = (pDveConfig->Dve_Cand_7.DVE_CAND_SEL << 24) | (pDveConfig->Dve_Cand_6.DVE_CAND_SEL << 16) | (pDveConfig->Dve_Cand_5.DVE_CAND_SEL << 8) | (pDveConfig->Dve_Cand_4.DVE_CAND_SEL);
        dpe_DveConfig[i].DPE_DVE_CAND_SEL_2 = 0x10081206;
        dpe_DveConfig[i].DPE_DVE_CAND_TYPE_0 = (pDveConfig->Dve_Cand_7.DVE_CAND_TYPE << 28) | (pDveConfig->Dve_Cand_6.DVE_CAND_TYPE << 24) | (pDveConfig->Dve_Cand_5.DVE_CAND_TYPE << 20) | (pDveConfig->Dve_Cand_4.DVE_CAND_TYPE << 16) | (pDveConfig->Dve_Cand_3.DVE_CAND_TYPE << 12) | (pDveConfig->Dve_Cand_2.DVE_CAND_TYPE << 8) | (pDveConfig->Dve_Cand_1.DVE_CAND_TYPE << 4) | (pDveConfig->Dve_Cand_0.DVE_CAND_TYPE);
        dpe_DveConfig[i].DPE_DVE_CAND_TYPE_1 = 0x2121;
        dpe_DveConfig[i].DPE_DVE_RAND_LUT = (pDveConfig->Dve_Rand_Lut_3 << 24) | (pDveConfig->Dve_Rand_Lut_2 << 16) | (pDveConfig->Dve_Rand_Lut_1 << 8) | (pDveConfig->Dve_Rand_Lut_0);
        dpe_DveConfig[i].DPE_DVE_GMV = (pDveConfig->DVE_VERT_GMV << 16) | pDveConfig->DVE_HORZ_GMV;
        dpe_DveConfig[i].DPE_DVE_DV_INI = pDveConfig->Dve_Horz_Dv_Ini;
        dpe_DveConfig[i].DPE_DVE_BLK_VAR_CTRL = (pDveConfig->Dve_Coft_Shift << 16) | pDveConfig->Dve_Corner_Th;
        dpe_DveConfig[i].DPE_DVE_SMTH_LUMA_CTRL = (pDveConfig->Dve_Smth_Luma_Th_1 << 24) | (pDveConfig->Dve_Smth_Luma_Th_0 << 16) | (pDveConfig->Dve_Smth_Luma_Ada_Base << 8) | pDveConfig->Dve_Smth_Luma_Horz_Pnlty_Sel;
        dpe_DveConfig[i].DPE_DVE_SMTH_DV_CTRL = (pDveConfig->Dve_Smth_Dv_Th_1 << 28) | (pDveConfig->Dve_Smth_Dv_Th_0 << 20) | (pDveConfig->Dve_Smth_Dv_Ada_Base << 12) | (pDveConfig->Dve_Smth_Dv_Vert_Pnlty_Sel << 8) | (pDveConfig->Dve_Smth_Dv_Horz_Pnlty_Sel << 4) | pDveConfig->Dve_Smth_Dv_Mode;
        dpe_DveConfig[i].DPE_DVE_ORD_CTRL_0 = (pDveConfig->Dve_Ord_Th << 16) | (pDveConfig->Dve_Ord_Coring << 4) | (pDveConfig->Dve_Ord_DownSample_En << 8) | pDveConfig->Dve_Ord_Pnlty_Sel;
        dpe_DveConfig[i].DPE_DVE_ORD_CTRL_1 = (pDveConfig->Dve_Ord_As_TH);
        dpe_DveConfig[i].DPE_DVE_TYPE_CTRL_0 =  (pDveConfig->Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL << 28) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL << 24) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL << 20) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL << 16) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL << 8) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL << 4) | pDveConfig->Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL;
        dpe_DveConfig[i].DPE_DVE_TYPE_CTRL_1 =  (pDveConfig->Dve_Type_Penality_Ctrl.DVE_RAND_COST << 24) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_GMV_COST << 20) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_PREV_COST << 16) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_NBR_COST << 12) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_REFINE_COST << 8) | (pDveConfig->Dve_Type_Penality_Ctrl.DVE_TMPR_COST << 4) | pDveConfig->Dve_Type_Penality_Ctrl.DVE_SPTL_COST;

        dpe_DveConfig[i].DPE_DVE_ORD_AS_MASK_0 = pDveConfig->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_0;
        dpe_DveConfig[i].DPE_DVE_ORD_AS_MASK_1 = pDveConfig->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_1;
        dpe_DveConfig[i].DPE_DVE_ORD_AS_MASK_2 = pDveConfig->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_2;
        dpe_DveConfig[i].DPE_DVE_ORD_AS_MASK_3 = pDveConfig->Dve_Ord_As_Mask.DVE_ORD_AS_MASK_3;

        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_0 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_0;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_1 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_1;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_2 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_2;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_3 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_3;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_4 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_4;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_5 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_5;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_A_6 = pDveConfig->Dve_Ord_Ref_Mask_A.DVE_ORD_REF_MASK_A_6;

        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_0 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_0;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_1 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_1;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_2 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_2;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_3 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_3;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_4 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_4;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_5 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_5;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_B_6 = pDveConfig->Dve_Ord_Ref_Mask_B.DVE_ORD_REF_MASK_B_6;

        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_0 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_0;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_1 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_1;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_2 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_2;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_3 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_3;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_4 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_4;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_5 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_5;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_C_6 = pDveConfig->Dve_Ord_Ref_Mask_C.DVE_ORD_REF_MASK_C_6;

        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_0 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_0;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_1 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_1;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_2 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_2;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_3 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_3;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_4 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_4;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_5 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_5;
        dpe_DveConfig[i].DPE_DVE_ORD_REF_MASK_D_6 = pDveConfig->Dve_Ord_Ref_Mask_D.DVE_ORD_REF_MASK_D_6;

        dpe_DveConfig[i].DPE_DVE_IMGI_L_BASE_ADDR =  pDveConfig->Dve_Imgi_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_IMGI_L_STRIDE =  pDveConfig->Dve_Imgi_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_IMGI_L_BUFSIZE = pDveConfig->Dve_Imgi_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_IMGI_R_BASE_ADDR =  pDveConfig->Dve_Imgi_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_IMGI_R_STRIDE =  pDveConfig->Dve_Imgi_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_IMGI_R_BUFSIZE = pDveConfig->Dve_Imgi_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_DVI_L_BASE_ADDR =  pDveConfig->Dve_Dvi_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_DVI_L_STRIDE =  pDveConfig->Dve_Dvi_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_DVI_L_BUFSIZE =  pDveConfig->Dve_Dvi_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_DVI_R_BASE_ADDR =  pDveConfig->Dve_Dvi_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_DVI_R_STRIDE =  pDveConfig->Dve_Dvi_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_DVI_R_BUFSIZE =  pDveConfig->Dve_Dvi_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_MASKI_L_BASE_ADDR =  pDveConfig->Dve_Maski_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_MASKI_L_STRIDE =  pDveConfig->Dve_Maski_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_MASKI_L_BUFSIZE =  pDveConfig->Dve_Maski_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_MASKI_R_BASE_ADDR =  pDveConfig->Dve_Maski_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_MASKI_R_STRIDE =  pDveConfig->Dve_Maski_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_MASKI_R_BUFSIZE =  pDveConfig->Dve_Maski_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_DVO_L_BASE_ADDR =  pDveConfig->Dve_Dvo_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_DVO_L_STRIDE =  pDveConfig->Dve_Dvo_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_DVO_L_BUFSIZE =  pDveConfig->Dve_Dvo_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_DVO_R_BASE_ADDR =  pDveConfig->Dve_Dvo_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_DVO_R_STRIDE =  pDveConfig->Dve_Dvo_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_DVO_R_BUFSIZE =  pDveConfig->Dve_Dvo_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_CONFO_L_BASE_ADDR =  pDveConfig->Dve_Confo_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_CONFO_L_STRIDE =  pDveConfig->Dve_Confo_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_CONFO_L_BUFSIZE =  pDveConfig->Dve_Confo_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_CONFO_R_BASE_ADDR =  pDveConfig->Dve_Confo_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_CONFO_R_STRIDE =  pDveConfig->Dve_Confo_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_CONFO_R_BUFSIZE =  pDveConfig->Dve_Confo_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_RESPO_L_BASE_ADDR =  pDveConfig->Dve_Respo_l.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_RESPO_L_STRIDE =  pDveConfig->Dve_Respo_l.u4Stride;
        dpe_DveConfig[i].DPE_DVE_RESPO_L_BUFSIZE =  pDveConfig->Dve_Respo_l.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_RESPO_R_BASE_ADDR =  pDveConfig->Dve_Respo_r.u4BufPA;
        dpe_DveConfig[i].DPE_DVE_RESPO_R_STRIDE =  pDveConfig->Dve_Respo_r.u4Stride;
        dpe_DveConfig[i].DPE_DVE_RESPO_R_BUFSIZE =  pDveConfig->Dve_Respo_r.u4BufSize;
        dpe_DveConfig[i].DPE_DVE_IS_SECURE = pDveConfig->Dve_Is_Secure;

        LOG_INF("frm(%d), ctrl(0x%x), l_horz_bbox(0x%x), l_vert_bbox(0x%x), r_horz_bbox(0x%x), r_vert_bbox(0x%x), dve_org_size(0x%x)\n\
            ,imgi_l_addr(0x%lx), imgi_l_stride(0x%x), imgi_l_size(0x%x), imgi_r_addr(0x%lx), imgi_r_stride(0x%x), imgi_r_size(0x%x)\n\
            ,dvi_l_addr(0x%lx), dvi_l_stride(0x%x), dvi_l_size(0x%x), dvi_r_addr(0x%lx), dvi_r_stride(0x%x), dvi_r_size(0x%x)\n\
            ,maski_l_addr(0x%lx), maski_l_stride(0x%x), maski_l_size(0x%x), maski_r_addr(0x%lx), maski_r_stride(0x%x), maski_r_size(0x%x)\n\
            ,dvo_l_addr(0x%lx), dvo_l_stride(0x%x), dvo_l_size(0x%x), dvo_r_addr(0x%lx), dvo_r_stride(0x%x), dvo_r_size(0x%x)\n\
            ,confo_l_addr(0x%lx), confo_l_stride(0x%x), confo_l_size(0x%x), confo_r_addr(0x%lx), confo_r_stride(0x%x), confo_r_size(0x%x)\n\
            ,respo_l_addr(0x%lx), respo_l_stride(0x%x), respo_l_size(0x%x), respo_r_addr(0x%lx), respo_r_stride(0x%x), respo_r_size(0x%x)\n\
            ,is_secure(%d)",i, dpe_DveConfig[i].DPE_DVE_CTRL,
            dpe_DveConfig[i].DPE_DVE_ORG_L_HORZ_BBOX, dpe_DveConfig[i].DPE_DVE_ORG_L_VERT_BBOX,dpe_DveConfig[i].DPE_DVE_ORG_R_HORZ_BBOX, dpe_DveConfig[i].DPE_DVE_ORG_R_VERT_BBOX, dpe_DveConfig[i].DPE_DVE_ORG_SIZE,
            (unsigned long)pDveConfig->Dve_Imgi_l.u4BufPA, pDveConfig->Dve_Imgi_l.u4Stride, pDveConfig->Dve_Imgi_l.u4BufSize, (unsigned long)pDveConfig->Dve_Imgi_r.u4BufPA, pDveConfig->Dve_Imgi_r.u4Stride, pDveConfig->Dve_Imgi_r.u4BufSize,
            (unsigned long)pDveConfig->Dve_Dvi_l.u4BufPA, pDveConfig->Dve_Dvi_l.u4Stride, pDveConfig->Dve_Dvi_l.u4BufSize, (unsigned long)pDveConfig->Dve_Dvi_r.u4BufPA, pDveConfig->Dve_Dvi_r.u4Stride, pDveConfig->Dve_Dvi_r.u4BufSize,
            (unsigned long)pDveConfig->Dve_Maski_l.u4BufPA, pDveConfig->Dve_Maski_l.u4Stride, pDveConfig->Dve_Maski_l.u4BufSize, (unsigned long)pDveConfig->Dve_Maski_r.u4BufPA, pDveConfig->Dve_Maski_r.u4Stride, pDveConfig->Dve_Maski_r.u4BufSize,
            (unsigned long)pDveConfig->Dve_Dvo_l.u4BufPA, pDveConfig->Dve_Dvo_l.u4Stride, pDveConfig->Dve_Dvo_l.u4BufSize, (unsigned long)pDveConfig->Dve_Dvo_r.u4BufPA, pDveConfig->Dve_Dvo_r.u4Stride, pDveConfig->Dve_Dvo_r.u4BufSize,
            (unsigned long)pDveConfig->Dve_Confo_l.u4BufPA, pDveConfig->Dve_Confo_l.u4Stride, pDveConfig->Dve_Confo_l.u4BufSize, (unsigned long)pDveConfig->Dve_Confo_r.u4BufPA, pDveConfig->Dve_Confo_r.u4Stride, pDveConfig->Dve_Confo_r.u4BufSize,
            (unsigned long)pDveConfig->Dve_Respo_l.u4BufPA, pDveConfig->Dve_Respo_l.u4Stride, pDveConfig->Dve_Respo_l.u4BufSize, (unsigned long)pDveConfig->Dve_Respo_r.u4BufPA, pDveConfig->Dve_Respo_r.u4Stride, pDveConfig->Dve_Respo_r.u4BufSize,
            pDveConfig->Dve_Is_Secure);

    }

    dpe_dveRequest.m_ReqNum = num;
    dpe_dveRequest.m_pDpeConfig = dpe_DveConfig;

    Ret = ioctl(this->m_Fd,DPE_DVE_ENQUE_REQ,&dpe_dveRequest);
    if(Ret < 0)
    {
        LOG_ERR("DPE_DVE_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    return MTRUE;


}

MBOOL DpeDrvImp::dequeDVE(vector<NSCam::NSIoPipe::DVEConfig>& DveConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::DVEConfig DveConfig;
    DPE_DVERequest dpe_dveRequest;
    DPE_DVEConfig dpe_DveConfig[_SUPPORT_MAX_DPE_FRAME_REQUEST_];
    dpe_dveRequest.m_pDpeConfig = dpe_DveConfig;

    Ret = ioctl(this->m_Fd,DPE_DVE_DEQUE_REQ,&dpe_dveRequest);
    if(Ret < 0)
    {
        LOG_ERR("DPE_DVE_DEQUE_REQ fail(%d)\n", Ret);
        printf("DPE_DVE_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeDVE num:%d\n", dpe_dveRequest.m_ReqNum);

    for (unsigned int i=0; i< dpe_dveRequest.m_ReqNum; i++)
    {
        DveConfig.Dve_Horz_Sv = (dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_STA_0 & 0x3ff);
        DveConfig.Dve_Vert_Sv = (dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_STA_0 >> 16) & 0x3f;

        DveConfig.Dve_Imgi_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_IMGI_L_BASE_ADDR;
        DveConfig.Dve_Imgi_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_IMGI_L_STRIDE;
        DveConfig.Dve_Imgi_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_IMGI_R_BASE_ADDR;
        DveConfig.Dve_Imgi_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_IMGI_R_STRIDE;
        DveConfig.Dve_Dvi_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVI_L_BASE_ADDR;
        DveConfig.Dve_Dvi_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVI_L_STRIDE;
        DveConfig.Dve_Dvi_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVI_R_BASE_ADDR;
        DveConfig.Dve_Dvi_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVI_R_STRIDE;
        DveConfig.Dve_Maski_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_MASKI_L_BASE_ADDR;
        DveConfig.Dve_Maski_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_MASKI_L_STRIDE;
        DveConfig.Dve_Maski_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_MASKI_R_BASE_ADDR;
        DveConfig.Dve_Maski_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_MASKI_R_STRIDE;

        DveConfig.Dve_Dvo_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVO_L_BASE_ADDR;
        DveConfig.Dve_Dvo_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVO_L_STRIDE;
        DveConfig.Dve_Dvo_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVO_R_BASE_ADDR;
        DveConfig.Dve_Dvo_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_DVO_R_STRIDE;

        DveConfig.Dve_Confo_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_CONFO_L_BASE_ADDR;
        DveConfig.Dve_Confo_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_CONFO_L_STRIDE;
        DveConfig.Dve_Confo_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_CONFO_R_BASE_ADDR;
        DveConfig.Dve_Confo_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_CONFO_R_STRIDE;
        DveConfig.Dve_Respo_l.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_RESPO_L_BASE_ADDR;
        DveConfig.Dve_Respo_l.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_RESPO_L_STRIDE;
        DveConfig.Dve_Respo_r.u4BufPA = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_RESPO_R_BASE_ADDR;
        DveConfig.Dve_Respo_r.u4Stride = dpe_dveRequest.m_pDpeConfig[i].DPE_DVE_RESPO_R_STRIDE;
        DveConfigVec.push_back(DveConfig);

    }

    return MTRUE;
}

MBOOL DpeDrvImp::checkWMFESetting(NSCam::NSIoPipe::WMFECtrl* pWmfeCtrl)
{
    //WMFE IMGI
    MBOOL scale = MFALSE;

    if (pWmfeCtrl->WmfeImgiFmt !=DPE_IMGI_Y_FMT)
    {
        if (pWmfeCtrl->Wmfe_Imgi.u4Stride < (pWmfeCtrl->Wmfe_Width*2))
        {
            LOG_ERR("wmfi imgi stride must stride >= WMFE_WIDTH_0*2. wmfe_imgi_fmt(%d), wmfe_imgi_stride(0x%x), wmfe_width(0x%x)", 
                pWmfeCtrl->WmfeImgiFmt, pWmfeCtrl->Wmfe_Imgi.u4Stride, pWmfeCtrl->Wmfe_Width);
            return MFALSE;
        }
    }
    else
    {
        if (pWmfeCtrl->Wmfe_Imgi.u4Stride < pWmfeCtrl->Wmfe_Width)
        {
            LOG_ERR("wmfi imgi stride must stride >= WMFE_WIDTH_0. wmfe_imgi_fmt(%d), wmfe_imgi_stride(0x%x), wmfe_width(0x%x)", 
                pWmfeCtrl->WmfeImgiFmt, pWmfeCtrl->Wmfe_Imgi.u4Stride, pWmfeCtrl->Wmfe_Width);
            return MFALSE;
        }
    }
    //Check WMFE IMGI Buffer Size
    if (pWmfeCtrl->Wmfe_Imgi.u4BufSize < (pWmfeCtrl->Wmfe_Imgi.u4Stride*pWmfeCtrl->Wmfe_Height))
    {
        LOG_ERR("wmfi imgi buffer size >= stride * WMFE_HEIGHT_0. wmfe_imgi_bufsize(0x%x), wmfe_imgi_stride(0x%x), wmfe_height(0x%x)", pWmfeCtrl->Wmfe_Imgi.u4BufSize, pWmfeCtrl->Wmfe_Imgi.u4Stride, pWmfeCtrl->Wmfe_Height);
        return MFALSE;
    }

    //WMFE DPI
    if (pWmfeCtrl->Wmfe_Dpo.u4Stride != pWmfeCtrl->Wmfe_Dpi.u4Stride)
    {
        if (pWmfeCtrl->Wmfe_Dpo.u4Stride == (2*pWmfeCtrl->Wmfe_Dpi.u4Stride))
        {
            scale = MTRUE;
        }
        else
        {
            LOG_ERR("the size of Wmfe_Ctrl Dpo and Dpi is not support!! Dpo Stride:(%d), Dpi Stride:(%d)\n", pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4Stride);
        }
    }
    if (scale ==  MTRUE) //For WMFE_MODE_0 = 1,
    {
        if (pWmfeCtrl->Wmfe_Dpi.u4Stride < ((pWmfeCtrl->Wmfe_Width+1)/2))
        {
            LOG_ERR("wmfi dpi stride must stride >= (WMFE_WIDTH_0+1)/2. scale(0x%x), wmfe_dpo_stride(0x%x), wmfe_dpi_stride(0x%x), wmfe_width(0x%x)", 
                scale, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Width);
            return MFALSE;
        }        
        //Check WMFE DPI Buffer Size
        if (pWmfeCtrl->Wmfe_Dpi.u4BufSize < (pWmfeCtrl->Wmfe_Dpi.u4Stride*((pWmfeCtrl->Wmfe_Height+1)/2)))
        {
            LOG_ERR("wmfi dpi buffer size >= stride * ((WMFE_HEIGHT_0+1)/2). scale(0x%x), wmfe_dpo_stride(0x%x), wmfe_dpi_bufsize(0x%x), wmfe_dpi_stride(0x%x), wmfe_height(0x%x)", scale, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4BufSize, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Height);
            return MFALSE;
        }

    }
    else
    {
        if (pWmfeCtrl->WmfeDpiFmt !=WMFE_DPI_D_FMT)
        {
            if (pWmfeCtrl->Wmfe_Dpi.u4Stride < (pWmfeCtrl->Wmfe_Width*2))
            {
                LOG_ERR("wmfi dpi stride must stride >= WMFE_WIDTH_0*2. scale(0x%x), wmfe_dpo_stride(0x%x), wmfe_dpi_fmt(0x%x), wmfe_dpi_stride(0x%x), wmfe_width(0x%x)", 
                    scale, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->WmfeDpiFmt, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Width);
                return MFALSE;
            }
        }
        else
        {
            if (pWmfeCtrl->Wmfe_Dpi.u4Stride < pWmfeCtrl->Wmfe_Width)
            {
                LOG_ERR("wmfi dpi stride must stride >= WMFE_WIDTH_0. scale(0x%x), wmfe_dpo_stride(0x%x), wmfe_dpi_fmt(0x%x), wmfe_dpi_stride(0x%x), wmfe_width(0x%x)", 
                    scale, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->WmfeDpiFmt, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Width);
                return MFALSE;
            }
        }
        //Check WMFE DPI Buffer Size
        if (pWmfeCtrl->Wmfe_Dpi.u4BufSize < (pWmfeCtrl->Wmfe_Dpi.u4Stride*pWmfeCtrl->Wmfe_Height))
        {
            LOG_ERR("wmfi dpi buffer size >= stride * WMFE_HEIGHT_0. scale(0x%x), pWmfeCtrl->Wmfe_Dpo.u4Stride(0x%x), wmfe_dpi_bufsize(0x%x), wmfe_dpi_stride(0x%x), wmfe_height(0x%x)", scale, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4BufSize, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Height);
            return MFALSE;
        }

   }
    //WMFE TBLI
    if (pWmfeCtrl->Wmfe_Tbli.u4Stride < WMFE_TBLI_SIZE)
    {
        LOG_ERR("wmfi tbli stride must stride >= 256. wmfe_tbli_stride(0x%x)", pWmfeCtrl->Wmfe_Tbli.u4Stride);
        return MFALSE;
    }
    if (pWmfeCtrl->Wmfe_Tbli.u4BufSize < WMFE_TBLI_SIZE)
    {
        LOG_ERR("wmfi tbli buffer size >= 256. wmfe_tbli_stride(0x%x)", pWmfeCtrl->Wmfe_Tbli.u4BufSize);
        return MFALSE;
    }

    //WMFE MASKI
    if (pWmfeCtrl->Wmfe_Mask_En == true)
    {
        if (pWmfeCtrl->Wmfe_Maski.u4Stride < pWmfeCtrl->Wmfe_Width)
        {
            LOG_ERR("wmfi maski stride must stride >= WMFE_WIDTH_0. wmfe_maski_stride(0x%x), wmfe_width(0x%x)", pWmfeCtrl->Wmfe_Maski.u4Stride, pWmfeCtrl->Wmfe_Width);
            return MFALSE;
        }
        if (pWmfeCtrl->Wmfe_Maski.u4BufSize < (pWmfeCtrl->Wmfe_Maski.u4Stride*pWmfeCtrl->Wmfe_Height))
        {
            LOG_ERR("wmfi maski buffer size >= stride * WMFE_HEIGHT_0. wmfe_maski_bufsize(0x%x), wmfe_maski_stride(0x%x), wmfe_height(0x%x)", pWmfeCtrl->Wmfe_Maski.u4BufSize, pWmfeCtrl->Wmfe_Maski.u4Stride, pWmfeCtrl->Wmfe_Height);
            return MFALSE;
        }
    }

    //WMFE DPO
    if (pWmfeCtrl->Wmfe_Dpo.u4Stride < (((pWmfeCtrl->Wmfe_Width+15)>>4)<<4))
    {
        LOG_ERR("wmfi dpo stride must stride >= ((WMFE_WIDTH_0+15)>>4)<<4. wmfe_dpo_stride(0x%x), wmfe_width(0x%x)", pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Width);
        return MFALSE;
    }
    if (pWmfeCtrl->Wmfe_Dpo.u4BufSize < (pWmfeCtrl->Wmfe_Dpo.u4Stride*pWmfeCtrl->Wmfe_Height))
    {
        LOG_ERR("wmfi dpo buffer size >= stride * WMFE_HEIGHT_0. wmfe_dpo_bufsize(0x%x), wmfe_dpo_stride(0x%x), wmfe_height(0x%x)", pWmfeCtrl->Wmfe_Dpo.u4BufSize, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Height);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL DpeDrvImp::enqueWMFE(vector<NSCam::NSIoPipe::WMFEConfig>& WmfeConfigVec)
{

    MINT32 Ret;
    MBOOL scale = MFALSE;
    unsigned int num = WmfeConfigVec.size();
    NSCam::NSIoPipe::WMFEConfig* pWmfeConfig;
    NSCam::NSIoPipe::WMFECtrl* pWmfeCtrl;
    DPE_WMFERequest dpe_wmfeRequest;
    DPE_WMFEConfig dpe_WmfeConfig[_SUPPORT_MAX_DPE_FRAME_REQUEST_];

    for (unsigned int i=0; i<WmfeConfigVec.size(); i++)
    {
        pWmfeConfig = &(WmfeConfigVec.at(i));
        if(pWmfeConfig->mWMFECtrlVec.size() >=6)
        {
            LOG_ERR("The Maximum Frame Num is Five!! VecNum:%zu\n", pWmfeConfig->mWMFECtrlVec.size());
            return MFALSE;
        }
        dpe_WmfeConfig[i].WmfeCtrlSize = pWmfeConfig->mWMFECtrlVec.size();
        
        for (unsigned int j=0; j<pWmfeConfig->mWMFECtrlVec.size(); j++)
        {
            pWmfeCtrl = &(pWmfeConfig->mWMFECtrlVec.at(j));

            if (MFALSE == checkWMFESetting(pWmfeCtrl))
            {
                LOG_ERR("WMFE Enque setting is wrong!!idx(%d), wmfctrl idx(%d), numb(%d)\n", i, j, num);
                return MFALSE;
            }
            //WMFE Config 0
            scale = MFALSE;
            if (pWmfeCtrl->Wmfe_Dpo.u4Stride != pWmfeCtrl->Wmfe_Dpi.u4Stride)
            {
                if (pWmfeCtrl->Wmfe_Dpo.u4Stride == (2*pWmfeCtrl->Wmfe_Dpi.u4Stride))
                {
                    scale = MTRUE;
                }
                else
                {
                    LOG_ERR("the size of Wmfe_Ctrl_%d Dpo and Dpi is not support!! Dpo Stride:(%d), Dpi Stride:(%d)\n", j, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4Stride);
                }
            }
						if (j<=4)
					  {
	            if ((j==4) &&((DPE_DEFAULT_UT == g_DPE_UnitTest_Num) || (DPE_TESTCASE_UT_0 == g_DPE_UnitTest_Num)))
	            {
	                dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_CTRL = ((pWmfeCtrl->Wmfe_Mask_Value & 0xff)<<24) | ((pWmfeCtrl->Wmfe_Mask_Mode & 0x01)<<18) | (pWmfeCtrl->WmfeVertScOrd<<17) | (pWmfeCtrl->WmfeHorzScOrd<<16) | (pWmfeCtrl->WmfeFilterSize<<12) | (pWmfeCtrl->WmfeDpiFmt<<10) | (pWmfeCtrl->WmfeImgiFmt<<8) | (pWmfeCtrl->Wmfe_Mask_En<<7) | (pWmfeCtrl->Wmfe_Dpnd_En<<6) | (1<<5) | (1<<4) | (pWmfeCtrl->Wmfe_Enable);
	            }
	            else
	            {
	                dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_CTRL = ((pWmfeCtrl->Wmfe_Mask_Value & 0xff)<<24) | ((pWmfeCtrl->Wmfe_Mask_Mode & 0x01)<<18) | (pWmfeCtrl->WmfeVertScOrd<<17) | (pWmfeCtrl->WmfeHorzScOrd<<16) | (pWmfeCtrl->WmfeFilterSize<<12) | (pWmfeCtrl->WmfeDpiFmt<<10) | (pWmfeCtrl->WmfeImgiFmt<<8) | (pWmfeCtrl->Wmfe_Mask_En<<7) | (pWmfeCtrl->Wmfe_Dpnd_En<<6) | (1<<5) | (scale << 4) | (pWmfeCtrl->Wmfe_Enable);
	            }
            }
            else
            {
	            LOG_ERR("The Maximum Frame Num is Four:%d!! \n", j);
	            return MFALSE;
            }

            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_SIZE = (pWmfeCtrl->Wmfe_Height<<16) | (pWmfeCtrl->Wmfe_Width);
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_IMGI_BASE_ADDR = pWmfeCtrl->Wmfe_Imgi.u4BufPA;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_IMGI_STRIDE = pWmfeCtrl->Wmfe_Imgi.u4Stride;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_DPI_BASE_ADDR = pWmfeCtrl->Wmfe_Dpi.u4BufPA;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_DPI_STRIDE = pWmfeCtrl->Wmfe_Dpi.u4Stride;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_TBLI_BASE_ADDR = pWmfeCtrl->Wmfe_Tbli.u4BufPA;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_TBLI_STRIDE = pWmfeCtrl->Wmfe_Tbli.u4Stride;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_MASKI_BASE_ADDR = pWmfeCtrl->Wmfe_Maski.u4BufPA;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_MASKI_STRIDE = pWmfeCtrl->Wmfe_Maski.u4Stride;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_DPO_BASE_ADDR = pWmfeCtrl->Wmfe_Dpo.u4BufPA;
            dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_DPO_STRIDE = pWmfeCtrl->Wmfe_Dpo.u4Stride;

            LOG_INF("frm(%d), which wmfe ctrl(0x%x), wmfe size(0x%x)\n\
                ,imgi_l_addr(0x%lx), imgi_l_stride(0x%x), imgi_l_size(0x%x),\n\
                ,dvi_l_addr(0x%lx), dvi_l_stride(0x%x), dvi_l_size(0x%x),\n\
                ,maski_l_addr(0x%lx), maski_l_stride(0x%x), maski_l_size(0x%x),\n\
                ,dvo_l_addr(0x%lx), dvo_l_stride(0x%x), dvo_l_size(0x%x),\n\
                ,confo_l_addr(0x%lx), confo_l_stride(0x%x), confo_l_size(0x%x),\n",i, j, dpe_WmfeConfig[i].WmfeCtrl[j].WMFE_SIZE,
                (unsigned long)pWmfeCtrl->Wmfe_Imgi.u4BufPA, pWmfeCtrl->Wmfe_Imgi.u4Stride, pWmfeCtrl->Wmfe_Imgi.u4BufSize,
                (unsigned long)pWmfeCtrl->Wmfe_Dpi.u4BufPA, pWmfeCtrl->Wmfe_Dpi.u4Stride, pWmfeCtrl->Wmfe_Dpi.u4BufSize,
                (unsigned long)pWmfeCtrl->Wmfe_Tbli.u4BufPA, pWmfeCtrl->Wmfe_Tbli.u4Stride, pWmfeCtrl->Wmfe_Tbli.u4BufSize,
                (unsigned long)pWmfeCtrl->Wmfe_Maski.u4BufPA, pWmfeCtrl->Wmfe_Maski.u4Stride, pWmfeCtrl->Wmfe_Maski.u4BufSize,
                (unsigned long)pWmfeCtrl->Wmfe_Dpo.u4BufPA, pWmfeCtrl->Wmfe_Dpo.u4Stride, pWmfeCtrl->Wmfe_Dpo.u4BufSize);

        }
        
    }

    dpe_wmfeRequest.m_ReqNum = num;
    dpe_wmfeRequest.m_pWmfeConfig = dpe_WmfeConfig;

    Ret = ioctl(this->m_Fd,DPE_WMFE_ENQUE_REQ,&dpe_wmfeRequest);
    if(Ret < 0)
    {
        LOG_ERR("DPE_WMFE_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    return MTRUE;


}

MBOOL DpeDrvImp::dequeWMFE(vector<NSCam::NSIoPipe::WMFEConfig>& WmfeConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::WMFEConfig WmfeConfig;
    DPE_WMFERequest dpe_wmfeRequest;
    DPE_WMFEConfig dpe_WmfeConfig[_SUPPORT_MAX_DPE_FRAME_REQUEST_];
    dpe_wmfeRequest.m_pWmfeConfig = dpe_WmfeConfig;

    Ret = ioctl(this->m_Fd,DPE_WMFE_DEQUE_REQ,&dpe_wmfeRequest);
    if(Ret < 0)
    {
        LOG_ERR("DPE_WMFE_DEQUE_REQ fail(%d)\n", Ret);
        printf("DPE_WMFE_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeWMFE num:%d\n", dpe_wmfeRequest.m_ReqNum);
    for (unsigned int i=0; i< dpe_wmfeRequest.m_ReqNum; i++)
    {
        WmfeConfigVec.push_back(WmfeConfig);
    }

    return MTRUE;
}


MUINT32 DpeDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    (void)caller;
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = DPE_REG_RANGE;
    LOG_DBG("+,Dpe_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->DpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==DPE_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pDpeHwRegAddr[(Addr>>2)];
    }
    else{
        DPE_REG_IO_STRUCT DpeRegIo;
        DPE_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        DpeRegIo.pData = (DPE_REG_STRUCT*)&RegIo;
        DpeRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, DPE_READ_REGISTER, &DpeRegIo);
        if(Ret < 0)
        {
            LOG_ERR("DPE_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Dpe_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL DpeDrvImp::readRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)caller;
    MINT32 Ret;
    MUINT32 legal_range = DPE_REG_RANGE;
    android::Mutex::Autolock lock(this->DpeRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == DPE_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pDpeHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        DPE_REG_IO_STRUCT DpeRegIo;
        //pRegIo->module = this->m_HWmodule;
        DpeRegIo.pData = (DPE_REG_STRUCT*)pRegIo;
        DpeRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, DPE_READ_REGISTER, &DpeRegIo);
        if(Ret < 0)
        {
            LOG_ERR("DPE_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Dpe_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL DpeDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    (void)caller;
    MINT32 ret=0;
    MUINT32 legal_range = DPE_REG_RANGE;
    LOG_DBG("Dpe_write:m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->DpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case DPE_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pDpeHwRegAddr[(Addr>>2)] = Data;
            break;
        case DPE_DRV_RW_IOCTL:
            DPE_REG_IO_STRUCT DpeRegIo;
            DPE_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            DpeRegIo.pData = (DPE_REG_STRUCT*)&RegIo;
            DpeRegIo.Count = 1;
            ret = ioctl(this->m_Fd, DPE_WRITE_REGISTER, &DpeRegIo);
            if(ret < 0){
                LOG_ERR("DPE_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case DPE_DRV_R_ONLY:
            LOG_ERR("DPE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->DpeRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL DpeDrvImp::writeRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)caller;
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = DPE_REG_RANGE;
    android::Mutex::Autolock lock(this->DpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case DPE_DRV_RW_IOCTL:
            DPE_REG_IO_STRUCT DpeRegIo;
            //pRegIo->module = this->m_HWmodule;
            DpeRegIo.pData = (DPE_REG_STRUCT*)pRegIo;
            DpeRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, DPE_WRITE_REGISTER, &DpeRegIo);
            if(Ret < 0){
                LOG_ERR("DPE_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case DPE_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pDpeHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case DPE_DRV_R_ONLY:
            LOG_ERR("DPE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Dpe_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 DpeDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL DpeDrvImp::setRWMode(DPE_DRV_RW_MODE rwMode)
{
    if(rwMode > DPE_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

