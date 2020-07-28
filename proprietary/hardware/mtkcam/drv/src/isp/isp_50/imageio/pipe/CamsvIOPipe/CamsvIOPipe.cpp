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
#define LOG_TAG "camsviopipe"

//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "CamsvIOPipe.h"
//
#include <cutils/properties.h>  // For property_get().


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

EXTERN_DBG_LOG_VARIABLE(pipe);

#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_ERR

#if 1
#define PIPE_DBG(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_INF(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_ERR(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)
#else
#define PIPE_DBG(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#define PIPE_INF(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#define PIPE_ERR(fmt, arg...)        do { { printf("[%s %d]:" fmt "\n",__FUNCTION__, __LINE__, ##arg);  } } while(0)
#endif

//#define CAMSV_SUBSAMPLE


/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/


//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
CamsvIOPipe::CamsvIOPipe()
{
    //
    DBG_LOG_CONFIG(imageio, pipe);


    m_szUsrName[0] = '\0';


    m_FSM = op_unknown;
    m_pipe_opt = ICamIOPipe::CAMSVIO;
    m_ResMgr.m_occupied = MFALSE;
    m_hwModule = CAMSV_0;
    m_pIspDrvCamsv = NULL;
    m_occupied = MFALSE;
    PIPE_INF(":X");
}

CamsvIOPipe::~CamsvIOPipe()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::FSM_CHECK(MUINT32 op)
{
    MBOOL ret = MTRUE;
    this->m_FSMLock.lock();
    switch(op){
        case op_unknown:
            if(this->m_FSM != op_uninit)
                ret = MFALSE;
            break;
        case op_init:
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
            break;
        case op_cfg:
            if(this->m_FSM != op_init)
                ret = MFALSE;
            break;
        case op_start:
            if(this->m_FSM != op_cfg)
                ret = MFALSE;
            break;
        case op_stop:
            if(this->m_FSM != op_start)
                ret = MFALSE;
            break;
        case op_uninit:
            switch(this->m_FSM){
                case op_init:
                case op_cfg:
                case op_stop:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_cmd:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                case op_stop:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        case op_endeq:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE)
        PIPE_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);
    this->m_FSMLock.unlock();
    return ret;
}

