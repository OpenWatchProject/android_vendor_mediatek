#define LOG_TAG "CmdQMgr"

#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"
#include "sec_mgr.h"

#include <stdlib.h>
#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;


DECLARE_DBG_LOG_VARIABLE(CmdQMgr);

// Clear previous define, use our own define.
#undef CmdQMgr_VRB
#undef CmdQMgr_DBG
#undef CmdQMgr_INF
#undef CmdQMgr_WRN
#undef CmdQMgr_ERR
#undef CmdQMgr_AST

#define CmdQMgr_VRB(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_DBG(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_INF(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_WRN(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_ERR(fmt, arg...)        do { if (CmdQMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("[0x%x]:" fmt,this->mModule, ##arg); } } while(0)
#define CmdQMgr_AST(cond, fmt, arg...)  do { if (CmdQMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)



//class for cq0 : cq running with cq link-list.
class CmdQMgrImp : public DupCmdQMgr
{
public:
    CmdQMgrImp();
    virtual ~CmdQMgrImp(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_ImmStart(void);
    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);

    virtual MINT32          CmdQMgr_suspend(void);
    virtual MINT32          CmdQMgr_resume(void);

    virtual MINT32          CmdQMgr_update(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void);
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
            DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2){arg1;arg2;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);

    virtual MBOOL           CmdQMgr_updateCQMode(void){return MFALSE;}
private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);
            MBOOL           CmdQMgr_specialCQCtrl(void);
public:
    //public for slave cam
    ISP_DRV_CAM***  pCamDrv;
    ISP_DRV_CAM***  pDummyDrv;
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam
    #define SS_TH1      (120)
    #define SS_TH2      (240)

    #define BURST(fps)  (fps>SS_TH2)? (8):( (fps>SS_TH1)?(4):(1) )

    #define DEFAULT_DUP_NUM (3)

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop,
        op_suspend,
    };
    MUINT32         m_bStart;

    MUINT32         mBurstQ;
    MUINT32         mDupCmdQ;

    MUINT32         mBurstQIdx;
    MUINT32         mDupCmdQIdx;

    ISP_HW_MODULE   mModule;
    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMA_CQ0         m_CQ0;

    char***         m_pUserName;
    char***         m_pDummyName;
};

/**
    Class of CmdQMgrImp
*/
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
                case op_suspend:
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
    m_bStart = 0;
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    mDupCmdQ = 0;
    mBurstQIdx = 0;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    pDummyDrv = NULL;
    m_pDummyName = NULL;

    DBG_LOG_CONFIG(imageio, CmdQMgr);
}

