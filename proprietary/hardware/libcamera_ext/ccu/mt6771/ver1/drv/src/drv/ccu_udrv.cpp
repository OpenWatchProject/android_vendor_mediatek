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
#include <string.h>
#include <errno.h>

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

#include <sys/stat.h>                   // "Struct stat"

#include <sys/mman.h>
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include "utilSystrace.h"

 /* kernel files */
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "ccu_ext_interface/ccu_mailbox_extif.h"
#include "ccu_drv.h"
#include "ccu_platform_def.h"

//#include <mtkcam/def/common.h>
#include <string.h>
#include <ccu_udrv_stddef.h>
#include <ccu_udrv.h>
#include "ccu_drvutil.h"

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "ccu_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(ccu_drv);

#define EVEREST_EP_MARK_CODE
using namespace NSCcuIf;

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
            LOG_DBG("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_DBG("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
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

#define LOG_NORMAL_HEADER 0xCCCC
#define LOG_MUST_HEADER 0xBBBB
#define LOG_ENDEND 0xDDDDDDDD
static void printCcuLog(const char *logStr, MBOOL mustPrint=MFALSE)
{
    #define LOG_LEN_LIMIT 800
    const char *logPtr = logStr;
    static char logBuf[LOG_LEN_LIMIT + 1];

    int logSegCnt = 0;
    MUINT32 *ccuLOGBasePtr = (MUINT32 *)logStr;
    char log_str[256] = {0};
    MUINT32 log_value[8] = {0};
    MUINT32 log_header, log_header_id;

    unsigned char log_para_num, log_str_len;
    char plog_out_str[LOG_LEN_LIMIT + 1] = {0};
    MUINT32 log_offset;

    bool printing = true;
    bool print_log_all = false;;

    if ((mustPrint==true) || (ccu_drv_DbgLogEnable_DEBUG) || (ccu_drv_DbgLogEnable_VERBOSE))
    {
        print_log_all=true;
    }

    while(printing)
    {
LOG_CONTINUE :

        log_header=*ccuLOGBasePtr++;

        log_offset = 0;
        while (1)
        {
            log_header_id=log_header>>16;
            log_para_num=(log_header&0xFF00)>>8;
            log_str_len=((log_header &0xFF)+3) & 0xFC;

            if (log_header_id==LOG_NORMAL_HEADER)
            {
                if (print_log_all)
                {
                    break;
                }
                else
                {   // do not print the log, jump to next log packet.
                    ccuLOGBasePtr+=(log_para_num + (log_str_len>>2));
                    log_header=*ccuLOGBasePtr++;
                }
            }
            else if (log_header_id==LOG_MUST_HEADER)
            {
                break;
            }
            else if (log_header==LOG_ENDEND)
            {
                goto LOG_EXIT;
            }
            else
            {
                log_header=*ccuLOGBasePtr++;
            }
        }

        while (1)
        {
            if(log_para_num > 8)
            {
                ccuLOGBasePtr++;
                goto LOG_CONTINUE;
            }

            for (int i=0; i<log_para_num; i++)
            {
                if(*ccuLOGBasePtr!=LOG_ENDEND)
                    log_value[i]=*ccuLOGBasePtr++;
                else
                    goto LOG_EXIT;
            }

            char *ccuLOGstringPtr = (char *)ccuLOGBasePtr;

            for (int i=0; i<log_str_len; i+=4)
            {
                if(*(MUINT32 *)ccuLOGstringPtr!=LOG_ENDEND)
                {
                    log_str[i]=*ccuLOGstringPtr++;
                    log_str[i+1]=*ccuLOGstringPtr++;
                    log_str[i+2]=*ccuLOGstringPtr++;
                    log_str[i+3]=*ccuLOGstringPtr++;
                }
                else
                {
                    goto LOG_EXIT;
                }
            }

            ccuLOGBasePtr += (log_str_len>>2);

            if(log_offset + log_str_len + 4*log_para_num> (LOG_LEN_LIMIT-64))
            {
                *(plog_out_str+log_offset)='\0';

                if(mustPrint)
                    LOG_WRN("\n----- DUMP SEG[%d] -----\n%s", logSegCnt, plog_out_str);
                else
                    LOG_INF("\n%s", plog_out_str);

                log_offset = 0;
                logSegCnt++;
            }

            int logValue_idx = 0;

            for (unsigned char i=0; i<log_str_len; i++)
            {
                if ((*(log_str+i)=='%') && (log_offset < LOG_LEN_LIMIT - 10))
                {
                    i+=1;
                    if ((*(log_str+i)=='d' || *(log_str+i)=='D') && (logValue_idx < 8))
                        sprintf(plog_out_str+log_offset,"%d", log_value[logValue_idx++]);
                    else if ((*(log_str+i)=='x' || *(log_str+i)=='X') && (logValue_idx < 8))
                        sprintf(plog_out_str+log_offset,"0x%08X", log_value[logValue_idx++]);

                    log_offset +=strlen(plog_out_str+log_offset);
                }
                else if((*(log_str+i)!='#') && (log_offset < LOG_LEN_LIMIT - 1))
                {
                    *(plog_out_str+log_offset)=*(log_str+i);
                    log_offset += 1;
                }
            }

            log_header=*ccuLOGBasePtr++;
            while (1)
            {
                log_header_id=log_header>>16;
                log_para_num=(log_header&0xFF00)>>8;
                log_str_len=((log_header &0xFF)+3) & 0xFC;

                if (log_header_id==LOG_NORMAL_HEADER)
                {
                    if (print_log_all)
                    {
                        break;
                    }
                    else
                    {   // do not print the log, jump to next log packet.
                        ccuLOGBasePtr+=(log_para_num + (log_str_len>>2));
                        log_header=*ccuLOGBasePtr++;
                    }
                }
                else if (log_header_id==LOG_MUST_HEADER)
                {
                    break;
                }
                else if (log_header==LOG_ENDEND)
                {
                    goto LOG_EXIT;
                }
                else
                {
                    log_header=*ccuLOGBasePtr++;
                }
            }
        }

LOG_EXIT :
        *(plog_out_str+log_offset)='\0';

        if(mustPrint)
            LOG_WRN("\n----- DUMP SEG[%d] -----\n%s", logSegCnt, plog_out_str);
        else
            LOG_INF("\n%s", plog_out_str);
        printing = false;
    }

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

static  CcuDrvImp    gCcuDrvObj[MAX_CCU_HW_MODULE];
CcuDrv* CcuDrvImp::createInstance(CCU_HW_MODULE module)
{
    LOG_DBG("+,module(%d)",module);
    gCcuDrvObj[module].m_HWmodule = module;
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
        LOG_ERR("CCU_IOCTL_SET_POWER: %d, bON(%d)\n", this->m_HWmodule, m_power.bON);
        return MFALSE;
    }
    LOG_DBG_MUST("%s: setpower(%d) cmd done.\n", __FUNCTION__, m_power.bON);

    LOG_DBG("-:%s\n", __FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::importmem(import_mem_t mempool)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_IMPORT_MEM, &mempool) < 0)
    {
        LOG_ERR("CCU_IOCTL_IMPORT_MEM: %d, bON(%d)\n", this->m_HWmodule, mempool);
        return MFALSE;
    }
    return MTRUE;
}

