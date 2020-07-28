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

#include "camera_rsc.h"
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
    LOG_DBG("singleton[0x%p].",(void*)&singleton);

    return &singleton;
}


MBOOL RscDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
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
            this->m_UserName[this->m_UserCnt][strlen(userName)] = '\0';
            old_cnt = android_atomic_inc(&m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    // Open rsc device
    this->m_Fd = open(RSC_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("RSC kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(RSC_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("RSC kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=RSC_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pRscHwRegAddr = (MUINT32 *) mmap(0, RSC_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, RSC_BASE_HW);

            if(m_pRscHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("RSC mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=RSC_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, RSC_RESET, NULL) < 0){
        LOG_ERR("RSC Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
    this->m_UserName[this->m_UserCnt][strlen(userName)] = '\0';
    old_cnt = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            munmap(m_pRscHwRegAddr, RSC_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close rsc device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pRscHwRegAddr(0x%p)", Result, this->m_UserCnt, (void*)m_pRscHwRegAddr);
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

    if(m_pRscHwRegAddr != MAP_FAILED){
         munmap(m_pRscHwRegAddr, RSC_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=RSC_DRV_R_ONLY;
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
        LOG_ERR("no rsc device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL RscDrvImp::waitIrq(RSC_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    RSC_IRQ_CLEAR_ENUM OrgClr;
    RscDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    RSC_WAIT_IRQ_STRUCT waitirq;
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
        Ret = ioctl(this->m_Fd,RSC_WAIT_IRQ,&waitirq);
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
        LOG_ERR("RSC_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL RscDrvImp::clearIrq(RSC_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    RSC_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(RSC_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,RSC_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("RSC_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL RscDrvImp::waitRSCFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    RSC_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = RSC_IRQ_WAIT_CLEAR;
    WaitIrq.Type = RSC_IRQ_TYPE_INT_RSC_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    DRV_TRACE_BEGIN("RSC waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & RSC_INT_ST)
        {
            LOG_DBG("RSC Wait Interupt Frame Done Success!!\n");
        }
        DRV_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & RSC_INT_ST)
        {
            LOG_DBG("RSC Wait Interupt Frame Done Fail!!\n");
        }
        DRV_TRACE_END();
        return MFALSE;
    }


}
#define BYPASS_REG (0)

#define BVO_STRIDE_MIN(W) (((((W+1)/2)+15)>>4)<<4)
#define MVO_STRIDE_MIN(W) (((((W+1)/2)+6 )/ 7)*16)
#define APLI_LAST_LINE_OF_RSS_FORMAT_SUPPORT
MBOOL RscDrvImp::enqueRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec)
{

    MINT32 Ret;
    MUINT32 min;
    MBOOL scale = MFALSE;
    unsigned int num = RscConfigVec.size();
    NSCam::NSIoPipe::RSCConfig* pRscConfig;
    RSC_Request rsc_Request;
    RSC_Config rsc_Config[_SUPPORT_MAX_RSC_FRAME_REQUEST_];
    MUINT32 h_rsc_c, h_rsc_p;

#if 1

    for (unsigned int i=0; i<RscConfigVec.size(); i++)
    {
        pRscConfig = &(RscConfigVec.at(i));
#if !BYPASS_REG
	if(pRscConfig->Rsc_Size_Width == 0 || pRscConfig->Rsc_Size_Height == 0)
            LOG_ERR("Zero input size is used, H/V=(%d/%d). Go check RSSO.\n", pRscConfig->Rsc_Size_Width, pRscConfig->Rsc_Size_Height);

        min = BVO_STRIDE_MIN(pRscConfig->Rsc_Size_Width);
        if (pRscConfig->Rsc_bvo.u4Stride < min )
        {
                LOG_ERR("Too small BVO Stride:(%d)< min:(%d) with width(%d)\n", pRscConfig->Rsc_bvo.u4Stride, min, pRscConfig->Rsc_Size_Width);
        }

        min = MVO_STRIDE_MIN(pRscConfig->Rsc_Size_Width);
        if (pRscConfig->Rsc_mvo.u4Stride < min )
        {
                LOG_ERR("Too small BVO Stride:(%d)< min:(%d) with width(%d)\n", pRscConfig->Rsc_mvo.u4Stride, min, pRscConfig->Rsc_Size_Width);
        }
#ifndef APLI_LAST_LINE_OF_RSS_FORMAT_SUPPORT
        h_rsc_c = pRscConfig->Rsc_Size_Height;
#else
        h_rsc_c = pRscConfig->Rsc_Size_Height - 1;
        h_rsc_p = pRscConfig->Rsc_Size_Height_p - 1;
#endif
        rsc_Config[i].RSC_CTRL = (pRscConfig->Rsc_Ctrl_Init_MV_Waddr << 16) | (pRscConfig->Rsc_Ctrl_Init_MV_Flush_cnt << 12) | (pRscConfig->Rsc_Ctrl_Trig_Num << 8) | (pRscConfig->Rsc_Ctrl_Imgi_c_Fmt << 6) | (pRscConfig->Rsc_Ctrl_Imgi_p_Fmt << 4) | (pRscConfig->Rsc_Ctrl_Gmv_mode << 3) | (pRscConfig->Rsc_Ctrl_First_Me << 2) | (pRscConfig->Rsc_Ctrl_Skip_Pre_Mv << 1) | (pRscConfig->Rsc_Ctrl_Start_Scan_Order << 0);
        rsc_Config[i].RSC_SIZE = (h_rsc_c << 16) | pRscConfig->Rsc_Size_Width;
        rsc_Config[i].RSC_IMGI_C_BASE_ADDR = pRscConfig->Rsc_Imgi_c.u4BufPA;
        rsc_Config[i].RSC_IMGI_P_BASE_ADDR = pRscConfig->Rsc_Imgi_p.u4BufPA;
        rsc_Config[i].RSC_MVI_BASE_ADDR    = pRscConfig->Rsc_mvi.u4BufPA;
#ifndef APLI_LAST_LINE_OF_RSS_FORMAT_SUPPORT
        rsc_Config[i].RSC_APLI_C_BASE_ADDR = pRscConfig->Rsc_Apli_c.u4BufPA;
        rsc_Config[i].RSC_APLI_P_BASE_ADDR = pRscConfig->Rsc_Apli_p.u4BufPA;
#else
        rsc_Config[i].RSC_APLI_C_BASE_ADDR = pRscConfig->Rsc_Imgi_c.u4BufPA + h_rsc_c * pRscConfig->Rsc_Size_Width;
        rsc_Config[i].RSC_APLI_P_BASE_ADDR = pRscConfig->Rsc_Imgi_p.u4BufPA + h_rsc_p * pRscConfig->Rsc_Size_Width_p;
#endif
        rsc_Config[i].RSC_MVO_BASE_ADDR    = pRscConfig->Rsc_mvo.u4BufPA;
        rsc_Config[i].RSC_BVO_BASE_ADDR    = pRscConfig->Rsc_bvo.u4BufPA;

        rsc_Config[i].RSC_IMGI_C_STRIDE = pRscConfig->Rsc_Imgi_c.u4Stride;
        rsc_Config[i].RSC_IMGI_P_STRIDE = pRscConfig->Rsc_Imgi_p.u4Stride;
        rsc_Config[i].RSC_MVI_STRIDE    = pRscConfig->Rsc_mvi.u4Stride;
        rsc_Config[i].RSC_MVO_STRIDE    = pRscConfig->Rsc_mvo.u4Stride;
        rsc_Config[i].RSC_BVO_STRIDE    = pRscConfig->Rsc_bvo.u4Stride;
#ifdef RSC_TUNABLE
        rsc_Config[i].RSC_MV_OFFSET = (pRscConfig->Rsc_Vert_P_Mv_Offset << 24) | (pRscConfig->Rsc_Horz_P_Mv_Offset << 16 ) | (pRscConfig->Rsc_Vert_C_Mv_Offset << 8)| (pRscConfig->Rsc_Horz_C_Mv_Offset << 0 ) ;
        rsc_Config[i].RSC_GMV_OFFSET = (pRscConfig->Rsc_Vert_G_Mv_Offset << 8) | (pRscConfig->Rsc_Horz_G_Mv_Offset << 0);
        rsc_Config[i].RSC_CAND_NUM = pRscConfig->Rsc_Cand_Num;
        rsc_Config[i].RSC_RAND_HORZ_LUT = (pRscConfig->Rsc_Rand_Horz_Lut_3 << 24) | (pRscConfig->Rsc_Rand_Horz_Lut_2 << 16) | (pRscConfig->Rsc_Rand_Horz_Lut_1 << 8) | (pRscConfig->Rsc_Rand_Horz_Lut_0 << 0);
        rsc_Config[i].RSC_RAND_VERT_LUT = (pRscConfig->Rsc_Rand_Vert_Lut_3 << 24) | (pRscConfig->Rsc_Rand_Vert_Lut_2 << 16) | (pRscConfig->Rsc_Rand_Vert_Lut_1 << 8) | (pRscConfig->Rsc_Rand_Vert_Lut_0 << 0);
        rsc_Config[i].RSC_SAD_CTRL = (pRscConfig->Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th <<24) | (pRscConfig->Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode << 21) | (pRscConfig->Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En << 20) | (pRscConfig->Rsc_Sad_Ctrl_Sad_Gain << 16) | (pRscConfig->Rsc_Sad_Ctrl_Sad_Coring_Th << 12) | (pRscConfig->Rsc_Sad_Ctrl_Prev_Y_Offset << 0);
        rsc_Config[i].RSC_SAD_EDGE_GAIN_CTRL = (pRscConfig->Rsc_Sad_Edge_Gain_Step << 16) | (pRscConfig->Rsc_Sad_Edge_Gain_TH_L << 8) | (pRscConfig->Rsc_Sad_Edge_Gain << 0);
        rsc_Config[i].RSC_SAD_CRNR_GAIN_CTRL = (pRscConfig->Rsc_Sad_Crnr_Gain_Step << 16)|(pRscConfig->Rsc_Sad_Crnr_Gain_TH_L << 8) | (pRscConfig->Rsc_Sad_Crnr_Gain << 0);
        rsc_Config[i].RSC_STILL_STRIP_CTRL0  = (pRscConfig->Rsc_Still_Strip_Zero_Pnlty_Dis << 1)|(pRscConfig->Rsc_Still_Strip_Blk_Th_En << 0);
        rsc_Config[i].RSC_STILL_STRIP_CTRL1  = (pRscConfig->Rsc_Still_Strip_Var_Step << 24)|(pRscConfig->Rsc_Still_Strip_Var_Step_Th_L << 16)|(pRscConfig->Rsc_Still_Strip_Sad_Step << 8)|(pRscConfig->Rsc_Still_Strip_Sad_Step_Th_L << 0);
        rsc_Config[i].RSC_RAND_PNLTY_CTRL = (pRscConfig->Rsc_Rand_Pnlty_Bndry << 12)|(pRscConfig->Rsc_Rand_Pnlty << 4) | (pRscConfig->Rsc_Rand_Pnlty_Gain << 0);
        rsc_Config[i].RSC_RAND_PNLTY_GAIN_CTRL0 = (pRscConfig->Rsc_Rand_Pnlty_Edge_Resp_Step << 16)|(pRscConfig->Rsc_Rand_Pnlty_Edge_Resp_Th_L << 8)|(pRscConfig->Rsc_Rand_Pnlty_Edge_Gain<< 0);
        rsc_Config[i].RSC_RAND_PNLTY_GAIN_CTRL1 = (pRscConfig->Rsc_Rand_Pnlty_Var_Resp_Step << 16)|(pRscConfig->Rsc_Rand_Pnlty_Var_Resp_Th_L << 8)|(pRscConfig->Rsc_Rand_Pnlty_Avg_Gain<< 0);
#endif
        //WMFE Config 2
        scale = MFALSE;
#endif
    }

    rsc_Request.m_ReqNum = num;
    rsc_Request.m_pRscConfig = rsc_Config;

    Ret = ioctl(this->m_Fd,RSC_ENQUE_REQ,&rsc_Request);
    if(Ret < 0)
    {
        LOG_ERR("RSC_WMFE_ENQUE_REQ fail(%d)\n", Ret);
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

MBOOL RscDrvImp::dequeRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::RSCConfig RscConfig;
    RSC_Request rsc_Request;
    RSC_Config rsc_Config[_SUPPORT_MAX_RSC_FRAME_REQUEST_];
    rsc_Request.m_pRscConfig = rsc_Config;
#if 1
    Ret = ioctl(this->m_Fd,RSC_DEQUE_REQ,&rsc_Request);
    if(Ret < 0)
    {
        LOG_ERR("RSC_DEQUE_REQ fail(%d)\n", Ret);
        printf("RSC_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeRSC num:%d\n", rsc_Request.m_ReqNum);
    for (unsigned int i=0; i< rsc_Request.m_ReqNum; i++)
    {
        RscConfig.feedback.RSC_STA_0 = rsc_Config[i].RSC_STA_0;
        RscConfigVec.push_back(RscConfig);
    }

#else
    MINT32 Ret;
    unsigned int num;
    NSCam::NSIoPipe::RSCConfig RscConfig;
    RSC_RSCConfig rsc_Config;
    Ret = ioctl(this->m_Fd,RSC_DEQUE_NUM,&num);
    if(Ret < 0)
    {
        LOG_ERR("RSC_DEQUE_NUM fail(%d)\n", Ret);
        printf("RSC_DEQUE_NUM fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeRSC num:%d\n", num);
    for (unsigned int i=0; i< num; i++)
    {
        Ret = ioctl(this->m_Fd,RSC_DEQUE,&rsc_Config);
        if(Ret < 0)
        {
            LOG_ERR("RSC_DEQUE fail(%d)\n", Ret);
            printf("RSC_DEQUE fail(%d)\n", Ret);
            return MFALSE;
        }
        RscConfigVec.push_back(RscConfig);
    }
#endif

    return MTRUE;
}


MUINT32 RscDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = RSC_REG_RANGE;
    (void) caller;

    LOG_DBG("+,Rsc_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->RscRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==RSC_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pRscHwRegAddr[(Addr>>2)];
    }
    else{
        RSC_REG_IO_STRUCT RscRegIo;
        RSC_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        RscRegIo.pData = (RSC_REG_STRUCT*)&RegIo;
        RscRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, RSC_READ_REGISTER, &RscRegIo);
        if(Ret < 0)
        {
            LOG_ERR("RSC_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Rsc_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL RscDrvImp::readRegs(RSC_DRV_REG_IO_STRUCT* pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = RSC_REG_RANGE;
    (void) caller;

    android::Mutex::Autolock lock(this->RscRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == RSC_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pRscHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        RSC_REG_IO_STRUCT RscRegIo;
        //pRegIo->module = this->m_HWmodule;
        RscRegIo.pData = (RSC_REG_STRUCT*)pRegIo;
        RscRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, RSC_READ_REGISTER, &RscRegIo);
        if(Ret < 0)
        {
            LOG_ERR("RSC_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Rsc_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL RscDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = RSC_REG_RANGE;
    (void) caller;

    LOG_DBG("Rsc_write:m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->RscRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case RSC_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pRscHwRegAddr[(Addr>>2)] = Data;
            break;
        case RSC_DRV_RW_IOCTL:
            RSC_REG_IO_STRUCT RscRegIo;
            RSC_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            RscRegIo.pData = (RSC_REG_STRUCT*)&RegIo;
            RscRegIo.Count = 1;
            ret = ioctl(this->m_Fd, RSC_WRITE_REGISTER, &RscRegIo);
            if(ret < 0){
                LOG_ERR("RSC_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case RSC_DRV_R_ONLY:
            LOG_ERR("RSC Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->RscRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL RscDrvImp::writeRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = RSC_REG_RANGE;
    (void) caller;

    android::Mutex::Autolock lock(this->RscRegMutex);
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case RSC_DRV_RW_IOCTL:
            RSC_REG_IO_STRUCT RscRegIo;
            //pRegIo->module = this->m_HWmodule;
            RscRegIo.pData = (RSC_REG_STRUCT*)pRegIo;
            RscRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, RSC_WRITE_REGISTER, &RscRegIo);
            if(Ret < 0){
                LOG_ERR("RSC_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case RSC_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pRscHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case RSC_DRV_R_ONLY:
            LOG_ERR("RSC Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Rsc_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 RscDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL RscDrvImp::setRWMode(RSC_DRV_RW_MODE rwMode)
{
    if(rwMode > RSC_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

