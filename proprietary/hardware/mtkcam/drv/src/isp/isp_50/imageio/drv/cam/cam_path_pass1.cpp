#define LOG_TAG "path_cam"

#include "cam_path_cam.h"
#include "cam_capibility.h"

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


DECLARE_DBG_LOG_VARIABLE(path);

using namespace NSImageio;
using namespace NSIspio;


inline static MBOOL isUfo(EImageFormat iFmt)
{
    switch (iFmt ) {
    case eImgFmt_UFO_BAYER8:
    case eImgFmt_UFO_BAYER10:
    case eImgFmt_UFO_BAYER12:
    case eImgFmt_UFO_BAYER14:
    case eImgFmt_UFO_FG_BAYER8:
    case eImgFmt_UFO_FG_BAYER10:
    case eImgFmt_UFO_FG_BAYER12:
    case eImgFmt_UFO_FG_BAYER14:
        return MTRUE;
    default:
        return MFALSE;
    }
}

inline static EImageFormat bayerUfoFormatConvert(EImageFormat iFmt)
{
    switch (iFmt ) {
    case eImgFmt_UFO_BAYER8:        return eImgFmt_BAYER8;
    case eImgFmt_UFO_BAYER10:       return eImgFmt_BAYER10;
    case eImgFmt_UFO_BAYER12:       return eImgFmt_BAYER12;
    case eImgFmt_UFO_BAYER14:       return eImgFmt_BAYER14;
    case eImgFmt_UFO_FG_BAYER8:     return eImgFmt_FG_BAYER8;
    case eImgFmt_UFO_FG_BAYER10:    return eImgFmt_FG_BAYER10;
    case eImgFmt_UFO_FG_BAYER12:    return eImgFmt_FG_BAYER12;
    case eImgFmt_UFO_FG_BAYER14:    return eImgFmt_FG_BAYER14;

    case eImgFmt_BAYER8:        return eImgFmt_UFO_BAYER8;
    case eImgFmt_BAYER10:       return eImgFmt_UFO_BAYER10;
    case eImgFmt_BAYER12:       return eImgFmt_UFO_BAYER12;
    case eImgFmt_BAYER14:       return eImgFmt_UFO_BAYER14;
    case eImgFmt_FG_BAYER8:     return eImgFmt_UFO_FG_BAYER8;
    case eImgFmt_FG_BAYER10:    return eImgFmt_UFO_FG_BAYER10;
    case eImgFmt_FG_BAYER12:    return eImgFmt_UFO_FG_BAYER12;
    case eImgFmt_FG_BAYER14:    return eImgFmt_UFO_FG_BAYER14;
    default:
        return eImgFmt_UNKNOWN;
    }
}

