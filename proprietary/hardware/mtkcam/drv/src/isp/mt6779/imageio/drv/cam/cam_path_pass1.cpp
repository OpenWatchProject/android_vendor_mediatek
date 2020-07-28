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

CAM_TG_CTRL  CamPathPass1::g_TgCtrl[PHY_CAM];
MUINT32      CamPathPass1::g_SenDev[PHY_CAM];
IspDMACfg    CamPathPass1::g_ImgoDmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_RrzoDmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_YuvoDmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_YuvboDmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_YuvcoDmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_CrzoR1DmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_CrzboR1DmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_CrzoR2DmaCfg[PHY_CAM];
IspDMACfg    CamPathPass1::g_CrzboR2DmaCfg[PHY_CAM];

P1_TUNING_NOTIFY* CamPathPass1::CamPathPass1::g_p1NotifyObj[PHY_CAM][CamPathPass1::CB_node] = {NULL};

inline static MBOOL isUf(EImageFormat iFmt)
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
/* for dmao ports hiden from user: ufeo/ufgo/yuvbo/yuvco/crzboR1/crzboR2 */
inline static void dmaCfgConvert(MUINT32 portId, IspDMACfg &a_dma, IspDMACfg &bayer_dma, E_BufPlaneID PlanId)
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

    a_dma.size.xsize        = rst.xsize_byte[PlanId];
    a_dma.size.stride       = rst.stride_byte[PlanId];

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
        BASE_LOG_ERR("NOT support this format(0x%x) in cam\n", bayer_dma.lIspColorfmt);
        break;
    case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
    case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
    case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
    case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
    case eImgFmt_BAYER16_APPLY_LSC:   /*!< Bayer format, 16-bit */
        a_dma.format_en=0;
        a_dma.bus_size_en=0;
        a_dma.format=0;
        a_dma.bus_size=1;
        break;
    case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
    case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
    case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
    case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
    case eImgFmt_YV16:      //422 format, 3 plane
    case eImgFmt_NV16:      //422 format, 2 plane
    default:
        a_dma.format_en=0;
        a_dma.bus_size_en=0;
        a_dma.format=0;
        a_dma.bus_size=0;
        break;
    }
}

inline static E_BufPlaneID QueryYuvPlaneMax(EImageFormat iFmt)
{//not support 420 1 plane & 16 bit
    switch (iFmt ) {
    case eImgFmt_YVYU://422              // 8 bit
    case eImgFmt_UYVY://422
    case eImgFmt_VYUY://422
    case eImgFmt_YUY2://422
    case eImgFmt_MTK_YUYV_Y210://422   //10 bit
    case eImgFmt_MTK_YVYU_Y210://422
    case eImgFmt_MTK_UYVY_Y210://422
    case eImgFmt_MTK_VYUY_Y210://422
    case eImgFmt_YUYV_Y210://422        //16 bit
    case eImgFmt_YVYU_Y210://422
    case eImgFmt_UYVY_Y210://422
    case eImgFmt_VYUY_Y210://422
        return ePlane_1st;
    case eImgFmt_NV61://422             // 8 bit
    case eImgFmt_NV16://422
    case eImgFmt_NV12://420
    case eImgFmt_NV21://420
    case eImgFmt_MTK_YUV_P210://422    //10 bit
    case eImgFmt_MTK_YVU_P210://422
    case eImgFmt_MTK_YUV_P010://420
    case eImgFmt_MTK_YVU_P010://420
    case eImgFmt_YUV_P210://422         //16 bit
    case eImgFmt_YVU_P210://422
    case eImgFmt_YUV_P010://420
    case eImgFmt_YVU_P010://420
        return ePlane_2nd;
    //case eImgFmt_YV16://422                     // 8 bit
    case eImgFmt_I422:
    //case eImgFmt_YV12://420
    case eImgFmt_I420:
    case eImgFmt_MTK_YUV_P210_3PLANE://422   //10 bit
    case eImgFmt_MTK_YUV_P010_3PLANE://420
    case eImgFmt_YUV_P210_3PLANE://422        //16 bit
    case eImgFmt_YUV_P010_3PLANE://420
        return ePlane_3rd;
    default:
        BASE_LOG_ERR("NOT supported yuv format(0x%x)\n", iFmt);
        return ePlane_1st;
    }
}

