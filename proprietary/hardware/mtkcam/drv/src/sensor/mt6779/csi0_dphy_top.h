#ifndef __csi0_dphy_top_reg_REGS_H__
#define __csi0_dphy_top_reg_REGS_H__

typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_lc0_en            : 1;
        FIELD dphy_rx_lc1_en            : 1;
        FIELD rsv_2                     : 6;
        FIELD dphy_rx_ld0_en            : 1;
        FIELD dphy_rx_ld1_en            : 1;
        FIELD dphy_rx_ld2_en            : 1;
        FIELD dphy_rx_ld3_en            : 1;
        FIELD rsv_12                    : 19;
        FIELD dphy_rx_sw_rst            : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_LANE_EN, *PREG_DPHY_RX_LANE_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_lc0_sel        : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_dphy_rx_lc1_sel        : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_dphy_rx_ld0_sel        : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_dphy_rx_ld1_sel        : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_dphy_rx_ld2_sel        : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_dphy_rx_ld3_sel        : 3;
        FIELD rsv_23                    : 8;
        FIELD dphy_rx_ck_data_mux_en    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_LANE_SELECT, *PREG_DPHY_RX_LANE_SELECT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_lc0_hsrx_en_sw : 1;
        FIELD rg_dphy_rx_lc1_hsrx_en_sw : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_cdphy_rx_ld0_trio0_hsrx_en_sw : 1;
        FIELD rg_cdphy_rx_ld1_trio1_hsrx_en_sw : 1;
        FIELD rg_cdphy_rx_ld2_trio2_hsrx_en_sw : 1;
        FIELD rg_cdphy_rx_ld2_trio3_hsrx_en_sw : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_HS_RX_EN_SW, *PREG_DPHY_RX_HS_RX_EN_SW;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_lc0_hs_prepare_parameter : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_dphy_rx_lc0_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_dphy_rx_lc0_hs_prepare_en : 1;
        FIELD rsv_29                    : 1;
        FIELD rg_dphy_rx_lc0_hs_option  : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_CLOCK_LANE0_HS_PARAMETER, *PREG_DPHY_RX_CLOCK_LANE0_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_lc1_hs_prepare_parameter : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_dphy_rx_lc1_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_dphy_rx_lc1_hs_prepare_en : 1;
        FIELD rsv_29                    : 1;
        FIELD rg_dphy_rx_lc1_hs_option  : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_CLOCK_LANE1_HS_PARAMETER, *PREG_DPHY_RX_CLOCK_LANE1_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cdphy_rx_ld0_trio0_hs_prepare_parameter : 8;
        FIELD rg_dphy_rx_ld0_hs_trail_parameter : 8;
        FIELD rg_cdphy_rx_ld0_trio0_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_cdphy_rx_ld0_trio0_hs_prepare_en : 1;
        FIELD rg_dphy_rx_ld0_hs_trail_en : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE0_HS_PARAMETER, *PREG_DPHY_RX_DATA_LANE0_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cdphy_rx_ld1_trio1_hs_prepare_parameter : 8;
        FIELD rg_dphy_rx_ld1_hs_trail_parameter : 8;
        FIELD rg_cdphy_rx_ld1_trio1_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_cdphy_rx_ld1_trio1_hs_prepare_en : 1;
        FIELD rg_dphy_rx_ld1_hs_trail_en : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE1_HS_PARAMETER, *PREG_DPHY_RX_DATA_LANE1_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cdphy_rx_ld2_trio2_hs_prepare_parameter : 8;
        FIELD rg_dphy_rx_ld2_hs_trail_parameter : 8;
        FIELD rg_cdphy_rx_ld2_trio2_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_cdphy_rx_ld2_trio2_hs_prepare_en : 1;
        FIELD rg_dphy_rx_ld2_hs_trail_en : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE2_HS_PARAMETER, *PREG_DPHY_RX_DATA_LANE2_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cdphy_rx_ld3_trio3_hs_prepare_parameter : 8;
        FIELD rg_dphy_rx_ld3_hs_trail_parameter : 8;
        FIELD rg_cdphy_rx_ld3_trio3_hs_settle_parameter : 8;
        FIELD rsv_24                    : 4;
        FIELD rg_cdphy_rx_ld3_trio3_hs_prepare_en : 1;
        FIELD rg_dphy_rx_ld3_hs_trail_en : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE3_HS_PARAMETER, *PREG_DPHY_RX_DATA_LANE3_HS_PARAMETER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_cl0_fsm        : 6;
        FIELD rsv_6                     : 2;
        FIELD ro_dphy_rx_cl1_fsm        : 6;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_CLOCK_LANE_FSM, *PREG_DPHY_RX_CLOCK_LANE_FSM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_dl0_fsm        : 6;
        FIELD rsv_6                     : 2;
        FIELD ro_dphy_rx_dl1_fsm        : 6;
        FIELD rsv_14                    : 2;
        FIELD ro_dphy_rx_dl2_fsm        : 6;
        FIELD rsv_22                    : 2;
        FIELD ro_dphy_rx_dl3_fsm        : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE_FSM, *PREG_DPHY_RX_DATA_LANE_FSM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_ld_sync_seq_pat_norm : 16;
        FIELD rg_dphy_rx_ld_sync_seq_mask_norm : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE_SYNC_DETECT_NORMAL, *PREG_DPHY_RX_DATA_LANE_SYNC_DETECT_NORMAL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_ld_sync_seq_pat_deskew : 16;
        FIELD rg_dphy_rx_ld_sync_seq_mask_deskew : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DATA_LANE_SYNC_DETECT_DESKEW, *PREG_DPHY_RX_DATA_LANE_SYNC_DETECT_DESKEW;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_initial_setup : 4;
        FIELD rg_dphy_rx_deskew_offset  : 4;
        FIELD rg_dphy_rx_deskew_en      : 1;
        FIELD rg_dphy_rx_deskew_code_unit_sel : 1;
        FIELD rg_dphy_rx_deskew_hw_delay_apply_opt : 1;
        FIELD rsv_11                    : 1;
        FIELD rg_dphy_rx_deskew_acc_mode : 4;
        FIELD rg_dphy_rx_deskew_apply_mode : 4;
        FIELD rsv_20                    : 4;
        FIELD rg_dphy_rx_deskew_length  : 6;
        FIELD rsv_30                    : 1;
        FIELD dphy_rx_deskew_sw_rst     : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_CTRL, *PREG_DPHY_RX_DESKEW_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_setup_cnt : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_dphy_rx_deskew_hold_cnt : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_dphy_rx_deskew_detect_cnt : 7;
        FIELD rsv_23                    : 1;
        FIELD rg_dphy_rx_deskew_cmplength : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_TIMING_CTRL, *PREG_DPHY_RX_DESKEW_TIMING_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_lane0_swap : 2;
        FIELD rg_dphy_rx_deskew_lane1_swap : 2;
        FIELD rg_dphy_rx_deskew_lane2_swap : 2;
        FIELD rg_dphy_rx_deskew_lane3_swap : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE_SWAP, *PREG_DPHY_RX_DESKEW_LANE_SWAP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_deskew_l0_delay_en : 1;
        FIELD rg_dphy_rx_deskew_l0_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_dphy_rx_deskew_l0_delay_code : 6;
        FIELD rsv_14                    : 2;
        FIELD rg_dphy_rx_deskew_l0_periodic : 1;
        FIELD rg_periodic_deskew_1dir_only : 1;
        FIELD rsv_18                    : 2;
        FIELD rg_dphy_rx_deskew_l0_periodic_delay_code_cnt : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE0_CTRL, *PREG_DPHY_RX_DESKEW_LANE0_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_deskew_l1_delay_en : 1;
        FIELD rg_dphy_rx_deskew_l1_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_dphy_rx_deskew_l1_delay_code : 6;
        FIELD rsv_14                    : 2;
        FIELD rg_dphy_rx_deskew_l1_periodic : 1;
        FIELD rsv_17                    : 3;
        FIELD rg_dphy_rx_deskew_l1_periodic_delay_code_cnt : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE1_CTRL, *PREG_DPHY_RX_DESKEW_LANE1_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_deskew_l2_delay_en : 1;
        FIELD rg_dphy_rx_deskew_l2_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_dphy_rx_deskew_l2_delay_code : 6;
        FIELD rsv_14                    : 2;
        FIELD rg_dphy_rx_deskew_l2_periodic : 1;
        FIELD rsv_17                    : 3;
        FIELD rg_dphy_rx_deskew_l2_periodic_delay_code_cnt : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE2_CTRL, *PREG_DPHY_RX_DESKEW_LANE2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_deskew_l3_delay_en : 1;
        FIELD rg_dphy_rx_deskew_l3_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_dphy_rx_deskew_l3_delay_code : 6;
        FIELD rsv_14                    : 2;
        FIELD rg_dphy_rx_deskew_l3_periodic : 1;
        FIELD rsv_17                    : 3;
        FIELD rg_dphy_rx_deskew_l3_periodic_delay_code_cnt : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE3_CTRL, *PREG_DPHY_RX_DESKEW_LANE3_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_da_dphy_l0_delay_en    : 1;
        FIELD ro_da_dphy_l0_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD ro_da_dphy_l0_delay_code  : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE0_DA_READ, *PREG_DPHY_RX_DESKEW_LANE0_DA_READ;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_da_dphy_l1_delay_en    : 1;
        FIELD ro_da_dphy_l1_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD ro_da_dphy_l0_delay_code  : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE1_DA_READ, *PREG_DPHY_RX_DESKEW_LANE1_DA_READ;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_da_dphy_l2_delay_en    : 1;
        FIELD ro_da_dphy_l2_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD ro_da_dphy_l0_delay_code  : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE2_DA_READ, *PREG_DPHY_RX_DESKEW_LANE2_DA_READ;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_da_dphy_l3_delay_en    : 1;
        FIELD ro_da_dphy_l3_delay_apply : 1;
        FIELD rsv_2                     : 6;
        FIELD ro_da_dphy_l0_delay_code  : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_LANE3_DA_READ, *PREG_DPHY_RX_DESKEW_LANE3_DA_READ;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_irq_en  : 16;
        FIELD rsv_16                    : 15;
        FIELD rg_dphy_rx_deskew_irq_w1c_en : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_IRQ_EN, *PREG_DPHY_RX_DESKEW_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_deskew_irq_status : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_IRQ_STATUS, *PREG_DPHY_RX_DESKEW_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_err_sot_sync_hs_l0_irq_en : 1;
        FIELD rg_dphy_rx_err_sot_sync_hs_l1_irq_en : 1;
        FIELD rg_dphy_rx_err_sot_sync_hs_l2_irq_en : 1;
        FIELD rg_dphy_rx_err_sot_sync_hs_l3_irq_en : 1;
        FIELD rsv_4                     : 27;
        FIELD rg_dphy_rx_irq_w1c_en     : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_IRQ_EN, *PREG_DPHY_RX_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_err_sot_sync_hs_l0_irq : 1;
        FIELD dphy_rx_err_sot_sync_hs_l1_irq : 1;
        FIELD dphy_rx_err_sot_sync_hs_l2_irq : 1;
        FIELD dphy_rx_err_sot_sync_hs_l3_irq : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_IRQ_STATUS, *PREG_DPHY_RX_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_ld0_sync_seq_pos_norm : 4;
        FIELD ro_dphy_rx_ld1_sync_seq_pos_norm : 4;
        FIELD ro_dphy_rx_ld2_sync_seq_pos_norm : 4;
        FIELD ro_dphy_rx_ld3_sync_seq_pos_norm : 4;
        FIELD ro_dphy_rx_ld0_sync_seq_pos_deskew : 4;
        FIELD ro_dphy_rx_ld1_sync_seq_pos_deskew : 4;
        FIELD ro_dphy_rx_ld2_sync_seq_pos_deskew : 4;
        FIELD ro_dphy_rx_ld3_sync_seq_pos_deskew : 4;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATUS_0, *PREG_DPHY_RX_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_dbg_mux : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_DBG_MUX, *PREG_DPHY_RX_DESKEW_DBG_MUX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_deskew_dbg_out : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_DESKEW_OUT, *PREG_DPHY_RX_DESKEW_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_spare0         : 6;
        FIELD rg_post_cnt               : 6;
        FIELD rg_dphy_rx_spare0_2       : 20;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_SPARE0, *PREG_DPHY_RX_SPARE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_spare1         : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_SPARE1, *PREG_DPHY_RX_SPARE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_bist_en_lane0     : 1;
        FIELD dphy_rx_bist_en_lane1     : 1;
        FIELD dphy_rx_bist_en_lane2     : 1;
        FIELD dphy_rx_bist_en_lane3     : 1;
        FIELD rsv_4                     : 4;
        FIELD rg_phy2mac_cdphy_gen_en   : 1;
        FIELD rg_phy2mac_cdphy_gen_mode : 1;
        FIELD rg_phy2mac_cdphy_gen_sel  : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_ENABLE, *PREG_DPHY_RX_BIST_ENABLE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_ok_lane0  : 1;
        FIELD ro_dphy_rx_bist_ok_lane1  : 1;
        FIELD ro_dphy_rx_bist_ok_lane2  : 1;
        FIELD ro_dphy_rx_bist_ok_lane3  : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_dphy_rx_bist_hs_mode_ok_lane0 : 1;
        FIELD ro_dphy_rx_bist_hs_mode_ok_lane1 : 1;
        FIELD ro_dphy_rx_bist_hs_mode_ok_lane2 : 1;
        FIELD ro_dphy_rx_bist_hs_mode_ok_lane3 : 1;
        FIELD rsv_12                    : 4;
        FIELD ro_dphy_rx_bist_lp_mode_ok_lane0 : 1;
        FIELD ro_dphy_rx_bist_lp_mode_ok_lane1 : 1;
        FIELD ro_dphy_rx_bist_lp_mode_ok_lane2 : 1;
        FIELD ro_dphy_rx_bist_lp_mode_ok_lane3 : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS, *PREG_DPHY_RX_BIST_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_mode_lane0 : 2;
        FIELD rsv_2                     : 2;
        FIELD rg_dphy_rx_bist_hs_fix_pat_lane0 : 1;
        FIELD rg_dphy_rx_bist_hs_sync_det_unblock_lane0 : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_dphy_rx_bist_hs_ran_pat_mask_n_lane0 : 8;
        FIELD rg_dphy_rx_bist_hs_length_lane0 : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_0_LANE0, *PREG_DPHY_RX_BIST_CONFIG_0_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_lp_mode_seq0_lane0 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq1_lane0 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq2_lane0 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq3_lane0 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq4_lane0 : 2;
        FIELD rsv_10                    : 6;
        FIELD rg_dphy_rx_bist_lp_mode_length_lane0 : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_1_LANE0, *PREG_DPHY_RX_BIST_CONFIG_1_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_start_lane0 : 1;
        FIELD ro_dphy_rx_bist_hs_ok_lane0 : 1;
        FIELD ro_dphy_rx_bist_data_ok_lane0 : 1;
        FIELD ro_dphy_rx_bist_fsm_ok_lane0 : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_dphy_rx_bist_lp_fail_seq_number_lane0 : 3;
        FIELD rsv_11                    : 1;
        FIELD ro_dphy_rx_bist_lp_fail_seq_lane0 : 2;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_LANE0, *PREG_DPHY_RX_BIST_STATUS_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_mode_lane1 : 2;
        FIELD rsv_2                     : 2;
        FIELD rg_dphy_rx_bist_hs_fix_pat_lane1 : 1;
        FIELD rg_dphy_rx_bist_hs_sync_det_unblock_lane1 : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_dphy_rx_bist_hs_ran_pat_mask_n_lane1 : 8;
        FIELD rg_dphy_rx_bist_hs_length_lane1 : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_0_LANE1, *PREG_DPHY_RX_BIST_CONFIG_0_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_lp_mode_seq0_lane1 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq1_lane1 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq2_lane1 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq3_lane1 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq4_lane1 : 2;
        FIELD rsv_10                    : 6;
        FIELD rg_dphy_rx_bist_lp_mode_length_lane1 : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_1_LANE1, *PREG_DPHY_RX_BIST_CONFIG_1_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_start_lane1 : 1;
        FIELD ro_dphy_rx_bist_hs_ok_lane1 : 1;
        FIELD ro_dphy_rx_bist_data_ok_lane1 : 1;
        FIELD ro_dphy_rx_bist_fsm_ok_lane1 : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_dphy_rx_bist_lp_fail_seq_number_lane1 : 3;
        FIELD rsv_11                    : 1;
        FIELD ro_dphy_rx_bist_lp_fail_seq_lane1 : 2;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_LANE1, *PREG_DPHY_RX_BIST_STATUS_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_mode_lane2 : 2;
        FIELD rsv_2                     : 2;
        FIELD rg_dphy_rx_bist_hs_fix_pat_lane2 : 1;
        FIELD rg_dphy_rx_bist_hs_sync_det_unblock_lane2 : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_dphy_rx_bist_hs_ran_pat_mask_n_lane2 : 8;
        FIELD rg_dphy_rx_bist_hs_length_lane2 : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_0_LANE2, *PREG_DPHY_RX_BIST_CONFIG_0_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_lp_mode_seq0_lane2 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq1_lane2 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq2_lane2 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq3_lane2 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq4_lane2 : 2;
        FIELD rsv_10                    : 6;
        FIELD rg_dphy_rx_bist_lp_mode_length_lane2 : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_1_LANE2, *PREG_DPHY_RX_BIST_CONFIG_1_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_start_lane2 : 1;
        FIELD ro_dphy_rx_bist_hs_ok_lane2 : 1;
        FIELD ro_dphy_rx_bist_data_ok_lane2 : 1;
        FIELD ro_dphy_rx_bist_fsm_ok_lane2 : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_dphy_rx_bist_lp_fail_seq_number_lane2 : 3;
        FIELD rsv_11                    : 1;
        FIELD ro_dphy_rx_bist_lp_fail_seq_lane2 : 2;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_LANE2, *PREG_DPHY_RX_BIST_STATUS_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_mode_lane3 : 2;
        FIELD rsv_2                     : 2;
        FIELD rg_dphy_rx_bist_hs_fix_pat_lane3 : 1;
        FIELD rg_dphy_rx_bist_hs_sync_det_unblock_lane3 : 1;
        FIELD rsv_6                     : 2;
        FIELD rg_dphy_rx_bist_hs_ran_pat_mask_n_lane3 : 8;
        FIELD rg_dphy_rx_bist_hs_length_lane3 : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_0_LANE3, *PREG_DPHY_RX_BIST_CONFIG_0_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_bist_lp_mode_seq0_lane3 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq1_lane3 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq2_lane3 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq3_lane3 : 2;
        FIELD rg_dphy_rx_bist_lp_mode_seq4_lane3 : 2;
        FIELD rsv_10                    : 6;
        FIELD rg_dphy_rx_bist_lp_mode_length_lane3 : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_CONFIG_1_LANE3, *PREG_DPHY_RX_BIST_CONFIG_1_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_start_lane3 : 1;
        FIELD ro_dphy_rx_bist_hs_ok_lane3 : 1;
        FIELD ro_dphy_rx_bist_data_ok_lane3 : 1;
        FIELD ro_dphy_rx_bist_fsm_ok_lane3 : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_dphy_rx_bist_lp_fail_seq_number_lane3 : 3;
        FIELD rsv_11                    : 1;
        FIELD ro_dphy_rx_bist_lp_fail_seq_lane3 : 2;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_LANE3, *PREG_DPHY_RX_BIST_STATUS_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_cnt_lane0 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE0, *PREG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_err_cnt_lane0 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE0, *PREG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_cnt_lane1 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE1, *PREG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_err_cnt_lane1 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE1, *PREG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_cnt_lane2 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE2, *PREG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_err_cnt_lane2 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE2, *PREG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_cnt_lane3 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE3, *PREG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_bist_word_err_cnt_lane3 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE3, *PREG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_cal_en            : 1;
        FIELD rg_dphy_preamble_en       : 1;
        FIELD rg_hsidle_en              : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_DPHY_DPHYV21_CTRL, *PREG_DPHY_DPHYV21_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD csr_rlrn_prbs_seed        : 18;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_DPHY_PRBS_SEED, *PREG_DPHY_PRBS_SEED;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_ck_det_byte_ck_div     : 8;
        FIELD rg_ck_det_ck_cnt          : 8;
        FIELD rg_hsrc_en_mask_cnt       : 6;
        FIELD rsv_22                    : 2;
        FIELD rg_div_ck_det_sel         : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_DPHY_HS_IDLE_CNT, *PREG_DPHY_HS_IDLE_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_lp00_2_sync_code_clr : 1;
        FIELD rg_dphy_error_cmp_clr_lane0 : 1;
        FIELD rg_dphy_error_cmp_clr_lane1 : 1;
        FIELD rg_dphy_error_cmp_clr_lane2 : 1;
        FIELD rg_dphy_error_cmp_clr_lane3 : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_DPHY_DFT_CLR, *PREG_DPHY_DFT_CLR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_mode_lane0     : 5;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_CTRL_LANE0, *PREG_DPHY_ALT_CAL_CTRL_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt0_lane0 : 16;
        FIELD ro_alt_cal_err_cnt_lane0  : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT0_LANE0, *PREG_DPHY_ALT_CAL_WORD_CNT0_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt1_lane0 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT1_LANE0, *PREG_DPHY_ALT_CAL_WORD_CNT1_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_alt_cal_err_cnt_lane0  : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_ERR_CNT_LANE0, *PREG_DPHY_ALT_CAL_ERR_CNT_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_mode_lane1     : 5;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_CTRL_LANE1, *PREG_DPHY_ALT_CAL_CTRL_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt0_lane1 : 16;
        FIELD ro_alt_cal_err_cnt_lane1  : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT0_LANE1, *PREG_DPHY_ALT_CAL_WORD_CNT0_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt1_lane1 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT1_LANE1, *PREG_DPHY_ALT_CAL_WORD_CNT1_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_alt_cal_err_cnt_lane1  : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_ERR_CNT_LANE1, *PREG_DPHY_ALT_CAL_ERR_CNT_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_mode_lane2     : 5;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_CTRL_LANE2, *PREG_DPHY_ALT_CAL_CTRL_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt0_lane2 : 16;
        FIELD ro_alt_cal_err_cnt_lane2  : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT0_LANE2, *PREG_DPHY_ALT_CAL_WORD_CNT0_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt1_lane2 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT1_LANE2, *PREG_DPHY_ALT_CAL_WORD_CNT1_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_alt_cal_err_cnt_lane2  : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_ERR_CNT_LANE2, *PREG_DPHY_ALT_CAL_ERR_CNT_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_mode_lane3     : 5;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_CTRL_LANE3, *PREG_DPHY_ALT_CAL_CTRL_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt0_lane3 : 16;
        FIELD ro_alt_cal_err_cnt_lane3  : 16;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT0_LANE3, *PREG_DPHY_ALT_CAL_WORD_CNT0_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_alt_cal_word_cnt1_lane3 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_WORD_CNT1_LANE3, *PREG_DPHY_ALT_CAL_WORD_CNT1_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_alt_cal_err_cnt_lane3  : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_ERR_CNT_LANE3, *PREG_DPHY_ALT_CAL_ERR_CNT_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_prbs_burst_cnt_lane0 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_CNT_LANE0, *PREG_DPHY_ALT_CAL_BURST_CNT_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_prbs_burst_err_cnt_lane0 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE0, *PREG_DPHY_ALT_CAL_BURST_ERRCNT_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_prbs_burst_cnt_lane1 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_CNT_LANE1, *PREG_DPHY_ALT_CAL_BURST_CNT_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_prbs_burst_err_cnt_lane1 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE1, *PREG_DPHY_ALT_CAL_BURST_ERRCNT_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_prbs_burst_cnt_lane2 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_CNT_LANE2, *PREG_DPHY_ALT_CAL_BURST_CNT_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_prbs_burst_err_cnt_lane2 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE2, *PREG_DPHY_ALT_CAL_BURST_ERRCNT_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_prbs_burst_cnt_lane3 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_CNT_LANE3, *PREG_DPHY_ALT_CAL_BURST_CNT_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_prbs_burst_err_cnt_lane3 : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE3, *PREG_DPHY_ALT_CAL_BURST_ERRCNT_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dphy_rx_state_chk_en_lane0 : 1;
        FIELD dphy_rx_state_chk_en_lane1 : 1;
        FIELD dphy_rx_state_chk_en_lane2 : 1;
        FIELD dphy_rx_state_chk_en_lane3 : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_EN, *PREG_DPHY_RX_STATE_CHK_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_dphy_rx_state_chk_rx_timer_cnt : 32;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_TIMER, *PREG_DPHY_RX_STATE_CHK_TIMER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_state_chk_err_lane0 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_dphy_rx_state_chk_err_cnt_lane0 : 3;
        FIELD rsv_7                     : 9;
        FIELD dphy_rx_state_chk_fail_seq0_lane0 : 3;
        FIELD rsv_19                    : 1;
        FIELD dphy_rx_state_chk_fail_seq1_lane0 : 3;
        FIELD rsv_23                    : 1;
        FIELD dphy_rx_state_chk_fail_seq2_lane0 : 3;
        FIELD rsv_27                    : 1;
        FIELD dphy_rx_state_chk_fail_seq3_lane0 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_STATUS_LANE0, *PREG_DPHY_RX_STATE_CHK_STATUS_LANE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_state_chk_err_lane1 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_dphy_rx_state_chk_err_cnt_lane1 : 3;
        FIELD rsv_7                     : 9;
        FIELD dphy_rx_state_chk_fail_seq0_lane1 : 3;
        FIELD rsv_19                    : 1;
        FIELD dphy_rx_state_chk_fail_seq1_lane1 : 3;
        FIELD rsv_23                    : 1;
        FIELD dphy_rx_state_chk_fail_seq2_lane1 : 3;
        FIELD rsv_27                    : 1;
        FIELD dphy_rx_state_chk_fail_seq3_lane1 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_STATUS_LANE1, *PREG_DPHY_RX_STATE_CHK_STATUS_LANE1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_state_chk_err_lane2 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_dphy_rx_state_chk_err_cnt_lane2 : 3;
        FIELD rsv_7                     : 9;
        FIELD dphy_rx_state_chk_fail_seq0_lane2 : 3;
        FIELD rsv_19                    : 1;
        FIELD dphy_rx_state_chk_fail_seq1_lane2 : 3;
        FIELD rsv_23                    : 1;
        FIELD dphy_rx_state_chk_fail_seq2_lane2 : 3;
        FIELD rsv_27                    : 1;
        FIELD dphy_rx_state_chk_fail_seq3_lane2 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_STATUS_LANE2, *PREG_DPHY_RX_STATE_CHK_STATUS_LANE2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_dphy_rx_state_chk_err_lane3 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_dphy_rx_state_chk_err_cnt_lane3 : 3;
        FIELD rsv_7                     : 9;
        FIELD dphy_rx_state_chk_fail_seq0_lane3 : 3;
        FIELD rsv_19                    : 1;
        FIELD dphy_rx_state_chk_fail_seq1_lane3 : 3;
        FIELD rsv_23                    : 1;
        FIELD dphy_rx_state_chk_fail_seq2_lane3 : 3;
        FIELD rsv_27                    : 1;
        FIELD dphy_rx_state_chk_fail_seq3_lane3 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_STATE_CHK_STATUS_LANE3, *PREG_DPHY_RX_STATE_CHK_STATUS_LANE3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi_cdphy_dbg_port_sel : 32;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_DEBUG_PORT_SELECT, *PREG_CDPHY_DEBUG_PORT_SELECT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi_cdphy_dbg_mod_sel  : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_DEBUG_MODE_SELECT, *PREG_CDPHY_DEBUG_MODE_SELECT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi_cdphy_dbg_out_read : 32;
    } Bits;
    UINT32 Raw;
} REG_CDPHY_DEBUG_OUT_READ, *PREG_CDPHY_DEBUG_OUT_READ;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_tinit_cnt         : 20;
        FIELD rsv_20                    : 11;
        FIELD rg_csi2_tinit_cnt_en      : 1;
    } Bits;
    UINT32 Raw;
} REG_DPHY_RX_INIT, *PREG_DPHY_RX_INIT;

