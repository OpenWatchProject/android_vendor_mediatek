#ifndef _MFB_REG_H_
#define _MFB_REG_H_

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */

typedef volatile union _MFB_REG_CON_
{
		volatile struct	/* 0x15010200 */
		{
				FIELD  MFB_BLD_MODE              : 2;
				FIELD  MFB_BLD_OUT_VALID         : 1;
				FIELD  rsv_3                     : 3;
				FIELD  MFB_BLD_MBD_WT_EN         : 1;
				FIELD  MFB_BLD_SR_WT_EN          : 1;
				FIELD  rsv_8                     : 8;
				FIELD  MFB_BLD_BYPASS            : 1;
				FIELD  rsv_17                    : 15;
		} Bits;
		UINT32 Raw;
}MFB_REG_CON;	/* MFB_CON */

typedef volatile union _MFB_REG_LL_CON1_
{
		volatile struct	/* 0x15010204 */
		{
				FIELD  MFB_BLD_LL_FLT_MODE       : 2;
				FIELD  rsv_2                     : 2;
				FIELD  MFB_BLD_LL_FLT_WT_MODE1   : 3;
				FIELD  rsv_7                     : 1;
				FIELD  MFB_BLD_LL_FLT_WT_MODE2   : 2;
				FIELD  rsv_10                    : 6;
				FIELD  MFB_BLD_LL_CLIP_TH1       : 8;
				FIELD  MFB_BLD_LL_CLIP_TH2       : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON1;	/* MFB_LL_CON1 */

typedef volatile union _MFB_REG_LL_CON2_
{
		volatile struct	/* 0x15010208 */
		{
				FIELD  rsv_0                     : 8;
				FIELD  MFB_BLD_LL_DT1            : 8;
				FIELD  MFB_BLD_LL_TH1            : 8;
				FIELD  MFB_BLD_LL_TH2            : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON2;	/* MFB_LL_CON2 */

typedef volatile union _MFB_REG_LL_CON3_
{
		volatile struct	/* 0x1501020C */
		{
				FIELD  rsv_0                     : 32;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON3;	/* MFB_LL_CON3 */

typedef volatile union _MFB_REG_LL_CON4_
{
		volatile struct	/* 0x15010210 */
		{
				FIELD  rsv_0                     : 32;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON4;	/* MFB_LL_CON4 */

typedef volatile union _MFB_REG_EDGE_
{
		volatile struct	/* 0x15010214 */
		{
				FIELD  MFB_BLD_TILE_EDGE         : 4;
				FIELD  rsv_4                     : 28;
		} Bits;
		UINT32 Raw;
}MFB_REG_EDGE;	/* MFB_EDGE */

typedef volatile union _MFB_REG_LL_CON5_
{
		volatile struct	/* 0x15010218 */
		{
				FIELD  MFB_BLD_LL_GRAD_R1        : 8;
				FIELD  MFB_BLD_LL_GRAD_R2        : 8;
				FIELD  MFB_BLD_LL_TH3            : 8;
				FIELD  MFB_BLD_LL_TH4            : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON5;	/* MFB_LL_CON5 */

typedef volatile union _MFB_REG_LL_CON6_
{
		volatile struct	/* 0x1501021C */
		{
				FIELD  MFB_BLD_LL_TH5            : 8;
				FIELD  MFB_BLD_LL_TH6            : 8;
				FIELD  MFB_BLD_LL_TH7            : 8;
				FIELD  MFB_BLD_LL_TH8            : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON6;	/* MFB_LL_CON6 */

typedef volatile union _MFB_REG_LL_CON7_
{
		volatile struct	/* 0x15010220 */
		{
				FIELD  MFB_BLD_LL_C1             : 8;
				FIELD  MFB_BLD_LL_C2             : 8;
				FIELD  MFB_BLD_LL_C3             : 8;
				FIELD  rsv_24                    : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON7;	/* MFB_LL_CON7 */

typedef volatile union _MFB_REG_LL_CON8_
{
		volatile struct	/* 0x15010224 */
		{
				FIELD  MFB_BLD_LL_SU1            : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_LL_SU2            : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_LL_SU3            : 6;
				FIELD  rsv_22                    : 10;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON8;	/* MFB_LL_CON8 */

typedef volatile union _MFB_REG_LL_CON9_
{
		volatile struct	/* 0x15010228 */
		{
				FIELD  MFB_BLD_LL_SL1            : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_LL_SL2            : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_LL_SL3            : 6;
				FIELD  rsv_22                    : 10;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON9;	/* MFB_LL_CON9 */

typedef volatile union _MFB_REG_LL_CON10_
{
		volatile struct	/* 0x1501022C */
		{
				FIELD  MFB_BLD_CONF_MAP_EN       : 1;
				FIELD  rsv_1                     : 3;
				FIELD  MFB_BLD_LL_GRAD_EN        : 1;
				FIELD  rsv_5                     : 3;
				FIELD  MFB_BLD_LL_GRAD_ENTH      : 8;
				FIELD  MFB_BLD_LL_MAX_WT_FMT2    : 8;
				FIELD  rsv_24                    : 8;
		} Bits;
		UINT32 Raw;
}MFB_REG_LL_CON10;	/* MFB_LL_CON10 */

typedef volatile union _MFB_REG_MBD_CON0_
{
		volatile struct	/* 0x15010230 */
		{
				FIELD  MFB_BLD_MBD_MAX_WT        : 8;
				FIELD  rsv_8                     : 24;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON0;	/* MFB_MBD_CON0 */

typedef volatile union _MFB_REG_MBD_CON1_
{
		volatile struct	/* 0x15010234 */
		{
				FIELD  MFB_BLD_MBD_YL1_THL_0     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL1_THL_1     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL1_THL_2     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL1_THL_3     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON1;	/* MFB_MBD_CON1 */

typedef volatile union _MFB_REG_MBD_CON2_
{
		volatile struct	/* 0x15010238 */
		{
				FIELD  MFB_BLD_MBD_YL1_THL_4     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL1_THH_0     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL1_THH_1     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL1_THH_2     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON2;	/* MFB_MBD_CON2 */

typedef volatile union _MFB_REG_MBD_CON3_
{
		volatile struct	/* 0x1501023C */
		{
				FIELD  MFB_BLD_MBD_YL1_THH_3     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL1_THH_4     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THL_0     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THL_1     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON3;	/* MFB_MBD_CON3 */

typedef volatile union _MFB_REG_MBD_CON4_
{
		volatile struct	/* 0x15010240 */
		{
				FIELD  MFB_BLD_MBD_YL2_THL_2     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL2_THL_3     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THL_4     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THH_0     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON4;	/* MFB_MBD_CON4 */

typedef volatile union _MFB_REG_MBD_CON5_
{
		volatile struct	/* 0x15010244 */
		{
				FIELD  MFB_BLD_MBD_YL2_THH_1     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL2_THH_2     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THH_3     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL2_THH_4     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON5;	/* MFB_MBD_CON5 */

typedef volatile union _MFB_REG_MBD_CON6_
{
		volatile struct	/* 0x15010248 */
		{
				FIELD  MFB_BLD_MBD_YL3_THL_0     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL3_THL_1     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL3_THL_2     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL3_THL_3     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON6;	/* MFB_MBD_CON6 */

typedef volatile union _MFB_REG_MBD_CON7_
{
		volatile struct	/* 0x1501024C */
		{
				FIELD  MFB_BLD_MBD_YL3_THL_4     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL3_THH_0     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_YL3_THH_1     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_YL3_THH_2     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON7;	/* MFB_MBD_CON7 */

typedef volatile union _MFB_REG_MBD_CON8_
{
		volatile struct	/* 0x15010250 */
		{
				FIELD  MFB_BLD_MBD_YL3_THH_3     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_YL3_THH_4     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THL_0     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THL_1     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON8;	/* MFB_MBD_CON8 */

typedef volatile union _MFB_REG_MBD_CON9_
{
		volatile struct	/* 0x15010254 */
		{
				FIELD  MFB_BLD_MBD_CL1_THL_2     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_CL1_THL_3     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THL_4     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THH_0     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON9;	/* MFB_MBD_CON9 */

typedef volatile union _MFB_REG_MBD_CON10_
{
		volatile struct	/* 0x15010258 */
		{
				FIELD  MFB_BLD_MBD_CL1_THH_1     : 6;
				FIELD  rsv_6                     : 2;
				FIELD  MFB_BLD_MBD_CL1_THH_2     : 6;
				FIELD  rsv_14                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THH_3     : 6;
				FIELD  rsv_22                    : 2;
				FIELD  MFB_BLD_MBD_CL1_THH_4     : 6;
				FIELD  rsv_30                    : 2;
		} Bits;
		UINT32 Raw;
}MFB_REG_MBD_CON10;	/* MFB_MBD_CON10 */

typedef volatile struct _mfb_reg_t_	/* 0x15010200..0x15010258 */
{
	MFB_REG_CON                                     MFB_CON;                                         /* 0000, 0x15010200 */
	MFB_REG_LL_CON1                                 MFB_LL_CON1;                                     /* 0004, 0x15010204 */
	MFB_REG_LL_CON2                                 MFB_LL_CON2;                                     /* 0008, 0x15010208 */
	MFB_REG_LL_CON3                                 MFB_LL_CON3;                                     /* 000C, 0x1501020C */
	MFB_REG_LL_CON4                                 MFB_LL_CON4;                                     /* 0010, 0x15010210 */
	MFB_REG_EDGE                                    MFB_EDGE;                                        /* 0014, 0x15010214 */
	MFB_REG_LL_CON5                                 MFB_LL_CON5;                                     /* 0018, 0x15010218 */
	MFB_REG_LL_CON6                                 MFB_LL_CON6;                                     /* 001C, 0x1501021C */
	MFB_REG_LL_CON7                                 MFB_LL_CON7;                                     /* 0020, 0x15010220 */
	MFB_REG_LL_CON8                                 MFB_LL_CON8;                                     /* 0024, 0x15010224 */
	MFB_REG_LL_CON9                                 MFB_LL_CON9;                                     /* 0028, 0x15010228 */
	MFB_REG_LL_CON10                                MFB_LL_CON10;                                    /* 002C, 0x1501022C */
	MFB_REG_MBD_CON0                                MFB_MBD_CON0;                                    /* 0030, 0x15010230 */
	MFB_REG_MBD_CON1                                MFB_MBD_CON1;                                    /* 0034, 0x15010234 */
	MFB_REG_MBD_CON2                                MFB_MBD_CON2;                                    /* 0038, 0x15010238 */
	MFB_REG_MBD_CON3                                MFB_MBD_CON3;                                    /* 003C, 0x1501023C */
	MFB_REG_MBD_CON4                                MFB_MBD_CON4;                                    /* 0040, 0x15010240 */
	MFB_REG_MBD_CON5                                MFB_MBD_CON5;                                    /* 0044, 0x15010244 */
	MFB_REG_MBD_CON6                                MFB_MBD_CON6;                                    /* 0048, 0x15010248 */
	MFB_REG_MBD_CON7                                MFB_MBD_CON7;                                    /* 004C, 0x1501024C */
	MFB_REG_MBD_CON8                                MFB_MBD_CON8;                                    /* 0050, 0x15010250 */
	MFB_REG_MBD_CON9                                MFB_MBD_CON9;                                    /* 0054, 0x15010254 */
	MFB_REG_MBD_CON10                               MFB_MBD_CON10;                                   /* 0058, 0x15010258 */
}mfb_reg_t;

/* auto insert ralf auto gen above */

#endif // _MFB_REG_H_
