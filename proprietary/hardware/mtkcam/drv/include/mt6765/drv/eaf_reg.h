#ifndef _EAF_REG_H_
#define _EAF_REG_H_

#if 0

#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)


typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;


/* auto insert ralf auto gen below */

typedef volatile union _EAF_REG_DMA_SOFT_RSTSTAT_
{
        volatile struct /* 0x15028800 */
        {
                FIELD  EAFO_SOFT_RST_STAT                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  EAFI_SOFT_RST_STAT                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_SOFT_RSTSTAT;  /* EAF_DMA_SOFT_RSTSTAT */

typedef volatile union _EAF_REG_VERTICAL_FLIP_EN_
{
        volatile struct /* 0x15028804 */
        {
                FIELD  EAFO_V_FLIP_EN                        :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  EAFI_V_FLIP_EN                        :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_VERTICAL_FLIP_EN;  /* EAF_VERTICAL_FLIP_EN */

typedef volatile union _EAF_REG_DMA_SOFT_RESET_
{
        volatile struct /* 0x15028808 */
        {
                FIELD  EAFO_SOFT_RST                         :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  EAFI_SOFT_RST                         :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  SEPARATE_SOFT_RST_EN                  :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_SOFT_RESET;    /* EAF_DMA_SOFT_RESET */

typedef volatile union _EAF_REG_LAST_ULTRA_EN_
{
        volatile struct /* 0x1502880C */
        {
                FIELD  EAFO_LAST_ULTRA_EN                    :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  EAFI_LAST_ULTRA_EN                    :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 15;      /* 17..31, 0xFFFE0000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_LAST_ULTRA_EN; /* EAF_LAST_ULTRA_EN */

typedef volatile union _EAF_REG_SPECIAL_FUN_EN_
{
        volatile struct /* 0x15028810 */
        {
                FIELD  SLOW_CNT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                :  7;      /* 16..22, 0x007F0000 */
                FIELD  CONTINUOUS_COM_CON                    :  2;      /* 23..24, 0x01800000 */
                FIELD  CONTINUOUS_COM_EN                     :  1;      /* 25..25, 0x02000000 */
                FIELD  rsv_26                                :  4;      /* 26..29, 0x3C000000 */
                FIELD  INTERLACE_MODE                        :  1;      /* 30..30, 0x40000000 */
                FIELD  SLOW_EN                               :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_SPECIAL_FUN_EN;    /* EAF_SPECIAL_FUN_EN */

typedef volatile union _EAFO_REG_BASE_ADDR_
{
        volatile struct /* 0x15028830 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAFO_REG_BASE_ADDR;    /* EAFO_BASE_ADDR */

typedef volatile union _EAFO_REG_BASE_ADDR_2_
{
        volatile struct /* 0x15028834 */
        {
                FIELD  BASE_ADDR_2                           :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}EAFO_REG_BASE_ADDR_2;  /* EAFO_BASE_ADDR_2 */

typedef volatile union _EAFO_REG_OFST_ADDR_
{
        volatile struct /* 0x15028838 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAFO_REG_OFST_ADDR;    /* EAFO_OFST_ADDR */

typedef volatile union _EAFO_REG_OFST_ADDR_2_
{
        volatile struct /* 0x1502883C */
        {
                FIELD  OFFSET_ADDR_2                         :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}EAFO_REG_OFST_ADDR_2;  /* EAFO_OFST_ADDR_2 */

typedef volatile union _EAFO_REG_XSIZE_
{
        volatile struct /* 0x15028840 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_XSIZE;    /* EAFO_XSIZE */

typedef volatile union _EAFO_REG_YSIZE_
{
        volatile struct /* 0x15028844 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_YSIZE;    /* EAFO_YSIZE */

typedef volatile union _EAFO_REG_STRIDE_
{
        volatile struct /* 0x15028848 */
        {
                FIELD  STRIDE                                : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_STRIDE;   /* EAFO_STRIDE */

typedef volatile union _EAFO_REG_CON_
{
        volatile struct /* 0x1502884C */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_CON;  /* EAFO_CON */

typedef volatile union _EAFO_REG_CON2_
{
        volatile struct /* 0x15028850 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_CON2; /* EAFO_CON2 */

typedef volatile union _EAFO_REG_CON3_
{
        volatile struct /* 0x15028854 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_CON3; /* EAFO_CON3 */

typedef volatile union _EAFI_REG_BASE_ADDR_
{
        volatile struct /* 0x15028890 */
        {
                FIELD  BASE_ADDR                             : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAFI_REG_BASE_ADDR;    /* EAFI_BASE_ADDR */

typedef volatile union _EAFI_REG_BASE_ADDR_2_
{
        volatile struct /* 0x15028894 */
        {
                FIELD  BASE_ADDR_2                           :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}EAFI_REG_BASE_ADDR_2;  /* EAFI_BASE_ADDR_2 */

typedef volatile union _EAFI_REG_OFST_ADDR_
{
        volatile struct /* 0x15028898 */
        {
                FIELD  OFFSET_ADDR                           : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAFI_REG_OFST_ADDR;    /* EAFI_OFST_ADDR */

typedef volatile union _EAFI_REG_OFST_ADDR_2_
{
        volatile struct /* 0x1502889C */
        {
                FIELD  OFFSET_ADDR_2                         :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 : 30;      /*  2..31, 0xFFFFFFFC */
        } Bits;
        UINT32 Raw;
}EAFI_REG_OFST_ADDR_2;  /* EAFI_OFST_ADDR_2 */

typedef volatile union _EAFI_REG_XSIZE_
{
        volatile struct /* 0x150288A0 */
        {
                FIELD  XSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_XSIZE;    /* EAFI_XSIZE */

typedef volatile union _EAFI_REG_YSIZE_
{
        volatile struct /* 0x150288A4 */
        {
                FIELD  YSIZE                                 : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_YSIZE;    /* EAFI_YSIZE */

typedef volatile union _EAFI_REG_STRIDE_
{
        volatile struct /* 0x150288A8 */
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
}EAFI_REG_STRIDE;   /* EAFI_STRIDE */

typedef volatile union _EAFI_REG_CON_
{
        volatile struct /* 0x150288AC */
        {
                FIELD  FIFO_SIZE                             : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                : 16;      /* 12..27, 0x0FFFF000 */
                FIELD  MAX_BURST_LEN                         :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_CON;  /* EAFI_CON */

typedef volatile union _EAFI_REG_CON2_
{
        volatile struct /* 0x150288B0 */
        {
                FIELD  FIFO_PRI_THRL                         : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRI_THRH                         : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_CON2; /* EAFI_CON2 */

typedef volatile union _EAFI_REG_CON3_
{
        volatile struct /* 0x150288B4 */
        {
                FIELD  FIFO_PRE_PRI_THRL                     : 12;      /*  0..11, 0x00000FFF */
                FIELD  rsv_12                                :  4;      /* 12..15, 0x0000F000 */
                FIELD  FIFO_PRE_PRI_THRH                     : 12;      /* 16..27, 0x0FFF0000 */
                FIELD  rsv_28                                :  4;      /* 28..31, 0xF0000000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_CON3; /* EAFI_CON3 */

typedef volatile union _EAF_REG_DMA_ERR_CTRL_
{
        volatile struct /* 0x15028900 */
        {
                FIELD  EAFO_ERR                              :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 15;      /*  1..15, 0x0000FFFE */
                FIELD  EAFI_ERR                              :  1;      /* 16..16, 0x00010000 */
                FIELD  rsv_17                                : 14;      /* 17..30, 0x7FFE0000 */
                FIELD  ERR_CLR_MD                            :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_ERR_CTRL;  /* EAF_DMA_ERR_CTRL */

typedef volatile union _EAFO_REG_ERR_STAT_
{
        volatile struct /* 0x15028904 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFO_REG_ERR_STAT; /* EAFO_ERR_STAT */

typedef volatile union _EAFI_REG_ERR_STAT_
{
        volatile struct /* 0x15028908 */
        {
                FIELD  ERR_STAT                              : 16;      /*  0..15, 0x0000FFFF */
                FIELD  ERR_EN                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}EAFI_REG_ERR_STAT; /* EAFI_ERR_STAT */

typedef volatile union _EAF_REG_DMA_DEBUG_ADDR_
{
        volatile struct /* 0x1502890C */
        {
                FIELD  DEBUG_ADDR                            : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_DEBUG_ADDR;    /* EAF_DMA_DEBUG_ADDR */

typedef volatile union _EAF_REG_DMA_RSV1_
{
        volatile struct /* 0x15028910 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV1;  /* EAF_DMA_RSV1 */

typedef volatile union _EAF_REG_DMA_RSV2_
{
        volatile struct /* 0x15028914 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV2;  /* EAF_DMA_RSV2 */

typedef volatile union _EAF_REG_DMA_RSV3_
{
        volatile struct /* 0x15028918 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV3;  /* EAF_DMA_RSV3 */

typedef volatile union _EAF_REG_DMA_RSV4_
{
        volatile struct /* 0x1502891C */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV4;  /* EAF_DMA_RSV4 */

typedef volatile union _EAF_REG_DMA_RSV5_
{
        volatile struct /* 0x15028920 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV5;  /* EAF_DMA_RSV5 */

typedef volatile union _EAF_REG_DMA_RSV6_
{
        volatile struct /* 0x15028924 */
        {
                FIELD  RSV                                   : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_RSV6;  /* EAF_DMA_RSV6 */

typedef volatile union _EAF_REG_DMA_DEBUG_SEL_
{
        volatile struct /* 0x15028928 */
        {
                FIELD  DMA_TOP_SEL                           :  8;      /*  0.. 7, 0x000000FF */
                FIELD  R_W_DMA_TOP_SEL                       :  8;      /*  8..15, 0x0000FF00 */
                FIELD  SUB_MODULE_SEL                        :  8;      /* 16..23, 0x00FF0000 */
                FIELD  rsv_24                                :  6;      /* 24..29, 0x3F000000 */
                FIELD  ARBITER_BVALID_FULL                   :  1;      /* 30..30, 0x40000000 */
                FIELD  ARBITER_COM_FULL                      :  1;      /* 31..31, 0x80000000 */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_DEBUG_SEL; /* EAF_DMA_DEBUG_SEL */

typedef volatile union _EAF_REG_DMA_BW_SELF_TEST_
{
        volatile struct /* 0x1502892C */
        {
                FIELD  BW_SELF_TEST_EN_EAFO                  :  1;      /*  0.. 0, 0x00000001 */
                FIELD  rsv_1                                 : 31;      /*  1..31, 0xFFFFFFFE */
        } Bits;
        UINT32 Raw;
}EAF_REG_DMA_BW_SELF_TEST;  /* EAF_DMA_BW_SELF_TEST */



typedef volatile struct _dpe_reg_t_ /* 0x15028800..0x1502893F */
{
    EAF_REG_DMA_SOFT_RSTSTAT                        EAF_DMA_SOFT_RSTSTAT;                            /* 0000, 0x15028800 */
    EAF_REG_VERTICAL_FLIP_EN                        EAF_VERTICAL_FLIP_EN;                            /* 0004, 0x15028804 */
    EAF_REG_DMA_SOFT_RESET                          EAF_DMA_SOFT_RESET;                              /* 0008, 0x15028808 */
    EAF_REG_LAST_ULTRA_EN                           EAF_LAST_ULTRA_EN;                               /* 000C, 0x1502880C */
    EAF_REG_SPECIAL_FUN_EN                          EAF_SPECIAL_FUN_EN;                              /* 0010, 0x15028810 */
    UINT32                                          rsv_0014[7];                                     /* 0014..002F, 0x15028814..1502882F */
    EAFO_REG_BASE_ADDR                              EAFO_BASE_ADDR;                                  /* 0030, 0x15028830 */
    EAFO_REG_BASE_ADDR_2                            EAFO_BASE_ADDR_2;                                /* 0034, 0x15028834 */
    EAFO_REG_OFST_ADDR                              EAFO_OFST_ADDR;                                  /* 0038, 0x15028838 */
    EAFO_REG_OFST_ADDR_2                            EAFO_OFST_ADDR_2;                                /* 003C, 0x1502883C */
    EAFO_REG_XSIZE                                  EAFO_XSIZE;                                      /* 0040, 0x15028840 */
    EAFO_REG_YSIZE                                  EAFO_YSIZE;                                      /* 0044, 0x15028844 */
    EAFO_REG_STRIDE                                 EAFO_STRIDE;                                     /* 0048, 0x15028848 */
    EAFO_REG_CON                                    EAFO_CON;                                        /* 004C, 0x1502884C */
    EAFO_REG_CON2                                   EAFO_CON2;                                       /* 0050, 0x15028850 */
    EAFO_REG_CON3                                   EAFO_CON3;                                       /* 0054, 0x15028854 */
    UINT32                                          rsv_0058[14];                                    /* 0058..008F, 0x15028858..1502888F */
    EAFI_REG_BASE_ADDR                              EAFI_BASE_ADDR;                                  /* 0090, 0x15028890 */
    EAFI_REG_BASE_ADDR_2                            EAFI_BASE_ADDR_2;                                /* 0094, 0x15028894 */
    EAFI_REG_OFST_ADDR                              EAFI_OFST_ADDR;                                  /* 0098, 0x15028898 */
    EAFI_REG_OFST_ADDR_2                            EAFI_OFST_ADDR_2;                                /* 009C, 0x1502889C */
    EAFI_REG_XSIZE                                  EAFI_XSIZE;                                      /* 00A0, 0x150288A0 */
    EAFI_REG_YSIZE                                  EAFI_YSIZE;                                      /* 00A4, 0x150288A4 */
    EAFI_REG_STRIDE                                 EAFI_STRIDE;                                     /* 00A8, 0x150288A8 */
    EAFI_REG_CON                                    EAFI_CON;                                        /* 00AC, 0x150288AC */
    EAFI_REG_CON2                                   EAFI_CON2;                                       /* 00B0, 0x150288B0 */
    EAFI_REG_CON3                                   EAFI_CON3;                                       /* 00B4, 0x150288B4 */
    UINT32                                          rsv_00B8[18];                                    /* 00B8..00FF, 0x150288B8..150288FF */
    EAF_REG_DMA_ERR_CTRL                            EAF_DMA_ERR_CTRL;                                /* 0100, 0x15028900 */
    EAFO_REG_ERR_STAT                               EAFO_ERR_STAT;                                   /* 0104, 0x15028904 */
    EAFI_REG_ERR_STAT                               EAFI_ERR_STAT;                                   /* 0108, 0x15028908 */
    EAF_REG_DMA_DEBUG_ADDR                          EAF_DMA_DEBUG_ADDR;                              /* 010C, 0x1502890C */
    EAF_REG_DMA_RSV1                                EAF_DMA_RSV1;                                    /* 0110, 0x15028910 */
    EAF_REG_DMA_RSV2                                EAF_DMA_RSV2;                                    /* 0114, 0x15028914 */
    EAF_REG_DMA_RSV3                                EAF_DMA_RSV3;                                    /* 0118, 0x15028918 */
    EAF_REG_DMA_RSV4                                EAF_DMA_RSV4;                                    /* 011C, 0x1502891C */
    EAF_REG_DMA_RSV5                                EAF_DMA_RSV5;                                    /* 0120, 0x15028920 */
    EAF_REG_DMA_RSV6                                EAF_DMA_RSV6;                                    /* 0124, 0x15028924 */
    EAF_REG_DMA_DEBUG_SEL                           EAF_DMA_DEBUG_SEL;                               /* 0128, 0x15028928 */
    EAF_REG_DMA_BW_SELF_TEST                        EAF_DMA_BW_SELF_TEST;                            /* 012C, 0x1502892C */
    UINT32                                          rsv_0130[4];                                     /* 0130..013F, 15028930..1502893F */
}dpe_reg_t;
#endif
/* auto insert ralf auto gen above */

#endif // _EAF_REG_H_