inline static void bayerUfoDmaCfgConvert(MUINT32 portId, IspDMACfg &a_dma, IspDMACfg &bayer_dma)
{
    capibility  CamInfo;
    tCAM_rst    rst;

    a_dma.memBuf.size       = 0;
    a_dma.memBuf.base_vAddr = 0;
    a_dma.memBuf.base_pAddr = 0;
    a_dma.memBuf.alignment  = 0;
    a_dma.memBuf.ofst_addr  = 0; //offset at isp function

    a_dma.lIspColorfmt      = bayer_dma.lIspColorfmt;
    a_dma.pixel_byte        = bayer_dma.pixel_byte;

    a_dma.size.w            = bayer_dma.size.w;
    a_dma.size.h            = bayer_dma.size.h;

    //input xsize unit:byte
    if (CamInfo.GetCapibility(
            portId,
            (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd)(NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE),
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(bayer_dma.lIspColorfmt, a_dma.size.w, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN) == MFALSE) {
        BASE_LOG_ERR("CamInfo.GetCapibility error");
    }
    a_dma.size.xsize        = rst.xsize_byte[1];
    a_dma.size.stride       = rst.stride_byte[1];

    a_dma.crop.x            = 0;
    a_dma.crop.floatX       = 0;
    a_dma.crop.y            = 0;
    a_dma.crop.floatY       = 0;
    a_dma.crop.w            = bayer_dma.crop.w;
    a_dma.crop.h            = bayer_dma.crop.h;

    a_dma.swap              = bayer_dma.swap;
    a_dma.capbility         = bayer_dma.capbility;

    switch(bayer_dma.lIspColorfmt) {
    case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
        a_dma.format_en=1;
        a_dma.format=1;
        a_dma.bus_size_en=1;
        a_dma.bus_size=1;
        a_dma.swap=1;
        break;
    case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
        a_dma.format_en=1;
        a_dma.format=1;
        a_dma.bus_size_en=1;
        a_dma.bus_size=1;
        a_dma.swap=0;
        break;
    case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
        a_dma.format_en=1;
        a_dma.format=1;
        a_dma.bus_size_en=1;
        a_dma.bus_size=1;
        a_dma.swap=3;
        break;
    case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
        a_dma.format_en=1;
        a_dma.format=1;
        a_dma.bus_size_en=1;
        a_dma.bus_size=1;
        a_dma.swap=2;
        break;
    case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
    case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
    case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
    case eImgFmt_YV16:      //422 format, 3 plane
    case eImgFmt_NV16:      //422 format, 2 plane
        BASE_LOG_ERR("NOT support this format(0x%x) in cam\n", bayer_dma.lIspColorfmt);
        break;
    case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
    case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
    case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
    case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
        a_dma.format_en=0;
        a_dma.bus_size_en=0;
        a_dma.format=0;
        a_dma.bus_size=1;
        break;
    case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
    case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
    case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
    case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
    default:
        a_dma.format_en=0;
        a_dma.bus_size_en=0;
        a_dma.format=0;
        a_dma.bus_size=0;
        break;
    }
}

CamPathPass1::CamPathPass1() :
    m_p1NotifyTbl{  NSImageio::NSIspio::EPIPECmd_SET_RRZ_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_SGG2_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_EIS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_LCS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_RSS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_TUNING_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_AWB_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_REGDUMP_CBFP,//NOTE: cmd order must same as E_CB_MAP
                    NSImageio::NSIspio::EPIPECmd_MAX}
{
    DBG_LOG_CONFIG(imageio, path);

    for (int i = 0;i < p1Notify_node; i++) {
        this->m_p1NotifyObj[i] = NULL;
    }
    for (int i = 0; i < CAM_FUNCTION_MAX_NUM; i++) {
        this->m_isp_function_list[i] = NULL;
    }
    this->m_pCmdQdrv = NULL;
    this->m_isp_function_count = 0;
    this->m_TuningFlg = MFALSE;
    this->m_hwModule = MAX_ISP_HW_MODULE;
    this->m_FSM = op_unknown;
}


MINT32 CamPathPass1::config(struct CamPathPass1Parameter* p_parameter)
{
    MINT32 ret = 0;
    MUINT32 cnt=0;
    CAM_TIMESTAMP* pTime;

    ISP_PATH_DBG("CamPathPass1::config E");

    for (MUINT32 i = 0; i < p1Notify_node; i++) {
        this->m_p1NotifyObj[i] = NULL;
    }

    if (this->m_pCmdQdrv == NULL) {
        ISP_PATH_ERR("NULL cmdQMgr\n");
        return 1;
    }

    this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_TopCtrl;

    if (p_parameter->m_Path == CamPathPass1Parameter::_TG_IN) {
        switch (p_parameter->m_top_ctl.FMT_SEL.Bits.TG_FMT) {
        case TG_FMT_RAW8:
        case TG_FMT_RAW10:
        case TG_FMT_RAW12:
        case TG_FMT_RAW14:
            this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_RawCtrl;
            break;
        case TG_FMT_YUV422:
            this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_YuvCtrl;
            break;
        default:
            ISP_PATH_ERR("unsupported TG format:0x%x\n",p_parameter->m_top_ctl.FMT_SEL.Bits.TG_FMT);
            return 1;
        }
    }
    else {
        switch (p_parameter->m_RawIFmt) {
        case RAWI_FMT_RAW8:
        case RAWI_FMT_RAW10:
        case RAWI_FMT_RAW12:
        case RAWI_FMT_RAW14:
            this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_RawCtrl;
            break;
        default:
            ISP_PATH_ERR("unsupported DMAI format:0x%x\n",p_parameter->m_RawIFmt);
            return 1;
        }
    }

    if (p_parameter->bypass_imgo == MFALSE) {
        //revmove from functionlist, because of dmao enable & disable r via CQ.
        //it means dmao enable/disable need to start/stop as much times as DupCmdQ number
        //this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_Imgo;
    }
    else {
        this->m_Imgo.m_pIspDrv = NULL;
    }

    if (p_parameter->bypass_rrzo == MFALSE) {
        //revmove from functionlist, because of dmao enable & disable r via CQ.
        //it means dmao enable/disable need to start/stop as much times as DupCmdQ number
        //this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_Rrzo;

        this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_Rrz;
    }
    else {
        this->m_Rrzo.m_pIspDrv = NULL;
    }

    this->m_isp_function_count = cnt;

    //cfg CmdQMgr
    //CQ config moved into CmdQMgr
    //this->m_pCmdQdrv->CmdQMgr_attach(p_parameter->m_fps,p_parameter->m_subSample,this->m_hwModule,ISP_DRV_CQ_THRE0,(void*)&this->m_CQ0);

    if (p_parameter->bypass_tg == MFALSE) {
        this->m_TgCtrl.m_Crop = p_parameter->m_src_size;
        if(p_parameter->m_Path == CamPathPass1Parameter::_TG_IN)//TG input support only continuous mode
            this->m_TgCtrl.m_continuous = MTRUE;
        else
            this->m_TgCtrl.m_continuous = MFALSE;

        this->m_TgCtrl.m_SubSample = p_parameter->m_subSample;

        this->m_TgCtrl.m_PixMode = p_parameter->m_src_pixmode;

        //tg setting is via phy reg, so all vir_ptr are all acceptable
        this->m_TgCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];

        this->m_TgCtrl.m_Datapat = p_parameter->m_data_pat;

        //tg setting is not via m_isp_function_list . so setting directly here
        ret += this->m_TgCtrl.config();
        //tg need to enable 1st for vsync signal
        ret += this->m_TgCtrl.enable(NULL);

    }

    //asign timestamp clk rate
    pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0]);
    pTime->TimeStamp_SrcClk(p_parameter->m_tTimeClk);

    //top
    switch(this->m_hwModule){
        case CAM_A:
            if(p_parameter->m_Path == CamPathPass1Parameter::_TG_IN){
                this->m_TopCtrl.CAM_Path = IF_CAM_A;
            }
            else {
                this->m_TopCtrl.CAM_Path = IF_RAWI_CAM_A;
            }
            break;
        case CAM_B:
            if(p_parameter->m_Path == CamPathPass1Parameter::_TG_IN){
                this->m_TopCtrl.CAM_Path = IF_CAM_B;
            }
            else {
                this->m_TopCtrl.CAM_Path = IF_RAWI_CAM_B;
            }
            break;
        default:
            ISP_PATH_ERR("config fail:module out of range:0x%x\n",this->m_hwModule);
            return -1;
            break;
    }

    this->m_TopCtrl.SubSample = p_parameter->m_subSample;//this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); // not drv self-define

    // C++11 do not allow this kind of operation, use memcpy for workaround
    //this->m_TopCtrl.cam_top_ctl = p_parameter->m_top_ctl;
    memcpy((void *) &m_TopCtrl.cam_top_ctl, (void *) &p_parameter->m_top_ctl, sizeof(ST_CAM_TOP_CTRL));
    //this->m_TopCtrl.m_pIspDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
    this->m_RawCtrl.m_xmxo.pix_mode_dmxi = (E_CamPixelMode)p_parameter->m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXI;
    this->m_RawCtrl.m_xmxo.pix_mode_dmxo = (E_CamPixelMode)p_parameter->m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXO;
    this->m_RawCtrl.m_xmxo.pix_mode_bmxo = (E_CamPixelMode)p_parameter->m_top_ctl.FMT_SEL.Bits.PIX_BUS_BMXO;
    this->m_RawCtrl.m_xmxo.pix_mode_rmxo = (E_CamPixelMode)p_parameter->m_top_ctl.FMT_SEL.Bits.PIX_BUS_RMXO;
    this->m_RawCtrl.m_xmxo.pix_mode_amxo = (E_CamPixelMode)p_parameter->m_top_ctl.FMT_SEL.Bits.PIX_BUS_AMXO;
    this->m_RawCtrl.m_bRRz = (p_parameter->bypass_rrzo == MTRUE)?(MFALSE):(MTRUE);

    //raw or yuv pipe
    if(p_parameter->m_Path == CamPathPass1Parameter::_TG_IN){
        switch(p_parameter->m_top_ctl.FMT_SEL.Bits.TG_FMT){
            case TG_FMT_RAW8:
            case TG_FMT_RAW10:
            case TG_FMT_RAW12:
            case TG_FMT_RAW14:
                //raw pipe
                switch(p_parameter->m_NonScale_RawType){
                    case CamPathPass1Parameter::_PURE_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_FromTG;
                        break;
                    case CamPathPass1Parameter::_NON_LSC_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BeforLSC;
                        break;
                    default:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_AFTERLSC;
                        break;
                }
                this->m_RawCtrl.m_Source = this->m_TopCtrl.CAM_Path;
                this->m_RawCtrl.m_pP1Tuning = NULL;//null at config()
                this->m_RawCtrl.m_DataPat = p_parameter->m_data_pat;
                //this->m_RawCtrl.m_pIspDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                break;
            case TG_FMT_YUV422:
                //yuv pipe
                this->m_YuvCtrl.m_pP1Tuning = NULL;
                //this->m_YuvCtrl.m_pIspDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                break;
            default:
                ISP_PATH_ERR("unsupported TG format:0x%x\n",p_parameter->m_top_ctl.FMT_SEL.Bits.TG_FMT);
                return 1;
                break;
        }
    }
    else{
        switch (p_parameter->m_RawIFmt) {
            case RAWI_FMT_RAW8:
            case RAWI_FMT_RAW10:
            case RAWI_FMT_RAW12:
            case RAWI_FMT_RAW14:
                //raw pipe
                switch(p_parameter->m_NonScale_RawType){
                    case CamPathPass1Parameter::_PURE_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_FromTG;
                        break;
                    case CamPathPass1Parameter::_NON_LSC_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BeforLSC;
                        break;
                    default:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_AFTERLSC;
                        break;
                }
                this->m_RawCtrl.m_Source = this->m_TopCtrl.CAM_Path;
                this->m_RawCtrl.m_pP1Tuning = NULL;//null at config()
                this->m_RawCtrl.m_DataPat = p_parameter->m_data_pat;
                break;
            default:
                ISP_PATH_ERR("unsupported DMAI format:0x%x\n",p_parameter->m_RawIFmt);
                return 1;
                break;
        }
    }


    //dmao & dmao fbc

    //====================================================
    // DMAO part config
    //====================================================
    if(p_parameter->bypass_imgo == MFALSE){
        this->m_Imgo.dma_cfg        = p_parameter->m_imgo.dmao;
        this->m_Imgo.Header_Addr    = p_parameter->m_imgo.FH_PA;
        this->m_Imgo.m_fps          = p_parameter->m_fps;


        bayerUfoDmaCfgConvert(EPortIndex_IMGO, this->m_Ufeo.dma_cfg, p_parameter->m_imgo.dmao);

        this->m_Ufeo.m_fps = p_parameter->m_fps;

    }
    if(p_parameter->bypass_rrzo == MFALSE){
        this->m_Rrzo.dma_cfg        = p_parameter->m_rrzo.dmao;
        this->m_Rrzo.Header_Addr    = p_parameter->m_rrzo.FH_PA;
        this->m_Rrzo.m_fps          = p_parameter->m_fps;

        //rrz initial-cfg
        this->m_Rrz.rrz_in_roi = p_parameter->rrz_in_roi;
        this->m_Rrz.rrz_out_size = p_parameter->m_Scaler;
        this->m_Rrz.rrz_rlb_offset = p_parameter->rrz_rlb_offset;

        bayerUfoDmaCfgConvert(EPortIndex_RRZO, this->m_Ufgo.dma_cfg, p_parameter->m_rrzo.dmao);

        this->m_Ufgo.m_fps = p_parameter->m_fps;

    }


    //loop for burst / duplicated cmdQ
    for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++){
            //top
            this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];


            //CQ need no update burst & dup here. [cq-config r all phy]

            //raw or yuv pipe
            if(p_parameter->bypass_ispRawPipe == MFALSE){
                this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }
            else{
                this->m_YuvCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }

            //dmao & dmao fbc
            //if(p_parameter->bypass_imgo == MFALSE){
            //    this->m_Imgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            //}

            if(p_parameter->bypass_rrzo == MFALSE){
                //this->m_Rrzo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

                this->m_Rrz.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }
            ret += this->_config(NULL);

            if(p_parameter->bypass_ispRawPipe == MFALSE)
                this->m_RawCtrl.configNotify();
        }
    }


    //====================================================
    // FBC part config
    //====================================================
    this->m_Lcso_FBC.m_pTimeStamp = pTime;
    this->m_Eiso_FBC.m_pTimeStamp = pTime;
    this->m_Rsso_FBC.m_pTimeStamp = pTime;

    for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++){
            IspDrvVir *_pDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

            this->m_Magic.m_pIspDrv = _pDrv;
            this->m_Magic._write2CQ();

            if(p_parameter->bypass_imgo == MFALSE){
                //fbc's control is moved to campath_bufctrl due to UF mode

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                //plz reference the description at line:81
                this->m_Imgo.m_pIspDrv = _pDrv;
                ret += this->m_Imgo.config();
                ret += this->m_Imgo.write2CQ();
                ret += this->m_Imgo.enable(NULL);

                //uf's fbc is moved to campath_bufctrl due to UF mode

                this->m_Ufeo.m_pIspDrv = _pDrv;
                ret += this->m_Ufeo.config();
                ret += this->m_Ufeo.write2CQ();
                //dynami enable/disable
            }
            if(p_parameter->bypass_rrzo == MFALSE){
                //fbc's control is moved to campath_bufctrl due to UF mode

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                //plz reference the description at line:81
                this->m_Rrzo.m_pIspDrv = _pDrv;
                ret += this->m_Rrzo.config();
                ret += this->m_Rrzo.write2CQ();
                ret += this->m_Rrzo.enable(NULL);

                //uf's fbc is moved to campath_bufctrl due to UF mode

                this->m_Ufgo.m_pIspDrv = _pDrv;
                ret += this->m_Ufgo.config();
                ret += this->m_Ufgo.write2CQ();
                //dynami enable/disable
            }
        }
    }


    {
        list<MUINT32> channel;
        NSImageio::NSIspio::PortID input;

        channel.clear();
        if (p_parameter->bypass_rrzo == MFALSE)
            channel.push_back(_rrzo_);
        if (p_parameter->bypass_imgo == MFALSE)
            channel.push_back(_imgo_);

        input.tTimeClk = p_parameter->m_tTimeClk;
        input.tgFps = p_parameter->m_fps;
        //
        this->m_PBC.PBC_config((void*)this,NULL,&channel,&input);
    }

    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, this->m_hwModule, CAM_BUF_CTRL::eCamDmaType_main);// modify in cannon migrated from bianco

    //tuning drv need to init before isp start, and init only once before or after start
    //no sensor id , remove to p1tunecfg
    //this->m_Tuning._config();
    //this->m_Tuning._enable(NULL);

    ISP_PATH_DBG("CamPathPass1::config X");

    //inorder to dump 1st frame cq for dbg only
    this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];

    this->m_FSM = op_cfg;

    return ret;
}


