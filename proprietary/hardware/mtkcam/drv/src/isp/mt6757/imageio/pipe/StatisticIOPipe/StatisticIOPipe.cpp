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


//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
STTIOPipe::STTIOPipe()
{
    //
    DBG_LOG_CONFIG(imageio, pipe);


    m_szUsrName[0] = '\0';


    m_FSM = op_unknown;
    m_pipe_opt = ICamIOPipe::STTIO;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        m_pCmdQMgr[i] = NULL;

    m_byPassFLKO = MTRUE;
    m_byPassPDO = MTRUE;
    m_byPassAFO = MTRUE;
    m_byPassAAO = MTRUE;

    m_occupied = MFALSE;
    m_TwinMgr = NULL;
    m_hwModule = UNKNOWN_HW;

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
    m_occupied = MFALSE;
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
            if(gSttioPipe[CAM_A].m_occupied == MTRUE){
                BASE_LOG_ERR("TG_A is occupied by user:%s\n",gSttioPipe[CAM_A].m_szUsrName);
            }
            else{
                gSttioPipe[CAM_A].m_hwModule = CAM_A;
                gSttioPipe[CAM_A].m_occupied = MTRUE;
                std::strcpy((char*)gSttioPipe[CAM_A].m_szUsrName,(char const*)szUsrName);

                ptr = &gSttioPipe[CAM_A];
            }
            break;
        case TG_B:
            if(gSttioPipe[CAM_B].m_occupied == MTRUE){
                BASE_LOG_ERR("TG_B is occupied by user:%s\n",gSttioPipe[CAM_B].m_szUsrName);
            }
            else{
                gSttioPipe[CAM_B].m_hwModule = CAM_B;
                gSttioPipe[CAM_B].m_occupied = MTRUE;
                std::strcpy((char*)gSttioPipe[CAM_B].m_szUsrName,(char const*)szUsrName);

                ptr = &gSttioPipe[CAM_B];
            }
            break;
        default:
            BASE_LOG_ERR("unsupported input source\n",InPut);
            break;
    }

    return ptr;
}

