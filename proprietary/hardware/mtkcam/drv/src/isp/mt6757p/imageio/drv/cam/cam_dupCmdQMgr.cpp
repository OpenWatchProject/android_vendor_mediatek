#define LOG_TAG "CmdQMgr"

#include "cam_dupCmdQMgr.h"
#include "isp_function_cam.h"

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

    virtual MINT32          CmdQMgr_start(void);
    virtual MINT32          CmdQMgr_stop(void);
    virtual MINT32          CmdQMgr_update(void);

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void);
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void);

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
    virtual DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

            MBOOL           CmdQMgr_specialCQCtrl(void);

    virtual MUINT32         CmdQMgr_GetCounter(MBOOL bPhy=MFALSE);
private:
            MBOOL           FSM_CHK(MUINT32 op);

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
};

/**
    Class of CmdQMgrImp
*/
MBOOL CmdQMgrImp::FSM_CHK(MUINT32 op)
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
        CmdQMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x\n",this->mFSM,op);

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
    if(this->FSM_CHK(op_cfg) == MFALSE){
        return NULL;
    }
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_attach+: fps:0x%x,subsample:0x%x,module:0x%x,cq:0x%x\n",fps,subsample,module,cq);
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
    this->mModule = module;
    this->mCQ = cq;
    this->mDupCmdQ = DEFAULT_DUP_NUM;



    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp::CmdQMgr_attach: memory leakage:0x%x\n",this->pCamDrv);

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

            //
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
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x\n",this->mDupCmdQ,this->mBurstQ);
    return this;
}

MINT32 CmdQMgrImp::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }
    CmdQMgr_DBG("CmdQMgrImp::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }

    return this->pCamDrv[DupIdx];
}

MINT32 CmdQMgrImp::CmdQMgr_update(void)
{
    MUINT32 curDummyCQIdx = 0;
    MUINT32 prvDummyCQIdx = 0;


    if(this->FSM_CHK(op_update) == MFALSE){
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
    for(MUINT32 i=0;i<this->mBurstQ;i++)
        this->pCamDrv[this->mDupCmdQIdx][i]->flushCmdQ();


        //at high speed mode, cpu can't program to outer register directly because of
        //cam_b always have sw p1 done under twin mode, wirte to outer register will
        //break down the cq link-list.
        //side effect:
        //sw program timing will reduce to (n-1)/n * (time of p1_sof to p1_done)
#if TWIN_SW_P1_DONE_WROK_AROUND
    if(this->mBurstQ == 1)
#endif
    {
        //at normal mode , its orignal design
        this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];
        this->m_CQ0.dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv[this->mDupCmdQIdx][0]->getCQDescBufPhyAddr();
        this->m_CQ0.config();
    }



    CmdQMgr_INF("CmdQMgrImp::CmdQMgr_update: 0x%x_0x%x",this->mDupCmdQIdx,this->m_CQ0.dma_cfg.memBuf.base_pAddr);

    return 0;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetBurstQ(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mBurstQ;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetDuqQ(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mDupCmdQ;
}


MUINT32 CmdQMgrImp::CmdQMgr_GetDuqQIdx(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mDupCmdQIdx;
}

MUINT32 CmdQMgrImp::CmdQMgr_GetBurstQIdx(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return 0;
    }

    return this->mBurstQIdx;
}


MINT32 CmdQMgrImp::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }

    this->m_bStart = MTRUE;

    //
    if(this->mBurstQ>1)
        this->CmdQMgr_specialCQCtrl();

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
    //
    if(this->m_CQ0.TrigCQ() == MFALSE){
        return 1;
    }

    //switch to continuous mode when isp_start
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
    if(this->FSM_CHK(op_stop) == MFALSE){
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
    if(this->FSM_CHK(op_getinfo) == MFALSE){
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

    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }

    return this;
}

