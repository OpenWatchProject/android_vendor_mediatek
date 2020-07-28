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
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>

#include "ccu_udrv.h"
#include "ccuif_compat.h"
#include "ccu_cmd_param_info.h"
#include "iccu_mgr.h"
#include "ccu_platform_def.h"
#include "ccu_mem_mgr.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include <string.h>

namespace NSCcuIf {

#define LOG_TAG "CcuMemMgr"
EXTERN_DBG_LOG_VARIABLE(ccu_drv);

static android::Mutex _ccuMemMgrCreationMutex;

CcuMemMgr *CcuMemMgr::getInstance()
{
    android::Mutex::Autolock lock(_ccuMemMgrCreationMutex);
    static CcuMemMgr _ccuMemMgrSingletonInstance;
    return &_ccuMemMgrSingletonInstance;
}

/*
static CcuMemMgr* _ccuMemMgrSingletonInstance = NULL;
static android::Mutex _ccuMemMgrCreationMutex;

CcuMemMgr *CcuMemMgr::getInstance()
{
    android::Mutex::Autolock lock(_ccuMemMgrCreationMutex);
    if (_ccuMemMgrSingletonInstance == NULL)
    {
        _ccuMemMgrSingletonInstance = new CcuMemMgr();
    }
    return _ccuMemMgrSingletonInstance;
}
*/

/*******************************************************************************
* Public Functions
********************************************************************************/
CcuMemMgr::CcuMemMgr()
{
    DBG_LOG_CONFIG(ccuif, ccu_drv);
}

bool CcuMemMgr::init()
{
    bool ret = true;
    
    LOG_DBG("+");

    m_ionDevFd = mt_ion_open("CcuMemMgr");
    if  ( 0 > m_ionDevFd )
    {
        LOG_ERR("mt_ion_open() return %d", m_ionDevFd);
        ret = false;
        goto MEM_MGR_EXIT;
    }

    reset();   

    LOG_DBG("-");

MEM_MGR_EXIT:
    return ret;
}

bool CcuMemMgr::uninit()
{
    bool ret = true;
    if(0 <= m_ionDevFd)
    {
        ion_close(m_ionDevFd);
    }

    return true;
}

bool CcuMemMgr::reset()
{
    bool ret = true;

    LOG_DBG("+");

    m_memHandle[CCU_BUFTYPE_DDRBIN].occupiedSize = 0;
    m_memHandle[CCU_BUFTYPE_DDRBIN].align_mva = 0;
    m_memHandle[CCU_BUFTYPE_CPUREF].occupiedSize = 0;
    m_memHandle[CCU_BUFTYPE_CPUREF].align_mva = 0;
    memset(m_ccuBufferRecs, 0, sizeof(struct CcuBuffer *) * CCU_BUFFER_COUNT_CAPACITY);
    m_ccuBufferRecIdx = 0;

    LOG_DBG("-");

MEM_MGR_EXIT:
    return ret;
}

bool CcuMemMgr::allocateSystemMem()
{
    bool ret = true;
    
    LOG_DBG("allocating memory of CCU BIN");
    ret = _ccuAllocMem(&m_memHandle[CCU_BUFTYPE_DDRBIN], CCU_CACHE_SIZE + CCU_CTRL_BUF_TOTAL_SIZE, false);
    if(!ret) goto ALLOC_SYS_MEM_EXIT;
    ret = _ccuAllocMva(&m_memHandle[CCU_BUFTYPE_DDRBIN], CCU_DDR_BUF_MVA_LOWER_BOUND, CCU_DDR_BUF_MVA_UPPER_BOUND);
    if(!ret) goto ALLOC_SYS_MEM_EXIT;

    LOG_DBG_MUST("memory of CCU BIN allocation success: ionHandle(%d), share_fd(%d), va(%p), mva(%p), size(%d), cached(%d)",
        m_memHandle[CCU_BUFTYPE_DDRBIN].ionHandle,
        m_memHandle[CCU_BUFTYPE_DDRBIN].shareFd,
        m_memHandle[CCU_BUFTYPE_DDRBIN].va,
        m_memHandle[CCU_BUFTYPE_DDRBIN].mva,
        m_memHandle[CCU_BUFTYPE_DDRBIN].size,
        m_memHandle[CCU_BUFTYPE_DDRBIN].cached);

    //since CPU will use those buffers, cached memory should be used
    LOG_DBG("allocating memory of CCU managed buffers for CPU reference");
    ret = _ccuAllocMem(&m_memHandle[CCU_BUFTYPE_CPUREF], CCU_CPUREF_BUF_TOTAL_SIZE, true);
    if(!ret) goto ALLOC_SYS_MEM_EXIT;
    ret = _ccuAllocMva(&m_memHandle[CCU_BUFTYPE_CPUREF], CCU_CTRL_BUFS_LOWER_BOUND, CCU_CTRL_BUFS_UPPER_BOUND);

    LOG_DBG_MUST("memory of CPU reference allocation success: ionHandle(%d), share_fd(%d), va(%p), mva(%p), size(%d), cached(%d)",
        m_memHandle[CCU_BUFTYPE_CPUREF].ionHandle,
        m_memHandle[CCU_BUFTYPE_CPUREF].shareFd,
        m_memHandle[CCU_BUFTYPE_CPUREF].va,
        m_memHandle[CCU_BUFTYPE_CPUREF].mva,
        m_memHandle[CCU_BUFTYPE_CPUREF].size,
        m_memHandle[CCU_BUFTYPE_CPUREF].cached);

    if ( m_memHandle[CCU_BUFTYPE_CPUREF].mva < m_memHandle[CCU_BUFTYPE_DDRBIN].mva )
    {
        LOG_ERR("ION allocated CCU_BUFTYPE_CPUREF(%x) > CCU_BUFTYPE_DDRBIN(%x)",
            m_memHandle[CCU_BUFTYPE_CPUREF].mva, m_memHandle[CCU_BUFTYPE_DDRBIN].mva);
        ret = false;
        goto ALLOC_SYS_MEM_EXIT;
    }
    LOG_DBG("memory address checking ok.");

    //Force align CPU reference buffer to CCU_CPUREF_BUF_ALIGN_SIZE base address
    if(ret) {
        m_memHandle[CCU_BUFTYPE_CPUREF].align_mva = (unsigned int)CCU_CPUREF_BUF_ALIGN_SIZE + (m_memHandle[CCU_BUFTYPE_CPUREF].mva & CCU_CPUREF_BUF_ALIGN_MASK);
        m_memHandle[CCU_BUFTYPE_CPUREF].occupiedSize = m_memHandle[CCU_BUFTYPE_CPUREF].align_mva - m_memHandle[CCU_BUFTYPE_CPUREF].mva;
        LOG_DBG_MUST("processing CPU reference alignment, align_size(%x) mva(%x) mva_align(%x) occup(%x)", 
            CCU_CPUREF_BUF_ALIGN_SIZE, 
            m_memHandle[CCU_BUFTYPE_CPUREF].mva,
            m_memHandle[CCU_BUFTYPE_CPUREF].align_mva,
            m_memHandle[CCU_BUFTYPE_CPUREF].occupiedSize);
    }
ALLOC_SYS_MEM_EXIT:
    return ret;
}

bool CcuMemMgr::deallocateSystemMem()
{
    bool ret = true;
    
    //deallocate memory from Android system for CCU instruction/data in DRAM
    LOG_DBG("deallocating memory of CCU BIN");
    ret = _ccuDeallocMem(&m_memHandle[CCU_BUFTYPE_DDRBIN]);
    if(!ret) goto DEALLOC_SYS_MEM_EXIT;
    ret = _ccuDeallocMva(&m_memHandle[CCU_BUFTYPE_DDRBIN]);
    if(!ret) goto DEALLOC_SYS_MEM_EXIT;

    //deallocate memory from Android system for CCU control command buffers
    //LOG_DBG("deallocating memory of CCU CtrlBufs");
    //ret = _ccuDeallocMem(&m_memHandle[CCU_BUFTYPE_CTRL]);
    //if(!ret) goto DEALLOC_SYS_MEM_EXIT;
    //ret = _ccuDeallocMva(&m_memHandle[CCU_BUFTYPE_CTRL]);

    LOG_DBG("deallocating memory of CCU managed buffers for CPU reference");
    ret = _ccuDeallocMem(&m_memHandle[CCU_BUFTYPE_CPUREF]);
    if(!ret) goto DEALLOC_SYS_MEM_EXIT;
    ret = _ccuDeallocMva(&m_memHandle[CCU_BUFTYPE_CPUREF]);

DEALLOC_SYS_MEM_EXIT:
    return ret;
}

bool CcuMemMgr::allocateCcuBuffer(struct CcuBuffer *buffer)
{
    android::Mutex::Autolock lock(_allocBufMutex);

    bool ret = true;
    struct CcuMemHandle *memHandlePtr;
    unsigned int remainingSize = 0;

    LOG_DBG("alloc info: name(%s), reqSize(%d), reqBufType(%d), cached(%d), ringCnt(%d)",
        buffer->name, buffer->size, buffer->bufType, buffer->cached, buffer->ringCnt);

    //check buffer type requested
    if((buffer->bufType <= CCU_BUFTYPE_NONE) || (buffer->bufType >= CCU_BUFTYPE_MAX))
    {
        LOG_ERR("invalid buffer type: %d", buffer->bufType);
        ret = false;
        goto ALLOC_CCU_BUF_EXIT;
    }
    
    //map bufType to corresponding memory handle to use
    memHandlePtr = &m_memHandle[buffer->bufType];

    //check remaining rec slot
    if(m_ccuBufferRecIdx >= CCU_BUFFER_COUNT_CAPACITY)
    {
        LOG_ERR("fail, out of buffer count, m_ccuBufferRecIdx(%d), recCapacity(%d)",
            m_ccuBufferRecIdx, CCU_BUFFER_COUNT_CAPACITY);
        ret = false;
        goto ALLOC_CCU_BUF_EXIT;
    }

    for(int i=0 ; i < buffer->ringCnt ; i++)
    {
        //check remaining size
        remainingSize = memHandlePtr->size - memHandlePtr->occupiedSize;
        if(remainingSize < buffer->size)
        {
            LOG_ERR("fail, out of remaining size, reqBufType(%d), reqSize(%d), remainingSize(%d), totalSize(%d)",
                buffer->bufType, buffer->size, remainingSize, memHandlePtr->size);
            ret = false;
            goto ALLOC_CCU_BUF_EXIT;
        }

        //slice address region for request
        buffer->va[i] = memHandlePtr->va + memHandlePtr->occupiedSize;
        buffer->mva[i] = memHandlePtr->mva + memHandlePtr->occupiedSize;
        memHandlePtr->occupiedSize += buffer->size;
    }
    buffer->cached = memHandlePtr->cached;

    //record this allocation in list
    m_ccuBufferRecs[m_ccuBufferRecIdx] = buffer;
    m_ccuBufferRecIdx++;

ALLOC_CCU_BUF_EXIT:
    return ret;
}

int CcuMemMgr::getIonDevFd()
{
    return m_ionDevFd;
}

ion_user_handle_t CcuMemMgr::getIonHandle()
{
    return m_memHandle[CCU_BUFTYPE_CPUREF].ionHandle;
}

bool CcuMemMgr::dumpAllocationTable()
{
    for(int i=CCU_BUFTYPE_MIN ; i<CCU_BUFTYPE_MAX ; i++)
    {
        unsigned int remainingSize = m_memHandle[i].size - m_memHandle[i].occupiedSize;
        LOG_DBG("bufType(%d), remainingSize(%d), occupiedSize(%d), totalSize(%d)",
            i,
            remainingSize, 
            m_memHandle[i].occupiedSize,
            m_memHandle[i].size);
    }
    
    LOG_DBG("RecIdx(%d)", m_ccuBufferRecIdx);
    for(int i=0 ; i<m_ccuBufferRecIdx ; i++)
    {
        for(int k=0 ; k < m_ccuBufferRecs[i]->ringCnt ; k++)
        {
            LOG_DBG("name(%s), size(%d), va(%p), mva(0x%x), bufType(%d), cached(%d)",
                m_ccuBufferRecs[i]->name,
                m_ccuBufferRecs[i]->size,
                m_ccuBufferRecs[i]->va[k],
                m_ccuBufferRecs[i]->mva[k],
                m_ccuBufferRecs[i]->bufType,
                m_ccuBufferRecs[i]->cached);
        }
        
    }

    return true;
}

bool CcuMemMgr::importMemoriesToKernel()
{
    import_mem_t import_buf;
    uint32_t importIdx = 0;

    CcuDrvImp *m_pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance();
    if ( NULL == m_pDrvCcu ) {
        LOG_ERR("createInstance(CCU_A)return %p", m_pDrvCcu);
        return false;
    }

    for (int i = 0; i < CCU_IMPORT_BUF_NUM; i++) {
        import_buf.memID[i] = CCU_IMPORT_BUF_UNDEF;
    }

    LOG_DBG_MUST("import buffers into kernel\n");    
    for(int i=CCU_BUFTYPE_MIN ; i<CCU_BUFTYPE_MAX ; i++)
    {
        importIdx = i-CCU_BUFTYPE_MIN;
        import_buf.memID[i-CCU_BUFTYPE_MIN] = m_memHandle[i].shareFd;
        LOG_DBG("importIdx(%d), bufType(%d), shareFd(%d)", importIdx, i, m_memHandle[i].shareFd);
    }

    m_pDrvCcu->importCcuMemToKernel(import_buf);
    LOG_DBG_MUST("import ccu memories to kernel done\n");

    return true;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuMemMgr::_ccuAllocMem(struct CcuMemHandle *memHandle, int size, bool cached)
{
    LOG_VRB("+");

    // allocate ion buffer handle
    if(ion_alloc_mm(m_ionDevFd, (size_t)size, 0, (cached)?3:0, &memHandle->ionHandle))
    {
        LOG_WRN("fail to get ion buffer handle (m_ionDevFd=0x%x, size=%d)", m_ionDevFd, size);
        return false;
    }

    // get ion buffer share handle
    if(ion_share(m_ionDevFd, memHandle->ionHandle, &memHandle->shareFd))
    {
        LOG_WRN("fail to get ion buffer share handle");
        if(ion_free(m_ionDevFd, memHandle->ionHandle))
            LOG_WRN("ion free fail");
        return false;
    }

    // get buffer virtual address
    memHandle->va = ( char *)ion_mmap(m_ionDevFd, NULL, (size_t)size, PROT_READ|PROT_WRITE, MAP_SHARED, memHandle->shareFd, 0);
    if(memHandle->va == NULL) {
        LOG_WRN("fail to get buffer virtual address");
        return false;
    }

    memHandle->size = size;
    memHandle->cached = cached;

    LOG_DBG("success: ionHandle(%d), share_fd(%d), va(%p), size(%d), cached(%d)",
        memHandle->ionHandle, memHandle->shareFd, memHandle->va, memHandle->size, memHandle->cached);

    LOG_VRB("-");

    return (memHandle->va != NULL) ? true : false;
}

bool CcuMemMgr::_ccuDeallocMem(struct CcuMemHandle *memHandle)
{

    //ion_user_handle_t buf_handle = {0};

    LOG_VRB("+");

    LOG_DBG("free import ion: ion_buf_handle %d, share_fd %d", memHandle->ionHandle, memHandle->shareFd);
    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    // if(ion_import(m_ionDevFd, memHandle->shareFd, &buf_handle))
    // {
    //     LOG_WRN("fail to get import share buffer fd");
    //     return false;
    // }
    // // 2. free for IMPORT ref cnt
    // if(ion_free(m_ionDevFd, buf_handle))
    // {
    //     LOG_WRN("fail to free ion buffer (free ion_import ref cnt)");
    //     return false;
    // }
    // 3. unmap virtual memory address
    if(ion_munmap(m_ionDevFd, (void *)memHandle->va, (size_t)memHandle->size))
    {
        LOG_WRN("fail to get unmap virtual memory");
        return false;
    }
    // 4. close share buffer fd
    if(ion_share_close(m_ionDevFd, memHandle->shareFd))
    {
        LOG_WRN("fail to close share buffer fd");
        return false;
    }
    // 5. pair of ion_alloc_mm
    if(ion_free(m_ionDevFd, memHandle->ionHandle))
    {
        LOG_WRN("fail to free ion buffer (free ion_alloc_mm ref cnt)");
        return false;
    }

    LOG_VRB("-");

    return true;
}

bool CcuMemMgr::_ccuAllocMva(struct CcuMemHandle *memHandle, unsigned int startAddr, unsigned int endAddr)
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_VRB("+");

    //a. get handle from IonBufFd and increase handle ref count
    /*if(ion_import(m_ionDevFd, memHandle->shareFd, memHandle->ionHandle))
    {
        LOG_ERR("ion_import fail, ion_handle(0x%x)", memHandle->ionHandle);
        return false;
    }
    LOG_DBG("ion_import: share_fd %d, ion_handle %d", memHandle->shareFd, memHandle->ionHandle);
    */
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = memHandle->ionHandle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_CCU0;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start  = startAddr;
    mm_data.config_buffer_param.reserve_iova_end    = endAddr;
    err = ion_custom_ioctl(m_ionDevFd, ION_CMD_MULTIMEDIA, &mm_data);
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        LOG_ERR("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        LOG_ERR("ion_custom_ioctl ION_CMD_MULTIMEDIA failed, ionDevFd(%d), ionhandle(%d), startAddr(%x), endAddr(%x)", 
            m_ionDevFd, memHandle->ionHandle, startAddr, endAddr);
    }
    //c. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = memHandle->ionHandle;
    sys_data.get_phys_param.phy_addr = (M4U_PORT_CCU0<<24) | ION_FLAG_GET_FIXED_PHYS;
    sys_data.get_phys_param.len = ION_FLAG_GET_FIXED_PHYS;
    if(ion_custom_ioctl(m_ionDevFd, ION_CMD_SYSTEM, &sys_data))
    {
        LOG_ERR("ion_custom_ioctl get_phys_param failed!");
        return false;
    }
    //
    memHandle->mva = (unsigned int)sys_data.get_phys_param.phy_addr;

    LOG_DBG("success: mva(%x)", memHandle->mva);

    LOG_VRB("-");

    return true;
}

bool CcuMemMgr::_ccuDeallocMva(struct CcuMemHandle *memHandle)
{
    LOG_VRB("+");

    LOG_DBG("ion_free: ion_handle %d", memHandle->ionHandle);
    // decrease handle ref count
    if(ion_free(m_ionDevFd, memHandle->ionHandle))
    {
        LOG_ERR("ion_free fail");
        return false;
    }

    LOG_VRB("-");
    return true;
}

unsigned int CcuMemMgr::getAlignMva(enum CCU_BUFFER_TYPE){
    return m_memHandle[CCU_BUFTYPE_CPUREF].align_mva;
}

}//end of NSCcuIf