void STTIOPipe::Destroy(void)
{
    PIPE_INF(":E:user:%s, with input source:0x%x\n",this->m_szUsrName,this->m_hwModule);

    this->m_occupied = MFALSE;


    //
    if(this->m_TwinMgr->getIsTwin() == MTRUE) {
        list<ISP_HW_MODULE>::iterator it;
        for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
            if(gSttioPipe[*it].m_occupied == MTRUE){
                gSttioPipe[*it].m_occupied = MFALSE;
            }
            else{
                PIPE_ERR("twin destroy fail,module(%d) is not occupied,user:%s\n", *it, \
                    gSttioPipe[*it].m_szUsrName);
            }
            gSttioPipe[*it].m_szUsrName[0] = '\0';
        }
    }
    //this->m_TwinMgr->destroyInstance();
    this->m_szUsrName[0] = '\0';

    this->FSM_UPDATE(op_unknown);
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        this->m_pCmdQMgr[i] = NULL;

    this->m_byPassFLKO = MTRUE;
    this->m_byPassPDO = MTRUE;
    this->m_byPassAFO = MTRUE;
    this->m_byPassAAO = MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::init()
{
    PIPE_INF("(%s):E",this->m_szUsrName);
    if(this->m_szUsrName[0] == '0'){
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
    PIPE_INF("(%s):E",this->m_szUsrName);

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
        if(this->m_TwinMgr->getIsTwin() == MTRUE) {
            list<ISP_HW_MODULE>::iterator it;
            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_detach()){
                    PIPE_ERR("Twin CQ5 uninit fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }
        }
    }

    if(this->m_byPassPDO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ7 uninit fail\n",this->m_hwModule);
            return MFALSE;
        }
#if PDO_SW_WORK_AROUND

        //
        if(this->m_TwinMgr->getIsTwin() == MTRUE) {
            list<ISP_HW_MODULE>::iterator it;
            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_detach()){
                    PIPE_ERR("Twin CQ7 uninit fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }
        }
#endif
    }

    if(this->m_byPassFLKO == MFALSE){
        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE8]->CmdQMgr_detach()){
            PIPE_ERR("[%d]:CQ8 uninit fail\n",this->m_hwModule);
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
        if(this->m_TwinMgr->getIsTwin() == MTRUE) {
            list<ISP_HW_MODULE>::iterator it;
            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_start()){
                    PIPE_ERR("Twin CQ5 start fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }
        }

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_start()){
            PIPE_ERR("[%d]:CQ5 start fail\n",this->m_hwModule);
            return MFALSE;
        }
    }

    if(this->m_byPassPDO == MFALSE){

#if PDO_SW_WORK_AROUND
        if(this->m_TwinMgr->getIsTwin() == MTRUE) {
            list<ISP_HW_MODULE>::iterator it;
            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_start()){
                    PIPE_ERR("Twin CQ7 start fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }
        }
#endif

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

    //no dmao enable. dmao enable is via tuning

    this->FSM_UPDATE(op_start);
    PIPE_DBG("-\n");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL STTIOPipe::stop(MBOOL bForce, MBOOL detachUni)
{
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
        if(MTRUE == this->m_TwinMgr->getIsTwin()) {
            this->m_TwinMgr->Twin_Lock();
        }

        if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_stop()){
            PIPE_ERR("[%d]:CQ5 stop fail\n",this->m_hwModule);
            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                this->m_TwinMgr->Twin_UnLock();
            }
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

        if(MTRUE == this->m_TwinMgr->getIsTwin()) {
            list<ISP_HW_MODULE>::iterator it = this->m_TwinMgr->getCurModule()->begin();
            MUINT32 curModule = CAM_MAX;
            for(MUINT32 i = 0 ; i < this->m_TwinMgr->getCurModule()->size() ; i++, it++){
                curModule = *it;
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_stop()){
                    PIPE_ERR("Twin CQ5 stop fail at cam:0x%x\n",*it);
                    this->m_TwinMgr->Twin_UnLock();
                    return MFALSE;
                }
            }

            if(curModule >= 0 && curModule < CAM_MAX){
                gSttioPipe[curModule].m_AFO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[curModule].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(0)[0];
                gSttioPipe[curModule].m_AFO_FBC.disable();

                for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                        gSttioPipe[curModule].m_AFO.m_pIspDrv = (IspDrvVir*)gSttioPipe[curModule].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                        gSttioPipe[curModule].m_AFO.disable();
                    }
                }

                this->m_TwinMgr->Twin_UnLock();
            }
            else{
                PIPE_ERR("current module index is out of range\n");
                this->m_TwinMgr->Twin_UnLock();
                return MFALSE;
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

#if PDO_SW_WORK_AROUND
        if(MTRUE == this->m_TwinMgr->getIsTwin()) {
            list<ISP_HW_MODULE>::iterator it;
            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_stop()){
                    PIPE_ERR("Twin CQ5 stop fail at cam:0x%x\n",*it);
                    return MFALSE;
                }
            }

            gSttioPipe[*it].m_PDO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(0)[0];
            gSttioPipe[*it].m_PDO_FBC.disable();

            for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
                for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                    gSttioPipe[*it].m_PDO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                    gSttioPipe[*it].m_PDO.disable();
                }
            }

        }
#endif
    }

    if(this->m_byPassFLKO == MFALSE){
        MUINTPTR pDmao,ptr;

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



    this->FSM_UPDATE(op_stop);
    PIPE_DBG("-\n");
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
                if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&ptr,&_pfbc) == MFALSE){
                    return -1;
                }
                pFBC = (CAM_BUF_CTRL*)_pfbc;
            }
            cq = ISP_DRV_CQ_THRE8;

            pFBC->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
            break;
        default:
            PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
            return -1;
            break;
    }

    if(this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ()!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("enque buf number is mismatched with subsample (0x%x_0x%x)\n",rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ());
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

        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, bufInfo.u_op.enque.image.mem_info.memID,\
            bufInfo.u_op.enque.image.mem_info.va_addr,\
            bufInfo.u_op.enque.image.mem_info.pa_addr,\
            bufInfo.u_op.enque.header.va_addr);



        //
        pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if(MFALSE == pFBC->enqueueHwBuf(bufInfo)){
            PIPE_ERR("enque fail at burst:0x%x\n",i);
            ret = -1;
        }

        // If twin, enque twinHwModule's afo
        switch(dmaChannel){
            case _afo_:
                if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                    list<ISP_HW_MODULE>::iterator it3;
                    for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                        if(this->m_FSM == op_start)
                            targetIdx = ((gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx() + 1)% gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQ());
                        else
                            targetIdx = gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx();

                        gSttioPipe[*it3].m_AFO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                        gSttioPipe[*it3].m_AFO_FBC.enqueueHwBuf(bufInfo);
                    }
                }
                break;
