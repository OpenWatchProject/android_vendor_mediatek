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

#include "twin_mgr.h"
#include "twin_drv_reg.h"
#include "dual_isp_config.h"


#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

//#define TWIN_EVEREST_EP_TEST // define this only for ep test cause ion mem not enough

using namespace std;


DECLARE_DBG_LOG_VARIABLE(TwinMgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#define LOG_VRB(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (TwinMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (TwinMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#ifndef TWIN_EVEREST_EP_TEST
    #define TG_CROP_WIDTH_MAX   CAM_BNR_MAX_LINE_BUFFER_IN_PIXEL
#else
/// JUST FOR TEST: Reduce the size for test, cause ion size will be not enough
    #define TG_CROP_WIDTH_MAX   3000 // for test only
#endif

#define RRZ_OUT_WIDTH_MAX   CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL

typedef struct
{
    DUAL_IN_CONFIG_STRUCT*  dualInCfg;
    DUAL_OUT_CONFIG_STRUCT* dualOutCfg;
    ISP_REG_PTR_STRUCT*     dualIspReg;

}TWIN_DUAL_INPUT_PARAM;

TwinMgr::TwinMgr()
{
    /// Init value

    this->m_hwModule = CAM_A;
    this->m_twinHwModule = CAM_MAX;
    this->m_occupiedModule.clear();

    // tg
    this->m_bypass_tg = MFALSE;
    this->m_pix_mode_tg = eTwinMgr_1_Pix;
    this->m_tg_crop_w = 0;

    // rrz
    this->m_bypass_rrzo = MFALSE;
    this->m_max_rrz_out_w = 0;
    this->m_rrz_out_w = 0;

    // pixel mode of mux
    this->m_pix_mode_dmxi = eTwinMgr_1_Pix;
    this->m_pix_mode_dmxo = eTwinMgr_1_Pix;
    this->m_pix_mode_bmxo = eTwinMgr_1_Pix;
    this->m_pix_mode_rmxo = eTwinMgr_1_Pix;


    // frontal binning
    this->m_dbn_en = 0;
    this->m_bin_en = 0;

    // twin
    this->m_isTwin = MFALSE;
    this->m_hwModuleNum = 0;

    this->mConfigCount = 0;
    this->mInitTwinCount = 0;

    this->m_pMainCmdQMgr = NULL;
    this->m_pMainCQ5CmdQMgr = NULL;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        this->m_pTwinCmdQMgr[i] = NULL;
    this->m_pMainIspDrv = NULL;
    this->m_pTwinIspDrv = NULL;

    this->m_FSM = op_unknown;
    this->m_pTwinDrv = NULL;
    DBG_LOG_CONFIG(imageio, TwinMgr);
}

//current, only 1 dual_isp drv instance , so 1 gDualInputParam is enough.
static TWIN_DUAL_INPUT_PARAM           gDualInputParam = {NULL,NULL,NULL};

static TwinMgr gTwinMgrObj[CAM_MAX];
TwinMgr* TwinMgr::createInstance(ISP_HW_MODULE hwModule)
{
    LOG_INF("createInstance: module(%d)", hwModule);

    if(hwModule >= CAM_MAX || hwModule < 0)
    {
        LOG_ERR("Unsupported hwModule(%d) (Must in 0 ~ %d, Set it to be 0)", hwModule, CAM_MAX - 1);

        hwModule = CAM_A;
    }

    gTwinMgrObj[hwModule].m_hwModule = hwModule;
    return &gTwinMgrObj[hwModule];
}

void TwinMgr::destroyInstance()
{
    if(this->FSM_CHECK(op_unknown) == MFALSE)
        LOG_ERR("FSM check error, destroy may be abnormal");

    LOG_INF("destroyInstance: module(%d)", this->m_hwModule);

    /// Init value

    this->m_hwModule = CAM_A;
    this->m_twinHwModule = CAM_MAX;
    this->m_occupiedModule.clear();

    // tg
    this->m_bypass_tg = MFALSE;
    this->m_pix_mode_tg = eTwinMgr_1_Pix;
    this->m_tg_crop_w = 0;

    // rrz
    this->m_bypass_rrzo = MFALSE;
    this->m_max_rrz_out_w = 0;
    this->m_rrz_out_w = 0;

    // pixel mode of mux
    this->m_pix_mode_dmxi = eTwinMgr_1_Pix;
    this->m_pix_mode_dmxo = eTwinMgr_1_Pix;
    this->m_pix_mode_bmxo = eTwinMgr_1_Pix;
    this->m_pix_mode_rmxo = eTwinMgr_1_Pix;


    // frontal binning
    this->m_dbn_en = 0;
    this->m_bin_en = 0;

    // twin
    this->m_isTwin = MFALSE;
    this->m_hwModuleNum = 0;

    this->mConfigCount = 0;
    this->mInitTwinCount = 0;

    this->m_pMainCmdQMgr = NULL;
    this->m_pMainCQ5CmdQMgr = NULL;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        this->m_pTwinCmdQMgr[i] = NULL;
    this->m_pMainIspDrv = NULL;
    this->m_pTwinIspDrv = NULL;

    this->m_FSM = op_unknown;
}


// Must configPath() before call TwinMgr init()
// Config path according to input params, includes:
// (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
// Notice!!! Can configPath only at the 1st time in current design
MBOOL TwinMgr::configPath(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam)
{
    MBOOL ret = MTRUE;
    MUINT32 nRet = 0;
    const char Name[32] = {"TwinMgr"};

    Mutex::Autolock lock(this->mConfigParamLock);

    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    LOG_INF(" -E. mConfigCount(%d)", this->mConfigCount);

    //init outparam
    memset((char*)&cfgOutParam,0,sizeof(TWIN_MGR_PATH_CFG_OUT_PARAM));

    if(this->mConfigCount == 0)
    {
        // configPath

        if(cfgInParam.bypass_tg == MTRUE)
        {
            LOG_ERR("bypass_tg is true!!, twin_mgr support no RAWI currently! \n");
            goto EXIT;
        }

        // 1. DMXI: same with TG
        cfgOutParam.pix_mode_dmxi = cfgInParam.pix_mode_tg;

TWIN_CHECK:
        // 2. TWIN = 1, if
        //      (A) TG_CROP_W > 5376 or RRZ_OUT_WD > 4608
        //      (B) dmxi pixel mode = 4
        if(((cfgInParam.tg_crop_w > TG_CROP_WIDTH_MAX) || ((cfgInParam.bypass_rrzo == MFALSE) && (cfgInParam.max_rrz_out_w > RRZ_OUT_WIDTH_MAX))) \
           || (eTwinMgr_4_Pix == cfgOutParam.pix_mode_dmxi) || \
           (cfgOutParam.isTwin == MTRUE))//this case is for rrzcheck
        {
            cfgOutParam.isTwin = MTRUE;

            this->m_hwModuleNum = eTwinMgr_TwinNum_2; // two hwmodules

            // Assign twin module here
            switch(this->m_hwModuleNum){
                case eTwinMgr_TwinNum_2:
                    switch(this->m_hwModule)
                    {
                        case CAM_A:
                            this->m_twinHwModule = CAM_B;
                            this->m_occupiedModule.push_back(CAM_B);
                            // Occupied CAM_B, Move this step to CamIOPipe::configPipe(). Not here, for avoding recursive library include
                            //this->m_pCamIOPipe[CAM_B] = ICamIOPipe::createInstance((MINT8 const*)Name, ICamIOPipe::TG_B, ICamIOPipe::CAMIO);
                            break;
                        case CAM_B:
                            #if 0   //twin_drv support only CAM_A + CAM_B, and the order must be CAM_A+CAM_B, can't be CAM_B+CAM_A
                            // Occupied CAM_A, Move this step to CamIOPipe::configPipe(). Not here, for avoding recursive library include
                            this->m_twinHwModule = CAM_A;
                            //this->m_pCamIOPipe[CAM_A] = ICamIOPipe::createInstance((MINT8 const*)Name, ICamIOPipe::TG_A, ICamIOPipe::CAMIO);
                            #else
                            LOG_ERR("hwModule(%d) is unsupported!", this->m_hwModule);
                            ret = MFALSE;
                            goto EXIT;
                            #endif
                            break;
                        default:
                            LOG_ERR("hwModule(%d) is unsupported!", this->m_hwModule);
                            ret = MFALSE;
                            goto EXIT;
                    }
                    break;
                default:
                    LOG_ERR("unsupported twin number_%d\n",this->m_hwModuleNum);
                    return MFALSE;
                    break;
            }

        }
        else
        {
            cfgOutParam.isTwin = MFALSE;
        }

        // 3. DMXO:
        //      (a) if TWIN = 0, DMXO = DMXI;
        //      (b) if TWIN = 1, DMXO = DMXI/2
        cfgOutParam.pix_mode_dmxo = (cfgOutParam.isTwin == MFALSE) ? cfgOutParam.pix_mode_dmxi : cfgOutParam.pix_mode_dmxi / 2;
        if(cfgOutParam.pix_mode_dmxo >= eTwinMgr_unknown)
        {
            LOG_WRN("pix_mode_dmxo(%d) unknown!!, set it to 1 pixel", cfgOutParam.pix_mode_dmxo);
            cfgOutParam.pix_mode_dmxo = eTwinMgr_1_Pix;
        }

        // 4. BIN_EN: if DMXO = 2-pix or 4-pix, BIN_EN = 1; otherwise, BIN_EN = 0
        if(cfgInParam.bDbn == MTRUE){
            if(cfgOutParam.pix_mode_dmxo == eTwinMgr_1_Pix)
                LOG_ERR("DBN with 1-pix is not verified yet!\n");
            cfgOutParam.dbn_en = 1;
            cfgOutParam.bin_en = 0;//dbn&bin is exclusive
        }
        else{
            cfgOutParam.bin_en = (cfgOutParam.pix_mode_dmxo > eTwinMgr_1_Pix) ? 1 : 0;
            // DBN need to use specific sensor cause don't use it in normal case when tg 4-pix. => change to enable twin
            //cfgOutParam.dbn_en= (cfgOutParam.pix_mode_dmxo > eTwinMgr_2_Pix) ? 1 : 0;
            cfgOutParam.dbn_en= 0; // always let to to be 0
        }

        // 5. BMXO = DMXI - DMXO = DMXI - BIN - DBN
        // Max is 2-pix
        cfgOutParam.pix_mode_bmxo = cfgOutParam.pix_mode_dmxi - cfgOutParam.pix_mode_dmxo;
        if(cfgOutParam.pix_mode_bmxo >= eTwinMgr_unknown) {
            LOG_WRN("pix_mode_bmxo(%d) unknown!!, set it to 1 pixel", cfgOutParam.pix_mode_bmxo);
            cfgOutParam.pix_mode_bmxo = eTwinMgr_1_Pix;
        }

        // 6. RMXO = BMXO
        cfgOutParam.pix_mode_rmxo = cfgOutParam.pix_mode_bmxo;

        ///
        nRet = this->chkRRZOutSize(cfgInParam, cfgOutParam);
        if(nRet == 2){
            ret = MTRUE;
            goto TWIN_CHECK;
        }

        // 7. for to disable frontal binning if needed
        if(cfgInParam.offBin == MTRUE){
            if(cfgInParam.pix_mode_tg != eTwinMgr_4_Pix){
                if(cfgOutParam.bin_en == MTRUE){
                    cfgOutParam.bin_en = MFALSE;
                    cfgOutParam.isTwin = MTRUE;
                    goto TWIN_CHECK;
                }
            }
            else{
                LOG_ERR("forced binning off fail,because of current pix mode is 4 pix mode\n");
                LOG_ERR("binning is still enabled\n");
            }
        }

        // Assign cfgInParam data to TwinMgr
        this->m_bypass_tg = cfgInParam.bypass_tg;
        this->m_pix_mode_tg = cfgInParam.pix_mode_tg;
        this->m_tg_crop_w = cfgInParam.tg_crop_w;
        this->m_bypass_rrzo = cfgInParam.bypass_rrzo;
        this->m_max_rrz_out_w = cfgInParam.max_rrz_out_w;

        // Assign cfgOutParam data to TwinMgr
        this->m_pix_mode_dmxi = cfgOutParam.pix_mode_dmxi;
        this->m_pix_mode_dmxo = cfgOutParam.pix_mode_dmxo;
        this->m_pix_mode_bmxo = cfgOutParam.pix_mode_bmxo;
        this->m_pix_mode_rmxo = cfgOutParam.pix_mode_rmxo;
        this->m_dbn_en = cfgOutParam.dbn_en;
        this->m_bin_en = cfgOutParam.bin_en;
        this->m_isTwin= cfgOutParam.isTwin;


        LOG_INF("cfgInParam: bypass_tg(%d), pix_mode_tg(%d), tg_crop_w(%d), bypass_rrzo(%d), max_rrz_out_w(%d)", \
                cfgInParam.bypass_tg, cfgInParam.pix_mode_tg, cfgInParam.tg_crop_w, cfgInParam.bypass_rrzo, cfgInParam.max_rrz_out_w);


        LOG_INF("cfgOutParam: pix_mode_dmxi(%d), pix_mode_dmxo(%d), pix_mode_bmxo(%d), pix_mode_rmxo(%d), dbn_en(%d), bin_en(%d), isTwin(%d)", \
                cfgOutParam.pix_mode_dmxi, cfgOutParam.pix_mode_dmxo, cfgOutParam.pix_mode_bmxo, cfgOutParam.pix_mode_rmxo, \
                cfgOutParam.dbn_en, cfgOutParam.bin_en, cfgOutParam.isTwin);

    }
    else
    {
        LOG_WRN("mConfigCount > 0, do nothing!!");
    }

    android_atomic_inc(&this->mConfigCount);
    this->FSM_UPDATE(op_cfg);

EXIT:
    return ret;
}


// FSM check
MBOOL TwinMgr::FSM_CHECK(MUINT32 op)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mFSMLock);

    switch(op){
        case op_unknown:
            switch(this->m_FSM){
                case op_stopTwin:
                case op_cfg:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_cfg:
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
            break;
        case op_startTwin:
            if(this->m_FSM != op_cfg)
                ret = MFALSE;
            break;
        case op_stopTwin:
            switch(this->m_FSM){
                case op_startTwin:
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
    if(ret == MFALSE)
        LOG_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);

    return ret;
}

MBOOL TwinMgr::FSM_UPDATE(MUINT32 op)
{
    Mutex::Autolock lock(this->mFSMLock);
    this->m_FSM = (E_FSM)op;

    return MTRUE;
}

MBOOL TwinMgr::initTwin(DupCmdQMgr* pMainCmdQMgr, MUINT32 fps,MUINT32 subSample)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);


    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }


     // 1. Init Twin drv
    this->m_pTwinDrv = TwinDrv::createInstance();
    this->m_pTwinDrv->init();

    gDualInputParam.dualInCfg = (DUAL_IN_CONFIG_STRUCT*)this->m_pTwinDrv->getInCfg();
    gDualInputParam.dualOutCfg = (DUAL_OUT_CONFIG_STRUCT*)this->m_pTwinDrv->getOutCfg();
    gDualInputParam.dualIspReg = (ISP_REG_PTR_STRUCT*)this->m_pTwinDrv->getIspPtr();


    //cfg cmdQ
    if((this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_attach(fps, \
        subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE0)) == NULL){
        LOG_ERR("Twin CQ0 init fail\n");
        this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0] = NULL;
        return MFALSE;
    }

    //cfg cmdq11
    if(subSample){
        if( (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE11] = DupCmdQMgr::CmdQMgr_attach(fps,\
            subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE11)) == NULL){
            LOG_ERR("Twin CQ11 init fail\n");
            this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE11] = NULL;
            return MFALSE;
        }
    }
    else{   //cfg cq1 for ae smoothing
        if( (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE1] = DupCmdQMgr::CmdQMgr_attach(fps,\
            subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE1)) == NULL){
            LOG_ERR("Twin CQ1 init fail\n");
            this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE1] = NULL;
            return MFALSE;
        }
    }

    this->m_pMainCmdQMgr = pMainCmdQMgr;


    this->m_TwinCtrl.m_hwModule = this->m_hwModule;
    this->m_TwinCtrl.m_twinHwModule = this->m_twinHwModule;
    this->m_TwinCtrl.m_subsample = subSample;


