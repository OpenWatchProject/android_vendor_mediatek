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



TwinMgr_IMP::TwinMgr_IMP()
{
    /// Init value

    m_hwModule = CAM_A;
    m_occupiedModule.clear();


    // twin
    m_isTwin_AFO.init();
    m_isTwin_IMGO.init();
    m_isTwin_RRZO.init();

    m_hwModuleNum = eTwinMgr_none;

    m_pMainCmdQMgr = NULL;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        m_pTwinCmdQMgr[i] = NULL;
    m_pMainIspDrv = NULL;
    m_pTwinIspDrv = NULL;

    m_FSM = op_unknown;
    m_pTwinDrv = NULL;

    m_switchLatency = 0;

    m_twinHwModule = CAM_MAX;
    m_DualInputParam.dualInCfg = NULL;
    m_DualInputParam.dualOutCfg = NULL;
    m_DualInputParam.dualIspReg = NULL;

    m_lateset_AFO = 0;
    m_lateset_IMGO = 0;
    m_lateset_RRZO = 0;

    m_xmx.pix_mode_amxo = _1_pix_;
    m_xmx.pix_mode_bmxo = _1_pix_;
    m_xmx.pix_mode_rmxo = _1_pix_;
    m_xmx.pix_mode_pmxo = _1_pix_;
    m_xmx.pix_mode_dmxo = _1_pix_;
    m_xmx.pix_mode_dmxi = _1_pix_;

    m_TG_size.w = 0;
    m_TG_size.h = 0;
    m_TG_size.dbn_en = 0;
    m_TG_size.bin_en = 0;

    DBG_LOG_CONFIG(imageio, TwinMgr);
}


static TwinMgr_IMP gTwinMgrObj[CAM_MAX];


TwinMgr_IMP* TwinMgr_IMP::createInstance(ISP_HW_MODULE hwModule)
{
    //BASE_LOG_DBG("[%s][%d] createInstance: ",__FUNCTION__,hwModule);

    if(hwModule == CAM_A_TWIN){
        BASE_LOG_ERR("[%s][%d] Unsupported hwModule (Must in 0 ~ %d, Set it to be 0 ",__FUNCTION__,hwModule, CAM_MAX - 1 -1); //-1 for virtual twin cam

        return NULL;
    }

    if(hwModule >= CAM_MAX || hwModule < 0)
    {
        BASE_LOG_ERR("[%s][%d] Unsupported hwModule (Must in 0 ~ %d, Set it to be 0 ",__FUNCTION__,hwModule, CAM_MAX - 1 -1); //-1 for virtual twin cam

        return NULL;
    }

    gTwinMgrObj[hwModule].m_hwModule = hwModule;
    return &gTwinMgrObj[hwModule];
}

