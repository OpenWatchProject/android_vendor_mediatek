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
#define LOG_TAG "ispDrvCamsv"

#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>
#include "isp_drv_camsv.h"
#include <isp_drv_stddef.h>



#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv_camsv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_camsv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_camsv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_camsv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_camsv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_camsv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_camsv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

typedef enum{
    IspDrv_UNKNONW      = 0,
    IspDrv_Create       = 1,
    IspDrv_Init         = 2,
    IspDrv_Uninit       = 3,
}E_IspDrvStatus;

typedef enum{
    OP_IspDrv_Destroy,
    OP_IspDrv_sig,         //wait/clr/reg signal
    OP_IspDrv_sig_sig,     //signal signal
    OP_IspDrv_Reg,         //register read/write operation
    OP_IspDrv_Device,      //device driver ctrl
    OP_IspDrv_init,
    OP_IspDrv_Uninit,
}E_IspDrvOP;


IspDrvCamsv::IspDrvCamsv()
{
    DBG_LOG_CONFIG(drv, isp_drv_camsv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_UserCnt = 0;
    m_pIspDrvImp = NULL;
    m_pIspHwRegAddr = NULL;
    m_FSM = IspDrv_UNKNONW;
    m_hwModule = CAMSV_MAX;
}

static IspDrvCamsv gCamsvDrvObj[CAMSV_MAX - CAMSV_START];

IspDrv* IspDrvCamsv::createInstance(ISP_HW_MODULE hwModule)
{
    MUINT32 moduleIdx;

    if(hwModule < CAMSV_MAX && hwModule >= CAMSV_START){
        moduleIdx = hwModule - CAMSV_START;
        LOG_DBG("moduleIdx(%d),hwModule(%d)",moduleIdx,hwModule);
    } else {
        LOG_ERR("[Error]hwModule(%d) out of the range",hwModule);
        return NULL;
    }

    gCamsvDrvObj[moduleIdx].m_pIspDrvImp = (IspDrvImp*)IspDrvImp::createInstance(hwModule);
    gCamsvDrvObj[moduleIdx].m_hwModule = hwModule;

    gCamsvDrvObj[moduleIdx].m_FSM = IspDrv_Create;
    //
    return (IspDrv*)&gCamsvDrvObj[moduleIdx];
}


MBOOL IspDrvCamsv::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    if(this->FSM_CHK(OP_IspDrv_init) == MFALSE)
        return MFALSE;
    //
    LOG_INF("+,m_UserCnt(%d),curUser(%s).", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }
    //
    if(this->m_UserCnt > 0)
    {
        tmp = android_atomic_inc(&this->m_UserCnt);
        LOG_INF("-,m_UserCnt(%d)", this->m_UserCnt);
        return Result;
    }

    if(this->m_pIspDrvImp->init("isp_drv_camsv") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }

    //check wheather kernel is existed or not
    if (this->m_Fd < 0) {    // 1st time open failed.
        LOG_ERR("ISP kernel is not existed\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    tmp = android_atomic_inc(&this->m_UserCnt);

    this->m_FSM = IspDrv_Init;

EXIT:

    LOG_DBG("-,ret(%d),mInitCount(%d)", Result, this->m_UserCnt);
    return Result;
}


//-----------------------------------------------------------------------------
MBOOL IspDrvCamsv::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MINT32 tmp=0;
    //
    android::Mutex::Autolock lock(this->IspRegMutex);

    if(this->FSM_CHK(OP_IspDrv_Uninit) == MFALSE)
        return MFALSE;
    //
    LOG_INF("+,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);
    //
    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to uninit isp driver\n");
        return MFALSE;
    }

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in isp_drv_camsv , curUser(%s)",userName);
        goto EXIT;
    }
    // More than one user
    tmp = android_atomic_dec(&this->m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;
    //
    if(this->m_pIspDrvImp->uninit("isp_drv_camsv") == MFALSE){
        Result = MFALSE;
        goto EXIT;
    }

    this->m_FSM = IspDrv_Uninit;
EXIT:

    LOG_DBG("-,ret(%d),m_UserCnt(%d)", Result, this->m_UserCnt);
    return Result;
}


void IspDrvCamsv::destroyInstance(void)
{
    if(this->FSM_CHK(OP_IspDrv_Destroy) == MFALSE){
        LOG_ERR("FSM_CHK ERROR!!!");
    }

    this->m_pIspDrvImp = NULL;
}

MBOOL IspDrvCamsv::waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig) == MFALSE) {
        LOG_ERR("FSM_CHK ERROR!!!");
        return MFALSE;
    }
    return this->m_pIspDrvImp->waitIrq(pWaitIrq);
}

