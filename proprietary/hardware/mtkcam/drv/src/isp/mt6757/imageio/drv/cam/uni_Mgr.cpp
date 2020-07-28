#define LOG_TAG "UniMgr"

#include "uni_mgr.h"
#include "isp_function_cam.h"

#include <unistd.h> //for usleep

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;


DECLARE_DBG_LOG_VARIABLE(UniMgr);

// Clear previous define, use our own define.
#undef UniMgr_VRB
#undef UniMgr_DBG
#undef UniMgr_INF
#undef UniMgr_WRN
#undef UniMgr_ERR
#undef UniMgr_AST
#define UniMgr_VRB(fmt, arg...)        do { if (UniMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define UniMgr_DBG(fmt, arg...)        do { if (UniMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define UniMgr_INF(fmt, arg...)        do { if (UniMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define UniMgr_WRN(fmt, arg...)        do { if (UniMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define UniMgr_ERR(fmt, arg...)        do { if (UniMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define UniMgr_AST(cond, fmt, arg...)  do { if (UniMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)




class UniMgrImp : public UniMgr
{
public:
    UniMgrImp();
    virtual ~UniMgrImp(){}

    virtual MINT32          UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn = NULL);
    virtual MINT32          UniMgr_detach(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_start(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_stop(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_suspend(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_resume(ISP_HW_MODULE module );

    virtual UniDrvImp*      UniMgr_GetCurObj(void);
    virtual ISP_HW_MODULE   UniMgr_GetCurLink(void);

    virtual MBOOL           UniMgr_SetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB,MVOID* pNotify);
    virtual MVOID*          UniMgr_GetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB);

    virtual MBOOL           UniMgr_GetModuleObj(E_UNI_WDMA module, MUINTPTR* pWdma,MUINTPTR* pFbc);

private:
            MBOOL           FSM_CHK(MUINT32 op);

public:
private:

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop,
    };
    MUINT32         m_bStart;

    UNI_HW_MODULE   mModule;
    ISP_HW_MODULE   mConnectModule;

    MUINT32         mFSM;

    UniDrvImp*      pUniDrv;
    DupCmdQMgr*     pCamDrv;

    UNI_TOP_CTRL    m_UniCtrl;
    DMA_RAWI        m_RawI;

    BUF_CTRL_FLKO   m_FLKO_FBC;
    BUF_CTRL_EISO   m_EISO_FBC;
    BUF_CTRL_RSSO   m_RSSO_FBC;
    DMA_EISO        m_EISO;
    DMA_FLKO        m_FLKO;
    DMA_RSSO        m_RSSO;


    MBOOL           m_bRAWI;

    MVOID*          m_pData[_Max_CB_];

    mutable Mutex   m_lock;  // for multi-thread
};

/**
    Class of UniMgrImp
*/
MBOOL UniMgrImp::FSM_CHK(MUINT32 op)
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
                case op_cfg:
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
        UniMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x, connected module:0x%x\n",this->mFSM,op,this->mConnectModule);

    return ret;
}


UniMgrImp::UniMgrImp()
{
    m_bStart = 0;
    pUniDrv = NULL;
    pCamDrv = NULL;
    mFSM = op_unknow;
    m_bRAWI = MFALSE;
    mModule = MAX_UNI_HW_MODULE;
    mConnectModule = CAM_MAX;
    for(MUINT32 i=0;i<_Max_CB_;i++)
        m_pData[i] = NULL;

    DBG_LOG_CONFIG(imageio, UniMgr);
}

