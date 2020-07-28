#include "val_types_private.h"

#ifndef _VDEC_REG_H_
#define _VDEC_REG_H_

#define VDEC_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.u.bits.FieldName)
#define VDEC_REG(RegBase, RegName) (RegBase->RegName.u.reg)

#define _VDEC_HW_BASE       u4VDEC_HW_BASE      // MT6582, MT8135: 0x16000000
#define _VDEC_SYS           u4VDEC_SYS          // MT6582, MT8135: 0x16000000
#define _VDEC_MISC          u4VDEC_MISC         // MT6582, MT8135: 0x16020000
#define _VDEC_VLD           u4VDEC_VLD          // MT6582, MT8135: 0x16021000
#define _VDEC_VLD_TOP       u4VDEC_VLD_TOP      // MT6582, MT8135: 0x16021800
#define _VDEC_MC            u4VDEC_MC           // MT6582, MT8135: 0x16022000
#define _VDEC_AVC_VLD       u4VDEC_AVC_VLD      // MT6582, MT8135: 0x16023000
#define _VDEC_AVC_MV        u4VDEC_AVC_MV       // MT6582, MT8135: 0x16024000
#define _VDEC_PP            u4VDEC_PP           // MT6582, MT8135: 0x16025000
#define _VDEC_SQT           u4VDEC_SQT          // MT6582, MT8135: 0x16025000
#define _VDEC_VP8_VLD       u4VDEC_VP8_VLD      // MT6582, MT8135: 0x16026800
#define _VDEC_VP6_VLD       u4VDEC_VP6_VLD      // MT6582, MT8135: 0x16027000
#define _VDEC_VP8_VLD2      u4VDEC_VP8_VLD2     // MT6582, MT8135: 0x16027800
#define _VDEC_BS2           u4VDEC_BS2          // 0x16027800
#define _VDEC_VP9_VLD       u4VDEC_VP9_VLD      // 0x16028400
#define _VDEC_UFO           u4VDEC_UFO          // 0x16024F00

#define _VDEC_VLD_TOP_MM_OFFSET     u4VDEC_VLD_TOP_MM_OFFSET        // MT6582, MT8135: 0x00000800
#define _VDEC_VP8_VLD_MM_OFFSET     u4VDEC_VP8_VLD_MM_OFFSET        // MT6582, MT8135: 0x00000800
#define _VDEC_VP8_VLD2_MM_OFFSET    u4VDEC_VP8_VLD2_MM_OFFSET       // MT6582, MT8135: 0x00000800
#define _VDEC_BS2_MM_OFFSET         u4VDEC_BS2_MM_OFFSET            // 0x00000800
#define _VDEC_VP9_VLD_MM_OFFSET     u4VDEC_VP9_VLD_MM_OFFSET        // 0x00000400
#define _VDEC_UFO_MM_OFFSET         u4VDEC_UFO_MM_OFFSET            // 0x00000F00

#define _VDEC_SYS_RANGE         u4VDEC_SYS_RANGE        // MT6582, MT8135: 0x0100
#define _VDEC_MISC_RANGE        u4VDEC_MISC_RANGE       // MT6582, MT8135: 0x0200
#define _VDEC_VLD_RANGE         u4VDEC_VLD_RANGE        // MT6582, MT8135: 0x0400
#define _VDEC_VLD_TOP_RANGE     u4VDEC_VLD_TOP_RANGE    // MT6582, MT8135: 0x1000
#define _VDEC_MC_RANGE          u4VDEC_MC_RANGE         // MT6582, MT8135: 0x1000
#define _VDEC_AVC_VLD_RANGE     u4VDEC_AVC_VLD_RANGE    // MT6582, MT8135: 0x1000
#define _VDEC_AVC_MV_RANGE      u4VDEC_AVC_MV_RANGE     // MT6582, MT8135: 0x0300
#define _VDEC_PP_RANGE          u4VDEC_PP_RANGE         // MT6582, MT8135: 0x1000
#define _VDEC_SQT_RANGE         u4VDEC_SQT_RANGE        // MT6582, MT8135: 0x1000
#define _VDEC_VP8_VLD_RANGE     u4VDEC_VP8_VLD_RANGE    // MT6582, MT8135: 0x1000
#define _VDEC_VP6_VLD_RANGE     u4VDEC_VP6_VLD_RANGE    // MT6582, MT8135: 0x1000
#define _VDEC_VP8_VLD2_RANGE    u4VDEC_VP8_VLD2_RANGE   // MT6582, MT8135: 0x1000
#define _VDEC_BS2_RANGE         u4VDEC_BS2_RANGE        // 0x1000
#define _VDEC_VP9_VLD_RANGE     u4VDEC_VP9_VLD_RANGE    // 0x1000
#define _VDEC_UFO_RANGE         u4VDEC_UFO_RANGE        // 0x1000