MBOOL CmdQMgrImp::CmdQMgr_specialCQCtrl(void)
{
    MBOOL ret = MTRUE;
    if(this->FSM_CHK(op_update) == MFALSE){
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
    if(this->FSM_CHK(op_update) == MFALSE){
        return 0;
    }

    this->m_CQ0.m_pIspDrv = (IspDrvVir*)this->pCamDrv[this->mDupCmdQIdx][0];

    return this->m_CQ0.GetCQupdateCnt(bPhy);
}

#if 0
#endif
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
    virtual MINT32          CmdQMgr_update(void){return 0;}

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

    virtual MUINT32         CmdQMgr_GetBurstQIdx(void){return 0;}
    virtual MUINT32         CmdQMgr_GetDuqQIdx(void){return 0;}

    virtual MUINT32         CmdQMgr_GetBurstQ(void);
    virtual MUINT32         CmdQMgr_GetDuqQ(void);

    virtual MBOOL           CmdQMgr_GetCurStatus(void);
    virtual DupCmdQMgr*     CmdQMgr_GetCurMgrObj(E_ISP_CAM_CQ cq);

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
    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_attach+: fps:0x%x,subsample:0x%x,module:0x%x,cq:0x%x\n",fps,subsample,module,cq);
    //
    switch(cq){
        case ISP_DRV_CQ_THRE4:
            this->m_pCQ = (DMAI_B*)new DMA_CQ4();
            break;
        /*case ISP_DRV_CQ_THRE5://move to CmdQMgrImp3
            this->m_pCQ = (DMAI_B*)new DMA_CQ5();
            break;*/
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
    this->mModule = module;
    this->mCQ = cq;
    this->mDupCmdQ = DEFAULT_DUP_NUM2;


    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp2::CmdQMgr_attach: memory leakage:0x%x\n",this->pCamDrv);

    //create & init isp drv
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);
    this->m_pUserName[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp2:%d",this->mCQ);
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,0,(const char*)this->m_pUserName);
    //
    this->pCamDrv->init((const char*)this->m_pUserName);
    //CQI config
    this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
    this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
    this->m_pCQ->config();

    //switch to immediate-mode before start_isp
    switch(this->mCQ){
        case ISP_DRV_CQ_THRE4:
            ((DMA_CQ4*)this->m_pCQ)->setCQTriggerMode(DMA_CQ4::_immediate_);
            break;
        /*case ISP_DRV_CQ_THRE5://change to CmdQMgrImp3
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_immediate_);
            break;*/
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
    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_attach-: DupQ:0x%x,BurstQ:0x%x,CQ_PA:0x%x,\n",fps,module,cq);
    return this;
}

MINT32 CmdQMgrImp2::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }
    CmdQMgr_DBG("CmdQMgrImp2::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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
        /*case ISP_DRV_CQ_THRE5://change to CmdQMgrImp3
            if(((DMA_CQ5*)this->m_pCQ)->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ5*)this->m_pCQ)->setCQTriggerMode(DMA_CQ5::_continuouse_);
            break;*/
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

    virtual ISP_DRV_CAM**   CmdQMgr_GetCurCycleObj(MUINT32 DupIdx);

private:
            MBOOL           FSM_CHK(MUINT32 op);

public:
private:
    #define STR_LENG    (32)    //follow the length of username in isp_drv_cam

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_getinfo,
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
        case op_getinfo:
            switch(this->mFSM){
                case op_unknow:
                    ret = MFALSE;
                    break;
                default:
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
    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_attach+: fps:0x%x,subsample:0x%x,module:0x%x,cq:0x%x\n",fps,subsample,module,cq);


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
        case ISP_DRV_CQ_THRE5:
            this->m_pCQ = (DMAI_B*)new DMA_CQ5();
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
    this->mModule = module;
    this->mCQ = cq;


    if(this->pCamDrv != NULL)
        CmdQMgr_ERR("CmdQMgrImp3::CmdQMgr_attach: memory leakage:0x%x\n",this->pCamDrv);

    //create & init isp drv
    //
    this->m_pUserName = (char*)malloc(sizeof(char)*STR_LENG);
    this->m_pCQ10Name = (char*)malloc(sizeof(char)*STR_LENG);

    this->m_pUserName[0] = '\0';
    this->m_pCQ10Name[0] = '\0';
    sprintf(this->m_pUserName,"CmdQMgrImp3:%d",this->mCQ);



    //
    this->pCamDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(this->mModule,this->mCQ,\
    0,(const char*)this->m_pUserName);



    //
    this->pCamDrv->init((const char*)this->m_pUserName);


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
        case ISP_DRV_CQ_THRE5:
            //CQI config
            this->m_pCQ->m_pIspDrv = (IspDrvVir*)this->pCamDrv;
            this->m_pCQ->dma_cfg.memBuf.base_pAddr = (MUINTPTR)this->pCamDrv->getCQDescBufPhyAddr();
            this->m_pCQ->config();

            //add cqmodule
            //this->m_pCQ->write2CQ();
            //
            ((DMA_CQ5*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ5::_immediate_);
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
    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_attach-: \n");
    return this;
}


MINT32 CmdQMgrImp3::CmdQMgr_detach(void)
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }
    CmdQMgr_DBG("CmdQMgrImp3::CmdQMgr_detach+: module:0x%x,cq:0x%x\n",this->mModule,this->mCQ);

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

ISP_DRV_CAM** CmdQMgrImp3::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    (void)DupIdx;
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }
    return (this->mCQ == ISP_DRV_CQ_THRE11)? &this->pCQ10Drv : &this->pCamDrv;
}

