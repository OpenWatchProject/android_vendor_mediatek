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
#define LOG_TAG "CcuBuffer"

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/properties.h>  // For property_get().
#include "ccu_buffer.h"
/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

CcuBufferList::
CcuBufferList():
    isValid(false),
    m_IonDevFD(0),
    m_featureType(CCU_FEATURE_UNDEF)
    {
        mailboxInBuf.reset();
        mailboxOutBuf.reset();

        for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
        {
            CtrlMsgBufs[i].reset();
        }
        LOG_DBG("CcuBufferList reset all buffers in constructor");
    }

bool CcuBufferList::init(int ionDevFd)
{
    m_IonDevFD = ionDevFd;

    //Set mailbox in/out buffer size, and initialize them
    this->mailboxInBuf.size = MAILBOX_IN_BUF_SIZE;
    this->mailboxOutBuf.size = MAILBOX_OUT_BUF_SIZE;
    LOG_DBG("init mailboxInBuf");
    if(this->mailboxInBuf.init(m_IonDevFD, 0) == false)
        return false;
    LOG_DBG("init mailboxOutBuf");
    if(this->mailboxOutBuf.init(m_IonDevFD, 0) == false)
        return false;
    LOG_DBG("init initControlBuffers");
    //Initialize control command buffers, according to CCU_CTRL_PARAM_DESCRIPTOR_LIST
    if(_initControlBuffers() == false)
        return false;

    isValid = true;

    return true;
}

bool CcuBufferList::uninit()
{
    isValid = false;

    if(this->mailboxInBuf.uninit() == false)
        return false;

    if(this->mailboxOutBuf.uninit() == false)
        return false;

    if(_uninitControlBuffers() == false)
        return false;

    return true;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuBufferList::_initControlBuffers()
{
    bool ret = true;
    struct CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        //map buffer from CtrlMsgBufs and clear buffer info
        buffer = &(this->CtrlMsgBufs[i]);
        memset(buffer, 0, sizeof(struct CcuBuffer));

        //check if command available
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        //check if command needs input ptr buffer copying
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->intput_ptr_descs == NULL) continue;

        LOG_DBG("init_control_buffers[%d]: %s\n", i, CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->msg_literal);

        buffer->size = CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->buffer_size;
        ret = buffer->init(m_IonDevFD, 0);
        if(ret == false)
        {
            LOG_ERR("init_control_buffers[%d] fail\n", i);
            break;
        }
    }

    LOG_DBG("-");

    return ret;
}

bool CcuBufferList::_uninitControlBuffers()
{
    bool ret = true;
    CcuBuffer *buffer;

    LOG_DBG("+");

    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        //check if command available
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        //check if command needs input ptr buffer copying
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->intput_ptr_descs == NULL) continue;

        buffer = &(this->CtrlMsgBufs[i]);

        LOG_DBG("uninit_control_buffers[%d]\n", i);

        ret = buffer->uninit();
        if(ret == false)
        {
            LOG_ERR("uninit_control_buffers[%d] fail\n", i);
            break;
        }
    }

    LOG_DBG("-");

    return ret;
}

MINT32  CcuBufferList::doIonCacheFlush(MINT32 memID,eIONCacheFlushType /*flushtype*/)
{
    #if defined (__ISP_USE_ION__)
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    MINT32 err;
    CAM_LOGD_IF( m_3AMemLogEnable,"+");
    //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
    IonBufFd = memID;
    if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
    {
        CAM_LOGE("ion_import fail,memID(0x%x)",IonBufFd);
        return -1;
    }

    //b. cache sync by range
    struct ion_sys_data sys_data;
    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=(ion_user_handle_t)pIonHandle;
    sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
    if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
    {
        CAM_LOGE("CAN NOT DO SYNC, memID/(0x%x)",memID);
        if(ion_free(mIonDrv,pIonHandle))
        {
            CAM_LOGE("ion_free fail");
            return -1;
        }
        return -1;
    }

    //c. decrease handle ref count
    if(ion_free(mIonDrv,pIonHandle))
    {
        CAM_LOGE("ion_free fail");
        return -1;
    }
    CAM_LOGD_IF( m_3AMemLogEnable,"-");
    #endif
    return 0;
}

