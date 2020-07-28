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
#define LOG_TAG "CcuMgr"

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include "ccu_mgr.h"
//#include "isp_ccu_reg.h"

#include <cutils/properties.h>  // For property_get().

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

DECLARE_DBG_LOG_VARIABLE(ccu_drv);

static CcuMgr *_ccuMgrSingleton = NULL;
static int32_t _ccuMgrUserCnt = 0;
static android::Mutex _ccuMgrCreationMutex;
static int32_t _ccuBootCnt = 0;
static int32_t _ccuInitCnt = 0;

/*******************************************************************************
* Singleton Factory Function
********************************************************************************/

ICcuMgrExt *ICcuMgrExt::createInstance(const char userName[32])
{
    LOG_INF_MUST("+ User: %s\n", userName);

    /*ICcuMgrExt *ccuMgrPtr;

    ccuMgrPtr = (ICcuMgrExt *)_createCcuMgr();

    return ccuMgrPtr;*/

    //return (ICcuMgrExt *)_createCcuMgr();
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    _ccuMgrUserCnt += 1;
    LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);

    if(_ccuMgrSingleton == NULL)
    {
        _ccuMgrSingleton = new CcuMgr();
    }

    return (ICcuMgrExt *)_ccuMgrSingleton;

    LOG_INF_MUST("-\n");
}

bool ICcuMgrExt::ccuIsSupportSecurity()
{
    return CCU_SECURITY_SUPPORT;
}

ICcuMgrPriv *ICcuMgrPriv::createInstance()
{
    /*ICcuMgrPriv *ccuMgrPtr;

    ccuMgrPtr = (ICcuMgrPriv *)_createCcuMgr();

    return ccuMgrPtr;*/

    //return (ICcuMgrPriv *)_createCcuMgr();

    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    _ccuMgrUserCnt += 1;
    LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);

    if(_ccuMgrSingleton == NULL)
    {
        _ccuMgrSingleton = new CcuMgr();
    }

    return (ICcuMgrPriv *)_ccuMgrSingleton;
}

static void _destroyCcuMgr()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    _ccuMgrUserCnt -= 1;
    LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);

    if((_ccuMgrSingleton != NULL) && (_ccuMgrUserCnt == 0))
    {
        delete _ccuMgrSingleton;
        _ccuMgrSingleton = NULL;
    }
}

void ICcuMgrExt::destroyInstance(void)
{
    LOG_INF_MUST("+\n");
    _destroyCcuMgr();
    /*android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    _ccuMgrUserCnt -= 1;
    LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);

    if((_ccuMgrSingleton != NULL) && (_ccuMgrUserCnt == 0))
    {
        delete _ccuMgrSingleton;
        _ccuMgrSingleton = NULL;
    }*/
    LOG_INF_MUST("-\n");
}

void ICcuMgrPriv::destroyInstance(void)
{
    LOG_INF_MUST("+\n");
    _destroyCcuMgr();
    LOG_INF_MUST("-\n");
}

/*******************************************************************************
* Constructor
********************************************************************************/
CcuMgr::CcuMgr()
{
    DBG_LOG_CONFIG(ccuif, ccu_drv);

    m_BufferList.isValid = false;

    _checkCcuEnable();
}

/*******************************************************************************
* Public Functions
********************************************************************************/

int CcuMgr::ccuInit()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);

    LOG_INF_MUST("+\n");

    if(_ccuInitCnt == 0)
    {
        if(!m_CcuEnable)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        m_IonDevFD = mt_ion_open("CcuMgr");
        if  ( 0 > m_IonDevFD )
        {
            LOG_ERR("mt_ion_open() return %d", m_IonDevFD);
            return false;
        }

        m_pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance(CCU_A);
        if ( NULL == m_pDrvCcu ) {
            LOG_ERR("createInstance(CCU_A)return %p", m_pDrvCcu);
            return false;
        }

        //Clear bufferlist
        memset(&m_BufferList, 0, sizeof(struct CcuBufferList));

        //Set mailbox in/out buffer size, and initialize them
        _setBufferSizes();
        if(_initBuffer(&(m_BufferList.mailboxInBuf)) == MFALSE)
            return false;
        if(_initBuffer(&(m_BufferList.mailboxOutBuf)) == MFALSE)
            return false;

        //Initialize control command buffers, according to CCU_CTRL_PARAM_DESCRIPTOR_LIST
        if(_initControlBuffers() == MFALSE)
            return false;

        LOG_INF("m_BufferList.mailboxInBuf.va: %p\n", m_BufferList.mailboxInBuf.va);
        LOG_INF("m_BufferList.mailboxInBuf.mva: %x\n", m_BufferList.mailboxInBuf.mva);
        LOG_INF("m_BufferList.mailboxOutBuf.va: %p\n", m_BufferList.mailboxOutBuf.va);
        LOG_INF("m_BufferList.mailboxOutBuf.mva: %x\n", m_BufferList.mailboxOutBuf.mva);

        m_BufferList.isValid = true;
    }
    else
        LOG_DBG_MUST("already init\n");
    _ccuInitCnt = _ccuInitCnt + 1;
    LOG_INF_MUST("-\n");

    return  0;
}

