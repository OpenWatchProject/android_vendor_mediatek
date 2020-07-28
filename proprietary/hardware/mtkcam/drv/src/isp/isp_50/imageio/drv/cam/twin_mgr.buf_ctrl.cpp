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


MBOOL Path_BufCtrl::FSM_UPDATE(E_FSM op)
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
    m_pTwinMgrImp = NULL;
    m_pMainCmdQMgr = NULL;
    m_pTwinCtrl = NULL;
    m_pActiveTwinCQMgr.clear();
    m_FSM = op_unknown;
}

MBOOL TwinMgr_BufCtrl::PBC_config(void* pPathMgr, vector<DupCmdQMgr*>* pSlave,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput)
{
/* vTwin pass all possible cqMgrs:
 * [dtwin:1]: cam_a/cam_a_twin_c/cam_a_tripple_b/cam_a_tripple_c
 * [dtwin:0]: cam_a/cam_c, cam_a/cam_b/cam_c
 */
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL*   pFBC = NULL, *pFBCUfo = NULL;
    DMAO_B*         pDmao = NULL, *pDmaoUfo = NULL;
    DupCmdQMgr*     pCmdQ = NULL;
    vector<DupCmdQMgr*> vCqMgrs;

    if(this->FSM_UPDATE(op_cfg) == MFALSE)
        return MFALSE;

    this->m_pTwinMgrImp = (TwinMgr_IMP*)pPathMgr;
    this->m_pTwinCtrl = &(this->m_pTwinMgrImp->m_TwinCtrl);;
    this->m_pMainCmdQMgr = this->m_pTwinMgrImp->m_pMainCmdQMgr;
    this->m_OpenedChannel.clear();
    this->m_enque_IMGO.init();
    this->m_enque_RRZO.init();
    this->m_enqRecImgo.init();
    this->m_enqRecRrzo.init();
    this->m_bUF_imgo.clear();
    this->m_bUF_rrzo.clear();

    this->m_pActiveTwinCQMgr.assign(CAM_MAX, NULL);
    for (MUINT32 i = 0; i< pSlave->size(); i++) {
        this->m_pActiveTwinCQMgr.at(pSlave->at(i)->CmdQMgr_GetCurModule()) = pSlave->at(i);
    }

    vCqMgrs.push_back(this->m_pMainCmdQMgr);
    for (MUINT32 mod = 0; mod < pSlave->size(); mod++) {
        vCqMgrs.push_back(pSlave->at(mod));
    }

    for (MUINT32 i = 0; i < vCqMgrs.size(); i++) {
        CAM_TIMESTAMP* pTime;

        pCmdQ = vCqMgrs.at(i);

        pTime = CAM_TIMESTAMP::getInstance(pCmdQ->CmdQMgr_GetCurModule(),pCmdQ->CmdQMgr_GetCurCycleObj(0)[0]);
        pTime->TimeStamp_SrcClk(pInput->tTimeClk);

        for (it = pchannel->begin(); it != pchannel->end(); it++) {
            switch (*it) {
            case _imgo_:
                pFBC = &this->m_Imgo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Imgo[pCmdQ->CmdQMgr_GetCurModule()];

                pFBCUfo = &this->m_Ufeo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                pDmaoUfo = &this->m_Ufeo[pCmdQ->CmdQMgr_GetCurModule()];
                break;
            case _rrzo_:
                pFBC = &this->m_Rrzo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                pDmao = &this->m_Rrzo[pCmdQ->CmdQMgr_GetCurModule()];

                pFBCUfo = &this->m_Ufgo_FBC[pCmdQ->CmdQMgr_GetCurModule()];
                pDmaoUfo = &this->m_Ufgo[pCmdQ->CmdQMgr_GetCurModule()];
                break;
            default:
                LOG_ERR("unsupported dmao:%d\n",*it);
                return MFALSE;
                break;
            }

            pFBC->m_pTimeStamp = pTime;
            pFBCUfo->m_pTimeStamp = pTime;

            for(MUINT32 i = 0;i<pCmdQ->CmdQMgr_GetDuqQ();i++){
                for(MUINT32 j=0;j<pCmdQ->CmdQMgr_GetBurstQ();j++){
                    IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];

                    pFBC->m_pIspDrv = _ispDrv;
                    pFBC->config();
                    pFBC->write2CQ();
                    pFBC->enable(NULL);

                    pDmao->m_pIspDrv = _ispDrv;
                    //twin's dmao cfg is cloned via twin_drv. can't config here due to have no dmao's size information.
                    //pDmao->config();
                    pDmao->write2CQ();
                    pDmao->enable(NULL);


                    pFBCUfo->m_pIspDrv = _ispDrv;
                    pFBCUfo->config();
                    pFBCUfo->write2CQ();

                    pDmaoUfo->m_pIspDrv = _ispDrv;
                    pDmaoUfo->config();
                    pDmaoUfo->write2CQ();
                }
            }

            //do need to push twice at master&slave cam
            if (i == 1) { //(pCmdQ == this->m_pTwinCmdQMgr) {
                this->m_OpenedChannel.push_back(*it);
            }
        }
    }

    return MTRUE;
}


