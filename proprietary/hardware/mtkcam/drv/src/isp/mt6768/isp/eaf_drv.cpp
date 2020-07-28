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
#define LOG_TAG "EafDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_eaf.h"
#include <mtkcam/def/common.h>
#include <drv/eaf_drv.h>
#include <drv/imem_drv.h>
//tpipe
#include <tpipe_config.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

DECLARE_DBG_LOG_VARIABLE(eaf_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (eaf_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (eaf_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (eaf_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (eaf_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (eaf_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (eaf_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   20
#define CROP_CDP_HARDWARE_BIT   8

#define FAKE_TILE_CALCULATION //QQ

class EafDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    EafDbgTimer(char const*const pszTitle)
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

MINT32              EafDrv::m_Fd = -1;
volatile MINT32     EafDrvImp::m_UserCnt = 0;
EAF_DRV_RW_MODE     EafDrv::m_regRWMode = EAF_DRV_R_ONLY;
MUINT32*            EafDrvImp::m_pEafHwRegAddr = NULL;
android::Mutex      EafDrvImp::m_EafInitMutex;


#if 1
char                EafDrvImp::m_UserName[EAF_MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};
#endif

EafDrvImp::EafDrvImp()
{
    DBG_LOG_CONFIG(drv, eaf_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pEafHwRegAddr = NULL;
}

EafDrv* EafDrv::createInstance()
{
    return EafDrvImp::getInstance();
}

static EafDrvImp singleton;

EafDrv* EafDrvImp::getInstance()
{
    LOG_DBG("singleton[0x%08x].", &singleton);

    return &singleton;
}

IMEM_BUF_INFO  gTileBuffer;
IMEM_BUF_INFO  gTileDesc;
IMemDrv*	   m_pIMemDrv = NULL;
ISP_TPIPE_CONFIG_STRUCT *pTpipeCfgInfo;
ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer 

MBOOL EafDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    //
    android::Mutex::Autolock lock(this->m_EafInitMutex);
    //
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to use eaf driver\n");
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

    // Open eaf device
    this->m_Fd = open(EAF_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("EAF kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(EAF_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("EAF kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=EAF_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
            // fd opened only once at the very 1st init
            m_pEafHwRegAddr = (MUINT32 *) mmap(0, EAF_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, EAF_BASE_HW);

            if(m_pEafHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("EAF mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            this->m_regRWMode=EAF_DRV_RW_MMAP;
    }

// allocate Tpipe buffer		
	    m_pIMemDrv = IMemDrv::createInstance();
	    m_pIMemDrv->init();

		gTileDesc.size = 256*256;
	    gTileDesc.useNoncache = 1; //alloc non-cacheable mem for tpipe.
	    if ( m_pIMemDrv->allocVirtBuf(&gTileDesc) ) {
	        LOG_INF("[ERROR]allocVirtBuf(gTileDesc) fail,size(0x%x)",gTileDesc.size);
			
			Result = MFALSE;
	        goto EXIT;
	    }
	    if ( m_pIMemDrv->mapPhyAddr(&gTileDesc) ) {
	        LOG_ERR("[ERROR]m_pIMemDrv->mapPhyAddr(tpipeBufInfo)");
			Result = MFALSE;
	        goto EXIT;
	    }
		
#ifdef FAKE_TILE_CALCULATION
		gTileBuffer.size = 1024*8;//QQ
#else
		gTileBuffer.size = tpipe_main_query_platform_working_buffer_size(8192);//QQ
#endif
	    gTileBuffer.useNoncache = 1; //alloc non-cacheable mem for tpipe.
	    if ( m_pIMemDrv->allocVirtBuf(&gTileBuffer) ) {
	        LOG_INF("[ERROR]allocVirtBuf(gTileBuffer) fail,size(0x%x)",gTileBuffer.size);
			
			Result = MFALSE;
	        goto EXIT;
	    }
	    if ( m_pIMemDrv->mapPhyAddr(&gTileBuffer) ) {
	        LOG_ERR("[ERROR]m_pIMemDrv->mapPhyAddr(tpipeBufInfo)");
			Result = MFALSE;
	        goto EXIT;
	    }
	    LOG_DBG("[Tile Buffer]size(0x%x),Va(0x%x),Pa(0x%x)",gTileBuffer.size,gTileBuffer.virtAddr,gTileBuffer.phyAddr);

    if(ioctl(this->m_Fd, EAF_RESET, NULL) < 0){
        LOG_ERR("EAF Reset fail !!\n");
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
            munmap(m_pEafHwRegAddr, EAF_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close eaf device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pEafHwRegAddr(0x%x)", Result, this->m_UserCnt, m_pEafHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL EafDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    //
    android::Mutex::Autolock lock(this->m_EafInitMutex);
    //
    //
    if (!((userName !=NULL) && (strlen(userName) <= (MAX_USER_NAME_SIZE))))
    {
        LOG_ERR("Plz add userName if you want to uninit eaf driver\n");
        return MFALSE;
    }

    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in EafDrv , curUser(%s)",userName);
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

    if(m_pEafHwRegAddr != MAP_FAILED){
         munmap(m_pEafHwRegAddr, EAF_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=EAF_DRV_R_ONLY;
    }

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

    if(this->m_UserCnt!= 0){
        LOG_INF("current working user:\n");
        for(MUINT32 i=0;i<EAF_MAX_USER_NUMBER;i+=8)
            LOG_INF("current user: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
            ,this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]  \
            ,this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
    }
    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no eaf device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL EafDrvImp::waitIrq(EAF_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    EAF_IRQ_CLEAR_ENUM OrgClr;
    EafDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    EAF_WAIT_IRQ_STRUCT waitirq;
	
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

    LOG_DBG(" Type(%d),Status(0x%08x),Timeout(%d us)\n",waitirq.Type, waitirq.Status, waitirq.Timeout);

    while( waitirq.Timeout > 0 )//receive restart system call again
    {
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,EAF_WAIT_IRQ,&waitirq);
        Tb=dbgTmr.getUs();
        if( Ret== (-SIG_ERESTARTSYS) )
        {
            waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
            LOG_DBG("ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d us)\n",waitirq.Type, waitirq.Status, waitirq.Timeout);
        }
        else
        {
            break;
        }
    }

    if(Ret < 0) {
        LOG_DBG("EAF_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL EafDrvImp::clearIrq(EAF_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    EAF_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(EAF_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,EAF_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("EAF_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}



MBOOL EafDrvImp::waitEAFFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    EAF_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = EAF_IRQ_WAIT_CLEAR;
    WaitIrq.Type = EAF_IRQ_TYPE_INT_EAF_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;


//    DRV_TRACE_BEGIN("EAF waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & EAF_INT_ST)
        {
            LOG_DBG("EAF Wait  Interupt Frame Done Success!!\n");
        }

//        DRV_TRACE_END();
        return MTRUE;
    }
    else
    {
        if (Status & EAF_INT_ST)
        {
            LOG_DBG("EAF Wait  Interupt Frame Done Fail!!\n");
        }
//        DRV_TRACE_END();
        return MFALSE;
    }


}



MBOOL EafDrvImp::enqueEAF(vector<NSCam::NSIoPipe::EAFConfig>& EafConfigParam)
{
    MINT32 Ret;
    unsigned int num = EafConfigParam.size();
    NSCam::NSIoPipe::EAFConfig* pEafConfig;
    EAF_Request eaf_Request;
    EAF_Config eaf_Config[_SUPPORT_MAX_EAF_FRAME_REQUEST_];
	
    LOG_DBG("EafDrvImp::enqueEAF \n");

	
    for (unsigned int i=0; i<EafConfigParam.size(); i++)
    {
        pEafConfig = &(EafConfigParam.at(i));
		
        memset((void*)&eaf_Config[i], 0x00, sizeof(eaf_Config[i]));

		eaf_Config[i].EAF_MAIN_CFG0 |= 
			(pEafConfig->MAIN_CFG0.eaf_en <<0)|
			(pEafConfig->MAIN_CFG0.jbfr_ref_sel <<1)|			
			(pEafConfig->MAIN_CFG0.eaf_hist_en << 2)|
			(pEafConfig->MAIN_CFG0.eaf_lkh_ext_map_en << 3)|	 
			(pEafConfig->MAIN_CFG0.eaf_jbfr_en << 4)|
			(pEafConfig->MAIN_CFG0.eaf_boxf_en << 5)|
			(pEafConfig->MAIN_CFG0.eaf_srz_en << 6)|
			(pEafConfig->MAIN_CFG0.eaf_prob_en << 7)|
			(pEafConfig->MAIN_CFG0.eaf_srz_pk8_en << 8)|
			(pEafConfig->MAIN_CFG0.eaf_hist_pk8_en << 9)|
			(pEafConfig->MAIN_CFG0.eaf_div_en << 10)|
			(pEafConfig->MAIN_CFG0.eaf_div_pk8_en << 11)|
			(pEafConfig->MAIN_CFG0.eaf_lkhf_en << 12)|
			(pEafConfig->MAIN_CFG0.eraf_jbfr_uvch_en << 19);

		eaf_Config[i].EAF_MAIN_CFG0 |= 0x0f000000;


		eaf_Config[i].EAF_MAIN_CFG1 |= 
			(pEafConfig->MAIN_CFG1.eafi_mask_en << 0)|
			(pEafConfig->MAIN_CFG1.eafi_cur_y_en << 1)|
			(pEafConfig->MAIN_CFG1.eafi_cur_uv_en << 2)|
			(pEafConfig->MAIN_CFG1.eafi_pre_y_en << 3)|
			(pEafConfig->MAIN_CFG1.eafi_pre_uv_en << 4)|
			(pEafConfig->MAIN_CFG1.eafi_dep_en << 5)|
			(pEafConfig->MAIN_CFG1.eafi_lkh_wmap_en << 6)|
			(pEafConfig->MAIN_CFG1.eafi_lkh_emap_en << 7)|
			(pEafConfig->MAIN_CFG1.eafi_prob_en << 8)|
			(pEafConfig->MAIN_CFG1.eafo_fout_en << 9)|
			(pEafConfig->MAIN_CFG1.eafo_pout_en << 10)|
			(pEafConfig->MAIN_CFG1.eafi_en << 16)|
			(pEafConfig->MAIN_CFG1.eaf2i_en << 17)|
			(pEafConfig->MAIN_CFG1.eaf3i_en << 18)|
			(pEafConfig->MAIN_CFG1.eaf4i_en << 19)|
			(pEafConfig->MAIN_CFG1.eaf5i_en << 20)|
			(pEafConfig->MAIN_CFG1.eaf6i_en << 21)|
			(pEafConfig->MAIN_CFG1.eafo_en << 22);

		if (pEafConfig->mode == 1)
		{	
			eaf_Config[i].EAF_MAIN_CFG0 |= 0x0f000000;
			eaf_Config[i].EAF_MAIN_CFG3 = 0x0002c688;
			eaf_Config[i].EAF_TDRI_BASE_ADDR = 0x0;
		}
		else if (pEafConfig->mode == 4)
		{		
			eaf_Config[i].EAF_MAIN_CFG0 |= 0x0f002000;// TDR enable
			eaf_Config[i].EAF_MAIN_CFG3 = 0x0002a843;
			eaf_Config[i].EAF_TDRI_BASE_ADDR = gTileBuffer.phyAddr; 
			#ifdef FAKE_TILE_CALCULATION
			eaf_Config[i].EAF_TDRI_BASE_ADDR = pEafConfig->EAF_TDRI_BASE_ADDR;
			#endif
		}
		else
		{
			LOG_ERR("Not support EAF mode(%d)\n", pEafConfig->mode);
			return MFALSE;
		}

		printf("EAF_MAIN_CFG0 %x \n",eaf_Config[i].EAF_MAIN_CFG0);
		printf("EAF_MAIN_CFG1 %x \n",eaf_Config[i].EAF_MAIN_CFG1);
		printf("EAF_MAIN_CFG3 %x \n",eaf_Config[i].EAF_MAIN_CFG3);
			
		if (pEafConfig->MAIN_CFG0.eaf_hist_en == 1)
		{
			eaf_Config[i].EAF_HIST_CFG0 = pEafConfig->EAF_HIST_CFG.ctrl;
			eaf_Config[i].EAF_HIST_CFG1 = (pEafConfig->EAF_HIST_CFG.h << 16)|(pEafConfig->EAF_HIST_CFG.w << 0);	
			printf("EAF_HIST_CFG0 %x \n",eaf_Config[i].EAF_HIST_CFG0);
			printf("EAF_HIST_CFG1 %x \n",eaf_Config[i].EAF_HIST_CFG1);
		}

		if (pEafConfig->MAIN_CFG0.eaf_srz_en == 1)
			eaf_Config[i].EAF_SRZ_CFG0 = (pEafConfig->EAF_SRZ_CFG.h << 16)|(pEafConfig->EAF_SRZ_CFG.w << 0); 

		printf("EAF_SRZ_CFG0 %x \n",eaf_Config[i].EAF_SRZ_CFG0);

		if (pEafConfig->MAIN_CFG0.eaf_boxf_en == 1)
			eaf_Config[i].EAF_BOXF_CFG0 = (pEafConfig->EAF_BOXF_CFG.h << 16)|(pEafConfig->EAF_BOXF_CFG.w << 0); 

		printf("EAF_BOXF_CFG0 %x \n",eaf_Config[i].EAF_BOXF_CFG0);

		if (pEafConfig->MAIN_CFG0.eaf_div_en == 1)
			eaf_Config[i].EAF_DIV_CFG0 = (pEafConfig->EAF_DIV_CFG.h << 16)|(pEafConfig->EAF_DIV_CFG.w << 0); 

		printf("EAF_DIV_CFG0 %x \n",eaf_Config[i].EAF_DIV_CFG0);
		
		if (pEafConfig->MAIN_CFG0.eaf_lkhf_en == 1)
		{
			eaf_Config[i].EAF_LKHF_CFG0 = (pEafConfig->EAF_LKHF_CFG.h << 16)|(pEafConfig->EAF_LKHF_CFG.w << 0); 
			eaf_Config[i].EAF_LKHF_CFG1 = pEafConfig->EAF_LKHF_CFG.ctrl;
			printf("EAF_LKHF_CFG0 %x \n",eaf_Config[i].EAF_LKHF_CFG0);
			printf("EAF_LKHF_CFG1 %x \n",eaf_Config[i].EAF_LKHF_CFG1);
		}
		
		if (pEafConfig->MAIN_CFG1.eafi_mask_en == 1)
		{
			eaf_Config[i].EAFI_MASK0 = pEafConfig->EAFI_MASK.u4BufPA;
			eaf_Config[i].EAFI_MASK1 = pEafConfig->EAFI_MASK.u4Offset;
			eaf_Config[i].EAFI_MASK2 = pEafConfig->EAFI_MASK.u4Stride;
			eaf_Config[i].EAFI_MASK3 = ((pEafConfig->EAFI_MASK.u4ImgHeight) << 16) |((pEafConfig->EAFI_MASK.u4ImgWidth) << 0);		
		}
		
		if (pEafConfig->MAIN_CFG1.eafi_cur_y_en == 1)
		{
			eaf_Config[i].EAFI_CUR_Y0 = pEafConfig->EAFI_CUR_Y.u4BufPA;
			eaf_Config[i].EAFI_CUR_Y1 = pEafConfig->EAFI_CUR_Y.u4Offset;
			eaf_Config[i].EAFI_CUR_Y2 = pEafConfig->EAFI_CUR_Y.u4Stride;
			eaf_Config[i].EAFI_CUR_Y3 = ((pEafConfig->EAFI_CUR_Y.u4ImgHeight) << 16) |((pEafConfig->EAFI_CUR_Y.u4ImgWidth) << 0);
		}

		if (pEafConfig->MAIN_CFG1.eafi_cur_uv_en == 1)
		{
			eaf_Config[i].EAFI_CUR_UV0 = pEafConfig->EAFI_CUR_UV.u4BufPA;
			eaf_Config[i].EAFI_CUR_UV1 = pEafConfig->EAFI_CUR_UV.u4Offset;
			eaf_Config[i].EAFI_CUR_UV2 = pEafConfig->EAFI_CUR_UV.u4Stride;
			eaf_Config[i].EAFI_CUR_UV3 = ((pEafConfig->EAFI_CUR_UV.u4ImgHeight) << 16) |((pEafConfig->EAFI_CUR_UV.u4ImgWidth) << 0);  
		}
		
		if (pEafConfig->MAIN_CFG1.eafi_pre_y_en == 1)
		{
			eaf_Config[i].EAFI_PRE_Y0 = pEafConfig->EAFI_PRE_Y.u4BufPA;
			eaf_Config[i].EAFI_PRE_Y1 = pEafConfig->EAFI_PRE_Y.u4Offset;
			eaf_Config[i].EAFI_PRE_Y2 = pEafConfig->EAFI_PRE_Y.u4Stride;
			eaf_Config[i].EAFI_PRE_Y3 = ((pEafConfig->EAFI_PRE_Y.u4ImgHeight) << 16) |((pEafConfig->EAFI_PRE_Y.u4ImgWidth) << 0);  
		}		
		
		if (pEafConfig->MAIN_CFG1.eafi_pre_uv_en == 1)
		{		
			eaf_Config[i].EAFI_PRE_UV0 = pEafConfig->EAFI_PRE_UV.u4BufPA;		   
			eaf_Config[i].EAFI_PRE_UV1 = pEafConfig->EAFI_PRE_UV.u4Offset;		   
			eaf_Config[i].EAFI_PRE_UV2 = pEafConfig->EAFI_PRE_UV.u4Stride;		   
			eaf_Config[i].EAFI_PRE_UV3 = ((pEafConfig->EAFI_PRE_UV.u4ImgHeight) << 16) |((pEafConfig->EAFI_PRE_UV.u4ImgWidth) << 0);   
		}		
		
		if (pEafConfig->MAIN_CFG1.eafi_dep_en == 1)
		{
			eaf_Config[i].EAFI_DEP0 = pEafConfig->EAFI_DEP.u4BufPA;				   
			eaf_Config[i].EAFI_DEP1 = pEafConfig->EAFI_DEP.u4Offset;				   
			eaf_Config[i].EAFI_DEP2 = pEafConfig->EAFI_DEP.u4Stride;				   
			eaf_Config[i].EAFI_DEP3 = ((pEafConfig->EAFI_DEP.u4ImgHeight) << 16) |((pEafConfig->EAFI_DEP.u4ImgWidth) << 0);	
		}		

		
		if (pEafConfig->MAIN_CFG1.eafi_lkh_wmap_en == 1)
		{
			eaf_Config[i].EAFI_LKH_WMAP0 = pEafConfig->EAFI_LKH_WMAP.u4BufPA;			   
			eaf_Config[i].EAFI_LKH_WMAP1 = pEafConfig->EAFI_LKH_WMAP.u4Offset;		   
			eaf_Config[i].EAFI_LKH_WMAP2 = pEafConfig->EAFI_LKH_WMAP.u4Stride;		   
			eaf_Config[i].EAFI_LKH_WMAP3 = ((pEafConfig->EAFI_LKH_WMAP.u4ImgHeight) << 16) |((pEafConfig->EAFI_LKH_WMAP.u4ImgWidth) << 0);	
		}		

		
		if (pEafConfig->MAIN_CFG1.eafi_lkh_emap_en == 1)
		{
			eaf_Config[i].EAFI_LKH_EMAP0 = pEafConfig->EAFI_LKH_EMAP.u4BufPA;		   
			eaf_Config[i].EAFI_LKH_EMAP1 = pEafConfig->EAFI_LKH_EMAP.u4Offset;		   
			eaf_Config[i].EAFI_LKH_EMAP2 = pEafConfig->EAFI_LKH_EMAP.u4Stride;		   
			eaf_Config[i].EAFI_LKH_EMAP3 = ((pEafConfig->EAFI_LKH_EMAP.u4ImgHeight) << 16) |((pEafConfig->EAFI_LKH_EMAP.u4ImgWidth) << 0);	
		}		
		
		if (pEafConfig->MAIN_CFG1.eafi_prob_en == 1)
		{
			eaf_Config[i].EAFI_PROB0 = pEafConfig->EAFI_PROB.u4BufPA;			   
			eaf_Config[i].EAFI_PROB1 = pEafConfig->EAFI_PROB.u4Offset;				   
			eaf_Config[i].EAFI_PROB2 = pEafConfig->EAFI_PROB.u4Stride;				   
			eaf_Config[i].EAFI_PROB3 = ((pEafConfig->EAFI_PROB.u4ImgHeight) << 16) |((pEafConfig->EAFI_PROB.u4ImgWidth) << 0);
		}		
		
		if (pEafConfig->MAIN_CFG1.eafo_fout_en == 1)
		{
			eaf_Config[i].EAFO_FOUT0 = pEafConfig->EAFO_FOUT.u4BufPA;				   
			eaf_Config[i].EAFO_FOUT1 = pEafConfig->EAFO_FOUT.u4Offset;				   
			eaf_Config[i].EAFO_FOUT2 = pEafConfig->EAFO_FOUT.u4Stride;			   
			eaf_Config[i].EAFO_FOUT3 = ((pEafConfig->EAFO_FOUT.u4ImgHeight) << 16) |((pEafConfig->EAFO_FOUT.u4ImgWidth) << 0);			   
			eaf_Config[i].EAFO_FOUT4 = (pEafConfig->EAFO_FOUT.u4YOffset << 16) |(pEafConfig->EAFO_FOUT.u4XOffset << 0);			   	
		}		
		
		if (pEafConfig->MAIN_CFG1.eafo_pout_en == 1)
		{
			eaf_Config[i].EAFO_POUT0 = pEafConfig->EAFO_POUT.u4BufPA;				   
			eaf_Config[i].EAFO_POUT1 = pEafConfig->EAFO_POUT.u4Offset;				   
			eaf_Config[i].EAFO_POUT2 = pEafConfig->EAFO_POUT.u4Stride;			   
			eaf_Config[i].EAFO_POUT3 = ((pEafConfig->EAFO_POUT.u4ImgHeight) << 16) |((pEafConfig->EAFO_POUT.u4ImgWidth) << 0);			   
			eaf_Config[i].EAFO_POUT4 = (pEafConfig->EAFO_POUT.u4YOffset << 16) |(pEafConfig->EAFO_POUT.u4XOffset << 0);	
		}		

		eaf_Config[i].EAF_MASK_LB_CTL0 = pEafConfig->EAF_MASK_LB_CTL.ctrl; 		   
		eaf_Config[i].EAF_MASK_LB_CTL1 = (pEafConfig->EAF_MASK_LB_CTL.h << 16)|(pEafConfig->EAF_MASK_LB_CTL.w << 0);			  

		eaf_Config[i].EAF_PRE_Y_LB_CTL0 = pEafConfig->EAF_PRE_Y_LB_CTL.ctrl;			   
		eaf_Config[i].EAF_PRE_Y_LB_CTL1 = (pEafConfig->EAF_PRE_Y_LB_CTL.h << 16)|(pEafConfig->EAF_PRE_Y_LB_CTL.w << 0);		   

		eaf_Config[i].EAF_PRE_UV_LB_CTL0 = pEafConfig->EAF_PRE_UV_LB_CTL.ctrl;			   
		eaf_Config[i].EAF_PRE_UV_LB_CTL1 = (pEafConfig->EAF_PRE_UV_LB_CTL.h << 16)|(pEafConfig->EAF_PRE_UV_LB_CTL.w << 0);		

		eaf_Config[i].EAF_CUR_UV_LB_CTL0 = pEafConfig->EAF_CUR_UV_LB_CTL.ctrl;		   
		eaf_Config[i].EAF_CUR_UV_LB_CTL1 = (pEafConfig->EAF_CUR_UV_LB_CTL.h << 16)|(pEafConfig->EAF_CUR_UV_LB_CTL.w << 0);		   

	
		if(pEafConfig->MAIN_CFG0.eaf_jbfr_en != 0)
		{
			eaf_Config[i].JBFR_CFG0 = pEafConfig->jbfrcfg.cfg0;
			eaf_Config[i].JBFR_CFG1 = pEafConfig->jbfrcfg.cfg1;
			eaf_Config[i].JBFR_CFG2 = pEafConfig->jbfrcfg.cfg2;
			eaf_Config[i].JBFR_CFG3 = pEafConfig->jbfrcfg.cfg3;
			eaf_Config[i].JBFR_CFG4 = pEafConfig->jbfrcfg.cfg4;
			eaf_Config[i].JBFR_CFG5 = pEafConfig->jbfrcfg.cfg5;
			eaf_Config[i].JBFR_SIZE = pEafConfig->jbfrcfg.size;
			eaf_Config[i].JBFR_CFG6 = pEafConfig->jbfrcfg.cfg6;
			eaf_Config[i].JBFR_CFG7 = pEafConfig->jbfrcfg.cfg7;
			eaf_Config[i].JBFR_CFG8 = pEafConfig->jbfrcfg.cfg8;
		}

		if(pEafConfig->MAIN_CFG0.eaf_srz_en != 0)
		{
//			if(pEafConfig->srz6Cfg.crop.w > pEafConfig->srz6Cfg.inout_size.in_w)
//				eaf_Config[i].SRZ6_CONTROL = 0x13;
//			else
//				eaf_Config[i].SRZ6_CONTROL = 0x03;

			eaf_Config[i].SRZ6_CONTROL = pEafConfig->srz6Cfg.ctrl;
			pEafConfig->srz6Cfg.crop.floatX =((pEafConfig->srz6Cfg.crop.floatX >>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));		
			pEafConfig->srz6Cfg.crop.floatY =((pEafConfig->srz6Cfg.crop.floatY >>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));
			eaf_Config[i].SRZ6_IN_IMG = (pEafConfig->srz6Cfg.inout_size.in_h << 16)|(pEafConfig->srz6Cfg.inout_size.in_w << 0);	
			eaf_Config[i].SRZ6_OUT_IMG = (pEafConfig->srz6Cfg.inout_size.out_h << 16)|(pEafConfig->srz6Cfg.inout_size.out_w << 0);	
			eaf_Config[i].SRZ6_HORI_STEP = pEafConfig->srz6Cfg.h_step;//(( pEafConfig->srz6Cfg.crop.w-1)*32768+((pEafConfig->srz6Cfg.inout_size.out_w-1)>>1) )/(pEafConfig->srz6Cfg.inout_size.out_w-1);
			eaf_Config[i].SRZ6_VERT_STEP = pEafConfig->srz6Cfg.v_step;//(( pEafConfig->srz6Cfg.crop.h-1)*32768+((pEafConfig->srz6Cfg.inout_size.out_h-1)>>1) )/(pEafConfig->srz6Cfg.inout_size.out_h-1);
			eaf_Config[i].SRZ6_HORI_INT_OFST = pEafConfig->srz6Cfg.crop.x;
			eaf_Config[i].SRZ6_HORI_SUB_OFST = pEafConfig->srz6Cfg.crop.floatX;	
			eaf_Config[i].SRZ6_VERT_INT_OFST = pEafConfig->srz6Cfg.crop.y;	
			eaf_Config[i].SRZ6_VERT_SUB_OFST = pEafConfig->srz6Cfg.crop.floatY;	
			
		}
		
		if (pEafConfig->MAIN_CFG1.eafo_en == true)
		{
			eaf_Config[i].EAFO_BASE_ADDR = pEafConfig->EAFO.u4BufPA;			
			eaf_Config[i].EAFO_OFST_ADDR = pEafConfig->EAFO.u4Offset;		
			eaf_Config[i].EAFO_XSIZE = pEafConfig->EAFO.u4ImgWidth;			
			eaf_Config[i].EAFO_YSIZE = pEafConfig->EAFO.u4ImgHeight;			
			eaf_Config[i].EAFO_STRIDE = pEafConfig->EAFO.u4Stride;		
			eaf_Config[i].EAFO_CROP = (pEafConfig->EAFO.u4XOffset << 16)|(pEafConfig->EAFO.u4YOffset << 0);			
		}
		
		if (pEafConfig->MAIN_CFG1.eafi_en == true)
		{
			eaf_Config[i].EAFI_BASE_ADDR = pEafConfig->EAFI.u4BufPA;		
			eaf_Config[i].EAFI_OFST_ADDR = pEafConfig->EAFI.u4Offset;		
			eaf_Config[i].EAFI_XSIZE = pEafConfig->EAFI.u4ImgWidth;			
			eaf_Config[i].EAFI_YSIZE = pEafConfig->EAFI.u4ImgHeight;			
			eaf_Config[i].EAFI_STRIDE = pEafConfig->EAFI.u4Stride;		
		}		

		if (pEafConfig->MAIN_CFG1.eaf2i_en == true)
		{
			eaf_Config[i].EAF2I_BASE_ADDR = pEafConfig->EAF2I.u4BufPA;	
			eaf_Config[i].EAF2I_OFST_ADDR = pEafConfig->EAF2I.u4Offset;	
			eaf_Config[i].EAF2I_XSIZE = pEafConfig->EAF2I.u4ImgWidth;		
			eaf_Config[i].EAF2I_YSIZE = pEafConfig->EAF2I.u4ImgHeight;		
			eaf_Config[i].EAF2I_STRIDE = pEafConfig->EAF2I.u4Stride; 	
		}		
		
		if (pEafConfig->MAIN_CFG1.eaf3i_en == true)
		{
			eaf_Config[i].EAF3I_BASE_ADDR = pEafConfig->EAF3I.u4BufPA;	
			eaf_Config[i].EAF3I_OFST_ADDR = pEafConfig->EAF3I.u4Offset;	
			eaf_Config[i].EAF3I_XSIZE = pEafConfig->EAF3I.u4ImgWidth;		
			eaf_Config[i].EAF3I_YSIZE = pEafConfig->EAF3I.u4ImgHeight;		
			eaf_Config[i].EAF3I_STRIDE = pEafConfig->EAF3I.u4Stride;	
		}		
		
		if (pEafConfig->MAIN_CFG1.eaf4i_en == true)
		{
			eaf_Config[i].EAF4I_BASE_ADDR = pEafConfig->EAF4I.u4BufPA;	
			eaf_Config[i].EAF4I_OFST_ADDR = pEafConfig->EAF4I.u4Offset;	
			eaf_Config[i].EAF4I_XSIZE = pEafConfig->EAF4I.u4ImgWidth;		
			eaf_Config[i].EAF4I_YSIZE = pEafConfig->EAF4I.u4ImgHeight;		
			eaf_Config[i].EAF4I_STRIDE = pEafConfig->EAF4I.u4Stride;	
		}		
		
		if (pEafConfig->MAIN_CFG1.eaf5i_en == true)
		{
			eaf_Config[i].EAF5I_BASE_ADDR = pEafConfig->EAF5I.u4BufPA;	
			eaf_Config[i].EAF5I_OFST_ADDR = pEafConfig->EAF5I.u4Offset;	
			eaf_Config[i].EAF5I_XSIZE = pEafConfig->EAF5I.u4ImgWidth;		
			eaf_Config[i].EAF5I_YSIZE = pEafConfig->EAF5I.u4ImgHeight;		
			eaf_Config[i].EAF5I_STRIDE = pEafConfig->EAF5I.u4Stride; 	
		}		

		if (pEafConfig->MAIN_CFG1.eaf6i_en == true)
		{
			eaf_Config[i].EAF6I_BASE_ADDR = pEafConfig->EAF6I.u4BufPA;	
			eaf_Config[i].EAF6I_OFST_ADDR = pEafConfig->EAF6I.u4Offset;	
			eaf_Config[i].EAF6I_XSIZE = pEafConfig->EAF6I.u4ImgWidth;		
			eaf_Config[i].EAF6I_YSIZE = pEafConfig->EAF6I.u4ImgHeight;		
			eaf_Config[i].EAF6I_STRIDE = pEafConfig->EAF6I.u4Stride;
		}				
    }

	#ifdef FAKE_TILE_CALCULATION
	// test code will calculate result
	#else
	if ((pEafConfig->mode == 4)||(pEafConfig->mode == 1))
	{
		// calculate tile part

        char* pWorkingBuffer =  NULL;
        MUINT32 tpipeWorkingSize = tpipe_main_query_platform_working_buffer_size(8192);
        pWorkingBuffer = new char[tpipeWorkingSize];

		pTpipeCfgInfo = &gTileDesc;
		
		pTpipeCfgInfo->top.eaf_ctl_extension_en = 0;
		pTpipeCfgInfo->top.eaf_en = pEafConfig->MAIN_CFG0.eaf_en;
		pTpipeCfgInfo->top.eafi_mask_en = pEafConfig->MAIN_CFG1.eafi_mask_en;
		pTpipeCfgInfo->top.eafi_cur_y_en = pEafConfig->MAIN_CFG1.eafi_cur_y_en;
		pTpipeCfgInfo->top.eafi_cur_uv_en = pEafConfig->MAIN_CFG1.eafi_cur_uv_en;
		pTpipeCfgInfo->top.eafi_pre_y_en = pEafConfig->MAIN_CFG1.eafi_pre_y_en;
		pTpipeCfgInfo->top.eafi_pre_uv_en = pEafConfig->MAIN_CFG1.eafi_pre_uv_en;
		pTpipeCfgInfo->top.eafi_depth_en = pEafConfig->MAIN_CFG1.eafi_dep_en;
		pTpipeCfgInfo->top.eafi_lkh_wmap_en = pEafConfig->MAIN_CFG1.eafi_lkh_wmap_en;
		pTpipeCfgInfo->top.eafi_lkh_emap_en = pEafConfig->MAIN_CFG1.eafi_lkh_emap_en;
		pTpipeCfgInfo->top.eafo_fout_en = pEafConfig->MAIN_CFG1.eafo_fout_en;
		pTpipeCfgInfo->top.srz6_en = pEafConfig->MAIN_CFG0.eaf_srz_en;
						
		pTpipeCfgInfo->eaf.eaf_phase = 4-1;
		pTpipeCfgInfo->eaf.jbfr_ref_sel = pEafConfig->MAIN_CFG0.jbfr_ref_sel;
		
		pTpipeCfgInfo->eafi.mask_xsize = pEafConfig->EAFI_MASK.u4ImgWidth;
		pTpipeCfgInfo->eafi.mask_ysize = pEafConfig->EAFI_MASK.u4ImgHeight;
		pTpipeCfgInfo->eafi.mask_stride = pEafConfig->EAFI_MASK.u4Stride;
		pTpipeCfgInfo->eafi.cur_y_xsize = pEafConfig->EAFI_CUR_Y.u4ImgWidth;
		pTpipeCfgInfo->eafi.cur_y_ysize = pEafConfig->EAFI_CUR_Y.u4ImgHeight;
		pTpipeCfgInfo->eafi.cur_y_stride = pEafConfig->EAFI_CUR_Y.u4Stride;
		pTpipeCfgInfo->eafi.cur_uv_stride = pEafConfig->EAFI_CUR_UV.u4Stride;
		pTpipeCfgInfo->eafi.pre_y_stride = pEafConfig->EAFI_PRE_Y.u4Stride;
		pTpipeCfgInfo->eafi.pre_uv_stride = pEafConfig->EAFI_PRE_UV.u4Stride;
		pTpipeCfgInfo->eafi.lkh_wmap_stride = pEafConfig->EAFI_LKH_WMAP.u4Stride;
		pTpipeCfgInfo->eafi.lkh_emap_stride = pEafConfig->EAFI_LKH_EMAP.u4Stride;
		pTpipeCfgInfo->eafi.depth_stride = pEafConfig->EAFI_DEP.u4Stride;
		
		pTpipeCfgInfo->eafo.fout_stride = pEafConfig->EAFO_FOUT.u4Stride;
		pTpipeCfgInfo->eafo.fout_xoffset = pEafConfig->EAFO_FOUT.u4XOffset;
		pTpipeCfgInfo->eafo.fout_yoffset = pEafConfig->EAFO_FOUT.u4YOffset;
		pTpipeCfgInfo->eafo.fout_xsize = pEafConfig->EAFO.u4ImgWidth;
		pTpipeCfgInfo->eafo.fout_ysize = pEafConfig->EAFO.u4ImgHeight;

		pTpipeCfgInfo->srz6.srz_input_crop_width = pEafConfig->srz6Cfg.inout_size.in_w;
		pTpipeCfgInfo->srz6.srz_input_crop_height = pEafConfig->srz6Cfg.inout_size.in_h;
		pTpipeCfgInfo->srz6.srz_output_width = pEafConfig->srz6Cfg.inout_size.out_w;
		pTpipeCfgInfo->srz6.srz_output_height = pEafConfig->srz6Cfg.inout_size.out_h;
		pTpipeCfgInfo->srz6.srz_luma_horizontal_integer_offset = pEafConfig->srz6Cfg.crop.x;
		pTpipeCfgInfo->srz6.srz_luma_horizontal_subpixel_offset = pEafConfig->srz6Cfg.crop.floatX;
		pTpipeCfgInfo->srz6.srz_luma_vertical_integer_offset = pEafConfig->srz6Cfg.crop.y;
		pTpipeCfgInfo->srz6.srz_luma_vertical_subpixel_offset = pEafConfig->srz6Cfg.crop.floatY;
		pTpipeCfgInfo->srz6.srz_horizontal_coeff_step = pEafConfig->srz6Cfg.h_step;
		pTpipeCfgInfo->srz6.srz_vertical_coeff_step = pEafConfig->srz6Cfg.v_step;
		
		pTpipeCfgInfo->sw.log_en = 1;	
		pTpipeCfgInfo->sw.src_width_eaf = pEafConfig->EAFI.u4ImgWidth;
		pTpipeCfgInfo->sw.src_height_eaf = pEafConfig->EAFI.u4ImgHeight;
		pTpipeCfgInfo->sw.tpipe_irq_mode = 2;	
		pTpipeCfgInfo->sw.tpipe_width_eaf = 512;
		pTpipeCfgInfo->sw.tpipe_height_eaf = 16383;
		pTpipeCfgInfo->sw.tpipe_sel_mode = 0x1000;

		tpipe_main_platform(pTpipeCfgInfo, &tpipeDesc, gTileBuffer.virtAddr, gTileBuffer.size,NULL, NULL, NULL);

		delete pWorkingBuffer;
		pWorkingBuffer = NULL;

		m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &tpipeDesc);

    }
	#endif


    eaf_Request.m_ReqNum = num;
    eaf_Request.m_pEafConfig = eaf_Config;

	LOG_DBG("ioctl(this->m_Fd,EAF_ENQUE_REQ \n");

    Ret = ioctl(this->m_Fd,EAF_ENQUE_REQ,&eaf_Request);
    if(Ret < 0)
    {
        LOG_ERR("EAF_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }


    return MTRUE;


}

MBOOL EafDrvImp::dequeEAF(vector<NSCam::NSIoPipe::EAFConfig>& EafConfigParam)
{
    MINT32 Ret;
    NSCam::NSIoPipe::EAFConfig EafConfig;
    EAF_Request eaf_Request;
    EAF_Config eaf_Config[_SUPPORT_MAX_EAF_FRAME_REQUEST_];
    eaf_Request.m_pEafConfig = eaf_Config;

    Ret = ioctl(this->m_Fd,EAF_DEQUE_REQ,&eaf_Request);
    if(Ret < 0)
    {
        LOG_ERR("EAF_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    LOG_DBG("dequeEAF num:%d\n", eaf_Request.m_ReqNum);

    for (unsigned int i=0; i< eaf_Request.m_ReqNum; i++)
    {
        EafConfigParam.push_back(EafConfig);

    }

    return MTRUE;
}


MUINT32 EafDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = EAF_REG_RANGE;
    LOG_DBG("+,Eaf_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->EafRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }


    if(this->m_regRWMode==EAF_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pEafHwRegAddr[(Addr>>2)];
    }
    else{
        EAF_REG_IO_STRUCT EafRegIo;
        EAF_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        EafRegIo.pData = (EAF_REG_STRUCT*)&RegIo;
        EafRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, EAF_READ_REGISTER, &EafRegIo);
        if(Ret < 0)
        {
            LOG_ERR("EAF_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Eaf_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL EafDrvImp::readRegs(EAF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = EAF_REG_RANGE;
    android::Mutex::Autolock lock(this->EafRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == EAF_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pEafHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        EAF_REG_IO_STRUCT EafRegIo;
        //pRegIo->module = this->m_HWmodule;
        EafRegIo.pData = (EAF_REG_STRUCT*)pRegIo;
        EafRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, EAF_READ_REGISTER, &EafRegIo);
        if(Ret < 0)
        {
            LOG_ERR("EAF_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Eaf_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}


MBOOL EafDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = EAF_REG_RANGE;
    LOG_DBG("Eaf_write:m_regRWMode(0x%x),(0x%x,0x%x)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->EafRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case EAF_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pEafHwRegAddr[(Addr>>2)] = Data;
            break;
        case EAF_DRV_RW_IOCTL:
            EAF_REG_IO_STRUCT EafRegIo;
            EAF_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            EafRegIo.pData = (EAF_REG_STRUCT*)&RegIo;
            EafRegIo.Count = 1;
            ret = ioctl(this->m_Fd, EAF_WRITE_REGISTER, &EafRegIo);
            if(ret < 0){
                LOG_ERR("EAF_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case EAF_DRV_R_ONLY:
            LOG_ERR("EAF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->EafRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL EafDrvImp::writeRegs(EAF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = EAF_REG_RANGE;
    android::Mutex::Autolock lock(this->EafRegMutex);
    //(void)caller;
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case EAF_DRV_RW_IOCTL:
            EAF_REG_IO_STRUCT EafRegIo;
            //pRegIo->module = this->m_HWmodule;
            EafRegIo.pData = (EAF_REG_STRUCT*)pRegIo;
            EafRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, EAF_WRITE_REGISTER, &EafRegIo);
            if(Ret < 0){
                LOG_ERR("EAF_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case EAF_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pEafHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case EAF_DRV_R_ONLY:
            LOG_ERR("EAF Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Eaf_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 EafDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL EafDrvImp::setRWMode(EAF_DRV_RW_MODE rwMode)
{
    if(rwMode > EAF_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}