#if PDO_SW_WORK_AROUND
            case _pdo_:
                if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                    list<ISP_HW_MODULE>::iterator it3;
                    for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                        if(this->m_FSM == op_start)
                            targetIdx = ((gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx() + 1)% gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQ());
                        else
                            targetIdx = gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx();

                        gSttioPipe[*it3].m_PDO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                        gSttioPipe[*it3].m_PDO_FBC.enqueueHwBuf(bufInfo);
                    }
                }
                break;
#endif
            default:
                break;
        }

    }


    this->FSM_UPDATE(op_cmd);
    return  ret;
}


/*******************************************************************************
* return value:
* 1: already stopped
* 0: sucessed
*-1: fail
********************************************************************************/
MINT32 STTIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    MUINT32 dmaChannel = 0;
    CAM_BUF_CTRL* pFBC = NULL;
    BufInfo buf;
    MINT32 ret = 0, deqRet;
    E_ISP_CAM_CQ cq;

    //warning free
    u4TimeoutMs;
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
                if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&ptr,&_pfbc) == MFALSE){
                    return -1;
                }
                pFBC = (CAM_BUF_CTRL*)_pfbc;
            }
            cq = ISP_DRV_CQ_THRE8;

            pFBC->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
            break;
        default:
            PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
            return -1;
            break;
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();


     //cfg for waitbufready
    pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];


    switch(dmaChannel){
        case _afo_:
            //check if there is already filled buffer
            if ( MFALSE == this->m_AFO_FBC.waitBufReady(MTRUE, pNotify)) {
                if(this->m_FSM == op_stop) {
                    PIPE_DBG("warning: waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                    return 1;
                }
                PIPE_ERR("waitBufReady fail");
                return -1;
            }
            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                list<ISP_HW_MODULE>::iterator it3;

                for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                    gSttioPipe[*it3].m_AFO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];
                    if(gSttioPipe[*it3].m_AFO_FBC.waitBufReady(MFALSE, pNotify) ==  MFALSE){
                        if(this->m_FSM == op_stop) {
                            PIPE_DBG("warning: waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                            return 1;
                        }
                        PIPE_ERR("waitBufReady fail");
                        return -1;
                    }
                    if(this->m_FSM == op_stop) {
                        PIPE_INF("warning: waitBufRdy:VF_EN=0, dma(0x%x) after waitBufReady\n", dmaChannel);
                        return 1;
                    }
                }

            }
            break;
        case _pdo_:
            buf.m_pPrivate = (MUINT32*)&this->m_Xmx;
            //check if there is already filled buffer
            if ( MFALSE == this->m_PDO_FBC.waitBufReady(MTRUE, pNotify)) {
                if(this->m_FSM == op_stop) {
                    PIPE_DBG("warning: waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                    return 1;
                }
                PIPE_ERR("waitBufReady fail");
                return MFALSE;
            }
#if PDO_SW_WORK_AROUND
            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                list<ISP_HW_MODULE>::iterator it3;

                for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                    gSttioPipe[*it3].m_PDO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];
                    if(gSttioPipe[*it3].m_PDO_FBC.waitBufReady(MFALSE, pNotify) ==  MFALSE){
                        if(this->m_FSM == op_stop) {
                            PIPE_DBG("warning: waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                            return 1;
                        }
                        PIPE_ERR("waitBufReady fail");
                        return MFALSE;
                    }
                    if(this->m_FSM == op_stop) {
                        PIPE_INF("warning: waitBufRdy:VF_EN=0, dma(0x%x) after waitBufReady\n", dmaChannel);
                        return 1;
                    }
                }

            }
