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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "ResMgr"

//
#include <mtkcam/utils/std/Log.h>

//
#include <cutils/properties.h>  // For property_get().
#include "Cam_ResMgr.h"
#include "cam_capibility.h"

////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(resmgr);

#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_ERR
#undef PIPE_WRN

#define PIPE_DBG(fmt, arg...)        do {\
    if (resmgr_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)

#define PIPE_INF(fmt, arg...)        do {\
    if (resmgr_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)

#define PIPE_WRN(fmt, arg...)        do {\
        if (resmgr_DbgLogEnable_ERROR  ) { \
            BASE_LOG_WRN("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
        }\
    } while(0)


#define PIPE_ERR(fmt, arg...)        do {\
    if (resmgr_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x][%s]:" fmt,this->m_hwModule,this->m_User, ##arg); \
    }\
} while(0)


MBOOL Central::get_availNum(ISP_HW_MODULE master, vector<ISP_HW_MODULE>& v_available, char Name[32])
{
    Mutex::Autolock lock(this->mLock);
    MBOOL   ret = MTRUE;
    MUINT32 availsize = this->m_available;

    if (*Name == '\0') {
        BASE_LOG_ERR("cant have NULL userName\n");
        return MFALSE;
    }

    v_available.clear();
    if (master == CAM_C) {
 //raw_c is not twinable
        return ret;
    }
    // this->m_module[i].User : NormalPipe
    // Nmae : IResMgr_0
    for (MUINT32 i = 0; i < CAM_C; i++) {
        if (this->m_module[i].occupied == MFALSE) {
            v_available.push_back((ISP_HW_MODULE)i);
        }
        else if (std::strcmp((const char*)this->m_module[i].User, (const char*)Name) == 0) {
            //this module is used by CamResMgr itself, so still count into available cam
            v_available.push_back((ISP_HW_MODULE)i);
            availsize++;
        }
    }
    if (this->m_module[CAM_C].occupied == MFALSE) {
        availsize--;
    }

    if (v_available.size() != availsize) {
        string _s;

        BASE_LOG_ERR("size mismatch : %d_%d\n",(MUINT32)v_available.size(),availsize);

        for (MUINT32 i=0;i<PHY_CAM;i++) {
            char *cam_name;

            switch (i) {
            case CAM_A: cam_name = "cam:a_"; break;
            case CAM_B: cam_name = "cam:b_"; break;
            case CAM_C: cam_name = "cam:c_"; break;
            default: cam_name = "cam:unknown_"; break;
            }
            _s = _s + cam_name + (this->m_module[i].occupied ? "occupied, " : "available, ");
        }

        BASE_LOG_ERR("resource pool: %s\n", _s.c_str());

        v_available.clear();

        ret = MFALSE;
    }

    return ret;
}

MBOOL Central::Register(ISP_HW_MODULE module,char Name[32],MUINT32 logLevel)
{
    Mutex::Autolock lock(this->mLock);
    MBOOL ret = MTRUE;

    if (module >= PHY_CAM) {
        BASE_LOG_ERR("unsupported cam:%d\n",module);
        ret = MFALSE;
        goto EXIT;
    }
    else if (*Name == '\0') {
        BASE_LOG_ERR("cant have NULL userName\n");
        ret = MFALSE;
        goto EXIT;
    }

    if (this->m_module[module].occupied == MFALSE) {
        this->m_module[module].occupied = MTRUE;
        memset((void*)this->m_module[module].User, 0, sizeof(this->m_module[module].User));
        std::strncpy((char*)this->m_module[module].User,(char const*)Name,sizeof(this->m_module[module].User)-1);
        this->m_available--;
    }
    else if (std::strncmp((const char*)Name,(const char*)Cam_ResMgr::m_staticName,strlen((char*)Cam_ResMgr::m_staticName)) == 0) {//from ResMgr
        if (std::strcmp((const char*)this->m_module[module].User,(const char*)Name) == 0) {//the same ResMgr
            //do nothing, this module is used by CamResMgr itself,can register agian & agian due to per-frame request
        }
    }
    else{
        if(logLevel){
            BASE_LOG_ERR("cam:%d already occupied by user:%s,can't register by user:%s\n",module,this->m_module[module].User,Name);
        }else{
            BASE_LOG_WRN("cam:%d already occupied by user:%s,can't register by user:%s,auto-try another\n",module,this->m_module[module].User,Name);
        }
        ret = MFALSE;
        goto EXIT;
    }

EXIT:
    return ret;
}


