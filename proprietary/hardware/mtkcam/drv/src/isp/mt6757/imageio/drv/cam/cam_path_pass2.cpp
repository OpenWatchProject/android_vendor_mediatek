#define LOG_TAG "iio/pathp2"
//
//
#include "cam_path_dip.h"
#include <utils/Trace.h> //for systrace

//

/*******************************************************************************
*
********************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);

DECLARE_DBG_LOG_VARIABLE(campassp2 );
// Clear previous define, use our own define.
#undef ISP_PATH_VRB
#undef ISP_PATH_DBG
#undef ISP_PATH_INF
#undef ISP_PATH_WRN
#undef ISP_PATH_ERR
#undef ISP_PATH_AST
#define ISP_PATH_VRB(fmt, arg...)        do { if (campassp2_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_PATH_DBG(fmt, arg...)        do { if (campassp2_DbgLogEnable_VERBOSE && campassp2_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_PATH_INF(fmt, arg...)        do { if (campassp2_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_PATH_WRN(fmt, arg...)        do { if (campassp2_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_PATH_ERR(fmt, arg...)        do { if (campassp2_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_PATH_AST(cond, fmt, arg...)  do { if (campassp2_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


// the size only affect tpipe table
#define MAX_TPIPE_WIDTH                  (768)
#define MAX_TPIPE_HEIGHT                 (8192)

// tpipe irq mode
#define TPIPE_IRQ_FRAME     (0)
#define TPIPE_IRQ_LINE      (1)
#define TPIPE_IRQ_TPIPE     (2)


// digital zoom setting
#define DIGITAL_ZOOM_FP_NUM     (10)
#define CDRZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define CDRZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define CURZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/2)
#define CURZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define PRZ_MIN_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define PRZ_MAX_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define BASIC_SCALE_RATIO       (1 << DIGITAL_ZOOM_FP_NUM)

#define CROP_MAX_RATIO      (1000 << DIGITAL_ZOOM_FP_NUM)


#define IS_ERROR_BLOCKING   (0)   // 1:blocking the current thread

#if IS_ERROR_BLOCKING
#define FUNCTION_BLOCKING   do{ISP_PATH_ERR("[Error]blocking ");}while(1);
#else
#define FUNCTION_BLOCKING
#endif


/*******************************************************************************
*
********************************************************************************/
/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass2::config( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;    // 0: success. -1: error.
    MUINT32 cnt;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable, vTable;
    MUINT32 hCoeffStep,vCoeffStep;
    MUINT32 vidoXCropRatio, vidoYCropRatio, vidoCropRatio;
    MUINT32 dispoXCropRatio, dispoYCropRatio, dispoCropRatio;
    //MINT32  p2SubMode;
	MUINT32 tuningRGBTag,tuningYUVTag,tuningYUV2Tag;
    ispMdpPipe.WDMAPQParam = NULL;
    ispMdpPipe.WROTPQParam = NULL;

    ISP_TRACE_CALL();

    ISP_PATH_INF("+,CamPathPass2:tdri(%d),dupCqIdx(%d),burstQueIdx(%d)",p_parameter->tpipe,p_parameter->dupCqIdx,p_parameter->burstQueIdx);

    /*set function List*/
    cnt = 0;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMACQ;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&ispTopCtrl;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&ispRawPipe;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&ispRgbPipe;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&ispYuvPipe;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&crzPipe;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImgi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImgbi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImgci;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAUfdi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMALcei;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAVipi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAVip2i;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAVip3i;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&tdriPipe;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMADepi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMADmgi;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAMfbo;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImg2o;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImg2bo;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImg3o;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImg3bo;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAImg3co;
    m_isp_function_list[cnt++] = (IspFunctionDip_B*)&DMAFeo;
	m_isp_function_list[cnt++] = (IspFunctionDip_B*)&ispMdpPipe;
	m_isp_function_count = cnt;
    //
    //
    //dip_top
    ispTopCtrl.dipTh = p_parameter->dipTh;
    ispTopCtrl.isDipOnly = p_parameter->isDipOnly;
    ispTopCtrl.hwModule = p_parameter->hwModule;
    ispTopCtrl.moduleIdx = p_parameter->moduleIdx;
	
    // C++11 do not allow this kind of operation, use memcpy for workaround
    //ispTopCtrl.isp_top_ctl= p_parameter->isp_top_ctl;
    memcpy((void *) &ispTopCtrl.isp_top_ctl, (void *) &p_parameter->isp_top_ctl, sizeof(ST_DIP_TOP_CTRL));

    ispTopCtrl.dupCqIdx = p_parameter->dupCqIdx;
    ispTopCtrl.burstQueIdx = p_parameter->burstQueIdx;
    ispTopCtrl.isApplyTuning = p_parameter->isApplyTuning;
    //dip_raw
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    if(ispRawPipe.bypass == 0) {

        ispRawPipe.dipTh = p_parameter->dipTh;
        ispRawPipe.hwModule = p_parameter->hwModule;
        ispRawPipe.moduleIdx = p_parameter->moduleIdx;
        ispRawPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispRawPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispRawPipe.rgb_en = p_parameter->isp_top_ctl.RGB_EN.Raw;
        ispRawPipe.isApplyTuning = p_parameter->isApplyTuning;
        ispRawPipe.pTuningIspReg = p_parameter->pTuningIspReg;
        ispRawPipe.enFgMode = p_parameter->isp_top_ctl.FMT_SEL.Bits.FG_MODE;
    }
    //dip_rgb
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    if(ispRgbPipe.bypass == 0) {
        ispRgbPipe.dipTh = p_parameter->dipTh;
        ispRgbPipe.hwModule = p_parameter->hwModule;
        ispRgbPipe.moduleIdx = p_parameter->moduleIdx;
        ispRgbPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispRgbPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispRgbPipe.rgb_en = p_parameter->isp_top_ctl.RGB_EN.Raw;
        ispRgbPipe.isApplyTuning = p_parameter->isApplyTuning;
        ispRgbPipe.pTuningIspReg = p_parameter->pTuningIspReg;
        //
        //lce
        if(MTRUE == p_parameter->isp_top_ctl.RGB_EN.Bits.LCE_EN) {
            ispRgbPipe.lceCfg.in.w = p_parameter->imgi.size.w;
            ispRgbPipe.lceCfg.in.h = p_parameter->imgi.size.h;
            //
            ISP_PATH_DBG("[Lce](w,h)=(%d,%d)",ispRgbPipe.lceCfg.in.w,ispRgbPipe.lceCfg.in.h);
        }

    }
    //dip_yuv
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    if(ispYuvPipe.bypass == 0) {
        ispYuvPipe.dipTh = p_parameter->dipTh;
        ispYuvPipe.hwModule = p_parameter->hwModule;
        ispYuvPipe.moduleIdx = p_parameter->moduleIdx;
        ispYuvPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispYuvPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispYuvPipe.yuv_en = p_parameter->isp_top_ctl.YUV_EN.Raw;
        ispYuvPipe.yuv2_en = p_parameter->isp_top_ctl.YUV2_EN.Raw;
        ispYuvPipe.isApplyTuning = p_parameter->isApplyTuning;
        ispYuvPipe.pTuningIspReg = p_parameter->pTuningIspReg;
        //
        memcpy(&ispYuvPipe.c02Cfg, &p_parameter->c02_cfg, sizeof(C02Cfg));
        memcpy(&ispYuvPipe.c02bCfg, &p_parameter->c02b_cfg, sizeof(C02Cfg));
        memcpy(&ispYuvPipe.mfb_cfg, &p_parameter->mfb_cfg, sizeof(MfbCfg));
        //
        //srz1
        if(MTRUE == p_parameter->isp_top_ctl.YUV_EN.Bits.SRZ1_EN)
        {
            //ctrl
            MUINT32 ctrl=0x3;   //srz1_vert_en=1 & srz1_hori_en=1
            if(p_parameter->srz1_cfg.crop.w>p_parameter->srz1_cfg.inout_size.in_w)
            {
                ctrl |= 0x10;
            }
            p_parameter->srz1_cfg.ctrl=ctrl;
            //update floating offset for tpipe /*20 bit base*/
            p_parameter->srz1_cfg.crop.floatX= \
                ((p_parameter->srz1_cfg.crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            p_parameter->srz1_cfg.crop.floatY= \
                ((p_parameter->srz1_cfg.crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            //coeffienct value for tpipe structure
            p_parameter->srz1_cfg.h_step=(( p_parameter->srz1_cfg.crop.w-1)*32768+((p_parameter->srz1_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz1_cfg.inout_size.out_w-1);
            p_parameter->srz1_cfg.v_step=(( p_parameter->srz1_cfg.crop.h-1)*32768+((p_parameter->srz1_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz1_cfg.inout_size.out_h-1);
            //
            memcpy(&ispYuvPipe.srz1Cfg, &p_parameter->srz1_cfg, sizeof(SrzCfg));

            ISP_PATH_DBG("SRZ1:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%d, %d) out[%d,%d, cof(0x%x/0x%x)] ",p_parameter->srz1_cfg.ctrl, p_parameter->srz1_cfg.inout_size.in_w, p_parameter->srz1_cfg.inout_size.in_h, \
                p_parameter->srz1_cfg.crop.x, p_parameter->srz1_cfg.crop.y, p_parameter->srz1_cfg.crop.floatX, p_parameter->srz1_cfg.crop.floatY, \
                p_parameter->srz1_cfg.crop.w, p_parameter->srz1_cfg.crop.h, p_parameter->srz1_cfg.inout_size.out_w, p_parameter->srz1_cfg.inout_size.out_h,\
                p_parameter->srz1_cfg.h_step, p_parameter->srz1_cfg.v_step);
        }

        //srz2
        if(MTRUE == p_parameter->isp_top_ctl.YUV_EN.Bits.SRZ2_EN)
        {
            //ctrl
            MUINT32 ctrl=0x3;   //srz2_vert_en=1 & srz2_hori_en=1
            if(p_parameter->srz2_cfg.crop.w>p_parameter->srz2_cfg.inout_size.in_w)
            {
                ctrl |= 0x10;
            }
            p_parameter->srz2_cfg.ctrl=ctrl;
            //update floating offset for tpipe /*20 bit base*/
            p_parameter->srz2_cfg.crop.floatX= \
                ((p_parameter->srz2_cfg.crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            p_parameter->srz2_cfg.crop.floatY= \
                ((p_parameter->srz2_cfg.crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            //coeffienct value for tpipe structure
            p_parameter->srz2_cfg.h_step=(( p_parameter->srz2_cfg.crop.w-1)*32768+((p_parameter->srz2_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz2_cfg.inout_size.out_w-1);
            p_parameter->srz2_cfg.v_step=(( p_parameter->srz2_cfg.crop.h-1)*32768+((p_parameter->srz2_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz2_cfg.inout_size.out_h-1);
            //
            memcpy(&ispYuvPipe.srz2Cfg, &p_parameter->srz2_cfg, sizeof(SrzCfg));

            ISP_PATH_DBG("SRZ2:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%d, %d) out[%d,%d, cof(0x%x/0x%x)] ",p_parameter->srz2_cfg.ctrl, p_parameter->srz2_cfg.inout_size.in_w, p_parameter->srz2_cfg.inout_size.in_h, \
                p_parameter->srz2_cfg.crop.x, p_parameter->srz2_cfg.crop.y, p_parameter->srz2_cfg.crop.floatX, p_parameter->srz2_cfg.crop.floatY, \
                p_parameter->srz2_cfg.crop.w, p_parameter->srz2_cfg.crop.h, p_parameter->srz2_cfg.inout_size.out_w, p_parameter->srz2_cfg.inout_size.out_h,\
                p_parameter->srz2_cfg.h_step, p_parameter->srz2_cfg.v_step);
        }

        //srz3
        if(MTRUE == p_parameter->isp_top_ctl.YUV2_EN.Bits.SRZ3_EN)
        {

            //ctrl
            MUINT32 ctrl=0x3;   //srz3_vert_en=1 & srz3_hori_en=1
            if(p_parameter->srz3_cfg.crop.w>p_parameter->srz3_cfg.inout_size.in_w)
            {
                ctrl |= 0x10;
            }
            p_parameter->srz3_cfg.ctrl=ctrl;
            //update floating offset for tpipe /*20 bit base*/
            p_parameter->srz3_cfg.crop.floatX= \
                ((p_parameter->srz3_cfg.crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            p_parameter->srz3_cfg.crop.floatY= \
                ((p_parameter->srz3_cfg.crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            //coeffienct value for tpipe structure
            p_parameter->srz3_cfg.h_step=(( p_parameter->srz3_cfg.crop.w-1)*32768+((p_parameter->srz3_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz3_cfg.inout_size.out_w-1);
            p_parameter->srz3_cfg.v_step=(( p_parameter->srz3_cfg.crop.h-1)*32768+((p_parameter->srz3_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz3_cfg.inout_size.out_h-1);
            //
            memcpy(&ispYuvPipe.srz3Cfg, &p_parameter->srz3_cfg, sizeof(SrzCfg));
            //
            ISP_PATH_DBG("SRZ3:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%d, %d) out[%d,%d, cof(0x%x/0x%x)] ",p_parameter->srz3_cfg.ctrl, p_parameter->srz3_cfg.inout_size.in_w, p_parameter->srz3_cfg.inout_size.in_h, \
                p_parameter->srz3_cfg.crop.x, p_parameter->srz3_cfg.crop.y, p_parameter->srz3_cfg.crop.floatX, p_parameter->srz3_cfg.crop.floatY, \
                p_parameter->srz3_cfg.crop.w, p_parameter->srz3_cfg.crop.h, p_parameter->srz3_cfg.inout_size.out_w, p_parameter->srz3_cfg.inout_size.out_h,\
                p_parameter->srz3_cfg.h_step, p_parameter->srz3_cfg.v_step);
        //
        }

        //srz4
        if(MTRUE == p_parameter->isp_top_ctl.YUV2_EN.Bits.SRZ4_EN)
        {

            //ctrl
            MUINT32 ctrl=0x3;   //srz4_vert_en=1 & srz4_hori_en=1
            if(p_parameter->srz4_cfg.crop.w>p_parameter->srz4_cfg.inout_size.in_w)
            {
                ctrl |= 0x10;
            }
            p_parameter->srz4_cfg.ctrl=ctrl;
            //update floating offset for tpipe /*20 bit base*/
            p_parameter->srz4_cfg.crop.floatX= \
                ((p_parameter->srz4_cfg.crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            p_parameter->srz4_cfg.crop.floatY= \
                ((p_parameter->srz4_cfg.crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
            //coeffienct value for tpipe structure
            p_parameter->srz4_cfg.h_step=(( p_parameter->srz4_cfg.crop.w-1)*32768+((p_parameter->srz4_cfg.inout_size.out_w-1)>>1) )/(p_parameter->srz4_cfg.inout_size.out_w-1);
            p_parameter->srz4_cfg.v_step=(( p_parameter->srz4_cfg.crop.h-1)*32768+((p_parameter->srz4_cfg.inout_size.out_h-1)>>1) )/(p_parameter->srz4_cfg.inout_size.out_h-1);
            //
            memcpy(&ispYuvPipe.srz4Cfg, &p_parameter->srz4_cfg, sizeof(SrzCfg));
            //
            ISP_PATH_DBG("SRZ4:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%d, %d) out[%d,%d, cof(0x%x/0x%x)] ",p_parameter->srz4_cfg.ctrl, p_parameter->srz4_cfg.inout_size.in_w, p_parameter->srz4_cfg.inout_size.in_h, \
                p_parameter->srz4_cfg.crop.x, p_parameter->srz4_cfg.crop.y, p_parameter->srz4_cfg.crop.floatX, p_parameter->srz4_cfg.crop.floatY, \
                p_parameter->srz4_cfg.crop.w, p_parameter->srz4_cfg.crop.h, p_parameter->srz4_cfg.inout_size.out_w, p_parameter->srz4_cfg.inout_size.out_h,\
                p_parameter->srz4_cfg.h_step, p_parameter->srz4_cfg.v_step);
        }


    }
    //dma_cq
    DMACQ.bypass =( ISP_DRV_DIP_CQ_NONE == p_parameter->dipTh )? 1 : 0;
    if(DMACQ.bypass == 0) {
        DMACQ.dipTh = p_parameter->dipTh;
        DMACQ.hwModule = p_parameter->hwModule;
        DMACQ.dupCqIdx = p_parameter->dupCqIdx;
        DMACQ.burstQueIdx = p_parameter->burstQueIdx;
        DMACQ.isApplyTuning = p_parameter->isApplyTuning;
        DMACQ.isDipOnly = p_parameter->isDipOnly;
        DMACQ.dip_cq_thr_ctl = p_parameter->dip_cq_thr_ctl;
    }
    //dma_imgi
    DMAImgi.bypass = ( p_parameter->isp_top_ctl.DMA_EN.Bits.IMGI_EN)? 0 : 1;
    if(DMAImgi.bypass == 0) {
        DMAImgi.dipTh = p_parameter->dipTh;
        DMAImgi.hwModule = p_parameter->hwModule;
        DMAImgi.dupCqIdx = p_parameter->dupCqIdx;
        DMAImgi.burstQueIdx = p_parameter->burstQueIdx;
        DMAImgi.isApplyTuning = p_parameter->isApplyTuning;
        DMAImgi.isDipOnly = p_parameter->isDipOnly;
        DMAImgi.dma_cfg = p_parameter->imgi;
    }
    //dma_imgbi
    DMAImgbi.bypass = ( p_parameter->isp_top_ctl.DMA_EN.Bits.IMGBI_EN)? 0 : 1;
    if(DMAImgbi.bypass == 0) {
        DMAImgbi.dipTh = p_parameter->dipTh;
        DMAImgbi.hwModule = p_parameter->hwModule;
        DMAImgbi.dupCqIdx = p_parameter->dupCqIdx;
        DMAImgbi.burstQueIdx = p_parameter->burstQueIdx;
        DMAImgbi.isApplyTuning = p_parameter->isApplyTuning;
        DMAImgbi.isDipOnly = p_parameter->isDipOnly;
        DMAImgbi.dma_cfg = p_parameter->imgbi;
    }
    //dma_imgci
    DMAImgci.bypass = ( p_parameter->isp_top_ctl.DMA_EN.Bits.IMGCI_EN)? 0 : 1;
    if(DMAImgci.bypass == 0) {
        DMAImgci.dipTh = p_parameter->dipTh;
        DMAImgci.hwModule = p_parameter->hwModule;
        DMAImgci.dupCqIdx = p_parameter->dupCqIdx;
        DMAImgci.burstQueIdx = p_parameter->burstQueIdx;
        DMAImgci.isApplyTuning = p_parameter->isApplyTuning;
        DMAImgci.isDipOnly = p_parameter->isDipOnly;
        DMAImgci.dma_cfg = p_parameter->imgci;
    }
    //dma_ufdi
    DMAUfdi.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.UFDI_EN)? 0 : 1;
    if(DMAUfdi.bypass == 0) {
        DMAUfdi.dipTh = p_parameter->dipTh;
        DMAUfdi.hwModule = p_parameter->hwModule;
        DMAUfdi.dupCqIdx = p_parameter->dupCqIdx;
        DMAUfdi.burstQueIdx = p_parameter->burstQueIdx;
        DMAUfdi.isApplyTuning = p_parameter->isApplyTuning;
        DMAUfdi.isDipOnly = p_parameter->isDipOnly;
        DMAUfdi.dma_cfg = p_parameter->ufdi;
    }
    //dma_lcei
    DMALcei.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.LCEI_EN)? 0 : 1;
    if(DMALcei.bypass == 0) {
        DMALcei.dipTh = p_parameter->dipTh;
        DMALcei.hwModule = p_parameter->hwModule;
        DMALcei.dupCqIdx = p_parameter->dupCqIdx;
        DMALcei.burstQueIdx = p_parameter->burstQueIdx;
        DMALcei.isApplyTuning = p_parameter->isApplyTuning;
        DMALcei.isDipOnly = p_parameter->isDipOnly;
        DMALcei.dma_cfg = p_parameter->lcei;
    }
    //dma_vipi
    DMAVipi.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.VIPI_EN)? 0 : 1;
    if(DMAVipi.bypass == 0) {
        DMAVipi.dipTh = p_parameter->dipTh;
        DMAVipi.hwModule = p_parameter->hwModule;
        DMAVipi.dupCqIdx = p_parameter->dupCqIdx;
        DMAVipi.burstQueIdx = p_parameter->burstQueIdx;
        DMAVipi.isApplyTuning = p_parameter->isApplyTuning;
        DMAVipi.isDipOnly = p_parameter->isDipOnly;
        DMAVipi.dma_cfg = p_parameter->vipi;
    }
    //dma_vip2i
    DMAVip2i.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.VIP2I_EN)? 0 : 1;
    if(DMAVip2i.bypass == 0) {
        DMAVip2i.dipTh = p_parameter->dipTh;
        DMAVip2i.hwModule = p_parameter->hwModule;
        DMAVip2i.dupCqIdx = p_parameter->dupCqIdx;
        DMAVip2i.burstQueIdx = p_parameter->burstQueIdx;
        DMAVip2i.isApplyTuning = p_parameter->isApplyTuning;
        DMAVip2i.isDipOnly = p_parameter->isDipOnly;
        DMAVip2i.dma_cfg = p_parameter->vip2i;
    }
    //dma_vip3i
    DMAVip3i.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.VIP3I_EN)? 0 : 1;
    if(DMAVip3i.bypass == 0) {
        DMAVip3i.dipTh = p_parameter->dipTh;
        DMAVip3i.hwModule = p_parameter->hwModule;
        DMAVip3i.dupCqIdx = p_parameter->dupCqIdx;
        DMAVip3i.burstQueIdx = p_parameter->burstQueIdx;
        DMAVip3i.isApplyTuning = p_parameter->isApplyTuning;
        DMAVip3i.isDipOnly = p_parameter->isDipOnly;
        DMAVip3i.dma_cfg = p_parameter->vip3i;
    }
    //dma_depi
    DMADepi.dipTh = p_parameter->dipTh;
    DMADepi.hwModule = p_parameter->hwModule;
    DMADepi.dupCqIdx = p_parameter->dupCqIdx;
    DMADepi.burstQueIdx = p_parameter->burstQueIdx;
    DMADepi.isApplyTuning = p_parameter->isApplyTuning;
    DMADepi.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.DEPI_EN)? 0 : 1;
    DMADepi.isDipOnly = p_parameter->isDipOnly;
    DMADepi.dma_cfg = p_parameter->depi;
    //dma_dmgi
    DMADmgi.dipTh = p_parameter->dipTh;
    DMADmgi.hwModule = p_parameter->hwModule;
    DMADmgi.dupCqIdx = p_parameter->dupCqIdx;
    DMADmgi.burstQueIdx = p_parameter->burstQueIdx;
    DMADmgi.isApplyTuning = p_parameter->isApplyTuning;
    DMADmgi.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.DMGI_EN)? 0 : 1;
    DMADmgi.isDipOnly = p_parameter->isDipOnly;
    DMADmgi.dma_cfg = p_parameter->dmgi;
    //tdri_pipe
    tdriPipe.bypass = 0;  // tile_edge need to be set in frame mode
    if(tdriPipe.bypass == 0) {
        tdriPipe.dipTh = p_parameter->dipTh;
        tdriPipe.hwModule = p_parameter->hwModule;
        tdriPipe.dupCqIdx = p_parameter->dupCqIdx;
        tdriPipe.burstQueIdx = p_parameter->burstQueIdx;
        tdriPipe.isApplyTuning = p_parameter->isApplyTuning;
        tdriPipe.tdr_ctl = p_parameter->tdr_ctl;
        tdriPipe.tdr_tpipe = p_parameter->tdr_tpipe;
        tdriPipe.tdr_tcm_en = p_parameter->tdr_tcm_en;
        tdriPipe.tdr_tcm2_en = p_parameter->tdr_tcm2_en;
        tdriPipe.enTdri = (p_parameter->tdr_ctl&DIP_X_REG_CTL_TDR_RN)?(1):(0);
        tdriPipe.regCount = p_parameter->regCnt;
        tdriPipe.pReadAddrList = p_parameter->pReadAddrList;
        tdriPipe.pRegiAddr = p_parameter->pRegiAddr;
    }
    //dma_mfbo
    DMAMfbo.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.MFBO_EN)? 0 : 1;
    if(DMAMfbo.bypass == 0) {
        DMAMfbo.dipTh = p_parameter->dipTh;
        DMAMfbo.hwModule = p_parameter->hwModule;
        DMAMfbo.dupCqIdx = p_parameter->dupCqIdx;
        DMAMfbo.burstQueIdx = p_parameter->burstQueIdx;
        DMAMfbo.dma_cfg = p_parameter->mfbo;
    }
    //dma_img2o
    DMAImg2o.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.IMG2O_EN)? 0 : 1;
    if(DMAImg2o.bypass == 0) {
        DMAImg2o.dipTh = p_parameter->dipTh;
        DMAImg2o.hwModule = p_parameter->hwModule;
        DMAImg2o.dupCqIdx = p_parameter->dupCqIdx;
        DMAImg2o.burstQueIdx = p_parameter->burstQueIdx;
        DMAImg2o.dma_cfg = p_parameter->img2o;
    }
    //dma_img3o
    DMAImg3o.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.IMG3O_EN)? 0 : 1;
    if(DMAImg3o.bypass == 0) {
        DMAImg3o.dipTh = p_parameter->dipTh;
        DMAImg3o.hwModule = p_parameter->hwModule;
        DMAImg3o.dupCqIdx = p_parameter->dupCqIdx;
        DMAImg3o.burstQueIdx = p_parameter->burstQueIdx;
        DMAImg3o.dma_cfg = p_parameter->img3o;
    }
    //dma_img3bo
    DMAImg3bo.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.IMG3BO_EN)? 0 : 1;
    if(DMAImg3bo.bypass == 0) {
        DMAImg3bo.dipTh = p_parameter->dipTh;
        DMAImg3bo.hwModule = p_parameter->hwModule;
        DMAImg3bo.dupCqIdx = p_parameter->dupCqIdx;
        DMAImg3bo.burstQueIdx = p_parameter->burstQueIdx;
        DMAImg3bo.dma_cfg = p_parameter->img3bo;
    }
    //dma_img3co
    DMAImg3co.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.IMG3CO_EN)? 0 : 1;
    if(DMAImg3co.bypass == 0) {
        DMAImg3co.dipTh = p_parameter->dipTh;
        DMAImg3co.hwModule = p_parameter->hwModule;
        DMAImg3co.dupCqIdx = p_parameter->dupCqIdx;
        DMAImg3co.burstQueIdx = p_parameter->burstQueIdx;
        DMAImg3co.dma_cfg = p_parameter->img3co;
    }
    //dma_feo
    DMAFeo.bypass = (p_parameter->isp_top_ctl.DMA_EN.Bits.FEO_EN)? 0 : 1;
    if(DMAFeo.bypass == 0) {
        DMAFeo.dipTh = p_parameter->dipTh;
        DMAFeo.hwModule = p_parameter->hwModule;
        DMAFeo.dupCqIdx = p_parameter->dupCqIdx;
        DMAFeo.burstQueIdx = p_parameter->burstQueIdx;
        DMAFeo.dma_cfg = p_parameter->feo;
    }
    //mdp_pipe
    ispMdpPipe.bypass = (p_parameter->isDipOnly==MTRUE)?(1):(0);
    if(ispMdpPipe.bypass == 0) {
        ispMdpPipe.dipTh = p_parameter->dipTh;
        ispMdpPipe.hwModule = p_parameter->hwModule;
        ispMdpPipe.moduleIdx = p_parameter->moduleIdx;
        ispMdpPipe.dupCqIdx = p_parameter->dupCqIdx;
        ispMdpPipe.burstQueIdx = p_parameter->burstQueIdx;
        ispMdpPipe.lastframe = p_parameter->lastframe;
        ispMdpPipe.drvScenario = p_parameter->drvScenario;
        ispMdpPipe.imgi_dma_cfg = p_parameter->imgi;
        #if 0
        ispMdpPipe.src_crop.x = p_parameter->croppara.mdp_crop.x;
        ispMdpPipe.src_crop.floatX = ((p_parameter->croppara.mdp_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.y = p_parameter->croppara.mdp_crop.y;
        ispMdpPipe.src_crop.floatY = ((p_parameter->croppara.mdp_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        ispMdpPipe.src_crop.w = p_parameter->croppara.mdp_crop.w;
        ispMdpPipe.src_crop.h = p_parameter->croppara.mdp_crop.h;
        #endif
        ispMdpPipe.isWaitBuf = p_parameter->isWaitBuf;
        ispMdpPipe.dip_dma_en = p_parameter->isp_top_ctl.DMA_EN.Raw;
        ispMdpPipe.mdp_imgxo_p2_en = p_parameter->mdp_imgxo_p2_en;
        ispMdpPipe.venco_out = p_parameter->venco; //venco
        ispMdpPipe.wroto_out = p_parameter->wroto; //wroto
        ispMdpPipe.wdmao_out = p_parameter->wdmao; //wdmao
        ispMdpPipe.imgxo_out = p_parameter->imgxo; //imgxo
        //
        ispMdpPipe.p2MdpSrcFmt = p_parameter->croppara.mdp_src_fmt;
        ispMdpPipe.p2MdpSrcW = p_parameter->croppara.mdp_Yin.w;
        ispMdpPipe.p2MdpSrcH = p_parameter->croppara.mdp_Yin.h;
        ispMdpPipe.p2MdpSrcYStride = p_parameter->croppara.mdp_Yin.stride;
        ispMdpPipe.p2MdpSrcUVStride = p_parameter->croppara.mdp_UVin.stride;
        ispMdpPipe.p2MdpSrcSize = p_parameter->croppara.mdp_src_size;
        ispMdpPipe.p2MdpSrcCSize = p_parameter->croppara.mdp_src_Csize;
        ispMdpPipe.p2MdpSrcVSize = p_parameter->croppara.mdp_src_Vsize;
        ispMdpPipe.p2MdpSrcPlaneNum = p_parameter->croppara.mdp_src_Plane_Num;
        ispMdpPipe.dipCtlDmaEn = p_parameter->isp_top_ctl.DMA_EN.Raw;
        //
        ispMdpPipe.yuv2_en = p_parameter->isp_top_ctl.YUV2_EN.Raw;
        ispMdpPipe.pTuningIspReg = p_parameter->pTuningIspReg;
        ispMdpPipe.WDMAPQParam = p_parameter->WDMAPQParam;
        ispMdpPipe.WROTPQParam = p_parameter->WROTPQParam;
        //
        ISP_PATH_DBG("[ispMdpPipe]p2MdpSrcFmt(0x%x),w(%d),h(%d),stride(%d,%d),size(%d,%d,%d),planeN(%d)", \
            ispMdpPipe.p2MdpSrcFmt,ispMdpPipe.p2MdpSrcW,ispMdpPipe.p2MdpSrcH,ispMdpPipe.p2MdpSrcYStride,ispMdpPipe.p2MdpSrcUVStride,ispMdpPipe.p2MdpSrcSize,ispMdpPipe.p2MdpSrcCSize,ispMdpPipe.p2MdpSrcVSize,ispMdpPipe.p2MdpSrcPlaneNum);
    }
    //
    //dip_crz
    crzPipe.bypass = 0;
    if(crzPipe.bypass == 0) {
        crzPipe.dipTh = p_parameter->dipTh;
        crzPipe.hwModule = p_parameter->hwModule;
        crzPipe.moduleIdx = p_parameter->moduleIdx;
        crzPipe.dupCqIdx = p_parameter->dupCqIdx;
        crzPipe.burstQueIdx = p_parameter->burstQueIdx;
        crzPipe.isApplyTuning = p_parameter->isApplyTuning;
    	ISP_PATH_DBG("p_parameter->croppara.crz_enable %d",p_parameter->croppara.crz_enable);
    	ISP_PATH_INF("p_parameter->isp_top_ctl.YUV_EN.Bits.CRZ_EN %d/%d",p_parameter->isp_top_ctl.YUV_EN.Bits.CRZ_EN, p_parameter->croppara.crz_enable);
    }

	if (MTRUE == p_parameter->isp_top_ctl.YUV_EN.Bits.CRZ_EN)
	{
		ISP_PATH_DBG("p_parameter->croppara.crz_enable");
		crzPipe.conf_cdrz = 1;
		crzPipe.crz_out = p_parameter->croppara.crz_out;
		crzPipe.crz_in = p_parameter->croppara.crz_in;
		crzPipe.crz_crop.x = p_parameter->croppara.crz_crop.x;
		crzPipe.crz_crop.floatX = p_parameter->croppara.crz_crop.floatX;
		crzPipe.crz_crop.y = p_parameter->croppara.crz_crop.y;
		crzPipe.crz_crop.floatY = p_parameter->croppara.crz_crop.floatY;
		crzPipe.crz_crop.w = p_parameter->croppara.crz_crop.w;
		crzPipe.crz_crop.h = p_parameter->croppara.crz_crop.h;
        	//
        	// for sl2e tuning
        	if(p_parameter->isSl2eAheadCrz==MTRUE){
          	 ispYuvPipe.CrzRatio = 1;
        	} else {
        	    // use height to avoid alignment issue
       	    	ispYuvPipe.CrzRatio = (MFLOAT)crzPipe.crz_out.h/(MFLOAT)crzPipe.crz_crop.h;
       	 	}
	} else {
		crzPipe.conf_cdrz = 0;
        	ispYuvPipe.CrzRatio = 1; // for sl2e tuning
    }
    //crz set for tpipe input
    p_parameter->crz.crz_input_crop_width = crzPipe.crz_crop.w;
    p_parameter->crz.crz_input_crop_height = crzPipe.crz_crop.h;
    p_parameter->crz.crz_output_width = crzPipe.crz_out.w;
    p_parameter->crz.crz_output_height = crzPipe.crz_out.h;
    p_parameter->crz.crz_horizontal_integer_offset = crzPipe.crz_crop.x;/* pixel base */
    p_parameter->crz.crz_horizontal_subpixel_offset = \
        ((crzPipe.crz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
    p_parameter->crz.crz_vertical_integer_offset = crzPipe.crz_crop.y;/* pixel base */
    p_parameter->crz.crz_vertical_subpixel_offset = \
        ((crzPipe.crz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */

    ISP_PATH_DBG("CRZ:crz_vertical_integer_offset(%u),crz_vertical_subpixel_offset(%u) ",p_parameter->crz.crz_vertical_integer_offset,p_parameter->crz.crz_vertical_subpixel_offset);
    ISP_PATH_DBG("CRZ:crz_horizontal_integer_offset(%u),crrz_horizontal_subpixel_offset(%u) ",p_parameter->crz.crz_horizontal_integer_offset,p_parameter->crz.crz_horizontal_subpixel_offset);


    Result = crzPipe.CalAlgoAndCStep((CRZ_DRV_MODE_ENUM)crzPipe.tpipeMode,
                                                          CRZ_DRV_RZ_CRZ,
                                                          crzPipe.crz_in.w,
                                                          crzPipe.crz_in.h,
                                                          crzPipe.crz_crop.w,
                                                          crzPipe.crz_crop.h,
                                                          crzPipe.crz_out.w,
                                                          crzPipe.crz_out.h,
                                                          (CRZ_DRV_ALGO_ENUM*)&hAlgo,
                                                          (CRZ_DRV_ALGO_ENUM*)&vAlgo,
                                                          &hTable,
                                                          &vTable,
                                                          &hCoeffStep,
                                                          &vCoeffStep);
    if(!Result)
    {
        ISP_PATH_ERR(" crzPipe.CalAlgoAndCStep error");
        return Result;
    }


    p_parameter->crz.crz_horizontal_luma_algorithm = hAlgo;
    p_parameter->crz.crz_vertical_luma_algorithm = vAlgo;
    p_parameter->crz.crz_horizontal_coeff_step = hCoeffStep;
    p_parameter->crz.crz_vertical_coeff_step = vCoeffStep;


    ISP_PATH_DBG("CDRZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x, 0x%x) out[%d,%d] ",crzPipe.crz_in.w,crzPipe.crz_in.h, \
            crzPipe.crz_crop.x,crzPipe.crz_crop.y, crzPipe.crz_crop.w,crzPipe.crz_crop.h, \
            crzPipe.crz_crop.floatX,crzPipe.crz_crop.floatY,crzPipe.crz_out.w,crzPipe.crz_out.h);
    ISP_PATH_DBG("hCoeffStep(%d) vCoeffStep(%d) ",hCoeffStep,vCoeffStep);

//SL2E
    ISP_PATH_DBG("SL2E");
    if(p_parameter->pTuningIspReg != NULL){
		p_parameter->pTuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP = p_parameter->pTuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP;
		p_parameter->pTuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP = p_parameter->pTuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP;
    }

    //
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        this->configTpipeData(p_parameter);
    } else {
        // do nothing
    }

    ret = this->_config((void*)p_parameter);
    ISP_PATH_DBG("CamPathPass2:,hwModule(%d),tdri(%d),fdmode(%d/%d),dupCqIdx(%d),burstQueIdx(%d)",p_parameter->hwModule,p_parameter->tpipe,ispRawPipe.enFgMode,p_parameter->isp_top_ctl.FMT_SEL.Bits.FG_MODE,\
        p_parameter->dupCqIdx,p_parameter->burstQueIdx);

EXIT:

    ISP_PATH_DBG("-");
    return ret;
}



int CamPathPass2::configTpipeData( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;

    DIP_X_REG_CTL_YUV_EN *pYuvEn;
    DIP_X_REG_CTL_YUV2_EN *pYuv2En;
    DIP_X_REG_CTL_RGB_EN *pRgbEn;
    DIP_X_REG_CTL_DMA_EN *pDmaEn;
    DIP_X_REG_CTL_FMT_SEL *pFmtSel;
    DIP_X_REG_CTL_PATH_SEL *pPathSel;
    DIP_X_REG_CTL_MISC_SEL *pMiscSel;
    DIP_X_REG_CTL_TDR_CTL *pTdrCtl;
    dip_x_reg_t *tuningIspReg;
    dip_x_reg_t tmpTuningIspReg;
    //
    pYuvEn = (DIP_X_REG_CTL_YUV_EN*)&(p_parameter->isp_top_ctl.YUV_EN);
    pYuv2En = (DIP_X_REG_CTL_YUV2_EN*)&(p_parameter->isp_top_ctl.YUV2_EN);
    pRgbEn = (DIP_X_REG_CTL_RGB_EN*)&(p_parameter->isp_top_ctl.RGB_EN);
    pDmaEn = (DIP_X_REG_CTL_DMA_EN*)&(p_parameter->isp_top_ctl.DMA_EN);
    pFmtSel = (DIP_X_REG_CTL_FMT_SEL*)&(p_parameter->isp_top_ctl.FMT_SEL);
    pPathSel = (DIP_X_REG_CTL_PATH_SEL*)&(p_parameter->isp_top_ctl.PATH_SEL);
    pMiscSel = (DIP_X_REG_CTL_MISC_SEL*)&(p_parameter->isp_top_ctl.MISC_EN);
    pTdrCtl = (DIP_X_REG_CTL_TDR_CTL*)&(p_parameter->tdr_ctl);
    if((p_parameter->isApplyTuning==MTRUE) && (p_parameter->pTuningIspReg != NULL)) {
        tuningIspReg = p_parameter->pTuningIspReg;
    } else {
        memset((void*)&tmpTuningIspReg, 0x00, sizeof(dip_x_reg_t));
        tuningIspReg = &tmpTuningIspReg;
    }
    ISP_PATH_DBG("pYuvEn(0x%x)",pYuvEn);
    //
    tdriPipe.tdri.top.pixel_id = pFmtSel->Bits.PIX_ID;
    tdriPipe.tdri.top.cam_in_fmt = pFmtSel->Bits.IMGI_FMT;
    tdriPipe.tdri.top.ctl_extension_en = pTdrCtl->Bits.CTL_EXTENSION_EN;
    tdriPipe.tdri.top.fg_mode = pFmtSel->Bits.FG_MODE;
    tdriPipe.tdri.top.ufo_imgi_en = pRgbEn->Bits.UFD_EN;
    tdriPipe.tdri.top.ufdi_fmt = pFmtSel->Bits.UFDI_FMT;
    tdriPipe.tdri.top.vipi_fmt = pFmtSel->Bits.VIPI_FMT;
    tdriPipe.tdri.top.img3o_fmt = pFmtSel->Bits.IMG3O_FMT;
    tdriPipe.tdri.top.img2o_fmt = pFmtSel->Bits.IMG2O_FMT;
    tdriPipe.tdri.top.pak2_fmt = pMiscSel->Bits.PAK2_FMT;
    tdriPipe.tdri.top.imgi_en = pDmaEn->Bits.IMGI_EN;
    tdriPipe.tdri.top.imgbi_en = pDmaEn->Bits.IMGBI_EN;
    tdriPipe.tdri.top.imgci_en = pDmaEn->Bits.IMGCI_EN;
    tdriPipe.tdri.top.ufdi_en = pDmaEn->Bits.UFDI_EN;
    tdriPipe.tdri.top.unp_en = pRgbEn->Bits.UNP_EN;
    tdriPipe.tdri.top.ufd_en = pRgbEn->Bits.UFD_EN;
    tdriPipe.tdri.top.ufd_crop_en = 0; // Paul will remove it later.
    tdriPipe.tdri.top.bnr_en = pRgbEn->Bits.BNR2_EN;
    tdriPipe.tdri.top.lsc2_en = pRgbEn->Bits.LSC2_EN;
    tdriPipe.tdri.top.rcp2_en = pRgbEn->Bits.RCP2_EN;
    tdriPipe.tdri.top.pak2_en = pRgbEn->Bits.PAK2_EN;
    tdriPipe.tdri.top.c24_en = pYuvEn->Bits.C24_EN;
    tdriPipe.tdri.top.sl2_en = pRgbEn->Bits.SL2_EN;
    tdriPipe.tdri.top.udm_en = pRgbEn->Bits.UDM_EN;
    tdriPipe.tdri.top.vipi_en = pDmaEn->Bits.VIPI_EN;
    tdriPipe.tdri.top.vip2i_en = pDmaEn->Bits.VIP2I_EN;
    tdriPipe.tdri.top.vip3i_en = pDmaEn->Bits.VIP3I_EN;
    tdriPipe.tdri.top.mfb_en = pYuvEn->Bits.MFB_EN;
    tdriPipe.tdri.top.mfbw_en = pYuvEn->Bits.MFBW_EN;
    tdriPipe.tdri.top.mfbo_en = pDmaEn->Bits.MFBO_EN;
    tdriPipe.tdri.top.g2c_en = pYuvEn->Bits.G2C_EN;
    tdriPipe.tdri.top.c42_en = pYuvEn->Bits.C42_EN;
    tdriPipe.tdri.top.sl2b_en = pYuvEn->Bits.SL2B_EN;
    tdriPipe.tdri.top.nbc_en = pYuvEn->Bits.NBC_EN;
    tdriPipe.tdri.top.dmgi_en = pDmaEn->Bits.DMGI_EN;
    tdriPipe.tdri.top.depi_en = pDmaEn->Bits.DEPI_EN;
    tdriPipe.tdri.top.nbc2_en = pYuvEn->Bits.NBC2_EN;
    tdriPipe.tdri.top.srz1_en = pYuvEn->Bits.SRZ1_EN;
    tdriPipe.tdri.top.mix1_en = pYuvEn->Bits.MIX1_EN;
    tdriPipe.tdri.top.srz2_en = pYuvEn->Bits.SRZ2_EN;
    tdriPipe.tdri.top.mix2_en = pYuvEn->Bits.MIX2_EN;
    tdriPipe.tdri.top.sl2c_en = pYuvEn->Bits.SL2C_EN;
    tdriPipe.tdri.top.sl2d_en = pYuvEn->Bits.SL2D_EN;
    tdriPipe.tdri.top.sl2e_en = pYuvEn->Bits.SL2E_EN;
    tdriPipe.tdri.top.seee_en = pYuvEn->Bits.SEEE_EN;
    tdriPipe.tdri.top.lcei_en = pDmaEn->Bits.LCEI_EN;
    tdriPipe.tdri.top.lce_en = pRgbEn->Bits.LCE_EN;
    tdriPipe.tdri.top.mix3_en = pYuvEn->Bits.MIX3_EN;
    tdriPipe.tdri.top.crz_en = pYuvEn->Bits.CRZ_EN;
    tdriPipe.tdri.top.img2o_en = pDmaEn->Bits.IMG2O_EN;
    tdriPipe.tdri.top.img2bo_en = pDmaEn->Bits.IMG2BO_EN;
    tdriPipe.tdri.top.fe_en = pYuvEn->Bits.FE_EN;
    tdriPipe.tdri.top.feo_en = pDmaEn->Bits.FEO_EN;
    tdriPipe.tdri.top.c02_en = pYuvEn->Bits.C02_EN;
    tdriPipe.tdri.top.c02b_en = pYuvEn->Bits.C02B_EN;
    tdriPipe.tdri.top.nr3d_en = pYuvEn->Bits.NR3D_EN;
    tdriPipe.tdri.top.crsp_en = pYuvEn->Bits.CRSP_EN;
    tdriPipe.tdri.top.img3o_en = pDmaEn->Bits.IMG3O_EN;
    tdriPipe.tdri.top.img3bo_en = pDmaEn->Bits.IMG3BO_EN;
    tdriPipe.tdri.top.img3co_en = pDmaEn->Bits.IMG3CO_EN;
    tdriPipe.tdri.top.c24b_en = pYuvEn->Bits.C24B_EN;
    tdriPipe.tdri.top.mdp_crop_en = pYuvEn->Bits.MDPCROP_EN;
    tdriPipe.tdri.top.plnr1_en = pYuvEn->Bits.PLNR1_EN;
    tdriPipe.tdri.top.plnr2_en = pYuvEn->Bits.PLNR2_EN;
    tdriPipe.tdri.top.plnw1_en = pYuvEn->Bits.PLNW1_EN;
    tdriPipe.tdri.top.plnw2_en = pYuvEn->Bits.PLNW2_EN;
    tdriPipe.tdri.top.dbs2_en = pRgbEn->Bits.DBS2_EN;
    tdriPipe.tdri.top.obc2_en = pRgbEn->Bits.OBC2_EN;
    tdriPipe.tdri.top.rmm2_en = pRgbEn->Bits.RMM2_EN;
    tdriPipe.tdri.top.rmg2_en = pRgbEn->Bits.RMG2_EN;
    tdriPipe.tdri.top.gdr1_en = pRgbEn->Bits.GDR1_EN;
    tdriPipe.tdri.top.gdr2_en = pRgbEn->Bits.GDR2_EN;
    tdriPipe.tdri.top.bnr2_en = pRgbEn->Bits.BNR2_EN;
    tdriPipe.tdri.top.fm_en = pYuv2En->Bits.FM_EN;
    tdriPipe.tdri.top.srz3_en = pYuv2En->Bits.SRZ3_EN;
    tdriPipe.tdri.top.srz4_en = pYuv2En->Bits.SRZ4_EN;
    tdriPipe.tdri.top.interlace_mode = 0; //INTERLACE_MODE
    tdriPipe.tdri.top.pgn_sel = pPathSel->Bits.PGN_SEL;
    tdriPipe.tdri.top.g2g_sel = pPathSel->Bits.G2G_SEL;
    tdriPipe.tdri.top.g2c_sel = pPathSel->Bits.G2C_SEL;
    tdriPipe.tdri.top.srz1_sel = pPathSel->Bits.SRZ1_SEL;
    tdriPipe.tdri.top.mix1_sel = pPathSel->Bits.MIX1_SEL;
    tdriPipe.tdri.top.crz_sel = pPathSel->Bits.CRZ_SEL;
    tdriPipe.tdri.top.nr3d_sel = pPathSel->Bits.NR3D_SEL;
    tdriPipe.tdri.top.fe_sel = pPathSel->Bits.FE_SEL;
    tdriPipe.tdri.top.mdp_sel = pPathSel->Bits.MDP_SEL;
    tdriPipe.tdri.top.pca_en = pYuvEn->Bits.PCA_EN;
    tdriPipe.tdri.top.pgn_en = pRgbEn->Bits.PGN_EN;
    tdriPipe.tdri.top.g2g_en = pRgbEn->Bits.G2G_EN;
    tdriPipe.tdri.top.ggm_en = pRgbEn->Bits.GGM_EN;
    tdriPipe.tdri.top.nbcw_sel = pPathSel->Bits.NBCW_SEL;
    tdriPipe.tdri.top.nbc_sel = pPathSel->Bits.NBC_SEL;
    tdriPipe.tdri.top.rcp2_sel = pPathSel->Bits.RCP2_SEL;
    tdriPipe.tdri.top.mfbo_sel = pMiscSel->Bits.MFBO_SEL;
    tdriPipe.tdri.top.rnr_en = pRgbEn->Bits.RNR_EN;
    tdriPipe.tdri.top.sl2g_en = pRgbEn->Bits.SL2G_EN;
    tdriPipe.tdri.top.sl2h_en = pRgbEn->Bits.SL2H_EN;
    //tdriPipe.tdri.top.srz5_en = pYuv2En->Bits.SRZ5_EN;	// For this chip's E2 version
    tdriPipe.tdri.top.hfg_en = pYuv2En->Bits.HFG_EN;
    tdriPipe.tdri.top.sl2i_en = pYuv2En->Bits.SL2I_EN;
    tdriPipe.tdri.top.crsp_sel = pPathSel->Bits.CRSP_SEL;
    tdriPipe.tdri.top.feo_sel = pMiscSel->Bits.FEO_SEL;
    //
    tdriPipe.tdri.sw.log_en = 1;
    tdriPipe.tdri.sw.src_width = DMAImgi.dma_cfg.size.w;
    tdriPipe.tdri.sw.src_height = DMAImgi.dma_cfg.size.h;
    tdriPipe.tdri.sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE;
    tdriPipe.tdri.sw.tpipe_width = MAX_TPIPE_WIDTH;
    tdriPipe.tdri.sw.tpipe_height = MAX_TPIPE_HEIGHT;
    tdriPipe.tdri.sw.tpipe_sel_mode = 0x1; /* Paul suggested setting 1 for this */
    //
    tdriPipe.tdri.imgi.imgi_v_flip_en = DMAImgi.dma_cfg.v_flip_en;
    tdriPipe.tdri.imgi.imgi_stride = DMAImgi.dma_cfg.size.stride;
    //
    tdriPipe.tdri.imgbi.imgbi_v_flip_en = DMAImgbi.dma_cfg.v_flip_en;
    tdriPipe.tdri.imgbi.imgbi_stride = DMAImgbi.dma_cfg.size.stride;
    //
    tdriPipe.tdri.imgci.imgci_v_flip_en = DMAImgci.dma_cfg.v_flip_en;
    tdriPipe.tdri.imgci.imgci_stride = DMAImgci.dma_cfg.size.stride;
    //
    if(tdriPipe.tdri.top.ufdi_en)
    {
        tdriPipe.tdri.ufdi.ufdi_v_flip_en = DMAUfdi.dma_cfg.v_flip_en;
        tdriPipe.tdri.ufdi.ufdi_xsize = DMAUfdi.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.ufdi.ufdi_ysize = DMAUfdi.dma_cfg.size.h - 1;
        tdriPipe.tdri.ufdi.ufdi_stride = DMAUfdi.dma_cfg.size.stride;
    }
    else
    {
        tdriPipe.tdri.ufdi.ufdi_v_flip_en = 0;
        tdriPipe.tdri.ufdi.ufdi_xsize = 0;
        tdriPipe.tdri.ufdi.ufdi_ysize = 0;
        tdriPipe.tdri.ufdi.ufdi_stride = 0;
    }
    //
    tdriPipe.tdri.bnr.bpc_en = tuningIspReg->DIP_X_BNR2_BPC_CON.Bits.BPC_EN;
    tdriPipe.tdri.bnr.bpc_tbl_en = tuningIspReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN;
    //
    tdriPipe.tdri.rmg.rmg_ihdr_en = tuningIspReg->DIP_X_RMG2_HDR_CFG.Bits.RMG_IHDR_EN;
    tdriPipe.tdri.rmg.rmg_zhdr_en = tuningIspReg->DIP_X_RMG2_HDR_CFG.Bits.RMG_ZHDR_EN;
    //
    tdriPipe.tdri.lsc2.extend_coef_mode= tuningIspReg->DIP_X_LSC2_CTL1.Bits.LSC_EXTEND_COEF_MODE;
    tdriPipe.tdri.lsc2.sdblk_width = tuningIspReg->DIP_X_LSC2_CTL2.Bits.LSC_SDBLK_WIDTH;
    tdriPipe.tdri.lsc2.sdblk_xnum = tuningIspReg->DIP_X_LSC2_CTL2.Bits.LSC_SDBLK_XNUM;
    tdriPipe.tdri.lsc2.sdblk_last_width = tuningIspReg->DIP_X_LSC2_LBLOCK.Bits.LSC_SDBLK_lWIDTH;
    tdriPipe.tdri.lsc2.sdblk_height = tuningIspReg->DIP_X_LSC2_CTL3.Bits.LSC_SDBLK_HEIGHT;
    tdriPipe.tdri.lsc2.sdblk_ynum = tuningIspReg->DIP_X_LSC2_CTL3.Bits.LSC_SDBLK_YNUM;
    tdriPipe.tdri.lsc2.sdblk_last_height = tuningIspReg->DIP_X_LSC2_LBLOCK.Bits.LSC_SDBLK_lHEIGHT;
    //
    tdriPipe.tdri.sl2.sl2_hrz_comp = tuningIspReg->DIP_X_SL2_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2.sl2_vrz_comp = tuningIspReg->DIP_X_SL2_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.udm.bayer_bypass = tuningIspReg->DIP_X_UDM_INTP_CRS.Bits.UDM_BYP;
    //
    if(tdriPipe.tdri.top.vipi_en)
    {
        tdriPipe.tdri.vipi.vipi_v_flip_en = DMAVipi.dma_cfg.v_flip_en;
        tdriPipe.tdri.vipi.vipi_xsize = DMAVipi.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.vipi.vipi_ysize = DMAVipi.dma_cfg.size.h - 1;
        tdriPipe.tdri.vipi.vipi_stride = DMAVipi.dma_cfg.size.stride;
    }
    else
    {
        tdriPipe.tdri.vipi.vipi_v_flip_en = 0;
        tdriPipe.tdri.vipi.vipi_xsize = 0;
        tdriPipe.tdri.vipi.vipi_ysize = 0;
        tdriPipe.tdri.vipi.vipi_stride = 0;
    }
    //
    if(tdriPipe.tdri.top.vip2i_en)
    {
        tdriPipe.tdri.vip2i.vip2i_v_flip_en = DMAVip2i.dma_cfg.v_flip_en;
        tdriPipe.tdri.vip2i.vip2i_xsize = DMAVip2i.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.vip2i.vip2i_ysize = DMAVip2i.dma_cfg.size.h - 1;
        tdriPipe.tdri.vip2i.vip2i_stride = DMAVip2i.dma_cfg.size.stride;
    }
    else
    {
        tdriPipe.tdri.vip2i.vip2i_v_flip_en = 0;
        tdriPipe.tdri.vip2i.vip2i_xsize = 0;
        tdriPipe.tdri.vip2i.vip2i_ysize = 0;
        tdriPipe.tdri.vip2i.vip2i_stride = 0;
    }
    //
    if(tdriPipe.tdri.top.vip3i_en)
    {
        tdriPipe.tdri.vip3i.vip3i_v_flip_en = DMAVip3i.dma_cfg.v_flip_en;
        tdriPipe.tdri.vip3i.vip3i_xsize = DMAVip3i.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.vip3i.vip3i_ysize = DMAVip3i.dma_cfg.size.h - 1;
        tdriPipe.tdri.vip3i.vip3i_stride = DMAVip3i.dma_cfg.size.stride;
    }
    else
    {
        tdriPipe.tdri.vip3i.vip3i_v_flip_en = 0;
        tdriPipe.tdri.vip3i.vip3i_xsize = 0;
        tdriPipe.tdri.vip3i.vip3i_ysize = 0;
        tdriPipe.tdri.vip3i.vip3i_stride = 0;
    }
    //
    tdriPipe.tdri.mfb.bld_deblock_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_DB_EN;
    tdriPipe.tdri.mfb.bld_brz_en = tuningIspReg->DIP_X_MFB_CON.Bits.BLD_LL_BRZ_EN;
    //
    if(tdriPipe.tdri.top.mfbo_en)
    {
        tdriPipe.tdri.mfbo.mfbo_stride = DMAMfbo.dma_cfg.size.stride;
        tdriPipe.tdri.mfbo.mfbo_xoffset = DMAMfbo.dma_cfg.offset.x;
        tdriPipe.tdri.mfbo.mfbo_yoffset = DMAMfbo.dma_cfg.offset.y;
        tdriPipe.tdri.mfbo.mfbo_xsize = DMAMfbo.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.mfbo.mfbo_ysize = DMAMfbo.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.mfbo.mfbo_stride = 0;
        tdriPipe.tdri.mfbo.mfbo_xoffset = 0;
        tdriPipe.tdri.mfbo.mfbo_yoffset = 0;
        tdriPipe.tdri.mfbo.mfbo_xsize = 0;
        tdriPipe.tdri.mfbo.mfbo_ysize = 0;
    }
    //
    tdriPipe.tdri.g2c.g2c_shade_en = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_EN;
    tdriPipe.tdri.g2c.g2c_shade_xmid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_XMID;
    tdriPipe.tdri.g2c.g2c_shade_ymid = tuningIspReg->DIP_X_G2C_SHADE_TAR.Bits.G2C_SHADE_YMID;
    tdriPipe.tdri.g2c.g2c_shade_var = tuningIspReg->DIP_X_G2C_SHADE_CON_1.Bits.G2C_SHADE_VAR;
    //
    tdriPipe.tdri.sl2b.sl2b_hrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2b.sl2b_vrz_comp = tuningIspReg->DIP_X_SL2B_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.nbc.anr_eny = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_ENY;
    tdriPipe.tdri.nbc.anr_enc = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_ENC;
    tdriPipe.tdri.nbc.anr_scale_mode = tuningIspReg->DIP_X_ANR_CON1.Bits.ANR_SCALE_MODE;
    //
    tdriPipe.tdri.nbc2.anr2_eny = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_ENY;
    tdriPipe.tdri.nbc2.anr2_enc = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_ENC;
    tdriPipe.tdri.nbc2.anr2_scale_mode = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_SCALE_MODE;
    tdriPipe.tdri.nbc2.anr2_mode = tuningIspReg->DIP_X_ANR2_CON1.Bits.ANR2_MODE;
    tdriPipe.tdri.nbc2.anr2_bok_mode = tuningIspReg->DIP_X_BOK_CON.Bits.BOK_MODE;
    tdriPipe.tdri.nbc2.anr2_bok_pf_en = tuningIspReg->DIP_X_BOK_CON.Bits.BOK_PF_EN;
    tdriPipe.tdri.nbc2.abf_en = tuningIspReg->DIP_X_ABF_CON1.Bits.ABF_EN;
    //
    tdriPipe.tdri.sl2c.sl2c_hrz_comp = tuningIspReg->DIP_X_SL2C_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2c.sl2c_vrz_comp = tuningIspReg->DIP_X_SL2C_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.sl2d.sl2d_hrz_comp = tuningIspReg->DIP_X_SL2D_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2d.sl2d_vrz_comp = tuningIspReg->DIP_X_SL2D_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.sl2e.sl2e_hrz_comp = tuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2e.sl2e_vrz_comp = tuningIspReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.sl2g.sl2g_hrz_comp = tuningIspReg->DIP_X_SL2G_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2g.sl2g_vrz_comp = tuningIspReg->DIP_X_SL2G_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.sl2h.sl2h_hrz_comp = tuningIspReg->DIP_X_SL2H_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2h.sl2h_vrz_comp = tuningIspReg->DIP_X_SL2H_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.sl2i.sl2i_hrz_comp = tuningIspReg->DIP_X_SL2I_RZ.Bits.SL2_HRZ_COMP;
    tdriPipe.tdri.sl2i.sl2i_vrz_comp = tuningIspReg->DIP_X_SL2I_RZ.Bits.SL2_VRZ_COMP;
    //
    tdriPipe.tdri.hfg.non_block_base = 0;	// need check with DE (It's cmodel parameter, don't care.)
    //
    tdriPipe.tdri.seee.se_edge = tuningIspReg->DIP_X_SEEE_OUT_EDGE_CTRL.Bits.SEEE_OUT_EDGE_SEL;
    tdriPipe.tdri.seee.itune_see_method = 2; /* 0:SEEE v2.1(for K2,Denalix,Jade...exc)  , 2:SEEE v3.0(for Everest) */
    //
    if(tdriPipe.tdri.top.lcei_en)
    {
        tdriPipe.tdri.lcei.lcei_v_flip_en = DMALcei.dma_cfg.v_flip_en;
        tdriPipe.tdri.lcei.lcei_xsize = DMALcei.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.lcei.lcei_ysize = DMALcei.dma_cfg.size.h - 1;
        tdriPipe.tdri.lcei.lcei_stride = DMALcei.dma_cfg.size.stride;
    }
    else
    {
        tdriPipe.tdri.lcei.lcei_v_flip_en = 0;
        tdriPipe.tdri.lcei.lcei_xsize = 0;
        tdriPipe.tdri.lcei.lcei_ysize = 0;
        tdriPipe.tdri.lcei.lcei_stride = 0;
    }
    //
    tdriPipe.tdri.lce.lce_bc_mag_kubnx = tuningIspReg->DIP_X_LCE25_ZR.Bits.LCE_BCMK_X;
    tdriPipe.tdri.lce.lce_slm_width = tuningIspReg->DIP_X_LCE25_SLM_SIZE.Bits.LCE_SLM_WD;
    tdriPipe.tdri.lce.lce_bc_mag_kubny = tuningIspReg->DIP_X_LCE25_ZR.Bits.LCE_BCMK_Y;
    tdriPipe.tdri.lce.lce_slm_height = tuningIspReg->DIP_X_LCE25_SLM_SIZE.Bits.LCE_SLM_HT;
    tdriPipe.tdri.lce.lce_full_slm_width = tuningIspReg->DIP_X_LCE25_SLM.Bits.LCE_TPIPE_SLM_WD;    
    tdriPipe.tdri.lce.lce_full_slm_height = tuningIspReg->DIP_X_LCE25_SLM.Bits.LCE_TPIPE_SLM_HT;    
    tdriPipe.tdri.lce.lce_full_xoff = tuningIspReg->DIP_X_LCE25_OFFSET.Bits.LCE_TPIPE_OFFSET_X;     
    tdriPipe.tdri.lce.lce_full_yoff = tuningIspReg->DIP_X_LCE25_OFFSET.Bits.LCE_TPIPE_OFFSET_Y;     
    tdriPipe.tdri.lce.lce_full_out_height = tuningIspReg->DIP_X_LCE25_OUT.Bits.LCE_TPIPE_OUT_HT;      
    //
    if(tdriPipe.tdri.top.crz_en)
    {
        tdriPipe.tdri.cdrz.cdrz_input_crop_width = p_parameter->crz.crz_input_crop_width;
        tdriPipe.tdri.cdrz.cdrz_input_crop_height = p_parameter->crz.crz_input_crop_height;
        tdriPipe.tdri.cdrz.cdrz_output_width = p_parameter->crz.crz_output_width;
        tdriPipe.tdri.cdrz.cdrz_output_height = p_parameter->crz.crz_output_height;
        tdriPipe.tdri.cdrz.cdrz_luma_horizontal_integer_offset = p_parameter->crz.crz_horizontal_integer_offset;
        tdriPipe.tdri.cdrz.cdrz_luma_horizontal_subpixel_offset = p_parameter->crz.crz_horizontal_subpixel_offset;
        tdriPipe.tdri.cdrz.cdrz_luma_vertical_integer_offset = p_parameter->crz.crz_vertical_integer_offset;
        tdriPipe.tdri.cdrz.cdrz_luma_vertical_subpixel_offset = p_parameter->crz.crz_vertical_subpixel_offset;
        tdriPipe.tdri.cdrz.cdrz_horizontal_luma_algorithm = p_parameter->crz.crz_horizontal_luma_algorithm;
        tdriPipe.tdri.cdrz.cdrz_vertical_luma_algorithm = p_parameter->crz.crz_vertical_luma_algorithm;
        tdriPipe.tdri.cdrz.cdrz_horizontal_coeff_step = p_parameter->crz.crz_horizontal_coeff_step;
        tdriPipe.tdri.cdrz.cdrz_vertical_coeff_step = p_parameter->crz.crz_vertical_coeff_step;
    }
    else
    {
        tdriPipe.tdri.cdrz.cdrz_input_crop_width = 0;
        tdriPipe.tdri.cdrz.cdrz_input_crop_height = 0;
        tdriPipe.tdri.cdrz.cdrz_output_width = 0;
        tdriPipe.tdri.cdrz.cdrz_output_height = 0;
        tdriPipe.tdri.cdrz.cdrz_luma_horizontal_integer_offset = 0;
        tdriPipe.tdri.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
        tdriPipe.tdri.cdrz.cdrz_luma_vertical_integer_offset = 0 ;
        tdriPipe.tdri.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
        tdriPipe.tdri.cdrz.cdrz_horizontal_luma_algorithm = 0;
        tdriPipe.tdri.cdrz.cdrz_vertical_luma_algorithm = 0;
        tdriPipe.tdri.cdrz.cdrz_horizontal_coeff_step = 0;
        tdriPipe.tdri.cdrz.cdrz_vertical_coeff_step = 0;
    }
    //
    if(tdriPipe.tdri.top.img2o_en)
    {
        tdriPipe.tdri.img2o.img2o_stride = DMAImg2o.dma_cfg.size.stride;
        tdriPipe.tdri.img2o.img2o_xoffset = DMAImg2o.dma_cfg.offset.x;
        tdriPipe.tdri.img2o.img2o_yoffset = DMAImg2o.dma_cfg.offset.y;
        tdriPipe.tdri.img2o.img2o_xsize = DMAImg2o.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.img2o.img2o_ysize = DMAImg2o.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.img2o.img2o_stride = 0;
        tdriPipe.tdri.img2o.img2o_xoffset = 0;
        tdriPipe.tdri.img2o.img2o_yoffset = 0;
        tdriPipe.tdri.img2o.img2o_xsize = 0;
        tdriPipe.tdri.img2o.img2o_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.img2bo_en)
    {
        tdriPipe.tdri.img2bo.img2bo_stride = DMAImg2bo.dma_cfg.size.stride;
        tdriPipe.tdri.img2bo.img2bo_xoffset = DMAImg2bo.dma_cfg.offset.x;
        tdriPipe.tdri.img2bo.img2bo_yoffset = DMAImg2bo.dma_cfg.offset.y;
        tdriPipe.tdri.img2bo.img2bo_xsize = DMAImg2bo.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.img2bo.img2bo_ysize = DMAImg2bo.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.img2bo.img2bo_stride = 0;
        tdriPipe.tdri.img2bo.img2bo_xoffset = 0;
        tdriPipe.tdri.img2bo.img2bo_yoffset = 0;
        tdriPipe.tdri.img2bo.img2bo_xsize = 0;
        tdriPipe.tdri.img2bo.img2bo_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.srz1_en)
    {
        tdriPipe.tdri.srz1.srz_input_crop_width = p_parameter->srz1_cfg.crop.w;
        tdriPipe.tdri.srz1.srz_input_crop_height = p_parameter->srz1_cfg.crop.h;
        tdriPipe.tdri.srz1.srz_output_width = p_parameter->srz1_cfg.inout_size.out_w;
        tdriPipe.tdri.srz1.srz_output_height = p_parameter->srz1_cfg.inout_size.out_h;
        tdriPipe.tdri.srz1.srz_luma_horizontal_integer_offset = p_parameter->srz1_cfg.crop.x;
        tdriPipe.tdri.srz1.srz_luma_horizontal_subpixel_offset = p_parameter->srz1_cfg.crop.floatX;
        tdriPipe.tdri.srz1.srz_luma_vertical_integer_offset = p_parameter->srz1_cfg.crop.y;
        tdriPipe.tdri.srz1.srz_luma_vertical_subpixel_offset = p_parameter->srz1_cfg.crop.floatY;
        tdriPipe.tdri.srz1.srz_horizontal_coeff_step = p_parameter->srz1_cfg.h_step;
        tdriPipe.tdri.srz1.srz_vertical_coeff_step = p_parameter->srz1_cfg.v_step;
    }
    else
    {
        tdriPipe.tdri.srz1.srz_input_crop_width = 0;
        tdriPipe.tdri.srz1.srz_input_crop_height = 0;
        tdriPipe.tdri.srz1.srz_output_width = 0;
        tdriPipe.tdri.srz1.srz_output_height = 0;
        tdriPipe.tdri.srz1.srz_luma_horizontal_integer_offset = 0;
        tdriPipe.tdri.srz1.srz_luma_horizontal_subpixel_offset = 0;
        tdriPipe.tdri.srz1.srz_luma_vertical_integer_offset = 0;
        tdriPipe.tdri.srz1.srz_luma_vertical_subpixel_offset = 0;
        tdriPipe.tdri.srz1.srz_horizontal_coeff_step = 0;
        tdriPipe.tdri.srz1.srz_vertical_coeff_step = 0;
    }
    //
    if(tdriPipe.tdri.top.srz2_en)
    {
        tdriPipe.tdri.srz2.srz_input_crop_width = p_parameter->srz2_cfg.inout_size.in_w;
        tdriPipe.tdri.srz2.srz_input_crop_height = p_parameter->srz2_cfg.inout_size.in_h;
        tdriPipe.tdri.srz2.srz_output_width = p_parameter->srz2_cfg.inout_size.out_w;
        tdriPipe.tdri.srz2.srz_output_height = p_parameter->srz2_cfg.inout_size.out_h;
        tdriPipe.tdri.srz2.srz_luma_horizontal_integer_offset = p_parameter->srz2_cfg.crop.x;
        tdriPipe.tdri.srz2.srz_luma_horizontal_subpixel_offset = p_parameter->srz2_cfg.crop.floatX;
        tdriPipe.tdri.srz2.srz_luma_vertical_integer_offset = p_parameter->srz2_cfg.crop.y;
        tdriPipe.tdri.srz2.srz_luma_vertical_subpixel_offset = p_parameter->srz2_cfg.crop.floatY;
        tdriPipe.tdri.srz2.srz_horizontal_coeff_step = p_parameter->srz2_cfg.h_step;
        tdriPipe.tdri.srz2.srz_vertical_coeff_step = p_parameter->srz2_cfg.v_step;
    }
    else
    {
        tdriPipe.tdri.srz2.srz_input_crop_width = 0;
        tdriPipe.tdri.srz2.srz_input_crop_height = 0;
        tdriPipe.tdri.srz2.srz_output_width = 0;
        tdriPipe.tdri.srz2.srz_output_height = 0;
        tdriPipe.tdri.srz2.srz_luma_horizontal_integer_offset = 0;
        tdriPipe.tdri.srz2.srz_luma_horizontal_subpixel_offset = 0;
        tdriPipe.tdri.srz2.srz_luma_vertical_integer_offset = 0;
        tdriPipe.tdri.srz2.srz_luma_vertical_subpixel_offset = 0;
        tdriPipe.tdri.srz2.srz_horizontal_coeff_step = 0;
        tdriPipe.tdri.srz2.srz_vertical_coeff_step = 0;
    }
    //
    if(tdriPipe.tdri.top.srz3_en)
    {
        tdriPipe.tdri.srz3.srz_input_crop_width = p_parameter->srz3_cfg.inout_size.in_w;
        tdriPipe.tdri.srz3.srz_input_crop_height = p_parameter->srz3_cfg.inout_size.in_h;
        tdriPipe.tdri.srz3.srz_output_width = p_parameter->srz3_cfg.inout_size.out_w;
        tdriPipe.tdri.srz3.srz_output_height = p_parameter->srz3_cfg.inout_size.out_h;
        tdriPipe.tdri.srz3.srz_luma_horizontal_integer_offset = p_parameter->srz3_cfg.crop.x;
        tdriPipe.tdri.srz3.srz_luma_horizontal_subpixel_offset = p_parameter->srz3_cfg.crop.floatX;
        tdriPipe.tdri.srz3.srz_luma_vertical_integer_offset = p_parameter->srz3_cfg.crop.y;
        tdriPipe.tdri.srz3.srz_luma_vertical_subpixel_offset = p_parameter->srz3_cfg.crop.floatY;
        tdriPipe.tdri.srz3.srz_horizontal_coeff_step = p_parameter->srz3_cfg.h_step;
        tdriPipe.tdri.srz3.srz_vertical_coeff_step = p_parameter->srz3_cfg.v_step;
    }
    else
    {
        tdriPipe.tdri.srz3.srz_input_crop_width = 0;
        tdriPipe.tdri.srz3.srz_input_crop_height = 0;
        tdriPipe.tdri.srz3.srz_output_width = 0;
        tdriPipe.tdri.srz3.srz_output_height = 0;
        tdriPipe.tdri.srz3.srz_luma_horizontal_integer_offset = 0;
        tdriPipe.tdri.srz3.srz_luma_horizontal_subpixel_offset = 0;
        tdriPipe.tdri.srz3.srz_luma_vertical_integer_offset = 0;
        tdriPipe.tdri.srz3.srz_luma_vertical_subpixel_offset = 0;
        tdriPipe.tdri.srz3.srz_horizontal_coeff_step = 0;
        tdriPipe.tdri.srz3.srz_vertical_coeff_step = 0;
    }
    //
    if(tdriPipe.tdri.top.srz4_en)
    {
        tdriPipe.tdri.srz4.srz_input_crop_width = p_parameter->srz4_cfg.inout_size.in_w;
        tdriPipe.tdri.srz4.srz_input_crop_height = p_parameter->srz4_cfg.inout_size.in_h;
        tdriPipe.tdri.srz4.srz_output_width = p_parameter->srz4_cfg.inout_size.out_w;
        tdriPipe.tdri.srz4.srz_output_height = p_parameter->srz4_cfg.inout_size.out_h;
        tdriPipe.tdri.srz4.srz_luma_horizontal_integer_offset = p_parameter->srz4_cfg.crop.x;
        tdriPipe.tdri.srz4.srz_luma_horizontal_subpixel_offset = p_parameter->srz4_cfg.crop.floatX;
        tdriPipe.tdri.srz4.srz_luma_vertical_integer_offset = p_parameter->srz4_cfg.crop.y;
        tdriPipe.tdri.srz4.srz_luma_vertical_subpixel_offset = p_parameter->srz4_cfg.crop.floatY;
        tdriPipe.tdri.srz4.srz_horizontal_coeff_step = p_parameter->srz4_cfg.h_step;
        tdriPipe.tdri.srz4.srz_vertical_coeff_step = p_parameter->srz4_cfg.v_step;
    }
    else
    {
        tdriPipe.tdri.srz4.srz_input_crop_width = 0;
        tdriPipe.tdri.srz4.srz_input_crop_height = 0;
        tdriPipe.tdri.srz4.srz_output_width = 0;
        tdriPipe.tdri.srz4.srz_output_height = 0;
        tdriPipe.tdri.srz4.srz_luma_horizontal_integer_offset = 0;
        tdriPipe.tdri.srz4.srz_luma_horizontal_subpixel_offset = 0;
        tdriPipe.tdri.srz4.srz_luma_vertical_integer_offset = 0;
        tdriPipe.tdri.srz4.srz_luma_vertical_subpixel_offset = 0;
        tdriPipe.tdri.srz4.srz_horizontal_coeff_step = 0;
        tdriPipe.tdri.srz4.srz_vertical_coeff_step = 0;
    }
    //
    tdriPipe.tdri.fe.fe_mode = tuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE;
    ISP_PATH_DBG("fe_mode(%d/%d)",tdriPipe.tdri.fe.fe_mode, tuningIspReg->DIP_X_FE_CTRL1.Bits.FE_MODE);
    //
    tdriPipe.tdri.feo.feo_stride = DMAFeo.dma_cfg.size.stride;
    //
    tdriPipe.tdri.nr3d.nr3d_on_en = tuningIspReg->DIP_X_NR3D_ON_CON.Bits.NR3D_ON_EN;
    tdriPipe.tdri.nr3d.nr3d_on_xoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_X;
    tdriPipe.tdri.nr3d.nr3d_on_yoffset = tuningIspReg->DIP_X_NR3D_ON_OFF.Bits.NR3D_ON_OFST_Y;
    tdriPipe.tdri.nr3d.nr3d_on_width = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_WD;
    tdriPipe.tdri.nr3d.nr3d_on_height = tuningIspReg->DIP_X_NR3D_ON_SIZ.Bits.NR3D_ON_HT;
    //
    tdriPipe.tdri.crsp.crsp_ystep = 4;  /* Paul suggested setting 0x04 for this that same as rome */
    tdriPipe.tdri.crsp.crsp_xoffset = 0x00;  /* Paul suggested setting 0x00 for this that same as rome */
    tdriPipe.tdri.crsp.crsp_yoffset = 0x01;  /* Paul suggested setting 0x01 for this that same as rome */
    //
    if(tdriPipe.tdri.top.img3o_en)
    {
        tdriPipe.tdri.img3o.img3o_stride = DMAImg3o.dma_cfg.size.stride;
        tdriPipe.tdri.img3o.img3o_xoffset = DMAImg3o.dma_cfg.offset.x;
        tdriPipe.tdri.img3o.img3o_yoffset = DMAImg3o.dma_cfg.offset.y;
        tdriPipe.tdri.img3o.img3o_xsize = DMAImg3o.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.img3o.img3o_ysize = DMAImg3o.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.img3o.img3o_stride = 0;
        tdriPipe.tdri.img3o.img3o_xoffset = 0;
        tdriPipe.tdri.img3o.img3o_yoffset = 0;
        tdriPipe.tdri.img3o.img3o_xsize = 0;
        tdriPipe.tdri.img3o.img3o_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.img3bo_en)
    {
        tdriPipe.tdri.img3bo.img3bo_stride = DMAImg3bo.dma_cfg.size.stride;
        tdriPipe.tdri.img3bo.img3bo_xsize = DMAImg3bo.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.img3bo.img3bo_ysize = DMAImg3bo.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.img3bo.img3bo_stride = 0;
        tdriPipe.tdri.img3bo.img3bo_xsize = 0;
        tdriPipe.tdri.img3bo.img3bo_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.img3co_en)
    {
        tdriPipe.tdri.img3co.img3co_stride = DMAImg3co.dma_cfg.size.stride;
        tdriPipe.tdri.img3co.img3co_xsize = DMAImg3co.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.img3co.img3co_ysize = DMAImg3co.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.img3co.img3co_stride = 0;
        tdriPipe.tdri.img3co.img3co_xsize = 0;
        tdriPipe.tdri.img3co.img3co_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.dmgi_en)
    {
        tdriPipe.tdri.dmgi.dmgi_v_flip_en = DMADmgi.dma_cfg.v_flip_en;
        tdriPipe.tdri.dmgi.dmgi_stride = DMADmgi.dma_cfg.size.stride;
        tdriPipe.tdri.dmgi.dmgi_offset = DMADmgi.dma_cfg.offset.x; // for non-image data, use x-offset to regard as general offset
        tdriPipe.tdri.dmgi.dmgi_xsize = DMADmgi.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.dmgi.dmgi_ysize = DMADmgi.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.dmgi.dmgi_v_flip_en = 0;
        tdriPipe.tdri.dmgi.dmgi_stride = 0;
        tdriPipe.tdri.dmgi.dmgi_offset = 0; // for non-image data, use x-offset to regard as general offset
        tdriPipe.tdri.dmgi.dmgi_xsize = 0;
        tdriPipe.tdri.dmgi.dmgi_ysize = 0;
    }
    //
    if(tdriPipe.tdri.top.depi_en)
    {
        tdriPipe.tdri.depi.depi_v_flip_en = DMADepi.dma_cfg.v_flip_en;
        tdriPipe.tdri.depi.depi_stride = DMADepi.dma_cfg.size.stride;
        tdriPipe.tdri.depi.depi_xsize = DMADepi.dma_cfg.size.xsize - 1;
        tdriPipe.tdri.depi.depi_ysize = DMADepi.dma_cfg.size.h - 1;
    }
    else
    {
        tdriPipe.tdri.depi.depi_v_flip_en = 0;
        tdriPipe.tdri.depi.depi_stride = 0;
        tdriPipe.tdri.depi.depi_xsize = 0;
        tdriPipe.tdri.depi.depi_ysize = 0;        
    }
    //
    tdriPipe.tdri.pca.pca_cfc_en = tuningIspReg->DIP_X_PCA_CON1.Bits.PCA_CFC_EN;
    tdriPipe.tdri.pca.pca_cnv_en = tuningIspReg->DIP_X_PCA_CON1.Bits.PCA_CNV_EN;
    //
EXIT:
    return ret;

}

int CamPathPass2::setZoom( MUINT32 zoomRatio )
{
int ret = 0;

    ISP_PATH_DBG(":E");

//kk test    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);


    /*function List*/
     m_isp_function_list[0 ] = (IspFunctionDip_B*)&crzPipe;
     m_isp_function_list[1 ] = (IspFunctionDip_B*)&DMAImgi;
     m_isp_function_list[2 ] = (IspFunctionDip_B*)&DMAVipi;
     m_isp_function_list[3 ] = (IspFunctionDip_B*)&DMAVip2i;
     m_isp_function_list[4 ] = (IspFunctionDip_B*)&DMATdri;
    m_isp_function_count=5;

    //cdp
    //crzPipe.dipTh = DIP_ISP_CQ1;

    this->_setZoom(NULL);

    ISP_PATH_DBG(":X");

    return ret;

}

int CamPathPass2::_waitIrq(int type, unsigned int irq, int userNumber )
{
    int ret = 0;
    ISP_WAIT_IRQ_ST WaitIrq;

    ISP_PATH_DBG("+,");

//kk test    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2Lock);

    WaitIrq.Clear = ISP_IRQ_CLEAR_NONE;
    WaitIrq.St_type = (ISP_ST_ENUM)type;//SIGNAL_INT;
    WaitIrq.UserKey= userNumber;
    WaitIrq.Status = irq;
    WaitIrq.Timeout = CAM_INT_PASS2_WAIT_TIMEOUT_MS;//ms ,0 means pass through.

    if (0 == ispBufCtrl.waitIrq(&WaitIrq) ) {
        ISP_PATH_ERR("waitIrq fail");
        ret = -1;
    }

    ISP_PATH_DBG("-,");

    return ret;
}


//
EIspRetStatus CamPathPass2::dequeueBuf( MUINT32 dmaChannel, MINT32 const eDrvSce, MINT32 dequeCq,MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx, MUINT32 moduleIdx)
{
    EIspRetStatus ret;

    ISP_PATH_DBG("+path(%d), dequeCq(%d),dequeBurstQueIdx(%d),dequeDupCqIdx(%d),moduleIdx(%d)",eDrvSce, dequeCq,dequeBurstQueIdx,dequeDupCqIdx,moduleIdx);

//kk test    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gPass2DequeLock);

    this->ispBufCtrl.dipTh = dequeCq;
    this->ispBufCtrl.dupCqIdx = dequeDupCqIdx;
    this->ispBufCtrl.burstQueIdx = dequeBurstQueIdx;
    this->ispBufCtrl.drvScenario = eDrvSce;
    this->ispBufCtrl.moduleIdx = moduleIdx;


    //move FILLED buffer from hw to sw list
    ret = this->ispBufCtrl.dequeueHwBuf( dmaChannel);

    ISP_PATH_DBG("-,");
    return ret;
}
//
int CamPathPass2::dequeueMdpFrameEnd(MINT32 const eDrvSce,MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx)
{
    int ret = 0;
    ISP_PATH_DBG("+,");

    if ( 0!= this->ispBufCtrl.dequeueMdpFrameEnd(eDrvSce,dequeCq, dequeBurstQIdx,dequeDupCqIdx, moduleIdx) ) {
        ISP_PATH_ERR("ERROR:dequeueMdpFrameEnd");
        return -1;
    }
    ISP_PATH_DBG("-,");

    return ret;
}