#endif
            break;
        default:
            //check if there is already filled buffer
            if ( MFALSE == pFBC->waitBufReady(pNotify)) {
                if(this->m_FSM == op_stop) {
                    PIPE_DBG("warning: waitBufRdy:VF_EN=0, dma(0x%x)\n", dmaChannel);
                    return 1;
                }
                PIPE_ERR("waitBufReady fail");
                return -1;
            }
            break;
    }

    for(MUINT32 i=0;i<this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ();i++){

        pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[i];
        if ( (deqRet = pFBC->dequeueHwBuf( buf )) < 0 ) {
            PIPE_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
            ret = -1;
        }
        else if( deqRet > 0){
            PIPE_DBG("warning: deq dma[0x%x]:VF_EN=0[%d]dma[0x%x]FSM[%d]\n",dmaChannel,deqRet,this->m_FSM);
            ret = 1;
        }
        else{
            if(this->m_FSM == op_stop) {
                PIPE_DBG("warning: deq dma[0x%x]:FSM=stop[%d]dma[0x%x]FSM[%d]\n",dmaChannel,deqRet,this->m_FSM);
                ret = 1;
            }
        }
        rQBufInfo.vBufInfo.push_back(buf);

        //
        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, rQBufInfo.vBufInfo[i].memID[0],\
            rQBufInfo.vBufInfo[i].u4BufVA[0],\
            rQBufInfo.vBufInfo[i].u4BufPA[0],\
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA);
    }

    // If twin, enque twinHwModule's afo
    switch(dmaChannel){
        case _afo_:
            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                list<ISP_HW_MODULE>::iterator it3;
                if(ret == 0){
                    for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){

                        for(MUINT32 i=0;i<gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ();i++){
                            gSttioPipe[*it3].m_AFO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[i];
                            if ( (deqRet = gSttioPipe[*it3].m_AFO_FBC.dequeueHwBuf( buf )) < 0 ) {
                                PIPE_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
                                ret = -1;
                            }
                            else if( deqRet > 0){
                                PIPE_DBG("warning: deq dma[0x%x]:VF_EN=0[%d]dma[0x%x]FSM[%d]\n",dmaChannel,deqRet,this->m_FSM);
                                ret = 1;
                            }
                            else{
                                if(this->m_FSM == op_stop) {
                                    PIPE_DBG("warning: deq dma[0x%x]:FSM=stop[%d]dma[0x%x]FSM[%d]\n",dmaChannel,deqRet,this->m_FSM);
                                    ret = 1;
                                }
                            }
                        }
                    }
                }
            }
            break;
#if PDO_SW_WORK_AROUND
        case _pdo_:
            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                list<ISP_HW_MODULE>::iterator it3;
                if(ret != MFALSE){
                    for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){

                        for(MUINT32 i=0;i<gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ();i++){
                            gSttioPipe[*it3].m_PDO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj(gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[i];
                            if ( MFALSE == gSttioPipe[*it3].m_PDO_FBC.dequeueHwBuf( buf ) ) {
                                PIPE_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
                                ret = MFALSE;
                            }
                        }
                    }
                }
            }
            break;