void TwinMgr_IMP::destroyInstance()
{

    if(this->FSM_CHECK(op_unknown) == MFALSE)
        LOG_ERR("FSM check error, destroy may be abnormal");

    LOG_INF("destroyInstance: module(%d)", this->m_hwModule);

    /// Init value

    this->m_hwModule = CAM_A;
    this->m_twinHwModule = CAM_MAX;
    this->m_occupiedModule.clear();


    // twin
    this->m_hwModuleNum = eTwinMgr_none;


    this->m_pMainCmdQMgr = NULL;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        this->m_pTwinCmdQMgr[i] = NULL;
    this->m_pMainIspDrv = NULL;
    this->m_pTwinIspDrv = NULL;

    this->m_switchLatency = 0;

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
            if(this->m_FSM != op_stopTwin)
                ret = MFALSE;
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

MBOOL TwinMgr_IMP::initPath()
{
    MBOOL ret = MTRUE;

    if(this->FSM_CHECK(op_pathctrl) == MFALSE){
        ret = MFALSE;
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
    }

    this->m_isTwin_AFO.init();
    this->m_isTwin_IMGO.init();
    this->m_isTwin_RRZO.init();

    return ret;
}

// Must configPath() before call TwinMgr init()
// Config path according to input params, includes:
// (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
// Notice!!! Can configPath only at the 1st time in current design
MBOOL TwinMgr_IMP::configPath(const TWIN_MGR_PATH_CFG_IN_PARAM& cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam,MUINT32 navailCam)
{
    #define ClkMargin   95/100
    MBOOL ret = MTRUE;
    MUINT32 nRet = 0;
    const char Name[32] = {"TwinMgr"};
    MUINT32 thru_put = 0;
    MUINT32 bin_status = (cfgInParam.bDbn == MTRUE)?(1):(0);
    MUINT32 bmxo = 0;
    tCAM_rst rst;
    capibility CamInfo;
    MUINT32 moduleNum = (MUINT32)eTwinMgr_none;
    MUINT32 bBin_useful = 0xFF;
    MUINT32 bin_in_size;
    char str[512] = {'\0'}, _tmp[256] = {'\0'};
    MINT32 curLeng;

    Mutex::Autolock lock(this->mTwinLock);//for protect occupied module

    if(this->FSM_CHECK(op_pathctrl) == MFALSE){
        ret = MFALSE;
        goto EXIT;
    }

    //
    curLeng = snprintf(str, 511,\
            "cfgIn:tg_en(%d), tg_pix(%d), tg_crop(%dx%d),fps(%d), rrzo_en(%d), rrz_out(%d),cur clk:%d,off_bin:%d,avail_cam:%d\n", \
            cfgInParam.bypass_tg, cfgInParam.pix_mode_tg, cfgInParam.tg_crop_w, cfgInParam.tg_crop_h,cfgInParam.tg_fps,\
            cfgInParam.bypass_rrzo, cfgInParam.rrz_out_w,cfgInParam.vClk.at(cfgOutParam.clk_level),cfgInParam.offBin,navailCam);


    //init outparam
    memset((char*)&cfgOutParam,0,sizeof(TWIN_MGR_PATH_CFG_OUT_PARAM));
    /**
        note:
            also need to make sure cfgOutParam.clk_level is started from 0. always use lowest clk, and notify clk request of isp to MMDVFS each frame.
        */


    // configPath

    if(cfgInParam.bypass_tg == MTRUE)
    {
        LOG_ERR("bypass_tg is true!!, twin_mgr support no RAWI currently! \n");
        ret = MFALSE;
        goto EXIT;
    }

    // 0. total data size
    thru_put = (MUINT32) (((MUINT64)cfgInParam.tg_crop_w)*cfgInParam.tg_crop_h*cfgInParam.tg_fps / 10);
    if (cfgInParam.datPat == E_QUAD_CODE) {
        thru_put >>= bin_status;
    }
    if(thru_put == 0){
        LOG_ERR("thru put error,fps(x10):%d,size:%d_%d\n",cfgInParam.tg_fps,cfgInParam.tg_crop_w,cfgInParam.tg_crop_h);
        ret = MFALSE;
        goto EXIT;
    }
    else{
        if(thru_put > ((MUINT64)(1<<((MUINT32)cfgInParam.pix_mode_tg)) * cfgInParam.vClk.back() * ClkMargin)){
            LOG_ERR("output of spec.,thru-put(%d,tg pixmode:%d) over max speed:%d_%d\n",thru_put,cfgInParam.pix_mode_tg,\
                cfgInParam.vClk.back(),(MUINT32)((MUINT64)cfgInParam.vClk.back() * ClkMargin));
            ret = MFALSE;
            goto EXIT;
        }
    }

    // 1. dbn set
    if (cfgInParam.bDbn || (cfgInParam.datPat == E_QUAD_CODE)) {
        //DMXO should be configured as two pixel-out in Dual_PD case.
        bmxo++;
        if(cfgInParam.pix_mode_tg == _1_pix_)
            LOG_ERR("DMXI is one pixel-in but DMXO is two pixel-out. This path is not verified!!");
        //thru_put = thru_put>>1; //Because TG_crop w size in DualPD case will be divided by two first.Therefore, thr_put is no needed to divide by two.
        bBin_useful = MFALSE;
        navailCam = 0;
    }

    //2. check max size, if over max size, need enable twin
    //query pipeline size
    if(CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                rst,E_CAM_pipeline_size) == MFALSE){
        ret = MFALSE;
        goto EXIT;
    }

    switch (cfgInParam.SrcImgFmt) {
    case TG_FMT_RAW8:
    case TG_FMT_RAW10:
    case TG_FMT_RAW12:
    case TG_FMT_RAW14:
        if (cfgInParam.datPat == E_QUAD_CODE) {
            bin_in_size = (cfgInParam.tg_crop_w >> bin_status);
        }
        else {
            bin_in_size = cfgInParam.tg_crop_w;
        }

        if (bin_in_size > rst.pipeSize) {
            //rrz can't scale-up
            if (bin_status) {
                bBin_useful = MFALSE; //dbn, bin cannot both enable
            }
            else {
                bBin_useful = (cfgInParam.offBin == MTRUE) ? MFALSE :
                    ((cfgInParam.rrz_out_w <= (bin_in_size>>1)) ? (MTRUE):(MFALSE));
            }

            if(bBin_useful){
                bBin_useful = MFALSE;
                cfgOutParam.bin_en = MTRUE;
                thru_put = thru_put>>1;
                bmxo++;
                LOG_DBG("bin is enabled because of over max size:%d_%d\n",cfgInParam.tg_crop_w,rst.pipeSize);
            }
            else{
                cfgOutParam.isTwin = MTRUE; //twin must be enabled
                thru_put = thru_put>>1;  //twin path
                if(navailCam){
                    moduleNum++;
                    navailCam--;
                }
                else{
                    ret = MFALSE;
                    LOG_ERR("twin is enabled because of linebuffer , but availcam is not enough\n");
                    goto EXIT;
                }
                LOG_INF("twin is enabled because of over max size:%d_%d\n",cfgInParam.tg_crop_w,rst.pipeSize);
            }
        }
        //no rrz line buffer constraint , because of new hw module:RLB

        //3. check thru put
        {
            //rrz can't scale-up
            if(bBin_useful == 0xFF){//if bin is not enabled by over pipelinesize
                bBin_useful = (cfgInParam.offBin == MTRUE) ? MFALSE : \
                ((cfgInParam.rrz_out_w <= (cfgInParam.tg_crop_w>>(bin_status+1))) ? (MTRUE):(MFALSE));
            }
            //start calc. thru put
            //priority : 1:Bin.  2:Twin.  3:Clk.
            while(thru_put >= (MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * ClkMargin))
            {
                if(bBin_useful == MTRUE){   //1st
                    cfgOutParam.bin_en = MTRUE;
                    bBin_useful = MFALSE;
                    thru_put = thru_put>>1;
                    bmxo++;
                }
                else if(navailCam>0){               //2nd
                    cfgOutParam.isTwin = MTRUE;
                    navailCam--;
                    moduleNum++;
                    thru_put = thru_put / (1<<moduleNum);
                }
                else if(cfgOutParam.clk_level < (cfgInParam.vClk.size()-1)){    //3rd
                    cfgOutParam.clk_level += 1;
                }
                else{
                    LOG_ERR("over spec! thru put:(%dx%dx%d,total:%d), max clk:%d(margin:%d), bin_en(%d_%d), dbn_en(%d),twin_en(%d)\n",\
                        cfgInParam.tg_crop_w,cfgInParam.tg_crop_h,cfgInParam.tg_fps,thru_put,\
                        cfgInParam.vClk.at(cfgOutParam.clk_level),(MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * ClkMargin),\
                        cfgInParam.offBin,cfgOutParam.bin_en,\
                        cfgInParam.bDbn,\
                        cfgOutParam.isTwin);
                    ret = MFALSE;
                    goto EXIT;
                    break;
                }
            }

            this->m_hwModuleNum = (ETwinMgr_TwinNum)moduleNum;
        }
        break;
    default:
        LOG_INF("yuv path currently off twin\n");
        cfgOutParam.isTwin = MFALSE;
        bmxo = (MUINT32)cfgInParam.pix_mode_tg;
        break;
    }

    //
    if(cfgInParam.bypass_rrzo == MTRUE){
        if(cfgOutParam.isTwin == MTRUE){
            LOG_ERR("rrz is needed under twin mode\n");
            ret = MFALSE;
            goto EXIT;
        }

    }

    //4. xmx pix mode setting
    cfgOutParam.xmxo.pix_mode_dmxi = cfgInParam.pix_mode_tg;
    cfgOutParam.xmxo.pix_mode_dmxo = (E_PIX_MODE)bmxo;
    cfgOutParam.xmxo.pix_mode_bmxo = _1_pix_;
    cfgOutParam.xmxo.pix_mode_rmxo = _1_pix_;
    cfgOutParam.xmxo.pix_mode_pmxo = _1_pix_;
    cfgOutParam.xmxo.pix_mode_amxo = _1_pix_;

    //update occupied module info,reference "note1"
    if(this->m_switchLatency == 0)
        this->m_occupiedModule.clear();
    else if(this->m_switchLatency)
        --this->m_switchLatency;
    else
        LOG_ERR("logic error: \n");

    //
    if(cfgOutParam.isTwin == MTRUE){
        if(cfgOutParam.xmxo.pix_mode_dmxi > _1_pix_){
            cfgOutParam.xmxo.pix_mode_bmxo = _1_pix_;//always 1-pix mode, use dbn to reduce data thru. put.
            cfgOutParam.xmxo.pix_mode_rmxo = _2_pix_;
        }

        // Assign twin module here
        switch(this->m_hwModuleNum){
            case eTwinMgr_TwinNum_2:
                switch(this->m_hwModule)
                {
                    case CAM_A:
                        {
                            capibility CamInfo;
                            if(CamInfo.m_DTwin.GetDTwin() == MFALSE)
                                this->m_twinHwModule = CAM_B;        //vir HW for CQ
                            else
                                this->m_twinHwModule = CAM_A_TWIN;        //vir HW for CQ

                            if(this->m_occupiedModule.size() == 0){
                                this->m_occupiedModule.push_back(CAM_B);//phy HW
                            }
                            // Occupied CAM_B, Move this step to CamIOPipe::configPipe(). Not here, for avoding recursive library include
                            //this->m_pCamIOPipe[CAM_B] = ICamIOPipe::createInstance((MINT8 const*)Name, ICamIOPipe::TG_B, ICamIOPipe::CAMIO);
                        }
                        break;
                    case CAM_B:
                        {
                            capibility CamInfo;
                            if(CamInfo.m_DTwin.GetDTwin() == MFALSE)
                                this->m_twinHwModule = CAM_A;
                            else
                                this->m_twinHwModule = CAM_B_TWIN;

                            if(this->m_occupiedModule.size() == 0){
                                this->m_occupiedModule.push_back(CAM_A);//phy HW
                            }
                        }
                        break;
                    default:
                        LOG_ERR("hwModule(%d) is unsupported!", this->m_hwModule);
                        ret = MFALSE;
                        goto EXIT;
                }
                break;
            default:
                LOG_ERR("unsupported twin number_%d\n",this->m_hwModuleNum);
                ret = MFALSE;
                goto EXIT;
                break;
        }

        this->m_switchLatency = ENABLE_TWIN;
    }
    else{
        //note1:
        //marked, can't clear immediately, need to wait until pipline is running at single path
        //this->m_occupiedModule.clear();

        this->m_switchLatency = DISABLE_TWIN;
    }


    //
    this->m_xmx = cfgOutParam.xmxo;

    //
    this->TwinInfo_update(cfgInParam,cfgOutParam.isTwin);


    snprintf(_tmp, 255,\
        "[configPath] cfgOutParam: pix_mode_dmxi(%d), pix_mode_dmxo(%d), pix_mode_bmxo(%d), pix_mode_rmxo(%d), bin_en(%d), isTwin(%d),clk rate(%d), pipeSz(%d)", \
            cfgOutParam.xmxo.pix_mode_dmxi, cfgOutParam.xmxo.pix_mode_dmxo, cfgOutParam.xmxo.pix_mode_bmxo, cfgOutParam.xmxo.pix_mode_rmxo, \
            cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level), rst.pipeSize);
    strncat(str,_tmp, (511-curLeng));

    if(this->m_FSM == op_unknown)   //this if statement is add  for dynamic twin
        this->FSM_UPDATE(op_pathctrl);


    //keep tg ,bin info
    this->m_TG_size.w = cfgInParam.tg_crop_w;
    this->m_TG_size.h = cfgInParam.tg_crop_h;
    this->m_TG_size.dbn_en = cfgInParam.bDbn;
    this->m_TG_size.bin_en = cfgOutParam.bin_en;

