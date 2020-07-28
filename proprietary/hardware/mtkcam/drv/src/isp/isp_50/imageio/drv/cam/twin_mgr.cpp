/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "TwinMgr"


#include "twin_mgr_imp.h"
#include "cam_capibility.h"

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include "cam_capibility.h"
#include "uni_mgr.h"        //for hw_recover()

#include <string>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;


DECLARE_DBG_LOG_VARIABLE(TwinMgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#define LOG_VRB(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[%s][%d]" fmt,__FUNCTION__,this->m_hwModule, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[%s][%d]" fmt,__FUNCTION__,this->m_hwModule, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF("[%s][%d]" fmt,__FUNCTION__,this->m_hwModule, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[%s][%d]" fmt,__FUNCTION__,this->m_hwModule, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("[%s][%d]" fmt,__FUNCTION__,this->m_hwModule, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (TwinMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt,__FUNCTION__, ##arg); } } while(0)


#define MODULE_ID_TO_STR(_module) ({\
    char *str;\
    switch (_module) {\
    case CAM_A: str = "<cam_a>"; break;\
    case CAM_B: str = "<cam_b>"; break;\
    case CAM_C: str = "<cam_c>"; break;\
    case CAM_A_TWIN_C: str = "<cam_a_twin_c>"; break;\
    case CAM_A_TRIPLE_B: str = "<cam_a_triple_b>"; break;\
    case CAM_A_TRIPLE_C: str = "<cam_a_triple_c>"; break;\
    case CAM_B_TWIN_C: str = "<cam_b_twin_c>"; break;\
    default: str = "<what the fuxk>"; break;\
    }\
    str;\
})


static TwinMgr_IMP gTwinMgrObj[CAM_MAX];



TwinMgr_IMP::TwinMgr_IMP()
{
    /// Init value

    m_hwModule = CAM_A;
    m_twinVHWModules.clear();
    m_occupiedModule.clear();

    // twin
    m_isTwin_AFO.init();
    m_isTwin_IMGO.init();
    m_isTwin_RRZO.init();

    m_hwModuleNum = eTwinMgr_none;

    m_pMainCmdQMgr = NULL;
    for (MUINT32 i = 0; i < ISP_DRV_CAM_BASIC_CQ_NUM; i++) {
        m_pActiveTwinCQMgrs[i].clear();
    }
    m_pMainIspDrv = NULL;

    m_FSM = op_unknown;
    m_pTwinDrv = NULL;

    m_DualInputParam.dualInCfg = NULL;
    m_DualInputParam.dualOutCfg = NULL;
    m_DualInputParam.dualIspReg = NULL;

    m_lateset_AFO = MFALSE;
    m_lateset_IMGO = MFALSE;
    m_lateset_RRZO = MFALSE;
    m_latestIsTwin = MFALSE;

    m_xmx.pix_mode_amxo = ePixMode_1;
    m_xmx.pix_mode_bmxo = ePixMode_1;
    m_xmx.pix_mode_rmxo = ePixMode_1;
    m_xmx.pix_mode_dmxo = ePixMode_1;
    m_xmx.pix_mode_dmxi = ePixMode_1;

    m_TG_size.w = 0;
    m_TG_size.h = 0;
    m_TG_size.dbn_en = 0;
    m_TG_size.bin_en = 0;

    DBG_LOG_CONFIG(imageio, TwinMgr);
}

TwinMgr_IMP* TwinMgr_IMP::createInstance(ISP_HW_MODULE hwModule)
{
    //BASE_LOG_DBG("[%s][%d] createInstance: ",__FUNCTION__,hwModule);

    switch (hwModule) {
    case CAM_A:
    case CAM_B:
        break;
    default:
        BASE_LOG_ERR("[%s][%d] Unsupported hwModule (Must in 0 ~ %d, Set it to be 0)",__FUNCTION__, hwModule, CAM_B);
        return NULL;
    }

    gTwinMgrObj[hwModule].CamPathPass1::m_hwModule = hwModule;

    return &gTwinMgrObj[hwModule];
}

void TwinMgr_IMP::destroyInstance()
{

    if(this->FSM_CHECK(op_unknown) == MFALSE)
        LOG_ERR("FSM check error, destroy may be abnormal");

    LOG_INF("destroyInstance: module(%d)", this->m_hwModule);

    /// Init value

    this->m_hwModule = CAM_A;
    this->m_twinVHWModules.clear();
    this->m_occupiedModule.clear();
    this->m_appliedTwinVMod.clear();


    // twin
    this->m_hwModuleNum = eTwinMgr_none;


    this->m_pMainCmdQMgr = NULL;
    for (MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++) {
        this->m_pActiveTwinCQMgrs[i].clear();
    }
    this->m_pMainIspDrv = NULL;

    this->FSM_UPDATE(op_unknown);
}