/*******************************************************************************
* Imp. of CcuBuffer function
********************************************************************************/

bool CcuBuffer::_ccuAllocBuffer(int drv_h, int len, int *buf_share_fd, char **buf_va, bool cached)
{
    ion_user_handle_t buf_handle;

    LOG_VRB("+");

    // allocate ion buffer handle
    if(ion_alloc_mm(drv_h, (size_t)len, 0, (cached)?3:0, &buf_handle))
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
    LOG_VRB("-");

    return (*buf_va != NULL) ? true : false;
}

bool CcuBuffer::_ccuFreeBuffer(int drv_h, int len,int buf_share_fd, char *buf_va)
{
    ion_user_handle_t buf_handle = {0};

    LOG_VRB("+");

    LOG_DBG("free import ion: ion_buf_handle %d, share_fd %d", buf_handle, buf_share_fd);
    // 1. get handle of ION_IOC_SHARE from fd_data.fd
    if(ion_import(drv_h, buf_share_fd, &buf_handle))
    {
        LOG_WRN("fail to get import share buffer fd");
        return false;
    }
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

    LOG_VRB("-");

    return true;
}

bool CcuBuffer::_mmapMva( int buf_share_fd, ion_user_handle_t *p_ion_handle, unsigned int *mva, 
                        unsigned int start_addr, unsigned int end_addr)
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_VRB("+");

    //a. get handle from IonBufFd and increase handle ref count
    if(ion_import(m_IonDevFD, buf_share_fd, p_ion_handle))
    {
        LOG_ERR("ion_import fail, ion_handle(0x%x)", *p_ion_handle);
        return false;
    }
    LOG_DBG("ion_import: share_fd %d, ion_handle %d", buf_share_fd, *p_ion_handle);
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER_EXT;
    mm_data.config_buffer_param.handle      = *p_ion_handle;
    mm_data.config_buffer_param.eModuleID   = M4U_PORT_CAM_CCUG;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 1;
    mm_data.config_buffer_param.reserve_iova_start  = start_addr;
    mm_data.config_buffer_param.reserve_iova_end    = end_addr;
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

    LOG_VRB("-");

    return true;
}

bool CcuBuffer::_munmapMva( ion_user_handle_t ion_handle )
{
    LOG_VRB("+");

    // decrease handle ref count
    if(ion_free(m_IonDevFD, ion_handle))
    {
        LOG_ERR("ion_free fail");
        return false;
    }
    LOG_DBG("ion_free: ion_handle %d", ion_handle);

    LOG_VRB("-");
    return true;
}

void CcuBuffer::reset()
{
    share_fd = 0;
    ion_handle = 0;
    va = NULL;
    mva = 0;
    size = 0;
}

bool CcuBuffer::init(int ionDevFd, bool cached, uint32_t lowerBound, uint32_t upperBound)
{
    LOG_VRB("+");

    struct CcuBuffer *buffer = this;
    m_IonDevFD = ionDevFd;

    _ccuAllocBuffer( m_IonDevFD, buffer->size, &buffer->share_fd, &buffer->va, cached);
    if ( NULL == buffer->va ) {
        LOG_ERR("ccu_get_buffer va error: %p", buffer->va);
        return false;
    }
    _mmapMva( buffer->share_fd, &buffer->ion_handle, &buffer->mva, lowerBound, upperBound);
    if ( 0 == buffer->mva) {
        LOG_ERR("mmapMVA error: %x", buffer->mva);
        return false;
    }

    LOG_DBG("size(%d), va(%p), mva(%x), shred_fd(%d)\n", buffer->size, buffer->va, buffer->mva, buffer->share_fd);

    LOG_VRB("-");
    return true;
}

bool CcuBuffer::uninit()
{
    LOG_VRB("+");

    struct CcuBuffer *buffer = this;

    if ( false == _munmapMva( buffer->ion_handle ) ) {
        LOG_ERR("munmapMVA fail\n");
    }

    if ( false == _ccuFreeBuffer( m_IonDevFD, buffer->size, buffer->share_fd, buffer->va) ) {
        LOG_ERR("ccu_free_buffer fail\n");
    }

    LOG_VRB("-");

    return true;
}

};  //namespace NSCcuIf