#endif
        default:
            break;
    }

    /* prevent change FSM after stop */
    if (ret > 0)
        return ret;

    if(rQBufInfo.vBufInfo.size() != this->m_pCmdQMgr[cq]->CmdQMgr_GetBurstQ()){
        PIPE_ERR("dequeout data length is mismatch");
        ret = -1;
    }

    //
    //PIPE_DBG("X ");
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
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 subsample = func.Bits.SUBSAMPLE;
    vInPorts;

    //
    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    if (0 == vOutPorts.size()) {
        PIPE_ERR("outport:x%x size err\n", vOutPorts.size());
        return MFALSE;
    }

    //output port
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) {
            PIPE_INF("dummy output vector at:0x%x\n",i);
            continue;
        }

        PIPE_DBG("statistic vOutPorts:[%d]:\n", i);

        //
        switch(vOutPorts[i]->index){
            case EPortIndex_AAO:
                idx_aao = i;
                cq = ISP_DRV_CQ_THRE4;
                this->m_byPassAAO = MFALSE;
                break;
            case EPortIndex_AFO:
                idx_afo = i;
                cq = ISP_DRV_CQ_THRE5;
                this->m_byPassAFO = MFALSE;
                break;
            case EPortIndex_FLKO:
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
                this->m_byPassPDO = MFALSE;
                break;
            default:
                PIPE_ERR("unspuported port:0x%x\n",vOutPorts[i]->index);
                return MFALSE;
                break;
        }

        //cfg cmdQ
        if(cq >= 0 && cq < ISP_DRV_CAM_BASIC_CQ_NUM){
            if((this->m_pCmdQMgr[cq] = DupCmdQMgr::CmdQMgr_attach(0,subsample,this->m_hwModule,cq)) == NULL){
                PIPE_ERR("CQ init fail:0x%x\n",cq);
                return MFALSE;
            }
        }
    }

    /// Twin: Only need to take care AFO
    // get twin status from TwinMgr
    this->m_TwinMgr = TwinMgr::createInstance(this->m_hwModule);

    if(MTRUE == this->m_TwinMgr->getIsTwin()) {
        list<ISP_HW_MODULE>::iterator it;

        //
        for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
            if(gSttioPipe[*it].m_occupied != MTRUE)
            {
                gSttioPipe[*it].m_hwModule = *it;
                gSttioPipe[*it].m_occupied = MTRUE;
                sprintf((char*)gSttioPipe[*it].m_szUsrName,"TwinMgr:%d",*it);

                PIPE_INF("Twin enable!! Occupy CamIoPipe: module(%d) by %s.", *it,gSttioPipe[*it].m_szUsrName);
            }
            else{
                PIPE_ERR("twin init fail,module(%d) is occupied by user:%s\n", *it, \
                    gSttioPipe[*it].m_szUsrName);
                return MFALSE;
            }

            // If AFO is enable, attach CmdQ thread 5 for twin use
            if(MFALSE == this->m_byPassAFO) {
                cq = ISP_DRV_CQ_THRE5;
                //cfg cmdQ
                if((gSttioPipe[*it].m_pCmdQMgr[cq] = DupCmdQMgr::CmdQMgr_attach(0,subsample,*it,cq)) == NULL){
                    PIPE_ERR("Twin CQ_%d init fail\n",cq);
                    return MFALSE;
                }
            }
#if PDO_SW_WORK_AROUND
            if(MFALSE == this->m_byPassPDO){
                cq = ISP_DRV_CQ_THRE7;
                //cfg cmdQ
                if((gSttioPipe[*it].m_pCmdQMgr[cq] = DupCmdQMgr::CmdQMgr_attach(0,subsample,*it,cq)) == NULL){
                    PIPE_ERR("Twin CQ_%d init fail\n",cq);
                    return MFALSE;
                }
            }
#endif
        }
    }


    this->m_pCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_GetCurMgrObj(this->m_hwModule,ISP_DRV_CQ_THRE0);
    if(this->m_pCmdQMgr[ISP_DRV_CQ_THRE0] == NULL){
        PIPE_ERR("camiopipe configure fail or not configure yet!\n");
        return MFALSE;
    }

    //for dual pd to set PMX
    for(MUINT32 j=0;j<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
        for(MUINT32 i=0;i<this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
            this->m_PmxCtrl.m_pIspDrv =(IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
            this->m_PmxCtrl.config();
        }
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

        // afo for twin
        if(MTRUE == this->m_TwinMgr->getIsTwin()) {
            list<ISP_HW_MODULE>::iterator it;

            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                for(MUINT32 j=0;j<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetBurstQ();i++){
                        gSttioPipe[*it].m_AFO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(j)[i];
                        gSttioPipe[*it].m_AFO.Init();
                    }
                }

                gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_GetCurMgrObj(*it,ISP_DRV_CQ_THRE0);
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0] == NULL){
                    PIPE_ERR("camiopipe configure fail or not configure yet at cam:0x%x!\n",*it);
                    return MFALSE;
                }

                for(MUINT32 j=0;j<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                        gSttioPipe[*it].m_AFO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                        gSttioPipe[*it].m_AFO.write2CQ();
                        //this->m_AFO.config();remove, setting via cam_tuning_ctrl
                        gSttioPipe[*it].m_AFO.enable(NULL);
                    }
                }

                gSttioPipe[*it].m_AFO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(0)[0];
                gSttioPipe[*it].m_AFO_FBC.config();
                gSttioPipe[*it].m_AFO_FBC.enable((void*)&subsample);
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
            }
        }
        this->m_PDO_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PDO_FBC.config();
        this->m_PDO_FBC.enable((void*)&subsample);