struct shared_buf_map *CcuDrvImp::getSharedBufMap()
{
    return m_shared_buf_map_ptr;
}

void *CcuDrvImp::ccuAddrToVa(MUINT32 ccuAddr)
{
    void *vaOut = NULL;
    MUINT32 ccuAddrOffset = 0;
    MUINT8 *ccuDmemBasePtr = (MUINT8 *)gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr;

    if(ccuAddr >= CCU_DCCM_REMAP_BASE) //the address in located in DCCM
    {
        ccuAddrOffset = ccuAddr - CCU_DCCM_REMAP_BASE;
        vaOut = (void *)(ccuDmemBasePtr + ccuAddrOffset);
    }
    else if(ccuAddr >= CCU_CACHE_BASE) //the address in located in DDR
    {
        ccuAddrOffset = ccuAddr - CCU_CACHE_BASE;
        vaOut = (void *)(m_ddr_buf_va + ccuAddrOffset);
    }
    else
    {
        LOG_ERR("ccuAddr not in DDR nor in DCCM");
    }

    LOG_DBG("dmembase(%p), ddrBase(%p), ccuAddr(%x), offset(%x), vaOut(%p)", ccuDmemBasePtr, m_ddr_buf_va, ccuAddr, ccuAddrOffset, vaOut);

    return vaOut;
}

