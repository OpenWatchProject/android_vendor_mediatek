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

#define NN NSCam::NSIoPipe::NSCamIOPipe
#define BIN_THRU_PUT    (1)
#define ClkMargin       95/100

MBOOL Central::get_availNum(vector<ISP_HW_MODULE>& v_available,char Name[32])
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);
    MUINT32 availsize = this->m_available;

    if(*Name == '\0'){
        BASE_LOG_ERR("cant have NULL userName\n");
        return MFALSE;
    }

    v_available.clear();
    for(MUINT32 i=0;i<PHY_CAM;i++){
        if(this->m_module[i].occupied == MFALSE){
            v_available.push_back((ISP_HW_MODULE)i);
        }
        else if( std::strcmp((const char*)this->m_module[i].User,(const char*)Name) == 0){
            //this module is used by CamResMgr itself, so still count into available cam
            v_available.push_back((ISP_HW_MODULE)i);
            availsize++;
        }
    }

    if(v_available.size() != availsize){
        #define str_size (128)
        char str[str_size];
        int cx = 0;
        *str = '\0';
        BASE_LOG_ERR("size mismatch : %zu%d\n",v_available.size(),availsize);
        for(MUINT32 i=0;i<PHY_CAM;i++){
            cx = std::snprintf(str+cx,str_size-cx,"cam:%d_%d,",i ,this->m_module[i].occupied);
        }
        BASE_LOG_ERR("resource pool: %s\n",str);
        v_available.clear();
        ret = MFALSE;
    }

    return ret;
}