EXIT:
    return ret;
}
/*for fast af*/
MBOOL TwinMgr::initTwin(DupCmdQMgr* pAFMainCmdQMgr)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);


    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }

    this->m_pMainCQ5CmdQMgr = pAFMainCmdQMgr;

    //cfg cq5 for af fast fatch, here has no fps info, set it to 1
    if( (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE5] = DupCmdQMgr::CmdQMgr_attach(1,\
        this->m_TwinCtrl.m_subsample,this->m_twinHwModule,ISP_DRV_CQ_THRE5)) == NULL){
        LOG_ERR("Twin CQ5 init fail\n");
        this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE5] = NULL;
        return MFALSE;
    }

EXIT:
    return ret;
}


MBOOL TwinMgr::configTwin()
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);

    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }


    this->m_TwinCtrl.m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0];

    this->m_TwinCtrl.config();


EXIT:
    return ret;
}


MBOOL TwinMgr::startTwin(void)
{
    MBOOL ret = MTRUE;
    int i;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_startTwin) == MFALSE)
        return MFALSE;

    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }

    LOG_DBG(" -E. mInitTwinCount(%d)", this->mInitTwinCount);

    if(this->mInitTwinCount == 0)
    {

        /// No need to runTwinDrv, only need to do it in updateTwin
        // 1. Run TwinDrv to calcuate two CQs
        //if(MFALSE == this->runTwinDrv()) {
        //    return MFALSE;
        //}

        this->m_TwinCtrl.m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0];

        this->m_TwinCtrl.enable(NULL);


        // 4. start twin CQ
        if(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_start()) {
            LOG_ERR("Twin CQ start fail");
            ret = MFALSE;
            goto EXIT;

        }

        this->FSM_UPDATE(op_startTwin);
    }


