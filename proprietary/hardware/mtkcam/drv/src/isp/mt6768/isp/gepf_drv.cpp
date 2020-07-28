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
#define LOG_TAG "GepfDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_gepf.h"
#include <mtkcam/def/common.h>
#include <drv/gepf_drv.h>
#include <drv/gepfunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(gepf_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (gepf_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (gepf_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (gepf_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (gepf_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (gepf_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (gepf_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class GepfDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    GepfDbgTimer(char const*const pszTitle)
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

MINT32              GepfDrv::m_Fd = -1;
volatile MINT32     GepfDrvImp::m_UserCnt = 0;
GEPF_DRV_RW_MODE    GepfDrv::m_regRWMode = GEPF_DRV_R_ONLY;
MUINT32*            GepfDrvImp::m_pGepfHwRegAddr = NULL;
android::Mutex      GepfDrvImp::m_GepfInitMutex;


char                GepfDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

GepfDrvImp::GepfDrvImp()
{
    DBG_LOG_CONFIG(drv, gepf_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pGepfHwRegAddr = NULL;
}

GepfDrv* GepfDrv::createInstance()
{
    return GepfDrvImp::getInstance();
}

static GepfDrvImp singleton;

GepfDrv* GepfDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%08x].", &singleton);

    return &singleton;
}


