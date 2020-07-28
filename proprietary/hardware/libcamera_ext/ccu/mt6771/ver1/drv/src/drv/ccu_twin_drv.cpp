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
#include <algorithm>    // std::swap

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CcuTwinDrv"

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "ccu_log.h"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

#include "ccu_ext_interface/ccu_af_reg.h"
#include "ccu_ext_interface/ccu_ext_interface.h"

#define DUAL_AF_TAPS (12) /* must be even */
#define DUAL_AF_MAX_BLOCK_WIDTH (80) /* must be even */
#define DUAL_AF_FAST_MARGIN (128) /* must be even */
#define DUAL_AF_FAST_MARGIN_L (32) /* must be even */

#define DUAL_ALIGN_PIXEL_MODE(n) (1<<(n))

#define DUAL_TILE_LOSS_DBS (2)
#define DUAL_TILE_LOSS_BNR (4)
#define DUAL_TILE_LOSS_RMM (2)
#define DUAL_TILE_LOSS_CAC (6)
#define DUAL_TILE_LOSS_HLR (2)
#define DUAL_TILE_LOSS_ALL (DUAL_TILE_LOSS_DBS + DUAL_TILE_LOSS_BNR + DUAL_TILE_LOSS_RMM + DUAL_TILE_LOSS_CAC + DUAL_TILE_LOSS_HLR)
#define DUAL_TILE_LOSS_ALL_L (DUAL_TILE_LOSS_DBS + DUAL_TILE_LOSS_BNR + DUAL_TILE_LOSS_RMM + DUAL_TILE_LOSS_CAC + DUAL_TILE_LOSS_HLR)

