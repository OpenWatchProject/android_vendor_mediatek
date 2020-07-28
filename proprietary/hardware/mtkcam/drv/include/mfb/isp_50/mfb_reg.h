#ifndef _MFB_REG_H_
#define _MFB_REG_H_

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */

typedef volatile union _MFB_REG_CON_
{
		volatile struct	/* 0x1502E000 */
		{
				FIELD  BLD_MODE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BLD_LL_BRZ_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BLD_LL_DB_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BLD_MBD_WT_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BLD_SR_WT_EN                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BLD_LL_TH_E                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_CON;	/* MFB_CON */

typedef volatile union _MFB_REG_LL_CON1_
{
		volatile struct	/* 0x1502E004 */
		{
				FIELD  BLD_LL_FLT_MODE                       :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  BLD_LL_FLT_WT_MODE1                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BLD_LL_FLT_WT_MODE2                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  BLD_LL_CLIP_TH1                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_CLIP_TH2                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON1;	/* MFB_LL_CON1 */

typedef volatile union _MFB_REG_LL_CON2_
{
		volatile struct	/* 0x1502E008 */
		{
				FIELD  BLD_LL_MAX_WT                         :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  BLD_LL_DT1                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH1                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH2                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON2;	/* MFB_LL_CON2 */

typedef volatile union _MFB_REG_LL_CON3_
{
		volatile struct	/* 0x1502E00C */
		{
				FIELD  BLD_LL_OUT_XSIZE                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  BLD_LL_OUT_XOFST                      :  1;		/* 15..15, 0x00008000 */
				FIELD  BLD_LL_OUT_YSIZE                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON3;	/* MFB_LL_CON3 */

typedef volatile union _MFB_REG_LL_CON4_
{
		volatile struct	/* 0x1502E010 */
		{
				FIELD  BLD_LL_DB_XDIST                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  BLD_LL_DB_YDIST                       :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON4;	/* MFB_LL_CON4 */

typedef volatile union _MFB_REG_EDGE_
{
		volatile struct	/* 0x1502E014 */
		{
				FIELD  BLD_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_EDGE;	/* MFB_EDGE */

typedef volatile union _MFB_REG_LL_CON5_
{
		volatile struct	/* 0x1502E018 */
		{
				FIELD  BLD_LL_GRAD_R1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_GRAD_R2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON5;	/* MFB_LL_CON5 */

typedef volatile union _MFB_REG_LL_CON6_
{
		volatile struct	/* 0x1502E01C */
		{
				FIELD  BLD_LL_TH5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_TH6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH8                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON6;	/* MFB_LL_CON6 */

typedef volatile union _MFB_REG_LL_CON7_
{
		volatile struct	/* 0x1502E020 */
		{
				FIELD  BLD_LL_C1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_C2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_C3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON7;	/* MFB_LL_CON7 */

typedef volatile union _MFB_REG_LL_CON8_
{
		volatile struct	/* 0x1502E024 */
		{
				FIELD  BLD_LL_SU1                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_LL_SU2                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_LL_SU3                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON8;	/* MFB_LL_CON8 */

typedef volatile union _MFB_REG_LL_CON9_
{
		volatile struct	/* 0x1502E028 */
		{
				FIELD  BLD_LL_SL1                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_LL_SL2                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_LL_SL3                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON9;	/* MFB_LL_CON9 */

typedef volatile union _MFB_REG_LL_CON10_
{
		volatile struct	/* 0x1502E02C */
		{
				FIELD  BLD_CONF_MAP_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BLD_LL_GRAD_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  BLD_LL_GRAD_ENTH                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON10;	/* MFB_LL_CON10 */

typedef volatile union _MFB_REG_MBD_CON0_
{
		volatile struct	/* 0x1502E030 */
		{
				FIELD  BLD_MBD_MAX_WT                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON0;	/* MFB_MBD_CON0 */

typedef volatile union _MFB_REG_MBD_CON1_
{
		volatile struct	/* 0x1502E034 */
		{
				FIELD  BLD_MBD_YL1_THL_0                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL1_THL_1                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL1_THL_2                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL1_THL_3                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON1;	/* MFB_MBD_CON1 */

typedef volatile union _MFB_REG_MBD_CON2_
{
		volatile struct	/* 0x1502E038 */
		{
				FIELD  BLD_MBD_YL1_THL_4                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL1_THH_0                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL1_THH_1                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL1_THH_2                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON2;	/* MFB_MBD_CON2 */

typedef volatile union _MFB_REG_MBD_CON3_
{
		volatile struct	/* 0x1502E03C */
		{
				FIELD  BLD_MBD_YL1_THH_3                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL1_THH_4                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL2_THL_0                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL2_THL_1                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON3;	/* MFB_MBD_CON3 */

typedef volatile union _MFB_REG_MBD_CON4_
{
		volatile struct	/* 0x1502E040 */
		{
				FIELD  BLD_MBD_YL2_THL_2                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL2_THL_3                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL2_THL_4                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL2_THH_0                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON4;	/* MFB_MBD_CON4 */

typedef volatile union _MFB_REG_MBD_CON5_
{
		volatile struct	/* 0x1502E044 */
		{
				FIELD  BLD_MBD_YL2_THH_1                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL2_THH_2                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL2_THH_3                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL2_THH_4                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON5;	/* MFB_MBD_CON5 */

typedef volatile union _MFB_REG_MBD_CON6_
{
		volatile struct	/* 0x1502E048 */
		{
				FIELD  BLD_MBD_YL3_THL_0                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL3_THL_1                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL3_THL_2                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL3_THL_3                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON6;	/* MFB_MBD_CON6 */

typedef volatile union _MFB_REG_MBD_CON7_
{
		volatile struct	/* 0x1502E04C */
		{
				FIELD  BLD_MBD_YL3_THL_4                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL3_THH_0                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_YL3_THH_1                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_YL3_THH_2                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON7;	/* MFB_MBD_CON7 */

typedef volatile union _MFB_REG_MBD_CON8_
{
		volatile struct	/* 0x1502E050 */
		{
				FIELD  BLD_MBD_YL3_THH_3                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_YL3_THH_4                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_CL1_THL_0                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_CL1_THL_1                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON8;	/* MFB_MBD_CON8 */

typedef volatile union _MFB_REG_MBD_CON9_
{
		volatile struct	/* 0x1502E054 */
		{
				FIELD  BLD_MBD_CL1_THL_2                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_CL1_THL_3                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_CL1_THL_4                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_CL1_THH_0                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON9;	/* MFB_MBD_CON9 */

typedef volatile union _MFB_REG_MBD_CON10_
{
		volatile struct	/* 0x1502E058 */
		{
				FIELD  BLD_MBD_CL1_THH_1                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_MBD_CL1_THH_2                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_MBD_CL1_THH_3                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BLD_MBD_CL1_THH_4                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON10;	/* MFB_MBD_CON10 */

typedef volatile union _MFB_REG_TOP_CFG0_
{
		volatile struct	/* 0x1502E05C */
		{
				FIELD  MFB_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB_TDR_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFB_SRZ_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MFB_FORMAT                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_TOP_CFG0;	/* MFB_TOP_CFG0 */

typedef volatile union _MFB_REG_TOP_CFG1_
{
		volatile struct	/* 0x1502E060 */
		{
				FIELD  MFB_TRIG_START                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MFB_REG_TOP_CFG1;	/* MFB_TOP_CFG1 */

typedef volatile union _MFB_REG_TOP_CFG2_
{
		volatile struct	/* 0x1502E064 */
		{
				FIELD  MFB_MFBI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB_MFB2I_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFB_MFB3I_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MFB_MFB4I_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MFB_MFBO_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_MFB2O_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFB_MFBI_B_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MFB_MFB2I_B_EN                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  MFB_MFBO_B_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_TOP_CFG2;	/* MFB_TOP_CFG2 */

typedef volatile union _MFB_REG_INT_CTL_
{
		volatile struct	/* 0x1502E080 */
		{
				FIELD  MFB_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  MFB_INT_CLR_MODE                      :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_INT_CTL;	/* MFB_INT_CTL */

typedef volatile union _MFB_REG_INT_STATUS_
{
		volatile struct	/* 0x1502E084 */
		{
				FIELD  MFB_INT_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MFB_REG_INT_STATUS;	/* MFB_INT_STATUS */

typedef volatile union _MFB_REG_SW_RST_
{
		volatile struct	/* 0x1502E088 */
		{
				FIELD  MFB_DMA_STOP                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB_DMA_STOP_STATUS                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  MFB_RST_BIT                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_SW_RST;	/* MFB_SW_RST */

typedef volatile union _MFB_REG_MAIN_DCM_ST_
{
		volatile struct	/* 0x1502E08C */
		{
				FIELD  MFM_MAIN_DCM_ST                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_MAIN_DCM_ST;	/* MFB_MAIN_DCM_ST */

typedef volatile union _MFB_REG_DMA_DCM_ST_
{
		volatile struct	/* 0x1502E090 */
		{
				FIELD  MFB_DMA_DCM_ST                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DMA_DCM_ST;	/* MFB_DMA_DCM_ST */

typedef volatile union _MFB_REG_MAIN_DCM_DIS_
{
		volatile struct	/* 0x1502E094 */
		{
				FIELD  MFB_MAIN_DCM_DIS                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_MAIN_DCM_DIS;	/* MFB_MAIN_DCM_DIS */

typedef volatile union _MFB_REG_DMA_DCM_DIS_
{
		volatile struct	/* 0x1502E098 */
		{
				FIELD  MFB_DMA_DCM_DIS                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DMA_DCM_DIS;	/* MFB_DMA_DCM_DIS */

typedef volatile union _MFB_REG_DBG_CTL0_
{
		volatile struct	/* 0x1502E09C */
		{
				FIELD  MFB_CORE_DBG_SEL                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MFB_SRZ_DBG_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MFB_DMA_DBG_SEL                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_CTL0;	/* MFB_DBG_CTL0 */

typedef volatile union _MFB_REG_DBG_OUT0_
{
		volatile struct	/* 0x1502E0A0 */
		{
				FIELD  MFB_CORE_DBG_OUT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_OUT0;	/* MFB_DBG_OUT0 */

typedef volatile union _MFB_REG_DBG_OUT1_
{
		volatile struct	/* 0x1502E0A4 */
		{
				FIELD  MFB_SRZ_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_OUT1;	/* MFB_DBG_OUT1 */

typedef volatile union _MFB_REG_DBG_OUT2_
{
		volatile struct	/* 0x1502E0A8 */
		{
				FIELD  MFB_DMA_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_OUT2;	/* MFB_DBG_OUT2 */

typedef volatile union _MFB_REG_DBG_OUT3_
{
		volatile struct	/* 0x1502E0AC */
		{
				FIELD  MFB_C02_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_OUT3;	/* MFB_DBG_OUT3 */

typedef volatile union _MFB_REG_DBG_OUT4_
{
		volatile struct	/* 0x1502E0B0 */
		{
				FIELD  MFB_CRSP_DBG_OUT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_DBG_OUT4;	/* MFB_DBG_OUT4 */

typedef volatile struct _mfb_reg_t_	/* 0x1502E000..0x1502E0B0 */
{
	MFB_REG_CON                                     MFB_CON;                                         /* 0000, 0x1502E000 */
	MFB_REG_LL_CON1                                 MFB_LL_CON1;                                     /* 0004, 0x1502E004 */
	MFB_REG_LL_CON2                                 MFB_LL_CON2;                                     /* 0008, 0x1502E008 */
	MFB_REG_LL_CON3                                 MFB_LL_CON3;                                     /* 000C, 0x1502E00C */
	MFB_REG_LL_CON4                                 MFB_LL_CON4;                                     /* 0010, 0x1502E010 */
	MFB_REG_EDGE                                    MFB_EDGE;                                        /* 0014, 0x1502E014 */
	MFB_REG_LL_CON5                                 MFB_LL_CON5;                                     /* 0018, 0x1502E018 */
	MFB_REG_LL_CON6                                 MFB_LL_CON6;                                     /* 001C, 0x1502E01C */
	MFB_REG_LL_CON7                                 MFB_LL_CON7;                                     /* 0020, 0x1502E020 */
	MFB_REG_LL_CON8                                 MFB_LL_CON8;                                     /* 0024, 0x1502E024 */
	MFB_REG_LL_CON9                                 MFB_LL_CON9;                                     /* 0028, 0x1502E028 */
	MFB_REG_LL_CON10                                MFB_LL_CON10;                                    /* 002C, 0x1502E02C */
	MFB_REG_MBD_CON0                                MFB_MBD_CON0;                                    /* 0030, 0x1502E030 */
	MFB_REG_MBD_CON1                                MFB_MBD_CON1;                                    /* 0034, 0x1502E034 */
	MFB_REG_MBD_CON2                                MFB_MBD_CON2;                                    /* 0038, 0x1502E038 */
	MFB_REG_MBD_CON3                                MFB_MBD_CON3;                                    /* 003C, 0x1502E03C */
	MFB_REG_MBD_CON4                                MFB_MBD_CON4;                                    /* 0040, 0x1502E040 */
	MFB_REG_MBD_CON5                                MFB_MBD_CON5;                                    /* 0044, 0x1502E044 */
	MFB_REG_MBD_CON6                                MFB_MBD_CON6;                                    /* 0048, 0x1502E048 */
	MFB_REG_MBD_CON7                                MFB_MBD_CON7;                                    /* 004C, 0x1502E04C */
	MFB_REG_MBD_CON8                                MFB_MBD_CON8;                                    /* 0050, 0x1502E050 */
	MFB_REG_MBD_CON9                                MFB_MBD_CON9;                                    /* 0054, 0x1502E054 */
	MFB_REG_MBD_CON10                               MFB_MBD_CON10;                                   /* 0058, 0x1502E058 */
	MFB_REG_TOP_CFG0                                MFB_TOP_CFG0;                                    /* 005C, 0x1502E05C */
	MFB_REG_TOP_CFG1                                MFB_TOP_CFG1;                                    /* 0060, 0x1502E060 */
	MFB_REG_TOP_CFG2                                MFB_TOP_CFG2;                                    /* 0064, 0x1502E064 */
	UINT32                                          rsv_0068[6];                                     /* 0068..007F, 0x1502E068..1502E07F */
	MFB_REG_INT_CTL                                 MFB_INT_CTL;                                     /* 0080, 0x1502E080 */
	MFB_REG_INT_STATUS                              MFB_INT_STATUS;                                  /* 0084, 0x1502E084 */
	MFB_REG_SW_RST                                  MFB_SW_RST;                                      /* 0088, 0x1502E088 */
	MFB_REG_MAIN_DCM_ST                             MFB_MAIN_DCM_ST;                                 /* 008C, 0x1502E08C */
	MFB_REG_DMA_DCM_ST                              MFB_DMA_DCM_ST;                                  /* 0090, 0x1502E090 */
	MFB_REG_MAIN_DCM_DIS                            MFB_MAIN_DCM_DIS;                                /* 0094, 0x1502E094 */
	MFB_REG_DMA_DCM_DIS                             MFB_DMA_DCM_DIS;                                 /* 0098, 0x1502E098 */
	MFB_REG_DBG_CTL0                                MFB_DBG_CTL0;                                    /* 009C, 0x1502E09C */
	MFB_REG_DBG_OUT0                                MFB_DBG_OUT0;                                    /* 00A0, 0x1502E0A0 */
	MFB_REG_DBG_OUT1                                MFB_DBG_OUT1;                                    /* 00A4, 0x1502E0A4 */
	MFB_REG_DBG_OUT2                                MFB_DBG_OUT2;                                    /* 00A8, 0x1502E0A8 */
	MFB_REG_DBG_OUT3                                MFB_DBG_OUT3;                                    /* 00AC, 0x1502E0AC */
	MFB_REG_DBG_OUT4                                MFB_DBG_OUT4;                                    /* 00B0, 0x1502E0B0 */
}mfb_reg_t;

/* auto insert ralf auto gen above */

#endif // _MFB_REG_H_