int CcuMgr::ccuUninit()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);

    LOG_DBG_MUST("+");
    MBOOL ret;
    CcuBuffer *buffer;

    if(_ccuInitCnt == 1)
    {
        if(!m_CcuEnable)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        if(_uninitBuffer(&(m_BufferList.mailboxInBuf)) == MFALSE)
            return false;

        if(_uninitBuffer(&(m_BufferList.mailboxOutBuf)) == MFALSE)
            return false;

        if(_uninitControlBuffers() == MFALSE)
            return false;

        //
        if (m_pDrvCcu) {
            m_pDrvCcu->destroyInstance();
        }
        //
        if( 0 <= m_IonDevFD ) {
            ion_close(m_IonDevFD);
        }

        LOG_DBG_MUST("-: CcuMgr uninitial done gracefully\n");
    }
    else
        LOG_DBG_MUST("another user is working\n");
    _ccuInitCnt = _ccuInitCnt - 1;
    return  0;
}

int CcuMgr::ccuBoot()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);

    MBOOL rlt;

    LOG_DBG_MUST("+:\n");

    if(_ccuBootCnt == 0)
    {
        if(!m_CcuEnable)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        rlt = m_pDrvCcu->init("CcuMgr");
        if(rlt == MFALSE)
        {
            LOG_ERR("m_pDrvCcu->init(), failed: %x\n", rlt);
            return -CCU_MGR_ERR_UNKNOWN;
        }
    }
    else
        LOG_DBG_MUST("already boot or illegal boot (%x)\n", _ccuBootCnt);
    _ccuBootCnt = _ccuBootCnt + 1;
    LOG_DBG_MUST("-:\n");

    return 0;
}

int CcuMgr::ccuShutdown()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);

    MBOOL rlt;

    LOG_DBG_MUST("+");

    if(_ccuBootCnt == 1)
    {
        if(!m_CcuEnable)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        rlt = m_pDrvCcu->shutdown();
        if ( MFALSE == rlt )
        {
            LOG_ERR("m_pDrvCcu->shutdown() failed: %x", rlt);
        }

        rlt = m_pDrvCcu->powerOff();
         if ( MFALSE == rlt ) {
            LOG_ERR("m_pDrvCcu->powerOff() failed: %x", rlt);
        }

        rlt = m_pDrvCcu->uninit("CCU");
        if ( MFALSE == rlt ) {
            LOG_ERR("m_pDrvCcu->uninit() failed: %x", rlt);
        }
    }
    else
        LOG_DBG_MUST("another user is working or illegal shudown! (%x)\n", _ccuBootCnt);
    _ccuBootCnt = _ccuBootCnt - 1;
    LOG_DBG_MUST("-");

    return 0;
}

struct CcuBufferList CcuMgr::getCcuBufferList()
{
    return m_BufferList;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
void CcuMgr::_checkCcuEnable()
{
    m_CcuEnable = MFALSE;

    char value[100] = {'\0'};

    property_get("vendor.debug.ccu_mgr_ccu.enable", value, "1");
    m_CcuEnable = atoi(value);
}

MBOOL CcuMgr::_ccuAllocBuffer(int drv_h, int len, int *buf_share_fd, char **buf_va)
{
    ion_user_handle_t buf_handle = {};

    LOG_DBG("+");

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
    LOG_DBG("alloc ion: ion_buf_handle %d, share_fd %d, va: %p", buf_handle, *buf_share_fd, *buf_va);
    //
    LOG_DBG("-");

    return (*buf_va != NULL) ? MTRUE : MFALSE;
}

MBOOL CcuMgr::_ccuFreeBuffer(int drv_h, int len,int buf_share_fd, char *buf_va)
{
    ion_user_handle_t buf_handle;

    LOG_DBG("+");

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

    LOG_DBG("-");

    return true;
}

MBOOL CcuMgr::_mmapMva( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva )
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_DBG("+");

