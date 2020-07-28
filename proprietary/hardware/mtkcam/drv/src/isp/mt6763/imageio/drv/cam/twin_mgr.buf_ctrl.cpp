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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "TwinMgr_buf"

#include "twin_mgr_imp.h"
#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include "cam_path_cam.h"
#include "cam_capibility.h"


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;


EXTERN_DBG_LOG_VARIABLE(TwinMgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#define LOG_VRB(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[%s]" fmt,__FUNCTION__, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[%s]" fmt,__FUNCTION__, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF("[%s]" fmt,__FUNCTION__, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[%s]" fmt,__FUNCTION__, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("error:[%s]" fmt,__FUNCTION__, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (TwinMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt,__FUNCTION__, ##arg); } } while(0)

#ifdef E_FSM
#undef E_FSM
#endif

typedef enum _E_FSM{
    op_unknown  = 0,
    op_cfg      = 1,
    op_startTwin,
    op_runtwin,
}E_FSM;

// FSM check
MBOOL TwinMgr_BufCtrl::FSM_UPDATE(MUINT32 op)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mFSMLock);

    switch(op){
        case op_unknown:
            if(this->m_FSM != op_startTwin)
                ret = MFALSE;
            else
                this->m_FSM = op;
            break;
        case op_cfg:
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
            else
                this->m_FSM = op;
            break;
        case op_startTwin:
            if(this->m_FSM != op_cfg)
                ret = MFALSE;
            else
                this->m_FSM = op;
            break;
        case op_runtwin:
            if(this->m_FSM == op_unknown)
                ret = MFALSE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE)
        LOG_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);

    return ret;
}


TwinMgr_BufCtrl::TwinMgr_BufCtrl()
{
    m_bEnque = MFALSE;
    m_pMainCmdQMgr = NULL;
    m_pTwinCmdQMgr = NULL;
    m_pTwinCtrl = NULL;

    m_FSM = op_unknown;
}

TwinMgr_BufCtrl::~TwinMgr_BufCtrl()
{

}

MBOOL TwinMgr_BufCtrl::TBC_config(CAM_TWIN_PIPE* pTwinctrl,DupCmdQMgr* pMain,DupCmdQMgr* pTwin,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput)
{
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL* pFBC = NULL;
    DMAO_B* pDmao = NULL;
    DupCmdQMgr* pCmdQ = NULL;

    if(this->FSM_UPDATE(op_cfg) == MFALSE)
        return MFALSE;

    this->m_pTwinCtrl = pTwinctrl;
    this->m_pMainCmdQMgr = pMain;
    this->m_pTwinCmdQMgr = pTwin;
    this->m_OpenedChannel.clear();
    this->m_enque_IMGO.init();
    this->m_enque_RRZO.init();

    while(1){
        if(pCmdQ == NULL){
            pCmdQ = this->m_pMainCmdQMgr;
        }
        else if(pCmdQ == this->m_pMainCmdQMgr){
            pCmdQ = this->m_pTwinCmdQMgr;
        }
        else if(pCmdQ == this->m_pTwinCmdQMgr){
            break;
        }
        else{
            LOG_ERR("logic error: \n");
            return MFALSE;
        }

        for(it=pchannel->begin();it!=pchannel->end();it++){
            switch(*it){
                case _imgo_:
                    pFBC = &this->m_Imgo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Imgo[pCmdQ->CmdQMgr_GetCurModule()];
                    break;
                case _rrzo_:
                    pFBC = &this->m_Rrzo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Rrzo[pCmdQ->CmdQMgr_GetCurModule()];
                    break;
                default:
                    LOG_ERR("unsupported dmao:%d\n",*it);
                    return MFALSE;
                    break;
            }
            //under twin case, master cam & slave cam need to share the same timestamp obj.
            pFBC->m_pTimeStamp = CAM_TIMESTAMP::getInstance(this->m_pMainCmdQMgr->CmdQMgr_GetCurModule(),NULL);
            pFBC->m_pTimeStamp->TimeStamp_SrcClk(pInput->tTimeClk);
#if TEMP_SW_TIMESTAMP
            pFBC->m_TimeStamp.TimeStamp_SrcClk(pInput->tTimeClk);
#endif
            for(MUINT32 i = 0;i<pCmdQ->CmdQMgr_GetDuqQ();i++){
                for(MUINT32 j=0;j<pCmdQ->CmdQMgr_GetBurstQ();j++){
                    //fbc
                    pFBC->m_pIspDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];
                    pFBC->config();
                    pFBC->write2CQ();
                    pFBC->enable(NULL);

                    //dma
                    pDmao->m_pIspDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];
                    //twin's dmao cfg is cloned via twin_drv. can't config here due to have no dmao's size information.
                    //pDmao->config();
                    pDmao->write2CQ();
                    pDmao->enable(NULL);
                }
            }

            //do need to push twice at master&slave cam
            if(pCmdQ == this->m_pTwinCmdQMgr)
                this->m_OpenedChannel.push_back(*it);
        }
    }
    return MTRUE;
}