EXIT:
    LOG_INF("%s\n", str);
    return ret;
}

MBOOL TwinMgr_IMP::TwinInfo_update(const TWIN_MGR_PATH_CFG_IN_PARAM& cfgInParam,MBOOL isTwin)
{
    MUINT32 _size;

    //assume afo is always needed at camera app
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

    //
    if((this->m_pMainCmdQMgr != NULL) && (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]!=NULL)){
        MUINT32 targetIdxMain;
        //update twin's info into ifunc_cam
        //due to configpath is perframe invoked, we can also update twin's info perframe.
        //if cmdqmgr is null , it means current sw is at configuration phase, twin's info will be updated latter when initTwin.

        if((this->m_FSM == op_startTwin) || (this->m_FSM == op_suspendTwin)) {
            targetIdxMain = ((this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() + 1) % this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ());
        }

        else {
            targetIdxMain = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();
        }

        for(MUINT32 j=0;j<this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();j++){

            this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[j];
            //using master cam's dupQidx due to slave cam may be suspended.
            this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(targetIdxMain)[j];

            this->m_TwinCtrl.updateTwinInfo(isTwin);
        }

        //
        //need to patch slave cam's dupQidx , and make it the same as master cam's dupQidx.
        //otherwise, slave cam's enque will set to a mismatch cq-page,comparing to master cam's cq-page.
        //patch here because of  invoked by configppath perframe.
        if( (isTwin == MFALSE) && (this->m_FSM == op_suspendTwin) ){
            if(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_update()){
                LOG_ERR("[%d]:Twin CQ undate fail\n", this->m_twinHwModule);
                return MFALSE;
            }
        }
    }

    return MTRUE;
}