// VDEC SYS REGISTER
#define VDEC_CKEN_SET       0x0000
#define VDEC_CKEN_CLR       0x0004
#define LARB_CKEN_SET       0x0008
#define LARB_CHEN_CLR       0x000C
#define VDEC_RESETB         0x0010
#define LARB_RESTB          0x0014
#define VDEC_DCM_CON        0x0018
#define VDEC_SRAM_CFG       0x001C
#define VDEC_ROM_CFG        0x0020
#define VDEC_MBIST_MODE     0x0028
#define VDEC_MBIST_RESETB   0x002C
#define VDEC_MBIST_DEBUG    0x0030
#define VDEC_MBIST_HOLDB    0x0034
#define VDEC_MBIST_BACKGROU 0x0038
#define VDEC_MBIST_DONE     0x003C
#define VDEC_MBIST_FAIL_0   0x0040
#define VDEC_MBIST_FAIL_1   0x0044
#define VDEC_MBIST_BSEL_1   0x0054
#define VDEC_MBIST_BSEL_2   0x0058
#define VDEC_MBIST_BSEL_3   0x005C
#define VDEC_MBIST_BSEL_4   0x0060
#define VDEC_MBIST_BSEL_5   0x0064
#define VDEC_MBIST_BSEL_6   0x0068
#define VDEC_MBIST_BSEL_7   0x006C
#define VDEC_MBIST_BSEL_12  0x0070
#define VDEC_MBIST_BSEL_13  0x0074
#define VDEC_MBIST_BSEL_14  0x0078
#define VDEC_MBIST_BSEL_15  0x007C
#define VDEC_MBIST_BSEL_16  0x0080
#define VDEC_MBIST_BSEL_17  0x0084
#define VDEC_MBIST_BSEL_18  0x0088
#define VDEC_MBIST_BSEL_19  0x008C
#define VDEC_MBIST_BSEL_23  0x0090
#define MBIST_SCAN_OUT_SEL  0x00F0
#define MBIST_FAIL_SEL      0x00F4
/*
typedef volatile struct _vdec_sys_ {
    vdec_cken_set           VDEC_CKEN_SET;
    vdec_cken_clr           VDEC_CKEN_CLR;
    larb_cken_set           LARB_CKEN_SET;
    larb_cken_clr           LARB_CHEN_CLR;
    vdec_resetb             VDEC_RESETB;
    larb_restb              LARB_RESTB;
    vdec_dcm_con            VDEC_DCM_CON;
    vdec_sram_cfg           VDEC_SRAM_CFG;
    vdec_rom_cfg            VDEC_ROM_CFG;
    VAL_UINT32              RESERVED0024;
    vdec_mbist_mode         VDEC_MBIST_MODE; // 0x0028
    vdec_mbist_resetb       VDEC_MBIST_RESETB;
    vdec_mbist_debug        VDEC_MBIST_DEBUG;
    vdec_mbist_holdb        VDEC_MBIST_HOLDB;
    vdec_mbist_background   VDEC_MBIST_BACKGROUND;
    vdec_mbist_done         VDEC_MBIST_DONE;
    vdec_mbist_fail_0       VDEC_MBIST_FAIL_0;
    vdec_mbist_fail_1       VDEC_MBIST_FAIL_1;
    VAL_UINT32              RESERVED0048[3];
    vdec_mbist_bsel_1       VDEC_MBIST_BSEL_1; // 0x0054
    vdec_mbist_bsel_2       VDEC_MBIST_BSEL_2;
    vdec_mbist_bsel_3       VDEC_MBIST_BSEL_3;
    vdec_mbist_bsel_4       VDEC_MBIST_BSEL_4;
    vdec_mbist_bsel_5       VDEC_MBIST_BSEL_5;
    vdec_mbist_bsel_6       VDEC_MBIST_BSEL_6;
    vdec_mbist_bsel_7       VDEC_MBIST_BSEL_7;
    vdec_mbist_bsel_12      VDEC_MBIST_BSEL_12;
    vdec_mbist_bsel_13      VDEC_MBIST_BSEL_13;
    vdec_mbist_bsel_14      VDEC_MBIST_BSEL_14;
    vdec_mbist_bsel_15      VDEC_MBIST_BSEL_15;
    vdec_mbist_bsel_16      VDEC_MBIST_BSEL_16;
    vdec_mbist_bsel_17      VDEC_MBIST_BSEL_17;
    vdec_mbist_bsel_18      VDEC_MBIST_BSEL_18;
    vdec_mbist_bsel_19      VDEC_MBIST_BSEL_19;
    vdec_mbist_bsel_23      VDEC_MBIST_BSEL_23;
    VAL_UINT32              RESERVED0094[23];
    mbist_scan_out_sel      MBIST_SCAN_OUT_SEL; // 0x00F0
    mbist_fail_sel          MBIST_FAIL_SEL;
} _vdec_sys_t;
*/

