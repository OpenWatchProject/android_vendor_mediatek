#include "cam_dupCmdQMgr.type1.h"
#include "sec_mgr.h"

MBOOL CmdQMgrImp::FSM_CHK(MUINT32 op,char const* caller)
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
                //case op_suspend:   //for enque before resume
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



CmdQMgrImp::CmdQMgrImp()
{
    mBurstQ = 0;
    mDupCmdQIdx = 0;
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    mDupCmdQ = 0;
    mBurstQIdx = 0;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    pDummyDrv = NULL;
    m_pDummyName = NULL;
}

DupCmdQMgr* CmdQMgrImp::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

    fps;
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
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
        case ISP_DRV_CQ_THRE0:
            this->mCQ = cq;
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }

    this->mDupCmdQ = DupCmdQMgr::DEFAULT_DUP_NUM;

    if(this->mDupCmdQ < 1){
        CmdQMgr_ERR("DupQ must be> 1\n");
        return NULL;
    }

    if(subsample >= (Max_PageNum/this->mDupCmdQ - 1)){
        CmdQMgr_ERR("BurstQ is out of range:max:0x%x,cur:0x%x\n",(Max_PageNum/DupCmdQMgr::DEFAULT_DUP_NUM - 1),subsample);
        return NULL;
    }
    else{
        this->mBurstQ = subsample + 1;
        this->m_CQ0.m_bSubsample = (this->mBurstQ >1)? (MTRUE):(MFALSE);
    }

    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //malloc & create & init isp drv
    this->pCamDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);
    this->pDummyDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);

    this->m_pUserName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);
    this->m_pDummyName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        this->pCamDrv[j] = (ISP_DRV_CAM**)malloc(sizeof(ISP_DRV_CAM*)*this->mBurstQ);
        this->pDummyDrv[j] = (ISP_DRV_CAM**)malloc(sizeof(ISP_DRV_CAM*)*this->mBurstQ);

        this->m_pUserName[j] = (char**)malloc(sizeof(char*)*this->mBurstQ);
        this->m_pDummyName[j] = (char**)malloc(sizeof(char*)*this->mBurstQ);
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_pUserName[j][i] = (char*)malloc(sizeof(char)*STR_LENG);

            this->m_pUserName[j][i][0] = '\0';
            sprintf(this->m_pUserName[j][i],"CmdQMgrImp:%d",(j*this->mBurstQ + i));

            this->m_pDummyName[j][i] = (char*)malloc(sizeof(char)*STR_LENG);

            this->m_pDummyName[j][i][0] = '\0';
            sprintf(this->m_pDummyName[j][i],"dummy:%d",(j*this->mBurstQ + i));

            //
            this->pCamDrv[j][i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,\
            (j*this->mBurstQ + i),(const char*)this->m_pUserName[j][i]);

            this->pDummyDrv[j][i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ, \
                (this->mBurstQ * this->mDupCmdQ)+(j*this->mBurstQ + i) ,(const char*)this->m_pDummyName[j][i]);

            //
            this->pCamDrv[j][i]->init((const char*)this->m_pUserName[j][i]);

            this->pDummyDrv[j][i]->init((const char*)this->m_pDummyName[j][i]);

            //for security
            if(pSec){
                if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
                    //
                    SecMgr_SecInfo secinfo;
 
                    secinfo.module = this->mModule;
                    secinfo.cq = this->mCQ;
                    secinfo.dupq = j;
                    secinfo.dummy = 0;

                    //descriptor
                    secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
                    secinfo.buff_size = this->pCamDrv[j][i]->getCQDescBufSize();
                    secinfo.buff_va = (uint64_t)this->pCamDrv[j][i]->getCQDescBufVirAddr();
                    secinfo.memID = this->pCamDrv[j][i]->getCQIonFD();

                    //vir reg.
                    secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
                    secinfo.sub.buff_size = this->pCamDrv[j][i]->getIspVirRegSize();
                    secinfo.sub.buff_va = (uint64_t)this->pCamDrv[j][i]->getIspVirRegVirAddr();
                    secinfo.sub.memID = this->pCamDrv[j][i]->getIspVirIonFD();


                    CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx memID:0x%x",
                                 secinfo.module,secinfo.cq,secinfo.dupq,secinfo.buff_size,secinfo.buff_va,secinfo.memID);
                    CmdQMgr_DBG("type:SECMEM_VIRTUAL_REG_TABLE module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx memID:0x%x",
                                 secinfo.module,secinfo.cq,secinfo.dupq,secinfo.sub.buff_size,secinfo.sub.buff_va,secinfo.sub.memID);

                    if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1RegisterShareMem failed\n");
                        return NULL;
                    }

                    //Dummy descriptor
                    secinfo.dummy = 1;
                    secinfo.buff_size = this->pDummyDrv[j][i]->getCQDescBufSize();
                    secinfo.buff_va = (uint64_t)this->pDummyDrv[j][i]->getCQDescBufVirAddr();
                    secinfo.memID = this->pDummyDrv[j][i]->getCQIonFD();

                    //Dummy vir reg.
                    secinfo.sub.buff_size = this->pDummyDrv[j][i]->getIspVirRegSize();
                    secinfo.sub.buff_va = (uint64_t)this->pDummyDrv[j][i]->getIspVirRegVirAddr();
                    secinfo.sub.memID = this->pDummyDrv[j][i]->getIspVirIonFD();


                    CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE(dummy) module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx buf_pa:0x%x memID:0x%x",
                                 secinfo.module,secinfo.cq,secinfo.dupq,secinfo.buff_size,secinfo.buff_va,
                                 this->pDummyDrv[j][i]->getCQDescBufPhyAddr(),secinfo.memID);
                    CmdQMgr_DBG("type:SECMEM_VIRTUAL_REG_TABLE(dummy) module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx buf_pa:0x%x memID:0x%x",
                                 secinfo.module,secinfo.cq,secinfo.dupq,secinfo.sub.buff_size,secinfo.sub.buff_va,
                                 this->pDummyDrv[j][i]->getIspVirRegPhyAddr(),secinfo.sub.memID);

                    if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1RegisterShareMem failed\n");
                        return NULL;
                    }


                }
            }
            else{
                CmdQMgr_ERR("security function lunch fail\n");
            }
        }
    }


    //cq page link-list
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){

        //if subsample function enabled , each burst cq link to next-cq, and the last burst cq link to dummy
        //if subsample function disabled, each duplicated cq link to dummy
        if(m_CQ0.m_bSubsample){//subsample function enabled
            for(MUINT32 i=0;i<(this->mBurstQ-1);i++){
                //normal cq
                this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][i];

                //link to next burst cq
                this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[j][i+1]->getCQDescBufPhyAddr();
                this->m_CQ0.setBaseAddr_byCQ();

                //dummy cq
                this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];

                //link to next burst cq
                this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][i+1]->getCQDescBufPhyAddr();
                this->m_CQ0.setBaseAddr_byCQ();

            }
            //last burst cq link to dummy
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][(this->mBurstQ-1)];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();
#if 0   //remove to CmdQMgr_start,because of other bit-field is configured at top_ctrl::_enalbe
            //enable sub-done
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][0];
            this->m_CQ0.setDoneSubSample(MTRUE);
