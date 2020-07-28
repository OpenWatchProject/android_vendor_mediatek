#ifndef __seninf1_csi2_REGS_H__
#define __seninf1_csi2_REGS_H__

typedef unsigned int FIELD;
typedef unsigned int UINT32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD csi2_lane0_en             : 1;
        FIELD csi2_lane1_en             : 1;
        FIELD csi2_lane2_en             : 1;
        FIELD csi2_lane3_en             : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_EN, *PREG_SENINF_CSI2_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_cphy_sel          : 1;
        FIELD rg_csi2_ecc_en            : 1;
        FIELD rg_csi2_b2p_en            : 1;
        FIELD rg_csi2_generic_long_packet_en : 1;
        FIELD rg_csi2_img_packet_en     : 1;
        FIELD rg_csi2_spec_v2p0_sel     : 1;
        FIELD rg_csi2_descramble_en     : 1;
        FIELD rsv_7                     : 1;
        FIELD rg_csi2_vs_output_mode    : 1;
        FIELD rg_csi2_vs_ouput_len_sel  : 1;
        FIELD rsv_10                    : 2;
        FIELD rg_csi2_hsync_pol         : 1;
        FIELD rg_csi2_vsync_pol         : 1;
        FIELD rsv_14                    : 2;
        FIELD rg_csi2_fifo_push_en      : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_OPT, *PREG_SENINF_CSI2_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_header_mode       : 8;
        FIELD rg_csi2_header_len        : 3;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_HDR_MODE_0, *PREG_SENINF_CSI2_HDR_MODE_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_cphy_header_di_pos : 8;
        FIELD rg_csi2_cphy_header_wc_pos : 8;
        FIELD rg_csi2_cphy_header_vcx_pos : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_HDR_MODE_1, *PREG_SENINF_CSI2_HDR_MODE_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_resync_cycle_cnt  : 5;
        FIELD rsv_5                     : 3;
        FIELD rg_csi2_resync_cycle_cnt_opt : 1;
        FIELD rg_csi2_resync_dataout_opt : 1;
        FIELD rg_csi2_resync_bypass     : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_RESYNC_MERGE_CTRL, *PREG_SENINF_CSI2_RESYNC_MERGE_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_dpcm_mode         : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_csi2_dt_30_dpcm_en     : 1;
        FIELD rg_csi2_dt_31_dpcm_en     : 1;
        FIELD rg_csi2_dt_32_dpcm_en     : 1;
        FIELD rg_csi2_dt_33_dpcm_en     : 1;
        FIELD rg_csi2_dt_34_dpcm_en     : 1;
        FIELD rg_csi2_dt_35_dpcm_en     : 1;
        FIELD rg_csi2_dt_36_dpcm_en     : 1;
        FIELD rg_csi2_dt_37_dpcm_en     : 1;
        FIELD rg_csi2_dt_2a_dpcm_en     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_DPCM_CTRL, *PREG_SENINF_CSI2_DPCM_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s0_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s0_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s0_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s0_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S0_DI_CTRL, *PREG_SENINF_CSI2_S0_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s1_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s1_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s1_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s1_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S1_DI_CTRL, *PREG_SENINF_CSI2_S1_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s2_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s2_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s2_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s2_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S2_DI_CTRL, *PREG_SENINF_CSI2_S2_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s3_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s3_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s3_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s3_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S3_DI_CTRL, *PREG_SENINF_CSI2_S3_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s4_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s4_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s4_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s4_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S4_DI_CTRL, *PREG_SENINF_CSI2_S4_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s5_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s5_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s5_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s5_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S5_DI_CTRL, *PREG_SENINF_CSI2_S5_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s6_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s6_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s6_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s6_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S6_DI_CTRL, *PREG_SENINF_CSI2_S6_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_s7_vc_interleave_en : 1;
        FIELD rsv_1                     : 3;
        FIELD rg_csi2_s7_dt_interleave_mode : 2;
        FIELD rsv_6                     : 2;
        FIELD rg_csi2_s7_vc_sel         : 5;
        FIELD rsv_13                    : 3;
        FIELD rg_csi2_s7_dt_sel         : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_S7_DI_CTRL, *PREG_SENINF_CSI2_S7_DI_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_ch0_grp_mode      : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_csi2_ch0_s0_grp_en     : 1;
        FIELD rg_csi2_ch0_s1_grp_en     : 1;
        FIELD rg_csi2_ch0_s2_grp_en     : 1;
        FIELD rg_csi2_ch0_s3_grp_en     : 1;
        FIELD rg_csi2_ch0_s4_grp_en     : 1;
        FIELD rg_csi2_ch0_s5_grp_en     : 1;
        FIELD rg_csi2_ch0_s6_grp_en     : 1;
        FIELD rg_csi2_ch0_s7_grp_en     : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_CH0_CTRL, *PREG_SENINF_CSI2_CH0_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_ch1_grp_mode      : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_csi2_ch1_s0_grp_en     : 1;
        FIELD rg_csi2_ch1_s1_grp_en     : 1;
        FIELD rg_csi2_ch1_s2_grp_en     : 1;
        FIELD rg_csi2_ch1_s3_grp_en     : 1;
        FIELD rg_csi2_ch1_s4_grp_en     : 1;
        FIELD rg_csi2_ch1_s5_grp_en     : 1;
        FIELD rg_csi2_ch1_s6_grp_en     : 1;
        FIELD rg_csi2_ch1_s7_grp_en     : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_CH1_CTRL, *PREG_SENINF_CSI2_CH1_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_ch2_grp_mode      : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_csi2_ch2_s0_grp_en     : 1;
        FIELD rg_csi2_ch2_s1_grp_en     : 1;
        FIELD rg_csi2_ch2_s2_grp_en     : 1;
        FIELD rg_csi2_ch2_s3_grp_en     : 1;
        FIELD rg_csi2_ch2_s4_grp_en     : 1;
        FIELD rg_csi2_ch2_s5_grp_en     : 1;
        FIELD rg_csi2_ch2_s6_grp_en     : 1;
        FIELD rg_csi2_ch2_s7_grp_en     : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_CH2_CTRL, *PREG_SENINF_CSI2_CH2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_ch3_grp_mode      : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_csi2_ch3_s0_grp_en     : 1;
        FIELD rg_csi2_ch3_s1_grp_en     : 1;
        FIELD rg_csi2_ch3_s2_grp_en     : 1;
        FIELD rg_csi2_ch3_s3_grp_en     : 1;
        FIELD rg_csi2_ch3_s4_grp_en     : 1;
        FIELD rg_csi2_ch3_s5_grp_en     : 1;
        FIELD rg_csi2_ch3_s6_grp_en     : 1;
        FIELD rg_csi2_ch3_s7_grp_en     : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_CH3_CTRL, *PREG_SENINF_CSI2_CH3_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l0_descramble_type0_seed : 16;
        FIELD rg_csi2_l0_descramble_type1_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L0_DESCRAMBLE_SEED_0, *PREG_SENINF_CSI2_L0_DESCRAMBLE_SEED_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l0_descramble_type2_seed : 16;
        FIELD rg_csi2_l0_descramble_type3_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L0_DESCRAMBLE_SEED_1, *PREG_SENINF_CSI2_L0_DESCRAMBLE_SEED_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l1_descramble_type0_seed : 16;
        FIELD rg_csi2_l1_descramble_type1_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L1_DESCRAMBLE_SEED_0, *PREG_SENINF_CSI2_L1_DESCRAMBLE_SEED_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l1_descramble_type2_seed : 16;
        FIELD rg_csi2_l1_descramble_type3_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L1_DESCRAMBLE_SEED_1, *PREG_SENINF_CSI2_L1_DESCRAMBLE_SEED_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l2_descramble_type0_seed : 16;
        FIELD rg_csi2_l2_descramble_type1_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L2_DESCRAMBLE_SEED_0, *PREG_SENINF_CSI2_L2_DESCRAMBLE_SEED_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l2_descramble_type2_seed : 16;
        FIELD rg_csi2_l2_descramble_type3_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L2_DESCRAMBLE_SEED_1, *PREG_SENINF_CSI2_L2_DESCRAMBLE_SEED_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l3_descramble_type0_seed : 16;
        FIELD rg_csi2_l3_descramble_type1_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L3_DESCRAMBLE_SEED_0, *PREG_SENINF_CSI2_L3_DESCRAMBLE_SEED_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_l3_descramble_type2_seed : 16;
        FIELD rg_csi2_l3_descramble_type3_seed : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_L3_DESCRAMBLE_SEED_1, *PREG_SENINF_CSI2_L3_DESCRAMBLE_SEED_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_err_frame_sync_irq_en : 1;
        FIELD rg_csi2_err_id_irq_en     : 1;
        FIELD rg_csi2_ecc_err_undetected_irq_en : 1;
        FIELD rg_csi2_ecc_err_corrected_irq_en : 1;
        FIELD rg_csi2_ecc_err_double_irq_en : 1;
        FIELD rg_csi2_crc_correct_irq_en : 1;
        FIELD rg_csi2_crc_err_irq_en    : 1;
        FIELD rg_csi2_err_multi_lane_sync_irq_en : 1;
        FIELD rg_csi2_fs_receive_irq_en : 1;
        FIELD rg_csi2_fe_receive_irq_en : 1;
        FIELD rg_csi2_ls_receive_irq_en : 1;
        FIELD rg_csi2_le_receive_irq_en : 1;
        FIELD rg_csi2_gs_receive_irq_en : 1;
        FIELD rg_csi2_err_lane_resync_irq_en : 1;
        FIELD rg_csi2_lane_merge_fifo_af_irq_en : 1;
        FIELD rsv_15                    : 1;
        FIELD rg_csi2_err_frame_sync_s0_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s1_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s2_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s3_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s4_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s5_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s6_irq_en : 1;
        FIELD rg_csi2_err_frame_sync_s7_irq_en : 1;
        FIELD rg_csi2_resync_fifo_overflow_l0_irq_en : 1;
        FIELD rg_csi2_resync_fifo_overflow_l1_irq_en : 1;
        FIELD rg_csi2_resync_fifo_overflow_l2_irq_en : 1;
        FIELD rg_csi2_resync_fifo_overflow_l3_irq_en : 1;
        FIELD rg_csi2_async_fifo_overrun_irq_en : 1;
        FIELD rg_csi2_receive_data_not_enough_irq_en : 1;
        FIELD rsv_30                    : 1;
        FIELD rg_csi2_irq_clr_mode      : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_IRQ_EN, *PREG_SENINF_CSI2_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_err_frame_sync_irq : 1;
        FIELD ro_csi2_err_id_irq        : 1;
        FIELD ro_csi2_ecc_err_undetected_irq : 1;
        FIELD ro_csi2_ecc_err_corrected_irq : 1;
        FIELD ro_csi2_ecc_err_double_irq : 1;
        FIELD ro_csi2_crc_correct_irq   : 1;
        FIELD ro_csi2_crc_err_irq       : 1;
        FIELD ro_csi2_err_multi_lane_sync_irq : 1;
        FIELD ro_csi2_fs_receive_irq    : 1;
        FIELD ro_csi2_fe_receive_irq    : 1;
        FIELD ro_csi2_ls_receive_irq    : 1;
        FIELD ro_csi2_le_receive_irq    : 1;
        FIELD ro_csi2_gs_receive_irq    : 1;
        FIELD ro_csi2_err_lane_resync_irq : 1;
        FIELD ro_csi2_lane_merge_fifo_af_irq : 1;
        FIELD rsv_15                    : 1;
        FIELD ro_csi2_err_frame_sync_s0_irq : 1;
        FIELD ro_csi2_err_frame_sync_s1_irq : 1;
        FIELD ro_csi2_err_frame_sync_s2_irq : 1;
        FIELD ro_csi2_err_frame_sync_s3_irq : 1;
        FIELD ro_csi2_err_frame_sync_s4_irq : 1;
        FIELD ro_csi2_err_frame_sync_s5_irq : 1;
        FIELD ro_csi2_err_frame_sync_s6_irq : 1;
        FIELD ro_csi2_err_frame_sync_s7_irq : 1;
        FIELD ro_csi2_resync_fifo_overflow_l0_irq : 1;
        FIELD ro_csi2_resync_fifo_overflow_l1_irq : 1;
        FIELD ro_csi2_resync_fifo_overflow_l2_irq : 1;
        FIELD ro_csi2_resync_fifo_overflow_l3_irq : 1;
        FIELD ro_csi2_async_fifo_overrun_irq : 1;
        FIELD ro_csi2_receive_data_not_enough_irq : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_IRQ_STATUS, *PREG_SENINF_CSI2_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_line_num          : 16;
        FIELD ro_csi2_frame_num         : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_LINE_FRAME_NUM, *PREG_SENINF_CSI2_LINE_FRAME_NUM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_packet_dt         : 6;
        FIELD rsv_6                     : 2;
        FIELD ro_csi2_packet_vc         : 5;
        FIELD rsv_13                    : 3;
        FIELD ro_csi2_packet_wc         : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_PACKET_STATUS, *PREG_SENINF_CSI2_PACKET_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_generic_short_packet_dt : 6;
        FIELD rsv_6                     : 10;
        FIELD ro_csi2_generic_short_packet_wc : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_GEN_SHORT_PACKET_STATUS, *PREG_SENINF_CSI2_GEN_SHORT_PACKET_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_packet_cnt        : 16;
        FIELD ro_csi2_packet_cnt_buf    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_PACKET_CNT_STATUS, *PREG_SENINF_CSI2_PACKET_CNT_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_dbg_sel           : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_csi2_dbg_en            : 1;
        FIELD rg_csi2_dbg_packet_cnt_en : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_DBG_CTRL, *PREG_SENINF_CSI2_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_csi2_dbg_out           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_DBG_OUT, *PREG_SENINF_CSI2_DBG_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_csi2_spare_0           : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_csi2_spare_1           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_SPARE, *PREG_SENINF_CSI2_SPARE;

