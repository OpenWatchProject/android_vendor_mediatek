#ifndef _ISP_REG_H_
#define _ISP_REG_H_

//include for reg size
//#include "camera_isp.h"
#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)
#define ISP_BASE_OFFSET 0x4000
//#define ISP_BASE_HW     0x15004000
#define ISP_BASE_HW     0x15000000
//#define ISP_BASE_RANGE  0x10000
#define ISP_BASE_RANGE  0xA000
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

typedef volatile union _CAM_REG_CTL_START_
{
		volatile struct	/* 0x15004000 */
		{
				FIELD  PASS2_START                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PASS2B_START                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  PASS2C_START                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ0_START                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0B_START                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC_PREFETCH_TRIG                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_PREFETCH_TRIG_D                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  CQ0_D_START                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ0B_D_START                          :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_START;	/* CAM_CTL_START */

typedef volatile union _CAM_REG_CTL_EN_P1_
{
		volatile struct	/* 0x15004004 */
		{
				FIELD  TG1_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_EN                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  OB_EN                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BNR_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LSC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RPG_EN                                :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RRZ_EN                                :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RMX_EN                                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAKG_EN                               :  1;		/* 10..10, 0x00000400 */
				FIELD  BMX_EN                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PAK_EN                                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  SGG1_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  AF_EN                                 :  1;		/* 16..16, 0x00010000 */
				FIELD  FLK_EN                                :  1;		/* 17..17, 0x00020000 */
				FIELD  HBIN2_EN                              :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_EN                                 :  1;		/* 19..19, 0x00080000 */
				FIELD  SGG2_EN                               :  1;		/* 20..20, 0x00100000 */
				FIELD  EIS_EN                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RMG_EN                                :  1;		/* 22..22, 0x00400000 */
				FIELD  CPG_EN                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DBS_EN                                :  1;		/* 24..24, 0x01000000 */
				FIELD  RLB_EN                                :  1;		/* 25..25, 0x02000000 */
				FIELD  HBIN1_EN                              :  1;		/* 26..26, 0x04000000 */
				FIELD  SGG3_EN                               :  1;		/* 27..27, 0x08000000 */
				FIELD  SGG5_EN                               :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P1;	/* CAM_CTL_EN_P1 */

typedef volatile union _CAM_REG_CTL_EN_P1_DMA_
{
		volatile struct	/* 0x15004008 */
		{
				FIELD  IMGO_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ESFKO_EN                              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_EN                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_EN                               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P1_DMA;	/* CAM_CTL_EN_P1_DMA */

typedef volatile union _CAM_REG_CTL_EN_P1_D_
{
		volatile struct	/* 0x15004010 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_D_EN                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  OB_D_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BNR_D_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LSC_D_EN                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RPG_D_EN                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RRZ_D_EN                              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RMX_D_EN                              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  BMX_D_EN                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  2;		/* 20..21, 0x00300000 */
				FIELD  RMG_D_EN                              :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DBS_D_EN                              :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P1_D;	/* CAM_CTL_EN_P1_D */

typedef volatile union _CAM_REG_CTL_EN_P1_DMA_D_
{
		volatile struct	/* 0x15004014 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_D_EN                              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_D_EN                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_D_EN                             :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P1_DMA_D;	/* CAM_CTL_EN_P1_DMA_D */

typedef volatile union _CAM_REG_CTL_EN_P2_
{
		volatile struct	/* 0x15004018 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PGN_EN                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SL2_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CFA_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CCL_EN                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  G2G_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  GGM_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  C24_EN                                :  1;		/*  8.. 8, 0x00000100 */
				FIELD  G2C_EN                                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  C42_EN                                :  1;		/* 10..10, 0x00000400 */
				FIELD  NBC_EN                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PCA_EN                                :  1;		/* 12..12, 0x00001000 */
				FIELD  SEEE_EN                               :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  SL2C_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  CRZ_EN                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  rsv_24                                :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_25                                :  1;		/* 26..26, 0x04000000 */
				FIELD  C24B_EN                               :  1;		/* 27..27, 0x08000000 */
				FIELD  MDPCROP_EN                            :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  UNP_EN                                :  1;		/* 30..30, 0x40000000 */
				FIELD  NSL2A_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P2;	/* CAM_CTL_EN_P2 */

typedef volatile union _CAM_REG_CTL_EN_P2_DMA_
{
		volatile struct	/* 0x1500401C */
		{
				FIELD  IMGI_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_EN                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN_P2_DMA;	/* CAM_CTL_EN_P2_DMA */

typedef volatile union _CAM_REG_CTL_CQ_EN_
{
		volatile struct	/* 0x15004020 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ0A_MASK_CQ0B_D_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ0A_MASK_CQ0C_D_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ0_CONT_D                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ0B_CONT_D                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ0C_IMGO_D_SEL                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0B_D_SEL                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_D_MODE                            :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ0C_D_RRZO_SEL                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ0B_D_MODE                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ0C_D_EN                             :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ0B_D_EN                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  3;		/* 12..14, 0x00007000 */
				FIELD  CQ0_D_EN                              :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_APB_2T                             :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ0A_MASK_CQ0B_EN                     :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ0A_MASK_CQ0C_EN                     :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ0_CONT                              :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ0B_CONT                             :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ0C_IMGO_SEL                         :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ0B_SEL                              :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ0_MODE                              :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ0C_RRZO_SEL                         :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ0B_MODE                             :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ0C_EN                               :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ0B_EN                               :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ0_EN                                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ_EN;	/* CAM_CTL_CQ_EN */

typedef volatile union _CAM_REG_CTL_SCENARIO_
{
		volatile struct	/* 0x15004024 */
		{
				FIELD  SCENARIO                              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SUB_MODE                              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SCENARIO;	/* CAM_CTL_SCENARIO */

typedef volatile union _CAM_REG_CTL_FMT_SEL_P1_
{
		volatile struct	/* 0x15004028 */
		{
				FIELD  PIX_ID                                :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  PAKG_FMT                              :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  6;		/*  6..11, 0x00000FC0 */
				FIELD  OUT_FMT                               :  4;		/* 12..15, 0x0000F000 */
				FIELD  TG1_FMT                               :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  4;		/* 19..22, 0x00780000 */
				FIELD  TWO_PIX_BMX                           :  1;		/* 23..23, 0x00800000 */
				FIELD  TWO_PIX                               :  1;		/* 24..24, 0x01000000 */
				FIELD  TWO_PIX_RMX                           :  1;		/* 25..25, 0x02000000 */
				FIELD  TWO_PIX_DMX                           :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  TG1_SW                                :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_FMT_SEL_P1;	/* CAM_CTL_FMT_SEL_P1 */

typedef volatile union _CAM_REG_CTL_FMT_SEL_P1_D_
{
		volatile struct	/* 0x1500402C */
		{
				FIELD  PIX_ID_D                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  PAKG_FMT_D                            :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  6;		/*  6..11, 0x00000FC0 */
				FIELD  OUT_FMT_D                             :  4;		/* 12..15, 0x0000F000 */
				FIELD  TG1_FMT_D                             :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  4;		/* 19..22, 0x00780000 */
				FIELD  TWO_PIX_BMX_D                         :  1;		/* 23..23, 0x00800000 */
				FIELD  TWO_PIX_D                             :  1;		/* 24..24, 0x01000000 */
				FIELD  TWO_PIX_RMX_D                         :  1;		/* 25..25, 0x02000000 */
				FIELD  TWO_PIX_DMX_D                         :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  TG1_SW_D                              :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_FMT_SEL_P1_D;	/* CAM_CTL_FMT_SEL_P1_D */

typedef volatile union _CAM_REG_CTL_FMT_SEL_P2_
{
		volatile struct	/* 0x15004030 */
		{
				FIELD  IN_FMT                                :  4;		/*  0.. 3, 0x0000000F */
				FIELD  IMG3O_FMT                             :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  VIPI_FMT                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  UFDI_FMT                              :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  1;		/* 13..13, 0x00002000 */
				FIELD  PIX_ID_P2                             :  2;		/* 14..15, 0x0000C000 */
				FIELD  FG_MODE                               :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_FMT_SEL_P2;	/* CAM_CTL_FMT_SEL_P2 */

typedef volatile union _CAM_REG_CTL_SEL_P1_
{
		volatile struct	/* 0x15004034 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SGG_SEL                               :  2;		/*  1.. 2, 0x00000006 */
				FIELD  SGG_SEL_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCS_SEL                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCS_SEL_EN                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  IMG_SEL                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  UFE_SEL                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EIS_SEL                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  W2G_SEL                               :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL_P1;	/* CAM_CTL_SEL_P1 */

typedef volatile union _CAM_REG_CTL_SEL_P1_D_
{
		volatile struct	/* 0x15004038 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SGG_SEL_D                             :  2;		/*  1.. 2, 0x00000006 */
				FIELD  SGG_SEL_EN_D                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCS_SEL_D                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCS_SEL_EN_D                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  IMG_SEL_D                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  UFE_SEL_D                             :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EIS_SEL_D                             :  2;		/* 10..11, 0x00000C00 */
				FIELD  W2G_SEL_D                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL_P1_D;	/* CAM_CTL_SEL_P1_D */

typedef volatile union _CAM_REG_CTL_SEL_P2_
{
		volatile struct	/* 0x1500403C */
		{
				FIELD  CCL_SEL                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CCL_SEL_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  G2G_SEL                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2G_SEL_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UFD_SEL                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  C24_SEL                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SRZ1_SEL                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MIX1_SEL                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CRZ_SEL                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  NR3D_SEL                              :  1;		/* 10..10, 0x00000400 */
				FIELD  MDP_SEL                               :  1;		/* 11..11, 0x00000800 */
				FIELD  FE_SEL                                :  1;		/* 12..12, 0x00001000 */
				FIELD  PCA_SEL                               :  1;		/* 13..13, 0x00002000 */
				FIELD  MFB_MODE                              :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL_P2;	/* CAM_CTL_SEL_P2 */

typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_
{
		volatile struct	/* 0x15004040 */
		{
				FIELD  PASS1_DB_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PASS1_D_DB_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  APB_CLK_GATE_BYPASS                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  DUAL_MODE                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BIN_SEL                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BIN_SEL_EN                            :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PREGAIN_SEL                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  PREGAIN_SEL_EN                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSC_EARLY_SOF_EN                      :  1;		/* 10..10, 0x00000400 */
				FIELD  W2G_MODE                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  INT_MRG                               :  1;		/* 14..14, 0x00004000 */
				FIELD  BIN_SEL_D                             :  1;		/* 15..15, 0x00008000 */
				FIELD  BIN_SEL_EN_D                          :  1;		/* 16..16, 0x00010000 */
				FIELD  PREGAIN_SEL_D                         :  1;		/* 17..17, 0x00020000 */
				FIELD  PREGAIN_SEL_EN_D                      :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC_EARLY_SOF_EN_D                    :  1;		/* 19..19, 0x00080000 */
				FIELD  W2G_MODE_D                            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL_GLOBAL;	/* CAM_CTL_SEL_GLOBAL */

typedef volatile union _CAM_REG_CTL_INT_P1_EN_
{
		volatile struct	/* 0x15004048 */
		{
				FIELD  VS1_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_EN                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_EN                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_EN                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_DON_EN                             :  1;		/* 16..16, 0x00010000 */
				FIELD  FLK_DON_EN                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  IMGO_ERR_EN                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_EN                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_EN                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  ESFKO_ERR_EN                          :  1;		/* 24..24, 0x01000000 */
				FIELD  FLK_ERR_EN                            :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_EN                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_EN                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_EN                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_EN;	/* CAM_CTL_INT_P1_EN */

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS_
{
		volatile struct	/* 0x1500404C */
		{
				FIELD  VS1_INT_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_ST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_ST                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_DON_ST                             :  1;		/* 16..16, 0x00010000 */
				FIELD  FLK_DON_ST                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  ESFKO_ERR_ST                          :  1;		/* 24..24, 0x01000000 */
				FIELD  FLK_ERR_ST                            :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_ST                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_ST                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_STATUS;	/* CAM_CTL_INT_P1_STATUS */

typedef volatile union _CAM_REG_CTL_INT_P1_EN2_
{
		volatile struct	/* 0x15004050 */
		{
				FIELD  IMGO_DONE_ST_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST_EN                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ESFKO_DONE_ST_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DONE_ST_EN                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DONE_ST_EN                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_DONE_ST_EN                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_DONE_ST_EN                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AF_TAR_DONE_ST_EN                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  AF_FLO1_DONE_ST_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AF_FLO2_DONE_ST_EN                    :  1;		/* 10..10, 0x00000400 */
				FIELD  AF_FLO3_DONE_ST_EN                    :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ0_DONE_ST_EN                        :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ0B_DONE_ST_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ0C_DONE_ST_EN                       :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_EN2;	/* CAM_CTL_INT_P1_EN2 */

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS2_
{
		volatile struct	/* 0x15004054 */
		{
				FIELD  IMGO_DONE_ST                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ESFKO_DONE_ST                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DONE_ST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DONE_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_DONE_ST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_DONE_ST                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AF_TAR_DONE_ST                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  AF_FLO1_DONE_ST                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AF_FLO2_DONE_ST                       :  1;		/* 10..10, 0x00000400 */
				FIELD  AF_FLO3_DONE_ST                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ0_DONE_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ0B_DONE_ST                          :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ0C_DONE_ST                          :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_STATUS2;	/* CAM_CTL_INT_P1_STATUS2 */

typedef volatile union _CAM_REG_CTL_INT_P1_EN_D_
{
		volatile struct	/* 0x15004058 */
		{
				FIELD  VS1_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_EN                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_EN                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_EN                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_DON_EN                             :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  IMGO_ERR_EN                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_EN                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_EN                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  AFO_ERR_EN                            :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_EN                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_EN                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_EN                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_EN_D;	/* CAM_CTL_INT_P1_EN_D */

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS_D_
{
		volatile struct	/* 0x1500405C */
		{
				FIELD  VS1_INT_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_ST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_ST                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_DON_ST                             :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  AFO_ERR_ST                            :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_ST                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_ST                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_STATUS_D;	/* CAM_CTL_INT_P1_STATUS_D */

typedef volatile union _CAM_REG_CTL_INT_P1_EN2_D_
{
		volatile struct	/* 0x15004060 */
		{
				FIELD  IMGO_D_DONE_ST_EN                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_D_DONE_ST_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_D_DONE_ST_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_D_DONE_ST_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_D_DONE_ST_EN                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_D_DONE_ST_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_D_DONE_ST_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AF_TAR_DONE_EN                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  AF_FLO1_DONE_EN                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AF_FLO2_DONE_EN                       :  1;		/* 10..10, 0x00000400 */
				FIELD  AF_FLO3_DONE_EN                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ0_DONE_ST_EN                        :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ0B_DONE_ST_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ0C_DONE_ST_EN                       :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_EN2_D;	/* CAM_CTL_INT_P1_EN2_D */

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS2_D_
{
		volatile struct	/* 0x15004064 */
		{
				FIELD  IMGO_D_DONE_ST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_D_DONE_ST                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_D_DONE_ST                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_D_DONE_ST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_D_DONE_ST                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_D_DONE_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_D_DONE_ST                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AF_TAR_DONE_ST                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  AF_FLO1_DONE_ST                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AF_FLO2_DONE_ST                       :  1;		/* 10..10, 0x00000400 */
				FIELD  AF_FLO3_DONE_ST                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ0_DONE_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ0B_DONE_ST                          :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ0C_DONE_ST                          :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P1_STATUS2_D;	/* CAM_CTL_INT_P1_STATUS2_D */

typedef volatile union _CAM_REG_CTL_INT_P2_EN_
{
		volatile struct	/* 0x15004068 */
		{
				FIELD  CQ_ERR_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PASS2_DON_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TILE_DON_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_DON_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TDR_ERR_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  PASS2A_DONE_ST_EN                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PASS2B_DONE_ST_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PASS2C_DONE_ST_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ1_DONE_ST_EN                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ2_DONE_ST_EN                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ3_DONE_ST_EN                        :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS2A_ERR_TRIG_ST_EN                 :  1;		/* 11..11, 0x00000800 */
				FIELD  PASS2B_ERR_TRIG_ST_EN                 :  1;		/* 12..12, 0x00001000 */
				FIELD  PASS2C_ERR_TRIG_ST_EN                 :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  IMGI_DONE_ST_EN                       :  1;		/* 15..15, 0x00008000 */
				FIELD  UFDI_DONE_ST_EN                       :  1;		/* 16..16, 0x00010000 */
				FIELD  VIPI_DONE_ST_EN                       :  1;		/* 17..17, 0x00020000 */
				FIELD  VIP2I_DONE_ST_EN                      :  1;		/* 18..18, 0x00040000 */
				FIELD  VIP3I_DONE_ST_EN                      :  1;		/* 19..19, 0x00080000 */
				FIELD  LCEI_DONE_ST_EN                       :  1;		/* 20..20, 0x00100000 */
				FIELD  MFBO_DONE_ST_EN                       :  1;		/* 21..21, 0x00200000 */
				FIELD  IMG2O_DONE_ST_EN                      :  1;		/* 22..22, 0x00400000 */
				FIELD  IMG3O_DONE_ST_EN                      :  1;		/* 23..23, 0x00800000 */
				FIELD  IMG3BO_DONE_ST_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  IMG3CO_DONE_ST_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FEO_DONE_ST_EN                        :  1;		/* 26..26, 0x04000000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM_COLLISION_ST_EN                   :  1;		/* 28..28, 0x10000000 */
				FIELD  PCA_COLLISION_ST_EN                   :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P2_EN;	/* CAM_CTL_INT_P2_EN */

typedef volatile union _CAM_REG_CTL_INT_P2_STATUS_
{
		volatile struct	/* 0x1500406C */
		{
				FIELD  CQ_ERR_ST                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PASS2_DON_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TILE_DON_ST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_DON_ST                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TDR_ERR_ST                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  PASS2A_DONE_ST                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PASS2B_DONE_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PASS2C_DONE_ST                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ1_DONE_ST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ2_DONE_ST                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ3_DONE_ST                           :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS2A_ERR_TRIG_ST                    :  1;		/* 11..11, 0x00000800 */
				FIELD  PASS2B_ERR_TRIG_ST                    :  1;		/* 12..12, 0x00001000 */
				FIELD  PASS2C_ERR_TRIG_ST                    :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  IMGI_DONE_ST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  UFDI_DONE_ST                          :  1;		/* 16..16, 0x00010000 */
				FIELD  VIPI_DONE_ST                          :  1;		/* 17..17, 0x00020000 */
				FIELD  VIP2I_DONE_ST                         :  1;		/* 18..18, 0x00040000 */
				FIELD  VIP3I_DONE_ST                         :  1;		/* 19..19, 0x00080000 */
				FIELD  LCEI_DONE_ST                          :  1;		/* 20..20, 0x00100000 */
				FIELD  MFBO_DONE_ST                          :  1;		/* 21..21, 0x00200000 */
				FIELD  IMG2O_DONE_ST                         :  1;		/* 22..22, 0x00400000 */
				FIELD  IMG3BO_DONE_ST                        :  1;		/* 23..23, 0x00800000 */
				FIELD  IMG3O_DONE_ST                         :  1;		/* 24..24, 0x01000000 */
				FIELD  IMG3CO_DONE_ST                        :  1;		/* 25..25, 0x02000000 */
				FIELD  FEO_DONE_ST                           :  1;		/* 26..26, 0x04000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM_COLLISION_ST                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PCA_COLLISION_ST                      :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_P2_STATUS;	/* CAM_CTL_INT_P2_STATUS */

typedef volatile union _CAM_REG_CTL_INT_STATUSX_
{
		volatile struct	/* 0x15004070 */
		{
				FIELD  VS1_INT_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_ST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_ST                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_ERR_ST                             :  1;		/* 13..13, 0x00002000 */
				FIELD  PASS2_DON_ST                          :  1;		/* 14..14, 0x00004000 */
				FIELD  TILE_DON_ST                           :  1;		/* 15..15, 0x00008000 */
				FIELD  AF_DON_ST                             :  1;		/* 16..16, 0x00010000 */
				FIELD  FLK_DON_ST                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_DON_ST                             :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  ESFKO_ERR_ST                          :  1;		/* 24..24, 0x01000000 */
				FIELD  FLK_ERR_ST                            :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_ST                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_ST                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_STATUSX;	/* CAM_CTL_INT_STATUSX */

typedef volatile union _CAM_REG_CTL_INT_STATUS2X_
{
		volatile struct	/* 0x15004074 */
		{
				FIELD  VS1_INT_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG1_INT1_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG1_INT2_ST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_ERR_ST                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG1_GBERR                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ0_ERR                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ0_VS_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF1_INT_ST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_DON_ST                             :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  AFO_ERR_ST                            :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  FBC_RRZO_DON_ST                       :  1;		/* 27..27, 0x08000000 */
				FIELD  FBC_IMGO_DON_ST                       :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_STATUS2X;	/* CAM_CTL_INT_STATUS2X */

typedef volatile union _CAM_REG_CTL_INT_STATUS3X_
{
		volatile struct	/* 0x15004078 */
		{
				FIELD  IMGO_DONE_ST                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ESFKO_DONE_ST                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DONE_ST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DONE_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_DONE_ST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_DONE_ST                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMGO_D_DONE_ST                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RRZO_D_DONE_ST                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AFO_D_DONE_ST                         :  1;		/* 10..10, 0x00000400 */
				FIELD  LCSO_D_DONE_ST                        :  1;		/* 11..11, 0x00000800 */
				FIELD  AAO_D_DONE_ST                         :  1;		/* 12..12, 0x00001000 */
				FIELD  BPCI_D_DONE_ST                        :  1;		/* 13..13, 0x00002000 */
				FIELD  LSCI_D_DONE_ST                        :  1;		/* 14..14, 0x00004000 */
				FIELD  IMGI_DONE_ST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  UFDI_DONE_ST                          :  1;		/* 16..16, 0x00010000 */
				FIELD  VIPI_DONE_ST                          :  1;		/* 17..17, 0x00020000 */
				FIELD  VIP2I_DONE_ST                         :  1;		/* 18..18, 0x00040000 */
				FIELD  VIP3I_DONE_ST                         :  1;		/* 19..19, 0x00080000 */
				FIELD  LCEI_DONE_ST                          :  1;		/* 20..20, 0x00100000 */
				FIELD  MFBO_DONE_ST                          :  1;		/* 21..21, 0x00200000 */
				FIELD  IMG2O_DONE_ST                         :  1;		/* 22..22, 0x00400000 */
				FIELD  IMG3O_DONE_ST                         :  1;		/* 23..23, 0x00800000 */
				FIELD  IMG3BO_DONE_ST                        :  1;		/* 24..24, 0x01000000 */
				FIELD  IMG3CO_DONE_ST                        :  1;		/* 25..25, 0x02000000 */
				FIELD  FEO_DONE_ST                           :  1;		/* 26..26, 0x04000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM_COLLISION_ST                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PCA_COLLISION_ST                      :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_INT_STATUS3X;	/* CAM_CTL_INT_STATUS3X */

typedef volatile union _CAM_REG_CTL_TILE_
{
		volatile struct	/* 0x1500407C */
		{
				FIELD  TILE_EDGE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  TILE_IRQ                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LAST_TILE                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NSL2A_TCM_EN                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  C24B_TCM_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CRZ_EDGE                              :  4;		/*  8..11, 0x00000F00 */
				FIELD  LCEI_TCM_EN                           :  1;		/* 12..12, 0x00001000 */
				FIELD  UFDI_TCM_EN                           :  1;		/* 13..13, 0x00002000 */
				FIELD  VIPI_TCM_EN                           :  1;		/* 14..14, 0x00004000 */
				FIELD  VIP2I_TCM_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  VIP3I_TCM_EN                          :  1;		/* 16..16, 0x00010000 */
				FIELD  FEO_TCM_EN                            :  1;		/* 17..17, 0x00020000 */
				FIELD  IMG2O_TCM_EN                          :  1;		/* 18..18, 0x00040000 */
				FIELD  IMG3O_TCM_EN                          :  1;		/* 19..19, 0x00080000 */
				FIELD  IMG3BO_TCM_EN                         :  1;		/* 20..20, 0x00100000 */
				FIELD  IMG3CO_TCM_EN                         :  1;		/* 21..21, 0x00200000 */
				FIELD  FE_TCM_EN                             :  1;		/* 22..22, 0x00400000 */
				FIELD  CRZ_TCM_EN                            :  1;		/* 23..23, 0x00800000 */
				FIELD  MDPCROP_TCM_EN                        :  1;		/* 24..24, 0x01000000 */
				FIELD  LCE_TCM_EN                            :  1;		/* 25..25, 0x02000000 */
				FIELD  SRZ1_TCM_EN                           :  1;		/* 26..26, 0x04000000 */
				FIELD  SRZ2_TCM_EN                           :  1;		/* 27..27, 0x08000000 */
				FIELD  CRSP_TCM_EN                           :  1;		/* 28..28, 0x10000000 */
				FIELD  NR3D_TCM_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  CTL_EXTENSION_EN                      :  1;		/* 30..30, 0x40000000 */
				FIELD  MIX3_TCM_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TILE;	/* CAM_CTL_TILE */

typedef volatile union _CAM_REG_CTL_TDR_EN_STATUS_
{
		volatile struct	/* 0x15004080 */
		{
				FIELD  CTL_TDR_EN_STATUS                     : 31;		/*  0..30, 0x7FFFFFFF */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TDR_EN_STATUS;	/* CAM_CTL_TDR_EN_STATUS */

typedef volatile union _CAM_REG_CTL_TCM_EN_
{
		volatile struct	/* 0x15004084 */
		{
				FIELD  TILE_WIDTH                            : 10;		/*  0.. 9, 0x000003FF */
				FIELD  TILE_HEIGHT                           : 13;		/* 10..22, 0x007FFC00 */
				FIELD  SEEE_TCM_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  SL2C_TCM_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  MIX2_TCM_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  PCA_TCM_EN                            :  1;		/* 26..26, 0x04000000 */
				FIELD  MIX1_TCM_EN                           :  1;		/* 27..27, 0x08000000 */
				FIELD  MFBO_TCM_EN                           :  1;		/* 28..28, 0x10000000 */
				FIELD  TDR_SOF_RST_EN                        :  1;		/* 29..29, 0x20000000 */
				FIELD  TDR_SZ_DET                            :  1;		/* 30..30, 0x40000000 */
				FIELD  TDR_EN                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TCM_EN;	/* CAM_CTL_TCM_EN */

typedef volatile union _CAM_REG_CTL_TDR_DBG_STATUS_
{
		volatile struct	/* 0x15004088 */
		{
				FIELD  TPIPE_CNT                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LOAD_SIZE                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TDR_DBG_STATUS;	/* CAM_CTL_TDR_DBG_STATUS */

typedef volatile union _CAM_REG_CTL_SW_CTL_
{
		volatile struct	/* 0x1500408C */
		{
				FIELD  SW_RST_Trig                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SW_RST_ST                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  HW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SW_RST_P2_Trig                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SW_RST_P2_ST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HW_RST_P2                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SW_CTL;	/* CAM_CTL_SW_CTL */

typedef volatile union _CAM_REG_CTL_SPARE0_
{
		volatile struct	/* 0x15004090 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE0;	/* CAM_CTL_SPARE0 */

typedef volatile union _CAM_REG_CTL_SPARE1_
{
		volatile struct	/* 0x15004094 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE1;	/* CAM_CTL_SPARE1 */

typedef volatile union _CAM_REG_CTL_SPARE2_
{
		volatile struct	/* 0x1500409C */
		{
				FIELD  SPARE2                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE2;	/* CAM_CTL_SPARE2 */

typedef volatile union _CAM_REG_CTL_CQ1_BASEADDR_
{
		volatile struct	/* 0x150040A0 */
		{
				FIELD  CQ1_BASEADDR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ1_BASEADDR;	/* CAM_CTL_CQ1_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ2_BASEADDR_
{
		volatile struct	/* 0x150040A4 */
		{
				FIELD  CQ2_BASEADDR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ2_BASEADDR;	/* CAM_CTL_CQ2_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ3_BASEADDR_
{
		volatile struct	/* 0x150040A8 */
		{
				FIELD  CQ3_BASEADDR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ3_BASEADDR;	/* CAM_CTL_CQ3_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0_BASEADDR_
{
		volatile struct	/* 0x150040AC */
		{
				FIELD  CQ0_BASEADDR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0_BASEADDR;	/* CAM_CTL_CQ0_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0B_BASEADDR_
{
		volatile struct	/* 0x150040B0 */
		{
				FIELD  CQ0B_BASEADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0B_BASEADDR;	/* CAM_CTL_CQ0B_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0C_BASEADDR_
{
		volatile struct	/* 0x150040B4 */
		{
				FIELD  CQ0C_BASEADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0C_BASEADDR;	/* CAM_CTL_CQ0C_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0_BASEADDR_
{
		volatile struct	/* 0x150040B8 */
		{
				FIELD  CUR_CQ0_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0_BASEADDR;	/* CAM_CTL_CUR_CQ0_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0B_BASEADDR_
{
		volatile struct	/* 0x150040BC */
		{
				FIELD  CUR_CQ0B_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0B_BASEADDR;	/* CAM_CTL_CUR_CQ0B_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0C_BASEADDR_
{
		volatile struct	/* 0x150040C0 */
		{
				FIELD  CTL_CUR_CQ0C_BASEADDR                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0C_BASEADDR;	/* CAM_CTL_CUR_CQ0C_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0_D_BASEADDR_
{
		volatile struct	/* 0x150040C4 */
		{
				FIELD  CTL_CQ0_D_BASEADDR                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0_D_BASEADDR;	/* CAM_CTL_CQ0_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0B_D_BASEADDR_
{
		volatile struct	/* 0x150040C8 */
		{
				FIELD  CTL_CQ0B_D_BASEADDR                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0B_D_BASEADDR;	/* CAM_CTL_CQ0B_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_CQ0C_D_BASEADDR_
{
		volatile struct	/* 0x150040CC */
		{
				FIELD  CTL_CQ0C_D_BASEADDR                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ0C_D_BASEADDR;	/* CAM_CTL_CQ0C_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0_D_BASEADDR_
{
		volatile struct	/* 0x150040D0 */
		{
				FIELD  CTL_CUR_CQ0_D_BASEADDR                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0_D_BASEADDR;	/* CAM_CTL_CUR_CQ0_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0B_D_BASEADDR_
{
		volatile struct	/* 0x150040D4 */
		{
				FIELD  CTL_CUR_CQ0B_D_BASEADDR               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0B_D_BASEADDR;	/* CAM_CTL_CUR_CQ0B_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_CUR_CQ0C_D_BASEADDR_
{
		volatile struct	/* 0x150040D8 */
		{
				FIELD  CTL_CUR_CQ0C_D_BASEADDR               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CUR_CQ0C_D_BASEADDR;	/* CAM_CTL_CUR_CQ0C_D_BASEADDR */

typedef volatile union _CAM_REG_CTL_DB_LOAD_D_
{
		volatile struct	/* 0x150040DC */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DB_LOAD_HOLD_D                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DB_LOAD_D;	/* CAM_CTL_DB_LOAD_D */

typedef volatile union _CAM_REG_CTL_DB_LOAD_
{
		volatile struct	/* 0x150040E0 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DB_LOAD_HOLD                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DB_LOAD;	/* CAM_CTL_DB_LOAD */

typedef volatile union _CAM_REG_CTL_P1_DONE_BYP_
{
		volatile struct	/* 0x150040E4 */
		{
				FIELD  IMGO_DONE_BYP                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_BYP                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_BYP                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ESFKO_DONE_BYP                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DONE_BYP                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DONE_BYP                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_DONE_BYP                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_DONE_BYP                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  TG1_DONE_BYP                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_P1_DONE_BYP;	/* CAM_CTL_P1_DONE_BYP */

typedef volatile union _CAM_REG_CTL_P1_DONE_BYP_D_
{
		volatile struct	/* 0x150040E8 */
		{
				FIELD  IMGO_DONE_BYP_D                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_BYP_D                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_DONE_BYP_D                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DONE_BYP_D                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DONE_BYP_D                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BPCI_DONE_BYP_D                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSCI_DONE_BYP_D                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  TG1_DONE_BYP_D                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_P1_DONE_BYP_D;	/* CAM_CTL_P1_DONE_BYP_D */

typedef volatile union _CAM_REG_CTL_P2_DONE_BYP_
{
		volatile struct	/* 0x150040EC */
		{
				FIELD  IMGI_DONE_BYP                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFDI_DONE_BYP                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  VIPI_DONE_BYP                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  VIP2I_DONE_BYP                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIP3I_DONE_BYP                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_DONE_BYP                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_DONE_BYP                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_DONE_BYP                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMG3O_DONE_BYP                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMG3BO_DONE_BYP                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  IMG3CO_DONE_BYP                       :  1;		/* 10..10, 0x00000400 */
				FIELD  FEO_DONE_BYP                          :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_P2_DONE_BYP;	/* CAM_CTL_P2_DONE_BYP */

typedef volatile union _CAM_REG_CTL_IMGO_FBC_
{
		volatile struct	/* 0x150040F0 */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IMGO_FBC;	/* CAM_CTL_IMGO_FBC */

typedef volatile union _CAM_REG_CTL_RRZO_FBC_
{
		volatile struct	/* 0x150040F4 */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RRZO_FBC;	/* CAM_CTL_RRZO_FBC */

typedef volatile union _CAM_REG_CTL_IMGO_D_FBC_
{
		volatile struct	/* 0x150040F8 */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IMGO_D_FBC;	/* CAM_CTL_IMGO_D_FBC */

typedef volatile union _CAM_REG_CTL_RRZO_D_FBC_
{
		volatile struct	/* 0x150040FC */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RRZO_D_FBC;	/* CAM_CTL_RRZO_D_FBC */

typedef volatile union _CAM_REG_CTL_SPARE3_
{
		volatile struct	/* 0x15004100 */
		{
				FIELD  SPARE3                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE3;	/* CAM_CTL_SPARE3 */

typedef volatile union _CAM_REG_CTL_IHDR_
{
		volatile struct	/* 0x15004104 */
		{
				FIELD  IHDR_GAIN                             :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  IHDR_MODE                             :  1;		/* 12..12, 0x00001000 */
				FIELD  IHDR_LE_FIRST                         :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IHDR;	/* CAM_CTL_IHDR */

typedef volatile union _CAM_REG_CTL_IHDR_D_
{
		volatile struct	/* 0x15004108 */
		{
				FIELD  IHDR_D_GAIN                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  IHDR_D_MODE                           :  1;		/* 12..12, 0x00001000 */
				FIELD  IHDR_D_LE_FIRST                       :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IHDR_D;	/* CAM_CTL_IHDR_D */

typedef volatile union _CAM_REG_CTL_CQ_EN_P2_
{
		volatile struct	/* 0x1500410C */
		{
				FIELD  CQ1_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ2_EN                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ3_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CQ_EN_P2;	/* CAM_CTL_CQ_EN_P2 */

typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_P2_
{
		volatile struct	/* 0x15004110 */
		{
				FIELD  PASS2_DB_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL_GLOBAL_P2;	/* CAM_CTL_SEL_GLOBAL_P2 */

typedef volatile union _CAM_REG_CTL_DBG_SET_
{
		volatile struct	/* 0x15004160 */
		{
				FIELD  DEBUG_MOD_SEL                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DEBUG_SEL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  DEBUG_TOP_SEL                         :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DBG_SET;	/* CAM_CTL_DBG_SET */

typedef volatile union _CAM_REG_CTL_DBG_PORT_
{
		volatile struct	/* 0x15004164 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DBG_PORT;	/* CAM_CTL_DBG_PORT */

typedef volatile union _CAM_REG_CTL_IMGI_CHECK_
{
		volatile struct	/* 0x15004168 */
		{
				FIELD  CTL_IMGI_CHECK                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IMGI_CHECK;	/* CAM_CTL_IMGI_CHECK */

typedef volatile union _CAM_REG_CTL_IMGO_CHECK_
{
		volatile struct	/* 0x1500416C */
		{
				FIELD  CTL_IMGO_CHECK                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IMGO_CHECK;	/* CAM_CTL_IMGO_CHECK */

typedef volatile union _CAM_REG_CTL_CLK_EN_
{
		volatile struct	/* 0x15004170 */
		{
				FIELD  RAW_DP_CK_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RAW_D_DP_CK_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DIP_DP_CK_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DMA_DP_CK_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CLK_EN;	/* CAM_CTL_CLK_EN */

typedef volatile union _CAM_REG_CTL_DATE_CODE_
{
		volatile struct	/* 0x15004180 */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DATE_CODE;	/* CAM_CTL_DATE_CODE */

typedef volatile union _CAM_REG_CTL_PROJ_CODE_
{
		volatile struct	/* 0x15004184 */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_PROJ_CODE;	/* CAM_CTL_PROJ_CODE */

typedef volatile union _CAM_REG_CTL_RAW_DCM_DIS_
{
		volatile struct	/* 0x15004188 */
		{
				FIELD  UFD_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PGN_DCM_DIS                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  UNP_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TG1_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DMX_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_DCM_DIS                            :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_DCM_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_DCM_DIS                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_DCM_DIS                           :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_DCM_DIS                           :  1;		/* 11..11, 0x00000800 */
				FIELD  RMX_DCM_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_DCM_DIS                          :  1;		/* 13..13, 0x00002000 */
				FIELD  BMX_DCM_DIS                           :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_DCM_DIS                           :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_DCM_DIS                           :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_DCM_DIS                           :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_DCM_DIS                          :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_DCM_DIS                            :  1;		/* 19..19, 0x00080000 */
				FIELD  FLK_DCM_DIS                           :  1;		/* 20..20, 0x00100000 */
				FIELD  HBIN2_DCM_DIS                         :  1;		/* 21..21, 0x00200000 */
				FIELD  AA_DCM_DIS                            :  1;		/* 22..22, 0x00400000 */
				FIELD  SGG2_DCM_DIS                          :  1;		/* 23..23, 0x00800000 */
				FIELD  EIS_DCM_DIS                           :  1;		/* 24..24, 0x01000000 */
				FIELD  RMG_DCM_DIS                           :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_DCM_DIS                           :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_DCM_DIS                           :  1;		/* 27..27, 0x08000000 */
				FIELD  HBIN1_DCM_DIS                         :  1;		/* 28..28, 0x10000000 */
				FIELD  SGG3_DCM_DIS                          :  1;		/* 29..29, 0x20000000 */
				FIELD  SGG5_DCM_DIS                          :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_DIS;	/* CAM_CTL_RAW_DCM_DIS */

typedef volatile union _CAM_REG_CTL_RAW_D_DCM_DIS_
{
		volatile struct	/* 0x1500418C */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TG1_D_DCM_DIS                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_D_DCM_DIS                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_D_DCM_DIS                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_D_DCM_DIS                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_D_DCM_DIS                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_D_DCM_DIS                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_D_DCM_DIS                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_D_DCM_DIS                         :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_D_DCM_DIS                        :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_D_DCM_DIS                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_D_DCM_DIS                         :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_D_DCM_DIS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_D_DCM_DIS                          :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  2;		/* 20..21, 0x00300000 */
				FIELD  AA_D_DCM_DIS                          :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  2;		/* 23..24, 0x01800000 */
				FIELD  RMG_D_DCM_DIS                         :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_D_DCM_DIS                         :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_D_DCM_DIS                         :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_D_DCM_DIS;	/* CAM_CTL_RAW_D_DCM_DIS */

typedef volatile union _CAM_REG_CTL_DMA_DCM_DIS_
{
		volatile struct	/* 0x15004190 */
		{
				FIELD  IMGI_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFDI_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  VIPI_DCM_DIS                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  VIP2I_DCM_DIS                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIP3I_DCM_DIS                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_DCM_DIS                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_DCM_DIS                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_DCM_DIS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMG3O_DCM_DIS                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMG3BO_DCM_DIS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  IMG3CO_DCM_DIS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  FEO_DCM_DIS                           :  1;		/* 11..11, 0x00000800 */
				FIELD  IMGO_D_DCM_DIS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_D_DCM_DIS                        :  1;		/* 13..13, 0x00002000 */
				FIELD  AFO_D_DCM_DIS                         :  1;		/* 14..14, 0x00004000 */
				FIELD  LCSO_D_DCM_DIS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_D_DCM_DIS                         :  1;		/* 16..16, 0x00010000 */
				FIELD  BPCI_D_DCM_DIS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_D_DCM_DIS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DCM_DIS                          :  1;		/* 19..19, 0x00080000 */
				FIELD  UFEO_DCM_DIS                          :  1;		/* 20..20, 0x00100000 */
				FIELD  RRZO_DCM_DIS                          :  1;		/* 21..21, 0x00200000 */
				FIELD  ESFKO_DCM_DIS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_DCM_DIS                          :  1;		/* 23..23, 0x00800000 */
				FIELD  AAO_DCM_DIS                           :  1;		/* 24..24, 0x01000000 */
				FIELD  BPCI_DCM_DIS                          :  1;		/* 25..25, 0x02000000 */
				FIELD  LSCI_DCM_DIS                          :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_DIS;	/* CAM_CTL_DMA_DCM_DIS */

typedef volatile union _CAM_REG_CTL_RGB_DCM_DIS_
{
		volatile struct	/* 0x15004194 */
		{
				FIELD  SL2_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CFA_DCM_DIS                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCL_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2G_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C24_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  GGM_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RGB_DCM_DIS;	/* CAM_CTL_RGB_DCM_DIS */

typedef volatile union _CAM_REG_CTL_YUV_DCM_DIS_
{
		volatile struct	/* 0x15004198 */
		{
				FIELD  C02_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  G2C_DCM_DIS                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C42_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NBC_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PCA_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEEE_DCM_DIS                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCE_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NR3D_DCM_DIS                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SL2B_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SL2C_DCM_DIS                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SRZ1_DCM_DIS                          :  1;		/* 10..10, 0x00000400 */
				FIELD  SRZ2_DCM_DIS                          :  1;		/* 11..11, 0x00000800 */
				FIELD  CRZ_DCM_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  MIX1_DCM_DIS                          :  1;		/* 13..13, 0x00002000 */
				FIELD  MIX2_DCM_DIS                          :  1;		/* 14..14, 0x00004000 */
				FIELD  MIX3_DCM_DIS                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CRSP_DCM_DIS                          :  1;		/* 16..16, 0x00010000 */
				FIELD  C24B_DCM_DIS                          :  1;		/* 17..17, 0x00020000 */
				FIELD  MDPCROP_DCM_DIS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  FE_DCM_DIS                            :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_YUV_DCM_DIS;	/* CAM_CTL_YUV_DCM_DIS */

typedef volatile union _CAM_REG_CTL_TOP_DCM_DIS_
{
		volatile struct	/* 0x1500419C */
		{
				FIELD  TOP_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_DCM_DIS                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FBC_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TDR_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_DIS;	/* CAM_CTL_TOP_DCM_DIS */

typedef volatile union _CAM_REG_CTL_RAW_DCM_STATUS_
{
		volatile struct	/* 0x150041A0 */
		{
				FIELD  UFD_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PGN_DCM_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  UNP_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TG1_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DMX_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_DCM_STATUS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_DCM_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_DCM_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_DCM_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_DCM_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  RMX_DCM_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_DCM_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BMX_DCM_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_DCM_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_DCM_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_DCM_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_DCM_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_DCM_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  FLK_DCM_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  HBIN_DCM_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  AA_DCM_STATUS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SGG2_DCM_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  EIS_DCM_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  RMG_DCM_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_DCM_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_DCM_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  HBIN1_DCM_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  SGG3_DCM_STATUS                       :  1;		/* 29..29, 0x20000000 */
				FIELD  SGG5_DCM_STATUS                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_STATUS;	/* CAM_CTL_RAW_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_RAW_D_DCM_STATUS_
{
		volatile struct	/* 0x150041A4 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TG1_D_DCM_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_D_DCM_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_D_DCM_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_D_DCM_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_D_DCM_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_D_DCM_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_D_DCM_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_D_DCM_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_D_DCM_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_D_DCM_STATUS                      :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_D_DCM_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_D_DCM_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_D_DCM_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  2;		/* 20..21, 0x00300000 */
				FIELD  AA_D_DCM_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  2;		/* 23..24, 0x01800000 */
				FIELD  RMG_D_DCM_STATUS                      :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_D_DCM_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_D_DCM_STATUS                      :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_D_DCM_STATUS;	/* CAM_CTL_RAW_D_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_DMA_DCM_STATUS_
{
		volatile struct	/* 0x150041A8 */
		{
				FIELD  IMGI_DCM_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFDI_DCM_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  VIPI_DCM_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  VIP2I_DCM_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIP3I_DCM_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_DCM_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_DCM_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_DCM_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMG3O_DCM_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMG3BO_DCM_STATUS                     :  1;		/*  9.. 9, 0x00000200 */
				FIELD  IMG3CO_DCM_STATUS                     :  1;		/* 10..10, 0x00000400 */
				FIELD  FEO_DCM_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  IMGO_D_DCM_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_D_DCM_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  AFO_D_DCM_STATUS                      :  1;		/* 14..14, 0x00004000 */
				FIELD  LCSO_D_DCM_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_D_DCM_STATUS                      :  1;		/* 16..16, 0x00010000 */
				FIELD  BPCI_D_DCM_STATUS                     :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_D_DCM_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DCM_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  UFEO_DCM_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RRZO_DCM_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  ESFKO_DCM_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_DCM_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  AAO_DCM_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  BPCI_DCM_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  LSCI_DCM_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_STATUS;	/* CAM_CTL_DMA_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_RGB_DCM_STATUS_
{
		volatile struct	/* 0x150041AC */
		{
				FIELD  SL2_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CFA_DCM_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCL_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2G_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C24_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  GGM_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RGB_DCM_STATUS;	/* CAM_CTL_RGB_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_YUV_DCM_STATUS_
{
		volatile struct	/* 0x150041B0 */
		{
				FIELD  C02_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  G2C_DCM_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C42_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NBC_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PCA_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEEE_DCM_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCE_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NR3D_DCM_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SL2B_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SL2C_DCM_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SRZ1_DCM_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SRZ2_DCM_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CRZ_DCM_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  MIX1_DCM_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  MIX2_DCM_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  MIX3_DCM_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  CRSP_DCM_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  C24B_DCM_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  MDPCROP_DCM_STATUS                    :  1;		/* 18..18, 0x00040000 */
				FIELD  FE_DCM_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_YUV_DCM_STATUS;	/* CAM_CTL_YUV_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_TOP_DCM_STATUS_
{
		volatile struct	/* 0x150041B4 */
		{
				FIELD  TOP_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_DCM_STATUS                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FBC_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TDR_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_STATUS;	/* CAM_CTL_TOP_DCM_STATUS */

typedef volatile union _CAM_REG_CTL_RAW_REQ_STATUS_
{
		volatile struct	/* 0x150041B8 */
		{
				FIELD  UFD_REQ_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PGN_REQ_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  UNP_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TG1_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DMX_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_REQ_STATUS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_REQ_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_REQ_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_REQ_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_REQ_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  RMX_REQ_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_REQ_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BMX_REQ_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_REQ_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_REQ_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_REQ_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_REQ_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_REQ_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  FLK_REQ_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  HBIN_REQ_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  AA_REQ_STATUS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SGG2_REQ_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  EIS_REQ_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  RMG_REQ_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_REQ_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_REQ_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  HBIN1_REQ_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  SGG3_REQ_STATUS                       :  1;		/* 29..29, 0x20000000 */
				FIELD  SGG5_REQ_STATUS                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_REQ_STATUS;	/* CAM_CTL_RAW_REQ_STATUS */

typedef volatile union _CAM_REG_CTL_RAW_D_REQ_STATUS_
{
		volatile struct	/* 0x150041BC */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TG1_D_REQ_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_D_REQ_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_D_REQ_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_D_REQ_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_D_REQ_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_D_REQ_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_D_REQ_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_D_REQ_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_D_REQ_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_D_REQ_STATUS                      :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_D_REQ_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_D_REQ_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_D_REQ_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  2;		/* 20..21, 0x00300000 */
				FIELD  AA_D_REQ_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  2;		/* 23..24, 0x01800000 */
				FIELD  RMG_D_REQ_STATUS                      :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_D_REQ_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_D_REQ_STATUS                      :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_D_REQ_STATUS;	/* CAM_CTL_RAW_D_REQ_STATUS */

typedef volatile union _CAM_REG_CTL_DMA_REQ_STATUS_
{
		volatile struct	/* 0x150041C0 */
		{
				FIELD  IMGI_REQ_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFDI_REQ_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  VIPI_REQ_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  VIP2I_REQ_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIP3I_REQ_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_REQ_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_REQ_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_REQ_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMG3O_REQ_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMG3BO_REQ_STATUS                     :  1;		/*  9.. 9, 0x00000200 */
				FIELD  IMG3CO_REQ_STATUS                     :  1;		/* 10..10, 0x00000400 */
				FIELD  FEO_REQ_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  IMGO_D_REQ_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_D_REQ_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  AFO_D_REQ_STATUS                      :  1;		/* 14..14, 0x00004000 */
				FIELD  LCSO_D_REQ_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_D_REQ_STATUS                      :  1;		/* 16..16, 0x00010000 */
				FIELD  BPCI_D_REQ_STATUS                     :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_D_REQ_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_REQ_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  UFEO_REQ_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RRZO_REQ_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  ESFKO_REQ_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_REQ_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  AAO_REQ_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  BPCI_REQ_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  LSCI_REQ_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_REQ_STATUS;	/* CAM_CTL_DMA_REQ_STATUS */

typedef volatile union _CAM_REG_CTL_RGB_REQ_STATUS_
{
		volatile struct	/* 0x150041C4 */
		{
				FIELD  SL2_REQ_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CFA_REQ_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCL_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2G_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C24_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  GGM_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RGB_REQ_STATUS;	/* CAM_CTL_RGB_REQ_STATUS */

typedef volatile union _CAM_REG_CTL_YUV_REQ_STATUS_
{
		volatile struct	/* 0x150041C8 */
		{
				FIELD  C02_REQ_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  G2C_REQ_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C42_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NBC_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PCA_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEEE_REQ_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCE_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NR3D_REQ_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SL2B_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SL2C_REQ_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SRZ1_REQ_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SRZ2_REQ_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CRZ_REQ_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  MIX1_REQ_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  MIX2_REQ_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  MIX3_REQ_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  CRSP_REQ_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  C24B_REQ_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  MDPCROP_REQ_STATUS                    :  1;		/* 18..18, 0x00040000 */
				FIELD  FE_REQ_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_YUV_REQ_STATUS;	/* CAM_CTL_YUV_REQ_STATUS */

typedef volatile union _CAM_REG_CTL_RAW_RDY_STATUS_
{
		volatile struct	/* 0x150041CC */
		{
				FIELD  UFD_RDY_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PGN_RDY_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  UNP_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TG1_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DMX_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_RDY_STATUS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_RDY_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_RDY_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_RDY_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_RDY_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  RMX_RDY_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_RDY_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BMX_RDY_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_RDY_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_RDY_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_RDY_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_RDY_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_RDY_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  FLK_RDY_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  HBIN_RDY_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  AA_RDY_STATUS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SGG2_RDY_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  EIS_RDY_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  RMG_RDY_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_RDY_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_RDY_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  HBIN1_RDY_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  SGG3_RDY_STATUS                       :  1;		/* 29..29, 0x20000000 */
				FIELD  SGG5_RDY_STATUS                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_RDY_STATUS;	/* CAM_CTL_RAW_RDY_STATUS */

typedef volatile union _CAM_REG_CTL_RAW_D_RDY_STATUS_
{
		volatile struct	/* 0x150041D0 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TG1_D_RDY_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WBN_D_RDY_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  W2G_D_RDY_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  OB_D_RDY_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BNR_D_RDY_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LSC_D_RDY_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RPG_D_RDY_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  RRZ_D_RDY_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  PAKG_D_RDY_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_D_RDY_STATUS                      :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  LCS_D_RDY_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_D_RDY_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  AF_D_RDY_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  2;		/* 20..21, 0x00300000 */
				FIELD  AA_D_RDY_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  2;		/* 23..24, 0x01800000 */
				FIELD  RMG_D_RDY_STATUS                      :  1;		/* 25..25, 0x02000000 */
				FIELD  CPG_D_RDY_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  DBS_D_RDY_STATUS                      :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_D_RDY_STATUS;	/* CAM_CTL_RAW_D_RDY_STATUS */

typedef volatile union _CAM_REG_CTL_DMA_RDY_STATUS_
{
		volatile struct	/* 0x150041D4 */
		{
				FIELD  IMGI_RDY_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFDI_RDY_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  VIPI_RDY_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  VIP2I_RDY_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIP3I_RDY_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_RDY_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_RDY_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  IMG2O_RDY_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  IMG3O_RDY_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMG3BO_RDY_STATUS                     :  1;		/*  9.. 9, 0x00000200 */
				FIELD  IMG3CO_RDY_STATUS                     :  1;		/* 10..10, 0x00000400 */
				FIELD  FEO_RDY_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  IMGO_D_RDY_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_D_RDY_STATUS                     :  1;		/* 13..13, 0x00002000 */
				FIELD  AFO_D_RDY_STATUS                      :  1;		/* 14..14, 0x00004000 */
				FIELD  LCSO_D_RDY_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_D_RDY_STATUS                      :  1;		/* 16..16, 0x00010000 */
				FIELD  BPCI_D_RDY_STATUS                     :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_D_RDY_STATUS                     :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_RDY_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  UFEO_RDY_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RRZO_RDY_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  ESFKO_RDY_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_RDY_STATUS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  AAO_RDY_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  BPCI_RDY_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  LSCI_RDY_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_RDY_STATUS;	/* CAM_CTL_DMA_RDY_STATUS */

typedef volatile union _CAM_REG_CTL_RGB_RDY_STATUS_
{
		volatile struct	/* 0x150041D8 */
		{
				FIELD  SL2_RDY_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CFA_RDY_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCL_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2G_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C24_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MFB_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  GGM_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RGB_RDY_STATUS;	/* CAM_CTL_RGB_RDY_STATUS */

typedef volatile union _CAM_REG_CTL_YUV_RDY_STATUS_
{
		volatile struct	/* 0x150041DC */
		{
				FIELD  C02_RDY_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  G2C_RDY_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C42_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NBC_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PCA_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEEE_RDY_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCE_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NR3D_RDY_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SL2B_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SL2C_RDY_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SRZ1_RDY_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SRZ2_RDY_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CRZ_RDY_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  MIX1_RDY_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  MIX2_RDY_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  MIX3_RDY_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  CRSP_RDY_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  C24B_RDY_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  MDPCROP_RDY_STATUS                    :  1;		/* 18..18, 0x00040000 */
				FIELD  FE_RDY_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_YUV_RDY_STATUS;	/* CAM_CTL_YUV_RDY_STATUS */

typedef volatile union _CAM_REG_TG_SEN_MODE_
{
		volatile struct	/* 0x15004410 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_MODE;	/* CAM_TG_SEN_MODE */

typedef volatile union _CAM_REG_TG_VF_CON_
{
		volatile struct	/* 0x15004414 */
		{
				FIELD  VFDATA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SINGLE_MODE                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FR_CON                                :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SP_DELAY                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SPDELAY_MODE                          :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_VF_CON;	/* CAM_TG_VF_CON */

typedef volatile union _CAM_REG_TG_SEN_GRAB_PXL_
{
		volatile struct	/* 0x15004418 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_PXL;	/* CAM_TG_SEN_GRAB_PXL */

typedef volatile union _CAM_REG_TG_SEN_GRAB_LIN_
{
		volatile struct	/* 0x1500441C */
		{
				FIELD  LIN_S                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LIN_E                                 : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_LIN;	/* CAM_TG_SEN_GRAB_LIN */

typedef volatile union _CAM_REG_TG_PATH_CFG_
{
		volatile struct	/* 0x15004420 */
		{
				FIELD  SEN_IN_LSB                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  JPGINF_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MEMIN_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  JPG_LINEND_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DB_LOAD_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DB_LOAD_SRC                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DB_LOAD_VSPOL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_PATH_CFG;	/* CAM_TG_PATH_CFG */

typedef volatile union _CAM_REG_TG_MEMIN_CTL_
{
		volatile struct	/* 0x15004424 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  FBC_CNT                               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MEMIN_CTL;	/* CAM_TG_MEMIN_CTL */

typedef volatile union _CAM_REG_TG_INT1_
{
		volatile struct	/* 0x15004428 */
		{
				FIELD  TG_INT1_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_INT1;	/* CAM_TG_INT1 */

typedef volatile union _CAM_REG_TG_INT2_
{
		volatile struct	/* 0x1500442C */
		{
				FIELD  TG_INT2_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_INT2;	/* CAM_TG_INT2 */

typedef volatile union _CAM_REG_TG_SOF_CNT_
{
		volatile struct	/* 0x15004430 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SOF_CNT;	/* CAM_TG_SOF_CNT */

typedef volatile union _CAM_REG_TG_SOT_CNT_
{
		volatile struct	/* 0x15004434 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SOT_CNT;	/* CAM_TG_SOT_CNT */

typedef volatile union _CAM_REG_TG_EOT_CNT_
{
		volatile struct	/* 0x15004438 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_EOT_CNT;	/* CAM_TG_EOT_CNT */

typedef volatile union _CAM_REG_TG_ERR_CTL_
{
		volatile struct	/* 0x1500443C */
		{
				FIELD  GRAB_ERR_FLIMIT_NO                    :  4;		/*  0.. 3, 0x0000000F */
				FIELD  GRAB_ERR_FLIMIT_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  GRAB_ERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DBG_SRC_SEL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_ERR_CTL;	/* CAM_TG_ERR_CTL */

typedef volatile union _CAM_REG_TG_DAT_NO_
{
		volatile struct	/* 0x15004440 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_DAT_NO;	/* CAM_TG_DAT_NO */

typedef volatile union _CAM_REG_TG_FRM_CNT_ST_
{
		volatile struct	/* 0x15004444 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FRM_CNT_ST;	/* CAM_TG_FRM_CNT_ST */

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_
{
		volatile struct	/* 0x15004448 */
		{
				FIELD  LINE_CNT                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FRMSIZE_ST;	/* CAM_TG_FRMSIZE_ST */

typedef volatile union _CAM_REG_TG_INTER_ST_
{
		volatile struct	/* 0x1500444C */
		{
				FIELD  SYN_VF_DATA_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  OUT_RDY                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OUT_REQ                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  TG_CAM_CS                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CAM_FRM_CNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_INTER_ST;	/* CAM_TG_INTER_ST */

typedef volatile union _CAM_REG_TG_FLASHA_CTL_
{
		volatile struct	/* 0x15004460 */
		{
				FIELD  FLASHA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLASH_EN                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FLASHA_STARTPNT                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FLASHA_END_FRM                        :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  FLASH_POL                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHA_CTL;	/* CAM_TG_FLASHA_CTL */

typedef volatile union _CAM_REG_TG_FLASHA_LINE_CNT_
{
		volatile struct	/* 0x15004464 */
		{
				FIELD  FLASHA_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHA_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHA_LINE_CNT;	/* CAM_TG_FLASHA_LINE_CNT */

typedef volatile union _CAM_REG_TG_FLASHA_POS_
{
		volatile struct	/* 0x15004468 */
		{
				FIELD  FLASHA_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHA_LINE                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHA_POS;	/* CAM_TG_FLASHA_POS */

typedef volatile union _CAM_REG_TG_FLASHB_CTL_
{
		volatile struct	/* 0x1500446C */
		{
				FIELD  FLASHB_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLASHB_TRIG_SRC                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FLASHB_STARTPNT                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FLASHB_START_FRM                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  FLASHB_CONT_FRM                       :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_CTL;	/* CAM_TG_FLASHB_CTL */

typedef volatile union _CAM_REG_TG_FLASHB_LINE_CNT_
{
		volatile struct	/* 0x15004470 */
		{
				FIELD  FLASHB_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHB_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_LINE_CNT;	/* CAM_TG_FLASHB_LINE_CNT */

typedef volatile union _CAM_REG_TG_FLASHB_POS_
{
		volatile struct	/* 0x15004474 */
		{
				FIELD  FLASHB_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHB_LINE                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_POS;	/* CAM_TG_FLASHB_POS */

typedef volatile union _CAM_REG_TG_FLASHB_POS1_
{
		volatile struct	/* 0x15004478 */
		{
				FIELD  FLASHB_CYC_CNT                        : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_POS1;	/* CAM_TG_FLASHB_POS1 */

typedef volatile union _CAM_REG_TG_GSCTRL_CTL_
{
		volatile struct	/* 0x1500447C */
		{
				FIELD  GSCTRL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  GSCTRL_POL                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_GSCTRL_CTL;	/* CAM_TG_GSCTRL_CTL */

typedef volatile union _CAM_REG_TG_GSCTRL_TIME_
{
		volatile struct	/* 0x15004480 */
		{
				FIELD  GS_EPTIME                             : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  GSMS_TIMEU                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_GSCTRL_TIME;	/* CAM_TG_GSCTRL_TIME */

typedef volatile union _CAM_REG_TG_MS_PHASE_
{
		volatile struct	/* 0x15004484 */
		{
				FIELD  MSCTRL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MSCTRL_VSPOL                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MSCTRL_OPEN_TRSRC                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  MSCTRL_TRSRC                          :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  MSCP1_PH0                             :  1;		/* 16..16, 0x00010000 */
				FIELD  MSCP1_PH1                             :  1;		/* 17..17, 0x00020000 */
				FIELD  MSCP1_PH2                             :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  MSOP1_PH0                             :  1;		/* 20..20, 0x00100000 */
				FIELD  MSOP1_PH1                             :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  MSP1_RST                              :  1;		/* 23..23, 0x00800000 */
				FIELD  MSCP2_PH0                             :  1;		/* 24..24, 0x01000000 */
				FIELD  MSCP2_PH1                             :  1;		/* 25..25, 0x02000000 */
				FIELD  MSCP2_PH2                             :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MSOP2_PH0                             :  1;		/* 28..28, 0x10000000 */
				FIELD  MSOP2_PH1                             :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  MSP2_RST                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MS_PHASE;	/* CAM_TG_MS_PHASE */

typedef volatile union _CAM_REG_TG_MS_CL_TIME_
{
		volatile struct	/* 0x15004488 */
		{
				FIELD  MS_TCLOSE                             : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MS_CL_TIME;	/* CAM_TG_MS_CL_TIME */

typedef volatile union _CAM_REG_TG_MS_OP_TIME_
{
		volatile struct	/* 0x1500448C */
		{
				FIELD  MS_TOPEN                              : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MS_OP_TIME;	/* CAM_TG_MS_OP_TIME */

typedef volatile union _CAM_REG_TG_MS_CLPH_TIME_
{
		volatile struct	/* 0x15004490 */
		{
				FIELD  MS_CL_T1                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MS_CL_T2                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MS_CLPH_TIME;	/* CAM_TG_MS_CLPH_TIME */

typedef volatile union _CAM_REG_TG_MS_OPPH_TIME_
{
		volatile struct	/* 0x15004494 */
		{
				FIELD  MS_OP_T3                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MS_OP_T4                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MS_OPPH_TIME;	/* CAM_TG_MS_OPPH_TIME */

typedef volatile union _CAM_REG_HBN_SIZE_
{
		volatile struct	/* 0x150044F0 */
		{
				FIELD  HBN_IN_H                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HBN_IN_V                              : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HBN_SIZE;	/* CAM_HBN_SIZE */

typedef volatile union _CAM_REG_HBN_MODE_
{
		volatile struct	/* 0x150044F4 */
		{
				FIELD  HBIN_ACC                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  HBIN_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HBN_MODE;	/* CAM_HBN_MODE */

typedef volatile union _CAM_REG_HBN2_SIZE_
{
		volatile struct	/* 0x150044F8 */
		{
				FIELD  HBN_IN_H                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HBN_IN_V                              : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HBN2_SIZE;	/* CAM_HBN2_SIZE */

typedef volatile union _CAM_REG_HBN2_MODE_
{
		volatile struct	/* 0x150044FC */
		{
				FIELD  HBIN_ACC                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  HBIN_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HBN2_MODE;	/* CAM_HBN2_MODE */

typedef volatile union _CAM_REG_OBC_OFFST0_
{
		volatile struct	/* 0x15004500 */
		{
				FIELD  OBC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST0;	/* CAM_OBC_OFFST0 */

typedef volatile union _CAM_REG_OBC_OFFST1_
{
		volatile struct	/* 0x15004504 */
		{
				FIELD  OBC_OFST_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST1;	/* CAM_OBC_OFFST1 */

typedef volatile union _CAM_REG_OBC_OFFST2_
{
		volatile struct	/* 0x15004508 */
		{
				FIELD  OBC_OFST_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST2;	/* CAM_OBC_OFFST2 */

typedef volatile union _CAM_REG_OBC_OFFST3_
{
		volatile struct	/* 0x1500450C */
		{
				FIELD  OBC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST3;	/* CAM_OBC_OFFST3 */

typedef volatile union _CAM_REG_OBC_GAIN0_
{
		volatile struct	/* 0x15004510 */
		{
				FIELD  OBC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN0;	/* CAM_OBC_GAIN0 */

typedef volatile union _CAM_REG_OBC_GAIN1_
{
		volatile struct	/* 0x15004514 */
		{
				FIELD  OBC_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN1;	/* CAM_OBC_GAIN1 */

typedef volatile union _CAM_REG_OBC_GAIN2_
{
		volatile struct	/* 0x15004518 */
		{
				FIELD  OBC_GAIN_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN2;	/* CAM_OBC_GAIN2 */

typedef volatile union _CAM_REG_OBC_GAIN3_
{
		volatile struct	/* 0x1500451C */
		{
				FIELD  OBC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN3;	/* CAM_OBC_GAIN3 */

typedef volatile union _CAM_REG_LSC_CTL1_
{
		volatile struct	/* 0x15004530 */
		{
				FIELD  SDBLK_YOFST                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  SDBLK_XOFST                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  SD_COEFRD_MODE                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  SD_ULTRA_MODE                         :  1;		/* 28..28, 0x10000000 */
				FIELD  LSC_PRC_MODE                          :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_CTL1;	/* CAM_LSC_CTL1 */

typedef volatile union _CAM_REG_LSC_CTL2_
{
		volatile struct	/* 0x15004534 */
		{
				FIELD  LSC_SDBLK_WIDTH                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  LSC_SDBLK_XNUM                        :  5;		/* 12..16, 0x0001F000 */
				FIELD  LSC_OFLN                              :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_CTL2;	/* CAM_LSC_CTL2 */

typedef volatile union _CAM_REG_LSC_CTL3_
{
		volatile struct	/* 0x15004538 */
		{
				FIELD  LSC_SDBLK_HEIGHT                      : 12;		/*  0..11, 0x00000FFF */
				FIELD  LSC_SDBLK_YNUM                        :  5;		/* 12..16, 0x0001F000 */
				FIELD  LSC_SPARE                             : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_CTL3;	/* CAM_LSC_CTL3 */

typedef volatile union _CAM_REG_LSC_LBLOCK_
{
		volatile struct	/* 0x1500453C */
		{
				FIELD  LSC_SDBLK_lHEIGHT                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LSC_SDBLK_lWIDTH                      : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_LBLOCK;	/* CAM_LSC_LBLOCK */

typedef volatile union _CAM_REG_LSC_RATIO_
{
		volatile struct	/* 0x15004540 */
		{
				FIELD  LSC_RA3                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  LSC_RA2                               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_RA1                               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  LSC_RA0                               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_RATIO;	/* CAM_LSC_RATIO */

typedef volatile union _CAM_REG_LSC_TPIPE_OFST_
{
		volatile struct	/* 0x15004544 */
		{
				FIELD  LSC_TPIPE_OFST_Y                      : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LSC_TPIPE_OFST_X                      : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_TPIPE_OFST;	/* CAM_LSC_TPIPE_OFST */

typedef volatile union _CAM_REG_LSC_TPIPE_SIZE_
{
		volatile struct	/* 0x15004548 */
		{
				FIELD  LSC_TPIPE_SIZE_Y                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_TPIPE_SIZE_X                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_TPIPE_SIZE;	/* CAM_LSC_TPIPE_SIZE */

typedef volatile union _CAM_REG_LSC_GAIN_TH_
{
		volatile struct	/* 0x1500454C */
		{
				FIELD  LSC_GAIN_TH2                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSC_GAIN_TH1                          :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  LSC_GAIN_TH0                          :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_GAIN_TH;	/* CAM_LSC_GAIN_TH */

typedef volatile union _CAM_REG_RPG_SATU_1_
{
		volatile struct	/* 0x15004550 */
		{
				FIELD  RPG_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_SATU_1;	/* CAM_RPG_SATU_1 */

typedef volatile union _CAM_REG_RPG_SATU_2_
{
		volatile struct	/* 0x15004554 */
		{
				FIELD  RPG_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_SATU_2;	/* CAM_RPG_SATU_2 */

typedef volatile union _CAM_REG_RPG_GAIN_1_
{
		volatile struct	/* 0x15004558 */
		{
				FIELD  RPG_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_GAIN_1;	/* CAM_RPG_GAIN_1 */

typedef volatile union _CAM_REG_RPG_GAIN_2_
{
		volatile struct	/* 0x1500455C */
		{
				FIELD  RPG_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_GAIN_2;	/* CAM_RPG_GAIN_2 */

typedef volatile union _CAM_REG_RPG_OFST_1_
{
		volatile struct	/* 0x15004560 */
		{
				FIELD  RPG_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_OFST_1;	/* CAM_RPG_OFST_1 */

typedef volatile union _CAM_REG_RPG_OFST_2_
{
		volatile struct	/* 0x15004564 */
		{
				FIELD  RPG_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_OFST_2;	/* CAM_RPG_OFST_2 */

typedef volatile union _CAM_REG_SGG3_PGN_
{
		volatile struct	/* 0x15004570 */
		{
				FIELD  SGG3_GAIN                             : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG3_PGN;	/* CAM_SGG3_PGN */

typedef volatile union _CAM_REG_SGG3_GMRC_1_
{
		volatile struct	/* 0x15004574 */
		{
				FIELD  SGG3_GMR_1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG3_GMR_2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG3_GMR_3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG3_GMR_4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG3_GMRC_1;	/* CAM_SGG3_GMRC_1 */

typedef volatile union _CAM_REG_SGG3_GMRC_2_
{
		volatile struct	/* 0x15004578 */
		{
				FIELD  SGG3_GMR_5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG3_GMR_6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG3_GMR_7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG3_GMRC_2;	/* CAM_SGG3_GMRC_2 */

typedef volatile union _CAM_REG_SGG2_PGN_
{
		volatile struct	/* 0x15004580 */
		{
				FIELD  SGG2_GAIN                             : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_PGN;	/* CAM_SGG2_PGN */

typedef volatile union _CAM_REG_SGG2_GMRC_1_
{
		volatile struct	/* 0x15004584 */
		{
				FIELD  SGG2_GMR_1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG2_GMR_2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG2_GMR_3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG2_GMR_4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_GMRC_1;	/* CAM_SGG2_GMRC_1 */

typedef volatile union _CAM_REG_SGG2_GMRC_2_
{
		volatile struct	/* 0x15004588 */
		{
				FIELD  SGG2_GMR_5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG2_GMR_6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG2_GMR_7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_GMRC_2;	/* CAM_SGG2_GMRC_2 */

typedef volatile union _CAM_REG_SGG5_PGN_
{
		volatile struct	/* 0x15004590 */
		{
				FIELD  SGG5_GAIN                             : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_PGN;	/* CAM_SGG5_PGN */

typedef volatile union _CAM_REG_SGG5_GMRC_1_
{
		volatile struct	/* 0x15004594 */
		{
				FIELD  SGG5_GMR_1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG5_GMR_2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG5_GMR_3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG5_GMR_4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_GMRC_1;	/* CAM_SGG5_GMRC_1 */

typedef volatile union _CAM_REG_SGG5_GMRC_2_
{
		volatile struct	/* 0x15004598 */
		{
				FIELD  SGG5_GMR_5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG5_GMR_6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG5_GMR_7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_GMRC_2;	/* CAM_SGG5_GMRC_2 */

typedef volatile union _CAM_REG_AWB_WIN_ORG_
{
		volatile struct	/* 0x150045B0 */
		{
				FIELD  AWB_W_HORG                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AWB_W_VORG                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_ORG;	/* CAM_AWB_WIN_ORG */

typedef volatile union _CAM_REG_AWB_WIN_SIZE_
{
		volatile struct	/* 0x150045B4 */
		{
				FIELD  AWB_W_HSIZE                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AWB_W_VSIZE                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_SIZE;	/* CAM_AWB_WIN_SIZE */

typedef volatile union _CAM_REG_AWB_WIN_PIT_
{
		volatile struct	/* 0x150045B8 */
		{
				FIELD  AWB_W_HPIT                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AWB_W_VPIT                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_PIT;	/* CAM_AWB_WIN_PIT */

typedef volatile union _CAM_REG_AWB_WIN_NUM_
{
		volatile struct	/* 0x150045BC */
		{
				FIELD  AWB_W_HNUM                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_W_VNUM                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_NUM;	/* CAM_AWB_WIN_NUM */

typedef volatile union _CAM_REG_AWB_GAIN1_0_
{
		volatile struct	/* 0x150045C0 */
		{
				FIELD  AWB_GAIN1_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AWB_GAIN1_G                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_GAIN1_0;	/* CAM_AWB_GAIN1_0 */

typedef volatile union _CAM_REG_AWB_GAIN1_1_
{
		volatile struct	/* 0x150045C4 */
		{
				FIELD  AWB_GAIN1_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_GAIN1_1;	/* CAM_AWB_GAIN1_1 */

typedef volatile union _CAM_REG_AWB_LMT1_0_
{
		volatile struct	/* 0x150045C8 */
		{
				FIELD  AWB_LMT1_R                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_LMT1_G                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LMT1_0;	/* CAM_AWB_LMT1_0 */

typedef volatile union _CAM_REG_AWB_LMT1_1_
{
		volatile struct	/* 0x150045CC */
		{
				FIELD  AWB_LMT1_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LMT1_1;	/* CAM_AWB_LMT1_1 */

typedef volatile union _CAM_REG_AWB_LOW_THR_
{
		volatile struct	/* 0x150045D0 */
		{
				FIELD  AWB_LOW_THR0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AWB_LOW_THR1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_LOW_THR2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LOW_THR;	/* CAM_AWB_LOW_THR */

typedef volatile union _CAM_REG_AWB_HI_THR_
{
		volatile struct	/* 0x150045D4 */
		{
				FIELD  AWB_HI_THR0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AWB_HI_THR1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_HI_THR2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_HI_THR;	/* CAM_AWB_HI_THR */

typedef volatile union _CAM_REG_AWB_PIXEL_CNT0_
{
		volatile struct	/* 0x150045D8 */
		{
				FIELD  AWB_PIXEL_CNT0                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT0;	/* CAM_AWB_PIXEL_CNT0 */

typedef volatile union _CAM_REG_AWB_PIXEL_CNT1_
{
		volatile struct	/* 0x150045DC */
		{
				FIELD  AWB_PIXEL_CNT1                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT1;	/* CAM_AWB_PIXEL_CNT1 */

typedef volatile union _CAM_REG_AWB_PIXEL_CNT2_
{
		volatile struct	/* 0x150045E0 */
		{
				FIELD  AWB_PIXEL_CNT2                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT2;	/* CAM_AWB_PIXEL_CNT2 */

typedef volatile union _CAM_REG_AWB_ERR_THR_
{
		volatile struct	/* 0x150045E4 */
		{
				FIELD  AWB_ERR_THR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_ERR_SFT                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_ERR_THR;	/* CAM_AWB_ERR_THR */

typedef volatile union _CAM_REG_AWB_ROT_
{
		volatile struct	/* 0x150045E8 */
		{
				FIELD  AWB_C                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  AWB_S                                 : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_ROT;	/* CAM_AWB_ROT */

typedef volatile union _CAM_REG_AWB_L0_X_
{
		volatile struct	/* 0x150045EC */
		{
				FIELD  AWB_L0_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L0_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L0_X;	/* CAM_AWB_L0_X */

typedef volatile union _CAM_REG_AWB_L0_Y_
{
		volatile struct	/* 0x150045F0 */
		{
				FIELD  AWB_L0_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L0_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L0_Y;	/* CAM_AWB_L0_Y */

typedef volatile union _CAM_REG_AWB_L1_X_
{
		volatile struct	/* 0x150045F4 */
		{
				FIELD  AWB_L1_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L1_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L1_X;	/* CAM_AWB_L1_X */

typedef volatile union _CAM_REG_AWB_L1_Y_
{
		volatile struct	/* 0x150045F8 */
		{
				FIELD  AWB_L1_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L1_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L1_Y;	/* CAM_AWB_L1_Y */

typedef volatile union _CAM_REG_AWB_L2_X_
{
		volatile struct	/* 0x150045FC */
		{
				FIELD  AWB_L2_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L2_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L2_X;	/* CAM_AWB_L2_X */

typedef volatile union _CAM_REG_AWB_L2_Y_
{
		volatile struct	/* 0x15004600 */
		{
				FIELD  AWB_L2_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L2_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L2_Y;	/* CAM_AWB_L2_Y */

typedef volatile union _CAM_REG_AWB_L3_X_
{
		volatile struct	/* 0x15004604 */
		{
				FIELD  AWB_L3_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L3_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L3_X;	/* CAM_AWB_L3_X */

typedef volatile union _CAM_REG_AWB_L3_Y_
{
		volatile struct	/* 0x15004608 */
		{
				FIELD  AWB_L3_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L3_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L3_Y;	/* CAM_AWB_L3_Y */

typedef volatile union _CAM_REG_AWB_L4_X_
{
		volatile struct	/* 0x1500460C */
		{
				FIELD  AWB_L4_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L4_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L4_X;	/* CAM_AWB_L4_X */

typedef volatile union _CAM_REG_AWB_L4_Y_
{
		volatile struct	/* 0x15004610 */
		{
				FIELD  AWB_L4_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L4_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L4_Y;	/* CAM_AWB_L4_Y */

typedef volatile union _CAM_REG_AWB_L5_X_
{
		volatile struct	/* 0x15004614 */
		{
				FIELD  AWB_L5_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L5_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L5_X;	/* CAM_AWB_L5_X */

typedef volatile union _CAM_REG_AWB_L5_Y_
{
		volatile struct	/* 0x15004618 */
		{
				FIELD  AWB_L5_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L5_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L5_Y;	/* CAM_AWB_L5_Y */

typedef volatile union _CAM_REG_AWB_L6_X_
{
		volatile struct	/* 0x1500461C */
		{
				FIELD  AWB_L6_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L6_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L6_X;	/* CAM_AWB_L6_X */

typedef volatile union _CAM_REG_AWB_L6_Y_
{
		volatile struct	/* 0x15004620 */
		{
				FIELD  AWB_L6_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L6_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L6_Y;	/* CAM_AWB_L6_Y */

typedef volatile union _CAM_REG_AWB_L7_X_
{
		volatile struct	/* 0x15004624 */
		{
				FIELD  AWB_L7_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L7_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L7_X;	/* CAM_AWB_L7_X */

typedef volatile union _CAM_REG_AWB_L7_Y_
{
		volatile struct	/* 0x15004628 */
		{
				FIELD  AWB_L7_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L7_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L7_Y;	/* CAM_AWB_L7_Y */

typedef volatile union _CAM_REG_AWB_L8_X_
{
		volatile struct	/* 0x1500462C */
		{
				FIELD  AWB_L8_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L8_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L8_X;	/* CAM_AWB_L8_X */

typedef volatile union _CAM_REG_AWB_L8_Y_
{
		volatile struct	/* 0x15004630 */
		{
				FIELD  AWB_L8_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L8_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L8_Y;	/* CAM_AWB_L8_Y */

typedef volatile union _CAM_REG_AWB_L9_X_
{
		volatile struct	/* 0x15004634 */
		{
				FIELD  AWB_L9_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L9_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L9_X;	/* CAM_AWB_L9_X */

typedef volatile union _CAM_REG_AWB_L9_Y_
{
		volatile struct	/* 0x15004638 */
		{
				FIELD  AWB_L9_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L9_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L9_Y;	/* CAM_AWB_L9_Y */

typedef volatile union _CAM_REG_AWB_SPARE_
{
		volatile struct	/* 0x1500463C */
		{
				FIELD  AWB_SPARE                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_SPARE;	/* CAM_AWB_SPARE */

typedef volatile union _CAM_REG_AE_HST_CTL_
{
		volatile struct	/* 0x15004650 */
		{
				FIELD  AE_HST0_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_HST1_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AE_HST2_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AE_HST3_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST_CTL;	/* CAM_AE_HST_CTL */

typedef volatile union _CAM_REG_AE_GAIN2_0_
{
		volatile struct	/* 0x15004654 */
		{
				FIELD  AE_GAIN2_R                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_GAIN2_G                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_GAIN2_0;	/* CAM_AE_GAIN2_0 */

typedef volatile union _CAM_REG_AE_GAIN2_1_
{
		volatile struct	/* 0x15004658 */
		{
				FIELD  AE_GAIN2_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_GAIN2_1;	/* CAM_AE_GAIN2_1 */

typedef volatile union _CAM_REG_AE_LMT2_0_
{
		volatile struct	/* 0x1500465C */
		{
				FIELD  AE_LMT2_R                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_LMT2_G                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_LMT2_0;	/* CAM_AE_LMT2_0 */

typedef volatile union _CAM_REG_AE_LMT2_1_
{
		volatile struct	/* 0x15004660 */
		{
				FIELD  AE_LMT2_B                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_LMT2_1;	/* CAM_AE_LMT2_1 */

typedef volatile union _CAM_REG_AE_RC_CNV_0_
{
		volatile struct	/* 0x15004664 */
		{
				FIELD  AE_RC_CNV00                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV01                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_0;	/* CAM_AE_RC_CNV_0 */

typedef volatile union _CAM_REG_AE_RC_CNV_1_
{
		volatile struct	/* 0x15004668 */
		{
				FIELD  AE_RC_CNV02                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV10                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_1;	/* CAM_AE_RC_CNV_1 */

typedef volatile union _CAM_REG_AE_RC_CNV_2_
{
		volatile struct	/* 0x1500466C */
		{
				FIELD  AE_RC_CNV11                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV12                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_2;	/* CAM_AE_RC_CNV_2 */

typedef volatile union _CAM_REG_AE_RC_CNV_3_
{
		volatile struct	/* 0x15004670 */
		{
				FIELD  AE_RC_CNV20                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV21                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_3;	/* CAM_AE_RC_CNV_3 */

typedef volatile union _CAM_REG_AE_RC_CNV_4_
{
		volatile struct	/* 0x15004674 */
		{
				FIELD  AE_RC_CNV22                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_ACC                             :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_4;	/* CAM_AE_RC_CNV_4 */

typedef volatile union _CAM_REG_AE_YGAMMA_0_
{
		volatile struct	/* 0x15004678 */
		{
				FIELD  Y_GMR1                                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  Y_GMR2                                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  Y_GMR3                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  Y_GMR4                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_YGAMMA_0;	/* CAM_AE_YGAMMA_0 */

typedef volatile union _CAM_REG_AE_YGAMMA_1_
{
		volatile struct	/* 0x1500467C */
		{
				FIELD  Y_GMR5                                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_YGAMMA_1;	/* CAM_AE_YGAMMA_1 */

typedef volatile union _CAM_REG_AE_HST_SET_
{
		volatile struct	/* 0x15004680 */
		{
				FIELD  AE_BIN_MODE_0                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  AE_BIN_MODE_1                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  AE_BIN_MODE_2                         :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  AE_BIN_MODE_3                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AE_COLOR_MODE_0                       :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  AE_COLOR_MODE_1                       :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_COLOR_MODE_2                       :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  AE_COLOR_MODE_3                       :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST_SET;	/* CAM_AE_HST_SET */

typedef volatile union _CAM_REG_AE_HST0_RNG_
{
		volatile struct	/* 0x15004684 */
		{
				FIELD  AE_X_LOW_0                            :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AE_X_HI_0                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_Y_LOW_0                            :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_Y_HI_0                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST0_RNG;	/* CAM_AE_HST0_RNG */

typedef volatile union _CAM_REG_AE_HST1_RNG_
{
		volatile struct	/* 0x15004688 */
		{
				FIELD  AE_X_LOW_1                            :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AE_X_HI_1                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_Y_LOW_1                            :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_Y_HI_1                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST1_RNG;	/* CAM_AE_HST1_RNG */

typedef volatile union _CAM_REG_AE_HST2_RNG_
{
		volatile struct	/* 0x1500468C */
		{
				FIELD  AE_X_LOW_2                            :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AE_X_HI_2                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_Y_LOW_2                            :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_Y_HI_2                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST2_RNG;	/* CAM_AE_HST2_RNG */

typedef volatile union _CAM_REG_AE_HST3_RNG_
{
		volatile struct	/* 0x15004690 */
		{
				FIELD  AE_X_LOW_3                            :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AE_X_HI_3                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_Y_LOW_3                            :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_Y_HI_3                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_HST3_RNG;	/* CAM_AE_HST3_RNG */

typedef volatile union _CAM_REG_AE_SPARE_
{
		volatile struct	/* 0x15004694 */
		{
				FIELD  AE_SPARE                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_SPARE;	/* CAM_AE_SPARE */

typedef volatile union _CAM_REG_AE_STAT_EN_
{
		volatile struct	/* 0x15004698 */
		{
				FIELD  AE_TSF_STAT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_STAT_EN;	/* CAM_AE_STAT_EN */

typedef volatile union _CAM_REG_SGG1_PGN_
{
		volatile struct	/* 0x150046A0 */
		{
				FIELD  SGG1_GAIN                             : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_PGN;	/* CAM_SGG1_PGN */

typedef volatile union _CAM_REG_SGG1_GMRC_1_
{
		volatile struct	/* 0x150046A4 */
		{
				FIELD  SGG1_GMR_1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG1_GMR_2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG1_GMR_3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG1_GMR_4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_GMRC_1;	/* CAM_SGG1_GMRC_1 */

typedef volatile union _CAM_REG_SGG1_GMRC_2_
{
		volatile struct	/* 0x150046A8 */
		{
				FIELD  SGG1_GMR_5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG1_GMR_6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG1_GMR_7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_GMRC_2;	/* CAM_SGG1_GMRC_2 */

typedef volatile union _CAM_REG_AF_CON_
{
		volatile struct	/* 0x150046B0 */
		{
				FIELD  AF_BLF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  AF_BLF_D_LVL                          :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AF_BLF_R_LVL                          :  4;		/*  8..11, 0x00000F00 */
				FIELD  AF_BLF_VFIR_MUX                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AF_H_GONLY                            :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_V_GONLY                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  AF_V_AVG_LVL                          :  2;		/* 20..21, 0x00300000 */
				FIELD  AF_EXT_STAT_EN                        :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  5;		/* 23..27, 0x0F800000 */
				FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_CON;	/* CAM_AF_CON */

typedef volatile union _CAM_REG_AF_TH_0_
{
		volatile struct	/* 0x150046B4 */
		{
				FIELD  AF_H_TH_0                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_H_TH_1                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_0;	/* CAM_AF_TH_0 */

typedef volatile union _CAM_REG_AF_TH_1_
{
		volatile struct	/* 0x150046B8 */
		{
				FIELD  AF_V_TH                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_R_SAT_TH                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_G_SAT_TH                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_B_SAT_TH                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_1;	/* CAM_AF_TH_1 */

typedef volatile union _CAM_REG_AF_FLT_1_
{
		volatile struct	/* 0x150046BC */
		{
				FIELD  AF_HFLT0_P1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P4                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_1;	/* CAM_AF_FLT_1 */

typedef volatile union _CAM_REG_AF_FLT_2_
{
		volatile struct	/* 0x150046C0 */
		{
				FIELD  AF_HFLT0_P5                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P6                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P7                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P8                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_2;	/* CAM_AF_FLT_2 */

typedef volatile union _CAM_REG_AF_FLT_3_
{
		volatile struct	/* 0x150046C4 */
		{
				FIELD  AF_HFLT0_P9                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P10                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P11                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P12                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_3;	/* CAM_AF_FLT_3 */

typedef volatile union _CAM_REG_AF_FLT_4_
{
		volatile struct	/* 0x150046C8 */
		{
				FIELD  AF_HFLT1_P1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P4                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_4;	/* CAM_AF_FLT_4 */

typedef volatile union _CAM_REG_AF_FLT_5_
{
		volatile struct	/* 0x150046CC */
		{
				FIELD  AF_HFLT1_P5                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P6                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P7                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P8                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_5;	/* CAM_AF_FLT_5 */

typedef volatile union _CAM_REG_AF_FLT_6_
{
		volatile struct	/* 0x150046D0 */
		{
				FIELD  AF_HFLT1_P9                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P10                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P11                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P12                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_6;	/* CAM_AF_FLT_6 */

typedef volatile union _CAM_REG_AF_FLT_7_
{
		volatile struct	/* 0x150046D4 */
		{
				FIELD  AF_VFLT_X0                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AF_VFLT_X1                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_7;	/* CAM_AF_FLT_7 */

typedef volatile union _CAM_REG_AF_FLT_8_
{
		volatile struct	/* 0x150046D8 */
		{
				FIELD  AF_VFLT_X2                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AF_VFLT_X3                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_8;	/* CAM_AF_FLT_8 */

typedef volatile union _CAM_REG_AF_SIZE_
{
		volatile struct	/* 0x150046E0 */
		{
				FIELD  AF_IMAGE_WD                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_SIZE;	/* CAM_AF_SIZE */

typedef volatile union _CAM_REG_AF_VLD_
{
		volatile struct	/* 0x150046E4 */
		{
				FIELD  AF_VLD_XSTART                         : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AF_VLD_YSTART                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_VLD;	/* CAM_AF_VLD */

typedef volatile union _CAM_REG_AF_BLK_0_
{
		volatile struct	/* 0x150046E8 */
		{
				FIELD  AF_BLK_XSIZE                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_BLK_YSIZE                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_BLK_0;	/* CAM_AF_BLK_0 */

typedef volatile union _CAM_REG_AF_BLK_1_
{
		volatile struct	/* 0x150046EC */
		{
				FIELD  AF_BLK_XNUM                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  AF_BLK_YNUM                           :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_BLK_1;	/* CAM_AF_BLK_1 */

typedef volatile union _CAM_REG_AF_TH_2_
{
		volatile struct	/* 0x150046F0 */
		{
				FIELD  AF_HFLT2_SAT_TH0                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT2_SAT_TH1                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT2_SAT_TH2                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT2_SAT_TH3                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_2;	/* CAM_AF_TH_2 */

typedef volatile union _CAM_REG_FLK_CON_
{
		volatile struct	/* 0x15004770 */
		{
				FIELD  RESERVED                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_FLK_CON;	/* CAM_FLK_CON */

typedef volatile union _CAM_REG_FLK_OFST_
{
		volatile struct	/* 0x15004774 */
		{
				FIELD  FLK_OFST_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_OFST_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FLK_OFST;	/* CAM_FLK_OFST */

typedef volatile union _CAM_REG_FLK_SIZE_
{
		volatile struct	/* 0x15004778 */
		{
				FIELD  FLK_SIZE_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_SIZE_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FLK_SIZE;	/* CAM_FLK_SIZE */

typedef volatile union _CAM_REG_FLK_NUM_
{
		volatile struct	/* 0x1500477C */
		{
				FIELD  FLK_NUM_X                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLK_NUM_Y                             :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FLK_NUM;	/* CAM_FLK_NUM */

typedef volatile union _CAM_REG_RRZ_CTL_
{
		volatile struct	/* 0x150047A0 */
		{
				FIELD  RRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  RRZ_HORI_TBL_SEL                      :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RRZ_VERT_TBL_SEL                      :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_CTL;	/* CAM_RRZ_CTL */

typedef volatile union _CAM_REG_RRZ_IN_IMG_
{
		volatile struct	/* 0x150047A4 */
		{
				FIELD  RRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_IN_IMG;	/* CAM_RRZ_IN_IMG */

typedef volatile union _CAM_REG_RRZ_OUT_IMG_
{
		volatile struct	/* 0x150047A8 */
		{
				FIELD  RRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_OUT_IMG;	/* CAM_RRZ_OUT_IMG */

typedef volatile union _CAM_REG_RRZ_HORI_STEP_
{
		volatile struct	/* 0x150047AC */
		{
				FIELD  RRZ_HORI_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_STEP;	/* CAM_RRZ_HORI_STEP */

typedef volatile union _CAM_REG_RRZ_VERT_STEP_
{
		volatile struct	/* 0x150047B0 */
		{
				FIELD  RRZ_VERT_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_STEP;	/* CAM_RRZ_VERT_STEP */

typedef volatile union _CAM_REG_RRZ_HORI_INT_OFST_
{
		volatile struct	/* 0x150047B4 */
		{
				FIELD  RRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_INT_OFST;	/* CAM_RRZ_HORI_INT_OFST */

typedef volatile union _CAM_REG_RRZ_HORI_SUB_OFST_
{
		volatile struct	/* 0x150047B8 */
		{
				FIELD  RRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_SUB_OFST;	/* CAM_RRZ_HORI_SUB_OFST */

typedef volatile union _CAM_REG_RRZ_VERT_INT_OFST_
{
		volatile struct	/* 0x150047BC */
		{
				FIELD  RRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_INT_OFST;	/* CAM_RRZ_VERT_INT_OFST */

typedef volatile union _CAM_REG_RRZ_VERT_SUB_OFST_
{
		volatile struct	/* 0x150047C0 */
		{
				FIELD  RRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_SUB_OFST;	/* CAM_RRZ_VERT_SUB_OFST */

typedef volatile union _CAM_REG_RRZ_MODE_TH_
{
		volatile struct	/* 0x150047C4 */
		{
				FIELD  RRZ_TH_MD                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RRZ_TH_HI                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RRZ_TH_LO                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RRZ_TH_MD2                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_MODE_TH;	/* CAM_RRZ_MODE_TH */

typedef volatile union _CAM_REG_RRZ_MODE_CTL_
{
		volatile struct	/* 0x150047C8 */
		{
				FIELD  RRZ_PRF_BLD                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RRZ_PRF                               :  2;		/*  9..10, 0x00000600 */
				FIELD  RRZ_BLD_SL                            :  5;		/* 11..15, 0x0000F800 */
				FIELD  RRZ_CR_MODE                           :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_MODE_CTL;	/* CAM_RRZ_MODE_CTL */

typedef volatile union _CAM_REG_RRZ_RLB_AOFST_
{
		volatile struct	/* 0x150047CC */
		{
				FIELD  RRZ_RLB_AOFST                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_RLB_AOFST;	/* CAM_RRZ_RLB_AOFST */

typedef volatile union _CAM_REG_BNR_BPC_CON_
{
		volatile struct	/* 0x15004800 */
		{
				FIELD  BPC_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BPC_LUT_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BPC_TABLE_END_MODE                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BPC_AVG_MODE                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BPC_DTC_MODE                          :  2;		/* 12..13, 0x00003000 */
				FIELD  BPC_CS_MODE                           :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_CRC_MODE                          :  2;		/* 16..17, 0x00030000 */
				FIELD  BPC_EXC                               :  1;		/* 18..18, 0x00040000 */
				FIELD  BPC_BLD_MODE                          :  1;		/* 19..19, 0x00080000 */
				FIELD  BNR_LE_INV_CTL                        :  4;		/* 20..23, 0x00F00000 */
				FIELD  BNR_OSC_COUNT                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BNR_EDGE                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_CON;	/* CAM_BNR_BPC_CON */

typedef volatile union _CAM_REG_BNR_BPC_TH1_
{
		volatile struct	/* 0x15004804 */
		{
				FIELD  BPC_TH_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_Y                              : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_BLD_SLP0                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH1;	/* CAM_BNR_BPC_TH1 */

typedef volatile union _CAM_REG_BNR_BPC_TH2_
{
		volatile struct	/* 0x15004808 */
		{
				FIELD  BPC_TH_UPB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_BLD0                              :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_BLD1                              :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH2;	/* CAM_BNR_BPC_TH2 */

typedef volatile union _CAM_REG_BNR_BPC_TH3_
{
		volatile struct	/* 0x1500480C */
		{
				FIELD  BPC_TH_XA                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_XB                             : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_TH_SLA                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_TH_SLB                            :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH3;	/* CAM_BNR_BPC_TH3 */

typedef volatile union _CAM_REG_BNR_BPC_TH4_
{
		volatile struct	/* 0x15004810 */
		{
				FIELD  BPC_DK_TH_XA                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_DK_TH_XB                          : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_DK_TH_SLA                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_DK_TH_SLB                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH4;	/* CAM_BNR_BPC_TH4 */

typedef volatile union _CAM_REG_BNR_BPC_DTC_
{
		volatile struct	/* 0x15004814 */
		{
				FIELD  BPC_RNG                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  BPC_CS_RNG                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BPC_CT_LV                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_TH_MUL                            :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  BPC_NO_LV                             :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_DTC;	/* CAM_BNR_BPC_DTC */

typedef volatile union _CAM_REG_BNR_BPC_COR_
{
		volatile struct	/* 0x15004818 */
		{
				FIELD  BPC_DIR_MAX                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BPC_DIR_TH                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BPC_RANK_IDXR                         :  3;		/* 16..18, 0x00070000 */
				FIELD  BPC_RANK_IDXG                         :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPC_DIR_TH2                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_COR;	/* CAM_BNR_BPC_COR */

typedef volatile union _CAM_REG_BNR_BPC_TBLI1_
{
		volatile struct	/* 0x1500481C */
		{
				FIELD  BPC_XOFFSET                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YOFFSET                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI1;	/* CAM_BNR_BPC_TBLI1 */

typedef volatile union _CAM_REG_BNR_BPC_TBLI2_
{
		volatile struct	/* 0x15004820 */
		{
				FIELD  BPC_XSIZE                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YSIZE                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI2;	/* CAM_BNR_BPC_TBLI2 */

typedef volatile union _CAM_REG_BNR_BPC_TH1_C_
{
		volatile struct	/* 0x15004824 */
		{
				FIELD  BPC_C_TH_LWB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_Y                            : 12;		/* 12..23, 0x00FFF000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH1_C;	/* CAM_BNR_BPC_TH1_C */

typedef volatile union _CAM_REG_BNR_BPC_TH2_C_
{
		volatile struct	/* 0x15004828 */
		{
				FIELD  BPC_C_TH_UPB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  BPC_RANK_IDXB                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  BPC_BLD_LWB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH2_C;	/* CAM_BNR_BPC_TH2_C */

typedef volatile union _CAM_REG_BNR_BPC_TH3_C_
{
		volatile struct	/* 0x1500482C */
		{
				FIELD  BPC_C_TH_XA                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_XB                           : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_C_TH_SLA                          :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_C_TH_SLB                          :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH3_C;	/* CAM_BNR_BPC_TH3_C */

typedef volatile union _CAM_REG_BNR_NR1_CON_
{
		volatile struct	/* 0x15004830 */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR1_CT_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_NR1_CON;	/* CAM_BNR_NR1_CON */

typedef volatile union _CAM_REG_BNR_NR1_CT_CON_
{
		volatile struct	/* 0x15004834 */
		{
				FIELD  NR1_CT_MD                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR1_CT_MD2                            :  2;		/*  2.. 3, 0x0000000C */
				FIELD  NR1_CT_THRD                           : 10;		/*  4..13, 0x00003FF0 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR1_MBND                              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  NR1_CT_SLOPE                          :  2;		/* 28..29, 0x30000000 */
				FIELD  NR1_CT_DIV                            :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_NR1_CT_CON;	/* CAM_BNR_NR1_CT_CON */

typedef volatile union _CAM_REG_BNR_RSV1_
{
		volatile struct	/* 0x15004838 */
		{
				FIELD  RSV1                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_RSV1;	/* CAM_BNR_RSV1 */

typedef volatile union _CAM_REG_BNR_RSV2_
{
		volatile struct	/* 0x1500483C */
		{
				FIELD  RSV2                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_RSV2;	/* CAM_BNR_RSV2 */

typedef volatile union _CAM_REG_BNR_PDC_CON_
{
		volatile struct	/* 0x15004840 */
		{
				FIELD  PDC_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  PDC_CT                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PDC_MODE                              :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  PDC_OUT                               :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_CON;	/* CAM_BNR_PDC_CON */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L0_
{
		volatile struct	/* 0x15004844 */
		{
				FIELD  PDC_GCF_L00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L0;	/* CAM_BNR_PDC_GAIN_L0 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L1_
{
		volatile struct	/* 0x15004848 */
		{
				FIELD  PDC_GCF_L01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L1;	/* CAM_BNR_PDC_GAIN_L1 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L2_
{
		volatile struct	/* 0x1500484C */
		{
				FIELD  PDC_GCF_L11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L2;	/* CAM_BNR_PDC_GAIN_L2 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L3_
{
		volatile struct	/* 0x15004850 */
		{
				FIELD  PDC_GCF_L30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L3;	/* CAM_BNR_PDC_GAIN_L3 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L4_
{
		volatile struct	/* 0x15004854 */
		{
				FIELD  PDC_GCF_L12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L4;	/* CAM_BNR_PDC_GAIN_L4 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R0_
{
		volatile struct	/* 0x15004858 */
		{
				FIELD  PDC_GCF_R00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R0;	/* CAM_BNR_PDC_GAIN_R0 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R1_
{
		volatile struct	/* 0x1500485C */
		{
				FIELD  PDC_GCF_R01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R1;	/* CAM_BNR_PDC_GAIN_R1 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R2_
{
		volatile struct	/* 0x15004860 */
		{
				FIELD  PDC_GCF_R11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R2;	/* CAM_BNR_PDC_GAIN_R2 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R3_
{
		volatile struct	/* 0x15004864 */
		{
				FIELD  PDC_GCF_R30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R3;	/* CAM_BNR_PDC_GAIN_R3 */

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R4_
{
		volatile struct	/* 0x15004868 */
		{
				FIELD  PDC_GCF_R12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R4;	/* CAM_BNR_PDC_GAIN_R4 */

typedef volatile union _CAM_REG_BNR_PDC_TH_GB_
{
		volatile struct	/* 0x1500486C */
		{
				FIELD  PDC_GTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_BTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_GB;	/* CAM_BNR_PDC_TH_GB */

typedef volatile union _CAM_REG_BNR_PDC_TH_IA_
{
		volatile struct	/* 0x15004870 */
		{
				FIELD  PDC_ITH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_ATH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_IA;	/* CAM_BNR_PDC_TH_IA */

typedef volatile union _CAM_REG_BNR_PDC_TH_HD_
{
		volatile struct	/* 0x15004874 */
		{
				FIELD  PDC_NTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_DTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_HD;	/* CAM_BNR_PDC_TH_HD */

typedef volatile union _CAM_REG_BNR_PDC_SL_
{
		volatile struct	/* 0x15004878 */
		{
				FIELD  PDC_GSL                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  PDC_BSL                               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  PDC_ISL                               :  4;		/*  8..11, 0x00000F00 */
				FIELD  PDC_ASL                               :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_NORM                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_SL;	/* CAM_BNR_PDC_SL */

typedef volatile union _CAM_REG_BNR_PDC_POS_
{
		volatile struct	/* 0x1500487C */
		{
				FIELD  PDC_XCENTER                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDC_YCENTER                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_POS;	/* CAM_BNR_PDC_POS */

typedef volatile union _CAM_REG_PGN_SATU_1_
{
		volatile struct	/* 0x15004880 */
		{
				FIELD  PGN_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_SATU_1;	/* CAM_PGN_SATU_1 */

typedef volatile union _CAM_REG_PGN_SATU_2_
{
		volatile struct	/* 0x15004884 */
		{
				FIELD  PGN_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_SATU_2;	/* CAM_PGN_SATU_2 */

typedef volatile union _CAM_REG_PGN_GAIN_1_
{
		volatile struct	/* 0x15004888 */
		{
				FIELD  PGN_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PGN_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_GAIN_1;	/* CAM_PGN_GAIN_1 */

typedef volatile union _CAM_REG_PGN_GAIN_2_
{
		volatile struct	/* 0x1500488C */
		{
				FIELD  PGN_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PGN_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_GAIN_2;	/* CAM_PGN_GAIN_2 */

typedef volatile union _CAM_REG_PGN_OFST_1_
{
		volatile struct	/* 0x15004890 */
		{
				FIELD  PGN_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_OFST_1;	/* CAM_PGN_OFST_1 */

typedef volatile union _CAM_REG_PGN_OFST_2_
{
		volatile struct	/* 0x15004894 */
		{
				FIELD  PGN_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PGN_OFST_2;	/* CAM_PGN_OFST_2 */

typedef volatile union _CAM_REG_DM_O_BYP_
{
		volatile struct	/* 0x150048A0 */
		{
				FIELD  DM_BYP                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DM_DEBUG_MODE                         :  2;		/*  1.. 2, 0x00000006 */
				FIELD  DM_HF_LSC_MAX_BYP                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DM_FG_MODE                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_BYP;	/* CAM_DM_O_BYP */

typedef volatile union _CAM_REG_DM_O_ED_FLAT_
{
		volatile struct	/* 0x150048A4 */
		{
				FIELD  DM_FLAT_DET_MODE                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DM_STEP_DET_MODE                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  DM_FLAT_TH                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_ED_FLAT;	/* CAM_DM_O_ED_FLAT */

typedef volatile union _CAM_REG_DM_O_ED_NYQ_
{
		volatile struct	/* 0x150048A8 */
		{
				FIELD  DM_NYQ_TH_1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DM_NYQ_TH_2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DM_NYQ_TH_3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DM_HF_NYQ_GAIN                        :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_ED_NYQ;	/* CAM_DM_O_ED_NYQ */

typedef volatile union _CAM_REG_DM_O_ED_STEP_
{
		volatile struct	/* 0x150048AC */
		{
				FIELD  DM_STEP_TH_1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DM_STEP_TH_2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DM_STEP_TH_3                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  7;		/* 24..30, 0x7F000000 */
				FIELD  DM_RB_MODE                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_ED_STEP;	/* CAM_DM_O_ED_STEP */

typedef volatile union _CAM_REG_DM_O_RGB_HF_
{
		volatile struct	/* 0x150048B0 */
		{
				FIELD  DM_HF_CORE_GAIN                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_ROUGH_RB_F                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_ROUGH_RB_D                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_ROUGH_G_F                          :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_ROUGH_G_D                          :  5;		/* 20..24, 0x01F00000 */
				FIELD  DM_RB_MODE_F                          :  2;		/* 25..26, 0x06000000 */
				FIELD  DM_RB_MODE_D                          :  2;		/* 27..28, 0x18000000 */
				FIELD  DM_RB_MODE_HV                         :  2;		/* 29..30, 0x60000000 */
				FIELD  DM_SSG_MODE                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_RGB_HF;	/* CAM_DM_O_RGB_HF */

typedef volatile union _CAM_REG_DM_O_DOT_
{
		volatile struct	/* 0x150048B4 */
		{
				FIELD  DM_DOT_B_TH                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DM_DOT_W_TH                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_DOT;	/* CAM_DM_O_DOT */

typedef volatile union _CAM_REG_DM_O_F1_ACT_
{
		volatile struct	/* 0x150048B8 */
		{
				FIELD  DM_F1_TH_1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  DM_F1_TH_2                            :  9;		/*  9..17, 0x0003FE00 */
				FIELD  DM_F1_SLOPE_1                         :  2;		/* 18..19, 0x000C0000 */
				FIELD  DM_F1_SLOPE_2                         :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F1_ACT;	/* CAM_DM_O_F1_ACT */

typedef volatile union _CAM_REG_DM_O_F2_ACT_
{
		volatile struct	/* 0x150048BC */
		{
				FIELD  DM_F2_TH_1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  DM_F2_TH_2                            :  9;		/*  9..17, 0x0003FE00 */
				FIELD  DM_F2_SLOPE_1                         :  2;		/* 18..19, 0x000C0000 */
				FIELD  DM_F2_SLOPE_2                         :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F2_ACT;	/* CAM_DM_O_F2_ACT */

typedef volatile union _CAM_REG_DM_O_F3_ACT_
{
		volatile struct	/* 0x150048C0 */
		{
				FIELD  DM_F3_TH_1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  DM_F3_TH_2                            :  9;		/*  9..17, 0x0003FE00 */
				FIELD  DM_F3_SLOPE_1                         :  2;		/* 18..19, 0x000C0000 */
				FIELD  DM_F3_SLOPE_2                         :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F3_ACT;	/* CAM_DM_O_F3_ACT */

typedef volatile union _CAM_REG_DM_O_F4_ACT_
{
		volatile struct	/* 0x150048C4 */
		{
				FIELD  DM_F4_TH_1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  DM_F4_TH_2                            :  9;		/*  9..17, 0x0003FE00 */
				FIELD  DM_F4_SLOPE_1                         :  2;		/* 18..19, 0x000C0000 */
				FIELD  DM_F4_SLOPE_2                         :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F4_ACT;	/* CAM_DM_O_F4_ACT */

typedef volatile union _CAM_REG_DM_O_F1_L_
{
		volatile struct	/* 0x150048C8 */
		{
				FIELD  DM_F1_LLUT_Y0                         :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_F1_LLUT_Y1                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_F1_LLUT_Y2                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_F1_LLUT_Y3                         :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_F1_LLUT_Y4                         :  5;		/* 20..24, 0x01F00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F1_L;	/* CAM_DM_O_F1_L */

typedef volatile union _CAM_REG_DM_O_F2_L_
{
		volatile struct	/* 0x150048CC */
		{
				FIELD  DM_F2_LLUT_Y0                         :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_F2_LLUT_Y1                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_F2_LLUT_Y2                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_F2_LLUT_Y3                         :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_F2_LLUT_Y4                         :  5;		/* 20..24, 0x01F00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F2_L;	/* CAM_DM_O_F2_L */

typedef volatile union _CAM_REG_DM_O_F3_L_
{
		volatile struct	/* 0x150048D0 */
		{
				FIELD  DM_F3_LLUT_Y0                         :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_F3_LLUT_Y1                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_F3_LLUT_Y2                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_F3_LLUT_Y3                         :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_F3_LLUT_Y4                         :  5;		/* 20..24, 0x01F00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F3_L;	/* CAM_DM_O_F3_L */

typedef volatile union _CAM_REG_DM_O_F4_L_
{
		volatile struct	/* 0x150048D4 */
		{
				FIELD  DM_F4_LLUT_Y0                         :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_F4_LLUT_Y1                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_F4_LLUT_Y2                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_F4_LLUT_Y3                         :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_F4_LLUT_Y4                         :  5;		/* 20..24, 0x01F00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_F4_L;	/* CAM_DM_O_F4_L */

typedef volatile union _CAM_REG_DM_O_HF_RB_
{
		volatile struct	/* 0x150048D8 */
		{
				FIELD  DM_RB_DIFF_TH                         : 10;		/*  0.. 9, 0x000003FF */
				FIELD  DM_HF_CLIP                            :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_HF_RB;	/* CAM_DM_O_HF_RB */

typedef volatile union _CAM_REG_DM_O_HF_GAIN_
{
		volatile struct	/* 0x150048DC */
		{
				FIELD  DM_HF_GLOBL_GAIN                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_HF_GAIN;	/* CAM_DM_O_HF_GAIN */

typedef volatile union _CAM_REG_DM_O_HF_COMP_
{
		volatile struct	/* 0x150048E0 */
		{
				FIELD  DM_HF_LSC_GAIN0                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  DM_HF_LSC_GAIN1                       :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DM_HF_LSC_GAIN2                       :  4;		/*  8..11, 0x00000F00 */
				FIELD  DM_HF_LSC_GAIN3                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DM_HF_UND_TH                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DM_HF_UND_ACT_TH                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_HF_COMP;	/* CAM_DM_O_HF_COMP */

typedef volatile union _CAM_REG_DM_O_HF_CORIN_TH_
{
		volatile struct	/* 0x150048E4 */
		{
				FIELD  DM_HF_CORIN_TH                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_HF_CORIN_TH;	/* CAM_DM_O_HF_CORIN_TH */

typedef volatile union _CAM_REG_DM_O_ACT_LUT_
{
		volatile struct	/* 0x150048E8 */
		{
				FIELD  DM_ACT_LUT_Y0                         :  5;		/*  0.. 4, 0x0000001F */
				FIELD  DM_ACT_LUT_Y1                         :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  DM_ACT_LUT_Y2                         :  5;		/* 10..14, 0x00007C00 */
				FIELD  DM_ACT_LUT_Y3                         :  5;		/* 15..19, 0x000F8000 */
				FIELD  DM_ACT_LUT_Y4                         :  5;		/* 20..24, 0x01F00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_ACT_LUT;	/* CAM_DM_O_ACT_LUT */

typedef volatile union _CAM_REG_DM_O_SPARE_
{
		volatile struct	/* 0x150048F0 */
		{
				FIELD  DM_MG_BYP                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DM_O_SPARE                            : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_SPARE;	/* CAM_DM_O_SPARE */

typedef volatile union _CAM_REG_DM_O_BB_
{
		volatile struct	/* 0x150048F4 */
		{
				FIELD  DM_BB_TH_1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  DM_BB_TH_2                            :  9;		/* 12..20, 0x001FF000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DM_O_BB;	/* CAM_DM_O_BB */

typedef volatile union _CAM_REG_CCL_GTC_
{
		volatile struct	/* 0x15004910 */
		{
				FIELD  CCL_GLOBL_TH                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCL_GTC;	/* CAM_CCL_GTC */

typedef volatile union _CAM_REG_CCL_ADC_
{
		volatile struct	/* 0x15004914 */
		{
				FIELD  CCL_ADSAT_TH                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CCL_ADSAT_SLOPE                       :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCL_ADC;	/* CAM_CCL_ADC */

typedef volatile union _CAM_REG_CCL_BAC_
{
		volatile struct	/* 0x15004918 */
		{
				FIELD  CCL_B_OFST                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  CCL_B_SLOPE                           :  4;		/* 12..15, 0x0000F000 */
				FIELD  CCL_B_DIFF                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCL_BAC;	/* CAM_CCL_BAC */

typedef volatile union _CAM_REG_G2G_CNV_1_
{
		volatile struct	/* 0x15004920 */
		{
				FIELD  G2G_CNV_00                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_01                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_1;	/* CAM_G2G_CNV_1 */

typedef volatile union _CAM_REG_G2G_CNV_2_
{
		volatile struct	/* 0x15004924 */
		{
				FIELD  G2G_CNV_02                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_2;	/* CAM_G2G_CNV_2 */

typedef volatile union _CAM_REG_G2G_CNV_3_
{
		volatile struct	/* 0x15004928 */
		{
				FIELD  G2G_CNV_10                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_11                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_3;	/* CAM_G2G_CNV_3 */

typedef volatile union _CAM_REG_G2G_CNV_4_
{
		volatile struct	/* 0x1500492C */
		{
				FIELD  G2G_CNV_12                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_4;	/* CAM_G2G_CNV_4 */

typedef volatile union _CAM_REG_G2G_CNV_5_
{
		volatile struct	/* 0x15004930 */
		{
				FIELD  G2G_CNV_20                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_21                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_5;	/* CAM_G2G_CNV_5 */

typedef volatile union _CAM_REG_G2G_CNV_6_
{
		volatile struct	/* 0x15004934 */
		{
				FIELD  G2G_CNV_22                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CNV_6;	/* CAM_G2G_CNV_6 */

typedef volatile union _CAM_REG_G2G_CTRL_
{
		volatile struct	/* 0x15004938 */
		{
				FIELD  G2G_ACC                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  G2G_MOFST_R                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  G2G_POFST_R                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2G_CTRL;	/* CAM_G2G_CTRL */

typedef volatile union _CAM_REG_UNP_OFST_
{
		volatile struct	/* 0x15004948 */
		{
				FIELD  UNP_OFST_STB                          :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  UNP_OFST_EDB                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UNP_OFST;	/* CAM_UNP_OFST */

typedef volatile union _CAM_REG_CPG_SATU_1_
{
		volatile struct	/* 0x15004A00 */
		{
				FIELD  CPG_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_SATU_1;	/* CAM_CPG_SATU_1 */

typedef volatile union _CAM_REG_CPG_SATU_2_
{
		volatile struct	/* 0x15004A04 */
		{
				FIELD  CPG_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_SATU_2;	/* CAM_CPG_SATU_2 */

typedef volatile union _CAM_REG_CPG_GAIN_1_
{
		volatile struct	/* 0x15004A08 */
		{
				FIELD  CPG_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_GAIN_1;	/* CAM_CPG_GAIN_1 */

typedef volatile union _CAM_REG_CPG_GAIN_2_
{
		volatile struct	/* 0x15004A0C */
		{
				FIELD  CPG_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_GAIN_2;	/* CAM_CPG_GAIN_2 */

typedef volatile union _CAM_REG_CPG_OFST_1_
{
		volatile struct	/* 0x15004A10 */
		{
				FIELD  CPG_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_OFST_1;	/* CAM_CPG_OFST_1 */

typedef volatile union _CAM_REG_CPG_OFST_2_
{
		volatile struct	/* 0x15004A14 */
		{
				FIELD  CPG_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_OFST_2;	/* CAM_CPG_OFST_2 */

typedef volatile union _CAM_REG_C42_CON_
{
		volatile struct	/* 0x15004A1C */
		{
				FIELD  C42_FILT_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_C42_CON;	/* CAM_C42_CON */

typedef volatile union _CAM_REG_SEEE_SRK_CTRL_
{
		volatile struct	/* 0x15004AA0 */
		{
				FIELD  rsv_0                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SRK_CTRL;	/* CAM_SEEE_SRK_CTRL */

typedef volatile union _CAM_REG_SEEE_CLIP_CTRL_
{
		volatile struct	/* 0x15004AA4 */
		{
				FIELD  SEEE_OVRSH_CLIP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SEEE_OVRSH_CLIP_STR                   :  3;		/*  1.. 3, 0x0000000E */
				FIELD  SEEE_DOT_REDUC_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEEE_DOT_WF                           :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SEEE_DOT_TH                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_RESP_CLIP                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_CLIP_CTRL;	/* CAM_SEEE_CLIP_CTRL */

typedef volatile union _CAM_REG_SEEE_FLT_CTRL_1_
{
		volatile struct	/* 0x15004AA8 */
		{
				FIELD  SEEE_FLT_TH                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_FLT_AMP                          :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_FLT_CTRL_1;	/* CAM_SEEE_FLT_CTRL_1 */

typedef volatile union _CAM_REG_SEEE_FLT_CTRL_2_
{
		volatile struct	/* 0x15004AAC */
		{
				FIELD  SEEE_FLT_G1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_FLT_G2                           :  6;		/*  8..13, 0x00003F00 */
				FIELD  SEEE_FLT_G3                           :  5;		/* 14..18, 0x0007C000 */
				FIELD  SEEE_FLT_G4                           :  4;		/* 19..22, 0x00780000 */
				FIELD  SEEE_FLT_G5                           :  4;		/* 23..26, 0x07800000 */
				FIELD  SEEE_FLT_G6                           :  3;		/* 27..29, 0x38000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_FLT_CTRL_2;	/* CAM_SEEE_FLT_CTRL_2 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_01_
{
		volatile struct	/* 0x15004AB0 */
		{
				FIELD  SEEE_GLUT_X1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y1                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_01;	/* CAM_SEEE_GLUT_CTRL_01 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_02_
{
		volatile struct	/* 0x15004AB4 */
		{
				FIELD  SEEE_GLUT_X2                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y2                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_02;	/* CAM_SEEE_GLUT_CTRL_02 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_03_
{
		volatile struct	/* 0x15004AB8 */
		{
				FIELD  SEEE_GLUT_X3                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S3                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y3                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_03;	/* CAM_SEEE_GLUT_CTRL_03 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_04_
{
		volatile struct	/* 0x15004ABC */
		{
				FIELD  SEEE_GLUT_X4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S4                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y4                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_04;	/* CAM_SEEE_GLUT_CTRL_04 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_05_
{
		volatile struct	/* 0x15004AC0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S5                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_05;	/* CAM_SEEE_GLUT_CTRL_05 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_06_
{
		volatile struct	/* 0x15004AC4 */
		{
				FIELD  SEEE_GLUT_TH_OVR                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_TH_UND                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_TH_MIN                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_06;	/* CAM_SEEE_GLUT_CTRL_06 */

typedef volatile union _CAM_REG_SEEE_EDTR_CTRL_
{
		volatile struct	/* 0x15004AC8 */
		{
				FIELD  SEEE_EDTR_DIAG_AMP                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  SEEE_EDTR_AMP                         :  6;		/*  3.. 8, 0x000001F8 */
				FIELD  SEEE_EDTR_LV                          :  3;		/*  9..11, 0x00000E00 */
				FIELD  SEEE_EDTR_FLT_MODE                    :  2;		/* 12..13, 0x00003000 */
				FIELD  SEEE_EDTR_FLT_2_EN                    :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_EDTR_CTRL;	/* CAM_SEEE_EDTR_CTRL */

typedef volatile union _CAM_REG_SEEE_OUT_EDGE_CTRL_
{
		volatile struct	/* 0x15004ACC */
		{
				FIELD  SEEE_OUT_EDGE_SEL                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_OUT_EDGE_CTRL;	/* CAM_SEEE_OUT_EDGE_CTRL */

typedef volatile union _CAM_REG_SEEE_SE_Y_CTRL_
{
		volatile struct	/* 0x15004AD0 */
		{
				FIELD  SEEE_SE_CONST_Y_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SEEE_SE_CONST_Y_VAL                   :  8;		/*  1.. 8, 0x000001FE */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  SEEE_SE_YOUT_QBIT                     :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  SEEE_SE_COUT_QBIT                     :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_Y_CTRL;	/* CAM_SEEE_SE_Y_CTRL */

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_1_
{
		volatile struct	/* 0x15004AD4 */
		{
				FIELD  SEEE_SE_HORI_EDGE_SEL                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SEEE_SE_HORI_EDGE_GAIN_A              :  4;		/*  1.. 4, 0x0000001E */
				FIELD  SEEE_SE_HORI_EDGE_GAIN_B              :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  SEEE_SE_HORI_EDGE_GAIN_C              :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  SEEE_SE_VERT_EDGE_SEL                 :  1;		/* 16..16, 0x00010000 */
				FIELD  SEEE_SE_VERT_EDGE_GAIN_A              :  4;		/* 17..20, 0x001E0000 */
				FIELD  SEEE_SE_VERT_EDGE_GAIN_B              :  5;		/* 21..25, 0x03E00000 */
				FIELD  SEEE_SE_VERT_EDGE_GAIN_C              :  5;		/* 26..30, 0x7C000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_EDGE_CTRL_1;	/* CAM_SEEE_SE_EDGE_CTRL_1 */

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_2_
{
		volatile struct	/* 0x15004AD8 */
		{
				FIELD  SEEE_SE_EDGE_DET_GAIN                 :  5;		/*  0.. 4, 0x0000001F */
				FIELD  SEEE_SE_BOSS_IN_SEL                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SEEE_SE_BOSS_GAIN_OFF                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_EDGE_CTRL_2;	/* CAM_SEEE_SE_EDGE_CTRL_2 */

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_3_
{
		volatile struct	/* 0x15004ADC */
		{
				FIELD  SEEE_SE_CONVT_FORCE_EN                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SEEE_SE_CONVT_CORE_TH                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SEEE_SE_CONVT_E_TH                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_SE_CONVT_SLOPE_SEL               :  1;		/* 16..16, 0x00010000 */
				FIELD  SEEE_SE_OIL_EN                        :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_EDGE_CTRL_3;	/* CAM_SEEE_SE_EDGE_CTRL_3 */

typedef volatile union _CAM_REG_SEEE_SE_SPECL_CTRL_
{
		volatile struct	/* 0x15004AE0 */
		{
				FIELD  SEEE_SE_SPECL_HALF_MODE               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  SEEE_SE_SPECL_ABS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SEEE_SE_SPECL_INV                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SEEE_SE_SPECL_GAIN                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  SEEE_SE_KNEE_SEL                      :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_SPECL_CTRL;	/* CAM_SEEE_SE_SPECL_CTRL */

typedef volatile union _CAM_REG_SEEE_SE_CORE_CTRL_1_
{
		volatile struct	/* 0x15004AE4 */
		{
				FIELD  SEEE_SE_CORE_HORI_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  SEEE_SE_CORE_HORI_SUP                 :  2;		/*  7.. 8, 0x00000180 */
				FIELD  SEEE_SE_CORE_HORI_X2                  :  8;		/*  9..16, 0x0001FE00 */
				FIELD  SEEE_SE_CORE_HORI_SDN                 :  2;		/* 17..18, 0x00060000 */
				FIELD  SEEE_SE_CORE_HORI_Y5                  :  6;		/* 19..24, 0x01F80000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_CORE_CTRL_1;	/* CAM_SEEE_SE_CORE_CTRL_1 */

typedef volatile union _CAM_REG_SEEE_SE_CORE_CTRL_2_
{
		volatile struct	/* 0x15004AE8 */
		{
				FIELD  SEEE_SE_CORE_VERT_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  SEEE_SE_CORE_VERT_SUP                 :  2;		/*  7.. 8, 0x00000180 */
				FIELD  SEEE_SE_CORE_VERT_X2                  :  8;		/*  9..16, 0x0001FE00 */
				FIELD  SEEE_SE_CORE_VERT_SDN                 :  2;		/* 17..18, 0x00060000 */
				FIELD  SEEE_SE_CORE_VERT_Y5                  :  6;		/* 19..24, 0x01F80000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_SE_CORE_CTRL_2;	/* CAM_SEEE_SE_CORE_CTRL_2 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_07_
{
		volatile struct	/* 0x15004AEC */
		{
				FIELD  SEEE_GLUT_X1_1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S1_1                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_S2_1                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_GLUT_LLINK_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_07;	/* CAM_SEEE_GLUT_CTRL_07 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_08_
{
		volatile struct	/* 0x15004AF0 */
		{
				FIELD  SEEE_GLUT_X1_2                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S1_2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_S2_2                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_08;	/* CAM_SEEE_GLUT_CTRL_08 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_09_
{
		volatile struct	/* 0x15004AF4 */
		{
				FIELD  SEEE_GLUT_X1_3                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S1_3                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_S2_3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_09;	/* CAM_SEEE_GLUT_CTRL_09 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_10_
{
		volatile struct	/* 0x15004AF8 */
		{
				FIELD  SEEE_GLUT_TH_OVR_1                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_TH_UND_1                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_TH_OVR_2                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_GLUT_TH_UND_2                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_10;	/* CAM_SEEE_GLUT_CTRL_10 */

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_11_
{
		volatile struct	/* 0x15004AFC */
		{
				FIELD  SEEE_GLUT_TH_OVR_3                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_TH_UND_3                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SEEE_GLUT_CTRL_11;	/* CAM_SEEE_GLUT_CTRL_11 */

typedef volatile union _CAM_REG_CRZ_CONTROL_
{
		volatile struct	/* 0x15004B00 */
		{
				FIELD  CRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  CRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CRZ_HORI_ALGO                         :  2;		/*  5.. 6, 0x00000060 */
				FIELD  CRZ_VERT_ALGO                         :  2;		/*  7.. 8, 0x00000180 */
				FIELD  CRZ_DER_EN                            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CRZ_TRNC_HORI                         :  3;		/* 10..12, 0x00001C00 */
				FIELD  CRZ_TRNC_VERT                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  CRZ_HORI_TBL_SEL                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  CRZ_VERT_TBL_SEL                      :  5;		/* 21..25, 0x03E00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_CONTROL;	/* CAM_CRZ_CONTROL */

typedef volatile union _CAM_REG_CRZ_IN_IMG_
{
		volatile struct	/* 0x15004B04 */
		{
				FIELD  CRZ_IN_WD                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CRZ_IN_HT                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_IN_IMG;	/* CAM_CRZ_IN_IMG */

typedef volatile union _CAM_REG_CRZ_OUT_IMG_
{
		volatile struct	/* 0x15004B08 */
		{
				FIELD  CRZ_OUT_WD                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CRZ_OUT_HT                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_OUT_IMG;	/* CAM_CRZ_OUT_IMG */

typedef volatile union _CAM_REG_CRZ_HORI_STEP_
{
		volatile struct	/* 0x15004B0C */
		{
				FIELD  CRZ_HORI_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_HORI_STEP;	/* CAM_CRZ_HORI_STEP */

typedef volatile union _CAM_REG_CRZ_VERT_STEP_
{
		volatile struct	/* 0x15004B10 */
		{
				FIELD  CRZ_VERT_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_VERT_STEP;	/* CAM_CRZ_VERT_STEP */

typedef volatile union _CAM_REG_CRZ_LUMA_HORI_INT_OFST_
{
		volatile struct	/* 0x15004B14 */
		{
				FIELD  CRZ_LUMA_HORI_INT_OFST                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_LUMA_HORI_INT_OFST;	/* CAM_CRZ_LUMA_HORI_INT_OFST */

typedef volatile union _CAM_REG_CRZ_LUMA_HORI_SUB_OFST_
{
		volatile struct	/* 0x15004B18 */
		{
				FIELD  CRZ_LUMA_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_LUMA_HORI_SUB_OFST;	/* CAM_CRZ_LUMA_HORI_SUB_OFST */

typedef volatile union _CAM_REG_CRZ_LUMA_VERT_INT_OFST_
{
		volatile struct	/* 0x15004B1C */
		{
				FIELD  CRZ_LUMA_VERT_INT_OFST                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_LUMA_VERT_INT_OFST;	/* CAM_CRZ_LUMA_VERT_INT_OFST */

typedef volatile union _CAM_REG_CRZ_LUMA_VERT_SUB_OFST_
{
		volatile struct	/* 0x15004B20 */
		{
				FIELD  CRZ_LUMA_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_LUMA_VERT_SUB_OFST;	/* CAM_CRZ_LUMA_VERT_SUB_OFST */

typedef volatile union _CAM_REG_CRZ_CHRO_HORI_INT_OFST_
{
		volatile struct	/* 0x15004B24 */
		{
				FIELD  CRZ_CHRO_HORI_INT_OFST                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_CHRO_HORI_INT_OFST;	/* CAM_CRZ_CHRO_HORI_INT_OFST */

typedef volatile union _CAM_REG_CRZ_CHRO_HORI_SUB_OFST_
{
		volatile struct	/* 0x15004B28 */
		{
				FIELD  CRZ_CHRO_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_CHRO_HORI_SUB_OFST;	/* CAM_CRZ_CHRO_HORI_SUB_OFST */

typedef volatile union _CAM_REG_CRZ_CHRO_VERT_INT_OFST_
{
		volatile struct	/* 0x15004B2C */
		{
				FIELD  CRZ_CHRO_VERT_INT_OFST                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_CHRO_VERT_INT_OFST;	/* CAM_CRZ_CHRO_VERT_INT_OFST */

typedef volatile union _CAM_REG_CRZ_CHRO_VERT_SUB_OFST_
{
		volatile struct	/* 0x15004B30 */
		{
				FIELD  CRZ_CHRO_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_CHRO_VERT_SUB_OFST;	/* CAM_CRZ_CHRO_VERT_SUB_OFST */

typedef volatile union _CAM_REG_CRZ_DER_1_
{
		volatile struct	/* 0x15004B34 */
		{
				FIELD  CRZ_SPARE_1V                          :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  CRZ_SPARE_1H                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_DER_1;	/* CAM_CRZ_DER_1 */

typedef volatile union _CAM_REG_CRZ_DER_2_
{
		volatile struct	/* 0x15004B38 */
		{
				FIELD  CRZ_SPARE_2V                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CRZ_SPARE_2H                          :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CRZ_DER_2;	/* CAM_CRZ_DER_2 */

typedef volatile union _CAM_REG_G2C_CONV_0A_
{
		volatile struct	/* 0x15004BA0 */
		{
				FIELD  G2C_CNV_00                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_01                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_0A;	/* CAM_G2C_CONV_0A */

typedef volatile union _CAM_REG_G2C_CONV_0B_
{
		volatile struct	/* 0x15004BA4 */
		{
				FIELD  G2C_CNV_02                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_Y_OFST                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_0B;	/* CAM_G2C_CONV_0B */

typedef volatile union _CAM_REG_G2C_CONV_1A_
{
		volatile struct	/* 0x15004BA8 */
		{
				FIELD  G2C_CNV_10                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_11                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_1A;	/* CAM_G2C_CONV_1A */

typedef volatile union _CAM_REG_G2C_CONV_1B_
{
		volatile struct	/* 0x15004BAC */
		{
				FIELD  G2C_CNV_12                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_U_OFST                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_1B;	/* CAM_G2C_CONV_1B */

typedef volatile union _CAM_REG_G2C_CONV_2A_
{
		volatile struct	/* 0x15004BB0 */
		{
				FIELD  G2C_CNV_20                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_21                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_2A;	/* CAM_G2C_CONV_2A */

typedef volatile union _CAM_REG_G2C_CONV_2B_
{
		volatile struct	/* 0x15004BB4 */
		{
				FIELD  G2C_CNV_22                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_V_OFST                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_CONV_2B;	/* CAM_G2C_CONV_2B */

typedef volatile union _CAM_REG_G2C_SHADE_CON_1_
{
		volatile struct	/* 0x15004BB8 */
		{
				FIELD  G2C_SHADE_VAR                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  G2C_SHADE_P0                          : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  G2C_SHADE_EN                          :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_SHADE_CON_1;	/* CAM_G2C_SHADE_CON_1 */

typedef volatile union _CAM_REG_G2C_SHADE_CON_2_
{
		volatile struct	/* 0x15004BBC */
		{
				FIELD  G2C_SHADE_P1                          : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  G2C_SHADE_P2                          : 11;		/* 12..22, 0x007FF000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_SHADE_CON_2;	/* CAM_G2C_SHADE_CON_2 */

typedef volatile union _CAM_REG_G2C_SHADE_CON_3_
{
		volatile struct	/* 0x15004BC0 */
		{
				FIELD  G2C_SHADE_UB                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_SHADE_CON_3;	/* CAM_G2C_SHADE_CON_3 */

typedef volatile union _CAM_REG_G2C_SHADE_TAR_
{
		volatile struct	/* 0x15004BC4 */
		{
				FIELD  G2C_SHADE_XMID                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2C_SHADE_YMID                        : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_SHADE_TAR;	/* CAM_G2C_SHADE_TAR */

typedef volatile union _CAM_REG_G2C_SHADE_SP_
{
		volatile struct	/* 0x15004BC8 */
		{
				FIELD  G2C_SHADE_XSP                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2C_SHADE_YSP                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_G2C_SHADE_SP;	/* CAM_G2C_SHADE_SP */

typedef volatile union _CAM_REG_DBS_SIGMA_
{
		volatile struct	/* 0x15004C00 */
		{
				FIELD  DBS_OFST                              : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_SL                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SIGMA;	/* CAM_DBS_SIGMA */

typedef volatile union _CAM_REG_DBS_BSTBL_0_
{
		volatile struct	/* 0x15004C04 */
		{
				FIELD  DBS_BIAS_Y0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_0;	/* CAM_DBS_BSTBL_0 */

typedef volatile union _CAM_REG_DBS_BSTBL_1_
{
		volatile struct	/* 0x15004C08 */
		{
				FIELD  DBS_BIAS_Y4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_1;	/* CAM_DBS_BSTBL_1 */

typedef volatile union _CAM_REG_DBS_BSTBL_2_
{
		volatile struct	/* 0x15004C0C */
		{
				FIELD  DBS_BIAS_Y8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_2;	/* CAM_DBS_BSTBL_2 */

typedef volatile union _CAM_REG_DBS_BSTBL_3_
{
		volatile struct	/* 0x15004C10 */
		{
				FIELD  DBS_BIAS_Y12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_3;	/* CAM_DBS_BSTBL_3 */

typedef volatile union _CAM_REG_DBS_CTL_
{
		volatile struct	/* 0x15004C14 */
		{
				FIELD  DBS_HDR_GNP                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DBS_SL_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  DBS_LE_INV_CTL                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  DBS_EDGE                              :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_HDR_GAIN                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_HDR_GAIN2                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_CTL;	/* CAM_DBS_CTL */

typedef volatile union _CAM_REG_DBS_CTL_2_
{
		volatile struct	/* 0x15004C18 */
		{
				FIELD  DBS_HDR_OSCTH                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_CTL_2;	/* CAM_DBS_CTL_2 */

typedef volatile union _CAM_REG_DBS_SIGMA_2_
{
		volatile struct	/* 0x15004C1C */
		{
				FIELD  DBS_MUL_B                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_MUL_GB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_MUL_GR                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_MUL_R                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SIGMA_2;	/* CAM_DBS_SIGMA_2 */

typedef volatile union _CAM_REG_DBS_YGN_
{
		volatile struct	/* 0x15004C20 */
		{
				FIELD  DBS_YGN_B                             :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  DBS_YGN_GB                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  DBS_YGN_GR                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DBS_YGN_R                             :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_YGN;	/* CAM_DBS_YGN */

typedef volatile union _CAM_REG_DBS_SL_Y12_
{
		volatile struct	/* 0x15004C24 */
		{
				FIELD  DBS_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_Y12;	/* CAM_DBS_SL_Y12 */

typedef volatile union _CAM_REG_DBS_SL_Y34_
{
		volatile struct	/* 0x15004C28 */
		{
				FIELD  DBS_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_Y34;	/* CAM_DBS_SL_Y34 */

typedef volatile union _CAM_REG_DBS_SL_G12_
{
		volatile struct	/* 0x15004C2C */
		{
				FIELD  DBS_SL_G1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_G12;	/* CAM_DBS_SL_G12 */

typedef volatile union _CAM_REG_DBS_SL_G34_
{
		volatile struct	/* 0x15004C30 */
		{
				FIELD  DBS_SL_G3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_G34;	/* CAM_DBS_SL_G34 */

typedef volatile union _CAM_REG_MDP_CROP_X_
{
		volatile struct	/* 0x15004D10 */
		{
				FIELD  MDP_CROP_STR_X                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  MDP_CROP_END_X                        : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_MDP_CROP_X;	/* CAM_MDP_CROP_X */

typedef volatile union _CAM_REG_MDP_CROP_Y_
{
		volatile struct	/* 0x15004D20 */
		{
				FIELD  MDP_CROP_STR_Y                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  MDP_CROP_END_Y                        : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_MDP_CROP_Y;	/* CAM_MDP_CROP_Y */

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL1_
{
		volatile struct	/* 0x15004DC0 */
		{
				FIELD  EIS_OP_HORI                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  EIS_OP_VERT                           :  3;		/*  3.. 5, 0x00000038 */
				FIELD  EIS_SUBG_EN                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  EIS_NUM_HRP                           :  5;		/*  8..12, 0x00001F00 */
				FIELD  EIS_KNEE_2                            :  4;		/* 13..16, 0x0001E000 */
				FIELD  EIS_KNEE_1                            :  4;		/* 17..20, 0x001E0000 */
				FIELD  EIS_NUM_VRP                           :  4;		/* 21..24, 0x01E00000 */
				FIELD  EIS_NUM_HWIN                          :  3;		/* 25..27, 0x0E000000 */
				FIELD  EIS_NUM_VWIN                          :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_PREP_ME_CTRL1;	/* CAM_EIS_PREP_ME_CTRL1 */

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL2_
{
		volatile struct	/* 0x15004DC4 */
		{
				FIELD  EIS_PROC_GAIN                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  EIS_HORI_SHR                          :  3;		/*  2.. 4, 0x0000001C */
				FIELD  EIS_VERT_SHR                          :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  EIS_DC_DL                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  EIS_RP_MODI                           :  1;		/* 14..14, 0x00004000 */
				FIELD  EIS_FIRST_FRM                         :  1;		/* 15..15, 0x00008000 */
				FIELD  EIS_SPARE                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_PREP_ME_CTRL2;	/* CAM_EIS_PREP_ME_CTRL2 */

typedef volatile union _CAM_REG_EIS_LMV_TH_
{
		volatile struct	/* 0x15004DC8 */
		{
				FIELD  LMV_TH_Y_SURROUND                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LMV_TH_Y_CENTER                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LMV_TH_X_SOURROUND                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LMV_TH_X_CENTER                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_LMV_TH;	/* CAM_EIS_LMV_TH */

typedef volatile union _CAM_REG_EIS_FL_OFFSET_
{
		volatile struct	/* 0x15004DCC */
		{
				FIELD  EIS_WIN_VOFST                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HOFST                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_FL_OFFSET;	/* CAM_EIS_FL_OFFSET */

typedef volatile union _CAM_REG_EIS_MB_OFFSET_
{
		volatile struct	/* 0x15004DD0 */
		{
				FIELD  EIS_RP_VOFST                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_RP_HOFST                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_MB_OFFSET;	/* CAM_EIS_MB_OFFSET */

typedef volatile union _CAM_REG_EIS_MB_INTERVAL_
{
		volatile struct	/* 0x15004DD4 */
		{
				FIELD  EIS_WIN_VSIZE                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HSIZE                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_MB_INTERVAL;	/* CAM_EIS_MB_INTERVAL */

typedef volatile union _CAM_REG_EIS_GMV_
{
		volatile struct	/* 0x15004DD8 */
		{
				FIELD  GMV_Y                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  GMV_X                                 : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_GMV;	/* CAM_EIS_GMV */

typedef volatile union _CAM_REG_EIS_ERR_CTRL_
{
		volatile struct	/* 0x15004DDC */
		{
				FIELD  ERR_STATUS                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  CHK_SUM_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  CHK_SUM_OUT                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ERR_MASK                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  CLEAR_ERR                             :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_ERR_CTRL;	/* CAM_EIS_ERR_CTRL */

typedef volatile union _CAM_REG_EIS_IMAGE_CTRL_
{
		volatile struct	/* 0x15004DE0 */
		{
				FIELD  HEIGHT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  WIDTH                                 : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  2;		/* 29..30, 0x60000000 */
				FIELD  PIPE_MODE                             :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EIS_IMAGE_CTRL;	/* CAM_EIS_IMAGE_CTRL */

typedef volatile union _CAM_REG_DMX_CTL_
{
		volatile struct	/* 0x15004E00 */
		{
				FIELD  DMX_IHDR_SEP_MODE                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DMX_IHDR_ORD                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 11;		/*  5..15, 0x0000FFE0 */
				FIELD  DMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  DMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_CTL;	/* CAM_DMX_CTL */

typedef volatile union _CAM_REG_DMX_CROP_
{
		volatile struct	/* 0x15004E04 */
		{
				FIELD  DMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_CROP;	/* CAM_DMX_CROP */

typedef volatile union _CAM_REG_DMX_VSIZE_
{
		volatile struct	/* 0x15004E08 */
		{
				FIELD  DMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_VSIZE;	/* CAM_DMX_VSIZE */

typedef volatile union _CAM_REG_BMX_CTL_
{
		volatile struct	/* 0x15004E10 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BMX_SINGLE_MODE_1                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BMX_SINGLE_MODE_2                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  BMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  BMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_CTL;	/* CAM_BMX_CTL */

typedef volatile union _CAM_REG_BMX_CROP_
{
		volatile struct	/* 0x15004E14 */
		{
				FIELD  BMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  BMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_CROP;	/* CAM_BMX_CROP */

typedef volatile union _CAM_REG_BMX_VSIZE_
{
		volatile struct	/* 0x15004E18 */
		{
				FIELD  BMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_VSIZE;	/* CAM_BMX_VSIZE */

typedef volatile union _CAM_REG_RMX_CTL_
{
		volatile struct	/* 0x15004E20 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RMX_SINGLE_MODE_1                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  RMX_SINGLE_MODE_2                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  RMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_CTL;	/* CAM_RMX_CTL */

typedef volatile union _CAM_REG_RMX_CROP_
{
		volatile struct	/* 0x15004E24 */
		{
				FIELD  RMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_CROP;	/* CAM_RMX_CROP */

typedef volatile union _CAM_REG_RMX_VSIZE_
{
		volatile struct	/* 0x15004E28 */
		{
				FIELD  RMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_VSIZE;	/* CAM_RMX_VSIZE */

typedef volatile union _CAM_REG_SL2_CEN_
{
		volatile struct	/* 0x15004F40 */
		{
				FIELD  SL2_CENTR_X                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SL2_CENTR_Y                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_CEN;	/* CAM_SL2_CEN */

typedef volatile union _CAM_REG_SL2_MAX0_RR_
{
		volatile struct	/* 0x15004F44 */
		{
				FIELD  SL2_RR_0                              : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_MAX0_RR;	/* CAM_SL2_MAX0_RR */

typedef volatile union _CAM_REG_SL2_MAX1_RR_
{
		volatile struct	/* 0x15004F48 */
		{
				FIELD  SL2_RR_1                              : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_MAX1_RR;	/* CAM_SL2_MAX1_RR */

typedef volatile union _CAM_REG_SL2_MAX2_RR_
{
		volatile struct	/* 0x15004F4C */
		{
				FIELD  SL2_RR_2                              : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_MAX2_RR;	/* CAM_SL2_MAX2_RR */

typedef volatile union _CAM_REG_SL2_HRZ_
{
		volatile struct	/* 0x15004F50 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_HRZ;	/* CAM_SL2_HRZ */

typedef volatile union _CAM_REG_SL2_XOFF_
{
		volatile struct	/* 0x15004F54 */
		{
				FIELD  SL2_X_OFST                            : 27;		/*  0..26, 0x07FFFFFF */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_XOFF;	/* CAM_SL2_XOFF */

typedef volatile union _CAM_REG_SL2_YOFF_
{
		volatile struct	/* 0x15004F58 */
		{
				FIELD  SL2_Y_OFST                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2_YOFF;	/* CAM_SL2_YOFF */

typedef volatile union _CAM_REG_NSL2A_CEN_
{
		volatile struct	/* 0x15004F60 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_CEN;	/* CAM_NSL2A_CEN */

typedef volatile union _CAM_REG_NSL2A_RR_CON0_
{
		volatile struct	/* 0x15004F64 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_RR_CON0;	/* CAM_NSL2A_RR_CON0 */

typedef volatile union _CAM_REG_NSL2A_RR_CON1_
{
		volatile struct	/* 0x15004F68 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_RR_CON1;	/* CAM_NSL2A_RR_CON1 */

typedef volatile union _CAM_REG_NSL2A_GAIN_
{
		volatile struct	/* 0x15004F6C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_GAIN;	/* CAM_NSL2A_GAIN */

typedef volatile union _CAM_REG_NSL2A_RZ_
{
		volatile struct	/* 0x15004F70 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_RZ;	/* CAM_NSL2A_RZ */

typedef volatile union _CAM_REG_NSL2A_XOFF_
{
		volatile struct	/* 0x15004F74 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_XOFF;	/* CAM_NSL2A_XOFF */

typedef volatile union _CAM_REG_NSL2A_YOFF_
{
		volatile struct	/* 0x15004F78 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_YOFF;	/* CAM_NSL2A_YOFF */

typedef volatile union _CAM_REG_NSL2A_SLP_CON0_
{
		volatile struct	/* 0x15004F7C */
		{
				FIELD  SL2_SLP_1                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SL2_SLP_2                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_SLP_CON0;	/* CAM_NSL2A_SLP_CON0 */

typedef volatile union _CAM_REG_NSL2A_SLP_CON1_
{
		volatile struct	/* 0x15004F80 */
		{
				FIELD  SL2_SLP_3                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SL2_SLP_4                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_SLP_CON1;	/* CAM_NSL2A_SLP_CON1 */

typedef volatile union _CAM_REG_NSL2A_SIZE_
{
		volatile struct	/* 0x15004F84 */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_NSL2A_SIZE;	/* CAM_NSL2A_SIZE */

typedef volatile union _CAM_REG_SL2C_CEN_
{
		volatile struct	/* 0x15004FC0 */
		{
				FIELD  SL2C_CENTR_X                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SL2C_CENTR_Y                          : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_CEN;	/* CAM_SL2C_CEN */

typedef volatile union _CAM_REG_SL2C_MAX0_RR_
{
		volatile struct	/* 0x15004FC4 */
		{
				FIELD  SL2C_RR_0                             : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_MAX0_RR;	/* CAM_SL2C_MAX0_RR */

typedef volatile union _CAM_REG_SL2C_MAX1_RR_
{
		volatile struct	/* 0x15004FC8 */
		{
				FIELD  SL2C_RR_1                             : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_MAX1_RR;	/* CAM_SL2C_MAX1_RR */

typedef volatile union _CAM_REG_SL2C_MAX2_RR_
{
		volatile struct	/* 0x15004FCC */
		{
				FIELD  SL2C_RR_2                             : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_MAX2_RR;	/* CAM_SL2C_MAX2_RR */

typedef volatile union _CAM_REG_SL2C_HRZ_
{
		volatile struct	/* 0x15004FD0 */
		{
				FIELD  SL2C_HRZ_COMP                         : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_HRZ;	/* CAM_SL2C_HRZ */

typedef volatile union _CAM_REG_SL2C_XOFF_
{
		volatile struct	/* 0x15004FD4 */
		{
				FIELD  SL2C_X_OFST                           : 27;		/*  0..26, 0x07FFFFFF */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_XOFF;	/* CAM_SL2C_XOFF */

typedef volatile union _CAM_REG_SL2C_YOFF_
{
		volatile struct	/* 0x15004FD8 */
		{
				FIELD  SL2C_Y_OFST                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2C_YOFF;	/* CAM_SL2C_YOFF */

typedef volatile union _CAM_REG_GGM_LUT_RB_
{
		volatile struct	/* 0x15005000 ~ 0x1500523F */
		{
				FIELD  GGM_R                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  GGM_B                                 : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_GGM_LUT_RB;	/* CAM_GGM_LUT_RB */

typedef volatile union _CAM_REG_GGM_LUT_G_
{
		volatile struct	/* 0x15005240 ~ 0x1500547F */
		{
				FIELD  GGM_G                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_GGM_LUT_G;	/* CAM_GGM_LUT_G */

typedef volatile union _CAM_REG_GGM_CTRL_
{
		volatile struct	/* 0x15005480 */
		{
				FIELD  GGM_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_GGM_CTRL;	/* CAM_GGM_CTRL */

typedef volatile union _CAM_REG_PCA_TBL_
{
		volatile struct	/* 0x15005800 ~ 0x15005D9F */
		{
				FIELD  PCA_LUMA_GAIN                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  PCA_SAT_GAIN                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  PCA_HUE_SHIFT                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PCA_TBL;	/* CAM_PCA_TBL */

typedef volatile union _CAM_REG_PCA_CON1_
{
		volatile struct	/* 0x15005E00 */
		{
				FIELD  PCA_LUT_360                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  RSV                                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PCA_CON1;	/* CAM_PCA_CON1 */

typedef volatile union _CAM_REG_PCA_CON2_
{
		volatile struct	/* 0x15005E04 */
		{
				FIELD  PCA_C_TH                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RSV                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  PCA_S_TH_EN                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PCA_S_TH                              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  PCA_Y2YLUT_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PCA_CON2;	/* CAM_PCA_CON2 */

typedef volatile union _CAM_REG_TILE_RING_CON1_
{
		volatile struct	/* 0x15005FF0 */
		{
				FIELD  TILE_IRQ_MODE                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RING_BUFFER_MCU_NO                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RING_BUFFER_MCU_Y_SIZE                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  EIS_FE_ONE_SEL                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TILE_RING_CON1;	/* CAM_TILE_RING_CON1 */

typedef volatile union _CAM_REG_CTL_IMGI_SIZE_
{
		volatile struct	/* 0x15005FF4 */
		{
				FIELD  IMGI_YSIZE                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  IMGI_XSIZE                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_IMGI_SIZE;	/* CAM_CTL_IMGI_SIZE */

typedef volatile union _CAM_REG_TG2_SEN_MODE_
{
		volatile struct	/* 0x15006410 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_SEN_MODE;	/* CAM_TG2_SEN_MODE */

typedef volatile union _CAM_REG_TG2_VF_CON_
{
		volatile struct	/* 0x15006414 */
		{
				FIELD  VFDATA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SINGLE_MODE                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FR_CON                                :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SP_DELAY                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SPDELAY_MODE                          :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_VF_CON;	/* CAM_TG2_VF_CON */

typedef volatile union _CAM_REG_TG2_SEN_GRAB_PXL_
{
		volatile struct	/* 0x15006418 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_SEN_GRAB_PXL;	/* CAM_TG2_SEN_GRAB_PXL */

typedef volatile union _CAM_REG_TG2_SEN_GRAB_LIN_
{
		volatile struct	/* 0x1500641C */
		{
				FIELD  LIN_S                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LIN_E                                 : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_SEN_GRAB_LIN;	/* CAM_TG2_SEN_GRAB_LIN */

typedef volatile union _CAM_REG_TG2_PATH_CFG_
{
		volatile struct	/* 0x15006420 */
		{
				FIELD  SEN_IN_LSB                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  JPGINF_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MEMIN_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  JPG_LINEND_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DB_LOAD_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DB_LOAD_SRC                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DB_LOAD_VSPOL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_PATH_CFG;	/* CAM_TG2_PATH_CFG */

typedef volatile union _CAM_REG_TG2_MEMIN_CTL_
{
		volatile struct	/* 0x15006424 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  FBC_CNT                               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_MEMIN_CTL;	/* CAM_TG2_MEMIN_CTL */

typedef volatile union _CAM_REG_TG2_INT1_
{
		volatile struct	/* 0x15006428 */
		{
				FIELD  TG_INT1_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_INT1;	/* CAM_TG2_INT1 */

typedef volatile union _CAM_REG_TG2_INT2_
{
		volatile struct	/* 0x1500642C */
		{
				FIELD  TG_INT2_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_INT2;	/* CAM_TG2_INT2 */

typedef volatile union _CAM_REG_TG2_SOF_CNT_
{
		volatile struct	/* 0x15006430 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_SOF_CNT;	/* CAM_TG2_SOF_CNT */

typedef volatile union _CAM_REG_TG2_SOT_CNT_
{
		volatile struct	/* 0x15006434 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_SOT_CNT;	/* CAM_TG2_SOT_CNT */

typedef volatile union _CAM_REG_TG2_EOT_CNT_
{
		volatile struct	/* 0x15006438 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_EOT_CNT;	/* CAM_TG2_EOT_CNT */

typedef volatile union _CAM_REG_TG2_ERR_CTL_
{
		volatile struct	/* 0x1500643C */
		{
				FIELD  GRAB_ERR_FLIMIT_NO                    :  4;		/*  0.. 3, 0x0000000F */
				FIELD  GRAB_ERR_FLIMIT_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  GRAB_ERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DBG_SRC_SEL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_ERR_CTL;	/* CAM_TG2_ERR_CTL */

typedef volatile union _CAM_REG_TG2_DAT_NO_
{
		volatile struct	/* 0x15006440 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_DAT_NO;	/* CAM_TG2_DAT_NO */

typedef volatile union _CAM_REG_TG2_FRM_CNT_ST_
{
		volatile struct	/* 0x15006444 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_FRM_CNT_ST;	/* CAM_TG2_FRM_CNT_ST */

typedef volatile union _CAM_REG_TG2_FRMSIZE_ST_
{
		volatile struct	/* 0x15006448 */
		{
				FIELD  LINE_CNT                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_FRMSIZE_ST;	/* CAM_TG2_FRMSIZE_ST */

typedef volatile union _CAM_REG_TG2_INTER_ST_
{
		volatile struct	/* 0x1500644C */
		{
				FIELD  SYN_VF_DATA_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  OUT_RDY                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OUT_REQ                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  TG_CAM_CS                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CAM_FRM_CNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG2_INTER_ST;	/* CAM_TG2_INTER_ST */

typedef volatile union _CAM_REG_OBC_D_OFFST0_
{
		volatile struct	/* 0x15006500 */
		{
				FIELD  OBC_D_OFST_B                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_OFFST0;	/* CAM_OBC_D_OFFST0 */

typedef volatile union _CAM_REG_OBC_D_OFFST1_
{
		volatile struct	/* 0x15006504 */
		{
				FIELD  OBC_D_OFST_GR                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_OFFST1;	/* CAM_OBC_D_OFFST1 */

typedef volatile union _CAM_REG_OBC_D_OFFST2_
{
		volatile struct	/* 0x15006508 */
		{
				FIELD  OBC_D_OFST_GB                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_OFFST2;	/* CAM_OBC_D_OFFST2 */

typedef volatile union _CAM_REG_OBC_D_OFFST3_
{
		volatile struct	/* 0x1500650C */
		{
				FIELD  OBC_D_OFST_R                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_OFFST3;	/* CAM_OBC_D_OFFST3 */

typedef volatile union _CAM_REG_OBC_D_GAIN0_
{
		volatile struct	/* 0x15006510 */
		{
				FIELD  OBC_D_GAIN_B                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_GAIN0;	/* CAM_OBC_D_GAIN0 */

typedef volatile union _CAM_REG_OBC_D_GAIN1_
{
		volatile struct	/* 0x15006514 */
		{
				FIELD  OBC_D_GAIN_GR                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_GAIN1;	/* CAM_OBC_D_GAIN1 */

typedef volatile union _CAM_REG_OBC_D_GAIN2_
{
		volatile struct	/* 0x15006518 */
		{
				FIELD  OBC_D_GAIN_GB                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_GAIN2;	/* CAM_OBC_D_GAIN2 */

typedef volatile union _CAM_REG_OBC_D_GAIN3_
{
		volatile struct	/* 0x1500651C */
		{
				FIELD  OBC_D_GAIN_R                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_D_GAIN3;	/* CAM_OBC_D_GAIN3 */

typedef volatile union _CAM_REG_LSC_D_CTL1_
{
		volatile struct	/* 0x15006530 */
		{
				FIELD  SDBLK_YOFST                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  SDBLK_XOFST                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  SD_COEFRD_MODE                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  SD_ULTRA_MODE                         :  1;		/* 28..28, 0x10000000 */
				FIELD  LSC_D_PRC_MODE                        :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_CTL1;	/* CAM_LSC_D_CTL1 */

typedef volatile union _CAM_REG_LSC_D_CTL2_
{
		volatile struct	/* 0x15006534 */
		{
				FIELD  LSC_D_SDBLK_WIDTH                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  LSC_D_SDBLK_XNUM                      :  5;		/* 12..16, 0x0001F000 */
				FIELD  LSC_D_OFLN                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_CTL2;	/* CAM_LSC_D_CTL2 */

typedef volatile union _CAM_REG_LSC_D_CTL3_
{
		volatile struct	/* 0x15006538 */
		{
				FIELD  LSC_D_SDBLK_HEIGHT                    : 12;		/*  0..11, 0x00000FFF */
				FIELD  LSC_D_SDBLK_YNUM                      :  5;		/* 12..16, 0x0001F000 */
				FIELD  LSC_SPARE                             : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_CTL3;	/* CAM_LSC_D_CTL3 */

typedef volatile union _CAM_REG_LSC_D_LBLOCK_
{
		volatile struct	/* 0x1500653C */
		{
				FIELD  LSC_D_SDBLK_lHEIGHT                   : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LSC_D_SDBLK_lWIDTH                    : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_LBLOCK;	/* CAM_LSC_D_LBLOCK */

typedef volatile union _CAM_REG_LSC_D_RATIO_
{
		volatile struct	/* 0x15006540 */
		{
				FIELD  LSC_D_RA3                             :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  LSC_D_RA2                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_D_RA1                             :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  LSC_D_RA0                             :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_RATIO;	/* CAM_LSC_D_RATIO */

typedef volatile union _CAM_REG_LSC_D_TPIPE_OFST_
{
		volatile struct	/* 0x15006544 */
		{
				FIELD  LSC_D_TPIPE_OFST_Y                    : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LSC_D_TPIPE_OFST_X                    : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_TPIPE_OFST;	/* CAM_LSC_D_TPIPE_OFST */

typedef volatile union _CAM_REG_LSC_D_TPIPE_SIZE_
{
		volatile struct	/* 0x15006548 */
		{
				FIELD  LSC_D_TPIPE_SIZE_Y                    : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_D_TPIPE_SIZE_X                    : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_TPIPE_SIZE;	/* CAM_LSC_D_TPIPE_SIZE */

typedef volatile union _CAM_REG_LSC_D_GAIN_TH_
{
		volatile struct	/* 0x1500654C */
		{
				FIELD  LSC_D_GAIN_TH2                        :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSC_D_GAIN_TH1                        :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  LSC_D_GAIN_TH0                        :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_D_GAIN_TH;	/* CAM_LSC_D_GAIN_TH */

typedef volatile union _CAM_REG_RPG_D_SATU_1_
{
		volatile struct	/* 0x15006550 */
		{
				FIELD  RPG_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_SATU_1;	/* CAM_RPG_D_SATU_1 */

typedef volatile union _CAM_REG_RPG_D_SATU_2_
{
		volatile struct	/* 0x15006554 */
		{
				FIELD  RPG_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_SATU_2;	/* CAM_RPG_D_SATU_2 */

typedef volatile union _CAM_REG_RPG_D_GAIN_1_
{
		volatile struct	/* 0x15006558 */
		{
				FIELD  RPG_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_GAIN_1;	/* CAM_RPG_D_GAIN_1 */

typedef volatile union _CAM_REG_RPG_D_GAIN_2_
{
		volatile struct	/* 0x1500655C */
		{
				FIELD  RPG_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_GAIN_2;	/* CAM_RPG_D_GAIN_2 */

typedef volatile union _CAM_REG_RPG_D_OFST_1_
{
		volatile struct	/* 0x15006560 */
		{
				FIELD  RPG_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_OFST_1;	/* CAM_RPG_D_OFST_1 */

typedef volatile union _CAM_REG_RPG_D_OFST_2_
{
		volatile struct	/* 0x15006564 */
		{
				FIELD  RPG_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_D_OFST_2;	/* CAM_RPG_D_OFST_2 */

typedef volatile union _CAM_REG_RRZ_D_CTL_
{
		volatile struct	/* 0x150067A0 */
		{
				FIELD  RRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  RRZ_HORI_TBL_SEL                      :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RRZ_VERT_TBL_SEL                      :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_CTL;	/* CAM_RRZ_D_CTL */

typedef volatile union _CAM_REG_RRZ_D_IN_IMG_
{
		volatile struct	/* 0x150067A4 */
		{
				FIELD  RRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_IN_IMG;	/* CAM_RRZ_D_IN_IMG */

typedef volatile union _CAM_REG_RRZ_D_OUT_IMG_
{
		volatile struct	/* 0x150067A8 */
		{
				FIELD  RRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_OUT_IMG;	/* CAM_RRZ_D_OUT_IMG */

typedef volatile union _CAM_REG_RRZ_D_HORI_STEP_
{
		volatile struct	/* 0x150067AC */
		{
				FIELD  RRZ_HORI_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_HORI_STEP;	/* CAM_RRZ_D_HORI_STEP */

typedef volatile union _CAM_REG_RRZ_D_VERT_STEP_
{
		volatile struct	/* 0x150067B0 */
		{
				FIELD  RRZ_VERT_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_VERT_STEP;	/* CAM_RRZ_D_VERT_STEP */

typedef volatile union _CAM_REG_RRZ_D_HORI_INT_OFST_
{
		volatile struct	/* 0x150067B4 */
		{
				FIELD  RRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_HORI_INT_OFST;	/* CAM_RRZ_D_HORI_INT_OFST */

typedef volatile union _CAM_REG_RRZ_D_HORI_SUB_OFST_
{
		volatile struct	/* 0x150067B8 */
		{
				FIELD  RRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_HORI_SUB_OFST;	/* CAM_RRZ_D_HORI_SUB_OFST */

typedef volatile union _CAM_REG_RRZ_D_VERT_INT_OFST_
{
		volatile struct	/* 0x150067BC */
		{
				FIELD  RRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_VERT_INT_OFST;	/* CAM_RRZ_D_VERT_INT_OFST */

typedef volatile union _CAM_REG_RRZ_D_VERT_SUB_OFST_
{
		volatile struct	/* 0x150067C0 */
		{
				FIELD  RRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_VERT_SUB_OFST;	/* CAM_RRZ_D_VERT_SUB_OFST */

typedef volatile union _CAM_REG_RRZ_D_MODE_TH_
{
		volatile struct	/* 0x150067C4 */
		{
				FIELD  RRZ_TH_MD                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RRZ_TH_HI                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RRZ_TH_LO                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RRZ_TH_MD2                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_MODE_TH;	/* CAM_RRZ_D_MODE_TH */

typedef volatile union _CAM_REG_RRZ_D_MODE_CTL_
{
		volatile struct	/* 0x150067C8 */
		{
				FIELD  RRZ_PRF_BLD                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RRZ_PRF                               :  2;		/*  9..10, 0x00000600 */
				FIELD  RRZ_BLD_SL                            :  5;		/* 11..15, 0x0000F800 */
				FIELD  RRZ_CR_MODE                           :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_MODE_CTL;	/* CAM_RRZ_D_MODE_CTL */

typedef volatile union _CAM_REG_RRZ_D_RLB_AOFST_
{
		volatile struct	/* 0x150067CC */
		{
				FIELD  RRZ_RLB_AOFST                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_D_RLB_AOFST;	/* CAM_RRZ_D_RLB_AOFST */

typedef volatile union _CAM_REG_BNR_D_BPC_CON_
{
		volatile struct	/* 0x15006800 */
		{
				FIELD  BPC_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BPC_LUT_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BPC_TABLE_END_MODE                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BPC_AVG_MODE                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BPC_DTC_MODE                          :  2;		/* 12..13, 0x00003000 */
				FIELD  BPC_CS_MODE                           :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_CRC_MODE                          :  2;		/* 16..17, 0x00030000 */
				FIELD  BPC_EXC                               :  1;		/* 18..18, 0x00040000 */
				FIELD  BPC_BLD_MODE                          :  1;		/* 19..19, 0x00080000 */
				FIELD  BNR_LE_INV_CTL                        :  4;		/* 20..23, 0x00F00000 */
				FIELD  BNR_OSC_COUNT                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BNR_EDGE                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_CON;	/* CAM_BNR_D_BPC_CON */

typedef volatile union _CAM_REG_BNR_D_BPC_TH1_
{
		volatile struct	/* 0x15006804 */
		{
				FIELD  BPC_TH_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_Y                              : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_BLD_SLP0                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH1;	/* CAM_BNR_D_BPC_TH1 */

typedef volatile union _CAM_REG_BNR_D_BPC_TH2_
{
		volatile struct	/* 0x15006808 */
		{
				FIELD  BPC_TH_UPB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_BLD0                              :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_BLD1                              :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH2;	/* CAM_BNR_D_BPC_TH2 */

typedef volatile union _CAM_REG_BNR_D_BPC_TH3_
{
		volatile struct	/* 0x1500680C */
		{
				FIELD  BPC_TH_XA                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_XB                             : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_TH_SLA                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_TH_SLB                            :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH3;	/* CAM_BNR_D_BPC_TH3 */

typedef volatile union _CAM_REG_BNR_D_BPC_TH4_
{
		volatile struct	/* 0x15006810 */
		{
				FIELD  BPC_DK_TH_XA                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_DK_TH_XB                          : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_DK_TH_SLA                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_DK_TH_SLB                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH4;	/* CAM_BNR_D_BPC_TH4 */

typedef volatile union _CAM_REG_BNR_D_BPC_DTC_
{
		volatile struct	/* 0x15006814 */
		{
				FIELD  BPC_RNG                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  BPC_CS_RNG                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BPC_CT_LV                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_TH_MUL                            :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  BPC_NO_LV                             :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_DTC;	/* CAM_BNR_D_BPC_DTC */

typedef volatile union _CAM_REG_BNR_D_BPC_COR_
{
		volatile struct	/* 0x15006818 */
		{
				FIELD  BPC_DIR_MAX                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BPC_DIR_TH                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BPC_RANK_IDXR                         :  3;		/* 16..18, 0x00070000 */
				FIELD  BPC_RANK_IDXG                         :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPC_DIR_TH2                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_COR;	/* CAM_BNR_D_BPC_COR */

typedef volatile union _CAM_REG_BNR_D_BPC_TBLI1_
{
		volatile struct	/* 0x1500681C */
		{
				FIELD  BPC_XOFFSET                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YOFFSET                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TBLI1;	/* CAM_BNR_D_BPC_TBLI1 */

typedef volatile union _CAM_REG_BNR_D_BPC_TBLI2_
{
		volatile struct	/* 0x15006820 */
		{
				FIELD  BPC_XSIZE                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YSIZE                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TBLI2;	/* CAM_BNR_D_BPC_TBLI2 */

typedef volatile union _CAM_REG_BNR_D_BPC_TH1_C_
{
		volatile struct	/* 0x15006824 */
		{
				FIELD  BPC_C_TH_LWB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_Y                            : 12;		/* 12..23, 0x00FFF000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH1_C;	/* CAM_BNR_D_BPC_TH1_C */

typedef volatile union _CAM_REG_BNR_D_BPC_TH2_C_
{
		volatile struct	/* 0x15006828 */
		{
				FIELD  BPC_C_TH_UPB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  BPC_RANK_IDXB                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  BPC_BLD_LWB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH2_C;	/* CAM_BNR_D_BPC_TH2_C */

typedef volatile union _CAM_REG_BNR_D_BPC_TH3_C_
{
		volatile struct	/* 0x1500682C */
		{
				FIELD  BPC_C_TH_XA                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_XB                           : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_C_TH_SLA                          :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_C_TH_SLB                          :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_BPC_TH3_C;	/* CAM_BNR_D_BPC_TH3_C */

typedef volatile union _CAM_REG_BNR_D_NR1_CON_
{
		volatile struct	/* 0x15006830 */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR1_CT_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_NR1_CON;	/* CAM_BNR_D_NR1_CON */

typedef volatile union _CAM_REG_BNR_D_NR1_CT_CON_
{
		volatile struct	/* 0x15006834 */
		{
				FIELD  NR1_CT_MD                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR1_CT_MD2                            :  2;		/*  2.. 3, 0x0000000C */
				FIELD  NR1_CT_THRD                           : 10;		/*  4..13, 0x00003FF0 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR1_MBND                              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  NR1_CT_SLOPE                          :  2;		/* 28..29, 0x30000000 */
				FIELD  NR1_CT_DIV                            :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_NR1_CT_CON;	/* CAM_BNR_D_NR1_CT_CON */

typedef volatile union _CAM_REG_BNR_D_RSV1_
{
		volatile struct	/* 0x15006838 */
		{
				FIELD  RSV1                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_RSV1;	/* CAM_BNR_D_RSV1 */

typedef volatile union _CAM_REG_BNR_D_RSV2_
{
		volatile struct	/* 0x1500683C */
		{
				FIELD  RSV2                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_RSV2;	/* CAM_BNR_D_RSV2 */

typedef volatile union _CAM_REG_BNR_D_PDC_CON_
{
		volatile struct	/* 0x15006840 */
		{
				FIELD  PDC_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  PDC_CT                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PDC_MODE                              :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  PDC_OUT                               :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_CON;	/* CAM_BNR_D_PDC_CON */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_L0_
{
		volatile struct	/* 0x15006844 */
		{
				FIELD  PDC_GCF_L00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_L0;	/* CAM_BNR_D_PDC_GAIN_L0 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_L1_
{
		volatile struct	/* 0x15006848 */
		{
				FIELD  PDC_GCF_L01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_L1;	/* CAM_BNR_D_PDC_GAIN_L1 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_L2_
{
		volatile struct	/* 0x1500684C */
		{
				FIELD  PDC_GCF_L11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_L2;	/* CAM_BNR_D_PDC_GAIN_L2 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_L3_
{
		volatile struct	/* 0x15006850 */
		{
				FIELD  PDC_GCF_L30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_L3;	/* CAM_BNR_D_PDC_GAIN_L3 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_L4_
{
		volatile struct	/* 0x15006854 */
		{
				FIELD  PDC_GCF_L12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_L4;	/* CAM_BNR_D_PDC_GAIN_L4 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_R0_
{
		volatile struct	/* 0x15006858 */
		{
				FIELD  PDC_GCF_R00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_R0;	/* CAM_BNR_D_PDC_GAIN_R0 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_R1_
{
		volatile struct	/* 0x1500685C */
		{
				FIELD  PDC_GCF_R01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_R1;	/* CAM_BNR_D_PDC_GAIN_R1 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_R2_
{
		volatile struct	/* 0x15006860 */
		{
				FIELD  PDC_GCF_R11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_R2;	/* CAM_BNR_D_PDC_GAIN_R2 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_R3_
{
		volatile struct	/* 0x15006864 */
		{
				FIELD  PDC_GCF_R30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_R3;	/* CAM_BNR_D_PDC_GAIN_R3 */

typedef volatile union _CAM_REG_BNR_D_PDC_GAIN_R4_
{
		volatile struct	/* 0x15006868 */
		{
				FIELD  PDC_GCF_R12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_GAIN_R4;	/* CAM_BNR_D_PDC_GAIN_R4 */

typedef volatile union _CAM_REG_BNR_D_PDC_TH_GB_
{
		volatile struct	/* 0x1500686C */
		{
				FIELD  PDC_GTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_BTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_TH_GB;	/* CAM_BNR_D_PDC_TH_GB */

typedef volatile union _CAM_REG_BNR_D_PDC_TH_IA_
{
		volatile struct	/* 0x15006870 */
		{
				FIELD  PDC_ITH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_ATH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_TH_IA;	/* CAM_BNR_D_PDC_TH_IA */

typedef volatile union _CAM_REG_BNR_D_PDC_TH_HD_
{
		volatile struct	/* 0x15006874 */
		{
				FIELD  PDC_NTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_DTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_TH_HD;	/* CAM_BNR_D_PDC_TH_HD */

typedef volatile union _CAM_REG_BNR_D_PDC_SL_
{
		volatile struct	/* 0x15006878 */
		{
				FIELD  PDC_GSL                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  PDC_BSL                               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  PDC_ISL                               :  4;		/*  8..11, 0x00000F00 */
				FIELD  PDC_ASL                               :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_NORM                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_SL;	/* CAM_BNR_D_PDC_SL */

typedef volatile union _CAM_REG_BNR_D_PDC_POS_
{
		volatile struct	/* 0x1500687C */
		{
				FIELD  PDC_XCENTER                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDC_YCENTER                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_D_PDC_POS;	/* CAM_BNR_D_PDC_POS */

typedef volatile union _CAM_REG_CPG_D_SATU_1_
{
		volatile struct	/* 0x15006A00 */
		{
				FIELD  CPG_D_SATU_B                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_D_SATU_GB                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_SATU_1;	/* CAM_CPG_D_SATU_1 */

typedef volatile union _CAM_REG_CPG_D_SATU_2_
{
		volatile struct	/* 0x15006A04 */
		{
				FIELD  CPG_D_SATU_GR                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_D_SATU_R                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_SATU_2;	/* CAM_CPG_D_SATU_2 */

typedef volatile union _CAM_REG_CPG_D_GAIN_1_
{
		volatile struct	/* 0x15006A08 */
		{
				FIELD  CPG_D_GAIN_B                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_D_GAIN_GB                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_GAIN_1;	/* CAM_CPG_D_GAIN_1 */

typedef volatile union _CAM_REG_CPG_D_GAIN_2_
{
		volatile struct	/* 0x15006A0C */
		{
				FIELD  CPG_D_GAIN_GR                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_D_GAIN_R                          : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_GAIN_2;	/* CAM_CPG_D_GAIN_2 */

typedef volatile union _CAM_REG_CPG_D_OFST_1_
{
		volatile struct	/* 0x15006A10 */
		{
				FIELD  CPG_D_OFST_B                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_D_OFST_GB                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_OFST_1;	/* CAM_CPG_D_OFST_1 */

typedef volatile union _CAM_REG_CPG_D_OFST_2_
{
		volatile struct	/* 0x15006A14 */
		{
				FIELD  CPG_D_OFST_GR                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_D_OFST_R                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_D_OFST_2;	/* CAM_CPG_D_OFST_2 */

typedef volatile union _CAM_REG_DBS_D_SIGMA_
{
		volatile struct	/* 0x15006C00 */
		{
				FIELD  DBS_OFST                              : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_SL                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SIGMA;	/* CAM_DBS_D_SIGMA */

typedef volatile union _CAM_REG_DBS_D_BSTBL_0_
{
		volatile struct	/* 0x15006C04 */
		{
				FIELD  DBS_BIAS_Y0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_BSTBL_0;	/* CAM_DBS_D_BSTBL_0 */

typedef volatile union _CAM_REG_DBS_D_BSTBL_1_
{
		volatile struct	/* 0x15006C08 */
		{
				FIELD  DBS_BIAS_Y4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_BSTBL_1;	/* CAM_DBS_D_BSTBL_1 */

typedef volatile union _CAM_REG_DBS_D_BSTBL_2_
{
		volatile struct	/* 0x15006C0C */
		{
				FIELD  DBS_BIAS_Y8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_BSTBL_2;	/* CAM_DBS_D_BSTBL_2 */

typedef volatile union _CAM_REG_DBS_D_BSTBL_3_
{
		volatile struct	/* 0x15006C10 */
		{
				FIELD  DBS_BIAS_Y12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_BSTBL_3;	/* CAM_DBS_D_BSTBL_3 */

typedef volatile union _CAM_REG_DBS_D_CTL_
{
		volatile struct	/* 0x15006C14 */
		{
				FIELD  DBS_HDR_GNP                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DBS_SL_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  DBS_LE_INV_CTL                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  DBS_EDGE                              :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_HDR_GAIN                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_HDR_GAIN2                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_CTL;	/* CAM_DBS_D_CTL */

typedef volatile union _CAM_REG_DBS_D_CTL_2_
{
		volatile struct	/* 0x15006C18 */
		{
				FIELD  DBS_HDR_OSCTH                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_CTL_2;	/* CAM_DBS_D_CTL_2 */

typedef volatile union _CAM_REG_DBS_D_SIGMA_2_
{
		volatile struct	/* 0x15006C1C */
		{
				FIELD  DBS_MUL_B                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_MUL_GB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_MUL_GR                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_MUL_R                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SIGMA_2;	/* CAM_DBS_D_SIGMA_2 */

typedef volatile union _CAM_REG_DBS_D_YGN_
{
		volatile struct	/* 0x15006C20 */
		{
				FIELD  DBS_YGN_B                             :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  DBS_YGN_GB                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  DBS_YGN_GR                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DBS_YGN_R                             :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_YGN;	/* CAM_DBS_D_YGN */

typedef volatile union _CAM_REG_DBS_D_SL_Y12_
{
		volatile struct	/* 0x15006C24 */
		{
				FIELD  DBS_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SL_Y12;	/* CAM_DBS_D_SL_Y12 */

typedef volatile union _CAM_REG_DBS_D_SL_Y34_
{
		volatile struct	/* 0x15006C28 */
		{
				FIELD  DBS_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SL_Y34;	/* CAM_DBS_D_SL_Y34 */

typedef volatile union _CAM_REG_DBS_D_SL_G12_
{
		volatile struct	/* 0x15006C2C */
		{
				FIELD  DBS_SL_G1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SL_G12;	/* CAM_DBS_D_SL_G12 */

typedef volatile union _CAM_REG_DBS_D_SL_G34_
{
		volatile struct	/* 0x15006C30 */
		{
				FIELD  DBS_SL_G3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_D_SL_G34;	/* CAM_DBS_D_SL_G34 */

typedef volatile union _CAM_REG_DMX_D_CTL_
{
		volatile struct	/* 0x15006E00 */
		{
				FIELD  DMX_IHDR_SEP_MODE                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DMX_IHDR_ORD                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 11;		/*  5..15, 0x0000FFE0 */
				FIELD  DMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  DMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_D_CTL;	/* CAM_DMX_D_CTL */

typedef volatile union _CAM_REG_DMX_D_CROP_
{
		volatile struct	/* 0x15006E04 */
		{
				FIELD  DMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_D_CROP;	/* CAM_DMX_D_CROP */

typedef volatile union _CAM_REG_DMX_D_VSIZE_
{
		volatile struct	/* 0x15006E08 */
		{
				FIELD  DMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_D_VSIZE;	/* CAM_DMX_D_VSIZE */

typedef volatile union _CAM_REG_BMX_D_CTL_
{
		volatile struct	/* 0x15006E10 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BMX_SINGLE_MODE_1                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BMX_SINGLE_MODE_2                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  BMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  BMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_D_CTL;	/* CAM_BMX_D_CTL */

typedef volatile union _CAM_REG_BMX_D_CROP_
{
		volatile struct	/* 0x15006E14 */
		{
				FIELD  BMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  BMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_D_CROP;	/* CAM_BMX_D_CROP */

typedef volatile union _CAM_REG_BMX_D_VSIZE_
{
		volatile struct	/* 0x15006E18 */
		{
				FIELD  BMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_D_VSIZE;	/* CAM_BMX_D_VSIZE */

typedef volatile union _CAM_REG_RMX_D_CTL_
{
		volatile struct	/* 0x15006E20 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RMX_SINGLE_MODE_1                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  RMX_SINGLE_MODE_2                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  RMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_D_CTL;	/* CAM_RMX_D_CTL */

typedef volatile union _CAM_REG_RMX_D_CROP_
{
		volatile struct	/* 0x15006E24 */
		{
				FIELD  RMX_STR_X                             : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMX_END_X                             : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_D_CROP;	/* CAM_RMX_D_CROP */

typedef volatile union _CAM_REG_RMX_D_VSIZE_
{
		volatile struct	/* 0x15006E28 */
		{
				FIELD  RMX_HT                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_D_VSIZE;	/* CAM_RMX_D_VSIZE */

typedef volatile union _CAM_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x15007200 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGI_SOFT_RST_STAT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BPCI_SOFT_RST_STAT                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LSCI_SOFT_RST_STAT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  7;		/*  5..11, 0x00000FE0 */
				FIELD  IMGO_SOFT_RST_STAT                    :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_SOFT_RST_STAT                    :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  ESFKO_SOFT_RST_STAT                   :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_SOFT_RST_STAT                     :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  4;		/* 17..20, 0x001E0000 */
				FIELD  IMG2O_SOFT_RST_STAT                   :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPCI_D_SOFT_RST_STAT                  :  1;		/* 24..24, 0x01000000 */
				FIELD  LSCI_D_SOFT_RST_STAT                  :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  3;		/* 26..28, 0x1C000000 */
				FIELD  AFO_D_SOFT_RST_STAT                   :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_SOFT_RSTSTAT;	/* CAM_DMA_SOFT_RSTSTAT */

typedef volatile union _CAM_REG_TDRI_BASE_ADDR_
{
		volatile struct	/* 0x15007204 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TDRI_BASE_ADDR;	/* CAM_TDRI_BASE_ADDR */

typedef volatile union _CAM_REG_TDRI_OFST_ADDR_
{
		volatile struct	/* 0x15007208 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TDRI_OFST_ADDR;	/* CAM_TDRI_OFST_ADDR */

typedef volatile union _CAM_REG_TDRI_XSIZE_
{
		volatile struct	/* 0x1500720C */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TDRI_XSIZE;	/* CAM_TDRI_XSIZE */

typedef volatile union _CAM_REG_CQ0I_BASE_ADDR_
{
		volatile struct	/* 0x15007210 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_BASE_ADDR;	/* CAM_CQ0I_BASE_ADDR */

typedef volatile union _CAM_REG_CQ0I_XSIZE_
{
		volatile struct	/* 0x15007214 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_XSIZE;	/* CAM_CQ0I_XSIZE */

typedef volatile union _CAM_REG_CQ0I_D_BASE_ADDR_
{
		volatile struct	/* 0x15007218 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_D_BASE_ADDR;	/* CAM_CQ0I_D_BASE_ADDR */

typedef volatile union _CAM_REG_CQ0I_D_XSIZE_
{
		volatile struct	/* 0x1500721C */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_D_XSIZE;	/* CAM_CQ0I_D_XSIZE */

typedef volatile union _CAM_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x15007220 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGI_V_FLIP_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BPCI_V_FLIP_EN                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LSCI_V_FLIP_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  7;		/*  5..11, 0x00000FE0 */
				FIELD  IMGO_V_FLIP_EN                        :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_V_FLIP_EN                        :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  ESFKO_V_FLIP_EN                       :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_V_FLIP_EN                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  4;		/* 17..20, 0x001E0000 */
				FIELD  IMG2O_V_FLIP_EN                       :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPCI_D_V_FLIP_EN                      :  1;		/* 24..24, 0x01000000 */
				FIELD  LSCI_D_V_FLIP_EN                      :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  3;		/* 26..28, 0x1C000000 */
				FIELD  AFO_D_V_FLIP_EN                       :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_VERTICAL_FLIP_EN;	/* CAM_VERTICAL_FLIP_EN */

typedef volatile union _CAM_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x15007224 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGI_SOFT_RST                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BPCI_SOFT_RST                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LSCI_SOFT_RST                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  7;		/*  5..11, 0x00000FE0 */
				FIELD  IMGO_SOFT_RST                         :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_SOFT_RST                         :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  ESFKO_SOFT_RST                        :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_SOFT_RST                          :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  4;		/* 17..20, 0x001E0000 */
				FIELD  IMG2O_SOFT_RST                        :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPCI_D_SOFT_RST                       :  1;		/* 24..24, 0x01000000 */
				FIELD  LSCI_D_SOFT_RST                       :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  3;		/* 26..28, 0x1C000000 */
				FIELD  AFO_D_SOFT_RST                        :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_SOFT_RESET;	/* CAM_DMA_SOFT_RESET */

typedef volatile union _CAM_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x15007228 */
		{
				FIELD  BPCI_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  LSCI_LAST_ULTRA_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BPCI_D_LAST_ULTRA_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  LSCI_D_LAST_ULTRA_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  8;		/*  4..11, 0x00000FF0 */
				FIELD  IMGO_LAST_ULTRA_EN                    :  1;		/* 12..12, 0x00001000 */
				FIELD  RRZO_LAST_ULTRA_EN                    :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  ESFKO_LAST_ULTRA_EN                   :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_LAST_ULTRA_EN                     :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  4;		/* 17..20, 0x001E0000 */
				FIELD  IMG2O_LAST_ULTRA_EN                   :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  7;		/* 22..28, 0x1FC00000 */
				FIELD  AFO_D_LAST_ULTRA_EN                   :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LAST_ULTRA_EN;	/* CAM_LAST_ULTRA_EN */

typedef volatile union _CAM_REG_IMGI_SLOW_DOWN_
{
		volatile struct	/* 0x1500722C */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  IMGO_RRZO_SRAM_CHANGE_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  6;		/* 17..22, 0x007E0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 23..24, 0x01800000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_ULTRA_LSCI_EN                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_ULTRA_IMGI_EN                      :  1;		/* 27..27, 0x08000000 */
				FIELD  UFO_IMGO_EN                           :  1;		/* 28..28, 0x10000000 */
				FIELD  UFO_IMGI_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_SLOW_DOWN;	/* CAM_IMGI_SLOW_DOWN */

typedef volatile union _CAM_REG_IMGI_BASE_ADDR_
{
		volatile struct	/* 0x15007230 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_BASE_ADDR;	/* CAM_IMGI_BASE_ADDR */

typedef volatile union _CAM_REG_IMGI_OFST_ADDR_
{
		volatile struct	/* 0x15007234 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_OFST_ADDR;	/* CAM_IMGI_OFST_ADDR */

typedef volatile union _CAM_REG_IMGI_XSIZE_
{
		volatile struct	/* 0x15007238 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_XSIZE;	/* CAM_IMGI_XSIZE */

typedef volatile union _CAM_REG_IMGI_YSIZE_
{
		volatile struct	/* 0x1500723C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_YSIZE;	/* CAM_IMGI_YSIZE */

typedef volatile union _CAM_REG_IMGI_STRIDE_
{
		volatile struct	/* 0x15007240 */
		{
				FIELD  STRIDE                                : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_STRIDE;	/* CAM_IMGI_STRIDE */

typedef volatile union _CAM_REG_IMGI_CON_
{
		volatile struct	/* 0x15007248 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_CON;	/* CAM_IMGI_CON */

typedef volatile union _CAM_REG_IMGI_CON2_
{
		volatile struct	/* 0x1500724C */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_CON2;	/* CAM_IMGI_CON2 */

typedef volatile union _CAM_REG_BPCI_BASE_ADDR_
{
		volatile struct	/* 0x15007250 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_BASE_ADDR;	/* CAM_BPCI_BASE_ADDR */

typedef volatile union _CAM_REG_BPCI_OFST_ADDR_
{
		volatile struct	/* 0x15007254 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_OFST_ADDR;	/* CAM_BPCI_OFST_ADDR */

typedef volatile union _CAM_REG_BPCI_XSIZE_
{
		volatile struct	/* 0x15007258 */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_XSIZE;	/* CAM_BPCI_XSIZE */

typedef volatile union _CAM_REG_BPCI_YSIZE_
{
		volatile struct	/* 0x1500725C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_YSIZE;	/* CAM_BPCI_YSIZE */

typedef volatile union _CAM_REG_BPCI_STRIDE_
{
		volatile struct	/* 0x15007260 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_STRIDE;	/* CAM_BPCI_STRIDE */

typedef volatile union _CAM_REG_BPCI_CON_
{
		volatile struct	/* 0x15007264 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON;	/* CAM_BPCI_CON */

typedef volatile union _CAM_REG_BPCI_CON2_
{
		volatile struct	/* 0x15007268 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON2;	/* CAM_BPCI_CON2 */

typedef volatile union _CAM_REG_LSCI_BASE_ADDR_
{
		volatile struct	/* 0x1500726C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_BASE_ADDR;	/* CAM_LSCI_BASE_ADDR */

typedef volatile union _CAM_REG_LSCI_OFST_ADDR_
{
		volatile struct	/* 0x15007270 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_OFST_ADDR;	/* CAM_LSCI_OFST_ADDR */

typedef volatile union _CAM_REG_LSCI_XSIZE_
{
		volatile struct	/* 0x15007274 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_XSIZE;	/* CAM_LSCI_XSIZE */

typedef volatile union _CAM_REG_LSCI_YSIZE_
{
		volatile struct	/* 0x15007278 */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_YSIZE;	/* CAM_LSCI_YSIZE */

typedef volatile union _CAM_REG_LSCI_STRIDE_
{
		volatile struct	/* 0x1500727C */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_STRIDE;	/* CAM_LSCI_STRIDE */

typedef volatile union _CAM_REG_LSCI_CON_
{
		volatile struct	/* 0x15007280 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON;	/* CAM_LSCI_CON */

typedef volatile union _CAM_REG_LSCI_CON2_
{
		volatile struct	/* 0x15007284 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON2;	/* CAM_LSCI_CON2 */

typedef volatile union _CAM_REG_IMGO_BASE_ADDR_
{
		volatile struct	/* 0x15007300 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_BASE_ADDR;	/* CAM_IMGO_BASE_ADDR */

typedef volatile union _CAM_REG_IMGO_OFST_ADDR_
{
		volatile struct	/* 0x15007304 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_OFST_ADDR;	/* CAM_IMGO_OFST_ADDR */

typedef volatile union _CAM_REG_IMGO_XSIZE_
{
		volatile struct	/* 0x15007308 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_XSIZE;	/* CAM_IMGO_XSIZE */

typedef volatile union _CAM_REG_IMGO_YSIZE_
{
		volatile struct	/* 0x1500730C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_YSIZE;	/* CAM_IMGO_YSIZE */

typedef volatile union _CAM_REG_IMGO_STRIDE_
{
		volatile struct	/* 0x15007310 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_STRIDE;	/* CAM_IMGO_STRIDE */

typedef volatile union _CAM_REG_IMGO_CON_
{
		volatile struct	/* 0x15007314 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON;	/* CAM_IMGO_CON */

typedef volatile union _CAM_REG_IMGO_CON2_
{
		volatile struct	/* 0x15007318 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON2;	/* CAM_IMGO_CON2 */

typedef volatile union _CAM_REG_IMGO_CROP_
{
		volatile struct	/* 0x1500731C */
		{
				FIELD  XOFFSET                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  YOFFSET                               : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CROP;	/* CAM_IMGO_CROP */

typedef volatile union _CAM_REG_RRZO_BASE_ADDR_
{
		volatile struct	/* 0x15007320 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_BASE_ADDR;	/* CAM_RRZO_BASE_ADDR */

typedef volatile union _CAM_REG_RRZO_OFST_ADDR_
{
		volatile struct	/* 0x15007324 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_OFST_ADDR;	/* CAM_RRZO_OFST_ADDR */

typedef volatile union _CAM_REG_RRZO_XSIZE_
{
		volatile struct	/* 0x15007328 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_XSIZE;	/* CAM_RRZO_XSIZE */

typedef volatile union _CAM_REG_RRZO_YSIZE_
{
		volatile struct	/* 0x1500732C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_YSIZE;	/* CAM_RRZO_YSIZE */

typedef volatile union _CAM_REG_RRZO_STRIDE_
{
		volatile struct	/* 0x15007330 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_STRIDE;	/* CAM_RRZO_STRIDE */

typedef volatile union _CAM_REG_RRZO_CON_
{
		volatile struct	/* 0x15007334 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON;	/* CAM_RRZO_CON */

typedef volatile union _CAM_REG_RRZO_CON2_
{
		volatile struct	/* 0x15007338 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON2;	/* CAM_RRZO_CON2 */

typedef volatile union _CAM_REG_RRZO_CROP_
{
		volatile struct	/* 0x1500733C */
		{
				FIELD  XOFFSET                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  YOFFSET                               : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CROP;	/* CAM_RRZO_CROP */

typedef volatile union _CAM_REG_EISO_BASE_ADDR_
{
		volatile struct	/* 0x1500735C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_EISO_BASE_ADDR;	/* CAM_EISO_BASE_ADDR */

typedef volatile union _CAM_REG_EISO_XSIZE_
{
		volatile struct	/* 0x15007360 */
		{
				FIELD  XSIZE                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_EISO_XSIZE;	/* CAM_EISO_XSIZE */

typedef volatile union _CAM_REG_AFO_BASE_ADDR_
{
		volatile struct	/* 0x15007364 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_BASE_ADDR;	/* CAM_AFO_BASE_ADDR */

typedef volatile union _CAM_REG_AFO_XSIZE_
{
		volatile struct	/* 0x15007368 */
		{
				FIELD  XSIZE                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_XSIZE;	/* CAM_AFO_XSIZE */

typedef volatile union _CAM_REG_ESFKO_BASE_ADDR_
{
		volatile struct	/* 0x1500736C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_BASE_ADDR;	/* CAM_ESFKO_BASE_ADDR */

typedef volatile union _CAM_REG_ESFKO_XSIZE_
{
		volatile struct	/* 0x15007370 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_XSIZE;	/* CAM_ESFKO_XSIZE */

typedef volatile union _CAM_REG_ESFKO_OFST_ADDR_
{
		volatile struct	/* 0x15007374 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_OFST_ADDR;	/* CAM_ESFKO_OFST_ADDR */

typedef volatile union _CAM_REG_ESFKO_YSIZE_
{
		volatile struct	/* 0x15007378 */
		{
				FIELD  RSV                                   : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_YSIZE;	/* CAM_ESFKO_YSIZE */

typedef volatile union _CAM_REG_ESFKO_STRIDE_
{
		volatile struct	/* 0x1500737C */
		{
				FIELD  RSV                                   : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_STRIDE;	/* CAM_ESFKO_STRIDE */

typedef volatile union _CAM_REG_ESFKO_CON_
{
		volatile struct	/* 0x15007380 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_CON;	/* CAM_ESFKO_CON */

typedef volatile union _CAM_REG_ESFKO_CON2_
{
		volatile struct	/* 0x15007384 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_CON2;	/* CAM_ESFKO_CON2 */

typedef volatile union _CAM_REG_AAO_BASE_ADDR_
{
		volatile struct	/* 0x15007388 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_BASE_ADDR;	/* CAM_AAO_BASE_ADDR */

typedef volatile union _CAM_REG_AAO_OFST_ADDR_
{
		volatile struct	/* 0x1500738C */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_OFST_ADDR;	/* CAM_AAO_OFST_ADDR */

typedef volatile union _CAM_REG_AAO_XSIZE_
{
		volatile struct	/* 0x15007390 */
		{
				FIELD  XSIZE                                 : 17;		/*  0..16, 0x0001FFFF */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_XSIZE;	/* CAM_AAO_XSIZE */

typedef volatile union _CAM_REG_AAO_YSIZE_
{
		volatile struct	/* 0x15007394 */
		{
				FIELD  RSV                                   : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_YSIZE;	/* CAM_AAO_YSIZE */

typedef volatile union _CAM_REG_AAO_STRIDE_
{
		volatile struct	/* 0x15007398 */
		{
				FIELD  RSV                                   : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_STRIDE;	/* CAM_AAO_STRIDE */

typedef volatile union _CAM_REG_AAO_CON_
{
		volatile struct	/* 0x1500739C */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON;	/* CAM_AAO_CON */

typedef volatile union _CAM_REG_AAO_CON2_
{
		volatile struct	/* 0x150073A0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON2;	/* CAM_AAO_CON2 */

typedef volatile union _CAM_REG_IMG2O_BASE_ADDR_
{
		volatile struct	/* 0x15007440 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_BASE_ADDR;	/* CAM_IMG2O_BASE_ADDR */

typedef volatile union _CAM_REG_IMG2O_OFST_ADDR_
{
		volatile struct	/* 0x15007444 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_OFST_ADDR;	/* CAM_IMG2O_OFST_ADDR */

typedef volatile union _CAM_REG_IMG2O_XSIZE_
{
		volatile struct	/* 0x15007448 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_XSIZE;	/* CAM_IMG2O_XSIZE */

typedef volatile union _CAM_REG_IMG2O_YSIZE_
{
		volatile struct	/* 0x1500744C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_YSIZE;	/* CAM_IMG2O_YSIZE */

typedef volatile union _CAM_REG_IMG2O_STRIDE_
{
		volatile struct	/* 0x15007450 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_STRIDE;	/* CAM_IMG2O_STRIDE */

typedef volatile union _CAM_REG_IMG2O_CON_
{
		volatile struct	/* 0x15007454 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_CON;	/* CAM_IMG2O_CON */

typedef volatile union _CAM_REG_IMG2O_CON2_
{
		volatile struct	/* 0x15007458 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_CON2;	/* CAM_IMG2O_CON2 */

typedef volatile union _CAM_REG_IMG2O_CROP_
{
		volatile struct	/* 0x1500745C */
		{
				FIELD  XOFFSET                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  YOFFSET                               : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_CROP;	/* CAM_IMG2O_CROP */

typedef volatile union _CAM_REG_BPCI_D_BASE_ADDR_
{
		volatile struct	/* 0x1500749C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_BASE_ADDR;	/* CAM_BPCI_D_BASE_ADDR */

typedef volatile union _CAM_REG_BPCI_D_OFST_ADDR_
{
		volatile struct	/* 0x150074A0 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_OFST_ADDR;	/* CAM_BPCI_D_OFST_ADDR */

typedef volatile union _CAM_REG_BPCI_D_XSIZE_
{
		volatile struct	/* 0x150074A4 */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_XSIZE;	/* CAM_BPCI_D_XSIZE */

typedef volatile union _CAM_REG_BPCI_D_YSIZE_
{
		volatile struct	/* 0x150074A8 */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_YSIZE;	/* CAM_BPCI_D_YSIZE */

typedef volatile union _CAM_REG_BPCI_D_STRIDE_
{
		volatile struct	/* 0x150074AC */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_STRIDE;	/* CAM_BPCI_D_STRIDE */

typedef volatile union _CAM_REG_BPCI_D_CON_
{
		volatile struct	/* 0x150074B0 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_CON;	/* CAM_BPCI_D_CON */

typedef volatile union _CAM_REG_BPCI_D_CON2_
{
		volatile struct	/* 0x150074B4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_CON2;	/* CAM_BPCI_D_CON2 */

typedef volatile union _CAM_REG_LSCI_D_BASE_ADDR_
{
		volatile struct	/* 0x150074B8 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_BASE_ADDR;	/* CAM_LSCI_D_BASE_ADDR */

typedef volatile union _CAM_REG_LSCI_D_OFST_ADDR_
{
		volatile struct	/* 0x150074BC */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_OFST_ADDR;	/* CAM_LSCI_D_OFST_ADDR */

typedef volatile union _CAM_REG_LSCI_D_XSIZE_
{
		volatile struct	/* 0x150074C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_XSIZE;	/* CAM_LSCI_D_XSIZE */

typedef volatile union _CAM_REG_LSCI_D_YSIZE_
{
		volatile struct	/* 0x150074C4 */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_YSIZE;	/* CAM_LSCI_D_YSIZE */

typedef volatile union _CAM_REG_LSCI_D_STRIDE_
{
		volatile struct	/* 0x150074C8 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_STRIDE;	/* CAM_LSCI_D_STRIDE */

typedef volatile union _CAM_REG_LSCI_D_CON_
{
		volatile struct	/* 0x150074CC */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_CON;	/* CAM_LSCI_D_CON */

typedef volatile union _CAM_REG_LSCI_D_CON2_
{
		volatile struct	/* 0x150074D0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_CON2;	/* CAM_LSCI_D_CON2 */

typedef volatile union _CAM_REG_IMGI_CON3_
{
		volatile struct	/* 0x15007500 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_CON3;	/* CAM_IMGI_CON3 */

typedef volatile union _CAM_REG_BPCI_CON3_
{
		volatile struct	/* 0x15007504 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON3;	/* CAM_BPCI_CON3 */

typedef volatile union _CAM_REG_LSCI_CON3_
{
		volatile struct	/* 0x15007508 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON3;	/* CAM_LSCI_CON3 */

typedef volatile union _CAM_REG_IMGO_CON3_
{
		volatile struct	/* 0x1500750C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON3;	/* CAM_IMGO_CON3 */

typedef volatile union _CAM_REG_RRZO_CON3_
{
		volatile struct	/* 0x15007510 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON3;	/* CAM_RRZO_CON3 */

typedef volatile union _CAM_REG_ESFKO_CON3_
{
		volatile struct	/* 0x15007514 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_CON3;	/* CAM_ESFKO_CON3 */

typedef volatile union _CAM_REG_AAO_CON3_
{
		volatile struct	/* 0x15007518 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON3;	/* CAM_AAO_CON3 */

typedef volatile union _CAM_REG_IMG2O_CON3_
{
		volatile struct	/* 0x1500751C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_CON3;	/* CAM_IMG2O_CON3 */

typedef volatile union _CAM_REG_BPCI_D_CON3_
{
		volatile struct	/* 0x15007520 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_CON3;	/* CAM_BPCI_D_CON3 */

typedef volatile union _CAM_REG_LSCI_D_CON3_
{
		volatile struct	/* 0x15007524 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_CON3;	/* CAM_LSCI_D_CON3 */

typedef volatile union _CAM_REG_AFO_D_CON3_
{
		volatile struct	/* 0x15007528 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_CON3;	/* CAM_AFO_D_CON3 */

typedef volatile union _CAM_REG_AFO_D_BASE_ADDR_
{
		volatile struct	/* 0x15007530 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_BASE_ADDR;	/* CAM_AFO_D_BASE_ADDR */

typedef volatile union _CAM_REG_AFO_D_XSIZE_
{
		volatile struct	/* 0x15007534 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_XSIZE;	/* CAM_AFO_D_XSIZE */

typedef volatile union _CAM_REG_AFO_D_OFST_ADDR_
{
		volatile struct	/* 0x15007538 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_OFST_ADDR;	/* CAM_AFO_D_OFST_ADDR */

typedef volatile union _CAM_REG_AFO_D_YSIZE_
{
		volatile struct	/* 0x1500753C */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_YSIZE;	/* CAM_AFO_D_YSIZE */

typedef volatile union _CAM_REG_AFO_D_STRIDE_
{
		volatile struct	/* 0x15007540 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_STRIDE;	/* CAM_AFO_D_STRIDE */

typedef volatile union _CAM_REG_AFO_D_CON_
{
		volatile struct	/* 0x15007544 */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_CON;	/* CAM_AFO_D_CON */

typedef volatile union _CAM_REG_AFO_D_CON2_
{
		volatile struct	/* 0x15007548 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_CON2;	/* CAM_AFO_D_CON2 */

typedef volatile union _CAM_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x15007568 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGI_ERR                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BPCI_ERR                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LSCI_ERR                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  7;		/*  5..11, 0x00000FE0 */
				FIELD  IMGO_ERR                              :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG2O_ERR                             :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  ESFKO_ERR                             :  1;		/* 15..15, 0x00008000 */
				FIELD  AAO_ERR                               :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  4;		/* 17..20, 0x001E0000 */
				FIELD  RRZO_ERR                              :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPCI_D_ERR                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSCI_D_ERR                            :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  3;		/* 26..28, 0x1C000000 */
				FIELD  AFO_D_ERR                             :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_ERR_CTRL;	/* CAM_DMA_ERR_CTRL */

typedef volatile union _CAM_REG_IMGI_ERR_STAT_
{
		volatile struct	/* 0x1500756C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGI_ERR_STAT;	/* CAM_IMGI_ERR_STAT */

typedef volatile union _CAM_REG_BPCI_ERR_STAT_
{
		volatile struct	/* 0x15007570 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_ERR_STAT;	/* CAM_BPCI_ERR_STAT */

typedef volatile union _CAM_REG_LSCI_ERR_STAT_
{
		volatile struct	/* 0x15007574 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_ERR_STAT;	/* CAM_LSCI_ERR_STAT */

typedef volatile union _CAM_REG_IMGO_ERR_STAT_
{
		volatile struct	/* 0x1500758C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_ERR_STAT;	/* CAM_IMGO_ERR_STAT */

typedef volatile union _CAM_REG_RRZO_ERR_STAT_
{
		volatile struct	/* 0x15007590 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_ERR_STAT;	/* CAM_RRZO_ERR_STAT */

typedef volatile union _CAM_REG_ESFKO_ERR_STAT_
{
		volatile struct	/* 0x15007598 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ESFKO_ERR_STAT;	/* CAM_ESFKO_ERR_STAT */

typedef volatile union _CAM_REG_AAO_ERR_STAT_
{
		volatile struct	/* 0x1500759C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_ERR_STAT;	/* CAM_AAO_ERR_STAT */

typedef volatile union _CAM_REG_IMG2O_ERR_STAT_
{
		volatile struct	/* 0x150075B0 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMG2O_ERR_STAT;	/* CAM_IMG2O_ERR_STAT */

typedef volatile union _CAM_REG_BPCI_D_ERR_STAT_
{
		volatile struct	/* 0x150075BC */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_D_ERR_STAT;	/* CAM_BPCI_D_ERR_STAT */

typedef volatile union _CAM_REG_LSCI_D_ERR_STAT_
{
		volatile struct	/* 0x150075C0 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_D_ERR_STAT;	/* CAM_LSCI_D_ERR_STAT */

typedef volatile union _CAM_REG_AFO_D_ERR_STAT_
{
		volatile struct	/* 0x150075D0 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_D_ERR_STAT;	/* CAM_AFO_D_ERR_STAT */

typedef volatile union _CAM_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x150075D8 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_DEBUG_ADDR;	/* CAM_DMA_DEBUG_ADDR */

typedef volatile union _CAM_REG_DMA_RSV1_
{
		volatile struct	/* 0x150075DC */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV1;	/* CAM_DMA_RSV1 */

typedef volatile union _CAM_REG_DMA_RSV2_
{
		volatile struct	/* 0x150075E0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV2;	/* CAM_DMA_RSV2 */

typedef volatile union _CAM_REG_DMA_RSV3_
{
		volatile struct	/* 0x150075E4 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV3;	/* CAM_DMA_RSV3 */

typedef volatile union _CAM_REG_DMA_RSV4_
{
		volatile struct	/* 0x150075E8 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV4;	/* CAM_DMA_RSV4 */

typedef volatile union _CAM_REG_DMA_RSV5_
{
		volatile struct	/* 0x150075EC */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV5;	/* CAM_DMA_RSV5 */

typedef volatile union _CAM_REG_DMA_RSV6_
{
		volatile struct	/* 0x150075F0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV6;	/* CAM_DMA_RSV6 */

typedef volatile union _CAM_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x150075F4 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  2;		/* 24..25, 0x03000000 */
				FIELD  IMGI_ADDR_GEN_WAIT                    :  1;		/* 26..26, 0x04000000 */
				FIELD  IMGI_UFD_FIFO_FULL_OFFSET             :  1;		/* 27..27, 0x08000000 */
				FIELD  IMGI_UFD_FIFO_FULL_XSIZE              :  1;		/* 28..28, 0x10000000 */
				FIELD  IMGO_UFE_FIFO_FULL_XSIZE              :  1;		/* 29..29, 0x20000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_DEBUG_SEL;	/* CAM_DMA_DEBUG_SEL */

typedef volatile union _CAM_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x150075F8 */
		{
				FIELD  BW_SELF_TEST_EN_IMGO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  BW_SELF_TEST_EN_RRZO                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  BW_SELF_TEST_EN_AAO                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BW_SELF_TEST_EN_AFO_D                 :  1;		/* 12..12, 0x00001000 */
				FIELD  BW_SELF_TEST_EN_IMG2O                 :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BW_SELF_TEST_EN_ESFKO                 :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_BW_SELF_TEST;	/* CAM_DMA_BW_SELF_TEST */

typedef volatile union _CAM_REG_ANR_TBL_
{
		volatile struct	/* 0x15007600 ~ 0x150079FF */
		{
				FIELD  ANR1_TBL_Y0                           :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR1_TBL_U0                           :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_TBL_V0                           :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  ANR1_TBL_Y1                           :  4;		/* 16..19, 0x000F0000 */
				FIELD  ANR1_TBL_U1                           :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR1_TBL_V1                           :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_TBL;	/* CAM_ANR_TBL */

typedef volatile union _CAM_REG_ANR_CON1_
{
		volatile struct	/* 0x15007A00 */
		{
				FIELD  ANR1_ENC                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ANR1_ENY                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  ANR1_SCALE_MODE                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  ANR1_TILE_EDGE                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  ANR1_MEDIAN_EN                        :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_LCE_LINK                         :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  ANR1_TABLE_EN                         :  1;		/* 28..28, 0x10000000 */
				FIELD  ANR1_TBL_PRC                          :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_CON1;	/* CAM_ANR_CON1 */

typedef volatile union _CAM_REG_ANR_CON2_
{
		volatile struct	/* 0x15007A04 */
		{
				FIELD  ANR1_IMPL_MODE                        :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  ANR1_C_MED_EN                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR1_C_SM_EDGE                        :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  ANR1_FLT_C                            :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_LR                               :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR1_ALPHA                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_CON2;	/* CAM_ANR_CON2 */

typedef volatile union _CAM_REG_ANR_YAD1_
{
		volatile struct	/* 0x15007A08 */
		{
				FIELD  ANR1_CEN_GAIN_LO_TH                   :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_CEN_GAIN_HI_TH                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_K_LO_TH                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR1_K_HI_TH                          :  4;		/* 24..27, 0x0F000000 */
				FIELD  ANR1_K_TH_C                           :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_YAD1;	/* CAM_ANR_YAD1 */

typedef volatile union _CAM_REG_ANR_YAD2_
{
		volatile struct	/* 0x15007A0C */
		{
				FIELD  ANR1_PTY_VGAIN                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_PTY_GAIN_TH                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_YAD2;	/* CAM_ANR_YAD2 */

typedef volatile union _CAM_REG_ANR_Y4LUT1_
{
		volatile struct	/* 0x15007A10 */
		{
				FIELD  ANR1_Y_CPX1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_Y_CPX2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_Y_CPX3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_Y4LUT1;	/* CAM_ANR_Y4LUT1 */

typedef volatile union _CAM_REG_ANR_Y4LUT2_
{
		volatile struct	/* 0x15007A14 */
		{
				FIELD  ANR1_Y_SCALE_CPY0                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_Y_SCALE_CPY1                     :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_Y_SCALE_CPY2                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_Y_SCALE_CPY3                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_Y4LUT2;	/* CAM_ANR_Y4LUT2 */

typedef volatile union _CAM_REG_ANR_Y4LUT3_
{
		volatile struct	/* 0x15007A18 */
		{
				FIELD  ANR1_Y_SCALE_SP0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_Y_SCALE_SP1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_Y_SCALE_SP2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_Y_SCALE_SP3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_Y4LUT3;	/* CAM_ANR_Y4LUT3 */

typedef volatile union _CAM_REG_ANR_C4LUT1_
{
		volatile struct	/* 0x15007A1C */
		{
				FIELD  ANR1_C_CPX1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_C_CPX2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_C_CPX3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_C4LUT1;	/* CAM_ANR_C4LUT1 */

typedef volatile union _CAM_REG_ANR_C4LUT2_
{
		volatile struct	/* 0x15007A20 */
		{
				FIELD  ANR1_C_SCALE_CPY0                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_C_SCALE_CPY1                     :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_C_SCALE_CPY2                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_C_SCALE_CPY3                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_C4LUT2;	/* CAM_ANR_C4LUT2 */

typedef volatile union _CAM_REG_ANR_C4LUT3_
{
		volatile struct	/* 0x15007A24 */
		{
				FIELD  ANR1_C_SCALE_SP0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_C_SCALE_SP1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_C_SCALE_SP2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_C_SCALE_SP3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_C4LUT3;	/* CAM_ANR_C4LUT3 */

typedef volatile union _CAM_REG_ANR_A4LUT2_
{
		volatile struct	/* 0x15007A28 */
		{
				FIELD  ANR1_Y_ACT_CPY0                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_Y_ACT_CPY1                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_Y_ACT_CPY2                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR1_Y_ACT_CPY3                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_A4LUT2;	/* CAM_ANR_A4LUT2 */

typedef volatile union _CAM_REG_ANR_A4LUT3_
{
		volatile struct	/* 0x15007A2C */
		{
				FIELD  ANR1_Y_ACT_SP0                        :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR1_Y_ACT_SP1                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR1_Y_ACT_SP2                        :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR1_Y_ACT_SP3                        :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_A4LUT3;	/* CAM_ANR_A4LUT3 */

typedef volatile union _CAM_REG_ANR_L4LUT1_
{
		volatile struct	/* 0x15007A30 */
		{
				FIELD  ANR1_LCE_X1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_LCE_X2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_LCE_X3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_L4LUT1;	/* CAM_ANR_L4LUT1 */

typedef volatile union _CAM_REG_ANR_L4LUT2_
{
		volatile struct	/* 0x15007A34 */
		{
				FIELD  ANR1_LCE_GAIN0                        :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR1_LCE_GAIN1                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR1_LCE_GAIN2                        :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR1_LCE_GAIN3                        :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_L4LUT2;	/* CAM_ANR_L4LUT2 */

typedef volatile union _CAM_REG_ANR_L4LUT3_
{
		volatile struct	/* 0x15007A38 */
		{
				FIELD  ANR1_LCE_SP0                          :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR1_LCE_SP1                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR1_LCE_SP2                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR1_LCE_SP3                          :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_L4LUT3;	/* CAM_ANR_L4LUT3 */

typedef volatile union _CAM_REG_ANR_PTY_
{
		volatile struct	/* 0x15007A3C */
		{
				FIELD  ANR1_PTY1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_PTY2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_PTY3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR1_PTY4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_PTY;	/* CAM_ANR_PTY */

typedef volatile union _CAM_REG_ANR_CAD_
{
		volatile struct	/* 0x15007A40 */
		{
				FIELD  ANR1_PTC_VGAIN                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_PTC_GAIN_TH                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_C_L_DIFF_TH                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_CAD;	/* CAM_ANR_CAD */

typedef volatile union _CAM_REG_ANR_PTC_
{
		volatile struct	/* 0x15007A44 */
		{
				FIELD  ANR1_PTC1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_PTC2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_PTC3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR1_PTC4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_PTC;	/* CAM_ANR_PTC */

typedef volatile union _CAM_REG_ANR_LCE_
{
		volatile struct	/* 0x15007A48 */
		{
				FIELD  ANR1_LCE_C_GAIN                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR1_LCE_SCALE_GAIN                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  ANR1_LM_WT                            :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_LCE;	/* CAM_ANR_LCE */

typedef volatile union _CAM_REG_ANR_MED1_
{
		volatile struct	/* 0x15007A4C */
		{
				FIELD  ANR1_COR_TH                           :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_COR_SL                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  ANR1_MCD_TH                           :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR1_MCD_SL                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR1_LCL_TH                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_MED1;	/* CAM_ANR_MED1 */

typedef volatile union _CAM_REG_ANR_MED2_
{
		volatile struct	/* 0x15007A50 */
		{
				FIELD  ANR1_LCL_SL                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ANR1_LCL_LV                           :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  ANR1_SCL_TH                           :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  ANR1_SCL_SL                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR1_SCL_LV                           :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_MED2;	/* CAM_ANR_MED2 */

typedef volatile union _CAM_REG_ANR_MED3_
{
		volatile struct	/* 0x15007A54 */
		{
				FIELD  ANR1_NCL_TH                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_NCL_SL                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  ANR1_NCL_LV                           :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR1_VAR                              :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR1_Y0                               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_MED3;	/* CAM_ANR_MED3 */

typedef volatile union _CAM_REG_ANR_MED4_
{
		volatile struct	/* 0x15007A58 */
		{
				FIELD  ANR1_Y1                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR1_Y2                               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR1_Y3                               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR1_Y4                               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_MED4;	/* CAM_ANR_MED4 */

typedef volatile union _CAM_REG_ANR_HP1_
{
		volatile struct	/* 0x15007A5C */
		{
				FIELD  ANR1_HP_A                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_HP_B                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR1_HP_C                             :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_HP_D                             :  4;		/* 24..27, 0x0F000000 */
				FIELD  ANR1_HP_E                             :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_HP1;	/* CAM_ANR_HP1 */

typedef volatile union _CAM_REG_ANR_HP2_
{
		volatile struct	/* 0x15007A60 */
		{
				FIELD  ANR1_HP_S1                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR1_HP_S2                            :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_HP_X1                            :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR1_HP_F                             :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_HP2;	/* CAM_ANR_HP2 */

typedef volatile union _CAM_REG_ANR_HP3_
{
		volatile struct	/* 0x15007A64 */
		{
				FIELD  ANR1_HP_Y_GAIN_CLIP                   :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ANR1_HP_Y_SP                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_HP_Y_LO                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR1_HP_CLIP                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_HP3;	/* CAM_ANR_HP3 */

typedef volatile union _CAM_REG_ANR_ACT1_
{
		volatile struct	/* 0x15007A68 */
		{
				FIELD  ANR1_ACT_LCE_GAIN                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_ACT_SCALE_OFT                    :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_ACT_SCALE_GAIN                   :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  ANR1_ACT_DIF_GAIN                     :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR1_ACT_DIF_LO_TH                    :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACT1;	/* CAM_ANR_ACT1 */

typedef volatile union _CAM_REG_ANR_ACT2_
{
		volatile struct	/* 0x15007A6C */
		{
				FIELD  ANR1_ACT_SIZE_GAIN                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  ANR1_ACT_SIZE_LO_TH                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_COR_TH1                          :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_COR_SL1                          :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACT2;	/* CAM_ANR_ACT2 */

typedef volatile union _CAM_REG_ANR_ACT3_
{
		volatile struct	/* 0x15007A70 */
		{
				FIELD  ANR1_COR_ACT_TH                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_COR_ACT_SL1                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  ANR1_COR_ACT_SL2                      :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACT3;	/* CAM_ANR_ACT3 */

typedef volatile union _CAM_REG_ANR_ACTYH_
{
		volatile struct	/* 0x15007A74 */
		{
				FIELD  ANR1_ACT_TH_Y_H                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_ACT_BLD_BASE_Y_H                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR1_ACT_SLANT_Y_H                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_ACT_BLD_TH_Y_H                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACTYH;	/* CAM_ANR_ACTYH */

typedef volatile union _CAM_REG_ANR_ACTC_
{
		volatile struct	/* 0x15007A78 */
		{
				FIELD  ANR1_ACT_TH_C                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_ACT_BLD_BASE_C                   :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR1_ACT_SLANT_C                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_ACT_BLD_TH_C                     :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACTC;	/* CAM_ANR_ACTC */

typedef volatile union _CAM_REG_ANR_ACTYL_
{
		volatile struct	/* 0x15007A7C */
		{
				FIELD  ANR1_ACT_TH_Y_L                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_ACT_BLD_BASE_Y_L                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR1_ACT_SLANT_Y_L                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR1_ACT_BLD_TH_Y_L                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_ACTYL;	/* CAM_ANR_ACTYL */

typedef volatile union _CAM_REG_ANR_YLAD_
{
		volatile struct	/* 0x15007A80 */
		{
				FIELD  ANR1_CEN_GAIN_LO_TH_LPF               :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR1_CEN_GAIN_HI_TH_LPF               :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR1_K_LMT                            :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR1_K_LPF                            :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_YLAD;	/* CAM_ANR_YLAD */

typedef volatile union _CAM_REG_ANR_PTYL_
{
		volatile struct	/* 0x15007A84 */
		{
				FIELD  ANR1_PTY1_LPF                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR1_PTY2_LPF                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR1_PTY3_LPF                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR1_PTY4_LPF                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_PTYL;	/* CAM_ANR_PTYL */

typedef volatile union _CAM_REG_ANR_LCOEF_
{
		volatile struct	/* 0x15007A88 */
		{
				FIELD  ANR1_COEF_A                           :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR1_COEF_B                           :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR1_COEF_C                           :  4;		/*  8..11, 0x00000F00 */
				FIELD  ANR1_COEF_D                           :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_LCOEF;	/* CAM_ANR_LCOEF */

typedef volatile union _CAM_REG_ANR_RSV1_
{
		volatile struct	/* 0x15007A8C */
		{
				FIELD  ANR1_RSV1                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_ANR_RSV1;	/* CAM_ANR_RSV1 */

typedef volatile union _CAM_REG_CCR_CON_
{
		volatile struct	/* 0x15007AA0 */
		{
				FIELD  CCR_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  4;		/*  1.. 4, 0x0000001E */
				FIELD  CCR_OR_MODE                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CCR_UV_GAIN_MODE                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_UV_GAIN2                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_Y_CPX3                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_CON;	/* CAM_CCR_CON */

typedef volatile union _CAM_REG_CCR_YLUT_
{
		volatile struct	/* 0x15007AA4 */
		{
				FIELD  CCR_Y_CPX1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_Y_CPX2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_Y_SP1                             :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_Y_CPY1                            :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_YLUT;	/* CAM_CCR_YLUT */

typedef volatile union _CAM_REG_CCR_UVLUT_
{
		volatile struct	/* 0x15007AA8 */
		{
				FIELD  CCR_UV_X1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_UV_X2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_UV_X3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CCR_UV_GAIN1                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_UVLUT;	/* CAM_CCR_UVLUT */

typedef volatile union _CAM_REG_CCR_YLUT2_
{
		volatile struct	/* 0x15007AAC */
		{
				FIELD  CCR_Y_SP0                             :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CCR_Y_SP2                             :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  CCR_Y_CPY0                            :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_Y_CPY2                            :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_YLUT2;	/* CAM_CCR_YLUT2 */

typedef volatile union _CAM_REG_CCR_SAT_CTRL_
{
		volatile struct	/* 0x15007AB0 */
		{
				FIELD  CCR_MODE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CCR_CEN_U                             :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  CCR_CEN_V                             :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_SAT_CTRL;	/* CAM_CCR_SAT_CTRL */

typedef volatile union _CAM_REG_CCR_UVLUT_SP_
{
		volatile struct	/* 0x15007AB4 */
		{
				FIELD  CCR_UV_GAIN_SP1                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  CCR_UV_GAIN_SP2                       : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_UVLUT_SP;	/* CAM_CCR_UVLUT_SP */

typedef volatile union _CAM_REG_CCR_HUE1_
{
		volatile struct	/* 0x15007AB8 */
		{
				FIELD  CCR_HUE_X1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_HUE_X2                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_HUE1;	/* CAM_CCR_HUE1 */

typedef volatile union _CAM_REG_CCR_HUE2_
{
		volatile struct	/* 0x15007ABC */
		{
				FIELD  CCR_HUE_X3                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_HUE_X4                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_HUE2;	/* CAM_CCR_HUE2 */

typedef volatile union _CAM_REG_CCR_HUE3_
{
		volatile struct	/* 0x15007AC0 */
		{
				FIELD  CCR_HUE_SP1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_HUE_SP2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_HUE_GAIN1                         :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_HUE_GAIN2                         :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_HUE3;	/* CAM_CCR_HUE3 */

typedef volatile union _CAM_REG_CCR_RSV1_
{
		volatile struct	/* 0x15007AC4 */
		{
				FIELD  CCR_RSV1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CCR_RSV1;	/* CAM_CCR_RSV1 */

typedef volatile union _CAMSV_REG_MODULE_EN_
{
		volatile struct	/* 0x15009000 */
		{
				FIELD  TG_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  PAK_SEL                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMGO_EN_                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 25;		/*  5..29, 0x3FFFFFE0 */
				FIELD  DB_EN                                 :  1;		/* 30..30, 0x40000000 */
				FIELD  DB_LOCK                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_MODULE_EN;	/* CAMSV_MODULE_EN */

typedef volatile union _CAMSV_REG_FMT_SEL_
{
		volatile struct	/* 0x15009004 */
		{
				FIELD  TG1_FMT                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_SW                                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  IMGO_FORMAT                           :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  IMGO_BUS_SIZE                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FMT_SEL;	/* CAMSV_FMT_SEL */

typedef volatile union _CAMSV_REG_INT_EN_
{
		volatile struct	/* 0x15009008 */
		{
				FIELD  VS1_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_INT_EN                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_INT_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_INT_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TG_DROP_INT_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TG_SOF_INT_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  2;		/*  8.. 9, 0x00000300 */
				FIELD  PASS1_DON_INT_EN                      :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGO_ERR_INT_EN                       :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGO_OVERR_INT_EN                     :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DROP_INT_EN                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_INT_EN;	/* CAMSV_INT_EN */

typedef volatile union _CAMSV_REG_INT_STATUS_
{
		volatile struct	/* 0x1500900C */
		{
				FIELD  VS1_ST                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_ST1                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_ST2                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_ST                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TG_DROP_INT_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TG_SOF1_INT_ST                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  2;		/*  8.. 9, 0x00000300 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGO_OVERR_ST                         :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DROP_ST                          :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_INT_STATUS;	/* CAMSV_INT_STATUS */

typedef volatile union _CAMSV_REG_SW_CTL_
{
		volatile struct	/* 0x15009010 */
		{
				FIELD  IMGO_RST_TRIG                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_RST_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SW_CTL;	/* CAMSV_SW_CTL */

typedef volatile union _CAMSV_REG_SPARE0_
{
		volatile struct	/* 0x15009014 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SPARE0;	/* CAMSV_SPARE0 */

typedef volatile union _CAMSV_REG_SPARE1_
{
		volatile struct	/* 0x15009018 */
		{
				FIELD  CTL_SPARE1                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SPARE1;	/* CAMSV_SPARE1 */

typedef volatile union _CAMSV_REG_IMGO_FBC_
{
		volatile struct	/* 0x1500901C */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FBC;	/* CAMSV_IMGO_FBC */

typedef volatile union _CAMSV_REG_CLK_EN_
{
		volatile struct	/* 0x15009020 */
		{
				FIELD  TG_DP_CK_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_DP_CK_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 12;		/*  3..14, 0x00007FF8 */
				FIELD  DMA_DP_CK_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_CLK_EN;	/* CAMSV_CLK_EN */

typedef volatile union _CAMSV_REG_DBG_SET_
{
		volatile struct	/* 0x15009024 */
		{
				FIELD  DEBUG_MOD_SEL                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DEBUG_SEL                             : 12;		/*  8..19, 0x000FFF00 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DBG_SET;	/* CAMSV_DBG_SET */

typedef volatile union _CAMSV_REG_DBG_PORT_
{
		volatile struct	/* 0x15009028 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DBG_PORT;	/* CAMSV_DBG_PORT */

typedef volatile union _CAMSV_REG_DATE_CODE_
{
		volatile struct	/* 0x1500902C */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DATE_CODE;	/* CAMSV_DATE_CODE */

typedef volatile union _CAMSV_REG_PROJ_CODE_
{
		volatile struct	/* 0x15009030 */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_PROJ_CODE;	/* CAMSV_PROJ_CODE */

typedef volatile union _CAMSV_REG_DCM_DIS_
{
		volatile struct	/* 0x15009034 */
		{
				FIELD  PAK_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DCM_DIS;	/* CAMSV_DCM_DIS */

typedef volatile union _CAMSV_REG_DCM_STATUS_
{
		volatile struct	/* 0x15009038 */
		{
				FIELD  PAK_DCM_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DCM_STATUS;	/* CAMSV_DCM_STATUS */

typedef volatile union _CAMSV_REG_PAK_
{
		volatile struct	/* 0x1500903C */
		{
				FIELD  PAK_MODE                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  PAK_DBL_MODE                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_PAK;	/* CAMSV_PAK */

typedef volatile union _CAMSV_REG_DMA_SOFT_RSTSTAT_SV_
{
		volatile struct	/* 0x15009200 */
		{
				FIELD  IMGO_SV_SOFT_RST_STAT                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_SV_D_SOFT_RST_STAT               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_SOFT_RSTSTAT_SV;	/* CAMSV_DMA_SOFT_RSTSTAT_SV */

typedef volatile union _CAMSV_REG_LAST_ULTRA_EN_SV_
{
		volatile struct	/* 0x15009204 */
		{
				FIELD  IMGO_SV_LAST_ULTRA_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_SV_D_LAST_ULTRA_EN               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_LAST_ULTRA_EN_SV;	/* CAMSV_LAST_ULTRA_EN_SV */

typedef volatile union _CAMSV_REG_IMGO_SV_BASE_ADDR_
{
		volatile struct	/* 0x15009208 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_BASE_ADDR;	/* CAMSV_IMGO_SV_BASE_ADDR */

typedef volatile union _CAMSV_REG_IMGO_SV_OFST_ADDR_
{
		volatile struct	/* 0x1500920C */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_OFST_ADDR;	/* CAMSV_IMGO_SV_OFST_ADDR */

typedef volatile union _CAMSV_REG_IMGO_SV_XSIZE_
{
		volatile struct	/* 0x15009210 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_XSIZE;	/* CAMSV_IMGO_SV_XSIZE */

typedef volatile union _CAMSV_REG_IMGO_SV_YSIZE_
{
		volatile struct	/* 0x15009214 */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_YSIZE;	/* CAMSV_IMGO_SV_YSIZE */

typedef volatile union _CAMSV_REG_IMGO_SV_STRIDE_
{
		volatile struct	/* 0x15009218 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_STRIDE;	/* CAMSV_IMGO_SV_STRIDE */

typedef volatile union _CAMSV_REG_IMGO_SV_CON_
{
		volatile struct	/* 0x1500921C */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_CON;	/* CAMSV_IMGO_SV_CON */

typedef volatile union _CAMSV_REG_IMGO_SV_CON2_
{
		volatile struct	/* 0x15009220 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_CON2;	/* CAMSV_IMGO_SV_CON2 */

typedef volatile union _CAMSV_REG_IMGO_SV_CROP_
{
		volatile struct	/* 0x15009224 */
		{
				FIELD  XOFFSET                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  YOFFSET                               : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_CROP;	/* CAMSV_IMGO_SV_CROP */

typedef volatile union _CAMSV_REG_IMGO_SV_D_BASE_ADDR_
{
		volatile struct	/* 0x15009228 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_BASE_ADDR;	/* CAMSV_IMGO_SV_D_BASE_ADDR */

typedef volatile union _CAMSV_REG_IMGO_SV_D_OFST_ADDR_
{
		volatile struct	/* 0x1500922C */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_OFST_ADDR;	/* CAMSV_IMGO_SV_D_OFST_ADDR */

typedef volatile union _CAMSV_REG_IMGO_SV_D_XSIZE_
{
		volatile struct	/* 0x15009230 */
		{
				FIELD  XSIZE                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_XSIZE;	/* CAMSV_IMGO_SV_D_XSIZE */

typedef volatile union _CAMSV_REG_IMGO_SV_D_YSIZE_
{
		volatile struct	/* 0x15009234 */
		{
				FIELD  YSIZE                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_YSIZE;	/* CAMSV_IMGO_SV_D_YSIZE */

typedef volatile union _CAMSV_REG_IMGO_SV_D_STRIDE_
{
		volatile struct	/* 0x15009238 */
		{
				FIELD  STRIDE                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_STRIDE;	/* CAMSV_IMGO_SV_D_STRIDE */

typedef volatile union _CAMSV_REG_IMGO_SV_D_CON_
{
		volatile struct	/* 0x1500923C */
		{
				FIELD  FIFO_SIZE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRI_THRL                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRI_THRH                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_SIZE_MSB                         :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRI_THRL_MSB                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_PRI_THRH_MSB                     :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_CON;	/* CAMSV_IMGO_SV_D_CON */

typedef volatile union _CAMSV_REG_IMGO_SV_D_CON2_
{
		volatile struct	/* 0x15009240 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FIFO_PRE_PRI_THRL                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  FIFO_PRE_PRI_THRH                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FIFO_PRE_PRI_THRL_MSB                 :  1;		/* 24..24, 0x01000000 */
				FIELD  FIFO_PRE_PRI_THRH_MSB                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_CON2;	/* CAMSV_IMGO_SV_D_CON2 */

typedef volatile union _CAMSV_REG_IMGO_SV_D_CROP_
{
		volatile struct	/* 0x15009244 */
		{
				FIELD  XOFFSET                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  YOFFSET                               : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_CROP;	/* CAMSV_IMGO_SV_D_CROP */

typedef volatile union _CAMSV_REG_DMA_ERR_CTRL_SV_
{
		volatile struct	/* 0x15009248 */
		{
				FIELD  IMGO_SV_ERR                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_SV_D_ERR                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 29;		/*  2..30, 0x7FFFFFFC */
				FIELD  ERR_CLR_MD_SV                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_ERR_CTRL_SV;	/* CAMSV_DMA_ERR_CTRL_SV */

typedef volatile union _CAMSV_REG_IMGO_SV_ERR_STAT_
{
		volatile struct	/* 0x1500924C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_ERR_STAT;	/* CAMSV_IMGO_SV_ERR_STAT */

typedef volatile union _CAMSV_REG_IMGO_SV_D_ERR_STAT_
{
		volatile struct	/* 0x15009250 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_SV_D_ERR_STAT;	/* CAMSV_IMGO_SV_D_ERR_STAT */

typedef volatile union _CAMSV_REG_DMA_DEBUG_ADDR_SV_
{
		volatile struct	/* 0x15009254 */
		{
				FIELD  DEBUG_ADDR_SV                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_DEBUG_ADDR_SV;	/* CAMSV_DMA_DEBUG_ADDR_SV */

typedef volatile union _CAMSV_REG_DMA_RSV1_SV_
{
		volatile struct	/* 0x15009258 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV1_SV;	/* CAMSV_DMA_RSV1_SV */

typedef volatile union _CAMSV_REG_DMA_RSV2_SV_
{
		volatile struct	/* 0x1500925C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV2_SV;	/* CAMSV_DMA_RSV2_SV */

typedef volatile union _CAMSV_REG_DMA_RSV3_SV_
{
		volatile struct	/* 0x15009260 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV3_SV;	/* CAMSV_DMA_RSV3_SV */

typedef volatile union _CAMSV_REG_DMA_RSV4_SV_
{
		volatile struct	/* 0x15009264 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV4_SV;	/* CAMSV_DMA_RSV4_SV */

typedef volatile union _CAMSV_REG_DMA_RSV5_SV_
{
		volatile struct	/* 0x15009268 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV5_SV;	/* CAMSV_DMA_RSV5_SV */

typedef volatile union _CAMSV_REG_DMA_RSV6_SV_
{
		volatile struct	/* 0x1500926C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV6_SV;	/* CAMSV_DMA_RSV6_SV */

typedef volatile union _CAMSV_REG_TG_SEN_MODE_
{
		volatile struct	/* 0x15009410 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_MODE;	/* CAMSV_TG_SEN_MODE */

typedef volatile union _CAMSV_REG_TG_VF_CON_
{
		volatile struct	/* 0x15009414 */
		{
				FIELD  VFDATA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SINGLE_MODE                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FR_CON                                :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SP_DELAY                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SPDELAY_MODE                          :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_VF_CON;	/* CAMSV_TG_VF_CON */

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_PXL_
{
		volatile struct	/* 0x15009418 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_PXL;	/* CAMSV_TG_SEN_GRAB_PXL */

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_LIN_
{
		volatile struct	/* 0x1500941C */
		{
				FIELD  LIN_S                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LIN_E                                 : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_LIN;	/* CAMSV_TG_SEN_GRAB_LIN */

typedef volatile union _CAMSV_REG_TG_PATH_CFG_
{
		volatile struct	/* 0x15009420 */
		{
				FIELD  SEN_IN_LSB                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  JPGINF_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MEMIN_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  JPG_LINEND_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DB_LOAD_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DB_LOAD_SRC                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DB_LOAD_VSPOL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_PATH_CFG;	/* CAMSV_TG_PATH_CFG */

typedef volatile union _CAMSV_REG_TG_MEMIN_CTL_
{
		volatile struct	/* 0x15009424 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  FBC_CNT                               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MEMIN_CTL;	/* CAMSV_TG_MEMIN_CTL */

typedef volatile union _CAMSV_REG_TG_INT1_
{
		volatile struct	/* 0x15009428 */
		{
				FIELD  TG_INT1_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_INT1;	/* CAMSV_TG_INT1 */

typedef volatile union _CAMSV_REG_TG_INT2_
{
		volatile struct	/* 0x1500942C */
		{
				FIELD  TG_INT2_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_INT2;	/* CAMSV_TG_INT2 */

typedef volatile union _CAMSV_REG_TG_SOF_CNT_
{
		volatile struct	/* 0x15009430 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SOF_CNT;	/* CAMSV_TG_SOF_CNT */

typedef volatile union _CAMSV_REG_TG_SOT_CNT_
{
		volatile struct	/* 0x15009434 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SOT_CNT;	/* CAMSV_TG_SOT_CNT */

typedef volatile union _CAMSV_REG_TG_EOT_CNT_
{
		volatile struct	/* 0x15009438 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_EOT_CNT;	/* CAMSV_TG_EOT_CNT */

typedef volatile union _CAMSV_REG_TG_ERR_CTL_
{
		volatile struct	/* 0x1500943C */
		{
				FIELD  GRAB_ERR_FLIMIT_NO                    :  4;		/*  0.. 3, 0x0000000F */
				FIELD  GRAB_ERR_FLIMIT_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  GRAB_ERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DBG_SRC_SEL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_ERR_CTL;	/* CAMSV_TG_ERR_CTL */

typedef volatile union _CAMSV_REG_TG_DAT_NO_
{
		volatile struct	/* 0x15009440 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_DAT_NO;	/* CAMSV_TG_DAT_NO */

typedef volatile union _CAMSV_REG_TG_FRM_CNT_ST_
{
		volatile struct	/* 0x15009444 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FRM_CNT_ST;	/* CAMSV_TG_FRM_CNT_ST */

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_
{
		volatile struct	/* 0x15009448 */
		{
				FIELD  LINE_CNT                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FRMSIZE_ST;	/* CAMSV_TG_FRMSIZE_ST */

typedef volatile union _CAMSV_REG_TG_INTER_ST_
{
		volatile struct	/* 0x1500944C */
		{
				FIELD  SYN_VF_DATA_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  OUT_RDY                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OUT_REQ                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  TG_CAM_CS                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CAM_FRM_CNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_INTER_ST;	/* CAMSV_TG_INTER_ST */

typedef volatile union _CAMSV_REG_TG_FLASHA_CTL_
{
		volatile struct	/* 0x15009460 */
		{
				FIELD  FLASHA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLASH_EN                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FLASHA_STARTPNT                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FLASHA_END_FRM                        :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  FLASH_POL                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHA_CTL;	/* CAMSV_TG_FLASHA_CTL */

typedef volatile union _CAMSV_REG_TG_FLASHA_LINE_CNT_
{
		volatile struct	/* 0x15009464 */
		{
				FIELD  FLASHA_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHA_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHA_LINE_CNT;	/* CAMSV_TG_FLASHA_LINE_CNT */

typedef volatile union _CAMSV_REG_TG_FLASHA_POS_
{
		volatile struct	/* 0x15009468 */
		{
				FIELD  FLASHA_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHA_LINE                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHA_POS;	/* CAMSV_TG_FLASHA_POS */

typedef volatile union _CAMSV_REG_TG_FLASHB_CTL_
{
		volatile struct	/* 0x1500946C */
		{
				FIELD  FLASHB_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLASHB_TRIG_SRC                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FLASHB_STARTPNT                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FLASHB_START_FRM                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  FLASHB_CONT_FRM                       :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_CTL;	/* CAMSV_TG_FLASHB_CTL */

typedef volatile union _CAMSV_REG_TG_FLASHB_LINE_CNT_
{
		volatile struct	/* 0x15009470 */
		{
				FIELD  FLASHB_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHB_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_LINE_CNT;	/* CAMSV_TG_FLASHB_LINE_CNT */

typedef volatile union _CAMSV_REG_TG_FLASHB_POS_
{
		volatile struct	/* 0x15009474 */
		{
				FIELD  FLASHB_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHB_LINE                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS;	/* CAMSV_TG_FLASHB_POS */

typedef volatile union _CAMSV_REG_TG_FLASHB_POS1_
{
		volatile struct	/* 0x15009478 */
		{
				FIELD  FLASHB_CYC_CNT                        : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS1;	/* CAMSV_TG_FLASHB_POS1 */

typedef volatile union _CAMSV_REG_TG_GSCTRL_CTL_
{
		volatile struct	/* 0x1500947C */
		{
				FIELD  GSCTRL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  GSCTRL_POL                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_GSCTRL_CTL;	/* CAMSV_TG_GSCTRL_CTL */

typedef volatile union _CAMSV_REG_TG_GSCTRL_TIME_
{
		volatile struct	/* 0x15009480 */
		{
				FIELD  GS_EPTIME                             : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  GSMS_TIMEU                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_GSCTRL_TIME;	/* CAMSV_TG_GSCTRL_TIME */

typedef volatile union _CAMSV_REG_TG_MS_PHASE_
{
		volatile struct	/* 0x15009484 */
		{
				FIELD  MSCTRL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MSCTRL_VSPOL                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MSCTRL_OPEN_TRSRC                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  MSCTRL_TRSRC                          :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  MSCP1_PH0                             :  1;		/* 16..16, 0x00010000 */
				FIELD  MSCP1_PH1                             :  1;		/* 17..17, 0x00020000 */
				FIELD  MSCP1_PH2                             :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  MSOP1_PH0                             :  1;		/* 20..20, 0x00100000 */
				FIELD  MSOP1_PH1                             :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  MSP1_RST                              :  1;		/* 23..23, 0x00800000 */
				FIELD  MSCP2_PH0                             :  1;		/* 24..24, 0x01000000 */
				FIELD  MSCP2_PH1                             :  1;		/* 25..25, 0x02000000 */
				FIELD  MSCP2_PH2                             :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  MSOP2_PH0                             :  1;		/* 28..28, 0x10000000 */
				FIELD  MSOP2_PH1                             :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  MSP2_RST                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MS_PHASE;	/* CAMSV_TG_MS_PHASE */

typedef volatile union _CAMSV_REG_TG_MS_CL_TIME_
{
		volatile struct	/* 0x15009488 */
		{
				FIELD  MS_TCLOSE                             : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MS_CL_TIME;	/* CAMSV_TG_MS_CL_TIME */

typedef volatile union _CAMSV_REG_TG_MS_OP_TIME_
{
		volatile struct	/* 0x1500948C */
		{
				FIELD  MS_TOPEN                              : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MS_OP_TIME;	/* CAMSV_TG_MS_OP_TIME */

typedef volatile union _CAMSV_REG_TG_MS_CLPH_TIME_
{
		volatile struct	/* 0x15009490 */
		{
				FIELD  MS_CL_T1                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MS_CL_T2                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MS_CLPH_TIME;	/* CAMSV_TG_MS_CLPH_TIME */

typedef volatile union _CAMSV_REG_TG_MS_OPPH_TIME_
{
		volatile struct	/* 0x15009494 */
		{
				FIELD  MS_OP_T3                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MS_OP_T4                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MS_OPPH_TIME;	/* CAMSV_TG_MS_OPPH_TIME */

typedef volatile union _CAMSV_REG_TOP_DEBUG_
{
		volatile struct	/* 0x1500950C */
		{
				FIELD  DEBUG                                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TOP_DEBUG;	/* CAMSV_TOP_DEBUG */

typedef volatile union _CAMSV2_REG_MODULE_EN_
{
		volatile struct	/* 0x15009800 */
		{
				FIELD  TG_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  PAK_SEL                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMGO_EN_                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 25;		/*  5..29, 0x3FFFFFE0 */
				FIELD  DB_EN                                 :  1;		/* 30..30, 0x40000000 */
				FIELD  DB_LOCK                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_MODULE_EN;	/* CAMSV2_MODULE_EN */

typedef volatile union _CAMSV2_REG_FMT_SEL_
{
		volatile struct	/* 0x15009804 */
		{
				FIELD  TG1_FMT                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_SW                                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  IMGO_FORMAT                           :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  IMGO_BUS_SIZE                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_FMT_SEL;	/* CAMSV2_FMT_SEL */

typedef volatile union _CAMSV2_REG_INT_EN_
{
		volatile struct	/* 0x15009808 */
		{
				FIELD  VS1_INT_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_INT_EN                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_INT_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_INT_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TG_DROP_INT_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TG_SOF_INT_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  2;		/*  8.. 9, 0x00000300 */
				FIELD  PASS1_DON_INT_EN                      :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGO_ERR_INT_EN                       :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGO_OVERR_INT_EN                     :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DROP_INT_EN                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_INT_EN;	/* CAMSV2_INT_EN */

typedef volatile union _CAMSV2_REG_INT_STATUS_
{
		volatile struct	/* 0x1500980C */
		{
				FIELD  VS1_ST                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_ST1                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_ST2                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON1_ST                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_ST                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TG_DROP_INT_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TG_SOF1_INT_ST                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  2;		/*  8.. 9, 0x00000300 */
				FIELD  PASS1_DON_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGO_OVERR_ST                         :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  1;		/* 18..18, 0x00040000 */
				FIELD  IMGO_DROP_ST                          :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_INT_STATUS;	/* CAMSV2_INT_STATUS */

typedef volatile union _CAMSV2_REG_SW_CTL_
{
		volatile struct	/* 0x15009810 */
		{
				FIELD  IMGO_RST_TRIG                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_RST_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_SW_CTL;	/* CAMSV2_SW_CTL */

typedef volatile union _CAMSV2_REG_SPARE0_
{
		volatile struct	/* 0x15009814 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_SPARE0;	/* CAMSV2_SPARE0 */

typedef volatile union _CAMSV2_REG_SPARE1_
{
		volatile struct	/* 0x15009818 */
		{
				FIELD  CTL_SPARE1                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_SPARE1;	/* CAMSV2_SPARE1 */

typedef volatile union _CAMSV2_REG_IMGO_FBC_
{
		volatile struct	/* 0x1500981C */
		{
				FIELD  FBC_CNT                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  7;		/*  4..10, 0x000007F0 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  2;		/* 12..13, 0x00003000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_IMGO_FBC;	/* CAMSV2_IMGO_FBC */

typedef volatile union _CAMSV2_REG_CLK_EN_
{
		volatile struct	/* 0x15009820 */
		{
				FIELD  TG_DP_CK_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_DP_CK_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 12;		/*  3..14, 0x00007FF8 */
				FIELD  DMA_DP_CK_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_CLK_EN;	/* CAMSV2_CLK_EN */

typedef volatile union _CAMSV2_REG_DBG_SET_
{
		volatile struct	/* 0x15009824 */
		{
				FIELD  DEBUG_MOD_SEL                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DEBUG_SEL                             : 12;		/*  8..19, 0x000FFF00 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_DBG_SET;	/* CAMSV2_DBG_SET */

typedef volatile union _CAMSV2_REG_DBG_PORT_
{
		volatile struct	/* 0x15009828 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_DBG_PORT;	/* CAMSV2_DBG_PORT */

typedef volatile union _CAMSV2_REG_DATE_CODE_
{
		volatile struct	/* 0x1500982C */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_DATE_CODE;	/* CAMSV2_DATE_CODE */

typedef volatile union _CAMSV2_REG_PROJ_CODE_
{
		volatile struct	/* 0x15009830 */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_PROJ_CODE;	/* CAMSV2_PROJ_CODE */

typedef volatile union _CAMSV2_REG_DCM_DIS_
{
		volatile struct	/* 0x15009834 */
		{
				FIELD  PAK_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_DCM_DIS;	/* CAMSV2_DCM_DIS */

typedef volatile union _CAMSV2_REG_DCM_STATUS_
{
		volatile struct	/* 0x15009838 */
		{
				FIELD  PAK_DCM_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_DCM_STATUS;	/* CAMSV2_DCM_STATUS */

typedef volatile union _CAMSV2_REG_PAK_
{
		volatile struct	/* 0x1500983C */
		{
				FIELD  PAK_MODE                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  PAK_DBL_MODE                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAMSV2_REG_PAK;	/* CAMSV2_PAK */

typedef volatile union _CAMSV_REG_TG2_SEN_MODE_
{
		volatile struct	/* 0x15009C10 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_SEN_MODE;	/* CAMSV_TG2_SEN_MODE */

typedef volatile union _CAMSV_REG_TG2_VF_CON_
{
		volatile struct	/* 0x15009C14 */
		{
				FIELD  VFDATA_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SINGLE_MODE                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  FR_CON                                :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SP_DELAY                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SPDELAY_MODE                          :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_VF_CON;	/* CAMSV_TG2_VF_CON */

typedef volatile union _CAMSV_REG_TG2_SEN_GRAB_PXL_
{
		volatile struct	/* 0x15009C18 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_SEN_GRAB_PXL;	/* CAMSV_TG2_SEN_GRAB_PXL */

typedef volatile union _CAMSV_REG_TG2_SEN_GRAB_LIN_
{
		volatile struct	/* 0x15009C1C */
		{
				FIELD  LIN_S                                 : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LIN_E                                 : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_SEN_GRAB_LIN;	/* CAMSV_TG2_SEN_GRAB_LIN */

typedef volatile union _CAMSV_REG_TG2_PATH_CFG_
{
		volatile struct	/* 0x15009C20 */
		{
				FIELD  SEN_IN_LSB                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  JPGINF_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MEMIN_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  JPG_LINEND_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DB_LOAD_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DB_LOAD_SRC                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DB_LOAD_VSPOL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RCNT_INC                              :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  FBC_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  LOCK_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FB_NUM                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  RCNT                                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  WCNT                                  :  4;		/* 24..27, 0x0F000000 */
				FIELD  DROP_CNT                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_PATH_CFG;	/* CAMSV_TG2_PATH_CFG */

typedef volatile union _CAMSV_REG_TG2_MEMIN_CTL_
{
		volatile struct	/* 0x15009C24 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  FBC_CNT                               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_MEMIN_CTL;	/* CAMSV_TG2_MEMIN_CTL */

typedef volatile union _CAMSV_REG_TG2_INT1_
{
		volatile struct	/* 0x15009C28 */
		{
				FIELD  TG_INT1_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_INT1;	/* CAMSV_TG2_INT1 */

typedef volatile union _CAMSV_REG_TG2_INT2_
{
		volatile struct	/* 0x15009C2C */
		{
				FIELD  TG_INT2_LINENO                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_INT2;	/* CAMSV_TG2_INT2 */

typedef volatile union _CAMSV_REG_TG2_SOF_CNT_
{
		volatile struct	/* 0x15009C30 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_SOF_CNT;	/* CAMSV_TG2_SOF_CNT */

typedef volatile union _CAMSV_REG_TG2_SOT_CNT_
{
		volatile struct	/* 0x15009C34 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_SOT_CNT;	/* CAMSV_TG2_SOT_CNT */

typedef volatile union _CAMSV_REG_TG2_EOT_CNT_
{
		volatile struct	/* 0x15009C38 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_EOT_CNT;	/* CAMSV_TG2_EOT_CNT */

typedef volatile union _CAMSV_REG_TG2_ERR_CTL_
{
		volatile struct	/* 0x15009C3C */
		{
				FIELD  GRAB_ERR_FLIMIT_NO                    :  4;		/*  0.. 3, 0x0000000F */
				FIELD  GRAB_ERR_FLIMIT_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  GRAB_ERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  REZ_OVRUN_FLIMIT_NO                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  REZ_OVRUN_FLIMIT_EN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DBG_SRC_SEL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_ERR_CTL;	/* CAMSV_TG2_ERR_CTL */

typedef volatile union _CAMSV_REG_TG2_DAT_NO_
{
		volatile struct	/* 0x15009C40 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_DAT_NO;	/* CAMSV_TG2_DAT_NO */

typedef volatile union _CAMSV_REG_TG2_FRM_CNT_ST_
{
		volatile struct	/* 0x15009C44 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_FRM_CNT_ST;	/* CAMSV_TG2_FRM_CNT_ST */

typedef volatile union _CAMSV_REG_TG2_FRMSIZE_ST_
{
		volatile struct	/* 0x15009C48 */
		{
				FIELD  LINE_CNT                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_FRMSIZE_ST;	/* CAMSV_TG2_FRMSIZE_ST */

typedef volatile union _CAMSV_REG_TG2_INTER_ST_
{
		volatile struct	/* 0x15009C4C */
		{
				FIELD  SYN_VF_DATA_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  OUT_RDY                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OUT_REQ                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  TG_CAM_CS                             :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CAM_FRM_CNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG2_INTER_ST;	/* CAMSV_TG2_INTER_ST */

/* manual add */
typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_SET_
{
    volatile struct
    {
        FIELD PASS1_DB_EN_SET               : 1;
        FIELD PASS1_D_DB_EN_SET             : 1;
        FIELD rsv_2                         : 1;
        FIELD APB_CLK_GATE_BYPASS_SET       : 1;
        FIELD rsv_4                         : 1;
        FIELD DUAL_MODE_SET                 : 1;
        FIELD BIN_SEL_SET                   : 1;
        FIELD BIN_SEL_EN_SET                : 1;
        FIELD PREGAIN_SEL_SET               : 1;
        FIELD PREGAIN_SEL_EN_SET            : 1;
        FIELD LSC_EARLY_SOF_EN_SET          : 1;
        FIELD W2G_MODE_SET                  : 1;
        FIELD rsv_12                        : 1;
        FIELD MFB_MODE_SET                  : 1;
        FIELD INT_MRG_SET                   : 1;
        FIELD BIN_SEL_D_SET                 : 1;
        FIELD BIN_SEL_EN_D_SET              : 1;
        FIELD PREGAIN_SEL_D_SET             : 1;
        FIELD PREGAIN_SEL_EN_D_SET          : 1;
        FIELD LSC_EARLY_SOF_EN_D_SET        : 1;
        FIELD W2G_MODE_D_SET                : 1;
        FIELD rsv_21                        : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL_SET;
typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_CLR_
{
    volatile struct
    {
        FIELD PASS1_DB_EN_SET               : 1;
        FIELD PASS1_D_DB_EN_SET             : 1;
        FIELD rsv_2                         : 1;
        FIELD APB_CLK_GATE_BYPASS_SET       : 1;
        FIELD rsv_4                         : 1;
        FIELD DUAL_MODE_SET                 : 1;
        FIELD BIN_SEL_SET                   : 1;
        FIELD BIN_SEL_EN_SET                : 1;
        FIELD PREGAIN_SEL_SET               : 1;
        FIELD PREGAIN_SEL_EN_SET            : 1;
        FIELD LSC_EARLY_SOF_EN_SET          : 1;
        FIELD W2G_MODE_SET                  : 1;
        FIELD rsv_12                        : 1;
        FIELD MFB_MODE_SET                  : 1;
        FIELD INT_MRG_SET                   : 1;
        FIELD BIN_SEL_D_SET                 : 1;
        FIELD BIN_SEL_EN_D_SET              : 1;
        FIELD PREGAIN_SEL_D_SET             : 1;
        FIELD PREGAIN_SEL_EN_D_SET          : 1;
        FIELD LSC_EARLY_SOF_EN_D_SET        : 1;
        FIELD W2G_MODE_D_SET                : 1;
        FIELD rsv_21                        : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL_CLR;

typedef volatile struct _isp_reg_t_	/* 0x15000000..0x15009C4F */
{
	UINT32                                          rsv_0000[4096];                                  /* 0000..3FFC, 0x15000000..15003FFC */
	CAM_REG_CTL_START                               CAM_CTL_START;                                   /* 4000, 0x15004000 */
	CAM_REG_CTL_EN_P1                               CAM_CTL_EN_P1;                                   /* 4004, 0x15004004 */
	CAM_REG_CTL_EN_P1_DMA                           CAM_CTL_EN_P1_DMA;                               /* 4008, 0x15004008 */
	UINT32                                          rsv_400C;                                        /* 400C, 0x1500400C */
	CAM_REG_CTL_EN_P1_D                             CAM_CTL_EN_P1_D;                                 /* 4010, 0x15004010 */
	CAM_REG_CTL_EN_P1_DMA_D                         CAM_CTL_EN_P1_DMA_D;                             /* 4014, 0x15004014 */
	CAM_REG_CTL_EN_P2                               CAM_CTL_EN_P2;                                   /* 4018, 0x15004018 */
	CAM_REG_CTL_EN_P2_DMA                           CAM_CTL_EN_P2_DMA;                               /* 401C, 0x1500401C */
	CAM_REG_CTL_CQ_EN                               CAM_CTL_CQ_EN;                                   /* 4020, 0x15004020 */
	CAM_REG_CTL_SCENARIO                            CAM_CTL_SCENARIO;                                /* 4024, 0x15004024 */
	CAM_REG_CTL_FMT_SEL_P1                          CAM_CTL_FMT_SEL_P1;                              /* 4028, 0x15004028 */
	CAM_REG_CTL_FMT_SEL_P1_D                        CAM_CTL_FMT_SEL_P1_D;                            /* 402C, 0x1500402C */
	CAM_REG_CTL_FMT_SEL_P2                          CAM_CTL_FMT_SEL_P2;                              /* 4030, 0x15004030 */
	CAM_REG_CTL_SEL_P1                              CAM_CTL_SEL_P1;                                  /* 4034, 0x15004034 */
	CAM_REG_CTL_SEL_P1_D                            CAM_CTL_SEL_P1_D;                                /* 4038, 0x15004038 */
	CAM_REG_CTL_SEL_P2                              CAM_CTL_SEL_P2;                                  /* 403C, 0x1500403C */
	CAM_REG_CTL_SEL_GLOBAL                          CAM_CTL_SEL_GLOBAL;                              /* 4040, 0x15004040 */
	UINT32                                          rsv_4044;                                        /* 4044, 0x15004044 */
	CAM_REG_CTL_INT_P1_EN                           CAM_CTL_INT_P1_EN;                               /* 4048, 0x15004048 */
	CAM_REG_CTL_INT_P1_STATUS                       CAM_CTL_INT_P1_STATUS;                           /* 404C, 0x1500404C */
	CAM_REG_CTL_INT_P1_EN2                          CAM_CTL_INT_P1_EN2;                              /* 4050, 0x15004050 */
	CAM_REG_CTL_INT_P1_STATUS2                      CAM_CTL_INT_P1_STATUS2;                          /* 4054, 0x15004054 */
	CAM_REG_CTL_INT_P1_EN_D                         CAM_CTL_INT_P1_EN_D;                             /* 4058, 0x15004058 */
	CAM_REG_CTL_INT_P1_STATUS_D                     CAM_CTL_INT_P1_STATUS_D;                         /* 405C, 0x1500405C */
	CAM_REG_CTL_INT_P1_EN2_D                        CAM_CTL_INT_P1_EN2_D;                            /* 4060, 0x15004060 */
	CAM_REG_CTL_INT_P1_STATUS2_D                    CAM_CTL_INT_P1_STATUS2_D;                        /* 4064, 0x15004064 */
	CAM_REG_CTL_INT_P2_EN                           CAM_CTL_INT_P2_EN;                               /* 4068, 0x15004068 */
	CAM_REG_CTL_INT_P2_STATUS                       CAM_CTL_INT_P2_STATUS;                           /* 406C, 0x1500406C */
	CAM_REG_CTL_INT_STATUSX                         CAM_CTL_INT_STATUSX;                             /* 4070, 0x15004070 */
	CAM_REG_CTL_INT_STATUS2X                        CAM_CTL_INT_STATUS2X;                            /* 4074, 0x15004074 */
	CAM_REG_CTL_INT_STATUS3X                        CAM_CTL_INT_STATUS3X;                            /* 4078, 0x15004078 */
	CAM_REG_CTL_TILE                                CAM_CTL_TILE;                                    /* 407C, 0x1500407C */
	CAM_REG_CTL_TDR_EN_STATUS                       CAM_CTL_TDR_EN_STATUS;                           /* 4080, 0x15004080 */
	CAM_REG_CTL_TCM_EN                              CAM_CTL_TCM_EN;                                  /* 4084, 0x15004084 */
	CAM_REG_CTL_TDR_DBG_STATUS                      CAM_CTL_TDR_DBG_STATUS;                          /* 4088, 0x15004088 */
	CAM_REG_CTL_SW_CTL                              CAM_CTL_SW_CTL;                                  /* 408C, 0x1500408C */
	CAM_REG_CTL_SPARE0                              CAM_CTL_SPARE0;                                  /* 4090, 0x15004090 */
	CAM_REG_CTL_SPARE1                              CAM_RRZ_OUT_W;                                   /* 4094, 0x15004094 */
	UINT32                                          rsv_4098;                                        /* 4098, 0x15004098 */
	CAM_REG_CTL_SPARE2                              CAM_RRZ_OUT_W_D;                                 /* 409C, 0x1500409C */
	CAM_REG_CTL_CQ1_BASEADDR                        CAM_CTL_CQ1_BASEADDR;                            /* 40A0, 0x150040A0 */
	CAM_REG_CTL_CQ2_BASEADDR                        CAM_CTL_CQ2_BASEADDR;                            /* 40A4, 0x150040A4 */
	CAM_REG_CTL_CQ3_BASEADDR                        CAM_CTL_CQ3_BASEADDR;                            /* 40A8, 0x150040A8 */
	CAM_REG_CTL_CQ0_BASEADDR                        CAM_CTL_CQ0_BASEADDR;                            /* 40AC, 0x150040AC */
	CAM_REG_CTL_CQ0B_BASEADDR                       CAM_CTL_CQ0B_BASEADDR;                           /* 40B0, 0x150040B0 */
	CAM_REG_CTL_CQ0C_BASEADDR                       CAM_CTL_CQ0C_BASEADDR;                           /* 40B4, 0x150040B4 */
	CAM_REG_CTL_CUR_CQ0_BASEADDR                    CAM_CTL_CUR_CQ0_BASEADDR;                        /* 40B8, 0x150040B8 */
	CAM_REG_CTL_CUR_CQ0B_BASEADDR                   CAM_CTL_CUR_CQ0B_BASEADDR;                       /* 40BC, 0x150040BC */
	CAM_REG_CTL_CUR_CQ0C_BASEADDR                   CAM_CTL_CUR_CQ0C_BASEADDR;                       /* 40C0, 0x150040C0 */
	CAM_REG_CTL_CQ0_D_BASEADDR                      CAM_CTL_CQ0_D_BASEADDR;                          /* 40C4, 0x150040C4 */
	CAM_REG_CTL_CQ0B_D_BASEADDR                     CAM_CTL_CQ0B_D_BASEADDR;                         /* 40C8, 0x150040C8 */
	CAM_REG_CTL_CQ0C_D_BASEADDR                     CAM_CTL_CQ0C_D_BASEADDR;                         /* 40CC, 0x150040CC */
	CAM_REG_CTL_CUR_CQ0_D_BASEADDR                  CAM_CTL_CUR_CQ0_D_BASEADDR;                      /* 40D0, 0x150040D0 */
	CAM_REG_CTL_CUR_CQ0B_D_BASEADDR                 CAM_CTL_CUR_CQ0B_D_BASEADDR;                     /* 40D4, 0x150040D4 */
	CAM_REG_CTL_CUR_CQ0C_D_BASEADDR                 CAM_CTL_CUR_CQ0C_D_BASEADDR;                     /* 40D8, 0x150040D8 */
	CAM_REG_CTL_DB_LOAD_D                           CAM_CTL_DB_LOAD_D;                               /* 40DC, 0x150040DC */
	CAM_REG_CTL_DB_LOAD                             CAM_CTL_DB_LOAD;                                 /* 40E0, 0x150040E0 */
	CAM_REG_CTL_P1_DONE_BYP                         CAM_CTL_P1_DONE_BYP;                             /* 40E4, 0x150040E4 */
	CAM_REG_CTL_P1_DONE_BYP_D                       CAM_CTL_P1_DONE_BYP_D;                           /* 40E8, 0x150040E8 */
	CAM_REG_CTL_P2_DONE_BYP                         CAM_CTL_P2_DONE_BYP;                             /* 40EC, 0x150040EC */
	CAM_REG_CTL_IMGO_FBC                            CAM_CTL_IMGO_FBC;                                /* 40F0, 0x150040F0 */
	CAM_REG_CTL_RRZO_FBC                            CAM_CTL_RRZO_FBC;                                /* 40F4, 0x150040F4 */
	CAM_REG_CTL_IMGO_D_FBC                          CAM_CTL_IMGO_D_FBC;                              /* 40F8, 0x150040F8 */
	CAM_REG_CTL_RRZO_D_FBC                          CAM_CTL_RRZO_D_FBC;                              /* 40FC, 0x150040FC */
	CAM_REG_CTL_SPARE3                              CAM_CTL_SPARE3;                                  /* 4100, 0x15004100 */
	CAM_REG_CTL_IHDR                                CAM_CTL_IHDR;                                    /* 4104, 0x15004104 */
	CAM_REG_CTL_IHDR_D                              CAM_CTL_IHDR_D;                                  /* 4108, 0x15004108 */
	CAM_REG_CTL_CQ_EN_P2                            CAM_CTL_CQ_EN_P2;                                /* 410C, 0x1500410C */
	CAM_REG_CTL_SEL_GLOBAL_P2                       CAM_CTL_SEL_GLOBAL_P2;                           /* 4110, 0x15004110 */
	UINT32                                          rsv_4114[19];                                    /* 4114..415F, 0x15004114..1500415F */
	CAM_REG_CTL_DBG_SET                             CAM_CTL_DBG_SET;                                 /* 4160, 0x15004160 */
	CAM_REG_CTL_DBG_PORT                            CAM_CTL_DBG_PORT;                                /* 4164, 0x15004164 */
	CAM_REG_CTL_IMGI_CHECK                          CAM_CTL_IMGI_CHECK;                              /* 4168, 0x15004168 */
	CAM_REG_CTL_IMGO_CHECK                          CAM_CTL_IMGO_CHECK;                              /* 416C, 0x1500416C */
	CAM_REG_CTL_CLK_EN                              CAM_CTL_CLK_EN;                                  /* 4170, 0x15004170 */
	UINT32                                          rsv_4174[3];                                     /* 4174..417F, 0x15004174..1500417F */
	CAM_REG_CTL_DATE_CODE                           CAM_CTL_DATE_CODE;                               /* 4180, 0x15004180 */
	CAM_REG_CTL_PROJ_CODE                           CAM_CTL_PROJ_CODE;                               /* 4184, 0x15004184 */
	CAM_REG_CTL_RAW_DCM_DIS                         CAM_CTL_RAW_DCM_DIS;                             /* 4188, 0x15004188 */
	CAM_REG_CTL_RAW_D_DCM_DIS                       CAM_CTL_RAW_D_DCM_DIS;                           /* 418C, 0x1500418C */
	CAM_REG_CTL_DMA_DCM_DIS                         CAM_CTL_DMA_DCM_DIS;                             /* 4190, 0x15004190 */
	CAM_REG_CTL_RGB_DCM_DIS                         CAM_CTL_RGB_DCM_DIS;                             /* 4194, 0x15004194 */
	CAM_REG_CTL_YUV_DCM_DIS                         CAM_CTL_YUV_DCM_DIS;                             /* 4198, 0x15004198 */
	CAM_REG_CTL_TOP_DCM_DIS                         CAM_CTL_TOP_DCM_DIS;                             /* 419C, 0x1500419C */
	CAM_REG_CTL_RAW_DCM_STATUS                      CAM_CTL_RAW_DCM_STATUS;                          /* 41A0, 0x150041A0 */
	CAM_REG_CTL_RAW_D_DCM_STATUS                    CAM_CTL_RAW_D_DCM_STATUS;                        /* 41A4, 0x150041A4 */
	CAM_REG_CTL_DMA_DCM_STATUS                      CAM_CTL_DMA_DCM_STATUS;                          /* 41A8, 0x150041A8 */
	CAM_REG_CTL_RGB_DCM_STATUS                      CAM_CTL_RGB_DCM_STATUS;                          /* 41AC, 0x150041AC */
	CAM_REG_CTL_YUV_DCM_STATUS                      CAM_CTL_YUV_DCM_STATUS;                          /* 41B0, 0x150041B0 */
	CAM_REG_CTL_TOP_DCM_STATUS                      CAM_CTL_TOP_DCM_STATUS;                          /* 41B4, 0x150041B4 */
	CAM_REG_CTL_RAW_REQ_STATUS                      CAM_CTL_RAW_REQ_STATUS;                          /* 41B8, 0x150041B8 */
	CAM_REG_CTL_RAW_D_REQ_STATUS                    CAM_CTL_RAW_D_REQ_STATUS;                        /* 41BC, 0x150041BC */
	CAM_REG_CTL_DMA_REQ_STATUS                      CAM_CTL_DMA_REQ_STATUS;                          /* 41C0, 0x150041C0 */
	CAM_REG_CTL_RGB_REQ_STATUS                      CAM_CTL_RGB_REQ_STATUS;                          /* 41C4, 0x150041C4 */
	CAM_REG_CTL_YUV_REQ_STATUS                      CAM_CTL_YUV_REQ_STATUS;                          /* 41C8, 0x150041C8 */
	CAM_REG_CTL_RAW_RDY_STATUS                      CAM_CTL_RAW_RDY_STATUS;                          /* 41CC, 0x150041CC */
	CAM_REG_CTL_RAW_D_RDY_STATUS                    CAM_CTL_RAW_D_RDY_STATUS;                        /* 41D0, 0x150041D0 */
	CAM_REG_CTL_DMA_RDY_STATUS                      CAM_CTL_DMA_RDY_STATUS;                          /* 41D4, 0x150041D4 */
	CAM_REG_CTL_RGB_RDY_STATUS                      CAM_CTL_RGB_RDY_STATUS;                          /* 41D8, 0x150041D8 */
	CAM_REG_CTL_YUV_RDY_STATUS                      CAM_CTL_YUV_RDY_STATUS;                          /* 41DC, 0x150041DC */
	UINT32                                          rsv_41E0[140];                                   /* 41E0..440F, 0x150041E0..1500440F */
	CAM_REG_TG_SEN_MODE                             CAM_TG_SEN_MODE;                                 /* 4410, 0x15004410 */
	CAM_REG_TG_VF_CON                               CAM_TG_VF_CON;                                   /* 4414, 0x15004414 */
	CAM_REG_TG_SEN_GRAB_PXL                         CAM_TG_SEN_GRAB_PXL;                             /* 4418, 0x15004418 */
	CAM_REG_TG_SEN_GRAB_LIN                         CAM_TG_SEN_GRAB_LIN;                             /* 441C, 0x1500441C */
	CAM_REG_TG_PATH_CFG                             CAM_TG_PATH_CFG;                                 /* 4420, 0x15004420 */
	CAM_REG_TG_MEMIN_CTL                            CAM_TG_MEMIN_CTL;                                /* 4424, 0x15004424 */
	CAM_REG_TG_INT1                                 CAM_TG_INT1;                                     /* 4428, 0x15004428 */
	CAM_REG_TG_INT2                                 CAM_TG_INT2;                                     /* 442C, 0x1500442C */
	CAM_REG_TG_SOF_CNT                              CAM_TG_SOF_CNT;                                  /* 4430, 0x15004430 */
	CAM_REG_TG_SOT_CNT                              CAM_TG_SOT_CNT;                                  /* 4434, 0x15004434 */
	CAM_REG_TG_EOT_CNT                              CAM_TG_EOT_CNT;                                  /* 4438, 0x15004438 */
	CAM_REG_TG_ERR_CTL                              CAM_TG_ERR_CTL;                                  /* 443C, 0x1500443C */
	CAM_REG_TG_DAT_NO                               CAM_TG_DAT_NO;                                   /* 4440, 0x15004440 */
	CAM_REG_TG_FRM_CNT_ST                           CAM_TG_FRM_CNT_ST;                               /* 4444, 0x15004444 */
	CAM_REG_TG_FRMSIZE_ST                           CAM_TG_FRMSIZE_ST;                               /* 4448, 0x15004448 */
	CAM_REG_TG_INTER_ST                             CAM_TG_INTER_ST;                                 /* 444C, 0x1500444C */
	UINT32                                          rsv_4450[4];                                     /* 4450..445F, 0x15004450..1500445F */
	CAM_REG_TG_FLASHA_CTL                           CAM_TG_FLASHA_CTL;                               /* 4460, 0x15004460 */
	CAM_REG_TG_FLASHA_LINE_CNT                      CAM_TG_FLASHA_LINE_CNT;                          /* 4464, 0x15004464 */
	CAM_REG_TG_FLASHA_POS                           CAM_TG_FLASHA_POS;                               /* 4468, 0x15004468 */
	CAM_REG_TG_FLASHB_CTL                           CAM_TG_FLASHB_CTL;                               /* 446C, 0x1500446C */
	CAM_REG_TG_FLASHB_LINE_CNT                      CAM_TG_FLASHB_LINE_CNT;                          /* 4470, 0x15004470 */
	CAM_REG_TG_FLASHB_POS                           CAM_TG_FLASHB_POS;                               /* 4474, 0x15004474 */
	CAM_REG_TG_FLASHB_POS1                          CAM_TG_FLASHB_POS1;                              /* 4478, 0x15004478 */
	CAM_REG_TG_GSCTRL_CTL                           CAM_TG_GSCTRL_CTL;                               /* 447C, 0x1500447C */
	CAM_REG_TG_GSCTRL_TIME                          CAM_TG_GSCTRL_TIME;                              /* 4480, 0x15004480 */
	CAM_REG_TG_MS_PHASE                             CAM_TG_MS_PHASE;                                 /* 4484, 0x15004484 */
	CAM_REG_TG_MS_CL_TIME                           CAM_TG_MS_CL_TIME;                               /* 4488, 0x15004488 */
	CAM_REG_TG_MS_OP_TIME                           CAM_TG_MS_OP_TIME;                               /* 448C, 0x1500448C */
	CAM_REG_TG_MS_CLPH_TIME                         CAM_TG_MS_CLPH_TIME;                             /* 4490, 0x15004490 */
	CAM_REG_TG_MS_OPPH_TIME                         CAM_TG_MS_OPPH_TIME;                             /* 4494, 0x15004494 */
	UINT32                                          rsv_4498[22];                                    /* 4498..44EF, 0x15004498..150044EF */
	CAM_REG_HBN_SIZE                                CAM_HBN_SIZE;                                    /* 44F0, 0x150044F0 */
	CAM_REG_HBN_MODE                                CAM_HBN_MODE;                                    /* 44F4, 0x150044F4 */
	CAM_REG_HBN2_SIZE                               CAM_HBN2_SIZE;                                   /* 44F8, 0x150044F8 */
	CAM_REG_HBN2_MODE                               CAM_HBN2_MODE;                                   /* 44FC, 0x150044FC */
	CAM_REG_OBC_OFFST0                              CAM_OBC_OFFST0;                                  /* 4500, 0x15004500 */
	CAM_REG_OBC_OFFST1                              CAM_OBC_OFFST1;                                  /* 4504, 0x15004504 */
	CAM_REG_OBC_OFFST2                              CAM_OBC_OFFST2;                                  /* 4508, 0x15004508 */
	CAM_REG_OBC_OFFST3                              CAM_OBC_OFFST3;                                  /* 450C, 0x1500450C */
	CAM_REG_OBC_GAIN0                               CAM_OBC_GAIN0;                                   /* 4510, 0x15004510 */
	CAM_REG_OBC_GAIN1                               CAM_OBC_GAIN1;                                   /* 4514, 0x15004514 */
	CAM_REG_OBC_GAIN2                               CAM_OBC_GAIN2;                                   /* 4518, 0x15004518 */
	CAM_REG_OBC_GAIN3                               CAM_OBC_GAIN3;                                   /* 451C, 0x1500451C */
	UINT32                                          rsv_4520[4];                                     /* 4520..452F, 0x15004520..1500452F */
	CAM_REG_LSC_CTL1                                CAM_LSC_CTL1;                                    /* 4530, 0x15004530 */
	CAM_REG_LSC_CTL2                                CAM_LSC_CTL2;                                    /* 4534, 0x15004534 */
	CAM_REG_LSC_CTL3                                CAM_LSC_CTL3;                                    /* 4538, 0x15004538 */
	CAM_REG_LSC_LBLOCK                              CAM_LSC_LBLOCK;                                  /* 453C, 0x1500453C */
	CAM_REG_LSC_RATIO                               CAM_LSC_RATIO;                                   /* 4540, 0x15004540 */
	CAM_REG_LSC_TPIPE_OFST                          CAM_LSC_TPIPE_OFST;                              /* 4544, 0x15004544 */
	CAM_REG_LSC_TPIPE_SIZE                          CAM_LSC_TPIPE_SIZE;                              /* 4548, 0x15004548 */
	CAM_REG_LSC_GAIN_TH                             CAM_LSC_GAIN_TH;                                 /* 454C, 0x1500454C */
	CAM_REG_RPG_SATU_1                              CAM_RPG_SATU_1;                                  /* 4550, 0x15004550 */
	CAM_REG_RPG_SATU_2                              CAM_RPG_SATU_2;                                  /* 4554, 0x15004554 */
	CAM_REG_RPG_GAIN_1                              CAM_RPG_GAIN_1;                                  /* 4558, 0x15004558 */
	CAM_REG_RPG_GAIN_2                              CAM_RPG_GAIN_2;                                  /* 455C, 0x1500455C */
	CAM_REG_RPG_OFST_1                              CAM_RPG_OFST_1;                                  /* 4560, 0x15004560 */
	CAM_REG_RPG_OFST_2                              CAM_RPG_OFST_2;                                  /* 4564, 0x15004564 */
	UINT32                                          rsv_4568[2];                                     /* 4568..456F, 0x15004568..1500456F */
	CAM_REG_SGG3_PGN                                CAM_SGG3_PGN;                                    /* 4570, 0x15004570 */
	CAM_REG_SGG3_GMRC_1                             CAM_SGG3_GMRC_1;                                 /* 4574, 0x15004574 */
	CAM_REG_SGG3_GMRC_2                             CAM_SGG3_GMRC_2;                                 /* 4578, 0x15004578 */
	UINT32                                          rsv_457C;                                        /* 457C, 0x1500457C */
	CAM_REG_SGG2_PGN                                CAM_SGG2_PGN;                                    /* 4580, 0x15004580 */
	CAM_REG_SGG2_GMRC_1                             CAM_SGG2_GMRC_1;                                 /* 4584, 0x15004584 */
	CAM_REG_SGG2_GMRC_2                             CAM_SGG2_GMRC_2;                                 /* 4588, 0x15004588 */
	UINT32                                          rsv_458C;                                        /* 458C, 0x1500458C */
	CAM_REG_SGG5_PGN                                CAM_SGG5_PGN;                                    /* 4590, 0x15004590 */
	CAM_REG_SGG5_GMRC_1                             CAM_SGG5_GMRC_1;                                 /* 4594, 0x15004594 */
	CAM_REG_SGG5_GMRC_2                             CAM_SGG5_GMRC_2;                                 /* 4598, 0x15004598 */
	UINT32                                          rsv_459C[5];                                     /* 459C..45AF, 0x1500459C..150045AF */
	CAM_REG_AWB_WIN_ORG                             CAM_AWB_WIN_ORG;                                 /* 45B0, 0x150045B0 */
	CAM_REG_AWB_WIN_SIZE                            CAM_AWB_WIN_SIZE;                                /* 45B4, 0x150045B4 */
	CAM_REG_AWB_WIN_PIT                             CAM_AWB_WIN_PIT;                                 /* 45B8, 0x150045B8 */
	CAM_REG_AWB_WIN_NUM                             CAM_AWB_WIN_NUM;                                 /* 45BC, 0x150045BC */
	CAM_REG_AWB_GAIN1_0                             CAM_AWB_GAIN1_0;                                 /* 45C0, 0x150045C0 */
	CAM_REG_AWB_GAIN1_1                             CAM_AWB_GAIN1_1;                                 /* 45C4, 0x150045C4 */
	CAM_REG_AWB_LMT1_0                              CAM_AWB_LMT1_0;                                  /* 45C8, 0x150045C8 */
	CAM_REG_AWB_LMT1_1                              CAM_AWB_LMT1_1;                                  /* 45CC, 0x150045CC */
	CAM_REG_AWB_LOW_THR                             CAM_AWB_LOW_THR;                                 /* 45D0, 0x150045D0 */
	CAM_REG_AWB_HI_THR                              CAM_AWB_HI_THR;                                  /* 45D4, 0x150045D4 */
	CAM_REG_AWB_PIXEL_CNT0                          CAM_AWB_PIXEL_CNT0;                              /* 45D8, 0x150045D8 */
	CAM_REG_AWB_PIXEL_CNT1                          CAM_AWB_PIXEL_CNT1;                              /* 45DC, 0x150045DC */
	CAM_REG_AWB_PIXEL_CNT2                          CAM_AWB_PIXEL_CNT2;                              /* 45E0, 0x150045E0 */
	CAM_REG_AWB_ERR_THR                             CAM_AWB_ERR_THR;                                 /* 45E4, 0x150045E4 */
	CAM_REG_AWB_ROT                                 CAM_AWB_ROT;                                     /* 45E8, 0x150045E8 */
	CAM_REG_AWB_L0_X                                CAM_AWB_L0_X;                                    /* 45EC, 0x150045EC */
	CAM_REG_AWB_L0_Y                                CAM_AWB_L0_Y;                                    /* 45F0, 0x150045F0 */
	CAM_REG_AWB_L1_X                                CAM_AWB_L1_X;                                    /* 45F4, 0x150045F4 */
	CAM_REG_AWB_L1_Y                                CAM_AWB_L1_Y;                                    /* 45F8, 0x150045F8 */
	CAM_REG_AWB_L2_X                                CAM_AWB_L2_X;                                    /* 45FC, 0x150045FC */
	CAM_REG_AWB_L2_Y                                CAM_AWB_L2_Y;                                    /* 4600, 0x15004600 */
	CAM_REG_AWB_L3_X                                CAM_AWB_L3_X;                                    /* 4604, 0x15004604 */
	CAM_REG_AWB_L3_Y                                CAM_AWB_L3_Y;                                    /* 4608, 0x15004608 */
	CAM_REG_AWB_L4_X                                CAM_AWB_L4_X;                                    /* 460C, 0x1500460C */
	CAM_REG_AWB_L4_Y                                CAM_AWB_L4_Y;                                    /* 4610, 0x15004610 */
	CAM_REG_AWB_L5_X                                CAM_AWB_L5_X;                                    /* 4614, 0x15004614 */
	CAM_REG_AWB_L5_Y                                CAM_AWB_L5_Y;                                    /* 4618, 0x15004618 */
	CAM_REG_AWB_L6_X                                CAM_AWB_L6_X;                                    /* 461C, 0x1500461C */
	CAM_REG_AWB_L6_Y                                CAM_AWB_L6_Y;                                    /* 4620, 0x15004620 */
	CAM_REG_AWB_L7_X                                CAM_AWB_L7_X;                                    /* 4624, 0x15004624 */
	CAM_REG_AWB_L7_Y                                CAM_AWB_L7_Y;                                    /* 4628, 0x15004628 */
	CAM_REG_AWB_L8_X                                CAM_AWB_L8_X;                                    /* 462C, 0x1500462C */
	CAM_REG_AWB_L8_Y                                CAM_AWB_L8_Y;                                    /* 4630, 0x15004630 */
	CAM_REG_AWB_L9_X                                CAM_AWB_L9_X;                                    /* 4634, 0x15004634 */
	CAM_REG_AWB_L9_Y                                CAM_AWB_L9_Y;                                    /* 4638, 0x15004638 */
	CAM_REG_AWB_SPARE                               CAM_AWB_SPARE;                                   /* 463C, 0x1500463C */
	UINT32                                          rsv_4640[4];                                     /* 4640..464F, 0x15004640..1500464F */
	CAM_REG_AE_HST_CTL                              CAM_AE_HST_CTL;                                  /* 4650, 0x15004650 */
	CAM_REG_AE_GAIN2_0                              CAM_AE_GAIN2_0;                                  /* 4654, 0x15004654 */
	CAM_REG_AE_GAIN2_1                              CAM_AE_GAIN2_1;                                  /* 4658, 0x15004658 */
	CAM_REG_AE_LMT2_0                               CAM_AE_LMT2_0;                                   /* 465C, 0x1500465C */
	CAM_REG_AE_LMT2_1                               CAM_AE_LMT2_1;                                   /* 4660, 0x15004660 */
	CAM_REG_AE_RC_CNV_0                             CAM_AE_RC_CNV_0;                                 /* 4664, 0x15004664 */
	CAM_REG_AE_RC_CNV_1                             CAM_AE_RC_CNV_1;                                 /* 4668, 0x15004668 */
	CAM_REG_AE_RC_CNV_2                             CAM_AE_RC_CNV_2;                                 /* 466C, 0x1500466C */
	CAM_REG_AE_RC_CNV_3                             CAM_AE_RC_CNV_3;                                 /* 4670, 0x15004670 */
	CAM_REG_AE_RC_CNV_4                             CAM_AE_RC_CNV_4;                                 /* 4674, 0x15004674 */
	CAM_REG_AE_YGAMMA_0                             CAM_AE_YGAMMA_0;                                 /* 4678, 0x15004678 */
	CAM_REG_AE_YGAMMA_1                             CAM_AE_YGAMMA_1;                                 /* 467C, 0x1500467C */
	CAM_REG_AE_HST_SET                              CAM_AE_HST_SET;                                  /* 4680, 0x15004680 */
	CAM_REG_AE_HST0_RNG                             CAM_AE_HST0_RNG;                                 /* 4684, 0x15004684 */
	CAM_REG_AE_HST1_RNG                             CAM_AE_HST1_RNG;                                 /* 4688, 0x15004688 */
	CAM_REG_AE_HST2_RNG                             CAM_AE_HST2_RNG;                                 /* 468C, 0x1500468C */
	CAM_REG_AE_HST3_RNG                             CAM_AE_HST3_RNG;                                 /* 4690, 0x15004690 */
	CAM_REG_AE_SPARE                                CAM_AE_SPARE;                                    /* 4694, 0x15004694 */
	CAM_REG_AE_STAT_EN                              CAM_AE_STAT_EN;                                  /* 4698, 0x15004698 */
	UINT32                                          rsv_469C;                                        /* 469C, 0x1500469C */
	CAM_REG_SGG1_PGN                                CAM_SGG1_PGN;                                    /* 46A0, 0x150046A0 */
	CAM_REG_SGG1_GMRC_1                             CAM_SGG1_GMRC_1;                                 /* 46A4, 0x150046A4 */
	CAM_REG_SGG1_GMRC_2                             CAM_SGG1_GMRC_2;                                 /* 46A8, 0x150046A8 */
	UINT32                                          rsv_46AC;                                        /* 46AC, 0x150046AC */
	CAM_REG_AF_CON                                  CAM_AF_CON;                                      /* 46B0, 0x150046B0 */
	CAM_REG_AF_TH_0                                 CAM_AF_TH_0;                                     /* 46B4, 0x150046B4 */
	CAM_REG_AF_TH_1                                 CAM_AF_TH_1;                                     /* 46B8, 0x150046B8 */
	CAM_REG_AF_FLT_1                                CAM_AF_FLT_1;                                    /* 46BC, 0x150046BC */
	CAM_REG_AF_FLT_2                                CAM_AF_FLT_2;                                    /* 46C0, 0x150046C0 */
	CAM_REG_AF_FLT_3                                CAM_AF_FLT_3;                                    /* 46C4, 0x150046C4 */
	CAM_REG_AF_FLT_4                                CAM_AF_FLT_4;                                    /* 46C8, 0x150046C8 */
	CAM_REG_AF_FLT_5                                CAM_AF_FLT_5;                                    /* 46CC, 0x150046CC */
	CAM_REG_AF_FLT_6                                CAM_AF_FLT_6;                                    /* 46D0, 0x150046D0 */
	CAM_REG_AF_FLT_7                                CAM_AF_FLT_7;                                    /* 46D4, 0x150046D4 */
	CAM_REG_AF_FLT_8                                CAM_AF_FLT_8;                                    /* 46D8, 0x150046D8 */
	UINT32                                          rsv_46DC;                                        /* 46DC, 0x150046DC */
	CAM_REG_AF_SIZE                                 CAM_AF_SIZE;                                     /* 46E0, 0x150046E0 */
	CAM_REG_AF_VLD                                  CAM_AF_VLD;                                      /* 46E4, 0x150046E4 */
	CAM_REG_AF_BLK_0                                CAM_AF_BLK_0;                                    /* 46E8, 0x150046E8 */
	CAM_REG_AF_BLK_1                                CAM_AF_BLK_1;                                    /* 46EC, 0x150046EC */
	CAM_REG_AF_TH_2                                 CAM_AF_TH_2;                                     /* 46F0, 0x150046F0 */
	UINT32                                          rsv_46F4[31];                                    /* 46F4..476F, 0x150046F4..1500476F */
	CAM_REG_FLK_CON                                 CAM_FLK_CON;                                     /* 4770, 0x15004770 */
	CAM_REG_FLK_OFST                                CAM_FLK_OFST;                                    /* 4774, 0x15004774 */
	CAM_REG_FLK_SIZE                                CAM_FLK_SIZE;                                    /* 4778, 0x15004778 */
	CAM_REG_FLK_NUM                                 CAM_FLK_NUM;                                     /* 477C, 0x1500477C */
	UINT32                                          rsv_4780[8];                                     /* 4780..479F, 0x15004780..1500479F */
	CAM_REG_RRZ_CTL                                 CAM_RRZ_CTL;                                     /* 47A0, 0x150047A0 */
	CAM_REG_RRZ_IN_IMG                              CAM_RRZ_IN_IMG;                                  /* 47A4, 0x150047A4 */
	CAM_REG_RRZ_OUT_IMG                             CAM_RRZ_OUT_IMG;                                 /* 47A8, 0x150047A8 */
	CAM_REG_RRZ_HORI_STEP                           CAM_RRZ_HORI_STEP;                               /* 47AC, 0x150047AC */
	CAM_REG_RRZ_VERT_STEP                           CAM_RRZ_VERT_STEP;                               /* 47B0, 0x150047B0 */
	CAM_REG_RRZ_HORI_INT_OFST                       CAM_RRZ_HORI_INT_OFST;                           /* 47B4, 0x150047B4 */
	CAM_REG_RRZ_HORI_SUB_OFST                       CAM_RRZ_HORI_SUB_OFST;                           /* 47B8, 0x150047B8 */
	CAM_REG_RRZ_VERT_INT_OFST                       CAM_RRZ_VERT_INT_OFST;                           /* 47BC, 0x150047BC */
	CAM_REG_RRZ_VERT_SUB_OFST                       CAM_RRZ_VERT_SUB_OFST;                           /* 47C0, 0x150047C0 */
	CAM_REG_RRZ_MODE_TH                             CAM_RRZ_MODE_TH;                                 /* 47C4, 0x150047C4 */
	CAM_REG_RRZ_MODE_CTL                            CAM_RRZ_MODE_CTL;                                /* 47C8, 0x150047C8 */
	CAM_REG_RRZ_RLB_AOFST                           CAM_RRZ_RLB_AOFST;                               /* 47CC, 0x150047CC */
	UINT32                                          rsv_47D0[12];                                    /* 47D0..47FF, 0x150047D0..150047FF */
	CAM_REG_BNR_BPC_CON                             CAM_BNR_BPC_CON;                                 /* 4800, 0x15004800 */
	CAM_REG_BNR_BPC_TH1                             CAM_BNR_BPC_TH1;                                 /* 4804, 0x15004804 */
	CAM_REG_BNR_BPC_TH2                             CAM_BNR_BPC_TH2;                                 /* 4808, 0x15004808 */
	CAM_REG_BNR_BPC_TH3                             CAM_BNR_BPC_TH3;                                 /* 480C, 0x1500480C */
	CAM_REG_BNR_BPC_TH4                             CAM_BNR_BPC_TH4;                                 /* 4810, 0x15004810 */
	CAM_REG_BNR_BPC_DTC                             CAM_BNR_BPC_DTC;                                 /* 4814, 0x15004814 */
	CAM_REG_BNR_BPC_COR                             CAM_BNR_BPC_COR;                                 /* 4818, 0x15004818 */
	CAM_REG_BNR_BPC_TBLI1                           CAM_BNR_BPC_TBLI1;                               /* 481C, 0x1500481C */
	CAM_REG_BNR_BPC_TBLI2                           CAM_BNR_BPC_TBLI2;                               /* 4820, 0x15004820 */
	CAM_REG_BNR_BPC_TH1_C                           CAM_BNR_BPC_TH1_C;                               /* 4824, 0x15004824 */
	CAM_REG_BNR_BPC_TH2_C                           CAM_BNR_BPC_TH2_C;                               /* 4828, 0x15004828 */
	CAM_REG_BNR_BPC_TH3_C                           CAM_BNR_BPC_TH3_C;                               /* 482C, 0x1500482C */
	CAM_REG_BNR_NR1_CON                             CAM_BNR_NR1_CON;                                 /* 4830, 0x15004830 */
	CAM_REG_BNR_NR1_CT_CON                          CAM_BNR_NR1_CT_CON;                              /* 4834, 0x15004834 */
	CAM_REG_BNR_RSV1                                CAM_BNR_RSV1;                                    /* 4838, 0x15004838 */
	CAM_REG_BNR_RSV2                                CAM_BNR_RSV2;                                    /* 483C, 0x1500483C */
	CAM_REG_BNR_PDC_CON                             CAM_BNR_PDC_CON;                                 /* 4840, 0x15004840 */
	CAM_REG_BNR_PDC_GAIN_L0                         CAM_BNR_PDC_GAIN_L0;                             /* 4844, 0x15004844 */
	CAM_REG_BNR_PDC_GAIN_L1                         CAM_BNR_PDC_GAIN_L1;                             /* 4848, 0x15004848 */
	CAM_REG_BNR_PDC_GAIN_L2                         CAM_BNR_PDC_GAIN_L2;                             /* 484C, 0x1500484C */
	CAM_REG_BNR_PDC_GAIN_L3                         CAM_BNR_PDC_GAIN_L3;                             /* 4850, 0x15004850 */
	CAM_REG_BNR_PDC_GAIN_L4                         CAM_BNR_PDC_GAIN_L4;                             /* 4854, 0x15004854 */
	CAM_REG_BNR_PDC_GAIN_R0                         CAM_BNR_PDC_GAIN_R0;                             /* 4858, 0x15004858 */
	CAM_REG_BNR_PDC_GAIN_R1                         CAM_BNR_PDC_GAIN_R1;                             /* 485C, 0x1500485C */
	CAM_REG_BNR_PDC_GAIN_R2                         CAM_BNR_PDC_GAIN_R2;                             /* 4860, 0x15004860 */
	CAM_REG_BNR_PDC_GAIN_R3                         CAM_BNR_PDC_GAIN_R3;                             /* 4864, 0x15004864 */
	CAM_REG_BNR_PDC_GAIN_R4                         CAM_BNR_PDC_GAIN_R4;                             /* 4868, 0x15004868 */
	CAM_REG_BNR_PDC_TH_GB                           CAM_BNR_PDC_TH_GB;                               /* 486C, 0x1500486C */
	CAM_REG_BNR_PDC_TH_IA                           CAM_BNR_PDC_TH_IA;                               /* 4870, 0x15004870 */
	CAM_REG_BNR_PDC_TH_HD                           CAM_BNR_PDC_TH_HD;                               /* 4874, 0x15004874 */
	CAM_REG_BNR_PDC_SL                              CAM_BNR_PDC_SL;                                  /* 4878, 0x15004878 */
	CAM_REG_BNR_PDC_POS                             CAM_BNR_PDC_POS;                                 /* 487C, 0x1500487C */
	CAM_REG_PGN_SATU_1                              CAM_PGN_SATU_1;                                  /* 4880, 0x15004880 */
	CAM_REG_PGN_SATU_2                              CAM_PGN_SATU_2;                                  /* 4884, 0x15004884 */
	CAM_REG_PGN_GAIN_1                              CAM_PGN_GAIN_1;                                  /* 4888, 0x15004888 */
	CAM_REG_PGN_GAIN_2                              CAM_PGN_GAIN_2;                                  /* 488C, 0x1500488C */
	CAM_REG_PGN_OFST_1                              CAM_PGN_OFST_1;                                  /* 4890, 0x15004890 */
	CAM_REG_PGN_OFST_2                              CAM_PGN_OFST_2;                                  /* 4894, 0x15004894 */
	UINT32                                          rsv_4898[2];                                     /* 4898..489F, 0x15004898..1500489F */
	CAM_REG_DM_O_BYP                                CAM_DM_O_BYP;                                    /* 48A0, 0x150048A0 */
	CAM_REG_DM_O_ED_FLAT                            CAM_DM_O_ED_FLAT;                                /* 48A4, 0x150048A4 */
	CAM_REG_DM_O_ED_NYQ                             CAM_DM_O_ED_NYQ;                                 /* 48A8, 0x150048A8 */
	CAM_REG_DM_O_ED_STEP                            CAM_DM_O_ED_STEP;                                /* 48AC, 0x150048AC */
	CAM_REG_DM_O_RGB_HF                             CAM_DM_O_RGB_HF;                                 /* 48B0, 0x150048B0 */
	CAM_REG_DM_O_DOT                                CAM_DM_O_DOT;                                    /* 48B4, 0x150048B4 */
	CAM_REG_DM_O_F1_ACT                             CAM_DM_O_F1_ACT;                                 /* 48B8, 0x150048B8 */
	CAM_REG_DM_O_F2_ACT                             CAM_DM_O_F2_ACT;                                 /* 48BC, 0x150048BC */
	CAM_REG_DM_O_F3_ACT                             CAM_DM_O_F3_ACT;                                 /* 48C0, 0x150048C0 */
	CAM_REG_DM_O_F4_ACT                             CAM_DM_O_F4_ACT;                                 /* 48C4, 0x150048C4 */
	CAM_REG_DM_O_F1_L                               CAM_DM_O_F1_L;                                   /* 48C8, 0x150048C8 */
	CAM_REG_DM_O_F2_L                               CAM_DM_O_F2_L;                                   /* 48CC, 0x150048CC */
	CAM_REG_DM_O_F3_L                               CAM_DM_O_F3_L;                                   /* 48D0, 0x150048D0 */
	CAM_REG_DM_O_F4_L                               CAM_DM_O_F4_L;                                   /* 48D4, 0x150048D4 */
	CAM_REG_DM_O_HF_RB                              CAM_DM_O_HF_RB;                                  /* 48D8, 0x150048D8 */
	CAM_REG_DM_O_HF_GAIN                            CAM_DM_O_HF_GAIN;                                /* 48DC, 0x150048DC */
	CAM_REG_DM_O_HF_COMP                            CAM_DM_O_HF_COMP;                                /* 48E0, 0x150048E0 */
	CAM_REG_DM_O_HF_CORIN_TH                        CAM_DM_O_HF_CORIN_TH;                            /* 48E4, 0x150048E4 */
	CAM_REG_DM_O_ACT_LUT                            CAM_DM_O_ACT_LUT;                                /* 48E8, 0x150048E8 */
	UINT32                                          rsv_48EC;                                        /* 48EC, 0x150048EC */
	CAM_REG_DM_O_SPARE                              CAM_DM_O_SPARE;                                  /* 48F0, 0x150048F0 */
	CAM_REG_DM_O_BB                                 CAM_DM_O_BB;                                     /* 48F4, 0x150048F4 */
	UINT32                                          rsv_48F8[6];                                     /* 48F8..490F, 0x150048F8..1500490F */
	CAM_REG_CCL_GTC                                 CAM_CCL_GTC;                                     /* 4910, 0x15004910 */
	CAM_REG_CCL_ADC                                 CAM_CCL_ADC;                                     /* 4914, 0x15004914 */
	CAM_REG_CCL_BAC                                 CAM_CCL_BAC;                                     /* 4918, 0x15004918 */
	UINT32                                          rsv_491C;                                        /* 491C, 0x1500491C */
	CAM_REG_G2G_CNV_1                               CAM_G2G_CNV_1;                                   /* 4920, 0x15004920 */
	CAM_REG_G2G_CNV_2                               CAM_G2G_CNV_2;                                   /* 4924, 0x15004924 */
	CAM_REG_G2G_CNV_3                               CAM_G2G_CNV_3;                                   /* 4928, 0x15004928 */
	CAM_REG_G2G_CNV_4                               CAM_G2G_CNV_4;                                   /* 492C, 0x1500492C */
	CAM_REG_G2G_CNV_5                               CAM_G2G_CNV_5;                                   /* 4930, 0x15004930 */
	CAM_REG_G2G_CNV_6                               CAM_G2G_CNV_6;                                   /* 4934, 0x15004934 */
	CAM_REG_G2G_CTRL                                CAM_G2G_CTRL;                                    /* 4938, 0x15004938 */
	UINT32                                          rsv_493C[3];                                     /* 493C..4947, 0x1500493C..15004947 */
	CAM_REG_UNP_OFST                                CAM_UNP_OFST;                                    /* 4948, 0x15004948 */
	UINT32                                          rsv_494C[45];                                    /* 494C..49FF, 0x1500494C..150049FF */
	CAM_REG_CPG_SATU_1                              CAM_CPG_SATU_1;                                  /* 4A00, 0x15004A00 */
	CAM_REG_CPG_SATU_2                              CAM_CPG_SATU_2;                                  /* 4A04, 0x15004A04 */
	CAM_REG_CPG_GAIN_1                              CAM_CPG_GAIN_1;                                  /* 4A08, 0x15004A08 */
	CAM_REG_CPG_GAIN_2                              CAM_CPG_GAIN_2;                                  /* 4A0C, 0x15004A0C */
	CAM_REG_CPG_OFST_1                              CAM_CPG_OFST_1;                                  /* 4A10, 0x15004A10 */
	CAM_REG_CPG_OFST_2                              CAM_CPG_OFST_2;                                  /* 4A14, 0x15004A14 */
	UINT32                                          rsv_4A18;                                        /* 4A18, 0x15004A18 */
	CAM_REG_C42_CON                                 CAM_C42_CON;                                     /* 4A1C, 0x15004A1C */
	UINT32                                          rsv_4A20[32];                                    /* 4A20..4A9F, 0x15004A20..15004A9F */
	CAM_REG_SEEE_SRK_CTRL                           CAM_SEEE_SRK_CTRL;                               /* 4AA0, 0x15004AA0 */
	CAM_REG_SEEE_CLIP_CTRL                          CAM_SEEE_CLIP_CTRL;                              /* 4AA4, 0x15004AA4 */
	CAM_REG_SEEE_FLT_CTRL_1                         CAM_SEEE_FLT_CTRL_1;                             /* 4AA8, 0x15004AA8 */
	CAM_REG_SEEE_FLT_CTRL_2                         CAM_SEEE_FLT_CTRL_2;                             /* 4AAC, 0x15004AAC */
	CAM_REG_SEEE_GLUT_CTRL_01                       CAM_SEEE_GLUT_CTRL_01;                           /* 4AB0, 0x15004AB0 */
	CAM_REG_SEEE_GLUT_CTRL_02                       CAM_SEEE_GLUT_CTRL_02;                           /* 4AB4, 0x15004AB4 */
	CAM_REG_SEEE_GLUT_CTRL_03                       CAM_SEEE_GLUT_CTRL_03;                           /* 4AB8, 0x15004AB8 */
	CAM_REG_SEEE_GLUT_CTRL_04                       CAM_SEEE_GLUT_CTRL_04;                           /* 4ABC, 0x15004ABC */
	CAM_REG_SEEE_GLUT_CTRL_05                       CAM_SEEE_GLUT_CTRL_05;                           /* 4AC0, 0x15004AC0 */
	CAM_REG_SEEE_GLUT_CTRL_06                       CAM_SEEE_GLUT_CTRL_06;                           /* 4AC4, 0x15004AC4 */
	CAM_REG_SEEE_EDTR_CTRL                          CAM_SEEE_EDTR_CTRL;                              /* 4AC8, 0x15004AC8 */
	CAM_REG_SEEE_OUT_EDGE_CTRL                      CAM_SEEE_OUT_EDGE_CTRL;                          /* 4ACC, 0x15004ACC */
	CAM_REG_SEEE_SE_Y_CTRL                          CAM_SEEE_SE_Y_CTRL;                              /* 4AD0, 0x15004AD0 */
	CAM_REG_SEEE_SE_EDGE_CTRL_1                     CAM_SEEE_SE_EDGE_CTRL_1;                         /* 4AD4, 0x15004AD4 */
	CAM_REG_SEEE_SE_EDGE_CTRL_2                     CAM_SEEE_SE_EDGE_CTRL_2;                         /* 4AD8, 0x15004AD8 */
	CAM_REG_SEEE_SE_EDGE_CTRL_3                     CAM_SEEE_SE_EDGE_CTRL_3;                         /* 4ADC, 0x15004ADC */
	CAM_REG_SEEE_SE_SPECL_CTRL                      CAM_SEEE_SE_SPECL_CTRL;                          /* 4AE0, 0x15004AE0 */
	CAM_REG_SEEE_SE_CORE_CTRL_1                     CAM_SEEE_SE_CORE_CTRL_1;                         /* 4AE4, 0x15004AE4 */
	CAM_REG_SEEE_SE_CORE_CTRL_2                     CAM_SEEE_SE_CORE_CTRL_2;                         /* 4AE8, 0x15004AE8 */
	CAM_REG_SEEE_GLUT_CTRL_07                       CAM_SEEE_GLUT_CTRL_07;                           /* 4AEC, 0x15004AEC */
	CAM_REG_SEEE_GLUT_CTRL_08                       CAM_SEEE_GLUT_CTRL_08;                           /* 4AF0, 0x15004AF0 */
	CAM_REG_SEEE_GLUT_CTRL_09                       CAM_SEEE_GLUT_CTRL_09;                           /* 4AF4, 0x15004AF4 */
	CAM_REG_SEEE_GLUT_CTRL_10                       CAM_SEEE_GLUT_CTRL_10;                           /* 4AF8, 0x15004AF8 */
	CAM_REG_SEEE_GLUT_CTRL_11                       CAM_SEEE_GLUT_CTRL_11;                           /* 4AFC, 0x15004AFC */
	CAM_REG_CRZ_CONTROL                             CAM_CRZ_CONTROL;                                 /* 4B00, 0x15004B00 */
	CAM_REG_CRZ_IN_IMG                              CAM_CRZ_IN_IMG;                                  /* 4B04, 0x15004B04 */
	CAM_REG_CRZ_OUT_IMG                             CAM_CRZ_OUT_IMG;                                 /* 4B08, 0x15004B08 */
	CAM_REG_CRZ_HORI_STEP                           CAM_CRZ_HORI_STEP;                               /* 4B0C, 0x15004B0C */
	CAM_REG_CRZ_VERT_STEP                           CAM_CRZ_VERT_STEP;                               /* 4B10, 0x15004B10 */
	CAM_REG_CRZ_LUMA_HORI_INT_OFST                  CAM_CRZ_LUMA_HORI_INT_OFST;                      /* 4B14, 0x15004B14 */
	CAM_REG_CRZ_LUMA_HORI_SUB_OFST                  CAM_CRZ_LUMA_HORI_SUB_OFST;                      /* 4B18, 0x15004B18 */
	CAM_REG_CRZ_LUMA_VERT_INT_OFST                  CAM_CRZ_LUMA_VERT_INT_OFST;                      /* 4B1C, 0x15004B1C */
	CAM_REG_CRZ_LUMA_VERT_SUB_OFST                  CAM_CRZ_LUMA_VERT_SUB_OFST;                      /* 4B20, 0x15004B20 */
	CAM_REG_CRZ_CHRO_HORI_INT_OFST                  CAM_CRZ_CHRO_HORI_INT_OFST;                      /* 4B24, 0x15004B24 */
	CAM_REG_CRZ_CHRO_HORI_SUB_OFST                  CAM_CRZ_CHRO_HORI_SUB_OFST;                      /* 4B28, 0x15004B28 */
	CAM_REG_CRZ_CHRO_VERT_INT_OFST                  CAM_CRZ_CHRO_VERT_INT_OFST;                      /* 4B2C, 0x15004B2C */
	CAM_REG_CRZ_CHRO_VERT_SUB_OFST                  CAM_CRZ_CHRO_VERT_SUB_OFST;                      /* 4B30, 0x15004B30 */
	CAM_REG_CRZ_DER_1                               CAM_CRZ_DER_1;                                   /* 4B34, 0x15004B34 */
	CAM_REG_CRZ_DER_2                               CAM_CRZ_DER_2;                                   /* 4B38, 0x15004B38 */
	UINT32                                          rsv_4B3C[25];                                    /* 4B3C..4B9F, 0x15004B3C..15004B9F */
	CAM_REG_G2C_CONV_0A                             CAM_G2C_CONV_0A;                                 /* 4BA0, 0x15004BA0 */
	CAM_REG_G2C_CONV_0B                             CAM_G2C_CONV_0B;                                 /* 4BA4, 0x15004BA4 */
	CAM_REG_G2C_CONV_1A                             CAM_G2C_CONV_1A;                                 /* 4BA8, 0x15004BA8 */
	CAM_REG_G2C_CONV_1B                             CAM_G2C_CONV_1B;                                 /* 4BAC, 0x15004BAC */
	CAM_REG_G2C_CONV_2A                             CAM_G2C_CONV_2A;                                 /* 4BB0, 0x15004BB0 */
	CAM_REG_G2C_CONV_2B                             CAM_G2C_CONV_2B;                                 /* 4BB4, 0x15004BB4 */
	CAM_REG_G2C_SHADE_CON_1                         CAM_G2C_SHADE_CON_1;                             /* 4BB8, 0x15004BB8 */
	CAM_REG_G2C_SHADE_CON_2                         CAM_G2C_SHADE_CON_2;                             /* 4BBC, 0x15004BBC */
	CAM_REG_G2C_SHADE_CON_3                         CAM_G2C_SHADE_CON_3;                             /* 4BC0, 0x15004BC0 */
	CAM_REG_G2C_SHADE_TAR                           CAM_G2C_SHADE_TAR;                               /* 4BC4, 0x15004BC4 */
	CAM_REG_G2C_SHADE_SP                            CAM_G2C_SHADE_SP;                                /* 4BC8, 0x15004BC8 */
	UINT32                                          rsv_4BCC[13];                                    /* 4BCC..4BFF, 0x15004BCC..15004BFF */
	CAM_REG_DBS_SIGMA                               CAM_DBS_SIGMA;                                   /* 4C00, 0x15004C00 */
	CAM_REG_DBS_BSTBL_0                             CAM_DBS_BSTBL_0;                                 /* 4C04, 0x15004C04 */
	CAM_REG_DBS_BSTBL_1                             CAM_DBS_BSTBL_1;                                 /* 4C08, 0x15004C08 */
	CAM_REG_DBS_BSTBL_2                             CAM_DBS_BSTBL_2;                                 /* 4C0C, 0x15004C0C */
	CAM_REG_DBS_BSTBL_3                             CAM_DBS_BSTBL_3;                                 /* 4C10, 0x15004C10 */
	CAM_REG_DBS_CTL                                 CAM_DBS_CTL;                                     /* 4C14, 0x15004C14 */
	CAM_REG_DBS_CTL_2                               CAM_DBS_CTL_2;                                   /* 4C18, 0x15004C18 */
	CAM_REG_DBS_SIGMA_2                             CAM_DBS_SIGMA_2;                                 /* 4C1C, 0x15004C1C */
	CAM_REG_DBS_YGN                                 CAM_DBS_YGN;                                     /* 4C20, 0x15004C20 */
	CAM_REG_DBS_SL_Y12                              CAM_DBS_SL_Y12;                                  /* 4C24, 0x15004C24 */
	CAM_REG_DBS_SL_Y34                              CAM_DBS_SL_Y34;                                  /* 4C28, 0x15004C28 */
	CAM_REG_DBS_SL_G12                              CAM_DBS_SL_G12;                                  /* 4C2C, 0x15004C2C */
	CAM_REG_DBS_SL_G34                              CAM_DBS_SL_G34;                                  /* 4C30, 0x15004C30 */
	UINT32                                          rsv_4C34[55];                                    /* 4C34..4D0F, 0x15004C34..15004D0F */
	CAM_REG_MDP_CROP_X                              CAM_MDP_CROP_X;                                  /* 4D10, 0x15004D10 */
	UINT32                                          rsv_4D14[3];                                     /* 4D14..4D1F, 0x15004D14..15004D1F */
	CAM_REG_MDP_CROP_Y                              CAM_MDP_CROP_Y;                                  /* 4D20, 0x15004D20 */
	UINT32                                          rsv_4D24[39];                                    /* 4D24..4DBF, 0x15004D24..15004DBF */
	CAM_REG_EIS_PREP_ME_CTRL1                       CAM_EIS_PREP_ME_CTRL1;                           /* 4DC0, 0x15004DC0 */
	CAM_REG_EIS_PREP_ME_CTRL2                       CAM_EIS_PREP_ME_CTRL2;                           /* 4DC4, 0x15004DC4 */
	CAM_REG_EIS_LMV_TH                              CAM_EIS_LMV_TH;                                  /* 4DC8, 0x15004DC8 */
	CAM_REG_EIS_FL_OFFSET                           CAM_EIS_FL_OFFSET;                               /* 4DCC, 0x15004DCC */
	CAM_REG_EIS_MB_OFFSET                           CAM_EIS_MB_OFFSET;                               /* 4DD0, 0x15004DD0 */
	CAM_REG_EIS_MB_INTERVAL                         CAM_EIS_MB_INTERVAL;                             /* 4DD4, 0x15004DD4 */
	CAM_REG_EIS_GMV                                 CAM_EIS_GMV;                                     /* 4DD8, 0x15004DD8 */
	CAM_REG_EIS_ERR_CTRL                            CAM_EIS_ERR_CTRL;                                /* 4DDC, 0x15004DDC */
	CAM_REG_EIS_IMAGE_CTRL                          CAM_EIS_IMAGE_CTRL;                              /* 4DE0, 0x15004DE0 */
	UINT32                                          rsv_4DE4[7];                                     /* 4DE4..4DFF, 0x15004DE4..15004DFF */
	CAM_REG_DMX_CTL                                 CAM_DMX_CTL;                                     /* 4E00, 0x15004E00 */
	CAM_REG_DMX_CROP                                CAM_DMX_CROP;                                    /* 4E04, 0x15004E04 */
	CAM_REG_DMX_VSIZE                               CAM_DMX_VSIZE;                                   /* 4E08, 0x15004E08 */
	UINT32                                          rsv_4E0C;                                        /* 4E0C, 0x15004E0C */
	CAM_REG_BMX_CTL                                 CAM_BMX_CTL;                                     /* 4E10, 0x15004E10 */
	CAM_REG_BMX_CROP                                CAM_BMX_CROP;                                    /* 4E14, 0x15004E14 */
	CAM_REG_BMX_VSIZE                               CAM_BMX_VSIZE;                                   /* 4E18, 0x15004E18 */
	UINT32                                          rsv_4E1C;                                        /* 4E1C, 0x15004E1C */
	CAM_REG_RMX_CTL                                 CAM_RMX_CTL;                                     /* 4E20, 0x15004E20 */
	CAM_REG_RMX_CROP                                CAM_RMX_CROP;                                    /* 4E24, 0x15004E24 */
	CAM_REG_RMX_VSIZE                               CAM_RMX_VSIZE;                                   /* 4E28, 0x15004E28 */
	UINT32                                          rsv_4E2C[69];                                    /* 4E2C..4F3F, 0x15004E2C..15004F3F */
	CAM_REG_SL2_CEN                                 CAM_SL2_CEN;                                     /* 4F40, 0x15004F40 */
	CAM_REG_SL2_MAX0_RR                             CAM_SL2_MAX0_RR;                                 /* 4F44, 0x15004F44 */
	CAM_REG_SL2_MAX1_RR                             CAM_SL2_MAX1_RR;                                 /* 4F48, 0x15004F48 */
	CAM_REG_SL2_MAX2_RR                             CAM_SL2_MAX2_RR;                                 /* 4F4C, 0x15004F4C */
	CAM_REG_SL2_HRZ                                 CAM_SL2_HRZ;                                     /* 4F50, 0x15004F50 */
	CAM_REG_SL2_XOFF                                CAM_SL2_XOFF;                                    /* 4F54, 0x15004F54 */
	CAM_REG_SL2_YOFF                                CAM_SL2_YOFF;                                    /* 4F58, 0x15004F58 */
	UINT32                                          rsv_4F5C;                                        /* 4F5C, 0x15004F5C */
	CAM_REG_NSL2A_CEN                               CAM_NSL2A_CEN;                                   /* 4F60, 0x15004F60 */
	CAM_REG_NSL2A_RR_CON0                           CAM_NSL2A_RR_CON0;                               /* 4F64, 0x15004F64 */
	CAM_REG_NSL2A_RR_CON1                           CAM_NSL2A_RR_CON1;                               /* 4F68, 0x15004F68 */
	CAM_REG_NSL2A_GAIN                              CAM_NSL2A_GAIN;                                  /* 4F6C, 0x15004F6C */
	CAM_REG_NSL2A_RZ                                CAM_NSL2A_RZ;                                    /* 4F70, 0x15004F70 */
	CAM_REG_NSL2A_XOFF                              CAM_NSL2A_XOFF;                                  /* 4F74, 0x15004F74 */
	CAM_REG_NSL2A_YOFF                              CAM_NSL2A_YOFF;                                  /* 4F78, 0x15004F78 */
	CAM_REG_NSL2A_SLP_CON0                          CAM_NSL2A_SLP_CON0;                              /* 4F7C, 0x15004F7C */
	CAM_REG_NSL2A_SLP_CON1                          CAM_NSL2A_SLP_CON1;                              /* 4F80, 0x15004F80 */
	CAM_REG_NSL2A_SIZE                              CAM_NSL2A_SIZE;                                  /* 4F84, 0x15004F84 */
	UINT32                                          rsv_4F88[14];                                    /* 4F88..4FBF, 0x15004F88..15004FBF */
	CAM_REG_SL2C_CEN                                CAM_SL2C_CEN;                                    /* 4FC0, 0x15004FC0 */
	CAM_REG_SL2C_MAX0_RR                            CAM_SL2C_MAX0_RR;                                /* 4FC4, 0x15004FC4 */
	CAM_REG_SL2C_MAX1_RR                            CAM_SL2C_MAX1_RR;                                /* 4FC8, 0x15004FC8 */
	CAM_REG_SL2C_MAX2_RR                            CAM_SL2C_MAX2_RR;                                /* 4FCC, 0x15004FCC */
	CAM_REG_SL2C_HRZ                                CAM_SL2C_HRZ;                                    /* 4FD0, 0x15004FD0 */
	CAM_REG_SL2C_XOFF                               CAM_SL2C_XOFF;                                   /* 4FD4, 0x15004FD4 */
	CAM_REG_SL2C_YOFF                               CAM_SL2C_YOFF;                                   /* 4FD8, 0x15004FD8 */
	UINT32                                          rsv_4FDC[9];                                     /* 4FDC..4FFF, 0x15004FDC..15004FFF */
	CAM_REG_GGM_LUT_RB                              CAM_GGM_LUT_RB[144];                             /* 5000..523F, 0x15005000..1500523F */
	CAM_REG_GGM_LUT_G                               CAM_GGM_LUT_G[144];                              /* 5240..547F, 0x15005240..1500547F */
	CAM_REG_GGM_CTRL                                CAM_GGM_CTRL;                                    /* 5480, 0x15005480 */
	UINT32                                          rsv_5484[223];                                   /* 5484..57FF, 0x15005484..150057FF */
	CAM_REG_PCA_TBL                                 CAM_PCA_TBL[360];                                /* 5800..5D9F, 0x15005800..15005D9F */
	UINT32                                          rsv_5DA0[24];                                    /* 5DA0..5DFF, 0x15005DA0..15005DFF */
	CAM_REG_PCA_CON1                                CAM_PCA_CON1;                                    /* 5E00, 0x15005E00 */
	CAM_REG_PCA_CON2                                CAM_PCA_CON2;                                    /* 5E04, 0x15005E04 */
	UINT32                                          rsv_5E08[122];                                   /* 5E08..5FEF, 0x15005E08..15005FEF */
	CAM_REG_TILE_RING_CON1                          CAM_TILE_RING_CON1;                              /* 5FF0, 0x15005FF0 */
	CAM_REG_CTL_IMGI_SIZE                           CAM_CTL_IMGI_SIZE;                               /* 5FF4, 0x15005FF4 */
	UINT32                                          rsv_5FF8[18];                                    /* 5FF8..603C, 0x15005FF8..0x1500603C */
	CAM_REG_CTL_SEL_GLOBAL_SET                      CAM_CTL_SEL_GLOBAL_SET;                          /* 6040, SET for 4040 */
	UINT32                                          rsv_6044[243];                                   /* 6044..640C, 0x15006044..0x1500640C */
	CAM_REG_TG2_SEN_MODE                            CAM_TG2_SEN_MODE;                                /* 6410, 0x15006410 */
	CAM_REG_TG2_VF_CON                              CAM_TG2_VF_CON;                                  /* 6414, 0x15006414 */
	CAM_REG_TG2_SEN_GRAB_PXL                        CAM_TG2_SEN_GRAB_PXL;                            /* 6418, 0x15006418 */
	CAM_REG_TG2_SEN_GRAB_LIN                        CAM_TG2_SEN_GRAB_LIN;                            /* 641C, 0x1500641C */
	CAM_REG_TG2_PATH_CFG                            CAM_TG2_PATH_CFG;                                /* 6420, 0x15006420 */
	CAM_REG_TG2_MEMIN_CTL                           CAM_TG2_MEMIN_CTL;                               /* 6424, 0x15006424 */
	CAM_REG_TG2_INT1                                CAM_TG2_INT1;                                    /* 6428, 0x15006428 */
	CAM_REG_TG2_INT2                                CAM_TG2_INT2;                                    /* 642C, 0x1500642C */
	CAM_REG_TG2_SOF_CNT                             CAM_TG2_SOF_CNT;                                 /* 6430, 0x15006430 */
	CAM_REG_TG2_SOT_CNT                             CAM_TG2_SOT_CNT;                                 /* 6434, 0x15006434 */
	CAM_REG_TG2_EOT_CNT                             CAM_TG2_EOT_CNT;                                 /* 6438, 0x15006438 */
	CAM_REG_TG2_ERR_CTL                             CAM_TG2_ERR_CTL;                                 /* 643C, 0x1500643C */
	CAM_REG_TG2_DAT_NO                              CAM_TG2_DAT_NO;                                  /* 6440, 0x15006440 */
	CAM_REG_TG2_FRM_CNT_ST                          CAM_TG2_FRM_CNT_ST;                              /* 6444, 0x15006444 */
	CAM_REG_TG2_FRMSIZE_ST                          CAM_TG2_FRMSIZE_ST;                              /* 6448, 0x15006448 */
	CAM_REG_TG2_INTER_ST                            CAM_TG2_INTER_ST;                                /* 644C, 0x1500644C */
	UINT32                                          rsv_6450[44];                                    /* 6450..64FF, 0x15006450..150064FF */
	CAM_REG_OBC_D_OFFST0                            CAM_OBC_D_OFFST0;                                /* 6500, 0x15006500 */
	CAM_REG_OBC_D_OFFST1                            CAM_OBC_D_OFFST1;                                /* 6504, 0x15006504 */
	CAM_REG_OBC_D_OFFST2                            CAM_OBC_D_OFFST2;                                /* 6508, 0x15006508 */
	CAM_REG_OBC_D_OFFST3                            CAM_OBC_D_OFFST3;                                /* 650C, 0x1500650C */
	CAM_REG_OBC_D_GAIN0                             CAM_OBC_D_GAIN0;                                 /* 6510, 0x15006510 */
	CAM_REG_OBC_D_GAIN1                             CAM_OBC_D_GAIN1;                                 /* 6514, 0x15006514 */
	CAM_REG_OBC_D_GAIN2                             CAM_OBC_D_GAIN2;                                 /* 6518, 0x15006518 */
	CAM_REG_OBC_D_GAIN3                             CAM_OBC_D_GAIN3;                                 /* 651C, 0x1500651C */
	UINT32                                          rsv_6520[4];                                     /* 6520..652F, 0x15006520..1500652F */
	CAM_REG_LSC_D_CTL1                              CAM_LSC_D_CTL1;                                  /* 6530, 0x15006530 */
	CAM_REG_LSC_D_CTL2                              CAM_LSC_D_CTL2;                                  /* 6534, 0x15006534 */
	CAM_REG_LSC_D_CTL3                              CAM_LSC_D_CTL3;                                  /* 6538, 0x15006538 */
	CAM_REG_LSC_D_LBLOCK                            CAM_LSC_D_LBLOCK;                                /* 653C, 0x1500653C */
	CAM_REG_LSC_D_RATIO                             CAM_LSC_D_RATIO;                                 /* 6540, 0x15006540 */
	CAM_REG_LSC_D_TPIPE_OFST                        CAM_LSC_D_TPIPE_OFST;                            /* 6544, 0x15006544 */
	CAM_REG_LSC_D_TPIPE_SIZE                        CAM_LSC_D_TPIPE_SIZE;                            /* 6548, 0x15006548 */
	CAM_REG_LSC_D_GAIN_TH                           CAM_LSC_D_GAIN_TH;                               /* 654C, 0x1500654C */
	CAM_REG_RPG_D_SATU_1                            CAM_RPG_D_SATU_1;                                /* 6550, 0x15006550 */
	CAM_REG_RPG_D_SATU_2                            CAM_RPG_D_SATU_2;                                /* 6554, 0x15006554 */
	CAM_REG_RPG_D_GAIN_1                            CAM_RPG_D_GAIN_1;                                /* 6558, 0x15006558 */
	CAM_REG_RPG_D_GAIN_2                            CAM_RPG_D_GAIN_2;                                /* 655C, 0x1500655C */
	CAM_REG_RPG_D_OFST_1                            CAM_RPG_D_OFST_1;                                /* 6560, 0x15006560 */
	CAM_REG_RPG_D_OFST_2                            CAM_RPG_D_OFST_2;                                /* 6564, 0x15006564 */
	UINT32                                          rsv_6568[142];                                   /* 6568..679F, 0x15006568..1500679F */
	CAM_REG_RRZ_D_CTL                               CAM_RRZ_D_CTL;                                   /* 67A0, 0x150067A0 */
	CAM_REG_RRZ_D_IN_IMG                            CAM_RRZ_D_IN_IMG;                                /* 67A4, 0x150067A4 */
	CAM_REG_RRZ_D_OUT_IMG                           CAM_RRZ_D_OUT_IMG;                               /* 67A8, 0x150067A8 */
	CAM_REG_RRZ_D_HORI_STEP                         CAM_RRZ_D_HORI_STEP;                             /* 67AC, 0x150067AC */
	CAM_REG_RRZ_D_VERT_STEP                         CAM_RRZ_D_VERT_STEP;                             /* 67B0, 0x150067B0 */
	CAM_REG_RRZ_D_HORI_INT_OFST                     CAM_RRZ_D_HORI_INT_OFST;                         /* 67B4, 0x150067B4 */
	CAM_REG_RRZ_D_HORI_SUB_OFST                     CAM_RRZ_D_HORI_SUB_OFST;                         /* 67B8, 0x150067B8 */
	CAM_REG_RRZ_D_VERT_INT_OFST                     CAM_RRZ_D_VERT_INT_OFST;                         /* 67BC, 0x150067BC */
	CAM_REG_RRZ_D_VERT_SUB_OFST                     CAM_RRZ_D_VERT_SUB_OFST;                         /* 67C0, 0x150067C0 */
	CAM_REG_RRZ_D_MODE_TH                           CAM_RRZ_D_MODE_TH;                               /* 67C4, 0x150067C4 */
	CAM_REG_RRZ_D_MODE_CTL                          CAM_RRZ_D_MODE_CTL;                              /* 67C8, 0x150067C8 */
	CAM_REG_RRZ_D_RLB_AOFST                         CAM_RRZ_D_RLB_AOFST;                             /* 67CC, 0x150067CC */
	UINT32                                          rsv_67D0[12];                                    /* 67D0..67FF, 0x150067D0..150067FF */
	CAM_REG_BNR_D_BPC_CON                           CAM_BNR_D_BPC_CON;                               /* 6800, 0x15006800 */
	CAM_REG_BNR_D_BPC_TH1                           CAM_BNR_D_BPC_TH1;                               /* 6804, 0x15006804 */
	CAM_REG_BNR_D_BPC_TH2                           CAM_BNR_D_BPC_TH2;                               /* 6808, 0x15006808 */
	CAM_REG_BNR_D_BPC_TH3                           CAM_BNR_D_BPC_TH3;                               /* 680C, 0x1500680C */
	CAM_REG_BNR_D_BPC_TH4                           CAM_BNR_D_BPC_TH4;                               /* 6810, 0x15006810 */
	CAM_REG_BNR_D_BPC_DTC                           CAM_BNR_D_BPC_DTC;                               /* 6814, 0x15006814 */
	CAM_REG_BNR_D_BPC_COR                           CAM_BNR_D_BPC_COR;                               /* 6818, 0x15006818 */
	CAM_REG_BNR_D_BPC_TBLI1                         CAM_BNR_D_BPC_TBLI1;                             /* 681C, 0x1500681C */
	CAM_REG_BNR_D_BPC_TBLI2                         CAM_BNR_D_BPC_TBLI2;                             /* 6820, 0x15006820 */
	CAM_REG_BNR_D_BPC_TH1_C                         CAM_BNR_D_BPC_TH1_C;                             /* 6824, 0x15006824 */
	CAM_REG_BNR_D_BPC_TH2_C                         CAM_BNR_D_BPC_TH2_C;                             /* 6828, 0x15006828 */
	CAM_REG_BNR_D_BPC_TH3_C                         CAM_BNR_D_BPC_TH3_C;                             /* 682C, 0x1500682C */
	CAM_REG_BNR_D_NR1_CON                           CAM_BNR_D_NR1_CON;                               /* 6830, 0x15006830 */
	CAM_REG_BNR_D_NR1_CT_CON                        CAM_BNR_D_NR1_CT_CON;                            /* 6834, 0x15006834 */
	CAM_REG_BNR_D_RSV1                              CAM_BNR_D_RSV1;                                  /* 6838, 0x15006838 */
	CAM_REG_BNR_D_RSV2                              CAM_BNR_D_RSV2;                                  /* 683C, 0x1500683C */
	CAM_REG_BNR_D_PDC_CON                           CAM_BNR_D_PDC_CON;                               /* 6840, 0x15006840 */
	CAM_REG_BNR_D_PDC_GAIN_L0                       CAM_BNR_D_PDC_GAIN_L0;                           /* 6844, 0x15006844 */
	CAM_REG_BNR_D_PDC_GAIN_L1                       CAM_BNR_D_PDC_GAIN_L1;                           /* 6848, 0x15006848 */
	CAM_REG_BNR_D_PDC_GAIN_L2                       CAM_BNR_D_PDC_GAIN_L2;                           /* 684C, 0x1500684C */
	CAM_REG_BNR_D_PDC_GAIN_L3                       CAM_BNR_D_PDC_GAIN_L3;                           /* 6850, 0x15006850 */
	CAM_REG_BNR_D_PDC_GAIN_L4                       CAM_BNR_D_PDC_GAIN_L4;                           /* 6854, 0x15006854 */
	CAM_REG_BNR_D_PDC_GAIN_R0                       CAM_BNR_D_PDC_GAIN_R0;                           /* 6858, 0x15006858 */
	CAM_REG_BNR_D_PDC_GAIN_R1                       CAM_BNR_D_PDC_GAIN_R1;                           /* 685C, 0x1500685C */
	CAM_REG_BNR_D_PDC_GAIN_R2                       CAM_BNR_D_PDC_GAIN_R2;                           /* 6860, 0x15006860 */
	CAM_REG_BNR_D_PDC_GAIN_R3                       CAM_BNR_D_PDC_GAIN_R3;                           /* 6864, 0x15006864 */
	CAM_REG_BNR_D_PDC_GAIN_R4                       CAM_BNR_D_PDC_GAIN_R4;                           /* 6868, 0x15006868 */
	CAM_REG_BNR_D_PDC_TH_GB                         CAM_BNR_D_PDC_TH_GB;                             /* 686C, 0x1500686C */
	CAM_REG_BNR_D_PDC_TH_IA                         CAM_BNR_D_PDC_TH_IA;                             /* 6870, 0x15006870 */
	CAM_REG_BNR_D_PDC_TH_HD                         CAM_BNR_D_PDC_TH_HD;                             /* 6874, 0x15006874 */
	CAM_REG_BNR_D_PDC_SL                            CAM_BNR_D_PDC_SL;                                /* 6878, 0x15006878 */
	CAM_REG_BNR_D_PDC_POS                           CAM_BNR_D_PDC_POS;                               /* 687C, 0x1500687C */
	UINT32                                          rsv_6880[96];                                    /* 6880..69FF, 0x15006880..150069FF */
	CAM_REG_CPG_D_SATU_1                            CAM_CPG_D_SATU_1;                                /* 6A00, 0x15006A00 */
	CAM_REG_CPG_D_SATU_2                            CAM_CPG_D_SATU_2;                                /* 6A04, 0x15006A04 */
	CAM_REG_CPG_D_GAIN_1                            CAM_CPG_D_GAIN_1;                                /* 6A08, 0x15006A08 */
	CAM_REG_CPG_D_GAIN_2                            CAM_CPG_D_GAIN_2;                                /* 6A0C, 0x15006A0C */
	CAM_REG_CPG_D_OFST_1                            CAM_CPG_D_OFST_1;                                /* 6A10, 0x15006A10 */
	CAM_REG_CPG_D_OFST_2                            CAM_CPG_D_OFST_2;                                /* 6A14, 0x15006A14 */
	UINT32                                          rsv_6A18[122];                                   /* 6A18..6BFF, 0x15006A18..15006BFF */
	CAM_REG_DBS_D_SIGMA                             CAM_DBS_D_SIGMA;                                 /* 6C00, 0x15006C00 */
	CAM_REG_DBS_D_BSTBL_0                           CAM_DBS_D_BSTBL_0;                               /* 6C04, 0x15006C04 */
	CAM_REG_DBS_D_BSTBL_1                           CAM_DBS_D_BSTBL_1;                               /* 6C08, 0x15006C08 */
	CAM_REG_DBS_D_BSTBL_2                           CAM_DBS_D_BSTBL_2;                               /* 6C0C, 0x15006C0C */
	CAM_REG_DBS_D_BSTBL_3                           CAM_DBS_D_BSTBL_3;                               /* 6C10, 0x15006C10 */
	CAM_REG_DBS_D_CTL                               CAM_DBS_D_CTL;                                   /* 6C14, 0x15006C14 */
	CAM_REG_DBS_D_CTL_2                             CAM_DBS_D_CTL_2;                                 /* 6C18, 0x15006C18 */
	CAM_REG_DBS_D_SIGMA_2                           CAM_DBS_D_SIGMA_2;                               /* 6C1C, 0x15006C1C */
	CAM_REG_DBS_D_YGN                               CAM_DBS_D_YGN;                                   /* 6C20, 0x15006C20 */
	CAM_REG_DBS_D_SL_Y12                            CAM_DBS_D_SL_Y12;                                /* 6C24, 0x15006C24 */
	CAM_REG_DBS_D_SL_Y34                            CAM_DBS_D_SL_Y34;                                /* 6C28, 0x15006C28 */
	CAM_REG_DBS_D_SL_G12                            CAM_DBS_D_SL_G12;                                /* 6C2C, 0x15006C2C */
	CAM_REG_DBS_D_SL_G34                            CAM_DBS_D_SL_G34;                                /* 6C30, 0x15006C30 */
	UINT32                                          rsv_6C34[115];                                   /* 6C34..6DFF, 0x15006C34..15006DFF */
	CAM_REG_DMX_D_CTL                               CAM_DMX_D_CTL;                                   /* 6E00, 0x15006E00 */
	CAM_REG_DMX_D_CROP                              CAM_DMX_D_CROP;                                  /* 6E04, 0x15006E04 */
	CAM_REG_DMX_D_VSIZE                             CAM_DMX_D_VSIZE;                                 /* 6E08, 0x15006E08 */
	UINT32                                          rsv_6E0C;                                        /* 6E0C, 0x15006E0C */
	CAM_REG_BMX_D_CTL                               CAM_BMX_D_CTL;                                   /* 6E10, 0x15006E10 */
	CAM_REG_BMX_D_CROP                              CAM_BMX_D_CROP;                                  /* 6E14, 0x15006E14 */
	CAM_REG_BMX_D_VSIZE                             CAM_BMX_D_VSIZE;                                 /* 6E18, 0x15006E18 */
	UINT32                                          rsv_6E1C;                                        /* 6E1C, 0x15006E1C */
	CAM_REG_RMX_D_CTL                               CAM_RMX_D_CTL;                                   /* 6E20, 0x15006E20 */
	CAM_REG_RMX_D_CROP                              CAM_RMX_D_CROP;                                  /* 6E24, 0x15006E24 */
	CAM_REG_RMX_D_VSIZE                             CAM_RMX_D_VSIZE;                                 /* 6E28, 0x15006E28 */
	UINT32                                          rsv_6E2C[133];                                   /* 6E2C...703C, 0x15006E2C..0x1500703C */
	CAM_REG_CTL_SEL_GLOBAL_CLR                      CAM_CTL_SEL_GLOBAL_CLR;                          /* 7040, CLR for 4040 */
	UINT32                                          rsv_7044[111];                                   /* 7044...71FC, 0x15007044..0x150071FC */
	CAM_REG_DMA_SOFT_RSTSTAT                        CAM_DMA_SOFT_RSTSTAT;                            /* 7200, 0x15007200 */
	CAM_REG_TDRI_BASE_ADDR                          CAM_TDRI_BASE_ADDR;                              /* 7204, 0x15007204 */
	CAM_REG_TDRI_OFST_ADDR                          CAM_TDRI_OFST_ADDR;                              /* 7208, 0x15007208 */
	CAM_REG_TDRI_XSIZE                              CAM_TDRI_XSIZE;                                  /* 720C, 0x1500720C */
	CAM_REG_CQ0I_BASE_ADDR                          CAM_CQ0I_BASE_ADDR;                              /* 7210, 0x15007210 */
	CAM_REG_CQ0I_XSIZE                              CAM_CQ0I_XSIZE;                                  /* 7214, 0x15007214 */
	CAM_REG_CQ0I_D_BASE_ADDR                        CAM_CQ0I_D_BASE_ADDR;                            /* 7218, 0x15007218 */
	CAM_REG_CQ0I_D_XSIZE                            CAM_CQ0I_D_XSIZE;                                /* 721C, 0x1500721C */
	CAM_REG_VERTICAL_FLIP_EN                        CAM_VERTICAL_FLIP_EN;                            /* 7220, 0x15007220 */
	CAM_REG_DMA_SOFT_RESET                          CAM_DMA_SOFT_RESET;                              /* 7224, 0x15007224 */
	CAM_REG_LAST_ULTRA_EN                           CAM_LAST_ULTRA_EN;                               /* 7228, 0x15007228 */
	CAM_REG_IMGI_SLOW_DOWN                          CAM_IMGI_SLOW_DOWN;                              /* 722C, 0x1500722C */
	CAM_REG_IMGI_BASE_ADDR                          CAM_IMGI_BASE_ADDR;                              /* 7230, 0x15007230 */
	CAM_REG_IMGI_OFST_ADDR                          CAM_IMGI_OFST_ADDR;                              /* 7234, 0x15007234 */
	CAM_REG_IMGI_XSIZE                              CAM_IMGI_XSIZE;                                  /* 7238, 0x15007238 */
	CAM_REG_IMGI_YSIZE                              CAM_IMGI_YSIZE;                                  /* 723C, 0x1500723C */
	CAM_REG_IMGI_STRIDE                             CAM_IMGI_STRIDE;                                 /* 7240, 0x15007240 */
	UINT32                                          rsv_7244;                                        /* 7244, 0x15007244 */
	CAM_REG_IMGI_CON                                CAM_IMGI_CON;                                    /* 7248, 0x15007248 */
	CAM_REG_IMGI_CON2                               CAM_IMGI_CON2;                                   /* 724C, 0x1500724C */
	CAM_REG_BPCI_BASE_ADDR                          CAM_BPCI_BASE_ADDR;                              /* 7250, 0x15007250 */
	CAM_REG_BPCI_OFST_ADDR                          CAM_BPCI_OFST_ADDR;                              /* 7254, 0x15007254 */
	CAM_REG_BPCI_XSIZE                              CAM_BPCI_XSIZE;                                  /* 7258, 0x15007258 */
	CAM_REG_BPCI_YSIZE                              CAM_BPCI_YSIZE;                                  /* 725C, 0x1500725C */
	CAM_REG_BPCI_STRIDE                             CAM_BPCI_STRIDE;                                 /* 7260, 0x15007260 */
	CAM_REG_BPCI_CON                                CAM_BPCI_CON;                                    /* 7264, 0x15007264 */
	CAM_REG_BPCI_CON2                               CAM_BPCI_CON2;                                   /* 7268, 0x15007268 */
	CAM_REG_LSCI_BASE_ADDR                          CAM_LSCI_BASE_ADDR;                              /* 726C, 0x1500726C */
	CAM_REG_LSCI_OFST_ADDR                          CAM_LSCI_OFST_ADDR;                              /* 7270, 0x15007270 */
	CAM_REG_LSCI_XSIZE                              CAM_LSCI_XSIZE;                                  /* 7274, 0x15007274 */
	CAM_REG_LSCI_YSIZE                              CAM_LSCI_YSIZE;                                  /* 7278, 0x15007278 */
	CAM_REG_LSCI_STRIDE                             CAM_LSCI_STRIDE;                                 /* 727C, 0x1500727C */
	CAM_REG_LSCI_CON                                CAM_LSCI_CON;                                    /* 7280, 0x15007280 */
	CAM_REG_LSCI_CON2                               CAM_LSCI_CON2;                                   /* 7284, 0x15007284 */
	UINT32                                          rsv_7288[30];                                    /* 7288..72FF, 0x15007288..150072FF */
	CAM_REG_IMGO_BASE_ADDR                          CAM_IMGO_BASE_ADDR;                              /* 7300, 0x15007300 */
	CAM_REG_IMGO_OFST_ADDR                          CAM_IMGO_OFST_ADDR;                              /* 7304, 0x15007304 */
	CAM_REG_IMGO_XSIZE                              CAM_IMGO_XSIZE;                                  /* 7308, 0x15007308 */
	CAM_REG_IMGO_YSIZE                              CAM_IMGO_YSIZE;                                  /* 730C, 0x1500730C */
	CAM_REG_IMGO_STRIDE                             CAM_IMGO_STRIDE;                                 /* 7310, 0x15007310 */
	CAM_REG_IMGO_CON                                CAM_IMGO_CON;                                    /* 7314, 0x15007314 */
	CAM_REG_IMGO_CON2                               CAM_IMGO_CON2;                                   /* 7318, 0x15007318 */
	CAM_REG_IMGO_CROP                               CAM_IMGO_CROP;                                   /* 731C, 0x1500731C */
	CAM_REG_RRZO_BASE_ADDR                          CAM_RRZO_BASE_ADDR;                              /* 7320, 0x15007320 */
	CAM_REG_RRZO_OFST_ADDR                          CAM_RRZO_OFST_ADDR;                              /* 7324, 0x15007324 */
	CAM_REG_RRZO_XSIZE                              CAM_RRZO_XSIZE;                                  /* 7328, 0x15007328 */
	CAM_REG_RRZO_YSIZE                              CAM_RRZO_YSIZE;                                  /* 732C, 0x1500732C */
	CAM_REG_RRZO_STRIDE                             CAM_RRZO_STRIDE;                                 /* 7330, 0x15007330 */
	CAM_REG_RRZO_CON                                CAM_RRZO_CON;                                    /* 7334, 0x15007334 */
	CAM_REG_RRZO_CON2                               CAM_RRZO_CON2;                                   /* 7338, 0x15007338 */
	CAM_REG_RRZO_CROP                               CAM_RRZO_CROP;                                   /* 733C, 0x1500733C */
	UINT32                                          rsv_7340[7];                                     /* 7340..735B, 0x15007340..1500735B */
	CAM_REG_EISO_BASE_ADDR                          CAM_EISO_BASE_ADDR;                              /* 735C, 0x1500735C */
	CAM_REG_EISO_XSIZE                              CAM_EISO_XSIZE;                                  /* 7360, 0x15007360 */
	CAM_REG_AFO_BASE_ADDR                           CAM_AFO_BASE_ADDR;                               /* 7364, 0x15007364 */
	CAM_REG_AFO_XSIZE                               CAM_AFO_XSIZE;                                   /* 7368, 0x15007368 */
	CAM_REG_ESFKO_BASE_ADDR                         CAM_ESFKO_BASE_ADDR;                             /* 736C, 0x1500736C */
	CAM_REG_ESFKO_XSIZE                             CAM_ESFKO_XSIZE;                                 /* 7370, 0x15007370 */
	CAM_REG_ESFKO_OFST_ADDR                         CAM_ESFKO_OFST_ADDR;                             /* 7374, 0x15007374 */
	CAM_REG_ESFKO_YSIZE                             CAM_ESFKO_YSIZE;                                 /* 7378, 0x15007378 */
	CAM_REG_ESFKO_STRIDE                            CAM_ESFKO_STRIDE;                                /* 737C, 0x1500737C */
	CAM_REG_ESFKO_CON                               CAM_ESFKO_CON;                                   /* 7380, 0x15007380 */
	CAM_REG_ESFKO_CON2                              CAM_ESFKO_CON2;                                  /* 7384, 0x15007384 */
	CAM_REG_AAO_BASE_ADDR                           CAM_AAO_BASE_ADDR;                               /* 7388, 0x15007388 */
	CAM_REG_AAO_OFST_ADDR                           CAM_AAO_OFST_ADDR;                               /* 738C, 0x1500738C */
	CAM_REG_AAO_XSIZE                               CAM_AAO_XSIZE;                                   /* 7390, 0x15007390 */
	CAM_REG_AAO_YSIZE                               CAM_AAO_YSIZE;                                   /* 7394, 0x15007394 */
	CAM_REG_AAO_STRIDE                              CAM_AAO_STRIDE;                                  /* 7398, 0x15007398 */
	CAM_REG_AAO_CON                                 CAM_AAO_CON;                                     /* 739C, 0x1500739C */
	CAM_REG_AAO_CON2                                CAM_AAO_CON2;                                    /* 73A0, 0x150073A0 */
	UINT32                                          rsv_73A4[39];                                    /* 73A4..743F, 0x150073A4..1500743F */
	CAM_REG_IMG2O_BASE_ADDR                         CAM_IMG2O_BASE_ADDR;                             /* 7440, 0x15007440 */
	CAM_REG_IMG2O_OFST_ADDR                         CAM_IMG2O_OFST_ADDR;                             /* 7444, 0x15007444 */
	CAM_REG_IMG2O_XSIZE                             CAM_IMG2O_XSIZE;                                 /* 7448, 0x15007448 */
	CAM_REG_IMG2O_YSIZE                             CAM_IMG2O_YSIZE;                                 /* 744C, 0x1500744C */
	CAM_REG_IMG2O_STRIDE                            CAM_IMG2O_STRIDE;                                /* 7450, 0x15007450 */
	CAM_REG_IMG2O_CON                               CAM_IMG2O_CON;                                   /* 7454, 0x15007454 */
	CAM_REG_IMG2O_CON2                              CAM_IMG2O_CON2;                                  /* 7458, 0x15007458 */
	CAM_REG_IMG2O_CROP                              CAM_IMG2O_CROP;                                  /* 745C, 0x1500745C */
	UINT32                                          rsv_7460[15];                                    /* 7460..749B, 0x15007460..1500749B */
	CAM_REG_BPCI_D_BASE_ADDR                        CAM_BPCI_D_BASE_ADDR;                            /* 749C, 0x1500749C */
	CAM_REG_BPCI_D_OFST_ADDR                        CAM_BPCI_D_OFST_ADDR;                            /* 74A0, 0x150074A0 */
	CAM_REG_BPCI_D_XSIZE                            CAM_BPCI_D_XSIZE;                                /* 74A4, 0x150074A4 */
	CAM_REG_BPCI_D_YSIZE                            CAM_BPCI_D_YSIZE;                                /* 74A8, 0x150074A8 */
	CAM_REG_BPCI_D_STRIDE                           CAM_BPCI_D_STRIDE;                               /* 74AC, 0x150074AC */
	CAM_REG_BPCI_D_CON                              CAM_BPCI_D_CON;                                  /* 74B0, 0x150074B0 */
	CAM_REG_BPCI_D_CON2                             CAM_BPCI_D_CON2;                                 /* 74B4, 0x150074B4 */
	CAM_REG_LSCI_D_BASE_ADDR                        CAM_LSCI_D_BASE_ADDR;                            /* 74B8, 0x150074B8 */
	CAM_REG_LSCI_D_OFST_ADDR                        CAM_LSCI_D_OFST_ADDR;                            /* 74BC, 0x150074BC */
	CAM_REG_LSCI_D_XSIZE                            CAM_LSCI_D_XSIZE;                                /* 74C0, 0x150074C0 */
	CAM_REG_LSCI_D_YSIZE                            CAM_LSCI_D_YSIZE;                                /* 74C4, 0x150074C4 */
	CAM_REG_LSCI_D_STRIDE                           CAM_LSCI_D_STRIDE;                               /* 74C8, 0x150074C8 */
	CAM_REG_LSCI_D_CON                              CAM_LSCI_D_CON;                                  /* 74CC, 0x150074CC */
	CAM_REG_LSCI_D_CON2                             CAM_LSCI_D_CON2;                                 /* 74D0, 0x150074D0 */
	UINT32                                          rsv_74D4[11];                                    /* 74D4..74FF, 0x150074D4..150074FF */
	CAM_REG_IMGI_CON3                               CAM_IMGI_CON3;                                   /* 7500, 0x15007500 */
	CAM_REG_BPCI_CON3                               CAM_BPCI_CON3;                                   /* 7504, 0x15007504 */
	CAM_REG_LSCI_CON3                               CAM_LSCI_CON3;                                   /* 7508, 0x15007508 */
	CAM_REG_IMGO_CON3                               CAM_IMGO_CON3;                                   /* 750C, 0x1500750C */
	CAM_REG_RRZO_CON3                               CAM_RRZO_CON3;                                   /* 7510, 0x15007510 */
	CAM_REG_ESFKO_CON3                              CAM_ESFKO_CON3;                                  /* 7514, 0x15007514 */
	CAM_REG_AAO_CON3                                CAM_AAO_CON3;                                    /* 7518, 0x15007518 */
	CAM_REG_IMG2O_CON3                              CAM_IMG2O_CON3;                                  /* 751C, 0x1500751C */
	CAM_REG_BPCI_D_CON3                             CAM_BPCI_D_CON3;                                 /* 7520, 0x15007520 */
	CAM_REG_LSCI_D_CON3                             CAM_LSCI_D_CON3;                                 /* 7524, 0x15007524 */
	CAM_REG_AFO_D_CON3                              CAM_AFO_D_CON3;                                  /* 7528, 0x15007528 */
	UINT32                                          rsv_752C;                                        /* 752C, 0x1500752C */
	CAM_REG_AFO_D_BASE_ADDR                         CAM_AFO_D_BASE_ADDR;                             /* 7530, 0x15007530 */
	CAM_REG_AFO_D_XSIZE                             CAM_AFO_D_XSIZE;                                 /* 7534, 0x15007534 */
	CAM_REG_AFO_D_OFST_ADDR                         CAM_AFO_D_OFST_ADDR;                             /* 7538, 0x15007538 */
	CAM_REG_AFO_D_YSIZE                             CAM_AFO_D_YSIZE;                                 /* 753C, 0x1500753C */
	CAM_REG_AFO_D_STRIDE                            CAM_AFO_D_STRIDE;                                /* 7540, 0x15007540 */
	CAM_REG_AFO_D_CON                               CAM_AFO_D_CON;                                   /* 7544, 0x15007544 */
	CAM_REG_AFO_D_CON2                              CAM_AFO_D_CON2;                                  /* 7548, 0x15007548 */
	UINT32                                          rsv_754C[7];                                     /* 754C..7567, 0x1500754C..15007567 */
	CAM_REG_DMA_ERR_CTRL                            CAM_DMA_ERR_CTRL;                                /* 7568, 0x15007568 */
	CAM_REG_IMGI_ERR_STAT                           CAM_IMGI_ERR_STAT;                               /* 756C, 0x1500756C */
	CAM_REG_BPCI_ERR_STAT                           CAM_BPCI_ERR_STAT;                               /* 7570, 0x15007570 */
	CAM_REG_LSCI_ERR_STAT                           CAM_LSCI_ERR_STAT;                               /* 7574, 0x15007574 */
	UINT32                                          rsv_7578[5];                                     /* 7578..758B, 0x15007578..1500758B */
	CAM_REG_IMGO_ERR_STAT                           CAM_IMGO_ERR_STAT;                               /* 758C, 0x1500758C */
	CAM_REG_RRZO_ERR_STAT                           CAM_RRZO_ERR_STAT;                               /* 7590, 0x15007590 */
	UINT32                                          rsv_7594;                                        /* 7594, 0x15007594 */
	CAM_REG_ESFKO_ERR_STAT                          CAM_ESFKO_ERR_STAT;                              /* 7598, 0x15007598 */
	CAM_REG_AAO_ERR_STAT                            CAM_AAO_ERR_STAT;                                /* 759C, 0x1500759C */
	UINT32                                          rsv_75A0[4];                                     /* 75A0..75AF, 0x150075A0..150075AF */
	CAM_REG_IMG2O_ERR_STAT                          CAM_IMG2O_ERR_STAT;                              /* 75B0, 0x150075B0 */
	UINT32                                          rsv_75B4[2];                                     /* 75B4..75BB, 0x150075B4..150075BB */
	CAM_REG_BPCI_D_ERR_STAT                         CAM_BPCI_D_ERR_STAT;                             /* 75BC, 0x150075BC */
	CAM_REG_LSCI_D_ERR_STAT                         CAM_LSCI_D_ERR_STAT;                             /* 75C0, 0x150075C0 */
	UINT32                                          rsv_75C4[3];                                     /* 75C4..75CF, 0x150075C4..150075CF */
	CAM_REG_AFO_D_ERR_STAT                          CAM_AFO_D_ERR_STAT;                              /* 75D0, 0x150075D0 */
	UINT32                                          rsv_75D4;                                        /* 75D4, 0x150075D4 */
	CAM_REG_DMA_DEBUG_ADDR                          CAM_DMA_DEBUG_ADDR;                              /* 75D8, 0x150075D8 */
	CAM_REG_DMA_RSV1                                CAM_RAW_MAGIC_NUM0;                              /* 75DC, 0x150075DC */
	CAM_REG_DMA_RSV2                                CAM_RRZ_CROP_IN;                                 /* 75E0, 0x150075E0 */
	CAM_REG_DMA_RSV3                                CAM_RAW_D_MAGIC_NUM0;                            /* 75E4, 0x150075E4 */
	CAM_REG_DMA_RSV4                                CAM_RRZ_CROP_IN_D;                               /* 75E8, 0x150075E8 */
	CAM_REG_DMA_RSV5                                CAM_DMA_RSV5;                                    /* 75EC, 0x150075EC */
	CAM_REG_DMA_RSV6                                CAM_DMA_RSV6;                                    /* 75F0, 0x150075F0 */
	CAM_REG_DMA_DEBUG_SEL                           CAM_DMA_DEBUG_SEL;                               /* 75F4, 0x150075F4 */
	CAM_REG_DMA_BW_SELF_TEST                        CAM_DMA_BW_SELF_TEST;                            /* 75F8, 0x150075F8 */
	UINT32                                          rsv_75FC;                                        /* 75FC, 0x150075FC */
	CAM_REG_ANR_TBL                                 CAM_ANR_TBL[256];                                /* 7600..79FF, 0x15007600..150079FF */
	CAM_REG_ANR_CON1                                CAM_ANR_CON1;                                    /* 7A00, 0x15007A00 */
	CAM_REG_ANR_CON2                                CAM_ANR_CON2;                                    /* 7A04, 0x15007A04 */
	CAM_REG_ANR_YAD1                                CAM_ANR_YAD1;                                    /* 7A08, 0x15007A08 */
	CAM_REG_ANR_YAD2                                CAM_ANR_YAD2;                                    /* 7A0C, 0x15007A0C */
	CAM_REG_ANR_Y4LUT1                              CAM_ANR_Y4LUT1;                                  /* 7A10, 0x15007A10 */
	CAM_REG_ANR_Y4LUT2                              CAM_ANR_Y4LUT2;                                  /* 7A14, 0x15007A14 */
	CAM_REG_ANR_Y4LUT3                              CAM_ANR_Y4LUT3;                                  /* 7A18, 0x15007A18 */
	CAM_REG_ANR_C4LUT1                              CAM_ANR_C4LUT1;                                  /* 7A1C, 0x15007A1C */
	CAM_REG_ANR_C4LUT2                              CAM_ANR_C4LUT2;                                  /* 7A20, 0x15007A20 */
	CAM_REG_ANR_C4LUT3                              CAM_ANR_C4LUT3;                                  /* 7A24, 0x15007A24 */
	CAM_REG_ANR_A4LUT2                              CAM_ANR_A4LUT2;                                  /* 7A28, 0x15007A28 */
	CAM_REG_ANR_A4LUT3                              CAM_ANR_A4LUT3;                                  /* 7A2C, 0x15007A2C */
	CAM_REG_ANR_L4LUT1                              CAM_ANR_L4LUT1;                                  /* 7A30, 0x15007A30 */
	CAM_REG_ANR_L4LUT2                              CAM_ANR_L4LUT2;                                  /* 7A34, 0x15007A34 */
	CAM_REG_ANR_L4LUT3                              CAM_ANR_L4LUT3;                                  /* 7A38, 0x15007A38 */
	CAM_REG_ANR_PTY                                 CAM_ANR_PTY;                                     /* 7A3C, 0x15007A3C */
	CAM_REG_ANR_CAD                                 CAM_ANR_CAD;                                     /* 7A40, 0x15007A40 */
	CAM_REG_ANR_PTC                                 CAM_ANR_PTC;                                     /* 7A44, 0x15007A44 */
	CAM_REG_ANR_LCE                                 CAM_ANR_LCE;                                     /* 7A48, 0x15007A48 */
	CAM_REG_ANR_MED1                                CAM_ANR_MED1;                                    /* 7A4C, 0x15007A4C */
	CAM_REG_ANR_MED2                                CAM_ANR_MED2;                                    /* 7A50, 0x15007A50 */
	CAM_REG_ANR_MED3                                CAM_ANR_MED3;                                    /* 7A54, 0x15007A54 */
	CAM_REG_ANR_MED4                                CAM_ANR_MED4;                                    /* 7A58, 0x15007A58 */
	CAM_REG_ANR_HP1                                 CAM_ANR_HP1;                                     /* 7A5C, 0x15007A5C */
	CAM_REG_ANR_HP2                                 CAM_ANR_HP2;                                     /* 7A60, 0x15007A60 */
	CAM_REG_ANR_HP3                                 CAM_ANR_HP3;                                     /* 7A64, 0x15007A64 */
	CAM_REG_ANR_ACT1                                CAM_ANR_ACT1;                                    /* 7A68, 0x15007A68 */
	CAM_REG_ANR_ACT2                                CAM_ANR_ACT2;                                    /* 7A6C, 0x15007A6C */
	CAM_REG_ANR_ACT3                                CAM_ANR_ACT3;                                    /* 7A70, 0x15007A70 */
	CAM_REG_ANR_ACTYH                               CAM_ANR_ACTYH;                                   /* 7A74, 0x15007A74 */
	CAM_REG_ANR_ACTC                                CAM_ANR_ACTC;                                    /* 7A78, 0x15007A78 */
	CAM_REG_ANR_ACTYL                               CAM_ANR_ACTYL;                                   /* 7A7C, 0x15007A7C */
	CAM_REG_ANR_YLAD                                CAM_ANR_YLAD;                                    /* 7A80, 0x15007A80 */
	CAM_REG_ANR_PTYL                                CAM_ANR_PTYL;                                    /* 7A84, 0x15007A84 */
	CAM_REG_ANR_LCOEF                               CAM_ANR_LCOEF;                                   /* 7A88, 0x15007A88 */
	CAM_REG_ANR_RSV1                                CAM_ANR_RSV1;                                    /* 7A8C, 0x15007A8C */
	UINT32                                          rsv_7A90[4];                                     /* 7A90..7A9F, 0x15007A90..15007A9F */
	CAM_REG_CCR_CON                                 CAM_CCR_CON;                                     /* 7AA0, 0x15007AA0 */
	CAM_REG_CCR_YLUT                                CAM_CCR_YLUT;                                    /* 7AA4, 0x15007AA4 */
	CAM_REG_CCR_UVLUT                               CAM_CCR_UVLUT;                                   /* 7AA8, 0x15007AA8 */
	CAM_REG_CCR_YLUT2                               CAM_CCR_YLUT2;                                   /* 7AAC, 0x15007AAC */
	CAM_REG_CCR_SAT_CTRL                            CAM_CCR_SAT_CTRL;                                /* 7AB0, 0x15007AB0 */
	CAM_REG_CCR_UVLUT_SP                            CAM_CCR_UVLUT_SP;                                /* 7AB4, 0x15007AB4 */
	CAM_REG_CCR_HUE1                                CAM_CCR_HUE1;                                    /* 7AB8, 0x15007AB8 */
	CAM_REG_CCR_HUE2                                CAM_CCR_HUE2;                                    /* 7ABC, 0x15007ABC */
	CAM_REG_CCR_HUE3                                CAM_CCR_HUE3;                                    /* 7AC0, 0x15007AC0 */
	CAM_REG_CCR_RSV1                                CAM_CCR_RSV1;                                    /* 7AC4, 0x15007AC4 */
	UINT32                                          rsv_7AC8[1358];                                  /* 7AC8..8FFC, 0x15007AC8..15008FFC */
	CAMSV_REG_MODULE_EN                             CAMSV_MODULE_EN;                                 /* 9000, 0x15009000 */
	CAMSV_REG_FMT_SEL                               CAMSV_FMT_SEL;                                   /* 9004, 0x15009004 */
	CAMSV_REG_INT_EN                                CAMSV_INT_EN;                                    /* 9008, 0x15009008 */
	CAMSV_REG_INT_STATUS                            CAMSV_INT_STATUS;                                /* 900C, 0x1500900C */
	CAMSV_REG_SW_CTL                                CAMSV_SW_CTL;                                    /* 9010, 0x15009010 */
	CAMSV_REG_SPARE0                                CAMSV_SPARE0;                                    /* 9014, 0x15009014 */
	CAMSV_REG_SPARE1                                CAMSV_SPARE1;                                    /* 9018, 0x15009018 */
	CAMSV_REG_IMGO_FBC                              CAMSV_IMGO_FBC;                                  /* 901C, 0x1500901C */
	CAMSV_REG_CLK_EN                                CAMSV_CLK_EN;                                    /* 9020, 0x15009020 */
	CAMSV_REG_DBG_SET                               CAMSV_DBG_SET;                                   /* 9024, 0x15009024 */
	CAMSV_REG_DBG_PORT                              CAMSV_DBG_PORT;                                  /* 9028, 0x15009028 */
	CAMSV_REG_DATE_CODE                             CAMSV_DATE_CODE;                                 /* 902C, 0x1500902C */
	CAMSV_REG_PROJ_CODE                             CAMSV_PROJ_CODE;                                 /* 9030, 0x15009030 */
	CAMSV_REG_DCM_DIS                               CAMSV_DCM_DIS;                                   /* 9034, 0x15009034 */
	CAMSV_REG_DCM_STATUS                            CAMSV_DCM_STATUS;                                /* 9038, 0x15009038 */
	CAMSV_REG_PAK                                   CAMSV_PAK;                                       /* 903C, 0x1500903C */
	UINT32                                          rsv_9040[112];                                   /* 9040..91FF, 0x15009040..150091FF */
	CAMSV_REG_DMA_SOFT_RSTSTAT_SV                   CAMSV_DMA_SOFT_RSTSTAT_SV;                       /* 9200, 0x15009200 */
	CAMSV_REG_LAST_ULTRA_EN_SV                      CAMSV_LAST_ULTRA_EN_SV;                          /* 9204, 0x15009204 */
	CAMSV_REG_IMGO_SV_BASE_ADDR                     CAMSV_IMGO_SV_BASE_ADDR;                         /* 9208, 0x15009208 */
	CAMSV_REG_IMGO_SV_OFST_ADDR                     CAMSV_IMGO_SV_OFST_ADDR;                         /* 920C, 0x1500920C */
	CAMSV_REG_IMGO_SV_XSIZE                         CAMSV_IMGO_SV_XSIZE;                             /* 9210, 0x15009210 */
	CAMSV_REG_IMGO_SV_YSIZE                         CAMSV_IMGO_SV_YSIZE;                             /* 9214, 0x15009214 */
	CAMSV_REG_IMGO_SV_STRIDE                        CAMSV_IMGO_SV_STRIDE;                            /* 9218, 0x15009218 */
	CAMSV_REG_IMGO_SV_CON                           CAMSV_IMGO_SV_CON;                               /* 921C, 0x1500921C */
	CAMSV_REG_IMGO_SV_CON2                          CAMSV_IMGO_SV_CON2;                              /* 9220, 0x15009220 */
	CAMSV_REG_IMGO_SV_CROP                          CAMSV_IMGO_SV_CROP;                              /* 9224, 0x15009224 */
	CAMSV_REG_IMGO_SV_D_BASE_ADDR                   CAMSV_IMGO_SV_D_BASE_ADDR;                       /* 9228, 0x15009228 */
	CAMSV_REG_IMGO_SV_D_OFST_ADDR                   CAMSV_IMGO_SV_D_OFST_ADDR;                       /* 922C, 0x1500922C */
	CAMSV_REG_IMGO_SV_D_XSIZE                       CAMSV_IMGO_SV_D_XSIZE;                           /* 9230, 0x15009230 */
	CAMSV_REG_IMGO_SV_D_YSIZE                       CAMSV_IMGO_SV_D_YSIZE;                           /* 9234, 0x15009234 */
	CAMSV_REG_IMGO_SV_D_STRIDE                      CAMSV_IMGO_SV_D_STRIDE;                          /* 9238, 0x15009238 */
	CAMSV_REG_IMGO_SV_D_CON                         CAMSV_IMGO_SV_D_CON;                             /* 923C, 0x1500923C */
	CAMSV_REG_IMGO_SV_D_CON2                        CAMSV_IMGO_SV_D_CON2;                            /* 9240, 0x15009240 */
	CAMSV_REG_IMGO_SV_D_CROP                        CAMSV_IMGO_SV_D_CROP;                            /* 9244, 0x15009244 */
	CAMSV_REG_DMA_ERR_CTRL_SV                       CAMSV_DMA_ERR_CTRL_SV;                           /* 9248, 0x15009248 */
	CAMSV_REG_IMGO_SV_ERR_STAT                      CAMSV_IMGO_SV_ERR_STAT;                          /* 924C, 0x1500924C */
	CAMSV_REG_IMGO_SV_D_ERR_STAT                    CAMSV_IMGO_SV_D_ERR_STAT;                        /* 9250, 0x15009250 */
	CAMSV_REG_DMA_DEBUG_ADDR_SV                     CAMSV_DMA_DEBUG_ADDR_SV;                         /* 9254, 0x15009254 */
	CAMSV_REG_DMA_RSV1_SV                           CAMSV_DMA_RSV1_SV;                               /* 9258, 0x15009258 */
	CAMSV_REG_DMA_RSV2_SV                           CAMSV_DMA_RSV2_SV;                               /* 925C, 0x1500925C */
	CAMSV_REG_DMA_RSV3_SV                           CAMSV_DMA_RSV3_SV;                               /* 9260, 0x15009260 */
	CAMSV_REG_DMA_RSV4_SV                           CAMSV_DMA_RSV4_SV;                               /* 9264, 0x15009264 */
	CAMSV_REG_DMA_RSV5_SV                           CAMSV_DMA_RSV5_SV;                               /* 9268, 0x15009268 */
	CAMSV_REG_DMA_RSV6_SV                           CAMSV_DMA_RSV6_SV;                               /* 926C, 0x1500926C */
	UINT32                                          rsv_9270[104];                                   /* 9270..940F, 0x15009270..1500940F */
	CAMSV_REG_TG_SEN_MODE                           CAMSV_TG_SEN_MODE;                               /* 9410, 0x15009410 */
	CAMSV_REG_TG_VF_CON                             CAMSV_TG_VF_CON;                                 /* 9414, 0x15009414 */
	CAMSV_REG_TG_SEN_GRAB_PXL                       CAMSV_TG_SEN_GRAB_PXL;                           /* 9418, 0x15009418 */
	CAMSV_REG_TG_SEN_GRAB_LIN                       CAMSV_TG_SEN_GRAB_LIN;                           /* 941C, 0x1500941C */
	CAMSV_REG_TG_PATH_CFG                           CAMSV_TG_PATH_CFG;                               /* 9420, 0x15009420 */
	CAMSV_REG_TG_MEMIN_CTL                          CAMSV_TG_MEMIN_CTL;                              /* 9424, 0x15009424 */
	CAMSV_REG_TG_INT1                               CAMSV_TG_INT1;                                   /* 9428, 0x15009428 */
	CAMSV_REG_TG_INT2                               CAMSV_TG_INT2;                                   /* 942C, 0x1500942C */
	CAMSV_REG_TG_SOF_CNT                            CAMSV_TG_SOF_CNT;                                /* 9430, 0x15009430 */
	CAMSV_REG_TG_SOT_CNT                            CAMSV_TG_SOT_CNT;                                /* 9434, 0x15009434 */
	CAMSV_REG_TG_EOT_CNT                            CAMSV_TG_EOT_CNT;                                /* 9438, 0x15009438 */
	CAMSV_REG_TG_ERR_CTL                            CAMSV_TG_ERR_CTL;                                /* 943C, 0x1500943C */
	CAMSV_REG_TG_DAT_NO                             CAMSV_TG_DAT_NO;                                 /* 9440, 0x15009440 */
	CAMSV_REG_TG_FRM_CNT_ST                         CAMSV_TG_FRM_CNT_ST;                             /* 9444, 0x15009444 */
	CAMSV_REG_TG_FRMSIZE_ST                         CAMSV_TG_FRMSIZE_ST;                             /* 9448, 0x15009448 */
	CAMSV_REG_TG_INTER_ST                           CAMSV_TG_INTER_ST;                               /* 944C, 0x1500944C */
	UINT32                                          rsv_9450[4];                                     /* 9450..945F, 0x15009450..1500945F */
	CAMSV_REG_TG_FLASHA_CTL                         CAMSV_TG_FLASHA_CTL;                             /* 9460, 0x15009460 */
	CAMSV_REG_TG_FLASHA_LINE_CNT                    CAMSV_TG_FLASHA_LINE_CNT;                        /* 9464, 0x15009464 */
	CAMSV_REG_TG_FLASHA_POS                         CAMSV_TG_FLASHA_POS;                             /* 9468, 0x15009468 */
	CAMSV_REG_TG_FLASHB_CTL                         CAMSV_TG_FLASHB_CTL;                             /* 946C, 0x1500946C */
	CAMSV_REG_TG_FLASHB_LINE_CNT                    CAMSV_TG_FLASHB_LINE_CNT;                        /* 9470, 0x15009470 */
	CAMSV_REG_TG_FLASHB_POS                         CAMSV_TG_FLASHB_POS;                             /* 9474, 0x15009474 */
	CAMSV_REG_TG_FLASHB_POS1                        CAMSV_TG_FLASHB_POS1;                            /* 9478, 0x15009478 */
	CAMSV_REG_TG_GSCTRL_CTL                         CAMSV_TG_GSCTRL_CTL;                             /* 947C, 0x1500947C */
	CAMSV_REG_TG_GSCTRL_TIME                        CAMSV_TG_GSCTRL_TIME;                            /* 9480, 0x15009480 */
	CAMSV_REG_TG_MS_PHASE                           CAMSV_TG_MS_PHASE;                               /* 9484, 0x15009484 */
	CAMSV_REG_TG_MS_CL_TIME                         CAMSV_TG_MS_CL_TIME;                             /* 9488, 0x15009488 */
	CAMSV_REG_TG_MS_OP_TIME                         CAMSV_TG_MS_OP_TIME;                             /* 948C, 0x1500948C */
	CAMSV_REG_TG_MS_CLPH_TIME                       CAMSV_TG_MS_CLPH_TIME;                           /* 9490, 0x15009490 */
	CAMSV_REG_TG_MS_OPPH_TIME                       CAMSV_TG_MS_OPPH_TIME;                           /* 9494, 0x15009494 */
	UINT32                                          rsv_9498[29];                                    /* 9498..950B, 0x15009498..1500950B */
	CAMSV_REG_TOP_DEBUG                             CAMSV_TOP_DEBUG;                                 /* 950C, 0x1500950C */
	UINT32                                          rsv_9510[188];                                   /* 9510..97FF, 0x15009510..150097FF */
	CAMSV2_REG_MODULE_EN                            CAMSV2_MODULE_EN;                                /* 9800, 0x15009800 */
	CAMSV2_REG_FMT_SEL                              CAMSV2_FMT_SEL;                                  /* 9804, 0x15009804 */
	CAMSV2_REG_INT_EN                               CAMSV2_INT_EN;                                   /* 9808, 0x15009808 */
	CAMSV2_REG_INT_STATUS                           CAMSV2_INT_STATUS;                               /* 980C, 0x1500980C */
	CAMSV2_REG_SW_CTL                               CAMSV2_SW_CTL;                                   /* 9810, 0x15009810 */
	CAMSV2_REG_SPARE0                               CAMSV2_SPARE0;                                   /* 9814, 0x15009814 */
	CAMSV2_REG_SPARE1                               CAMSV2_SPARE1;                                   /* 9818, 0x15009818 */
	CAMSV2_REG_IMGO_FBC                             CAMSV2_IMGO_FBC;                                 /* 981C, 0x1500981C */
	CAMSV2_REG_CLK_EN                               CAMSV2_CLK_EN;                                   /* 9820, 0x15009820 */
	CAMSV2_REG_DBG_SET                              CAMSV2_DBG_SET;                                  /* 9824, 0x15009824 */
	CAMSV2_REG_DBG_PORT                             CAMSV2_DBG_PORT;                                 /* 9828, 0x15009828 */
	CAMSV2_REG_DATE_CODE                            CAMSV2_DATE_CODE;                                /* 982C, 0x1500982C */
	CAMSV2_REG_PROJ_CODE                            CAMSV2_PROJ_CODE;                                /* 9830, 0x15009830 */
	CAMSV2_REG_DCM_DIS                              CAMSV2_DCM_DIS;                                  /* 9834, 0x15009834 */
	CAMSV2_REG_DCM_STATUS                           CAMSV2_DCM_STATUS;                               /* 9838, 0x15009838 */
	CAMSV2_REG_PAK                                  CAMSV2_PAK;                                      /* 983C, 0x1500983C */
	UINT32                                          rsv_9840[244];                                   /* 9840..9C0F, 0x15009840..15009C0F */
	CAMSV_REG_TG2_SEN_MODE                          CAMSV_TG2_SEN_MODE;                              /* 9C10, 0x15009C10 */
	CAMSV_REG_TG2_VF_CON                            CAMSV_TG2_VF_CON;                                /* 9C14, 0x15009C14 */
	CAMSV_REG_TG2_SEN_GRAB_PXL                      CAMSV_TG2_SEN_GRAB_PXL;                          /* 9C18, 0x15009C18 */
	CAMSV_REG_TG2_SEN_GRAB_LIN                      CAMSV_TG2_SEN_GRAB_LIN;                          /* 9C1C, 0x15009C1C */
	CAMSV_REG_TG2_PATH_CFG                          CAMSV_TG2_PATH_CFG;                              /* 9C20, 0x15009C20 */
	CAMSV_REG_TG2_MEMIN_CTL                         CAMSV_TG2_MEMIN_CTL;                             /* 9C24, 0x15009C24 */
	CAMSV_REG_TG2_INT1                              CAMSV_TG2_INT1;                                  /* 9C28, 0x15009C28 */
	CAMSV_REG_TG2_INT2                              CAMSV_TG2_INT2;                                  /* 9C2C, 0x15009C2C */
	CAMSV_REG_TG2_SOF_CNT                           CAMSV_TG2_SOF_CNT;                               /* 9C30, 0x15009C30 */
	CAMSV_REG_TG2_SOT_CNT                           CAMSV_TG2_SOT_CNT;                               /* 9C34, 0x15009C34 */
	CAMSV_REG_TG2_EOT_CNT                           CAMSV_TG2_EOT_CNT;                               /* 9C38, 0x15009C38 */
	CAMSV_REG_TG2_ERR_CTL                           CAMSV_TG2_ERR_CTL;                               /* 9C3C, 0x15009C3C */
	CAMSV_REG_TG2_DAT_NO                            CAMSV_TG2_DAT_NO;                                /* 9C40, 0x15009C40 */
	CAMSV_REG_TG2_FRM_CNT_ST                        CAMSV_TG2_FRM_CNT_ST;                            /* 9C44, 0x15009C44 */
	CAMSV_REG_TG2_FRMSIZE_ST                        CAMSV_TG2_FRMSIZE_ST;                            /* 9C48, 0x15009C48 */
	CAMSV_REG_TG2_INTER_ST                          CAMSV_TG2_INTER_ST;                              /* 9C4C, 0x15009C4C */
}isp_reg_t;

#endif // _ISP_REG_H_