MBOOL TwinMgr_IMP::initTwin(DupCmdQMgr* pMainCmdQMgr,const NSImageio::NSIspio::PortID* pInput,MUINT32 subSample,list<MUINT32>* pChannle)
{
    MBOOL ret = MTRUE;
    MUINT32 _size,_size2 = 0;
    capibility CamInfo;

    Mutex::Autolock lock(this->mTwinLock);

    this->m_isTwin_IMGO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);
    this->m_isTwin_RRZO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size2,0);

    if((_size+_size2) == 0)//if twin is not needed, twin can't be init
        return MFALSE;


    if(this->FSM_CHECK(op_init) == MFALSE)
        return MFALSE;


     // 1. Init Twin drv
    this->m_pTwinDrv = TwinDrv::createInstance();
    if(CamInfo.m_DTwin.GetDTwin())
        this->m_pTwinDrv->init(twin_non_af_mode);
    else
        this->m_pTwinDrv->init(twin_normal_mode);

    this->m_DualInputParam.dualInCfg = (DUAL_IN_CONFIG_STRUCT*)this->m_pTwinDrv->getInCfg();
    this->m_DualInputParam.dualOutCfg = (DUAL_OUT_CONFIG_STRUCT*)this->m_pTwinDrv->getOutCfg();
    this->m_DualInputParam.dualIspReg = (ISP_REG_PTR_STRUCT*)this->m_pTwinDrv->getIspPtr();

    //cfg cmdQ
    if((this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0] = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps, \
        subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE0)) == NULL){
        LOG_ERR("Twin CQ0 init fail\n");
        this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0] = NULL;
        return MFALSE;
    }

    //cfg cmdq11
    if(subSample){
        if( (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE11] = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps,\
            subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE11)) == NULL){
            LOG_ERR("Twin CQ11 init fail\n");
            this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE11] = NULL;
            return MFALSE;
        }
    }
    else{   //cfg cq1 for ae smoothing
        if( (this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE1] = DupCmdQMgr::CmdQMgr_attach(pInput->tgFps,\
            subSample,this->m_twinHwModule,ISP_DRV_CQ_THRE1)) == NULL){
            LOG_ERR("Twin CQ1 init fail\n");
            this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE1] = NULL;
            return MFALSE;
        }
    }

    this->m_pMainCmdQMgr = pMainCmdQMgr;

    this->m_TwinCtrl.m_subsample = subSample;

    for(MUINT32 i = 0;i<this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetBurstQ();j++){

            this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(i)[j];
            this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(i)[j];

            this->m_TwinCtrl.write2CQ();
            this->m_TwinCtrl.config();

        }
    }

    //cfg twin fbc
    this->m_TBC.TBC_config(&this->m_TwinCtrl,this->m_pMainCmdQMgr,\
        this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0],pChannle,pInput);

    //patch master cam's cq when twin is needed
    this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_updateCQMode();


    this->FSM_UPDATE(op_init);
    //update state
    {
        list<ISP_HW_MODULE>::iterator it;
        for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();it != this->m_occupiedModule.end();it++){
            CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_main);
            CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_stt);
        }
    }