EXIT:

    android_atomic_inc(&this->mInitTwinCount);
    return ret;
}


MBOOL TwinMgr::stopTwin()
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_stopTwin) == MFALSE)
        return MFALSE;

    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }

    android_atomic_dec(&this->mInitTwinCount);

    LOG_INF(" -E. mInitTwinCount(%d)", this->mInitTwinCount);

    if(this->mInitTwinCount == 0)
    {

        this->m_TwinCtrl.m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0];

        this->m_TwinCtrl.disable();

        //
        for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++){
            if(this->m_pTwinCmdQMgr[i] != NULL){
                if(this->m_pTwinCmdQMgr[i]->CmdQMgr_stop()) {
                    LOG_ERR("Twin CQ:0x%x stop fail\n",i);
                    ret = MFALSE;
                }
            }
        }
    }

    this->FSM_UPDATE(op_stopTwin);

EXIT:
    return ret;
}

MBOOL TwinMgr::uninitTwin()
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mTwinLock);


    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }

    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++){
        if(this->m_pTwinCmdQMgr[i] != NULL){
            if(this->m_pTwinCmdQMgr[i]->CmdQMgr_detach()){
                LOG_ERR("twin cq:0x%x uninit fail\n",i);
                ret = MFALSE;
            }
            this->m_pTwinCmdQMgr[i] = NULL;
        }
    }

    this->m_pTwinDrv->uninit();
    this->m_pTwinDrv->destroyInstance();

    gDualInputParam.dualInCfg = NULL;
    gDualInputParam.dualOutCfg = NULL;
    gDualInputParam.dualIspReg = NULL;

