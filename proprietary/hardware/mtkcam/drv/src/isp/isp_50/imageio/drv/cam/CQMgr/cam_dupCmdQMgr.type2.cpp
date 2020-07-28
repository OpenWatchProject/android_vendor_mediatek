#include "cam_dupCmdQMgr.type2.h"
#include "sec_mgr.h"

MBOOL CmdQMgrImp2::FSM_CHK(MUINT32 op,char const* caller)
{
    MBOOL ret =MTRUE;

    switch(op){
        case op_unknow: //detach
            switch(this->mFSM){
                case op_stop:
                case op_cfg:
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
                case op_suspend:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_update:
            switch(this->mFSM){
                case op_unknow:
                case op_stop:
                    ret = MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case op_getinfo:
            switch(this->mFSM){
                case op_unknow:
                    ret = MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case op_stop:
            switch(this->mFSM){
                case op_start:
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


CmdQMgrImp2::CmdQMgrImp2()
{
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    m_pCQ = NULL;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}

DupCmdQMgr* CmdQMgrImp2::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    fps;
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
    if(this->FSM_CHK(op_cfg,__FUNCTION__) == MFALSE){
        return NULL;
    }

    switch(module){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            this->mModule = module;
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO module:0x%x\n",module);
            return NULL;
            break;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE4:
            this->m_pCQ = (DMAI_B*)new DMA_CQ4();
            break;
        case ISP_DRV_CQ_THRE5:
            this->m_pCQ = (DMAI_B*)new DMA_CQ5();
            break;
        case ISP_DRV_CQ_THRE7:
            this->m_pCQ = (DMAI_B*)new DMA_CQ7();
            break;
        case ISP_DRV_CQ_THRE8:
            this->m_pCQ = (DMAI_B*)new DMA_CQ8();
            break;
        case ISP_DRV_CQ_THRE12:
            this->m_pCQ = (DMAI_B*)new DMA_CQ12();
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }
    this->mCQ = cq;

    if(subsample > 0){
        CmdQMgr_WRN("support no BurstQ mode\n");
    }

    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //create & init isp drv
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);

    if(this->m_pUserName == NULL){
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: m_pUserName is NULL\n");
    }

    this->m_pUserName[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp2:%d",this->mCQ);
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,0,(const char*)this->m_pUserName);
    //
    this->pCamDrv->init((const char*)this->m_pUserName);

    //for security
    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            //
            SecMgr_SecInfo secinfo;
            //descriptor
            secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
            secinfo.module = this->mModule;
            secinfo.cq = this->mCQ;
            secinfo.dupq = 0;
            //secinfo.burstQ = this->CmdQMgr_GetBurstQ();
            secinfo.dummy = 0;
            secinfo.buff_size = this->pCamDrv->getCQDescBufSize();
            secinfo.buff_va = (uint64_t)this->pCamDrv->getCQDescBufVirAddr();
            secinfo.memID = this->pCamDrv->getCQIonFD();

            CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx,memID:0x%x",
                                 secinfo.module,secinfo.cq,secinfo.dupq,secinfo.buff_size,secinfo.buff_va,secinfo.memID);

            if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1RegisterShareMem failed\n");
                return NULL;
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }

    //CQI config
    this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
    this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
    this->m_pCQ->config();

    //switch to immediate-mode before start_isp
    switch(this->mCQ){
        case ISP_DRV_CQ_THRE4:
            ((DMA_CQ4*)this->m_pCQ)->setCQTriggerMode(DMA_CQ4::_immediate_);
            break;
        case ISP_DRV_CQ_THRE5:
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_immediate_);
            break;
        case ISP_DRV_CQ_THRE7:
            ((DMA_CQ7*)this->m_pCQ)->setCQTriggerMode(DMA_CQ7::_immediate_);
            break;
        case ISP_DRV_CQ_THRE8:
            ((DMA_CQ8*)this->m_pCQ)->setCQTriggerMode(DMA_CQ8::_immediate_);
            break;
        case ISP_DRV_CQ_THRE12:
            ((DMA_CQ12*)this->m_pCQ)->setCQTriggerMode(DMA_CQ12::_immediate_);
            break;
        default:
            CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
            return NULL;
            break;
    }

    this->mFSM = op_cfg;
    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_attach-: CQ_PA:0x%x\n",cq);
    return this;
}

MINT32 CmdQMgrImp2::CmdQMgr_detach(void)
{
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_detach+: cq:0x%x\n",this->mCQ);
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

    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_detach-: cq:0x%x\n",this->mCQ);
    return 0;
}

ISP_DRV_CAM** CmdQMgrImp2::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    (void)DupIdx;
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }
    return &this->pCamDrv;
}


