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
#include "ccu_drvutil.h"
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

static CcuMgr _ccuMgrSingletonInstance;
static CcuMgr *_ccuMgrSingleton = &_ccuMgrSingletonInstance;
static int32_t _ccuMgrUserCnt = 0;
static android::Mutex _ccuMgrCreationMutex;
static int32_t _ccuBootCnt = 0;
static int32_t _ccuInitCnt = 0;
bool CcuMgr::IsCcuMgrEnabled;

/*******************************************************************************
* Singleton Factory Function
********************************************************************************/
ICcuMgrExt *ICcuMgrExt::createInstance(const char userName[32])
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    LOG_INF_MUST("IsCcuMgrEnabled: %d\n", CcuMgr::IsCcuMgrEnabled);

    if(CcuMgr::IsCcuMgrEnabled)
    {
        _ccuMgrUserCnt += 1;
        LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);
        return (ICcuMgrExt *)_ccuMgrSingleton;
    }
    else
        return NULL;
}

ICcuMgrPriv *ICcuMgrPriv::createInstance()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    LOG_INF_MUST("IsCcuMgrEnabled: %d\n", CcuMgr::IsCcuMgrEnabled);

    if(CcuMgr::IsCcuMgrEnabled)
    {
        _ccuMgrUserCnt += 1;
        LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);
        return (ICcuMgrPriv *)_ccuMgrSingleton;
    }
    else 
        return NULL;
}

bool ICcuMgrExt::ccuIsSupportSecurity()
{
    return CCU_SECURITY_SUPPORT;
}

static void _destroyCcuMgr()
{
    android::Mutex::Autolock lock(_ccuMgrCreationMutex);
    _ccuMgrUserCnt -= 1;
    LOG_INF_MUST("CcuMgr User Cnt: %d\n", _ccuMgrUserCnt);
}