DupCmdQMgr* CmdQMgrImp::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(this->FSM_CHK(op_cfg,__FUNCTION__) == MFALSE){
        return NULL;
    }
    this->mModule = module;
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
    //
    switch(cq){
        case ISP_DRV_CQ_THRE0:
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }
    if(fps == 0){
        CmdQMgr_ERR("fps can't be 0\n");
        fps+=1;
    }
    //new setting
    this->mBurstQ = subsample + 1;//BURST(fps);   // via user self-define, not drv self-define
    if(this->mBurstQ >= DupCmdQMgr::REV_CQ){
        CmdQMgr_ERR("BurstQ is out of range:max:0x%x,cur:0x%x\n",DupCmdQMgr::REV_CQ-1,this->mBurstQ);
        return NULL;
    }
    //this->mModule = module;
    this->mCQ = cq;
    this->mDupCmdQ = DEFAULT_DUP_NUM;



    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //malloc & create & init isp drv
    this->pCamDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);
    this->pDummyDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);

    this->m_pUserName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);
    this->m_pDummyName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);

    if (this->pCamDrv == NULL || this->pDummyDrv == NULL ||
            this->m_pUserName == NULL || this->m_pDummyName == NULL) {
        CmdQMgr_ERR("malloc error\n");
        return NULL;
    }

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
            // for security
            if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
                //
                SecMgr_SecInfo secinfo;
                SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

                secinfo.module = this->mModule;
                secinfo.cq = this->mCQ;
                secinfo.dupq = j;
                secinfo.dummy = 0;

                // descriptor
                secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
                secinfo.buff_size = this->pCamDrv[j][i]->getCQDescBufSize();
                secinfo.buff_va = (uint64_t)this->pCamDrv[j][i]->getCQDescBufVirAddr();
                secinfo.memID = this->pCamDrv[j][i]->getCQIonFD();

                // vir reg.
                secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
                secinfo.sub.buff_size = this->pCamDrv[j][i]->getIspVirRegSize();
                secinfo.sub.buff_va = (uint64_t)this->pCamDrv[j][i]->getIspVirRegVirAddr();
                secinfo.sub.memID = this->pCamDrv[j][i]->getIspVirIonFD();

                CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx memID:0x%x",
                             secinfo.module,secinfo.cq,secinfo.dupq,secinfo.buff_size,secinfo.buff_va,secinfo.memID);
                CmdQMgr_DBG("type:SECMEM_VIRTUAL_REG_TABLE module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx memID:0x%x",
                             secinfo.module,secinfo.cq,secinfo.dupq,secinfo.sub.buff_size,secinfo.sub.buff_va,secinfo.sub.memID);

                if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
                    CmdQMgr_ERR("P1RegisterShareMem failed\n");
                    return NULL;
    }

                // Dummy descriptor
                secinfo.dummy = 1;
                secinfo.buff_size = this->pDummyDrv[j][i]->getCQDescBufSize();
                secinfo.buff_va = (uint64_t)this->pDummyDrv[j][i]->getCQDescBufVirAddr();
                secinfo.memID = this->pDummyDrv[j][i]->getCQIonFD();

                // Dummy vir reg.
                secinfo.sub.buff_size = this->pDummyDrv[j][i]->getIspVirRegSize();
                secinfo.sub.buff_va = (uint64_t)this->pDummyDrv[j][i]->getIspVirRegVirAddr();
                secinfo.sub.memID = this->pDummyDrv[j][i]->getIspVirIonFD();

                CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE(dummy) module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx buf_pa:0x%x memID:0x%x",
                             secinfo.module,secinfo.cq,secinfo.dupq,secinfo.buff_size,secinfo.buff_va,
                             this->pDummyDrv[j][i]->getCQDescBufPhyAddr(),secinfo.memID);
                CmdQMgr_DBG("type:SECMEM_VIRTUAL_REG_TABLE(dummy) module:%d cq:%d dupq:%d buf_size:0x%x buf_va:0x%llx buf_pa:0x%x memID:0x%x",
                             secinfo.module,secinfo.cq,secinfo.dupq,secinfo.sub.buff_size,secinfo.sub.buff_va,
                             this->pDummyDrv[j][i]->getIspVirRegPhyAddr(),secinfo.sub.memID);

                if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
                    CmdQMgr_ERR("P1RegisterShareMem failed\n");
                    return NULL;
                }

            }
        }
    }

    //CQI config
    this->m_CQ0.m_bSubsample = (subsample >0)? (MTRUE):(MFALSE);
    //cq page link-list
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){

        //if subsample function enabled , each burst cq link to next-cq, and the last burst cq link to dummy
        //if subsample function disabled, each duplicated cq link to dummy
        if(this->mBurstQ > 1){//subsample function enabled
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
            //also need to update outer pa to dummy at the very 1st burst cq.
            this->m_CQ0.m_bSubsample = MFALSE;
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][0];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][0]->getCQDescBufPhyAddr();
            this->m_CQ0.setBaseAddr_byCQ();
            this->m_CQ0.write2CQ();
            this->m_CQ0.m_bSubsample = (subsample >0)? (MTRUE):(MFALSE);
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
    this->m_CQ0.config();

    //add cqmodule
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][i];
            this->m_CQ0.write2CQ();

            //for dummy cq's link list
            if(this->mBurstQ > 1){
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
    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x\n",this->mDupCmdQ,this->mBurstQ);
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


    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 1;
    }

    if(m_bStart){
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
    for (MUINT32 i = 0; i < this->mBurstQ; i++) {
        this->pCamDrv[this->mDupCmdQIdx][i]->flushCmdQ();
    }

        //at high speed mode, cpu can't program to outer register directly because of
        //cam_b always have sw p1 done under twin mode, wirte to outer register will
        //break down the cq link-list.
        //side effect:
        //sw program timing will reduce to (n-1)/n * (time of p1_sof to p1_done)
#if TWIN_SW_P1_DONE_WROK_AROUND
    if(this->mBurstQ == 1)
#endif
    {
        //at normal mode , just follow orignal design:update CQ's BA phy reg directly.
        if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) < 1) {
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr =
                 (MUINTPTR)this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr();
            this->m_CQ0.config();
        }
    }

    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        //only migrate non-dummy here, dummy is migrated at start
        SecMgr_SecInfo secinfo;
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dupq = this->mDupCmdQIdx;
        secinfo.dummy = 0;
        for (MUINT32 i = 0; i < this->mBurstQ; i++) {
            //secinfo.burstQ = i;
            secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
            secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
            CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE/SECMEM_VIRTUAL_REG_TABLE module:%d cq:%d dupq:%d",
                         secinfo.module,secinfo.cq,secinfo.dupq);
            if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                return NULL;
            }

            CmdQMgr_DBG("CAM_CQ_THR0_BASEADDR:0x%x",CAM_READ_REG(this->pCamDrv[this->mDupCmdQIdx][0]->getPhyObj(),CAM_CQ_THR0_BASEADDR));
        }
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
    }
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

    return this->mDupCmdQIdx;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetBurstQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQIdx;
}


MINT32 CmdQMgrImp::CmdQMgr_ImmStart(void)
{
    switch (this->mCQ) {
        case ISP_DRV_CQ_THRE0:
        break;
    default:
        CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
        return 1;
    }

    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_);

    this->m_CQ0.enable(NULL);

    if (this->m_CQ0.TrigCQ(MTRUE) == MFALSE) {
        return 1;
    }

    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_continuouse_);

    if (this->mBurstQ > 1) {//???
        this->m_CQ0.setLoadMode(DMA_CQ0::_inner_, MTRUE);
    }
    else {
        this->m_CQ0.setLoadMode(DMA_CQ0::_outer_, MTRUE);
    }

    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_ImmStart + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    return 0;
}