MINT32 CmdQMgrImp2::CmdQMgr_start(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_start + cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    this->pCamDrv->start();
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
                    CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE module:%d cq:%d dupq:%d",secinfo.module,secinfo.cq,secinfo.dupq);
                    if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",j,i);
                        return NULL;
                    }

                    switch(this->mCQ){
                        case ISP_DRV_CQ_THRE4:
                             CmdQMgr_DBG("CAM_CQ_THR4_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR4_BASEADDR));
                             break;
                        case ISP_DRV_CQ_THRE5:
                             CmdQMgr_DBG("CAM_CQ_THR5_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR5_BASEADDR));
                             break;
                        case ISP_DRV_CQ_THRE7:
                             CmdQMgr_DBG("CAM_CQ_THR7_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR7_BASEADDR));
                             break;
                        case ISP_DRV_CQ_THRE8:
                             CmdQMgr_DBG("CAM_CQ_THR8_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR8_BASEADDR));
                             break;
                        case ISP_DRV_CQ_THRE12:
                             CmdQMgr_DBG("CAM_CQ_THR12_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR12_BASEADDR));
                             break;
                        default:
                             CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
                             break;
                    }
                }
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }

    //
    this->m_pCQ->enable(NULL);


    //switch to continuous mode when isp_start
    switch(this->mCQ){
        case ISP_DRV_CQ_THRE4:
            if(((DMA_CQ4*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ4*)this->m_pCQ)->setCQTriggerMode(DMA_CQ4::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE5:
            if(((DMA_CQ5*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE7:
            if(((DMA_CQ7*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ7*)this->m_pCQ)->setCQTriggerMode(DMA_CQ7::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE8:
            if(((DMA_CQ8*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ8*)this->m_pCQ)->setCQTriggerMode(DMA_CQ8::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE12:
            if(((DMA_CQ12*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ12*)this->m_pCQ)->setCQTriggerMode(DMA_CQ12::_continuouse_);
            break;
        default:
            CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
            return 1;
            break;
    }

    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_start - cq:0x%x\n",this->mCQ);
    this->mFSM = op_start;
    return 0;
}
MINT32 CmdQMgrImp2::CmdQMgr_stop(void)
{
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_stop + cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_stop,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    this->pCamDrv->stop();

    //can't access hw after suspend. hw maybe occupied by other sensor.
    if(this->mFSM != op_suspend)
        this->m_pCQ->disable();

    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_stop - cq:0x%x\n",this->mCQ);
    this->mFSM = op_stop;
    return 0;
}


ISP_HW_MODULE CmdQMgrImp2::CmdQMgr_GetCurModule(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return CAM_MAX;
    }

    return this->mModule;
}


MINT32 CmdQMgrImp2::CmdQMgr_suspend(void)
{
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_suspend + cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_suspend,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_pCQ->disable();


    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_suspend - cq:0x%x\n",this->mCQ);
    this->mFSM = op_suspend;
    return 0;
}

MINT32 CmdQMgrImp2::CmdQMgr_resume(void)
{
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_resume + cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    this->m_pCQ->enable(NULL);

    //always force cq of type2 to start with inital address
    this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
    this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
    this->m_pCQ->config();

    //
    switch(this->mCQ){
        case ISP_DRV_CQ_THRE4:
            ((DMA_CQ4*)this->m_pCQ)->setCQTriggerMode(DMA_CQ4::_immediate_);
            if(((DMA_CQ4*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ4*)this->m_pCQ)->setCQTriggerMode(DMA_CQ4::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE5:
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_immediate_);
            if(((DMA_CQ5*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE7:
            ((DMA_CQ7*)this->m_pCQ)->setCQTriggerMode(DMA_CQ7::_immediate_);
            if(((DMA_CQ7*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ7*)this->m_pCQ)->setCQTriggerMode(DMA_CQ7::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE8:
            ((DMA_CQ8*)this->m_pCQ)->setCQTriggerMode(DMA_CQ8::_immediate_);
            if(((DMA_CQ8*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ8*)this->m_pCQ)->setCQTriggerMode(DMA_CQ8::_continuouse_);
            break;
        case ISP_DRV_CQ_THRE12:
            ((DMA_CQ12*)this->m_pCQ)->setCQTriggerMode(DMA_CQ12::_immediate_);
            if(((DMA_CQ12*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ12*)this->m_pCQ)->setCQTriggerMode(DMA_CQ12::_continuouse_);
            break;
        default:
            CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
            return 1;
            break;
    }

    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_resume - cq:0x%x\n",this->mCQ);
    this->mFSM = op_start;
    return 0;
}