MINT32 UniMgrImp::UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn)
{
    ISP_HW_MODULE _module;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 subsample;

    Mutex::Autolock lock(this->m_lock);

    if(this->FSM_CHK(op_cfg) == MFALSE){
        return 1;
    }

    pConnectCam->CmdQMgr_GetCurCycleObj(0)[0]->getCurObjInfo(&_module,&cq,&page);

    //new setting
    this->mModule = module;
    this->mConnectModule = _module;
    this->pCamDrv = pConnectCam;
    if(pRawIn != NULL){
        this->m_bRAWI = MTRUE;
        memcpy((void*)&this->m_RawI.dma_cfg,pRawIn,sizeof(IspDMACfg));
    }
    else
        this->m_bRAWI = MFALSE;

    UniMgr_INF("UniMgrImp::UniMgr_attach+: module:0x%x,connect cam:0x%x, IsRawIn= %d\n",module,_module,this->m_bRAWI);


    //malloc & create & init uni drv
    if((this->pUniDrv = (UniDrvImp*)UniDrvImp::createInstance(module,(IspDrvImp *) this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj())) == NULL){
        UniMgr_ERR("UniMgrImp::uni drv create fail\n");
        return 1;
    }
    this->pUniDrv->init("UNI_CTRL");


    //uni_top
    this->m_UniCtrl.uni_top_ctl.FUNC_EN.Raw = 0x0;
    this->m_UniCtrl.uni_top_ctl.INTE_EN.Raw = (DMA_ERR_INT_EN_| \
                        FLKO_A_ERR_INT_EN_| \
                        EISO_A_ERR_INT_EN_| \
                        RSSO_A_ERR_INT_EN_);
    this->m_UniCtrl.uni_top_ctl.FMT_SEL.Raw = 0x0;
    this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw = 0x0;

    this->m_UniCtrl.m_pUniDrv = this->pUniDrv;

    if(this->m_bRAWI){
        this->m_UniCtrl.uni_top_ctl.FUNC_EN.Bits.UNP2_A_EN = 1;
        this->m_UniCtrl.m_Crop.x = 0;
        this->m_UniCtrl.m_Crop.y = 0;
        this->m_UniCtrl.m_Crop.w = this->m_RawI.dma_cfg.size.w;
        this->m_UniCtrl.m_Crop.h = this->m_RawI.dma_cfg.size.h;
    }

    switch(this->mConnectModule){
        case CAM_A:
            //
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= HDS2_A_SEL_0;
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= FLK2_A_SEL_0;
            //
            if(this->m_bRAWI)
                this->m_UniCtrl.UNI_Source = IF_RAWI_CAM_A;
            else
                this->m_UniCtrl.UNI_Source = IF_CAM_A;
            break;
        case CAM_B:
            //
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= HDS2_A_SEL_1;
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= FLK2_A_SEL_1;
            //
            if(this->m_bRAWI)
                this->m_UniCtrl.UNI_Source = IF_RAWI_CAM_B;
            else
                this->m_UniCtrl.UNI_Source = IF_CAM_B;
            break;
        default:
            UniMgr_ERR("unsuported cam:0x%x\n",this->mConnectModule);
            return 1;
            break;
    }

    //
    for(MUINT32 i=0;i<pConnectCam->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<pConnectCam->CmdQMgr_GetBurstQ();j++){
            this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_UniCtrl.config();
            this->m_UniCtrl.write2CQ();

            //

            this->m_EISO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_EISO_FBC.m_pUniDrv = this->pUniDrv;
            this->m_EISO_FBC.write2CQ();
            this->m_EISO_FBC.config();

            this->m_RSSO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_RSSO_FBC.m_pUniDrv = this->pUniDrv;
            this->m_RSSO_FBC.write2CQ();
            this->m_RSSO_FBC.config();


            subsample = (this->pCamDrv->CmdQMgr_GetBurstQ()-1);

            this->m_EISO_FBC.enable(NULL);
            this->m_RSSO_FBC.enable(NULL);
        }
    }

    //flko follow statistic pipe dmao
    this->m_FLKO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_FLKO_FBC.m_pUniDrv = this->pUniDrv;
    this->m_FLKO_FBC.config();
    this->m_FLKO_FBC.enable((void*)&subsample);

    this->mFSM = op_cfg;
    UniMgr_INF("UniMgrImp::UniMgr_attach-\n");
    return 0;
}