// FSM check
MBOOL TwinMgr_IMP::FSM_CHECK(MUINT32 op)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mFSMLock);

    switch(op){
        case op_unknown:
            switch(this->m_FSM){
                case op_stopTwin:
                case op_unknown://support ceateinstance and than detroy directly
                case op_pathctrl:    //if twin is not enabled, status will be kept at cfg stage
                case op_uninit: //for destroy
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        case op_pathctrl:
            /*  remove this FSM in order to support dynamic twin
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
                */
            break;
        case op_init:
            if(this->m_FSM != op_pathctrl)
                ret = MFALSE;
            break;
        case op_startTwin:
            switch(this->m_FSM){
                case op_init:
                case op_suspendTwin:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_stopTwin:
            switch(this->m_FSM){
                case op_startTwin:
                case op_suspendTwin:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_uninit:
            switch(this->m_FSM){
                case op_stopTwin:
                case op_init:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_runtwin:
            switch(this->m_FSM){
                case op_unknown:
                case op_stopTwin:
                case op_uninit:
                case op_pathctrl:
                case op_suspendTwin:
                    ret = MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case op_twininfo:
            switch(this->m_FSM){
                case op_unknown:
                    ret = MFALSE;
                    break;
                default:
                    break;
            }
            break;
        case op_suspendTwin:
            if(this->m_FSM != op_startTwin)
                ret = MFALSE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE)
        LOG_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);

    return ret;
}

MBOOL TwinMgr_IMP::FSM_UPDATE(MUINT32 op)
{
    Mutex::Autolock lock(this->mFSMLock);
    this->m_FSM = (E_FSM)op;

    return MTRUE;
}

MINT32 TwinMgr_IMP::init(DupCmdQMgr* pMainCmdQMgr)
{
    MINT32 ret = 0;

    if (this->FSM_CHECK(op_pathctrl) == MFALSE) {
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        ret = 1;
        goto _ERR_INIT;
    }

    ret = ((CamPathPass1 *)this)->init(pMainCmdQMgr);
    if (ret) {
        goto _ERR_INIT;
    }

    this->m_isTwin_AFO.init();
    this->m_isTwin_IMGO.init();
    this->m_isTwin_RRZO.init();

    for (MUINT32 i = 0; i < ISP_DRV_CAM_BASIC_CQ_NUM; i++) {
        m_pActiveTwinCQMgrs[i].clear();
    }

    this->m_appliedTwinVMod.clear();

_ERR_INIT:

    return ret;
}

MINT32 TwinMgr_IMP::uninit(void)
{
    if (this->getIsInitTwin()) {
        if(this->uninitTwin() == MFALSE){
            LOG_ERR("twinmgr uninit fail\n");
            return 1;
        }
    }

    return ((CamPathPass1 *)this)->uninit();
}

MINT32 TwinMgr_IMP::start(void* pParam)
{
    // If twin is enable, call startTwin() of TwinMgr to
    // 1. Compute the CQ data via twin_drv (dual_isp_driver)
    // 2. Only twin hwModule: flush whole isp setting into phy address
    if (this->getIsTwin() == MTRUE) {
        if (this->startTwin(MFALSE) == MFALSE) {
            LOG_ERR("Start Twin fail\n");
            return -1;
        }
    }

    return ((CamPathPass1 *)this)->start(pParam);
}

MINT32 TwinMgr_IMP::stop(void* pParam)
{
    MINT32 _step = 0;

    if (((CamPathPass1 *)this)->stop(pParam)) {
        _step = 1;
        goto _STOP_ERR;
    }

    if(this->getIsStartTwin() == MTRUE){
        if (this->stopTwin() == MFALSE) {
            _step = 2;
            goto _STOP_ERR;
        }
    }

    return 0;

_STOP_ERR:

    LOG_ERR("PathMgr stop fail @step %d", _step);

    return -1;
}


MINT32 TwinMgr_IMP::setP1Update(void)
{
    DMA_UFGO ufgo;
    DMA_UFEO ufeo;
    MUINT32 targetIdxMain;
    MBOOL off_uf_imgo,off_uf_rrzo;
    Path_BufCtrl::T_UF uf;
    vector<Path_BufCtrl::T_UF> v_uf;
    v_uf.clear();
    //need to check uf before runtwin when twin mode is enabled
    targetIdxMain = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    //need to check before run twin driver
    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        ufgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
        ufeo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
        ufgo.check(off_uf_rrzo);
        ufeo.check(off_uf_imgo);

        uf.dmachannel = _imgo_;
        uf.bOFF = off_uf_imgo;
        v_uf.push_back(uf);
        uf.dmachannel = _rrzo_;
        uf.bOFF = off_uf_rrzo;
        v_uf.push_back(uf);
    }


    if (this->getIsTwin() == MTRUE) {

        if(this->m_TBC.enque_UF_patch(&v_uf) == MFALSE){
            LOG_ERR("UF's patch is fail\n");
            return 1;
        }

        if (this->runTwinDrv() == MFALSE) {
            LOG_ERR("Run Twin drv fail\n");
            return 1;
        }

        //enque into HW
        this->m_TBC.enque_pop();

        if (this->updateTwin() == MFALSE){
            LOG_ERR("Update Twin CQ fail\n");
            return 1;
        }
    }
    else{
        if(this->m_PBC.enque_UF_patch(&v_uf) == MFALSE){
            LOG_ERR("UF's patch is fail\n");
            return 1;
        }

        this->m_PBC.enque_pop();
    }

    this->m_appliedTwinVMod = this->m_twinVHWModules;

    return ((CamPathPass1 *)this)->setP1Update();
}


// Must configPath() before call TwinMgr init()
// Config path according to input params, includes:
// (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
// Notice!!! Can configPath only at the 1st time in current design

MBOOL TwinMgr_IMP::pathControl(TWIN_MGR_PATH_CFG_IN_PARAM &cfgInParam)
{
    MBOOL       ret = MTRUE;
    MUINT32     moduleNum = (MUINT32)eTwinMgr_none;
    capibility  CamInfo;

    Mutex::Autolock lock(this->mTwinLock);//for protect occupied module


    if (this->FSM_CHECK(op_pathctrl) == MFALSE) {
        ret = MFALSE;
        goto EXIT;
    }

    LOG_DBG("cfgIn: isTwin:%d tg_crop(%dx%d), rrzo_bypass(%d)\n", cfgInParam.isTwin,
        cfgInParam.tg_crop_w, cfgInParam.tg_crop_h, cfgInParam.bypass_rrzo);

    /**
     * note:
     *   also need to make sure cfgOutParam.clk_level is started from 0. always use lowest clk, and notify clk request of isp to MMDVFS each frame.
     */

    switch (cfgInParam.slave_mod.size()) {
    case 0:
        this->m_hwModuleNum = eTwinMgr_none;
        break;
    case 1:
        this->m_hwModuleNum = eTwinMgr_TwinNum_2;
        break;
    case 2:
        this->m_hwModuleNum = eTwinMgr_TwinNum_3;
        break;
    default:
        ret = MFALSE;
        LOG_ERR("Unsupported twin slave module num: %d\n", cfgInParam.slave_mod.size());
        goto EXIT;
    }

    this->m_twinVHWModules.clear();
    this->m_occupiedModule.clear();

    if (cfgInParam.isTwin == MFALSE) {
        goto NOTWIN_EXIT;
    }

    for (MUINT32 i = 0; i < cfgInParam.slave_mod.size(); i++) {

        if (CamInfo.m_DTwin.GetDTwin() == MFALSE){
            switch (cfgInParam.slave_mod.at(i)) {
            case CAM_A:
            case CAM_B:
            case CAM_C:
                this->m_twinVHWModules.push_back(cfgInParam.slave_mod.at(i));
                this->m_occupiedModule.push_back(cfgInParam.slave_mod.at(i));
                break;
            default:
                ret = MFALSE;
                LOG_ERR("hwModule(%d) slaveModule(%d)/size(%d) is unsupported!\n", this->m_hwModule,
                    cfgInParam.slave_mod.at(i), cfgInParam.slave_mod.size());
                goto EXIT;
            }
        }
        else {
            ISP_HW_MODULE   virtModule = CAM_C;

            switch (cfgInParam.slave_mod.at(i)) {
            case CAM_C:
                switch (cfgInParam.slave_mod.size()) {
                case 1:
                    switch (this->m_hwModule) {
                    case CAM_A:
                        virtModule = CAM_A_TWIN_C;
                        break;
                    case CAM_B:
                        virtModule = CAM_B_TWIN_C;
                        break;
                    default:
                        LOG_ERR("Wrong master cam %d of cam c\n", this->m_hwModule);
                        ret = MFALSE;
                        goto EXIT;
                    }
                    break;
                case 2:
                    virtModule = CAM_A_TRIPLE_C;
                    break;
                default:
                    LOG_ERR("Wrong num %d of slave cam c\n", cfgInParam.slave_mod.size());
                    ret = MFALSE;
                    goto EXIT;
                }
                this->m_twinVHWModules.push_back(virtModule);
                this->m_occupiedModule.push_back(CAM_C);//phy HW
                break;
            case CAM_B:
                if (cfgInParam.slave_mod.size() != 2) {
                    LOG_ERR("Wrong num %d of slave cam b\n", cfgInParam.slave_mod.size());
                    ret = MFALSE;
                    goto EXIT;
                }
                this->m_twinVHWModules.push_back(CAM_A_TRIPLE_B);
                this->m_occupiedModule.push_back(CAM_B);//phy HW
                break;
            default:
                ret = MFALSE;
                LOG_ERR("hwModule(%d) slaveModule(%d) is unsupported!\n", this->m_hwModule, cfgInParam.slave_mod.at(i));
                goto EXIT;
            }
        }
    }

NOTWIN_EXIT:
    if (CamInfo.m_DTwin.GetDTwin()){//delete cq link 1st
        MUINT32 targetIdxMain;
        targetIdxMain = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

        for(MUINT32 i=0;i<this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].size();i++){

            if(this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(i)!= NULL){
                this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(i)->CmdQMgr_clearCQMode(this->m_pMainCmdQMgr,targetIdxMain);
            }
        }
    }

    this->twinInfoUpdate(cfgInParam, cfgInParam.isTwin);

    //keep tg ,bin info
    this->m_TG_size.w = cfgInParam.tg_crop_w;
    this->m_TG_size.h = cfgInParam.tg_crop_h;
    this->m_TG_size.dbn_en = cfgInParam.bDbn;
    this->m_TG_size.bin_en = cfgInParam.bBin;

    if (this->m_FSM == op_unknown) { //this if statement is add  for dynamic twin
        this->FSM_UPDATE(op_pathctrl);
    }

EXIT:

    return ret;
}


typedef struct _TWIN_MODULE_PAIR {
    ISP_HW_MODULE   virModule;
    ISP_HW_MODULE   phyModule;
} TWIN_MODULE_PAIR;
MBOOL TwinMgr_IMP::twinInfoUpdate(const TWIN_MGR_PATH_CFG_IN_PARAM& cfgInParam, MBOOL isTwin)
{
    MBOOL       ret = MTRUE;
    MUINT32     _size = 0, targetIdxMain;
    capibility  CamInfo;

    //assume afo is always needed at camera app
    this->m_latestIsTwin = isTwin;
    this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size == 0){// in order to simply logic in getistwin()
        RACING_PROT update(&this->m_lateset_AFO,isTwin);
    }
    this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_push,(MUINTPTR)&isTwin,0);
    if(_size > MAX_QUE_SIZE){
        this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_erase,1,0);
        LOG_DBG("no deque on AFO over %d frames\n",MAX_QUE_SIZE);
    }

    if(cfgInParam.bypass_imgo == MFALSE){
        this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);
        if(_size == 0){// in order to simply logic in getistwin()
            RACING_PROT update(&this->m_lateset_IMGO,isTwin);
        }
        this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_push,(MUINTPTR)&isTwin,0);
        if(_size > MAX_QUE_SIZE){
            this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_erase,1,0);
            LOG_WRN("no deque on IMGO over %d frames\n",MAX_QUE_SIZE);
        }
    }
    if(cfgInParam.bypass_rrzo == MFALSE){
        this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);
        if(_size == 0){// in order to simply logic in getistwin()
            RACING_PROT update(&this->m_lateset_RRZO,isTwin);
        }
        this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_push,(MUINTPTR)&isTwin,0);
        if(_size > MAX_QUE_SIZE){
            this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_erase,1,0);
            LOG_WRN("no deque on RRZO over %d frames\n",MAX_QUE_SIZE);
        }
    }

    if (((CamPathPass1*)this)->getCamPathState() == camstat_unknown) {
        LOG_INF("twinSlaveNum=%d. not yet config done.\n", this->m_twinVHWModules.size());
        goto _UPDATE_ERR;
    }

    //update twin's info into ifunc_cam
    //due to configpath is perframe invoked, we can also update twin's info perframe.
    //if cmdqmgr is null , it means current sw is at configuration phase, twin's info will be updated latter when initTwin.
    if (this->getIsInitTwin() == MFALSE) {
        // not yet init twin, at 1) configPipe or 2) 1st time single to twin or 3) static-twin
        if (this->m_pCmdQdrv == NULL) { //cam_path
            LOG_ERR("twinSlaveNum=%d main cqMgr is null\n", this->m_twinVHWModules.size());
            goto _UPDATE_ERR;
        }
        targetIdxMain = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

        for (MUINT32 burst = 0; burst < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); burst++) {
            this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdxMain)[burst];
            this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
            this->m_TwinCtrl.m_pTwinCB = cfgInParam.pTwinCB;
            this->m_TwinCtrl.updateTwinInfo(MFALSE, CAM_TWIN_PIPE::E_ENABLE_TWIN);
        }
        goto _UPDATE_ERR;
    }
    if (this->m_pMainCmdQMgr == NULL) {
        LOG_ERR("master cam cqmgr is ull\n");
        ret = MFALSE;
        goto _UPDATE_ERR;
    }

    //update twin's info into ifunc_cam
    //due to pathControl is perframe, we can also update twin's info perframe.
    //if cmdqmgr is null , it means current sw is at configuration phase, twin's info will be updated latter when initTwin.
    targetIdxMain = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();

    for (MUINT32 burst = 0; burst < this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ(); burst++) {

        this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[burst];
        this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
        this->m_TwinCtrl.m_pTwinCB = cfgInParam.pTwinCB;
        for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
            //using master cam's dupQidx due to slave cam may be suspended.
            this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
                this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i))->CmdQMgr_GetCurCycleObj(targetIdxMain)[burst]
            );
        }

        this->m_TwinCtrl.updateTwinInfo(isTwin, CAM_TWIN_PIPE::E_ENABLE_TWIN);
    }

    if (CamInfo.m_DTwin.GetDTwin()) {
        // 1. suspend/resume
        // 2. updateCQMode
        string                  dbg_str;
        vector<DupCmdQMgr *>    sorted;
        vector<ISP_HW_MODULE>   suspendTwinVMod;

        //note:!!!!
        //why need to check resume here? plz reference the comment at resumeTwin()
        this->m_TwinFlowCtrl.perFrameCheck();

        suspendTwinVMod.clear();
        if (this->m_appliedTwinVMod.size() || this->m_twinVHWModules.size()) { // non-twin case, through camio twinPath initTwin flow
            if (this->m_appliedTwinVMod.size() > this->m_twinVHWModules.size()) {
                // suspend some cam, 3 => 2 raw
                vector<DupCmdQMgr*> vec;
                string dbg_str = "cam: ";

                for (MUINT32 prev = 0; prev < this->m_appliedTwinVMod.size(); prev++) {
                    ISP_HW_MODULE prev_phy_cam = ModuleVirtToPhy(this->m_appliedTwinVMod.at(prev));
                    MUINT32 cur = 0;

                    for ( ; cur < this->m_twinVHWModules.size(); cur++) {
                        if (prev_phy_cam == ModuleVirtToPhy(this->m_twinVHWModules.at(cur))) {
                            break;
                        }
                    }
                    if (cur == this->m_twinVHWModules.size()) {
                        //not found in new twin cam list, suspened
                        suspendTwinVMod.push_back(this->m_appliedTwinVMod.at(prev));
                        dbg_str += string(MODULE_ID_TO_STR(this->m_appliedTwinVMod.at(prev)));
                    }
                }
                LOG_INF("suspending list %s\n", dbg_str.c_str());

                vec.push_back(this->m_pMainCmdQMgr);
                for (MUINT32 mod = 0; mod < suspendTwinVMod.size(); mod++) {
                    vec.push_back(this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(suspendTwinVMod.at(mod)));
                }

                this->m_TwinFlowCtrl.suspendInit(&vec, &this->m_TwinCtrl);

                this->m_TwinFlowCtrl.perFrameCheck();
            }
            else if (this->m_appliedTwinVMod.size() < this->m_twinVHWModules.size()) {
                // resume some cam, 2 => 3 raw
                vector<DupCmdQMgr*> vec;

                vec.push_back(this->m_pMainCmdQMgr);
                for (MUINT32 mod = 0; mod < this->m_twinVHWModules.size(); mod++) {
                    vec.push_back(this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(mod)));
                }

                this->m_TwinFlowCtrl.resumeInit(&vec, &this->m_TwinCtrl);

                this->m_TwinFlowCtrl.perFrameCheck();
            }
        }

        dbg_str = MODULE_ID_TO_STR(this->m_pMainCmdQMgr->CmdQMgr_GetCurModule());
        sorted.push_back(this->m_pMainCmdQMgr);
        for (MUINT32 cam = CAM_A; cam < PHY_CAM; cam++) {
            MUINT32 i;
            for (i = 0; i < this->m_twinVHWModules.size(); i++) {
                if (ModuleVirtToPhy(this->m_twinVHWModules.at(i)) == cam) {
                    break;
                }
            }
            if (i < this->m_twinVHWModules.size()) {
                sorted.push_back(this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i)));
                dbg_str += MODULE_ID_TO_STR(this->m_twinVHWModules.at(i));
            }
            else {
                for (i = 0; i < suspendTwinVMod.size(); i++) {
                    if (ModuleVirtToPhy(suspendTwinVMod.at(i)) == cam) {
                        break;
                    }
                }
                if (i < suspendTwinVMod.size()) {
                    sorted.push_back(this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(suspendTwinVMod.at(i)));
                    dbg_str += MODULE_ID_TO_STR(suspendTwinVMod.at(i));
                }
            }
        }
        if (sorted.size() != (1 + this->m_twinVHWModules.size() + suspendTwinVMod.size())) {
            LOG_ERR("[%d] wrong cqMgr twinVM:%d suspendVM:%d link[%d]:%s\n", this->m_pMainCmdQMgr->CmdQMgr_GetCurModule(),
                    this->m_twinVHWModules.size(), suspendTwinVMod.size(), sorted.size(), dbg_str.c_str());
            ret = MFALSE;
            goto _UPDATE_ERR;
        }
        LOG_INF("[%d] current twinVM:%d cqMgr link[%d]:%s\n", this->m_pMainCmdQMgr->CmdQMgr_GetCurModule(),
                this->m_twinVHWModules.size(), sorted.size(), dbg_str.c_str());

        for (MUINT32 i = 1; i < sorted.size(); i++) {
            sorted.at(i)->CmdQMgr_updateCQMode(sorted.at(i-1), targetIdxMain);
        }
    }