MBOOL Central::Release(ISP_HW_MODULE module)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);

    if(this->m_module[module].occupied == MTRUE){
        this->m_module[module].occupied = MFALSE;
        *this->m_module[module].User = '\0';
        this->m_available++;

    }
    else{
        ret = MFALSE;
        BASE_LOG_ERR("cam:%d logic error: ,should be occupied\n",module);
    }

    return ret;
}

ISP_HW_MODULE Central::Register_search(MUINT32 width,char Name[32])
{
    if(this->Register(CAM_A,Name,0) == MTRUE){
        return CAM_A;
    }
    else if(this->Register(CAM_B,Name,0) == MTRUE) { //if CAM_A is occupied using CAM_B
        return CAM_B;
    }
    else if(this->Register(CAM_C,Name) == MTRUE) {
        return CAM_C;
    }
    else{
        BASE_LOG_ERR("no available cam, isp lunch fail\n");
        return CAM_MAX;
    }

    BASE_LOG_ERR("isp lunch fail due to no correct pipeline size\n");

    return CAM_MAX;
}

/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
Central Cam_ResMgr::m_central;
Mutex   Cam_ResMgr::mLock;
char    Cam_ResMgr::m_staticName[16] = "IResMgr";
Cam_ResMgr::Cam_ResMgr(ISP_HW_MODULE master,char Name[32])
{
    DBG_LOG_CONFIG(imageio, resmgr);

    m_hwModule = master;
    *m_User = '\0';
    memset((void*)m_User, 0, sizeof(m_User));
    std::strncpy((char*)m_User, (char const*)Name, sizeof(m_User)-1);

    m_vSlave.clear();
    m_slave_module_release.clear();
    m_switchLatency = Cam_ResMgr::REG_Module;
    sprintf(m_internalUser,"%s_%d", Cam_ResMgr::m_staticName, master);
}

Cam_ResMgr* Cam_ResMgr::Res_Attach(ISP_HW_MODULE master,char Name[32])
{
    Mutex::Autolock lock(Cam_ResMgr::mLock);
    MBOOL ret = MTRUE;

    BASE_LOG_INF("CAM Res_Attach:: module:%d,name:%s\n",master,Name);

    if(Cam_ResMgr::m_central.Register(master,Name) == MFALSE){
        return NULL;
    }

    return new Cam_ResMgr(master,Name);
}

Cam_ResMgr* Cam_ResMgr::Res_Attach(MUINT32 width,char Name[32])
{
    Mutex::Autolock lock(Cam_ResMgr::mLock);
    MBOOL ret = MTRUE;
    ISP_HW_MODULE master;

    BASE_LOG_INF("CAM Res_Attach:: size:%d,name:%s\n",width,Name);

    if( (master = Cam_ResMgr::m_central.Register_search(width,Name))  == CAM_MAX){
        return NULL;
    }

    return new Cam_ResMgr(master,Name);
}


MBOOL Cam_ResMgr::Res_Detach(void)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(mLock);
    PIPE_INF("Res_Detach:\n");

    for(MUINT32 i=0;i<this->m_vSlave.size();i++){
        this->mLock.unlock();
        if(this->m_vSlave.at(i)->Res_Detach() == MFALSE)
            ret = MFALSE;
        this->mLock.lock();
    }

    if(this->m_central.Release(this->m_hwModule) == MFALSE){
        ret = MFALSE;
    }

    this->mLock.unlock();
    if(ret == MTRUE)
        delete this;
    else
        PIPE_ERR("release fail, obj won't be destroied \n");
EXIT:
    return ret;
}