int cal_dual_af(int tgInfo, CAM_REG_AF_FMT* pReg_af_fmt) {
#if 0
    // update single/twin register
    cam_reg_af_t* ptr_param = &pReg_af_fmt->data.reg;
    cam_reg_af_t* ptr_param_a = (tgInfo == CCU_CAM_TG_1)? &pReg_af_fmt->data_a.reg : &pReg_af_fmt->data_b.reg;
    cam_reg_af_t* ptr_param_b = (tgInfo == CCU_CAM_TG_1)? &pReg_af_fmt->data_b.reg : &pReg_af_fmt->data_a.reg;

    int const TWIN_AF_OFFSET = ptr_param->CAM_AF_VLD.Bits.AF_VLD_XSTART;
    int const TWIN_AF_BLOCK_XNUM = ptr_param->CAM_AF_BLK_1.Bits.AF_BLK_XNUM;

    int bin_mode_b = 1;/* default 2 pixel mode, after bin, align bin */
    int bin_min_b = 1;/* default 2 pixel mode, after bin, align bin */
    int dmx_valid[2];
    dmx_valid[0] = (pReg_af_fmt->binWidth+3)/4*2;
    dmx_valid[1] = pReg_af_fmt->binWidth - dmx_valid[0];

    int left_loss_af = 0;
    int right_loss_af = 0;
    int left_margin_af = 0;
    int right_margin_af = 0;

    /* RTL verif or platform */
    //ptr_param_a->CAM_RCP_CROP_CON1.Raw = 0xA6F0000;//0x5370000;//0xA6F0000;
    //ptr_param_b->CAM_RCP_CROP_CON1.Raw = 0xA6F0000;//0x5370000;//0xA6F0000;
    if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
    {
        if (ptr_param->CAM_CTL_EN.Bits.AF_EN)
        {
            left_loss_af = DUAL_AF_TAPS;
            right_loss_af = DUAL_AF_TAPS;
            left_margin_af = 0;
            right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
            if ((int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
            {
                LOG_ERR("CCU_DUAL_MESSAGE_AF_FAST_MODE_CONFIG_ERROR");
                return -1;
            }
        }
    }

    /* update bin_mode_b for DMX */
    if (ptr_param->CAM_CTL_EN.Bits.BIN_EN)
    {
        /* remarked for af fast mode */
        /* if (ptr_in_param->CAM_A_RMG_HDR_CFG.Bits.RMG_ZHDR_EN) */
        {
            /* DMX_A_STR, DMX_B_END, x4 after bin, x8 before bin */
            if (bin_mode_b < 2)
            {
                bin_mode_b = 2;
            }
        }
    }
    if (ptr_param->CAM_CTL_DMA_EN.Bits.SCM_EN)
    {
        /* DMX_A_STR, DMX_B_END, x4 after scm, x8 before scm */
        if (bin_mode_b < 2)
        {
            bin_mode_b = 2;
        }
    }


    //if (ptr_param->CAM_CTL_EN.Bits.RCP_EN)/*alwaye enable for CCU*/
    {
        /* RCROP */
        int rcp_a_width_x = dmx_valid[0] + right_loss_af + right_margin_af;
        int rcp_b_width_x = dmx_valid[1] + left_loss_af + left_margin_af;
        /* AF & AFO */
        if (ptr_param->CAM_CTL_EN.Bits.AF_EN)
        {
            /* AF & AF_D */
            if ((rcp_a_width_x < 32) || (rcp_b_width_x < 32))
            {
                LOG_ERR("CCU_DUAL_MESSAGE_INVALID_CONFIG_ERROR1");
                return -1;
            }
            /* RTL verif or platform */
            if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
            {
                /* AFO size will be possibly different */
                /* cal valid af config*/
                if ((ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE & 0x1) || (TWIN_AF_OFFSET & 0x1))
                {
                    LOG_ERR("CCU_DUAL_MESSAGE_INVALID_CONFIG_ERROR2");
                    return -1;
                }
                /* platform */
                if ((ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE != ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE) ||
                    ((int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
                    ((dmx_valid[0] + dmx_valid[1]) < TWIN_AF_OFFSET +
                    (int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE*TWIN_AF_BLOCK_XNUM) ||
                    (TWIN_AF_BLOCK_XNUM <= 0))
                {
                    LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                    return -1;
                }
                /* check AF_A no output */
                LOG_DBG("CCU_TEST  RCP_WIDTH(%d)  TWIN_AF_OFFSET(%d) AF_BLK_XSIZE(%d) right_loss_af(%d)",
                       rcp_a_width_x,
                       TWIN_AF_OFFSET,
                       ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE,
                       right_loss_af );
                if (rcp_a_width_x < TWIN_AF_OFFSET +
                    (int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE + right_loss_af)
                {
                    ptr_param_a->CAM_CTL_EN.Bits.AF_EN = false;
                    ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN = false;
                }
                else
                {
                    LOG_DBG("CCU_TEST  TWIN_AF_BLOCK_XNUM(%d)  AF_BLK_XSIZE(%d) TWIN_AF_OFFSET(%d) dmx_valid(%d) right_margin_af(%d)",
                       TWIN_AF_BLOCK_XNUM,
                       ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE,
                       TWIN_AF_OFFSET,
                       dmx_valid[0],
                       right_margin_af );

                    ptr_param_a->CAM_AF_VLD.Bits.AF_VLD_XSTART = TWIN_AF_OFFSET;
                    /* check AF_A output all */
                    if (TWIN_AF_BLOCK_XNUM*(int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE +
                        TWIN_AF_OFFSET <= dmx_valid[0] + right_margin_af)
                    {
                        ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = TWIN_AF_BLOCK_XNUM;
                    }
                    else
                    {
                        ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + right_margin_af - TWIN_AF_OFFSET)/
                            (int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE;

                        LOG_DBG("CCU_TEST  CAM_A AF_BLK_XNUM(%d) =  (dmx_valid(%d) + right_margin_af(%d) - TWIN_AF_OFFSET(%d) )/ AF_BLK_XSIZE(%d)",
                            ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM,
                            dmx_valid[0],
                            right_margin_af,
                            TWIN_AF_OFFSET,
                            ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE );
                    }
                 }
            }
            else
            {
                ptr_param_a->CAM_AF_VLD.Bits.AF_VLD_XSTART = 0;
                if (dmx_valid[0] < (int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM)
                {
                    ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[0]/(int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE;
                }
            }
            if (ptr_param_a->CAM_CTL_EN.Bits.AF_EN)
            {
                /* check max 128 */
                if (ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
                {
                    if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                    {
                        LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR");
                        return -1;
                    }
                    else
                    {
                        /* RTL */
                        ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
                    }
                }
            }
            /* RTL verif or platform */
            if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
            {
                LOG_DBG("CCU_TEST  dmx_valid(%d) + right_margin_af(%d) < TWIN_AF_OFFSET(%d) + AF_BLK_XSIZE(%d)",
                       dmx_valid[0],
                       right_margin_af,
                       TWIN_AF_OFFSET,
                       ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE );
                /* AF_B output all */
                if (dmx_valid[0] + right_margin_af < TWIN_AF_OFFSET + (int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE)
                {
                    ptr_param_b->CAM_AF_VLD.Bits.AF_VLD_XSTART = TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
                    ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = TWIN_AF_BLOCK_XNUM;
                }
                else
                {
                    LOG_DBG("CCU_TEST  TWIN_AF_BLOCK_XNUM(%d) > AF_BLK_XNUM(%d)",
                       TWIN_AF_BLOCK_XNUM,
                       ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM );
                    /* AF_B with output */
                    if (TWIN_AF_BLOCK_XNUM > (int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM)
                    {
                        ptr_param_b->CAM_AF_VLD.Bits.AF_VLD_XSTART = TWIN_AF_OFFSET +
                            (int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE -
                            (dmx_valid[0] - left_loss_af - left_margin_af);
                        ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = TWIN_AF_BLOCK_XNUM -
                            (int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM;

                        LOG_DBG("CCU_TEST  CAM_B AF_VLD_XSTART(%d) = TWIN_AF_OFFSET(%d) + AF_BLK_XNUM(%d)*AF_BLK_XSIZE(%d) - (dmx_valid(%d)-left_loss_af(%d)-left_margin_af(%d))",
                           ptr_param_b->CAM_AF_VLD.Bits.AF_VLD_XSTART,
                           TWIN_AF_OFFSET,
                           ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM,
                           ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE,
                           dmx_valid[0],
                           left_loss_af,
                           left_margin_af);

                        LOG_DBG("CCU_TEST  CAM_B AF_BLK_XNUM(%d) = TWIN_AF_BLOCK_XNUM(%d) - AF_BLK_XNUM(%d)",
                           ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM,
                           TWIN_AF_BLOCK_XNUM,
                           ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM);
                    }
                    else
                    {
                        ptr_param_b->CAM_CTL_EN.Bits.AF_EN = false;
                        ptr_param_b->CAM_CTL_DMA_EN.Bits.AFO_EN = false;
                    }
                }
            }
            else
            {
                ptr_param_b->CAM_AF_VLD.Bits.AF_VLD_XSTART = 0;
                if (rcp_b_width_x < (int)ptr_param_b->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM)
                {
                    ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = rcp_b_width_x/(int)ptr_param_b->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE;
                }
            }
            if (ptr_param_b->CAM_CTL_EN.Bits.AF_EN)
            {
                /* check max 128 */
                if (ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
                {
                    if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                    {
                        LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR");
                        return -1;
                    }
                    else
                    {
                        /* RTL */
                        ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
                    }
                }
            }
            if (ptr_param->CAM_CTL_DMA_EN.Bits.AFO_EN)
            {
                if (ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE < 16)
                {
                    LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                    return -1;
                }
                if (ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN)
                {
                    ptr_param_a->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
                    ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE = (int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM*16*
                        ((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
                    /* check stride */
                    if ((unsigned int)ptr_param_a->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE + 1 >
                        (unsigned int)ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE)
                    {
                        if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                            return -1;
                        }
                        else
                        {
                            /* RTL */
                            ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE -
                                (unsigned int)ptr_param_a->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN);
                            ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
                                (int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
                        }
                    }
                }
                if (ptr_param_b->CAM_CTL_DMA_EN.Bits.AFO_EN)
                {
                    ptr_param_b->CAM_AFO_XSIZE.Bits.XSIZE = (int)ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM*16*
                        ((int)ptr_param_b->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
                    if (ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE < 16)
                    {
                        LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                        return -1;
                    }
                    /* RTL verif or platform */
                    if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                    {
                        ptr_param_b->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN ?
                            ((int)ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM*16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1)):0;
                        ptr_param_b->CAM_AFO_YSIZE.Raw = ptr_param_a->CAM_AFO_YSIZE.Raw;
                    }
                    if ((unsigned int)ptr_param_b->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_param_b->CAM_AFO_XSIZE.Bits.XSIZE + 1 >
                        (unsigned int)ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE)
                    {
                        if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                            return -1;
                        }
                        else
                        {
                            /* RTL */
                            ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE -
                                (unsigned int)ptr_param_b->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_param_b->CAM_AF_CON.Bits.AF_EXT_STAT_EN);
                            ptr_param_b->CAM_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_param_b->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
                                (int)ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
                        }
                    }
                    if ((ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN ? (ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE + 1):0) +
                        ptr_param_b->CAM_AFO_XSIZE.Bits.XSIZE + 1 > ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE)
                    {
                        if (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR");
                            return -1;
                        }
                    }
                }
                /* AFO_A padding */
                if ((false == ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN) &&
                    (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR))
                {
                    LOG_DBG("CCU_TEST  AFO_A padding   (TWIN_AF_BLOCK_XNUM(%d)+1)*16*(AF_EXT_STAT_EN+1)<=STRIDE(%d)",
                           TWIN_AF_BLOCK_XNUM,
                           ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN,
                           ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE);
                    if ((TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
                        (int)ptr_param_a->CAM_AFO_STRIDE.Bits.STRIDE)
                    {
                        ptr_param_a->CAM_AF_VLD.Bits.AF_VLD_XSTART = 0;
                        ptr_param_a->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
                        ptr_param_a->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 16*TWIN_AF_BLOCK_XNUM*
                            ((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1);
                        ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
                        ptr_param_a->CAM_CTL_EN.Bits.AF_EN = true;
                        ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN = true;
                    }
                }
                if (ptr_param_a->CAM_CTL_DMA_EN.Bits.AFO_EN)
                {
                    /* error check A */
                    if ((3 == ptr_param_a->CAM_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_param_a->CAM_AF_CON.Bits.AF_V_GONLY))
                    {
                        if (ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                    else if ((3 == ptr_param_a->CAM_AF_CON.Bits.AF_V_AVG_LVL) ||
                        ((2 == ptr_param_a->CAM_AF_CON.Bits.AF_V_AVG_LVL) &&
                        (1 == ptr_param_a->CAM_AF_CON.Bits.AF_V_GONLY)))
                    {
                        if (ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                    else
                    {
                        if (ptr_param_a->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                }
                /* AFO_B padding */
                if ((false == ptr_param_b->CAM_CTL_DMA_EN.Bits.AFO_EN) &&
                    (ptr_param_a->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_param_b->CAM_AFO_BASE_ADDR.Bits.BASE_ADDR))
                {
                    LOG_DBG("CCU_TEST  AFO_B padding   (TWIN_AF_BLOCK_XNUM(%d)+1)*16*(AF_EXT_STAT_EN+1)<=STRIDE(%d)",
                           TWIN_AF_BLOCK_XNUM,
                           ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN,
                           ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE);
                    if ((TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
                        (int)ptr_param_b->CAM_AFO_STRIDE.Bits.STRIDE)
                    {
                        ptr_param_b->CAM_AF_VLD.Bits.AF_VLD_XSTART = 0;
                        ptr_param_b->CAM_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
                        ptr_param_b->CAM_AFO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_param_a->CAM_AFO_XSIZE.Bits.XSIZE + 1;
                        ptr_param_b->CAM_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_param_a->CAM_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
                        ptr_param_b->CAM_CTL_EN.Bits.AF_EN = true;
                        ptr_param_b->CAM_CTL_DMA_EN.Bits.AFO_EN = true;
                    }
                }
                if (ptr_param_b->CAM_CTL_DMA_EN.Bits.AFO_EN)
                {
                    /* error check B */
                    if ((3 == ptr_param_b->CAM_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_param_b->CAM_AF_CON.Bits.AF_V_GONLY))
                    {
                        if (ptr_param_b->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                    else if ((3 == ptr_param_b->CAM_AF_CON.Bits.AF_V_AVG_LVL) ||
                        ((2 == ptr_param_b->CAM_AF_CON.Bits.AF_V_AVG_LVL) &&
                        (1 == ptr_param_b->CAM_AF_CON.Bits.AF_V_GONLY)))
                    {
                        if (ptr_param_b->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                    else
                    {
                        if (ptr_param_b->CAM_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
                        {
                            LOG_ERR("CCU_DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR");
                            return -1;
                        }
                    }
                }
            }
            /* sync RCP size & AF_IMAGE_WD */
            ptr_param_a->CAM_AF_SIZE.Bits.AF_IMAGE_WD = rcp_a_width_x;
            ptr_param_b->CAM_AF_SIZE.Bits.AF_IMAGE_WD = rcp_b_width_x;
        }
    }
#endif
    return 0;
}
