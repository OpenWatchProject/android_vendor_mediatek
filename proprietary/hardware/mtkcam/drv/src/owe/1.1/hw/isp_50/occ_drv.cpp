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
#define LOG_TAG "OccDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_owe.h"
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <occ_drv.h>
//#include <oweunittest.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(occ_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (occ_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (occ_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (occ_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (occ_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (occ_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (occ_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/* #define BYPASS_REG */
#define BYPASS_RULES

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

MINT32              OccDrvImp::m_Fd = -1;
volatile MINT32     OccDrvImp::m_UserCnt = 0;
OWE_DRV_RW_MODE     OccDrvImp::m_regRWMode = OWE_DRV_R_ONLY;
MUINT32*            OccDrvImp::m_pHwRegAddr = NULL;
android::Mutex      OccDrvImp::mInitMutex;


char                OccDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

OccDrvImp::OccDrvImp()
{
    DBG_LOG_CONFIG(drv, occ_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pHwRegAddr = NULL;
}

template<>
EngineDrv<OCCConfig>* EngineDrv<OCCConfig>::createInstance()
{
    return OccDrvImp::getInstance();
}

static OccDrvImp singleton;

EngineDrv<OCCConfig>* OccDrvImp::getInstance()
{
    singleton.ctl_tbl = occ_ctl;

    LOG_DBG("singleton[0x%p].",static_cast<void *>(&singleton));

    return &singleton;
}


MBOOL OccDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(mInitMutex);
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

    // Open owe device
    this->m_Fd = open(OWE_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("OWE kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(OWE_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("OWE kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=OWE_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pHwRegAddr = (MUINT32 *) mmap(0, OWE_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, OWE_BASE_HW);

            if(m_pHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("OWE mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=OWE_DRV_RW_MMAP;
    }


    if(ioctl(this->m_Fd, OWE_RESET, NULL) < 0){
        LOG_ERR("OWE Reset fail !!\n");
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
            munmap(m_pHwRegAddr, OWE_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close owe device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pHwRegAddr(0x%lx)", Result, m_UserCnt, (unsigned long)m_pHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL OccDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(mInitMutex);
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

    if(m_pHwRegAddr != MAP_FAILED){
         munmap(m_pHwRegAddr, OWE_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
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

MBOOL OccDrvImp::waitIrq(OWE_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    OWE_IRQ_CLEAR_ENUM OrgClr;
    OweDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    OWE_WAIT_IRQ_STRUCT waitirq;
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
        Ret = ioctl(this->m_Fd,OWE_WAIT_IRQ,&waitirq);
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
        LOG_ERR("OWE_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL OccDrvImp::clearIrq(OWE_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    OWE_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(OWE_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,OWE_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("OWE_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL OccDrvImp::waitFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    OWE_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = OWE_IRQ_WAIT_CLEAR;
    WaitIrq.Type = OWE_IRQ_TYPE_INT_OWE_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    CAM_TRACE_BEGIN("OWE waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & OWE_OCC_INT_ST)
        {
            LOG_DBG("OWE Wait OCC Interupt Frame Done Success!!\n");
        }
        else
        {
            LOG_DBG("OWE Wait WMFE Interupt Frame Done Success!!\n");
        }
        CAM_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & OWE_OCC_INT_ST)
        {
            LOG_DBG("OWE Wait OCC Interupt Frame Done Fail!!\n");
        }
        else
        {
            LOG_DBG("OWE Wait WMFE Interupt Frame Done Fail!!\n");
        }
        CAM_TRACE_END();
        return MFALSE;
    }


}

#define MIN(A, B) ((pOccConfig->A) < (pOccConfig->B) ? (pOccConfig->A) : (pOccConfig->B))
MBOOL OccDrvImp::checkOCCSetting(NSCam::NSIoPipe::OCCConfig* pOccConfig)
{
#ifndef BYPASS_RULES
    int x;

    /* CROP S/E */
    if (pOccConfig->occ_h_skip_mode == 1 && pOccConfig->occ_h_crop_e % 2 == 0) {
	LOG_ERR("h_crop_e(%d) should be odd for h_skip_mode = 1\n", pOccConfig->occ_h_crop_e);
	return MFALSE;
    }
    if (pOccConfig->occ_h_skip_mode == 1 && pOccConfig->occ_h_crop_s % 2 == 1) {
	LOG_ERR("h_crop_s(%d) should be even for h_skip_mode = 1\n", pOccConfig->occ_h_crop_s);
	return MFALSE;
    }

    /* REF_VEC */
    if (pOccConfig->OCC_REF_VEC.u4Stride < pOccConfig->occ_hsize * 2) {
	LOG_ERR("ref_vec_stride(%d) >= 2 * hsize(%d)\n", pOccConfig->OCC_REF_VEC.u4Stride, pOccConfig->occ_hsize);
	return MFALSE;
    }
    if (pOccConfig->OCC_REF_VEC.u4BufSize < pOccConfig->OCC_REF_VEC.u4Stride * pOccConfig->occ_vsize) {
	LOG_ERR("ref_vec_buf_size(%d) >= stride(%d)*vsize(%d)\n",
		pOccConfig->OCC_REF_VEC.u4BufSize, pOccConfig->OCC_REF_VEC.u4Stride, pOccConfig->occ_vsize);
	return MFALSE;
    }

    /* REF_PXL */
    if (pOccConfig->occ_imgi_ref_fmt != OWE_IMGI_Y_FMT)
        if (pOccConfig->OCC_REF_PXL.u4Stride < pOccConfig->occ_hsize * 2) {
	    LOG_ERR("ref_pix_stride(%d) >= 2 * hsize(%d)\n", pOccConfig->OCC_REF_PXL.u4Stride, pOccConfig->occ_hsize);
	    return MFALSE;
        }
    else
        if (pOccConfig->OCC_REF_PXL.u4Stride < pOccConfig->occ_hsize) {
	    LOG_ERR("ref_pix_stride(%d) >= 1 * hsize(%d)\n", pOccConfig->OCC_REF_PXL.u4Stride, pOccConfig->occ_hsize);
	    return MFALSE;
        }
    if (pOccConfig->OCC_REF_PXL.u4BufSize < pOccConfig->OCC_REF_PXL.u4Stride * pOccConfig->occ_vsize) {
	LOG_ERR("ref_pix_buf_size(%d) >= stride(%d)*vsize(%d)\n",
		pOccConfig->OCC_REF_PXL.u4BufSize, pOccConfig->OCC_REF_PXL.u4Stride, pOccConfig->occ_vsize);
	return MFALSE;
    }

    /* MAJ_VEC */
    if (pOccConfig->OCC_MAJ_VEC.u4Stride < pOccConfig->occ_hsize * 2) {
	LOG_ERR("maj_vec_stride(%d) >= 2 * hsize(%d)\n", pOccConfig->OCC_MAJ_VEC.u4Stride, pOccConfig->occ_hsize);
	return MFALSE;
    }
    if (pOccConfig->OCC_MAJ_VEC.u4BufSize < pOccConfig->OCC_MAJ_VEC.u4Stride * pOccConfig->occ_vsize) {
	LOG_ERR("maj_vec_buf_size(%d) >= stride(%d)*vsize(%d)\n",
		pOccConfig->OCC_MAJ_VEC.u4BufSize, pOccConfig->OCC_MAJ_VEC.u4Stride, pOccConfig->occ_vsize);
	return MFALSE;
    }

    /* MAJ_PXL */
    if (pOccConfig->occ_imgi_maj_fmt != OWE_IMGI_Y_FMT)
        if (pOccConfig->OCC_MAJ_PXL.u4Stride < pOccConfig->occ_hsize * 2) {
	    LOG_ERR("maj_pix_stride(%d) >= 2 * hsize(%d)\n", pOccConfig->OCC_MAJ_PXL.u4Stride, pOccConfig->occ_hsize);
	    return MFALSE;
        }
    else
        if (pOccConfig->OCC_MAJ_PXL.u4Stride < pOccConfig->occ_hsize) {
	    LOG_ERR("maj_pix_stride(%d) >= 1 * hsize(%d)\n", pOccConfig->OCC_MAJ_PXL.u4Stride, pOccConfig->occ_hsize);
	    return MFALSE;
        }
    if (pOccConfig->OCC_MAJ_PXL.u4BufSize < pOccConfig->OCC_MAJ_PXL.u4Stride * pOccConfig->occ_vsize) {
	LOG_ERR("maj_pix_buf_size(%d) >= stride(%d)*vsize(%d)\n",
		pOccConfig->OCC_MAJ_PXL.u4BufSize, pOccConfig->OCC_MAJ_PXL.u4Stride, pOccConfig->occ_vsize);
	return MFALSE;
    }

    /* WDMA */
    x = (MIN(occ_h_crop_e + 1, occ_hsize) - pOccConfig->occ_h_crop_s) / (pOccConfig->occ_h_skip_mode + 1);
    if (pOccConfig->OCC_WDMA.u4Stride < x) {
        LOG_ERR("wdma_stride(%d) >= (%d)\n", pOccConfig->OCC_WDMA.u4Stride, x);
	    return MFALSE;
    }
    x = MIN(occ_v_crop_e + 1, occ_vsize) - pOccConfig->occ_v_crop_s;
    if (pOccConfig->OCC_WDMA.u4BufSize < pOccConfig->OCC_WDMA.u4Stride * x) {
	LOG_ERR("wdma_buf_size(%d) >= stride(%d) * vsize'(%d)\n",
		pOccConfig->OCC_WDMA.u4BufSize, pOccConfig->OCC_WDMA.u4Stride, x);
	return MFALSE;
    }

#endif
    return MTRUE;
}


MBOOL OccDrvImp::enque(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec)
{
    MINT32 Ret, xsize;
    unsigned int num = OccConfigVec.size();
    NSCam::NSIoPipe::OCCConfig* pOccConfig;
    OWE_OCCRequest owe_occRequest;
    OWE_OCCConfig owe_OccConfig[_SUPPORT_MAX_OWE_FRAME_REQUEST_];


    for (unsigned int i=0; i<OccConfigVec.size(); i++)
    {
        pOccConfig = &(OccConfigVec.at(i));
        if (MFALSE == checkOCCSetting(pOccConfig))
        {
            LOG_ERR("OCC Enque setting is wrong!!idx(%d), numb(%d)\n", i, num);
            return MFALSE;
        }
#ifndef BYPASS_REG
#define FIELD(R,S) ((pOccConfig->R) << S)
        owe_OccConfig[i].DPE_OCC_CTRL_0 =  FIELD(occ_scan_r2l,0)|FIELD(occ_horz_ds4,1)|FIELD(occ_vert_ds4,2)|FIELD(occ_h_skip_mode,3)|FIELD(occ_imgi_maj_fmt,8)|FIELD(occ_imgi_ref_fmt,10);
        owe_OccConfig[i].DPE_OCC_CTRL_1 =  FIELD(occ_hsize,0)|FIELD(occ_vsize,16);
        owe_OccConfig[i].DPE_OCC_CTRL_2 =  FIELD(occ_v_crop_s,0)|FIELD(occ_v_crop_e,16);
        owe_OccConfig[i].DPE_OCC_CTRL_3 =  FIELD(occ_h_crop_s,0)|FIELD(occ_h_crop_e,16);
        owe_OccConfig[i].DPE_OCC_PQ_0   =  FIELD(occ_th_luma,0)|FIELD(occ_th_h,8)|FIELD(occ_th_v,16)|FIELD(occ_vec_shift,20)|FIELD(occ_vec_offset,24);
        owe_OccConfig[i].DPE_OCC_PQ_1   =  FIELD(occ_invalid_value,0)|FIELD(occ_owc_th,8)|FIELD(occ_owc_en,16)|FIELD(occ_depth_clip_en,17);
        owe_OccConfig[i].DPE_OCC_SPARE  =  FIELD(occ_spare,0);

        owe_OccConfig[i].DPE_OCC_REF_VEC_BASE   =  FIELD(OCC_REF_VEC.u4BufPA, 0);
        owe_OccConfig[i].DPE_OCC_REF_VEC_STRIDE =  FIELD(OCC_REF_VEC.u4Stride, 0)|FIELD(OCC_REF_VEC.u4Stride - 1, 16);
        owe_OccConfig[i].DPE_OCC_REF_PXL_BASE   =  FIELD(OCC_REF_PXL.u4BufPA, 0);
        owe_OccConfig[i].DPE_OCC_REF_PXL_STRIDE =  FIELD(OCC_REF_PXL.u4Stride, 0)|FIELD(OCC_REF_PXL.u4Stride - 1, 16);
        owe_OccConfig[i].DPE_OCC_MAJ_VEC_BASE   =  FIELD(OCC_MAJ_VEC.u4BufPA, 0);
        owe_OccConfig[i].DPE_OCC_MAJ_VEC_STRIDE =  FIELD(OCC_MAJ_VEC.u4Stride, 0)|FIELD(OCC_MAJ_VEC.u4Stride - 1, 16);
        owe_OccConfig[i].DPE_OCC_MAJ_PXL_BASE   =  FIELD(OCC_MAJ_PXL.u4BufPA, 0);
        owe_OccConfig[i].DPE_OCC_MAJ_PXL_STRIDE =  FIELD(OCC_MAJ_PXL.u4Stride, 0)|FIELD(OCC_MAJ_PXL.u4Stride - 1, 16);
        owe_OccConfig[i].DPE_OCC_WDMA_BASE      =  FIELD(OCC_WDMA.u4BufPA, 0);

        xsize = ((MIN(occ_h_crop_e + 1, occ_hsize) - pOccConfig->occ_h_crop_s) / (pOccConfig->occ_h_skip_mode + 1)) - 1;
        owe_OccConfig[i].DPE_OCC_WDMA_STRIDE    =  FIELD(OCC_WDMA.u4Stride, 0)| (xsize << 16);

        owe_OccConfig[i].eng_secured = pOccConfig->egn_secured;
        if (pOccConfig->OCC_REF_VEC.is_sec == 1)
            owe_OccConfig[i].dma_sec_size[OCC_DMA_REF_VEC] = pOccConfig->OCC_REF_VEC.u4BufSize;
        else
            owe_OccConfig[i].dma_sec_size[OCC_DMA_REF_VEC] = 0;

        if (pOccConfig->OCC_REF_PXL.is_sec == 1)
            owe_OccConfig[i].dma_sec_size[OCC_DMA_REF_PXL] = pOccConfig->OCC_REF_PXL.u4BufSize;
        else
            owe_OccConfig[i].dma_sec_size[OCC_DMA_REF_PXL] = 0;

        if (pOccConfig->OCC_MAJ_VEC.is_sec == 1)
            owe_OccConfig[i].dma_sec_size[OCC_DMA_MAJ_VEC] = pOccConfig->OCC_MAJ_VEC.u4BufSize;
        else
            owe_OccConfig[i].dma_sec_size[OCC_DMA_MAJ_VEC] = 0;

        if (pOccConfig->OCC_MAJ_PXL.is_sec == 1)
            owe_OccConfig[i].dma_sec_size[OCC_DMA_MAJ_PXL] = pOccConfig->OCC_MAJ_PXL.u4BufSize;
        else
            owe_OccConfig[i].dma_sec_size[OCC_DMA_MAJ_PXL] = 0;

        if (pOccConfig->OCC_WDMA.is_sec == 1)
            owe_OccConfig[i].dma_sec_size[OCC_DMA_WDMA] = pOccConfig->OCC_WDMA.u4BufSize;
        else
            owe_OccConfig[i].dma_sec_size[OCC_DMA_WDMA] = 0;

#if 0
        LOG_INF("frm(%d)\n\
            ,imgi_l_addr(0x%lx), imgi_l_stride(0x%x), imgi_l_size(0x%x), imgi_r_addr(0x%lx), imgi_r_stride(0x%x), imgi_r_size(0x%x)\n\
            ,dvi_l_addr(0x%lx), dvi_l_stride(0x%x), dvi_l_size(0x%x), dvi_r_addr(0x%lx), dvi_r_stride(0x%x), dvi_r_size(0x%x)\n\
            ,maski_l_addr(0x%lx), maski_l_stride(0x%x), maski_l_size(0x%x), maski_r_addr(0x%lx), maski_r_stride(0x%x), maski_r_size(0x%x)\n\
            ,dvo_l_addr(0x%lx), dvo_l_stride(0x%x), dvo_l_size(0x%x), dvo_r_addr(0x%lx), dvo_r_stride(0x%x), dvo_r_size(0x%x)\n\
            ,confo_l_addr(0x%lx), confo_l_stride(0x%x), confo_l_size(0x%x), confo_r_addr(0x%lx), confo_r_stride(0x%x), confo_r_size(0x%x)\n\
            ,respo_l_addr(0x%lx), respo_l_stride(0x%x), respo_l_size(0x%x), respo_r_addr(0x%lx), respo_r_stride(0x%x), respo_r_size(0x%x)\n",i, 
            (unsigned long)pOccConfig->Occ_Imgi_l.u4BufPA, pOccConfig->Occ_Imgi_l.u4Stride, pOccConfig->Occ_Imgi_l.u4BufSize, (unsigned long)pOccConfig->Occ_Imgi_r.u4BufPA, pOccConfig->Occ_Imgi_r.u4Stride, pOccConfig->Occ_Imgi_r.u4BufSize,
            (unsigned long)pOccConfig->Occ_Dvi_l.u4BufPA, pOccConfig->Occ_Dvi_l.u4Stride, pOccConfig->Occ_Dvi_l.u4BufSize, (unsigned long)pOccConfig->Occ_Dvi_r.u4BufPA, pOccConfig->Occ_Dvi_r.u4Stride, pOccConfig->Occ_Dvi_r.u4BufSize,
            (unsigned long)pOccConfig->Occ_Maski_l.u4BufPA, pOccConfig->Occ_Maski_l.u4Stride, pOccConfig->Occ_Maski_l.u4BufSize, (unsigned long)pOccConfig->Occ_Maski_r.u4BufPA, pOccConfig->Occ_Maski_r.u4Stride, pOccConfig->Occ_Maski_r.u4BufSize,
            (unsigned long)pOccConfig->Occ_Dvo_l.u4BufPA, pOccConfig->Occ_Dvo_l.u4Stride, pOccConfig->Occ_Dvo_l.u4BufSize, (unsigned long)pOccConfig->Occ_Dvo_r.u4BufPA, pOccConfig->Occ_Dvo_r.u4Stride, pOccConfig->Occ_Dvo_r.u4BufSize,
            (unsigned long)pOccConfig->Occ_Confo_l.u4BufPA, pOccConfig->Occ_Confo_l.u4Stride, pOccConfig->Occ_Confo_l.u4BufSize, (unsigned long)pOccConfig->Occ_Confo_r.u4BufPA, pOccConfig->Occ_Confo_r.u4Stride, pOccConfig->Occ_Confo_r.u4BufSize,
            (unsigned long)pOccConfig->Occ_Respo_l.u4BufPA, pOccConfig->Occ_Respo_l.u4Stride, pOccConfig->Occ_Respo_l.u4BufSize, (unsigned long)pOccConfig->Occ_Respo_r.u4BufPA, pOccConfig->Occ_Respo_r.u4Stride, pOccConfig->Occ_Respo_r.u4BufSize);
#endif
#endif
    }

    owe_occRequest.m_ReqNum = num;
    owe_occRequest.m_pOweConfig = owe_OccConfig;

    Ret = ioctl(this->m_Fd,OWE_OCC_ENQUE_REQ,&owe_occRequest);
    if(Ret < 0)
    {
        LOG_ERR("OWE_OCC_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    return MTRUE;


}

MBOOL OccDrvImp::deque(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::OCCConfig OccConfig;
    OWE_OCCRequest owe_occRequest;
    OWE_OCCConfig owe_OccConfig[_SUPPORT_MAX_OWE_FRAME_REQUEST_];
    owe_occRequest.m_pOweConfig = owe_OccConfig;

    Ret = ioctl(this->m_Fd,OWE_OCC_DEQUE_REQ,&owe_occRequest);
    if(Ret < 0)
    {
        LOG_ERR("OWE_OCC_DEQUE_REQ fail(%d)\n", Ret);
        printf("OWE_OCC_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    printf("dequeOCC num:%d\n", owe_occRequest.m_ReqNum);

    for (unsigned int i=0; i< owe_occRequest.m_ReqNum; i++)
    {
#ifndef BYPASS_REG
        OccConfig.OCC_REF_VEC.u4BufPA    = owe_occRequest.m_pOweConfig[i].DPE_OCC_REF_VEC_BASE;  
        OccConfig.OCC_REF_VEC.u4Stride   = owe_occRequest.m_pOweConfig[i].DPE_OCC_REF_VEC_STRIDE;
        OccConfig.OCC_REF_PXL.u4BufPA    = owe_occRequest.m_pOweConfig[i].DPE_OCC_REF_PXL_BASE;  
        OccConfig.OCC_REF_PXL.u4Stride   = owe_occRequest.m_pOweConfig[i].DPE_OCC_REF_PXL_STRIDE;
        OccConfig.OCC_MAJ_VEC.u4BufPA    = owe_occRequest.m_pOweConfig[i].DPE_OCC_MAJ_VEC_BASE;  
        OccConfig.OCC_MAJ_VEC.u4Stride   = owe_occRequest.m_pOweConfig[i].DPE_OCC_MAJ_VEC_STRIDE;
        OccConfig.OCC_MAJ_PXL.u4BufPA    = owe_occRequest.m_pOweConfig[i].DPE_OCC_MAJ_PXL_BASE;  
        OccConfig.OCC_MAJ_PXL.u4Stride   = owe_occRequest.m_pOweConfig[i].DPE_OCC_MAJ_PXL_STRIDE;
        OccConfig.OCC_WDMA.u4BufPA       = owe_occRequest.m_pOweConfig[i].DPE_OCC_WDMA_BASE;     
        OccConfig.OCC_WDMA.u4Stride      = owe_occRequest.m_pOweConfig[i].DPE_OCC_WDMA_STRIDE;   
#endif
        OccConfigVec.push_back(OccConfig);

    }

    return MTRUE;
}

MUINT32 OccDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    (void)caller;
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = OWE_REG_RANGE;
    LOG_DBG("+,Owe_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->RegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==OWE_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pHwRegAddr[(Addr>>2)];
    }
    else{
        OWE_REG_IO_STRUCT OweRegIo;
        OWE_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        OweRegIo.pData = (OWE_REG_STRUCT*)&RegIo;
        OweRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, OWE_READ_REGISTER, &OweRegIo);
        if(Ret < 0)
        {
            LOG_ERR("OWE_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Owe_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL OccDrvImp::readRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)caller;
    MINT32 Ret;
    MUINT32 legal_range = OWE_REG_RANGE;
    android::Mutex::Autolock lock(this->RegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == OWE_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        OWE_REG_IO_STRUCT OweRegIo;
        //pRegIo->module = this->m_HWmodule;
        OweRegIo.pData = (OWE_REG_STRUCT*)pRegIo;
        OweRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, OWE_READ_REGISTER, &OweRegIo);
        if(Ret < 0)
        {
            LOG_ERR("OWE_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Owe_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL OccDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    (void)caller;
    MINT32 ret=0;
    MUINT32 legal_range = OWE_REG_RANGE;
    LOG_DBG("Owe_write:m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->RegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case OWE_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pHwRegAddr[(Addr>>2)] = Data;
            break;
        case OWE_DRV_RW_IOCTL:
            OWE_REG_IO_STRUCT OweRegIo;
            OWE_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            OweRegIo.pData = (OWE_REG_STRUCT*)&RegIo;
            OweRegIo.Count = 1;
            ret = ioctl(this->m_Fd, OWE_WRITE_REGISTER, &OweRegIo);
            if(ret < 0){
                LOG_ERR("OWE_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case OWE_DRV_R_ONLY:
            LOG_ERR("OWE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->RegMutex.unlock();
    //
    return MTRUE;
}

MBOOL OccDrvImp::writeRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    (void)caller;
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = OWE_REG_RANGE;
    android::Mutex::Autolock lock(this->RegMutex);
    //(void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case OWE_DRV_RW_IOCTL:
            OWE_REG_IO_STRUCT OweRegIo;
            //pRegIo->module = this->m_HWmodule;
            OweRegIo.pData = (OWE_REG_STRUCT*)pRegIo;
            OweRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, OWE_WRITE_REGISTER, &OweRegIo);
            if(Ret < 0){
                LOG_ERR("OWE_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case OWE_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case OWE_DRV_R_ONLY:
            LOG_ERR("OWE Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Owe_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 OccDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL OccDrvImp::setRWMode(OWE_DRV_RW_MODE rwMode)
{
    if(rwMode > OWE_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