// VDEC MISC REGISTER
#define CRC_CON             0x0004
#define CRC_YDATA0          0x0008
#define CRC_YDATA1          0x000C
#define CRC_YDATA2          0x0010
#define CRC_YDATA3          0x0014
#define CRC_CDATA0          0x0018
#define CRC_CDATA1          0x001C
#define CRC_CDATA2          0x0020
#define CRC_CDATA3          0x0024
#define MEMERR_CTRL         0x0028
#define VDMERR_STADDR0      0x002C
#define VDMERR_ENDADDR0     0x0030
#define VDMERR_STADDR1      0x0034
#define VDMERR_ENDADDR1     0x0038
#define VDMERR_STADDR2      0x003C
#define VDMERR_ENDADDR2     0x0040
#define VDMERR_STADDR3      0x0044
#define VDMERR_ENDADDR3     0x0048
#define VDMERR_STADDR4      0x004C
#define VDMERR_ENDADDR4     0x0050
#define VDMERR_STADDR5      0x0054
#define VDMERR_ENDADDR5     0x0058
#define VDMERR_STADDR6      0x005C
#define VDMERR_ENDADDR6     0x0060
#define MCMERR_STADDR       0x0064
#define MEMERR_ENDADDR      0x0068
#define MEMERR_STATUS       0x006C
#define SRAMCSON            0x0070
#define VDLRBMONSEL         0x0074
#define VDLRBMONOUT         0x0078
#define MCLRBMONSEL         0x007C
#define MCLRBMONOUT         0x0080
#define SYS_SEL             0x0084
#define VDEC_INT_CFG        0x00A4
#define SPEC_CFG            0x00C8
#define FUNCTION_CFG        0x00CC
#define VDEC_LAT_BUF_CFG    0x00F0
#define VDEC_BREAK_CMD      0x0100
#define VDEC_BREAK_ST       0x0104
#define VDEC_DRAM_PRI       0x0160
#define FUNCTION_CFG2       0x0178

/*
typedef volatile struct _vdec_misc_ {
    VAL_UINT32          RESERVED0000;
    crc_con             ;
    crc_ydata0          ;
    crc_ydata1          ;
    crc_ydata2          ;
    crc_ydata3          ;
    crc_cdata0          ;
    crc_cdata1          ;
    crc_cdata2          ;
    crc_cdata3          ;
    memerr_ctrl         ;
    vdmerr_staddr0      ;
    vdmerr_endaddr0     ;
    vdmerr_staddr1      ;
    vdmerr_endaddr1     ;
    vdmerr_staddr2      ;
    vdmerr_endaddr2     ;
    vdmerr_staddr3      ;
    vdmerr_endaddr3     ;
    vdmerr_staddr4      ;
    vdmerr_endaddr4     ;
    vdmerr_staddr5      ;
    vdmerr_endaddr5     ;
    vdmerr_staddr6      ;
    vdmerr_endaddr6     ;
    mcmerr_staddr       ;
    memerr_endaddr      ;
    memerr_status       ;
    sramcson            ;
    vdlrbmonsel         ;
    vdlrbmonout         ;
    mclrbmonsel         ;
    mclrbmonout         ;
    sys_sel             ;
    VAL_UINT32          RESERVED0088[7];
    vdec_int_cfg        ; // 0x00A4
    VAL_UINT32          RESERVED00A8[8];
    spec_cfg            ; // 0x00C8
    function_cfg        ;
    VAL_UINT32          RESERVED00D0[12];
    vdec_lat_buf_cfg    ; // 0x0100
    vdec_break_cmd      ;
    vdec_break_st       ;
    vdec_dram_pri       ;
} _vdec_misc__t;
*/