#if PDO_SW_WORK_AROUND
        // pdo twin
        if(MTRUE == this->m_TwinMgr->getIsTwin()) {
            list<ISP_HW_MODULE>::iterator it;

            for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
                for(MUINT32 j=0;j<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetBurstQ();i++){
                        gSttioPipe[*it].m_PDO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(j)[i];
                        gSttioPipe[*it].m_PDO.Init();
                    }
                }

                gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_GetCurMgrObj(*it,ISP_DRV_CQ_THRE0);
                if(gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0] == NULL){
                    PIPE_ERR("camiopipe configure fail or not configure yet at cam:0x%x!\n",*it);
                    return MFALSE;
                }

                for(MUINT32 j=0;j<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();j++){
                    for(MUINT32 i=0;i<gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();i++){
                        gSttioPipe[*it].m_PDO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(j)[i];
                        gSttioPipe[*it].m_PDO.write2CQ();
                        //this->m_PDO.config();remove, setting via cam_tuning_ctrl
                        gSttioPipe[*it].m_PDO.enable(NULL);
                    }
                }

                gSttioPipe[*it].m_PDO_FBC.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it].m_pCmdQMgr[ISP_DRV_CQ_THRE7]->CmdQMgr_GetCurCycleObj(0)[0];
                gSttioPipe[*it].m_PDO_FBC.config();
                gSttioPipe[*it].m_PDO_FBC.enable((void*)&subsample);
            }
        }
#endif
    }
    //flko
    if(this->m_byPassFLKO == MFALSE){
        MUINTPTR pDmao,ptr;


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

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

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
                list<MUINTPTR>* pImgPA_L;
                list<MUINTPTR>* pFHPA_L;
                list<MUINTPTR>::iterator it;
                list<MUINTPTR>::iterator it2;
                list<ISP_HW_MODULE>::iterator it3;
                E_ISP_CAM_CQ cq;
                DMAO_B *pDmao = NULL;
                MUINT32 i;
                char str[256];
                char tmp[32];
                str[0] = '\0';
                if(this->m_FSM != op_cfg){
                    PIPE_ERR("this cmd:0x%x is only avail at cfg stage, cur stage:0x%x\n",cmd,this->m_FSM);
                    return MFALSE;
                }

                dmaChannel = (MUINT32)this->PortID_MAP(arg1);

                pImgPA_L = (list<MUINTPTR>*)arg2;
                pFHPA_L = (list<MUINTPTR>*)arg3;
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

                        if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&_pdma,&ptr) == MFALSE){
                            return MFALSE;
                        }
                        pDmao = (DMAO_B *)_pdma;

                        cq = ISP_DRV_CQ_THRE8;

                        pDmao->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
                        break;
                    default:
                        PIPE_ERR("unsupported dmao:0x%x\n",dmaChannel);
                        return MFALSE;
                        break;
                }

                for(i=0,it = pImgPA_L->begin(),it2 = pFHPA_L->begin();it != pImgPA_L->end() ;it++,it2++, i++){
                    switch(dmaChannel){
                        case _afo_:
                            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                                for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                                    // update other cam's AFO BA under twin
                                    gSttioPipe[*it3].m_AFO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                                    gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];

                                    gSttioPipe[*it3].m_AFO.dma_cfg.memBuf.base_pAddr = *it;
                                    gSttioPipe[*it3].m_AFO.Header_Addr = *it2;

                                    if(gSttioPipe[*it3].m_AFO.setBaseAddr()){
                                        PIPE_ERR("[twin]set baseaddress at ring depth:0x%x at cam:0x%x\n",i,*it3);
                                        ret = MFALSE;
                                    }

                                    sprintf(tmp,"[twin_%d]0x%x_0x%x,",*it3,*it,*it2);
                                    strcat(str,tmp);
                                }
                            }
                            break;
