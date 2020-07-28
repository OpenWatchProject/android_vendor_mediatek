#include "cam_dupCmdQMgr.type3.imme.h"
#include "sec_mgr.h"


MBOOL CmdQMgrImp3_imme::FSM_CHK(MUINT32 op,char const* caller)
{
    MBOOL ret =MTRUE;

    switch(op){
        case op_unknow: //detach
            switch(this->mFSM){
                case op_cfg:
                case op_start:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_cfg:
            switch(this->mFSM){
                case op_unknow:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_start:
            switch(this->mFSM){
                case op_cfg:
                case op_start:  //cq is for event trig
                case op_suspend:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_suspend:
            switch(this->mFSM){
                case op_start:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret != MTRUE)
        CmdQMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x,from:%s\n",this->mFSM,op,caller);

    return ret;
}


CmdQMgrImp3_imme::CmdQMgrImp3_imme()
{
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    m_pCQ = NULL;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    m_pMaster = NULL;
    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}


DupCmdQMgr* CmdQMgrImp3_imme::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    fps;
    CmdQMgr_INF("CmdQMgrImp3_imme::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
    if(this->FSM_CHK(op_cfg,__FUNCTION__) == MFALSE){
        return NULL;
    }


    switch(module){
        case CAM_A_TWIN_B:
        case CAM_A_TWIN_C:
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_B:
        case CAM_A_TRIPLE_C:
            this->mModule = module;
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO module:0x%x\n",module);
            return NULL;
            break;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE11:
            if(subsample == 0){
                CmdQMgr_ERR("CQ11 is supported only subsample is enbaled\n");
                return NULL;
            }
            /*****************************************************
                    //due to CQ11 have on continuous mode.  this is not work for D-Twin.
                    //using CQ10 with immediate mode which is trigged by master cam's CQ11.
                    ********************************************************/
            this->m_pCQ = (DMAI_B*)new DMA_CQ10();
            cq = ISP_DRV_CQ_THRE10;
            break;
        case ISP_DRV_CQ_THRE1:
            if(subsample){
                CmdQMgr_ERR("CQ1 is supported only subsample is disabled\n");
                return NULL;
            }
            this->m_pCQ = (DMAI_B*)new DMA_CQ1();
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }
    this->mCQ = cq;


    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //create & init isp drv
    //
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);

    this->m_pUserName[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp3_imme:%d",this->mCQ);


    //
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,\
    0,(const char*)this->m_pUserName);


    //
    this->pCamDrv->init((const char*)this->m_pUserName);

    //for security
    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            //
            SecMgr_SecInfo secinfo;
            secinfo.module = this->mModule;
            secinfo.cq = this->mCQ;
            secinfo.dupq = this->CmdQMgr_GetDuqQ();
            //secinfo.burstQ = this->CmdQMgr_GetBurstQ();
            secinfo.dummy = 0;

            //descriptor
            secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
            secinfo.buff_size = this->pCamDrv->getCQDescBufSize();
            secinfo.buff_va = (uint64_t)this->pCamDrv->getCQDescBufVirAddr();
            secinfo.memID = this->pCamDrv->getCQIonFD();

            //vir reg.
            secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
            secinfo.sub.buff_size = this->pCamDrv->getIspVirRegSize();
            secinfo.sub.buff_va = (uint64_t)this->pCamDrv->getIspVirRegVirAddr();
            secinfo.sub.memID = this->pCamDrv->getIspVirIonFD();


            if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1RegisterShareMem failed\n");
                return NULL;
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }

    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:
            //add cqmodule
            //this->m_pCQ->write2CQ();
            //
            //switch to immediate-mode
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
            ((DMA_CQ1*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ1::_immediate_);
            break;
        case ISP_DRV_CQ_THRE10:
            //add cqmodule
            //this->m_pCQ->write2CQ();
            //
            //switch to immediate-mode
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
            ((DMA_CQ10*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ10::_immediate_);
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return NULL;
            break;
    }

    this->mFSM = op_cfg;
    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_attach-: \n");
    return this;
}


MINT32 CmdQMgrImp3_imme::CmdQMgr_detach(void)
{
    CmdQMgr_INF("CmdQMgrImp3_imme::CmdQMgr_detach+: cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_unknow,__FUNCTION__) == MFALSE){
        return 1;
    }

    //free previous memory.
    if(this->pCamDrv != NULL){
        //
        this->pCamDrv->uninit( (const char *) this->m_pUserName);
        //
        this->pCamDrv->destroyInstance();
    }


    //
    free(this->m_pUserName);

    delete this->m_pCQ;

    this->m_pCQ = NULL;
    this->pCamDrv = NULL;
    this->m_pUserName = NULL;

    this->mFSM = op_unknow;
    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_detach -: cq:0x%x\n",this->mCQ);
    return 0;
}


MINT32 CmdQMgrImp3_imme::CmdQMgr_start(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    CmdQMgr_INF("CmdQMgrImp3_imme::CmdQMgr_start + cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    this->pCamDrv->flushCmdQ();

    //
    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            SecMgr_SecInfo secinfo;
            secinfo.module = this->mModule;
            secinfo.cq = this->mCQ;
            secinfo.dummy = 0;
            for(MUINT32 j=0;j<this->CmdQMgr_GetDuqQ();j++){

                secinfo.dupq = j;
                for(MUINT32 i=0;i<this->CmdQMgr_GetBurstQ();i++){

                    //secinfo.burstQ = i;
                    secinfo.type= SECMEM_CQ_DESCRIPTOR_TABLE;
                    secinfo.sub.type= SECMEM_VIRTUAL_REG_TABLE;

                    if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",j,i);
                        return NULL;
                    }

                }
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }

    this->mFSM = op_start;

    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_start - cq:0x%x\n",this->mCQ);

    return 0;
}

MINT32 CmdQMgrImp3_imme::CmdQMgr_stop(void)
{
    CmdQMgr_INF("CmdQMgrImp3_imme::CmdQMgr_stop + cq:0x%x\n",this->mCQ);
    this->m_pCQ->disable();


    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_stop - cq:0x%x\n",this->mCQ);
    return 0;
}

MINT32 CmdQMgrImp3_imme::CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:
            if(((DMA_CQ1*)(this->m_pCQ))->setCQContent(arg1, arg2) == MFALSE)
                return 1;
            break;
        case ISP_DRV_CQ_THRE10:
            if(((DMA_CQ10*)(this->m_pCQ))->setCQContent(arg1, arg2) == MFALSE)
                return 1;
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return 1;
            break;
    }

    return 0;
}




ISP_HW_MODULE CmdQMgrImp3_imme::CmdQMgr_GetCurModule(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return CAM_MAX;
    }

    return this->mModule;
}
MBOOL CmdQMgrImp3_imme::CmdQMgr_clearCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain)
{
    targetIdxMain;
    ISP_HW_MODULE _module;

    if(pMaster == NULL){
        CmdQMgr_ERR("no master cam info\n");
        return MFALSE;
    }

    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_clearCQMode + cq:0x%x,master module:0x%x\n",this->mCQ,pMaster->CmdQMgr_GetCurModule());

    switch(this->mModule){
        case CAM_A:
            _module = CAM_A;
            break;
        case CAM_B:
        case CAM_A_TWIN_B:
        case CAM_A_TRIPLE_B:
            _module = CAM_B;
            break;
        case CAM_C:
        case CAM_A_TWIN_C:
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_C:
            _module = CAM_C;
            break;
        default:
            CmdQMgr_ERR("unsupported module:%d\n",this->mModule);
            return MFALSE;
            break;
    }

    //

    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)((CmdQMgrImp3*)pMaster)->pCamDrv;
            ((DMA_CQ1*)(this->m_pCQ))->IO2CQ(DMA_CQ1::_cq_delete_, _module);
            break;
        case ISP_DRV_CQ_THRE10:
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)((CmdQMgrImp3*)pMaster)->pCamDrv;
            ((DMA_CQ10*)(this->m_pCQ))->IO2CQ(DMA_CQ10::_cq_delete_, _module);
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return MFALSE;
            break;
    }


    CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_clearCQMode - cq:0x%x\n",this->mCQ);
    return MTRUE;
}