_UPDATE_ERR:

    return ret;
}

MBOOL TwinMgr_IMP::initTwin(DupCmdQMgr* pMainCmdQMgr,const NSImageio::NSIspio::PortID* pInput,MUINT32 subSample,list<MUINT32>* pChannle)
{
    MBOOL       ret = MTRUE;
    MUINT32     _size = 0, _size2 = 0, sum = 0;
    string      dbg_str;
    capibility  CamInfo;
    vector<vector<ISP_HW_MODULE>> twinCombinations;
    vector<DupCmdQMgr*> vPossibleTwinCqMgr;

    Mutex::Autolock lock(this->mTwinLock);

    LOG_DBG("channel size:%d subsample:%d\n", pChannle->size(), subSample);

    this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);
    this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size2,0);

    if ((_size + _size2) == 0) { //if twin is not needed, twin can't be init
        LOG_ERR("imgo/rrzo both enque %d_%d\n", _size, _size2);
        return MFALSE;
    }

    if (this->FSM_CHECK(op_init) == MFALSE) {
        return MFALSE;
    }

     // 1. Init Twin drv
    this->m_pTwinDrv = TwinDrv::createInstance(pMainCmdQMgr->CmdQMgr_GetCurModule());
    if (this->m_pTwinDrv == NULL) {
        LOG_ERR("Invalid twinDrv ptr\n");
        return MFALSE;
    }
    if (CamInfo.m_DTwin.GetDTwin()) {
        this->m_pTwinDrv->init(twin_non_af_mode);
    }
    else {
        this->m_pTwinDrv->init(twin_normal_mode);
    }

    this->m_DualInputParam.dualInCfg = (DUAL_IN_CONFIG_STRUCT*)this->m_pTwinDrv->getInCfg();
    this->m_DualInputParam.dualOutCfg = (DUAL_OUT_CONFIG_STRUCT*)this->m_pTwinDrv->getOutCfg();
    this->m_DualInputParam.dualIspReg = (ISP_REG_PTR_STRUCT*)this->m_pTwinDrv->getIspPtr();

    this->m_pMainCmdQMgr = pMainCmdQMgr;

    //create set of all possible cqMgrs during the preview
    if (CamInfo.m_DTwin.GetDTwin()) {
        switch (pMainCmdQMgr->CmdQMgr_GetCurModule()) {
        case CAM_A:
            twinCombinations.resize(2);
            twinCombinations.at(0).clear();
            twinCombinations.at(1).clear();
            twinCombinations.at(0).push_back(CAM_A_TWIN_C);
            twinCombinations.at(1).push_back(CAM_A_TRIPLE_B);
            twinCombinations.at(1).push_back(CAM_A_TRIPLE_C);
            break;
        case CAM_B:
            twinCombinations.resize(1);
            twinCombinations.at(0).clear();
            twinCombinations.at(0).push_back(CAM_B_TWIN_C);
            break;
        default:
            LOG_ERR("Unsupported master cam: %d\n", pMainCmdQMgr->CmdQMgr_GetCurModule());
            break;
        }
    }
    else {
        // static-twin should not change twinModules at runtime
        twinCombinations.resize(1);
        twinCombinations.at(0) = this->m_twinVHWModules;
    }

    this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].assign(CAM_MAX, NULL);
    if (subSample) {
        this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE11].assign(CAM_MAX, NULL);
    }
    else {
        this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE1].assign(CAM_MAX, NULL);
    }

    dbg_str = "cam: ";
    sum = 0;
    for (MUINT32 i = 0; i< twinCombinations.size(); i++) {
        for (MUINT32 j = 0; j < twinCombinations.at(i).size(); j++) {
            DupCmdQMgr      *pCqmgr;
            ISP_HW_MODULE   _mod = twinCombinations.at(i).at(j);

            pCqmgr = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps, subSample, _mod, ISP_DRV_CQ_THRE0);
            if (pCqmgr == NULL) {
                LOG_ERR("Twin CQ0 init fail twinModule: %d\n", _mod);
                return MFALSE;
            }

            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod) = pCqmgr;

            vPossibleTwinCqMgr.push_back(pCqmgr);

            if (subSample) {
                pCqmgr = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps, subSample,_mod,ISP_DRV_CQ_THRE11);
                if (pCqmgr == NULL) {
                    LOG_ERR("Twin CQ11 init fail twinModule: %d\n", _mod);
                    return MFALSE;
                }
                this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE11].at(_mod) = pCqmgr;
            }
            else { //cfg cq1 for ae smoothing
                pCqmgr = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps, subSample,_mod,ISP_DRV_CQ_THRE1);
                if (pCqmgr == NULL) {
                    LOG_ERR("Twin CQ1 init fail twinModule: %d\n", _mod);
                    return MFALSE;
                }
                this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE1].at(_mod) = pCqmgr;
            }
            sum++;

            dbg_str += MODULE_ID_TO_STR(_mod);
        }
    }

    LOG_INF("Total twin slave CqMgr depth:%d, possibleNum:%d %s\n", this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].size(), sum, dbg_str.c_str());


    this->m_TwinCtrl.m_subsample = subSample;

    for(MUINT32 dup = 0; dup < this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ(); dup++){
        for(MUINT32 bst = 0; bst < this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ(); bst++){

            for (MUINT32 i = 0; i< twinCombinations.size(); i++) {

                this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(dup)[bst];
                this->m_TwinCtrl.m_pTwinIspDrv_v.clear();

                for (MUINT32 j = 0; j < twinCombinations.at(i).size(); j++) {
                    ISP_HW_MODULE _mod = twinCombinations.at(i).at(j);

                    if ((this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ() !=
                            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod)->CmdQMgr_GetDuqQ())
                        ||
                        (this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ() !=
                            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod)->CmdQMgr_GetBurstQ()))
                    {
                        LOG_ERR("twinCQMgr error: master(dup:%d burst:%d) slave_%d(dup:%d burst:%d)",
                            this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ(), this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ(), _mod,
                            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod)->CmdQMgr_GetDuqQ(),
                            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod)->CmdQMgr_GetBurstQ());
                        return MFALSE;
                    }

                    this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
                        this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(_mod)->CmdQMgr_GetCurCycleObj(dup)[bst]
                    );
                }

                this->m_TwinCtrl.write2CQ();
                this->m_TwinCtrl.config();
            }

        }

    }

    //cfg twin fbc, this->m_TwinCtrl.m_pTwinIspDrv_v already pushed
    this->m_TBC.PBC_config(this, &vPossibleTwinCqMgr, pChannle, pInput);

    this->m_appliedTwinVMod = this->m_twinVHWModules;

    this->FSM_UPDATE(op_init);

