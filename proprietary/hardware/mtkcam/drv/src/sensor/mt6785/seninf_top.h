#ifndef __seninf_top_REGS_H__
#define __seninf_top_REGS_H__

// ----------------- seninf_top Bit Field Definitions -------------------

typedef unsigned int FIELD;
typedef unsigned int UINT32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_top_sw_rst         : 1;
        FIELD rsv_1                     : 3;
        FIELD seninf_top_n3d_sw_rst     : 1;
        FIELD rsv_5                     : 3;
        FIELD rg_seninf1_pcam_pclk_sel  : 1;
        FIELD rg_seninf2_pcam_pclk_sel  : 1;
        FIELD rsv_10                    : 2;
        FIELD rg_seninf1_pcam_pclk_en   : 1;
        FIELD rg_seninf2_pcam_pclk_en   : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CTRL, *PREG_SENINF_TOP_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_top_dbg_sel     : 5;
        FIELD rsv_5                     : 11;
        FIELD rg_seninf_top_dbg_byte0_sel : 2;
        FIELD rsv_18                    : 2;
        FIELD rg_seninf_top_dbg_byte1_sel : 2;
        FIELD rsv_22                    : 2;
        FIELD rg_seninf_top_dbg_byte2_sel : 2;
        FIELD rsv_26                    : 2;
        FIELD rg_seninf_top_dbg_byte3_sel : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_DBG_CTRL, *PREG_SENINF_TOP_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux1_src_sel    : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_mux2_src_sel    : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_mux3_src_sel    : 4;
        FIELD rsv_20                    : 4;
        FIELD rg_seninf_mux4_src_sel    : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_MUX_CTRL_0, *PREG_SENINF_TOP_MUX_CTRL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux5_src_sel    : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_mux6_src_sel    : 4;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_mux7_src_sel    : 4;
        FIELD rsv_20                    : 4;
        FIELD rg_seninf_mux8_src_sel    : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_MUX_CTRL_1, *PREG_SENINF_TOP_MUX_CTRL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD phy_seninf_mux0_dphy_en   : 1;
        FIELD phy_seninf_mux0_cphy_en   : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_phy_seninf_mux0_cphy_mode : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_CTRL_CSI0, *PREG_SENINF_TOP_PHY_CTRL_CSI0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD phy_seninf_mux1_dphy_en   : 1;
        FIELD phy_seninf_mux1_cphy_en   : 1;
        FIELD rsv_2                     : 6;
        FIELD rg_phy_seninf_mux1_cphy_mode : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_CTRL_CSI1, *PREG_SENINF_TOP_PHY_CTRL_CSI1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD phy_seninf_mux2_dphy_en   : 1;
        FIELD phy_seninf_mux2_cphy_en   : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_CTRL_CSI2, *PREG_SENINF_TOP_PHY_CTRL_CSI2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_n3d_seninf1_vsync_src_sel_a : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_n3d_seninf2_vsync_src_sel_a : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_N3D_A_CTRL, *PREG_SENINF_TOP_N3D_A_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_n3d_seninf1_vsync_src_sel_b : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_n3d_seninf2_vsync_src_sel_b : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_N3D_B_CTRL, *PREG_SENINF_TOP_N3D_B_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD phy_seninf_mux0_phyd2mac_dphy_chk_en : 1;
        FIELD phy_seninf_mux1_phyd2mac_dphy_chk_en : 1;
        FIELD phy_seninf_mux2_phyd2mac_dphy_chk_en : 1;
        FIELD rsv_3                     : 5;
        FIELD phy_seninf_mux0_phyd2mac_cphy_chk_en : 1;
        FIELD phy_seninf_mux1_phyd2mac_cphy_chk_en : 1;
        FIELD phy_seninf_mux2_phyd2mac_cphy_chk_en : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_CHK_EN, *PREG_SENINF_TOP_PHY_CHK_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_phy_seninf_mux0_phyd2mac_dphy_chk_mode : 1;
        FIELD rg_phy_seninf_mux1_phyd2mac_dphy_chk_mode : 1;
        FIELD rg_phy_seninf_mux2_phyd2mac_dphy_chk_mode : 1;
        FIELD rsv_3                     : 5;
        FIELD rg_phy_seninf_mux0_phyd2mac_cphy_chk_mode : 1;
        FIELD rg_phy_seninf_mux1_phyd2mac_cphy_chk_mode : 1;
        FIELD rg_phy_seninf_mux2_phyd2mac_cphy_chk_mode : 1;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_CHK_MODE, *PREG_SENINF_TOP_PHY_CHK_MODE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l0_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l1_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l2_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l3_chk_done : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l0_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l1_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l2_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_dphy_l3_chk_fail : 1;
        FIELD rsv_12                    : 4;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t0_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t1_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t2_chk_done : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t3_chk_done : 1;
        FIELD rsv_20                    : 4;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t0_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t1_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t2_chk_fail : 1;
        FIELD ro_phy_seninf_mux0_phyd2mac_cphy_t3_chk_fail : 1;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY0_CHK_RES, *PREG_SENINF_TOP_PHY0_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l0_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l1_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l2_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l3_chk_done : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l0_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l1_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l2_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_dphy_l3_chk_fail : 1;
        FIELD rsv_12                    : 4;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t0_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t1_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t2_chk_done : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t3_chk_done : 1;
        FIELD rsv_20                    : 4;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t0_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t1_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t2_chk_fail : 1;
        FIELD ro_phy_seninf_mux1_phyd2mac_cphy_t3_chk_fail : 1;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY1_CHK_RES, *PREG_SENINF_TOP_PHY1_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l0_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l1_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l2_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l3_chk_done : 1;
        FIELD rsv_4                     : 4;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l0_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l1_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l2_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_dphy_l3_chk_fail : 1;
        FIELD rsv_12                    : 4;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t0_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t1_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t2_chk_done : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t3_chk_done : 1;
        FIELD rsv_20                    : 4;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t0_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t1_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t2_chk_fail : 1;
        FIELD ro_phy_seninf_mux2_phyd2mac_cphy_t3_chk_fail : 1;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY2_CHK_RES, *PREG_SENINF_TOP_PHY2_CHK_RES;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_phy_seninf_mux0_dbg_en : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_phy_seninf_mux0_dbg_sel : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX0_DBG_CTRL, *PREG_SENINF_TOP_PHY_SENINF_MUX0_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux0_dbg_out : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX0_DBG_OUT, *PREG_SENINF_TOP_PHY_SENINF_MUX0_DBG_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_phy_seninf_mux1_dbg_en : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_phy_seninf_mux1_dbg_sel : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX1_DBG_CTRL, *PREG_SENINF_TOP_PHY_SENINF_MUX1_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux2_dbg_out : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX1_DBG_OUT, *PREG_SENINF_TOP_PHY_SENINF_MUX1_DBG_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_phy_seninf_mux2_dbg_en : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_phy_seninf_mux2_dbg_sel : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX2_DBG_CTRL, *PREG_SENINF_TOP_PHY_SENINF_MUX2_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_phy_seninf_mux2_dbg_out : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_PHY_SENINF_MUX2_DBG_OUT, *PREG_SENINF_TOP_PHY_SENINF_MUX2_DBG_OUT;

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
} REG_SENINF_TOP_SPARE, *PREG_SENINF_TOP_SPARE;