MINT32 CamPathPass1::_config(void *)
{
    MINT32          ret_val = ISP_ERROR_CODE_OK, ispcount;
    IspFunction_B** isplist;

    ISP_TRACE_CALL();

    ISP_PATH_DBG("[_config:0x%x]: E ",this->m_hwModule);

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for (int i = 0; i < ispcount; i++) {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("[%d]<%s> bypass:",i,isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if (( ret_val = isplist[i]->config() ) < 0) {
            ISP_PATH_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            goto EXIT;
        }

        if (( ret_val = isplist[i]->write2CQ() ) < 0) {
            ISP_PATH_ERR("[ERROR] _config  ");
            goto EXIT;
        }
    }

EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}

MINT32 CamPathPass1::start( void* pParam )
{
    MINT32          ret_val = ISP_ERROR_CODE_OK, ispcount, isp_start = 0;
    IspFunction_B** isplist;

    ISP_PATH_DBG("start:0x%x E ",this->m_hwModule);

    //flush whole isp setting into phy address
    //enforce cq load into reg before VF_EN if update rrz/tuning is needed
    if (this->m_pCmdQdrv->CmdQMgr_start()) {
        ISP_PATH_ERR("CQ start fail\n");
        return ISP_ERROR_CODE_FAIL;
    }

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    if (ret_val < 0) {
        ISP_PATH_ERR("[ERROR] _start ");
        goto EXIT;
    }

    for (int i = (ispcount-1) ; i >= 0 ; i-- ) {
        if( ( ret_val = isplist[i]->enable((void*)pParam ) ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _start enable ");
            goto EXIT;
        }
    }

    this->m_FSM = op_start;
    this->m_PBC.PBC_Start();
EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}


MINT32 CamPathPass1::init(DupCmdQMgr* pMainCmdQMgr)
{
    ISP_PATH_DBG("_init:0x%x E ",this->m_hwModule);

    this->m_pCmdQdrv = pMainCmdQMgr;

    this->m_Tuning._init();

    this->m_FSM = op_unknown;

    return 0;
}

MINT32 CamPathPass1::uninit(void)
{
    ISP_PATH_DBG("_uninit:0x%x E ",this->m_hwModule);

    if (this->m_TuningFlg) {
        this->m_Tuning._uninit();
        this->m_TuningFlg = MFALSE;
    }

    return 0;
}

/**
add this membor is for FBC disable
*/
MINT32 CamPathPass1::stop( void* pParam )
{
    MINT32              ret = 0;
    IspFunction_B**     isplist;
    int                 ispcount;

    // if under suspending, when dynamic twin, this may be twin's slave cam, and cannot
    // change phy reg during stop
    ISP_PATH_WRN("stop:: CAM_BUF_CTRL::m_CamState[0x%x]=0x%x this->m_FSM=0x%x",
        this->m_hwModule, CAM_BUF_CTRL::m_CamState[this->m_hwModule],this->m_FSM);

    if (this->m_FSM == op_suspend) {
        ISP_PATH_INF("stop when suspending, bypass touch hw reg\n");
        goto EXIT;
    }

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for (MUINT32 i = 0; i < this->m_pCmdQdrv->CmdQMgr_GetDuqQ(); i++) {
        for (MUINT32 j = 0; j < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); j++) {
            if (this->m_Imgo.m_pIspDrv != NULL) {
                this->m_Imgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                ret += this->m_Imgo.disable();
            }
            if (this->m_Ufeo.m_pIspDrv != NULL) {
                this->m_Ufeo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                ret += this->m_Ufeo.disable();
            }
            if (this->m_Rrzo.m_pIspDrv != NULL) {
                this->m_Rrzo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                ret += this->m_Rrzo.disable();
            }
            if (this->m_Ufgo.m_pIspDrv != NULL) {
                this->m_Ufgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                ret += this->m_Ufgo.disable();
            }
        }
    }

    for (int i = 0; i < ispcount; i++ ) {
        if( ( ret += isplist[i]->disable(pParam) ) != 0 ){
            ISP_PATH_ERR("_stop fail\n");
            goto EXIT;
        }
    }

    if ((this->m_Lcso_FBC.m_pIspDrv || m_Eiso_FBC.m_pIspDrv || m_Rsso_FBC.m_pIspDrv )) {
        for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++) {
                if(this->m_Lcso_FBC.m_pIspDrv != NULL){
                    this->m_Lcso_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                    ret += this->m_Lcso_FBC.disable();
                }
                if(this->m_Eiso_FBC.m_pIspDrv != NULL){
                    this->m_Eiso_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                    ret += this->m_Eiso_FBC.disable();
                }
                if(this->m_Rsso_FBC.m_pIspDrv != NULL){
                    this->m_Rsso_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                    ret += this->m_Rsso_FBC.disable();
                }
                //fbc's diable move to campath_buf_ctrl due to uf mode
            }
        }
    }
    this->m_Tuning._disable();  // tuning buf free in uninit()


    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, this->m_hwModule, CAM_BUF_CTRL::eCamDmaType_main);


EXIT:
    this->m_PBC.PBC_Stop();
    if (this->m_TgCtrl.m_pIspDrv != NULL) {
        this->m_TgCtrl.disable();
    }
    if (this->m_pCmdQdrv->CmdQMgr_stop()) {
        ret = 1;
    }

    this->m_FSM = op_stop;

    ISP_PATH_DBG(":X ");

    return ret;
}

MINT32 CamPathPass1::setP1Update(void)
{
    if (this->m_pCmdQdrv->CmdQMgr_update()) {
        ISP_PATH_ERR("CQ undate fail\n");
        return 1;
    }
    return 0;
}