MBOOL Cam_ResMgr::Res_Meter(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam)
{
    Mutex::Autolock lock(mLock);
    MBOOL       ret = MTRUE;
    Cam_ResMgr  *ptr = NULL;
    char        str[512] = {'\0'}, _tmp[256] = {'\0'};
    MINT32      curLeng;
    MUINT32     cur_clkL = cfgOutParam.clk_level;
    vector<ISP_HW_MODULE> availCam;

    //reset cfgOutParam
    cfgOutParam.clk_level = 0;
    cfgOutParam.slave_module.clear();
    cfgOutParam.bin_en = MFALSE;
    cfgOutParam.isTwin = MFALSE;

    if (this->Res_Meter_Release() == MFALSE) {
        PIPE_ERR("meter_release fail\n");
        ret = MFALSE;
        goto EXIT;
    }

    this->m_central.get_availNum(this->m_hwModule, availCam, this->m_internalUser);

    curLeng = snprintf(str, 511,
    "cfgIn:tg_off(%d),tg_pix(%d),tg_crop(%dx%d),fps(%d),rrzo_off(%d),rrz_out(%d),curclk:%d,off_bin:%d,off_twin:%d,avail_cam:%zu,MIPI_pixrate:%dkhz,fmt:%d,senNum:%d\n",
            cfgInParam.bypass_tg, cfgInParam.pix_mode_tg, cfgInParam.img_crop_w, cfgInParam.img_crop_h,cfgInParam.raw_fps,
            cfgInParam.bypass_rrzo, cfgInParam.rrz_out_w,cfgInParam.vClk.at(cur_clkL),cfgInParam.offBin,
            cfgInParam.offTwin,availCam.size(),cfgInParam.MIPI_pixrate,cfgInParam.SrcImgFmt,cfgInParam.SenNum);

    /**
       note:
         also need to make sure cfgOutParam.clk_level is started from 0. always use lowest clk, and notify clk request of isp to MMDVFS each frame.
     */
    switch (cfgInParam.SrcImgFmt) {
        case TG_FMT_RAW8:
        case TG_FMT_RAW10:
        case TG_FMT_RAW12:
        case TG_FMT_RAW14:
            if( (ret = this->Res_Meter_Bayer(cfgInParam,cfgOutParam,cur_clkL)) == MFALSE){
                ret = MFALSE;
                goto EXIT;
            }
            break;
        case TG_FMT_YUV422:
            if( (ret = this->Res_Meter_YUV(cfgInParam,cfgOutParam)) == MFALSE){
                ret = MFALSE;
                goto EXIT;
            }
            break;
        default:
            PIPE_ERR("unsupported fmt:%d\n",cfgInParam.SrcImgFmt);
            ret = MFALSE;
            goto EXIT;
            break;
    }
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};

        property_get("vendor.camsys.dfslv", value, "0");
        MINT32 clklv = atoi(value);
        if(clklv != 0){
            PIPE_INF("force to lv:%d\n",clklv);
            cfgOutParam.clk_level = (--clklv);
        }

    }
    if(cfgOutParam.slave_module.size()<1){
        snprintf(_tmp, 255,\
            "[%d]cfgOutParam: PixModeSEPI(%d), PixModeSEPO(%d), PixModeMRG_R8(%d), bin_en(%d), isTwin(%d),clk rate(%d)", \
                this->m_hwModule,cfgOutParam.dlp.SEPI, cfgOutParam.dlp.SEPO,  cfgOutParam.dlp.MRG_R8, \
                cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level));
    }
    else if(cfgOutParam.slave_module.size() == 1){
        snprintf(_tmp, 255,\
            "[%d]cfgOutParam: PixModeSEPI(%d), PixModeSEPO(%d), PixModeMRG_R8(%d), bin_en(%d), isTwin(%d),clk rate(%d),slave(%d)", \
                this->m_hwModule,cfgOutParam.dlp.SEPI, cfgOutParam.dlp.SEPO,  cfgOutParam.dlp.MRG_R8, \
                cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level),cfgOutParam.slave_module.at(0));
    }
    else{
        snprintf(_tmp, 255,\
            "[%d]cfgOutParam: PixModeSEPI(%d), PixModeSEPO(%d), PixModeMRG_R8(%d), bin_en(%d), isTwin(%d),clk rate(%d),slave1(%d),slave2(%d)", \
                this->m_hwModule,cfgOutParam.dlp.SEPI, cfgOutParam.dlp.SEPO,  cfgOutParam.dlp.MRG_R8, \
                cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level),cfgOutParam.slave_module.at(0),cfgOutParam.slave_module.at(1));
    }

    //register slave cam
    if(this->m_vSlave.size() == cfgOutParam.slave_module.size()){//check only
        if(resmgr_DbgLogEnable_DEBUG){
            for(MUINT32 i=0;i<cfgOutParam.slave_module.size();i++){
                if(cfgOutParam.slave_module.at(i) != this->m_vSlave.at(i)->Res_GetCurrentModule()){
                    PIPE_ERR("logic error,these 2 vector should be the same when size r equal(%d_%d_%d)\n",\
                        i,cfgOutParam.slave_module.at(i),this->m_vSlave.at(i)->Res_GetCurrentModule());
                }
            }
        }
    }
    else if(this->m_vSlave.size() < cfgOutParam.slave_module.size()){//e.g. single->twin, twin2->twin3
        if(resmgr_DbgLogEnable_DEBUG){
            for(MUINT32 i=0;i<this->m_vSlave.size();i++){//check only
                if(cfgOutParam.slave_module.at(i) != this->m_vSlave.at(i)->Res_GetCurrentModule()){
                    PIPE_ERR("logic error,these 2 vector should be the same when size r equal(%d_%d_%d)\n",\
                        i,cfgOutParam.slave_module.at(i),this->m_vSlave.at(i)->Res_GetCurrentModule());
                }
            }
        }

        for(MUINT32 i=this->m_vSlave.size();i<cfgOutParam.slave_module.size();i++){
            this->mLock.unlock();
            ptr = Cam_ResMgr::Res_Attach(cfgOutParam.slave_module.at(i),this->m_internalUser);
            this->mLock.lock();
            if(ptr != NULL){
                this->m_vSlave.push_back(ptr);
                ptr = NULL;
            }
            else{

            }
        }
    }
    else{// e.g.: twin -> single, twin3->twin2
        //will be released at next tiime's Cam_ResMgr::Res_Meter()
        this->m_switchLatency = REL_Module;
        this->m_slave_module_release.clear();
        for(MUINT32 i=0;i<cfgOutParam.slave_module.size();i++){
            this->m_slave_module_release.push_back(cfgOutParam.slave_module.at(i));
        }
    }