EXIT:
    return ret;
}

MBOOL TwinMgr_IMP::getIsInit()
{
    Mutex::Autolock lock(this->mFSMLock);

    if (this->m_FSM >= op_init) {
        return MTRUE;
    }
    else {
        return MFALSE;
    }
}

MBOOL TwinMgr_IMP::getIsStart()
{
    Mutex::Autolock lock(this->mFSMLock);

    if (this->m_FSM >= op_startTwin) {
        return MTRUE;
    }
    else {
        return MFALSE;
    }
}

MBOOL TwinMgr_IMP::configTwin()
{//do nothing, this api is useless. exit for backward compeitible
    return MTRUE;
}


MBOOL TwinMgr_IMP::startTwin(void)
{
    MBOOL ret = MTRUE;
    int i;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_startTwin) == MFALSE)
        return MFALSE;

    /// No need to runTwinDrv, only need to do it in updateTwin
    // 1. Run TwinDrv to calcuate two CQs
    //if(MFALSE == this->runTwinDrv()) {
    //    return MFALSE;
    //}

    this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(0)[0];

    this->m_TwinCtrl.enable(NULL);


    // 4. start twin CQ
    if(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_start()) {
        LOG_ERR("Twin CQ start fail");
        ret = MFALSE;
        goto EXIT;

    }


    this->m_TBC.TBC_Start();
    this->FSM_UPDATE(op_startTwin);

    //this is for dynamic twin case when starttwin after sreaming on
    while(this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() != this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx()){
        this->updateTwin();
    }

EXIT:

    return ret;
}


MBOOL TwinMgr_IMP::stopTwin()
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_stopTwin) == MFALSE)
        return MFALSE;

    this->m_TBC.TBC_Stop();

    this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
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

    this->FSM_UPDATE(op_stopTwin);
    //update state
    {
        list<ISP_HW_MODULE>::iterator it;
        for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();it != this->m_occupiedModule.end();it++){
            CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, *it, CAM_BUF_CTRL::eCamDmaType_main);
            CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, *it, CAM_BUF_CTRL::eCamDmaType_stt);
        }
    }
EXIT:
    return ret;
}


MBOOL TwinMgr_IMP::suspendTwin(E_SUSPEND_OP op)
{
    MBOOL ret = MTRUE;
    MUINT32 burstQ;
    MUINT32 targetIdx = 0;
    MUINT32 i;

    Mutex::Autolock lock(this->mTwinLock);

    switch(op){
        case E_SUS_START:
            if(this->FSM_CHECK(op_suspendTwin) == MFALSE)
                return MFALSE;


            burstQ = this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ();


            //slave cam's page idx must be the same with master cam's under dynamic twin
            targetIdx = ((this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() + 1) % this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ());

            LOG_DBG("+suspendTwin!! targetIdx(%d), burstQ(%d)",  targetIdx, burstQ);

            //clr slave cam's cq
            for(i = 0; i < burstQ; i++){
                this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(targetIdx)[i];

                this->m_TwinCtrl.suspend();
            }


            //can't delete cq's descritpor here, need to wait until p1_done
            //this->m_pMainCmdQMgr->CmdQMgr_suspend();

            this->FSM_UPDATE(op_suspendTwin);

            //update BA itself,not via camio, because camio will get the infor of "not twin" from twin_mgr
            if(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_update()){
                LOG_ERR("[%d]:Twin CQ undate fail\n", this->m_twinHwModule);
                ret = MFALSE;
            }

            break;
        case E_SUS_READY:
            this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_suspend();
            //update state
            i=0;
            for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin(); \
                i<this->m_occupiedModule.size();\
                it++,i++){
                CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspend, *it, CAM_BUF_CTRL::eCamDmaType_main);
                CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspend, *it, CAM_BUF_CTRL::eCamDmaType_stt);
            }

            break;
        default:
            break;
    }
    return ret;
}