#endif
            //last dummy link to 1st dummy
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][(this->mBurstQ-1)];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();
#if 0   //remove to to CmdQMgr_start,because of other bit-field is configured at top_ctrl::_enalbe
            //close sub-done to avoid hw counting during drop frame
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][0];
            this->m_CQ0.setDoneSubSample(MFALSE);
#endif


#if (TWIN_SW_P1_DONE_WROK_AROUND == 0)
            //also need to update outter pa to dummy at the very 1st burst cq.
            this->m_CQ0.m_bSubsample = MFALSE;
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][0];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();
            this->m_CQ0.write2CQ();
            this->m_CQ0.m_bSubsample = MTRUE;
#endif
        }
        else{//no subsample
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][0];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();
        }

    }


    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[0][0];
    this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[0][0]->getCQDescBufPhyAddr();
    this->m_CQ0.Init();
    this->m_CQ0.config();

    //add cqmodule
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][i];
            this->m_CQ0.write2CQ();

            //for dummy cq's link list
            if(this->m_CQ0.m_bSubsample){
                this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
                this->m_CQ0.write2CQ();
            }
        }
    }

    //
#if CQ_SW_WORK_AROUND
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
            this->m_CQ0.DummyDescriptor();
        }
    }
#endif

    //switch to immediate-mode before start_isp
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_);

    this->mFSM = op_cfg;
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x\n",this->mDupCmdQ,this->mBurstQ);
    return this;
}