MBOOL CmdQMgrImp3_imme::CmdQMgr_updateCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain)
{
    targetIdxMain;
#if TWIN_CQ_SW_WORKAROUND
    ISP_HW_MODULE _module;

    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    //
    if(pMaster == NULL){
        CmdQMgr_ERR("master cam's obj is NULL\n");
        return MFALSE;
    }
    else{
        this->m_pMaster = (CmdQMgrImp3*)pMaster;
        if( this->m_pMaster->pCamDrv == NULL){
            CmdQMgr_ERR("master cam's cq obj is NULL\n");
            return MFALSE;
        }

        CmdQMgr_DBG("CmdQMgrImp3_imme::CmdQMgr_updateCQMode + master:0x%x\n",this->m_pMaster->CmdQMgr_GetCurModule());
    }

    switch(this->mModule){
        case CAM_A:
            _module = CAM_A;
            break;
        case CAM_B:
        case CAM_A_TWIN_B:
        case CAM_A_TRIPLE_B:
            _module = CAM_B;
            break;
        case CAM_C:
        case CAM_A_TWIN_C:
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_C:
            _module = CAM_C;
            break;
        default:
            CmdQMgr_ERR("unsupported module:%d\n",this->mModule);
            return MFALSE;
            break;
    }


    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:
            //set cq page's ba onto master cam's cq
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->m_pMaster->pCamDrv;
            this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR) this->pCamDrv->getCQDescBufPhyAddr();
            ((DMA_CQ1*)(this->m_pCQ))->setSlaveAddr_byCQ(_module);

            //set master cq's descriptor & setting for slave cq
            ((DMA_CQ1*)(this->m_pCQ))->IO2CQ(DMA_CQ1::_cq_add_, _module);
            ((DMA_CQ1*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ1::_immediate_,(void*)this->m_pCQ);
            ((DMA_CQ1*)(this->m_pCQ))->TrigCQ(MFALSE);
            ((DMA_CQ1*)(this->m_pCQ))->enable((void*)this->m_pCQ);
            break;
        case ISP_DRV_CQ_THRE10:
            //set cq page's ba onto master cam's cq
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->m_pMaster->pCamDrv;
            this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR) this->pCamDrv->getCQDescBufPhyAddr();
            ((DMA_CQ10*)(this->m_pCQ))->setSlaveAddr_byCQ(_module);

            //set master cq's descriptor & setting for slave cq
            ((DMA_CQ10*)(this->m_pCQ))->IO2CQ(DMA_CQ10::_cq_add_, _module);
            ((DMA_CQ10*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ10::_immediate_,(void*)this->m_pCQ);
            ((DMA_CQ10*)(this->m_pCQ))->TrigCQ(MFALSE);
            ((DMA_CQ10*)(this->m_pCQ))->enable((void*)this->m_pCQ);
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return MFALSE;
            break;
    }


    return MTRUE;
#else
    return MTRUE;
#endif
}