MBOOL TwinMgr_IMP::resumeTwin(void)
{
    MBOOL ret = MTRUE;
    MUINT32 burstQ;
    MUINT32 targetIdx = 0;
    MUINT32 i;

    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_startTwin) == MFALSE)
        return MFALSE;


    burstQ = this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ();

    //note:!!!!
    //slave cam's cq-page may not be the same with the page when suspend. so need to patch slave cam's cq page idx , must be the same with master cams'.
    //why can just update slave cam's page idx here? it may cause hw crash once if current master cam's page is update after hw_p1_done.
    //because normalpipe will check cq's update is fetched or not via CmdQMgr_GetCounter().
    while(this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() != this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx()){
        this->updateTwin();

    }
    targetIdx = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();


    //set slave cam's cq & enable master cam's twin_en
    for(i = 0; i < burstQ; i++){
        this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        this->m_TwinCtrl.resume();
    }



    //unlike suspend, not update BA itself, because camio will get the infor of "is twin" from twin_mgr
    //this->updateTwin();

    //add cq's descritpor
    this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_resume();

    //need to patch slave cam's dupQidx , and make it the same as master cam's dupQidx.
    //otherwise, slave cam's enque will set to a mismatch cq-page,compare to master cam's cq-page.
    //patch at twininfo_update , and invoked by configppath perframe.

    this->FSM_UPDATE(op_startTwin);

    //update state
    i=0;
    for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin(); \
        i<this->m_occupiedModule.size();\
        it++,i++){
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_main);
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, *it, CAM_BUF_CTRL::eCamDmaType_stt);
    }

    //init resume check, cause cmdq is a ring-buffer. we also need to resume other pages
    {
        vector<DupCmdQMgr *> vec;
        vec.push_back(this->m_pMainCmdQMgr);
        vec.push_back(this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]);

        this->m_TwinResume.Resueme_init(&vec,&this->m_TwinCtrl);
    }
    return ret;
}



MBOOL TwinMgr_IMP::uninitTwin()
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mTwinLock);

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;


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

    this->m_DualInputParam.dualInCfg = NULL;
    this->m_DualInputParam.dualOutCfg = NULL;
    this->m_DualInputParam.dualIspReg = NULL;


    this->m_isTwin_IMGO.uninit();
    this->m_isTwin_RRZO.uninit();
    this->m_isTwin_AFO.uninit();


    /// Init value

    this->m_hwModule = CAM_A;
    this->m_twinHwModule = CAM_MAX;
    this->m_occupiedModule.clear();


    // twin
    this->m_hwModuleNum = eTwinMgr_none;


    this->m_pMainCmdQMgr = NULL;
    for(MUINT32 i=0;i<ISP_DRV_CAM_BASIC_CQ_NUM;i++)
        this->m_pTwinCmdQMgr[i] = NULL;
    this->m_pMainIspDrv = NULL;
    this->m_pTwinIspDrv = NULL;

    this->m_switchLatency = 0;

    this->FSM_UPDATE(op_uninit);
EXIT:
    return ret;

}

MBOOL TwinMgr_IMP::updateTwin()
{
    MBOOL ret = MTRUE;
    DupCmdQMgr* pCmdQ = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0];


    if(this->FSM_CHECK(op_runtwin) == MFALSE){
        LOG_ERR("updateTwin: fail\n");
        return MFALSE;
    }

    //note:!!!!
    //why need to check resume here? plz reference the comment at resumeTwin()
    this->m_TwinResume.Resume_check();

    if(pCmdQ->CmdQMgr_update()){
        LOG_ERR("[%d]:Twin CQ undate fail\n", this->m_twinHwModule);
        ret = MFALSE;
        goto EXIT;
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
    MUINT32 _size = 0;
    MBOOL isTwin;

    //if fsm is false->twin is not enabled, other
    if(this->FSM_CHECK(op_twininfo) == MFALSE){
        LOG_ERR("FSM is op_unknown. Cannot get isTwin status currently!! isTwin is valid after do configPath().");
        return MFALSE;
    }

    this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_getsize,(MUINTPTR)&_size,0);

    if(_size == 0){
        LOG_ERR("que size is 0\n");
        return MFALSE;
    }

    switch(dmaChannel){
        case _afo_:
            if(bDeque == MTRUE){
                if(_size <= 1){
#if D_TWIN_AF
                    isTwin = this->m_lateset_AFO;
#else
                    // 1 is from configpipe, 2 is from enque. so need > 1.
                    LOG_ERR("logic error, size must be >1. 1 is from 1st configpath() in configpipe(), >1 is from enque");
                    return MFALSE;
#endif
                }
                else{
                    //always return the oldest info ,but not front(). front() is from configpipe()
                    this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_at,1,(MUINTPTR)&isTwin);
                }
            }
            else{
                //always return latest info. if size = 1, maybe timing is at deque to next enque, or before start.
                if(_size == 1){
                    RACING_PROT update(&isTwin,this->m_lateset_AFO);
                }
                else{
                    //
                    this->m_isTwin_AFO.sendCmd(QueueMgr<MBOOL>::eCmd_back,(MUINTPTR)&isTwin,0);
                }
            }
            break;
        default:
            LOG_ERR("unsupported dma:%d\n",dmaChannel);
            return MFALSE;
            break;
    }

    LOG_DBG("dma:0x%x que size:%d,isTwin:%d,cur fsm:%d\n",dmaChannel,_size,isTwin,this->m_FSM);
    return isTwin;
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
    MUINT32 i;


    if(MFALSE == this->FSM_CHECK(op_runtwin)) {
        LOG_ERR("Twin is not enabled!!! Please check why upper user call TwinMgr!");
        return MFALSE;
    }


    // Get m_pMainIspDrv & m_pTwinIspDrv from DupCmdQMgr
    if(this->m_FSM == op_startTwin) {
        targetIdxMain = ((this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx() + 1) % this->m_pMainCmdQMgr->CmdQMgr_GetDuqQ());
        targetIdxTwin = ((this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx() + 1) % this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQ());
    }
    // when m_FSM != op_startTwin, DupCmdQMgr is not start yet
    else {
        targetIdxMain = this->m_pMainCmdQMgr->CmdQMgr_GetDuqQIdx();
        targetIdxTwin = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetDuqQIdx();
    }

    burstQ = this->m_pMainCmdQMgr->CmdQMgr_GetBurstQ();

    LOG_DBG("+Update Twin!! targetIdxMain(%d), targetIdxTwin(%d), burstQ(%d)", targetIdxMain, targetIdxTwin, burstQ);

    for(i = 0; i < burstQ; i++)
    {
        this->m_pMainIspDrv = this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
        this->m_pTwinIspDrv = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0]->CmdQMgr_GetCurCycleObj(targetIdxTwin)[i];

        this->m_TwinCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainIspDrv;
        this->m_TwinCtrl.m_pTwinIspDrv = this->m_pTwinIspDrv;

        // Twin will modify the following values, need to keep its value before run twin for debugging
        // These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
        // Also, write register that are existed in physical but not in CQ. EX: TG cropping window
        if(MFALSE == this->prepareDualInCfg())
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
        if(this->m_TwinCtrl.update_afterRunTwin())
            ret = MFALSE;

    }

    //enque into HW
    this->m_TBC.enque_pop();

    LOG_DBG("-Update Twin!!");