MINT32 CamPathPass1::setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L)
{
    MINT32 ret = 0;
    capibility CamInfo;
    tCAM_rst rst,rstStrideByte;
    MUINT32 targetIdx = 0;
    MBOOL   bWithUfo = MFALSE;
    E_CamPixelMode _pix = ePixMode_NONE;
    list<DMACfg>::iterator it;
    int i;
    MUINT32 HwImgoFmt = IMGO_FMT_RAW8;
    Header_IMGO fh_imgo;
    Header_UFEO fh_ufeo;
    CAM_REG_CTL_FMT_SEL fmt_sel;

    if (pImgo_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("imgo data length mismatch:0x%x_0x%x\n",pImgo_cfg_L->size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();


    for (i = 0, it = pImgo_cfg_L->begin(); it != pImgo_cfg_L->end(); it++, i++) {
        this->m_Imgo.dma_cfg.lIspColorfmt = it->img_fmt;
        bWithUfo = isUfo(this->m_Imgo.dma_cfg.lIspColorfmt);

        switch(this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.TG_FMT){
            case TG_FMT_RAW8:
            case TG_FMT_RAW10:
            case TG_FMT_RAW12:
            case TG_FMT_RAW14:
                switch(it->rawType){
                case 0:
                    this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_AFTERLSC;
                    break;
                case 1:
                    this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_FromTG;
                    break;
                case 2:
                    this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BeforLSC;
                    break;
                default:
                    ISP_PATH_ERR("unsupported format:%d\n",it->rawType);
                    break;
                }
                this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                this->m_RawCtrl.setIMG_SEL();//setIMG_SEL must call before RawCtrl.getCurPixMode
                _pix = (E_CamPixelMode)this->m_RawCtrl.getCurPixMode(_imgo_);
                break;
            case TG_FMT_YUV422:
                _pix = (E_CamPixelMode)this->m_YuvCtrl.getCurPixMode(_imgo_);
                break;
            default:
                ISP_PATH_ERR("unsupported TG format:0x%x\n",this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.TG_FMT);
                break;
        }

        if (CamInfo.GetCapibility(
                EPortIndex_IMGO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Imgo.dma_cfg.lIspColorfmt, it->crop.w, _pix),
                rst, E_CAM_UNKNOWNN) == MFALSE) {
            ISP_PATH_ERR("cropping size err(0x%x)\n",it->crop.w);
            ret = 1;
        }

        if (CamInfo.GetCapibility(
                EPortIndex_IMGO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Imgo.dma_cfg.lIspColorfmt, it->tgCropW, ePixMode_4),
                rstStrideByte, E_CAM_UNKNOWNN) == MFALSE) {
            ISP_PATH_ERR("querying STRIDE_BYTE size err(0x%x)\n",it->tgCropW);
            ret = 1;
        }

        if(it->enqueue_img_stride < rstStrideByte.stride_byte[0]){
            ISP_PATH_ERR("enqueue_img_stride(%d) < query_img_stride(%d)\n",it->enqueue_img_stride, rstStrideByte.stride_byte[0]);
            return 1;
        }

        // Dynamic Change Pak/Unpak Fmt
        if(MTRUE == dynamicPakFmtMapping(this->m_Imgo.dma_cfg.lIspColorfmt, (MUINT32*)&HwImgoFmt)){
            this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(MFALSE == this->m_RawCtrl.dynamicPak(HwImgoFmt)){
                ISP_PATH_ERR("dynamicPak error at burstidx:0x%x\n",i);
                ret = 1;
            }
        }

        //new IMGO FMT's stride for Dynamic UFO/Pak/UnPak
        this->m_Imgo.dma_cfg.size.stride = rstStrideByte.stride_byte[0];
        this->m_Imgo.dma_cfg.pixel_byte = it->pixel_byte;

        ISP_PATH_DBG("BurstIdx:0x%x,imago:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,imgo)stride_byte:(%d,%d,%d,%d),rawtype:0x%x,dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x),pixel_byte(%d)\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Imgo.dma_cfg.size.stride,it->rawType,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN.Bits.DBN_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN.Bits.BIN_EN,
            it->img_fmt_configpipe, this->m_Imgo.dma_cfg.lIspColorfmt, this->m_Imgo.dma_cfg.pixel_byte);

        //After Config, Enque New Img Fmt to Header
        fmt_sel.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], CAM_CTL_FMT_SEL);
        fh_imgo.Header_Enque(Header_IMGO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.IMGO_FMT);
        fh_ufeo.Header_Enque(Header_UFEO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.IMGO_FMT);

        this->m_Imgo.dma_cfg.crop.x = it->crop.x;
        this->m_Imgo.dma_cfg.crop.y = it->crop.y;
        this->m_Imgo.dma_cfg.crop.w = it->crop.w;
        this->m_Imgo.dma_cfg.crop.h = it->crop.h;
        this->m_Imgo.dma_cfg.size.xsize = rst.xsize_byte[0];
        this->m_Imgo.dma_cfg.size.h = it->crop.h;

        this->m_Imgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Imgo.config()!=0){
            ISP_PATH_ERR("imgo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (bWithUfo) {
             this->m_Ufeo.dma_cfg.size.w = this->m_Imgo.dma_cfg.crop.w;
             this->m_Ufeo.dma_cfg.size.h = this->m_Imgo.dma_cfg.size.h;
             this->m_Ufeo.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_Ufeo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Ufeo.config() != 0) {
                 ISP_PATH_ERR("ufeo config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

    }

    return ret;
}

MINT32 CamPathPass1::setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L)
{
    MINT32  ret = 0, i;
    MUINT32 targetIdx = 0;
    MBOOL   bWithUfo = MFALSE;
    capibility CamInfo;
    tCAM_rst rst;
    list<IspRrzCfg>::iterator it;

    if (pRrz_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("rrz data length mismatch:0x%x_0x%x\n", pRrz_cfg_L->size(), this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }
    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    for (i = 0, it = pRrz_cfg_L->begin(); it != pRrz_cfg_L->end(); i++, it++) {

        this->m_Rrzo.dma_cfg.lIspColorfmt = it->img_fmt;
        bWithUfo = isUfo(this->m_Rrzo.dma_cfg.lIspColorfmt);

        ISP_PATH_DBG("BurstIdx:0x%x,rrz out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x\n",
            i, it->rrz_out_size.w, it->rrz_out_size.h, it->rrz_in_roi.x, it->rrz_in_roi.y,
            it->rrz_in_roi.w, it->rrz_in_roi.h, this->m_Rrzo.dma_cfg.lIspColorfmt);

        //rrzo use no dmao cropping function, cropping function is depended on rrz
        this->m_Rrzo.dma_cfg.crop.x = 0;
        this->m_Rrzo.dma_cfg.crop.y = 0;
        this->m_Rrzo.dma_cfg.crop.floatX = 0;
        this->m_Rrzo.dma_cfg.crop.floatY = 0;
        this->m_Rrzo.dma_cfg.crop.w = it->rrz_in_roi.w;
        this->m_Rrzo.dma_cfg.crop.h = it->rrz_in_roi.h;
        this->m_Rrzo.dma_cfg.size.w = it->rrz_out_size.w;
        this->m_Rrzo.dma_cfg.size.h = it->rrz_out_size.h;

        //mark, use original stride which is configured at init phase
        //DMARrzo.dma_cfg.size.stride = rrz_cfg.rrz_out_size.stride;
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if (CamInfo.GetCapibility(EPortIndex_RRZO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Rrzo.dma_cfg.lIspColorfmt,
                    this->m_Rrzo.dma_cfg.size.w, (E_CamPixelMode)this->m_RawCtrl.getCurPixMode(_rrzo_)),
                rst, E_CAM_UNKNOWNN) == MFALSE)
        {
            ISP_PATH_ERR("rrzo size err(0x%x)\n",this->m_Rrzo.dma_cfg.size.w);
            ret = 1;
        }
        this->m_Rrzo.dma_cfg.size.xsize      =  rst.xsize_byte[0];

        //update xsize cfg
        this->m_Rrzo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Rrzo.config()!=0){
            ISP_PATH_ERR("rrzo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        this->m_Rrz.rrz_in_roi      = it->rrz_in_roi;
        this->m_Rrz.rrz_out_size    = it->rrz_out_size;

        //update rrz cfg
        this->m_Rrz.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Rrz.config() != 0){
            ISP_PATH_ERR("rrz config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (bWithUfo) {
             this->m_Ufgo.dma_cfg.size.w = it->rrz_out_size.w;
             this->m_Ufgo.dma_cfg.size.h = it->rrz_out_size.h;
             this->m_Ufgo.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_Ufgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Ufgo.config() != 0) {
                 ISP_PATH_ERR("ufeo config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

    }

    return ret;
}

MINT32 CamPathPass1::setP1TuneCfg(list<IspP1TuningCfg>* pTune_L)
{
    MINT32 ret = 0;
    MUINT32 targetIdx = 0;
    list<IspP1TuningCfg>::iterator it;
    int i;

    if(pTune_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()){
        ISP_PATH_ERR("tuning data length mismatch:0x%x_0x%x\n",pTune_L->size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    if(this->m_TuningFlg == MFALSE){
        //ref line:344
        this->m_Tuning.m_SenDev = pTune_L->begin()->SenDev;
        this->m_Tuning._config();
        this->m_Tuning._enable(NULL);
        if(NULL != this->m_Tuning.m_Magic){
            //free here , not at stop is because of user can uninit before stop.
            free(this->m_Tuning.m_Magic);
        }
        this->m_Tuning.m_Magic = (MUINT32*)malloc(sizeof(MINT32) * this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        this->m_TuningFlg = MTRUE;
    }

    for(i=0,it=pTune_L->begin();it!=pTune_L->end();i++,it++){
        this->m_Tuning.m_Magic[i] = it->magic;
    }

    for(i=0,it=pTune_L->begin();it!=pTune_L->end();i++,it++)
    {
        ISP_PATH_DBG("BurstIdx:0x%x,magic:0x%x\n",i,it->magic);

        //update magic
        this->m_Magic.m_nMagic = it->magic;

        this->m_Magic.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Magic._config()!=0){
            ISP_PATH_ERR("maic config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        //update tuning
        //remove to above for loop for working with tuningMgr.
        //this->m_Tuning.m_Magic[i] = it->magic;
        this->m_Tuning.m_SenDev = it->SenDev;

        this->m_Tuning.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Tuning.update(MTRUE) != 0){
            ISP_PATH_ERR("Tuning error\n");
            ret = 1;
        }
    }

    if(this->m_Tuning.update_end() != 0){
        ISP_PATH_ERR("Tuning enque error\n");
        ret = 1;
    }


    return ret;
}

MBOOL CamPathPass1::P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj)
{
    MBOOL ret = MTRUE;
    MUINT32 tbl_len = 0;
    CAM_BUF_CTRL *pFbc = NULL;

    while (tbl_len < p1Notify_node) {
        if (this->m_p1NotifyTbl[tbl_len] == cmd) {
            break;
        }
        tbl_len++;
    }

    //if notify module is exist in drv,
    //enabled FBC , modules which belong to FBC is enabled in UNI_MGR
    if(tbl_len < p1Notify_node){
        switch(cmd){
            case EPIPECmd_SET_LCS_CBFP:
                pFbc = &m_Lcso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_EIS_CBFP:
                pFbc = &m_Eiso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_SGG2_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RSS_CBFP:
                pFbc = &m_Rsso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RRZ_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_TUNING_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_REGDUMP_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_AWB_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            default:
                ret = MFALSE;
                ISP_PATH_ERR("un-supported p1notify_0x%x\n",cmd);
                break;
        }
    }

    if (ret) {
        if (pFbc) {
            for(MUINT32 i = 0; i < this->m_pCmdQdrv->CmdQMgr_GetDuqQ(); i++) {
                for(MUINT32 j = 0; j < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); j++) {
                    pFbc->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                    pFbc->config();
                    pFbc->write2CQ();
                    pFbc->enable(NULL);
                }
            }
        }
    }

    return ret;
}

MINT32 CamPathPass1::setP1Notify(void)
{
    MUINT32 node = 0;
    MINT32 ret = 0;
    MUINT32 targetIdx = 0;

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    switch(this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.TG_FMT){
    case TG_FMT_RAW8:
    case TG_FMT_RAW10:
    case TG_FMT_RAW12:
    case TG_FMT_RAW14:
    while (node < p1Notify_node) {
        if (this->m_p1NotifyObj[node] != NULL) {

            this->m_RawCtrl.m_pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[node];
            ISP_PATH_DBG("P1Notify: %s\n",this->m_RawCtrl.m_pP1Tuning->TuningName());

            for (MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++) {

                this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

                switch(node){
                case _RRZ_:
                    if (this->m_RawCtrl.setRRZ() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _LMV_:
                    if (this->m_RawCtrl.setLMV() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _RSS_:
                    if (this->m_RawCtrl.setRSS() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _LCS_:
                    if (this->m_RawCtrl.setLCS() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _SGG2_:
                    if (this->m_RawCtrl.setSGG2() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _TUNING_:
                    if (this->m_RawCtrl.setTuning() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _REG_DUMP_:
                    if (this->m_RawCtrl.dumpCQReg() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _AWB_:
                    if (this->m_RawCtrl.setAWB() == MFALSE) {
                        ret = 1;
                    }
                    break;
                default:
                    ISP_PATH_INF("node:0x%x is not supported in drv\n",node);
                    ret = 1;
                    break;
                }

                if (ret != 0) {
                    ISP_PATH_ERR("P1Notify_%s fail! \n",this->m_RawCtrl.m_pP1Tuning->TuningName());
                }
            }
        }
        node++;
    }

    this->m_RawCtrl.setNotifyDone();

    break;

    case TG_FMT_YUV422:
        while(node < p1Notify_node){
            if(this->m_p1NotifyObj[node]!= NULL){

                this->m_YuvCtrl.m_pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[node];
                ISP_PATH_INF("P1Notify: %s\n",this->m_YuvCtrl.m_pP1Tuning->TuningName());
                for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){

                    this->m_YuvCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                    switch(node){
                    case _LMV_:
                        if(this->m_YuvCtrl.setLMV() == MFALSE)
                            ret = 1;
                        break;
                    case _RSS_:
                        if(this->m_YuvCtrl.setRSS() == MFALSE)
                            ret = 1;
                        break;
                    case _LCS_:
                        if(this->m_YuvCtrl.setLCS() == MFALSE)
                            ret = 1;
                        break;
                    case _SGG2_:
                        if(this->m_YuvCtrl.setSGG2() == MFALSE)
                            ret = 1;
                        break;
                    case _RRZ_:
                    case _TUNING_:
                    case _REG_DUMP_:
                    case _AWB_:
                    default:
                        ISP_PATH_INF("node:0x%x is not supported in drv\n",node);
                        ret =1;
                        break;
                    }
                    if(ret != 0)
                        ISP_PATH_ERR("P1Notify_%s fail! \n",this->m_YuvCtrl.m_pP1Tuning->TuningName());
                }
            }
            node++;
        }

        break;
    default:
        ISP_PATH_ERR("unsupported TG format:0x%x\n",this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.TG_FMT);
        return 1;
        break;
    }


    return ret;
}


E_CAMPATH_STAT  CamPathPass1::getCamPathState(void)
{
    switch(this->m_FSM) {
    case op_cfg:
        return camstat_config;
    case op_start:
        return camstat_start;
    case op_stop:
        return camstat_stop;
    case op_suspend:
        return camstat_suspend;
    case op_unknown:
    default:
        return camstat_unknown;
    }
}


/* return value:
   0: sucessed
else: fail */
MINT32 CamPathPass1::enqueueBuf(MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo)
{
    CAM_BUF_CTRL        *pFBC = NULL;
    DMAO_B              *pDmao = NULL;
    MUINT32 targetIdx = 0;
    MINT32  ret = 0;

    switch(dmaChannel){
    case _imgo_:
    case _rrzo_:
        if(this->m_PBC.enque_push(dmaChannel,bufInfo,NULL) == MFALSE)
            return 1;
        else
            return 0;
        break;
    case _lcso_:
        pFBC = &this->m_Lcso_FBC;
        pDmao = &this->m_Lcso;
        break;
    case _lmvo_:
        pFBC = &this->m_Eiso_FBC;
        pDmao = &this->m_Eiso;
        break;
    case _rsso_:
        pFBC = &this->m_Rsso_FBC;
        pDmao = &this->m_Rsso;
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:0x%x\n",dmaChannel);
        return 1;
    }

    if(bufInfo.size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()){
        ISP_PATH_ERR("enque data length mismatch:0x%x_0x%x\n",bufInfo.size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();


    this->m_lock.lock();
    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        IspDrvVir *_pCurDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        pFBC->m_pIspDrv = _pCurDrv;

        if(eCmd_Fail == pFBC->enqueueHwBuf(bufInfo.front())){
            ISP_PATH_ERR("enque fail at burst:0x%x\n",i);
            ret = 1;
        }

        if(bufInfo.front().bReplace == MTRUE){
            pDmao->dma_cfg.memBuf.base_pAddr = bufInfo.front().Replace.image.mem_info.pa_addr;
            pDmao->Header_Addr = bufInfo.front().Replace.header.pa_addr;
        }
        else{
            pDmao->dma_cfg.memBuf.base_pAddr = bufInfo.front().u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
            pDmao->Header_Addr = bufInfo.front().u_op.enque.at(ePlane_1st).header.pa_addr;
        }
        pDmao->m_pIspDrv = _pCurDrv;
        if(pDmao->setBaseAddr()){
            ISP_PATH_ERR("set baseaddress at burst:0x%x\n",i);
            ret = 1;
        }

        bufInfo.push_back(bufInfo.front());
        bufInfo.pop_front();

    }
    this->m_lock.unlock();
    return ret;

}


/* return value:
 1: already stopped
 0: sucessed
-1: fail */
MINT32 CamPathPass1::dequeueBuf(MUINT32 const dmaChannel, vector<BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify)
{
    CAM_BUF_CTRL* pFBC = NULL, *pFBCUfo = NULL;
    BufInfo buf;
    MINT32  ret = 0, deqRet = 0;
    MUINT32 _size = 0;

    switch(dmaChannel){
    case _imgo_:
    case _rrzo_:
        return this->m_PBC.deque(dmaChannel,bufInfo,pNotify);
        break;
    case _lcso_:
        pFBC = &this->m_Lcso_FBC;
        break;
    case _lmvo_:
        pFBC = &this->m_Eiso_FBC;
        break;
    case _rsso_:
        pFBC = &this->m_Rsso_FBC;
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:0x%x\n",dmaChannel);
        return -1;
        break;
    }

    if(bufInfo.size() != 0){
        ISP_PATH_ERR("list is not empty\n");
        return -1;
    }

    //can't update this para. , in order to prevent racing condition
    //pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[0];
    //check if there is already filled buffer
    switch(pFBC->waitBufReady(pNotify)){
        case eCmd_Fail:
            return -1;
            break;
        case eCmd_Stop_Pass:
            ISP_PATH_WRN("waitBufRdy: VF_EN=0, dma(0x%x)\n", dmaChannel);
            return 1;
            break;
        case eCmd_Suspending_Pass:
            break;
        default:
            break;
    }

    this->m_lock.lock();
    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        //can't update this para. , in order to prevent racing condition
        //pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[i];
        switch(pFBC->dequeueHwBuf(buf)) {
        case eCmd_Fail:
            ISP_PATH_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
            ret = -1;
            break;
        case eCmd_Stop_Pass:
            ISP_PATH_WRN("current status:eCmd_Stop_Pass\n");
            ret = 1;
            break;
        case eCmd_Suspending_Pass:
            ISP_PATH_WRN("current status:eCmd_Suspending_Pass\n");
            ret = 2;
            break;
        default:
            break;
        }

        bufInfo.push_back(buf);
    }
    this->m_lock.unlock();

    return ret;
}


MBOOL CamPathPass1::suspend(E_CAMPATH_SUSPEND_OP eOP) // cannno_ep
{
    MUINT32 _step = 0;

    ISP_PATH_DBG("enter suspending mode:\n");

    switch (eOP) {
    case _CAMPATH_SUSPEND_STOPHW:
        if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                    CAM_BUF_CTRL::eSus_HWOFF) == MFALSE) {
            _step = 1;
        }
        if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                    CAM_BUF_CTRL::eSus_HW_SW_STATE) == MFALSE) {
            _step = 2;
        }

        this->m_pCmdQdrv->CmdQMgr_suspend();

        this->m_FSM = op_suspend;
        break;
    case _CAMPATH_SUSPEND_FLUSH:
        if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                    CAM_BUF_CTRL::eSus_SIGNAL) == MFALSE) {
            _step = 3;
        }
        break;
    default:
        ISP_PATH_ERR("Error suspend state: %d\n", (MUINT32)eOP);
        return MFALSE;
    }

    if (_step) {
        ISP_PATH_ERR("Error suspending step:%d, op:%d\n", _step, (MUINT32)eOP);
        return MFALSE;
    }


    return MTRUE;
}

MBOOL CamPathPass1::resume(void) // cannon_ep
{
    MBOOL ret;

    ISP_PATH_INF("exit suspending mode");

    this->m_pCmdQdrv->CmdQMgr_resume();

    ret = CAM_BUF_CTRL::resume(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0]);

    this->m_FSM = op_start;

    return ret;
}

MBOOL CamPathPass1::recoverSingle(E_CAMPATH_STEP op)
{
    ISP_PATH_DBG("enter recoverSingle:%d\n",op);

    switch (op) {
    case eCmd_path_stop:
        return this->m_TopCtrl.HW_recover(0);
    case eCmd_path_restart:
        return this->m_TopCtrl.HW_recover(1);
    default:
        ISP_PATH_ERR("unsupported:%d",op);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL CamPathPass1::updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset)
{
    return CAM_BUF_CTRL::updateFrameTime(timeInMs, reqOffset, this->m_hwModule);
}

MBOOL CamPathPass1::updatePath(UpdatePathParam pathParm)
{
    MBOOL ret = MTRUE;
    MUINT32 targetIdx = 0;

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    memcpy((void *) &this->m_RawCtrl.m_xmxo, (void *)pathParm.pXmx, sizeof(T_XMXO));
    this->m_RawCtrl.m_bBin = pathParm.bBin;

    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        this->m_RawCtrl.config();
    }

    return ret;
}

MBOOL CamPathPass1::dynamicPakFmtMapping(ImgInfo::EImgFmt_t imgFmt, MUINT32* pHwImgoFmt)
{
    MBOOL ret = MTRUE;

    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pHwImgoFmt = IMGO_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pHwImgoFmt = IMGO_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pHwImgoFmt = IMGO_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pHwImgoFmt = IMGO_FMT_RAW14;
            break;
        case eImgFmt_BAYER8_UNPAK:
            *pHwImgoFmt = IMGO_FMT_RAW8_2B;
            break;
        case eImgFmt_BAYER10_UNPAK:
            *pHwImgoFmt = IMGO_FMT_RAW10_2B;
            break;
        case eImgFmt_BAYER12_UNPAK:
            *pHwImgoFmt = IMGO_FMT_RAW12_2B;
            break;
        case eImgFmt_BAYER14_UNPAK:
            *pHwImgoFmt = IMGO_FMT_RAW14_2B;
            break;
        default:
            ISP_PATH_DBG("eImgFmt:[0x%x]NOT Support PakUnPakFmtMapping",imgFmt);
            ret = MFALSE;
    }
    //
    ISP_PATH_DBG("input imgFmt(0x%x),output HwImgoFmtValue:0x%x",imgFmt, *pHwImgoFmt);
    return ret;
}

Cam_path_sel::Cam_path_sel(CamPathPass1Parameter* pObj)
{
    m_this = pObj;
    m_hwModule = CAM_MAX;
}

Cam_path_sel::~Cam_path_sel()
{}

MBOOL Cam_path_sel::Path_sel_YUV(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern)
{   // YUV
    MBOOL rst = MTRUE;

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_0;

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= SGG_SEL_0;

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= LCS_SEL_0;

    //IMG_SEL + UFE_SEL
    this->m_this->m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_1;
    this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_2;

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= FLK1_SEL_1;


    //PMX & RCP3 is useless under YUV format

    return rst;
}


MBOOL Cam_path_sel::Path_sel_RAW(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat)
{    //bayer
    MBOOL rst = MTRUE;

    if(this->m_this->bypass_rrzo)
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_0;
    else
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_1;

    if(this->m_this->bypass_ufeo){
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFEG_SEL_0;
    }
    else{
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFEG_SEL_1;
    }
    this->m_this->m_top_ctl.CTRL_SEL.Raw |= SGG_SEL_1;

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= LCS_SEL_1;

    //IMG_SEL + UFE_SEL + PMX + rcp3
    if(bPureRaw){
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_2;

        if(bPak)
            this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_0;
        else
            this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_2;

        //rcp3 is userless at pure data when data
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= RCP3_SEL_2;

        //PMX+rcp3
        switch(data_pat){
            case eCAM_DUAL_PIX:
                this->m_this->m_top_ctl.CTRL_SEL.Raw |= PDO_SEL_1;
                //rcp3 is fixed at 2 when pure + dual pattern
                 this->m_this->m_top_ctl.CTRL_SEL.Raw |= RCP3_SEL_2;
                break;
            case eCAM_NORMAL:
            default:
                this->m_this->m_top_ctl.CTRL_SEL.Raw |= PDO_SEL_0;
                //rcp3 is useless under normal pattern at pure data
                break;
        }
    }
    else{
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_2;
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_0;

        //PMX+rcp3
        switch(data_pat){
            case eCAM_DUAL_PIX:
                this->m_this->m_top_ctl.CTRL_SEL.Raw |= PDO_SEL_1;
                break;
            case eCAM_NORMAL:
            default:
                this->m_this->m_top_ctl.CTRL_SEL.Raw |= PDO_SEL_0;
                break;
        }
        //rcp3 is fixed , 0/1 is not supported yet!
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= RCP3_SEL_3;
    }

    this->m_this->m_top_ctl.CTRL_SEL.Raw |= FLK1_SEL_0;

    return rst;
}




MBOOL Cam_path_sel::Path_sel(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat)
{
    MBOOL rst = MTRUE;
    this->m_this->m_top_ctl.CTRL_SEL.Raw = 0x0;

    if(bRawI){  //DMAI

        //DMX_SEL
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= DMX_SEL_2;
        ISP_PATH_ERR("RAW IN is not supported yet!\n");
        rst = MFALSE;
    }
    else{   // TG IN.

        //DMX_SEL
        this->m_this->m_top_ctl.CTRL_SEL.Raw |= DMX_SEL_0;
    }

    if(this->m_this->bypass_ispRawPipe == MFALSE){    //bayer

        rst = this->Path_sel_RAW(bPureRaw,bPak,bRawI,data_pat);

    }
    else{   // YUV

        rst = this->Path_sel_YUV(bPureRaw,bPak,bRawI,data_pat);
    }

    return rst;
}


CamPath_BufCtrl::CamPath_BufCtrl()
{
    m_pCamPathImp = NULL;
    m_FSM = op_unknown;
    m_hwModule = CAM_MAX;
}

MBOOL CamPath_BufCtrl::PBC_config(void* pPathMgr, vector<DupCmdQMgr*>* pSlave,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput)
{
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL*   pFBC = NULL, *pFBCUfo = NULL;
    DupCmdQMgr*     pCmdQ = NULL;
    CAM_TIMESTAMP*  pTime;
    CAM_BUF_CTRL fbc;
    pSlave;
    pInput;


    if(this->FSM_UPDATE(op_cfg) == MFALSE)
        return MFALSE;

    this->m_pCamPathImp = (CamPathPass1*)pPathMgr;
    this->m_hwModule = this->m_pCamPathImp->m_hwModule;
    pCmdQ = this->m_pCamPathImp->m_pCmdQdrv;
    this->m_OpenedChannel.clear();
    this->m_enque_IMGO.init();
    this->m_enque_RRZO.init();
    this->m_enqRecImgo.init();
    this->m_enqRecRrzo.init();
    this->m_bUF_imgo.clear();
    this->m_bUF_rrzo.clear();

    //initialize timestamp
    pTime = CAM_TIMESTAMP::getInstance(pCmdQ->CmdQMgr_GetCurModule(),pCmdQ->CmdQMgr_GetCurCycleObj(0)[0]);
    pTime->TimeStamp_SrcClk(pInput->tTimeClk);

    //initialize fbc's timeout method
    fbc.m_fps[this->m_hwModule] = pInput->tgFps;
    for (MUINT32 iii = 0; iii < MAX_RECENT_GRPFRM_TIME; iii++) {
        fbc.m_recentFrmTimeMs[this->m_hwModule][iii] = MIN_GRPFRM_TIME_MS;
    }

    for (it = pchannel->begin(); it != pchannel->end(); it++) {
        switch (*it) {
        case _imgo_:
            pFBC = &this->m_Imgo_FBC;
            pFBCUfo = &this->m_Ufeo_FBC;
            break;
        case _rrzo_:
            pFBC = &this->m_Rrzo_FBC;
            pFBCUfo = &this->m_Ufgo_FBC;
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n",*it);
            return MFALSE;
            break;
        }

        pFBC->m_pTimeStamp = pTime;
        pFBCUfo->m_pTimeStamp = pTime;

        for(MUINT32 i = 0;i<pCmdQ->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<pCmdQ->CmdQMgr_GetBurstQ();j++){
                IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];
                //
                pFBC->m_pIspDrv = _ispDrv;
                pFBC->config();
                pFBC->write2CQ();
                pFBC->enable(NULL);

                //
                pFBCUfo->m_pIspDrv = _ispDrv;
                pFBCUfo->config();
                pFBCUfo->write2CQ();

                //dma's config/enable/write2cq is via campathpass1
            }
        }


        this->m_OpenedChannel.push_back(*it);
    }

    return MTRUE;
}

