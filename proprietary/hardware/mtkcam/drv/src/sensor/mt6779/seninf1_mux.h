#ifndef __seninf1_mux_REGS_H__
#define __seninf1_mux_REGS_H__

typedef unsigned int FIELD;
typedef unsigned int UINT32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_mux_en             : 1;
        FIELD seninf_mux_irq_sw_rst     : 1;
        FIELD seninf_mux_sw_rst         : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_CTRL_0, *PREG_SENINF_MUX_CTRL_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_src_sel     : 4;
        FIELD rsv_4                     : 4;
        FIELD rg_seninf_mux_pix_mode_sel : 2;
        FIELD rsv_10                    : 6;
        FIELD rg_seninf_mux_fifo_push_en : 6;
        FIELD rsv_22                    : 2;
        FIELD rg_seninf_mux_rdy_force_mode_en : 1;
        FIELD rg_seninf_mux_rdy_force_mode_val : 1;
        FIELD rsv_26                    : 2;
        FIELD rg_seninf_mux_crop_en     : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_CTRL_1, *PREG_SENINF_MUX_CTRL_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_cnt_init_opt : 2;
        FIELD rsv_2                     : 6;
        FIELD rg_seninf_mux_fifo_full_output_opt : 2;
        FIELD rg_seninf_mux_fifo_full_wr_mode : 2;
        FIELD rg_seninf_mux_fifo_overrun_rst_en : 1;
        FIELD rsv_13                    : 3;
        FIELD rg_seninf_mux_hsync_pol   : 1;
        FIELD rg_seninf_mux_vsync_pol   : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_OPT, *PREG_SENINF_MUX_OPT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_fifo_overrun_irq_en : 1;
        FIELD rg_seninf_mux_fsm_err_irq_en : 1;
        FIELD rg_seninf_mux_hsize_err_irq_en : 1;
        FIELD rg_seninf_mux_vsize_err_irq_en : 1;
        FIELD rsv_4                     : 27;
        FIELD rg_seninf_mux_irq_clr_mode : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_IRQ_EN, *PREG_SENINF_MUX_IRQ_EN;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_fifo_overrun_irq : 1;
        FIELD ro_seninf_mux_fsm_err_irq : 1;
        FIELD ro_seninf_mux_hsize_err_irq : 1;
        FIELD ro_seninf_mux_vsize_err_irq : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_IRQ_STATUS, *PREG_SENINF_MUX_IRQ_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_expect_hsize : 16;
        FIELD rg_seninf_mux_expect_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_IMG_SIZE, *PREG_SENINF_MUX_IMG_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_crop_start_8pix_cnt : 12;
        FIELD rsv_12                    : 4;
        FIELD rg_seninf_mux_crop_end_8pix_cnt : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_CROP_PIX_CTRL, *PREG_SENINF_MUX_CROP_PIX_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_rcv_hsize   : 16;
        FIELD ro_seninf_mux_rcv_vsize   : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_SIZE, *PREG_SENINF_MUX_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_rcv_err_hsize : 16;
        FIELD ro_seninf_mux_rcv_err_vsize : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_ERR_SIZE, *PREG_SENINF_MUX_ERR_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_fifo_wa     : 9;
        FIELD rsv_9                     : 3;
        FIELD ro_seninf_mux_fifo_wcs    : 1;
        FIELD rsv_13                    : 3;
        FIELD ro_seninf_mux_fifo_ra     : 9;
        FIELD rsv_25                    : 3;
        FIELD ro_seninf_mux_fifo_rcs    : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FIFO_STATUS, *PREG_SENINF_MUX_FIFO_STATUS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_dbg_en      : 1;
        FIELD rsv_1                     : 7;
        FIELD rg_seninf_mux_dbg_sel     : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_DBG_CTRL, *PREG_SENINF_MUX_DBG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_dbg_out     : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_DBG_OUT, *PREG_SENINF_MUX_DBG_OUT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_cam_mon_0   : 16;
        FIELD ro_seninf_mux_cam_mon_1   : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_CAM_MON, *PREG_SENINF_MUX_CAM_MON;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_pix_cnt     : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_PIX_CNT, *PREG_SENINF_MUX_PIX_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_frame_size_mon_en : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FRAME_SIZE_MON_CTRL, *PREG_SENINF_MUX_FRAME_SIZE_MON_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_frame_h_valid : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FRAME_SIZE_MON_H_VALID, *PREG_SENINF_MUX_FRAME_SIZE_MON_H_VALID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_frame_h_blank : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FRAME_SIZE_MON_H_BLANK, *PREG_SENINF_MUX_FRAME_SIZE_MON_H_BLANK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_frame_v_valid : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FRAME_SIZE_MON_V_VALID, *PREG_SENINF_MUX_FRAME_SIZE_MON_V_VALID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ro_seninf_mux_frame_v_blank : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_FRAME_SIZE_MON_V_BLANK, *PREG_SENINF_MUX_FRAME_SIZE_MON_V_BLANK;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_mux_spare_0     : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_seninf_mux_spare_1     : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF_MUX_SPARE, *PREG_SENINF_MUX_SPARE;

