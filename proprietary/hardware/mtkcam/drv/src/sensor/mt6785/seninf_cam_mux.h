#ifndef __seninf_cam_mux_REGS_H__
#define __seninf_cam_mux_REGS_H__

// ----------------- seninf_cam_mux Bit Field Definitions -------------------

//#define PACKING volatile
typedef unsigned int FIELD;
typedef unsigned int UINT32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux0_src_sel : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_cam_mux1_src_sel : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_cam_mux2_src_sel : 4;
        FIELD rsv_20                    : 4;
        FIELD rg_seninf_cam_mux3_src_sel : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_CTRL_0, *PREG_SENINF_CAM_MUX_CTRL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux4_src_sel : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_cam_mux5_src_sel : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_cam_mux6_src_sel : 4;
        FIELD rsv_20                    : 4;
        FIELD rg_seninf_cam_mux7_src_sel : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_CTRL_1, *PREG_SENINF_CAM_MUX_CTRL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux8_src_sel : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_cam_mux9_src_sel : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_cam_mux10_src_sel : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_CTRL_2, *PREG_SENINF_CAM_MUX_CTRL_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_cam_mux0_en        : 1;
        FIELD seninf_cam_mux1_en        : 1;
        FIELD seninf_cam_mux2_en        : 1;
        FIELD seninf_cam_mux3_en        : 1;
        FIELD seninf_cam_mux4_en        : 1;
        FIELD seninf_cam_mux5_en        : 1;
        FIELD seninf_cam_mux6_en        : 1;
        FIELD seninf_cam_mux7_en        : 1;
        FIELD seninf_cam_mux8_en        : 1;
        FIELD seninf_cam_mux9_en        : 1;
        FIELD seninf_cam_mux10_en       : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_EN, *PREG_SENINF_CAM_MUX_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_cam_mux0_chk_en    : 1;
        FIELD seninf_cam_mux1_chk_en    : 1;
        FIELD seninf_cam_mux2_chk_en    : 1;
        FIELD seninf_cam_mux3_chk_en    : 1;
        FIELD seninf_cam_mux4_chk_en    : 1;
        FIELD seninf_cam_mux5_chk_en    : 1;
        FIELD seninf_cam_mux6_chk_en    : 1;
        FIELD seninf_cam_mux7_chk_en    : 1;
        FIELD seninf_cam_mux8_chk_en    : 1;
        FIELD seninf_cam_mux9_chk_en    : 1;
        FIELD seninf_cam_mux10_chk_en   : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_CHK_EN, *PREG_SENINF_CAM_MUX_CHK_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux0_vc_en  : 1;
        FIELD rg_seninf_cam_mux0_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux0_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux0_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX0_OPT, *PREG_SENINF_CAM_MUX0_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux1_vc_en  : 1;
        FIELD rg_seninf_cam_mux1_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux1_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux1_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX1_OPT, *PREG_SENINF_CAM_MUX1_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux2_vc_en  : 1;
        FIELD rg_seninf_cam_mux2_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux2_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux2_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX2_OPT, *PREG_SENINF_CAM_MUX2_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux3_vc_en  : 1;
        FIELD rg_seninf_cam_mux3_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux3_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux3_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX3_OPT, *PREG_SENINF_CAM_MUX3_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux4_vc_en  : 1;
        FIELD rg_seninf_cam_mux4_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux4_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux4_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX4_OPT, *PREG_SENINF_CAM_MUX4_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux5_vc_en  : 1;
        FIELD rg_seninf_cam_mux5_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux5_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux5_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX5_OPT, *PREG_SENINF_CAM_MUX5_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux6_vc_en  : 1;
        FIELD rg_seninf_cam_mux6_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux6_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux6_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX6_OPT, *PREG_SENINF_CAM_MUX6_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux7_vc_en  : 1;
        FIELD rg_seninf_cam_mux7_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux7_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux7_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX7_OPT, *PREG_SENINF_CAM_MUX7_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux8_vc_en  : 1;
        FIELD rg_seninf_cam_mux8_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux8_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux8_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX8_OPT, *PREG_SENINF_CAM_MUX8_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux9_vc_en  : 1;
        FIELD rg_seninf_cam_mux9_dt_en  : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux9_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux9_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX9_OPT, *PREG_SENINF_CAM_MUX9_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux10_vc_en : 1;
        FIELD rg_seninf_cam_mux10_dt_en : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_cam_mux10_vc_sel : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_cam_mux10_dt_sel : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX10_OPT, *PREG_SENINF_CAM_MUX10_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_cam_mux_sw_rst     : 1;
        FIELD seninf_cam_mux_irq_sw_rst : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_CTRL, *PREG_SENINF_CAM_MUX_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux0_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux0_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux1_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux1_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux2_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux2_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux3_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux3_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux4_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux4_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux5_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux5_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux6_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux6_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux7_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux7_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux8_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux8_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux9_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux9_vsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux10_hsize_err_irq_en : 1;
        FIELD rg_seninf_cam_mux10_vsize_err_irq_en : 1;
        FIELD rsv_22                    : 9;
        FIELD rg_seninf_cam_mux_irq_clr_mode : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_IRQ_EN, *PREG_SENINF_CAM_MUX_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux0_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux0_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux1_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux1_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux2_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux2_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux3_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux3_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux4_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux4_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux5_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux5_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux6_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux6_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux7_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux7_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux8_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux8_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux9_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux9_vsize_err_irq : 1;
        FIELD ro_seninf_cam_mux10_hsize_err_irq : 1;
        FIELD ro_seninf_cam_mux10_vsize_err_irq : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_IRQ_STATUS, *PREG_SENINF_CAM_MUX_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux_dbg_en  : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_seninf_cam_mux_dbg_sel : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_DBG_CTRL, *PREG_SENINF_CAM_MUX_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux_dbg_out : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX_DBG_OUT, *PREG_SENINF_CAM_MUX_DBG_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_top_spare_0     : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_seninf_top_spare_1     : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_SPARE, *PREG_SENINF_CAM_SPARE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux0_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX0_CHK_CTL_0, *PREG_SENINF_CAM_MUX0_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux0_exp_hsize : 16;
        FIELD rg_seninf_cam_mux0_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX0_CHK_CTL_1, *PREG_SENINF_CAM_MUX0_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux0_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux0_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX0_CHK_RES, *PREG_SENINF_CAM_MUX0_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux0_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux0_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX0_CHK_ERR_RES, *PREG_SENINF_CAM_MUX0_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux1_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX1_CHK_CTL_0, *PREG_SENINF_CAM_MUX1_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux1_exp_hsize : 16;
        FIELD rg_seninf_cam_mux1_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX1_CHK_CTL_1, *PREG_SENINF_CAM_MUX1_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux1_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux1_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX1_CHK_RES, *PREG_SENINF_CAM_MUX1_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux1_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux1_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX1_CHK_ERR_RES, *PREG_SENINF_CAM_MUX1_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux2_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX2_CHK_CTL_0, *PREG_SENINF_CAM_MUX2_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux2_exp_hsize : 16;
        FIELD rg_seninf_cam_mux2_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX2_CHK_CTL_1, *PREG_SENINF_CAM_MUX2_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux2_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux2_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX2_CHK_RES, *PREG_SENINF_CAM_MUX2_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux2_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux2_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX2_CHK_ERR_RES, *PREG_SENINF_CAM_MUX2_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux3_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX3_CHK_CTL_0, *PREG_SENINF_CAM_MUX3_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux3_exp_hsize : 16;
        FIELD rg_seninf_cam_mux3_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX3_CHK_CTL_1, *PREG_SENINF_CAM_MUX3_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux3_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux3_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX3_CHK_RES, *PREG_SENINF_CAM_MUX3_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux3_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux3_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX3_CHK_ERR_RES, *PREG_SENINF_CAM_MUX3_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux4_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX4_CHK_CTL_0, *PREG_SENINF_CAM_MUX4_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux4_exp_hsize : 16;
        FIELD rg_seninf_cam_mux4_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX4_CHK_CTL_1, *PREG_SENINF_CAM_MUX4_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux4_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux4_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX4_CHK_RES, *PREG_SENINF_CAM_MUX4_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux4_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux4_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX4_CHK_ERR_RES, *PREG_SENINF_CAM_MUX4_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux5_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX5_CHK_CTL_0, *PREG_SENINF_CAM_MUX5_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux5_exp_hsize : 16;
        FIELD rg_seninf_cam_mux5_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX5_CHK_CTL_1, *PREG_SENINF_CAM_MUX5_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux5_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux5_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX5_CHK_RES, *PREG_SENINF_CAM_MUX5_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux5_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux5_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX5_CHK_ERR_RES, *PREG_SENINF_CAM_MUX5_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux6_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX6_CHK_CTL_0, *PREG_SENINF_CAM_MUX6_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux6_exp_hsize : 16;
        FIELD rg_seninf_cam_mux6_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX6_CHK_CTL_1, *PREG_SENINF_CAM_MUX6_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux6_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux6_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX6_CHK_RES, *PREG_SENINF_CAM_MUX6_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux6_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux6_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX6_CHK_ERR_RES, *PREG_SENINF_CAM_MUX6_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux7_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX7_CHK_CTL_0, *PREG_SENINF_CAM_MUX7_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux7_exp_hsize : 16;
        FIELD rg_seninf_cam_mux7_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX7_CHK_CTL_1, *PREG_SENINF_CAM_MUX7_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux7_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux7_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX7_CHK_RES, *PREG_SENINF_CAM_MUX7_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux7_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux7_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX7_CHK_ERR_RES, *PREG_SENINF_CAM_MUX7_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux8_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX8_CHK_CTL_0, *PREG_SENINF_CAM_MUX8_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux8_exp_hsize : 16;
        FIELD rg_seninf_cam_mux8_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX8_CHK_CTL_1, *PREG_SENINF_CAM_MUX8_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux8_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux8_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX8_CHK_RES, *PREG_SENINF_CAM_MUX8_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux8_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux8_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX8_CHK_ERR_RES, *PREG_SENINF_CAM_MUX8_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux9_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX9_CHK_CTL_0, *PREG_SENINF_CAM_MUX9_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux9_exp_hsize : 16;
        FIELD rg_seninf_cam_mux9_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX9_CHK_CTL_1, *PREG_SENINF_CAM_MUX9_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux9_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux9_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX9_CHK_RES, *PREG_SENINF_CAM_MUX9_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux9_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux9_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX9_CHK_ERR_RES, *PREG_SENINF_CAM_MUX9_CHK_ERR_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux10_pix_mode_sel : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX10_CHK_CTL_0, *PREG_SENINF_CAM_MUX10_CHK_CTL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_cam_mux10_exp_hsize : 16;
        FIELD rg_seninf_cam_mux10_exp_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX10_CHK_CTL_1, *PREG_SENINF_CAM_MUX10_CHK_CTL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux10_rcv_hsize : 16;
        FIELD ro_seninf_cam_mux10_rcv_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX10_CHK_RES, *PREG_SENINF_CAM_MUX10_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_cam_mux10_rcv_err_hsize : 16;
        FIELD ro_seninf_cam_mux10_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CAM_MUX10_CHK_ERR_RES, *PREG_SENINF_CAM_MUX10_CHK_ERR_RES;