EXIT:

    return ret;
}

MBOOL TwinMgr_IMP::getIsInitTwin()
{
    Mutex::Autolock lock(this->mFSMLock);

    if (this->m_FSM >= op_init) {
        return MTRUE;
    }
    else {
        return MFALSE;
    }
}

MBOOL TwinMgr_IMP::getIsStartTwin()
{
    Mutex::Autolock lock(this->mFSMLock);

    if (this->m_FSM >= op_startTwin) {
        return MTRUE;
    }
    else {
        return MFALSE;
    }
}

MBOOL TwinMgr_IMP::startTwin(MBOOL bStreaming)
{
    MBOOL ret = MTRUE;
    int i, dbg_cnt = 0;

    Mutex::Autolock lock(this->mTwinLock);

    if (this->FSM_CHECK(op_startTwin) == MFALSE) {
        return MFALSE;
    }
    if (this->m_twinVHWModules.size() == 0) {
        LOG_ERR("wrong twin slave mod size\n");
        return MFALSE;
    }

    LOG_INF("twin start slave size: %d masterCqIdx:%d\n", this->m_twinVHWModules.size(), this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx());

    /// No need to runTwinDrv, only need to do it in updateTwin
    // 1. Run TwinDrv to calcuate two CQs
    //if(MFALSE == this->runTwinDrv()) {
    //    return MFALSE;
    //}
    this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
        this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i))->CmdQMgr_GetCurCycleObj(0)[0]
        );
    }
    this->m_TwinCtrl.enable(NULL);

    for (MUINT32 mod = 0; mod < this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].size(); mod++) {
        if (this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(mod) != NULL) {
            if (this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(mod)->CmdQMgr_start()) {
                LOG_ERR("Twin[%d] CQ:0x%x stop fail\n", mod, ISP_DRV_CQ_THRE0);
                ret = MFALSE;
            }
        }
    }

    this->m_TBC.PBC_Start();

    this->FSM_UPDATE(op_startTwin);

    //this is for dynamic twin case when starttwin after sreaming on
    if(bStreaming){
        while (this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() !=
            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(0))->CmdQMgr_GetDuqQIdx())
        {
            this->updateTwin();
            dbg_cnt++;
        }
    }

    for (list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();it != this->m_occupiedModule.end();it++) {
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_main);
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_stt);
    }

    LOG_INF("twin start done masterCqIdx:%d updateTwinCnt:%d\n", this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx(), dbg_cnt);

    return ret;
}


MBOOL TwinMgr_IMP::stopTwin()
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_stopTwin) == MFALSE)
        return MFALSE;

    this->m_TBC.PBC_Stop();

    this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
        this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
            this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i))->CmdQMgr_GetCurCycleObj(0)[0]
        );
    }
    this->m_TwinCtrl.disable();

    for (MUINT32 i = 0; i < ISP_DRV_CAM_BASIC_CQ_NUM; i++) {
        for (MUINT32 mod = 0; mod < this->m_pActiveTwinCQMgrs[i].size(); mod++) {
            if (this->m_pActiveTwinCQMgrs[i].at(mod) != NULL) {
                if (this->m_pActiveTwinCQMgrs[i].at(mod)->CmdQMgr_stop()) {
                    LOG_ERR("Twin[%d] CQ:0x%x stop fail\n", mod, i);
                    ret = MFALSE;
                }
            }
        }
    }

    this->FSM_UPDATE(op_stopTwin);

    for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();it != this->m_occupiedModule.end();it++){
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, *it, CAM_BUF_CTRL::eCamDmaType_main);
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, *it, CAM_BUF_CTRL::eCamDmaType_stt);
    }
EXIT:
    return ret;
}

MINT32 TwinMgr_IMP::config(struct CamPathPass1Parameter* p_parameter)
{
    return ((CamPathPass1 *)this)->config(p_parameter);
}

MINT32 TwinMgr_IMP::setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L)
{
    return ((CamPathPass1 *)this)->setP1RrzCfg(pRrz_cfg_L);
}

MINT32 TwinMgr_IMP::setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L)
{
    return ((CamPathPass1 *)this)->setP1ImgoCfg(pImgo_cfg_L);
}

MINT32 TwinMgr_IMP::setP1TuneCfg(list<IspP1TuningCfg>* pTune_L)
{
    return ((CamPathPass1 *)this)->setP1TuneCfg(pTune_L);
}

MINT32 TwinMgr_IMP::setP1Notify(void)
{
    return ((CamPathPass1 *)this)->setP1Notify();
}

MINT32 TwinMgr_IMP::enqueueBuf(MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo)
{
    MBOOL bMultiChannel = MFALSE;

    switch (dmaChannel) {
    case _imgo_:
    case _rrzo_:
    case _ufeo_:
    case _ufgo_:
        bMultiChannel = MTRUE;
        break;
    default:
        bMultiChannel = MFALSE;
        break;
    }

    if (!bMultiChannel) {
        return ((CamPathPass1 *)this)->enqueueBuf(dmaChannel, bufInfo);
    }

    if (this->getIsTwin()) {
        return this->enqueTwin(dmaChannel, bufInfo);
    }
    else {
        return ((CamPathPass1 *)this)->enqueueBuf(dmaChannel, bufInfo);
    }
}

MINT32 TwinMgr_IMP::dequeueBuf(MUINT32 const dmaChannel, vector<NSImageio::NSIspio::BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify)
{
    MBOOL bMultiChannel = MFALSE;
    MINT32 ret = 0;

    switch (dmaChannel) {
    case _imgo_:
    case _rrzo_:
    case _ufeo_:
    case _ufgo_:
        bMultiChannel = MTRUE;
        break;
    default:
        bMultiChannel = MFALSE;
        break;
    }

    if (!bMultiChannel) {
        return ((CamPathPass1 *)this)->dequeueBuf(dmaChannel, bufInfo, pNotify);
    }

    if (this->getIsTwin(MTRUE)) {
        ret = this->dequeTwin(dmaChannel, bufInfo, pNotify);
    }
    else {
        ret = ((CamPathPass1 *)this)->dequeueBuf(dmaChannel, bufInfo, pNotify);
    }

    if (ret != -1) {
        this->popTwinRst(dmaChannel);
    }

    return ret;
}

MBOOL  TwinMgr_IMP::P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj)
{
    return ((CamPathPass1 *)this)->P1Notify_Mapping(cmd, pObj);
}

MBOOL  TwinMgr_IMP::HW_recover(E_CAMPATH_STEP op)
{
    if (this->getIsTwin() == MTRUE) {
        return this->recoverTwin(op);
    }
    else {
        return ((CamPathPass1 *)this)->recoverSingle(op);
    }
}

MBOOL  TwinMgr_IMP::updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset)
{
    return ((CamPathPass1 *)this)->updateFrameTime(timeInMs, reqOffset);
}

MBOOL  TwinMgr_IMP::updatePath(UpdatePathParam pathParm)
{
    return ((CamPathPass1 *)this)->updatePath(pathParm);
}


MBOOL TwinMgr_IMP::suspendTwin(E_SUSPEND_OP op)
{
    LOG_ERR("Not support now\n");
    return MFALSE;
}

