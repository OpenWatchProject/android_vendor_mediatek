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
#define LOG_TAG "sttiopipe"

//
#include <mtkcam/utils/std/Log.h>
//
#include "StatisticIOPipe.h"
//
#include <cutils/properties.h>  // For property_get().
//#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h> // for p1hwcfg module sel
#include <ispio_utility.h>
#include "cam_capibility.h"
#include "imageio_log.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


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



/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/



/*******************************************************************************
*
********************************************************************************/
STTIOPipe::STTIOPipe()
{
    //
    DBG_LOG_CONFIG(imageio, pipe);


    m_FSM = op_unknown;
    m_pipe_opt = ICamIOPipe::STTIO;

    memset((void*)this->m_pCmdQMgr,0,sizeof(DupCmdQMgr*)*ISP_DRV_CAM_BASIC_CQ_NUM);
    m_byPassFLKO = MTRUE;
    m_byPassPDO = MTRUE;
    m_byPassAFO = MTRUE;
    m_byPassAAO = MTRUE;
    m_byPassPSO = MTRUE;

    m_hwModule = CAM_MAX;

    PIPE_INF(":X");
}

STTIOPipe::~STTIOPipe()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::FSM_CHECK(MUINT32 op)
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

MBOOL STTIOPipe::FSM_UPDATE(MUINT32 op)
{
    if(op == op_cmd)
        return MTRUE;
    //
    this->m_FSMLock.lock();
    this->m_FSM = (E_FSM)op;
    this->m_FSMLock.unlock();

    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
static STTIOPipe    gSttioPipe[CAM_MAX];

STTIOPipe* STTIOPipe::Create(MINT8 const szUsrName[32], E_INPUT InPut)
{
    STTIOPipe* ptr = NULL;
    BASE_LOG_INF(":E:user:%s, with input source:0x%x\n",szUsrName,InPut);

    switch(InPut){
        case TG_A:
            if(*gSttioPipe[CAM_A].m_ResMgr.Get_Res() != '\0'){
                BASE_LOG_ERR("TG_A is occupied by user:%s\n",gSttioPipe[CAM_A].m_ResMgr.Get_Res());
            }
            else{
                gSttioPipe[CAM_A].m_ResMgr.m_hwModule = gSttioPipe[CAM_A].m_hwModule = CAM_A;
                gSttioPipe[CAM_A].m_ResMgr.Register_Res((char*)szUsrName);

                ptr = &gSttioPipe[CAM_A];
            }
            break;
        case TG_B:
            if(*gSttioPipe[CAM_B].m_ResMgr.Get_Res() != '\0'){
                BASE_LOG_ERR("TG_B is occupied by user:%s\n",gSttioPipe[CAM_B].m_ResMgr.Get_Res());
            }
            else{
                gSttioPipe[CAM_B].m_ResMgr.m_hwModule = gSttioPipe[CAM_B].m_hwModule = CAM_B;
                gSttioPipe[CAM_B].m_ResMgr.Register_Res((char*)szUsrName);

                ptr = &gSttioPipe[CAM_B];
            }
            break;
        default:
            BASE_LOG_ERR("unsupported input source(%d)\n",InPut);
            break;
    }

    return ptr;
}

void STTIOPipe::Destroy(void)
{
    PIPE_INF(":E:user:%s, with input source:0x%x\n",this->m_ResMgr.Get_Res(),this->m_hwModule);


    this->m_ResMgr.Release_Res();

    this->m_FSMLock.lock();
    this->m_FSM = op_unknown;
    this->m_FSMLock.unlock();

    memset((void*)this->m_pCmdQMgr,0,sizeof(DupCmdQMgr*)*ISP_DRV_CAM_BASIC_CQ_NUM);

    this->m_Twin.clear();

    this->m_byPassFLKO = MTRUE;
    this->m_byPassPDO = MTRUE;
    this->m_byPassAFO = MTRUE;
    this->m_byPassAAO = MTRUE;
    this->m_byPassPSO = MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::init()
{
    PIPE_INF("(%s):E",this->m_ResMgr.Get_Res());
    if(*this->m_ResMgr.Get_Res() == '0'){
        PIPE_ERR("can't null username\n");
        return MFALSE;
    }
    if(this->FSM_CHECK(op_init) == MFALSE)
        return MFALSE;
    //attach cmdQMgr
    //move into configPipe,  because of some input para.
    //this->m_pCmdQMgr->CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq,void * cq_ptr)

    //attach uni_mgr
    //move into configpipe when rawi, otherwise, put in sendcommand.
    this->FSM_UPDATE(op_init);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::uninit()
{
    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
    PIPE_INF("(%s):E",this->m_ResMgr.Get_Res());

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;

    //datach cmdqMgr
    if(this->m_byPassAAO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ4 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }
    }
    if(this->m_byPassAFO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ5 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }

        //
        if(pTwin->getIsTwin(_afo_)) {
            vector<ISP_HW_MODULE>::iterator it;
            vector<ISP_HW_MODULE> l_module ;
            if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                return MFALSE;

            for(it = l_module.begin();it!=l_module.end();it++){
                if(this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_detach()){
                    PIPE_ERR("Twin CQ5 uninit fail at cam:0x%x\n",*it);
                    return MFALSE;
                }

                if(strcmp(gSttioPipe[*it].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) == 0){
                    gSttioPipe[*it].m_ResMgr.Release_Res();
                }
                else{
                    //do nothing, this only means twin had been used before, current pipline is running is multi-sensor
                }
            }
        }
    }

    if(this->m_byPassPDO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ7 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    if(this->m_byPassFLKO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ8 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    if(this->m_byPassPSO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ12 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    this->FSM_UPDATE(op_uninit);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::start()
{
    void* ptr = NULL;
    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
    PIPE_INF(":E");


    if(this->FSM_CHECK(op_start) == MFALSE)
        return MFALSE;


    //start cq
    if(this->m_byPassAAO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ4 start fail\n",this->m_hwModule);
            return MFALSE;
        }
    }
    if(this->m_byPassAFO == MFALSE){

        //
        if(pTwin->getIsTwin(_afo_)) {
            vector<ISP_HW_MODULE>::iterator it;
            vector<ISP_HW_MODULE> l_module;
            if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                return MFALSE;

            for(it = l_module.begin();it!=l_module.end();it++){
                if(this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_start()){
                    PIPE_ERR("Twin CQ5 start fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }
            this->m_Twin.m_TwinStatus = TwinMgr::E_START;
        }

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ5 start fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    if(this->m_byPassPDO == MFALSE){

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ7 start fail\n",this->m_hwModule);
            return MFALSE;
        }

    }

    if(this->m_byPassFLKO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ8 start fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    if(this->m_byPassPSO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ12 start fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    //no dmao enable. dmao enable is via tuning

    this->FSM_UPDATE(op_start);
    PIPE_INF("-\n");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::stop(MBOOL bForce, MBOOL detachUni)
{
    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
    bForce;detachUni;
    PIPE_INF(":E");

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;

    //main purpose is for close FBC
    //for performance consideration,
    //dmao_en is don't case, because of buf will be de-allocated only after camiopipe::stop

    //
    if(this->m_byPassAAO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ4 stop fail\n",this->m_hwModule);
            return MFALSE;
        }
        this->m_AAO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_AAO_FBC.disable();

        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_AAO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AAO.disable();
            }
        }
    }

    if(this->m_byPassAFO == MFALSE){

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ5 stop fail\n",this->m_hwModule);
            return MFALSE;
        }

        this->m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_AFO_FBC.disable();

        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AFO.disable();
            }
        }

        if(pTwin->getIsTwin(_afo_)) {
            vector<ISP_HW_MODULE> l_module;
            if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                return MFALSE;

            vector<ISP_HW_MODULE>::iterator it = l_module.begin();
            for(MUINT32 i = 0;i < l_module.size();i++, it++){
                if(this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_stop()){
                    PIPE_ERR("Twin CQ5 stop fail at cam:0x%x\n",*it);
                    return MFALSE;
                }

                this->m_Twin.m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj(0)[0];
                this->m_Twin.m_AFO_FBC.disable();

                for(MUINT32 j=0;j<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 k=0;k<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetBurstQ();k++){
                        this->m_Twin.m_AFO.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetCurCycleObj(j)[k];
                        this->m_Twin.m_AFO.disable();
                    }
                }
            }
        }


    }

    if(this->m_byPassPDO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ7 stop fail\n",this->m_hwModule);
            return MFALSE;
        }
        this->m_PDO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PDO_FBC.disable();

        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_PDO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PDO.disable();
            }
        }

    }

    if(this->m_byPassFLKO == MFALSE){
        MUINTPTR pDmao,ptr;
        this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
        if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&pDmao,&ptr) == MFALSE){
            return MFALSE;
        }

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ8 stop fail\n",this->m_hwModule);
            return MFALSE;
        }

        //remove into uni_mgr
        //this->m_FLKO_FBC.disable();


        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                ((DMA_FLKO*)pDmao)->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                ((DMA_FLKO*)pDmao)->disable();
            }
        }
    }

    if(this->m_byPassPSO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ12 stop fail\n",this->m_hwModule);
            return MFALSE;
        }
        this->m_PSO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PSO_FBC.disable();

        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_PSO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PSO.disable();
            }
        }
    }

    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, this->m_hwModule,CAM_BUF_CTRL::eCamDmaType_stt);

    this->FSM_UPDATE(op_stop);
    PIPE_INF("-\n");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::abortDma(PortID const port)
{
    MUINT32         dmaChannel = 0;
    E_ISP_CAM_CQ    cq;
    ISP_WAIT_IRQ_ST irq;

    //
    if(this->m_FSM != op_stop){
        PIPE_ERR("STTIOPipe FSM(%d)", this->m_FSM);
        return MFALSE;
    }

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.Timeout = 1000;

    dmaChannel = (MUINT32)this->PortID_MAP(port.index);
    switch(dmaChannel){
        case _aao_:
            cq = ISP_DRV_CQ_THRE4;
            irq.St_type = DMA_INT;
            irq.Status = AAO_DONE_ST;
            break;
        case _afo_:
            cq = ISP_DRV_CQ_THRE5;
            irq.St_type = DMA_INT;
            irq.Status = AFO_DONE_ST;
            break;
        case _pdo_:
            cq = ISP_DRV_CQ_THRE7;
            irq.St_type = DMA_INT;
            irq.Status = PDO_DONE_ST;
            break;
        case _flko_:
            cq = ISP_DRV_CQ_THRE8;
            irq.St_type = DMA_INT;
            irq.Status = FLKO_DONE_ST;
            break;
        case _pso_:
            cq = ISP_DRV_CQ_THRE12;
            irq.St_type = DMA_INT;
            irq.Status = PSO_DONE_ST;
            break;
        default:
            PIPE_ERR("Not support dma(0x%x)\n", port.index);
            return MFALSE;

    }

    if( this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(0)[0]->signalIrq(&irq) == MFALSE )
    {
        PIPE_ERR("abort DMA error!");
        return  MFALSE;
    }

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
_isp_dma_enum_ STTIOPipe::PortID_MAP(MUINT32 PortID)
{
    switch(PortID){
        case EPortIndex_AAO:
            if(this->m_byPassAAO == MFALSE)
                return _aao_;
            else
                PIPE_ERR("[0x%x]:AAO is bypass\n",this->m_hwModule);
            break;
        case EPortIndex_AFO:
            if(this->m_byPassAFO == MFALSE)
                return _afo_;
            else
                PIPE_ERR("[0x%x]:AFO is bypass\n",this->m_hwModule);
            break;
        case EPortIndex_PDO:
            if(this->m_byPassPDO == MFALSE)
                return _pdo_;
            else
                PIPE_ERR("[0x%x]:PDO is bypass\n",this->m_hwModule);
            break;
        case EPortIndex_FLKO:
            if(this->m_byPassFLKO == MFALSE)
                return _flko_;
            else
                PIPE_ERR("[0x%x]:FLKO is bypass\n",this->m_hwModule);
            break;
        case EPortIndex_PSO:
            if(this->m_byPassPSO == MFALSE)
                return _pso_;
            else
                PIPE_ERR("[0x%x]:PSO is bypass\n",this->m_hwModule);
            break;
        default:
            PIPE_ERR("un-supported portID:0x%x\n",PortID);
            break;
    }
    return _cam_max_;
}


/*******************************************************************************
* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
********************************************************************************/
MINT32 STTIOPipe::enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MBOOL bImdMode)
{
    MUINT32         dmaChannel = 0;
    stISP_BUF_INFO  bufInfo;
    CAM_BUF_CTRL* pFBC = NULL;
    MUINT32 targetIdx = 0;
    E_ISP_CAM_CQ cq;
    MINT32 ret = 0;

    //warning free
    bImdMode;

    //
    if(this->FSM_CHECK(op_endeq) == MFALSE){
        if(this->m_FSM == op_stop)
            return 1;
        return -1;
    }
    //
    dmaChannel = (MUINT32)this->PortID_MAP(portID.index);

    switch(dmaChannel){
        case _aao_:
            pFBC = &this->m_AAO_FBC;
            cq = ISP_DRV_CQ_THRE4;
            break;
        case _afo_:
            pFBC = &this->m_AFO_FBC;
            cq = ISP_DRV_CQ_THRE5;
            break;
        case _pdo_:
            pFBC = &this->m_PDO_FBC;
            cq = ISP_DRV_CQ_THRE7;
            break;
        case _flko_:
            if(this->m_UniMgr.UniMgr_GetCurLink() != this->m_hwModule){
                if(this->m_FSM == op_stop && this->m_UniMgr.UniMgr_GetCurLink() == CAM_MAX){
                    PIPE_DBG("warning: [%d] uni:FSM=stop",this->m_hwModule);
                    return 1;
                }
                PIPE_ERR("[%d] uni is linked with cam:%d\n",this->m_hwModule,this->m_UniMgr.UniMgr_GetCurLink());
                return -1;
            }
            {
                MUINTPTR _pfbc,ptr;
                this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
                if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&ptr,&_pfbc) == MFALSE){
                    return -1;
                }
                pFBC = (CAM_BUF_CTRL*)_pfbc;
            }
            cq = ISP_DRV_CQ_THRE8;

            pFBC->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
            break;
        case _pso_:
            pFBC = &this->m_PSO_FBC;
            cq = ISP_DRV_CQ_THRE12;
            break;
        default:
            PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
            return -1;
            break;
    }

    if(this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ()!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("enque buf number is mismatched with subsample (0x%x_0x%x)\n",(MUINT32)rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ());
        return -1;
    }

    if(this->m_FSM == op_start)
        targetIdx = ((this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx() + 1)% this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQ());
    else
        targetIdx = this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx();

    if(rQBufInfo.vBufInfo.size() != 1){
        PIPE_ERR("sttiopipe support only enque-1-frame-at-1-time\n");
        return -1;
    }

    for(MUINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_ERR("replace function are not supported in STTIOPipe\n");
            return -1;
        }
        bufInfo.u_op.enque.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[0];
        bufInfo.u_op.enque.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[0];
        bufInfo.u_op.enque.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[0];
        bufInfo.u_op.enque.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[0];
        bufInfo.u_op.enque.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[0];
        bufInfo.u_op.enque.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[0];

        //header
        bufInfo.u_op.enque.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA;
        bufInfo.u_op.enque.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA;
        bufInfo.u_op.enque.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize;
        bufInfo.u_op.enque.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID;
        bufInfo.u_op.enque.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu;
        bufInfo.u_op.enque.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe;

        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
            i,\
            portID.index, bufInfo.u_op.enque.image.mem_info.memID,\
            bufInfo.u_op.enque.image.mem_info.va_addr,\
            bufInfo.u_op.enque.image.mem_info.pa_addr,\
            bufInfo.u_op.enque.header.va_addr);



        //
        pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if(eCmd_Fail == pFBC->enqueueHwBuf(bufInfo)){
            PIPE_ERR("enque fail at burst:0x%x\n",i);
            ret = -1;
        }

        // If twin, enque twinHwModule's afo
        switch(dmaChannel){
            case _afo_:
                {
                    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
                    if(pTwin->getIsTwin(dmaChannel)) {
                        vector<ISP_HW_MODULE>::iterator it3;
                        vector<ISP_HW_MODULE> l_module;
                        if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                            return MFALSE;

                        for(it3 = l_module.begin();it3!=l_module.end();it3++){
                            if(this->m_FSM == op_start)
                                targetIdx = ((this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQIdx() + 1)% this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQ());
                            else
                                targetIdx = this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQIdx();

                            this->m_Twin.m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                            this->m_Twin.m_AFO_FBC.enqueueHwBuf(bufInfo);
                        }
                    }
                }
                break;
            default:
                break;
        }

    }


    this->FSM_UPDATE(op_cmd);
    return  ret;
}


