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

typedef enum{
    op_switchnull,
    op_switchout,
    op_switchouttrig,
    op_switchin,
    op_switchintrig,
}E_FSM_UNI_SWITCH;


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

    virtual MVOID           UniMgr_rstBufCtrl(MBOOL rst);

    virtual MINT32          UniMgr_SwitchOut(ISP_HW_MODULE module);
    virtual MINT32          UniMgr_SwitchOutTrig(ISP_HW_MODULE module);
    virtual MINT32          UniMgr_SwitchIn(ISP_HW_MODULE module);
    virtual MINT32          UniMgr_SwitchInTrig(ISP_HW_MODULE module);
    virtual ISP_HW_MODULE    UniMgr_GetCurAttachModule();
    virtual ISP_HW_MODULE    UniMgr_GetFLKConnectModule();

private:
            MBOOL           FSM_CHK(MUINT32 op);
    MBOOL           FSM_CHK_SWITCH(MUINT32 op);

public:
    ISP_HW_MODULE mConModule; /* To record cur cam is attached or not */
private:

    enum E_FSM{
        op_unknow = 0x0,
        op_cfg,
        op_start,
        op_update,
        op_getinfo,
        op_stop, //0x5
    };
    MUINT32         m_bStart;

    UNI_HW_MODULE   mModule;

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
    static Mutex  m_drvlock;

    static E_FSM_UNI_SWITCH    m_FSM_UniSwitch; /* To record FSM of uni_switch process */
    static MUINT32                      m_uniSwitchTotalCnt; /* To record how many times uni switch do */
    static ISP_HW_MODULE         mConnectModule; /* To record uni is actually attached to which cam */
    static ISP_HW_MODULE         m_FLKConnectModule; /* To record FLK is currently attached to which cam */
    static DupCmdQMgr*             m_CamDrv[CAM_MAX]; /* To clear FLK setting on CAM_A/B if FLK is turn off. */
};

Mutex  UniMgrImp::m_drvlock;
E_FSM_UNI_SWITCH    UniMgrImp::m_FSM_UniSwitch       = op_switchnull;
ISP_HW_MODULE         UniMgrImp::mConnectModule          = CAM_MAX;
ISP_HW_MODULE         UniMgrImp::m_FLKConnectModule = CAM_MAX;
MUINT32                      UniMgrImp::m_uniSwitchTotalCnt   = 0;
DupCmdQMgr*             UniMgrImp::m_CamDrv[CAM_MAX] = {0, };

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
        UniMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x, connected module:0x%x\n", this->mFSM,op,mConnectModule);

    return ret;
}

