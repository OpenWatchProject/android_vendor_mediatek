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

//#include <mtkcam/def/common.h>
#include <string.h>
#include <ccu_udrv_stddef.h>
#include <ccu_udrv.h>
#include <ccu_drvutil.h>

using namespace NSCcuIf;

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "ccu_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(ccu_drv);

//constant definitions
#define CCU_DRV_DEV_NAME            "/dev/ccu"
#define OFFSET_CCU_A_INFO00      0x80//ccu log enable statue
#define OFFSET_CCU_A_INFO20      0xD0//ccu log enable statue
#define CCU_H2X_MMU_ENABLE_REG   0x2C//ccu Cache to EMI control reg
#define MMU_ENABLE_BIT           0x33//ccu Cache to EMI enable
#define OFFSET_CCU_A_INFO21      0xD4//ccu log mask
#define OFFSET_CCU_A_INFO22      0xD8//cpu ref buf align mva

//CCU log parser definitions
#define LOG_NORMAL_HEADER 0xCCCC
#define LOG_MUST_HEADER 0xBBBB
#define LOG_ENDEND 0xDDDDDDDD

//Singleton definition
static CcuDrvImp gCcuDrvObj;

static void _get_log_taglevel(MUINT32 *log_taglevel);

/*******************************************************************************
* Public Functions
********************************************************************************/
CcuDrvImp::CcuDrvImp()
{
    DBG_LOG_CONFIG(drv, ccu_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_Fd = -1;
    m_pHwRegAddr[CCU_A] = NULL;
    for (int i = 0; i < CCU_SENSOR_BIN_CAPACITY; ++i)
    {
        m_CcuSensorSlot[i] = IMGSENSOR_SENSOR_IDX_NONE;
    }    
}

CcuDrvImp* CcuDrvImp::createInstance()
{
    return &gCcuDrvObj;
}

MBOOL CcuDrvImp::importCcuMemToKernel(import_mem_t mempool)
{
    uint32_t ret;
    if(ret = ioctl(this->m_Fd, CCU_IOCTL_IMPORT_MEM, &mempool) < 0)
    {
        LOG_ERR("CCU_IOCTL_IMPORT_MEM:(%d)\n", ret);
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
    MUINT8 *ccuDmemBasePtr = (MUINT8 *)m_pHwRegAddr[CCU_DMEM];

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
        LOG_ERR("ccuAddr not in DDR nor in DCCM, ccuAddr(%x), CCU_DCCM_REMAP_BASE(%x), CCU_CACHE_BASE(%x)", 
            ccuAddr, CCU_DCCM_REMAP_BASE, CCU_CACHE_BASE);
    }

    LOG_DBG("dmembase(%p), ddrBase(%p), ccuAddr(%x), offset(%x), vaOut(%p)", ccuDmemBasePtr, m_ddr_buf_va, ccuAddr, ccuAddrOffset, vaOut);

    return vaOut;
}

MBOOL CcuDrvImp::init(struct CcuDrvInitData initData)
{
    MBOOL ret = MTRUE;
    MUINT8 *ccuDmemBasePtr = NULL;

#if defined(CCU_IS_USER_LOAD)
    DBG_LOG_SET_IS_USER_LOAD();
    DBG_LOG_SET_DEFAULT_LV3();
    LOG_INF_MUST("supress CCU detailed logs in user load.");
#endif
    DBG_LOG_CONFIG(drv, ccu_drv);

    android::Mutex::Autolock lock(this->m_CcuInitMutex);
    
    //setup log/ddr buffer address
    LOG_INF_MUST("+: ddr_buf_mva(0x%x), ddr_buf_va(%p)", initData.ddr_buf_mva, initData.ddr_buf_va);
    LOG_INF_MUST("+: logBufMva[0](0x%x), logBufMva[1](0x%x), logBufVa[0](%p), logBufVa[1](%p)", 
        initData.log_buf_mva_0, initData.log_buf_mva_1, initData.log_buf_va_0, initData.log_buf_va_1);
    if((initData.log_buf_mva_0 == 0) || (initData.log_buf_mva_1 == 0) || (initData.ddr_buf_mva == 0) || (initData.ddr_buf_va == 0))
    {
        LOG_ERR("buffer address violation");
        ret = MFALSE;
        goto EXIT;
    }
    LOG_INF_MUST("+: cpu_ref_buf_mva(0x%x)", initData.cpu_ref_buf_mva);

    m_ddr_buf_mva = initData.ddr_buf_mva;
    m_ddr_buf_va = initData.ddr_buf_va;
    m_pLogBuf[0] = initData.log_buf_va_0;
    m_pLogBuf[1] = initData.log_buf_va_1;

    LOG_INF_MUST("+: logBufMva[0](0x%x), logBufMva[1](0x%x), log_buf_va_0(%p), log_buf_va_1(%p)", 
        m_pLogBuf[0], m_pLogBuf[1], initData.log_buf_va_0, initData.log_buf_va_1);

    m_power.workBuf.mva_log[0] = initData.log_buf_mva_0;
    m_power.workBuf.mva_log[1] = initData.log_buf_mva_1;
    m_CpuRefBufMva = initData.cpu_ref_buf_mva;
    //
    CcuAeeMgrDl::CcuAeeMgrDlInit();

    // Open ccu device
    if (!(_openCcuKdrv()))
    {
        ret = MFALSE;
        goto EXIT;
    }
    LOG_DBG_MUST("CCU kernel open ok, fd:%d.", this->m_Fd);

    //mmap HW regs
    if(!_mapHwRegs())
    {
        ret = MFALSE;
        goto EXIT;
    }

    //setup shared buffer map pointer
    ccuDmemBasePtr = (MUINT8 *)m_pHwRegAddr[CCU_DMEM];
    m_shared_buf_map_ptr = (struct shared_buf_map *)(ccuDmemBasePtr + OFFSET_CCU_SHARED_BUF_MAP_BASE);
    LOG_DBG("shared_buf_map_offset: %p\n", OFFSET_CCU_SHARED_BUF_MAP_BASE);
    LOG_DBG("m_shared_buf_map_ptr: %p\n", m_shared_buf_map_ptr);

    m_CcuBkDataMva = initData.bkdata_buf_mva;

EXIT:
    if (!ret)    // If some init step goes wrong.
    {
        this->_realUninit();
    }


    LOG_DBG("-,ret: %d. m_pHwRegAddr[CCU_A](0x%x)\n", ret, m_pHwRegAddr[CCU_A]);
    return ret;
}

MBOOL CcuDrvImp::boot()
{
    MBOOL ret = MTRUE;
    isPowerOn = MFALSE;

    //reset log buffers
    memset((char*)m_pLogBuf[0], 0x00, SIZE_1MB);
    *(MUINT32*)(m_pLogBuf[0] + 0) = LOG_ENDEND;
    
    memset((char*)m_pLogBuf[1], 0x00, SIZE_1MB);
    *(MUINT32*)(m_pLogBuf[1] + 0) = LOG_ENDEND;

    //create worker thread
    _createCcuWorkerThread();

    //power on CCU
    if(!this->_setCcuPower(1))
    {
        isPowerOn = MFALSE;
        ret = MFALSE;
        goto EXIT;
    }
    isPowerOn = MTRUE;

    //load binary here, only binary of CCU SRAM is loaded
    if ( !this->loadCcuBin() ) {
        LOG_ERR("load binary file fail\n");
        ret = MFALSE;
        goto EXIT;
    }

    //set CCU run
    if(!this->_setCcuRun())
    {
        PrintReg();
        ret = MFALSE;
        goto EXIT;
    }

    //init IPC
    _createCcuDrvIpc();

EXIT:
    if ((!ret) && isPowerOn)    // If boot failed and power is on.
    {
        this->_onlypowerOff();
    }

    return ret;
}

bool CcuDrvImp::_ccuSwHwReset()
{
    double ts_start;
    double ts_end;
    double duration = 0;

    volatile uint32_t *ccuResetRegPtr = (uint32_t *)m_pHwRegAddr[CCU_A];
    volatile uint32_t *ccuStatusRegPtr = ((uint32_t *)m_pHwRegAddr[CCU_A]) + (OFFSET_CCU_STAT/4);

    //check halt is up
    LOG_INF_MUST("[%s] polling CCU halt(0x%08x)\n", __FUNCTION__, *ccuStatusRegPtr);
    duration = 0;
    ts_start = CcuDrvUtil::getTimeUs();
    while((*ccuStatusRegPtr & 0x100) != 0x100)
    {
        ts_end = CcuDrvUtil::getTimeUs();
        duration = CcuDrvUtil::diffTimeUs(ts_start, ts_end);
        if(duration > 1000)
        {
            LOG_ERR("[%s] polling CCU halt, 1ms timout: (0x%08x)\n", __FUNCTION__, *ccuStatusRegPtr);
            break;
        }
    }
    LOG_INF_MUST("[%s] polling CCU halt done(0x%08x)\n", __FUNCTION__, *ccuStatusRegPtr);

    //do SW reset
    LOG_INF_MUST("[%s] CCU SW reset: before(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    *ccuResetRegPtr = *ccuResetRegPtr | 0x700;
    LOG_INF_MUST("[%s] CCU SW reset: after(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);

    LOG_INF_MUST("[%s] polling CCU SW reset(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    duration = 0;
    ts_start = CcuDrvUtil::getTimeUs();
    while((*ccuResetRegPtr & 0x7) != 0x7)
    {
        ts_end = CcuDrvUtil::getTimeUs();
        duration = CcuDrvUtil::diffTimeUs(ts_start, ts_end);
        if(duration > 1000)
        {
            LOG_ERR("[%s] polling CCU SW reset, 1ms timout: (0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
            break;
        }
    }
    LOG_INF_MUST("[%s] polling CCU SW reset done(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    LOG_INF_MUST("[%s] release CCU SW reset: before(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    *ccuResetRegPtr = *ccuResetRegPtr & (~0x700);
    LOG_INF_MUST("[%s] release CCU SW reset: after(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);

    //do HW reset
    LOG_INF_MUST("[%s] CCU HW reset: before(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    *ccuResetRegPtr = *ccuResetRegPtr | 0xFF0000;
    LOG_INF_MUST("[%s] CCU HW reset: after(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
   
    LOG_INF_MUST("[%s] release CCU HW reset: before(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);
    *ccuResetRegPtr = *ccuResetRegPtr & (~0xFF0000);
    LOG_INF_MUST("[%s] release CCU HW reset: after(0x%08x)\n", __FUNCTION__, *ccuResetRegPtr);

    return true;
}

MBOOL CcuDrvImp::shutdown()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);
    isPowerOn = false;
	//Send MSG_TO_CCU_SHUTDOWN command
    struct ccu_msg msg = {CCU_FEATURE_SYSCTRL, MSG_TO_CCU_SHUTDOWN, NULL, NULL, CCU_CAM_TG_NONE};

  	if(!this->sendCmdIpc(&msg)) {
		LOG_ERR("cmd(%d) fail \n", msg.msg_id);
		return MFALSE;
    }

    _destroyCcuDrvIpc();    

    if(this->m_taskWorkerThreadReady == MTRUE)
    {
        this->_destroyCcuWorkerThread();
    }

    //do CCU HW reset flow before clock disable
    _ccuSwHwReset();

    LOG_DBG_MUST("-:%s\n", __FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::powerOff()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    //power off
    if(!this->_setCcuPower(0))
    {
        return MFALSE;
    }

    LOG_DBG_MUST("-:%s\n",__FUNCTION__);

    return MTRUE;
}

bool CcuDrvImp::_mapHwRegs()
{
    uint32_t size, addr;
    bool ret = true;

    LOG_DBG("Mmap HW Addrs.");

    /* MUST: HW address should be 4K alignment, otherwise mmap got error(22):invalid argument!! */
    for(int i=0 ; i<MAX_CCU_HW_MODULE ; i++)
    {
        switch(i)
        {
            case CCU_A:
                addr = CCU_HW_BASE - CCU_HW_OFFSET;
                size = PAGE_SIZE;
                break;
            case CCU_CAMSYS:
                addr = CCU_CAMSYS_BASE;
                size = CCU_CAMSYS_SIZE;
                break;
            case CCU_PMEM:
                addr = CCU_PMEM_BASE;
                size = CCU_PMEM_SIZE;
                break;
            case CCU_DMEM:
                addr = CCU_DMEM_BASE;
                size = CCU_DMEM_SIZE;
                break;
            default:
                LOG_ERR("this hw module(%d) is unsupported\n",i);
                break;
        }

        LOG_DBG_MUST("mapping module(%d), addr(0x%x), size(0x%x)", i, addr, size);
        m_pHwRegAddr[i] = (MUINT32 *) mmap(0, size, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, addr);

        if(m_pHwRegAddr[i] == MAP_FAILED)
        {
            LOG_ERR("mmap fail , errno(%d):%s", errno, strerror(errno));
            ret = false;
        }
    }

    m_pHwRegAddr[CCU_A] = m_pHwRegAddr[CCU_A] + (CCU_HW_OFFSET/4);
    LOG_DBG("\n");
    LOG_DBG("mmap CCU addr:%p\n", m_pHwRegAddr[CCU_A]);
    LOG_DBG("mmap CAMSYS addr:%p\n", m_pHwRegAddr[CCU_CAMSYS]);
    LOG_DBG("mmap PMEM addr:%p\n", m_pHwRegAddr[CCU_PMEM]);
    LOG_DBG("mmap DMEM addr:%p\n", m_pHwRegAddr[CCU_DMEM]);

    return ret;
}

MBOOL CcuDrvImp::uninit()
{
    android::Mutex::Autolock lock(this->m_CcuInitMutex);

    return this->_realUninit();
}

MUINT32 CcuDrvImp::readInfoReg(MUINT32 regNo)
{
    MINT32 Ret;

    Ret = *(m_pHwRegAddr[CCU_A] + ((OFFSET_CCU_A_INFO00)/4 + regNo));

    return Ret;
}

MBOOL CcuDrvImp::sendCmdIpc(struct ccu_msg *msg)
{
    android::Mutex::Autolock lock(this->m_CcuSendCmdMutex);
    MBOOL result;

    //since CCU might be shutdown while CcuCtrlXX sending command, and m_ccuDrvIpc will be destroyed
    //thus incurrs segmentation fault in this function while using m_ccuDrvIpc
    //here m_ccuDrvIpc should be checked first
    //and functions of create/destroy m_ccuDrvIpc should be put into critical region with m_CcuSendCmdMutex
    if(m_ccuDrvIpc == NULL)
    {
        LOG_ERR("[%s] CcuDrvIpc alreay destroyed, skip \n", __FUNCTION__);
        return MFALSE;
    }

    result = m_ccuDrvIpc->sendCmd(msg);
    if(result == MFALSE)
    {
        LOG_ERR("[%s] CCU send cmd fail: ftype(%d), msg(%d), tg(%d)\n", 
            __FUNCTION__, msg->feature_type, msg->msg_id, msg->tg_info);
        LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[0] =============\n");
        LOG_WRN("===== CCU LOG DUMP START =====\n");
        PrintCcuLogByIdx(0, MTRUE);
        LOG_WRN("============ CCU TIMEOUT LOG DUMP: LOGBUF[1] =============\n");
        PrintCcuLogByIdx(1, MTRUE);
        LOG_WRN("===== CCU LOG DUMP END =====\n");
        DumpSramLog();
        PrintReg();
    }
    return result;
}

MBOOL CcuDrvImp::getIpcIOBuf(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu)
{
    //LOG_DBG_MUST("RCDBG: [%s] m_ccuDrvIpc(%p)", __FUNCTION__, m_ccuDrvIpc);
    if(m_ccuDrvIpc == NULL)
    {
        LOG_ERR("[%s] CcuDrvIpc alreay destroyed, skip \n", __FUNCTION__);
        return MFALSE;
    }
    return m_ccuDrvIpc->getIpcIOBufAddr(ipcInDataPtr, ipcOutDataPtr, ipcInDataAddrCcu, ipcOutDataAddrCcu);
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
    long longTell;

    fseek(fp, 0L, SEEK_END);
    longTell = ftell(fp);

    if (0 < longTell) szTell = (size_t)longTell;

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

size_t loadDrvToBuffer(FILE *fp, unsigned int *buf, CCU_BIN_SLOT bin_slot)
{
    size_t szTell=0, szRead=0;
    size_t szBin = 0;
    struct stat stbuf;
    int fstat_res;
    int ret=0;

    fstat_res = fstat(fileno(fp), &stbuf);
    
    if(fstat_res != 0)
    {
        LOG_ERR("File stat error: %s", strerror(errno));
        return 0;
    }
    
    szTell = stbuf.st_size;
    // fseek(fp, 0L, SEEK_END);
    // szTell = ftell(fp);
    LOG_DBG("SLOT = %d",bin_slot);

    szBin = CCU_SENSOR_BIN_SIZE;

    if(szTell > (szBin * 2))
    {
        LOG_ERR("file size > %x, szTell=%x\n", szTell, szBin);
    }
    else if(bin_slot == BIN_CUSTFUNC_BIN)
    {
        rewind(fp);
        LOG_DBG("read file into buffer, szTell=%x\n", szTell);
        LOG_DBG("Before seek=%x\n", ftell(fp));
        szRead = fread ( (void*) buf, sizeof(char), szBin, fp );
        LOG_DBG("read file done\n");
    } 
    else if(szTell == (szBin * 2))
    {
        rewind(fp);
        LOG_DBG("read file into buffer, szTell=%x\n", szTell);
        LOG_DBG("Before seek=%x\n", ftell(fp));
        if(bin_slot == BIN_SENSOR_BIN_2||bin_slot == BIN_LENS_BIN_2||bin_slot == BIN_CUSTSENSOR_BIN_2)
            ret = fseek(fp, szBin, SEEK_SET);
        if (ret == 0) {
            LOG_DBG("After seek=%x\n", ftell(fp));
            szRead = fread ( (void*) buf, sizeof(char), szBin, fp );
            LOG_DBG("read file done\n");
        }
        else {
            LOG_ERR("file seek error: %x(%d)\n", szBin, errno);
        }
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
    LOG_DBG_MUST("args: %p, %p, %x, %x, %x", memBaseAddr, buf, memSize, binSize, offset);
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

MBOOL loadBin(char *binPath, unsigned int *memBaseAddr, unsigned int *buf, unsigned int memSize, unsigned int offset, CCU_BIN_SLOT bin_slot, unsigned int loadToccu)
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
    if(bin_slot == BIN_CCU) //CCU DM/PM
        binSize = loadFileToBuffer(pFile, buf);
    else
        binSize = loadDrvToBuffer(pFile, buf, bin_slot);
    fclose(pFile);
    LOG_DBG("read Bin done\n");

    LOG_DBG("clear MEM & load Bin buffer onto MEM\n");
    if(loadToccu)
        clearAndLoadBinToMem(memBaseAddr, buf, memSize, binSize, offset);
    else
        LOG_DBG_MUST("Just load to buf not load Bin to MEM\n");

    LOG_DBG("-:%s\n",__FUNCTION__);    

    return MTRUE;
}

#define CCU_WAIT_HALT_TIMOUT_SPEC 3000
MBOOL CcuDrvImp::loadCcuBin()
{
    LOG_INF_MUST("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    
    volatile unsigned int status;
    uint32_t checkCnt = 3;
    double duration = 0;

    char *pmPath = "/system/vendor/bin/lib3a.ccu.pm";
    char *dmPath = "/system/vendor/bin/lib3a.ccu.dm";

    unsigned int *p_pmemBuf = NULL;
    unsigned int *p_dmemBuf = NULL;

    //raise CCU core reset, and release
    volatile MUINT32 *ccuCtrlBasePtr = (MUINT32 *)m_pHwRegAddr[CCU_A];
    *(ccuCtrlBasePtr) = 0x0;
    CcuDrvUtil::delayInUs(10);
    *(ccuCtrlBasePtr) = 0x00010000;
    CcuDrvUtil::delayInUs(10);
    *(ccuCtrlBasePtr) = 0x0;
    CcuDrvUtil::delayInUs(10);

    LOG_INF_MUST("Wait halt, 1113");
    double ts_start = CcuDrvUtil::getTimeUs();
    double ts_end = ts_start;
    status = *(ccuCtrlBasePtr + OFFSET_CCU_STAT/4);
    while(!(status & 0x100))
    {
        status = *(ccuCtrlBasePtr + OFFSET_CCU_STAT/4);
        duration = CcuDrvUtil::diffTimeUs(ts_start, ts_end);
        if(duration > CCU_WAIT_HALT_TIMOUT_SPEC)
        {
            status = *(ccuCtrlBasePtr + OFFSET_CCU_STAT/4);
            if (!(status & 0x100))
            {
                LOG_ERR("wait ccu halt before load bin, timout: %lf, retry: %d", duration, checkCnt);

                if(checkCnt <= 0)
                {
                    PrintReg();
                    ret = MFALSE;
                    goto LOAD_CCU_BIN_EXIT;
                }

                checkCnt--;
            }
            else
            {
                LOG_WRN("CCU wait halt timeout false alarm");
            }
        }
        ts_end = CcuDrvUtil::getTimeUs();
    }

    //========================= Load PMEM binary ===========================
    LOG_INF_MUST("Load PM");

    p_pmemBuf = (unsigned int*)malloc((CCU_PMEM_SIZE>>2) * sizeof(unsigned int));//128K

    ret = loadBin(pmPath, m_pHwRegAddr[CCU_PMEM], p_pmemBuf, CCU_PMEM_SIZE, 0, BIN_CCU, true);
    free(p_pmemBuf);

    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

    //========================= Load DMEM binary ===========================
    LOG_INF_MUST("Load DM");

    p_dmemBuf = (unsigned int*)malloc((CCU_DMEM_SIZE>>2) * sizeof(unsigned int));//128K

    ret = loadBin(dmPath, m_pHwRegAddr[CCU_DMEM], p_dmemBuf, CCU_DMEM_SIZE, CCU_DMEM_OFFSET, BIN_CCU, true);

    free(p_dmemBuf);
    if(ret == MFALSE)
        goto LOAD_CCU_BIN_EXIT;

LOAD_CCU_BIN_EXIT:
    if(ret == MFALSE)
    {
        TriggerAee("loadCcuBin error");
    }
    LOG_INF_MUST("-:%s\n",__FUNCTION__);
    return ret;
}

MBOOL CcuDrvImp::loadCcuDdrBin()
{
    LOG_INF_MUST("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    char *ddrPath = "/system/vendor/bin/lib3a.ccu.ddr";

    //========================= Load DDR binary ===========================
    LOG_INF_MUST("Load DDR binary");
    unsigned int *p_ddrBuf = (unsigned int*)malloc((CCU_CACHE_SIZE>>2) * sizeof(unsigned int));//128K
    ret = loadBin(ddrPath, (unsigned int*)m_ddr_buf_va, p_ddrBuf, CCU_CACHE_SIZE, SENSOR_RESERVED, BIN_CCU, true);
    free(p_ddrBuf);

    if(ret == MFALSE)
    {
        TriggerAee("loadCcuDdrBin error");
    }

    LOG_INF_MUST("-:%s\n",__FUNCTION__);
    return ret;
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
int CcuDrvImp::dequeueAFO(MUINT32 m_sensorIdx)
{
    int ret;
    LOG_DBG("+\n");

    ret = dequeueAFO_Imp(this, m_sensorIdx);

    LOG_DBG("ret: %d\n", ret);
    LOG_DBG("-\n");

    return ret;
}

void CcuDrvImp::RequestCamFreqUpdate(uint32_t freqLevel)
{
    LOG_DBG_MUST("+:%s(%d)\n", __FUNCTION__, freqLevel);

    ioctl(this->m_Fd, CCU_IOCTL_UPDATE_CAM_FREQ_REQUEST, &freqLevel);

    LOG_DBG_MUST("-:%s\n", __FUNCTION__);
}

static int js_cnt = -1;
void CcuDrvImp::PrintReg()
{
    MUINT32 *ccuCtrlBasePtr = (MUINT32 *)m_pHwRegAddr[CCU_A];
    MUINT32 *ccuCtrlPtr = (MUINT32 *)m_pHwRegAddr[CCU_A];
    MUINT32 *ccuDmPtr = (MUINT32 *)m_pHwRegAddr[CCU_DMEM];
    MUINT32 *ccuPmPtr = (MUINT32 *)m_pHwRegAddr[CCU_PMEM];

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

void CcuDrvImp::PrintDdrBuffer()
{
    MUINT32 *ccuDdrPtr = (MUINT32 *)m_ddr_buf_va;
    LOG_WRN("=============== CCU DDR DUMP START ===============\n");
    for(int i=0 ; i<0x300000 ; i += 16)
    {
        LOG_WRN("0x1%07x: 0x%08x ,0x%08x ,0x%08x ,0x%08x\n", i, *(ccuDdrPtr), *(ccuDdrPtr+1), *(ccuDdrPtr+2), *(ccuDdrPtr+3));
        ccuDdrPtr += 4;
    }
}

void CcuDrvImp::CheckPm()
{
    volatile MUINT32 *ccuPmPtr = (MUINT32 *)m_pHwRegAddr[CCU_PMEM];
    
    unsigned int *p_pmemBuf = (unsigned int*)malloc((CCU_PMEM_SIZE>>2) * sizeof(unsigned int));//128K
    char *pmPath = "/system/vendor/bin/lib3a.ccu.pm";
    if (p_pmemBuf == NULL) {
        LOG_ERR("%s:pmem buf alloc failed.\n", __FUNCTION__);
        return;
    }
    loadBin(pmPath, m_pHwRegAddr[CCU_PMEM], p_pmemBuf, CCU_PMEM_SIZE, 0, BIN_CCU, false);

    int i=0;
    LOG_WRN("=============== CCU CHECK PM START ===============\n");
    for(i=0 ; i<CCU_PMEM_SIZE/4 ; i++)
    {
        if(*(ccuPmPtr + i) != p_pmemBuf[i])
            LOG_WRN("CCU PM corruption index 0x%08x O(0x%08x), X(0x%08x)\n", i*4, p_pmemBuf[i], *(ccuPmPtr + i));        
    }
    LOG_WRN("=============== CCU CHECK PM STOP ===============\n");
    free(p_pmemBuf);
    

    volatile MUINT32 *ccuiddrPtr = (unsigned int*)m_ddr_buf_va;

    unsigned int *p_ddrBuf = (unsigned int*)malloc((CCU_CACHE_SIZE>>2) * sizeof(unsigned int));//128K
    char *ddrPath = "/system/vendor/bin/lib3a.ccu.ddr";
    if (p_ddrBuf == NULL) {
        LOG_ERR("%s:ddr bin buf alloc failed.\n", __FUNCTION__);
        return;
    }
    loadBin(ddrPath, (unsigned int*)m_ddr_buf_va, p_ddrBuf, CCU_CACHE_SIZE, SENSOR_RESERVED, BIN_CCU, false);

    LOG_WRN("=============== CCU CHECK DDR START 0x%08x ===============\n", CCU_IDDR_BASE);
    for(i=0 ; i< CCU_IDDR_SIZE/4 ; i++)
    {
        if(*(ccuiddrPtr + (SENSOR_RESERVED/4) + i) != p_ddrBuf[i])
            LOG_WRN("CCU DDR corruption index 0x%08x O(0x%08x), X(0x%08x)\n", (i*4 + CCU_IDDR_BASE), p_ddrBuf[i], *(ccuiddrPtr + (SENSOR_RESERVED/4) + i));
    }
    LOG_WRN("=============== CCU CHECK DDR STOP ===============\n");
    free(p_ddrBuf);
}

static MUINT32 logBuf_1[CCU_LOG_SIZE/4];
static MUINT32 logBuf_2[CCU_LOG_SIZE/4];
static MUINT32 isrlogBuf[CCU_ISR_LOG_SIZE/4];
void CcuDrvImp::DumpSramLog()
{
    volatile char *ccuCtrlBasePtr = (char *)m_pHwRegAddr[CCU_DMEM];
    volatile char *ccuLogPtr_1 = ccuCtrlBasePtr + m_CcuLogBaseOffset;
    volatile char *ccuLogPtr_2 = ccuCtrlBasePtr + m_CcuLogBaseOffset + CCU_LOG_SIZE;
    volatile char *isrLogPtr = ccuCtrlBasePtr + m_CcuLogBaseOffset + (CCU_LOG_SIZE * 2);
    
    volatile MUINT32 *from_sram;
    volatile MUINT32 *to_dram;

    from_sram = (MUINT32 *)ccuLogPtr_1;
    to_dram = (MUINT32 *)logBuf_1;
    for(int i = 0; i<CCU_LOG_SIZE/4-1; i++)
    {
        *(to_dram+i) = *(from_sram+i);
    }
    from_sram = (MUINT32 *)ccuLogPtr_2;
    to_dram = (MUINT32 *)logBuf_2;
    for(int i = 0; i<CCU_LOG_SIZE/4-1; i++)
    {
        *(to_dram+i) = *(from_sram+i);
    }
    from_sram = (MUINT32 *)isrLogPtr;
    to_dram = (MUINT32 *)isrlogBuf;
    for(int i = 0; i<CCU_ISR_LOG_SIZE/4-1; i++)
    {
        *(to_dram+i) = *(from_sram+i);
    }

    LOG_WRN("=============== CCU INTERNAL LOG DUMP START ===============\n");

    logBuf_1[CCU_LOG_SIZE/4 -1] = LOG_ENDEND;
    logBuf_2[CCU_LOG_SIZE/4 -1] = LOG_ENDEND;
    isrlogBuf[CCU_ISR_LOG_SIZE/4 -1] = LOG_ENDEND;

    LOG_WRN("---------CHUNK 1-----------\n");
    _parseAndPrintCcuLog((char *)logBuf_1, (MUINT32 *)(logBuf_1 + CCU_LOG_SIZE/4), MTRUE);
    LOG_WRN("---------CHUNK 2-----------\n");
    _parseAndPrintCcuLog((char *)logBuf_2, (MUINT32 *)(logBuf_2 + CCU_LOG_SIZE/4), MTRUE);
    LOG_WRN("---------ISR LOG-----------\n");
    _parseAndPrintCcuLog((char *)isrlogBuf, (MUINT32 *)(isrlogBuf + CCU_ISR_LOG_SIZE/4), MTRUE);
    
    LOG_WRN("=============== CCU INTERNAL LOG DUMP END ===============\n");
}

void CcuDrvImp::DumpDramLog()
{
    LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
    PrintCcuLogByIdx(0, MTRUE);
    LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
    PrintCcuLogByIdx(1, MTRUE);
}

int CcuDrvImp::dequeueAFO_Imp(void *arg, MUINT32 m_sensorIdx)
{
    CcuDrvImp *_this = reinterpret_cast<CcuDrvImp*>(arg);
    MBOOL ret;
    CCU_WAIT_IRQ_ST _irq;

    LOG_DBG("+:\n");
    LOG_VRB("AFcall _waitIrq() to wait ccu interrupt...\n");

    _irq.Clear      = CCU_IRQ_CLEAR_WAIT;
    _irq.Status     = CCU_INT_ST;
    _irq.St_type    = CCU_SIGNAL_INT;
    _irq.Timeout    = 1000; //wait forever
    //_irq.Timeout    = 100; //wait 100 ms
    _irq.TimeInfo.passedbySigcnt = 0x00ccdd00*m_sensorIdx;
    ret = _this->_waitAFIrq(&_irq, m_sensorIdx);

    if ( MFALSE == ret )
    {
        LOG_ERR("AFwaitIrq fail\n");
    }
    else
    {
        LOG_VRB("AFwaitIrq() done, irq_cnt(%d), irq_task(%d) \n", ++js_cnt, _irq.TimeInfo.passedbySigcnt);
        //break;
        if ( m_sensorIdx == _irq.TimeInfo.passedbySigcnt )
        {
            LOG_DBG("AFm_taskWorkerThread : AFO done\n");
            return(m_sensorIdx);
        }
        else if( MSG_TO_APMCU_CAM_AFO_ABORT == _irq.TimeInfo.passedbySigcnt )
        {
            LOG_VRB("AF abort task: %d\n", _irq.TimeInfo.passedbySigcnt);
            return(MSG_TO_APMCU_CAM_AFO_ABORT);
        }
        else if(0x00ccdd00*m_sensorIdx == _irq.TimeInfo.passedbySigcnt)
        {
            LOG_VRB("AFno irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
        }
        else
        {
            LOG_ERR("AFunknow irq_task: %d\n", _irq.TimeInfo.passedbySigcnt);
        }
    }

    LOG_DBG("-:\n");

    return(0);
}

void CcuDrvImp::TriggerAee(char *msg)
{
    AEE_ASSERT_CCU_USER(msg);
}

int32_t CcuDrvImp::getSensorI2cSlaveAddr(int32_t *sensorI2cSlaveAddr)
{
    if(ioctl(this->m_Fd, CCU_IOCTL_GET_SENSOR_I2C_SLAVE_ADDR , sensorI2cSlaveAddr) < 0)
    {
        LOG_ERR("ERROR: CCU_IOCTL_GET_SENSOR_SLAVE_ID :%x\n", *sensorI2cSlaveAddr);
        return MFALSE;
    }

    return MTRUE;
}

void CcuDrvImp::PrintCcuLogByIdx(uint32_t logBufIdx, MBOOL mustPrint)
{
    LOG_DBG("+:logBufIdx(%d), mustPrint(%d)\n", logBufIdx, mustPrint);
    //get corresponding log buffer base address
    char *logStr = (char *)m_pLogBuf[logBufIdx];
    //put LOG_END into the tail of buffer
    *(MUINT32 *)(logStr + SIZE_1MB - 4) = LOG_ENDEND;

    //call to real parse & print function
    _parseAndPrintCcuLog(logStr, (MUINT32 *)(logStr + SIZE_1MB), mustPrint);
}

void* CcuDrvImp::ApTaskWorkerThreadLoop(void *arg)
{
    CcuDrvImp *_this = reinterpret_cast<CcuDrvImp*>(arg);
    MBOOL ret;
    CCU_WAIT_IRQ_ST _irq;
    MUINT32 ccu_bw_request[3] = {0};

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

        LOG_VRB("call _waitIrq() to wait ccu interrupt...\n");

        _irq.Clear      = CCU_IRQ_CLEAR_WAIT;
        _irq.Status     = CCU_INT_ST;
        _irq.St_type    = CCU_SIGNAL_INT;
        //_irq.Timeout    = 0; //wait forever
        _irq.Timeout    = 100; //wait 100 ms
        _irq.TimeInfo.passedbySigcnt = 0x00aabb00;
        ret = _this->_waitIrq(&_irq);

        UTIL_TRACE_BEGIN("CcuHardworking");

        if ( _this->m_taskWorkerThreadEnd )
        {
            LOG_DBG("m_taskWorkerThreadEnd\n");
            break;
        }

        if ( MFALSE == ret )
        {
            LOG_ERR("_waitIrq fail\n");
        }
        else
        {
            LOG_VRB("_waitIrq() done, irq_cnt(%d), irq_task(%d) \n", ++js_cnt, _irq.TimeInfo.passedbySigcnt);
            //break;

            if(ioctl(_this->m_Fd, CCU_IOCTL_UPDATE_QOS_REQUEST , ccu_bw_request) < 0)
            {
                LOG_DBG("CCU_IOCTL_UPDATE_QOS_REQUEST not update\n");
            }
            else
            {
                LOG_DBG("CCU_IOCTL_UPDATE_QOS_REQUEST :I(%x),O(%x),G(%x)\n", ccu_bw_request[0], ccu_bw_request[1], ccu_bw_request[2]);
            }

            if ( 1 == _irq.TimeInfo.passedbySigcnt ) //1 means dump 1st log buffer (index 0)
            {
                _this->PrintCcuLogByIdx(0);
            }
            else if ( 2 == _irq.TimeInfo.passedbySigcnt ) //2 means dump 2nd log buffer (index 1)
            {
                _this->PrintCcuLogByIdx(1);
            }
            else if ( -1 == _irq.TimeInfo.passedbySigcnt ) //-1 means assert occured
            {
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[0] =============\n");
                LOG_WRN("===== CCU LOG DUMP START =====\n");
                _this->PrintCcuLogByIdx(0, MTRUE);
                LOG_WRN("============ CCU ASSERT LOG DUMP: LOGBUF[1] =============\n");
                _this->PrintCcuLogByIdx(1, MTRUE);
                LOG_WRN("===== CCU LOG DUMP END =====\n");
                if(_this->isPowerOn)
                {
                    _this->CheckPm();
                    _this->DumpSramLog();
                    _this->PrintReg();
                    _this->PrintDdrBuffer();
                }
                AEE_ASSERT_CCU_USER("CCU ASSERT\n");
            }
            else if ( -2 == _irq.TimeInfo.passedbySigcnt ) //-1 means warning occured
            {
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[0] =============\n");
                _this->PrintCcuLogByIdx(0, MTRUE);
                LOG_WRN("============ CCU WARNING LOG DUMP: LOGBUF[1] =============\n");
                _this->PrintCcuLogByIdx(1, MTRUE);
            }
            else if(0x00aabb00 == _irq.TimeInfo.passedbySigcnt) //magic number 0x00aabb00 means no msg from CCU to APMCU
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

/*******************************************************************************
* Private Functions
********************************************************************************/
MBOOL CcuDrvImp::_closeCcuKdrv()
{
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
    }

    return MTRUE;
}

MBOOL CcuDrvImp::_openCcuKdrv()
{

    LOG_DBG_MUST("%s+",__FUNCTION__);

    if(this->m_Fd >= 0)
    {
        //if kdrv is already opened, return
        LOG_DBG_MUST("CCU kernel already opened.");
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
    }

    LOG_DBG_MUST("%s-",__FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::_setCcuPower(int powerCtrlType)
{
    LOG_DBG("+:%s\n", __FUNCTION__);
    m_power.bON = powerCtrlType;
    m_power.freq = 500;
    m_power.power = 15;

    if(ioctl(this->m_Fd, CCU_IOCTL_SET_POWER, &m_power) < 0)
    {
        LOG_ERR("CCU_IOCTL_SET_POWER: bON(%d)\n", m_power.bON);
        return MFALSE;
    }
    LOG_DBG_MUST("%s: setpower(%d) cmd done.\n", __FUNCTION__, m_power.bON);

    LOG_DBG("-:%s\n", __FUNCTION__);

    return MTRUE;
}

MBOOL CcuDrvImp::_setCcuRun()
{
    uint32_t ret;
    MUINT32 *ccuCtrlBasePtr = (MUINT32 *)m_pHwRegAddr[CCU_A];
    MUINT8 *ccuDmemBasePtr = (MUINT8 *)m_pHwRegAddr[CCU_DMEM];

    LOG_DBG("+\n");
    //Set security disable
    *(ccuCtrlBasePtr + OFFSET_CCU_SECURITY_CTRL_OFFSET) = 0x00000180; //set security reg

    //Set remap offset
    MUINT32 remapOffset = m_ddr_buf_mva - CCU_CACHE_BASE;
    *(ccuCtrlBasePtr + OFFSET_CCU_REMAP_OFFSET) = remapOffset;
    LOG_INF_MUST("set CCU remap offset: %x\n", remapOffset);

    LOG_DBG("+:check CCU_A_INFO20 : %x\n",ccuCtrlBasePtr + OFFSET_CCU_A_INFO20/4);
    *(ccuCtrlBasePtr + OFFSET_CCU_A_INFO20/4) = (ccu_drv_DbgLogEnable_VERBOSE)? 6 : (ccu_drv_DbgLogEnable_DEBUG)? 5 : (ccu_drv_DbgLogEnable_INFO)? 4 : 0;

    MUINT32 log_taglevel = 0;
    _get_log_taglevel(&log_taglevel);
    LOG_INF_MUST("log_taglevel is %x\n", log_taglevel);
    *(ccuCtrlBasePtr + OFFSET_CCU_A_INFO21/4) = log_taglevel;

    LOG_INF_MUST("CPU Ref Buf MVA %x\n", m_CpuRefBufMva);
    *(ccuCtrlBasePtr + OFFSET_CCU_A_INFO22/4) = m_CpuRefBufMva-remapOffset;

    //enable CCU single access DRAM bus
    MUINT32 mmu_enable_reg;
    mmu_enable_reg = *(ccuCtrlBasePtr + CCU_H2X_MMU_ENABLE_REG/4);
    *(ccuCtrlBasePtr + CCU_H2X_MMU_ENABLE_REG/4) = (mmu_enable_reg | MMU_ENABLE_BIT);

    //fill in backup data buffer address
    LOG_DBG("bkdata_ddr_buf_mva: %x\n", m_CcuBkDataMva);
    m_shared_buf_map_ptr->bkdata_ddr_buf_mva = m_CcuBkDataMva;
    
    ret = ioctl(this->m_Fd, CCU_IOCTL_SET_RUN);
    if(ret < 0){
        LOG_ERR("CCU_IOCTL_SET_RUN:(%d)", ret);
        return MFALSE;
    }

    //sp.reg.25 refers sram log base address
    m_CcuLogBaseOffset = *(ccuCtrlBasePtr + OFFSET_CCU_SRAM_LOG_BASE);
    LOG_INF_MUST("CCU log base offset: %x\n", m_CcuLogBaseOffset);

    LOG_DBG("shared_buf_map_offset: %p\n", OFFSET_CCU_SHARED_BUF_MAP_BASE);
    LOG_DBG("m_shared_buf_map_ptr: %p\n", m_shared_buf_map_ptr);
    LOG_DBG("ipc_in_data_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_in_data_base_offset);
    LOG_DBG("ipc_out_data_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_out_data_base_offset);
    LOG_DBG("ipc_base_offset: 0x%x\n", m_shared_buf_map_ptr->ipc_base_offset);

    LOG_DBG("ae_algo_data_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_algo_data_addr);
    LOG_DBG("ae_init_data_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_init_data_addr);
    LOG_DBG("ae_vsync_info_tg1_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].ae_vsync_info_addr);
    LOG_DBG("aesync_algo_in_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].aesync_algo_in_addr);
    LOG_DBG("aesync_algo_out_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_1)].aesync_algo_out_addr);

    LOG_DBG("ae_algo_data_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_algo_data_addr);
    LOG_DBG("ae_init_data_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_init_data_addr);
    LOG_DBG("ae_vsync_info_tg2_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].ae_vsync_info_addr);
    LOG_DBG("aesync_algo_in_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].aesync_algo_in_addr);
    LOG_DBG("aesync_algo_out_addr: %x", m_shared_buf_map_ptr->exif_data_addrs[CCU_TG2IDX(CCU_CAM_TG_2)].aesync_algo_out_addr);

    LOG_DBG("-:CCU_IOCTL_SET_RUN\n");
    return MTRUE;
}

MBOOL CcuDrvImp::_onlypowerOff()
{
    LOG_DBG_MUST("+:%s\n",__FUNCTION__);

    //power off
    if(!this->_setCcuPower(4))
    {
        return MFALSE;
    }
    LOG_DBG_MUST("-:%s\n",__FUNCTION__);
    return MTRUE;
}

MBOOL CcuDrvImp::_realUninit()
{
    MBOOL ret = MTRUE;

    LOG_DBG_MUST("+\n");

    _unMapHwRegs();

    CcuAeeMgrDl::CcuAeeMgrDlUninit();

    //
    this->_closeCcuKdrv();

EXIT:

    LOG_INF_MUST("CcuDrvImp uinitial done gracefully\n");
    return ret;
}

MBOOL CcuDrvImp::_unMapHwRegs()
{
    uint32_t size;
    for(MUINT32 i=0;i<MAX_CCU_HW_MODULE;i++)
    {
        if(m_pHwRegAddr[i] != MAP_FAILED && m_pHwRegAddr[i] != NULL)
        {
            int ret = 0;
            switch(i)
            {
                case CCU_A:
                    LOG_DBG_MUST("Uinit_CCU_A munmap: %p\n", m_pHwRegAddr[i]);
                    m_pHwRegAddr[i] = m_pHwRegAddr[i] - (CCU_HW_OFFSET/4);
                    size = PAGE_SIZE;
                    break;
                case CCU_CAMSYS:
                    LOG_DBG_MUST("Uinit_CCU_CAMSYS munmap: %p\n", m_pHwRegAddr[i]);
                    size = CCU_CAMSYS_SIZE;
                    break;
                case CCU_PMEM:
                    LOG_DBG_MUST("Uinit_CCU_PMEM munmap: %p\n", m_pHwRegAddr[i]);
                    size = CCU_PMEM_SIZE;
                    break;
                case CCU_DMEM:
                    LOG_DBG_MUST("Uinit_CCU_DMEM munmap: %p\n", m_pHwRegAddr[i]);
                    size = CCU_DMEM_SIZE;
                    break;
                default:
                    LOG_ERR("this hw module(%d) is unsupported\n",i);
                    break;
            }

            ret = munmap(m_pHwRegAddr[i], size);

            if (ret < 0)
            {
                LOG_ERR("munmap fail: %p\n", m_pHwRegAddr[i]);
            }

            m_pHwRegAddr[i] = NULL;
        }
    }

    return MTRUE;
}

MBOOL CcuDrvImp::_waitIrq(CCU_WAIT_IRQ_ST* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    CCU_IRQ_CLEAR_ENUM OrgClr;
    CCU_WAIT_IRQ_STRUCT wait;

    LOG_DBG(" + Status(0x%08x),Timeout(%d).\n", pWaitIrq->Status, pWaitIrq->Timeout);

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    wait.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(CCU_WAIT_IRQ_ST));

    LOG_DBG("CCU_IOCTL_WAIT_IRQ ioctl call, arg is CCU_WAIT_IRQ_STRUCT, size: %d\n", sizeof(CCU_WAIT_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_IRQ,&wait);

    memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(CCU_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;


    if(Ret < 0) {
        LOG_ERR("CCU_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL CcuDrvImp::_waitAFIrq(CCU_WAIT_IRQ_ST* pWaitIrq, MUINT32 m_sensorIdx)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    CCU_IRQ_CLEAR_ENUM OrgClr;
    CCU_WAIT_IRQ_STRUCT wait;

    LOG_DBG(" + Status(0x%08x),Timeout(%d).\n", pWaitIrq->Status, pWaitIrq->Timeout);

    OrgTimeOut = pWaitIrq->Timeout;
    OrgClr = pWaitIrq->Clear;
    wait.Type = CCU_IRQ_TYPE_INT_CCU_A_ST;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(CCU_WAIT_IRQ_ST));

    LOG_DBG("CCU_IOCTL_WAIT_AF_IRQ ioctl call, arg is CCU_WAIT_IRQ_STRUCT, size: %d\n", sizeof(CCU_WAIT_IRQ_STRUCT));

    if((m_sensorIdx >= IMGSENSOR_SENSOR_IDX_MIN_NUM) && 
                (m_sensorIdx < IMGSENSOR_SENSOR_IDX_MAX_NUM))
    {
        wait.bDumpReg = m_sensorIdx;
        Ret = ioctl(this->m_Fd,CCU_IOCTL_WAIT_AF_IRQ,&wait);
    }
    else
    {
        LOG_ERR("unknow m_sensorIdx:0x%x\n",m_sensorIdx);
        return MFALSE;
    }

    memcpy(&pWaitIrq->TimeInfo,&wait.EventInfo.TimeInfo,sizeof(CCU_IRQ_TIME_STRUCT));
    pWaitIrq->Timeout = OrgTimeOut;
    pWaitIrq->Clear = OrgClr;

    if(Ret < 0) {
        LOG_ERR("CCU_IOCTL_WAIT_AF_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }


    return MTRUE;
}

void CcuDrvImp::_createCcuDrvIpc()
{
    android::Mutex::Autolock lock(this->m_CcuSendCmdMutex);

    LOG_DBG_MUST("[%s] +", __FUNCTION__);

    m_ccuDrvIpc = new CcuDrvIpc();
    m_ccuDrvIpc->init(m_shared_buf_map_ptr, m_pHwRegAddr[CCU_DMEM], m_pHwRegAddr[CCU_A]);

    LOG_DBG_MUST("[%s] -", __FUNCTION__);
}

void CcuDrvImp::_destroyCcuDrvIpc()
{
    android::Mutex::Autolock lock(this->m_CcuSendCmdMutex);

    LOG_DBG_MUST("[%s] +", __FUNCTION__);

    if(m_ccuDrvIpc != NULL)
    {
        delete m_ccuDrvIpc;
        m_ccuDrvIpc = NULL;
    }

    LOG_DBG_MUST("[%s] -", __FUNCTION__);
}

MBOOL CcuDrvImp::allocSensorSlot(uint32_t sensor_idx, uint8_t *sensor_slot)
{
    android::Mutex::Autolock lock(this->m_CcuSensorMutex);

    for (int i = 0; i < CCU_SENSOR_BIN_CAPACITY; ++i)
    {
        if (m_CcuSensorSlot[i] == IMGSENSOR_SENSOR_IDX_NONE || m_CcuSensorSlot[i] == sensor_idx)
        {
            *sensor_slot = i;
            m_CcuSensorSlot[i] = sensor_idx;
            LOG_DBG("Alloc sensor %d slot:%d",sensor_idx , i);
            return true;
        }

    }
    LOG_ERR("Alloc sensor %d available slot not found", sensor_idx);
    for (int i = 0; i < CCU_SENSOR_BIN_CAPACITY; ++i)
    {
        LOG_WRN("sensor slot:%d %d",i , m_CcuSensorSlot[i]);
    }
    return false;    
}

void CcuDrvImp::FreeSensorSlot(uint32_t sensor_idx)
{
    android::Mutex::Autolock lock(this->m_CcuSensorMutex);

    for (int i = 0; i < CCU_SENSOR_BIN_CAPACITY; ++i)
    {
        if (m_CcuSensorSlot[i] == sensor_idx)
        {
            LOG_DBG("Free sensor %d slot:%d",sensor_idx , i);
            m_CcuSensorSlot[i] = IMGSENSOR_SENSOR_IDX_NONE;
            return;
        }
    }

    LOG_ERR("Free sensor %d not found", sensor_idx);
    for (int i = 0; i < CCU_SENSOR_BIN_CAPACITY; ++i)
    {
        LOG_WRN("sensor slot:%d %d",i , m_CcuSensorSlot[i]);
    }
    return;
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
    this->_getSensorName(this->m_Fd, sensor_idx, sensorName);
    LOG_DBG_MUST("ccu sensor name: %s\n", sensorName);

    //due to the samsung sensor defect, CCU must by pass exposure setting while performing long exposure
    //long expossure only works when set and trigger i2c to sensor in APMCU side
    if((strncmp(sensorName, "s5k3p9sx", 8) == 0))
    {
        //if the senosr is s5k3p9, mark it on indicator
        LOG_INF_MUST("detected sp sensor s5k3p9sx");
        *isSpSensor = false; //for 6779, do not mark for default, mark on needed
    }
    else if((strncmp(sensorName, "s5k3l6", 6) == 0))
    {
        //for s5k3l6, also bypass CCU long-exposure settings
        LOG_INF_MUST("detected sp sensor s5k3l6");
        *isSpSensor = false; //for 6779, do not mark for default, mark on needed
    }
    else if((strncmp(sensorName, "s5k2l7", 6) == 0))
    {
        //for s5k2l7, reduce require AAO line for sensor setting latched
        LOG_INF_MUST("detected sp sensor s5k2l7");
        *isSpSensor = true; //for 6779, do not mark for default, mark on needed
    }
    else
    {
        LOG_INF_MUST("no sp sensor detected");
        *isSpSensor = false;
    }

    strncat(sensorBinPath, sensorPathPrefix, 50);
    strncat(sensorBinPath, sensorName, 45);
    strncat(sensorBinPath, ".ddr", 5);

    //========================= Load Sensor DM binary ===========================
    LOG_DBG("Load Sensor DM");
    if ((sensor_slot == 0))
        ret = loadBin(sensorBinPath, (unsigned int *)m_ddr_buf_va, sensorBinBuf, 0, SENSOR_BIN_OFFSET_SLOT_1, BIN_SENSOR_BIN_1, true);
    else if ((sensor_slot == 1))
        ret = loadBin(sensorBinPath, (unsigned int *)m_ddr_buf_va, sensorBinBuf, 0, SENSOR_BIN_OFFSET_SLOT_2, BIN_SENSOR_BIN_2, true);
    if(ret == MFALSE)
        goto LOAD_SENSOR_BIN_EXIT;

LOAD_SENSOR_BIN_EXIT:
    if(ret == MFALSE)
    {
        TriggerAee("CCU loadSensorBin error");
    }
    LOG_DBG("-:%s\n",__FUNCTION__);    
    return ret;
}

MBOOL CcuDrvImp::loadLensBin(uint32_t sensor_idx, uint8_t lens_slot, char* lensName)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    unsigned int lensBinBuf[CCU_LENS_BIN_SIZE];
    char *lensPathPrefix = "/system/vendor/bin/libccu_";
    char lensBinPath[100] = "";
    char lensPathPostFIX[6][10] = {"_main","_sub","_main2","_sub2","_main3","_main4"};
    //========================= Get lens name via ioctl ===========================
    LOG_DBG_MUST("ccu lens name: %s\n", lensName);

    strncat(lensBinPath, lensPathPrefix, 50);
    strncat(lensBinPath, lensName, 32);

    MUINT32 sensor_nm_idx = CcuDrvUtil::sensorIdxToNonmapIdx(sensor_idx);
    if (sensor_nm_idx > 5)
    {
        ret = MFALSE;
        goto LOAD_LENS_BIN_EXIT;
    }
    strncat(lensBinPath, lensPathPostFIX[sensor_nm_idx], strlen(lensPathPostFIX[sensor_nm_idx]));

    strncat(lensBinPath, ".ddr", 5);

    LOG_DBG("Load Lens DM");
    if ((lens_slot == 0))
        ret = loadBin(lensBinPath, (unsigned int *)m_ddr_buf_va, lensBinBuf, 0, LENS_BIN_OFFSET_SLOT_1, BIN_LENS_BIN_1, true);
    else if ((lens_slot == 1))
        ret = loadBin(lensBinPath, (unsigned int *)m_ddr_buf_va, lensBinBuf, 0, LENS_BIN_OFFSET_SLOT_2, BIN_LENS_BIN_2, true);
    if(ret == MFALSE)
        goto LOAD_LENS_BIN_EXIT;

LOAD_LENS_BIN_EXIT:
    if(ret == MFALSE)
    {
        TriggerAee("CCU loadLensBin error");
    }
    LOG_DBG("-:%s\n",__FUNCTION__);
    return ret;
}

MBOOL CcuDrvImp::loadCustSensorBin(uint32_t sensor_idx, uint8_t sensor_slot)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    unsigned int custsensorBinBuf[CCU_CUSTSENSOR_BIN_SIZE];
    char *custsensorPathPrefix = "/system/vendor/bin/libccu_";
    char sensorName[100] = "";
    char custsensorBinPath[100] = "";

    //========================= Get cust sensor name via ioctl ===========================
    this->_getSensorName(this->m_Fd, sensor_idx, sensorName);
    LOG_DBG_MUST("ccu sensor name: %s\n", sensorName);

    strncat(custsensorBinPath, custsensorPathPrefix, 50);
    strncat(custsensorBinPath, sensorName, 45);
    strncat(custsensorBinPath, "_custsensor.ddr", 16);

    //========================= Load cust sensor DM binary ===========================
    LOG_DBG("Load Cust Sensor DM");
    if ((sensor_slot == 0))
        ret = loadBin(custsensorBinPath, (unsigned int *)m_ddr_buf_va, custsensorBinBuf, 0, CUSTSENSOR_BIN_OFFSET_SLOT_1, BIN_CUSTSENSOR_BIN_1, true);
    else if ((sensor_slot == 1))
        ret = loadBin(custsensorBinPath, (unsigned int *)m_ddr_buf_va, custsensorBinBuf, 0, CUSTSENSOR_BIN_OFFSET_SLOT_2, BIN_CUSTSENSOR_BIN_2, true);
    if(ret == MFALSE)
        goto LOAD_CUSTSENSOR_BIN_EXIT;

LOAD_CUSTSENSOR_BIN_EXIT:
    if(ret == MFALSE)
    {
        TriggerAee("CCU loadCustSensorBin error");
    }
    LOG_DBG("-:%s\n",__FUNCTION__);    
    return ret;
}

MBOOL CcuDrvImp::loadCustFuncBin()
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    MBOOL ret = MTRUE;
    unsigned int custfuncBinBuf[CCU_CUSTSENSOR_BIN_SIZE];
    char custfuncBinPath[100]  = "/system/vendor/bin/libccu_custfunc.ddr";
    //========================= Load cust sensor DM binary ===========================
    LOG_DBG("Load Cust Func DM");
    ret = loadBin(custfuncBinPath, (unsigned int *)m_ddr_buf_va, custfuncBinBuf, 0, CUSTFUNC_BIN_OFFSET_SLOT, BIN_CUSTFUNC_BIN, true);
    if(ret == MFALSE)
        goto LOAD_CUSTFUNC_BIN_EXIT;

LOAD_CUSTFUNC_BIN_EXIT:
    if(ret == MFALSE)
    {
        TriggerAee("CCU loadCustFuncBin error");
    }
    LOG_DBG("-:%s\n",__FUNCTION__);    
    return ret;
}

MBOOL CcuDrvImp::_getSensorName(MINT32 kdrvFd, uint32_t sensor_idx, char *sensorName)
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

void CcuDrvImp::_createCcuWorkerThread()
{
    LOG_DBG("+\n");

    this->m_taskWorkerThreadReady = MFALSE;
    this->m_taskWorkerThreadEnd = MFALSE;

	pthread_create(&m_CcuWorkerThread, NULL, ApTaskWorkerThreadLoop, this);
    pthread_setname_np(m_CcuWorkerThread, "ccu_worker");

    LOG_DBG_MUST("ApTaskWorkerThreadLoop created, wait for m_taskWorkerThreadReady\n");
    LOG_DBG_MUST("m_taskWorkerThreadReady addr: %p\n", &(this->m_taskWorkerThreadReady));
    while(this->m_taskWorkerThreadReady != MTRUE)
    {
        usleep(500);
    }
    LOG_DBG_MUST("m_taskWorkerThreadReady: %d\n", this->m_taskWorkerThreadReady);

    LOG_DBG("-\n");
}

void CcuDrvImp::_destroyCcuWorkerThread()
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
	pthread_join(m_CcuWorkerThread, NULL);

	//
	LOG_DBG("-\n");
}
//MUINT32 *ccuLOGBasePtr;
bool CcuDrvImp::_parsenextloginfo(MUINT32 **ccuLOGBasePtr, unsigned char &log_para_num, unsigned char &log_str_len, bool print_log_all, const MUINT32 *logTail)
{
    MUINT32 log_header_id;
    MUINT32 log_header = *(*ccuLOGBasePtr)++;

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
                if((*ccuLOGBasePtr) > (logTail - (log_para_num + (log_str_len>>2))))
                {
                    LOG_WRN("LOG exceed num(%d),len(%d)", log_para_num, log_str_len);
                    LOG_WRN("(*ccuLOGBasePtr) : 0x%x logTail : 0x%x", (*ccuLOGBasePtr), logTail);
                    return false;
                }
                (*ccuLOGBasePtr)+=(log_para_num + (log_str_len>>2));
                log_header=*(*ccuLOGBasePtr)++;
            }
        }
        else if (log_header_id==LOG_MUST_HEADER)
        {
            break;
        }
        else if (log_header==LOG_ENDEND)
        {
            return false;
        }
        else
        {
            log_header=*(*ccuLOGBasePtr)++;
        }
    }
    return true;
}

void CcuDrvImp::_parseAndPrintCcuLog(const char *logStr, const MUINT32 *logTail, MBOOL mustPrint)
{
    #define LOG_LEN_LIMIT 800
    static char logBuf[LOG_LEN_LIMIT + 1];

    int logSegCnt = 0;
    MUINT32 *ccuLOGBasePtr = (MUINT32 *)logStr;
    char log_str[256] = {0};
    MUINT32 log_value[8] = {0};
    MUINT32 log_header;

    unsigned char log_para_num, log_str_len;
    char plog_out_str[LOG_LEN_LIMIT + 1] = {0};
    MUINT32 log_offset = 0;

    bool printing = true;
    bool print_log_all = false;;

    if ((mustPrint==true) || (ccu_drv_DbgLogEnable_DEBUG) || (ccu_drv_DbgLogEnable_VERBOSE))
    {
        print_log_all=true;
    }

    while(printing)
    {
LOG_CONTINUE :

        //log_header=*ccuLOGBasePtr++;
        if(_parsenextloginfo(&ccuLOGBasePtr, log_para_num, log_str_len, print_log_all, logTail) == false)
            goto LOG_EXIT;

        log_offset = 0;

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

            if(ccuLOGBasePtr > logTail - ((log_str_len>>2)))
            {
                LOG_WRN("LOG exceed num(%d),len(%d)", log_para_num, log_str_len);
                LOG_WRN("(*ccuLOGBasePtr) : 0x%x logTail : 0x%x", (*ccuLOGBasePtr), logTail);
                goto LOG_EXIT;
            }
            ccuLOGBasePtr += (log_str_len>>2);

            if(log_offset + log_str_len + 4*log_para_num> (LOG_LEN_LIMIT-64))
            {

                *(plog_out_str+log_offset)='\0';

                if(mustPrint)
                {
                    LOG_WRN("ccuLOGBasePtr : 0x%x log_str_len(%d) logTail(0x%x)\n", ccuLOGBasePtr, log_str_len, logTail);
                    LOG_WRN("\n----- DUMP SEG[%d] -----\n%s", logSegCnt, plog_out_str);
                }
                else
                {
                    LOG_INF("ccuLOGBasePtr : 0x%x log_str_len(%d) logTail(0x%x)\n", ccuLOGBasePtr, log_str_len, logTail);
                    LOG_INF("\n%s", plog_out_str);
                }

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

            //log_header=*ccuLOGBasePtr++;
            if(_parsenextloginfo(&ccuLOGBasePtr, log_para_num, log_str_len, print_log_all, logTail) == false)
                goto LOG_EXIT;
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

static void _get_log_taglevel(MUINT32 *log_taglevel)
{
    char logvalue[100] = {'\0'};//32bit

    LOG_INF_MUST("log_taglevelpre %x\n", *log_taglevel);
    *log_taglevel = 0;
/*notify : tag number cannot over 16 now use reserve first*/
    property_get("vendor.drv.ccu.log.reserve1", logvalue, DSIABLE_S);/*16*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_RESERVE1_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_RESERVE1_LOGTAG));

    property_get("vendor.drv.ccu.log.reserve2", logvalue, DSIABLE_S);/*15*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_RESERVE2_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_RESERVE2_LOGTAG));

    property_get("vendor.drv.ccu.log.reserve3", logvalue, DSIABLE_S);/*14*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_VERBOSE_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_VERBOSE_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuafsyncalgo", logvalue, DSIABLE_S);/*13*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AF_SYNC_ALGO_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AF_SYNC_ALGO_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuaesyncalgo", logvalue, DSIABLE_S);/*12*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AE_SYNC_ALGO_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AE_SYNC_ALGO_LOGTAG));

    property_get("vendor.drv.ccu.log.cculens", logvalue, IGNORE_S);/*11*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_LENS_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_LENS_LOGTAG));

    property_get("vendor.drv.ccu.log.ccui2c", logvalue, IGNORE_S);/*10*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_I2C_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_I2C_LOGTAG));

    property_get("vendor.drv.ccu.log.ccultmalgo", logvalue, DSIABLE_S);/*9*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_LTM_ALGO_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_LTM_ALGO_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuafalgo", logvalue, PRINT_S);/*8*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AF_ALGO_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AF_ALGO_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuflowltm", logvalue, IGNORE_S);/*7*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_LTM_FLOW_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_LTM_FLOW_LOGTAG));

    property_get("vendor.drv.ccu.log.ccumust", logvalue, PRINT_S);/*6*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_MUST_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_MUST_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuaealgo", logvalue, DSIABLE_S);/*5*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AE_ALGO_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AE_ALGO_LOGTAG));

    property_get("vendor.drv.ccu.log.ccusensor", logvalue, IGNORE_S);/*4*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_SENSOR_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_SENSOR_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuflowaf", logvalue, IGNORE_S/*PRINT_S*//*IGNORE_S*/);/*3*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AF_FLOW_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AF_FLOW_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuflowae", logvalue, IGNORE_S);/*2*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_AE_FLOW_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_AE_FLOW_LOGTAG));

    property_get("vendor.drv.ccu.log.ccuflowbasic", logvalue, IGNORE_S);/*1*/
    *log_taglevel = (*log_taglevel & ~(CCU_LOG_MASK(CCU_FLOW_BASIC_LOGTAG))) | (atoi(logvalue) << CCU_LOG_MASK_OFFSET(CCU_FLOW_BASIC_LOGTAG));
/*notify : tag number cannot over 16 now use reserve first*/
    
    char log_control[100];
    sprintf(log_control,"%d", *log_taglevel);
    LOG_INF_MUST("log_control %s\n", log_control);

    property_get("vendor.drv.ccu.log", logvalue, log_control);
    *log_taglevel = atoi(logvalue);
    LOG_INF_MUST("log_taglevel %x\n", *log_taglevel);
}