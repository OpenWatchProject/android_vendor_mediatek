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
#define LOG_TAG "WpeDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_wpe.h"
#include <mtkcam/def/common.h>
#include <wpe_drv.h>
#include <wpeunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(wpe_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (wpe_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (wpe_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (wpe_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (wpe_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (wpe_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (wpe_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

class WpeDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    WpeDbgTimer(char const*const pszTitle)
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

MINT32              WpeDrv::m_Fd = -1;
volatile MINT32     WpeDrvImp::m_UserCnt = 0;
WPE_DRV_RW_MODE    WpeDrv::m_regRWMode = WPE_DRV_R_ONLY;
MUINT32*            WpeDrvImp::m_pWpeHwRegAddr = NULL;
android::Mutex      WpeDrvImp::m_WpeInitMutex;
#define WPE_BASE_HW   0x1502a000


char                WpeDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

WpeDrvImp::WpeDrvImp()
{
    DBG_LOG_CONFIG(drv, wpe_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pWpeHwRegAddr = NULL;
}

WpeDrv* WpeDrv::createInstance()
{
    return WpeDrvImp::getInstance();
}

static WpeDrvImp singleton;

WpeDrv* WpeDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%08x].", &singleton);

    return &singleton;
}


MBOOL WpeDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
	MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_WpeInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to use wpe driver\n");
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

    // Open wpe device
    this->m_Fd = open(WPE_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("WPE kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(WPE_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("WPE kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=WPE_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pWpeHwRegAddr = (MUINT32 *) mmap(0, WPE_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, WPE_BASE_HW);

            if(m_pWpeHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("WPE mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=WPE_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, WPE_RESET, NULL) < 0){
        LOG_ERR("WPE Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strncpy((char*)this->m_UserName[m_UserCnt],userName, strlen(userName));
    this->m_UserName[m_UserCnt][strlen(userName)] = '\0';
	tmp = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            munmap(m_pWpeHwRegAddr, WPE_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close wpe device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pWpeHwRegAddr(0x%x)", Result, this->m_UserCnt, m_pWpeHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL WpeDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
	MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(m_WpeInitMutex);
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
		LOG_ERR("no more user in WpeDrv , curUser(%s)",userName);
		goto EXIT;
	}
	
	for(MUINT32 i=0;i<MAX_USER_NUMBER;i++){
		if(strcmp((const char*)this->m_UserName[i],userName) == 0){
			bMatch = i+1;	 //avoid match at the very 1st
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

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    if(m_pWpeHwRegAddr != MAP_FAILED){
         munmap(m_pWpeHwRegAddr, WPE_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=WPE_DRV_R_ONLY;
    }

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

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
        LOG_ERR("no wpe device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL WpeDrvImp::waitIrq(WPE_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    WPE_IRQ_CLEAR_ENUM OrgClr;
    WpeDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    WPE_WAIT_IRQ_STRUCT waitirq;
    LOG_INF(" - E. Status(0x%08x),Timeout(%d).\n",pWaitIrq->Status, pWaitIrq->Timeout);
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
	LOG_INF(" - E. waitirq.Status(0x%08x).\n",waitirq.Status);

    while( waitirq.Timeout > 0 )//receive restart system call again
    {
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,WPE_WAIT_IRQ,&waitirq);
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
        LOG_ERR("WPE_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL WpeDrvImp::clearIrq(WPE_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    WPE_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(WPE_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,WPE_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("WPE_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL WpeDrvImp::waitWPEFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    WPE_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = WPE_IRQ_WAIT_CLEAR;
    WaitIrq.Type = WPE_IRQ_TYPE_INT_WPE_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;
    LOG_DBG("waitWPEFrameDone  WaitIrq.Status : %d!!\n",  WaitIrq.Status);
    DRV_TRACE_BEGIN("WPE waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & WPE_INT_ST)
        {
            LOG_DBG("WPE Wait Interupt Frame Done Success!!\n");
        }
        DRV_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & WPE_INT_ST)
        {
            LOG_DBG("WPE Wait Interupt Frame Done Fail!!\n");
        }
        DRV_TRACE_END();
        return MFALSE;
    }


}
#define BYPASS_REG (0)

#define STRIDE_ALIGNMENT_8(STRIDE) (((STRIDE+7)>>3)<<3)

MBOOL WpeDrvImp::setDeviceInfo(E_WPE_DEVICE_INFO eCmd,MUINT8* pData)
{
    ISP_WPE_BUFQUE_STRUCT p2bufQ;

    switch(eCmd){
        case _SET_WPE_BUF_INFO:
            p2bufQ.ctrl = static_cast<ISP_WPE_BUFQUE_CTRL_ENUM>(pData[0]);
            p2bufQ.property = static_cast<ISP_WPE_BUFQUE_PROPERTY>(pData[1]);
            p2bufQ.processID = 0;//static_cast<MUINT32>(pData[2]);
            p2bufQ.callerID = static_cast<MUINT32>(pData[3]) | (static_cast<MUINT32>(pData[4])<<8) | (static_cast<MUINT32>(pData[5])<<16) | (static_cast<MUINT32>(pData[6])<<24);
            p2bufQ.frameNum = static_cast<MINT32>(pData[7]);
            p2bufQ.cQIdx = static_cast<MINT32>(pData[8]);
            p2bufQ.dupCQIdx = static_cast<MINT32>(pData[9]);
            p2bufQ.burstQIdx = static_cast<MINT32>(pData[10]);
            p2bufQ.timeoutIns = static_cast<MUINT32>(pData[11]);
            LOG_DBG("p2bufQ(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",p2bufQ.ctrl,p2bufQ.property,p2bufQ.processID,p2bufQ.callerID,\
                p2bufQ.frameNum,p2bufQ.cQIdx ,p2bufQ.dupCQIdx,p2bufQ.burstQIdx,p2bufQ.timeoutIns);
            if(ioctl(this->m_Fd,WPE_BUFQUE_CTRL,&p2bufQ) < 0){
                LOG_ERR("WPE_BUFQUE_CTRL(%d) error\n",p2bufQ.ctrl);
                return MFALSE;
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return MTRUE;
}


MBOOL WpeDrvImp::enqueWPE(vector<WPE_Config>& WpeConfigVec)
{

    MINT32 Ret;
    MINT32 min;
    unsigned int num = WpeConfigVec.size();
    WPE_Config* pWpeConfig;
    WPE_Request wpe_Request;
    WPE_Config wpe_Config[_SUPPORT_MAX_WPE_FRAME_REQUEST_];

    for (unsigned int i=0; i<WpeConfigVec.size(); i++)
    {
        pWpeConfig = &(WpeConfigVec.at(i));
#if !BYPASS_REG
		LOG_INF("Enter BYPASS_REG \n");

		wpe_Config[i] = (WpeConfigVec.at(i));

#endif
    }

    wpe_Request.m_ReqNum = num;
    wpe_Request.m_pWpeConfig = wpe_Config;
	LOG_INF("WPE_ENQUE_REQ \n");

    Ret = ioctl(this->m_Fd,WPE_ENQUE_REQ,&wpe_Request);
    if(Ret < 0)
    {
        LOG_ERR("WPE_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    return MTRUE;


}

MBOOL WpeDrvImp::dequeWPE(vector<WPE_Config>& WpeConfigVec)
{
    MINT32 Ret;
    WPE_Config WpeConfig;
    WPE_Request wpe_Request;
    WPE_Config wpe_Config[_SUPPORT_MAX_WPE_FRAME_REQUEST_];
    wpe_Request.m_pWpeConfig = wpe_Config;
	LOG_INF("WPE_DEQUE_REQ\n");

    //Ret = ioctl(this->m_Fd,WPE_DEQUE_REQ,&wpe_Request);
    Ret = ioctl(this->m_Fd,WPE_DEQUE_DONE,&wpe_Request);
    if(Ret < 0)
    {
        LOG_ERR("WPE_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    for (unsigned int i=0; i< wpe_Request.m_ReqNum; i++)
    {
        WpeConfigVec.push_back(WpeConfig);
    }


	LOG_DBG("Leave WPE_DEQUE_REQ\n");

    return MTRUE;
}

MBOOL WpeDrvImp::waitDeque()
{
    MINT32 Ret=0;
    
    LOG_DBG("+,Wpe_waitDeque\n");
    Ret = ioctl(this->m_Fd,WPE_WAIT_DEQUE);
    if(Ret < 0)
    {
        LOG_ERR("WPE_WAIT_DEQUE fail(%d)\n", Ret);

        return MFALSE;
    }

    LOG_DBG("-,Wpe_waitDeque");
    return MTRUE;
}

MUINT32 WpeDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = WPE_REG_RANGE;
    LOG_DBG("+,Wpe_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->WpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==WPE_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pWpeHwRegAddr[(Addr>>2)];
    }
    else{
        WPE_REG_IO_STRUCT WpeRegIo;
        WPE_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        WpeRegIo.pData = (WPE_REG_STRUCT*)&RegIo;
        WpeRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, WPE_READ_REGISTER, &WpeRegIo);
        if(Ret < 0)
        {
            LOG_ERR("WPE_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Wpe_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL WpeDrvImp::readRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = WPE_REG_RANGE;
    android::Mutex::Autolock lock(this->WpeRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == WPE_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pWpeHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        WPE_REG_IO_STRUCT WpeRegIo;
        //pRegIo->module = this->m_HWmodule;
        WpeRegIo.pData = (WPE_REG_STRUCT*)pRegIo;
        WpeRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, WPE_READ_REGISTER, &WpeRegIo);
        if(Ret < 0)
        {
            LOG_ERR("WPE_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Wpe_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL WpeDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = WPE_REG_RANGE;
    LOG_DBG("Wpe_write:m_regRWMode(0x%x),(0x%x,0x%x)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->WpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case WPE_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pWpeHwRegAddr[(Addr>>2)] = Data;
            break;
        case WPE_DRV_RW_IOCTL:
            WPE_REG_IO_STRUCT WpeRegIo;
            WPE_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            WpeRegIo.pData = (WPE_REG_STRUCT*)&RegIo;
            WpeRegIo.Count = 1;
            ret = ioctl(this->m_Fd, WPE_WRITE_REGISTER, &WpeRegIo);
            if(ret < 0){
                LOG_ERR("WPE_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case WPE_DRV_R_ONLY:
            LOG_ERR("WPE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->WpeRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL WpeDrvImp::writeRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = WPE_REG_RANGE;
    android::Mutex::Autolock lock(this->WpeRegMutex);
    //(void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case WPE_DRV_RW_IOCTL:
            WPE_REG_IO_STRUCT WpeRegIo;
            //pRegIo->module = this->m_HWmodule;
            WpeRegIo.pData = (WPE_REG_STRUCT*)pRegIo;
            WpeRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, WPE_WRITE_REGISTER, &WpeRegIo);
            if(Ret < 0){
                LOG_ERR("WPE_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case WPE_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pWpeHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case WPE_DRV_R_ONLY:
            LOG_ERR("WPE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Wpe_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 WpeDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL WpeDrvImp::setRWMode(WPE_DRV_RW_MODE rwMode)
{
    if(rwMode > WPE_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