EXIT:
    return ret;

}


MBOOL TwinMgr_IMP::cvtDupCmdQRegtoDualIspReg()
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
        this->m_DualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pTwinIspDrv->getIspVirRegVirAddr();
        this->m_DualInputParam.dualIspReg->ptr_isp_uni = (cam_uni_reg_t*)(this->m_pMainIspDrv->getIspVirRegVirAddr() + (CAM_BASE_RANGE_SPECIAL >> 2));
    }
    else if(CAM_B == this->m_hwModule) {
        #if 1 // ba twin patch
        this->m_DualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pTwinIspDrv->getIspVirRegVirAddr();
        #else
        this->m_DualInputParam.dualIspReg->ptr_isp_reg = (cam_a_reg_t*)this->m_pTwinIspDrv->getIspVirRegVirAddr();
        this->m_DualInputParam.dualIspReg->ptr_isp_reg_d = (cam_b_reg_t*)this->m_pMainIspDrv->getIspVirRegVirAddr();
        #endif
        this->m_DualInputParam.dualIspReg->ptr_isp_uni = (cam_uni_reg_t*)(this->m_pMainIspDrv->getIspVirRegVirAddr() + (CAM_BASE_RANGE_SPECIAL >> 2));
    }

EXIT:
    return ret;
}



// Twin will modify the following values, need to keep its value before run twin for debugging
// These value are input to twin drv: DUAL_IN_CONFIG_STRUCT
// Also, write register that are existed in physical but not in CQ. EX: TG cropping window
MBOOL TwinMgr_IMP::prepareDualInCfg()
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
    this->m_DualInputParam.dualInCfg->DEBUG.DUAL_LOG_EN = 0;
    this->m_DualInputParam.dualInCfg->DEBUG.DUAL_LOG_ID = 1;
    this->m_DualInputParam.dualInCfg->SW.DUAL_SEL = 3;

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

    if(this->m_TwinCtrl.m_subsample)
        cq = ISP_DRV_CQ_THRE11;
    else
        cq = ISP_DRV_CQ_THRE1;

    if(this->m_pTwinCmdQMgr[cq]->CmdQMgr_Cfg(arg1,arg2) != 0)
        return 1;
    else
        return 0;
}

MUINT32 TwinMgr_IMP::CQ_Trig(void)
{
    E_ISP_CAM_CQ cq;

    if(this->m_TwinCtrl.m_subsample)
        cq = ISP_DRV_CQ_THRE11;
    else
        cq = ISP_DRV_CQ_THRE1;

    if(this->m_pTwinCmdQMgr[cq]->CmdQMgr_start() != 0)
        return 1;
    else
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
        LOG_ERR("header size error\n");;
        return 0;
    }

    switch(dmaChannel){
        case _imgo_:
        case _rrzo_:
            return rst.HeaderSize;
            break;
        case _afo_:
            return AFO_STRIDE;
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
    TwinMgr_BufCtrl* pFbc;
    if(this->getIsTwin() == MFALSE){
        LOG_ERR("twin mode is not enabled\n");
        ret = 1;
        goto EXIT;
    }

    if(this->m_TBC.enque_push(dmaChannel,bufInfo) == MFALSE){
        ret = 1;
        goto EXIT;
    }

EXIT:
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


DupCmdQMgr* TwinMgr_IMP::getTwinCmdQ(ISP_HW_MODULE module,E_ISP_CAM_CQ cq)
{
    if(this->FSM_CHECK(op_runtwin) == MFALSE){
        return NULL;
    }

    module;

    return this->m_pTwinCmdQMgr[cq];
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
    MUINT32 i;
    DupCmdQMgr* pCmdQ = this->m_pTwinCmdQMgr[ISP_DRV_CQ_THRE0];
    MBOOL ret = MTRUE;
    LOG_DBG("enter suspending mode:\n");

    i = 0;
    for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin();
        i<this->m_occupiedModule.size();
        it++, i++)
    {
        ret = CAM_BUF_CTRL::suspend(*it, pCmdQ->CmdQMgr_GetCurCycleObj(pCmdQ->CmdQMgr_GetDuqQIdx())[0],
                                    NULL, CAM_BUF_CTRL::eSus_HW_SW_STATE);
    }

    return ret;
}