EXIT:
    return ret;

}



MBOOL TwinMgr::updateTwin()
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mUpdateTwinLock);

    if(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_update()){
        LOG_ERR("[%d]:Twin CQ undate fail\n", this->m_twinHwModule);
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    return ret;

}


MBOOL TwinMgr::getIsTwin()
{
    if(this->m_FSM == op_unknown) {
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        return MFALSE;
    }

    return this->m_isTwin;
}


///
/// run twin_drv: dual_cam_cal() to update two CQ

/// TODO:   1. . RAWI
MBOOL TwinMgr::runTwinDrv(MUINT32 scenario)
{
    MBOOL ret = MTRUE;

    MUINT32 targetIdxMain = 0;
    MUINT32 targetIdxTwin = 0;
    MUINT32 burstQ = 0;
    MUINT32 i;

    Mutex::Autolock lock(this->mConfigTwinLock);


    if(MFALSE == this->m_isTwin) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }


    // Get m_pMainIspDrv & m_pTwinIspDrv from DupCmdQMgr
    if(this->m_FSM == op_startTwin) {
        switch (scenario){
            case TWN_SCENARIO_AF_FAST_P2:
                targetIdxMain = targetIdxTwin = 0;
                break;
            case TWN_SCENARIO_NORMAL:
            case TWN_SCENARIO_AF_FAST_P1:
            default:
                targetIdxMain = ((this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() + 1) % this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ());
                targetIdxTwin = ((this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx() + 1) % this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ());
                break;
        }
    }
    // when m_FSM != op_startTwin, DupCmdQMgr is not start yet
    else {
        switch (scenario){
            case TWN_SCENARIO_AF_FAST_P2:
                targetIdxMain = targetIdxTwin = 0;
                break;
            case TWN_SCENARIO_NORMAL:
            case TWN_SCENARIO_AF_FAST_P1:
            default:
                targetIdxMain = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();
                targetIdxTwin = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx();
                break;
        }
    }

    switch (scenario){
        case TWN_SCENARIO_AF_FAST_P2:
            burstQ = 1;
            break;
        case TWN_SCENARIO_NORMAL:
        case TWN_SCENARIO_AF_FAST_P1:
        default:
            burstQ = this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ();
            break;
    }

    LOG_DBG("+Update Twin!! scenario(%d),targetIdxMain(%d), targetIdxTwin(%d), burstQ(%d)", scenario, targetIdxMain, targetIdxTwin, burstQ);

    for(i = 0; i < burstQ; i++)
    {
        switch (scenario){
            case TWN_SCENARIO_AF_FAST_P2:
                this->m_pMainIspDrv = this->m_pMainCQ5CmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
                this->m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE5]->CmdQMgr_GetCurCycleObj(targetIdxTwin)[i];
                break;
            case TWN_SCENARIO_NORMAL:
            case TWN_SCENARIO_AF_FAST_P1:
            default:
                this->m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
                this->m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(targetIdxTwin)[i];
            break;
        }

        this->m_TwinCtrl.m_pMainIspDrv = this->m_pMainIspDrv;
        this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinIspDrv;

        // Twin will modify the following values, need to keep its value before run twin for debugging
        // These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
        // Also, write register that are existed in physical but not in CQ. EX: TG cropping window
        if(MFALSE == this->prepareDualInCfg(scenario))
        {
            LOG_ERR("prepareDualInCfg() error!!!");
            ret = MFALSE;
            goto EXIT;
        }



        // Put register's base addr from DupCmdQMgr to Dual Isp, includes CAM_A/B and UNI
        if(MFALSE == this->cvtDupCmdQRegtoDualIspReg())
        {
            LOG_ERR("cvtDupCmdQRegtoDualIspReg() error!!!");
            ret = MFALSE;
            goto EXIT;
        }


        // Execute Twin drv to call dual_cal_platform() of dual_isp_driver.cpp to update CAM_A & CAM_B registers via Twin
            if(MFALSE == this->m_pTwinDrv->configTwinPara())
        {
                LOG_ERR("TwinDrv: configTwinPara error!!!");
            ret = MFALSE;
            goto EXIT;
        }

        // Modify CAMB's CQ descriptor and
        // 1. Copy modules in main's CQ descriptor to twin's CQ descriptor
        // 2. Modify CAMB's registers physically
        ret = this->updateTwinCQandReg();
    }

    LOG_DBG("-Update Twin!!");

