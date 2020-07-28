#ifndef _RSC_REG_H_
#define _RSC_REG_H_

#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)


typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */

typedef volatile union _RSC_REG_RST_
{
		volatile struct	/* 0x15029000 */
		{
				FIELD  DMA_STOP                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMA_STOP_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  HARD_RST                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RST;	/* RSC_RST */

typedef volatile union _RSC_REG_START_
{
		volatile struct	/* 0x15029004 */
		{
				FIELD  START                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}RSC_REG_START;	/* RSC_START */

typedef volatile union _RSC_REG_DCM_CTRL_
{
		volatile struct	/* 0x15029008 */
		{
				FIELD  RSC_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MEM_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RDMA_0_DCM_DIS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  WDMA_0_DCM_DIS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DCM_CTRL;	/* RSC_DCM_CTRL */

typedef volatile union _RSC_REG_DCM_STATUS_
{
		volatile struct	/* 0x1502900C */
		{
				FIELD  RSC_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MEM_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RDMA_0_DCM_STATUS                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  WDMA_0_DCM_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DCM_STATUS;	/* RSC_DCM_STATUS */

typedef volatile union _RSC_REG_INT_CTRL_
{
		volatile struct	/* 0x15029010 */
		{
				FIELD  INT_ENABLE                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 30;		/*  1..30, 0x7FFFFFFE */
				FIELD  INT_CLR_MODE                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_INT_CTRL;	/* RSC_INT_CTRL */

typedef volatile union _RSC_REG_INT_STATUS_
{
		volatile struct	/* 0x15029014 */
		{
				FIELD  INT                                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}RSC_REG_INT_STATUS;	/* RSC_INT_STATUS */

typedef volatile union _RSC_REG_CTRL_
{
		volatile struct	/* 0x15029018 */
		{
				FIELD  START_SCAN_ORDER                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SKIP_PREV_MV                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FIRST_ME                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  GMV_MODE                              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMGI_C_FMT                            :  2;		/*  4.. 5, 0x00000030 */
				FIELD  IMGI_P_FMT                            :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  TRIGG_NUM                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  INIT_MV_FLUSH_CNT                     :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  INIT_MV_WADDR                         :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_CTRL;	/* RSC_CTRL */

typedef volatile union _RSC_REG_SIZE_
{
		volatile struct	/* 0x1502901C */
		{
				FIELD  WIDTH                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  HEIGHT                                :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SIZE;	/* RSC_SIZE */

typedef volatile union _RSC_REG_SR_
{
		volatile struct	/* 0x15029020 */
		{
				FIELD  HORZ_SR                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  VERT_SR                               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SR;	/* RSC_SR */

typedef volatile union _RSC_REG_BR_
{
		volatile struct	/* 0x15029024 */
		{
				FIELD  BR_LEFT                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  BR_RIGHT                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  BR_BOTTOM                             :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  BR_TOP                                :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_BR;	/* RSC_BR */

typedef volatile union _RSC_REG_MV_OFFSET_
{
		volatile struct	/* 0x15029028 */
		{
				FIELD  HORZ_C_MV_OFFSET                      :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  VERT_C_MV_OFFSET                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HORZ_P_MV_OFFSET                      :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  VERT_P_MV_OFFSET                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_MV_OFFSET;	/* RSC_MV_OFFSET */

typedef volatile union _RSC_REG_GMV_OFFSET_
{
		volatile struct	/* 0x1502902C */
		{
				FIELD  HORZ_G_MV_OFFSET                      :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  VERT_G_MV_OFFSET                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_GMV_OFFSET;	/* RSC_GMV_OFFSET */

typedef volatile union _RSC_REG_PREPARE_MV_CTRL_
{
		volatile struct	/* 0x15029030 */
		{
				FIELD  HORZ_RAND_GAIN                        :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  VERT_RAND_GAIN                        :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  AVG_CAND_DIFF_TH                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_PREPARE_MV_CTRL;	/* RSC_PREPARE_MV_CTRL */

typedef volatile union _RSC_REG_CAND_NUM_
{
		volatile struct	/* 0x15029034 */
		{
				FIELD  CAND_NUM                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}RSC_REG_CAND_NUM;	/* RSC_CAND_NUM */

typedef volatile union _RSC_REG_EVEN_CAND_SEL_0_
{
		volatile struct	/* 0x15029038 */
		{
				FIELD  EVEN_CAND_SEL_00                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EVEN_CAND_SEL_01                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  EVEN_CAND_SEL_02                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  EVEN_CAND_SEL_03                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_EVEN_CAND_SEL_0;	/* RSC_EVEN_CAND_SEL_0 */

typedef volatile union _RSC_REG_EVEN_CAND_SEL_1_
{
		volatile struct	/* 0x1502903C */
		{
				FIELD  EVEN_CAND_SEL_04                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EVEN_CAND_SEL_05                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  EVEN_CAND_SEL_06                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  EVEN_CAND_SEL_07                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_EVEN_CAND_SEL_1;	/* RSC_EVEN_CAND_SEL_1 */

typedef volatile union _RSC_REG_EVEN_CAND_SEL_2_
{
		volatile struct	/* 0x15029040 */
		{
				FIELD  EVEN_CAND_SEL_08                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EVEN_CAND_SEL_09                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  EVEN_CAND_SEL_10                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  EVEN_CAND_SEL_11                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_EVEN_CAND_SEL_2;	/* RSC_EVEN_CAND_SEL_2 */

typedef volatile union _RSC_REG_EVEN_CAND_SEL_3_
{
		volatile struct	/* 0x15029044 */
		{
				FIELD  EVEN_CAND_SEL_12                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EVEN_CAND_SEL_13                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  EVEN_CAND_SEL_14                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  EVEN_CAND_SEL_15                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_EVEN_CAND_SEL_3;	/* RSC_EVEN_CAND_SEL_3 */

typedef volatile union _RSC_REG_EVEN_CAND_SEL_4_
{
		volatile struct	/* 0x15029048 */
		{
				FIELD  EVEN_CAND_SEL_16                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EVEN_CAND_SEL_17                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_EVEN_CAND_SEL_4;	/* RSC_EVEN_CAND_SEL_4 */

typedef volatile union _RSC_REG_ODD_CAND_SEL_0_
{
		volatile struct	/* 0x1502904C */
		{
				FIELD  ODD_CAND_SEL_00                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ODD_CAND_SEL_01                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ODD_CAND_SEL_02                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ODD_CAND_SEL_03                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ODD_CAND_SEL_0;	/* RSC_ODD_CAND_SEL_0 */

typedef volatile union _RSC_REG_ODD_CAND_SEL_1_
{
		volatile struct	/* 0x15029050 */
		{
				FIELD  ODD_CAND_SEL_04                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ODD_CAND_SEL_05                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ODD_CAND_SEL_06                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ODD_CAND_SEL_07                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ODD_CAND_SEL_1;	/* RSC_ODD_CAND_SEL_1 */

typedef volatile union _RSC_REG_ODD_CAND_SEL_2_
{
		volatile struct	/* 0x15029054 */
		{
				FIELD  ODD_CAND_SEL_08                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ODD_CAND_SEL_09                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ODD_CAND_SEL_10                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ODD_CAND_SEL_11                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ODD_CAND_SEL_2;	/* RSC_ODD_CAND_SEL_2 */

typedef volatile union _RSC_REG_ODD_CAND_SEL_3_
{
		volatile struct	/* 0x15029058 */
		{
				FIELD  ODD_CAND_SEL_12                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ODD_CAND_SEL_13                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ODD_CAND_SEL_14                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ODD_CAND_SEL_15                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ODD_CAND_SEL_3;	/* RSC_ODD_CAND_SEL_3 */

typedef volatile union _RSC_REG_ODD_CAND_SEL_4_
{
		volatile struct	/* 0x1502905C */
		{
				FIELD  ODD_CAND_SEL_16                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ODD_CAND_SEL_17                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ODD_CAND_SEL_4;	/* RSC_ODD_CAND_SEL_4 */

typedef volatile union _RSC_REG_RAND_HORZ_LUT_
{
		volatile struct	/* 0x15029060 */
		{
				FIELD  HORZ_RAND_LUT_0                       :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  HORZ_RAND_LUT_1                       :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  HORZ_RAND_LUT_2                       :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  HORZ_RAND_LUT_3                       :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RAND_HORZ_LUT;	/* RSC_RAND_HORZ_LUT */

typedef volatile union _RSC_REG_RAND_VERT_LUT_
{
		volatile struct	/* 0x15029064 */
		{
				FIELD  VERT_RAND_LUT_0                       :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  VERT_RAND_LUT_1                       :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  VERT_RAND_LUT_2                       :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  VERT_RAND_LUT_3                       :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RAND_VERT_LUT;	/* RSC_RAND_VERT_LUT */

typedef volatile union _RSC_REG_CURR_BLK_CTRL_
{
		volatile struct	/* 0x15029068 */
		{
				FIELD  SORT_MV_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NO_ZERO_SORT_MV_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  OUT_VAR_SHIFT                         :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SORT_CAND_NUM                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  NON_ZERO_BLK_CAND_SEL                 :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  VAR_CORING_TH                         :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  EDGE_TH                               :  6;		/* 20..25, 0x03F00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_CURR_BLK_CTRL;	/* RSC_CURR_BLK_CTRL */

typedef volatile union _RSC_REG_SAD_CTRL_
{
		volatile struct	/* 0x1502906C */
		{
				FIELD  PREV_Y_OFFSET                         :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SAD_CORING_TH                         :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  SAD_GAIN                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PREV_Y_OFFSET_EN                      :  1;		/* 20..20, 0x00100000 */
				FIELD  PREV_Y_OFFSET_MODE                    :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  PREV_Y_OFFSET_CORING_TH               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SAD_CTRL;	/* RSC_SAD_CTRL */

typedef volatile union _RSC_REG_SAD_EDGE_GAIN_CTRL_
{
		volatile struct	/* 0x15029070 */
		{
				FIELD  SAD_EDGE_GAIN                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SAD_EDGE_GAIN_TH_L                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SAD_EDGE_GAIN_STEP                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SAD_EDGE_GAIN_CTRL;	/* RSC_SAD_EDGE_GAIN_CTRL */

typedef volatile union _RSC_REG_SAD_CRNR_GAIN_CTRL_
{
		volatile struct	/* 0x15029074 */
		{
				FIELD  SAD_CRNR_GAIN                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  SAD_CRNR_GAIN_TH_L                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SAD_CRNR_GAIN_STEP                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SAD_CRNR_GAIN_CTRL;	/* RSC_SAD_CRNR_GAIN_CTRL */

typedef volatile union _RSC_REG_STILL_STRIP_CTRL_0_
{
		volatile struct	/* 0x15029078 */
		{
				FIELD  STILL_STRIP_BLK_TH_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  STILL_STRIP_ZERO_PNLTY_DIS            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}RSC_REG_STILL_STRIP_CTRL_0;	/* RSC_STILL_STRIP_CTRL_0 */

typedef volatile union _RSC_REG_STILL_STRIP_CTRL_1_
{
		volatile struct	/* 0x1502907C */
		{
				FIELD  STILL_STRIP_SAD_TH_L                  :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  STILL_STRIP_SAD_STEP                  :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  STILL_STRIP_VAR_TH_L                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  STILL_STRIP_VAR_STEP                  :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_STILL_STRIP_CTRL_1;	/* RSC_STILL_STRIP_CTRL_1 */

typedef volatile union _RSC_REG_MV_PNLTY_CTRL_
{
		volatile struct	/* 0x15029080 */
		{
				FIELD  MV_PNLTY_CLIP_EN                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MV_PNLTY_CLIP_TH                      :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  MV_PNLTY_SEL                          :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  MV_PNLTY_BLK_TH_0                     :  4;		/* 16..19, 0x000F0000 */
				FIELD  MV_PNLTY_BLK_TH_1                     :  4;		/* 20..23, 0x00F00000 */
				FIELD  MV_PNLTY_EDGE_TH                      :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_MV_PNLTY_CTRL;	/* RSC_MV_PNLTY_CTRL */

typedef volatile union _RSC_REG_ZERO_PNLTY_CTRL_
{
		volatile struct	/* 0x15029084 */
		{
				FIELD  ZERO_PNLTY                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BNDRY_ZERO_PNLTY                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ZERO_PNLTY_BLK_TH                     :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_ZERO_PNLTY_CTRL;	/* RSC_ZERO_PNLTY_CTRL */

typedef volatile union _RSC_REG_RAND_PNLTY_CTRL_
{
		volatile struct	/* 0x15029088 */
		{
				FIELD  RAND_GAIN                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RAND_PNLTY                            :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  BNDRY_RAND_PNLTY                      :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RAND_PNLTY_CTRL;	/* RSC_RAND_PNLTY_CTRL */

typedef volatile union _RSC_REG_RAND_PNLTY_GAIN_CTRL_0_
{
		volatile struct	/* 0x1502908C */
		{
				FIELD  RAND_PNLTY_EDGE_GAIN                  :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RAND_PNLTY_EDGE_RESP_TH_L             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RAND_PNLTY_EDGE_RESP_STEP             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RAND_PNLTY_GAIN_CTRL_0;	/* RSC_RAND_PNLTY_GAIN_CTRL_0 */

typedef volatile union _RSC_REG_RAND_PNLTY_GAIN_CTRL_1_
{
		volatile struct	/* 0x15029090 */
		{
				FIELD  RAND_PNLTY_AVG_GAIN                   :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  RAND_PNLTY_VAR_RESP_TH_L              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RAND_PNLTY_VAR_RESP_STEP              :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_RAND_PNLTY_GAIN_CTRL_1;	/* RSC_RAND_PNLTY_GAIN_CTRL_1 */

typedef volatile union _RSC_REG_TMPR_PNLTY_CTRL_0_
{
		volatile struct	/* 0x15029094 */
		{
				FIELD  TMPR_GAIN                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TMPR_PNLTY_BLK_TH                     :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BNDRY_TMPR_PNLTY                      :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_TMPR_PNLTY_CTRL_0;	/* RSC_TMPR_PNLTY_CTRL_0 */

typedef volatile union _RSC_REG_TMPR_PNLTY_CTRL_1_
{
		volatile struct	/* 0x15029098 */
		{
				FIELD  TMPR_PNLTY_STATIC_0                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TMPR_PNLTY_STATIC_1                   :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  TMPR_PNLTY_MOTION_0                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  TMPR_PNLTY_MOTION_1                   :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_TMPR_PNLTY_CTRL_1;	/* RSC_TMPR_PNLTY_CTRL_1 */

typedef volatile union _RSC_REG_IMGI_C_BASE_ADDR_
{
		volatile struct	/* 0x1502909C */
		{
				FIELD  IMGI_C_BASE_ADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_IMGI_C_BASE_ADDR;	/* RSC_IMGI_C_BASE_ADDR */

typedef volatile union _RSC_REG_IMGI_C_STRIDE_
{
		volatile struct	/* 0x150290A0 */
		{
				FIELD  IMGI_C_STRIDE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_IMGI_C_STRIDE;	/* RSC_IMGI_C_STRIDE */

typedef volatile union _RSC_REG_IMGI_P_BASE_ADDR_
{
		volatile struct	/* 0x150290A4 */
		{
				FIELD  IMGI_P_BASE_ADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_IMGI_P_BASE_ADDR;	/* RSC_IMGI_P_BASE_ADDR */

typedef volatile union _RSC_REG_IMGI_P_STRIDE_
{
		volatile struct	/* 0x150290A8 */
		{
				FIELD  IMGI_P_STRIDE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_IMGI_P_STRIDE;	/* RSC_IMGI_P_STRIDE */

typedef volatile union _RSC_REG_MVI_BASE_ADDR_
{
		volatile struct	/* 0x150290AC */
		{
				FIELD  MVI_BASE_ADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_MVI_BASE_ADDR;	/* RSC_MVI_BASE_ADDR */

typedef volatile union _RSC_REG_MVI_STRIDE_
{
		volatile struct	/* 0x150290B0 */
		{
				FIELD  MVI_STRIDE                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_MVI_STRIDE;	/* RSC_MVI_STRIDE */

typedef volatile union _RSC_REG_APLI_C_BASE_ADDR_
{
		volatile struct	/* 0x150290B4 */
		{
				FIELD  APLI_C_BASE_ADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_APLI_C_BASE_ADDR;	/* RSC_APLI_C_BASE_ADDR */

typedef volatile union _RSC_REG_APLI_P_BASE_ADDR_
{
		volatile struct	/* 0x150290B8 */
		{
				FIELD  APLI_P_BASE_ADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_APLI_P_BASE_ADDR;	/* RSC_APLI_P_BASE_ADDR */

typedef volatile union _RSC_REG_MVO_BASE_ADDR_
{
		volatile struct	/* 0x150290BC */
		{
				FIELD  MVO_BASE_ADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_MVO_BASE_ADDR;	/* RSC_MVO_BASE_ADDR */

typedef volatile union _RSC_REG_MVO_STRIDE_
{
		volatile struct	/* 0x150290C0 */
		{
				FIELD  MVO_STRIDE                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_MVO_STRIDE;	/* RSC_MVO_STRIDE */

typedef volatile union _RSC_REG_BVO_BASE_ADDR_
{
		volatile struct	/* 0x150290C4 */
		{
				FIELD  BVO_BASE_ADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_BVO_BASE_ADDR;	/* RSC_BVO_BASE_ADDR */

typedef volatile union _RSC_REG_BVO_STRIDE_
{
		volatile struct	/* 0x150290C8 */
		{
				FIELD  BVO_STRIDE                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_BVO_STRIDE;	/* RSC_BVO_STRIDE */

typedef volatile union _RSC_REG_STA_0_
{
		volatile struct	/* 0x15029100 */
		{
				FIELD  HORZ_GMV                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VERT_GMV                              :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_STA_0;	/* RSC_STA_0 */

typedef volatile union _RSC_REG_DBG_INFO_00_
{
		volatile struct	/* 0x15029120 */
		{
				FIELD  CTRL_FSM                              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PRCSS_FSM                             :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  P_SA_FSM                              :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  C_SA_FSM                              :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  C_MV_FSM                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  SORT_MV_FSM                           :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NZBLK_FSM                             :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_00;	/* RSC_DBG_INFO_00 */

typedef volatile union _RSC_REG_DBG_INFO_01_
{
		volatile struct	/* 0x15029124 */
		{
				FIELD  RDMA_FSM                              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  WDMA_FSM                              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  C2Y_FSM                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_01;	/* RSC_DBG_INFO_01 */

typedef volatile union _RSC_REG_DBG_INFO_02_
{
		volatile struct	/* 0x15029128 */
		{
				FIELD  TRIGG_CNT                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  PRELOAD_LINE_CNT                      :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  HORZ_SCAN_ORDER                       :  1;		/* 12..12, 0x00001000 */
				FIELD  VERT_SCAN_ORDER                       :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_02;	/* RSC_DBG_INFO_02 */

typedef volatile union _RSC_REG_DBG_INFO_03_
{
		volatile struct	/* 0x1502912C */
		{
				FIELD  CURR_BLK_X                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CURR_BLK_Y                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_03;	/* RSC_DBG_INFO_03 */

typedef volatile union _RSC_REG_DBG_INFO_04_
{
		volatile struct	/* 0x15029130 */
		{
				FIELD  IMGI_P_Y_CNT                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  IMGI_C_Y_CNT                          :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_04;	/* RSC_DBG_INFO_04 */

typedef volatile union _RSC_REG_DBG_INFO_05_
{
		volatile struct	/* 0x15029134 */
		{
				FIELD  MVI_BLK_Y_CNT                         :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  WDMA_BLK_Y_CNT                        :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_05;	/* RSC_DBG_INFO_05 */

typedef volatile union _RSC_REG_DBG_INFO_06_
{
		volatile struct	/* 0x15029138 */
		{
				FIELD  DBG_PREV_Y_OFFSET                     :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DBG_INFO_06;	/* RSC_DBG_INFO_06 */

typedef volatile union _RSC_REG_VERSION_
{
		volatile struct	/* 0x150291F4 */
		{
				FIELD  DAY                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MONTH                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  YEAR                                  : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_VERSION;	/* RSC_VERSION */

typedef volatile union _RSC_REG_SPARE_0_
{
		volatile struct	/* 0x150291F8 */
		{
				FIELD  SPARE_0                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_SPARE_0;	/* RSC_SPARE_0 */

typedef volatile union _RSC_REG_SPARE_1_
{
		volatile struct	/* 0x150291FC */
		{
				FIELD  SPARE_1                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_SPARE_1;	/* RSC_SPARE_1 */

typedef volatile union _RSC_REG_DMA_DBG_
{
		volatile struct	/* 0x150297F4 */
		{
				FIELD  DMA_DBG_DATA                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_DBG;	/* RSC_DMA_DBG */

typedef volatile union _RSC_REG_DMA_REQ_STATUS_
{
		volatile struct	/* 0x150297F8 */
		{
				FIELD  DMA_REQ_STATUS                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_REQ_STATUS;	/* RSC_DMA_REQ_STATUS */

typedef volatile union _RSC_REG_DMA_RDY_STATUS_
{
		volatile struct	/* 0x150297FC */
		{
				FIELD  DMA_RDY_STATUS                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RDY_STATUS;	/* RSC_DMA_RDY_STATUS */

typedef volatile union _RSC_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x15029800 */
		{
				FIELD  RSCO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  RSCI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_SOFT_RSTSTAT;	/* RSC_DMA_SOFT_RSTSTAT */

typedef volatile union _RSC_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x15029804 */
		{
				FIELD  RSCO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  RSCI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_VERTICAL_FLIP_EN;	/* RSC_VERTICAL_FLIP_EN */

typedef volatile union _RSC_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x15029808 */
		{
				FIELD  RSCO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  RSCI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_SOFT_RESET;	/* RSC_DMA_SOFT_RESET */

typedef volatile union _RSC_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1502980C */
		{
				FIELD  RSCO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  RSCI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_LAST_ULTRA_EN;	/* RSC_LAST_ULTRA_EN */

typedef volatile union _RSC_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x15029810 */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  7;		/* 16..22, 0x007F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 23..24, 0x01800000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  4;		/* 26..29, 0x3C000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_SPECIAL_FUN_EN;	/* RSC_SPECIAL_FUN_EN */

typedef volatile union _RSCO_REG_BASE_ADDR_
{
		volatile struct	/* 0x15029830 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSCO_REG_BASE_ADDR;	/* RSCO_BASE_ADDR */

typedef volatile union _RSCO_REG_BASE_ADDR_2_
{
		volatile struct	/* 0x15029834 */
		{
				FIELD  BASE_ADDR_2                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}RSCO_REG_BASE_ADDR_2;	/* RSCO_BASE_ADDR_2 */

typedef volatile union _RSCO_REG_OFST_ADDR_
{
		volatile struct	/* 0x15029838 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSCO_REG_OFST_ADDR;	/* RSCO_OFST_ADDR */

typedef volatile union _RSCO_REG_OFST_ADDR_2_
{
		volatile struct	/* 0x1502983C */
		{
				FIELD  OFFSET_ADDR_2                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}RSCO_REG_OFST_ADDR_2;	/* RSCO_OFST_ADDR_2 */

typedef volatile union _RSCO_REG_XSIZE_
{
		volatile struct	/* 0x15029840 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_XSIZE;	/* RSCO_XSIZE */

typedef volatile union _RSCO_REG_YSIZE_
{
		volatile struct	/* 0x15029844 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_YSIZE;	/* RSCO_YSIZE */

typedef volatile union _RSCO_REG_STRIDE_
{
		volatile struct	/* 0x15029848 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_STRIDE;	/* RSCO_STRIDE */

typedef volatile union _RSCO_REG_CON_
{
		volatile struct	/* 0x1502984C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_CON;	/* RSCO_CON */

typedef volatile union _RSCO_REG_CON2_
{
		volatile struct	/* 0x15029850 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_CON2;	/* RSCO_CON2 */

typedef volatile union _RSCO_REG_CON3_
{
		volatile struct	/* 0x15029854 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_CON3;	/* RSCO_CON3 */

typedef volatile union _RSCI_REG_BASE_ADDR_
{
		volatile struct	/* 0x15029890 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSCI_REG_BASE_ADDR;	/* RSCI_BASE_ADDR */

typedef volatile union _RSCI_REG_BASE_ADDR_2_
{
		volatile struct	/* 0x15029894 */
		{
				FIELD  BASE_ADDR_2                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}RSCI_REG_BASE_ADDR_2;	/* RSCI_BASE_ADDR_2 */

typedef volatile union _RSCI_REG_OFST_ADDR_
{
		volatile struct	/* 0x15029898 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSCI_REG_OFST_ADDR;	/* RSCI_OFST_ADDR */

typedef volatile union _RSCI_REG_OFST_ADDR_2_
{
		volatile struct	/* 0x1502989C */
		{
				FIELD  OFFSET_ADDR_2                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}RSCI_REG_OFST_ADDR_2;	/* RSCI_OFST_ADDR_2 */

typedef volatile union _RSCI_REG_XSIZE_
{
		volatile struct	/* 0x150298A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_XSIZE;	/* RSCI_XSIZE */

typedef volatile union _RSCI_REG_YSIZE_
{
		volatile struct	/* 0x150298A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_YSIZE;	/* RSCI_YSIZE */

typedef volatile union _RSCI_REG_STRIDE_
{
		volatile struct	/* 0x150298A8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_STRIDE;	/* RSCI_STRIDE */

typedef volatile union _RSCI_REG_CON_
{
		volatile struct	/* 0x150298AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_CON;	/* RSCI_CON */

typedef volatile union _RSCI_REG_CON2_
{
		volatile struct	/* 0x150298B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_CON2;	/* RSCI_CON2 */

typedef volatile union _RSCI_REG_CON3_
{
		volatile struct	/* 0x150298B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_CON3;	/* RSCI_CON3 */

typedef volatile union _RSC_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x15029900 */
		{
				FIELD  RSCO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  RSCI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_ERR_CTRL;	/* RSC_DMA_ERR_CTRL */

typedef volatile union _RSCO_REG_ERR_STAT_
{
		volatile struct	/* 0x15029904 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCO_REG_ERR_STAT;	/* RSCO_ERR_STAT */

typedef volatile union _RSCI_REG_ERR_STAT_
{
		volatile struct	/* 0x15029908 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}RSCI_REG_ERR_STAT;	/* RSCI_ERR_STAT */

typedef volatile union _RSC_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1502990C */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_DEBUG_ADDR;	/* RSC_DMA_DEBUG_ADDR */

typedef volatile union _RSC_REG_DMA_RSV1_
{
		volatile struct	/* 0x15029910 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV1;	/* RSC_DMA_RSV1 */

typedef volatile union _RSC_REG_DMA_RSV2_
{
		volatile struct	/* 0x15029914 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV2;	/* RSC_DMA_RSV2 */

typedef volatile union _RSC_REG_DMA_RSV3_
{
		volatile struct	/* 0x15029918 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV3;	/* RSC_DMA_RSV3 */

typedef volatile union _RSC_REG_DMA_RSV4_
{
		volatile struct	/* 0x1502991C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV4;	/* RSC_DMA_RSV4 */

typedef volatile union _RSC_REG_DMA_RSV5_
{
		volatile struct	/* 0x15029920 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV5;	/* RSC_DMA_RSV5 */

typedef volatile union _RSC_REG_DMA_RSV6_
{
		volatile struct	/* 0x15029924 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_RSV6;	/* RSC_DMA_RSV6 */

typedef volatile union _RSC_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x15029928 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_DEBUG_SEL;	/* RSC_DMA_DEBUG_SEL */

typedef volatile union _RSC_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1502992C */
		{
				FIELD  BW_SELF_TEST_EN_RSCO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}RSC_REG_DMA_BW_SELF_TEST;	/* RSC_DMA_BW_SELF_TEST */

typedef volatile struct _rsc_reg_t_	/* 0x15029000..0x150327FF */
{
	RSC_REG_RST                                     RSC_RST;                                         /* 0000, 0x15029000 */
	RSC_REG_START                                   RSC_START;                                       /* 0004, 0x15029004 */
	RSC_REG_DCM_CTRL                                RSC_DCM_CTRL;                                    /* 0008, 0x15029008 */
	RSC_REG_DCM_STATUS                              RSC_DCM_STATUS;                                  /* 000C, 0x1502900C */
	RSC_REG_INT_CTRL                                RSC_INT_CTRL;                                    /* 0010, 0x15029010 */
	RSC_REG_INT_STATUS                              RSC_INT_STATUS;                                  /* 0014, 0x15029014 */
	RSC_REG_CTRL                                    RSC_CTRL;                                        /* 0018, 0x15029018 */
	RSC_REG_SIZE                                    RSC_SIZE;                                        /* 001C, 0x1502901C */
	RSC_REG_SR                                      RSC_SR;                                          /* 0020, 0x15029020 */
	RSC_REG_BR                                      RSC_BR;                                          /* 0024, 0x15029024 */
	RSC_REG_MV_OFFSET                               RSC_MV_OFFSET;                                   /* 0028, 0x15029028 */
	RSC_REG_GMV_OFFSET                              RSC_GMV_OFFSET;                                  /* 002C, 0x1502902C */
	RSC_REG_PREPARE_MV_CTRL                         RSC_PREPARE_MV_CTRL;                             /* 0030, 0x15029030 */
	RSC_REG_CAND_NUM                                RSC_CAND_NUM;                                    /* 0034, 0x15029034 */
	RSC_REG_EVEN_CAND_SEL_0                         RSC_EVEN_CAND_SEL_0;                             /* 0038, 0x15029038 */
	RSC_REG_EVEN_CAND_SEL_1                         RSC_EVEN_CAND_SEL_1;                             /* 003C, 0x1502903C */
	RSC_REG_EVEN_CAND_SEL_2                         RSC_EVEN_CAND_SEL_2;                             /* 0040, 0x15029040 */
	RSC_REG_EVEN_CAND_SEL_3                         RSC_EVEN_CAND_SEL_3;                             /* 0044, 0x15029044 */
	RSC_REG_EVEN_CAND_SEL_4                         RSC_EVEN_CAND_SEL_4;                             /* 0048, 0x15029048 */
	RSC_REG_ODD_CAND_SEL_0                          RSC_ODD_CAND_SEL_0;                              /* 004C, 0x1502904C */
	RSC_REG_ODD_CAND_SEL_1                          RSC_ODD_CAND_SEL_1;                              /* 0050, 0x15029050 */
	RSC_REG_ODD_CAND_SEL_2                          RSC_ODD_CAND_SEL_2;                              /* 0054, 0x15029054 */
	RSC_REG_ODD_CAND_SEL_3                          RSC_ODD_CAND_SEL_3;                              /* 0058, 0x15029058 */
	RSC_REG_ODD_CAND_SEL_4                          RSC_ODD_CAND_SEL_4;                              /* 005C, 0x1502905C */
	RSC_REG_RAND_HORZ_LUT                           RSC_RAND_HORZ_LUT;                               /* 0060, 0x15029060 */
	RSC_REG_RAND_VERT_LUT                           RSC_RAND_VERT_LUT;                               /* 0064, 0x15029064 */
	RSC_REG_CURR_BLK_CTRL                           RSC_CURR_BLK_CTRL;                               /* 0068, 0x15029068 */
	RSC_REG_SAD_CTRL                                RSC_SAD_CTRL;                                    /* 006C, 0x1502906C */
	RSC_REG_SAD_EDGE_GAIN_CTRL                      RSC_SAD_EDGE_GAIN_CTRL;                          /* 0070, 0x15029070 */
	RSC_REG_SAD_CRNR_GAIN_CTRL                      RSC_SAD_CRNR_GAIN_CTRL;                          /* 0074, 0x15029074 */
	RSC_REG_STILL_STRIP_CTRL_0                      RSC_STILL_STRIP_CTRL_0;                          /* 0078, 0x15029078 */
	RSC_REG_STILL_STRIP_CTRL_1                      RSC_STILL_STRIP_CTRL_1;                          /* 007C, 0x1502907C */
	RSC_REG_MV_PNLTY_CTRL                           RSC_MV_PNLTY_CTRL;                               /* 0080, 0x15029080 */
	RSC_REG_ZERO_PNLTY_CTRL                         RSC_ZERO_PNLTY_CTRL;                             /* 0084, 0x15029084 */
	RSC_REG_RAND_PNLTY_CTRL                         RSC_RAND_PNLTY_CTRL;                             /* 0088, 0x15029088 */
	RSC_REG_RAND_PNLTY_GAIN_CTRL_0                  RSC_RAND_PNLTY_GAIN_CTRL_0;                      /* 008C, 0x1502908C */
	RSC_REG_RAND_PNLTY_GAIN_CTRL_1                  RSC_RAND_PNLTY_GAIN_CTRL_1;                      /* 0090, 0x15029090 */
	RSC_REG_TMPR_PNLTY_CTRL_0                       RSC_TMPR_PNLTY_CTRL_0;                           /* 0094, 0x15029094 */
	RSC_REG_TMPR_PNLTY_CTRL_1                       RSC_TMPR_PNLTY_CTRL_1;                           /* 0098, 0x15029098 */
	RSC_REG_IMGI_C_BASE_ADDR                        RSC_IMGI_C_BASE_ADDR;                            /* 009C, 0x1502909C */
	RSC_REG_IMGI_C_STRIDE                           RSC_IMGI_C_STRIDE;                               /* 00A0, 0x150290A0 */
	RSC_REG_IMGI_P_BASE_ADDR                        RSC_IMGI_P_BASE_ADDR;                            /* 00A4, 0x150290A4 */
	RSC_REG_IMGI_P_STRIDE                           RSC_IMGI_P_STRIDE;                               /* 00A8, 0x150290A8 */
	RSC_REG_MVI_BASE_ADDR                           RSC_MVI_BASE_ADDR;                               /* 00AC, 0x150290AC */
	RSC_REG_MVI_STRIDE                              RSC_MVI_STRIDE;                                  /* 00B0, 0x150290B0 */
	RSC_REG_APLI_C_BASE_ADDR                        RSC_APLI_C_BASE_ADDR;                            /* 00B4, 0x150290B4 */
	RSC_REG_APLI_P_BASE_ADDR                        RSC_APLI_P_BASE_ADDR;                            /* 00B8, 0x150290B8 */
	RSC_REG_MVO_BASE_ADDR                           RSC_MVO_BASE_ADDR;                               /* 00BC, 0x150290BC */
	RSC_REG_MVO_STRIDE                              RSC_MVO_STRIDE;                                  /* 00C0, 0x150290C0 */
	RSC_REG_BVO_BASE_ADDR                           RSC_BVO_BASE_ADDR;                               /* 00C4, 0x150290C4 */
	RSC_REG_BVO_STRIDE                              RSC_BVO_STRIDE;                                  /* 00C8, 0x150290C8 */
	UINT32                                          rsv_00CC[13];                                    /* 00CC..00FF, 0x150290CC..150290FF */
	RSC_REG_STA_0                                   RSC_STA_0;                                       /* 0100, 0x15029100 */
	UINT32                                          rsv_0104[7];                                     /* 0104..011F, 0x15029104..1502911F */
	RSC_REG_DBG_INFO_00                             RSC_DBG_INFO_00;                                 /* 0120, 0x15029120 */
	RSC_REG_DBG_INFO_01                             RSC_DBG_INFO_01;                                 /* 0124, 0x15029124 */
	RSC_REG_DBG_INFO_02                             RSC_DBG_INFO_02;                                 /* 0128, 0x15029128 */
	RSC_REG_DBG_INFO_03                             RSC_DBG_INFO_03;                                 /* 012C, 0x1502912C */
	RSC_REG_DBG_INFO_04                             RSC_DBG_INFO_04;                                 /* 0130, 0x15029130 */
	RSC_REG_DBG_INFO_05                             RSC_DBG_INFO_05;                                 /* 0134, 0x15029134 */
	RSC_REG_DBG_INFO_06                             RSC_DBG_INFO_06;                                 /* 0138, 0x15029138 */
	UINT32                                          rsv_013C[46];                                    /* 013C..01F3, 0x1502913C..150291F3 */
	RSC_REG_VERSION                                 RSC_VERSION;                                     /* 01F4, 0x150291F4 */
	RSC_REG_SPARE_0                                 RSC_SPARE_0;                                     /* 01F8, 0x150291F8 */
	RSC_REG_SPARE_1                                 RSC_SPARE_1;                                     /* 01FC, 0x150291FC */
	UINT32                                          rsv_0200[381];                                   /* 0200..07F3, 0x15029200..150297F3 */
	RSC_REG_DMA_DBG                                 RSC_DMA_DBG;                                     /* 07F4, 0x150297F4 */
	RSC_REG_DMA_REQ_STATUS                          RSC_DMA_REQ_STATUS;                              /* 07F8, 0x150297F8 */
	RSC_REG_DMA_RDY_STATUS                          RSC_DMA_RDY_STATUS;                              /* 07FC, 0x150297FC */
	RSC_REG_DMA_SOFT_RSTSTAT                        RSC_DMA_SOFT_RSTSTAT;                            /* 0800, 0x15029800 */
	RSC_REG_VERTICAL_FLIP_EN                        RSC_VERTICAL_FLIP_EN;                            /* 0804, 0x15029804 */
	RSC_REG_DMA_SOFT_RESET                          RSC_DMA_SOFT_RESET;                              /* 0808, 0x15029808 */
	RSC_REG_LAST_ULTRA_EN                           RSC_LAST_ULTRA_EN;                               /* 080C, 0x1502980C */
	RSC_REG_SPECIAL_FUN_EN                          RSC_SPECIAL_FUN_EN;                              /* 0810, 0x15029810 */
	UINT32                                          rsv_0814[7];                                     /* 0814..082F, 0x15029814..1502982F */
	RSCO_REG_BASE_ADDR                              RSCO_BASE_ADDR;                                  /* 0830, 0x15029830 */
	RSCO_REG_BASE_ADDR_2                            RSCO_BASE_ADDR_2;                                /* 0834, 0x15029834 */
	RSCO_REG_OFST_ADDR                              RSCO_OFST_ADDR;                                  /* 0838, 0x15029838 */
	RSCO_REG_OFST_ADDR_2                            RSCO_OFST_ADDR_2;                                /* 083C, 0x1502983C */
	RSCO_REG_XSIZE                                  RSCO_XSIZE;                                      /* 0840, 0x15029840 */
	RSCO_REG_YSIZE                                  RSCO_YSIZE;                                      /* 0844, 0x15029844 */
	RSCO_REG_STRIDE                                 RSCO_STRIDE;                                     /* 0848, 0x15029848 */
	RSCO_REG_CON                                    RSCO_CON;                                        /* 084C, 0x1502984C */
	RSCO_REG_CON2                                   RSCO_CON2;                                       /* 0850, 0x15029850 */
	RSCO_REG_CON3                                   RSCO_CON3;                                       /* 0854, 0x15029854 */
	UINT32                                          rsv_0858[14];                                    /* 0858..088F, 0x15029858..1502988F */
	RSCI_REG_BASE_ADDR                              RSCI_BASE_ADDR;                                  /* 0890, 0x15029890 */
	RSCI_REG_BASE_ADDR_2                            RSCI_BASE_ADDR_2;                                /* 0894, 0x15029894 */
	RSCI_REG_OFST_ADDR                              RSCI_OFST_ADDR;                                  /* 0898, 0x15029898 */
	RSCI_REG_OFST_ADDR_2                            RSCI_OFST_ADDR_2;                                /* 089C, 0x1502989C */
	RSCI_REG_XSIZE                                  RSCI_XSIZE;                                      /* 08A0, 0x150298A0 */
	RSCI_REG_YSIZE                                  RSCI_YSIZE;                                      /* 08A4, 0x150298A4 */
	RSCI_REG_STRIDE                                 RSCI_STRIDE;                                     /* 08A8, 0x150298A8 */
	RSCI_REG_CON                                    RSCI_CON;                                        /* 08AC, 0x150298AC */
	RSCI_REG_CON2                                   RSCI_CON2;                                       /* 08B0, 0x150298B0 */
	RSCI_REG_CON3                                   RSCI_CON3;                                       /* 08B4, 0x150298B4 */
	UINT32                                          rsv_08B8[18];                                    /* 08B8..08FF, 0x150298B8..150298FF */
	RSC_REG_DMA_ERR_CTRL                            RSC_DMA_ERR_CTRL;                                /* 0900, 0x15029900 */
	RSCO_REG_ERR_STAT                               RSCO_ERR_STAT;                                   /* 0904, 0x15029904 */
	RSCI_REG_ERR_STAT                               RSCI_ERR_STAT;                                   /* 0908, 0x15029908 */
	RSC_REG_DMA_DEBUG_ADDR                          RSC_DMA_DEBUG_ADDR;                              /* 090C, 0x1502990C */
	RSC_REG_DMA_RSV1                                RSC_DMA_RSV1;                                    /* 0910, 0x15029910 */
	RSC_REG_DMA_RSV2                                RSC_DMA_RSV2;                                    /* 0914, 0x15029914 */
	RSC_REG_DMA_RSV3                                RSC_DMA_RSV3;                                    /* 0918, 0x15029918 */
	RSC_REG_DMA_RSV4                                RSC_DMA_RSV4;                                    /* 091C, 0x1502991C */
	RSC_REG_DMA_RSV5                                RSC_DMA_RSV5;                                    /* 0920, 0x15029920 */
	RSC_REG_DMA_RSV6                                RSC_DMA_RSV6;                                    /* 0924, 0x15029924 */
	RSC_REG_DMA_DEBUG_SEL                           RSC_DMA_DEBUG_SEL;                               /* 0928, 0x15029928 */
	RSC_REG_DMA_BW_SELF_TEST                        RSC_DMA_BW_SELF_TEST;                            /* 092C, 0x1502992C */
	UINT32                                          rsv_0930[20];                                    /* 0930..097F, 15029930..1502997F */
}rsc_reg_t;

/* auto insert ralf auto gen above */

#endif // _RSC_REG_H_