/*******************************************************************************
* return value:
* 2: suspending
* 1: already stopped
* 0: sucessed
*-1: fail
********************************************************************************/
E_BUF_STATUS STTIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    MUINT32 dmaChannel = 0;
    CAM_BUF_CTRL* pFBC = NULL;
    BufInfo buf;
    E_BUF_STATUS ret = eBuf_Pass;
    E_ISP_CAM_CQ cq;

    //warning free
    u4TimeoutMs;
    //
    if(this->FSM_CHECK(op_endeq) == MFALSE){
        if(this->m_FSM == op_stop)
            return eBuf_stopped;
        return eBuf_Fail;
    }

    //
    dmaChannel = (MUINT32)this->PortID_MAP(portID.index);
    switch(dmaChannel){
        case _aao_:
            pFBC = &this->m_AAO_FBC;
            cq = ISP_DRV_CQ_THRE4;
            break;
        case _afo_:
            pFBC = &this->m_AFO_FBC;
            cq = ISP_DRV_CQ_THRE5;
            break;
        case _pdo_:
            pFBC = &this->m_PDO_FBC;
            cq = ISP_DRV_CQ_THRE7;
            break;
        case _flko_:
            if(this->m_UniMgr.UniMgr_GetCurLink() != this->m_hwModule){
                if(this->m_FSM == op_stop && this->m_UniMgr.UniMgr_GetCurLink() == CAM_MAX){
                    PIPE_DBG("warning: [%d] uni:FSM=stop",this->m_hwModule);
                    return eBuf_stopped;
                }
                PIPE_ERR("[%d] uni is linked with cam:%d\n",this->m_hwModule,this->m_UniMgr.UniMgr_GetCurLink());
                return eBuf_Fail;
            }
            {
                MUINTPTR _pfbc,ptr;
                this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
                if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&ptr,&_pfbc) == MFALSE){
                    return eBuf_Fail;
                }
                pFBC = (CAM_BUF_CTRL*)_pfbc;
            }
            cq = ISP_DRV_CQ_THRE8;

            pFBC->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
            break;
        case _pso_:
            pFBC = &this->m_PSO_FBC;
            cq = ISP_DRV_CQ_THRE12;
            break;
        default:
            PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
            return eBuf_Fail;
            break;
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();


     //cfg for waitbufready
    pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];


    switch(dmaChannel){
        case _afo_:
            {
                TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);

                //check if there is already filled buffer
                switch(this->m_AFO_FBC.waitBufReady(MTRUE, pNotify)){
                    case eCmd_Fail:
                        return eBuf_Fail;
                        break;
                    case eCmd_Stop_Pass:
                        PIPE_WRN("waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                        return eBuf_stopped;
                        break;
                    case eCmd_Suspending_Pass:
                        //design concept is just the same with main image.
                        //need to do deque , in order to pop all those requests in drv.
                        //but acually, behavior is depended on the application of iopipe
                        break;
                    default:
                        break;
                }
                if(pTwin->getIsTwin(dmaChannel,MTRUE)) {
                    vector<ISP_HW_MODULE>::iterator it3;
                    vector<ISP_HW_MODULE> l_module;
                    if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                        return eBuf_Fail;

                    for(it3 = l_module.begin();it3!=l_module.end();it3++){
                        this->m_Twin.m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetCurCycleObj(this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQIdx())[0];
                        //check if there is already filled buffer
                        switch(this->m_Twin.m_AFO_FBC.waitBufReady(MFALSE, pNotify)){
                            case eCmd_Fail:
                                return eBuf_Fail;
                                break;
                            case eCmd_Stop_Pass:
                                PIPE_WRN("waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                                return eBuf_stopped;
                                break;
                            case eCmd_Suspending_Pass:
                                //design concept is just the same with main image.
                                //but acually, behavior is depended on the application of iopipe
                                //need to do deque , in order to pop all those requests in drv.
                                break;
                            default:
                                break;
                        }
                    }

                }
            }
            break;
        default:
            //pdo have an extra private data
            if(dmaChannel == _pdo_){
                buf.m_pPrivate = (MUINT32*)&this->m_Xmx;
            }
            //check if there is already filled buffer
            switch(pFBC->waitBufReady(pNotify)){
                case eCmd_Fail:
                    return eBuf_Fail;
                    break;
                case eCmd_Stop_Pass:
                    PIPE_WRN("waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                    return eBuf_stopped;
                    break;
                case eCmd_Suspending_Pass:
                    //design concept is just the same with main image.
                    //but acually, behavior is depended on the application of iopipe
                    //need to do deque , in order to pop all those requests in drv.
                    break;
                default:
                    break;
            }
            break;
    }

    for(MUINT32 i=0;i<this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ();i++){

        pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[i];
        switch(pFBC->dequeueHwBuf( buf )){
            case eCmd_Fail:
                PIPE_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
                ret = eBuf_Fail;
                break;
            case eCmd_Stop_Pass:
                PIPE_WRN("current status:eCmd_Stop_Pass\n");
                ret = eBuf_stopped;
                break;
            case eCmd_Suspending_Pass:
                PIPE_WRN("current status:eCmd_Suspending_Pass\n");
                ret = eBuf_suspending;
                break;
            default:
                break;
        }
        rQBufInfo.vBufInfo.push_back(buf);

        //
        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
            i,\
            portID.index, rQBufInfo.vBufInfo[i].memID[0],\
            rQBufInfo.vBufInfo[i].u4BufVA[0],\
            rQBufInfo.vBufInfo[i].u4BufPA[0],\
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA);
    }

    // If twin, enque twinHwModule's afo
    switch(dmaChannel){
        case _afo_:
            {
                if(ret != eBuf_Pass)
                    break;

                TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
                if(pTwin->getIsTwin(dmaChannel,MTRUE)) {
                    vector<ISP_HW_MODULE>::iterator it3;
                    vector<ISP_HW_MODULE> l_module;
                    if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                        return eBuf_Fail;

                    //
                    for(it3 = l_module.begin();it3!=l_module.end();it3++){

                        for(MUINT32 i=0;i<this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetBurstQ();i++){
                            this->m_Twin.m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetCurCycleObj(this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQIdx())[i];
                            //
                            switch(this->m_Twin.m_AFO_FBC.dequeueHwBuf( buf )){
                                case eCmd_Fail:
                                    PIPE_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
                                    ret = eBuf_Fail;
                                    break;
                                case eCmd_Stop_Pass:
                                    PIPE_WRN("current status:eCmd_Stop_Pass\n");
                                    ret = eBuf_stopped;
                                    break;
                                case eCmd_Suspending_Pass:
                                    PIPE_WRN("current status:eCmd_Suspending_Pass\n");
                                    ret = eBuf_suspending;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }

                }
                //need to pop even if current path is single
                pTwin->popTwinRst(dmaChannel);
            }
            break;
        default:
            break;
    }


    if(rQBufInfo.vBufInfo.size() != this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ())
        PIPE_ERR("dequeout data length is mismatch");

    //
    /* prevent change FSM after stop, need the status of stop */
    if(ret == eBuf_Pass)
        this->FSM_UPDATE(op_cmd);
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts,CAMIO_Func func)
{
    MINT32 idx_aao = -1;
    MINT32 idx_afo = -1;
    MINT32 idx_pdo = -1;
    MINT32 idx_flko = -1;
    MINT32 idx_pso = -1;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_THRE0;
    MUINT32 subsample = func.Bits.SUBSAMPLE;
    TwinMgr* pTwin = NULL;

    //
    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    if (0 == vOutPorts.size()) {
        PIPE_ERR("outport:x%x size err\n", (MUINT32)vOutPorts.size());
        return MFALSE;
    }

    //output port
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) {
            PIPE_INF("dummy output vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("statistic vOutPorts:[%d]:\n", i);

        //
        switch(vOutPorts[i]->index){
            case EPortIndex_AAO:
                idx_aao = i;
                cq = ISP_DRV_CQ_THRE4;
                if (vInPorts.size()) {
                    switch(vInPorts[0]->eImgFmt){
                    case eImgFmt_BAYER8:
                    case eImgFmt_BAYER10:
                    case eImgFmt_BAYER12:
                    case eImgFmt_BAYER14:
                        this->m_byPassAAO = MFALSE;
                        break;
                    default:
                        this->m_byPassAAO = MTRUE;
                        PIPE_ERR("AAO r not supported at non-bayer format\n");
                        break;
                    }
                }
                else
                    this->m_byPassAAO = MFALSE;
                break;
            case EPortIndex_AFO:
                {
                    capibility CamInfo;
                    idx_afo = i;
                    cq = ISP_DRV_CQ_THRE5;
                    this->m_byPassAFO = MFALSE;

                    if(CamInfo.m_DTwin.GetDTwin()){
                        PIPE_ERR("AF should be controlled by CCU under dynamic twin\n");
                        this->m_byPassAFO = MFALSE;
                    }
                }
                break;
            case EPortIndex_FLKO:
                this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
                if(this->m_UniMgr.UniMgr_GetCurLink() != this->m_hwModule){
                    PIPE_ERR("[%d]:FLKO is connected with cam:0x%x, bypass flko\n",this->m_hwModule,this->m_UniMgr.UniMgr_GetCurLink());
                }
                else{
                    idx_flko = i;
                    cq = ISP_DRV_CQ_THRE8;
                    this->m_byPassFLKO = MFALSE;
                }
                break;
            case EPortIndex_PDO:
                idx_pdo = i;
                cq = ISP_DRV_CQ_THRE7;
                if(vInPorts.size()){
                    switch(vInPorts[0]->eImgFmt){
                    case eImgFmt_BAYER8:
                    case eImgFmt_BAYER10:
                    case eImgFmt_BAYER12:
                    case eImgFmt_BAYER14:
                        this->m_byPassPDO = MFALSE;
                        break;
                    default:
                        this->m_byPassPDO = MTRUE;
                        PIPE_ERR("PDO r not supported at non-bayer format\n");
                        break;
                    }
                }
                else
                    this->m_byPassPDO = MFALSE;
                break;
            case EPortIndex_PSO:
                idx_pso = i;
                cq = ISP_DRV_CQ_THRE12;
                if(vInPorts.size()){
                    switch(vInPorts[0]->eImgFmt){
                    case eImgFmt_BAYER8:
                    case eImgFmt_BAYER10:
                    case eImgFmt_BAYER12:
                    case eImgFmt_BAYER14:
                        this->m_byPassPSO = MFALSE;
                        break;
                    default:
                        this->m_byPassPSO = MTRUE;
                        PIPE_ERR("PSO r not supported at non-bayer format\n");
                        break;
                    }
                }
                else
                    this->m_byPassPSO = MFALSE;
                break;
            default:
                PIPE_ERR("unspuported port:0x%x\n",vOutPorts[i]->index);
                return MFALSE;
                break;
        }

        //cfg cmdQ
        if((this->m_pCmdQMgr[cq] = DupCmdQMgr::CmdQMgr_attach(0,subsample,this->m_hwModule,cq)) == NULL){
            PIPE_ERR("CQ init fail:0x%x\n",cq);
            return MFALSE;
        }


    }

    /// Twin: Only need to take care AFO
    // get twin status from TwinMgr
    pTwin = TwinMgr::createInstance(this->m_hwModule);
    // If AFO is enable, attach CmdQ thread 5 for twin use
    if(MFALSE == this->m_byPassAFO) {
        cq = ISP_DRV_CQ_THRE5;
        if(pTwin->getIsTwin(_afo_)) {
            vector<ISP_HW_MODULE>::iterator it;
            vector<ISP_HW_MODULE> l_module;
            vector<DupCmdQMgr*> v_pCqMgr;
            if(pTwin->sendCommand(TWIN_CMD_GET_CUR_SLAVE_CQMGR,(MINTPTR)ISP_DRV_CQ_THRE0,(MINTPTR)&l_module,(MINTPTR)&v_pCqMgr) == MFALSE)
                return MFALSE;
            else
                it = l_module.begin();
            //
            for(MUINT32 i=0;i<l_module.size();i++,it++){
                if(*gSttioPipe[*it].m_ResMgr.Get_Res() == '\0')
                {
                    gSttioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
                    gSttioPipe[*it].m_ResMgr.m_hwModule = gSttioPipe[*it].m_hwModule = *it;


                    PIPE_INF("Twin enable!! Occupy STTIoPipe: module(%d) by %s.", *it,gSttioPipe[*it].m_ResMgr.Get_Res());
                }
                else{
                    PIPE_ERR("twin init fail,module(%d) is occupied by user:%s\n", *it, \
                        gSttioPipe[*it].m_ResMgr.Get_Res());
                    return MFALSE;
                }

                //cfg cmdQ
                if(*it == CAM_B){//so far, design is only cam_b only. align twin drv
                    capibility CamInfo;
                    if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                        if((this->m_Twin.m_pTwinCQMgr[cq][*it] = DupCmdQMgr::CmdQMgr_attach(0,subsample,CAM_B,cq)) == NULL){
                            PIPE_ERR("Twin CQ_%d init fail\n",cq);
                            return MFALSE;
                        }
                    }
                    else{
                        if((this->m_Twin.m_pTwinCQMgr[cq][*it] = DupCmdQMgr::CmdQMgr_attach(0,subsample,CAM_A_TWIN,cq)) == NULL){
                            PIPE_ERR("Twin CQ_%d init fail\n",cq);
                            return MFALSE;
                        }
                    }
                    this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it] = v_pCqMgr.at(i);
                }
                else{
                    PIPE_ERR("virtual cam is only cam_b only\n");
                    return MFALSE;
                }
            }

            //
            this->m_Twin.m_TwinStatus = TwinMgr::E_INIT;
        }
    }

    this->m_pCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_GetCurMgrObj(this->m_hwModule,ISP_DRV_CQ_THRE0);
    if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0] == NULL){
        PIPE_ERR("camiopipe configure fail or not configure yet!\n");
        return MFALSE;
    }


    //aao
    if(this->m_byPassAAO == MFALSE){
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_GetBurstQ();i++){
                this->m_AAO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AAO.Init();
            }
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_AAO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AAO.write2CQ();
                //this->m_AAO.config();, remove, setting via cam_tuning_ctrl
                this->m_AAO.enable(NULL);
            }
        }
        this->m_AAO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE4]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_AAO_FBC.config();
        this->m_AAO_FBC.enable((void*)&subsample);

        //
        this->m_AAO_FBC.m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_hwModule,NULL);
    }
    //afo
    if(this->m_byPassAFO == MFALSE){
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetBurstQ();i++){
                this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AFO.Init();
            }
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_AFO.write2CQ();
                //this->m_AFO.config();remove, setting via cam_tuning_ctrl
                this->m_AFO.enable(NULL);
            }
        }


        this->m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_AFO_FBC.config();
        this->m_AFO_FBC.enable((void*)&subsample);

        //
        this->m_AFO_FBC.m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_hwModule,NULL);

        // afo for twin
        if(pTwin->getIsTwin(_afo_)) {
            vector<ISP_HW_MODULE>::iterator it;
            vector<ISP_HW_MODULE> l_module;
            if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                return MFALSE;

            for(it = l_module.begin();it!=l_module.end();it++){
                for(MUINT32 j=0;j<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetBurstQ();i++){
                        this->m_Twin.m_AFO.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj(j)[i];
                        this->m_Twin.m_AFO.Init();
                    }
                }


                for(MUINT32 j=0;j<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetBurstQ();i++){
                        this->m_Twin.m_AFO.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetCurCycleObj(j)[i];
                        this->m_Twin.m_AFO.write2CQ();
                        //this->m_AFO.config();remove, setting via cam_tuning_ctrl
                        this->m_Twin.m_AFO.enable(NULL);
                    }
                }

                this->m_Twin.m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj(0)[0];
                this->m_Twin.m_AFO_FBC.config();
                this->m_Twin.m_AFO_FBC.enable((void*)&subsample);

                //master & slave cam must share the same timestamp obj
                this->m_Twin.m_AFO_FBC.m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_hwModule,NULL);
            }
        }

    }
    //pdo
    if(this->m_byPassPDO == MFALSE){
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetBurstQ();i++){
                this->m_PDO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PDO.Init();
            }
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_PDO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PDO.write2CQ();
                //this->m_PDO.config();remove, setting via cam_tuning_ctrl
                this->m_PDO.enable(NULL);

                switch(func.Bits.Density){
                    case 0:
                        this->m_DPD_CTRL.m_Density = CAM_DPD_CTRL::_level_1;
                        break;
                    case 1:
                        this->m_DPD_CTRL.m_Density = CAM_DPD_CTRL::_level_2;
                        PIPE_ERR("Cur Density:%d is not ready\n",func.Bits.Density);
                        break;
                    default:
                        PIPE_ERR("unsupported Density:%d\n",func.Bits.Density);
                        break;
                }

                this->m_DPD_CTRL.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_DPD_CTRL.config();
                this->m_DPD_CTRL.write2CQ();
            }
        }
        this->m_PDO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PDO_FBC.config();
        this->m_PDO_FBC.enable((void*)&subsample);
        //
        this->m_PDO_FBC.m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_hwModule,NULL);

    }
    //flko
    if(this->m_byPassFLKO == MFALSE){
        MUINTPTR pDmao,ptr;
        this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
        if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&pDmao,&ptr) == MFALSE){
            return MFALSE;
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_GetBurstQ();i++){
                ((DMA_FLKO*)pDmao)->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_GetCurCycleObj(j)[i];
                ((DMA_FLKO*)pDmao)->Init();
            }
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                ((DMA_FLKO*)pDmao)->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                ((DMA_FLKO*)pDmao)->write2CQ();
                //Dmao.config();
                ((DMA_FLKO*)pDmao)->enable(NULL);
            }
        }
        #if 0 //remove into uni_mgr
        if((this->m_FLKO_FBC.m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj()) == NULL){
            PIPE_ERR("flko get uni fail\n");
            return MFALSE;
        }
        this->m_FLKO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_FLKO_FBC.config();
        this->m_FLKO_FBC.enable((void*)&subsample);
        #endif
    }

    //pso
    if(this->m_byPassPSO == MFALSE){
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_GetBurstQ();i++){
                this->m_PSO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PSO.Init();
            }
        }
        for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
            for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                this->m_PSO.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                this->m_PSO.write2CQ();
                //this->m_PSO.config();, remove, setting via cam_tuning_ctrl
                this->m_PSO.enable(NULL);
            }
        }
        this->m_PSO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE12]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PSO_FBC.config();
        this->m_PSO_FBC.enable((void*)&subsample);
        //
        this->m_PSO_FBC.m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_hwModule,NULL);
    }

    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, this->m_hwModule,CAM_BUF_CTRL::eCamDmaType_stt);

    this->FSM_UPDATE(op_cfg);
    PIPE_INF("-\n");
    return  MTRUE;
}





