#ifndef _FDVT_REG_H_
#define _FDVT_REG_H_

//include for reg size
//#include "camera_isp.h"

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif
#ifndef MUINT16
typedef unsigned short MUINT16;
#endif
#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MUINT64
typedef unsigned long long MUINT64;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif

#define FDVT_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define FDVT_REG(RegBase, RegName) (RegBase->RegName.Raw)

/**
    REG size for each module is 0x1000
*/
#define FDVT_BASE_RANGE (0x1000)

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

typedef volatile union _MFB_REG_A_MFB_CON_
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
}MFB_REG_A_MFB_CON;	/* MFB_A_MFB_CON */

typedef volatile union _MFB_REG_A_MFB_LL_CON1_
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
}MFB_REG_A_MFB_LL_CON1;	/* MFB_A_MFB_LL_CON1 */

typedef volatile union _MFB_REG_A_MFB_LL_CON2_
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
}MFB_REG_A_MFB_LL_CON2;	/* MFB_A_MFB_LL_CON2 */

typedef volatile union _MFB_REG_A_MFB_LL_CON3_
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
}MFB_REG_A_MFB_LL_CON3;	/* MFB_A_MFB_LL_CON3 */

typedef volatile union _MFB_REG_A_MFB_LL_CON4_
{
		volatile struct	/* 0x1502E010 */
		{
				FIELD  BLD_LL_DB_XDIST                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  BLD_LL_DB_YDIST                       :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_LL_CON4;	/* MFB_A_MFB_LL_CON4 */

typedef volatile union _MFB_REG_A_MFB_EDGE_
{
		volatile struct	/* 0x1502E014 */
		{
				FIELD  BLD_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_EDGE;	/* MFB_A_MFB_EDGE */

typedef volatile union _MFB_REG_A_MFB_LL_CON5_
{
		volatile struct	/* 0x1502E018 */
		{
				FIELD  BLD_LL_GRAD_R1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_GRAD_R2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_LL_CON5;	/* MFB_A_MFB_LL_CON5 */

typedef volatile union _MFB_REG_A_MFB_LL_CON6_
{
		volatile struct	/* 0x1502E01C */
		{
				FIELD  BLD_LL_TH5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_TH6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH8                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_LL_CON6;	/* MFB_A_MFB_LL_CON6 */

typedef volatile union _MFB_REG_A_MFB_LL_CON7_
{
		volatile struct	/* 0x1502E020 */
		{
				FIELD  BLD_LL_C1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_C2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_C3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_LL_CON7;	/* MFB_A_MFB_LL_CON7 */

typedef volatile union _MFB_REG_A_MFB_LL_CON8_
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
}MFB_REG_A_MFB_LL_CON8;	/* MFB_A_MFB_LL_CON8 */

typedef volatile union _MFB_REG_A_MFB_LL_CON9_
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
}MFB_REG_A_MFB_LL_CON9;	/* MFB_A_MFB_LL_CON9 */

typedef volatile union _MFB_REG_A_MFB_LL_CON10_
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
}MFB_REG_A_MFB_LL_CON10;	/* MFB_A_MFB_LL_CON10 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON0_
{
		volatile struct	/* 0x1502E030 */
		{
				FIELD  BLD_MBD_MAX_WT                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_MBD_CON0;	/* MFB_A_MFB_MBD_CON0 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON1_
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
}MFB_REG_A_MFB_MBD_CON1;	/* MFB_A_MFB_MBD_CON1 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON2_
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
}MFB_REG_A_MFB_MBD_CON2;	/* MFB_A_MFB_MBD_CON2 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON3_
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
}MFB_REG_A_MFB_MBD_CON3;	/* MFB_A_MFB_MBD_CON3 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON4_
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
}MFB_REG_A_MFB_MBD_CON4;	/* MFB_A_MFB_MBD_CON4 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON5_
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
}MFB_REG_A_MFB_MBD_CON5;	/* MFB_A_MFB_MBD_CON5 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON6_
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
}MFB_REG_A_MFB_MBD_CON6;	/* MFB_A_MFB_MBD_CON6 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON7_
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
}MFB_REG_A_MFB_MBD_CON7;	/* MFB_A_MFB_MBD_CON7 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON8_
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
}MFB_REG_A_MFB_MBD_CON8;	/* MFB_A_MFB_MBD_CON8 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON9_
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
}MFB_REG_A_MFB_MBD_CON9;	/* MFB_A_MFB_MBD_CON9 */

typedef volatile union _MFB_REG_A_MFB_MBD_CON10_
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
}MFB_REG_A_MFB_MBD_CON10;	/* MFB_A_MFB_MBD_CON10 */

typedef volatile union _MFB_REG_A_MFB_TOP_CFG0_
{
		volatile struct	/* 0x1502E05C */
		{
				FIELD  MFB_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB_TDR_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFB_SRZ_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_TOP_CFG0;	/* MFB_A_MFB_TOP_CFG0 */

typedef volatile union _MFB_REG_A_MFB_TOP_CFG1_
{
		volatile struct	/* 0x1502E060 */
		{
				FIELD  MFB_TRIG_START                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_TOP_CFG1;	/* MFB_A_MFB_TOP_CFG1 */

typedef volatile union _MFB_REG_A_MFB_TOP_CFG2_
{
		volatile struct	/* 0x1502E064 */
		{
				FIELD  MFB_MFBI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB_MFB2I_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFB_MFB3I_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MFB_MFB4I_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MFB_MFBO_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_MFB2O_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_TOP_CFG2;	/* MFB_A_MFB_TOP_CFG2 */

typedef volatile union _MFB_REG_A_MFB_INT_CTL_
{
		volatile struct	/* 0x1502E080 */
		{
				FIELD  MFB_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  MFB_INT_CLR_MODE                      :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_INT_CTL;	/* MFB_A_MFB_INT_CTL */

typedef volatile union _MFB_REG_A_MFB_INT_STATUS_
{
		volatile struct	/* 0x1502E084 */
		{
				FIELD  MFB_INT_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_INT_STATUS;	/* MFB_A_MFB_INT_STATUS */

typedef volatile union _MFB_REG_A_MFB_SW_RST_
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
}MFB_REG_A_MFB_SW_RST;	/* MFB_A_MFB_SW_RST */

typedef volatile union _MFB_REG_A_MFB_MAIN_DCM_ST_
{
		volatile struct	/* 0x1502E08C */
		{
				FIELD  MFM_MAIN_DCM_ST                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_MAIN_DCM_ST;	/* MFB_A_MFB_MAIN_DCM_ST */

typedef volatile union _MFB_REG_A_MFB_DMA_DCM_ST_
{
		volatile struct	/* 0x1502E090 */
		{
				FIELD  MFB_DMA_DCM_ST                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DMA_DCM_ST;	/* MFB_A_MFB_DMA_DCM_ST */

typedef volatile union _MFB_REG_A_MFB_MAIN_DCM_DIS_
{
		volatile struct	/* 0x1502E094 */
		{
				FIELD  MFB_MAIN_DCM_DIS                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_MAIN_DCM_DIS;	/* MFB_A_MFB_MAIN_DCM_DIS */

typedef volatile union _MFB_REG_A_MFB_DMA_DCM_DIS_
{
		volatile struct	/* 0x1502E098 */
		{
				FIELD  MFB_DMA_DCM_DIS                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DMA_DCM_DIS;	/* MFB_A_MFB_DMA_DCM_DIS */

typedef volatile union _MFB_REG_A_MFB_DBG_CTL0_
{
		volatile struct	/* 0x1502E09C */
		{
				FIELD  MFB_CORE_DBG_SEL                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MFB_SRZ_DBG_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MFB_DMA_DBG_SEL                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_CTL0;	/* MFB_A_MFB_DBG_CTL0 */

typedef volatile union _MFB_REG_A_MFB_DBG_OUT0_
{
		volatile struct	/* 0x1502E0A0 */
		{
				FIELD  MFB_CORE_DBG_OUT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_OUT0;	/* MFB_A_MFB_DBG_OUT0 */

typedef volatile union _MFB_REG_A_MFB_DBG_OUT1_
{
		volatile struct	/* 0x1502E0A4 */
		{
				FIELD  MFB_SRZ_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_OUT1;	/* MFB_A_MFB_DBG_OUT1 */

typedef volatile union _MFB_REG_A_MFB_DBG_OUT2_
{
		volatile struct	/* 0x1502E0A8 */
		{
				FIELD  MFB_DMA_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_OUT2;	/* MFB_A_MFB_DBG_OUT2 */

typedef volatile union _MFB_REG_A_MFB_DBG_OUT3_
{
		volatile struct	/* 0x1502E0AC */
		{
				FIELD  MFB_C02_DBG_OUT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_OUT3;	/* MFB_A_MFB_DBG_OUT3 */

typedef volatile union _MFB_REG_A_MFB_DBG_OUT4_
{
		volatile struct	/* 0x1502E0B0 */
		{
				FIELD  MFB_CRSP_DBG_OUT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB_DBG_OUT4;	/* MFB_A_MFB_DBG_OUT4 */

typedef volatile union _MFB_REG_A_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1502E100 */
		{
				FIELD  MFBO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB2O_SOFT_RST_STAT                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFBO_B_SOFT_RST_STAT                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  MFBI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  MFB2I_SOFT_RST_STAT                   :  1;		/* 17..17, 0x00020000 */
				FIELD  MFB3I_SOFT_RST_STAT                   :  1;		/* 18..18, 0x00040000 */
				FIELD  MFB4I_SOFT_RST_STAT                   :  1;		/* 19..19, 0x00080000 */
				FIELD  MFBI_B_SOFT_RST_STAT                  :  1;		/* 20..20, 0x00100000 */
				FIELD  MFB2I_B_SOFT_RST_STAT                 :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_SOFT_RSTSTAT;	/* MFB_A_DMA_SOFT_RSTSTAT */

typedef volatile union _MFB_REG_A_TDRI_BASE_ADDR_
{
		volatile struct	/* 0x1502E104 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_TDRI_BASE_ADDR;	/* MFB_A_TDRI_BASE_ADDR */

typedef volatile union _MFB_REG_A_TDRI_OFST_ADDR_
{
		volatile struct	/* 0x1502E108 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_TDRI_OFST_ADDR;	/* MFB_A_TDRI_OFST_ADDR */

typedef volatile union _MFB_REG_A_TDRI_XSIZE_
{
		volatile struct	/* 0x1502E10C */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_TDRI_XSIZE;	/* MFB_A_TDRI_XSIZE */

typedef volatile union _MFB_REG_A_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x1502E110 */
		{
				FIELD  MFBO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB2O_V_FLIP_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFBO_B_V_FLIP_EN                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  MFBI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  MFB2I_V_FLIP_EN                       :  1;		/* 17..17, 0x00020000 */
				FIELD  MFB3I_V_FLIP_EN                       :  1;		/* 18..18, 0x00040000 */
				FIELD  MFB4I_V_FLIP_EN                       :  1;		/* 19..19, 0x00080000 */
				FIELD  MFBI_B_V_FLIP_EN                      :  1;		/* 20..20, 0x00100000 */
				FIELD  MFB2I_B_V_FLIP_EN                     :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_VERTICAL_FLIP_EN;	/* MFB_A_VERTICAL_FLIP_EN */

typedef volatile union _MFB_REG_A_DMA_SOFT_RESET_
{
		volatile struct	/* 0x1502E114 */
		{
				FIELD  MFBO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB2O_SOFT_RST                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFBO_B_SOFT_RST                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  MFBI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  MFB2I_SOFT_RST                        :  1;		/* 17..17, 0x00020000 */
				FIELD  MFB3I_SOFT_RST                        :  1;		/* 18..18, 0x00040000 */
				FIELD  MFB4I_SOFT_RST                        :  1;		/* 19..19, 0x00080000 */
				FIELD  MFBI_B_SOFT_RST                       :  1;		/* 20..20, 0x00100000 */
				FIELD  MFB2I_B_SOFT_RST                      :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  9;		/* 22..30, 0x7FC00000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_SOFT_RESET;	/* MFB_A_DMA_SOFT_RESET */

typedef volatile union _MFB_REG_A_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1502E118 */
		{
				FIELD  MFBO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB2O_LAST_ULTRA_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFBO_B_LAST_ULTRA_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_LAST_ULTRA_EN;	/* MFB_A_LAST_ULTRA_EN */

typedef volatile union _MFB_REG_A_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1502E11C */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 26..26, 0x04000000 */
				FIELD  GCLAST_EN                             :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SPECIAL_FUN_EN;	/* MFB_A_SPECIAL_FUN_EN */

typedef volatile union _MFB_REG_A_MFBO_BASE_ADDR_
{
		volatile struct	/* 0x1502E130 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_BASE_ADDR;	/* MFB_A_MFBO_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFBO_OFST_ADDR_
{
		volatile struct	/* 0x1502E138 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_OFST_ADDR;	/* MFB_A_MFBO_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFBO_XSIZE_
{
		volatile struct	/* 0x1502E140 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_XSIZE;	/* MFB_A_MFBO_XSIZE */

typedef volatile union _MFB_REG_A_MFBO_YSIZE_
{
		volatile struct	/* 0x1502E144 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_YSIZE;	/* MFB_A_MFBO_YSIZE */

typedef volatile union _MFB_REG_A_MFBO_STRIDE_
{
		volatile struct	/* 0x1502E148 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_STRIDE;	/* MFB_A_MFBO_STRIDE */

typedef volatile union _MFB_REG_A_MFBO_CON_
{
		volatile struct	/* 0x1502E14C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_CON;	/* MFB_A_MFBO_CON */

typedef volatile union _MFB_REG_A_MFBO_CON2_
{
		volatile struct	/* 0x1502E150 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_CON2;	/* MFB_A_MFBO_CON2 */

typedef volatile union _MFB_REG_A_MFBO_CON3_
{
		volatile struct	/* 0x1502E154 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_CON3;	/* MFB_A_MFBO_CON3 */

typedef volatile union _MFB_REG_A_MFBO_CROP_
{
		volatile struct	/* 0x1502E158 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_CROP;	/* MFB_A_MFBO_CROP */

typedef volatile union _MFB_REG_A_MFB2O_BASE_ADDR_
{
		volatile struct	/* 0x1502E160 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_BASE_ADDR;	/* MFB_A_MFB2O_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFB2O_OFST_ADDR_
{
		volatile struct	/* 0x1502E168 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_OFST_ADDR;	/* MFB_A_MFB2O_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFB2O_XSIZE_
{
		volatile struct	/* 0x1502E170 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_XSIZE;	/* MFB_A_MFB2O_XSIZE */

typedef volatile union _MFB_REG_A_MFB2O_YSIZE_
{
		volatile struct	/* 0x1502E174 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_YSIZE;	/* MFB_A_MFB2O_YSIZE */

typedef volatile union _MFB_REG_A_MFB2O_STRIDE_
{
		volatile struct	/* 0x1502E178 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_STRIDE;	/* MFB_A_MFB2O_STRIDE */

typedef volatile union _MFB_REG_A_MFB2O_CON_
{
		volatile struct	/* 0x1502E17C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_CON;	/* MFB_A_MFB2O_CON */

typedef volatile union _MFB_REG_A_MFB2O_CON2_
{
		volatile struct	/* 0x1502E180 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_CON2;	/* MFB_A_MFB2O_CON2 */

typedef volatile union _MFB_REG_A_MFB2O_CON3_
{
		volatile struct	/* 0x1502E184 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_CON3;	/* MFB_A_MFB2O_CON3 */

typedef volatile union _MFB_REG_A_MFB2O_CROP_
{
		volatile struct	/* 0x1502E188 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_CROP;	/* MFB_A_MFB2O_CROP */

typedef volatile union _MFB_REG_A_MFBI_BASE_ADDR_
{
		volatile struct	/* 0x1502E190 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_BASE_ADDR;	/* MFB_A_MFBI_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFBI_OFST_ADDR_
{
		volatile struct	/* 0x1502E198 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_OFST_ADDR;	/* MFB_A_MFBI_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFBI_XSIZE_
{
		volatile struct	/* 0x1502E1A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_XSIZE;	/* MFB_A_MFBI_XSIZE */

typedef volatile union _MFB_REG_A_MFBI_YSIZE_
{
		volatile struct	/* 0x1502E1A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_YSIZE;	/* MFB_A_MFBI_YSIZE */

typedef volatile union _MFB_REG_A_MFBI_STRIDE_
{
		volatile struct	/* 0x1502E1A8 */
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
}MFB_REG_A_MFBI_STRIDE;	/* MFB_A_MFBI_STRIDE */

typedef volatile union _MFB_REG_A_MFBI_CON_
{
		volatile struct	/* 0x1502E1AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_CON;	/* MFB_A_MFBI_CON */

typedef volatile union _MFB_REG_A_MFBI_CON2_
{
		volatile struct	/* 0x1502E1B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_CON2;	/* MFB_A_MFBI_CON2 */

typedef volatile union _MFB_REG_A_MFBI_CON3_
{
		volatile struct	/* 0x1502E1B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_CON3;	/* MFB_A_MFBI_CON3 */

typedef volatile union _MFB_REG_A_MFB2I_BASE_ADDR_
{
		volatile struct	/* 0x1502E1C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_BASE_ADDR;	/* MFB_A_MFB2I_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFB2I_OFST_ADDR_
{
		volatile struct	/* 0x1502E1C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_OFST_ADDR;	/* MFB_A_MFB2I_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFB2I_XSIZE_
{
		volatile struct	/* 0x1502E1D0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_XSIZE;	/* MFB_A_MFB2I_XSIZE */

typedef volatile union _MFB_REG_A_MFB2I_YSIZE_
{
		volatile struct	/* 0x1502E1D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_YSIZE;	/* MFB_A_MFB2I_YSIZE */

typedef volatile union _MFB_REG_A_MFB2I_STRIDE_
{
		volatile struct	/* 0x1502E1D8 */
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
}MFB_REG_A_MFB2I_STRIDE;	/* MFB_A_MFB2I_STRIDE */

typedef volatile union _MFB_REG_A_MFB2I_CON_
{
		volatile struct	/* 0x1502E1DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_CON;	/* MFB_A_MFB2I_CON */

typedef volatile union _MFB_REG_A_MFB2I_CON2_
{
		volatile struct	/* 0x1502E1E0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_CON2;	/* MFB_A_MFB2I_CON2 */

typedef volatile union _MFB_REG_A_MFB2I_CON3_
{
		volatile struct	/* 0x1502E1E4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_CON3;	/* MFB_A_MFB2I_CON3 */

typedef volatile union _MFB_REG_A_MFB3I_BASE_ADDR_
{
		volatile struct	/* 0x1502E1F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_BASE_ADDR;	/* MFB_A_MFB3I_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFB3I_OFST_ADDR_
{
		volatile struct	/* 0x1502E1F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_OFST_ADDR;	/* MFB_A_MFB3I_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFB3I_XSIZE_
{
		volatile struct	/* 0x1502E200 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_XSIZE;	/* MFB_A_MFB3I_XSIZE */

typedef volatile union _MFB_REG_A_MFB3I_YSIZE_
{
		volatile struct	/* 0x1502E204 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_YSIZE;	/* MFB_A_MFB3I_YSIZE */

typedef volatile union _MFB_REG_A_MFB3I_STRIDE_
{
		volatile struct	/* 0x1502E208 */
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
}MFB_REG_A_MFB3I_STRIDE;	/* MFB_A_MFB3I_STRIDE */

typedef volatile union _MFB_REG_A_MFB3I_CON_
{
		volatile struct	/* 0x1502E20C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_CON;	/* MFB_A_MFB3I_CON */

typedef volatile union _MFB_REG_A_MFB3I_CON2_
{
		volatile struct	/* 0x1502E210 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_CON2;	/* MFB_A_MFB3I_CON2 */

typedef volatile union _MFB_REG_A_MFB3I_CON3_
{
		volatile struct	/* 0x1502E214 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_CON3;	/* MFB_A_MFB3I_CON3 */

typedef volatile union _MFB_REG_A_MFB4I_BASE_ADDR_
{
		volatile struct	/* 0x1502E220 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_BASE_ADDR;	/* MFB_A_MFB4I_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFB4I_OFST_ADDR_
{
		volatile struct	/* 0x1502E228 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_OFST_ADDR;	/* MFB_A_MFB4I_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFB4I_XSIZE_
{
		volatile struct	/* 0x1502E230 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_XSIZE;	/* MFB_A_MFB4I_XSIZE */

typedef volatile union _MFB_REG_A_MFB4I_YSIZE_
{
		volatile struct	/* 0x1502E234 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_YSIZE;	/* MFB_A_MFB4I_YSIZE */

typedef volatile union _MFB_REG_A_MFB4I_STRIDE_
{
		volatile struct	/* 0x1502E238 */
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
}MFB_REG_A_MFB4I_STRIDE;	/* MFB_A_MFB4I_STRIDE */

typedef volatile union _MFB_REG_A_MFB4I_CON_
{
		volatile struct	/* 0x1502E23C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_CON;	/* MFB_A_MFB4I_CON */

typedef volatile union _MFB_REG_A_MFB4I_CON2_
{
		volatile struct	/* 0x1502E240 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_CON2;	/* MFB_A_MFB4I_CON2 */

typedef volatile union _MFB_REG_A_MFB4I_CON3_
{
		volatile struct	/* 0x1502E244 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_CON3;	/* MFB_A_MFB4I_CON3 */

typedef volatile union _MFB_REG_A_DMA_ERR_CTRL_
{
		volatile struct	/* 0x1502E250 */
		{
				FIELD  MFBO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MFB2O_ERR                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MFBO_B_ERR                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  MFBI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  MFB2I_ERR                             :  1;		/* 17..17, 0x00020000 */
				FIELD  MFB3I_ERR                             :  1;		/* 18..18, 0x00040000 */
				FIELD  MFB4I_ERR                             :  1;		/* 19..19, 0x00080000 */
				FIELD  MFBI_B_ERR                            :  1;		/* 20..20, 0x00100000 */
				FIELD  MFB2I_B_ERR                           :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  9;		/* 22..30, 0x7FC00000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_ERR_CTRL;	/* MFB_A_DMA_ERR_CTRL */

typedef volatile union _MFB_REG_A_MFBO_ERR_STAT_
{
		volatile struct	/* 0x1502E254 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_ERR_STAT;	/* MFB_A_MFBO_ERR_STAT */

typedef volatile union _MFB_REG_A_MFB2O_ERR_STAT_
{
		volatile struct	/* 0x1502E258 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2O_ERR_STAT;	/* MFB_A_MFB2O_ERR_STAT */

typedef volatile union _MFB_REG_A_MFBO_B_ERR_STAT_
{
		volatile struct	/* 0x1502E25C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_ERR_STAT;	/* MFB_A_MFBO_B_ERR_STAT */

typedef volatile union _MFB_REG_A_MFBI_ERR_STAT_
{
		volatile struct	/* 0x1502E260 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_ERR_STAT;	/* MFB_A_MFBI_ERR_STAT */

typedef volatile union _MFB_REG_A_MFB2I_ERR_STAT_
{
		volatile struct	/* 0x1502E264 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_ERR_STAT;	/* MFB_A_MFB2I_ERR_STAT */

typedef volatile union _MFB_REG_A_MFB3I_ERR_STAT_
{
		volatile struct	/* 0x1502E268 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB3I_ERR_STAT;	/* MFB_A_MFB3I_ERR_STAT */

typedef volatile union _MFB_REG_A_MFB4I_ERR_STAT_
{
		volatile struct	/* 0x1502E26C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB4I_ERR_STAT;	/* MFB_A_MFB4I_ERR_STAT */

typedef volatile union _MFB_REG_A_MFBI_B_ERR_STAT_
{
		volatile struct	/* 0x1502E270 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_ERR_STAT;	/* MFB_A_MFBI_B_ERR_STAT */

typedef volatile union _MFB_REG_A_MFB2I_B_ERR_STAT_
{
		volatile struct	/* 0x1502E274 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_ERR_STAT;	/* MFB_A_MFB2I_B_ERR_STAT */

typedef volatile union _MFB_REG_A_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1502E278 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_DEBUG_ADDR;	/* MFB_A_DMA_DEBUG_ADDR */

typedef volatile union _MFB_REG_A_DMA_RSV1_
{
		volatile struct	/* 0x1502E27C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_RSV1;	/* MFB_A_DMA_RSV1 */

typedef volatile union _MFB_REG_A_DMA_RSV2_
{
		volatile struct	/* 0x1502E280 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_RSV2;	/* MFB_A_DMA_RSV2 */

typedef volatile union _MFB_REG_A_DMA_RSV3_
{
		volatile struct	/* 0x1502E284 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_RSV3;	/* MFB_A_DMA_RSV3 */

typedef volatile union _MFB_REG_A_DMA_RSV4_
{
		volatile struct	/* 0x1502E288 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_RSV4;	/* MFB_A_DMA_RSV4 */

typedef volatile union _MFB_REG_A_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x1502E28C */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_DEBUG_SEL;	/* MFB_A_DMA_DEBUG_SEL */

typedef volatile union _MFB_REG_A_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1502E290 */
		{
				FIELD  BW_SELF_TEST_EN_MFBO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_MFB2O                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_MFBO_B                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_DMA_BW_SELF_TEST;	/* MFB_A_DMA_BW_SELF_TEST */

typedef volatile union _MFB_REG_A_MFBO_B_BASE_ADDR_
{
		volatile struct	/* 0x1502E2A0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_BASE_ADDR;	/* MFB_A_MFBO_B_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFBO_B_OFST_ADDR_
{
		volatile struct	/* 0x1502E2A8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_OFST_ADDR;	/* MFB_A_MFBO_B_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFBO_B_XSIZE_
{
		volatile struct	/* 0x1502E2B0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_XSIZE;	/* MFB_A_MFBO_B_XSIZE */

typedef volatile union _MFB_REG_A_MFBO_B_YSIZE_
{
		volatile struct	/* 0x1502E2B4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_YSIZE;	/* MFB_A_MFBO_B_YSIZE */

typedef volatile union _MFB_REG_A_MFBO_B_STRIDE_
{
		volatile struct	/* 0x1502E2B8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_STRIDE;	/* MFB_A_MFBO_B_STRIDE */

typedef volatile union _MFB_REG_A_MFBO_B_CON_
{
		volatile struct	/* 0x1502E2BC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_CON;	/* MFB_A_MFBO_B_CON */

typedef volatile union _MFB_REG_A_MFBO_B_CON2_
{
		volatile struct	/* 0x1502E2C0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_CON2;	/* MFB_A_MFBO_B_CON2 */

typedef volatile union _MFB_REG_A_MFBO_B_CON3_
{
		volatile struct	/* 0x1502E2C4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_CON3;	/* MFB_A_MFBO_B_CON3 */

typedef volatile union _MFB_REG_A_MFBO_B_CROP_
{
		volatile struct	/* 0x1502E2C8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBO_B_CROP;	/* MFB_A_MFBO_B_CROP */

typedef volatile union _MFB_REG_A_MFBI_B_BASE_ADDR_
{
		volatile struct	/* 0x1502E2D0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_BASE_ADDR;	/* MFB_A_MFBI_B_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFBI_B_OFST_ADDR_
{
		volatile struct	/* 0x1502E2D8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_OFST_ADDR;	/* MFB_A_MFBI_B_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFBI_B_XSIZE_
{
		volatile struct	/* 0x1502E2E0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_XSIZE;	/* MFB_A_MFBI_B_XSIZE */

typedef volatile union _MFB_REG_A_MFBI_B_YSIZE_
{
		volatile struct	/* 0x1502E2E4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_YSIZE;	/* MFB_A_MFBI_B_YSIZE */

typedef volatile union _MFB_REG_A_MFBI_B_STRIDE_
{
		volatile struct	/* 0x1502E2E8 */
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
}MFB_REG_A_MFBI_B_STRIDE;	/* MFB_A_MFBI_B_STRIDE */

typedef volatile union _MFB_REG_A_MFBI_B_CON_
{
		volatile struct	/* 0x1502E2EC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_CON;	/* MFB_A_MFBI_B_CON */

typedef volatile union _MFB_REG_A_MFBI_B_CON2_
{
		volatile struct	/* 0x1502E2F0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_CON2;	/* MFB_A_MFBI_B_CON2 */

typedef volatile union _MFB_REG_A_MFBI_B_CON3_
{
		volatile struct	/* 0x1502E2F4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFBI_B_CON3;	/* MFB_A_MFBI_B_CON3 */

typedef volatile union _MFB_REG_A_MFB2I_B_BASE_ADDR_
{
		volatile struct	/* 0x1502E300 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_BASE_ADDR;	/* MFB_A_MFB2I_B_BASE_ADDR */

typedef volatile union _MFB_REG_A_MFB2I_B_OFST_ADDR_
{
		volatile struct	/* 0x1502E308 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_OFST_ADDR;	/* MFB_A_MFB2I_B_OFST_ADDR */

typedef volatile union _MFB_REG_A_MFB2I_B_XSIZE_
{
		volatile struct	/* 0x1502E310 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_XSIZE;	/* MFB_A_MFB2I_B_XSIZE */

typedef volatile union _MFB_REG_A_MFB2I_B_YSIZE_
{
		volatile struct	/* 0x1502E314 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_YSIZE;	/* MFB_A_MFB2I_B_YSIZE */

typedef volatile union _MFB_REG_A_MFB2I_B_STRIDE_
{
		volatile struct	/* 0x1502E318 */
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
}MFB_REG_A_MFB2I_B_STRIDE;	/* MFB_A_MFB2I_B_STRIDE */

typedef volatile union _MFB_REG_A_MFB2I_B_CON_
{
		volatile struct	/* 0x1502E31C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_CON;	/* MFB_A_MFB2I_B_CON */

typedef volatile union _MFB_REG_A_MFB2I_B_CON2_
{
		volatile struct	/* 0x1502E320 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_CON2;	/* MFB_A_MFB2I_B_CON2 */

typedef volatile union _MFB_REG_A_MFB2I_B_CON3_
{
		volatile struct	/* 0x1502E324 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_MFB2I_B_CON3;	/* MFB_A_MFB2I_B_CON3 */

typedef volatile union _MFB_REG_A_SRZ_CONTROL_
{
		volatile struct	/* 0x1502E340 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_CONTROL;	/* MFB_A_SRZ_CONTROL */

typedef volatile union _MFB_REG_A_SRZ_IN_IMG_
{
		volatile struct	/* 0x1502E344 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_IN_IMG;	/* MFB_A_SRZ_IN_IMG */

typedef volatile union _MFB_REG_A_SRZ_OUT_IMG_
{
		volatile struct	/* 0x1502E348 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_OUT_IMG;	/* MFB_A_SRZ_OUT_IMG */

typedef volatile union _MFB_REG_A_SRZ_HORI_STEP_
{
		volatile struct	/* 0x1502E34C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_HORI_STEP;	/* MFB_A_SRZ_HORI_STEP */

typedef volatile union _MFB_REG_A_SRZ_VERT_STEP_
{
		volatile struct	/* 0x1502E350 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_VERT_STEP;	/* MFB_A_SRZ_VERT_STEP */

typedef volatile union _MFB_REG_A_SRZ_HORI_INT_OFST_
{
		volatile struct	/* 0x1502E354 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_HORI_INT_OFST;	/* MFB_A_SRZ_HORI_INT_OFST */

typedef volatile union _MFB_REG_A_SRZ_HORI_SUB_OFST_
{
		volatile struct	/* 0x1502E358 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_HORI_SUB_OFST;	/* MFB_A_SRZ_HORI_SUB_OFST */

typedef volatile union _MFB_REG_A_SRZ_VERT_INT_OFST_
{
		volatile struct	/* 0x1502E35C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_VERT_INT_OFST;	/* MFB_A_SRZ_VERT_INT_OFST */

typedef volatile union _MFB_REG_A_SRZ_VERT_SUB_OFST_
{
		volatile struct	/* 0x1502E360 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_SRZ_VERT_SUB_OFST;	/* MFB_A_SRZ_VERT_SUB_OFST */

typedef volatile union _MFB_REG_A_C02A_CON_
{
		volatile struct	/* 0x1502E380 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  C02_INTERP_MODE                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02A_CON;	/* MFB_A_C02A_CON */

typedef volatile union _MFB_REG_A_C02A_CROP_CON1_
{
		volatile struct	/* 0x1502E384 */
		{
				FIELD  C02_CROP_XSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_XEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02A_CROP_CON1;	/* MFB_A_C02A_CROP_CON1 */

typedef volatile union _MFB_REG_A_C02A_CROP_CON2_
{
		volatile struct	/* 0x1502E388 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02A_CROP_CON2;	/* MFB_A_C02A_CROP_CON2 */

typedef volatile union _MFB_REG_A_C02B_CON_
{
		volatile struct	/* 0x1502E3A0 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  C02_INTERP_MODE                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02B_CON;	/* MFB_A_C02B_CON */

typedef volatile union _MFB_REG_A_C02B_CROP_CON1_
{
		volatile struct	/* 0x1502E3A4 */
		{
				FIELD  C02_CROP_XSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_XEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02B_CROP_CON1;	/* MFB_A_C02B_CROP_CON1 */

typedef volatile union _MFB_REG_A_C02B_CROP_CON2_
{
		volatile struct	/* 0x1502E3A8 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_C02B_CROP_CON2;	/* MFB_A_C02B_CROP_CON2 */

typedef volatile union _MFB_REG_A_CRSP_CTRL_
{
		volatile struct	/* 0x1502E3C0 */
		{
				FIELD  CRSP_HORI_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CRSP_VERT_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CRSP_OUTPUT_WAIT_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CRSP_RSV_1                            : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  CRSP_CROP_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_CRSP_CTRL;	/* MFB_A_CRSP_CTRL */

typedef volatile union _MFB_REG_A_CRSP_OUT_IMG_
{
		volatile struct	/* 0x1502E3C8 */
		{
				FIELD  CRSP_WD                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_HT                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_CRSP_OUT_IMG;	/* MFB_A_CRSP_OUT_IMG */

typedef volatile union _MFB_REG_A_CRSP_STEP_OFST_
{
		volatile struct	/* 0x1502E3CC */
		{
				FIELD  CRSP_STEP_X                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  CRSP_OFST_X                           :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  CRSP_STEP_Y                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  CRSP_OFST_Y                           :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_CRSP_STEP_OFST;	/* MFB_A_CRSP_STEP_OFST */

typedef volatile union _MFB_REG_A_CRSP_CROP_X_
{
		volatile struct	/* 0x1502E3D0 */
		{
				FIELD  CRSP_CROP_STR_X                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_X                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_CRSP_CROP_X;	/* MFB_A_CRSP_CROP_X */

typedef volatile union _MFB_REG_A_CRSP_CROP_Y_
{
		volatile struct	/* 0x1502E3D4 */
		{
				FIELD  CRSP_CROP_STR_Y                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_Y                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MFB_REG_A_CRSP_CROP_Y;	/* MFB_A_CRSP_CROP_Y */

typedef volatile struct _mfb_reg_t_	/* 0x1502E000..0x1502E3FF */
{
	MFB_REG_A_MFB_CON                               MFB_A_MFB_CON;                                   /* 0000, 0x1502E000 */
	MFB_REG_A_MFB_LL_CON1                           MFB_A_MFB_LL_CON1;                               /* 0004, 0x1502E004 */
	MFB_REG_A_MFB_LL_CON2                           MFB_A_MFB_LL_CON2;                               /* 0008, 0x1502E008 */
	MFB_REG_A_MFB_LL_CON3                           MFB_A_MFB_LL_CON3;                               /* 000C, 0x1502E00C */
	MFB_REG_A_MFB_LL_CON4                           MFB_A_MFB_LL_CON4;                               /* 0010, 0x1502E010 */
	MFB_REG_A_MFB_EDGE                              MFB_A_MFB_EDGE;                                  /* 0014, 0x1502E014 */
	MFB_REG_A_MFB_LL_CON5                           MFB_A_MFB_LL_CON5;                               /* 0018, 0x1502E018 */
	MFB_REG_A_MFB_LL_CON6                           MFB_A_MFB_LL_CON6;                               /* 001C, 0x1502E01C */
	MFB_REG_A_MFB_LL_CON7                           MFB_A_MFB_LL_CON7;                               /* 0020, 0x1502E020 */
	MFB_REG_A_MFB_LL_CON8                           MFB_A_MFB_LL_CON8;                               /* 0024, 0x1502E024 */
	MFB_REG_A_MFB_LL_CON9                           MFB_A_MFB_LL_CON9;                               /* 0028, 0x1502E028 */
	MFB_REG_A_MFB_LL_CON10                          MFB_A_MFB_LL_CON10;                              /* 002C, 0x1502E02C */
	MFB_REG_A_MFB_MBD_CON0                          MFB_A_MFB_MBD_CON0;                              /* 0030, 0x1502E030 */
	MFB_REG_A_MFB_MBD_CON1                          MFB_A_MFB_MBD_CON1;                              /* 0034, 0x1502E034 */
	MFB_REG_A_MFB_MBD_CON2                          MFB_A_MFB_MBD_CON2;                              /* 0038, 0x1502E038 */
	MFB_REG_A_MFB_MBD_CON3                          MFB_A_MFB_MBD_CON3;                              /* 003C, 0x1502E03C */
	MFB_REG_A_MFB_MBD_CON4                          MFB_A_MFB_MBD_CON4;                              /* 0040, 0x1502E040 */
	MFB_REG_A_MFB_MBD_CON5                          MFB_A_MFB_MBD_CON5;                              /* 0044, 0x1502E044 */
	MFB_REG_A_MFB_MBD_CON6                          MFB_A_MFB_MBD_CON6;                              /* 0048, 0x1502E048 */
	MFB_REG_A_MFB_MBD_CON7                          MFB_A_MFB_MBD_CON7;                              /* 004C, 0x1502E04C */
	MFB_REG_A_MFB_MBD_CON8                          MFB_A_MFB_MBD_CON8;                              /* 0050, 0x1502E050 */
	MFB_REG_A_MFB_MBD_CON9                          MFB_A_MFB_MBD_CON9;                              /* 0054, 0x1502E054 */
	MFB_REG_A_MFB_MBD_CON10                         MFB_A_MFB_MBD_CON10;                             /* 0058, 0x1502E058 */
	MFB_REG_A_MFB_TOP_CFG0                          MFB_A_MFB_TOP_CFG0;                              /* 005C, 0x1502E05C */
	MFB_REG_A_MFB_TOP_CFG1                          MFB_A_MFB_TOP_CFG1;                              /* 0060, 0x1502E060 */
	MFB_REG_A_MFB_TOP_CFG2                          MFB_A_MFB_TOP_CFG2;                              /* 0064, 0x1502E064 */
	UINT32                                          rsv_0068[6];                                     /* 0068..007F, 0x1502E068..1502E07F */
	MFB_REG_A_MFB_INT_CTL                           MFB_A_MFB_INT_CTL;                               /* 0080, 0x1502E080 */
	MFB_REG_A_MFB_INT_STATUS                        MFB_A_MFB_INT_STATUS;                            /* 0084, 0x1502E084 */
	MFB_REG_A_MFB_SW_RST                            MFB_A_MFB_SW_RST;                                /* 0088, 0x1502E088 */
	MFB_REG_A_MFB_MAIN_DCM_ST                       MFB_A_MFB_MAIN_DCM_ST;                           /* 008C, 0x1502E08C */
	MFB_REG_A_MFB_DMA_DCM_ST                        MFB_A_MFB_DMA_DCM_ST;                            /* 0090, 0x1502E090 */
	MFB_REG_A_MFB_MAIN_DCM_DIS                      MFB_A_MFB_MAIN_DCM_DIS;                          /* 0094, 0x1502E094 */
	MFB_REG_A_MFB_DMA_DCM_DIS                       MFB_A_MFB_DMA_DCM_DIS;                           /* 0098, 0x1502E098 */
	MFB_REG_A_MFB_DBG_CTL0                          MFB_A_MFB_DBG_CTL0;                              /* 009C, 0x1502E09C */
	MFB_REG_A_MFB_DBG_OUT0                          MFB_A_MFB_DBG_OUT0;                              /* 00A0, 0x1502E0A0 */
	MFB_REG_A_MFB_DBG_OUT1                          MFB_A_MFB_DBG_OUT1;                              /* 00A4, 0x1502E0A4 */
	MFB_REG_A_MFB_DBG_OUT2                          MFB_A_MFB_DBG_OUT2;                              /* 00A8, 0x1502E0A8 */
	MFB_REG_A_MFB_DBG_OUT3                          MFB_A_MFB_DBG_OUT3;                              /* 00AC, 0x1502E0AC */
	MFB_REG_A_MFB_DBG_OUT4                          MFB_A_MFB_DBG_OUT4;                              /* 00B0, 0x1502E0B0 */
	UINT32                                          rsv_00B4[19];                                    /* 00B4..00FF, 0x1502E0B4..1502E0FF */
	MFB_REG_A_DMA_SOFT_RSTSTAT                      MFB_A_DMA_SOFT_RSTSTAT;                          /* 0100, 0x1502E100 */
	MFB_REG_A_TDRI_BASE_ADDR                        MFB_A_TDRI_BASE_ADDR;                            /* 0104, 0x1502E104 */
	MFB_REG_A_TDRI_OFST_ADDR                        MFB_A_TDRI_OFST_ADDR;                            /* 0108, 0x1502E108 */
	MFB_REG_A_TDRI_XSIZE                            MFB_A_TDRI_XSIZE;                                /* 010C, 0x1502E10C */
	MFB_REG_A_VERTICAL_FLIP_EN                      MFB_A_VERTICAL_FLIP_EN;                          /* 0110, 0x1502E110 */
	MFB_REG_A_DMA_SOFT_RESET                        MFB_A_DMA_SOFT_RESET;                            /* 0114, 0x1502E114 */
	MFB_REG_A_LAST_ULTRA_EN                         MFB_A_LAST_ULTRA_EN;                             /* 0118, 0x1502E118 */
	MFB_REG_A_SPECIAL_FUN_EN                        MFB_A_SPECIAL_FUN_EN;                            /* 011C, 0x1502E11C */
	UINT32                                          rsv_0120[4];                                     /* 0120..012F, 0x1502E120..1502E12F */
	MFB_REG_A_MFBO_BASE_ADDR                        MFB_A_MFBO_BASE_ADDR;                            /* 0130, 0x1502E130 */
	UINT32                                          rsv_0134;                                        /* 0134, 0x1502E134 */
	MFB_REG_A_MFBO_OFST_ADDR                        MFB_A_MFBO_OFST_ADDR;                            /* 0138, 0x1502E138 */
	UINT32                                          rsv_013C;                                        /* 013C, 0x1502E13C */
	MFB_REG_A_MFBO_XSIZE                            MFB_A_MFBO_XSIZE;                                /* 0140, 0x1502E140 */
	MFB_REG_A_MFBO_YSIZE                            MFB_A_MFBO_YSIZE;                                /* 0144, 0x1502E144 */
	MFB_REG_A_MFBO_STRIDE                           MFB_A_MFBO_STRIDE;                               /* 0148, 0x1502E148 */
	MFB_REG_A_MFBO_CON                              MFB_A_MFBO_CON;                                  /* 014C, 0x1502E14C */
	MFB_REG_A_MFBO_CON2                             MFB_A_MFBO_CON2;                                 /* 0150, 0x1502E150 */
	MFB_REG_A_MFBO_CON3                             MFB_A_MFBO_CON3;                                 /* 0154, 0x1502E154 */
	MFB_REG_A_MFBO_CROP                             MFB_A_MFBO_CROP;                                 /* 0158, 0x1502E158 */
	UINT32                                          rsv_015C;                                        /* 015C, 0x1502E15C */
	MFB_REG_A_MFB2O_BASE_ADDR                       MFB_A_MFB2O_BASE_ADDR;                           /* 0160, 0x1502E160 */
	UINT32                                          rsv_0164;                                        /* 0164, 0x1502E164 */
	MFB_REG_A_MFB2O_OFST_ADDR                       MFB_A_MFB2O_OFST_ADDR;                           /* 0168, 0x1502E168 */
	UINT32                                          rsv_016C;                                        /* 016C, 0x1502E16C */
	MFB_REG_A_MFB2O_XSIZE                           MFB_A_MFB2O_XSIZE;                               /* 0170, 0x1502E170 */
	MFB_REG_A_MFB2O_YSIZE                           MFB_A_MFB2O_YSIZE;                               /* 0174, 0x1502E174 */
	MFB_REG_A_MFB2O_STRIDE                          MFB_A_MFB2O_STRIDE;                              /* 0178, 0x1502E178 */
	MFB_REG_A_MFB2O_CON                             MFB_A_MFB2O_CON;                                 /* 017C, 0x1502E17C */
	MFB_REG_A_MFB2O_CON2                            MFB_A_MFB2O_CON2;                                /* 0180, 0x1502E180 */
	MFB_REG_A_MFB2O_CON3                            MFB_A_MFB2O_CON3;                                /* 0184, 0x1502E184 */
	MFB_REG_A_MFB2O_CROP                            MFB_A_MFB2O_CROP;                                /* 0188, 0x1502E188 */
	UINT32                                          rsv_018C;                                        /* 018C, 0x1502E18C */
	MFB_REG_A_MFBI_BASE_ADDR                        MFB_A_MFBI_BASE_ADDR;                            /* 0190, 0x1502E190 */
	UINT32                                          rsv_0194;                                        /* 0194, 0x1502E194 */
	MFB_REG_A_MFBI_OFST_ADDR                        MFB_A_MFBI_OFST_ADDR;                            /* 0198, 0x1502E198 */
	UINT32                                          rsv_019C;                                        /* 019C, 0x1502E19C */
	MFB_REG_A_MFBI_XSIZE                            MFB_A_MFBI_XSIZE;                                /* 01A0, 0x1502E1A0 */
	MFB_REG_A_MFBI_YSIZE                            MFB_A_MFBI_YSIZE;                                /* 01A4, 0x1502E1A4 */
	MFB_REG_A_MFBI_STRIDE                           MFB_A_MFBI_STRIDE;                               /* 01A8, 0x1502E1A8 */
	MFB_REG_A_MFBI_CON                              MFB_A_MFBI_CON;                                  /* 01AC, 0x1502E1AC */
	MFB_REG_A_MFBI_CON2                             MFB_A_MFBI_CON2;                                 /* 01B0, 0x1502E1B0 */
	MFB_REG_A_MFBI_CON3                             MFB_A_MFBI_CON3;                                 /* 01B4, 0x1502E1B4 */
	UINT32                                          rsv_01B8[2];                                     /* 01B8..01BF, 0x1502E1B8..1502E1BF */
	MFB_REG_A_MFB2I_BASE_ADDR                       MFB_A_MFB2I_BASE_ADDR;                           /* 01C0, 0x1502E1C0 */
	UINT32                                          rsv_01C4;                                        /* 01C4, 0x1502E1C4 */
	MFB_REG_A_MFB2I_OFST_ADDR                       MFB_A_MFB2I_OFST_ADDR;                           /* 01C8, 0x1502E1C8 */
	UINT32                                          rsv_01CC;                                        /* 01CC, 0x1502E1CC */
	MFB_REG_A_MFB2I_XSIZE                           MFB_A_MFB2I_XSIZE;                               /* 01D0, 0x1502E1D0 */
	MFB_REG_A_MFB2I_YSIZE                           MFB_A_MFB2I_YSIZE;                               /* 01D4, 0x1502E1D4 */
	MFB_REG_A_MFB2I_STRIDE                          MFB_A_MFB2I_STRIDE;                              /* 01D8, 0x1502E1D8 */
	MFB_REG_A_MFB2I_CON                             MFB_A_MFB2I_CON;                                 /* 01DC, 0x1502E1DC */
	MFB_REG_A_MFB2I_CON2                            MFB_A_MFB2I_CON2;                                /* 01E0, 0x1502E1E0 */
	MFB_REG_A_MFB2I_CON3                            MFB_A_MFB2I_CON3;                                /* 01E4, 0x1502E1E4 */
	UINT32                                          rsv_01E8[2];                                     /* 01E8..01EF, 0x1502E1E8..1502E1EF */
	MFB_REG_A_MFB3I_BASE_ADDR                       MFB_A_MFB3I_BASE_ADDR;                           /* 01F0, 0x1502E1F0 */
	UINT32                                          rsv_01F4;                                        /* 01F4, 0x1502E1F4 */
	MFB_REG_A_MFB3I_OFST_ADDR                       MFB_A_MFB3I_OFST_ADDR;                           /* 01F8, 0x1502E1F8 */
	UINT32                                          rsv_01FC;                                        /* 01FC, 0x1502E1FC */
	MFB_REG_A_MFB3I_XSIZE                           MFB_A_MFB3I_XSIZE;                               /* 0200, 0x1502E200 */
	MFB_REG_A_MFB3I_YSIZE                           MFB_A_MFB3I_YSIZE;                               /* 0204, 0x1502E204 */
	MFB_REG_A_MFB3I_STRIDE                          MFB_A_MFB3I_STRIDE;                              /* 0208, 0x1502E208 */
	MFB_REG_A_MFB3I_CON                             MFB_A_MFB3I_CON;                                 /* 020C, 0x1502E20C */
	MFB_REG_A_MFB3I_CON2                            MFB_A_MFB3I_CON2;                                /* 0210, 0x1502E210 */
	MFB_REG_A_MFB3I_CON3                            MFB_A_MFB3I_CON3;                                /* 0214, 0x1502E214 */
	UINT32                                          rsv_0218[2];                                     /* 0218..021F, 0x1502E218..1502E21F */
	MFB_REG_A_MFB4I_BASE_ADDR                       MFB_A_MFB4I_BASE_ADDR;                           /* 0220, 0x1502E220 */
	UINT32                                          rsv_0224;                                        /* 0224, 0x1502E224 */
	MFB_REG_A_MFB4I_OFST_ADDR                       MFB_A_MFB4I_OFST_ADDR;                           /* 0228, 0x1502E228 */
	UINT32                                          rsv_022C;                                        /* 022C, 0x1502E22C */
	MFB_REG_A_MFB4I_XSIZE                           MFB_A_MFB4I_XSIZE;                               /* 0230, 0x1502E230 */
	MFB_REG_A_MFB4I_YSIZE                           MFB_A_MFB4I_YSIZE;                               /* 0234, 0x1502E234 */
	MFB_REG_A_MFB4I_STRIDE                          MFB_A_MFB4I_STRIDE;                              /* 0238, 0x1502E238 */
	MFB_REG_A_MFB4I_CON                             MFB_A_MFB4I_CON;                                 /* 023C, 0x1502E23C */
	MFB_REG_A_MFB4I_CON2                            MFB_A_MFB4I_CON2;                                /* 0240, 0x1502E240 */
	MFB_REG_A_MFB4I_CON3                            MFB_A_MFB4I_CON3;                                /* 0244, 0x1502E244 */
	UINT32                                          rsv_0248[2];                                     /* 0248..024F, 0x1502E248..1502E24F */
	MFB_REG_A_DMA_ERR_CTRL                          MFB_A_DMA_ERR_CTRL;                              /* 0250, 0x1502E250 */
	MFB_REG_A_MFBO_ERR_STAT                         MFB_A_MFBO_ERR_STAT;                             /* 0254, 0x1502E254 */
	MFB_REG_A_MFB2O_ERR_STAT                        MFB_A_MFB2O_ERR_STAT;                            /* 0258, 0x1502E258 */
	MFB_REG_A_MFBO_B_ERR_STAT                       MFB_A_MFBO_B_ERR_STAT;                           /* 025C, 0x1502E25C */
	MFB_REG_A_MFBI_ERR_STAT                         MFB_A_MFBI_ERR_STAT;                             /* 0260, 0x1502E260 */
	MFB_REG_A_MFB2I_ERR_STAT                        MFB_A_MFB2I_ERR_STAT;                            /* 0264, 0x1502E264 */
	MFB_REG_A_MFB3I_ERR_STAT                        MFB_A_MFB3I_ERR_STAT;                            /* 0268, 0x1502E268 */
	MFB_REG_A_MFB4I_ERR_STAT                        MFB_A_MFB4I_ERR_STAT;                            /* 026C, 0x1502E26C */
	MFB_REG_A_MFBI_B_ERR_STAT                       MFB_A_MFBI_B_ERR_STAT;                           /* 0270, 0x1502E270 */
	MFB_REG_A_MFB2I_B_ERR_STAT                      MFB_A_MFB2I_B_ERR_STAT;                          /* 0274, 0x1502E274 */
	MFB_REG_A_DMA_DEBUG_ADDR                        MFB_A_DMA_DEBUG_ADDR;                            /* 0278, 0x1502E278 */
	MFB_REG_A_DMA_RSV1                              MFB_A_DMA_RSV1;                                  /* 027C, 0x1502E27C */
	MFB_REG_A_DMA_RSV2                              MFB_A_DMA_RSV2;                                  /* 0280, 0x1502E280 */
	MFB_REG_A_DMA_RSV3                              MFB_A_DMA_RSV3;                                  /* 0284, 0x1502E284 */
	MFB_REG_A_DMA_RSV4                              MFB_A_DMA_RSV4;                                  /* 0288, 0x1502E288 */
	MFB_REG_A_DMA_DEBUG_SEL                         MFB_A_DMA_DEBUG_SEL;                             /* 028C, 0x1502E28C */
	MFB_REG_A_DMA_BW_SELF_TEST                      MFB_A_DMA_BW_SELF_TEST;                          /* 0290, 0x1502E290 */
	UINT32                                          rsv_0294[3];                                     /* 0294..029F, 0x1502E294..1502E29F */
	MFB_REG_A_MFBO_B_BASE_ADDR                      MFB_A_MFBO_B_BASE_ADDR;                          /* 02A0, 0x1502E2A0 */
	UINT32                                          rsv_02A4;                                        /* 02A4, 0x1502E2A4 */
	MFB_REG_A_MFBO_B_OFST_ADDR                      MFB_A_MFBO_B_OFST_ADDR;                          /* 02A8, 0x1502E2A8 */
	UINT32                                          rsv_02AC;                                        /* 02AC, 0x1502E2AC */
	MFB_REG_A_MFBO_B_XSIZE                          MFB_A_MFBO_B_XSIZE;                              /* 02B0, 0x1502E2B0 */
	MFB_REG_A_MFBO_B_YSIZE                          MFB_A_MFBO_B_YSIZE;                              /* 02B4, 0x1502E2B4 */
	MFB_REG_A_MFBO_B_STRIDE                         MFB_A_MFBO_B_STRIDE;                             /* 02B8, 0x1502E2B8 */
	MFB_REG_A_MFBO_B_CON                            MFB_A_MFBO_B_CON;                                /* 02BC, 0x1502E2BC */
	MFB_REG_A_MFBO_B_CON2                           MFB_A_MFBO_B_CON2;                               /* 02C0, 0x1502E2C0 */
	MFB_REG_A_MFBO_B_CON3                           MFB_A_MFBO_B_CON3;                               /* 02C4, 0x1502E2C4 */
	MFB_REG_A_MFBO_B_CROP                           MFB_A_MFBO_B_CROP;                               /* 02C8, 0x1502E2C8 */
	UINT32                                          rsv_02CC;                                        /* 02CC, 0x1502E2CC */
	MFB_REG_A_MFBI_B_BASE_ADDR                      MFB_A_MFBI_B_BASE_ADDR;                          /* 02D0, 0x1502E2D0 */
	UINT32                                          rsv_02D4;                                        /* 02D4, 0x1502E2D4 */
	MFB_REG_A_MFBI_B_OFST_ADDR                      MFB_A_MFBI_B_OFST_ADDR;                          /* 02D8, 0x1502E2D8 */
	UINT32                                          rsv_02DC;                                        /* 02DC, 0x1502E2DC */
	MFB_REG_A_MFBI_B_XSIZE                          MFB_A_MFBI_B_XSIZE;                              /* 02E0, 0x1502E2E0 */
	MFB_REG_A_MFBI_B_YSIZE                          MFB_A_MFBI_B_YSIZE;                              /* 02E4, 0x1502E2E4 */
	MFB_REG_A_MFBI_B_STRIDE                         MFB_A_MFBI_B_STRIDE;                             /* 02E8, 0x1502E2E8 */
	MFB_REG_A_MFBI_B_CON                            MFB_A_MFBI_B_CON;                                /* 02EC, 0x1502E2EC */
	MFB_REG_A_MFBI_B_CON2                           MFB_A_MFBI_B_CON2;                               /* 02F0, 0x1502E2F0 */
	MFB_REG_A_MFBI_B_CON3                           MFB_A_MFBI_B_CON3;                               /* 02F4, 0x1502E2F4 */
	UINT32                                          rsv_02F8[2];                                     /* 02F8..02FF, 0x1502E2F8..1502E2FF */
	MFB_REG_A_MFB2I_B_BASE_ADDR                     MFB_A_MFB2I_B_BASE_ADDR;                         /* 0300, 0x1502E300 */
	UINT32                                          rsv_0304;                                        /* 0304, 0x1502E304 */
	MFB_REG_A_MFB2I_B_OFST_ADDR                     MFB_A_MFB2I_B_OFST_ADDR;                         /* 0308, 0x1502E308 */
	UINT32                                          rsv_030C;                                        /* 030C, 0x1502E30C */
	MFB_REG_A_MFB2I_B_XSIZE                         MFB_A_MFB2I_B_XSIZE;                             /* 0310, 0x1502E310 */
	MFB_REG_A_MFB2I_B_YSIZE                         MFB_A_MFB2I_B_YSIZE;                             /* 0314, 0x1502E314 */
	MFB_REG_A_MFB2I_B_STRIDE                        MFB_A_MFB2I_B_STRIDE;                            /* 0318, 0x1502E318 */
	MFB_REG_A_MFB2I_B_CON                           MFB_A_MFB2I_B_CON;                               /* 031C, 0x1502E31C */
	MFB_REG_A_MFB2I_B_CON2                          MFB_A_MFB2I_B_CON2;                              /* 0320, 0x1502E320 */
	MFB_REG_A_MFB2I_B_CON3                          MFB_A_MFB2I_B_CON3;                              /* 0324, 0x1502E324 */
	UINT32                                          rsv_0328[6];                                     /* 0328..033F, 0x1502E328..1502E33F */
	MFB_REG_A_SRZ_CONTROL                           MFB_A_SRZ_CONTROL;                               /* 0340, 0x1502E340 */
	MFB_REG_A_SRZ_IN_IMG                            MFB_A_SRZ_IN_IMG;                                /* 0344, 0x1502E344 */
	MFB_REG_A_SRZ_OUT_IMG                           MFB_A_SRZ_OUT_IMG;                               /* 0348, 0x1502E348 */
	MFB_REG_A_SRZ_HORI_STEP                         MFB_A_SRZ_HORI_STEP;                             /* 034C, 0x1502E34C */
	MFB_REG_A_SRZ_VERT_STEP                         MFB_A_SRZ_VERT_STEP;                             /* 0350, 0x1502E350 */
	MFB_REG_A_SRZ_HORI_INT_OFST                     MFB_A_SRZ_HORI_INT_OFST;                         /* 0354, 0x1502E354 */
	MFB_REG_A_SRZ_HORI_SUB_OFST                     MFB_A_SRZ_HORI_SUB_OFST;                         /* 0358, 0x1502E358 */
	MFB_REG_A_SRZ_VERT_INT_OFST                     MFB_A_SRZ_VERT_INT_OFST;                         /* 035C, 0x1502E35C */
	MFB_REG_A_SRZ_VERT_SUB_OFST                     MFB_A_SRZ_VERT_SUB_OFST;                         /* 0360, 0x1502E360 */
	UINT32                                          rsv_0364[7];                                     /* 0364..037F, 0x1502E364..1502E37F */
	MFB_REG_A_C02A_CON                              MFB_A_C02A_CON;                                  /* 0380, 0x1502E380 */
	MFB_REG_A_C02A_CROP_CON1                        MFB_A_C02A_CROP_CON1;                            /* 0384, 0x1502E384 */
	MFB_REG_A_C02A_CROP_CON2                        MFB_A_C02A_CROP_CON2;                            /* 0388, 0x1502E388 */
	UINT32                                          rsv_038C[5];                                     /* 038C..039F, 0x1502E38C..1502E39F */
	MFB_REG_A_C02B_CON                              MFB_A_C02B_CON;                                  /* 03A0, 0x1502E3A0 */
	MFB_REG_A_C02B_CROP_CON1                        MFB_A_C02B_CROP_CON1;                            /* 03A4, 0x1502E3A4 */
	MFB_REG_A_C02B_CROP_CON2                        MFB_A_C02B_CROP_CON2;                            /* 03A8, 0x1502E3A8 */
	UINT32                                          rsv_03AC[5];                                     /* 03AC..03BF, 0x1502E3AC..1502E3BF */
	MFB_REG_A_CRSP_CTRL                             MFB_A_CRSP_CTRL;                                 /* 03C0, 0x1502E3C0 */
	UINT32                                          rsv_03C4;                                        /* 03C4, 0x1502E3C4 */
	MFB_REG_A_CRSP_OUT_IMG                          MFB_A_CRSP_OUT_IMG;                              /* 03C8, 0x1502E3C8 */
	MFB_REG_A_CRSP_STEP_OFST                        MFB_A_CRSP_STEP_OFST;                            /* 03CC, 0x1502E3CC */
	MFB_REG_A_CRSP_CROP_X                           MFB_A_CRSP_CROP_X;                               /* 03D0, 0x1502E3D0 */
	MFB_REG_A_CRSP_CROP_Y                           MFB_A_CRSP_CROP_Y;                               /* 03D4, 0x1502E3D4 */
	UINT32                                          rsv_03D8[10];                                    /* 03D8..03FF, 1502E3D8..1502E3FF */
}mfb_reg_t;

#endif