MBOOL TwinMgr_BufCtrl::PBC_Start(MVOID)
{
    return this->FSM_UPDATE(op_startTwin);
}

MBOOL TwinMgr_BufCtrl::PBC_Stop(MVOID)
{
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL* pFBC = NULL, *pFBCUfo = NULL;
    DMAO_B* pDmao = NULL, *pDmaoUfo = NULL;

    if(this->FSM_UPDATE(op_unknown) == MFALSE)
        return MFALSE;

    for (MUINT32 mod = 0; mod < this->m_pActiveTwinCQMgr.size() ; mod++) {
        if (this->m_pActiveTwinCQMgr.at(mod) == NULL) {
            // inactive cam, no need to disable
            continue;
        }

        //main path's stop is cfg at cam_path_p1
        for (it = this->m_OpenedChannel.begin(); it!=this->m_OpenedChannel.end(); it++) {
            switch(*it){
                case _imgo_:
                    pFBC = &this->m_Imgo_FBC[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Imgo[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];

                    pFBCUfo = &this->m_Ufeo_FBC[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    pDmaoUfo = &this->m_Ufeo[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    break;
                case _rrzo_:
                    pFBC = &this->m_Rrzo_FBC[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Rrzo[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];

                    pFBCUfo = &this->m_Ufgo_FBC[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    pDmaoUfo = &this->m_Ufgo[this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurModule()];
                    break;
                default:
                    LOG_ERR("unsupported dmao:%d\n",*it);
                    return MFALSE;
                    break;
            }

            for (MUINT32 i = 0; i < this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetDuqQ(); i++) {
                for (MUINT32 j = 0; j < this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetBurstQ(); j++) {
                    IspDrvVir *_ispDrv = (IspDrvVir*)this->m_pActiveTwinCQMgr.at(mod)->CmdQMgr_GetCurCycleObj(i)[j];

                    pFBC->m_pIspDrv = _ispDrv;
                    pFBC->disable();
                    pDmao->m_pIspDrv = _ispDrv;
                    pDmao->disable();

                    pFBCUfo->m_pIspDrv = _ispDrv;
                    pFBCUfo->disable();
                    pDmaoUfo->m_pIspDrv = _ispDrv;
                    pDmaoUfo->disable();
                }
            }
        }
    }

    return MTRUE;
}



MBOOL TwinMgr_BufCtrl::enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, vector<ISP_HW_MODULE> *pTwinVMod)
{
/* vTwinVMod:
 * [cam_a]
 *      cam_a_twin_c
 *      cam_a_triple_b/cam_a_triple_c
 *      cam_c
 * [cam_b]
 *      cam_b_twin_c
 *      cam_c
 */
    MBOOL                       ret = MTRUE;
    MUINT32                     targetIdx = 0;
    QueueMgr<ISP_BUF_INFO_L>    *pQue = NULL;
    CAM_BUF_CTRL                *pFBC = NULL, *pFBCUfo = NULL;
    DMAO_B                      *pDmao = NULL, *pDmaoUfo = NULL;

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        return MFALSE;
    }

    switch (dmaChannel) {
    case _imgo_:
        pQue = &this->m_enque_IMGO;
        this->m_bUF_imgo.clear();
        break;
    case _rrzo_:
        pQue = &this->m_enque_RRZO;
        this->m_bUF_rrzo.clear();
        break;
    default:
        LOG_ERR("unsupported dmao:%d\n", dmaChannel);
        ret = MFALSE;
        goto EXIT;
    }

    pQue->sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_push,(MUINTPTR)&bufInfo,0);
    //behavior of enque into hw will be invoked after runtwin

    //always open dmao/fbc for twin_drv's input info.
    for (MUINT32 mod = 0; mod < pTwinVMod->size() ; mod++) {

        {
            switch (dmaChannel) {
            case _imgo_:
                pFBC = &this->m_Imgo_FBC[pTwinVMod->at(mod)];
                pDmao = &this->m_Imgo[pTwinVMod->at(mod)];

                pFBCUfo = &this->m_Ufeo_FBC[pTwinVMod->at(mod)];
                pDmaoUfo = &this->m_Ufeo[pTwinVMod->at(mod)];

                break;
            case _rrzo_:
                pFBC = &this->m_Rrzo_FBC[pTwinVMod->at(mod)];
                pDmao = &this->m_Rrzo[pTwinVMod->at(mod)];

                pFBCUfo = &this->m_Ufgo_FBC[pTwinVMod->at(mod)];
                pDmaoUfo = &this->m_Ufgo[pTwinVMod->at(mod)];

                break;
            default:
                LOG_ERR("unsupported dmao:%d\n",dmaChannel);
                return MFALSE;
            }

            targetIdx = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();


            LOG_DBG("slave:0x%x dma:0x%x dupIdx:%d plane:%d\n", pTwinVMod->at(mod), dmaChannel, targetIdx,bufInfo.front().u_op.enque.size());
            for (MUINT32 j = 0; j < this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ(); j++) {
                IspDrvVir *_ispDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[j];

                pFBC->m_pIspDrv = _ispDrv;
                pFBC->enable(NULL);

                pDmao->m_pIspDrv = _ispDrv;
                pDmao->enable(NULL);

                pFBCUfo->m_pIspDrv = _ispDrv;
                pDmaoUfo->m_pIspDrv = _ispDrv;
                if (bufInfo.front().u_op.enque.size() == 3) {
                    pFBCUfo->enable(NULL);
                    pDmaoUfo->enable(NULL);
                }
                else {
                    pFBCUfo->disable();
                    pDmaoUfo->disable();
                }
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
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA> channel;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA>::iterator it_frame;
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;
    CAM_BUF_CTRL                *pFBC[CAM_MAX];
    CAM_BUF_CTRL                *pFBCUfo[CAM_MAX];
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    UFDG_META_INFO              localMetaBuf[PHY_CAM];
    NSImageio::NSIspio::BufInfo buf;
    DupCmdQMgr                  *pCQMgr = NULL;
    MINT32                      ret = 0;
    MBOOL                       bWaitFlg[CAM_MAX];
    MUINT32                     burstIdx = 0, _size = 0, deqCamBitMap = 0;

    memset(bWaitFlg, MFALSE, sizeof(MBOOL)*CAM_MAX);

    if(this->FSM_UPDATE(op_runtwin) == MFALSE) {
        return 1;
    }

    if (bufInfo.size() != 0) {
        LOG_ERR("list is not empty\n");
        return -1;
    }

    if (this->m_pTwinCtrl->get_RunTwinRst(dmaChannel, channel, MTRUE)) {
        LOG_ERR("twin's dma ctrl result error\n");
        return -1;
    }

    for (it_frame = channel.begin(); it_frame != channel.end(); it_frame++) {//for subsample
        for (it = it_frame->begin(); it != it_frame->end(); it++) {
            if (it->bFrameEnd == MFALSE) {
                bWaitFlg[it->module] = MTRUE;
            }
        }
    }

    switch(dmaChannel){
    case _imgo_:
        for (MUINT32 i = 0; i < CAM_MAX; i++) {
            pFBC[i] = &this->m_Imgo_FBC[i];
            pFBCUfo[i] = &this->m_Ufeo_FBC[i];
        }
        pEnqBufMgr = &this->m_enqRecImgo;
        break;
    case _rrzo_:
        for (MUINT32 i = 0; i < CAM_MAX; i++) {
            pFBC[i] = &this->m_Rrzo_FBC[i];
            pFBCUfo[i] = &this->m_Ufgo_FBC[i];
        }
        pEnqBufMgr = &this->m_enqRecRrzo;
        break;
    default:
        LOG_ERR("unsupported dmao:%d\n", it->channel);
        return -1;
    }

    //wait twice is because of twin_mgr have no information of which signal being used at ifunc_buf_ctrl
    if (bWaitFlg[this->m_pMainCmdQMgr->CmdQMgr_GetCurModule()]) {
        switch (pFBC[this->m_pMainCmdQMgr->CmdQMgr_GetCurModule()]->waitBufReady(pNotify)) {
        case eCmd_Fail:
            return -1;
        case eCmd_Stop_Pass:
            LOG_WRN("%s: cam:0x%x dma(0x%x) under stop\n", "master", this->m_pMainCmdQMgr->CmdQMgr_GetCurModule(), dmaChannel);
            return 1;
        case eCmd_Suspending_Pass:
            //need to do deque , in order to pop all those requests in drv.
            break;
        default:
            break;
        }
    }
    bWaitFlg[this->m_pMainCmdQMgr->CmdQMgr_GetCurModule()] = MFALSE; //already wait master cam

    for (MUINT32 i = CAM_A; i < CAM_MAX; i++) {
        if (bWaitFlg[i]) {
            switch(pFBC[i]->waitBufReady(pNotify)){
            case eCmd_Fail:
                return -1;
            case eCmd_Stop_Pass:
                LOG_WRN("%s: cam:0x%x dma(0x%x) under stop\n", "slave", this->m_pMainCmdQMgr->CmdQMgr_GetCurModule(), dmaChannel);
                return 1;
            case eCmd_Suspending_Pass:
                //need to do deque , in order to pop all those requests in drv.
                break;
            default:
                break;
            }
        }
    }

    this->m_lock.lock();
    for (it_frame = channel.begin(); it_frame!=channel.end(); it_frame++) {//for subsample
        deqCamBitMap = 0;
        for (it = it_frame->begin(); it != it_frame->end(); it++) {//for multi-cam
            if (it->bFrameEnd == MFALSE) {
                //can't update this para. , in order to prevent racing condition
                //also , this m_pispdrv is useless under waitbufready & dequeuehwbuf
                //pFBC[pCQMgr->CmdQMgr_GetCurModule()].m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[0];
                switch (pFBC[it->module]->dequeueHwBuf(buf)) {
                case eCmd_Fail:
                    LOG_ERR("data is rdy on dram,but deque fail at burst:0x%x,cam:%d\n", burstIdx, it->module);
                    ret = -1;
                    goto EXIT;
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

                if (buf.bUF_DataFmt) {
                    BufInfo         bufUfo;

                    switch (ModuleVirtToPhy(it->module)) {
                    case CAM_A:
                        deqCamBitMap |= 0x1;
                        bufUfo.m_pPrivate = (MUINT32 *)&(localMetaBuf[CAM_A]);
                        break;
                    case CAM_B:
                        deqCamBitMap |= 0x2;
                        bufUfo.m_pPrivate = (MUINT32 *)&(localMetaBuf[CAM_B]);
                        break;
                    case CAM_C:
                        deqCamBitMap |= 0x4;
                        bufUfo.m_pPrivate = (MUINT32 *)&(localMetaBuf[CAM_C]);
                        break;
                    default:
                        bufUfo.m_pPrivate = (MUINT32 *)&(localMetaBuf[CAM_A]);
                        LOG_ERR("Unknown cam module: %d_%d\n", ModuleVirtToPhy(it->module), it->module);
                        break;
                    }
                    memset(bufUfo.m_pPrivate, 0, sizeof(UFDG_META_INFO));

                    switch(pFBCUfo[it->module]->dequeueHwBuf(bufUfo)) {
                    case eCmd_Fail:
                        LOG_ERR("[%d] data is rdy on dram,but deque fail\n", it->module);
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

                    // update after loop ((UFDG_META_INFO*)bufUfo.m_pPrivate)->UFDG_BOND_MODE = 0;// non-twin in this path

                    buf.u4BufSize[ePlane_2nd]    = bufUfo.u4BufSize[ePlane_2nd];  //  Per buffer size
                    buf.u4BufVA[ePlane_2nd]      = bufUfo.u4BufVA[ePlane_2nd];    //  Vir Address of pool
                    buf.u4BufPA[ePlane_2nd]      = bufUfo.u4BufPA[ePlane_2nd];    //  Phy Address of pool
                    buf.u4BufOffset[ePlane_2nd]  = bufUfo.u4BufOffset[ePlane_2nd];
                    buf.memID[ePlane_2nd]        = bufUfo.memID[ePlane_2nd];      //  memory ID
                    buf.bufSecu[ePlane_2nd]      = bufUfo.bufSecu[ePlane_2nd];
                    buf.bufCohe[ePlane_2nd]      = bufUfo.bufCohe[ePlane_2nd];
                    buf.Frame_Header.u4BufPA[ePlane_2nd]   = bufUfo.Frame_Header.u4BufSize[ePlane_2nd];
                    buf.Frame_Header.u4BufVA[ePlane_2nd]   = bufUfo.Frame_Header.u4BufVA[ePlane_2nd];
                    buf.Frame_Header.u4BufSize[ePlane_2nd] = bufUfo.Frame_Header.u4BufSize[ePlane_2nd];
                    buf.Frame_Header.memID[ePlane_2nd]     = bufUfo.Frame_Header.memID[ePlane_2nd];
                    buf.Frame_Header.bufCohe[ePlane_2nd]   = bufUfo.Frame_Header.bufCohe[ePlane_2nd];
                    buf.Frame_Header.bufSecu[ePlane_2nd]   = bufUfo.Frame_Header.bufSecu[ePlane_2nd];
                }

            }
            else {
                stISP_BUF_INFO  bufRec;
                UFDG_META_INFO *pUfMeta = NULL;

                pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_getsize, (MUINTPTR)&_size, 0);
                if (_size == 0) {
                    LOG_ERR("m_pPrivate queue buffer depth cannot be 0\n");
                    ret = -1;
                    break; //to next burstIdx
                }
                else {
                    pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&bufRec);
                    pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_pop, 0, 0);

                    if (bufRec.u_op.enque.size() >= ePlane_3rd) {
                        pUfMeta = (UFDG_META_INFO *)(bufRec.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);
                    }
                }

                buf.m_pPrivate = (MUINT32 *)pUfMeta;

                if (pUfMeta) { //enque with 3 plane
                    pUfMeta->bUF = (deqCamBitMap)? MTRUE: MFALSE;

                    if ((deqCamBitMap & 0x6) == 0x6) { // b,c
                        pUfMeta->UFDG_BITSTREAM_OFST_ADDR   = localMetaBuf[CAM_C].UFDG_BITSTREAM_OFST_ADDR;
                        pUfMeta->UFDG_BS_AU_START           = localMetaBuf[CAM_C].UFDG_BS_AU_START;
                        pUfMeta->UFDG_AU2_SIZE              = localMetaBuf[CAM_C].UFDG_AU2_SIZE;
                        pUfMeta->UFDG_BOND_MODE             = 1;
                    }
                    else if ((deqCamBitMap & 0x5) == 0x5) { // a,c
                        pUfMeta->UFDG_BITSTREAM_OFST_ADDR   = localMetaBuf[CAM_C].UFDG_BITSTREAM_OFST_ADDR;
                        pUfMeta->UFDG_BS_AU_START           = localMetaBuf[CAM_C].UFDG_BS_AU_START;
                        pUfMeta->UFDG_AU2_SIZE              = localMetaBuf[CAM_C].UFDG_AU2_SIZE;
                        pUfMeta->UFDG_BOND_MODE             = 1;
                    }
                    else if ((deqCamBitMap & 0x3) == 0x3) { // a,b
                        pUfMeta->UFDG_BITSTREAM_OFST_ADDR   = localMetaBuf[CAM_B].UFDG_BITSTREAM_OFST_ADDR;
                        pUfMeta->UFDG_BS_AU_START           = localMetaBuf[CAM_B].UFDG_BS_AU_START;
                        pUfMeta->UFDG_AU2_SIZE              = localMetaBuf[CAM_B].UFDG_AU2_SIZE;
                        pUfMeta->UFDG_BOND_MODE             = 1;
                    }
                    else {
                        pUfMeta->UFDG_BOND_MODE             = 0;
                    }
                    if (deqCamBitMap == 0) {
                        // enque with ufo but hw constraint

                        for (MUINT32 plane = 0; plane < ePlane_max; plane++) {
                            buf.Frame_Header.u4BufPA[plane]     = bufRec.u_op.enque.at(plane).header.pa_addr;
                            buf.Frame_Header.u4BufVA[plane]     = bufRec.u_op.enque.at(plane).header.va_addr;
                            buf.Frame_Header.u4BufSize[plane]   = bufRec.u_op.enque.at(plane).header.size;
                            buf.Frame_Header.memID[plane]       = bufRec.u_op.enque.at(plane).header.memID;
                            buf.Frame_Header.bufCohe[plane]     = bufRec.u_op.enque.at(plane).header.bufCohe;
                            buf.Frame_Header.bufSecu[plane]     = bufRec.u_op.enque.at(plane).header.bufSecu;
                        }
                    }
                    else {
                        buf.Frame_Header.u4BufPA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.pa_addr;
                        buf.Frame_Header.u4BufVA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.va_addr;
                        buf.Frame_Header.u4BufSize[ePlane_3rd] = bufRec.u_op.enque.at(ePlane_3rd).header.size;
                        buf.Frame_Header.memID[ePlane_3rd]     = bufRec.u_op.enque.at(ePlane_3rd).header.memID;
                        buf.Frame_Header.bufCohe[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufCohe;
                        buf.Frame_Header.bufSecu[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufSecu;
                    }
                }
                else { //always 3 plane of fh ??
                }

                bufInfo.push_back(buf);
                burstIdx++;

                deqCamBitMap = 0;
            }
        }
    }

EXIT:
    this->m_lock.unlock();

    if (ret == 0) {
        this->m_pTwinCtrl->clr_RunTwinRst(dmaChannel);
    }

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
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA> channel;
    vector<CAM_TWIN_PIPE::L_T_TWIN_DMA>::iterator it_frame;
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;
    CAM_BUF_CTRL    *pFBC = NULL, *pFBCUfo = NULL;
    DMAO_B          *pDmao = NULL, *pDmaoUfo = NULL;
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    DupCmdQMgr      *pCQMgr = NULL;
    ISP_BUF_INFO_L  _buf_list;
    MUINT32         ret = 0, targetIdx = 0, FrameCnt = 0;
    vector<MBOOL>   *pUF = NULL;

    Mutex::Autolock __lock(this->m_lock);

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        ret = 1;
        goto EXIT;
    }

    if (this->m_pTwinCtrl->get_RunTwinRst(dmaChannel, channel)) {
        LOG_ERR("twin's dma ctrl result error\n");
        ret = 1;
        goto EXIT;
    }

    switch (dmaChannel) {
    case _imgo_:
        this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front, (MUINTPTR)&_buf_list, 0);
        break;
    case _rrzo_:
        this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front, (MUINTPTR)&_buf_list, 0);
        break;
    default:
        LOG_ERR("unsupported dmao:%d\n", dmaChannel);
        ret = 1;
        goto EXIT;
        break;
    }

    for (it_frame = channel.begin(); it_frame != channel.end(); it_frame++) {//for subsample
        for (it = it_frame->begin(); it != it_frame->end(); it++) {

            if (it->bFrameEnd == MFALSE) {
                IspDrvVir       *_pCurPage = NULL;
                stISP_BUF_INFO  _tmp_buf = _buf_list.front();

                pCQMgr = (it->module == this->m_pMainCmdQMgr->CmdQMgr_GetCurModule())
                            ? this->m_pMainCmdQMgr
                            : this->m_pActiveTwinCQMgr.at(it->module);

                if (pCQMgr == NULL) {
                    LOG_ERR("cannot find correct CmdQMgr module:%d main:%d\n",
                        it->module, this->m_pMainCmdQMgr->CmdQMgr_GetCurModule());
                    ret = 1;
                    goto EXIT;
                }

                switch(dmaChannel){
                case _imgo_:
                    pFBC = &this->m_Imgo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Imgo[pCQMgr->CmdQMgr_GetCurModule()];

                    pFBCUfo = &this->m_Ufeo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                    pDmaoUfo = &this->m_Ufeo[pCQMgr->CmdQMgr_GetCurModule()];

                    pEnqBufMgr = &this->m_enqRecImgo;

                    pUF = &this->m_bUF_imgo;
                    break;
                case _rrzo_:
                    pFBC = &this->m_Rrzo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                    pDmao = &this->m_Rrzo[pCQMgr->CmdQMgr_GetCurModule()];

                    pFBCUfo = &this->m_Ufgo_FBC[pCQMgr->CmdQMgr_GetCurModule()];
                    pDmaoUfo = &this->m_Ufgo[pCQMgr->CmdQMgr_GetCurModule()];

                    pEnqBufMgr = &this->m_enqRecRrzo;

                    pUF = &this->m_bUF_rrzo;
                    break;
                default:
                    LOG_ERR("unsupported dmao:%d\n",it->channel);
                    ret = 1;
                    goto EXIT;
                }

                targetIdx = pCQMgr->CmdQMgr_GetDuqQIdx();
                _pCurPage = (IspDrvVir*)pCQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[FrameCnt];

                pFBC->m_pIspDrv = _pCurPage;

                //patch header's address which is for separate DMAO
                this->HeaderUpdate(_tmp_buf, it->module);

                if(eCmd_Fail == pFBC->enqueueHwBuf(_tmp_buf, MTRUE)){
                    LOG_ERR("enque fail at burst:0x%x\n",FrameCnt);
                    ret = 1;
                }

                if(_tmp_buf.bReplace == MTRUE){
                    pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.Replace.image.mem_info.pa_addr;

                    pDmao->Header_Addr = _tmp_buf.Replace.header.pa_addr;
                }
                else {
                    pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;

                    pDmao->Header_Addr = _tmp_buf.u_op.enque.at(ePlane_1st).header.pa_addr;
                }
                pDmao->dma_cfg.memBuf.ofst_addr = it->offsetAdr;//update offsetadr updated by twin

                pDmao->m_pIspDrv = _pCurPage;

                if (pDmao->setBaseAddr()) {
                    LOG_ERR("set baseaddress at burst:0x%x pa:0x%x\n", FrameCnt, pDmao->dma_cfg.memBuf.base_pAddr);
                    ret = 1;
                }

                if (_tmp_buf.u_op.enque.size() == 3) {
                    if((pUF == NULL) ||(pUF->size() == 0)){
                        LOG_ERR("UF patch should be NULL(0x%x) or 0",pUF);
                        ret = 1;
                    }
                    else if(pUF->at(FrameCnt) == MFALSE){
                        stISP_BUF_INFO  ufoBuf;
                        ST_BUF_INFO stbuf;


                        pFBCUfo->m_pIspDrv = _pCurPage;
                        pDmaoUfo->m_pIspDrv = _pCurPage;

                        stbuf.image = _tmp_buf.u_op.enque.at(ePlane_2nd).image;
                        stbuf.header = _tmp_buf.u_op.enque.at(ePlane_2nd).header;
                        ufoBuf.u_op.enque.push_back(stbuf);

                        this->HeaderUpdate(ufoBuf, it->module);

                        if(eCmd_Fail == pFBCUfo->enqueueHwBuf(ufoBuf)){
                            LOG_ERR("dma[0x%x] ufo_pa=0x%x m_pPrivate\n", dmaChannel, ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                            ret = 1;
                        }

                        if (ufoBuf.bReplace == MTRUE) {
                            pDmaoUfo->dma_cfg.memBuf.base_pAddr = ufoBuf.Replace.image.mem_info.pa_addr;
                            pDmaoUfo->Header_Addr = ufoBuf.Replace.header.pa_addr;
                        }
                        else {
                            pDmaoUfo->dma_cfg.memBuf.base_pAddr = ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
                            pDmaoUfo->Header_Addr = ufoBuf.u_op.enque.at(ePlane_1st).header.pa_addr;
                        }
                        pDmaoUfo->dma_cfg.memBuf.ofst_addr = it->uf_offsetAdr;//update offsetadr updated by twin
                        if (pDmaoUfo->setBaseAddr()) {
                            LOG_ERR("dma[0x%x] bayer_pa=0x%x ufo_pa=0x%x burst=%d\n", dmaChannel,
                                _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                            ret = 1;
                        }
                    }
                }
                //no need to disable ufe, already disabled by twin-pipe && enque-push
            }
            else {
                stISP_BUF_INFO  _buf = _buf_list.front();

                if(!pEnqBufMgr){
                    LOG_ERR("module:%x has no dma:%x enque records", it->module, it->channel);
                    ret = 1;
                    goto EXIT;
                }

                if (_buf.u_op.enque.size() == 3) {
                    UFDG_META_INFO  *pUfMeta = (UFDG_META_INFO *)(_buf.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);

                    if (pUfMeta) {
                        memset(pUfMeta, 0, sizeof(UFDG_META_INFO));
                    }
                    else {
                        LOG_ERR("dma[0x%x] bayer_pa=0x%x m_pPrivate buf ptr cannot be null!!!\n",
                            dmaChannel, _buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                        ret = 1;
                    }
                }

                pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_push, (MUINTPTR)&_buf, 0);

                _buf_list.pop_front();

                FrameCnt++;
            }

        }

    }

    if (pCQMgr) {
        if (FrameCnt != pCQMgr->CmdQMgr_GetBurstQ()) {
            LOG_ERR("enque number != burst number .(%d_%d)\n",FrameCnt,pCQMgr->CmdQMgr_GetBurstQ());
            ret = 1;
        }
    }
    else {
        LOG_ERR("Null cqmgr obj invalid flow\n");
    }

    switch (dmaChannel) {
    case _imgo_:
        this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop, 0, 0);
        break;
    case _rrzo_:
        this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop, 0, 0);
        break;
    default:
        LOG_ERR("unsupported dmao:%d\n",dmaChannel);
        ret = 1;
        break;
    }

EXIT:

    return ret;
}

MBOOL TwinMgr_BufCtrl::HeaderUpdate(stISP_BUF_INFO& buf, ISP_HW_MODULE curModule)
{
    CAM_TWIN_PIPE::L_T_TWIN_DMA::iterator it;
    capibility  CamInfo;
    tCAM_rst    rst;
    MUINT32     distance = 0;

    if (CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                rst,E_CAM_HEADER_size) == MFALSE){
        return MFALSE;
    }

    switch (curModule) {
    case CAM_A:
    case CAM_B:
        if (this->m_pMainCmdQMgr->CmdQMgr_GetCurModule() != curModule)//3raw
            distance = 2;
        else
            distance = 0;
        break;
    case CAM_C:
    case CAM_A_TWIN_C:
    case CAM_B_TWIN_C:
    case CAM_A_TRIPLE_B:
        distance = 1;
        break;
    case CAM_A_TRIPLE_C:
        distance = 2;
        break;
    default:
        LOG_ERR("Unsupported module: %d\n", curModule);
        return MFALSE;
    }

    if (buf.bReplace == MTRUE) {
        if (buf.Replace.header.size == 0) {
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }
        buf.Replace.header.pa_addr += (ALIGN_HEAD_SIZE(rst.HeaderSize) * distance);
        buf.Replace.header.va_addr += (ALIGN_HEAD_SIZE(rst.HeaderSize) * distance);
    }
    else {
        if (buf.u_op.enque.at(ePlane_1st).header.size == 0) {
            LOG_ERR("header size can't be 0\n");
            return MFALSE;
        }
        buf.u_op.enque.at(ePlane_1st).header.pa_addr += (ALIGN_HEAD_SIZE(rst.HeaderSize) * distance);
        buf.u_op.enque.at(ePlane_1st).header.va_addr += (ALIGN_HEAD_SIZE(rst.HeaderSize) * distance);
    }

    return MTRUE;
}

MBOOL TwinMgr_BufCtrl::enque_UF_patch(vector<T_UF>* pUF)
{
    list<MUINT32>::iterator it;
    MBOOL bmatch = MFALSE;
    vector<MBOOL>* pVector;

    if(pUF == NULL){
        LOG_ERR("can't be NULL");
        return MFALSE;
    }
    for (it = this->m_OpenedChannel.begin(); it!=this->m_OpenedChannel.end(); it++) {
        MUINT32 i;
        switch(*it){
            case _imgo_:
                pVector = &this->m_bUF_imgo;
                break;
            case _rrzo_:
                pVector = &this->m_bUF_rrzo;
                break;
            default:
                bmatch = MFALSE;
                LOG_ERR("unsupported port:%d\n",*it);
                return MFALSE;
                break;
        }

        for(i=0;i<pUF->size();i++){
            if(pUF->at(i).dmachannel == *it){
                bmatch = MTRUE;
                break;
            }
        }
        if(bmatch)
            pVector->push_back(pUF->at(i).bOFF);
        else{
            LOG_ERR("logic err,find no match case\n");
        }
    }



    return bmatch;
}