/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0; // 0 for ok , -1 for fail

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%" PRIXPTR ",0x%" PRIXPTR ",0x%" PRIXPTR ")", gettid(), cmd, arg1, arg2, arg3);

    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch ( cmd ) {
#if 0
        case EPIPECmd_SET_P1_UPDATE:
            for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++){
                if(this->m_pCmdQMgr[i] != NULL)
                    if(this->m_pCmdQMgr[i]->CmdQMgr_update()){
                        PIPE_ERR("[%d]:CQ undate fail\n",this->m_hwModule);
                        return MFALSE;
                    }
            }
            break;
#endif
        case EPIPECmd_SET_STT_BA:
            //arg1: type:EPortIndex,
            //arg2: list of image PA,MUINTPTR. list size must = 4
            //arg3: list of header PA,MUINTPTR. list size must = 4
            {
                #define RING_DEPTH  4
                MUINT32 dmaChannel = 0;
                #define STR_LENGTH  64

                vector<MUINTPTR>* pImgPA_L;
                vector<MUINTPTR>* pFHPA_L;
                vector<MUINTPTR>::iterator it;
                vector<MUINTPTR>::iterator it2;
                E_ISP_CAM_CQ cq;
                DMAO_B *pDmao = NULL;
                MUINT32 i;
                char tmp[STR_LENGTH];
                char str[STR_LENGTH * RING_DEPTH];
                str[0] = '\0';
                if(this->m_FSM != op_cfg){
                    PIPE_ERR("this cmd:0x%x is only avail at cfg stage, cur stage:0x%x\n",cmd,this->m_FSM);
                    return MFALSE;
                }

                dmaChannel = (MUINT32)this->PortID_MAP(arg1);

                pImgPA_L = (vector<MUINTPTR>*)arg2;
                pFHPA_L = (vector<MUINTPTR>*)arg3;
#if 0   //temp mark
                if( (pImgPA_L->size() != RING_DEPTH) || (pFHPA_L->size() != RING_DEPTH) ){
                    PIPE_ERR("list size must be 4\n");
                    return MFALSE;
                }
#endif
                switch(dmaChannel){
                    case _aao_:
                        pDmao = &this->m_AAO;
                        cq = ISP_DRV_CQ_THRE4;
                        break;
                    case _afo_:
                        pDmao = &this->m_AFO;
                        cq = ISP_DRV_CQ_THRE5;
                        break;
                    case _pdo_:
                        pDmao = &this->m_PDO;
                        cq = ISP_DRV_CQ_THRE7;
                        break;
                    case _flko_:
                        MUINTPTR _pdma,ptr;
                        if(this->m_UniMgr.UniMgr_GetCurLink() != this->m_hwModule){
                            PIPE_ERR("[%d] uni is linked with cam:%d\n",this->m_hwModule,this->m_UniMgr.UniMgr_GetCurLink());
                            return MFALSE;
                        }
                        this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
                        if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&_pdma,&ptr) == MFALSE){
                            return MFALSE;
                        }
                        pDmao = (DMAO_B *)_pdma;

                        cq = ISP_DRV_CQ_THRE8;

                        pDmao->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
                        break;
                    case _pso_:
                        pDmao = &this->m_PSO;
                        cq = ISP_DRV_CQ_THRE12;
                        break;
                    default:
                        PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
                        return MFALSE;
                        break;
                }

                this->m_Twin.m_FHPA_L.clear();
                this->m_Twin.m_ImgPA_L.clear();
                for(i=0,it = pImgPA_L->begin(),it2 = pFHPA_L->begin();it != pImgPA_L->end() ;it++,it2++, i++){
                    switch(dmaChannel){
                        case _afo_:
                            {
                                TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);

                                if(pTwin->getIsTwin(dmaChannel)) {
                                    vector<ISP_HW_MODULE>::iterator it3;
                                    vector<ISP_HW_MODULE> l_module;
                                    if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                                        return MFALSE;

                                    for(it3 = l_module.begin();it3!=l_module.end();it3++){
                                        // update other cam's AFO BA under twin
                                        this->m_Twin.m_AFO.m_pIspDrv = (IspDrvVir*)this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetCurCycleObj( \
                                        this->m_Twin.m_pTwinCQMgr[cq][*it3]->CmdQMgr_GetDuqQIdx())[0];

                                        this->m_Twin.m_AFO.dma_cfg.memBuf.base_pAddr = *it;
                                        this->m_Twin.m_AFO.Header_Addr = *it2;

                                        if(this->m_Twin.m_AFO.setBaseAddr()){
                                            PIPE_ERR("[twin]set baseaddress at ring depth:0x%x at cam:0x%x\n",i,*it3);
                                            ret = MFALSE;
                                        }
                                    }
                                }
                                else{//reserved for twin mode. twin is not needed before isp_start()
                                    this->m_Twin.m_ImgPA_L.push_back(*it);
                                    this->m_Twin.m_FHPA_L.push_back(*it2);
                                }
                            }
                            break;
                        default:
                            break;
                    }

                    pDmao->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                        this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];

                    pDmao->dma_cfg.memBuf.base_pAddr = *it;
                    pDmao->Header_Addr = *it2;

                    if(pDmao->setBaseAddr()){
                        PIPE_ERR("set baseaddress at ring depth:0x%x\n",i);
                        ret = MFALSE;
                        goto EXIT;
                    }

                    snprintf(tmp,sizeof(tmp), "0x%x_0x%x,",(MUINT32)*it,(MUINT32)*it2);
                    strncat(str,tmp, (sizeof(str) - strlen(str)) - 1);
                }

                PIPE_INF("statistic vOutPorts:dma:0x%x, %s\n",dmaChannel,str);
                #undef RING_DEPTH
                #undef STR_LENGTH
            }
            break;
        case EPIPECmd_GET_AFO_CONS:
            *(MUINT32*)arg1 = (AFO_DUMMY_WIN + AFO_MAX_SIZE);
            break;
        case EPIPECmd_GET_AAO_CONS:
            *(MUINT32*)arg1 = AAO_MAX_SIZE;
            break;
        case EPIPECmd_GET_PSO_CONS:
            *(MUINT32*)arg1 = PSO_MAX_SIZE;
            break;
        case EPIPECmd_GET_FLKO_CONS:
            *(MUINT32*)arg1 = FLKO_MAX_SIZE(arg3);
            break;
        case EPIPECmd_GET_PDO_CONS:
            //arg1: output size_w
            //arg2: output size_h
            {
                MUINT32 Width,Height;

                if(CAM_READ_REG(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_SEL) & PMX_SEL_1){
                    //input size is useless here. max size is clamped by hw
                    switch(this->m_DPD_CTRL.m_Density){
                        case CAM_DPD_CTRL::_level_1:
                            Width = CAM_READ_BITS(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL,PXL_E) - \
                                CAM_READ_BITS(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL,PXL_S);
                            Height = CAM_READ_BITS(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN,LIN_E) - \
                                CAM_READ_BITS(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN,LIN_S);

                            Width = PBIN_WIDTH(Width);
                            Height = PBIN_HEIGHT(Height);
                            break;
                        case CAM_DPD_CTRL::_level_2:
                        default:
                            PIPE_ERR("this density is not supported(%d\n\n",this->m_DPD_CTRL.m_Density);
                            ret = -1;
                            goto EXIT;
                            break;
                    }
                }
                else{
                    Width = (MUINT32)arg2;
                    Height = (MUINT32)arg3;
                }

                *(MUINT32*)arg1 = PDO_MAX_SIZE(Width,Height);
                PIPE_DBG("PDO size_%d (%d x %d)",*(MUINT32*)arg1, Width,Height);
            }
            break;
        case EPIPECmd_GET_STT_CUR_BUF:
            {
                //arg1: [in] dma port.index
                //arg2: [out] MUINTPTR *: dma buf
                MUINT32 _portidx = (MUINT32)arg1;
                E_ISP_CAM_CQ cq;
                //for FLKO use
                DMAO_B *pDmao = NULL;
                MUINTPTR _pdma, ptr;

                if(!arg2){
                    PIPE_ERR("dma(0x%x)param err\n", _portidx);
                    ret = -1;
                    goto EXIT;
                }

                switch (_portidx){
                case EPortIndex_AAO:
                    cq = ISP_DRV_CQ_THRE4;
                    *(MUINTPTR*)arg2 = CAM_READ_REG(this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                        this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0]->getPhyObj(),CAM_AAO_BASE_ADDR);
                    break;
                case EPortIndex_AFO:
                    cq = ISP_DRV_CQ_THRE5;
                    *(MUINTPTR*)arg2 = CAM_READ_REG(this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                        this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0]->getPhyObj(),CAM_AFO_BASE_ADDR);
                    break;
                case EPortIndex_PDO:
                    cq = ISP_DRV_CQ_THRE7;
                    *(MUINTPTR*)arg2 = CAM_READ_REG(this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                        this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0]->getPhyObj(),CAM_PDO_BASE_ADDR);
                    break;
                case EPortIndex_FLKO:
                    this->m_UniMgr.m_module = this->m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
                    if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&_pdma,&ptr) == MFALSE){
                        ret = -1;
                        break;
                    }
                    pDmao = (DMAO_B *)_pdma;
                    pDmao->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
                    *(MUINTPTR*)arg2 = UNI_READ_REG(pDmao->m_pUniDrv,CAM_UNI_FLKO_BASE_ADDR);
                    break;
                case EPortIndex_PSO:
                    cq = ISP_DRV_CQ_THRE12;
                    *(MUINTPTR*)arg2 = CAM_READ_REG(this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                        this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0]->getPhyObj(),CAM_PSO_BASE_ADDR);
                    break;
                default:
                    PIPE_ERR("not support dma(0x%x)\n", _portidx);
                    ret = -1;
                    goto EXIT;
                }
            }
            break;
        case EPIPECmd_GET_HEADER_SIZE:
            //arg1: input portID, eg:NSImageio::NSIspio::EPortIndex_IMGO
            //arg2: return size of Header, unit: byte
            {
                capibility CamInfo;
                tCAM_rst rst;
                MUINT32 channel;

                if(CamInfo.GetCapibility(arg1,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                            rst,E_CAM_HEADER_size) == MFALSE){
                    ret = 1;
                    goto EXIT;
                }

                channel = this->PortID_MAP(arg1);

                {//always add twin'size due to dynamic twin
                    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
                    MUINT32 size;
                    if(pTwin->sendCommand(TWIN_CMD_GET_SLAVE_HEADERSIZE,(MINTPTR)channel,(MINTPTR)&size,0) == MFALSE){
                        ret = 1;
                        goto EXIT;
                    }
                    *(MUINT32*)arg2 = rst.HeaderSize + size;
                }
            }
            break;
        default:
            PIPE_ERR("NOT support command!");
            ret = -1;
            break;
    }
