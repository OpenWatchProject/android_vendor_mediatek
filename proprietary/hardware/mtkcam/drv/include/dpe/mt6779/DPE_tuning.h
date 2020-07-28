// ----------------- DPE_DVS_ME Bit Field Definitions -------------------

#define PACKING
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_dv_cand_num             : 4;
        FIELD rsv_4                     : 5;
        FIELD c_ord_pnlty_shift_dir     : 1;
        FIELD c_ord_pnlty_shift_num     : 2;
        FIELD rsv_12                    : 4;
        FIELD c_ord_th                  : 10;
        FIELD rsv_26                    : 2;
        FIELD c_ord_coring              : 4;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_00, *PREG_DVS_ME_00;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_sm_dv_x_pnlty_sel       : 4;
        FIELD c_sm_dv_y_pnlty_sel       : 4;
        FIELD c_sm_dv_ada_base          : 4;
        FIELD rsv_12                    : 2;
        FIELD c_sm_dv_th                : 8;
        FIELD c_sm_dv_th2               : 4;
        FIELD rsv_26                    : 6;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_01, *PREG_DVS_ME_01;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_sptl_pnlty_sel          : 4;
        FIELD c_tmpr_pnlty_sel          : 4;
        FIELD c_rand_pnlty_sel          : 4;
        FIELD rsv_12                    : 4;
        FIELD c_nbr_pnlty_sel           : 4;
        FIELD c_prev_pnlty_sel          : 4;
        FIELD c_gmv_pnlty_sel           : 4;
        FIELD c_refine_pnlty_sel        : 4;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_02, *PREG_DVS_ME_02;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_sptl_cost               : 4;
        FIELD c_tmpr_cost               : 4;
        FIELD c_refine_cost             : 4;
        FIELD c_nbr_cost                : 4;
        FIELD c_prev_cost               : 4;
        FIELD c_gmv_cost                : 4;
        FIELD c_rand_cost               : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_03, *PREG_DVS_ME_03;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_bg_mv_th                : 8;
        FIELD c_bg_mv_pnlty_sel         : 4;
        FIELD c_bg_mv_cost              : 4;
        FIELD c_bg_mv_diff              : 6;
        FIELD rsv_22                    : 6;
        FIELD c_conf_div                : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_04, *PREG_DVS_ME_04;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_seed_l_c04              : 6;
        FIELD c_seed_l_c03              : 6;
        FIELD c_seed_l_c02              : 6;
        FIELD c_seed_l_c01              : 6;
        FIELD c_seed_l_c00              : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_05, *PREG_DVS_ME_05;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_seed_r_c01              : 6;
        FIELD c_seed_r_c00              : 6;
        FIELD c_seed_l_c07              : 6;
        FIELD c_seed_l_c06              : 6;
        FIELD c_seed_l_c05              : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_06, *PREG_DVS_ME_06;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_seed_r_c06              : 6;
        FIELD c_seed_r_c05              : 6;
        FIELD c_seed_r_c04              : 6;
        FIELD c_seed_r_c03              : 6;
        FIELD c_seed_r_c02              : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_07, *PREG_DVS_ME_07;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_dv_ini_x                : 9;
        FIELD c_gmv_y1                  : 5;
        FIELD c_gmv_x1                  : 11;
        FIELD c_seed_r_c07              : 6;
        FIELD rsv_31                    : 1;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_08, *PREG_DVS_ME_08;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_type_l_c07              : 3;
        FIELD c_type_l_c06              : 3;
        FIELD c_type_l_c05              : 3;
        FIELD c_type_l_c04              : 3;
        FIELD c_type_l_c03              : 3;
        FIELD c_type_l_c02              : 3;
        FIELD c_type_l_c01              : 3;
        FIELD c_type_l_c00              : 3;
        FIELD rsv_24                    : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_09, *PREG_DVS_ME_09;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_type_r_c07              : 3;
        FIELD c_type_r_c06              : 3;
        FIELD c_type_r_c05              : 3;
        FIELD c_type_r_c04              : 3;
        FIELD c_type_r_c03              : 3;
        FIELD c_type_r_c02              : 3;
        FIELD c_type_r_c01              : 3;
        FIELD c_type_r_c00              : 3;
        FIELD rsv_24                    : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_10, *PREG_DVS_ME_10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sta_shift_h               : 11;
        FIELD sta_shift_v               : 5;
        FIELD rsv_16                    : 16;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_11, *PREG_DVS_ME_11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rand_max_mode           : 4;
        FIELD rsv_4                     : 4;
        FIELD c_rand_round_offset       : 8;
        FIELD rsv_16                    : 2;
        FIELD c_rand_init               : 2;
        FIELD c_rand1_vec_base_mv_shift : 1;
        FIELD c_rand2_vec_base_mv_shift : 1;
        FIELD c_refine_vec_base_mv_shift : 1;
        FIELD c_rand3_vec_base_mv_shift : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_12, *PREG_DVS_ME_12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 18;
        FIELD c_rand_dv4                : 9;
        FIELD rsv_27                    : 5;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_13, *PREG_DVS_ME_13;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rand_dv1                : 9;
        FIELD c_rand_dv2                : 9;
        FIELD c_rand_dv3                : 9;
        FIELD rsv_27                    : 5;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_14, *PREG_DVS_ME_14;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_ad_shift                : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_15, *PREG_DVS_ME_15;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_ad_c07              : 4;
        FIELD c_lut_ad_c06              : 4;
        FIELD c_lut_ad_c05              : 4;
        FIELD c_lut_ad_c04              : 4;
        FIELD c_lut_ad_c03              : 4;
        FIELD c_lut_ad_c02              : 4;
        FIELD c_lut_ad_c01              : 4;
        FIELD c_lut_ad_c00              : 4;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_16, *PREG_DVS_ME_16;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_ad_c15              : 4;
        FIELD c_lut_ad_c14              : 4;
        FIELD c_lut_ad_c13              : 4;
        FIELD c_lut_ad_c12              : 4;
        FIELD c_lut_ad_c11              : 4;
        FIELD c_lut_ad_c10              : 4;
        FIELD c_lut_ad_c09              : 4;
        FIELD c_lut_ad_c08              : 4;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_17, *PREG_DVS_ME_17;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_seed_priority_r         : 8;
        FIELD rsv_8                     : 8;
        FIELD c_seed_priority_l         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_18, *PREG_DVS_ME_18;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_sr_h                    : 10;
        FIELD c_sr_v                    : 4;
        FIELD c_zrv                     : 8;
        FIELD rsv_22                    : 10;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_19, *PREG_DVS_ME_19;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_var_frm_th              : 4;
        FIELD c_coner_th                : 6;
        FIELD c_sr_h_warp               : 10;
        FIELD rsv_20                    : 12;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_20, *PREG_DVS_ME_20;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD c_lp_box_en               : 1;
        FIELD rsv_2                     : 4;
        FIELD c_as_en                   : 1;
        FIELD rsv_7                     : 6;
        FIELD c_conf_mode               : 1;
        FIELD rsv_14                    : 6;
        FIELD c_as_th                   : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_21, *PREG_DVS_ME_21;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_cmlm_c03            : 8;
        FIELD c_lut_cmlm_c02            : 8;
        FIELD c_lut_cmlm_c01            : 8;
        FIELD c_lut_cmlm_c00            : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_22, *PREG_DVS_ME_22;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_cmlm_c07            : 8;
        FIELD c_lut_cmlm_c06            : 8;
        FIELD c_lut_cmlm_c05            : 8;
        FIELD c_lut_cmlm_c04            : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_23, *PREG_DVS_ME_23;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_cmlm_c11            : 8;
        FIELD c_lut_cmlm_c10            : 8;
        FIELD c_lut_cmlm_c09            : 8;
        FIELD c_lut_cmlm_c08            : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_24, *PREG_DVS_ME_24;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_cmlm_c15            : 8;
        FIELD c_lut_cmlm_c14            : 8;
        FIELD c_lut_cmlm_c13            : 8;
        FIELD c_lut_cmlm_c12            : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_25, *PREG_DVS_ME_25;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 21;
        FIELD c_lut_cmlm_step           : 3;
        FIELD c_lut_cmlm_c16            : 8;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_26, *PREG_DVS_ME_26;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_conf_c03            : 3;
        FIELD c_lut_conf_c02            : 3;
        FIELD c_lut_conf_c01            : 3;
        FIELD c_lut_conf_c00            : 3;
        FIELD rsv_12                    : 20;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_27, *PREG_DVS_ME_27;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_conf_c07            : 3;
        FIELD c_lut_conf_c06            : 3;
        FIELD c_lut_conf_c05            : 3;
        FIELD c_lut_conf_c04            : 3;
        FIELD rsv_12                    : 20;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_28, *PREG_DVS_ME_28;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_conf_c11            : 3;
        FIELD c_lut_conf_c10            : 3;
        FIELD c_lut_conf_c09            : 3;
        FIELD c_lut_conf_c08            : 3;
        FIELD rsv_12                    : 20;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_29, *PREG_DVS_ME_29;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lut_conf_c15            : 3;
        FIELD c_lut_conf_c14            : 3;
        FIELD c_lut_conf_c13            : 3;
        FIELD c_lut_conf_c12            : 3;
        FIELD rsv_12                    : 20;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_30, *PREG_DVS_ME_30;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_bgcmmn                  : 3;
        FIELD c_bgcpkrn                 : 3;
        FIELD c_bgclrd                  : 3;
        FIELD c_bgcmlm                  : 7;
        FIELD rsv_16                    : 16;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_31, *PREG_DVS_ME_31;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wgtcmmn                 : 3;
        FIELD c_wgtcpkrn                : 3;
        FIELD c_wgtclrd                 : 3;
        FIELD c_wgtcmlm                 : 3;
        FIELD c_divwgt                  : 3;
        FIELD c_cost_validnum_th        : 4;
        FIELD rsv_19                    : 13;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_32, *PREG_DVS_ME_32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lp_box_l_left           : 13;
        FIELD rsv_13                    : 3;
        FIELD c_lp_box_l_right          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_33, *PREG_DVS_ME_33;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lp_box_l_top            : 13;
        FIELD rsv_13                    : 3;
        FIELD c_lp_box_l_bottom         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_34, *PREG_DVS_ME_34;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lp_box_r_left           : 13;
        FIELD rsv_13                    : 3;
        FIELD c_lp_box_r_right          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_35, *PREG_DVS_ME_35;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lp_box_r_top            : 13;
        FIELD rsv_13                    : 3;
        FIELD c_lp_box_r_bottom         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVS_ME_36, *PREG_DVS_ME_36;

