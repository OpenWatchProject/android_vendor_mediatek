#ifndef __mipi_rx_ana_cdphy_csi0a_reg_REGS_H__
#define __mipi_rx_ana_cdphy_csi0a_reg_REGS_H__

typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_BG_CORE_EN        : 1;
        FIELD RG_CSI0_BG_LPF_EN         : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_EQ_PROTECT_EN     : 1;
        FIELD RG_CSI0_BYTE_CK_RSTB_SEL  : 1;
        FIELD rsv_6                     : 2;
        FIELD RG_CSI0_DPHY_L0_CKSEL     : 1;
        FIELD RG_CSI0_DPHY_L1_CKSEL     : 1;
        FIELD RG_CSI0_DPHY_L2_CKSEL     : 1;
        FIELD rsv_11                    : 1;
        FIELD RG_CSI0_DPHY_L0_CKMODE_EN : 1;
        FIELD RG_CSI0_DPHY_L1_CKMODE_EN : 1;
        FIELD RG_CSI0_DPHY_L2_CKMODE_EN : 1;
        FIELD rsv_15                    : 1;
        FIELD RG_CSI0_CDPHY_L0_T0_BYTECK_INVERT : 1;
        FIELD RG_CSI0_DPHY_L1_BYTECK_INVERT : 1;
        FIELD RG_CSI0_CDPHY_L2_T1_BYTECK_INVERT : 1;
        FIELD rsv_19                    : 1;
        FIELD RG_CSI0_CPHY_EN           : 1;
        FIELD RG_CSI0_CPHY_T0_CDR_FIRST_EDGE_EN : 1;
        FIELD RG_CSI0_CPHY_T1_CDR_FIRST_EDGE_EN : 1;
        FIELD rsv_23                    : 1;
        FIELD RG_CSI0_CDPHY_L0_T0_FORCE_INIT : 1;
        FIELD RG_CSI0_DPHY_L1_FORCE_INIT : 1;
        FIELD RG_CSI0_CDPHY_L2_T1_FORCE_INIT : 1;
        FIELD rsv_27                    : 1;
        FIELD RG_CSI0_CDPHY_L0_T0_SYNC_INIT_CTRL : 1;
        FIELD RG_CSI0_DPHY_L1_SYNC_INIT_CTRL : 1;
        FIELD RG_CSI0_CDPHY_L2_T1_SYNC_INIT_CTRL : 1;
        FIELD RG_CSI0_FORCE_HSRT_EN     : 1;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_0, *PREG_CDPHY_RX_ANA_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_BG_LPRX_VTL_SEL   : 3;
        FIELD rsv_3                     : 1;
        FIELD RG_CSI0_BG_LPRX_VTH_SEL   : 3;
        FIELD rsv_7                     : 1;
        FIELD RG_CSI0_BG_VREF_SEL       : 4;
        FIELD RG_CSI0_CDPHY_DELAYCAL_CK_SEL : 3;
        FIELD rsv_15                    : 1;
        FIELD RG_CSI0_CDPHY_EQ_DES_VREF_SEL : 3;
        FIELD rsv_19                    : 1;
        FIELD RG_CSI0_CDPHY_DELAY_VREF_SEL : 3;
        FIELD rsv_23                    : 1;
        FIELD RG_CSI0_DPHY_L0_DELAYCAL_EN : 1;
        FIELD RG_CSI0_DPHY_L1_DELAYCAL_EN : 1;
        FIELD RG_CSI0_DPHY_L2_DELAYCAL_EN : 1;
        FIELD rsv_27                    : 1;
        FIELD RG_CSI0_DPHY_L0_DELAYCAL_RSTB : 1;
        FIELD RG_CSI0_DPHY_L1_DELAYCAL_RSTB : 1;
        FIELD RG_CSI0_DPHY_L2_DELAYCAL_RSTB : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_1, *PREG_CDPHY_RX_ANA_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_L0P_T0A_HSRT_CODE : 5;
        FIELD rsv_5                     : 3;
        FIELD RG_CSI0_L0N_T0B_HSRT_CODE : 5;
        FIELD rsv_13                    : 3;
        FIELD RG_CSI0_CPHY_T0_CDR_SELF_CAL_EN : 1;
        FIELD RG_CSI0_CPHY_T1_CDR_SELF_CAL_EN : 1;
        FIELD rsv_18                    : 6;
        FIELD RG_CSI0_BG_ALP_RX_VTH_SEL : 3;
        FIELD rsv_27                    : 1;
        FIELD RG_CSI0_BG_ALP_RX_VTL_SEL : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_2, *PREG_CDPHY_RX_ANA_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_L1P_T0C_HSRT_CODE : 5;
        FIELD rsv_5                     : 3;
        FIELD RG_CSI0_L1N_T1A_HSRT_CODE : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi0_os_cal_cnt_sel    : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_csi0_eq_des_vref_sel   : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_3, *PREG_CDPHY_RX_ANA_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_L2P_T1B_HSRT_CODE : 5;
        FIELD rsv_5                     : 3;
        FIELD RG_CSI0_L2N_T1C_HSRT_CODE : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_4, *PREG_CDPHY_RX_ANA_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_CDPHY_EQ_BW       : 2;
        FIELD RG_CSI0_CDPHY_EQ_IS       : 2;
        FIELD RG_CSI0_CDPHY_EQ_DG0_EN   : 1;
        FIELD RG_CSI0_CDPHY_EQ_DG1_EN   : 1;
        FIELD RG_CSI0_CDPHY_EQ_LATCH_EN : 1;
        FIELD rsv_7                     : 1;
        FIELD RG_CSI0_CDPHY_EQ_SR0      : 4;
        FIELD RG_CSI0_CDPHY_EQ_SR1      : 4;
        FIELD RG_CSI0_L0_T0AB_EQ_MON_EN : 1;
        FIELD RG_CSI0_L1_T1AB_EQ_MON_EN : 1;
        FIELD RG_CSI0_L2_T1BC_EQ_MON_EN : 1;
        FIELD rsv_19                    : 5;
        FIELD RG_CSI0_XX_T0BC_EQ_MON_EN : 1;
        FIELD RG_CSI0_XX_T0CA_EQ_MON_EN : 1;
        FIELD RG_CSI0_XX_T1CA_EQ_MON_EN : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_5, *PREG_CDPHY_RX_ANA_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_CPHY_T0_CDR_AB_WIDTH : 6;
        FIELD rsv_6                     : 2;
        FIELD RG_CSI0_CPHY_T0_CDR_BC_WIDTH : 6;
        FIELD rsv_14                    : 2;
        FIELD RG_CSI0_CPHY_T0_CDR_CA_WIDTH : 6;
        FIELD rsv_22                    : 2;
        FIELD RG_CSI0_CPHY_T0_CDR_CK_DELAY : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_6, *PREG_CDPHY_RX_ANA_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RG_CSI0_CPHY_T1_CDR_AB_WIDTH : 6;
        FIELD rsv_6                     : 2;
        FIELD RG_CSI0_CPHY_T1_CDR_BC_WIDTH : 6;
        FIELD rsv_14                    : 2;
        FIELD RG_CSI0_CPHY_T1_CDR_CA_WIDTH : 6;
        FIELD rsv_22                    : 2;
        FIELD RG_CSI0_CPHY_T1_CDR_CK_DELAY : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_7, *PREG_CDPHY_RX_ANA_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD RGS_CSI0_CDPHY_L0_T0AB_OS_CAL_CPLT : 1;
        FIELD RGS_CSI0_CDPHY_L1_T1AB_OS_CAL_CPLT : 1;
        FIELD RGS_CSI0_CDPHY_L2_T1BC_OS_CAL_CPLT : 1;
        FIELD RGS_CSI0_CPHY_T0BC_OS_CAL_CPLT : 1;
        FIELD RGS_CSI0_CPHY_T0CA_OS_CAL_CPLT : 1;
        FIELD RGS_CSI0_CPHY_T1CA_OS_CAL_CPLT : 1;
        FIELD rsv_6                     : 2;
        FIELD RGS_CSI0_OS_CAL_CODE      : 8;
        FIELD RG_CSI0_L0_T0AB_EQ_OS_CAL_EN : 1;
        FIELD RG_CSI0_L1_T1AB_EQ_OS_CAL_EN : 1;
        FIELD RG_CSI0_L2_T1BC_EQ_OS_CAL_EN : 1;
        FIELD RG_CSI0_XX_T0BC_EQ_OS_CAL_EN : 1;
        FIELD RG_CSI0_XX_T0CA_EQ_OS_CAL_EN : 1;
        FIELD RG_CSI0_XX_T1CA_EQ_OS_CAL_EN : 1;
        FIELD rsv_22                    : 2;
        FIELD RG_CSI0_OS_CAL_SEL        : 3;
        FIELD rsv_27                    : 1;
        FIELD RG_CSI0_OS_CAL_DIV        : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_8, *PREG_CDPHY_RX_ANA_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 24;
        FIELD RG_CSI0_RESERVE           : 8;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_9, *PREG_CDPHY_RX_ANA_9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_cdphy_l0p_t0a_lprx_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l0n_t0b_lprx_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_val_da_csi0_cdphy_l1p_t0c_lprx_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l1n_t1a_lprx_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2p_t1b_lprx_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2n_t1c_lprx_en : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_cdphy_l0p_t0a_lprx_en : 1;
        FIELD ro_da_csi0_cdphy_l0n_t0b_lprx_en : 1;
        FIELD rsv_18                    : 2;
        FIELD ro_da_csi0_cdphy_l1p_t0c_lprx_en : 1;
        FIELD ro_da_csi0_cdphy_l1n_t1a_lprx_en : 1;
        FIELD rsv_22                    : 2;
        FIELD ro_da_csi0_cdphy_l2p_t1b_lprx_en : 1;
        FIELD ro_da_csi0_cdphy_l2n_t1c_lprx_en : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_0, *PREG_CDPHY_RX_ANA_FORCE_MODE_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_cdphy_l0p_t0a_lprx_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l0n_t0b_lprx_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_en_da_csi0_cdphy_l1p_t0c_lprx_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l1n_t1a_lprx_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2p_t1b_lprx_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2n_t1c_lprx_en : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_0, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_ad_csi0_cdphy_l0p_t0a_lprx_out : 1;
        FIELD ro_ad_csi0_cdphy_l0n_t0b_lprx_out : 1;
        FIELD rsv_2                     : 2;
        FIELD ro_ad_csi0_cdphy_l1p_t0c_lprx_out : 1;
        FIELD ro_ad_csi0_cdphy_l1n_t1a_lprx_out : 1;
        FIELD rsv_6                     : 2;
        FIELD ro_ad_csi0_cdphy_l2p_t1b_lprx_out : 1;
        FIELD ro_ad_csi0_cdphy_l2n_t1c_lprx_out : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_AD_0, *PREG_CDPHY_RX_ANA_AD_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_dphy_l0_alprx_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_alprx_en : 1;
        FIELD rsv_5                     : 3;
        FIELD rg_sw_force_val_da_csi0_dphy_l2_alprx_en : 1;
        FIELD rsv_9                     : 7;
        FIELD ro_da_csi0_dphy_l0_alprx_en : 1;
        FIELD rsv_17                    : 3;
        FIELD ro_da_csi0_dphy_l1_alprx_en : 1;
        FIELD rsv_21                    : 3;
        FIELD ro_da_csi0_dphy_l2_alprx_en : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_1, *PREG_CDPHY_RX_ANA_FORCE_MODE_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_dphy_l0_alprx_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_alprx_en : 1;
        FIELD rsv_5                     : 3;
        FIELD rg_sw_force_en_da_csi0_dphy_l2_alprx_en : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_1, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_ad_csi0_dphy_l0_alprx_out : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_ad_csi0_dphy_l1_alprx_out : 1;
        FIELD rsv_5                     : 3;
        FIELD ro_ad_csi0_dphy_l2_alprx_out : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_AD_1, *PREG_CDPHY_RX_ANA_AD_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_cdphy_l0p_t0a_hsrt_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l0n_t0b_hsrt_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_val_da_csi0_cdphy_l1p_t0c_hsrt_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l1n_t1a_hsrt_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2p_t1b_hsrt_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2n_t1c_hsrt_en : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_cdphy_l0p_t0a_hsrt_en : 1;
        FIELD ro_da_csi0_cdphy_l0n_t0b_hsrt_en : 1;
        FIELD rsv_18                    : 2;
        FIELD ro_da_csi0_cdphy_l1p_t0c_hsrt_en : 1;
        FIELD ro_da_csi0_cdphy_l1n_t1a_hsrt_en : 1;
        FIELD rsv_22                    : 2;
        FIELD ro_da_csi0_cdphy_l2p_t1b_hsrt_en : 1;
        FIELD ro_da_csi0_cdphy_l2n_t1c_hsrt_en : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_2, *PREG_CDPHY_RX_ANA_FORCE_MODE_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_cdphy_l0p_t0a_hsrt_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l0n_t0b_hsrt_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_en_da_csi0_cdphy_l1p_t0c_hsrt_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l1n_t1a_hsrt_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2p_t1b_hsrt_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2n_t1c_hsrt_en : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_2, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_cdphy_l0_t0ab_eq_en : 1;
        FIELD rg_sw_force_val_da_csi0_cphy_t0ca_eq_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_val_da_csi0_cphy_t0bc_eq_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l1_t1ab_eq_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_val_da_csi0_cphy_t1ca_eq_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2_t1bc_eq_en : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_cdphy_l0_t0ab_eq_en : 1;
        FIELD ro_da_csi0_cphy_t0ca_eq_en : 1;
        FIELD rsv_18                    : 2;
        FIELD ro_da_csi0_cphy_t0bc_eq_en : 1;
        FIELD ro_da_csi0_cdphy_l1_t1ab_eq_en : 1;
        FIELD rsv_22                    : 2;
        FIELD ro_da_csi0_cphy_t1ca_eq_en : 1;
        FIELD ro_da_csi0_cdphy_l2_t1bc_eq_en : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_3, *PREG_CDPHY_RX_ANA_FORCE_MODE_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_cdphy_l0_t0ab_eq_en : 1;
        FIELD rg_sw_force_en_da_csi0_cphy_t0ca_eq_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_en_da_csi0_cphy_t0bc_eq_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l1_t1ab_eq_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_en_da_csi0_cphy_t1ca_eq_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2_t1bc_eq_en : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_3, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_dphy_l0_samp_en : 1;
        FIELD rg_sw_force_val_da_csi0_cphy_t0_cdr_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_samp_en : 1;
        FIELD rg_sw_force_val_da_csi0_cphy_t1_cdr_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_val_da_csi0_dphy_l2_samp_en : 1;
        FIELD rsv_9                     : 7;
        FIELD ro_da_csi0_dphy_l0_samp_en : 1;
        FIELD ro_da_csi0_cphy_t0_cdr_en : 1;
        FIELD rsv_18                    : 2;
        FIELD ro_da_csi0_dphy_l1_samp_en : 1;
        FIELD ro_da_csi0_cphy_t1_cdr_en : 1;
        FIELD rsv_22                    : 2;
        FIELD ro_da_csi0_dphy_l2_samp_en : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_4, *PREG_CDPHY_RX_ANA_FORCE_MODE_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_dphy_l0_samp_en : 1;
        FIELD rg_sw_force_en_da_csi0_cphy_t0_cdr_en : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_samp_en : 1;
        FIELD rg_sw_force_en_da_csi0_cphy_t1_cdr_en : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_sw_force_en_da_csi0_dphy_l2_samp_en : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_4, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_cdphy_l0_t0_des_en : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_des_en : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2_t1_des_en : 1;
        FIELD rsv_3                     : 5;
        FIELD rg_sw_force_val_da_csi0_cdphy_l0_t0_des_sync_init : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_des_sync_init : 1;
        FIELD rg_sw_force_val_da_csi0_cdphy_l2_t1_des_sync_init : 1;
        FIELD rsv_11                    : 5;
        FIELD ro_da_csi0_cdphy_l0_t0_des_en : 1;
        FIELD ro_da_csi0_dphy_l1_des_en : 1;
        FIELD ro_da_csi0_cdphy_l2_t1_des_en : 1;
        FIELD rsv_19                    : 5;
        FIELD ro_da_csi0_cdphy_l0_t0_des_sync_init : 1;
        FIELD ro_da_csi0_dphy_l1_des_sync_init : 1;
        FIELD ro_da_csi0_cdphy_l2_t1_des_sync_init : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_5, *PREG_CDPHY_RX_ANA_FORCE_MODE_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_cdphy_l0_t0_des_en : 1;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_des_en : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2_t1_des_en : 1;
        FIELD rsv_3                     : 5;
        FIELD rg_sw_force_en_da_csi0_cdphy_l0_t0_des_sync_init : 1;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_des_sync_init : 1;
        FIELD rg_sw_force_en_da_csi0_cdphy_l2_t1_des_sync_init : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_5, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_ad_csi0_cphy_t0ca      : 8;
        FIELD ro_ad_csi0_cphy_t0bc      : 8;
        FIELD ro_ad_csi0_cdphy_l0_t0ab  : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_AD_HS_0, *PREG_CDPHY_RX_ANA_AD_HS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD ro_ad_csi0_dphy_l1        : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_AD_HS_1, *PREG_CDPHY_RX_ANA_AD_HS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD ro_ad_csi0_cphy_t1ca      : 8;
        FIELD ro_ad_csi0_cdphy_l2_t1bc  : 8;
        FIELD ro_ad_csi0_cphy_t1ab      : 8;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_AD_HS_2, *PREG_CDPHY_RX_ANA_AD_HS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_dphy_l0_delay_code : 7;
        FIELD rsv_7                     : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l0_delay_en : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l0_delay_apply : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_dphy_l0_delay_code : 7;
        FIELD rsv_23                    : 1;
        FIELD ro_da_csi0_dphy_l0_delay_en : 1;
        FIELD ro_da_csi0_dphy_l0_delay_apply : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_6, *PREG_CDPHY_RX_ANA_FORCE_MODE_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_dphy_l0_delay_code : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_sw_force_en_da_csi0_dphy_l0_delay_en : 1;
        FIELD rg_sw_force_en_da_csi0_dphy_l0_delay_apply : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_6, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_dphy_l1_delay_code : 7;
        FIELD rsv_7                     : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_delay_en : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l1_delay_apply : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_dphy_l1_delay_code : 7;
        FIELD rsv_23                    : 1;
        FIELD ro_da_csi0_dphy_l1_delay_en : 1;
        FIELD ro_da_csi0_dphy_l1_delay_apply : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_7, *PREG_CDPHY_RX_ANA_FORCE_MODE_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_dphy_l1_delay_code : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_delay_en : 1;
        FIELD rg_sw_force_en_da_csi0_dphy_l1_delay_apply : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_7, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_val_da_csi0_dphy_l2_delay_code : 7;
        FIELD rsv_7                     : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l2_delay_en : 1;
        FIELD rg_sw_force_val_da_csi0_dphy_l2_delay_apply : 1;
        FIELD rsv_10                    : 6;
        FIELD ro_da_csi0_dphy_l2_delay_code : 7;
        FIELD rsv_23                    : 1;
        FIELD ro_da_csi0_dphy_l2_delay_en : 1;
        FIELD ro_da_csi0_dphy_l2_delay_apply : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_8, *PREG_CDPHY_RX_ANA_FORCE_MODE_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_sw_force_en_da_csi0_dphy_l2_delay_code : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_sw_force_en_da_csi0_dphy_l2_delay_en : 1;
        FIELD rg_sw_force_en_da_csi0_dphy_l2_delay_apply : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_FORCE_MODE_EN_8, *PREG_CDPHY_RX_ANA_FORCE_MODE_EN_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD csr_csi_clk_mon           : 1;
        FIELD csr_csi_clk_en            : 1;
        FIELD rsv_2                     : 2;
        FIELD csr_async_fifo_gating_sel : 4;
        FIELD csr_csi_mon_mux           : 8;
        FIELD csr_csi_rst_mode          : 2;
        FIELD rsv_18                    : 6;
        FIELD csr_sw_rst                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_SETTING_0, *PREG_CDPHY_RX_ANA_SETTING_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi0_cdphy_fmck_sel    : 2;
        FIELD rsv_2                     : 2;
        FIELD rg_csi0_async_option      : 4;
        FIELD rg_cal_method_3s          : 1;
        FIELD rsv_9                     : 7;
        FIELD rg_csi0_cdphy_spare_reg   : 16;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_SETTING_1, *PREG_CDPHY_RX_ANA_SETTING_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD csi_debug_out             : 32;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_RX_ANA_DBG_OUT, *PREG_CDPHY_RX_ANA_DBG_OUT;