MINT32 CmdQMgrImp3::CmdQMgr_start(void)
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }

    //
    this->pCamDrv->flushCmdQ();

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
        case ISP_DRV_CQ_THRE5:

            //
            if(this->mFSM != op_start)
                this->m_pCQ->enable(NULL);

            if(((DMA_CQ5*)(this->m_pCQ))->TrigCQ() == MFALSE){
                return 1;
            }
            ((DMA_CQ5*)(this->m_pCQ))->setCQTriggerMode(DMA_CQ5::_p1_done_once_);

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
        case ISP_DRV_CQ_THRE5:
            if(((DMA_CQ5*)(this->m_pCQ))->setCQContent(arg1, arg2) == MFALSE)
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


#if 0
#endif

typedef struct{
    CmdQMgrImp  CQ_TYPE1;                           //support only cq0 currentlly
    CmdQMgrImp2 CQ_TYPE2[ISP_DRV_CAM_BASIC_CQ_NUM]; //support cq4/7/8/12
    CmdQMgrImp3 CQ_TYPE3[3];                        //cq1,cq11,cq5
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
            if(gCmdQMgr[module].CQ_TYPE1.CmdQMgr_attach_imp(fps,subsample,module,cq)){
                gCmdQMgr[module].CQ_TYPE1.m_module = module;
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE1;
            }
            else
                return NULL;
            break;
        case ISP_DRV_CQ_THRE1:
            if(gCmdQMgr[module].CQ_TYPE3[0].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                gCmdQMgr[module].CQ_TYPE3[0].m_module = module;
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE3[0];
            }
            else
                return NULL;

            break;
        case ISP_DRV_CQ_THRE5:
            if(gCmdQMgr[module].CQ_TYPE3[2].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                gCmdQMgr[module].CQ_TYPE3[2].m_module = module;
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE3[2];
            }
            else
                return NULL;

            break;
        case ISP_DRV_CQ_THRE11:
            if(gCmdQMgr[module].CQ_TYPE3[1].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                gCmdQMgr[module].CQ_TYPE3[1].m_module = module;
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE3[1];
            }
            else
                return NULL;

            break;
        default:
            if(gCmdQMgr[module].CQ_TYPE2[cq].CmdQMgr_attach_imp(fps,subsample,module,cq)){
                gCmdQMgr[module].CQ_TYPE2[cq].m_module = module;
                return (DupCmdQMgr*)&gCmdQMgr[module].CQ_TYPE2[cq];
            }
            else
                return NULL;
            break;
    }


    return NULL;
}

MINT32 DupCmdQMgr::CmdQMgr_detach(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_detach();
    return 1;
}


ISP_DRV_CAM** DupCmdQMgr::CmdQMgr_GetCurCycleObj(MUINT32 DupIdx)
{
    (void) DupIdx;
    //return gCmdQMgr[this->m_module].CmdQMgr_GetCurCycleObj(DupIdx);
    return NULL;
}

MINT32 DupCmdQMgr::CmdQMgr_update(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_update();
    return 1;
}

MUINT32 DupCmdQMgr::CmdQMgr_GetBurstQ(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_GetBurstQ();
    return 0;
}

MUINT32 DupCmdQMgr::CmdQMgr_GetDuqQ(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_GetDuqQ();
    return 0;
}


MUINT32 DupCmdQMgr::CmdQMgr_GetDuqQIdx(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_GetDuqQIdx();
    return 0;
}

MUINT32 DupCmdQMgr::CmdQMgr_GetBurstQIdx(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_GetBurstQIdx();
    return 0;
}


MINT32 DupCmdQMgr::CmdQMgr_start(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_start();
    return 1;
}
MINT32 DupCmdQMgr::CmdQMgr_stop(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_stop();
    return 1;
}

MBOOL DupCmdQMgr::CmdQMgr_GetCurStatus(void)
{
    //return gCmdQMgr[this->m_module].CmdQMgr_GetCurStatus();
    return MFALSE;
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


MINT32 DupCmdQMgr::CmdQMgr_Cfg(MUINTPTR arg1,MUINTPTR arg2)
{
    (void)arg1;(void)arg2;
    return 1;
}

ISP_HW_MODULE DupCmdQMgr::CmdQMgr_GetCurModule(void)
{
    BASE_LOG_DBG("module:0x%x\n",this->m_module);
    return this->m_module;
}

MUINT32 DupCmdQMgr::CmdQMgr_GetCounter(MBOOL bPhy)
{
    (void)bPhy;
    return 0;
}

