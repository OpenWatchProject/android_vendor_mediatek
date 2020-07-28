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
#define LOG_TAG "CcuDrv"

#include <stdio.h>
#include <stdlib.h>

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <sys/mman.h>
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

/* kernel files */
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "ccu_ext_interface/ccu_mailbox_extif.h"
#include "ccu_drv.h"
#include "ccu_platform_def.h"

#include <mtkcam/def/PriorityDefs.h>
#include <string.h>
#include <ccu_udrv_stddef.h>
#include <ccu_udrv.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "ccu_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(ccu_drv);

#define EVEREST_EP_MARK_CODE
#define CCU_INIT_TIMEOUT_SPEC 50000

class CcuDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    CcuDbgTimer(char const*const pszTitle)
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

MINT32              CcuDrv::m_Fd = -1;


volatile MINT32     CcuDrvImp::m_UserCnt = 0;
CCU_DRV_RW_MODE     CcuDrvImp::m_regRWMode = CCU_DRV_R_ONLY;
android::Mutex      CcuDrvImp::m_CcuInitMutex;

char                CcuDrvImp::m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

static void printCcuLog(const char *logStr, MBOOL mustPrint=MFALSE)
{
    //Log parsing procedure happens only when log level > 4;
    if((!mustPrint) && (!ccu_drv_DbgLogEnable_DEBUG))
    {
        return;
    }
    #define LOG_LEN_LIMIT 800
    size_t length = 0;
    const char *logPtr = logStr;
    static char logBuf[LOG_LEN_LIMIT + 1];
    int logSegCnt = 0;
    int copyLen = 0;

    length = strlen(logStr);
    logSegCnt = length / LOG_LEN_LIMIT + 1;

    //LOG_DBG("logging ccu, length: %ld\n", length);
    //LOG_DBG("logging ccu, logSegCnt: %ld\n", length);
    if(mustPrint)
        LOG_WRN("===== CCU LOG DUMP START =====\n");
    else
        LOG_DBG("===== CCU LOG DUMP START =====\n");
    for(int i=0 ; i<logSegCnt; i++)
    {
        if(i == logSegCnt-1)
        {
            copyLen = length % LOG_LEN_LIMIT;
        }
        else
        {
            copyLen = LOG_LEN_LIMIT;
        }

        //LOG_DBG("logging ccu[%d], copyLen: %d\n", i, copyLen);

        if(copyLen > 0)
        {
            //LOG_DBG("logging ccu[%d], logPtr: %p\n", logPtr);
            memcpy(logBuf, logPtr, copyLen);
            logBuf[copyLen] = '\0';
            logPtr += copyLen;
            if(mustPrint)
                LOG_WRN("\n----- DUMP SEG[%d] -----\n%s\n", i, logBuf, i);
            else
                LOG_DBG("\n----- DUMP SEG[%d] -----\n%s\n", i, logBuf, i);
        }
    }
    if(mustPrint)
        LOG_WRN("===== CCU LOG DUMP END =====\n");
    else
        LOG_DBG("===== CCU LOG DUMP END =====\n");
    #undef LOG_LEN_LIMIT
}


CcuDrvImp::CcuDrvImp()
{
    DBG_LOG_CONFIG(drv, ccu_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pCcuHwRegAddr = NULL;
    pCcuMem = NULL;
}

static MINT8       *g_pLogBuf[MAX_LOG_BUF_NUM];
static CcuDrvImp   *p_gCcuDrvObj[MAX_CCU_HW_MODULE];
CcuDrv* CcuDrvImp::createInstance(CCU_HW_MODULE module)
{
    static CcuDrvImp    gCcuDrvObj[MAX_CCU_HW_MODULE];
    LOG_DBG("+,module(%d)",module);
    gCcuDrvObj[module].m_HWmodule = module;

    for(MUINT32 i=0; i<MAX_CCU_HW_MODULE; i++)
    {
        p_gCcuDrvObj[i] = &gCcuDrvObj[i];
    }

    return (CcuDrv*)&gCcuDrvObj[module];
}

#define _USER_PRINTF_CCU_LOG_

MBOOL CcuDrvImp::CloseCcuKdrv()
{
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=CCU_DRV_R_ONLY;
    }

    return MTRUE;
}

MBOOL CcuDrvImp::OpenCcuKdrv()
{
    if(this->m_Fd >= 0)
    {
        //if kdrv is already opened, return
        return MTRUE;
    }

    this->m_Fd = open(CCU_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("CCU kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(CCU_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("CCU kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            return MFALSE;
        }
        else
            this->m_regRWMode=CCU_DRV_R_ONLY;
    }

    return MTRUE;
}

MBOOL CcuDrvImp::setCcuPower(int powerCtrlType)
{
    LOG_DBG("+:%s\n", __FUNCTION__);
    m_power.bON = powerCtrlType;
    m_power.freq = 500;
    m_power.power = 15;

    if(ioctl(this->m_Fd, CCU_IOCTL_SET_POWER, &m_power) < 0)
    {
        LOG_ERR("CCU_IOCTL_SET_POWER_T: %d, bON(%d)\n", this->m_HWmodule, m_power.bON);
        LOG_WRN("============ CCU FAIL LOG DUMP: LOGBUF[0] =============\n");
        printCcuLog((const char*)g_pLogBuf[0], MTRUE);
        LOG_WRN("============ CCU FAIL LOG DUMP: LOGBUF[1] =============\n");
        printCcuLog((const char*)g_pLogBuf[1], MTRUE);
        DumpSramLog();
        PrintReg();
        return MFALSE;
    }
    LOG_DBG_MUST("%s: setpower(%d) cmd done.\n", __FUNCTION__, m_power.bON);

    LOG_DBG("-:%s\n", __FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::setCcuRun()
{
    LOG_DBG("+:CCU_IOCTL_SET_RUN\n");
    if(ioctl(this->m_Fd, CCU_IOCTL_SET_RUN) < 0){
        LOG_ERR("CCU_IOCTL_SET_RUN: %d\n", this->m_HWmodule);
        return MFALSE;
    }
    LOG_DBG("-:CCU_IOCTL_SET_RUN\n");

    return MTRUE;
}

MBOOL CcuDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 strLen;
    MBOOL isPowerOn = MFALSE;

    DBG_LOG_CONFIG(drv, ccu_drv);
    //
    android::Mutex::Autolock lock(this->m_CcuInitMutex);
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).\n", this->m_UserCnt,userName);
    //

    if(this->m_UserCnt < 0)
    {
        LOG_DBG_MUST("m_UserCnt : %d is nagtive\n",this->m_UserCnt);
        return MFALSE;
    }

    strLen = strlen(userName);
    if((strLen<1)||(strLen>=MAX_USER_NAME_SIZE))
    {
        if(strLen<1)
            LOG_ERR("[Error]Plz add userName if you want to use ccu driver\n");
        else if(strLen>=MAX_USER_NAME_SIZE)
            LOG_ERR("[Error]user's userName length(%d) exceed the default length(%d)\n",strLen,MAX_USER_NAME_SIZE);
        else
            LOG_ERR("[Error]coding error, please check judgement condition\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt > 0)
    {
        if(this->m_UserCnt < MAX_USER_NUMBER){
            strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE-1);
            android_atomic_inc(&this->m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d,UserName:%s", this->m_UserCnt,userName);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE-1);
    android_atomic_inc(&this->m_UserCnt);
    
    //allocate ccu buffers
    LOG_DBG("+:pCcuMem\n");
    this->pCcuMem = (CcuMemImp*)CcuMemImp::createInstance();
    if ( NULL == this->pCcuMem ) {
        LOG_ERR("createInstance pCcuMem return %p", pCcuMem);
        return false;
    }
    this->pCcuMem->init("CCU_UDRV");
    LOG_DBG("-:pCcuMem\n");

    this->allocateBuffers();

    // Open ccu device
    if (!(this->OpenCcuKdrv()))
    {
        Result = MFALSE;
        goto EXIT;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
        LOG_DBG("CCU kernel open ok, fd:%d.", this->m_Fd);
        // fd opened only once at the very 1st init

        LOG_DBG("MMAP CCU HW Addrs.");
        switch(this->m_HWmodule)
        {
            case CCU_A:
            case CCU_CAMSYS:
				/* MUST: CCU_HW_BASE should be 4K alignment, otherwise mmap got error(22):invalid argument!! */
                LOG_DBG_MUST("CCU_HW_BASE: %x", CCU_HW_BASE);
                p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr 		= (MUINT32 *) mmap(0, PAGE_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_HW_BASE - CCU_HW_OFFSET);
                LOG_DBG_MUST("mmap CCU_A done:%p\n",      p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr);
                p_gCcuDrvObj[CCU_CAMSYS]->m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_CAMSYS_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_CAMSYS_BASE);
                LOG_DBG_MUST("mmap CCU_CAMSYS done:%p\n", p_gCcuDrvObj[CCU_CAMSYS]->m_pCcuHwRegAddr);
                p_gCcuDrvObj[CCU_PMEM]->m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_PMEM_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_PMEM_BASE);
                LOG_DBG_MUST("mmap CCU_PMEM done:%p\n",   p_gCcuDrvObj[CCU_PMEM]->m_pCcuHwRegAddr);
                p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_DMEM_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_DMEM_BASE);
                LOG_DBG_MUST("mmap CCU_DMEM done:%p\n",   p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr);
                p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr = p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr + (CCU_HW_OFFSET/4);

                break;
            default:
                LOG_ERR("#############\n");
                LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                LOG_ERR("#############\n");
                goto EXIT;
                break;
        }

        if(p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr == MAP_FAILED)
        {
            LOG_ERR("CCU mmap fail , errno(%d):%s",errno, strerror(errno));
			LOG_ERR("(%x)(%x)\n",PAGE_SIZE,CCU_HW_BASE);
            Result = MFALSE;
            goto EXIT;
        }

        this->m_regRWMode = CCU_DRV_RW_MMAP;
    }

    //
    CcuAeeMgrDl::CcuAeeMgrDlInit();

	//power on
    if(!this->setCcuPower(1))
    {
        Result = MFALSE;
        goto EXIT;
    }
    else
        isPowerOn = MTRUE;

	//load binary here
	LOG_DBG_MUST("+:loadCCUBin\n");
	if ( !this->loadCCUBin() ) {
		LOG_ERR("load binary file fail\n");
        Result = MFALSE;
        goto EXIT;
	}
    LOG_DBG_MUST("-:loadCCUBin\n");

#if defined( _USER_PRINTF_CCU_LOG_ )
    LOG_DBG_MUST("+:_USER_PRINTF_CCU_LOG_ create thread\n");
    this->createThread();
    LOG_DBG_MUST("-:_USER_PRINTF_CCU_LOG_ create thread\n");
#endif

    LOG_DBG_MUST("+:setCcuRun\n");
    if(!this->setCcuRun())
    {
        PrintReg();
        Result = MFALSE;
        goto EXIT;
    }
    LOG_DBG_MUST("-:setCcuRun\n");

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if (isPowerOn)    // If power is on.
        {
            this->OnlypowerOff();
        }
        this->RealUninit();
    }


    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pCcuHwRegAddr(0x%x)\n", Result, this->m_UserCnt, p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr);
    return Result;
}