MINT32 UniMgrImp::UniMgr_detach(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("UniMgr_stop fail, unpaired connect cam, input:0x%x, cur:0x%x\n",module,this->mConnectModule);
        return 1;
    }

    UniMgr_INF("UniMgrImp::UniMgr_detach+: module:0x%x,connected cam:0x%x\n",this->mModule,this->mConnectModule);

    this->pCamDrv = NULL;

    this->pUniDrv->uninit("UNI_CTRL");
    this->pUniDrv->destroyInstance();
    this->pUniDrv = NULL;

    for(MUINT32 i=0;i<_Max_CB_;i++)
        this->m_pData[i] = NULL;

    this->mModule = MAX_UNI_HW_MODULE;
    this->mConnectModule = CAM_MAX;

    this->m_bRAWI = MFALSE;

    this->mFSM = op_unknow;

    return 0;
}

UniDrvImp* UniMgrImp::UniMgr_GetCurObj(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }

    return this->pUniDrv;
}

MINT32 UniMgrImp::UniMgr_start(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("UniMgr_stop fail, unpaired connect cam, input:0x%x, cur:0x%x\n",module,this->mConnectModule);
        return 1;
    }

    this->m_bStart = MTRUE;

    this->m_UniCtrl.enable(NULL);

    UniMgr_INF("UniMgrImp::UniMgr_start + module:0x%x,connect cam:0x%x\n",this->mModule,this->mConnectModule);
    this->mFSM = op_start;
    return 0;
}
MINT32 UniMgrImp::UniMgr_stop(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("UniMgr_stop fail, unpaired connect cam, input:0x%x, cur:0x%x\n",module,this->mConnectModule);
        return 1;
    }

    this->m_bStart = MFALSE;
    if(this->m_bRAWI){
        this->m_UniCtrl.disable();
    }
    else{
        //clr all uni func on cmdq 1st
        for(MUINT32 i=0;i<this->pCamDrv->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<this->pCamDrv->CmdQMgr_GetBurstQ();j++){
                this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_UniCtrl.ClrUniFunc();

                this->m_EISO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_EISO_FBC.disable();

                this->m_RSSO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_RSSO_FBC.disable();
            }
        }
        //wait till uni func r truely stopped.
        this->m_UniCtrl.disable();
    }

    //flko follow statistic pipe
    this->m_FLKO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_FLKO_FBC.disable();

    UniMgr_INF("UniMgrImp::UniMgr_stop + module:0x%x,connect cam:0x%x\n",this->mModule,this->mConnectModule);
    this->mFSM = op_stop;
    return 0;
}


MINT32 UniMgrImp::UniMgr_suspend(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("UniMgr_stop fail, unpaired connect cam, input:0x%x, cur:0x%x\n",module,this->mConnectModule);
        return 1;
    }

    if(this->m_UniCtrl.suspend() == MFALSE)
        return 1;
    else
        return 0;
}

MINT32 UniMgrImp::UniMgr_resume(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("UniMgr_stop fail, unpaired connect cam, input:0x%x, cur:0x%x\n",module,this->mConnectModule);
        return 1;
    }

    if(this->m_UniCtrl.resume() == MFALSE)
        return 1;
    else
        return 0;
}

ISP_HW_MODULE UniMgrImp::UniMgr_GetCurLink(void)
{
#if 0   //remove this chk to avoid the false-alarm,error log.
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return CAM_MAX;
    }
#endif
    return this->mConnectModule;
}