MBOOL TwinMgr_BufCtrl::TBC_Start(MVOID)
{
    return this->FSM_UPDATE(op_startTwin);
}

MBOOL TwinMgr_BufCtrl::TBC_Stop(MVOID)
{
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL* pFBC = NULL;
    DMAO_B* pDmao = NULL;

    if(this->FSM_UPDATE(op_unknown) == MFALSE)
        return MFALSE;

    //main path's stop is cfg at cam_path_p1
    for(it=this->m_OpenedChannel.begin();it!=this->m_OpenedChannel.end();it++){

        switch(*it){
            case _imgo_:
                pFBC = &this->m_Imgo_FBC[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Imgo[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                break;
            case _rrzo_:
                pFBC = &this->m_Rrzo_FBC[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Rrzo[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                break;
            default:
                LOG_ERR("unsupported dmao:%d\n",*it);
                return MFALSE;
                break;
        }

        for(MUINT32 i = 0;i<this->m_pTwinCmdQMgr->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<this->m_pTwinCmdQMgr->CmdQMgr_GetBurstQ();j++){
                //
                pFBC->m_pIspDrv = (IspDrvVir*)this->m_pTwinCmdQMgr->CmdQMgr_GetCurCycleObj(i)[j];
                pFBC->disable();

                //
                pDmao->m_pIspDrv = (IspDrvVir*)this->m_pTwinCmdQMgr->CmdQMgr_GetCurCycleObj(i)[j];
                pDmao->disable();
            }
        }
    }

    return MTRUE;
}



MBOOL TwinMgr_BufCtrl::enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo)
{
    MBOOL ret = MTRUE;
    QueueMgr<ISP_BUF_INFO_L>* pQue = NULL;
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL* pFBC = NULL;
    DMAO_B* pDmao = NULL;

    if(this->FSM_UPDATE(op_runtwin) == MFALSE)
        return MFALSE;

    switch(dmaChannel){
        case _imgo_:
            pQue = &this->m_enque_IMGO;
            break;
        case _rrzo_:
            pQue = &this->m_enque_RRZO;
            break;
        default:
            LOG_ERR("unsupported dmao:%d\n",dmaChannel);
            ret = MFALSE;
            goto EXIT;
            break;
    }

    pQue->sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_push,(MUINTPTR)&bufInfo,0);
    //behavior of enque into hw will be invoked after runtwin


    //always open dmao/fbc for twin_drv's input info.
    for(it=this->m_OpenedChannel.begin();it!=this->m_OpenedChannel.end();it++){
        switch(*it){
            case _imgo_:
                pFBC = &this->m_Imgo_FBC[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Imgo[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                break;
            case _rrzo_:
                pFBC = &this->m_Rrzo_FBC[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Rrzo[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()];
                break;
            default:
                LOG_ERR("unsupported dmao:%d\n",*it);
                return MFALSE;
                break;
        }

        for(MUINT32 i = 0;i<this->m_pTwinCmdQMgr->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<this->m_pTwinCmdQMgr->CmdQMgr_GetBurstQ();j++){
                //fbc
                pFBC->m_pIspDrv = (IspDrvVir*)this->m_pTwinCmdQMgr->CmdQMgr_GetCurCycleObj(i)[j];
                pFBC->enable(NULL);

                //dma
                pDmao->m_pIspDrv = (IspDrvVir*)this->m_pTwinCmdQMgr->CmdQMgr_GetCurCycleObj(i)[j];
                pDmao->enable(NULL);
            }
        }
    }
EXIT:
    return ret;
}

/* return value:
 1: already stopped
 0: sucessed
-1: fail */
MINT32 TwinMgr_BufCtrl::deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify)
{
    MINT32 ret = 0;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA> channel;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA>::iterator it_frame;
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;

    CAM_BUF_CTRL* pFBC[CAM_MAX];
    NSImageio::NSIspio::BufInfo buf;
    DupCmdQMgr* pCQMgr = NULL;
    MBOOL bWaitFlg[CAM_MAX];
    MUINT32 FrameCnt=0;

    memset(bWaitFlg,MFALSE,sizeof(MBOOL)*CAM_MAX);

    if(this->FSM_UPDATE(op_runtwin) == MFALSE)
        return 1;

    if(bufInfo.size() != 0){
        LOG_ERR("list is not empty\n");
        return -1;
    }

    if(this->m_pTwinCtrl->get_RunTwinRst(dmaChannel,channel,MTRUE) ){
        LOG_ERR("twin's dma ctrl result error\n");
        return -1;
    }


    //
    for(it_frame=channel.begin();it_frame!=channel.end();it_frame++){//for subsample
        for(it = it_frame->begin();it != it_frame->end();it++){
            if(it->bFrameEnd == MFALSE)
                bWaitFlg[it->module] = MTRUE;
        }
    }


    //
    switch(dmaChannel){
        case _imgo_:
            for(MUINT32 i=0;i<CAM_MAX;i++){
                pFBC[i] = &this->m_Imgo_FBC[i];
                pFBC[i]->m_pUniDrv = NULL;//avoid pipe check NE if previous scenario have uni ptr and current scenario have deque fail
            }
            break;
        case _rrzo_:
            for(MUINT32 i=0;i<CAM_MAX;i++){
                pFBC[i] = &this->m_Rrzo_FBC[i];
                pFBC[i]->m_pUniDrv = NULL;//avoid pipe check NE if previous scenario have uni ptr and current scenario have deque fail
            }
            break;
        default:
            LOG_ERR("unsupported dmao:%d\n",it->channel);
            ret = -1;
            goto EXIT;
            break;
    }

    //wait twice is because of twin_mgr have no information of which signal being used at ifunc_buf_ctrl
    if(bWaitFlg[this->m_pMainCmdQMgr->CmdQMgr_GetCurModule()]){
        switch(pFBC[this->m_pMainCmdQMgr->CmdQMgr_GetCurModule()]->waitBufReady(pNotify)){
            case eCmd_Fail:
                return -1;
                break;
            case eCmd_Stop_Pass:
                LOG_WRN("waitBufRdy: VF_EN=0, dma(0x%x)\n", dmaChannel);
                return 1;
                break;
            case eCmd_Suspending_Pass:
                //need to do deque , in order to pop all those requests in drv.
                break;
            default:
                break;
        }
    }
    if(bWaitFlg[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()]){
        switch(pFBC[this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule()]->waitBufReady(pNotify)){
            case eCmd_Fail:
                return -1;
                break;
            case eCmd_Stop_Pass:
                LOG_WRN("waitBufRdy: VF_EN=0, dma(0x%x)\n", dmaChannel);
                return 1;
                break;
            case eCmd_Suspending_Pass:
                //need to do deque , in order to pop all those requests in drv.
                break;
            default:
                break;
        }
    }

    //
    this->m_lock.lock();
    for(it_frame=channel.begin();it_frame!=channel.end();it_frame++){//for subsample
        for(it = it_frame->begin();it != it_frame->end();it++){//for multi-cam
            if(it->bFrameEnd == MFALSE){
                capibility CamInfo;
                if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                    if(it->module == CAM_A){
                        pCQMgr = this->m_pMainCmdQMgr;
                    }
                    else if(it->module == CAM_B){
                        pCQMgr = this->m_pTwinCmdQMgr;
                    }
                    else{
                        LOG_ERR("unsupported module:%d\n",it->module);
                        ret = 1;
                        goto EXIT;
                    }
                }
                else{
                    switch (it->module) {
                    case CAM_A:
                    case CAM_B:
                        pCQMgr = this->m_pMainCmdQMgr;
                        break;
                    case CAM_A_TWIN:
                    case CAM_B_TWIN:
                        pCQMgr = this->m_pTwinCmdQMgr;
                        break;
                    default:
                        LOG_ERR("unsupported module:%d\n",it->module);
                        ret = 1;
                        goto EXIT;
                    }
                }
                //can't update this para. , in order to prevent racing condition
                //also , this m_pispdrv is useless under waitbufready & dequeuehwbuf
                //pFBC[pCQMgr->CmdQMgr_GetCurModule()].m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[0];
                switch(pFBC[pCQMgr->CmdQMgr_GetCurModule()]->dequeueHwBuf( buf )){
                    case eCmd_Fail:
                        LOG_ERR("data is rdy on dram,but deque fail at burst:0x%x,cam:%d\n",FrameCnt,pCQMgr->CmdQMgr_GetCurModule());
                        ret = -1;
                        break;
                    case eCmd_Stop_Pass:
                        LOG_WRN("current status:eCmd_Stop_Pass\n");
                        ret = 1;
                        break;
                    case eCmd_Suspending_Pass:
                        LOG_WRN("current status:eCmd_Suspending_Pass\n");
                        ret = 2;
                        break;
                    default:
                        break;
                }

            }
            else{
                bufInfo.push_back(buf);
                FrameCnt++;
            }
        }
    }

EXIT:
    this->m_lock.unlock();

    if(ret == 0)
        this->m_pTwinCtrl->clr_RunTwinRst(dmaChannel);
    return ret;
}

MINT32 TwinMgr_BufCtrl::enque_pop(MVOID)
{
    MINT32 ret = 0;
    MUINT32 _size = 0;
    list<MUINT32>::iterator it;

    for(it=this->m_OpenedChannel.begin();it!=this->m_OpenedChannel.end();it++){
        switch(*it){
            case _imgo_:
                this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_imgo_);
                else{
                    ret = 1;
                    LOG_ERR("IMGO have no buffer for enque\n");
                }
                break;
            case _rrzo_:
                this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_rrzo_);
                else{
                    ret = 1;
                    LOG_ERR("RRZO have no buffer for enque\n");
                }
                break;
            default:
                LOG_ERR("unsupported DMA:%d\n",*it);
                break;
        }
    }

    return ret;
}