MBOOL Central::Register(ISP_HW_MODULE module,char Name[32],MUINT32 logLevel)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);
    if(module >= PHY_CAM){
        BASE_LOG_ERR("unsupported cam:%d\n",module);
        ret = MFALSE;
        goto EXIT;
    }
    else if(*Name == '\0'){
        BASE_LOG_ERR("cant have NULL userName\n");
        ret = MFALSE;
        goto EXIT;

    }

    if(this->m_module[module].occupied == MFALSE){
        this->m_module[module].occupied = MTRUE;
        memset((void*)this->m_module[module].User, 0, sizeof(this->m_module[module].User));
        std::strncpy((char*)this->m_module[module].User,(char const*)Name,sizeof(this->m_module[module].User)-1);
        this->m_available--;
    }
    else if( std::strncmp((const char*)Name,(const char*)Cam_ResMgr::m_staticName,strlen((char*)Cam_ResMgr::m_staticName)) == 0){//from ResMgr
        if( std::strcmp((const char*)this->m_module[module].User,(const char*)Name) == 0){//the same ResMgr
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
    tCAM_rst rst;
    capibility CamInfo(NN::ENPipe_CAM_B);
    NN::NormalPipe_InputInfo info;
    info.width = width;

    if(CamInfo.GetCapibility(0,NN::ENPipeQueryCmd_UNKNOWN,
                                info,rst,E_CAM_pipeline_size) == MFALSE){
        goto EXIT;
    }


    if(width < rst.pipeSize){
        if(this->Register(CAM_B,Name,0) == MTRUE){        //CAM_B 1st when width < pipeline size
            return CAM_B;
        }
        else if(this->Register(CAM_A,Name) == MTRUE){   //if CAM_B is occupied using CAM_A
            return CAM_A;
        }
        else{
            BASE_LOG_ERR("no available cam, isp lunch fail\n");
            return CAM_MAX;
        }
    }
    else{
        if(this->Register(CAM_A,Name,0) == MTRUE){        //CAM_A 1st when width > pipeline size
            return CAM_A;
        }
        else if(this->Register(CAM_B,Name) == MTRUE){   //if CAM_A is occupied using CAM_B
            return CAM_B;
        }
        else{
            BASE_LOG_ERR("no available cam, isp lunch fail\n");
            return CAM_MAX;
        }
    }

EXIT:
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
    std::strncpy((char*)m_User,(char const*)Name,sizeof(m_User)-1);

    m_vSlave.clear();
    m_slave_module_release.clear();
    m_switchLatency = Cam_ResMgr::REG_Module;
    sprintf(m_internalUser,"%s_%d",Cam_ResMgr::m_staticName,master);
}

Cam_ResMgr* Cam_ResMgr::Res_Attach(ISP_HW_MODULE master,char Name[32])
{
    MBOOL ret = MTRUE;
    BASE_LOG_INF("CAM Res_Attach:: module:%d,name:%s\n",master,Name);
    Mutex::Autolock lock(Cam_ResMgr::mLock);
    if(Cam_ResMgr::m_central.Register(master,Name) == MFALSE){
        return NULL;
    }

    return new Cam_ResMgr(master,Name);
}

Cam_ResMgr* Cam_ResMgr::Res_Attach(MUINT32 width,char Name[32])
{
    MBOOL ret = MTRUE;
    ISP_HW_MODULE master;

    BASE_LOG_INF("CAM Res_Attach:: size:%d,name:%s\n",width,Name);
    Mutex::Autolock lock(Cam_ResMgr::mLock);
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
#define ClkMargin   95/100
    MBOOL ret = MTRUE;
    tCAM_rst rst;
    capibility* pCamInfo = NULL;
    vector<ISP_HW_MODULE> availCam;
    Cam_ResMgr* ptr = NULL;
    char str[512] = {'\0'}, _tmp[256] = {'\0'};
    MINT32 curLeng;
    Mutex::Autolock lock(mLock);
    MUINT32 cur_clkL = cfgOutParam.clk_level;

    //reset clk_level
    cfgOutParam.clk_level = 0;
    cfgOutParam.slave_module.clear();

    //
    if(this->Res_Meter_Release() == MFALSE){
        PIPE_ERR("meter_release fail\n");
        ret = MFALSE;
        goto EXIT;
    }

    this->m_central.get_availNum(availCam,this->m_internalUser);
    //
    curLeng = snprintf(str, 511,\
    "cfgIn:tg_en(%d), tg_pix(%d), tg_crop(%dx%d),fps(%d), rrzo_en(%d), rrz_out(%d),cur clk:%d,off_bin:%d,off_twin:%d,avail_cam:%zu,MIPI_pixrate:%d khz,fmt:%d,senNum:%d\n", \
            cfgInParam.bypass_tg, cfgInParam.pix_mode_tg, cfgInParam.tg_crop_w, cfgInParam.tg_crop_h,cfgInParam.tg_fps,\
            cfgInParam.bypass_rrzo, cfgInParam.rrz_out_w,cfgInParam.vClk.at(cur_clkL),cfgInParam.offBin,\
            cfgInParam.offTwin,availCam.size(),cfgInParam.MIPI_pixrate,cfgInParam.SrcImgFmt,cfgInParam.SenNum);


    /**
        note:
            also need to make sure cfgOutParam.clk_level is started from 0. always use lowest clk, and notify clk request of isp to MMDVFS each frame.
        */


    // configPath
    if(cfgInParam.bypass_tg == MTRUE)
    {
        PIPE_ERR("bypass_tg is true!!, twin_mgr support no RAWI currently! \n");
        ret = MFALSE;
        goto EXIT;
    }



    //query pipeline size
    switch(this->m_hwModule){
        case CAM_A:
            pCamInfo = new capibility(NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_A);
            break;
        case CAM_B:
            pCamInfo = new capibility(NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_B);
            break;
        case CAM_C:
            pCamInfo = new capibility(NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C);
            break;
        default:
            PIPE_ERR("unsupported module:%d \n",this->m_hwModule);
            ret = MFALSE;
            goto EXIT;
            break;
    }
    if(pCamInfo != NULL){
        if(pCamInfo->GetCapibility(0,NN::ENPipeQueryCmd_UNKNOWN,
                                    NN::NormalPipe_InputInfo(),
                                    rst,E_CAM_pipeline_size) == MFALSE){
            ret = MFALSE;
            goto EXIT;
        }
    }
    else{
        PIPE_ERR("this obj shouldn't be null\n");
        ret = MFALSE;
        goto EXIT;
    }

    switch (cfgInParam.SrcImgFmt) {
        case TG_FMT_RAW8:
        case TG_FMT_RAW10:
        case TG_FMT_RAW12:
        case TG_FMT_RAW14:
            if( (ret = this->Res_Meter_Bayer(cfgInParam,cfgOutParam,&rst,cur_clkL)) == MFALSE){
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
            "[%d]cfgOutParam: pix_mode_dmxi(%d), pix_mode_dmxo(%d), pix_mode_rmxo(%d), bin_en(%d), isTwin(%d),clk rate(%d)", \
                this->m_hwModule,cfgOutParam.xmxo.pix_mode_dmxi, cfgOutParam.xmxo.pix_mode_dmxo,  cfgOutParam.xmxo.pix_mode_rmxo, \
                cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level));
    }
    else if(cfgOutParam.slave_module.size() == 1){
        snprintf(_tmp, 255,\
            "[%d]cfgOutParam: pix_mode_dmxi(%d), pix_mode_dmxo(%d), pix_mode_rmxo(%d), bin_en(%d), isTwin(%d),clk rate(%d),slave(%d)", \
                this->m_hwModule,cfgOutParam.xmxo.pix_mode_dmxi, cfgOutParam.xmxo.pix_mode_dmxo,  cfgOutParam.xmxo.pix_mode_rmxo, \
                cfgOutParam.bin_en, cfgOutParam.isTwin,cfgInParam.vClk.at(cfgOutParam.clk_level),cfgOutParam.slave_module.at(0));
    }
    else{
        snprintf(_tmp, 255,\
            "[%d]cfgOutParam: pix_mode_dmxi(%d), pix_mode_dmxo(%d), pix_mode_rmxo(%d), bin_en(%d), isTwin(%d),clk rate(%d),slave1(%d),slave2(%d)", \
                this->m_hwModule,cfgOutParam.xmxo.pix_mode_dmxi, cfgOutParam.xmxo.pix_mode_dmxo,  cfgOutParam.xmxo.pix_mode_rmxo, \
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
    if(pCamInfo)
        delete pCamInfo;

    strncat(str,_tmp, (511-curLeng));
    PIPE_INF("%s\n", str);

    this->m_SenInfo.assign(cfgInParam.tg_fps,cfgInParam.MIPI_pixrate,\
        cfgInParam.SrcImgFmt,cfgInParam.pattern,cfgOutParam.clk_level,(vector<MUINT32>*)&cfgInParam.vClk);

    this->mLock.unlock();
    return ret;
}

MBOOL Cam_ResMgr::Res_Meter_Release(void)
{
    MBOOL ret = MTRUE;

    // e.g.: twin -> single, twin3->twin2
    if(this->m_switchLatency == Cam_ResMgr::REL_Module){
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

MVOID Cam_ResMgr::K_BIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,MBOOL& bBin_useful,MUINT32& bin_pix_mode)
{
    bBin_useful = MFALSE;
    cfgOutParam.bin_en = MTRUE;
    thru_put = thru_put>>BIN_THRU_PUT;
    bin_pix_mode++;
}

MVOID Cam_ResMgr::K_TWIN_CASE(Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,vector<ISP_HW_MODULE>& availCam)
{
    if(availCam.size()>0){
        MUINT32 i=0;
        MBOOL bflag = MFALSE;
        //cam_c is 1st priority
        for(i=0;i<availCam.size();i++){
            if(availCam.at(i) == CAM_C){
                bflag = MTRUE;
                break;
            }
        }
        i= (bflag == MFALSE)?( (i>=1)?(i-1):(i) ) : (i);//if search no CAM_C
        cfgOutParam.slave_module.push_back(availCam.at(i));
        availCam.erase(availCam.begin()+i);
        cfgOutParam.isTwin = MTRUE; //twin must be enabled
        if(cfgOutParam.slave_module.size() > 1)
            thru_put = thru_put*cfgOutParam.slave_module.size();
        thru_put = thru_put / (cfgOutParam.slave_module.size()+1); //
    }
}

MBOOL Cam_ResMgr::Res_Meter_kernel(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam,MUINT32& thru_put,MBOOL& bBin_useful,MUINT32& bin_pix_mode,vector<ISP_HW_MODULE>& availCam)
{
    //check thru put, always counting from clk_level = 0 for isp minimum request
    //start calc. thru put
    //priority : 1:twin.  2:bin.  3:Clk.
    while(thru_put >= (MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * ClkMargin))
    {
        if((cfgInParam.offTwin == MFALSE) && (availCam.size() != 0)){
            this->K_TWIN_CASE(cfgOutParam,thru_put,availCam);
        }
        else if(bBin_useful == MTRUE){   //2nd
            this->K_BIN_CASE(cfgOutParam,thru_put,bBin_useful,bin_pix_mode);
        }
        else if(cfgOutParam.clk_level < (cfgInParam.vClk.size()-1)){    //3rd
            this->K_CLK_CASE(cfgOutParam);
        }
        else{
            PIPE_ERR("over spec! pattern:%d thru put:(%dx%dx%d,total:%d), max clk:%d(margin:%d), bin_en(%d_%d), twin_en(%d)\n",\
                cfgInParam.pattern,\
                cfgInParam.tg_crop_w,cfgInParam.tg_crop_h,cfgInParam.tg_fps,thru_put,\
                cfgInParam.vClk.at(cfgOutParam.clk_level),(MUINT32)((MUINT64)cfgInParam.vClk.at(cfgOutParam.clk_level) * ClkMargin),\
                cfgInParam.offBin,cfgOutParam.bin_en,\
                cfgOutParam.isTwin);
            return MFALSE;
            break;
        }
    }

    return MTRUE;
}

MBOOL Cam_ResMgr::Res_Meter_Bayer(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam,const void *ptr,MUINT32 cur_clkL)
{
    MBOOL ret = MTRUE;
    MUINT32 thru_put = 0;
    MUINT32 bin_pix_mode = 0;
    MBOOL bBin_useful = MTRUE;
    vector<ISP_HW_MODULE> availCam;

    //
    if(ptr == NULL){
        PIPE_ERR("capibility can't be NULL");
        return MFALSE;
    }

    // 0. total data size
    if(cfgInParam.MIPI_pixrate == 1000000){
        thru_put = (MUINT32) (((MUINT64)cfgInParam.tg_crop_w)*cfgInParam.tg_crop_h*cfgInParam.tg_fps / 10);
    }
    else
        thru_put = cfgInParam.MIPI_pixrate;

    if(thru_put == 0){
        PIPE_ERR("thru put error,fps(x10):%d,size:%d_%d\n",cfgInParam.tg_fps,cfgInParam.tg_crop_w,cfgInParam.tg_crop_h);
        ret = MFALSE;
        goto EXIT;
    }

    this->m_central.get_availNum(availCam,this->m_internalUser);
    //check sensor_number & modify availCam
    if(cfgInParam.SenNum != NSImageio::NSIspio::ECamio_1_SEN){
        MUINT32 _loop = 0;
        do{
            if(availCam.size() >=1){
                if( ( *availCam.begin() == CAM_A) || ( *availCam.begin() == CAM_B)){
                    availCam.erase(availCam.begin());
                }
            }
        }while(_loop++ < 5);
    }

    //rrz can't scale-up
    if(bBin_useful == MTRUE){
        bBin_useful = (cfgInParam.offBin == MTRUE) ? MFALSE : \
        ((cfgInParam.rrz_out_w <= (cfgInParam.tg_crop_w>>BIN_THRU_PUT)) ? (MTRUE):(MFALSE));
    }

    //check dbn/bin/linebuffer case
    {
        tCAM_rst* rst = (tCAM_rst*)ptr;
        MUINT32 _div;

        switch(cfgInParam.pattern){
            case eCAM_DUAL_PIX:
            case eCAM_4CELL:
            case eCAM_4CELL_IVHDR:
            case eCAM_4CELL_ZVHDR:
                if(cfgInParam.pattern == eCAM_DUAL_PIX)
                    thru_put = thru_put << 1;//*2 is for TG_width under dualPD, because of TG from sesnor is not the real size of TG_grab
                bBin_useful = MFALSE;   //Bin/DBN is mutually exclusive

                //for DBN
                //DMXO should be configured as two pixel-out in Dual_PD case.
                bin_pix_mode++;
                if(cfgInParam.pix_mode_tg == ePixMode_1)
                    PIPE_ERR("DMXI is one pixel-in but DMXO is two pixel-out. This path is not verified!!");
                thru_put = thru_put>>1;

                if(cfgInParam.pattern != eCAM_DUAL_PIX)
                    _div = 2;
                else
                    _div = 1;//dualpd use 1 is because of TG will be the size which is after DBN.
                if((cfgInParam.tg_crop_w/_div) > rst->pipeSize){
                    PIPE_ERR("under pattern:%d, size after DBN still too large(%d_%d), need twin\n",\
                        cfgInParam.pattern,cfgInParam.tg_crop_w,rst->pipeSize);
                    //twin + dbn is not verified!
                }
                else{
                    PIPE_WRN("force to off Twin\n");
                    //twin + dbn is not verified
                    availCam.clear();
                }

                break;
            case eCAM_MONO:
            case eCAM_ZVHDR:
                bBin_useful = MFALSE;
                break;
            default:
                break;
        }
        // check max size, if over max size, need enable twin
        if(cfgInParam.tg_crop_w > rst->pipeSize){
            MUINT32 _width = cfgInParam.tg_crop_w;
            MUINT32 _loop = 0;
            do{
                if(bBin_useful){
                    this->K_BIN_CASE(cfgOutParam,thru_put,bBin_useful,bin_pix_mode);
                    PIPE_DBG("bin is enabled because of over max size:%d_%d\n",cfgInParam.tg_crop_w,rst->pipeSize);
                }
                else{
                    if(cfgInParam.offTwin == MFALSE){
                        this->K_TWIN_CASE(cfgOutParam,thru_put,availCam);
                        PIPE_DBG("twin is enabled because of over max size:%d_%d,slave:%d\n",cfgInParam.tg_crop_w,\
                            rst->pipeSize,cfgOutParam.slave_module.at(0));
                    }
                    else{
                        ret = MFALSE;
                        PIPE_ERR("twin need to be enabled because of linebuffer , but offTwin is forced on\n");
                        goto EXIT;
                    }
                }
                _width /= 2;
                if(_width <= rst->pipeSize)
                    break;
                if(++_loop > 5){   //avoid close loop
                    PIPE_ERR("logic err\n");
                    break;
                }
            }while(1);
        }
    }

    //thru-put
    if(MFALSE == this->Res_Meter_kernel(cfgInParam,cfgOutParam,thru_put,bBin_useful,bin_pix_mode,availCam)){
        ret = MFALSE;
        goto EXIT;
    }

    //
    if(cfgInParam.bypass_rrzo == MTRUE){
        if(cfgOutParam.isTwin == MTRUE){
            PIPE_ERR("rrz is needed under twin mode\n");
            return MFALSE;
        }
    }

    //4. xmx pix mode setting
    cfgOutParam.xmxo.pix_mode_dmxi = cfgInParam.pix_mode_tg;
    switch(bin_pix_mode){//fix dmx_pos_sel
        case 0: cfgOutParam.xmxo.pix_mode_dmxo = ePixMode_1; break;
        case 1: cfgOutParam.xmxo.pix_mode_dmxo = ePixMode_2; break;
        case 2: cfgOutParam.xmxo.pix_mode_dmxo = ePixMode_4; break;
        default:
            PIPE_ERR("unsupported pix:%d\n",bin_pix_mode);
            break;
    }
    cfgOutParam.xmxo.pix_mode_bmxo = ePixMode_1;
    cfgOutParam.xmxo.pix_mode_rmxo = ePixMode_1;
    cfgOutParam.xmxo.pix_mode_amxo = ePixMode_1;

    if(cfgOutParam.isTwin == MTRUE){
        //cfgOutParam.xmxo.pix_mode_bmxo = ePixMode_1;//always 1-pix mode, use rmb to reduce data thru. put.
        switch(cfgOutParam.slave_module.size()){
            case 1:
                cfgOutParam.xmxo.pix_mode_amxo = cfgOutParam.xmxo.pix_mode_rmxo = ePixMode_2;
                break;
            case 2:
                cfgOutParam.xmxo.pix_mode_amxo = cfgOutParam.xmxo.pix_mode_rmxo = ePixMode_4;
                break;
            default:
                PIPE_ERR("unsupported twin:%zu\n",cfgOutParam.slave_module.size());
                break;
        }
    }

EXIT:
    return ret;
}


MBOOL Cam_ResMgr::Res_Meter_YUV(const Res_Meter_IN& cfgInParam, Res_Meter_OUT &cfgOutParam)
{
    MBOOL ret = MTRUE;

    cfgOutParam.xmxo.pix_mode_dmxi = cfgInParam.pix_mode_tg;
    cfgOutParam.xmxo.pix_mode_dmxo = cfgInParam.pix_mode_tg;
    cfgOutParam.xmxo.pix_mode_bmxo = ePixMode_1;
    cfgOutParam.xmxo.pix_mode_rmxo = ePixMode_1;
    cfgOutParam.xmxo.pix_mode_amxo = ePixMode_1;

    return ret;
}



ISP_HW_MODULE Cam_ResMgr::Res_GetCurrentModule(void)
{
    return this->m_hwModule;
}


MINT32 Cam_ResMgr::Res_Recursive(V_CAM_THRU_PUT* pcfgIn)
{
    MUINT32 clk_margin = 95;
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

        thru_put = (((MUINT64)pcfgIn->at(i).DMX_W)*pcfgIn->at(i).DMX_H*this->m_SenInfo.tg_fps / 10)+1;
        thru_put = thru_put>>shift;
        if(final_clk == -1)
            final_clk = this->m_SenInfo.curClklv;
        while(thru_put >= (MUINT32)((MUINT64)this->m_SenInfo.vClk.at(final_clk) * clk_margin / 100)){
            if(final_clk < (this->m_SenInfo.vClk.size()-1)){
                final_clk++;
                PIPE_WRN("CAM%d:speed up clk level due to twin overhead:(%d,%d),(%d_%d_%d)",i,this->m_SenInfo.curClklv,final_clk,\
                    pcfgIn->at(i).DMX_W,pcfgIn->at(i).DMX_H,this->m_SenInfo.tg_fps);
            }
            else{
                PIPE_ERR("out of spec with twin's overhead(CAM%d:clk:%d,thru-put(%d_%d_%d))\n",\
                    i,this->m_SenInfo.vClk.at(final_clk),thru_put,\
                    pcfgIn->at(i).DMX_W,pcfgIn->at(i).DMX_H,this->m_SenInfo.tg_fps);
                return -1;
            }
        }
    }

    if(final_clk == this->m_SenInfo.curClklv)
        return -1;
    else
        return final_clk;
}