MBOOL TwinMgr_IMP::resume(void)
{
    MUINT32 i;
    MBOOL ret = MTRUE;
    LOG_DBG("exit suspending mode:\n");

    i=0;
    for(list<ISP_HW_MODULE>::iterator it = this->m_occupiedModule.begin(); \
        i<this->m_occupiedModule.size();\
        it++,i++){
        ret = CAM_BUF_CTRL::resume(*it, NULL);
        if (ret == MFALSE) {
            LOG_ERR("suspending fail at : %d", i);
            break;
        }
    }

    return ret;
}

MBOOL TwinMgr_IMP::recoverTwin(E_CAMPATH_STEP step)
{
    UniMgr UniMgr;

    LOG_DBG("enter HW_recover:%d\n",step);
    switch(step){
        case eCmd_path_stop:
            if(UniMgr.UniMgr_GetCurLink() == this->m_hwModule){
                UniMgr.UniMgr_recover(eCmd_Uni_stop,this->m_hwModule);
            }

            //sequence : master cam 1st, then slave cam.
            //master cam
            this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pMainCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
            this->m_TopCtrl.HW_recover(0);
            //slave cam
            return this->m_TwinCtrl.HW_recover(0);
            break;
        case eCmd_path_restart:
            if(UniMgr.UniMgr_GetCurLink() == this->m_hwModule){
                UniMgr.UniMgr_recover(eCmd_Uni_restart,this->m_hwModule);
            }

            //sequence : slave cam 1st, then master cam.
            //slave cam
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
        //
        ret = CAM_TWIN_PIPE::GetTwinRegAddr(CAM_A,&addr);
        pAddr->push_back(addr);
        break;
    case CAM_B:
        //currently only CAM_A is master cam
        pMaster->push_back(CAM_B);
        //
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
        case TWIN_CMD_GET_CUR_SLAVE_CQMGR:
            if( (arg2==0) || (arg3==0) ){
                ret = MFALSE;
                goto EXIT;
            }
            if((ret = this->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,arg2,0,0)) == MFALSE){
                goto EXIT;
            }
            else{
                list<ISP_HW_MODULE>* l_module = (list<ISP_HW_MODULE>*)arg2;
                list<ISP_HW_MODULE>::iterator it = l_module->begin();
                ((vector<DupCmdQMgr*>*)arg3)->clear();
                for(MUINT32 i=0;i<= l_module->size();i++,it++){
                    ((vector<DupCmdQMgr*>*)arg3)->push_back(this->getTwinCmdQ(*it,(E_ISP_CAM_CQ)arg1));
                }
            }
            break;
        case TWIN_CMD_GET_CUR_TWINMODULE:
            if(arg1==0){
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


MBOOL Twin_Resume_check::Resueme_init(vector<DupCmdQMgr*>* pvector,CAM_TWIN_PIPE* pTwinpipe)
{
    MBOOL ret = MTRUE;

    if( (pvector == NULL) || (pTwinpipe == NULL)){
        ret= MFALSE;
        goto EXIT;
    }

    for(MUINT32 i=0;i< pvector->size();i++)
        this->m_pCmdQ.push_back(pvector->at(i));

    this->m_pTwinpipe = pTwinpipe;
    this->m_loop = this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQ();

EXIT:
    return ret;
}

MBOOL Twin_Resume_check::Resume_check(void)
{
    MBOOL ret = MTRUE;
    MUINT32 targetIdxMain = 0;
    MUINT32 targetIdxTwin = 0;

    if( (this->m_pCmdQ.size() < 1) || (this->m_pTwinpipe == NULL)){
        goto EXIT;
    }

    if(this->m_loop){//this loop is the same size of cmdQ ring-buffer size . for CPU saving after all pages r resumed.
        //note:!!!!
        //why need to check resume here? plz reference the comment at resumeTwin()
        targetIdxMain = ((this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQIdx() + 1) % this->m_pCmdQ.at(0)->CmdQMgr_GetDuqQ());
        targetIdxTwin = ((this->m_pCmdQ.at(1)->CmdQMgr_GetDuqQIdx() + 1) % this->m_pCmdQ.at(1)->CmdQMgr_GetDuqQ());


        //check cq page's setting
        for(MUINT32 i = 0; i < this->m_pCmdQ.at(0)->CmdQMgr_GetBurstQ(); i++){
            this->m_pTwinpipe->m_pIspDrv = (IspDrvVir*)this->m_pCmdQ.at(0)->CmdQMgr_GetCurCycleObj(targetIdxMain)[i];
            this->m_pTwinpipe->m_pTwinIspDrv = this->m_pCmdQ.at(1)->CmdQMgr_GetCurCycleObj(targetIdxTwin)[i];

            this->m_pTwinpipe->resume_check();
        }

        this->m_loop--;
    }


EXIT:
    return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
TwinMgr* TwinMgr::createInstance(ISP_HW_MODULE hwModule)
{
    return (TwinMgr*)TwinMgr_IMP::createInstance(hwModule);
}

char* TwinMgr::ModuleName(void)
{
    static const char TwinName[] = {"TwinMgr_CAMB"};

    return (char*)TwinName;
}