#if PDO_SW_WORK_AROUND
                        case _pdo_:
                            if(MTRUE == this->m_TwinMgr->getIsTwin()) {
                                for(it3 = this->m_TwinMgr->getCurModule()->begin();it3!=this->m_TwinMgr->getCurModule()->end();it3++){
                                    // update other cam's AFO BA under twin

                                    gSttioPipe[*it3].m_PDO.m_pIspDrv = (IspDrvVir*)gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetCurCycleObj( \
                                    gSttioPipe[*it3].m_pCmdQMgr[cq]->CmdQMgr_GetDuqQIdx())[0];

                                    gSttioPipe[*it3].m_PDO.dma_cfg.memBuf.base_pAddr = *it;
                                    gSttioPipe[*it3].m_PDO.Header_Addr = *it2;

                                    if(gSttioPipe[*it3].m_PDO.setBaseAddr()){
                                        PIPE_ERR("[twin]set baseaddress at ring depth:0x%x at cam:0x%x\n",i,*it3);
                                        ret = MFALSE;
                                    }

                                    sprintf(tmp,"[twin_%d]0x%x_0x%x,",*it3,*it,*it2);
                                    strcat(str,tmp);
                                }
                            }
                            break;
#endif
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
                    }

                    sprintf(tmp,"0x%x_0x%x,",*it,*it2);
                    strcat(str,tmp);
                }

                PIPE_INF("statistic vOutPorts:dma:0x%x, %s\n",dmaChannel,str);
                #undef RING_DEPTH
            }
            break;
        case EPIPECmd_GET_AFO_CONS:
            *(MUINT32*)arg1 = (AFO_DUMMY_WIN + AFO_MAX_SIZE);
            break;
        case EPIPECmd_GET_AAO_CONS:
            *(MUINT32*)arg1 = AAO_MAX_SIZE;
            break;
        case EPIPECmd_GET_FLKO_CONS:
            *(MUINT32*)arg1 = FLKO_MAX_SIZE(arg3);
            break;

        case EPIPECmd_GET_AFO_STRIDE_INFO:
            *(MUINT32*)arg1 = AFO_STRIDE;
            *(MUINT32*)arg2 = AFO_DUMMY_STRIDE;
            break;

        case EPIPECmd_GET_PDO_CONS:
            {
                MUINT32 Width,Height;
                Width = (MUINT32)arg2;
                Height = (MUINT32)arg3;
                *(MUINT32*)arg1 = PDO_MAX_SIZE(Width,Height);
                PIPE_DBG("PDO size_%d (%d x %d)",*(MUINT32*)arg1, Width,Height);
            }
            break;
        case EPIPECmd_GET_PDO_STRIDE_INFO:
#if PDO_SW_WORK_AROUND
            *(MUINT32*)arg2 = PDO_STRIDE_CONS((*(MUINT32*)arg1));
#else
            *(MUINT32*)arg2 = 0;
#endif
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
                if(this->m_UniMgr.UniMgr_GetModuleObj(UniMgr::_UNI_FLK_,&_pdma,&ptr) == MFALSE){
                    ret = -1;
                    break;
                }
                pDmao = (DMAO_B *)_pdma;
                pDmao->m_pUniDrv = this->m_UniMgr.UniMgr_GetCurObj();
                *(MUINTPTR*)arg2 = UNI_READ_REG(pDmao->m_pUniDrv,CAM_UNI_FLKO_BASE_ADDR);
                break;
            default:
                PIPE_ERR("not support dma(0x%x)\n", _portidx);
                ret = -1;
                goto EXIT;
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


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio


