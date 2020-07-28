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

    virtual UniMgr*         UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn = NULL);
    virtual MINT32          UniMgr_detach(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_start(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_stop(ISP_HW_MODULE module );

    virtual MINT32          UniMgr_suspend(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_resume(ISP_HW_MODULE module );
    virtual MINT32          UniMgr_recover(E_UNI_STEP op,ISP_HW_MODULE module);
    virtual UniDrvImp*      UniMgr_GetCurObj(void);

    virtual MBOOL           UniMgr_SetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB,MVOID* pNotify);
    virtual MVOID*          UniMgr_GetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB);


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

    MUINT32         mFSM;

    UniDrvImp*      pUniDrv;
    IspDrvImp*      pCamDrv[2];

    UNI_TOP_CTRL    m_UniCtrl;
    DMA_RAWI        m_RawI;

    MBOOL           m_bRAWI;


    mutable Mutex   m_lock;  // for multi-thread
    static MBOOL    m_op[PHY_CAM-1][op_stop+1];
};

/**
    Class of UniMgrImp
*/
MBOOL UniMgrImp::m_op[PHY_CAM-1][op_stop+1] = {0};
MBOOL UniMgrImp::FSM_CHK(MUINT32 op)
{
    MBOOL ret =MTRUE;

    switch(op){
        case op_unknow: //detach
            switch(this->mFSM){
                case op_start:
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
                case op_start:
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
        UniMgr_ERR("FSM CHK error:cur:0x%x, tar:0x%x\n",this->mFSM,op);

    return ret;
}


UniMgrImp::UniMgrImp()
{
    m_bStart = 0;
    pUniDrv = NULL;
    pCamDrv[0] = NULL;
    pCamDrv[1] = NULL;
    mFSM = op_unknow;
    m_bRAWI = MFALSE;
    mModule = MAX_UNI_HW_MODULE;
    DBG_LOG_CONFIG(imageio, UniMgr);
}

UniMgr* UniMgrImp::UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn)
{
    ISP_HW_MODULE _module;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    Mutex::Autolock lock(this->m_lock);

    pConnectCam->CmdQMgr_GetCurCycleObj(0)[0]->getCurObjInfo(&_module,&cq,&page);
    this->pCamDrv[_module] = (IspDrvImp*)(pConnectCam->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj());

    if(this->m_op[_module][op_cfg]){
        UniMgr_ERR("attach again before detach first");
        return NULL;
    }

    switch(_module){
        case CAM_A:
            if(this->m_op[CAM_B][op_cfg] == MTRUE){
                this->m_op[_module][op_cfg] = MTRUE;
                return this;
            }
            break;
        case CAM_B:
            if(this->m_op[CAM_A][op_cfg] == MTRUE){
                this->m_op[_module][op_cfg] = MTRUE;
                return this;
            }
            break;
        default:
            UniMgr_ERR("unsupported:%d\n",_module);
            return NULL;
            break;
    }

    if(this->FSM_CHK(op_cfg) == MFALSE){
        return NULL;
    }
    //
    this->m_op[_module][op_cfg] = MTRUE;

    //new setting
    this->mModule = module;
    if(pRawIn != NULL){
        this->m_bRAWI = MTRUE;
        memcpy((void*)&this->m_RawI.dma_cfg,pRawIn,sizeof(IspDMACfg));
    }
    else
        this->m_bRAWI = MFALSE;

    UniMgr_INF("UniMgrImp::UniMgr_attach+: module:0x%x,connect cam:0x%x, IsRawIn= %d\n",module,_module,this->m_bRAWI);


    //malloc & create & init uni drv
    if((this->pUniDrv = (UniDrvImp*)UniDrvImp::createInstance(module)) == NULL){
        UniMgr_ERR("UniMgrImp::uni drv create fail\n");
        return NULL;
    }
    this->pUniDrv->init("UNI_CTRL",(IspDrvImp *) this->pCamDrv[_module]);


    //uni_top
    this->m_UniCtrl.uni_top_ctl.FUNC_EN.Raw = 0x0;
    this->m_UniCtrl.uni_top_ctl.FMT_SEL.Raw = 0x0;

    this->m_UniCtrl.m_pUniDrv = this->pUniDrv;

    if(this->m_bRAWI){
        this->m_UniCtrl.uni_top_ctl.FUNC_EN.Bits.UNP2_A_EN = 1;
        this->m_UniCtrl.m_Crop.x = 0;
        this->m_UniCtrl.m_Crop.y = 0;
        this->m_UniCtrl.m_Crop.w = this->m_RawI.dma_cfg.size.w;
        this->m_UniCtrl.m_Crop.h = this->m_RawI.dma_cfg.size.h;
    }

    switch(_module){
        case CAM_A:
            if(this->m_bRAWI)
                this->m_UniCtrl.UNI_Source = IF_RAWI_CAM_A;
            else
                this->m_UniCtrl.UNI_Source = IF_CAM_A;
            break;
        case CAM_B:
            if(this->m_bRAWI)
                this->m_UniCtrl.UNI_Source = IF_RAWI_CAM_B;
            else
                this->m_UniCtrl.UNI_Source = IF_CAM_B;
            break;
        default:
            UniMgr_ERR("unsuported cam:0x%x\n",_module);
            return NULL;
            break;
    }

    //
    this->m_UniCtrl.m_pCamDrv = this->pCamDrv[_module];//uni won't access vir regs those r at CAM_x
    this->m_UniCtrl.config();

    this->mFSM = op_cfg;
    UniMgr_INF("UniMgrImp::UniMgr_attach-\n");
    return this;
}

MINT32 UniMgrImp::UniMgr_detach(ISP_HW_MODULE module )
{
    Mutex::Autolock lock(this->m_lock);

    switch(module){
        case CAM_A://let last user to detach uni
            if( (this->m_op[CAM_B][op_cfg] == MTRUE) && (this->m_op[module][op_cfg] == MTRUE) ){
                this->m_op[module][op_cfg] = MFALSE;
                return 0;
            }
            break;
        case CAM_B:
            if( (this->m_op[CAM_A][op_cfg] == MTRUE) && (this->m_op[module][op_cfg] == MTRUE) ){
                this->m_op[module][op_cfg] = MFALSE;
                return 0;
            }
            break;
        default:
            UniMgr_ERR("unsupported:%d\n",module);
            return 1;
            break;
    }


    if(this->FSM_CHK(op_unknow) == MFALSE){
        return 1;
    }

    // stop flow
    this->m_bStart = MFALSE;
    if(this->m_bRAWI){
        this->m_UniCtrl.disable();
    }
    else{
        this->m_UniCtrl.m_pCamDrv = this->pCamDrv[module];//uni won't access vir regs those r at CAM_x
        this->m_UniCtrl.ClrUniFunc();
        //wait till uni func r truely stopped.
        this->m_UniCtrl.disable();
    }

    //
    memset((void*)this->m_op,MFALSE,sizeof(this->m_op));

    UniMgr_INF("UniMgrImp::UniMgr_detach+: module:0x%x\n",this->mModule);

    if (this->pUniDrv == NULL) {
        UniMgr_ERR("pUniDrv Null pointer\n");
        return 1;
    }

    this->pUniDrv->uninit("UNI_CTRL");
    this->pUniDrv->destroyInstance();
    this->pUniDrv = NULL;
    this->pCamDrv[0] = NULL;
    this->pCamDrv[1] = NULL;

    this->mModule = MAX_UNI_HW_MODULE;

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
    Mutex::Autolock lock(this->m_lock);

    UniMgr_INF("UniMgrImp::UniMgr_start + module:0x%x (%x_%x_%x_%x)\n",this->mModule,
        this->m_op[CAM_A][op_start],this->m_op[CAM_B][op_start],this->m_op[CAM_A][op_stop],this->m_op[CAM_B][op_stop]);

    if(this->m_op[module][op_start]){
        UniMgr_DBG("module:%d already start", module);
        return 0;
    }

    switch(module){//let first user to start uni
        case CAM_A:
            if(this->m_op[CAM_B][op_start] == MTRUE){
                this->m_op[module][op_start] = MTRUE;
                return 0;
            }
            break;
        case CAM_B:
            if(this->m_op[CAM_A][op_start] == MTRUE){
                this->m_op[module][op_start] = MTRUE;
                return 0;
            }
            break;
        default:
            UniMgr_ERR("unsupported:%d\n",module);
            return 1;
            break;
    }

    if(this->FSM_CHK(op_start) == MFALSE){
        return 1;
    }
    //
    this->m_op[module][op_start] = MTRUE;

    this->m_bStart = MTRUE;

    this->m_UniCtrl.enable(NULL);

    UniMgr_DBG("UniMgrImp::UniMgr_start + module:0x%x\n",this->mModule);
    this->mFSM = op_start;
    return 0;
}
MINT32 UniMgrImp::UniMgr_stop(ISP_HW_MODULE module )
{
    Mutex::Autolock lock(this->m_lock);

    UniMgr_INF("UniMgrImp::UniMgr_stop + module:0x%x (%x_%x_%x_%x)\n",this->mModule,
        this->m_op[CAM_A][op_start],this->m_op[CAM_B][op_start],this->m_op[CAM_A][op_stop],this->m_op[CAM_B][op_stop]);

    switch(module){//let last user to stop uni
        case CAM_A:
        case CAM_B:
            break;
        default:
            UniMgr_ERR("unsupported:%d\n",module);
            return 1;
            break;
    }

    //
    this->m_op[module][op_stop] = MTRUE;

    UniMgr_DBG("UniMgrImp::UniMgr_stop + module:0x%x\n",this->mModule);
    return 0;
}


MINT32 UniMgrImp::UniMgr_suspend(ISP_HW_MODULE module )
{
    UniMgr_ERR("UniMgr_suspend is not unsupported\n");
    return 1;
}

MINT32 UniMgrImp::UniMgr_resume(ISP_HW_MODULE module )
{
    UniMgr_ERR("UniMgr_resume is not unsupported\n");
    return 1;
}

MINT32 UniMgrImp::UniMgr_recover(E_UNI_STEP op,ISP_HW_MODULE module)
{
    UniMgr_ERR("UniMgr_recover is not unsupported\n");
    return MFALSE;
}

MBOOL UniMgrImp::UniMgr_SetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB,MVOID* pNotify)
{
    UniMgr_ERR("UniMgr_SetCB is not unsupported\n");
    return MFALSE;
}