MBOOL IspDrvCamsv::clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->clearIrq(pClearIrq);
}

MBOOL IspDrvCamsv::registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq)
{
    if(this->FSM_CHK(OP_IspDrv_sig) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->registerIrq(pRegIrq);
}

MBOOL IspDrvCamsv::readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    if(this->FSM_CHK(OP_IspDrv_Reg) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->readRegs(pRegIo, Count, caller);
}

MUINT32 IspDrvCamsv::readReg(MUINT32 Addr,MINT32 caller)
{
    if(this->FSM_CHK(OP_IspDrv_Reg) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->readReg(Addr, caller);
}

MBOOL IspDrvCamsv::writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    if(this->FSM_CHK(OP_IspDrv_Reg) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->writeRegs(pRegIo, Count, caller);
}

MBOOL IspDrvCamsv::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    if(this->FSM_CHK(OP_IspDrv_Reg) == MFALSE)
        return MFALSE;

    return this->m_pIspDrvImp->writeReg(Addr, Data, caller);
}

MBOOL IspDrvCamsv::signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq)
{
    LOG_INF("IRQ SIGNAL:hwModule:0x%x, userKey:0x%x, status:0x%x",this->m_hwModule,pWaitIrq->UserKey,pWaitIrq->Status);
    ISP_WAIT_IRQ_STRUCT wait;

    if(this->FSM_CHK(OP_IspDrv_sig_sig) == MFALSE)
        return MFALSE;

    memcpy(&wait.EventInfo,pWaitIrq,sizeof(ISP_WAIT_IRQ_ST));
    switch(this->m_hwModule){
        case CAMSV_0:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
            break;
        case CAMSV_1:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
            break;
        case CAMSV_2:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
            break;
        case CAMSV_3:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
            break;
        case CAMSV_4:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
            break;
        case CAMSV_5:
            wait.Type = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
            break;
        default:
            LOG_ERR("unsupported hw hwModule:0x%x\n",this->m_hwModule);
            return MFALSE;
            break;
    }

    if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_FLUSH_IRQ_REQUEST,&wait) < 0){
        LOG_ERR("signal IRQ fail(irq:0x%x,status:0x%x)",wait.Type,wait.EventInfo.Status);
        return MFALSE;
    }
    return MTRUE;
}


MBOOL IspDrvCamsv::getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{
    MBOOL rst = MTRUE;

    switch(eCmd){
        case _GET_SOF_CNT:
            switch(this->m_hwModule){
                case CAMSV_0:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                    break;
                case CAMSV_1:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                    break;
                case CAMSV_2:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                    break;
                case CAMSV_3:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                    break;
                case CAMSV_4:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                    break;
                case CAMSV_5:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_hwModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_CUR_SOF,(unsigned char*)pData) < 0){
                LOG_ERR("dump sof fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_DMA_ERR:
            switch(this->m_hwModule){
                case CAMSV_0:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                    break;
                case CAMSV_1:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                    break;
                case CAMSV_2:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                    break;
                case CAMSV_3:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                    break;
                case CAMSV_4:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                    break;
                case CAMSV_5:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_hwModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_DMA_ERR,(unsigned char*)pData) < 0){
                LOG_ERR("dump dma_err fail");
                rst = MFALSE;
            }
            break;
        case _GET_INT_ERR:
            {
                struct ISP_RAW_INT_STATUS IntStatus[ISP_IRQ_TYPE_AMOUNT];
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_INT_ERR,(struct ISP_RAW_INT_STATUS*)IntStatus) < 0){
                    LOG_ERR("dump int_err fail\n");
                    rst = MFALSE;
                }
                else{
                    switch(this->m_hwModule){
                        case CAMSV_0:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_0_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAMSV_1:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_1_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAMSV_2:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_2_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAMSV_3:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_3_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAMSV_4:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_4_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        case CAMSV_5:
                            memcpy(pData, &IntStatus[ISP_IRQ_TYPE_INT_CAMSV_5_ST], sizeof(struct ISP_RAW_INT_STATUS));
                            break;
                        default:
                            LOG_ERR("unsuported module:0x%x\n",this->m_hwModule);
                            break;
                    }
                }
            }
            break;
        case _GET_DROP_FRAME_STATUS:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_DROP_FRAME,(unsigned char*)pData) < 0){
                LOG_ERR("dump drop frame status fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_START_TIME:
            switch(this->m_hwModule){
                case CAMSV_0:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                    break;
                case CAMSV_1:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                    break;
                case CAMSV_2:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                    break;
                case CAMSV_3:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                    break;
                case CAMSV_4:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                    break;
                case CAMSV_5:
                    *(MUINT32*)pData = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_hwModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_START_TIME,(unsigned char*)pData) < 0){
                LOG_ERR("get start time fail\n");
                rst = MFALSE;
            }
            break;
        case _GET_GLOBAL_TIME:
            {
                MUINT64 time[_e_TS_max] = {0,};
                time[_e_mono_] = ((MUINT64*)pData)[_e_mono_];
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_GET_GLOBAL_TIME,(unsigned long long*)time) < 0){
                    LOG_ERR("_GET_GLOBAL_TIME fail\n");
                    rst = MFALSE;
                }
                memcpy(pData, (void*)time, sizeof(MUINT64)*_e_TS_max);
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x\n",eCmd);
            return MFALSE;
        break;
    }
    return rst;

}