// ----------------- seninf1_csi2 Register Definition -------------------
typedef volatile struct /*0x1A004A00*/
{
    REG_SENINF_CSI2_EN              SENINF_CSI2_EN;   // 4A00
    REG_SENINF_CSI2_OPT             SENINF_CSI2_OPT;  // 4A04
    REG_SENINF_CSI2_HDR_MODE_0      SENINF_CSI2_HDR_MODE_0; // 4A08
    REG_SENINF_CSI2_HDR_MODE_1      SENINF_CSI2_HDR_MODE_1; // 4A0C
    REG_SENINF_CSI2_RESYNC_MERGE_CTRL SENINF_CSI2_RESYNC_MERGE_CTRL; // 4A10
    UINT32                          rsv_4A14;         // 4A14
    REG_SENINF_CSI2_DPCM_CTRL       SENINF_CSI2_DPCM_CTRL; // 4A18
    UINT32                          rsv_4A1C;         // 4A1C
    REG_SENINF_CSI2_S0_DI_CTRL      SENINF_CSI2_S0_DI_CTRL; // 4A20
    REG_SENINF_CSI2_S1_DI_CTRL      SENINF_CSI2_S1_DI_CTRL; // 4A24
    REG_SENINF_CSI2_S2_DI_CTRL      SENINF_CSI2_S2_DI_CTRL; // 4A28
    REG_SENINF_CSI2_S3_DI_CTRL      SENINF_CSI2_S3_DI_CTRL; // 4A2C
    REG_SENINF_CSI2_S4_DI_CTRL      SENINF_CSI2_S4_DI_CTRL; // 4A30
    REG_SENINF_CSI2_S5_DI_CTRL      SENINF_CSI2_S5_DI_CTRL; // 4A34
    REG_SENINF_CSI2_S6_DI_CTRL      SENINF_CSI2_S6_DI_CTRL; // 4A38
    REG_SENINF_CSI2_S7_DI_CTRL      SENINF_CSI2_S7_DI_CTRL; // 4A3C
    UINT32                          rsv_4A40[8];      // 4A40..4A5C
    REG_SENINF_CSI2_CH0_CTRL        SENINF_CSI2_CH0_CTRL; // 4A60
    REG_SENINF_CSI2_CH1_CTRL        SENINF_CSI2_CH1_CTRL; // 4A64
    REG_SENINF_CSI2_CH2_CTRL        SENINF_CSI2_CH2_CTRL; // 4A68
    REG_SENINF_CSI2_CH3_CTRL        SENINF_CSI2_CH3_CTRL; // 4A6C
    UINT32                          rsv_4A70[4];      // 4A70..4A7C
    REG_SENINF_CSI2_L0_DESCRAMBLE_SEED_0 SENINF_CSI2_L0_DESCRAMBLE_SEED_0; // 4A80
    REG_SENINF_CSI2_L0_DESCRAMBLE_SEED_1 SENINF_CSI2_L0_DESCRAMBLE_SEED_1; // 4A84
    UINT32                          rsv_4A88[2];      // 4A88..4A8C
    REG_SENINF_CSI2_L1_DESCRAMBLE_SEED_0 SENINF_CSI2_L1_DESCRAMBLE_SEED_0; // 4A90
    REG_SENINF_CSI2_L1_DESCRAMBLE_SEED_1 SENINF_CSI2_L1_DESCRAMBLE_SEED_1; // 4A94
    UINT32                          rsv_4A98[2];      // 4A98..4A9C
    REG_SENINF_CSI2_L2_DESCRAMBLE_SEED_0 SENINF_CSI2_L2_DESCRAMBLE_SEED_0; // 4AA0
    REG_SENINF_CSI2_L2_DESCRAMBLE_SEED_1 SENINF_CSI2_L2_DESCRAMBLE_SEED_1; // 4AA4
    UINT32                          rsv_4AA8[2];      // 4AA8..4AAC
    REG_SENINF_CSI2_L3_DESCRAMBLE_SEED_0 SENINF_CSI2_L3_DESCRAMBLE_SEED_0; // 4AB0
    REG_SENINF_CSI2_L3_DESCRAMBLE_SEED_1 SENINF_CSI2_L3_DESCRAMBLE_SEED_1; // 4AB4
    UINT32                          rsv_4AB8[2];      // 4AB8..4ABC
    REG_SENINF_CSI2_IRQ_EN          SENINF_CSI2_IRQ_EN; // 4AC0
    UINT32                          rsv_4AC4;         // 4AC4
    REG_SENINF_CSI2_IRQ_STATUS      SENINF_CSI2_IRQ_STATUS; // 4AC8
    UINT32                          rsv_4ACC;         // 4ACC
    REG_SENINF_CSI2_LINE_FRAME_NUM  SENINF_CSI2_LINE_FRAME_NUM; // 4AD0
    REG_SENINF_CSI2_PACKET_STATUS   SENINF_CSI2_PACKET_STATUS; // 4AD4
    REG_SENINF_CSI2_GEN_SHORT_PACKET_STATUS SENINF_CSI2_GEN_SHORT_PACKET_STATUS; // 4AD8
    REG_SENINF_CSI2_PACKET_CNT_STATUS SENINF_CSI2_PACKET_CNT_STATUS; // 4ADC
    REG_SENINF_CSI2_DBG_CTRL        SENINF_CSI2_DBG_CTRL; // 4AE0
    UINT32                          rsv_4AE4[4];      // 4AE4..4AF0
    REG_SENINF_CSI2_DBG_OUT         SENINF_CSI2_DBG_OUT; // 4AF4
    REG_SENINF_CSI2_SPARE           SENINF_CSI2_SPARE; // 4AF8
}seninf1_csi2_REGS, *Pseninf1_csi2_REGS;

#endif // __seninf1_csi2_REGS_H__
