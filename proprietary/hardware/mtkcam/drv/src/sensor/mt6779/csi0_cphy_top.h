#ifndef __csi0_cphy_top_reg_REGS_H__
#define __csi0_cphy_top_reg_REGS_H__

typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cphy_rx_tr0_lprx_en       : 1;
        FIELD cphy_rx_tr1_lprx_en       : 1;
        FIELD cphy_rx_tr2_lprx_en       : 1;
        FIELD cphy_rx_tr3_lprx_en       : 1;
        FIELD cphy_rx_tr0_hsrx_en       : 1;
        FIELD cphy_rx_tr1_hsrx_en       : 1;
        FIELD cphy_rx_tr2_hsrx_en       : 1;
        FIELD cphy_rx_tr3_hsrx_en       : 1;
        FIELD rsv_8                     : 8;
        FIELD cphy_rx_tr0_bist_en       : 1;
        FIELD cphy_rx_tr1_bist_en       : 1;
        FIELD cphy_rx_tr2_bist_en       : 1;
        FIELD cphy_rx_tr3_bist_en       : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_CTRL, *PREG_CPHY_RX_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_prbs_gen_data_delay_en : 1;
        FIELD rg_cphy_rx_lp_mode        : 1;
        FIELD rsv_2                     : 2;
        FIELD rg_cphy_rx_sync_detect_opt : 3;
        FIELD rsv_7                     : 9;
        FIELD rg_cphy_rx_tr0_init_wire_state : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr1_init_wire_state : 3;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr2_init_wire_state : 3;
        FIELD rsv_27                    : 1;
        FIELD rg_cphy_rx_tr3_init_wire_state : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_OPT, *PREG_CPHY_RX_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_detect_7s_dis_sync : 1;
        FIELD rg_cphy_rx_detect_7s_mask_sync : 7;
        FIELD rg_cphy_rx_detect_7s_word_sync : 21;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_DETECT_CTRL_SYNC, *PREG_CPHY_RX_DETECT_CTRL_SYNC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_detect_7s_dis_escape : 1;
        FIELD rg_cphy_rx_detect_7s_mask_escape : 7;
        FIELD rg_cphy_rx_detect_7s_word_escape : 21;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_DETECT_CTRL_ESCAPE, *PREG_CPHY_RX_DETECT_CTRL_ESCAPE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_detect_7s_dis_post : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_cphy_rx_data_valid_post_en : 1;
        FIELD rsv_5                     : 3;
        FIELD rg_cphy_rx_detect_7s_word_post : 21;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_DETECT_CTRL_POST, *PREG_CPHY_RX_DETECT_CTRL_POST;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_symbol_stream_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO0_DEBUG_0, *PREG_CPHY_RX_TRIO0_DEBUG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_symbol_stream_1 : 10;
        FIELD rsv_10                    : 2;
        FIELD ro_cphy_rx_tr0_symbol_stram_valid : 1;
        FIELD rsv_13                    : 3;
        FIELD ro_cphy_rx_tr0_detect_sync : 1;
        FIELD ro_cphy_rx_tr0_detect_escape : 1;
        FIELD ro_cphy_rx_tr0_detect_post : 1;
        FIELD rsv_19                    : 5;
        FIELD ro_cphy_rx_tr0_position_sync : 4;
        FIELD ro_cphy_rx_tr0_position_escape : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO0_DEBUG_1, *PREG_CPHY_RX_TRIO0_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_symbol_stream_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO1_DEBUG_0, *PREG_CPHY_RX_TRIO1_DEBUG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_symbol_stream_1 : 10;
        FIELD rsv_10                    : 2;
        FIELD ro_cphy_rx_tr1_symbol_stram_valid : 1;
        FIELD rsv_13                    : 3;
        FIELD ro_cphy_rx_tr1_detect_sync : 1;
        FIELD ro_cphy_rx_tr1_detect_escape : 1;
        FIELD ro_cphy_rx_tr1_detect_post : 1;
        FIELD rsv_19                    : 5;
        FIELD ro_cphy_rx_tr1_position_sync : 4;
        FIELD ro_cphy_rx_tr1_position_escape : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO1_DEBUG_1, *PREG_CPHY_RX_TRIO1_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_symbol_stream_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO2_DEBUG_0, *PREG_CPHY_RX_TRIO2_DEBUG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_symbol_stream_1 : 10;
        FIELD rsv_10                    : 2;
        FIELD ro_cphy_rx_tr2_symbol_stram_valid : 1;
        FIELD rsv_13                    : 3;
        FIELD ro_cphy_rx_tr2_detect_sync : 1;
        FIELD ro_cphy_rx_tr2_detect_escape : 1;
        FIELD ro_cphy_rx_tr2_detect_post : 1;
        FIELD rsv_19                    : 5;
        FIELD ro_cphy_rx_tr2_position_sync : 4;
        FIELD ro_cphy_rx_tr2_position_escape : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO2_DEBUG_1, *PREG_CPHY_RX_TRIO2_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_symbol_stream_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO3_DEBUG_0, *PREG_CPHY_RX_TRIO3_DEBUG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_symbol_stream_1 : 10;
        FIELD rsv_10                    : 2;
        FIELD ro_cphy_rx_tr3_symbol_stram_valid : 1;
        FIELD rsv_13                    : 3;
        FIELD ro_cphy_rx_tr3_detect_sync : 1;
        FIELD ro_cphy_rx_tr3_detect_escape : 1;
        FIELD ro_cphy_rx_tr3_detect_post : 1;
        FIELD rsv_19                    : 5;
        FIELD ro_cphy_rx_tr3_position_sync : 4;
        FIELD ro_cphy_rx_tr3_position_escape : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_TRIO3_DEBUG_1, *PREG_CPHY_RX_TRIO3_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr0_bist_progseq_s0 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s1 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s2 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s3 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s4 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s5 : 3;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s6 : 3;
        FIELD rsv_27                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s7 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_PROGSEQ_0, *PREG_CPHY_RX_BIST_TRIO0_PROGSEQ_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr0_bist_progseq_s8 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s9 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s10 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s11 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s12 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr0_bist_progseq_s13 : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_PROGSEQ_1, *PREG_CPHY_RX_BIST_TRIO0_PROGSEQ_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr1_bist_progseq_s0 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s1 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s2 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s3 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s4 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s5 : 3;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s6 : 3;
        FIELD rsv_27                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s7 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_PROGSEQ_0, *PREG_CPHY_RX_BIST_TRIO1_PROGSEQ_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr1_bist_progseq_s8 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s9 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s10 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s11 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s12 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr1_bist_progseq_s13 : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_PROGSEQ_1, *PREG_CPHY_RX_BIST_TRIO1_PROGSEQ_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr2_bist_progseq_s0 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s1 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s2 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s3 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s4 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s5 : 3;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s6 : 3;
        FIELD rsv_27                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s7 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_PROGSEQ_0, *PREG_CPHY_RX_BIST_TRIO2_PROGSEQ_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr2_bist_progseq_s8 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s9 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s10 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s11 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s12 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr2_bist_progseq_s13 : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_PROGSEQ_1, *PREG_CPHY_RX_BIST_TRIO2_PROGSEQ_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr3_bist_progseq_s0 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s1 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s2 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s3 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s4 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s5 : 3;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s6 : 3;
        FIELD rsv_27                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s7 : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_PROGSEQ_0, *PREG_CPHY_RX_BIST_TRIO3_PROGSEQ_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr3_bist_progseq_s8 : 3;
        FIELD rsv_3                     : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s9 : 3;
        FIELD rsv_7                     : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s10 : 3;
        FIELD rsv_11                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s11 : 3;
        FIELD rsv_15                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s12 : 3;
        FIELD rsv_19                    : 1;
        FIELD rg_cphy_rx_tr3_bist_progseq_s13 : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_PROGSEQ_1, *PREG_CPHY_RX_BIST_TRIO3_PROGSEQ_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr0_prbs_seed_0 : 8;
        FIELD rg_cphy_rx_tr0_prbs_seed_1 : 8;
        FIELD rg_cphy_rx_tr0_prbs_seed_2 : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_cphy_rx_tr0_bist_comp_mode : 1;
        FIELD rg_cphy_rx_tr0_fix_point_rst_mode : 1;
        FIELD cphy_rx_tr0_fix_point_rst : 1;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr0_prbs_pat_sel : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_CTRL, *PREG_CPHY_RX_BIST_TRIO0_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr1_prbs_seed_0 : 8;
        FIELD rg_cphy_rx_tr1_prbs_seed_1 : 8;
        FIELD rg_cphy_rx_tr1_prbs_seed_2 : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_cphy_rx_tr1_bist_comp_mode : 1;
        FIELD rg_cphy_rx_tr1_fix_point_rst_mode : 1;
        FIELD cphy_rx_tr1_fix_point_rst : 1;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr1_prbs_pat_sel : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_CTRL, *PREG_CPHY_RX_BIST_TRIO1_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr2_prbs_seed_0 : 8;
        FIELD rg_cphy_rx_tr2_prbs_seed_1 : 8;
        FIELD rg_cphy_rx_tr2_prbs_seed_2 : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_cphy_rx_tr2_bist_comp_mode : 1;
        FIELD rg_cphy_rx_tr2_fix_point_rst_mode : 1;
        FIELD cphy_rx_tr2_fix_point_rst : 1;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr2_prbs_pat_sel : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_CTRL, *PREG_CPHY_RX_BIST_TRIO2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr3_prbs_seed_0 : 8;
        FIELD rg_cphy_rx_tr3_prbs_seed_1 : 8;
        FIELD rg_cphy_rx_tr3_prbs_seed_2 : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_cphy_rx_tr3_bist_comp_mode : 1;
        FIELD rg_cphy_rx_tr3_fix_point_rst_mode : 1;
        FIELD cphy_rx_tr3_fix_point_rst : 1;
        FIELD rsv_23                    : 1;
        FIELD rg_cphy_rx_tr3_prbs_pat_sel : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_CTRL, *PREG_CPHY_RX_BIST_TRIO3_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_bist_lp_hs_lp_done : 1;
        FIELD ro_cphy_rx_tr1_bist_lp_hs_lp_done : 1;
        FIELD ro_cphy_rx_tr2_bist_lp_hs_lp_done : 1;
        FIELD ro_cphy_rx_tr3_bist_lp_hs_lp_done : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_DBG_STATUS, *PREG_CPHY_RX_BIST_DBG_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_bist_word_cnt_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_STATUS_0, *PREG_CPHY_RX_BIST_TRIO0_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_bist_word_cnt_1 : 16;
        FIELD ro_cphy_rx_tr0_bist_err_cnt : 8;
        FIELD ro_cphy_rx_tr0_bist_word_cnt_overflow : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_STATUS_1, *PREG_CPHY_RX_BIST_TRIO0_STATUS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_bist_err_record_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_STATUS_2, *PREG_CPHY_RX_BIST_TRIO0_STATUS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_bist_err_record_1 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO0_STATUS_3, *PREG_CPHY_RX_BIST_TRIO0_STATUS_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_bist_word_cnt_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_STATUS_0, *PREG_CPHY_RX_BIST_TRIO1_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_bist_word_cnt_1 : 16;
        FIELD ro_cphy_rx_tr1_bist_err_cnt : 8;
        FIELD ro_cphy_rx_tr1_bist_word_cnt_overflow : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_STATUS_1, *PREG_CPHY_RX_BIST_TRIO1_STATUS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_bist_err_record_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_STATUS_2, *PREG_CPHY_RX_BIST_TRIO1_STATUS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr1_bist_err_record_1 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO1_STATUS_3, *PREG_CPHY_RX_BIST_TRIO1_STATUS_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_bist_word_cnt_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_STATUS_0, *PREG_CPHY_RX_BIST_TRIO2_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_bist_word_cnt_1 : 16;
        FIELD ro_cphy_rx_tr2_bist_err_cnt : 8;
        FIELD ro_cphy_rx_tr2_bist_word_cnt_overflow : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_STATUS_1, *PREG_CPHY_RX_BIST_TRIO2_STATUS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_bist_err_record_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_STATUS_2, *PREG_CPHY_RX_BIST_TRIO2_STATUS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr2_bist_err_record_1 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO2_STATUS_3, *PREG_CPHY_RX_BIST_TRIO2_STATUS_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_bist_word_cnt_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_STATUS_0, *PREG_CPHY_RX_BIST_TRIO3_STATUS_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_bist_word_cnt_1 : 16;
        FIELD ro_cphy_rx_tr3_bist_err_cnt : 8;
        FIELD ro_cphy_rx_tr3_bist_word_cnt_overflow : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_STATUS_1, *PREG_CPHY_RX_BIST_TRIO3_STATUS_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_bist_err_record_0 : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_STATUS_2, *PREG_CPHY_RX_BIST_TRIO3_STATUS_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr3_bist_err_record_1 : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_BIST_TRIO3_STATUS_3, *PREG_CPHY_RX_BIST_TRIO3_STATUS_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cphy_rx_lb_trigger_sync_init : 1;
        FIELD cphy_rx_lb_release_sync_init : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_phyd2mac_cphy_gen_en   : 1;
        FIELD rg_phyd2mac_cphy_gen_mode : 1;
        FIELD rg_phyd2mac_cphy_gen_sel  : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_LOOPBACK_OPT, *PREG_CPHY_RX_LOOPBACK_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_tr0_fsm        : 7;
        FIELD rsv_7                     : 1;
        FIELD ro_cphy_rx_tr1_fsm        : 7;
        FIELD rsv_15                    : 1;
        FIELD ro_cphy_rx_tr2_fsm        : 7;
        FIELD rsv_23                    : 1;
        FIELD ro_cphy_rx_tr3_fsm        : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_FSM_STATUS, *PREG_CPHY_RX_FSM_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_tr0_err_sot_sync_hs_irq_en : 1;
        FIELD rg_cphy_rx_tr1_err_sot_sync_hs_irq_en : 1;
        FIELD rg_cphy_rx_tr2_err_sot_sync_hs_irq_en : 1;
        FIELD rg_cphy_rx_tr3_err_sot_sync_hs_irq_en : 1;
        FIELD rg_cphy_rx_tr0_detect_escape_pulse_irq_en : 1;
        FIELD rg_cphy_rx_tr1_detect_escape_pulse_irq_en : 1;
        FIELD rg_cphy_rx_tr2_detect_escape_pulse_irq_en : 1;
        FIELD rg_cphy_rx_tr3_detect_escape_pulse_irq_en : 1;
        FIELD rsv_8                     : 23;
        FIELD rg_cphy_rx_irq_w1c_en     : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_IRQ_EN, *PREG_CPHY_RX_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cphy_rx_tr0_err_sot_sync_hs_irq : 1;
        FIELD cphy_rx_tr1_err_sot_sync_hs_irq : 1;
        FIELD cphy_rx_tr2_err_sot_sync_hs_irq : 1;
        FIELD cphy_rx_tr3_err_sot_sync_hs_irq : 1;
        FIELD cphy_rx_tr0_detect_escape_pulse_irq : 1;
        FIELD cphy_rx_tr1_detect_escape_pulse_irq : 1;
        FIELD cphy_rx_tr2_detect_escape_pulse_irq : 1;
        FIELD cphy_rx_tr3_detect_escape_pulse_irq : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_IRQ_STATUS, *PREG_CPHY_RX_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cphy_rx_state_chk_en_trio0 : 1;
        FIELD cphy_rx_state_chk_en_trio1 : 1;
        FIELD cphy_rx_state_chk_en_trio2 : 1;
        FIELD cphy_rx_state_chk_en_trio3 : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_EN, *PREG_CPHY_RX_STATE_CHK_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_cphy_rx_state_chk_rx_timer_cnt : 32;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_TIMER, *PREG_CPHY_RX_STATE_CHK_TIMER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_state_chk_err_trio0 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_cphy_rx_state_chk_err_cnt_trio0 : 3;
        FIELD rsv_7                     : 9;
        FIELD cphy_rx_state_chk_fail_seq0_trio0 : 4;
        FIELD cphy_rx_state_chk_fail_seq1_trio0 : 4;
        FIELD cphy_rx_state_chk_fail_seq2_trio0 : 4;
        FIELD cphy_rx_state_chk_fail_seq3_trio0 : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_STATUS_TRIO0, *PREG_CPHY_RX_STATE_CHK_STATUS_TRIO0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_state_chk_err_trio1 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_cphy_rx_state_chk_err_cnt_trio1 : 3;
        FIELD rsv_7                     : 9;
        FIELD cphy_rx_state_chk_fail_seq0_trio1 : 4;
        FIELD cphy_rx_state_chk_fail_seq1_trio1 : 4;
        FIELD cphy_rx_state_chk_fail_seq2_trio1 : 4;
        FIELD cphy_rx_state_chk_fail_seq3_trio1 : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_STATUS_TRIO1, *PREG_CPHY_RX_STATE_CHK_STATUS_TRIO1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_state_chk_err_trio2 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_cphy_rx_state_chk_err_cnt_trio2 : 3;
        FIELD rsv_7                     : 9;
        FIELD cphy_rx_state_chk_fail_seq0_trio2 : 4;
        FIELD cphy_rx_state_chk_fail_seq1_trio2 : 4;
        FIELD cphy_rx_state_chk_fail_seq2_trio2 : 4;
        FIELD cphy_rx_state_chk_fail_seq3_trio2 : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_STATUS_TRIO2, *PREG_CPHY_RX_STATE_CHK_STATUS_TRIO2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_cphy_rx_state_chk_err_trio3 : 1;
        FIELD rsv_1                     : 3;
        FIELD ro_cphy_rx_state_chk_err_cnt_trio3 : 3;
        FIELD rsv_7                     : 9;
        FIELD cphy_rx_state_chk_fail_seq0_trio3 : 4;
        FIELD cphy_rx_state_chk_fail_seq1_trio3 : 4;
        FIELD cphy_rx_state_chk_fail_seq2_trio3 : 4;
        FIELD cphy_rx_state_chk_fail_seq3_trio3 : 4;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_STATE_CHK_STATUS_TRIO3, *PREG_CPHY_RX_STATE_CHK_STATUS_TRIO3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_tinit_cnt         : 20;
        FIELD rsv_20                    : 11;
        FIELD rg_csi2_tinit_cnt_en      : 1;
    } Bits;
    UINT32 Raw;
} REG_CPHY_RX_INIT, *PREG_CPHY_RX_INIT;