MBOOL IspDrvCamsv::setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData)
{

    switch(eCmd){
        case _SET_DBG_INT:
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_DEBUG_FLAG,(unsigned char*)pData) < 0){
                LOG_ERR("kernel log enable error\n");
                return MFALSE;
            }
            break;
        case _SET_VF_OFF:
            {
                MUINT32 dbg[2];
                switch(this->m_hwModule){
                    case CAMSV_0: dbg[1] = ISP_CAMSV0_IDX;
                        break;
                    case CAMSV_1: dbg[1] = ISP_CAMSV1_IDX;
                        break;
                    case CAMSV_2: dbg[1] = ISP_CAMSV2_IDX;
                        break;
                    case CAMSV_3: dbg[1] = ISP_CAMSV3_IDX;
                        break;
                    case CAMSV_4: dbg[1] = ISP_CAMSV4_IDX;
                        break;
                    case CAMSV_5: dbg[1] = ISP_CAMSV5_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_hwModule);
                        return MFALSE;
                }
                dbg[0] = 10;
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_VF_LOG,(unsigned int*)dbg) < 0){
                    LOG_ERR("_SET_VF_OFF error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_VF_ON:
            {
                MUINT32 dbg[2];
                switch(this->m_hwModule){
                    case CAMSV_0: dbg[1] = ISP_CAMSV0_IDX;
                        break;
                    case CAMSV_1: dbg[1] = ISP_CAMSV1_IDX;
                        break;
                    case CAMSV_2: dbg[1] = ISP_CAMSV2_IDX;
                        break;
                    case CAMSV_3: dbg[1] = ISP_CAMSV3_IDX;
                        break;
                    case CAMSV_4: dbg[1] = ISP_CAMSV4_IDX;
                        break;
                    case CAMSV_5: dbg[1] = ISP_CAMSV5_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_hwModule);
                        return MFALSE;
                }
                dbg[0] = 11;
                if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_VF_LOG,(unsigned char*)dbg) < 0){
                    LOG_ERR("_SET_VF_ON error\n");
                    return MFALSE;
                }
            }
            break;
        case _SET_BUF_CTRL:
            switch(this->m_hwModule){
                case CAMSV_0:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_0_ST;
                    break;
                case CAMSV_1:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_1_ST;
                    break;
                case CAMSV_2:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_2_ST;
                    break;
                case CAMSV_3:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_3_ST;
                    break;
                case CAMSV_4:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_4_ST;
                    break;
                case CAMSV_5:
                    ((ISP_BUFFER_CTRL_STRUCT*)pData)->module = ISP_IRQ_TYPE_INT_CAMSV_5_ST;
                    break;
                default:
                    LOG_ERR("unsupported module:0x%x\n",this->m_hwModule);
                    return MFALSE;
                    break;
            }
            if(ioctl(this->m_pIspDrvImp->m_Fd,ISP_BUFFER_CTRL,(ISP_BUFFER_CTRL_STRUCT*)pData) < 0){
                LOG_ERR("_set_buf_ctrl error,ctrl:0x%x\n",((ISP_BUFFER_CTRL_STRUCT*)pData)->ctrl);
                return MFALSE;
            }
            break;
        case _SET_RESET_HW_MOD:
            {
                MUINT32 resetModule = ISP_CAMSV0_IDX;

                switch(this->m_hwModule){
                    case CAMSV_0:
                        resetModule = ISP_CAMSV0_IDX;
                        break;
                    case CAMSV_1:
                        resetModule = ISP_CAMSV1_IDX;
                        break;
                    case CAMSV_2:
                        resetModule = ISP_CAMSV2_IDX;
                        break;
                    case CAMSV_3:
                        resetModule = ISP_CAMSV3_IDX;
                        break;
                    case CAMSV_4:
                        resetModule = ISP_CAMSV4_IDX;
                        break;
                    case CAMSV_5:
                        resetModule = ISP_CAMSV5_IDX;
                        break;
                    default:
                        LOG_ERR("unsupported module:0x%x\n", this->m_hwModule);
                        return MFALSE;
                }

                LOG_INF("Reset hw module: %d, corresponding resetModule: %d ......\n", this->m_hwModule, resetModule);
                if (ioctl(this->m_pIspDrvImp->m_Fd, ISP_RESET_BY_HWMODULE, &resetModule) < 0) {
                    LOG_ERR("Error: Fail reset hw module: %d, corresponding resetModule: %d\n", this->m_hwModule, resetModule);
                    return MFALSE;
                }

            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x",eCmd);
            return MFALSE;
        break;
    }
    return MTRUE;
}