MVOID* UniMgrImp::UniMgr_GetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB)
{
    UniMgr_ERR("UniMgr_GetCB is not unsupported\n");
    return MFALSE;
}

static UniMgrImp gUniMgr[MAX_UNI_HW_MODULE];


/**
    Class of UniMgr
*/
UNI_HW_MODULE UniMgr::m_module = UNI_A;
UniMgr* UniMgr::UniMgr_attach(UNI_HW_MODULE module,DupCmdQMgr* pConnectCam,IspDMACfg* pRawIn)
{
    MINT32 ret = 0;

    if(module >= MAX_UNI_HW_MODULE){
        UniMgr_ERR("unsupported module:0x%x\n",module);
        return NULL;
    }


    if(gUniMgr[module].UniMgr_attach(module,pConnectCam,pRawIn)){
        gUniMgr[module].m_module = module;
        return (UniMgr*)&gUniMgr[module];
    }
    else{
        UniMgr_ERR("unimgr attach fail: uni:0x%x \n",module);
        return NULL;
    }

}

MINT32 UniMgr::UniMgr_detach(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_detach(module);
}


UniDrvImp* UniMgr::UniMgr_GetCurObj(void)
{
    return gUniMgr[m_module].UniMgr_GetCurObj();
}

MINT32 UniMgr::UniMgr_start(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_start(module);
}
MINT32 UniMgr::UniMgr_stop(ISP_HW_MODULE module )
{
    return gUniMgr[this->m_module].UniMgr_stop(module);
}

MBOOL UniMgr::UniMgr_SetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB,MVOID* pNotify)
{
    return gUniMgr[this->m_module].UniMgr_SetCB(module,eUNI_CB,pNotify);
}

MVOID* UniMgr::UniMgr_GetCB(ISP_HW_MODULE module ,MUINT32 eUNI_CB)
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

MINT32 UniMgr::UniMgr_recover(E_UNI_STEP op,ISP_HW_MODULE module)
{
    return gUniMgr[this->m_module].UniMgr_recover(op,module);
}
