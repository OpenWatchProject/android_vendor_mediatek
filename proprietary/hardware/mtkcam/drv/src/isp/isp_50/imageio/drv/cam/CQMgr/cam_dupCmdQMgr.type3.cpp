#include "cam_dupCmdQMgr.type3.h"
#include "sec_mgr.h"

MBOOL CmdQMgrImp3::FSM_CHK(MUINT32 op,char const* caller)
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


CmdQMgrImp3::CmdQMgrImp3()
{
    pCamDrv = NULL;
    pCQ10Drv = NULL;
    m_pUserName = NULL;
    m_pCQ10Name = NULL;
    mFSM = op_unknow;
    m_pCQ = NULL;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}

MBOOL CmdQMgrImp3::Load_CQ11_Addr(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    MUINT32 _cnt=0;
    MBOOL ret = MTRUE;

    sprintf(this->m_pCQ10Name,"CmdQMgrImp3_imme:%d",ISP_DRV_CQ_THRE10);
    //
    this->pCQ10Drv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,ISP_DRV_CQ_THRE10,\
        0,(const char*)this->m_pCQ10Name);
    //
    this->pCQ10Drv->init((const char*)this->m_pCQ10Name);

    this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCQ10Drv;
    this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
    this->m_pCQ->config();

    this->m_CQ10.m_pIspDrv = (IspDrvVir*)this->pCQ10Drv;
    this->m_CQ10.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCQ10Drv->getCQDescBufPhyAddr();
    this->m_CQ10.config();

    //add cqmodule
    //this->m_CQ11.m_pIspDrv = (IspDrvVir*)this->pCamDrv;
    //this->m_CQ11.write2CQ();
    this->m_CQ10.m_pIspDrv = (IspDrvVir*)this->pCQ10Drv;
    this->m_CQ10.write2CQ();

    //
    this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
    this->m_CQ10.setCQTriggerMode(DMA_CQ10::_immediate_);


    this->pCQ10Drv->flushCmdQ();

    //for security
    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            //
            SecMgr_SecInfo secinfo;
            //descriptor
            secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
            secinfo.module = this->mModule;
            secinfo.cq = ISP_DRV_CQ_THRE10;
            secinfo.dupq = 1;
            //secinfo.burstQ = 1;
            secinfo.dummy = 0;
            secinfo.buff_size = this->pCQ10Drv->getCQDescBufSize();
            secinfo.buff_va = (uint64_t)this->pCQ10Drv->getCQDescBufVirAddr();
            secinfo.memID = this->pCQ10Drv->getCQIonFD();

            if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1RegisterShareMem failed\n");
                return NULL;
            }
            if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1Migrate Table failed\n");
                return NULL;
            }

            //vir reg.
            secinfo.type = SECMEM_VIRTUAL_REG_TABLE;
            secinfo.buff_size = this->pCQ10Drv->getIspVirRegSize();
            secinfo.buff_va = (uint64_t)this->pCQ10Drv->getIspVirRegVirAddr();
            secinfo.memID = this->pCQ10Drv->getIspVirIonFD();

            if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1RegisterShareMem failed\n");
                return NULL;
            }
            if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                CmdQMgr_ERR("P1Migrate Table failed\n");
                return NULL;
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }


    this->m_CQ10.enable(NULL);

    if (this->m_CQ10.TrigCQ() == MFALSE) {
        return MFALSE;
    }
    while (CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR11_BASEADDR) != ((MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr())){
        CmdQMgr_INF("wait CQ10 load done!\n");
        usleep(10);
        if(_cnt++ > 100){
            CmdQMgr_ERR("CQ10 load fail:0x%x,0x%p\n",\
                CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR11_BASEADDR),\
                this->pCamDrv->getCQDescBufPhyAddr() );
            ret = MFALSE;
            goto EXIT;
            break;
        }
    }

EXIT:
    return ret;
}

