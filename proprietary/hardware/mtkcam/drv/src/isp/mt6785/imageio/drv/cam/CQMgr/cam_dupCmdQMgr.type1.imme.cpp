#include "cam_dupCmdQMgr.type1.imme.h"
#include "sec_mgr.h"

MBOOL CmdQMgrImp1_imme::FSM_CHK(MUINT32 op,char const* caller)
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
                //case op_suspend: /*marked, in order make sure slave cams' dupQidx &cq_counter can sync with master cams'.*/
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


CmdQMgrImp1_imme::CmdQMgrImp1_imme()
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
    m_pMaster = NULL;

    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}

DupCmdQMgr* CmdQMgrImp1_imme::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    fps;
    CmdQMgr_INF("CmdQMgrImp1_imme::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
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
            sprintf(this->m_pUserName[j][i],"CmdQMgrImp1_imme:%d",(j*this->mBurstQ + i));

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

                    if(pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1RegisterShareMem failed\n");
                        return NULL;
                    }
                }
            }
            else{
                CmdQMgr_ERR("security function lunch fail\n");
            }

            //
#if CQ_SW_WORK_AROUND
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
            this->m_CQ0.DummyDescriptor();
#endif
        }
    }

    //no need to wirte2CQ(), because of cq address of imme mode is controlled by master cam


    //switch to immediate-mode
    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[0][0];
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_);

    this->mFSM = op_cfg;
    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x\n",this->mDupCmdQ,this->mBurstQ);
    return this;
}

MINT32 CmdQMgrImp1_imme::CmdQMgr_detach(void)
{
    CmdQMgr_INF("CmdQMgrImp1_imme::CmdQMgr_detach+: cq:0x%x\n",this->mCQ);
    if(this->FSM_CHK(op_unknow,__FUNCTION__) == MFALSE){
        return 1;
    }

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

    this->m_pMaster = NULL;

    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_detach-: cq:0x%x\n",this->mCQ);
    return 0;
}


ISP_DRV_CAM** CmdQMgrImp1_imme::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }

    return this->pCamDrv[DupIdx];
}

MINT32 CmdQMgrImp1_imme::CmdQMgr_update(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 1;
    }

    if(this->mFSM == op_start){
        this->mDupCmdQIdx = ((this->mDupCmdQIdx+1) % this->mDupCmdQ);
    }

    //update cq counter at new cq,at high speed mode, only 1st page have counter update
    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
    this->m_CQ0.SetCQupdateCnt();
    this->pCamDrv[this->mDupCmdQIdx][0]->flushCmdQ();

    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->mModule) > 0){
            SecMgr_SecInfo secinfo;
            secinfo.type= SECMEM_CQ_DESCRIPTOR_TABLE;
            secinfo.sub.type= SECMEM_VIRTUAL_REG_TABLE;
            secinfo.module = this->mModule;
            secinfo.cq = this->mCQ;
            secinfo.dupq = this->mDupCmdQIdx;
            secinfo.dummy = 0;
            for(MUINT32 i=0;i<this->mBurstQ;i++){
                //secinfo.burstQ = i;
                if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                    CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                    return NULL;
                }
            }
        }
    }
    else{
        CmdQMgr_ERR("security function lunch fail\n");
    }


    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_update: 0x%x_0x%" PRIXPTR "",this->mDupCmdQIdx,this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr());

    return 0;
}

MINT32 CmdQMgrImp1_imme::CmdQMgr_update_IMME(MUINT32 idx)
{
    CmdQMgr_WRN("idx:%d\n",idx);
    this->mDupCmdQIdx = idx;

    return 0;
}

MUINT32 CmdQMgrImp1_imme::CmdQMgr_GetBurstQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQ;
}

MUINT32 CmdQMgrImp1_imme::CmdQMgr_GetDuqQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mDupCmdQ;
}


MUINT32 CmdQMgrImp1_imme::CmdQMgr_GetDuqQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }
    if( (this->mFSM == op_start) || (this->mFSM == op_suspend))
        return ((this->mDupCmdQIdx + 1) % this->mDupCmdQ);
    else
        return this->mDupCmdQIdx;
}

MUINT32 CmdQMgrImp1_imme::CmdQMgr_GetBurstQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQIdx;
}


MINT32 CmdQMgrImp1_imme::CmdQMgr_start(void)
{
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    CmdQMgr_INF("CmdQMgrImp1_imme::CmdQMgr_start + cq:0x%x\n",this->mCQ);
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
                    secinfo.type= SECMEM_CQ_DESCRIPTOR_TABLE;
                    secinfo.sub.type= SECMEM_VIRTUAL_REG_TABLE;
                    secinfo.module = this->mModule;
                    secinfo.cq = this->mCQ;
                    secinfo.dupq = j;
                    //secinfo.burstQ = i;
                    secinfo.dummy = 1;
                    if(pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                        CmdQMgr_ERR("P1Migrate dummy table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                        return NULL;
                    }
                }
            }
            else{
                CmdQMgr_ERR("security function lunch fail\n");
            }
