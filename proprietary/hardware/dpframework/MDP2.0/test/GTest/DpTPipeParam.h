#ifndef __DP_TPIPE_H__
#define __DP_TPIPE_H__

#include "DpIspStream.h"
#include "DpDataType.h"
#include <tpipe_config.h>

class DpTPipe
{
public:
    DpTPipe(){};

    ~DpTPipe();

    DP_STATUS_ENUM getHardCodeTPipeInfo(ISP_TPIPE_CONFIG_STRUCT *ISPInfo)
    {
        memset(ISPInfo, 0, sizeof(ISP_TPIPE_CONFIG_STRUCT));
        /*
        ISPInfo->sw.log_en = 1;
        ISPInfo->sw.src_width = 3264;
        ISPInfo->sw.src_height = 2448;
        ISPInfo->sw.tpipe_width = 768;
        ISPInfo->sw.tpipe_height = 8192;
        ISPInfo->sw.tpipe_irq_mode = 0;
        ISPInfo->top.scenario = 6;
        ISPInfo->top.mode = 0;
        ISPInfo->top.pixel_id = 0;
        ISPInfo->top.cam_in_fmt = 0;
        ISPInfo->top.ctl_extension_en = 0;
        ISPInfo->top.fg_mode = 0;
        ISPInfo->top.ufdi_fmt = 0;
        ISPInfo->top.vipi_fmt = 2;
        ISPInfo->top.img3o_fmt = 2;
        ISPInfo->top.imgi_en = 1;
        ISPInfo->top.g2c_en = 1;
        ISPInfo->top.c42_en = 1;
        ISPInfo->top.nbc_en = 1;
        ISPInfo->top.sl2c_en = 1;
        ISPInfo->top.seee_en = 1;
        ISPInfo->top.crz_en = 1;
        ISPInfo->top.img2o_en = 1;
        ISPInfo->top.crsp_en = 0;
        ISPInfo->top.img3o_en = 1;
        ISPInfo->top.c24b_en = 0;
        ISPInfo->top.mdp_crop_en = 0;
        ISPInfo->top.imgi_v_flip_en = 0;
        ISPInfo->top.ufd_sel = 0;
        ISPInfo->top.ccl_sel = 0;
        ISPInfo->top.ccl_sel_en = 1;
        ISPInfo->top.g2g_sel = 1;
        ISPInfo->top.g2g_sel_en = 1;
        ISPInfo->top.mix1_sel = 0;
        ISPInfo->top.crz_sel = 0;
        ISPInfo->top.nr3d_sel = 0;
        ISPInfo->top.fe_sel = 1;
        ISPInfo->top.mdp_sel = 0;
        ISPInfo->top.pca_sel = 0;
        ISPInfo->top.interlace_mode = 0;
        ISPInfo->imgi.imgi_stride = 4080;
        ISPInfo->g2c.g2c_shade_en = 1;
        ISPInfo->nbc.anr_eny = 1;
        ISPInfo->nbc.anr_enc = 1;
        ISPInfo->nbc.anr_iir_mode = 0;
        ISPInfo->nbc.anr_scale_mode = 2;
        ISPInfo->sl2c.sl2c_hrz_comp = 0;
        ISPInfo->seee.se_edge = 1;
        ISPInfo->cdrz.cdrz_input_crop_width = 2612;
        ISPInfo->cdrz.cdrz_input_crop_height = 1959;
        ISPInfo->cdrz.cdrz_output_width = 3264;
        ISPInfo->cdrz.cdrz_output_height = 2448;
        ISPInfo->cdrz.cdrz_luma_horizontal_integer_offset = 326;
        ISPInfo->cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
        ISPInfo->cdrz.cdrz_luma_vertical_integer_offset = 244;
        ISPInfo->cdrz.cdrz_luma_vertical_subpixel_offset = 0;
        ISPInfo->cdrz.cdrz_horizontal_luma_algorithm = 0;
        ISPInfo->cdrz.cdrz_vertical_luma_algorithm = 0;
        ISPInfo->cdrz.cdrz_horizontal_coeff_step = 26220;
        ISPInfo->cdrz.cdrz_vertical_coeff_step = 26220;
        ISPInfo->img2o.img2o_stride = 0;
        ISPInfo->img2o.img2o_xoffset = 0;
        ISPInfo->img2o.img2o_yoffset = 0;
        ISPInfo->img2o.img2o_xsize = 6527;
        ISPInfo->img2o.img2o_ysize = 2447;
        ISPInfo->img3o.img3o_stride = 0;
        ISPInfo->img3o.img3o_xoffset = 0;
        ISPInfo->img3o.img3o_yoffset = 0;
        ISPInfo->img3o.img3o_xsize = 6527;
        ISPInfo->img3o.img3o_ysize = 2447;
        */
        return DP_STATUS_RETURN_SUCCESS;
    }

private:
};

#endif  // __DP_TPIPE_H__