MINT32 CmdQMgrImp::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_bStart = MTRUE;

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
        }
    }



    //
    this->m_CQ0.enable(NULL);

    if(this->m_CQ0.TrigCQ() == MFALSE){
        return 1;
    }

    //switch to continuous mode when isp_start
    //at twin case, slave cam using immediate mode , trig by master cam
    switch(this->mCQ){
        case ISP_DRV_CQ_THRE0:
            this->m_CQ0.setCQTriggerMode(DMA_CQ0::_continuouse_);
            break;
        default:
            CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
            return 1;
            break;
    }

    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_start + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}
MINT32 CmdQMgrImp::CmdQMgr_stop(void)
{
    if(this->FSM_CHK(op_stop,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_bStart = MFALSE;
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

    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_stop + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_stop;
    return 0;
}

MBOOL CmdQMgrImp::CmdQMgr_GetCurStatus(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    return this->m_bStart;
}
DupCmdQMgr* CmdQMgrImp::CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq)
{
    switch(cq){
        case ISP_DRV_CQ_THRE0:
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }

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
    return this->mModule;
}

//clr descritpor on all pages
MINT32 CmdQMgrImp::CmdQMgr_suspend(void)
{
    if(this->FSM_CHK(op_suspend,__FUNCTION__) == MFALSE){
        return 1;
    }


    //
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][i];
            //this->m_CQ0.disable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_);

            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
            //this->m_CQ0.disable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_);
        }
    }


    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_suspend + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_suspend;
    return 0;
}

//set descritpor on all pages
MINT32 CmdQMgrImp::CmdQMgr_resume(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[j][i];
            //this->m_CQ0.enable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);

            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
            //this->m_CQ0.enable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);
        }
    }

    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_resume + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}


#if 0
#endif

//class for slave cam only , cq will not be imp. by link-list. Bassaddress will be controlled by master cam
class CmdQMgrImp4 : public DupCmdQMgr
{
public:
    CmdQMgrImp4();
    virtual ~CmdQMgrImp4(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq,CmdQMgrImp*  pMasterCam);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);

    virtual MINT32          CmdQMgr_suspend(void);
    virtual MINT32          CmdQMgr_resume(void);

    virtual MINT32          CmdQMgr_update(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void);
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
            DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2){arg1;arg2;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);

    virtual MBOOL           CmdQMgr_updateCQMode(void);
private:
            MBOOL           FSM_CHK(MUINT32 op,char const* caller);
            MBOOL           CmdQMgr_specialCQCtrl(void);
public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam
    #define SS_TH1      (120)
    #define SS_TH2      (240)

    #define BURST(fps)  (fps>SS_TH2)? (8):( (fps>SS_TH1)?(4):(1) )

    #define DEFAULT_DUP_NUM (3)

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop,
        op_suspend,
    };
    MUINT32         m_bStart;

    MUINT32         mBurstQ;
    MUINT32         mDupCmdQ;

    MUINT32         mBurstQIdx;
    MUINT32         mDupCmdQIdx;

    ISP_HW_MODULE   mModule;
    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMA_CQ0         m_CQ0;
    ISP_DRV_CAM***  pCamDrv;

    ISP_DRV_CAM***  pDummyDrv;

    char***         m_pUserName;
    char***         m_pDummyName;

    CmdQMgrImp*     m_pMasterCam;
};

/**
    Class of CmdQMgrImp4
*/
MBOOL CmdQMgrImp4::FSM_CHK(MUINT32 op,char const* caller)
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


CmdQMgrImp4::CmdQMgrImp4()
{
    mBurstQ = 0;
    mDupCmdQIdx = 0;
    m_bStart = 0;
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    mDupCmdQ = 0;
    mBurstQIdx = 0;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    pDummyDrv = NULL;
    m_pDummyName = NULL;
    m_pMasterCam = NULL;

    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}