CamPathPass1::CamPathPass1() :
    m_p1NotifyTbl{  NSImageio::NSIspio::EPIPECmd_SET_RRZ_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_SGG2_CBFP,//SGG2 merge into GSE
                    NSImageio::NSIspio::EPIPECmd_SET_EIS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_LCS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_RSS_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_CRZR1_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_CRZR2_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_3A_HAL_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_TUNING_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_AWB_CBFP,
                    NSImageio::NSIspio::EPIPECmd_SET_REGDUMP_CBFP,//NOTE: cmd order must same as E_CB_MAP
                    NSImageio::NSIspio::EPIPECmd_SET_CQSTART_CBFP,
                    NSImageio::NSIspio::EPIPECmd_MAX}
{
    DBG_LOG_CONFIG(imageio, path);

    for (int i = 0;i < CB_node; i++) {
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
    this->m_TgCtrl = NULL;
    this->m_CrzR1 = NULL;
    this->m_CrzR2 = NULL;
}


MINT32 CamPathPass1::config(struct CamPathPass1Parameter* p_parameter)
{
    MINT32 ret = 0;
    MUINT32 cnt=0;
    CAM_TIMESTAMP* pTime;
    IspRect srcImgSize;

    ISP_PATH_DBG("CamPathPass1::config E");

    for (MUINT32 i = 0; i < CB_node; i++) {
        this->m_p1NotifyObj[i] = NULL;
    }

    if (this->m_pCmdQdrv == NULL) {
        ISP_PATH_ERR("NULL cmdQMgr\n");
        return 1;
    }

    this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_TopCtrl;

    if (p_parameter->m_Path & CamPathPass1Parameter::_TG_IN) {
        switch (p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT) {
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
            ISP_PATH_ERR("unsupported TG format:0x%x\n",p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT);
            return 1;
        }
    }
    else {
        switch (p_parameter->m_rawi.dmao.format) {//E_RAWI_TOP_FMT
        case RAWI_FMT_RAW8:
        case RAWI_FMT_RAW10:
        case RAWI_FMT_RAW12:
        case RAWI_FMT_RAW14:
        case RAWI_FMT_RAW8_2B:
        case RAWI_FMT_RAW10_2B:
        case RAWI_FMT_RAW12_2B:
        case RAWI_FMT_RAW14_2B:
            this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_RawCtrl;
            break;
        default:
            ISP_PATH_ERR("unsupported DMAI format:0x%x\n",p_parameter->m_rawi.dmao.format);
            return 1;
        }
    }

    if (p_parameter->bypass_ispDcifPipe == MFALSE) {
        this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_DcifCtrl;
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

    if (p_parameter->bypass_yuvo == MFALSE) {
        //revmove from functionlist, because of dmao enable & disable r via CQ.
        //it means dmao enable/disable need to start/stop as much times as DupCmdQ number
        //this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_Yuvo;
    }
    else {
        this->m_Yuvo.m_pIspDrv = NULL;
    }

    if (p_parameter->bypass_crzor1 == MFALSE) {
        //revmove from functionlist, because of dmao enable & disable r via CQ.
        //it means dmao enable/disable need to start/stop as much times as DupCmdQ number
        //this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_CrzboR1;
    }
    else {
        this->m_CrzboR1.m_pIspDrv = NULL;
    }

    if (p_parameter->bypass_crzor2 == MFALSE) {
        //revmove from functionlist, because of dmao enable & disable r via CQ.
        //it means dmao enable/disable need to start/stop as much times as DupCmdQ number
        //this->m_isp_function_list[cnt++ ] = (IspFunction_B*)&this->m_CrzboR2;
    }
    else {
        this->m_CrzboR2.m_pIspDrv = NULL;
    }

    this->m_isp_function_count = cnt;

    //cfg CmdQMgr
    //CQ config moved into CmdQMgr
    //this->m_pCmdQdrv->CmdQMgr_attach(p_parameter->m_fps,p_parameter->m_subSample,this->m_hwModule,ISP_DRV_CQ_THRE0,(void*)&this->m_CQ0);

    srcImgSize = p_parameter->m_src_size;
    switch(p_parameter->m_data_pat){
        case eCAM_DUAL_PIX:
            srcImgSize.w *= 2;
            if((srcImgSize.w % 16) != 0){
                ISP_PATH_ERR("cropping size need 16-alignment at dual_pix pattern\n");
                return 1;
            }
            break;
        case eCAM_YUV:
            srcImgSize.w *= 2;
            break;
        case eCAM_NORMAL:
        case eCAM_NORMAL_PD:
        case eCAM_4CELL:
        case eCAM_MONO:
        case eCAM_IVHDR:
        case eCAM_ZVHDR:
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
        default:
            break;
    }
    if((srcImgSize.w % 4) != 0){
        ISP_PATH_ERR("cropping size need 4-alignment\n");
        return 1;
    }

    this->m_srcImgSize = srcImgSize;
    if (p_parameter->m_Path & CamPathPass1Parameter::_TG_IN) {// DL/Bit-T
        this->m_TgCtrl = &this->g_TgCtrl[this->m_hwModule];
        this->m_TgCtrl->m_Crop = srcImgSize;
        if(p_parameter->m_Path & CamPathPass1Parameter::_TG_IN)//TG input support only continuous mode
            this->m_TgCtrl->m_continuous = MTRUE;
        else
            this->m_TgCtrl->m_continuous = MFALSE;

        this->m_TgCtrl->m_SubSample = p_parameter->m_subSample;

        this->m_TgCtrl->m_PixMode = p_parameter->m_src_pixmode;

        //tg setting is via phy reg, so all vir_ptr are all acceptable
        this->m_TgCtrl->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];

        this->m_TgCtrl->m_Datapat = p_parameter->m_data_pat;

        //tg setting is not via m_isp_function_list . so setting directly here
        ret += this->m_TgCtrl->config();
        //tg need to enable 1st for vsync signal
        ret += this->m_TgCtrl->pre_enable();
    }
    else if(p_parameter->m_Path == CamPathPass1Parameter::_DRAM_IN){//for DCIF debug convenient, always assign TG crop even if use RAWI.
        this->m_TgCtrl = &this->g_TgCtrl[this->m_hwModule];
        this->m_TgCtrl->m_Crop = srcImgSize;
        this->m_TgCtrl->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_TgCtrl->config();
    }

    //assign timestamp clk rate
    pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0]);
    pTime->TimeStamp_SrcClk(p_parameter->m_tTimeClk);
    if(p_parameter->bypass_ispDcifPipe == MFALSE){
        pTime->TimeStamp_SrcSofSel(TSTMP_SRC_EXTERNAL);
    }
    else{
        //use external as default, and timestamp source will refer to raw_sel and tg_sel
        pTime->TimeStamp_SrcSofSel(TSTMP_SRC_EXTERNAL);
    }

    //top
    switch(this->m_hwModule){
        case CAM_A:
            if(p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN){
                this->m_TopCtrl.CAM_Path = IF_RAWI_CAM_A;
            }
            else {
                this->m_TopCtrl.CAM_Path = IF_CAM_A;
            }
            break;
        case CAM_B:
            if(p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN){
                this->m_TopCtrl.CAM_Path = IF_RAWI_CAM_B;
            }
            else {
                this->m_TopCtrl.CAM_Path = IF_CAM_B;
            }
            break;
        case CAM_C:
            if(p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN){
                this->m_TopCtrl.CAM_Path = IF_RAWI_CAM_C;
            }
            else {
                this->m_TopCtrl.CAM_Path = IF_CAM_C;
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

    this->m_RawCtrl.m_dlp.RAW_SELO = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT2_SEL.Bits.CAMCTL_PIX_BUS_RAW_SELO,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.SEPI     = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT2_SEL.Bits.CAMCTL_PIX_BUS_SEPI,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.SEPO     = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_PIX_BUS_SEPO,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.MRG_R3   = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.MRG_R6   = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R6O,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.MRG_R7   = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O,ISP_PATH_ERR);
    this->m_RawCtrl.m_dlp.MRG_R8   = MAP_CAM_PIXMODE(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O,ISP_PATH_ERR);
    this->m_RawCtrl.m_bBin = m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN;
    this->m_Tuning.m_bBin = m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN;
    this->m_RawCtrl.m_SrcTG = (E_INPUT)MAP_MODULE_TG(this->m_hwModule,MFALSE,ISP_PATH_ERR);//op=0: hwModule to TG

    this->m_RawCtrl.m_bRRz = (p_parameter->bypass_rrzo == MTRUE)?(MFALSE):(MTRUE);
    this->m_RawCtrl.m_bDYUV = ((p_parameter->bypass_yuvo == MTRUE) &&
                               (p_parameter->bypass_crzor1 == MTRUE) &&
                               (p_parameter->bypass_crzor2 == MTRUE))?(MFALSE):(MTRUE);

    //raw or yuv pipe
    if(p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN){
        switch (p_parameter->m_rawi.dmao.format) {
            case RAWI_FMT_RAW8:
            case RAWI_FMT_RAW10:
            case RAWI_FMT_RAW12:
            case RAWI_FMT_RAW14:
            case RAWI_FMT_RAW8_2B:
            case RAWI_FMT_RAW10_2B:
            case RAWI_FMT_RAW12_2B:
            case RAWI_FMT_RAW14_2B:
                //raw pipe
                switch(p_parameter->m_NonScale_RawType){
                    case CamPathPass1Parameter::_PURE_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_FromTG;
                        break;
                    case CamPathPass1Parameter::_NON_LSC_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BeforLSC;
                        break;
                    case CamPathPass1Parameter::_BTW_LSC_WB_RAW:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BetweenLSCWB;
                        break;
                    default:
                        this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_AFTERLSC;
                        break;
                }
                this->m_RawCtrl.m_Source = this->m_TopCtrl.CAM_Path;
                this->m_RawCtrl.m_pP1Tuning = NULL;//null at config()
                this->m_RawCtrl.m_DataPat = this->m_RawCtrl.m_DataPat_config = p_parameter->m_data_pat;
                //this->m_RawCtrl.m_pIspDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

                break;
            default:
                ISP_PATH_ERR("unsupported DMAI format:0x%x\n",p_parameter->m_rawi.dmao.format);
                return 1;
                break;
        }
    }
    else if(p_parameter->m_Path & CamPathPass1Parameter::_TG_IN){
        switch(p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT){
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
                this->m_RawCtrl.m_DataPat = this->m_RawCtrl.m_DataPat_config = p_parameter->m_data_pat;

                //dma
                this->m_Rawi.dma_cfg        = p_parameter->m_rawi.dmao;
                if(isUf(p_parameter->m_rawi.dmao.lIspColorfmt)){
                    this->m_Ufdi.dma_cfg        = p_parameter->m_rawi.dmao;
                    this->m_DcifCtrl.m_bRawInUF = MTRUE;
                }
                else{
                    this->m_DcifCtrl.m_bRawInUF = MFALSE;
                }

                break;
            case TG_FMT_YUV422:
                //yuv pipe
                this->m_YuvCtrl.m_pP1Tuning = NULL;
                //this->m_YuvCtrl.m_pIspDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                break;
            default:
                ISP_PATH_ERR("unsupported TG format:0x%x\n",p_parameter->m_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT);
                return 1;
                break;
        }
    }
    else{
        ISP_PATH_ERR("path err:%d\n",p_parameter->m_Path);
        return 1;
    }
    //for stagger mode(tg+rawi) or rawi only, set dma (rawi/ufdi)
    if (p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN) {
        this->m_Rawi.dma_cfg        = p_parameter->m_rawi.dmao;
        if(isUf(p_parameter->m_rawi.dmao.lIspColorfmt)){
            capibility CamInfo;
            tCAM_rst rst;

            this->m_Ufdi.dma_cfg        = p_parameter->m_rawi.dmao;
            this->m_DcifCtrl.m_bRawInUF = MTRUE;
            //
            if (CamInfo.GetCapibility(
                    EPortIndex_RAWI,
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE,
                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)p_parameter->m_rawi.dmao.lIspColorfmt, p_parameter->m_rawi.dmao.crop.w, ePixMode_1),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                ISP_PATH_ERR("cropping size err(0x%x)\n",p_parameter->m_rawi.dmao.crop.w);
                ret = 1;
            }
            this->m_Ufdi.dma_cfg.size.xsize = rst.xsize_byte[1];
        }
        else{
            this->m_DcifCtrl.m_bRawInUF = MFALSE;
        }
    }

    this->m_RawCtrl.m_SrcSize.w = this->m_srcImgSize.w;
    this->m_RawCtrl.m_SrcSize.h = this->m_srcImgSize.h;
    this->m_Tuning.m_SrcSize.w = this->m_srcImgSize.w;
    this->m_Tuning.m_SrcSize.h = this->m_srcImgSize.h;
    this->m_DcifCtrl.m_SrcSize.w = this->m_srcImgSize.w;
    this->m_DcifCtrl.m_SrcSize.h = this->m_srcImgSize.h;

    this->m_DcifCtrl.m_bSvTwin = p_parameter->m_bSvTwin;

    //dmao & dmao fbc

    //====================================================
    // DMAO part config
    //====================================================
    if(p_parameter->bypass_imgo == MFALSE){
        this->m_Imgo.dma_cfg        = p_parameter->m_imgo.dmao;
        this->m_Imgo.Header_Addr    = p_parameter->m_imgo.FH_PA;
        this->m_Imgo.m_fps          = p_parameter->m_fps;
        this->g_ImgoDmaCfg[this->m_hwModule] = this->m_Imgo.dma_cfg;

        dmaCfgConvert(EPortIndex_IMGO, this->m_Ufeo.dma_cfg, p_parameter->m_imgo.dmao,ePlane_2nd);

        this->m_Ufeo.m_fps = p_parameter->m_fps;
    }
    if(p_parameter->bypass_rrzo == MFALSE){
        this->m_Rrzo.dma_cfg        = p_parameter->m_rrzo.dmao;
        this->m_Rrzo.Header_Addr    = p_parameter->m_rrzo.FH_PA;
        this->m_Rrzo.m_fps          = p_parameter->m_fps;
        this->g_RrzoDmaCfg[this->m_hwModule] = this->m_Rrzo.dma_cfg;

        //rrz initial-cfg
        this->m_Rrz.rrz_in_roi = p_parameter->rrz_in_roi;
        this->m_Rrz.rrz_out_size = p_parameter->m_Scaler;
        this->m_Rrz.rrz_rlb_offset = p_parameter->rrz_rlb_offset;
        this->m_RawCtrl.m_rrz_rlb_offset = p_parameter->rrz_rlb_offset;

        dmaCfgConvert(EPortIndex_RRZO, this->m_Ufgo.dma_cfg, p_parameter->m_rrzo.dmao,ePlane_2nd);

        this->m_Ufgo.m_fps = p_parameter->m_fps;
    }
    if(p_parameter->bypass_yuvo == MFALSE){
        E_BufPlaneID Plane_Id = QueryYuvPlaneMax(p_parameter->m_yuvo.dmao.lIspColorfmt);

        switch(Plane_Id) {
            case ePlane_3rd:
                this->m_Yuvco.dma_cfg        = p_parameter->m_yuvo.dmao;
                this->m_Yuvco.m_fps          = p_parameter->m_fps;
                dmaCfgConvert(EPortIndex_YUVO, this->m_Yuvco.dma_cfg, p_parameter->m_yuvo.dmao,Plane_Id);
                this->g_YuvcoDmaCfg[this->m_hwModule] = this->m_Yuvco.dma_cfg;
            case ePlane_2nd:
                this->m_Yuvbo.dma_cfg        = p_parameter->m_yuvo.dmao;
                this->m_Yuvbo.m_fps          = p_parameter->m_fps;
                dmaCfgConvert(EPortIndex_YUVO, this->m_Yuvbo.dma_cfg, p_parameter->m_yuvo.dmao,Plane_Id);
                this->g_YuvboDmaCfg[this->m_hwModule] = this->m_Yuvbo.dma_cfg;
            case ePlane_1st:
                this->m_Yuvo.dma_cfg        = p_parameter->m_yuvo.dmao;
                this->m_Yuvo.Header_Addr    = p_parameter->m_yuvo.FH_PA;
                this->m_Yuvo.m_fps          = p_parameter->m_fps;
                this->g_YuvoDmaCfg[this->m_hwModule] = this->m_Yuvo.dma_cfg;
                break;
            default:
                ISP_PATH_ERR("NOT supported yuvo format(0x%x)\n",p_parameter->m_yuvo.dmao.lIspColorfmt);
                return 1;
                break;
        }
    }
    if(p_parameter->bypass_crzor1 == MFALSE) {
        E_BufPlaneID Plane_Id = QueryYuvPlaneMax(p_parameter->m_crzor1.dmao.lIspColorfmt);
        CAM_CRZ_CTRL *lCrzCtl = new(std::nothrow) CAM_CRZ_CTRL(CAM_CRZ_CTRL::CRZ_R1,&(this->m_RawCtrl),MFALSE);

        if(lCrzCtl == NULL){
            ISP_PATH_ERR("create CRZ_R1 obj fail\n");
            return 1;
        }
        if(this->m_CrzR1){
            delete this->m_CrzR1;
        }
        this->m_CrzR1                 = lCrzCtl;

        switch(Plane_Id){
            case ePlane_2nd:
                this->m_CrzboR1.dma_cfg        = p_parameter->m_crzor1.dmao;
                this->m_CrzboR1.m_fps          = p_parameter->m_fps;
                dmaCfgConvert(EPortIndex_CRZO, this->m_CrzboR1.dma_cfg, p_parameter->m_crzor1.dmao, Plane_Id);
                this->g_CrzboR1DmaCfg[this->m_hwModule] = this->m_CrzboR1.dma_cfg;
            case ePlane_1st:
                this->m_CrzoR1.dma_cfg        = p_parameter->m_crzor1.dmao;
                this->m_CrzoR1.Header_Addr    = p_parameter->m_crzor1.FH_PA;
                this->m_CrzoR1.m_fps          = p_parameter->m_fps;
                this->g_CrzoR1DmaCfg[this->m_hwModule] = this->m_CrzoR1.dma_cfg;
                break;
            default:
                ISP_PATH_ERR("NOT supported crzo_r1 format(0x%x)\n",p_parameter->m_crzor1.dmao.lIspColorfmt);
                return 1;
                break;
        }

        //CrzR1 initial-cfg, support no crop
        this->m_CrzR1->crz_in_roi = p_parameter->crzR1_in_roi;
        this->m_CrzR1->crz_out_size = p_parameter->m_ScalerYuvR1;
    }
    if(p_parameter->bypass_crzor2 == MFALSE){
        E_BufPlaneID Plane_Id = QueryYuvPlaneMax(p_parameter->m_crzor2.dmao.lIspColorfmt);
        CAM_CRZ_CTRL *lCrzCtl = new(std::nothrow) CAM_CRZ_CTRL(CAM_CRZ_CTRL::CRZ_R2, &(this->m_RawCtrl),MFALSE);

        if(lCrzCtl == NULL){
            ISP_PATH_ERR("create CRZ_R2 obj fail\n");
            return 1;
        }
        if(this->m_CrzR2){
            delete this->m_CrzR2;
        }

        this->m_CrzR2                 = lCrzCtl;

        switch(Plane_Id){
            case ePlane_2nd:
                this->m_CrzboR2.dma_cfg        = p_parameter->m_crzor2.dmao;
                this->m_CrzboR2.m_fps          = p_parameter->m_fps;
                dmaCfgConvert(EPortIndex_CRZO_R2, this->m_CrzboR2.dma_cfg, p_parameter->m_crzor2.dmao, Plane_Id);
                this->g_CrzboR2DmaCfg[this->m_hwModule] = this->m_CrzboR2.dma_cfg;
            case ePlane_1st:
                this->m_CrzoR2.dma_cfg        = p_parameter->m_crzor2.dmao;
                this->m_CrzoR2.Header_Addr    = p_parameter->m_crzor2.FH_PA;
                this->m_CrzoR2.m_fps          = p_parameter->m_fps;
                this->g_CrzoR2DmaCfg[this->m_hwModule] = this->m_CrzoR2.dma_cfg;
                break;
            default:
                ISP_PATH_ERR("NOT supported crzo_r2 format(0x%x)\n",p_parameter->m_crzor2.dmao.lIspColorfmt);
                return 1;
                break;
        }

        //CrzR2 initial-cfg, support no crop
        this->m_CrzR2->crz_in_roi   = p_parameter->crzR2_in_roi;
        this->m_CrzR2->crz_out_size = p_parameter->m_ScalerYuvR2;
    }

    //loop for burst / duplicated cmdQ
    for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++){
            //top
            this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

            if(p_parameter->bypass_ispDcifPipe == MFALSE){
                this->m_DcifCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }

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
            if(p_parameter->bypass_crzor1 == MFALSE){
                //this->m_CrzoR1.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_CrzR1->m_pDrv = (ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }
            if(p_parameter->bypass_crzor2 == MFALSE){
                //this->m_CrzoR2.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
                this->m_CrzR2->m_pDrv = (ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];
            }

            ret += this->_config(j);

            if(p_parameter->bypass_ispRawPipe == MFALSE){
                this->m_RawCtrl.setNotify(E_Notify_Config);
            }
        }
    }


    //====================================================
    // FBC part config
    //====================================================
    this->m_Lcso_FBC.m_pTimeStamp = pTime;
    this->m_Eiso_FBC.m_pTimeStamp = pTime;
    this->m_Rsso_FBC.m_pTimeStamp = pTime;
    this->m_Rawi_FBC.m_pTimeStamp = pTime;
    this->m_Ufdi_FBC.m_pTimeStamp = pTime;

    for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++){
            IspDrvVir *_pDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

            this->m_Magic.m_pIspDrv = _pDrv;
            this->m_Magic.write2CQ(j);

            if(p_parameter->bypass_imgo == MFALSE){
                //fbc's control is moved to campath_bufctrl due to UF mode

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                //plz reference the description at line:81
                this->m_Imgo.m_pIspDrv = _pDrv;
                ret += this->m_Imgo.config();
                ret += this->m_Imgo.write2CQ(j);
                ret += this->m_Imgo.enable(NULL);

                //uf's fbc is moved to campath_bufctrl due to UF mode

                this->m_Ufeo.m_pIspDrv = _pDrv;
                ret += this->m_Ufeo.config();
                ret += this->m_Ufeo.write2CQ(j);
                //dynami enable/disable
            }
            if(p_parameter->bypass_rrzo == MFALSE){
                //fbc's control is moved to campath_bufctrl due to UF mode

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                //plz reference the description at line:81
                this->m_Rrzo.m_pIspDrv = _pDrv;
                ret += this->m_Rrzo.config();
                ret += this->m_Rrzo.write2CQ(j);
                ret += this->m_Rrzo.enable(NULL);

                //uf's fbc is moved to campath_bufctrl due to UF mode

                this->m_Ufgo.m_pIspDrv = _pDrv;
                ret += this->m_Ufgo.config();
                ret += this->m_Ufgo.write2CQ(j);
                //dynami enable/disable
            }
            if(p_parameter->bypass_yuvo == MFALSE){
                //fbc's control is moved to campath_bufctrl due to multi-plane

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                switch(QueryYuvPlaneMax(p_parameter->m_yuvo.dmao.lIspColorfmt)){
                    case ePlane_3rd:
                        this->m_Yuvco.m_pIspDrv = _pDrv;
                        ret += this->m_Yuvco.config();
                        ret += this->m_Yuvco.write2CQ(j);
                        ret += this->m_Yuvco.enable(NULL);
                    case ePlane_2nd:
                        this->m_Yuvbo.m_pIspDrv = _pDrv;
                        ret += this->m_Yuvbo.config();
                        ret += this->m_Yuvbo.write2CQ(j);
                        ret += this->m_Yuvbo.enable(NULL);
                    case ePlane_1st:
                        this->m_Yuvo.m_pIspDrv = _pDrv;
                        ret += this->m_Yuvo.config();
                        ret += this->m_Yuvo.write2CQ(j);
                        ret += this->m_Yuvo.enable(NULL);
                        break;
                    default:
                        break;
                }
            }
            if(p_parameter->bypass_crzor1 == MFALSE){
                //fbc's control is moved to campath_bufctrl due to multi-plane

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                switch(QueryYuvPlaneMax(p_parameter->m_crzor1.dmao.lIspColorfmt)){
                    case ePlane_2nd:
                        this->m_CrzboR1.m_pIspDrv = _pDrv;
                        ret += this->m_CrzboR1.config();
                        ret += this->m_CrzboR1.write2CQ(j);
                        ret += this->m_CrzboR1.enable(NULL);
                    case ePlane_1st:
                        this->m_CrzoR1.m_pIspDrv = _pDrv;
                        ret += this->m_CrzoR1.config();
                        ret += this->m_CrzoR1.write2CQ(j);
                        ret += this->m_CrzoR1.enable(NULL);
                        break;
                    default:
                        break;
                }
            }
            if(p_parameter->bypass_crzor2 == MFALSE){
                //fbc's control is moved to campath_bufctrl due to multi-plane

                //special case for imgo/rrzo dmao_en,  because dma_en is via cq .
                switch(QueryYuvPlaneMax(p_parameter->m_crzor2.dmao.lIspColorfmt)){
                    case ePlane_2nd:
                        this->m_CrzboR2.m_pIspDrv = _pDrv;
                        ret += this->m_CrzboR2.config();
                        ret += this->m_CrzboR2.write2CQ(j);
                        ret += this->m_CrzboR2.enable(NULL);
                    case ePlane_1st:
                        this->m_CrzoR2.m_pIspDrv = _pDrv;
                        ret += this->m_CrzoR2.config();
                        ret += this->m_CrzoR2.write2CQ(j);
                        ret += this->m_CrzoR2.enable(NULL);
                        break;
                    default:
                        break;
                }
            }
            if(p_parameter->m_Path & CamPathPass1Parameter::_DRAM_IN){
                this->m_vUFI.clear();
                if(isUf(p_parameter->m_rawi.dmao.lIspColorfmt)){
                    this->m_Ufdi.m_pIspDrv = _pDrv;
                    ret += this->m_Ufdi.config();
                    ret += this->m_Ufdi.write2CQ(j);
                    ret += this->m_Ufdi.enable(NULL);

                    this->m_Ufdi_FBC.m_pIspDrv = _pDrv;
                    ret += this->m_Ufdi_FBC.config();
                    ret += this->m_Ufdi_FBC.write2CQ(j);
                    ret += this->m_Ufdi_FBC.enable(NULL);
                }
                this->m_Rawi.m_pIspDrv = _pDrv;
                ret += this->m_Rawi.config();
                ret += this->m_Rawi.write2CQ(j);
                ret += this->m_Rawi.enable(NULL);

                this->m_Rawi_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Rawi_FBC.config();
                ret += this->m_Rawi_FBC.write2CQ(j);
                ret += this->m_Rawi_FBC.enable(NULL);
            }
        }
    }


    {//for imgo/ufeo, rrzo/ufgo,port which is affected by twin's resoult
        list<MUINT32> channel;
        NSImageio::NSIspio::PortID input;

        channel.clear();
        if (p_parameter->bypass_rrzo == MFALSE)
            channel.push_back(_rrzo_);
        if (p_parameter->bypass_imgo == MFALSE)
            channel.push_back(_imgo_);
        if (p_parameter->bypass_yuvo == MFALSE)
            channel.push_back(_yuvo_);
        if (p_parameter->bypass_crzor1 == MFALSE)
            channel.push_back(_crzo_);
        if (p_parameter->bypass_crzor2 == MFALSE)
            channel.push_back(_crzo_r2_);

        input.tTimeClk = p_parameter->m_tTimeClk;
        input.tgFps = p_parameter->m_fps;
        //
        this->m_PBC.PBC_config((void*)this,NULL,&channel,&input);
    }

    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, this->m_hwModule, CAM_BUF_CTRL::eCamDmaType_main);

    //tuning drv need to init before isp start, and init only once before or after start
    //no sensor id , remove to p1tunecfg
    //this->m_Tuning._config();
    //this->m_Tuning._enable(NULL);
    this->m_Tuning.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_Tuning.configNotify();//initial m_NotifyFrame
    this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];
    this->m_RawCtrl.configNotify();//initial m_NotifyFrame

    //init pipechk for debug convenient
    {
        PIPE_CHECK ppc;

        ppc.m_pDrv = this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];
        if(p_parameter->bypass_ispDcifPipe == MFALSE){
            ppc.Register_HW_PATH(E_PPC_DC);
        }
        else{
            if(p_parameter->m_Path == (CamPathPass1Parameter::_TG_IN|CamPathPass1Parameter::_DRAM_IN)){
                ppc.Register_HW_PATH(E_PPC_RAWI);
            }
            else if(p_parameter->m_Path == CamPathPass1Parameter::_TG_IN){
                ppc.Register_HW_PATH(E_PPC_DL);
            }
            else{
                ppc.Register_HW_PATH(E_PPC_MAX);
                ISP_PATH_WRN("path unsupported for HW pipechk\n");
            }
        }
    }
    ISP_PATH_DBG("CamPathPass1::config X");

    //inorder to dump 1st frame cq for dbg only
    this->m_TopCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0];

    this->m_FSM = op_cfg;

    return ret;
}