MBOOL TwinMgr_IMP::resumeTwin(void)
{
    LOG_ERR("Not support now\n");
    return MFALSE;
}



MBOOL TwinMgr_IMP::uninitTwin()
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mTwinLock);

    if (this->FSM_CHECK(op_uninit) == MFALSE) {
        return MFALSE;
    }

    for (MUINT32 i = 0; i < ISP_DRV_CAM_BASIC_CQ_NUM; i++) {
        for (MUINT32 mod = 0; mod < this->m_pActiveTwinCQMgrs[i].size(); mod++) {
            if(this->m_pActiveTwinCQMgrs[i].at(mod) != NULL){
                if(this->m_pActiveTwinCQMgrs[i].at(mod)->CmdQMgr_detach()){
                    LOG_ERR("cam:%d twin cq:0x%x uninit fail\n", mod, i);
                    ret = MFALSE;
                }
            }
        }
        this->m_pActiveTwinCQMgrs[i].clear();
    }

    this->m_pTwinDrv->uninit();
    this->m_pTwinDrv->destroyInstance();

    this->m_DualInputParam.dualInCfg = NULL;
    this->m_DualInputParam.dualOutCfg = NULL;
    this->m_DualInputParam.dualIspReg = NULL;

    this->m_isTwin_IMGO.uninit();
    this->m_isTwin_RRZO.uninit();
    this->m_isTwin_AFO.uninit();

    this->m_hwModule = CAM_A;
    this->m_twinVHWModules.clear();
    this->m_occupiedModule.clear();

    this->m_hwModuleNum = eTwinMgr_none;

    this->m_pMainCmdQMgr = NULL;
    this->m_pMainIspDrv = NULL;

    this->FSM_UPDATE(op_uninit);
EXIT:
    return ret;

}

MBOOL TwinMgr_IMP::updateTwin()
{
    MBOOL ret = MTRUE;

    if (this->FSM_CHECK(op_runtwin) == MFALSE) {
        LOG_ERR("updateTwin: fail\n");
        ret = MFALSE;
        goto EXIT;
    }

    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
        if (this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i))->CmdQMgr_update()) {
            LOG_ERR("[%d]:Twin CQ undate fail\n", this->m_twinVHWModules.at(i));
            ret = MFALSE;
            goto EXIT;
        }
    }

EXIT:
    return ret;
}


MBOOL TwinMgr_IMP::getIsTwin(MBOOL bDeque)
{
    MBOOL isTwin;
    MUINT32 _size_img = 0,_size_rrz = 0,_size = 0;
    QueueMgr<MBOOL>* ptr = NULL;
    MBOOL* _ptr = NULL;
    //if fsm is false->twin is not enabled, other
    if(this->FSM_CHECK(op_twininfo) == MFALSE){
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        return MFALSE;
    }

    this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size_img,0);
    this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size_rrz,0);
    if(_size_img != 0){
        _size = _size_img;
        ptr = &this->m_isTwin_IMGO;
        _ptr = &this->m_lateset_IMGO;
    }
    else if(_size_rrz != 0){
        _size = _size_rrz;
        ptr = &this->m_isTwin_RRZO;
        _ptr = &this->m_lateset_RRZO;
    }
    else{
        LOG_ERR("can't run pipeline with both imgo/rrzo r closed\n");
        return MFALSE;
    }

    if(bDeque == MTRUE){
        if(_size <= 1){
            // 1 is from configpipe, 2 is from enque. so need > 1.
            LOG_ERR("logic error, size must be >1. 1 is from 1st configpath() in configpipe(), >1 is from enque");
            return MFALSE;
        }
        else{
            //always return the oldest info ,but not front(). front() is from configpipe()
            ptr->sendCmd(QueueMgr<MBOOL>::eCmd_at,1,(MUINTPTR)&isTwin);
        }
    }
    else{
        //always return latest info. if size = 1, maybe timing is at deque to next enque, or before start.
        if(_size == 1){
            RACING_PROT update(&isTwin,*_ptr);
        }
        else{
            //
            ptr->sendCmd(QueueMgr<MBOOL>::eCmd_back,(MUINTPTR)&isTwin,0);
        }
    }


    LOG_DBG("que size:%d,isTwin:%d,cur fsm:%d\n",_size,isTwin,this->m_FSM);
    return isTwin;
}

MBOOL TwinMgr_IMP::getIsTwin(MUINT32 dmaChannel,MBOOL bDeque)
{
    capibility  CamInfo;

    //if fsm is false->twin is not enabled, other
    if(this->FSM_CHECK(op_twininfo) == MFALSE){
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        return MFALSE;
    }

    switch(dmaChannel){
    case _afo_:
        break;
    default:
        LOG_ERR("dma:0x%x getIsTwin dmaChannel API not support\n", dmaChannel);
        return MFALSE;
    }

    if (CamInfo.m_DTwin.GetDTwin()) {
        LOG_ERR("dma:0x%x getIsTwin dmaChannel API not support in d-twin mode\n", dmaChannel);
        return MFALSE;
    }

    return this->m_latestIsTwin;
}

MBOOL TwinMgr_IMP::popTwinRst(MUINT32 dmaChannel)
{
    MBOOL ret = MTRUE;
    MUINT32 _size = 0;
    QueueMgr<MBOOL>* ptr = NULL;
    MBOOL* _ptr = NULL;
    MBOOL isTwin;

    //if fsm is false->twin is not enabled, other
    if(this->FSM_CHECK(op_twininfo) == MFALSE){
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        return MFALSE;
    }


    switch(dmaChannel){
        case _afo_:
            ptr = &this->m_isTwin_AFO;
            _ptr = &this->m_lateset_AFO;
            break;
        case _imgo_:
            ptr = &this->m_isTwin_IMGO;
            _ptr = &this->m_lateset_IMGO;
            break;
        case _rrzo_:
            ptr = &this->m_isTwin_RRZO;
            _ptr = &this->m_lateset_RRZO;
            break;
        default:
            LOG_ERR("unsupported dma:%d\n",dmaChannel);
            return MFALSE;
            break;
    }

    ptr->sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);

    if(_size > 2){//>2 is because of deque/enque is multi-thread
        ptr->sendCmd(QueueMgr<MBOOL>::eCmd_erase,1,0);//erase oldest info, excluding info push at configpipe()
    }
    else if(_size == 2){
        ptr->sendCmd(QueueMgr<MBOOL>::eCmd_at,1,(MUINTPTR)&isTwin);
        {
            RACING_PROT update(_ptr,isTwin);
        }
        ptr->sendCmd(QueueMgr<MBOOL>::eCmd_erase,1,0);
    }
    else{
#if D_TWIN_AF
        //do nothing. no need to pop.
#else
        LOG_ERR("logic error: %d should not deque before enque_%d\n",dmaChannel,_size);
        return MFALSE;
#endif
    }


    return ret;
}


///
/// run twin_drv: dual_cam_cal() to update two CQ

/// TODO:   1. . RAWI
MBOOL TwinMgr_IMP::runTwinDrv()
{
    MBOOL ret = MTRUE;
    MUINT32 targetIdxMain = 0;
    MUINT32 targetIdxTwin = 0;
    MUINT32 burstQ = 0;
    vector<ISP_DRV_CAM *> vTwinDrvPtr;


    if(MFALSE == this->FSM_CHECK(op_runtwin)) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }

    // Get m_pMainIspDrv & m_pTwinIspDrv from DupCmdQMgr
    targetIdxMain = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();


    burstQ = this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ();

    for (MUINT32 bidx = 0; bidx < burstQ; bidx++) {

        this->m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[bidx];

        this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainIspDrv;
        this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
        vTwinDrvPtr.clear();

        for (MUINT32 mod = 0; mod < this->m_twinVHWModules.size(); mod++) {
            DupCmdQMgr  *pCqMgr = this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(mod));
            targetIdxTwin = pCqMgr->CmdQMgr_GetDuqQIdx();

            vTwinDrvPtr.push_back(pCqMgr->CmdQMgr_GetCurCycleObj(targetIdxTwin)[bidx]);
            this->m_TwinCtrl.m_pTwinIspDrv_v.push_back(pCqMgr->CmdQMgr_GetCurCycleObj(targetIdxTwin)[bidx]);

            // Twin will modify the following values, need to keep its value before run twin for debugging
            // These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
            // Also, write register that are existed in physical but not in CQ. EX: TG cropping window
        }

        if (MFALSE == this->prepareDualInCfg()) {
            LOG_ERR("prepareDualInCfg() error!!!");
            ret = MFALSE;
            goto EXIT;
        }

        // Put register's base addr from DupCmdQMgr to Dual Isp, includes CAM_A/B and UNI
        if (MFALSE == this->cvtDupCmdQRegtoDualIspReg(vTwinDrvPtr)) {
            LOG_ERR("cvtDupCmdQRegtoDualIspReg() error!!!");
            ret = MFALSE;
            goto EXIT;
        }

        // Execute Twin drv to call dual_cal_platform() of dual_isp_driver.cpp to update CAM_A & CAM_B registers via Twin
        if (MFALSE == this->m_pTwinDrv->configTwinPara()) {
            LOG_ERR("TwinDrv: configTwinPara error!!!");
            ret = MFALSE;
            goto EXIT;
        }

        // Modify CAMB's CQ descriptor and
        // 1. Copy modules in main's CQ descriptor to twin's CQ descriptor
        // 2. Modify CAMB's registers physically

        //this->m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
        //this->m_pMainIspDrv is ready above
        //this->m_TwinCtrl.m_pTwinIspDrv_v is ready above

        if (this->m_TwinCtrl.updateTwinInfo(1, CAM_TWIN_PIPE::E_AFTER_TWIN)) {
            ret = MFALSE;
        }

    }

    LOG_DBG("-Update Twin!!");