DupCmdQMgr* CmdQMgrImp4::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq,CmdQMgrImp*  pMasterCam)
{
    if(this->FSM_CHK(op_cfg,__FUNCTION__) == MFALSE){
        return NULL;
    }
    this->mModule = module;
    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
    //
    if(this->mModule != CAM_A_TWIN){
        CmdQMgr_ERR("only slave cam r supported\n");
        return NULL;
    }
    //
    switch(cq){
        case ISP_DRV_CQ_THRE0:
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }
    //
    if((pMasterCam == NULL)|| (pMasterCam->pCamDrv == NULL)||(pMasterCam->pDummyDrv == NULL)){
        CmdQMgr_ERR("master cam's obj is NULL\n");
        return NULL;
    }
    else{
        this->m_pMasterCam = pMasterCam;
    }

    if(fps == 0){
        CmdQMgr_ERR("fps can't be 0\n");
        fps+=1;
    }
    //new setting
    this->mBurstQ = subsample + 1;//BURST(fps);   // via user self-define, not drv self-define
    if(this->mBurstQ >= DupCmdQMgr::REV_CQ){
        CmdQMgr_ERR("BurstQ is out of range:max:0x%x,cur:0x%x\n",DupCmdQMgr::REV_CQ-1,this->mBurstQ);
        return NULL;
    }
    //this->mModule = module;
    this->mCQ = cq;
    this->mDupCmdQ = DEFAULT_DUP_NUM;



    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp4::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //malloc & create & init isp drv
    this->pCamDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);
    this->pDummyDrv = (ISP_DRV_CAM***)malloc(sizeof(ISP_DRV_CAM**)*this->mDupCmdQ);

    this->m_pUserName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);
    this->m_pDummyName = (char***)malloc(sizeof(char**)*this->mDupCmdQ);

    if (this->pCamDrv == NULL || this->pDummyDrv == NULL ||
            this->m_pUserName == NULL || this->m_pDummyName == NULL) {
        CmdQMgr_ERR("malloc error\n");
        return NULL;
    }

    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        this->pCamDrv[j] = (ISP_DRV_CAM**)malloc(sizeof(ISP_DRV_CAM*)*this->mBurstQ);
        this->pDummyDrv[j] = (ISP_DRV_CAM**)malloc(sizeof(ISP_DRV_CAM*)*this->mBurstQ);

        this->m_pUserName[j] = (char**)malloc(sizeof(char*)*this->mBurstQ);
        this->m_pDummyName[j] = (char**)malloc(sizeof(char*)*this->mBurstQ);
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_pUserName[j][i] = (char*)malloc(sizeof(char)*STR_LENG);

            this->m_pUserName[j][i][0] = '\0';
            sprintf(this->m_pUserName[j][i],"CmdQMgrImp4:%d",(j*this->mBurstQ + i));

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
            // for security
            if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
                //
                SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
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

                if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
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

                if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
                    CmdQMgr_ERR("P1RegisterShareMem failed\n");
                    return NULL;
                }
            } else {
                CmdQMgr_DBG("security function isn't lunched\n");
            }
        }
    }

    //CQI config
    this->m_CQ0.m_bSubsample = (subsample >0)? (MTRUE):(MFALSE);

    //set all cq page's ba onto master cam's cq
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        //
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //normal page
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pCamDrv[j][i];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[j][i]->getCQDescBufPhyAddr();
            this->m_CQ0.setSlaveAddr_byCQ();

            this->m_pMasterCam->pCamDrv[j][i]->flushCmdQ();

            //dummy page
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pDummyDrv[j][i];
            this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pDummyDrv[j][i]->getCQDescBufPhyAddr();
            this->m_CQ0.setSlaveAddr_byCQ();

#if CQ_SW_WORK_AROUND
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pDummyDrv[j][i];
            this->m_CQ0.DummyDescriptor();
#endif

            this->m_pMasterCam->pDummyDrv[j][i]->flushCmdQ();
        }

    }


    //no need to wirte2CQ(), because of address is controlled by master cam


    //switch to immediate-mode
    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[0][0];
    this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_);

    this->mFSM = op_cfg;
    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x\n",this->mDupCmdQ,this->mBurstQ);
    return this;
}

MINT32 CmdQMgrImp4::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow,__FUNCTION__) == MFALSE){
        return 1;
    }
    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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

    this->m_pMasterCam = NULL;
    return 0;
}


ISP_DRV_CAM** CmdQMgrImp4::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }

    return this->pCamDrv[DupIdx];
}

MINT32 CmdQMgrImp4::CmdQMgr_update(void)
{
    if(this->FSM_CHK(op_update,__FUNCTION__) == MFALSE){
        return 1;
    }

    if(m_bStart){
        this->mDupCmdQIdx = ((this->mDupCmdQIdx+1) % this->mDupCmdQ);
    }

    //update cq counter at new cq,at high speed mode, only 1st page have counter update
    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
    this->m_CQ0.SetCQupdateCnt();
    this->pCamDrv[this->mDupCmdQIdx][0]->flushCmdQ();

    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
        SecMgr_SecInfo secinfo;

        secinfo.type= SECMEM_CQ_DESCRIPTOR_TABLE;
        secinfo.sub.type= SECMEM_VIRTUAL_REG_TABLE;
        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dupq = this->mDupCmdQIdx;
        secinfo.dummy = 0;
        for (MUINT32 i = 0; i < this->mBurstQ; i++) {
            //secinfo.burstQ = i;
            if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",this->mDupCmdQIdx,i);
                return NULL;
            }
        }
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
    }

    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_update: 0x%x_0x%p",this->mDupCmdQIdx,this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr());

    return 0;
}

MUINT32 CmdQMgrImp4::CmdQMgr_GetBurstQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQ;
}

MUINT32 CmdQMgrImp4::CmdQMgr_GetDuqQ(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mDupCmdQ;
}


MUINT32 CmdQMgrImp4::CmdQMgr_GetDuqQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mDupCmdQIdx;
}