MBOOL CamPath_BufCtrl::PBC_Start(MVOID)
{
    return this->FSM_UPDATE(op_startTwin);
}


MBOOL CamPath_BufCtrl::PBC_Stop(MVOID)
{
    list<MUINT32>::iterator it;
    CAM_BUF_CTRL* pFBC = NULL, *pFBCUfo = NULL;
    DupCmdQMgr* pCmdQ = this->m_pCamPathImp->m_pCmdQdrv;

    if(this->FSM_UPDATE(op_unknown) == MFALSE)
        return MFALSE;


    for (it = this->m_OpenedChannel.begin(); it!=this->m_OpenedChannel.end(); it++) {
        switch(*it){
            case _imgo_:
                pFBC = &this->m_Imgo_FBC;
                pFBCUfo = &this->m_Ufeo_FBC;
                break;
            case _rrzo_:
                pFBC = &this->m_Rrzo_FBC;
                pFBCUfo = &this->m_Ufgo_FBC;
                break;
            default:
                ISP_PATH_ERR("unsupported dmao:%d\n",*it);
                return MFALSE;
                break;
        }

        for (MUINT32 i = 0; i < pCmdQ->CmdQMgr_GetDuqQ(); i++) {
            for (MUINT32 j = 0; j < pCmdQ->CmdQMgr_GetBurstQ(); j++) {
                IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];

                pFBC->m_pIspDrv = _ispDrv;
                pFBC->disable();

                pFBCUfo->m_pIspDrv = _ispDrv;
                pFBCUfo->disable();
            }
        }
    }


    return MTRUE;
}