EXIT:
    strncat(str,_tmp, (511-curLeng));
    PIPE_INF("%s\n", str);

    this->m_SenInfo.assign(cfgInParam.raw_fps,cfgInParam.MIPI_pixrate,
        cfgInParam.SrcImgFmt,cfgInParam.pattern,cfgOutParam.clk_level,(vector<MUINT32>*)&cfgInParam.vClk);

    this->mLock.unlock();
    return ret;
}

MBOOL Cam_ResMgr::Res_Meter_Release(void)
{
    MBOOL ret = MTRUE;

    // e.g.: twin -> single, twin3->twin2
    if (this->m_switchLatency == Cam_ResMgr::REL_Module) {
        PIPE_INF("release slave cam(%zu_%zu)\n",this->m_slave_module_release.size(),this->m_vSlave.size());
        if(resmgr_DbgLogEnable_DEBUG){//check only
            for(MUINT32 i=0;i<this->m_slave_module_release.size();i++){
                if(this->m_slave_module_release.at(i) != this->m_vSlave.at(i)->Res_GetCurrentModule()){
                    PIPE_ERR("logic error,these 2 vector should be the same when size r equal(%d_%d_%d)\n",\
                        i,this->m_slave_module_release.at(i),this->m_vSlave.at(i)->Res_GetCurrentModule());
                }
            }
        }

        for(MUINT32 i=this->m_slave_module_release.size();i<this->m_vSlave.size();i++){
            this->mLock.unlock();
            if( this->m_vSlave.at(i)->Res_Detach() == MFALSE)
                ret = MFALSE;
            else
                this->m_vSlave.erase(this->m_vSlave.begin()+i);
            this->mLock.lock();
        }

        //
        this->m_switchLatency = Cam_ResMgr::REG_Module;
    }
    else{
        //do nothing
    }

    this->mLock.unlock();
    return ret;
}

MVOID Cam_ResMgr::K_CLK_CASE(Res_Meter_OUT &cfgOutParam)
{
    cfgOutParam.clk_level += 1;
}

MVOID Cam_ResMgr::K_BIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,MBOOL& bBin_useful)
{
    bBin_useful = MFALSE;
    cfgOutParam.bin_en = MTRUE;
    thru_put = thru_put >> BIN_THRU_PUT;
}

MVOID Cam_ResMgr::K_TWIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,vector<ISP_HW_MODULE>& availCam)
{
    if (availCam.size() > 0) {
        MUINT32 i = 0;

        cfgOutParam.slave_module.push_back(availCam.at(i));

        availCam.erase(availCam.begin() + i);

        cfgOutParam.isTwin = MTRUE; //twin must be enabled

        if (cfgOutParam.slave_module.size() > 1) {
            thru_put = thru_put * cfgOutParam.slave_module.size();
        }
        thru_put = thru_put / (cfgOutParam.slave_module.size() + 1);
    }
}