// ----------------- csi0_cphy_top_reg Register Definition -------------------
//0x11c8 3000
typedef volatile struct
{
    REG_CPHY_RX_CTRL                CPHY_RX_CTRL;     // 3000
    UINT32                          rsv_3004;         // 3004
    REG_CPHY_RX_OPT                 CPHY_RX_OPT;      // 3008
    UINT32                          rsv_300C;         // 300C
    REG_CPHY_RX_DETECT_CTRL_SYNC    CPHY_RX_DETECT_CTRL_SYNC; // 3010
    REG_CPHY_RX_DETECT_CTRL_ESCAPE  CPHY_RX_DETECT_CTRL_ESCAPE; // 3014
    REG_CPHY_RX_DETECT_CTRL_POST    CPHY_RX_DETECT_CTRL_POST; // 3018
    UINT32                          rsv_301C;         // 301C
    REG_CPHY_RX_TRIO0_DEBUG_0       CPHY_RX_TRIO0_DEBUG_0; // 3020
    REG_CPHY_RX_TRIO0_DEBUG_1       CPHY_RX_TRIO0_DEBUG_1; // 3024
    UINT32                          rsv_3028[2];      // 3028..302C
    REG_CPHY_RX_TRIO1_DEBUG_0       CPHY_RX_TRIO1_DEBUG_0; // 3030
    REG_CPHY_RX_TRIO1_DEBUG_1       CPHY_RX_TRIO1_DEBUG_1; // 3034
    UINT32                          rsv_3038[2];      // 3038..303C
    REG_CPHY_RX_TRIO2_DEBUG_0       CPHY_RX_TRIO2_DEBUG_0; // 3040
    REG_CPHY_RX_TRIO2_DEBUG_1       CPHY_RX_TRIO2_DEBUG_1; // 3044
    UINT32                          rsv_3048[2];      // 3048..304C
    REG_CPHY_RX_TRIO3_DEBUG_0       CPHY_RX_TRIO3_DEBUG_0; // 3050
    REG_CPHY_RX_TRIO3_DEBUG_1       CPHY_RX_TRIO3_DEBUG_1; // 3054
    UINT32                          rsv_3058[2];      // 3058..305C
    REG_CPHY_RX_BIST_TRIO0_PROGSEQ_0 CPHY_RX_BIST_TRIO0_PROGSEQ_0; // 3060
    REG_CPHY_RX_BIST_TRIO0_PROGSEQ_1 CPHY_RX_BIST_TRIO0_PROGSEQ_1; // 3064
    REG_CPHY_RX_BIST_TRIO1_PROGSEQ_0 CPHY_RX_BIST_TRIO1_PROGSEQ_0; // 3068
    REG_CPHY_RX_BIST_TRIO1_PROGSEQ_1 CPHY_RX_BIST_TRIO1_PROGSEQ_1; // 306C
    REG_CPHY_RX_BIST_TRIO2_PROGSEQ_0 CPHY_RX_BIST_TRIO2_PROGSEQ_0; // 3070
    REG_CPHY_RX_BIST_TRIO2_PROGSEQ_1 CPHY_RX_BIST_TRIO2_PROGSEQ_1; // 3074
    REG_CPHY_RX_BIST_TRIO3_PROGSEQ_0 CPHY_RX_BIST_TRIO3_PROGSEQ_0; // 3078
    REG_CPHY_RX_BIST_TRIO3_PROGSEQ_1 CPHY_RX_BIST_TRIO3_PROGSEQ_1; // 307C
    REG_CPHY_RX_BIST_TRIO0_CTRL     CPHY_RX_BIST_TRIO0_CTRL; // 3080
    REG_CPHY_RX_BIST_TRIO1_CTRL     CPHY_RX_BIST_TRIO1_CTRL; // 3084
    REG_CPHY_RX_BIST_TRIO2_CTRL     CPHY_RX_BIST_TRIO2_CTRL; // 3088
    REG_CPHY_RX_BIST_TRIO3_CTRL     CPHY_RX_BIST_TRIO3_CTRL; // 308C
    REG_CPHY_RX_BIST_DBG_STATUS     CPHY_RX_BIST_DBG_STATUS; // 3090
    UINT32                          rsv_3094[3];      // 3094..309C
    REG_CPHY_RX_BIST_TRIO0_STATUS_0 CPHY_RX_BIST_TRIO0_STATUS_0; // 30A0
    REG_CPHY_RX_BIST_TRIO0_STATUS_1 CPHY_RX_BIST_TRIO0_STATUS_1; // 30A4
    REG_CPHY_RX_BIST_TRIO0_STATUS_2 CPHY_RX_BIST_TRIO0_STATUS_2; // 30A8
    REG_CPHY_RX_BIST_TRIO0_STATUS_3 CPHY_RX_BIST_TRIO0_STATUS_3; // 30AC
    REG_CPHY_RX_BIST_TRIO1_STATUS_0 CPHY_RX_BIST_TRIO1_STATUS_0; // 30B0
    REG_CPHY_RX_BIST_TRIO1_STATUS_1 CPHY_RX_BIST_TRIO1_STATUS_1; // 30B4
    REG_CPHY_RX_BIST_TRIO1_STATUS_2 CPHY_RX_BIST_TRIO1_STATUS_2; // 30B8
    REG_CPHY_RX_BIST_TRIO1_STATUS_3 CPHY_RX_BIST_TRIO1_STATUS_3; // 30BC
    REG_CPHY_RX_BIST_TRIO2_STATUS_0 CPHY_RX_BIST_TRIO2_STATUS_0; // 30C0
    REG_CPHY_RX_BIST_TRIO2_STATUS_1 CPHY_RX_BIST_TRIO2_STATUS_1; // 30C4
    REG_CPHY_RX_BIST_TRIO2_STATUS_2 CPHY_RX_BIST_TRIO2_STATUS_2; // 30C8
    REG_CPHY_RX_BIST_TRIO2_STATUS_3 CPHY_RX_BIST_TRIO2_STATUS_3; // 30CC
    REG_CPHY_RX_BIST_TRIO3_STATUS_0 CPHY_RX_BIST_TRIO3_STATUS_0; // 30D0
    REG_CPHY_RX_BIST_TRIO3_STATUS_1 CPHY_RX_BIST_TRIO3_STATUS_1; // 30D4
    REG_CPHY_RX_BIST_TRIO3_STATUS_2 CPHY_RX_BIST_TRIO3_STATUS_2; // 30D8
    REG_CPHY_RX_BIST_TRIO3_STATUS_3 CPHY_RX_BIST_TRIO3_STATUS_3; // 30DC
    REG_CPHY_RX_LOOPBACK_OPT        CPHY_RX_LOOPBACK_OPT; // 30E0
    REG_CPHY_RX_FSM_STATUS          CPHY_RX_FSM_STATUS; // 30E4
    UINT32                          rsv_30E8[2];      // 30E8..30EC
    REG_CPHY_RX_IRQ_EN              CPHY_RX_IRQ_EN;   // 30F0
    REG_CPHY_RX_IRQ_STATUS          CPHY_RX_IRQ_STATUS; // 30F4
    UINT32                          rsv_30F8[2];      // 30F8..30FC
    REG_CPHY_RX_STATE_CHK_EN        CPHY_RX_STATE_CHK_EN; // 3100
    REG_CPHY_RX_STATE_CHK_TIMER     CPHY_RX_STATE_CHK_TIMER; // 3104
    UINT32                          rsv_3108[2];      // 3108..310C
    REG_CPHY_RX_STATE_CHK_STATUS_TRIO0 CPHY_RX_STATE_CHK_STATUS_TRIO0; // 3110
    REG_CPHY_RX_STATE_CHK_STATUS_TRIO1 CPHY_RX_STATE_CHK_STATUS_TRIO1; // 3114
    REG_CPHY_RX_STATE_CHK_STATUS_TRIO2 CPHY_RX_STATE_CHK_STATUS_TRIO2; // 3118
    REG_CPHY_RX_STATE_CHK_STATUS_TRIO3 CPHY_RX_STATE_CHK_STATUS_TRIO3; // 311C
    UINT32                          rsv_3120[5];      // 3120..3130
    REG_CPHY_RX_INIT                CPHY_RX_INIT;     // 3134
}csi0_cphy_top_reg_REGS, *Pcsi0_cphy_top_reg_REGS;

#endif // __csi0_cphy_top_reg_REGS_H__