// VDEC VLD REGISTER
#define FW_BS               0x0000
#define VDMR                0x0084
#define PLPR                0x0088
#define PIC_PARAM           0x0088
#define RCPR                0x008C
#define PIC_PROC            0x008C
#define VSSR                0x0090
#define MBRP                0x0094
#define MB_ROW_PARAM        0x0094
#define MBHP                0x0098
#define MB_HORI_ADD         0x0098
#define NEW_MP2_VLD_MD      0x009C
#define LIQM                0x00A0
#define LNIQM               0x00A4
#define DIGMBYOFF           0x00A8
#define VPMN                0x00AC
#define RPAR                0x00B0
#define VSTA                0x00B4
#define VEND                0x00B8
#define PFPR                0x00BC
#define SSCS                0x00C0
#define LPCR                0x00C4
#define STER                0x00C8
#define IDLE_MAX            0x00CC
#define VB_SRAM_CFG         0x00D0
#define VMIR                0x00D4
#define CMSR                0x00D8
#define MIBT                0x00DC
#define FSBI                0x00E0
#define EC_SR               0x00E4
#define FFSR                0x00E8
#define VBST                0x00EC
#define VBIW                0x00F0
#define VBCS                0x00F4
#define VWPT                0x00F8
#define VRPT                0x00FC
#define VBPF                0x0104
#define SWRST               0x0108
#define SOFT_RESET          0x0108
#define ERCR                0x010C
#define RCRW                0x0110
#define DBSR                0x0114
#define INI_INPUT_WINDOW    0x0130
#define FLUSH_VB            0x0144
#define BS_THLD_DET         0x014C
#define BS_THLD_DET_ST      0x0150
#define M4HP                0x01C4
#define M4FLAG              0x01C8
#define LDSR                0x01CC
#define LDSH                0x01D0
#define MP4_LOAD_SUM        0x01D0
#define M4QP                0x01D4
#define M4SH                0x01D8
#define R311                0x01DC
#define M4DGR               0x01E0
#define M4DEC               0x01E4
#define M4DGR1              0x01E8
#define M4DGR2              0x01EC
#define M4DGR3              0x01F0
#define M4DGR4              0x01F4
#define M4DGR5              0x01F8
#define M4DGR6              0x01FC
#define M4DGR7              0x0200
#define WMVDEC              0x0208
#define WMV_MODE            0x020C
#define WMV_PARA1           0x0210
#define WMV_PARA2           0x0214
#define WMVEC               0x0218
#define WMV_LOAD_SUM        0x021C
#define WMV_PARA3           0x0220
#define WMVBPTYPE           0x0224
#define WMVMM               0x0228
#define WMVXIS              0x022C
#define WMVQM               0x0230
#define WMVSTATUS           0x0234
#define WMVTAB              0x0238
#define WMVABS              0x023C
#define WMVPUD              0x0244
#define WMVPBN              0x0248
#define WMVSTAT             0x024C
#define WMVEC_CNT           0x0250
#define WMVDMY              0x0254
#define WMVPW               0x0258
#define EMB_WDBT            0x025C
#define RCRQ                0x0260
#define QMAT                0x0264
#define DTOT_WERCR          0x0268
#define MIDQ                0x026C
#define ESR                 0x0270
#define ISR1_WSI1           0x0274
#define ISR2_WSI2           0x0278
#define WSI3                0x027C
#define MV1X_WEDT           0x0280
#define MV2X_WMVES          0x0284
#define MV3X_WMVET          0x0288
#define MV4X_NWMV           0x028C
#define MV1Y                0x0290
#define MV2Y                0x0294
#define MV3Y                0x0298
#define MV4Y                0x029C
#define MVTN1               0x02A0
#define MVTN2               0x02A4
#define NOOP                0x02A8
#define NOOB                0x02AC
#define VLDOKCLR            0x02B0
#define MP4DPDS             0x02BC
#define DVX311_BOUND_MODE   0x02C0
#define DVX311_BOUND_R1     0x02C4
#define DVX311_BOUND_R2     0x02C8
#define DVX311_BOUND_R3     0x02CC
#define MP4NCDBS            0x02D4
#define MP4DPDBS            0x02D8
#define MP4DS               0x02DC
#define MP4ERRDIS0          0x02F4
#define MP4ERRDIS1          0x02F8
#define VOP_TYPE            0X0350
#define MP4ERRTYPE0         0x03C4
#define MP4ERRTYPE1         0x03C8
#define MP4ERRCHK           0x03CC
#define MP4ERRTYPE2         0x03D0
#define MP4ERRTYPE3         0x03D4
#define CHKSUM_BITS_OUT     0x03E4
#define CHKSUM_INPUT_WINDOW 0x03E8
#define CHKSUM_VLD_OUT      0x03EC
#define SEPR                0x03F0
#define SQSC                0x03F4

/*
typedef volatile struct _vdec_vld_ {
    fw_bs           ;
    vdmr            ;
    plpr            ;
    rcpr            ;
    vssr            ;
    mbrp            ;
    mbhp            ;
    new_mp2_vld_md  ;
    liqm            ;
    lniqm           ;
    vpmn            ;
    rpar            ;
    vsta            ;
    vend            ;
    pfpr            ;
    sscs            ;
    lpcr            ;
    ster            ;
    idle_max        ;
    vb_sram_cfg     ;
    vmir            ;
    cmsr            ;
    mibt            ;
    fsbi            ;
    esr             ;
    ffsr            ;
    vbst            ;
    vbiw            ;
    vbcs            ;
    vwpt            ;
    vrpt            ;
    vbpf            ;
    swrst           ;
    ercr            ;

} _vdec_vld_t;
*/

// VDEC VLD_TOP REGISTER
#define VLD_RDY_CFG         0x0000
#define VLD_TIMEOUT_ST_CLR  0x0004
#define VLD_DEC_FIN_ST_CLR  0x0008
#define PRED_DRAM_B_ADDR    0x0028
#define PRED_DRAM_ADDR      0x0028
#define PRED_SRAM_CFG       0x002C
#define RNG_ST_ADDR         0x0030
#define RNG_END_ADDR        0x0034
#define PF_BS_CFG           0x0038
#define SEG_ID_B_ADDR       0x003C
#define PRED_MODE           0x0040
#define WEBP                0x0048
#define MBROWTRG            0x004C
#define VDEC_TO_THRD        0x0050
#define VDEC_TO_INT_SW      0x0054
#define MC_BUSY_THRD        0x0058
#define ERR_CONCEAL         0x005C
#define DEDERR_TYPE         0x0060
#define ERR_TYPE_EN         0x0064
#define PIC_MB_SIZE_M1      0x0068
#define PIC_TOTAL_MBS       0x006C
#define PIC_PIX_SIZE        0x0070
#define VP8_ERR_SWITCH      0x0074
#define LOW_POWER           0x0078
#define VP8_CBP_ENHANCE     0x007C
#define MC_ABORT            0x0084
#define MV_ERR_CONCEAL      0x0088
#define READY_DELAY         0x008C
#define VDEC_TO_CNT         0x00A0
#define VLD_CHK_SUM         0x00A4
#define WEBP_MON            0x00B4