EXIT:
    if( ret != 0 )
    {
        PIPE_ERR("sendCommand(0x%x) error!",cmd);
        return MFALSE;
    }
    return  MTRUE;
}

/**
    note: only for AF's twin ctrl
*/

MBOOL STTIOPipe::configFrame(E_FRM_EVENT event,CAMIO_Func func)
{
    MBOOL ret = MTRUE;
    TwinMgr* pTwin = TwinMgr::createInstance(this->m_hwModule);
    event;
    func.Raw;

    if(pTwin->getIsTwin(_afo_))  {
        this->m_Twin.TwinPath(this);
    }
    else{
        this->m_Twin.SinglePath(this);
    }

    return ret;
}

MBOOL STTIOPipe::TWIN_STT::TwinPath(STTIOPipe* _this)
{
    TwinMgr* pTwin = TwinMgr::createInstance(_this->m_hwModule);
    this->m_hwModule = _this->m_hwModule;

    switch(this->m_TwinStatus){
        case TwinMgr::E_NOTWIN:
            {
                vector<ISP_HW_MODULE>::iterator it;
                MUINT32 subsample = _this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetBurstQ();
                vector<ISP_HW_MODULE> l_module;
                vector<DupCmdQMgr*> v_pCqMgr;
                if(pTwin->sendCommand(TWIN_CMD_GET_CUR_SLAVE_CQMGR,(MINTPTR)ISP_DRV_CQ_THRE0,(MINTPTR)&l_module,(MINTPTR)&v_pCqMgr) == MFALSE)
                    return MFALSE;
                else
                    it = l_module.begin();
                //
                for(MUINT32 k=0;k<l_module.size();k++,it++){
                    //resource check
                    if(*gSttioPipe[*it].m_ResMgr.Get_Res() == '\0')
                    {
                        gSttioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
                        gSttioPipe[*it].m_ResMgr.m_hwModule = gSttioPipe[*it].m_hwModule = *it;
                    }
                    else{
                        PIPE_ERR("twin init fail,module(%d) is occupied by user:%s\n", *it, \
                            gSttioPipe[*it].m_ResMgr.Get_Res());
                        return MFALSE;
                    }

                    //cfg cmdQ
                    if(*it == CAM_B){//so far, design is only cam_b only. align twin drv
                        if((this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it] = DupCmdQMgr::CmdQMgr_attach(0,\
                            subsample,CAM_A_TWIN,ISP_DRV_CQ_THRE5)) == NULL){
                            PIPE_ERR("Twin CQ_%d init fail\n",ISP_DRV_CQ_THRE5);
                            return MFALSE;
                        }

                        this->m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it] = v_pCqMgr.at(k);
                    }
                    else{
                        PIPE_ERR("virtual cam is only cam_b only\n");
                        return MFALSE;
                    }

                    //cfg dmao/fbc
                    for(MUINT32 j=0;j<this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetDuqQ();j++){
                        for(MUINT32 i=0;i<this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetBurstQ();i++){
                            this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj(j)[i];
                            this->m_AFO.Init();
                        }
                    }


                    for(MUINT32 j=0;j<this->m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetDuqQ();j++){
                        for(MUINT32 i=0;i<this->m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetBurstQ();i++){
                            this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pTwinCQMgr[ISP_DRV_CQ_THRE0][*it]->CmdQMgr_GetCurCycleObj(j)[i];
                            this->m_AFO.write2CQ();
                            //this->m_AFO.config();remove, setting via cam_tuning_ctrl
                            this->m_AFO.enable(NULL);
                        }
                    }

                    this->m_AFO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj(0)[0];
                    this->m_AFO_FBC.config();
                    this->m_AFO_FBC.enable((void*)&subsample);

                    // update other cam's AFO BA under twin
                    {
                        #if 1
                            PIPE_ERR("this won't be work if start with single path,and then switch to twin during streaming\n");
                            PIPE_ERR("because of setbaseaddre will be failed when vf_en is ON\n");
                            PIPE_ERR("wait cq2 to replace cq5\n");
                        #endif
                        vector<MUINTPTR>::iterator it2;
                        vector<MUINTPTR>::iterator it3;
                        MUINT32 i;

                        for(i=0,it2 = this->m_ImgPA_L.begin(),it3 = this->m_FHPA_L.begin();\
                            it2!= this->m_ImgPA_L.end(),it3!=this->m_FHPA_L.end();it2++,it3++,i++){

                            this->m_AFO.m_pIspDrv = (IspDrvVir*)this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetCurCycleObj( \
                            this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_GetDuqQIdx())[0];

                            this->m_AFO.dma_cfg.memBuf.base_pAddr = *it2;
                            this->m_AFO.Header_Addr = *it3;

                            if(this->m_AFO.setBaseAddr()){
                                PIPE_ERR("[twin]set baseaddress at ring depth:0x%x at cam:0x%x\n",i,*it);
                                return MFALSE;
                            }
                        }
                    }

                    PIPE_ERR("######################\n");
                    PIPE_ERR("behavior of CAM_A trig CAM_B is not patched yet!\n");
                    PIPE_ERR("######################\n");
                }


                //
                this->m_TwinStatus = TwinMgr::E_INIT;
            }
            break;
        case TwinMgr::E_INIT:
            {
                vector<ISP_HW_MODULE>::iterator it;
                vector<ISP_HW_MODULE> l_module;
                if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                    return MFALSE;

                for(it = l_module.begin();it!=l_module.end();it++){
                    //resource check
                    if(*gSttioPipe[*it].m_ResMgr.Get_Res() == '\0'){
                        gSttioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
                        gSttioPipe[*it].m_ResMgr.m_hwModule = gSttioPipe[*it].m_hwModule = *it;
                    }
                    else{
                        PIPE_ERR("CAM_%d is occupied by %s,lunch fail\n",*it,gSttioPipe[*it].m_ResMgr.Get_Res());
                        return MFALSE;
                    }

                    PIPE_ERR("######################\n");
                    PIPE_ERR("behavior of CAM_A trig CAM_B is not patched yet!\n");
                    PIPE_ERR("######################\n");

                    //
                    if(this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_start()){
                        PIPE_ERR("Twin CQ5 start fail at cam:0x%x\n",*it);
                        return MFALSE;
                    }
                }

                //
                this->m_TwinStatus = TwinMgr::E_START;
            }
            break;
        case TwinMgr::E_START:
            //do nothing, already running in twin
            break;
        case TwinMgr::E_SUSPEND:
            {
                vector<ISP_HW_MODULE>::iterator it;
                vector<ISP_HW_MODULE> l_module;
                if(pTwin->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
                    return MFALSE;
                //
                for(it = l_module.begin();it!=l_module.end();it++){
                    //resource check
                    if(*gSttioPipe[*it].m_ResMgr.Get_Res() == '\0'){
                        gSttioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
                        gSttioPipe[*it].m_ResMgr.m_hwModule = gSttioPipe[*it].m_hwModule = *it;
                    }
                    else{
                        if(strcmp(gSttioPipe[*it].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) == 0){
                            //do nothing
                        }
                        else{
                            PIPE_ERR("CAM_%d is occupied by %s,lunch fail\n",*it,gSttioPipe[*it].m_ResMgr.Get_Res());
                            return MFALSE;
                        }
                    }

                    //
                    if(this->m_pTwinCQMgr[ISP_DRV_CQ_THRE5][*it]->CmdQMgr_resume()){
                        PIPE_ERR("af resume fail\n");
                        return MFALSE;
                    }
                }

                //
                this->m_TwinStatus = TwinMgr::E_START;
            }
            break;
        default:
            PIPE_ERR("unsupported status:%d\n",this->m_TwinStatus);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL STTIOPipe::TWIN_STT::SinglePath(STTIOPipe* _this)
{
    TwinMgr* pTwin = TwinMgr::createInstance(_this->m_hwModule);
    this->m_hwModule = _this->m_hwModule;

    switch(this->m_TwinStatus){
        case TwinMgr::E_NOTWIN:
            // donothing
            break;
        case TwinMgr::E_INIT:
            {
                MUINT32 bTwin = MFALSE;
                //
                for(MUINT32 i=0;i<CAM_MAX;i++){//can't use twin's getCurModule(), because of no twin
                    if(i != _this->m_hwModule){
                        //resource release
                        if(strcmp(gSttioPipe[i].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) == 0){
                            gSttioPipe[i].m_ResMgr.Release_Res();

                            gSttioPipe[i].m_ResMgr.m_hwModule = gSttioPipe[i].m_hwModule = CAM_MAX;

                            bTwin = MTRUE;
                        }
                        else{
                            //do nothing
                        }
                    }
                }

                if(bTwin){
                    PIPE_ERR("######################\n");
                    PIPE_ERR("diable behavior of CAM_A trig CAM_B !\n");
                    PIPE_ERR("######################\n");

                }
            }
            break;
        case TwinMgr::E_START:
            {
                if(_this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_suspend()){
                    PIPE_ERR("af suspsend fail\n");
                    return MFALSE;
                }
                //
                //can't free resouce here, need to wait untial hw is suspended.

                //
                this->m_TwinStatus = TwinMgr::E_SUSPEND;
            }
            break;
        case TwinMgr::E_SUSPEND:
            //
            for(MUINT32 i=0;i<CAM_MAX;i++){//can't use twin's getCurModule(), because of no twin
                if(i != _this->m_hwModule){
                    //resource release
                    if(strcmp(gSttioPipe[i].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) == 0){
                        gSttioPipe[i].m_ResMgr.Release_Res();

                        gSttioPipe[i].m_ResMgr.m_hwModule = gSttioPipe[i].m_hwModule = CAM_MAX;
                    }
                    else{
                        //do nothing,
                    }
                }
            }
            break;
        default:
            PIPE_ERR("unsupported status:%d\n",this->m_TwinStatus);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL STTIOPipe::TWIN_STT::clear(void)
{
    this->m_TwinStatus = TwinMgr::E_NOTWIN;
    memset((void*)m_pTwinCQMgr,0,sizeof(DupCmdQMgr*)*ISP_DRV_CAM_BASIC_CQ_NUM*CAM_MAX);

    return MTRUE;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
MBOOL STTIOPipe::ResMgr::Register_Res(char Name[32])
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied == MFALSE){
        if(*Name == '\0'){
            ret = MFALSE;
            PIPE_ERR("cant have NULL userName\n");
        }
        else{
            std::strncpy((char*)this->m_User,(char const*)Name, sizeof(this->m_User)-1);
            this->m_occupied = MTRUE;
        }
    }
    else{
        ret = MFALSE;
        PIPE_ERR("already occupied by user:%s\n",this->m_User);
    }


    return ret;
}

MBOOL STTIOPipe::ResMgr::Release_Res(void)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied){
        this->m_occupied = MFALSE;
        *this->m_User = '\0';
    }
    else{
        ret = MFALSE;
        PIPE_ERR("logic error: ,should be occupied\n");
    }
    return ret;
}

char* STTIOPipe::ResMgr::Get_Res(void)
{
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied){
        if(*this->m_User == '\0'){
            PIPE_ERR("logic error: ,username should not be NULL,(%s)\n",this->m_User);
        }
    }
    return this->m_User;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