    //a. get handle from IonBufFd and increase handle ref count
    if(ion_import(m_IonDevFD, buf_share_fd, p_ion_handle))
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
    err = ion_custom_ioctl(m_IonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
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
    if(ion_custom_ioctl(m_IonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        LOG_ERR("ion_custom_ioctl get_phys_param failed!");
        return false;
    }
    //
    *mva = (unsigned int)sys_data.get_phys_param.phy_addr;

    LOG_DBG("-");

    return true;
}

MBOOL CcuMgr::_munmapMva( ion_user_handle_t ion_handle )
{
    LOG_DBG("+");

    // decrease handle ref count
    if(ion_free(m_IonDevFD, ion_handle))
    {
        LOG_ERR("ion_free fail");
        return false;
    }
    LOG_VRB("ion_free: ion_handle %d", ion_handle);

    LOG_DBG("-");
    return true;
}

MBOOL CcuMgr::_setBufferSizes()
{
    m_BufferList.mailboxInBuf.size = 1024;
    m_BufferList.mailboxOutBuf.size = 1024;

    return true;
}

MBOOL CcuMgr::_initBuffer(struct CcuBuffer *buffer)
{
    LOG_DBG("+");

    _ccuAllocBuffer( m_IonDevFD, buffer->size, &buffer->share_fd, &buffer->va);
    if ( NULL == buffer->va ) {
        LOG_ERR("ccu_get_buffer va error: %p", buffer->va);
        return MFALSE;
    }
    _mmapMva( buffer->share_fd, &buffer->ion_handle, &buffer->mva);
    if ( 0 == buffer->mva) {
        LOG_ERR("mmapMVA error: %x", buffer->mva);
        return MFALSE;
    }

    LOG_INF("va: %p\n", buffer->va);
    LOG_INF("mva: %x\n", buffer->mva);

    LOG_DBG("-");
    return MTRUE;
}

MBOOL CcuMgr::_uninitBuffer(struct CcuBuffer *buffer)
{
    LOG_DBG("+");

    if ( false == _munmapMva( buffer->ion_handle ) ) {
        LOG_ERR("munmapMVA fail\n");
    }

    if ( false == _ccuFreeBuffer( m_IonDevFD, buffer->size, buffer->share_fd, buffer->va) ) {
        LOG_ERR("ccu_free_buffer fail\n");
    }

    LOG_DBG("-");

    return MTRUE;
}

MBOOL CcuMgr::_initControlBuffers()
{
    MBOOL ret = MTRUE;
    CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        //check if command available
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        //check if command needs input ptr buffer copying
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->intput_ptr_descs == NULL) continue;

        LOG_DBG("init_control_buffers[%d]\n", i);

        //map buffer from m_BufferList
        buffer = &(m_BufferList.CtrlMsgBufs[i]);
        buffer->size = CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->buffer_size;
        ret = this->_initBuffer(buffer);
        if(ret == MFALSE)
        {
            LOG_ERR("init_control_buffers[%d] fail\n", i);
            break;
        }
    }

    LOG_DBG("-");

    return ret;
}

MBOOL CcuMgr::_uninitControlBuffers()
{
    MBOOL ret = MTRUE;
    CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        //check if command available
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        //check if command needs input ptr buffer copying
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->intput_ptr_descs == NULL) continue;

        buffer = &(m_BufferList.CtrlMsgBufs[i]);

        LOG_DBG("uninit_control_buffers[%d]\n", i);

        ret = this->_uninitBuffer(buffer);
        if(ret == MFALSE)
        {
            LOG_ERR("uninit_control_buffers[%d] fail\n", i);
            break;
        }
    }

    LOG_DBG("-");

    return ret;
}

};  //namespace NSCcuIf