// ----------------- csi0_dphy_top_reg Register Definition -------------------
//0x11c8 2000
typedef volatile struct
{
    REG_DPHY_RX_LANE_EN             DPHY_RX_LANE_EN;  // 2000
    REG_DPHY_RX_LANE_SELECT         DPHY_RX_LANE_SELECT; // 2004
    REG_DPHY_RX_HS_RX_EN_SW         DPHY_RX_HS_RX_EN_SW; // 2008
    UINT32                          rsv_200C;         // 200C
    REG_DPHY_RX_CLOCK_LANE0_HS_PARAMETER DPHY_RX_CLOCK_LANE0_HS_PARAMETER; // 2010
    REG_DPHY_RX_CLOCK_LANE1_HS_PARAMETER DPHY_RX_CLOCK_LANE1_HS_PARAMETER; // 2014
    UINT32                          rsv_2018[2];      // 2018..201C
    REG_DPHY_RX_DATA_LANE0_HS_PARAMETER DPHY_RX_DATA_LANE0_HS_PARAMETER; // 2020
    REG_DPHY_RX_DATA_LANE1_HS_PARAMETER DPHY_RX_DATA_LANE1_HS_PARAMETER; // 2024
    REG_DPHY_RX_DATA_LANE2_HS_PARAMETER DPHY_RX_DATA_LANE2_HS_PARAMETER; // 2028
    REG_DPHY_RX_DATA_LANE3_HS_PARAMETER DPHY_RX_DATA_LANE3_HS_PARAMETER; // 202C
    REG_DPHY_RX_CLOCK_LANE_FSM      DPHY_RX_CLOCK_LANE_FSM; // 2030
    REG_DPHY_RX_DATA_LANE_FSM       DPHY_RX_DATA_LANE_FSM; // 2034
    UINT32                          rsv_2038[2];      // 2038..203C
    REG_DPHY_RX_DATA_LANE_SYNC_DETECT_NORMAL DPHY_RX_DATA_LANE_SYNC_DETECT_NORMAL; // 2040
    REG_DPHY_RX_DATA_LANE_SYNC_DETECT_DESKEW DPHY_RX_DATA_LANE_SYNC_DETECT_DESKEW; // 2044
    UINT32                          rsv_2048[2];      // 2048..204C
    REG_DPHY_RX_DESKEW_CTRL         DPHY_RX_DESKEW_CTRL; // 2050
    REG_DPHY_RX_DESKEW_TIMING_CTRL  DPHY_RX_DESKEW_TIMING_CTRL; // 2054
    REG_DPHY_RX_DESKEW_LANE_SWAP    DPHY_RX_DESKEW_LANE_SWAP; // 2058
    UINT32                          rsv_205C;         // 205C
    REG_DPHY_RX_DESKEW_LANE0_CTRL   DPHY_RX_DESKEW_LANE0_CTRL; // 2060
    REG_DPHY_RX_DESKEW_LANE1_CTRL   DPHY_RX_DESKEW_LANE1_CTRL; // 2064
    REG_DPHY_RX_DESKEW_LANE2_CTRL   DPHY_RX_DESKEW_LANE2_CTRL; // 2068
    REG_DPHY_RX_DESKEW_LANE3_CTRL   DPHY_RX_DESKEW_LANE3_CTRL; // 206C
    REG_DPHY_RX_DESKEW_LANE0_DA_READ DPHY_RX_DESKEW_LANE0_DA_READ; // 2070
    REG_DPHY_RX_DESKEW_LANE1_DA_READ DPHY_RX_DESKEW_LANE1_DA_READ; // 2074
    REG_DPHY_RX_DESKEW_LANE2_DA_READ DPHY_RX_DESKEW_LANE2_DA_READ; // 2078
    REG_DPHY_RX_DESKEW_LANE3_DA_READ DPHY_RX_DESKEW_LANE3_DA_READ; // 207C
    REG_DPHY_RX_DESKEW_IRQ_EN       DPHY_RX_DESKEW_IRQ_EN; // 2080
    REG_DPHY_RX_DESKEW_IRQ_STATUS   DPHY_RX_DESKEW_IRQ_STATUS; // 2084
    REG_DPHY_RX_IRQ_EN              DPHY_RX_IRQ_EN;   // 2088
    REG_DPHY_RX_IRQ_STATUS          DPHY_RX_IRQ_STATUS; // 208C
    UINT32                          rsv_2090[4];      // 2090..209C
    REG_DPHY_RX_STATUS_0            DPHY_RX_STATUS_0; // 20A0
    UINT32                          rsv_20A4[15];     // 20A4..20DC
    REG_DPHY_RX_DESKEW_DBG_MUX      DPHY_RX_DESKEW_DBG_MUX; // 20E0
    REG_DPHY_RX_DESKEW_OUT          DPHY_RX_DESKEW_OUT; // 20E4
    UINT32                          rsv_20E8[2];      // 20E8..20EC
    REG_DPHY_RX_SPARE0              DPHY_RX_SPARE0;   // 20F0
    REG_DPHY_RX_SPARE1              DPHY_RX_SPARE1;   // 20F4
    UINT32                          rsv_20F8[2];      // 20F8..20FC
    REG_DPHY_RX_BIST_ENABLE         DPHY_RX_BIST_ENABLE; // 2100
    REG_DPHY_RX_BIST_STATUS         DPHY_RX_BIST_STATUS; // 2104
    UINT32                          rsv_2108[2];      // 2108..210C
    REG_DPHY_RX_BIST_CONFIG_0_LANE0 DPHY_RX_BIST_CONFIG_0_LANE0; // 2110
    REG_DPHY_RX_BIST_CONFIG_1_LANE0 DPHY_RX_BIST_CONFIG_1_LANE0; // 2114
    REG_DPHY_RX_BIST_STATUS_LANE0   DPHY_RX_BIST_STATUS_LANE0; // 2118
    UINT32                          rsv_211C;         // 211C
    REG_DPHY_RX_BIST_CONFIG_0_LANE1 DPHY_RX_BIST_CONFIG_0_LANE1; // 2120
    REG_DPHY_RX_BIST_CONFIG_1_LANE1 DPHY_RX_BIST_CONFIG_1_LANE1; // 2124
    REG_DPHY_RX_BIST_STATUS_LANE1   DPHY_RX_BIST_STATUS_LANE1; // 2128
    UINT32                          rsv_212C;         // 212C
    REG_DPHY_RX_BIST_CONFIG_0_LANE2 DPHY_RX_BIST_CONFIG_0_LANE2; // 2130
    REG_DPHY_RX_BIST_CONFIG_1_LANE2 DPHY_RX_BIST_CONFIG_1_LANE2; // 2134
    REG_DPHY_RX_BIST_STATUS_LANE2   DPHY_RX_BIST_STATUS_LANE2; // 2138
    UINT32                          rsv_213C;         // 213C
    REG_DPHY_RX_BIST_CONFIG_0_LANE3 DPHY_RX_BIST_CONFIG_0_LANE3; // 2140
    REG_DPHY_RX_BIST_CONFIG_1_LANE3 DPHY_RX_BIST_CONFIG_1_LANE3; // 2144
    REG_DPHY_RX_BIST_STATUS_LANE3   DPHY_RX_BIST_STATUS_LANE3; // 2148
    UINT32                          rsv_214C;         // 214C
    REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE0 DPHY_RX_BIST_STATUS_WORD_CNT_LANE0; // 2150
    REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE0 DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE0; // 2154
    REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE1 DPHY_RX_BIST_STATUS_WORD_CNT_LANE1; // 2158
    REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE1 DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE1; // 215C
    REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE2 DPHY_RX_BIST_STATUS_WORD_CNT_LANE2; // 2160
    REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE2 DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE2; // 2164
    REG_DPHY_RX_BIST_STATUS_WORD_CNT_LANE3 DPHY_RX_BIST_STATUS_WORD_CNT_LANE3; // 2168
    REG_DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE3 DPHY_RX_BIST_STATUS_WORD_ERR_CNT_LANE3; // 216C
    UINT32                          rsv_2170[4];      // 2170..217C
    REG_DPHY_DPHYV21_CTRL           DPHY_DPHYV21_CTRL; // 2180
    REG_DPHY_PRBS_SEED              DPHY_PRBS_SEED;   // 2184
    REG_DPHY_HS_IDLE_CNT            DPHY_HS_IDLE_CNT; // 2188
    REG_DPHY_DFT_CLR                DPHY_DFT_CLR;     // 218C
    REG_DPHY_ALT_CAL_CTRL_LANE0     DPHY_ALT_CAL_CTRL_LANE0; // 2190
    REG_DPHY_ALT_CAL_WORD_CNT0_LANE0 DPHY_ALT_CAL_WORD_CNT0_LANE0; // 2194
    REG_DPHY_ALT_CAL_WORD_CNT1_LANE0 DPHY_ALT_CAL_WORD_CNT1_LANE0; // 2198
    REG_DPHY_ALT_CAL_ERR_CNT_LANE0  DPHY_ALT_CAL_ERR_CNT_LANE0; // 219C
    REG_DPHY_ALT_CAL_CTRL_LANE1     DPHY_ALT_CAL_CTRL_LANE1; // 21A0
    REG_DPHY_ALT_CAL_WORD_CNT0_LANE1 DPHY_ALT_CAL_WORD_CNT0_LANE1; // 21A4
    REG_DPHY_ALT_CAL_WORD_CNT1_LANE1 DPHY_ALT_CAL_WORD_CNT1_LANE1; // 21A8
    REG_DPHY_ALT_CAL_ERR_CNT_LANE1  DPHY_ALT_CAL_ERR_CNT_LANE1; // 21AC
    REG_DPHY_ALT_CAL_CTRL_LANE2     DPHY_ALT_CAL_CTRL_LANE2; // 21B0
    REG_DPHY_ALT_CAL_WORD_CNT0_LANE2 DPHY_ALT_CAL_WORD_CNT0_LANE2; // 21B4
    REG_DPHY_ALT_CAL_WORD_CNT1_LANE2 DPHY_ALT_CAL_WORD_CNT1_LANE2; // 21B8
    REG_DPHY_ALT_CAL_ERR_CNT_LANE2  DPHY_ALT_CAL_ERR_CNT_LANE2; // 21BC
    REG_DPHY_ALT_CAL_CTRL_LANE3     DPHY_ALT_CAL_CTRL_LANE3; // 21C0
    REG_DPHY_ALT_CAL_WORD_CNT0_LANE3 DPHY_ALT_CAL_WORD_CNT0_LANE3; // 21C4
    REG_DPHY_ALT_CAL_WORD_CNT1_LANE3 DPHY_ALT_CAL_WORD_CNT1_LANE3; // 21C8
    REG_DPHY_ALT_CAL_ERR_CNT_LANE3  DPHY_ALT_CAL_ERR_CNT_LANE3; // 21CC
    REG_DPHY_ALT_CAL_BURST_CNT_LANE0 DPHY_ALT_CAL_BURST_CNT_LANE0; // 21D0
    REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE0 DPHY_ALT_CAL_BURST_ERRCNT_LANE0; // 21D4
    REG_DPHY_ALT_CAL_BURST_CNT_LANE1 DPHY_ALT_CAL_BURST_CNT_LANE1; // 21D8
    REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE1 DPHY_ALT_CAL_BURST_ERRCNT_LANE1; // 21DC
    REG_DPHY_ALT_CAL_BURST_CNT_LANE2 DPHY_ALT_CAL_BURST_CNT_LANE2; // 21E0
    REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE2 DPHY_ALT_CAL_BURST_ERRCNT_LANE2; // 21E4
    REG_DPHY_ALT_CAL_BURST_CNT_LANE3 DPHY_ALT_CAL_BURST_CNT_LANE3; // 21E8
    REG_DPHY_ALT_CAL_BURST_ERRCNT_LANE3 DPHY_ALT_CAL_BURST_ERRCNT_LANE3; // 21EC
    UINT32                          rsv_21F0[4];      // 21F0..21FC
    REG_DPHY_RX_STATE_CHK_EN        DPHY_RX_STATE_CHK_EN; // 2200
    UINT32                          rsv_2204[24];     // 2204..2260
    REG_DPHY_RX_STATE_CHK_TIMER     DPHY_RX_STATE_CHK_TIMER; // 2264
    UINT32                          rsv_2268[2];      // 2268..226C
    REG_DPHY_RX_STATE_CHK_STATUS_LANE0 DPHY_RX_STATE_CHK_STATUS_LANE0; // 2270
    REG_DPHY_RX_STATE_CHK_STATUS_LANE1 DPHY_RX_STATE_CHK_STATUS_LANE1; // 2274
    REG_DPHY_RX_STATE_CHK_STATUS_LANE2 DPHY_RX_STATE_CHK_STATUS_LANE2; // 2278
    REG_DPHY_RX_STATE_CHK_STATUS_LANE3 DPHY_RX_STATE_CHK_STATUS_LANE3; // 227C
    REG_CDPHY_DEBUG_PORT_SELECT     CDPHY_DEBUG_PORT_SELECT; // 2280
    REG_CDPHY_DEBUG_MODE_SELECT     CDPHY_DEBUG_MODE_SELECT; // 2284
    REG_CDPHY_DEBUG_OUT_READ        CDPHY_DEBUG_OUT_READ; // 2288
    REG_DPHY_RX_INIT                DPHY_RX_INIT;     // 228C
}csi0_dphy_top_reg_REGS, *Pcsi0_dphy_top_reg_REGS;

#endif // __csi0_dphy_top_reg_REGS_H__