MUINT32 CmdQMgrImp4::CmdQMgr_GetBurstQIdx(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return 0;
    }

    return this->mBurstQIdx;
}


MINT32 CmdQMgrImp4::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_bStart = MTRUE;

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
        }
    }



    //can't enable/trig cq here, cq is enabled by master cam


    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_start + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}

MINT32 CmdQMgrImp4::CmdQMgr_stop(void)
{
    if(this->FSM_CHK(op_stop,__FUNCTION__) == MFALSE){
        return 1;
    }

    this->m_bStart = MFALSE;
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

    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_stop + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_stop;
    return 0;
}

MBOOL CmdQMgrImp4::CmdQMgr_GetCurStatus(void)
{
    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return MFALSE;
    }

    return this->m_bStart;
}
DupCmdQMgr* CmdQMgrImp4::CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq)
{
    switch(cq){
        case ISP_DRV_CQ_THRE0:
            break;
        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }

    if(this->FSM_CHK(op_getinfo,__FUNCTION__) == MFALSE){
        return NULL;
    }

    return this;
}

MBOOL CmdQMgrImp4::CmdQMgr_specialCQCtrl(void)
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

ISP_HW_MODULE CmdQMgrImp4::CmdQMgr_GetCurModule(void)
{
    return this->mModule;
}

//this func imp. here,not in twinmgr is because that only cqmgr have the info. of dummy CQ
MBOOL CmdQMgrImp4::CmdQMgr_updateCQMode(void)
{
#if TWIN_CQ_SW_WORKAROUND
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return MFALSE;
    }


    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pDummyDrv[j][i];

            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);
            this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_,(void*)&this->m_CQ0);
            this->m_CQ0.TrigCQ(MFALSE);
            this->m_CQ0.enable((void*)&this->m_CQ0);

            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pCamDrv[j][i];

            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);
            this->m_CQ0.setCQTriggerMode(DMA_CQ0::_immediate_,(void*)&this->m_CQ0);
            this->m_CQ0.TrigCQ(MFALSE);
            this->m_CQ0.enable((void*)&this->m_CQ0);

        }
    }

    return MTRUE;
#else
    return MTRUE;
#endif
}

//clr descritpor on all pages
MINT32 CmdQMgrImp4::CmdQMgr_suspend(void)
{
    if(this->FSM_CHK(op_suspend,__FUNCTION__) == MFALSE){
        return 1;
    }


    //
    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pCamDrv[j][i];
            //this->m_CQ0.disable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_);

            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pDummyDrv[j][i];
            //this->m_CQ0.disable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_delete_);
        }
    }


    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_suspend + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_suspend;
    return 0;
}

//set descritpor on all pages
MINT32 CmdQMgrImp4::CmdQMgr_resume(void)
{
    if(this->FSM_CHK(op_start,__FUNCTION__) == MFALSE){
        return 1;
    }

    for(MUINT32 j=0;j<this->mDupCmdQ;j++){
        for(MUINT32 i=0;i<this->mBurstQ;i++){
            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pCamDrv[j][i];
            //this->m_CQ0.enable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);

            //
            this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->m_pMasterCam->pDummyDrv[j][i];
            //this->m_CQ0.enable((void*)&this->m_CQ0);

            //
            this->m_CQ0.IO2CQ(DMA_CQ0::_cq_add_);
        }
    }

    CmdQMgr_INF("CmdQMgrImp4::CmdQMgr_resume + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}

//class for cq4/5/7/8 : cq running with lagecy cq0c mode
class CmdQMgrImp2 : public DupCmdQMgr
{
public:
    CmdQMgrImp2();
    virtual ~CmdQMgrImp2(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
    virtual DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

    virtual MINT32          CmdQMgr_suspend(void){CmdQMgr_ERR("\n");return 1;}
    virtual MINT32          CmdQMgr_resume(void){CmdQMgr_ERR("\n");return 1;}

    virtual MINT32          CmdQMgr_update(void){CmdQMgr_ERR("\n");return 1;}


    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 1;}


    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}
    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2){arg1;arg2;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);

    virtual MBOOL           CmdQMgr_updateCQMode(void){CmdQMgr_ERR("\n");return MFALSE;}
private:
            MBOOL           FSM_CHK(MUINT32 op);

public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    #define DEFAULT_DUP_NUM2 (1)
    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop,
    };
    MUINT32         m_bStart;

    MUINT32         mBurstQ;
    MUINT32         mDupCmdQ;

    ISP_HW_MODULE   mModule;
    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;

    DMAI_B*         m_pCQ;

    ISP_DRV_CAM*    pCamDrv;

    char*           m_pUserName;
};

/**
    Class of CmdQMgrImp2
*/
MBOOL CmdQMgrImp2::FSM_CHK(MUINT32 op)
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
        CmdQMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x:\n",this->mFSM,op);

    return ret;
}