MBOOL Cam_ResMgr::Res_Meter_kernel(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,
                    MBOOL& bBin_useful, MBOOL &bBin_first, vector<ISP_HW_MODULE>& availCam)
{
    // check thru put, always counting from clk_level = 0 for isp minimum request
    // priority : 1:twin.  2:bin.  3:Clk.
    while (thru_put >= (MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * CLK_MARGIN_N / CLK_MARGIN_M)) {

        if ((bBin_useful == MTRUE) && (bBin_first == MTRUE)) {
            this->K_BIN_CASE(cfgOutParam, thru_put, bBin_useful);
        }
        else if ((cfgInParam.offTwin == MFALSE) && (availCam.size() != 0) && (cfgOutParam.bin_en == MFALSE)) {
            this->K_TWIN_CASE(cfgOutParam, thru_put, availCam);
        }
        else if ((bBin_useful == MTRUE) && (bBin_first == MFALSE) && (cfgOutParam.isTwin == MFALSE)) {
            this->K_BIN_CASE(cfgOutParam, thru_put, bBin_useful);
        }
        else if (cfgOutParam.clk_level < (cfgInParam.vClk.size()-1)) {
            this->K_CLK_CASE(cfgOutParam);
        }
        else {
            PIPE_ERR("over spec! pattern:%d thru put:(%dx%dx%d,total:%d), max clk:%d(margin:%d), bin_en(%d_%d), twin_en(%d)\n",
                cfgInParam.pattern,
                cfgInParam.img_crop_w, cfgInParam.img_crop_h, cfgInParam.raw_fps,thru_put,
                cfgInParam.vClk.at(cfgOutParam.clk_level), (MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * CLK_MARGIN_N / CLK_MARGIN_M),
                cfgInParam.offBin, cfgOutParam.bin_en,
                cfgOutParam.isTwin);
            return MFALSE;
        }
    }

    return MTRUE;
}



#define _GET_PIPELINE_SIZE(_cap_obj, _qrst, _bin, _fbnd)     ({ \
        tCapInPut       pipeCapIn;\
        tPipeLineSize   pipeSizeBin;\
        pipeSizeBin.bBin = _bin;\
        pipeSizeBin.bFBN = _fbnd;\
        pipeCapIn.e_Op = NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN;\
        pipeCapIn.inerOP = E_CAM_pipeline_size;\
        pipeCapIn.pInput = (MVOID*)&pipeSizeBin;\
        if (_cap_obj.GetCapibility(pipeCapIn, qrst) == MFALSE) {\
            PIPE_ERR("query pipesize fail");\
        }\
        _qrst.pipeSize;\
    })