MBOOL CamPath_BufCtrl::enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, vector<ISP_HW_MODULE>* pTwinVMod)
{
    MBOOL                       ret = MTRUE;
    MUINT32                     targetIdx = 0;
    QueueMgr<ISP_BUF_INFO_L>    *pQue = NULL;
    CAM_BUF_CTRL                *pFBCUfo = NULL;
    DMAO_B                      *pDmaoUfo = NULL;
    DupCmdQMgr                  *pCmdQ = this->m_pCamPathImp->m_pCmdQdrv;
    pTwinVMod;

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        return MFALSE;
    }

    switch (dmaChannel) {
        case _imgo_:
            pQue = &this->m_enque_IMGO;
            break;
        case _rrzo_:
            pQue = &this->m_enque_RRZO;
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n", dmaChannel);
            ret = MFALSE;
            goto EXIT;
    }

    if(bufInfo.size() != pCmdQ->CmdQMgr_GetBurstQ()){
        ISP_PATH_ERR("enque data length mismatch:0x%x_0x%x\n",bufInfo.size(),pCmdQ->CmdQMgr_GetBurstQ());
        return 1;
    }

    pQue->sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_push,(MUINTPTR)&bufInfo,0);

    //behavior of enque into hw will be invoked after p1update
    switch (dmaChannel) {
        case _imgo_:
            pFBCUfo = &this->m_Ufeo_FBC ;
            pDmaoUfo = &this->m_pCamPathImp->m_Ufeo ;
            this->m_bUF_imgo.clear();
            break;
        case _rrzo_:
            pFBCUfo = &this->m_Ufgo_FBC ;
            pDmaoUfo = &this->m_pCamPathImp->m_Ufgo ;
            this->m_bUF_rrzo.clear();
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n",dmaChannel);
            return MFALSE;
    }

    targetIdx = pCmdQ->CmdQMgr_GetDuqQIdx();

    for (MUINT32 j = 0; j < pCmdQ->CmdQMgr_GetBurstQ(); j++) {
        IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(targetIdx)[j];
        //under single path, imgo/rrzo dynamic on/off r not supported
        pFBCUfo->m_pIspDrv = _ispDrv;
        pDmaoUfo->m_pIspDrv = _ispDrv;
        if (bufInfo.front().u_op.enque.size() == 3) {
            pFBCUfo->enable(NULL);
            pDmaoUfo->enable(NULL);
        }
        else {
            pFBCUfo->disable();
            pDmaoUfo->disable();
        }

    }
