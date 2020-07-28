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
#include "ccu_mem_mgr.h"
#include "ccu_ext_interface/af_cxu_param.h"
#include <cutils/properties.h>  // For property_get().

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include "kd_camera_feature.h"
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
static android::Mutex _ccuMgrBootMutex;
static android::Mutex _ccuMgrInstanceMutex;
static int32_t _ccuBootCnt = 0;
static int32_t _ccuInitCnt = 0;
static const uint32_t _apmcuStructSizes[] =
{
    GEN_CCU_STRUCT_SIZE_MAP(GEN_STRUCT_ENTRY_APMCU)
};
static const char *_apmcuStructNames[] =
{
    GEN_CCU_STRUCT_SIZE_MAP(GEN_STRUCT_STRING_APMCU)
};
static const char *_ccuStructNames[] =
{
    GEN_CCU_STRUCT_SIZE_MAP(GEN_STRUCT_STRING_CCU)
};

extern "C"
NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);

bool CcuMgr::IsCcuMgrEnabled;

/*******************************************************************************
* Singleton Factory Function
********************************************************************************/
ICcuMgrExt *ICcuMgrExt::createInstance(const char userName[32])
{
    android::Mutex::Autolock lock(_ccuMgrInstanceMutex);
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
    LOG_INF_MUST("Accuire _ccuMgrInstanceMutex...");
    android::Mutex::Autolock lock(_ccuMgrInstanceMutex);
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

bool ICcuMgrPriv::isCcuBooted()
{
    android::Mutex::Autolock lock(_ccuMgrBootMutex);

    if(_ccuBootCnt > 0)
        return true;
    else
        return false;
}

static void _destroyCcuMgr()
{
    android::Mutex::Autolock lock(_ccuMgrInstanceMutex);
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
* Constructor/Destructor
********************************************************************************/
CcuMgr::CcuMgr()
{

#if defined(CCU_IS_USER_LOAD)
    DBG_LOG_SET_IS_USER_LOAD();
    LOG_INF_MUST("detected user load in CcuMgr constructor");
#endif
    DBG_LOG_CONFIG(ccuif, ccu_drv);
    struct CcuDrvInitData drvInitData;

    LOG_DBG("CcuMgr constructor");
    _checkCcuEnable();

    CcuMemMgr *memMgr = CcuMemMgr::getInstance();
    LOG_DBG("CcuMgr constructor, pmemMgr(%p)", memMgr);
    memMgr->init();
    if(!memMgr->allocateSystemMem())
    {
        m_pDrvCcu->TriggerAee("allocateSystemMem failed");
    }

    _initBufferLists();
    memMgr->dumpAllocationTable();

    m_pDrvCcu = (CcuDrvImp*)CcuDrvImp::createInstance();
    if ( NULL == m_pDrvCcu ) {
        LOG_ERR("createInstance(CCU_A)return %p", m_pDrvCcu);
    }

    unsigned int cpu_buf_align_mva = memMgr->getAlignMva(CCU_BUFTYPE_CPUREF);

    drvInitData = {
        m_logBuffers[0].getMva(), 
        m_logBuffers[1].getMva(), 
        (MINT8 *)m_logBuffers[0].getVa(), 
        (MINT8 *)m_logBuffers[1].getVa(),
        m_ddrBuffer.getMva(), 
        (MINT8 *)m_ddrBuffer.getVa(),
        cpu_buf_align_mva,
        m_backupDataBuffer.getMva()
    };

    if(m_pDrvCcu->init(drvInitData) == MFALSE)
    {
        m_pDrvCcu->TriggerAee("CcuDrv init failed");
    }    

    //CcuDrv should be initialed to provide function of importMemToKernel
    memMgr->importMemoriesToKernel();

    //load CCU DDR binary in cameraserver linking time, no need to repeat load at each camera open/close iteration
    m_pDrvCcu->loadCcuDdrBin();

    //Get instance of ICcuCtrlSystem
    m_pCcuCtrlSys = ICcuCtrlSystem::createInstance();
}

CcuMgr::~CcuMgr()
{
    LOG_DBG("CcuMgr destructor");
    CcuMemMgr *memMgr = CcuMemMgr::getInstance();
    
    memMgr->reset();
    memMgr->dumpAllocationTable();
    memMgr->deallocateSystemMem();
    memMgr->uninit();

    if (!m_pDrvCcu->uninit()) {
        LOG_ERR("m_pDrvCcu->uninit() failed");
    }
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int CcuMgr::ccuInit()
{
    android::Mutex::Autolock lock(_ccuMgrBootMutex);
    //init is move to CcuMgr constructor, do only once at cameraserver linking
    //thus this function no longer to perform any action
    return  0;
}

int CcuMgr::ccuUninit()
{
    android::Mutex::Autolock lock(_ccuMgrBootMutex);
    //uninit is move to CcuMgr destructor, do only once at cameraserver terminated
    //thus this function no longer to perform any action
    return  0;
}

// extern "C"
// NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);

// static void querySensor2TgMapping()
// {
//     LOG_DBG_MUST("[%s]+", __FUNCTION__);

//     NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;
//     uint32_t result;

//     for(int sidx = IMGSENSOR_SENSOR_IDX_MIN_NUM ; sidx < IMGSENSOR_SENSOR_IDX_MAX_NUM ; sidx ++)
//     {
//         mpNormalPipe = createInstance_INormalPipe(sidx, "ccuMgr");
//         mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_INDEX, (MINTPTR)&result, 0, 0);
//         mpNormalPipe->destroyInstance("ccuMgr");
//         LOG_DBG_MUST("sensorIdx(%d)->tgInfo(%d)", sidx, result);
//     }

//     LOG_DBG_MUST("[%s]-", __FUNCTION__);
// }

int CcuMgr::ccuBoot()
{
    android::Mutex::Autolock lock(_ccuMgrBootMutex);

    CcuMemMgr *memMgr = CcuMemMgr::getInstance();
    bool rlt;

    LOG_DBG_MUST("+: bootCnt(%d)\n", _ccuBootCnt);

    LOG_DBG("CcuMgr boot, pmemMgr(%p)", memMgr);
    memMgr->dumpAllocationTable();

    if(_ccuBootCnt == 0)
    {
        if(!CcuMgr::IsCcuMgrEnabled)
        {
            LOG_INF_MUST("CCUMgr is not enabled, exit");
            return 0;
        }

        rlt = m_pDrvCcu->boot();
        if(rlt == false)
        {
            LOG_ERR("m_pDrvCcu->boot(), failed: %x\n", rlt);
            m_pDrvCcu->TriggerAee("ccuBoot failed");
            return -CCU_MGR_ERR_UNKNOWN;
        }

        //check struct size matching
        uint32_t ccuStructSizesAddr = m_pDrvCcu->readInfoReg(23);
        uint32_t *ccuStructSizes = (uint32_t *)m_pDrvCcu->ccuAddrToVa(ccuStructSizesAddr);
        LOG_DBG("ccuStructSizeMapAddr(%x), ccuStructSizeMapPtr(%p)", ccuStructSizesAddr ,ccuStructSizes);

        
        int structCnt = sizeof(_apmcuStructSizes)/sizeof(uint32_t);
        LOG_DBG("CCU struct size matching check, cnt(%d)", structCnt);
        for(int i=0 ; i<structCnt ; i++)
        {
            LOG_DBG("%d: ccu_struct(%s), ccuSize(%d), apmcu_struct(%s), apmcuSize(%d)", 
                i, _ccuStructNames[i], ccuStructSizes[i], _apmcuStructNames[i], _apmcuStructSizes[i]);
            if(ccuStructSizes[i] != _apmcuStructSizes[i])
            {
                LOG_WRN("ccu struct size matching check violation: ccu_struct(%s), ccuSize(%d), apmcu_struct(%s), apmcuSize(%d)", 
                    _ccuStructNames[i], ccuStructSizes[i], _apmcuStructNames[i], _apmcuStructSizes[i]);
            }
        }

        LOG_DBG("init m_pCcuCtrlSys(%p)", m_pCcuCtrlSys);
        m_pCcuCtrlSys->init();
    }
    else
        LOG_DBG_MUST("already booted, skip boot, booCnt(%d)\n", _ccuBootCnt);

    _ccuBootCnt = _ccuBootCnt + 1;

    if(_ccuBootCnt > TG_COUNT_SUPPORTED)
    {
        LOG_ERR("ccuBootCnt exceeds supportedCnt, caller should check flow: bootCnt(%d), support(%c)\n", _ccuBootCnt, TG_COUNT_SUPPORTED);
        m_pDrvCcu->TriggerAee("CCU boot count over supported count");
    }


    LOG_DBG_MUST("-: bootCnt(%d)\n", _ccuBootCnt);

    return 0;
}

int CcuMgr::ccuShutdown()
{
    android::Mutex::Autolock lock(_ccuMgrBootMutex);

    bool rlt;

    LOG_DBG_MUST("+: booCnt(%d)\n", _ccuBootCnt);

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

        LOG_DBG("uninit m_pCcuCtrlSys(%p)", m_pCcuCtrlSys);
        m_pCcuCtrlSys->uninit();
    }
    else
        LOG_DBG_MUST("another user is working or illegal shutdown, booCnt(%d)\n", _ccuBootCnt);
    
    _ccuBootCnt = _ccuBootCnt - 1;

    if(_ccuBootCnt < 0)
    {
        LOG_ERR("ccuBootCnt < 0, caller should check flow: bootCnt(%d)\n", _ccuBootCnt);
        m_pDrvCcu->TriggerAee("CCU boot count less than zero");
    }
    LOG_DBG_MUST("-: booCnt(%d)\n", _ccuBootCnt);

    return 0;
}

int CcuMgr::ccuSetSensorIdx(ESensorDev_T sensorDev, MUINT32 sensorIdx)
{
    LOG_DBG_MUST("[%s]+", __FUNCTION__);

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;
    uint32_t tgInfo;

    mpNormalPipe = createInstance_INormalPipe(sensorIdx, "ccuMgr");
    mpNormalPipe->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_INDEX, (MINTPTR)&tgInfo, 0, 0);
    mpNormalPipe->destroyInstance("ccuMgr");
    LOG_DBG_MUST("sensorIdx(%d)->tgInfo(%d)", sensorIdx, tgInfo);

    //send the mapping to CCU
    struct ccu_tg2sensor_pair tg2sensor_pair;
    tg2sensor_pair.tg_info = tgInfo;
    tg2sensor_pair.sensor_idx = sensorIdx;
    m_pCcuCtrlSys->ccuControl(MSG_TO_CCU_UPDATE_TG_SENSOR_MAP, &tg2sensor_pair, NULL);

    LOG_DBG_MUST("[%s]-", __FUNCTION__);

    return 0;
}

int CcuMgr::ccuInvalidateSensorIdx(ESensorDev_T sensorDev, MUINT32 sensorIdx)
{
    LOG_DBG_MUST("[%s]+", __FUNCTION__);

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpNormalPipe;
    uint32_t tgInfo;


    LOG_DBG_MUST("Invalidate sensorIdx(%d)", sensorIdx);

    //send the mapping to CCU
    m_pCcuCtrlSys->ccuControl(MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP, &sensorIdx, NULL);
    m_pDrvCcu->FreeSensorSlot(sensorIdx);
    LOG_DBG_MUST("[%s]-", __FUNCTION__);

    return 0;
}

int CcuMgr::ccuSuspend(ESensorDev_T sensorDev, MUINT32 sensorIdx)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    enum ccu_tg_info tgInfo = CcuDrvUtil::sensorDevToTgInfo(sensorDev, sensorIdx);

    struct ccu_msg msg = {CCU_FEATURE_SYSCTRL, MSG_TO_CCU_SUSPEND, NULL, NULL, tgInfo, sensorIdx};

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

    enum ccu_tg_info tgInfo = CcuDrvUtil::sensorDevToTgInfo(sensorDev, sensorIdx);

    struct ccu_msg msg = {CCU_FEATURE_SYSCTRL, MSG_TO_CCU_RESUME, NULL, NULL, tgInfo, sensorIdx};

    if(!m_pDrvCcu->sendCmdIpc(&msg))
    {
        LOG_ERR("cmd(%d) fail \n", msg.msg_id);
        return -1;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return 0;
};

struct CcuBufferList *CcuMgr::getCcuBufferList(enum ccu_feature_type ccuFeature, uint32_t sensor_idx)
{
    if(sensor_idx >= IMGSENSOR_SENSOR_IDX_MIN_NUM && sensor_idx < IMGSENSOR_SENSOR_IDX_MAX_NUM)
    {
        switch(ccuFeature)
        {
            case CCU_FEATURE_AE:
                LOG_INF_MUST("get buffer list [%d] of AE", sensor_idx);
                return &m_AeBufferList[sensor_idx];
            case CCU_FEATURE_AF:
                LOG_INF_MUST("get buffer list [%d] of AF", sensor_idx);
                return &m_AfBufferList[sensor_idx];
            case CCU_FEATURE_LTM:
                LOG_INF_MUST("get buffer list [%d] of LTM", sensor_idx);
                return &m_LtmBufferList[sensor_idx];
            case CCU_FEATURE_3ACTRL:
                LOG_INF_MUST("get buffer list of 3ASync");
                return &m_3asyncBufferList;
            case CCU_FEATURE_SYSCTRL:
                LOG_INF_MUST("get buffer list of SysCtrl");
                return &m_sysctrlBufferList;
            default:
                LOG_ERR("feature type invalid: feature type(%d), sensor_idx(%d)", ccuFeature, sensor_idx);
                return NULL;
        }
    }
    else
    {
        LOG_ERR("sensor_idx invalid: feature type(%d), sensor_idx(%d)", ccuFeature, sensor_idx);
        return NULL;
    }
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
}

bool CcuMgr::_initBufferLists()
{
    bool ret;
    
    LOG_DBG_MUST("init DDR buffer\n");
    m_ddrBuffer.size = CCU_CACHE_SIZE;
    ret = m_ddrBuffer.init("CcuBinDdrBuff", CCU_CACHE_SIZE, CCU_BUFTYPE_DDRBIN);

    if(ret == false)
        goto INIT_BUF_LIST_EXIT;
    LOG_INF_MUST("CCU DDR buffer: va(%p), mva(%x), sz(%d)\n", m_ddrBuffer.getVa(), m_ddrBuffer.getMva(), m_ddrBuffer.size);
    
    LOG_DBG_MUST("init AE buffer lists\n");
    for(int i=IMGSENSOR_SENSOR_IDX_MIN_NUM ; i<IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
    {
        LOG_DBG_MUST("init AE buffer lists[%d]\n", i);
        ret = m_AeBufferList[i].init();
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init AF buffer lists\n");
    for(int i=IMGSENSOR_SENSOR_IDX_MIN_NUM ; i<IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
    {
        LOG_DBG_MUST("init AF buffer lists[%d]\n", i);
        ret = m_AfBufferList[i].init();
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init LTM buffer lists\n");
    for(int i=IMGSENSOR_SENSOR_IDX_MIN_NUM ; i<IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
    {
        LOG_DBG_MUST("init LTM buffer lists[%d]\n", i);
        ret = m_LtmBufferList[i].init();
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init 3ASync buffer lists\n");
    ret = m_3asyncBufferList.init();
    if(ret == false)
        goto INIT_BUF_LIST_EXIT;

    LOG_DBG_MUST("init System buffer lists\n");
    ret = m_sysctrlBufferList.init();
    if(ret == false)
        goto INIT_BUF_LIST_EXIT;

    for(int i=0 ; i<CCU_DRAM_LOG_BUF_CNT ; i++)
    {
        LOG_DBG_MUST("init log buffers[%d]\n", i);
        ret = m_logBuffers[i].init("CcuLog", CCU_DRAM_LOG_BUF_SIZE, CCU_BUFTYPE_DDRBIN);
        if(ret == false)
            goto INIT_BUF_LIST_EXIT;
    }

    LOG_DBG_MUST("init BKDATA buffer\n");
    m_backupDataBuffer.size = CCU_BKDATA_BUF_SIZE;
    ret = m_backupDataBuffer.init("CcuBackupDataBuffer", CCU_BKDATA_BUF_SIZE, CCU_BUFTYPE_DDRBIN);
    if(ret == false)
        goto INIT_BUF_LIST_EXIT;
    LOG_INF_MUST("CCU BKDATA buffer: va(%p), mva(%x), sz(%d)\n", m_backupDataBuffer.getVa(), m_backupDataBuffer.getMva(), m_backupDataBuffer.size);

INIT_BUF_LIST_EXIT:
    return ret;
}

};  //namespace NSCcuIf