MINT32 TwinMgr_BufCtrl::enqueHW(MUINT32 const dmaChannel)
{
    MINT32 ret = 0;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA> channel;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA>::iterator it_frame;
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;


    MUINT32 targetIdx = 0;
    CAM_BUF_CTRL* pFBC = NULL;
    DMAO_B* pDmao = NULL;
    DupCmdQMgr* pCQMgr = NULL;
    ISP_BUF_INFO_L _buf_list;
    MUINT32 FrameCnt=0;

    this->m_lock.lock();


    if(this->FSM_UPDATE(op_runtwin) == MFALSE){
        ret = 1;
        goto EXIT;
    }


    if(this->m_pTwinCtrl->get_RunTwinRst(dmaChannel,channel) ){
        LOG_ERR("twin's dma ctrl result error\n");
        ret = 1;
        goto EXIT;
    }

    //
    switch(dmaChannel){
        case _imgo_:
            this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front  ,(MUINTPTR)&_buf_list,0);
            break;
        case _rrzo_:
            this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front  ,(MUINTPTR)&_buf_list,0);
            break;
        default:
            LOG_ERR("unsupported dmao:%d\n",dmaChannel);
            ret = 1;
            goto EXIT;
            break;
    }

    for(it_frame=channel.begin();it_frame!=channel.end();it_frame++){//for subsample
        for(it = it_frame->begin();it != it_frame->end();it++){

            //
            if(it->bFrameEnd == MFALSE){
                capibility CamInfo;
                stISP_BUF_INFO _tmp_buf = _buf_list.front();
                if(CamInfo.m_DTwin.GetDTwin() == MFALSE){
                    //
                    if(it->module == CAM_A){
                        pCQMgr = this->m_pMainCmdQMgr;
                    }
                    else if(it->module == CAM_B){
                        pCQMgr = this->m_pTwinCmdQMgr;
                    }
                    else{
                        LOG_ERR("unsupported module:%d\n",it->module);
                        ret = 1;
                        goto EXIT;
                    }
                }
                else{
                    switch (it->module) {
                    case CAM_A:
                    case CAM_B:
                        pCQMgr = this->m_pMainCmdQMgr;
                        break;
                    case CAM_A_TWIN:
                    case CAM_B_TWIN:
                        pCQMgr = this->m_pTwinCmdQMgr;
                        break;
                    default:
                        LOG_ERR("unsupported module:%d\n",it->module);
                        ret = 1;
                        goto EXIT;
                    }
                }

                switch(dmaChannel){
                    case _imgo_:
                        pFBC = &this->m_Imgo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                        pDmao = &this->m_Imgo[pCQMgr->CmdQMgr_GetCurModule()];
                        break;
                    case _rrzo_:
                        pFBC = &this->m_Rrzo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                        pDmao = &this->m_Rrzo[pCQMgr->CmdQMgr_GetCurModule()];
                        break;
                    default:
                        LOG_ERR("unsupported dmao:%d\n",it->channel);
                        ret = 1;
                        goto EXIT;
                        break;
                }


                //
                if(this->m_FSM == op_startTwin)
                    targetIdx = ((pCQMgr->CmdQMgr_GetDuqQIdx() + 1)% pCQMgr->CmdQMgr_GetDuqQ());
                else
                    targetIdx = pCQMgr->CmdQMgr_GetDuqQIdx();

                //
                pFBC->m_pIspDrv = (IspDrvVir*)pCQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[FrameCnt];



                //patch header's address which is for separate DMAO
                this->HeaderUpdate(_tmp_buf,it->module);

                //
                if(eCmd_Fail == pFBC->enqueueHwBuf(_tmp_buf,MTRUE)){
                    LOG_ERR("enque fail at burst:0x%x\n",FrameCnt);
                    ret = 1;
                }

                //patch header's address by dmao's offset address before enque/dmao cfg
                this->HEADER_ADDR_HW_LIMITATION(_tmp_buf,it->offsetAdr);

                if(_tmp_buf.bReplace == MTRUE){
                    pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.Replace.image.mem_info.pa_addr;

                    pDmao->Header_Addr = _tmp_buf.Replace.header.pa_addr;
                }
                else{
                    pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.u_op.enque.image.mem_info.pa_addr;

                    pDmao->Header_Addr = _tmp_buf.u_op.enque.header.pa_addr;
                }
                pDmao->dma_cfg.memBuf.ofst_addr = it->offsetAdr;//update offsetadr updated by twin

                pDmao->m_pIspDrv = (IspDrvVir*)pCQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[FrameCnt];
                if(pDmao->setBaseAddr()){
                    LOG_ERR("set baseaddress at burst:0x%x\n",FrameCnt);
                    ret = 1;
                }
            }
            else{//frame end
                _buf_list.pop_front();

                FrameCnt++;
            }

        }

    }

    if (pCQMgr) {
        if(FrameCnt != pCQMgr->CmdQMgr_GetBurstQ()){
            LOG_ERR("enque number != burst number .(%d_%d)\n",FrameCnt,pCQMgr->CmdQMgr_GetBurstQ());
            ret = 1;
        }
    }

    //
    switch(dmaChannel){
        case _imgo_:
            this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop  ,0,0);
            break;
        case _rrzo_:
            this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop  ,0,0);
            break;
        default:
            LOG_ERR("unsupported dmao:%d\n",dmaChannel);
            ret = 1;
            break;
    }