// VDEC MC REGISTER
#define R1Y_ADD                 0x0000
#define R1C_ADD                 0x0004
#define R2Y_ADD                 0x0008
#define R2C_ADD                 0x000C
#define BY_ADD                  0x0010
#define BC_ADD                  0x0014
#define FW_PIC                  0x0018
#define FORWARD_REF_PIC         0x0018
#define DE_FLD                  0x001C
#define SE_FLD                  0x0020
#define OUT_BF                  0x0024
#define OUTPUT_BUFFER           0x0024
#define RF_FLD                  0x0060
#define ADD_SWAP                0x0090
#define PB1Y_ADD                0x009C
#define PB1C_ADD                0x00A0
#define PB2Y_ADD                0x00A4
#define PB2C_ADD                0x00A8
#define Q_TYPE                  0x01FC
#define QUARTER_PEL_TYPE        0x01FC
#define Q_SAMPLE                0x0200
#define QUARTER_SAMPLE          0x0200
#define ROUNDCTL                0x0204
#define ROUND_CTL               0x0204
#define UMV_WIDTH               0x0208
#define UMV_HEIGHT              0x020C
#define WARP_POINT_NUM          0x0210
#define GMC_YMV                 0x0214
#define GMC_CMV                 0x0218
#define QP_CMV_CAL              0x021C
#define POSTEN                  0x0220
#define POST_PROCESS_EN         0x0220
#define PY_ADD                  0x0224
#define PP_Y_BUFFER             0x0224
#define PC_ADD                  0x0228
#define PP_C_BUFFER             0x0228
#define POST_WD                 0x022C
#define POST_MB_WIDTH           0x022C
#define POST_HEIGHT             0x0230
#define QP_CTL                  0x0234
#define DBLK_CTL                0x0238
#define DEBLOCK_CTL             0x0238
#define DEBLOKK_THR             0x023C
#define WPOST                   0x0250
#define WRITE_BY_POST           0x0250
#define HMBRNG                  0x0260
#define PP_H_MB_RANGE           0x0260
#define VMBRNG                  0x0264
#define PP_V_MB_RANGE           0x0264
#define WMV9_PRE_PROC           0x0270
#define WMV9_ICOMP_EN           0x0274
#define ISCALE1_X1              0x0278
#define ISCALE1_X3              0x027C
#define ISCALE1_X5              0x0280
#define ISCALE1_X7              0x0284
#define ISCALE1_X9              0x0288
#define ISCALE1_X11             0x028C
#define ISCALE1_X13             0x0290
#define ISCALE1_X15             0x0294
#define YSHIFT_OFF1             0x0298
#define CSHIFT_OFF1             0x029C
#define FILTER_TYPE             0x02A0
#define WRITE_BUS_TYPE          0x02A4
#define INTRA_BLK_ADD128        0x02A8
#define FAST_UVMC               0x02B4
#define OVERLAY_EN              0x02BC
#define CND_OVER                0x02C0
#define DEBLOCK_OPT             0x02C4
#define DBLK_YU                 0x02C8
#define DBLK_CU                 0x02CC
#define WMV8_MIXED_MV           0x02D0
#define ISCALE2_X1              0x02EC
#define ISCALE2_X3              0x02F0
#define ISCALE2_X5              0x02F4
#define ISCALE2_X7              0x02F8
#define ISCALE2_X9              0x02FC
#define ISCALE2_X11             0x0300
#define ISCALE2_X13             0x0304
#define ISCALE2_X15             0x0308
#define YSHIFT_OFF2             0x030C
#define CSHIFT_OFF2             0x0310
#define ICOMP_TYPE              0x0314
#define ISCALE3_X1              0x0318
#define ISCALE3_X3              0x031C
#define ISCALE3_X5              0x0320
#define ISCALE3_X7              0x0324
#define ISCALE3_X9              0x0328
#define ISCALE3_X11             0x032C
#define ISCALE3_X13             0x0330
#define ISCALE3_X15             0x0334
#define YSHIFT_OFF3             0x0338
#define CSHIFT_OFF3             0x033C
#define ICOMP2_EN               0x0340
#define ISCALE4_X1              0x0344
#define ISCALE4_X3              0x0348
#define ISCALE4_X5              0x034C
#define ISCALE4_X7              0x0350
#define ISCALE4_X9              0x0354
#define ISCALE4_X11             0x0358
#define ISCALE4_X13             0x035C
#define ISCALE4_X15             0x0360
#define YSHIFT_OFF4             0x0364
#define CSHIFT_OFF4             0x0368
#define ISCALE5_X1              0x036C
#define ISCALE5_X3              0x0370
#define ISCALE5_X5              0x0374
#define ISCALE5_X7              0x0378
#define ISCALE5_X9              0x037C
#define ISCALE5_X11             0x0380
#define ISCALE5_X13             0x0384
#define ISCALE5_X15             0x0388
#define YSHIFT_OFF5             0x038C
#define CSHIFT_OFF5             0x0390
#define BWD_ICOMP_FLD           0x0394
#define SAME_ICOMP              0x0398
#define REF_PIC_TYPE            0x039C
#define ISCALE6_X1              0x03A0
#define ISCALE6_X3              0x03A4
#define ISCALE6_X5              0x03A8
#define ISCALE6_X7              0x03AC
#define ISCALE6_X9              0x03B0
#define ISCALE6_X11             0x03B4
#define ISCALE6_X13             0x03B8
#define ISCALE6_X15             0x03BC
#define YSHIFT_OFF6             0x03C0
#define CSHIFT_OFF6             0x03C4
#define P_L0_P_0_Y              0x03DC
#define P_L0_P_1_Y              0x03E0
#define P_L0_P_2_Y              0x03E4
#define P_L0_P_3_Y              0x03E8
#define P_L0_P_4_Y              0x03EC
#define P_L0_P_5_Y              0x03F0
#define P_L0_P_6_Y              0x03F4
#define P_L0_P_7_Y              0x03F8
#define P_L0_P_8_Y              0x03FC
#define P_L0_P_9_Y              0x0400
#define P_L0_P_10_Y             0x0404
#define P_L0_P_11_Y             0x0408
#define P_L0_P_12_Y             0x040C
#define P_L0_P_13_Y             0x0410
#define P_L0_P_14_Y             0x0414
#define P_L0_P_15_Y             0x0418
#define P_L0_P_16_Y             0x041C
#define P_L0_P_17_Y             0x0420
#define P_L0_P_18_Y             0x0424
#define P_L0_P_19_Y             0x0428
#define P_L0_P_20_Y             0x042C
#define P_L0_P_21_Y             0x0430
#define P_L0_P_22_Y             0x0434
#define P_L0_P_23_Y             0x0438
#define P_L0_P_24_Y             0x043C
#define P_L0_P_25_Y             0x0440
#define P_L0_P_26_Y             0x0444
#define P_L0_P_27_Y             0x0448
#define P_L0_P_28_Y             0x044C
#define P_L0_P_29_Y             0x0450
#define P_L0_P_30_Y             0x0454
#define P_L0_P_31_Y             0x0458
#define B_L0_P_0_Y              0x045C
#define B_L0_P_1_Y              0x0460
#define B_L0_P_2_Y              0x0464
#define B_L0_P_3_Y              0x0468
#define B_L0_P_4_Y              0x046C
#define B_L0_P_5_Y              0x0470
#define B_L0_P_6_Y              0x0474
#define B_L0_P_7_Y              0x0478
#define B_L0_P_8_Y              0x047C
#define B_L0_P_9_Y              0x0480
#define B_L0_P_10_Y             0x0484
#define B_L0_P_11_Y             0x0488
#define B_L0_P_12_Y             0x048C
#define B_L0_P_13_Y             0x0490
#define B_L0_P_14_Y             0x0494
#define B_L0_P_15_Y             0x0498
#define B_L0_P_16_Y             0x049C
#define B_L0_P_17_Y             0x04A0
#define B_L0_P_18_Y             0x04A4
#define B_L0_P_19_Y             0x04A8
#define B_L0_P_20_Y             0x04AC
#define B_L0_P_21_Y             0x04B0
#define B_L0_P_22_Y             0x04B4
#define B_L0_P_23_Y             0x04B8
#define B_L0_P_24_Y             0x04BC
#define B_L0_P_25_Y             0x04C0
#define B_L0_P_26_Y             0x04C4
#define B_L0_P_27_Y             0x04C8
#define B_L0_P_28_Y             0x04CC
#define B_L0_P_29_Y             0x04D0
#define B_L0_P_30_Y             0x04D4
#define B_L0_P_31_Y             0x04D8
#define B_L1_P_0_Y              0x04DC
#define B_L1_P_1_Y              0x04E0
#define B_L1_P_2_Y              0x04E4
#define B_L1_P_3_Y              0x04E8
#define B_L1_P_4_Y              0x04EC
#define B_L1_P_5_Y              0x04F0
#define B_L1_P_6_Y              0x04F4
#define B_L1_P_7_Y              0x04F8
#define B_L1_P_8_Y              0x04FC
#define B_L1_P_9_Y              0x0500
#define B_L1_P_10_Y             0x0504
#define B_L1_P_11_Y             0x0508
#define B_L1_P_12_Y             0x050C
#define B_L1_P_13_Y             0x0510
#define B_L1_P_14_Y             0x0514
#define B_L1_P_15_Y             0x0518
#define B_L1_P_16_Y             0x051C
#define B_L1_P_17_Y             0x0520
#define B_L1_P_18_Y             0x0524
#define B_L1_P_19_Y             0x0528
#define B_L1_P_20_Y             0x052C
#define B_L1_P_21_Y             0x0530
#define B_L1_P_22_Y             0x0534
#define B_L1_P_23_Y             0x0538
#define B_L1_P_24_Y             0x053C
#define B_L1_P_25_Y             0x0540
#define B_L1_P_26_Y             0x0544
#define B_L1_P_27_Y             0x0548
#define B_L1_P_28_Y             0x054C
#define B_L1_P_29_Y             0x0550
#define B_L1_P_30_Y             0x0554
#define B_L1_P_31_Y             0x0558
#define H264_C_OFFSET           0x055C
#define H264_OUT_PIC_Y          0x0560
#define P_L0_REF_FIELD          0x0564
#define B_L0_REF_FIELD          0x0568
#define B_L1_REF_FIELD          0x056C
#define BYPASS_DEBLOCKING       0x057C
#define DBKCTL                  0x0658
#define PDBKMBXY                0x065C
#define AGPRI                   0x0660
#define MCRV5                   0x0690
#define MCRV6                   0x0694
#define MCRV8                   0x069C
#define QPBPT                   0x0700
#define DBKATHR                 0x0704
#define PRBCNT_PIC_CYCLE        0x0724
#define PRBCNT_ALE_CYCLE        0x0728
#define PRBCNT_REQ_CYCLE        0x072C
#define PRBCNT_SETTING          0x0730
#define CABAC_WRAP              0x080C
#define ERR_CONCEAL_20          0x0810
#define ERR_CONCEAL_21          0x0814
#define ERR_CONCEAL_22          0x0818
#define CABAC_WRAP_2            0x081C
#define CABAC_WRAP_3            0x0820
#define CABAC_WRAP_4            0x0824
#define PP_OUT_BUF_BYPASS       0x0834
#define PP_DRAM_PROT0           0x0850
#define PP_DRAM_PROT1           0x0854
#define PP_DRAM_PROT2           0x0858
#define VP6_BKDOOR              0x085C
#define VP6_LOOPDBK             0x0860
#define VP6_MV_TH               0x0864
#define VP6_AUTOSEL             0x0868
#define VP6_COEF0_P1            0x086C
#define VP6_COEF0_P2            0x0870
#define VP6_COEF1_P1            0x0874
#define VP6_COEF1_P2            0x0878
#define VP6_COEF2_P1            0x087C
#define VP6_COEF2_P2            0x0880
#define VP6_COEF3_P1            0x0884
#define VP6_COEF3_P2            0x0888
#define VP6_BKDOOR2             0x088C
#define FLDCMP                  0x0920
#define AVS_PB                  0x0930
#define AVS_LSIZE               0x0934
#define DIR_VLDWRAP             0x093C
#define DIR_VLDWRAP2            0x0940
#define MC_VP8                  0x097C
#define MC_PITCH                0x0980
#define MPG2_MC_LAST_MB         0x098C
#define MP4ALLSRAM              0x0990
#define VLDWRAPPER_BASE         0x0A20
#define PPWRAPPER_BASE          0x0A24