MBOOL CcuDrvImp::setCcuRun()
{
    MUINT32 *ccuCtrlBasePtr = (MUINT32 *)gCcuDrvObj[CCU_A].m_pCcuHwRegAddr;
    MUINT8 *ccuDmemBasePtr = (MUINT8 *)gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr;

    LOG_DBG("+:CCU_IOCTL_SET_RUN\n");

    //Set remap offset
    MUINT32 remapOffset = m_ddr_buf_mva - CCU_CACHE_BASE;
    *(ccuCtrlBasePtr + OFFSET_CCU_REMAP_OFFSET) = remapOffset;
    LOG_INF_MUST("set CCU remap offset: %x\n", remapOffset);

    LOG_DBG("+:CCU_IOCTL_SET_RUN : %x\n",ccuCtrlBasePtr + OFFSET_CCU_A_INFO20/4);
    *(ccuCtrlBasePtr + OFFSET_CCU_A_INFO20/4) = (ccu_drv_DbgLogEnable_VERBOSE)? 6 : (ccu_drv_DbgLogEnable_DEBUG)? 5 : (ccu_drv_DbgLogEnable_INFO)? 4 : 4;

    if(ioctl(this->m_Fd, CCU_IOCTL_SET_RUN) < 0){
        LOG_ERR("CCU_IOCTL_SET_RUN: %d\n", this->m_HWmodule);
        return MFALSE;
    }

    //sp.reg.25 refers sram log base address
    m_CcuLogBaseOffset = *(ccuCtrlBasePtr + OFFSET_CCU_SRAM_LOG_BASE);
    LOG_INF_MUST("CCU log base offset: %x\n", m_CcuLogBaseOffset);

    m_shared_buf_map_ptr = (struct shared_buf_map *)(ccuDmemBasePtr + *(ccuCtrlBasePtr + OFFSET_CCU_SHARED_BUF_MAP_BASE));
    LOG_DBG("shared_buf_map_offset: %p\n", *(ccuCtrlBasePtr + OFFSET_CCU_SHARED_BUF_MAP_BASE));
    LOG_DBG("m_shared_buf_map_ptr: %p\n", m_shared_buf_map_ptr);
    LOG_DBG("ipc_in_data_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_in_data_base_offset);
    LOG_DBG("ipc_out_data_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_out_data_base_offset);
    LOG_DBG("ipc_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_base_offset);

    LOG_DBG("ae_algo_data_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_algo_data_addr);
    LOG_DBG("ae_init_data_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_init_data_addr);
    LOG_DBG("ae_vsync_info_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_vsync_info_addr);
    LOG_DBG("n3d_ae_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].n3d_ae_addr);
    LOG_DBG("n3d_output_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].n3d_output_addr);
    LOG_DBG("n3d_init_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].n3d_init_addr);
    LOG_DBG("ae_algo_data_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_algo_data_addr);
    LOG_DBG("ae_init_data_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_init_data_addr);
    LOG_DBG("ae_vsync_info_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_vsync_info_addr);
    LOG_DBG("n3d_ae_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_vsync_info_addr);
    LOG_DBG("n3d_output_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_vsync_info_addr);
    LOG_DBG("n3d_init_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_vsync_info_addr);

    LOG_DBG("-:CCU_IOCTL_SET_RUN\n");
    return MTRUE;
}

MBOOL CcuDrvImp::init(const char* userName, uint32_t log_buf_mva_0, uint32_t log_buf_mva_1, MINT8 *log_buf_va_0, MINT8 *log_buf_va_1, 
    uint32_t ddr_buf_mva, MINT8 *ddr_buf_va)
{
    MBOOL Result = MTRUE;
    MUINT32 strLen;
    MBOOL isPowerOn = MFALSE;

#if defined(CCU_IS_USER_LOAD) || defined(CCU_IS_USER_DEBUG_LOAD)
    DBG_LOG_SET_DEFAULT_LV3();
    LOG_INF_MUST("supress CCU detailed logs in user/userdebug load.");
#endif
    DBG_LOG_CONFIG(drv, ccu_drv);
    //
    android::Mutex::Autolock lock(this->m_CcuInitMutex);
    //
    LOG_DBG("+,m_UserCnt(%d), curUser(%s).\n", this->m_UserCnt,userName);
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
            LOG_DBG(" - X. m_UserCnt: %d,UserName:%s", this->m_UserCnt,userName);
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
    m_ddr_buf_mva = ddr_buf_mva;
    m_ddr_buf_va = ddr_buf_va;
    LOG_DBG("+:pCcuMem\n");
    this->pCcuMem = (CcuMemImp*)CcuMemImp::createInstance();
    if ( NULL == this->pCcuMem ) {
        LOG_ERR("createInstance pCcuMem return %p", pCcuMem);
        return false;
    }
    this->pCcuMem->init("CCU_UDRV");
    LOG_DBG("-:pCcuMem\n");

    //
    CcuAeeMgrDl::CcuAeeMgrDlInit();

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
                gCcuDrvObj[CCU_A].m_pCcuHwRegAddr 		= (MUINT32 *) mmap(0, PAGE_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_HW_BASE - CCU_HW_OFFSET);
                gCcuDrvObj[CCU_CAMSYS].m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_CAMSYS_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_CAMSYS_BASE);
                gCcuDrvObj[CCU_PMEM].m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_PMEM_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_PMEM_BASE);
                gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr 	= (MUINT32 *) mmap(0, CCU_DMEM_SIZE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CCU_DMEM_BASE);

                gCcuDrvObj[CCU_A].m_pCcuHwRegAddr = gCcuDrvObj[CCU_A].m_pCcuHwRegAddr + (CCU_HW_OFFSET/4);
                LOG_DBG("\n");
				LOG_DBG("mmap CCU_A:%p\n",      gCcuDrvObj[CCU_A].m_pCcuHwRegAddr);
				LOG_DBG("mmap CCU_CAMSYS:%p\n", gCcuDrvObj[CCU_CAMSYS].m_pCcuHwRegAddr);
				LOG_DBG("mmap CCU_PMEM:%p\n",   gCcuDrvObj[CCU_PMEM].m_pCcuHwRegAddr);
				LOG_DBG("mmap CCU_DMEM:%p\n",   gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr);

                break;
            default:
                LOG_ERR("#############\n");
                LOG_ERR("this hw module(%d) is unsupported\n",this->m_HWmodule);
                LOG_ERR("#############\n");
                goto EXIT;
                break;
        }

        if(gCcuDrvObj[CCU_A].m_pCcuHwRegAddr == MAP_FAILED)
        {
            LOG_ERR("CCU mmap fail , errno(%d):%s",errno, strerror(errno));
			LOG_ERR("(%x)(%x)\n",PAGE_SIZE,CCU_HW_BASE);
            Result = MFALSE;
            goto EXIT;
        }

        this->m_regRWMode = CCU_DRV_RW_MMAP;
    }

	//power on
    LOG_INF_MUST("+:setCcuPower, logBufMva[0](%x), logBufMva[1](%x), logBufVa[0](%x), logBufVa[1](%x)", 
        log_buf_mva_0, log_buf_mva_1, log_buf_va_0, log_buf_va_1);
    m_power.workBuf.mva_log[0] = log_buf_mva_0;
    m_power.workBuf.mva_log[1] = log_buf_mva_1;
    g_pLogBuf[0] = log_buf_va_0;
    memset((char*)g_pLogBuf[0],0x00,SIZE_1MB);
    *(MUINT32*)(g_pLogBuf[0]+0) = LOG_ENDEND;
    g_pLogBuf[1] = log_buf_va_1;
    memset((char*)g_pLogBuf[1],0x00,SIZE_1MB);
    *(MUINT32*)(g_pLogBuf[1]+0) = LOG_ENDEND;
    if((log_buf_mva_0 == 0) || (log_buf_mva_1 == 0))
    {
        Result = MFALSE;
        goto EXIT;
    }
    if(!this->setCcuPower(1))
    {
        Result = MFALSE;
        goto EXIT;
    }
    else
        isPowerOn = MTRUE;
    LOG_INF("-:setCcuPower");

	//load binary here
	if ( !this->loadCCUBin() ) {
		LOG_ERR("load binary file fail\n");
        Result = MFALSE;
        goto EXIT;
	}

    LOG_DBG("+:create CCU worker thread\n");
    this->createThread();
    LOG_DBG("-:create CCU worker thread\n");

    m_shared_buf_map_ptr = NULL;
    if(!this->setCcuRun())
    {
        PrintReg();
        Result = MFALSE;
        goto EXIT;
    }
    m_ccuDrvIpc = new CcuDrvIpc();
    m_ccuDrvIpc->init(m_shared_buf_map_ptr, gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr, gCcuDrvObj[CCU_A].m_pCcuHwRegAddr);

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


    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pCcuHwRegAddr(0x%x)\n", Result, this->m_UserCnt, gCcuDrvObj[CCU_A].m_pCcuHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL CcuDrvImp::shutdown()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

	//Send MSG_TO_CCU_SHUTDOWN command
    struct ccu_msg msg = {MSG_TO_CCU_SHUTDOWN, NULL, NULL, CCU_CAM_TG_NONE};

  	if(!this->sendCmdIpc(&msg)) {
		LOG_ERR("cmd(%d) fail \n", msg.msg_id);
		return MFALSE;
    }

    LOG_DBG_MUST("-:%s\n", __FUNCTION__);

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

    LOG_DBG_MUST("+:release thread\n");
    if(this->m_taskWorkerThreadReady == MTRUE)
    {
        this->destroyThread();
    }
    LOG_DBG_MUST("-:release thread\n");

    //
    UnMapHwRegs();

    //
    LOG_DBG_MUST("+:release buffers\n");
    if(this->pCcuMem)
    {
	    this->pCcuMem->uninit("CCU_UDRV");
	    this->pCcuMem->destroyInstance();
        this->pCcuMem = NULL;
	}
    LOG_DBG_MUST("+:release buffers\n");

    if(m_ccuDrvIpc != NULL)
    {
        delete m_ccuDrvIpc;
        m_ccuDrvIpc = NULL;
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
                LOG_DBG("current user[%d]: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
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
        if(gCcuDrvObj[i].m_pCcuHwRegAddr != MAP_FAILED && gCcuDrvObj[i].m_pCcuHwRegAddr != NULL)
        {
            int ret = 0;
            switch(i){
                case CCU_A:
                    LOG_DBG_MUST("Uinit_CCU_A munmap: %p\n", gCcuDrvObj[i].m_pCcuHwRegAddr);
                    gCcuDrvObj[i].m_pCcuHwRegAddr = gCcuDrvObj[i].m_pCcuHwRegAddr - (CCU_HW_OFFSET/4);
                    ret = munmap(gCcuDrvObj[i].m_pCcuHwRegAddr, PAGE_SIZE);
                    break;
                case CCU_CAMSYS:
                    LOG_DBG_MUST("Uinit_CCU_CAMSYS munmap: %p\n", gCcuDrvObj[i].m_pCcuHwRegAddr);
                    ret = munmap(gCcuDrvObj[i].m_pCcuHwRegAddr, PAGE_SIZE);
                    break;
                case CCU_PMEM:
                    LOG_DBG_MUST("Uinit_CCU_PMEM munmap: %p\n", gCcuDrvObj[i].m_pCcuHwRegAddr);
                    ret = munmap(gCcuDrvObj[i].m_pCcuHwRegAddr, CCU_PMEM_SIZE);
                    break;
                case CCU_DMEM:
                    LOG_DBG_MUST("Uinit_CCU_DMEM munmap: %p\n", gCcuDrvObj[i].m_pCcuHwRegAddr);
                    ret = munmap(gCcuDrvObj[i].m_pCcuHwRegAddr, CCU_DMEM_SIZE);
                    break;
                default:
                    LOG_ERR("this hw module(%d) is unsupported\n",i);
                    break;
            }

            if (ret < 0)
            {
                LOG_ERR("munmap fail: %p\n", gCcuDrvObj[i].m_pCcuHwRegAddr);
                return MFALSE;
            }

            gCcuDrvObj[i].m_pCcuHwRegAddr = NULL;
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

MBOOL CcuDrvImp::waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq, MUINT32 user_tg)
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
    if(user_tg == CCU_CAM_TG_1)
    {
        wait.bDumpReg = CCU_CAM_TG_1;
        Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_AF_IRQ,&wait);
    }
    else if(user_tg == CCU_CAM_TG_2)
    {
        wait.bDumpReg = CCU_CAM_TG_2;
        Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_AF_IRQ,&wait);
    }
    else
    {
        LOG_ERR("unknow tg_user:0x%x\n",user_tg);
        return MFALSE;
    }

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

MBOOL CcuDrvImp::registerIrq(CCU_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    MINT32 Ret;
    LOG_DBG(" - E. hw module:0x%x,userkey(%d), name(%s)\n",this->m_HWmodule,pRegIrq->userKey, pRegIrq->userName);
    //
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

MBOOL CcuDrvImp::sendCmdIpc(struct ccu_msg *msg)
{
    android::Mutex::Autolock lock(this->m_CcuSendCmdMutex);
    MBOOL result;
    result = m_ccuDrvIpc->sendCmd(msg);
    if(result == MFALSE)
    {
        LOG_ERR("CCU wait cmd ack timeout: msg(%d), tg(%d)\n", msg->msg_id, msg->tg_info);
        LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[0] =============\n");
        LOG_WRN("===== CCU LOG DUMP START =====\n");
        *(MUINT32 *)(g_pLogBuf[0] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
        printCcuLog((const char*)g_pLogBuf[0], MTRUE);
        LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[1] =============\n");
        *(MUINT32 *)(g_pLogBuf[1] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
        printCcuLog((const char*)g_pLogBuf[1], MTRUE);
        LOG_WRN("===== CCU LOG DUMP END =====\n");
        DumpSramLog();
        PrintReg();
    }
    return result;
}

MBOOL CcuDrvImp::getIpcIOBuf(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu)
{
    if(m_ccuDrvIpc == NULL)
    {
        LOG_ERR("[%s]m_ccuDrvIpc invalid., m_ccuDrvIpc(%p)", __FUNCTION__, m_ccuDrvIpc);
        return MFALSE;
    }
    return m_ccuDrvIpc->getIpcIOBufAddr(ipcInDataPtr, ipcOutDataPtr, ipcInDataAddrCcu, ipcOutDataAddrCcu);
}

MBOOL CcuDrvImp::waitCmd( ccu_cmd_st *pCMD_R )
{
	//ccu_cmd_st cmd_get;

	LOG_DBG("CCU_IOCTL_DEQUE_COMMAND\n");
	if(ioctl(this->m_Fd, CCU_IOCTL_DEQUE_COMMAND, pCMD_R) < 0)
	{
		LOG_ERR("ERROR: CCU_IOCTL_DEQUE_COMMAND:%x\n",pCMD_R->task.msg_id);
		return MFALSE;;
	}

	//::memcpy( pCMD, &cmd_get, sizeof(ccu_cmd_st) );

	//LOG_INF("(%d),(%p),(%p),(%x),(%x),(%d), \n",cmd_get.task.msg_id,
	LOG_DBG("CCU_IOCTL_DEQUE_COMMAND result: (%d),(%p),(%p),(%d), \n",pCMD_R->task.msg_id, \
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

    LOG_DBG("open file, path: %s\n", path1);
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
        LOG_DBG("read file into buffer, szTell=%x\n", szTell);
        szRead = fread ( (void*) buf, sizeof(char), szTell, fp );
        LOG_DBG("read file done\n");
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

size_t loadSensorDrvToBuffer(FILE *fp, unsigned int *buf, CCU_BIN_SLOT sensor_slot)
{
    size_t szTell=0, szRead=0;
    size_t szBin = 0;
    struct stat stbuf;
    int fstat_res;
    fstat_res = fstat(fileno(fp), &stbuf);
    
    if(fstat_res != 0)
    {
        LOG_ERR("File stat error: %s", strerror(errno));
        return 0;
    }
    
    szTell = stbuf.st_size;
    // fseek(fp, 0L, SEEK_END);
    // szTell = ftell(fp);
    LOG_DBG("SLOT = %d",sensor_slot);

    szBin = CCU_SENSOR_BIN_SIZE;

    if(szTell > (szBin * 2))
    {
        LOG_ERR("file size > %x, szTell=%x\n", szTell, szBin);
    }
    else if(szTell == (szBin * 2))
    {
        rewind(fp);
        LOG_DBG("read file into buffer, szTell=%x\n", szTell);
        LOG_DBG("Before seek=%x\n", ftell(fp));
        if(sensor_slot == BIN_SENSOR_BIN_2)
            fseek(fp, szBin, SEEK_SET);
        LOG_DBG("After seek=%x\n", ftell(fp));
        szRead = fread ( (void*) buf, sizeof(char), szBin, fp );
        LOG_DBG("read file done\n");
    }
    else
    {
        LOG_ERR("file size <= 0, szTell=%x\n", szTell);
    }

    if(szRead != szBin)
    {
        LOG_ERR("read file error: szRead=%x, szBin=%x\n", szRead, szBin);
    }

    return szRead;
}

void clearAndLoadBinToMem(unsigned int *memBaseAddr, unsigned int *buf, unsigned int memSize, unsigned int binSize, unsigned int offset)
{
    volatile int dummy = 0;
    LOG_DBG("clear MEM");
    LOG_DBG("args: %p, %p, %x, %x, %x", memBaseAddr, buf, memSize, binSize, offset);
    /*Must clear mem with 4 byte aligned access, due to APMCU->CCU access path APB restriction*/
    for (unsigned int i=0;i<memSize/4;i++)
    {
        //for HW Test tool, must add a dummy operation between mem access, otherwise incurrs SIGBUS/BUS_ADRALN
        //root casue under checking...
        dummy = dummy + 1;
        *(memBaseAddr + i) = 0;
    }

    LOG_DBG("load bin buffer onto MEM");
    LOG_DBG("%x,%x,%x\n",buf[0],buf[1],buf[2]);

    for (unsigned int i=0;i<binSize/4;i++) 
    {
        *(memBaseAddr + (offset/4) + i) = buf[i];
    }
}

MBOOL loadBin(char *binPath, unsigned int *memBaseAddr, unsigned int *buf, unsigned int memSize, unsigned int offset, CCU_BIN_SLOT sensor_slot)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    FILE *pFile;
    size_t binSize;

    LOG_DBG("open Bin file, path: %s\n", binPath);
    pFile = tryOpenFile(binPath);
    LOG_DBG("open Bin file result:%d\n", pFile);
    if (pFile == NULL)
    {
        LOG_ERR("Open Bin file fail\n");
        return MFALSE;
    }

    LOG_DBG("read Bin file into Bin buffer\n");
    if(sensor_slot == BIN_CCU) //CCU DM/PM
        binSize = loadFileToBuffer(pFile, buf);
    else
        binSize = loadSensorDrvToBuffer(pFile, buf, sensor_slot);
    fclose(pFile);
    LOG_DBG("read Bin done\n");

    LOG_DBG("clear MEM & load Bin buffer onto MEM\n");
    clearAndLoadBinToMem(memBaseAddr, buf, memSize, binSize, offset);

    LOG_DBG("-:%s\n",__FUNCTION__);    

    return MTRUE;
}

static unsigned int pmemBuf[CCU_PMEM_SIZE];
static unsigned int dmemBuf[CCU_DMEM_SIZE];
static unsigned int ddrBuf[CCU_CACHE_SIZE];
MBOOL CcuDrvImp::loadCCUBin()
{
    LOG_INF_MUST("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    
    volatile unsigned int status;

    char *pmPath = "/system/vendor/bin/lib3a.ccu.pm";
    char *dmPath = "/system/vendor/bin/lib3a.ccu.dm";
    char *ddrPath = "/system/vendor/bin/lib3a.ccu.ddr";

    volatile MUINT32 *ccuCtrlBasePtr = (MUINT32 *)gCcuDrvObj[CCU_A].m_pCcuHwRegAddr;

    *(ccuCtrlBasePtr) = 0x0;
    LOG_INF_MUST("Wait halt");
    do
    {
        status = *(ccuCtrlBasePtr + 0x28/4);
    }while(!(status&0x100));
    //========================= Load PMEM binary ===========================
    LOG_INF_MUST("Load PM");
    ret = loadBin(pmPath, gCcuDrvObj[CCU_PMEM].m_pCcuHwRegAddr, pmemBuf, CCU_PMEM_SIZE, 0, BIN_CCU);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

    //========================= Load DMEM binary ===========================
    LOG_INF_MUST("Load DM");
    ret = loadBin(dmPath, gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr, dmemBuf, CCU_DMEM_SIZE, CCU_DMEM_OFFSET, BIN_CCU);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

    //========================= Load DMEM binary ===========================
    LOG_INF_MUST("Load DDR");
    ret = loadBin(ddrPath, (unsigned int*)m_ddr_buf_va, ddrBuf, CCU_CACHE_SIZE, 0, BIN_CCU);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

LOAD_CCU_BIN_EXIT:
    LOG_INF_MUST("-:%s\n",__FUNCTION__);
	return ret;
}

MBOOL CcuDrvImp::loadSensorBin(uint32_t sensor_idx, uint8_t sensor_slot, bool *isSpSensor)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    unsigned int sensorBinBuf[CCU_SENSOR_BIN_SIZE];
    char *sensorPathPrefix = "/system/vendor/bin/libccu_";
    char sensorName[100] = "";
    char sensorBinPath[100] = "";

    //========================= Get sensor name via ioctl ===========================
    this->GetSensorName(this->m_Fd, sensor_idx, sensorName);
    LOG_DBG_MUST("ccu sensor name: %s\n", sensorName);

    //if the senosr is s5k3p9, mark it on indicator
    //due to the samsung sensor defect, CCU must by pass exposure setting while performing long exposure
    //long expossure only works when set and trigger i2c to sensor in APMCU side
    if((strncmp(sensorName, "s5k3p9sx", 8) == 0) && (sensor_slot == 1))
    {
        LOG_INF_MUST("detected s5k3p9sx");
        *isSpSensor = true;
    }
    else if((strncmp(sensorName, "s5k3l6", 6) == 0) && (sensor_slot == 1))
    {
        //for s5k3l6, also bypass CCU long-exposure settings
        LOG_INF_MUST("detected s5k3l6");
        *isSpSensor = true;
    }
    else
    {
        LOG_INF_MUST("no sp sensor detected");
    }

    strncat(sensorBinPath, sensorPathPrefix, 50);
    strncat(sensorBinPath, sensorName, 45);
    strncat(sensorBinPath, ".ddr", 5);

    //========================= Load Sensor DM binary ===========================
    LOG_DBG("Load Sensor DM");
    if ((sensor_slot == 1))
        ret = loadBin(sensorBinPath, (unsigned int *)m_ddr_buf_va, sensorBinBuf, 0, SENSOR_BIN_OFFSET_SLOT_1, BIN_SENSOR_BIN_1);
    else if ((sensor_slot == 2))
        ret = loadBin(sensorBinPath, (unsigned int *)m_ddr_buf_va, sensorBinBuf, 0, SENSOR_BIN_OFFSET_SLOT_2, BIN_SENSOR_BIN_2);
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
    char sensorBinPath[100] = "";
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

    strncat(sensorBinPath, sensorPathPrefix, 50);
    strncat(sensorBinPath, sensorName, 45);
    strncat(sensorBinPath, ".pm", 5);

    strncat(sensorDmPath, sensorPathPrefix, 50);
    strncat(sensorDmPath, sensorName, 45);
    strncat(sensorDmPath, ".dm", 5);

    //========================= Try Open Sensor PMEM binary ===========================
    LOG_DBG("open sensorPm file, path: %s\n", sensorBinPath);
    pFile = tryOpenFile(sensorBinPath);
    LOG_DBG("open sensorPm file result:%d\n", pFile);
    if (pFile == NULL) { LOG_ERR("Open sensorPm fail\n"); goto FAIL_EXIT; }

    fclose(pFile);
    LOG_DBG("read sensorDm done\n");

    //========================= Try Open Sensor DMEM binary ===========================
    LOG_DBG("open sensorDm file, path: %s\n", sensorDmPath);
    pFile = tryOpenFile(sensorDmPath);
    LOG_DBG("open sensorDm file result:%d\n", pFile);
    if (pFile == NULL) { LOG_ERR("Open sensorDm fail\n"); goto FAIL_EXIT; }

    fclose(pFile);
    LOG_DBG("read sensorDm done\n");

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
    MUINT32 sensor_nm_idx = CcuDrvUtil::sensorIdxToNonmapIdx(sensor_idx);

    if(ioctl(kdrvFd, CCU_IOCTL_GET_SENSOR_NAME, sensorNames) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_SENSOR_NAME\n");
        goto FAIL_EXIT;
    }

    for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; ++i)
    {
        LOG_DBG_MUST("ccu sensor %d name: %s\n", i, sensorNames[i]);

    }

    if (sensor_nm_idx >= IMGSENSOR_SENSOR_IDX_MIN_NUM && sensor_nm_idx < IMGSENSOR_SENSOR_IDX_MAX_NUM)
    {
        memcpy(sensorName, sensorNames[sensor_nm_idx], strlen(sensorNames[sensor_nm_idx])+1);
    }
    else
    {
        LOG_ERR("_getSensorName error, invalid sensoridx: %x\n", sensor_nm_idx);
        goto FAIL_EXIT;
    }

    #undef SENSOR_NAME_MAX_LEN

    return MTRUE;

FAIL_EXIT:
    return MFALSE;
}

MBOOL CcuDrvImp::getI2CDmaBufferAddr(struct ccu_i2c_buf_mva_ioarg *ioarg)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_GET_I2C_DMA_BUF_ADDR, ioarg) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_I2C_DMA_BUF_ADDR\n");
        return MFALSE;
    }

    return MTRUE;
}


MBOOL CcuDrvImp::initI2cController(uint32_t i2c_id)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_SET_I2C_MODE, i2c_id) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_SET_I2C_MODE\n");
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
	pthread_create(&mThread, NULL, ApTaskWorkerThreadLoop, this);
    pthread_setname_np(mThread, "ccu_worker");

    LOG_DBG("ApTaskWorkerThreadLoop created, wait for m_taskWorkerThreadReady\n");
    LOG_DBG("m_taskWorkerThreadReady addr: %p\n", &(this->m_taskWorkerThreadReady));
    while(this->m_taskWorkerThreadReady != MTRUE){}
    LOG_DBG("m_taskWorkerThreadReady: %d\n", this->m_taskWorkerThreadReady);

    LOG_DBG("-\n");
}
int CcuDrvImp::dequeueAFO_A()
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
    else if(ret1==(void *)5)
        return 5;
    else
        return 0;

}

int CcuDrvImp::dequeueAFO_B()
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
    else if(ret1==(void *)5)
        return 5;
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

static int js_cnt = -1;
void CcuDrvImp::PrintReg()
{
    MUINT32 *ccuCtrlBasePtr = (MUINT32 *)gCcuDrvObj[CCU_A].m_pCcuHwRegAddr;
    MUINT32 *ccuCtrlPtr = (MUINT32 *)gCcuDrvObj[CCU_A].m_pCcuHwRegAddr;
    MUINT32 *ccuDmPtr = (MUINT32 *)gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr;
    MUINT32 *ccuPmPtr = (MUINT32 *)gCcuDrvObj[CCU_PMEM].m_pCcuHwRegAddr;

    LOG_WRN("=============== CCU REG DUMP START ===============\n");
    for(int i=0 ; i<CCU_HW_DUMP_SIZE ; i += 16)
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
/*
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
    */
}

static MUINT32 logBuf_1[CCU_LOG_SIZE/4];
static MUINT32 logBuf_2[CCU_LOG_SIZE/4];
static MUINT32 isrlogBuf[CCU_ISR_LOG_SIZE/4];
void CcuDrvImp::DumpSramLog()
{
    LOG_WRN("=============== DumpSramLog ===============\n");
    char *ccuCtrlBasePtr = (char *)gCcuDrvObj[CCU_DMEM].m_pCcuHwRegAddr;
    char *ccuLogPtr_1 = ccuCtrlBasePtr + m_CcuLogBaseOffset;
    char *ccuLogPtr_2 = ccuCtrlBasePtr + m_CcuLogBaseOffset + CCU_LOG_SIZE;
    char *isrLogPtr = ccuCtrlBasePtr + m_CcuLogBaseOffset + (CCU_LOG_SIZE * 2);
    
    LOG_WRN("ccuCtrlBasePtr(%x)\n", ccuCtrlBasePtr);
    LOG_WRN("ccuLogPtr_1(%x)\n", ccuCtrlBasePtr);
    LOG_WRN("ccuLogPtr_2(%x)\n", ccuCtrlBasePtr);
    LOG_WRN("isrLogPtr(%x)\n", ccuCtrlBasePtr);
    LOG_WRN("m_CcuLogBaseOffset(%x)\n", m_CcuLogBaseOffset);
    LOG_WRN("CCU_LOG_SIZE(%d)\n", CCU_LOG_SIZE);

    MUINT32 *from_sram;
    MUINT32 *to_dram;

    from_sram = (MUINT32 *)ccuLogPtr_1;
    to_dram = (MUINT32 *)logBuf_1;

    LOG_WRN("ccuLogPtr_1(%x)\n", from_sram);
    LOG_WRN("logBuf_1(%x)\n", to_dram);
    LOG_WRN("(CCU_LOG_SIZE/4)(%d)\n", (CCU_LOG_SIZE/4));
    
    for(int i = 0; i<(CCU_LOG_SIZE/4); i++)
    {
        if(i==0)
            LOG_WRN("i(%d) to_dram+i(%x) from_sram+i(%x)\n", i, (to_dram+i), (from_sram+i));
        *(to_dram+i) = *(from_sram+i);
    }
    from_sram = (MUINT32 *)ccuLogPtr_2;
    to_dram = (MUINT32 *)logBuf_2;
    for(int i = 0; i<(CCU_LOG_SIZE/4); i++)
    {
        if(i==0)
            LOG_WRN("i(%d) to_dram+i(%x) from_sram+i(%x)\n", i, (to_dram+i), (from_sram+i));
        *(to_dram+i) = *(from_sram+i);
    }
    from_sram = (MUINT32 *)isrLogPtr;
    to_dram = (MUINT32 *)isrlogBuf;
    for(int i = 0; i<(CCU_ISR_LOG_SIZE/4); i++)
    {
        if(i==0)
            LOG_WRN("i(%d) to_dram+i(%x) from_sram+i(%x)\n", i, (to_dram+i), (from_sram+i));
        *(to_dram+i) = *(from_sram+i);
    }

    LOG_WRN("=============== CCU INTERNAL LOG DUMP START ===============\n");

    logBuf_1[CCU_LOG_SIZE/4 -1] = LOG_ENDEND;
    logBuf_2[CCU_LOG_SIZE/4 -1] = LOG_ENDEND;
    isrlogBuf[CCU_ISR_LOG_SIZE/4 -1] = LOG_ENDEND;

    LOG_WRN("---------CHUNK 1-----------\n");
    printCcuLog((char *)logBuf_1, MTRUE);
    LOG_WRN("---------CHUNK 2-----------\n");
    printCcuLog((char *)logBuf_2, MTRUE);
    LOG_WRN("---------ISR LOG-----------\n");
    printCcuLog((char *)isrlogBuf, MTRUE);
    
    LOG_WRN("=============== CCU INTERNAL LOG DUMP END ===============\n");
}

void CcuDrvImp::DumpDramLog()
{
    LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
    *(MUINT32 *)(g_pLogBuf[0] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
    printCcuLog((const char*)g_pLogBuf[0], MTRUE);
    LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
    *(MUINT32 *)(g_pLogBuf[1] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
    printCcuLog((const char*)g_pLogBuf[1], MTRUE);
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
        ret = _this->waitAFIrq(&_irq, CCU_CAM_TG_1);


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
            else if( 5 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_VRB("AF abort task: %d\n", _irq.TimeInfo.passedbySigcnt);
                return((void *)5);
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
        ret = _this->waitAFIrq(&_irq, CCU_CAM_TG_2);


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
            else if( 5 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_VRB("AF abort task: %d\n", _irq.TimeInfo.passedbySigcnt);
                return((void *)5);
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

        UTIL_TRACE_BEGIN("CcuHardworking");

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
                //LOG_INF("===== CCU LOG DUMP START =====\n");
                *(MUINT32 *)(g_pLogBuf[0] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[0]);
                //_this->DumpSramLog();
                //LOG_INF("===== CCU LOG DUMP END =====\n");
                //_this->PrintReg();
            }
            else if ( 2 == _irq.TimeInfo.passedbySigcnt )
            {
                //LOG_INF("===== CCU LOG DUMP START =====\n");
                *(MUINT32 *)(g_pLogBuf[1] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[1]);
                //_this->DumpSramLog();
                //LOG_INF("===== CCU LOG DUMP END =====\n");
            }
            else if ( -1 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[0] =============\n");
                LOG_WRN("===== CCU LOG DUMP START =====\n");
                *(MUINT32 *)(g_pLogBuf[0] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[1] =============\n");
                *(MUINT32 *)(g_pLogBuf[1] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
                LOG_WRN("===== CCU LOG DUMP END =====\n");
                _this->DumpSramLog();
                _this->PrintReg();
                AEE_ASSERT_CCU_USER("CCU ASSERT\n");
            }
            else if ( -2 == _irq.TimeInfo.passedbySigcnt )
            {
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
                *(MUINT32 *)(g_pLogBuf[0] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[0], MTRUE);
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
                *(MUINT32 *)(g_pLogBuf[1] + LOG_CHECK_POINT - 4) = LOG_ENDEND;
                printCcuLog((const char*)g_pLogBuf[1], MTRUE);
            }
            else if(0x00aabb00 == _irq.TimeInfo.passedbySigcnt)
            {
                LOG_VRB("no irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
            else
            {
                LOG_ERR("unknow irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
            }
        }

        UTIL_TRACE_END();

	}while(1);

	LOG_DBG("-:\n");

	return NULL;
}

void CcuDrvImp::TriggerAee(char *msg)
{
    AEE_ASSERT_CCU_USER(msg);
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
MBOOL CcuMemImp::mmapMVA( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva, MUINT32 upper_bound, MUINT32 lower_bound)
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
    mm_data.config_buffer_param.reserve_iova_start  = lower_bound;
    mm_data.config_buffer_param.reserve_iova_end    = upper_bound;
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