CmdQMgrImp2::CmdQMgrImp2()
{
    mBurstQ = 0;
    m_bStart = 0;
    pCamDrv = NULL;
    m_pUserName = NULL;
    mFSM = op_unknow;
    m_pCQ = NULL;
    mDupCmdQ = 0;
    mModule = CAM_MAX;
    mCQ = ISP_DRV_CQ_NONE;
    //mark , already defined in constructor of CmdQMgrImp
    //DBG_LOG_CONFIG(imageio, CmdQMgr);
}

DupCmdQMgr* CmdQMgrImp2::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(this->FSM_CHK(op_cfg) == MFALSE){
        return NULL;
    }
    this->mModule = module;
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);
    //
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

    //new setting
    this->mBurstQ = 1;//subsample + 1;//BURST(fps);   // via user self-define, not drv self-define

    if(this->mBurstQ >= DupCmdQMgr::REV_CQ){
        CmdQMgr_ERR("BurstQ is out of range:max:0x%x,cur:0x%x\n",DupCmdQMgr::REV_CQ-1,this->mBurstQ);
        return NULL;
    }
    //this->mModule = module;
    this->mCQ = cq;
    this->mDupCmdQ = DEFAULT_DUP_NUM2;


    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp2::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //create & init isp drv
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);
    if (this->m_pUserName == NULL) {
        CmdQMgr_ERR("malloc err");
        return NULL;
    }
    this->m_pUserName[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp2:%d",this->mCQ);
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,0,(const char*)this->m_pUserName);
    //
    this->pCamDrv->init((const char*)this->m_pUserName);

    // for security
    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        //
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
        SecMgr_SecInfo secinfo;
        // descriptor
        secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dupq = 0;
        // secinfo.burstQ = this->CmdQMgr_GetBurstQ();
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
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
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
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x,CQ_PA:0x%x,\n",fps,module,cq);
    return this;
}

MINT32 CmdQMgrImp2::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }
    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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
    return 0;
}

ISP_DRV_CAM** CmdQMgrImp2::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    (void)DupIdx;
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }
    return &this->pCamDrv;
}

MUINT32 CmdQMgrImp2::CmdQMgr_GetBurstQ(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mBurstQ;
}

MUINT32 CmdQMgrImp2::CmdQMgr_GetDuqQ(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mDupCmdQ;
}


MINT32 CmdQMgrImp2::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }

    this->m_bStart = MTRUE;
    //
    this->pCamDrv->start();
    //
    this->pCamDrv->flushCmdQ();

    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
        SecMgr_SecInfo secinfo;

        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dummy = 0;
        for (MUINT32 j = 0; j < this->CmdQMgr_GetDuqQ(); j++) {

            secinfo.dupq = j;
            for (MUINT32 i = 0; i < this->CmdQMgr_GetBurstQ(); i++) {

                //secinfo.burstQ = i;
                secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
                CmdQMgr_DBG("type:SECMEM_CQ_DESCRIPTOR_TABLE module:%d cq:%d dupq:%d",secinfo.module,secinfo.cq,secinfo.dupq);
                if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                    CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n",j,i);
                    return NULL;
                }

                switch (this->mCQ) {
                    case ISP_DRV_CQ_THRE4:
                         CmdQMgr_DBG("CAM_CQ_THR4_BASEADDR:0x%x", CAM_READ_REG(this->pCamDrv->getPhyObj(), CAM_CQ_THR4_BASEADDR));
                         break;
                    case ISP_DRV_CQ_THRE5:
                         CmdQMgr_DBG("CAM_CQ_THR5_BASEADDR:0x%x", CAM_READ_REG(this->pCamDrv->getPhyObj(), CAM_CQ_THR5_BASEADDR));
                         break;
                    case ISP_DRV_CQ_THRE7:
                         CmdQMgr_DBG("CAM_CQ_THR7_BASEADDR:0x%x", CAM_READ_REG(this->pCamDrv->getPhyObj(), CAM_CQ_THR7_BASEADDR));
                         break;
                    case ISP_DRV_CQ_THRE8:
                         CmdQMgr_DBG("CAM_CQ_THR8_BASEADDR:0x%x", CAM_READ_REG(this->pCamDrv->getPhyObj(), CAM_CQ_THR8_BASEADDR));
                         break;
                    case ISP_DRV_CQ_THRE12:
                         CmdQMgr_DBG("CAM_CQ_THR12_BASEADDR:0x%x", CAM_READ_REG(this->pCamDrv->getPhyObj(), CAM_CQ_THR12_BASEADDR));
                         break;
                    default:
                         CmdQMgr_ERR("unsupported cq:0x%x\n",this->mCQ);
                         break;
                }
            }
        }
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
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

    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_start + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_start;
    return 0;
}
MINT32 CmdQMgrImp2::CmdQMgr_stop(void)
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    this->m_bStart = MFALSE;
    //
    this->pCamDrv->stop();

    //
    this->m_pCQ->disable();

    CmdQMgr_INF("CmdQMgrImp2::CmdQMgr_stop + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    this->mFSM = op_stop;
    return 0;
}

MBOOL CmdQMgrImp2::CmdQMgr_GetCurStatus(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return MFALSE;
    }

    return this->m_bStart;
}