MINT32 CmdQMgrImp::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow,__FUNCTION__) == MFALSE){
        return 1;
    }
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

    //free previous memory.
    if(this->pCamDrv != NULL){
        for(MUINT32 j=0;j<this->mDupCmdQ;j++){
            for(MUINT32 i=0;i<this->mBurstQ;i++){
                //
                this->pCamDrv[j][i]->uninit( (const char *) this->m_pUserName[j][i]);

                this->pDummyDrv[j][i]->uninit( (const char *) this->m_pDummyName[j][i]);
                //
                this->pCamDrv[j][i]->destroyInstance();

                this->pDummyDrv[j][i]->destroyInstance();

                free(this->m_pUserName[j][i]);
                free(this->m_pDummyName[j][i]);
            }
            free(this->pCamDrv[j]);
            free(this->m_pUserName[j]);

            free(this->pDummyDrv[j]);
            free(this->m_pDummyName[j]);
        }
        free(this->pCamDrv);
        free(this->m_pUserName);

        free(this->pDummyDrv);
        free(this->m_pDummyName);
    }



    this->pCamDrv = NULL;
    this->m_pUserName = NULL;

    this->pDummyDrv = NULL;
    this->m_pDummyName = NULL;

    this->mFSM = op_unknow;
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    return 0;
}


ISP_DRV_CAM** CmdQMgrImp::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }

    return this->pCamDrv[DupIdx];
}

MINT32 CmdQMgrImp::CmdQMgr_update(void)
{
    MUINT32 curDummyCQIdx = 0;
    MUINT32 prvDummyCQIdx = 0;
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 1;
    }

    if((this->mFSM == op_start)||(this->mFSM == op_suspend)){
        curDummyCQIdx = this->mDupCmdQIdx;
        prvDummyCQIdx = (this->mDupCmdQIdx > 0)? (this->mDupCmdQIdx - 1):(this->mDupCmdQ - 1);

        this->mDupCmdQIdx = ((this->mDupCmdQIdx+1) % this->mDupCmdQ);

        //update cq counter at new cq,at high speed mode, only 1st page have counter update
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
        this->m_CQ0.SetCQupdateCnt();

        if(this->mBurstQ > 1){

            //update the link of current last burst normal cq to new cq addr
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[curDummyCQIdx][this->mBurstQ-1];

            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();

            this->pCamDrv[curDummyCQIdx][this->mBurstQ-1]->flushCmdQ();

            //update the link of current last burst dummy cq to new cq addr
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[curDummyCQIdx][this->mBurstQ-1];

            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();

            this->pDummyDrv[curDummyCQIdx][this->mBurstQ-1]->flushCmdQ();

            //update the link of previous last burst dummy cq to its 1st dummy cq
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[prvDummyCQIdx][this->mBurstQ-1];

            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[prvDummyCQIdx][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();

            this->pDummyDrv[prvDummyCQIdx][this->mBurstQ-1]->flushCmdQ();

            //update the link of previous last burst normal cq to its 1st dummy cq
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[prvDummyCQIdx][this->mBurstQ-1];

            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[prvDummyCQIdx][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();

            this->pCamDrv[prvDummyCQIdx][this->mBurstQ-1]->flushCmdQ();

        }
    }
    else {
        //update cq counter at new cq,at high speed mode, only 1st page have counter update
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
        this->m_CQ0.SetCQupdateCnt();
    }


    //
    for(MUINT32 i=0;i<this->mBurstQ;i++)
        this->pCamDrv[this->mDupCmdQIdx][i]->flushCmdQ();


        //at high speed mode, cpu can't program to outer register directly because of
        //cam_b always have sw p1 done under twin mode, wirte to outer register will
        //break down the cq link-list.
        //side effect:
        //sw program timing will reduce to (n-1)/n * (time of p1_sof to p1_done)
#if (TWIN_SW_P1_DONE_WROK_AROUND==1)
    if(this->mBurstQ == 1)
#endif
    {
        //at normal mode , just follow orignal design:update CQ's BA phy reg directly.
        if(pSec){
            if(pSec->SecMgr_GetSecureCamStatus(this->mModule) < 1){
                this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
                this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr();
                this->m_CQ0.config();
            }
        }
    }

    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            //only migrate non-dummy here, dummy is migrated at start
            SecMgr_SecInfo secinfo;
            secinfo.module = this->mModule;
            secinfo.cq = this->mCQ;
            secinfo.dupq = this->mDupCmdQIdx;
            secinfo.dummy = 0;
            for(MUINT32 i=0;i<this->mBurstQ;i++){
                //secinfo.burstQ = i;
                secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
                secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
                CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE/SECMEM_VIRTUAL_REG_TABLE module:%d cq:%d dupq:%d",
                             secinfo.module,secinfo.cq,secinfo.dupq);
                if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                    CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                    return NULL;
                }

                CmdQMgr_DBG("CAM_CQ_THR0_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv[this->mDupCmdQIdx][0]->getPhyObj(),CAM_CQ_THR0_BASEADDR));
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }
    //this->pCamDrv[this->mDupCmdQIdx][0]->dumpCQTable();
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_update: 0x%x_0x%" PRIXPTR "",this->mDupCmdQIdx,this->m_CQ0.dma_cfg.memBuf.base_pAddr);

    return 0;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetBurstQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQ;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetDuqQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mDupCmdQ;
}