EXIT:
    return ret;
}

MINT32 CamPath_BufCtrl::enque_pop(MVOID)
{
    MINT32 ret = 0;
    MUINT32 _size = 0;
    list<MUINT32>::iterator it;

    for(it=this->m_OpenedChannel.begin();it!=this->m_OpenedChannel.end();it++){
        switch(*it){
            case _imgo_:
                this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_imgo_);
                else{
                    ret = 1;
                    ISP_PATH_ERR("IMGO have no buffer for enque\n");
                }
                break;
            case _rrzo_:
                this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_rrzo_);
                else{
                    ret = 1;
                    ISP_PATH_ERR("RRZO have no buffer for enque\n");
                }
                break;
            default:
                ISP_PATH_ERR("unsupported DMA:%d\n",*it);
                break;
        }
    }

    return ret;
}

MBOOL CamPath_BufCtrl::enque_UF_patch(vector<T_UF>* pUF)
{
    list<MUINT32>::iterator it;
    MBOOL bmatch = MFALSE;;
    vector<MBOOL>* pVector;

    if(pUF == NULL){
        ISP_PATH_ERR("can't be NULL");
        return MFALSE;
    }
    for (it = this->m_OpenedChannel.begin(); it!=this->m_OpenedChannel.end(); it++) {
        MUINT32 i;
        switch(*it){
            case _imgo_:
                pVector = &this->m_bUF_imgo;
                break;
            case _rrzo_:
                pVector = &this->m_bUF_rrzo;
                break;
            default:
                bmatch = MFALSE;
                ISP_PATH_ERR("unsupported port:%d\n",*it);
                return MFALSE;
                break;
        }

        for(i=0;i<pUF->size();i++){
            if(pUF->at(i).dmachannel == *it){
                bmatch = MTRUE;
                break;
            }
        }
        if(bmatch)
            pVector->push_back(pUF->at(i).bOFF);
        else{
            ISP_PATH_ERR("logic err,find no match case\n");
        }
    }

    return bmatch;
}

MINT32 CamPath_BufCtrl::deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify)
{
    CAM_BUF_CTRL* pFBC = NULL, *pFBCUfo = NULL;
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    BufInfo buf;
    MINT32  ret = 0, deqRet = 0;
    MUINT32 _size = 0;


    switch(dmaChannel){
    case _imgo_:
        pFBC = &this->m_Imgo_FBC;
        pFBCUfo = &this->m_Ufeo_FBC;
        pEnqBufMgr = &this->m_enqRecImgo;
        break;
    case _rrzo_:
        pFBC = &this->m_Rrzo_FBC;
        pFBCUfo = &this->m_Ufgo_FBC;
        pEnqBufMgr = &this->m_enqRecRrzo;
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:0x%x\n",dmaChannel);
        return -1;
        break;
    }

    if(bufInfo.size() != 0){
        ISP_PATH_ERR("list is not empty\n");
        return -1;
    }

    //can't update this para. , in order to prevent racing condition
    //pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[0];
    //check if there is already filled buffer
    switch(pFBC->waitBufReady(pNotify)){
        case eCmd_Fail:
            return -1;
            break;
        case eCmd_Stop_Pass:
            ISP_PATH_WRN("waitBufRdy: VF_EN=0, dma(0x%x)\n", dmaChannel);
            return 1;
            break;
        case eCmd_Suspending_Pass:
            break;
        default:
            break;
    }

    this->m_lock.lock();
    for(MUINT32 i=0;i<this->m_pCamPathImp->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        stISP_BUF_INFO  bufRec;
        UFDG_META_INFO  *pUfMeta = NULL;
        //can't update this para. , in order to prevent racing condition
        //pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[i];
        switch(pFBC->dequeueHwBuf(buf)) {
        case eCmd_Fail:
            ISP_PATH_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
            ret = -1;
            break;
        case eCmd_Stop_Pass:
            ISP_PATH_WRN("current status:eCmd_Stop_Pass\n");
            ret = 1;
            break;
        case eCmd_Suspending_Pass:
            ISP_PATH_WRN("current status:eCmd_Suspending_Pass\n");
            ret = 2;
            break;
        default:
            break;
        }


        pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_getsize, (MUINTPTR)&_size, 0);
        if (_size == 0) {
            ISP_PATH_ERR("dma[0x%x] buf queue depth cannot be 0\n", dmaChannel);
            buf.m_pPrivate = NULL;
            ret = 1;
            break;
        }
        else {
            pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&bufRec);
            pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_pop, 0, 0);

            if (bufRec.u_op.enque.size() >= ePlane_3rd) {
                pUfMeta = (UFDG_META_INFO *)(bufRec.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);
            }
        }
        buf.m_pPrivate = (MUINT32 *)pUfMeta;

        if (buf.bUF_DataFmt) {
            BufInfo bufUfo;

            bufUfo.m_pPrivate = (MUINT32 *)pUfMeta;

            switch(pFBCUfo->dequeueHwBuf(bufUfo)) {
            case eCmd_Fail:
                ISP_PATH_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
                ret = -1;
                break;
            case eCmd_Stop_Pass:
                ISP_PATH_WRN("current status:eCmd_Stop_Pass\n");
                ret = 1;
                break;
            case eCmd_Suspending_Pass:
                ISP_PATH_WRN("current status:eCmd_Suspending_Pass\n");
                ret = 2;
                break;
            default:
                break;
            }

            pUfMeta->bUF = MTRUE;
            pUfMeta->UFDG_BOND_MODE = 0;// non-twin in this path

            buf.u4BufSize[ePlane_2nd]   = bufUfo.u4BufSize[ePlane_2nd];
            buf.u4BufVA[ePlane_2nd]     = bufUfo.u4BufVA[ePlane_2nd];
            buf.u4BufPA[ePlane_2nd]     = bufUfo.u4BufPA[ePlane_2nd];
            buf.u4BufOffset[ePlane_2nd] = bufUfo.u4BufOffset[ePlane_2nd];
            buf.memID[ePlane_2nd]       = bufUfo.memID[ePlane_2nd];
            buf.bufSecu[ePlane_2nd]     = bufUfo.bufSecu[ePlane_2nd];
            buf.bufCohe[ePlane_2nd]     = bufUfo.bufCohe[ePlane_2nd];

            buf.Frame_Header.u4BufPA[ePlane_2nd]   = bufUfo.Frame_Header.u4BufSize[ePlane_2nd];
            buf.Frame_Header.u4BufVA[ePlane_2nd]   = bufUfo.Frame_Header.u4BufVA[ePlane_2nd];
            buf.Frame_Header.u4BufSize[ePlane_2nd] = bufUfo.Frame_Header.u4BufSize[ePlane_2nd];
            buf.Frame_Header.memID[ePlane_2nd]     = bufUfo.Frame_Header.memID[ePlane_2nd];
            buf.Frame_Header.bufCohe[ePlane_2nd]   = bufUfo.Frame_Header.bufCohe[ePlane_2nd];
            buf.Frame_Header.bufSecu[ePlane_2nd]   = bufUfo.Frame_Header.bufSecu[ePlane_2nd];
            buf.Frame_Header.u4BufPA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.pa_addr;
            buf.Frame_Header.u4BufVA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.va_addr;
            buf.Frame_Header.u4BufSize[ePlane_3rd] = bufRec.u_op.enque.at(ePlane_3rd).header.size;
            buf.Frame_Header.memID[ePlane_3rd]     = bufRec.u_op.enque.at(ePlane_3rd).header.memID;
            buf.Frame_Header.bufCohe[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufCohe;
            buf.Frame_Header.bufSecu[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufSecu;
        }
        else if (pUfMeta) {
            // enque with ufo but hw constraint

            pUfMeta->bUF = MFALSE;
            pUfMeta->UFDG_BOND_MODE = 0;

            for (MUINT32 plane = 0; plane < ePlane_max; plane++) {
                buf.Frame_Header.u4BufPA[plane]     = bufRec.u_op.enque.at(plane).header.pa_addr;
                buf.Frame_Header.u4BufVA[plane]     = bufRec.u_op.enque.at(plane).header.va_addr;
                buf.Frame_Header.u4BufSize[plane]   = bufRec.u_op.enque.at(plane).header.size;
                buf.Frame_Header.memID[plane]       = bufRec.u_op.enque.at(plane).header.memID;
                buf.Frame_Header.bufCohe[plane]     = bufRec.u_op.enque.at(plane).header.bufCohe;
                buf.Frame_Header.bufSecu[plane]     = bufRec.u_op.enque.at(plane).header.bufSecu;
            }
        }
        else { //always 3 plane of fh ??
        }

        bufInfo.push_back(buf);
    }
    this->m_lock.unlock();

    return ret;
}