DupCmdQMgr* CmdQMgrImp3::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    MUINT32 _cnt=0;
    fps;
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
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
        case ISP_DRV_CQ_THRE11:
            if(subsample == 0){
                CmdQMgr_ERR("CQ11 is supported only subsample is enbaled\n");
                return NULL;
            }
            this->m_pCQ = (DMAI_B*)new DMA_CQ11();
            break;
        case ISP_DRV_CQ_THRE1:
            if(subsample){
                CmdQMgr_ERR("CQ10 is supported only subsample is disabled\n");
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
    this->m_pCQ10Name = (char*)malloc(sizeof(char)*STR_LENG);

    if(this->m_pCQ10Name == NULL){
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: m_pCQ10Name is NULL\n");
    }

    this->m_pUserName[0] = '\0';
    this->m_pCQ10Name[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp3:%d",this->mCQ);


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
            //CQI config
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
            this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
            this->m_pCQ->config();

            //add cqmodule
            //this->m_pCQ->write2CQ();
            //
            ((DMA_CQ1*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ1::_p1_done_once_);
            break;
        case ISP_DRV_CQ_THRE11:
            //need to use CQ10 additionally
            //use CQ10 to load CQ11 inner & outter address.
            //because CPU can't program inner address when using device tree.
            this->Load_CQ11_Addr();

            ((DMA_CQ11*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ11::_event_trig);
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return NULL;
            break;
    }


    this->mFSM = op_cfg;
    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_attach-: \n");
    return this;
}


MINT32 CmdQMgrImp3::CmdQMgr_detach(void)
{
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_detach+: cq:0x%x\n",this->mCQ);
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

    if(this->pCQ10Drv){
        //
        this->pCQ10Drv->uninit( (const char *) this->m_pCQ10Name);
        //
        this->pCQ10Drv->destroyInstance();
    }

    //
    free(this->m_pUserName);
    free(this->m_pCQ10Name);

    delete this->m_pCQ;

    this->m_pCQ = NULL;
    this->pCamDrv = NULL;
    this->m_pUserName = NULL;
    this->pCQ10Drv = NULL;
    this->m_pCQ10Name = NULL;

    this->mFSM = op_unknow;
    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_detach -: cq:0x%x\n",this->mCQ);
    return 0;
}


MINT32 CmdQMgrImp3::CmdQMgr_start(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_start + cq:0x%x\n",this->mCQ);
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

    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:

            //
            if(this->mFSM != op_start)
                this->m_pCQ->enable(NULL);
            //
            if(((DMA_CQ1*)(this->m_pCQ))->TrigCQ() == MFALSE){
                return 1;
            }
            break;
        case ISP_DRV_CQ_THRE11:

            if(this->mFSM != op_start){
                #if 0 // move to CamQMgr_attach_imp
                MUINTPTR tmp = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
                this->pCQ10Drv->flushCmdQ();

                //
                this->m_CQ10.enable(NULL);
                //
                if(this->m_CQ10.TrigCQ() == MFALSE){
                    return 1;
                }
                while(CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR11_BASEADDR) != tmp ){
                    CmdQMgr_INF("wait CQ10 load done!\n");
                }
                #endif

                //
                this->m_pCQ->enable(NULL);
            }

            if(((DMA_CQ11*)(this->m_pCQ))->TrigCQ() == MFALSE){
                return 1;
            }
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return 1;
            break;
    }


    this->mFSM = op_start;

    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_start - cq:0x%x\n",this->mCQ);

    return 0;
}

MINT32 CmdQMgrImp3::CmdQMgr_stop(void)
{
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_stop + cq:0x%x\n",this->mCQ);
    this->m_pCQ->disable();

    //
    if(this->mCQ == ISP_DRV_CQ_THRE11)
        this->m_CQ10.disable();


    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_stop - cq:0x%x\n",this->mCQ);
    return 0;
}

MINT32 CmdQMgrImp3::CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    switch(this->mCQ){
        case ISP_DRV_CQ_THRE1:
            if(((DMA_CQ1*)(this->m_pCQ))->setCQContent(arg1, arg2) == MFALSE)
                return 1;
            break;
        case ISP_DRV_CQ_THRE11:
            if(((DMA_CQ11*)(this->m_pCQ))->setCQContent(arg1, arg2) == MFALSE)
                return 1;
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return 1;
            break;
    }

    return 0;
}

ISP_HW_MODULE CmdQMgrImp3::CmdQMgr_GetCurModule(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return CAM_MAX;
    }

    return this->mModule;
}