// VDEC AVC_VLD REGISTER
#define AVC_VBSR            0x0000
#define AVC_CTRL_REG        0x0084
#define SPS_REG             0x0088
#define PSIZ                0x008C
#define PPS1                0x0090
#define PPS2                0x0094
#define SH1                 0x0098
#define SH2                 0x009C
#define MPR                 0x00A0
#define CPR                 0x00A4
#define PLPRN               0x00A8
#define BL0PRN              0x0128
#define BL1PRN              0x01A8
#define RPNR                0x0228
#define RODR                0x022C
#define WTDR                0x0230
#define ICR                 0x0234
#define AVC_HDR             0x0238
#define UEDR                0x023C
#define SEDR                0x0240
#define SHDR                0x0248
#define SWTR                0x024C
#define SMKR                0x0250
#define SSDR                0x0254
#define SMBR                0x0258
#define SMPR                0x025C
#define SRER                0x0260
#define SRBR                0x0264
#define AVC_ERTR            0x0270
#define AVC_VOKR            0x0274
#define SROR                0x0278
#define HF2R                0x027C
#define ERTER               0x0280
#define ERAR                0x0284
#define SHRT                0x028C
#define DICKSM              0x0294
#define DOCKSM              0x0298
#define AVC_RECKSM          0x029C
#define BSCKSM              0x02A0
#define AVC_LTHR            0x02A4
#define DWCNTR              0x02B4
#define DRCNTR              0x02B8
#define DMWACS              0x02BC
#define DMRACS              0x02C0
#define VOKCR               0x02C4
#define RM03R               0x02C8
#define BRCR                0x02CC
#define ERRRECON            0x02D0
#define DSCKSMN             0x02D4
#define SHR                 0x02D8
#define EMBXYR              0x02E4
#define LPR                 0x02E8
#define ANACL0              0x02F0
#define ANACL1              0x0370
#define MVIDL0R             0x03F0
#define MVIDL1R             0x03F4
#define MVCR                0x03F8
#define LXRDR               0x03FC
#define VBS2R               0x0800
#define SM2R                0x0884
#define UE2DR               0x0888
#define SE2DR               0x088C