EXIT:
    return ret;

}


MBOOL TwinMgr_IMP::cvtDupCmdQRegtoDualIspReg(vector<ISP_DRV_CAM *> &rTwinDrvs)
{
    MBOOL           ret = MTRUE;
    E_ISP_CAM_CQ    cq;
    MUINT32         page;
    ISP_HW_MODULE   module;

    if(NULL == this->m_pMainIspDrv) {
        LOG_ERR("m_pMainIspDrv is null!!");
        ret = MFALSE;
        goto EXIT;
    }

    if(rTwinDrvs.size() == 0) {
        LOG_ERR("TwinIspDrv is empty!!");
        ret = MFALSE;
        goto EXIT;
    }

    this->m_DualInputParam.dualIspReg->ptr_isp_reg = NULL;
    this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = NULL;
    this->m_DualInputParam.dualIspReg->ptr_isp_reg_t = NULL;

    this->m_pMainIspDrv->getCurObjInfo(&module, &cq, &page);
    switch (module) {
    case CAM_A:
        this->m_DualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        break;
    case CAM_B:
        this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        break;
    default:
        LOG_ERR("unsupported master cam : %d!!\n", module);
        return MFALSE;
    }

    for (MUINT32 i = 0; i < rTwinDrvs.size(); i++) {
        rTwinDrvs.at(i)->getCurObjInfo(&module, &cq, &page);
        switch (module) {
        case CAM_B:
        case CAM_A_TRIPLE_B:
            this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)rTwinDrvs.at(i)->getIspVirRegVirAddr();
            break;
        case CAM_C:
        case CAM_A_TWIN_C:
        case CAM_B_TWIN_C:
        case CAM_A_TRIPLE_C:
            this->m_DualInputParam.dualIspReg->ptr_isp_reg_t = (cam_c_reg_t*)rTwinDrvs.at(i)->getIspVirRegVirAddr();
            break;
        case CAM_A:
            this->m_DualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)rTwinDrvs.at(i)->getIspVirRegVirAddr();
            break;
        default:
            LOG_ERR("unsupported slave cam : %d!!\n", module);
            return MFALSE;
        }
    }

    this->m_DualInputParam.dualIspReg->ptr_isp_uni = (cam_uni_reg_t*)(this->m_pMainIspDrv->getIspVirRegVirAddr() + (CAM_BASE_RANGE_SPECIAL >> 2));

EXIT:
    return ret;
}



// Twin will modify the following values, need to keep its value before run twin for debugging
// These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
// Also, write register that are existed in physical but not in CQ. EX: TG cropping window
MBOOL TwinMgr_IMP::prepareDualInCfg()
{
    MBOOL   ret = MTRUE;
    MUINT32 dual_sel = 0;

    Mutex::Autolock lock(this->mTwinLock);

    if(NULL == this->m_pMainIspDrv) {
        LOG_ERR("m_pMainIspDrv is null!!");
        ret = MFALSE;
        goto EXIT;
    }

    if(this->m_TwinCtrl.updateTwinInfo(1, CAM_TWIN_PIPE::E_BEFORE_TWIN)){
        ret = MFALSE;
        goto EXIT;
    }

    switch(this->m_hwModule) {
    case CAM_A:
        dual_sel = 0x01;
        break;
    case CAM_B:
        dual_sel = 0x02;
        break;
    default:
        LOG_ERR("unsupported master cam hwModule:%d\n", this->m_hwModule);
        ret = MFALSE;
        goto EXIT;
    }
    for (list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();it != this->m_occupiedModule.end();it++) {
        switch(*it) {
        case CAM_A:
            dual_sel |= 0x10; //cam_a as slave cam
            break;
        case CAM_B:
            dual_sel |= 0x02;
            break;
        case CAM_C:
            dual_sel |= 0x04;
            break;
        default:
            LOG_ERR("unsupported slave cam hwModule:%d\n", *it);
            ret = MFALSE;
            goto EXIT;
        }
    }

    // DUAL_IN_CONFIG_STRUCT
    this->m_DualInputParam.dualInCfg->DEBUG.DUAL_LOG_EN = 0;
    this->m_DualInputParam.dualInCfg->DEBUG.DUAL_LOG_ID = 1;
    this->m_DualInputParam.dualInCfg->SW.DUAL_SEL = dual_sel;

    this->m_DualInputParam.dualInCfg->SW.TWIN_MODE_SDBLK_XNUM_ALL = this->m_TwinCtrl.m_lsc_win_num_x; // Can set to 0
    this->m_DualInputParam.dualInCfg->SW.TWIN_MODE_SDBLK_lWIDTH_ALL = this->m_TwinCtrl.m_lsc_lwidth; // Can set to 0

    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_IN_CROP_HT = 0; // This value can be set 0. But RRZ_VERT_STEP must be != 0
    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST = this->m_TwinCtrl.m_rrz_roi.x;
    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST = \
        this->m_TwinCtrl.m_rrz_roi.x + this->m_TwinCtrl.m_rrz_roi.w;
    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST += (this->m_TwinCtrl.m_rrz_roi.floatX != 0)?(1):(0);

    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST = \
        this->m_TwinCtrl.m_rrz_in.w - this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST;

    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_SUB_OFST = 0; // Can set to 0
    this->m_DualInputParam.dualInCfg->SW.TWIN_RRZ_OUT_WD = this->m_TwinCtrl.m_rrz_out_wd;

    this->m_DualInputParam.dualInCfg->SW.TWIN_AF_OFFSET = this->m_TwinCtrl.m_af_vld_xstart;
    this->m_DualInputParam.dualInCfg->SW.TWIN_AF_BLOCK_XNUM = this->m_TwinCtrl.m_af_x_win_num;
    this->m_DualInputParam.dualInCfg->SW.TWIN_RAWI_XSIZE = 0; /// TODO: Confirm with TC is size is pixel?? Set to 0 first cause RAWI is not support currently

EXIT:
    return ret;
}


MUINT32 TwinMgr_IMP::CQ_cfg(MINTPTR arg1, MINTPTR arg2)
{
    E_ISP_CAM_CQ cq;

    if (this->m_TwinCtrl.m_subsample) {
        cq = ISP_DRV_CQ_THRE11;
    }
    else {
        cq = ISP_DRV_CQ_THRE1;
    }

    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
        DupCmdQMgr *pCqMgr = this->m_pActiveTwinCQMgrs[cq].at(this->m_twinVHWModules.at(i));

        if (pCqMgr == NULL) {
            LOG_ERR("Null cmdMgr ptr cam:%d\m", this->m_twinVHWModules.at(i));
            return 1;
        }
        if (pCqMgr->CmdQMgr_Cfg(arg1,arg2)) {
            return 1;
        }
    }

    return 0;
}

MUINT32 TwinMgr_IMP::CQ_Trig(void)
{
    E_ISP_CAM_CQ cq;

    if (this->m_TwinCtrl.m_subsample) {
        cq = ISP_DRV_CQ_THRE11;
    }
    else {
        cq = ISP_DRV_CQ_THRE1;
    }

    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
        DupCmdQMgr *pCqMgr = this->m_pActiveTwinCQMgrs[cq].at(this->m_twinVHWModules.at(i));

        if (pCqMgr == NULL) {
            LOG_ERR("Null cmdMgr ptr cam:%d\m", this->m_twinVHWModules.at(i));
            return 1;
        }
        if (pCqMgr->CmdQMgr_start()) {
            return 1;
        }
    }

    return 0;
}

list<ISP_HW_MODULE> TwinMgr_IMP::getCurModule(void)
{
    Mutex::Autolock lock(this->mTwinLock);
    list<ISP_HW_MODULE>     occupied;//double buffer for occupiedmodule,for multitreahd
    occupied.clear();
    occupied.assign(this->m_occupiedModule.begin(),this->m_occupiedModule.end());

    return occupied;
}


MUINT32 TwinMgr_IMP::getTwinHeaderSize(MUINT32 dmaChannel)
{
    capibility CamInfo;
    tCAM_rst rst;

    if(CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                rst,E_CAM_HEADER_size) == MFALSE){
        LOG_ERR("header size error\n");
        return 0;
    }

    switch (dmaChannel) {
    case _imgo_:
    case _rrzo_:
    case _afo_:
        return ALIGN_HEAD_SIZE(rst.HeaderSize) * (PHY_CAM - CAM_A); //for 3raw max 2 slave
        break;
    case _pdo_:
        return 0;
        break;
    default:
        return 0;
        break;
    }

}

MINT32 TwinMgr_IMP::enqueTwin( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo)
{
    MINT32 ret = 0;

    if (this->m_TBC.enque_push(dmaChannel, bufInfo, &this->m_twinVHWModules) == MFALSE) {
        ret = 1;
    }

    return ret;
}

/* return value:
 1: already stopped
 0: sucessed
-1: fail */
MINT32 TwinMgr_IMP::dequeTwin( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify)
{
    if(this->getIsTwin(MTRUE) == MFALSE){
        LOG_ERR("twin mode is not enabled\n");
        return -1;
    }

    return this->m_TBC.deque(dmaChannel,bufInfo,pNotify);
}