MBOOL Cam_ResMgr::Res_Meter_Bayer(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam, MUINT32 cur_clkL)
{
/**
 * 1. bin useful by input param (rrz_out_w, off_bin)
 * 2. bin/fbnd/dbn combination
 * 3. pipe size limitation under bin/fbnd/db combination
 * 4. meter calculation
 * 5. pipe pix mode decision
 */
/**
 * a) single; b) twin; c) 48M; d) dcif
 * 48M@30: 8080x6064
 * 32M@30: 6632x4976 (7656x4312)
 * 16M@30: 5504x3096
 *  8M@30: 3625x3032
 * output: bin, twin, clk_lv
 */
    static const E_CamPixelMode _PixMode_Down[ePixMode_MAX] = {
        [ePixMode_NONE] = ePixMode_NONE,
        [ePixMode_1]    = ePixMode_1,
        [ePixMode_2]    = ePixMode_1,
        [ePixMode_4]    = ePixMode_2 };

    MBOOL           ret = MTRUE, bBin_useful = MTRUE, bDbn_on = MFALSE, bFbnd_on = MFALSE, bBin_first = MFALSE;
    MUINT32         thru_put = 0;
    MUINT32         pipesize = 0, pipesize_bin = 0, pipesize_twin = 0;
    E_CamPixelMode  cur_pix_mode;
    tCAM_rst        qrst;
    capibility      camcap(this->m_hwModule);
    vector<ISP_HW_MODULE> availCam;

    // 0. total data size
    if ((cfgInParam.bypass_tg) || (cfgInParam.MIPI_pixrate == DEFAULT_MIPI_PIXRATE)) {
        thru_put = (MUINT32) (((MUINT64)cfgInParam.img_crop_w)*cfgInParam.img_crop_h*cfgInParam.raw_fps / 10);
    }
    else {
        thru_put = cfgInParam.MIPI_pixrate;
    }

    if (thru_put == 0) {
        PIPE_ERR("thru put error,fps(x10):%d,size:%d_%d mipi:%d\n",cfgInParam.raw_fps,cfgInParam.img_crop_w,cfgInParam.img_crop_h,cfgInParam.MIPI_pixrate);
        ret = MFALSE;
        goto EXIT;
    }

    this->m_central.get_availNum(this->m_hwModule, availCam, this->m_internalUser);

    pipesize = _GET_PIPELINE_SIZE(camcap, qrst, MFALSE, MFALSE);
    pipesize_bin = _GET_PIPELINE_SIZE(camcap, qrst, MTRUE, MFALSE);
    //twin and fbnd throughput both div2, which contraint the max width
    pipesize_twin = _GET_PIPELINE_SIZE(camcap, qrst, MFALSE, MTRUE); /* 6632 */

    // rrz can't scale-up
    if (bBin_useful == MTRUE) {
        bBin_useful = (cfgInParam.offBin == MTRUE) ? MFALSE :
            ((cfgInParam.rrz_out_w <= (cfgInParam.img_crop_w >> 1)) ? (MTRUE):(MFALSE));
    }

    //check dbn/bin/linebuffer case
    switch (cfgInParam.pattern) {
    case eCAM_DUAL_PIX:
        bBin_useful = MFALSE; //bin/dbn is mutually exclusive
        bDbn_on = MTRUE;
        //thru_put    = thru_put << 1; //*2 is for TG_width under dualPD, because of TG from sesnor is not the real size of TG_grabs
        //thru_put    = thru_put >> 1;
        //for DBN, DMXO should be configured as two pixel-out in Dual_PD case.

        if (cfgInParam.pix_mode_tg == ePixMode_1) {
            PIPE_ERR("DMXI is one pixel-in but DMXO is two pixel-out. This path is not verified!!");
        }
        //dualpd img_crop_w/1 is because of TG will be the size which is after DBN.
        if ((cfgInParam.img_crop_w) > pipesize) {
            PIPE_ERR("under pattern:%d, size after DBN still too large(%d_%d), need twin\n",
                cfgInParam.pattern, cfgInParam.img_crop_w, pipesize);
            //twin + dbn is not verified!
        }
        //else {
            ////twin + dbn is not verified
            //availCam.clear();
        //}

        break;
    case eCAM_4CELL: // quad-code
        bBin_useful = MFALSE;  //bin/fbnd is mutually exclusive
        bFbnd_on    = MTRUE;
        thru_put    = thru_put >> 1; //fbnd only shrink 1/2 throughput
        pipesize    = _GET_PIPELINE_SIZE(camcap, qrst, MFALSE, MTRUE);

        if (cfgInParam.pix_mode_tg == ePixMode_1) {
            PIPE_ERR("SEPI=1pix SEPO=2pix. This path is not verified!!\n");
        }

        if (cfgInParam.img_crop_w > pipesize/*6632*/) {
            PIPE_WRN("SensorPattern:%d size too large(%d_%d) need twin\n",
                cfgInParam.pattern, cfgInParam.img_crop_w, pipesize);
        }
        break;
    case eCAM_MONO:
        bBin_useful = MFALSE;
        bFbnd_on    = MTRUE;
        break;
    case eCAM_ZVHDR:
        bBin_useful = MFALSE;
        break;
    #if 0
    case eCAM_4CELL_HDR_LSSL: // 4cell XHDR
        bFbnd_on = MTRUE;
        bBin_useful = MFALSE;
        //pipesize align _GET_PIPELINE_SIZE(camcap, qrst, MFALSE, MFALSE)
        break;
    #endif
    case eCAM_4CELL_IVHDR:
    case eCAM_4CELL_ZVHDR:
        PIPE_ERR("not supported sensor pattern : %d\n", cfgInParam.pattern);
        break;
    default:
        break;
    }
    /**
     * bBin_useful/bFbnd_on decision done
     */

    if (cfgInParam.img_crop_w > pipesize_bin) {
        ret = MFALSE;
        PIPE_ERR("size too large: %d\n", cfgInParam.img_crop_w);
        goto EXIT;
    }
    if (cfgInParam.img_crop_w > pipesize) {
        if (cfgInParam.img_crop_w > pipesize_twin) {
            bBin_first = MTRUE;
        }
    }
    if ((cfgInParam.offTwin == MFALSE) && (availCam.size() > 0)) {
        if ((thru_put/2) >= (MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * CLK_MARGIN_N / CLK_MARGIN_M)) {
            bBin_first = MTRUE;
        }
    }

    if (MFALSE == this->Res_Meter_kernel(cfgInParam, cfgOutParam, thru_put, bBin_useful, bBin_first, availCam)) {
        ret = MFALSE;
        goto EXIT;
    }
    #if 1// check if need to add width > max_width contraint case
    if (cfgInParam.img_crop_w > pipesize) {
        MUINT32 max_pipesize = 0;

        PIPE_DBG("post adjust twin/bin for img size too large, but throughput is too small\n");
        do {
            if (cfgOutParam.bin_en) {
                max_pipesize = pipesize_bin;
            }
            else {
                if (cfgOutParam.isTwin && bFbnd_on) {
                    max_pipesize = pipesize_bin;
                }
                else if (bFbnd_on) {
                    max_pipesize = _GET_PIPELINE_SIZE(camcap, qrst, MFALSE, MTRUE);;
                }
                else if(cfgOutParam.isTwin) {
                    max_pipesize = pipesize_twin;
                }
                else {
                    max_pipesize = pipesize;
                }
            }

            if (cfgInParam.img_crop_w <= max_pipesize) {
                break;
            }

            if (bBin_useful == MTRUE) {
                this->K_BIN_CASE(cfgOutParam, thru_put, bBin_useful);
            }
            else if ((cfgInParam.offTwin == MFALSE) && (availCam.size() != 0)) {
                this->K_TWIN_CASE(cfgOutParam, thru_put, availCam);
            }
            else {
                PIPE_ERR("input size too large, out of spec: width(%d) bin/twin=%d/%d pipesize:%d/%d/%d/%d\n",
                    cfgInParam.img_crop_w, cfgOutParam.bin_en, cfgOutParam.isTwin, pipesize, pipesize_bin,
                    pipesize_twin, max_pipesize);
                ret = MFALSE;
                goto EXIT;
            }
        }while (cfgInParam.img_crop_w > max_pipesize);
    }
    #endif

    if ((cfgInParam.bypass_rrzo == MTRUE) && (cfgOutParam.isTwin == MTRUE)) {
        PIPE_ERR("rrz is needed under twin mode\n");
        ret = MFALSE;
        goto EXIT;
    }

    //4. dlp pix mode setting
    if (cfgInParam.bypass_tg) { //rawi path
        //rawi support upto 2pix mode, capability suggests direct codes 2pix here,
        //due to no 1/2 pix change possiblity
        cfgOutParam.dlp.RAW_SELO = ePixMode_2;
    }
    else {
        cfgOutParam.dlp.RAW_SELO = cfgInParam.pix_mode_tg;
    }

    cur_pix_mode = cfgOutParam.dlp.RAW_SELO;
    if (bDbn_on) {
        cur_pix_mode = _PixMode_Down[cur_pix_mode];
    }
    if (cfgOutParam.bin_en) {
        cur_pix_mode = _PixMode_Down[cur_pix_mode];
    }
    cfgOutParam.dlp.SEPI = cur_pix_mode;

    if (cfgOutParam.bin_en) {
        cur_pix_mode = _PixMode_Down[cur_pix_mode];
    }

    if (bFbnd_on) {
        cfgOutParam.dlp.SEPO = (cfgOutParam.dlp.SEPI < ePixMode_2) ? cfgOutParam.dlp.SEPI : ePixMode_2;
    }
    else {
        cfgOutParam.dlp.SEPO = ePixMode_1;
    }

    if (cfgOutParam.dlp.SEPI < cfgOutParam.dlp.SEPO) {
        PIPE_ERR("pix_mode: raw_sel(%d)-sepi(%d)-sepo(%d) cur_pix_mode(%d) bin(%d) dbn(%d) fbnd(%d) sepo pix mode too high!!\n",
            cfgOutParam.dlp.RAW_SELO, cfgOutParam.dlp.SEPI, cfgOutParam.dlp.SEPO, cur_pix_mode, cfgOutParam.bin_en, bDbn_on, bFbnd_on);
    }

    if (cfgOutParam.isTwin == MTRUE) {
        switch (cfgOutParam.slave_module.size()) {
        case 1:
            //partial merge if not 2pix mode, still have overhead
            cfgOutParam.dlp.MRG_R3 = cfgOutParam.dlp.MRG_R12 = ePixMode_2;
            cfgOutParam.dlp.MRG_R7 = cfgOutParam.dlp.MRG_R11 = ePixMode_2;
            cfgOutParam.dlp.MRG_R13= cfgOutParam.dlp.MRG_R14 = ePixMode_2;
            cfgOutParam.dlp.MRG_R8 = ePixMode_2;
            cfgOutParam.dlp.MRG_R6 = ePixMode_2;
            break;
        default:
            PIPE_ERR("unsupported twin slave num: %d\n",cfgOutParam.slave_module.size());
            break;
        }
    }
    else {
        cfgOutParam.dlp.MRG_R3 = cfgOutParam.dlp.MRG_R12 = ePixMode_1;
        cfgOutParam.dlp.MRG_R7 = cfgOutParam.dlp.MRG_R11 = ePixMode_1;
        cfgOutParam.dlp.MRG_R13= cfgOutParam.dlp.MRG_R14 = ePixMode_1;
        cfgOutParam.dlp.MRG_R8 = ePixMode_1;
        cfgOutParam.dlp.MRG_R6 = ePixMode_1;
    }
    PIPE_INF("twin:%d pix_mode: raw_sel(%d)-sepi(%d)-sepo(%d) bin(%d) dbn(%d) fbnd(%d)\n", cfgOutParam.isTwin,
        cfgOutParam.dlp.RAW_SELO, cfgOutParam.dlp.SEPI, cfgOutParam.dlp.SEPO, cfgOutParam.bin_en, bDbn_on, bFbnd_on);

EXIT:
    return ret;
}