EXIT:
    this->m_lock.unlock();

    return ret;
}

MBOOL TwinMgr_BufCtrl::HeaderUpdate(stISP_BUF_INFO& buf,ISP_HW_MODULE curModule)
{
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;
    capibility CamInfo;
    tCAM_rst rst;

    if(CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                rst,E_CAM_HEADER_size) == MFALSE){
        return MFALSE;
    }

    if(buf.bReplace == MTRUE){
        if(buf.Replace.header.size == 0){
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }

        //if module is belong to twinmodule, offset header's va/pa
        if(this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule() == curModule){
            buf.Replace.header.pa_addr += rst.HeaderSize;
            buf.Replace.header.va_addr += rst.HeaderSize;
        }

    }
    else{

        if(buf.u_op.enque.header.size == 0){
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }

        //if module is belong to twinmodule, offset header's va/pa
        if(this->m_pTwinCmdQMgr->CmdQMgr_GetCurModule() == curModule){
            buf.u_op.enque.header.pa_addr += rst.HeaderSize;
            buf.u_op.enque.header.va_addr += rst.HeaderSize;
        }

    }

    return MTRUE;
}


MBOOL TwinMgr_BufCtrl::HEADER_ADDR_HW_LIMITATION(stISP_BUF_INFO& buf,MUINT32 offset)
{
    if(buf.bReplace == MTRUE){
        if(buf.Replace.header.size == 0){
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }

        //offset pa/va's addr by dmao's offset reg
        buf.Replace.header.pa_addr -= offset;
        buf.Replace.header.va_addr -= offset;
    }
    else{

        if(buf.u_op.enque.header.size == 0){
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }


        //offset pa/va's addr by dmao's offset reg
        buf.u_op.enque.header.pa_addr -= offset;
        buf.u_op.enque.header.va_addr -= offset;
    }

    return MTRUE;
}