// VDEC AVC_MV REGISTER
#define PLIST0_0                0x0000
#define BLIST0_0                0x0080
#define BLIST1_0                0x0100
#define DIRECT_ADR              0x0180
#define CUR_POC                 0x0200
#define TOP_POC                 0x0204
#define BOT_POC                 0x0208
#define DIRECT_WR_START_ADR     0x020C
#define MV_ERR_CFG              0x0210
#define MV_ERR_IMPLICT          0x0214
#define I_PIC_COL               0x0218
#define IRATIO                  0x021C
#define MV_CFG                  0x0220
#define VC1_MV_CFG              0x0224
#define MV_MAX_1                0x0228
#define MV_MAX_2                0x022C
#define CHK_SUM_TYPE            0x024C
#define CHK_SUM_MV0             0x0250
#define CHK_SUM_MV1             0x0254
#define CHK_SUM_DRAM_DI         0x0258
#define CHK_SUM_DRAM_DO         0x025C
#define FW_BLOCK_DISTANCE_1_0   0x02A8
#define FW_BLOCK_DISTANCE_3_2   0x02AC
#define BW_BLOCK_DISTANCE_1_0   0x02B0
#define FW_MED_RATIO_1_0        0x02B4
#define FW_MED_RATIO_3_2        0x02B8
#define BW_MED_RATIO_1_0        0x02BC
#define DIRECT_RATIO_1_0        0x02C0
#define DIRECT_RATIO_3_2        0x02C4
#define WMV78_MV_CFG            0x02D0
#define VP8_MV_CFG              0x02D4
#define MP4_RISC_CFG            0x02D8
#define MP4_MV_GMC              0x02DC
#define MP4_DRT_CFG_0           0x02E0
#define MP4_DRT_CFG_1           0x02E4
#define MP4_DRT_CFG_2           0x02E8
#define MP4_DRT_CFG_3           0x02EC
#define MP4_DRT_RATIO_0         0x02F0
#define MP4_DRT_RATIO_1         0x02F4
#define MP4_DRT_RATIO_2         0x02F8
#define MP4_DRT_RATIO_3         0x02FC