MINT32 CamPathPass1::_config(MUINT32 BurstIdx)
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

        if (( ret_val = isplist[i]->write2CQ(BurstIdx) ) < 0) {
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
    /* Call back to ispMgr to notify CQ start.*/
    this->setCQStartNotify();

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

    if(this->m_TgCtrl && (ret_val = this->m_TgCtrl->enable(NULL)< 0) ) {
        ISP_PATH_ERR("[ERROR] tg enable fail");
        goto EXIT;
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
    if(this->m_TgCtrl){
        this->m_TgCtrl = NULL;
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
    MBOOL               bForce = *(MBOOL *)pParam;

    // if under suspending, when dynamic twin, this may be twin's slave cam, and cannot
    // change phy reg during stop
    ISP_PATH_WRN("stop:: CAM_BUF_CTRL::m_CamState[0x%x]=0x%x this->m_FSM=0x%x",
        this->m_hwModule, CAM_BUF_CTRL::m_CamState[this->m_hwModule],this->m_FSM);

    // reset global cb info when stop
    for(MUINT32 i = 0 ; i < CamPathPass1::CB_node ; i++){
        g_p1NotifyObj[this->m_hwModule][i] = NULL;
    }

    if (this->m_FSM == op_suspend) {
        ISP_PATH_INF("stop when suspending, bypass touch hw reg\n");
        goto EXIT;
    }

    if (!bForce){//normal stop
        if(!this->m_TgCtrl){
            ISP_PATH_ERR("null tg_ctrl obj. no config or after uninit?\n");
            goto EXIT;
        }

        if(this->m_TgCtrl->m_pIspDrv) {
            this->m_TgCtrl->pre_disable();
        }
    } else {//force stop: no need wait vsync after disable VF
        if (this->m_TgCtrl && this->m_TgCtrl->m_pIspDrv != NULL) {
            this->m_TgCtrl->disable();
            this->m_TgCtrl->m_pIspDrv = NULL;//for multi-cam disable check
        }
    }

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for (MUINT32 i = 0; i < this->m_pCmdQdrv->CmdQMgr_GetDuqQ(); i++) {
        for (MUINT32 j = 0; j < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); j++) {
            IspDrvVir *_pDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

            if (this->m_Imgo.m_pIspDrv != NULL) {
                this->m_Imgo.m_pIspDrv = _pDrv;
                ret += this->m_Imgo.disable();
            }
            if (this->m_Ufeo.m_pIspDrv != NULL) {
                this->m_Ufeo.m_pIspDrv = _pDrv;
                ret += this->m_Ufeo.disable();
            }
            if (this->m_Rrzo.m_pIspDrv != NULL) {
                this->m_Rrzo.m_pIspDrv = _pDrv;
                ret += this->m_Rrzo.disable();
            }
            if (this->m_Ufgo.m_pIspDrv != NULL) {
                this->m_Ufgo.m_pIspDrv = _pDrv;
                ret += this->m_Ufgo.disable();
            }
            if(this->m_Ufdi.m_pIspDrv != NULL){
                this->m_Ufdi.m_pIspDrv = _pDrv;
                ret += this->m_Ufdi.disable();
            }
            if(this->m_Rawi.m_pIspDrv != NULL){
                this->m_Rawi.m_pIspDrv = _pDrv;
                ret += this->m_Rawi.disable();
            }
            if (this->m_Yuvo.m_pIspDrv != NULL) {
                this->m_Yuvo.m_pIspDrv = _pDrv;
                ret += this->m_Yuvo.disable();
            }
            if (this->m_Yuvbo.m_pIspDrv != NULL) {
                this->m_Yuvbo.m_pIspDrv = _pDrv;
                ret += this->m_Yuvbo.disable();
            }
            if (this->m_Yuvco.m_pIspDrv != NULL) {
                this->m_Yuvco.m_pIspDrv = _pDrv;
                ret += this->m_Yuvco.disable();
            }
            if (this->m_CrzoR1.m_pIspDrv != NULL) {
                this->m_CrzoR1.m_pIspDrv = _pDrv;
                ret += this->m_CrzoR1.disable();
            }
            if (this->m_CrzboR1.m_pIspDrv != NULL) {
                this->m_CrzboR1.m_pIspDrv = _pDrv;
                ret += this->m_CrzboR1.disable();
            }
            if (this->m_CrzoR2.m_pIspDrv != NULL) {
                this->m_CrzoR2.m_pIspDrv = _pDrv;
                ret += this->m_CrzoR2.disable();
            }
            if (this->m_CrzboR2.m_pIspDrv != NULL) {
                this->m_CrzboR2.m_pIspDrv = _pDrv;
                ret += this->m_CrzboR2.disable();
            }
        }
    }

    for (int i = 0; i < ispcount; i++ ) {
        if( ( ret += isplist[i]->disable(pParam) ) != 0 ){
            ISP_PATH_ERR("_stop fail\n");
            goto EXIT;
        }
    }
    //FBC
    for(MUINT32 i = 0;i<this->m_pCmdQdrv->CmdQMgr_GetDuqQ();i++){
        for(MUINT32 j=0;j<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();j++) {
            IspDrvVir *_pDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[j];

            if(this->m_Lcso_FBC.m_pIspDrv != NULL){
                this->m_Lcso_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Lcso_FBC.disable();
            }
            if(this->m_Eiso_FBC.m_pIspDrv != NULL){
                this->m_Eiso_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Eiso_FBC.disable();
            }
            if(this->m_Rsso_FBC.m_pIspDrv != NULL){
                this->m_Rsso_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Rsso_FBC.disable();
            }
            if(this->m_Rawi_FBC.m_pIspDrv != NULL){
                this->m_Rawi_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Rawi_FBC.disable();
            }
            if(this->m_Ufdi_FBC.m_pIspDrv != NULL){
                this->m_Ufdi_FBC.m_pIspDrv = _pDrv;
                ret += this->m_Ufdi_FBC.disable();
            }
            //imgo/ufeo/rrzo/ufgo/yuv*o/crz*o fbc's diable move to campath_buf_ctrl due to uf mode
        }
    }


    this->m_Tuning._disable();  // tuning buf free in uninit()

    if(this->m_CrzR1){
        delete this->m_CrzR1;
        this->m_CrzR1 = NULL;
    }
    if(this->m_CrzR2){
        delete this->m_CrzR2;
        this->m_CrzR2 = NULL;
    }

    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, this->m_hwModule, CAM_BUF_CTRL::eCamDmaType_main);


EXIT:
    this->m_PBC.PBC_Stop();
    if (this->m_TgCtrl &&  this->m_TgCtrl->m_pIspDrv != NULL) {
        this->m_TgCtrl->disable();
        this->m_TgCtrl->m_pIspDrv = NULL;//for multi-cam disable check
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
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    Pak_Func Pakfun;

    if (pImgo_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("imgo data length mismatch:0x%x_0x%x\n",pImgo_cfg_L->size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();
    Pakfun.Bits.PAK_DMAO = 0;//imgo

    //burst 0
    for (i = 0, it = pImgo_cfg_L->begin(); i < 1; it++, i++) {
        this->m_Imgo.dma_cfg.lIspColorfmt = it->img_fmt;
        bWithUfo = isUf(this->m_Imgo.dma_cfg.lIspColorfmt);
        switch(this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT){
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
                case 3:
                    this->m_RawCtrl.m_RawType = CAM_RAW_PIPE::E_BetweenLSCWB;
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
                ISP_PATH_ERR("unsupported TG format:0x%x\n",this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT);
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
        if(MTRUE == dynamicPakFmtMapping(this->m_Imgo.dma_cfg.lIspColorfmt, this->m_RawCtrl.m_RawType, (MUINT32*)&HwImgoFmt)){
            this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(MFALSE == this->m_RawCtrl.dynamicPak(HwImgoFmt, Pakfun)){
                ISP_PATH_ERR("dynamicPak error at burstidx:0x%x\n",i);
                ret = 1;
            }
        }

        //new IMGO FMT's stride for Dynamic UFO/Pak/UnPak
        this->m_Imgo.dma_cfg.size.stride = it->enqueue_img_stride;
        this->m_Imgo.dma_cfg.pixel_byte = it->pixel_byte;

        ISP_PATH_DBG("BurstIdx:0x%x,imgo:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,imgo)stride_byte:(%d,%d,%d,%d),rawtype:0x%x,dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x),pixel_byte(%d)\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Imgo.dma_cfg.size.stride,it->rawType,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_DBN_R1_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN,
            it->img_fmt_configpipe, this->m_Imgo.dma_cfg.lIspColorfmt, this->m_Imgo.dma_cfg.pixel_byte);

        //After dynamicPak(), Enque New Img Fmt to Header
        fmt_sel.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], CAMCTL_R1_CAMCTL_FMT_SEL);
        fh_imgo.Header_Enque(Header_IMGO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_IMGO_R1_FMT);
        fh_ufeo.Header_Enque(Header_UFEO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_IMGO_R1_FMT);

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
    //other pages: update dma ysize only, no support pak/unpak/ufo/raw_type changed
    for (i = 1; it != pImgo_cfg_L->end(); it++, i++) {
        ISP_PATH_DBG("BurstIdx:0x%x,imago:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,imgo)stride_byte:(%d,%d,%d,%d),rawtype:0x%x,dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x)\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Imgo.dma_cfg.size.stride,it->rawType,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_DBN_R1_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN,
            it->img_fmt_configpipe, this->m_Imgo.dma_cfg.lIspColorfmt);

        //imgo
        this->m_Imgo.dma_cfg.crop.h = it->crop.h;
        this->m_Imgo.dma_cfg.size.h = it->crop.h;

        this->m_Imgo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Imgo.config()!=0){
            ISP_PATH_ERR("imgo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (bWithUfo) {
             this->m_Ufeo.dma_cfg.size.h = this->m_Imgo.dma_cfg.size.h;

             //update ysize cfg
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
    tCAM_rst rst, rstStrideByte;
    list<IspRrzCfg>::iterator it;

    if (pRrz_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("rrz data length mismatch:0x%x_0x%x\n", pRrz_cfg_L->size(), this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }
    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    //burst 0
    for (i = 0, it = pRrz_cfg_L->begin(); i < 1; i++, it++) {

        this->m_Rrzo.dma_cfg.lIspColorfmt = it->img_fmt;
        bWithUfo = isUf(this->m_Rrzo.dma_cfg.lIspColorfmt);

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

        if(this->m_p1NotifyObj[_RRZ_] == NULL)
            this->m_Tuning.setSLK();

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

    //other pages: update dma ysize only
    for (i = 1; it != pRrz_cfg_L->end(); i++, it++) {
        ISP_PATH_DBG("BurstIdx:0x%x,rrz out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x\n",
            i, it->rrz_out_size.w, it->rrz_out_size.h, it->rrz_in_roi.x, it->rrz_in_roi.y,
            it->rrz_in_roi.w, it->rrz_in_roi.h, this->m_Rrzo.dma_cfg.lIspColorfmt);

        //rrzo use no dmao cropping function, cropping function is depended on rrz
        this->m_Rrzo.dma_cfg.size.h = it->rrz_out_size.h;

        //update ysize cfg
        this->m_Rrzo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Rrzo.config()!=0){
            ISP_PATH_ERR("rrzo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (bWithUfo) {
             this->m_Ufgo.dma_cfg.size.h = it->rrz_out_size.h;

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

MINT32 CamPathPass1::setP1YuvoCfg(list<DMACfg>* pYuvo_cfg_L)
{
    MINT32 ret = 0;

    capibility CamInfo;
    tCAM_rst rst,rstStrideByte;
    MUINT32 targetIdx = 0;
    E_BufPlaneID nPlane = ePlane_1st;
    E_CamPixelMode _pix = ePixMode_NONE;
    list<DMACfg>::iterator it;
    int i;
    Header_YUVO fh_yuvo;
    Header_YUVBO fh_yuvbo;
    Header_YUVCO fh_yuvco;
    REG_CAMCTL_R1_CAMCTL_FMT2_SEL fmt_sel2;
    MUINT32 HwDmaoFmt = YUVO_FMT_YUV422_1P;
    MBOOL bDNG = MFALSE;
    Pak_Func Pakfun;

    if (pYuvo_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("yuvo data length mismatch:0x%x_0x%x\n",pYuvo_cfg_L->size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();
    Pakfun.Bits.PAK_DMAO = 1;//yuvo

    //burst 0
    for (i = 0, it = pYuvo_cfg_L->begin(); i < 1; it++, i++) {
        this->m_Yuvo.dma_cfg.lIspColorfmt = it->img_fmt;
        nPlane = QueryYuvPlaneMax(this->m_Yuvo.dma_cfg.lIspColorfmt);
        _pix = (E_CamPixelMode)this->m_RawCtrl.getCurPixMode(_yuvo_);

        if (CamInfo.GetCapibility(
                EPortIndex_YUVO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Yuvo.dma_cfg.lIspColorfmt, it->crop.w, _pix),
                rst, E_CAM_UNKNOWNN) == MFALSE) {
            ISP_PATH_ERR("cropping size err(0x%x)\n",it->crop.w);
            ret = 1;
        }
        if (CamInfo.GetCapibility(
                EPortIndex_YUVO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Yuvo.dma_cfg.lIspColorfmt, it->tgCropW, ePixMode_4),
                rstStrideByte, E_CAM_UNKNOWNN) == MFALSE) {
            ISP_PATH_ERR("querying STRIDE_BYTE size err(0x%x)\n",it->tgCropW);
            ret = 1;
        }

        if(it->enqueue_img_stride != rstStrideByte.stride_byte[0]){
            ISP_PATH_ERR("enqueue_img_stride(%d) != query_img_stride(%d)\n",it->enqueue_img_stride, rstStrideByte.stride_byte[0]);
            return 1;
        }

        // Dynamic Change Pak/Unpak Fmt
        if(MTRUE == dynamicDngFmtMapping(this->m_Yuvo.dma_cfg.lIspColorfmt, (MUINT32*)&HwDmaoFmt, (Pak_Func*)&Pakfun)){
            this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(MFALSE == this->m_RawCtrl.dynamicPak(HwDmaoFmt, Pakfun)){
                ISP_PATH_ERR("dynamicPak error at burstidx:0x%x\n",i);
                ret = 1;
            }
        }

        //new CRZO FMT's stride for Dynamic UFO/Pak/UnPak
        this->m_Yuvo.dma_cfg.size.stride = rstStrideByte.stride_byte[0];

        ISP_PATH_DBG("BurstIdx:0x%x,yuvo:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,imgo)stride_byte:(%d,%d,%d,%d),dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x),Plane:%d\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Yuvo.dma_cfg.size.stride,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_DBN_R1_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN,
            it->img_fmt_configpipe, this->m_Yuvo.dma_cfg.lIspColorfmt,nPlane);

        //After dynamicPak(), Enque New Img Fmt to Header
        //hw dma fmt
        fmt_sel2.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], CAMCTL_R1_CAMCTL_FMT2_SEL);
        fh_yuvo.Header_Enque(Header_YUVO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);
        fh_yuvbo.Header_Enque(Header_YUVBO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);
        fh_yuvco.Header_Enque(Header_YUVCO::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel2.Bits.CAMCTL_YUVO_R1_FMT);

        //sw fmt
        fh_yuvo.Header_Enque(Header_YUVO::E_SW_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], it->img_fmt);

        this->m_Yuvo.dma_cfg.crop.x = it->crop.x;
        this->m_Yuvo.dma_cfg.crop.y = it->crop.y;
        this->m_Yuvo.dma_cfg.crop.w = it->crop.w;
        this->m_Yuvo.dma_cfg.crop.h = it->crop.h;
        this->m_Yuvo.dma_cfg.size.xsize = rst.xsize_byte[0];
        this->m_Yuvo.dma_cfg.size.h = it->crop.h;

        this->m_Yuvo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Yuvo.config()!=0){
            ISP_PATH_ERR("yuvo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (nPlane >= ePlane_2nd) {
             this->m_Yuvbo.dma_cfg.size.w = this->m_Yuvo.dma_cfg.crop.w;
             this->m_Yuvbo.dma_cfg.size.h = this->m_Yuvo.dma_cfg.size.h;
             this->m_Yuvbo.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_Yuvbo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Yuvbo.config() != 0) {
                 ISP_PATH_ERR("yuvbo config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

        if (nPlane == ePlane_3rd) {
             this->m_Yuvco.dma_cfg.size.w = this->m_Yuvo.dma_cfg.crop.w;
             this->m_Yuvco.dma_cfg.size.h = this->m_Yuvo.dma_cfg.size.h;
             this->m_Yuvco.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_Yuvco.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Yuvco.config() != 0) {
                 ISP_PATH_ERR("yuvco config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

    }

    //other pages: update dma ysize only
    for (i = 1; it != pYuvo_cfg_L->end(); it++, i++) {
        ISP_PATH_DBG("BurstIdx:0x%x,yuvo:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,imgo)stride_byte:(%d,%d,%d,%d),dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x),Plane:%d\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Yuvo.dma_cfg.size.stride,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_DBN_R1_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN,
            it->img_fmt_configpipe, this->m_Yuvo.dma_cfg.lIspColorfmt,nPlane);

        this->m_Yuvo.dma_cfg.crop.h = it->crop.h;
        this->m_Yuvo.dma_cfg.size.h = it->crop.h;

        this->m_Yuvo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Yuvo.config()!=0){
            ISP_PATH_ERR("yuvo config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (nPlane >= ePlane_2nd) {
             this->m_Yuvbo.dma_cfg.size.h = this->m_Yuvo.dma_cfg.size.h;
             //update ysize cfg
             this->m_Yuvbo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Yuvbo.config() != 0) {
                 ISP_PATH_ERR("yuvbo config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

        if (nPlane == ePlane_3rd) {
             this->m_Yuvco.dma_cfg.size.h = this->m_Yuvo.dma_cfg.size.h;
             //update ysize cfg
             this->m_Yuvco.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_Yuvco.config() != 0) {
                 ISP_PATH_ERR("yuvco config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }
    }

    return ret;
}

MINT32 CamPathPass1::setP1CrzoR1Cfg(list<IspCrzrCfg>* pCrzo_cfg_L)
{
    MINT32 ret = 0, i;
    MUINT32 targetIdx = 0;
    E_BufPlaneID nPlane = ePlane_1st;
    capibility CamInfo;
    tCAM_rst rst, rstStrideByte;
    list<IspCrzrCfg>::iterator it;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    Header_CRZO_R1  fh_crzo;
    Header_CRZBO_R1 fh_crzbo;

    if (pCrzo_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("crz_r1 data length mismatch:0x%x_0x%x\n", pCrzo_cfg_L->size(), this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }
    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    //burst 0
    for (i = 0, it = pCrzo_cfg_L->begin(); i < 1; i++, it++) {

        this->m_CrzoR1.dma_cfg.lIspColorfmt = it->img_fmt;
        nPlane = QueryYuvPlaneMax(this->m_CrzoR1.dma_cfg.lIspColorfmt);

        ISP_PATH_DBG("BurstIdx:0x%x,crz_r1 out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x,nPlane:%d\n",
            i, it->crz_out_size.w, it->crz_out_size.h, it->crz_in_roi.x, it->crz_in_roi.y,
            it->crz_in_roi.w, it->crz_in_roi.h, this->m_CrzoR1.dma_cfg.lIspColorfmt,nPlane);

        //FH
        fmt_sel.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], CAMCTL_R1_CAMCTL_FMT_SEL);
        fh_crzo.Header_Enque(Header_CRZO_R1::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_CRZO_R1_FMT);
        fh_crzbo.Header_Enque(Header_CRZBO_R1::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_CRZO_R1_FMT);

        //sw fmt
        fh_crzo.Header_Enque(Header_CRZO_R1::E_SW_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], it->img_fmt);

        //crzo use no dmao cropping function, cropping function is depended on rrz
        this->m_CrzoR1.dma_cfg.crop.x = 0;
        this->m_CrzoR1.dma_cfg.crop.y = 0;
        this->m_CrzoR1.dma_cfg.crop.floatX = 0;
        this->m_CrzoR1.dma_cfg.crop.floatY = 0;
        this->m_CrzoR1.dma_cfg.crop.w = it->crz_in_roi.w;//support no crop
        this->m_CrzoR1.dma_cfg.crop.h = it->crz_in_roi.h;
        this->m_CrzoR1.dma_cfg.size.w = it->crz_out_size.w;
        this->m_CrzoR1.dma_cfg.size.h = it->crz_out_size.h;

        //mark, use original stride which is configured at init phase
        //DMARrzo.dma_cfg.size.stride = rrz_cfg.crz_out_size.stride;
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if (CamInfo.GetCapibility(EPortIndex_CRZO,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_CrzoR1.dma_cfg.lIspColorfmt,
                    this->m_CrzoR1.dma_cfg.size.w, (E_CamPixelMode)this->m_RawCtrl.getCurPixMode(_crzo_)),
                rst, E_CAM_UNKNOWNN) == MFALSE)
        {
            ISP_PATH_ERR("crzo_r1 size err(0x%x)\n",this->m_CrzoR1.dma_cfg.size.w);
            ret = 1;
        }
        this->m_CrzoR1.dma_cfg.size.xsize      =  rst.xsize_byte[0];

        //update xsize cfg
        this->m_CrzoR1.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzoR1.config()!= 0){
            ISP_PATH_ERR("crzo_r1 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
        if (nPlane == ePlane_2nd) {
             this->m_CrzboR1.dma_cfg.size.w = it->crz_out_size.w;
             this->m_CrzboR1.dma_cfg.size.h = it->crz_out_size.h;
             this->m_CrzboR1.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_CrzboR1.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_CrzboR1.config() != 0) {
                 ISP_PATH_ERR("crzbo_r1 config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

        //dv decides crz input & crop size (input == crop)
        this->m_CrzR1->crz_in_roi = it->crz_in_roi;
        this->m_CrzR1->crz_out_size = it->crz_out_size;

        //update rrz cfg
        this->m_CrzR1->m_pDrv = (ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzR1->config() < 0){
            ISP_PATH_ERR("crz_r1 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
    }

    //other pages: update dma ysize only
    for (i = 1; it != pCrzo_cfg_L->end(); i++, it++) {
        ISP_PATH_DBG("BurstIdx:0x%x,crz_r1 out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x,plane:%d\n",
            i, it->crz_out_size.w, it->crz_out_size.h, it->crz_in_roi.x, it->crz_in_roi.y,
            it->crz_in_roi.w, it->crz_in_roi.h, this->m_CrzoR1.dma_cfg.lIspColorfmt,nPlane);

        //crzo use no dmao cropping function, cropping function is depended on rrz
        this->m_CrzoR1.dma_cfg.size.h = it->crz_out_size.h;

        //update xsize cfg
        this->m_CrzoR1.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzoR1.config()!= 0){
            ISP_PATH_ERR("crzo_r1 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
        if (nPlane == ePlane_2nd) {
             this->m_CrzboR1.dma_cfg.size.h = it->crz_out_size.h;
             //update ysize cfg
             this->m_CrzboR1.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_CrzboR1.config() != 0) {
                 ISP_PATH_ERR("crzbo_r1 config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }
    }

    return ret;
}

MINT32 CamPathPass1::setP1CrzoR2Cfg(list<IspCrzrCfg>* pCrzo_cfg_L)
{
    MINT32 ret = 0, i;
    MUINT32 targetIdx = 0;
    E_BufPlaneID nPlane = ePlane_1st;
    capibility CamInfo;
    tCAM_rst rst, rstStrideByte;
    list<IspCrzrCfg>::iterator it;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    Header_CRZO_R2  fh_crzo;
    Header_CRZBO_R2 fh_crzbo;

    if (pCrzo_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("crz_r2 data length mismatch:0x%x_0x%x\n", pCrzo_cfg_L->size(), this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }
    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    //burst 0
    for (i = 0, it = pCrzo_cfg_L->begin(); i < 1; i++, it++) {
        this->m_CrzoR2.dma_cfg.lIspColorfmt = it->img_fmt;
        nPlane = QueryYuvPlaneMax(this->m_CrzoR2.dma_cfg.lIspColorfmt);

        ISP_PATH_DBG("BurstIdx:0x%x,crz_r2 out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x,Plane:%d\n",
            i, it->crz_out_size.w, it->crz_out_size.h, it->crz_in_roi.x, it->crz_in_roi.y,
            it->crz_in_roi.w, it->crz_in_roi.h, this->m_CrzoR2.dma_cfg.lIspColorfmt,nPlane);

        //FH
        fmt_sel.Raw = CAM_READ_REG(this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], CAMCTL_R1_CAMCTL_FMT_SEL);
        fh_crzo.Header_Enque(Header_CRZO_R2::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_CRZO_R2_FMT);
        fh_crzbo.Header_Enque(Header_CRZBO_R2::E_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], fmt_sel.Bits.CAMCTL_CRZO_R2_FMT);

        //sw fmt
        fh_crzo.Header_Enque(Header_CRZO_R2::E_SW_FMT,(ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i], it->img_fmt);

        //crzo use no dmao cropping function, cropping function is depended on rrz
        this->m_CrzoR2.dma_cfg.crop.x = 0;
        this->m_CrzoR2.dma_cfg.crop.y = 0;
        this->m_CrzoR2.dma_cfg.crop.floatX = 0;
        this->m_CrzoR2.dma_cfg.crop.floatY = 0;
        this->m_CrzoR2.dma_cfg.crop.w = it->crz_in_roi.w;//support no crop
        this->m_CrzoR2.dma_cfg.crop.h = it->crz_in_roi.h;
        this->m_CrzoR2.dma_cfg.size.w = it->crz_out_size.w;
        this->m_CrzoR2.dma_cfg.size.h = it->crz_out_size.h;

        //mark, use original stride which is configured at init phase
        //DMACrzo.dma_cfg.size.stride = crz_cfg.crz_out_size.stride;
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if (CamInfo.GetCapibility(EPortIndex_CRZO_R2,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_CrzoR2.dma_cfg.lIspColorfmt,
                    this->m_CrzoR2.dma_cfg.size.w, (E_CamPixelMode)this->m_RawCtrl.getCurPixMode(_crzo_r2_)),
                rst, E_CAM_UNKNOWNN) == MFALSE)
        {
            ISP_PATH_ERR("crzo_r2 size err(0x%x)\n",this->m_CrzoR2.dma_cfg.size.w);
            ret = 1;
        }
        this->m_CrzoR2.dma_cfg.size.xsize      =  rst.xsize_byte[0];

        //update xsize cfg
        this->m_CrzoR2.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzoR2.config()!= 0){
            ISP_PATH_ERR("crzo_r2 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
        if (nPlane == ePlane_2nd) {
             this->m_CrzboR2.dma_cfg.size.w = it->crz_out_size.w;
             this->m_CrzboR2.dma_cfg.size.h = it->crz_out_size.h;
             this->m_CrzboR2.dma_cfg.size.xsize = rst.xsize_byte[1];

             //update xsize cfg
             this->m_CrzboR2.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_CrzboR2.config() != 0) {
                 ISP_PATH_ERR("crzbo_r2 config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }

        //dv decides crz input & crop size (input == crop)
        this->m_CrzR2->crz_in_roi     = it->crz_in_roi;
        this->m_CrzR2->crz_out_size   = it->crz_out_size;

        //update rrz cfg
        this->m_CrzR2->m_pDrv = (ISP_DRV_CAM*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzR2->config() < 0){
            ISP_PATH_ERR("crz_r2 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
    }

    //other pages: update dma ysize only
    for (i = 1; it != pCrzo_cfg_L->end(); i++, it++) {
        ISP_PATH_DBG("BurstIdx:0x%x,crz_r2 out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x fmt:0x%x,Plane:%d\n",
            i, it->crz_out_size.w, it->crz_out_size.h, it->crz_in_roi.x, it->crz_in_roi.y,
            it->crz_in_roi.w, it->crz_in_roi.h, this->m_CrzoR2.dma_cfg.lIspColorfmt,nPlane);

        //crzo use no dmao cropping function, cropping function is depended on rrz
        this->m_CrzoR2.dma_cfg.size.h = it->crz_out_size.h;
        //update ysize cfg
        this->m_CrzoR2.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_CrzoR2.config()!= 0){
            ISP_PATH_ERR("crzo_r2 config error at burstidx:0x%x\n",i);
            ret = 1;
        }
        if (nPlane == ePlane_2nd) {
             this->m_CrzboR2.dma_cfg.size.h = it->crz_out_size.h;
             //update ysize cfg
             this->m_CrzboR2.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
             if (this->m_CrzboR2.config() != 0) {
                 ISP_PATH_ERR("crzbo_r2 config error at burstidx:0x%x\n",i);
                 ret = 1;
             }
        }
    }

    return ret;
}

MINT32 CamPathPass1::setP1RawiCfg(list<DMACfg>* pRawi_cfg_L)
{
    MINT32 ret = 0;
    capibility CamInfo;
    tCAM_rst rst,rstStrideByte;
    MUINT32 targetIdx = 0;
    MBOOL   bWithUf = MFALSE;
    E_CamPixelMode _pix = ePixMode_NONE;
    list<DMACfg>::iterator it;
    int i;
    MUINT32 HwImgoFmt = RAWI_FMT_RAW8;
    REG_CAMCTL_R1_CAMCTL_FMT3_SEL fmt_sel3;
    Pak_Func Pakfun;

    if (pRawi_cfg_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()) {
        ISP_PATH_ERR("rawi data length mismatch:0x%x_0x%x\n",pRawi_cfg_L->size(),this->m_pCmdQdrv->CmdQMgr_GetBurstQ());
        return 1;
    }

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();
    Pakfun.Bits.PAK_DMAO = 2;//rawi

    //burst 0
    for (i = 0, it = pRawi_cfg_L->begin(); i < 1; it++, i++) {
        this->m_Rawi.dma_cfg.lIspColorfmt = it->img_fmt;
        bWithUf = isUf(this->m_Rawi.dma_cfg.lIspColorfmt);
        switch(this->m_TopCtrl.cam_top_ctl.FMT3_SEL.Bits.CAMCTL_RAWI_R2_FMT){
            case RAWI_FMT_RAW8:
            case RAWI_FMT_RAW10:
            case RAWI_FMT_RAW12:
            case RAWI_FMT_RAW14:
            case RAWI_FMT_RAW8_2B:
            case RAWI_FMT_RAW10_2B:
            case RAWI_FMT_RAW12_2B:
            case RAWI_FMT_RAW14_2B:
                _pix = (E_CamPixelMode)ePixMode_1;//only support 1 pix
                break;
            default:
                ISP_PATH_ERR("unsupported rawi format:0x%x\n",this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT);
                break;
        }

        if (CamInfo.GetCapibility(
                EPortIndex_RAWI,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->m_Rawi.dma_cfg.lIspColorfmt, it->crop.w, _pix),
                rst, E_CAM_UNKNOWNN) == MFALSE) {
            ISP_PATH_ERR("cropping size err(0x%x)\n",it->crop.w);
            ret = 1;
        }


        // Dynamic Change Pak/Unpak Fmt
        if(MTRUE == dynamicUNPFmtMapping(this->m_Rawi.dma_cfg.lIspColorfmt, (MUINT32*)&HwImgoFmt)){
            this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(MFALSE == this->m_RawCtrl.dynamicPak(HwImgoFmt, Pakfun)){
                ISP_PATH_ERR("dynamicPak error at burstidx:0x%x\n",i);
                ret = 1;
            }
        }

        //rawi no need to support dynamic input stride
        //this->m_Rawi.dma_cfg.size.stride = rstStrideByte.stride_byte[0];

        ISP_PATH_DBG("BurstIdx:0x%x,rawi:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x,(query,configpipe,enqueue,rawi)stride_byte:(%d,%d,%d,%d),rawtype:0x%x,dbn:%d,bin:%d,(configpipe,enque)fmt:(0x%x,0x%x)\n",
            i, it->crop.x, it->crop.y, it->crop.w, it->crop.h, rst.xsize_byte[0],rst.xsize_byte[1],rstStrideByte.stride_byte[0],it->out.stride,it->enqueue_img_stride,this->m_Rawi.dma_cfg.size.stride,it->rawType,
            this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_DBN_R1_EN, this->m_TopCtrl.cam_top_ctl.FUNC_EN2.Bits.CAMCTL_BIN_R1_EN,
            it->img_fmt_configpipe, this->m_Rawi.dma_cfg.lIspColorfmt);

        this->m_Rawi.dma_cfg.size.w = it->crop.w;
        this->m_Rawi.dma_cfg.size.h = it->crop.h;
        this->m_Rawi.dma_cfg.size.xsize = rst.xsize_byte[0];

        this->m_Rawi.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_Rawi.config()!=0){
            ISP_PATH_ERR("rawi config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        if (bWithUf) {
            this->m_Ufdi.dma_cfg.size.w = this->m_Rawi.dma_cfg.size.w;
            this->m_Ufdi.dma_cfg.size.h = this->m_Rawi.dma_cfg.size.h;
            this->m_Ufdi.dma_cfg.size.xsize = rst.xsize_byte[1];

            this->m_Ufdi.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if (this->m_Ufdi.config() != 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
            if (this->m_Ufdi.write2CQ(i) != 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
            if(this->m_Ufdi.enable(NULL)!= 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
            this->m_Ufdi_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if (this->m_Ufdi_FBC.config() != 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
            if (this->m_Ufdi_FBC.write2CQ(i) != 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
            if(this->m_Ufdi_FBC.enable(NULL)!= 0) {
                ISP_PATH_ERR("error at burstidx:0x%x\n",i);
                ret = 1;
            }
        }
        else{
            this->m_Ufdi.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(this->m_Ufdi.disable()!= 0) {
                ISP_PATH_ERR("ufdi error at burstidx:0x%x\n",i);
                ret = 1;
            }
            this->m_Ufdi_FBC.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
            if(this->m_Ufdi_FBC.disable()!= 0) {
                ISP_PATH_ERR("ufdi error at burstidx:0x%x\n",i);
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

    //if(pTune_L->size() != this->m_pCmdQdrv->CmdQMgr_GetBurstQ()){
    if(pTune_L->size() != 1){//due to 3A updating rate is subsmapled
        ISP_PATH_ERR("tuning data length mismatch:0x%x_1\n",pTune_L->size());
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
        this->g_SenDev[this->m_hwModule] = pTune_L->begin()->SenDev;
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
        if(this->m_Magic.config()!=0){
            ISP_PATH_ERR("maic config error at burstidx:0x%x\n",i);
            ret = 1;
        }

        //update tuning
        //remove to above for loop for working with tuningMgr.
        //this->m_Tuning.m_Magic[i] = it->magic;
        this->m_Tuning.m_SenDev = it->SenDev;

        //MUST before tuning.update to decide whether pdo on/off
        this->m_Dpd_ctl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        if(this->m_RawCtrl.m_DataPat != this->m_RawCtrl.m_DataPat_config){
            if((this->m_RawCtrl.m_DataPat == eCAM_DUAL_PIX) ||
                (this->m_RawCtrl.m_DataPat_config == eCAM_DUAL_PIX)){
                ISP_PATH_ERR("switching dual pd is not supported (%d_%d)\n",
                    this->m_RawCtrl.m_DataPat, this->m_RawCtrl.m_DataPat_config);
            }
            else {
                this->m_Dpd_ctl.PD_PATH_EN(MFALSE);
            }
        }
        else {
            if(this->m_RawCtrl.m_DataPat_config == eCAM_NORMAL_PD){
                this->m_Dpd_ctl.PD_PATH_EN(MTRUE);//for CAM_BUF_CTRL::updateState
            }
        }

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

MBOOL CamPathPass1::P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj,MBOOL bInitFBC)
{
    MBOOL ret = MTRUE;
    MUINT32 tbl_len = 0;
    CAM_BUF_CTRL *pFbc = NULL;

    while (tbl_len < CB_node) {
        if (this->m_p1NotifyTbl[tbl_len] == cmd) {
            break;
        }
        tbl_len++;
    }

    //if notify module is exist in drv,
    //enabled FBC , modules which belong to FBC is enabled in UNI_MGR
    if(tbl_len < CB_node){
        switch(cmd){
            case EPIPECmd_SET_LCS_CBFP:
                pFbc = &m_Lcso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_EIS_CBFP:
                pFbc = &m_Eiso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_SGG2_CBFP://SGG2 merge into GSE
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RSS_CBFP:
                pFbc = &m_Rsso_FBC;
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RRZ_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case EPIPECmd_SET_3A_HAL_CBFP:
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
            case  EPIPECmd_SET_CRZR1_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case  EPIPECmd_SET_CRZR2_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            case  EPIPECmd_SET_CQSTART_CBFP:
                this->m_p1NotifyObj[tbl_len] = pObj;
                break;
            default:
                ret = MFALSE;
                ISP_PATH_ERR("un-supported p1notify_0x%x\n",cmd);
                break;
        }
    }

    if (ret && pFbc && bInitFBC) {
        for(MUINT32 i = 0; i < this->m_pCmdQdrv->CmdQMgr_GetDuqQ(); i++) {
            //for(MUINT32 j = 0; j < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); j++)
            //only config/write2CQ/enable at burst 0
            {
                pFbc->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(i)[0];
                pFbc->config();
                pFbc->write2CQ(0);
                pFbc->enable(NULL);
            }
        }
    }

    return ret;
}

MBOOL CamPathPass1::P1Notify_Global_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj)
{
    MBOOL ret = MTRUE;
    MUINT32 tbl_len = 0;

    while (tbl_len < CB_node) {
        if (this->m_p1NotifyTbl[tbl_len] == cmd) {
            break;
        }
        tbl_len++;
    }

    //if notify module is exist in drv,
    //enabled FBC , modules which belong to FBC is enabled in UNI_MGR
    if(tbl_len < CB_node){
        switch(cmd){
            case EPIPECmd_SET_LCS_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_EIS_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_SGG2_CBFP://SGG2 merge into GSE
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RSS_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_RRZ_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_3A_HAL_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_TUNING_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_REGDUMP_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case EPIPECmd_SET_AWB_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case  EPIPECmd_SET_CRZR1_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case  EPIPECmd_SET_CRZR2_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            case  EPIPECmd_SET_CQSTART_CBFP:
                this->g_p1NotifyObj[this->m_hwModule][tbl_len] = pObj;
                break;
            default:
                ret = MFALSE;
                ISP_PATH_ERR("un-supported p1notify_0x%x\n",cmd);
                break;
        }
    }

    return ret;
}

MINT32 CamPathPass1::setP1Notify(void)
{
    MUINT32 node = 0;
    MINT32 ret = 0;
    MUINT32 targetIdx = 0;
    MBOOL bWdmaOn;

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();
    switch(this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT){
    case TG_FMT_RAW8:
    case TG_FMT_RAW10:
    case TG_FMT_RAW12:
    case TG_FMT_RAW14:
    while (node < p1Notify_node) {
        if (this->m_p1NotifyObj[node] != NULL) {

            this->m_RawCtrl.m_pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[node];
            this->m_Tuning.m_pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[node];
            ISP_PATH_DBG("P1Notify: %s\n",this->m_RawCtrl.m_pP1Tuning->TuningName());

            /* Not loop burstQ, due to db_load_hold, 3A updating rate is subsampled */
            /*for (MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++) */{
                MUINT32 i=0;
                this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
                this->m_Tuning.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];

                switch(node){
                case _RRZ_:
                    if (this->m_RawCtrl.setRRZ() == MFALSE) {
                        ret = 1;
                    }
                    if (this->m_Tuning.setSLK() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _LMV_:
                    bWdmaOn = MFALSE;
                    if (this->m_RawCtrl.setLMV(bWdmaOn) == MFALSE) {
                        ret = 1;
                    }
                    if (bWdmaOn) {
                        DMA_EISO lmvo;
                        for (MUINT32 burst = 1; burst < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); burst++) {
                            lmvo.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[burst];
                            lmvo.write2CQ(burst);
                        }
                    }
                    break;
                case _RSS_:
                    bWdmaOn = MFALSE;
                    if (this->m_RawCtrl.setRSS(bWdmaOn) == MFALSE) {
                        ret = 1;
                    }
                    if (bWdmaOn) {
                        DMA_RSSO rsso;
                        for (MUINT32 burst = 1; burst < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); burst++) {
                            rsso.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[burst];
                            rsso.write2CQ(burst);
                        }
                    }
                    break;
                case _LCS_:
                    bWdmaOn = MFALSE;
                    if (this->m_RawCtrl.setLCS(bWdmaOn) == MFALSE) {
                        ret = 1;
                    }
                    if (bWdmaOn) {
                        DMA_LCSO lcso;
                        for (MUINT32 burst = 1; burst < this->m_pCmdQdrv->CmdQMgr_GetBurstQ(); burst++) {
                            lcso.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[burst];
                            lcso.write2CQ(burst);
                        }
                    }
                    break;
                case _GSE_:
                    if (this->m_RawCtrl.setGSE() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _TUNING_:
                    if (this->m_Tuning.setTuning() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _REG_DUMP_:
                    if (this->m_Tuning.dumpCQReg() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _AWB_:
                    if (this->m_Tuning.setAWB() == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _CRZR1_:
                    if (this->m_RawCtrl.setCRZ((MUINT32)CAM_CRZ_CTRL::CRZ_R1) == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _CRZR2_:
                    if (this->m_RawCtrl.setCRZ((MUINT32)CAM_CRZ_CTRL::CRZ_R2) == MFALSE) {
                        ret = 1;
                    }
                    break;
                case _3A_:
                    if (this->m_RawCtrl.set3A() == MFALSE) {
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

    this->m_Tuning.setNotify(E_Notify_Update);
    this->m_RawCtrl.setNotify(E_Notify_Update);
    this->m_Tuning.setNotifyDone();
    this->m_RawCtrl.setNotifyDone();

    //MUST after p1Notify to over-write pde_en/pdi_en
    this->m_Dpd_ctl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[0];
    if(this->m_RawCtrl.m_DataPat != this->m_RawCtrl.m_DataPat_config){
        if((this->m_RawCtrl.m_DataPat == eCAM_DUAL_PIX) ||
            (this->m_RawCtrl.m_DataPat_config == eCAM_DUAL_PIX)){
            ISP_PATH_ERR("switching dual pd is not supported (%d_%d)\n",
                this->m_RawCtrl.m_DataPat, this->m_RawCtrl.m_DataPat_config);
        }
        else {
            this->m_Dpd_ctl.PD_PATH_EN(MFALSE);
        }
    }
    else {
        if(this->m_RawCtrl.m_DataPat_config == eCAM_NORMAL_PD){
            this->m_Dpd_ctl.PD_PATH_EN(MTRUE);//for CAM_BUF_CTRL::updateState
        }
    }

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
                    case _GSE_:
                        if(this->m_YuvCtrl.setGSE() == MFALSE)
                            ret = 1;
                        break;
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
        ISP_PATH_ERR("unsupported TG format:0x%x\n",this->m_TopCtrl.cam_top_ctl.FMT_SEL.Bits.CAMCTL_TG_FMT);
        return 1;
        break;
    }

    return ret;
}

MINT32 CamPathPass1::setCQStartNotify(void)
{
    MINT32 ret = 0;
    if (this->m_p1NotifyObj[_CQSTART_]!= NULL) {
        this->m_Tuning.m_pP1Tuning = (P1_TUNING_NOTIFY*)this->m_p1NotifyObj[_CQSTART_];
        this->m_Tuning.m_pP1Tuning->p1TuningNotify(NULL, NULL);
    } else {
        ret = -1;
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
    CAM_BUF_CTRL        *pFBC = NULL, *pFBC_UF = NULL;
    DMAO_B              *pDmao = NULL;
    DMAI_B              *pDmai = NULL,*pDma_UF = NULL;
    MUINT32 targetIdx = 0;
    MINT32  ret = 0;

    switch(dmaChannel){
    case _imgo_:
    case _rrzo_:
    case _yuvo_:
    case _crzo_:
    case _crzo_r2_:
        //multi-plane
        if(this->m_PBC.enque_push(dmaChannel,bufInfo,NULL) == MFALSE)
            return 1;
        else
            return 0;
        break;
    case _rawi_:
        pFBC = &this->m_Rawi_FBC;
        pDmai = &this->m_Rawi;

        pFBC_UF = &this->m_Ufdi_FBC ;
        pDma_UF = &this->m_Ufdi ;
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

        if(pDmao){
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
        }
        else if(pDmai) {
            if(bufInfo.front().bReplace == MTRUE){
                pDmai->dma_cfg.memBuf.base_pAddr = bufInfo.front().Replace.image.mem_info.pa_addr;
            }
            else{
                pDmai->dma_cfg.memBuf.base_pAddr = bufInfo.front().u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
            }
            pDmai->m_pIspDrv = _pCurDrv;
            if(pDmai->setBaseAddr()){
                ISP_PATH_ERR("set baseaddress at burst:0x%x\n",i);
                ret = 1;
            }
        }
        else{
            ISP_PATH_ERR("logic err at burst:0x%x\n",i);
            ret = 1;
        }

        if((bufInfo.front().u_op.enque.size()-1) == (MUINT32)ePlane_3rd){
            MBOOL bUF = MFALSE;
            if(pFBC_UF && pDma_UF){
                pFBC_UF->m_pIspDrv = _pCurDrv;
                stISP_BUF_INFO  _Buf;
                ST_BUF_INFO stbuf;
                stbuf.image = bufInfo.front().u_op.enque.at(ePlane_2nd).image;
                stbuf.header = bufInfo.front().u_op.enque.at(ePlane_2nd).header;
                _Buf.u_op.enque.push_back(stbuf);

                if(eCmd_Fail == pFBC->enqueueHwBuf(_Buf)){
                    ISP_PATH_ERR("enque fail at burst:0x%x\n",i);
                    ret = 1;
                }

                if(bufInfo.front().bReplace == MTRUE){
                    pDma_UF->dma_cfg.memBuf.base_pAddr = bufInfo.front().Replace.image.mem_info.pa_addr;
                }
                else{
                    pDma_UF->dma_cfg.memBuf.base_pAddr = bufInfo.front().u_op.enque.at(ePlane_2nd).image.mem_info.pa_addr;
                }
                pDma_UF->m_pIspDrv = _pCurDrv;
                if(pDma_UF->setBaseAddr()){
                    ISP_PATH_ERR("set baseaddress at burst:0x%x\n",i);
                    ret = 1;
                }
                bUF = MTRUE;
                this->m_vUFI.push_back(bUF);
            }
            else{
                ISP_PATH_ERR("UFDI logic errorat burst:0x%x\n",i);
                ret = 1;
            }


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
    case _yuvo_:
    case _crzo_:
    case _crzo_r2_:
        //multi-plane
        return this->m_PBC.deque(dmaChannel,bufInfo,pNotify);
        break;
    case _rawi_:
        pFBC = &this->m_Rawi_FBC;
        pFBCUfo = &this->m_Ufdi_FBC;
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

        if(this->m_vUFI.size()){
            if(this->m_vUFI.front() == MTRUE){
                if(pFBCUfo){
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
                }
                else{
                    ret = -1;
                    ISP_PATH_ERR("logic err, deque fail at burst:0x%x\n",i);
                }
            }
            this->m_vUFI.pop_front();
        }

        bufInfo.push_back(buf);
    }
    this->m_lock.unlock();

    return ret;
}


MBOOL CamPathPass1::suspend(E_CAMPATH_SUSPEND_OP eOP) // cannno_ep
{
    MUINT32 _step = 0;
    ISP_HW_MODULE srcTgModule = (ISP_HW_MODULE)MAP_MODULE_TG(this->m_RawCtrl.m_SrcTG, 0, ISP_PATH_ERR);

    ISP_PATH_INF("enter suspending mode: srcTg:%d srcTgModule:%d\n", this->m_RawCtrl.m_SrcTG, srcTgModule);

    switch (eOP) {
    case _CAMPATH_SUSPEND_STOPHW:
        if(this->m_DcifCtrl.m_pIspDrv == NULL){//non-DCIF case, need to disable VF firstly
            if(this->g_TgCtrl[srcTgModule].m_pIspDrv == NULL){
                ISP_PATH_ERR("m_pIspDrv must not be null");
                return MFALSE;
            }
            if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                        srcTgModule, (ISP_DRV_CAM*)this->g_TgCtrl[srcTgModule].m_pIspDrv, CAM_BUF_CTRL::eSus_HWOFF) == MFALSE) {
                _step = 1;
            }
        }
        if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                    this->m_hwModule, NULL, CAM_BUF_CTRL::eSus_HW_SW_STATE) == MFALSE) {
            _step = 2;
        }

        this->m_pCmdQdrv->CmdQMgr_suspend();

        this->m_FSM = op_suspend;
        break;
    case _CAMPATH_SUSPEND_FLUSH:
        if (CAM_BUF_CTRL::suspend(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                    this->m_hwModule, NULL, CAM_BUF_CTRL::eSus_SIGNAL) == MFALSE) {
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

MBOOL CamPathPass1::resume(void)
{
    MBOOL ret;
    ISP_HW_MODULE srcTgModule = (ISP_HW_MODULE)MAP_MODULE_TG(this->m_RawCtrl.m_SrcTG, 0, ISP_PATH_ERR);

    ISP_PATH_INF("exit suspending mode: srcTg:%d srcTgModule:%d\n", this->m_RawCtrl.m_SrcTG, srcTgModule);

    this->m_pCmdQdrv->CmdQMgr_resume();

    ret = CAM_BUF_CTRL::resume(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                        NULL, CAM_BUF_CTRL::eRes_HW_STATE);
    ret = CAM_BUF_CTRL::resume(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                                        NULL, CAM_BUF_CTRL::eRes_SW_STATE);

    if(this->m_DcifCtrl.m_pIspDrv == NULL){//non-DCIF case, need to enable VF
        if(this->g_TgCtrl[srcTgModule].m_pIspDrv == NULL){
            ISP_PATH_ERR("m_pIspDrv must not be null");
            ret = MFALSE;
            goto EXIT;
        }
        ret = CAM_BUF_CTRL::resume(this->m_hwModule, this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(0)[0],
                               (ISP_DRV_CAM*)this->g_TgCtrl[srcTgModule].m_pIspDrv, CAM_BUF_CTRL::eRes_HWON);
    }

    this->m_FSM = op_start;

EXIT:
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

MBOOL CamPathPass1::DoneCollector(MBOOL bDoneCollect, E_INPUT TgIdx)
{
    MBOOL ret = MTRUE;
    MUINT32 targetIdx = 0;

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        this->m_RawCtrl.DoneCollector(bDoneCollect, TgIdx);
    }

    return ret;
}

MBOOL CamPathPass1::updatePath(UpdatePathParam pathParm)
{
    MBOOL ret = MTRUE;
    MUINT32 targetIdx = 0;

    targetIdx = this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx();

    memcpy((void *) &this->m_RawCtrl.m_dlp, (void *)pathParm.pDlp, sizeof(T_DLP));
    this->m_RawCtrl.m_bBin = pathParm.bBin;

    //switch RAW_SEL
    if(pathParm.TgEvent == UpdatePathParam::TG_EXTERNAL){
        ISP_PATH_DBG("updatePath switch+: TgIdx:%d bBin:%d bDoneCollect:%d", pathParm.TgIdx, pathParm.bBin, pathParm.bDoneCollect);
        //chagne tg obj
        ISP_HW_MODULE hw_module = (ISP_HW_MODULE)MAP_MODULE_TG(pathParm.TgIdx,0,ISP_PATH_ERR);//op=0: tg ->HW_MODULE
        E_CamPattern  data_pat = this->g_TgCtrl[hw_module].m_Datapat;

        //chagne tg obj for disable
        if(this->g_TgCtrl[hw_module].m_pIspDrv == NULL){
            return MFALSE;
        }

        //change TgCtrl obj
        this->m_TgCtrl = &this->g_TgCtrl[hw_module];

        //update tuningDrv
        this->m_Tuning.updateTuningDrv(this->g_SenDev[hw_module]);

        //update p1notify obj
        if(this->updateP1NotifyObj(hw_module) == MFALSE){
            ISP_PATH_ERR("update p1notify obj fail");
        }

        //update dma config
        this->m_Imgo.dma_cfg = this->g_ImgoDmaCfg[hw_module];
        this->m_Rrzo.dma_cfg = this->g_RrzoDmaCfg[hw_module];
        this->m_Yuvo.dma_cfg = this->g_YuvoDmaCfg[hw_module];
        this->m_Yuvbo.dma_cfg = this->g_YuvboDmaCfg[hw_module];
        this->m_Yuvco.dma_cfg = this->g_YuvcoDmaCfg[hw_module];
        this->m_CrzoR1.dma_cfg = this->g_CrzoR1DmaCfg[hw_module];
        this->m_CrzboR1.dma_cfg = this->g_CrzboR1DmaCfg[hw_module];
        this->m_CrzoR2.dma_cfg = this->g_CrzoR2DmaCfg[hw_module];
        this->m_CrzboR2.dma_cfg = this->g_CrzoR2DmaCfg[hw_module];

        //update tg idx
        this->m_RawCtrl.m_SrcTG = pathParm.TgIdx;

        //update SrcSize
        this->m_RawCtrl.m_SrcSize.w = this->m_Tuning.m_SrcSize.w = pathParm.SrcSize.w;
        this->m_RawCtrl.m_SrcSize.h = this->m_Tuning.m_SrcSize.h = pathParm.SrcSize.h;
        if(this->m_DcifCtrl.m_pIspDrv){
            if(this->m_DcifCtrl.m_SrcSize.w != pathParm.SrcSize.w){
                ISP_PATH_ERR("logic conflict. DCIF not support per-frame update input size\n");
                ret = MFALSE;
            }
        }

        ISP_PATH_DBG("updatePath switch-: TgIdx:%d bBin:%d bDoneCollect:%d", pathParm.TgIdx, pathParm.bBin, pathParm.bDoneCollect);

        //switch data pattern
        this->m_RawCtrl.m_DataPat = data_pat;

        if((hw_module == CAM_C) && (data_pat == eCAM_DUAL_PIX)){
            ISP_PATH_ERR("CamC, dual_pd type is not supported\n");
        }
    }

    for(MUINT32 i=0;i<this->m_pCmdQdrv->CmdQMgr_GetBurstQ();i++){
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[i];
        this->m_RawCtrl.config();
    }

    // RawCtrl re-config here after initTwin(), so need to do setNotify() again
    if(pathParm.bSetNotify){
        this->m_Tuning.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[0];
        this->m_Tuning.setNotify(E_Notify_Config);
        this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[0];
        this->m_RawCtrl.setNotify(E_Notify_Config);
    }

    //no burst
    this->m_Tuning.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[0];
    this->m_Tuning.m_bBin = pathParm.bBin;
    this->m_Tuning.configCB(pathParm.bForceConfigCB);

    this->m_RawCtrl.m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(targetIdx)[0];
    this->m_RawCtrl.m_bBin = pathParm.bBin;
    this->m_RawCtrl.configCB(pathParm.bForceConfigCB);

    return ret;
}

MBOOL CamPathPass1::updateP1NotifyObj(ISP_HW_MODULE hwModule)
{
    MBOOL ret = MTRUE;

    // for w+t switch, need to update p1 notify obj on stage 2, but no need to re-init fbc
    for(MUINT32 i = 0 ; i < CB_node - 1 ; i++){
        ret &= P1Notify_Mapping(m_p1NotifyTbl[i], g_p1NotifyObj[hwModule][i], MFALSE);
    }

    return ret;
}

MBOOL CamPathPass1::dynamicPakFmtMapping(ImgInfo::EImgFmt_t imgFmt, CAM_RAW_PIPE::E_RawType mRawType, MUINT32* pHwImgoFmt)
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
        case eImgFmt_BAYER16_APPLY_LSC: //= 0x221A,   //Bayer format, 16-bit
            *pHwImgoFmt = IMGO_FMT_RAW16;
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

MBOOL CamPathPass1::dynamicUNPFmtMapping(ImgInfo::EImgFmt_t imgFmt, MUINT32* pHwRawiFmt)
{
    MBOOL ret = MTRUE;

    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pHwRawiFmt = RAWI_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pHwRawiFmt = RAWI_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pHwRawiFmt = RAWI_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pHwRawiFmt = RAWI_FMT_RAW14;
            break;
        case eImgFmt_BAYER8_UNPAK:
            *pHwRawiFmt = RAWI_FMT_RAW8_2B;
            break;
        case eImgFmt_BAYER10_UNPAK:
            *pHwRawiFmt = RAWI_FMT_RAW10_2B;
            break;
        case eImgFmt_BAYER12_UNPAK:
            *pHwRawiFmt = RAWI_FMT_RAW12_2B;
            break;
        case eImgFmt_BAYER14_UNPAK:
            *pHwRawiFmt = RAWI_FMT_RAW14_2B;
            break;
        default:
            ISP_PATH_DBG("eImgFmt:[0x%x]NOT Support UNPFmtMapping",imgFmt);
            ret = MFALSE;
    }
    //
    ISP_PATH_DBG("input imgFmt(0x%x),output HwRawiFmtValue:0x%x",imgFmt, *pHwRawiFmt);
    return ret;
}

MBOOL CamPathPass1::dynamicDngFmtMapping(ImgInfo::EImgFmt_t imgFmt, MUINT32* pHwYuvoFmt,Pak_Func* pYuvPak)
{
    MBOOL ret = MTRUE;

    if(pHwYuvoFmt == NULL || pYuvPak == NULL){
        ISP_PATH_ERR("parameter is null (0x%x_0x%x",pHwYuvoFmt, pYuvPak);
        return MFALSE;
    }

    switch (imgFmt) {
        /******************************
         *  1 plane, 8bit
         ******************************/
        case eImgFmt_YUY2:                //422 format 1 plane (YUYV)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_YVYU:                //422 format 1 plane (YVYU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 2;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_UYVY:                //422 format 1 plane (UYVY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 1;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_VYUY:               //422 format 1 plane (VYUY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 3;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /******************************
         *  1 plane, 10bit
         ******************************/
        case eImgFmt_MTK_YUYV_Y210:      //422 format 10bit  1 plane (YUYV) = Y210
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_YVYU_Y210:      //422 format 10bit  1 plane (YVYU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 2;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_UYVY_Y210:      //422 format 10bit  1 plane (UYVY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 1;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_VYUY_Y210:      //422 format 10bit  1 plane (VYUY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 3;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /******************************
         *  1 plane, 16bit
         ******************************/
        case eImgFmt_YUYV_Y210:          //422 format 16bit  1 plane (YUYV) = Y210
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_YVYU_Y210:          //422 format 16bit  1 plane (YVYU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 2;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_UYVY_Y210:          //422 format 16bit  1 plane (UYVY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 1;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_VYUY_Y210:          //422 format 16bit  1 plane (VYUY)
            *pHwYuvoFmt = YUVO_FMT_YUV422_1P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 3;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /******************************
         *  2 plane, 8bit
         ******************************/
        case eImgFmt_NV16:               //422 format 2 plane (Y)(UV)
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_NV61:               //422 format 2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        case eImgFmt_NV12:               //420 format 2 plane (Y)(UV)
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_NV21:               //420 format 2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        /******************************
         *  2 plane, 10bit
         ******************************/
        case eImgFmt_MTK_YUV_P210:       //422 format 10bit  2 plane (Y)(UV) = P210
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_YVU_P210:       //422 format 10bit  2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        case eImgFmt_MTK_YUV_P010:       //420 format 10bit  2 plane (Y)(UV) = P010
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_YVU_P010:       //420 format 10bit  2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        /******************************
         *  2 plane, 16bit
         ******************************/
        case eImgFmt_YUV_P210:           //422 format 16bit  2 plane (Y)(UV) = P210
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_YVU_P210:           //422 format 16bit  2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV422_2P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        case eImgFmt_YUV_P010:           //420 format 16bit  2 plane (Y)(UV) = P010
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_YVU_P010:           //420 format 16bit  2 plane (Y)(VU)
            *pHwYuvoFmt = YUVO_FMT_YUV420_2P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 1;
            break;
        /******************************
         *  3 plane, 8bit
         ******************************/
        case eImgFmt_I422:               //422 format 3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV422_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /*case eImgFmt_YV16:               //422 format 3 plane (Y)(V)(U)
            *pHwYuvoFmt = YUVO_FMT_YUV422_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;*/
        case eImgFmt_I420:               //420 format 3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV420_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /*case eImgFmt_YV12:               //420 format 3 plane (Y)(V)(U)
            *pHwYuvoFmt = YUVO_FMT_YUV420_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 0;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;*/
        /******************************
         *  3 plane, 10bit
         ******************************/
        case eImgFmt_MTK_YUV_P210_3PLANE: //422 format 10bit  3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV422_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_MTK_YUV_P010_3PLANE: //420 format 10bit  3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV420_3P;
            pYuvPak->Bits.PAK_DNG = 0;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        /******************************
         *  3 plane, 16bit
         ******************************/
        case eImgFmt_YUV_P210_3PLANE:    //422 format 16bit  3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV422_3P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        case eImgFmt_YUV_P010_3PLANE:    //420 format 16bit  3 plane (Y)(U)(V)
            *pHwYuvoFmt = YUVO_FMT_YUV420_3P;
            pYuvPak->Bits.PAK_DNG = 1;
            pYuvPak->Bits.PAK_BIT = 1;
            pYuvPak->Bits.PAK_R3_ODR = 0;
            pYuvPak->Bits.PAK_R4_ODR = 0;
            break;
        default:
            ISP_PATH_ERR("eImgFmt:[0x%x]NOT Support PakDNGFmtMapping",imgFmt);
            ret = MFALSE;
    }
    //
    ISP_PATH_DBG("input imgFmt(0x%x),output HwYuvoFmtValue:0x%x",imgFmt, *pHwYuvoFmt);
    return ret;
}

Cam_path_sel::Cam_path_sel(CamPathPass1Parameter* pObj)
{
    m_this = pObj;
    m_hwModule = CAM_MAX;
}

Cam_path_sel::~Cam_path_sel()
{}

MBOOL Cam_path_sel::Path_sel_YUV(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern)
{   // YUV
    MBOOL rst = MTRUE;

    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_HDS_SEL = 0;

    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_AA_SEL = 0;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_AF_SEL = 0;

    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_LCES_SEL = 0;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_BPC_R2_SEL = 1;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_FLK_SEL = 5;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_LTMS_SEL = 0;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_DGN_SEL = 1;

    if(bRawI)
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 2;
    else
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 0;

    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMGO_SEL = 2;

    //CrpR3/IMG_SEL are useless under YUV format
    //CAMCTL_CRP_R1_SEL decide by stt config

    return rst;
}


MBOOL Cam_path_sel::Path_sel_RAW(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat)
{    //bayer
    MBOOL rst = MTRUE;

    if(this->m_this->bypass_rrzo){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_HDS_SEL = 0;
    }else{
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_HDS_SEL = 1;
    }
    if(!isUf(this->m_this->m_rrzo.dmao.lIspColorfmt)){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_UFEG_SEL = 0;
    }
    else{
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_UFEG_SEL = 1;
    }
    if(this->m_this->bypass_yuvo){
        this->m_this->m_top_ctl.CTRL_SEL2.Bits.CAMCTL_DM_R1_SEL = 1;
    }
    else{
        this->m_this->m_top_ctl.CTRL_SEL2.Bits.CAMCTL_DM_R1_SEL = 0;
    }

    switch(data_pat){
        case eCAM_ZVHDR:
        case eCAM_4CELL_ZVHDR:
        case eCAM_DUAL_PIX_ZVHDR:
            this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_DGN_SEL = 0;
            break;
        default:
            this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_DGN_SEL = 1;
            break;
    }

    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_AA_SEL = 1;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_AF_SEL = 1;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_LCES_SEL = 1;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_BPC_R2_SEL = 1;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_FLK_SEL = 0;
    this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_LTMS_SEL = 1;

    if(bRawI){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 2;
    }
    else {
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 0;
    }

    //IMG_SEL + IMGO_SEL + CRPr3_SEL + PDO_SEL
    if(mPureRaw == 1){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMG_SEL = 0;
        if(bPak){
            this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMGO_SEL = 0;
        }else{
            this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMGO_SEL = 2;
        }

        //PDO+CrpR3
        switch(data_pat){
            case eCAM_DUAL_PIX:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 0;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 6;
                break;
            case eCAM_NORMAL:
            case eCAM_NORMAL_PD:
            default:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 1;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 0;
                break;
        }
    }
    else if (mPureRaw == 2){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMG_SEL = 2;
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMGO_SEL = 0;

        //PDO+CrpR3
        switch(data_pat){
            case eCAM_DUAL_PIX:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 0;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMG_SEL = 0;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 6;
                break;
            case eCAM_NORMAL:
            default:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 1;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 3;
                break;
        }
    }
    else{
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMG_SEL = 2;
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMGO_SEL = 0;

        //PDO+CrpR3
        switch(data_pat){
            case eCAM_DUAL_PIX:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 0;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_IMG_SEL = 0;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 6;
                break;
            case eCAM_NORMAL:
            case eCAM_NORMAL_PD:
            default:
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_PDO_SEL = 1;
                this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_CRP_R3_SEL = 1;
                break;
        }
    }

    //CAMCTL_CRP_R1_SEL decide by stt config

    return rst;
}




MBOOL Cam_path_sel::Path_sel(MUINT32 mPureRaw,MBOOL bPak,MBOOL bRawI, E_CamPattern data_pat)
{
    MBOOL rst = MTRUE;

    this->m_this->m_top_ctl.CTRL_SEL.Raw = 0x0;

    if(bRawI){
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 2;

        if(isUf(this->m_this->m_rawi.dmao.lIspColorfmt))
            this->m_this->m_top_ctl.CTRL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL = 1;
        else
            this->m_this->m_top_ctl.CTRL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL = 0;
    }
    else{   // TG IN.
        this->m_this->m_top_ctl.CTRL_SEL.Bits.CAMCTL_RAW_SEL = 0;
    }

    if(this->m_this->bypass_ispRawPipe == MFALSE){    //bayer

        rst = this->Path_sel_RAW(mPureRaw,bPak,bRawI,data_pat);

    }
    else{   // YUV

        rst = this->Path_sel_YUV(mPureRaw,bPak,bRawI,data_pat);
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
    CAM_BUF_CTRL*  pFBC[ePlane_max] = {NULL,NULL,NULL};
    DupCmdQMgr*    pCmdQ = NULL;
    CAM_TIMESTAMP* pTime;
    CAM_BUF_CTRL   fbc;
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
    this->m_enque_YUVO.init();
    this->m_enque_CRZOr1.init();
    this->m_enque_CRZOr2.init();

    this->m_enqRecImgo.init();
    this->m_enqRecRrzo.init();
    this->m_enqRecYuvo.init();
    this->m_enqRecCrzoR1.init();
    this->m_enqRecCrzoR2.init();

    this->m_bUF_imgo.clear();
    this->m_bUF_rrzo.clear();

    //initialize timestamp
    pTime = CAM_TIMESTAMP::getInstance(pCmdQ->CmdQMgr_GetCurModule(),pCmdQ->CmdQMgr_GetCurCycleObj(0)[0]);
    pTime->TimeStamp_SrcClk(pInput->tTimeClk);
    if (this->m_pCamPathImp->m_DcifCtrl.m_pIspDrv != NULL)
        pTime->TimeStamp_SrcSofSel(TSTMP_SRC_EXTERNAL);

    //initialize fbc's timeout method
    fbc.m_fps[this->m_hwModule] = pInput->tgFps;
    for (MUINT32 iii = 0; iii < MAX_RECENT_GRPFRM_TIME; iii++) {
        fbc.m_recentFrmTimeMs[this->m_hwModule][iii] = MIN_GRPFRM_TIME_MS;
    }

    for (it = pchannel->begin(); it != pchannel->end(); it++) {
        pFBC[ePlane_2nd] = NULL;
        pFBC[ePlane_3rd] = NULL;
        ISP_PATH_DBG("PBC_CONFIG:port:%d\n",*it);
        switch (*it) {
        case _imgo_:
            pFBC[ePlane_1st] = &this->m_Imgo_FBC;
            pFBC[ePlane_2nd] = &this->m_Ufeo_FBC;
            break;
        case _rrzo_:
            pFBC[ePlane_1st] = &this->m_Rrzo_FBC;
            pFBC[ePlane_2nd] = &this->m_Ufgo_FBC;
            break;
        case _yuvo_:
            pFBC[ePlane_1st] = &this->m_Yuvo_FBC;
            pFBC[ePlane_2nd] = &this->m_Yuvbo_FBC;
            pFBC[ePlane_3rd] = &this->m_Yuvco_FBC;
            break;
        case _crzo_:
            pFBC[ePlane_1st] = &this->m_CrzoR1_FBC;
            pFBC[ePlane_2nd] = &this->m_CrzboR1_FBC;
            break;
        case _crzo_r2_:
            pFBC[ePlane_1st] = &this->m_CrzoR2_FBC;
            pFBC[ePlane_2nd] = &this->m_CrzboR2_FBC;
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n",*it);
            return MFALSE;
            break;
        }

        pFBC[ePlane_1st]->m_pTimeStamp = pTime;
        pFBC[ePlane_2nd]->m_pTimeStamp = pTime;
        if(pFBC[ePlane_3rd]) pFBC[ePlane_3rd]->m_pTimeStamp = pTime;

        for(MUINT32 i = 0;i<pCmdQ->CmdQMgr_GetDuqQ();i++){
            for(MUINT32 j=0;j<pCmdQ->CmdQMgr_GetBurstQ();j++){
                IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];
                //
                pFBC[ePlane_1st]->m_pIspDrv = _ispDrv;
                pFBC[ePlane_1st]->config();
                pFBC[ePlane_1st]->write2CQ(j);
                pFBC[ePlane_1st]->enable(NULL);
                //
                pFBC[ePlane_2nd]->m_pIspDrv = _ispDrv;
                pFBC[ePlane_2nd]->config();
                pFBC[ePlane_2nd]->write2CQ(j);
                pFBC[ePlane_2nd]->enable(NULL);

                if(pFBC[ePlane_3rd]){
                    pFBC[ePlane_3rd]->m_pIspDrv = _ispDrv;
                    pFBC[ePlane_3rd]->config();
                    pFBC[ePlane_3rd]->write2CQ(j);
                    pFBC[ePlane_3rd]->enable(NULL);
                }

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
    CAM_BUF_CTRL* pFBC[ePlane_max] = {NULL,NULL,NULL};
    DupCmdQMgr* pCmdQ = this->m_pCamPathImp->m_pCmdQdrv;

    if(this->FSM_UPDATE(op_unknown) == MFALSE)
        return MFALSE;


    for (it = this->m_OpenedChannel.begin(); it!=this->m_OpenedChannel.end(); it++) {
        pFBC[ePlane_2nd] = NULL;
        pFBC[ePlane_3rd] = NULL;

        switch(*it){
            case _imgo_:
                pFBC[ePlane_1st] = &this->m_Imgo_FBC;
                pFBC[ePlane_2nd] = &this->m_Ufeo_FBC;
                break;
            case _rrzo_:
                pFBC[ePlane_1st] = &this->m_Rrzo_FBC;
                pFBC[ePlane_2nd] = &this->m_Ufgo_FBC;
                break;
            case _yuvo_:
                pFBC[ePlane_1st] = &this->m_Yuvo_FBC;
                pFBC[ePlane_2nd] = &this->m_Yuvbo_FBC;
                pFBC[ePlane_3rd] = &this->m_Yuvco_FBC;
                break;
            case _crzo_:
                pFBC[ePlane_1st] = &this->m_CrzoR1_FBC;
                pFBC[ePlane_2nd] = &this->m_CrzboR1_FBC;
                break;
            case _crzo_r2_:
                pFBC[ePlane_1st] = &this->m_CrzoR2_FBC;
                pFBC[ePlane_2nd] = &this->m_CrzboR2_FBC;
                break;
            default:
                ISP_PATH_ERR("unsupported dmao:%d\n",*it);
                return MFALSE;
                break;
        }

        for (MUINT32 i = 0; i < pCmdQ->CmdQMgr_GetDuqQ(); i++) {
            for (MUINT32 j = 0; j < pCmdQ->CmdQMgr_GetBurstQ(); j++) {
                IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(i)[j];

                pFBC[ePlane_1st]->m_pIspDrv = _ispDrv;
                pFBC[ePlane_1st]->disable();

                pFBC[ePlane_2nd]->m_pIspDrv = _ispDrv;
                pFBC[ePlane_2nd]->disable();

                if (pFBC[ePlane_3rd]){
                    pFBC[ePlane_3rd]->m_pIspDrv = _ispDrv;
                    pFBC[ePlane_3rd]->disable();
                }
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
    CAM_BUF_CTRL                *pFBC[ePlane_max] = {NULL,NULL,NULL};
    DMA_B                       *pDma[ePlane_max] = {NULL,NULL,NULL};
    DupCmdQMgr                  *pCmdQ = this->m_pCamPathImp->m_pCmdQdrv;
    MBOOL                       bCompressable = MFALSE;
    pTwinVMod;

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        return MFALSE;
    }
    ISP_PATH_DBG("push:dma:%d\n",dmaChannel);
    switch (dmaChannel) {
        case _imgo_:
            pQue = &this->m_enque_IMGO;
            break;
        case _rrzo_:
            pQue = &this->m_enque_RRZO;
            break;
        case _yuvo_:
            pQue = &this->m_enque_YUVO;
            break;
        case _crzo_:
            pQue = &this->m_enque_CRZOr1;
            break;
        case _crzo_r2_:
            pQue = &this->m_enque_CRZOr2;
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
            pFBC[ePlane_2nd] = &this->m_Ufeo_FBC ;
            pDma[ePlane_2nd] = &this->m_pCamPathImp->m_Ufeo ;
            this->m_bUF_imgo.clear();
            bCompressable = MTRUE;
            break;
        case _rrzo_:
            pFBC[ePlane_2nd] = &this->m_Ufgo_FBC ;
            pDma[ePlane_2nd] = &this->m_pCamPathImp->m_Ufgo ;
            this->m_bUF_rrzo.clear();
            bCompressable = MTRUE;
            break;
        case _yuvo_:
            pFBC[ePlane_2nd] = &this->m_Yuvbo_FBC ;
            pDma[ePlane_2nd] = &this->m_pCamPathImp->m_Yuvbo ;
            pFBC[ePlane_3rd] = &this->m_Yuvco_FBC ;
            pDma[ePlane_3rd] = &this->m_pCamPathImp->m_Yuvco ;
            break;
        case _crzo_:
        case _crzo_r2_:
            //No need per-frame enable/disable dma & FBC
            goto EXIT;
            break;
        default:
            ISP_PATH_ERR("unsupported dmao:%d\n",dmaChannel);
            return MFALSE;
    }

    if(pFBC[ePlane_2nd] == NULL || pDma[ePlane_2nd]== NULL){
        ISP_PATH_ERR("dma[0x%x] 2nd plane, pointer is null(0x%x_0x%x)\n",dmaChannel,pFBC[ePlane_2nd],pDma[ePlane_2nd]);
        return MFALSE;
    }

    targetIdx = pCmdQ->CmdQMgr_GetDuqQIdx();

    for (MUINT32 j = 0; j < pCmdQ->CmdQMgr_GetBurstQ(); j++) {
        IspDrvVir *_ispDrv = (IspDrvVir*)pCmdQ->CmdQMgr_GetCurCycleObj(targetIdx)[j];
        //under single path, imgo/rrzo/yuvo dynamic on/off r not supported
        pFBC[ePlane_2nd]->m_pIspDrv = _ispDrv;
        pDma[ePlane_2nd]->m_pIspDrv = _ispDrv;
        if(pFBC[ePlane_3rd]){
            pFBC[ePlane_3rd]->m_pIspDrv = _ispDrv;
            pDma[ePlane_3rd]->m_pIspDrv = _ispDrv;
        }

        if(bCompressable){//ufo case only 2 planes (3rd plane is meta)
            if (bufInfo.front().u_op.enque.size() == 3) {
                pFBC[ePlane_2nd]->enable(NULL);
                pDma[ePlane_2nd]->enable(NULL);
            }
            else {
                pFBC[ePlane_2nd]->disable();
                pDma[ePlane_2nd]->disable();
            }
        } else {//change format dynamically
            if (bufInfo.front().u_op.enque.size() >= 2) {
                pFBC[ePlane_2nd]->enable(NULL);
                pDma[ePlane_2nd]->enable(NULL);
            } else{
                pFBC[ePlane_2nd]->disable();
                pDma[ePlane_2nd]->disable();
            }
            if(pFBC[ePlane_3rd]){
                if (bufInfo.front().u_op.enque.size() == 3) {
                    pFBC[ePlane_3rd]->enable(NULL);
                    pDma[ePlane_3rd]->enable(NULL);
                } else{
                    pFBC[ePlane_3rd]->disable();
                    pDma[ePlane_3rd]->disable();
                }
            }
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
        ISP_PATH_DBG("pop enque:dma:%d\n",*it);
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
            case _yuvo_:
                this->m_enque_YUVO.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_yuvo_);
                else{
                    ret = 1;
                    ISP_PATH_ERR("YUVO have no buffer for enque\n");
                }
                break;
            case _crzo_:
                this->m_enque_CRZOr1.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_crzo_);
                else{
                    ret = 1;
                    ISP_PATH_ERR("CRZO_R1 have no buffer for enque\n");
                }
                break;
            case _crzo_r2_:
                this->m_enque_CRZOr2.sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_getsize,(MUINTPTR)&_size,0);
                if(_size != 0)
                    this->enqueHW(_crzo_r2_);
                else{
                    ret = 1;
                    ISP_PATH_ERR("CRZO_R2 have no buffer for enque\n");
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
            case _yuvo_:
            case _crzo_:
            case _crzo_r2_:
                continue; //skip
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
    CAM_BUF_CTRL* pFBC[ePlane_max] = {NULL,NULL,NULL};
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    BufInfo buf;
    MINT32  ret = 0, deqRet = 0;
    MUINT32 _size = 0;
    MBOOL  bCompressable = MFALSE;

    switch(dmaChannel){
    case _imgo_:
        pFBC[ePlane_1st] = &this->m_Imgo_FBC;
        pFBC[ePlane_2nd] = &this->m_Ufeo_FBC;
        pEnqBufMgr = &this->m_enqRecImgo;
        bCompressable = MTRUE;
        break;
    case _rrzo_:
        pFBC[ePlane_1st] = &this->m_Rrzo_FBC;
        pFBC[ePlane_2nd] = &this->m_Ufgo_FBC;
        pEnqBufMgr = &this->m_enqRecRrzo;
        bCompressable = MTRUE;
        break;
    case _yuvo_:
        pFBC[ePlane_1st] = &this->m_Yuvo_FBC;
        pFBC[ePlane_2nd] = &this->m_Yuvbo_FBC;
        pFBC[ePlane_3rd] = &this->m_Yuvco_FBC;
        pEnqBufMgr = &this->m_enqRecYuvo;
        break;
    case _crzo_:
        pFBC[ePlane_1st] = &this->m_CrzoR1_FBC;
        pFBC[ePlane_2nd] = &this->m_CrzboR1_FBC;
        pEnqBufMgr = &this->m_enqRecCrzoR1;
        break;
    case _crzo_r2_:
        pFBC[ePlane_1st] = &this->m_CrzoR2_FBC;
        pFBC[ePlane_2nd] = &this->m_CrzboR2_FBC;
        pEnqBufMgr = &this->m_enqRecCrzoR2;
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
    switch(pFBC[ePlane_1st]->waitBufReady(pNotify)){
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
        MUINT32 nPlaneNum = 0;

        //can't update this para. , in order to prevent racing condition
        //pFBC->m_pIspDrv = (IspDrvVir*)this->m_pCmdQdrv->CmdQMgr_GetCurCycleObj(this->m_pCmdQdrv->CmdQMgr_GetDuqQIdx())[i];
        switch(pFBC[ePlane_1st]->dequeueHwBuf(buf)) {
        case eCmd_Fail:
            ISP_PATH_ERR("data is rdy on dram,but deque fail at burst:0x%x\n",i);
            this->m_lock.unlock();
            return -1;
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

            nPlaneNum = bufRec.u_op.enque.size();
            if (bCompressable && nPlaneNum == 3) {
                pUfMeta = (UFDG_META_INFO *)(bufRec.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);
            }
        }
        buf.m_pPrivate = (MUINT32 *)pUfMeta;

        if((nPlaneNum > ePlane_max) || !nPlaneNum){
            ISP_PATH_ERR("dmao[0x%x]_%d, plane number error(%d), max:%d\n",dmaChannel,i,nPlaneNum,ePlane_max);
            ret = 1;
            continue;
        }

        if(bCompressable && !buf.bUF_DataFmt){//imgo/rrzo, no ufo
            goto SKIP_DEQ;
        }

        //multi-plane deque (plane2 & plane3)
        for(MUINT32 j=ePlane_2nd; j<nPlaneNum; j++)
        {
            if(pFBC[j] == NULL){
                ISP_PATH_ERR("dma[0x%x] %d_plane, pointer is null\n",dmaChannel,j);
                continue;
            }

            switch(pFBC[j]->dequeueHwBuf(buf)) {
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

            //ufo case, break loop(planeNum)
            if(bCompressable)
                break;
        }

SKIP_DEQ:
        if (buf.bUF_DataFmt) {
            if (pUfMeta) {
                pUfMeta->bUF = MTRUE;
                pUfMeta->UFDG_BOND_MODE = 0;// non-twin in this path
            }

            if (bufRec.u_op.enque.size() > ePlane_3rd) {
                buf.Frame_Header.u4BufPA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.pa_addr;
                buf.Frame_Header.u4BufVA[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.va_addr;
                buf.Frame_Header.u4BufSize[ePlane_3rd] = bufRec.u_op.enque.at(ePlane_3rd).header.size;
                buf.Frame_Header.memID[ePlane_3rd]     = bufRec.u_op.enque.at(ePlane_3rd).header.memID;
                buf.Frame_Header.bufCohe[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufCohe;
                buf.Frame_Header.bufSecu[ePlane_3rd]   = bufRec.u_op.enque.at(ePlane_3rd).header.bufSecu;
            }
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

        bufInfo.push_back(buf);
    }
    this->m_lock.unlock();

    return ret;
}




MINT32 CamPath_BufCtrl::enqueHW(MUINT32 const dmaChannel)
{
    CAM_BUF_CTRL    *pFBC[ePlane_max] = {NULL,NULL,NULL};
    DMAO_B          *pDmao[ePlane_max] = {NULL,NULL,NULL};
    DMAI_B          *pDmai[ePlane_max] = {NULL,NULL,NULL};
    QueueMgr<stISP_BUF_INFO>    *pEnqBufMgr = NULL;
    QueueMgr<ISP_BUF_INFO_L>    *pQue = NULL;

    DupCmdQMgr      *pCQMgr = this->m_pCamPathImp->m_pCmdQdrv;
    ISP_BUF_INFO_L  _buf_list;
    MUINT32         ret = 0, targetIdx = 0;
    vector<MBOOL>   *pUF = NULL;
    MBOOL  bCompressable = MFALSE;//MTRUE: dma is imgo/rrzo. if ufo case, plane#=3 and pUF can't be NULL.

    Mutex::Autolock __lock(this->m_lock);

    if (this->FSM_UPDATE(op_runtwin) == MFALSE) {
        ret = 1;
        goto EXIT;
    }

    switch (dmaChannel) {
    case _imgo_:
        pQue = &this->m_enque_IMGO;
        pEnqBufMgr = &this->m_enqRecImgo;
        //
        pFBC[ePlane_1st] = &this->m_Imgo_FBC;
        pDmao[ePlane_1st] = &this->m_pCamPathImp->m_Imgo;
        pFBC[ePlane_2nd] = &this->m_Ufeo_FBC;
        pDmao[ePlane_2nd] = &this->m_pCamPathImp->m_Ufeo;
        //
        bCompressable = MTRUE;
        pUF = &this->m_bUF_imgo;
        break;
    case _rrzo_:
        pQue = &this->m_enque_RRZO;
        pEnqBufMgr = &this->m_enqRecRrzo;
        //
        pFBC[ePlane_1st] = &this->m_Rrzo_FBC;
        pDmao[ePlane_1st] = &this->m_pCamPathImp->m_Rrzo;
        pFBC[ePlane_2nd] = &this->m_Ufgo_FBC;
        pDmao[ePlane_2nd] = &this->m_pCamPathImp->m_Ufgo;
        //
        bCompressable = MTRUE;
        pUF = &this->m_bUF_rrzo;
        break;
    case _yuvo_:
        pQue = &this->m_enque_YUVO;
        pEnqBufMgr = &this->m_enqRecYuvo;
        //
        pFBC[ePlane_1st] = &this->m_Yuvo_FBC;
        pDmao[ePlane_1st] = &this->m_pCamPathImp->m_Yuvo;
        pFBC[ePlane_2nd] = &this->m_Yuvbo_FBC;
        pDmao[ePlane_2nd] = &this->m_pCamPathImp->m_Yuvbo;
        pFBC[ePlane_3rd] = &this->m_Yuvco_FBC;
        pDmao[ePlane_3rd] = &this->m_pCamPathImp->m_Yuvco;
        break;
    case _crzo_:
        pQue = &this->m_enque_CRZOr1;
        pEnqBufMgr = &this->m_enqRecCrzoR1;
        //
        pFBC[ePlane_1st] = &this->m_CrzoR1_FBC;
        pDmao[ePlane_1st] = &this->m_pCamPathImp->m_CrzoR1;
        pFBC[ePlane_2nd] = &this->m_CrzboR1_FBC;
        pDmao[ePlane_2nd] = &this->m_pCamPathImp->m_CrzboR1;
        break;
    case _crzo_r2_:
        pQue = &this->m_enque_CRZOr2;
        pEnqBufMgr = &this->m_enqRecCrzoR2;
        //
        pFBC[ePlane_1st] = &this->m_CrzoR2_FBC;
        pDmao[ePlane_1st] = &this->m_pCamPathImp->m_CrzoR2;
        pFBC[ePlane_2nd] = &this->m_CrzboR2_FBC;
        pDmao[ePlane_2nd] = &this->m_pCamPathImp->m_CrzboR2;
        break;
    default:
        ISP_PATH_ERR("unsupported dmao:%d\n", dmaChannel);
        ret = 1;
        goto EXIT;
        break;
    }

    if(pQue)
        pQue->sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_front, (MUINTPTR)&_buf_list, 0);

    for (MUINT32 i=0;i<pCQMgr->CmdQMgr_GetBurstQ();i++) {//for subsample
        IspDrvVir       *_pCurPage = NULL;
        stISP_BUF_INFO  _tmp_buf = _buf_list.front();
        MUINT32         nPlaneNum = _tmp_buf.u_op.enque.size();

        targetIdx = pCQMgr->CmdQMgr_GetDuqQIdx();

        _pCurPage = (IspDrvVir*)pCQMgr->CmdQMgr_GetCurCycleObj(targetIdx)[i];

        if((nPlaneNum > ePlane_max) || !nPlaneNum){
            ISP_PATH_ERR("dmao[0x%x]_%d, plane number error(%d), max:%d\n",dmaChannel,i,nPlaneNum,ePlane_max);
            ret = 1;
            goto EXIT;
        }

        pFBC[ePlane_1st]->m_pIspDrv = _pCurPage;
        if(eCmd_Fail == pFBC[ePlane_1st]->enqueueHwBuf(_tmp_buf, MTRUE)){
            ISP_PATH_ERR("enque fail at burst:0x%x\n",i);
            ret = 1;
        }

        if(pDmao[ePlane_1st]){
            if(_tmp_buf.bReplace == MTRUE){
                pDmao[ePlane_1st]->dma_cfg.memBuf.base_pAddr = _tmp_buf.Replace.image.mem_info.pa_addr;
                pDmao[ePlane_1st]->Header_Addr = _tmp_buf.Replace.header.pa_addr;
            }
            else {
                pDmao[ePlane_1st]->dma_cfg.memBuf.base_pAddr = _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
                pDmao[ePlane_1st]->Header_Addr = _tmp_buf.u_op.enque.at(ePlane_1st).header.pa_addr;

                //
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
                        pDmao[ePlane_1st]->Header_Addr = SecPA;
                        ISP_PATH_INF("DMAO:%d Sec FH Addr:0x%x->0x%x E_IMG_PA:0x%x",
                                     dmaChannel,secinfo.buff_va,pDmao[ePlane_1st]->Header_Addr,pDmao[ePlane_1st]->dma_cfg.memBuf.base_pAddr);
                    }else{
                        ISP_PATH_ERR("DMAO:%d Translate FH PA failed",dmaChannel);
                        ret = 1;
                        goto EXIT;
                    }
                }
            }
            pDmao[ePlane_1st]->m_pIspDrv = _pCurPage;
            if (pDmao[ePlane_1st]->setBaseAddr()) {
                ISP_PATH_ERR("dma[0x%x] set addr at burst:0x%x pa:0x%x\n",
                    dmaChannel, i, pDmao[ePlane_1st]->dma_cfg.memBuf.base_pAddr);
                ret = 1;
            }
        }
        else {//pDmai[ePlane_1st]
            pDmai[ePlane_1st]->dma_cfg.memBuf.base_pAddr = _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
            pDmai[ePlane_1st]->m_pIspDrv = _pCurPage;
            if (pDmai[ePlane_1st]->setBaseAddr()) {
                ISP_PATH_ERR("dma[0x%x] set addr at burst:0x%x pa:0x%x\n",
                    dmaChannel, i, pDmai[ePlane_1st]->dma_cfg.memBuf.base_pAddr);
                ret = 1;
            }
        }

        if(bCompressable && (nPlaneNum == 3)){//for ufo case check. if fail, skip enque then push buf into pEnqBufMgr
            if ((pUF == NULL) ||(pUF->size() == 0)){
                ISP_PATH_ERR("UF patch should be NULL(0x%x) or 0",pUF);
                ret = 1;
                goto SKIP_EQ;
            } else if(pUF->at(0) == MTRUE){//off ufo
                goto SKIP_EQ;
            }
        }

        //multi-plane enque (plane2 & plane3)
        for(MUINT32 j=ePlane_2nd;j<nPlaneNum;j++)
        {
            stISP_BUF_INFO  _Buf;
            ST_BUF_INFO stbuf;

            if(pFBC[j] == NULL || (pDmao[j]== NULL && pDmai[j] == NULL)){
                ISP_PATH_ERR("dma[0x%x] %d_plane, pointer is null(0x%x_0x%x_0x%x)\n",dmaChannel,j,pFBC[j],pDmao[j],pDmai[j]);
                continue;
            }


            pFBC[j]->m_pIspDrv = _pCurPage;
            if(pDmao[j]) pDmao[j]->m_pIspDrv = _pCurPage;
            if(pDmai[j]) pDmai[j]->m_pIspDrv = _pCurPage;

            stbuf.image = _tmp_buf.u_op.enque.at(j).image;
            stbuf.header = _tmp_buf.u_op.enque.at(j).header;
            _Buf.u_op.enque.push_back(stbuf);

            if(bCompressable){
                UFDG_META_INFO *pUfMeta = (UFDG_META_INFO *)(_tmp_buf.u_op.enque.at(ePlane_3rd).image.mem_info.va_addr);
                if (pUfMeta) {
                    memset(pUfMeta, 0, sizeof(UFDG_META_INFO));
                }
                else {
                    ISP_PATH_ERR("dma[0x%x] 1st_plane_pa=0x%x, m_pPrivate buf ptr cannot be null!!!\n",
                        dmaChannel, _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr);
                    ret = 1;
                }
            }

            if(eCmd_Fail == pFBC[j]->enqueueHwBuf(_Buf)){
                ISP_PATH_ERR("dma[0x%x] 1st_plane_pa=0x%x,%d_plane_pa=0x%x at burst=%d, enq fail\n",
                    dmaChannel,_tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr,
                    j,_Buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, i);
                ret = 1;
            }

            if(pDmao[j]){
                if (_Buf.bReplace == MTRUE) {
                    pDmao[j]->dma_cfg.memBuf.base_pAddr = _Buf.Replace.image.mem_info.pa_addr;
                    pDmao[j]->Header_Addr = _Buf.Replace.header.pa_addr;
                }
                else {
                    pDmao[j]->dma_cfg.memBuf.base_pAddr = _Buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
                    pDmao[j]->Header_Addr = _Buf.u_op.enque.at(ePlane_1st).header.pa_addr;
                }
                if (pDmao[j]->setBaseAddr()) {
                    ISP_PATH_ERR("dma[0x%x] 1st_plane_pa=0x%x,%d_plane_pa=0x%x at burst=%d\n", dmaChannel,
                        _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, j,_Buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, i);
                    ret = 1;
                }
            } else {//pDmai[j]
                pDmai[j]->dma_cfg.memBuf.base_pAddr = _Buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr;
                if (pDmai[j]->setBaseAddr()) {
                    ISP_PATH_ERR("dma[0x%x] 1st_plane_pa=0x%x,%d_plane_pa=0x%x at burst=%d\n", dmaChannel,
                        _tmp_buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, j,_Buf.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr, i);
                    ret = 1;
                }
            }

            if(bCompressable)//for ufo case, break loop(planeNum)
                break;
        }

SKIP_EQ:
        pEnqBufMgr->sendCmd(QueueMgr<stISP_BUF_INFO>::eCmd_push, (MUINTPTR)&_tmp_buf, 0);
        _buf_list.pop_front();
    }

    if(pQue)
        pQue->sendCmd(QueueMgr<ISP_BUF_INFO_L>::eCmd_pop, 0, 0);

EXIT:

    return ret;
}