MBOOL IspDrvCamsv::FSM_CHK(MUINT32 op)
{
    switch(op){
        case OP_IspDrv_init:
            switch(this->m_FSM){
                case IspDrv_Create:
                    break;
                default:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
            }
            break;
        case OP_IspDrv_Uninit:
            switch(this->m_FSM){
                case IspDrv_Init:
                    return MTRUE;
                    break;
                default:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
            }
            break;
        case OP_IspDrv_Destroy:
            switch(this->m_FSM){
                case IspDrv_Uninit:
                    break;
                default:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
            }
            break;
        case OP_IspDrv_sig:
            switch(this->m_FSM){
                case IspDrv_UNKNONW:
                case IspDrv_Create:
                case IspDrv_Uninit:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case OP_IspDrv_sig_sig:
            switch(this->m_FSM){
                case IspDrv_Create:
                case IspDrv_Uninit:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case OP_IspDrv_Reg:
        case OP_IspDrv_Device:
            switch(this->m_FSM){
                case IspDrv_Init:
                    break;
                default:
                    LOG_ERR("FSM error: op:0x%x, cur status:0x%x",op,this->m_FSM);
                    return MFALSE;
                    break;
            }
            break;
        default:
            LOG_ERR("unsupport Operation:0x%x",op);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MBOOL IspDrvCamsv::DumpReg(MBOOL bPhy)
{
    char _tmpchr[16] = "\0";
    char _chr[256] = "\0";
    MUINT32 shift=0x0;

    switch(this->m_hwModule){
        case CAMSV_0:
            shift = 0x0000;
            break;
        case CAMSV_1:
            shift = 0x1000;
            break;
        case CAMSV_2:
            shift = 0x2000;
            break;
        case CAMSV_3:
            shift = 0x2000;
            break;
        case CAMSV_4:
            shift = 0x3000;
            break;
        case CAMSV_5:
            shift = 0x4000;
            break;
        default:
            break;
    }

    if(bPhy){
        LOG_ERR("###################\n");
        LOG_ERR("start dump camsv phy reg\n");
        for(MUINT32 i=0x0;i<0x600;i+=0x20){
            _chr[0] = '\0';
            snprintf(_chr, sizeof(_chr), "0x%x: ",shift + i);
            for(MUINT32 j=i;j<(i+0x20);j+=0x4){
                if(j >= CAM_BASE_RANGE)
                    break;
                _tmpchr[0] = '\0';
                snprintf(_tmpchr, sizeof(_tmpchr), "0x%8x - ",this->m_pIspDrvImp->readReg(j));
                strncat(_chr,_tmpchr, strlen(_tmpchr));
            }
            LOG_WRN("%s\n",_chr);
        }
        LOG_ERR("###################\n");
    }
    else{
        LOG_ERR("camsv no vir part!!!");
    }
    return MTRUE;
}