MUINT32 CmdQMgrImp::CmdQMgr_GetDuqQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }
    if( (this->mFSM == op_start) || (this->mFSM == op_suspend))
        return ((this->mDupCmdQIdx + 1) % this->mDupCmdQ);
    else
        return this->mDupCmdQIdx;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetBurstQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQIdx;
}


MINT32 CmdQMgrImp::CmdQMgr_start(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_start + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

#if SUBSAMPLE_SW_WORK_AROUND
    if(this->mBurstQ>1)
        this->CmdQMgr_specialCQCtrl();
#endif

    //
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){

        for(MUINT32 i=0;i<this->mBurstQ;i++){
            this->pCamDrv[j][i]->start();

            this->pDummyDrv[j][i]->start();

            //
            this->pDummyDrv[j][i]->flushCmdQ();

            //
            if(j==0)
                this->pCamDrv[0][i]->flushCmdQ();
#if 0
            if(pSec){
                if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
                    //only migrate dummy here,non-dummy is migrated at update each frame
                    SecMgr_SecInfo secinfo;
                    secinfo.module = this->mModule;
                    secinfo.cq = this->mCQ;
                    secinfo.dupq = j;
                    //secinfo.burstQ = i;
                    secinfo.dummy = 1;

                    secinfo.type= SECMEM_CQ_DESCRIPTOR_TABLE;
                    secinfo.sub.type= SECMEM_VIRTUAL_REG_TABLE;
                    CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE/SECMEM_VIRTUAL_REG_TABLE(dummy) module:%d cq:%d dupq:%d",
                                 secinfo.module,secinfo.cq,secinfo.dupq);
                    if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1Migrate dummy table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                        return NULL;
                    }

                    CmdQMgr_DBG("CAM_CQ_THR0_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv[0][0]->getPhyObj(),CAM_CQ_THR0_BASEADDR));
                }
            }
            else{
                CmdQMgr_ERR("security function lunch fail\n");
            }
#endif
        }
    }



    //
    this->m_CQ0.enable(NULL);

    if(this->m_CQ0.TrigCQ() == MFALSE){
        return 1;
    }

    //switch to continuous mode when isp_start
    //at twin case, slave cam using immediate mode , trig by master cam
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_continuouse_);

    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_start - module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}
MINT32 CmdQMgrImp::CmdQMgr_stop(void)
{
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_stop + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    if(this->FSM_CHK(op_stop,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            this->pCamDrv[j][i]->stop();

            this->pDummyDrv[j][i]->stop();
        }
    }

    this->mBurstQIdx = 0;
    this->mDupCmdQIdx = 0;
    //can't access hw after suspend. hw maybe occupied by other sensor.
    if(this->mFSM != op_suspend)
        this->m_CQ0.disable();

    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_stop - module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_stop;
    return 0;
}

DupCmdQMgr* CmdQMgrImp::CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq)
{
    cq;
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }

    return this;
}

MBOOL CmdQMgrImp::CmdQMgr_specialCQCtrl(void)
{
    MBOOL ret = MTRUE;
    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        //plz ref to line:295
        //close sub-done to avoid hw counting during drop frame
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][0];
        this->m_CQ0.setDoneSubSample(this->pCamDrv[0][0],MFALSE,this->mBurstQ-1);


        //enable sub-done
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][0];
        this->m_CQ0.setDoneSubSample(this->pCamDrv[0][0],MTRUE,this->mBurstQ-1);
    }

    return ret;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetCounter(MBOOL bPhy)
{
    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 0;
    }

    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];

    return this->m_CQ0.GetCQupdateCnt(bPhy);
}

ISP_HW_MODULE CmdQMgrImp::CmdQMgr_GetCurModule(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return CAM_MAX;
    }

    return this->mModule;
}

//clr descritpor on all pages
MINT32 CmdQMgrImp::CmdQMgr_suspend(void)
{
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_suspend + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    if(this->FSM_CHK(op_suspend,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_CQ0.disable();


    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_suspend - module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_suspend;
    return 0;
}

//set descritpor on all pages
MINT32 CmdQMgrImp::CmdQMgr_resume(void)
{
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_resume + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    //
    this->m_CQ0.enable(NULL);

    //switch to continuous mode when isp_start
    //at twin case, slave cam using immediate mode , trig by master cam
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_);

    if(this->m_CQ0.TrigCQ() == MFALSE){
        return 1;
    }

    //switch to continuous mode when isp_start
    //at twin case, slave cam using immediate mode , trig by master cam
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_continuouse_);

    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_resume - module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}