void ICcuMgrExt::destroyInstance(void)
{
    LOG_INF_MUST("+\n");
    _destroyCcuMgr();
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

    _checkCcuEnable();

    m_isCcuBooted = false;
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
        if(!CcuMgr::IsCcuMgrEnabled)
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

        if(_initBufferLists(m_IonDevFD) == false)
            return false;
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
    bool ret;
    CcuBuffer *buffer;

    if(_ccuInitCnt == 1)
    {
        if(!CcuMgr::IsCcuMgrEnabled)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        if(_uninitBufferLists() == false)
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

    bool rlt;

    LOG_DBG_MUST("+:\n");

    if(_ccuBootCnt == 0)
    {
        if(!CcuMgr::IsCcuMgrEnabled)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        rlt = m_pDrvCcu->init("CcuMgr", m_logBuffers[0].mva, m_logBuffers[1].mva, 
            (MINT8 *)m_logBuffers[0].va, (MINT8 *)m_logBuffers[1].va,
            m_ddrBuffer.mva, (MINT8 *)m_ddrBuffer.va);
        if(rlt == false)
        {
            LOG_ERR("m_pDrvCcu->init(), failed: %x\n", rlt);
            m_pDrvCcu->TriggerAee("ccuBoot failed");
            return -CCU_MGR_ERR_UNKNOWN;
        }

        _importBuffersToKernel();
        m_isCcuBooted = true;
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

    bool rlt;

    LOG_DBG_MUST("+");

    if(_ccuBootCnt == 1)
    {
        if(!CcuMgr::IsCcuMgrEnabled)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        rlt = m_pDrvCcu->shutdown();
        if ( false == rlt )
        {
            LOG_ERR("m_pDrvCcu->shutdown() failed: %x", rlt);
        }

        rlt = m_pDrvCcu->powerOff();
         if ( false == rlt ) {
            LOG_ERR("m_pDrvCcu->powerOff() failed: %x", rlt);
        }

        rlt = m_pDrvCcu->uninit("CCU");
        if ( false == rlt ) {
            LOG_ERR("m_pDrvCcu->uninit() failed: %x", rlt);
        }

        m_isCcuBooted = false;
    }
    else
        LOG_DBG_MUST("another user is working or illegal shudown! (%x)\n", _ccuBootCnt);
    _ccuBootCnt = _ccuBootCnt - 1;
    LOG_DBG_MUST("-");

    return 0;
}

int CcuMgr::ccuSuspend(ESensorDev_T sensorDev, MUINT32 sensorIdx)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    if(!isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -1;
    }

    enum ccu_tg_info tgInfo = CcuDrvUtil::sensorDevToTgInfo(sensorDev, sensorIdx);

    struct ccu_msg msg = {MSG_TO_CCU_SUSPEND, NULL, NULL, tgInfo};

    if(!m_pDrvCcu->sendCmdIpc(&msg))
    {
        LOG_ERR("cmd(%d) fail \n", msg.msg_id);
        return -1;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return 0;
};

int CcuMgr::ccuResume(ESensorDev_T sensorDev, MUINT32 sensorIdx)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    if(!isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return -1;
    }

    enum ccu_tg_info tgInfo = CcuDrvUtil::sensorDevToTgInfo(sensorDev, sensorIdx);

    struct ccu_msg msg = {MSG_TO_CCU_RESUME, NULL, NULL, tgInfo};

    if(!m_pDrvCcu->sendCmdIpc(&msg))
    {
        LOG_ERR("cmd(%d) fail \n", msg.msg_id);
        return -1;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return 0;
};

struct CcuBufferList *CcuMgr::getCcuBufferList(enum ccu_feature_type ccuFeature, enum ccu_tg_info tgInfo)
{
    int tgIdx = CCU_TG2IDX(tgInfo);

    if(tgIdx>=CCU_CAM_TG_MIN_IDX && tgIdx <CCU_CAM_TG_MAX_IDX)
    {
        switch(ccuFeature)
        {
            case CCU_FEATURE_AE:
                LOG_INF_MUST("get buffer list [%d] of AE", tgIdx);
                return &m_AeBufferList[tgIdx];
            case CCU_FEATURE_AF:
                LOG_INF_MUST("get buffer list [%d] of AF", tgIdx);
                return &m_AfBufferList[tgIdx];
            case CCU_FEATURE_3ASYNC:
                LOG_INF_MUST("get buffer list [%d] of 3ASync", tgIdx);
                return &m_3asyncBufferList;
            default:
                LOG_ERR("feature type invalid: feature type(%d), tgInfo(%d)", ccuFeature, tgInfo);
                return NULL;
        }
        
    }
    else
    {
        LOG_ERR("tgInfo invalid: feature type(%d), tgInfo(%d)", ccuFeature, tgInfo);
        return NULL;
    }
}

bool CcuMgr::returnCcuBufferList(enum ccu_feature_type ccuFeature, enum ccu_tg_info tgInfo)
{
    return true;
}

bool CcuMgr::isCcuBooted()
{
    return m_isCcuBooted;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
void CcuMgr::_checkCcuEnable()
{
    CcuMgr::IsCcuMgrEnabled = false;

    char value[100] = {'\0'};

    property_get("vendor.debug.ccu_mgr_ccu.enable", value, "1");
    CcuMgr::IsCcuMgrEnabled = atoi(value);

#if defined(DISABLE_CCU_IN_ACTIVE_STEREO_LOAD)
    CcuMgr::IsCcuMgrEnabled = 0;
#endif
}

bool CcuMgr::_initBufferLists(int ionDevFd)
{
    bool ret;
    //dump cmd buffer usage info
    LOG_DBG("dump buffer usage info");
    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        LOG_DBG("cmd(%d/0x%x) %s", i, i, CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->msg_literal);
        LOG_DBG("inDataSzie(%d), outDataSzie(%d), ptrBufSzie(%d), ptrCount(%d)",
            CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->input_non_ptr_total_size,
            CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->output_non_ptr_total_size,
            CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->buffer_size,
            CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->ptr_count);
    }

    LOG_DBG_MUST("init AE buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        LOG_DBG_MUST("init AE buffer lists[%d]\n", i);
        ret = m_AeBufferList[i].init(ionDevFd);
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init AF buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        LOG_DBG_MUST("init AF buffer lists[%d]\n", i);
        ret = m_AfBufferList[i].init(ionDevFd);
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init 3ASync buffer lists\n");
    ret = m_3asyncBufferList.init(ionDevFd);
    if(ret == false)
        goto INIT_BUF_LIST_EXIT;

    for(int i=0 ; i<CCU_DRAM_LOG_BUF_CNT ; i++)
    {
        LOG_DBG_MUST("init log buffers[%d]\n", i);
        m_logBuffers[i].size = CCU_DRAM_LOG_BUF_SIZE;
        ret = m_logBuffers[i].init(ionDevFd, 0);
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init DDR buffer\n");
    m_ddrBuffer.size = CCU_CACHE_SIZE;
    ret = m_ddrBuffer.init(ionDevFd, 0, CCU_DDR_BUF_MVA_LOWER_BOUND, CCU_DDR_BUF_MVA_UPPER_BOUND);
    if (( m_ddrBuffer.mva < CCU_DDR_BUF_MVA_LOWER_BOUND) || ( m_ddrBuffer.mva > CCU_DDR_BUF_MVA_UPPER_BOUND))
    {
        LOG_ERR("ION allocated CCU DDR buffer MVA exceeds requested region, expected(0x%x~0x%x), actual(0x%x)",
            CCU_DDR_BUF_MVA_LOWER_BOUND, CCU_DDR_BUF_MVA_UPPER_BOUND, m_ddrBuffer.mva);
        AEE_ASSERT_CCU_USER("ION allocated CCU DDR buffer MVA exceeds requested region, expected(0x%x~0x%x), actual(0x%x)",
            CCU_DDR_BUF_MVA_LOWER_BOUND, CCU_DDR_BUF_MVA_UPPER_BOUND, m_ddrBuffer.mva);
        ret = false;
    }
    if(ret == false)
        goto INIT_BUF_LIST_EXIT;
    LOG_INF_MUST("CCU DDR buffer:va(%p),mva(%x),sz(%d)\n", m_ddrBuffer.va, m_ddrBuffer.mva, m_ddrBuffer.size);

INIT_BUF_LIST_EXIT:
    return ret;
}

bool CcuMgr::_importBuffersToKernel()
{
    import_mem_t import_buf;
    uint32_t importIdx = 0;

    for (int i = 0; i < CCU_IMPORT_BUF_NUM; i++) {
        import_buf.memID[i] = CCU_IMPORT_BUF_UNDEF;
    }

    //import buffers into kernel driver, thus kdrv cam handle HW stop & buffer release in feasible order while abnormal shutdown
    LOG_DBG_MUST("import AE buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        _importBufferListToKernel(&m_AeBufferList[i], &import_buf, &importIdx);
    }

    LOG_DBG_MUST("import AF buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        _importBufferListToKernel(&m_AfBufferList[i], &import_buf, &importIdx);
    }

    LOG_DBG_MUST("import AFO buffers\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
        for(int j = 0; j<2 ;j++)
            for(int k = 0; k<3 ;k++)
            {
                import_buf.memID[importIdx++] = m_AfBufferList[i].AF_Buf[j][k].share_fd;
            }

    LOG_DBG_MUST("import 3ASync buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        _importBufferListToKernel(&m_3asyncBufferList, &import_buf, &importIdx);
    }

    LOG_DBG_MUST("import log & ddr buffers\n");
    import_buf.memID[importIdx++] = m_logBuffers[0].share_fd;
    import_buf.memID[importIdx++] = m_logBuffers[1].share_fd;
    import_buf.memID[importIdx++] = m_ddrBuffer.share_fd;

    LOG_DBG_MUST("import buffers into kernel, total cnt(%d)\n", importIdx);
    m_pDrvCcu->importmem(import_buf);

    return true;
}

bool CcuMgr::_importBufferListToKernel(struct CcuBufferList *list, import_mem_t *import_buf, uint32_t *importIdx)
{
    struct CcuBuffer *buffer;

    for(int i=0 ; i<CCU_EXT_MSG_COUNT ; i++)
    {
        //check if command available
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i] == NULL) continue;
        //check if command needs input ptr buffer copying
        if(CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->intput_ptr_descs == NULL) continue;

        buffer = &(list->CtrlMsgBufs[i]);

        LOG_DBG("import_control_buffers[%d]: %s\n", i, CCU_CTRL_PARAM_DESCRIPTOR_LIST[i]->msg_literal);
        import_buf->memID[*importIdx] = buffer->share_fd;
        *importIdx = (*importIdx) + 1;
    }

    return true;
}

bool CcuMgr::_uninitBufferLists()
{
    bool ret;

    LOG_DBG_MUST("uninit AE buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        ret = m_AeBufferList[i].uninit();
        if(ret == false)
            goto UNINIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("uninit AF buffer lists\n");
    for(int i=CCU_CAM_TG_MIN_IDX ; i<CCU_CAM_TG_MAX_IDX ; i++)
    {
        ret = m_AfBufferList[i].uninit();
        if(ret == false)
            goto UNINIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("uninit 3ASync buffer lists\n");
    ret = m_3asyncBufferList.uninit();
    if(ret == false)
        goto UNINIT_BUF_LIST_EXIT;

    for(int i=0 ; i<CCU_DRAM_LOG_BUF_CNT ; i++)
    {
        LOG_DBG_MUST("uninit log buffers[%d]\n", i);
        ret = m_logBuffers[i].uninit();
        if(ret == false)
            goto UNINIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("uninit ddr buffer\n");
    ret = m_ddrBuffer.uninit();
    if(ret == false)
        goto UNINIT_BUF_LIST_EXIT;

UNINIT_BUF_LIST_EXIT:
    return ret;
}

};  //namespace NSCcuIf