MBOOL CcuDrvImp::restart()
{
    MBOOL Result = MTRUE;

    LOG_DBG("+:%s\n",__FUNCTION__);

    android::Mutex::Autolock lock(this->m_CcuInitMutex);

    //CCU kdrv may had been closed by checkSensorSupport(), check if need to re-open
    if (!(this->OpenCcuKdrv()))
    {
        Result = MFALSE;
        goto EXIT;
    }

    if(!this->setCcuPower(2))
    {
        Result = MFALSE;
        goto EXIT;
    }

    if(!this->setCcuRun())
    {
        Result = MFALSE;
        goto EXIT;
    }

EXIT:
    if (!Result)    // If some init step goes wrong.
    {
        this->RealUninit();
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pCcuHwRegAddr(0x%x)\n", Result, this->m_UserCnt, p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr);
    LOG_DBG("-:%s\n",__FUNCTION__);

    return Result;
}

MBOOL CcuDrvImp::allocateBuffers()
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    //allocate log buffers
    for (int i=0; i<MAX_LOG_BUF_NUM; i++ )
    {
        m_power.workBuf.sz_log[i] = SIZE_1MB;

    	this->pCcuMem->ccu_malloc( this->pCcuMem->gIonDevFD, m_power.workBuf.sz_log[i], &m_power.workBuf.fd_log[i], &m_power.workBuf.va_log[i]);
    	if ( NULL == m_power.workBuf.va_log[i] )
        {
    		LOG_ERR("ccu_malloc %p", m_power.workBuf.va_log[i]);
    		return false;
    	}

        g_pLogBuf[i] = (MINT8*)m_power.workBuf.va_log[i];
        memset((char*)g_pLogBuf[i],0x00,SIZE_1MB);
        sprintf((char*)g_pLogBuf[i],"g_pLogBuf[%d]\n",i);
    	//this->pCcuMem->mmapMVA_m4u(m_power.workBuf.va_log[i], &m_power.workBuf.mva_log[i], m_power.workBuf.sz_log[i]);
        this->pCcuMem->mmapMVA( m_power.workBuf.fd_log[i], &m_logBufIonHandle[i], &m_power.workBuf.mva_log[i]);
        if ( 0 == m_power.workBuf.mva_log[i] ) {
            LOG_ERR("mmapMVA(m_power.workBuf.mva_log[%d]) Error: %x", i , m_power.workBuf.mva_log[i]);
            return false;
        }

        LOG_DBG("CCU_LOG(%d) buffer:va(%p),mva(%x),sz(%d)\n", i, m_power.workBuf.va_log[i],m_power.workBuf.mva_log[i], m_power.workBuf.sz_log[i]);
    }

    LOG_DBG("-:%s\n",__FUNCTION__);

    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL CcuDrvImp::shutdown()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

	//Send MSG_TO_CCU_SHUTDOWN command
    ccu_cmd_st cmd_t = { {MSG_TO_CCU_SHUTDOWN, NULL, NULL, CCU_CAM_TG_NONE}, CCU_ENG_STATUS_INVALID };
    	ccu_cmd_st cmd_r;

      	if ( !this->sendCmd( &cmd_t, &cmd_r, true ) ) {
    		LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
    		return MFALSE;
    }

    LOG_DBG_MUST("-:%s\n",__FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::powerOff()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    //power off
    if(!this->setCcuPower(0))
    {
        return MFALSE;
    }
    LOG_DBG_MUST("-:%s\n",__FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::OnlypowerOff()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    //power off
    if(!this->setCcuPower(4))
    {
        return MFALSE;
    }
    LOG_DBG_MUST("-:%s\n",__FUNCTION__);
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL CcuDrvImp::pause()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    // 1.send MSG_TO_CCU_SHUTDOWN command
    ccu_cmd_st cmd_t = { {MSG_TO_CCU_SHUTDOWN, 0, 0}, CCU_ENG_STATUS_INVALID };
    ccu_cmd_st cmd_r;
    if ( !this->sendCmd( &cmd_t, &cmd_r, true ) ) {
        LOG_ERR("cmd(%d) fail \n",cmd_t.task.msg_id);
        return MFALSE;
    }
    LOG_DBG_MUST("%s: shutdown cmd done.\n",__FUNCTION__);

    // 2. power off without CG
    if(!this->setCcuPower(3))
    {
        return MFALSE;
    }

    LOG_DBG_MUST("-:%s\n\n",__FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::RealUninit(const char* userName)
{
    MBOOL Result = MTRUE;
    //
    LOG_INF_MUST("+,m_UserCnt(%d),curUser(%s)\n", this->m_UserCnt, userName);
    //

    if(this->m_UserCnt <= 0)
    {
        LOG_DBG_MUST("m_UserCnt : %d is nagtive or zero\n",this->m_UserCnt);
        return MFALSE;
    }

#if defined( _USER_PRINTF_CCU_LOG_ )
    LOG_DBG_MUST("+:release thread\n");
    if(this->m_taskWorkerThreadReady == MTRUE)
    {
        this->destroyThread();
    }
    LOG_DBG_MUST("-:release thread\n");
#endif
    //
    UnMapHwRegs();

    //
    LOG_DBG_MUST("+:release buffer\n");
    for (int i=0; i<MAX_LOG_BUF_NUM; i++ )
    {
        // if ( false == this->pCcuMem->munmapMVA_m4u( m_power.workBuf.va_log[i], m_power.workBuf.mva_log[i], m_power.workBuf.sz_log[i] ) ) {
        //     LOG_ERR("munmapMVA_m4u fail:m4u\n");
        // }

        //
        if ( false == this->pCcuMem->munmapMVA(m_logBufIonHandle[i]))
        {
            LOG_ERR("munmapMVA fail:ion_handle_in_data\n");
        }
        if ( false == this->pCcuMem->ccu_free( this->pCcuMem->gIonDevFD, m_power.workBuf.sz_log[i], m_power.workBuf.fd_log[i], m_power.workBuf.va_log[i]) ) {
        	LOG_ERR("ccu_free_buffer fail: %d\n", i);
        }

        g_pLogBuf[i] = NULL;
    }
    LOG_DBG_MUST("-:release buffer\n");

	if ( this->pCcuMem )
    {
	    this->pCcuMem->uninit("CCU_UDRV");
	    this->pCcuMem->destroyInstance();
	}

    CcuAeeMgrDl::CcuAeeMgrDlUninit();

    //
    this->CloseCcuKdrv();

    // More than one user
	android_atomic_dec(&this->m_UserCnt);
    //
EXIT:

    if(this->m_UserCnt!= 0){
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=8) {
            if(this->m_UserCnt > (MINT32)i) {
                LOG_INF("current user[%d]: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
                ,i,this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]  \
                ,this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
            }
        }
    }

    LOG_INF_MUST("CcuDrv uinitial done gracefully\n");

    LOG_INF_MUST(" - X. ret: %d. m_UserCnt: %d\n", Result, this->m_UserCnt);

    return Result;
}

MBOOL CcuDrvImp::UnMapHwRegs()
{
    for(MUINT32 i=0;i<MAX_CCU_HW_MODULE;i++)
    {
        if(p_gCcuDrvObj[i]->m_pCcuHwRegAddr != MAP_FAILED && p_gCcuDrvObj[i]->m_pCcuHwRegAddr != NULL)
        {
            int ret = 0;
            switch(i){
                case CCU_A:
                    LOG_DBG_MUST("Uinit_CCU_A munmap: %p\n", p_gCcuDrvObj[i]->m_pCcuHwRegAddr);
                    p_gCcuDrvObj[i]->m_pCcuHwRegAddr = p_gCcuDrvObj[i]->m_pCcuHwRegAddr - (CCU_HW_OFFSET/4);
                    ret = munmap(p_gCcuDrvObj[i]->m_pCcuHwRegAddr, PAGE_SIZE);
                    break;
                case CCU_CAMSYS:
                    LOG_DBG_MUST("Uinit_CCU_CAMSYS munmap: %p\n", p_gCcuDrvObj[i]->m_pCcuHwRegAddr);
                    ret = munmap(p_gCcuDrvObj[i]->m_pCcuHwRegAddr, PAGE_SIZE);
                    break;
                case CCU_PMEM:
                    LOG_DBG_MUST("Uinit_CCU_PMEM munmap: %p\n", p_gCcuDrvObj[i]->m_pCcuHwRegAddr);
                    ret = munmap(p_gCcuDrvObj[i]->m_pCcuHwRegAddr, CCU_PMEM_SIZE);
                    break;
                case CCU_DMEM:
                    LOG_DBG_MUST("Uinit_CCU_DMEM munmap: %p\n", p_gCcuDrvObj[i]->m_pCcuHwRegAddr);
                    ret = munmap(p_gCcuDrvObj[i]->m_pCcuHwRegAddr, CCU_DMEM_SIZE);
                    break;
                default:
                    LOG_ERR("this hw module(%d) is unsupported\n",i);
                    break;
            }

            if (ret < 0)
            {
                LOG_ERR("munmap fail: %p\n", p_gCcuDrvObj[i]->m_pCcuHwRegAddr);
                return MFALSE;
            }

            p_gCcuDrvObj[i]->m_pCcuHwRegAddr = NULL;
        }
    }

    return MTRUE;
}

MBOOL CcuDrvImp::uninit(const char* userName)
{
    android::Mutex::Autolock lock(this->m_CcuInitMutex);

    return this->RealUninit(userName);
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no ccu device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL CcuDrvImp::waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    CCU_IRQ_CLEAR_ENUM OrgClr;
    CcuDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    CCU_WAIT_IRQ_STRUCT wait;

    LOG_DBG(" + E. hwmodule:0x%x,Status(0x%08x),Timeout(%d).\n",this->m_HWmodule, pWaitIrq->Status, pWaitIrq->Timeout);

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    switch(this->m_HWmodule){
        case CCU_A:     wait.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     wait.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(CCU_WAIT_IRQ_ST));

    LOG_DBG("CCU_IOCTL_WAIT_IRQ ioctl call, arg is CCU_WAIT_IRQ_STRUCT, size: %d\n", sizeof(CCU_WAIT_IRQ_STRUCT));

    Ta=dbgTmr.getUs();
    Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_IRQ,&wait);
    Tb=dbgTmr.getUs();

    memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(CCU_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("CCU(0x%x)_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n",this->m_HWmodule, Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL CcuDrvImp::clearIrq(CCU_CLEAR_IRQ_ST* pClearIrq)
{
    MINT32 Ret;
    CCU_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. hw module:0x%x,user(%d), Status(%d)\n",this->m_HWmodule,pClearIrq->UserKey, pClearIrq->Status);
    if(FD_CHK() == -1){
        return MFALSE;
    }
    switch(this->m_HWmodule){
        case CCU_A:     clear.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     clear.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }
    //
    memcpy(&clear.EventInfo,pClearIrq,sizeof(CCU_CLEAR_IRQ_ST));
    Ret = ioctl(this->m_Fd,CCU_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("CCU(0x%x)_CLEAR_IRQ fail(%d)\n",this->m_HWmodule,Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL CcuDrvImp::waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    CCU_IRQ_CLEAR_ENUM OrgClr;
    CcuDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    CCU_WAIT_IRQ_STRUCT wait;

    LOG_DBG(" + E. hwmodule:0x%x,Status(0x%08x),Timeout(%d).\n",this->m_HWmodule, pWaitIrq->Status, pWaitIrq->Timeout);

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    switch(this->m_HWmodule){
        case CCU_A:     wait.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     wait.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(CCU_WAIT_IRQ_ST));

    LOG_DBG("CCU_IOCTL_WAIT_IRQ ioctl call, arg is CCU_WAIT_IRQ_STRUCT, size: %d\n", sizeof(CCU_WAIT_IRQ_STRUCT));

    Ta=dbgTmr.getUs();
    Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_AF_IRQ,&wait);
    Tb=dbgTmr.getUs();

    memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(CCU_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("CCU(0x%x)_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n",this->m_HWmodule, Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL CcuDrvImp::clearAFIrq(CCU_CLEAR_IRQ_ST* pClearIrq)
{
    MINT32 Ret;
    CCU_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. hw module:0x%x,user(%d), Status(%d)\n",this->m_HWmodule,pClearIrq->UserKey, pClearIrq->Status);
    if(FD_CHK() == -1){
        return MFALSE;
    }
    switch(this->m_HWmodule){
        case CCU_A:     clear.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     clear.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }
    //
    memcpy(&clear.EventInfo,pClearIrq,sizeof(CCU_CLEAR_IRQ_ST));
    Ret = ioctl(this->m_Fd,CCU_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("CCU(0x%x)_CLEAR_IRQ fail(%d)\n",this->m_HWmodule,Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL CcuDrvImp::waitAFBIrq(CCU_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    CCU_IRQ_CLEAR_ENUM OrgClr;
    CcuDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    CCU_WAIT_IRQ_STRUCT wait;

    LOG_DBG(" + E. hwmodule:0x%x,Status(0x%08x),Timeout(%d).\n",this->m_HWmodule, pWaitIrq->Status, pWaitIrq->Timeout);

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    switch(this->m_HWmodule){
        case CCU_A:     wait.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     wait.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(CCU_WAIT_IRQ_ST));

    LOG_DBG("CCU_IOCTL_WAIT_IRQ ioctl call, arg is CCU_WAIT_IRQ_STRUCT, size: %d\n", sizeof(CCU_WAIT_IRQ_STRUCT));

    Ta=dbgTmr.getUs();
    Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_AFB_IRQ,&wait);
    Tb=dbgTmr.getUs();

    memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(CCU_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("CCU(0x%x)_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n",this->m_HWmodule, Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL CcuDrvImp::clearAFBIrq(CCU_CLEAR_IRQ_ST* pClearIrq)
{
    MINT32 Ret;
    CCU_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. hw module:0x%x,user(%d), Status(%d)\n",this->m_HWmodule,pClearIrq->UserKey, pClearIrq->Status);
    if(FD_CHK() == -1){
        return MFALSE;
    }
    switch(this->m_HWmodule){
        case CCU_A:     clear.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;
            break;
        case CCU_CAMSYS:     clear.Type = CCU_IRQ_TYPE_INT_CCU_B_ST;
            break;
        default:
            LOG_ERR("unsupported hw module:0x%x\n",this->m_HWmodule);
            return MFALSE;
            break;
    }
    //
    memcpy(&clear.EventInfo,pClearIrq,sizeof(CCU_CLEAR_IRQ_ST));
    Ret = ioctl(this->m_Fd,CCU_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("CCU(0x%x)_CLEAR_IRQ fail(%d)\n",this->m_HWmodule,Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL CcuDrvImp::registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    MINT32 Ret;
    LOG_DBG(" - E. hw module:0x%x,userkey(%d), name(%s)\n",this->m_HWmodule,pRegIrq->userKey, pRegIrq->userName);

    Ret = ioctl(this->m_Fd,CCU_REGISTER_IRQ_USER_KEY,pRegIrq);
    if(Ret < 0) {
        LOG_ERR("CCU_REGISTER_IRQ fail(%d). hw module:0x%x, userkey(%d), name(%s\n", Ret, this->m_HWmodule, pRegIrq->userKey, pRegIrq->userName);
        return MFALSE;
    }
    return MTRUE;
}

MUINT32 CcuDrvImp::readInfoReg(MUINT32 regNo)
{
    MINT32 Ret;

    Ret = ioctl(this->m_Fd,CCU_READ_REGISTER, (void *)(uint64_t)regNo);

    return Ret;
}

MBOOL CcuDrvImp::sendCmd( ccu_cmd_st *pCMD_T, ccu_cmd_st *pCMD_R, MBOOL bSync )
{
    //ccu_cmd_st cmd;
    //ccu_cmd_st cmd_get;

    //::memcpy(&cmd, pCMD, sizeof(ccu_cmd_st) );
    android::Mutex::Autolock lock(this->m_CcuSendCmdMutex);
    int ret = 0;

    if(this->m_UserCnt <= 0)
    {
        LOG_DBG_MUST("m_UserCnt : %d is nagtive or zero\n",this->m_UserCnt);
        return MFALSE;
    }

    LOG_INF("CCU_IOCTL_ENQUE_COMMAND: msg(%d), tg(%d), in(%p), out(%p), st(%d)\n",
                pCMD_T->task.msg_id, \
                pCMD_T->task.tg_info, \
                pCMD_T->task.in_data_ptr, \
                pCMD_T->task.out_data_ptr, \
                pCMD_T->status);

    if(ioctl(this->m_Fd, CCU_IOCTL_ENQUE_COMMAND, pCMD_T) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_ENQUE_COMMAND: msg(%d), tg(%d)\n", pCMD_T->task.msg_id, pCMD_T->task.tg_info);
        return MFALSE;
    }

	if ( bSync )
    {
		LOG_INF("CCU_IOCTL_DEQUE_COMMAND wait ack...\n");
        ret = ioctl(this->m_Fd, CCU_IOCTL_DEQUE_COMMAND, pCMD_R);
		if(ret < 0)
		{
		    LOG_ERR("CCU_IOCTL_DEQUE_COMMAND: msg(%d), tg(%d), ret:%d\n", pCMD_T->task.msg_id, pCMD_T->task.tg_info, ret);
		    return MFALSE;
		}

		//LOG_INF("(%d),(%p),(%p),(%x),(%x),(%d), \n",cmd_get.task.msg_id,
		LOG_INF("CCU_IOCTL_DEQUE_COMMAND ack: msg(%d)->ack(%d), tg(%d), in(%p), out(%p), st(%d), \n",
		        pCMD_T->task.msg_id, \
                pCMD_R->task.msg_id, \
                pCMD_T->task.tg_info, \
                pCMD_R->task.in_data_ptr, \
                pCMD_R->task.out_data_ptr, \
                pCMD_R->status);

        if(pCMD_R->status == CCU_ENG_STATUS_TIMEOUT)
        {
            LOG_ERR("CCU wait ack CCU_IOCTL_DEQUE_COMMAND timeout: msg(%d), tg(%d)\n", pCMD_T->task.msg_id, pCMD_T->task.tg_info);
            LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[0] =============\n");
            printCcuLog((const char*)g_pLogBuf[0], MTRUE);
            LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[1] =============\n");
            printCcuLog((const char*)g_pLogBuf[1], MTRUE);
            DumpSramLog();
            PrintReg();
        }
        if(pCMD_R->status != CCU_ENG_STATUS_SUCCESS)
        {
            LOG_WRN("CCU_IOCTL_DEQUE_COMMAND status fail: %d\n", pCMD_R->status);
            return MFALSE;
        }
	}

	return true;

}

MBOOL CcuDrvImp::waitCmd( ccu_cmd_st *pCMD_R )
{
	//ccu_cmd_st cmd_get;

	LOG_INF("CCU_IOCTL_DEQUE_COMMAND\n");
	if(ioctl(this->m_Fd, CCU_IOCTL_DEQUE_COMMAND, pCMD_R) < 0)
	{
		LOG_ERR("ERROR: CCU_IOCTL_DEQUE_COMMAND:%x\n",pCMD_R->task.msg_id);
		return MFALSE;;
	}

	//::memcpy( pCMD, &cmd_get, sizeof(ccu_cmd_st) );

	//LOG_INF("(%d),(%p),(%p),(%x),(%x),(%d), \n",cmd_get.task.msg_id,
	LOG_INF("CCU_IOCTL_DEQUE_COMMAND result: (%d),(%p),(%p),(%d), \n",pCMD_R->task.msg_id, \
				  pCMD_R->task.in_data_ptr, \
				  pCMD_R->task.out_data_ptr, \
				  //cmd_get.task.sz_in_data,
				  //cmd_get.task.sz_out_data,
				  pCMD_R->status);

	return true;
}

FILE *tryOpenFile(char *path1, char *path2=NULL)
{
    FILE *pFile;
	FILE *pFile_empty;

    LOG_INF("open file, path: %s\n", path1);
    pFile = fopen(path1, "rwb");

    if(pFile == NULL)
    {
        LOG_ERR("open file fail: %d\n", pFile);
        LOG_ERR("open file path: %s\n", path1);

        if(path2 != NULL)
        {
            LOG_ERR("open file, path2: %s\n", path2);
            pFile = fopen(path2, "rwb");
            if(pFile == NULL)
            {
                LOG_ERR("open file fail_2: %d\n", pFile);
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }
    pFile_empty = pFile;

    if(fgetc(pFile_empty) == EOF)
    {
        LOG_ERR("Empty file\n");
        fclose(pFile_empty);
        return NULL;
    }
    return pFile;
}

size_t loadFileToBuffer(FILE *fp, unsigned int *buf)
{
    size_t szTell=0, szRead=0;

    fseek(fp, 0L, SEEK_END);
    szTell = ftell(fp);

    if(szTell > 0)
    {
        rewind(fp);
        LOG_INF("read file into buffer, szTell=%x\n", szTell);
        szRead = fread ( (void*) buf, sizeof(char), szTell, fp );
        LOG_INF("read file done\n");
    }
    else
    {
        LOG_ERR("file size <= 0, szTell=%x\n", szTell);
    }

    if(szRead != szTell)
    {
        LOG_ERR("read file error: szRead=%x, szTell=%x\n", szRead, szTell);
    }

    return szRead;
}

void clearAndLoadBinToMem(unsigned int *memBaseAddr, unsigned int *buf, unsigned int memSize, unsigned int binSize, unsigned int offset)
{
    volatile int dummy = 0;
    LOG_INF("clear MEM");
    LOG_INF("args: %p, %p, %x, %x, %x", memBaseAddr, buf, memSize, binSize, offset);
    /*Must clear mem with 4 byte aligned access, due to APMCU->CCU access path APB restriction*/
    for (unsigned int i=0;i<memSize/4;i++)
    {
        //for HW Test tool, must add a dummy operation between mem access, otherwise incurrs SIGBUS/BUS_ADRALN
        //root casue under checking...
        dummy = dummy + 1;
        *(memBaseAddr + i) = 0;
    }

    LOG_INF("load bin buffer onto MEM");
    LOG_INF("%x,%x,%x\n",buf[0],buf[1],buf[2]);

    for (unsigned int i=0;i<binSize/4;i++) {
        *(memBaseAddr + (offset/4) + i) = buf[i];
    }
}

MBOOL loadBin(char *binPath, unsigned int *memBaseAddr, unsigned int *buf, unsigned int memSize, unsigned int offset)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    FILE *pFile;
    size_t binSize;

    LOG_INF("open Bin file, path: %s\n", binPath);
    pFile = tryOpenFile(binPath);
    LOG_INF("open Bin file result:%d\n", pFile);
    if (pFile == NULL)
    {
        LOG_ERR("Open Bin file fail\n");
        return MFALSE;
    }

    LOG_INF("read Bin file into Bin buffer\n");
    binSize = loadFileToBuffer(pFile, buf);
    fclose(pFile);
    LOG_INF("read Bin done\n");

    LOG_INF("clear MEM & load Bin buffer onto MEM\n");
    clearAndLoadBinToMem(memBaseAddr, buf, memSize, binSize, offset);

    LOG_DBG("-:%s\n",__FUNCTION__);    

    return MTRUE;
}

static unsigned int pmemBuf[CCU_PMEM_SIZE] = {};
static unsigned int dmemBuf[CCU_DMEM_SIZE] = {};
MBOOL CcuDrvImp::loadCCUBin()
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;

    char *pmPath = "/system/vendor/bin/lib3a.ccu.pm";
    char *dmPath = "/system/vendor/bin/lib3a.ccu.dm";

    //========================= Load PMEM binary ===========================
    LOG_INF("Load PM");
	ret = loadBin(pmPath, p_gCcuDrvObj[CCU_PMEM]->m_pCcuHwRegAddr, pmemBuf, CCU_PMEM_SIZE, 0);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

    //========================= Load DMEM binary ===========================
    LOG_INF("Load DM");
    ret = loadBin(dmPath, p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr, dmemBuf, CCU_DMEM_SIZE, CCU_DMEM_OFFSET);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

LOAD_CCU_BIN_EXIT:
    LOG_DBG("-:%s\n",__FUNCTION__);
	return ret;
}

MBOOL CcuDrvImp::loadSensorBin(uint32_t sensor_idx)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    unsigned int sensorPmBuf[CCU_SENSOR_PM_SIZE];
    unsigned int sensorDmBuf[CCU_SENSOR_DM_SIZE];
    char *sensorPathPrefix = "/system/vendor/bin/libccu_";
    char sensorName[100] = "";
    char sensorPmPath[100] = "";
    char sensorDmPath[100] = "";

    //========================= Get sensor name via ioctl ===========================
    this->GetSensorName(this->m_Fd, sensor_idx, sensorName);
    LOG_DBG_MUST("ccu sensor name: %s\n", sensorName);

    strncat(sensorPmPath, sensorPathPrefix, 50);
    strncat(sensorPmPath, sensorName, 45);
    strncat(sensorPmPath, ".pm", 5);

    strncat(sensorDmPath, sensorPathPrefix, 50);
    strncat(sensorDmPath, sensorName, 45);
    strncat(sensorDmPath, ".dm", 5);

    //========================= Load Sensor PM binary ===========================
    LOG_INF("Load Sensor PM");
    ret = loadBin(sensorPmPath, p_gCcuDrvObj[CCU_PMEM]->m_pCcuHwRegAddr, sensorPmBuf, 0, SENSOR_PM_OFFSET);
    if(ret == MFALSE)
        goto LOAD_SENSOR_BIN_EXIT;

    //========================= Load Sensor DM binary ===========================
    LOG_INF("Load Sensor DM");
    ret = loadBin(sensorDmPath, p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr, sensorDmBuf, 0, SENSOR_DM_OFFSET);
    if(ret == MFALSE)
        goto LOAD_SENSOR_BIN_EXIT;

LOAD_SENSOR_BIN_EXIT:
    LOG_DBG("-:%s\n",__FUNCTION__);    
    return ret;
}

MBOOL CcuDrvImp::checkSensorSupport(uint32_t sensor_idx)
{
    FILE *pFile;
    char *sensorPathPrefix = "/system/vendor/bin/libccu_";
    char sensorName[100] = "";
    char sensorPmPath[100] = "";
    char sensorDmPath[100] = "";

    //========================= try open CCU kdrv ===========================
    if (!(this->OpenCcuKdrv()))
    {
        LOG_ERR("OpenCcuKdrv Fail\n");
        goto FAIL_EXIT;
    }

    //========================= Get sensor name via ioctl ===========================
    this->GetSensorName(this->m_Fd, sensor_idx, sensorName);
    LOG_DBG_MUST("ccu sensor name: %s\n", sensorName);

    strncat(sensorPmPath, sensorPathPrefix, 50);
    strncat(sensorPmPath, sensorName, 45);
    strncat(sensorPmPath, ".pm", 5);

    strncat(sensorDmPath, sensorPathPrefix, 50);
    strncat(sensorDmPath, sensorName, 45);
    strncat(sensorDmPath, ".dm", 5);

    //========================= Try Open Sensor PMEM binary ===========================
    LOG_INF("open sensorPm file, path: %s\n", sensorPmPath);
    pFile = tryOpenFile(sensorPmPath);
    LOG_INF("open sensorPm file result:%d\n", pFile);
    if (pFile == NULL) { LOG_ERR("Open sensorPm fail\n"); goto FAIL_EXIT; }

    fclose(pFile);
    LOG_INF("read sensorDm done\n");

    //========================= Try Open Sensor DMEM binary ===========================
    LOG_INF("open sensorDm file, path: %s\n", sensorDmPath);
    pFile = tryOpenFile(sensorDmPath);
    LOG_INF("open sensorDm file result:%d\n", pFile);
    if (pFile == NULL) { LOG_ERR("Open sensorDm fail\n"); goto FAIL_EXIT; }

    fclose(pFile);
    LOG_INF("read sensorDm done\n");

    this->CloseCcuKdrv();
    return MTRUE;

FAIL_EXIT:
    this->CloseCcuKdrv();
    return MFALSE;
}

MBOOL CcuDrvImp::GetSensorName(MINT32 kdrvFd, uint32_t sensor_idx, char *sensorName)
{
    #define SENSOR_NAME_MAX_LEN 32
    char sensorNames[IMGSENSOR_SENSOR_IDX_MAX_NUM][SENSOR_NAME_MAX_LEN];

    if(ioctl(kdrvFd, CCU_IOCTL_GET_SENSOR_NAME, sensorNames) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_SENSOR_NAME\n");
        goto FAIL_EXIT;
    }

    for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; ++i)
    {
        LOG_DBG_MUST("ccu sensor %d name: %s\n", i, sensorNames[i]);
    }

    if (sensor_idx >= IMGSENSOR_SENSOR_IDX_MIN_NUM && sensor_idx < IMGSENSOR_SENSOR_IDX_MAX_NUM)
    {
        memcpy(sensorName, sensorNames[sensor_idx], strlen(sensorNames[sensor_idx])+1);
    }
    else
    {
        LOG_ERR("_getSensorName error, invalid sensoridx: %x\n", sensor_idx);
        goto FAIL_EXIT;
    }

    #undef SENSOR_NAME_MAX_LEN

    return MTRUE;

FAIL_EXIT:
    return MFALSE;
}

MBOOL CcuDrvImp::setI2CChannel(uint32_t channel)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_SET_I2C_CHANNEL, &channel) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_SET_I2C_CHANNEL_T\n");
        LOG_WRN("============ CCU FAIL LOG DUMP: LOGBUF[0] =============\n");
        printCcuLog((const char*)g_pLogBuf[0], MTRUE);
        LOG_WRN("============ CCU FAIL LOG DUMP: LOGBUF[1] =============\n");
        printCcuLog((const char*)g_pLogBuf[1], MTRUE);
        DumpSramLog();
        PrintReg();
        return MFALSE;
    }

    return MTRUE;

}

MBOOL CcuDrvImp::getI2CDmaBufferAddr(uint32_t *va, uint32_t *pa_h, uint32_t *pa_l, uint32_t *i2c_id)
{
    uint32_t arg[4];

    if(ioctl(this->m_Fd, CCU_IOCTL_GET_I2C_DMA_BUF_ADDR, arg) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_I2C_DMA_BUF_ADDR\n");
        return MFALSE;
    }

    *va   = arg[0];
    *pa_h = arg[1];
    *pa_l = arg[2];
	*i2c_id = arg[3];
    return MTRUE;
}


MBOOL CcuDrvImp::setI2CMode(unsigned int i2c_write_id, unsigned int transfer_len)
{
    struct ccu_i2c_arg i2c_arg = {i2c_write_id, transfer_len};

    if(ioctl(this->m_Fd, CCU_IOCTL_SET_I2C_MODE, &i2c_arg) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_SET_I2C_MODE:%x, %x\n",i2c_write_id, transfer_len);
        return MFALSE;
    }

    return MTRUE;
}

int32_t CcuDrvImp::getCurrentFps(int32_t *current_fps)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_GET_CURRENT_FPS, current_fps) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_CURRENT_FPS:%x\n", *current_fps);
        return MFALSE;
    }

    return MTRUE;
}

void CcuDrvImp::createThread()
{
    LOG_DBG("+\n");

    this->m_taskWorkerThreadReady = MFALSE;
    this->m_taskWorkerThreadEnd = MFALSE;

	sem_init(&mSem, 0, 0);

    pthread_attr_t attr;
    struct sched_param pthread_param = {//set thread priority referencr from ThreadPawImp.cpp
        .sched_priority = NICE_CAMERA_3A_MAIN
    };
    pthread_attr_setschedparam(&attr, &pthread_param);
	pthread_create(&mThread, NULL, ApTaskWorkerThreadLoop, this);
    pthread_setname_np(mThread, "ccu_worker");

    LOG_DBG("ApTaskWorkerThreadLoop created, wait for m_taskWorkerThreadReady\n");
    LOG_DBG("m_taskWorkerThreadReady addr: %p\n", &(this->m_taskWorkerThreadReady));
    LOG_DBG_MUST("ApTaskWorkerThreadLoop start\n");
    double starttime, endtime, duration;
    struct timespec sts,ets;
    clock_gettime(CLOCK_REALTIME, &sts);
    starttime = (1000000.0*sts.tv_sec) + (sts.tv_nsec/1000);
    endtime = starttime;
    while(this->m_taskWorkerThreadReady != MTRUE) {
        clock_gettime(CLOCK_REALTIME, &ets);
        endtime = (1000000.0*ets.tv_sec) + (ets.tv_nsec/1000);
        duration = endtime - starttime;
        if(duration > CCU_INIT_TIMEOUT_SPEC)
        {
            LOG_WRN("CCU ApTaskWorkerThreadLoop created duration(%lf) ts_end(%lf) ts_start(%lf) ", duration, endtime, starttime);
            break;
        }
    }
 
    LOG_DBG_MUST("ApTaskWorkerThreadLoop duration %lf\n",endtime - starttime);
    LOG_DBG("m_taskWorkerThreadReady: %d\n", this->m_taskWorkerThreadReady);

    LOG_DBG("-\n");
}
int CcuDrvImp::AFcreateThread()
{
    //int ret;
    void *ret1;
    int ret2;
    LOG_DBG("+\n");

    this->AFm_taskWorkerThreadReady = MFALSE;
    this->AFm_taskWorkerThreadEnd = MFALSE;

	sem_init(&AFmSem, 0, 0);

    //ret1 = AFApTaskWorkerThread(this);

	ret1 = AFApTaskWorkerThread(this);

    LOG_DBG("AFApTaskWorkerThreadLoop created, wait for m_taskWorkerThreadReady\n");
    LOG_DBG("AFm_taskWorkerThreadReady addr: %p\n", &(this->AFm_taskWorkerThreadReady));
    while(this->AFm_taskWorkerThreadReady != MTRUE){}
    LOG_DBG("AFm_taskWorkerThreadReady: %d\n", this->AFm_taskWorkerThreadReady);
    LOG_DBG("ret1: %d\n", ret1);
    LOG_DBG("-\n");

    if(ret1==(void *)3)
        return 3;
    else
        return 0;

}

int CcuDrvImp::AFBcreateThread()
{
    //int ret;
    void *ret1;
    int ret2;
    LOG_DBG("+\n");

    this->AFBm_taskWorkerThreadReady = MFALSE;
    this->AFBm_taskWorkerThreadEnd = MFALSE;

	sem_init(&AFBmSem, 0, 0);

    //ret1 = AFApTaskWorkerThread(this);

	ret1 = AFBApTaskWorkerThread(this);

    LOG_DBG("AFbApTaskWorkerThreadLoop created, wait for m_taskWorkerThreadReady\n");
    LOG_DBG("AFbm_taskWorkerThreadReady addr: %p\n", &(this->AFBm_taskWorkerThreadReady));
    while(this->AFBm_taskWorkerThreadReady != MTRUE){}
    LOG_DBG("AFbm_taskWorkerThreadReady: %d\n", this->AFBm_taskWorkerThreadReady);
    LOG_DBG("ret1: %d\n", ret1);
    LOG_DBG("-\n");

    if(ret1==(void *)4)
        return 4;
    else
        return 0;

}

void CcuDrvImp::destroyThread()
{
    MINT32 temp = 0;

	LOG_DBG("+\n");

    this->m_taskWorkerThreadEnd = MTRUE;
    this->m_taskWorkerThreadReady = MFALSE;

    if(ioctl(this->m_Fd, CCU_IOCTL_FLUSH_LOG, &temp) < 0)
    {
        LOG_ERR("CCU_IOCTL_FLUSH_LOG\n");
    }

    //pthread_kill(threadid, SIGKILL);
	pthread_join(mThread, NULL);

	//
	LOG_DBG("-\n");
}

void CcuDrvImp::AFdestroyThread()
{
    MINT32 temp = 0;

	LOG_DBG("+\n");

    this->AFm_taskWorkerThreadEnd = MTRUE;
    this->AFm_taskWorkerThreadReady = MFALSE;

    if(ioctl(this->m_Fd, CCU_IOCTL_FLUSH_LOG, &temp) < 0)
    {
        LOG_ERR("CCU_IOCTL_FLUSH_LOG\n");
    }

    //pthread_kill(threadid, SIGKILL);
	pthread_join(AFmThread, NULL);

	//
	LOG_DBG("-\n");
}

void CcuDrvImp::AFBdestroyThread()
{
    MINT32 temp = 0;

	LOG_DBG("+\n");

    this->AFBm_taskWorkerThreadEnd = MTRUE;
    this->AFBm_taskWorkerThreadReady = MFALSE;

    if(ioctl(this->m_Fd, CCU_IOCTL_FLUSH_LOG, &temp) < 0)
    {
        LOG_ERR("CCU_IOCTL_FLUSH_LOG\n");
    }

    //pthread_kill(threadid, SIGKILL);
	pthread_join(AFBmThread, NULL);

	//
	LOG_DBG("-\n");
}

static int js_cnt = -1;
void CcuDrvImp::PrintReg()
{
    MUINT32 *ccuCtrlBasePtr = (MUINT32 *)p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr;
    MUINT32 *ccuCtrlPtr = (MUINT32 *)p_gCcuDrvObj[CCU_A]->m_pCcuHwRegAddr;
    MUINT32 *ccuDmPtr = (MUINT32 *)p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr;
    MUINT32 *ccuPmPtr = (MUINT32 *)p_gCcuDrvObj[CCU_PMEM]->m_pCcuHwRegAddr;

    LOG_WRN("=============== CCU REG DUMP START ===============\n");
    for(int i=0 ; i<CCU_HW_DUMP_SIZE ; i += 0x10)
    {
        LOG_WRN("0x%08x: 0x%08x ,0x%08x ,0x%08x ,0x%08x\n", i, *(ccuCtrlPtr), *(ccuCtrlPtr+1), *(ccuCtrlPtr+2), *(ccuCtrlPtr+3));
        ccuCtrlPtr += 4;
    }
    LOG_WRN("=============== CCU DM DUMP START ===============\n");
    for(int i=0 ; i<CCU_DMEM_SIZE ; i += 16)
    {
        LOG_WRN("0x8%07x: 0x%08x ,0x%08x ,0x%08x ,0x%08x\n", i, *(ccuDmPtr), *(ccuDmPtr+1), *(ccuDmPtr+2), *(ccuDmPtr+3));
        ccuDmPtr += 4;
    }
    LOG_WRN("=============== CCU PM DUMP START ===============\n");
    for(int i=0 ; i<CCU_PMEM_SIZE ; i += 16)
    {
        LOG_WRN("0x%08x: 0x%08x ,0x%08x ,0x%08x ,0x%08x\n", i, *(ccuPmPtr), *(ccuPmPtr+1), *(ccuPmPtr+2), *(ccuPmPtr+3));
        ccuPmPtr += 4;
    }
    LOG_WRN("-------- DMA DEBUG INFO --------\n");
    *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG_SEL) = 0x0013;
    LOG_WRN("SET DMA_DBG_SEL 0x0013, read out DMA_DBG: %08x\n", *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG));
    *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG_SEL) = 0x0113;
    LOG_WRN("SET DMA_DBG_SEL 0x0113, read out DMA_DBG: %08x\n", *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG));
    *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG_SEL) = 0x0213;
    LOG_WRN("SET DMA_DBG_SEL 0x0213, read out DMA_DBG: %08x\n", *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG));
    *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG_SEL) = 0x0313;
    LOG_WRN("SET DMA_DBG_SEL 0x0313, read out DMA_DBG: %08x\n", *(ccuCtrlBasePtr + OFFSET_CCU_A_DMA_DEBUG));
    LOG_WRN("=============== CCU REG DUMP END ===============\n");
}

char ccu_sram_log_tmp_buf[CCU_LOG_SIZE];
char ccu_sram_isr_log_tmp_buf[CCU_ISR_LOG_SIZE];
void CcuDrvImp::DumpSramLog()
{
    char *ccuCtrlBasePtr = (char *)p_gCcuDrvObj[CCU_DMEM]->m_pCcuHwRegAddr;
    char *ccuLogPtr_1 = ccuCtrlBasePtr + CCU_LOG_BASE;
    char *ccuLogPtr_2 = ccuCtrlBasePtr + CCU_LOG_BASE + CCU_LOG_SIZE;
    char *isrLogPtr = ccuCtrlBasePtr + CCU_ISR_LOG_BASE;

    LOG_WRN("=============== CCU INTERNAL LOG DUMP START ===============\n");
    //Avoid to 64bits system APB bus hang
    for(int i = 0 ; i < CCU_LOG_SIZE ; i++)
        ccu_sram_log_tmp_buf[i] = *(ccuLogPtr_1 + i);
    ccu_sram_log_tmp_buf[CCU_LOG_SIZE - 1] = '\0';
    LOG_WRN("---------CHUNK 1-----------\n%s", ccu_sram_log_tmp_buf);

    for(int i = 0 ; i < CCU_LOG_SIZE ; i++)
        ccu_sram_log_tmp_buf[i] = *(ccuLogPtr_2 + i);
    ccu_sram_log_tmp_buf[CCU_LOG_SIZE - 1] = '\0';
    LOG_WRN("---------CHUNK 2-----------\n%s", ccu_sram_log_tmp_buf);

    for(int i = 0 ; i < CCU_ISR_LOG_SIZE ; i++)
        ccu_sram_isr_log_tmp_buf[i] = *(isrLogPtr + i);
    ccu_sram_isr_log_tmp_buf[CCU_ISR_LOG_SIZE - 1] = '\0';
    LOG_WRN("---------ISR LOG-----------\n%s", ccu_sram_isr_log_tmp_buf);
    LOG_WRN("=============== CCU INTERNAL LOG DUMP END ===============\n");
}

void* CcuDrvImp::AFApTaskWorkerThread(void *arg)
{
    CcuDrvImp *_this = reinterpret_cast<CcuDrvImp*>(arg);
    MBOOL ret;
    CCU_WAIT_IRQ_ST _irq;
    int i = 1;
	LOG_DBG("+:\n");

    _this->AFm_taskWorkerThreadReady = MTRUE;
    LOG_VRB("AFm_taskWorkerThreadReady addr: %p\n", &(_this->AFm_taskWorkerThreadReady));
    LOG_VRB("AFm_taskWorkerThreadReady val: %d\n", _this->AFm_taskWorkerThreadReady);

	do
    {
        //check if ap task thread should terminate
        if ( _this->AFm_taskWorkerThreadEnd )
        {
            LOG_DBG("AFm_taskWorkerThreadEnd\n");
            break;
        }

        LOG_VRB("AFcall waitIrq() to wait ccu interrupt...\n");

        _irq.Clear      = CCU_IRQ_CLEAR_WAIT;
        _irq.Status     = CCU_INT_ST;
        _irq.St_type    = CCU_SIGNAL_INT;
        _irq.Timeout    = 1000; //wait forever
        //_irq.Timeout    = 100; //wait 100 ms
        _irq.TimeInfo.passedbySigcnt = 0x00ccdd00;
        ret = _this->waitAFIrq(&_irq);


        if ( _this->AFm_taskWorkerThreadEnd )
        {
            LOG_DBG("AFm_taskWorkerThreadEnd\n");
            break;
        }

        if ( MFALSE == ret )
        {
            LOG_ERR("AFwaitIrq fail\n");
        }
        else
        {
            LOG_VRB("AFwaitIrq() done, irq_cnt(%d), irq_task(%d) \n", ++js_cnt, _irq.TimeInfo.passedbySigcnt);
            //break;
            if ( 3 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_DBG("AFm_taskWorkerThread : AFO done\n");
                return((void *)3);

            }
            else if(0x00ccdd00 == _irq.TimeInfo.passedbySigcnt)
            {
                LOG_VRB("AFno irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
            else
            {
                LOG_ERR("AFunknow irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
        }
    i = 0;
	}while(i);

	LOG_DBG("-:\n");

	//return NULL;
    return((void *)0);
}

void* CcuDrvImp::AFBApTaskWorkerThread(void *arg)
{
    CcuDrvImp *_this = reinterpret_cast<CcuDrvImp*>(arg);
    MBOOL ret;
    CCU_WAIT_IRQ_ST _irq;
    int i = 1;
	LOG_DBG("+:\n");

    _this->AFBm_taskWorkerThreadReady = MTRUE;
    LOG_VRB("AFBm_taskWorkerThreadReady addr: %p\n", &(_this->AFBm_taskWorkerThreadReady));
    LOG_VRB("AFBm_taskWorkerThreadReady val: %d\n", _this->AFBm_taskWorkerThreadReady);

	do
    {
        //check if ap task thread should terminate
        if ( _this->AFBm_taskWorkerThreadEnd )
        {
            LOG_DBG("AFBm_taskWorkerThreadEnd\n");
            break;
        }

        LOG_VRB("AFBcall waitIrq() to wait ccu interrupt...\n");

        _irq.Clear      = CCU_IRQ_CLEAR_WAIT;
        _irq.Status     = CCU_INT_ST;
        _irq.St_type    = CCU_SIGNAL_INT;
        _irq.Timeout    = 1000; //wait forever
        //_irq.Timeout    = 100; //wait 100 ms
        _irq.TimeInfo.passedbySigcnt = 0x00eeff00;
        ret = _this->waitAFBIrq(&_irq);


        if ( _this->AFBm_taskWorkerThreadEnd )
        {
            LOG_DBG("AFBm_taskWorkerThreadEnd\n");
            break;
        }

        if ( MFALSE == ret )
        {
            LOG_ERR("AFBwaitIrq fail\n");
        }
        else
        {
            LOG_VRB("AFBwaitIrq() done, irq_cnt(%d), irq_task(%d) \n", ++js_cnt, _irq.TimeInfo.passedbySigcnt);
            //break;
            if ( 4 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_DBG("AFBm_taskWorkerThread : AFO done\n");
                return((void *)4);

            }
            else if(0x00eeff00 == _irq.TimeInfo.passedbySigcnt)
            {
                LOG_VRB("AFbno irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
            else
            {
                LOG_ERR("AFbunknow irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
        }
    i = 0;
	}while(i);

	LOG_DBG("-:\n");

	//return NULL;
    return((void *)0);
}

void* CcuDrvImp::ApTaskWorkerThreadLoop(void *arg)
{
    CcuDrvImp *_this = reinterpret_cast<CcuDrvImp*>(arg);
    MBOOL ret;
    CCU_WAIT_IRQ_ST _irq;

	LOG_DBG("+:\n");

    _this->m_taskWorkerThreadReady = MTRUE;
    LOG_VRB("m_taskWorkerThreadReady addr: %p\n", &(_this->m_taskWorkerThreadReady));
    LOG_VRB("m_taskWorkerThreadReady val: %d\n", _this->m_taskWorkerThreadReady);

	do
    {
        //check if ap task thread should terminate
        if ( _this->m_taskWorkerThreadEnd )
        {
            LOG_DBG("m_taskWorkerThreadEnd\n");
            break;
        }

        LOG_VRB("call waitIrq() to wait ccu interrupt...\n");

        _irq.Clear      = CCU_IRQ_CLEAR_WAIT;
        _irq.Status     = CCU_INT_ST;
        _irq.St_type    = CCU_SIGNAL_INT;
        //_irq.Timeout    = 0; //wait forever
        _irq.Timeout    = 100; //wait 100 ms
        _irq.TimeInfo.passedbySigcnt = 0x00aabb00;
        ret = _this->waitIrq(&_irq);

        if ( _this->m_taskWorkerThreadEnd )
        {
            LOG_DBG("m_taskWorkerThreadEnd\n");
            break;
        }

        if ( MFALSE == ret )
        {
            LOG_ERR("waitIrq fail\n");
        }
        else
        {
            LOG_VRB("waitIrq() done, irq_cnt(%d), irq_task(%d) \n", ++js_cnt, _irq.TimeInfo.passedbySigcnt);
            //break;
            if ( 1 == _irq.TimeInfo.passedbySigcnt )
            {
                printCcuLog((const char*)g_pLogBuf[0]);
            }
            else if ( 2 == _irq.TimeInfo.passedbySigcnt )
            {
                printCcuLog((const char*)g_pLogBuf[1]);
            }
            else if ( -1 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[0] =============\n");
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[1] =============\n");
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
                _this->DumpSramLog();
                _this->PrintReg();
                AEE_ASSERT_CCU_USER("CCU ASSERT\n");
            }
            else if ( -2 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
            }
            else if ( -3 == _irq.TimeInfo.passedbySigcnt )
            {
                //I2C Timeout Warning
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
                AEE_ASSERT_CCU_USER("CCU I2C Timeout Recovery\n");
            }
            else if(0x00aabb00 == _irq.TimeInfo.passedbySigcnt)
            {
                LOG_VRB("no irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
            else
            {
                LOG_WRN("unknow irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
                LOG_WRN("============ CCU UNKNOW_IRQ LOG DUMP: LOGBUF[0] =============\n");
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU UNKNOW_IRQ LOG DUMP: LOGBUF[1] =============\n");
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
                _this->DumpSramLog();
                _this->PrintReg();
            }
        }
	}while(1);

	LOG_DBG("-:\n");

	return NULL;
}

/*=======================================================================================

=======================================================================================*/
static CcuMemImp    gCcuMemObj;
//
CcuMemImp::CcuMemImp():
gIonDevFD(-1)
{
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
}
//
CcuMemImp* CcuMemImp::createInstance()
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    return (CcuMemImp*)&gCcuMemObj;
}
//
MBOOL CcuMemImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    LOG_DBG("+:%s/%s\n",__FUNCTION__,userName);

    this->gIonDevFD = mt_ion_open("CCU");
    if  ( 0 > this->gIonDevFD )
    {
        LOG_ERR("mt_ion_open() return %d", this->gIonDevFD);
		return false;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return Result;
}
//
MBOOL CcuMemImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;

    LOG_DBG("+:%s/%s\n",__FUNCTION__,userName);

	if( 0 <= this->gIonDevFD ) {
		ion_close(this->gIonDevFD);
	}

    LOG_DBG("-:%s\n",__FUNCTION__);

    return Result;
}
//
MBOOL CcuMemImp::ccu_malloc(int drv_h, int len, int *buf_share_fd, char **buf_va)
{
    ion_user_handle_t buf_handle;

    LOG_DBG("+:%s\n",__FUNCTION__);

    // allocate ion buffer handle
    if(ion_alloc_mm(drv_h, (size_t)len, 0, 0, &buf_handle))        // no alignment, non-cache
    {
        LOG_WRN("fail to get ion buffer handle (drv_h=0x%x, len=%d)", drv_h, len);
        return false;
    }
    // get ion buffer share handle
    if(ion_share(drv_h, buf_handle, buf_share_fd))
    {
        LOG_WRN("fail to get ion buffer share handle");
        if(ion_free(drv_h,buf_handle))
            LOG_WRN("ion free fail");
        return false;
    }
    // get buffer virtual address
    *buf_va = ( char *)ion_mmap(drv_h, NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, *buf_share_fd, 0);
    if(*buf_va == NULL) {
        LOG_WRN("fail to get buffer virtual address");
    }
    LOG_DBG("alloc ion: ion_buf_handle %d, share_fd %d, va: %p \n", buf_handle, *buf_share_fd, *buf_va);
    //
    LOG_DBG("-:%s\n",__FUNCTION__);

    return (*buf_va != NULL)?MTRUE:MFALSE;

}
//
MBOOL CcuMemImp::ccu_free(int drv_h, int len,int buf_share_fd, char *buf_va)
{
    ion_user_handle_t buf_handle;

    LOG_DBG("+:%s,fd(%x),va(%p),sz(%d)\n",__FUNCTION__,buf_share_fd,buf_va,len);

    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        LOG_WRN("fail to get import share buffer fd");
        return false;
    }
    LOG_VRB("import ion: ion_buf_handle %d, share_fd %d", buf_handle, buf_share_fd);
    // 2. free for IMPORT ref cnt
    if(ion_free(drv_h, buf_handle))
    {
        LOG_WRN("fail to free ion buffer (free ion_import ref cnt)");
        return false;
    }
    // 3. unmap virtual memory address
    if(ion_munmap(drv_h, (void *)buf_va, (size_t)len))
    {
        LOG_WRN("fail to get unmap virtual memory");
        return false;
    }
    // 4. close share buffer fd
    if(ion_share_close(drv_h, buf_share_fd))
    {
        LOG_WRN("fail to close share buffer fd");
        return false;
    }
    // 5. pair of ion_alloc_mm
    if(ion_free(drv_h, buf_handle))
    {
        LOG_WRN("fail to free ion buffer (free ion_alloc_mm ref cnt)");
        return false;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;

}
//
MBOOL CcuMemImp::mmapMVA( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva )
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_DBG("+:%s\n",__FUNCTION__);

    //a. get handle from IonBufFd and increase handle ref count
    if(ion_import(gIonDevFD, buf_share_fd, p_ion_handle))
    {
        LOG_ERR("ion_import fail, ion_handle(0x%x)", *p_ion_handle);
        return false;
    }
    LOG_VRB("ion_import: share_fd %d, ion_handle %d", buf_share_fd, *p_ion_handle);
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = *p_ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_CAM_CCUG;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start  = 0x10000000;
    mm_data.config_buffer_param.reserve_iova_end    = 0xFFFFFFFF;
    err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        LOG_ERR("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        LOG_ERR("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
    }
    //c. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = *p_ion_handle;
    sys_data.get_phys_param.phy_addr = (M4U_PORT_CAM_CCUG<<24) | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        LOG_ERR("ion_custom_ioctl get_phys_param failed!");
        return false;
    }
	//
	*mva = (unsigned int)sys_data.get_phys_param.phy_addr;

    //req_buf->plane[plane_idx].ion_handle = ion_handle;
    //req_buf->plane[plane_idx].mva        = sys_data.get_phys_param.phy_addr;
    //LOG_VRB("get_phys: buffer[%d], plane[%d], mva 0x%x", buf_idx, plane_idx, buf_n->planes[plane_idx].ptr);

    LOG_DBG("\n");
    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;
}
//
MBOOL CcuMemImp::munmapMVA( ion_user_handle_t ion_handle )
{
       LOG_DBG("+:%s\n",__FUNCTION__);

        // decrease handle ref count
        if(ion_free(this->gIonDevFD, ion_handle))
        {
            LOG_ERR("ion_free fail");
            return false;
        }
        LOG_VRB("ion_free: ion_handle %d", ion_handle);

        LOG_DBG("-:%s\n",__FUNCTION__);
        return true;
}
//

int32_t CcuDrvImp::getSensorI2CInfo(struct ccu_i2c_info *sensorI2CInfo)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_GET_SENSOR_I2C_SLAVE_ADDR , sensorI2CInfo) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_SENSOR_SLAVE_ID\n");
        return MFALSE;
    }

    return MTRUE;
}