#endif
        }
    }



    //can't enable/trig cq here, cq is enabled by master cam



    this->mFSM = op_start;

    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_start - cq:0x%x\n",this->mCQ);
    return 0;
}

MINT32 CmdQMgrImp1_imme::CmdQMgr_stop(void)
{
    CmdQMgr_INF("CmdQMgrImp1_imme::CmdQMgr_stop + cq:0x%x\n",this->mCQ);
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
    //
    this->m_CQ0.disable();


    this->mFSM = op_stop;

    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_stop - cq:0x%x\n",this->mCQ);
    return 0;
}

MBOOL CmdQMgrImp1_imme::CmdQMgr_specialCQCtrl(void)
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

MUINT32 CmdQMgrImp1_imme::CmdQMgr_GetCounter(MBOOL bPhy)
{
    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 0;
    }

    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];

    return this->m_CQ0.GetCQupdateCnt(bPhy);
}

ISP_HW_MODULE CmdQMgrImp1_imme::CmdQMgr_GetCurModule(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return CAM_MAX;
    }

    return this->mModule;
}

MBOOL CmdQMgrImp1_imme::CmdQMgr_updateCQMode(DupCmdQMgr*  pMaster, MUINT32 targetIdxMain)
{
#if TWIN_CQ_SW_WORKAROUND
    ISP_HW_MODULE _module;
    CmdQMgrImp* pMain = (CmdQMgrImp*)pMaster;

    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return MFALSE;
    }


    //
    if(pMain == NULL){
        CmdQMgr_ERR("master cam's obj is NULL\n");
        return MFALSE;
    }
    else{
        CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_updateCQMode + master:0x%x,mDupCmdQIdx:%d\n",
            pMain->CmdQMgr_GetCurModule(), targetIdxMain);
        //
        this->m_pMaster = (CmdQMgrImp*)pMaster;
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

    for(MUINT32 i=0;i<this->mBurstQ;i++){
        //
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMaster->pDummyDrv[targetIdxMain][i];

        this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_,_module);
        this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_,(void*)&this->m_CQ0);
        this->m_CQ0.TrigCQ(MFALSE);
        this->m_CQ0.enable((void*)&this->m_CQ0);

        //
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMaster->pCamDrv[targetIdxMain][i];

        this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_, _module);
        this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_,(void*)&this->m_CQ0);
        this->m_CQ0.TrigCQ(MFALSE);
        this->m_CQ0.enable((void*)&this->m_CQ0);

        //set all cq page's ba onto master cam's cq
        //normal page
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMaster->pCamDrv[targetIdxMain][i];
        this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR) this->pCamDrv[targetIdxMain][i]->getCQDescBufPhyAddr();
        this->m_CQ0.setSlaveAddr_byCQ(_module);

        this->pCamDrv[targetIdxMain][i]->flushCmdQ();
        //dummy page
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMaster->pDummyDrv[targetIdxMain][i];
        this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR) this->pDummyDrv[targetIdxMain][i]->getCQDescBufPhyAddr();
        this->m_CQ0.setSlaveAddr_byCQ(_module);

        this->pDummyDrv[targetIdxMain][i]->flushCmdQ();
    }



    return MTRUE;
#else
    return MTRUE;
#endif
}

//clr descritpor on all pages
MBOOL CmdQMgrImp1_imme::CmdQMgr_clearCQMode(DupCmdQMgr* pMaster, MUINT32 targetIdxMain)
{
    ISP_HW_MODULE _module;
    CmdQMgrImp* pMain = (CmdQMgrImp*)pMaster;
    if(pMain == NULL){
        CmdQMgr_ERR("no master cam info\n");
        return MFALSE;
    }

    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    CmdQMgr_DBG("CmdQMgrImp1_imme::CmdQMgr_clearCQMode + cq:0x%x,master module:0x%x,Qidx:0x%x\n",this->mCQ,pMain->CmdQMgr_GetCurModule(),targetIdxMain);

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
    for(MUINT32 i=0;i<this->mBurstQ;i++){
        //
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)pMain->pCamDrv[targetIdxMain][i];
        //this->m_CQ0.disable((void*)&this->m_CQ0);

        //
        this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_, _module);

        //
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)pMain->pDummyDrv[targetIdxMain][i];
        //this->m_CQ0.disable((void*)&this->m_CQ0);

        //
        this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_, _module);
    }

    return MTRUE;
}