MBOOL Cam_ResMgr::Res_Meter_YUV(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam)
{
    MBOOL ret = MTRUE;

    cfgOutParam.dlp.RAW_SELO = cfgInParam.pix_mode_tg;
    cfgOutParam.dlp.SEPI = cfgInParam.pix_mode_tg;
    cfgOutParam.dlp.SEPO = cfgInParam.pix_mode_tg;
    //LafiteEP_TODO: R1 has no pixmode opt: cfgOutParam.dlp.MRG_R1 = ePixMode_1;
    cfgOutParam.dlp.MRG_R8 = ePixMode_1;
    return ret;
}



ISP_HW_MODULE Cam_ResMgr::Res_GetCurrentModule(void)
{
    return this->m_hwModule;
}


MINT32 Cam_ResMgr::Res_Recursive(V_CAM_THRU_PUT* pcfgIn)
{
    MUINT32 clk_margin = CLK_MARGIN_N;
    MINT32  final_clk = -1;
    MUINT32 thru_put,dataBin=0;
    MUINT32 shift = 0;

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};

        property_get("vendor.camsys.margin", value, "0");
        MINT32 clklv = atoi(value);
        if(clklv != 0){
            PIPE_INF("force to margin:%d\n",clklv);
            clk_margin = clklv;
        }
    }

    switch (this->m_SenInfo.SrcImgFmt) {
        case TG_FMT_RAW8:
        case TG_FMT_RAW10:
        case TG_FMT_RAW12:
        case TG_FMT_RAW14:
            break;
        default:
            PIPE_ERR("unsupported fmt:%d\n",this->m_SenInfo.SrcImgFmt);
            return -1;
            break;
    }

    switch(this->m_SenInfo.pattern){
        case eCAM_DUAL_PIX:
        case eCAM_4CELL:
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
            dataBin = 1;
            break;
        default:
            break;
    }

    for(MUINT32 i=0;i<pcfgIn->size();i++){
        shift = 0;
        shift += (pcfgIn->at(i).bBin + dataBin);

        thru_put = (((MUINT64)pcfgIn->at(i).SEP_W)*pcfgIn->at(i).SEP_H*this->m_SenInfo.raw_fps / 10)+1;
        thru_put = thru_put>>shift;
        if(final_clk == -1)
            final_clk = this->m_SenInfo.curClklv;
        while(thru_put >= (MUINT32)((MUINT64)this->m_SenInfo.vClk.at(final_clk) * clk_margin / CLK_MARGIN_M)){
            if(final_clk < (this->m_SenInfo.vClk.size()-1)){
                final_clk++;
                PIPE_WRN("CAM%d:speed up clk level due to twin overhead:(%d,%d),(%d_%d_%d)",i,this->m_SenInfo.curClklv,final_clk,\
                    pcfgIn->at(i).SEP_W,pcfgIn->at(i).SEP_H,this->m_SenInfo.raw_fps);
            }
            else{
                PIPE_ERR("out of spec with twin's overhead(CAM%d:clk:%d,thru-put(%d_%d_%d))\n",\
                    i,this->m_SenInfo.vClk.at(final_clk),thru_put,\
                    pcfgIn->at(i).SEP_W,pcfgIn->at(i).SEP_H,this->m_SenInfo.raw_fps);
                return -1;
            }
        }
    }

    if(final_clk == this->m_SenInfo.curClklv)
        return -1;
    else
        return final_clk;
}

