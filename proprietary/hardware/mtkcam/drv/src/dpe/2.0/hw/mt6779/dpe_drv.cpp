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
#include <mtkcam/drv/def/dpecommon_v20.h>
#include <dpe_drv.h>
#include <dpeunittest.h>
#include <imem_drv.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include <cutils/properties.h>
DECLARE_DBG_LOG_VARIABLE(dpe_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        BASE_LOG_VRB(fmt, ##arg);
#define LOG_DBG(fmt, arg...)        do { if (g_isDPELogEnable) { ALOGI(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        ALOGI(fmt, ##arg);
#define LOG_WRN(fmt, arg...)        BASE_LOG_WRN(fmt, ##arg);
#define LOG_ERR(fmt, arg...)        ALOGE(fmt, ##arg);
#define LOG_AST(cond, fmt, arg...)  BASE_LOG_AST(cond, fmt, ##arg);
MBOOL g_isDPELogEnable = MFALSE;
MBOOL g_enqueFlag = MFALSE;

#if 1
StdIMemDrv *g_DpeImemDrv = NULL;
IMEM_BUF_INFO g_DpeWorkingBuffer;
IMEM_BUF_INFO Dpe_Imem_Buf_Info;
MUINT8   *Imem_pLogVir=NULL;
MINT32   Imem_MemID;
MUINT32  Imem_Size;
MUINT32  Dpe_Imem_AllocNum = 0;
#endif

// MEDV buffer width size should be 64B align
// All other buffer width size should be 128B align
// Assume max width = 640 should meet above requirements
#define DPE_MAX_FRAME_SIZE 307200 //640x480
#define WB_INT_MEDV_SIZE (DPE_MAX_FRAME_SIZE*4)
#define WB_MEDV_SIZE (DPE_MAX_FRAME_SIZE*4)
#define WB_DCV_CONF_SIZE (DPE_MAX_FRAME_SIZE)
#define WB_DCV_L_SIZE (DPE_MAX_FRAME_SIZE*16)
#define WB_DCV_R_SIZE (DPE_MAX_FRAME_SIZE*16)
#define WB_ASFRM_SIZE (DPE_MAX_FRAME_SIZE)
#define WB_ASFRMExt_SIZE (DPE_MAX_FRAME_SIZE)
#define WB_WMFHF_SIZE (DPE_MAX_FRAME_SIZE)
//#define WB_CRM_SIZE (DPE_MAX_FRAME_SIZE*2)

#define WB_TOTAL_SIZE (WB_INT_MEDV_SIZE+WB_MEDV_SIZE+WB_DCV_L_SIZE+WB_DCV_R_SIZE+WB_ASFRM_SIZE+WB_ASFRMExt_SIZE+WB_WMFHF_SIZE)

#define DPEWB_InterMEDV_VA (g_DpeWorkingBuffer.virtAddr)
#define DPEWB_InterMEDV_PA (g_DpeWorkingBuffer.phyAddr)
#define DPEWB_MEDV1_VA (DPEWB_InterMEDV_VA+WB_INT_MEDV_SIZE)
#define DPEWB_MEDV1_PA (DPEWB_InterMEDV_PA+WB_INT_MEDV_SIZE)
#define DPEWB_DCVFrm1_L_VA (DPEWB_MEDV1_VA+WB_MEDV_SIZE)
#define DPEWB_DCVFrm1_L_PA (DPEWB_MEDV1_PA+WB_MEDV_SIZE)
#define DPEWB_DCVFrm1_R_VA (DPEWB_DCVFrm1_L_VA+WB_DCV_L_SIZE)
#define DPEWB_DCVFrm1_R_PA (DPEWB_DCVFrm1_L_PA+WB_DCV_L_SIZE)
#define DPEWB_ASFRM_VA (DPEWB_DCVFrm1_R_VA+WB_DCV_R_SIZE)
#define DPEWB_ASFRM_PA (DPEWB_DCVFrm1_R_PA+WB_DCV_R_SIZE)
#define DPEWB_ASFRMExt_VA (DPEWB_ASFRM_VA+WB_ASFRM_SIZE)
#define DPEWB_ASFRMExt_PA (DPEWB_ASFRM_PA+WB_ASFRM_SIZE)
#define DPEWB_WMFHF_VA (DPEWB_ASFRMExt_VA+WB_ASFRM_SIZE)
#define DPEWB_WMFHF_PA (DPEWB_ASFRMExt_PA+WB_ASFRM_SIZE)

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

MINT32              DpeDrvImp::m_Fd = -1;
volatile MINT32     DpeDrvImp::m_UserCnt = 0;
DPE_DRV_RW_MODE     DpeDrvImp::m_regRWMode = DPE_DRV_R_ONLY;
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

template<>
EngineDrv<DPEConfig>* EngineDrv<DPEConfig>::createInstance()
{
    return DpeDrvImp::getInstance();
}

static DpeDrvImp singleton;

EngineDrv<DPEConfig>* DpeDrvImp::getInstance()
{
    singleton.ctl_tbl = dpe_ctl;

    LOG_DBG("singleton[0x%p].",static_cast<void *>(&singleton));

    return &singleton;
}

MINT32 DPE_Imem_alloc(MUINT32 size,MINT32 *memId,MUINT8 **vAddr,MUINTPTR *pAddr)
{
    if ( NULL == g_DpeImemDrv ) {
        g_DpeImemDrv = StdIMemDrv::createInstance();
        g_DpeImemDrv->init();
    }
    //
    Dpe_Imem_Buf_Info.size = size;
    Dpe_Imem_Buf_Info.useNoncache = 1;
    if (g_DpeImemDrv->allocVirtBuf(&Dpe_Imem_Buf_Info) != 0)
        LOG_ERR("Imem Allocate Virtual Buffer Fail!");
    *memId = Dpe_Imem_Buf_Info.memID;
    *vAddr = (MUINT8 *)Dpe_Imem_Buf_Info.virtAddr;
    //
    if (g_DpeImemDrv->mapPhyAddr(&Dpe_Imem_Buf_Info) != 0)
        LOG_ERR("Imem Map Physical Address Fail!");
    *pAddr = (MUINTPTR)Dpe_Imem_Buf_Info.phyAddr;

    g_DpeWorkingBuffer.phyAddr=(MUINTPTR)Dpe_Imem_Buf_Info.phyAddr;
    g_DpeWorkingBuffer.virtAddr=(MUINTPTR)Imem_pLogVir;
    g_DpeWorkingBuffer.memID=Imem_MemID;
    g_DpeWorkingBuffer.size=Imem_Size;

    Dpe_Imem_AllocNum ++;

    /*MY_LOGD("Dpe_Imem_AllocNum(%d)\n",Dpe_Imem_AllocNum);*/
    /*MY_LOGD("vAddr(0x%p) pAddr(0x%p) Dpe_Imem_AllocNum(%d)\n",*vAddr,*pAddr,Dpe_Imem_AllocNum);*/

    return 0;
}

MINT32 DPE_Imem_free(MUINT8 *vAddr, MUINTPTR phyAddr,MUINT32 size,MINT32 memId)
{
    Dpe_Imem_Buf_Info.size = size;
    Dpe_Imem_Buf_Info.memID = memId;
    Dpe_Imem_Buf_Info.virtAddr = (MUINTPTR)vAddr;
    Dpe_Imem_Buf_Info.phyAddr = (MUINTPTR)phyAddr;
    //

    if(g_DpeImemDrv)
    {
        if(g_DpeImemDrv->unmapPhyAddr(&Dpe_Imem_Buf_Info))
            LOG_ERR("Imem Unmap Physical Address Fail!");

        if(g_DpeImemDrv->freeVirtBuf(&Dpe_Imem_Buf_Info))
            LOG_ERR("Imem Free Virtual Buffer Fail!");

        if(Dpe_Imem_AllocNum)
        {
            Dpe_Imem_AllocNum--;
            if(Dpe_Imem_AllocNum==0)
            {
                g_DpeImemDrv->uninit();
                g_DpeImemDrv = NULL;
            }
        }
    }
    else
    {
        LOG_ERR("No DPE ImemDrv!");
    }

    /*MY_LOGD("Dpe_Imem_AllocNum(%d)\n",Dpe_Imem_AllocNum);*/

    return 0;
}

MBOOL DpeDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_DpeInitMutex);
    //

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.dpe.debug.enable", value, "0");
    g_isDPELogEnable = atoi(value);

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
    Imem_Size = WB_TOTAL_SIZE+1024; // 1KB buffer
    DPE_Imem_alloc(Imem_Size, &Imem_MemID, &Imem_pLogVir, &g_DpeWorkingBuffer.phyAddr);
/*
    if(ioctl(this->m_Fd, DPE_RESET, NULL) < 0){
        LOG_ERR("DPE Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }
*/
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

    DPE_Imem_free((MUINT8 *)g_DpeWorkingBuffer.virtAddr, g_DpeWorkingBuffer.phyAddr, g_DpeWorkingBuffer.size, g_DpeWorkingBuffer.memID);

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
            LOG_DBG("DPE_WAIT_IRQ Success, Period: %d us", Tb-Ta);
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

MBOOL DpeDrvImp::waitFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    MINT32 Ret;
    DPE_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = DPE_IRQ_WAIT_CLEAR;
    WaitIrq.Type = DPE_IRQ_TYPE_INT_DPE_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    DRV_TRACE_BEGIN("DPE waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & DPE_INT_ST)
        {
            LOG_INF("DPE Wait Interupt Frame Done Success\n");
        }
        DRV_TRACE_END();

        return MTRUE;
    }
    else
    {
        if (Status & DPE_INT_ST)
        {
            LOG_INF("DPE Wait Interupt Frame Done Fail!!\n");
        }
        DRV_TRACE_END();
        Ret = ioctl(this->m_Fd, DPE_DUMP_REG);

        return MFALSE;
    }
}