// ----------------- seninf_cam_mux  Grouping Definitions -------------------
// ----------------- seninf_cam_mux Register Definition -------------------
typedef volatile struct /*0x1A004400*/
{
    REG_SENINF_CAM_MUX_CTRL_0       SENINF_CAM_MUX_CTRL_0; // 4400
    REG_SENINF_CAM_MUX_CTRL_1       SENINF_CAM_MUX_CTRL_1; // 4404
    REG_SENINF_CAM_MUX_CTRL_2       SENINF_CAM_MUX_CTRL_2; // 4408
    UINT32                          rsv_440C;         // 440C
    REG_SENINF_CAM_MUX_EN           SENINF_CAM_MUX_EN; // 4410
    UINT32                          rsv_4414;         // 4414
    REG_SENINF_CAM_MUX_CHK_EN       SENINF_CAM_MUX_CHK_EN; // 4418
    UINT32                          rsv_441C;         // 441C
    REG_SENINF_CAM_MUX0_OPT         SENINF_CAM_MUX0_OPT; // 4420
    REG_SENINF_CAM_MUX1_OPT         SENINF_CAM_MUX1_OPT; // 4424
    REG_SENINF_CAM_MUX2_OPT         SENINF_CAM_MUX2_OPT; // 4428
    REG_SENINF_CAM_MUX3_OPT         SENINF_CAM_MUX3_OPT; // 442C
    REG_SENINF_CAM_MUX4_OPT         SENINF_CAM_MUX4_OPT; // 4430
    REG_SENINF_CAM_MUX5_OPT         SENINF_CAM_MUX5_OPT; // 4434
    REG_SENINF_CAM_MUX6_OPT         SENINF_CAM_MUX6_OPT; // 4438
    REG_SENINF_CAM_MUX7_OPT         SENINF_CAM_MUX7_OPT; // 443C
    REG_SENINF_CAM_MUX8_OPT         SENINF_CAM_MUX8_OPT; // 4440
    REG_SENINF_CAM_MUX9_OPT         SENINF_CAM_MUX9_OPT; // 4444
    REG_SENINF_CAM_MUX10_OPT        SENINF_CAM_MUX10_OPT; // 4448
    UINT32                          rsv_444C[13];     // 444C..447C
    REG_SENINF_CAM_MUX_CTRL         SENINF_CAM_MUX_CTRL; // 4480
    UINT32                          rsv_4484[7];      // 4484..449C
    REG_SENINF_CAM_MUX_IRQ_EN       SENINF_CAM_MUX_IRQ_EN; // 44A0
    UINT32                          rsv_44A4;         // 44A4
    REG_SENINF_CAM_MUX_IRQ_STATUS   SENINF_CAM_MUX_IRQ_STATUS; // 44A8
    UINT32                          rsv_44AC[5];      // 44AC..44BC
    REG_SENINF_CAM_MUX_DBG_CTRL     SENINF_CAM_MUX_DBG_CTRL; // 44C0
    REG_SENINF_CAM_MUX_DBG_OUT      SENINF_CAM_MUX_DBG_OUT; // 44C4
    UINT32                          rsv_44C8[12];     // 44C8..44F4
    REG_SENINF_CAM_SPARE            SENINF_CAM_SPARE; // 44F8
    UINT32                          rsv_44FC;         // 44FC
    REG_SENINF_CAM_MUX0_CHK_CTL_0   SENINF_CAM_MUX0_CHK_CTL_0; // 4500
    REG_SENINF_CAM_MUX0_CHK_CTL_1   SENINF_CAM_MUX0_CHK_CTL_1; // 4504
    REG_SENINF_CAM_MUX0_CHK_RES     SENINF_CAM_MUX0_CHK_RES; // 4508
    REG_SENINF_CAM_MUX0_CHK_ERR_RES SENINF_CAM_MUX0_CHK_ERR_RES; // 450C
    REG_SENINF_CAM_MUX1_CHK_CTL_0   SENINF_CAM_MUX1_CHK_CTL_0; // 4510
    REG_SENINF_CAM_MUX1_CHK_CTL_1   SENINF_CAM_MUX1_CHK_CTL_1; // 4514
    REG_SENINF_CAM_MUX1_CHK_RES     SENINF_CAM_MUX1_CHK_RES; // 4518
    REG_SENINF_CAM_MUX1_CHK_ERR_RES SENINF_CAM_MUX1_CHK_ERR_RES; // 451C
    REG_SENINF_CAM_MUX2_CHK_CTL_0   SENINF_CAM_MUX2_CHK_CTL_0; // 4520
    REG_SENINF_CAM_MUX2_CHK_CTL_1   SENINF_CAM_MUX2_CHK_CTL_1; // 4524
    REG_SENINF_CAM_MUX2_CHK_RES     SENINF_CAM_MUX2_CHK_RES; // 4528
    REG_SENINF_CAM_MUX2_CHK_ERR_RES SENINF_CAM_MUX2_CHK_ERR_RES; // 452C
    REG_SENINF_CAM_MUX3_CHK_CTL_0   SENINF_CAM_MUX3_CHK_CTL_0; // 4530
    REG_SENINF_CAM_MUX3_CHK_CTL_1   SENINF_CAM_MUX3_CHK_CTL_1; // 4534
    REG_SENINF_CAM_MUX3_CHK_RES     SENINF_CAM_MUX3_CHK_RES; // 4538
    REG_SENINF_CAM_MUX3_CHK_ERR_RES SENINF_CAM_MUX3_CHK_ERR_RES; // 453C
    REG_SENINF_CAM_MUX4_CHK_CTL_0   SENINF_CAM_MUX4_CHK_CTL_0; // 4540
    REG_SENINF_CAM_MUX4_CHK_CTL_1   SENINF_CAM_MUX4_CHK_CTL_1; // 4544
    REG_SENINF_CAM_MUX4_CHK_RES     SENINF_CAM_MUX4_CHK_RES; // 4548
    REG_SENINF_CAM_MUX4_CHK_ERR_RES SENINF_CAM_MUX4_CHK_ERR_RES; // 454C
    REG_SENINF_CAM_MUX5_CHK_CTL_0   SENINF_CAM_MUX5_CHK_CTL_0; // 4550
    REG_SENINF_CAM_MUX5_CHK_CTL_1   SENINF_CAM_MUX5_CHK_CTL_1; // 4554
    REG_SENINF_CAM_MUX5_CHK_RES     SENINF_CAM_MUX5_CHK_RES; // 4558
    REG_SENINF_CAM_MUX5_CHK_ERR_RES SENINF_CAM_MUX5_CHK_ERR_RES; // 455C
    REG_SENINF_CAM_MUX6_CHK_CTL_0   SENINF_CAM_MUX6_CHK_CTL_0; // 4560
    REG_SENINF_CAM_MUX6_CHK_CTL_1   SENINF_CAM_MUX6_CHK_CTL_1; // 4564
    REG_SENINF_CAM_MUX6_CHK_RES     SENINF_CAM_MUX6_CHK_RES; // 4568
    REG_SENINF_CAM_MUX6_CHK_ERR_RES SENINF_CAM_MUX6_CHK_ERR_RES; // 456C
    REG_SENINF_CAM_MUX7_CHK_CTL_0   SENINF_CAM_MUX7_CHK_CTL_0; // 4570
    REG_SENINF_CAM_MUX7_CHK_CTL_1   SENINF_CAM_MUX7_CHK_CTL_1; // 4574
    REG_SENINF_CAM_MUX7_CHK_RES     SENINF_CAM_MUX7_CHK_RES; // 4578
    REG_SENINF_CAM_MUX7_CHK_ERR_RES SENINF_CAM_MUX7_CHK_ERR_RES; // 457C
    REG_SENINF_CAM_MUX8_CHK_CTL_0   SENINF_CAM_MUX8_CHK_CTL_0; // 4580
    REG_SENINF_CAM_MUX8_CHK_CTL_1   SENINF_CAM_MUX8_CHK_CTL_1; // 4584
    REG_SENINF_CAM_MUX8_CHK_RES     SENINF_CAM_MUX8_CHK_RES; // 4588
    REG_SENINF_CAM_MUX8_CHK_ERR_RES SENINF_CAM_MUX8_CHK_ERR_RES; // 458C
    REG_SENINF_CAM_MUX9_CHK_CTL_0   SENINF_CAM_MUX9_CHK_CTL_0; // 4590
    REG_SENINF_CAM_MUX9_CHK_CTL_1   SENINF_CAM_MUX9_CHK_CTL_1; // 4594
    REG_SENINF_CAM_MUX9_CHK_RES     SENINF_CAM_MUX9_CHK_RES; // 4598
    REG_SENINF_CAM_MUX9_CHK_ERR_RES SENINF_CAM_MUX9_CHK_ERR_RES; // 459C
    REG_SENINF_CAM_MUX10_CHK_CTL_0  SENINF_CAM_MUX10_CHK_CTL_0; // 45A0
    REG_SENINF_CAM_MUX10_CHK_CTL_1  SENINF_CAM_MUX10_CHK_CTL_1; // 45A4
    REG_SENINF_CAM_MUX10_CHK_RES    SENINF_CAM_MUX10_CHK_RES; // 45A8
    REG_SENINF_CAM_MUX10_CHK_ERR_RES SENINF_CAM_MUX10_CHK_ERR_RES; // 45AC
}seninf_cam_mux_REGS, *Pseninf_cam_mux_REGS;

#endif // __seninf_cam_mux_REGS_H__