MBOOL UniMgrImp::UniMgr_SetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB,MVOID* pNotify)
{
    if(module != this->mConnectModule){
        UniMgr_ERR("SetCB_%d fail, unpaired connect cam, input:0x%x, cur:0x%x\n",eUNI_CB,module,this->mConnectModule);
        return MFALSE;
    }

    if(this->FSM_CHK(op_update) == MFALSE){
        return MFALSE;
    }

    if(this->m_pData[eUNI_CB] == NULL){
        this->m_pData[eUNI_CB] = pNotify;
    }
    else{
        UniMgr_ERR("SetCB_%d is not NULL , module:0x%x,connect cam:0x%x\n",eUNI_CB,this->mModule,this->mConnectModule);
        return MFALSE;
    }
    return MTRUE;
}

MVOID* UniMgrImp::UniMgr_GetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB)
{
    if(module != this->mConnectModule){
        UniMgr_ERR("GetCB_%d fail, unpaired connect cam, input:0x%x, cur:0x%x\n",eUNI_CB,module,this->mConnectModule);
        return NULL;
    }

    if(this->FSM_CHK(op_update) == MFALSE){
        return NULL;
    }

    return this->m_pData[eUNI_CB];
}


MBOOL UniMgrImp::UniMgr_GetModuleObj(E_UNI_WDMA module, MUINTPTR* pWdma,MUINTPTR* pFbc)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return MFALSE;
    }

    switch(module){
        case _UNI_EIS_:
            *pWdma = (MUINTPTR)&this->m_EISO;
            *pFbc = (MUINTPTR)&this->m_EISO_FBC;
            break;
        case _UNI_FLK_:
            *pWdma = (MUINTPTR)&this->m_FLKO;
            *pFbc = (MUINTPTR)&this->m_FLKO_FBC;
            break;
        case _UNI_RSS_:
            *pWdma = (MUINTPTR)&this->m_RSSO;
            *pFbc = (MUINTPTR)&this->m_RSSO_FBC;
            break;
        default:
            *pWdma = 0;
            *pFbc = 0;
            UniMgr_ERR("unsupported module:0x%x\n",module);
            return MFALSE;
            break;
    }
    return MTRUE;
}


#if 0
#endif

static UniMgrImp gUniMgr[MAX_UNI_HW_MODULE];


/**
    Class of UniMgr
*/
UNI_HW_MODULE UniMgr::m_module = UNI_A;
MINT32 UniMgr::UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn)
{
    MINT32 ret = 0;

    if(module >= MAX_UNI_HW_MODULE){
        UniMgr_ERR("unsupported module:0x%x\n",module);
        return 1;
    }


    if((ret=gUniMgr[module].UniMgr_attach(module,pConnectCam,pRawIn)) == 0){
        this->m_module = module;
    }
    else{
        UniMgr_ERR("unimgr attach fail: uni:0x%x \n",module);
    }

    return ret;
}

MINT32 UniMgr::UniMgr_detach(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_detach(module);
}


UniDrvImp* UniMgr::UniMgr_GetCurObj(void)
{
    return gUniMgr[this->m_module].UniMgr_GetCurObj();
}

MINT32 UniMgr::UniMgr_start(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_start(module);
}
MINT32 UniMgr::UniMgr_stop(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_stop(module);
}

ISP_HW_MODULE UniMgr::UniMgr_GetCurLink(void)
{
    return gUniMgr[this->m_module].UniMgr_GetCurLink();
}

MBOOL UniMgr::UniMgr_SetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB,MVOID* pNotify)
{
    return gUniMgr[this->m_module].UniMgr_SetCB(module,eUNI_CB,pNotify);
}

MVOID* UniMgr::UniMgr_GetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB)
{
    return gUniMgr[this->m_module].UniMgr_GetCB(module,eUNI_CB);
}

MINT32 UniMgr::UniMgr_suspend(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_suspend(module);
}


MINT32 UniMgr::UniMgr_resume(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_resume(module);
}


MBOOL UniMgr::UniMgr_GetModuleObj(E_UNI_WDMA module, MUINTPTR* pWdma,MUINTPTR* pFbc)
{
    return gUniMgr[this->m_module].UniMgr_GetModuleObj(module,pWdma,pFbc);
}