// VDEC PP REGISTER
#define DBK_SPEC            0x0000
#define RV_PARA             0x0008
#define PP_STATUS           0x0010
#define PPY_CHK_SUM         0x0014
#define PPC_CHK_SUM         0x0018
#define RV_QP_VALUE         0x001C
#define VP8_PARA            0x0020
#define VP8_FILTER_LEVEL    0x0024
#define VP8_REF_DELTA       0x0028
#define VP8_MODE_DELTA      0x002C
#define PP_SPECIAL_MODE     0x0030
#define PP_OUT_BUF_PARA     0x003C
#define PP_WEBP_BUF_SIZE    0x006C
#define PP_WEBP_OFFSET      0x0070
#define PP_CKGEN            0x0078
#define DIVX3_DFT_PRED      0x0704

// VDEC SQT REGISTER
#define QUANT_REG               0x0500
#define WMV_DCAC_CTRL           0x0700
#define WMV_DCAC_DEFAULT_VALUE  0x0704

// VDEC VP8_VLD REGISTER
#define VP8_VBSR            0x0000
#define VRLR                0x0084
#define VP8_CTRL_REG        0x00A4
#define PH0                 0x00A8
#define PH1                 0x00AC
#define PPROB               0x00B0
#define FWPROB              0x00B4
#define VP8_HDR             0x00B8
#define COEFR0              0x00C0
#define COEFR1              0x00C4
#define COEFR2              0x00C8
#define COEFR3              0x00CC
#define COEFR4              0x00D0
#define COEFR5              0x00D4
#define COEFR6              0x00D8
#define COEFR7              0x00DC
#define COEFPB0             0x00E0
#define COEFPB1             0x00E4
#define COEFPB2             0x00E8
#define COEFPB3             0x00EC
#define COEFPB4             0x00F0
#define COEFPB5             0x00F4
#define COEFPB6             0x00F8
#define COEFPB7             0x00FC
#define BSASET              0x0100
#define BSDSET              0x0104
#define CSASET              0x0108
#define CSDSET              0x010C
#define BINIT               0x0110
#define BINIT2              0x0114
#define PCHCFG              0x0118
#define RESUME              0x011C
#define MVPROB              0x0120
#define VP8_PICFSM          0x0124
#define MBFSM               0x0128
#define STXFSM              0x012C
#define COEFRPTH            0x0130
#define COEFRPTL            0x0134
#define BITCNTFST           0x0138
#define BITCNTSEC           0x013C
#define SRT                 0x0140
#define BCRT                0x0144
#define CPUT                0x0148
#define MVPUT               0x014C
#define BOOLSR              0x0150
#define MBTSR               0x0154
#define COEFFSM             0x0158
#define VP8_ERTR            0x015C
#define ERRXY               0x0160
#define VP8_RECKSM          0x0164
#define VP8_LTHR            0x0168
#define LTHR2               0x016C
#define VP8_VOKR            0x0170
#define PFR                 0x0174
#define STATUS              0x0178
#define XY                  0x017C
#define HWA                 0x0180
#define RESETXY             0x0190

#endif