void DpeDrvImp::DPE_Config_DVS(NSCam::NSIoPipe::DPEConfig* pDpeConfig, DPE_Config *pConfigToKernel)
{
    unsigned int frmWidth = pDpeConfig->Dpe_DVSSettings.frmWidth;
    unsigned int frmHeight = pDpeConfig->Dpe_DVSSettings.frmHeight;
    unsigned int engStartX = pDpeConfig->Dpe_DVSSettings.engStart_x;
    unsigned int engStartY = pDpeConfig->Dpe_DVSSettings.engStart_y;
    unsigned int engWidth = frmWidth - engStartX;
    unsigned int engHeight = frmHeight - (engStartY * 2);
    unsigned int occStartX = engStartX;
    unsigned int occStartY = engStartY;
    unsigned int occWidth = frmWidth - (engStartX * 2);
    unsigned int occHeight = frmHeight - (engStartY * 2);
    unsigned int pitch = pDpeConfig->Dpe_DVSSettings.frmWidth >> 3 >> 4; // frame width / 8 / 16
    unsigned int curFrm = 0, prevFrm = 0;

    LOG_INF("DVS param: frm width(%d), frm height(%d), eng startX(%d), eng startY(%d), eng width(%d), eng height(%d), occ width(%d), occ height(%d), occ startX(%d), occ startY(%d), pitch(%d), \
main eye(%d), 16bit mode(%d), dv_en(%d), conf_en(%d), cv_en(%d), occ_en(%d), have DV input(%d)\n",
        frmWidth, frmHeight, engStartX, engStartY, engWidth, engHeight, occWidth, occHeight, occStartX, occStartY, pitch,
        pDpeConfig->Dpe_DVSSettings.mainEyeSel,
        pDpeConfig->Dpe_is16BitMode,
        pDpeConfig->Dpe_DVSSettings.SubModule_EN.dvme_en,
        pDpeConfig->Dpe_DVSSettings.SubModule_EN.conf_en,
        pDpeConfig->Dpe_DVSSettings.SubModule_EN.cv_en,
        pDpeConfig->Dpe_DVSSettings.SubModule_EN.occ_en,
        pDpeConfig->Dpe_DVSSettings.haveDVInput);

    if (pDpeConfig->Dpe_DVSSettings.haveDVInput == 1) {
        if (pDpeConfig->Dpe_InBuf_DV == 0x0) {
            LOG_INF("No DV Input Buffer, Force to Disable DV Input Reference!");
            curFrm = 0;
            prevFrm = 0;
        }
        else {
            if (g_enqueFlag == 0) {
                curFrm = 0;
                prevFrm = 1;
            } else {
                curFrm = 2;
                prevFrm = 3;
            }
        }
    }
    else {
        if (g_enqueFlag == 0) {
            curFrm = 0;
            prevFrm = 0;
        } else {
            curFrm = 2;
            prevFrm = 2;
        }
    }

    pConfigToKernel->DVS_CTRL00 = ((pDpeConfig->Dpe_DVSSettings.Iteration.y_IterTimes & 0xF) << 0) |
        ((pDpeConfig->Dpe_DVSSettings.Iteration.y_IterStartDirect & 0x1) << 4) |
        ((pDpeConfig->Dpe_DVSSettings.Iteration.x_IterTimes_Up & 0x3) << 5) |
        ((pDpeConfig->Dpe_DVSSettings.Iteration.x_IterTimes_Down & 0x3) << 7) |
        ((pDpeConfig->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up & 0x1) << 9) |
        ((pDpeConfig->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down & 0x1) << 10) |
        (0xD << 11) |
        (0xF << 16) |
        ((curFrm & 0x3) << 21) |
        ((prevFrm & 0x3) << 23) |
        ((pDpeConfig->Dpe_DVSSettings.SubModule_EN.dvme_en & 0x1) << 26) |
        ((pDpeConfig->Dpe_DVSSettings.SubModule_EN.conf_en & 0x1) << 27) |
        ((pDpeConfig->Dpe_DVSSettings.SubModule_EN.cv_en & 0x1) << 28) |
        ((pDpeConfig->Dpe_DVSSettings.SubModule_EN.occ_en & 0x1) << 29) |
        ((0x0 & 0x1) << 30) |
        ((0x1 & 0x1) << 31);

    pConfigToKernel->DVS_DRAM_PITCH = ((pitch & 0x7F) << 4) | ((pitch & 0x7F) << 20);

    pConfigToKernel->DVS_SRC_00 = ((frmHeight & 0x1FFF) << 0) | ((frmWidth & 0x1FFF) << 16) |
        ((pDpeConfig->Dpe_is16BitMode & 0x1) << 29) |
        ((pDpeConfig->Dpe_DVSSettings.mainEyeSel & 0x1) << 30);

    pConfigToKernel->DVS_SRC_01 = ((engHeight & 0x1FFF) << 0) | ((engWidth & 0x1FFF) << 16);

    pConfigToKernel->DVS_SRC_02 = ((engStartX & 0x1FFF) << 0) | ((engStartX & 0x1FFF) << 16);

    pConfigToKernel->DVS_SRC_03 = (engStartY & 0x1FFF);

    pConfigToKernel->DVS_SRC_04 = ((occWidth & 0x1FFF) << 0) | ((occStartX & 0x1FFF) << 16);

    pConfigToKernel->DVS_SRC_21_INTER_MEDV = (unsigned int)DPEWB_InterMEDV_PA;

    if (pDpeConfig->Dpe_OutBuf_DV != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_DV->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_22_MEDV0 = (unsigned int)pDpeConfig->Dpe_OutBuf_DV->getBufPA(0);
            pConfigToKernel->DVS_SRC_24_MEDV2 = (unsigned int)pDpeConfig->Dpe_OutBuf_DV->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    } else {
        LOG_INF("No MEDV OutBuffer PA, use internal working buffer\n");
        pConfigToKernel->DVS_SRC_22_MEDV0 = (unsigned int)DPEWB_MEDV1_PA;
        pConfigToKernel->DVS_SRC_24_MEDV2 = (unsigned int)DPEWB_MEDV1_PA;
    }
    //pConfigToKernel->DVS_SRC_22_MEDV0 = (unsigned int)DPEWB_MEDV1_PA;
    //pConfigToKernel->DVS_SRC_23_MEDV1 = (unsigned int)DPEWB_MEDV2_PA;
    //pConfigToKernel->DVS_SRC_24_MEDV2 = (unsigned int)DPEWB_MEDV3_PA;
    //pConfigToKernel->DVS_SRC_25_MEDV3 = (unsigned int)DPEWB_MEDV4_PA;
	//pConfigToKernel->DVS_SRC_30_DCV_CONF0 = (unsigned int)DPEWB_DCVConf1_PA;
    if (pDpeConfig->Dpe_OutBuf_CONF != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_CONF->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_30_DCV_CONF0 = (unsigned int)pDpeConfig->Dpe_OutBuf_CONF->getBufPA(0);
            pConfigToKernel->DVS_SRC_32_DCV_CONF2 = (unsigned int)pDpeConfig->Dpe_OutBuf_CONF->getBufPA(0);
        }
    }
	//pConfigToKernel->DVS_SRC_31_DCV_CONF1 = (unsigned int)DPEWB_DCVConf2_PA;
	//pConfigToKernel->DVS_SRC_32_DCV_CONF2 = (unsigned int)DPEWB_DCVConf3_PA;
	//pConfigToKernel->DVS_SRC_33_DCV_CONF3 = (unsigned int)DPEWB_DCVConf4_PA;
	pConfigToKernel->DVS_SRC_34_DCV_L_FRM0 = (unsigned int)DPEWB_DCVFrm1_L_PA;
    pConfigToKernel->DVS_SRC_36_DCV_L_FRM2 = (unsigned int)DPEWB_DCVFrm1_L_PA;
	//pConfigToKernel->DVS_SRC_35_DCV_L_FRM1 = (unsigned int)DPEWB_DCVFrm2_L_PA;
	//pConfigToKernel->DVS_SRC_36_DCV_L_FRM2 = (unsigned int)DPEWB_DCVFrm3_L_PA;
	//pConfigToKernel->DVS_SRC_37_DCV_L_FRM3 = (unsigned int)DPEWB_DCVFrm4_L_PA;
	pConfigToKernel->DVS_SRC_38_DCV_R_FRM0 = (unsigned int)DPEWB_DCVFrm1_R_PA;
    pConfigToKernel->DVS_SRC_40_DCV_R_FRM2 = (unsigned int)DPEWB_DCVFrm1_R_PA;
	//pConfigToKernel->DVS_SRC_39_DCV_R_FRM1 = (unsigned int)DPEWB_DCVFrm2_R_PA;
	//pConfigToKernel->DVS_SRC_40_DCV_R_FRM2 = (unsigned int)DPEWB_DCVFrm3_R_PA;
	//pConfigToKernel->DVS_SRC_41_DCV_R_FRM3 = (unsigned int)DPEWB_DCVFrm4_R_PA;

    if (pDpeConfig->Dpe_InBuf_DV != 0x0) {
        if (pDpeConfig->Dpe_InBuf_DV->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_23_MEDV1 = (unsigned int)pDpeConfig->Dpe_InBuf_DV->getBufPA(0);
            pConfigToKernel->DVS_SRC_25_MEDV3 = (unsigned int)pDpeConfig->Dpe_InBuf_DV->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }

    if (pDpeConfig->Dpe_InBuf_SrcImg_Y_L != 0x0) {
        if (pDpeConfig->Dpe_InBuf_SrcImg_Y_L->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_05_L_FRM0 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_Y_L->getBufPA(0);
            pConfigToKernel->DVS_SRC_07_L_FRM2 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_Y_L->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No Left Src Image Y!\n");
    }
    if (pDpeConfig->Dpe_InBuf_SrcImg_Y_R != 0x0) {
        if (pDpeConfig->Dpe_InBuf_SrcImg_Y_R->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_09_R_FRM0 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_Y_R->getBufPA(0);
            pConfigToKernel->DVS_SRC_11_R_FRM2 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_Y_R->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No Right Src Image Y!\n");
    }
    if (pDpeConfig->Dpe_InBuf_ValidMap_L != 0x0) {
        if (pDpeConfig->Dpe_InBuf_ValidMap_L->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_13_L_VMAP0 = (unsigned int)pDpeConfig->Dpe_InBuf_ValidMap_L->getBufPA(0);
            pConfigToKernel->DVS_SRC_15_L_VMAP2 = (unsigned int)pDpeConfig->Dpe_InBuf_ValidMap_L->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No Left Valid Map!\n");
    }
    if (pDpeConfig->Dpe_InBuf_ValidMap_R != 0x0) {
        if (pDpeConfig->Dpe_InBuf_ValidMap_R->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_17_R_VMAP0 = (unsigned int)pDpeConfig->Dpe_InBuf_ValidMap_R->getBufPA(0);
            pConfigToKernel->DVS_SRC_19_R_VMAP2 = (unsigned int)pDpeConfig->Dpe_InBuf_ValidMap_R->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No Right Valid Map!\n");
    }
    if (pDpeConfig->Dpe_OutBuf_OCC != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_OCC->getBufPA(0) != 0x0) {
            pConfigToKernel->DVS_SRC_26_OCCDV0 = (unsigned int)pDpeConfig->Dpe_OutBuf_OCC->getBufPA(0);
            pConfigToKernel->DVS_SRC_28_OCCDV2 = (unsigned int)pDpeConfig->Dpe_OutBuf_OCC->getBufPA(0);
        } else {
            LOG_INF("No Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No DVS Output Buffer!\n");
    }
    if (pDpeConfig->Dpe_is16BitMode != 0) {
        if (pDpeConfig->Dpe_OutBuf_OCC_Ext != 0x0) {
            if (pDpeConfig->Dpe_OutBuf_OCC_Ext->getBufPA(0) != 0x0) {
                pConfigToKernel->DVS_SRC_42_OCCDV_EXT0 = (unsigned int)pDpeConfig->Dpe_OutBuf_OCC_Ext->getBufPA(0);
                pConfigToKernel->DVS_SRC_44_OCCDV_EXT2 = (unsigned int)pDpeConfig->Dpe_OutBuf_OCC_Ext->getBufPA(0);
            } else {
                LOG_INF("No Buffer PA, may be released?");
            }
        }
        else {
            LOG_INF("No DVS Ext Output Buffer!\n");
        }
    }

    memcpy(&pConfigToKernel->DVS_ME_00, &pDpeConfig->Dpe_DVSSettings.TuningBuf_ME, sizeof(pDpeConfig->Dpe_DVSSettings.TuningBuf_ME));
    memcpy(&pConfigToKernel->DVS_OCC_PQ_0, &pDpeConfig->Dpe_DVSSettings.TuningBuf_OCC, sizeof(pDpeConfig->Dpe_DVSSettings.TuningBuf_OCC));
}

void DpeDrvImp::DPE_Config_DVP(NSCam::NSIoPipe::DPEConfig* pDpeConfig, DPE_Config *pConfigToKernel)
{
    unsigned int engStartX = pDpeConfig->Dpe_DVPSettings.engStart_x;
    unsigned int engStartY = pDpeConfig->Dpe_DVPSettings.engStart_y;
    unsigned int frmWidth = pDpeConfig->Dpe_DVPSettings.frmWidth - engStartX;
    unsigned int frmHeight = pDpeConfig->Dpe_DVPSettings.frmHeight - (engStartY * 2);
    unsigned int engWidth = pDpeConfig->Dpe_DVPSettings.frmWidth - (engStartX * 2);
    unsigned int engHeight = frmHeight;
    unsigned int occStartX = engStartX;
    unsigned int occStartY = 0;
    unsigned int occWidth = engWidth;
    unsigned int occHeight = engHeight;
    unsigned int pitch = pDpeConfig->Dpe_DVPSettings.frmWidth >> 3 >> 4; // frame width / 8 / 16

    unsigned int engStart_offset_Y = (engStartY >> 3) * (pitch << 4);
    unsigned int engStart_offset_C = engStart_offset_Y >> 1;

    LOG_INF("DVP param: frm width(%d), frm height(%d), eng startX(%d), eng startY(%d), eng width(%d), eng height(%d), occ width(%d), occ height(%d), occ startX(%d), occ startY(%d), pitch(%d), eng start offset Y(0x%x), C(0x%x)\n",
        frmWidth, frmHeight, engStartX, engStartY, engWidth, engHeight, occWidth, occHeight, occStartX, occStartY, pitch, engStart_offset_Y, engStart_offset_C);

    pConfigToKernel->DVP_CTRL04 = (pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_en << 0) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_rm_en << 1) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_rd_en << 2) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_hf_en << 3) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.wmf_hf_en << 4) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.wmf_filt_en << 5) |
        (pDpeConfig->Dpe_DVPSettings.DV_guide_en << 6) |
        ((pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_hf_rounds & (0xF)) << 8) |
        ((pDpeConfig->Dpe_DVPSettings.SubModule_EN.wmf_filt_rounds & (0x3)) << 16) |
        (pDpeConfig->Dpe_is16BitMode << 18) |
        (pDpeConfig->Dpe_DVPSettings.SubModule_EN.asf_recursive_en << 19);

    pConfigToKernel->DVP_DRAM_PITCH = ((pitch & 0x7F) << 4) | ((pitch & 0x7F) << 20);

    pConfigToKernel->DVP_SRC_00 = ((engHeight & 0x1FFF) << 0) | ((engWidth & 0x1FFF) << 16) |
        ((pDpeConfig->Dpe_DVPSettings.Y_only & 0x1) << 29) |
        ((pDpeConfig->Dpe_DVPSettings.mainEyeSel & 0x1) << 30);

    pConfigToKernel->DVP_SRC_01 = ((frmHeight & 0x1FFF) << 0) | ((frmWidth & 0x1FFF) << 16);

    pConfigToKernel->DVP_SRC_02 = ((engStartX & 0x1FFF) << 0);// | ((engStartY & 0x1FFF) << 16); for Simulation

    pConfigToKernel->DVP_SRC_03 = ((occHeight & 0x1FFF) << 0) | ((occWidth & 0x1FFF) << 16);

    pConfigToKernel->DVP_SRC_04 = 0x0;//((occStartX & 0x1FFF) << 0) | ((occStartY & 0x1FFF) << 16); for Simulation

    //pConfigToKernel->DVP_SRC_17_CRM = (unsigned int)DPEWB_CRM_PA;
    if (pDpeConfig->Dpe_OutBuf_CRM != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_CRM->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_17_CRM = (unsigned int)pDpeConfig->Dpe_OutBuf_CRM->getBufPA(0);
        } else {
            LOG_INF("No CRM Buffer PA, may be released?");
        }
    } else {
        LOG_INF("No CRM Buffer!\n");
    }

    pConfigToKernel->DVP_SRC_18_ASF_RMDV = (unsigned int)DPEWB_ASFRM_PA;
    //if (pDpeConfig->Dpe_OutBuf_ASF_RM != 0x0) {
    //    if (pDpeConfig->Dpe_OutBuf_ASF_RM->getBufPA(0) != 0x0) {
    //        pConfigToKernel->DVP_SRC_18_ASF_RMDV = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_RM->getBufPA(0);
    //    }
    //}
    pConfigToKernel->DVP_EXT_SRC_18_ASF_RMDV = (unsigned int)DPEWB_ASFRMExt_PA;
    //if (pDpeConfig->Dpe_OutBuf_ASF_RM_Ext != 0x0) {
    //    if (pDpeConfig->Dpe_OutBuf_ASF_RM_Ext->getBufPA(0) != 0x0) {
    //        pConfigToKernel->DVP_EXT_SRC_18_ASF_RMDV = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_RM_Ext->getBufPA(0);
    //    }
    //}
    if (pDpeConfig->Dpe_OutBuf_ASF_RD != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_ASF_RD->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_19_ASF_RDDV = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_RD->getBufPA(0);
        }
    }
    if (pDpeConfig->Dpe_OutBuf_ASF_RD_Ext != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_ASF_RD_Ext->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_EXT_SRC_19_ASF_RDDV= (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_RD_Ext->getBufPA(0);
        }
    }

    pConfigToKernel->DVP_SRC_24_WMF_HFDV = (unsigned int)DPEWB_WMFHF_PA;
    //if (pDpeConfig->Dpe_OutBuf_WMF_HF != 0x0) {
    //    if (pDpeConfig->Dpe_OutBuf_WMF_HF->getBufPA(0) != 0x0) {
    //        pConfigToKernel->DVP_SRC_24_WMF_HFDV = (unsigned int)pDpeConfig->Dpe_OutBuf_WMF_HF->getBufPA(0);
    //    }
    //}

    if (pDpeConfig->Dpe_InBuf_SrcImg_C != 0x0) {
        if (pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_05_Y_FRM0 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(0) + (engStart_offset_Y);
            pConfigToKernel->DVP_SRC_07_Y_FRM2 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(0) + (engStart_offset_Y);
        } else {
            LOG_INF("No Src Image Y Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No DVP Src Image Y!\n");
    }

    if (pDpeConfig->Dpe_InBuf_SrcImg_C != 0x0) {
        if (pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(1) != 0x0) {
            pConfigToKernel->DVP_SRC_09_C_FRM0 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(1) + (engStart_offset_C);
            pConfigToKernel->DVP_SRC_11_C_FRM2 = (unsigned int)pDpeConfig->Dpe_InBuf_SrcImg_C->getBufPA(1) + (engStart_offset_C);
        } else {
            LOG_INF("No Src Image C Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No Src Image C!\n");
    }
    if (pDpeConfig->Dpe_InBuf_OCC != 0x0) {
        if (pDpeConfig->Dpe_InBuf_OCC->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_13_OCCDV0 = (unsigned int)pDpeConfig->Dpe_InBuf_OCC->getBufPA(0);
            pConfigToKernel->DVP_SRC_15_OCCDV2 = (unsigned int)pDpeConfig->Dpe_InBuf_OCC->getBufPA(0);
        } else {
            LOG_INF("No DVP OCC Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No DVP OCC In!\n");
    }
    if (pDpeConfig->Dpe_OutBuf_ASF_HF != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_ASF_HF->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_20_ASF_DV0 = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_HF->getBufPA(0);
            pConfigToKernel->DVP_SRC_22_ASF_DV2 = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_HF->getBufPA(0);
        } else {
            LOG_INF("No ASF Output Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No ASF Output!\n");
    }
    if (pDpeConfig->Dpe_OutBuf_WMF_FILT != 0x0) {
        if (pDpeConfig->Dpe_OutBuf_WMF_FILT->getBufPA(0) != 0x0) {
            pConfigToKernel->DVP_SRC_25_WMF_DV0 = (unsigned int)pDpeConfig->Dpe_OutBuf_WMF_FILT->getBufPA(0);
            pConfigToKernel->DVP_SRC_27_WMF_DV2 = (unsigned int)pDpeConfig->Dpe_OutBuf_WMF_FILT->getBufPA(0);
        } else {
            LOG_INF("No WMF Output Buffer PA, may be released?");
        }
    }
    else {
        LOG_INF("No WMF Output Buffer!\n");
    }
    if (pDpeConfig->Dpe_is16BitMode != 0) {
        if (pDpeConfig->Dpe_InBuf_OCC_Ext != 0x0) {
            if (pDpeConfig->Dpe_InBuf_OCC_Ext->getBufPA(0) != 0x0) {
                pConfigToKernel->DVP_EXT_SRC_13_OCCDV0 = (unsigned int)pDpeConfig->Dpe_InBuf_OCC_Ext->getBufPA(0);
                pConfigToKernel->DVP_EXT_SRC_15_OCCDV2 = (unsigned int)pDpeConfig->Dpe_InBuf_OCC_Ext->getBufPA(0);
            } else {
                LOG_INF("No DVP Ext OCC Buffer PA, may be released?");
            }
        }
        else {
            LOG_INF("No DVP Ext OCC Input Buffer!\n");
        }
        if (pDpeConfig->Dpe_OutBuf_ASF_HF_Ext != 0x0) {
            if (pDpeConfig->Dpe_OutBuf_ASF_HF_Ext->getBufPA(0) != 0x0) {
                pConfigToKernel->DVP_EXT_SRC_20_ASF_DV0 = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_HF_Ext->getBufPA(0);
                pConfigToKernel->DVP_EXT_SRC_22_ASF_DV2 = (unsigned int)pDpeConfig->Dpe_OutBuf_ASF_HF_Ext->getBufPA(0);
            } else {
                LOG_INF("No DVP Ext ASF Buffer PA, may be released?");
            }
        }
        else {
            LOG_INF("No DVP Ext ASF Output Buffer!\n");
        }

    }

    memcpy(&pConfigToKernel->DVP_CORE_00, &pDpeConfig->Dpe_DVPSettings.TuningBuf_CORE, sizeof(pDpeConfig->Dpe_DVPSettings.TuningBuf_CORE));

}

#define BYPASS_REG (0)
MBOOL DpeDrvImp::enque(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec)
{
    LOG_DBG("DPE Start Enque!!\n");
    MINT32 Ret;
    MUINT32 min;
    unsigned int num = DpeConfigVec.size();
    NSCam::NSIoPipe::DPEConfig* pDpeConfig;
    DPE_Request dpe_Request;
    DPE_Config dpe_ConfigToKernel[_SUPPORT_MAX_DPE_FRAME_REQUEST_];
    memset(&dpe_ConfigToKernel, 0, sizeof(dpe_ConfigToKernel));

    for (unsigned int i=0; i<DpeConfigVec.size(); i++)
    {
        pDpeConfig = &(DpeConfigVec.at(i));
        //LOG_INF("DPE ConfigVec[%d] Enque, BLD Mode: %d, SRZ En: %d\n", i, pDpeConfig->Dpe_bldmode, pDpeConfig->Dpe_srzEn);
        if (pDpeConfig->Dpe_engineSelect == MODE_DVS_DVP_BOTH) {
            DPE_Config_DVS(pDpeConfig, &dpe_ConfigToKernel[i]);
            DPE_Config_DVP(pDpeConfig, &dpe_ConfigToKernel[i]);
            dpe_ConfigToKernel[i].DPE_MODE = 0;
        }
        else if (pDpeConfig->Dpe_engineSelect == MODE_DVS_ONLY) {
            DPE_Config_DVS(pDpeConfig, &dpe_ConfigToKernel[i]);
            dpe_ConfigToKernel[i].DPE_MODE = 1;
        }
        else if (pDpeConfig->Dpe_engineSelect == MODE_DVP_ONLY) {
            DPE_Config_DVP(pDpeConfig, &dpe_ConfigToKernel[i]);
            dpe_ConfigToKernel[i].DPE_MODE = 2;
        }
        if (g_isDPELogEnable == 1) {
            dpe_ConfigToKernel[i].USERDUMP_EN = 1;
        } else {
            dpe_ConfigToKernel[i].USERDUMP_EN = 0;
        }
    }

    dpe_Request.m_ReqNum = num;
    dpe_Request.m_pDpeConfig = dpe_ConfigToKernel;

    LOG_INF("Start to trigger, ReqNum = %d, DVS_SRC: 0x%08X, DVP_SRC: 0x%08X\n",
        dpe_Request.m_ReqNum, dpe_ConfigToKernel[0].DVS_SRC_05_L_FRM0, dpe_ConfigToKernel[0].DVP_SRC_05_Y_FRM0);

    Ret = ioctl(this->m_Fd, DPE_ENQUE_REQ, &dpe_Request);
    if(Ret < 0)
    {
        LOG_ERR("DPE_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    g_enqueFlag = !g_enqueFlag;

    return MTRUE;
}

MBOOL DpeDrvImp::deque(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::DPEConfig DpeConfig;
    DPE_Request dpe_Request;
    DPE_Config dpe_ConfigToKernel[_SUPPORT_MAX_DPE_FRAME_REQUEST_];
    dpe_Request.m_pDpeConfig = dpe_ConfigToKernel;

    Ret = ioctl(this->m_Fd, DPE_DEQUE_REQ,&dpe_Request);
    if(Ret < 0)
    {
        LOG_ERR("DPE_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    LOG_DBG("dequeDPE num:%d\n", dpe_Request.m_ReqNum);
    for (unsigned int i=0; i< dpe_Request.m_ReqNum; i++)
    {
        /* TODO: Engine-specific statistics for feature use */
        #if 0
            DpeConfig.feedback.reg1 = 0xFFFFFFFF;
            DpeConfig.feedback.reg2 = 0xFFFFFFFF;
        #endif
        DpeConfigVec.push_back(DpeConfig);
    }

    return MTRUE;
}

MUINT32 DpeDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = DPE_REG_RANGE;
    (void) caller;

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
MBOOL DpeDrvImp::readRegs(DPE_DRV_REG_IO_STRUCT* pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = DPE_REG_RANGE;
    (void) caller;

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
    MINT32 ret=0;
    MUINT32 legal_range = DPE_REG_RANGE;
    (void) caller;

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
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = DPE_REG_RANGE;
    (void) caller;

    android::Mutex::Autolock lock(this->DpeRegMutex);
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