MINT32 CamPath_BufCtrl::enqueHW(MUINT32 const dmaChannel)
{
    CAM_BUF_CTRL    *pFBC = NULL, *pFBCUfo = NULL;
    DMAO_B          *pDmao = NULL, *pDmaoUfo = NULL;
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    DupCmdQMgr      *pCQMgr = this->m_pCamPathImp->m_pCmdQdrv;
    ISP_BUF_INFO_L  _buf_list;
    MUINT32         ret = 0, targetIdx = 0;
    vector<MBOOL>   *pUF = NULL;

    Mutex::Autolock __lock(this->m_lock);

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        ret = 1;
        goto EXIT;
    }


    switch (dmaChannel) {
    case _imgo_:
        this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front, (MUINTPTR)&_buf_list, 0);
        break;
    case _rrzo_:
        this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front, (MUINTPTR)&_buf_list, 0);
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:%d\n", dmaChannel);
        ret = 1;
        goto EXIT;
        break;
    }

    for (MUINT32 i=0;i<pCQMgr->CmdQMgr_GetBurstQ();i++) {//for subsample
        IspDrvVir       *_pCurPage = NULL;
        stISP_BUF_INFO  _tmp_buf = _buf_list.front();

        targetIdx = pCQMgr->CmdQMgr_GetDuqQIdx();

        _pCurPage = (IspDrvVir*)pCQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        switch(dmaChannel){
        case _imgo_:
            pFBC = &this->m_Imgo_FBC;
            pDmao = &this->m_pCamPathImp->m_Imgo;

            pFBCUfo = &this->m_Ufeo_FBC;
            pDmaoUfo = &this->m_pCamPathImp->m_Ufeo;

            pEnqBufMgr = &this->m_enqRecImgo;

            pUF = &this->m_bUF_imgo;
            break;
        case _rrzo_:
            pFBC = &this->m_Rrzo_FBC;
            pDmao = &this->m_pCamPathImp->m_Rrzo;

            pFBCUfo = &this->m_Ufgo_FBC;
            pDmaoUfo = &this->m_pCamPathImp->m_Ufgo;

            pEnqBufMgr = &this->m_enqRecRrzo;

            pUF = &this->m_bUF_rrzo;
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n",dmaChannel);
            ret = 1;
            goto EXIT;
        }


        pFBC->m_pIspDrv = _pCurPage;
        if(eCmd_Fail == pFBC->enqueueHwBuf(_tmp_buf, MTRUE)){
            ISP_PATH_ERR("enque fail at burst:0x%x\n",i);
            ret = 1;
        }

        if(_tmp_buf.bReplace == MTRUE){
            pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.Replace.image.mem_info.pa_addr;

            pDmao->Header_Addr = _tmp_buf.Replace.header.pa_addr;
        }
        else {
            pDmao->dma_cfg.memBuf.base_pAddr = _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;

            pDmao->Header_Addr = _tmp_buf.u_op.enque.at(ePlane_1st).header.pa_addr;

            if(SecMgr::SecMgr_GetSecurePortStatus(this->m_hwModule,dmaChannel)){
                SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                MUINT32 SecPA = 0;
                SecMgr_SecInfo secinfo;
                memset(&secinfo, 0, sizeof(SecMgr_SecInfo));
                secinfo.type = SECMEM_FRAME_HEADER;
                secinfo.module = this->m_hwModule;
                secinfo.buff_size = _tmp_buf.u_op.enque.at(ePlane_1st).header.size;
                secinfo.buff_va = _tmp_buf.u_op.enque.at(ePlane_1st).header.va_addr;
                secinfo.port = dmaChannel;
                if(mpSecMgr->SecMgr_QueryFHSecMVA(secinfo,&SecPA)){
                    pDmao->Header_Addr = SecPA;
                    ISP_PATH_INF("Sec FH Addr:0x%x",pDmao->Header_Addr);
                }else{
                    ISP_PATH_ERR("Translate FH PA failed");
                    ret = 1;
                    goto EXIT;
                }
            }
        }

        pDmao->m_pIspDrv = _pCurPage;
        if (pDmao->setBaseAddr()) {
            ISP_PATH_ERR("set baseaddress at burst:0x%x pa:0x%x\n", i, pDmao->dma_cfg.memBuf.base_pAddr);
            ret = 1;
        }

        if (_tmp_buf.u_op.enque.size() == 3) {
            if((pUF == NULL) ||(pUF->size() == 0)){
                ISP_PATH_ERR("UF patch should be NULL(0x%x) or 0",pUF);
                ret = 1;
            }
            else if(pUF->at(i) == MFALSE){
                stISP_BUF_INFO  ufoBuf;
                ST_BUF_INFO stbuf;
                UFDG_META_INFO  *pUfMeta = (UFDG_META_INFO *)(_tmp_buf.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);

                pFBCUfo->m_pIspDrv = _pCurPage;
                pDmaoUfo->m_pIspDrv = _pCurPage;

                stbuf.image = _tmp_buf.u_op.enque.at(ePlane_2nd).image;
                stbuf.header = _tmp_buf.u_op.enque.at(ePlane_2nd).header;
                ufoBuf.u_op.enque.push_back(stbuf);


                pUfMeta = (UFDG_META_INFO *)(_tmp_buf.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);
                if (pUfMeta) {
                    memset(pUfMeta, 0, sizeof(UFDG_META_INFO));
                }
                else {
                    ISP_PATH_ERR("dma[0x%x] bayer_pa=0x%x m_pPrivate buf ptr cannot be null!!!\n",
                        dmaChannel, _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                    ret = 1;
                }

                if(eCmd_Fail == pFBCUfo->enqueueHwBuf(ufoBuf)){
                    ISP_PATH_ERR("dma[0x%x] ufo_pa=0x%x m_pPrivate\n", dmaChannel, ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                    ret = 1;
                }

                if (ufoBuf.bReplace == MTRUE) {
                    pDmaoUfo->dma_cfg.memBuf.base_pAddr = ufoBuf.Replace.image.mem_info.pa_addr;
                    pDmaoUfo->Header_Addr = ufoBuf.Replace.header.pa_addr;
                }
                else {
                    pDmaoUfo->dma_cfg.memBuf.base_pAddr = ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
                    pDmaoUfo->Header_Addr = ufoBuf.u_op.enque.at(ePlane_1st).header.pa_addr;
                }
                if (pDmaoUfo->setBaseAddr()) {
                    ISP_PATH_ERR("dma[0x%x] bayer_pa=0x%x ufo_pa=0x%x burst=%d\n", dmaChannel,
                        _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, ufoBuf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                    ret = 1;
                }
            }
        }


        pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_push, (MUINTPTR)&_tmp_buf, 0);
        _buf_list.pop_front();
    }


    switch (dmaChannel) {
    case _imgo_:
        this->m_enque_IMGO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop, 0, 0);
        break;
    case _rrzo_:
        this->m_enque_RRZO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop, 0, 0);
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:%d\n",dmaChannel);
        ret = 1;
        break;
    }

EXIT:

    return ret;
}