// ----------------- mipi_rx_ana_cdphy_csi0a_reg Register Definition -------------------
typedef volatile struct /*0x11C8 0000*/
{
    REG_CDPHY_RX_ANA_0              CDPHY_RX_ANA_0;   // 0000
    REG_CDPHY_RX_ANA_1              CDPHY_RX_ANA_1;   // 0004
    REG_CDPHY_RX_ANA_2              CDPHY_RX_ANA_2;   // 0008
    REG_CDPHY_RX_ANA_3              CDPHY_RX_ANA_3;   // 000C
    REG_CDPHY_RX_ANA_4              CDPHY_RX_ANA_4;   // 0010
    REG_CDPHY_RX_ANA_5              CDPHY_RX_ANA_5;   // 0014
    REG_CDPHY_RX_ANA_6              CDPHY_RX_ANA_6;   // 0018
    REG_CDPHY_RX_ANA_7              CDPHY_RX_ANA_7;   // 001C
    REG_CDPHY_RX_ANA_8              CDPHY_RX_ANA_8;   // 0020
    REG_CDPHY_RX_ANA_9              CDPHY_RX_ANA_9;   // 0024
    UINT32                          rsv_0028[6];      // 0028..003C
    REG_CDPHY_RX_ANA_FORCE_MODE_0   CDPHY_RX_ANA_FORCE_MODE_0; // 0040
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_0 CDPHY_RX_ANA_FORCE_MODE_EN_0; // 0044
    REG_CDPHY_RX_ANA_AD_0           CDPHY_RX_ANA_AD_0; // 0048
    UINT32                          rsv_004C;         // 004C
    REG_CDPHY_RX_ANA_FORCE_MODE_1   CDPHY_RX_ANA_FORCE_MODE_1; // 0050
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_1 CDPHY_RX_ANA_FORCE_MODE_EN_1; // 0054
    REG_CDPHY_RX_ANA_AD_1           CDPHY_RX_ANA_AD_1; // 0058
    UINT32                          rsv_005C;         // 005C
    REG_CDPHY_RX_ANA_FORCE_MODE_2   CDPHY_RX_ANA_FORCE_MODE_2; // 0060
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_2 CDPHY_RX_ANA_FORCE_MODE_EN_2; // 0064
    UINT32                          rsv_0068[2];      // 0068..006C
    REG_CDPHY_RX_ANA_FORCE_MODE_3   CDPHY_RX_ANA_FORCE_MODE_3; // 0070
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_3 CDPHY_RX_ANA_FORCE_MODE_EN_3; // 0074
    UINT32                          rsv_0078[2];      // 0078..007C
    REG_CDPHY_RX_ANA_FORCE_MODE_4   CDPHY_RX_ANA_FORCE_MODE_4; // 0080
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_4 CDPHY_RX_ANA_FORCE_MODE_EN_4; // 0084
    UINT32                          rsv_0088[2];      // 0088..008C
    REG_CDPHY_RX_ANA_FORCE_MODE_5   CDPHY_RX_ANA_FORCE_MODE_5; // 0090
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_5 CDPHY_RX_ANA_FORCE_MODE_EN_5; // 0094
    UINT32                          rsv_0098[2];      // 0098..009C
    REG_CDPHY_RX_ANA_AD_HS_0        CDPHY_RX_ANA_AD_HS_0; // 00A0
    REG_CDPHY_RX_ANA_AD_HS_1        CDPHY_RX_ANA_AD_HS_1; // 00A4
    REG_CDPHY_RX_ANA_AD_HS_2        CDPHY_RX_ANA_AD_HS_2; // 00A8
    UINT32                          rsv_00AC;         // 00AC
    REG_CDPHY_RX_ANA_FORCE_MODE_6   CDPHY_RX_ANA_FORCE_MODE_6; // 00B0
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_6 CDPHY_RX_ANA_FORCE_MODE_EN_6; // 00B4
    UINT32                          rsv_00B8[2];      // 00B8..00BC
    REG_CDPHY_RX_ANA_FORCE_MODE_7   CDPHY_RX_ANA_FORCE_MODE_7; // 00C0
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_7 CDPHY_RX_ANA_FORCE_MODE_EN_7; // 00C4
    UINT32                          rsv_00C8[2];      // 00C8..00CC
    REG_CDPHY_RX_ANA_FORCE_MODE_8   CDPHY_RX_ANA_FORCE_MODE_8; // 00D0
    REG_CDPHY_RX_ANA_FORCE_MODE_EN_8 CDPHY_RX_ANA_FORCE_MODE_EN_8; // 00D4
    UINT32                          rsv_00D8[6];      // 00D8..00EC
    REG_CDPHY_RX_ANA_SETTING_0      CDPHY_RX_ANA_SETTING_0; // 00F0
    REG_CDPHY_RX_ANA_SETTING_1      CDPHY_RX_ANA_SETTING_1; // 00F4
    REG_CDPHY_RX_ANA_DBG_OUT        CDPHY_RX_ANA_DBG_OUT; // 00F8
}mipi_rx_ana_cdphy_csi0a_reg_REGS, *Pmipi_rx_ana_cdphy_csi0a_reg_REGS;

#endif // __mipi_rx_ana_cdphy_csi0a_reg_REGS_H__