EXIT:
    return ret;

}



// ret = 1 : pass
// ret = 0 : fail
// ret = 2 : re-cal from twin
MUINT32 TwinMgr::chkRRZOutSize(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam)
{
    MBOOL ret = 1;

    MUINT32 bin_status = cfgOutParam.bin_en;

    if(cfgInParam.max_rrz_out_w > (cfgInParam.tg_crop_w >> bin_status)) {
        LOG_INF("max_rrz_out_w(0x%x) > (tg_crop_w>>bin_status)(0x%x)!!!, tg_crop_w/dbn/bin(0x%x/%d/%d)\n", \
            cfgInParam.max_rrz_out_w, (cfgInParam.tg_crop_w >> bin_status), \
            cfgInParam.tg_crop_w, cfgOutParam.dbn_en, cfgOutParam.bin_en);

        /// Enable twin and close bin: when 1st time cfgInParam.max_rrz_out_w > (cfgInParam.tg_crop_w>> bin_status)
        ///       And check if it still cfgInParam.max_rrz_out_w > (cfgInParam.tg_crop_w>> bin_status),
        ///       If yes, return false, else, return true.
        if(cfgOutParam.dbn_en == 0 && cfgOutParam.bin_en == 1 && cfgOutParam.isTwin== 0)
        {
            cfgOutParam.bin_en = 0;
            cfgOutParam.isTwin = 1;

            cfgOutParam.pix_mode_dmxo = eTwinMgr_1_Pix;
            cfgOutParam.pix_mode_bmxo = eTwinMgr_2_Pix;
            cfgOutParam.pix_mode_rmxo = eTwinMgr_2_Pix;

            ret = 2;
            if(cfgInParam.max_rrz_out_w > cfgInParam.tg_crop_w)
            {
                LOG_ERR("Still chkRRZOutSize error!!! max_rrz_out_w(0x%x) > tg_crop_w(0x%x)!!!, tg_crop_w/dbn/bin(0x%x/%d/%d)\n", \
                cfgInParam.max_rrz_out_w, cfgInParam.tg_crop_w , \
                cfgInParam.tg_crop_w, cfgOutParam.dbn_en, cfgOutParam.bin_en);

                ret = 0;
            }
        }
        else
        {
            LOG_ERR("chkRRZOutSize error!!! \n");
            ret = 0;
        }
    }

EXIT:
    return ret;
}