DupCmdQMgr* TwinMgr_IMP::getTwinCmdQ(ISP_HW_MODULE module, E_ISP_CAM_CQ cq)
{
    if (this->FSM_CHECK(op_runtwin) == MFALSE) {
        return NULL;
    }
    if ((cq >= ISP_DRV_CAM_BASIC_CQ_NUM) || (module >= CAM_MAX)) {
        LOG_ERR("wrong cq:%d cam:%d\n", cq, module);
        return NULL;
    }
    if ((this->m_pActiveTwinCQMgrs[cq].size() != CAM_MAX)) {
        LOG_ERR("wrong twinCmdQMgrs[cq:%d] size: %d\n", cq, this->m_pActiveTwinCQMgrs[cq].size());
        return NULL;
    }
    if (this->m_pActiveTwinCQMgrs[cq].at(module) == NULL) {
        LOG_ERR("wrong null obj twinCmdQMgrs[cq:%d] cam:%d: %d\n", cq, module);
        return NULL;
    }

    return this->m_pActiveTwinCQMgrs[cq].at(module);
}

TwinMgr::E_TWIN_STATUS TwinMgr_IMP::getTwinStatuts(void)
{
    TwinMgr::E_TWIN_STATUS status;

    switch(this->m_FSM){
        case op_startTwin:
            status = TwinMgr::E_START;
            break;
        case op_init:
            status = TwinMgr::E_INIT;
            break;
        case op_suspendTwin:
            status = TwinMgr::E_SUSPEND;
            break;
        default:
            status = TwinMgr::E_NOTWIN;
            LOG_DBG("current fsm = %d\n",this->m_FSM);
            break;
    }

    return status;
}

MBOOL TwinMgr_IMP::getTwinALLCropinfo(vector<vector<ISP_HW_MODULE>>* pPath,vector<vector<STImgCrop>>* pcrop)
{
    MBOOL ret = MTRUE;
    vector<ISP_HW_MODULE> v_module;
    vector<STImgCrop> v_crop;
    STImgCrop crop;

    pPath->clear();
    pcrop->clear();
    v_module.clear();
    v_crop.clear();

    //support only cama+camb currently
    //following logic should be combied with the assignment of twin module at the end of configpath()
    //currently , support only twin + static bin ctrl.
    if(this->m_hwModule == CAM_A){
        //
        v_module.clear();
        //CAM_A + CAM_B
        v_module.push_back(CAM_A);
        v_module.push_back(CAM_B);
        pPath->push_back(v_module);

        //CAM_A of CAM_A+CAM_B
        crop.x = crop.y = 0;
        crop.w = (this->m_TG_size.w >>(this->m_TG_size.dbn_en + this->m_TG_size.bin_en))>> 1;    //this calculation is dealed with twin_drv
        crop.h = (this->m_TG_size.h >> this->m_TG_size.bin_en);
        v_crop.push_back(crop);
        //CAM_B of CAM_A+CAM_B
        v_crop.push_back(crop);
        pcrop->push_back(v_crop);

        LOG_INF("twin crop size:%d_%d_%d_%d\n",crop.x,crop.y,crop.w,crop.h);
    }
    else{
        //
        v_module.clear();
        //CAM_B + CAM_A
        v_module.push_back(CAM_B);
        v_module.push_back(CAM_A);
        pPath->push_back(v_module);

        crop.x = crop.y = 0;
        crop.w = (this->m_TG_size.w >>(this->m_TG_size.dbn_en + this->m_TG_size.bin_en))>> 1;    //this calculation is dealed with twin_drv
        crop.h = (this->m_TG_size.h >> this->m_TG_size.bin_en);
        v_crop.push_back(crop);

        v_crop.push_back(crop);
        pcrop->push_back(v_crop);

        LOG_INF("twin crop size:%d_%d_%d_%d\n",crop.x,crop.y,crop.w,crop.h);
    }

    return ret;
}

MBOOL TwinMgr_IMP::suspend(void)
{
    MBOOL ret = MTRUE;
    CAM_REG_CTL_TWIN_STATUS twin;
    capibility  CamInfo;

    ret = ((CamPathPass1*)this)->suspend(CamPathPass1::_CAMPATH_SUSPEND_STOPHW);
    if (ret == MFALSE) {
        LOG_ERR("suspend stop hw fail\n");
        return ret;
    }

    /* Ref to phy reg due to HW/CQ already stopped after cam_path:suspend(STOP_HW) */
    twin.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_TWIN_STATUS);
    if (twin.Bits.TWIN_EN == MTRUE) {
        string dbg_str = "list: ";
        for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
            DupCmdQMgr*     pCmdQ;
            ISP_HW_MODULE   phy_mod = ModuleVirtToPhy(this->m_twinVHWModules.at(i));

            if (phy_mod >= CAM_MAX) {
                LOG_ERR("wrong vir module: %d, skip\n", phy_mod);
                return MFALSE;
            }

            pCmdQ = this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i));

            ret = CAM_BUF_CTRL::suspend(phy_mod, pCmdQ->CmdQMgr_GetCurCycleObj(pCmdQ->CmdQMgr_GetDuqQIdx())[0], CAM_BUF_CTRL::eSus_HW_SW_STATE);
            if (ret == MFALSE) {
                LOG_ERR("twin suspend fail master:%d cur_slave:%d/%d_%d\n", this->m_pCmdQdrv->CmdQMgr_GetCurModule(),
                    pCmdQ->CmdQMgr_GetCurModule(), this->m_twinVHWModules.at(i), phy_mod);
                return ret;
            }
            dbg_str += MODULE_ID_TO_STR(this->m_twinVHWModules.at(i));

            if (CamInfo.m_DTwin.GetDTwin() == MFALSE)
                pCmdQ->CmdQMgr_suspend();
        }
        LOG_INF("suspending twin module %s\n", dbg_str.c_str());

        if (ret == MFALSE) {
            LOG_ERR("twin suspend fail\n");
            return ret;
        }
    }

    ret = ((CamPathPass1*)this)->suspend(CamPathPass1::_CAMPATH_SUSPEND_FLUSH);
    if (ret == MFALSE) {
        LOG_ERR("suspend flush fail\n");
        return ret;
    }

    return ret;
}


MBOOL TwinMgr_IMP::resume(void)
{
    MBOOL ret = MTRUE;
    CAM_REG_CTL_TWIN_STATUS twin;
    string dbg_str = "list: ";
    capibility  CamInfo;

    LOG_INF("exit suspending mode:\n");

    if (ret == MTRUE) {//if twin mode fail, no need to run master cam
        ret = ((CamPathPass1*)this)->resume();
    }

    twin.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_TWIN_STATUS);
    if (twin.Bits.TWIN_EN == MTRUE) {
        DupCmdQMgr*     pCmdQ;
        for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
            ret = CAM_BUF_CTRL::resume(ModuleVirtToPhy(this->m_twinVHWModules.at(i)),
                        NULL);

            if (ret == MFALSE) {
                LOG_ERR("suspending fail at : %d", i);
                break;
            }

            dbg_str += MODULE_ID_TO_STR(this->m_twinVHWModules.at(i));

            pCmdQ = this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(i));
            if (CamInfo.m_DTwin.GetDTwin() == MFALSE)
                pCmdQ->CmdQMgr_resume();
        }
    }

    LOG_INF("resume module %s\n", dbg_str.c_str());

    return ret;
}

MBOOL TwinMgr_IMP::recoverTwin(E_CAMPATH_STEP step)
{
    UniMgr UniMgr;

    LOG_DBG("enter recoverTwin:%d\n",step);

    switch(step){
        case eCmd_path_stop:
            //sequence : master cam 1st, then slave cam.
            //master cam
            this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
            this->m_TopCtrl.HW_recover(0);

            this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
            this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
            for (MUINT32 mod = 0; mod < this->m_twinVHWModules.size(); mod++) {
                this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
                    this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(mod))->CmdQMgr_GetCurCycleObj(0)[0]
                );
            }

            return this->m_TwinCtrl.HW_recover(0);
            break;
        case eCmd_path_restart:
            //sequence : slave cam 1st, then master cam.
            //slave cam
            this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
            this->m_TwinCtrl.m_pTwinIspDrv_v.clear();
            for (MUINT32 mod = 0; mod < this->m_twinVHWModules.size(); mod++) {
                this->m_TwinCtrl.m_pTwinIspDrv_v.push_back (
                    this->m_pActiveTwinCQMgrs[ISP_DRV_CQ_THRE0].at(this->m_twinVHWModules.at(mod))->CmdQMgr_GetCurCycleObj(0)[0]
                );
            }

            this->m_TwinCtrl.HW_recover(1);
            //master cam
            this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
            return this->m_TopCtrl.HW_recover(1);
            break;
        default:
            LOG_ERR("unsupported:%d\n",step);
            break;
    }
    return MFALSE;
}

MBOOL TwinMgr_IMP::getTwinReg(vector<ISP_HW_MODULE>* pMaster, vector<MUINT32>* pAddr)
{
    MBOOL ret = MTRUE;
    MUINT32 addr;

    pMaster->clear();
    pAddr->clear();

    switch(this->m_hwModule) {
    case CAM_A:
        //currently only CAM_A is master cam
        pMaster->push_back(CAM_A);

        ret = CAM_TWIN_PIPE::GetTwinRegAddr(CAM_A,&addr);

        pAddr->push_back(addr);
        break;
    case CAM_B:
        //currently only CAM_A is master cam
        pMaster->push_back(CAM_B);

        ret = CAM_TWIN_PIPE::GetTwinRegAddr(CAM_B,&addr);

        pAddr->push_back(addr);
        break;
    default:
        ret = MFALSE;
        break;
    }

    return ret;
}