// ----------------- seninf1_mux Register Definition -------------------
typedef volatile struct /*0x1A004D00*/
{
    REG_SENINF_MUX_CTRL_0           SENINF_MUX_CTRL_0; // 4D00
    REG_SENINF_MUX_CTRL_1           SENINF_MUX_CTRL_1; // 4D04
    REG_SENINF_MUX_OPT              SENINF_MUX_OPT;   // 4D08
    UINT32                          rsv_4D0C;         // 4D0C
    REG_SENINF_MUX_IRQ_EN           SENINF_MUX_IRQ_EN; // 4D10
    UINT32                          rsv_4D14;         // 4D14
    REG_SENINF_MUX_IRQ_STATUS       SENINF_MUX_IRQ_STATUS; // 4D18
    UINT32                          rsv_4D1C;         // 4D1C
    REG_SENINF_MUX_IMG_SIZE         SENINF_MUX_IMG_SIZE; // 4D20
    UINT32                          rsv_4D24;         // 4D24
    REG_SENINF_MUX_CROP_PIX_CTRL    SENINF_MUX_CROP_PIX_CTRL; // 4D28
    UINT32                          rsv_4D2C;         // 4D2C
    REG_SENINF_MUX_SIZE             SENINF_MUX_SIZE;  // 4D30
    REG_SENINF_MUX_ERR_SIZE         SENINF_MUX_ERR_SIZE; // 4D34
    UINT32                          rsv_4D38[2];      // 4D38..4D3C
    REG_SENINF_MUX_FIFO_STATUS      SENINF_MUX_FIFO_STATUS; // 4D40
    UINT32                          rsv_4D44[15];     // 4D44..4D7C
    REG_SENINF_MUX_DBG_CTRL         SENINF_MUX_DBG_CTRL; // 4D80
    UINT32                          rsv_4D84;         // 4D84
    REG_SENINF_MUX_DBG_OUT          SENINF_MUX_DBG_OUT; // 4D88
    UINT32                          rsv_4D8C[5];      // 4D8C..4D9C
    REG_SENINF_MUX_CAM_MON          SENINF_MUX_CAM_MON; // 4DA0
    REG_SENINF_MUX_PIX_CNT          SENINF_MUX_PIX_CNT; // 4DA4
    REG_SENINF_MUX_FRAME_SIZE_MON_CTRL SENINF_MUX_FRAME_SIZE_MON_CTRL; // 4DA8
    UINT32                          rsv_4DAC;         // 4DAC
    REG_SENINF_MUX_FRAME_SIZE_MON_H_VALID SENINF_MUX_FRAME_SIZE_MON_H_VALID; // 4DB0
    REG_SENINF_MUX_FRAME_SIZE_MON_H_BLANK SENINF_MUX_FRAME_SIZE_MON_H_BLANK; // 4DB4
    REG_SENINF_MUX_FRAME_SIZE_MON_V_VALID SENINF_MUX_FRAME_SIZE_MON_V_VALID; // 4DB8
    REG_SENINF_MUX_FRAME_SIZE_MON_V_BLANK SENINF_MUX_FRAME_SIZE_MON_V_BLANK; // 4DBC
    UINT32                          rsv_4DC0[12];     // 4DC0..4DEC
    REG_SENINF_MUX_SPARE            SENINF_MUX_SPARE; // 4DF0
}seninf1_mux_REGS, *Pseninf1_mux_REGS;

#endif // __seninf1_mux_REGS_H__