MBOOL GepfDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    //
    android::Mutex::Autolock lock(this->m_GepfInitMutex);
    //
    //LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    if( (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE) || (userName==NULL))
    {
        LOG_ERR("Plz add userName if you want to use gepf driver\n");
        return MFALSE;
    }

    LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt > 0)
    {
        if(this->m_UserCnt < MAX_USER_NUMBER){
            strcpy((char*)this->m_UserName[this->m_UserCnt],userName);
            android_atomic_inc(&this->m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    // Open gepf device
    this->m_Fd = open(GEPF_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("GEPF kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(GEPF_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("GEPF kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=GEPF_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pGepfHwRegAddr = (MUINT32 *) mmap(0, GEPF_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, GEPF_BASE_HW);

            if(m_pGepfHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("GEPF mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=GEPF_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, GEPF_RESET, NULL) < 0){
        LOG_ERR("GEPF Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strcpy((char*)this->m_UserName[this->m_UserCnt],userName);
    android_atomic_inc(&this->m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            munmap(m_pGepfHwRegAddr, GEPF_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close gepf device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pGepfHwRegAddr(0x%x)", Result, this->m_UserCnt, m_pGepfHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL GepfDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    //
    android::Mutex::Autolock lock(this->m_GepfInitMutex);
    //
    //LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);
    //
    if( (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE) || (userName==NULL))
    {
        LOG_ERR("Plz add userName if you want to uninit gepf driver\n");
        return MFALSE;
    }

    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in GepfDrv , curUser(%s)",userName);
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
    android_atomic_dec(&this->m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    if(m_pGepfHwRegAddr != MAP_FAILED){
         munmap(m_pGepfHwRegAddr, GEPF_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=GEPF_DRV_R_ONLY;
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
        LOG_ERR("no gepf device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL GepfDrvImp::waitIrq(GEPF_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    GEPF_IRQ_CLEAR_ENUM OrgClr;
    GepfDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    GEPF_WAIT_IRQ_STRUCT waitirq;
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
        Ret = ioctl(this->m_Fd,GEPF_WAIT_IRQ,&waitirq);
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
        LOG_ERR("GEPF_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL GepfDrvImp::clearIrq(GEPF_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    GEPF_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(GEPF_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,GEPF_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("GEPF_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL GepfDrvImp::waitGEPFFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    GEPF_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = GEPF_IRQ_WAIT_CLEAR;
    WaitIrq.Type = GEPF_IRQ_TYPE_INT_GEPF_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    DRV_TRACE_BEGIN("GEPF waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & GEPF_INT_ST)
        {
            LOG_DBG("GEPF Wait Interupt Frame Done Success!!\n");
        }
        DRV_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & GEPF_INT_ST)
        {
            LOG_DBG("GEPF Wait Interupt Frame Done Fail!!\n");
        }
        DRV_TRACE_END();
        return MFALSE;
    }


}
#define BYPASS_REG (0)

#define STRIDE_ALIGNMENT_8(STRIDE) (((STRIDE+7)>>3)<<3)

MBOOL GepfDrvImp::enqueGEPF(vector<NSCam::NSIoPipe::GEPFConfig>& GepfConfigVec)
{

    MINT32 Ret;
    MINT32 min;
    unsigned int num = GepfConfigVec.size();
    NSCam::NSIoPipe::GEPFConfig* pGepfConfig;
    GEPF_Request gepf_Request;
    GEPF_Config gepf_Config[_SUPPORT_MAX_GEPF_FRAME_REQUEST_];

#if 1

    for (unsigned int i=0; i<GepfConfigVec.size(); i++)
    {
        pGepfConfig = &(GepfConfigVec.at(i));
#if !BYPASS_REG
        gepf_Config[i].GEPF_CRT_0 = (pGepfConfig->Gepf_Ctr_Gepf_Mode << 28) | (pGepfConfig->Gepf_Ctr_Iteration_480 << 24) | (pGepfConfig->Gepf_Ctr_Max_Filter_Val_Weight << 8) | (pGepfConfig->Gepf_Ctr_Iteration << 4);
        gepf_Config[i].GEPF_CRT_1 = (pGepfConfig->Gepf_Ctr_Dof_M << 24) | (pGepfConfig->Gepf_Ctr_Lamda << 8) | pGepfConfig->Gepf_Ctr_Focus_Value;
        gepf_Config[i].GEPF_CRT_2 = pGepfConfig->Gepf_Ctr_Lamda_480;
        gepf_Config[i].GEPF_FOCUS_BASE_ADDR = pGepfConfig->Gepf_Focus.u4BufPA;
        gepf_Config[i].GEPF_FOCUS_OFFSET = (STRIDE_ALIGNMENT_8(pGepfConfig->Temp_Pre_Depth.u4Stride) << 16) | pGepfConfig->Gepf_Focus.u4BufSize;
        gepf_Config[i].GEPF_Y_BASE_ADDR = pGepfConfig->Gepf_Y.u4BufPA;
        gepf_Config[i].GEPF_YUV_IMG_SIZE = (pGepfConfig->Gepf_YUV.u4ImgHeight << 16) | pGepfConfig->Gepf_YUV.u4ImgWidth;
        gepf_Config[i].GEPF_UV_BASE_ADDR = pGepfConfig->Gepf_UV.u4BufPA;
        gepf_Config[i].GEPF_DEPTH_BASE_ADDR = pGepfConfig->Gepf_Depth.u4BufPA;
        gepf_Config[i].GEPF_DEPTH_IMG_SIZE = (pGepfConfig->Gepf_Depth.u4ImgHeight << 16) | pGepfConfig->Gepf_Depth.u4ImgWidth;
        gepf_Config[i].GEPF_BLUR_BASE_ADDR = pGepfConfig->Gepf_Blur.u4BufPA;
        gepf_Config[i].GEPF_YUV_BASE_ADDR = pGepfConfig->Gepf_YUV.u4BufPA;
        gepf_Config[i].TEMP_PRE_Y_BASE_ADDR = pGepfConfig->Temp_Pre_Y.u4BufPA;
        gepf_Config[i].TEMP_Y_BASE_ADDR = pGepfConfig->Temp_Y.u4BufPA;
        gepf_Config[i].TEMP_DEPTH_BASE_ADDR = pGepfConfig->Temp_Depth.u4BufPA;
        gepf_Config[i].TEMP_PRE_DEPTH_BASE_ADDR = pGepfConfig->Temp_Pre_Depth.u4BufPA;
        gepf_Config[i].BYPASS_DEPTH_BASE_ADDR = pGepfConfig->Bypass_Depth.u4BufPA;
        gepf_Config[i].BYPASS_DEPTH_WR_BASE_ADDR = pGepfConfig->Bypass_Depth_WR.u4BufPA;
        gepf_Config[i].BYPASS_BLUR_BASE_ADDR = pGepfConfig->Bypass_Blur.u4BufPA;
        gepf_Config[i].BYPASS_BLUR_WR_BASE_ADDR = pGepfConfig->Bypass_Blur_WR.u4BufPA;
        gepf_Config[i].TEMP_BLUR_BASE_ADDR = pGepfConfig->Temp_Blur.u4BufPA;
        gepf_Config[i].TEMP_PRE_BLUR_BASE_ADDR = pGepfConfig->Temp_Pre_Blur.u4BufPA;
        gepf_Config[i].TEMP_BLUR_WR_BASE_ADDR = pGepfConfig->Temp_Blur_WR.u4BufPA;
        gepf_Config[i].TEMP_DEPTH_WR_BASE_ADDR = pGepfConfig->Temp_Depth_WR.u4BufPA;
        gepf_Config[i].GEPF_DEPTH_WR_BASE_ADDR = pGepfConfig->Gepf_Depth_WR.u4BufPA;
        gepf_Config[i].GEPF_BLUR_WR_BASE_ADDR = pGepfConfig->Gepf_Blur_WR.u4BufPA;
        gepf_Config[i].TEMP_CTR_1 = (pGepfConfig->Temp_Ctr_Bypass_En << 16) | pGepfConfig->Temp_Ctr_Coeff_Value;
        gepf_Config[i].GEPF_480_Y_ADDR = pGepfConfig->Gepf_Y_480.u4BufPA;
        gepf_Config[i].GEPF_480_UV_ADDR = pGepfConfig->Gepf_UV_480.u4BufPA;
        gepf_Config[i].GEPF_STRIDE_Y_UV = (STRIDE_ALIGNMENT_8(pGepfConfig->Gepf_UV.u4Stride) << 16) | (STRIDE_ALIGNMENT_8(pGepfConfig->Gepf_Y.u4Stride));
        gepf_Config[i].GEPF_STRIDE_DEPTH = (STRIDE_ALIGNMENT_8(pGepfConfig->Temp_Y.u4Stride) << 16) | (STRIDE_ALIGNMENT_8(pGepfConfig->Gepf_Depth.u4Stride));
        gepf_Config[i].TEMP_STRIDE_DEPTH = (STRIDE_ALIGNMENT_8(pGepfConfig->Temp_Pre_Y.u4Stride) << 16) | (STRIDE_ALIGNMENT_8(pGepfConfig->Temp_Depth.u4Stride));
        gepf_Config[i].GEPF_STRIDE_Y_UV_480 = (STRIDE_ALIGNMENT_8(pGepfConfig->Gepf_UV_480.u4Stride) << 16) | (STRIDE_ALIGNMENT_8(pGepfConfig->Gepf_Y_480.u4Stride));

#endif
    }

    gepf_Request.m_ReqNum = num;
    gepf_Request.m_pGepfConfig = gepf_Config;

    Ret = ioctl(this->m_Fd,GEPF_ENQUE_REQ,&gepf_Request);
    if(Ret < 0)
    {
        LOG_ERR("GEPF_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }


#else
    MINT32 Ret;
    unsigned int num = RscConfigVec.size();
    NSCam::NSIoPipe::RSCConfig* pRscConfig;
    RSC_RSCConfig rsc_Config;
    Ret = ioctl(this->m_Fd,RSC_ENQNUE_NUM,&num);
    if(Ret < 0)
    {
        LOG_ERR("RSC_WMFE_EQNUE_NUM fail(%d)\n", Ret);
        return MFALSE;
    }

    for (unsigned int i=0; i<RscConfigVec.size(); i++)
    {
        pRscConfig = &(RscConfigVec.at(i));

        //WMFE Config 0
        rsc_Config.RSC_WMFE_CTRL_0 = (pRscConfig->Wmfe_Ctrl_0.WmfeFilterSize<<12) | (pRscConfig->Wmfe_Ctrl_0.WmfeDpiFmt<<10) | (pRscConfig->Wmfe_Ctrl_0.WmfeImgiFmt<<8) | (1<<5) | (pRscConfig->Wmfe_Ctrl_0.Wmfe_Enable);
        rsc_Config.RSC_WMFE_SIZE_0 = (pRscConfig->Wmfe_Ctrl_0.Wmfe_Height<<16) | (pRscConfig->Wmfe_Ctrl_0.Wmfe_Width);
        rsc_Config.RSC_WMFE_IMGI_BASE_ADDR_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Imgi.u4BufPA;
        rsc_Config.RSC_WMFE_IMGI_STRIDE_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Imgi.u4Stride;
        rsc_Config.RSC_WMFE_DPI_BASE_ADDR_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Dpi.u4BufPA;
        rsc_Config.RSC_WMFE_DPI_STRIDE_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Dpi.u4Stride;
        rsc_Config.RSC_WMFE_TBLI_BASE_ADDR_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Tbli.u4BufPA;
        rsc_Config.RSC_WMFE_TBLI_STRIDE_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Tbli.u4Stride;
        rsc_Config.RSC_WMFE_DPO_BASE_ADDR_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Dpo.u4BufPA;
        rsc_Config.RSC_WMFE_DPO_STRIDE_0 = pRscConfig->Wmfe_Ctrl_0.Wmfe_Dpo.u4Stride;

        //WMFE Config 1
        rsc_Config.RSC_WMFE_CTRL_1 = (pRscConfig->Wmfe_Ctrl_1.WmfeFilterSize<<12) | (pRscConfig->Wmfe_Ctrl_1.WmfeDpiFmt<<10) | (pRscConfig->Wmfe_Ctrl_1.WmfeImgiFmt<<8) | (1<<5) | (pRscConfig->Wmfe_Ctrl_1.Wmfe_Enable);
        rsc_Config.RSC_WMFE_SIZE_1 = (pRscConfig->Wmfe_Ctrl_1.Wmfe_Height<<16) | (pRscConfig->Wmfe_Ctrl_1.Wmfe_Width);
        rsc_Config.RSC_WMFE_IMGI_BASE_ADDR_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Imgi.u4BufPA;
        rsc_Config.RSC_WMFE_IMGI_STRIDE_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Imgi.u4Stride;
        rsc_Config.RSC_WMFE_DPI_BASE_ADDR_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Dpi.u4BufPA;
        rsc_Config.RSC_WMFE_DPI_STRIDE_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Dpi.u4Stride;
        rsc_Config.RSC_WMFE_TBLI_BASE_ADDR_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Tbli.u4BufPA;
        rsc_Config.RSC_WMFE_TBLI_STRIDE_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Tbli.u4Stride;
        rsc_Config.RSC_WMFE_DPO_BASE_ADDR_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Dpo.u4BufPA;
        rsc_Config.RSC_WMFE_DPO_STRIDE_1 = pRscConfig->Wmfe_Ctrl_1.Wmfe_Dpo.u4Stride;

        //WMFE Config 2
        if ((RSC_DEFAULT_UT == g_RSC_UnitTest_Num) || (RSC_TESTCASE_UT_0 == g_RSC_UnitTest_Num))
        {
            rsc_Config.RSC_WMFE_CTRL_2 = (pRscConfig->Wmfe_Ctrl_2.WmfeFilterSize<<12) | (pRscConfig->Wmfe_Ctrl_2.WmfeDpiFmt<<10) | (pRscConfig->Wmfe_Ctrl_2.WmfeImgiFmt<<8) | (1<<5)  | (1<<4) | (pRscConfig->Wmfe_Ctrl_2.Wmfe_Enable);
        }
        else
        {
            rsc_Config.RSC_WMFE_CTRL_2 = (pRscConfig->Wmfe_Ctrl_2.WmfeFilterSize<<12) | (pRscConfig->Wmfe_Ctrl_2.WmfeDpiFmt<<10) | (pRscConfig->Wmfe_Ctrl_2.WmfeImgiFmt<<8) | (1<<5) | (pRscConfig->Wmfe_Ctrl_2.Wmfe_Enable);
        }
        rsc_Config.RSC_WMFE_SIZE_2 = (pRscConfig->Wmfe_Ctrl_2.Wmfe_Height<<16) | (pRscConfig->Wmfe_Ctrl_2.Wmfe_Width);
        rsc_Config.RSC_WMFE_IMGI_BASE_ADDR_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Imgi.u4BufPA;
        rsc_Config.RSC_WMFE_IMGI_STRIDE_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Imgi.u4Stride;
        rsc_Config.RSC_WMFE_DPI_BASE_ADDR_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Dpi.u4BufPA;
        rsc_Config.RSC_WMFE_DPI_STRIDE_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Dpi.u4Stride;
        rsc_Config.RSC_WMFE_TBLI_BASE_ADDR_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Tbli.u4BufPA;
        rsc_Config.RSC_WMFE_TBLI_STRIDE_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Tbli.u4Stride;
        rsc_Config.RSC_WMFE_DPO_BASE_ADDR_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Dpo.u4BufPA;
        rsc_Config.RSC_WMFE_DPO_STRIDE_2 = pRscConfig->Wmfe_Ctrl_2.Wmfe_Dpo.u4Stride;

        Ret = ioctl(this->m_Fd,RSC_ENQUE,&rsc_Config);
        if(Ret < 0)
        {
            LOG_ERR("RSC_WMFE_ENQUE_REQ fail(%d)\n", Ret);
            return MFALSE;
        }
    }
#endif
    return MTRUE;


}

MBOOL GepfDrvImp::dequeGEPF(vector<NSCam::NSIoPipe::GEPFConfig>& GepfConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::GEPFConfig GepfConfig;
    GEPF_Request gepf_Request;
    GEPF_Config gepf_Config[_SUPPORT_MAX_GEPF_FRAME_REQUEST_];
    gepf_Request.m_pGepfConfig = gepf_Config;
#if 1
    Ret = ioctl(this->m_Fd,GEPF_DEQUE_REQ,&gepf_Request);
    if(Ret < 0)
    {
        LOG_ERR("GEPF_DEQUE_REQ fail(%d)\n", Ret);
        printf("GEPF_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeGEPF num:%d\n", gepf_Request.m_ReqNum);
    for (unsigned int i=0; i< gepf_Request.m_ReqNum; i++)
    {
        GepfConfigVec.push_back(GepfConfig);
    }

#else
    MINT32 Ret;
    unsigned int num;
    NSCam::NSIoPipe::GEPFConfig GepfConfig;
    GEPF_GEPFConfig gepf_Config;
    Ret = ioctl(this->m_Fd,GEPF_DEQUE_NUM,&num);
    if(Ret < 0)
    {
        LOG_ERR("GEPF_DEQUE_NUM fail(%d)\n", Ret);
        printf("GEPF_DEQUE_NUM fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeGEPF num:%d\n", num);
    for (unsigned int i=0; i< num; i++)
    {
        Ret = ioctl(this->m_Fd,GEPF_DEQUE,&gepf_Config);
        if(Ret < 0)
        {
            LOG_ERR("GEPF_DEQUE fail(%d)\n", Ret);
            printf("GEPF_DEQUE fail(%d)\n", Ret);
            return MFALSE;
        }
        GepfConfigVec.push_back(GepfConfig);
    }
#endif

    return MTRUE;
}


MUINT32 GepfDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = GEPF_REG_RANGE;
    LOG_DBG("+,Gepf_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->GepfRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==GEPF_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pGepfHwRegAddr[(Addr>>2)];
    }
    else{
        GEPF_REG_IO_STRUCT GepfRegIo;
        GEPF_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        GepfRegIo.pData = (GEPF_REG_STRUCT*)&RegIo;
        GepfRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, GEPF_READ_REGISTER, &GepfRegIo);
        if(Ret < 0)
        {
            LOG_ERR("GEPF_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Gepf_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL GepfDrvImp::readRegs(GEPF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = GEPF_REG_RANGE;
    android::Mutex::Autolock lock(this->GepfRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == GEPF_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pGepfHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        GEPF_REG_IO_STRUCT GepfRegIo;
        //pRegIo->module = this->m_HWmodule;
        GepfRegIo.pData = (GEPF_REG_STRUCT*)pRegIo;
        GepfRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, GEPF_READ_REGISTER, &GepfRegIo);
        if(Ret < 0)
        {
            LOG_ERR("GEPF_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Gepf_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL GepfDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = GEPF_REG_RANGE;
    LOG_DBG("Gepf_write:m_regRWMode(0x%x),(0x%x,0x%x)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->GepfRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case GEPF_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pGepfHwRegAddr[(Addr>>2)] = Data;
            break;
        case GEPF_DRV_RW_IOCTL:
            GEPF_REG_IO_STRUCT GepfRegIo;
            GEPF_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            GepfRegIo.pData = (GEPF_REG_STRUCT*)&RegIo;
            GepfRegIo.Count = 1;
            ret = ioctl(this->m_Fd, GEPF_WRITE_REGISTER, &GepfRegIo);
            if(ret < 0){
                LOG_ERR("GEPF_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case GEPF_DRV_R_ONLY:
            LOG_ERR("GEPF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->GepfRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL GepfDrvImp::writeRegs(GEPF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = GEPF_REG_RANGE;
    android::Mutex::Autolock lock(this->GepfRegMutex);
    //(void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case GEPF_DRV_RW_IOCTL:
            GEPF_REG_IO_STRUCT GepfRegIo;
            //pRegIo->module = this->m_HWmodule;
            GepfRegIo.pData = (GEPF_REG_STRUCT*)pRegIo;
            GepfRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, GEPF_WRITE_REGISTER, &GepfRegIo);
            if(Ret < 0){
                LOG_ERR("GEPF_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case GEPF_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pGepfHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case GEPF_DRV_R_ONLY:
            LOG_ERR("GEPF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Gepf_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 GepfDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL GepfDrvImp::setRWMode(GEPF_DRV_RW_MODE rwMode)
{
    if(rwMode > GEPF_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