// ----------------- DPE_DVS_OCC Bit Field Definitions -------------------
typedef PACKING union
{
    PACKING struct
    {
        FIELD c_lrc_sft_x               : 8;
        FIELD c_lrc_thr_h               : 4;
        FIELD c_lrc_thr_v               : 4;
        FIELD c_owc_en                  : 1;
        FIELD c_owc_tlr_thr             : 6;
        FIELD c_ohf_en                  : 1;
        FIELD c_ohf_tlr_thr             : 7;
        FIELD c_occ_dv_lsb_sft          : 1;
    } Bits;
    unsigned int Raw;
} REG_DVS_OCC_PQ_0, *PREG_DVS_OCC_PQ_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_occ_dv_msb_sft          : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    unsigned int Raw;
} REG_DVS_OCC_PQ_1, *PREG_DVS_OCC_PQ_1;

// ----------------- DPE_DVP_CTRL Bit Field Definitions -------------------
typedef PACKING union
{
    PACKING struct
    {
        FIELD c_asf_thd_y               : 8;
        FIELD c_asf_thd_cbcr            : 8;
        FIELD c_asf_arm_ud              : 4;
        FIELD c_asf_arm_lr              : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_00, *PREG_DVP_CORE_00;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rmv_mean_lut_0          : 8;
        FIELD c_rmv_mean_lut_1          : 8;
        FIELD c_rmv_mean_lut_2          : 8;
        FIELD c_rmv_mean_lut_3          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_01, *PREG_DVP_CORE_01;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rmv_mean_lut_4          : 8;
        FIELD c_rmv_mean_lut_5          : 8;
        FIELD c_rmv_mean_lut_6          : 8;
        FIELD c_rmv_mean_lut_7          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_02, *PREG_DVP_CORE_02;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rmv_dsty_lut_0          : 5;
        FIELD rsv_5                     : 3;
        FIELD c_rmv_dsty_lut_1          : 5;
        FIELD rsv_13                    : 3;
        FIELD c_rmv_dsty_lut_2          : 5;
        FIELD rsv_21                    : 3;
        FIELD c_rmv_dsty_lut_3          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_03, *PREG_DVP_CORE_03;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_rmv_dsty_lut_4          : 5;
        FIELD rsv_5                     : 3;
        FIELD c_rmv_dsty_lut_5          : 5;
        FIELD rsv_13                    : 3;
        FIELD c_rmv_dsty_lut_6          : 5;
        FIELD rsv_21                    : 3;
        FIELD c_rmv_dsty_lut_7          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_04, *PREG_DVP_CORE_04;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_hf_thd_r1               : 6;
        FIELD rsv_6                     : 2;
        FIELD c_hf_thd_r2               : 6;
        FIELD rsv_14                    : 2;
        FIELD c_hf_thd_r3               : 6;
        FIELD rsv_22                    : 2;
        FIELD c_hf_thd_r4               : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_05, *PREG_DVP_CORE_05;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_hf_thd_r5               : 6;
        FIELD rsv_6                     : 2;
        FIELD c_hf_thd_r6               : 6;
        FIELD rsv_14                    : 2;
        FIELD c_hf_thd_r7               : 6;
        FIELD rsv_22                    : 2;
        FIELD c_hf_thd_r8               : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_06, *PREG_DVP_CORE_06;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_hf_thin_en              : 1;
        FIELD rsv_1                     : 3;
        FIELD c_hf_thd_thin             : 2;
        FIELD rsv_6                     : 10;
        FIELD c_hf_thd_r9               : 6;
        FIELD rsv_22                    : 2;
        FIELD c_hf_thd_r10              : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_07, *PREG_DVP_CORE_07;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_0           : 8;
        FIELD c_wmf_wgt_lut_1           : 8;
        FIELD c_wmf_wgt_lut_2           : 8;
        FIELD c_wmf_wgt_lut_3           : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_08, *PREG_DVP_CORE_08;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_4           : 8;
        FIELD c_wmf_wgt_lut_5           : 8;
        FIELD c_wmf_wgt_lut_6           : 8;
        FIELD c_wmf_wgt_lut_7           : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_09, *PREG_DVP_CORE_09;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_8           : 8;
        FIELD c_wmf_wgt_lut_9           : 8;
        FIELD c_wmf_wgt_lut_10          : 8;
        FIELD c_wmf_wgt_lut_11          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_10, *PREG_DVP_CORE_10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_12          : 8;
        FIELD c_wmf_wgt_lut_13          : 8;
        FIELD c_wmf_wgt_lut_14          : 8;
        FIELD c_wmf_wgt_lut_15          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_11, *PREG_DVP_CORE_11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_16          : 8;
        FIELD c_wmf_wgt_lut_17          : 8;
        FIELD c_wmf_wgt_lut_18          : 8;
        FIELD c_wmf_wgt_lut_19          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_12, *PREG_DVP_CORE_12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_20          : 8;
        FIELD c_wmf_wgt_lut_21          : 8;
        FIELD c_wmf_wgt_lut_22          : 8;
        FIELD c_wmf_wgt_lut_23          : 8;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_13, *PREG_DVP_CORE_13;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_wmf_wgt_lut_24          : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_14, *PREG_DVP_CORE_14;

typedef PACKING union
{
    PACKING struct
    {
        FIELD c_hf_mode_r1              : 1;
        FIELD c_hf_mode_r2              : 1;
        FIELD c_hf_mode_r3              : 1;
        FIELD c_hf_mode_r4              : 1;
        FIELD c_hf_mode_r5              : 1;
        FIELD c_hf_mode_r6              : 1;
        FIELD c_hf_mode_r7              : 1;
        FIELD c_hf_mode_r8              : 1;
        FIELD c_hf_mode_r9              : 1;
        FIELD c_hf_mode_r10             : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    unsigned int Raw;
} REG_DVP_CORE_15, *PREG_DVP_CORE_15;

// ----------------- DPE_DVS_ME  Grouping Definitions -------------------
typedef struct
{
    REG_DVS_ME_00                   DVS_ME_00;           //1B100300
    REG_DVS_ME_01                   DVS_ME_01;           //1B100304
    REG_DVS_ME_02                   DVS_ME_02;           //1B100308
    REG_DVS_ME_03                   DVS_ME_03;           //1B10030C
    REG_DVS_ME_04                   DVS_ME_04;           //1B100310
    REG_DVS_ME_05                   DVS_ME_05;           //1B100314
    REG_DVS_ME_06                   DVS_ME_06;           //1B100318
    REG_DVS_ME_07                   DVS_ME_07;           //1B10031C
    REG_DVS_ME_08                   DVS_ME_08;           //1B100320
    REG_DVS_ME_09                   DVS_ME_09;           //1B100324
    REG_DVS_ME_10                   DVS_ME_10;           //1B100328
    REG_DVS_ME_11                   DVS_ME_11;           //1B10032C
    REG_DVS_ME_12                   DVS_ME_12;           //1B100330
    REG_DVS_ME_13                   DVS_ME_13;           //1B100334
    REG_DVS_ME_14                   DVS_ME_14;           //1B100338
    REG_DVS_ME_15                   DVS_ME_15;           //1B10033C
    REG_DVS_ME_16                   DVS_ME_16;           //1B100340
    REG_DVS_ME_17                   DVS_ME_17;           //1B100344
    REG_DVS_ME_18                   DVS_ME_18;           //1B100348
    REG_DVS_ME_19                   DVS_ME_19;           //1B10034C
    REG_DVS_ME_20                   DVS_ME_20;           //1B100350
    REG_DVS_ME_21                   DVS_ME_21;           //1B100354
    REG_DVS_ME_22                   DVS_ME_22;           //1B100358
    REG_DVS_ME_23                   DVS_ME_23;           //1B10035C
    REG_DVS_ME_24                   DVS_ME_24;           //1B100360
    REG_DVS_ME_25                   DVS_ME_25;           //1B100364
    REG_DVS_ME_26                   DVS_ME_26;           //1B100368
    REG_DVS_ME_27                   DVS_ME_27;           //1B10036C
    REG_DVS_ME_28                   DVS_ME_28;           //1B100370
    REG_DVS_ME_29                   DVS_ME_29;           //1B100374
    REG_DVS_ME_30                   DVS_ME_30;           //1B100378
    REG_DVS_ME_31                   DVS_ME_31;           //1B10037C
    REG_DVS_ME_32                   DVS_ME_32;           //1B100380
    REG_DVS_ME_33                   DVS_ME_33;           //1B100384
    REG_DVS_ME_34                   DVS_ME_34;           //1B100388
    REG_DVS_ME_35                   DVS_ME_35;           //1B10038C
    REG_DVS_ME_36                   DVS_ME_36;           //1B100390
}DVS_ME_CFG, *PDVS_ME_CFG;

// ----------------- DPE_DVS_OCC  Grouping Definitions -------------------
typedef struct
{
    REG_DVS_OCC_PQ_0                DVS_OCC_PQ_0;        //1B100030
    REG_DVS_OCC_PQ_1                DVS_OCC_PQ_1;        //1B100034
}DVS_OCC_CFG, *PDVS_OCC_CFG;

// ----------------- DPE_DVP_CTRL  Grouping Definitions -------------------
typedef struct
{
    REG_DVP_CORE_00                 DVP_CORE_00;         //1B100900
    REG_DVP_CORE_01                 DVP_CORE_01;         //1B100904
    REG_DVP_CORE_02                 DVP_CORE_02;         //1B100908
    REG_DVP_CORE_03                 DVP_CORE_03;         //1B10090C
    REG_DVP_CORE_04                 DVP_CORE_04;         //1B100910
    REG_DVP_CORE_05                 DVP_CORE_05;         //1B100914
    REG_DVP_CORE_06                 DVP_CORE_06;         //1B100918
    REG_DVP_CORE_07                 DVP_CORE_07;         //1B10091C
    REG_DVP_CORE_08                 DVP_CORE_08;         //1B100920
    REG_DVP_CORE_09                 DVP_CORE_09;         //1B100924
    REG_DVP_CORE_10                 DVP_CORE_10;         //1B100928
    REG_DVP_CORE_11                 DVP_CORE_11;         //1B10092C
    REG_DVP_CORE_12                 DVP_CORE_12;         //1B100930
    REG_DVP_CORE_13                 DVP_CORE_13;         //1B100934
    REG_DVP_CORE_14                 DVP_CORE_14;         //1B100938
    REG_DVP_CORE_15                 DVP_CORE_15;         //1B10093C
}DVP_CORE_CFG, *PDVP_CORE_CFG;