MBOOL TwinMgr_IMP::sendCommand(E_TWIN_CMD cmd,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3)
{
    MBOOL ret = MTRUE;

    switch(cmd){
        case TWIN_CMD_GET_TWIN_REG:
            {
                vector<ISP_HW_MODULE> v_module;
                vector<MUINT32> v_addr;

                if((ret = this->getTwinReg(&v_module,&v_addr)) == MFALSE){
                    goto EXIT;
                }
                else{
                    if( (arg1==0) || (arg2==0) || (arg3==0) ){
                        ret = MFALSE;
                        goto EXIT;
                    }
                    for(MUINT32 i=0;i<v_module.size();i++){
                        if(v_module.at(i) == CAM_A)
                            *(MUINT32*)arg1 = v_addr.at(i);
                        if(v_module.at(i) == CAM_B)
                            *(MUINT32*)arg2 = v_addr.at(i);
                        #if 0
                        if(v_module.at(i) == CAM_C)
                            *(MUINT32*)arg3 = v_addr.at(i);
                        #endif
                    }
                }
            }
            break;
        case TWIN_CMD_GET_TWIN_CROPINFO:
            if( (arg1==0) || (arg2==0) ){
                ret = MFALSE;
                goto EXIT;
            }
            ret = this->getTwinALLCropinfo((vector < vector < ISP_HW_MODULE >>*)arg1,(vector < vector < STImgCrop >>*)arg2);

            break;
        case TWIN_CMD_GET_TWIN_STATE:
            if(arg1==0){
                ret = MFALSE;
                goto EXIT;
            }
            *(E_TWIN_STATUS*)arg1 = this->getTwinStatuts();
            break;
        case TWIN_CMD_GET_SLAVE_HEADERSIZE:
            if(arg2==0){
                ret = MFALSE;
                goto EXIT;
            }
            *(MUINT32*)arg2 = this->getTwinHeaderSize((_isp_dma_enum_)arg1);
            break;
        case TWIN_CMD_GET_CUR_SLAVE_CQMGR: // d-twin not support this cmd
            if ((arg2==0) || (arg3==0)) {
                ret = MFALSE;
                goto EXIT;
            }
            else {
                capibility  CamInfo;

                if (CamInfo.m_DTwin.GetDTwin()) {
                    LOG_ERR("Not support under-dtwin due to cqMgr will dynamically change\n");
                    goto EXIT;
                }

                if((ret = this->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,arg2,0,0)) == MFALSE){
                    goto EXIT;
                }

                ((vector<DupCmdQMgr*>*)arg3)->clear();

                do {
                    Mutex::Autolock lock(this->mTwinLock);
                    for (MUINT32 i = 0; i < this->m_twinVHWModules.size(); i++) {
                        ((vector<DupCmdQMgr*>*)arg3)->push_back(this->m_pActiveTwinCQMgrs[(E_ISP_CAM_CQ)arg1].at(this->m_twinVHWModules.at(i)));
                    }
                } while(0);
            }
            break;
        case TWIN_CMD_GET_CUR_TWINMODULE:
            if (arg1 == 0) {
                ret = MFALSE;
                goto EXIT;
            }
            *(list<ISP_HW_MODULE>*)arg1 = this->getCurModule();
            break;
        default:
            ret = MFALSE;
            break;
    }
EXIT:
    if(ret == MFALSE){
        LOG_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%" PRIXPTR ",0x%" PRIXPTR ",0x%" PRIXPTR ")", cmd, arg1, arg2, arg3);
    }
    return ret;
}


MBOOL TwinFlowCheck::resumeInit(vector<DupCmdQMgr*>* pvector, CAM_TWIN_PIPE* pTwinpipe)
{
    MBOOL ret = MTRUE;

    if ((pvector == NULL) || (pTwinpipe == NULL)) {
        return MFALSE;
    }

    this->m_pCmdQ = *pvector;

    this->m_hwModule = pvector->at(0)->CmdQMgr_GetCurModule();

    this->m_loop = pvector->at(0)->CmdQMgr_GetDuqQ();

    this->m_pTwinpipe = pTwinpipe;

    this->m_twinFlowState = E_TWIN_FLOW_RESUME;

    for (MUINT32 slave = 1; slave < this->m_pCmdQ.size(); slave++) {
        //align twin cqMgr dupIdx to master cam
        DupCmdQMgr *pCqMgr = this->m_pCmdQ.at(slave);

        if (pCqMgr == NULL) {
            LOG_ERR("[%d] twinCqMgr cam:%d is null\n", this->m_pCmdQ.at(0)->CmdQMgr_GetCurModule(), this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule());
            ret = MFALSE;
            break;
        }

        //reset dupQ idx for dbg msg alignment
        pCqMgr->CmdQMgr_update_IMME(this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQIdx());
    }

    return ret;
}

MBOOL TwinFlowCheck::suspendInit(vector<DupCmdQMgr*>* pvector, CAM_TWIN_PIPE* pTwinpipe)
{
    MBOOL ret = MTRUE;

    if ((pvector == NULL) || (pTwinpipe == NULL)) {
        return MFALSE;
    }

    this->m_pCmdQ = *pvector;

    this->m_hwModule = pvector->at(0)->CmdQMgr_GetCurModule();

    this->m_loop = 2;

    this->m_pTwinpipe = pTwinpipe;

    this->m_twinFlowState = E_TWIN_FLOW_SUSPEND;

    return ret;
}

MBOOL TwinFlowCheck::perFrameCheck(void)
{
    MBOOL   ret = MTRUE;
    MUINT32 targetIdxMain = 0;
    MUINT32 targetIdxTwin = 0;

    if( (this->m_pCmdQ.size() < 1) || (this->m_pTwinpipe == NULL) || (this->m_twinFlowState == E_TWIN_FLOW_NONE)) {
        goto EXIT;
    }

    if (this->m_loop) {//this loop is the same size of cmdQ ring-buffer size . for CPU saving after all pages r resumed.
        //note:!!!!
        //why need to check resume here? plz reference the comment at resumeTwin()
        if (this->m_twinFlowState == E_TWIN_FLOW_RESUME) {
            if (this->m_loop == this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQ()) {
                string dbg_str = "resume : ";

                for (MUINT32 slave = 1; slave < this->m_pCmdQ.size(); slave++) {
                    dbg_str += MODULE_ID_TO_STR(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule());
                    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable,
                        ModuleVirtToPhy(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule()), CAM_BUF_CTRL::eCamDmaType_main);
                    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable,
                        ModuleVirtToPhy(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule()), CAM_BUF_CTRL::eCamDmaType_stt);
                }

                LOG_INF("[%d] twin flow start %s\n", this->m_pCmdQ.at(0)->CmdQMgr_GetCurModule(), dbg_str.c_str());
            }
        }

        targetIdxMain = this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQIdx();

        for (MUINT32 i = 0; i < this->m_pCmdQ.at(0)->CmdQMgr_GetBurstQ(); i++){

            this->m_pTwinpipe->m_pIspDrv = (IspDrvVir*)this->m_pCmdQ.at(0)->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
            this->m_pTwinpipe->m_pTwinIspDrv_v.clear();
            for (MUINT32 slave = 1; slave < this->m_pCmdQ.size(); slave++) {
                targetIdxTwin = this->m_pCmdQ.at(slave)->CmdQMgr_GetDuqQIdx();
                this->m_pTwinpipe->m_pTwinIspDrv_v.push_back(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurCycleObj(targetIdxTwin)[i]);
            }

            if (this->m_twinFlowState == E_TWIN_FLOW_RESUME) {
                this->m_pTwinpipe->resume();
            }
            else if (this->m_twinFlowState == E_TWIN_FLOW_SUSPEND) {
                this->m_pTwinpipe->suspend();
            }
        }

        this->m_loop--;
        if (this->m_loop == 0) {
            if (this->m_twinFlowState == E_TWIN_FLOW_SUSPEND) {
                string dbg_str = "suspend : ";

                for (MUINT32 slave = 1; slave < this->m_pCmdQ.size(); slave++) {
                    dbg_str += MODULE_ID_TO_STR(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule());
                    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspending,
                        ModuleVirtToPhy(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule()), CAM_BUF_CTRL::eCamDmaType_main);
                    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspending,
                        ModuleVirtToPhy(this->m_pCmdQ.at(slave)->CmdQMgr_GetCurModule()), CAM_BUF_CTRL::eCamDmaType_stt);
                }

                LOG_INF("[%d] twin flow done %s\n", this->m_pCmdQ.at(0)->CmdQMgr_GetCurModule(), dbg_str.c_str());
            }

            this->m_twinFlowState = E_TWIN_FLOW_NONE;
        }

        //check if need change cam_state/stt_state if m_loop == 0
    }


EXIT:
    return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
TwinMgr* TwinMgr::createInstance(ISP_HW_MODULE hwModule)
{
    return (TwinMgr*)TwinMgr_IMP::createInstance(hwModule);
}

char* TwinMgr::ModuleName(ISP_HW_MODULE hwModule)
{
    char *TwinName = NULL;

    switch (hwModule) {
    case CAM_A:
        TwinName = "TwinMgr_CAMA";
        break;
    case CAM_B:
        TwinName = "TwinMgr_CAMB";
        break;
    case CAM_C:
        TwinName = "TwinMgr_CAMC";
        break;
    default:
        TwinName = "TwinMgr_Unknown";
        //LOG_ERR("Unknown module: %d\n", hwModule);
        break;
    }

    return (char*)TwinName;
}