MBOOL UniMgrImp::FSM_CHK_SWITCH(MUINT32 op)
{
    MBOOL ret =MTRUE;

    switch(op){
        case op_switchout:
            switch(this->m_FSM_UniSwitch){
                case op_switchnull:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_switchouttrig:
            switch(this->m_FSM_UniSwitch){
                case op_switchout:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_switchin:
            switch(this->m_FSM_UniSwitch){
                case op_switchouttrig:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_switchintrig:
            switch(this->m_FSM_UniSwitch){
                case op_switchin:
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
        UniMgr_ERR("[Uni Switch] FSM CHK error:cur:0x%x, tar:0x%x, connected module:0x%x\n",this->m_FSM_UniSwitch,op,this->mConnectModule);

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

    mConModule = CAM_MAX;

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
    this->mConModule = _module;
    this->pCamDrv = pConnectCam;
    this->m_CamDrv[_module] = pConnectCam;

    if(this->mConnectModule == CAM_MAX)
        this->mConnectModule = _module;

    if(pRawIn != NULL){
        this->m_bRAWI = MTRUE;
        memcpy((void*)&this->m_RawI.dma_cfg,pRawIn,sizeof(IspDMACfg));
    }
    else
        this->m_bRAWI = MFALSE;

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x, IsRawIn=%d\n", this->mModule, this->mConModule, this->mConnectModule, this->m_bRAWI);

    //malloc & create & init uni drv
    if((this->pUniDrv = (UniDrvImp*)UniDrvImp::createInstance(module,(IspDrvImp *) this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj())) == NULL){
        UniMgr_ERR("UniMgrImp::uni drv create fail\n");
        return 1;
    }

    if(module == UNI_A)
        this->pUniDrv->init("UNI_A_CTRL");
    else if (module == UNI_B)
        this->pUniDrv->init("UNI_B_CTRL");

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

    switch(this->mConModule){
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
            UniMgr_ERR("unsuported cam:0x%x\n",this->mConModule);
            return 1;
            break;
    }

    if(this->mConnectModule == this->mConModule) {
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
        this->m_FLKConnectModule = this->mConModule;
    } else {
        for(MUINT32 i=0;i<pConnectCam->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<pConnectCam->CmdQMgr_GetBurstQ();j++){
                this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];

                this->m_EISO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_EISO_FBC.m_pUniDrv = this->pUniDrv;
                this->m_EISO_FBC.config();

                this->m_RSSO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_RSSO_FBC.m_pUniDrv = this->pUniDrv;
                this->m_RSSO_FBC.config();

                this->m_EISO_FBC.enable(NULL);
                this->m_RSSO_FBC.enable(NULL);
            }
        }
    }

    this->mFSM = op_cfg;
    UniMgr_INF("-mFSM(0x%x)\n", this->mFSM);
    return 0;
}

MINT32 UniMgrImp::UniMgr_detach(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }

    if(module != this->mConModule){
        UniMgr_INF("Module(0x%x) is not attached. mConModule(0x%x)\n", module, this->mConModule);
        return 0;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    Mutex::Autolock lock(this->m_drvlock);

    this->pCamDrv = NULL;
    this->m_CamDrv[module] = NULL;

    if(this->mModule == UNI_A)
        this->pUniDrv->uninit("UNI_A_CTRL");
    else if (this->mModule == UNI_B)
        this->pUniDrv->uninit("UNI_B_CTRL");

    this->pUniDrv->destroyInstance();
    this->pUniDrv = NULL;

    for(MUINT32 i=0;i<_Max_CB_;i++)
        this->m_pData[i] = NULL;

    if(module == this->mConnectModule) {
    this->mConnectModule = CAM_MAX;
        this->m_FSM_UniSwitch = op_switchnull;
        this->m_uniSwitchTotalCnt = 0;
    }

    this->mModule = MAX_UNI_HW_MODULE;
    this->m_bRAWI = MFALSE;

    this->mFSM = op_unknow;

    UniMgr_INF("-mFSM(0x%x)\n", this->mFSM);
    return 0;
}

UniDrvImp* UniMgrImp::UniMgr_GetCurObj(void)
{
    if(this->FSM_CHK(op_getinfo) == MFALSE){
        return NULL;
    }

    Mutex::Autolock lock(this->m_lock);
    return this->pUniDrv;
}

MINT32 UniMgrImp::UniMgr_start(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }

    if(module != this->mConModule){
        UniMgr_ERR("module(0x%x is not attached. mConModule(0x%x))\n",module,this->mConModule);
        return 1;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    this->m_bStart = MTRUE;
    this->m_UniCtrl.enable(NULL);

    this->mFSM = op_start;
    return 0;
}

MINT32 UniMgrImp::UniMgr_stop(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    if(module != this->mConModule){
        UniMgr_ERR("module(0x%x) is not attached. mConModule(0x%x))\n",module,this->mConModule);
        return 1;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x, FLKConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule, this->m_FLKConnectModule);

    this->m_bStart = MFALSE;
    if(this->m_bRAWI){
        this->m_UniCtrl.disable();
    }
    else{
        //clr all uni func on cmdq 1st
        for(MUINT32 i=0;i<this->pCamDrv->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<this->pCamDrv->CmdQMgr_GetBurstQ();j++){
                if(module == this->mConnectModule) {
                    this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                    if(module == this->m_FLKConnectModule) {
                        /* lmv and flk are attached to same module then disalbe all */
                        this->m_UniCtrl.ClrUniFunc();
                    } else {
                        /* lmv is attached then disalbe lmv */
                        this->m_UniCtrl.ClrLMVFunc();
                    }

                    this->m_EISO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                    this->m_EISO_FBC.disable();

                    this->m_RSSO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
                    this->m_RSSO_FBC.disable();
                }
            }
        }
        if(module == this->mConnectModule && module == this->m_FLKConnectModule) {
            /* lmv and flk are attached to same module then disalbe all */
            this->m_UniCtrl.disable();
        } else if(module == this->mConnectModule) {
            /* lmv is attached then disalbe lmv */
            this->m_UniCtrl.ClrLMVFunc();
        }
    }

    if(module == this->m_FLKConnectModule) {
    //flko follow statistic pipe
	Mutex::Autolock lock(this->m_drvlock);

        for(MUINT32 k=0; k < CAM_MAX; k++){
            UniMgr_INF("Clear CAM(0x%x) flk setting on page", k);
            if(this->m_CamDrv[k] != NULL) {
                for(MUINT32 i=0;i<this->m_CamDrv[k]->CmdQMgr_GetDuqQ();i++){
                    for(MUINT32 j=0;j<this->m_CamDrv[k]->CmdQMgr_GetBurstQ();j++){
                        this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->m_CamDrv[k]->CmdQMgr_GetCurCycleObj(i)[j];
                        this->m_UniCtrl.ClearFLKSettingOnCQ();
                    }
                }
            }
        }

        this->m_FLKO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_FLKO_FBC.disable();
        this->m_FLKConnectModule = CAM_MAX;
    }

    this->mFSM = op_stop;
    return 0;
}


MINT32 UniMgrImp::UniMgr_suspend(ISP_HW_MODULE module )
{
    if(this->FSM_CHK(op_stop) == MFALSE){
        return 1;
    }

    if(module != this->mConModule){
        UniMgr_ERR("module(0x%x) is not attached. mConModule(0x%x))\n",module,this->mConModule);
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

    if(module != this->mConModule){
        UniMgr_ERR("module(0x%x) is not attached. mConModule(0x%x))\n",module,this->mConModule);
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

    return (ISP_HW_MODULE)this->mConModule;
}

MBOOL UniMgrImp::UniMgr_SetCB(ISP_HW_MODULE module ,E_UNI_CB eUNI_CB,MVOID* pNotify)
{
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
    if(module != this->mConModule){
        UniMgr_ERR("module(0x%x) is not attached. mConModule(0x%x))\n",module,this->mConModule);
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

MINT32 UniMgrImp::UniMgr_SwitchOut(ISP_HW_MODULE module )
{
    if(this->FSM_CHK_SWITCH(op_switchout) == MFALSE){
        return 1;
    }

    if(module != this->mConnectModule){
        UniMgr_ERR("Current connectModule(%x) is not same with module(%x)\n", this->mConnectModule, module);
        return 1;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    for(MUINT32 i=0;i<this->pCamDrv->CmdQMgr_GetDuqQ();i++) {
        for(MUINT32 j=0;j<this->pCamDrv->CmdQMgr_GetBurstQ();j++) {
            this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_UniCtrl.SwitchOut();
        }
    }

    Mutex::Autolock lock(this->m_lock);

    this->mConnectModule = CAM_MAX;
    this->m_FSM_UniSwitch = op_switchout;
    UniMgr_INF("-: FSMSwitch:0x%x\n", this->m_FSM_UniSwitch);

    return 0;
}

MINT32 UniMgrImp::UniMgr_SwitchOutTrig(ISP_HW_MODULE module )
{
    if(this->FSM_CHK_SWITCH(op_switchouttrig) == MFALSE){
        return 1;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    for(MUINT32 i=0;i<this->pCamDrv->CmdQMgr_GetDuqQ();i++) {
        for(MUINT32 j=0;j<this->pCamDrv->CmdQMgr_GetBurstQ();j++) {
            this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_UniCtrl.SwitchOut();
            this->m_UniCtrl.SwitchOutTrig();
        }
    }

    Mutex::Autolock lock(this->m_lock);

    this->m_FSM_UniSwitch = op_switchouttrig;
    UniMgr_INF("-: FSMSwitch:0x%x\n", this->m_FSM_UniSwitch);

    return 0;
}

MINT32 UniMgrImp::UniMgr_SwitchIn(ISP_HW_MODULE module )
{
    if(this->FSM_CHK_SWITCH(op_switchin) == MFALSE){
        return 1;
    }

    MUINT32 subsample;
    MUINT32 enableEIS = MFALSE, enableRSS = MFALSE;

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    this->mConnectModule = module;
    this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw = 0;

    this->m_UniCtrl.uni_top_ctl.INTE_EN.Raw = (DMA_ERR_INT_EN_| \
                        FLKO_A_ERR_INT_EN_| \
                        EISO_A_ERR_INT_EN_| \
                        RSSO_A_ERR_INT_EN_);

    switch(module){
        case CAM_A:
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= HDS2_A_SEL_0;
            this->m_UniCtrl.UNI_Source = IF_CAM_A;
            break;
        case CAM_B:
            this->m_UniCtrl.uni_top_ctl.CTRL_SEL.Raw |= HDS2_A_SEL_1;
            this->m_UniCtrl.UNI_Source = IF_CAM_B;
            break;
        default:
            UniMgr_ERR("unsuported cam:0x%x\n",this->mConnectModule);
            return 1;
            break;
    }

    if(this->m_pData[_EIS_CB_] != NULL)
        enableEIS = MTRUE;
    if(this->m_pData[_RSS_CB_] != NULL)
        enableRSS = MTRUE;

    for(MUINT32 i=0;i<this->pCamDrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->pCamDrv->CmdQMgr_GetBurstQ();j++){
            this->m_UniCtrl.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_UniCtrl.SwitchIn(enableEIS, enableRSS);

            this->m_EISO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_EISO_FBC.m_pUniDrv = this->pUniDrv;
            this->m_EISO_FBC.write2CQ();

            this->m_RSSO_FBC.m_pIspDrv = (IspDrvVir*)this->pCamDrv->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_RSSO_FBC.m_pUniDrv = this->pUniDrv;
            this->m_RSSO_FBC.write2CQ();
        }
    }

    this->m_UniCtrl.enable(NULL);
    this->m_FSM_UniSwitch = op_switchin;

    UniMgr_INF("-: FSMSwitch:0x%x\n", this->m_FSM_UniSwitch);
    return 0;
}

MINT32 UniMgrImp::UniMgr_SwitchInTrig(ISP_HW_MODULE module )
{
    if(this->FSM_CHK_SWITCH(op_switchintrig) == MFALSE){
        return 1;
    }

    UniMgr_INF("+: module:0x%x, attach_cam:0x%x, mConnectModule=0x%x\n",this->mModule, this->mConModule, this->mConnectModule);

    this->m_UniCtrl.SwitchInTrig();

    this->m_FSM_UniSwitch = op_switchnull;
    this->m_uniSwitchTotalCnt++;

    UniMgr_INF("-: FSMSwitch:0x%x totalCnt(%d)\n", this->m_FSM_UniSwitch, this->m_uniSwitchTotalCnt);
    return 0;
}

ISP_HW_MODULE UniMgrImp::UniMgr_GetCurAttachModule()
{
    Mutex::Autolock lock(this->m_lock);

    UniMgr_DBG("CurAttachModule(%d)", this->mConnectModule);
    return this->mConnectModule;
}

ISP_HW_MODULE UniMgrImp::UniMgr_GetFLKConnectModule()
{
    UniMgr_DBG("CurFLKAttachModule(%d)", this->m_FLKConnectModule);
    return this->m_FLKConnectModule;
}

MVOID UniMgrImp::UniMgr_rstBufCtrl(MBOOL rst)
{
    //reset OverFlow counter for HW timestamp
    this->m_FLKO_FBC.clrTSBase(rst,this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0]);
    this->m_EISO_FBC.clrTSBase(rst,this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0]);
    this->m_RSSO_FBC.clrTSBase(rst,this->pCamDrv->CmdQMgr_GetCurCycleObj(0)[0]);
}

static UniMgrImp gUniMgr[MAX_UNI_HW_MODULE];

/**
    Class of UniMgr
*/
MBOOL UniMgr::m_UniFree = MTRUE;
UNI_HW_MODULE UniMgr::UniMgr_getUniModule(ISP_HW_MODULE module)
{
    UNI_HW_MODULE UniModule = MAX_UNI_HW_MODULE;

    if(gUniMgr[UNI_A].mConModule == module)
        UniModule = UNI_A;
    else if (gUniMgr[UNI_B].mConModule == module)
        UniModule = UNI_B;

    return UniModule;
}

MINT32 UniMgr::UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn)
{
    MINT32 ret = 0;

    if(module >= MAX_UNI_HW_MODULE){
        UniMgr_ERR("unsupported module:0x%x\n",module);
        return 1;
    }

    if(this->m_UniFree) {
        if((ret=gUniMgr[module].UniMgr_attach(module,pConnectCam,pRawIn)) == 0){
            this->m_module = module;
                this->m_UniFree = MFALSE;
        }
        else{
        UniMgr_ERR("unimgr attach fail: uni:0x%x \n",module);
            ret = -1;
        }
    } else {
        if((ret=gUniMgr[UNI_B].UniMgr_attach(UNI_B,pConnectCam,pRawIn)) == 0){
            this->m_module = UNI_B;
        }
        else{
            UniMgr_ERR("unimgr attach fail: uni:0x%x \n",UNI_B);
            ret = -1;
        }
    }

    return ret;
}

MINT32 UniMgr::UniMgr_detach(ISP_HW_MODULE module )
{
    MUINT32 ret;

    ret = gUniMgr[this->m_module].UniMgr_detach(module);
    if(this->m_module == UNI_A)
        this->m_UniFree = MTRUE;

    return ret;
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

MINT32 UniMgr::UniMgr_SwitchOut(ISP_HW_MODULE module)
{
    return gUniMgr[this->m_module].UniMgr_SwitchOut(module);
}

MINT32 UniMgr::UniMgr_SwitchOutTrig(ISP_HW_MODULE module)
{
    return gUniMgr[this->m_module].UniMgr_SwitchOutTrig(module);
}

MINT32 UniMgr::UniMgr_SwitchIn(ISP_HW_MODULE module)
{
    return gUniMgr[this->m_module].UniMgr_SwitchIn(module);
}

MINT32 UniMgr::UniMgr_SwitchInTrig(ISP_HW_MODULE module)
{
    return gUniMgr[this->m_module].UniMgr_SwitchInTrig(module);
}

ISP_HW_MODULE UniMgr::UniMgr_GetCurAttachModule()
{
    return gUniMgr[UNI_A].UniMgr_GetCurAttachModule();
}

ISP_HW_MODULE UniMgr::UniMgr_GetFLKConnectModule()
{
    return gUniMgr[UNI_A].UniMgr_GetFLKConnectModule();
}

MVOID UniMgr::UniMgr_rstBufCtrl(MBOOL rst)
{
    return gUniMgr[this->m_module].UniMgr_rstBufCtrl(rst);
}