MBOOL TwinMgr::cvtDupCmdQRegtoDualIspReg()
{
    MBOOL ret = MTRUE;

    if(NULL == this->m_pMainIspDrv) {
        LOG_ERR("m_pMainIspDrv is null!!");
        ret = MFALSE;
        goto EXIT;
    }

    if(NULL == this->m_pTwinIspDrv) {
        LOG_ERR("m_pTwinIspDrv is null!!");
        ret = MFALSE;
        goto EXIT;
    }


    if(CAM_A == this->m_hwModule) {
        gDualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        gDualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pTwinIspDrv->getIspVirRegVirAddr();
        gDualInputParam.dualIspReg->ptr_isp_uni = (cam_uni_reg_t*)(this->m_pMainIspDrv->getIspVirRegVirAddr() + (CAM_BASE_RANGE_SPECIAL >> 2));
    }
    else if(CAM_B == this->m_hwModule) {
        gDualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pTwinIspDrv->getIspVirRegVirAddr();
        gDualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        gDualInputParam.dualIspReg->ptr_isp_uni = (cam_uni_reg_t*)(this->m_pMainIspDrv->getIspVirRegVirAddr() + (CAM_BASE_RANGE_SPECIAL >> 2));
    }

EXIT:
    return ret;
}



// Twin will modify the following values, need to keep its value before run twin for debugging
// These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
// Also, write register that are existed in physical but not in CQ. EX: TG cropping window
MBOOL TwinMgr::prepareDualInCfg(MUINT32 scenario)
{
    MBOOL ret = MTRUE;

    if(NULL == this->m_pMainIspDrv) {
        LOG_ERR("m_pMainIspDrv is null!!");
        ret = MFALSE;
        goto EXIT;
    }

    if(this->m_TwinCtrl.update_beforeRunTwin()){
        ret = MFALSE;
        goto EXIT;
    }

    // DUAL_IN_CONFIG_STRUCT
    gDualInputParam.dualInCfg->DEBUG.DUAL_LOG_EN = 0;
    gDualInputParam.dualInCfg->DEBUG.DUAL_LOG_ID = 1;
    gDualInputParam.dualInCfg->SW.DUAL_SEL = 3;

    gDualInputParam.dualInCfg->SW.TWIN_MODE_SDBLK_XNUM_ALL = this->m_TwinCtrl.m_lsc_win_num_x; // Can set to 0
    gDualInputParam.dualInCfg->SW.TWIN_MODE_SDBLK_lWIDTH_ALL = this->m_TwinCtrl.m_lsc_lwidth; // Can set to 0

    gDualInputParam.dualInCfg->SW.TWIN_RRZ_IN_CROP_HT = 0; // This value can be set 0. But RRZ_VERT_STEP must be != 0
    gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST = this->m_TwinCtrl.m_rrz_roi.x;
    gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST = \
        this->m_TwinCtrl.m_rrz_roi.x + this->m_TwinCtrl.m_rrz_roi.w;
    gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST += (this->m_TwinCtrl.m_rrz_roi.floatX != 0)?(1):(0);

    gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST = \
        this->m_TwinCtrl.m_rrz_in.w - gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_INT_OFST_LAST;

    gDualInputParam.dualInCfg->SW.TWIN_RRZ_HORI_SUB_OFST = 0; // Can set to 0
    gDualInputParam.dualInCfg->SW.TWIN_RRZ_OUT_WD = this->m_TwinCtrl.m_rrz_out_wd;

    gDualInputParam.dualInCfg->SW.TWIN_AF_OFFSET = this->m_TwinCtrl.m_af_vld_xstart;
    gDualInputParam.dualInCfg->SW.TWIN_AF_BLOCK_XNUM = this->m_TwinCtrl.m_af_x_win_num;
    gDualInputParam.dualInCfg->SW.TWIN_RAWI_XSIZE = 0; /// TODO: Confirm with TC is size is pixel?? Set to 0 first cause RAWI is not support currently

    //
    gDualInputParam.dualInCfg->SW.TWIN_SCENARIO = scenario;
EXIT:
    return ret;
}