DupCmdQMgr* CmdQMgrImp2::CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq)
{
    switch(cq){
        case ISP_DRV_CQ_THRE4:
        case ISP_DRV_CQ_THRE5:
        case ISP_DRV_CQ_THRE7:
        case ISP_DRV_CQ_THRE8:
        case ISP_DRV_CQ_THRE12:
            break;

        default:
            CmdQMgr_ERR("SUPPORTE NO CQ:0x%x\n",cq);
            return NULL;
            break;
    }

    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }

    return this;
}

ISP_HW_MODULE CmdQMgrImp2::CmdQMgr_GetCurModule(void)
{
    return this->mModule;
}


class CmdQMgrImp3 : public DupCmdQMgr
{
public:
    CmdQMgrImp3();
    virtual ~CmdQMgrImp3(){}

    virtual DupCmdQMgr*     CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq);
    virtual MINT32          CmdQMgr_detach(void);

    virtual MINT32          CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2);
    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);


    virtual MINT32          CmdQMgr_suspend(void){CmdQMgr_ERR("\n");return 1;}
    virtual MINT32          CmdQMgr_resume(void){CmdQMgr_ERR("\n");return 1;}

    virtual MINT32          CmdQMgr_update(void){CmdQMgr_ERR("\n");return 1;}

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx){DupIdx;CmdQMgr_ERR("\n");return NULL;}

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 1;}

    virtual MUINT32         CmdQMgr_GetBurstQ(void){return 1;}
    virtual MUINT32         CmdQMgr_GetDuqQ(void){return 1;}

    virtual MBOOL           CmdQMgr_GetCurStatus(void){CmdQMgr_ERR("\n");return MFALSE;}


    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE){bPhy;CmdQMgr_ERR("\n");return 1;}

    virtual ISP_HW_MODULE   CmdQMgr_GetCurModule(void);

    virtual MBOOL           CmdQMgr_updateCQMode(void){CmdQMgr_ERR("\n");return MFALSE;}
private:
            MBOOL           FSM_CHK(MUINT32 op);

public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
    };

    ISP_HW_MODULE   mModule;
    E_ISP_CAM_CQ    mCQ;

    MUINT32         mFSM;


    DMA_CQ10        m_CQ10;//use CQ10 to cfg CQ11 inner BA


    DMAI_B*         m_pCQ;

    ISP_DRV_CAM*    pCamDrv;//for CQ11
    ISP_DRV_CAM*    pCQ10Drv;    //for CQ10

    char*           m_pUserName;//for CQ11
    char*           m_pCQ10Name;//for CQ10
};

MBOOL CmdQMgrImp3::FSM_CHK(MUINT32 op)
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
        CmdQMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x:\n",this->mFSM,op);

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

DupCmdQMgr* CmdQMgrImp3::CmdQMgr_attach_imp(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(this->FSM_CHK(op_cfg) == MFALSE){
        return NULL;
    }
    this->mModule = module;
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_attach+: fps:%d,subsample:%d,module:0x%x,cq:%d\n",fps,subsample,module,cq);


    //
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
    if(fps == 0){
        CmdQMgr_ERR("fps can't be 0\n");
        fps+=1;
    }

    //new setting
    //this->mModule = module;
    this->mCQ = cq;


    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp3::CmdQMgr_attach: memory leakage:0x%p\n",this->pCamDrv);

    //create & init isp drv
    //
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);
    this->m_pCQ10Name = (char*)malloc(sizeof(char)*STR_LENG);

    if (this->m_pUserName == NULL || this->m_pCQ10Name == NULL) {
        CmdQMgr_ERR("malloc error\n");
        return NULL;
    }

    this->m_pUserName[0] = '\0';
    this->m_pCQ10Name[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp3:%d",this->mCQ);



    //
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,\
    0,(const char*)this->m_pUserName);

    //
    this->pCamDrv->init((const char*)this->m_pUserName);

    // for security
    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        //
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
        SecMgr_SecInfo secinfo;
        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dupq = this->CmdQMgr_GetDuqQ();
        // secinfo.burstQ = this->CmdQMgr_GetBurstQ();
        secinfo.dummy = 0;

        // descriptor
        secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
        secinfo.buff_size = this->pCamDrv->getCQDescBufSize();
        secinfo.buff_va = (uint64_t)this->pCamDrv->getCQDescBufVirAddr();
        secinfo.memID = this->pCamDrv->getCQIonFD();

        // vir reg.
        secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;
        secinfo.sub.buff_size = this->pCamDrv->getIspVirRegSize();
        secinfo.sub.buff_va = (uint64_t)this->pCamDrv->getIspVirRegVirAddr();
        secinfo.sub.memID = this->pCamDrv->getIspVirIonFD();

        if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
            CmdQMgr_ERR("P1RegisterShareMem failed\n");
            return NULL;
        }
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
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
            sprintf(this->m_pCQ10Name,"CmdQMgrImp3:%d",ISP_DRV_CQ_THRE10);
            //
            this->pCQ10Drv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,ISP_DRV_CQ_THRE10,\
                0,(const char*)this->m_pCQ10Name);
            //
            this->pCQ10Drv->init((const char*)this->m_pCQ10Name);

            //CQI config
            //use CQ10 to load CQ11 inner & outter address.
            //because CPU can't program inner address when using device tree.
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
            ((DMA_CQ11*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ11::_event_trig);
            this->m_CQ10.setCQTriggerMode(DMA_CQ10::_immediate_);


            this->pCQ10Drv->flushCmdQ();

            this->m_CQ10.enable(NULL);

            if (this->m_CQ10.TrigCQ() == MFALSE) {
                return NULL;
            }
            while (CAM_READ_REG(this->pCamDrv->getPhyObj(),CAM_CQ_THR11_BASEADDR) != ((MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr())){
                CmdQMgr_INF("wait CQ10 load done!\n");
            }
            break;
        default:
            CmdQMgr_ERR("unsupported CQ:0x%x\n",this->mCQ);
            return NULL;
            break;
    }

    this->mFSM = op_cfg;
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_attach-: \n");
    return this;
}