// ----------------- seninf_top Register Definition -------------------
typedef volatile struct /*0x1A004000*/
{
    REG_SENINF_TOP_CTRL             SENINF_TOP_CTRL;  // 4000
    UINT32                          rsv_4004;         // 4004
    REG_SENINF_TOP_DBG_CTRL         SENINF_TOP_DBG_CTRL; // 4008
    UINT32                          rsv_400C;         // 400C
    REG_SENINF_TOP_MUX_CTRL_0       SENINF_TOP_MUX_CTRL_0; // 4010
    REG_SENINF_TOP_MUX_CTRL_1       SENINF_TOP_MUX_CTRL_1; // 4014
    UINT32                          rsv_4018[18];     // 4018..405C
    REG_SENINF_TOP_PHY_CTRL_CSI0    SENINF_TOP_PHY_CTRL_CSI0; // 4060
    REG_SENINF_TOP_PHY_CTRL_CSI1    SENINF_TOP_PHY_CTRL_CSI1; // 4064
    REG_SENINF_TOP_PHY_CTRL_CSI2    SENINF_TOP_PHY_CTRL_CSI2; // 4068
    UINT32                          rsv_406C[5];      // 406C..407C
    REG_SENINF_TOP_N3D_A_CTRL       SENINF_TOP_N3D_A_CTRL; // 4080
    REG_SENINF_TOP_N3D_B_CTRL       SENINF_TOP_N3D_B_CTRL; // 4084
    UINT32                          rsv_4088[6];      // 4088..409C
    REG_SENINF_TOP_PHY_CHK_EN       SENINF_TOP_PHY_CHK_EN; // 40A0
    REG_SENINF_TOP_PHY_CHK_MODE     SENINF_TOP_PHY_CHK_MODE; // 40A4
    UINT32                          rsv_40A8[2];      // 40A8..40AC
    REG_SENINF_TOP_PHY0_CHK_RES     SENINF_TOP_PHY0_CHK_RES; // 40B0
    REG_SENINF_TOP_PHY1_CHK_RES     SENINF_TOP_PHY1_CHK_RES; // 40B4
    REG_SENINF_TOP_PHY2_CHK_RES     SENINF_TOP_PHY2_CHK_RES; // 40B8
    UINT32                          rsv_40BC;         // 40BC
    REG_SENINF_TOP_PHY_SENINF_MUX0_DBG_CTRL SENINF_TOP_PHY_SENINF_MUX0_DBG_CTRL; // 40C0
    REG_SENINF_TOP_PHY_SENINF_MUX0_DBG_OUT SENINF_TOP_PHY_SENINF_MUX0_DBG_OUT; // 40C4
    REG_SENINF_TOP_PHY_SENINF_MUX1_DBG_CTRL SENINF_TOP_PHY_SENINF_MUX1_DBG_CTRL; // 40C8
    REG_SENINF_TOP_PHY_SENINF_MUX1_DBG_OUT SENINF_TOP_PHY_SENINF_MUX1_DBG_OUT; // 40CC
    REG_SENINF_TOP_PHY_SENINF_MUX2_DBG_CTRL SENINF_TOP_PHY_SENINF_MUX2_DBG_CTRL; // 40D0
    REG_SENINF_TOP_PHY_SENINF_MUX2_DBG_OUT SENINF_TOP_PHY_SENINF_MUX2_DBG_OUT; // 40D4
    UINT32                          rsv_40D8[8];      // 40D8..40F4
    REG_SENINF_TOP_SPARE            SENINF_TOP_SPARE; // 40F8
}seninf_top_REGS, *Pseninf_top_REGS;

#endif // __seninf_top_REGS_H__