// 1. Update Twin's CQ descriptor according to Main's CQ descriptor
// 2. Set CAMB's registers
MBOOL TwinMgr::updateTwinCQandReg()
{
    MBOOL ret = MTRUE;
    MUINT32* main_IspDescript_vir;
    MUINT32* main_IspVirReg_phy;
    MUINT32* twin_IspDescript_vir;
    MUINT32* twin_IspVirReg_phy;
    MUINT32  afo_stride, afo_ofst, sgg;
    MUINT32 tmp;
    int i = 0;


    // 1. Update Twin's CQ descriptor according to Main's CQ descriptor
    main_IspDescript_vir = this->m_pMainIspDrv->getCQDescBufVirAddr();
    main_IspVirReg_phy = this->m_pMainIspDrv->getIspVirRegPhyAddr();

    twin_IspDescript_vir = this->m_pTwinIspDrv->getCQDescBufVirAddr();
    twin_IspVirReg_phy = this->m_pTwinIspDrv->getIspVirRegPhyAddr();

    // Copy Main's CQ descriptor to Twin's
    // Copy each CQ module separatly
    for(int i = 0; i < CAM_CAM_MODULE_MAX; i++)
    {
        if(CAM_A == this->m_hwModule) {
            // twin module is CAM_B: addr + 0x1000 of CAM_A
            tmp = *(main_IspDescript_vir + 2*i);
            if((tmp&CQ_DES_RANGE)>0x4000){//UNI r excluded
                *(twin_IspDescript_vir + 2*i) = tmp + 0x1000;
                *(twin_IspDescript_vir + (2*i + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*i + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
            }

        }
        else if (CAM_B == this->m_hwModule) {
            // twin module is CAM_A: addr + 0x1000 of CAM_B
            tmp = *(main_IspDescript_vir + 2*i);
            if((tmp&CQ_DES_RANGE)>0x5000){//UNI r excluded
                *(twin_IspDescript_vir + 2*i) = tmp - 0x1000;
                *(twin_IspDescript_vir + (2*i + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*i + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
            }
        }

#if 0
        LOG_INF("0x%x(0x%x), 0x%x(0x%x)", twin_IspDescript_vir + 2*i, *(twin_IspDescript_vir + 2*i), \
                twin_IspDescript_vir + (2*i + 1), *(twin_IspDescript_vir + (2*i + 1)));
#endif
    }


    if(this->m_TwinCtrl.update_afterRunTwin())
        ret = MFALSE;

EXIT:
    return ret;
}

MUINT32 TwinMgr::CQ_cfg(E_ISP_CAM_CQ cq, MINTPTR arg1, MINTPTR arg2)
{
    switch(cq){
        case ISP_DRV_CQ_THRE1:
            if(this->m_TwinCtrl.m_subsample)
                cq = ISP_DRV_CQ_THRE11;
            break;
        case ISP_DRV_CQ_THRE5:
        case ISP_DRV_CQ_THRE11:
            break;
        default:
            LOG_ERR("unsupported CQ:%d", cq);
            return 1;
            break;
    }

    if(this->m_pTwinCmdQMgr[cq]->CmdQMgr_Cfg(arg1,arg2) != 0)
        return 1;
    else
        return 0;
}

MUINT32 TwinMgr::CQ_Trig(E_ISP_CAM_CQ cq)
{
    switch(cq){
        case ISP_DRV_CQ_THRE1:
            if(this->m_TwinCtrl.m_subsample)
                cq = ISP_DRV_CQ_THRE11;
            break;
        case ISP_DRV_CQ_THRE5:
        case ISP_DRV_CQ_THRE11:
            break;
        default:
            LOG_ERR("unsupported CQ:%d", cq);
            return 1;
            break;
    }

    if(this->m_pTwinCmdQMgr[cq]->CmdQMgr_start() != 0)
        return 1;
    else
        return 0;
}

list<ISP_HW_MODULE>* TwinMgr::getCurModule(void)
{
    return &this->m_occupiedModule;
}

void TwinMgr::Twin_Lock(void)
{
    this->mConfigTwinLock.lock();
}

void TwinMgr::Twin_UnLock(void)
{
    this->mConfigTwinLock.unlock();
}