MINT32 CmdQMgrImp3::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }
    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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
    return 0;
}

MINT32 CmdQMgrImp3::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }
    //
    this->pCamDrv->flushCmdQ();

    if (SecMgr::SecMgr_GetSecureCamStatus(this->mModule) > 0) {
        SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
        SecMgr_SecInfo secinfo;

        secinfo.module = this->mModule;
        secinfo.cq = this->mCQ;
        secinfo.dummy = 0;
        for (MUINT32 j = 0; j < this->CmdQMgr_GetDuqQ(); j++) {
            secinfo.dupq = j;
            for (MUINT32 i = 0; i < this->CmdQMgr_GetBurstQ(); i++) {
                //secinfo.burstQ = i;
                secinfo.type = SECMEM_CQ_DESCRIPTOR_TABLE;
                secinfo.sub.type = SECMEM_VIRTUAL_REG_TABLE;

                if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                    CmdQMgr_ERR("P1Migrate Table failed(0x%x_0x%x)\n", j, i);
                    return NULL;
                }
            }
        }
    } else {
        CmdQMgr_DBG("security function isn't lunched\n");
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

    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_start + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

    return 0;
}

MINT32 CmdQMgrImp3::CmdQMgr_stop(void)
{

    this->m_pCQ->disable();
    //
    if(this->mCQ == ISP_DRV_CQ_THRE11)
        this->m_CQ10.disable();


    CmdQMgr_INF("CmdQMgrImp3::CmdQMgr_stop + module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);
    return 0;
}

MINT32 CmdQMgrImp3::CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2)
{
    if(this->FSM_CHK(op_start) == MFALSE){
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
    return this->mModule;
}

typedef struct{
    CmdQMgrImp  CQ_TYPE1;                           //support only cq0 currentlly
    CmdQMgrImp2 CQ_TYPE2[ISP_DRV_CAM_BASIC_CQ_NUM]; //support cq4/5/7/8
    CmdQMgrImp3 CQ_TYPE3[2];                        //cq1,cq11
    CmdQMgrImp4 CQ_TYPE4;                           //support only cq0 at cam_a_twin only
}ST_CmdQ;

static ST_CmdQ gCmdQMgr[CAM_MAX];

/**
    Class of DupCmdQMgr
*/
DupCmdQMgr* DupCmdQMgr::CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(module >= CAM_MAX){
        BASE_LOG_ERR("unsupported module:0x%x\n",module);
        return NULL;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:
            switch(module){
                case CAM_A_TWIN:
                    if(gCmdQMgr[module].CQ_TYPE4.CmdQMgr_attach_imp(fps,subsample,module,cq,&gCmdQMgr[CAM_A].CQ_TYPE1)){
                        return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE4;
                    }
                    else
                        return NULL;
                    break;
                default:
                    if(gCmdQMgr[module].CQ_TYPE1.CmdQMgr_attach_imp(fps,subsample,module,cq)){
                        return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE1;
                    }
                    else
                        return NULL;
                    break;
            }
            break;
        case ISP_DRV_CQ_THRE1:
            if(gCmdQMgr[module].CQ_TYPE3[0].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE3[0];
            }
            else
                return NULL;

            break;
        case ISP_DRV_CQ_THRE11:
            if(gCmdQMgr[module].CQ_TYPE3[1].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE3[1];
            }
            else
                return NULL;

            break;
        default:
            if(gCmdQMgr[module].CQ_TYPE2[cq].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE2[cq];
            }
            else
                return NULL;
            break;
    }


    return NULL;
}



DupCmdQMgr* DupCmdQMgr::CmdQMgr_GetCurMgrObj(ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(module >= CAM_MAX){
        BASE_LOG_ERR("unsupported module:0x%x\n",module);
        return NULL;
    }

    switch(cq){
        case ISP_DRV_CQ_THRE0:
            return gCmdQMgr[module].CQ_TYPE1.CmdQMgr_GetCurMgrObj(cq);
            break;
        default:
            return gCmdQMgr[module].CQ_TYPE2[cq].CmdQMgr_GetCurMgrObj(cq);
            break;
    }


    return NULL;
}