MBOOL CamsvIOPipe::FSM_UPDATE(MUINT32 op)
{
    PIPE_DBG("CamsvIOPipe::FSM_UPDATE [%d,%d]", op, this->m_FSM);
    if(op == op_cmd) {
        return MTRUE;
    }
    this->m_FSMLock.lock();
    this->m_FSM = (E_FSM)op;
    this->m_FSMLock.unlock();
    this->m_ResMgr.m_occupied = MFALSE;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
static CamsvIOPipe gCamsvioPipe[CAMSV_MAX - CAMSV_START]; // CAMSV_0 ~ CAMSV_5

CamsvIOPipe* CamsvIOPipe::Create(MINT8 const szUsrName[32], const E_INPUT& InPut)
{
    CamsvIOPipe* ptr = NULL;
    BASE_LOG_INF(":E:user:%s, with input source:0x%x\n",szUsrName,InPut);

    switch(InPut){
        // CAMSV
        case TG_CAMSV_0:
            if(gCamsvioPipe[CAMSV_0-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_0 is occupied by user:%s\n",gCamsvioPipe[CAMSV_0-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_0-CAMSV_START].m_hwModule = CAMSV_0;
                gCamsvioPipe[CAMSV_0-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_0-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_0-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_0-CAMSV_START];

                gCamsvioPipe[CAMSV_0-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
                gCamsvioPipe[CAMSV_0-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_0");
            }
            break;
        case TG_CAMSV_1:
            if(gCamsvioPipe[CAMSV_1-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_1 is occupied by user:%s\n",gCamsvioPipe[CAMSV_1-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_1-CAMSV_START].m_hwModule = CAMSV_1;
                gCamsvioPipe[CAMSV_1-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_1-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_1-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_1-CAMSV_START];

                gCamsvioPipe[CAMSV_1-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
                gCamsvioPipe[CAMSV_1-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_1");
            }
            break;
        case TG_CAMSV_2:
            if(gCamsvioPipe[CAMSV_2-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_2 is occupied by user:%s\n",gCamsvioPipe[CAMSV_2-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_2-CAMSV_START].m_hwModule = CAMSV_2;
                gCamsvioPipe[CAMSV_2-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_2-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_2-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_2-CAMSV_START];

                gCamsvioPipe[CAMSV_2-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_2);
                gCamsvioPipe[CAMSV_2-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_2");
            }
            break;
        case TG_CAMSV_3:
            if(gCamsvioPipe[CAMSV_3-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_3 is occupied by user:%s\n",gCamsvioPipe[CAMSV_3-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_3-CAMSV_START].m_hwModule = CAMSV_3;
                gCamsvioPipe[CAMSV_3-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_3-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_3-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_3-CAMSV_START];

                gCamsvioPipe[CAMSV_3-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_3);
                gCamsvioPipe[CAMSV_3-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_3");
            }
            break;
        case TG_CAMSV_4:
            if(gCamsvioPipe[CAMSV_4-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_4 is occupied by user:%s\n",gCamsvioPipe[CAMSV_4-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_4-CAMSV_START].m_hwModule = CAMSV_4;
                gCamsvioPipe[CAMSV_4-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_4-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_4-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_4-CAMSV_START];

                gCamsvioPipe[CAMSV_4-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_4);
                gCamsvioPipe[CAMSV_4-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_4");
            }
            break;
        case TG_CAMSV_5:
            if(gCamsvioPipe[CAMSV_5-CAMSV_START].m_ResMgr.m_occupied == MTRUE){
                BASE_LOG_ERR("CAMSV_5 is occupied by user:%s\n",gCamsvioPipe[CAMSV_5-CAMSV_START].m_szUsrName);
            }
            else{
                gCamsvioPipe[CAMSV_5-CAMSV_START].m_hwModule = CAMSV_5;
                gCamsvioPipe[CAMSV_5-CAMSV_START].m_ResMgr.m_occupied = MTRUE;
                std::strncpy((char*)gCamsvioPipe[CAMSV_5-CAMSV_START].m_szUsrName,(char const*)szUsrName,
                             sizeof(gCamsvioPipe[CAMSV_5-CAMSV_START].m_szUsrName)-1);

                ptr = &gCamsvioPipe[CAMSV_5-CAMSV_START];

                gCamsvioPipe[CAMSV_5-CAMSV_START].m_pIspDrvCamsv = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_5);
                gCamsvioPipe[CAMSV_5-CAMSV_START].m_pIspDrvCamsv->init("CamsvIOPipe CamSV_5");
            }
            break;
        default:
            BASE_LOG_ERR("unsupported input source(%d)\n",InPut);
            break;
    }

    return ptr;
}

void CamsvIOPipe::Destroy(void)
{
    PIPE_INF(":E:user:%s, with input source:0x%x\n",this->m_szUsrName,this->m_hwModule);

    this->m_ResMgr.m_occupied = MFALSE;
    this->m_szUsrName[0] = '\0';
    this->m_FSMLock.lock();
    this->m_FSM = op_unknown;
    this->m_FSMLock.unlock();
    this->m_pIspDrvCamsv->uninit("CamsvIOPipe");
    this->m_pIspDrvCamsv->destroyInstance();
    this->m_pIspDrvCamsv = NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::init()
{
    PIPE_INF("(%s):E",this->m_szUsrName);
    if(this->m_szUsrName[0] == '0'){
        PIPE_ERR("can't null username\n");
        return MFALSE;
    }
    if(this->FSM_CHECK(op_init) == MFALSE)
        return MFALSE;
    this->FSM_UPDATE(op_init);

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::uninit()
{
    PIPE_INF("CamsvIOPipe::uninit");

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;

    this->FSM_UPDATE(op_uninit);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::start()
{
    void* ptr = NULL;
    PIPE_INF("+");


    if(this->FSM_CHECK(op_start) == MFALSE)
        return MFALSE;

    // Start top
    this->m_TopCtrl.enable(NULL);


    this->FSM_UPDATE(op_start);
    PIPE_INF("-");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::stop(MBOOL bForce)
{
    PIPE_INF("CamsvIOPipe::stop+");

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;

#if 0 /* Jessy - original */
    this->m_TGCtrl.disable();

    this->m_FbcImgo.disable();

    this->m_DmaImgo.disable();

    this->m_TopCtrl.disable();
#else
    this->m_TopCtrl.disable(&bForce);

    this->m_FbcImgo.disable();

    this->m_DmaImgo.disable();

    this->m_TGCtrl.disable();
#endif

    this->FSM_UPDATE(op_stop);

    PIPE_INF("CamsvIOPipe::stop-");

    return  MTRUE;
}

MBOOL CamsvIOPipe::suspend(E_SUSPEND_MODE suspendMode)
{
    suspendMode;
    return this->m_FbcImgo.suspend();
}

MBOOL CamsvIOPipe::resume(E_SUSPEND_MODE suspendMode)
{
    suspendMode;
    //
    return this->m_FbcImgo.resume();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::abortDma(PortID const port)
{
    MUINT32         dmaChannel = 0;
    ISP_WAIT_IRQ_ST irq;

    //
    if(this->m_FSM != op_stop){
        PIPE_ERR("CamsvIOPipe FSM(%d)", this->m_FSM);
        return MFALSE;
    }

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.Timeout = 1000;

    dmaChannel = (MUINT32)this->PortID_MAP(port.index);
    switch(dmaChannel){
        case _camsv_imgo_:
            irq.St_type = SIGNAL_INT;
            irq.Status = SV_SW_PASS1_DON_ST;
            break;
        default:
            PIPE_ERR("Not support dma(0x%x)\n", port.index);
            return MFALSE;

    }

    if( this->m_pIspDrvCamsv->signalIrq(&irq) == MFALSE )
    {
        PIPE_ERR("flush DMA error!");
        return  MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
_isp_dma_enum_ CamsvIOPipe::PortID_MAP(MUINT32 PortID)
{
    switch(PortID){
        case EPortIndex_CAMSV_IMGO:
                return _camsv_imgo_;
            break;
        default:
            PIPE_ERR("un-supported portID:0x%x\n",PortID);
            break;
    }

    return _cam_max_;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MBOOL bImdMode)
{
    MBOOL ret = MTRUE;


    MUINT32         dmaChannel = 0;
    ST_BUF_INFO     ImageAndHeaderInfo;
    MUINT32 targetIdx = 0;
    //warning free
    (void)bImdMode;

    PIPE_DBG("CamsvIOPipe::enqueOutBuf+");

    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_ERR("op_cmd(%d)", op_cmd);
        return MFALSE;
    }

    #ifdef CAMSV_SUBSAMPLE
    if(rQBufInfo.vBufInfo.size() != (this->m_TopCtrl.SubSample+1))
    #else
    if(rQBufInfo.vBufInfo.size() != 1)
    #endif
    {
        PIPE_ERR("camsviopipe support only enque-1-frame-at-1-time %zu\n", rQBufInfo.vBufInfo.size());
        return MFALSE;
    }

    for(MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++){
        stISP_BUF_INFO  bufInfo;

        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_ERR("replace function are not supported in CamsvIOPipe\n");
            return MFALSE;
        }
        ImageAndHeaderInfo.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[ePlane_1st];
        ImageAndHeaderInfo.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[ePlane_1st];

        //header
        ImageAndHeaderInfo.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st];
        ImageAndHeaderInfo.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st];
        ImageAndHeaderInfo.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st];
        ImageAndHeaderInfo.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID[ePlane_1st];
        ImageAndHeaderInfo.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu[ePlane_1st];
        ImageAndHeaderInfo.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe[ePlane_1st];
        bufInfo.u_op.enque.push_back(ImageAndHeaderInfo);

        if(!bufInfo.u_op.enque.empty()){
            PIPE_DBG("burstidx:%d: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)",\
                i,\
                portID.index,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.memID,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.va_addr,\
                bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr,\
                bufInfo.u_op.enque.at(ePlane_1st).header.va_addr);
        }else{
            PIPE_ERR("CamsvIOPipe bufInfo.u_op.enque empty\n");
        }

        if(MFALSE == this->m_FbcImgo.enqueueHwBuf(bufInfo)){
            PIPE_ERR("enque fail at burst:0x%x\n",i);
            ret = MFALSE;
        }

    }

    this->FSM_UPDATE(op_cmd);

    PIPE_DBG("CamsvIOPipe::enqueOutBuf-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
E_BUF_STATUS CamsvIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQTBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    E_BUF_STATUS ret = eBuf_Pass;
    MUINT32 dmaChannel = 0;
    BufInfo buf;
    CAMSV_BUF_CTRL::E_BC_STATUS state;
    int i = 0;
    (void)pNotify;
    (void)u4TimeoutMs;
    //warning free

    PIPE_DBG("CamsvIOPipe::dequeOutBuf+");

    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_ERR("op_cmd(%d)", op_cmd);
        return eBuf_Fail;
    }

    //clear remained data in container
    rQTBufInfo.vBufInfo.clear();

    #ifdef CAMSV_SUBSAMPLE
    for (MUINT32 i = 0; i <= this->m_TopCtrl.SubSample; i++)
    #endif
    {
        //check if there is already filled buffer
        //wait P1_done
        state = this->m_FbcImgo.waitBufReady();
        switch(state){
            case CAMSV_BUF_CTRL::eCmd_Fail:
                return eBuf_Fail;
                break;
            case CAMSV_BUF_CTRL::eCmd_Suspending_Pass:
                return eBuf_suspending;
                break;
            case CAMSV_BUF_CTRL::eCmd_Pass:
                break;
            default:
                PIPE_ERR("unsupported state:%d\n",state);
                return eBuf_Fail;
                break;
        }

        switch(this->m_FbcImgo.dequeueHwBuf(buf)){
            case CAMSV_BUF_CTRL::eCmd_Fail:
                PIPE_ERR("data is rdy on dram,but deque fail\n");
                ret = eBuf_Fail;
                break;
            case CAMSV_BUF_CTRL::eCmd_Suspending_Pass:
                ret = eBuf_suspending;
                break;
            case CAMSV_BUF_CTRL::eCmd_Pass:
                ret = eBuf_Pass;
                break;
            default:
                PIPE_ERR("unsupported state:%d\n",state);
                return eBuf_Fail;
                break;
        }

        rQTBufInfo.vBufInfo.push_back(buf);

        PIPE_DBG("PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)",\
            portID.index, rQTBufInfo.vBufInfo[i].memID[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].u4BufVA[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].u4BufPA[ePlane_1st],\
            rQTBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]);
    }

    #ifdef CAMSV_SUBSAMPLE
    if (rQTBufInfo.vBufInfo.size() != (this->m_TopCtrl.SubSample+1))
    #else
    if(rQTBufInfo.vBufInfo.size() != 1)
    #endif
    {
        PIPE_ERR("dequeout data length(%zu) is mismatch", rQTBufInfo.vBufInfo.size());
        ret = eBuf_Fail;
    }

    this->FSM_UPDATE(op_cmd);

    PIPE_DBG("CamsvIOPipe::dequeOutBuf-");

    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc)
{
    MBOOL   bPak = MTRUE;

    MUINT32 imgo_fmt = 0;

    MINT32 idx_in_src = -1;
    MINT32 InSrcFmt = -1;
    MINT32 InSrcTGSw = -1;

    MINT32 idx_imgo = -1;
    MINT32 pix_id_tg = -1;
    MINT32 pixel_byte_imgo = -1;

    //ENUM_CAM_CTL_EN
    MUINT32 en_p1 = 0;

    PIPE_INF("CamsvIOPipe::configPipe+");
    //
    if(this->FSM_CHECK(op_cfg) == MFALSE) {
        return MFALSE;
    }

    // OutPort is only IMGO
    if ( (vOutPorts.size() != 1) || (vInPorts.size() != 1)) {
        PIPE_ERR("inport:0x%zx || outport:x%zx size err\n", vInPorts.size(), vOutPorts.size());
        return MFALSE;
    }

    //input source
    //inport support only 1 at current driver design
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) {
            PIPE_INF("dummy input vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vInPorts_%d:fmt(0x%x),PM(%d),w/h(%d_%d),crop(%d_%d_%d_%d),tg_idx(%d),dir(%d),fps(%d),timeclk(0x%x)\n",\
                i, \
                vInPorts[i]->eImgFmt, \
                vInPorts[i]->ePxlMode,  \
                vInPorts[i]->u4ImgWidth, vInPorts[i]->u4ImgHeight,\
                vInPorts[i]->crop1.x,\
                vInPorts[i]->crop1.y,\
                vInPorts[i]->crop1.w,\
                vInPorts[i]->crop1.h,\
                vInPorts[i]->index, \
                vInPorts[i]->inout,\
                vInPorts[i]->tgFps,\
                vInPorts[i]->tTimeClk);

        //
        switch ( vInPorts[i]->index ) {
            case EPortIndex_CAMSV_0_TGI:
                PIPE_INF("EPortIndex_CAMSV_0_TGI");
                if(this->m_hwModule != CAMSV_0){
                    PIPE_ERR("TG0 must be with CAMSV_0\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG0 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG0 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_1_TGI:
                PIPE_INF("EPortIndex_CAMSV_1_TGI");
                if(this->m_hwModule != CAMSV_1){
                    PIPE_ERR("TG1 must be with CAMSV_1\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG1 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG1 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_2_TGI:
                PIPE_INF("EPortIndex_CAMSV_2_TGI");
                if(this->m_hwModule != CAMSV_2){
                    PIPE_ERR("TG2 must be with CAMSV_2\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG2 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG2 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_3_TGI:
                PIPE_INF("EPortIndex_CAMSV_3_TGI");
                if(this->m_hwModule != CAMSV_3){
                    PIPE_ERR("TG3 must be with CAMSV_3\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG3 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG3 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_4_TGI:
                PIPE_INF("EPortIndex_CAMSV_4_TGI");
                if(this->m_hwModule != CAMSV_4){
                    PIPE_ERR("TG4 must be with CAMSV_4\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG4 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG4 input SW error\n");
                    return MFALSE;
                }

                break;
            case EPortIndex_CAMSV_5_TGI:
                PIPE_INF("EPortIndex_CAMSV_5_TGI");
                if(this->m_hwModule != CAMSV_5){
                    PIPE_ERR("TG5 must be with CAMSV_5\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG5 input fmt error\n");
                    return MFALSE;
                }

                if((InSrcTGSw= this->SrcFmtToHwVal_TG_SW(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG5 input SW error\n");
                    return MFALSE;
                }

                break;
            default:
                PIPE_ERR("Not supported input source:0x%x !!!!!!!!!!!!!!!!!!!!!!",vInPorts[i]->index);
                return MFALSE;
                break;
        }

        idx_in_src = i;
    }

    //output port
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {

        if ( 0 == vOutPorts[i] ) {
            PIPE_INF("dummy output vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vOutPorts:[%d]:fmt(0x%x),PureRaw(%d),w(%d),h(%d),stirde(%d),dmao(%d),dir(%d)", \
                 i, vOutPorts[i]->eImgFmt, vOutPorts[i]->u4PureRaw, \
                 vOutPorts[i]->u4ImgWidth, vOutPorts[i]->u4ImgHeight,   \
                 vOutPorts[i]->u4Stride[ePlane_1st], \
                 vOutPorts[i]->index, vOutPorts[i]->inout);

        switch(vOutPorts[i]->index){
            case EPortIndex_CAMSV_IMGO:
                PIPE_INF("EPortIndex_CAMSV_IMGO");
                idx_imgo = i;
                this->getOutPxlByteNFmt(vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&imgo_fmt );
                break;
            default:
                PIPE_INF("OutPorts index(%d) err\n", vOutPorts[i]->index);
                //break;
                return MFALSE;
        }
    }

    // Top ctrl setting
    this->m_TopCtrl.m_pDrv = this->m_pIspDrvCamsv;

    //subsample
    this->m_TopCtrl.SubSample = pInFunc->Bits.SUBSAMPLE;

    // fmt sel
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Raw = 0x00;
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Bits.TG1_FMT = InSrcFmt;
    this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Bits.TG1_SW = InSrcTGSw;
    this->m_TopCtrl.m_PixMode = this->m_TGCtrl.m_PixMode = this->m_DmaImgo.m_PixMode = vInPorts[idx_in_src]->ePxlMode;

    /// TG setting
    this->m_TGCtrl.m_pDrv = this->m_pIspDrvCamsv;
    this->m_TGCtrl.m_SubSample = pInFunc->Bits.SUBSAMPLE;
    this->m_TGCtrl.m_continuous = MTRUE; //TG input support only continuous mode

    this->m_TGCtrl.m_Crop.x = vInPorts[idx_in_src]->crop1.x;
    this->m_TGCtrl.m_Crop.y = vInPorts[idx_in_src]->crop1.y;
    this->m_TGCtrl.m_Crop.w = vInPorts[idx_in_src]->crop1.w;
    this->m_TGCtrl.m_Crop.h = vInPorts[idx_in_src]->crop1.h;

    //asign timestamp clk rate

    CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule, NULL, this->m_pIspDrvCamsv);
    pTime->TimeStamp_SrcClk(vInPorts[idx_in_src]->tTimeClk);

    /// IMGO setting

    if(vOutPorts[idx_imgo]->crop1.floatX || vOutPorts[idx_imgo]->crop1.floatY){
        PIPE_ERR("imgo support no floating-crop_start , replaced by 0\n");
    }

    // use output dma crop
    this->configDmaPort(vOutPorts[idx_imgo],this->m_DmaImgo.dma_cfg ,(MUINT32)pixel_byte_imgo);
    this->m_DmaImgo.m_pDrv = this->m_pIspDrvCamsv;
    this->m_DmaImgo.fmt_sel.Raw = this->m_TopCtrl.camsv_top_ctl.FMT_SEL.Raw;
    this->m_DmaImgo.m_fps = vInPorts[idx_in_src]->tgFps;

    /// IMGO FBC setting
    this->m_FbcImgo.m_pDrv = this->m_pIspDrvCamsv;
    this->m_FbcImgo.m_fps = vInPorts[idx_in_src]->tgFps;
    this->m_FbcImgo.m_pTimeStamp = pTime;

    /// 1. TG config, enable
    /// 2. Top config
    /// 3. IMGO & FBC config, enable
    if(0 != this->m_TGCtrl.config()){
        PIPE_ERR("m_TGCtrl.config fail");
        return MFALSE;
    }

    if(0 != this->m_TopCtrl.config()){
        PIPE_ERR("m_TopCtrl.config fail");
        return MFALSE;
    }

    if(0 != this->m_FbcImgo.config()){
        PIPE_ERR("m_TopCtrl.config fail");
        return MFALSE;
    }

    if(0 != this->m_DmaImgo.config()){
        PIPE_ERR("m_TopCtrl.config fail");
        return MFALSE;
    }

    if(0 != this->m_TGCtrl.enable(NULL)){
        PIPE_ERR("m_TopCtrl.enable fail");
        return MFALSE;
    }

    if(0 != this->m_FbcImgo.enable(NULL)){
        PIPE_ERR("m_FbcImgo.enable fail");
        return MFALSE;
    }

    if(0 != this->m_DmaImgo.enable(NULL)){
        PIPE_ERR("m_DmaImgo.enable fail");
        return MFALSE;
    }

    this->FSM_UPDATE(op_cfg);

    PIPE_INF("CamsvIOPipe::configPipe-");
    return  MTRUE;

}





/*******************************************************************************
*
********************************************************************************/
MBOOL CamsvIOPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL    ret = MTRUE; // 0 for ok , -1 for fail

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08zx,0x%08zx,0x%08zx)", gettid(), cmd, arg1, arg2, arg3);

    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch ( cmd ) {
        case EPIPECmd_SET_FRM_TIME:
            if (MFALSE == this->FSM_CHECK(op_cmd)) {
                PIPE_ERR("EPIPECmd_SET_FRM_TIME FSM error");
                ret = MFALSE;
                break;
            }

            if (MFALSE == this->m_BufCtrl.updateFrameTime(
                    (MUINT32)arg1, (MUINT32)arg2, this->m_hwModule)) {
                PIPE_ERR("Update frame time fail: arg1:%d arg2:%d hwModule:%d", (MUINT32)arg1, (MUINT32)arg2, this->m_hwModule);
                ret = MFALSE;
                break;
            }

            break;
        default:
            PIPE_ERR("NOT support command!");
            ret = MFALSE;
            break;
    }
EXIT:
    if( ret != MTRUE )
    {
        PIPE_ERR("sendCommand(0x%x) error!",cmd);
    }

    return  ret;
}

MINT32 CamsvIOPipe::SrcFmtToHwVal_TG( ImgInfo::EImgFmt_t imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8: return TG_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:return TG_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:return TG_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:return TG_FMT_RAW14;
            break;
        //case eImgFmt_RGB565: return TG_FMT_RGB565;
        //case eImgFmt_RGB888: return TG_FMT_RGB888;
        //case eImgFmt_JPEG:   return TG_FMT_JPG;

        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
             return TG_FMT_YUV422;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}


MINT32 CamsvIOPipe::SrcFmtToHwVal_TG_SW( ImgInfo::EImgFmt_t imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_BAYER14:
            return TG_SW_UYVY; // 0
            break;
        //case eImgFmt_RGB565: return TG_FMT_RGB565;
        //case eImgFmt_RGB888: return TG_FMT_RGB888;
        //case eImgFmt_JPEG:   return TG_FMT_JPG;

        case eImgFmt_YUY2:
            return TG_SW_YUYV;
            break;
        case eImgFmt_UYVY:
            return TG_SW_UYVY;
            break;
        case eImgFmt_YVYU:
            return TG_SW_YVYU;
            break;
        case eImgFmt_VYUY:
             return TG_SW_VYUY;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}


//return HW register format
MINT32 CamsvIOPipe::getOutPxlByteNFmt(ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt)
{
    //
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }


    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_RAW8;
            break;
        case eImgFmt_FG_BAYER8:
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = RRZO_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = IMGO_FMT_RAW10;
            break;
        case eImgFmt_FG_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = RRZO_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW12;
            break;
        case eImgFmt_FG_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW14;
            break;
        case eImgFmt_FG_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW14;
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_YUV422_1P;
            break;
        case eImgFmt_RGB565:
            //*pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB565;
            //break;
        case eImgFmt_RGB888:
            //*pPixel_byte = 3 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB888;
            //break;
        case eImgFmt_JPEG:
        case eImgFmt_BLOB:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW10):(CAM_FMT_SEL_BAYER10);
            //break;
        case eImgFmt_UFO_BAYER10:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = CAM_FMT_SEL_TG_FMT_RAW10;
            //break;
        default:
            PIPE_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    //
    PIPE_INF("input imgFmt(0x%x),output fmt:0x%x,*pPixel_byte(%d)",imgFmt,*pFmt,*pPixel_byte);
    //PIPE_INF(" ");

    return 0;
}


MBOOL CamsvIOPipe::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,E_BufPlaneID planeNum)
{
    (void)isBypassOffset;

    a_dma.memBuf.size        = portInfo->u4BufSize[planeNum];
    a_dma.memBuf.base_vAddr  = portInfo->u4BufVA[planeNum];
    a_dma.memBuf.base_pAddr  = portInfo->u4BufPA[planeNum];
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->u4ImgWidth;
    a_dma.size.h            = portInfo->u4ImgHeight;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->u4Stride[planeNum];

    //
    a_dma.lIspColorfmt = portInfo->eImgFmt;

    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte

    a_dma.size.xsize        =  portInfo->u4ImgWidth;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum == ePlane_1st) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    a_dma.crop.x            = portInfo->crop1.x;
    a_dma.crop.floatX       = 0;//portInfo->crop1.floatX;
    a_dma.crop.y            = portInfo->crop1.y;
    a_dma.crop.floatY       = 0;//portInfo->crop1.floatY;
    a_dma.crop.w            = portInfo->crop1.w;
    a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    a_dma.memBuf.ofst_addr = 0;//offset at isp function
    //

    switch( portInfo->eImgFmt ) {
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=2;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            //break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=2;
            //break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=3;
            //break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in cam\n",portInfo->eImgFmt);
            break;
        case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=1;
            break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
        default:
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=0;
            break;
    }
    //

    PIPE_INF("w(%lu),h(%lu),stride(%lu),xsize(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%zx),pa(0x%zx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_INF("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

MBOOL CamsvIOPipe::irq(Irq_t* pIrq)
{
    ISP_WAIT_IRQ_ST _irq;

    PIPE_INF("CamsvIOPipe::irq+");
    //
    if(this->FSM_CHECK(op_cmd) == MFALSE) {
        PIPE_INF("");
        return MFALSE;
    }

    switch(pIrq->Type){
        case Irq_t::_CLEAR_NONE:
            PIPE_INF("Irq_t::_CLEAR_NONE");
            _irq.Clear = ISP_IRQ_CLEAR_NONE;
            break;
        case Irq_t::_CLEAR_WAIT:
            PIPE_INF("Irq_t::_CLEAR_WAIT");
            _irq.Clear = ISP_IRQ_CLEAR_WAIT;
            break;
        default:
            PIPE_ERR("unsupported type:0x%x\n",pIrq->Type);
            return MFALSE;
            break;
    }

    switch(pIrq->StatusType){
        case Irq_t::_SIGNAL_INT_:
            PIPE_INF("Irq_t::_SIGNAL_INT_");
            switch(pIrq->Status){
                case Irq_t::_VSYNC_:
                    PIPE_INF("Irq_t::_VSYNC_");
                    _irq.Status = SV_VS1_ST;
                    break;
                case Irq_t::_SOF_:
                    PIPE_INF("Irq_t::_SOF_");
                    _irq.Status = SV_SOF_INT_ST;
                    break;
                case Irq_t::_EOF_:
                    PIPE_INF("Irq_t::_EOF_");
                    _irq.Status = SV_EXPDON_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",pIrq->Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = SIGNAL_INT;
            break;
        case Irq_t::_DMA_INT_:
            PIPE_INF("Irq_t::_DMA_INT_");
            switch(pIrq->Status){
                default:
                    PIPE_ERR("unsupported DMA signal:0x%x\n",pIrq->Status);
                    break;
            }
            _irq.St_type = DMA_INT;
            return MFALSE;
            break;
        default:
            PIPE_ERR("unsupported status type:0x%x\n",pIrq->StatusType);
            return MFALSE;
            break;
    }

    _irq.UserKey = pIrq->UserKey;
    _irq.Timeout = pIrq->Timeout;

    PIPE_INF("pIrq->Timeout(%d)", pIrq->Timeout);

    if( this->m_pIspDrvCamsv->waitIrq(&_irq) == MFALSE )
    {
        PIPE_ERR("waitirq error!\n");
        return  MFALSE;
    }

    pIrq->TimeInfo.tLastSig_sec = (MUINT32)_irq.TimeInfo.tLastSig_sec;
    pIrq->TimeInfo.tLastSig_usec = (MUINT32)_irq.TimeInfo.tLastSig_usec;
    pIrq->TimeInfo.tMark2WaitSig_sec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_sec;
    pIrq->TimeInfo.tMark2WaitSig_usec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_usec;
    pIrq->TimeInfo.tLastSig2GetSig_sec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_sec;
    pIrq->TimeInfo.tLastSig2GetSig_usec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_usec;
    pIrq->TimeInfo.passedbySigcnt = (MUINT32)_irq.TimeInfo.passedbySigcnt;

    this->FSM_UPDATE(op_cmd);

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio


