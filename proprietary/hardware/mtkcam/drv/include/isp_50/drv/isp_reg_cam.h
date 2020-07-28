#ifndef _ISP_REG_CAM_H_
#define _ISP_REG_CAM_H_


/* auto insert ralf auto gen below */

typedef volatile union _CAM_UNI_REG_TOP_CTL_
{
        volatile struct /* 0x1A003000 */
        {
                FIELD  INT_MRG                               :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  7;      /*  1.. 7, 0x000000FE */
                FIELD  RAW_A_XMX_IN_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RAW_B_XMX_IN_SEL                      :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RAW_C_XMX_IN_SEL                      :  1;      /* 10..10, 0x00000400 */
                FIELD  RAW_D_XMX_IN_SEL                      :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_CTL;   /* CAM_UNI_TOP_CTL */

typedef volatile union _CAM_UNI_REG_TOP_MISC_
{
        volatile struct /* 0x1A003004 */
        {
                FIELD  DB_LOAD_HOLD                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  DB_EN                                 :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  APB_CLK_GATE_BYPASS                   :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MISC;  /* CAM_UNI_TOP_MISC */

typedef volatile union _CAM_UNI_REG_TOP_SW_CTL_
{
        volatile struct /* 0x1A003008 */
        {
                FIELD  RAWI_SW_RST_Trig                      :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RAWI_SW_RST_ST                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RAWI_HW_RST                           :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  9;      /*  3..11, 0x00000FF8 */
                FIELD  CTL_HW_RST                            :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_SW_CTL;    /* CAM_UNI_TOP_SW_CTL */

typedef volatile union _CAM_UNI_REG_TOP_RAWI_TRIG_
{
        volatile struct /* 0x1A00300C */
        {
                FIELD  TOP_RAWI_TRIG                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_RAWI_TRIG; /* CAM_UNI_TOP_RAWI_TRIG */

typedef volatile union _CAM_UNI_REG_TOP_MOD_EN_
{
        volatile struct /* 0x1A003010 */
        {
                FIELD  UNP2_A_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 27;      /*  1..27, 0x0FFFFFFE */
                FIELD  ADL_A_EN                              :  1;      /* 28..28, 0x10000000 */
                FIELD  RLB_A_EN                              :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_EN;    /* CAM_UNI_TOP_MOD_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_EN_
{
        volatile struct /* 0x1A003014 */
        {
                FIELD  RAWI_A_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_EN;    /* CAM_UNI_TOP_DMA_EN */

typedef volatile union _CAM_UNI_REG_TOP_FMT_SEL_
{
        volatile struct /* 0x1A00301C */
        {
                FIELD  RAWI_FMT                              :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  PIX_BUS_RAWI                          :  2;      /*  8.. 9, 0x00000300 */
                FIELD  rsv_10                                :  2;      /* 10..11, 0x00000C00 */
                FIELD  STAG_MODE_RAWI                        :  2;      /* 12..13, 0x00003000 */
                FIELD  rsv_14                                : 17;      /* 14..30, 0x7FFFC000 */
                FIELD  LP_MODE_RAWI                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_FMT_SEL;   /* CAM_UNI_TOP_FMT_SEL */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_EN_
{
        volatile struct /* 0x1A003020 */
        {
                FIELD  DMA_ERR_INT_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 28;      /*  1..28, 0x1FFFFFFE */
                FIELD  ADL_A_DON_INT_EN                      :  1;      /* 29..29, 0x20000000 */
                FIELD  ADL_A_ERR_INT_EN                      :  1;      /* 30..30, 0x40000000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_EN;    /* CAM_UNI_TOP_DMA_INT_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_STATUS_
{
        volatile struct /* 0x1A003024 */
        {
                FIELD  DMA_ERR_INT_STATUS                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 28;      /*  1..28, 0x1FFFFFFE */
                FIELD  ADL_A_DON_INT_STATUS                  :  1;      /* 29..29, 0x20000000 */
                FIELD  ADL_A_ERR_INT_STATUS                  :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_STATUS;    /* CAM_UNI_TOP_DMA_INT_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_STATUSX_
{
        volatile struct /* 0x1A003028 */
        {
                FIELD  DMA_ERR_INT_STATUS                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 28;      /*  1..28, 0x1FFFFFFE */
                FIELD  ADL_A_DON_INT_STATUS                  :  1;      /* 29..29, 0x20000000 */
                FIELD  ADL_A_ERR_INT_STATUS                  :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_STATUSX;   /* CAM_UNI_TOP_DMA_INT_STATUSX */

typedef volatile union _CAM_UNI_REG_TOP_DBG_SET_
{
        volatile struct /* 0x1A00302C */
        {
                FIELD  DEBUG_MOD_SEL                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DEBUG_SEL                             :  4;      /*  8..11, 0x00000F00 */
                FIELD  DEBUG_TOP_SEL                         :  4;      /* 12..15, 0x0000F000 */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DBG_SET;   /* CAM_UNI_TOP_DBG_SET */

typedef volatile union _CAM_UNI_REG_TOP_DBG_PORT_
{
        volatile struct /* 0x1A003030 */
        {
                FIELD  TOP_DBG_PORT                          : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DBG_PORT;  /* CAM_UNI_TOP_DBG_PORT */

typedef volatile union _CAM_UNI_REG_TOP_DMA_CCU_INT_EN_
{
        volatile struct /* 0x1A003034 */
        {
                FIELD  DMA_ERR_INT_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 28;      /*  1..28, 0x1FFFFFFE */
                FIELD  ADL_A_DON_INT_EN                      :  1;      /* 29..29, 0x20000000 */
                FIELD  ADL_A_ERR_INT_EN                      :  1;      /* 30..30, 0x40000000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_CCU_INT_EN;    /* CAM_UNI_TOP_DMA_CCU_INT_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_CCU_INT_STATUS_
{
        volatile struct /* 0x1A003038 */
        {
                FIELD  DMA_ERR_INT_STATUS                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 28;      /*  1..28, 0x1FFFFFFE */
                FIELD  ADL_A_DON_INT_STATUS                  :  1;      /* 29..29, 0x20000000 */
                FIELD  ADL_A_ERR_INT_STATUS                  :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_CCU_INT_STATUS;    /* CAM_UNI_TOP_DMA_CCU_INT_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_DCM_DIS_
{
        volatile struct /* 0x1A003040 */
        {
                FIELD  UNP2_A_DCM_DIS                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SPARE_DCM_DIS_1                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  3;      /*  2.. 4, 0x0000001C */
                FIELD  SPARE_DCM_DIS_0                       :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  SPARE_DCM_DIS_2                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 19;      /*  9..27, 0x0FFFFE00 */
                FIELD  ADL_A_DCM_DIS                         :  1;      /* 28..28, 0x10000000 */
                FIELD  RLB_A_DCM_DIS                         :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_DCM_DIS;   /* CAM_UNI_TOP_MOD_DCM_DIS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_DCM_DIS_
{
        volatile struct /* 0x1A003044 */
        {
                FIELD  RAWI_A_DCM_DIS                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_DCM_DIS;   /* CAM_UNI_TOP_DMA_DCM_DIS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_DCM_STATUS_
{
        volatile struct /* 0x1A003050 */
        {
                FIELD  UNP2_A_DCM_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 27;      /*  1..27, 0x0FFFFFFE */
                FIELD  ADL_A_DCM_STATUS                      :  1;      /* 28..28, 0x10000000 */
                FIELD  RLB_A_DCM_STATUS                      :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_DCM_STATUS;    /* CAM_UNI_TOP_MOD_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_DCM_STATUS_
{
        volatile struct /* 0x1A003054 */
        {
                FIELD  RAWI_A_DCM_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_DCM_STATUS;    /* CAM_UNI_TOP_DMA_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_REQ_STATUS_
{
        volatile struct /* 0x1A003060 */
        {
                FIELD  UNP2_A_REQ_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 27;      /*  1..27, 0x0FFFFFFE */
                FIELD  ADL_A_REQ_STATUS                      :  1;      /* 28..28, 0x10000000 */
                FIELD  RLB_A_REQ_STATUS                      :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_REQ_STATUS;    /* CAM_UNI_TOP_MOD_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_REQ_STATUS_
{
        volatile struct /* 0x1A003064 */
        {
                FIELD  RAWI_A_REQ_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_REQ_STATUS;    /* CAM_UNI_TOP_DMA_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_RDY_STATUS_
{
        volatile struct /* 0x1A003070 */
        {
                FIELD  UNP2_A_RDY_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 27;      /*  1..27, 0x0FFFFFFE */
                FIELD  ADL_A_RDY_STATUS                      :  1;      /* 28..28, 0x10000000 */
                FIELD  RLB_A_RDY_STATUS                      :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_RDY_STATUS;    /* CAM_UNI_TOP_MOD_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_RDY_STATUS_
{
        volatile struct /* 0x1A003074 */
        {
                FIELD  RAWI_A_RDY_STATUS                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_RDY_STATUS;    /* CAM_UNI_TOP_DMA_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_DMA_SOFT_RSTSTAT_
{
        volatile struct /* 0x1A003100 */
        {
                FIELD  rsv_0                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RAWI_SOFT_RST_STAT                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_SOFT_RSTSTAT;  /* CAM_UNI_DMA_SOFT_RSTSTAT */

typedef volatile union _CAM_UNI_REG_VERTICAL_FLIP_EN_
{
        volatile struct /* 0x1A003104 */
        {
                FIELD  rsv_0                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RAWI_V_FLIP_EN                        :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_VERTICAL_FLIP_EN;  /* CAM_UNI_VERTICAL_FLIP_EN */

typedef volatile union _CAM_UNI_REG_DMA_SOFT_RESET_
{
        volatile struct /* 0x1A003108 */
        {
                FIELD  rsv_0                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RAWI_SOFT_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  SEPARATE_SOFT_RST_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_SOFT_RESET;    /* CAM_UNI_DMA_SOFT_RESET */

typedef volatile union _CAM_UNI_REG_LAST_ULTRA_EN_
{
        volatile struct /* 0x1A00310C */
        {
                FIELD  rsv_0                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RAWI_LAST_ULTRA_EN                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_LAST_ULTRA_EN; /* CAM_UNI_LAST_ULTRA_EN */

typedef volatile union _CAM_UNI_REG_SPECIAL_FUN_EN_
{
        volatile struct /* 0x1A003110 */
        {
                FIELD  SLOW_CNT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                :  4;      /* 16..19, 0x000F0000 */
                FIELD  CONTINUOUS_COM_CON                    :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  GCLAST_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  MULTI_PLANE_ID_EN                     :  1;      /* 24..24, 0x01000000 */
                FIELD  CONTINUOUS_COM_EN                     :  1;      /* 25..25, 0x02000000 */
                FIELD  rsv_26                                :  2;      /* 26..27, 0x0C000000 */
                FIELD  FIFO_CHANGE_EN                        :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  1;      /* 29..29, 0x20000000 */
                FIELD  INTERLACE_MODE                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SLOW_EN                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_SPECIAL_FUN_EN;    /* CAM_UNI_SPECIAL_FUN_EN */

typedef volatile union _CAM_UNI_REG_SPECIAL_FUN2_EN_
{
        volatile struct /* 0x1A003114 */
        {
                FIELD  STAGER_SENSOR_DMA_DLNO                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  STAGER_SENSOR_DMA_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_SPECIAL_FUN2_EN;   /* CAM_UNI_SPECIAL_FUN2_EN */

typedef volatile union _CAM_UNI_REG_RAWI_BASE_ADDR_
{
        volatile struct /* 0x1A003120 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_BASE_ADDR;    /* CAM_UNI_RAWI_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RAWI_OFST_ADDR_
{
        volatile struct /* 0x1A003128 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_OFST_ADDR;    /* CAM_UNI_RAWI_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_RAWI_DRS_
{
        volatile struct /* 0x1A00312C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_DRS;  /* CAM_UNI_RAWI_DRS */

typedef volatile union _CAM_UNI_REG_RAWI_XSIZE_
{
        volatile struct /* 0x1A003130 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_XSIZE;    /* CAM_UNI_RAWI_XSIZE */

typedef volatile union _CAM_UNI_REG_RAWI_YSIZE_
{
        volatile struct /* 0x1A003134 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_YSIZE;    /* CAM_UNI_RAWI_YSIZE */

typedef volatile union _CAM_UNI_REG_RAWI_STRIDE_
{
        volatile struct /* 0x1A003138 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  FORMAT                                :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_STRIDE;   /* CAM_UNI_RAWI_STRIDE */

typedef volatile union _CAM_UNI_REG_RAWI_CON_
{
        volatile struct /* 0x1A00313C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_CON;  /* CAM_UNI_RAWI_CON */

typedef volatile union _CAM_UNI_REG_RAWI_CON2_
{
        volatile struct /* 0x1A003140 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_CON2; /* CAM_UNI_RAWI_CON2 */

typedef volatile union _CAM_UNI_REG_RAWI_CON3_
{
        volatile struct /* 0x1A003144 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_CON3; /* CAM_UNI_RAWI_CON3 */

typedef volatile union _CAM_UNI_REG_RAWI_CON4_
{
        volatile struct /* 0x1A00314C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_CON4; /* CAM_UNI_RAWI_CON4 */

typedef volatile union _CAM_UNI_REG_DMA_ERR_CTRL_
{
        volatile struct /* 0x1A003150 */
        {
                FIELD  rsv_0                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RAWI_ERR                              :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  ERR_CLR_MD                            :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_ERR_CTRL;  /* CAM_UNI_DMA_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_RAWI_ERR_STAT_
{
        volatile struct /* 0x1A003154 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_RAWI_ERR_STAT; /* CAM_UNI_RAWI_ERR_STAT */

typedef volatile union _CAM_UNI_REG_DMA_DEBUG_ADDR_
{
        volatile struct /* 0x1A003158 */
        {
                FIELD  DEBUG_ADDR                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_DEBUG_ADDR;    /* CAM_UNI_DMA_DEBUG_ADDR */

typedef volatile union _CAM_UNI_REG_DMA_RSV1_
{
        volatile struct /* 0x1A00315C */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV1;  /* CAM_UNI_DMA_RSV1 */

typedef volatile union _CAM_UNI_REG_DMA_RSV2_
{
        volatile struct /* 0x1A003160 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV2;  /* CAM_UNI_DMA_RSV2 */

typedef volatile union _CAM_UNI_REG_DMA_RSV3_
{
        volatile struct /* 0x1A003164 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV3;  /* CAM_UNI_DMA_RSV3 */

typedef volatile union _CAM_UNI_REG_DMA_RSV4_
{
        volatile struct /* 0x1A003168 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV4;  /* CAM_UNI_DMA_RSV4 */

typedef volatile union _CAM_UNI_REG_DMA_RSV5_
{
        volatile struct /* 0x1A00316C */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV5;  /* CAM_UNI_DMA_RSV5 */

typedef volatile union _CAM_UNI_REG_DMA_RSV6_
{
        volatile struct /* 0x1A003170 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_RSV6;  /* CAM_UNI_DMA_RSV6 */

typedef volatile union _CAM_UNI_REG_DMA_DEBUG_SEL_
{
        volatile struct /* 0x1A003174 */
        {
                FIELD  DMA_TOP_SEL                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  R_W_DMA_TOP_SEL                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SUB_MODULE_SEL                        :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  6;      /* 24..29, 0x3F000000 */
                FIELD  ARBITER_BVALID_FULL                   :  1;      /* 30..30, 0x40000000 */
                FIELD  ARBITER_COM_FULL                      :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_DMA_DEBUG_SEL; /* CAM_UNI_DMA_DEBUG_SEL */

typedef volatile union _CAM_UNI_REG_UNP2_A_OFST_
{
        volatile struct /* 0x1A003200 */
        {
                FIELD  UNP2_OFST_STB                         :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  UNP2_OFST_EDB                         :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                : 18;      /* 14..31, 0xFFFFC000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_UNP2_A_OFST;   /* CAM_UNI_UNP2_A_OFST */

typedef volatile union _CAM_UNI_REG_TOP_CAM_XMX_SET_
{
        volatile struct /* 0x1A003204 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  RAW_A_XMX_IN_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RAW_B_XMX_IN_SEL                      :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RAW_C_XMX_IN_SEL                      :  1;      /* 10..10, 0x00000400 */
                FIELD  RAW_D_XMX_IN_SEL                      :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_CAM_XMX_SET;   /* CAM_UNI_TOP_CAM_XMX_SET */

typedef volatile union _CAM_UNI_REG_TOP_CAM_XMX_CLR_
{
        volatile struct /* 0x1A003208 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  RAW_A_XMX_IN_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RAW_B_XMX_IN_SEL                      :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RAW_C_XMX_IN_SEL                      :  1;      /* 10..10, 0x00000400 */
                FIELD  RAW_D_XMX_IN_SEL                      :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_TOP_CAM_XMX_CLR;   /* CAM_UNI_TOP_CAM_XMX_CLR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT_
{
        volatile struct /* 0x1A003C00 */
        {
                FIELD  IPUO_SOFT_RST_STAT                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  IPUI_SOFT_RST_STAT                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT;    /* CAM_UNI_ADL_DMA_A_DMA_SOFT_RSTSTAT */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_VERTICAL_FLIP_EN_
{
        volatile struct /* 0x1A003C04 */
        {
                FIELD  IPUO_V_FLIP_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  IPUI_V_FLIP_EN                        :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_VERTICAL_FLIP_EN;    /* CAM_UNI_ADL_DMA_A_VERTICAL_FLIP_EN */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RESET_
{
        volatile struct /* 0x1A003C08 */
        {
                FIELD  IPUO_SOFT_RST                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  IPUI_SOFT_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  SEPARATE_SOFT_RST_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RESET;  /* CAM_UNI_ADL_DMA_A_DMA_SOFT_RESET */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_LAST_ULTRA_EN_
{
        volatile struct /* 0x1A003C0C */
        {
                FIELD  IPUO_LAST_ULTRA_EN                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  IPUI_LAST_ULTRA_EN                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_LAST_ULTRA_EN;   /* CAM_UNI_ADL_DMA_A_LAST_ULTRA_EN */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_SPECIAL_FUN_EN_
{
        volatile struct /* 0x1A003C10 */
        {
                FIELD  SLOW_CNT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                :  7;      /* 16..22, 0x007F0000 */
                FIELD  CONTINUOUS_COM_CON                    :  2;      /* 23..24, 0x01800000 */
                FIELD  CONTINUOUS_COM_EN                     :  1;      /* 25..25, 0x02000000 */
                FIELD  FIFO_CHANGE_EN                        :  1;      /* 26..26, 0x04000000 */
                FIELD  rsv_27                                :  3;      /* 27..29, 0x38000000 */
                FIELD  INTERLACE_MODE                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SLOW_EN                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_SPECIAL_FUN_EN;  /* CAM_UNI_ADL_DMA_A_SPECIAL_FUN_EN */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_RING_
{
        volatile struct /* 0x1A003C14 */
        {
                FIELD  IPUO_RING_YSIZE                       : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  IPUO_RING_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_RING;   /* CAM_UNI_ADL_DMA_A_IPUO_RING */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_RING_
{
        volatile struct /* 0x1A003C18 */
        {
                FIELD  IPUI_RING_YSIZE                       : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  IPUI_RING_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_RING;   /* CAM_UNI_ADL_DMA_A_IPUI_RING */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_BASE_ADDR_
{
        volatile struct /* 0x1A003C30 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_BASE_ADDR;  /* CAM_UNI_ADL_DMA_A_IPUO_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_OFST_ADDR_
{
        volatile struct /* 0x1A003C38 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_OFST_ADDR;  /* CAM_UNI_ADL_DMA_A_IPUO_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_XSIZE_
{
        volatile struct /* 0x1A003C40 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_XSIZE;  /* CAM_UNI_ADL_DMA_A_IPUO_XSIZE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_YSIZE_
{
        volatile struct /* 0x1A003C44 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_YSIZE;  /* CAM_UNI_ADL_DMA_A_IPUO_YSIZE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_STRIDE_
{
        volatile struct /* 0x1A003C48 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                :  4;      /* 20..23, 0x00F00000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_STRIDE; /* CAM_UNI_ADL_DMA_A_IPUO_STRIDE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_CON_
{
        volatile struct /* 0x1A003C4C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_CON;    /* CAM_UNI_ADL_DMA_A_IPUO_CON */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_CON2_
{
        volatile struct /* 0x1A003C50 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_CON2;   /* CAM_UNI_ADL_DMA_A_IPUO_CON2 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_CON3_
{
        volatile struct /* 0x1A003C54 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_CON3;   /* CAM_UNI_ADL_DMA_A_IPUO_CON3 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_BASE_ADDR_
{
        volatile struct /* 0x1A003C90 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_BASE_ADDR;  /* CAM_UNI_ADL_DMA_A_IPUI_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_OFST_ADDR_
{
        volatile struct /* 0x1A003C98 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_OFST_ADDR;  /* CAM_UNI_ADL_DMA_A_IPUI_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_XSIZE_
{
        volatile struct /* 0x1A003CA0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_XSIZE;  /* CAM_UNI_ADL_DMA_A_IPUI_XSIZE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_YSIZE_
{
        volatile struct /* 0x1A003CA4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_YSIZE;  /* CAM_UNI_ADL_DMA_A_IPUI_YSIZE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_STRIDE_
{
        volatile struct /* 0x1A003CA8 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  FORMAT                                :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_STRIDE; /* CAM_UNI_ADL_DMA_A_IPUI_STRIDE */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_CON_
{
        volatile struct /* 0x1A003CAC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_CON;    /* CAM_UNI_ADL_DMA_A_IPUI_CON */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_CON2_
{
        volatile struct /* 0x1A003CB0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_CON2;   /* CAM_UNI_ADL_DMA_A_IPUI_CON2 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_CON3_
{
        volatile struct /* 0x1A003CB4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_CON3;   /* CAM_UNI_ADL_DMA_A_IPUI_CON3 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_ERR_CTRL_
{
        volatile struct /* 0x1A003D00 */
        {
                FIELD  IPUO_ERR                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  IPUI_ERR                              :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  ERR_CLR_MD                            :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_ERR_CTRL;    /* CAM_UNI_ADL_DMA_A_DMA_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUO_ERR_STAT_
{
        volatile struct /* 0x1A003D04 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUO_ERR_STAT;   /* CAM_UNI_ADL_DMA_A_IPUO_ERR_STAT */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_IPUI_ERR_STAT_
{
        volatile struct /* 0x1A003D08 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_IPUI_ERR_STAT;   /* CAM_UNI_ADL_DMA_A_IPUI_ERR_STAT */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_ADDR_
{
        volatile struct /* 0x1A003D0C */
        {
                FIELD  DEBUG_ADDR                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_ADDR;  /* CAM_UNI_ADL_DMA_A_DMA_DEBUG_ADDR */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV1_
{
        volatile struct /* 0x1A003D10 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV1;    /* CAM_UNI_ADL_DMA_A_DMA_RSV1 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV2_
{
        volatile struct /* 0x1A003D14 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV2;    /* CAM_UNI_ADL_DMA_A_DMA_RSV2 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV3_
{
        volatile struct /* 0x1A003D18 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV3;    /* CAM_UNI_ADL_DMA_A_DMA_RSV3 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV4_
{
        volatile struct /* 0x1A003D1C */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV4;    /* CAM_UNI_ADL_DMA_A_DMA_RSV4 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV5_
{
        volatile struct /* 0x1A003D20 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV5;    /* CAM_UNI_ADL_DMA_A_DMA_RSV5 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_RSV6_
{
        volatile struct /* 0x1A003D24 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_RSV6;    /* CAM_UNI_ADL_DMA_A_DMA_RSV6 */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_SEL_
{
        volatile struct /* 0x1A003D28 */
        {
                FIELD  DMA_TOP_SEL                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  R_W_DMA_TOP_SEL                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SUB_MODULE_SEL                        :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  6;      /* 24..29, 0x3F000000 */
                FIELD  ARBITER_BVALID_FULL                   :  1;      /* 30..30, 0x40000000 */
                FIELD  ARBITER_COM_FULL                      :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_SEL;   /* CAM_UNI_ADL_DMA_A_DMA_DEBUG_SEL */

typedef volatile union _CAM_UNI_REG_ADL_DMA_A_DMA_BW_SELF_TEST_
{
        volatile struct /* 0x1A003D2C */
        {
                FIELD  BW_SELF_TEST_EN_IPUO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_A_DMA_BW_SELF_TEST;    /* CAM_UNI_ADL_DMA_A_DMA_BW_SELF_TEST */

typedef volatile union _CAM_UNI_REG_ADL_RESET_
{
        volatile struct /* 0x1A003F00 */
        {
                FIELD  DMA_0_SOFT_RST_ST                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  7;      /*  1.. 7, 0x000000FE */
                FIELD  DMA_0_SOFT_RST                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  RDMA_0_HW_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  WDMA_0_HW_RST                         :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  6;      /* 18..23, 0x00FC0000 */
                FIELD  AHB2GMC_HW_RST                        :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_RESET; /* CAM_UNI_ADL_RESET */

typedef volatile union _CAM_UNI_REG_ADL_CTL_
{
        volatile struct /* 0x1A003F04 */
        {
                FIELD  SRC_SEL                               :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  DST_SEL                               :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  SRC_TYPE                              :  2;      /*  8.. 9, 0x00000300 */
                FIELD  DST_TYPE                              :  2;      /* 10..11, 0x00000C00 */
                FIELD  H2G_GID                               :  2;      /* 12..13, 0x00003000 */
                FIELD  H2G_GULTRA_ENABLE                     :  1;      /* 14..14, 0x00004000 */
                FIELD  H2G_EARLY_RESP                        :  1;      /* 15..15, 0x00008000 */
                FIELD  rsv_16                                :  4;      /* 16..19, 0x000F0000 */
                FIELD  ENABLE                                :  2;      /* 20..21, 0x00300000 */
                FIELD  DONE_DIS                              :  3;      /* 22..24, 0x01C00000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_CTL;   /* CAM_UNI_ADL_CTL */

typedef volatile union _CAM_UNI_REG_ADL_CROP_IN_START_
{
        volatile struct /* 0x1A003F30 */
        {
                FIELD  CROP_IN_START_X                       : 16;      /*  0..15, 0x0000FFFF */
                FIELD  CROP_IN_START_Y                       : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_CROP_IN_START; /* CAM_UNI_ADL_CROP_IN_START */

typedef volatile union _CAM_UNI_REG_ADL_CROP_IN_END_
{
        volatile struct /* 0x1A003F34 */
        {
                FIELD  CROP_IN_END_X                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  CROP_IN_END_Y                         : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_CROP_IN_END;   /* CAM_UNI_ADL_CROP_IN_END */

typedef volatile union _CAM_UNI_REG_ADL_CROP_OUT_START_
{
        volatile struct /* 0x1A003F38 */
        {
                FIELD  CROP_OUT_START_X                      : 16;      /*  0..15, 0x0000FFFF */
                FIELD  CROP_OUT_START_Y                      : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_CROP_OUT_START;    /* CAM_UNI_ADL_CROP_OUT_START */

typedef volatile union _CAM_UNI_REG_ADL_CROP_OUT_END_
{
        volatile struct /* 0x1A003F3C */
        {
                FIELD  CROP_OUT_END_X                        : 16;      /*  0..15, 0x0000FFFF */
                FIELD  CROP_OUT_END_Y                        : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_CROP_OUT_END;  /* CAM_UNI_ADL_CROP_OUT_END */

typedef volatile union _CAM_UNI_REG_ADL_DMA_ST_
{
        volatile struct /* 0x1A003F40 */
        {
                FIELD  RDMA_0_DONE_ST                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  WDMA_0_DONE_ST                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RDMA_1_DONE_ST                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  WDMA_1_DONE_ST                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 : 28;      /*  4..31, 0xFFFFFFF0 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_ST;    /* CAM_UNI_ADL_DMA_ST */

typedef volatile union _CAM_UNI_REG_ADL_DCM_DIS_
{
        volatile struct /* 0x1A003F44 */
        {
                FIELD  RDMA_DCM_DIS                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  WDMA_DCM_DIS                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  SRAM_DCM_DIS                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
                FIELD  H2G_CG_DIS                            :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DCM_DIS;   /* CAM_UNI_ADL_DCM_DIS */

typedef volatile union _CAM_UNI_REG_ADL_DCM_ST_
{
        volatile struct /* 0x1A003F48 */
        {
                FIELD  RDMA_0_DCM_ST                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  WDMA_0_DCM_ST                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  RDMA_1_DCM_ST                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
                FIELD  WDMA_1_DCM_ST                         :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  DMA_0_SRAM_DCM_ST                     :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                :  3;      /* 17..19, 0x000E0000 */
                FIELD  DMA_1_SRAM_DCM_ST                     :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DCM_ST;    /* CAM_UNI_ADL_DCM_ST */

typedef volatile union _CAM_UNI_REG_ADL_DMA_ERR_ST_
{
        volatile struct /* 0x1A003F4C */
        {
                FIELD  DMA_0_ERR_INT                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  DMA_0_REQ_ST                          :  5;      /*  4.. 8, 0x000001F0 */
                FIELD  DMA_0_RDY_ST                          :  5;      /*  9..13, 0x00003E00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  DMA_1_ERR_INT                         :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                :  3;      /* 17..19, 0x000E0000 */
                FIELD  DMA_1_REQ_ST                          :  5;      /* 20..24, 0x01F00000 */
                FIELD  DMA_1_RDY_ST                          :  5;      /* 25..29, 0x3E000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_ERR_ST;    /* CAM_UNI_ADL_DMA_ERR_ST */

typedef volatile union _CAM_UNI_REG_ADL_DMA_0_DEBUG_
{
        volatile struct /* 0x1A003F50 */
        {
                FIELD  DMA_0_DEBUG                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_0_DEBUG;   /* CAM_UNI_ADL_DMA_0_DEBUG */

typedef volatile union _CAM_UNI_REG_ADL_DMA_1_DEBUG_
{
        volatile struct /* 0x1A003F54 */
        {
                FIELD  DMA_1_DEBUG                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_DMA_1_DEBUG;   /* CAM_UNI_ADL_DMA_1_DEBUG */

typedef volatile union _CAM_UNI_REG_ADL_SPARE_
{
        volatile struct /* 0x1A003F70 */
        {
                FIELD  SPARE                                 : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_SPARE; /* CAM_UNI_ADL_SPARE */

typedef volatile union _CAM_UNI_REG_ADL_INFO00_
{
        volatile struct /* 0x1A003F80 */
        {
                FIELD  INFO00                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO00;    /* CAM_UNI_ADL_INFO00 */

typedef volatile union _CAM_UNI_REG_ADL_INFO01_
{
        volatile struct /* 0x1A003F84 */
        {
                FIELD  INFO01                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO01;    /* CAM_UNI_ADL_INFO01 */

typedef volatile union _CAM_UNI_REG_ADL_INFO02_
{
        volatile struct /* 0x1A003F88 */
        {
                FIELD  INFO02                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO02;    /* CAM_UNI_ADL_INFO02 */

typedef volatile union _CAM_UNI_REG_ADL_INFO03_
{
        volatile struct /* 0x1A003F8C */
        {
                FIELD  INFO03                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO03;    /* CAM_UNI_ADL_INFO03 */

typedef volatile union _CAM_UNI_REG_ADL_INFO04_
{
        volatile struct /* 0x1A003F90 */
        {
                FIELD  INFO04                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO04;    /* CAM_UNI_ADL_INFO04 */

typedef volatile union _CAM_UNI_REG_ADL_INFO05_
{
        volatile struct /* 0x1A003F94 */
        {
                FIELD  INFO05                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO05;    /* CAM_UNI_ADL_INFO05 */

typedef volatile union _CAM_UNI_REG_ADL_INFO06_
{
        volatile struct /* 0x1A003F98 */
        {
                FIELD  INFO06                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO06;    /* CAM_UNI_ADL_INFO06 */

typedef volatile union _CAM_UNI_REG_ADL_INFO07_
{
        volatile struct /* 0x1A003F9C */
        {
                FIELD  INFO07                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO07;    /* CAM_UNI_ADL_INFO07 */

typedef volatile union _CAM_UNI_REG_ADL_INFO08_
{
        volatile struct /* 0x1A003FA0 */
        {
                FIELD  INFO08                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO08;    /* CAM_UNI_ADL_INFO08 */

typedef volatile union _CAM_UNI_REG_ADL_INFO09_
{
        volatile struct /* 0x1A003FA4 */
        {
                FIELD  INFO09                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO09;    /* CAM_UNI_ADL_INFO09 */

typedef volatile union _CAM_UNI_REG_ADL_INFO10_
{
        volatile struct /* 0x1A003FA8 */
        {
                FIELD  INFO10                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO10;    /* CAM_UNI_ADL_INFO10 */

typedef volatile union _CAM_UNI_REG_ADL_INFO11_
{
        volatile struct /* 0x1A003FAC */
        {
                FIELD  INFO11                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO11;    /* CAM_UNI_ADL_INFO11 */

typedef volatile union _CAM_UNI_REG_ADL_INFO12_
{
        volatile struct /* 0x1A003FB0 */
        {
                FIELD  INFO12                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO12;    /* CAM_UNI_ADL_INFO12 */

typedef volatile union _CAM_UNI_REG_ADL_INFO13_
{
        volatile struct /* 0x1A003FB4 */
        {
                FIELD  INFO13                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO13;    /* CAM_UNI_ADL_INFO13 */

typedef volatile union _CAM_UNI_REG_ADL_INFO14_
{
        volatile struct /* 0x1A003FB8 */
        {
                FIELD  INFO14                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO14;    /* CAM_UNI_ADL_INFO14 */

typedef volatile union _CAM_UNI_REG_ADL_INFO15_
{
        volatile struct /* 0x1A003FBC */
        {
                FIELD  INFO15                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO15;    /* CAM_UNI_ADL_INFO15 */

typedef volatile union _CAM_UNI_REG_ADL_INFO16_
{
        volatile struct /* 0x1A003FC0 */
        {
                FIELD  INFO16                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO16;    /* CAM_UNI_ADL_INFO16 */

typedef volatile union _CAM_UNI_REG_ADL_INFO17_
{
        volatile struct /* 0x1A003FC4 */
        {
                FIELD  INFO17                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO17;    /* CAM_UNI_ADL_INFO17 */

typedef volatile union _CAM_UNI_REG_ADL_INFO18_
{
        volatile struct /* 0x1A003FC8 */
        {
                FIELD  INFO18                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO18;    /* CAM_UNI_ADL_INFO18 */

typedef volatile union _CAM_UNI_REG_ADL_INFO19_
{
        volatile struct /* 0x1A003FCC */
        {
                FIELD  INFO19                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO19;    /* CAM_UNI_ADL_INFO19 */

typedef volatile union _CAM_UNI_REG_ADL_INFO20_
{
        volatile struct /* 0x1A003FD0 */
        {
                FIELD  INFO20                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO20;    /* CAM_UNI_ADL_INFO20 */

typedef volatile union _CAM_UNI_REG_ADL_INFO21_
{
        volatile struct /* 0x1A003FD4 */
        {
                FIELD  INFO21                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO21;    /* CAM_UNI_ADL_INFO21 */

typedef volatile union _CAM_UNI_REG_ADL_INFO22_
{
        volatile struct /* 0x1A003FD8 */
        {
                FIELD  INFO22                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO22;    /* CAM_UNI_ADL_INFO22 */

typedef volatile union _CAM_UNI_REG_ADL_INFO23_
{
        volatile struct /* 0x1A003FDC */
        {
                FIELD  INFO23                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO23;    /* CAM_UNI_ADL_INFO23 */

typedef volatile union _CAM_UNI_REG_ADL_INFO24_
{
        volatile struct /* 0x1A003FE0 */
        {
                FIELD  INFO24                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO24;    /* CAM_UNI_ADL_INFO24 */

typedef volatile union _CAM_UNI_REG_ADL_INFO25_
{
        volatile struct /* 0x1A003FE4 */
        {
                FIELD  INFO25                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO25;    /* CAM_UNI_ADL_INFO25 */

typedef volatile union _CAM_UNI_REG_ADL_INFO26_
{
        volatile struct /* 0x1A003FE8 */
        {
                FIELD  INFO26                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO26;    /* CAM_UNI_ADL_INFO26 */

typedef volatile union _CAM_UNI_REG_ADL_INFO27_
{
        volatile struct /* 0x1A003FEC */
        {
                FIELD  INFO27                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO27;    /* CAM_UNI_ADL_INFO27 */

typedef volatile union _CAM_UNI_REG_ADL_INFO28_
{
        volatile struct /* 0x1A003FF0 */
        {
                FIELD  INFO28                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO28;    /* CAM_UNI_ADL_INFO28 */

typedef volatile union _CAM_UNI_REG_ADL_INFO29_
{
        volatile struct /* 0x1A003FF4 */
        {
                FIELD  INFO29                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO29;    /* CAM_UNI_ADL_INFO29 */

typedef volatile union _CAM_UNI_REG_ADL_INFO30_
{
        volatile struct /* 0x1A003FF8 */
        {
                FIELD  INFO30                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO30;    /* CAM_UNI_ADL_INFO30 */

typedef volatile union _CAM_UNI_REG_ADL_INFO31_
{
        volatile struct /* 0x1A003FFC */
        {
                FIELD  INFO31                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_UNI_REG_ADL_INFO31;    /* CAM_UNI_ADL_INFO31 */

typedef volatile union _CAM_REG_CTL_START_
{
        volatile struct /* 0x1A004000 */
        {
                FIELD  CQ_THR0_START                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  CQ_THR1_START                         :  1;      /*  1.. 1, 0x00000002 */
                FIELD  CQ_THR2_START                         :  1;      /*  2.. 2, 0x00000004 */
                FIELD  CQ_THR3_START                         :  1;      /*  3.. 3, 0x00000008 */
                FIELD  CQ_THR4_START                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  CQ_THR5_START                         :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_THR6_START                         :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_THR7_START                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_THR8_START                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CQ_THR9_START                         :  1;      /*  9.. 9, 0x00000200 */
                FIELD  CQ_THR10_START                        :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  CQ_THR12_START                        :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_START; /* CAM_CTL_START, CAM_A_CTL_START*/

typedef volatile union _CAM_REG_CTL_EN_
{
        volatile struct /* 0x1A004004 */
        {
                FIELD  TG_EN                                 :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DMX_EN                                :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SGM_EN                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RMG_EN                                :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_EN                                :  1;      /*  4.. 4, 0x00000010 */
                FIELD  OBC_EN                                :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BNR_EN                                :  1;      /*  6.. 6, 0x00000040 */
                FIELD  LSC_EN                                :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CAC_EN                                :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RPG_EN                                :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZ_EN                                :  1;      /* 10..10, 0x00000400 */
                FIELD  RMX_EN                                :  1;      /* 11..11, 0x00000800 */
                FIELD  PAKG_EN                               :  1;      /* 12..12, 0x00001000 */
                FIELD  BMX_EN                                :  1;      /* 13..13, 0x00002000 */
                FIELD  CPG_EN                                :  1;      /* 14..14, 0x00004000 */
                FIELD  PAK_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  UFE_EN                                :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_EN                                 :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG1_EN                               :  1;      /* 18..18, 0x00040000 */
                FIELD  AA_EN                                 :  1;      /* 19..19, 0x00080000 */
                FIELD  QBIN1_EN                              :  1;      /* 20..20, 0x00100000 */
                FIELD  LCS_EN                                :  1;      /* 21..21, 0x00200000 */
                FIELD  QBIN2_EN                              :  1;      /* 22..22, 0x00400000 */
                FIELD  RCP_EN                                :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_EN                               :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  2;      /* 25..26, 0x06000000 */
                FIELD  BIN_EN                                :  1;      /* 27..27, 0x08000000 */
                FIELD  DBS_EN                                :  1;      /* 28..28, 0x10000000 */
                FIELD  DBN_EN                                :  1;      /* 29..29, 0x20000000 */
                FIELD  PBN_EN                                :  1;      /* 30..30, 0x40000000 */
                FIELD  UFEG_EN                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_EN;    /* CAM_CTL_EN, CAM_A_CTL_EN*/

typedef volatile union _CAM_REG_CTL_DMA_EN_
{
        volatile struct /* 0x1A004008 */
        {
                FIELD  IMGO_EN                               :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_EN                               :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_EN                               :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_EN                                :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_EN                               :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_EN                                :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_EN                                :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_EN                               :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_EN                               :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CACI_EN                               :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_EN                                :  1;      /* 10..10, 0x00000400 */
                FIELD  LSC3I_EN                              :  1;      /* 11..11, 0x00000800 */
                FIELD  PDI_EN                                :  1;      /* 12..12, 0x00001000 */
                FIELD  FLKO_EN                               :  1;      /* 13..13, 0x00002000 */
                FIELD  LMVO_EN                               :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_EN                               :  1;      /* 15..15, 0x00008000 */
                FIELD  UFGO_EN                               :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DMA_EN;    /* CAM_CTL_DMA_EN, CAM_A_CTL_DMA_EN*/

typedef volatile union _CAM_REG_CTL_FMT_SEL_
{
        volatile struct /* 0x1A00400C */
        {
                FIELD  PIX_ID                                :  2;      /*  0.. 1, 0x00000003 */
                FIELD  RRZO_FMT                              :  2;      /*  2.. 3, 0x0000000C */
                FIELD  IMGO_FMT                              :  5;      /*  4.. 8, 0x000001F0 */
                FIELD  rsv_9                                 :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PIX_BUS_DMXO                          :  2;      /* 10..11, 0x00000C00 */
                FIELD  TG_FMT                                :  3;      /* 12..14, 0x00007000 */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  TG_SWAP                               :  2;      /* 16..17, 0x00030000 */
                FIELD  HLR_MODE                              :  2;      /* 18..19, 0x000C0000 */
                FIELD  DMX_ID                                :  2;      /* 20..21, 0x00300000 */
                FIELD  PIX_BUS_AMXO                          :  2;      /* 22..23, 0x00C00000 */
                FIELD  PIX_BUS_DMXI                          :  2;      /* 24..25, 0x03000000 */
                FIELD  PIX_BUS_BMXO                          :  2;      /* 26..27, 0x0C000000 */
                FIELD  PIX_BUS_RMXO                          :  2;      /* 28..29, 0x30000000 */
                FIELD  RRZO_FG_MODE                          :  1;      /* 30..30, 0x40000000 */
                FIELD  LP_MODE                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_FMT_SEL;   /* CAM_CTL_FMT_SEL, CAM_A_CTL_FMT_SEL*/

typedef volatile union _CAM_REG_CTL_SEL_
{
        volatile struct /* 0x1A004010 */
        {
                FIELD  DMX_SEL                               :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  HDS1_SEL                              :  1;      /*  4.. 4, 0x00000010 */
                FIELD  DMX_POS_SEL                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  RMBN_SEL                              :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  STM_SEL                               :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  1;      /*  9.. 9, 0x00000200 */
                FIELD  AA_SEL                                :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  SGG_SEL                               :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  1;      /* 13..13, 0x00002000 */
                FIELD  LCS_SEL                               :  1;      /* 14..14, 0x00004000 */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  IMG_SEL                               :  2;      /* 16..17, 0x00030000 */
                FIELD  UFE_SEL                               :  2;      /* 18..19, 0x000C0000 */
                FIELD  FLK1_SEL                              :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  1;      /* 21..21, 0x00200000 */
                FIELD  PDO_SEL                               :  1;      /* 22..22, 0x00400000 */
                FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_SEL                              :  2;      /* 24..25, 0x03000000 */
                FIELD  UFEG_SEL                              :  1;      /* 26..26, 0x04000000 */
                FIELD  rsv_27                                :  1;      /* 27..27, 0x08000000 */
                FIELD  RCP_SEL                               :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  1;      /* 29..29, 0x20000000 */
                FIELD  AAO_SEL                               :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_SEL;   /* CAM_CTL_SEL, CAM_A_CTL_SEL*/

typedef volatile union _CAM_REG_CTL_MISC_
{
        volatile struct /* 0x1A004014 */
        {
                FIELD  DB_LOAD_HOLD                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  DB_EN                                 :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  APB_CLK_GATE_BYPASS                   :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
                FIELD  AF_DB_LOAD_HOLD                       :  1;      /* 12..12, 0x00001000 */
                FIELD  AA_DB_LOAD_HOLD                       :  1;      /* 13..13, 0x00002000 */
                FIELD  rsv_14                                : 18;      /* 14..31, 0xFFFFC000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_MISC;  /* CAM_CTL_MISC, CAM_A_CTL_MISC*/

typedef volatile union _CAM_REG_CTL_EN2_
{
        volatile struct /* 0x1A004018 */
        {
                FIELD  SGG3_EN                               :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FLK_EN                                :  1;      /*  1.. 1, 0x00000002 */
                FIELD  HDS_EN                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMV_EN                                :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RSS_EN                                :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  8;      /*  5..12, 0x00001FE0 */
                FIELD  CPN_EN                                :  1;      /* 13..13, 0x00002000 */
                FIELD  DCPN_EN                               :  1;      /* 14..14, 0x00004000 */
                FIELD  ADBS_EN                               :  1;      /* 15..15, 0x00008000 */
                FIELD  BMX2_EN                               :  1;      /* 16..16, 0x00010000 */
                FIELD  PSB_EN                                :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG2_EN                               :  1;      /* 18..18, 0x00040000 */
                FIELD  PDE_EN                                :  1;      /* 19..19, 0x00080000 */
                FIELD  GSE_EN                                :  1;      /* 20..20, 0x00100000 */
                FIELD  PCP_EN                                :  1;      /* 21..21, 0x00200000 */
                FIELD  RMB_EN                                :  1;      /* 22..22, 0x00400000 */
                FIELD  PS_EN                                 :  1;      /* 23..23, 0x00800000 */
                FIELD  HLR_EN                                :  1;      /* 24..24, 0x01000000 */
                FIELD  AMX_EN                                :  1;      /* 25..25, 0x02000000 */
                FIELD  SL2F_EN                               :  1;      /* 26..26, 0x04000000 */
                FIELD  VBN_EN                                :  1;      /* 27..27, 0x08000000 */
                FIELD  SL2J_EN                               :  1;      /* 28..28, 0x10000000 */
                FIELD  STM_EN                                :  1;      /* 29..29, 0x20000000 */
                FIELD  SCM_EN                                :  1;      /* 30..30, 0x40000000 */
                FIELD  SGG5_EN                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_EN2;   /* CAM_CTL_EN2, CAM_A_CTL_EN2*/

typedef volatile union _CAM_REG_CTL_RAW_INT_EN_
{
        volatile struct /* 0x1A004020 */
        {
                FIELD  VS_INT_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_EN                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_EN                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON_EN                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_CODE_ERR_EN                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_APB_ERR_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_VS_ERR_EN                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_DROP_FRAME_EN                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_DROP_FRAME_EN                    :  1;      /* 10..10, 0x00000400 */
                FIELD  PASS1_DON_EN                          :  1;      /* 11..11, 0x00000800 */
                FIELD  SOF_INT_EN                            :  1;      /* 12..12, 0x00001000 */
                FIELD  SOF_WAIT_EN                           :  1;      /* 13..13, 0x00002000 */
                FIELD  BMX2_ERR_EN                           :  1;      /* 14..14, 0x00004000 */
                FIELD  AMX_ERR_EN                            :  1;      /* 15..15, 0x00008000 */
                FIELD  RMX_ERR_EN                            :  1;      /* 16..16, 0x00010000 */
                FIELD  BMX_ERR_EN                            :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_ERR_EN                           :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_ERR_EN                            :  1;      /* 19..19, 0x00080000 */
                FIELD  IMGO_ERR_EN                           :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_ERR_EN                            :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_ERR_EN                            :  1;      /* 22..22, 0x00400000 */
                FIELD  LCSO_ERR_EN                           :  1;      /* 23..23, 0x00800000 */
                FIELD  BPC_ERR_EN                            :  1;      /* 24..24, 0x01000000 */
                FIELD  LSC_ERR_EN                            :  1;      /* 25..25, 0x02000000 */
                FIELD  UFGO_ERR_EN                           :  1;      /* 26..26, 0x04000000 */
                FIELD  UFEO_ERR_EN                           :  1;      /* 27..27, 0x08000000 */
                FIELD  PDO_ERR_EN                            :  1;      /* 28..28, 0x10000000 */
                FIELD  DMA_ERR_EN                            :  1;      /* 29..29, 0x20000000 */
                FIELD  SW_PASS1_DON_EN                       :  1;      /* 30..30, 0x40000000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT_EN;    /* CAM_CTL_RAW_INT_EN, CAM_A_CTL_RAW_INT_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT_STATUS_
{
        volatile struct /* 0x1A004024 */
        {
                FIELD  VS_INT_ST                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_ST                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_ST                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON_ST                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_ST                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_ST                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_CODE_ERR_ST                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_APB_ERR_ST                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_VS_ERR_ST                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_DROP_FRAME_ST                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_DROP_FRAME_ST                    :  1;      /* 10..10, 0x00000400 */
                FIELD  PASS1_DON_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  SOF_INT_ST                            :  1;      /* 12..12, 0x00001000 */
                FIELD  SOF_WAIT_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  BMX2_ERR_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  AMX_ERR_ST                            :  1;      /* 15..15, 0x00008000 */
                FIELD  RMX_ERR_ST                            :  1;      /* 16..16, 0x00010000 */
                FIELD  BMX_ERR_ST                            :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_ERR_ST                            :  1;      /* 19..19, 0x00080000 */
                FIELD  IMGO_ERR_ST                           :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_ERR_ST                            :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_ERR_ST                            :  1;      /* 22..22, 0x00400000 */
                FIELD  LCSO_ERR_ST                           :  1;      /* 23..23, 0x00800000 */
                FIELD  BNR_ERR_ST                            :  1;      /* 24..24, 0x01000000 */
                FIELD  LSC_ERR_ST                            :  1;      /* 25..25, 0x02000000 */
                FIELD  UFGO_ERR_ST                           :  1;      /* 26..26, 0x04000000 */
                FIELD  UFEO_ERR_ST                           :  1;      /* 27..27, 0x08000000 */
                FIELD  PDO_ERR_ST                            :  1;      /* 28..28, 0x10000000 */
                FIELD  DMA_ERR_ST                            :  1;      /* 29..29, 0x20000000 */
                FIELD  SW_PASS1_DON_ST                       :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT_STATUS;    /* CAM_CTL_RAW_INT_STATUS, CAM_A_CTL_RAW_INT_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT_STATUSX_
{
        volatile struct /* 0x1A004028 */
        {
                FIELD  VS_INT_ST                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_ST                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_ST                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON_ST                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_ST                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_ST                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_CODE_ERR_ST                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_APB_ERR_ST                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_VS_ERR_ST                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_DROP_FRAME_ST                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_DROP_FRAME_ST                    :  1;      /* 10..10, 0x00000400 */
                FIELD  PASS1_DON_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  SOF_INT_ST                            :  1;      /* 12..12, 0x00001000 */
                FIELD  SOF_WAIT_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  BMX2_ERR_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  AMX_ERR_ST                            :  1;      /* 15..15, 0x00008000 */
                FIELD  RMX_ERR_ST                            :  1;      /* 16..16, 0x00010000 */
                FIELD  BMX_ERR_ST                            :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_ERR_ST                            :  1;      /* 19..19, 0x00080000 */
                FIELD  IMGO_ERR_ST                           :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_ERR_ST                            :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_ERR_ST                            :  1;      /* 22..22, 0x00400000 */
                FIELD  LCSO_ERR_ST                           :  1;      /* 23..23, 0x00800000 */
                FIELD  BNR_ERR_ST                            :  1;      /* 24..24, 0x01000000 */
                FIELD  LSC_ERR_ST                            :  1;      /* 25..25, 0x02000000 */
                FIELD  UFGO_ERR_ST                           :  1;      /* 26..26, 0x04000000 */
                FIELD  UFEO_ERR_ST                           :  1;      /* 27..27, 0x08000000 */
                FIELD  PDO_ERR_ST                            :  1;      /* 28..28, 0x10000000 */
                FIELD  DMA_ERR_ST                            :  1;      /* 29..29, 0x20000000 */
                FIELD  SW_PASS1_DON_ST                       :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT_STATUSX;   /* CAM_CTL_RAW_INT_STATUSX, CAM_A_CTL_RAW_INT_STATUSX*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_EN_
{
        volatile struct /* 0x1A004030 */
        {
                FIELD  IMGO_DONE_EN                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DONE_EN                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DONE_EN                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_DONE_EN                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_DONE_EN                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_DONE_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  LCSO_DONE_EN                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AAO_DONE_EN                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSC3I_DONE_EN                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BPCI_DONE_EN                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSCI_DONE_EN                          :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_DONE_EN                          :  1;      /* 11..11, 0x00000800 */
                FIELD  AF_TAR_DONE_EN                        :  1;      /* 12..12, 0x00001000 */
                FIELD  PDO_DONE_EN                           :  1;      /* 13..13, 0x00002000 */
                FIELD  PSO_DONE_EN                           :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DONE_EN                          :  1;      /* 15..15, 0x00008000 */
                FIELD  CQ_THR0_DONE_EN                       :  1;      /* 16..16, 0x00010000 */
                FIELD  CQ_THR1_DONE_EN                       :  1;      /* 17..17, 0x00020000 */
                FIELD  CQ_THR2_DONE_EN                       :  1;      /* 18..18, 0x00040000 */
                FIELD  CQ_THR3_DONE_EN                       :  1;      /* 19..19, 0x00080000 */
                FIELD  CQ_THR4_DONE_EN                       :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_THR5_DONE_EN                       :  1;      /* 21..21, 0x00200000 */
                FIELD  CQ_THR6_DONE_EN                       :  1;      /* 22..22, 0x00400000 */
                FIELD  CQ_THR7_DONE_EN                       :  1;      /* 23..23, 0x00800000 */
                FIELD  CQ_THR8_DONE_EN                       :  1;      /* 24..24, 0x01000000 */
                FIELD  CQ_THR9_DONE_EN                       :  1;      /* 25..25, 0x02000000 */
                FIELD  CQ_THR10_DONE_EN                      :  1;      /* 26..26, 0x04000000 */
                FIELD  CQ_THR11_DONE_EN                      :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_THR12_DONE_EN                      :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_EN;   /* CAM_CTL_RAW_INT2_EN, CAM_A_CTL_RAW_INT2_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_STATUS_
{
        volatile struct /* 0x1A004034 */
        {
                FIELD  IMGO_DONE_ST                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DONE_ST                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DONE_ST                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_DONE_ST                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_DONE_ST                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_DEONE_ST                          :  1;      /*  5.. 5, 0x00000020 */
                FIELD  LCSO_DONE_ST                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AAO_DONE_ST                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSC3I_DONE_ST                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BPCI_DONE_ST                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSCI_DONE_ST                          :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_DONE_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  AF_TAR_DONE_ST                        :  1;      /* 12..12, 0x00001000 */
                FIELD  PDO_DONE_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  PSO_DONE_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DONE_ST                          :  1;      /* 15..15, 0x00008000 */
                FIELD  CQ_THR0_DONE_ST                       :  1;      /* 16..16, 0x00010000 */
                FIELD  CQ_THR1_DONE_ST                       :  1;      /* 17..17, 0x00020000 */
                FIELD  CQ_THR2_DONE_ST                       :  1;      /* 18..18, 0x00040000 */
                FIELD  CQ_THR3_DONE_ST                       :  1;      /* 19..19, 0x00080000 */
                FIELD  CQ_THR4_DONE_ST                       :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_THR5_DONE_ST                       :  1;      /* 21..21, 0x00200000 */
                FIELD  CQ_THR6_DONE_ST                       :  1;      /* 22..22, 0x00400000 */
                FIELD  CQ_THR7_DONE_ST                       :  1;      /* 23..23, 0x00800000 */
                FIELD  CQ_THR8_DONE_ST                       :  1;      /* 24..24, 0x01000000 */
                FIELD  CQ_THR9_DONE_ST                       :  1;      /* 25..25, 0x02000000 */
                FIELD  CQ_THR10_DONE_ST                      :  1;      /* 26..26, 0x04000000 */
                FIELD  CQ_THR11_DONE_ST                      :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_THR12_DONE_ST                      :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_STATUS;   /* CAM_CTL_RAW_INT2_STATUS, CAM_A_CTL_RAW_INT2_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_STATUSX_
{
        volatile struct /* 0x1A004038 */
        {
                FIELD  IMGO_DONE_ST                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DONE_ST                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DONE_ST                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_DONE_ST                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_DONE_ST                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_DEONE_ST                          :  1;      /*  5.. 5, 0x00000020 */
                FIELD  LCSO_DONE_ST                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AAO_DONE_ST                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSC3I_DONE_ST                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BPCI_DONE_ST                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSCI_DONE_ST                          :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_DONE_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  AF_TAR_DONE_ST                        :  1;      /* 12..12, 0x00001000 */
                FIELD  PDO_DONE_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  PSO_DONE_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DONE_ST                          :  1;      /* 15..15, 0x00008000 */
                FIELD  CQ_THR0_DONE_ST                       :  1;      /* 16..16, 0x00010000 */
                FIELD  CQ_THR1_DONE_ST                       :  1;      /* 17..17, 0x00020000 */
                FIELD  CQ_THR2_DONE_ST                       :  1;      /* 18..18, 0x00040000 */
                FIELD  CQ_THR3_DONE_ST                       :  1;      /* 19..19, 0x00080000 */
                FIELD  CQ_THR4_DONE_ST                       :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_THR5_DONE_ST                       :  1;      /* 21..21, 0x00200000 */
                FIELD  CQ_THR6_DONE_ST                       :  1;      /* 22..22, 0x00400000 */
                FIELD  CQ_THR7_DONE_ST                       :  1;      /* 23..23, 0x00800000 */
                FIELD  CQ_THR8_DONE_ST                       :  1;      /* 24..24, 0x01000000 */
                FIELD  CQ_THR9_DONE_ST                       :  1;      /* 25..25, 0x02000000 */
                FIELD  CQ_THR10_DONE_ST                      :  1;      /* 26..26, 0x04000000 */
                FIELD  CQ_THR11_DONE_ST                      :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_THR12_DONE_ST                      :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_STATUSX;  /* CAM_CTL_RAW_INT2_STATUSX, CAM_A_CTL_RAW_INT2_STATUSX*/

typedef volatile union _CAM_REG_CTL_SW_CTL_
{
        volatile struct /* 0x1A004040 */
        {
                FIELD  SW_RST_Trig                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SW_RST_ST                             :  1;      /*  1.. 1, 0x00000002 */
                FIELD  HW_RST                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 : 29;      /*  3..31, 0xFFFFFFF8 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_SW_CTL;    /* CAM_CTL_SW_CTL, CAM_A_CTL_SW_CTL*/

typedef volatile union _CAM_REG_CTL_AB_DONE_SEL_
{
        volatile struct /* 0x1A004044 */
        {
                FIELD  IMGO_A_DONE_SEL                       :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_A_DONE_SEL                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_A_DONE_SEL                       :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_A_DONE_SEL                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_A_DONE_SEL                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_A_DONE_SEL                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_A_DONE_SEL                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_A_DONE_SEL                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_A_DONE_SEL                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  UFGO_A_DONE_SEL                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_A_DONE_SEL                        :  1;      /* 10..10, 0x00000400 */
                FIELD  TG_A_DONE_SEL                         :  1;      /* 11..11, 0x00000800 */
                FIELD  LSC3I_A_DONE_SEL                      :  1;      /* 12..12, 0x00001000 */
                FIELD  PDI_A_DONE_SEL                        :  1;      /* 13..13, 0x00002000 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  IMGO_B_DONE_SEL                       :  1;      /* 16..16, 0x00010000 */
                FIELD  UFEO_B_DONE_SEL                       :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_B_DONE_SEL                       :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_B_DONE_SEL                        :  1;      /* 19..19, 0x00080000 */
                FIELD  LCSO_B_DONE_SEL                       :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_B_DONE_SEL                        :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_B_DONE_SEL                        :  1;      /* 22..22, 0x00400000 */
                FIELD  BPCI_B_DONE_SEL                       :  1;      /* 23..23, 0x00800000 */
                FIELD  LSCI_B_DONE_SEL                       :  1;      /* 24..24, 0x01000000 */
                FIELD  UFGO_B_DONE_SEL                       :  1;      /* 25..25, 0x02000000 */
                FIELD  PDO_B_DONE_SEL                        :  1;      /* 26..26, 0x04000000 */
                FIELD  TG_B_DONE_SEL                         :  1;      /* 27..27, 0x08000000 */
                FIELD  LSC3I_B_DONE_SEL                      :  1;      /* 28..28, 0x10000000 */
                FIELD  PDI_B_DONE_SEL                        :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_AB_DONE_SEL;   /* CAM_CTL_AB_DONE_SEL, CAM_A_CTL_AB_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_CD_DONE_SEL_
{
        volatile struct /* 0x1A004048 */
        {
                FIELD  IMGO_C_DONE_SEL                       :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_C_DONE_SEL                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_C_DONE_SEL                       :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_C_DONE_SEL                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_C_DONE_SEL                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_C_DONE_SEL                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_C_DONE_SEL                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_C_DONE_SEL                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_C_DONE_SEL                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  UFGO_C_DONE_SEL                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_C_DONE_SEL                        :  1;      /* 10..10, 0x00000400 */
                FIELD  TG_C_DONE_SEL                         :  1;      /* 11..11, 0x00000800 */
                FIELD  LSC3I_C_DONE_SEL                      :  1;      /* 12..12, 0x00001000 */
                FIELD  PDI_C_DONE_SEL                        :  1;      /* 13..13, 0x00002000 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  IMGO_D_DONE_SEL                       :  1;      /* 16..16, 0x00010000 */
                FIELD  UFEO_D_DONE_SEL                       :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_D_DONE_SEL                       :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_D_DONE_SEL                        :  1;      /* 19..19, 0x00080000 */
                FIELD  LCSO_D_DONE_SEL                       :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_D_DONE_SEL                        :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_D_DONE_SEL                        :  1;      /* 22..22, 0x00400000 */
                FIELD  BPCI_D_DONE_SEL                       :  1;      /* 23..23, 0x00800000 */
                FIELD  LSCI_D_DONE_SEL                       :  1;      /* 24..24, 0x01000000 */
                FIELD  UFGO_D_DONE_SEL                       :  1;      /* 25..25, 0x02000000 */
                FIELD  PDO_D_DONE_SEL                        :  1;      /* 26..26, 0x04000000 */
                FIELD  TG_D_DONE_SEL                         :  1;      /* 27..27, 0x08000000 */
                FIELD  LSC3I_D_DONE_SEL                      :  1;      /* 28..28, 0x10000000 */
                FIELD  PDI_D_DONE_SEL                        :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_CD_DONE_SEL;   /* CAM_CTL_CD_DONE_SEL, CAM_A_CTL_CD_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_UNI_DONE_SEL_
{
        volatile struct /* 0x1A00404C */
        {
                FIELD  DONE_SEL_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RSSO_A_DONE_SEL                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_A_DONE_SEL                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_A_DONE_SEL                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  4;      /*  5.. 8, 0x000001E0 */
                FIELD  RSSO_C_DONE_SEL                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  rsv_10                                :  1;      /* 10..10, 0x00000400 */
                FIELD  LMVO_C_DONE_SEL                       :  1;      /* 11..11, 0x00000800 */
                FIELD  FLKO_C_DONE_SEL                       :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 15;      /* 13..27, 0x0FFFE000 */
                FIELD  ADL_A_DONE_SEL                        :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_UNI_DONE_SEL;  /* CAM_CTL_UNI_DONE_SEL, CAM_A_CTL_UNI_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_TWIN_STATUS_
{
        volatile struct /* 0x1A004050 */
        {
                FIELD  TWIN_EN                               : 4;       /*  0.. 3, 0x0000000F */
                FIELD  MASTER_MODULE                         : 4;       /*  4.. 7, 0x000000F0 */
                FIELD  SLAVE_CAM_NUM                         : 4;       /*  8..11, 0x00000F00 */
                FIELD  TWIN_MODULE                           : 4;       /* 12..15, 0x0000F000 */
                FIELD  TRIPLE_MODULE                         : 4;       /* 16..19, 0x000F0000 */
                FIELD  SPARE0                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_TWIN_STATUS;   /* CAM_CTL_TWIN_STATUS, CAM_A_CTL_SPARE0*/

typedef volatile union _CAM_REG_CTL_SPARE1_
{
        volatile struct /* 0x1A004054 */
        {
                FIELD  SPARE1                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_SPARE1;    /* CAM_CTL_SPARE1, CAM_A_CTL_SPARE1*/

typedef volatile union _CAM_REG_CTL_SPARE2_
{
        volatile struct /* 0x1A004058 */
        {
                FIELD  SPARE2                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_SPARE2;    /* CAM_CTL_SPARE2, CAM_A_CTL_SPARE2*/

typedef volatile union _CAM_REG_CTL_SW_PASS1_DONE_
{
        volatile struct /* 0x1A00405C */
        {
                FIELD  DOWN_SAMPLE_PERIOD                    :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  DOWN_SAMPLE_EN                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_FIFO_FULL_DROP                   :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_FIFO_FULL_DROP                   :  1;      /* 10..10, 0x00000400 */
                FIELD  LCSO_FIFO_FULL_DROP                   :  1;      /* 11..11, 0x00000800 */
                FIELD  UFEO_FIFO_FULL_DROP                   :  1;      /* 12..12, 0x00001000 */
                FIELD  LMVO_FIFO_FULL_DROP                   :  1;      /* 13..13, 0x00002000 */
                FIELD  RSSO_FIFO_FULL_DROP                   :  1;      /* 14..14, 0x00004000 */
                FIELD  UFGO_FIFO_FULL_DROP                   :  1;      /* 15..15, 0x00008000 */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_SW_PASS1_DONE; /* CAM_CTL_SW_PASS1_DONE, CAM_A_CTL_SW_PASS1_DONE*/

typedef volatile union _CAM_REG_CTL_FBC_RCNT_INC_
{
        volatile struct /* 0x1A004060 */
        {
                FIELD  IMGO_RCNT_INC                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_RCNT_INC                         :  1;      /*  1.. 1, 0x00000002 */
                FIELD  LCSO_RCNT_INC                         :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AAO_RCNT_INC                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  PDO_A_RCNT_INC                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_A_RCNT_INC                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  AFO_B_RCNT_INC                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AFO_C_RCNT_INC                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  AFO_D_RCNT_INC                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  UFEO_RCNT_INC                         :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LMVO_RCNT_INC                         :  1;      /* 10..10, 0x00000400 */
                FIELD  RSSO_RCNT_INC                         :  1;      /* 11..11, 0x00000800 */
                FIELD  FLKO_RCNT_INC                         :  1;      /* 12..12, 0x00001000 */
                FIELD  PSO_RCNT_INC                          :  1;      /* 13..13, 0x00002000 */
                FIELD  UFGO_RCNT_INC                         :  1;      /* 14..14, 0x00004000 */
                FIELD  rsv_15                                : 17;      /* 15..31, 0xFFFF8000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_FBC_RCNT_INC;  /* CAM_CTL_FBC_RCNT_INC, CAM_A_CTL_FBC_RCNT_INC*/

typedef volatile union _CAM_REG_CTL_DBG_SET_
{
        volatile struct /* 0x1A004070 */
        {
                FIELD  DEBUG_MOD_SEL                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DEBUG_SEL                             :  4;      /*  8..11, 0x00000F00 */
                FIELD  DEBUG_TOP_SEL                         :  4;      /* 12..15, 0x0000F000 */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DBG_SET;   /* CAM_CTL_DBG_SET, CAM_A_CTL_DBG_SET*/

typedef volatile union _CAM_REG_CTL_DBG_PORT_
{
        volatile struct /* 0x1A004074 */
        {
                FIELD  CTL_DBG_PORT                          : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DBG_PORT;  /* CAM_CTL_DBG_PORT, CAM_A_CTL_DBG_PORT*/

typedef volatile union _CAM_REG_CTL_DATE_CODE_
{
        volatile struct /* 0x1A004078 */
        {
                FIELD  CTL_DATE_CODE                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DATE_CODE; /* CAM_CTL_DATE_CODE, CAM_A_CTL_DATE_CODE*/

typedef volatile union _CAM_REG_CTL_PROJ_CODE_
{
        volatile struct /* 0x1A00407C */
        {
                FIELD  CTL_PROJ_CODE                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_PROJ_CODE; /* CAM_CTL_PROJ_CODE, CAM_A_CTL_PROJ_CODE*/

typedef volatile union _CAM_REG_CTL_RAW_DCM_DIS_
{
        volatile struct /* 0x1A004080 */
        {
                FIELD  TG_DCM_DIS                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DMX_DCM_DIS                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SGM_DCM_DIS                           :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RMG_DCM_DIS                           :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_DCM_DIS                           :  1;      /*  4.. 4, 0x00000010 */
                FIELD  OBC_DCM_DIS                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BNR_DCM_DIS                           :  1;      /*  6.. 6, 0x00000040 */
                FIELD  LSC_DCM_DIS                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CAC_DCM_DIS                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RPG_DCM_DIS                           :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZ_DCM_DIS                           :  1;      /* 10..10, 0x00000400 */
                FIELD  RMX_DCM_DIS                           :  1;      /* 11..11, 0x00000800 */
                FIELD  PAKG_DCM_DIS                          :  1;      /* 12..12, 0x00001000 */
                FIELD  BMX_DCM_DIS                           :  1;      /* 13..13, 0x00002000 */
                FIELD  CPG_DCM_DIS                           :  1;      /* 14..14, 0x00004000 */
                FIELD  PAK_DCM_DIS                           :  1;      /* 15..15, 0x00008000 */
                FIELD  UFE_DCM_DIS                           :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_DCM_DIS                            :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG1_DCM_DIS                          :  1;      /* 18..18, 0x00040000 */
                FIELD  AA_DCM_DIS                            :  1;      /* 19..19, 0x00080000 */
                FIELD  QBIN1_DCM_DIS                         :  1;      /* 20..20, 0x00100000 */
                FIELD  LCS_DCM_DIS                           :  1;      /* 21..21, 0x00200000 */
                FIELD  QBIN2_DCM_DIS                         :  1;      /* 22..22, 0x00400000 */
                FIELD  RCP_DCM_DIS                           :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_DCM_DIS                          :  1;      /* 24..24, 0x01000000 */
                FIELD  PMX_DCM_DIS                           :  1;      /* 25..25, 0x02000000 */
                FIELD  PKP_DCM_DIS                           :  1;      /* 26..26, 0x04000000 */
                FIELD  BIN_DCM_DIS                           :  1;      /* 27..27, 0x08000000 */
                FIELD  DBS_DCM_DIS                           :  1;      /* 28..28, 0x10000000 */
                FIELD  DBN_DCM_DIS                           :  1;      /* 29..29, 0x20000000 */
                FIELD  PBN_DCM_DIS                           :  1;      /* 30..30, 0x40000000 */
                FIELD  UFEG_DCM_DIS                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_DIS;   /* CAM_CTL_RAW_DCM_DIS, CAM_A_CTL_RAW_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_DMA_DCM_DIS_
{
        volatile struct /* 0x1A004084 */
        {
                FIELD  IMGO_DCM_DIS                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DCM_DIS                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DCM_DIS                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_DCM_DIS                           :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_DCM_DIS                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_DCM_DIS                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_DCM_DIS                           :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_DCM_DIS                          :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_DCM_DIS                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CACI_DCM_DIS                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_DCM_DIS                           :  1;      /* 10..10, 0x00000400 */
                FIELD  LSC3I_DCM_DIS                         :  1;      /* 11..11, 0x00000800 */
                FIELD  PDI_DCM_DIS                           :  1;      /* 12..12, 0x00001000 */
                FIELD  FLKO_DCM_DIS                          :  1;      /* 13..13, 0x00002000 */
                FIELD  LMVO_DCM_DIS                          :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DCM_DIS                          :  1;      /* 15..15, 0x00008000 */
                FIELD  UFGO_DCM_DIS                          :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_DIS;   /* CAM_CTL_DMA_DCM_DIS, CAM_A_CTL_DMA_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_TOP_DCM_DIS_
{
        volatile struct /* 0x1A004088 */
        {
                FIELD  TOP_DCM_DIS                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_DIS;   /* CAM_CTL_TOP_DCM_DIS, CAM_A_CTL_TOP_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_RAW_DCM_STATUS_
{
        volatile struct /* 0x1A004090 */
        {
                FIELD  TG_DCM_STATUS                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DMX_DCM_STATUS                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SGM_DCM_STATUS                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RMG_DCM_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_DCM_STATUS                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  OBC_DCM_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BNR_DCM_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  LSC_DCM_STATUS                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CAC_DCM_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RPG_DCM_STATUS                        :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZ_DCM_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  RMX_DCM_STATUS                        :  1;      /* 11..11, 0x00000800 */
                FIELD  PAKG_DCM_STATUS                       :  1;      /* 12..12, 0x00001000 */
                FIELD  BMX_DCM_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  CPG_DCM_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  PAK_DCM_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  UFE_DCM_STATUS                        :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_DCM_STATUS                         :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG1_DCM_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  AA_DCM_STATUS                         :  1;      /* 19..19, 0x00080000 */
                FIELD  QBIN1_DCM_STATUS                      :  1;      /* 20..20, 0x00100000 */
                FIELD  LCS_DCM_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  QBIN2_DCM_STATUS                      :  1;      /* 22..22, 0x00400000 */
                FIELD  RCP_DCM_DIS                           :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_DCM_STATUS                       :  1;      /* 24..24, 0x01000000 */
                FIELD  PMX_DCM_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  PKP_DCM_STATUS                        :  1;      /* 26..26, 0x04000000 */
                FIELD  BIN_DCM_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  DBS_DCM_STATUS                        :  1;      /* 28..28, 0x10000000 */
                FIELD  DBN_DCM_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  PBN_DCM_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  UFEG_DCM_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_STATUS;    /* CAM_CTL_RAW_DCM_STATUS, CAM_A_CTL_RAW_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_DCM_STATUS_
{
        volatile struct /* 0x1A004094 */
        {
                FIELD  IMGO_DCM_STATUS                       :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DCM_STATUS                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DCM_STATUS                       :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_DCM_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_DCM_STATUS                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_DCM_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_DCM_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_DCM_STATUS                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_DCM_STATUS                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CACI_DCM_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_DCM_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  LSC3I_DCM_STATUS                      :  1;      /* 11..11, 0x00000800 */
                FIELD  PDI_DCM_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  FLKO_DCM_DIS                          :  1;      /* 13..13, 0x00002000 */
                FIELD  LMVO_DCM_DIS                          :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DCM_STATUS                       :  1;      /* 15..15, 0x00008000 */
                FIELD  UFGO_DCM_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_STATUS;    /* CAM_CTL_DMA_DCM_STATUS, CAM_A_CTL_DMA_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_TOP_DCM_STATUS_
{
        volatile struct /* 0x1A004098 */
        {
                FIELD  TOP_DCM_STATUS                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_STATUS;    /* CAM_CTL_TOP_DCM_STATUS, CAM_A_CTL_TOP_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_REQ_STATUS_
{
        volatile struct /* 0x1A0040A0 */
        {
                FIELD  TG_REQ_STATUS                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DMX_REQ_STATUS                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SGM_REQ_STATUS                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RMG_REQ_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_REQ_STATUS                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  OBC_REQ_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BNR_REQ_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  LSC_REQ_STATUS                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CAC_REQ_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RPG_REQ_STATUS                        :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZ_REQ_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  RMX_REQ_STATUS                        :  1;      /* 11..11, 0x00000800 */
                FIELD  PAKG_REQ_STATUS                       :  1;      /* 12..12, 0x00001000 */
                FIELD  BMX_REQ_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  CPG_REQ_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  PAK_REQ_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  UFE_REQ_STATUS                        :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_REQ_STATUS                         :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG1_REQ_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  AA_REQ_STATUS                         :  1;      /* 19..19, 0x00080000 */
                FIELD  QBIN1_REQ_STATUS                      :  1;      /* 20..20, 0x00100000 */
                FIELD  LCS_REQ_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  QBIN2_REQ_STATUS                      :  1;      /* 22..22, 0x00400000 */
                FIELD  RCP_REQ_DIS                           :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_REQ_STATUS                       :  1;      /* 24..24, 0x01000000 */
                FIELD  PMX_REQ_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  PKP_REQ_STATUS                        :  1;      /* 26..26, 0x04000000 */
                FIELD  BIN_REQ_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  DBS_REQ_STATUS                        :  1;      /* 28..28, 0x10000000 */
                FIELD  DBN_REQ_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  PBN_REQ_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  UFEG_REQ_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_REQ_STATUS;    /* CAM_CTL_RAW_REQ_STATUS, CAM_A_CTL_RAW_REQ_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_REQ_STATUS_
{
        volatile struct /* 0x1A0040A4 */
        {
                FIELD  IMGO_REQ_STATUS                       :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_REQ_STATUS                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_REQ_STATUS                       :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_REQ_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_REQ_STATUS                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_REQ_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_REQ_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_REQ_STATUS                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_REQ_STATUS                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CACI_REQ_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_REQ_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  LSC3I_REQ_STATUS                      :  1;      /* 11..11, 0x00000800 */
                FIELD  PDI_REQ_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  FLKO_REQ_STATUS                       :  1;      /* 13..13, 0x00002000 */
                FIELD  LMVO_REQ_STATUS                       :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_REQ_STATUS                       :  1;      /* 15..15, 0x00008000 */
                FIELD  UFGO_REQ_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DMA_REQ_STATUS;    /* CAM_CTL_DMA_REQ_STATUS, CAM_A_CTL_DMA_REQ_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_RDY_STATUS_
{
        volatile struct /* 0x1A0040A8 */
        {
                FIELD  TG_RDY_STATUS                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DMX_RDY_STATUS                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SGM_RDY_STATUS                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RMG_RDY_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_RDY_STATUS                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  OBC_RDY_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BNR_RDY_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  LSC_RDY_STATUS                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CAC_RDY_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  RPG_RDY_STATUS                        :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZ_RDY_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  RMX_RDY_STATUS                        :  1;      /* 11..11, 0x00000800 */
                FIELD  PAKG_RDY_STATUS                       :  1;      /* 12..12, 0x00001000 */
                FIELD  BMX_RDY_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  CPG_RDY_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  PAK_RDY_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  UFE_RDY_STATUS                        :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_RDY_STATUS                         :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG1_RDY_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  AA_RDY_STATUS                         :  1;      /* 19..19, 0x00080000 */
                FIELD  QBIN1_RDY_STATUS                      :  1;      /* 20..20, 0x00100000 */
                FIELD  LCS_RDY_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  QBIN2_RDY_STATUS                      :  1;      /* 22..22, 0x00400000 */
                FIELD  RCP_RDY_STATUS                        :  1;      /* 23..23, 0x00800000 */
                FIELD  RCP3_RDY_STATUS                       :  1;      /* 24..24, 0x01000000 */
                FIELD  PMX_RDY_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  PKP_RDY_STATUS                        :  1;      /* 26..26, 0x04000000 */
                FIELD  BIN_RDY_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  DBS_RDY_STATUS                        :  1;      /* 28..28, 0x10000000 */
                FIELD  DBN_RDY_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  PBN_RDY_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  UFEG_RDY_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_RDY_STATUS;    /* CAM_CTL_RAW_RDY_STATUS, CAM_A_CTL_RAW_RDY_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_RDY_STATUS_
{
        volatile struct /* 0x1A0040AC */
        {
                FIELD  IMGO_RDY_STATUS                       :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_RDY_STATUS                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_RDY_STATUS                       :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_RDY_STATUS                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_RDY_STATUS                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AAO_RDY_STATUS                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PSO_RDY_STATUS                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BPCI_RDY_STATUS                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSCI_RDY_STATUS                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  CACI_RDY_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  PDO_RDY_STATUS                        :  1;      /* 10..10, 0x00000400 */
                FIELD  LSC3I_RDY_STATUS                      :  1;      /* 11..11, 0x00000800 */
                FIELD  PDI_RDY_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  FLKO_RDY_STATUS                       :  1;      /* 13..13, 0x00002000 */
                FIELD  LMVO_RDY_STATUS                       :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_RDY_STATUS                       :  1;      /* 15..15, 0x00008000 */
                FIELD  UFGO_RDY_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_DMA_RDY_STATUS;    /* CAM_CTL_DMA_RDY_STATUS, CAM_A_CTL_DMA_RDY_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT_EN_
{
        volatile struct /* 0x1A0040B0 */
        {
                FIELD  VS_INT_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_EN                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_EN                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON_EN                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_CODE_ERR_EN                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_APB_ERR_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_VS_ERR_EN                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_DROP_FRAME_EN                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_DROP_FRAME_EN                    :  1;      /* 10..10, 0x00000400 */
                FIELD  PASS1_DON_EN                          :  1;      /* 11..11, 0x00000800 */
                FIELD  SOF_INT_EN                            :  1;      /* 12..12, 0x00001000 */
                FIELD  SOF_WAIT_EN                           :  1;      /* 13..13, 0x00002000 */
                FIELD  BMX2_ERR_EN                           :  1;      /* 14..14, 0x00004000 */
                FIELD  AMX_ERR_EN                            :  1;      /* 15..15, 0x00008000 */
                FIELD  RMX_ERR_EN                            :  1;      /* 16..16, 0x00010000 */
                FIELD  BMX_ERR_EN                            :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_ERR_EN                           :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_ERR_EN                            :  1;      /* 19..19, 0x00080000 */
                FIELD  IMGO_ERR_EN                           :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_ERR_EN                            :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_ERR_EN                            :  1;      /* 22..22, 0x00400000 */
                FIELD  LCSO_ERR_EN                           :  1;      /* 23..23, 0x00800000 */
                FIELD  BPC_ERR_EN                            :  1;      /* 24..24, 0x01000000 */
                FIELD  LSC_ERR_EN                            :  1;      /* 25..25, 0x02000000 */
                FIELD  UFGO_ERR_EN                           :  1;      /* 26..26, 0x04000000 */
                FIELD  UFEO_ERR_EN                           :  1;      /* 27..27, 0x08000000 */
                FIELD  PDO_ERR_EN                            :  1;      /* 28..28, 0x10000000 */
                FIELD  DMA_ERR_EN                            :  1;      /* 29..29, 0x20000000 */
                FIELD  SW_PASS1_DON_EN                       :  1;      /* 30..30, 0x40000000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT_EN;    /* CAM_CTL_RAW_CCU_INT_EN, CAM_A_CTL_RAW_CCU_INT_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT_STATUS_
{
        volatile struct /* 0x1A0040B4 */
        {
                FIELD  VS_INT_ST                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_ST                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_ST                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON_ST                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_ST                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_ST                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  CQ_CODE_ERR_ST                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  CQ_APB_ERR_ST                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  CQ_VS_ERR_ST                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  IMGO_DROP_FRAME_ST                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RRZO_DROP_FRAME_ST                    :  1;      /* 10..10, 0x00000400 */
                FIELD  PASS1_DON_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  SOF_INT_ST                            :  1;      /* 12..12, 0x00001000 */
                FIELD  SOF_WAIT_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  BMX2_ERR_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  AMX_ERR_ST                            :  1;      /* 15..15, 0x00008000 */
                FIELD  RMX_ERR_ST                            :  1;      /* 16..16, 0x00010000 */
                FIELD  BMX_ERR_ST                            :  1;      /* 17..17, 0x00020000 */
                FIELD  RRZO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  AFO_ERR_ST                            :  1;      /* 19..19, 0x00080000 */
                FIELD  IMGO_ERR_ST                           :  1;      /* 20..20, 0x00100000 */
                FIELD  AAO_ERR_ST                            :  1;      /* 21..21, 0x00200000 */
                FIELD  PSO_ERR_ST                            :  1;      /* 22..22, 0x00400000 */
                FIELD  LCSO_ERR_ST                           :  1;      /* 23..23, 0x00800000 */
                FIELD  BNR_ERR_ST                            :  1;      /* 24..24, 0x01000000 */
                FIELD  LSC_ERR_ST                            :  1;      /* 25..25, 0x02000000 */
                FIELD  UFGO_ERR_ST                           :  1;      /* 26..26, 0x04000000 */
                FIELD  UFEO_ERR_ST                           :  1;      /* 27..27, 0x08000000 */
                FIELD  PDO_ERR_ST                            :  1;      /* 28..28, 0x10000000 */
                FIELD  DMA_ERR_ST                            :  1;      /* 29..29, 0x20000000 */
                FIELD  SW_PASS1_DON_ST                       :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT_STATUS;    /* CAM_CTL_RAW_CCU_INT_STATUS, CAM_A_CTL_RAW_CCU_INT_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT2_EN_
{
        volatile struct /* 0x1A0040B8 */
        {
                FIELD  IMGO_DONE_EN                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DONE_EN                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DONE_EN                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_DONE_EN                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_DONE_EN                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_DONE_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  LCSO_DONE_EN                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AAO_DONE_EN                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSC3I_DONE_EN                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BPCI_DONE_EN                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSCI_DONE_EN                          :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_DONE_EN                          :  1;      /* 11..11, 0x00000800 */
                FIELD  AF_TAR_DONE_EN                        :  1;      /* 12..12, 0x00001000 */
                FIELD  PDO_DONE_EN                           :  1;      /* 13..13, 0x00002000 */
                FIELD  PSO_DONE_EN                           :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DONE_EN                          :  1;      /* 15..15, 0x00008000 */
                FIELD  CQ_THR0_DONE_EN                       :  1;      /* 16..16, 0x00010000 */
                FIELD  CQ_THR1_DONE_EN                       :  1;      /* 17..17, 0x00020000 */
                FIELD  CQ_THR2_DONE_EN                       :  1;      /* 18..18, 0x00040000 */
                FIELD  CQ_THR3_DONE_EN                       :  1;      /* 19..19, 0x00080000 */
                FIELD  CQ_THR4_DONE_EN                       :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_THR5_DONE_EN                       :  1;      /* 21..21, 0x00200000 */
                FIELD  CQ_THR6_DONE_EN                       :  1;      /* 22..22, 0x00400000 */
                FIELD  CQ_THR7_DONE_EN                       :  1;      /* 23..23, 0x00800000 */
                FIELD  CQ_THR8_DONE_EN                       :  1;      /* 24..24, 0x01000000 */
                FIELD  CQ_THR9_DONE_EN                       :  1;      /* 25..25, 0x02000000 */
                FIELD  CQ_THR10_DONE_EN                      :  1;      /* 26..26, 0x04000000 */
                FIELD  CQ_THR11_DONE_EN                      :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_THR12_DONE_EN                      :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT2_EN;   /* CAM_CTL_RAW_CCU_INT2_EN, CAM_A_CTL_RAW_CCU_INT2_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT2_STATUS_
{
        volatile struct /* 0x1A0040BC */
        {
                FIELD  IMGO_DONE_ST                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  UFEO_DONE_ST                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZO_DONE_ST                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_DONE_ST                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_DONE_ST                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AFO_DEONE_ST                          :  1;      /*  5.. 5, 0x00000020 */
                FIELD  LCSO_DONE_ST                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  AAO_DONE_ST                           :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LSC3I_DONE_ST                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BPCI_DONE_ST                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSCI_DONE_ST                          :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_DONE_ST                          :  1;      /* 11..11, 0x00000800 */
                FIELD  AF_TAR_DONE_ST                        :  1;      /* 12..12, 0x00001000 */
                FIELD  PDO_DONE_ST                           :  1;      /* 13..13, 0x00002000 */
                FIELD  PSO_DONE_ST                           :  1;      /* 14..14, 0x00004000 */
                FIELD  RSSO_DONE_ST                          :  1;      /* 15..15, 0x00008000 */
                FIELD  CQ_THR0_DONE_ST                       :  1;      /* 16..16, 0x00010000 */
                FIELD  CQ_THR1_DONE_ST                       :  1;      /* 17..17, 0x00020000 */
                FIELD  CQ_THR2_DONE_ST                       :  1;      /* 18..18, 0x00040000 */
                FIELD  CQ_THR3_DONE_ST                       :  1;      /* 19..19, 0x00080000 */
                FIELD  CQ_THR4_DONE_ST                       :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_THR5_DONE_ST                       :  1;      /* 21..21, 0x00200000 */
                FIELD  CQ_THR6_DONE_ST                       :  1;      /* 22..22, 0x00400000 */
                FIELD  CQ_THR7_DONE_ST                       :  1;      /* 23..23, 0x00800000 */
                FIELD  CQ_THR8_DONE_ST                       :  1;      /* 24..24, 0x01000000 */
                FIELD  CQ_THR9_DONE_ST                       :  1;      /* 25..25, 0x02000000 */
                FIELD  CQ_THR10_DONE_ST                      :  1;      /* 26..26, 0x04000000 */
                FIELD  CQ_THR11_DONE_ST                      :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_THR12_DONE_ST                      :  1;      /* 28..28, 0x10000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT2_STATUS;   /* CAM_CTL_RAW_CCU_INT2_STATUS, CAM_A_CTL_RAW_CCU_INT2_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_EN_
{
        volatile struct /* 0x1A0040C0 */
        {
                FIELD  AE_CCU_READ_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_CCU_CLLSN_EN                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_CCU_READ_EN                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_CLLSN_EN                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDI_DONE_EN                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  FLKO_ERR_EN                           :  1;      /* 16..16, 0x00010000 */
                FIELD  LMVO_ERR_EN                           :  1;      /* 17..17, 0x00020000 */
                FIELD  RSSO_ERR_EN                           :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_EN;   /* CAM_CTL_RAW_INT3_EN, CAM_A_CTL_RAW_INT3_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_STATUS_
{
        volatile struct /* 0x1A0040C4 */
        {
                FIELD  AE_CCU_READ_ST                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_CCU_CLLSN_ST                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_CCU_READ_ST                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_CLLSN_ST                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDI_DONE_ST                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  FLKO_ERR_ST                           :  1;      /* 16..16, 0x00010000 */
                FIELD  LMVO_ERR_ST                           :  1;      /* 17..17, 0x00020000 */
                FIELD  RSSO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_STATUS;   /* CAM_CTL_RAW_INT3_STATUS, CAM_A_CTL_RAW_INT3_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_STATUSX_
{
        volatile struct /* 0x1A0040C8 */
        {
                FIELD  AE_CCU_READ_ST                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_CCU_CLLSN_ST                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_CCU_READ_ST                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_CLLSN_ST                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDI_DONE_ST                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  FLKO_ERR_ST                           :  1;      /* 16..16, 0x00010000 */
                FIELD  LMVO_ERR_ST                           :  1;      /* 17..17, 0x00020000 */
                FIELD  RSSO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_STATUSX;  /* CAM_CTL_RAW_INT3_STATUSX, CAM_A_CTL_RAW_INT3_STATUSX*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT3_EN_
{
        volatile struct /* 0x1A0040D0 */
        {
                FIELD  AE_CCU_READ_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_CCU_CLLSN_EN                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_CCU_READ_EN                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_CLLSN_EN                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDI_DONE_EN                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  FLKO_ERR_EN                           :  1;      /* 16..16, 0x00010000 */
                FIELD  LMVO_ERR_EN                           :  1;      /* 17..17, 0x00020000 */
                FIELD  RSSO_ERR_EN                           :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT3_EN;   /* CAM_CTL_RAW_CCU_INT3_EN, CAM_A_CTL_RAW_CCU_INT3_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT3_STATUS_
{
        volatile struct /* 0x1A0040D4 */
        {
                FIELD  AE_CCU_READ_ST                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_CCU_CLLSN_ST                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_CCU_READ_ST                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_CLLSN_ST                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDI_DONE_ST                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  FLKO_ERR_ST                           :  1;      /* 16..16, 0x00010000 */
                FIELD  LMVO_ERR_ST                           :  1;      /* 17..17, 0x00020000 */
                FIELD  RSSO_ERR_ST                           :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT3_STATUS;   /* CAM_CTL_RAW_CCU_INT3_STATUS, CAM_A_CTL_RAW_CCU_INT3_STATUS*/

typedef volatile union _CAM_REG_CTL_UNI_B_DONE_SEL_
{
        volatile struct /* 0x1A0040D8 */
        {
                FIELD  rsv_0                                 :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RSSO_B_DONE_SEL                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  1;      /*  2.. 2, 0x00000004 */
                FIELD  LMVO_B_DONE_SEL                       :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLKO_B_DONE_SEL                       :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  4;      /*  5.. 8, 0x000001E0 */
                FIELD  RSSO_D_DONE_SEL                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  rsv_10                                :  1;      /* 10..10, 0x00000400 */
                FIELD  LMVO_D_DONE_SEL                       :  1;      /* 11..11, 0x00000800 */
                FIELD  FLKO_D_DONE_SEL                       :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_UNI_B_DONE_SEL;    /* CAM_CTL_UNI_B_DONE_SEL, CAM_A_CTL_UNI_B_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_RAW2_DCM_DIS_
{
        volatile struct /* 0x1A0040E0 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  CPN_DCM_DIS                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DCPN_DCM_DIS                          :  1;      /*  9.. 9, 0x00000200 */
                FIELD  ADBS_DCM_DIS                          :  1;      /* 10..10, 0x00000400 */
                FIELD  BMX2_DCM_DIS                          :  1;      /* 11..11, 0x00000800 */
                FIELD  RSS_DCM_DIS                           :  1;      /* 12..12, 0x00001000 */
                FIELD  LMV_DCM_DIS                           :  1;      /* 13..13, 0x00002000 */
                FIELD  HDS_DCM_DIS                           :  1;      /* 14..14, 0x00004000 */
                FIELD  FLK_DCM_DIS                           :  1;      /* 15..15, 0x00008000 */
                FIELD  SGG3_DCM_DIS                          :  1;      /* 16..16, 0x00010000 */
                FIELD  PSB_DCM_DIS                           :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG2_DCM_DIS                          :  1;      /* 18..18, 0x00040000 */
                FIELD  PDE_DCM_DIS                           :  1;      /* 19..19, 0x00080000 */
                FIELD  GSE_DCM_DIS                           :  1;      /* 20..20, 0x00100000 */
                FIELD  PCP_DCM_DIS                           :  1;      /* 21..21, 0x00200000 */
                FIELD  RMB_DCM_DIS                           :  1;      /* 22..22, 0x00400000 */
                FIELD  PS_DCM_DIS                            :  1;      /* 23..23, 0x00800000 */
                FIELD  HLR_DCM_DIS                           :  1;      /* 24..24, 0x01000000 */
                FIELD  AMX_DCM_DIS                           :  1;      /* 25..25, 0x02000000 */
                FIELD  SL2F_DCM_DIS                          :  1;      /* 26..26, 0x04000000 */
                FIELD  VBN_DCM_DIS                           :  1;      /* 27..27, 0x08000000 */
                FIELD  SL2J_DCM_DIS                          :  1;      /* 28..28, 0x10000000 */
                FIELD  STM_DCM_DIS                           :  1;      /* 29..29, 0x20000000 */
                FIELD  SCM_DCM_DIS                           :  1;      /* 30..30, 0x40000000 */
                FIELD  SGG5_DCM_DIS                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW2_DCM_DIS;  /* CAM_CTL_RAW2_DCM_DIS, CAM_A_CTL_RAW2_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_RAW2_DCM_STATUS_
{
        volatile struct /* 0x1A0040E4 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  CPN_DCM_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DCPN_DCM_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  ADBS_DCM_STATUS                       :  1;      /* 10..10, 0x00000400 */
                FIELD  BMX2_DCM_STATUS                       :  1;      /* 11..11, 0x00000800 */
                FIELD  RSS_DCM_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  LMV_DCM_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  HDS_DCM_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  FLK_DCM_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  SGG3_DCM_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  PSB_DCM_STATUS                        :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG2_DCM_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  GSE_DCM_STATUS                        :  1;      /* 20..20, 0x00100000 */
                FIELD  PCP_DCM_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  RMB_DCM_STATUS                        :  1;      /* 22..22, 0x00400000 */
                FIELD  PS_DCM_STATUS                         :  1;      /* 23..23, 0x00800000 */
                FIELD  HLR_DCM_STATUS                        :  1;      /* 24..24, 0x01000000 */
                FIELD  AMX_DCM_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  SL2F_DCM_STATUS                       :  1;      /* 26..26, 0x04000000 */
                FIELD  VBN_DCM_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  SL2J_DCM_STATUS                       :  1;      /* 28..28, 0x10000000 */
                FIELD  STM_DCM_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  SCM_DCM_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SGG5_DCM_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW2_DCM_STATUS;   /* CAM_CTL_RAW2_DCM_STATUS, CAM_A_CTL_RAW2_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW2_REQ_STATUS_
{
        volatile struct /* 0x1A0040E8 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  CPN_REQ_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DCPN_REQ_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  ADBS_REQ_STATUS                       :  1;      /* 10..10, 0x00000400 */
                FIELD  BMX2_REQ_STATUS                       :  1;      /* 11..11, 0x00000800 */
                FIELD  RSS_REQ_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  LMV_REQ_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  HDS_REQ_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  FLK_REQ_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  SGG3_REQ_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  PSB_REQ_STATUS                        :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG2_REQ_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  PDE_REQ_STATUS                        :  1;      /* 19..19, 0x00080000 */
                FIELD  GSE_REQ_STATUS                        :  1;      /* 20..20, 0x00100000 */
                FIELD  PCP_REQ_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  RMB_REQ_STATUS                        :  1;      /* 22..22, 0x00400000 */
                FIELD  PS_REQ_STATUS                         :  1;      /* 23..23, 0x00800000 */
                FIELD  HLR_REQ_STATUS                        :  1;      /* 24..24, 0x01000000 */
                FIELD  AMX_REQ_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  SL2F_REQ_STATUS                       :  1;      /* 26..26, 0x04000000 */
                FIELD  VBN_REQ_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  SL2J_REQ_STATUS                       :  1;      /* 28..28, 0x10000000 */
                FIELD  STM_REQ_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  SCM_REQ_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SGG5_REQ_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW2_REQ_STATUS;   /* CAM_CTL_RAW2_REQ_STATUS, CAM_A_CTL_RAW2_REQ_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW2_RDY_STATUS_
{
        volatile struct /* 0x1A0040EC */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  CPN_RDY_STATUS                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DCPN_RDY_STATUS                       :  1;      /*  9.. 9, 0x00000200 */
                FIELD  ADBS_RDY_STATUS                       :  1;      /* 10..10, 0x00000400 */
                FIELD  BMX2_RDY_STATUS                       :  1;      /* 11..11, 0x00000800 */
                FIELD  RSS_RDY_STATUS                        :  1;      /* 12..12, 0x00001000 */
                FIELD  LMV_RDY_STATUS                        :  1;      /* 13..13, 0x00002000 */
                FIELD  HDS_RDY_STATUS                        :  1;      /* 14..14, 0x00004000 */
                FIELD  FLK_RDY_STATUS                        :  1;      /* 15..15, 0x00008000 */
                FIELD  SGG3_RDY_STATUS                       :  1;      /* 16..16, 0x00010000 */
                FIELD  PSB_RDY_STATUS                        :  1;      /* 17..17, 0x00020000 */
                FIELD  SGG2_RDY_STATUS                       :  1;      /* 18..18, 0x00040000 */
                FIELD  PDE_RDY_STATUS                        :  1;      /* 19..19, 0x00080000 */
                FIELD  GSE_RDY_STATUS                        :  1;      /* 20..20, 0x00100000 */
                FIELD  PCP_RDY_STATUS                        :  1;      /* 21..21, 0x00200000 */
                FIELD  RMB_RDY_STATUS                        :  1;      /* 22..22, 0x00400000 */
                FIELD  PS_RDY_STATUS                         :  1;      /* 23..23, 0x00800000 */
                FIELD  HLR_RDY_STATUS                        :  1;      /* 24..24, 0x01000000 */
                FIELD  AMX_RDY_STATUS                        :  1;      /* 25..25, 0x02000000 */
                FIELD  SL2F_RDY_STATUS                       :  1;      /* 26..26, 0x04000000 */
                FIELD  VBN_RDY_STATUS                        :  1;      /* 27..27, 0x08000000 */
                FIELD  SL2J_RDY_STATUS                       :  1;      /* 28..28, 0x10000000 */
                FIELD  STM_RDY_STATUS                        :  1;      /* 29..29, 0x20000000 */
                FIELD  SCM_RDY_STATUS                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SGG5_RDY_STATUS                       :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CTL_RAW2_RDY_STATUS;   /* CAM_CTL_RAW2_RDY_STATUS, CAM_A_CTL_RAW2_RDY_STATUS*/

typedef volatile union _CAM_REG_FBC_IMGO_CTL1_
{
        volatile struct /* 0x1A004110 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_IMGO_CTL1; /* CAM_FBC_IMGO_CTL1, CAM_A_FBC_IMGO_CTL1*/

typedef volatile union _CAM_REG_FBC_IMGO_CTL2_
{
        volatile struct /* 0x1A004114 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_IMGO_CTL2; /* CAM_FBC_IMGO_CTL2, CAM_A_FBC_IMGO_CTL2*/

typedef volatile union _CAM_REG_FBC_RRZO_CTL1_
{
        volatile struct /* 0x1A004118 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_RRZO_CTL1; /* CAM_FBC_RRZO_CTL1, CAM_A_FBC_RRZO_CTL1*/

typedef volatile union _CAM_REG_FBC_RRZO_CTL2_
{
        volatile struct /* 0x1A00411C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_RRZO_CTL2; /* CAM_FBC_RRZO_CTL2, CAM_A_FBC_RRZO_CTL2*/

typedef volatile union _CAM_REG_FBC_UFEO_CTL1_
{
        volatile struct /* 0x1A004120 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_UFEO_CTL1; /* CAM_FBC_UFEO_CTL1, CAM_A_FBC_UFEO_CTL1*/

typedef volatile union _CAM_REG_FBC_UFEO_CTL2_
{
        volatile struct /* 0x1A004124 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_UFEO_CTL2; /* CAM_FBC_UFEO_CTL2, CAM_A_FBC_UFEO_CTL2*/

typedef volatile union _CAM_REG_FBC_LCSO_CTL1_
{
        volatile struct /* 0x1A004128 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_LCSO_CTL1; /* CAM_FBC_LCSO_CTL1, CAM_A_FBC_LCSO_CTL1*/

typedef volatile union _CAM_REG_FBC_LCSO_CTL2_
{
        volatile struct /* 0x1A00412C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_LCSO_CTL2; /* CAM_FBC_LCSO_CTL2, CAM_A_FBC_LCSO_CTL2*/

typedef volatile union _CAM_REG_FBC_AFO_CTL1_
{
        volatile struct /* 0x1A004130 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_AFO_CTL1;  /* CAM_FBC_AFO_CTL1, CAM_A_FBC_AFO_CTL1*/

typedef volatile union _CAM_REG_FBC_AFO_CTL2_
{
        volatile struct /* 0x1A004134 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_AFO_CTL2;  /* CAM_FBC_AFO_CTL2, CAM_A_FBC_AFO_CTL2*/

typedef volatile union _CAM_REG_FBC_AAO_CTL1_
{
        volatile struct /* 0x1A004138 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_AAO_CTL1;  /* CAM_FBC_AAO_CTL1, CAM_A_FBC_AAO_CTL1*/

typedef volatile union _CAM_REG_FBC_AAO_CTL2_
{
        volatile struct /* 0x1A00413C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_AAO_CTL2;  /* CAM_FBC_AAO_CTL2, CAM_A_FBC_AAO_CTL2*/

typedef volatile union _CAM_REG_FBC_PDO_CTL1_
{
        volatile struct /* 0x1A004140 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_PDO_CTL1;  /* CAM_FBC_PDO_CTL1, CAM_A_FBC_PDO_CTL1*/

typedef volatile union _CAM_REG_FBC_PDO_CTL2_
{
        volatile struct /* 0x1A004144 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_PDO_CTL2;  /* CAM_FBC_PDO_CTL2, CAM_A_FBC_PDO_CTL2*/

typedef volatile union _CAM_REG_FBC_PSO_CTL1_
{
        volatile struct /* 0x1A004148 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_PSO_CTL1;  /* CAM_FBC_PSO_CTL1, CAM_A_FBC_PSO_CTL1*/

typedef volatile union _CAM_REG_FBC_PSO_CTL2_
{
        volatile struct /* 0x1A00414C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_PSO_CTL2;  /* CAM_FBC_PSO_CTL2, CAM_A_FBC_PSO_CTL2*/

typedef volatile union _CAM_REG_FBC_FLKO_CTL1_
{
        volatile struct /* 0x1A004150 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_FLKO_CTL1; /* CAM_FBC_FLKO_CTL1, CAM_A_FBC_FLKO_CTL1*/

typedef volatile union _CAM_REG_FBC_FLKO_CTL2_
{
        volatile struct /* 0x1A004154 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_FLKO_CTL2; /* CAM_FBC_FLKO_CTL2, CAM_A_FBC_FLKO_CTL2*/

typedef volatile union _CAM_REG_FBC_LMVO_CTL1_
{
        volatile struct /* 0x1A004158 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_LMVO_CTL1; /* CAM_FBC_LMVO_CTL1, CAM_A_FBC_LMVO_CTL1*/

typedef volatile union _CAM_REG_FBC_LMVO_CTL2_
{
        volatile struct /* 0x1A00415C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_LMVO_CTL2; /* CAM_FBC_LMVO_CTL2, CAM_A_FBC_LMVO_CTL2*/

typedef volatile union _CAM_REG_FBC_RSSO_CTL1_
{
        volatile struct /* 0x1A004160 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_RSSO_CTL1; /* CAM_FBC_RSSO_CTL1, CAM_A_FBC_RSSO_CTL1*/

typedef volatile union _CAM_REG_FBC_RSSO_CTL2_
{
        volatile struct /* 0x1A004164 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_RSSO_CTL2; /* CAM_FBC_RSSO_CTL2, CAM_A_FBC_RSSO_CTL2*/

typedef volatile union _CAM_REG_FBC_UFGO_CTL1_
{
        volatile struct /* 0x1A004168 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FBC_RESET                             :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  6;      /*  9..14, 0x00007E00 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_UFGO_CTL1; /* CAM_FBC_UFGO_CTL1, CAM_A_FBC_UFGO_CTL1*/

typedef volatile union _CAM_REG_FBC_UFGO_CTL2_
{
        volatile struct /* 0x1A00416C */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FBC_UFGO_CTL2; /* CAM_FBC_UFGO_CTL2, CAM_A_FBC_UFGO_CTL2*/

typedef volatile union _CAM_REG_CQ_EN_
{
        volatile struct /* 0x1A004190 */
        {
                FIELD  CQ_APB_2T                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  CQ_DROP_FRAME_EN                      :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  CQ_DB_EN                              :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  CQ_DB_LOAD_MODE                       :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  CQ_RESET                              :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_EN; /* CAM_CQ_EN, CAM_A_CQ_EN*/

typedef volatile union _CAM_REG_CQ_THR0_CTL_
{
        volatile struct /* 0x1A004194 */
        {
                FIELD  CQ_THR0_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR0_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR0_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR0_CTL;   /* CAM_CQ_THR0_CTL, CAM_A_CQ_THR0_CTL*/

typedef volatile union _CAM_REG_CQ_THR0_BASEADDR_
{
        volatile struct /* 0x1A004198 */
        {
                FIELD  CQ_THR0_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR0_BASEADDR;  /* CAM_CQ_THR0_BASEADDR, CAM_A_CQ_THR0_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR0_DESC_SIZE_
{
        volatile struct /* 0x1A00419C */
        {
                FIELD  CQ_THR0_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR0_DESC_SIZE; /* CAM_CQ_THR0_DESC_SIZE, CAM_A_CQ_THR0_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR1_CTL_
{
        volatile struct /* 0x1A0041A0 */
        {
                FIELD  CQ_THR1_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR1_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR1_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR1_CTL;   /* CAM_CQ_THR1_CTL, CAM_A_CQ_THR1_CTL*/

typedef volatile union _CAM_REG_CQ_THR1_BASEADDR_
{
        volatile struct /* 0x1A0041A4 */
        {
                FIELD  CQ_THR1_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR1_BASEADDR;  /* CAM_CQ_THR1_BASEADDR, CAM_A_CQ_THR1_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR1_DESC_SIZE_
{
        volatile struct /* 0x1A0041A8 */
        {
                FIELD  CQ_THR1_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR1_DESC_SIZE; /* CAM_CQ_THR1_DESC_SIZE, CAM_A_CQ_THR1_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR2_CTL_
{
        volatile struct /* 0x1A0041AC */
        {
                FIELD  CQ_THR2_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR2_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR2_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR2_CTL;   /* CAM_CQ_THR2_CTL, CAM_A_CQ_THR2_CTL*/

typedef volatile union _CAM_REG_CQ_THR2_BASEADDR_
{
        volatile struct /* 0x1A0041B0 */
        {
                FIELD  CQ_THR2_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR2_BASEADDR;  /* CAM_CQ_THR2_BASEADDR, CAM_A_CQ_THR2_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR2_DESC_SIZE_
{
        volatile struct /* 0x1A0041B4 */
        {
                FIELD  CQ_THR2_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR2_DESC_SIZE; /* CAM_CQ_THR2_DESC_SIZE, CAM_A_CQ_THR2_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR3_CTL_
{
        volatile struct /* 0x1A0041B8 */
        {
                FIELD  CQ_THR3_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR3_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR3_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR3_CTL;   /* CAM_CQ_THR3_CTL, CAM_A_CQ_THR3_CTL*/

typedef volatile union _CAM_REG_CQ_THR3_BASEADDR_
{
        volatile struct /* 0x1A0041BC */
        {
                FIELD  CQ_THR3_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR3_BASEADDR;  /* CAM_CQ_THR3_BASEADDR, CAM_A_CQ_THR3_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR3_DESC_SIZE_
{
        volatile struct /* 0x1A0041C0 */
        {
                FIELD  CQ_THR3_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR3_DESC_SIZE; /* CAM_CQ_THR3_DESC_SIZE, CAM_A_CQ_THR3_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR4_CTL_
{
        volatile struct /* 0x1A0041C4 */
        {
                FIELD  CQ_THR4_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR4_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR4_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR4_CTL;   /* CAM_CQ_THR4_CTL, CAM_A_CQ_THR4_CTL*/

typedef volatile union _CAM_REG_CQ_THR4_BASEADDR_
{
        volatile struct /* 0x1A0041C8 */
        {
                FIELD  CQ_THR4_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR4_BASEADDR;  /* CAM_CQ_THR4_BASEADDR, CAM_A_CQ_THR4_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR4_DESC_SIZE_
{
        volatile struct /* 0x1A0041CC */
        {
                FIELD  CQ_THR4_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR4_DESC_SIZE; /* CAM_CQ_THR4_DESC_SIZE, CAM_A_CQ_THR4_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR5_CTL_
{
        volatile struct /* 0x1A0041D0 */
        {
                FIELD  CQ_THR5_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR5_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR5_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR5_CTL;   /* CAM_CQ_THR5_CTL, CAM_A_CQ_THR5_CTL*/

typedef volatile union _CAM_REG_CQ_THR5_BASEADDR_
{
        volatile struct /* 0x1A0041D4 */
        {
                FIELD  CQ_THR5_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR5_BASEADDR;  /* CAM_CQ_THR5_BASEADDR, CAM_A_CQ_THR5_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR5_DESC_SIZE_
{
        volatile struct /* 0x1A0041D8 */
        {
                FIELD  CQ_THR5_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR5_DESC_SIZE; /* CAM_CQ_THR5_DESC_SIZE, CAM_A_CQ_THR5_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR6_CTL_
{
        volatile struct /* 0x1A0041DC */
        {
                FIELD  CQ_THR6_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR6_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR6_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR6_CTL;   /* CAM_CQ_THR6_CTL, CAM_A_CQ_THR6_CTL*/

typedef volatile union _CAM_REG_CQ_THR6_BASEADDR_
{
        volatile struct /* 0x1A0041E0 */
        {
                FIELD  CQ_THR6_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR6_BASEADDR;  /* CAM_CQ_THR6_BASEADDR, CAM_A_CQ_THR6_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR6_DESC_SIZE_
{
        volatile struct /* 0x1A0041E4 */
        {
                FIELD  CQ_THR6_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR6_DESC_SIZE; /* CAM_CQ_THR6_DESC_SIZE, CAM_A_CQ_THR6_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR7_CTL_
{
        volatile struct /* 0x1A0041E8 */
        {
                FIELD  CQ_THR7_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR7_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR7_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR7_CTL;   /* CAM_CQ_THR7_CTL, CAM_A_CQ_THR7_CTL*/

typedef volatile union _CAM_REG_CQ_THR7_BASEADDR_
{
        volatile struct /* 0x1A0041EC */
        {
                FIELD  CQ_THR7_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR7_BASEADDR;  /* CAM_CQ_THR7_BASEADDR, CAM_A_CQ_THR7_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR7_DESC_SIZE_
{
        volatile struct /* 0x1A0041F0 */
        {
                FIELD  CQ_THR7_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR7_DESC_SIZE; /* CAM_CQ_THR7_DESC_SIZE, CAM_A_CQ_THR7_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR8_CTL_
{
        volatile struct /* 0x1A0041F4 */
        {
                FIELD  CQ_THR8_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR8_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR8_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR8_CTL;   /* CAM_CQ_THR8_CTL, CAM_A_CQ_THR8_CTL*/

typedef volatile union _CAM_REG_CQ_THR8_BASEADDR_
{
        volatile struct /* 0x1A0041F8 */
        {
                FIELD  CQ_THR8_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR8_BASEADDR;  /* CAM_CQ_THR8_BASEADDR, CAM_A_CQ_THR8_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR8_DESC_SIZE_
{
        volatile struct /* 0x1A0041FC */
        {
                FIELD  CQ_THR8_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR8_DESC_SIZE; /* CAM_CQ_THR8_DESC_SIZE, CAM_A_CQ_THR8_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR9_CTL_
{
        volatile struct /* 0x1A004200 */
        {
                FIELD  CQ_THR9_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR9_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR9_DONE_SEL                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR9_CTL;   /* CAM_CQ_THR9_CTL, CAM_A_CQ_THR9_CTL*/

typedef volatile union _CAM_REG_CQ_THR9_BASEADDR_
{
        volatile struct /* 0x1A004204 */
        {
                FIELD  CQ_THR9_BASEADDR                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR9_BASEADDR;  /* CAM_CQ_THR9_BASEADDR, CAM_A_CQ_THR9_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR9_DESC_SIZE_
{
        volatile struct /* 0x1A004208 */
        {
                FIELD  CQ_THR9_DESC_SIZE                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR9_DESC_SIZE; /* CAM_CQ_THR9_DESC_SIZE, CAM_A_CQ_THR9_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR10_CTL_
{
        volatile struct /* 0x1A00420C */
        {
                FIELD  CQ_THR10_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR10_MODE                         :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR10_DONE_SEL                     :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR10_CTL;  /* CAM_CQ_THR10_CTL, CAM_A_CQ_THR10_CTL*/

typedef volatile union _CAM_REG_CQ_THR10_BASEADDR_
{
        volatile struct /* 0x1A004210 */
        {
                FIELD  CQ_THR10_BASEADDR                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR10_BASEADDR; /* CAM_CQ_THR10_BASEADDR, CAM_A_CQ_THR10_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR10_DESC_SIZE_
{
        volatile struct /* 0x1A004214 */
        {
                FIELD  CQ_THR10_DESC_SIZE                    : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR10_DESC_SIZE;    /* CAM_CQ_THR10_DESC_SIZE, CAM_A_CQ_THR10_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR11_CTL_
{
        volatile struct /* 0x1A004218 */
        {
                FIELD  CQ_THR11_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR11_MODE                         :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR11_DONE_SEL                     :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR11_CTL;  /* CAM_CQ_THR11_CTL, CAM_A_CQ_THR11_CTL*/

typedef volatile union _CAM_REG_CQ_THR11_BASEADDR_
{
        volatile struct /* 0x1A00421C */
        {
                FIELD  CQ_THR11_BASEADDR                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR11_BASEADDR; /* CAM_CQ_THR11_BASEADDR, CAM_A_CQ_THR11_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR11_DESC_SIZE_
{
        volatile struct /* 0x1A004220 */
        {
                FIELD  CQ_THR11_DESC_SIZE                    : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR11_DESC_SIZE;    /* CAM_CQ_THR11_DESC_SIZE, CAM_A_CQ_THR11_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR12_CTL_
{
        volatile struct /* 0x1A004224 */
        {
                FIELD  CQ_THR12_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  CQ_THR12_MODE                         :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  CQ_THR12_DONE_SEL                     :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR12_CTL;  /* CAM_CQ_THR12_CTL, CAM_A_CQ_THR12_CTL*/

typedef volatile union _CAM_REG_CQ_THR12_BASEADDR_
{
        volatile struct /* 0x1A004228 */
        {
                FIELD  CQ_THR12_BASEADDR                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR12_BASEADDR; /* CAM_CQ_THR12_BASEADDR, CAM_A_CQ_THR12_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR12_DESC_SIZE_
{
        volatile struct /* 0x1A00422C */
        {
                FIELD  CQ_THR12_DESC_SIZE                    : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ_THR12_DESC_SIZE;    /* CAM_CQ_THR12_DESC_SIZE, CAM_A_CQ_THR12_DESC_SIZE*/

typedef volatile union _CAM_REG_TG_SEN_MODE_
{
        volatile struct /* 0x1A004230 */
        {
                FIELD  CMOS_EN                               :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DBL_DATA_BUS                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SOT_MODE                              :  1;      /*  2.. 2, 0x00000004 */
                FIELD  SOT_CLR_MODE                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  SOF_SRC                               :  2;      /*  8.. 9, 0x00000300 */
                FIELD  EOF_SRC                               :  2;      /* 10..11, 0x00000C00 */
                FIELD  PXL_CNT_RST_SRC                       :  1;      /* 12..12, 0x00001000 */
                FIELD  DBL_DATA_BUS1                         :  1;      /* 13..13, 0x00002000 */
                FIELD  SOF_WAIT_CQ                           :  1;      /* 14..14, 0x00004000 */
                FIELD  FIFO_FULL_CTL_EN                      :  1;      /* 15..15, 0x00008000 */
                FIELD  TIME_STP_EN                           :  1;      /* 16..16, 0x00010000 */
                FIELD  VS_SUB_EN                             :  1;      /* 17..17, 0x00020000 */
                FIELD  SOF_SUB_EN                            :  1;      /* 18..18, 0x00040000 */
                FIELD  I2C_CQ_EN                             :  1;      /* 19..19, 0x00080000 */
                FIELD  EOF_ALS_RDY_EN                        :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_SEL                                :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                : 10;      /* 22..31, 0xFFC00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SEN_MODE;   /* CAM_TG_SEN_MODE, CAM_A_TG_SEN_MODE*/

typedef volatile union _CAM_REG_TG_VF_CON_
{
        volatile struct /* 0x1A004234 */
        {
                FIELD  VFDATA_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SINGLE_MODE                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FR_CON                                :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  SP_DELAY                              :  3;      /*  8..10, 0x00000700 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  SPDELAY_MODE                          :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_VF_CON; /* CAM_TG_VF_CON, CAM_A_TG_VF_CON*/

typedef volatile union _CAM_REG_TG_SEN_GRAB_PXL_
{
        volatile struct /* 0x1A004238 */
        {
                FIELD  PXL_S                                 : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  PXL_E                                 : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_PXL;   /* CAM_TG_SEN_GRAB_PXL, CAM_A_TG_SEN_GRAB_PXL*/

typedef volatile union _CAM_REG_TG_SEN_GRAB_LIN_
{
        volatile struct /* 0x1A00423C */
        {
                FIELD  LIN_S                                 : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LIN_E                                 : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_LIN;   /* CAM_TG_SEN_GRAB_LIN, CAM_A_TG_SEN_GRAB_LIN*/

typedef volatile union _CAM_REG_TG_PATH_CFG_
{
        volatile struct /* 0x1A004240 */
        {
                FIELD  SEN_IN_LSB                            :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  JPGINF_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  MEMIN_EN                              :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  1;      /*  6.. 6, 0x00000040 */
                FIELD  JPG_LINEND_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  DB_LOAD_DIS                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DB_LOAD_SRC                           :  1;      /*  9.. 9, 0x00000200 */
                FIELD  DB_LOAD_VSPOL                         :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  YUV_U2S_DIS                           :  1;      /* 12..12, 0x00001000 */
                FIELD  YUV_BIN_EN                            :  1;      /* 13..13, 0x00002000 */
                FIELD  TG_ERR_SEL                            :  1;      /* 14..14, 0x00004000 */
                FIELD  TG_FULL_SEL                           :  1;      /* 15..15, 0x00008000 */
                FIELD  TG_FULL_SEL2                          :  1;      /* 16..16, 0x00010000 */
                FIELD  FLUSH_DISABLE                         :  1;      /* 17..17, 0x00020000 */
                FIELD  INT_BANK_DISABLE                      :  1;      /* 18..18, 0x00040000 */
                FIELD  EXP_ESC                               :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_PATH_CFG;   /* CAM_TG_PATH_CFG, CAM_A_TG_PATH_CFG*/

typedef volatile union _CAM_REG_TG_MEMIN_CTL_
{
        volatile struct /* 0x1A004244 */
        {
                FIELD  MEMIN_DUMMYPXL                        :  8;      /*  0.. 7, 0x000000FF */
                FIELD  MEMIN_DUMMYLIN                        :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_MEMIN_CTL;  /* CAM_TG_MEMIN_CTL, CAM_A_TG_MEMIN_CTL*/

typedef volatile union _CAM_REG_TG_INT1_
{
        volatile struct /* 0x1A004248 */
        {
                FIELD  TG_INT1_LINENO                        : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  TG_INT1_PXLNO                         : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  VSYNC_INT_POL                         :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_INT1;   /* CAM_TG_INT1, CAM_A_TG_INT1*/

typedef volatile union _CAM_REG_TG_INT2_
{
        volatile struct /* 0x1A00424C */
        {
                FIELD  TG_INT2_LINENO                        : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  TG_INT2_PXLNO                         : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_INT2;   /* CAM_TG_INT2, CAM_A_TG_INT2*/

typedef volatile union _CAM_REG_TG_SOF_CNT_
{
        volatile struct /* 0x1A004250 */
        {
                FIELD  SOF_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SOF_CNT;    /* CAM_TG_SOF_CNT, CAM_A_TG_SOF_CNT*/

typedef volatile union _CAM_REG_TG_SOT_CNT_
{
        volatile struct /* 0x1A004254 */
        {
                FIELD  SOT_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SOT_CNT;    /* CAM_TG_SOT_CNT, CAM_A_TG_SOT_CNT*/

typedef volatile union _CAM_REG_TG_EOT_CNT_
{
        volatile struct /* 0x1A004258 */
        {
                FIELD  EOT_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_EOT_CNT;    /* CAM_TG_EOT_CNT, CAM_A_TG_EOT_CNT*/

typedef volatile union _CAM_REG_TG_ERR_CTL_
{
        volatile struct /* 0x1A00425C */
        {
                FIELD  GRAB_ERR_FLIMIT_NO                    :  4;      /*  0.. 3, 0x0000000F */
                FIELD  GRAB_ERR_FLIMIT_EN                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  GRAB_ERR_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;      /*  8..11, 0x00000F00 */
                FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  DBG_SRC_SEL                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_ERR_CTL;    /* CAM_TG_ERR_CTL, CAM_A_TG_ERR_CTL*/

typedef volatile union _CAM_REG_TG_DAT_NO_
{
        volatile struct /* 0x1A004260 */
        {
                FIELD  DAT_NO                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_DAT_NO; /* CAM_TG_DAT_NO, CAM_A_TG_DAT_NO*/

typedef volatile union _CAM_REG_TG_FRM_CNT_ST_
{
        volatile struct /* 0x1A004264 */
        {
                FIELD  REZ_OVRUN_FCNT                        :  4;      /*  0.. 3, 0x0000000F */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  GRAB_ERR_FCNT                         :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FRM_CNT_ST; /* CAM_TG_FRM_CNT_ST, CAM_A_TG_FRM_CNT_ST*/

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_
{
        volatile struct /* 0x1A004268 */
        {
                FIELD  LINE_CNT                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PXL_CNT                               : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FRMSIZE_ST; /* CAM_TG_FRMSIZE_ST, CAM_A_TG_FRMSIZE_ST*/

typedef volatile union _CAM_REG_TG_INTER_ST_
{
        volatile struct /* 0x1A00426C */
        {
                FIELD  SYN_VF_DATA_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  OUT_RDY                               :  1;      /*  1.. 1, 0x00000002 */
                FIELD  OUT_REQ                               :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  5;      /*  3.. 7, 0x000000F8 */
                FIELD  TG_CAM_CS                             :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  CAM_FRM_CNT                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_INTER_ST;   /* CAM_TG_INTER_ST, CAM_A_TG_INTER_ST*/

typedef volatile union _CAM_REG_TG_FLASHA_CTL_
{
        volatile struct /* 0x1A004270 */
        {
                FIELD  FLASHA_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FLASH_EN                              :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FLASHA_STARTPNT                       :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FLASHA_END_FRM                        :  3;      /*  8..10, 0x00000700 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  FLASH_POL                             :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHA_CTL; /* CAM_TG_FLASHA_CTL, CAM_A_TG_FLASHA_CTL*/

typedef volatile union _CAM_REG_TG_FLASHA_LINE_CNT_
{
        volatile struct /* 0x1A004274 */
        {
                FIELD  FLASHA_LUNIT                          : 20;      /*  0..19, 0x000FFFFF */
                FIELD  FLASHA_LUNIT_NO                       :  4;      /* 20..23, 0x00F00000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHA_LINE_CNT;    /* CAM_TG_FLASHA_LINE_CNT, CAM_A_TG_FLASHA_LINE_CNT*/

typedef volatile union _CAM_REG_TG_FLASHA_POS_
{
        volatile struct /* 0x1A004278 */
        {
                FIELD  FLASHA_PXL                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FLASHA_LINE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHA_POS; /* CAM_TG_FLASHA_POS, CAM_A_TG_FLASHA_POS*/

typedef volatile union _CAM_REG_TG_FLASHB_CTL_
{
        volatile struct /* 0x1A00427C */
        {
                FIELD  FLASHB_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FLASHB_TRIG_SRC                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FLASHB_STARTPNT                       :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FLASHB_START_FRM                      :  4;      /*  8..11, 0x00000F00 */
                FIELD  FLASHB_CONT_FRM                       :  3;      /* 12..14, 0x00007000 */
                FIELD  rsv_15                                : 17;      /* 15..31, 0xFFFF8000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHB_CTL; /* CAM_TG_FLASHB_CTL, CAM_A_TG_FLASHB_CTL*/

typedef volatile union _CAM_REG_TG_FLASHB_LINE_CNT_
{
        volatile struct /* 0x1A004280 */
        {
                FIELD  FLASHB_LUNIT                          : 20;      /*  0..19, 0x000FFFFF */
                FIELD  FLASHB_LUNIT_NO                       :  4;      /* 20..23, 0x00F00000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHB_LINE_CNT;    /* CAM_TG_FLASHB_LINE_CNT, CAM_A_TG_FLASHB_LINE_CNT*/

typedef volatile union _CAM_REG_TG_FLASHB_POS_
{
        volatile struct /* 0x1A004284 */
        {
                FIELD  FLASHB_PXL                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FLASHB_LINE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHB_POS; /* CAM_TG_FLASHB_POS, CAM_A_TG_FLASHB_POS*/

typedef volatile union _CAM_REG_TG_FLASHB_POS1_
{
        volatile struct /* 0x1A004288 */
        {
                FIELD  FLASHB_CYC_CNT                        : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FLASHB_POS1;    /* CAM_TG_FLASHB_POS1, CAM_A_TG_FLASHB_POS1*/

typedef volatile union _CAM_REG_TG_I2C_CQ_TRIG_
{
        volatile struct /* 0x1A004290 */
        {
                FIELD  TG_I2C_CQ_TRIG                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_I2C_CQ_TRIG;    /* CAM_TG_I2C_CQ_TRIG, CAM_A_TG_I2C_CQ_TRIG*/

typedef volatile union _CAM_REG_TG_CQ_TIMING_
{
        volatile struct /* 0x1A004294 */
        {
                FIELD  TG_I2C_CQ_TIM                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_CQ_TIMING;  /* CAM_TG_CQ_TIMING, CAM_A_TG_CQ_TIMING*/

typedef volatile union _CAM_REG_TG_CQ_NUM_
{
        volatile struct /* 0x1A004298 */
        {
                FIELD  TG_CQ_NUM                             :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_CQ_NUM; /* CAM_TG_CQ_NUM, CAM_A_TG_CQ_NUM*/

typedef volatile union _CAM_REG_TG_TIME_STAMP_
{
        volatile struct /* 0x1A0042A0 */
        {
                FIELD  TG_TIME_STAMP                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_TIME_STAMP; /* CAM_TG_TIME_STAMP, CAM_A_TG_TIME_STAMP*/

typedef volatile union _CAM_REG_TG_SUB_PERIOD_
{
        volatile struct /* 0x1A0042A4 */
        {
                FIELD  VS_PERIOD                             :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  SOF_PERIOD                            :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_SUB_PERIOD; /* CAM_TG_SUB_PERIOD, CAM_A_TG_SUB_PERIOD*/

typedef volatile union _CAM_REG_TG_DAT_NO_R_
{
        volatile struct /* 0x1A0042A8 */
        {
                FIELD  DAT_NO                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_DAT_NO_R;   /* CAM_TG_DAT_NO_R, CAM_A_TG_DAT_NO_R*/

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_R_
{
        volatile struct /* 0x1A0042AC */
        {
                FIELD  LINE_CNT                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PXL_CNT                               : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_FRMSIZE_ST_R;   /* CAM_TG_FRMSIZE_ST_R, CAM_A_TG_FRMSIZE_ST_R*/

typedef volatile union _CAM_REG_TG_TIME_STAMP_CTL_
{
        volatile struct /* 0x1A0042B0 */
        {
                FIELD  TG_TIME_STAMP_SEL                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  TG_TIME_STAMP_LOCK                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_TIME_STAMP_CTL; /* CAM_TG_TIME_STAMP_CTL, CAM_A_TG_TIME_STAMP_CTL*/

typedef volatile union _CAM_REG_TG_TIME_STAMP_MSB_
{
        volatile struct /* 0x1A0042B4 */
        {
                FIELD  TG_TIME_STAMP                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_TG_TIME_STAMP_MSB; /* CAM_TG_TIME_STAMP_MSB, CAM_A_TG_TIME_STAMP_MSB*/

typedef volatile union _CAM_REG_DMX_CTL_
{
        volatile struct /* 0x1A004330 */
        {
                FIELD  DMX_SRAM_SIZE                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  DMX_EDGE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 11;      /* 20..30, 0x7FF00000 */
                FIELD  DMX_EDGE_SET                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMX_CTL;   /* CAM_DMX_CTL, CAM_A_DMX_CTL*/

typedef volatile union _CAM_REG_DMX_CROP_
{
        volatile struct /* 0x1A004334 */
        {
                FIELD  DMX_STR_X                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  DMX_END_X                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMX_CROP;  /* CAM_DMX_CROP, CAM_A_DMX_CROP*/

typedef volatile union _CAM_REG_DMX_VSIZE_
{
        volatile struct /* 0x1A004338 */
        {
                FIELD  DMX_HT                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMX_VSIZE; /* CAM_DMX_VSIZE, CAM_A_DMX_VSIZE*/

typedef volatile union _CAM_REG_RMG_HDR_CFG_
{
        volatile struct /* 0x1A004350 */
        {
                FIELD  RMG_IHDR_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RMG_IHDR_LE_FIRST                     :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  RMG_ZHDR_EN                           :  1;      /*  4.. 4, 0x00000010 */
                FIELD  RMG_ZHDR_RLE                          :  1;      /*  5.. 5, 0x00000020 */
                FIELD  RMG_ZHDR_GLE                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  RMG_ZHDR_BLE                          :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RMG_HDR_TH                            : 12;      /*  8..19, 0x000FFF00 */
                FIELD  RMG_OSC_TH                            : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMG_HDR_CFG;   /* CAM_RMG_HDR_CFG, CAM_A_RMG_HDR_CFG*/

typedef volatile union _CAM_REG_RMG_HDR_GAIN_
{
        volatile struct /* 0x1A004354 */
        {
                FIELD  RMG_HDR_GAIN                          :  9;      /*  0.. 8, 0x000001FF */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  RMG_HDR_RATIO                         :  9;      /* 16..24, 0x01FF0000 */
                FIELD  rsv_25                                :  3;      /* 25..27, 0x0E000000 */
                FIELD  RMG_LE_INV_CTL                        :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMG_HDR_GAIN;  /* CAM_RMG_HDR_GAIN, CAM_A_RMG_HDR_GAIN*/

typedef volatile union _CAM_REG_RMG_HDR_CFG2_
{
        volatile struct /* 0x1A004358 */
        {
                FIELD  RMG_HDR_THK                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMG_HDR_CFG2;  /* CAM_RMG_HDR_CFG2, CAM_A_RMG_HDR_CFG2*/

typedef volatile union _CAM_REG_RMM_OSC_
{
        volatile struct /* 0x1A004390 */
        {
                FIELD  RMM_OSC_TH                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  RMM_SEDIR_SL                          :  3;      /* 12..14, 0x00007000 */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  RMM_SEBLD_WD                          :  4;      /* 16..19, 0x000F0000 */
                FIELD  RMM_LEBLD_WD                          :  4;      /* 20..23, 0x00F00000 */
                FIELD  RMM_LE_INV_CTL                        :  4;      /* 24..27, 0x0F000000 */
                FIELD  RMM_EDGE                              :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_OSC;   /* CAM_RMM_OSC, CAM_A_RMM_OSC*/

typedef volatile union _CAM_REG_RMM_MC_
{
        volatile struct /* 0x1A004394 */
        {
                FIELD  RMM_MO_EDGE                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RMM_MO_EN                             :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RMM_MOBLD_FT                          :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RMM_MOTH_RATIO                        :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  RMM_HORI_ADDWT                        :  5;      /* 16..20, 0x001F0000 */
                FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_MC;    /* CAM_RMM_MC, CAM_A_RMM_MC*/

typedef volatile union _CAM_REG_RMM_REVG_1_
{
        volatile struct /* 0x1A004398 */
        {
                FIELD  RMM_REVG_R                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  RMM_REVG_GR                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_REVG_1;    /* CAM_RMM_REVG_1, CAM_A_RMM_REVG_1*/

typedef volatile union _CAM_REG_RMM_REVG_2_
{
        volatile struct /* 0x1A00439C */
        {
                FIELD  RMM_REVG_B                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  RMM_REVG_GB                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_REVG_2;    /* CAM_RMM_REVG_2, CAM_A_RMM_REVG_2*/

typedef volatile union _CAM_REG_RMM_LEOS_
{
        volatile struct /* 0x1A0043A0 */
        {
                FIELD  RMM_LEOS_GRAY                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_LEOS;  /* CAM_RMM_LEOS, CAM_A_RMM_LEOS*/

typedef volatile union _CAM_REG_RMM_MC2_
{
        volatile struct /* 0x1A0043A4 */
        {
                FIELD  RMM_MOSE_TH                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  RMM_MOSE_BLDWD                        :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_MC2;   /* CAM_RMM_MC2, CAM_A_RMM_MC2*/

typedef volatile union _CAM_REG_RMM_DIFF_LB_
{
        volatile struct /* 0x1A0043A8 */
        {
                FIELD  RMM_DIFF_LB                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_DIFF_LB;   /* CAM_RMM_DIFF_LB, CAM_A_RMM_DIFF_LB*/

typedef volatile union _CAM_REG_RMM_MA_
{
        volatile struct /* 0x1A0043AC */
        {
                FIELD  RMM_MASE_RATIO                        :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  RMM_OSBLD_WD                          :  4;      /*  8..11, 0x00000F00 */
                FIELD  RMM_MASE_BLDWD                        :  4;      /* 12..15, 0x0000F000 */
                FIELD  RMM_SENOS_LEFAC                       :  5;      /* 16..20, 0x001F0000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  RMM_SEYOS_LEFAC                       :  5;      /* 24..28, 0x1F000000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_MA;    /* CAM_RMM_MA, CAM_A_RMM_MA*/

typedef volatile union _CAM_REG_RMM_TUNE_
{
        volatile struct /* 0x1A0043B0 */
        {
                FIELD  RMM_PSHOR_SEEN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RMM_PS_BLUR                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  RMM_OSC_REPEN                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  RMM_SOFT_TH_EN                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  RMM_LE_LOWPA_EN                       :  1;      /*  6.. 6, 0x00000040 */
                FIELD  RMM_SE_LOWPA_EN                       :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RMM_PSSEC_ONLY                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
                FIELD  RMM_OSCLE_ONLY                        :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  RMM_PS_TH                             : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  RMM_MOLE_DIREN                        :  1;      /* 28..28, 0x10000000 */
                FIELD  RMM_MOSE_DIREN                        :  1;      /* 29..29, 0x20000000 */
                FIELD  RMM_MO_2DBLD_EN                       :  1;      /* 30..30, 0x40000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMM_TUNE;  /* CAM_RMM_TUNE, CAM_A_RMM_TUNE*/

typedef volatile union _CAM_REG_OBC_OFFST0_
{
        volatile struct /* 0x1A0043F0 */
        {
                FIELD  OBC_OFST_B                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_OFFST0;    /* CAM_OBC_OFFST0, CAM_A_OBC_OFFST0*/

typedef volatile union _CAM_REG_OBC_OFFST1_
{
        volatile struct /* 0x1A0043F4 */
        {
                FIELD  OBC_OFST_GR                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_OFFST1;    /* CAM_OBC_OFFST1, CAM_A_OBC_OFFST1*/

typedef volatile union _CAM_REG_OBC_OFFST2_
{
        volatile struct /* 0x1A0043F8 */
        {
                FIELD  OBC_OFST_GB                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_OFFST2;    /* CAM_OBC_OFFST2, CAM_A_OBC_OFFST2*/

typedef volatile union _CAM_REG_OBC_OFFST3_
{
        volatile struct /* 0x1A0043FC */
        {
                FIELD  OBC_OFST_R                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_OFFST3;    /* CAM_OBC_OFFST3, CAM_A_OBC_OFFST3*/

typedef volatile union _CAM_REG_OBC_GAIN0_
{
        volatile struct /* 0x1A004400 */
        {
                FIELD  OBC_GAIN_B                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_GAIN0; /* CAM_OBC_GAIN0, CAM_A_OBC_GAIN0*/

typedef volatile union _CAM_REG_OBC_GAIN1_
{
        volatile struct /* 0x1A004404 */
        {
                FIELD  OBC_GAIN_GR                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_GAIN1; /* CAM_OBC_GAIN1, CAM_A_OBC_GAIN1*/

typedef volatile union _CAM_REG_OBC_GAIN2_
{
        volatile struct /* 0x1A004408 */
        {
                FIELD  OBC_GAIN_GB                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_GAIN2; /* CAM_OBC_GAIN2, CAM_A_OBC_GAIN2*/

typedef volatile union _CAM_REG_OBC_GAIN3_
{
        volatile struct /* 0x1A00440C */
        {
                FIELD  OBC_GAIN_R                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_OBC_GAIN3; /* CAM_OBC_GAIN3, CAM_A_OBC_GAIN3*/

typedef volatile union _CAM_REG_BNR_BPC_CON_
{
        volatile struct /* 0x1A004420 */
        {
                FIELD  BPC_EN                                :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  BPC_LUT_EN                            :  1;      /*  4.. 4, 0x00000010 */
                FIELD  BPC_TABLE_END_MODE                    :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  BPC_AVG_MODE                          :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 :  3;      /*  9..11, 0x00000E00 */
                FIELD  BPC_DTC_MODE                          :  2;      /* 12..13, 0x00003000 */
                FIELD  BPC_CS_MODE                           :  2;      /* 14..15, 0x0000C000 */
                FIELD  BPC_CRC_MODE                          :  2;      /* 16..17, 0x00030000 */
                FIELD  BPC_EXC                               :  1;      /* 18..18, 0x00040000 */
                FIELD  BPC_BLD_MODE                          :  1;      /* 19..19, 0x00080000 */
                FIELD  BNR_LE_INV_CTL                        :  4;      /* 20..23, 0x00F00000 */
                FIELD  BNR_OSC_COUNT                         :  4;      /* 24..27, 0x0F000000 */
                FIELD  BNR_EDGE                              :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_CON;   /* CAM_BNR_BPC_CON, CAM_A_BNR_BPC_CON*/

typedef volatile union _CAM_REG_BNR_BPC_TH1_
{
        volatile struct /* 0x1A004424 */
        {
                FIELD  BPC_TH_LWB                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  BPC_TH_Y                              : 12;      /* 12..23, 0x00FFF000 */
                FIELD  BPC_BLD_SLP0                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH1;   /* CAM_BNR_BPC_TH1, CAM_A_BNR_BPC_TH1*/

typedef volatile union _CAM_REG_BNR_BPC_TH2_
{
        volatile struct /* 0x1A004428 */
        {
                FIELD  BPC_TH_UPB                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  BPC_BLD0                              :  7;      /* 16..22, 0x007F0000 */
                FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
                FIELD  BPC_BLD1                              :  7;      /* 24..30, 0x7F000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH2;   /* CAM_BNR_BPC_TH2, CAM_A_BNR_BPC_TH2*/

typedef volatile union _CAM_REG_BNR_BPC_TH3_
{
        volatile struct /* 0x1A00442C */
        {
                FIELD  BPC_TH_XA                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  BPC_TH_XB                             : 12;      /* 12..23, 0x00FFF000 */
                FIELD  BPC_TH_SLA                            :  4;      /* 24..27, 0x0F000000 */
                FIELD  BPC_TH_SLB                            :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH3;   /* CAM_BNR_BPC_TH3, CAM_A_BNR_BPC_TH3*/

typedef volatile union _CAM_REG_BNR_BPC_TH4_
{
        volatile struct /* 0x1A004430 */
        {
                FIELD  BPC_DK_TH_XA                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  BPC_DK_TH_XB                          : 12;      /* 12..23, 0x00FFF000 */
                FIELD  BPC_DK_TH_SLA                         :  4;      /* 24..27, 0x0F000000 */
                FIELD  BPC_DK_TH_SLB                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH4;   /* CAM_BNR_BPC_TH4, CAM_A_BNR_BPC_TH4*/

typedef volatile union _CAM_REG_BNR_BPC_DTC_
{
        volatile struct /* 0x1A004434 */
        {
                FIELD  BPC_RNG                               :  4;      /*  0.. 3, 0x0000000F */
                FIELD  BPC_CS_RNG                            :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  BPC_CT_LV                             :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  BPC_TH_MUL                            :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                :  4;      /* 20..23, 0x00F00000 */
                FIELD  BPC_NO_LV                             :  3;      /* 24..26, 0x07000000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_DTC;   /* CAM_BNR_BPC_DTC, CAM_A_BNR_BPC_DTC*/

typedef volatile union _CAM_REG_BNR_BPC_COR_
{
        volatile struct /* 0x1A004438 */
        {
                FIELD  BPC_DIR_MAX                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  BPC_DIR_TH                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  BPC_RANK_IDXR                         :  3;      /* 16..18, 0x00070000 */
                FIELD  BPC_RANK_IDXG                         :  3;      /* 19..21, 0x00380000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  BPC_DIR_TH2                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_COR;   /* CAM_BNR_BPC_COR, CAM_A_BNR_BPC_COR*/

typedef volatile union _CAM_REG_BNR_BPC_TBLI1_
{
        volatile struct /* 0x1A00443C */
        {
                FIELD  BPC_XOFFSET                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  BPC_YOFFSET                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI1; /* CAM_BNR_BPC_TBLI1, CAM_A_BNR_BPC_TBLI1*/

typedef volatile union _CAM_REG_BNR_BPC_TBLI2_
{
        volatile struct /* 0x1A004440 */
        {
                FIELD  BPC_XSIZE                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  BPC_YSIZE                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI2; /* CAM_BNR_BPC_TBLI2, CAM_A_BNR_BPC_TBLI2*/

typedef volatile union _CAM_REG_BNR_BPC_TH1_C_
{
        volatile struct /* 0x1A004444 */
        {
                FIELD  BPC_C_TH_LWB                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  BPC_C_TH_Y                            : 12;      /* 12..23, 0x00FFF000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH1_C; /* CAM_BNR_BPC_TH1_C, CAM_A_BNR_BPC_TH1_C*/

typedef volatile union _CAM_REG_BNR_BPC_TH2_C_
{
        volatile struct /* 0x1A004448 */
        {
                FIELD  BPC_C_TH_UPB                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  1;      /* 12..12, 0x00001000 */
                FIELD  BPC_RANK_IDXB                         :  3;      /* 13..15, 0x0000E000 */
                FIELD  BPC_BLD_LWB                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH2_C; /* CAM_BNR_BPC_TH2_C, CAM_A_BNR_BPC_TH2_C*/

typedef volatile union _CAM_REG_BNR_BPC_TH3_C_
{
        volatile struct /* 0x1A00444C */
        {
                FIELD  BPC_C_TH_XA                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  BPC_C_TH_XB                           : 12;      /* 12..23, 0x00FFF000 */
                FIELD  BPC_C_TH_SLA                          :  4;      /* 24..27, 0x0F000000 */
                FIELD  BPC_C_TH_SLB                          :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_BPC_TH3_C; /* CAM_BNR_BPC_TH3_C, CAM_A_BNR_BPC_TH3_C*/

typedef volatile union _CAM_REG_BNR_NR1_CON_
{
        volatile struct /* 0x1A004450 */
        {
                FIELD  rsv_0                                 :  4;      /*  0.. 3, 0x0000000F */
                FIELD  NR1_CT_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_NR1_CON;   /* CAM_BNR_NR1_CON, CAM_A_BNR_NR1_CON*/

typedef volatile union _CAM_REG_BNR_NR1_CT_CON_
{
        volatile struct /* 0x1A004454 */
        {
                FIELD  NR1_CT_MD                             :  2;      /*  0.. 1, 0x00000003 */
                FIELD  NR1_CT_MD2                            :  2;      /*  2.. 3, 0x0000000C */
                FIELD  NR1_CT_THRD                           : 10;      /*  4..13, 0x00003FF0 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  NR1_MBND                              : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  2;      /* 26..27, 0x0C000000 */
                FIELD  NR1_CT_SLOPE                          :  2;      /* 28..29, 0x30000000 */
                FIELD  NR1_CT_DIV                            :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_NR1_CT_CON;    /* CAM_BNR_NR1_CT_CON, CAM_A_BNR_NR1_CT_CON*/

typedef volatile union _CAM_REG_BNR_NR1_CT_CON2_
{
        volatile struct /* 0x1A004458 */
        {
                FIELD  NR1_CT_LWB                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  NR1_CT_UPB                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_NR1_CT_CON2;   /* CAM_BNR_NR1_CT_CON2, CAM_A_BNR_NR1_CT_CON2*/

typedef volatile union _CAM_REG_BNR_NR1_CT_CON3_
{
        volatile struct /* 0x1A00445C */
        {
                FIELD  NR1_CT_LSP                            :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  NR1_CT_USP                            :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_NR1_CT_CON3;   /* CAM_BNR_NR1_CT_CON3, CAM_A_BNR_NR1_CT_CON3*/

typedef volatile union _CAM_REG_BNR_PDC_CON_
{
        volatile struct /* 0x1A004460 */
        {
                FIELD  PDC_EN                                :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  PDC_CT                                :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  PDC_MODE                              :  2;      /*  8.. 9, 0x00000300 */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  PDC_OUT                               :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_CON;   /* CAM_BNR_PDC_CON, CAM_A_BNR_PDC_CON*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L0_
{
        volatile struct /* 0x1A004464 */
        {
                FIELD  PDC_GCF_L00                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_L10                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L0;   /* CAM_BNR_PDC_GAIN_L0, CAM_A_BNR_PDC_GAIN_L0*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L1_
{
        volatile struct /* 0x1A004468 */
        {
                FIELD  PDC_GCF_L01                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_L20                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L1;   /* CAM_BNR_PDC_GAIN_L1, CAM_A_BNR_PDC_GAIN_L1*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L2_
{
        volatile struct /* 0x1A00446C */
        {
                FIELD  PDC_GCF_L11                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_L02                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L2;   /* CAM_BNR_PDC_GAIN_L2, CAM_A_BNR_PDC_GAIN_L2*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L3_
{
        volatile struct /* 0x1A004470 */
        {
                FIELD  PDC_GCF_L30                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_L21                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L3;   /* CAM_BNR_PDC_GAIN_L3, CAM_A_BNR_PDC_GAIN_L3*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L4_
{
        volatile struct /* 0x1A004474 */
        {
                FIELD  PDC_GCF_L12                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_L03                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L4;   /* CAM_BNR_PDC_GAIN_L4, CAM_A_BNR_PDC_GAIN_L4*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R0_
{
        volatile struct /* 0x1A004478 */
        {
                FIELD  PDC_GCF_R00                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_R10                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R0;   /* CAM_BNR_PDC_GAIN_R0, CAM_A_BNR_PDC_GAIN_R0*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R1_
{
        volatile struct /* 0x1A00447C */
        {
                FIELD  PDC_GCF_R01                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_R20                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R1;   /* CAM_BNR_PDC_GAIN_R1, CAM_A_BNR_PDC_GAIN_R1*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R2_
{
        volatile struct /* 0x1A004480 */
        {
                FIELD  PDC_GCF_R11                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_R02                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R2;   /* CAM_BNR_PDC_GAIN_R2, CAM_A_BNR_PDC_GAIN_R2*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R3_
{
        volatile struct /* 0x1A004484 */
        {
                FIELD  PDC_GCF_R30                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_R21                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R3;   /* CAM_BNR_PDC_GAIN_R3, CAM_A_BNR_PDC_GAIN_R3*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R4_
{
        volatile struct /* 0x1A004488 */
        {
                FIELD  PDC_GCF_R12                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_R03                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R4;   /* CAM_BNR_PDC_GAIN_R4, CAM_A_BNR_PDC_GAIN_R4*/

typedef volatile union _CAM_REG_BNR_PDC_TH_GB_
{
        volatile struct /* 0x1A00448C */
        {
                FIELD  PDC_GTH                               : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_BTH                               : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_TH_GB; /* CAM_BNR_PDC_TH_GB, CAM_A_BNR_PDC_TH_GB*/

typedef volatile union _CAM_REG_BNR_PDC_TH_IA_
{
        volatile struct /* 0x1A004490 */
        {
                FIELD  PDC_ITH                               : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_ATH                               : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_TH_IA; /* CAM_BNR_PDC_TH_IA, CAM_A_BNR_PDC_TH_IA*/

typedef volatile union _CAM_REG_BNR_PDC_TH_HD_
{
        volatile struct /* 0x1A004494 */
        {
                FIELD  PDC_NTH                               : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_DTH                               : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_TH_HD; /* CAM_BNR_PDC_TH_HD, CAM_A_BNR_PDC_TH_HD*/

typedef volatile union _CAM_REG_BNR_PDC_SL_
{
        volatile struct /* 0x1A004498 */
        {
                FIELD  PDC_GSL                               :  4;      /*  0.. 3, 0x0000000F */
                FIELD  PDC_BSL                               :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  PDC_ISL                               :  4;      /*  8..11, 0x00000F00 */
                FIELD  PDC_ASL                               :  4;      /* 12..15, 0x0000F000 */
                FIELD  PDC_GCF_NORM                          :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_SL;    /* CAM_BNR_PDC_SL, CAM_A_BNR_PDC_SL*/

typedef volatile union _CAM_REG_BNR_PDC_POS_
{
        volatile struct /* 0x1A00449C */
        {
                FIELD  PDC_XCENTER                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PDC_YCENTER                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BNR_PDC_POS;   /* CAM_BNR_PDC_POS, CAM_A_BNR_PDC_POS*/

typedef volatile union _CAM_REG_STM_CFG0_
{
        volatile struct /* 0x1A0044A0 */
        {
                FIELD  STM_IHDR_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  STM_ZHDR_EN                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAM_REG_STM_CFG0;  /* CAM_STM_CFG0, CAM_A_STM_CFG0*/

typedef volatile union _CAM_REG_STM_CFG1_
{
        volatile struct /* 0x1A0044A4 */
        {
                FIELD  STM_IHDR_LE_FIRST                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  STM_ZHDR_RLE                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  STM_ZHDR_GLE                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  STM_ZHDR_BLE                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  STM_LIN_SEL                           :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  STM_LE_INV_CTL                        :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_STM_CFG1;  /* CAM_STM_CFG1, CAM_A_STM_CFG1*/

typedef volatile union _CAM_REG_SCM_CFG0_
{
        volatile struct /* 0x1A0044B0 */
        {
                FIELD  SCM_IHDR_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SCM_ZHDR_EN                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAM_REG_SCM_CFG0;  /* CAM_SCM_CFG0, CAM_A_SCM_CFG0*/

typedef volatile union _CAM_REG_SCM_CFG1_
{
        volatile struct /* 0x1A0044B4 */
        {
                FIELD  SCM_IHDR_LE_FIRST                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SCM_ZHDR_RLE                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SCM_ZHDR_GLE                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  SCM_ZHDR_BLE                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  SCM_LIN_SEL                           :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  SCM_LE_INV_CTL                        :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SCM_CFG1;  /* CAM_SCM_CFG1, CAM_A_SCM_CFG1*/

typedef volatile union _CAM_REG_RPG_SATU_1_
{
        volatile struct /* 0x1A0044C0 */
        {
                FIELD  RPG_SATU_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  RPG_SATU_GB                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_SATU_1;    /* CAM_RPG_SATU_1, CAM_A_RPG_SATU_1*/

typedef volatile union _CAM_REG_RPG_SATU_2_
{
        volatile struct /* 0x1A0044C4 */
        {
                FIELD  RPG_SATU_GR                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  RPG_SATU_R                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_SATU_2;    /* CAM_RPG_SATU_2, CAM_A_RPG_SATU_2*/

typedef volatile union _CAM_REG_RPG_GAIN_1_
{
        volatile struct /* 0x1A0044C8 */
        {
                FIELD  RPG_GAIN_B                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  RPG_GAIN_GB                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_GAIN_1;    /* CAM_RPG_GAIN_1, CAM_A_RPG_GAIN_1*/

typedef volatile union _CAM_REG_RPG_GAIN_2_
{
        volatile struct /* 0x1A0044CC */
        {
                FIELD  RPG_GAIN_GR                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  RPG_GAIN_R                            : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_GAIN_2;    /* CAM_RPG_GAIN_2, CAM_A_RPG_GAIN_2*/

typedef volatile union _CAM_REG_RPG_OFST_1_
{
        volatile struct /* 0x1A0044D0 */
        {
                FIELD  RPG_OFST_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  RPG_OFST_GB                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_OFST_1;    /* CAM_RPG_OFST_1, CAM_A_RPG_OFST_1*/

typedef volatile union _CAM_REG_RPG_OFST_2_
{
        volatile struct /* 0x1A0044D4 */
        {
                FIELD  RPG_OFST_GR                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  RPG_OFST_R                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RPG_OFST_2;    /* CAM_RPG_OFST_2, CAM_A_RPG_OFST_2*/

typedef volatile union _CAM_REG_RRZ_CTL_
{
        volatile struct /* 0x1A0044E0 */
        {
                FIELD  RRZ_HORI_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZ_VERT_EN                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RRZ_OUTPUT_WAIT_EN                    :  1;      /*  2.. 2, 0x00000004 */
                FIELD  RRZ_MONO                              :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RRZ_CL                                :  1;      /*  4.. 4, 0x00000010 */
                FIELD  RRZ_CL_HL                             :  1;      /*  5.. 5, 0x00000020 */
                FIELD  RRZ_NNIR                              :  1;      /*  6.. 6, 0x00000040 */
                FIELD  RRZ_VDM_VRZ_MODE                      :  1;      /*  7.. 7, 0x00000080 */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  RRZ_HORI_TBL_SEL                      :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  RRZ_VERT_TBL_SEL                      :  6;      /* 24..29, 0x3F000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_CTL;   /* CAM_RRZ_CTL, CAM_A_RRZ_CTL*/

typedef volatile union _CAM_REG_RRZ_IN_IMG_
{
        volatile struct /* 0x1A0044E4 */
        {
                FIELD  RRZ_IN_WD                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RRZ_IN_HT                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_IN_IMG;    /* CAM_RRZ_IN_IMG, CAM_A_RRZ_IN_IMG*/

typedef volatile union _CAM_REG_RRZ_OUT_IMG_
{
        volatile struct /* 0x1A0044E8 */
        {
                FIELD  RRZ_OUT_WD                            : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RRZ_OUT_HT                            : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_OUT_IMG;   /* CAM_RRZ_OUT_IMG, CAM_A_RRZ_OUT_IMG*/

typedef volatile union _CAM_REG_RRZ_HORI_STEP_
{
        volatile struct /* 0x1A0044EC */
        {
                FIELD  RRZ_HORI_STEP                         : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_HORI_STEP; /* CAM_RRZ_HORI_STEP, CAM_A_RRZ_HORI_STEP*/

typedef volatile union _CAM_REG_RRZ_VERT_STEP_
{
        volatile struct /* 0x1A0044F0 */
        {
                FIELD  RRZ_VERT_STEP                         : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_VERT_STEP; /* CAM_RRZ_VERT_STEP, CAM_A_RRZ_VERT_STEP*/

typedef volatile union _CAM_REG_RRZ_HORI_INT_OFST_
{
        volatile struct /* 0x1A0044F4 */
        {
                FIELD  RRZ_HORI_INT_OFST                     : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_HORI_INT_OFST; /* CAM_RRZ_HORI_INT_OFST, CAM_A_RRZ_HORI_INT_OFST*/

typedef volatile union _CAM_REG_RRZ_HORI_SUB_OFST_
{
        volatile struct /* 0x1A0044F8 */
        {
                FIELD  RRZ_HORI_SUB_OFST                     : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                : 17;      /* 15..31, 0xFFFF8000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_HORI_SUB_OFST; /* CAM_RRZ_HORI_SUB_OFST, CAM_A_RRZ_HORI_SUB_OFST*/

typedef volatile union _CAM_REG_RRZ_VERT_INT_OFST_
{
        volatile struct /* 0x1A0044FC */
        {
                FIELD  RRZ_VERT_INT_OFST                     : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_VERT_INT_OFST; /* CAM_RRZ_VERT_INT_OFST, CAM_A_RRZ_VERT_INT_OFST*/

typedef volatile union _CAM_REG_RRZ_VERT_SUB_OFST_
{
        volatile struct /* 0x1A004500 */
        {
                FIELD  RRZ_VERT_SUB_OFST                     : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                : 17;      /* 15..31, 0xFFFF8000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_VERT_SUB_OFST; /* CAM_RRZ_VERT_SUB_OFST, CAM_A_RRZ_VERT_SUB_OFST*/

typedef volatile union _CAM_REG_RRZ_MODE_TH_
{
        volatile struct /* 0x1A004504 */
        {
                FIELD  RRZ_TH_MD                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  RRZ_TH_HI                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  RRZ_TH_LO                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  RRZ_TH_MD2                            :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_MODE_TH;   /* CAM_RRZ_MODE_TH, CAM_A_RRZ_MODE_TH*/

typedef volatile union _CAM_REG_RRZ_MODE_CTL_
{
        volatile struct /* 0x1A004508 */
        {
                FIELD  RRZ_PRF_BLD                           :  9;      /*  0.. 8, 0x000001FF */
                FIELD  RRZ_PRF                               :  2;      /*  9..10, 0x00000600 */
                FIELD  RRZ_BLD_SL                            :  5;      /* 11..15, 0x0000F800 */
                FIELD  RRZ_CR_MODE                           :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_MODE_CTL;  /* CAM_RRZ_MODE_CTL, CAM_A_RRZ_MODE_CTL*/

typedef volatile union _CAM_REG_RRZ_RLB_AOFST_
{
        volatile struct /* 0x1A00450C */
        {
                FIELD  RRZ_RLB_AOFST                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_RLB_AOFST; /* CAM_RRZ_RLB_AOFST, CAM_A_RRZ_RLB_AOFST*/

typedef volatile union _CAM_REG_RRZ_LBLD_CFG_
{
        volatile struct /* 0x1A004510 */
        {
                FIELD  RRZ_LBLD_TH                           : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  RRZ_LBLD_SL                           :  5;      /* 16..20, 0x001F0000 */
                FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_LBLD_CFG;  /* CAM_RRZ_LBLD_CFG, CAM_A_RRZ_LBLD_CFG*/

typedef volatile union _CAM_REG_RRZ_NNIR_TBL_SEL_
{
        volatile struct /* 0x1A004514 */
        {
                FIELD  RRZ_HORI_TBL2_SEL                     :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  RRZ_VERT_TBL2_SEL                     :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                : 18;      /* 14..31, 0xFFFFC000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZ_NNIR_TBL_SEL;  /* CAM_RRZ_NNIR_TBL_SEL, CAM_A_RRZ_NNIR_TBL_SEL*/

typedef volatile union _CAM_REG_RMX_CTL_
{
        volatile struct /* 0x1A004540 */
        {
                FIELD  RMX_SRAM_SIZE                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RMX_EDGE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  RMX_SINGLE_MODE_1                     :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  RMX_SINGLE_MODE_2                     :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  6;      /* 25..30, 0x7E000000 */
                FIELD  RMX_EDGE_SET                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMX_CTL;   /* CAM_RMX_CTL, CAM_A_RMX_CTL*/

typedef volatile union _CAM_REG_RMX_CROP_
{
        volatile struct /* 0x1A004544 */
        {
                FIELD  RMX_STR_X                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RMX_END_X                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMX_CROP;  /* CAM_RMX_CROP, CAM_A_RMX_CROP*/

typedef volatile union _CAM_REG_RMX_VSIZE_
{
        volatile struct /* 0x1A004548 */
        {
                FIELD  RMX_HT                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMX_VSIZE; /* CAM_RMX_VSIZE, CAM_A_RMX_VSIZE*/

typedef volatile union _CAM_REG_BMX_CTL_
{
        volatile struct /* 0x1A004580 */
        {
                FIELD  BMX_SRAM_SIZE                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BMX_EDGE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  BMX_SINGLE_MODE_1                     :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  BMX_SINGLE_MODE_2                     :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  6;      /* 25..30, 0x7E000000 */
                FIELD  BMX_EDGE_SET                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BMX_CTL;   /* CAM_BMX_CTL, CAM_A_BMX_CTL*/

typedef volatile union _CAM_REG_BMX_CROP_
{
        volatile struct /* 0x1A004584 */
        {
                FIELD  BMX_STR_X                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BMX_END_X                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BMX_CROP;  /* CAM_BMX_CROP, CAM_A_BMX_CROP*/

typedef volatile union _CAM_REG_BMX_VSIZE_
{
        volatile struct /* 0x1A004588 */
        {
                FIELD  BMX_HT                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BMX_VSIZE; /* CAM_BMX_VSIZE, CAM_A_BMX_VSIZE*/

typedef volatile union _CAM_REG_UFEG_CON_
{
        volatile struct /* 0x1A0045C0 */
        {
                FIELD  UFEG_FORCE_PCM                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  UFEG_TCCT_BYP                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEG_CON;  /* CAM_UFEG_CON, CAM_A_UFEG_CON*/

typedef volatile union _CAM_REG_LSC_CTL1_
{
        volatile struct /* 0x1A0045D0 */
        {
                FIELD  SDBLK_YOFST                           :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 : 10;      /*  6..15, 0x0000FFC0 */
                FIELD  SDBLK_XOFST                           :  6;      /* 16..21, 0x003F0000 */
                FIELD  LSC_EXTEND_COEF_MODE                  :  1;      /* 22..22, 0x00400000 */
                FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
                FIELD  SD_COEFRD_MODE                        :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  3;      /* 25..27, 0x0E000000 */
                FIELD  SD_ULTRA_MODE                         :  1;      /* 28..28, 0x10000000 */
                FIELD  LSC_PRC_MODE                          :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_CTL1;  /* CAM_LSC_CTL1, CAM_A_LSC_CTL1*/

typedef volatile union _CAM_REG_LSC_CTL2_
{
        volatile struct /* 0x1A0045D4 */
        {
                FIELD  LSC_SDBLK_WIDTH                       : 13;      /*  0..12, 0x00001FFF */
                FIELD  LSC_SDBLK_XNUM                        :  5;      /* 13..17, 0x0003E000 */
                FIELD  LSC_OFLN                              :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_CTL2;  /* CAM_LSC_CTL2, CAM_A_LSC_CTL2*/

typedef volatile union _CAM_REG_LSC_CTL3_
{
        volatile struct /* 0x1A0045D8 */
        {
                FIELD  LSC_SDBLK_HEIGHT                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  LSC_SDBLK_YNUM                        :  5;      /* 13..17, 0x0003E000 */
                FIELD  LSC_SPARE                             : 14;      /* 18..31, 0xFFFC0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_CTL3;  /* CAM_LSC_CTL3, CAM_A_LSC_CTL3*/

typedef volatile union _CAM_REG_LSC_LBLOCK_
{
        volatile struct /* 0x1A0045DC */
        {
                FIELD  LSC_SDBLK_lHEIGHT                     : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LSC_SDBLK_lWIDTH                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_LBLOCK;    /* CAM_LSC_LBLOCK, CAM_A_LSC_LBLOCK*/

typedef volatile union _CAM_REG_LSC_RATIO_0_
{
        volatile struct /* 0x1A0045E0 */
        {
                FIELD  LSC_RA03                              :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  LSC_RA02                              :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LSC_RA01                              :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  LSC_RA00                              :  6;      /* 24..29, 0x3F000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_RATIO_0;   /* CAM_LSC_RATIO_0, CAM_A_LSC_RATIO_0*/

typedef volatile union _CAM_REG_LSC_TPIPE_OFST_
{
        volatile struct /* 0x1A0045E4 */
        {
                FIELD  LSC_TPIPE_OFST_Y                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LSC_TPIPE_OFST_X                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_TPIPE_OFST;    /* CAM_LSC_TPIPE_OFST, CAM_A_LSC_TPIPE_OFST*/

typedef volatile union _CAM_REG_LSC_TPIPE_SIZE_
{
        volatile struct /* 0x1A0045E8 */
        {
                FIELD  LSC_TPIPE_SIZE_Y                      : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LSC_TPIPE_SIZE_X                      : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_TPIPE_SIZE;    /* CAM_LSC_TPIPE_SIZE, CAM_A_LSC_TPIPE_SIZE*/

typedef volatile union _CAM_REG_LSC_GAIN_TH_
{
        volatile struct /* 0x1A0045EC */
        {
                FIELD  LSC_GAIN_TH2                          :  9;      /*  0.. 8, 0x000001FF */
                FIELD  rsv_9                                 :  1;      /*  9.. 9, 0x00000200 */
                FIELD  LSC_GAIN_TH1                          :  9;      /* 10..18, 0x0007FC00 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  LSC_GAIN_TH0                          :  9;      /* 20..28, 0x1FF00000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_GAIN_TH;   /* CAM_LSC_GAIN_TH, CAM_A_LSC_GAIN_TH*/

typedef volatile union _CAM_REG_LSC_RATIO_1_
{
        volatile struct /* 0x1A0045F0 */
        {
                FIELD  LSC_RA13                              :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  LSC_RA12                              :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LSC_RA11                              :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  LSC_RA10                              :  6;      /* 24..29, 0x3F000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_RATIO_1;   /* CAM_LSC_RATIO_1, CAM_A_LSC_RATIO_1*/

typedef volatile union _CAM_REG_LSC_UPB_B_GB_
{
        volatile struct /* 0x1A0045F4 */
        {
                FIELD  LSC_UPB_GB                            : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LSC_UPB_B                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_UPB_B_GB;  /* CAM_LSC_UPB_B_GB, CAM_A_LSC_UPB_B_GB*/

typedef volatile union _CAM_REG_LSC_UPB_GR_R_
{
        volatile struct /* 0x1A0045F8 */
        {
                FIELD  LSC_UPB_R                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LSC_UPB_GR                            : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC_UPB_GR_R;  /* CAM_LSC_UPB_GR_R, CAM_A_LSC_UPB_GR_R*/

typedef volatile union _CAM_REG_AF_CON_
{
        volatile struct /* 0x1A004610 */
        {
                FIELD  AF_BLF_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  AF_BLF_D_LVL                          :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  AF_BLF_R_LVL                          :  4;      /*  8..11, 0x00000F00 */
                FIELD  AF_BLF_VFIR_MUX                       :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AF_H_GONLY                            :  1;      /* 16..16, 0x00010000 */
                FIELD  AF_V_GONLY                            :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  AF_V_AVG_LVL                          :  2;      /* 20..21, 0x00300000 */
                FIELD  AF_EXT_STAT_EN                        :  1;      /* 22..22, 0x00400000 */
                FIELD  AF_VFLT_MODE                          :  1;      /* 23..23, 0x00800000 */
                FIELD  rsv_24                                :  4;      /* 24..27, 0x0F000000 */
                FIELD  RESERVED                              :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_CON;    /* CAM_AF_CON, CAM_A_AF_CON*/

typedef volatile union _CAM_REG_AF_TH_0_
{
        volatile struct /* 0x1A004614 */
        {
                FIELD  AF_H_TH_0                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_1                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_TH_0;   /* CAM_AF_TH_0, CAM_A_AF_TH_0*/

typedef volatile union _CAM_REG_AF_TH_1_
{
        volatile struct /* 0x1A004618 */
        {
                FIELD  AF_V_TH                               :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_R_SAT_TH                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_G_SAT_TH                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_B_SAT_TH                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_TH_1;   /* CAM_AF_TH_1, CAM_A_AF_TH_1*/

typedef volatile union _CAM_REG_AF_FLT_1_
{
        volatile struct /* 0x1A00461C */
        {
                FIELD  AF_HFLT0_P1                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT0_P2                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT0_P3                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT0_P4                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_1;  /* CAM_AF_FLT_1, CAM_A_AF_FLT_1*/

typedef volatile union _CAM_REG_AF_FLT_2_
{
        volatile struct /* 0x1A004620 */
        {
                FIELD  AF_HFLT0_P5                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT0_P6                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT0_P7                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT0_P8                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_2;  /* CAM_AF_FLT_2, CAM_A_AF_FLT_2*/

typedef volatile union _CAM_REG_AF_FLT_3_
{
        volatile struct /* 0x1A004624 */
        {
                FIELD  AF_HFLT0_P9                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT0_P10                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT0_P11                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT0_P12                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_3;  /* CAM_AF_FLT_3, CAM_A_AF_FLT_3*/

typedef volatile union _CAM_REG_AF_FLT_4_
{
        volatile struct /* 0x1A004628 */
        {
                FIELD  AF_HFLT1_P1                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT1_P2                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT1_P3                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT1_P4                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_4;  /* CAM_AF_FLT_4, CAM_A_AF_FLT_4*/

typedef volatile union _CAM_REG_AF_FLT_5_
{
        volatile struct /* 0x1A00462C */
        {
                FIELD  AF_HFLT1_P5                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT1_P6                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT1_P7                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT1_P8                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_5;  /* CAM_AF_FLT_5, CAM_A_AF_FLT_5*/

typedef volatile union _CAM_REG_AF_FLT_6_
{
        volatile struct /* 0x1A004630 */
        {
                FIELD  AF_HFLT1_P9                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT1_P10                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT1_P11                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT1_P12                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_6;  /* CAM_AF_FLT_6, CAM_A_AF_FLT_6*/

typedef volatile union _CAM_REG_AF_FLT_7_
{
        volatile struct /* 0x1A004634 */
        {
                FIELD  AF_VFLT_X0                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X1                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_7;  /* CAM_AF_FLT_7, CAM_A_AF_FLT_7*/

typedef volatile union _CAM_REG_AF_FLT_8_
{
        volatile struct /* 0x1A004638 */
        {
                FIELD  AF_VFLT_X2                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X3                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_8;  /* CAM_AF_FLT_8, CAM_A_AF_FLT_8*/

typedef volatile union _CAM_REG_AF_SIZE_
{
        volatile struct /* 0x1A004640 */
        {
                FIELD  AF_IMAGE_WD                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                : 18;      /* 14..31, 0xFFFFC000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_SIZE;   /* CAM_AF_SIZE, CAM_A_AF_SIZE*/

typedef volatile union _CAM_REG_AF_VLD_
{
        volatile struct /* 0x1A004644 */
        {
                FIELD  AF_VLD_XSTART                         : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AF_VLD_YSTART                         : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_VLD;    /* CAM_AF_VLD, CAM_A_AF_VLD*/

typedef volatile union _CAM_REG_AF_BLK_0_
{
        volatile struct /* 0x1A004648 */
        {
                FIELD  AF_BLK_XSIZE                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_BLK_YSIZE                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_BLK_0;  /* CAM_AF_BLK_0, CAM_A_AF_BLK_0*/

typedef volatile union _CAM_REG_AF_BLK_1_
{
        volatile struct /* 0x1A00464C */
        {
                FIELD  AF_BLK_XNUM                           :  9;      /*  0.. 8, 0x000001FF */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  AF_BLK_YNUM                           :  9;      /* 16..24, 0x01FF0000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_BLK_1;  /* CAM_AF_BLK_1, CAM_A_AF_BLK_1*/

typedef volatile union _CAM_REG_AF_TH_2_
{
        volatile struct /* 0x1A004650 */
        {
                FIELD  AF_HFLT2_SAT_TH0                      :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_HFLT2_SAT_TH1                      :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_HFLT2_SAT_TH2                      :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_HFLT2_SAT_TH3                      :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_TH_2;   /* CAM_AF_TH_2, CAM_A_AF_TH_2*/

typedef volatile union _CAM_REG_AF_FLT_9_
{
        volatile struct /* 0x1A004654 */
        {
                FIELD  AF_VFLT_X4                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X5                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_9;  /* CAM_AF_FLT_9, CAM_A_AF_FLT_9*/

typedef volatile union _CAM_REG_AF_FLT_10_
{
        volatile struct /* 0x1A004658 */
        {
                FIELD  AF_VFLT_X6                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X7                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_10; /* CAM_AF_FLT_10, CAM_A_AF_FLT_10*/

typedef volatile union _CAM_REG_AF_FLT_11_
{
        volatile struct /* 0x1A00465C */
        {
                FIELD  AF_VFLT_X8                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X9                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_11; /* CAM_AF_FLT_11, CAM_A_AF_FLT_11*/

typedef volatile union _CAM_REG_AF_FLT_12_
{
        volatile struct /* 0x1A004660 */
        {
                FIELD  AF_VFLT_X10                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AF_VFLT_X11                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_FLT_12; /* CAM_AF_FLT_12, CAM_A_AF_FLT_12*/

typedef volatile union _CAM_REG_AF_LUT_H0_0_
{
        volatile struct /* 0x1A004670 */
        {
                FIELD  AF_H_TH_0_LUT_MODE                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AF_H_TH_0_GAIN                        :  9;      /*  1.. 9, 0x000003FE */
                FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H0_0;   /* CAM_AF_LUT_H0_0, CAM_A_AF_LUT_H0_0*/

typedef volatile union _CAM_REG_AF_LUT_H0_1_
{
        volatile struct /* 0x1A004674 */
        {
                FIELD  AF_H_TH_0_D1                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_0_D2                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_0_D3                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_0_D4                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H0_1;   /* CAM_AF_LUT_H0_1, CAM_A_AF_LUT_H0_1*/

typedef volatile union _CAM_REG_AF_LUT_H0_2_
{
        volatile struct /* 0x1A004678 */
        {
                FIELD  AF_H_TH_0_D5                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_0_D6                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_0_D7                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_0_D8                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H0_2;   /* CAM_AF_LUT_H0_2, CAM_A_AF_LUT_H0_2*/

typedef volatile union _CAM_REG_AF_LUT_H0_3_
{
        volatile struct /* 0x1A00467C */
        {
                FIELD  AF_H_TH_0_D9                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_0_D10                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_0_D11                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_0_D12                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H0_3;   /* CAM_AF_LUT_H0_3, CAM_A_AF_LUT_H0_3*/

typedef volatile union _CAM_REG_AF_LUT_H0_4_
{
        volatile struct /* 0x1A004680 */
        {
                FIELD  AF_H_TH_0_D13                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_0_D14                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_0_D15                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_0_D16                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H0_4;   /* CAM_AF_LUT_H0_4, CAM_A_AF_LUT_H0_4*/

typedef volatile union _CAM_REG_AF_LUT_H1_0_
{
        volatile struct /* 0x1A004690 */
        {
                FIELD  AF_H_TH_1_LUT_MODE                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AF_H_TH_1_GAIN                        :  9;      /*  1.. 9, 0x000003FE */
                FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H1_0;   /* CAM_AF_LUT_H1_0, CAM_A_AF_LUT_H1_0*/

typedef volatile union _CAM_REG_AF_LUT_H1_1_
{
        volatile struct /* 0x1A004694 */
        {
                FIELD  AF_H_TH_1_D1                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_1_D2                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_1_D3                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_1_D4                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H1_1;   /* CAM_AF_LUT_H1_1, CAM_A_AF_LUT_H1_1*/

typedef volatile union _CAM_REG_AF_LUT_H1_2_
{
        volatile struct /* 0x1A004698 */
        {
                FIELD  AF_H_TH_1_D5                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_1_D6                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_1_D7                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_1_D8                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H1_2;   /* CAM_AF_LUT_H1_2, CAM_A_AF_LUT_H1_2*/

typedef volatile union _CAM_REG_AF_LUT_H1_3_
{
        volatile struct /* 0x1A00469C */
        {
                FIELD  AF_H_TH_1_D9                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_1_D10                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_1_D11                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_1_D12                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H1_3;   /* CAM_AF_LUT_H1_3, CAM_A_AF_LUT_H1_3*/

typedef volatile union _CAM_REG_AF_LUT_H1_4_
{
        volatile struct /* 0x1A0046A0 */
        {
                FIELD  AF_H_TH_1_D13                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_H_TH_1_D14                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_H_TH_1_D15                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_H_TH_1_D16                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_H1_4;   /* CAM_AF_LUT_H1_4, CAM_A_AF_LUT_H1_4*/

typedef volatile union _CAM_REG_AF_LUT_V_0_
{
        volatile struct /* 0x1A0046B0 */
        {
                FIELD  AF_V_TH_LUT_MODE                      :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AF_V_TH_GAIN                          :  9;      /*  1.. 9, 0x000003FE */
                FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_V_0;    /* CAM_AF_LUT_V_0, CAM_A_AF_LUT_V_0*/

typedef volatile union _CAM_REG_AF_LUT_V_1_
{
        volatile struct /* 0x1A0046B4 */
        {
                FIELD  AF_V_TH_D1                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_V_TH_D2                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_V_TH_D3                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_V_TH_D4                            :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_V_1;    /* CAM_AF_LUT_V_1, CAM_A_AF_LUT_V_1*/

typedef volatile union _CAM_REG_AF_LUT_V_2_
{
        volatile struct /* 0x1A0046B8 */
        {
                FIELD  AF_V_TH_D5                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_V_TH_D6                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_V_TH_D7                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_V_TH_D8                            :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_V_2;    /* CAM_AF_LUT_V_2, CAM_A_AF_LUT_V_2*/

typedef volatile union _CAM_REG_AF_LUT_V_3_
{
        volatile struct /* 0x1A0046BC */
        {
                FIELD  AF_V_TH_D9                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_V_TH_D10                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_V_TH_D11                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_V_TH_D12                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_V_3;    /* CAM_AF_LUT_V_3, CAM_A_AF_LUT_V_3*/

typedef volatile union _CAM_REG_AF_LUT_V_4_
{
        volatile struct /* 0x1A0046C0 */
        {
                FIELD  AF_V_TH_D13                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AF_V_TH_D14                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_V_TH_D15                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  AF_V_TH_D16                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_LUT_V_4;    /* CAM_AF_LUT_V_4, CAM_A_AF_LUT_V_4*/

typedef volatile union _CAM_REG_AF_CON2_
{
        volatile struct /* 0x1A0046C4 */
        {
                FIELD  AF_DS_EN                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AF_H_FV1_ABS                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AF_H_FV2_ABS                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AF_V_FV_ABS                           :  1;      /*  3.. 3, 0x00000008 */
                FIELD  AF_H_MAXFV_ABS                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AF_H_WFV_ABS                          :  1;      /*  5.. 5, 0x00000020 */
                FIELD  AF_H_FV2_EN                           :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_CON2;   /* CAM_AF_CON2, CAM_A_AF_CON2*/

typedef volatile union _CAM_REG_AF_BLK_2_
{
        volatile struct /* 0x1A0046C8 */
        {
                FIELD  AF_PROT_BLK_XSIZE                     :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AF_PROT_BLK_YSIZE                     :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AF_BLK_2;  /* CAM_AF_BLK_2, CAM_A_AF_BLK_2*/

typedef volatile union _CAM_REG_HLR_CFG_
{
        volatile struct /* 0x1A004710 */
        {
                FIELD  HLR_DTH                               :  8;      /*  0.. 7, 0x000000FF */
                FIELD  HLR_DSL                               : 10;      /*  8..17, 0x0003FF00 */
                FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
                FIELD  HLR_SL_EN                             :  1;      /* 19..19, 0x00080000 */
                FIELD  HLR_RAT                               :  6;      /* 20..25, 0x03F00000 */
                FIELD  HLR_MODE                              :  2;      /* 26..27, 0x0C000000 */
                FIELD  HLR_EDGE                              :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_CFG;   /* CAM_HLR_CFG, CAM_A_HLR_CFG*/

typedef volatile union _CAM_REG_HLR_GAIN_
{
        volatile struct /* 0x1A004718 */
        {
                FIELD  HLR_GAIN_B                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  HLR_GAIN_GB                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_GAIN;  /* CAM_HLR_GAIN, CAM_A_HLR_GAIN*/

typedef volatile union _CAM_REG_HLR_GAIN_1_
{
        volatile struct /* 0x1A00471C */
        {
                FIELD  HLR_GAIN_GR                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  HLR_GAIN_R                            : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_GAIN_1;    /* CAM_HLR_GAIN_1, CAM_A_HLR_GAIN_1*/

typedef volatile union _CAM_REG_HLR_OFST_
{
        volatile struct /* 0x1A004720 */
        {
                FIELD  HLR_OFST_B                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  HLR_OFST_GB                           : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_OFST;  /* CAM_HLR_OFST, CAM_A_HLR_OFST*/

typedef volatile union _CAM_REG_HLR_OFST_1_
{
        volatile struct /* 0x1A004724 */
        {
                FIELD  HLR_OFST_GR                           : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  HLR_OFST_R                            : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_OFST_1;    /* CAM_HLR_OFST_1, CAM_A_HLR_OFST_1*/

typedef volatile union _CAM_REG_HLR_IVGN_
{
        volatile struct /* 0x1A004728 */
        {
                FIELD  HLR_IVGN_B                            : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  HLR_IVGN_GB                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_IVGN;  /* CAM_HLR_IVGN, CAM_A_HLR_IVGN*/

typedef volatile union _CAM_REG_HLR_IVGN_1_
{
        volatile struct /* 0x1A00472C */
        {
                FIELD  HLR_IVGN_GR                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  HLR_IVGN_R                            : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_IVGN_1;    /* CAM_HLR_IVGN_1, CAM_A_HLR_IVGN_1*/

typedef volatile union _CAM_REG_HLR_KC_
{
        volatile struct /* 0x1A004730 */
        {
                FIELD  HLR_KC_C0                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  HLR_KC_C1                             : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_KC;    /* CAM_HLR_KC, CAM_A_HLR_KC*/

typedef volatile union _CAM_REG_HLR_CFG_1_
{
        volatile struct /* 0x1A004734 */
        {
                FIELD  HLR_STR                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_CFG_1; /* CAM_HLR_CFG_1, CAM_A_HLR_CFG_1*/

typedef volatile union _CAM_REG_HLR_SL_PARA_
{
        volatile struct /* 0x1A004738 */
        {
                FIELD  HLR_SL_Y1                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  HLR_SL_Y2                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_SL_PARA;   /* CAM_HLR_SL_PARA, CAM_A_HLR_SL_PARA*/

typedef volatile union _CAM_REG_HLR_SL_PARA_1_
{
        volatile struct /* 0x1A00473C */
        {
                FIELD  HLR_SL_Y3                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  HLR_SL_Y4                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_HLR_SL_PARA_1; /* CAM_HLR_SL_PARA_1, CAM_A_HLR_SL_PARA_1*/

typedef volatile union _CAM_REG_LCS25_CON_
{
        volatile struct /* 0x1A004790 */
        {
                FIELD  LCS25_LOG                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  7;      /*  1.. 7, 0x000000FE */
                FIELD  LCS25_OUT_WD                          :  9;      /*  8..16, 0x0001FF00 */
                FIELD  rsv_17                                :  3;      /* 17..19, 0x000E0000 */
                FIELD  LCS25_OUT_HT                          :  9;      /* 20..28, 0x1FF00000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_CON; /* CAM_LCS25_CON, CAM_A_LCS25_CON*/

typedef volatile union _CAM_REG_LCS25_ST_
{
        volatile struct /* 0x1A004794 */
        {
                FIELD  LCS25_START_J                         : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_START_I                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_ST;  /* CAM_LCS25_ST, CAM_A_LCS25_ST*/

typedef volatile union _CAM_REG_LCS25_AWS_
{
        volatile struct /* 0x1A004798 */
        {
                FIELD  LCS25_IN_WD                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_IN_HT                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_AWS; /* CAM_LCS25_AWS, CAM_A_LCS25_AWS*/

typedef volatile union _CAM_REG_LCS25_FLR_
{
        volatile struct /* 0x1A00479C */
        {
                FIELD  LCS25_FLR_OFST                        :  8;      /*  0.. 7, 0x000000FF */
                FIELD  LCS25_FLR_GAIN                        : 12;      /*  8..19, 0x000FFF00 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_FLR; /* CAM_LCS25_FLR, CAM_A_LCS25_FLR*/

typedef volatile union _CAM_REG_LCS25_LRZR_1_
{
        volatile struct /* 0x1A0047A0 */
        {
                FIELD  LCS25_LRZR_X                          : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_LRZR_1;  /* CAM_LCS25_LRZR_1, CAM_A_LCS25_LRZR_1*/

typedef volatile union _CAM_REG_LCS25_LRZR_2_
{
        volatile struct /* 0x1A0047A4 */
        {
                FIELD  LCS25_LRZR_Y                          : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_LRZR_2;  /* CAM_LCS25_LRZR_2, CAM_A_LCS25_LRZR_2*/

typedef volatile union _CAM_REG_LCS25_SATU_1_
{
        volatile struct /* 0x1A0047A8 */
        {
                FIELD  LCS25_SATU_B                          : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCS25_SATU_GB                         : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_SATU_1;  /* CAM_LCS25_SATU_1, CAM_A_LCS25_SATU_1*/

typedef volatile union _CAM_REG_LCS25_SATU_2_
{
        volatile struct /* 0x1A0047AC */
        {
                FIELD  LCS25_SATU_GR                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCS25_SATU_R                          : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_SATU_2;  /* CAM_LCS25_SATU_2, CAM_A_LCS25_SATU_2*/

typedef volatile union _CAM_REG_LCS25_GAIN_1_
{
        volatile struct /* 0x1A0047B0 */
        {
                FIELD  LCS25_GAIN_B                          : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_GAIN_GB                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_GAIN_1;  /* CAM_LCS25_GAIN_1, CAM_A_LCS25_GAIN_1*/

typedef volatile union _CAM_REG_LCS25_GAIN_2_
{
        volatile struct /* 0x1A0047B4 */
        {
                FIELD  LCS25_GAIN_GR                         : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_GAIN_R                          : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_GAIN_2;  /* CAM_LCS25_GAIN_2, CAM_A_LCS25_GAIN_2*/

typedef volatile union _CAM_REG_LCS25_OFST_1_
{
        volatile struct /* 0x1A0047B8 */
        {
                FIELD  LCS25_OFST_B                          : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCS25_OFST_GB                         : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_OFST_1;  /* CAM_LCS25_OFST_1, CAM_A_LCS25_OFST_1*/

typedef volatile union _CAM_REG_LCS25_OFST_2_
{
        volatile struct /* 0x1A0047BC */
        {
                FIELD  LCS25_OFST_GR                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCS25_OFST_R                          : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_OFST_2;  /* CAM_LCS25_OFST_2, CAM_A_LCS25_OFST_2*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_1_
{
        volatile struct /* 0x1A0047C0 */
        {
                FIELD  LCS25_G2G_CNV_00                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_G2G_CNV_01                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_1;   /* CAM_LCS25_G2G_CNV_1, CAM_A_LCS25_G2G_CNV_1*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_2_
{
        volatile struct /* 0x1A0047C4 */
        {
                FIELD  LCS25_G2G_CNV_02                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_G2G_CNV_10                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_2;   /* CAM_LCS25_G2G_CNV_2, CAM_A_LCS25_G2G_CNV_2*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_3_
{
        volatile struct /* 0x1A0047C8 */
        {
                FIELD  LCS25_G2G_CNV_11                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_G2G_CNV_12                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_3;   /* CAM_LCS25_G2G_CNV_3, CAM_A_LCS25_G2G_CNV_3*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_4_
{
        volatile struct /* 0x1A0047CC */
        {
                FIELD  LCS25_G2G_CNV_20                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_G2G_CNV_21                      : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_4;   /* CAM_LCS25_G2G_CNV_4, CAM_A_LCS25_G2G_CNV_4*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_5_
{
        volatile struct /* 0x1A0047D0 */
        {
                FIELD  LCS25_G2G_CNV_22                      : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  LCS25_G2G_ACC                         :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_5;   /* CAM_LCS25_G2G_CNV_5, CAM_A_LCS25_G2G_CNV_5*/

typedef volatile union _CAM_REG_LCS25_LPF_
{
        volatile struct /* 0x1A0047D4 */
        {
                FIELD  LCS25_LPF_EN                          :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  LCS25_LPF_TH                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCS25_LPF; /* CAM_LCS25_LPF, CAM_A_LCS25_LPF*/

typedef volatile union _CAM_REG_RCP_CROP_CON1_
{
        volatile struct /* 0x1A004800 */
        {
                FIELD  RCP_STR_X                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  RCP_END_X                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RCP_CROP_CON1; /* CAM_RCP_CROP_CON1, CAM_A_RCP_CROP_CON1*/

typedef volatile union _CAM_REG_RCP_CROP_CON2_
{
        volatile struct /* 0x1A004804 */
        {
                FIELD  RCP_STR_Y                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  RCP_END_Y                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RCP_CROP_CON2; /* CAM_RCP_CROP_CON2, CAM_A_RCP_CROP_CON2*/

typedef volatile union _CAM_REG_SGG1_PGN_
{
        volatile struct /* 0x1A004810 */
        {
                FIELD  SGG_GAIN                              : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG1_PGN;  /* CAM_SGG1_PGN, CAM_A_SGG1_PGN*/

typedef volatile union _CAM_REG_SGG1_GMRC_1_
{
        volatile struct /* 0x1A004814 */
        {
                FIELD  SGG_GMR_1                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_2                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_3                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGG_GMR_4                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG1_GMRC_1;   /* CAM_SGG1_GMRC_1, CAM_A_SGG1_GMRC_1*/

typedef volatile union _CAM_REG_SGG1_GMRC_2_
{
        volatile struct /* 0x1A004818 */
        {
                FIELD  SGG_GMR_5                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_6                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_7                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG1_GMRC_2;   /* CAM_SGG1_GMRC_2, CAM_A_SGG1_GMRC_2*/

typedef volatile union _CAM_REG_QBN2_MODE_
{
        volatile struct /* 0x1A004820 */
        {
                FIELD  QBN_ACC                               :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  QBN_ACC_MODE                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_QBN2_MODE; /* CAM_QBN2_MODE, CAM_A_QBN2_MODE*/

typedef volatile union _CAM_REG_AWB_WIN_ORG_
{
        volatile struct /* 0x1A004830 */
        {
                FIELD  AWB_W_HORG                            : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VORG                            : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_WIN_ORG;   /* CAM_AWB_WIN_ORG, CAM_A_AWB_WIN_ORG*/

typedef volatile union _CAM_REG_AWB_WIN_SIZE_
{
        volatile struct /* 0x1A004834 */
        {
                FIELD  AWB_W_HSIZE                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VSIZE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_WIN_SIZE;  /* CAM_AWB_WIN_SIZE, CAM_A_AWB_WIN_SIZE*/

typedef volatile union _CAM_REG_AWB_WIN_PIT_
{
        volatile struct /* 0x1A004838 */
        {
                FIELD  AWB_W_HPIT                            : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VPIT                            : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_WIN_PIT;   /* CAM_AWB_WIN_PIT, CAM_A_AWB_WIN_PIT*/

typedef volatile union _CAM_REG_AWB_WIN_NUM_
{
        volatile struct /* 0x1A00483C */
        {
                FIELD  AWB_W_HNUM                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AWB_W_VNUM                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_WIN_NUM;   /* CAM_AWB_WIN_NUM, CAM_A_AWB_WIN_NUM*/

typedef volatile union _CAM_REG_AWB_GAIN1_0_
{
        volatile struct /* 0x1A004840 */
        {
                FIELD  AWB_GAIN1_R                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AWB_GAIN1_G                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_GAIN1_0;   /* CAM_AWB_GAIN1_0, CAM_A_AWB_GAIN1_0*/

typedef volatile union _CAM_REG_AWB_GAIN1_1_
{
        volatile struct /* 0x1A004844 */
        {
                FIELD  AWB_GAIN1_B                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_GAIN1_1;   /* CAM_AWB_GAIN1_1, CAM_A_AWB_GAIN1_1*/

typedef volatile union _CAM_REG_AWB_LMT1_0_
{
        volatile struct /* 0x1A004848 */
        {
                FIELD  AWB_LMT1_R                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_LMT1_G                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_LMT1_0;    /* CAM_AWB_LMT1_0, CAM_A_AWB_LMT1_0*/

typedef volatile union _CAM_REG_AWB_LMT1_1_
{
        volatile struct /* 0x1A00484C */
        {
                FIELD  AWB_LMT1_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_LMT1_1;    /* CAM_AWB_LMT1_1, CAM_A_AWB_LMT1_1*/

typedef volatile union _CAM_REG_AWB_LOW_THR_
{
        volatile struct /* 0x1A004850 */
        {
                FIELD  AWB_LOW_THR0                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AWB_LOW_THR1                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AWB_LOW_THR2                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_LOW_THR;   /* CAM_AWB_LOW_THR, CAM_A_AWB_LOW_THR*/

typedef volatile union _CAM_REG_AWB_HI_THR_
{
        volatile struct /* 0x1A004854 */
        {
                FIELD  AWB_HI_THR0                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AWB_HI_THR1                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AWB_HI_THR2                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_HI_THR;    /* CAM_AWB_HI_THR, CAM_A_AWB_HI_THR*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT0_
{
        volatile struct /* 0x1A004858 */
        {
                FIELD  AWB_PIXEL_CNT0                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT0;    /* CAM_AWB_PIXEL_CNT0, CAM_A_AWB_PIXEL_CNT0*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT1_
{
        volatile struct /* 0x1A00485C */
        {
                FIELD  AWB_PIXEL_CNT1                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT1;    /* CAM_AWB_PIXEL_CNT1, CAM_A_AWB_PIXEL_CNT1*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT2_
{
        volatile struct /* 0x1A004860 */
        {
                FIELD  AWB_PIXEL_CNT2                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT2;    /* CAM_AWB_PIXEL_CNT2, CAM_A_AWB_PIXEL_CNT2*/

typedef volatile union _CAM_REG_AWB_ERR_THR_
{
        volatile struct /* 0x1A004864 */
        {
                FIELD  AWB_ERR_THR                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_ERR_SFT                           :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_ERR_THR;   /* CAM_AWB_ERR_THR, CAM_A_AWB_ERR_THR*/

typedef volatile union _CAM_REG_AWB_ROT_
{
        volatile struct /* 0x1A004868 */
        {
                FIELD  AWB_C                                 : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  AWB_S                                 : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_ROT;   /* CAM_AWB_ROT, CAM_A_AWB_ROT*/

typedef volatile union _CAM_REG_AWB_L0_X_
{
        volatile struct /* 0x1A00486C */
        {
                FIELD  AWB_L0_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L0_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L0_X;  /* CAM_AWB_L0_X, CAM_A_AWB_L0_X*/

typedef volatile union _CAM_REG_AWB_L0_Y_
{
        volatile struct /* 0x1A004870 */
        {
                FIELD  AWB_L0_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L0_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L0_Y;  /* CAM_AWB_L0_Y, CAM_A_AWB_L0_Y*/

typedef volatile union _CAM_REG_AWB_L1_X_
{
        volatile struct /* 0x1A004874 */
        {
                FIELD  AWB_L1_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L1_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L1_X;  /* CAM_AWB_L1_X, CAM_A_AWB_L1_X*/

typedef volatile union _CAM_REG_AWB_L1_Y_
{
        volatile struct /* 0x1A004878 */
        {
                FIELD  AWB_L1_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L1_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L1_Y;  /* CAM_AWB_L1_Y, CAM_A_AWB_L1_Y*/

typedef volatile union _CAM_REG_AWB_L2_X_
{
        volatile struct /* 0x1A00487C */
        {
                FIELD  AWB_L2_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L2_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L2_X;  /* CAM_AWB_L2_X, CAM_A_AWB_L2_X*/

typedef volatile union _CAM_REG_AWB_L2_Y_
{
        volatile struct /* 0x1A004880 */
        {
                FIELD  AWB_L2_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L2_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L2_Y;  /* CAM_AWB_L2_Y, CAM_A_AWB_L2_Y*/

typedef volatile union _CAM_REG_AWB_L3_X_
{
        volatile struct /* 0x1A004884 */
        {
                FIELD  AWB_L3_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L3_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L3_X;  /* CAM_AWB_L3_X, CAM_A_AWB_L3_X*/

typedef volatile union _CAM_REG_AWB_L3_Y_
{
        volatile struct /* 0x1A004888 */
        {
                FIELD  AWB_L3_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L3_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L3_Y;  /* CAM_AWB_L3_Y, CAM_A_AWB_L3_Y*/

typedef volatile union _CAM_REG_AWB_L4_X_
{
        volatile struct /* 0x1A00488C */
        {
                FIELD  AWB_L4_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L4_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L4_X;  /* CAM_AWB_L4_X, CAM_A_AWB_L4_X*/

typedef volatile union _CAM_REG_AWB_L4_Y_
{
        volatile struct /* 0x1A004890 */
        {
                FIELD  AWB_L4_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L4_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L4_Y;  /* CAM_AWB_L4_Y, CAM_A_AWB_L4_Y*/

typedef volatile union _CAM_REG_AWB_L5_X_
{
        volatile struct /* 0x1A004894 */
        {
                FIELD  AWB_L5_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L5_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L5_X;  /* CAM_AWB_L5_X, CAM_A_AWB_L5_X*/

typedef volatile union _CAM_REG_AWB_L5_Y_
{
        volatile struct /* 0x1A004898 */
        {
                FIELD  AWB_L5_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L5_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L5_Y;  /* CAM_AWB_L5_Y, CAM_A_AWB_L5_Y*/

typedef volatile union _CAM_REG_AWB_L6_X_
{
        volatile struct /* 0x1A00489C */
        {
                FIELD  AWB_L6_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L6_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L6_X;  /* CAM_AWB_L6_X, CAM_A_AWB_L6_X*/

typedef volatile union _CAM_REG_AWB_L6_Y_
{
        volatile struct /* 0x1A0048A0 */
        {
                FIELD  AWB_L6_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L6_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L6_Y;  /* CAM_AWB_L6_Y, CAM_A_AWB_L6_Y*/

typedef volatile union _CAM_REG_AWB_L7_X_
{
        volatile struct /* 0x1A0048A4 */
        {
                FIELD  AWB_L7_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L7_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L7_X;  /* CAM_AWB_L7_X, CAM_A_AWB_L7_X*/

typedef volatile union _CAM_REG_AWB_L7_Y_
{
        volatile struct /* 0x1A0048A8 */
        {
                FIELD  AWB_L7_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L7_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L7_Y;  /* CAM_AWB_L7_Y, CAM_A_AWB_L7_Y*/

typedef volatile union _CAM_REG_AWB_L8_X_
{
        volatile struct /* 0x1A0048AC */
        {
                FIELD  AWB_L8_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L8_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L8_X;  /* CAM_AWB_L8_X, CAM_A_AWB_L8_X*/

typedef volatile union _CAM_REG_AWB_L8_Y_
{
        volatile struct /* 0x1A0048B0 */
        {
                FIELD  AWB_L8_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L8_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L8_Y;  /* CAM_AWB_L8_Y, CAM_A_AWB_L8_Y*/

typedef volatile union _CAM_REG_AWB_L9_X_
{
        volatile struct /* 0x1A0048B4 */
        {
                FIELD  AWB_L9_X_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L9_X_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L9_X;  /* CAM_AWB_L9_X, CAM_A_AWB_L9_X*/

typedef volatile union _CAM_REG_AWB_L9_Y_
{
        volatile struct /* 0x1A0048B8 */
        {
                FIELD  AWB_L9_Y_LOW                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_L9_Y_UP                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_L9_Y;  /* CAM_AWB_L9_Y, CAM_A_AWB_L9_Y*/

typedef volatile union _CAM_REG_AWB_SPARE_
{
        volatile struct /* 0x1A0048BC */
        {
                FIELD  AWB_SPARE                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_SPARE; /* CAM_AWB_SPARE, CAM_A_AWB_SPARE*/

typedef volatile union _CAM_REG_AWB_MOTION_THR_
{
        volatile struct /* 0x1A0048C0 */
        {
                FIELD  AWB_MOTION_THR                        : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_MOTION_MAP_EN                     :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_MOTION_THR;    /* CAM_AWB_MOTION_THR, CAM_A_AWB_MOTION_THR*/

typedef volatile union _CAM_REG_AWB_RC_CNV_0_
{
        volatile struct /* 0x1A0048C4 */
        {
                FIELD  AWB_RC_CNV00                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_RC_CNV01                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_RC_CNV_0;  /* CAM_AWB_RC_CNV_0, CAM_A_AWB_RC_CNV_0*/

typedef volatile union _CAM_REG_AWB_RC_CNV_1_
{
        volatile struct /* 0x1A0048C8 */
        {
                FIELD  AWB_RC_CNV02                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_RC_CNV10                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_RC_CNV_1;  /* CAM_AWB_RC_CNV_1, CAM_A_AWB_RC_CNV_1*/

typedef volatile union _CAM_REG_AWB_RC_CNV_2_
{
        volatile struct /* 0x1A0048CC */
        {
                FIELD  AWB_RC_CNV11                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_RC_CNV12                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_RC_CNV_2;  /* CAM_AWB_RC_CNV_2, CAM_A_AWB_RC_CNV_2*/

typedef volatile union _CAM_REG_AWB_RC_CNV_3_
{
        volatile struct /* 0x1A0048D0 */
        {
                FIELD  AWB_RC_CNV20                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_RC_CNV21                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_RC_CNV_3;  /* CAM_AWB_RC_CNV_3, CAM_A_AWB_RC_CNV_3*/

typedef volatile union _CAM_REG_AWB_RC_CNV_4_
{
        volatile struct /* 0x1A0048D4 */
        {
                FIELD  AWB_RC_CNV22                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AWB_RC_ACC                            :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AWB_RC_CNV_4;  /* CAM_AWB_RC_CNV_4, CAM_A_AWB_RC_CNV_4*/

typedef volatile union _CAM_REG_AE_GAIN2_0_
{
        volatile struct /* 0x1A004900 */
        {
                FIELD  AE_GAIN2_R                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_GAIN2_G                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_GAIN2_0;    /* CAM_AE_GAIN2_0, CAM_A_AE_GAIN2_0*/

typedef volatile union _CAM_REG_AE_GAIN2_1_
{
        volatile struct /* 0x1A004904 */
        {
                FIELD  AE_GAIN2_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_GAIN2_1;    /* CAM_AE_GAIN2_1, CAM_A_AE_GAIN2_1*/

typedef volatile union _CAM_REG_AE_LMT2_0_
{
        volatile struct /* 0x1A004908 */
        {
                FIELD  AE_LMT2_R                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_LMT2_G                             : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_LMT2_0; /* CAM_AE_LMT2_0, CAM_A_AE_LMT2_0*/

typedef volatile union _CAM_REG_AE_LMT2_1_
{
        volatile struct /* 0x1A00490C */
        {
                FIELD  AE_LMT2_B                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_LMT2_1; /* CAM_AE_LMT2_1, CAM_A_AE_LMT2_1*/

typedef volatile union _CAM_REG_AE_RC_CNV_0_
{
        volatile struct /* 0x1A004910 */
        {
                FIELD  AE_RC_CNV00                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AE_RC_CNV01                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_RC_CNV_0;   /* CAM_AE_RC_CNV_0, CAM_A_AE_RC_CNV_0*/

typedef volatile union _CAM_REG_AE_RC_CNV_1_
{
        volatile struct /* 0x1A004914 */
        {
                FIELD  AE_RC_CNV02                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AE_RC_CNV10                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_RC_CNV_1;   /* CAM_AE_RC_CNV_1, CAM_A_AE_RC_CNV_1*/

typedef volatile union _CAM_REG_AE_RC_CNV_2_
{
        volatile struct /* 0x1A004918 */
        {
                FIELD  AE_RC_CNV11                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AE_RC_CNV12                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_RC_CNV_2;   /* CAM_AE_RC_CNV_2, CAM_A_AE_RC_CNV_2*/

typedef volatile union _CAM_REG_AE_RC_CNV_3_
{
        volatile struct /* 0x1A00491C */
        {
                FIELD  AE_RC_CNV20                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AE_RC_CNV21                           : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_RC_CNV_3;   /* CAM_AE_RC_CNV_3, CAM_A_AE_RC_CNV_3*/

typedef volatile union _CAM_REG_AE_RC_CNV_4_
{
        volatile struct /* 0x1A004920 */
        {
                FIELD  AE_RC_CNV22                           : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  AE_RC_ACC                             :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_RC_CNV_4;   /* CAM_AE_RC_CNV_4, CAM_A_AE_RC_CNV_4*/

typedef volatile union _CAM_REG_AE_YGAMMA_0_
{
        volatile struct /* 0x1A004924 */
        {
                FIELD  Y_GMR1                                :  8;      /*  0.. 7, 0x000000FF */
                FIELD  Y_GMR2                                :  8;      /*  8..15, 0x0000FF00 */
                FIELD  Y_GMR3                                :  8;      /* 16..23, 0x00FF0000 */
                FIELD  Y_GMR4                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_YGAMMA_0;   /* CAM_AE_YGAMMA_0, CAM_A_AE_YGAMMA_0*/

typedef volatile union _CAM_REG_AE_YGAMMA_1_
{
        volatile struct /* 0x1A004928 */
        {
                FIELD  Y_GMR5                                :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_YGAMMA_1;   /* CAM_AE_YGAMMA_1, CAM_A_AE_YGAMMA_1*/

typedef volatile union _CAM_REG_AE_OVER_EXPO_CFG_
{
        volatile struct /* 0x1A00492C */
        {
                FIELD  AE_OVER_EXPO_THR                      :  8;      /*  0.. 7, 0x000000FF */
                FIELD  AE_OVER_EXPO_SFT                      :  1;      /*  8.. 8, 0x00000100 */
                FIELD  rsv_9                                 : 23;      /*  9..31, 0xFFFFFE00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_OVER_EXPO_CFG;  /* CAM_AE_OVER_EXPO_CFG, CAM_A_AE_OVER_EXPO_CFG*/

typedef volatile union _CAM_REG_AE_PIX_HST_CTL_
{
        volatile struct /* 0x1A004930 */
        {
                FIELD  AE_PIX_HST0_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_PIX_HST1_EN                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AE_PIX_HST2_EN                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AE_PIX_HST3_EN                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  AE_PIX_HST4_EN                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  AE_PIX_HST5_EN                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 : 26;      /*  6..31, 0xFFFFFFC0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST_CTL;    /* CAM_AE_PIX_HST_CTL, CAM_A_AE_PIX_HST_CTL*/

typedef volatile union _CAM_REG_AE_PIX_HST_SET_
{
        volatile struct /* 0x1A004934 */
        {
                FIELD  AE_PIX_BIN_MODE_0                     :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  AE_PIX_BIN_MODE_1                     :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  AE_PIX_BIN_MODE_2                     :  2;      /*  8.. 9, 0x00000300 */
                FIELD  rsv_10                                :  2;      /* 10..11, 0x00000C00 */
                FIELD  AE_PIX_BIN_MODE_3                     :  2;      /* 12..13, 0x00003000 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AE_PIX_COLOR_MODE_0                   :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  AE_PIX_COLOR_MODE_1                   :  3;      /* 20..22, 0x00700000 */
                FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
                FIELD  AE_PIX_COLOR_MODE_2                   :  3;      /* 24..26, 0x07000000 */
                FIELD  rsv_27                                :  1;      /* 27..27, 0x08000000 */
                FIELD  AE_PIX_COLOR_MODE_3                   :  3;      /* 28..30, 0x70000000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST_SET;    /* CAM_AE_PIX_HST_SET, CAM_A_AE_PIX_HST_SET*/

typedef volatile union _CAM_REG_AE_PIX_HST_SET_1_
{
        volatile struct /* 0x1A004938 */
        {
                FIELD  AE_PIX_BIN_MODE_4                     :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  AE_PIX_BIN_MODE_5                     :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  AE_PIX_COLOR_MODE_4                   :  3;      /*  8..10, 0x00000700 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  AE_PIX_COLOR_MODE_5                   :  4;      /* 12..15, 0x0000F000 */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST_SET_1;  /* CAM_AE_PIX_HST_SET_1, CAM_A_AE_PIX_HST_SET_1*/

typedef volatile union _CAM_REG_AE_PIX_HST0_YRNG_
{
        volatile struct /* 0x1A00493C */
        {
                FIELD  AE_PIX_Y_LOW_0                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_0                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST0_YRNG;  /* CAM_AE_PIX_HST0_YRNG, CAM_A_AE_PIX_HST0_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST0_XRNG_
{
        volatile struct /* 0x1A004940 */
        {
                FIELD  AE_PIX_X_LOW_0                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_0                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST0_XRNG;  /* CAM_AE_PIX_HST0_XRNG, CAM_A_AE_PIX_HST0_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST1_YRNG_
{
        volatile struct /* 0x1A004944 */
        {
                FIELD  AE_PIX_Y_LOW_1                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_1                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST1_YRNG;  /* CAM_AE_PIX_HST1_YRNG, CAM_A_AE_PIX_HST1_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST1_XRNG_
{
        volatile struct /* 0x1A004948 */
        {
                FIELD  AE_PIX_X_LOW_1                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_1                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST1_XRNG;  /* CAM_AE_PIX_HST1_XRNG, CAM_A_AE_PIX_HST1_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST2_YRNG_
{
        volatile struct /* 0x1A00494C */
        {
                FIELD  AE_PIX_Y_LOW_2                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_2                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST2_YRNG;  /* CAM_AE_PIX_HST2_YRNG, CAM_A_AE_PIX_HST2_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST2_XRNG_
{
        volatile struct /* 0x1A004950 */
        {
                FIELD  AE_PIX_X_LOW_2                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_2                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST2_XRNG;  /* CAM_AE_PIX_HST2_XRNG, CAM_A_AE_PIX_HST2_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST3_YRNG_
{
        volatile struct /* 0x1A004954 */
        {
                FIELD  AE_PIX_Y_LOW_3                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_3                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST3_YRNG;  /* CAM_AE_PIX_HST3_YRNG, CAM_A_AE_PIX_HST3_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST3_XRNG_
{
        volatile struct /* 0x1A004958 */
        {
                FIELD  AE_PIX_X_LOW_3                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_3                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST3_XRNG;  /* CAM_AE_PIX_HST3_XRNG, CAM_A_AE_PIX_HST3_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST4_YRNG_
{
        volatile struct /* 0x1A00495C */
        {
                FIELD  AE_PIX_Y_LOW_4                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_4                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST4_YRNG;  /* CAM_AE_PIX_HST4_YRNG, CAM_A_AE_PIX_HST4_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST4_XRNG_
{
        volatile struct /* 0x1A004960 */
        {
                FIELD  AE_PIX_X_LOW_4                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_4                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST4_XRNG;  /* CAM_AE_PIX_HST4_XRNG, CAM_A_AE_PIX_HST4_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST5_YRNG_
{
        volatile struct /* 0x1A004964 */
        {
                FIELD  AE_PIX_Y_LOW_5                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  AE_PIX_Y_HI_5                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST5_YRNG;  /* CAM_AE_PIX_HST5_YRNG, CAM_A_AE_PIX_HST5_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST5_XRNG_
{
        volatile struct /* 0x1A004968 */
        {
                FIELD  AE_PIX_X_LOW_5                        : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  AE_PIX_X_HI_5                         : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_PIX_HST5_XRNG;  /* CAM_AE_PIX_HST5_XRNG, CAM_A_AE_PIX_HST5_XRNG*/

typedef volatile union _CAM_REG_AE_STAT_EN_
{
        volatile struct /* 0x1A00496C */
        {
                FIELD  AE_TSF_STAT_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  AE_OVERCNT_EN                         :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AE_HDR_STAT_EN                        :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AE_HST_DMA_OUT_EN                     :  1;      /*  3.. 3, 0x00000008 */
                FIELD  AE_LINEAR_STAT_EN                     :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_STAT_EN;    /* CAM_AE_STAT_EN, CAM_A_AE_STAT_EN*/

typedef volatile union _CAM_REG_AE_YCOEF_
{
        volatile struct /* 0x1A004970 */
        {
                FIELD  AE_YCOEF_R                            :  4;      /*  0.. 3, 0x0000000F */
                FIELD  AE_YCOEF_G                            :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  AE_YCOEF_B                            :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_YCOEF;  /* CAM_AE_YCOEF, CAM_A_AE_YCOEF*/

typedef volatile union _CAM_REG_AE_CCU_HST_END_Y_
{
        volatile struct /* 0x1A004974 */
        {
                FIELD  AE_CCU_HST_END_Y                      :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_CCU_HST_END_Y;  /* CAM_AE_CCU_HST_END_Y, CAM_A_AE_CCU_HST_END_Y*/

typedef volatile union _CAM_REG_AE_SPARE_
{
        volatile struct /* 0x1A004978 */
        {
                FIELD  AE_SPARE                              : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AE_SPARE;  /* CAM_AE_SPARE, CAM_A_AE_SPARE*/

typedef volatile union _CAM_REG_QBN1_MODE_
{
        volatile struct /* 0x1A0049D0 */
        {
                FIELD  QBN_ACC                               :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  QBN_ACC_MODE                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_QBN1_MODE; /* CAM_QBN1_MODE, CAM_A_QBN1_MODE*/

typedef volatile union _CAM_REG_CPG_SATU_1_
{
        volatile struct /* 0x1A0049E0 */
        {
                FIELD  CPG_SATU_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPG_SATU_GB                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_SATU_1;    /* CAM_CPG_SATU_1, CAM_A_CPG_SATU_1*/

typedef volatile union _CAM_REG_CPG_SATU_2_
{
        volatile struct /* 0x1A0049E4 */
        {
                FIELD  CPG_SATU_GR                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPG_SATU_R                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_SATU_2;    /* CAM_CPG_SATU_2, CAM_A_CPG_SATU_2*/

typedef volatile union _CAM_REG_CPG_GAIN_1_
{
        volatile struct /* 0x1A0049E8 */
        {
                FIELD  CPG_GAIN_B                            : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  CPG_GAIN_GB                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_GAIN_1;    /* CAM_CPG_GAIN_1, CAM_A_CPG_GAIN_1*/

typedef volatile union _CAM_REG_CPG_GAIN_2_
{
        volatile struct /* 0x1A0049EC */
        {
                FIELD  CPG_GAIN_GR                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  CPG_GAIN_R                            : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_GAIN_2;    /* CAM_CPG_GAIN_2, CAM_A_CPG_GAIN_2*/

typedef volatile union _CAM_REG_CPG_OFST_1_
{
        volatile struct /* 0x1A0049F0 */
        {
                FIELD  CPG_OFST_B                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPG_OFST_GB                           : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_OFST_1;    /* CAM_CPG_OFST_1, CAM_A_CPG_OFST_1*/

typedef volatile union _CAM_REG_CPG_OFST_2_
{
        volatile struct /* 0x1A0049F4 */
        {
                FIELD  CPG_OFST_GR                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPG_OFST_R                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPG_OFST_2;    /* CAM_CPG_OFST_2, CAM_A_CPG_OFST_2*/

typedef volatile union _CAM_REG_VBN_GAIN_
{
        volatile struct /* 0x1A004A30 */
        {
                FIELD  VBN_GAIN                              : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_VBN_GAIN;  /* CAM_VBN_GAIN, CAM_A_VBN_GAIN*/

typedef volatile union _CAM_REG_VBN_OFST_
{
        volatile struct /* 0x1A004A34 */
        {
                FIELD  VBN_OFST                              : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_VBN_OFST;  /* CAM_VBN_OFST, CAM_A_VBN_OFST*/

typedef volatile union _CAM_REG_VBN_TYPE_
{
        volatile struct /* 0x1A004A38 */
        {
                FIELD  VBN_TYPE                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  VBN_DIAG_SEL_EN                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAM_REG_VBN_TYPE;  /* CAM_VBN_TYPE, CAM_A_VBN_TYPE*/

typedef volatile union _CAM_REG_VBN_SPARE_
{
        volatile struct /* 0x1A004A3C */
        {
                FIELD  VBN_SPARE                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_VBN_SPARE; /* CAM_VBN_SPARE, CAM_A_VBN_SPARE*/

typedef volatile union _CAM_REG_AMX_CTL_
{
        volatile struct /* 0x1A004A50 */
        {
                FIELD  AMX_SRAM_SIZE                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  AMX_EDGE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  AMX_SINGLE_MODE_1                     :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  3;      /* 21..23, 0x00E00000 */
                FIELD  AMX_SINGLE_MODE_2                     :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  6;      /* 25..30, 0x7E000000 */
                FIELD  AMX_EDGE_SET                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AMX_CTL;   /* CAM_AMX_CTL, CAM_A_AMX_CTL*/

typedef volatile union _CAM_REG_AMX_CROP_
{
        volatile struct /* 0x1A004A54 */
        {
                FIELD  AMX_STR_X                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  AMX_END_X                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AMX_CROP;  /* CAM_AMX_CROP, CAM_A_AMX_CROP*/

typedef volatile union _CAM_REG_AMX_VSIZE_
{
        volatile struct /* 0x1A004A58 */
        {
                FIELD  AMX_HT                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AMX_VSIZE; /* CAM_AMX_VSIZE, CAM_A_AMX_VSIZE*/

typedef volatile union _CAM_REG_BIN_CTL_
{
        volatile struct /* 0x1A004A70 */
        {
                FIELD  BIN_TYPE                              :  2;      /*  0.. 1, 0x00000003 */
                FIELD  BIN_MODE                              :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  BIN_DMD                               :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  BIN_OV_TH                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  BIN_LE_INV_CTL                        :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BIN_CTL;   /* CAM_BIN_CTL, CAM_A_BIN_CTL*/

typedef volatile union _CAM_REG_BIN_FTH_
{
        volatile struct /* 0x1A004A74 */
        {
                FIELD  BIN_FTH1                              :  8;      /*  0.. 7, 0x000000FF */
                FIELD  BIN_FTH2                              :  8;      /*  8..15, 0x0000FF00 */
                FIELD  BIN_FTH3                              :  8;      /* 16..23, 0x00FF0000 */
                FIELD  BIN_FTH4                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BIN_FTH;   /* CAM_BIN_FTH, CAM_A_BIN_FTH*/

typedef volatile union _CAM_REG_BIN_SPARE_
{
        volatile struct /* 0x1A004A78 */
        {
                FIELD  BIN_SPARE                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BIN_SPARE; /* CAM_BIN_SPARE, CAM_A_BIN_SPARE*/

typedef volatile union _CAM_REG_DBN_GAIN_
{
        volatile struct /* 0x1A004A90 */
        {
                FIELD  DBN_GAIN                              : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBN_GAIN;  /* CAM_DBN_GAIN, CAM_A_DBN_GAIN*/

typedef volatile union _CAM_REG_DBN_OFST_
{
        volatile struct /* 0x1A004A94 */
        {
                FIELD  DBN_OFST                              : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBN_OFST;  /* CAM_DBN_OFST, CAM_A_DBN_OFST*/

typedef volatile union _CAM_REG_DBN_SPARE_
{
        volatile struct /* 0x1A004A98 */
        {
                FIELD  DBN_SPARE                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBN_SPARE; /* CAM_DBN_SPARE, CAM_A_DBN_SPARE*/

typedef volatile union _CAM_REG_PBN_TYPE_
{
        volatile struct /* 0x1A004AA0 */
        {
                FIELD  PBN_TYPE                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  PBN_SEP                               :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PBN_OBIT                              :  4;      /*  2.. 5, 0x0000003C */
                FIELD  rsv_6                                 : 26;      /*  6..31, 0xFFFFFFC0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PBN_TYPE;  /* CAM_PBN_TYPE, CAM_A_PBN_TYPE*/

typedef volatile union _CAM_REG_PBN_LST_
{
        volatile struct /* 0x1A004AA4 */
        {
                FIELD  PBN_LST                               :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAM_REG_PBN_LST;   /* CAM_PBN_LST, CAM_A_PBN_LST*/

typedef volatile union _CAM_REG_PBN_VSIZE_
{
        volatile struct /* 0x1A004AA8 */
        {
                FIELD  PBN_VSIZE                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PBN_VSIZE; /* CAM_PBN_VSIZE, CAM_A_PBN_VSIZE*/

typedef volatile union _CAM_REG_RCP3_CROP_CON1_
{
        volatile struct /* 0x1A004AB0 */
        {
                FIELD  RCP_STR_X                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  RCP_END_X                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RCP3_CROP_CON1;    /* CAM_RCP3_CROP_CON1, CAM_A_RCP3_CROP_CON1*/

typedef volatile union _CAM_REG_RCP3_CROP_CON2_
{
        volatile struct /* 0x1A004AB4 */
        {
                FIELD  RCP_STR_Y                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  RCP_END_Y                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RCP3_CROP_CON2;    /* CAM_RCP3_CROP_CON2, CAM_A_RCP3_CROP_CON2*/

typedef volatile union _CAM_REG_DBS_SIGMA_
{
        volatile struct /* 0x1A004AC0 */
        {
                FIELD  DBS_OFST                              : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  DBS_SL                                :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SIGMA; /* CAM_DBS_SIGMA, CAM_A_DBS_SIGMA*/

typedef volatile union _CAM_REG_DBS_BSTBL_0_
{
        volatile struct /* 0x1A004AC4 */
        {
                FIELD  DBS_BIAS_Y0                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DBS_BIAS_Y1                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  DBS_BIAS_Y2                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  DBS_BIAS_Y3                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_BSTBL_0;   /* CAM_DBS_BSTBL_0, CAM_A_DBS_BSTBL_0*/

typedef volatile union _CAM_REG_DBS_BSTBL_1_
{
        volatile struct /* 0x1A004AC8 */
        {
                FIELD  DBS_BIAS_Y4                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DBS_BIAS_Y5                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  DBS_BIAS_Y6                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  DBS_BIAS_Y7                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_BSTBL_1;   /* CAM_DBS_BSTBL_1, CAM_A_DBS_BSTBL_1*/

typedef volatile union _CAM_REG_DBS_BSTBL_2_
{
        volatile struct /* 0x1A004ACC */
        {
                FIELD  DBS_BIAS_Y8                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DBS_BIAS_Y9                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  DBS_BIAS_Y10                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  DBS_BIAS_Y11                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_BSTBL_2;   /* CAM_DBS_BSTBL_2, CAM_A_DBS_BSTBL_2*/

typedef volatile union _CAM_REG_DBS_BSTBL_3_
{
        volatile struct /* 0x1A004AD0 */
        {
                FIELD  DBS_BIAS_Y12                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DBS_BIAS_Y13                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  DBS_BIAS_Y14                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_BSTBL_3;   /* CAM_DBS_BSTBL_3, CAM_A_DBS_BSTBL_3*/

typedef volatile union _CAM_REG_DBS_CTL_
{
        volatile struct /* 0x1A004AD4 */
        {
                FIELD  DBS_HDR_GNP                           :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  DBS_SL_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  DBS_LE_INV_CTL                        :  4;      /*  8..11, 0x00000F00 */
                FIELD  DBS_EDGE                              :  4;      /* 12..15, 0x0000F000 */
                FIELD  DBS_HDR_GAIN                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  DBS_HDR_GAIN2                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_CTL;   /* CAM_DBS_CTL, CAM_A_DBS_CTL*/

typedef volatile union _CAM_REG_DBS_CTL_2_
{
        volatile struct /* 0x1A004AD8 */
        {
                FIELD  DBS_HDR_OSCTH                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_CTL_2; /* CAM_DBS_CTL_2, CAM_A_DBS_CTL_2*/

typedef volatile union _CAM_REG_DBS_SIGMA_2_
{
        volatile struct /* 0x1A004ADC */
        {
                FIELD  DBS_MUL_B                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  DBS_MUL_GB                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  DBS_MUL_GR                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  DBS_MUL_R                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SIGMA_2;   /* CAM_DBS_SIGMA_2, CAM_A_DBS_SIGMA_2*/

typedef volatile union _CAM_REG_DBS_YGN_
{
        volatile struct /* 0x1A004AE0 */
        {
                FIELD  DBS_YGN_B                             :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  DBS_YGN_GB                            :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  DBS_YGN_GR                            :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DBS_YGN_R                             :  6;      /* 24..29, 0x3F000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_YGN;   /* CAM_DBS_YGN, CAM_A_DBS_YGN*/

typedef volatile union _CAM_REG_DBS_SL_Y12_
{
        volatile struct /* 0x1A004AE4 */
        {
                FIELD  DBS_SL_Y1                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  DBS_SL_Y2                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SL_Y12;    /* CAM_DBS_SL_Y12, CAM_A_DBS_SL_Y12*/

typedef volatile union _CAM_REG_DBS_SL_Y34_
{
        volatile struct /* 0x1A004AE8 */
        {
                FIELD  DBS_SL_Y3                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  DBS_SL_Y4                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SL_Y34;    /* CAM_DBS_SL_Y34, CAM_A_DBS_SL_Y34*/

typedef volatile union _CAM_REG_DBS_SL_G12_
{
        volatile struct /* 0x1A004AEC */
        {
                FIELD  DBS_SL_G1                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  DBS_SL_G2                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SL_G12;    /* CAM_DBS_SL_G12, CAM_A_DBS_SL_G12*/

typedef volatile union _CAM_REG_DBS_SL_G34_
{
        volatile struct /* 0x1A004AF0 */
        {
                FIELD  DBS_SL_G3                             : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  DBS_SL_G4                             : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DBS_SL_G34;    /* CAM_DBS_SL_G34, CAM_A_DBS_SL_G34*/

typedef volatile union _CAM_REG_SL2F_CEN_
{
        volatile struct /* 0x1A004B00 */
        {
                FIELD  SL2_CENTR_X                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_CENTR_Y                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_CEN;  /* CAM_SL2F_CEN, CAM_A_SL2F_CEN*/

typedef volatile union _CAM_REG_SL2F_RR_CON0_
{
        volatile struct /* 0x1A004B04 */
        {
                FIELD  SL2_R_0                               : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_R_1                               : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_RR_CON0;  /* CAM_SL2F_RR_CON0, CAM_A_SL2F_RR_CON0*/

typedef volatile union _CAM_REG_SL2F_RR_CON1_
{
        volatile struct /* 0x1A004B08 */
        {
                FIELD  SL2_R_2                               : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_GAIN_0                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SL2_GAIN_1                            :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_RR_CON1;  /* CAM_SL2F_RR_CON1, CAM_A_SL2F_RR_CON1*/

typedef volatile union _CAM_REG_SL2F_GAIN_
{
        volatile struct /* 0x1A004B0C */
        {
                FIELD  SL2_GAIN_2                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SL2_GAIN_3                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SL2_GAIN_4                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SL2_SET_ZERO                          :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_GAIN; /* CAM_SL2F_GAIN, CAM_A_SL2F_GAIN*/

typedef volatile union _CAM_REG_SL2F_RZ_
{
        volatile struct /* 0x1A004B10 */
        {
                FIELD  SL2_HRZ_COMP                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_VRZ_COMP                          : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_RZ;   /* CAM_SL2F_RZ, CAM_A_SL2F_RZ*/

typedef volatile union _CAM_REG_SL2F_XOFF_
{
        volatile struct /* 0x1A004B14 */
        {
                FIELD  SL2_X_OFST                            : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_XOFF; /* CAM_SL2F_XOFF, CAM_A_SL2F_XOFF*/

typedef volatile union _CAM_REG_SL2F_YOFF_
{
        volatile struct /* 0x1A004B18 */
        {
                FIELD  SL2_Y_OFST                            : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_YOFF; /* CAM_SL2F_YOFF, CAM_A_SL2F_YOFF*/

typedef volatile union _CAM_REG_SL2F_SLP_CON0_
{
        volatile struct /* 0x1A004B1C */
        {
                FIELD  SL2_SLP_1                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_SLP_CON0; /* CAM_SL2F_SLP_CON0, CAM_A_SL2F_SLP_CON0*/

typedef volatile union _CAM_REG_SL2F_SLP_CON1_
{
        volatile struct /* 0x1A004B20 */
        {
                FIELD  SL2_SLP_2                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_SLP_CON1; /* CAM_SL2F_SLP_CON1, CAM_A_SL2F_SLP_CON1*/

typedef volatile union _CAM_REG_SL2F_SLP_CON2_
{
        volatile struct /* 0x1A004B24 */
        {
                FIELD  SL2_SLP_3                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_SLP_CON2; /* CAM_SL2F_SLP_CON2, CAM_A_SL2F_SLP_CON2*/

typedef volatile union _CAM_REG_SL2F_SLP_CON3_
{
        volatile struct /* 0x1A004B28 */
        {
                FIELD  SL2_SLP_4                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_SLP_CON3; /* CAM_SL2F_SLP_CON3, CAM_A_SL2F_SLP_CON3*/

typedef volatile union _CAM_REG_SL2F_SIZE_
{
        volatile struct /* 0x1A004B2C */
        {
                FIELD  SL2_TPIPE_WD                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_TPIPE_HT                          : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2F_SIZE; /* CAM_SL2F_SIZE, CAM_A_SL2F_SIZE*/

typedef volatile union _CAM_REG_SL2J_CEN_
{
        volatile struct /* 0x1A004B40 */
        {
                FIELD  SL2_CENTR_X                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_CENTR_Y                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_CEN;  /* CAM_SL2J_CEN, CAM_A_SL2J_CEN*/

typedef volatile union _CAM_REG_SL2J_RR_CON0_
{
        volatile struct /* 0x1A004B44 */
        {
                FIELD  SL2_R_0                               : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_R_1                               : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_RR_CON0;  /* CAM_SL2J_RR_CON0, CAM_A_SL2J_RR_CON0*/

typedef volatile union _CAM_REG_SL2J_RR_CON1_
{
        volatile struct /* 0x1A004B48 */
        {
                FIELD  SL2_R_2                               : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_GAIN_0                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SL2_GAIN_1                            :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_RR_CON1;  /* CAM_SL2J_RR_CON1, CAM_A_SL2J_RR_CON1*/

typedef volatile union _CAM_REG_SL2J_GAIN_
{
        volatile struct /* 0x1A004B4C */
        {
                FIELD  SL2_GAIN_2                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SL2_GAIN_3                            :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SL2_GAIN_4                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SL2_SET_ZERO                          :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_GAIN; /* CAM_SL2J_GAIN, CAM_A_SL2J_GAIN*/

typedef volatile union _CAM_REG_SL2J_RZ_
{
        volatile struct /* 0x1A004B50 */
        {
                FIELD  SL2_HRZ_COMP                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_VRZ_COMP                          : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_RZ;   /* CAM_SL2J_RZ, CAM_A_SL2J_RZ*/

typedef volatile union _CAM_REG_SL2J_XOFF_
{
        volatile struct /* 0x1A004B54 */
        {
                FIELD  SL2_X_OFST                            : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_XOFF; /* CAM_SL2J_XOFF, CAM_A_SL2J_XOFF*/

typedef volatile union _CAM_REG_SL2J_YOFF_
{
        volatile struct /* 0x1A004B58 */
        {
                FIELD  SL2_Y_OFST                            : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_YOFF; /* CAM_SL2J_YOFF, CAM_A_SL2J_YOFF*/

typedef volatile union _CAM_REG_SL2J_SLP_CON0_
{
        volatile struct /* 0x1A004B5C */
        {
                FIELD  SL2_SLP_1                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_SLP_CON0; /* CAM_SL2J_SLP_CON0, CAM_A_SL2J_SLP_CON0*/

typedef volatile union _CAM_REG_SL2J_SLP_CON1_
{
        volatile struct /* 0x1A004B60 */
        {
                FIELD  SL2_SLP_2                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_SLP_CON1; /* CAM_SL2J_SLP_CON1, CAM_A_SL2J_SLP_CON1*/

typedef volatile union _CAM_REG_SL2J_SLP_CON2_
{
        volatile struct /* 0x1A004B64 */
        {
                FIELD  SL2_SLP_3                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_SLP_CON2; /* CAM_SL2J_SLP_CON2, CAM_A_SL2J_SLP_CON2*/

typedef volatile union _CAM_REG_SL2J_SLP_CON3_
{
        volatile struct /* 0x1A004B68 */
        {
                FIELD  SL2_SLP_4                             : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_SLP_CON3; /* CAM_SL2J_SLP_CON3, CAM_A_SL2J_SLP_CON3*/

typedef volatile union _CAM_REG_SL2J_SIZE_
{
        volatile struct /* 0x1A004B6C */
        {
                FIELD  SL2_TPIPE_WD                          : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SL2_TPIPE_HT                          : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SL2J_SIZE; /* CAM_SL2J_SIZE, CAM_A_SL2J_SIZE*/

typedef volatile union _CAM_REG_PCP_CROP_CON1_
{
        volatile struct /* 0x1A004B80 */
        {
                FIELD  PCP_STR_X                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PCP_END_X                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PCP_CROP_CON1; /* CAM_PCP_CROP_CON1, CAM_A_PCP_CROP_CON1*/

typedef volatile union _CAM_REG_PCP_CROP_CON2_
{
        volatile struct /* 0x1A004B84 */
        {
                FIELD  PCP_STR_Y                             : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PCP_END_Y                             : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PCP_CROP_CON2; /* CAM_PCP_CROP_CON2, CAM_A_PCP_CROP_CON2*/

typedef volatile union _CAM_REG_SGG2_PGN_
{
        volatile struct /* 0x1A004B90 */
        {
                FIELD  SGG_GAIN                              : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG2_PGN;  /* CAM_SGG2_PGN, CAM_A_SGG2_PGN*/

typedef volatile union _CAM_REG_SGG2_GMRC_1_
{
        volatile struct /* 0x1A004B94 */
        {
                FIELD  SGG_GMR_1                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_2                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_3                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGG_GMR_4                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG2_GMRC_1;   /* CAM_SGG2_GMRC_1, CAM_A_SGG2_GMRC_1*/

typedef volatile union _CAM_REG_SGG2_GMRC_2_
{
        volatile struct /* 0x1A004B98 */
        {
                FIELD  SGG_GMR_5                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_6                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_7                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG2_GMRC_2;   /* CAM_SGG2_GMRC_2, CAM_A_SGG2_GMRC_2*/

typedef volatile union _CAM_REG_PSB_CON_
{
        volatile struct /* 0x1A004BA0 */
        {
                FIELD  PSB_MODE                              :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  PSB_STR                               :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSB_CON;   /* CAM_PSB_CON, CAM_A_PSB_CON*/

typedef volatile union _CAM_REG_PSB_SIZE_
{
        volatile struct /* 0x1A004BA4 */
        {
                FIELD  PSB_WD                                : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PSB_HT                                : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSB_SIZE;  /* CAM_PSB_SIZE, CAM_A_PSB_SIZE*/

typedef volatile union _CAM_REG_PDE_TBLI1_
{
        volatile struct /* 0x1A004BB0 */
        {
                FIELD  PDE_XOFFSET                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PDE_YOFFSET                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDE_TBLI1; /* CAM_PDE_TBLI1, CAM_A_PDE_TBLI1*/

typedef volatile union _CAM_REG_PDE_CTL_
{
        volatile struct /* 0x1A004BB4 */
        {
                FIELD  PDE_OBIT                              :  4;      /*  0.. 3, 0x0000000F */
                FIELD  rsv_4                                 : 28;      /*  4..31, 0xFFFFFFF0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDE_CTL;   /* CAM_PDE_CTL, CAM_A_PDE_CTL*/

typedef volatile union _CAM_REG_RMB_MODE_
{
        volatile struct /* 0x1A004BC0 */
        {
                FIELD  ACC                                   :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  ACC_MODE                              :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  1;      /*  5.. 5, 0x00000020 */
                FIELD  SENSOR_TYPE                           :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RMB_MODE;  /* CAM_RMB_MODE, CAM_A_RMB_MODE*/

typedef volatile union _CAM_REG_PS_AWB_WIN_ORG_
{
        volatile struct /* 0x1A004BD0 */
        {
                FIELD  AWB_W_HORG                            : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VORG                            : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_WIN_ORG;    /* CAM_PS_AWB_WIN_ORG, CAM_A_PS_AWB_WIN_ORG*/

typedef volatile union _CAM_REG_PS_AWB_WIN_SIZE_
{
        volatile struct /* 0x1A004BD4 */
        {
                FIELD  AWB_W_HSIZE                           : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VSIZE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_WIN_SIZE;   /* CAM_PS_AWB_WIN_SIZE, CAM_A_PS_AWB_WIN_SIZE*/

typedef volatile union _CAM_REG_PS_AWB_WIN_PIT_
{
        volatile struct /* 0x1A004BD8 */
        {
                FIELD  AWB_W_HPIT                            : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  AWB_W_VPIT                            : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_WIN_PIT;    /* CAM_PS_AWB_WIN_PIT, CAM_A_PS_AWB_WIN_PIT*/

typedef volatile union _CAM_REG_PS_AWB_WIN_NUM_
{
        volatile struct /* 0x1A004BDC */
        {
                FIELD  AWB_W_HNUM                            :  8;      /*  0.. 7, 0x000000FF */
                FIELD  rsv_8                                 :  8;      /*  8..15, 0x0000FF00 */
                FIELD  AWB_W_VNUM                            :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_WIN_NUM;    /* CAM_PS_AWB_WIN_NUM, CAM_A_PS_AWB_WIN_NUM*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT0_
{
        volatile struct /* 0x1A004BE0 */
        {
                FIELD  AWB_PIXEL_CNT0                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT0; /* CAM_PS_AWB_PIXEL_CNT0, CAM_A_PS_AWB_PIXEL_CNT0*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT1_
{
        volatile struct /* 0x1A004BE4 */
        {
                FIELD  AWB_PIXEL_CNT1                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT1; /* CAM_PS_AWB_PIXEL_CNT1, CAM_A_PS_AWB_PIXEL_CNT1*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT2_
{
        volatile struct /* 0x1A004BE8 */
        {
                FIELD  AWB_PIXEL_CNT2                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT2; /* CAM_PS_AWB_PIXEL_CNT2, CAM_A_PS_AWB_PIXEL_CNT2*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT3_
{
        volatile struct /* 0x1A004BEC */
        {
                FIELD  AWB_PIXEL_CNT3                        : 24;      /*  0..23, 0x00FFFFFF */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT3; /* CAM_PS_AWB_PIXEL_CNT3, CAM_A_PS_AWB_PIXEL_CNT3*/

typedef volatile union _CAM_REG_PS_AE_YCOEF0_
{
        volatile struct /* 0x1A004BF0 */
        {
                FIELD  AE_YCOEF_R                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  AE_YCOEF_GR                           : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AE_YCOEF0;  /* CAM_PS_AE_YCOEF0, CAM_A_PS_AE_YCOEF0*/

typedef volatile union _CAM_REG_PS_AE_YCOEF1_
{
        volatile struct /* 0x1A004BF4 */
        {
                FIELD  AE_YCOEF_GB                           : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  AE_YCOEF_B                            : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_AE_YCOEF1;  /* CAM_PS_AE_YCOEF1, CAM_A_PS_AE_YCOEF1*/

typedef volatile union _CAM_REG_PS_DATA_TYPE_
{
        volatile struct /* 0x1A004BF8 */
        {
                FIELD  SENSOR_TYPE                           :  2;      /*  0.. 1, 0x00000003 */
                FIELD  G_LE_FIRST                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  R_LE_FIRST                            :  1;      /*  3.. 3, 0x00000008 */
                FIELD  B_LE_FIRST                            :  1;      /*  4.. 4, 0x00000010 */
                FIELD  EXP_MODE                              :  2;      /*  5.. 6, 0x00000060 */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_DATA_TYPE;  /* CAM_PS_DATA_TYPE, CAM_A_PS_DATA_TYPE*/

typedef volatile union _CAM_REG_PS_HST_CFG_
{
        volatile struct /* 0x1A004BFC */
        {
                FIELD  PS_HST_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PS_HST_SEP_G                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PS_CCU_HST_END_Y                      :  8;      /*  3..10, 0x000007F8 */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_HST_CFG;    /* CAM_PS_HST_CFG, CAM_A_PS_HST_CFG*/

typedef volatile union _CAM_REG_PS_HST_ROI_X_
{
        volatile struct /* 0x1A004C00 */
        {
                FIELD  PS_X_LOW                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PS_X_HI                               : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_HST_ROI_X;  /* CAM_PS_HST_ROI_X, CAM_A_PS_HST_ROI_X*/

typedef volatile union _CAM_REG_PS_HST_ROI_Y_
{
        volatile struct /* 0x1A004C04 */
        {
                FIELD  PS_Y_LOW                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PS_Y_HI                               : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PS_HST_ROI_Y;  /* CAM_PS_HST_ROI_Y, CAM_A_PS_HST_ROI_Y*/

typedef volatile union _CAM_REG_QBN3_MODE_
{
        volatile struct /* 0x1A004C30 */
        {
                FIELD  QBN_ACC                               :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  QBN_ACC_MODE                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_QBN3_MODE; /* CAM_QBN3_MODE, CAM_A_QBN3_MODE*/

typedef volatile union _CAM_REG_SGG3_PGN_
{
        volatile struct /* 0x1A004C40 */
        {
                FIELD  SGG_GAIN                              : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG3_PGN;  /* CAM_SGG3_PGN, CAM_A_SGG3_PGN*/

typedef volatile union _CAM_REG_SGG3_GMRC_1_
{
        volatile struct /* 0x1A004C44 */
        {
                FIELD  SGG_GMR_1                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_2                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_3                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGG_GMR_4                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG3_GMRC_1;   /* CAM_SGG3_GMRC_1, CAM_A_SGG3_GMRC_1*/

typedef volatile union _CAM_REG_SGG3_GMRC_2_
{
        volatile struct /* 0x1A004C48 */
        {
                FIELD  SGG_GMR_5                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_6                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_7                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG3_GMRC_2;   /* CAM_SGG3_GMRC_2, CAM_A_SGG3_GMRC_2*/

typedef volatile union _CAM_REG_FLK_A_CON_
{
        volatile struct /* 0x1A004C50 */
        {
                FIELD  RESERVED                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLK_A_CON; /* CAM_FLK_A_CON, CAM_A_FLK_A_CON*/

typedef volatile union _CAM_REG_FLK_A_OFST_
{
        volatile struct /* 0x1A004C54 */
        {
                FIELD  FLK_OFST_X                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FLK_OFST_Y                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLK_A_OFST;    /* CAM_FLK_A_OFST, CAM_A_FLK_A_OFST*/

typedef volatile union _CAM_REG_FLK_A_SIZE_
{
        volatile struct /* 0x1A004C58 */
        {
                FIELD  FLK_SIZE_X                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FLK_SIZE_Y                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLK_A_SIZE;    /* CAM_FLK_A_SIZE, CAM_A_FLK_A_SIZE*/

typedef volatile union _CAM_REG_FLK_A_NUM_
{
        volatile struct /* 0x1A004C5C */
        {
                FIELD  FLK_NUM_X                             :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FLK_NUM_Y                             :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLK_A_NUM; /* CAM_FLK_A_NUM, CAM_A_FLK_A_NUM*/

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL1_
{
        volatile struct /* 0x1A004C70 */
        {
                FIELD  EIS_OP_HORI                           :  3;      /*  0.. 2, 0x00000007 */
                FIELD  EIS_OP_VERT                           :  3;      /*  3.. 5, 0x00000038 */
                FIELD  EIS_SUBG_EN                           :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  EIS_NUM_HRP                           :  5;      /*  8..12, 0x00001F00 */
                FIELD  EIS_KNEE_2                            :  4;      /* 13..16, 0x0001E000 */
                FIELD  EIS_KNEE_1                            :  4;      /* 17..20, 0x001E0000 */
                FIELD  EIS_NUM_VRP                           :  4;      /* 21..24, 0x01E00000 */
                FIELD  EIS_NUM_HWIN                          :  3;      /* 25..27, 0x0E000000 */
                FIELD  EIS_NUM_VWIN                          :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_PREP_ME_CTRL1; /* CAM_EIS_PREP_ME_CTRL1, CAM_A_EIS_PREP_ME_CTRL1*/

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL2_
{
        volatile struct /* 0x1A004C74 */
        {
                FIELD  EIS_PROC_GAIN                         :  2;      /*  0.. 1, 0x00000003 */
                FIELD  EIS_HORI_SHR                          :  3;      /*  2.. 4, 0x0000001C */
                FIELD  EIS_VERT_SHR                          :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  EIS_DC_DL                             :  6;      /*  8..13, 0x00003F00 */
                FIELD  EIS_RP_MODI                           :  1;      /* 14..14, 0x00004000 */
                FIELD  EIS_FIRST_FRM                         :  1;      /* 15..15, 0x00008000 */
                FIELD  EIS_SPARE                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_PREP_ME_CTRL2; /* CAM_EIS_PREP_ME_CTRL2, CAM_A_EIS_PREP_ME_CTRL2*/

typedef volatile union _CAM_REG_EIS_LMV_TH_
{
        volatile struct /* 0x1A004C78 */
        {
                FIELD  LMV_TH_Y_SURROUND                     :  8;      /*  0.. 7, 0x000000FF */
                FIELD  LMV_TH_Y_CENTER                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  LMV_TH_X_SOURROUND                    :  8;      /* 16..23, 0x00FF0000 */
                FIELD  LMV_TH_X_CENTER                       :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_LMV_TH;    /* CAM_EIS_LMV_TH, CAM_A_EIS_LMV_TH*/

typedef volatile union _CAM_REG_EIS_FL_OFFSET_
{
        volatile struct /* 0x1A004C7C */
        {
                FIELD  EIS_WIN_VOFST                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  EIS_WIN_HOFST                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_FL_OFFSET; /* CAM_EIS_FL_OFFSET, CAM_A_EIS_FL_OFFSET*/

typedef volatile union _CAM_REG_EIS_MB_OFFSET_
{
        volatile struct /* 0x1A004C80 */
        {
                FIELD  EIS_RP_VOFST                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  EIS_RP_HOFST                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_MB_OFFSET; /* CAM_EIS_MB_OFFSET, CAM_A_EIS_MB_OFFSET*/

typedef volatile union _CAM_REG_EIS_MB_INTERVAL_
{
        volatile struct /* 0x1A004C84 */
        {
                FIELD  EIS_WIN_VSIZE                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  EIS_WIN_HSIZE                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_MB_INTERVAL;   /* CAM_EIS_MB_INTERVAL, CAM_A_EIS_MB_INTERVAL*/

typedef volatile union _CAM_REG_EIS_GMV_
{
        volatile struct /* 0x1A004C88 */
        {
                FIELD  GMV_Y                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  GMV_X                                 : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_GMV;   /* CAM_EIS_GMV, CAM_A_EIS_GMV*/

typedef volatile union _CAM_REG_EIS_ERR_CTRL_
{
        volatile struct /* 0x1A004C8C */
        {
                FIELD  ERR_STATUS                            :  4;      /*  0.. 3, 0x0000000F */
                FIELD  CHK_SUM_EN                            :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  CHK_SUM_OUT                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ERR_MASK                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  rsv_20                                : 11;      /* 20..30, 0x7FF00000 */
                FIELD  CLEAR_ERR                             :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_ERR_CTRL;  /* CAM_EIS_ERR_CTRL, CAM_A_EIS_ERR_CTRL*/

typedef volatile union _CAM_REG_EIS_IMAGE_CTRL_
{
        volatile struct /* 0x1A004C90 */
        {
                FIELD  HEIGHT                                : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WIDTH                                 : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  1;      /* 30..30, 0x40000000 */
                FIELD  PIPE_MODE                             :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_EIS_IMAGE_CTRL;    /* CAM_EIS_IMAGE_CTRL, CAM_A_EIS_IMAGE_CTRL*/

typedef volatile union _CAM_REG_SGG5_PGN_
{
        volatile struct /* 0x1A004CB0 */
        {
                FIELD  SGG_GAIN                              : 11;      /*  0..10, 0x000007FF */
                FIELD  rsv_11                                : 21;      /* 11..31, 0xFFFFF800 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG5_PGN;  /* CAM_SGG5_PGN, CAM_A_SGG5_PGN*/

typedef volatile union _CAM_REG_SGG5_GMRC_1_
{
        volatile struct /* 0x1A004CB4 */
        {
                FIELD  SGG_GMR_1                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_2                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_3                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGG_GMR_4                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG5_GMRC_1;   /* CAM_SGG5_GMRC_1, CAM_A_SGG5_GMRC_1*/

typedef volatile union _CAM_REG_SGG5_GMRC_2_
{
        volatile struct /* 0x1A004CB8 */
        {
                FIELD  SGG_GMR_5                             :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGG_GMR_6                             :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGG_GMR_7                             :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGG5_GMRC_2;   /* CAM_SGG5_GMRC_2, CAM_A_SGG5_GMRC_2*/

typedef volatile union _CAM_REG_HDS_MODE_
{
        volatile struct /* 0x1A004CC0 */
        {
                FIELD  HDS_DS                                :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAM_REG_HDS_MODE;  /* CAM_HDS_MODE, CAM_A_HDS_MODE*/

typedef volatile union _CAM_REG_RSS_A_CONTROL_
{
        volatile struct /* 0x1A004CE0 */
        {
                FIELD  RSS_HORI_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RSS_VERT_EN                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  RSS_OUTPUT_WAIT_EN                    :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  RSS_VERT_FIRST                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 11;      /*  5..15, 0x0000FFE0 */
                FIELD  RSS_HORI_TBL_SEL                      :  5;      /* 16..20, 0x001F0000 */
                FIELD  RSS_VERT_TBL_SEL                      :  5;      /* 21..25, 0x03E00000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_CONTROL; /* CAM_RSS_A_CONTROL, CAM_A_RSS_A_CONTROL*/

typedef volatile union _CAM_REG_RSS_A_IN_IMG_
{
        volatile struct /* 0x1A004CE4 */
        {
                FIELD  RSS_IN_WD                             : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RSS_IN_HT                             : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_IN_IMG;  /* CAM_RSS_A_IN_IMG, CAM_A_RSS_A_IN_IMG*/

typedef volatile union _CAM_REG_RSS_A_OUT_IMG_
{
        volatile struct /* 0x1A004CE8 */
        {
                FIELD  RSS_OUT_WD                            : 16;      /*  0..15, 0x0000FFFF */
                FIELD  RSS_OUT_HT                            : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_OUT_IMG; /* CAM_RSS_A_OUT_IMG, CAM_A_RSS_A_OUT_IMG*/

typedef volatile union _CAM_REG_RSS_A_HORI_STEP_
{
        volatile struct /* 0x1A004CEC */
        {
                FIELD  RSS_HORI_STEP                         : 21;      /*  0..20, 0x001FFFFF */
                FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_HORI_STEP;   /* CAM_RSS_A_HORI_STEP, CAM_A_RSS_A_HORI_STEP*/

typedef volatile union _CAM_REG_RSS_A_VERT_STEP_
{
        volatile struct /* 0x1A004CF0 */
        {
                FIELD  RSS_VERT_STEP                         : 21;      /*  0..20, 0x001FFFFF */
                FIELD  rsv_21                                : 11;      /* 21..31, 0xFFE00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_VERT_STEP;   /* CAM_RSS_A_VERT_STEP, CAM_A_RSS_A_VERT_STEP*/

typedef volatile union _CAM_REG_RSS_A_HORI_INT_OFST_
{
        volatile struct /* 0x1A004CF4 */
        {
                FIELD  RSS_HORI_INT_OFST                     : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_HORI_INT_OFST;   /* CAM_RSS_A_HORI_INT_OFST, CAM_A_RSS_A_HORI_INT_OFST*/

typedef volatile union _CAM_REG_RSS_A_HORI_SUB_OFST_
{
        volatile struct /* 0x1A004CF8 */
        {
                FIELD  RSS_HORI_SUB_OFST                     : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_HORI_SUB_OFST;   /* CAM_RSS_A_HORI_SUB_OFST, CAM_A_RSS_A_HORI_SUB_OFST*/

typedef volatile union _CAM_REG_RSS_A_VERT_INT_OFST_
{
        volatile struct /* 0x1A004CFC */
        {
                FIELD  RSS_VERT_INT_OFST                     : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_VERT_INT_OFST;   /* CAM_RSS_A_VERT_INT_OFST, CAM_A_RSS_A_VERT_INT_OFST*/

typedef volatile union _CAM_REG_RSS_A_VERT_SUB_OFST_
{
        volatile struct /* 0x1A004D00 */
        {
                FIELD  RSS_VERT_SUB_OFST                     : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSS_A_VERT_SUB_OFST;   /* CAM_RSS_A_VERT_SUB_OFST, CAM_A_RSS_A_VERT_SUB_OFST*/

typedef volatile union _CAM_REG_UFE_CON_
{
        volatile struct /* 0x1A004D20 */
        {
                FIELD  UFE_FORCE_PCM                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  UFE_TCCT_BYP                          :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFE_CON;   /* CAM_UFE_CON, CAM_A_UFE_CON*/

typedef volatile union _CAM_REG_ADBS_CTL_
{
        volatile struct /* 0x1A004D50 */
        {
                FIELD  ADBS_EDGE                             :  4;      /*  0.. 3, 0x0000000F */
                FIELD  ADBS_LE_INV_CTL                       :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_CTL;  /* CAM_ADBS_CTL, CAM_A_ADBS_CTL*/

typedef volatile union _CAM_REG_ADBS_GRAY_BLD_0_
{
        volatile struct /* 0x1A004D54 */
        {
                FIELD  ADBS_LUMA_MODE                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  ADBS_BLD_MXRT                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_GRAY_BLD_0;   /* CAM_ADBS_GRAY_BLD_0, CAM_A_ADBS_GRAY_BLD_0*/

typedef volatile union _CAM_REG_ADBS_GRAY_BLD_1_
{
        volatile struct /* 0x1A004D58 */
        {
                FIELD  ADBS_BLD_LOW                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  ADBS_BLD_SLP                          : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_GRAY_BLD_1;   /* CAM_ADBS_GRAY_BLD_1, CAM_A_ADBS_GRAY_BLD_1*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_R0_
{
        volatile struct /* 0x1A004D5C */
        {
                FIELD  ADBS_BIAS_R0                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_R1                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_R2                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_R3                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_R0;  /* CAM_ADBS_BIAS_LUT_R0, CAM_A_ADBS_BIAS_LUT_R0*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_R1_
{
        volatile struct /* 0x1A004D60 */
        {
                FIELD  ADBS_BIAS_R4                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_R5                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_R6                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_R7                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_R1;  /* CAM_ADBS_BIAS_LUT_R1, CAM_A_ADBS_BIAS_LUT_R1*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_R2_
{
        volatile struct /* 0x1A004D64 */
        {
                FIELD  ADBS_BIAS_R8                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_R9                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_R10                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_R11                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_R2;  /* CAM_ADBS_BIAS_LUT_R2, CAM_A_ADBS_BIAS_LUT_R2*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_R3_
{
        volatile struct /* 0x1A004D68 */
        {
                FIELD  ADBS_BIAS_R12                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_R13                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_R14                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_R3;  /* CAM_ADBS_BIAS_LUT_R3, CAM_A_ADBS_BIAS_LUT_R3*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_G0_
{
        volatile struct /* 0x1A004D6C */
        {
                FIELD  ADBS_BIAS_G0                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_G1                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_G2                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_G3                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_G0;  /* CAM_ADBS_BIAS_LUT_G0, CAM_A_ADBS_BIAS_LUT_G0*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_G1_
{
        volatile struct /* 0x1A004D70 */
        {
                FIELD  ADBS_BIAS_G4                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_G5                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_G6                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_G7                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_G1;  /* CAM_ADBS_BIAS_LUT_G1, CAM_A_ADBS_BIAS_LUT_G1*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_G2_
{
        volatile struct /* 0x1A004D74 */
        {
                FIELD  ADBS_BIAS_G8                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_G9                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_G10                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_G11                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_G2;  /* CAM_ADBS_BIAS_LUT_G2, CAM_A_ADBS_BIAS_LUT_G2*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_G3_
{
        volatile struct /* 0x1A004D78 */
        {
                FIELD  ADBS_BIAS_G12                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_G13                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_G14                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_G3;  /* CAM_ADBS_BIAS_LUT_G3, CAM_A_ADBS_BIAS_LUT_G3*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_B0_
{
        volatile struct /* 0x1A004D7C */
        {
                FIELD  ADBS_BIAS_B0                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_B1                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_B2                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_B3                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_B0;  /* CAM_ADBS_BIAS_LUT_B0, CAM_A_ADBS_BIAS_LUT_B0*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_B1_
{
        volatile struct /* 0x1A004D80 */
        {
                FIELD  ADBS_BIAS_B4                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_B5                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_B6                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_B7                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_B1;  /* CAM_ADBS_BIAS_LUT_B1, CAM_A_ADBS_BIAS_LUT_B1*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_B2_
{
        volatile struct /* 0x1A004D84 */
        {
                FIELD  ADBS_BIAS_B8                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_B9                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_B10                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  ADBS_BIAS_B11                         :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_B2;  /* CAM_ADBS_BIAS_LUT_B2, CAM_A_ADBS_BIAS_LUT_B2*/

typedef volatile union _CAM_REG_ADBS_BIAS_LUT_B3_
{
        volatile struct /* 0x1A004D88 */
        {
                FIELD  ADBS_BIAS_B12                         :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_BIAS_B13                         :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_BIAS_B14                         :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_BIAS_LUT_B3;  /* CAM_ADBS_BIAS_LUT_B3, CAM_A_ADBS_BIAS_LUT_B3*/

typedef volatile union _CAM_REG_ADBS_GAIN_0_
{
        volatile struct /* 0x1A004D8C */
        {
                FIELD  ADBS_GAIN_R                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  ADBS_GAIN_B                           : 13;      /* 16..28, 0x1FFF0000 */
                FIELD  rsv_29                                :  3;      /* 29..31, 0xE0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_GAIN_0;   /* CAM_ADBS_GAIN_0, CAM_A_ADBS_GAIN_0*/

typedef volatile union _CAM_REG_ADBS_GAIN_1_
{
        volatile struct /* 0x1A004D90 */
        {
                FIELD  ADBS_GAIN_G                           : 13;      /*  0..12, 0x00001FFF */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_GAIN_1;   /* CAM_ADBS_GAIN_1, CAM_A_ADBS_GAIN_1*/

typedef volatile union _CAM_REG_ADBS_IVGN_0_
{
        volatile struct /* 0x1A004D94 */
        {
                FIELD  ADBS_IVGN_R                           : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                :  6;      /* 10..15, 0x0000FC00 */
                FIELD  ADBS_IVGN_B                           : 10;      /* 16..25, 0x03FF0000 */
                FIELD  rsv_26                                :  6;      /* 26..31, 0xFC000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_IVGN_0;   /* CAM_ADBS_IVGN_0, CAM_A_ADBS_IVGN_0*/

typedef volatile union _CAM_REG_ADBS_IVGN_1_
{
        volatile struct /* 0x1A004D98 */
        {
                FIELD  ADBS_IVGN_G                           : 10;      /*  0.. 9, 0x000003FF */
                FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_IVGN_1;   /* CAM_ADBS_IVGN_1, CAM_A_ADBS_IVGN_1*/

typedef volatile union _CAM_REG_ADBS_HDR_
{
        volatile struct /* 0x1A004D9C */
        {
                FIELD  ADBS_HDR_GN                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  ADBS_HDR_GN2                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  ADBS_HDR_GNP                          :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  ADBS_HDR_OSCTH                        : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_HDR;  /* CAM_ADBS_HDR, CAM_A_ADBS_HDR*/

typedef volatile union _CAM_REG_ADBS_CMDL_ONLY_1_
{
        volatile struct /* 0x1A004DA0 */
        {
                FIELD  rsv_0                                 : 29;      /*  0..28, 0x1FFFFFFF */
                FIELD  ADBS_EIGER_EN                         :  1;      /* 29..29, 0x20000000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_ADBS_CMDL_ONLY_1;  /* CAM_ADBS_CMDL_ONLY_1, CAM_A_ADBS_CMDL_ONLY_1*/

typedef volatile union _CAM_REG_DCPN_HDR_EN_
{
        volatile struct /* 0x1A004DB0 */
        {
                FIELD  DCPN_HDR_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  DCPN_EDGE_INFO                        :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_HDR_EN;   /* CAM_DCPN_HDR_EN, CAM_A_DCPN_HDR_EN*/

typedef volatile union _CAM_REG_DCPN_IN_IMG_SIZE_
{
        volatile struct /* 0x1A004DB4 */
        {
                FIELD  DCPN_IN_IMG_H                         : 16;      /*  0..15, 0x0000FFFF */
                FIELD  DCPN_IN_IMG_W                         : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_IN_IMG_SIZE;  /* CAM_DCPN_IN_IMG_SIZE, CAM_A_DCPN_IN_IMG_SIZE*/

typedef volatile union _CAM_REG_DCPN_ALGO_PARAM1_
{
        volatile struct /* 0x1A004DB8 */
        {
                FIELD  DCPN_HDR_RATIO                        :  9;      /*  0.. 8, 0x000001FF */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  DCPN_HDR_GAIN                         :  9;      /* 16..24, 0x01FF0000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_ALGO_PARAM1;  /* CAM_DCPN_ALGO_PARAM1, CAM_A_DCPN_ALGO_PARAM1*/

typedef volatile union _CAM_REG_DCPN_ALGO_PARAM2_
{
        volatile struct /* 0x1A004DBC */
        {
                FIELD  DCPN_HDR_TH_K                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  DCPN_HDR_TH_T                         : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_ALGO_PARAM2;  /* CAM_DCPN_ALGO_PARAM2, CAM_A_DCPN_ALGO_PARAM2*/

typedef volatile union _CAM_REG_DCPN_GTM_X0_
{
        volatile struct /* 0x1A004DC0 */
        {
                FIELD  DCPN_GTM_X0                           : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_GTM_X0;   /* CAM_DCPN_GTM_X0, CAM_A_DCPN_GTM_X0*/

typedef volatile union _CAM_REG_DCPN_GTM_Y0_
{
        volatile struct /* 0x1A004DC4 */
        {
                FIELD  DCPN_GTM_Y0                           : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_GTM_Y0;   /* CAM_DCPN_GTM_Y0, CAM_A_DCPN_GTM_Y0*/

typedef volatile union _CAM_REG_DCPN_GTM_S0_
{
        volatile struct /* 0x1A004DC8 */
        {
                FIELD  DCPN_GTM_S0                           : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_GTM_S0;   /* CAM_DCPN_GTM_S0, CAM_A_DCPN_GTM_S0*/

typedef volatile union _CAM_REG_DCPN_GTM_S1_
{
        volatile struct /* 0x1A004DCC */
        {
                FIELD  DCPN_GTM_S1                           : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DCPN_GTM_S1;   /* CAM_DCPN_GTM_S1, CAM_A_DCPN_GTM_S1*/

typedef volatile union _CAM_REG_CPN_HDR_CTL_EN_
{
        volatile struct /* 0x1A004DF0 */
        {
                FIELD  CPN_HDR_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  CPN_AALLE_EN                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  CPN_PSSE_EN                           :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  CPN_EDGE_INFO                         :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  rsv_8                                 : 24;      /*  8..31, 0xFFFFFF00 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_HDR_CTL_EN;    /* CAM_CPN_HDR_CTL_EN, CAM_A_CPN_HDR_CTL_EN*/

typedef volatile union _CAM_REG_CPN_IN_IMG_SIZE_
{
        volatile struct /* 0x1A004DF4 */
        {
                FIELD  CPN_IN_IMG_H                          : 16;      /*  0..15, 0x0000FFFF */
                FIELD  CPN_IN_IMG_W                          : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_IN_IMG_SIZE;   /* CAM_CPN_IN_IMG_SIZE, CAM_A_CPN_IN_IMG_SIZE*/

typedef volatile union _CAM_REG_CPN_ALGO_PARAM1_
{
        volatile struct /* 0x1A004DF8 */
        {
                FIELD  CPN_HDR_RATIO                         :  9;      /*  0.. 8, 0x000001FF */
                FIELD  rsv_9                                 :  7;      /*  9..15, 0x0000FE00 */
                FIELD  CPN_HDR_GAIN                          :  9;      /* 16..24, 0x01FF0000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_ALGO_PARAM1;   /* CAM_CPN_ALGO_PARAM1, CAM_A_CPN_ALGO_PARAM1*/

typedef volatile union _CAM_REG_CPN_ALGO_PARAM2_
{
        volatile struct /* 0x1A004DFC */
        {
                FIELD  CPN_HDR_TH_K                          : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_HDR_TH_T                          : 11;      /* 16..26, 0x07FF0000 */
                FIELD  rsv_27                                :  5;      /* 27..31, 0xF8000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_ALGO_PARAM2;   /* CAM_CPN_ALGO_PARAM2, CAM_A_CPN_ALGO_PARAM2*/

typedef volatile union _CAM_REG_CPN_GTM_X0X1_
{
        volatile struct /* 0x1A004E00 */
        {
                FIELD  CPN_GTM_X0                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_X1                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_X0X1;  /* CAM_CPN_GTM_X0X1, CAM_A_CPN_GTM_X0X1*/

typedef volatile union _CAM_REG_CPN_GTM_X2X3_
{
        volatile struct /* 0x1A004E04 */
        {
                FIELD  CPN_GTM_X2                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_X3                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_X2X3;  /* CAM_CPN_GTM_X2X3, CAM_A_CPN_GTM_X2X3*/

typedef volatile union _CAM_REG_CPN_GTM_X4X5_
{
        volatile struct /* 0x1A004E08 */
        {
                FIELD  CPN_GTM_X4                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_X5                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_X4X5;  /* CAM_CPN_GTM_X4X5, CAM_A_CPN_GTM_X4X5*/

typedef volatile union _CAM_REG_CPN_GTM_X6_
{
        volatile struct /* 0x1A004E0C */
        {
                FIELD  CPN_GTM_X6                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_X6;    /* CAM_CPN_GTM_X6, CAM_A_CPN_GTM_X6*/

typedef volatile union _CAM_REG_CPN_GTM_Y0Y1_
{
        volatile struct /* 0x1A004E10 */
        {
                FIELD  CPN_GTM_Y0                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_Y1                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_Y0Y1;  /* CAM_CPN_GTM_Y0Y1, CAM_A_CPN_GTM_Y0Y1*/

typedef volatile union _CAM_REG_CPN_GTM_Y2Y3_
{
        volatile struct /* 0x1A004E14 */
        {
                FIELD  CPN_GTM_Y2                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_Y3                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_Y2Y3;  /* CAM_CPN_GTM_Y2Y3, CAM_A_CPN_GTM_Y2Y3*/

typedef volatile union _CAM_REG_CPN_GTM_Y4Y5_
{
        volatile struct /* 0x1A004E18 */
        {
                FIELD  CPN_GTM_Y4                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_Y5                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_Y4Y5;  /* CAM_CPN_GTM_Y4Y5, CAM_A_CPN_GTM_Y4Y5*/

typedef volatile union _CAM_REG_CPN_GTM_Y6_
{
        volatile struct /* 0x1A004E1C */
        {
                FIELD  CPN_GTM_Y6                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_Y6;    /* CAM_CPN_GTM_Y6, CAM_A_CPN_GTM_Y6*/

typedef volatile union _CAM_REG_CPN_GTM_S0S1_
{
        volatile struct /* 0x1A004E20 */
        {
                FIELD  CPN_GTM_S0                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_S1                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_S0S1;  /* CAM_CPN_GTM_S0S1, CAM_A_CPN_GTM_S0S1*/

typedef volatile union _CAM_REG_CPN_GTM_S2S3_
{
        volatile struct /* 0x1A004E24 */
        {
                FIELD  CPN_GTM_S2                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_S3                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_S2S3;  /* CAM_CPN_GTM_S2S3, CAM_A_CPN_GTM_S2S3*/

typedef volatile union _CAM_REG_CPN_GTM_S4S5_
{
        volatile struct /* 0x1A004E28 */
        {
                FIELD  CPN_GTM_S4                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_S5                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_S4S5;  /* CAM_CPN_GTM_S4S5, CAM_A_CPN_GTM_S4S5*/

typedef volatile union _CAM_REG_CPN_GTM_S6S7_
{
        volatile struct /* 0x1A004E2C */
        {
                FIELD  CPN_GTM_S6                            : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  CPN_GTM_S7                            : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CPN_GTM_S6S7;  /* CAM_CPN_GTM_S6S7, CAM_A_CPN_GTM_S6S7*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE0_3_
{
        volatile struct /* 0x1A004FA0 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_R_OFST1                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_R_OFST2                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_R_OFST3                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE0_3;   /* CAM_SGM_R_OFST_TABLE0_3, CAM_A_SGM_R_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE4_7_
{
        volatile struct /* 0x1A004FA4 */
        {
                FIELD  SGM_R_OFST4                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_R_OFST5                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_R_OFST6                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_R_OFST7                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE4_7;   /* CAM_SGM_R_OFST_TABLE4_7, CAM_A_SGM_R_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE8_11_
{
        volatile struct /* 0x1A004FA8 */
        {
                FIELD  SGM_R_OFST8                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_R_OFST9                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_R_OFST10                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_R_OFST11                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE8_11;  /* CAM_SGM_R_OFST_TABLE8_11, CAM_A_SGM_R_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE12_15_
{
        volatile struct /* 0x1A004FAC */
        {
                FIELD  SGM_R_OFST12                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_R_OFST13                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_R_OFST14                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_R_OFST15                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE12_15; /* CAM_SGM_R_OFST_TABLE12_15, CAM_A_SGM_R_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE0_3_
{
        volatile struct /* 0x1A004FB0 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_G_OFST1                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_G_OFST2                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_G_OFST3                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE0_3;   /* CAM_SGM_G_OFST_TABLE0_3, CAM_A_SGM_G_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE4_7_
{
        volatile struct /* 0x1A004FB4 */
        {
                FIELD  SGM_G_OFST4                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_G_OFST5                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_G_OFST6                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_G_OFST7                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE4_7;   /* CAM_SGM_G_OFST_TABLE4_7, CAM_A_SGM_G_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE8_11_
{
        volatile struct /* 0x1A004FB8 */
        {
                FIELD  SGM_G_OFST8                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_G_OFST9                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_G_OFST10                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_G_OFST11                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE8_11;  /* CAM_SGM_G_OFST_TABLE8_11, CAM_A_SGM_G_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE12_15_
{
        volatile struct /* 0x1A004FBC */
        {
                FIELD  SGM_G_OFST12                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_G_OFST13                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_G_OFST14                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_G_OFST15                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE12_15; /* CAM_SGM_G_OFST_TABLE12_15, CAM_A_SGM_G_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE0_3_
{
        volatile struct /* 0x1A004FC0 */
        {
                FIELD  rsv_0                                 :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_B_OFST1                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_B_OFST2                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_B_OFST3                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE0_3;   /* CAM_SGM_B_OFST_TABLE0_3, CAM_A_SGM_B_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE4_7_
{
        volatile struct /* 0x1A004FC4 */
        {
                FIELD  SGM_B_OFST4                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_B_OFST5                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_B_OFST6                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_B_OFST7                           :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE4_7;   /* CAM_SGM_B_OFST_TABLE4_7, CAM_A_SGM_B_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE8_11_
{
        volatile struct /* 0x1A004FC8 */
        {
                FIELD  SGM_B_OFST8                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_B_OFST9                           :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_B_OFST10                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_B_OFST11                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE8_11;  /* CAM_SGM_B_OFST_TABLE8_11, CAM_A_SGM_B_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE12_15_
{
        volatile struct /* 0x1A004FCC */
        {
                FIELD  SGM_B_OFST12                          :  8;      /*  0.. 7, 0x000000FF */
                FIELD  SGM_B_OFST13                          :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SGM_B_OFST14                          :  8;      /* 16..23, 0x00FF0000 */
                FIELD  SGM_B_OFST15                          :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE12_15; /* CAM_SGM_B_OFST_TABLE12_15, CAM_A_SGM_B_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_DMA_SOFT_RSTSTAT_
{
        volatile struct /* 0x1A005000 */
        {
                FIELD  IMGO_SOFT_RST_STAT                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_SOFT_RST_STAT                    :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_SOFT_RST_STAT                     :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_SOFT_RST_STAT                     :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_SOFT_RST_STAT                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_SOFT_RST_STAT                    :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_SOFT_RST_STAT                     :  1;      /*  6.. 6, 0x00000040 */
                FIELD  PSO_SOFT_RST_STAT                     :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LMVO_SOFT_RST_STAT                    :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FLKO_SOFT_RST_STAT                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RSSO_A_SOFT_RST_STAT                  :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_SOFT_RST_STAT                    :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                :  3;      /* 12..14, 0x00007000 */
                FIELD  PDI_SOFT_RST_STAT                     :  1;      /* 15..15, 0x00008000 */
                FIELD  BPCI_SOFT_RST_STAT                    :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_SOFT_RST_STAT                    :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_SOFT_RST_STAT                    :  1;      /* 18..18, 0x00040000 */
                FIELD  LSC3I_SOFT_RST_STAT                   :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_SOFT_RSTSTAT;  /* CAM_DMA_SOFT_RSTSTAT, CAM_A_DMA_SOFT_RSTSTAT*/

typedef volatile union _CAM_REG_CQ0I_BASE_ADDR_
{
        volatile struct /* 0x1A005004 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ0I_BASE_ADDR;    /* CAM_CQ0I_BASE_ADDR, CAM_A_CQ0I_BASE_ADDR*/

typedef volatile union _CAM_REG_CQ0I_XSIZE_
{
        volatile struct /* 0x1A005008 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CQ0I_XSIZE;    /* CAM_CQ0I_XSIZE, CAM_A_CQ0I_XSIZE*/

typedef volatile union _CAM_REG_VERTICAL_FLIP_EN_
{
        volatile struct /* 0x1A00500C */
        {
                FIELD  IMGO_V_FLIP_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_V_FLIP_EN                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_V_FLIP_EN                         :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_V_FLIP_EN                         :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_V_FLIP_EN                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_V_FLIP_EN                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_V_FLIP_EN                         :  1;      /*  6.. 6, 0x00000040 */
                FIELD  PSO_V_FLIP_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LMVO_V_FLIP_EN                        :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FLKO_V_FLIP_EN                        :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RSSO_A_V_FLIP_EN                      :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_V_FLIP_EN                        :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                :  3;      /* 12..14, 0x00007000 */
                FIELD  PDI_V_FLIP_EN                         :  1;      /* 15..15, 0x00008000 */
                FIELD  BPCI_V_FLIP_EN                        :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_V_FLIP_EN                        :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_V_FLIP_EN                        :  1;      /* 18..18, 0x00040000 */
                FIELD  LSC3I_V_FLIP_EN                       :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_VERTICAL_FLIP_EN;  /* CAM_VERTICAL_FLIP_EN, CAM_A_VERTICAL_FLIP_EN*/

typedef volatile union _CAM_REG_DMA_SOFT_RESET_
{
        volatile struct /* 0x1A005010 */
        {
                FIELD  IMGO_SOFT_RST                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_SOFT_RST                         :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_SOFT_RST                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_SOFT_RST                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_SOFT_RST                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_SOFT_RST                         :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_SOFT_RST                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  PSO_SOFT_RST                          :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LMVO_SOFT_RST                         :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FLKO_SOFT_RST                         :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RSSO_A_SOFT_RST                       :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_SOFT_RST                         :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                :  3;      /* 12..14, 0x00007000 */
                FIELD  PDI_SOFT_RST                          :  1;      /* 15..15, 0x00008000 */
                FIELD  BPCI_SOFT_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_SOFT_RST                         :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_SOFT_RST                         :  1;      /* 18..18, 0x00040000 */
                FIELD  LSC3I_SOFT_RST                        :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 11;      /* 20..30, 0x7FF00000 */
                FIELD  SEPARATE_SOFT_RST_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_SOFT_RESET;    /* CAM_DMA_SOFT_RESET, CAM_A_DMA_SOFT_RESET*/

typedef volatile union _CAM_REG_LAST_ULTRA_EN_
{
        volatile struct /* 0x1A005014 */
        {
                FIELD  IMGO_LAST_ULTRA_EN                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_LAST_ULTRA_EN                    :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_LAST_ULTRA_EN                     :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_LAST_ULTRA_EN                     :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_LAST_ULTRA_EN                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_LAST_ULTRA_EN                    :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_LAST_ULTRA_EN                     :  1;      /*  6.. 6, 0x00000040 */
                FIELD  PSO_LAST_ULTRA_EN                     :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LMVO_LAST_ULTRA_EN                    :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FLKO_LAST_ULTRA_EN                    :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RSSO_A_LAST_ULTRA_EN                  :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_LAST_ULTRA_EN                    :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                :  3;      /* 12..14, 0x00007000 */
                FIELD  PDI_LAST_ULTRA_EN                     :  1;      /* 15..15, 0x00008000 */
                FIELD  BPCI_LAST_ULTRA_EN                    :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_LAST_ULTRA_EN                    :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_LAST_ULTRA_EN                    :  1;      /* 18..18, 0x00040000 */
                FIELD  LSC3I_LAST_ULTRA_EN                   :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LAST_ULTRA_EN; /* CAM_LAST_ULTRA_EN, CAM_A_LAST_ULTRA_EN*/

typedef volatile union _CAM_REG_SPECIAL_FUN_EN_
{
        volatile struct /* 0x1A005018 */
        {
                FIELD  rsv_0                                 : 20;      /*  0..19, 0x000FFFFF */
                FIELD  CONTINUOUS_COM_CON                    :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  MULTI_PLANE_ID_EN                     :  1;      /* 24..24, 0x01000000 */
                FIELD  CONTINUOUS_COM_EN                     :  1;      /* 25..25, 0x02000000 */
                FIELD  FIFO_CHANGE_EN                        :  1;      /* 26..26, 0x04000000 */
                FIELD  GCLAST_EN                             :  1;      /* 27..27, 0x08000000 */
                FIELD  CQ_ULTRA_BPCI_EN                      :  1;      /* 28..28, 0x10000000 */
                FIELD  CQ_ULTRA_LSCI_EN                      :  1;      /* 29..29, 0x20000000 */
                FIELD  UFOG_RRZO_EN                          :  1;      /* 30..30, 0x40000000 */
                FIELD  UFO_IMGO_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_SPECIAL_FUN_EN;    /* CAM_SPECIAL_FUN_EN, CAM_A_SPECIAL_FUN_EN*/

typedef volatile union _CAM_REG_IMGO_BASE_ADDR_
{
        volatile struct /* 0x1A005020 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_BASE_ADDR;    /* CAM_IMGO_BASE_ADDR, CAM_A_IMGO_BASE_ADDR*/

typedef volatile union _CAM_REG_IMGO_OFST_ADDR_
{
        volatile struct /* 0x1A005028 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_OFST_ADDR;    /* CAM_IMGO_OFST_ADDR, CAM_A_IMGO_OFST_ADDR*/

typedef volatile union _CAM_REG_IMGO_DRS_
{
        volatile struct /* 0x1A00502C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_DRS;  /* CAM_IMGO_DRS, CAM_A_IMGO_DRS*/

typedef volatile union _CAM_REG_IMGO_XSIZE_
{
        volatile struct /* 0x1A005030 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_XSIZE;    /* CAM_IMGO_XSIZE, CAM_A_IMGO_XSIZE*/

typedef volatile union _CAM_REG_IMGO_YSIZE_
{
        volatile struct /* 0x1A005034 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_YSIZE;    /* CAM_IMGO_YSIZE, CAM_A_IMGO_YSIZE*/

typedef volatile union _CAM_REG_IMGO_STRIDE_
{
        volatile struct /* 0x1A005038 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  FORMAT                                :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_STRIDE;   /* CAM_IMGO_STRIDE, CAM_A_IMGO_STRIDE*/

typedef volatile union _CAM_REG_IMGO_CON_
{
        volatile struct /* 0x1A00503C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_CON;  /* CAM_IMGO_CON, CAM_A_IMGO_CON*/

typedef volatile union _CAM_REG_IMGO_CON2_
{
        volatile struct /* 0x1A005040 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_CON2; /* CAM_IMGO_CON2, CAM_A_IMGO_CON2*/

typedef volatile union _CAM_REG_IMGO_CON3_
{
        volatile struct /* 0x1A005044 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_CON3; /* CAM_IMGO_CON3, CAM_A_IMGO_CON3*/

typedef volatile union _CAM_REG_IMGO_CROP_
{
        volatile struct /* 0x1A005048 */
        {
                FIELD  XOFFSET                               : 16;      /*  0..15, 0x0000FFFF */
                FIELD  YOFFSET                               : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_CROP; /* CAM_IMGO_CROP, CAM_A_IMGO_CROP*/

typedef volatile union _CAM_REG_IMGO_CON4_
{
        volatile struct /* 0x1A00504C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_CON4; /* CAM_IMGO_CON4, CAM_A_IMGO_CON4*/

typedef volatile union _CAM_REG_RRZO_BASE_ADDR_
{
        volatile struct /* 0x1A005050 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_BASE_ADDR;    /* CAM_RRZO_BASE_ADDR, CAM_A_RRZO_BASE_ADDR*/

typedef volatile union _CAM_REG_RRZO_OFST_ADDR_
{
        volatile struct /* 0x1A005058 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_OFST_ADDR;    /* CAM_RRZO_OFST_ADDR, CAM_A_RRZO_OFST_ADDR*/

typedef volatile union _CAM_REG_RRZO_DRS_
{
        volatile struct /* 0x1A00505C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_DRS;  /* CAM_RRZO_DRS, CAM_A_RRZO_DRS*/

typedef volatile union _CAM_REG_RRZO_XSIZE_
{
        volatile struct /* 0x1A005060 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_XSIZE;    /* CAM_RRZO_XSIZE, CAM_A_RRZO_XSIZE*/

typedef volatile union _CAM_REG_RRZO_YSIZE_
{
        volatile struct /* 0x1A005064 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_YSIZE;    /* CAM_RRZO_YSIZE, CAM_A_RRZO_YSIZE*/

typedef volatile union _CAM_REG_RRZO_STRIDE_
{
        volatile struct /* 0x1A005068 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  FORMAT                                :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_STRIDE;   /* CAM_RRZO_STRIDE, CAM_A_RRZO_STRIDE*/

typedef volatile union _CAM_REG_RRZO_CON_
{
        volatile struct /* 0x1A00506C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_CON;  /* CAM_RRZO_CON, CAM_A_RRZO_CON*/

typedef volatile union _CAM_REG_RRZO_CON2_
{
        volatile struct /* 0x1A005070 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_CON2; /* CAM_RRZO_CON2, CAM_A_RRZO_CON2*/

typedef volatile union _CAM_REG_RRZO_CON3_
{
        volatile struct /* 0x1A005074 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_CON3; /* CAM_RRZO_CON3, CAM_A_RRZO_CON3*/

typedef volatile union _CAM_REG_RRZO_CROP_
{
        volatile struct /* 0x1A005078 */
        {
                FIELD  XOFFSET                               : 16;      /*  0..15, 0x0000FFFF */
                FIELD  YOFFSET                               : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_CROP; /* CAM_RRZO_CROP, CAM_A_RRZO_CROP*/

typedef volatile union _CAM_REG_RRZO_CON4_
{
        volatile struct /* 0x1A00507C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_CON4; /* CAM_RRZO_CON4, CAM_A_RRZO_CON4*/

typedef volatile union _CAM_REG_AAO_BASE_ADDR_
{
        volatile struct /* 0x1A005080 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_BASE_ADDR; /* CAM_AAO_BASE_ADDR, CAM_A_AAO_BASE_ADDR*/

typedef volatile union _CAM_REG_AAO_OFST_ADDR_
{
        volatile struct /* 0x1A005088 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_OFST_ADDR; /* CAM_AAO_OFST_ADDR, CAM_A_AAO_OFST_ADDR*/

typedef volatile union _CAM_REG_AAO_DRS_
{
        volatile struct /* 0x1A00508C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_DRS;   /* CAM_AAO_DRS, CAM_A_AAO_DRS*/

typedef volatile union _CAM_REG_AAO_XSIZE_
{
        volatile struct /* 0x1A005090 */
        {
                FIELD  XSIZE                                 : 19;      /*  0..18, 0x0007FFFF */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_XSIZE; /* CAM_AAO_XSIZE, CAM_A_AAO_XSIZE*/

typedef volatile union _CAM_REG_AAO_YSIZE_
{
        volatile struct /* 0x1A005094 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_YSIZE; /* CAM_AAO_YSIZE, CAM_A_AAO_YSIZE*/

typedef volatile union _CAM_REG_AAO_STRIDE_
{
        volatile struct /* 0x1A005098 */
        {
                FIELD  RSV                                   : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  2;      /* 16..17, 0x00030000 */
                FIELD  rsv_18                                :  6;      /* 18..23, 0x00FC0000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_STRIDE;    /* CAM_AAO_STRIDE, CAM_A_AAO_STRIDE*/

typedef volatile union _CAM_REG_AAO_CON_
{
        volatile struct /* 0x1A00509C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_CON;   /* CAM_AAO_CON, CAM_A_AAO_CON*/

typedef volatile union _CAM_REG_AAO_CON2_
{
        volatile struct /* 0x1A0050A0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_CON2;  /* CAM_AAO_CON2, CAM_A_AAO_CON2*/

typedef volatile union _CAM_REG_AAO_CON3_
{
        volatile struct /* 0x1A0050A4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_CON3;  /* CAM_AAO_CON3, CAM_A_AAO_CON3*/

typedef volatile union _CAM_REG_AAO_CON4_
{
        volatile struct /* 0x1A0050AC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_CON4;  /* CAM_AAO_CON4, CAM_A_AAO_CON4*/

typedef volatile union _CAM_REG_AFO_BASE_ADDR_
{
        volatile struct /* 0x1A0050B0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_BASE_ADDR; /* CAM_AFO_BASE_ADDR, CAM_A_AFO_BASE_ADDR*/

typedef volatile union _CAM_REG_AFO_OFST_ADDR_
{
        volatile struct /* 0x1A0050B8 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_OFST_ADDR; /* CAM_AFO_OFST_ADDR, CAM_A_AFO_OFST_ADDR*/

typedef volatile union _CAM_REG_AFO_DRS_
{
        volatile struct /* 0x1A0050BC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_DRS;   /* CAM_AFO_DRS, CAM_A_AFO_DRS*/

typedef volatile union _CAM_REG_AFO_XSIZE_
{
        volatile struct /* 0x1A0050C0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_XSIZE; /* CAM_AFO_XSIZE, CAM_A_AFO_XSIZE*/

typedef volatile union _CAM_REG_AFO_YSIZE_
{
        volatile struct /* 0x1A0050C4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_YSIZE; /* CAM_AFO_YSIZE, CAM_A_AFO_YSIZE*/

typedef volatile union _CAM_REG_AFO_STRIDE_
{
        volatile struct /* 0x1A0050C8 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_STRIDE;    /* CAM_AFO_STRIDE, CAM_A_AFO_STRIDE*/

typedef volatile union _CAM_REG_AFO_CON_
{
        volatile struct /* 0x1A0050CC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_CON;   /* CAM_AFO_CON, CAM_A_AFO_CON*/

typedef volatile union _CAM_REG_AFO_CON2_
{
        volatile struct /* 0x1A0050D0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_CON2;  /* CAM_AFO_CON2, CAM_A_AFO_CON2*/

typedef volatile union _CAM_REG_AFO_CON3_
{
        volatile struct /* 0x1A0050D4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_CON3;  /* CAM_AFO_CON3, CAM_A_AFO_CON3*/

typedef volatile union _CAM_REG_AFO_CON4_
{
        volatile struct /* 0x1A0050DC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_CON4;  /* CAM_AFO_CON4, CAM_A_AFO_CON4*/

typedef volatile union _CAM_REG_LCSO_BASE_ADDR_
{
        volatile struct /* 0x1A0050E0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_BASE_ADDR;    /* CAM_LCSO_BASE_ADDR, CAM_A_LCSO_BASE_ADDR*/

typedef volatile union _CAM_REG_LCSO_OFST_ADDR_
{
        volatile struct /* 0x1A0050E8 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_OFST_ADDR;    /* CAM_LCSO_OFST_ADDR, CAM_A_LCSO_OFST_ADDR*/

typedef volatile union _CAM_REG_LCSO_DRS_
{
        volatile struct /* 0x1A0050EC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_DRS;  /* CAM_LCSO_DRS, CAM_A_LCSO_DRS*/

typedef volatile union _CAM_REG_LCSO_XSIZE_
{
        volatile struct /* 0x1A0050F0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_XSIZE;    /* CAM_LCSO_XSIZE, CAM_A_LCSO_XSIZE*/

typedef volatile union _CAM_REG_LCSO_YSIZE_
{
        volatile struct /* 0x1A0050F4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_YSIZE;    /* CAM_LCSO_YSIZE, CAM_A_LCSO_YSIZE*/

typedef volatile union _CAM_REG_LCSO_STRIDE_
{
        volatile struct /* 0x1A0050F8 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_STRIDE;   /* CAM_LCSO_STRIDE, CAM_A_LCSO_STRIDE*/

typedef volatile union _CAM_REG_LCSO_CON_
{
        volatile struct /* 0x1A0050FC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_CON;  /* CAM_LCSO_CON, CAM_A_LCSO_CON*/

typedef volatile union _CAM_REG_LCSO_CON2_
{
        volatile struct /* 0x1A005100 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_CON2; /* CAM_LCSO_CON2, CAM_A_LCSO_CON2*/

typedef volatile union _CAM_REG_LCSO_CON3_
{
        volatile struct /* 0x1A005104 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_CON3; /* CAM_LCSO_CON3, CAM_A_LCSO_CON3*/

typedef volatile union _CAM_REG_LCSO_CON4_
{
        volatile struct /* 0x1A00510C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_CON4; /* CAM_LCSO_CON4, CAM_A_LCSO_CON4*/

typedef volatile union _CAM_REG_UFEO_BASE_ADDR_
{
        volatile struct /* 0x1A005110 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_BASE_ADDR;    /* CAM_UFEO_BASE_ADDR, CAM_A_UFEO_BASE_ADDR*/

typedef volatile union _CAM_REG_UFEO_OFST_ADDR_
{
        volatile struct /* 0x1A005118 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_OFST_ADDR;    /* CAM_UFEO_OFST_ADDR, CAM_A_UFEO_OFST_ADDR*/

typedef volatile union _CAM_REG_UFEO_DRS_
{
        volatile struct /* 0x1A00511C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_DRS;  /* CAM_UFEO_DRS, CAM_A_UFEO_DRS*/

typedef volatile union _CAM_REG_UFEO_XSIZE_
{
        volatile struct /* 0x1A005120 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_XSIZE;    /* CAM_UFEO_XSIZE, CAM_A_UFEO_XSIZE*/

typedef volatile union _CAM_REG_UFEO_YSIZE_
{
        volatile struct /* 0x1A005124 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_YSIZE;    /* CAM_UFEO_YSIZE, CAM_A_UFEO_YSIZE*/

typedef volatile union _CAM_REG_UFEO_STRIDE_
{
        volatile struct /* 0x1A005128 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_STRIDE;   /* CAM_UFEO_STRIDE, CAM_A_UFEO_STRIDE*/

typedef volatile union _CAM_REG_UFEO_CON_
{
        volatile struct /* 0x1A00512C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_CON;  /* CAM_UFEO_CON, CAM_A_UFEO_CON*/

typedef volatile union _CAM_REG_UFEO_CON2_
{
        volatile struct /* 0x1A005130 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_CON2; /* CAM_UFEO_CON2, CAM_A_UFEO_CON2*/

typedef volatile union _CAM_REG_UFEO_CON3_
{
        volatile struct /* 0x1A005134 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_CON3; /* CAM_UFEO_CON3, CAM_A_UFEO_CON3*/

typedef volatile union _CAM_REG_UFEO_CON4_
{
        volatile struct /* 0x1A00513C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_CON4; /* CAM_UFEO_CON4, CAM_A_UFEO_CON4*/

typedef volatile union _CAM_REG_PDO_BASE_ADDR_
{
        volatile struct /* 0x1A005140 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_BASE_ADDR; /* CAM_PDO_BASE_ADDR, CAM_A_PDO_BASE_ADDR*/

typedef volatile union _CAM_REG_PDO_OFST_ADDR_
{
        volatile struct /* 0x1A005148 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_OFST_ADDR; /* CAM_PDO_OFST_ADDR, CAM_A_PDO_OFST_ADDR*/

typedef volatile union _CAM_REG_PDO_DRS_
{
        volatile struct /* 0x1A00514C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_DRS;   /* CAM_PDO_DRS, CAM_A_PDO_DRS*/

typedef volatile union _CAM_REG_PDO_XSIZE_
{
        volatile struct /* 0x1A005150 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_XSIZE; /* CAM_PDO_XSIZE, CAM_A_PDO_XSIZE*/

typedef volatile union _CAM_REG_PDO_YSIZE_
{
        volatile struct /* 0x1A005154 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_YSIZE; /* CAM_PDO_YSIZE, CAM_A_PDO_YSIZE*/

typedef volatile union _CAM_REG_PDO_STRIDE_
{
        volatile struct /* 0x1A005158 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_STRIDE;    /* CAM_PDO_STRIDE, CAM_A_PDO_STRIDE*/

typedef volatile union _CAM_REG_PDO_CON_
{
        volatile struct /* 0x1A00515C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_CON;   /* CAM_PDO_CON, CAM_A_PDO_CON*/

typedef volatile union _CAM_REG_PDO_CON2_
{
        volatile struct /* 0x1A005160 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_CON2;  /* CAM_PDO_CON2, CAM_A_PDO_CON2*/

typedef volatile union _CAM_REG_PDO_CON3_
{
        volatile struct /* 0x1A005164 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_CON3;  /* CAM_PDO_CON3, CAM_A_PDO_CON3*/

typedef volatile union _CAM_REG_PDO_CON4_
{
        volatile struct /* 0x1A00516C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_CON4;  /* CAM_PDO_CON4, CAM_A_PDO_CON4*/

typedef volatile union _CAM_REG_BPCI_BASE_ADDR_
{
        volatile struct /* 0x1A005170 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_BASE_ADDR;    /* CAM_BPCI_BASE_ADDR, CAM_A_BPCI_BASE_ADDR*/

typedef volatile union _CAM_REG_BPCI_OFST_ADDR_
{
        volatile struct /* 0x1A005178 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_OFST_ADDR;    /* CAM_BPCI_OFST_ADDR, CAM_A_BPCI_OFST_ADDR*/

typedef volatile union _CAM_REG_BPCI_DRS_
{
        volatile struct /* 0x1A00517C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_DRS;  /* CAM_BPCI_DRS, CAM_A_BPCI_DRS*/

typedef volatile union _CAM_REG_BPCI_XSIZE_
{
        volatile struct /* 0x1A005180 */
        {
                FIELD  XSIZE                                 : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_XSIZE;    /* CAM_BPCI_XSIZE, CAM_A_BPCI_XSIZE*/

typedef volatile union _CAM_REG_BPCI_YSIZE_
{
        volatile struct /* 0x1A005184 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_YSIZE;    /* CAM_BPCI_YSIZE, CAM_A_BPCI_YSIZE*/

typedef volatile union _CAM_REG_BPCI_STRIDE_
{
        volatile struct /* 0x1A005188 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  2;      /* 16..17, 0x00030000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  FORMAT                                :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  2;      /* 21..22, 0x00600000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_STRIDE;   /* CAM_BPCI_STRIDE, CAM_A_BPCI_STRIDE*/

typedef volatile union _CAM_REG_BPCI_CON_
{
        volatile struct /* 0x1A00518C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_CON;  /* CAM_BPCI_CON, CAM_A_BPCI_CON*/

typedef volatile union _CAM_REG_BPCI_CON2_
{
        volatile struct /* 0x1A005190 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_CON2; /* CAM_BPCI_CON2, CAM_A_BPCI_CON2*/

typedef volatile union _CAM_REG_BPCI_CON3_
{
        volatile struct /* 0x1A005194 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_CON3; /* CAM_BPCI_CON3, CAM_A_BPCI_CON3*/

typedef volatile union _CAM_REG_BPCI_CON4_
{
        volatile struct /* 0x1A00519C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_CON4; /* CAM_BPCI_CON4, CAM_A_BPCI_CON4*/

typedef volatile union _CAM_REG_CACI_BASE_ADDR_
{
        volatile struct /* 0x1A0051A0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_BASE_ADDR;    /* CAM_CACI_BASE_ADDR, CAM_A_CACI_BASE_ADDR*/

typedef volatile union _CAM_REG_CACI_OFST_ADDR_
{
        volatile struct /* 0x1A0051A8 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_OFST_ADDR;    /* CAM_CACI_OFST_ADDR, CAM_A_CACI_OFST_ADDR*/

typedef volatile union _CAM_REG_CACI_DRS_
{
        volatile struct /* 0x1A0051AC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_DRS;  /* CAM_CACI_DRS, CAM_A_CACI_DRS*/

typedef volatile union _CAM_REG_CACI_XSIZE_
{
        volatile struct /* 0x1A0051B0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_XSIZE;    /* CAM_CACI_XSIZE, CAM_A_CACI_XSIZE*/

typedef volatile union _CAM_REG_CACI_YSIZE_
{
        volatile struct /* 0x1A0051B4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_YSIZE;    /* CAM_CACI_YSIZE, CAM_A_CACI_YSIZE*/

typedef volatile union _CAM_REG_CACI_STRIDE_
{
        volatile struct /* 0x1A0051B8 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  2;      /* 16..17, 0x00030000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  FORMAT                                :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  2;      /* 21..22, 0x00600000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_STRIDE;   /* CAM_CACI_STRIDE, CAM_A_CACI_STRIDE*/

typedef volatile union _CAM_REG_CACI_CON_
{
        volatile struct /* 0x1A0051BC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_CON;  /* CAM_CACI_CON, CAM_A_CACI_CON*/

typedef volatile union _CAM_REG_CACI_CON2_
{
        volatile struct /* 0x1A0051C0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_CON2; /* CAM_CACI_CON2, CAM_A_CACI_CON2*/

typedef volatile union _CAM_REG_CACI_CON3_
{
        volatile struct /* 0x1A0051C4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_CON3; /* CAM_CACI_CON3, CAM_A_CACI_CON3*/

typedef volatile union _CAM_REG_CACI_CON4_
{
        volatile struct /* 0x1A0051CC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_CON4; /* CAM_CACI_CON4, CAM_A_CACI_CON4*/

typedef volatile union _CAM_REG_LSCI_BASE_ADDR_
{
        volatile struct /* 0x1A0051D0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_BASE_ADDR;    /* CAM_LSCI_BASE_ADDR, CAM_A_LSCI_BASE_ADDR*/

typedef volatile union _CAM_REG_LSCI_OFST_ADDR_
{
        volatile struct /* 0x1A0051D8 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_OFST_ADDR;    /* CAM_LSCI_OFST_ADDR, CAM_A_LSCI_OFST_ADDR*/

typedef volatile union _CAM_REG_LSCI_DRS_
{
        volatile struct /* 0x1A0051DC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_DRS;  /* CAM_LSCI_DRS, CAM_A_LSCI_DRS*/

typedef volatile union _CAM_REG_LSCI_XSIZE_
{
        volatile struct /* 0x1A0051E0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_XSIZE;    /* CAM_LSCI_XSIZE, CAM_A_LSCI_XSIZE*/

typedef volatile union _CAM_REG_LSCI_YSIZE_
{
        volatile struct /* 0x1A0051E4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_YSIZE;    /* CAM_LSCI_YSIZE, CAM_A_LSCI_YSIZE*/

typedef volatile union _CAM_REG_LSCI_STRIDE_
{
        volatile struct /* 0x1A0051E8 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  FORMAT                                :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  2;      /* 21..22, 0x00600000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_STRIDE;   /* CAM_LSCI_STRIDE, CAM_A_LSCI_STRIDE*/

typedef volatile union _CAM_REG_LSCI_CON_
{
        volatile struct /* 0x1A0051EC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_CON;  /* CAM_LSCI_CON, CAM_A_LSCI_CON*/

typedef volatile union _CAM_REG_LSCI_CON2_
{
        volatile struct /* 0x1A0051F0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_CON2; /* CAM_LSCI_CON2, CAM_A_LSCI_CON2*/

typedef volatile union _CAM_REG_LSCI_CON3_
{
        volatile struct /* 0x1A0051F4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_CON3; /* CAM_LSCI_CON3, CAM_A_LSCI_CON3*/

typedef volatile union _CAM_REG_LSCI_CON4_
{
        volatile struct /* 0x1A0051FC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_CON4; /* CAM_LSCI_CON4, CAM_A_LSCI_CON4*/

typedef volatile union _CAM_REG_LSC3I_BASE_ADDR_
{
        volatile struct /* 0x1A005200 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_BASE_ADDR;   /* CAM_LSC3I_BASE_ADDR, CAM_A_LSC3I_BASE_ADDR*/

typedef volatile union _CAM_REG_LSC3I_OFST_ADDR_
{
        volatile struct /* 0x1A005208 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_OFST_ADDR;   /* CAM_LSC3I_OFST_ADDR, CAM_A_LSC3I_OFST_ADDR*/

typedef volatile union _CAM_REG_LSC3I_DRS_
{
        volatile struct /* 0x1A00520C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_DRS; /* CAM_LSC3I_DRS, CAM_A_LSC3I_DRS*/

typedef volatile union _CAM_REG_LSC3I_XSIZE_
{
        volatile struct /* 0x1A005210 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_XSIZE;   /* CAM_LSC3I_XSIZE, CAM_A_LSC3I_XSIZE*/

typedef volatile union _CAM_REG_LSC3I_YSIZE_
{
        volatile struct /* 0x1A005214 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_YSIZE;   /* CAM_LSC3I_YSIZE, CAM_A_LSC3I_YSIZE*/

typedef volatile union _CAM_REG_LSC3I_STRIDE_
{
        volatile struct /* 0x1A005218 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  4;      /* 16..19, 0x000F0000 */
                FIELD  FORMAT                                :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  2;      /* 21..22, 0x00600000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_STRIDE;  /* CAM_LSC3I_STRIDE, CAM_A_LSC3I_STRIDE*/

typedef volatile union _CAM_REG_LSC3I_CON_
{
        volatile struct /* 0x1A00521C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_CON; /* CAM_LSC3I_CON, CAM_A_LSC3I_CON*/

typedef volatile union _CAM_REG_LSC3I_CON2_
{
        volatile struct /* 0x1A005220 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_CON2;    /* CAM_LSC3I_CON2, CAM_A_LSC3I_CON2*/

typedef volatile union _CAM_REG_LSC3I_CON3_
{
        volatile struct /* 0x1A005224 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_CON3;    /* CAM_LSC3I_CON3, CAM_A_LSC3I_CON3*/

typedef volatile union _CAM_REG_LSC3I_CON4_
{
        volatile struct /* 0x1A00522C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_CON4;    /* CAM_LSC3I_CON4, CAM_A_LSC3I_CON4*/

typedef volatile union _CAM_REG_PDI_BASE_ADDR_
{
        volatile struct /* 0x1A005230 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_BASE_ADDR; /* CAM_PDI_BASE_ADDR, CAM_A_PDI_BASE_ADDR*/

typedef volatile union _CAM_REG_PDI_OFST_ADDR_
{
        volatile struct /* 0x1A005234 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_OFST_ADDR; /* CAM_PDI_OFST_ADDR, CAM_A_PDI_OFST_ADDR*/

typedef volatile union _CAM_REG_PDI_DRS_
{
        volatile struct /* 0x1A00523C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_DRS;   /* CAM_PDI_DRS, CAM_A_PDI_DRS*/

typedef volatile union _CAM_REG_PDI_XSIZE_
{
        volatile struct /* 0x1A005240 */
        {
                FIELD  XSIZE                                 : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_XSIZE; /* CAM_PDI_XSIZE, CAM_A_PDI_XSIZE*/

typedef volatile union _CAM_REG_PDI_YSIZE_
{
        volatile struct /* 0x1A005244 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_YSIZE; /* CAM_PDI_YSIZE, CAM_A_PDI_YSIZE*/

typedef volatile union _CAM_REG_PDI_STRIDE_
{
        volatile struct /* 0x1A005248 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  2;      /* 16..17, 0x00030000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  FORMAT                                :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  2;      /* 21..22, 0x00600000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  5;      /* 25..29, 0x3E000000 */
                FIELD  SWAP                                  :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_STRIDE;    /* CAM_PDI_STRIDE, CAM_A_PDI_STRIDE*/

typedef volatile union _CAM_REG_PDI_CON_
{
        volatile struct /* 0x1A00524C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_CON;   /* CAM_PDI_CON, CAM_A_PDI_CON*/

typedef volatile union _CAM_REG_PDI_CON2_
{
        volatile struct /* 0x1A005250 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_CON2;  /* CAM_PDI_CON2, CAM_A_PDI_CON2*/

typedef volatile union _CAM_REG_PDI_CON3_
{
        volatile struct /* 0x1A005254 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_CON3;  /* CAM_PDI_CON3, CAM_A_PDI_CON3*/

typedef volatile union _CAM_REG_PDI_CON4_
{
        volatile struct /* 0x1A00525C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_CON4;  /* CAM_PDI_CON4, CAM_A_PDI_CON4*/

typedef volatile union _CAM_REG_PSO_BASE_ADDR_
{
        volatile struct /* 0x1A005260 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_BASE_ADDR; /* CAM_PSO_BASE_ADDR, CAM_A_PSO_BASE_ADDR*/

typedef volatile union _CAM_REG_PSO_OFST_ADDR_
{
        volatile struct /* 0x1A005264 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_OFST_ADDR; /* CAM_PSO_OFST_ADDR, CAM_A_PSO_OFST_ADDR*/

typedef volatile union _CAM_REG_PSO_DRS_
{
        volatile struct /* 0x1A00526C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_DRS;   /* CAM_PSO_DRS, CAM_A_PSO_DRS*/

typedef volatile union _CAM_REG_PSO_XSIZE_
{
        volatile struct /* 0x1A005270 */
        {
                FIELD  XSIZE                                 : 19;      /*  0..18, 0x0007FFFF */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_XSIZE; /* CAM_PSO_XSIZE, CAM_A_PSO_XSIZE*/

typedef volatile union _CAM_REG_PSO_YSIZE_
{
        volatile struct /* 0x1A005274 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_YSIZE; /* CAM_PSO_YSIZE, CAM_A_PSO_YSIZE*/

typedef volatile union _CAM_REG_PSO_STRIDE_
{
        volatile struct /* 0x1A005278 */
        {
                FIELD  RSV                                   : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  2;      /* 16..17, 0x00030000 */
                FIELD  rsv_18                                :  6;      /* 18..23, 0x00FC0000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_STRIDE;    /* CAM_PSO_STRIDE, CAM_A_PSO_STRIDE*/

typedef volatile union _CAM_REG_PSO_CON_
{
        volatile struct /* 0x1A00527C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_CON;   /* CAM_PSO_CON, CAM_A_PSO_CON*/

typedef volatile union _CAM_REG_PSO_CON2_
{
        volatile struct /* 0x1A005280 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_CON2;  /* CAM_PSO_CON2, CAM_A_PSO_CON2*/

typedef volatile union _CAM_REG_PSO_CON3_
{
        volatile struct /* 0x1A005284 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_CON3;  /* CAM_PSO_CON3, CAM_A_PSO_CON3*/

typedef volatile union _CAM_REG_PSO_CON4_
{
        volatile struct /* 0x1A00528C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_CON4;  /* CAM_PSO_CON4, CAM_A_PSO_CON4*/

typedef volatile union _CAM_REG_LMVO_BASE_ADDR_
{
        volatile struct /* 0x1A005290 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_BASE_ADDR;    /* CAM_LMVO_BASE_ADDR, CAM_A_LMVO_BASE_ADDR*/

typedef volatile union _CAM_REG_LMVO_OFST_ADDR_
{
        volatile struct /* 0x1A005294 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_OFST_ADDR;    /* CAM_LMVO_OFST_ADDR, CAM_A_LMVO_OFST_ADDR*/

typedef volatile union _CAM_REG_LMVO_DRS_
{
        volatile struct /* 0x1A00529C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_DRS;  /* CAM_LMVO_DRS, CAM_A_LMVO_DRS*/

typedef volatile union _CAM_REG_LMVO_XSIZE_
{
        volatile struct /* 0x1A0052A0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_XSIZE;    /* CAM_LMVO_XSIZE, CAM_A_LMVO_XSIZE*/

typedef volatile union _CAM_REG_LMVO_YSIZE_
{
        volatile struct /* 0x1A0052A4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_YSIZE;    /* CAM_LMVO_YSIZE, CAM_A_LMVO_YSIZE*/

typedef volatile union _CAM_REG_LMVO_STRIDE_
{
        volatile struct /* 0x1A0052A8 */
        {
                FIELD  RSV                                   : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                :  5;      /* 19..23, 0x00F80000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_STRIDE;   /* CAM_LMVO_STRIDE, CAM_A_LMVO_STRIDE*/

typedef volatile union _CAM_REG_LMVO_CON_
{
        volatile struct /* 0x1A0052AC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_CON;  /* CAM_LMVO_CON, CAM_A_LMVO_CON*/

typedef volatile union _CAM_REG_LMVO_CON2_
{
        volatile struct /* 0x1A0052B0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_CON2; /* CAM_LMVO_CON2, CAM_A_LMVO_CON2*/

typedef volatile union _CAM_REG_LMVO_CON3_
{
        volatile struct /* 0x1A0052B4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_CON3; /* CAM_LMVO_CON3, CAM_A_LMVO_CON3*/

typedef volatile union _CAM_REG_LMVO_CON4_
{
        volatile struct /* 0x1A0052BC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_CON4; /* CAM_LMVO_CON4, CAM_A_LMVO_CON4*/

typedef volatile union _CAM_REG_FLKO_BASE_ADDR_
{
        volatile struct /* 0x1A0052C0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_BASE_ADDR;    /* CAM_FLKO_BASE_ADDR, CAM_A_FLKO_BASE_ADDR*/

typedef volatile union _CAM_REG_FLKO_OFST_ADDR_
{
        volatile struct /* 0x1A0052C4 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_OFST_ADDR;    /* CAM_FLKO_OFST_ADDR, CAM_A_FLKO_OFST_ADDR*/

typedef volatile union _CAM_REG_FLKO_DRS_
{
        volatile struct /* 0x1A0052CC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_DRS;  /* CAM_FLKO_DRS, CAM_A_FLKO_DRS*/

typedef volatile union _CAM_REG_FLKO_XSIZE_
{
        volatile struct /* 0x1A0052D0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_XSIZE;    /* CAM_FLKO_XSIZE, CAM_A_FLKO_XSIZE*/

typedef volatile union _CAM_REG_FLKO_YSIZE_
{
        volatile struct /* 0x1A0052D4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_YSIZE;    /* CAM_FLKO_YSIZE, CAM_A_FLKO_YSIZE*/

typedef volatile union _CAM_REG_FLKO_STRIDE_
{
        volatile struct /* 0x1A0052D8 */
        {
                FIELD  RSV                                   : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                :  7;      /* 17..23, 0x00FE0000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_STRIDE;   /* CAM_FLKO_STRIDE, CAM_A_FLKO_STRIDE*/

typedef volatile union _CAM_REG_FLKO_CON_
{
        volatile struct /* 0x1A0052DC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_CON;  /* CAM_FLKO_CON, CAM_A_FLKO_CON*/

typedef volatile union _CAM_REG_FLKO_CON2_
{
        volatile struct /* 0x1A0052E0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_CON2; /* CAM_FLKO_CON2, CAM_A_FLKO_CON2*/

typedef volatile union _CAM_REG_FLKO_CON3_
{
        volatile struct /* 0x1A0052E4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_CON3; /* CAM_FLKO_CON3, CAM_A_FLKO_CON3*/

typedef volatile union _CAM_REG_FLKO_CON4_
{
        volatile struct /* 0x1A0052EC */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_CON4; /* CAM_FLKO_CON4, CAM_A_FLKO_CON4*/

typedef volatile union _CAM_REG_RSSO_A_BASE_ADDR_
{
        volatile struct /* 0x1A0052F0 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_BASE_ADDR;  /* CAM_RSSO_A_BASE_ADDR, CAM_A_RSSO_A_BASE_ADDR*/

typedef volatile union _CAM_REG_RSSO_A_OFST_ADDR_
{
        volatile struct /* 0x1A0052F4 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_OFST_ADDR;  /* CAM_RSSO_A_OFST_ADDR, CAM_A_RSSO_A_OFST_ADDR*/

typedef volatile union _CAM_REG_RSSO_A_DRS_
{
        volatile struct /* 0x1A0052FC */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_DRS;    /* CAM_RSSO_A_DRS, CAM_A_RSSO_A_DRS*/

typedef volatile union _CAM_REG_RSSO_A_XSIZE_
{
        volatile struct /* 0x1A005300 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_XSIZE;  /* CAM_RSSO_A_XSIZE, CAM_A_RSSO_A_XSIZE*/

typedef volatile union _CAM_REG_RSSO_A_YSIZE_
{
        volatile struct /* 0x1A005304 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_YSIZE;  /* CAM_RSSO_A_YSIZE, CAM_A_RSSO_A_YSIZE*/

typedef volatile union _CAM_REG_RSSO_A_STRIDE_
{
        volatile struct /* 0x1A005308 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_STRIDE; /* CAM_RSSO_A_STRIDE, CAM_A_RSSO_A_STRIDE*/

typedef volatile union _CAM_REG_RSSO_A_CON_
{
        volatile struct /* 0x1A00530C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_CON;    /* CAM_RSSO_A_CON, CAM_A_RSSO_A_CON*/

typedef volatile union _CAM_REG_RSSO_A_CON2_
{
        volatile struct /* 0x1A005310 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_CON2;   /* CAM_RSSO_A_CON2, CAM_A_RSSO_A_CON2*/

typedef volatile union _CAM_REG_RSSO_A_CON3_
{
        volatile struct /* 0x1A005314 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_CON3;   /* CAM_RSSO_A_CON3, CAM_A_RSSO_A_CON3*/

typedef volatile union _CAM_REG_RSSO_A_CON4_
{
        volatile struct /* 0x1A00531C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_CON4;   /* CAM_RSSO_A_CON4, CAM_A_RSSO_A_CON4*/

typedef volatile union _CAM_REG_UFGO_BASE_ADDR_
{
        volatile struct /* 0x1A005320 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_BASE_ADDR;    /* CAM_UFGO_BASE_ADDR, CAM_A_UFGO_BASE_ADDR*/

typedef volatile union _CAM_REG_UFGO_OFST_ADDR_
{
        volatile struct /* 0x1A005324 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_OFST_ADDR;    /* CAM_UFGO_OFST_ADDR, CAM_A_UFGO_OFST_ADDR*/

typedef volatile union _CAM_REG_UFGO_DRS_
{
        volatile struct /* 0x1A00532C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_DRS;  /* CAM_UFGO_DRS, CAM_A_UFGO_DRS*/

typedef volatile union _CAM_REG_UFGO_XSIZE_
{
        volatile struct /* 0x1A005330 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_XSIZE;    /* CAM_UFGO_XSIZE, CAM_A_UFGO_XSIZE*/

typedef volatile union _CAM_REG_UFGO_YSIZE_
{
        volatile struct /* 0x1A005334 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_YSIZE;    /* CAM_UFGO_YSIZE, CAM_A_UFGO_YSIZE*/

typedef volatile union _CAM_REG_UFGO_STRIDE_
{
        volatile struct /* 0x1A005338 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_STRIDE;   /* CAM_UFGO_STRIDE, CAM_A_UFGO_STRIDE*/

typedef volatile union _CAM_REG_UFGO_CON_
{
        volatile struct /* 0x1A00533C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_CON;  /* CAM_UFGO_CON, CAM_A_UFGO_CON*/

typedef volatile union _CAM_REG_UFGO_CON2_
{
        volatile struct /* 0x1A005340 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_CON2; /* CAM_UFGO_CON2, CAM_A_UFGO_CON2*/

typedef volatile union _CAM_REG_UFGO_CON3_
{
        volatile struct /* 0x1A005344 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_CON3; /* CAM_UFGO_CON3, CAM_A_UFGO_CON3*/

typedef volatile union _CAM_REG_UFGO_CON4_
{
        volatile struct /* 0x1A00534C */
        {
                FIELD  FIFO_DVFS_THRL                        : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DVFS_THRH                        : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DVFS_EN                          :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_CON4; /* CAM_UFGO_CON4, CAM_A_UFGO_CON4*/

typedef volatile union _CAM_REG_DMA_ERR_CTRL_
{
        volatile struct /* 0x1A005350 */
        {
                FIELD  IMGO_A_ERR                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_A_ERR                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_A_ERR                             :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_A_ERR                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_A_ERR                            :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_A_ERR                            :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_A_ERR                             :  1;      /*  6.. 6, 0x00000040 */
                FIELD  PSO_A_ERR                             :  1;      /*  7.. 7, 0x00000080 */
                FIELD  LMVO_A_ERR                            :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FLKO_A_ERR                            :  1;      /*  9.. 9, 0x00000200 */
                FIELD  RSSO_A_ERR                            :  1;      /* 10..10, 0x00000400 */
                FIELD  UFGO_A_ERR                            :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                :  3;      /* 12..14, 0x00007000 */
                FIELD  PDI_A_ERR                             :  1;      /* 15..15, 0x00008000 */
                FIELD  BPCI_A_ERR                            :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_A_ERR                            :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_A_ERR                            :  1;      /* 18..18, 0x00040000 */
                FIELD  LSC3I_A_ERR                           :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 11;      /* 20..30, 0x7FF00000 */
                FIELD  ERR_CLR_MD                            :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_ERR_CTRL;  /* CAM_DMA_ERR_CTRL, CAM_A_DMA_ERR_CTRL*/

typedef volatile union _CAM_REG_IMGO_ERR_STAT_
{
        volatile struct /* 0x1A005360 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_ERR_STAT; /* CAM_IMGO_ERR_STAT, CAM_A_IMGO_ERR_STAT*/

typedef volatile union _CAM_REG_RRZO_ERR_STAT_
{
        volatile struct /* 0x1A005364 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_ERR_STAT; /* CAM_RRZO_ERR_STAT, CAM_A_RRZO_ERR_STAT*/

typedef volatile union _CAM_REG_AAO_ERR_STAT_
{
        volatile struct /* 0x1A005368 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_ERR_STAT;  /* CAM_AAO_ERR_STAT, CAM_A_AAO_ERR_STAT*/

typedef volatile union _CAM_REG_AFO_ERR_STAT_
{
        volatile struct /* 0x1A00536C */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_ERR_STAT;  /* CAM_AFO_ERR_STAT, CAM_A_AFO_ERR_STAT*/

typedef volatile union _CAM_REG_LCSO_ERR_STAT_
{
        volatile struct /* 0x1A005370 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_ERR_STAT; /* CAM_LCSO_ERR_STAT, CAM_A_LCSO_ERR_STAT*/

typedef volatile union _CAM_REG_UFEO_ERR_STAT_
{
        volatile struct /* 0x1A005374 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_ERR_STAT; /* CAM_UFEO_ERR_STAT, CAM_A_UFEO_ERR_STAT*/

typedef volatile union _CAM_REG_PDO_ERR_STAT_
{
        volatile struct /* 0x1A005378 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_ERR_STAT;  /* CAM_PDO_ERR_STAT, CAM_A_PDO_ERR_STAT*/

typedef volatile union _CAM_REG_BPCI_ERR_STAT_
{
        volatile struct /* 0x1A00537C */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_BPCI_ERR_STAT; /* CAM_BPCI_ERR_STAT, CAM_A_BPCI_ERR_STAT*/

typedef volatile union _CAM_REG_CACI_ERR_STAT_
{
        volatile struct /* 0x1A005380 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_CACI_ERR_STAT; /* CAM_CACI_ERR_STAT, CAM_A_CACI_ERR_STAT*/

typedef volatile union _CAM_REG_LSCI_ERR_STAT_
{
        volatile struct /* 0x1A005384 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSCI_ERR_STAT; /* CAM_LSCI_ERR_STAT, CAM_A_LSCI_ERR_STAT*/

typedef volatile union _CAM_REG_LSC3I_ERR_STAT_
{
        volatile struct /* 0x1A005388 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LSC3I_ERR_STAT;    /* CAM_LSC3I_ERR_STAT, CAM_A_LSC3I_ERR_STAT*/

typedef volatile union _CAM_REG_PDI_ERR_STAT_
{
        volatile struct /* 0x1A00538C */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDI_ERR_STAT;  /* CAM_PDI_ERR_STAT, CAM_A_PDI_ERR_STAT*/

typedef volatile union _CAM_REG_LMVO_ERR_STAT_
{
        volatile struct /* 0x1A005390 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_ERR_STAT; /* CAM_LMVO_ERR_STAT, CAM_A_LMVO_ERR_STAT*/

typedef volatile union _CAM_REG_FLKO_ERR_STAT_
{
        volatile struct /* 0x1A005394 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_ERR_STAT; /* CAM_FLKO_ERR_STAT, CAM_A_FLKO_ERR_STAT*/

typedef volatile union _CAM_REG_RSSO_A_ERR_STAT_
{
        volatile struct /* 0x1A005398 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_ERR_STAT;   /* CAM_RSSO_A_ERR_STAT, CAM_A_RSSO_A_ERR_STAT*/

typedef volatile union _CAM_REG_UFGO_ERR_STAT_
{
        volatile struct /* 0x1A00539C */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_ERR_STAT; /* CAM_UFGO_ERR_STAT, CAM_A_UFGO_ERR_STAT*/

typedef volatile union _CAM_REG_PSO_ERR_STAT_
{
        volatile struct /* 0x1A0053A0 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_ERR_STAT;  /* CAM_PSO_ERR_STAT, CAM_A_PSO_ERR_STAT*/

typedef volatile union _CAM_REG_DMA_DEBUG_ADDR_
{
        volatile struct /* 0x1A0053AC */
        {
                FIELD  DEBUG_ADDR                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_DEBUG_ADDR;    /* CAM_DMA_DEBUG_ADDR, CAM_A_DMA_DEBUG_ADDR*/

typedef volatile union _CAM_REG_DMA_RSV1_
{
        volatile struct /* 0x1A0053B0 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_RSV1;  /* CAM_DMA_RSV1, CAM_A_DMA_RSV1*/

typedef volatile union _CAM_REG_DMA_RSV2_
{
        volatile struct /* 0x1A0053B4 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_RSV2;  /* CAM_DMA_RSV2, CAM_A_DMA_RSV2*/

typedef volatile union _CAM_REG_MAGIC_NUM_
{
        volatile struct /* 0x1A0053B8 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_MAGIC_NUM;  /* CAM_DMA_RSV3, CAM_A_DMA_RSV3*/

typedef volatile union _CAM_REG_DATA_PATTERN_
{
        volatile struct /* 0x1A0053BC */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DATA_PATTERN;  /* CAM_DMA_RSV4, CAM_A_DMA_RSV4*/

typedef volatile union _CAM_REG_DMA_RSV5_
{
        volatile struct /* 0x1A0053C0 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_RSV5;  /* CAM_DMA_RSV5, CAM_A_DMA_RSV5*/

typedef volatile union _CAM_REG_DMA_RSV6_
{
        volatile struct /* 0x1A0053C4 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_RSV6;  /* CAM_DMA_RSV6, CAM_A_DMA_RSV6*/

typedef volatile union _CAM_REG_DMA_DEBUG_SEL_
{
        volatile struct /* 0x1A0053C8 */
        {
                FIELD  DMA_TOP_SEL                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  R_W_DMA_TOP_SEL                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SUB_MODULE_SEL                        :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  4;      /* 24..27, 0x0F000000 */
                FIELD  PDO_FIFO_FULL_XSIZE                   :  1;      /* 28..28, 0x10000000 */
                FIELD  IMGO_UFE_FIFO_FULL_XSIZE              :  1;      /* 29..29, 0x20000000 */
                FIELD  ARBITER_BVALID_FULL                   :  1;      /* 30..30, 0x40000000 */
                FIELD  ARBITER_COM_FULL                      :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_DEBUG_SEL; /* CAM_DMA_DEBUG_SEL, CAM_A_DMA_DEBUG_SEL*/

typedef volatile union _CAM_REG_DMA_BW_SELF_TEST_
{
        volatile struct /* 0x1A0053CC */
        {
                FIELD  BW_SELF_TEST_EN_IMGO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  BW_SELF_TEST_EN_RRZO                  :  1;      /*  1.. 1, 0x00000002 */
                FIELD  BW_SELF_TEST_EN_AAO                   :  1;      /*  2.. 2, 0x00000004 */
                FIELD  BW_SELF_TEST_EN_AFO                   :  1;      /*  3.. 3, 0x00000008 */
                FIELD  BW_SELF_TEST_EN_LCSO                  :  1;      /*  4.. 4, 0x00000010 */
                FIELD  BW_SELF_TEST_EN_UFEO                  :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BW_SELF_TEST_EN_PDO                   :  1;      /*  6.. 6, 0x00000040 */
                FIELD  BW_SELF_TEST_EN_PSO                   :  1;      /*  7.. 7, 0x00000080 */
                FIELD  BW_SELF_TEST_EN_LMVO                  :  1;      /*  8.. 8, 0x00000100 */
                FIELD  BW_SELF_TEST_EN_FLKO                  :  1;      /*  9.. 9, 0x00000200 */
                FIELD  BW_SELF_TEST_EN_RSSO_A                :  1;      /* 10..10, 0x00000400 */
                FIELD  BW_SELF_TEST_EN_UFGO                  :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_BW_SELF_TEST;  /* CAM_DMA_BW_SELF_TEST, CAM_A_DMA_BW_SELF_TEST*/

typedef volatile union _CAM_REG_DMA_FRAME_HEADER_EN_
{
        volatile struct /* 0x1A005400 */
        {
                FIELD  FRAME_HEADER_EN_IMGO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FRAME_HEADER_EN_RRZO                  :  1;      /*  1.. 1, 0x00000002 */
                FIELD  FRAME_HEADER_EN_AAO                   :  1;      /*  2.. 2, 0x00000004 */
                FIELD  FRAME_HEADER_EN_AFO                   :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FRAME_HEADER_EN_LCSO                  :  1;      /*  4.. 4, 0x00000010 */
                FIELD  FRAME_HEADER_EN_UFEO                  :  1;      /*  5.. 5, 0x00000020 */
                FIELD  FRAME_HEADER_EN_PDO                   :  1;      /*  6.. 6, 0x00000040 */
                FIELD  FRAME_HEADER_EN_PSO                   :  1;      /*  7.. 7, 0x00000080 */
                FIELD  FRAME_HEADER_EN_LMVO                  :  1;      /*  8.. 8, 0x00000100 */
                FIELD  FRAME_HEADER_EN_FLKO                  :  1;      /*  9.. 9, 0x00000200 */
                FIELD  FRAME_HEADER_EN_RSSO_A                :  1;      /* 10..10, 0x00000400 */
                FIELD  FRAME_HEADER_EN_UFGO                  :  1;      /* 11..11, 0x00000800 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAM_REG_DMA_FRAME_HEADER_EN;   /* CAM_DMA_FRAME_HEADER_EN, CAM_A_DMA_FRAME_HEADER_EN*/

typedef volatile union _CAM_REG_IMGO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005404 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_BASE_ADDR; /* CAM_IMGO_FH_BASE_ADDR, CAM_A_IMGO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_RRZO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005408 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_BASE_ADDR; /* CAM_RRZO_FH_BASE_ADDR, CAM_A_RRZO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_AAO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A00540C */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_BASE_ADDR;  /* CAM_AAO_FH_BASE_ADDR, CAM_A_AAO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_AFO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005410 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_BASE_ADDR;  /* CAM_AFO_FH_BASE_ADDR, CAM_A_AFO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_LCSO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005414 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_BASE_ADDR; /* CAM_LCSO_FH_BASE_ADDR, CAM_A_LCSO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_UFEO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005418 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_BASE_ADDR; /* CAM_UFEO_FH_BASE_ADDR, CAM_A_UFEO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_PDO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A00541C */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_BASE_ADDR;  /* CAM_PDO_FH_BASE_ADDR, CAM_A_PDO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_PSO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005420 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_BASE_ADDR;  /* CAM_PSO_FH_BASE_ADDR, CAM_A_PSO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_LMVO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005424 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_BASE_ADDR; /* CAM_LMVO_FH_BASE_ADDR, CAM_A_LMVO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_FLKO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005428 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_BASE_ADDR; /* CAM_FLKO_FH_BASE_ADDR, CAM_A_FLKO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_RSSO_A_FH_BASE_ADDR_
{
        volatile struct /* 0x1A00542C */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_BASE_ADDR;   /* CAM_RSSO_A_FH_BASE_ADDR, CAM_A_RSSO_A_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_UFGO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A005430 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_BASE_ADDR; /* CAM_UFGO_FH_BASE_ADDR, CAM_A_UFGO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_2_
{
        volatile struct /* 0x1A005434 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_2;   /* CAM_IMGO_FH_SPARE_2, CAM_A_IMGO_FH_SPARE_2*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_3_
{
        volatile struct /* 0x1A005438 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_3;   /* CAM_IMGO_FH_SPARE_3, CAM_A_IMGO_FH_SPARE_3*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_4_
{
        volatile struct /* 0x1A00543C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_4;   /* CAM_IMGO_FH_SPARE_4, CAM_A_IMGO_FH_SPARE_4*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_5_
{
        volatile struct /* 0x1A005440 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_5;   /* CAM_IMGO_FH_SPARE_5, CAM_A_IMGO_FH_SPARE_5*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_6_
{
        volatile struct /* 0x1A005444 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_6;   /* CAM_IMGO_FH_SPARE_6, CAM_A_IMGO_FH_SPARE_6*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_7_
{
        volatile struct /* 0x1A005448 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_7;   /* CAM_IMGO_FH_SPARE_7, CAM_A_IMGO_FH_SPARE_7*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_8_
{
        volatile struct /* 0x1A00544C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_8;   /* CAM_IMGO_FH_SPARE_8, CAM_A_IMGO_FH_SPARE_8*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_9_
{
        volatile struct /* 0x1A005450 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_9;   /* CAM_IMGO_FH_SPARE_9, CAM_A_IMGO_FH_SPARE_9*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_10_
{
        volatile struct /* 0x1A005454 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_10;  /* CAM_IMGO_FH_SPARE_10, CAM_A_IMGO_FH_SPARE_10*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_11_
{
        volatile struct /* 0x1A005458 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_11;  /* CAM_IMGO_FH_SPARE_11, CAM_A_IMGO_FH_SPARE_11*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_12_
{
        volatile struct /* 0x1A00545C */
        {
                FIELD  E_IQ_LEVEL                              : 2;      /*  0..1, 0x00000003 */
                FIELD  E_IS_UFE_FMT                         : 1;      /*  2..2, 0x00000004 */
                FIELD  rsv                                             : 29;     /* 3..31, 0xFFFFFFF8 */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_12;  /* CAM_IMGO_FH_SPARE_12, CAM_A_IMGO_FH_SPARE_12*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_13_
{
        volatile struct /* 0x1A005460 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_13;  /* CAM_IMGO_FH_SPARE_13, CAM_A_IMGO_FH_SPARE_13*/


typedef volatile union _CAM_REG_RRZO_FH_SPARE_2_
{
        volatile struct /* 0x1A005474 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_2;   /* CAM_RRZO_FH_SPARE_2, CAM_A_RRZO_FH_SPARE_2*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_3_
{
        volatile struct /* 0x1A005478 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_3;   /* CAM_RRZO_FH_SPARE_3, CAM_A_RRZO_FH_SPARE_3*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_4_
{
        volatile struct /* 0x1A00547C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_4;   /* CAM_RRZO_FH_SPARE_4, CAM_A_RRZO_FH_SPARE_4*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_5_
{
        volatile struct /* 0x1A005480 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_5;   /* CAM_RRZO_FH_SPARE_5, CAM_A_RRZO_FH_SPARE_5*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_6_
{
        volatile struct /* 0x1A005484 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_6;   /* CAM_RRZO_FH_SPARE_6, CAM_A_RRZO_FH_SPARE_6*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_7_
{
        volatile struct /* 0x1A005488 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_7;   /* CAM_RRZO_FH_SPARE_7, CAM_A_RRZO_FH_SPARE_7*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_8_
{
        volatile struct /* 0x1A00548C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_8;   /* CAM_RRZO_FH_SPARE_8, CAM_A_RRZO_FH_SPARE_8*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_9_
{
        volatile struct /* 0x1A005490 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_9;   /* CAM_RRZO_FH_SPARE_9, CAM_A_RRZO_FH_SPARE_9*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_10_
{
        volatile struct /* 0x1A005494 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_10;  /* CAM_RRZO_FH_SPARE_10, CAM_A_RRZO_FH_SPARE_10*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_11_
{
        volatile struct /* 0x1A005498 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_11;  /* CAM_RRZO_FH_SPARE_11, CAM_A_RRZO_FH_SPARE_11*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_12_
{
        volatile struct /* 0x1A00549C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_12;  /* CAM_RRZO_FH_SPARE_12, CAM_A_RRZO_FH_SPARE_12*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_13_
{
        volatile struct /* 0x1A0054A0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_13;  /* CAM_RRZO_FH_SPARE_13, CAM_A_RRZO_FH_SPARE_13*/


typedef volatile union _CAM_REG_AAO_FH_SPARE_2_
{
        volatile struct /* 0x1A0054B4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_2;    /* CAM_AAO_FH_SPARE_2, CAM_A_AAO_FH_SPARE_2*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_3_
{
        volatile struct /* 0x1A0054B8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_3;    /* CAM_AAO_FH_SPARE_3, CAM_A_AAO_FH_SPARE_3*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_4_
{
        volatile struct /* 0x1A0054BC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_4;    /* CAM_AAO_FH_SPARE_4, CAM_A_AAO_FH_SPARE_4*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_5_
{
        volatile struct /* 0x1A0054C0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_5;    /* CAM_AAO_FH_SPARE_5, CAM_A_AAO_FH_SPARE_5*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_6_
{
        volatile struct /* 0x1A0054C4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_6;    /* CAM_AAO_FH_SPARE_6, CAM_A_AAO_FH_SPARE_6*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_7_
{
        volatile struct /* 0x1A0054C8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_7;    /* CAM_AAO_FH_SPARE_7, CAM_A_AAO_FH_SPARE_7*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_8_
{
        volatile struct /* 0x1A0054CC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_8;    /* CAM_AAO_FH_SPARE_8, CAM_A_AAO_FH_SPARE_8*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_9_
{
        volatile struct /* 0x1A0054D0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_9;    /* CAM_AAO_FH_SPARE_9, CAM_A_AAO_FH_SPARE_9*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_10_
{
        volatile struct /* 0x1A0054D4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_10;   /* CAM_AAO_FH_SPARE_10, CAM_A_AAO_FH_SPARE_10*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_11_
{
        volatile struct /* 0x1A0054D8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_11;   /* CAM_AAO_FH_SPARE_11, CAM_A_AAO_FH_SPARE_11*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_12_
{
        volatile struct /* 0x1A0054DC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_12;   /* CAM_AAO_FH_SPARE_12, CAM_A_AAO_FH_SPARE_12*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_13_
{
        volatile struct /* 0x1A0054E0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_13;   /* CAM_AAO_FH_SPARE_13, CAM_A_AAO_FH_SPARE_13*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_2_
{
        volatile struct /* 0x1A0054F4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_2;    /* CAM_AFO_FH_SPARE_2, CAM_A_AFO_FH_SPARE_2*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_3_
{
        volatile struct /* 0x1A0054F8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_3;    /* CAM_AFO_FH_SPARE_3, CAM_A_AFO_FH_SPARE_3*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_4_
{
        volatile struct /* 0x1A0054FC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_4;    /* CAM_AFO_FH_SPARE_4, CAM_A_AFO_FH_SPARE_4*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_5_
{
        volatile struct /* 0x1A005500 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_5;    /* CAM_AFO_FH_SPARE_5, CAM_A_AFO_FH_SPARE_5*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_6_
{
        volatile struct /* 0x1A005504 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_6;    /* CAM_AFO_FH_SPARE_6, CAM_A_AFO_FH_SPARE_6*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_7_
{
        volatile struct /* 0x1A005508 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_7;    /* CAM_AFO_FH_SPARE_7, CAM_A_AFO_FH_SPARE_7*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_8_
{
        volatile struct /* 0x1A00550C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_8;    /* CAM_AFO_FH_SPARE_8, CAM_A_AFO_FH_SPARE_8*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_9_
{
        volatile struct /* 0x1A005510 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_9;    /* CAM_AFO_FH_SPARE_9, CAM_A_AFO_FH_SPARE_9*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_10_
{
        volatile struct /* 0x1A005514 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_10;   /* CAM_AFO_FH_SPARE_10, CAM_A_AFO_FH_SPARE_10*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_11_
{
        volatile struct /* 0x1A005518 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_11;   /* CAM_AFO_FH_SPARE_11, CAM_A_AFO_FH_SPARE_11*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_12_
{
        volatile struct /* 0x1A00551C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_12;   /* CAM_AFO_FH_SPARE_12, CAM_A_AFO_FH_SPARE_12*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_13_
{
        volatile struct /* 0x1A005520 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_13;   /* CAM_AFO_FH_SPARE_13, CAM_A_AFO_FH_SPARE_13*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_2_
{
        volatile struct /* 0x1A005534 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_2;   /* CAM_LCSO_FH_SPARE_2, CAM_A_LCSO_FH_SPARE_2*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_3_
{
        volatile struct /* 0x1A005538 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_3;   /* CAM_LCSO_FH_SPARE_3, CAM_A_LCSO_FH_SPARE_3*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_4_
{
        volatile struct /* 0x1A00553C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_4;   /* CAM_LCSO_FH_SPARE_4, CAM_A_LCSO_FH_SPARE_4*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_5_
{
        volatile struct /* 0x1A005540 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_5;   /* CAM_LCSO_FH_SPARE_5, CAM_A_LCSO_FH_SPARE_5*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_6_
{
        volatile struct /* 0x1A005544 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_6;   /* CAM_LCSO_FH_SPARE_6, CAM_A_LCSO_FH_SPARE_6*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_7_
{
        volatile struct /* 0x1A005548 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_7;   /* CAM_LCSO_FH_SPARE_7, CAM_A_LCSO_FH_SPARE_7*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_8_
{
        volatile struct /* 0x1A00554C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_8;   /* CAM_LCSO_FH_SPARE_8, CAM_A_LCSO_FH_SPARE_8*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_9_
{
        volatile struct /* 0x1A005550 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_9;   /* CAM_LCSO_FH_SPARE_9, CAM_A_LCSO_FH_SPARE_9*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_10_
{
        volatile struct /* 0x1A005554 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_10;  /* CAM_LCSO_FH_SPARE_10, CAM_A_LCSO_FH_SPARE_10*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_11_
{
        volatile struct /* 0x1A005558 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_11;  /* CAM_LCSO_FH_SPARE_11, CAM_A_LCSO_FH_SPARE_11*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_12_
{
        volatile struct /* 0x1A00555C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_12;  /* CAM_LCSO_FH_SPARE_12, CAM_A_LCSO_FH_SPARE_12*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_13_
{
        volatile struct /* 0x1A005560 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_13;  /* CAM_LCSO_FH_SPARE_13, CAM_A_LCSO_FH_SPARE_13*/


typedef volatile union _CAM_REG_UFEO_FH_SPARE_2_
{
        volatile struct /* 0x1A005574 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_2;   /* CAM_UFEO_FH_SPARE_2, CAM_A_UFEO_FH_SPARE_2*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_3_
{
        volatile struct /* 0x1A005578 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_3;   /* CAM_UFEO_FH_SPARE_3, CAM_A_UFEO_FH_SPARE_3*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_4_
{
        volatile struct /* 0x1A00557C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_4;   /* CAM_UFEO_FH_SPARE_4, CAM_A_UFEO_FH_SPARE_4*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_5_
{
        volatile struct /* 0x1A005580 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_5;   /* CAM_UFEO_FH_SPARE_5, CAM_A_UFEO_FH_SPARE_5*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_6_
{
        volatile struct /* 0x1A005584 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_6;   /* CAM_UFEO_FH_SPARE_6, CAM_A_UFEO_FH_SPARE_6*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_7_
{
        volatile struct /* 0x1A005588 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_7;   /* CAM_UFEO_FH_SPARE_7, CAM_A_UFEO_FH_SPARE_7*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_8_
{
        volatile struct /* 0x1A00558C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_8;   /* CAM_UFEO_FH_SPARE_8, CAM_A_UFEO_FH_SPARE_8*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_9_
{
        volatile struct /* 0x1A005590 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_9;   /* CAM_UFEO_FH_SPARE_9, CAM_A_UFEO_FH_SPARE_9*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_10_
{
        volatile struct /* 0x1A005594 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_10;  /* CAM_UFEO_FH_SPARE_10, CAM_A_UFEO_FH_SPARE_10*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_11_
{
        volatile struct /* 0x1A005598 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_11;  /* CAM_UFEO_FH_SPARE_11, CAM_A_UFEO_FH_SPARE_11*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_12_
{
        volatile struct /* 0x1A00559C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_12;  /* CAM_UFEO_FH_SPARE_12, CAM_A_UFEO_FH_SPARE_12*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_13_
{
        volatile struct /* 0x1A0055A0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_13;  /* CAM_UFEO_FH_SPARE_13, CAM_A_UFEO_FH_SPARE_13*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_2_
{
        volatile struct /* 0x1A0055B4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_2;    /* CAM_PDO_FH_SPARE_2, CAM_A_PDO_FH_SPARE_2*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_3_
{
        volatile struct /* 0x1A0055B8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_3;    /* CAM_PDO_FH_SPARE_3, CAM_A_PDO_FH_SPARE_3*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_4_
{
        volatile struct /* 0x1A0055BC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_4;    /* CAM_PDO_FH_SPARE_4, CAM_A_PDO_FH_SPARE_4*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_5_
{
        volatile struct /* 0x1A0055C0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_5;    /* CAM_PDO_FH_SPARE_5, CAM_A_PDO_FH_SPARE_5*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_6_
{
        volatile struct /* 0x1A0055C4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_6;    /* CAM_PDO_FH_SPARE_6, CAM_A_PDO_FH_SPARE_6*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_7_
{
        volatile struct /* 0x1A0055C8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_7;    /* CAM_PDO_FH_SPARE_7, CAM_A_PDO_FH_SPARE_7*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_8_
{
        volatile struct /* 0x1A0055CC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_8;    /* CAM_PDO_FH_SPARE_8, CAM_A_PDO_FH_SPARE_8*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_9_
{
        volatile struct /* 0x1A0055D0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_9;    /* CAM_PDO_FH_SPARE_9, CAM_A_PDO_FH_SPARE_9*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_10_
{
        volatile struct /* 0x1A0055D4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_10;   /* CAM_PDO_FH_SPARE_10, CAM_A_PDO_FH_SPARE_10*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_11_
{
        volatile struct /* 0x1A0055D8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_11;   /* CAM_PDO_FH_SPARE_11, CAM_A_PDO_FH_SPARE_11*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_12_
{
        volatile struct /* 0x1A0055DC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_12;   /* CAM_PDO_FH_SPARE_12, CAM_A_PDO_FH_SPARE_12*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_13_
{
        volatile struct /* 0x1A0055E0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_13;   /* CAM_PDO_FH_SPARE_13, CAM_A_PDO_FH_SPARE_13*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_4_
{
        volatile struct /* 0x1A0055F0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_4;    /* CAM_PSO_FH_SPARE_4, CAM_A_PSO_FH_SPARE_4*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_2_
{
        volatile struct /* 0x1A0055F4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_2;    /* CAM_PSO_FH_SPARE_2, CAM_A_PSO_FH_SPARE_2*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_3_
{
        volatile struct /* 0x1A0055F8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_3;    /* CAM_PSO_FH_SPARE_3, CAM_A_PSO_FH_SPARE_3*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_5_
{
        volatile struct /* 0x1A005600 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_5;    /* CAM_PSO_FH_SPARE_5, CAM_A_PSO_FH_SPARE_5*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_6_
{
        volatile struct /* 0x1A005604 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_6;    /* CAM_PSO_FH_SPARE_6, CAM_A_PSO_FH_SPARE_6*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_7_
{
        volatile struct /* 0x1A005608 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_7;    /* CAM_PSO_FH_SPARE_7, CAM_A_PSO_FH_SPARE_7*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_8_
{
        volatile struct /* 0x1A00560C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_8;    /* CAM_PSO_FH_SPARE_8, CAM_A_PSO_FH_SPARE_8*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_9_
{
        volatile struct /* 0x1A005614 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_9;    /* CAM_PSO_FH_SPARE_9, CAM_A_PSO_FH_SPARE_9*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_10_
{
        volatile struct /* 0x1A005618 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_10;   /* CAM_PSO_FH_SPARE_10, CAM_A_PSO_FH_SPARE_10*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_11_
{
        volatile struct /* 0x1A00561C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_11;   /* CAM_PSO_FH_SPARE_11, CAM_A_PSO_FH_SPARE_11*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_12_
{
        volatile struct /* 0x1A005620 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_12;   /* CAM_PSO_FH_SPARE_12, CAM_A_PSO_FH_SPARE_12*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_13_
{
        volatile struct /* 0x1A005624 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_13;   /* CAM_PSO_FH_SPARE_13, CAM_A_PSO_FH_SPARE_13*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_2_
{
        volatile struct /* 0x1A005634 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_2;   /* CAM_LMVO_FH_SPARE_2, CAM_A_LMVO_FH_SPARE_2*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_3_
{
        volatile struct /* 0x1A005638 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_3;   /* CAM_LMVO_FH_SPARE_3, CAM_A_LMVO_FH_SPARE_3*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_4_
{
        volatile struct /* 0x1A00563C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_4;   /* CAM_LMVO_FH_SPARE_4, CAM_A_LMVO_FH_SPARE_4*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_5_
{
        volatile struct /* 0x1A005640 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_5;   /* CAM_LMVO_FH_SPARE_5, CAM_A_LMVO_FH_SPARE_5*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_6_
{
        volatile struct /* 0x1A005644 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_6;   /* CAM_LMVO_FH_SPARE_6, CAM_A_LMVO_FH_SPARE_6*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_7_
{
        volatile struct /* 0x1A005648 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_7;   /* CAM_LMVO_FH_SPARE_7, CAM_A_LMVO_FH_SPARE_7*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_8_
{
        volatile struct /* 0x1A00564C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_8;   /* CAM_LMVO_FH_SPARE_8, CAM_A_LMVO_FH_SPARE_8*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_9_
{
        volatile struct /* 0x1A005654 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_9;   /* CAM_LMVO_FH_SPARE_9, CAM_A_LMVO_FH_SPARE_9*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_10_
{
        volatile struct /* 0x1A005658 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_10;  /* CAM_LMVO_FH_SPARE_10, CAM_A_LMVO_FH_SPARE_10*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_11_
{
        volatile struct /* 0x1A00565C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_11;  /* CAM_LMVO_FH_SPARE_11, CAM_A_LMVO_FH_SPARE_11*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_12_
{
        volatile struct /* 0x1A005660 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_12;  /* CAM_LMVO_FH_SPARE_12, CAM_A_LMVO_FH_SPARE_12*/

typedef volatile union _CAM_REG_LMVO_FH_SPARE_13_
{
        volatile struct /* 0x1A005664 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_LMVO_FH_SPARE_13;  /* CAM_LMVO_FH_SPARE_13, CAM_A_LMVO_FH_SPARE_13*/


typedef volatile union _CAM_REG_FLKO_FH_SPARE_2_
{
        volatile struct /* 0x1A005674 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_2;   /* CAM_FLKO_FH_SPARE_2, CAM_A_FLKO_FH_SPARE_2*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_3_
{
        volatile struct /* 0x1A005678 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_3;   /* CAM_FLKO_FH_SPARE_3, CAM_A_FLKO_FH_SPARE_3*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_4_
{
        volatile struct /* 0x1A00567C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_4;   /* CAM_FLKO_FH_SPARE_4, CAM_A_FLKO_FH_SPARE_4*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_5_
{
        volatile struct /* 0x1A005680 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_5;   /* CAM_FLKO_FH_SPARE_5, CAM_A_FLKO_FH_SPARE_5*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_6_
{
        volatile struct /* 0x1A005684 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_6;   /* CAM_FLKO_FH_SPARE_6, CAM_A_FLKO_FH_SPARE_6*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_7_
{
        volatile struct /* 0x1A005688 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_7;   /* CAM_FLKO_FH_SPARE_7, CAM_A_FLKO_FH_SPARE_7*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_8_
{
        volatile struct /* 0x1A00568C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_8;   /* CAM_FLKO_FH_SPARE_8, CAM_A_FLKO_FH_SPARE_8*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_9_
{
        volatile struct /* 0x1A005694 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_9;   /* CAM_FLKO_FH_SPARE_9, CAM_A_FLKO_FH_SPARE_9*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_10_
{
        volatile struct /* 0x1A005698 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_10;  /* CAM_FLKO_FH_SPARE_10, CAM_A_FLKO_FH_SPARE_10*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_11_
{
        volatile struct /* 0x1A00569C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_11;  /* CAM_FLKO_FH_SPARE_11, CAM_A_FLKO_FH_SPARE_11*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_12_
{
        volatile struct /* 0x1A0056A0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_12;  /* CAM_FLKO_FH_SPARE_12, CAM_A_FLKO_FH_SPARE_12*/

typedef volatile union _CAM_REG_FLKO_FH_SPARE_13_
{
        volatile struct /* 0x1A0056A4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_FLKO_FH_SPARE_13;  /* CAM_FLKO_FH_SPARE_13, CAM_A_FLKO_FH_SPARE_13*/


typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_2_
{
        volatile struct /* 0x1A0056B4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_2; /* CAM_RSSO_A_FH_SPARE_2, CAM_A_RSSO_A_FH_SPARE_2*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_3_
{
        volatile struct /* 0x1A0056B8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_3; /* CAM_RSSO_A_FH_SPARE_3, CAM_A_RSSO_A_FH_SPARE_3*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_4_
{
        volatile struct /* 0x1A0056BC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_4; /* CAM_RSSO_A_FH_SPARE_4, CAM_A_RSSO_A_FH_SPARE_4*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_5_
{
        volatile struct /* 0x1A0056E0 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_5; /* CAM_RSSO_A_FH_SPARE_5, CAM_A_RSSO_A_FH_SPARE_5*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_6_
{
        volatile struct /* 0x1A0056E4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_6; /* CAM_RSSO_A_FH_SPARE_6, CAM_A_RSSO_A_FH_SPARE_6*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_7_
{
        volatile struct /* 0x1A0056E8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_7; /* CAM_RSSO_A_FH_SPARE_7, CAM_A_RSSO_A_FH_SPARE_7*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_8_
{
        volatile struct /* 0x1A0056EC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_8; /* CAM_RSSO_A_FH_SPARE_8, CAM_A_RSSO_A_FH_SPARE_8*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_9_
{
        volatile struct /* 0x1A0056F4 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_9; /* CAM_RSSO_A_FH_SPARE_9, CAM_A_RSSO_A_FH_SPARE_9*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_10_
{
        volatile struct /* 0x1A0056F8 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_10;    /* CAM_RSSO_A_FH_SPARE_10, CAM_A_RSSO_A_FH_SPARE_10*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_11_
{
        volatile struct /* 0x1A0056FC */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_11;    /* CAM_RSSO_A_FH_SPARE_11, CAM_A_RSSO_A_FH_SPARE_11*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_12_
{
        volatile struct /* 0x1A005700 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_12;    /* CAM_RSSO_A_FH_SPARE_12, CAM_A_RSSO_A_FH_SPARE_12*/

typedef volatile union _CAM_REG_RSSO_A_FH_SPARE_13_
{
        volatile struct /* 0x1A005704 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_RSSO_A_FH_SPARE_13;    /* CAM_RSSO_A_FH_SPARE_13, CAM_A_RSSO_A_FH_SPARE_13*/


typedef volatile union _CAM_REG_UFGO_FH_SPARE_2_
{
        volatile struct /* 0x1A005714 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_2;   /* CAM_UFGO_FH_SPARE_2, CAM_A_UFGO_FH_SPARE_2*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_3_
{
        volatile struct /* 0x1A005718 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_3;   /* CAM_UFGO_FH_SPARE_3, CAM_A_UFGO_FH_SPARE_3*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_4_
{
        volatile struct /* 0x1A00571C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_4;   /* CAM_UFGO_FH_SPARE_4, CAM_A_UFGO_FH_SPARE_4*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_5_
{
        volatile struct /* 0x1A005720 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_5;   /* CAM_UFGO_FH_SPARE_5, CAM_A_UFGO_FH_SPARE_5*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_6_
{
        volatile struct /* 0x1A005724 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_6;   /* CAM_UFGO_FH_SPARE_6, CAM_A_UFGO_FH_SPARE_6*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_7_
{
        volatile struct /* 0x1A005728 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_7;   /* CAM_UFGO_FH_SPARE_7, CAM_A_UFGO_FH_SPARE_7*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_8_
{
        volatile struct /* 0x1A00572C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_8;   /* CAM_UFGO_FH_SPARE_8, CAM_A_UFGO_FH_SPARE_8*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_9_
{
        volatile struct /* 0x1A005730 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_9;   /* CAM_UFGO_FH_SPARE_9, CAM_A_UFGO_FH_SPARE_9*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_10_
{
        volatile struct /* 0x1A005734 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_10;  /* CAM_UFGO_FH_SPARE_10, CAM_A_UFGO_FH_SPARE_10*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_11_
{
        volatile struct /* 0x1A005738 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_11;  /* CAM_UFGO_FH_SPARE_11, CAM_A_UFGO_FH_SPARE_11*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_12_
{
        volatile struct /* 0x1A00573C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_12;  /* CAM_UFGO_FH_SPARE_12, CAM_A_UFGO_FH_SPARE_12*/

typedef volatile union _CAM_REG_UFGO_FH_SPARE_13_
{
        volatile struct /* 0x1A005740 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAM_REG_UFGO_FH_SPARE_13;  /* CAM_UFGO_FH_SPARE_13, CAM_A_UFGO_FH_SPARE_13*/


typedef volatile union _CAMSV_REG_DMA_SOFT_RSTSTAT_
{
        volatile struct /* 0x1A050000 */
        {
                FIELD  IMGO_SOFT_RST_STAT                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_SOFT_RST_STAT                    :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_SOFT_RST_STAT                     :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_SOFT_RST_STAT                     :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_SOFT_RST_STAT                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_SOFT_RST_STAT                    :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_SOFT_RST_STAT                     :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  9;      /*  7..15, 0x0000FF80 */
                FIELD  BPCI_SOFT_RST_STAT                    :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_SOFT_RST_STAT                    :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_SOFT_RST_STAT                    :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_SOFT_RSTSTAT;    /* CAMSV_DMA_SOFT_RSTSTAT, CAMSV_0_DMA_SOFT_RSTSTAT*/

typedef volatile union _CAMSV_REG_CQ0I_BASE_ADDR_
{
        volatile struct /* 0x1A050004 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_CQ0I_BASE_ADDR;  /* CAMSV_CQ0I_BASE_ADDR, CAMSV_0_CQ0I_BASE_ADDR*/

typedef volatile union _CAMSV_REG_CQ0I_XSIZE_
{
        volatile struct /* 0x1A050008 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_CQ0I_XSIZE;  /* CAMSV_CQ0I_XSIZE, CAMSV_0_CQ0I_XSIZE*/

typedef volatile union _CAMSV_REG_VERTICAL_FLIP_EN_
{
        volatile struct /* 0x1A05000C */
        {
                FIELD  IMGO_V_FLIP_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_V_FLIP_EN                        :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_V_FLIP_EN                         :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_V_FLIP_EN                         :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_V_FLIP_EN                        :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_V_FLIP_EN                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_V_FLIP_EN                         :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  9;      /*  7..15, 0x0000FF80 */
                FIELD  BPCI_V_FLIP_EN                        :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_V_FLIP_EN                        :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_V_FLIP_EN                        :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_VERTICAL_FLIP_EN;    /* CAMSV_VERTICAL_FLIP_EN, CAMSV_0_VERTICAL_FLIP_EN*/

typedef volatile union _CAMSV_REG_DMA_SOFT_RESET_
{
        volatile struct /* 0x1A050010 */
        {
                FIELD  IMGO_SOFT_RST                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_SOFT_RST                         :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_SOFT_RST                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_SOFT_RST                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_SOFT_RST                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_SOFT_RST                         :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_SOFT_RST                          :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  9;      /*  7..15, 0x0000FF80 */
                FIELD  BPCI_SOFT_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_SOFT_RST                         :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_SOFT_RST                         :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 12;      /* 19..30, 0x7FF80000 */
                FIELD  SEPARATE_SOFT_RST_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_SOFT_RESET;  /* CAMSV_DMA_SOFT_RESET, CAMSV_0_DMA_SOFT_RESET*/

typedef volatile union _CAMSV_REG_LAST_ULTRA_EN_
{
        volatile struct /* 0x1A050014 */
        {
                FIELD  IMGO_LAST_ULTRA_EN                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_LAST_ULTRA_EN                    :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_LAST_ULTRA_EN                     :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_LAST_ULTRA_EN                     :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_LAST_ULTRA_EN                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_LAST_ULTRA_EN                    :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_LAST_ULTRA_EN                     :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  9;      /*  7..15, 0x0000FF80 */
                FIELD  BPCI_LAST_ULTRA_EN                    :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_LAST_ULTRA_EN                    :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_LAST_ULTRA_EN                    :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 13;      /* 19..31, 0xFFF80000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_LAST_ULTRA_EN;   /* CAMSV_LAST_ULTRA_EN, CAMSV_0_LAST_ULTRA_EN*/

typedef volatile union _CAMSV_REG_SPECIAL_FUN_EN_
{
        volatile struct /* 0x1A050018 */
        {
                FIELD  rsv_0                                 : 20;      /*  0..19, 0x000FFFFF */
                FIELD  CONTINUOUS_COM_CON                    :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  MULTI_PLANE_ID_EN                     :  1;      /* 24..24, 0x01000000 */
                FIELD  CONTINUOUS_COM_EN                     :  1;      /* 25..25, 0x02000000 */
                FIELD  FIFO_CHANGE_EN                        :  1;      /* 26..26, 0x04000000 */
                FIELD  rsv_27                                :  2;      /* 27..28, 0x18000000 */
                FIELD  CQ_ULTRA_BPCI_EN                      :  1;      /* 29..29, 0x20000000 */
                FIELD  CQ_ULTRA_LSCI_EN                      :  1;      /* 30..30, 0x40000000 */
                FIELD  UFO_IMGO_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_SPECIAL_FUN_EN;  /* CAMSV_SPECIAL_FUN_EN, CAMSV_0_SPECIAL_FUN_EN*/

typedef volatile union _CAMSV_REG_IMGO_BASE_ADDR_
{
        volatile struct /* 0x1A050020 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_BASE_ADDR;  /* CAMSV_IMGO_BASE_ADDR, CAMSV_0_IMGO_BASE_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_OFST_ADDR_
{
        volatile struct /* 0x1A050028 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_OFST_ADDR;  /* CAMSV_IMGO_OFST_ADDR, CAMSV_0_IMGO_OFST_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_DRS_
{
        volatile struct /* 0x1A05002C */
        {
                FIELD  FIFO_DRS_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_DRS_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  3;      /* 28..30, 0x70000000 */
                FIELD  FIFO_DRS_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_DRS;    /* CAMSV_IMGO_DRS, CAMSV_0_IMGO_DRS*/

typedef volatile union _CAMSV_REG_IMGO_XSIZE_
{
        volatile struct /* 0x1A050030 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_XSIZE;  /* CAMSV_IMGO_XSIZE, CAMSV_0_IMGO_XSIZE*/

typedef volatile union _CAMSV_REG_IMGO_YSIZE_
{
        volatile struct /* 0x1A050034 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_YSIZE;  /* CAMSV_IMGO_YSIZE, CAMSV_0_IMGO_YSIZE*/

typedef volatile union _CAMSV_REG_IMGO_STRIDE_
{
        volatile struct /* 0x1A050038 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  BUS_SIZE                              :  3;      /* 16..18, 0x00070000 */
                FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
                FIELD  FORMAT                                :  2;      /* 20..21, 0x00300000 */
                FIELD  rsv_22                                :  1;      /* 22..22, 0x00400000 */
                FIELD  FORMAT_EN                             :  1;      /* 23..23, 0x00800000 */
                FIELD  BUS_SIZE_EN                           :  1;      /* 24..24, 0x01000000 */
                FIELD  rsv_25                                :  7;      /* 25..31, 0xFE000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_STRIDE; /* CAMSV_IMGO_STRIDE, CAMSV_0_IMGO_STRIDE*/

typedef volatile union _CAMSV_REG_IMGO_CON_
{
        volatile struct /* 0x1A05003C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_CON;    /* CAMSV_IMGO_CON, CAMSV_0_IMGO_CON*/

typedef volatile union _CAMSV_REG_IMGO_CON2_
{
        volatile struct /* 0x1A050040 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_CON2;   /* CAMSV_IMGO_CON2, CAMSV_0_IMGO_CON2*/

typedef volatile union _CAMSV_REG_IMGO_CON3_
{
        volatile struct /* 0x1A050044 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_CON3;   /* CAMSV_IMGO_CON3, CAMSV_0_IMGO_CON3*/

typedef volatile union _CAMSV_REG_IMGO_CROP_
{
        volatile struct /* 0x1A050048 */
        {
                FIELD  XOFFSET                               : 16;      /*  0..15, 0x0000FFFF */
                FIELD  YOFFSET                               : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_CROP;   /* CAMSV_IMGO_CROP, CAMSV_0_IMGO_CROP*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CTL1_
{
        volatile struct /* 0x1A050110 */
        {
                FIELD  FBC_NUM                               :  6;      /*  0.. 5, 0x0000003F */
                FIELD  rsv_6                                 :  9;      /*  6..14, 0x00007FC0 */
                FIELD  FBC_EN                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FBC_MODE                              :  1;      /* 16..16, 0x00010000 */
                FIELD  LOCK_EN                               :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  2;      /* 18..19, 0x000C0000 */
                FIELD  DROP_TIMING                           :  1;      /* 20..20, 0x00100000 */
                FIELD  rsv_21                                :  1;      /* 21..21, 0x00200000 */
                FIELD  DMA_RING_EN                           :  1;      /* 22..22, 0x00400000 */
                FIELD  rsv_23                                :  1;      /* 23..23, 0x00800000 */
                FIELD  SUB_RATIO                             :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CTL1;   /* CAMSV_FBC_IMGO_CTL1, CAMSV_0_FBC_IMGO_CTL1*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CTL2_
{
        volatile struct /* 0x1A050114 */
        {
                FIELD  FBC_CNT                               :  7;      /*  0.. 6, 0x0000007F */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  RCNT                                  :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  WCNT                                  :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  2;      /* 22..23, 0x00C00000 */
                FIELD  DROP_CNT                              :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CTL2;   /* CAMSV_FBC_IMGO_CTL2, CAMSV_0_FBC_IMGO_CTL2*/

typedef volatile union _CAMSV_REG_FBC_IMGO_ENQ_ADDR_
{
        volatile struct /* 0x1A050118 */
        {
                FIELD  IMGO_ENQ_ADDR                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_FBC_IMGO_ENQ_ADDR;   /* CAMSV_FBC_IMGO_ENQ_ADDR, CAMSV_0_FBC_IMGO_ENQ_ADDR*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CUR_ADDR_
{
        volatile struct /* 0x1A05011C */
        {
                FIELD  IMGO_CUR_ADDR                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CUR_ADDR;   /* CAMSV_FBC_IMGO_CUR_ADDR, CAMSV_0_FBC_IMGO_CUR_ADDR*/

typedef volatile union _CAMSV_REG_TG_SEN_MODE_
{
        volatile struct /* 0x1A050230 */
        {
                FIELD  CMOS_EN                               :  1;      /*  0.. 0, 0x00000001 */
                FIELD  DBL_DATA_BUS                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SOT_MODE                              :  1;      /*  2.. 2, 0x00000004 */
                FIELD  SOT_CLR_MODE                          :  1;      /*  3.. 3, 0x00000008 */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  SOF_SRC                               :  2;      /*  8.. 9, 0x00000300 */
                FIELD  EOF_SRC                               :  2;      /* 10..11, 0x00000C00 */
                FIELD  PXL_CNT_RST_SRC                       :  1;      /* 12..12, 0x00001000 */
                FIELD  DBL_DATA_BUS1                         :  1;      /* 13..13, 0x00002000 */
                FIELD  SOF_WAIT_CQ                           :  1;      /* 14..14, 0x00004000 */
                FIELD  FIFO_FULL_CTL_EN                      :  1;      /* 15..15, 0x00008000 */
                FIELD  TIME_STP_EN                           :  1;      /* 16..16, 0x00010000 */
                FIELD  VS_SUB_EN                             :  1;      /* 17..17, 0x00020000 */
                FIELD  SOF_SUB_EN                            :  1;      /* 18..18, 0x00040000 */
                FIELD  I2C_CQ_EN                             :  1;      /* 19..19, 0x00080000 */
                FIELD  EOF_ALS_RDY_EN                        :  1;      /* 20..20, 0x00100000 */
                FIELD  CQ_SEL                                :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                : 10;      /* 22..31, 0xFFC00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SEN_MODE; /* CAMSV_TG_SEN_MODE, CAMSV_0_TG_SEN_MODE*/

typedef volatile union _CAMSV_REG_TG_VF_CON_
{
        volatile struct /* 0x1A050234 */
        {
                FIELD  VFDATA_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  SINGLE_MODE                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FR_CON                                :  3;      /*  4.. 6, 0x00000070 */
                FIELD  rsv_7                                 :  1;      /*  7.. 7, 0x00000080 */
                FIELD  SP_DELAY                              :  3;      /*  8..10, 0x00000700 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  SPDELAY_MODE                          :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_VF_CON;   /* CAMSV_TG_VF_CON, CAMSV_0_TG_VF_CON*/

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_PXL_
{
        volatile struct /* 0x1A050238 */
        {
                FIELD  PXL_S                                 : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  PXL_E                                 : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_PXL; /* CAMSV_TG_SEN_GRAB_PXL, CAMSV_0_TG_SEN_GRAB_PXL*/

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_LIN_
{
        volatile struct /* 0x1A05023C */
        {
                FIELD  LIN_S                                 : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  LIN_E                                 : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_LIN; /* CAMSV_TG_SEN_GRAB_LIN, CAMSV_0_TG_SEN_GRAB_LIN*/

typedef volatile union _CAMSV_REG_TG_PATH_CFG_
{
        volatile struct /* 0x1A050240 */
        {
                FIELD  SEN_IN_LSB                            :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  JPGINF_EN                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  MEMIN_EN                              :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  1;      /*  6.. 6, 0x00000040 */
                FIELD  JPG_LINEND_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  DB_LOAD_DIS                           :  1;      /*  8.. 8, 0x00000100 */
                FIELD  DB_LOAD_SRC                           :  1;      /*  9.. 9, 0x00000200 */
                FIELD  DB_LOAD_VSPOL                         :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  YUV_U2S_DIS                           :  1;      /* 12..12, 0x00001000 */
                FIELD  YUV_BIN_EN                            :  1;      /* 13..13, 0x00002000 */
                FIELD  TG_ERR_SEL                            :  1;      /* 14..14, 0x00004000 */
                FIELD  TG_FULL_SEL                           :  1;      /* 15..15, 0x00008000 */
                FIELD  TG_FULL_SEL2                          :  1;      /* 16..16, 0x00010000 */
                FIELD  FLUSH_DISABLE                         :  1;      /* 17..17, 0x00020000 */
                FIELD  INT_BANK_DISABLE                      :  1;      /* 18..18, 0x00040000 */
                FIELD  EXP_ESC                               :  1;      /* 19..19, 0x00080000 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_PATH_CFG; /* CAMSV_TG_PATH_CFG, CAMSV_0_TG_PATH_CFG*/

typedef volatile union _CAMSV_REG_TG_MEMIN_CTL_
{
        volatile struct /* 0x1A050244 */
        {
                FIELD  MEMIN_DUMMYPXL                        :  8;      /*  0.. 7, 0x000000FF */
                FIELD  MEMIN_DUMMYLIN                        :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_MEMIN_CTL;    /* CAMSV_TG_MEMIN_CTL, CAMSV_0_TG_MEMIN_CTL*/

typedef volatile union _CAMSV_REG_TG_INT1_
{
        volatile struct /* 0x1A050248 */
        {
                FIELD  TG_INT1_LINENO                        : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  TG_INT1_PXLNO                         : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  VSYNC_INT_POL                         :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_INT1; /* CAMSV_TG_INT1, CAMSV_0_TG_INT1*/

typedef volatile union _CAMSV_REG_TG_INT2_
{
        volatile struct /* 0x1A05024C */
        {
                FIELD  TG_INT2_LINENO                        : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  TG_INT2_PXLNO                         : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_INT2; /* CAMSV_TG_INT2, CAMSV_0_TG_INT2*/

typedef volatile union _CAMSV_REG_TG_SOF_CNT_
{
        volatile struct /* 0x1A050250 */
        {
                FIELD  SOF_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SOF_CNT;  /* CAMSV_TG_SOF_CNT, CAMSV_0_TG_SOF_CNT*/

typedef volatile union _CAMSV_REG_TG_SOT_CNT_
{
        volatile struct /* 0x1A050254 */
        {
                FIELD  SOT_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SOT_CNT;  /* CAMSV_TG_SOT_CNT, CAMSV_0_TG_SOT_CNT*/

typedef volatile union _CAMSV_REG_TG_EOT_CNT_
{
        volatile struct /* 0x1A050258 */
        {
                FIELD  EOT_CNT                               : 28;      /*  0..27, 0x0FFFFFFF */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_EOT_CNT;  /* CAMSV_TG_EOT_CNT, CAMSV_0_TG_EOT_CNT*/

typedef volatile union _CAMSV_REG_TG_ERR_CTL_
{
        volatile struct /* 0x1A05025C */
        {
                FIELD  GRAB_ERR_FLIMIT_NO                    :  4;      /*  0.. 3, 0x0000000F */
                FIELD  GRAB_ERR_FLIMIT_EN                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  GRAB_ERR_EN                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;      /*  8..11, 0x00000F00 */
                FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                :  3;      /* 13..15, 0x0000E000 */
                FIELD  DBG_SRC_SEL                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_ERR_CTL;  /* CAMSV_TG_ERR_CTL, CAMSV_0_TG_ERR_CTL*/

typedef volatile union _CAMSV_REG_TG_DAT_NO_
{
        volatile struct /* 0x1A050260 */
        {
                FIELD  DAT_NO                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_DAT_NO;   /* CAMSV_TG_DAT_NO, CAMSV_0_TG_DAT_NO*/

typedef volatile union _CAMSV_REG_TG_FRM_CNT_ST_
{
        volatile struct /* 0x1A050264 */
        {
                FIELD  REZ_OVRUN_FCNT                        :  4;      /*  0.. 3, 0x0000000F */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  GRAB_ERR_FCNT                         :  4;      /*  8..11, 0x00000F00 */
                FIELD  rsv_12                                : 20;      /* 12..31, 0xFFFFF000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FRM_CNT_ST;   /* CAMSV_TG_FRM_CNT_ST, CAMSV_0_TG_FRM_CNT_ST*/

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_
{
        volatile struct /* 0x1A050268 */
        {
                FIELD  LINE_CNT                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PXL_CNT                               : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FRMSIZE_ST;   /* CAMSV_TG_FRMSIZE_ST, CAMSV_0_TG_FRMSIZE_ST*/

typedef volatile union _CAMSV_REG_TG_INTER_ST_
{
        volatile struct /* 0x1A05026C */
        {
                FIELD  SYN_VF_DATA_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  OUT_RDY                               :  1;      /*  1.. 1, 0x00000002 */
                FIELD  OUT_REQ                               :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 :  5;      /*  3.. 7, 0x000000F8 */
                FIELD  TG_CAM_CS                             :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  CAM_FRM_CNT                           :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_INTER_ST; /* CAMSV_TG_INTER_ST, CAMSV_0_TG_INTER_ST*/

typedef volatile union _CAMSV_REG_TG_FLASHA_CTL_
{
        volatile struct /* 0x1A050270 */
        {
                FIELD  FLASHA_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FLASH_EN                              :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FLASHA_STARTPNT                       :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FLASHA_END_FRM                        :  3;      /*  8..10, 0x00000700 */
                FIELD  rsv_11                                :  1;      /* 11..11, 0x00000800 */
                FIELD  FLASH_POL                             :  1;      /* 12..12, 0x00001000 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHA_CTL;   /* CAMSV_TG_FLASHA_CTL, CAMSV_0_TG_FLASHA_CTL*/

typedef volatile union _CAMSV_REG_TG_FLASHA_LINE_CNT_
{
        volatile struct /* 0x1A050274 */
        {
                FIELD  FLASHA_LUNIT_NO                       : 20;      /*  0..19, 0x000FFFFF */
                FIELD  FLASHA_LUNIT                          :  4;      /* 20..23, 0x00F00000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHA_LINE_CNT;  /* CAMSV_TG_FLASHA_LINE_CNT, CAMSV_0_TG_FLASHA_LINE_CNT*/

typedef volatile union _CAMSV_REG_TG_FLASHA_POS_
{
        volatile struct /* 0x1A050278 */
        {
                FIELD  FLASHA_PXL                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FLASHA_LINE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHA_POS;   /* CAMSV_TG_FLASHA_POS, CAMSV_0_TG_FLASHA_POS*/

typedef volatile union _CAMSV_REG_TG_FLASHB_CTL_
{
        volatile struct /* 0x1A05027C */
        {
                FIELD  FLASHB_EN                             :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FLASHB_TRIG_SRC                       :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  FLASHB_STARTPNT                       :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  FLASHB_START_FRM                      :  4;      /*  8..11, 0x00000F00 */
                FIELD  FLASHB_CONT_FRM                       :  3;      /* 12..14, 0x00007000 */
                FIELD  rsv_15                                : 17;      /* 15..31, 0xFFFF8000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHB_CTL;   /* CAMSV_TG_FLASHB_CTL, CAMSV_0_TG_FLASHB_CTL*/

typedef volatile union _CAMSV_REG_TG_FLASHB_LINE_CNT_
{
        volatile struct /* 0x1A050280 */
        {
                FIELD  FLASHB_LUNIT_NO                       : 20;      /*  0..19, 0x000FFFFF */
                FIELD  FLASHB_LUNIT                          :  4;      /* 20..23, 0x00F00000 */
                FIELD  rsv_24                                :  8;      /* 24..31, 0xFF000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHB_LINE_CNT;  /* CAMSV_TG_FLASHB_LINE_CNT, CAMSV_0_TG_FLASHB_LINE_CNT*/

typedef volatile union _CAMSV_REG_TG_FLASHB_POS_
{
        volatile struct /* 0x1A050284 */
        {
                FIELD  FLASHB_PXL                            : 15;      /*  0..14, 0x00007FFF */
                FIELD  rsv_15                                :  1;      /* 15..15, 0x00008000 */
                FIELD  FLASHB_LINE                           : 14;      /* 16..29, 0x3FFF0000 */
                FIELD  rsv_30                                :  2;      /* 30..31, 0xC0000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS;   /* CAMSV_TG_FLASHB_POS, CAMSV_0_TG_FLASHB_POS*/

typedef volatile union _CAMSV_REG_TG_FLASHB_POS1_
{
        volatile struct /* 0x1A050288 */
        {
                FIELD  FLASHB_CYC_CNT                        : 20;      /*  0..19, 0x000FFFFF */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS1;  /* CAMSV_TG_FLASHB_POS1, CAMSV_0_TG_FLASHB_POS1*/

typedef volatile union _CAMSV_REG_TG_I2C_CQ_TRIG_
{
        volatile struct /* 0x1A050290 */
        {
                FIELD  TG_I2C_CQ_TRIG                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_I2C_CQ_TRIG;  /* CAMSV_TG_I2C_CQ_TRIG, CAMSV_0_TG_I2C_CQ_TRIG*/

typedef volatile union _CAMSV_REG_TG_CQ_TIMING_
{
        volatile struct /* 0x1A050294 */
        {
                FIELD  TG_I2C_CQ_TIM                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_CQ_TIMING;    /* CAMSV_TG_CQ_TIMING, CAMSV_0_TG_CQ_TIMING*/

typedef volatile union _CAMSV_REG_TG_CQ_NUM_
{
        volatile struct /* 0x1A050298 */
        {
                FIELD  TG_CQ_NUM                             :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_CQ_NUM;   /* CAMSV_TG_CQ_NUM, CAMSV_0_TG_CQ_NUM*/

typedef volatile union _CAMSV_REG_TG_TIME_STAMP_
{
        volatile struct /* 0x1A0502A0 */
        {
                FIELD  TG_TIME_STAMP                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_TIME_STAMP;   /* CAMSV_TG_TIME_STAMP, CAMSV_0_TG_TIME_STAMP*/

typedef volatile union _CAMSV_REG_TG_SUB_PERIOD_
{
        volatile struct /* 0x1A0502A4 */
        {
                FIELD  VS_PERIOD                             :  5;      /*  0.. 4, 0x0000001F */
                FIELD  rsv_5                                 :  3;      /*  5.. 7, 0x000000E0 */
                FIELD  SOF_PERIOD                            :  5;      /*  8..12, 0x00001F00 */
                FIELD  rsv_13                                : 19;      /* 13..31, 0xFFFFE000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_SUB_PERIOD;   /* CAMSV_TG_SUB_PERIOD, CAMSV_0_TG_SUB_PERIOD*/

typedef volatile union _CAMSV_REG_TG_DAT_NO_R_
{
        volatile struct /* 0x1A0502A8 */
        {
                FIELD  DAT_NO                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_DAT_NO_R; /* CAMSV_TG_DAT_NO_R, CAMSV_0_TG_DAT_NO_R*/

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_R_
{
        volatile struct /* 0x1A0502AC */
        {
                FIELD  LINE_CNT                              : 14;      /*  0..13, 0x00003FFF */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  PXL_CNT                               : 15;      /* 16..30, 0x7FFF0000 */
                FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_FRMSIZE_ST_R; /* CAMSV_TG_FRMSIZE_ST_R, CAMSV_0_TG_FRMSIZE_ST_R*/

typedef volatile union _CAMSV_REG_TG_TIME_STAMP_CTL_
{
        volatile struct /* 0x1A0502B0 */
        {
                FIELD  TG_TIME_STAMP_SEL                     :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  3;      /*  1.. 3, 0x0000000E */
                FIELD  TG_TIME_STAMP_LOCK                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_TIME_STAMP_CTL;   /* CAMSV_TG_TIME_STAMP_CTL, CAMSV_0_TG_TIME_STAMP_CTL*/

typedef volatile union _CAMSV_REG_TG_TIME_STAMP_MSB_
{
        volatile struct /* 0x1A0502B4 */
        {
                FIELD  TG_TIME_STAMP_MSB                     : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TG_TIME_STAMP_MSB;   /* CAMSV_TG_TIME_STAMP_MSB, CAMSV_0_TG_TIME_STAMP_MSB*/

typedef volatile union _CAMSV_REG_DMA_ERR_CTRL_
{
        volatile struct /* 0x1A050350 */
        {
                FIELD  IMGO_A_ERR                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  RRZO_A_ERR                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  AAO_A_ERR                             :  1;      /*  2.. 2, 0x00000004 */
                FIELD  AFO_A_ERR                             :  1;      /*  3.. 3, 0x00000008 */
                FIELD  LCSO_A_ERR                            :  1;      /*  4.. 4, 0x00000010 */
                FIELD  UFEO_A_ERR                            :  1;      /*  5.. 5, 0x00000020 */
                FIELD  PDO_A_ERR                             :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 :  9;      /*  7..15, 0x0000FF80 */
                FIELD  BPCI_A_ERR                            :  1;      /* 16..16, 0x00010000 */
                FIELD  CACI_A_ERR                            :  1;      /* 17..17, 0x00020000 */
                FIELD  LSCI_A_ERR                            :  1;      /* 18..18, 0x00040000 */
                FIELD  rsv_19                                : 12;      /* 19..30, 0x7FF80000 */
                FIELD  ERR_CLR_MD                            :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_ERR_CTRL;    /* CAMSV_DMA_ERR_CTRL, CAMSV_0_DMA_ERR_CTRL*/

typedef volatile union _CAMSV_REG_IMGO_ERR_STAT_
{
        volatile struct /* 0x1A050360 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_ERR_STAT;   /* CAMSV_IMGO_ERR_STAT, CAMSV_0_IMGO_ERR_STAT*/

typedef volatile union _CAMSV_REG_DMA_DEBUG_ADDR_
{
        volatile struct /* 0x1A0503AC */
        {
                FIELD  DEBUG_ADDR                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_DEBUG_ADDR;  /* CAMSV_DMA_DEBUG_ADDR, CAMSV_0_DMA_DEBUG_ADDR*/

typedef volatile union _CAMSV_REG_DMA_RSV1_
{
        volatile struct /* 0x1A0503B0 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV1;    /* CAMSV_DMA_RSV1, CAMSV_0_DMA_RSV1*/

typedef volatile union _CAMSV_REG_DMA_RSV2_
{
        volatile struct /* 0x1A0503B4 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV2;    /* CAMSV_DMA_RSV2, CAMSV_0_DMA_RSV2*/

typedef volatile union _CAMSV_REG_DMA_RSV3_
{
        volatile struct /* 0x1A0503B8 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV3;    /* CAMSV_DMA_RSV3, CAMSV_0_DMA_RSV3*/

typedef volatile union _CAMSV_REG_DMA_RSV4_
{
        volatile struct /* 0x1A0503BC */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV4;    /* CAMSV_DMA_RSV4, CAMSV_0_DMA_RSV4*/

typedef volatile union _CAMSV_REG_DMA_RSV5_
{
        volatile struct /* 0x1A0503C0 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV5;    /* CAMSV_DMA_RSV5, CAMSV_0_DMA_RSV5*/

typedef volatile union _CAMSV_REG_DMA_RSV6_
{
        volatile struct /* 0x1A0503C4 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_RSV6;    /* CAMSV_DMA_RSV6, CAMSV_0_DMA_RSV6*/

typedef volatile union _CAMSV_REG_DMA_DEBUG_SEL_
{
        volatile struct /* 0x1A0503C8 */
        {
                FIELD  DMA_TOP_SEL                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  R_W_DMA_TOP_SEL                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SUB_MODULE_SEL                        :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  4;      /* 24..27, 0x0F000000 */
                FIELD  PDO_FIFO_FULL_XSIZE                   :  1;      /* 28..28, 0x10000000 */
                FIELD  IMGO_UFE_FIFO_FULL_XSIZE              :  1;      /* 29..29, 0x20000000 */
                FIELD  ARBITER_BVALID_FULL                   :  1;      /* 30..30, 0x40000000 */
                FIELD  ARBITER_COM_FULL                      :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_DEBUG_SEL;   /* CAMSV_DMA_DEBUG_SEL, CAMSV_0_DMA_DEBUG_SEL*/

typedef volatile union _CAMSV_REG_DMA_BW_SELF_TEST_
{
        volatile struct /* 0x1A0503CC */
        {
                FIELD  BW_SELF_TEST_EN_IMGO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  BW_SELF_TEST_EN_RRZO                  :  1;      /*  1.. 1, 0x00000002 */
                FIELD  BW_SELF_TEST_EN_AAO                   :  1;      /*  2.. 2, 0x00000004 */
                FIELD  BW_SELF_TEST_EN_AFO                   :  1;      /*  3.. 3, 0x00000008 */
                FIELD  BW_SELF_TEST_EN_LCSO                  :  1;      /*  4.. 4, 0x00000010 */
                FIELD  BW_SELF_TEST_EN_UFEO                  :  1;      /*  5.. 5, 0x00000020 */
                FIELD  BW_SELF_TEST_EN_PDO                   :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_BW_SELF_TEST;    /* CAMSV_DMA_BW_SELF_TEST, CAMSV_0_DMA_BW_SELF_TEST*/

typedef volatile union _CAMSV_REG_DMA_FRAME_HEADER_EN_
{
        volatile struct /* 0x1A050400 */
        {
                FIELD  FRAME_HEADER_EN_IMGO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  FRAME_HEADER_EN_RRZO                  :  1;      /*  1.. 1, 0x00000002 */
                FIELD  FRAME_HEADER_EN_AAO                   :  1;      /*  2.. 2, 0x00000004 */
                FIELD  FRAME_HEADER_EN_AFO                   :  1;      /*  3.. 3, 0x00000008 */
                FIELD  FRAME_HEADER_EN_LCSO                  :  1;      /*  4.. 4, 0x00000010 */
                FIELD  FRAME_HEADER_EN_UFEO                  :  1;      /*  5.. 5, 0x00000020 */
                FIELD  FRAME_HEADER_EN_PDO                   :  1;      /*  6.. 6, 0x00000040 */
                FIELD  rsv_7                                 : 25;      /*  7..31, 0xFFFFFF80 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DMA_FRAME_HEADER_EN; /* CAMSV_DMA_FRAME_HEADER_EN, CAMSV_0_DMA_FRAME_HEADER_EN*/

typedef volatile union _CAMSV_REG_IMGO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A050404 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_BASE_ADDR;   /* CAMSV_IMGO_FH_BASE_ADDR, CAMSV_0_IMGO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_RRZO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A050408 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_RRZO_FH_BASE_ADDR;   /* CAMSV_RRZO_FH_BASE_ADDR, CAMSV_0_RRZO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_AAO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A05040C */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_AAO_FH_BASE_ADDR;    /* CAMSV_AAO_FH_BASE_ADDR, CAMSV_0_AAO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_AFO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A050410 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_AFO_FH_BASE_ADDR;    /* CAMSV_AFO_FH_BASE_ADDR, CAMSV_0_AFO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_LCSO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A050414 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_LCSO_FH_BASE_ADDR;   /* CAMSV_LCSO_FH_BASE_ADDR, CAMSV_0_LCSO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_UFEO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A050418 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_UFEO_FH_BASE_ADDR;   /* CAMSV_UFEO_FH_BASE_ADDR, CAMSV_0_UFEO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_PDO_FH_BASE_ADDR_
{
        volatile struct /* 0x1A05041C */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_PDO_FH_BASE_ADDR;    /* CAMSV_PDO_FH_BASE_ADDR, CAMSV_0_PDO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_2_
{
        volatile struct /* 0x1A050434 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_2; /* CAMSV_IMGO_FH_SPARE_2, CAMSV_0_IMGO_FH_SPARE_2*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_3_
{
        volatile struct /* 0x1A050438 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_3; /* CAMSV_IMGO_FH_SPARE_3, CAMSV_0_IMGO_FH_SPARE_3*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_4_
{
        volatile struct /* 0x1A05043C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_4; /* CAMSV_IMGO_FH_SPARE_4, CAMSV_0_IMGO_FH_SPARE_4*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_5_
{
        volatile struct /* 0x1A050440 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_5; /* CAMSV_IMGO_FH_SPARE_5, CAMSV_0_IMGO_FH_SPARE_5*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_6_
{
        volatile struct /* 0x1A050444 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_6; /* CAMSV_IMGO_FH_SPARE_6, CAMSV_0_IMGO_FH_SPARE_6*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_7_
{
        volatile struct /* 0x1A050448 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_7; /* CAMSV_IMGO_FH_SPARE_7, CAMSV_0_IMGO_FH_SPARE_7*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_8_
{
        volatile struct /* 0x1A05044C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_8; /* CAMSV_IMGO_FH_SPARE_8, CAMSV_0_IMGO_FH_SPARE_8*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_9_
{
        volatile struct /* 0x1A050450 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_9; /* CAMSV_IMGO_FH_SPARE_9, CAMSV_0_IMGO_FH_SPARE_9*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_10_
{
        volatile struct /* 0x1A050454 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_10;    /* CAMSV_IMGO_FH_SPARE_10, CAMSV_0_IMGO_FH_SPARE_10*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_11_
{
        volatile struct /* 0x1A050458 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_11;    /* CAMSV_IMGO_FH_SPARE_11, CAMSV_0_IMGO_FH_SPARE_11*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_12_
{
        volatile struct /* 0x1A05045C */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_12;    /* CAMSV_IMGO_FH_SPARE_12, CAMSV_0_IMGO_FH_SPARE_12*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_13_
{
        volatile struct /* 0x1A050460 */
        {
                FIELD  SPARE_REG                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_13;    /* CAMSV_IMGO_FH_SPARE_13, CAMSV_0_IMGO_FH_SPARE_13*/

typedef volatile union _CAMSV_REG_TOP_DEBUG_
{
        volatile struct /* 0x1A050500 */
        {
                FIELD  DEBUG                                 : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_TOP_DEBUG;   /* CAMSV_TOP_DEBUG, CAMSV_0_TOP_DEBUG*/

typedef volatile union _CAMSV_REG_MODULE_EN_
{
        volatile struct /* 0x1A050510 */
        {
                FIELD  TG_EN                                 :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PAK_EN                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  PAK_SEL                               :  1;      /*  3.. 3, 0x00000008 */
                FIELD  IMGO_EN                               :  1;      /*  4.. 4, 0x00000010 */
                FIELD  DOWN_SAMPLE_EN                        :  1;      /*  5.. 5, 0x00000020 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  DOWN_SAMPLE_PERIOD                    :  6;      /*  8..13, 0x00003F00 */
                FIELD  rsv_14                                :  2;      /* 14..15, 0x0000C000 */
                FIELD  SW_PASS1_DONE_FRAME_CNT               :  6;      /* 16..21, 0x003F0000 */
                FIELD  rsv_22                                :  8;      /* 22..29, 0x3FC00000 */
                FIELD  DB_EN                                 :  1;      /* 30..30, 0x40000000 */
                FIELD  DB_LOCK                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_MODULE_EN;   /* CAMSV_MODULE_EN, CAMSV_0_MODULE_EN*/

typedef volatile union _CAMSV_REG_FMT_SEL_
{
        volatile struct /* 0x1A050514 */
        {
                FIELD  TG1_FMT                               :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG1_SW                                :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 :  2;      /*  6.. 7, 0x000000C0 */
                FIELD  LP_MODE                               :  1;      /*  8.. 8, 0x00000100 */
                FIELD  HLR_MODE                              :  1;      /*  9.. 9, 0x00000200 */
                FIELD  rsv_10                                : 22;      /* 10..31, 0xFFFFFC00 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_FMT_SEL; /* CAMSV_FMT_SEL, CAMSV_0_FMT_SEL*/

typedef volatile union _CAMSV_REG_INT_EN_
{
        volatile struct /* 0x1A050518 */
        {
                FIELD  VS1_INT_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_EN                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_EN                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON1_INT_EN                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_INT_EN                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_INT_EN                       :  1;      /*  5.. 5, 0x00000020 */
                FIELD  TG_DROP_INT_EN                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  TG_SOF_INT_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  rsv_8                                 :  2;      /*  8.. 9, 0x00000300 */
                FIELD  PASS1_DON_INT_EN                      :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  IMGO_ERR_INT_EN                       :  1;      /* 16..16, 0x00010000 */
                FIELD  IMGO_OVERR_INT_EN                     :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
                FIELD  IMGO_DROP_INT_EN                      :  1;      /* 19..19, 0x00080000 */
                FIELD  SW_PASS1_DON_INT_EN                   :  1;      /* 20..20, 0x00100000 */
                FIELD  TG_SOF_WAIT_INT_EN                    :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                :  9;      /* 22..30, 0x7FC00000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_INT_EN;  /* CAMSV_INT_EN, CAMSV_0_INT_EN*/

typedef volatile union _CAMSV_REG_INT_STATUS_
{
        volatile struct /* 0x1A05051C */
        {
                FIELD  VS1_ST                                :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_ST1                                :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_ST2                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON1_ST                            :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_ST                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_ST                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  TG_DROP_INT_ST                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  TG_SOF1_INT_ST                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  rsv_8                                 :  2;      /*  8.. 9, 0x00000300 */
                FIELD  PASS1_DON_ST                          :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  IMGO_ERR_ST                           :  1;      /* 16..16, 0x00010000 */
                FIELD  IMGO_OVERR_ST                         :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
                FIELD  IMGO_DROP_ST                          :  1;      /* 19..19, 0x00080000 */
                FIELD  SW_PASS1_DON_ST                       :  1;      /* 20..20, 0x00100000 */
                FIELD  TG_SOF_WAIT_ST                        :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                : 10;      /* 22..31, 0xFFC00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_INT_STATUS;  /* CAMSV_INT_STATUS, CAMSV_0_INT_STATUS*/

typedef volatile union _CAMSV_REG_SW_CTL_
{
        volatile struct /* 0x1A050520 */
        {
                FIELD  IMGO_RST_TRIG                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  IMGO_RST_ST                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  SW_RST                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 : 29;      /*  3..31, 0xFFFFFFF8 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_SW_CTL;  /* CAMSV_SW_CTL, CAMSV_0_SW_CTL*/

typedef volatile union _CAMSV_REG_SPARE0_
{
        volatile struct /* 0x1A050524 */
        {
                FIELD  SPARE0                                : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_SPARE0;  /* CAMSV_SPARE0, CAMSV_0_SPARE0*/

typedef volatile union _CAMSV_REG_SPARE1_
{
        volatile struct /* 0x1A050528 */
        {
                FIELD  CTL_SPARE1                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_SPARE1;  /* CAMSV_SPARE1, CAMSV_0_SPARE1*/

typedef volatile union _CAMSV_REG_IMGO_FBC_
{
        volatile struct /* 0x1A05052C */
        {
                FIELD  RCNT_INC                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_IMGO_FBC;    /* CAMSV_IMGO_FBC, CAMSV_0_IMGO_FBC*/

typedef volatile union _CAMSV_REG_CLK_EN_
{
        volatile struct /* 0x1A050530 */
        {
                FIELD  TG_DP_CK_EN                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 :  1;      /*  1.. 1, 0x00000002 */
                FIELD  PAK_DP_CK_EN                          :  1;      /*  2.. 2, 0x00000004 */
                FIELD  rsv_3                                 : 12;      /*  3..14, 0x00007FF8 */
                FIELD  DMA_DP_CK_EN                          :  1;      /* 15..15, 0x00008000 */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_CLK_EN;  /* CAMSV_CLK_EN, CAMSV_0_CLK_EN*/

typedef volatile union _CAMSV_REG_DBG_SET_
{
        volatile struct /* 0x1A050534 */
        {
                FIELD  DEBUG_MOD_SEL                         :  4;      /*  0.. 3, 0x0000000F */
                FIELD  rsv_4                                 :  4;      /*  4.. 7, 0x000000F0 */
                FIELD  DEBUG_SEL                             : 12;      /*  8..19, 0x000FFF00 */
                FIELD  rsv_20                                : 12;      /* 20..31, 0xFFF00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DBG_SET; /* CAMSV_DBG_SET, CAMSV_0_DBG_SET*/

typedef volatile union _CAMSV_REG_DBG_PORT_
{
        volatile struct /* 0x1A050538 */
        {
                FIELD  CTL_DBG_PORT                          : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DBG_PORT;    /* CAMSV_DBG_PORT, CAMSV_0_DBG_PORT*/

typedef volatile union _CAMSV_REG_DATE_CODE_
{
        volatile struct /* 0x1A05053C */
        {
                FIELD  CTL_DATE_CODE                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DATE_CODE;   /* CAMSV_DATE_CODE, CAMSV_0_DATE_CODE*/

typedef volatile union _CAMSV_REG_PROJ_CODE_
{
        volatile struct /* 0x1A050540 */
        {
                FIELD  CTL_PROJ_CODE                         : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_PROJ_CODE;   /* CAMSV_PROJ_CODE, CAMSV_0_PROJ_CODE*/

typedef volatile union _CAMSV_REG_DCM_DIS_
{
        volatile struct /* 0x1A050544 */
        {
                FIELD  PAK_DCM_DIS                           :  1;      /*  0.. 0, 0x00000001 */
                FIELD  IMGO_DCM_DIS                          :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DCM_DIS; /* CAMSV_DCM_DIS, CAMSV_0_DCM_DIS*/

typedef volatile union _CAMSV_REG_DCM_STATUS_
{
        volatile struct /* 0x1A050548 */
        {
                FIELD  PAK_DCM_ST                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  IMGO_DCM_ST                           :  1;      /*  1.. 1, 0x00000002 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_DCM_STATUS;  /* CAMSV_DCM_STATUS, CAMSV_0_DCM_STATUS*/

typedef volatile union _CAMSV_REG_PAK_
{
        volatile struct /* 0x1A05054C */
        {
                FIELD  PAK_MODE                              :  3;      /*  0.. 2, 0x00000007 */
                FIELD  rsv_3                                 :  1;      /*  3.. 3, 0x00000008 */
                FIELD  PAK_DBL_MODE                          :  2;      /*  4.. 5, 0x00000030 */
                FIELD  rsv_6                                 : 26;      /*  6..31, 0xFFFFFFC0 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_PAK; /* CAMSV_PAK, CAMSV_0_PAK*/

typedef volatile union _CAMSV_REG_CCU_INT_EN_
{
        volatile struct /* 0x1A050550 */
        {
                FIELD  VS1_INT_EN                            :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_INT1_EN                            :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_INT2_EN                            :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON1_INT_EN                        :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_INT_EN                         :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_INT_EN                       :  1;      /*  5.. 5, 0x00000020 */
                FIELD  TG_DROP_INT_EN                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  TG_SOF_INT_EN                         :  1;      /*  7.. 7, 0x00000080 */
                FIELD  rsv_8                                 :  2;      /*  8.. 9, 0x00000300 */
                FIELD  PASS1_DON_INT_EN                      :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  IMGO_ERR_INT_EN                       :  1;      /* 16..16, 0x00010000 */
                FIELD  IMGO_OVERR_INT_EN                     :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
                FIELD  IMGO_DROP_INT_EN                      :  1;      /* 19..19, 0x00080000 */
                FIELD  SW_PASS1_DON_INT_EN                   :  1;      /* 20..20, 0x00100000 */
                FIELD  TG_SOF_WAIT_INT_EN                    :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                :  9;      /* 22..30, 0x7FC00000 */
                FIELD  INT_WCLR_EN                           :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_CCU_INT_EN;  /* CAMSV_CCU_INT_EN, CAMSV_0_CCU_INT_EN*/

typedef volatile union _CAMSV_REG_CCU_INT_STATUS_
{
        volatile struct /* 0x1A050554 */
        {
                FIELD  VS1_ST                                :  1;      /*  0.. 0, 0x00000001 */
                FIELD  TG_ST1                                :  1;      /*  1.. 1, 0x00000002 */
                FIELD  TG_ST2                                :  1;      /*  2.. 2, 0x00000004 */
                FIELD  EXPDON1_ST                            :  1;      /*  3.. 3, 0x00000008 */
                FIELD  TG_ERR_ST                             :  1;      /*  4.. 4, 0x00000010 */
                FIELD  TG_GBERR_ST                           :  1;      /*  5.. 5, 0x00000020 */
                FIELD  TG_DROP_INT_ST                        :  1;      /*  6.. 6, 0x00000040 */
                FIELD  TG_SOF1_INT_ST                        :  1;      /*  7.. 7, 0x00000080 */
                FIELD  rsv_8                                 :  2;      /*  8.. 9, 0x00000300 */
                FIELD  PASS1_DON_ST                          :  1;      /* 10..10, 0x00000400 */
                FIELD  rsv_11                                :  5;      /* 11..15, 0x0000F800 */
                FIELD  IMGO_ERR_ST                           :  1;      /* 16..16, 0x00010000 */
                FIELD  IMGO_OVERR_ST                         :  1;      /* 17..17, 0x00020000 */
                FIELD  rsv_18                                :  1;      /* 18..18, 0x00040000 */
                FIELD  IMGO_DROP_ST                          :  1;      /* 19..19, 0x00080000 */
                FIELD  SW_PASS1_DON_ST                       :  1;      /* 20..20, 0x00100000 */
                FIELD  TG_SOF_WAIT_ST                        :  1;      /* 21..21, 0x00200000 */
                FIELD  rsv_22                                : 10;      /* 22..31, 0xFFC00000 */
        } Bits;
        UINT32 Raw;
}CAMSV_REG_CCU_INT_STATUS;  /* CAMSV_CCU_INT_STATUS, CAMSV_0_CCU_INT_STATUS*/

typedef volatile struct _cam_uni_reg_t_ /* 0x1A003000..0x1A003FFF */
{
    CAM_UNI_REG_TOP_CTL                             CAM_UNI_TOP_CTL;                                 /* 0000, 0x1A003000 */
    CAM_UNI_REG_TOP_MISC                            CAM_UNI_TOP_MISC;                                /* 0004, 0x1A003004 */
    CAM_UNI_REG_TOP_SW_CTL                          CAM_UNI_TOP_SW_CTL;                              /* 0008, 0x1A003008 */
    CAM_UNI_REG_TOP_RAWI_TRIG                       CAM_UNI_TOP_RAWI_TRIG;                           /* 000C, 0x1A00300C */
    CAM_UNI_REG_TOP_MOD_EN                          CAM_UNI_TOP_MOD_EN;                              /* 0010, 0x1A003010 */
    CAM_UNI_REG_TOP_DMA_EN                          CAM_UNI_TOP_DMA_EN;                              /* 0014, 0x1A003014 */
    UINT32                                          rsv_0018;                                        /* 0018, 0x1A003018 */
    CAM_UNI_REG_TOP_FMT_SEL                         CAM_UNI_TOP_FMT_SEL;                             /* 001C, 0x1A00301C */
    CAM_UNI_REG_TOP_DMA_INT_EN                      CAM_UNI_TOP_DMA_INT_EN;                          /* 0020, 0x1A003020 */
    CAM_UNI_REG_TOP_DMA_INT_STATUS                  CAM_UNI_TOP_DMA_INT_STATUS;                      /* 0024, 0x1A003024 */
    CAM_UNI_REG_TOP_DMA_INT_STATUSX                 CAM_UNI_TOP_DMA_INT_STATUSX;                     /* 0028, 0x1A003028 */
    CAM_UNI_REG_TOP_DBG_SET                         CAM_UNI_TOP_DBG_SET;                             /* 002C, 0x1A00302C */
    CAM_UNI_REG_TOP_DBG_PORT                        CAM_UNI_TOP_DBG_PORT;                            /* 0030, 0x1A003030 */
    CAM_UNI_REG_TOP_DMA_CCU_INT_EN                  CAM_UNI_TOP_DMA_CCU_INT_EN;                      /* 0034, 0x1A003034 */
    CAM_UNI_REG_TOP_DMA_CCU_INT_STATUS              CAM_UNI_TOP_DMA_CCU_INT_STATUS;                  /* 0038, 0x1A003038 */
    UINT32                                          rsv_003C;                                        /* 003C, 0x1A00303C */
    CAM_UNI_REG_TOP_MOD_DCM_DIS                     CAM_UNI_TOP_MOD_DCM_DIS;                         /* 0040, 0x1A003040 */
    CAM_UNI_REG_TOP_DMA_DCM_DIS                     CAM_UNI_TOP_DMA_DCM_DIS;                         /* 0044, 0x1A003044 */
    UINT32                                          rsv_0048[2];                                     /* 0048..004F, 0x1A003048..1A00304F */
    CAM_UNI_REG_TOP_MOD_DCM_STATUS                  CAM_UNI_TOP_MOD_DCM_STATUS;                      /* 0050, 0x1A003050 */
    CAM_UNI_REG_TOP_DMA_DCM_STATUS                  CAM_UNI_TOP_DMA_DCM_STATUS;                      /* 0054, 0x1A003054 */
    UINT32                                          rsv_0058[2];                                     /* 0058..005F, 0x1A003058..1A00305F */
    CAM_UNI_REG_TOP_MOD_REQ_STATUS                  CAM_UNI_TOP_MOD_REQ_STATUS;                      /* 0060, 0x1A003060 */
    CAM_UNI_REG_TOP_DMA_REQ_STATUS                  CAM_UNI_TOP_DMA_REQ_STATUS;                      /* 0064, 0x1A003064 */
    UINT32                                          rsv_0068[2];                                     /* 0068..006F, 0x1A003068..1A00306F */
    CAM_UNI_REG_TOP_MOD_RDY_STATUS                  CAM_UNI_TOP_MOD_RDY_STATUS;                      /* 0070, 0x1A003070 */
    CAM_UNI_REG_TOP_DMA_RDY_STATUS                  CAM_UNI_TOP_DMA_RDY_STATUS;                      /* 0074, 0x1A003074 */
    UINT32                                          rsv_0078[34];                                    /* 0078..00FF, 0x1A003078..1A0030FF */
    CAM_UNI_REG_DMA_SOFT_RSTSTAT                    CAM_UNI_DMA_SOFT_RSTSTAT;                        /* 0100, 0x1A003100 */
    CAM_UNI_REG_VERTICAL_FLIP_EN                    CAM_UNI_VERTICAL_FLIP_EN;                        /* 0104, 0x1A003104 */
    CAM_UNI_REG_DMA_SOFT_RESET                      CAM_UNI_DMA_SOFT_RESET;                          /* 0108, 0x1A003108 */
    CAM_UNI_REG_LAST_ULTRA_EN                       CAM_UNI_LAST_ULTRA_EN;                           /* 010C, 0x1A00310C */
    CAM_UNI_REG_SPECIAL_FUN_EN                      CAM_UNI_SPECIAL_FUN_EN;                          /* 0110, 0x1A003110 */
    CAM_UNI_REG_SPECIAL_FUN2_EN                     CAM_UNI_SPECIAL_FUN2_EN;                         /* 0114, 0x1A003114 */
    UINT32                                          rsv_0118[2];                                     /* 0118..011F, 0x1A003118..1A00311F */
    CAM_UNI_REG_RAWI_BASE_ADDR                      CAM_UNI_RAWI_BASE_ADDR;                          /* 0120, 0x1A003120 */
    UINT32                                          rsv_0124;                                        /* 0124, 0x1A003124 */
    CAM_UNI_REG_RAWI_OFST_ADDR                      CAM_UNI_RAWI_OFST_ADDR;                          /* 0128, 0x1A003128 */
    CAM_UNI_REG_RAWI_DRS                            CAM_UNI_RAWI_DRS;                                /* 012C, 0x1A00312C */
    CAM_UNI_REG_RAWI_XSIZE                          CAM_UNI_RAWI_XSIZE;                              /* 0130, 0x1A003130 */
    CAM_UNI_REG_RAWI_YSIZE                          CAM_UNI_RAWI_YSIZE;                              /* 0134, 0x1A003134 */
    CAM_UNI_REG_RAWI_STRIDE                         CAM_UNI_RAWI_STRIDE;                             /* 0138, 0x1A003138 */
    CAM_UNI_REG_RAWI_CON                            CAM_UNI_RAWI_CON;                                /* 013C, 0x1A00313C */
    CAM_UNI_REG_RAWI_CON2                           CAM_UNI_RAWI_CON2;                               /* 0140, 0x1A003140 */
    CAM_UNI_REG_RAWI_CON3                           CAM_UNI_RAWI_CON3;                               /* 0144, 0x1A003144 */
    UINT32                                          rsv_0148;                                        /* 0148, 0x1A003148 */
    CAM_UNI_REG_RAWI_CON4                           CAM_UNI_RAWI_CON4;                               /* 014C, 0x1A00314C */
    CAM_UNI_REG_DMA_ERR_CTRL                        CAM_UNI_DMA_ERR_CTRL;                            /* 0150, 0x1A003150 */
    CAM_UNI_REG_RAWI_ERR_STAT                       CAM_UNI_RAWI_ERR_STAT;                           /* 0154, 0x1A003154 */
    CAM_UNI_REG_DMA_DEBUG_ADDR                      CAM_UNI_DMA_DEBUG_ADDR;                          /* 0158, 0x1A003158 */
    CAM_UNI_REG_DMA_RSV1                            CAM_UNI_DMA_RSV1;                                /* 015C, 0x1A00315C */
    CAM_UNI_REG_DMA_RSV2                            CAM_UNI_DMA_RSV2;                                /* 0160, 0x1A003160 */
    CAM_UNI_REG_DMA_RSV3                            CAM_UNI_DMA_RSV3;                                /* 0164, 0x1A003164 */
    CAM_UNI_REG_DMA_RSV4                            CAM_UNI_DMA_RSV4;                                /* 0168, 0x1A003168 */
    CAM_UNI_REG_DMA_RSV5                            CAM_UNI_DMA_RSV5;                                /* 016C, 0x1A00316C */
    CAM_UNI_REG_DMA_RSV6                            CAM_UNI_DMA_RSV6;                                /* 0170, 0x1A003170 */
    CAM_UNI_REG_DMA_DEBUG_SEL                       CAM_UNI_DMA_DEBUG_SEL;                           /* 0174, 0x1A003174 */
    UINT32                                          rsv_0178[34];                                    /* 0178..01FF, 0x1A003178..1A0031FF */
    CAM_UNI_REG_UNP2_A_OFST                         CAM_UNI_UNP2_A_OFST;                             /* 0200, 0x1A003200 */
    CAM_UNI_REG_TOP_CAM_XMX_SET                     CAM_UNI_TOP_CAM_XMX_SET;                         /* 0204, 0x1A013000 */
    CAM_UNI_REG_TOP_CAM_XMX_CLR                     CAM_UNI_TOP_CAM_XMX_CLR;                         /* 0208, 0x1A033000 */
    UINT32                                          rsv_0204[637];                                   /* 020C..0BFF, 0x1A003204..1A003BFF */
    CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT          CAM_UNI_ADL_DMA_A_DMA_SOFT_RSTSTAT;              /* 0C00, 0x1A003C00 */
    CAM_UNI_REG_ADL_DMA_A_VERTICAL_FLIP_EN          CAM_UNI_ADL_DMA_A_VERTICAL_FLIP_EN;              /* 0C04, 0x1A003C04 */
    CAM_UNI_REG_ADL_DMA_A_DMA_SOFT_RESET            CAM_UNI_ADL_DMA_A_DMA_SOFT_RESET;                /* 0C08, 0x1A003C08 */
    CAM_UNI_REG_ADL_DMA_A_LAST_ULTRA_EN             CAM_UNI_ADL_DMA_A_LAST_ULTRA_EN;                 /* 0C0C, 0x1A003C0C */
    CAM_UNI_REG_ADL_DMA_A_SPECIAL_FUN_EN            CAM_UNI_ADL_DMA_A_SPECIAL_FUN_EN;                /* 0C10, 0x1A003C10 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_RING                 CAM_UNI_ADL_DMA_A_IPUO_RING;                     /* 0C14, 0x1A003C14 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_RING                 CAM_UNI_ADL_DMA_A_IPUI_RING;                     /* 0C18, 0x1A003C18 */
    UINT32                                          rsv_0C1C[5];                                     /* 0C1C..0C2F, 0x1A003C1C..1A003C2F */
    CAM_UNI_REG_ADL_DMA_A_IPUO_BASE_ADDR            CAM_UNI_ADL_DMA_A_IPUO_BASE_ADDR;                /* 0C30, 0x1A003C30 */
    UINT32                                          rsv_0C34;                                        /* 0C34, 0x1A003C34 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_OFST_ADDR            CAM_UNI_ADL_DMA_A_IPUO_OFST_ADDR;                /* 0C38, 0x1A003C38 */
    UINT32                                          rsv_0C3C;                                        /* 0C3C, 0x1A003C3C */
    CAM_UNI_REG_ADL_DMA_A_IPUO_XSIZE                CAM_UNI_ADL_DMA_A_IPUO_XSIZE;                    /* 0C40, 0x1A003C40 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_YSIZE                CAM_UNI_ADL_DMA_A_IPUO_YSIZE;                    /* 0C44, 0x1A003C44 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_STRIDE               CAM_UNI_ADL_DMA_A_IPUO_STRIDE;                   /* 0C48, 0x1A003C48 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_CON                  CAM_UNI_ADL_DMA_A_IPUO_CON;                      /* 0C4C, 0x1A003C4C */
    CAM_UNI_REG_ADL_DMA_A_IPUO_CON2                 CAM_UNI_ADL_DMA_A_IPUO_CON2;                     /* 0C50, 0x1A003C50 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_CON3                 CAM_UNI_ADL_DMA_A_IPUO_CON3;                     /* 0C54, 0x1A003C54 */
    UINT32                                          rsv_0C58[14];                                    /* 0C58..0C8F, 0x1A003C58..1A003C8F */
    CAM_UNI_REG_ADL_DMA_A_IPUI_BASE_ADDR            CAM_UNI_ADL_DMA_A_IPUI_BASE_ADDR;                /* 0C90, 0x1A003C90 */
    UINT32                                          rsv_0C94;                                        /* 0C94, 0x1A003C94 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_OFST_ADDR            CAM_UNI_ADL_DMA_A_IPUI_OFST_ADDR;                /* 0C98, 0x1A003C98 */
    UINT32                                          rsv_0C9C;                                        /* 0C9C, 0x1A003C9C */
    CAM_UNI_REG_ADL_DMA_A_IPUI_XSIZE                CAM_UNI_ADL_DMA_A_IPUI_XSIZE;                    /* 0CA0, 0x1A003CA0 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_YSIZE                CAM_UNI_ADL_DMA_A_IPUI_YSIZE;                    /* 0CA4, 0x1A003CA4 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_STRIDE               CAM_UNI_ADL_DMA_A_IPUI_STRIDE;                   /* 0CA8, 0x1A003CA8 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_CON                  CAM_UNI_ADL_DMA_A_IPUI_CON;                      /* 0CAC, 0x1A003CAC */
    CAM_UNI_REG_ADL_DMA_A_IPUI_CON2                 CAM_UNI_ADL_DMA_A_IPUI_CON2;                     /* 0CB0, 0x1A003CB0 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_CON3                 CAM_UNI_ADL_DMA_A_IPUI_CON3;                     /* 0CB4, 0x1A003CB4 */
    UINT32                                          rsv_0CB8[18];                                    /* 0CB8..0CFF, 0x1A003CB8..1A003CFF */
    CAM_UNI_REG_ADL_DMA_A_DMA_ERR_CTRL              CAM_UNI_ADL_DMA_A_DMA_ERR_CTRL;                  /* 0D00, 0x1A003D00 */
    CAM_UNI_REG_ADL_DMA_A_IPUO_ERR_STAT             CAM_UNI_ADL_DMA_A_IPUO_ERR_STAT;                 /* 0D04, 0x1A003D04 */
    CAM_UNI_REG_ADL_DMA_A_IPUI_ERR_STAT             CAM_UNI_ADL_DMA_A_IPUI_ERR_STAT;                 /* 0D08, 0x1A003D08 */
    CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_ADDR            CAM_UNI_ADL_DMA_A_DMA_DEBUG_ADDR;                /* 0D0C, 0x1A003D0C */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV1                  CAM_UNI_ADL_DMA_A_DMA_RSV1;                      /* 0D10, 0x1A003D10 */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV2                  CAM_UNI_ADL_DMA_A_DMA_RSV2;                      /* 0D14, 0x1A003D14 */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV3                  CAM_UNI_ADL_DMA_A_DMA_RSV3;                      /* 0D18, 0x1A003D18 */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV4                  CAM_UNI_ADL_DMA_A_DMA_RSV4;                      /* 0D1C, 0x1A003D1C */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV5                  CAM_UNI_ADL_DMA_A_DMA_RSV5;                      /* 0D20, 0x1A003D20 */
    CAM_UNI_REG_ADL_DMA_A_DMA_RSV6                  CAM_UNI_ADL_DMA_A_DMA_RSV6;                      /* 0D24, 0x1A003D24 */
    CAM_UNI_REG_ADL_DMA_A_DMA_DEBUG_SEL             CAM_UNI_ADL_DMA_A_DMA_DEBUG_SEL;                 /* 0D28, 0x1A003D28 */
    CAM_UNI_REG_ADL_DMA_A_DMA_BW_SELF_TEST          CAM_UNI_ADL_DMA_A_DMA_BW_SELF_TEST;              /* 0D2C, 0x1A003D2C */
    UINT32                                          rsv_0D30[116];                                   /* 0D30..0EFF, 0x1A003D30..1A003EFF */
    CAM_UNI_REG_ADL_RESET                           CAM_UNI_ADL_RESET;                               /* 0F00, 0x1A003F00 */
    CAM_UNI_REG_ADL_CTL                             CAM_UNI_ADL_CTL;                                 /* 0F04, 0x1A003F04 */
    UINT32                                          rsv_0F08[10];                                    /* 0F08..0F2F, 0x1A003F08..1A003F2F */
    CAM_UNI_REG_ADL_CROP_IN_START                   CAM_UNI_ADL_CROP_IN_START;                       /* 0F30, 0x1A003F30 */
    CAM_UNI_REG_ADL_CROP_IN_END                     CAM_UNI_ADL_CROP_IN_END;                         /* 0F34, 0x1A003F34 */
    CAM_UNI_REG_ADL_CROP_OUT_START                  CAM_UNI_ADL_CROP_OUT_START;                      /* 0F38, 0x1A003F38 */
    CAM_UNI_REG_ADL_CROP_OUT_END                    CAM_UNI_ADL_CROP_OUT_END;                        /* 0F3C, 0x1A003F3C */
    CAM_UNI_REG_ADL_DMA_ST                          CAM_UNI_ADL_DMA_ST;                              /* 0F40, 0x1A003F40 */
    CAM_UNI_REG_ADL_DCM_DIS                         CAM_UNI_ADL_DCM_DIS;                             /* 0F44, 0x1A003F44 */
    CAM_UNI_REG_ADL_DCM_ST                          CAM_UNI_ADL_DCM_ST;                              /* 0F48, 0x1A003F48 */
    CAM_UNI_REG_ADL_DMA_ERR_ST                      CAM_UNI_ADL_DMA_ERR_ST;                          /* 0F4C, 0x1A003F4C */
    CAM_UNI_REG_ADL_DMA_0_DEBUG                     CAM_UNI_ADL_DMA_0_DEBUG;                         /* 0F50, 0x1A003F50 */
    CAM_UNI_REG_ADL_DMA_1_DEBUG                     CAM_UNI_ADL_DMA_1_DEBUG;                         /* 0F54, 0x1A003F54 */
    UINT32                                          rsv_0F58[6];                                     /* 0F58..0F6F, 0x1A003F58..1A003F6F */
    CAM_UNI_REG_ADL_SPARE                           CAM_UNI_ADL_SPARE;                               /* 0F70, 0x1A003F70 */
    UINT32                                          rsv_0F74[3];                                     /* 0F74..0F7F, 0x1A003F74..1A003F7F */
    CAM_UNI_REG_ADL_INFO00                          CAM_UNI_ADL_INFO00;                              /* 0F80, 0x1A003F80 */
    CAM_UNI_REG_ADL_INFO01                          CAM_UNI_ADL_INFO01;                              /* 0F84, 0x1A003F84 */
    CAM_UNI_REG_ADL_INFO02                          CAM_UNI_ADL_INFO02;                              /* 0F88, 0x1A003F88 */
    CAM_UNI_REG_ADL_INFO03                          CAM_UNI_ADL_INFO03;                              /* 0F8C, 0x1A003F8C */
    CAM_UNI_REG_ADL_INFO04                          CAM_UNI_ADL_INFO04;                              /* 0F90, 0x1A003F90 */
    CAM_UNI_REG_ADL_INFO05                          CAM_UNI_ADL_INFO05;                              /* 0F94, 0x1A003F94 */
    CAM_UNI_REG_ADL_INFO06                          CAM_UNI_ADL_INFO06;                              /* 0F98, 0x1A003F98 */
    CAM_UNI_REG_ADL_INFO07                          CAM_UNI_ADL_INFO07;                              /* 0F9C, 0x1A003F9C */
    CAM_UNI_REG_ADL_INFO08                          CAM_UNI_ADL_INFO08;                              /* 0FA0, 0x1A003FA0 */
    CAM_UNI_REG_ADL_INFO09                          CAM_UNI_ADL_INFO09;                              /* 0FA4, 0x1A003FA4 */
    CAM_UNI_REG_ADL_INFO10                          CAM_UNI_ADL_INFO10;                              /* 0FA8, 0x1A003FA8 */
    CAM_UNI_REG_ADL_INFO11                          CAM_UNI_ADL_INFO11;                              /* 0FAC, 0x1A003FAC */
    CAM_UNI_REG_ADL_INFO12                          CAM_UNI_ADL_INFO12;                              /* 0FB0, 0x1A003FB0 */
    CAM_UNI_REG_ADL_INFO13                          CAM_UNI_ADL_INFO13;                              /* 0FB4, 0x1A003FB4 */
    CAM_UNI_REG_ADL_INFO14                          CAM_UNI_ADL_INFO14;                              /* 0FB8, 0x1A003FB8 */
    CAM_UNI_REG_ADL_INFO15                          CAM_UNI_ADL_INFO15;                              /* 0FBC, 0x1A003FBC */
    CAM_UNI_REG_ADL_INFO16                          CAM_UNI_ADL_INFO16;                              /* 0FC0, 0x1A003FC0 */
    CAM_UNI_REG_ADL_INFO17                          CAM_UNI_ADL_INFO17;                              /* 0FC4, 0x1A003FC4 */
    CAM_UNI_REG_ADL_INFO18                          CAM_UNI_ADL_INFO18;                              /* 0FC8, 0x1A003FC8 */
    CAM_UNI_REG_ADL_INFO19                          CAM_UNI_ADL_INFO19;                              /* 0FCC, 0x1A003FCC */
    CAM_UNI_REG_ADL_INFO20                          CAM_UNI_ADL_INFO20;                              /* 0FD0, 0x1A003FD0 */
    CAM_UNI_REG_ADL_INFO21                          CAM_UNI_ADL_INFO21;                              /* 0FD4, 0x1A003FD4 */
    CAM_UNI_REG_ADL_INFO22                          CAM_UNI_ADL_INFO22;                              /* 0FD8, 0x1A003FD8 */
    CAM_UNI_REG_ADL_INFO23                          CAM_UNI_ADL_INFO23;                              /* 0FDC, 0x1A003FDC */
    CAM_UNI_REG_ADL_INFO24                          CAM_UNI_ADL_INFO24;                              /* 0FE0, 0x1A003FE0 */
    CAM_UNI_REG_ADL_INFO25                          CAM_UNI_ADL_INFO25;                              /* 0FE4, 0x1A003FE4 */
    CAM_UNI_REG_ADL_INFO26                          CAM_UNI_ADL_INFO26;                              /* 0FE8, 0x1A003FE8 */
    CAM_UNI_REG_ADL_INFO27                          CAM_UNI_ADL_INFO27;                              /* 0FEC, 0x1A003FEC */
    CAM_UNI_REG_ADL_INFO28                          CAM_UNI_ADL_INFO28;                              /* 0FF0, 0x1A003FF0 */
    CAM_UNI_REG_ADL_INFO29                          CAM_UNI_ADL_INFO29;                              /* 0FF4, 0x1A003FF4 */
    CAM_UNI_REG_ADL_INFO30                          CAM_UNI_ADL_INFO30;                              /* 0FF8, 0x1A003FF8 */
    CAM_UNI_REG_ADL_INFO31                          CAM_UNI_ADL_INFO31;                              /* 0FFC, 0x1A003FFC */
}cam_uni_reg_t;

typedef volatile struct _cam_reg_t_ /* 0x1A004000..0x1A00574F */
{
    CAM_REG_CTL_START                               CAM_CTL_START;                                   /* 0000, 0x1A004000, CAM_A_CTL_START */
    CAM_REG_CTL_EN                                  CAM_CTL_EN;                                      /* 0004, 0x1A004004, CAM_A_CTL_EN */
    CAM_REG_CTL_DMA_EN                              CAM_CTL_DMA_EN;                                  /* 0008, 0x1A004008, CAM_A_CTL_DMA_EN */
    CAM_REG_CTL_FMT_SEL                             CAM_CTL_FMT_SEL;                                 /* 000C, 0x1A00400C, CAM_A_CTL_FMT_SEL */
    CAM_REG_CTL_SEL                                 CAM_CTL_SEL;                                     /* 0010, 0x1A004010, CAM_A_CTL_SEL */
    CAM_REG_CTL_MISC                                CAM_CTL_MISC;                                    /* 0014, 0x1A004014, CAM_A_CTL_MISC */
    CAM_REG_CTL_EN2                                 CAM_CTL_EN2;                                     /* 0018, 0x1A004018, CAM_A_CTL_EN2 */
    UINT32                                          rsv_001C;                                        /* 001C, 0x1A00401C */
    CAM_REG_CTL_RAW_INT_EN                          CAM_CTL_RAW_INT_EN;                              /* 0020, 0x1A004020, CAM_A_CTL_RAW_INT_EN */
    CAM_REG_CTL_RAW_INT_STATUS                      CAM_CTL_RAW_INT_STATUS;                          /* 0024, 0x1A004024, CAM_A_CTL_RAW_INT_STATUS */
    CAM_REG_CTL_RAW_INT_STATUSX                     CAM_CTL_RAW_INT_STATUSX;                         /* 0028, 0x1A004028, CAM_A_CTL_RAW_INT_STATUSX */
    UINT32                                          rsv_002C;                                        /* 002C, 0x1A00402C */
    CAM_REG_CTL_RAW_INT2_EN                         CAM_CTL_RAW_INT2_EN;                             /* 0030, 0x1A004030, CAM_A_CTL_RAW_INT2_EN */
    CAM_REG_CTL_RAW_INT2_STATUS                     CAM_CTL_RAW_INT2_STATUS;                         /* 0034, 0x1A004034, CAM_A_CTL_RAW_INT2_STATUS */
    CAM_REG_CTL_RAW_INT2_STATUSX                    CAM_CTL_RAW_INT2_STATUSX;                        /* 0038, 0x1A004038, CAM_A_CTL_RAW_INT2_STATUSX */
    UINT32                                          rsv_003C;                                        /* 003C, 0x1A00403C */
    CAM_REG_CTL_SW_CTL                              CAM_CTL_SW_CTL;                                  /* 0040, 0x1A004040, CAM_A_CTL_SW_CTL */
    CAM_REG_CTL_AB_DONE_SEL                         CAM_CTL_AB_DONE_SEL;                             /* 0044, 0x1A004044, CAM_A_CTL_AB_DONE_SEL */
    CAM_REG_CTL_CD_DONE_SEL                         CAM_CTL_CD_DONE_SEL;                             /* 0048, 0x1A004048, CAM_A_CTL_CD_DONE_SEL */
    CAM_REG_CTL_UNI_DONE_SEL                        CAM_CTL_UNI_DONE_SEL;                            /* 004C, 0x1A00404C, CAM_A_CTL_UNI_DONE_SEL */
    CAM_REG_CTL_TWIN_STATUS                         CAM_CTL_TWIN_STATUS;                             /* 0050, 0x1A004050, CAM_A_CTL_SPARE0 */
    CAM_REG_CTL_SPARE1                              CAM_CTL_SPARE1;                                  /* 0054, 0x1A004054, CAM_A_CTL_SPARE1 */
    CAM_REG_CTL_SPARE2                              CAM_CTL_SPARE2;                                  /* 0058, 0x1A004058, CAM_A_CTL_SPARE2 */
    CAM_REG_CTL_SW_PASS1_DONE                       CAM_CTL_SW_PASS1_DONE;                           /* 005C, 0x1A00405C, CAM_A_CTL_SW_PASS1_DONE */
    CAM_REG_CTL_FBC_RCNT_INC                        CAM_CTL_FBC_RCNT_INC;                            /* 0060, 0x1A004060, CAM_A_CTL_FBC_RCNT_INC */
    UINT32                                          rsv_0064[3];                                     /* 0064..006F, 0x1A004064..1A00406F */
    CAM_REG_CTL_DBG_SET                             CAM_CTL_DBG_SET;                                 /* 0070, 0x1A004070, CAM_A_CTL_DBG_SET */
    CAM_REG_CTL_DBG_PORT                            CAM_CTL_DBG_PORT;                                /* 0074, 0x1A004074, CAM_A_CTL_DBG_PORT */
    CAM_REG_CTL_DATE_CODE                           CAM_CTL_DATE_CODE;                               /* 0078, 0x1A004078, CAM_A_CTL_DATE_CODE */
    CAM_REG_CTL_PROJ_CODE                           CAM_CTL_PROJ_CODE;                               /* 007C, 0x1A00407C, CAM_A_CTL_PROJ_CODE */
    CAM_REG_CTL_RAW_DCM_DIS                         CAM_CTL_RAW_DCM_DIS;                             /* 0080, 0x1A004080, CAM_A_CTL_RAW_DCM_DIS */
    CAM_REG_CTL_DMA_DCM_DIS                         CAM_CTL_DMA_DCM_DIS;                             /* 0084, 0x1A004084, CAM_A_CTL_DMA_DCM_DIS */
    CAM_REG_CTL_TOP_DCM_DIS                         CAM_CTL_TOP_DCM_DIS;                             /* 0088, 0x1A004088, CAM_A_CTL_TOP_DCM_DIS */
    UINT32                                          rsv_008C;                                        /* 008C, 0x1A00408C */
    CAM_REG_CTL_RAW_DCM_STATUS                      CAM_CTL_RAW_DCM_STATUS;                          /* 0090, 0x1A004090, CAM_A_CTL_RAW_DCM_STATUS */
    CAM_REG_CTL_DMA_DCM_STATUS                      CAM_CTL_DMA_DCM_STATUS;                          /* 0094, 0x1A004094, CAM_A_CTL_DMA_DCM_STATUS */
    CAM_REG_CTL_TOP_DCM_STATUS                      CAM_CTL_TOP_DCM_STATUS;                          /* 0098, 0x1A004098, CAM_A_CTL_TOP_DCM_STATUS */
    UINT32                                          rsv_009C;                                        /* 009C, 0x1A00409C */
    CAM_REG_CTL_RAW_REQ_STATUS                      CAM_CTL_RAW_REQ_STATUS;                          /* 00A0, 0x1A0040A0, CAM_A_CTL_RAW_REQ_STATUS */
    CAM_REG_CTL_DMA_REQ_STATUS                      CAM_CTL_DMA_REQ_STATUS;                          /* 00A4, 0x1A0040A4, CAM_A_CTL_DMA_REQ_STATUS */
    CAM_REG_CTL_RAW_RDY_STATUS                      CAM_CTL_RAW_RDY_STATUS;                          /* 00A8, 0x1A0040A8, CAM_A_CTL_RAW_RDY_STATUS */
    CAM_REG_CTL_DMA_RDY_STATUS                      CAM_CTL_DMA_RDY_STATUS;                          /* 00AC, 0x1A0040AC, CAM_A_CTL_DMA_RDY_STATUS */
    CAM_REG_CTL_RAW_CCU_INT_EN                      CAM_CTL_RAW_CCU_INT_EN;                          /* 00B0, 0x1A0040B0, CAM_A_CTL_RAW_CCU_INT_EN */
    CAM_REG_CTL_RAW_CCU_INT_STATUS                  CAM_CTL_RAW_CCU_INT_STATUS;                      /* 00B4, 0x1A0040B4, CAM_A_CTL_RAW_CCU_INT_STATUS */
    CAM_REG_CTL_RAW_CCU_INT2_EN                     CAM_CTL_RAW_CCU_INT2_EN;                         /* 00B8, 0x1A0040B8, CAM_A_CTL_RAW_CCU_INT2_EN */
    CAM_REG_CTL_RAW_CCU_INT2_STATUS                 CAM_CTL_RAW_CCU_INT2_STATUS;                     /* 00BC, 0x1A0040BC, CAM_A_CTL_RAW_CCU_INT2_STATUS */
    CAM_REG_CTL_RAW_INT3_EN                         CAM_CTL_RAW_INT3_EN;                             /* 00C0, 0x1A0040C0, CAM_A_CTL_RAW_INT3_EN */
    CAM_REG_CTL_RAW_INT3_STATUS                     CAM_CTL_RAW_INT3_STATUS;                         /* 00C4, 0x1A0040C4, CAM_A_CTL_RAW_INT3_STATUS */
    CAM_REG_CTL_RAW_INT3_STATUSX                    CAM_CTL_RAW_INT3_STATUSX;                        /* 00C8, 0x1A0040C8, CAM_A_CTL_RAW_INT3_STATUSX */
    UINT32                                          rsv_00CC;                                        /* 00CC, 0x1A0040CC */
    CAM_REG_CTL_RAW_CCU_INT3_EN                     CAM_CTL_RAW_CCU_INT3_EN;                         /* 00D0, 0x1A0040D0, CAM_A_CTL_RAW_CCU_INT3_EN */
    CAM_REG_CTL_RAW_CCU_INT3_STATUS                 CAM_CTL_RAW_CCU_INT3_STATUS;                     /* 00D4, 0x1A0040D4, CAM_A_CTL_RAW_CCU_INT3_STATUS */
    CAM_REG_CTL_UNI_B_DONE_SEL                      CAM_CTL_UNI_B_DONE_SEL;                          /* 00D8, 0x1A0040D8, CAM_A_CTL_UNI_B_DONE_SEL */
    UINT32                                          rsv_00DC;                                        /* 00DC, 0x1A0040DC */
    CAM_REG_CTL_RAW2_DCM_DIS                        CAM_CTL_RAW2_DCM_DIS;                            /* 00E0, 0x1A0040E0, CAM_A_CTL_RAW2_DCM_DIS */
    CAM_REG_CTL_RAW2_DCM_STATUS                     CAM_CTL_RAW2_DCM_STATUS;                         /* 00E4, 0x1A0040E4, CAM_A_CTL_RAW2_DCM_STATUS */
    CAM_REG_CTL_RAW2_REQ_STATUS                     CAM_CTL_RAW2_REQ_STATUS;                         /* 00E8, 0x1A0040E8, CAM_A_CTL_RAW2_REQ_STATUS */
    CAM_REG_CTL_RAW2_RDY_STATUS                     CAM_CTL_RAW2_RDY_STATUS;                         /* 00EC, 0x1A0040EC, CAM_A_CTL_RAW2_RDY_STATUS */
    UINT32                                          rsv_00F0[8];                                     /* 00F0..010F, 0x1A0040F0..1A00410F */
    CAM_REG_FBC_IMGO_CTL1                           CAM_FBC_IMGO_CTL1;                               /* 0110, 0x1A004110, CAM_A_FBC_IMGO_CTL1 */
    CAM_REG_FBC_IMGO_CTL2                           CAM_FBC_IMGO_CTL2;                               /* 0114, 0x1A004114, CAM_A_FBC_IMGO_CTL2 */
    CAM_REG_FBC_RRZO_CTL1                           CAM_FBC_RRZO_CTL1;                               /* 0118, 0x1A004118, CAM_A_FBC_RRZO_CTL1 */
    CAM_REG_FBC_RRZO_CTL2                           CAM_FBC_RRZO_CTL2;                               /* 011C, 0x1A00411C, CAM_A_FBC_RRZO_CTL2 */
    CAM_REG_FBC_UFEO_CTL1                           CAM_FBC_UFEO_CTL1;                               /* 0120, 0x1A004120, CAM_A_FBC_UFEO_CTL1 */
    CAM_REG_FBC_UFEO_CTL2                           CAM_FBC_UFEO_CTL2;                               /* 0124, 0x1A004124, CAM_A_FBC_UFEO_CTL2 */
    CAM_REG_FBC_LCSO_CTL1                           CAM_FBC_LCSO_CTL1;                               /* 0128, 0x1A004128, CAM_A_FBC_LCSO_CTL1 */
    CAM_REG_FBC_LCSO_CTL2                           CAM_FBC_LCSO_CTL2;                               /* 012C, 0x1A00412C, CAM_A_FBC_LCSO_CTL2 */
    CAM_REG_FBC_AFO_CTL1                            CAM_FBC_AFO_CTL1;                                /* 0130, 0x1A004130, CAM_A_FBC_AFO_CTL1 */
    CAM_REG_FBC_AFO_CTL2                            CAM_FBC_AFO_CTL2;                                /* 0134, 0x1A004134, CAM_A_FBC_AFO_CTL2 */
    CAM_REG_FBC_AAO_CTL1                            CAM_FBC_AAO_CTL1;                                /* 0138, 0x1A004138, CAM_A_FBC_AAO_CTL1 */
    CAM_REG_FBC_AAO_CTL2                            CAM_FBC_AAO_CTL2;                                /* 013C, 0x1A00413C, CAM_A_FBC_AAO_CTL2 */
    CAM_REG_FBC_PDO_CTL1                            CAM_FBC_PDO_CTL1;                                /* 0140, 0x1A004140, CAM_A_FBC_PDO_CTL1 */
    CAM_REG_FBC_PDO_CTL2                            CAM_FBC_PDO_CTL2;                                /* 0144, 0x1A004144, CAM_A_FBC_PDO_CTL2 */
    CAM_REG_FBC_PSO_CTL1                            CAM_FBC_PSO_CTL1;                                /* 0148, 0x1A004148, CAM_A_FBC_PSO_CTL1 */
    CAM_REG_FBC_PSO_CTL2                            CAM_FBC_PSO_CTL2;                                /* 014C, 0x1A00414C, CAM_A_FBC_PSO_CTL2 */
    CAM_REG_FBC_FLKO_CTL1                           CAM_FBC_FLKO_CTL1;                               /* 0150, 0x1A004150, CAM_A_FBC_FLKO_CTL1 */
    CAM_REG_FBC_FLKO_CTL2                           CAM_FBC_FLKO_CTL2;                               /* 0154, 0x1A004154, CAM_A_FBC_FLKO_CTL2 */
    CAM_REG_FBC_LMVO_CTL1                           CAM_FBC_LMVO_CTL1;                               /* 0158, 0x1A004158, CAM_A_FBC_LMVO_CTL1 */
    CAM_REG_FBC_LMVO_CTL2                           CAM_FBC_LMVO_CTL2;                               /* 015C, 0x1A00415C, CAM_A_FBC_LMVO_CTL2 */
    CAM_REG_FBC_RSSO_CTL1                           CAM_FBC_RSSO_CTL1;                               /* 0160, 0x1A004160, CAM_A_FBC_RSSO_CTL1 */
    CAM_REG_FBC_RSSO_CTL2                           CAM_FBC_RSSO_CTL2;                               /* 0164, 0x1A004164, CAM_A_FBC_RSSO_CTL2 */
    CAM_REG_FBC_UFGO_CTL1                           CAM_FBC_UFGO_CTL1;                               /* 0168, 0x1A004168, CAM_A_FBC_UFGO_CTL1 */
    CAM_REG_FBC_UFGO_CTL2                           CAM_FBC_UFGO_CTL2;                               /* 016C, 0x1A00416C, CAM_A_FBC_UFGO_CTL2 */
    UINT32                                          rsv_0170[8];                                     /* 0170..018F, 0x1A004170..1A00418F */
    CAM_REG_CQ_EN                                   CAM_CQ_EN;                                       /* 0190, 0x1A004190, CAM_A_CQ_EN */
    CAM_REG_CQ_THR0_CTL                             CAM_CQ_THR0_CTL;                                 /* 0194, 0x1A004194, CAM_A_CQ_THR0_CTL */
    CAM_REG_CQ_THR0_BASEADDR                        CAM_CQ_THR0_BASEADDR;                            /* 0198, 0x1A004198, CAM_A_CQ_THR0_BASEADDR */
    CAM_REG_CQ_THR0_DESC_SIZE                       CAM_CQ_THR0_DESC_SIZE;                           /* 019C, 0x1A00419C, CAM_A_CQ_THR0_DESC_SIZE */
    CAM_REG_CQ_THR1_CTL                             CAM_CQ_THR1_CTL;                                 /* 01A0, 0x1A0041A0, CAM_A_CQ_THR1_CTL */
    CAM_REG_CQ_THR1_BASEADDR                        CAM_CQ_THR1_BASEADDR;                            /* 01A4, 0x1A0041A4, CAM_A_CQ_THR1_BASEADDR */
    CAM_REG_CQ_THR1_DESC_SIZE                       CAM_CQ_THR1_DESC_SIZE;                           /* 01A8, 0x1A0041A8, CAM_A_CQ_THR1_DESC_SIZE */
    CAM_REG_CQ_THR2_CTL                             CAM_CQ_THR2_CTL;                                 /* 01AC, 0x1A0041AC, CAM_A_CQ_THR2_CTL */
    CAM_REG_CQ_THR2_BASEADDR                        CAM_CQ_THR2_BASEADDR;                            /* 01B0, 0x1A0041B0, CAM_A_CQ_THR2_BASEADDR */
    CAM_REG_CQ_THR2_DESC_SIZE                       CAM_CQ_THR2_DESC_SIZE;                           /* 01B4, 0x1A0041B4, CAM_A_CQ_THR2_DESC_SIZE */
    CAM_REG_CQ_THR3_CTL                             CAM_CQ_THR3_CTL;                                 /* 01B8, 0x1A0041B8, CAM_A_CQ_THR3_CTL */
    CAM_REG_CQ_THR3_BASEADDR                        CAM_CQ_THR3_BASEADDR;                            /* 01BC, 0x1A0041BC, CAM_A_CQ_THR3_BASEADDR */
    CAM_REG_CQ_THR3_DESC_SIZE                       CAM_CQ_THR3_DESC_SIZE;                           /* 01C0, 0x1A0041C0, CAM_A_CQ_THR3_DESC_SIZE */
    CAM_REG_CQ_THR4_CTL                             CAM_CQ_THR4_CTL;                                 /* 01C4, 0x1A0041C4, CAM_A_CQ_THR4_CTL */
    CAM_REG_CQ_THR4_BASEADDR                        CAM_CQ_THR4_BASEADDR;                            /* 01C8, 0x1A0041C8, CAM_A_CQ_THR4_BASEADDR */
    CAM_REG_CQ_THR4_DESC_SIZE                       CAM_CQ_THR4_DESC_SIZE;                           /* 01CC, 0x1A0041CC, CAM_A_CQ_THR4_DESC_SIZE */
    CAM_REG_CQ_THR5_CTL                             CAM_CQ_THR5_CTL;                                 /* 01D0, 0x1A0041D0, CAM_A_CQ_THR5_CTL */
    CAM_REG_CQ_THR5_BASEADDR                        CAM_CQ_THR5_BASEADDR;                            /* 01D4, 0x1A0041D4, CAM_A_CQ_THR5_BASEADDR */
    CAM_REG_CQ_THR5_DESC_SIZE                       CAM_CQ_THR5_DESC_SIZE;                           /* 01D8, 0x1A0041D8, CAM_A_CQ_THR5_DESC_SIZE */
    CAM_REG_CQ_THR6_CTL                             CAM_CQ_THR6_CTL;                                 /* 01DC, 0x1A0041DC, CAM_A_CQ_THR6_CTL */
    CAM_REG_CQ_THR6_BASEADDR                        CAM_CQ_THR6_BASEADDR;                            /* 01E0, 0x1A0041E0, CAM_A_CQ_THR6_BASEADDR */
    CAM_REG_CQ_THR6_DESC_SIZE                       CAM_CQ_THR6_DESC_SIZE;                           /* 01E4, 0x1A0041E4, CAM_A_CQ_THR6_DESC_SIZE */
    CAM_REG_CQ_THR7_CTL                             CAM_CQ_THR7_CTL;                                 /* 01E8, 0x1A0041E8, CAM_A_CQ_THR7_CTL */
    CAM_REG_CQ_THR7_BASEADDR                        CAM_CQ_THR7_BASEADDR;                            /* 01EC, 0x1A0041EC, CAM_A_CQ_THR7_BASEADDR */
    CAM_REG_CQ_THR7_DESC_SIZE                       CAM_CQ_THR7_DESC_SIZE;                           /* 01F0, 0x1A0041F0, CAM_A_CQ_THR7_DESC_SIZE */
    CAM_REG_CQ_THR8_CTL                             CAM_CQ_THR8_CTL;                                 /* 01F4, 0x1A0041F4, CAM_A_CQ_THR8_CTL */
    CAM_REG_CQ_THR8_BASEADDR                        CAM_CQ_THR8_BASEADDR;                            /* 01F8, 0x1A0041F8, CAM_A_CQ_THR8_BASEADDR */
    CAM_REG_CQ_THR8_DESC_SIZE                       CAM_CQ_THR8_DESC_SIZE;                           /* 01FC, 0x1A0041FC, CAM_A_CQ_THR8_DESC_SIZE */
    CAM_REG_CQ_THR9_CTL                             CAM_CQ_THR9_CTL;                                 /* 0200, 0x1A004200, CAM_A_CQ_THR9_CTL */
    CAM_REG_CQ_THR9_BASEADDR                        CAM_CQ_THR9_BASEADDR;                            /* 0204, 0x1A004204, CAM_A_CQ_THR9_BASEADDR */
    CAM_REG_CQ_THR9_DESC_SIZE                       CAM_CQ_THR9_DESC_SIZE;                           /* 0208, 0x1A004208, CAM_A_CQ_THR9_DESC_SIZE */
    CAM_REG_CQ_THR10_CTL                            CAM_CQ_THR10_CTL;                                /* 020C, 0x1A00420C, CAM_A_CQ_THR10_CTL */
    CAM_REG_CQ_THR10_BASEADDR                       CAM_CQ_THR10_BASEADDR;                           /* 0210, 0x1A004210, CAM_A_CQ_THR10_BASEADDR */
    CAM_REG_CQ_THR10_DESC_SIZE                      CAM_CQ_THR10_DESC_SIZE;                          /* 0214, 0x1A004214, CAM_A_CQ_THR10_DESC_SIZE */
    CAM_REG_CQ_THR11_CTL                            CAM_CQ_THR11_CTL;                                /* 0218, 0x1A004218, CAM_A_CQ_THR11_CTL */
    CAM_REG_CQ_THR11_BASEADDR                       CAM_CQ_THR11_BASEADDR;                           /* 021C, 0x1A00421C, CAM_A_CQ_THR11_BASEADDR */
    CAM_REG_CQ_THR11_DESC_SIZE                      CAM_CQ_THR11_DESC_SIZE;                          /* 0220, 0x1A004220, CAM_A_CQ_THR11_DESC_SIZE */
    CAM_REG_CQ_THR12_CTL                            CAM_CQ_THR12_CTL;                                /* 0224, 0x1A004224, CAM_A_CQ_THR12_CTL */
    CAM_REG_CQ_THR12_BASEADDR                       CAM_CQ_THR12_BASEADDR;                           /* 0228, 0x1A004228, CAM_A_CQ_THR12_BASEADDR */
    CAM_REG_CQ_THR12_DESC_SIZE                      CAM_CQ_THR12_DESC_SIZE;                          /* 022C, 0x1A00422C, CAM_A_CQ_THR12_DESC_SIZE */
    CAM_REG_TG_SEN_MODE                             CAM_TG_SEN_MODE;                                 /* 0230, 0x1A004230, CAM_A_TG_SEN_MODE */
    CAM_REG_TG_VF_CON                               CAM_TG_VF_CON;                                   /* 0234, 0x1A004234, CAM_A_TG_VF_CON */
    CAM_REG_TG_SEN_GRAB_PXL                         CAM_TG_SEN_GRAB_PXL;                             /* 0238, 0x1A004238, CAM_A_TG_SEN_GRAB_PXL */
    CAM_REG_TG_SEN_GRAB_LIN                         CAM_TG_SEN_GRAB_LIN;                             /* 023C, 0x1A00423C, CAM_A_TG_SEN_GRAB_LIN */
    CAM_REG_TG_PATH_CFG                             CAM_TG_PATH_CFG;                                 /* 0240, 0x1A004240, CAM_A_TG_PATH_CFG */
    CAM_REG_TG_MEMIN_CTL                            CAM_TG_MEMIN_CTL;                                /* 0244, 0x1A004244, CAM_A_TG_MEMIN_CTL */
    CAM_REG_TG_INT1                                 CAM_TG_INT1;                                     /* 0248, 0x1A004248, CAM_A_TG_INT1 */
    CAM_REG_TG_INT2                                 CAM_TG_INT2;                                     /* 024C, 0x1A00424C, CAM_A_TG_INT2 */
    CAM_REG_TG_SOF_CNT                              CAM_TG_SOF_CNT;                                  /* 0250, 0x1A004250, CAM_A_TG_SOF_CNT */
    CAM_REG_TG_SOT_CNT                              CAM_TG_SOT_CNT;                                  /* 0254, 0x1A004254, CAM_A_TG_SOT_CNT */
    CAM_REG_TG_EOT_CNT                              CAM_TG_EOT_CNT;                                  /* 0258, 0x1A004258, CAM_A_TG_EOT_CNT */
    CAM_REG_TG_ERR_CTL                              CAM_TG_ERR_CTL;                                  /* 025C, 0x1A00425C, CAM_A_TG_ERR_CTL */
    CAM_REG_TG_DAT_NO                               CAM_TG_DAT_NO;                                   /* 0260, 0x1A004260, CAM_A_TG_DAT_NO */
    CAM_REG_TG_FRM_CNT_ST                           CAM_TG_FRM_CNT_ST;                               /* 0264, 0x1A004264, CAM_A_TG_FRM_CNT_ST */
    CAM_REG_TG_FRMSIZE_ST                           CAM_TG_FRMSIZE_ST;                               /* 0268, 0x1A004268, CAM_A_TG_FRMSIZE_ST */
    CAM_REG_TG_INTER_ST                             CAM_TG_INTER_ST;                                 /* 026C, 0x1A00426C, CAM_A_TG_INTER_ST */
    CAM_REG_TG_FLASHA_CTL                           CAM_TG_FLASHA_CTL;                               /* 0270, 0x1A004270, CAM_A_TG_FLASHA_CTL */
    CAM_REG_TG_FLASHA_LINE_CNT                      CAM_TG_FLASHA_LINE_CNT;                          /* 0274, 0x1A004274, CAM_A_TG_FLASHA_LINE_CNT */
    CAM_REG_TG_FLASHA_POS                           CAM_TG_FLASHA_POS;                               /* 0278, 0x1A004278, CAM_A_TG_FLASHA_POS */
    CAM_REG_TG_FLASHB_CTL                           CAM_TG_FLASHB_CTL;                               /* 027C, 0x1A00427C, CAM_A_TG_FLASHB_CTL */
    CAM_REG_TG_FLASHB_LINE_CNT                      CAM_TG_FLASHB_LINE_CNT;                          /* 0280, 0x1A004280, CAM_A_TG_FLASHB_LINE_CNT */
    CAM_REG_TG_FLASHB_POS                           CAM_TG_FLASHB_POS;                               /* 0284, 0x1A004284, CAM_A_TG_FLASHB_POS */
    CAM_REG_TG_FLASHB_POS1                          CAM_TG_FLASHB_POS1;                              /* 0288, 0x1A004288, CAM_A_TG_FLASHB_POS1 */
    UINT32                                          rsv_028C;                                        /* 028C, 0x1A00428C */
    CAM_REG_TG_I2C_CQ_TRIG                          CAM_TG_I2C_CQ_TRIG;                              /* 0290, 0x1A004290, CAM_A_TG_I2C_CQ_TRIG */
    CAM_REG_TG_CQ_TIMING                            CAM_TG_CQ_TIMING;                                /* 0294, 0x1A004294, CAM_A_TG_CQ_TIMING */
    CAM_REG_TG_CQ_NUM                               CAM_TG_CQ_NUM;                                   /* 0298, 0x1A004298, CAM_A_TG_CQ_NUM */
    UINT32                                          rsv_029C;                                        /* 029C, 0x1A00429C */
    CAM_REG_TG_TIME_STAMP                           CAM_TG_TIME_STAMP;                               /* 02A0, 0x1A0042A0, CAM_A_TG_TIME_STAMP */
    CAM_REG_TG_SUB_PERIOD                           CAM_TG_SUB_PERIOD;                               /* 02A4, 0x1A0042A4, CAM_A_TG_SUB_PERIOD */
    CAM_REG_TG_DAT_NO_R                             CAM_TG_DAT_NO_R;                                 /* 02A8, 0x1A0042A8, CAM_A_TG_DAT_NO_R */
    CAM_REG_TG_FRMSIZE_ST_R                         CAM_TG_FRMSIZE_ST_R;                             /* 02AC, 0x1A0042AC, CAM_A_TG_FRMSIZE_ST_R */
    CAM_REG_TG_TIME_STAMP_CTL                       CAM_TG_TIME_STAMP_CTL;                           /* 02B0, 0x1A0042B0, CAM_A_TG_TIME_STAMP_CTL */
    CAM_REG_TG_TIME_STAMP_MSB                       CAM_TG_TIME_STAMP_MSB;                           /* 02B4, 0x1A0042B4, CAM_A_TG_TIME_STAMP_MSB */
    UINT32                                          rsv_02B8[30];                                    /* 02B8..032F, 0x1A0042B8..1A00432F */
    CAM_REG_DMX_CTL                                 CAM_DMX_CTL;                                     /* 0330, 0x1A004330, CAM_A_DMX_CTL */
    CAM_REG_DMX_CROP                                CAM_DMX_CROP;                                    /* 0334, 0x1A004334, CAM_A_DMX_CROP */
    CAM_REG_DMX_VSIZE                               CAM_DMX_VSIZE;                                   /* 0338, 0x1A004338, CAM_A_DMX_VSIZE */
    UINT32                                          rsv_033C[5];                                     /* 033C..034F, 0x1A00433C..1A00434F */
    CAM_REG_RMG_HDR_CFG                             CAM_RMG_HDR_CFG;                                 /* 0350, 0x1A004350, CAM_A_RMG_HDR_CFG */
    CAM_REG_RMG_HDR_GAIN                            CAM_RMG_HDR_GAIN;                                /* 0354, 0x1A004354, CAM_A_RMG_HDR_GAIN */
    CAM_REG_RMG_HDR_CFG2                            CAM_RMG_HDR_CFG2;                                /* 0358, 0x1A004358, CAM_A_RMG_HDR_CFG2 */
    UINT32                                          rsv_035C[13];                                    /* 035C..038F, 0x1A00435C..1A00438F */
    CAM_REG_RMM_OSC                                 CAM_RMM_OSC;                                     /* 0390, 0x1A004390, CAM_A_RMM_OSC */
    CAM_REG_RMM_MC                                  CAM_RMM_MC;                                      /* 0394, 0x1A004394, CAM_A_RMM_MC */
    CAM_REG_RMM_REVG_1                              CAM_RMM_REVG_1;                                  /* 0398, 0x1A004398, CAM_A_RMM_REVG_1 */
    CAM_REG_RMM_REVG_2                              CAM_RMM_REVG_2;                                  /* 039C, 0x1A00439C, CAM_A_RMM_REVG_2 */
    CAM_REG_RMM_LEOS                                CAM_RMM_LEOS;                                    /* 03A0, 0x1A0043A0, CAM_A_RMM_LEOS */
    CAM_REG_RMM_MC2                                 CAM_RMM_MC2;                                     /* 03A4, 0x1A0043A4, CAM_A_RMM_MC2 */
    CAM_REG_RMM_DIFF_LB                             CAM_RMM_DIFF_LB;                                 /* 03A8, 0x1A0043A8, CAM_A_RMM_DIFF_LB */
    CAM_REG_RMM_MA                                  CAM_RMM_MA;                                      /* 03AC, 0x1A0043AC, CAM_A_RMM_MA */
    CAM_REG_RMM_TUNE                                CAM_RMM_TUNE;                                    /* 03B0, 0x1A0043B0, CAM_A_RMM_TUNE */
    UINT32                                          rsv_03B4[15];                                    /* 03B4..03EF, 0x1A0043B4..1A0043EF */
    CAM_REG_OBC_OFFST0                              CAM_OBC_OFFST0;                                  /* 03F0, 0x1A0043F0, CAM_A_OBC_OFFST0 */
    CAM_REG_OBC_OFFST1                              CAM_OBC_OFFST1;                                  /* 03F4, 0x1A0043F4, CAM_A_OBC_OFFST1 */
    CAM_REG_OBC_OFFST2                              CAM_OBC_OFFST2;                                  /* 03F8, 0x1A0043F8, CAM_A_OBC_OFFST2 */
    CAM_REG_OBC_OFFST3                              CAM_OBC_OFFST3;                                  /* 03FC, 0x1A0043FC, CAM_A_OBC_OFFST3 */
    CAM_REG_OBC_GAIN0                               CAM_OBC_GAIN0;                                   /* 0400, 0x1A004400, CAM_A_OBC_GAIN0 */
    CAM_REG_OBC_GAIN1                               CAM_OBC_GAIN1;                                   /* 0404, 0x1A004404, CAM_A_OBC_GAIN1 */
    CAM_REG_OBC_GAIN2                               CAM_OBC_GAIN2;                                   /* 0408, 0x1A004408, CAM_A_OBC_GAIN2 */
    CAM_REG_OBC_GAIN3                               CAM_OBC_GAIN3;                                   /* 040C, 0x1A00440C, CAM_A_OBC_GAIN3 */
    UINT32                                          rsv_0410[4];                                     /* 0410..041F, 0x1A004410..1A00441F */
    CAM_REG_BNR_BPC_CON                             CAM_BNR_BPC_CON;                                 /* 0420, 0x1A004420, CAM_A_BNR_BPC_CON */
    CAM_REG_BNR_BPC_TH1                             CAM_BNR_BPC_TH1;                                 /* 0424, 0x1A004424, CAM_A_BNR_BPC_TH1 */
    CAM_REG_BNR_BPC_TH2                             CAM_BNR_BPC_TH2;                                 /* 0428, 0x1A004428, CAM_A_BNR_BPC_TH2 */
    CAM_REG_BNR_BPC_TH3                             CAM_BNR_BPC_TH3;                                 /* 042C, 0x1A00442C, CAM_A_BNR_BPC_TH3 */
    CAM_REG_BNR_BPC_TH4                             CAM_BNR_BPC_TH4;                                 /* 0430, 0x1A004430, CAM_A_BNR_BPC_TH4 */
    CAM_REG_BNR_BPC_DTC                             CAM_BNR_BPC_DTC;                                 /* 0434, 0x1A004434, CAM_A_BNR_BPC_DTC */
    CAM_REG_BNR_BPC_COR                             CAM_BNR_BPC_COR;                                 /* 0438, 0x1A004438, CAM_A_BNR_BPC_COR */
    CAM_REG_BNR_BPC_TBLI1                           CAM_BNR_BPC_TBLI1;                               /* 043C, 0x1A00443C, CAM_A_BNR_BPC_TBLI1 */
    CAM_REG_BNR_BPC_TBLI2                           CAM_BNR_BPC_TBLI2;                               /* 0440, 0x1A004440, CAM_A_BNR_BPC_TBLI2 */
    CAM_REG_BNR_BPC_TH1_C                           CAM_BNR_BPC_TH1_C;                               /* 0444, 0x1A004444, CAM_A_BNR_BPC_TH1_C */
    CAM_REG_BNR_BPC_TH2_C                           CAM_BNR_BPC_TH2_C;                               /* 0448, 0x1A004448, CAM_A_BNR_BPC_TH2_C */
    CAM_REG_BNR_BPC_TH3_C                           CAM_BNR_BPC_TH3_C;                               /* 044C, 0x1A00444C, CAM_A_BNR_BPC_TH3_C */
    CAM_REG_BNR_NR1_CON                             CAM_BNR_NR1_CON;                                 /* 0450, 0x1A004450, CAM_A_BNR_NR1_CON */
    CAM_REG_BNR_NR1_CT_CON                          CAM_BNR_NR1_CT_CON;                              /* 0454, 0x1A004454, CAM_A_BNR_NR1_CT_CON */
    CAM_REG_BNR_NR1_CT_CON2                         CAM_BNR_NR1_CT_CON2;                             /* 0458, 0x1A004458, CAM_A_BNR_NR1_CT_CON2 */
    CAM_REG_BNR_NR1_CT_CON3                         CAM_BNR_NR1_CT_CON3;                             /* 045C, 0x1A00445C, CAM_A_BNR_NR1_CT_CON3 */
    CAM_REG_BNR_PDC_CON                             CAM_BNR_PDC_CON;                                 /* 0460, 0x1A004460, CAM_A_BNR_PDC_CON */
    CAM_REG_BNR_PDC_GAIN_L0                         CAM_BNR_PDC_GAIN_L0;                             /* 0464, 0x1A004464, CAM_A_BNR_PDC_GAIN_L0 */
    CAM_REG_BNR_PDC_GAIN_L1                         CAM_BNR_PDC_GAIN_L1;                             /* 0468, 0x1A004468, CAM_A_BNR_PDC_GAIN_L1 */
    CAM_REG_BNR_PDC_GAIN_L2                         CAM_BNR_PDC_GAIN_L2;                             /* 046C, 0x1A00446C, CAM_A_BNR_PDC_GAIN_L2 */
    CAM_REG_BNR_PDC_GAIN_L3                         CAM_BNR_PDC_GAIN_L3;                             /* 0470, 0x1A004470, CAM_A_BNR_PDC_GAIN_L3 */
    CAM_REG_BNR_PDC_GAIN_L4                         CAM_BNR_PDC_GAIN_L4;                             /* 0474, 0x1A004474, CAM_A_BNR_PDC_GAIN_L4 */
    CAM_REG_BNR_PDC_GAIN_R0                         CAM_BNR_PDC_GAIN_R0;                             /* 0478, 0x1A004478, CAM_A_BNR_PDC_GAIN_R0 */
    CAM_REG_BNR_PDC_GAIN_R1                         CAM_BNR_PDC_GAIN_R1;                             /* 047C, 0x1A00447C, CAM_A_BNR_PDC_GAIN_R1 */
    CAM_REG_BNR_PDC_GAIN_R2                         CAM_BNR_PDC_GAIN_R2;                             /* 0480, 0x1A004480, CAM_A_BNR_PDC_GAIN_R2 */
    CAM_REG_BNR_PDC_GAIN_R3                         CAM_BNR_PDC_GAIN_R3;                             /* 0484, 0x1A004484, CAM_A_BNR_PDC_GAIN_R3 */
    CAM_REG_BNR_PDC_GAIN_R4                         CAM_BNR_PDC_GAIN_R4;                             /* 0488, 0x1A004488, CAM_A_BNR_PDC_GAIN_R4 */
    CAM_REG_BNR_PDC_TH_GB                           CAM_BNR_PDC_TH_GB;                               /* 048C, 0x1A00448C, CAM_A_BNR_PDC_TH_GB */
    CAM_REG_BNR_PDC_TH_IA                           CAM_BNR_PDC_TH_IA;                               /* 0490, 0x1A004490, CAM_A_BNR_PDC_TH_IA */
    CAM_REG_BNR_PDC_TH_HD                           CAM_BNR_PDC_TH_HD;                               /* 0494, 0x1A004494, CAM_A_BNR_PDC_TH_HD */
    CAM_REG_BNR_PDC_SL                              CAM_BNR_PDC_SL;                                  /* 0498, 0x1A004498, CAM_A_BNR_PDC_SL */
    CAM_REG_BNR_PDC_POS                             CAM_BNR_PDC_POS;                                 /* 049C, 0x1A00449C, CAM_A_BNR_PDC_POS */
    CAM_REG_STM_CFG0                                CAM_STM_CFG0;                                    /* 04A0, 0x1A0044A0, CAM_A_STM_CFG0 */
    CAM_REG_STM_CFG1                                CAM_STM_CFG1;                                    /* 04A4, 0x1A0044A4, CAM_A_STM_CFG1 */
    UINT32                                          rsv_04A8[2];                                     /* 04A8..04AF, 0x1A0044A8..1A0044AF */
    CAM_REG_SCM_CFG0                                CAM_SCM_CFG0;                                    /* 04B0, 0x1A0044B0, CAM_A_SCM_CFG0 */
    CAM_REG_SCM_CFG1                                CAM_SCM_CFG1;                                    /* 04B4, 0x1A0044B4, CAM_A_SCM_CFG1 */
    UINT32                                          rsv_04B8[2];                                     /* 04B8..04BF, 0x1A0044B8..1A0044BF */
    CAM_REG_RPG_SATU_1                              CAM_RPG_SATU_1;                                  /* 04C0, 0x1A0044C0, CAM_A_RPG_SATU_1 */
    CAM_REG_RPG_SATU_2                              CAM_RPG_SATU_2;                                  /* 04C4, 0x1A0044C4, CAM_A_RPG_SATU_2 */
    CAM_REG_RPG_GAIN_1                              CAM_RPG_GAIN_1;                                  /* 04C8, 0x1A0044C8, CAM_A_RPG_GAIN_1 */
    CAM_REG_RPG_GAIN_2                              CAM_RPG_GAIN_2;                                  /* 04CC, 0x1A0044CC, CAM_A_RPG_GAIN_2 */
    CAM_REG_RPG_OFST_1                              CAM_RPG_OFST_1;                                  /* 04D0, 0x1A0044D0, CAM_A_RPG_OFST_1 */
    CAM_REG_RPG_OFST_2                              CAM_RPG_OFST_2;                                  /* 04D4, 0x1A0044D4, CAM_A_RPG_OFST_2 */
    UINT32                                          rsv_04D8[2];                                     /* 04D8..04DF, 0x1A0044D8..1A0044DF */
    CAM_REG_RRZ_CTL                                 CAM_RRZ_CTL;                                     /* 04E0, 0x1A0044E0, CAM_A_RRZ_CTL */
    CAM_REG_RRZ_IN_IMG                              CAM_RRZ_IN_IMG;                                  /* 04E4, 0x1A0044E4, CAM_A_RRZ_IN_IMG */
    CAM_REG_RRZ_OUT_IMG                             CAM_RRZ_OUT_IMG;                                 /* 04E8, 0x1A0044E8, CAM_A_RRZ_OUT_IMG */
    CAM_REG_RRZ_HORI_STEP                           CAM_RRZ_HORI_STEP;                               /* 04EC, 0x1A0044EC, CAM_A_RRZ_HORI_STEP */
    CAM_REG_RRZ_VERT_STEP                           CAM_RRZ_VERT_STEP;                               /* 04F0, 0x1A0044F0, CAM_A_RRZ_VERT_STEP */
    CAM_REG_RRZ_HORI_INT_OFST                       CAM_RRZ_HORI_INT_OFST;                           /* 04F4, 0x1A0044F4, CAM_A_RRZ_HORI_INT_OFST */
    CAM_REG_RRZ_HORI_SUB_OFST                       CAM_RRZ_HORI_SUB_OFST;                           /* 04F8, 0x1A0044F8, CAM_A_RRZ_HORI_SUB_OFST */
    CAM_REG_RRZ_VERT_INT_OFST                       CAM_RRZ_VERT_INT_OFST;                           /* 04FC, 0x1A0044FC, CAM_A_RRZ_VERT_INT_OFST */
    CAM_REG_RRZ_VERT_SUB_OFST                       CAM_RRZ_VERT_SUB_OFST;                           /* 0500, 0x1A004500, CAM_A_RRZ_VERT_SUB_OFST */
    CAM_REG_RRZ_MODE_TH                             CAM_RRZ_MODE_TH;                                 /* 0504, 0x1A004504, CAM_A_RRZ_MODE_TH */
    CAM_REG_RRZ_MODE_CTL                            CAM_RRZ_MODE_CTL;                                /* 0508, 0x1A004508, CAM_A_RRZ_MODE_CTL */
    CAM_REG_RRZ_RLB_AOFST                           CAM_RRZ_RLB_AOFST;                               /* 050C, 0x1A00450C, CAM_A_RRZ_RLB_AOFST */
    CAM_REG_RRZ_LBLD_CFG                            CAM_RRZ_LBLD_CFG;                                /* 0510, 0x1A004510, CAM_A_RRZ_LBLD_CFG */
    CAM_REG_RRZ_NNIR_TBL_SEL                        CAM_RRZ_NNIR_TBL_SEL;                            /* 0514, 0x1A004514, CAM_A_RRZ_NNIR_TBL_SEL */
    UINT32                                          rsv_0518[10];                                    /* 0518..053F, 0x1A004518..1A00453F */
    CAM_REG_RMX_CTL                                 CAM_RMX_CTL;                                     /* 0540, 0x1A004540, CAM_A_RMX_CTL */
    CAM_REG_RMX_CROP                                CAM_RMX_CROP;                                    /* 0544, 0x1A004544, CAM_A_RMX_CROP */
    CAM_REG_RMX_VSIZE                               CAM_RMX_VSIZE;                                   /* 0548, 0x1A004548, CAM_A_RMX_VSIZE */
    UINT32                                          rsv_054C[13];                                    /* 054C..057F, 0x1A00454C..1A00457F */
    CAM_REG_BMX_CTL                                 CAM_BMX_CTL;                                     /* 0580, 0x1A004580, CAM_A_BMX_CTL */
    CAM_REG_BMX_CROP                                CAM_BMX_CROP;                                    /* 0584, 0x1A004584, CAM_A_BMX_CROP */
    CAM_REG_BMX_VSIZE                               CAM_BMX_VSIZE;                                   /* 0588, 0x1A004588, CAM_A_BMX_VSIZE */
    UINT32                                          rsv_058C[13];                                    /* 058C..05BF, 0x1A00458C..1A0045BF */
    CAM_REG_UFEG_CON                                CAM_UFEG_CON;                                    /* 05C0, 0x1A0045C0, CAM_A_UFEG_CON */
    UINT32                                          rsv_05C4[3];                                     /* 05C4..05CF, 0x1A0045C4..1A0045CF */
    CAM_REG_LSC_CTL1                                CAM_LSC_CTL1;                                    /* 05D0, 0x1A0045D0, CAM_A_LSC_CTL1 */
    CAM_REG_LSC_CTL2                                CAM_LSC_CTL2;                                    /* 05D4, 0x1A0045D4, CAM_A_LSC_CTL2 */
    CAM_REG_LSC_CTL3                                CAM_LSC_CTL3;                                    /* 05D8, 0x1A0045D8, CAM_A_LSC_CTL3 */
    CAM_REG_LSC_LBLOCK                              CAM_LSC_LBLOCK;                                  /* 05DC, 0x1A0045DC, CAM_A_LSC_LBLOCK */
    CAM_REG_LSC_RATIO_0                             CAM_LSC_RATIO_0;                                 /* 05E0, 0x1A0045E0, CAM_A_LSC_RATIO_0 */
    CAM_REG_LSC_TPIPE_OFST                          CAM_LSC_TPIPE_OFST;                              /* 05E4, 0x1A0045E4, CAM_A_LSC_TPIPE_OFST */
    CAM_REG_LSC_TPIPE_SIZE                          CAM_LSC_TPIPE_SIZE;                              /* 05E8, 0x1A0045E8, CAM_A_LSC_TPIPE_SIZE */
    CAM_REG_LSC_GAIN_TH                             CAM_LSC_GAIN_TH;                                 /* 05EC, 0x1A0045EC, CAM_A_LSC_GAIN_TH */
    CAM_REG_LSC_RATIO_1                             CAM_LSC_RATIO_1;                                 /* 05F0, 0x1A0045F0, CAM_A_LSC_RATIO_1 */
    CAM_REG_LSC_UPB_B_GB                            CAM_LSC_UPB_B_GB;                                /* 05F4, 0x1A0045F4, CAM_A_LSC_UPB_B_GB */
    CAM_REG_LSC_UPB_GR_R                            CAM_LSC_UPB_GR_R;                                /* 05F8, 0x1A0045F8, CAM_A_LSC_UPB_GR_R */
    UINT32                                          rsv_05FC[5];                                     /* 05FC..060F, 0x1A0045FC..1A00460F */
    CAM_REG_AF_CON                                  CAM_AF_CON;                                      /* 0610, 0x1A004610, CAM_A_AF_CON */
    CAM_REG_AF_TH_0                                 CAM_AF_TH_0;                                     /* 0614, 0x1A004614, CAM_A_AF_TH_0 */
    CAM_REG_AF_TH_1                                 CAM_AF_TH_1;                                     /* 0618, 0x1A004618, CAM_A_AF_TH_1 */
    CAM_REG_AF_FLT_1                                CAM_AF_FLT_1;                                    /* 061C, 0x1A00461C, CAM_A_AF_FLT_1 */
    CAM_REG_AF_FLT_2                                CAM_AF_FLT_2;                                    /* 0620, 0x1A004620, CAM_A_AF_FLT_2 */
    CAM_REG_AF_FLT_3                                CAM_AF_FLT_3;                                    /* 0624, 0x1A004624, CAM_A_AF_FLT_3 */
    CAM_REG_AF_FLT_4                                CAM_AF_FLT_4;                                    /* 0628, 0x1A004628, CAM_A_AF_FLT_4 */
    CAM_REG_AF_FLT_5                                CAM_AF_FLT_5;                                    /* 062C, 0x1A00462C, CAM_A_AF_FLT_5 */
    CAM_REG_AF_FLT_6                                CAM_AF_FLT_6;                                    /* 0630, 0x1A004630, CAM_A_AF_FLT_6 */
    CAM_REG_AF_FLT_7                                CAM_AF_FLT_7;                                    /* 0634, 0x1A004634, CAM_A_AF_FLT_7 */
    CAM_REG_AF_FLT_8                                CAM_AF_FLT_8;                                    /* 0638, 0x1A004638, CAM_A_AF_FLT_8 */
    UINT32                                          rsv_063C;                                        /* 063C, 0x1A00463C */
    CAM_REG_AF_SIZE                                 CAM_AF_SIZE;                                     /* 0640, 0x1A004640, CAM_A_AF_SIZE */
    CAM_REG_AF_VLD                                  CAM_AF_VLD;                                      /* 0644, 0x1A004644, CAM_A_AF_VLD */
    CAM_REG_AF_BLK_0                                CAM_AF_BLK_0;                                    /* 0648, 0x1A004648, CAM_A_AF_BLK_0 */
    CAM_REG_AF_BLK_1                                CAM_AF_BLK_1;                                    /* 064C, 0x1A00464C, CAM_A_AF_BLK_1 */
    CAM_REG_AF_TH_2                                 CAM_AF_TH_2;                                     /* 0650, 0x1A004650, CAM_A_AF_TH_2 */
    CAM_REG_AF_FLT_9                                CAM_AF_FLT_9;                                    /* 0654, 0x1A004654, CAM_A_AF_FLT_9 */
    CAM_REG_AF_FLT_10                               CAM_AF_FLT_10;                                   /* 0658, 0x1A004658, CAM_A_AF_FLT_10 */
    CAM_REG_AF_FLT_11                               CAM_AF_FLT_11;                                   /* 065C, 0x1A00465C, CAM_A_AF_FLT_11 */
    CAM_REG_AF_FLT_12                               CAM_AF_FLT_12;                                   /* 0660, 0x1A004660, CAM_A_AF_FLT_12 */
    UINT32                                          rsv_0664[3];                                     /* 0664..066F, 0x1A004664..1A00466F */
    CAM_REG_AF_LUT_H0_0                             CAM_AF_LUT_H0_0;                                 /* 0670, 0x1A004670, CAM_A_AF_LUT_H0_0 */
    CAM_REG_AF_LUT_H0_1                             CAM_AF_LUT_H0_1;                                 /* 0674, 0x1A004674, CAM_A_AF_LUT_H0_1 */
    CAM_REG_AF_LUT_H0_2                             CAM_AF_LUT_H0_2;                                 /* 0678, 0x1A004678, CAM_A_AF_LUT_H0_2 */
    CAM_REG_AF_LUT_H0_3                             CAM_AF_LUT_H0_3;                                 /* 067C, 0x1A00467C, CAM_A_AF_LUT_H0_3 */
    CAM_REG_AF_LUT_H0_4                             CAM_AF_LUT_H0_4;                                 /* 0680, 0x1A004680, CAM_A_AF_LUT_H0_4 */
    UINT32                                          rsv_0684[3];                                     /* 0684..068F, 0x1A004684..1A00468F */
    CAM_REG_AF_LUT_H1_0                             CAM_AF_LUT_H1_0;                                 /* 0690, 0x1A004690, CAM_A_AF_LUT_H1_0 */
    CAM_REG_AF_LUT_H1_1                             CAM_AF_LUT_H1_1;                                 /* 0694, 0x1A004694, CAM_A_AF_LUT_H1_1 */
    CAM_REG_AF_LUT_H1_2                             CAM_AF_LUT_H1_2;                                 /* 0698, 0x1A004698, CAM_A_AF_LUT_H1_2 */
    CAM_REG_AF_LUT_H1_3                             CAM_AF_LUT_H1_3;                                 /* 069C, 0x1A00469C, CAM_A_AF_LUT_H1_3 */
    CAM_REG_AF_LUT_H1_4                             CAM_AF_LUT_H1_4;                                 /* 06A0, 0x1A0046A0, CAM_A_AF_LUT_H1_4 */
    UINT32                                          rsv_06A4[3];                                     /* 06A4..06AF, 0x1A0046A4..1A0046AF */
    CAM_REG_AF_LUT_V_0                              CAM_AF_LUT_V_0;                                  /* 06B0, 0x1A0046B0, CAM_A_AF_LUT_V_0 */
    CAM_REG_AF_LUT_V_1                              CAM_AF_LUT_V_1;                                  /* 06B4, 0x1A0046B4, CAM_A_AF_LUT_V_1 */
    CAM_REG_AF_LUT_V_2                              CAM_AF_LUT_V_2;                                  /* 06B8, 0x1A0046B8, CAM_A_AF_LUT_V_2 */
    CAM_REG_AF_LUT_V_3                              CAM_AF_LUT_V_3;                                  /* 06BC, 0x1A0046BC, CAM_A_AF_LUT_V_3 */
    CAM_REG_AF_LUT_V_4                              CAM_AF_LUT_V_4;                                  /* 06C0, 0x1A0046C0, CAM_A_AF_LUT_V_4 */
    CAM_REG_AF_CON2                                 CAM_AF_CON2;                                     /* 06C4, 0x1A0046C4, CAM_A_AF_CON2 */
    CAM_REG_AF_BLK_2                                CAM_AF_BLK_2;                                    /* 06C8, 0x1A0046C8, CAM_A_AF_BLK_2 */
    UINT32                                          rsv_06CC[17];                                    /* 06CC..070F, 0x1A0046CC..1A00470F */
    CAM_REG_HLR_CFG                                 CAM_HLR_CFG;                                     /* 0710, 0x1A004710, CAM_A_HLR_CFG */
    UINT32                                          rsv_0714;                                        /* 0714, 0x1A004714 */
    CAM_REG_HLR_GAIN                                CAM_HLR_GAIN;                                    /* 0718, 0x1A004718, CAM_A_HLR_GAIN */
    CAM_REG_HLR_GAIN_1                              CAM_HLR_GAIN_1;                                  /* 071C, 0x1A00471C, CAM_A_HLR_GAIN_1 */
    CAM_REG_HLR_OFST                                CAM_HLR_OFST;                                    /* 0720, 0x1A004720, CAM_A_HLR_OFST */
    CAM_REG_HLR_OFST_1                              CAM_HLR_OFST_1;                                  /* 0724, 0x1A004724, CAM_A_HLR_OFST_1 */
    CAM_REG_HLR_IVGN                                CAM_HLR_IVGN;                                    /* 0728, 0x1A004728, CAM_A_HLR_IVGN */
    CAM_REG_HLR_IVGN_1                              CAM_HLR_IVGN_1;                                  /* 072C, 0x1A00472C, CAM_A_HLR_IVGN_1 */
    CAM_REG_HLR_KC                                  CAM_HLR_KC;                                      /* 0730, 0x1A004730, CAM_A_HLR_KC */
    CAM_REG_HLR_CFG_1                               CAM_HLR_CFG_1;                                   /* 0734, 0x1A004734, CAM_A_HLR_CFG_1 */
    CAM_REG_HLR_SL_PARA                             CAM_HLR_SL_PARA;                                 /* 0738, 0x1A004738, CAM_A_HLR_SL_PARA */
    CAM_REG_HLR_SL_PARA_1                           CAM_HLR_SL_PARA_1;                               /* 073C, 0x1A00473C, CAM_A_HLR_SL_PARA_1 */
    UINT32                                          rsv_0740[20];                                    /* 0740..078F, 0x1A004740..1A00478F */
    CAM_REG_LCS25_CON                               CAM_LCS25_CON;                                   /* 0790, 0x1A004790, CAM_A_LCS25_CON */
    CAM_REG_LCS25_ST                                CAM_LCS25_ST;                                    /* 0794, 0x1A004794, CAM_A_LCS25_ST */
    CAM_REG_LCS25_AWS                               CAM_LCS25_AWS;                                   /* 0798, 0x1A004798, CAM_A_LCS25_AWS */
    CAM_REG_LCS25_FLR                               CAM_LCS25_FLR;                                   /* 079C, 0x1A00479C, CAM_A_LCS25_FLR */
    CAM_REG_LCS25_LRZR_1                            CAM_LCS25_LRZR_1;                                /* 07A0, 0x1A0047A0, CAM_A_LCS25_LRZR_1 */
    CAM_REG_LCS25_LRZR_2                            CAM_LCS25_LRZR_2;                                /* 07A4, 0x1A0047A4, CAM_A_LCS25_LRZR_2 */
    CAM_REG_LCS25_SATU_1                            CAM_LCS25_SATU_1;                                /* 07A8, 0x1A0047A8, CAM_A_LCS25_SATU_1 */
    CAM_REG_LCS25_SATU_2                            CAM_LCS25_SATU_2;                                /* 07AC, 0x1A0047AC, CAM_A_LCS25_SATU_2 */
    CAM_REG_LCS25_GAIN_1                            CAM_LCS25_GAIN_1;                                /* 07B0, 0x1A0047B0, CAM_A_LCS25_GAIN_1 */
    CAM_REG_LCS25_GAIN_2                            CAM_LCS25_GAIN_2;                                /* 07B4, 0x1A0047B4, CAM_A_LCS25_GAIN_2 */
    CAM_REG_LCS25_OFST_1                            CAM_LCS25_OFST_1;                                /* 07B8, 0x1A0047B8, CAM_A_LCS25_OFST_1 */
    CAM_REG_LCS25_OFST_2                            CAM_LCS25_OFST_2;                                /* 07BC, 0x1A0047BC, CAM_A_LCS25_OFST_2 */
    CAM_REG_LCS25_G2G_CNV_1                         CAM_LCS25_G2G_CNV_1;                             /* 07C0, 0x1A0047C0, CAM_A_LCS25_G2G_CNV_1 */
    CAM_REG_LCS25_G2G_CNV_2                         CAM_LCS25_G2G_CNV_2;                             /* 07C4, 0x1A0047C4, CAM_A_LCS25_G2G_CNV_2 */
    CAM_REG_LCS25_G2G_CNV_3                         CAM_LCS25_G2G_CNV_3;                             /* 07C8, 0x1A0047C8, CAM_A_LCS25_G2G_CNV_3 */
    CAM_REG_LCS25_G2G_CNV_4                         CAM_LCS25_G2G_CNV_4;                             /* 07CC, 0x1A0047CC, CAM_A_LCS25_G2G_CNV_4 */
    CAM_REG_LCS25_G2G_CNV_5                         CAM_LCS25_G2G_CNV_5;                             /* 07D0, 0x1A0047D0, CAM_A_LCS25_G2G_CNV_5 */
    CAM_REG_LCS25_LPF                               CAM_LCS25_LPF;                                   /* 07D4, 0x1A0047D4, CAM_A_LCS25_LPF */
    UINT32                                          rsv_07D8[10];                                    /* 07D8..07FF, 0x1A0047D8..1A0047FF */
    CAM_REG_RCP_CROP_CON1                           CAM_RCP_CROP_CON1;                               /* 0800, 0x1A004800, CAM_A_RCP_CROP_CON1 */
    CAM_REG_RCP_CROP_CON2                           CAM_RCP_CROP_CON2;                               /* 0804, 0x1A004804, CAM_A_RCP_CROP_CON2 */
    UINT32                                          rsv_0808[2];                                     /* 0808..080F, 0x1A004808..1A00480F */
    CAM_REG_SGG1_PGN                                CAM_SGG1_PGN;                                    /* 0810, 0x1A004810, CAM_A_SGG1_PGN */
    CAM_REG_SGG1_GMRC_1                             CAM_SGG1_GMRC_1;                                 /* 0814, 0x1A004814, CAM_A_SGG1_GMRC_1 */
    CAM_REG_SGG1_GMRC_2                             CAM_SGG1_GMRC_2;                                 /* 0818, 0x1A004818, CAM_A_SGG1_GMRC_2 */
    UINT32                                          rsv_081C;                                        /* 081C, 0x1A00481C */
    CAM_REG_QBN2_MODE                               CAM_QBN2_MODE;                                   /* 0820, 0x1A004820, CAM_A_QBN2_MODE */
    UINT32                                          rsv_0824[3];                                     /* 0824..082F, 0x1A004824..1A00482F */
    CAM_REG_AWB_WIN_ORG                             CAM_AWB_WIN_ORG;                                 /* 0830, 0x1A004830, CAM_A_AWB_WIN_ORG */
    CAM_REG_AWB_WIN_SIZE                            CAM_AWB_WIN_SIZE;                                /* 0834, 0x1A004834, CAM_A_AWB_WIN_SIZE */
    CAM_REG_AWB_WIN_PIT                             CAM_AWB_WIN_PIT;                                 /* 0838, 0x1A004838, CAM_A_AWB_WIN_PIT */
    CAM_REG_AWB_WIN_NUM                             CAM_AWB_WIN_NUM;                                 /* 083C, 0x1A00483C, CAM_A_AWB_WIN_NUM */
    CAM_REG_AWB_GAIN1_0                             CAM_AWB_GAIN1_0;                                 /* 0840, 0x1A004840, CAM_A_AWB_GAIN1_0 */
    CAM_REG_AWB_GAIN1_1                             CAM_AWB_GAIN1_1;                                 /* 0844, 0x1A004844, CAM_A_AWB_GAIN1_1 */
    CAM_REG_AWB_LMT1_0                              CAM_AWB_LMT1_0;                                  /* 0848, 0x1A004848, CAM_A_AWB_LMT1_0 */
    CAM_REG_AWB_LMT1_1                              CAM_AWB_LMT1_1;                                  /* 084C, 0x1A00484C, CAM_A_AWB_LMT1_1 */
    CAM_REG_AWB_LOW_THR                             CAM_AWB_LOW_THR;                                 /* 0850, 0x1A004850, CAM_A_AWB_LOW_THR */
    CAM_REG_AWB_HI_THR                              CAM_AWB_HI_THR;                                  /* 0854, 0x1A004854, CAM_A_AWB_HI_THR */
    CAM_REG_AWB_PIXEL_CNT0                          CAM_AWB_PIXEL_CNT0;                              /* 0858, 0x1A004858, CAM_A_AWB_PIXEL_CNT0 */
    CAM_REG_AWB_PIXEL_CNT1                          CAM_AWB_PIXEL_CNT1;                              /* 085C, 0x1A00485C, CAM_A_AWB_PIXEL_CNT1 */
    CAM_REG_AWB_PIXEL_CNT2                          CAM_AWB_PIXEL_CNT2;                              /* 0860, 0x1A004860, CAM_A_AWB_PIXEL_CNT2 */
    CAM_REG_AWB_ERR_THR                             CAM_AWB_ERR_THR;                                 /* 0864, 0x1A004864, CAM_A_AWB_ERR_THR */
    CAM_REG_AWB_ROT                                 CAM_AWB_ROT;                                     /* 0868, 0x1A004868, CAM_A_AWB_ROT */
    CAM_REG_AWB_L0_X                                CAM_AWB_L0_X;                                    /* 086C, 0x1A00486C, CAM_A_AWB_L0_X */
    CAM_REG_AWB_L0_Y                                CAM_AWB_L0_Y;                                    /* 0870, 0x1A004870, CAM_A_AWB_L0_Y */
    CAM_REG_AWB_L1_X                                CAM_AWB_L1_X;                                    /* 0874, 0x1A004874, CAM_A_AWB_L1_X */
    CAM_REG_AWB_L1_Y                                CAM_AWB_L1_Y;                                    /* 0878, 0x1A004878, CAM_A_AWB_L1_Y */
    CAM_REG_AWB_L2_X                                CAM_AWB_L2_X;                                    /* 087C, 0x1A00487C, CAM_A_AWB_L2_X */
    CAM_REG_AWB_L2_Y                                CAM_AWB_L2_Y;                                    /* 0880, 0x1A004880, CAM_A_AWB_L2_Y */
    CAM_REG_AWB_L3_X                                CAM_AWB_L3_X;                                    /* 0884, 0x1A004884, CAM_A_AWB_L3_X */
    CAM_REG_AWB_L3_Y                                CAM_AWB_L3_Y;                                    /* 0888, 0x1A004888, CAM_A_AWB_L3_Y */
    CAM_REG_AWB_L4_X                                CAM_AWB_L4_X;                                    /* 088C, 0x1A00488C, CAM_A_AWB_L4_X */
    CAM_REG_AWB_L4_Y                                CAM_AWB_L4_Y;                                    /* 0890, 0x1A004890, CAM_A_AWB_L4_Y */
    CAM_REG_AWB_L5_X                                CAM_AWB_L5_X;                                    /* 0894, 0x1A004894, CAM_A_AWB_L5_X */
    CAM_REG_AWB_L5_Y                                CAM_AWB_L5_Y;                                    /* 0898, 0x1A004898, CAM_A_AWB_L5_Y */
    CAM_REG_AWB_L6_X                                CAM_AWB_L6_X;                                    /* 089C, 0x1A00489C, CAM_A_AWB_L6_X */
    CAM_REG_AWB_L6_Y                                CAM_AWB_L6_Y;                                    /* 08A0, 0x1A0048A0, CAM_A_AWB_L6_Y */
    CAM_REG_AWB_L7_X                                CAM_AWB_L7_X;                                    /* 08A4, 0x1A0048A4, CAM_A_AWB_L7_X */
    CAM_REG_AWB_L7_Y                                CAM_AWB_L7_Y;                                    /* 08A8, 0x1A0048A8, CAM_A_AWB_L7_Y */
    CAM_REG_AWB_L8_X                                CAM_AWB_L8_X;                                    /* 08AC, 0x1A0048AC, CAM_A_AWB_L8_X */
    CAM_REG_AWB_L8_Y                                CAM_AWB_L8_Y;                                    /* 08B0, 0x1A0048B0, CAM_A_AWB_L8_Y */
    CAM_REG_AWB_L9_X                                CAM_AWB_L9_X;                                    /* 08B4, 0x1A0048B4, CAM_A_AWB_L9_X */
    CAM_REG_AWB_L9_Y                                CAM_AWB_L9_Y;                                    /* 08B8, 0x1A0048B8, CAM_A_AWB_L9_Y */
    CAM_REG_AWB_SPARE                               CAM_AWB_SPARE;                                   /* 08BC, 0x1A0048BC, CAM_A_AWB_SPARE */
    CAM_REG_AWB_MOTION_THR                          CAM_AWB_MOTION_THR;                              /* 08C0, 0x1A0048C0, CAM_A_AWB_MOTION_THR */
    CAM_REG_AWB_RC_CNV_0                            CAM_AWB_RC_CNV_0;                                /* 08C4, 0x1A0048C4, CAM_A_AWB_RC_CNV_0 */
    CAM_REG_AWB_RC_CNV_1                            CAM_AWB_RC_CNV_1;                                /* 08C8, 0x1A0048C8, CAM_A_AWB_RC_CNV_1 */
    CAM_REG_AWB_RC_CNV_2                            CAM_AWB_RC_CNV_2;                                /* 08CC, 0x1A0048CC, CAM_A_AWB_RC_CNV_2 */
    CAM_REG_AWB_RC_CNV_3                            CAM_AWB_RC_CNV_3;                                /* 08D0, 0x1A0048D0, CAM_A_AWB_RC_CNV_3 */
    CAM_REG_AWB_RC_CNV_4                            CAM_AWB_RC_CNV_4;                                /* 08D4, 0x1A0048D4, CAM_A_AWB_RC_CNV_4 */
    UINT32                                          rsv_08D8[10];                                    /* 08D8..08FF, 0x1A0048D8..1A0048FF */
    CAM_REG_AE_GAIN2_0                              CAM_AE_GAIN2_0;                                  /* 0900, 0x1A004900, CAM_A_AE_GAIN2_0 */
    CAM_REG_AE_GAIN2_1                              CAM_AE_GAIN2_1;                                  /* 0904, 0x1A004904, CAM_A_AE_GAIN2_1 */
    CAM_REG_AE_LMT2_0                               CAM_AE_LMT2_0;                                   /* 0908, 0x1A004908, CAM_A_AE_LMT2_0 */
    CAM_REG_AE_LMT2_1                               CAM_AE_LMT2_1;                                   /* 090C, 0x1A00490C, CAM_A_AE_LMT2_1 */
    CAM_REG_AE_RC_CNV_0                             CAM_AE_RC_CNV_0;                                 /* 0910, 0x1A004910, CAM_A_AE_RC_CNV_0 */
    CAM_REG_AE_RC_CNV_1                             CAM_AE_RC_CNV_1;                                 /* 0914, 0x1A004914, CAM_A_AE_RC_CNV_1 */
    CAM_REG_AE_RC_CNV_2                             CAM_AE_RC_CNV_2;                                 /* 0918, 0x1A004918, CAM_A_AE_RC_CNV_2 */
    CAM_REG_AE_RC_CNV_3                             CAM_AE_RC_CNV_3;                                 /* 091C, 0x1A00491C, CAM_A_AE_RC_CNV_3 */
    CAM_REG_AE_RC_CNV_4                             CAM_AE_RC_CNV_4;                                 /* 0920, 0x1A004920, CAM_A_AE_RC_CNV_4 */
    CAM_REG_AE_YGAMMA_0                             CAM_AE_YGAMMA_0;                                 /* 0924, 0x1A004924, CAM_A_AE_YGAMMA_0 */
    CAM_REG_AE_YGAMMA_1                             CAM_AE_YGAMMA_1;                                 /* 0928, 0x1A004928, CAM_A_AE_YGAMMA_1 */
    CAM_REG_AE_OVER_EXPO_CFG                        CAM_AE_OVER_EXPO_CFG;                            /* 092C, 0x1A00492C, CAM_A_AE_OVER_EXPO_CFG */
    CAM_REG_AE_PIX_HST_CTL                          CAM_AE_PIX_HST_CTL;                              /* 0930, 0x1A004930, CAM_A_AE_PIX_HST_CTL */
    CAM_REG_AE_PIX_HST_SET                          CAM_AE_PIX_HST_SET;                              /* 0934, 0x1A004934, CAM_A_AE_PIX_HST_SET */
    CAM_REG_AE_PIX_HST_SET_1                        CAM_AE_PIX_HST_SET_1;                            /* 0938, 0x1A004938, CAM_A_AE_PIX_HST_SET_1 */
    CAM_REG_AE_PIX_HST0_YRNG                        CAM_AE_PIX_HST0_YRNG;                            /* 093C, 0x1A00493C, CAM_A_AE_PIX_HST0_YRNG */
    CAM_REG_AE_PIX_HST0_XRNG                        CAM_AE_PIX_HST0_XRNG;                            /* 0940, 0x1A004940, CAM_A_AE_PIX_HST0_XRNG */
    CAM_REG_AE_PIX_HST1_YRNG                        CAM_AE_PIX_HST1_YRNG;                            /* 0944, 0x1A004944, CAM_A_AE_PIX_HST1_YRNG */
    CAM_REG_AE_PIX_HST1_XRNG                        CAM_AE_PIX_HST1_XRNG;                            /* 0948, 0x1A004948, CAM_A_AE_PIX_HST1_XRNG */
    CAM_REG_AE_PIX_HST2_YRNG                        CAM_AE_PIX_HST2_YRNG;                            /* 094C, 0x1A00494C, CAM_A_AE_PIX_HST2_YRNG */
    CAM_REG_AE_PIX_HST2_XRNG                        CAM_AE_PIX_HST2_XRNG;                            /* 0950, 0x1A004950, CAM_A_AE_PIX_HST2_XRNG */
    CAM_REG_AE_PIX_HST3_YRNG                        CAM_AE_PIX_HST3_YRNG;                            /* 0954, 0x1A004954, CAM_A_AE_PIX_HST3_YRNG */
    CAM_REG_AE_PIX_HST3_XRNG                        CAM_AE_PIX_HST3_XRNG;                            /* 0958, 0x1A004958, CAM_A_AE_PIX_HST3_XRNG */
    CAM_REG_AE_PIX_HST4_YRNG                        CAM_AE_PIX_HST4_YRNG;                            /* 095C, 0x1A00495C, CAM_A_AE_PIX_HST4_YRNG */
    CAM_REG_AE_PIX_HST4_XRNG                        CAM_AE_PIX_HST4_XRNG;                            /* 0960, 0x1A004960, CAM_A_AE_PIX_HST4_XRNG */
    CAM_REG_AE_PIX_HST5_YRNG                        CAM_AE_PIX_HST5_YRNG;                            /* 0964, 0x1A004964, CAM_A_AE_PIX_HST5_YRNG */
    CAM_REG_AE_PIX_HST5_XRNG                        CAM_AE_PIX_HST5_XRNG;                            /* 0968, 0x1A004968, CAM_A_AE_PIX_HST5_XRNG */
    CAM_REG_AE_STAT_EN                              CAM_AE_STAT_EN;                                  /* 096C, 0x1A00496C, CAM_A_AE_STAT_EN */
    CAM_REG_AE_YCOEF                                CAM_AE_YCOEF;                                    /* 0970, 0x1A004970, CAM_A_AE_YCOEF */
    CAM_REG_AE_CCU_HST_END_Y                        CAM_AE_CCU_HST_END_Y;                            /* 0974, 0x1A004974, CAM_A_AE_CCU_HST_END_Y */
    CAM_REG_AE_SPARE                                CAM_AE_SPARE;                                    /* 0978, 0x1A004978, CAM_A_AE_SPARE */
    UINT32                                          rsv_097C[21];                                    /* 097C..09CF, 0x1A00497C..1A0049CF */
    CAM_REG_QBN1_MODE                               CAM_QBN1_MODE;                                   /* 09D0, 0x1A0049D0, CAM_A_QBN1_MODE */
    UINT32                                          rsv_09D4[3];                                     /* 09D4..09DF, 0x1A0049D4..1A0049DF */
    CAM_REG_CPG_SATU_1                              CAM_CPG_SATU_1;                                  /* 09E0, 0x1A0049E0, CAM_A_CPG_SATU_1 */
    CAM_REG_CPG_SATU_2                              CAM_CPG_SATU_2;                                  /* 09E4, 0x1A0049E4, CAM_A_CPG_SATU_2 */
    CAM_REG_CPG_GAIN_1                              CAM_CPG_GAIN_1;                                  /* 09E8, 0x1A0049E8, CAM_A_CPG_GAIN_1 */
    CAM_REG_CPG_GAIN_2                              CAM_CPG_GAIN_2;                                  /* 09EC, 0x1A0049EC, CAM_A_CPG_GAIN_2 */
    CAM_REG_CPG_OFST_1                              CAM_CPG_OFST_1;                                  /* 09F0, 0x1A0049F0, CAM_A_CPG_OFST_1 */
    CAM_REG_CPG_OFST_2                              CAM_CPG_OFST_2;                                  /* 09F4, 0x1A0049F4, CAM_A_CPG_OFST_2 */
    UINT32                                          rsv_09F8[14];                                    /* 09F8..0A2F, 0x1A0049F8..1A004A2F */
    CAM_REG_VBN_GAIN                                CAM_VBN_GAIN;                                    /* 0A30, 0x1A004A30, CAM_A_VBN_GAIN */
    CAM_REG_VBN_OFST                                CAM_VBN_OFST;                                    /* 0A34, 0x1A004A34, CAM_A_VBN_OFST */
    CAM_REG_VBN_TYPE                                CAM_VBN_TYPE;                                    /* 0A38, 0x1A004A38, CAM_A_VBN_TYPE */
    CAM_REG_VBN_SPARE                               CAM_VBN_SPARE;                                   /* 0A3C, 0x1A004A3C, CAM_A_VBN_SPARE */
    UINT32                                          rsv_0A40[4];                                     /* 0A40..0A4F, 0x1A004A40..1A004A4F */
    CAM_REG_AMX_CTL                                 CAM_AMX_CTL;                                     /* 0A50, 0x1A004A50, CAM_A_AMX_CTL */
    CAM_REG_AMX_CROP                                CAM_AMX_CROP;                                    /* 0A54, 0x1A004A54, CAM_A_AMX_CROP */
    CAM_REG_AMX_VSIZE                               CAM_AMX_VSIZE;                                   /* 0A58, 0x1A004A58, CAM_A_AMX_VSIZE */
    UINT32                                          rsv_0A5C[5];                                     /* 0A5C..0A6F, 0x1A004A5C..1A004A6F */
    CAM_REG_BIN_CTL                                 CAM_BIN_CTL;                                     /* 0A70, 0x1A004A70, CAM_A_BIN_CTL */
    CAM_REG_BIN_FTH                                 CAM_BIN_FTH;                                     /* 0A74, 0x1A004A74, CAM_A_BIN_FTH */
    CAM_REG_BIN_SPARE                               CAM_BIN_SPARE;                                   /* 0A78, 0x1A004A78, CAM_A_BIN_SPARE */
    UINT32                                          rsv_0A7C[5];                                     /* 0A7C..0A8F, 0x1A004A7C..1A004A8F */
    CAM_REG_DBN_GAIN                                CAM_DBN_GAIN;                                    /* 0A90, 0x1A004A90, CAM_A_DBN_GAIN */
    CAM_REG_DBN_OFST                                CAM_DBN_OFST;                                    /* 0A94, 0x1A004A94, CAM_A_DBN_OFST */
    CAM_REG_DBN_SPARE                               CAM_DBN_SPARE;                                   /* 0A98, 0x1A004A98, CAM_A_DBN_SPARE */
    UINT32                                          rsv_0A9C;                                        /* 0A9C, 0x1A004A9C */
    CAM_REG_PBN_TYPE                                CAM_PBN_TYPE;                                    /* 0AA0, 0x1A004AA0, CAM_A_PBN_TYPE */
    CAM_REG_PBN_LST                                 CAM_PBN_LST;                                     /* 0AA4, 0x1A004AA4, CAM_A_PBN_LST */
    CAM_REG_PBN_VSIZE                               CAM_PBN_VSIZE;                                   /* 0AA8, 0x1A004AA8, CAM_A_PBN_VSIZE */
    UINT32                                          rsv_0AAC;                                        /* 0AAC, 0x1A004AAC */
    CAM_REG_RCP3_CROP_CON1                          CAM_RCP3_CROP_CON1;                              /* 0AB0, 0x1A004AB0, CAM_A_RCP3_CROP_CON1 */
    CAM_REG_RCP3_CROP_CON2                          CAM_RCP3_CROP_CON2;                              /* 0AB4, 0x1A004AB4, CAM_A_RCP3_CROP_CON2 */
    UINT32                                          rsv_0AB8[2];                                     /* 0AB8..0ABF, 0x1A004AB8..1A004ABF */
    CAM_REG_DBS_SIGMA                               CAM_DBS_SIGMA;                                   /* 0AC0, 0x1A004AC0, CAM_A_DBS_SIGMA */
    CAM_REG_DBS_BSTBL_0                             CAM_DBS_BSTBL_0;                                 /* 0AC4, 0x1A004AC4, CAM_A_DBS_BSTBL_0 */
    CAM_REG_DBS_BSTBL_1                             CAM_DBS_BSTBL_1;                                 /* 0AC8, 0x1A004AC8, CAM_A_DBS_BSTBL_1 */
    CAM_REG_DBS_BSTBL_2                             CAM_DBS_BSTBL_2;                                 /* 0ACC, 0x1A004ACC, CAM_A_DBS_BSTBL_2 */
    CAM_REG_DBS_BSTBL_3                             CAM_DBS_BSTBL_3;                                 /* 0AD0, 0x1A004AD0, CAM_A_DBS_BSTBL_3 */
    CAM_REG_DBS_CTL                                 CAM_DBS_CTL;                                     /* 0AD4, 0x1A004AD4, CAM_A_DBS_CTL */
    CAM_REG_DBS_CTL_2                               CAM_DBS_CTL_2;                                   /* 0AD8, 0x1A004AD8, CAM_A_DBS_CTL_2 */
    CAM_REG_DBS_SIGMA_2                             CAM_DBS_SIGMA_2;                                 /* 0ADC, 0x1A004ADC, CAM_A_DBS_SIGMA_2 */
    CAM_REG_DBS_YGN                                 CAM_DBS_YGN;                                     /* 0AE0, 0x1A004AE0, CAM_A_DBS_YGN */
    CAM_REG_DBS_SL_Y12                              CAM_DBS_SL_Y12;                                  /* 0AE4, 0x1A004AE4, CAM_A_DBS_SL_Y12 */
    CAM_REG_DBS_SL_Y34                              CAM_DBS_SL_Y34;                                  /* 0AE8, 0x1A004AE8, CAM_A_DBS_SL_Y34 */
    CAM_REG_DBS_SL_G12                              CAM_DBS_SL_G12;                                  /* 0AEC, 0x1A004AEC, CAM_A_DBS_SL_G12 */
    CAM_REG_DBS_SL_G34                              CAM_DBS_SL_G34;                                  /* 0AF0, 0x1A004AF0, CAM_A_DBS_SL_G34 */
    UINT32                                          rsv_0AF4[3];                                     /* 0AF4..0AFF, 0x1A004AF4..1A004AFF */
    CAM_REG_SL2F_CEN                                CAM_SL2F_CEN;                                    /* 0B00, 0x1A004B00, CAM_A_SL2F_CEN */
    CAM_REG_SL2F_RR_CON0                            CAM_SL2F_RR_CON0;                                /* 0B04, 0x1A004B04, CAM_A_SL2F_RR_CON0 */
    CAM_REG_SL2F_RR_CON1                            CAM_SL2F_RR_CON1;                                /* 0B08, 0x1A004B08, CAM_A_SL2F_RR_CON1 */
    CAM_REG_SL2F_GAIN                               CAM_SL2F_GAIN;                                   /* 0B0C, 0x1A004B0C, CAM_A_SL2F_GAIN */
    CAM_REG_SL2F_RZ                                 CAM_SL2F_RZ;                                     /* 0B10, 0x1A004B10, CAM_A_SL2F_RZ */
    CAM_REG_SL2F_XOFF                               CAM_SL2F_XOFF;                                   /* 0B14, 0x1A004B14, CAM_A_SL2F_XOFF */
    CAM_REG_SL2F_YOFF                               CAM_SL2F_YOFF;                                   /* 0B18, 0x1A004B18, CAM_A_SL2F_YOFF */
    CAM_REG_SL2F_SLP_CON0                           CAM_SL2F_SLP_CON0;                               /* 0B1C, 0x1A004B1C, CAM_A_SL2F_SLP_CON0 */
    CAM_REG_SL2F_SLP_CON1                           CAM_SL2F_SLP_CON1;                               /* 0B20, 0x1A004B20, CAM_A_SL2F_SLP_CON1 */
    CAM_REG_SL2F_SLP_CON2                           CAM_SL2F_SLP_CON2;                               /* 0B24, 0x1A004B24, CAM_A_SL2F_SLP_CON2 */
    CAM_REG_SL2F_SLP_CON3                           CAM_SL2F_SLP_CON3;                               /* 0B28, 0x1A004B28, CAM_A_SL2F_SLP_CON3 */
    CAM_REG_SL2F_SIZE                               CAM_SL2F_SIZE;                                   /* 0B2C, 0x1A004B2C, CAM_A_SL2F_SIZE */
    UINT32                                          rsv_0B30[4];                                     /* 0B30..0B3F, 0x1A004B30..1A004B3F */
    CAM_REG_SL2J_CEN                                CAM_SL2J_CEN;                                    /* 0B40, 0x1A004B40, CAM_A_SL2J_CEN */
    CAM_REG_SL2J_RR_CON0                            CAM_SL2J_RR_CON0;                                /* 0B44, 0x1A004B44, CAM_A_SL2J_RR_CON0 */
    CAM_REG_SL2J_RR_CON1                            CAM_SL2J_RR_CON1;                                /* 0B48, 0x1A004B48, CAM_A_SL2J_RR_CON1 */
    CAM_REG_SL2J_GAIN                               CAM_SL2J_GAIN;                                   /* 0B4C, 0x1A004B4C, CAM_A_SL2J_GAIN */
    CAM_REG_SL2J_RZ                                 CAM_SL2J_RZ;                                     /* 0B50, 0x1A004B50, CAM_A_SL2J_RZ */
    CAM_REG_SL2J_XOFF                               CAM_SL2J_XOFF;                                   /* 0B54, 0x1A004B54, CAM_A_SL2J_XOFF */
    CAM_REG_SL2J_YOFF                               CAM_SL2J_YOFF;                                   /* 0B58, 0x1A004B58, CAM_A_SL2J_YOFF */
    CAM_REG_SL2J_SLP_CON0                           CAM_SL2J_SLP_CON0;                               /* 0B5C, 0x1A004B5C, CAM_A_SL2J_SLP_CON0 */
    CAM_REG_SL2J_SLP_CON1                           CAM_SL2J_SLP_CON1;                               /* 0B60, 0x1A004B60, CAM_A_SL2J_SLP_CON1 */
    CAM_REG_SL2J_SLP_CON2                           CAM_SL2J_SLP_CON2;                               /* 0B64, 0x1A004B64, CAM_A_SL2J_SLP_CON2 */
    CAM_REG_SL2J_SLP_CON3                           CAM_SL2J_SLP_CON3;                               /* 0B68, 0x1A004B68, CAM_A_SL2J_SLP_CON3 */
    CAM_REG_SL2J_SIZE                               CAM_SL2J_SIZE;                                   /* 0B6C, 0x1A004B6C, CAM_A_SL2J_SIZE */
    UINT32                                          rsv_0B70[4];                                     /* 0B70..0B7F, 0x1A004B70..1A004B7F */
    CAM_REG_PCP_CROP_CON1                           CAM_PCP_CROP_CON1;                               /* 0B80, 0x1A004B80, CAM_A_PCP_CROP_CON1 */
    CAM_REG_PCP_CROP_CON2                           CAM_PCP_CROP_CON2;                               /* 0B84, 0x1A004B84, CAM_A_PCP_CROP_CON2 */
    UINT32                                          rsv_0B88[2];                                     /* 0B88..0B8F, 0x1A004B88..1A004B8F */
    CAM_REG_SGG2_PGN                                CAM_SGG2_PGN;                                    /* 0B90, 0x1A004B90, CAM_A_SGG2_PGN */
    CAM_REG_SGG2_GMRC_1                             CAM_SGG2_GMRC_1;                                 /* 0B94, 0x1A004B94, CAM_A_SGG2_GMRC_1 */
    CAM_REG_SGG2_GMRC_2                             CAM_SGG2_GMRC_2;                                 /* 0B98, 0x1A004B98, CAM_A_SGG2_GMRC_2 */
    UINT32                                          rsv_0B9C;                                        /* 0B9C, 0x1A004B9C */
    CAM_REG_PSB_CON                                 CAM_PSB_CON;                                     /* 0BA0, 0x1A004BA0, CAM_A_PSB_CON */
    CAM_REG_PSB_SIZE                                CAM_PSB_SIZE;                                    /* 0BA4, 0x1A004BA4, CAM_A_PSB_SIZE */
    UINT32                                          rsv_0BA8[2];                                     /* 0BA8..0BAF, 0x1A004BA8..1A004BAF */
    CAM_REG_PDE_TBLI1                               CAM_PDE_TBLI1;                                   /* 0BB0, 0x1A004BB0, CAM_A_PDE_TBLI1 */
    CAM_REG_PDE_CTL                                 CAM_PDE_CTL;                                     /* 0BB4, 0x1A004BB4, CAM_A_PDE_CTL */
    UINT32                                          rsv_0BB8[2];                                     /* 0BB8..0BBF, 0x1A004BB8..1A004BBF */
    CAM_REG_RMB_MODE                                CAM_RMB_MODE;                                    /* 0BC0, 0x1A004BC0, CAM_A_RMB_MODE */
    UINT32                                          rsv_0BC4[3];                                     /* 0BC4..0BCF, 0x1A004BC4..1A004BCF */
    CAM_REG_PS_AWB_WIN_ORG                          CAM_PS_AWB_WIN_ORG;                              /* 0BD0, 0x1A004BD0, CAM_A_PS_AWB_WIN_ORG */
    CAM_REG_PS_AWB_WIN_SIZE                         CAM_PS_AWB_WIN_SIZE;                             /* 0BD4, 0x1A004BD4, CAM_A_PS_AWB_WIN_SIZE */
    CAM_REG_PS_AWB_WIN_PIT                          CAM_PS_AWB_WIN_PIT;                              /* 0BD8, 0x1A004BD8, CAM_A_PS_AWB_WIN_PIT */
    CAM_REG_PS_AWB_WIN_NUM                          CAM_PS_AWB_WIN_NUM;                              /* 0BDC, 0x1A004BDC, CAM_A_PS_AWB_WIN_NUM */
    CAM_REG_PS_AWB_PIXEL_CNT0                       CAM_PS_AWB_PIXEL_CNT0;                           /* 0BE0, 0x1A004BE0, CAM_A_PS_AWB_PIXEL_CNT0 */
    CAM_REG_PS_AWB_PIXEL_CNT1                       CAM_PS_AWB_PIXEL_CNT1;                           /* 0BE4, 0x1A004BE4, CAM_A_PS_AWB_PIXEL_CNT1 */
    CAM_REG_PS_AWB_PIXEL_CNT2                       CAM_PS_AWB_PIXEL_CNT2;                           /* 0BE8, 0x1A004BE8, CAM_A_PS_AWB_PIXEL_CNT2 */
    CAM_REG_PS_AWB_PIXEL_CNT3                       CAM_PS_AWB_PIXEL_CNT3;                           /* 0BEC, 0x1A004BEC, CAM_A_PS_AWB_PIXEL_CNT3 */
    CAM_REG_PS_AE_YCOEF0                            CAM_PS_AE_YCOEF0;                                /* 0BF0, 0x1A004BF0, CAM_A_PS_AE_YCOEF0 */
    CAM_REG_PS_AE_YCOEF1                            CAM_PS_AE_YCOEF1;                                /* 0BF4, 0x1A004BF4, CAM_A_PS_AE_YCOEF1 */
    CAM_REG_PS_DATA_TYPE                            CAM_PS_DATA_TYPE;                                /* 0BF8, 0x1A004BF8, CAM_A_PS_DATA_TYPE */
    CAM_REG_PS_HST_CFG                              CAM_PS_HST_CFG;                                  /* 0BFC, 0x1A004BFC, CAM_A_PS_HST_CFG */
    CAM_REG_PS_HST_ROI_X                            CAM_PS_HST_ROI_X;                                /* 0C00, 0x1A004C00, CAM_A_PS_HST_ROI_X */
    CAM_REG_PS_HST_ROI_Y                            CAM_PS_HST_ROI_Y;                                /* 0C04, 0x1A004C04, CAM_A_PS_HST_ROI_Y */
    UINT32                                          rsv_0C08[10];                                    /* 0C08..0C2F, 0x1A004C08..1A004C2F */
    CAM_REG_QBN3_MODE                               CAM_QBN3_MODE;                                   /* 0C30, 0x1A004C30, CAM_A_QBN3_MODE */
    UINT32                                          rsv_0C34[3];                                     /* 0C34..0C3F, 0x1A004C34..1A004C3F */
    CAM_REG_SGG3_PGN                                CAM_SGG3_PGN;                                    /* 0C40, 0x1A004C40, CAM_A_SGG3_PGN */
    CAM_REG_SGG3_GMRC_1                             CAM_SGG3_GMRC_1;                                 /* 0C44, 0x1A004C44, CAM_A_SGG3_GMRC_1 */
    CAM_REG_SGG3_GMRC_2                             CAM_SGG3_GMRC_2;                                 /* 0C48, 0x1A004C48, CAM_A_SGG3_GMRC_2 */
    UINT32                                          rsv_0C4C;                                        /* 0C4C, 0x1A004C4C */
    CAM_REG_FLK_A_CON                               CAM_FLK_A_CON;                                   /* 0C50, 0x1A004C50, CAM_A_FLK_A_CON */
    CAM_REG_FLK_A_OFST                              CAM_FLK_A_OFST;                                  /* 0C54, 0x1A004C54, CAM_A_FLK_A_OFST */
    CAM_REG_FLK_A_SIZE                              CAM_FLK_A_SIZE;                                  /* 0C58, 0x1A004C58, CAM_A_FLK_A_SIZE */
    CAM_REG_FLK_A_NUM                               CAM_FLK_A_NUM;                                   /* 0C5C, 0x1A004C5C, CAM_A_FLK_A_NUM */
    UINT32                                          rsv_0C60[4];                                     /* 0C60..0C6F, 0x1A004C60..1A004C6F */
    CAM_REG_EIS_PREP_ME_CTRL1                       CAM_EIS_PREP_ME_CTRL1;                           /* 0C70, 0x1A004C70, CAM_A_EIS_PREP_ME_CTRL1 */
    CAM_REG_EIS_PREP_ME_CTRL2                       CAM_EIS_PREP_ME_CTRL2;                           /* 0C74, 0x1A004C74, CAM_A_EIS_PREP_ME_CTRL2 */
    CAM_REG_EIS_LMV_TH                              CAM_EIS_LMV_TH;                                  /* 0C78, 0x1A004C78, CAM_A_EIS_LMV_TH */
    CAM_REG_EIS_FL_OFFSET                           CAM_EIS_FL_OFFSET;                               /* 0C7C, 0x1A004C7C, CAM_A_EIS_FL_OFFSET */
    CAM_REG_EIS_MB_OFFSET                           CAM_EIS_MB_OFFSET;                               /* 0C80, 0x1A004C80, CAM_A_EIS_MB_OFFSET */
    CAM_REG_EIS_MB_INTERVAL                         CAM_EIS_MB_INTERVAL;                             /* 0C84, 0x1A004C84, CAM_A_EIS_MB_INTERVAL */
    CAM_REG_EIS_GMV                                 CAM_EIS_GMV;                                     /* 0C88, 0x1A004C88, CAM_A_EIS_GMV */
    CAM_REG_EIS_ERR_CTRL                            CAM_EIS_ERR_CTRL;                                /* 0C8C, 0x1A004C8C, CAM_A_EIS_ERR_CTRL */
    CAM_REG_EIS_IMAGE_CTRL                          CAM_EIS_IMAGE_CTRL;                              /* 0C90, 0x1A004C90, CAM_A_EIS_IMAGE_CTRL */
    UINT32                                          rsv_0C94[7];                                     /* 0C94..0CAF, 0x1A004C94..1A004CAF */
    CAM_REG_SGG5_PGN                                CAM_SGG5_PGN;                                    /* 0CB0, 0x1A004CB0, CAM_A_SGG5_PGN */
    CAM_REG_SGG5_GMRC_1                             CAM_SGG5_GMRC_1;                                 /* 0CB4, 0x1A004CB4, CAM_A_SGG5_GMRC_1 */
    CAM_REG_SGG5_GMRC_2                             CAM_SGG5_GMRC_2;                                 /* 0CB8, 0x1A004CB8, CAM_A_SGG5_GMRC_2 */
    UINT32                                          rsv_0CBC;                                        /* 0CBC, 0x1A004CBC */
    CAM_REG_HDS_MODE                                CAM_HDS_MODE;                                    /* 0CC0, 0x1A004CC0, CAM_A_HDS_MODE */
    UINT32                                          rsv_0CC4[7];                                     /* 0CC4..0CDF, 0x1A004CC4..1A004CDF */
    CAM_REG_RSS_A_CONTROL                           CAM_RSS_A_CONTROL;                               /* 0CE0, 0x1A004CE0, CAM_A_RSS_A_CONTROL */
    CAM_REG_RSS_A_IN_IMG                            CAM_RSS_A_IN_IMG;                                /* 0CE4, 0x1A004CE4, CAM_A_RSS_A_IN_IMG */
    CAM_REG_RSS_A_OUT_IMG                           CAM_RSS_A_OUT_IMG;                               /* 0CE8, 0x1A004CE8, CAM_A_RSS_A_OUT_IMG */
    CAM_REG_RSS_A_HORI_STEP                         CAM_RSS_A_HORI_STEP;                             /* 0CEC, 0x1A004CEC, CAM_A_RSS_A_HORI_STEP */
    CAM_REG_RSS_A_VERT_STEP                         CAM_RSS_A_VERT_STEP;                             /* 0CF0, 0x1A004CF0, CAM_A_RSS_A_VERT_STEP */
    CAM_REG_RSS_A_HORI_INT_OFST                     CAM_RSS_A_HORI_INT_OFST;                         /* 0CF4, 0x1A004CF4, CAM_A_RSS_A_HORI_INT_OFST */
    CAM_REG_RSS_A_HORI_SUB_OFST                     CAM_RSS_A_HORI_SUB_OFST;                         /* 0CF8, 0x1A004CF8, CAM_A_RSS_A_HORI_SUB_OFST */
    CAM_REG_RSS_A_VERT_INT_OFST                     CAM_RSS_A_VERT_INT_OFST;                         /* 0CFC, 0x1A004CFC, CAM_A_RSS_A_VERT_INT_OFST */
    CAM_REG_RSS_A_VERT_SUB_OFST                     CAM_RSS_A_VERT_SUB_OFST;                         /* 0D00, 0x1A004D00, CAM_A_RSS_A_VERT_SUB_OFST */
    UINT32                                          rsv_0D04[7];                                     /* 0D04..0D1F, 0x1A004D04..1A004D1F */
    CAM_REG_UFE_CON                                 CAM_UFE_CON;                                     /* 0D20, 0x1A004D20, CAM_A_UFE_CON */
    UINT32                                          rsv_0D24[11];                                    /* 0D24..0D4F, 0x1A004D24..1A004D4F */
    CAM_REG_ADBS_CTL                                CAM_ADBS_CTL;                                    /* 0D50, 0x1A004D50, CAM_A_ADBS_CTL */
    CAM_REG_ADBS_GRAY_BLD_0                         CAM_ADBS_GRAY_BLD_0;                             /* 0D54, 0x1A004D54, CAM_A_ADBS_GRAY_BLD_0 */
    CAM_REG_ADBS_GRAY_BLD_1                         CAM_ADBS_GRAY_BLD_1;                             /* 0D58, 0x1A004D58, CAM_A_ADBS_GRAY_BLD_1 */
    CAM_REG_ADBS_BIAS_LUT_R0                        CAM_ADBS_BIAS_LUT_R0;                            /* 0D5C, 0x1A004D5C, CAM_A_ADBS_BIAS_LUT_R0 */
    CAM_REG_ADBS_BIAS_LUT_R1                        CAM_ADBS_BIAS_LUT_R1;                            /* 0D60, 0x1A004D60, CAM_A_ADBS_BIAS_LUT_R1 */
    CAM_REG_ADBS_BIAS_LUT_R2                        CAM_ADBS_BIAS_LUT_R2;                            /* 0D64, 0x1A004D64, CAM_A_ADBS_BIAS_LUT_R2 */
    CAM_REG_ADBS_BIAS_LUT_R3                        CAM_ADBS_BIAS_LUT_R3;                            /* 0D68, 0x1A004D68, CAM_A_ADBS_BIAS_LUT_R3 */
    CAM_REG_ADBS_BIAS_LUT_G0                        CAM_ADBS_BIAS_LUT_G0;                            /* 0D6C, 0x1A004D6C, CAM_A_ADBS_BIAS_LUT_G0 */
    CAM_REG_ADBS_BIAS_LUT_G1                        CAM_ADBS_BIAS_LUT_G1;                            /* 0D70, 0x1A004D70, CAM_A_ADBS_BIAS_LUT_G1 */
    CAM_REG_ADBS_BIAS_LUT_G2                        CAM_ADBS_BIAS_LUT_G2;                            /* 0D74, 0x1A004D74, CAM_A_ADBS_BIAS_LUT_G2 */
    CAM_REG_ADBS_BIAS_LUT_G3                        CAM_ADBS_BIAS_LUT_G3;                            /* 0D78, 0x1A004D78, CAM_A_ADBS_BIAS_LUT_G3 */
    CAM_REG_ADBS_BIAS_LUT_B0                        CAM_ADBS_BIAS_LUT_B0;                            /* 0D7C, 0x1A004D7C, CAM_A_ADBS_BIAS_LUT_B0 */
    CAM_REG_ADBS_BIAS_LUT_B1                        CAM_ADBS_BIAS_LUT_B1;                            /* 0D80, 0x1A004D80, CAM_A_ADBS_BIAS_LUT_B1 */
    CAM_REG_ADBS_BIAS_LUT_B2                        CAM_ADBS_BIAS_LUT_B2;                            /* 0D84, 0x1A004D84, CAM_A_ADBS_BIAS_LUT_B2 */
    CAM_REG_ADBS_BIAS_LUT_B3                        CAM_ADBS_BIAS_LUT_B3;                            /* 0D88, 0x1A004D88, CAM_A_ADBS_BIAS_LUT_B3 */
    CAM_REG_ADBS_GAIN_0                             CAM_ADBS_GAIN_0;                                 /* 0D8C, 0x1A004D8C, CAM_A_ADBS_GAIN_0 */
    CAM_REG_ADBS_GAIN_1                             CAM_ADBS_GAIN_1;                                 /* 0D90, 0x1A004D90, CAM_A_ADBS_GAIN_1 */
    CAM_REG_ADBS_IVGN_0                             CAM_ADBS_IVGN_0;                                 /* 0D94, 0x1A004D94, CAM_A_ADBS_IVGN_0 */
    CAM_REG_ADBS_IVGN_1                             CAM_ADBS_IVGN_1;                                 /* 0D98, 0x1A004D98, CAM_A_ADBS_IVGN_1 */
    CAM_REG_ADBS_HDR                                CAM_ADBS_HDR;                                    /* 0D9C, 0x1A004D9C, CAM_A_ADBS_HDR */
    CAM_REG_ADBS_CMDL_ONLY_1                        CAM_ADBS_CMDL_ONLY_1;                            /* 0DA0, 0x1A004DA0, CAM_A_ADBS_CMDL_ONLY_1 */
    UINT32                                          rsv_0DA4[3];                                     /* 0DA4..0DAF, 0x1A004DA4..1A004DAF */
    CAM_REG_DCPN_HDR_EN                             CAM_DCPN_HDR_EN;                                 /* 0DB0, 0x1A004DB0, CAM_A_DCPN_HDR_EN */
    CAM_REG_DCPN_IN_IMG_SIZE                        CAM_DCPN_IN_IMG_SIZE;                            /* 0DB4, 0x1A004DB4, CAM_A_DCPN_IN_IMG_SIZE */
    CAM_REG_DCPN_ALGO_PARAM1                        CAM_DCPN_ALGO_PARAM1;                            /* 0DB8, 0x1A004DB8, CAM_A_DCPN_ALGO_PARAM1 */
    CAM_REG_DCPN_ALGO_PARAM2                        CAM_DCPN_ALGO_PARAM2;                            /* 0DBC, 0x1A004DBC, CAM_A_DCPN_ALGO_PARAM2 */
    CAM_REG_DCPN_GTM_X0                             CAM_DCPN_GTM_X0;                                 /* 0DC0, 0x1A004DC0, CAM_A_DCPN_GTM_X0 */
    CAM_REG_DCPN_GTM_Y0                             CAM_DCPN_GTM_Y0;                                 /* 0DC4, 0x1A004DC4, CAM_A_DCPN_GTM_Y0 */
    CAM_REG_DCPN_GTM_S0                             CAM_DCPN_GTM_S0;                                 /* 0DC8, 0x1A004DC8, CAM_A_DCPN_GTM_S0 */
    CAM_REG_DCPN_GTM_S1                             CAM_DCPN_GTM_S1;                                 /* 0DCC, 0x1A004DCC, CAM_A_DCPN_GTM_S1 */
    UINT32                                          rsv_0DD0[8];                                     /* 0DD0..0DEF, 0x1A004DD0..1A004DEF */
    CAM_REG_CPN_HDR_CTL_EN                          CAM_CPN_HDR_CTL_EN;                              /* 0DF0, 0x1A004DF0, CAM_A_CPN_HDR_CTL_EN */
    CAM_REG_CPN_IN_IMG_SIZE                         CAM_CPN_IN_IMG_SIZE;                             /* 0DF4, 0x1A004DF4, CAM_A_CPN_IN_IMG_SIZE */
    CAM_REG_CPN_ALGO_PARAM1                         CAM_CPN_ALGO_PARAM1;                             /* 0DF8, 0x1A004DF8, CAM_A_CPN_ALGO_PARAM1 */
    CAM_REG_CPN_ALGO_PARAM2                         CAM_CPN_ALGO_PARAM2;                             /* 0DFC, 0x1A004DFC, CAM_A_CPN_ALGO_PARAM2 */
    CAM_REG_CPN_GTM_X0X1                            CAM_CPN_GTM_X0X1;                                /* 0E00, 0x1A004E00, CAM_A_CPN_GTM_X0X1 */
    CAM_REG_CPN_GTM_X2X3                            CAM_CPN_GTM_X2X3;                                /* 0E04, 0x1A004E04, CAM_A_CPN_GTM_X2X3 */
    CAM_REG_CPN_GTM_X4X5                            CAM_CPN_GTM_X4X5;                                /* 0E08, 0x1A004E08, CAM_A_CPN_GTM_X4X5 */
    CAM_REG_CPN_GTM_X6                              CAM_CPN_GTM_X6;                                  /* 0E0C, 0x1A004E0C, CAM_A_CPN_GTM_X6 */
    CAM_REG_CPN_GTM_Y0Y1                            CAM_CPN_GTM_Y0Y1;                                /* 0E10, 0x1A004E10, CAM_A_CPN_GTM_Y0Y1 */
    CAM_REG_CPN_GTM_Y2Y3                            CAM_CPN_GTM_Y2Y3;                                /* 0E14, 0x1A004E14, CAM_A_CPN_GTM_Y2Y3 */
    CAM_REG_CPN_GTM_Y4Y5                            CAM_CPN_GTM_Y4Y5;                                /* 0E18, 0x1A004E18, CAM_A_CPN_GTM_Y4Y5 */
    CAM_REG_CPN_GTM_Y6                              CAM_CPN_GTM_Y6;                                  /* 0E1C, 0x1A004E1C, CAM_A_CPN_GTM_Y6 */
    CAM_REG_CPN_GTM_S0S1                            CAM_CPN_GTM_S0S1;                                /* 0E20, 0x1A004E20, CAM_A_CPN_GTM_S0S1 */
    CAM_REG_CPN_GTM_S2S3                            CAM_CPN_GTM_S2S3;                                /* 0E24, 0x1A004E24, CAM_A_CPN_GTM_S2S3 */
    CAM_REG_CPN_GTM_S4S5                            CAM_CPN_GTM_S4S5;                                /* 0E28, 0x1A004E28, CAM_A_CPN_GTM_S4S5 */
    CAM_REG_CPN_GTM_S6S7                            CAM_CPN_GTM_S6S7;                                /* 0E2C, 0x1A004E2C, CAM_A_CPN_GTM_S6S7 */
    UINT32                                          rsv_0E30[92];                                    /* 0E30..0F9F, 0x1A004E30..1A004F9F */
    CAM_REG_SGM_R_OFST_TABLE0_3                     CAM_SGM_R_OFST_TABLE0_3;                         /* 0FA0, 0x1A004FA0, CAM_A_SGM_R_OFST_TABLE0_3 */
    CAM_REG_SGM_R_OFST_TABLE4_7                     CAM_SGM_R_OFST_TABLE4_7;                         /* 0FA4, 0x1A004FA4, CAM_A_SGM_R_OFST_TABLE4_7 */
    CAM_REG_SGM_R_OFST_TABLE8_11                    CAM_SGM_R_OFST_TABLE8_11;                        /* 0FA8, 0x1A004FA8, CAM_A_SGM_R_OFST_TABLE8_11 */
    CAM_REG_SGM_R_OFST_TABLE12_15                   CAM_SGM_R_OFST_TABLE12_15;                       /* 0FAC, 0x1A004FAC, CAM_A_SGM_R_OFST_TABLE12_15 */
    CAM_REG_SGM_G_OFST_TABLE0_3                     CAM_SGM_G_OFST_TABLE0_3;                         /* 0FB0, 0x1A004FB0, CAM_A_SGM_G_OFST_TABLE0_3 */
    CAM_REG_SGM_G_OFST_TABLE4_7                     CAM_SGM_G_OFST_TABLE4_7;                         /* 0FB4, 0x1A004FB4, CAM_A_SGM_G_OFST_TABLE4_7 */
    CAM_REG_SGM_G_OFST_TABLE8_11                    CAM_SGM_G_OFST_TABLE8_11;                        /* 0FB8, 0x1A004FB8, CAM_A_SGM_G_OFST_TABLE8_11 */
    CAM_REG_SGM_G_OFST_TABLE12_15                   CAM_SGM_G_OFST_TABLE12_15;                       /* 0FBC, 0x1A004FBC, CAM_A_SGM_G_OFST_TABLE12_15 */
    CAM_REG_SGM_B_OFST_TABLE0_3                     CAM_SGM_B_OFST_TABLE0_3;                         /* 0FC0, 0x1A004FC0, CAM_A_SGM_B_OFST_TABLE0_3 */
    CAM_REG_SGM_B_OFST_TABLE4_7                     CAM_SGM_B_OFST_TABLE4_7;                         /* 0FC4, 0x1A004FC4, CAM_A_SGM_B_OFST_TABLE4_7 */
    CAM_REG_SGM_B_OFST_TABLE8_11                    CAM_SGM_B_OFST_TABLE8_11;                        /* 0FC8, 0x1A004FC8, CAM_A_SGM_B_OFST_TABLE8_11 */
    CAM_REG_SGM_B_OFST_TABLE12_15                   CAM_SGM_B_OFST_TABLE12_15;                       /* 0FCC, 0x1A004FCC, CAM_A_SGM_B_OFST_TABLE12_15 */
    UINT32                                          rsv_0FD0[12];                                    /* 0FD0..0FFF, 0x1A004FD0..1A004FFF */
    CAM_REG_DMA_SOFT_RSTSTAT                        CAM_DMA_SOFT_RSTSTAT;                            /* 1000, 0x1A005000, CAM_A_DMA_SOFT_RSTSTAT */
    CAM_REG_CQ0I_BASE_ADDR                          CAM_CQ0I_BASE_ADDR;                              /* 1004, 0x1A005004, CAM_A_CQ0I_BASE_ADDR */
    CAM_REG_CQ0I_XSIZE                              CAM_CQ0I_XSIZE;                                  /* 1008, 0x1A005008, CAM_A_CQ0I_XSIZE */
    CAM_REG_VERTICAL_FLIP_EN                        CAM_VERTICAL_FLIP_EN;                            /* 100C, 0x1A00500C, CAM_A_VERTICAL_FLIP_EN */
    CAM_REG_DMA_SOFT_RESET                          CAM_DMA_SOFT_RESET;                              /* 1010, 0x1A005010, CAM_A_DMA_SOFT_RESET */
    CAM_REG_LAST_ULTRA_EN                           CAM_LAST_ULTRA_EN;                               /* 1014, 0x1A005014, CAM_A_LAST_ULTRA_EN */
    CAM_REG_SPECIAL_FUN_EN                          CAM_SPECIAL_FUN_EN;                              /* 1018, 0x1A005018, CAM_A_SPECIAL_FUN_EN */
    UINT32                                          rsv_101C;                                        /* 101C, 0x1A00501C */
    CAM_REG_IMGO_BASE_ADDR                          CAM_IMGO_BASE_ADDR;                              /* 1020, 0x1A005020, CAM_A_IMGO_BASE_ADDR */
    UINT32                                          rsv_1024;                                        /* 1024, 0x1A005024 */
    CAM_REG_IMGO_OFST_ADDR                          CAM_IMGO_OFST_ADDR;                              /* 1028, 0x1A005028, CAM_A_IMGO_OFST_ADDR */
    CAM_REG_IMGO_DRS                                CAM_IMGO_DRS;                                    /* 102C, 0x1A00502C, CAM_A_IMGO_DRS */
    CAM_REG_IMGO_XSIZE                              CAM_IMGO_XSIZE;                                  /* 1030, 0x1A005030, CAM_A_IMGO_XSIZE */
    CAM_REG_IMGO_YSIZE                              CAM_IMGO_YSIZE;                                  /* 1034, 0x1A005034, CAM_A_IMGO_YSIZE */
    CAM_REG_IMGO_STRIDE                             CAM_IMGO_STRIDE;                                 /* 1038, 0x1A005038, CAM_A_IMGO_STRIDE */
    CAM_REG_IMGO_CON                                CAM_IMGO_CON;                                    /* 103C, 0x1A00503C, CAM_A_IMGO_CON */
    CAM_REG_IMGO_CON2                               CAM_IMGO_CON2;                                   /* 1040, 0x1A005040, CAM_A_IMGO_CON2 */
    CAM_REG_IMGO_CON3                               CAM_IMGO_CON3;                                   /* 1044, 0x1A005044, CAM_A_IMGO_CON3 */
    CAM_REG_IMGO_CROP                               CAM_IMGO_CROP;                                   /* 1048, 0x1A005048, CAM_A_IMGO_CROP */
    CAM_REG_IMGO_CON4                               CAM_IMGO_CON4;                                   /* 104C, 0x1A00504C, CAM_A_IMGO_CON4 */
    CAM_REG_RRZO_BASE_ADDR                          CAM_RRZO_BASE_ADDR;                              /* 1050, 0x1A005050, CAM_A_RRZO_BASE_ADDR */
    UINT32                                          rsv_1054;                                        /* 1054, 0x1A005054 */
    CAM_REG_RRZO_OFST_ADDR                          CAM_RRZO_OFST_ADDR;                              /* 1058, 0x1A005058, CAM_A_RRZO_OFST_ADDR */
    CAM_REG_RRZO_DRS                                CAM_RRZO_DRS;                                    /* 105C, 0x1A00505C, CAM_A_RRZO_DRS */
    CAM_REG_RRZO_XSIZE                              CAM_RRZO_XSIZE;                                  /* 1060, 0x1A005060, CAM_A_RRZO_XSIZE */
    CAM_REG_RRZO_YSIZE                              CAM_RRZO_YSIZE;                                  /* 1064, 0x1A005064, CAM_A_RRZO_YSIZE */
    CAM_REG_RRZO_STRIDE                             CAM_RRZO_STRIDE;                                 /* 1068, 0x1A005068, CAM_A_RRZO_STRIDE */
    CAM_REG_RRZO_CON                                CAM_RRZO_CON;                                    /* 106C, 0x1A00506C, CAM_A_RRZO_CON */
    CAM_REG_RRZO_CON2                               CAM_RRZO_CON2;                                   /* 1070, 0x1A005070, CAM_A_RRZO_CON2 */
    CAM_REG_RRZO_CON3                               CAM_RRZO_CON3;                                   /* 1074, 0x1A005074, CAM_A_RRZO_CON3 */
    CAM_REG_RRZO_CROP                               CAM_RRZO_CROP;                                   /* 1078, 0x1A005078, CAM_A_RRZO_CROP */
    CAM_REG_RRZO_CON4                               CAM_RRZO_CON4;                                   /* 107C, 0x1A00507C, CAM_A_RRZO_CON4 */
    CAM_REG_AAO_BASE_ADDR                           CAM_AAO_BASE_ADDR;                               /* 1080, 0x1A005080, CAM_A_AAO_BASE_ADDR */
    UINT32                                          rsv_1084;                                        /* 1084, 0x1A005084 */
    CAM_REG_AAO_OFST_ADDR                           CAM_AAO_OFST_ADDR;                               /* 1088, 0x1A005088, CAM_A_AAO_OFST_ADDR */
    CAM_REG_AAO_DRS                                 CAM_AAO_DRS;                                     /* 108C, 0x1A00508C, CAM_A_AAO_DRS */
    CAM_REG_AAO_XSIZE                               CAM_AAO_XSIZE;                                   /* 1090, 0x1A005090, CAM_A_AAO_XSIZE */
    CAM_REG_AAO_YSIZE                               CAM_AAO_YSIZE;                                   /* 1094, 0x1A005094, CAM_A_AAO_YSIZE */
    CAM_REG_AAO_STRIDE                              CAM_AAO_STRIDE;                                  /* 1098, 0x1A005098, CAM_A_AAO_STRIDE */
    CAM_REG_AAO_CON                                 CAM_AAO_CON;                                     /* 109C, 0x1A00509C, CAM_A_AAO_CON */
    CAM_REG_AAO_CON2                                CAM_AAO_CON2;                                    /* 10A0, 0x1A0050A0, CAM_A_AAO_CON2 */
    CAM_REG_AAO_CON3                                CAM_AAO_CON3;                                    /* 10A4, 0x1A0050A4, CAM_A_AAO_CON3 */
    UINT32                                          rsv_10A8;                                        /* 10A8, 0x1A0050A8 */
    CAM_REG_AAO_CON4                                CAM_AAO_CON4;                                    /* 10AC, 0x1A0050AC, CAM_A_AAO_CON4 */
    CAM_REG_AFO_BASE_ADDR                           CAM_AFO_BASE_ADDR;                               /* 10B0, 0x1A0050B0, CAM_A_AFO_BASE_ADDR */
    UINT32                                          rsv_10B4;                                        /* 10B4, 0x1A0050B4 */
    CAM_REG_AFO_OFST_ADDR                           CAM_AFO_OFST_ADDR;                               /* 10B8, 0x1A0050B8, CAM_A_AFO_OFST_ADDR */
    CAM_REG_AFO_DRS                                 CAM_AFO_DRS;                                     /* 10BC, 0x1A0050BC, CAM_A_AFO_DRS */
    CAM_REG_AFO_XSIZE                               CAM_AFO_XSIZE;                                   /* 10C0, 0x1A0050C0, CAM_A_AFO_XSIZE */
    CAM_REG_AFO_YSIZE                               CAM_AFO_YSIZE;                                   /* 10C4, 0x1A0050C4, CAM_A_AFO_YSIZE */
    CAM_REG_AFO_STRIDE                              CAM_AFO_STRIDE;                                  /* 10C8, 0x1A0050C8, CAM_A_AFO_STRIDE */
    CAM_REG_AFO_CON                                 CAM_AFO_CON;                                     /* 10CC, 0x1A0050CC, CAM_A_AFO_CON */
    CAM_REG_AFO_CON2                                CAM_AFO_CON2;                                    /* 10D0, 0x1A0050D0, CAM_A_AFO_CON2 */
    CAM_REG_AFO_CON3                                CAM_AFO_CON3;                                    /* 10D4, 0x1A0050D4, CAM_A_AFO_CON3 */
    UINT32                                          rsv_10D8;                                        /* 10D8, 0x1A0050D8 */
    CAM_REG_AFO_CON4                                CAM_AFO_CON4;                                    /* 10DC, 0x1A0050DC, CAM_A_AFO_CON4 */
    CAM_REG_LCSO_BASE_ADDR                          CAM_LCSO_BASE_ADDR;                              /* 10E0, 0x1A0050E0, CAM_A_LCSO_BASE_ADDR */
    UINT32                                          rsv_10E4;                                        /* 10E4, 0x1A0050E4 */
    CAM_REG_LCSO_OFST_ADDR                          CAM_LCSO_OFST_ADDR;                              /* 10E8, 0x1A0050E8, CAM_A_LCSO_OFST_ADDR */
    CAM_REG_LCSO_DRS                                CAM_LCSO_DRS;                                    /* 10EC, 0x1A0050EC, CAM_A_LCSO_DRS */
    CAM_REG_LCSO_XSIZE                              CAM_LCSO_XSIZE;                                  /* 10F0, 0x1A0050F0, CAM_A_LCSO_XSIZE */
    CAM_REG_LCSO_YSIZE                              CAM_LCSO_YSIZE;                                  /* 10F4, 0x1A0050F4, CAM_A_LCSO_YSIZE */
    CAM_REG_LCSO_STRIDE                             CAM_LCSO_STRIDE;                                 /* 10F8, 0x1A0050F8, CAM_A_LCSO_STRIDE */
    CAM_REG_LCSO_CON                                CAM_LCSO_CON;                                    /* 10FC, 0x1A0050FC, CAM_A_LCSO_CON */
    CAM_REG_LCSO_CON2                               CAM_LCSO_CON2;                                   /* 1100, 0x1A005100, CAM_A_LCSO_CON2 */
    CAM_REG_LCSO_CON3                               CAM_LCSO_CON3;                                   /* 1104, 0x1A005104, CAM_A_LCSO_CON3 */
    UINT32                                          rsv_1108;                                        /* 1108, 0x1A005108 */
    CAM_REG_LCSO_CON4                               CAM_LCSO_CON4;                                   /* 110C, 0x1A00510C, CAM_A_LCSO_CON4 */
    CAM_REG_UFEO_BASE_ADDR                          CAM_UFEO_BASE_ADDR;                              /* 1110, 0x1A005110, CAM_A_UFEO_BASE_ADDR */
    UINT32                                          rsv_1114;                                        /* 1114, 0x1A005114 */
    CAM_REG_UFEO_OFST_ADDR                          CAM_UFEO_OFST_ADDR;                              /* 1118, 0x1A005118, CAM_A_UFEO_OFST_ADDR */
    CAM_REG_UFEO_DRS                                CAM_UFEO_DRS;                                    /* 111C, 0x1A00511C, CAM_A_UFEO_DRS */
    CAM_REG_UFEO_XSIZE                              CAM_UFEO_XSIZE;                                  /* 1120, 0x1A005120, CAM_A_UFEO_XSIZE */
    CAM_REG_UFEO_YSIZE                              CAM_UFEO_YSIZE;                                  /* 1124, 0x1A005124, CAM_A_UFEO_YSIZE */
    CAM_REG_UFEO_STRIDE                             CAM_UFEO_STRIDE;                                 /* 1128, 0x1A005128, CAM_A_UFEO_STRIDE */
    CAM_REG_UFEO_CON                                CAM_UFEO_CON;                                    /* 112C, 0x1A00512C, CAM_A_UFEO_CON */
    CAM_REG_UFEO_CON2                               CAM_UFEO_CON2;                                   /* 1130, 0x1A005130, CAM_A_UFEO_CON2 */
    CAM_REG_UFEO_CON3                               CAM_UFEO_CON3;                                   /* 1134, 0x1A005134, CAM_A_UFEO_CON3 */
    UINT32                                          rsv_1138;                                        /* 1138, 0x1A005138 */
    CAM_REG_UFEO_CON4                               CAM_UFEO_CON4;                                   /* 113C, 0x1A00513C, CAM_A_UFEO_CON4 */
    CAM_REG_PDO_BASE_ADDR                           CAM_PDO_BASE_ADDR;                               /* 1140, 0x1A005140, CAM_A_PDO_BASE_ADDR */
    UINT32                                          rsv_1144;                                        /* 1144, 0x1A005144 */
    CAM_REG_PDO_OFST_ADDR                           CAM_PDO_OFST_ADDR;                               /* 1148, 0x1A005148, CAM_A_PDO_OFST_ADDR */
    CAM_REG_PDO_DRS                                 CAM_PDO_DRS;                                     /* 114C, 0x1A00514C, CAM_A_PDO_DRS */
    CAM_REG_PDO_XSIZE                               CAM_PDO_XSIZE;                                   /* 1150, 0x1A005150, CAM_A_PDO_XSIZE */
    CAM_REG_PDO_YSIZE                               CAM_PDO_YSIZE;                                   /* 1154, 0x1A005154, CAM_A_PDO_YSIZE */
    CAM_REG_PDO_STRIDE                              CAM_PDO_STRIDE;                                  /* 1158, 0x1A005158, CAM_A_PDO_STRIDE */
    CAM_REG_PDO_CON                                 CAM_PDO_CON;                                     /* 115C, 0x1A00515C, CAM_A_PDO_CON */
    CAM_REG_PDO_CON2                                CAM_PDO_CON2;                                    /* 1160, 0x1A005160, CAM_A_PDO_CON2 */
    CAM_REG_PDO_CON3                                CAM_PDO_CON3;                                    /* 1164, 0x1A005164, CAM_A_PDO_CON3 */
    UINT32                                          rsv_1168;                                        /* 1168, 0x1A005168 */
    CAM_REG_PDO_CON4                                CAM_PDO_CON4;                                    /* 116C, 0x1A00516C, CAM_A_PDO_CON4 */
    CAM_REG_BPCI_BASE_ADDR                          CAM_BPCI_BASE_ADDR;                              /* 1170, 0x1A005170, CAM_A_BPCI_BASE_ADDR */
    UINT32                                          rsv_1174;                                        /* 1174, 0x1A005174 */
    CAM_REG_BPCI_OFST_ADDR                          CAM_BPCI_OFST_ADDR;                              /* 1178, 0x1A005178, CAM_A_BPCI_OFST_ADDR */
    CAM_REG_BPCI_DRS                                CAM_BPCI_DRS;                                    /* 117C, 0x1A00517C, CAM_A_BPCI_DRS */
    CAM_REG_BPCI_XSIZE                              CAM_BPCI_XSIZE;                                  /* 1180, 0x1A005180, CAM_A_BPCI_XSIZE */
    CAM_REG_BPCI_YSIZE                              CAM_BPCI_YSIZE;                                  /* 1184, 0x1A005184, CAM_A_BPCI_YSIZE */
    CAM_REG_BPCI_STRIDE                             CAM_BPCI_STRIDE;                                 /* 1188, 0x1A005188, CAM_A_BPCI_STRIDE */
    CAM_REG_BPCI_CON                                CAM_BPCI_CON;                                    /* 118C, 0x1A00518C, CAM_A_BPCI_CON */
    CAM_REG_BPCI_CON2                               CAM_BPCI_CON2;                                   /* 1190, 0x1A005190, CAM_A_BPCI_CON2 */
    CAM_REG_BPCI_CON3                               CAM_BPCI_CON3;                                   /* 1194, 0x1A005194, CAM_A_BPCI_CON3 */
    UINT32                                          rsv_1198;                                        /* 1198, 0x1A005198 */
    CAM_REG_BPCI_CON4                               CAM_BPCI_CON4;                                   /* 119C, 0x1A00519C, CAM_A_BPCI_CON4 */
    CAM_REG_CACI_BASE_ADDR                          CAM_CACI_BASE_ADDR;                              /* 11A0, 0x1A0051A0, CAM_A_CACI_BASE_ADDR */
    UINT32                                          rsv_11A4;                                        /* 11A4, 0x1A0051A4 */
    CAM_REG_CACI_OFST_ADDR                          CAM_CACI_OFST_ADDR;                              /* 11A8, 0x1A0051A8, CAM_A_CACI_OFST_ADDR */
    CAM_REG_CACI_DRS                                CAM_CACI_DRS;                                    /* 11AC, 0x1A0051AC, CAM_A_CACI_DRS */
    CAM_REG_CACI_XSIZE                              CAM_CACI_XSIZE;                                  /* 11B0, 0x1A0051B0, CAM_A_CACI_XSIZE */
    CAM_REG_CACI_YSIZE                              CAM_CACI_YSIZE;                                  /* 11B4, 0x1A0051B4, CAM_A_CACI_YSIZE */
    CAM_REG_CACI_STRIDE                             CAM_CACI_STRIDE;                                 /* 11B8, 0x1A0051B8, CAM_A_CACI_STRIDE */
    CAM_REG_CACI_CON                                CAM_CACI_CON;                                    /* 11BC, 0x1A0051BC, CAM_A_CACI_CON */
    CAM_REG_CACI_CON2                               CAM_CACI_CON2;                                   /* 11C0, 0x1A0051C0, CAM_A_CACI_CON2 */
    CAM_REG_CACI_CON3                               CAM_CACI_CON3;                                   /* 11C4, 0x1A0051C4, CAM_A_CACI_CON3 */
    UINT32                                          rsv_11C8;                                        /* 11C8, 0x1A0051C8 */
    CAM_REG_CACI_CON4                               CAM_CACI_CON4;                                   /* 11CC, 0x1A0051CC, CAM_A_CACI_CON4 */
    CAM_REG_LSCI_BASE_ADDR                          CAM_LSCI_BASE_ADDR;                              /* 11D0, 0x1A0051D0, CAM_A_LSCI_BASE_ADDR */
    UINT32                                          rsv_11D4;                                        /* 11D4, 0x1A0051D4 */
    CAM_REG_LSCI_OFST_ADDR                          CAM_LSCI_OFST_ADDR;                              /* 11D8, 0x1A0051D8, CAM_A_LSCI_OFST_ADDR */
    CAM_REG_LSCI_DRS                                CAM_LSCI_DRS;                                    /* 11DC, 0x1A0051DC, CAM_A_LSCI_DRS */
    CAM_REG_LSCI_XSIZE                              CAM_LSCI_XSIZE;                                  /* 11E0, 0x1A0051E0, CAM_A_LSCI_XSIZE */
    CAM_REG_LSCI_YSIZE                              CAM_LSCI_YSIZE;                                  /* 11E4, 0x1A0051E4, CAM_A_LSCI_YSIZE */
    CAM_REG_LSCI_STRIDE                             CAM_LSCI_STRIDE;                                 /* 11E8, 0x1A0051E8, CAM_A_LSCI_STRIDE */
    CAM_REG_LSCI_CON                                CAM_LSCI_CON;                                    /* 11EC, 0x1A0051EC, CAM_A_LSCI_CON */
    CAM_REG_LSCI_CON2                               CAM_LSCI_CON2;                                   /* 11F0, 0x1A0051F0, CAM_A_LSCI_CON2 */
    CAM_REG_LSCI_CON3                               CAM_LSCI_CON3;                                   /* 11F4, 0x1A0051F4, CAM_A_LSCI_CON3 */
    UINT32                                          rsv_11F8;                                        /* 11F8, 0x1A0051F8 */
    CAM_REG_LSCI_CON4                               CAM_LSCI_CON4;                                   /* 11FC, 0x1A0051FC, CAM_A_LSCI_CON4 */
    CAM_REG_LSC3I_BASE_ADDR                         CAM_LSC3I_BASE_ADDR;                             /* 1200, 0x1A005200, CAM_A_LSC3I_BASE_ADDR */
    UINT32                                          rsv_1204;                                        /* 1204, 0x1A005204 */
    CAM_REG_LSC3I_OFST_ADDR                         CAM_LSC3I_OFST_ADDR;                             /* 1208, 0x1A005208, CAM_A_LSC3I_OFST_ADDR */
    CAM_REG_LSC3I_DRS                               CAM_LSC3I_DRS;                                   /* 120C, 0x1A00520C, CAM_A_LSC3I_DRS */
    CAM_REG_LSC3I_XSIZE                             CAM_LSC3I_XSIZE;                                 /* 1210, 0x1A005210, CAM_A_LSC3I_XSIZE */
    CAM_REG_LSC3I_YSIZE                             CAM_LSC3I_YSIZE;                                 /* 1214, 0x1A005214, CAM_A_LSC3I_YSIZE */
    CAM_REG_LSC3I_STRIDE                            CAM_LSC3I_STRIDE;                                /* 1218, 0x1A005218, CAM_A_LSC3I_STRIDE */
    CAM_REG_LSC3I_CON                               CAM_LSC3I_CON;                                   /* 121C, 0x1A00521C, CAM_A_LSC3I_CON */
    CAM_REG_LSC3I_CON2                              CAM_LSC3I_CON2;                                  /* 1220, 0x1A005220, CAM_A_LSC3I_CON2 */
    CAM_REG_LSC3I_CON3                              CAM_LSC3I_CON3;                                  /* 1224, 0x1A005224, CAM_A_LSC3I_CON3 */
    UINT32                                          rsv_1228;                                        /* 1228, 0x1A005228 */
    CAM_REG_LSC3I_CON4                              CAM_LSC3I_CON4;                                  /* 122C, 0x1A00522C, CAM_A_LSC3I_CON4 */
    CAM_REG_PDI_BASE_ADDR                           CAM_PDI_BASE_ADDR;                               /* 1230, 0x1A005230, CAM_A_PDI_BASE_ADDR */
    CAM_REG_PDI_OFST_ADDR                           CAM_PDI_OFST_ADDR;                               /* 1234, 0x1A005234, CAM_A_PDI_OFST_ADDR */
    UINT32                                          rsv_1238;                                        /* 1238, 0x1A005238 */
    CAM_REG_PDI_DRS                                 CAM_PDI_DRS;                                     /* 123C, 0x1A00523C, CAM_A_PDI_DRS */
    CAM_REG_PDI_XSIZE                               CAM_PDI_XSIZE;                                   /* 1240, 0x1A005240, CAM_A_PDI_XSIZE */
    CAM_REG_PDI_YSIZE                               CAM_PDI_YSIZE;                                   /* 1244, 0x1A005244, CAM_A_PDI_YSIZE */
    CAM_REG_PDI_STRIDE                              CAM_PDI_STRIDE;                                  /* 1248, 0x1A005248, CAM_A_PDI_STRIDE */
    CAM_REG_PDI_CON                                 CAM_PDI_CON;                                     /* 124C, 0x1A00524C, CAM_A_PDI_CON */
    CAM_REG_PDI_CON2                                CAM_PDI_CON2;                                    /* 1250, 0x1A005250, CAM_A_PDI_CON2 */
    CAM_REG_PDI_CON3                                CAM_PDI_CON3;                                    /* 1254, 0x1A005254, CAM_A_PDI_CON3 */
    UINT32                                          rsv_1258;                                        /* 1258, 0x1A005258 */
    CAM_REG_PDI_CON4                                CAM_PDI_CON4;                                    /* 125C, 0x1A00525C, CAM_A_PDI_CON4 */
    CAM_REG_PSO_BASE_ADDR                           CAM_PSO_BASE_ADDR;                               /* 1260, 0x1A005260, CAM_A_PSO_BASE_ADDR */
    CAM_REG_PSO_OFST_ADDR                           CAM_PSO_OFST_ADDR;                               /* 1264, 0x1A005264, CAM_A_PSO_OFST_ADDR */
    UINT32                                          rsv_1268;                                        /* 1268, 0x1A005268 */
    CAM_REG_PSO_DRS                                 CAM_PSO_DRS;                                     /* 126C, 0x1A00526C, CAM_A_PSO_DRS */
    CAM_REG_PSO_XSIZE                               CAM_PSO_XSIZE;                                   /* 1270, 0x1A005270, CAM_A_PSO_XSIZE */
    CAM_REG_PSO_YSIZE                               CAM_PSO_YSIZE;                                   /* 1274, 0x1A005274, CAM_A_PSO_YSIZE */
    CAM_REG_PSO_STRIDE                              CAM_PSO_STRIDE;                                  /* 1278, 0x1A005278, CAM_A_PSO_STRIDE */
    CAM_REG_PSO_CON                                 CAM_PSO_CON;                                     /* 127C, 0x1A00527C, CAM_A_PSO_CON */
    CAM_REG_PSO_CON2                                CAM_PSO_CON2;                                    /* 1280, 0x1A005280, CAM_A_PSO_CON2 */
    CAM_REG_PSO_CON3                                CAM_PSO_CON3;                                    /* 1284, 0x1A005284, CAM_A_PSO_CON3 */
    UINT32                                          rsv_1288;                                        /* 1288, 0x1A005288 */
    CAM_REG_PSO_CON4                                CAM_PSO_CON4;                                    /* 128C, 0x1A00528C, CAM_A_PSO_CON4 */
    CAM_REG_LMVO_BASE_ADDR                          CAM_LMVO_BASE_ADDR;                              /* 1290, 0x1A005290, CAM_A_LMVO_BASE_ADDR */
    CAM_REG_LMVO_OFST_ADDR                          CAM_LMVO_OFST_ADDR;                              /* 1294, 0x1A005294, CAM_A_LMVO_OFST_ADDR */
    UINT32                                          rsv_1298;                                        /* 1298, 0x1A005298 */
    CAM_REG_LMVO_DRS                                CAM_LMVO_DRS;                                    /* 129C, 0x1A00529C, CAM_A_LMVO_DRS */
    CAM_REG_LMVO_XSIZE                              CAM_LMVO_XSIZE;                                  /* 12A0, 0x1A0052A0, CAM_A_LMVO_XSIZE */
    CAM_REG_LMVO_YSIZE                              CAM_LMVO_YSIZE;                                  /* 12A4, 0x1A0052A4, CAM_A_LMVO_YSIZE */
    CAM_REG_LMVO_STRIDE                             CAM_LMVO_STRIDE;                                 /* 12A8, 0x1A0052A8, CAM_A_LMVO_STRIDE */
    CAM_REG_LMVO_CON                                CAM_LMVO_CON;                                    /* 12AC, 0x1A0052AC, CAM_A_LMVO_CON */
    CAM_REG_LMVO_CON2                               CAM_LMVO_CON2;                                   /* 12B0, 0x1A0052B0, CAM_A_LMVO_CON2 */
    CAM_REG_LMVO_CON3                               CAM_LMVO_CON3;                                   /* 12B4, 0x1A0052B4, CAM_A_LMVO_CON3 */
    UINT32                                          rsv_12B8;                                        /* 12B8, 0x1A0052B8 */
    CAM_REG_LMVO_CON4                               CAM_LMVO_CON4;                                   /* 12BC, 0x1A0052BC, CAM_A_LMVO_CON4 */
    CAM_REG_FLKO_BASE_ADDR                          CAM_FLKO_BASE_ADDR;                              /* 12C0, 0x1A0052C0, CAM_A_FLKO_BASE_ADDR */
    CAM_REG_FLKO_OFST_ADDR                          CAM_FLKO_OFST_ADDR;                              /* 12C4, 0x1A0052C4, CAM_A_FLKO_OFST_ADDR */
    UINT32                                          rsv_12C8;                                        /* 12C8, 0x1A0052C8 */
    CAM_REG_FLKO_DRS                                CAM_FLKO_DRS;                                    /* 12CC, 0x1A0052CC, CAM_A_FLKO_DRS */
    CAM_REG_FLKO_XSIZE                              CAM_FLKO_XSIZE;                                  /* 12D0, 0x1A0052D0, CAM_A_FLKO_XSIZE */
    CAM_REG_FLKO_YSIZE                              CAM_FLKO_YSIZE;                                  /* 12D4, 0x1A0052D4, CAM_A_FLKO_YSIZE */
    CAM_REG_FLKO_STRIDE                             CAM_FLKO_STRIDE;                                 /* 12D8, 0x1A0052D8, CAM_A_FLKO_STRIDE */
    CAM_REG_FLKO_CON                                CAM_FLKO_CON;                                    /* 12DC, 0x1A0052DC, CAM_A_FLKO_CON */
    CAM_REG_FLKO_CON2                               CAM_FLKO_CON2;                                   /* 12E0, 0x1A0052E0, CAM_A_FLKO_CON2 */
    CAM_REG_FLKO_CON3                               CAM_FLKO_CON3;                                   /* 12E4, 0x1A0052E4, CAM_A_FLKO_CON3 */
    UINT32                                          rsv_12E8;                                        /* 12E8, 0x1A0052E8 */
    CAM_REG_FLKO_CON4                               CAM_FLKO_CON4;                                   /* 12EC, 0x1A0052EC, CAM_A_FLKO_CON4 */
    CAM_REG_RSSO_A_BASE_ADDR                        CAM_RSSO_A_BASE_ADDR;                            /* 12F0, 0x1A0052F0, CAM_A_RSSO_A_BASE_ADDR */
    CAM_REG_RSSO_A_OFST_ADDR                        CAM_RSSO_A_OFST_ADDR;                            /* 12F4, 0x1A0052F4, CAM_A_RSSO_A_OFST_ADDR */
    UINT32                                          rsv_12F8;                                        /* 12F8, 0x1A0052F8 */
    CAM_REG_RSSO_A_DRS                              CAM_RSSO_A_DRS;                                  /* 12FC, 0x1A0052FC, CAM_A_RSSO_A_DRS */
    CAM_REG_RSSO_A_XSIZE                            CAM_RSSO_A_XSIZE;                                /* 1300, 0x1A005300, CAM_A_RSSO_A_XSIZE */
    CAM_REG_RSSO_A_YSIZE                            CAM_RSSO_A_YSIZE;                                /* 1304, 0x1A005304, CAM_A_RSSO_A_YSIZE */
    CAM_REG_RSSO_A_STRIDE                           CAM_RSSO_A_STRIDE;                               /* 1308, 0x1A005308, CAM_A_RSSO_A_STRIDE */
    CAM_REG_RSSO_A_CON                              CAM_RSSO_A_CON;                                  /* 130C, 0x1A00530C, CAM_A_RSSO_A_CON */
    CAM_REG_RSSO_A_CON2                             CAM_RSSO_A_CON2;                                 /* 1310, 0x1A005310, CAM_A_RSSO_A_CON2 */
    CAM_REG_RSSO_A_CON3                             CAM_RSSO_A_CON3;                                 /* 1314, 0x1A005314, CAM_A_RSSO_A_CON3 */
    UINT32                                          rsv_1318;                                        /* 1318, 0x1A005318 */
    CAM_REG_RSSO_A_CON4                             CAM_RSSO_A_CON4;                                 /* 131C, 0x1A00531C, CAM_A_RSSO_A_CON4 */
    CAM_REG_UFGO_BASE_ADDR                          CAM_UFGO_BASE_ADDR;                              /* 1320, 0x1A005320, CAM_A_UFGO_BASE_ADDR */
    CAM_REG_UFGO_OFST_ADDR                          CAM_UFGO_OFST_ADDR;                              /* 1324, 0x1A005324, CAM_A_UFGO_OFST_ADDR */
    UINT32                                          rsv_1328;                                        /* 1328, 0x1A005328 */
    CAM_REG_UFGO_DRS                                CAM_UFGO_DRS;                                    /* 132C, 0x1A00532C, CAM_A_UFGO_DRS */
    CAM_REG_UFGO_XSIZE                              CAM_UFGO_XSIZE;                                  /* 1330, 0x1A005330, CAM_A_UFGO_XSIZE */
    CAM_REG_UFGO_YSIZE                              CAM_UFGO_YSIZE;                                  /* 1334, 0x1A005334, CAM_A_UFGO_YSIZE */
    CAM_REG_UFGO_STRIDE                             CAM_UFGO_STRIDE;                                 /* 1338, 0x1A005338, CAM_A_UFGO_STRIDE */
    CAM_REG_UFGO_CON                                CAM_UFGO_CON;                                    /* 133C, 0x1A00533C, CAM_A_UFGO_CON */
    CAM_REG_UFGO_CON2                               CAM_UFGO_CON2;                                   /* 1340, 0x1A005340, CAM_A_UFGO_CON2 */
    CAM_REG_UFGO_CON3                               CAM_UFGO_CON3;                                   /* 1344, 0x1A005344, CAM_A_UFGO_CON3 */
    UINT32                                          rsv_1348;                                        /* 1348, 0x1A005348 */
    CAM_REG_UFGO_CON4                               CAM_UFGO_CON4;                                   /* 134C, 0x1A00534C, CAM_A_UFGO_CON4 */
    CAM_REG_DMA_ERR_CTRL                            CAM_DMA_ERR_CTRL;                                /* 1350, 0x1A005350, CAM_A_DMA_ERR_CTRL */
    UINT32                                          rsv_1354[3];                                     /* 1354..135F, 0x1A005354..1A00535F */
    CAM_REG_IMGO_ERR_STAT                           CAM_IMGO_ERR_STAT;                               /* 1360, 0x1A005360, CAM_A_IMGO_ERR_STAT */
    CAM_REG_RRZO_ERR_STAT                           CAM_RRZO_ERR_STAT;                               /* 1364, 0x1A005364, CAM_A_RRZO_ERR_STAT */
    CAM_REG_AAO_ERR_STAT                            CAM_AAO_ERR_STAT;                                /* 1368, 0x1A005368, CAM_A_AAO_ERR_STAT */
    CAM_REG_AFO_ERR_STAT                            CAM_AFO_ERR_STAT;                                /* 136C, 0x1A00536C, CAM_A_AFO_ERR_STAT */
    CAM_REG_LCSO_ERR_STAT                           CAM_LCSO_ERR_STAT;                               /* 1370, 0x1A005370, CAM_A_LCSO_ERR_STAT */
    CAM_REG_UFEO_ERR_STAT                           CAM_UFEO_ERR_STAT;                               /* 1374, 0x1A005374, CAM_A_UFEO_ERR_STAT */
    CAM_REG_PDO_ERR_STAT                            CAM_PDO_ERR_STAT;                                /* 1378, 0x1A005378, CAM_A_PDO_ERR_STAT */
    CAM_REG_BPCI_ERR_STAT                           CAM_BPCI_ERR_STAT;                               /* 137C, 0x1A00537C, CAM_A_BPCI_ERR_STAT */
    CAM_REG_CACI_ERR_STAT                           CAM_CACI_ERR_STAT;                               /* 1380, 0x1A005380, CAM_A_CACI_ERR_STAT */
    CAM_REG_LSCI_ERR_STAT                           CAM_LSCI_ERR_STAT;                               /* 1384, 0x1A005384, CAM_A_LSCI_ERR_STAT */
    CAM_REG_LSC3I_ERR_STAT                          CAM_LSC3I_ERR_STAT;                              /* 1388, 0x1A005388, CAM_A_LSC3I_ERR_STAT */
    CAM_REG_PDI_ERR_STAT                            CAM_PDI_ERR_STAT;                                /* 138C, 0x1A00538C, CAM_A_PDI_ERR_STAT */
    CAM_REG_LMVO_ERR_STAT                           CAM_LMVO_ERR_STAT;                               /* 1390, 0x1A005390, CAM_A_LMVO_ERR_STAT */
    CAM_REG_FLKO_ERR_STAT                           CAM_FLKO_ERR_STAT;                               /* 1394, 0x1A005394, CAM_A_FLKO_ERR_STAT */
    CAM_REG_RSSO_A_ERR_STAT                         CAM_RSSO_A_ERR_STAT;                             /* 1398, 0x1A005398, CAM_A_RSSO_A_ERR_STAT */
    CAM_REG_UFGO_ERR_STAT                           CAM_UFGO_ERR_STAT;                               /* 139C, 0x1A00539C, CAM_A_UFGO_ERR_STAT */
    CAM_REG_PSO_ERR_STAT                            CAM_PSO_ERR_STAT;                                /* 13A0, 0x1A0053A0, CAM_A_PSO_ERR_STAT */
    UINT32                                          rsv_13A4[2];                                     /* 13A4..13AB, 0x1A0053A4..1A0053AB */
    CAM_REG_DMA_DEBUG_ADDR                          CAM_DMA_DEBUG_ADDR;                              /* 13AC, 0x1A0053AC, CAM_A_DMA_DEBUG_ADDR */
    CAM_REG_DMA_RSV1                                CAM_DMA_RSV1;                                    /* 13B0, 0x1A0053B0, CAM_A_DMA_RSV1 */
    CAM_REG_DMA_RSV2                                CAM_DMA_RSV2;                                    /* 13B4, 0x1A0053B4, CAM_A_DMA_RSV2 */
    CAM_REG_MAGIC_NUM                               CAM_MAGIC_NUM;                                   /* 13B8, 0x1A0053B8, CAM_A_DMA_RSV3 */
    CAM_REG_DATA_PATTERN                            CAM_DATA_PATTERN;                                /* 13BC, 0x1A0053BC, CAM_A_DMA_RSV4 */
    CAM_REG_DMA_RSV5                                CAM_DMA_RSV5;                                    /* 13C0, 0x1A0053C0, CAM_A_DMA_RSV5 */
    CAM_REG_DMA_RSV6                                CAM_DMA_RSV6;                                    /* 13C4, 0x1A0053C4, CAM_A_DMA_RSV6 */
    CAM_REG_DMA_DEBUG_SEL                           CAM_DMA_DEBUG_SEL;                               /* 13C8, 0x1A0053C8, CAM_A_DMA_DEBUG_SEL */
    CAM_REG_DMA_BW_SELF_TEST                        CAM_DMA_BW_SELF_TEST;                            /* 13CC, 0x1A0053CC, CAM_A_DMA_BW_SELF_TEST */
    UINT32                                          rsv_13D0[12];                                    /* 13D0..13FF, 0x1A0053D0..1A0053FF */
    CAM_REG_DMA_FRAME_HEADER_EN                     CAM_DMA_FRAME_HEADER_EN;                         /* 1400, 0x1A005400, CAM_A_DMA_FRAME_HEADER_EN */
    CAM_REG_IMGO_FH_BASE_ADDR                       CAM_IMGO_FH_BASE_ADDR;                           /* 1404, 0x1A005404, CAM_A_IMGO_FH_BASE_ADDR */
    CAM_REG_RRZO_FH_BASE_ADDR                       CAM_RRZO_FH_BASE_ADDR;                           /* 1408, 0x1A005408, CAM_A_RRZO_FH_BASE_ADDR */
    CAM_REG_AAO_FH_BASE_ADDR                        CAM_AAO_FH_BASE_ADDR;                            /* 140C, 0x1A00540C, CAM_A_AAO_FH_BASE_ADDR */
    CAM_REG_AFO_FH_BASE_ADDR                        CAM_AFO_FH_BASE_ADDR;                            /* 1410, 0x1A005410, CAM_A_AFO_FH_BASE_ADDR */
    CAM_REG_LCSO_FH_BASE_ADDR                       CAM_LCSO_FH_BASE_ADDR;                           /* 1414, 0x1A005414, CAM_A_LCSO_FH_BASE_ADDR */
    CAM_REG_UFEO_FH_BASE_ADDR                       CAM_UFEO_FH_BASE_ADDR;                           /* 1418, 0x1A005418, CAM_A_UFEO_FH_BASE_ADDR */
    CAM_REG_PDO_FH_BASE_ADDR                        CAM_PDO_FH_BASE_ADDR;                            /* 141C, 0x1A00541C, CAM_A_PDO_FH_BASE_ADDR */
    CAM_REG_PSO_FH_BASE_ADDR                        CAM_PSO_FH_BASE_ADDR;                            /* 1420, 0x1A005420, CAM_A_PSO_FH_BASE_ADDR */
    CAM_REG_LMVO_FH_BASE_ADDR                       CAM_LMVO_FH_BASE_ADDR;                           /* 1424, 0x1A005424, CAM_A_LMVO_FH_BASE_ADDR */
    CAM_REG_FLKO_FH_BASE_ADDR                       CAM_FLKO_FH_BASE_ADDR;                           /* 1428, 0x1A005428, CAM_A_FLKO_FH_BASE_ADDR */
    CAM_REG_RSSO_A_FH_BASE_ADDR                     CAM_RSSO_A_FH_BASE_ADDR;                         /* 142C, 0x1A00542C, CAM_A_RSSO_A_FH_BASE_ADDR */
    CAM_REG_UFGO_FH_BASE_ADDR                       CAM_UFGO_FH_BASE_ADDR;                           /* 1430, 0x1A005430, CAM_A_UFGO_FH_BASE_ADDR */
    CAM_REG_IMGO_FH_SPARE_2                         CAM_IMGO_FH_SPARE_2;                             /* 1434, 0x1A005434, CAM_A_IMGO_FH_SPARE_2 */
    CAM_REG_IMGO_FH_SPARE_3                         CAM_IMGO_FH_SPARE_3;                             /* 1438, 0x1A005438, CAM_A_IMGO_FH_SPARE_3 */
    CAM_REG_IMGO_FH_SPARE_4                         CAM_IMGO_FH_SPARE_4;                             /* 143C, 0x1A00543C, CAM_A_IMGO_FH_SPARE_4 */
    CAM_REG_IMGO_FH_SPARE_5                         CAM_IMGO_FH_SPARE_5;                             /* 1440, 0x1A005440, CAM_A_IMGO_FH_SPARE_5 */
    CAM_REG_IMGO_FH_SPARE_6                         CAM_IMGO_FH_SPARE_6;                             /* 1444, 0x1A005444, CAM_A_IMGO_FH_SPARE_6 */
    CAM_REG_IMGO_FH_SPARE_7                         CAM_IMGO_FH_SPARE_7;                             /* 1448, 0x1A005448, CAM_A_IMGO_FH_SPARE_7 */
    CAM_REG_IMGO_FH_SPARE_8                         CAM_IMGO_FH_SPARE_8;                             /* 144C, 0x1A00544C, CAM_A_IMGO_FH_SPARE_8 */
    CAM_REG_IMGO_FH_SPARE_9                         CAM_IMGO_FH_SPARE_9;                             /* 1450, 0x1A005450, CAM_A_IMGO_FH_SPARE_9 */
    CAM_REG_IMGO_FH_SPARE_10                        CAM_IMGO_FH_SPARE_10;                            /* 1454, 0x1A005454, CAM_A_IMGO_FH_SPARE_10 */
    CAM_REG_IMGO_FH_SPARE_11                        CAM_IMGO_FH_SPARE_11;                            /* 1458, 0x1A005458, CAM_A_IMGO_FH_SPARE_11 */
    CAM_REG_IMGO_FH_SPARE_12                        CAM_IMGO_FH_SPARE_12;                            /* 145C, 0x1A00545C, CAM_A_IMGO_FH_SPARE_12 */
    CAM_REG_IMGO_FH_SPARE_13                        CAM_IMGO_FH_SPARE_13;                            /* 1460, 0x1A005460, CAM_A_IMGO_FH_SPARE_13 */
    UINT32                                          rsv_1464;                                        /* 1464, 0x1A005464,  */
    UINT32                                          rsv_1468;                                        /* 1468, 0x1A005468,  */
    UINT32                                          rsv_146C;                                        /* 146C, 0x1A00546C,  */
    UINT32                                          rsv_1470;                                        /* 1470, 0x1A005470 */
    CAM_REG_RRZO_FH_SPARE_2                         CAM_RRZO_FH_SPARE_2;                             /* 1474, 0x1A005474, CAM_A_RRZO_FH_SPARE_2 */
    CAM_REG_RRZO_FH_SPARE_3                         CAM_RRZO_FH_SPARE_3;                             /* 1478, 0x1A005478, CAM_A_RRZO_FH_SPARE_3 */
    CAM_REG_RRZO_FH_SPARE_4                         CAM_RRZO_FH_SPARE_4;                             /* 147C, 0x1A00547C, CAM_A_RRZO_FH_SPARE_4 */
    CAM_REG_RRZO_FH_SPARE_5                         CAM_RRZO_FH_SPARE_5;                             /* 1480, 0x1A005480, CAM_A_RRZO_FH_SPARE_5 */
    CAM_REG_RRZO_FH_SPARE_6                         CAM_RRZO_FH_SPARE_6;                             /* 1484, 0x1A005484, CAM_A_RRZO_FH_SPARE_6 */
    CAM_REG_RRZO_FH_SPARE_7                         CAM_RRZO_FH_SPARE_7;                             /* 1488, 0x1A005488, CAM_A_RRZO_FH_SPARE_7 */
    CAM_REG_RRZO_FH_SPARE_8                         CAM_RRZO_FH_SPARE_8;                             /* 148C, 0x1A00548C, CAM_A_RRZO_FH_SPARE_8 */
    CAM_REG_RRZO_FH_SPARE_9                         CAM_RRZO_FH_SPARE_9;                             /* 1490, 0x1A005490, CAM_A_RRZO_FH_SPARE_9 */
    CAM_REG_RRZO_FH_SPARE_10                        CAM_RRZO_FH_SPARE_10;                            /* 1494, 0x1A005494, CAM_A_RRZO_FH_SPARE_10 */
    CAM_REG_RRZO_FH_SPARE_11                        CAM_RRZO_FH_SPARE_11;                            /* 1498, 0x1A005498, CAM_A_RRZO_FH_SPARE_11 */
    CAM_REG_RRZO_FH_SPARE_12                        CAM_RRZO_FH_SPARE_12;                            /* 149C, 0x1A00549C, CAM_A_RRZO_FH_SPARE_12 */
    CAM_REG_RRZO_FH_SPARE_13                        CAM_RRZO_FH_SPARE_13;                            /* 14A0, 0x1A0054A0, CAM_A_RRZO_FH_SPARE_13 */
    UINT32                                          rsv_14a4;                                        /* 14A4, 0x1A0054A4,  */
    UINT32                                          rsv_14a8;                                        /* 14A8, 0x1A0054A8,  */
    UINT32                                          rsv14ac;                                         /* 14AC, 0x1A0054AC,  */
    UINT32                                          rsv_14B0;                                        /* 14B0, 0x1A0054B0 */
    CAM_REG_AAO_FH_SPARE_2                          CAM_AAO_FH_SPARE_2;                              /* 14B4, 0x1A0054B4, CAM_A_AAO_FH_SPARE_2 */
    CAM_REG_AAO_FH_SPARE_3                          CAM_AAO_FH_SPARE_3;                              /* 14B8, 0x1A0054B8, CAM_A_AAO_FH_SPARE_3 */
    CAM_REG_AAO_FH_SPARE_4                          CAM_AAO_FH_SPARE_4;                              /* 14BC, 0x1A0054BC, CAM_A_AAO_FH_SPARE_4 */
    CAM_REG_AAO_FH_SPARE_5                          CAM_AAO_FH_SPARE_5;                              /* 14C0, 0x1A0054C0, CAM_A_AAO_FH_SPARE_5 */
    CAM_REG_AAO_FH_SPARE_6                          CAM_AAO_FH_SPARE_6;                              /* 14C4, 0x1A0054C4, CAM_A_AAO_FH_SPARE_6 */
    CAM_REG_AAO_FH_SPARE_7                          CAM_AAO_FH_SPARE_7;                              /* 14C8, 0x1A0054C8, CAM_A_AAO_FH_SPARE_7 */
    CAM_REG_AAO_FH_SPARE_8                          CAM_AAO_FH_SPARE_8;                              /* 14CC, 0x1A0054CC, CAM_A_AAO_FH_SPARE_8 */
    CAM_REG_AAO_FH_SPARE_9                          CAM_AAO_FH_SPARE_9;                              /* 14D0, 0x1A0054D0, CAM_A_AAO_FH_SPARE_9 */
    CAM_REG_AAO_FH_SPARE_10                         CAM_AAO_FH_SPARE_10;                             /* 14D4, 0x1A0054D4, CAM_A_AAO_FH_SPARE_10 */
    CAM_REG_AAO_FH_SPARE_11                         CAM_AAO_FH_SPARE_11;                             /* 14D8, 0x1A0054D8, CAM_A_AAO_FH_SPARE_11 */
    CAM_REG_AAO_FH_SPARE_12                         CAM_AAO_FH_SPARE_12;                             /* 14DC, 0x1A0054DC, CAM_A_AAO_FH_SPARE_12 */
    CAM_REG_AAO_FH_SPARE_13                         CAM_AAO_FH_SPARE_13;                             /* 14E0, 0x1A0054E0, CAM_A_AAO_FH_SPARE_13 */
    UINT32                                          rsv_14e4;                                        /* 14E4, 0x1A0054E4,  */
    UINT32                                          rsv_14e8;                                        /* 14E8, 0x1A0054E8,  */
    UINT32                                          rsv_14ec;                                        /* 14EC, 0x1A0054EC,  */
    UINT32                                          rsv_14F0;                                        /* 14F0, 0x1A0054F0 */
    CAM_REG_AFO_FH_SPARE_2                          CAM_AFO_FH_SPARE_2;                              /* 14F4, 0x1A0054F4, CAM_A_AFO_FH_SPARE_2 */
    CAM_REG_AFO_FH_SPARE_3                          CAM_AFO_FH_SPARE_3;                              /* 14F8, 0x1A0054F8, CAM_A_AFO_FH_SPARE_3 */
    CAM_REG_AFO_FH_SPARE_4                          CAM_AFO_FH_SPARE_4;                              /* 14FC, 0x1A0054FC, CAM_A_AFO_FH_SPARE_4 */
    CAM_REG_AFO_FH_SPARE_5                          CAM_AFO_FH_SPARE_5;                              /* 1500, 0x1A005500, CAM_A_AFO_FH_SPARE_5 */
    CAM_REG_AFO_FH_SPARE_6                          CAM_AFO_FH_SPARE_6;                              /* 1504, 0x1A005504, CAM_A_AFO_FH_SPARE_6 */
    CAM_REG_AFO_FH_SPARE_7                          CAM_AFO_FH_SPARE_7;                              /* 1508, 0x1A005508, CAM_A_AFO_FH_SPARE_7 */
    CAM_REG_AFO_FH_SPARE_8                          CAM_AFO_FH_SPARE_8;                              /* 150C, 0x1A00550C, CAM_A_AFO_FH_SPARE_8 */
    CAM_REG_AFO_FH_SPARE_9                          CAM_AFO_FH_SPARE_9;                              /* 1510, 0x1A005510, CAM_A_AFO_FH_SPARE_9 */
    CAM_REG_AFO_FH_SPARE_10                         CAM_AFO_FH_SPARE_10;                             /* 1514, 0x1A005514, CAM_A_AFO_FH_SPARE_10 */
    CAM_REG_AFO_FH_SPARE_11                         CAM_AFO_FH_SPARE_11;                             /* 1518, 0x1A005518, CAM_A_AFO_FH_SPARE_11 */
    CAM_REG_AFO_FH_SPARE_12                         CAM_AFO_FH_SPARE_12;                             /* 151C, 0x1A00551C, CAM_A_AFO_FH_SPARE_12 */
    CAM_REG_AFO_FH_SPARE_13                         CAM_AFO_FH_SPARE_13;                             /* 1520, 0x1A005520, CAM_A_AFO_FH_SPARE_13 */
    UINT32                                          rsv_1524;                                        /* 1524, 0x1A005524,  */
    UINT32                                          rsv_1528;                                        /* 1528, 0x1A005528,  */
    UINT32                                          rsv_152c;                                        /* 152C, 0x1A00552C,  */
    UINT32                                          rsv_1530;                                        /* 1530, 0x1A005530 */
    CAM_REG_LCSO_FH_SPARE_2                         CAM_LCSO_FH_SPARE_2;                             /* 1534, 0x1A005534, CAM_A_LCSO_FH_SPARE_2 */
    CAM_REG_LCSO_FH_SPARE_3                         CAM_LCSO_FH_SPARE_3;                             /* 1538, 0x1A005538, CAM_A_LCSO_FH_SPARE_3 */
    CAM_REG_LCSO_FH_SPARE_4                         CAM_LCSO_FH_SPARE_4;                             /* 153C, 0x1A00553C, CAM_A_LCSO_FH_SPARE_4 */
    CAM_REG_LCSO_FH_SPARE_5                         CAM_LCSO_FH_SPARE_5;                             /* 1540, 0x1A005540, CAM_A_LCSO_FH_SPARE_5 */
    CAM_REG_LCSO_FH_SPARE_6                         CAM_LCSO_FH_SPARE_6;                             /* 1544, 0x1A005544, CAM_A_LCSO_FH_SPARE_6 */
    CAM_REG_LCSO_FH_SPARE_7                         CAM_LCSO_FH_SPARE_7;                             /* 1548, 0x1A005548, CAM_A_LCSO_FH_SPARE_7 */
    CAM_REG_LCSO_FH_SPARE_8                         CAM_LCSO_FH_SPARE_8;                             /* 154C, 0x1A00554C, CAM_A_LCSO_FH_SPARE_8 */
    CAM_REG_LCSO_FH_SPARE_9                         CAM_LCSO_FH_SPARE_9;                             /* 1550, 0x1A005550, CAM_A_LCSO_FH_SPARE_9 */
    CAM_REG_LCSO_FH_SPARE_10                        CAM_LCSO_FH_SPARE_10;                            /* 1554, 0x1A005554, CAM_A_LCSO_FH_SPARE_10 */
    CAM_REG_LCSO_FH_SPARE_11                        CAM_LCSO_FH_SPARE_11;                            /* 1558, 0x1A005558, CAM_A_LCSO_FH_SPARE_11 */
    CAM_REG_LCSO_FH_SPARE_12                        CAM_LCSO_FH_SPARE_12;                            /* 155C, 0x1A00555C, CAM_A_LCSO_FH_SPARE_12 */
    CAM_REG_LCSO_FH_SPARE_13                        CAM_LCSO_FH_SPARE_13;                            /* 1560, 0x1A005560, CAM_A_LCSO_FH_SPARE_13 */
    UINT32                                          rsv_1564;                                        /* 1564, 0x1A005564,  */
    UINT32                                          rsv_1568;                                        /* 1568, 0x1A005568,  */
    UINT32                                          rsv_156c;                                        /* 156C, 0x1A00556C,  */
    UINT32                                          rsv_1570;                                        /* 1570, 0x1A005570 */
    CAM_REG_UFEO_FH_SPARE_2                         CAM_UFEO_FH_SPARE_2;                             /* 1574, 0x1A005574, CAM_A_UFEO_FH_SPARE_2 */
    CAM_REG_UFEO_FH_SPARE_3                         CAM_UFEO_FH_SPARE_3;                             /* 1578, 0x1A005578, CAM_A_UFEO_FH_SPARE_3 */
    CAM_REG_UFEO_FH_SPARE_4                         CAM_UFEO_FH_SPARE_4;                             /* 157C, 0x1A00557C, CAM_A_UFEO_FH_SPARE_4 */
    CAM_REG_UFEO_FH_SPARE_5                         CAM_UFEO_FH_SPARE_5;                             /* 1580, 0x1A005580, CAM_A_UFEO_FH_SPARE_5 */
    CAM_REG_UFEO_FH_SPARE_6                         CAM_UFEO_FH_SPARE_6;                             /* 1584, 0x1A005584, CAM_A_UFEO_FH_SPARE_6 */
    CAM_REG_UFEO_FH_SPARE_7                         CAM_UFEO_FH_SPARE_7;                             /* 1588, 0x1A005588, CAM_A_UFEO_FH_SPARE_7 */
    CAM_REG_UFEO_FH_SPARE_8                         CAM_UFEO_FH_SPARE_8;                             /* 158C, 0x1A00558C, CAM_A_UFEO_FH_SPARE_8 */
    CAM_REG_UFEO_FH_SPARE_9                         CAM_UFEO_FH_SPARE_9;                             /* 1590, 0x1A005590, CAM_A_UFEO_FH_SPARE_9 */
    CAM_REG_UFEO_FH_SPARE_10                        CAM_UFEO_FH_SPARE_10;                            /* 1594, 0x1A005594, CAM_A_UFEO_FH_SPARE_10 */
    CAM_REG_UFEO_FH_SPARE_11                        CAM_UFEO_FH_SPARE_11;                            /* 1598, 0x1A005598, CAM_A_UFEO_FH_SPARE_11 */
    CAM_REG_UFEO_FH_SPARE_12                        CAM_UFEO_FH_SPARE_12;                            /* 159C, 0x1A00559C, CAM_A_UFEO_FH_SPARE_12 */
    CAM_REG_UFEO_FH_SPARE_13                        CAM_UFEO_FH_SPARE_13;                            /* 15A0, 0x1A0055A0, CAM_A_UFEO_FH_SPARE_13 */
    UINT32                                          rsv_15a4;                                        /* 15A4, 0x1A0055A4,  */
    UINT32                                          rsv_15a8;                                        /* 15A8, 0x1A0055A8,  */
    UINT32                                          rsv_15ac;                                        /* 15AC, 0x1A0055AC,  */
    UINT32                                          rsv_15B0;                                        /* 15B0, 0x1A0055B0 */
    CAM_REG_PDO_FH_SPARE_2                          CAM_PDO_FH_SPARE_2;                              /* 15B4, 0x1A0055B4, CAM_A_PDO_FH_SPARE_2 */
    CAM_REG_PDO_FH_SPARE_3                          CAM_PDO_FH_SPARE_3;                              /* 15B8, 0x1A0055B8, CAM_A_PDO_FH_SPARE_3 */
    CAM_REG_PDO_FH_SPARE_4                          CAM_PDO_FH_SPARE_4;                              /* 15BC, 0x1A0055BC, CAM_A_PDO_FH_SPARE_4 */
    CAM_REG_PDO_FH_SPARE_5                          CAM_PDO_FH_SPARE_5;                              /* 15C0, 0x1A0055C0, CAM_A_PDO_FH_SPARE_5 */
    CAM_REG_PDO_FH_SPARE_6                          CAM_PDO_FH_SPARE_6;                              /* 15C4, 0x1A0055C4, CAM_A_PDO_FH_SPARE_6 */
    CAM_REG_PDO_FH_SPARE_7                          CAM_PDO_FH_SPARE_7;                              /* 15C8, 0x1A0055C8, CAM_A_PDO_FH_SPARE_7 */
    CAM_REG_PDO_FH_SPARE_8                          CAM_PDO_FH_SPARE_8;                              /* 15CC, 0x1A0055CC, CAM_A_PDO_FH_SPARE_8 */
    CAM_REG_PDO_FH_SPARE_9                          CAM_PDO_FH_SPARE_9;                              /* 15D0, 0x1A0055D0, CAM_A_PDO_FH_SPARE_9 */
    CAM_REG_PDO_FH_SPARE_10                         CAM_PDO_FH_SPARE_10;                             /* 15D4, 0x1A0055D4, CAM_A_PDO_FH_SPARE_10 */
    CAM_REG_PDO_FH_SPARE_11                         CAM_PDO_FH_SPARE_11;                             /* 15D8, 0x1A0055D8, CAM_A_PDO_FH_SPARE_11 */
    CAM_REG_PDO_FH_SPARE_12                         CAM_PDO_FH_SPARE_12;                             /* 15DC, 0x1A0055DC, CAM_A_PDO_FH_SPARE_12 */
    CAM_REG_PDO_FH_SPARE_13                         CAM_PDO_FH_SPARE_13;                             /* 15E0, 0x1A0055E0, CAM_A_PDO_FH_SPARE_13 */
    UINT32                                          rsv_15e4;                                        /* 15E4, 0x1A0055E4,  */
    UINT32                                          rsv_15e8;                                        /* 15E8, 0x1A0055E8,  */
    UINT32                                          rsv_15ec;                                        /* 15EC, 0x1A0055EC,  */
    CAM_REG_PSO_FH_SPARE_4                          CAM_PSO_FH_SPARE_4;                              /* 15F0, 0x1A0055F0, CAM_A_PSO_FH_SPARE_4 */
    CAM_REG_PSO_FH_SPARE_2                          CAM_PSO_FH_SPARE_2;                              /* 15F4, 0x1A0055F4, CAM_A_PSO_FH_SPARE_2 */
    CAM_REG_PSO_FH_SPARE_3                          CAM_PSO_FH_SPARE_3;                              /* 15F8, 0x1A0055F8, CAM_A_PSO_FH_SPARE_3 */
    UINT32                                          rsv_15FC;                                        /* 15FC, 0x1A0055FC */
    CAM_REG_PSO_FH_SPARE_5                          CAM_PSO_FH_SPARE_5;                              /* 1600, 0x1A005600, CAM_A_PSO_FH_SPARE_5 */
    CAM_REG_PSO_FH_SPARE_6                          CAM_PSO_FH_SPARE_6;                              /* 1604, 0x1A005604, CAM_A_PSO_FH_SPARE_6 */
    CAM_REG_PSO_FH_SPARE_7                          CAM_PSO_FH_SPARE_7;                              /* 1608, 0x1A005608, CAM_A_PSO_FH_SPARE_7 */
    CAM_REG_PSO_FH_SPARE_8                          CAM_PSO_FH_SPARE_8;                              /* 160C, 0x1A00560C, CAM_A_PSO_FH_SPARE_8 */
    UINT32                                          rsv_1610;                                        /* 1610, 0x1A005610 */
    CAM_REG_PSO_FH_SPARE_9                          CAM_PSO_FH_SPARE_9;                              /* 1614, 0x1A005614, CAM_A_PSO_FH_SPARE_9 */
    CAM_REG_PSO_FH_SPARE_10                         CAM_PSO_FH_SPARE_10;                             /* 1618, 0x1A005618, CAM_A_PSO_FH_SPARE_10 */
    CAM_REG_PSO_FH_SPARE_11                         CAM_PSO_FH_SPARE_11;                             /* 161C, 0x1A00561C, CAM_A_PSO_FH_SPARE_11 */
    CAM_REG_PSO_FH_SPARE_12                         CAM_PSO_FH_SPARE_12;                             /* 1620, 0x1A005620, CAM_A_PSO_FH_SPARE_12 */
    CAM_REG_PSO_FH_SPARE_13                         CAM_PSO_FH_SPARE_13;                             /* 1624, 0x1A005624, CAM_A_PSO_FH_SPARE_13 */
    UINT32                                          rsv_1628;                                        /* 1628, 0x1A005628,  */
    UINT32                                          rsv_162c;                                        /* 162C, 0x1A00562C,  */
    UINT32                                          rsv_1630;                                        /* 1630, 0x1A005630,  */
    CAM_REG_LMVO_FH_SPARE_2                         CAM_LMVO_FH_SPARE_2;                             /* 1634, 0x1A005634, CAM_A_LMVO_FH_SPARE_2 */
    CAM_REG_LMVO_FH_SPARE_3                         CAM_LMVO_FH_SPARE_3;                             /* 1638, 0x1A005638, CAM_A_LMVO_FH_SPARE_3 */
    CAM_REG_LMVO_FH_SPARE_4                         CAM_LMVO_FH_SPARE_4;                             /* 163C, 0x1A00563C, CAM_A_LMVO_FH_SPARE_4 */
    CAM_REG_LMVO_FH_SPARE_5                         CAM_LMVO_FH_SPARE_5;                             /* 1640, 0x1A005640, CAM_A_LMVO_FH_SPARE_5 */
    CAM_REG_LMVO_FH_SPARE_6                         CAM_LMVO_FH_SPARE_6;                             /* 1644, 0x1A005644, CAM_A_LMVO_FH_SPARE_6 */
    CAM_REG_LMVO_FH_SPARE_7                         CAM_LMVO_FH_SPARE_7;                             /* 1648, 0x1A005648, CAM_A_LMVO_FH_SPARE_7 */
    CAM_REG_LMVO_FH_SPARE_8                         CAM_LMVO_FH_SPARE_8;                             /* 164C, 0x1A00564C, CAM_A_LMVO_FH_SPARE_8 */
    UINT32                                          rsv_1650;                                        /* 1650, 0x1A005650 */
    CAM_REG_LMVO_FH_SPARE_9                         CAM_LMVO_FH_SPARE_9;                             /* 1654, 0x1A005654, CAM_A_LMVO_FH_SPARE_9 */
    CAM_REG_LMVO_FH_SPARE_10                        CAM_LMVO_FH_SPARE_10;                            /* 1658, 0x1A005658, CAM_A_LMVO_FH_SPARE_10 */
    CAM_REG_LMVO_FH_SPARE_11                        CAM_LMVO_FH_SPARE_11;                            /* 165C, 0x1A00565C, CAM_A_LMVO_FH_SPARE_11 */
    CAM_REG_LMVO_FH_SPARE_12                        CAM_LMVO_FH_SPARE_12;                            /* 1660, 0x1A005660, CAM_A_LMVO_FH_SPARE_12 */
    CAM_REG_LMVO_FH_SPARE_13                        CAM_LMVO_FH_SPARE_13;                            /* 1664, 0x1A005664, CAM_A_LMVO_FH_SPARE_13 */
    UINT32                                          rsv_1668;                                        /* 1668, 0x1A005668,  */
    UINT32                                          rsv_166c;                                        /* 166C, 0x1A00566C,  */
    UINT32                                          rsv_1670;                                        /* 1670, 0x1A005670,  */
    CAM_REG_FLKO_FH_SPARE_2                         CAM_FLKO_FH_SPARE_2;                             /* 1674, 0x1A005674, CAM_A_FLKO_FH_SPARE_2 */
    CAM_REG_FLKO_FH_SPARE_3                         CAM_FLKO_FH_SPARE_3;                             /* 1678, 0x1A005678, CAM_A_FLKO_FH_SPARE_3 */
    CAM_REG_FLKO_FH_SPARE_4                         CAM_FLKO_FH_SPARE_4;                             /* 167C, 0x1A00567C, CAM_A_FLKO_FH_SPARE_4 */
    CAM_REG_FLKO_FH_SPARE_5                         CAM_FLKO_FH_SPARE_5;                             /* 1680, 0x1A005680, CAM_A_FLKO_FH_SPARE_5 */
    CAM_REG_FLKO_FH_SPARE_6                         CAM_FLKO_FH_SPARE_6;                             /* 1684, 0x1A005684, CAM_A_FLKO_FH_SPARE_6 */
    CAM_REG_FLKO_FH_SPARE_7                         CAM_FLKO_FH_SPARE_7;                             /* 1688, 0x1A005688, CAM_A_FLKO_FH_SPARE_7 */
    CAM_REG_FLKO_FH_SPARE_8                         CAM_FLKO_FH_SPARE_8;                             /* 168C, 0x1A00568C, CAM_A_FLKO_FH_SPARE_8 */
    UINT32                                          rsv_1690;                                        /* 1690, 0x1A005690 */
    CAM_REG_FLKO_FH_SPARE_9                         CAM_FLKO_FH_SPARE_9;                             /* 1694, 0x1A005694, CAM_A_FLKO_FH_SPARE_9 */
    CAM_REG_FLKO_FH_SPARE_10                        CAM_FLKO_FH_SPARE_10;                            /* 1698, 0x1A005698, CAM_A_FLKO_FH_SPARE_10 */
    CAM_REG_FLKO_FH_SPARE_11                        CAM_FLKO_FH_SPARE_11;                            /* 169C, 0x1A00569C, CAM_A_FLKO_FH_SPARE_11 */
    CAM_REG_FLKO_FH_SPARE_12                        CAM_FLKO_FH_SPARE_12;                            /* 16A0, 0x1A0056A0, CAM_A_FLKO_FH_SPARE_12 */
    CAM_REG_FLKO_FH_SPARE_13                        CAM_FLKO_FH_SPARE_13;                            /* 16A4, 0x1A0056A4, CAM_A_FLKO_FH_SPARE_13 */
    UINT32                                          rsv_16a8;                                        /* 16A8, 0x1A0056A8,  */
    UINT32                                          rsv_16ac;                                        /* 16AC, 0x1A0056AC,  */
    UINT32                                          rsv_16b0;                                        /* 16B0, 0x1A0056B0,  */
    CAM_REG_RSSO_A_FH_SPARE_2                       CAM_RSSO_A_FH_SPARE_2;                           /* 16B4, 0x1A0056B4, CAM_A_RSSO_A_FH_SPARE_2 */
    CAM_REG_RSSO_A_FH_SPARE_3                       CAM_RSSO_A_FH_SPARE_3;                           /* 16B8, 0x1A0056B8, CAM_A_RSSO_A_FH_SPARE_3 */
    CAM_REG_RSSO_A_FH_SPARE_4                       CAM_RSSO_A_FH_SPARE_4;                           /* 16BC, 0x1A0056BC, CAM_A_RSSO_A_FH_SPARE_4 */
    UINT32                                          rsv_16C0[8];                                     /* 16C0..16DF, 0x1A0056C0..1A0056DF */
    CAM_REG_RSSO_A_FH_SPARE_5                       CAM_RSSO_A_FH_SPARE_5;                           /* 16E0, 0x1A0056E0, CAM_A_RSSO_A_FH_SPARE_5 */
    CAM_REG_RSSO_A_FH_SPARE_6                       CAM_RSSO_A_FH_SPARE_6;                           /* 16E4, 0x1A0056E4, CAM_A_RSSO_A_FH_SPARE_6 */
    CAM_REG_RSSO_A_FH_SPARE_7                       CAM_RSSO_A_FH_SPARE_7;                           /* 16E8, 0x1A0056E8, CAM_A_RSSO_A_FH_SPARE_7 */
    CAM_REG_RSSO_A_FH_SPARE_8                       CAM_RSSO_A_FH_SPARE_8;                           /* 16EC, 0x1A0056EC, CAM_A_RSSO_A_FH_SPARE_8 */
    UINT32                                          rsv_16F0;                                        /* 16F0, 0x1A0056F0 */
    CAM_REG_RSSO_A_FH_SPARE_9                       CAM_RSSO_A_FH_SPARE_9;                           /* 16F4, 0x1A0056F4, CAM_A_RSSO_A_FH_SPARE_9 */
    CAM_REG_RSSO_A_FH_SPARE_10                      CAM_RSSO_A_FH_SPARE_10;                          /* 16F8, 0x1A0056F8, CAM_A_RSSO_A_FH_SPARE_10 */
    CAM_REG_RSSO_A_FH_SPARE_11                      CAM_RSSO_A_FH_SPARE_11;                          /* 16FC, 0x1A0056FC, CAM_A_RSSO_A_FH_SPARE_11 */
    CAM_REG_RSSO_A_FH_SPARE_12                      CAM_RSSO_A_FH_SPARE_12;                          /* 1700, 0x1A005700, CAM_A_RSSO_A_FH_SPARE_12 */
    CAM_REG_RSSO_A_FH_SPARE_13                      CAM_RSSO_A_FH_SPARE_13;                          /* 1704, 0x1A005704, CAM_A_RSSO_A_FH_SPARE_13 */
    UINT32                                          rsv_1708;                                        /* 1708, 0x1A005708,  */
    UINT32                                          rsv_170c;                                        /* 170C, 0x1A00570C,  */
    UINT32                                          rsv_1710;                                        /* 1710, 0x1A005710,  */
    CAM_REG_UFGO_FH_SPARE_2                         CAM_UFGO_FH_SPARE_2;                             /* 1714, 0x1A005714, CAM_A_UFGO_FH_SPARE_2 */
    CAM_REG_UFGO_FH_SPARE_3                         CAM_UFGO_FH_SPARE_3;                             /* 1718, 0x1A005718, CAM_A_UFGO_FH_SPARE_3 */
    CAM_REG_UFGO_FH_SPARE_4                         CAM_UFGO_FH_SPARE_4;                             /* 171C, 0x1A00571C, CAM_A_UFGO_FH_SPARE_4 */
    CAM_REG_UFGO_FH_SPARE_5                         CAM_UFGO_FH_SPARE_5;                             /* 1720, 0x1A005720, CAM_A_UFGO_FH_SPARE_5 */
    CAM_REG_UFGO_FH_SPARE_6                         CAM_UFGO_FH_SPARE_6;                             /* 1724, 0x1A005724, CAM_A_UFGO_FH_SPARE_6 */
    CAM_REG_UFGO_FH_SPARE_7                         CAM_UFGO_FH_SPARE_7;                             /* 1728, 0x1A005728, CAM_A_UFGO_FH_SPARE_7 */
    CAM_REG_UFGO_FH_SPARE_8                         CAM_UFGO_FH_SPARE_8;                             /* 172C, 0x1A00572C, CAM_A_UFGO_FH_SPARE_8 */
    CAM_REG_UFGO_FH_SPARE_9                         CAM_UFGO_FH_SPARE_9;                             /* 1730, 0x1A005730, CAM_A_UFGO_FH_SPARE_9 */
    CAM_REG_UFGO_FH_SPARE_10                        CAM_UFGO_FH_SPARE_10;                            /* 1734, 0x1A005734, CAM_A_UFGO_FH_SPARE_10 */
    CAM_REG_UFGO_FH_SPARE_11                        CAM_UFGO_FH_SPARE_11;                            /* 1738, 0x1A005738, CAM_A_UFGO_FH_SPARE_11 */
    CAM_REG_UFGO_FH_SPARE_12                        CAM_UFGO_FH_SPARE_12;                            /* 173C, 0x1A00573C, CAM_A_UFGO_FH_SPARE_12 */
    CAM_REG_UFGO_FH_SPARE_13                        CAM_UFGO_FH_SPARE_13;                            /* 1740, 0x1A005740, CAM_A_UFGO_FH_SPARE_13 */
    UINT32                                          rsv_1744;                                        /* 1744, 0x1A005744,  */
    UINT32                                          rsv_1748;                                        /* 1748, 0x1A005748,  */
    UINT32                                          rsv_174c;                                        /* 174C, 0x1A00574C,  */
    UINT32                                          rsv_1750[556];                                   /* 1750..1FFC, 1A005750..1A005FFC */
    //below para is over 0x1000, for CQ baseaddr only.
    //in order to reduce memory space, because of inner register is at  + 0x20000, a special flag will be set when over 0x2000.
    MUINT32                                         CAM_CQ_THRE0_ADDR_INNER;                        // should map to 0x4198 + 0x20000,   2000
    MUINT32                                         CAM_CQ_THRE1_ADDR_INNER;                        // should map to 0x41A4 + 0x20000,   2004
    MUINT32                                         CAM_CQ_THRE2_ADDR_INNER;                        // should map to 0x41B0 + 0x20000,   2008
    MUINT32                                         CAM_CQ_THRE3_ADDR_INNER;                        // should map to 0x41BC + 0x20000,   200C
    MUINT32                                         CAM_CQ_THRE4_ADDR_INNER;                        // should map to 0x41C8 + 0x20000,   2010
    MUINT32                                         CAM_CQ_THRE5_ADDR_INNER;                        // should map to 0x41D4 + 0x20000,   2014
    MUINT32                                         CAM_CQ_THRE6_ADDR_INNER;                        // should map to 0x41E0 + 0x20000,   2018
    MUINT32                                         CAM_CQ_THRE7_ADDR_INNER;                        // should map to 0x41EC + 0x20000,   201C
    MUINT32                                         CAM_CQ_THRE8_ADDR_INNER;                        // should map to 0x41F8 + 0x20000,   2020
    MUINT32                                         CAM_CQ_THRE9_ADDR_INNER;                        // should map to 0x4204 + 0x20000,   2024
    MUINT32                                         CAM_CQ_THRE10_ADDR_INNER;                       // should map to 0x4210 + 0x20000,  2028
    MUINT32                                         CAM_CQ_THRE11_ADDR_INNER;                       // should map to 0x421C + 0x20000,  202C         /* 0DEC..0DFF, 1A004DEC..1A004DFF */
    MUINT32                                         CAM_CQ_THRE12_ADDR_INNER;                       // should map to 0x4228 + 0x20000,  2030         /* 0DEC..0DFF, 1A004DEC..1A004DFF */
    MUINT32                                         TWIN_CQ_THRE0_ADDR;                                    // should map to 0x4198 or 0x6198,   2034         this is for master cam's cq to set slave cam's cq-base-addr
    MUINT32                                         TWIN_CQ_THRE1_ADDR;                                    // should map to 0x41A4 or 0x61A4,   2038         this is for master cam's cq to set slave cam's cq-base-addr
    MUINT32                                         TWIN_CQ_THRE10_ADDR;                                  // should map to 0x4210 or 0x6210,   203C         this is for master cam's cq to set slave cam's cq-base-addr
}cam_reg_t;

typedef volatile struct _camsv_reg_t_   /* 0x1A050000..0x1A050E6B */
{
    CAMSV_REG_DMA_SOFT_RSTSTAT                      CAMSV_DMA_SOFT_RSTSTAT;                          /* 0000, 0x1A050000, CAMSV_0_DMA_SOFT_RSTSTAT */
    CAMSV_REG_CQ0I_BASE_ADDR                        CAMSV_CQ0I_BASE_ADDR;                            /* 0004, 0x1A050004, CAMSV_0_CQ0I_BASE_ADDR */
    CAMSV_REG_CQ0I_XSIZE                            CAMSV_CQ0I_XSIZE;                                /* 0008, 0x1A050008, CAMSV_0_CQ0I_XSIZE */
    CAMSV_REG_VERTICAL_FLIP_EN                      CAMSV_VERTICAL_FLIP_EN;                          /* 000C, 0x1A05000C, CAMSV_0_VERTICAL_FLIP_EN */
    CAMSV_REG_DMA_SOFT_RESET                        CAMSV_DMA_SOFT_RESET;                            /* 0010, 0x1A050010, CAMSV_0_DMA_SOFT_RESET */
    CAMSV_REG_LAST_ULTRA_EN                         CAMSV_LAST_ULTRA_EN;                             /* 0014, 0x1A050014, CAMSV_0_LAST_ULTRA_EN */
    CAMSV_REG_SPECIAL_FUN_EN                        CAMSV_SPECIAL_FUN_EN;                            /* 0018, 0x1A050018, CAMSV_0_SPECIAL_FUN_EN */
    UINT32                                          rsv_001C;                                        /* 001C, 0x1A05001C */
    CAMSV_REG_IMGO_BASE_ADDR                        CAMSV_IMGO_BASE_ADDR;                            /* 0020, 0x1A050020, CAMSV_0_IMGO_BASE_ADDR */
    UINT32                                          rsv_0024;                                        /* 0024, 0x1A050024 */
    CAMSV_REG_IMGO_OFST_ADDR                        CAMSV_IMGO_OFST_ADDR;                            /* 0028, 0x1A050028, CAMSV_0_IMGO_OFST_ADDR */
    CAMSV_REG_IMGO_DRS                              CAMSV_IMGO_DRS;                                  /* 002C, 0x1A05002C, CAMSV_0_IMGO_DRS */
    CAMSV_REG_IMGO_XSIZE                            CAMSV_IMGO_XSIZE;                                /* 0030, 0x1A050030, CAMSV_0_IMGO_XSIZE */
    CAMSV_REG_IMGO_YSIZE                            CAMSV_IMGO_YSIZE;                                /* 0034, 0x1A050034, CAMSV_0_IMGO_YSIZE */
    CAMSV_REG_IMGO_STRIDE                           CAMSV_IMGO_STRIDE;                               /* 0038, 0x1A050038, CAMSV_0_IMGO_STRIDE */
    CAMSV_REG_IMGO_CON                              CAMSV_IMGO_CON;                                  /* 003C, 0x1A05003C, CAMSV_0_IMGO_CON */
    CAMSV_REG_IMGO_CON2                             CAMSV_IMGO_CON2;                                 /* 0040, 0x1A050040, CAMSV_0_IMGO_CON2 */
    CAMSV_REG_IMGO_CON3                             CAMSV_IMGO_CON3;                                 /* 0044, 0x1A050044, CAMSV_0_IMGO_CON3 */
    CAMSV_REG_IMGO_CROP                             CAMSV_IMGO_CROP;                                 /* 0048, 0x1A050048, CAMSV_0_IMGO_CROP */
    UINT32                                          rsv_004C[49];                                    /* 004C..010F, 0x1A05004C..1A05010F */
    CAMSV_REG_FBC_IMGO_CTL1                         CAMSV_FBC_IMGO_CTL1;                             /* 0110, 0x1A050110, CAMSV_0_FBC_IMGO_CTL1 */
    CAMSV_REG_FBC_IMGO_CTL2                         CAMSV_FBC_IMGO_CTL2;                             /* 0114, 0x1A050114, CAMSV_0_FBC_IMGO_CTL2 */
    CAMSV_REG_FBC_IMGO_ENQ_ADDR                     CAMSV_FBC_IMGO_ENQ_ADDR;                         /* 0118, 0x1A050118, CAMSV_0_FBC_IMGO_ENQ_ADDR */
    CAMSV_REG_FBC_IMGO_CUR_ADDR                     CAMSV_FBC_IMGO_CUR_ADDR;                         /* 011C, 0x1A05011C, CAMSV_0_FBC_IMGO_CUR_ADDR */
    UINT32                                          rsv_0120[68];                                    /* 0120..022F, 0x1A050120..1A05022F */
    CAMSV_REG_TG_SEN_MODE                           CAMSV_TG_SEN_MODE;                               /* 0230, 0x1A050230, CAMSV_0_TG_SEN_MODE */
    CAMSV_REG_TG_VF_CON                             CAMSV_TG_VF_CON;                                 /* 0234, 0x1A050234, CAMSV_0_TG_VF_CON */
    CAMSV_REG_TG_SEN_GRAB_PXL                       CAMSV_TG_SEN_GRAB_PXL;                           /* 0238, 0x1A050238, CAMSV_0_TG_SEN_GRAB_PXL */
    CAMSV_REG_TG_SEN_GRAB_LIN                       CAMSV_TG_SEN_GRAB_LIN;                           /* 023C, 0x1A05023C, CAMSV_0_TG_SEN_GRAB_LIN */
    CAMSV_REG_TG_PATH_CFG                           CAMSV_TG_PATH_CFG;                               /* 0240, 0x1A050240, CAMSV_0_TG_PATH_CFG */
    CAMSV_REG_TG_MEMIN_CTL                          CAMSV_TG_MEMIN_CTL;                              /* 0244, 0x1A050244, CAMSV_0_TG_MEMIN_CTL */
    CAMSV_REG_TG_INT1                               CAMSV_TG_INT1;                                   /* 0248, 0x1A050248, CAMSV_0_TG_INT1 */
    CAMSV_REG_TG_INT2                               CAMSV_TG_INT2;                                   /* 024C, 0x1A05024C, CAMSV_0_TG_INT2 */
    CAMSV_REG_TG_SOF_CNT                            CAMSV_TG_SOF_CNT;                                /* 0250, 0x1A050250, CAMSV_0_TG_SOF_CNT */
    CAMSV_REG_TG_SOT_CNT                            CAMSV_TG_SOT_CNT;                                /* 0254, 0x1A050254, CAMSV_0_TG_SOT_CNT */
    CAMSV_REG_TG_EOT_CNT                            CAMSV_TG_EOT_CNT;                                /* 0258, 0x1A050258, CAMSV_0_TG_EOT_CNT */
    CAMSV_REG_TG_ERR_CTL                            CAMSV_TG_ERR_CTL;                                /* 025C, 0x1A05025C, CAMSV_0_TG_ERR_CTL */
    CAMSV_REG_TG_DAT_NO                             CAMSV_TG_DAT_NO;                                 /* 0260, 0x1A050260, CAMSV_0_TG_DAT_NO */
    CAMSV_REG_TG_FRM_CNT_ST                         CAMSV_TG_FRM_CNT_ST;                             /* 0264, 0x1A050264, CAMSV_0_TG_FRM_CNT_ST */
    CAMSV_REG_TG_FRMSIZE_ST                         CAMSV_TG_FRMSIZE_ST;                             /* 0268, 0x1A050268, CAMSV_0_TG_FRMSIZE_ST */
    CAMSV_REG_TG_INTER_ST                           CAMSV_TG_INTER_ST;                               /* 026C, 0x1A05026C, CAMSV_0_TG_INTER_ST */
    CAMSV_REG_TG_FLASHA_CTL                         CAMSV_TG_FLASHA_CTL;                             /* 0270, 0x1A050270, CAMSV_0_TG_FLASHA_CTL */
    CAMSV_REG_TG_FLASHA_LINE_CNT                    CAMSV_TG_FLASHA_LINE_CNT;                        /* 0274, 0x1A050274, CAMSV_0_TG_FLASHA_LINE_CNT */
    CAMSV_REG_TG_FLASHA_POS                         CAMSV_TG_FLASHA_POS;                             /* 0278, 0x1A050278, CAMSV_0_TG_FLASHA_POS */
    CAMSV_REG_TG_FLASHB_CTL                         CAMSV_TG_FLASHB_CTL;                             /* 027C, 0x1A05027C, CAMSV_0_TG_FLASHB_CTL */
    CAMSV_REG_TG_FLASHB_LINE_CNT                    CAMSV_TG_FLASHB_LINE_CNT;                        /* 0280, 0x1A050280, CAMSV_0_TG_FLASHB_LINE_CNT */
    CAMSV_REG_TG_FLASHB_POS                         CAMSV_TG_FLASHB_POS;                             /* 0284, 0x1A050284, CAMSV_0_TG_FLASHB_POS */
    CAMSV_REG_TG_FLASHB_POS1                        CAMSV_TG_FLASHB_POS1;                            /* 0288, 0x1A050288, CAMSV_0_TG_FLASHB_POS1 */
    UINT32                                          rsv_028C;                                        /* 028C, 0x1A05028C */
    CAMSV_REG_TG_I2C_CQ_TRIG                        CAMSV_TG_I2C_CQ_TRIG;                            /* 0290, 0x1A050290, CAMSV_0_TG_I2C_CQ_TRIG */
    CAMSV_REG_TG_CQ_TIMING                          CAMSV_TG_CQ_TIMING;                              /* 0294, 0x1A050294, CAMSV_0_TG_CQ_TIMING */
    CAMSV_REG_TG_CQ_NUM                             CAMSV_TG_CQ_NUM;                                 /* 0298, 0x1A050298, CAMSV_0_TG_CQ_NUM */
    UINT32                                          rsv_029C;                                        /* 029C, 0x1A05029C */
    CAMSV_REG_TG_TIME_STAMP                         CAMSV_TG_TIME_STAMP;                             /* 02A0, 0x1A0502A0, CAMSV_0_TG_TIME_STAMP */
    CAMSV_REG_TG_SUB_PERIOD                         CAMSV_TG_SUB_PERIOD;                             /* 02A4, 0x1A0502A4, CAMSV_0_TG_SUB_PERIOD */
    CAMSV_REG_TG_DAT_NO_R                           CAMSV_TG_DAT_NO_R;                               /* 02A8, 0x1A0502A8, CAMSV_0_TG_DAT_NO_R */
    CAMSV_REG_TG_FRMSIZE_ST_R                       CAMSV_TG_FRMSIZE_ST_R;                           /* 02AC, 0x1A0502AC, CAMSV_0_TG_FRMSIZE_ST_R */
    CAMSV_REG_TG_TIME_STAMP_CTL                     CAMSV_TG_TIME_STAMP_CTL;                         /* 02B0, 0x1A0502B0, CAMSV_0_TG_TIME_STAMP_CTL */
    CAMSV_REG_TG_TIME_STAMP_MSB                     CAMSV_TG_TIME_STAMP_MSB;                         /* 02B4, 0x1A0502B4, CAMSV_0_TG_TIME_STAMP_MSB */
    UINT32                                          rsv_02B8[38];                                    /* 02B8..034F, 0x1A0502B8..1A05034F */
    CAMSV_REG_DMA_ERR_CTRL                          CAMSV_DMA_ERR_CTRL;                              /* 0350, 0x1A050350, CAMSV_0_DMA_ERR_CTRL */
    UINT32                                          rsv_0354[3];                                     /* 0354..035F, 0x1A050354..1A05035F */
    CAMSV_REG_IMGO_ERR_STAT                         CAMSV_IMGO_ERR_STAT;                             /* 0360, 0x1A050360, CAMSV_0_IMGO_ERR_STAT */
    UINT32                                          rsv_0364[18];                                    /* 0364..03AB, 0x1A050364..1A0503AB */
    CAMSV_REG_DMA_DEBUG_ADDR                        CAMSV_DMA_DEBUG_ADDR;                            /* 03AC, 0x1A0503AC, CAMSV_0_DMA_DEBUG_ADDR */
    CAMSV_REG_DMA_RSV1                              CAMSV_DMA_RSV1;                                  /* 03B0, 0x1A0503B0, CAMSV_0_DMA_RSV1 */
    CAMSV_REG_DMA_RSV2                              CAMSV_DMA_RSV2;                                  /* 03B4, 0x1A0503B4, CAMSV_0_DMA_RSV2 */
    CAMSV_REG_DMA_RSV3                              CAMSV_DMA_RSV3;                                  /* 03B8, 0x1A0503B8, CAMSV_0_DMA_RSV3 */
    CAMSV_REG_DMA_RSV4                              CAMSV_DMA_RSV4;                                  /* 03BC, 0x1A0503BC, CAMSV_0_DMA_RSV4 */
    CAMSV_REG_DMA_RSV5                              CAMSV_DMA_RSV5;                                  /* 03C0, 0x1A0503C0, CAMSV_0_DMA_RSV5 */
    CAMSV_REG_DMA_RSV6                              CAMSV_DMA_RSV6;                                  /* 03C4, 0x1A0503C4, CAMSV_0_DMA_RSV6 */
    CAMSV_REG_DMA_DEBUG_SEL                         CAMSV_DMA_DEBUG_SEL;                             /* 03C8, 0x1A0503C8, CAMSV_0_DMA_DEBUG_SEL */
    CAMSV_REG_DMA_BW_SELF_TEST                      CAMSV_DMA_BW_SELF_TEST;                          /* 03CC, 0x1A0503CC, CAMSV_0_DMA_BW_SELF_TEST */
    UINT32                                          rsv_03D0[12];                                    /* 03D0..03FF, 0x1A0503D0..1A0503FF */
    CAMSV_REG_DMA_FRAME_HEADER_EN                   CAMSV_DMA_FRAME_HEADER_EN;                       /* 0400, 0x1A050400, CAMSV_0_DMA_FRAME_HEADER_EN */
    CAMSV_REG_IMGO_FH_BASE_ADDR                     CAMSV_IMGO_FH_BASE_ADDR;                         /* 0404, 0x1A050404, CAMSV_0_IMGO_FH_BASE_ADDR */
    CAMSV_REG_RRZO_FH_BASE_ADDR                     CAMSV_RRZO_FH_BASE_ADDR;                         /* 0408, 0x1A050408, CAMSV_0_RRZO_FH_BASE_ADDR */
    CAMSV_REG_AAO_FH_BASE_ADDR                      CAMSV_AAO_FH_BASE_ADDR;                          /* 040C, 0x1A05040C, CAMSV_0_AAO_FH_BASE_ADDR */
    CAMSV_REG_AFO_FH_BASE_ADDR                      CAMSV_AFO_FH_BASE_ADDR;                          /* 0410, 0x1A050410, CAMSV_0_AFO_FH_BASE_ADDR */
    CAMSV_REG_LCSO_FH_BASE_ADDR                     CAMSV_LCSO_FH_BASE_ADDR;                         /* 0414, 0x1A050414, CAMSV_0_LCSO_FH_BASE_ADDR */
    CAMSV_REG_UFEO_FH_BASE_ADDR                     CAMSV_UFEO_FH_BASE_ADDR;                         /* 0418, 0x1A050418, CAMSV_0_UFEO_FH_BASE_ADDR */
    CAMSV_REG_PDO_FH_BASE_ADDR                      CAMSV_PDO_FH_BASE_ADDR;                          /* 041C, 0x1A05041C, CAMSV_0_PDO_FH_BASE_ADDR */
    UINT32                                          rsv_0420[5];                                     /* 0420..0433, 0x1A050420..1A050433 */
    CAMSV_REG_IMGO_FH_SPARE_2                       CAMSV_IMGO_FH_SPARE_2;                           /* 0434, 0x1A050434, CAMSV_0_IMGO_FH_SPARE_2 */
    CAMSV_REG_IMGO_FH_SPARE_3                       CAMSV_IMGO_FH_SPARE_3;                           /* 0438, 0x1A050438, CAMSV_0_IMGO_FH_SPARE_3 */
    CAMSV_REG_IMGO_FH_SPARE_4                       CAMSV_IMGO_FH_SPARE_4;                           /* 043C, 0x1A05043C, CAMSV_0_IMGO_FH_SPARE_4 */
    CAMSV_REG_IMGO_FH_SPARE_5                       CAMSV_IMGO_FH_SPARE_5;                           /* 0440, 0x1A050440, CAMSV_0_IMGO_FH_SPARE_5 */
    CAMSV_REG_IMGO_FH_SPARE_6                       CAMSV_IMGO_FH_SPARE_6;                           /* 0444, 0x1A050444, CAMSV_0_IMGO_FH_SPARE_6 */
    CAMSV_REG_IMGO_FH_SPARE_7                       CAMSV_IMGO_FH_SPARE_7;                           /* 0448, 0x1A050448, CAMSV_0_IMGO_FH_SPARE_7 */
    CAMSV_REG_IMGO_FH_SPARE_8                       CAMSV_IMGO_FH_SPARE_8;                           /* 044C, 0x1A05044C, CAMSV_0_IMGO_FH_SPARE_8 */
    CAMSV_REG_IMGO_FH_SPARE_9                       CAMSV_IMGO_FH_SPARE_9;                           /* 0450, 0x1A050450, CAMSV_0_IMGO_FH_SPARE_9 */
    CAMSV_REG_IMGO_FH_SPARE_10                      CAMSV_IMGO_FH_SPARE_10;                          /* 0454, 0x1A050454, CAMSV_0_IMGO_FH_SPARE_10 */
    CAMSV_REG_IMGO_FH_SPARE_11                      CAMSV_IMGO_FH_SPARE_11;                          /* 0458, 0x1A050458, CAMSV_0_IMGO_FH_SPARE_11 */
    CAMSV_REG_IMGO_FH_SPARE_12                      CAMSV_IMGO_FH_SPARE_12;                          /* 045C, 0x1A05045C, CAMSV_0_IMGO_FH_SPARE_12 */
    CAMSV_REG_IMGO_FH_SPARE_13                      CAMSV_IMGO_FH_SPARE_13;                          /* 0460, 0x1A050460, CAMSV_0_IMGO_FH_SPARE_13 */
    UINT32                                          rsv_0464;                                        /* 0464, 0x1A050464,  */
    UINT32                                          rsv_0468;                                        /* 0468, 0x1A050468,  */
    UINT32                                          rsv_046c;                                        /* 046C, 0x1A05046C,  */
    UINT32                                          rsv_0470[36];                                    /* 0470..04FF, 0x1A050470..1A0504FF */
    CAMSV_REG_TOP_DEBUG                             CAMSV_TOP_DEBUG;                                 /* 0500, 0x1A050500, CAMSV_0_TOP_DEBUG */
    UINT32                                          rsv_0504[3];                                     /* 0504..050F, 0x1A050504..1A05050F */
    CAMSV_REG_MODULE_EN                             CAMSV_MODULE_EN;                                 /* 0510, 0x1A050510, CAMSV_0_MODULE_EN */
    CAMSV_REG_FMT_SEL                               CAMSV_FMT_SEL;                                   /* 0514, 0x1A050514, CAMSV_0_FMT_SEL */
    CAMSV_REG_INT_EN                                CAMSV_INT_EN;                                    /* 0518, 0x1A050518, CAMSV_0_INT_EN */
    CAMSV_REG_INT_STATUS                            CAMSV_INT_STATUS;                                /* 051C, 0x1A05051C, CAMSV_0_INT_STATUS */
    CAMSV_REG_SW_CTL                                CAMSV_SW_CTL;                                    /* 0520, 0x1A050520, CAMSV_0_SW_CTL */
    CAMSV_REG_SPARE0                                CAMSV_SPARE0;                                    /* 0524, 0x1A050524, CAMSV_0_SPARE0 */
    CAMSV_REG_SPARE1                                CAMSV_SPARE1;                                    /* 0528, 0x1A050528, CAMSV_0_SPARE1 */
    CAMSV_REG_IMGO_FBC                              CAMSV_IMGO_FBC;                                  /* 052C, 0x1A05052C, CAMSV_0_IMGO_FBC */
    CAMSV_REG_CLK_EN                                CAMSV_CLK_EN;                                    /* 0530, 0x1A050530, CAMSV_0_CLK_EN */
    CAMSV_REG_DBG_SET                               CAMSV_DBG_SET;                                   /* 0534, 0x1A050534, CAMSV_0_DBG_SET */
    CAMSV_REG_DBG_PORT                              CAMSV_DBG_PORT;                                  /* 0538, 0x1A050538, CAMSV_0_DBG_PORT */
    CAMSV_REG_DATE_CODE                             CAMSV_DATE_CODE;                                 /* 053C, 0x1A05053C, CAMSV_0_DATE_CODE */
    CAMSV_REG_PROJ_CODE                             CAMSV_PROJ_CODE;                                 /* 0540, 0x1A050540, CAMSV_0_PROJ_CODE */
    CAMSV_REG_DCM_DIS                               CAMSV_DCM_DIS;                                   /* 0544, 0x1A050544, CAMSV_0_DCM_DIS */
    CAMSV_REG_DCM_STATUS                            CAMSV_DCM_STATUS;                                /* 0548, 0x1A050548, CAMSV_0_DCM_STATUS */
    CAMSV_REG_PAK                                   CAMSV_PAK;                                       /* 054C, 0x1A05054C, CAMSV_0_PAK */
    CAMSV_REG_CCU_INT_EN                            CAMSV_CCU_INT_EN;                                /* 0550, 0x1A050550, CAMSV_0_CCU_INT_EN */
    CAMSV_REG_CCU_INT_STATUS                        CAMSV_CCU_INT_STATUS;                            /* 0554, 0x1A050554, CAMSV_0_CCU_INT_STATUS */
    UINT32                                          rsv_0558[581];                                   /* 0558..0E6B, 1A050558..1A050E6B */
}camsv_reg_t;

/* auto insert ralf auto gen above */

#endif // _ISP_REG_H_
