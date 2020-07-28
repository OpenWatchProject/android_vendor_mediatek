#ifndef _ISP_REG_DIP_H_
#define _ISP_REG_DIP_H_
/* auto insert ralf auto gen below */

#include "camera_dip.h"

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif
#ifndef MUINT64
typedef uint64_t MUINT64;
#endif


typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

typedef volatile union _DIP_X_REG_CTL_START_
{
		volatile struct	/* 0x15022000 */
		{
				FIELD  CQ_THR0_START                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_START                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_START                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_START                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_START                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_START                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_START                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_START                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_START                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_START                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_START                        :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_START                        :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_START                        :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_START                        :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_START                        :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_START                        :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_START                        :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_START                        :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_START                        :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_START;	/* DIP_X_CTL_START, DIP_A_CTL_START*/

typedef volatile union _DIP_X_REG_CTL_YUV_EN_
{
		volatile struct	/* 0x15022004 */
		{
				FIELD  MFB_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  C02B_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C24_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2C_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C42_EN                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC2_EN                               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_EN                                :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_EN                              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NR3D_EN                               :  1;		/* 10..10, 0x00000400 */
				FIELD  SL2B_EN                               :  1;		/* 11..11, 0x00000800 */
				FIELD  SL2C_EN                               :  1;		/* 12..12, 0x00001000 */
				FIELD  SL2D_EN                               :  1;		/* 13..13, 0x00002000 */
				FIELD  SL2E_EN                               :  1;		/* 14..14, 0x00004000 */
				FIELD  SRZ1_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  SRZ2_EN                               :  1;		/* 16..16, 0x00010000 */
				FIELD  CRZ_EN                                :  1;		/* 17..17, 0x00020000 */
				FIELD  MIX1_EN                               :  1;		/* 18..18, 0x00040000 */
				FIELD  MIX2_EN                               :  1;		/* 19..19, 0x00080000 */
				FIELD  MIX3_EN                               :  1;		/* 20..20, 0x00100000 */
				FIELD  CRSP_EN                               :  1;		/* 21..21, 0x00200000 */
				FIELD  C24B_EN                               :  1;		/* 22..22, 0x00400000 */
				FIELD  MDPCROP_EN                            :  1;		/* 23..23, 0x00800000 */
				FIELD  C02_EN                                :  1;		/* 24..24, 0x01000000 */
				FIELD  FE_EN                                 :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  PLNW1_EN                              :  1;		/* 28..28, 0x10000000 */
				FIELD  PLNR1_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  PLNW2_EN                              :  1;		/* 30..30, 0x40000000 */
				FIELD  PLNR2_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV_EN;	/* DIP_X_CTL_YUV_EN, DIP_A_CTL_YUV_EN*/

typedef volatile union _DIP_X_REG_CTL_YUV2_EN_
{
		volatile struct	/* 0x15022008 */
		{
				FIELD  FM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ3_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ4_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SRZ5_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HFG_EN                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SL2I_EN                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NDG_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NDG2_EN                               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3_EN                               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIX4_EN                               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV2_EN;	/* DIP_X_CTL_YUV2_EN, DIP_A_CTL_YUV2_EN*/

typedef volatile union _DIP_X_REG_CTL_RGB_EN_
{
		volatile struct	/* 0x1502200C */
		{
				FIELD  UNP_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFD_EN                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PGN_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SL2_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UDM_EN                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCE_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  G2G_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  GGM_EN                                :  1;		/*  7.. 7, 0x00000080 */
				FIELD  GDR1_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  GDR2_EN                               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DBS2_EN                               :  1;		/* 10..10, 0x00000400 */
				FIELD  OBC2_EN                               :  1;		/* 11..11, 0x00000800 */
				FIELD  RMG2_EN                               :  1;		/* 12..12, 0x00001000 */
				FIELD  BNR2_EN                               :  1;		/* 13..13, 0x00002000 */
				FIELD  RMM2_EN                               :  1;		/* 14..14, 0x00004000 */
				FIELD  LSC2_EN                               :  1;		/* 15..15, 0x00008000 */
				FIELD  RCP2_EN                               :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  RNR_EN                                :  1;		/* 18..18, 0x00040000 */
				FIELD  SL2G_EN                               :  1;		/* 19..19, 0x00080000 */
				FIELD  SL2H_EN                               :  1;		/* 20..20, 0x00100000 */
				FIELD  reserve_bit                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_EN                               :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_EN                                :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_EN                               :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX1_EN                               :  1;		/* 25..25, 0x02000000 */
				FIELD  PAKG2_EN                              :  1;		/* 26..26, 0x04000000 */
				FIELD  G2G2_EN                               :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM2_EN                               :  1;		/* 28..28, 0x10000000 */
				FIELD  WSHIFT_EN                             :  1;		/* 29..29, 0x20000000 */
				FIELD  WSYNC_EN                              :  1;		/* 30..30, 0x40000000 */
				FIELD  MDPCROP2_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB_EN;	/* DIP_X_CTL_RGB_EN, DIP_A_CTL_RGB_EN*/

typedef volatile union _DIP_X_REG_CTL_DMA_EN_
{
		volatile struct	/* 0x15022010 */
		{
				FIELD  IMGI_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_EN                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_EN                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_EN                              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_EN                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_EN                               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_EN                               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_EN                              :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_EN                              :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_EN                             :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_EN                              :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_EN                             :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_EN                             :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_EN                                :  1;		/* 16..16, 0x00010000 */
				FIELD  ADL2_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX1I_EN                              :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX2I_EN                              :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX3I_EN                              :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_EN                              :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_EN                              :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_EN                              :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4I_EN                              :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX4O_EN                              :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DMA_EN;	/* DIP_X_CTL_DMA_EN, DIP_A_CTL_DMA_EN*/

typedef volatile union _DIP_X_REG_CTL_FMT_SEL_
{
		volatile struct	/* 0x15022014 */
		{
				FIELD  IMGI_FMT                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  VIPI_FMT                              :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  UFDI_FMT                              :  1;		/* 16..16, 0x00010000 */
				FIELD  DMGI_FMT                              :  1;		/* 17..17, 0x00020000 */
				FIELD  DEPI_FMT                              :  1;		/* 18..18, 0x00040000 */
				FIELD  FG_MODE                               :  1;		/* 19..19, 0x00080000 */
				FIELD  IMG3O_FMT                             :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  IMG2O_FMT                             :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  PIX_ID                                :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  LP_MODE                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_FMT_SEL;	/* DIP_X_CTL_FMT_SEL, DIP_A_CTL_FMT_SEL*/

typedef volatile union _DIP_X_REG_CTL_PATH_SEL_
{
		volatile struct	/* 0x15022018 */
		{
				FIELD  G2G_SEL                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  PGN_SEL                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  G2C_SEL                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SRZ1_SEL                              :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  MIX1_SEL                              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NBC_SEL                               :  1;		/* 10..10, 0x00000400 */
				FIELD  NBCW_SEL                              :  1;		/* 11..11, 0x00000800 */
				FIELD  CRZ_SEL                               :  2;		/* 12..13, 0x00003000 */
				FIELD  NR3D_SEL                              :  1;		/* 14..14, 0x00004000 */
				FIELD  MDP_SEL                               :  1;		/* 15..15, 0x00008000 */
				FIELD  FE_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  RCP2_SEL                              :  2;		/* 17..18, 0x00060000 */
				FIELD  CRSP_SEL                              :  1;		/* 19..19, 0x00080000 */
				FIELD  WPE_SEL                               :  1;		/* 20..20, 0x00100000 */
				FIELD  GGM_SEL                               :  1;		/* 21..21, 0x00200000 */
				FIELD  IMGI_SEL                              :  1;		/* 22..22, 0x00400000 */
				FIELD  FEO_SEL                               :  1;		/* 23..23, 0x00800000 */
				FIELD  G2G2_SEL                              :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_PATH_SEL;	/* DIP_X_CTL_PATH_SEL, DIP_A_CTL_PATH_SEL*/

typedef volatile union _DIP_X_REG_CTL_MISC_SEL_
{
		volatile struct	/* 0x1502201C */
		{
				FIELD  APB_CLK_GATE_BYPASS                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  SRAM_MODE                             :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  PAK2O_SEL                             :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  PAK2_FMT                              :  4;		/* 12..15, 0x0000F000 */
				FIELD  reserve_bit                           :  2;		/* 16..17, 0x00030000 */
				FIELD  PAKG2_FMT                             :  4;		/* 18..21, 0x003C0000 */
				FIELD  PAKG2_FULL_G                          :  1;		/* 22..22, 0x00400000 */
				FIELD  PAKG2_FULL_G_BUS                      :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC_GMAP_LTM_MODE                     :  1;		/* 24..24, 0x01000000 */
				FIELD  WUV_MODE                              :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_MISC_SEL;	/* DIP_X_CTL_MISC_SEL, DIP_A_CTL_MISC_SEL*/

typedef volatile union _DIP_X_REG_CTL_INT_EN_
{
		volatile struct	/* 0x15022020 */
		{
				FIELD  MDP_DONE_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TDR_SIZE_ERR_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGI_DONE_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DONE_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DONE_EN                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DONE_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DONE_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DONE_EN                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DMGI_DONE_EN                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DEPI_DONE_EN                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DONE_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DONE_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG3O_DONE_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3BO_DONE_EN                        :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3CO_DONE_EN                        :  1;		/* 14..14, 0x00004000 */
				FIELD  FEO_DONE_EN                           :  1;		/* 15..15, 0x00008000 */
				FIELD  PASS2_DONE_EN                         :  1;		/* 16..16, 0x00010000 */
				FIELD  TILE_DONE_EN                          :  1;		/* 17..17, 0x00020000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 18..18, 0x00040000 */
				FIELD  GGM_COLLISION_EN                      :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_COLLISION_EN                      :  1;		/* 20..20, 0x00100000 */
				FIELD  NBC_COLLISION_EN                      :  1;		/* 21..21, 0x00200000 */
				FIELD  ADL2_DONE_EN                          :  1;		/* 22..22, 0x00400000 */
				FIELD  ADL2_P2X_ERR_EN                       :  1;		/* 23..23, 0x00800000 */
				FIELD  GGM2_COLLISION_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  MDP2_DONE_EN                          :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  4;		/* 26..29, 0x3C000000 */
				FIELD  APB_INTERFERE_EN                      :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_INT_EN;	/* DIP_X_CTL_INT_EN, DIP_A_CTL_INT_EN*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT_EN_
{
		volatile struct	/* 0x15022024 */
		{
				FIELD  CQ_THR0_DONE_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_DONE_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_DONE_EN                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_DONE_EN                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_DONE_EN                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_DONE_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_DONE_EN                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_DONE_EN                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_DONE_EN                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_DONE_EN                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_DONE_EN                      :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_DONE_EN                      :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_DONE_EN                      :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_DONE_EN                      :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_DONE_EN                      :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_DONE_EN                      :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_DONE_EN                      :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_DONE_EN                      :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_DONE_EN                      :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 10;		/* 19..28, 0x1FF80000 */
				FIELD  CQ_DON_EN                             :  1;		/* 29..29, 0x20000000 */
				FIELD  CQ_CODE_ERR_EN                        :  1;		/* 30..30, 0x40000000 */
				FIELD  CQ_APB_OUT_RANGE_EN                   :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT_EN;	/* DIP_X_CTL_CQ_INT_EN, DIP_A_CTL_CQ_INT_EN*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT2_EN_
{
		volatile struct	/* 0x15022028 */
		{
				FIELD  CQ_THR0_CODE_LD_DONE_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_CODE_LD_DONE_EN               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_CODE_LD_DONE_EN               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_CODE_LD_DONE_EN               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_CODE_LD_DONE_EN               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_CODE_LD_DONE_EN               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_CODE_LD_DONE_EN               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_CODE_LD_DONE_EN               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_CODE_LD_DONE_EN               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_CODE_LD_DONE_EN               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_CODE_LD_DONE_EN              :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_CODE_LD_DONE_EN              :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_CODE_LD_DONE_EN              :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_CODE_LD_DONE_EN              :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_CODE_LD_DONE_EN              :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_CODE_LD_DONE_EN              :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_CODE_LD_DONE_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_CODE_LD_DONE_EN              :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_CODE_LD_DONE_EN              :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT2_EN;	/* DIP_X_CTL_CQ_INT2_EN, DIP_A_CTL_CQ_INT2_EN*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT3_EN_
{
		volatile struct	/* 0x1502202C */
		{
				FIELD  CQ_THR0_ERR_TRIG_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_ERR_TRIG_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_ERR_TRIG_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_ERR_TRIG_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_ERR_TRIG_EN                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_ERR_TRIG_EN                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_ERR_TRIG_EN                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_ERR_TRIG_EN                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_ERR_TRIG_EN                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_ERR_TRIG_EN                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_ERR_TRIG_EN                  :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_ERR_TRIG_EN                  :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_ERR_TRIG_EN                  :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_ERR_TRIG_EN                  :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_ERR_TRIG_EN                  :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_ERR_TRIG_EN                  :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_ERR_TRIG_EN                  :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_ERR_TRIG_EN                  :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_ERR_TRIG_EN                  :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT3_EN;	/* DIP_X_CTL_CQ_INT3_EN, DIP_A_CTL_CQ_INT3_EN*/

typedef volatile union _DIP_X_REG_CTL_INT_STATUS_
{
		volatile struct	/* 0x15022030 */
		{
				FIELD  MDP_DONE_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TDR_SIZE_ERR_STATUS                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGI_DONE_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DONE_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DONE_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DONE_STATUS                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DONE_STATUS                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DONE_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DMGI_DONE_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DEPI_DONE_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DONE_STATUS                     :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DONE_STATUS                     :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG3O_DONE_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3BO_DONE_STATUS                    :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3CO_DONE_STATUS                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FEO_DONE_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  PASS2_DONE_STATUS                     :  1;		/* 16..16, 0x00010000 */
				FIELD  TILE_DONE_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  DMA_ERR_STATUS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  GGM_COLLISION_STATUS                  :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_COLLISION_STATUS                  :  1;		/* 20..20, 0x00100000 */
				FIELD  NBC_COLLISION_STATUS                  :  1;		/* 21..21, 0x00200000 */
				FIELD  ADL2_DONE_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  ADL2_P2X_ERR_STATUS                   :  1;		/* 23..23, 0x00800000 */
				FIELD  GGM2_COLLISION_STATUS                 :  1;		/* 24..24, 0x01000000 */
				FIELD  MDP2_DONE_STATUS                      :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  4;		/* 26..29, 0x3C000000 */
				FIELD  APB_INTERFERE_STATUS                  :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_INT_STATUS;	/* DIP_X_CTL_INT_STATUS, DIP_A_CTL_INT_STATUS*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT_STATUS_
{
		volatile struct	/* 0x15022034 */
		{
				FIELD  CQ_THR0_DONE_STATUS                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_DONE_STATUS                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_DONE_STATUS                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_DONE_STATUS                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_DONE_STATUS                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_DONE_STATUS                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_DONE_STATUS                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_DONE_STATUS                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_DONE_STATUS                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_DONE_STATUS                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_DONE_STATUS                  :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_DONE_STATUS                  :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_DONE_STATUS                  :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_DONE_STATUS                  :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_DONE_STATUS                  :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_DONE_STATUS                  :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_DONE_STATUS                  :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_DONE_STATUS                  :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_DONE_STATUS                  :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 10;		/* 19..28, 0x1FF80000 */
				FIELD  CQ_DON_STATUS                         :  1;		/* 29..29, 0x20000000 */
				FIELD  CQ_CODE_ERR_STATUS                    :  1;		/* 30..30, 0x40000000 */
				FIELD  CQ_APB_OUT_RANGE_STATUS               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT_STATUS;	/* DIP_X_CTL_CQ_INT_STATUS, DIP_A_CTL_CQ_INT_STATUS*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT2_STATUS_
{
		volatile struct	/* 0x15022038 */
		{
				FIELD  CQ_THR0_CODE_LD_DONE_STATUS           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_CODE_LD_DONE_STATUS           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_CODE_LD_DONE_STATUS           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_CODE_LD_DONE_STATUS           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_CODE_LD_DONE_STATUS           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_CODE_LD_DONE_STATUS           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_CODE_LD_DONE_STATUS           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_CODE_LD_DONE_STATUS           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_CODE_LD_DONE_STATUS           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_CODE_LD_DONE_STATUS           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_CODE_LD_DONE_STATUS          :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_CODE_LD_DONE_STATUS          :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_CODE_LD_DONE_STATUS          :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_CODE_LD_DONE_STATUS          :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_CODE_LD_DONE_STATUS          :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_CODE_LD_DONE_STATUS          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_CODE_LD_DONE_STATUS          :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_CODE_LD_DONE_STATUS          :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_CODE_LD_DONE_STATUS          :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT2_STATUS;	/* DIP_X_CTL_CQ_INT2_STATUS, DIP_A_CTL_CQ_INT2_STATUS*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT3_STATUS_
{
		volatile struct	/* 0x1502203C */
		{
				FIELD  CQ_THR0_ERR_TRIG_STATUS               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_ERR_TRIG_STATUS               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_ERR_TRIG_STATUS               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_ERR_TRIG_STATUS               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_ERR_TRIG_STATUS               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_ERR_TRIG_STATUS               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_ERR_TRIG_STATUS               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_ERR_TRIG_STATUS               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_ERR_TRIG_STATUS               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_ERR_TRIG_STATUS               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_ERR_TRIG_STATUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_ERR_TRIG_STATUS              :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_ERR_TRIG_STATUS              :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_ERR_TRIG_STATUS              :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_ERR_TRIG_STATUS              :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_ERR_TRIG_STATUS              :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_ERR_TRIG_STATUS              :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_ERR_TRIG_STATUS              :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_ERR_TRIG_STATUS              :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT3_STATUS;	/* DIP_X_CTL_CQ_INT3_STATUS, DIP_A_CTL_CQ_INT3_STATUS*/

typedef volatile union _DIP_X_REG_CTL_INT_STATUSX_
{
		volatile struct	/* 0x15022040 */
		{
				FIELD  MDP_DONE_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TDR_SIZE_ERR_STATUS                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGI_DONE_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DONE_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DONE_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DONE_STATUS                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DONE_STATUS                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DONE_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DMGI_DONE_STATUS                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DEPI_DONE_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DONE_STATUS                     :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DONE_STATUS                     :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG3O_DONE_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3BO_DONE_STATUS                    :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3CO_DONE_STATUS                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FEO_DONE_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  PASS2_DONE_STATUS                     :  1;		/* 16..16, 0x00010000 */
				FIELD  TILE_DONE_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  DMA_ERR_STATUS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  GGM_COLLISION_STATUS                  :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_COLLISION_STATUS                  :  1;		/* 20..20, 0x00100000 */
				FIELD  NBC_COLLISION_STATUS                  :  1;		/* 21..21, 0x00200000 */
				FIELD  ADL2_DONE_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  ADL2_P2X_ERR_STATUS                   :  1;		/* 23..23, 0x00800000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  APB_INTERFERE_STATUS                  :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_INT_STATUSX;	/* DIP_X_CTL_INT_STATUSX, DIP_A_CTL_INT_STATUSX*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT_STATUSX_
{
		volatile struct	/* 0x15022044 */
		{
				FIELD  CQ_THR0_DONE_STATUSX                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_DONE_STATUSX                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_DONE_STATUSX                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_DONE_STATUSX                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_DONE_STATUSX                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_DONE_STATUSX                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_DONE_STATUSX                  :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_DONE_STATUSX                  :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_DONE_STATUSX                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_DONE_STATUSX                  :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_DONE_STATUSX                 :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_DONE_STATUSX                 :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_DONE_STATUSX                 :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_DONE_STATUSX                 :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_DONE_STATUSX                 :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_DONE_STATUSX                 :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_DONE_STATUSX                 :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_DONE_STATUSX                 :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_DONE_STATUSX                 :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 10;		/* 19..28, 0x1FF80000 */
				FIELD  CQ_DON_STATUSX                        :  1;		/* 29..29, 0x20000000 */
				FIELD  CQ_CODE_ERR_STATUSX                   :  1;		/* 30..30, 0x40000000 */
				FIELD  CQ_APB_OUT_RANGE_STATUSX              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT_STATUSX;	/* DIP_X_CTL_CQ_INT_STATUSX, DIP_A_CTL_CQ_INT_STATUSX*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT2_STATUSX_
{
		volatile struct	/* 0x15022048 */
		{
				FIELD  CQ_THR0_CODE_LD_DONE_STATUSX          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_CODE_LD_DONE_STATUSX          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_CODE_LD_DONE_STATUSX          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_CODE_LD_DONE_STATUSX          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_CODE_LD_DONE_STATUSX          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_CODE_LD_DONE_STATUSX          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_CODE_LD_DONE_STATUSX          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_CODE_LD_DONE_STATUSX          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_CODE_LD_DONE_STATUSX          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_CODE_LD_DONE_STATUSX          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_CODE_LD_DONE_STATUSX         :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_CODE_LD_DONE_STATUSX         :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_CODE_LD_DONE_STATUSX         :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_CODE_LD_DONE_STATUSX         :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_CODE_LD_DONE_STATUSX         :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_CODE_LD_DONE_STATUSX         :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_CODE_LD_DONE_STATUSX         :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_CODE_LD_DONE_STATUSX         :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_CODE_LD_DONE_STATUSX         :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT2_STATUSX;	/* DIP_X_CTL_CQ_INT2_STATUSX, DIP_A_CTL_CQ_INT2_STATUSX*/

typedef volatile union _DIP_X_REG_CTL_CQ_INT3_STATUSX_
{
		volatile struct	/* 0x1502204C */
		{
				FIELD  CQ_THR0_ERR_TRIG_STATUSX              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_THR1_ERR_TRIG_STATUSX              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CQ_THR2_ERR_TRIG_STATUSX              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CQ_THR3_ERR_TRIG_STATUSX              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CQ_THR4_ERR_TRIG_STATUSX              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CQ_THR5_ERR_TRIG_STATUSX              :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_THR6_ERR_TRIG_STATUSX              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_THR7_ERR_TRIG_STATUSX              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_THR8_ERR_TRIG_STATUSX              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CQ_THR9_ERR_TRIG_STATUSX              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CQ_THR10_ERR_TRIG_STATUSX             :  1;		/* 10..10, 0x00000400 */
				FIELD  CQ_THR11_ERR_TRIG_STATUSX             :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_ERR_TRIG_STATUSX             :  1;		/* 12..12, 0x00001000 */
				FIELD  CQ_THR13_ERR_TRIG_STATUSX             :  1;		/* 13..13, 0x00002000 */
				FIELD  CQ_THR14_ERR_TRIG_STATUSX             :  1;		/* 14..14, 0x00004000 */
				FIELD  CQ_THR15_ERR_TRIG_STATUSX             :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR16_ERR_TRIG_STATUSX             :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR17_ERR_TRIG_STATUSX             :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR18_ERR_TRIG_STATUSX             :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_CQ_INT3_STATUSX;	/* DIP_X_CTL_CQ_INT3_STATUSX, DIP_A_CTL_CQ_INT3_STATUSX*/

typedef volatile union _DIP_X_REG_CTL_TDR_CTL_
{
		volatile struct	/* 0x15022050 */
		{
				FIELD  TDR_SOF_RST_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CTL_EXTENSION_EN                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TDR_SZ_DET                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TDR_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_CTL;	/* DIP_X_CTL_TDR_CTL, DIP_A_CTL_TDR_CTL*/

typedef volatile union _DIP_X_REG_CTL_TDR_TILE_
{
		volatile struct	/* 0x15022054 */
		{
				FIELD  TILE_EDGE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  TILE_IRQ                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LAST_TILE                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CRZ_EDGE                              :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TILE;	/* DIP_X_CTL_TDR_TILE, DIP_A_CTL_TDR_TILE*/

typedef volatile union _DIP_X_REG_CTL_TDR_TCM_EN_
{
		volatile struct	/* 0x15022058 */
		{
				FIELD  NDG_TCM_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGI_TCM_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGBI_TCM_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMGCI_TCM_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UFDI_TCM_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCEI_TCM_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIPI_TCM_EN                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  VIP2I_TCM_EN                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  VIP3I_TCM_EN                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_TCM_EN                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DEPI_TCM_EN                           :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_TCM_EN                          :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_TCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_TCM_EN                          :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_TCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_TCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_TCM_EN                            :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2O_TCM_EN                          :  1;		/* 17..17, 0x00020000 */
				FIELD  UFD_TCM_EN                            :  1;		/* 18..18, 0x00040000 */
				FIELD  UNP_TCM_EN                            :  1;		/* 19..19, 0x00080000 */
				FIELD  UDM_TCM_EN                            :  1;		/* 20..20, 0x00100000 */
				FIELD  LSC2_TCM_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2_TCM_EN                            :  1;		/* 22..22, 0x00400000 */
				FIELD  SL2B_TCM_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  SL2C_TCM_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  SL2D_TCM_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2E_TCM_EN                           :  1;		/* 26..26, 0x04000000 */
				FIELD  G2C_TCM_EN                            :  1;		/* 27..27, 0x08000000 */
				FIELD  NDG2_TCM_EN                           :  1;		/* 28..28, 0x10000000 */
				FIELD  SRZ1_TCM_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  SRZ2_TCM_EN                           :  1;		/* 30..30, 0x40000000 */
				FIELD  LCE_TCM_EN                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TCM_EN;	/* DIP_X_CTL_TDR_TCM_EN, DIP_A_CTL_TDR_TCM_EN*/

typedef volatile union _DIP_X_REG_CTL_TDR_TCM2_EN_
{
		volatile struct	/* 0x1502205C */
		{
				FIELD  CRZ_TCM_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLC2_TCM_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  NR3D_TCM_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MDPCROP_TCM_EN                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CRSP_TCM_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  C02_TCM_EN                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  C02B_TCM_EN                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  C24_TCM_EN                            :  1;		/*  7.. 7, 0x00000080 */
				FIELD  C42_TCM_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  C24B_TCM_EN                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MFB_TCM_EN                            :  1;		/* 10..10, 0x00000400 */
				FIELD  PCA_TCM_EN                            :  1;		/* 11..11, 0x00000800 */
				FIELD  SEEE_TCM_EN                           :  1;		/* 12..12, 0x00001000 */
				FIELD  NBC_TCM_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  NBC2_TCM_EN                           :  1;		/* 14..14, 0x00004000 */
				FIELD  DBS2_TCM_EN                           :  1;		/* 15..15, 0x00008000 */
				FIELD  RMG2_TCM_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  BNR2_TCM_EN                           :  1;		/* 17..17, 0x00020000 */
				FIELD  RMM2_TCM_EN                           :  1;		/* 18..18, 0x00040000 */
				FIELD  SRZ3_TCM_EN                           :  1;		/* 19..19, 0x00080000 */
				FIELD  SRZ4_TCM_EN                           :  1;		/* 20..20, 0x00100000 */
				FIELD  RCP2_TCM_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SRZ5_TCM_EN                           :  1;		/* 22..22, 0x00400000 */
				FIELD  RNR_TCM_EN                            :  1;		/* 23..23, 0x00800000 */
				FIELD  SL2G_TCM_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  SL2H_TCM_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2I_TCM_EN                           :  1;		/* 26..26, 0x04000000 */
				FIELD  HFG_TCM_EN                            :  1;		/* 27..27, 0x08000000 */
				FIELD  reserve_bit                           :  1;		/* 28..28, 0x10000000 */
				FIELD  SL2K_TCM_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL2_TCM_EN                           :  1;		/* 30..30, 0x40000000 */
				FIELD  FLC_TCM_EN                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TCM2_EN;	/* DIP_X_CTL_TDR_TCM2_EN, DIP_A_CTL_TDR_TCM2_EN*/

typedef volatile union _DIP_X_REG_CTL_TDR_EN_STATUS1_
{
		volatile struct	/* 0x15022060 */
		{
				FIELD  CTL_TDR_EN_STATUS1                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_EN_STATUS1;	/* DIP_X_CTL_TDR_EN_STATUS1, DIP_A_CTL_TDR_EN_STATUS1*/

typedef volatile union _DIP_X_REG_CTL_TDR_EN_STATUS2_
{
		volatile struct	/* 0x15022064 */
		{
				FIELD  CTL_TDR_EN_STATUS2                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_EN_STATUS2;	/* DIP_X_CTL_TDR_EN_STATUS2, DIP_A_CTL_TDR_EN_STATUS2*/

typedef volatile union _DIP_X_REG_CTL_TDR_DBG_STATUS_
{
		volatile struct	/* 0x15022068 */
		{
				FIELD  TPIPE_CNT                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LOAD_SIZE                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_DBG_STATUS;	/* DIP_X_CTL_TDR_DBG_STATUS, DIP_A_CTL_TDR_DBG_STATUS*/

typedef volatile union _DIP_X_REG_CTL_SW_CTL_
{
		volatile struct	/* 0x1502206C */
		{
				FIELD  SW_RST_Trig                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SW_RST_ST                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  HW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_SW_CTL;	/* DIP_X_CTL_SW_CTL, DIP_A_CTL_SW_CTL*/

typedef volatile union _DIP_X_REG_CTL_SPARE0_
{
		volatile struct	/* 0x15022070 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_SPARE0;	/* DIP_X_CTL_SPARE0, DIP_A_CTL_SPARE0*/

typedef volatile union _DIP_X_REG_CTL_SPARE1_
{
		volatile struct	/* 0x15022074 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_SPARE1;	/* DIP_X_CTL_SPARE1, DIP_A_CTL_SPARE1*/

typedef volatile union _DIP_X_REG_CTL_SPARE2_
{
		volatile struct	/* 0x15022078 */
		{
				FIELD  SPARE2                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_SPARE2;	/* DIP_X_CTL_SPARE2, DIP_A_CTL_SPARE2*/

typedef volatile union _DIP_X_REG_CTL_DONE_SEL_
{
		volatile struct	/* 0x1502207C */
		{
				FIELD  IMGI_DONE_SEL                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_DONE_SEL                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_DONE_SEL                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DONE_SEL                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DONE_SEL                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DONE_SEL                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DONE_SEL                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DONE_SEL                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_DONE_SEL                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_DONE_SEL                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DONE_SEL                        :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DONE_SEL                        :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DONE_SEL                       :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DONE_SEL                        :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DONE_SEL                       :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DONE_SEL                       :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DONE_SEL                          :  1;		/* 16..16, 0x00010000 */
				FIELD  ADL2_DONE_SEL                         :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX1I_DONE_SEL                        :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX2I_DONE_SEL                        :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX3I_DONE_SEL                        :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_DONE_SEL                        :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_DONE_SEL                        :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_DONE_SEL                        :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4I_DONE_SEL                        :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX4O_DONE_SEL                        :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  5;		/* 26..30, 0x7C000000 */
				FIELD  DONE_SEL_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DONE_SEL;	/* DIP_X_CTL_DONE_SEL, DIP_A_CTL_DONE_SEL*/

typedef volatile union _DIP_X_REG_CTL_DBG_SET_
{
		volatile struct	/* 0x15022080 */
		{
				FIELD  DEBUG_MOD_SEL                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DEBUG_SEL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  DEBUG_TOP_SEL                         :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DBG_SET;	/* DIP_X_CTL_DBG_SET, DIP_A_CTL_DBG_SET*/

typedef volatile union _DIP_X_REG_CTL_DBG_PORT_
{
		volatile struct	/* 0x15022084 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DBG_PORT;	/* DIP_X_CTL_DBG_PORT, DIP_A_CTL_DBG_PORT*/

typedef volatile union _DIP_X_REG_CTL_DATE_CODE_
{
		volatile struct	/* 0x15022088 */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DATE_CODE;	/* DIP_X_CTL_DATE_CODE, DIP_A_CTL_DATE_CODE*/

typedef volatile union _DIP_X_REG_CTL_PROJ_CODE_
{
		volatile struct	/* 0x1502208C */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_PROJ_CODE;	/* DIP_X_CTL_PROJ_CODE, DIP_A_CTL_PROJ_CODE*/

typedef volatile union _DIP_X_REG_CTL_YUV_DCM_DIS_
{
		volatile struct	/* 0x15022090 */
		{
				FIELD  MFB_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  C02B_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C24_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2C_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C42_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC2_DCM_DIS                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_DCM_DIS                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_DCM_DIS                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NR3D_DCM_DIS                          :  1;		/* 10..10, 0x00000400 */
				FIELD  SL2B_DCM_DIS                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SL2C_DCM_DIS                          :  1;		/* 12..12, 0x00001000 */
				FIELD  SL2D_DCM_DIS                          :  1;		/* 13..13, 0x00002000 */
				FIELD  SL2E_DCM_DIS                          :  1;		/* 14..14, 0x00004000 */
				FIELD  SRZ1_DCM_DIS                          :  1;		/* 15..15, 0x00008000 */
				FIELD  SRZ2_DCM_DIS                          :  1;		/* 16..16, 0x00010000 */
				FIELD  CRZ_DCM_DIS                           :  1;		/* 17..17, 0x00020000 */
				FIELD  MIX1_DCM_DIS                          :  1;		/* 18..18, 0x00040000 */
				FIELD  MIX2_DCM_DIS                          :  1;		/* 19..19, 0x00080000 */
				FIELD  MIX3_DCM_DIS                          :  1;		/* 20..20, 0x00100000 */
				FIELD  CRSP_DCM_DIS                          :  1;		/* 21..21, 0x00200000 */
				FIELD  C24B_DCM_DIS                          :  1;		/* 22..22, 0x00400000 */
				FIELD  MDPCROP_DCM_DIS                       :  1;		/* 23..23, 0x00800000 */
				FIELD  C02_DCM_DIS                           :  1;		/* 24..24, 0x01000000 */
				FIELD  FE_DCM_DIS                            :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  PLNW1_DCM_DIS                         :  1;		/* 28..28, 0x10000000 */
				FIELD  PLNR1_DCM_DIS                         :  1;		/* 29..29, 0x20000000 */
				FIELD  PLNW2_DCM_DIS                         :  1;		/* 30..30, 0x40000000 */
				FIELD  PLNR2_DCM_DIS                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV_DCM_DIS;	/* DIP_X_CTL_YUV_DCM_DIS, DIP_A_CTL_YUV_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_YUV2_DCM_DIS_
{
		volatile struct	/* 0x15022094 */
		{
				FIELD  FM_DCM_DIS                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ3_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ4_DCM_DIS                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SRZ5_DCM_DIS                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HFG_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SL2I_DCM_DIS                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NDG_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NDG2_DCM_DIS                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3_DCM_DIS                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIX4_DCM_DIS                          :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV2_DCM_DIS;	/* DIP_X_CTL_YUV2_DCM_DIS, DIP_A_CTL_YUV2_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_RGB_DCM_DIS_
{
		volatile struct	/* 0x15022098 */
		{
				FIELD  UNP_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFD_DCM_DIS                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PGN_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SL2_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UDM_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCE_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  G2G_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  GGM_DCM_DIS                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  GDR1_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  GDR2_DCM_DIS                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DBS2_DCM_DIS                          :  1;		/* 10..10, 0x00000400 */
				FIELD  OBC2_DCM_DIS                          :  1;		/* 11..11, 0x00000800 */
				FIELD  RMG2_DCM_DIS                          :  1;		/* 12..12, 0x00001000 */
				FIELD  BNR2_DCM_DIS                          :  1;		/* 13..13, 0x00002000 */
				FIELD  RMM2_DCM_DIS                          :  1;		/* 14..14, 0x00004000 */
				FIELD  LSC2_DCM_DIS                          :  1;		/* 15..15, 0x00008000 */
				FIELD  RCP2_DCM_DIS                          :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2_DCM_DIS                          :  1;		/* 17..17, 0x00020000 */
				FIELD  RNR_DCM_DIS                           :  1;		/* 18..18, 0x00040000 */
				FIELD  SL2G_DCM_DIS                          :  1;		/* 19..19, 0x00080000 */
				FIELD  SL2H_DCM_DIS                          :  1;		/* 20..20, 0x00100000 */
				FIELD  HLR2_DCM_DIS                          :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_DCM_DIS                          :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_DCM_DIS                           :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_DCM_DIS                          :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX1_DCM_DIS                          :  1;		/* 25..25, 0x02000000 */
				FIELD  PAKG2_DCM_DIS                         :  1;		/* 26..26, 0x04000000 */
				FIELD  G2G2_DCM_DIS                          :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM2_DCM_DIS                          :  1;		/* 28..28, 0x10000000 */
				FIELD  WSHIFT_DCM_DIS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  WSYNC_DCM_DIS                         :  1;		/* 30..30, 0x40000000 */
				FIELD  MDPCROP2_DCM_DIS                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB_DCM_DIS;	/* DIP_X_CTL_RGB_DCM_DIS, DIP_A_CTL_RGB_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_DMA_DCM_DIS_
{
		volatile struct	/* 0x1502209C */
		{
				FIELD  IMGI_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_DCM_DIS                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_DCM_DIS                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DCM_DIS                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DCM_DIS                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DCM_DIS                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DCM_DIS                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DCM_DIS                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_DCM_DIS                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DCM_DIS                         :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DCM_DIS                         :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DCM_DIS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DCM_DIS                         :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DCM_DIS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DCM_DIS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DCM_DIS                           :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX1I_DCM_DIS                         :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX2I_DCM_DIS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX3I_DCM_DIS                         :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_DCM_DIS                         :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_DCM_DIS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_DCM_DIS                         :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4I_DCM_DIS                         :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX4O_DCM_DIS                         :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DMA_DCM_DIS;	/* DIP_X_CTL_DMA_DCM_DIS, DIP_A_CTL_DMA_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_TOP_DCM_DIS_
{
		volatile struct	/* 0x150220A0 */
		{
				FIELD  TOP_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TOP_DCM_DIS;	/* DIP_X_CTL_TOP_DCM_DIS, DIP_A_CTL_TOP_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_YUV_DCM_STATUS_
{
		volatile struct	/* 0x150220A4 */
		{
				FIELD  MFB_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  C02B_DCM_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C24_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2C_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C42_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC2_DCM_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_DCM_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_DCM_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NR3D_DCM_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SL2B_DCM_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  SL2C_DCM_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  SL2D_DCM_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  SL2E_DCM_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  SRZ1_DCM_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  SRZ2_DCM_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CRZ_DCM_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  MIX1_DCM_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  MIX2_DCM_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  MIX3_DCM_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CRSP_DCM_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  C24B_DCM_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  MDPCROP_DCM_STATUS                    :  1;		/* 23..23, 0x00800000 */
				FIELD  C02_DCM_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  FE_DCM_STATUS                         :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  PLNW1_DCM_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PLNR1_DCM_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  PLNW2_DCM_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  PLNR2_DCM_STATUS                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV_DCM_STATUS;	/* DIP_X_CTL_YUV_DCM_STATUS, DIP_A_CTL_YUV_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_YUV2_DCM_STATUS_
{
		volatile struct	/* 0x150220A8 */
		{
				FIELD  FM_DCM_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ3_DCM_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ4_DCM_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SRZ5_DCM_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HFG_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SL2I_DCM_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NDG_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NDG2_DCM_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3_DCM_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIX4_DCM_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV2_DCM_STATUS;	/* DIP_X_CTL_YUV2_DCM_STATUS, DIP_A_CTL_YUV2_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_RGB_DCM_STATUS_
{
		volatile struct	/* 0x150220AC */
		{
				FIELD  UNP_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFD_DCM_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PGN_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SL2_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UDM_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCE_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  G2G_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  GGM_DCM_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  GDR1_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  GDR2_DCM_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DBS2_DCM_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  OBC2_DCM_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  RMG2_DCM_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BNR2_DCM_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  RMM2_DCM_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  LSC2_DCM_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RCP2_DCM_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2_DCM_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  RNR_DCM_STATUS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  SL2G_DCM_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  SL2H_DCM_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  HLR2_DCM_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_DCM_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_DCM_STATUS                        :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_DCM_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX1_DCM_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  PAKG2_DCM_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  G2G2_DCM_STATUS                       :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM2_DCM_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  WSHIFT_DCM_STATUS                     :  1;		/* 29..29, 0x20000000 */
				FIELD  WSYNC_DCM_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  MDPCROP2_DCM_STATUS                   :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB_DCM_STATUS;	/* DIP_X_CTL_RGB_DCM_STATUS, DIP_A_CTL_RGB_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_DMA_DCM_STATUS_
{
		volatile struct	/* 0x150220B0 */
		{
				FIELD  IMGI_DCM_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_DCM_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_DCM_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_DCM_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_DCM_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_DCM_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_DCM_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_DCM_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_DCM_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_DCM_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DCM_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DCM_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DCM_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DCM_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DCM_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DCM_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX1I_DCM_STATUS                      :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX2I_DCM_STATUS                      :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX3I_DCM_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_DCM_STATUS                      :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_DCM_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_DCM_STATUS                      :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4I_DCM_STATUS                      :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX4O_DCM_STATUS                      :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DMA_DCM_STATUS;	/* DIP_X_CTL_DMA_DCM_STATUS, DIP_A_CTL_DMA_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_TOP_DCM_STATUS_
{
		volatile struct	/* 0x150220B4 */
		{
				FIELD  TOP_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADL2_DCM_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TOP_DCM_STATUS;	/* DIP_X_CTL_TOP_DCM_STATUS, DIP_A_CTL_TOP_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_YUV_REQ_STATUS_
{
		volatile struct	/* 0x150220B8 */
		{
				FIELD  MFB_REQ_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  C02B_REQ_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C24_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2C_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C42_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC2_REQ_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_REQ_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_REQ_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NR3D_REQ_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SL2B_REQ_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  SL2C_REQ_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  SL2D_REQ_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  SL2E_REQ_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  SRZ1_REQ_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  SRZ2_REQ_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CRZ_REQ_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  MIX1_REQ_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  MIX2_REQ_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  MIX3_REQ_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CRSP_REQ_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  C24B_REQ_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  MDPCROP_REQ_STATUS                    :  1;		/* 23..23, 0x00800000 */
				FIELD  C02_REQ_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  FE_REQ_STATUS                         :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  PLNW1_REQ_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PLNR1_REQ_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  PLNW2_REQ_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  PLNR2_REQ_STATUS                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV_REQ_STATUS;	/* DIP_X_CTL_YUV_REQ_STATUS, DIP_A_CTL_YUV_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_YUV2_REQ_STATUS_
{
		volatile struct	/* 0x150220BC */
		{
				FIELD  FM_REQ_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ3_REQ_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ4_REQ_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SRZ5_REQ_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HFG_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SL2I_REQ_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NDG_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NDG2_REQ_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3_REQ_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIX4_REQ_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV2_REQ_STATUS;	/* DIP_X_CTL_YUV2_REQ_STATUS, DIP_A_CTL_YUV2_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_RGB_REQ_STATUS_
{
		volatile struct	/* 0x150220C0 */
		{
				FIELD  UNP_REQ_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFD_REQ_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PGN_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SL2_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UDM_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCE_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  G2G_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  GGM_REQ_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  GDR1_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  GDR2_REQ_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DBS2_REQ_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  OBC2_REQ_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  RMG2_REQ_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BNR2_REQ_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  RMM2_REQ_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  LSC2_REQ_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RCP2_REQ_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2_REQ_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  RNR_REQ_STATUS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  SL2G_REQ_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  SL2H_REQ_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  HLR2_REQ_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_REQ_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_REQ_STATUS                        :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_REQ_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX1_REQ_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  PAKG2_REQ_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  G2G2_REQ_STATUS                       :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM2_REQ_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  WSHIFT_REQ_STATUS                     :  1;		/* 29..29, 0x20000000 */
				FIELD  WSYNC_REQ_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  MDPCROP2_REQ_STATUS                   :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB_REQ_STATUS;	/* DIP_X_CTL_RGB_REQ_STATUS, DIP_A_CTL_RGB_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_DMA_REQ_STATUS_
{
		volatile struct	/* 0x150220C4 */
		{
				FIELD  IMGI_REQ_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_REQ_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_REQ_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_REQ_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_REQ_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_REQ_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_REQ_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_REQ_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_REQ_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_REQ_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_REQ_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_REQ_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_REQ_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_REQ_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_REQ_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_REQ_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  SMX1I_REQ_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX2I_REQ_STATUS                      :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX3I_REQ_STATUS                      :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX4I_REQ_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_REQ_STATUS                      :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_REQ_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_REQ_STATUS                      :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4O_REQ_STATUS                      :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DMA_REQ_STATUS;	/* DIP_X_CTL_DMA_REQ_STATUS, DIP_A_CTL_DMA_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_YUV_RDY_STATUS_
{
		volatile struct	/* 0x150220C8 */
		{
				FIELD  MFB_RDY_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  C02B_RDY_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  C24_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  G2C_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  C42_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC2_RDY_STATUS                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_RDY_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_RDY_STATUS                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  NR3D_RDY_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SL2B_RDY_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  SL2C_RDY_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  SL2D_RDY_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  SL2E_RDY_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  SRZ1_RDY_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  SRZ2_RDY_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CRZ_RDY_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  MIX1_RDY_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  MIX2_RDY_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  MIX3_RDY_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CRSP_RDY_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  C24B_RDY_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  MDPCROP_RDY_STATUS                    :  1;		/* 23..23, 0x00800000 */
				FIELD  C02_RDY_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  FE_RDY_STATUS                         :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  PLNW1_RDY_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PLNR1_RDY_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  PLNW2_RDY_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  PLNR2_RDY_STATUS                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV_RDY_STATUS;	/* DIP_X_CTL_YUV_RDY_STATUS, DIP_A_CTL_YUV_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CTL_YUV2_RDY_STATUS_
{
		volatile struct	/* 0x150220CC */
		{
				FIELD  FM_RDY_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ3_RDY_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ4_RDY_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SRZ5_RDY_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HFG_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SL2I_RDY_STATUS                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NDG_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  NDG2_RDY_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3_RDY_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIX4_RDY_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_YUV2_RDY_STATUS;	/* DIP_X_CTL_YUV2_RDY_STATUS, DIP_A_CTL_YUV2_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CTL_RGB_RDY_STATUS_
{
		volatile struct	/* 0x150220D0 */
		{
				FIELD  UNP_RDY_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFD_RDY_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PGN_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SL2_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  UDM_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  LCE_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  G2G_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  GGM_RDY_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  GDR1_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  GDR2_RDY_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DBS2_RDY_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  OBC2_RDY_STATUS                       :  1;		/* 11..11, 0x00000800 */
				FIELD  RMG2_RDY_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BNR2_RDY_STATUS                       :  1;		/* 13..13, 0x00002000 */
				FIELD  RMM2_RDY_STATUS                       :  1;		/* 14..14, 0x00004000 */
				FIELD  LSC2_RDY_STATUS                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RCP2_RDY_STATUS                       :  1;		/* 16..16, 0x00010000 */
				FIELD  PAK2_RDY_STATUS                       :  1;		/* 17..17, 0x00020000 */
				FIELD  RNR_RDY_STATUS                        :  1;		/* 18..18, 0x00040000 */
				FIELD  SL2G_RDY_STATUS                       :  1;		/* 19..19, 0x00080000 */
				FIELD  SL2H_RDY_STATUS                       :  1;		/* 20..20, 0x00100000 */
				FIELD  HLR2_RDY_STATUS                       :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_RDY_STATUS                       :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_RDY_STATUS                        :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_RDY_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  SMX1_RDY_STATUS                       :  1;		/* 25..25, 0x02000000 */
				FIELD  PAKG2_RDY_STATUS                      :  1;		/* 26..26, 0x04000000 */
				FIELD  G2G2_RDY_STATUS                       :  1;		/* 27..27, 0x08000000 */
				FIELD  GGM2_RDY_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  WSHIFT_RDY_STATUS                     :  1;		/* 29..29, 0x20000000 */
				FIELD  WSYNC_RDY_STATUS                      :  1;		/* 30..30, 0x40000000 */
				FIELD  MDPCROP2_RDY_STATUS                   :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB_RDY_STATUS;	/* DIP_X_CTL_RGB_RDY_STATUS, DIP_A_CTL_RGB_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CTL_DMA_RDY_STATUS_
{
		volatile struct	/* 0x150220D4 */
		{
				FIELD  IMGI_RDY_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGBI_RDY_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMGCI_RDY_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  UFDI_RDY_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  VIPI_RDY_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  VIP2I_RDY_STATUS                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  VIP3I_RDY_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LCEI_RDY_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DEPI_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DMGI_RDY_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PAK2O_RDY_STATUS                      :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_RDY_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_RDY_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_RDY_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_RDY_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_RDY_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_RDY_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  SMX1I_RDY_STATUS                      :  1;		/* 17..17, 0x00020000 */
				FIELD  SMX2I_RDY_STATUS                      :  1;		/* 18..18, 0x00040000 */
				FIELD  SMX3I_RDY_STATUS                      :  1;		/* 19..19, 0x00080000 */
				FIELD  SMX4I_RDY_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  SMX1O_RDY_STATUS                      :  1;		/* 21..21, 0x00200000 */
				FIELD  SMX2O_RDY_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  SMX3O_RDY_STATUS                      :  1;		/* 23..23, 0x00800000 */
				FIELD  SMX4O_RDY_STATUS                      :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_DMA_RDY_STATUS;	/* DIP_X_CTL_DMA_RDY_STATUS, DIP_A_CTL_DMA_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CTL_TOP_REQ_STATUS_
{
		volatile struct	/* 0x150220D8 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADL2_REQ_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TOP_REQ_STATUS;	/* DIP_X_CTL_TOP_REQ_STATUS, DIP_A_CTL_TOP_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_TOP_RDY_STATUS_
{
		volatile struct	/* 0x150220DC */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADL2_RDY_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TOP_RDY_STATUS;	/* DIP_X_CTL_TOP_RDY_STATUS, DIP_A_CTL_TOP_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CTL_TDR_TCM3_EN_
{
		volatile struct	/* 0x150220E0 */
		{
				FIELD  SMX1_TCM_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SMX2_TCM_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SMX3_TCM_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MDPCROP2_TCM_EN                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_TCM_EN                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SMX1I_TCM_EN                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SMX2I_TCM_EN                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX3I_TCM_EN                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX1O_TCM_EN                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX2O_TCM_EN                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX3O_TCM_EN                          :  1;		/* 10..10, 0x00000400 */
				FIELD  SMX4_TCM_EN                           :  1;		/* 11..11, 0x00000800 */
				FIELD  SMX4I_TCM_EN                          :  1;		/* 12..12, 0x00001000 */
				FIELD  SMX4O_TCM_EN                          :  1;		/* 13..13, 0x00002000 */
				FIELD  ADBS2_TCM_EN                          :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_TCM_EN                             :  1;		/* 15..15, 0x00008000 */
				FIELD  DCPN2_TCM_EN                          :  1;		/* 16..16, 0x00010000 */
				FIELD  CPN2_TCM_EN                           :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TCM3_EN;	/* DIP_X_CTL_TDR_TCM3_EN, DIP_A_CTL_TDR_TCM3_EN*/

typedef volatile union _DIP_X_REG_CTL_TDR_EN_STATUS3_
{
		volatile struct	/* 0x150220E4 */
		{
				FIELD  CTL_TDR_EN_STATUS3                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_EN_STATUS3;	/* DIP_X_CTL_TDR_EN_STATUS3, DIP_A_CTL_TDR_EN_STATUS3*/

typedef volatile union _DIP_X_REG_CTL_RGB2_EN_
{
		volatile struct	/* 0x150220E8 */
		{
				FIELD  SMX4_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADBS2_EN                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DCPN2_EN                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CPN2_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB2_EN;	/* DIP_X_CTL_RGB2_EN, DIP_A_CTL_RGB2_EN*/

typedef volatile union _DIP_X_REG_CTL_RGB2_DCM_DIS_
{
		volatile struct	/* 0x150220EC */
		{
				FIELD  SMX4_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADBS2_DCM_DIS                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DCPN2_DCM_DIS                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CPN2_DCM_DIS                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB2_DCM_DIS;	/* DIP_X_CTL_RGB2_DCM_DIS, DIP_A_CTL_RGB2_DCM_DIS*/

typedef volatile union _DIP_X_REG_CTL_RGB2_DCM_STATUS_
{
		volatile struct	/* 0x150220F0 */
		{
				FIELD  SMX4_DCM_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADBS2_DCM_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DCPN2_DCM_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CPN2_DCM_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB2_DCM_STATUS;	/* DIP_X_CTL_RGB2_DCM_STATUS, DIP_A_CTL_RGB2_DCM_STATUS*/

typedef volatile union _DIP_X_REG_CTL_RGB2_REQ_STATUS_
{
		volatile struct	/* 0x150220F4 */
		{
				FIELD  SMX4_REQ_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADBS2_REQ_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DCPN2_REQ_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CPN2_REQ_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB2_REQ_STATUS;	/* DIP_X_CTL_RGB2_REQ_STATUS, DIP_A_CTL_RGB2_REQ_STATUS*/

typedef volatile union _DIP_X_REG_CTL_RGB2_RDY_STATUS_
{
		volatile struct	/* 0x150220F8 */
		{
				FIELD  SMX4_RDY_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ADBS2_RDY_STATUS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DCPN2_RDY_STATUS                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CPN2_RDY_STATUS                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_RGB2_RDY_STATUS;	/* DIP_X_CTL_RGB2_RDY_STATUS, DIP_A_CTL_RGB2_RDY_STATUS*/

typedef volatile union _DIP_X_REG_CQ_EN_
{
		volatile struct	/* 0x15022200 */
		{
				FIELD  CQ_APB_2T                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_DROP_FRAME_EN                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 14;		/*  2..15, 0x0000FFFC */
				FIELD  CQ_RESET                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_EN;	/* DIP_X_CQ_EN, DIP_A_CQ_EN*/

typedef volatile union _DIP_X_REG_CQ_THR0_CTL_
{
		volatile struct	/* 0x15022204 */
		{
				FIELD  CQ_THR0_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR0_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR0_CTL;	/* DIP_X_CQ_THR0_CTL, DIP_A_CQ_THR0_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR0_BASEADDR_
{
		volatile struct	/* 0x15022208 */
		{
				FIELD  CQ_THR0_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR0_BASEADDR;	/* DIP_X_CQ_THR0_BASEADDR, DIP_A_CQ_THR0_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR0_DESC_SIZE_
{
		volatile struct	/* 0x1502220C */
		{
				FIELD  CQ_THR0_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR0_DESC_SIZE;	/* DIP_X_CQ_THR0_DESC_SIZE, DIP_A_CQ_THR0_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR1_CTL_
{
		volatile struct	/* 0x15022210 */
		{
				FIELD  CQ_THR1_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR1_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR1_CTL;	/* DIP_X_CQ_THR1_CTL, DIP_A_CQ_THR1_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR1_BASEADDR_
{
		volatile struct	/* 0x15022214 */
		{
				FIELD  CQ_THR1_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR1_BASEADDR;	/* DIP_X_CQ_THR1_BASEADDR, DIP_A_CQ_THR1_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR1_DESC_SIZE_
{
		volatile struct	/* 0x15022218 */
		{
				FIELD  CQ_THR1_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR1_DESC_SIZE;	/* DIP_X_CQ_THR1_DESC_SIZE, DIP_A_CQ_THR1_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR2_CTL_
{
		volatile struct	/* 0x1502221C */
		{
				FIELD  CQ_THR2_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR2_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR2_CTL;	/* DIP_X_CQ_THR2_CTL, DIP_A_CQ_THR2_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR2_BASEADDR_
{
		volatile struct	/* 0x15022220 */
		{
				FIELD  CQ_THR2_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR2_BASEADDR;	/* DIP_X_CQ_THR2_BASEADDR, DIP_A_CQ_THR2_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR2_DESC_SIZE_
{
		volatile struct	/* 0x15022224 */
		{
				FIELD  CQ_THR2_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR2_DESC_SIZE;	/* DIP_X_CQ_THR2_DESC_SIZE, DIP_A_CQ_THR2_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR3_CTL_
{
		volatile struct	/* 0x15022228 */
		{
				FIELD  CQ_THR3_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR3_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR3_CTL;	/* DIP_X_CQ_THR3_CTL, DIP_A_CQ_THR3_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR3_BASEADDR_
{
		volatile struct	/* 0x1502222C */
		{
				FIELD  CQ_THR3_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR3_BASEADDR;	/* DIP_X_CQ_THR3_BASEADDR, DIP_A_CQ_THR3_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR3_DESC_SIZE_
{
		volatile struct	/* 0x15022230 */
		{
				FIELD  CQ_THR3_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR3_DESC_SIZE;	/* DIP_X_CQ_THR3_DESC_SIZE, DIP_A_CQ_THR3_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR4_CTL_
{
		volatile struct	/* 0x15022234 */
		{
				FIELD  CQ_THR4_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR4_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR4_CTL;	/* DIP_X_CQ_THR4_CTL, DIP_A_CQ_THR4_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR4_BASEADDR_
{
		volatile struct	/* 0x15022238 */
		{
				FIELD  CQ_THR4_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR4_BASEADDR;	/* DIP_X_CQ_THR4_BASEADDR, DIP_A_CQ_THR4_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR4_DESC_SIZE_
{
		volatile struct	/* 0x1502223C */
		{
				FIELD  CQ_THR4_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR4_DESC_SIZE;	/* DIP_X_CQ_THR4_DESC_SIZE, DIP_A_CQ_THR4_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR5_CTL_
{
		volatile struct	/* 0x15022240 */
		{
				FIELD  CQ_THR5_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR5_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR5_CTL;	/* DIP_X_CQ_THR5_CTL, DIP_A_CQ_THR5_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR5_BASEADDR_
{
		volatile struct	/* 0x15022244 */
		{
				FIELD  CQ_THR5_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR5_BASEADDR;	/* DIP_X_CQ_THR5_BASEADDR, DIP_A_CQ_THR5_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR5_DESC_SIZE_
{
		volatile struct	/* 0x15022248 */
		{
				FIELD  CQ_THR5_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR5_DESC_SIZE;	/* DIP_X_CQ_THR5_DESC_SIZE, DIP_A_CQ_THR5_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR6_CTL_
{
		volatile struct	/* 0x1502224C */
		{
				FIELD  CQ_THR6_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR6_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR6_CTL;	/* DIP_X_CQ_THR6_CTL, DIP_A_CQ_THR6_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR6_BASEADDR_
{
		volatile struct	/* 0x15022250 */
		{
				FIELD  CQ_THR6_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR6_BASEADDR;	/* DIP_X_CQ_THR6_BASEADDR, DIP_A_CQ_THR6_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR6_DESC_SIZE_
{
		volatile struct	/* 0x15022254 */
		{
				FIELD  CQ_THR6_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR6_DESC_SIZE;	/* DIP_X_CQ_THR6_DESC_SIZE, DIP_A_CQ_THR6_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR7_CTL_
{
		volatile struct	/* 0x15022258 */
		{
				FIELD  CQ_THR7_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR7_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR7_CTL;	/* DIP_X_CQ_THR7_CTL, DIP_A_CQ_THR7_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR7_BASEADDR_
{
		volatile struct	/* 0x1502225C */
		{
				FIELD  CQ_THR7_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR7_BASEADDR;	/* DIP_X_CQ_THR7_BASEADDR, DIP_A_CQ_THR7_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR7_DESC_SIZE_
{
		volatile struct	/* 0x15022260 */
		{
				FIELD  CQ_THR7_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR7_DESC_SIZE;	/* DIP_X_CQ_THR7_DESC_SIZE, DIP_A_CQ_THR7_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR8_CTL_
{
		volatile struct	/* 0x15022264 */
		{
				FIELD  CQ_THR8_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR8_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR8_CTL;	/* DIP_X_CQ_THR8_CTL, DIP_A_CQ_THR8_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR8_BASEADDR_
{
		volatile struct	/* 0x15022268 */
		{
				FIELD  CQ_THR8_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR8_BASEADDR;	/* DIP_X_CQ_THR8_BASEADDR, DIP_A_CQ_THR8_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR8_DESC_SIZE_
{
		volatile struct	/* 0x1502226C */
		{
				FIELD  CQ_THR8_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR8_DESC_SIZE;	/* DIP_X_CQ_THR8_DESC_SIZE, DIP_A_CQ_THR8_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR9_CTL_
{
		volatile struct	/* 0x15022270 */
		{
				FIELD  CQ_THR9_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR9_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR9_CTL;	/* DIP_X_CQ_THR9_CTL, DIP_A_CQ_THR9_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR9_BASEADDR_
{
		volatile struct	/* 0x15022274 */
		{
				FIELD  CQ_THR9_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR9_BASEADDR;	/* DIP_X_CQ_THR9_BASEADDR, DIP_A_CQ_THR9_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR9_DESC_SIZE_
{
		volatile struct	/* 0x15022278 */
		{
				FIELD  CQ_THR9_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR9_DESC_SIZE;	/* DIP_X_CQ_THR9_DESC_SIZE, DIP_A_CQ_THR9_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR10_CTL_
{
		volatile struct	/* 0x1502227C */
		{
				FIELD  CQ_THR10_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR10_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR10_CTL;	/* DIP_X_CQ_THR10_CTL, DIP_A_CQ_THR10_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR10_BASEADDR_
{
		volatile struct	/* 0x15022280 */
		{
				FIELD  CQ_THR10_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR10_BASEADDR;	/* DIP_X_CQ_THR10_BASEADDR, DIP_A_CQ_THR10_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR10_DESC_SIZE_
{
		volatile struct	/* 0x15022284 */
		{
				FIELD  CQ_THR10_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR10_DESC_SIZE;	/* DIP_X_CQ_THR10_DESC_SIZE, DIP_A_CQ_THR10_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR11_CTL_
{
		volatile struct	/* 0x15022288 */
		{
				FIELD  CQ_THR11_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR11_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR11_CTL;	/* DIP_X_CQ_THR11_CTL, DIP_A_CQ_THR11_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR11_BASEADDR_
{
		volatile struct	/* 0x1502228C */
		{
				FIELD  CQ_THR11_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR11_BASEADDR;	/* DIP_X_CQ_THR11_BASEADDR, DIP_A_CQ_THR11_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR11_DESC_SIZE_
{
		volatile struct	/* 0x15022290 */
		{
				FIELD  CQ_THR11_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR11_DESC_SIZE;	/* DIP_X_CQ_THR11_DESC_SIZE, DIP_A_CQ_THR11_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR12_CTL_
{
		volatile struct	/* 0x15022294 */
		{
				FIELD  CQ_THR12_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR12_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR12_CTL;	/* DIP_X_CQ_THR12_CTL, DIP_A_CQ_THR12_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR12_BASEADDR_
{
		volatile struct	/* 0x15022298 */
		{
				FIELD  CQ_THR12_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR12_BASEADDR;	/* DIP_X_CQ_THR12_BASEADDR, DIP_A_CQ_THR12_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR12_DESC_SIZE_
{
		volatile struct	/* 0x1502229C */
		{
				FIELD  CQ_THR12_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR12_DESC_SIZE;	/* DIP_X_CQ_THR12_DESC_SIZE, DIP_A_CQ_THR12_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR13_CTL_
{
		volatile struct	/* 0x150222A0 */
		{
				FIELD  CQ_THR13_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR13_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR13_CTL;	/* DIP_X_CQ_THR13_CTL, DIP_A_CQ_THR13_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR13_BASEADDR_
{
		volatile struct	/* 0x150222A4 */
		{
				FIELD  CQ_THR13_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR13_BASEADDR;	/* DIP_X_CQ_THR13_BASEADDR, DIP_A_CQ_THR13_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR13_DESC_SIZE_
{
		volatile struct	/* 0x150222A8 */
		{
				FIELD  CQ_THR13_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR13_DESC_SIZE;	/* DIP_X_CQ_THR13_DESC_SIZE, DIP_A_CQ_THR13_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR14_CTL_
{
		volatile struct	/* 0x150222AC */
		{
				FIELD  CQ_THR14_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR14_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR14_CTL;	/* DIP_X_CQ_THR14_CTL, DIP_A_CQ_THR14_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR14_BASEADDR_
{
		volatile struct	/* 0x150222B0 */
		{
				FIELD  CQ_THR14_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR14_BASEADDR;	/* DIP_X_CQ_THR14_BASEADDR, DIP_A_CQ_THR14_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR14_DESC_SIZE_
{
		volatile struct	/* 0x150222B4 */
		{
				FIELD  CQ_THR14_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR14_DESC_SIZE;	/* DIP_X_CQ_THR14_DESC_SIZE, DIP_A_CQ_THR14_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR15_CTL_
{
		volatile struct	/* 0x150222B8 */
		{
				FIELD  CQ_THR15_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR15_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR15_CTL;	/* DIP_X_CQ_THR15_CTL, DIP_A_CQ_THR15_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR15_BASEADDR_
{
		volatile struct	/* 0x150222BC */
		{
				FIELD  CQ_THR15_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR15_BASEADDR;	/* DIP_X_CQ_THR15_BASEADDR, DIP_A_CQ_THR15_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR15_DESC_SIZE_
{
		volatile struct	/* 0x150222C0 */
		{
				FIELD  CQ_THR15_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR15_DESC_SIZE;	/* DIP_X_CQ_THR15_DESC_SIZE, DIP_A_CQ_THR15_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR16_CTL_
{
		volatile struct	/* 0x150222C4 */
		{
				FIELD  CQ_THR16_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR16_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR16_CTL;	/* DIP_X_CQ_THR16_CTL, DIP_A_CQ_THR16_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR16_BASEADDR_
{
		volatile struct	/* 0x150222C8 */
		{
				FIELD  CQ_THR16_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR16_BASEADDR;	/* DIP_X_CQ_THR16_BASEADDR, DIP_A_CQ_THR16_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR16_DESC_SIZE_
{
		volatile struct	/* 0x150222CC */
		{
				FIELD  CQ_THR16_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR16_DESC_SIZE;	/* DIP_X_CQ_THR16_DESC_SIZE, DIP_A_CQ_THR16_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR17_CTL_
{
		volatile struct	/* 0x150222D0 */
		{
				FIELD  CQ_THR17_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR17_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR17_CTL;	/* DIP_X_CQ_THR17_CTL, DIP_A_CQ_THR17_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR17_BASEADDR_
{
		volatile struct	/* 0x150222D4 */
		{
				FIELD  CQ_THR17_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR17_BASEADDR;	/* DIP_X_CQ_THR17_BASEADDR, DIP_A_CQ_THR17_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR17_DESC_SIZE_
{
		volatile struct	/* 0x150222D8 */
		{
				FIELD  CQ_THR17_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR17_DESC_SIZE;	/* DIP_X_CQ_THR17_DESC_SIZE, DIP_A_CQ_THR17_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR18_CTL_
{
		volatile struct	/* 0x150222DC */
		{
				FIELD  CQ_THR18_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR18_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR18_CTL;	/* DIP_X_CQ_THR18_CTL, DIP_A_CQ_THR18_CTL*/

typedef volatile union _DIP_X_REG_CQ_THR18_BASEADDR_
{
		volatile struct	/* 0x150222E0 */
		{
				FIELD  CQ_THR18_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR18_BASEADDR;	/* DIP_X_CQ_THR18_BASEADDR, DIP_A_CQ_THR18_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR18_DESC_SIZE_
{
		volatile struct	/* 0x150222E4 */
		{
				FIELD  CQ_THR18_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR18_DESC_SIZE;	/* DIP_X_CQ_THR18_DESC_SIZE, DIP_A_CQ_THR18_DESC_SIZE*/

typedef volatile union _DIP_X_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x15022300 */
		{
				FIELD  IMG2O_SOFT_RST_STAT                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_SOFT_RST_STAT                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_SOFT_RST_STAT                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_SOFT_RST_STAT                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_SOFT_RST_STAT                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_SOFT_RST_STAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAK2O_SOFT_RST_STAT                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX1O_SOFT_RST_STAT                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2O_SOFT_RST_STAT                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3O_SOFT_RST_STAT                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX4O_SOFT_RST_STAT                   :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGBI_SOFT_RST_STAT                   :  1;		/* 17..17, 0x00020000 */
				FIELD  IMGCI_SOFT_RST_STAT                   :  1;		/* 18..18, 0x00040000 */
				FIELD  VIPI_SOFT_RST_STAT                    :  1;		/* 19..19, 0x00080000 */
				FIELD  VIP2I_SOFT_RST_STAT                   :  1;		/* 20..20, 0x00100000 */
				FIELD  VIP3I_SOFT_RST_STAT                   :  1;		/* 21..21, 0x00200000 */
				FIELD  DMGI_SOFT_RST_STAT                    :  1;		/* 22..22, 0x00400000 */
				FIELD  DEPI_SOFT_RST_STAT                    :  1;		/* 23..23, 0x00800000 */
				FIELD  LCEI_SOFT_RST_STAT                    :  1;		/* 24..24, 0x01000000 */
				FIELD  UFDI_SOFT_RST_STAT                    :  1;		/* 25..25, 0x02000000 */
				FIELD  SMX1I_SOFT_RST_STAT                   :  1;		/* 26..26, 0x04000000 */
				FIELD  SMX2I_SOFT_RST_STAT                   :  1;		/* 27..27, 0x08000000 */
				FIELD  SMX3I_SOFT_RST_STAT                   :  1;		/* 28..28, 0x10000000 */
				FIELD  SMX4I_SOFT_RST_STAT                   :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_SOFT_RSTSTAT;	/* DIP_X_DMA_SOFT_RSTSTAT, DIP_A_DMA_SOFT_RSTSTAT*/

typedef volatile union _DIP_X_REG_TDRI_BASE_ADDR_
{
		volatile struct	/* 0x15022304 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_BASE_ADDR;	/* DIP_X_TDRI_BASE_ADDR, DIP_A_TDRI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_TDRI_OFST_ADDR_
{
		volatile struct	/* 0x15022308 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_OFST_ADDR;	/* DIP_X_TDRI_OFST_ADDR, DIP_A_TDRI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_TDRI_XSIZE_
{
		volatile struct	/* 0x1502230C */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_XSIZE;	/* DIP_X_TDRI_XSIZE, DIP_A_TDRI_XSIZE*/

typedef volatile union _DIP_X_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x15022310 */
		{
				FIELD  IMG2O_V_FLIP_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_V_FLIP_EN                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_V_FLIP_EN                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_V_FLIP_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_V_FLIP_EN                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_V_FLIP_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAK2O_V_FLIP_EN                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX1O_V_FLIP_EN                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2O_V_FLIP_EN                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3O_V_FLIP_EN                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX4O_V_FLIP_EN                       :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGBI_V_FLIP_EN                       :  1;		/* 17..17, 0x00020000 */
				FIELD  IMGCI_V_FLIP_EN                       :  1;		/* 18..18, 0x00040000 */
				FIELD  VIPI_V_FLIP_EN                        :  1;		/* 19..19, 0x00080000 */
				FIELD  VIP2I_V_FLIP_EN                       :  1;		/* 20..20, 0x00100000 */
				FIELD  VIP3I_V_FLIP_EN                       :  1;		/* 21..21, 0x00200000 */
				FIELD  DMGI_V_FLIP_EN                        :  1;		/* 22..22, 0x00400000 */
				FIELD  DEPI_V_FLIP_EN                        :  1;		/* 23..23, 0x00800000 */
				FIELD  LCEI_V_FLIP_EN                        :  1;		/* 24..24, 0x01000000 */
				FIELD  UFDI_V_FLIP_EN                        :  1;		/* 25..25, 0x02000000 */
				FIELD  SMX1I_V_FLIP_EN                       :  1;		/* 26..26, 0x04000000 */
				FIELD  SMX2I_V_FLIP_EN                       :  1;		/* 27..27, 0x08000000 */
				FIELD  SMX3I_V_FLIP_EN                       :  1;		/* 28..28, 0x10000000 */
				FIELD  SMX4I_V_FLIP_EN                       :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VERTICAL_FLIP_EN;	/* DIP_X_VERTICAL_FLIP_EN, DIP_A_VERTICAL_FLIP_EN*/

typedef volatile union _DIP_X_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x15022314 */
		{
				FIELD  IMG2O_SOFT_RST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_SOFT_RST                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_SOFT_RST                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_SOFT_RST                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_SOFT_RST                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_SOFT_RST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAK2O_SOFT_RST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX1O_SOFT_RST                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2O_SOFT_RST                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3O_SOFT_RST                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX4O_SOFT_RST                        :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGBI_SOFT_RST                        :  1;		/* 17..17, 0x00020000 */
				FIELD  IMGCI_SOFT_RST                        :  1;		/* 18..18, 0x00040000 */
				FIELD  VIPI_SOFT_RST                         :  1;		/* 19..19, 0x00080000 */
				FIELD  VIP2I_SOFT_RST                        :  1;		/* 20..20, 0x00100000 */
				FIELD  VIP3I_SOFT_RST                        :  1;		/* 21..21, 0x00200000 */
				FIELD  DMGI_SOFT_RST                         :  1;		/* 22..22, 0x00400000 */
				FIELD  DEPI_SOFT_RST                         :  1;		/* 23..23, 0x00800000 */
				FIELD  LCEI_SOFT_RST                         :  1;		/* 24..24, 0x01000000 */
				FIELD  UFDI_SOFT_RST                         :  1;		/* 25..25, 0x02000000 */
				FIELD  SMX1I_SOFT_RST                        :  1;		/* 26..26, 0x04000000 */
				FIELD  SMX2I_SOFT_RST                        :  1;		/* 27..27, 0x08000000 */
				FIELD  SMX3I_SOFT_RST                        :  1;		/* 28..28, 0x10000000 */
				FIELD  SMX4I_SOFT_RST                        :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_SOFT_RESET;	/* DIP_X_DMA_SOFT_RESET, DIP_A_DMA_SOFT_RESET*/

typedef volatile union _DIP_X_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x15022318 */
		{
				FIELD  IMG2O_LAST_ULTRA_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_LAST_ULTRA_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_LAST_ULTRA_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_LAST_ULTRA_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_LAST_ULTRA_EN                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_LAST_ULTRA_EN                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAK2O_LAST_ULTRA_EN                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX1O_LAST_ULTRA_EN                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2O_LAST_ULTRA_EN                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3O_LAST_ULTRA_EN                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX4O_LAST_ULTRA_EN                   :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LAST_ULTRA_EN;	/* DIP_X_LAST_ULTRA_EN, DIP_A_LAST_ULTRA_EN*/

typedef volatile union _DIP_X_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1502231C */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  GCLAST_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_ULTRA_LCEI_EN                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_ULTRA_IMGI_EN                      :  1;		/* 27..27, 0x08000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 28..28, 0x10000000 */
				FIELD  UFO_IMGI_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SPECIAL_FUN_EN;	/* DIP_X_SPECIAL_FUN_EN, DIP_A_SPECIAL_FUN_EN*/

typedef volatile union _DIP_X_REG_IMG2O_BASE_ADDR_
{
		volatile struct	/* 0x15022330 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_BASE_ADDR;	/* DIP_X_IMG2O_BASE_ADDR, DIP_A_IMG2O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG2O_OFST_ADDR_
{
		volatile struct	/* 0x15022338 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_OFST_ADDR;	/* DIP_X_IMG2O_OFST_ADDR, DIP_A_IMG2O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG2O_XSIZE_
{
		volatile struct	/* 0x15022340 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_XSIZE;	/* DIP_X_IMG2O_XSIZE, DIP_A_IMG2O_XSIZE*/

typedef volatile union _DIP_X_REG_IMG2O_YSIZE_
{
		volatile struct	/* 0x15022344 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_YSIZE;	/* DIP_X_IMG2O_YSIZE, DIP_A_IMG2O_YSIZE*/

typedef volatile union _DIP_X_REG_IMG2O_STRIDE_
{
		volatile struct	/* 0x15022348 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_STRIDE;	/* DIP_X_IMG2O_STRIDE, DIP_A_IMG2O_STRIDE*/

typedef volatile union _DIP_X_REG_IMG2O_CON_
{
		volatile struct	/* 0x1502234C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CON;	/* DIP_X_IMG2O_CON, DIP_A_IMG2O_CON*/

typedef volatile union _DIP_X_REG_IMG2O_CON2_
{
		volatile struct	/* 0x15022350 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CON2;	/* DIP_X_IMG2O_CON2, DIP_A_IMG2O_CON2*/

typedef volatile union _DIP_X_REG_IMG2O_CON3_
{
		volatile struct	/* 0x15022354 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CON3;	/* DIP_X_IMG2O_CON3, DIP_A_IMG2O_CON3*/

typedef volatile union _DIP_X_REG_IMG2O_CROP_
{
		volatile struct	/* 0x15022358 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CROP;	/* DIP_X_IMG2O_CROP, DIP_A_IMG2O_CROP*/

typedef volatile union _DIP_X_REG_IMG2BO_BASE_ADDR_
{
		volatile struct	/* 0x15022360 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_BASE_ADDR;	/* DIP_X_IMG2BO_BASE_ADDR, DIP_A_IMG2BO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG2BO_OFST_ADDR_
{
		volatile struct	/* 0x15022368 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_OFST_ADDR;	/* DIP_X_IMG2BO_OFST_ADDR, DIP_A_IMG2BO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG2BO_XSIZE_
{
		volatile struct	/* 0x15022370 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_XSIZE;	/* DIP_X_IMG2BO_XSIZE, DIP_A_IMG2BO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG2BO_YSIZE_
{
		volatile struct	/* 0x15022374 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_YSIZE;	/* DIP_X_IMG2BO_YSIZE, DIP_A_IMG2BO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG2BO_STRIDE_
{
		volatile struct	/* 0x15022378 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_STRIDE;	/* DIP_X_IMG2BO_STRIDE, DIP_A_IMG2BO_STRIDE*/

typedef volatile union _DIP_X_REG_IMG2BO_CON_
{
		volatile struct	/* 0x1502237C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CON;	/* DIP_X_IMG2BO_CON, DIP_A_IMG2BO_CON*/

typedef volatile union _DIP_X_REG_IMG2BO_CON2_
{
		volatile struct	/* 0x15022380 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CON2;	/* DIP_X_IMG2BO_CON2, DIP_A_IMG2BO_CON2*/

typedef volatile union _DIP_X_REG_IMG2BO_CON3_
{
		volatile struct	/* 0x15022384 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CON3;	/* DIP_X_IMG2BO_CON3, DIP_A_IMG2BO_CON3*/

typedef volatile union _DIP_X_REG_IMG2BO_CROP_
{
		volatile struct	/* 0x15022388 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CROP;	/* DIP_X_IMG2BO_CROP, DIP_A_IMG2BO_CROP*/

typedef volatile union _DIP_X_REG_IMG3O_BASE_ADDR_
{
		volatile struct	/* 0x15022390 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_BASE_ADDR;	/* DIP_X_IMG3O_BASE_ADDR, DIP_A_IMG3O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3O_OFST_ADDR_
{
		volatile struct	/* 0x15022398 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_OFST_ADDR;	/* DIP_X_IMG3O_OFST_ADDR, DIP_A_IMG3O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3O_XSIZE_
{
		volatile struct	/* 0x150223A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_XSIZE;	/* DIP_X_IMG3O_XSIZE, DIP_A_IMG3O_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3O_YSIZE_
{
		volatile struct	/* 0x150223A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_YSIZE;	/* DIP_X_IMG3O_YSIZE, DIP_A_IMG3O_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3O_STRIDE_
{
		volatile struct	/* 0x150223A8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_STRIDE;	/* DIP_X_IMG3O_STRIDE, DIP_A_IMG3O_STRIDE*/

typedef volatile union _DIP_X_REG_IMG3O_CON_
{
		volatile struct	/* 0x150223AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CON;	/* DIP_X_IMG3O_CON, DIP_A_IMG3O_CON*/

typedef volatile union _DIP_X_REG_IMG3O_CON2_
{
		volatile struct	/* 0x150223B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CON2;	/* DIP_X_IMG3O_CON2, DIP_A_IMG3O_CON2*/

typedef volatile union _DIP_X_REG_IMG3O_CON3_
{
		volatile struct	/* 0x150223B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CON3;	/* DIP_X_IMG3O_CON3, DIP_A_IMG3O_CON3*/

typedef volatile union _DIP_X_REG_IMG3O_CROP_
{
		volatile struct	/* 0x150223B8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CROP;	/* DIP_X_IMG3O_CROP, DIP_A_IMG3O_CROP*/

typedef volatile union _DIP_X_REG_IMG3BO_BASE_ADDR_
{
		volatile struct	/* 0x150223C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_BASE_ADDR;	/* DIP_X_IMG3BO_BASE_ADDR, DIP_A_IMG3BO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3BO_OFST_ADDR_
{
		volatile struct	/* 0x150223C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_OFST_ADDR;	/* DIP_X_IMG3BO_OFST_ADDR, DIP_A_IMG3BO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3BO_XSIZE_
{
		volatile struct	/* 0x150223D0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_XSIZE;	/* DIP_X_IMG3BO_XSIZE, DIP_A_IMG3BO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3BO_YSIZE_
{
		volatile struct	/* 0x150223D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_YSIZE;	/* DIP_X_IMG3BO_YSIZE, DIP_A_IMG3BO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3BO_STRIDE_
{
		volatile struct	/* 0x150223D8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_STRIDE;	/* DIP_X_IMG3BO_STRIDE, DIP_A_IMG3BO_STRIDE*/

typedef volatile union _DIP_X_REG_IMG3BO_CON_
{
		volatile struct	/* 0x150223DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CON;	/* DIP_X_IMG3BO_CON, DIP_A_IMG3BO_CON*/

typedef volatile union _DIP_X_REG_IMG3BO_CON2_
{
		volatile struct	/* 0x150223E0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CON2;	/* DIP_X_IMG3BO_CON2, DIP_A_IMG3BO_CON2*/

typedef volatile union _DIP_X_REG_IMG3BO_CON3_
{
		volatile struct	/* 0x150223E4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CON3;	/* DIP_X_IMG3BO_CON3, DIP_A_IMG3BO_CON3*/

typedef volatile union _DIP_X_REG_IMG3BO_CROP_
{
		volatile struct	/* 0x150223E8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CROP;	/* DIP_X_IMG3BO_CROP, DIP_A_IMG3BO_CROP*/

typedef volatile union _DIP_X_REG_IMG3CO_BASE_ADDR_
{
		volatile struct	/* 0x150223F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_BASE_ADDR;	/* DIP_X_IMG3CO_BASE_ADDR, DIP_A_IMG3CO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3CO_OFST_ADDR_
{
		volatile struct	/* 0x150223F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_OFST_ADDR;	/* DIP_X_IMG3CO_OFST_ADDR, DIP_A_IMG3CO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3CO_XSIZE_
{
		volatile struct	/* 0x15022400 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_XSIZE;	/* DIP_X_IMG3CO_XSIZE, DIP_A_IMG3CO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3CO_YSIZE_
{
		volatile struct	/* 0x15022404 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_YSIZE;	/* DIP_X_IMG3CO_YSIZE, DIP_A_IMG3CO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3CO_STRIDE_
{
		volatile struct	/* 0x15022408 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_STRIDE;	/* DIP_X_IMG3CO_STRIDE, DIP_A_IMG3CO_STRIDE*/

typedef volatile union _DIP_X_REG_IMG3CO_CON_
{
		volatile struct	/* 0x1502240C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CON;	/* DIP_X_IMG3CO_CON, DIP_A_IMG3CO_CON*/

typedef volatile union _DIP_X_REG_IMG3CO_CON2_
{
		volatile struct	/* 0x15022410 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CON2;	/* DIP_X_IMG3CO_CON2, DIP_A_IMG3CO_CON2*/

typedef volatile union _DIP_X_REG_IMG3CO_CON3_
{
		volatile struct	/* 0x15022414 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CON3;	/* DIP_X_IMG3CO_CON3, DIP_A_IMG3CO_CON3*/

typedef volatile union _DIP_X_REG_IMG3CO_CROP_
{
		volatile struct	/* 0x15022418 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CROP;	/* DIP_X_IMG3CO_CROP, DIP_A_IMG3CO_CROP*/

typedef volatile union _DIP_X_REG_FEO_BASE_ADDR_
{
		volatile struct	/* 0x15022420 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_BASE_ADDR;	/* DIP_X_FEO_BASE_ADDR, DIP_A_FEO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_FEO_OFST_ADDR_
{
		volatile struct	/* 0x15022428 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_OFST_ADDR;	/* DIP_X_FEO_OFST_ADDR, DIP_A_FEO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_FEO_XSIZE_
{
		volatile struct	/* 0x15022430 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_XSIZE;	/* DIP_X_FEO_XSIZE, DIP_A_FEO_XSIZE*/

typedef volatile union _DIP_X_REG_FEO_YSIZE_
{
		volatile struct	/* 0x15022434 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_YSIZE;	/* DIP_X_FEO_YSIZE, DIP_A_FEO_YSIZE*/

typedef volatile union _DIP_X_REG_FEO_STRIDE_
{
		volatile struct	/* 0x15022438 */
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
}DIP_X_REG_FEO_STRIDE;	/* DIP_X_FEO_STRIDE, DIP_A_FEO_STRIDE*/

typedef volatile union _DIP_X_REG_FEO_CON_
{
		volatile struct	/* 0x1502243C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_CON;	/* DIP_X_FEO_CON, DIP_A_FEO_CON*/

typedef volatile union _DIP_X_REG_FEO_CON2_
{
		volatile struct	/* 0x15022440 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_CON2;	/* DIP_X_FEO_CON2, DIP_A_FEO_CON2*/

typedef volatile union _DIP_X_REG_FEO_CON3_
{
		volatile struct	/* 0x15022444 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_CON3;	/* DIP_X_FEO_CON3, DIP_A_FEO_CON3*/

typedef volatile union _DIP_X_REG_PAK2O_BASE_ADDR_
{
		volatile struct	/* 0x15022450 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_BASE_ADDR;	/* DIP_X_PAK2O_BASE_ADDR, DIP_A_PAK2O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_PAK2O_OFST_ADDR_
{
		volatile struct	/* 0x15022458 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_OFST_ADDR;	/* DIP_X_PAK2O_OFST_ADDR, DIP_A_PAK2O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_PAK2O_XSIZE_
{
		volatile struct	/* 0x15022460 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_XSIZE;	/* DIP_X_PAK2O_XSIZE, DIP_A_PAK2O_XSIZE*/

typedef volatile union _DIP_X_REG_PAK2O_YSIZE_
{
		volatile struct	/* 0x15022464 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_YSIZE;	/* DIP_X_PAK2O_YSIZE, DIP_A_PAK2O_YSIZE*/

typedef volatile union _DIP_X_REG_PAK2O_STRIDE_
{
		volatile struct	/* 0x15022468 */
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
}DIP_X_REG_PAK2O_STRIDE;	/* DIP_X_PAK2O_STRIDE, DIP_A_PAK2O_STRIDE*/

typedef volatile union _DIP_X_REG_PAK2O_CON_
{
		volatile struct	/* 0x1502246C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_CON;	/* DIP_X_PAK2O_CON, DIP_A_PAK2O_CON*/

typedef volatile union _DIP_X_REG_PAK2O_CON2_
{
		volatile struct	/* 0x15022470 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_CON2;	/* DIP_X_PAK2O_CON2, DIP_A_PAK2O_CON2*/

typedef volatile union _DIP_X_REG_PAK2O_CON3_
{
		volatile struct	/* 0x15022474 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_CON3;	/* DIP_X_PAK2O_CON3, DIP_A_PAK2O_CON3*/

typedef volatile union _DIP_X_REG_PAK2O_CROP_
{
		volatile struct	/* 0x15022478 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_CROP;	/* DIP_X_PAK2O_CROP, DIP_A_PAK2O_CROP*/

typedef volatile union _DIP_X_REG_IMGI_BASE_ADDR_
{
		volatile struct	/* 0x15022500 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_BASE_ADDR;	/* DIP_X_IMGI_BASE_ADDR, DIP_A_IMGI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGI_OFST_ADDR_
{
		volatile struct	/* 0x15022508 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_OFST_ADDR;	/* DIP_X_IMGI_OFST_ADDR, DIP_A_IMGI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGI_XSIZE_
{
		volatile struct	/* 0x15022510 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_XSIZE;	/* DIP_X_IMGI_XSIZE, DIP_A_IMGI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGI_YSIZE_
{
		volatile struct	/* 0x15022514 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_YSIZE;	/* DIP_X_IMGI_YSIZE, DIP_A_IMGI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGI_STRIDE_
{
		volatile struct	/* 0x15022518 */
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
}DIP_X_REG_IMGI_STRIDE;	/* DIP_X_IMGI_STRIDE, DIP_A_IMGI_STRIDE*/

typedef volatile union _DIP_X_REG_IMGI_CON_
{
		volatile struct	/* 0x1502251C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_CON;	/* DIP_X_IMGI_CON, DIP_A_IMGI_CON*/

typedef volatile union _DIP_X_REG_IMGI_CON2_
{
		volatile struct	/* 0x15022520 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_CON2;	/* DIP_X_IMGI_CON2, DIP_A_IMGI_CON2*/

typedef volatile union _DIP_X_REG_IMGI_CON3_
{
		volatile struct	/* 0x15022524 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_CON3;	/* DIP_X_IMGI_CON3, DIP_A_IMGI_CON3*/

typedef volatile union _DIP_X_REG_IMGBI_BASE_ADDR_
{
		volatile struct	/* 0x15022530 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_BASE_ADDR;	/* DIP_X_IMGBI_BASE_ADDR, DIP_A_IMGBI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGBI_OFST_ADDR_
{
		volatile struct	/* 0x15022538 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_OFST_ADDR;	/* DIP_X_IMGBI_OFST_ADDR, DIP_A_IMGBI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGBI_XSIZE_
{
		volatile struct	/* 0x15022540 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_XSIZE;	/* DIP_X_IMGBI_XSIZE, DIP_A_IMGBI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGBI_YSIZE_
{
		volatile struct	/* 0x15022544 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_YSIZE;	/* DIP_X_IMGBI_YSIZE, DIP_A_IMGBI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGBI_STRIDE_
{
		volatile struct	/* 0x15022548 */
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
}DIP_X_REG_IMGBI_STRIDE;	/* DIP_X_IMGBI_STRIDE, DIP_A_IMGBI_STRIDE*/

typedef volatile union _DIP_X_REG_IMGBI_CON_
{
		volatile struct	/* 0x1502254C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_CON;	/* DIP_X_IMGBI_CON, DIP_A_IMGBI_CON*/

typedef volatile union _DIP_X_REG_IMGBI_CON2_
{
		volatile struct	/* 0x15022550 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_CON2;	/* DIP_X_IMGBI_CON2, DIP_A_IMGBI_CON2*/

typedef volatile union _DIP_X_REG_IMGBI_CON3_
{
		volatile struct	/* 0x15022554 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_CON3;	/* DIP_X_IMGBI_CON3, DIP_A_IMGBI_CON3*/

typedef volatile union _DIP_X_REG_IMGCI_BASE_ADDR_
{
		volatile struct	/* 0x15022560 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_BASE_ADDR;	/* DIP_X_IMGCI_BASE_ADDR, DIP_A_IMGCI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGCI_OFST_ADDR_
{
		volatile struct	/* 0x15022568 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_OFST_ADDR;	/* DIP_X_IMGCI_OFST_ADDR, DIP_A_IMGCI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGCI_XSIZE_
{
		volatile struct	/* 0x15022570 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_XSIZE;	/* DIP_X_IMGCI_XSIZE, DIP_A_IMGCI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGCI_YSIZE_
{
		volatile struct	/* 0x15022574 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_YSIZE;	/* DIP_X_IMGCI_YSIZE, DIP_A_IMGCI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGCI_STRIDE_
{
		volatile struct	/* 0x15022578 */
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
}DIP_X_REG_IMGCI_STRIDE;	/* DIP_X_IMGCI_STRIDE, DIP_A_IMGCI_STRIDE*/

typedef volatile union _DIP_X_REG_IMGCI_CON_
{
		volatile struct	/* 0x1502257C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_CON;	/* DIP_X_IMGCI_CON, DIP_A_IMGCI_CON*/

typedef volatile union _DIP_X_REG_IMGCI_CON2_
{
		volatile struct	/* 0x15022580 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_CON2;	/* DIP_X_IMGCI_CON2, DIP_A_IMGCI_CON2*/

typedef volatile union _DIP_X_REG_IMGCI_CON3_
{
		volatile struct	/* 0x15022584 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_CON3;	/* DIP_X_IMGCI_CON3, DIP_A_IMGCI_CON3*/

typedef volatile union _DIP_X_REG_VIPI_BASE_ADDR_
{
		volatile struct	/* 0x15022590 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_BASE_ADDR;	/* DIP_X_VIPI_BASE_ADDR, DIP_A_VIPI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIPI_OFST_ADDR_
{
		volatile struct	/* 0x15022598 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_OFST_ADDR;	/* DIP_X_VIPI_OFST_ADDR, DIP_A_VIPI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIPI_XSIZE_
{
		volatile struct	/* 0x150225A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_XSIZE;	/* DIP_X_VIPI_XSIZE, DIP_A_VIPI_XSIZE*/

typedef volatile union _DIP_X_REG_VIPI_YSIZE_
{
		volatile struct	/* 0x150225A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_YSIZE;	/* DIP_X_VIPI_YSIZE, DIP_A_VIPI_YSIZE*/

typedef volatile union _DIP_X_REG_VIPI_STRIDE_
{
		volatile struct	/* 0x150225A8 */
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
}DIP_X_REG_VIPI_STRIDE;	/* DIP_X_VIPI_STRIDE, DIP_A_VIPI_STRIDE*/

typedef volatile union _DIP_X_REG_VIPI_CON_
{
		volatile struct	/* 0x150225AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_CON;	/* DIP_X_VIPI_CON, DIP_A_VIPI_CON*/

typedef volatile union _DIP_X_REG_VIPI_CON2_
{
		volatile struct	/* 0x150225B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_CON2;	/* DIP_X_VIPI_CON2, DIP_A_VIPI_CON2*/

typedef volatile union _DIP_X_REG_VIPI_CON3_
{
		volatile struct	/* 0x150225B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_CON3;	/* DIP_X_VIPI_CON3, DIP_A_VIPI_CON3*/

typedef volatile union _DIP_X_REG_VIP2I_BASE_ADDR_
{
		volatile struct	/* 0x150225C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_BASE_ADDR;	/* DIP_X_VIP2I_BASE_ADDR, DIP_A_VIP2I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIP2I_OFST_ADDR_
{
		volatile struct	/* 0x150225C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_OFST_ADDR;	/* DIP_X_VIP2I_OFST_ADDR, DIP_A_VIP2I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIP2I_XSIZE_
{
		volatile struct	/* 0x150225D0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_XSIZE;	/* DIP_X_VIP2I_XSIZE, DIP_A_VIP2I_XSIZE*/

typedef volatile union _DIP_X_REG_VIP2I_YSIZE_
{
		volatile struct	/* 0x150225D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_YSIZE;	/* DIP_X_VIP2I_YSIZE, DIP_A_VIP2I_YSIZE*/

typedef volatile union _DIP_X_REG_VIP2I_STRIDE_
{
		volatile struct	/* 0x150225D8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_STRIDE;	/* DIP_X_VIP2I_STRIDE, DIP_A_VIP2I_STRIDE*/

typedef volatile union _DIP_X_REG_VIP2I_CON_
{
		volatile struct	/* 0x150225DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_CON;	/* DIP_X_VIP2I_CON, DIP_A_VIP2I_CON*/

typedef volatile union _DIP_X_REG_VIP2I_CON2_
{
		volatile struct	/* 0x150225E0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_CON2;	/* DIP_X_VIP2I_CON2, DIP_A_VIP2I_CON2*/

typedef volatile union _DIP_X_REG_VIP2I_CON3_
{
		volatile struct	/* 0x150225E4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_CON3;	/* DIP_X_VIP2I_CON3, DIP_A_VIP2I_CON3*/

typedef volatile union _DIP_X_REG_VIP3I_BASE_ADDR_
{
		volatile struct	/* 0x150225F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_BASE_ADDR;	/* DIP_X_VIP3I_BASE_ADDR, DIP_A_VIP3I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIP3I_OFST_ADDR_
{
		volatile struct	/* 0x150225F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_OFST_ADDR;	/* DIP_X_VIP3I_OFST_ADDR, DIP_A_VIP3I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIP3I_XSIZE_
{
		volatile struct	/* 0x15022600 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_XSIZE;	/* DIP_X_VIP3I_XSIZE, DIP_A_VIP3I_XSIZE*/

typedef volatile union _DIP_X_REG_VIP3I_YSIZE_
{
		volatile struct	/* 0x15022604 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_YSIZE;	/* DIP_X_VIP3I_YSIZE, DIP_A_VIP3I_YSIZE*/

typedef volatile union _DIP_X_REG_VIP3I_STRIDE_
{
		volatile struct	/* 0x15022608 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_STRIDE;	/* DIP_X_VIP3I_STRIDE, DIP_A_VIP3I_STRIDE*/

typedef volatile union _DIP_X_REG_VIP3I_CON_
{
		volatile struct	/* 0x1502260C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_CON;	/* DIP_X_VIP3I_CON, DIP_A_VIP3I_CON*/

typedef volatile union _DIP_X_REG_VIP3I_CON2_
{
		volatile struct	/* 0x15022610 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_CON2;	/* DIP_X_VIP3I_CON2, DIP_A_VIP3I_CON2*/

typedef volatile union _DIP_X_REG_VIP3I_CON3_
{
		volatile struct	/* 0x15022614 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_CON3;	/* DIP_X_VIP3I_CON3, DIP_A_VIP3I_CON3*/

typedef volatile union _DIP_X_REG_DMGI_BASE_ADDR_
{
		volatile struct	/* 0x15022620 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_BASE_ADDR;	/* DIP_X_DMGI_BASE_ADDR, DIP_A_DMGI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_DMGI_OFST_ADDR_
{
		volatile struct	/* 0x15022628 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_OFST_ADDR;	/* DIP_X_DMGI_OFST_ADDR, DIP_A_DMGI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_DMGI_XSIZE_
{
		volatile struct	/* 0x15022630 */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_XSIZE;	/* DIP_X_DMGI_XSIZE, DIP_A_DMGI_XSIZE*/

typedef volatile union _DIP_X_REG_DMGI_YSIZE_
{
		volatile struct	/* 0x15022634 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_YSIZE;	/* DIP_X_DMGI_YSIZE, DIP_A_DMGI_YSIZE*/

typedef volatile union _DIP_X_REG_DMGI_STRIDE_
{
		volatile struct	/* 0x15022638 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_STRIDE;	/* DIP_X_DMGI_STRIDE, DIP_A_DMGI_STRIDE*/

typedef volatile union _DIP_X_REG_DMGI_CON_
{
		volatile struct	/* 0x1502263C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_CON;	/* DIP_X_DMGI_CON, DIP_A_DMGI_CON*/

typedef volatile union _DIP_X_REG_DMGI_CON2_
{
		volatile struct	/* 0x15022640 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_CON2;	/* DIP_X_DMGI_CON2, DIP_A_DMGI_CON2*/

typedef volatile union _DIP_X_REG_DMGI_CON3_
{
		volatile struct	/* 0x15022644 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_CON3;	/* DIP_X_DMGI_CON3, DIP_A_DMGI_CON3*/

typedef volatile union _DIP_X_REG_DEPI_BASE_ADDR_
{
		volatile struct	/* 0x15022650 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_BASE_ADDR;	/* DIP_X_DEPI_BASE_ADDR, DIP_A_DEPI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_DEPI_OFST_ADDR_
{
		volatile struct	/* 0x15022658 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_OFST_ADDR;	/* DIP_X_DEPI_OFST_ADDR, DIP_A_DEPI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_DEPI_XSIZE_
{
		volatile struct	/* 0x15022660 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_XSIZE;	/* DIP_X_DEPI_XSIZE, DIP_A_DEPI_XSIZE*/

typedef volatile union _DIP_X_REG_DEPI_YSIZE_
{
		volatile struct	/* 0x15022664 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_YSIZE;	/* DIP_X_DEPI_YSIZE, DIP_A_DEPI_YSIZE*/

typedef volatile union _DIP_X_REG_DEPI_STRIDE_
{
		volatile struct	/* 0x15022668 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_STRIDE;	/* DIP_X_DEPI_STRIDE, DIP_A_DEPI_STRIDE*/

typedef volatile union _DIP_X_REG_DEPI_CON_
{
		volatile struct	/* 0x1502266C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_CON;	/* DIP_X_DEPI_CON, DIP_A_DEPI_CON*/

typedef volatile union _DIP_X_REG_DEPI_CON2_
{
		volatile struct	/* 0x15022670 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_CON2;	/* DIP_X_DEPI_CON2, DIP_A_DEPI_CON2*/

typedef volatile union _DIP_X_REG_DEPI_CON3_
{
		volatile struct	/* 0x15022674 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_CON3;	/* DIP_X_DEPI_CON3, DIP_A_DEPI_CON3*/

typedef volatile union _DIP_X_REG_LCEI_BASE_ADDR_
{
		volatile struct	/* 0x15022680 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_BASE_ADDR;	/* DIP_X_LCEI_BASE_ADDR, DIP_A_LCEI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_LCEI_OFST_ADDR_
{
		volatile struct	/* 0x15022688 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_OFST_ADDR;	/* DIP_X_LCEI_OFST_ADDR, DIP_A_LCEI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_LCEI_XSIZE_
{
		volatile struct	/* 0x15022690 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_XSIZE;	/* DIP_X_LCEI_XSIZE, DIP_A_LCEI_XSIZE*/

typedef volatile union _DIP_X_REG_LCEI_YSIZE_
{
		volatile struct	/* 0x15022694 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_YSIZE;	/* DIP_X_LCEI_YSIZE, DIP_A_LCEI_YSIZE*/

typedef volatile union _DIP_X_REG_LCEI_STRIDE_
{
		volatile struct	/* 0x15022698 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_STRIDE;	/* DIP_X_LCEI_STRIDE, DIP_A_LCEI_STRIDE*/

typedef volatile union _DIP_X_REG_LCEI_CON_
{
		volatile struct	/* 0x1502269C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_CON;	/* DIP_X_LCEI_CON, DIP_A_LCEI_CON*/

typedef volatile union _DIP_X_REG_LCEI_CON2_
{
		volatile struct	/* 0x150226A0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_CON2;	/* DIP_X_LCEI_CON2, DIP_A_LCEI_CON2*/

typedef volatile union _DIP_X_REG_LCEI_CON3_
{
		volatile struct	/* 0x150226A4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_CON3;	/* DIP_X_LCEI_CON3, DIP_A_LCEI_CON3*/

typedef volatile union _DIP_X_REG_UFDI_BASE_ADDR_
{
		volatile struct	/* 0x150226B0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_BASE_ADDR;	/* DIP_X_UFDI_BASE_ADDR, DIP_A_UFDI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_UFDI_OFST_ADDR_
{
		volatile struct	/* 0x150226B8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_OFST_ADDR;	/* DIP_X_UFDI_OFST_ADDR, DIP_A_UFDI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_UFDI_XSIZE_
{
		volatile struct	/* 0x150226C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_XSIZE;	/* DIP_X_UFDI_XSIZE, DIP_A_UFDI_XSIZE*/

typedef volatile union _DIP_X_REG_UFDI_YSIZE_
{
		volatile struct	/* 0x150226C4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_YSIZE;	/* DIP_X_UFDI_YSIZE, DIP_A_UFDI_YSIZE*/

typedef volatile union _DIP_X_REG_UFDI_STRIDE_
{
		volatile struct	/* 0x150226C8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  2;		/* 21..22, 0x00600000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_STRIDE;	/* DIP_X_UFDI_STRIDE, DIP_A_UFDI_STRIDE*/

typedef volatile union _DIP_X_REG_UFDI_CON_
{
		volatile struct	/* 0x150226CC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_CON;	/* DIP_X_UFDI_CON, DIP_A_UFDI_CON*/

typedef volatile union _DIP_X_REG_UFDI_CON2_
{
		volatile struct	/* 0x150226D0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_CON2;	/* DIP_X_UFDI_CON2, DIP_A_UFDI_CON2*/

typedef volatile union _DIP_X_REG_UFDI_CON3_
{
		volatile struct	/* 0x150226D4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_CON3;	/* DIP_X_UFDI_CON3, DIP_A_UFDI_CON3*/

typedef volatile union _DIP_X_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x15022740 */
		{
				FIELD  IMG2O_ERR                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_ERR                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_ERR                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_ERR                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_ERR                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_ERR                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAK2O_ERR                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMX1O_ERR                             :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SMX2O_ERR                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SMX3O_ERR                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SMX4O_ERR                             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  IMGI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  IMGBI_ERR                             :  1;		/* 17..17, 0x00020000 */
				FIELD  IMGCI_ERR                             :  1;		/* 18..18, 0x00040000 */
				FIELD  VIPI_ERR                              :  1;		/* 19..19, 0x00080000 */
				FIELD  VIP2I_ERR                             :  1;		/* 20..20, 0x00100000 */
				FIELD  VIP3I_ERR                             :  1;		/* 21..21, 0x00200000 */
				FIELD  DMGI_ERR                              :  1;		/* 22..22, 0x00400000 */
				FIELD  DEPI_ERR                              :  1;		/* 23..23, 0x00800000 */
				FIELD  LCEI_ERR                              :  1;		/* 24..24, 0x01000000 */
				FIELD  UFDI_ERR                              :  1;		/* 25..25, 0x02000000 */
				FIELD  SMX1I_ERR                             :  1;		/* 26..26, 0x04000000 */
				FIELD  SMX2I_ERR                             :  1;		/* 27..27, 0x08000000 */
				FIELD  SMX3I_ERR                             :  1;		/* 28..28, 0x10000000 */
				FIELD  SMX4I_ERR                             :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_ERR_CTRL;	/* DIP_X_DMA_ERR_CTRL, DIP_A_DMA_ERR_CTRL*/

typedef volatile union _DIP_X_REG_IMG2O_ERR_STAT_
{
		volatile struct	/* 0x15022744 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_ERR_STAT;	/* DIP_X_IMG2O_ERR_STAT, DIP_A_IMG2O_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG2BO_ERR_STAT_
{
		volatile struct	/* 0x15022748 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_ERR_STAT;	/* DIP_X_IMG2BO_ERR_STAT, DIP_A_IMG2BO_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3O_ERR_STAT_
{
		volatile struct	/* 0x1502274C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_ERR_STAT;	/* DIP_X_IMG3O_ERR_STAT, DIP_A_IMG3O_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3BO_ERR_STAT_
{
		volatile struct	/* 0x15022750 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_ERR_STAT;	/* DIP_X_IMG3BO_ERR_STAT, DIP_A_IMG3BO_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3CO_ERR_STAT_
{
		volatile struct	/* 0x15022754 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_ERR_STAT;	/* DIP_X_IMG3CO_ERR_STAT, DIP_A_IMG3CO_ERR_STAT*/

typedef volatile union _DIP_X_REG_FEO_ERR_STAT_
{
		volatile struct	/* 0x15022758 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_ERR_STAT;	/* DIP_X_FEO_ERR_STAT, DIP_A_FEO_ERR_STAT*/

typedef volatile union _DIP_X_REG_PAK2O_ERR_STAT_
{
		volatile struct	/* 0x1502275C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PAK2O_ERR_STAT;	/* DIP_X_PAK2O_ERR_STAT, DIP_A_PAK2O_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGI_ERR_STAT_
{
		volatile struct	/* 0x15022760 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_ERR_STAT;	/* DIP_X_IMGI_ERR_STAT, DIP_A_IMGI_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGBI_ERR_STAT_
{
		volatile struct	/* 0x15022764 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_ERR_STAT;	/* DIP_X_IMGBI_ERR_STAT, DIP_A_IMGBI_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGCI_ERR_STAT_
{
		volatile struct	/* 0x15022768 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_ERR_STAT;	/* DIP_X_IMGCI_ERR_STAT, DIP_A_IMGCI_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIPI_ERR_STAT_
{
		volatile struct	/* 0x1502276C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_ERR_STAT;	/* DIP_X_VIPI_ERR_STAT, DIP_A_VIPI_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIP2I_ERR_STAT_
{
		volatile struct	/* 0x15022770 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_ERR_STAT;	/* DIP_X_VIP2I_ERR_STAT, DIP_A_VIP2I_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIP3I_ERR_STAT_
{
		volatile struct	/* 0x15022774 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_ERR_STAT;	/* DIP_X_VIP3I_ERR_STAT, DIP_A_VIP3I_ERR_STAT*/

typedef volatile union _DIP_X_REG_DMGI_ERR_STAT_
{
		volatile struct	/* 0x15022778 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_ERR_STAT;	/* DIP_X_DMGI_ERR_STAT, DIP_A_DMGI_ERR_STAT*/

typedef volatile union _DIP_X_REG_DEPI_ERR_STAT_
{
		volatile struct	/* 0x1502277C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_ERR_STAT;	/* DIP_X_DEPI_ERR_STAT, DIP_A_DEPI_ERR_STAT*/

typedef volatile union _DIP_X_REG_LCEI_ERR_STAT_
{
		volatile struct	/* 0x15022780 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_ERR_STAT;	/* DIP_X_LCEI_ERR_STAT, DIP_A_LCEI_ERR_STAT*/

typedef volatile union _DIP_X_REG_UFDI_ERR_STAT_
{
		volatile struct	/* 0x15022784 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_ERR_STAT;	/* DIP_X_UFDI_ERR_STAT, DIP_A_UFDI_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX1O_ERR_STAT_
{
		volatile struct	/* 0x15022788 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_ERR_STAT;	/* DIP_X_SMX1O_ERR_STAT, DIP_A_SMX1O_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX2O_ERR_STAT_
{
		volatile struct	/* 0x1502278C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_ERR_STAT;	/* DIP_X_SMX2O_ERR_STAT, DIP_A_SMX2O_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX3O_ERR_STAT_
{
		volatile struct	/* 0x15022790 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_ERR_STAT;	/* DIP_X_SMX3O_ERR_STAT, DIP_A_SMX3O_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX4O_ERR_STAT_
{
		volatile struct	/* 0x15022794 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_ERR_STAT;	/* DIP_X_SMX4O_ERR_STAT, DIP_A_SMX4O_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX1I_ERR_STAT_
{
		volatile struct	/* 0x15022798 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_ERR_STAT;	/* DIP_X_SMX1I_ERR_STAT, DIP_A_SMX1I_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX2I_ERR_STAT_
{
		volatile struct	/* 0x1502279C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_ERR_STAT;	/* DIP_X_SMX2I_ERR_STAT, DIP_A_SMX2I_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX3I_ERR_STAT_
{
		volatile struct	/* 0x150227A0 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_ERR_STAT;	/* DIP_X_SMX3I_ERR_STAT, DIP_A_SMX3I_ERR_STAT*/

typedef volatile union _DIP_X_REG_SMX4I_ERR_STAT_
{
		volatile struct	/* 0x150227A4 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_ERR_STAT;	/* DIP_X_SMX4I_ERR_STAT, DIP_A_SMX4I_ERR_STAT*/

typedef volatile union _DIP_X_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x150227A8 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_DEBUG_ADDR;	/* DIP_X_DMA_DEBUG_ADDR, DIP_A_DMA_DEBUG_ADDR*/

typedef volatile union _DIP_X_REG_DMA_RSV1_
{
		volatile struct	/* 0x150227AC */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV1;	/* DIP_X_DMA_RSV1, DIP_A_DMA_RSV1*/

typedef volatile union _DIP_X_REG_DMA_RSV2_
{
		volatile struct	/* 0x150227B0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV2;	/* DIP_X_DMA_RSV2, DIP_A_DMA_RSV2*/

typedef volatile union _DIP_X_REG_DMA_RSV3_
{
		volatile struct	/* 0x150227B4 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV3;	/* DIP_X_DMA_RSV3, DIP_A_DMA_RSV3*/

typedef volatile union _DIP_X_REG_DMA_RSV4_
{
		volatile struct	/* 0x150227B8 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV4;	/* DIP_X_DMA_RSV4, DIP_A_DMA_RSV4*/

typedef volatile union _DIP_X_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x150227BC */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  2;		/* 24..25, 0x03000000 */
				FIELD  IMGI_ADDR_GEN_WAIT                    :  1;		/* 26..26, 0x04000000 */
				FIELD  IMGI_UFD_FIFO_FULL_OFFSET             :  1;		/* 27..27, 0x08000000 */
				FIELD  IMGI_UFD_FIFO_FULL_XSIZE              :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_DEBUG_SEL;	/* DIP_X_DMA_DEBUG_SEL, DIP_A_DMA_DEBUG_SEL*/

typedef volatile union _DIP_X_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x150227C0 */
		{
				FIELD  BW_SELF_TEST_EN_IMG2O                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_IMG2BO                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_IMG3O                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_IMG3BO                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  BW_SELF_TEST_EN_IMG3CO                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BW_SELF_TEST_EN_FEO                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BW_SELF_TEST_EN_PAK2O                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BW_SELF_TEST_EN_SMX1O                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  BW_SELF_TEST_EN_SMX2O                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BW_SELF_TEST_EN_SMX3O                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BW_SELF_TEST_EN_SMX4O                 :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_BW_SELF_TEST;	/* DIP_X_DMA_BW_SELF_TEST, DIP_A_DMA_BW_SELF_TEST*/

typedef volatile union _DIP_X_REG_SMX1O_BASE_ADDR_
{
		volatile struct	/* 0x150227D0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_BASE_ADDR;	/* DIP_X_SMX1O_BASE_ADDR, DIP_A_SMX1O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX1O_OFST_ADDR_
{
		volatile struct	/* 0x150227D8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_OFST_ADDR;	/* DIP_X_SMX1O_OFST_ADDR, DIP_A_SMX1O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX1O_XSIZE_
{
		volatile struct	/* 0x150227E0 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_XSIZE;	/* DIP_X_SMX1O_XSIZE, DIP_A_SMX1O_XSIZE*/

typedef volatile union _DIP_X_REG_SMX1O_YSIZE_
{
		volatile struct	/* 0x150227E4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_YSIZE;	/* DIP_X_SMX1O_YSIZE, DIP_A_SMX1O_YSIZE*/

typedef volatile union _DIP_X_REG_SMX1O_STRIDE_
{
		volatile struct	/* 0x150227E8 */
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
}DIP_X_REG_SMX1O_STRIDE;	/* DIP_X_SMX1O_STRIDE, DIP_A_SMX1O_STRIDE*/

typedef volatile union _DIP_X_REG_SMX1O_CON_
{
		volatile struct	/* 0x150227EC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_CON;	/* DIP_X_SMX1O_CON, DIP_A_SMX1O_CON*/

typedef volatile union _DIP_X_REG_SMX1O_CON2_
{
		volatile struct	/* 0x150227F0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_CON2;	/* DIP_X_SMX1O_CON2, DIP_A_SMX1O_CON2*/

typedef volatile union _DIP_X_REG_SMX1O_CON3_
{
		volatile struct	/* 0x150227F4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_CON3;	/* DIP_X_SMX1O_CON3, DIP_A_SMX1O_CON3*/

typedef volatile union _DIP_X_REG_SMX1O_CROP_
{
		volatile struct	/* 0x150227F8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1O_CROP;	/* DIP_X_SMX1O_CROP, DIP_A_SMX1O_CROP*/

typedef volatile union _DIP_X_REG_SMX2O_BASE_ADDR_
{
		volatile struct	/* 0x15022800 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_BASE_ADDR;	/* DIP_X_SMX2O_BASE_ADDR, DIP_A_SMX2O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX2O_OFST_ADDR_
{
		volatile struct	/* 0x15022808 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_OFST_ADDR;	/* DIP_X_SMX2O_OFST_ADDR, DIP_A_SMX2O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX2O_XSIZE_
{
		volatile struct	/* 0x15022810 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_XSIZE;	/* DIP_X_SMX2O_XSIZE, DIP_A_SMX2O_XSIZE*/

typedef volatile union _DIP_X_REG_SMX2O_YSIZE_
{
		volatile struct	/* 0x15022814 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_YSIZE;	/* DIP_X_SMX2O_YSIZE, DIP_A_SMX2O_YSIZE*/

typedef volatile union _DIP_X_REG_SMX2O_STRIDE_
{
		volatile struct	/* 0x15022818 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_STRIDE;	/* DIP_X_SMX2O_STRIDE, DIP_A_SMX2O_STRIDE*/

typedef volatile union _DIP_X_REG_SMX2O_CON_
{
		volatile struct	/* 0x1502281C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_CON;	/* DIP_X_SMX2O_CON, DIP_A_SMX2O_CON*/

typedef volatile union _DIP_X_REG_SMX2O_CON2_
{
		volatile struct	/* 0x15022820 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_CON2;	/* DIP_X_SMX2O_CON2, DIP_A_SMX2O_CON2*/

typedef volatile union _DIP_X_REG_SMX2O_CON3_
{
		volatile struct	/* 0x15022824 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_CON3;	/* DIP_X_SMX2O_CON3, DIP_A_SMX2O_CON3*/

typedef volatile union _DIP_X_REG_SMX2O_CROP_
{
		volatile struct	/* 0x15022828 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2O_CROP;	/* DIP_X_SMX2O_CROP, DIP_A_SMX2O_CROP*/

typedef volatile union _DIP_X_REG_SMX3O_BASE_ADDR_
{
		volatile struct	/* 0x15022830 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_BASE_ADDR;	/* DIP_X_SMX3O_BASE_ADDR, DIP_A_SMX3O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX3O_OFST_ADDR_
{
		volatile struct	/* 0x15022838 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_OFST_ADDR;	/* DIP_X_SMX3O_OFST_ADDR, DIP_A_SMX3O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX3O_XSIZE_
{
		volatile struct	/* 0x15022840 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_XSIZE;	/* DIP_X_SMX3O_XSIZE, DIP_A_SMX3O_XSIZE*/

typedef volatile union _DIP_X_REG_SMX3O_YSIZE_
{
		volatile struct	/* 0x15022844 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_YSIZE;	/* DIP_X_SMX3O_YSIZE, DIP_A_SMX3O_YSIZE*/

typedef volatile union _DIP_X_REG_SMX3O_STRIDE_
{
		volatile struct	/* 0x15022848 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_STRIDE;	/* DIP_X_SMX3O_STRIDE, DIP_A_SMX3O_STRIDE*/

typedef volatile union _DIP_X_REG_SMX3O_CON_
{
		volatile struct	/* 0x1502284C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_CON;	/* DIP_X_SMX3O_CON, DIP_A_SMX3O_CON*/

typedef volatile union _DIP_X_REG_SMX3O_CON2_
{
		volatile struct	/* 0x15022850 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_CON2;	/* DIP_X_SMX3O_CON2, DIP_A_SMX3O_CON2*/

typedef volatile union _DIP_X_REG_SMX3O_CON3_
{
		volatile struct	/* 0x15022854 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_CON3;	/* DIP_X_SMX3O_CON3, DIP_A_SMX3O_CON3*/

typedef volatile union _DIP_X_REG_SMX3O_CROP_
{
		volatile struct	/* 0x15022858 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3O_CROP;	/* DIP_X_SMX3O_CROP, DIP_A_SMX3O_CROP*/

typedef volatile union _DIP_X_REG_SMX4O_BASE_ADDR_
{
		volatile struct	/* 0x15022860 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_BASE_ADDR;	/* DIP_X_SMX4O_BASE_ADDR, DIP_A_SMX4O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX4O_OFST_ADDR_
{
		volatile struct	/* 0x15022868 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_OFST_ADDR;	/* DIP_X_SMX4O_OFST_ADDR, DIP_A_SMX4O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX4O_XSIZE_
{
		volatile struct	/* 0x15022870 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_XSIZE;	/* DIP_X_SMX4O_XSIZE, DIP_A_SMX4O_XSIZE*/

typedef volatile union _DIP_X_REG_SMX4O_YSIZE_
{
		volatile struct	/* 0x15022874 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_YSIZE;	/* DIP_X_SMX4O_YSIZE, DIP_A_SMX4O_YSIZE*/

typedef volatile union _DIP_X_REG_SMX4O_STRIDE_
{
		volatile struct	/* 0x15022878 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_STRIDE;	/* DIP_X_SMX4O_STRIDE, DIP_A_SMX4O_STRIDE*/

typedef volatile union _DIP_X_REG_SMX4O_CON_
{
		volatile struct	/* 0x1502287C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_CON;	/* DIP_X_SMX4O_CON, DIP_A_SMX4O_CON*/

typedef volatile union _DIP_X_REG_SMX4O_CON2_
{
		volatile struct	/* 0x15022880 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_CON2;	/* DIP_X_SMX4O_CON2, DIP_A_SMX4O_CON2*/

typedef volatile union _DIP_X_REG_SMX4O_CON3_
{
		volatile struct	/* 0x15022884 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_CON3;	/* DIP_X_SMX4O_CON3, DIP_A_SMX4O_CON3*/

typedef volatile union _DIP_X_REG_SMX4O_CROP_
{
		volatile struct	/* 0x15022888 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4O_CROP;	/* DIP_X_SMX4O_CROP, DIP_A_SMX4O_CROP*/

typedef volatile union _DIP_X_REG_SMX1I_BASE_ADDR_
{
		volatile struct	/* 0x15022890 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_BASE_ADDR;	/* DIP_X_SMX1I_BASE_ADDR, DIP_A_SMX1I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX1I_OFST_ADDR_
{
		volatile struct	/* 0x15022898 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_OFST_ADDR;	/* DIP_X_SMX1I_OFST_ADDR, DIP_A_SMX1I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX1I_XSIZE_
{
		volatile struct	/* 0x150228A0 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_XSIZE;	/* DIP_X_SMX1I_XSIZE, DIP_A_SMX1I_XSIZE*/

typedef volatile union _DIP_X_REG_SMX1I_YSIZE_
{
		volatile struct	/* 0x150228A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_YSIZE;	/* DIP_X_SMX1I_YSIZE, DIP_A_SMX1I_YSIZE*/

typedef volatile union _DIP_X_REG_SMX1I_STRIDE_
{
		volatile struct	/* 0x150228A8 */
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
}DIP_X_REG_SMX1I_STRIDE;	/* DIP_X_SMX1I_STRIDE, DIP_A_SMX1I_STRIDE*/

typedef volatile union _DIP_X_REG_SMX1I_CON_
{
		volatile struct	/* 0x150228AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_CON;	/* DIP_X_SMX1I_CON, DIP_A_SMX1I_CON*/

typedef volatile union _DIP_X_REG_SMX1I_CON2_
{
		volatile struct	/* 0x150228B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_CON2;	/* DIP_X_SMX1I_CON2, DIP_A_SMX1I_CON2*/

typedef volatile union _DIP_X_REG_SMX1I_CON3_
{
		volatile struct	/* 0x150228B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1I_CON3;	/* DIP_X_SMX1I_CON3, DIP_A_SMX1I_CON3*/

typedef volatile union _DIP_X_REG_SMX2I_BASE_ADDR_
{
		volatile struct	/* 0x150228C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_BASE_ADDR;	/* DIP_X_SMX2I_BASE_ADDR, DIP_A_SMX2I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX2I_OFST_ADDR_
{
		volatile struct	/* 0x150228C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_OFST_ADDR;	/* DIP_X_SMX2I_OFST_ADDR, DIP_A_SMX2I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX2I_XSIZE_
{
		volatile struct	/* 0x150228D0 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_XSIZE;	/* DIP_X_SMX2I_XSIZE, DIP_A_SMX2I_XSIZE*/

typedef volatile union _DIP_X_REG_SMX2I_YSIZE_
{
		volatile struct	/* 0x150228D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_YSIZE;	/* DIP_X_SMX2I_YSIZE, DIP_A_SMX2I_YSIZE*/

typedef volatile union _DIP_X_REG_SMX2I_STRIDE_
{
		volatile struct	/* 0x150228D8 */
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
}DIP_X_REG_SMX2I_STRIDE;	/* DIP_X_SMX2I_STRIDE, DIP_A_SMX2I_STRIDE*/

typedef volatile union _DIP_X_REG_SMX2I_CON_
{
		volatile struct	/* 0x150228DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_CON;	/* DIP_X_SMX2I_CON, DIP_A_SMX2I_CON*/

typedef volatile union _DIP_X_REG_SMX2I_CON2_
{
		volatile struct	/* 0x150228E0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_CON2;	/* DIP_X_SMX2I_CON2, DIP_A_SMX2I_CON2*/

typedef volatile union _DIP_X_REG_SMX2I_CON3_
{
		volatile struct	/* 0x150228E4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2I_CON3;	/* DIP_X_SMX2I_CON3, DIP_A_SMX2I_CON3*/

typedef volatile union _DIP_X_REG_SMX3I_BASE_ADDR_
{
		volatile struct	/* 0x150228F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_BASE_ADDR;	/* DIP_X_SMX3I_BASE_ADDR, DIP_A_SMX3I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX3I_OFST_ADDR_
{
		volatile struct	/* 0x150228F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_OFST_ADDR;	/* DIP_X_SMX3I_OFST_ADDR, DIP_A_SMX3I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX3I_XSIZE_
{
		volatile struct	/* 0x15022900 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_XSIZE;	/* DIP_X_SMX3I_XSIZE, DIP_A_SMX3I_XSIZE*/

typedef volatile union _DIP_X_REG_SMX3I_YSIZE_
{
		volatile struct	/* 0x15022904 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_YSIZE;	/* DIP_X_SMX3I_YSIZE, DIP_A_SMX3I_YSIZE*/

typedef volatile union _DIP_X_REG_SMX3I_STRIDE_
{
		volatile struct	/* 0x15022908 */
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
}DIP_X_REG_SMX3I_STRIDE;	/* DIP_X_SMX3I_STRIDE, DIP_A_SMX3I_STRIDE*/

typedef volatile union _DIP_X_REG_SMX3I_CON_
{
		volatile struct	/* 0x1502290C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_CON;	/* DIP_X_SMX3I_CON, DIP_A_SMX3I_CON*/

typedef volatile union _DIP_X_REG_SMX3I_CON2_
{
		volatile struct	/* 0x15022910 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_CON2;	/* DIP_X_SMX3I_CON2, DIP_A_SMX3I_CON2*/

typedef volatile union _DIP_X_REG_SMX3I_CON3_
{
		volatile struct	/* 0x15022914 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3I_CON3;	/* DIP_X_SMX3I_CON3, DIP_A_SMX3I_CON3*/

typedef volatile union _DIP_X_REG_SMX4I_BASE_ADDR_
{
		volatile struct	/* 0x15022920 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_BASE_ADDR;	/* DIP_X_SMX4I_BASE_ADDR, DIP_A_SMX4I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_SMX4I_OFST_ADDR_
{
		volatile struct	/* 0x15022928 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_OFST_ADDR;	/* DIP_X_SMX4I_OFST_ADDR, DIP_A_SMX4I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_SMX4I_XSIZE_
{
		volatile struct	/* 0x15022930 */
		{
				FIELD  XSIZE                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_XSIZE;	/* DIP_X_SMX4I_XSIZE, DIP_A_SMX4I_XSIZE*/

typedef volatile union _DIP_X_REG_SMX4I_YSIZE_
{
		volatile struct	/* 0x15022934 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_YSIZE;	/* DIP_X_SMX4I_YSIZE, DIP_A_SMX4I_YSIZE*/

typedef volatile union _DIP_X_REG_SMX4I_STRIDE_
{
		volatile struct	/* 0x15022938 */
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
}DIP_X_REG_SMX4I_STRIDE;	/* DIP_X_SMX4I_STRIDE, DIP_A_SMX4I_STRIDE*/

typedef volatile union _DIP_X_REG_SMX4I_CON_
{
		volatile struct	/* 0x1502293C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_CON;	/* DIP_X_SMX4I_CON, DIP_A_SMX4I_CON*/

typedef volatile union _DIP_X_REG_SMX4I_CON2_
{
		volatile struct	/* 0x15022940 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_CON2;	/* DIP_X_SMX4I_CON2, DIP_A_SMX4I_CON2*/

typedef volatile union _DIP_X_REG_SMX4I_CON3_
{
		volatile struct	/* 0x15022944 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4I_CON3;	/* DIP_X_SMX4I_CON3, DIP_A_SMX4I_CON3*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x15022B00 */
		{
				FIELD  IPUO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  IPUI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT;	/* DIP_X_ADL_DMA_A_DMA_SOFT_RSTSTAT, DIP_A_ADL_DMA_A_DMA_SOFT_RSTSTAT*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x15022B04 */
		{
				FIELD  IPUO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  IPUI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_VERTICAL_FLIP_EN;	/* DIP_X_ADL_DMA_A_VERTICAL_FLIP_EN, DIP_A_ADL_DMA_A_VERTICAL_FLIP_EN*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_SOFT_RESET_
{
		volatile struct	/* 0x15022B08 */
		{
				FIELD  IPUO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  IPUI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_SOFT_RESET;	/* DIP_X_ADL_DMA_A_DMA_SOFT_RESET, DIP_A_ADL_DMA_A_DMA_SOFT_RESET*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_LAST_ULTRA_EN_
{
		volatile struct	/* 0x15022B0C */
		{
				FIELD  IPUO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  IPUI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_LAST_ULTRA_EN;	/* DIP_X_ADL_DMA_A_LAST_ULTRA_EN, DIP_A_ADL_DMA_A_LAST_ULTRA_EN*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x15022B10 */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  7;		/* 16..22, 0x007F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 23..24, 0x01800000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  3;		/* 27..29, 0x38000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_SPECIAL_FUN_EN;	/* DIP_X_ADL_DMA_A_SPECIAL_FUN_EN, DIP_A_ADL_DMA_A_SPECIAL_FUN_EN*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_RING_
{
		volatile struct	/* 0x15022B14 */
		{
				FIELD  IPUO_RING_YSIZE                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  IPUO_RING_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_RING;	/* DIP_X_ADL_DMA_A_IPUO_RING, DIP_A_ADL_DMA_A_IPUO_RING*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_RING_
{
		volatile struct	/* 0x15022B18 */
		{
				FIELD  IPUI_RING_YSIZE                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  IPUI_RING_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_RING;	/* DIP_X_ADL_DMA_A_IPUI_RING, DIP_A_ADL_DMA_A_IPUI_RING*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_BASE_ADDR_
{
		volatile struct	/* 0x15022B30 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_BASE_ADDR;	/* DIP_X_ADL_DMA_A_IPUO_BASE_ADDR, DIP_A_ADL_DMA_A_IPUO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_OFST_ADDR_
{
		volatile struct	/* 0x15022B38 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_OFST_ADDR;	/* DIP_X_ADL_DMA_A_IPUO_OFST_ADDR, DIP_A_ADL_DMA_A_IPUO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_XSIZE_
{
		volatile struct	/* 0x15022B40 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_XSIZE;	/* DIP_X_ADL_DMA_A_IPUO_XSIZE, DIP_A_ADL_DMA_A_IPUO_XSIZE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_YSIZE_
{
		volatile struct	/* 0x15022B44 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_YSIZE;	/* DIP_X_ADL_DMA_A_IPUO_YSIZE, DIP_A_ADL_DMA_A_IPUO_YSIZE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_STRIDE_
{
		volatile struct	/* 0x15022B48 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_STRIDE;	/* DIP_X_ADL_DMA_A_IPUO_STRIDE, DIP_A_ADL_DMA_A_IPUO_STRIDE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_CON_
{
		volatile struct	/* 0x15022B4C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_CON;	/* DIP_X_ADL_DMA_A_IPUO_CON, DIP_A_ADL_DMA_A_IPUO_CON*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_CON2_
{
		volatile struct	/* 0x15022B50 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_CON2;	/* DIP_X_ADL_DMA_A_IPUO_CON2, DIP_A_ADL_DMA_A_IPUO_CON2*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_CON3_
{
		volatile struct	/* 0x15022B54 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_CON3;	/* DIP_X_ADL_DMA_A_IPUO_CON3, DIP_A_ADL_DMA_A_IPUO_CON3*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_BASE_ADDR_
{
		volatile struct	/* 0x15022B90 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_BASE_ADDR;	/* DIP_X_ADL_DMA_A_IPUI_BASE_ADDR, DIP_A_ADL_DMA_A_IPUI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_OFST_ADDR_
{
		volatile struct	/* 0x15022B98 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_OFST_ADDR;	/* DIP_X_ADL_DMA_A_IPUI_OFST_ADDR, DIP_A_ADL_DMA_A_IPUI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_XSIZE_
{
		volatile struct	/* 0x15022BA0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_XSIZE;	/* DIP_X_ADL_DMA_A_IPUI_XSIZE, DIP_A_ADL_DMA_A_IPUI_XSIZE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_YSIZE_
{
		volatile struct	/* 0x15022BA4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_YSIZE;	/* DIP_X_ADL_DMA_A_IPUI_YSIZE, DIP_A_ADL_DMA_A_IPUI_YSIZE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_STRIDE_
{
		volatile struct	/* 0x15022BA8 */
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
}DIP_X_REG_ADL_DMA_A_IPUI_STRIDE;	/* DIP_X_ADL_DMA_A_IPUI_STRIDE, DIP_A_ADL_DMA_A_IPUI_STRIDE*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_CON_
{
		volatile struct	/* 0x15022BAC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_CON;	/* DIP_X_ADL_DMA_A_IPUI_CON, DIP_A_ADL_DMA_A_IPUI_CON*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_CON2_
{
		volatile struct	/* 0x15022BB0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_CON2;	/* DIP_X_ADL_DMA_A_IPUI_CON2, DIP_A_ADL_DMA_A_IPUI_CON2*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_CON3_
{
		volatile struct	/* 0x15022BB4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_CON3;	/* DIP_X_ADL_DMA_A_IPUI_CON3, DIP_A_ADL_DMA_A_IPUI_CON3*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_ERR_CTRL_
{
		volatile struct	/* 0x15022C00 */
		{
				FIELD  IPUO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  IPUI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_ERR_CTRL;	/* DIP_X_ADL_DMA_A_DMA_ERR_CTRL, DIP_A_ADL_DMA_A_DMA_ERR_CTRL*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUO_ERR_STAT_
{
		volatile struct	/* 0x15022C04 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUO_ERR_STAT;	/* DIP_X_ADL_DMA_A_IPUO_ERR_STAT, DIP_A_ADL_DMA_A_IPUO_ERR_STAT*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_IPUI_ERR_STAT_
{
		volatile struct	/* 0x15022C08 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_IPUI_ERR_STAT;	/* DIP_X_ADL_DMA_A_IPUI_ERR_STAT, DIP_A_ADL_DMA_A_IPUI_ERR_STAT*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x15022C0C */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_DEBUG_ADDR;	/* DIP_X_ADL_DMA_A_DMA_DEBUG_ADDR, DIP_A_ADL_DMA_A_DMA_DEBUG_ADDR*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV1_
{
		volatile struct	/* 0x15022C10 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV1;	/* DIP_X_ADL_DMA_A_DMA_RSV1, DIP_A_ADL_DMA_A_DMA_RSV1*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV2_
{
		volatile struct	/* 0x15022C14 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV2;	/* DIP_X_ADL_DMA_A_DMA_RSV2, DIP_A_ADL_DMA_A_DMA_RSV2*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV3_
{
		volatile struct	/* 0x15022C18 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV3;	/* DIP_X_ADL_DMA_A_DMA_RSV3, DIP_A_ADL_DMA_A_DMA_RSV3*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV4_
{
		volatile struct	/* 0x15022C1C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV4;	/* DIP_X_ADL_DMA_A_DMA_RSV4, DIP_A_ADL_DMA_A_DMA_RSV4*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV5_
{
		volatile struct	/* 0x15022C20 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV5;	/* DIP_X_ADL_DMA_A_DMA_RSV5, DIP_A_ADL_DMA_A_DMA_RSV5*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_RSV6_
{
		volatile struct	/* 0x15022C24 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_RSV6;	/* DIP_X_ADL_DMA_A_DMA_RSV6, DIP_A_ADL_DMA_A_DMA_RSV6*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x15022C28 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_DEBUG_SEL;	/* DIP_X_ADL_DMA_A_DMA_DEBUG_SEL, DIP_A_ADL_DMA_A_DMA_DEBUG_SEL*/

typedef volatile union _DIP_X_REG_ADL_DMA_A_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x15022C2C */
		{
				FIELD  BW_SELF_TEST_EN_IPUO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_A_DMA_BW_SELF_TEST;	/* DIP_X_ADL_DMA_A_DMA_BW_SELF_TEST, DIP_A_ADL_DMA_A_DMA_BW_SELF_TEST*/

typedef volatile union _DIP_X_REG_AMD_RESET_
{
		volatile struct	/* 0x15022D00 */
		{
				FIELD  C24_HW_RST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MDP_CROP_HW_RST                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ADL_HW_RST                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  TOP_CTL_HW_RST                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_RESET;	/* DIP_X_AMD_RESET, DIP_A_AMD_RESET*/

typedef volatile union _DIP_X_REG_AMD_TOP_CTL_
{
		volatile struct	/* 0x15022D04 */
		{
				FIELD  AMD_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  INT_CLR_MODE                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_TOP_CTL;	/* DIP_X_AMD_TOP_CTL, DIP_A_AMD_TOP_CTL*/

typedef volatile union _DIP_X_REG_AMD_INT_EN_
{
		volatile struct	/* 0x15022D08 */
		{
				FIELD  MDP_CROP_INT_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AMD_INT_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_INT_EN;	/* DIP_X_AMD_INT_EN, DIP_A_AMD_INT_EN*/

typedef volatile union _DIP_X_REG_AMD_INT_ST_
{
		volatile struct	/* 0x15022D0C */
		{
				FIELD  MDP_CROP_INT_ST                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AMD_INT_ST                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_INT_ST;	/* DIP_X_AMD_INT_ST, DIP_A_AMD_INT_ST*/

typedef volatile union _DIP_X_REG_AMD_C24_CTL_
{
		volatile struct	/* 0x15022D10 */
		{
				FIELD  C24_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  C24_EN                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_C24_CTL;	/* DIP_X_AMD_C24_CTL, DIP_A_AMD_C24_CTL*/

typedef volatile union _DIP_X_REG_AMD_CROP_CTL_0_
{
		volatile struct	/* 0x15022D14 */
		{
				FIELD  MDP_CROP_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_0;	/* DIP_X_AMD_CROP_CTL_0, DIP_A_AMD_CROP_CTL_0*/

typedef volatile union _DIP_X_REG_AMD_CROP_CTL_1_
{
		volatile struct	/* 0x15022D18 */
		{
				FIELD  MDP_CROP_START_X                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_START_Y                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_1;	/* DIP_X_AMD_CROP_CTL_1, DIP_A_AMD_CROP_CTL_1*/

typedef volatile union _DIP_X_REG_AMD_CROP_CTL_2_
{
		volatile struct	/* 0x15022D1C */
		{
				FIELD  MDP_CROP_END_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_2;	/* DIP_X_AMD_CROP_CTL_2, DIP_A_AMD_CROP_CTL_2*/

typedef volatile union _DIP_X_REG_AMD_DCM_DIS_
{
		volatile struct	/* 0x15022D20 */
		{
				FIELD  C24_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MDP_CROP_DCM_DIS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_DCM_DIS;	/* DIP_X_AMD_DCM_DIS, DIP_A_AMD_DCM_DIS*/

typedef volatile union _DIP_X_REG_AMD_DCM_ST_
{
		volatile struct	/* 0x15022D24 */
		{
				FIELD  C24_DCM_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MDP_CROP_DCM_ST                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_DCM_ST;	/* DIP_X_AMD_DCM_ST, DIP_A_AMD_DCM_ST*/

typedef volatile union _DIP_X_REG_AMD_DEBUG_SEL_
{
		volatile struct	/* 0x15022D30 */
		{
				FIELD  C24_DEBUG_SEL                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  MDP_CROP_DEBUG_SEL                    :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_DEBUG_SEL;	/* DIP_X_AMD_DEBUG_SEL, DIP_A_AMD_DEBUG_SEL*/

typedef volatile union _DIP_X_REG_AMD_C24_DEBUG_
{
		volatile struct	/* 0x15022D34 */
		{
				FIELD  C24_DEBUG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_C24_DEBUG;	/* DIP_X_AMD_C24_DEBUG, DIP_A_AMD_C24_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_CROP_DEBUG_
{
		volatile struct	/* 0x15022D38 */
		{
				FIELD  MDP_CROP_DEBUG                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_DEBUG;	/* DIP_X_AMD_CROP_DEBUG, DIP_A_AMD_CROP_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_ADL_DEBUG_
{
		volatile struct	/* 0x15022D3C */
		{
				FIELD  ADL_DEBUG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_ADL_DEBUG;	/* DIP_X_AMD_ADL_DEBUG, DIP_A_AMD_ADL_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_SOF_
{
		volatile struct	/* 0x15022D40 */
		{
				FIELD  AMD_SOF                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_SOF;	/* DIP_X_AMD_SOF, DIP_A_AMD_SOF*/

typedef volatile union _DIP_X_REG_AMD_REQ_ST_
{
		volatile struct	/* 0x15022D44 */
		{
				FIELD  TILE_IN_0_REQ                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TILE_IN_1_REQ                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TILE_IN_2_REQ                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TILE_IN_3_REQ                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TILE_IN_4_REQ                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  C24_IN_REQ                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CROP_IN_REQ                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  TILE_OUT_0_REQ                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  TILE_OUT_1_REQ                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CROP_OUT_VALID                        :  1;		/* 10..10, 0x00000400 */
				FIELD  TILE_OUT_3_REQ                        :  1;		/* 11..11, 0x00000800 */
				FIELD  TILE_OUT_4_REQ                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_REQ_ST;	/* DIP_X_AMD_REQ_ST, DIP_A_AMD_REQ_ST*/

typedef volatile union _DIP_X_REG_AMD_RDY_ST_
{
		volatile struct	/* 0x15022D48 */
		{
				FIELD  TILE_IN_0_RDY                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TILE_IN_1_RDY                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TILE_IN_2_RDY                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TILE_IN_3_RDY                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TILE_IN_4_RDY                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  C24_IN_RDY                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CROP_IN_RDY                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  TILE_OUT_0_RDY                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  TILE_OUT_1_RDY                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CROP_OUT_READY                        :  1;		/* 10..10, 0x00000400 */
				FIELD  TILE_OUT_3_RDY                        :  1;		/* 11..11, 0x00000800 */
				FIELD  TILE_OUT_4_RDY                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_RDY_ST;	/* DIP_X_AMD_RDY_ST, DIP_A_AMD_RDY_ST*/

typedef volatile union _DIP_X_REG_AMD_SPARE_
{
		volatile struct	/* 0x15022D70 */
		{
				FIELD  AMD_SPARE                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_SPARE;	/* DIP_X_AMD_SPARE, DIP_A_AMD_SPARE*/

typedef volatile union _DIP_X_REG_ADL_RESET_
{
		volatile struct	/* 0x15022E00 */
		{
				FIELD  DMA_0_SOFT_RST_ST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  DMA_0_SOFT_RST                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  RDMA_0_HW_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  WDMA_0_HW_RST                         :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  AHB2GMC_HW_RST                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_RESET;	/* DIP_X_ADL_RESET, DIP_A_ADL_RESET*/

typedef volatile union _DIP_X_REG_ADL_CTL_
{
		volatile struct	/* 0x15022E04 */
		{
				FIELD  SRC_SEL                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DST_SEL                               :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SRC_TYPE                              :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DST_TYPE                              :  2;		/* 10..11, 0x00000C00 */
				FIELD  H2G_GID                               :  2;		/* 12..13, 0x00003000 */
				FIELD  H2G_GULTRA_ENABLE                     :  1;		/* 14..14, 0x00004000 */
				FIELD  H2G_EARLY_RESP                        :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  ENABLE                                :  2;		/* 20..21, 0x00300000 */
				FIELD  DONE_DIS                              :  3;		/* 22..24, 0x01C00000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_CTL;	/* DIP_X_ADL_CTL, DIP_A_ADL_CTL*/

typedef volatile union _DIP_X_REG_ADL_CROP_IN_START_
{
		volatile struct	/* 0x15022E30 */
		{
				FIELD  CROP_IN_START_X                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CROP_IN_START_Y                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_CROP_IN_START;	/* DIP_X_ADL_CROP_IN_START, DIP_A_ADL_CROP_IN_START*/

typedef volatile union _DIP_X_REG_ADL_CROP_IN_END_
{
		volatile struct	/* 0x15022E34 */
		{
				FIELD  CROP_IN_END_X                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CROP_IN_END_Y                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_CROP_IN_END;	/* DIP_X_ADL_CROP_IN_END, DIP_A_ADL_CROP_IN_END*/

typedef volatile union _DIP_X_REG_ADL_CROP_OUT_START_
{
		volatile struct	/* 0x15022E38 */
		{
				FIELD  CROP_OUT_START_X                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CROP_OUT_START_Y                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_CROP_OUT_START;	/* DIP_X_ADL_CROP_OUT_START, DIP_A_ADL_CROP_OUT_START*/

typedef volatile union _DIP_X_REG_ADL_CROP_OUT_END_
{
		volatile struct	/* 0x15022E3C */
		{
				FIELD  CROP_OUT_END_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CROP_OUT_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_CROP_OUT_END;	/* DIP_X_ADL_CROP_OUT_END, DIP_A_ADL_CROP_OUT_END*/

typedef volatile union _DIP_X_REG_ADL_DMA_ST_
{
		volatile struct	/* 0x15022E40 */
		{
				FIELD  RDMA_0_DONE_ST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  WDMA_0_DONE_ST                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RDMA_1_DONE_ST                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  WDMA_1_DONE_ST                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_ST;	/* DIP_X_ADL_DMA_ST, DIP_A_ADL_DMA_ST*/

typedef volatile union _DIP_X_REG_ADL_DCM_DIS_
{
		volatile struct	/* 0x15022E44 */
		{
				FIELD  RDMA_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WDMA_DCM_DIS                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SRAM_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  H2G_CG_DIS                            :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DCM_DIS;	/* DIP_X_ADL_DCM_DIS, DIP_A_ADL_DCM_DIS*/

typedef volatile union _DIP_X_REG_ADL_DCM_ST_
{
		volatile struct	/* 0x15022E48 */
		{
				FIELD  RDMA_0_DCM_ST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WDMA_0_DCM_ST                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RDMA_1_DCM_ST                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  WDMA_1_DCM_ST                         :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  DMA_0_SRAM_DCM_ST                     :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  DMA_1_SRAM_DCM_ST                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DCM_ST;	/* DIP_X_ADL_DCM_ST, DIP_A_ADL_DCM_ST*/

typedef volatile union _DIP_X_REG_ADL_DMA_ERR_ST_
{
		volatile struct	/* 0x15022E4C */
		{
				FIELD  DMA_0_ERR_INT                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DMA_0_REQ_ST                          :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  DMA_0_RDY_ST                          :  5;		/*  9..13, 0x00003E00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  DMA_1_ERR_INT                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  DMA_1_REQ_ST                          :  5;		/* 20..24, 0x01F00000 */
				FIELD  DMA_1_RDY_ST                          :  5;		/* 25..29, 0x3E000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_ERR_ST;	/* DIP_X_ADL_DMA_ERR_ST, DIP_A_ADL_DMA_ERR_ST*/

typedef volatile union _DIP_X_REG_ADL_DMA_0_DEBUG_
{
		volatile struct	/* 0x15022E50 */
		{
				FIELD  DMA_0_DEBUG                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_0_DEBUG;	/* DIP_X_ADL_DMA_0_DEBUG, DIP_A_ADL_DMA_0_DEBUG*/

typedef volatile union _DIP_X_REG_ADL_DMA_1_DEBUG_
{
		volatile struct	/* 0x15022E54 */
		{
				FIELD  DMA_1_DEBUG                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_DMA_1_DEBUG;	/* DIP_X_ADL_DMA_1_DEBUG, DIP_A_ADL_DMA_1_DEBUG*/

typedef volatile union _DIP_X_REG_ADL_SPARE_
{
		volatile struct	/* 0x15022E70 */
		{
				FIELD  SPARE                                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_SPARE;	/* DIP_X_ADL_SPARE, DIP_A_ADL_SPARE*/

typedef volatile union _DIP_X_REG_ADL_INFO00_
{
		volatile struct	/* 0x15022E80 */
		{
				FIELD  ADL_A_WR_RING_XSIZE                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ADL_A_WR_RING_YSIZE                   : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO00;	/* DIP_X_ADL_INFO00, DIP_A_ADL_INFO00*/

typedef volatile union _DIP_X_REG_ADL_INFO01_
{
		volatile struct	/* 0x15022E84 */
		{
				FIELD  ADL_A_WR_RING_STRIDE                  : 16;		/*  0..15, 0x0000FFFF */
				FIELD  Reserved                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO01;	/* DIP_X_ADL_INFO01, DIP_A_ADL_INFO01*/

typedef volatile union _DIP_X_REG_ADL_INFO02_
{
		volatile struct	/* 0x15022E88 */
		{
				FIELD  ADL_A_RD_RING_XSIZE                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ADL_A_RD_RING_YSIZE                   : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO02;	/* DIP_X_ADL_INFO02, DIP_A_ADL_INFO02*/

typedef volatile union _DIP_X_REG_ADL_INFO03_
{
		volatile struct	/* 0x15022E8C */
		{
				FIELD  ADL_A_RD_RING_STRIDE                  : 16;		/*  0..15, 0x0000FFFF */
				FIELD  Reserved                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO03;	/* DIP_X_ADL_INFO03, DIP_A_ADL_INFO03*/

typedef volatile union _DIP_X_REG_ADL_INFO04_
{
		volatile struct	/* 0x15022E90 */
		{
				FIELD  INFO04                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO04;	/* DIP_X_ADL_INFO04, DIP_A_ADL_INFO04*/

typedef volatile union _DIP_X_REG_ADL_INFO05_
{
		volatile struct	/* 0x15022E94 */
		{
				FIELD  INFO05                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO05;	/* DIP_X_ADL_INFO05, DIP_A_ADL_INFO05*/

typedef volatile union _DIP_X_REG_ADL_INFO06_
{
		volatile struct	/* 0x15022E98 */
		{
				FIELD  INFO06                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO06;	/* DIP_X_ADL_INFO06, DIP_A_ADL_INFO06*/

typedef volatile union _DIP_X_REG_ADL_INFO07_
{
		volatile struct	/* 0x15022E9C */
		{
				FIELD  INFO07                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO07;	/* DIP_X_ADL_INFO07, DIP_A_ADL_INFO07*/

typedef volatile union _DIP_X_REG_ADL_INFO08_
{
		volatile struct	/* 0x15022EA0 */
		{
				FIELD  INFO08                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO08;	/* DIP_X_ADL_INFO08, DIP_A_ADL_INFO08*/

typedef volatile union _DIP_X_REG_ADL_INFO09_
{
		volatile struct	/* 0x15022EA4 */
		{
				FIELD  INFO09                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO09;	/* DIP_X_ADL_INFO09, DIP_A_ADL_INFO09*/

typedef volatile union _DIP_X_REG_ADL_INFO10_
{
		volatile struct	/* 0x15022EA8 */
		{
				FIELD  INFO10                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO10;	/* DIP_X_ADL_INFO10, DIP_A_ADL_INFO10*/

typedef volatile union _DIP_X_REG_ADL_INFO11_
{
		volatile struct	/* 0x15022EAC */
		{
				FIELD  INFO11                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO11;	/* DIP_X_ADL_INFO11, DIP_A_ADL_INFO11*/

typedef volatile union _DIP_X_REG_ADL_INFO12_
{
		volatile struct	/* 0x15022EB0 */
		{
				FIELD  INFO12                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO12;	/* DIP_X_ADL_INFO12, DIP_A_ADL_INFO12*/

typedef volatile union _DIP_X_REG_ADL_INFO13_
{
		volatile struct	/* 0x15022EB4 */
		{
				FIELD  INFO13                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO13;	/* DIP_X_ADL_INFO13, DIP_A_ADL_INFO13*/

typedef volatile union _DIP_X_REG_ADL_INFO14_
{
		volatile struct	/* 0x15022EB8 */
		{
				FIELD  INFO14                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO14;	/* DIP_X_ADL_INFO14, DIP_A_ADL_INFO14*/

typedef volatile union _DIP_X_REG_ADL_INFO15_
{
		volatile struct	/* 0x15022EBC */
		{
				FIELD  INFO15                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO15;	/* DIP_X_ADL_INFO15, DIP_A_ADL_INFO15*/

typedef volatile union _DIP_X_REG_ADL_INFO16_
{
		volatile struct	/* 0x15022EC0 */
		{
				FIELD  INFO16                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO16;	/* DIP_X_ADL_INFO16, DIP_A_ADL_INFO16*/

typedef volatile union _DIP_X_REG_ADL_INFO17_
{
		volatile struct	/* 0x15022EC4 */
		{
				FIELD  INFO17                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO17;	/* DIP_X_ADL_INFO17, DIP_A_ADL_INFO17*/

typedef volatile union _DIP_X_REG_ADL_INFO18_
{
		volatile struct	/* 0x15022EC8 */
		{
				FIELD  INFO18                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO18;	/* DIP_X_ADL_INFO18, DIP_A_ADL_INFO18*/

typedef volatile union _DIP_X_REG_ADL_INFO19_
{
		volatile struct	/* 0x15022ECC */
		{
				FIELD  INFO19                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO19;	/* DIP_X_ADL_INFO19, DIP_A_ADL_INFO19*/

typedef volatile union _DIP_X_REG_ADL_INFO20_
{
		volatile struct	/* 0x15022ED0 */
		{
				FIELD  INFO20                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO20;	/* DIP_X_ADL_INFO20, DIP_A_ADL_INFO20*/

typedef volatile union _DIP_X_REG_ADL_INFO21_
{
		volatile struct	/* 0x15022ED4 */
		{
				FIELD  INFO21                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO21;	/* DIP_X_ADL_INFO21, DIP_A_ADL_INFO21*/

typedef volatile union _DIP_X_REG_ADL_INFO22_
{
		volatile struct	/* 0x15022ED8 */
		{
				FIELD  INFO22                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO22;	/* DIP_X_ADL_INFO22, DIP_A_ADL_INFO22*/

typedef volatile union _DIP_X_REG_ADL_INFO23_
{
		volatile struct	/* 0x15022EDC */
		{
				FIELD  INFO23                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO23;	/* DIP_X_ADL_INFO23, DIP_A_ADL_INFO23*/

typedef volatile union _DIP_X_REG_ADL_INFO24_
{
		volatile struct	/* 0x15022EE0 */
		{
				FIELD  INFO24                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO24;	/* DIP_X_ADL_INFO24, DIP_A_ADL_INFO24*/

typedef volatile union _DIP_X_REG_ADL_INFO25_
{
		volatile struct	/* 0x15022EE4 */
		{
				FIELD  INFO25                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO25;	/* DIP_X_ADL_INFO25, DIP_A_ADL_INFO25*/

typedef volatile union _DIP_X_REG_ADL_INFO26_
{
		volatile struct	/* 0x15022EE8 */
		{
				FIELD  INFO26                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO26;	/* DIP_X_ADL_INFO26, DIP_A_ADL_INFO26*/

typedef volatile union _DIP_X_REG_ADL_INFO27_
{
		volatile struct	/* 0x15022EEC */
		{
				FIELD  INFO27                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO27;	/* DIP_X_ADL_INFO27, DIP_A_ADL_INFO27*/

typedef volatile union _DIP_X_REG_ADL_INFO28_
{
		volatile struct	/* 0x15022EF0 */
		{
				FIELD  INFO28                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO28;	/* DIP_X_ADL_INFO28, DIP_A_ADL_INFO28*/

typedef volatile union _DIP_X_REG_ADL_INFO29_
{
		volatile struct	/* 0x15022EF4 */
		{
				FIELD  INFO29                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO29;	/* DIP_X_ADL_INFO29, DIP_A_ADL_INFO29*/

typedef volatile union _DIP_X_REG_ADL_INFO30_
{
		volatile struct	/* 0x15022EF8 */
		{
				FIELD  INFO30                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO30;	/* DIP_X_ADL_INFO30, DIP_A_ADL_INFO30*/

typedef volatile union _DIP_X_REG_ADL_INFO31_
{
		volatile struct	/* 0x15022EFC */
		{
				FIELD  INFO31                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADL_INFO31;	/* DIP_X_ADL_INFO31, DIP_A_ADL_INFO31*/

typedef volatile union _DIP_X_REG_UNP_OFST_
{
		volatile struct	/* 0x15023000 */
		{
				FIELD  UNP_OFST_STB                          :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  UNP_OFST_EDB                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UNP_OFST;	/* DIP_X_UNP_OFST, DIP_A_UNP_OFST*/

typedef volatile union _DIP_X_REG_UFDG_CON_
{
		volatile struct	/* 0x15023040 */
		{
				FIELD  UFDG_TCCT_BYP                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFOD_SEL                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  7;		/*  2.. 8, 0x000001FC */
				FIELD  UFDG_BOND_MODE                        :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_CON;	/* DIP_X_UFDG_CON, DIP_A_UFDG_CON*/

typedef volatile union _DIP_X_REG_UFDG_SIZE_CON_
{
		volatile struct	/* 0x15023044 */
		{
				FIELD  UFDG_WD                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFDG_HT                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_SIZE_CON;	/* DIP_X_UFDG_SIZE_CON, DIP_A_UFDG_SIZE_CON*/

typedef volatile union _DIP_X_REG_UFDG_AU_CON_
{
		volatile struct	/* 0x15023048 */
		{
				FIELD  UFDG_AU_OFST                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UFDG_AU_SIZE                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_AU_CON;	/* DIP_X_UFDG_AU_CON, DIP_A_UFDG_AU_CON*/

typedef volatile union _DIP_X_REG_UFDG_CROP_CON1_
{
		volatile struct	/* 0x1502304C */
		{
				FIELD  UFDG_X_START                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFDG_X_END                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_CROP_CON1;	/* DIP_X_UFDG_CROP_CON1, DIP_A_UFDG_CROP_CON1*/

typedef volatile union _DIP_X_REG_UFDG_CROP_CON2_
{
		volatile struct	/* 0x15023050 */
		{
				FIELD  UFDG_Y_START                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFDG_Y_END                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_CROP_CON2;	/* DIP_X_UFDG_CROP_CON2, DIP_A_UFDG_CROP_CON2*/

typedef volatile union _DIP_X_REG_UFDG_AU2_CON_
{
		volatile struct	/* 0x15023054 */
		{
				FIELD  UFDG_AU2_OFST                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UFDG_AU2_SIZE                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_AU2_CON;	/* DIP_X_UFDG_AU2_CON, DIP_A_UFDG_AU2_CON*/

typedef volatile union _DIP_X_REG_UFDG_ADDRESS_CON_
{
		volatile struct	/* 0x15023058 */
		{
				FIELD  UFDG_BITSTREAM_OFST_ADDR              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_ADDRESS_CON;	/* DIP_X_UFDG_ADDRESS_CON, DIP_A_UFDG_ADDRESS_CON*/

typedef volatile union _DIP_X_REG_UFDG_BS_AU_CON_
{
		volatile struct	/* 0x1502305C */
		{
				FIELD  UFDG_BS_AU_START                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDG_BS_AU_CON;	/* DIP_X_UFDG_BS_AU_CON, DIP_A_UFDG_BS_AU_CON*/

typedef volatile union _DIP_X_REG_SMX1_CTL_
{
		volatile struct	/* 0x15023100 */
		{
				FIELD  SMX_RIGH_DISABLE                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SMX_LEFT_DISABLE                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SMX_CRPOUT_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SMX_CRPINR_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SMX_CRPINL_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SMX_TRANS_UP_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SMX_TRANS_DOWN_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMXO_SEL                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CTL;	/* DIP_X_SMX1_CTL, DIP_A_SMX1_CTL*/

typedef volatile union _DIP_X_REG_SMX1_TRANS_CON_
{
		volatile struct	/* 0x15023104 */
		{
				FIELD  SMX_TRANS_PX_NUM                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SMX_TRANS_LN_NUM                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_TRANS_CON;	/* DIP_X_SMX1_TRANS_CON, DIP_A_SMX1_TRANS_CON*/

typedef volatile union _DIP_X_REG_SMX1_SPARE_
{
		volatile struct	/* 0x15023108 */
		{
				FIELD  SMX_SPARE                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_SPARE;	/* DIP_X_SMX1_SPARE, DIP_A_SMX1_SPARE*/

typedef volatile union _DIP_X_REG_SMX1_CRPINL_CON1_
{
		volatile struct	/* 0x1502310C */
		{
				FIELD  SMX_CRPINL_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPINL_CON1;	/* DIP_X_SMX1_CRPINL_CON1, DIP_A_SMX1_CRPINL_CON1*/

typedef volatile union _DIP_X_REG_SMX1_CRPINL_CON2_
{
		volatile struct	/* 0x15023110 */
		{
				FIELD  SMX_CRPINL_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPINL_CON2;	/* DIP_X_SMX1_CRPINL_CON2, DIP_A_SMX1_CRPINL_CON2*/

typedef volatile union _DIP_X_REG_SMX1_CRPINR_CON1_
{
		volatile struct	/* 0x15023114 */
		{
				FIELD  SMX_CRPINR_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPINR_CON1;	/* DIP_X_SMX1_CRPINR_CON1, DIP_A_SMX1_CRPINR_CON1*/

typedef volatile union _DIP_X_REG_SMX1_CRPINR_CON2_
{
		volatile struct	/* 0x15023118 */
		{
				FIELD  SMX_CRPINR_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPINR_CON2;	/* DIP_X_SMX1_CRPINR_CON2, DIP_A_SMX1_CRPINR_CON2*/

typedef volatile union _DIP_X_REG_SMX1_CRPOUT_CON1_
{
		volatile struct	/* 0x1502311C */
		{
				FIELD  SMX_CRPOUT_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPOUT_CON1;	/* DIP_X_SMX1_CRPOUT_CON1, DIP_A_SMX1_CRPOUT_CON1*/

typedef volatile union _DIP_X_REG_SMX1_CRPOUT_CON2_
{
		volatile struct	/* 0x15023120 */
		{
				FIELD  SMX_CRPOUT_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX1_CRPOUT_CON2;	/* DIP_X_SMX1_CRPOUT_CON2, DIP_A_SMX1_CRPOUT_CON2*/

typedef volatile union _DIP_X_REG_DBS2_SIGMA_
{
		volatile struct	/* 0x15023140 */
		{
				FIELD  DBS_OFST                              : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_SL                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SIGMA;	/* DIP_X_DBS2_SIGMA, DIP_A_DBS2_SIGMA*/

typedef volatile union _DIP_X_REG_DBS2_BSTBL_0_
{
		volatile struct	/* 0x15023144 */
		{
				FIELD  DBS_BIAS_Y0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_BSTBL_0;	/* DIP_X_DBS2_BSTBL_0, DIP_A_DBS2_BSTBL_0*/

typedef volatile union _DIP_X_REG_DBS2_BSTBL_1_
{
		volatile struct	/* 0x15023148 */
		{
				FIELD  DBS_BIAS_Y4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_BSTBL_1;	/* DIP_X_DBS2_BSTBL_1, DIP_A_DBS2_BSTBL_1*/

typedef volatile union _DIP_X_REG_DBS2_BSTBL_2_
{
		volatile struct	/* 0x1502314C */
		{
				FIELD  DBS_BIAS_Y8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_BSTBL_2;	/* DIP_X_DBS2_BSTBL_2, DIP_A_DBS2_BSTBL_2*/

typedef volatile union _DIP_X_REG_DBS2_BSTBL_3_
{
		volatile struct	/* 0x15023150 */
		{
				FIELD  DBS_BIAS_Y12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_BSTBL_3;	/* DIP_X_DBS2_BSTBL_3, DIP_A_DBS2_BSTBL_3*/

typedef volatile union _DIP_X_REG_DBS2_CTL_
{
		volatile struct	/* 0x15023154 */
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
}DIP_X_REG_DBS2_CTL;	/* DIP_X_DBS2_CTL, DIP_A_DBS2_CTL*/

typedef volatile union _DIP_X_REG_DBS2_CTL_2_
{
		volatile struct	/* 0x15023158 */
		{
				FIELD  DBS_HDR_OSCTH                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_CTL_2;	/* DIP_X_DBS2_CTL_2, DIP_A_DBS2_CTL_2*/

typedef volatile union _DIP_X_REG_DBS2_SIGMA_2_
{
		volatile struct	/* 0x1502315C */
		{
				FIELD  DBS_MUL_B                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_MUL_GB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_MUL_GR                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_MUL_R                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SIGMA_2;	/* DIP_X_DBS2_SIGMA_2, DIP_A_DBS2_SIGMA_2*/

typedef volatile union _DIP_X_REG_DBS2_YGN_
{
		volatile struct	/* 0x15023160 */
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
}DIP_X_REG_DBS2_YGN;	/* DIP_X_DBS2_YGN, DIP_A_DBS2_YGN*/

typedef volatile union _DIP_X_REG_DBS2_SL_Y12_
{
		volatile struct	/* 0x15023164 */
		{
				FIELD  DBS_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SL_Y12;	/* DIP_X_DBS2_SL_Y12, DIP_A_DBS2_SL_Y12*/

typedef volatile union _DIP_X_REG_DBS2_SL_Y34_
{
		volatile struct	/* 0x15023168 */
		{
				FIELD  DBS_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SL_Y34;	/* DIP_X_DBS2_SL_Y34, DIP_A_DBS2_SL_Y34*/

typedef volatile union _DIP_X_REG_DBS2_SL_G12_
{
		volatile struct	/* 0x1502316C */
		{
				FIELD  DBS_SL_G1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SL_G12;	/* DIP_X_DBS2_SL_G12, DIP_A_DBS2_SL_G12*/

typedef volatile union _DIP_X_REG_DBS2_SL_G34_
{
		volatile struct	/* 0x15023170 */
		{
				FIELD  DBS_SL_G3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SL_G34;	/* DIP_X_DBS2_SL_G34, DIP_A_DBS2_SL_G34*/

typedef volatile union _DIP_X_REG_SL2G_CEN_
{
		volatile struct	/* 0x15023180 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_CEN;	/* DIP_X_SL2G_CEN, DIP_A_SL2G_CEN*/

typedef volatile union _DIP_X_REG_SL2G_RR_CON0_
{
		volatile struct	/* 0x15023184 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_RR_CON0;	/* DIP_X_SL2G_RR_CON0, DIP_A_SL2G_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2G_RR_CON1_
{
		volatile struct	/* 0x15023188 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_RR_CON1;	/* DIP_X_SL2G_RR_CON1, DIP_A_SL2G_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2G_GAIN_
{
		volatile struct	/* 0x1502318C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_GAIN;	/* DIP_X_SL2G_GAIN, DIP_A_SL2G_GAIN*/

typedef volatile union _DIP_X_REG_SL2G_RZ_
{
		volatile struct	/* 0x15023190 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_RZ;	/* DIP_X_SL2G_RZ, DIP_A_SL2G_RZ*/

typedef volatile union _DIP_X_REG_SL2G_XOFF_
{
		volatile struct	/* 0x15023194 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_XOFF;	/* DIP_X_SL2G_XOFF, DIP_A_SL2G_XOFF*/

typedef volatile union _DIP_X_REG_SL2G_YOFF_
{
		volatile struct	/* 0x15023198 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_YOFF;	/* DIP_X_SL2G_YOFF, DIP_A_SL2G_YOFF*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON0_
{
		volatile struct	/* 0x1502319C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON0;	/* DIP_X_SL2G_SLP_CON0, DIP_A_SL2G_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON1_
{
		volatile struct	/* 0x150231A0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON1;	/* DIP_X_SL2G_SLP_CON1, DIP_A_SL2G_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON2_
{
		volatile struct	/* 0x150231A4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON2;	/* DIP_X_SL2G_SLP_CON2, DIP_A_SL2G_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON3_
{
		volatile struct	/* 0x150231A8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON3;	/* DIP_X_SL2G_SLP_CON3, DIP_A_SL2G_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2G_SIZE_
{
		volatile struct	/* 0x150231AC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SIZE;	/* DIP_X_SL2G_SIZE, DIP_A_SL2G_SIZE*/

typedef volatile union _DIP_X_REG_OBC2_OFFST0_
{
		volatile struct	/* 0x150231C0 */
		{
				FIELD  OBC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST0;	/* DIP_X_OBC2_OFFST0, DIP_A_OBC2_OFFST0*/

typedef volatile union _DIP_X_REG_OBC2_OFFST1_
{
		volatile struct	/* 0x150231C4 */
		{
				FIELD  OBC_OFST_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST1;	/* DIP_X_OBC2_OFFST1, DIP_A_OBC2_OFFST1*/

typedef volatile union _DIP_X_REG_OBC2_OFFST2_
{
		volatile struct	/* 0x150231C8 */
		{
				FIELD  OBC_OFST_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST2;	/* DIP_X_OBC2_OFFST2, DIP_A_OBC2_OFFST2*/

typedef volatile union _DIP_X_REG_OBC2_OFFST3_
{
		volatile struct	/* 0x150231CC */
		{
				FIELD  OBC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST3;	/* DIP_X_OBC2_OFFST3, DIP_A_OBC2_OFFST3*/

typedef volatile union _DIP_X_REG_OBC2_GAIN0_
{
		volatile struct	/* 0x150231D0 */
		{
				FIELD  OBC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN0;	/* DIP_X_OBC2_GAIN0, DIP_A_OBC2_GAIN0*/

typedef volatile union _DIP_X_REG_OBC2_GAIN1_
{
		volatile struct	/* 0x150231D4 */
		{
				FIELD  OBC_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN1;	/* DIP_X_OBC2_GAIN1, DIP_A_OBC2_GAIN1*/

typedef volatile union _DIP_X_REG_OBC2_GAIN2_
{
		volatile struct	/* 0x150231D8 */
		{
				FIELD  OBC_GAIN_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN2;	/* DIP_X_OBC2_GAIN2, DIP_A_OBC2_GAIN2*/

typedef volatile union _DIP_X_REG_OBC2_GAIN3_
{
		volatile struct	/* 0x150231DC */
		{
				FIELD  OBC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN3;	/* DIP_X_OBC2_GAIN3, DIP_A_OBC2_GAIN3*/

typedef volatile union _DIP_X_REG_RMG2_HDR_CFG_
{
		volatile struct	/* 0x15023200 */
		{
				FIELD  RMG_IHDR_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RMG_IHDR_LE_FIRST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RMG_ZHDR_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RMG_ZHDR_RLE                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RMG_ZHDR_GLE                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RMG_ZHDR_BLE                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RMG_HDR_TH                            : 12;		/*  8..19, 0x000FFF00 */
				FIELD  RMG_OSC_TH                            : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMG2_HDR_CFG;	/* DIP_X_RMG2_HDR_CFG, DIP_A_RMG2_HDR_CFG*/

typedef volatile union _DIP_X_REG_RMG2_HDR_GAIN_
{
		volatile struct	/* 0x15023204 */
		{
				FIELD  RMG_HDR_GAIN                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  RMG_HDR_RATIO                         :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  RMG_LE_INV_CTL                        :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMG2_HDR_GAIN;	/* DIP_X_RMG2_HDR_GAIN, DIP_A_RMG2_HDR_GAIN*/

typedef volatile union _DIP_X_REG_RMG2_HDR_CFG2_
{
		volatile struct	/* 0x15023208 */
		{
				FIELD  RMG_HDR_THK                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMG2_HDR_CFG2;	/* DIP_X_RMG2_HDR_CFG2, DIP_A_RMG2_HDR_CFG2*/

typedef volatile union _DIP_X_REG_BNR2_BPC_CON_
{
		volatile struct	/* 0x15023240 */
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
}DIP_X_REG_BNR2_BPC_CON;	/* DIP_X_BNR2_BPC_CON, DIP_A_BNR2_BPC_CON*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH1_
{
		volatile struct	/* 0x15023244 */
		{
				FIELD  BPC_TH_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_Y                              : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_BLD_SLP0                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH1;	/* DIP_X_BNR2_BPC_TH1, DIP_A_BNR2_BPC_TH1*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH2_
{
		volatile struct	/* 0x15023248 */
		{
				FIELD  BPC_TH_UPB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_BLD0                              :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_BLD1                              :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH2;	/* DIP_X_BNR2_BPC_TH2, DIP_A_BNR2_BPC_TH2*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH3_
{
		volatile struct	/* 0x1502324C */
		{
				FIELD  BPC_TH_XA                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_XB                             : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_TH_SLA                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_TH_SLB                            :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH3;	/* DIP_X_BNR2_BPC_TH3, DIP_A_BNR2_BPC_TH3*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH4_
{
		volatile struct	/* 0x15023250 */
		{
				FIELD  BPC_DK_TH_XA                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_DK_TH_XB                          : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_DK_TH_SLA                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_DK_TH_SLB                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH4;	/* DIP_X_BNR2_BPC_TH4, DIP_A_BNR2_BPC_TH4*/

typedef volatile union _DIP_X_REG_BNR2_BPC_DTC_
{
		volatile struct	/* 0x15023254 */
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
}DIP_X_REG_BNR2_BPC_DTC;	/* DIP_X_BNR2_BPC_DTC, DIP_A_BNR2_BPC_DTC*/

typedef volatile union _DIP_X_REG_BNR2_BPC_COR_
{
		volatile struct	/* 0x15023258 */
		{
				FIELD  BPC_DIR_MAX                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BPC_DIR_TH                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BPC_RANK_IDXR                         :  3;		/* 16..18, 0x00070000 */
				FIELD  BPC_RANK_IDXG                         :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPC_DIR_TH2                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_COR;	/* DIP_X_BNR2_BPC_COR, DIP_A_BNR2_BPC_COR*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TBLI1_
{
		volatile struct	/* 0x1502325C */
		{
				FIELD  BPC_XOFFSET                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YOFFSET                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TBLI1;	/* DIP_X_BNR2_BPC_TBLI1, DIP_A_BNR2_BPC_TBLI1*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TBLI2_
{
		volatile struct	/* 0x15023260 */
		{
				FIELD  BPC_XSIZE                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YSIZE                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TBLI2;	/* DIP_X_BNR2_BPC_TBLI2, DIP_A_BNR2_BPC_TBLI2*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH1_C_
{
		volatile struct	/* 0x15023264 */
		{
				FIELD  BPC_C_TH_LWB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_Y                            : 12;		/* 12..23, 0x00FFF000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH1_C;	/* DIP_X_BNR2_BPC_TH1_C, DIP_A_BNR2_BPC_TH1_C*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH2_C_
{
		volatile struct	/* 0x15023268 */
		{
				FIELD  BPC_C_TH_UPB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  BPC_RANK_IDXB                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  BPC_BLD_LWB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH2_C;	/* DIP_X_BNR2_BPC_TH2_C, DIP_A_BNR2_BPC_TH2_C*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH3_C_
{
		volatile struct	/* 0x1502326C */
		{
				FIELD  BPC_C_TH_XA                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_XB                           : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_C_TH_SLA                          :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_C_TH_SLB                          :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH3_C;	/* DIP_X_BNR2_BPC_TH3_C, DIP_A_BNR2_BPC_TH3_C*/

typedef volatile union _DIP_X_REG_BNR2_NR1_CON_
{
		volatile struct	/* 0x15023270 */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR1_CT_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_NR1_CON;	/* DIP_X_BNR2_NR1_CON, DIP_A_BNR2_NR1_CON*/

typedef volatile union _DIP_X_REG_BNR2_NR1_CT_CON_
{
		volatile struct	/* 0x15023274 */
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
}DIP_X_REG_BNR2_NR1_CT_CON;	/* DIP_X_BNR2_NR1_CT_CON, DIP_A_BNR2_NR1_CT_CON*/

typedef volatile union _DIP_X_REG_BNR2_NR1_CT_CON2_
{
		volatile struct	/* 0x15023278 */
		{
				FIELD  NR1_CT_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  NR1_CT_UPB                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_NR1_CT_CON2;	/* DIP_X_BNR2_NR1_CT_CON2, DIP_A_BNR2_NR1_CT_CON2*/

typedef volatile union _DIP_X_REG_BNR2_NR1_CT_CON3_
{
		volatile struct	/* 0x1502327C */
		{
				FIELD  NR1_CT_LSP                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NR1_CT_USP                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_NR1_CT_CON3;	/* DIP_X_BNR2_NR1_CT_CON3, DIP_A_BNR2_NR1_CT_CON3*/

typedef volatile union _DIP_X_REG_BNR2_PDC_CON_
{
		volatile struct	/* 0x15023280 */
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
}DIP_X_REG_BNR2_PDC_CON;	/* DIP_X_BNR2_PDC_CON, DIP_A_BNR2_PDC_CON*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_L0_
{
		volatile struct	/* 0x15023284 */
		{
				FIELD  PDC_GCF_L00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_L0;	/* DIP_X_BNR2_PDC_GAIN_L0, DIP_A_BNR2_PDC_GAIN_L0*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_L1_
{
		volatile struct	/* 0x15023288 */
		{
				FIELD  PDC_GCF_L01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_L1;	/* DIP_X_BNR2_PDC_GAIN_L1, DIP_A_BNR2_PDC_GAIN_L1*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_L2_
{
		volatile struct	/* 0x1502328C */
		{
				FIELD  PDC_GCF_L11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_L2;	/* DIP_X_BNR2_PDC_GAIN_L2, DIP_A_BNR2_PDC_GAIN_L2*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_L3_
{
		volatile struct	/* 0x15023290 */
		{
				FIELD  PDC_GCF_L30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_L3;	/* DIP_X_BNR2_PDC_GAIN_L3, DIP_A_BNR2_PDC_GAIN_L3*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_L4_
{
		volatile struct	/* 0x15023294 */
		{
				FIELD  PDC_GCF_L12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_L4;	/* DIP_X_BNR2_PDC_GAIN_L4, DIP_A_BNR2_PDC_GAIN_L4*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_R0_
{
		volatile struct	/* 0x15023298 */
		{
				FIELD  PDC_GCF_R00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_R0;	/* DIP_X_BNR2_PDC_GAIN_R0, DIP_A_BNR2_PDC_GAIN_R0*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_R1_
{
		volatile struct	/* 0x1502329C */
		{
				FIELD  PDC_GCF_R01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_R1;	/* DIP_X_BNR2_PDC_GAIN_R1, DIP_A_BNR2_PDC_GAIN_R1*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_R2_
{
		volatile struct	/* 0x150232A0 */
		{
				FIELD  PDC_GCF_R11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_R2;	/* DIP_X_BNR2_PDC_GAIN_R2, DIP_A_BNR2_PDC_GAIN_R2*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_R3_
{
		volatile struct	/* 0x150232A4 */
		{
				FIELD  PDC_GCF_R30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_R3;	/* DIP_X_BNR2_PDC_GAIN_R3, DIP_A_BNR2_PDC_GAIN_R3*/

typedef volatile union _DIP_X_REG_BNR2_PDC_GAIN_R4_
{
		volatile struct	/* 0x150232A8 */
		{
				FIELD  PDC_GCF_R12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_GAIN_R4;	/* DIP_X_BNR2_PDC_GAIN_R4, DIP_A_BNR2_PDC_GAIN_R4*/

typedef volatile union _DIP_X_REG_BNR2_PDC_TH_GB_
{
		volatile struct	/* 0x150232AC */
		{
				FIELD  PDC_GTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_BTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_TH_GB;	/* DIP_X_BNR2_PDC_TH_GB, DIP_A_BNR2_PDC_TH_GB*/

typedef volatile union _DIP_X_REG_BNR2_PDC_TH_IA_
{
		volatile struct	/* 0x150232B0 */
		{
				FIELD  PDC_ITH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_ATH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_TH_IA;	/* DIP_X_BNR2_PDC_TH_IA, DIP_A_BNR2_PDC_TH_IA*/

typedef volatile union _DIP_X_REG_BNR2_PDC_TH_HD_
{
		volatile struct	/* 0x150232B4 */
		{
				FIELD  PDC_NTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_DTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_TH_HD;	/* DIP_X_BNR2_PDC_TH_HD, DIP_A_BNR2_PDC_TH_HD*/

typedef volatile union _DIP_X_REG_BNR2_PDC_SL_
{
		volatile struct	/* 0x150232B8 */
		{
				FIELD  PDC_GSL                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  PDC_BSL                               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  PDC_ISL                               :  4;		/*  8..11, 0x00000F00 */
				FIELD  PDC_ASL                               :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_NORM                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_SL;	/* DIP_X_BNR2_PDC_SL, DIP_A_BNR2_PDC_SL*/

typedef volatile union _DIP_X_REG_BNR2_PDC_POS_
{
		volatile struct	/* 0x150232BC */
		{
				FIELD  PDC_XCENTER                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDC_YCENTER                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_POS;	/* DIP_X_BNR2_PDC_POS, DIP_A_BNR2_PDC_POS*/

typedef volatile union _DIP_X_REG_RMM2_OSC_
{
		volatile struct	/* 0x150232C0 */
		{
				FIELD  RMM_OSC_TH                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  RMM_SEDIR_SL                          :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RMM_SEBLD_WD                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  RMM_LEBLD_WD                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  RMM_LE_INV_CTL                        :  4;		/* 24..27, 0x0F000000 */
				FIELD  RMM_EDGE                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_OSC;	/* DIP_X_RMM2_OSC, DIP_A_RMM2_OSC*/

typedef volatile union _DIP_X_REG_RMM2_MC_
{
		volatile struct	/* 0x150232C4 */
		{
				FIELD  RMM_MO_EDGE                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RMM_MO_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_MOBLD_FT                          :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RMM_MOTH_RATIO                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMM_HORI_ADDWT                        :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_MC;	/* DIP_X_RMM2_MC, DIP_A_RMM2_MC*/

typedef volatile union _DIP_X_REG_RMM2_REVG_1_
{
		volatile struct	/* 0x150232C8 */
		{
				FIELD  RMM_REVG_R                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  RMM_REVG_GR                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_REVG_1;	/* DIP_X_RMM2_REVG_1, DIP_A_RMM2_REVG_1*/

typedef volatile union _DIP_X_REG_RMM2_REVG_2_
{
		volatile struct	/* 0x150232CC */
		{
				FIELD  RMM_REVG_B                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  RMM_REVG_GB                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_REVG_2;	/* DIP_X_RMM2_REVG_2, DIP_A_RMM2_REVG_2*/

typedef volatile union _DIP_X_REG_RMM2_LEOS_
{
		volatile struct	/* 0x150232D0 */
		{
				FIELD  RMM_LEOS_GRAY                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_LEOS;	/* DIP_X_RMM2_LEOS, DIP_A_RMM2_LEOS*/

typedef volatile union _DIP_X_REG_RMM2_MC2_
{
		volatile struct	/* 0x150232D4 */
		{
				FIELD  RMM_MOSE_TH                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RMM_MOSE_BLDWD                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_MC2;	/* DIP_X_RMM2_MC2, DIP_A_RMM2_MC2*/

typedef volatile union _DIP_X_REG_RMM2_DIFF_LB_
{
		volatile struct	/* 0x150232D8 */
		{
				FIELD  RMM_DIFF_LB                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_DIFF_LB;	/* DIP_X_RMM2_DIFF_LB, DIP_A_RMM2_DIFF_LB*/

typedef volatile union _DIP_X_REG_RMM2_MA_
{
		volatile struct	/* 0x150232DC */
		{
				FIELD  RMM_MASE_RATIO                        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RMM_OSBLD_WD                          :  4;		/*  8..11, 0x00000F00 */
				FIELD  RMM_MASE_BLDWD                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  RMM_SENOS_LEFAC                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RMM_SEYOS_LEFAC                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_MA;	/* DIP_X_RMM2_MA, DIP_A_RMM2_MA*/

typedef volatile union _DIP_X_REG_RMM2_TUNE_
{
		volatile struct	/* 0x150232E0 */
		{
				FIELD  RMM_PSHOR_SEEN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RMM_PS_BLUR                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RMM_OSC_REPEN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RMM_SOFT_TH_EN                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RMM_LE_LOWPA_EN                       :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RMM_SE_LOWPA_EN                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RMM_PSSEC_ONLY                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  RMM_OSCLE_ONLY                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RMM_PS_TH                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  RMM_MOLE_DIREN                        :  1;		/* 28..28, 0x10000000 */
				FIELD  RMM_MOSE_DIREN                        :  1;		/* 29..29, 0x20000000 */
				FIELD  RMM_MO_2DBLD_EN                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_TUNE;	/* DIP_X_RMM2_TUNE, DIP_A_RMM2_TUNE*/

typedef volatile union _DIP_X_REG_LSC2_CTL1_
{
		volatile struct	/* 0x15023340 */
		{
				FIELD  SDBLK_YOFST                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  SDBLK_XOFST                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  LSC_EXTEND_COEF_MODE                  :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  SD_COEFRD_MODE                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  SD_ULTRA_MODE                         :  1;		/* 28..28, 0x10000000 */
				FIELD  LSC_PRC_MODE                          :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_CTL1;	/* DIP_X_LSC2_CTL1, DIP_A_LSC2_CTL1*/

typedef volatile union _DIP_X_REG_LSC2_CTL2_
{
		volatile struct	/* 0x15023344 */
		{
				FIELD  LSC_SDBLK_WIDTH                       : 13;		/*  0..12, 0x00001FFF */
				FIELD  LSC_SDBLK_XNUM                        :  5;		/* 13..17, 0x0003E000 */
				FIELD  LSC_OFLN                              :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_CTL2;	/* DIP_X_LSC2_CTL2, DIP_A_LSC2_CTL2*/

typedef volatile union _DIP_X_REG_LSC2_CTL3_
{
		volatile struct	/* 0x15023348 */
		{
				FIELD  LSC_SDBLK_HEIGHT                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  LSC_SDBLK_YNUM                        :  5;		/* 13..17, 0x0003E000 */
				FIELD  LSC_SPARE                             : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_CTL3;	/* DIP_X_LSC2_CTL3, DIP_A_LSC2_CTL3*/

typedef volatile union _DIP_X_REG_LSC2_LBLOCK_
{
		volatile struct	/* 0x1502334C */
		{
				FIELD  LSC_SDBLK_lHEIGHT                     : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_SDBLK_lWIDTH                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_LBLOCK;	/* DIP_X_LSC2_LBLOCK, DIP_A_LSC2_LBLOCK*/

typedef volatile union _DIP_X_REG_LSC2_RATIO_0_
{
		volatile struct	/* 0x15023350 */
		{
				FIELD  LSC_RA03                              :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  LSC_RA02                              :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_RA01                              :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  LSC_RA00                              :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_RATIO_0;	/* DIP_X_LSC2_RATIO_0, DIP_A_LSC2_RATIO_0*/

typedef volatile union _DIP_X_REG_LSC2_TPIPE_OFST_
{
		volatile struct	/* 0x15023354 */
		{
				FIELD  LSC_TPIPE_OFST_Y                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_TPIPE_OFST_X                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_TPIPE_OFST;	/* DIP_X_LSC2_TPIPE_OFST, DIP_A_LSC2_TPIPE_OFST*/

typedef volatile union _DIP_X_REG_LSC2_TPIPE_SIZE_
{
		volatile struct	/* 0x15023358 */
		{
				FIELD  LSC_TPIPE_SIZE_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_TPIPE_SIZE_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_TPIPE_SIZE;	/* DIP_X_LSC2_TPIPE_SIZE, DIP_A_LSC2_TPIPE_SIZE*/

typedef volatile union _DIP_X_REG_LSC2_GAIN_TH_
{
		volatile struct	/* 0x1502335C */
		{
				FIELD  LSC_GAIN_TH2                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSC_GAIN_TH1                          :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  LSC_GAIN_TH0                          :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_GAIN_TH;	/* DIP_X_LSC2_GAIN_TH, DIP_A_LSC2_GAIN_TH*/

typedef volatile union _DIP_X_REG_LSC2_RATIO_1_
{
		volatile struct	/* 0x15023360 */
		{
				FIELD  LSC_RA13                              :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  LSC_RA12                              :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_RA11                              :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  LSC_RA10                              :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_RATIO_1;	/* DIP_X_LSC2_RATIO_1, DIP_A_LSC2_RATIO_1*/

typedef volatile union _DIP_X_REG_LSC2_UPB_B_GB_
{
		volatile struct	/* 0x15023364 */
		{
				FIELD  LSC_UPB_GB                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_UPB_B                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_UPB_B_GB;	/* DIP_X_LSC2_UPB_B_GB, DIP_A_LSC2_UPB_B_GB*/

typedef volatile union _DIP_X_REG_LSC2_UPB_GR_R_
{
		volatile struct	/* 0x15023368 */
		{
				FIELD  LSC_UPB_R                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_UPB_GR                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_UPB_GR_R;	/* DIP_X_LSC2_UPB_GR_R, DIP_A_LSC2_UPB_GR_R*/

typedef volatile union _DIP_X_REG_SL2K_CEN_
{
		volatile struct	/* 0x15023400 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_CEN;	/* DIP_X_SL2K_CEN, DIP_A_SL2K_CEN*/

typedef volatile union _DIP_X_REG_SL2K_RR_CON0_
{
		volatile struct	/* 0x15023404 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_RR_CON0;	/* DIP_X_SL2K_RR_CON0, DIP_A_SL2K_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2K_RR_CON1_
{
		volatile struct	/* 0x15023408 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_RR_CON1;	/* DIP_X_SL2K_RR_CON1, DIP_A_SL2K_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2K_GAIN_
{
		volatile struct	/* 0x1502340C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_GAIN;	/* DIP_X_SL2K_GAIN, DIP_A_SL2K_GAIN*/

typedef volatile union _DIP_X_REG_SL2K_RZ_
{
		volatile struct	/* 0x15023410 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_RZ;	/* DIP_X_SL2K_RZ, DIP_A_SL2K_RZ*/

typedef volatile union _DIP_X_REG_SL2K_XOFF_
{
		volatile struct	/* 0x15023414 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_XOFF;	/* DIP_X_SL2K_XOFF, DIP_A_SL2K_XOFF*/

typedef volatile union _DIP_X_REG_SL2K_YOFF_
{
		volatile struct	/* 0x15023418 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_YOFF;	/* DIP_X_SL2K_YOFF, DIP_A_SL2K_YOFF*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON0_
{
		volatile struct	/* 0x1502341C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON0;	/* DIP_X_SL2K_SLP_CON0, DIP_A_SL2K_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON1_
{
		volatile struct	/* 0x15023420 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON1;	/* DIP_X_SL2K_SLP_CON1, DIP_A_SL2K_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON2_
{
		volatile struct	/* 0x15023424 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON2;	/* DIP_X_SL2K_SLP_CON2, DIP_A_SL2K_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON3_
{
		volatile struct	/* 0x15023428 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON3;	/* DIP_X_SL2K_SLP_CON3, DIP_A_SL2K_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2K_SIZE_
{
		volatile struct	/* 0x1502342C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SIZE;	/* DIP_X_SL2K_SIZE, DIP_A_SL2K_SIZE*/

typedef volatile union _DIP_X_REG_PGN_SATU_1_
{
		volatile struct	/* 0x15023440 */
		{
				FIELD  PGN_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_SATU_1;	/* DIP_X_PGN_SATU_1, DIP_A_PGN_SATU_1*/

typedef volatile union _DIP_X_REG_PGN_SATU_2_
{
		volatile struct	/* 0x15023444 */
		{
				FIELD  PGN_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_SATU_2;	/* DIP_X_PGN_SATU_2, DIP_A_PGN_SATU_2*/

typedef volatile union _DIP_X_REG_PGN_GAIN_1_
{
		volatile struct	/* 0x15023448 */
		{
				FIELD  PGN_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PGN_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_GAIN_1;	/* DIP_X_PGN_GAIN_1, DIP_A_PGN_GAIN_1*/

typedef volatile union _DIP_X_REG_PGN_GAIN_2_
{
		volatile struct	/* 0x1502344C */
		{
				FIELD  PGN_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  PGN_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_GAIN_2;	/* DIP_X_PGN_GAIN_2, DIP_A_PGN_GAIN_2*/

typedef volatile union _DIP_X_REG_PGN_OFST_1_
{
		volatile struct	/* 0x15023450 */
		{
				FIELD  PGN_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_OFST_1;	/* DIP_X_PGN_OFST_1, DIP_A_PGN_OFST_1*/

typedef volatile union _DIP_X_REG_PGN_OFST_2_
{
		volatile struct	/* 0x15023454 */
		{
				FIELD  PGN_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_OFST_2;	/* DIP_X_PGN_OFST_2, DIP_A_PGN_OFST_2*/

typedef volatile union _DIP_X_REG_RCP2_CROP_CON1_
{
		volatile struct	/* 0x15023480 */
		{
				FIELD  RCP_STR_X                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_X                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RCP2_CROP_CON1;	/* DIP_X_RCP2_CROP_CON1, DIP_A_RCP2_CROP_CON1*/

typedef volatile union _DIP_X_REG_RCP2_CROP_CON2_
{
		volatile struct	/* 0x15023484 */
		{
				FIELD  RCP_STR_Y                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_Y                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RCP2_CROP_CON2;	/* DIP_X_RCP2_CROP_CON2, DIP_A_RCP2_CROP_CON2*/

typedef volatile union _DIP_X_REG_RNR_CON1_
{
		volatile struct	/* 0x150234C0 */
		{
				FIELD  RNR_RAT                               :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_EPS                               :  8;		/*  9..16, 0x0001FE00 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_CON1;	/* DIP_X_RNR_CON1, DIP_A_RNR_CON1*/

typedef volatile union _DIP_X_REG_RNR_VC0_
{
		volatile struct	/* 0x150234C4 */
		{
				FIELD  RNR_VC0_R                             :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_VC0_G                             :  9;		/*  9..17, 0x0003FE00 */
				FIELD  RNR_VC0_B                             :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_VC0;	/* DIP_X_RNR_VC0, DIP_A_RNR_VC0*/

typedef volatile union _DIP_X_REG_RNR_VC1_
{
		volatile struct	/* 0x150234C8 */
		{
				FIELD  RNR_VC1_R                             :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_VC1_G                             :  9;		/*  9..17, 0x0003FE00 */
				FIELD  RNR_VC1_B                             :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_VC1;	/* DIP_X_RNR_VC1, DIP_A_RNR_VC1*/

typedef volatile union _DIP_X_REG_RNR_GO_B_
{
		volatile struct	/* 0x150234CC */
		{
				FIELD  RNR_OFST_B                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_B                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_B;	/* DIP_X_RNR_GO_B, DIP_A_RNR_GO_B*/

typedef volatile union _DIP_X_REG_RNR_GO_G_
{
		volatile struct	/* 0x150234D0 */
		{
				FIELD  RNR_OFST_G                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_G                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_G;	/* DIP_X_RNR_GO_G, DIP_A_RNR_GO_G*/

typedef volatile union _DIP_X_REG_RNR_GO_R_
{
		volatile struct	/* 0x150234D4 */
		{
				FIELD  RNR_OFST_R                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_R                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_R;	/* DIP_X_RNR_GO_R, DIP_A_RNR_GO_R*/

typedef volatile union _DIP_X_REG_RNR_GAIN_ISO_
{
		volatile struct	/* 0x150234D8 */
		{
				FIELD  RNR_GAIN_ISO                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GAIN_ISO;	/* DIP_X_RNR_GAIN_ISO, DIP_A_RNR_GAIN_ISO*/

typedef volatile union _DIP_X_REG_RNR_L_
{
		volatile struct	/* 0x150234DC */
		{
				FIELD  RNR_L_R                               :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_L_G                               :  9;		/*  9..17, 0x0003FE00 */
				FIELD  RNR_L_B                               :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_L;	/* DIP_X_RNR_L, DIP_A_RNR_L*/

typedef volatile union _DIP_X_REG_RNR_C1_
{
		volatile struct	/* 0x150234E0 */
		{
				FIELD  RNR_C1_R                              :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_C1_G                              :  9;		/*  9..17, 0x0003FE00 */
				FIELD  RNR_C1_B                              :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_C1;	/* DIP_X_RNR_C1, DIP_A_RNR_C1*/

typedef volatile union _DIP_X_REG_RNR_C2_
{
		volatile struct	/* 0x150234E4 */
		{
				FIELD  RNR_C2_R                              :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_C2_G                              :  9;		/*  9..17, 0x0003FE00 */
				FIELD  RNR_C2_B                              :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_C2;	/* DIP_X_RNR_C2, DIP_A_RNR_C2*/

typedef volatile union _DIP_X_REG_RNR_RNG_
{
		volatile struct	/* 0x150234E8 */
		{
				FIELD  RNR_RNG1                              :  6;		/*  0.. 5, 0x0000003F */
				FIELD  RNR_RNG2                              :  6;		/*  6..11, 0x00000FC0 */
				FIELD  RNR_RNG3                              :  6;		/* 12..17, 0x0003F000 */
				FIELD  RNR_RNG4                              :  6;		/* 18..23, 0x00FC0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_RNG;	/* DIP_X_RNR_RNG, DIP_A_RNR_RNG*/

typedef volatile union _DIP_X_REG_RNR_CON2_
{
		volatile struct	/* 0x150234EC */
		{
				FIELD  RNR_MED_RAT                           :  7;		/*  0.. 6, 0x0000007F */
				FIELD  RNR_PRF_RAT                           :  7;		/*  7..13, 0x00003F80 */
				FIELD  RNR_RAD                               :  3;		/* 14..16, 0x0001C000 */
				FIELD  RNR_CTHR                              :  6;		/* 17..22, 0x007E0000 */
				FIELD  RNR_CTHL                              :  6;		/* 23..28, 0x1F800000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_CON2;	/* DIP_X_RNR_CON2, DIP_A_RNR_CON2*/

typedef volatile union _DIP_X_REG_RNR_CON3_
{
		volatile struct	/* 0x150234F0 */
		{
				FIELD  RNR_FL_MODE                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RNR_GD_RAT                            :  6;		/*  1.. 6, 0x0000007E */
				FIELD  RNR_LLP_RAT                           :  8;		/*  7..14, 0x00007F80 */
				FIELD  RNR_INT_OFST                          :  8;		/* 15..22, 0x007F8000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_CON3;	/* DIP_X_RNR_CON3, DIP_A_RNR_CON3*/

typedef volatile union _DIP_X_REG_RNR_SL_
{
		volatile struct	/* 0x150234F4 */
		{
				FIELD  RNR_SL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RNR_SL_Y2                             : 10;		/*  1..10, 0x000007FE */
				FIELD  RNR_SL_Y1                             : 10;		/* 11..20, 0x001FF800 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_SL;	/* DIP_X_RNR_SL, DIP_A_RNR_SL*/

typedef volatile union _DIP_X_REG_RNR_SSL_STH_
{
		volatile struct	/* 0x150234F8 */
		{
				FIELD  RNR_SSL_C2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RNR_STH_C2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RNR_SSL_C1                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RNR_STH_C1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_SSL_STH;	/* DIP_X_RNR_SSL_STH, DIP_A_RNR_SSL_STH*/

typedef volatile union _DIP_X_REG_RNR_TILE_EDGE_
{
		volatile struct	/* 0x150234FC */
		{
				FIELD  RNR_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_TILE_EDGE;	/* DIP_X_RNR_TILE_EDGE, DIP_A_RNR_TILE_EDGE*/

typedef volatile union _DIP_X_REG_SL2H_CEN_
{
		volatile struct	/* 0x15023500 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_CEN;	/* DIP_X_SL2H_CEN, DIP_A_SL2H_CEN*/

typedef volatile union _DIP_X_REG_SL2H_RR_CON0_
{
		volatile struct	/* 0x15023504 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_RR_CON0;	/* DIP_X_SL2H_RR_CON0, DIP_A_SL2H_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2H_RR_CON1_
{
		volatile struct	/* 0x15023508 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_RR_CON1;	/* DIP_X_SL2H_RR_CON1, DIP_A_SL2H_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2H_GAIN_
{
		volatile struct	/* 0x1502350C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_GAIN;	/* DIP_X_SL2H_GAIN, DIP_A_SL2H_GAIN*/

typedef volatile union _DIP_X_REG_SL2H_RZ_
{
		volatile struct	/* 0x15023510 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_RZ;	/* DIP_X_SL2H_RZ, DIP_A_SL2H_RZ*/

typedef volatile union _DIP_X_REG_SL2H_XOFF_
{
		volatile struct	/* 0x15023514 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_XOFF;	/* DIP_X_SL2H_XOFF, DIP_A_SL2H_XOFF*/

typedef volatile union _DIP_X_REG_SL2H_YOFF_
{
		volatile struct	/* 0x15023518 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_YOFF;	/* DIP_X_SL2H_YOFF, DIP_A_SL2H_YOFF*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON0_
{
		volatile struct	/* 0x1502351C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON0;	/* DIP_X_SL2H_SLP_CON0, DIP_A_SL2H_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON1_
{
		volatile struct	/* 0x15023520 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON1;	/* DIP_X_SL2H_SLP_CON1, DIP_A_SL2H_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON2_
{
		volatile struct	/* 0x15023524 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON2;	/* DIP_X_SL2H_SLP_CON2, DIP_A_SL2H_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON3_
{
		volatile struct	/* 0x15023528 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON3;	/* DIP_X_SL2H_SLP_CON3, DIP_A_SL2H_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2H_SIZE_
{
		volatile struct	/* 0x1502352C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SIZE;	/* DIP_X_SL2H_SIZE, DIP_A_SL2H_SIZE*/

typedef volatile union _DIP_X_REG_UDM_INTP_CRS_
{
		volatile struct	/* 0x15023540 */
		{
				FIELD  UDM_CDG_SL                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  UDM_CDG_OFST                          :  8;		/*  4..11, 0x00000FF0 */
				FIELD  UDM_CDG_RAT                           :  5;		/* 12..16, 0x0001F000 */
				FIELD  UDM_CD_KNL                            :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 12;		/* 18..29, 0x3FFC0000 */
				FIELD  UDM_BYP                               :  1;		/* 30..30, 0x40000000 */
				FIELD  UDM_MN_MODE                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_INTP_CRS;	/* DIP_X_UDM_INTP_CRS, DIP_A_UDM_INTP_CRS*/

typedef volatile union _DIP_X_REG_UDM_INTP_NAT_
{
		volatile struct	/* 0x15023544 */
		{
				FIELD  UDM_HL_OFST                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_L0_SL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  UDM_L0_OFST                           :  8;		/* 12..19, 0x000FF000 */
				FIELD  UDM_CD_SLL                            :  5;		/* 20..24, 0x01F00000 */
				FIELD  UDM_CD_SLC                            :  5;		/* 25..29, 0x3E000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_INTP_NAT;	/* DIP_X_UDM_INTP_NAT, DIP_A_UDM_INTP_NAT*/

typedef volatile union _DIP_X_REG_UDM_INTP_AUG_
{
		volatile struct	/* 0x15023548 */
		{
				FIELD  UDM_DN_OFST                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_L2_SL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  UDM_L2_OFST                           :  8;		/* 12..19, 0x000FF000 */
				FIELD  UDM_L1_SL                             :  4;		/* 20..23, 0x00F00000 */
				FIELD  UDM_L1_OFST                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_INTP_AUG;	/* DIP_X_UDM_INTP_AUG, DIP_A_UDM_INTP_AUG*/

typedef volatile union _DIP_X_REG_UDM_LUMA_LUT1_
{
		volatile struct	/* 0x1502354C */
		{
				FIELD  UDM_LM_Y2                             :  9;		/*  0.. 8, 0x000001FF */
				FIELD  UDM_LM_Y1                             :  9;		/*  9..17, 0x0003FE00 */
				FIELD  UDM_LM_Y0                             :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_LUMA_LUT1;	/* DIP_X_UDM_LUMA_LUT1, DIP_A_UDM_LUMA_LUT1*/

typedef volatile union _DIP_X_REG_UDM_LUMA_LUT2_
{
		volatile struct	/* 0x15023550 */
		{
				FIELD  UDM_LM_Y5                             :  9;		/*  0.. 8, 0x000001FF */
				FIELD  UDM_LM_Y4                             :  9;		/*  9..17, 0x0003FE00 */
				FIELD  UDM_LM_Y3                             :  9;		/* 18..26, 0x07FC0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_LUMA_LUT2;	/* DIP_X_UDM_LUMA_LUT2, DIP_A_UDM_LUMA_LUT2*/

typedef volatile union _DIP_X_REG_UDM_SL_CTL_
{
		volatile struct	/* 0x15023554 */
		{
				FIELD  UDM_SL_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UDM_SL_HR                             :  5;		/*  1.. 5, 0x0000003E */
				FIELD  UDM_SL_Y2                             :  8;		/*  6..13, 0x00003FC0 */
				FIELD  UDM_SL_Y1                             :  8;		/* 14..21, 0x003FC000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_SL_CTL;	/* DIP_X_UDM_SL_CTL, DIP_A_UDM_SL_CTL*/

typedef volatile union _DIP_X_REG_UDM_HFTD_CTL_
{
		volatile struct	/* 0x15023558 */
		{
				FIELD  UDM_CORE_TH1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_HD_GN2                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  UDM_HD_GN1                            :  5;		/* 13..17, 0x0003E000 */
				FIELD  UDM_HT_GN2                            :  5;		/* 18..22, 0x007C0000 */
				FIELD  UDM_HT_GN1                            :  5;		/* 23..27, 0x0F800000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_HFTD_CTL;	/* DIP_X_UDM_HFTD_CTL, DIP_A_UDM_HFTD_CTL*/

typedef volatile union _DIP_X_REG_UDM_NR_STR_
{
		volatile struct	/* 0x1502355C */
		{
				FIELD  UDM_N2_STR                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  UDM_N1_STR                            :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  UDM_N0_STR                            :  5;		/* 10..14, 0x00007C00 */
				FIELD  UDM_XTK_SL                            :  4;		/* 15..18, 0x00078000 */
				FIELD  UDM_XTK_OFST                          :  8;		/* 19..26, 0x07F80000 */
				FIELD  UDM_XTK_RAT                           :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_NR_STR;	/* DIP_X_UDM_NR_STR, DIP_A_UDM_NR_STR*/

typedef volatile union _DIP_X_REG_UDM_NR_ACT_
{
		volatile struct	/* 0x15023560 */
		{
				FIELD  UDM_NGR                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  UDM_NSL                               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  UDM_N2_OFST                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_N1_OFST                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  UDM_N0_OFST                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_NR_ACT;	/* DIP_X_UDM_NR_ACT, DIP_A_UDM_NR_ACT*/

typedef volatile union _DIP_X_REG_UDM_HF_STR_
{
		volatile struct	/* 0x15023564 */
		{
				FIELD  UDM_CORE_TH2                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_HI_RAT                            :  4;		/*  8..11, 0x00000F00 */
				FIELD  UDM_H3_GN                             :  5;		/* 12..16, 0x0001F000 */
				FIELD  UDM_H2_GN                             :  5;		/* 17..21, 0x003E0000 */
				FIELD  UDM_H1_GN                             :  5;		/* 22..26, 0x07C00000 */
				FIELD  UDM_HA_STR                            :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_HF_STR;	/* DIP_X_UDM_HF_STR, DIP_A_UDM_HF_STR*/

typedef volatile union _DIP_X_REG_UDM_HF_ACT1_
{
		volatile struct	/* 0x15023568 */
		{
				FIELD  UDM_H2_UPB                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_H2_LWB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_H1_UPB                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  UDM_H1_LWB                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_HF_ACT1;	/* DIP_X_UDM_HF_ACT1, DIP_A_UDM_HF_ACT1*/

typedef volatile union _DIP_X_REG_UDM_HF_ACT2_
{
		volatile struct	/* 0x1502356C */
		{
				FIELD  UDM_HSLL                              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  UDM_HSLR                              :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  UDM_H3_UPB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_H3_LWB                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_HF_ACT2;	/* DIP_X_UDM_HF_ACT2, DIP_A_UDM_HF_ACT2*/

typedef volatile union _DIP_X_REG_UDM_CLIP_
{
		volatile struct	/* 0x15023570 */
		{
				FIELD  UDM_CLIP_TH                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_UN_TH                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_OV_TH                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_CLIP;	/* DIP_X_UDM_CLIP, DIP_A_UDM_CLIP*/

typedef volatile union _DIP_X_REG_UDM_DSB_
{
		volatile struct	/* 0x15023574 */
		{
				FIELD  UDM_SC_RAT                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  UDM_SL_RAT                            :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  UDM_FL_MODE                           :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_DSB;	/* DIP_X_UDM_DSB, DIP_A_UDM_DSB*/

typedef volatile union _DIP_X_REG_UDM_TILE_EDGE_
{
		volatile struct	/* 0x15023578 */
		{
				FIELD  UDM_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_TILE_EDGE;	/* DIP_X_UDM_TILE_EDGE, DIP_A_UDM_TILE_EDGE*/

typedef volatile union _DIP_X_REG_UDM_P1_ACT_
{
		volatile struct	/* 0x1502357C */
		{
				FIELD  UDM_P1_UPB                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_P1_LWB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_P1_ACT;	/* DIP_X_UDM_P1_ACT, DIP_A_UDM_P1_ACT*/

typedef volatile union _DIP_X_REG_UDM_LR_RAT_
{
		volatile struct	/* 0x15023580 */
		{
				FIELD  UDM_LR_RAT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_LR_RAT;	/* DIP_X_UDM_LR_RAT, DIP_A_UDM_LR_RAT*/

typedef volatile union _DIP_X_REG_UDM_HFTD_CTL2_
{
		volatile struct	/* 0x15023584 */
		{
				FIELD  UDM_HD_GN3                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  UDM_HFRB_GN                           :  6;		/*  6..11, 0x00000FC0 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_HFTD_CTL2;	/* DIP_X_UDM_HFTD_CTL2, DIP_A_UDM_HFTD_CTL2*/

typedef volatile union _DIP_X_REG_UDM_EST_CTL_
{
		volatile struct	/* 0x15023588 */
		{
				FIELD  UDM_P2_CLIP                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UDM_P1_BLD                            :  5;		/*  1.. 5, 0x0000003E */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_EST_CTL;	/* DIP_X_UDM_EST_CTL, DIP_A_UDM_EST_CTL*/

typedef volatile union _DIP_X_REG_UDM_SPARE_2_
{
		volatile struct	/* 0x1502358C */
		{
				FIELD  UDM_SPARE_2                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_SPARE_2;	/* DIP_X_UDM_SPARE_2, DIP_A_UDM_SPARE_2*/

typedef volatile union _DIP_X_REG_UDM_SPARE_3_
{
		volatile struct	/* 0x15023590 */
		{
				FIELD  UDM_SPARE_3                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_SPARE_3;	/* DIP_X_UDM_SPARE_3, DIP_A_UDM_SPARE_3*/

typedef volatile union _DIP_X_REG_UDM_INT_CTL_
{
		volatile struct	/* 0x15023594 */
		{
				FIELD  UDM_INT_LTH                           :  4;		/*  0.. 3, 0x0000000F */
				FIELD  UDM_INT_CDTH                          :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_INT_CTL;	/* DIP_X_UDM_INT_CTL, DIP_A_UDM_INT_CTL*/

typedef volatile union _DIP_X_REG_UDM_EE_
{
		volatile struct	/* 0x15023598 */
		{
				FIELD  UDM_HPOS_GN                           :  5;		/*  0.. 4, 0x0000001F */
				FIELD  UDM_HNEG_GN                           :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_EE;	/* DIP_X_UDM_EE, DIP_A_UDM_EE*/

typedef volatile union _DIP_X_REG_SL2_CEN_
{
		volatile struct	/* 0x150235C0 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_CEN;	/* DIP_X_SL2_CEN, DIP_A_SL2_CEN*/

typedef volatile union _DIP_X_REG_SL2_RR_CON0_
{
		volatile struct	/* 0x150235C4 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_RR_CON0;	/* DIP_X_SL2_RR_CON0, DIP_A_SL2_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2_RR_CON1_
{
		volatile struct	/* 0x150235C8 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_RR_CON1;	/* DIP_X_SL2_RR_CON1, DIP_A_SL2_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2_GAIN_
{
		volatile struct	/* 0x150235CC */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_GAIN;	/* DIP_X_SL2_GAIN, DIP_A_SL2_GAIN*/

typedef volatile union _DIP_X_REG_SL2_RZ_
{
		volatile struct	/* 0x150235D0 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_RZ;	/* DIP_X_SL2_RZ, DIP_A_SL2_RZ*/

typedef volatile union _DIP_X_REG_SL2_XOFF_
{
		volatile struct	/* 0x150235D4 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_XOFF;	/* DIP_X_SL2_XOFF, DIP_A_SL2_XOFF*/

typedef volatile union _DIP_X_REG_SL2_YOFF_
{
		volatile struct	/* 0x150235D8 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_YOFF;	/* DIP_X_SL2_YOFF, DIP_A_SL2_YOFF*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON0_
{
		volatile struct	/* 0x150235DC */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON0;	/* DIP_X_SL2_SLP_CON0, DIP_A_SL2_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON1_
{
		volatile struct	/* 0x150235E0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON1;	/* DIP_X_SL2_SLP_CON1, DIP_A_SL2_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON2_
{
		volatile struct	/* 0x150235E4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON2;	/* DIP_X_SL2_SLP_CON2, DIP_A_SL2_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON3_
{
		volatile struct	/* 0x150235E8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON3;	/* DIP_X_SL2_SLP_CON3, DIP_A_SL2_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2_SIZE_
{
		volatile struct	/* 0x150235EC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SIZE;	/* DIP_X_SL2_SIZE, DIP_A_SL2_SIZE*/

typedef volatile union _DIP_X_REG_ADBS2_CTL_
{
		volatile struct	/* 0x15023600 */
		{
				FIELD  ADBS_EDGE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ADBS_LE_INV_CTL                       :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_CTL;	/* DIP_X_ADBS2_CTL, DIP_A_ADBS2_CTL*/

typedef volatile union _DIP_X_REG_ADBS2_GRAY_BLD_0_
{
		volatile struct	/* 0x15023604 */
		{
				FIELD  ADBS_LUMA_MODE                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  ADBS_BLD_MXRT                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_GRAY_BLD_0;	/* DIP_X_ADBS2_GRAY_BLD_0, DIP_A_ADBS2_GRAY_BLD_0*/

typedef volatile union _DIP_X_REG_ADBS2_GRAY_BLD_1_
{
		volatile struct	/* 0x15023608 */
		{
				FIELD  ADBS_BLD_LOW                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  ADBS_BLD_SLP                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_GRAY_BLD_1;	/* DIP_X_ADBS2_GRAY_BLD_1, DIP_A_ADBS2_GRAY_BLD_1*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_R0_
{
		volatile struct	/* 0x1502360C */
		{
				FIELD  ADBS_BIAS_R0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_R1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_R2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_R3                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_R0;	/* DIP_X_ADBS2_BIAS_LUT_R0, DIP_A_ADBS2_BIAS_LUT_R0*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_R1_
{
		volatile struct	/* 0x15023610 */
		{
				FIELD  ADBS_BIAS_R4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_R5                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_R6                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_R7                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_R1;	/* DIP_X_ADBS2_BIAS_LUT_R1, DIP_A_ADBS2_BIAS_LUT_R1*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_R2_
{
		volatile struct	/* 0x15023614 */
		{
				FIELD  ADBS_BIAS_R8                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_R9                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_R10                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_R11                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_R2;	/* DIP_X_ADBS2_BIAS_LUT_R2, DIP_A_ADBS2_BIAS_LUT_R2*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_R3_
{
		volatile struct	/* 0x15023618 */
		{
				FIELD  ADBS_BIAS_R12                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_R13                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_R14                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_R3;	/* DIP_X_ADBS2_BIAS_LUT_R3, DIP_A_ADBS2_BIAS_LUT_R3*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_G0_
{
		volatile struct	/* 0x1502361C */
		{
				FIELD  ADBS_BIAS_G0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_G1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_G2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_G3                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_G0;	/* DIP_X_ADBS2_BIAS_LUT_G0, DIP_A_ADBS2_BIAS_LUT_G0*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_G1_
{
		volatile struct	/* 0x15023620 */
		{
				FIELD  ADBS_BIAS_G4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_G5                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_G6                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_G7                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_G1;	/* DIP_X_ADBS2_BIAS_LUT_G1, DIP_A_ADBS2_BIAS_LUT_G1*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_G2_
{
		volatile struct	/* 0x15023624 */
		{
				FIELD  ADBS_BIAS_G8                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_G9                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_G10                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_G11                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_G2;	/* DIP_X_ADBS2_BIAS_LUT_G2, DIP_A_ADBS2_BIAS_LUT_G2*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_G3_
{
		volatile struct	/* 0x15023628 */
		{
				FIELD  ADBS_BIAS_G12                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_G13                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_G14                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_G3;	/* DIP_X_ADBS2_BIAS_LUT_G3, DIP_A_ADBS2_BIAS_LUT_G3*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_B0_
{
		volatile struct	/* 0x1502362C */
		{
				FIELD  ADBS_BIAS_B0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_B1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_B2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_B3                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_B0;	/* DIP_X_ADBS2_BIAS_LUT_B0, DIP_A_ADBS2_BIAS_LUT_B0*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_B1_
{
		volatile struct	/* 0x15023630 */
		{
				FIELD  ADBS_BIAS_B4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_B5                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_B6                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_B7                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_B1;	/* DIP_X_ADBS2_BIAS_LUT_B1, DIP_A_ADBS2_BIAS_LUT_B1*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_B2_
{
		volatile struct	/* 0x15023634 */
		{
				FIELD  ADBS_BIAS_B8                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_B9                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_B10                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ADBS_BIAS_B11                         :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_B2;	/* DIP_X_ADBS2_BIAS_LUT_B2, DIP_A_ADBS2_BIAS_LUT_B2*/

typedef volatile union _DIP_X_REG_ADBS2_BIAS_LUT_B3_
{
		volatile struct	/* 0x15023638 */
		{
				FIELD  ADBS_BIAS_B12                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_BIAS_B13                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_BIAS_B14                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_BIAS_LUT_B3;	/* DIP_X_ADBS2_BIAS_LUT_B3, DIP_A_ADBS2_BIAS_LUT_B3*/

typedef volatile union _DIP_X_REG_ADBS2_GAIN_0_
{
		volatile struct	/* 0x1502363C */
		{
				FIELD  ADBS_GAIN_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ADBS_GAIN_B                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_GAIN_0;	/* DIP_X_ADBS2_GAIN_0, DIP_A_ADBS2_GAIN_0*/

typedef volatile union _DIP_X_REG_ADBS2_GAIN_1_
{
		volatile struct	/* 0x15023640 */
		{
				FIELD  ADBS_GAIN_G                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_GAIN_1;	/* DIP_X_ADBS2_GAIN_1, DIP_A_ADBS2_GAIN_1*/

typedef volatile union _DIP_X_REG_ADBS2_IVGN_0_
{
		volatile struct	/* 0x15023644 */
		{
				FIELD  ADBS_IVGN_R                           : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  ADBS_IVGN_B                           : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_IVGN_0;	/* DIP_X_ADBS2_IVGN_0, DIP_A_ADBS2_IVGN_0*/

typedef volatile union _DIP_X_REG_ADBS2_IVGN_1_
{
		volatile struct	/* 0x15023648 */
		{
				FIELD  ADBS_IVGN_G                           : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_IVGN_1;	/* DIP_X_ADBS2_IVGN_1, DIP_A_ADBS2_IVGN_1*/

typedef volatile union _DIP_X_REG_ADBS2_HDR_
{
		volatile struct	/* 0x1502364C */
		{
				FIELD  ADBS_HDR_GN                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ADBS_HDR_GN2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ADBS_HDR_GNP                          :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  ADBS_HDR_OSCTH                        : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_HDR;	/* DIP_X_ADBS2_HDR, DIP_A_ADBS2_HDR*/

typedef volatile union _DIP_X_REG_ADBS2_CMDL_ONLY_1_
{
		volatile struct	/* 0x15023650 */
		{
				FIELD  rsv_0                                 :  29;		/*  0.. 28, 0x1FFFFFFF */
				FIELD  ADBS_EIGER_EN                         :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ADBS2_CMDL_ONLY_1;	/* DIP_X_ADBS2_CMDL_ONLY_1, DIP_A_ADBS2_CMDL_ONLY_1*/

typedef volatile union _DIP_X_REG_DCPN_HDR_EN_
{
		volatile struct	/* 0x15023680 */
		{
				FIELD  DCPN_HDR_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DCPN_EDGE_INFO                        :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_HDR_EN;	/* DIP_X_DCPN_HDR_EN, DIP_A_DCPN_HDR_EN*/

typedef volatile union _DIP_X_REG_DCPN_IN_IMG_SIZE_
{
		volatile struct	/* 0x15023684 */
		{
				FIELD  DCPN_IN_IMG_H                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  DCPN_IN_IMG_W                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_IN_IMG_SIZE;	/* DIP_X_DCPN_IN_IMG_SIZE, DIP_A_DCPN_IN_IMG_SIZE*/

typedef volatile union _DIP_X_REG_DCPN_ALGO_PARAM1_
{
		volatile struct	/* 0x15023688 */
		{
				FIELD  DCPN_HDR_RATIO                        :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  DCPN_HDR_GAIN                         :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_ALGO_PARAM1;	/* DIP_X_DCPN_ALGO_PARAM1, DIP_A_DCPN_ALGO_PARAM1*/

typedef volatile union _DIP_X_REG_DCPN_ALGO_PARAM2_
{
		volatile struct	/* 0x1502368C */
		{
				FIELD  DCPN_HDR_TH_K                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DCPN_HDR_TH_T                         : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_ALGO_PARAM2;	/* DIP_X_DCPN_ALGO_PARAM2, DIP_A_DCPN_ALGO_PARAM2*/

typedef volatile union _DIP_X_REG_DCPN_GTM_X0_
{
		volatile struct	/* 0x15023690 */
		{
				FIELD  DCPN_GTM_X0                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_GTM_X0;	/* DIP_X_DCPN_GTM_X0, DIP_A_DCPN_GTM_X0*/

typedef volatile union _DIP_X_REG_DCPN_GTM_Y0_
{
		volatile struct	/* 0x15023694 */
		{
				FIELD  DCPN_GTM_Y0                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_GTM_Y0;	/* DIP_X_DCPN_GTM_Y0, DIP_A_DCPN_GTM_Y0*/

typedef volatile union _DIP_X_REG_DCPN_GTM_S0_
{
		volatile struct	/* 0x15023698 */
		{
				FIELD  DCPN_GTM_S0                           : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_GTM_S0;	/* DIP_X_DCPN_GTM_S0, DIP_A_DCPN_GTM_S0*/

typedef volatile union _DIP_X_REG_DCPN_GTM_S1_
{
		volatile struct	/* 0x1502369C */
		{
				FIELD  DCPN_GTM_S1                           : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DCPN_GTM_S1;	/* DIP_X_DCPN_GTM_S1, DIP_A_DCPN_GTM_S1*/

typedef volatile union _DIP_X_REG_CPN_HDR_CTL_EN_
{
		volatile struct	/* 0x150236C0 */
		{
				FIELD  CPN_HDR_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPN_AALLE_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPN_PSSE_EN                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CPN_EDGE_INFO                         :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_HDR_CTL_EN;	/* DIP_X_CPN_HDR_CTL_EN, DIP_A_CPN_HDR_CTL_EN*/

typedef volatile union _DIP_X_REG_CPN_IN_IMG_SIZE_
{
		volatile struct	/* 0x150236C4 */
		{
				FIELD  CPN_IN_IMG_H                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CPN_IN_IMG_W                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_IN_IMG_SIZE;	/* DIP_X_CPN_IN_IMG_SIZE, DIP_A_CPN_IN_IMG_SIZE*/

typedef volatile union _DIP_X_REG_CPN_ALGO_PARAM1_
{
		volatile struct	/* 0x150236C8 */
		{
				FIELD  CPN_HDR_RATIO                         :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CPN_HDR_GAIN                          :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_ALGO_PARAM1;	/* DIP_X_CPN_ALGO_PARAM1, DIP_A_CPN_ALGO_PARAM1*/

typedef volatile union _DIP_X_REG_CPN_ALGO_PARAM2_
{
		volatile struct	/* 0x150236CC */
		{
				FIELD  CPN_HDR_TH_K                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_HDR_TH_T                          : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_ALGO_PARAM2;	/* DIP_X_CPN_ALGO_PARAM2, DIP_A_CPN_ALGO_PARAM2*/

typedef volatile union _DIP_X_REG_CPN_GTM_X0X1_
{
		volatile struct	/* 0x150236D0 */
		{
				FIELD  CPN_GTM_X0                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_X1                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_X0X1;	/* DIP_X_CPN_GTM_X0X1, DIP_A_CPN_GTM_X0X1*/

typedef volatile union _DIP_X_REG_CPN_GTM_X2X3_
{
		volatile struct	/* 0x150236D4 */
		{
				FIELD  CPN_GTM_X2                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_X3                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_X2X3;	/* DIP_X_CPN_GTM_X2X3, DIP_A_CPN_GTM_X2X3*/

typedef volatile union _DIP_X_REG_CPN_GTM_X4X5_
{
		volatile struct	/* 0x150236D8 */
		{
				FIELD  CPN_GTM_X4                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_X5                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_X4X5;	/* DIP_X_CPN_GTM_X4X5, DIP_A_CPN_GTM_X4X5*/

typedef volatile union _DIP_X_REG_CPN_GTM_X6_
{
		volatile struct	/* 0x150236DC */
		{
				FIELD  CPN_GTM_X6                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_X6;	/* DIP_X_CPN_GTM_X6, DIP_A_CPN_GTM_X6*/

typedef volatile union _DIP_X_REG_CPN_GTM_Y0Y1_
{
		volatile struct	/* 0x150236E0 */
		{
				FIELD  CPN_GTM_Y0                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_Y1                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_Y0Y1;	/* DIP_X_CPN_GTM_Y0Y1, DIP_A_CPN_GTM_Y0Y1*/

typedef volatile union _DIP_X_REG_CPN_GTM_Y2Y3_
{
		volatile struct	/* 0x150236E4 */
		{
				FIELD  CPN_GTM_Y2                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_Y3                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_Y2Y3;	/* DIP_X_CPN_GTM_Y2Y3, DIP_A_CPN_GTM_Y2Y3*/

typedef volatile union _DIP_X_REG_CPN_GTM_Y4Y5_
{
		volatile struct	/* 0x150236E8 */
		{
				FIELD  CPN_GTM_Y4                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_Y5                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_Y4Y5;	/* DIP_X_CPN_GTM_Y4Y5, DIP_A_CPN_GTM_Y4Y5*/

typedef volatile union _DIP_X_REG_CPN_GTM_Y6_
{
		volatile struct	/* 0x150236EC */
		{
				FIELD  CPN_GTM_Y6                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_Y6;	/* DIP_X_CPN_GTM_Y6, DIP_A_CPN_GTM_Y6*/

typedef volatile union _DIP_X_REG_CPN_GTM_S0S1_
{
		volatile struct	/* 0x150236F0 */
		{
				FIELD  CPN_GTM_S0                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_S1                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_S0S1;	/* DIP_X_CPN_GTM_S0S1, DIP_A_CPN_GTM_S0S1*/

typedef volatile union _DIP_X_REG_CPN_GTM_S2S3_
{
		volatile struct	/* 0x150236F4 */
		{
				FIELD  CPN_GTM_S2                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_S3                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_S2S3;	/* DIP_X_CPN_GTM_S2S3, DIP_A_CPN_GTM_S2S3*/

typedef volatile union _DIP_X_REG_CPN_GTM_S4S5_
{
		volatile struct	/* 0x150236F8 */
		{
				FIELD  CPN_GTM_S4                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_S5                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_S4S5;	/* DIP_X_CPN_GTM_S4S5, DIP_A_CPN_GTM_S4S5*/

typedef volatile union _DIP_X_REG_CPN_GTM_S6S7_
{
		volatile struct	/* 0x150236FC */
		{
				FIELD  CPN_GTM_S6                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPN_GTM_S7                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CPN_GTM_S6S7;	/* DIP_X_CPN_GTM_S6S7, DIP_A_CPN_GTM_S6S7*/

typedef volatile union _DIP_X_REG_FLC_OFFST0_
{
		volatile struct	/* 0x15024000 */
		{
				FIELD  FLC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST0;	/* DIP_X_FLC_OFFST0, DIP_A_FLC_OFFST0*/

typedef volatile union _DIP_X_REG_FLC_OFFST1_
{
		volatile struct	/* 0x15024004 */
		{
				FIELD  FLC_OFST_G                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST1;	/* DIP_X_FLC_OFFST1, DIP_A_FLC_OFFST1*/

typedef volatile union _DIP_X_REG_FLC_OFFST2_
{
		volatile struct	/* 0x15024008 */
		{
				FIELD  FLC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST2;	/* DIP_X_FLC_OFFST2, DIP_A_FLC_OFFST2*/

typedef volatile union _DIP_X_REG_FLC_GAIN0_
{
		volatile struct	/* 0x15024010 */
		{
				FIELD  FLC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN0;	/* DIP_X_FLC_GAIN0, DIP_A_FLC_GAIN0*/

typedef volatile union _DIP_X_REG_FLC_GAIN1_
{
		volatile struct	/* 0x15024014 */
		{
				FIELD  FLC_GAIN_G                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN1;	/* DIP_X_FLC_GAIN1, DIP_A_FLC_GAIN1*/

typedef volatile union _DIP_X_REG_FLC_GAIN2_
{
		volatile struct	/* 0x15024018 */
		{
				FIELD  FLC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN2;	/* DIP_X_FLC_GAIN2, DIP_A_FLC_GAIN2*/

typedef volatile union _DIP_X_REG_FLC2_OFFST0_
{
		volatile struct	/* 0x15024040 */
		{
				FIELD  FLC2_OFST_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST0;	/* DIP_X_FLC2_OFFST0, DIP_A_FLC2_OFFST0*/

typedef volatile union _DIP_X_REG_FLC2_OFFST1_
{
		volatile struct	/* 0x15024044 */
		{
				FIELD  FLC2_OFST_G                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST1;	/* DIP_X_FLC2_OFFST1, DIP_A_FLC2_OFFST1*/

typedef volatile union _DIP_X_REG_FLC2_OFFST2_
{
		volatile struct	/* 0x15024048 */
		{
				FIELD  FLC2_OFST_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST2;	/* DIP_X_FLC2_OFFST2, DIP_A_FLC2_OFFST2*/

typedef volatile union _DIP_X_REG_FLC2_GAIN0_
{
		volatile struct	/* 0x15024050 */
		{
				FIELD  FLC2_GAIN_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN0;	/* DIP_X_FLC2_GAIN0, DIP_A_FLC2_GAIN0*/

typedef volatile union _DIP_X_REG_FLC2_GAIN1_
{
		volatile struct	/* 0x15024054 */
		{
				FIELD  FLC2_GAIN_G                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN1;	/* DIP_X_FLC2_GAIN1, DIP_A_FLC2_GAIN1*/

typedef volatile union _DIP_X_REG_FLC2_GAIN2_
{
		volatile struct	/* 0x15024058 */
		{
				FIELD  FLC2_GAIN_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN2;	/* DIP_X_FLC2_GAIN2, DIP_A_FLC2_GAIN2*/

typedef volatile union _DIP_X_REG_G2G_CNV_1_
{
		volatile struct	/* 0x15024080 */
		{
				FIELD  G2G_CNV_00                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_01                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_1;	/* DIP_X_G2G_CNV_1, DIP_A_G2G_CNV_1*/

typedef volatile union _DIP_X_REG_G2G_CNV_2_
{
		volatile struct	/* 0x15024084 */
		{
				FIELD  G2G_CNV_02                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_2;	/* DIP_X_G2G_CNV_2, DIP_A_G2G_CNV_2*/

typedef volatile union _DIP_X_REG_G2G_CNV_3_
{
		volatile struct	/* 0x15024088 */
		{
				FIELD  G2G_CNV_10                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_11                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_3;	/* DIP_X_G2G_CNV_3, DIP_A_G2G_CNV_3*/

typedef volatile union _DIP_X_REG_G2G_CNV_4_
{
		volatile struct	/* 0x1502408C */
		{
				FIELD  G2G_CNV_12                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_4;	/* DIP_X_G2G_CNV_4, DIP_A_G2G_CNV_4*/

typedef volatile union _DIP_X_REG_G2G_CNV_5_
{
		volatile struct	/* 0x15024090 */
		{
				FIELD  G2G_CNV_20                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_21                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_5;	/* DIP_X_G2G_CNV_5, DIP_A_G2G_CNV_5*/

typedef volatile union _DIP_X_REG_G2G_CNV_6_
{
		volatile struct	/* 0x15024094 */
		{
				FIELD  G2G_CNV_22                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_6;	/* DIP_X_G2G_CNV_6, DIP_A_G2G_CNV_6*/

typedef volatile union _DIP_X_REG_G2G_CTRL_
{
		volatile struct	/* 0x15024098 */
		{
				FIELD  G2G_ACC                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  G2G_CFC_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  G2G_L                                 : 12;		/*  5..16, 0x0001FFE0 */
				FIELD  G2G_H                                 : 12;		/* 17..28, 0x1FFE0000 */
				FIELD  G2G_MOFST_R                           :  1;		/* 29..29, 0x20000000 */
				FIELD  G2G_POFST_R                           :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CTRL;	/* DIP_X_G2G_CTRL, DIP_A_G2G_CTRL*/

typedef volatile union _DIP_X_REG_G2G_CFC_
{
		volatile struct	/* 0x1502409C */
		{
				FIELD  G2G_LB                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  G2G_HB                                :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  G2G_LG                                :  5;		/* 10..14, 0x00007C00 */
				FIELD  G2G_HG                                :  5;		/* 15..19, 0x000F8000 */
				FIELD  G2G_LR                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  G2G_HR                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CFC;	/* DIP_X_G2G_CFC, DIP_A_G2G_CFC*/

typedef volatile union _DIP_X_REG_G2G2_CNV_1_
{
		volatile struct	/* 0x150240C0 */
		{
				FIELD  G2G_CNV_00                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_01                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_1;	/* DIP_X_G2G2_CNV_1, DIP_A_G2G2_CNV_1*/

typedef volatile union _DIP_X_REG_G2G2_CNV_2_
{
		volatile struct	/* 0x150240C4 */
		{
				FIELD  G2G_CNV_02                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_2;	/* DIP_X_G2G2_CNV_2, DIP_A_G2G2_CNV_2*/

typedef volatile union _DIP_X_REG_G2G2_CNV_3_
{
		volatile struct	/* 0x150240C8 */
		{
				FIELD  G2G_CNV_10                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_11                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_3;	/* DIP_X_G2G2_CNV_3, DIP_A_G2G2_CNV_3*/

typedef volatile union _DIP_X_REG_G2G2_CNV_4_
{
		volatile struct	/* 0x150240CC */
		{
				FIELD  G2G_CNV_12                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_4;	/* DIP_X_G2G2_CNV_4, DIP_A_G2G2_CNV_4*/

typedef volatile union _DIP_X_REG_G2G2_CNV_5_
{
		volatile struct	/* 0x150240D0 */
		{
				FIELD  G2G_CNV_20                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  G2G_CNV_21                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_5;	/* DIP_X_G2G2_CNV_5, DIP_A_G2G2_CNV_5*/

typedef volatile union _DIP_X_REG_G2G2_CNV_6_
{
		volatile struct	/* 0x150240D4 */
		{
				FIELD  G2G_CNV_22                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CNV_6;	/* DIP_X_G2G2_CNV_6, DIP_A_G2G2_CNV_6*/

typedef volatile union _DIP_X_REG_G2G2_CTRL_
{
		volatile struct	/* 0x150240D8 */
		{
				FIELD  G2G_ACC                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  G2G_CFC_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  G2G_L                                 : 12;		/*  5..16, 0x0001FFE0 */
				FIELD  G2G_H                                 : 12;		/* 17..28, 0x1FFE0000 */
				FIELD  G2G_MOFST_R                           :  1;		/* 29..29, 0x20000000 */
				FIELD  G2G_POFST_R                           :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CTRL;	/* DIP_X_G2G2_CTRL, DIP_A_G2G2_CTRL*/

typedef volatile union _DIP_X_REG_G2G2_CFC_
{
		volatile struct	/* 0x150240DC */
		{
				FIELD  G2G_LB                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  G2G_HB                                :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  G2G_LG                                :  5;		/* 10..14, 0x00007C00 */
				FIELD  G2G_HG                                :  5;		/* 15..19, 0x000F8000 */
				FIELD  G2G_LR                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  G2G_HR                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G2_CFC;	/* DIP_X_G2G2_CFC, DIP_A_G2G2_CFC*/

typedef volatile union _DIP_X_REG_LCE25_CON_
{
		volatile struct	/* 0x15024100 */
		{
				FIELD  LCE_GLOB                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  LCE_GLOB_VHALF                        :  7;		/*  1.. 7, 0x000000FE */
				FIELD  LCE_EDGE                              :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_CON;	/* DIP_X_LCE25_CON, DIP_A_LCE25_CON*/

typedef volatile union _DIP_X_REG_LCE25_ZR_
{
		volatile struct	/* 0x15024104 */
		{
				FIELD  LCE_BCMK_X                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LCE_BCMK_Y                            : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_ZR;	/* DIP_X_LCE25_ZR, DIP_A_LCE25_ZR*/

typedef volatile union _DIP_X_REG_LCE25_SLM_SIZE_
{
		volatile struct	/* 0x15024108 */
		{
				FIELD  LCE_SLM_WD                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  LCE_SLM_HT                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_SLM_SIZE;	/* DIP_X_LCE25_SLM_SIZE, DIP_A_LCE25_SLM_SIZE*/

typedef volatile union _DIP_X_REG_LCE25_OFST_
{
		volatile struct	/* 0x1502410C */
		{
				FIELD  LCE_OFST_X                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LCE_OFST_Y                            : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_OFST;	/* DIP_X_LCE25_OFST, DIP_A_LCE25_OFST*/

typedef volatile union _DIP_X_REG_LCE25_BIAS_
{
		volatile struct	/* 0x15024110 */
		{
				FIELD  LCE_BIAS_X                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LCE_BIAS_Y                            :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_BIAS;	/* DIP_X_LCE25_BIAS, DIP_A_LCE25_BIAS*/

typedef volatile union _DIP_X_REG_LCE25_IMAGE_SIZE_
{
		volatile struct	/* 0x15024114 */
		{
				FIELD  LCE_IMAGE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LCE_IMAGE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_IMAGE_SIZE;	/* DIP_X_LCE25_IMAGE_SIZE, DIP_A_LCE25_IMAGE_SIZE*/

typedef volatile union _DIP_X_REG_LCE25_BIL_TH0_
{
		volatile struct	/* 0x15024118 */
		{
				FIELD  LCE_BIL_TH1                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_BIL_TH2                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_BIL_TH0;	/* DIP_X_LCE25_BIL_TH0, DIP_A_LCE25_BIL_TH0*/

typedef volatile union _DIP_X_REG_LCE25_BIL_TH1_
{
		volatile struct	/* 0x1502411C */
		{
				FIELD  LCE_BIL_TH3                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_BIL_TH4                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_BIL_TH1;	/* DIP_X_LCE25_BIL_TH1, DIP_A_LCE25_BIL_TH1*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA0_
{
		volatile struct	/* 0x15024120 */
		{
				FIELD  LCE_CEN_END                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_CEN_BLD_WT                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  LCE_GLOB_TONE                         :  1;		/* 24..24, 0x01000000 */
				FIELD  LCE_GLOB_POS                          :  1;		/* 25..25, 0x02000000 */
				FIELD  LCE_LC_TONE                           :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA0;	/* DIP_X_LCE25_TM_PARA0, DIP_A_LCE25_TM_PARA0*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA1_
{
		volatile struct	/* 0x15024124 */
		{
				FIELD  LCE_CEN_MAX_SLP                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LCE_CEN_MIN_SLP                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LCE_TC_P1                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA1;	/* DIP_X_LCE25_TM_PARA1, DIP_A_LCE25_TM_PARA1*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA2_
{
		volatile struct	/* 0x15024128 */
		{
				FIELD  LCE_TC_P50                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_P500                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA2;	/* DIP_X_LCE25_TM_PARA2, DIP_A_LCE25_TM_PARA2*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA3_
{
		volatile struct	/* 0x1502412C */
		{
				FIELD  LCE_TC_P950                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_P999                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA3;	/* DIP_X_LCE25_TM_PARA3, DIP_A_LCE25_TM_PARA3*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA4_
{
		volatile struct	/* 0x15024130 */
		{
				FIELD  LCE_TC_O1                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_O50                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA4;	/* DIP_X_LCE25_TM_PARA4, DIP_A_LCE25_TM_PARA4*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA5_
{
		volatile struct	/* 0x15024134 */
		{
				FIELD  LCE_TC_O500                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_O950                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA5;	/* DIP_X_LCE25_TM_PARA5, DIP_A_LCE25_TM_PARA5*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA6_
{
		volatile struct	/* 0x15024138 */
		{
				FIELD  LCE_TC_O999                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  8;		/* 12..19, 0x000FF000 */
				FIELD  LCE_CLR_BLDWD                         :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA6;	/* DIP_X_LCE25_TM_PARA6, DIP_A_LCE25_TM_PARA6*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA0_
{
		volatile struct	/* 0x1502413C */
		{
				FIELD  LCE_HLR_TC_P1                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_P50                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  LCE_HLR_LMP_EN                        :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA0;	/* DIP_X_LCE25_HLR_PARA0, DIP_A_LCE25_HLR_PARA0*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA1_
{
		volatile struct	/* 0x15024140 */
		{
				FIELD  LCE_HLR_TC_P500                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_P950                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA1;	/* DIP_X_LCE25_HLR_PARA1, DIP_A_LCE25_HLR_PARA1*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA2_
{
		volatile struct	/* 0x15024144 */
		{
				FIELD  LCE_HLR_TC_P999                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_O1                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA2;	/* DIP_X_LCE25_HLR_PARA2, DIP_A_LCE25_HLR_PARA2*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA3_
{
		volatile struct	/* 0x15024148 */
		{
				FIELD  LCE_HLR_TC_O50                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_O500                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA3;	/* DIP_X_LCE25_HLR_PARA3, DIP_A_LCE25_HLR_PARA3*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA4_
{
		volatile struct	/* 0x1502414C */
		{
				FIELD  LCE_HLR_TC_O950                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_O999                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA4;	/* DIP_X_LCE25_HLR_PARA4, DIP_A_LCE25_HLR_PARA4*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA7_
{
		volatile struct	/* 0x15024150 */
		{
				FIELD  LCE_TC_P0                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_P250                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA7;	/* DIP_X_LCE25_TM_PARA7, DIP_A_LCE25_TM_PARA7*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA8_
{
		volatile struct	/* 0x15024154 */
		{
				FIELD  LCE_TC_O0                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_O250                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA8;	/* DIP_X_LCE25_TM_PARA8, DIP_A_LCE25_TM_PARA8*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA5_
{
		volatile struct	/* 0x15024158 */
		{
				FIELD  LCE_HLR_TC_P0                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_P250                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA5;	/* DIP_X_LCE25_HLR_PARA5, DIP_A_LCE25_HLR_PARA5*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA6_
{
		volatile struct	/* 0x1502415C */
		{
				FIELD  LCE_HLR_TC_O0                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_O250                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA6;	/* DIP_X_LCE25_HLR_PARA6, DIP_A_LCE25_HLR_PARA6*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA9_
{
		volatile struct	/* 0x15024160 */
		{
				FIELD  LCE_TC_P750                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_TC_O750                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA9;	/* DIP_X_LCE25_TM_PARA9, DIP_A_LCE25_TM_PARA9*/

typedef volatile union _DIP_X_REG_LCE25_HLR_PARA7_
{
		volatile struct	/* 0x15024164 */
		{
				FIELD  LCE_HLR_TC_P750                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_HLR_TC_O750                       : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_HLR_PARA7;	/* DIP_X_LCE25_HLR_PARA7, DIP_A_LCE25_HLR_PARA7*/

typedef volatile union _DIP_X_REG_LCE25_TCHL_PARA0_
{
		volatile struct	/* 0x15024168 */
		{
				FIELD  LCE_CEN_MAX_SLP_N                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LCE_CEN_MIN_SLP_N                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  LCE_CEN_SLOPE_DEP_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TCHL_PARA0;	/* DIP_X_LCE25_TCHL_PARA0, DIP_A_LCE25_TCHL_PARA0*/

typedef volatile union _DIP_X_REG_LCE25_TCHL_PARA1_
{
		volatile struct	/* 0x1502416C */
		{
				FIELD  LCE_TCHL_DTH1                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LCE_TCHL_MTH                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LCE_TCHL_MGIAN                        :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  LCE_TCHL_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TCHL_PARA1;	/* DIP_X_LCE25_TCHL_PARA1, DIP_A_LCE25_TCHL_PARA1*/

typedef volatile union _DIP_X_REG_LCE25_TCHL_PARA2_
{
		volatile struct	/* 0x15024170 */
		{
				FIELD  LCE_TCHL_BW1                          :  4;		/*  0.. 3, 0x0000000F */
				FIELD  LCE_TCHL_BW2                          :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LCE_TCHL_DTH2                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TCHL_PARA2;	/* DIP_X_LCE25_TCHL_PARA2, DIP_A_LCE25_TCHL_PARA2*/

typedef volatile union _DIP_X_REG_GGM_LUT_
{
		volatile struct	/* 0x15024180 ~ 0x1502447F */
		{
				FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  GGM_G                                 : 10;		/* 10..19, 0x000FFC00 */
				FIELD  GGM_B                                 : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM_LUT;	/* DIP_X_GGM_LUT, DIP_A_GGM_LUT*/

typedef volatile union _DIP_X_REG_GGM_CTRL_
{
		volatile struct	/* 0x15024480 */
		{
				FIELD  GGM_LNR                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  GGM_END_VAR                           : 10;		/*  1..10, 0x000007FE */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  GGM_RMP_VAR                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM_CTRL;	/* DIP_X_GGM_CTRL, DIP_A_GGM_CTRL*/

typedef volatile union _DIP_X_REG_GGM2_LUT_
{
		volatile struct	/* 0x15024640 ~ 0x1502493F */
		{
				FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  GGM_G                                 : 10;		/* 10..19, 0x000FFC00 */
				FIELD  GGM_B                                 : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM2_LUT;	/* DIP_X_GGM2_LUT, DIP_A_GGM2_LUT*/

typedef volatile union _DIP_X_REG_GGM2_CTRL_
{
		volatile struct	/* 0x15024940 */
		{
				FIELD  GGM_LNR                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  GGM_END_VAR                           : 10;		/*  1..10, 0x000007FE */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  GGM_RMP_VAR                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM2_CTRL;	/* DIP_X_GGM2_CTRL, DIP_A_GGM2_CTRL*/

typedef volatile union _DIP_X_REG_WSHIFT_SPARE0_
{
		volatile struct	/* 0x15024B00 */
		{
				FIELD  WSHIFT_SPARE0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSHIFT_SPARE0;	/* DIP_X_WSHIFT_SPARE0, DIP_A_WSHIFT_SPARE0*/

typedef volatile union _DIP_X_REG_WSHIFT_SPARE1_
{
		volatile struct	/* 0x15024B04 */
		{
				FIELD  WSHIFT_SPARE1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSHIFT_SPARE1;	/* DIP_X_WSHIFT_SPARE1, DIP_A_WSHIFT_SPARE1*/

typedef volatile union _DIP_X_REG_WSHIFT_SPARE2_
{
		volatile struct	/* 0x15024B08 */
		{
				FIELD  WSHIFT_SPARE2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSHIFT_SPARE2;	/* DIP_X_WSHIFT_SPARE2, DIP_A_WSHIFT_SPARE2*/

typedef volatile union _DIP_X_REG_WSHIFT_SPARE3_
{
		volatile struct	/* 0x15024B10 */
		{
				FIELD  WSHIFT_SPARE3                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSHIFT_SPARE3;	/* DIP_X_WSHIFT_SPARE3, DIP_A_WSHIFT_SPARE3*/

typedef volatile union _DIP_X_REG_WSYNC_SPARE0_
{
		volatile struct	/* 0x15024B40 */
		{
				FIELD  WSYNC_SPARE0                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSYNC_SPARE0;	/* DIP_X_WSYNC_SPARE0, DIP_A_WSYNC_SPARE0*/

typedef volatile union _DIP_X_REG_WSYNC_SPARE1_
{
		volatile struct	/* 0x15024B44 */
		{
				FIELD  WSYNC_SPARE1                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSYNC_SPARE1;	/* DIP_X_WSYNC_SPARE1, DIP_A_WSYNC_SPARE1*/

typedef volatile union _DIP_X_REG_WSYNC_SPARE2_
{
		volatile struct	/* 0x15024B48 */
		{
				FIELD  WSYNC_SPARE2                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSYNC_SPARE2;	/* DIP_X_WSYNC_SPARE2, DIP_A_WSYNC_SPARE2*/

typedef volatile union _DIP_X_REG_WSYNC_SPARE3_
{
		volatile struct	/* 0x15024B50 */
		{
				FIELD  WSYNC_SPARE3                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_WSYNC_SPARE3;	/* DIP_X_WSYNC_SPARE3, DIP_A_WSYNC_SPARE3*/

typedef volatile union _DIP_X_REG_MDP_CROP2_X_
{
		volatile struct	/* 0x15024B80 */
		{
				FIELD  MDP_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP2_X;	/* DIP_X_MDP_CROP2_X, DIP_A_MDP_CROP2_X*/

typedef volatile union _DIP_X_REG_MDP_CROP2_Y_
{
		volatile struct	/* 0x15024B84 */
		{
				FIELD  MDP_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP2_Y;	/* DIP_X_MDP_CROP2_Y, DIP_A_MDP_CROP2_Y*/

typedef volatile union _DIP_X_REG_SMX4_CTL_
{
		volatile struct	/* 0x15024BC0 */
		{
				FIELD  SMX_RIGH_DISABLE                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SMX_LEFT_DISABLE                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SMX_CRPOUT_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SMX_CRPINR_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SMX_CRPINL_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SMX_TRANS_UP_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SMX_TRANS_DOWN_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMXO_SEL                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CTL;	/* DIP_X_SMX4_CTL, DIP_A_SMX4_CTL*/

typedef volatile union _DIP_X_REG_SMX4_TRANS_CON_
{
		volatile struct	/* 0x15024BC4 */
		{
				FIELD  SMX_TRANS_PX_NUM                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SMX_TRANS_LN_NUM                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_TRANS_CON;	/* DIP_X_SMX4_TRANS_CON, DIP_A_SMX4_TRANS_CON*/

typedef volatile union _DIP_X_REG_SMX4_SPARE_
{
		volatile struct	/* 0x15024BC8 */
		{
				FIELD  SMX_SPARE                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_SPARE;	/* DIP_X_SMX4_SPARE, DIP_A_SMX4_SPARE*/

typedef volatile union _DIP_X_REG_SMX4_CRPINL_CON1_
{
		volatile struct	/* 0x15024BCC */
		{
				FIELD  SMX_CRPINL_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPINL_CON1;	/* DIP_X_SMX4_CRPINL_CON1, DIP_A_SMX4_CRPINL_CON1*/

typedef volatile union _DIP_X_REG_SMX4_CRPINL_CON2_
{
		volatile struct	/* 0x15024BD0 */
		{
				FIELD  SMX_CRPINL_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPINL_CON2;	/* DIP_X_SMX4_CRPINL_CON2, DIP_A_SMX4_CRPINL_CON2*/

typedef volatile union _DIP_X_REG_SMX4_CRPINR_CON1_
{
		volatile struct	/* 0x15024BD4 */
		{
				FIELD  SMX_CRPINR_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPINR_CON1;	/* DIP_X_SMX4_CRPINR_CON1, DIP_A_SMX4_CRPINR_CON1*/

typedef volatile union _DIP_X_REG_SMX4_CRPINR_CON2_
{
		volatile struct	/* 0x15024BD8 */
		{
				FIELD  SMX_CRPINR_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPINR_CON2;	/* DIP_X_SMX4_CRPINR_CON2, DIP_A_SMX4_CRPINR_CON2*/

typedef volatile union _DIP_X_REG_SMX4_CRPOUT_CON1_
{
		volatile struct	/* 0x15024BDC */
		{
				FIELD  SMX_CRPOUT_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPOUT_CON1;	/* DIP_X_SMX4_CRPOUT_CON1, DIP_A_SMX4_CRPOUT_CON1*/

typedef volatile union _DIP_X_REG_SMX4_CRPOUT_CON2_
{
		volatile struct	/* 0x15024BE0 */
		{
				FIELD  SMX_CRPOUT_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX4_CRPOUT_CON2;	/* DIP_X_SMX4_CRPOUT_CON2, DIP_A_SMX4_CRPOUT_CON2*/

typedef volatile union _DIP_X_REG_C24_TILE_EDGE_
{
		volatile struct	/* 0x15025000 */
		{
				FIELD  C24_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C24_TILE_EDGE;	/* DIP_X_C24_TILE_EDGE, DIP_A_C24_TILE_EDGE*/

typedef volatile union _DIP_X_REG_C42_CON_
{
		volatile struct	/* 0x15025040 */
		{
				FIELD  C42_FILT_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  C42_TDR_EDGE                          :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C42_CON;	/* DIP_X_C42_CON, DIP_A_C42_CON*/

typedef volatile union _DIP_X_REG_C02_CON_
{
		volatile struct	/* 0x15025080 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  C02_INTERP_MODE                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02_CON;	/* DIP_X_C02_CON, DIP_A_C02_CON*/

typedef volatile union _DIP_X_REG_C02_CROP_CON1_
{
		volatile struct	/* 0x15025084 */
		{
				FIELD  C02_CROP_XSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_XEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02_CROP_CON1;	/* DIP_X_C02_CROP_CON1, DIP_A_C02_CROP_CON1*/

typedef volatile union _DIP_X_REG_C02_CROP_CON2_
{
		volatile struct	/* 0x15025088 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02_CROP_CON2;	/* DIP_X_C02_CROP_CON2, DIP_A_C02_CROP_CON2*/

typedef volatile union _DIP_X_REG_C02B_CON_
{
		volatile struct	/* 0x150250C0 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  C02_INTERP_MODE                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02B_CON;	/* DIP_X_C02B_CON, DIP_A_C02B_CON*/

typedef volatile union _DIP_X_REG_C02B_CROP_CON1_
{
		volatile struct	/* 0x150250C4 */
		{
				FIELD  C02_CROP_XSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_XEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02B_CROP_CON1;	/* DIP_X_C02B_CROP_CON1, DIP_A_C02B_CROP_CON1*/

typedef volatile union _DIP_X_REG_C02B_CROP_CON2_
{
		volatile struct	/* 0x150250C8 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02B_CROP_CON2;	/* DIP_X_C02B_CROP_CON2, DIP_A_C02B_CROP_CON2*/

typedef volatile union _DIP_X_REG_G2C_CONV_0A_
{
		volatile struct	/* 0x15025240 */
		{
				FIELD  G2C_CNV_00                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_01                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_0A;	/* DIP_X_G2C_CONV_0A, DIP_A_G2C_CONV_0A*/

typedef volatile union _DIP_X_REG_G2C_CONV_0B_
{
		volatile struct	/* 0x15025244 */
		{
				FIELD  G2C_CNV_02                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_Y_OFST                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_0B;	/* DIP_X_G2C_CONV_0B, DIP_A_G2C_CONV_0B*/

typedef volatile union _DIP_X_REG_G2C_CONV_1A_
{
		volatile struct	/* 0x15025248 */
		{
				FIELD  G2C_CNV_10                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_11                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_1A;	/* DIP_X_G2C_CONV_1A, DIP_A_G2C_CONV_1A*/

typedef volatile union _DIP_X_REG_G2C_CONV_1B_
{
		volatile struct	/* 0x1502524C */
		{
				FIELD  G2C_CNV_12                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_U_OFST                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_1B;	/* DIP_X_G2C_CONV_1B, DIP_A_G2C_CONV_1B*/

typedef volatile union _DIP_X_REG_G2C_CONV_2A_
{
		volatile struct	/* 0x15025250 */
		{
				FIELD  G2C_CNV_20                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_CNV_21                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_2A;	/* DIP_X_G2C_CONV_2A, DIP_A_G2C_CONV_2A*/

typedef volatile union _DIP_X_REG_G2C_CONV_2B_
{
		volatile struct	/* 0x15025254 */
		{
				FIELD  G2C_CNV_22                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  G2C_V_OFST                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CONV_2B;	/* DIP_X_G2C_CONV_2B, DIP_A_G2C_CONV_2B*/

typedef volatile union _DIP_X_REG_G2C_SHADE_CON_1_
{
		volatile struct	/* 0x15025258 */
		{
				FIELD  G2C_SHADE_VAR                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  G2C_SHADE_P0                          : 11;		/* 18..28, 0x1FFC0000 */
				FIELD  G2C_SHADE_EN                          :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_CON_1;	/* DIP_X_G2C_SHADE_CON_1, DIP_A_G2C_SHADE_CON_1*/

typedef volatile union _DIP_X_REG_G2C_SHADE_CON_2_
{
		volatile struct	/* 0x1502525C */
		{
				FIELD  G2C_SHADE_P1                          : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  G2C_SHADE_P2                          : 11;		/* 12..22, 0x007FF000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_CON_2;	/* DIP_X_G2C_SHADE_CON_2, DIP_A_G2C_SHADE_CON_2*/

typedef volatile union _DIP_X_REG_G2C_SHADE_CON_3_
{
		volatile struct	/* 0x15025260 */
		{
				FIELD  G2C_SHADE_UB                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_CON_3;	/* DIP_X_G2C_SHADE_CON_3, DIP_A_G2C_SHADE_CON_3*/

typedef volatile union _DIP_X_REG_G2C_SHADE_TAR_
{
		volatile struct	/* 0x15025264 */
		{
				FIELD  G2C_SHADE_XMID                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  G2C_SHADE_YMID                        : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_TAR;	/* DIP_X_G2C_SHADE_TAR, DIP_A_G2C_SHADE_TAR*/

typedef volatile union _DIP_X_REG_G2C_SHADE_SP_
{
		volatile struct	/* 0x15025268 */
		{
				FIELD  G2C_SHADE_XSP                         : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  G2C_SHADE_YSP                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_SP;	/* DIP_X_G2C_SHADE_SP, DIP_A_G2C_SHADE_SP*/

typedef volatile union _DIP_X_REG_G2C_CFC_CON_1_
{
		volatile struct	/* 0x1502526C */
		{
				FIELD  G2C_CFC_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  G2C_L                                 : 10;		/*  4..13, 0x00003FF0 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  G2C_H                                 : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CFC_CON_1;	/* DIP_X_G2C_CFC_CON_1, DIP_A_G2C_CFC_CON_1*/

typedef volatile union _DIP_X_REG_G2C_CFC_CON_2_
{
		volatile struct	/* 0x15025270 */
		{
				FIELD  G2C_HR                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  G2C_LR                                :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  G2C_HG                                :  5;		/* 10..14, 0x00007C00 */
				FIELD  G2C_LG                                :  5;		/* 15..19, 0x000F8000 */
				FIELD  G2C_HB                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  G2C_LB                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_CFC_CON_2;	/* DIP_X_G2C_CFC_CON_2, DIP_A_G2C_CFC_CON_2*/

typedef volatile union _DIP_X_REG_MIX3_CTRL_0_
{
		volatile struct	/* 0x150252C0 */
		{
				FIELD  MIX3_WT_SEL                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MIX3_Y_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIX3_Y_DEFAULT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MIX3_UV_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MIX3_UV_DEFAULT                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  MIX3_B0                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX3_B1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  MIX3_DT                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX3_CTRL_0;	/* DIP_X_MIX3_CTRL_0, DIP_A_MIX3_CTRL_0*/

typedef volatile union _DIP_X_REG_MIX3_CTRL_1_
{
		volatile struct	/* 0x150252C4 */
		{
				FIELD  MIX3_M0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX3_M1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX3_CTRL_1;	/* DIP_X_MIX3_CTRL_1, DIP_A_MIX3_CTRL_1*/

typedef volatile union _DIP_X_REG_MIX3_SPARE_
{
		volatile struct	/* 0x150252C8 */
		{
				FIELD  MIX3_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX3_SPARE;	/* DIP_X_MIX3_SPARE, DIP_A_MIX3_SPARE*/

typedef volatile union _DIP_X_REG_NBC_ANR_TBL_
{
		volatile struct	/* 0x15025300 ~ 0x150256FF */
		{
				FIELD  NBC_ANR_TBL_Y0                        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  NBC_ANR_TBL_U0                        :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  NBC_ANR_TBL_V0                        :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  NBC_ANR_TBL_Y1                        :  5;		/* 16..20, 0x001F0000 */
				FIELD  NBC_ANR_TBL_U1                        :  5;		/* 21..25, 0x03E00000 */
				FIELD  NBC_ANR_TBL_V1                        :  5;		/* 26..30, 0x7C000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_TBL;	/* DIP_X_NBC_ANR_TBL, DIP_A_NBC_ANR_TBL*/

typedef volatile union _DIP_X_REG_NBC_ANR_CON1_
{
		volatile struct	/* 0x15025700 */
		{
				FIELD  NBC_ANR_ENC                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NBC_ANR_ENY                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  NBC_ANR_Y_FLT0_IDX                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NBC_ANR_Y_FLT1_IDX                    :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  NBC_ANR_Y_FLT2_IDX                    :  2;		/*  8.. 9, 0x00000300 */
				FIELD  NBC_ANR_Y_FLT3_IDX                    :  2;		/* 10..11, 0x00000C00 */
				FIELD  NBC_ANR_ACT_LP_EN                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_TILE_EDGE                     :  4;		/* 16..19, 0x000F0000 */
				FIELD  NBC_ANR_LTM_LINK                      :  1;		/* 20..20, 0x00100000 */
				FIELD  NBC_ANR_LTM_LUT_BYPASS                :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_SL2_LINK                      :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  NBC_ANR_TABLE_EN                      :  1;		/* 28..28, 0x10000000 */
				FIELD  NBC_ANR_TBL_PRC                       :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_CON1;	/* DIP_X_NBC_ANR_CON1, DIP_A_NBC_ANR_CON1*/

typedef volatile union _DIP_X_REG_NBC_ANR_CON2_
{
		volatile struct	/* 0x15025704 */
		{
				FIELD  NBC_ANR_IMPL_MODE                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NBC_ANR_C_SM_EDGE                     :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  NBC_ANR_FLT_C                         :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 11;		/* 13..23, 0x00FFE000 */
				FIELD  NBC_ANR_C_SM_EDGE_TH                  :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_CON2;	/* DIP_X_NBC_ANR_CON2, DIP_A_NBC_ANR_CON2*/

typedef volatile union _DIP_X_REG_NBC_ANR_YAD1_
{
		volatile struct	/* 0x15025708 */
		{
				FIELD  NBC_ANR_CEN_GAIN_LO_TH                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_CEN_GAIN_HI_TH                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_SLOPE_V_TH                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_SLOPE_H_TH                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_YAD1;	/* DIP_X_NBC_ANR_YAD1, DIP_A_NBC_ANR_YAD1*/

typedef volatile union _DIP_X_REG_NBC_ANR_YAD2_
{
		volatile struct	/* 0x1502570C */
		{
				FIELD  NBC_ANR_Y_VERT_ACT_TH                 :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_PTY_GAIN_TH                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_VERT_SIGMA                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_YAD2;	/* DIP_X_NBC_ANR_YAD2, DIP_A_NBC_ANR_YAD2*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT1_
{
		volatile struct	/* 0x15025710 */
		{
				FIELD  NBC_ANR_Y_CPX1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_CPX2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_CPX3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_CPX4                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT1;	/* DIP_X_NBC_ANR_Y4LUT1, DIP_A_NBC_ANR_Y4LUT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT2_
{
		volatile struct	/* 0x15025714 */
		{
				FIELD  NBC_ANR_Y_SCALE_CPY0                  :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_SCALE_CPY1                  :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_SCALE_CPY2                  :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_SCALE_CPY3                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT2;	/* DIP_X_NBC_ANR_Y4LUT2, DIP_A_NBC_ANR_Y4LUT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT3_
{
		volatile struct	/* 0x15025718 */
		{
				FIELD  NBC_ANR_Y_SCALE_SP0                   :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_SCALE_SP1                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_SCALE_SP2                   :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_SCALE_SP3                   :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT3;	/* DIP_X_NBC_ANR_Y4LUT3, DIP_A_NBC_ANR_Y4LUT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_C4LUT1_
{
		volatile struct	/* 0x1502571C */
		{
				FIELD  NBC_ANR_C_CPX1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_C_CPX2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_C_CPX3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_C4LUT1;	/* DIP_X_NBC_ANR_C4LUT1, DIP_A_NBC_ANR_C4LUT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_C4LUT2_
{
		volatile struct	/* 0x15025720 */
		{
				FIELD  NBC_ANR_C_SCALE_CPY0                  :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_C_SCALE_CPY1                  :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_C_SCALE_CPY2                  :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_C_SCALE_CPY3                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_C4LUT2;	/* DIP_X_NBC_ANR_C4LUT2, DIP_A_NBC_ANR_C4LUT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_C4LUT3_
{
		volatile struct	/* 0x15025724 */
		{
				FIELD  NBC_ANR_C_SCALE_SP0                   :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_C_SCALE_SP1                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_C_SCALE_SP2                   :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_C_SCALE_SP3                   :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_C4LUT3;	/* DIP_X_NBC_ANR_C4LUT3, DIP_A_NBC_ANR_C4LUT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_A4LUT2_
{
		volatile struct	/* 0x15025728 */
		{
				FIELD  NBC_ANR_Y_ACT_CPY0                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_ACT_CPY1                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_ACT_CPY2                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_ACT_CPY3                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_A4LUT2;	/* DIP_X_NBC_ANR_A4LUT2, DIP_A_NBC_ANR_A4LUT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_A4LUT3_
{
		volatile struct	/* 0x1502572C */
		{
				FIELD  NBC_ANR_Y_ACT_SP0                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC_ANR_Y_ACT_SP1                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC_ANR_Y_ACT_SP2                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_Y_ACT_SP3                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_A4LUT3;	/* DIP_X_NBC_ANR_A4LUT3, DIP_A_NBC_ANR_A4LUT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_L4LUT1_
{
		volatile struct	/* 0x15025730 */
		{
				FIELD  NBC_ANR_SL2_X1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_SL2_X2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_SL2_X3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_L4LUT1;	/* DIP_X_NBC_ANR_L4LUT1, DIP_A_NBC_ANR_L4LUT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_L4LUT2_
{
		volatile struct	/* 0x15025734 */
		{
				FIELD  NBC_ANR_SL2_GAIN0                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC_ANR_SL2_GAIN1                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC_ANR_SL2_GAIN2                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_SL2_GAIN3                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_L4LUT2;	/* DIP_X_NBC_ANR_L4LUT2, DIP_A_NBC_ANR_L4LUT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_L4LUT3_
{
		volatile struct	/* 0x15025738 */
		{
				FIELD  NBC_ANR_SL2_SP0                       :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC_ANR_SL2_SP1                       :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC_ANR_SL2_SP2                       :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_SL2_SP3                       :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_L4LUT3;	/* DIP_X_NBC_ANR_L4LUT3, DIP_A_NBC_ANR_L4LUT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY0V_
{
		volatile struct	/* 0x1502573C */
		{
				FIELD  NBC_ANR_Y_L0_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L0_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L0_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L0_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY0V;	/* DIP_X_NBC_ANR_PTY0V, DIP_A_NBC_ANR_PTY0V*/

typedef volatile union _DIP_X_REG_NBC_ANR_CAD_
{
		volatile struct	/* 0x15025740 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_PTC_GAIN_TH                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_C_L_DIFF_TH                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_CAD;	/* DIP_X_NBC_ANR_CAD, DIP_A_NBC_ANR_CAD*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY1V_
{
		volatile struct	/* 0x15025744 */
		{
				FIELD  NBC_ANR_Y_L1_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L1_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L1_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L1_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY1V;	/* DIP_X_NBC_ANR_PTY1V, DIP_A_NBC_ANR_PTY1V*/

typedef volatile union _DIP_X_REG_NBC_ANR_SL2_
{
		volatile struct	/* 0x15025748 */
		{
				FIELD  NBC_ANR_SL2_C_GAIN                    :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NBC_ANR_SL2_SCALE_GAIN                :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_SL2;	/* DIP_X_NBC_ANR_SL2, DIP_A_NBC_ANR_SL2*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY2V_
{
		volatile struct	/* 0x1502574C */
		{
				FIELD  NBC_ANR_Y_L2_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L2_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L2_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L2_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY2V;	/* DIP_X_NBC_ANR_PTY2V, DIP_A_NBC_ANR_PTY2V*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY3V_
{
		volatile struct	/* 0x15025750 */
		{
				FIELD  NBC_ANR_Y_L3_V_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L3_V_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L3_V_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L3_V_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY3V;	/* DIP_X_NBC_ANR_PTY3V, DIP_A_NBC_ANR_PTY3V*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY0H_
{
		volatile struct	/* 0x15025754 */
		{
				FIELD  NBC_ANR_Y_L0_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L0_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L0_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L0_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY0H;	/* DIP_X_NBC_ANR_PTY0H, DIP_A_NBC_ANR_PTY0H*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY1H_
{
		volatile struct	/* 0x15025758 */
		{
				FIELD  NBC_ANR_Y_L1_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L1_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L1_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L1_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY1H;	/* DIP_X_NBC_ANR_PTY1H, DIP_A_NBC_ANR_PTY1H*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY2H_
{
		volatile struct	/* 0x1502575C */
		{
				FIELD  NBC_ANR_Y_L2_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L2_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L2_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L2_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY2H;	/* DIP_X_NBC_ANR_PTY2H, DIP_A_NBC_ANR_PTY2H*/

typedef volatile union _DIP_X_REG_NBC_ANR_T4LUT1_
{
		volatile struct	/* 0x15025760 */
		{
				FIELD  NBC_ANR_TBL_CPX1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_TBL_CPX2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_TBL_CPX3                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_T4LUT1;	/* DIP_X_NBC_ANR_T4LUT1, DIP_A_NBC_ANR_T4LUT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_T4LUT2_
{
		volatile struct	/* 0x15025764 */
		{
				FIELD  NBC_ANR_TBL_GAIN_CPY0                 :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_TBL_GAIN_CPY1                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_TBL_GAIN_CPY2                 :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_TBL_GAIN_CPY3                 :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_T4LUT2;	/* DIP_X_NBC_ANR_T4LUT2, DIP_A_NBC_ANR_T4LUT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_T4LUT3_
{
		volatile struct	/* 0x15025768 */
		{
				FIELD  NBC_ANR_TBL_GAIN_SP0                  :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_TBL_GAIN_SP1                  :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_TBL_GAIN_SP2                  :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_TBL_GAIN_SP3                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_T4LUT3;	/* DIP_X_NBC_ANR_T4LUT3, DIP_A_NBC_ANR_T4LUT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_ACT1_
{
		volatile struct	/* 0x1502576C */
		{
				FIELD  NBC_ANR_ACT_SL2_GAIN                  :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  NBC_ANR_ACT_DIF_HI_TH                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  7;		/* 13..19, 0x000FE000 */
				FIELD  NBC_ANR_ACT_DIF_GAIN                  :  4;		/* 20..23, 0x00F00000 */
				FIELD  NBC_ANR_ACT_DIF_LO_TH                 :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_ACT1;	/* DIP_X_NBC_ANR_ACT1, DIP_A_NBC_ANR_ACT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTY3H_
{
		volatile struct	/* 0x15025770 */
		{
				FIELD  NBC_ANR_Y_L3_H_RNG1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_L3_H_RNG2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_L3_H_RNG3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_L3_H_RNG4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTY3H;	/* DIP_X_NBC_ANR_PTY3H, DIP_A_NBC_ANR_PTY3H*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTCV_
{
		volatile struct	/* 0x15025774 */
		{
				FIELD  NBC_ANR_C_V_RNG1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_C_V_RNG2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_C_V_RNG3                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_C_V_RNG4                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTCV;	/* DIP_X_NBC_ANR_PTCV, DIP_A_NBC_ANR_PTCV*/

typedef volatile union _DIP_X_REG_NBC_ANR_ACT4_
{
		volatile struct	/* 0x15025778 */
		{
				FIELD  NBC_ANR_Y_ACT_CEN_OFT                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC_ANR_Y_ACT_CEN_GAIN                :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC_ANR_Y_ACT_CEN_TH                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_ACT4;	/* DIP_X_NBC_ANR_ACT4, DIP_A_NBC_ANR_ACT4*/

typedef volatile union _DIP_X_REG_NBC_ANR_PTCH_
{
		volatile struct	/* 0x1502577C */
		{
				FIELD  NBC_ANR_C_H_RNG1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_C_H_RNG2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_C_H_RNG3                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_C_H_RNG4                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_PTCH;	/* DIP_X_NBC_ANR_PTCH, DIP_A_NBC_ANR_PTCH*/

typedef volatile union _DIP_X_REG_NBC_ANR_YLVL0_
{
		volatile struct	/* 0x15025780 */
		{
				FIELD  NBC_ANR_Y_L0_RNG_RAT_TH               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NBC_ANR_Y_L1_RNG_RAT_TH               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  NBC_ANR_Y_L2_RNG_RAT_TH               :  4;		/*  8..11, 0x00000F00 */
				FIELD  NBC_ANR_Y_L3_RNG_RAT_TH               :  4;		/* 12..15, 0x0000F000 */
				FIELD  NBC_ANR_Y_L0_RNG_RAT_SL               :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  NBC_ANR_Y_L1_RNG_RAT_SL               :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_Y_L2_RNG_RAT_SL               :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  NBC_ANR_Y_L3_RNG_RAT_SL               :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_YLVL0;	/* DIP_X_NBC_ANR_YLVL0, DIP_A_NBC_ANR_YLVL0*/

typedef volatile union _DIP_X_REG_NBC_ANR_YLVL1_
{
		volatile struct	/* 0x15025784 */
		{
				FIELD  NBC_ANR_Y_L0_HF_W                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_L1_HF_W                     :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_L2_HF_W                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_L3_HF_W                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_YLVL1;	/* DIP_X_NBC_ANR_YLVL1, DIP_A_NBC_ANR_YLVL1*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_COR_
{
		volatile struct	/* 0x15025788 */
		{
				FIELD  NBC_ANR_Y_HF_CORE_TH                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_HF_CORE_SL                  :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NBC_ANR_Y_HF_CLIP                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_HF_BAL_MODE                   :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_COR;	/* DIP_X_NBC_ANR_HF_COR, DIP_A_NBC_ANR_HF_COR*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_ACT0_
{
		volatile struct	/* 0x1502578C */
		{
				FIELD  NBC_ANR_Y_HF_ACT_X1                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_HF_ACT_X2                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_HF_ACT_X3                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_HF_ACT_X4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_ACT0;	/* DIP_X_NBC_ANR_HF_ACT0, DIP_A_NBC_ANR_HF_ACT0*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_ACT1_
{
		volatile struct	/* 0x15025790 */
		{
				FIELD  NBC_ANR_Y_HF_ACT_Y0                   :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  NBC_ANR_Y_HF_ACT_Y1                   :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  NBC_ANR_Y_HF_ACT_Y2                   :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC_ANR_Y_HF_ACT_Y3                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_ACT1;	/* DIP_X_NBC_ANR_HF_ACT1, DIP_A_NBC_ANR_HF_ACT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_ACTC_
{
		volatile struct	/* 0x15025794 */
		{
				FIELD  RSV                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_ACT_BLD_BASE_C                :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  NBC_ANR_C_DITH_U                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_C_DITH_V                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_ACTC;	/* DIP_X_NBC_ANR_ACTC, DIP_A_NBC_ANR_ACTC*/

typedef volatile union _DIP_X_REG_NBC_ANR_YLAD_
{
		volatile struct	/* 0x15025798 */
		{
				FIELD  NBC_ANR_CEN_GAIN_LO_TH_LPF            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_CEN_GAIN_HI_TH_LPF            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_YLAD;	/* DIP_X_NBC_ANR_YLAD, DIP_A_NBC_ANR_YLAD*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_ACT2_
{
		volatile struct	/* 0x1502579C */
		{
				FIELD  NBC_ANR_Y_HF_ACT_Y4                   :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  NBC_ANR_Y_HF_ACT_SP4                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_ACT2;	/* DIP_X_NBC_ANR_HF_ACT2, DIP_A_NBC_ANR_HF_ACT2*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_ACT3_
{
		volatile struct	/* 0x150257A0 */
		{
				FIELD  NBC_ANR_Y_HF_ACT_SP0                  :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC_ANR_Y_HF_ACT_SP1                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC_ANR_Y_HF_ACT_SP2                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC_ANR_Y_HF_ACT_SP3                  :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_ACT3;	/* DIP_X_NBC_ANR_HF_ACT3, DIP_A_NBC_ANR_HF_ACT3*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_LUMA0_
{
		volatile struct	/* 0x150257A4 */
		{
				FIELD  NBC_ANR_Y_HF_LUMA_X1                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_HF_LUMA_X2                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_HF_LUMA_Y0                  :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC_ANR_Y_HF_LUMA_Y1                  :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_LUMA0;	/* DIP_X_NBC_ANR_HF_LUMA0, DIP_A_NBC_ANR_HF_LUMA0*/

typedef volatile union _DIP_X_REG_NBC_ANR_HF_LUMA1_
{
		volatile struct	/* 0x150257A8 */
		{
				FIELD  NBC_ANR_Y_HF_LUMA_Y2                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  NBC_ANR_Y_HF_LUMA_SP0                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_HF_LUMA_SP1                 :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_HF_LUMA_SP2                 :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_HF_LUMA1;	/* DIP_X_NBC_ANR_HF_LUMA1, DIP_A_NBC_ANR_HF_LUMA1*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_GAIN1_
{
		volatile struct	/* 0x150257AC */
		{
				FIELD  NBC_ANR_LTM_GAIN_S0                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_GAIN_S1                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_GAIN_S2                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_LTM_GAIN_S3                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_GAIN1;	/* DIP_X_NBC_ANR_LTM_GAIN1, DIP_A_NBC_ANR_LTM_GAIN1*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_GAIN2_
{
		volatile struct	/* 0x150257B0 */
		{
				FIELD  NBC_ANR_LTM_GAIN_S4                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_GAIN_S5                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_GAIN_S6                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_GAIN2;	/* DIP_X_NBC_ANR_LTM_GAIN2, DIP_A_NBC_ANR_LTM_GAIN2*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTP1_
{
		volatile struct	/* 0x150257B4 */
		{
				FIELD  NBC_ANR_LTM_P0_S4                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_P1_S4                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_P50_S4                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_LTM_P250_S4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTP1;	/* DIP_X_NBC_ANR_LTM_LUTP1, DIP_A_NBC_ANR_LTM_LUTP1*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTP2_
{
		volatile struct	/* 0x150257B8 */
		{
				FIELD  NBC_ANR_LTM_P500_S4                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_P750_S4                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_P950_S4                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_LTM_P999_S4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTP2;	/* DIP_X_NBC_ANR_LTM_LUTP2, DIP_A_NBC_ANR_LTM_LUTP2*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTO1_
{
		volatile struct	/* 0x150257BC */
		{
				FIELD  NBC_ANR_LTM_O0_S4                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_O1_S4                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_O50_S4                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_LTM_O250_S4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTO1;	/* DIP_X_NBC_ANR_LTM_LUTO1, DIP_A_NBC_ANR_LTM_LUTO1*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTO2_
{
		volatile struct	/* 0x150257C0 */
		{
				FIELD  NBC_ANR_LTM_O500_S4                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_LTM_O750_S4                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_LTM_O950_S4                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_LTM_O999_S4                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTO2;	/* DIP_X_NBC_ANR_LTM_LUTO2, DIP_A_NBC_ANR_LTM_LUTO2*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTS1_
{
		volatile struct	/* 0x150257C4 */
		{
				FIELD  NBC_ANR_LTM_SP0                       : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_LTM_SP1                       : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTS1;	/* DIP_X_NBC_ANR_LTM_LUTS1, DIP_A_NBC_ANR_LTM_LUTS1*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTS2_
{
		volatile struct	/* 0x150257C8 */
		{
				FIELD  NBC_ANR_LTM_SP2                       : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_LTM_SP3                       : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTS2;	/* DIP_X_NBC_ANR_LTM_LUTS2, DIP_A_NBC_ANR_LTM_LUTS2*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTS3_
{
		volatile struct	/* 0x150257CC */
		{
				FIELD  NBC_ANR_LTM_SP4                       : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_LTM_SP5                       : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTS3;	/* DIP_X_NBC_ANR_LTM_LUTS3, DIP_A_NBC_ANR_LTM_LUTS3*/

typedef volatile union _DIP_X_REG_NBC_ANR_LTM_LUTS4_
{
		volatile struct	/* 0x150257D0 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NBC_ANR_LTM_SP6                       : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_LTM_LUTS4;	/* DIP_X_NBC_ANR_LTM_LUTS4, DIP_A_NBC_ANR_LTM_LUTS4*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT4_
{
		volatile struct	/* 0x150257D4 */
		{
				FIELD  NBC_ANR_Y_CPX5                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_CPX6                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_CPX7                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC_ANR_Y_CPX8                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT4;	/* DIP_X_NBC_ANR_Y4LUT4, DIP_A_NBC_ANR_Y4LUT4*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT5_
{
		volatile struct	/* 0x150257D8 */
		{
				FIELD  NBC_ANR_Y_SCALE_CPY4                  :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_SCALE_CPY5                  :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_SCALE_CPY6                  :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_SCALE_CPY7                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT5;	/* DIP_X_NBC_ANR_Y4LUT5, DIP_A_NBC_ANR_Y4LUT5*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT6_
{
		volatile struct	/* 0x150257DC */
		{
				FIELD  NBC_ANR_Y_SCALE_SP4                   :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_SCALE_SP5                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC_ANR_Y_SCALE_SP6                   :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC_ANR_Y_SCALE_SP7                   :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT6;	/* DIP_X_NBC_ANR_Y4LUT6, DIP_A_NBC_ANR_Y4LUT6*/

typedef volatile union _DIP_X_REG_NBC_ANR_Y4LUT7_
{
		volatile struct	/* 0x150257E0 */
		{
				FIELD  NBC_ANR_Y_SCALE_CPY8                  :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC_ANR_Y_SCALE_SP8                   :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_Y4LUT7;	/* DIP_X_NBC_ANR_Y4LUT7, DIP_A_NBC_ANR_Y4LUT7*/

typedef volatile union _DIP_X_REG_NBC_ANR_A4LUT1_
{
		volatile struct	/* 0x150257E4 */
		{
				FIELD  NBC_ANR_Y_ACT_CPX1                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC_ANR_Y_ACT_CPX2                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC_ANR_Y_ACT_CPX3                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_A4LUT1;	/* DIP_X_NBC_ANR_A4LUT1, DIP_A_NBC_ANR_A4LUT1*/

typedef volatile union _DIP_X_REG_NBC_ANR_RSV1_
{
		volatile struct	/* 0x150257F0 */
		{
				FIELD  NBC_ANR_RSV1                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC_ANR_RSV1;	/* DIP_X_NBC_ANR_RSV1, DIP_A_NBC_ANR_RSV1*/

typedef volatile union _DIP_X_REG_SL2B_CEN_
{
		volatile struct	/* 0x15025800 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_CEN;	/* DIP_X_SL2B_CEN, DIP_A_SL2B_CEN*/

typedef volatile union _DIP_X_REG_SL2B_RR_CON0_
{
		volatile struct	/* 0x15025804 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_RR_CON0;	/* DIP_X_SL2B_RR_CON0, DIP_A_SL2B_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2B_RR_CON1_
{
		volatile struct	/* 0x15025808 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_RR_CON1;	/* DIP_X_SL2B_RR_CON1, DIP_A_SL2B_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2B_GAIN_
{
		volatile struct	/* 0x1502580C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_GAIN;	/* DIP_X_SL2B_GAIN, DIP_A_SL2B_GAIN*/

typedef volatile union _DIP_X_REG_SL2B_RZ_
{
		volatile struct	/* 0x15025810 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_RZ;	/* DIP_X_SL2B_RZ, DIP_A_SL2B_RZ*/

typedef volatile union _DIP_X_REG_SL2B_XOFF_
{
		volatile struct	/* 0x15025814 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_XOFF;	/* DIP_X_SL2B_XOFF, DIP_A_SL2B_XOFF*/

typedef volatile union _DIP_X_REG_SL2B_YOFF_
{
		volatile struct	/* 0x15025818 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_YOFF;	/* DIP_X_SL2B_YOFF, DIP_A_SL2B_YOFF*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON0_
{
		volatile struct	/* 0x1502581C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON0;	/* DIP_X_SL2B_SLP_CON0, DIP_A_SL2B_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON1_
{
		volatile struct	/* 0x15025820 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON1;	/* DIP_X_SL2B_SLP_CON1, DIP_A_SL2B_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON2_
{
		volatile struct	/* 0x15025824 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON2;	/* DIP_X_SL2B_SLP_CON2, DIP_A_SL2B_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON3_
{
		volatile struct	/* 0x15025828 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON3;	/* DIP_X_SL2B_SLP_CON3, DIP_A_SL2B_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2B_SIZE_
{
		volatile struct	/* 0x1502582C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SIZE;	/* DIP_X_SL2B_SIZE, DIP_A_SL2B_SIZE*/

typedef volatile union _DIP_X_REG_NDG_RAN_0_
{
		volatile struct	/* 0x15025840 */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_0;	/* DIP_X_NDG_RAN_0, DIP_A_NDG_RAN_0*/

typedef volatile union _DIP_X_REG_NDG_RAN_1_
{
		volatile struct	/* 0x15025844 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_1;	/* DIP_X_NDG_RAN_1, DIP_A_NDG_RAN_1*/

typedef volatile union _DIP_X_REG_NDG_RAN_2_
{
		volatile struct	/* 0x15025848 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_2;	/* DIP_X_NDG_RAN_2, DIP_A_NDG_RAN_2*/

typedef volatile union _DIP_X_REG_NDG_RAN_3_
{
		volatile struct	/* 0x1502584C */
		{
				FIELD  NDG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_3;	/* DIP_X_NDG_RAN_3, DIP_A_NDG_RAN_3*/

typedef volatile union _DIP_X_REG_NDG_CROP_X_
{
		volatile struct	/* 0x15025850 */
		{
				FIELD  NDG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_CROP_X;	/* DIP_X_NDG_CROP_X, DIP_A_NDG_CROP_X*/

typedef volatile union _DIP_X_REG_NDG_CROP_Y_
{
		volatile struct	/* 0x15025854 */
		{
				FIELD  NDG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_CROP_Y;	/* DIP_X_NDG_CROP_Y, DIP_A_NDG_CROP_Y*/

typedef volatile union _DIP_X_REG_SRZ4_CONTROL_
{
		volatile struct	/* 0x15025880 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_CONTROL;	/* DIP_X_SRZ4_CONTROL, DIP_A_SRZ4_CONTROL*/

typedef volatile union _DIP_X_REG_SRZ4_IN_IMG_
{
		volatile struct	/* 0x15025884 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_IN_IMG;	/* DIP_X_SRZ4_IN_IMG, DIP_A_SRZ4_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ4_OUT_IMG_
{
		volatile struct	/* 0x15025888 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_OUT_IMG;	/* DIP_X_SRZ4_OUT_IMG, DIP_A_SRZ4_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_STEP_
{
		volatile struct	/* 0x1502588C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_STEP;	/* DIP_X_SRZ4_HORI_STEP, DIP_A_SRZ4_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_STEP_
{
		volatile struct	/* 0x15025890 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_STEP;	/* DIP_X_SRZ4_VERT_STEP, DIP_A_SRZ4_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_INT_OFST_
{
		volatile struct	/* 0x15025894 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_INT_OFST;	/* DIP_X_SRZ4_HORI_INT_OFST, DIP_A_SRZ4_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_SUB_OFST_
{
		volatile struct	/* 0x15025898 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_SUB_OFST;	/* DIP_X_SRZ4_HORI_SUB_OFST, DIP_A_SRZ4_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_INT_OFST_
{
		volatile struct	/* 0x1502589C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_INT_OFST;	/* DIP_X_SRZ4_VERT_INT_OFST, DIP_A_SRZ4_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_SUB_OFST_
{
		volatile struct	/* 0x150258A0 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_SUB_OFST;	/* DIP_X_SRZ4_VERT_SUB_OFST, DIP_A_SRZ4_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_MIX1_CTRL_0_
{
		volatile struct	/* 0x150258C0 */
		{
				FIELD  MIX1_WT_SEL                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MIX1_Y_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIX1_Y_DEFAULT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MIX1_UV_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MIX1_UV_DEFAULT                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  MIX1_B0                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX1_B1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  MIX1_DT                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX1_CTRL_0;	/* DIP_X_MIX1_CTRL_0, DIP_A_MIX1_CTRL_0*/

typedef volatile union _DIP_X_REG_MIX1_CTRL_1_
{
		volatile struct	/* 0x150258C4 */
		{
				FIELD  MIX1_M0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX1_M1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX1_CTRL_1;	/* DIP_X_MIX1_CTRL_1, DIP_A_MIX1_CTRL_1*/

typedef volatile union _DIP_X_REG_MIX1_SPARE_
{
		volatile struct	/* 0x150258C8 */
		{
				FIELD  MIX1_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX1_SPARE;	/* DIP_X_MIX1_SPARE, DIP_A_MIX1_SPARE*/

typedef volatile union _DIP_X_REG_SRZ1_CONTROL_
{
		volatile struct	/* 0x15025900 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_CONTROL;	/* DIP_X_SRZ1_CONTROL, DIP_A_SRZ1_CONTROL*/

typedef volatile union _DIP_X_REG_SRZ1_IN_IMG_
{
		volatile struct	/* 0x15025904 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_IN_IMG;	/* DIP_X_SRZ1_IN_IMG, DIP_A_SRZ1_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ1_OUT_IMG_
{
		volatile struct	/* 0x15025908 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_OUT_IMG;	/* DIP_X_SRZ1_OUT_IMG, DIP_A_SRZ1_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_STEP_
{
		volatile struct	/* 0x1502590C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_STEP;	/* DIP_X_SRZ1_HORI_STEP, DIP_A_SRZ1_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_STEP_
{
		volatile struct	/* 0x15025910 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_STEP;	/* DIP_X_SRZ1_VERT_STEP, DIP_A_SRZ1_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_INT_OFST_
{
		volatile struct	/* 0x15025914 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_INT_OFST;	/* DIP_X_SRZ1_HORI_INT_OFST, DIP_A_SRZ1_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_SUB_OFST_
{
		volatile struct	/* 0x15025918 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_SUB_OFST;	/* DIP_X_SRZ1_HORI_SUB_OFST, DIP_A_SRZ1_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_INT_OFST_
{
		volatile struct	/* 0x1502591C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_INT_OFST;	/* DIP_X_SRZ1_VERT_INT_OFST, DIP_A_SRZ1_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_SUB_OFST_
{
		volatile struct	/* 0x15025920 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_SUB_OFST;	/* DIP_X_SRZ1_VERT_SUB_OFST, DIP_A_SRZ1_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_FE_CTRL1_
{
		volatile struct	/* 0x15025940 */
		{
				FIELD  FE_MODE                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  FE_PARAM                              :  7;		/*  2.. 8, 0x000001FC */
				FIELD  FE_FLT_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  FE_TH_G                               :  8;		/* 10..17, 0x0003FC00 */
				FIELD  FE_TH_C                               : 12;		/* 18..29, 0x3FFC0000 */
				FIELD  FE_DSCR_SBIT                          :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_CTRL1;	/* DIP_X_FE_CTRL1, DIP_A_FE_CTRL1*/

typedef volatile union _DIP_X_REG_FE_IDX_CTRL_
{
		volatile struct	/* 0x15025944 */
		{
				FIELD  FE_XIDX                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  FE_YIDX                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_IDX_CTRL;	/* DIP_X_FE_IDX_CTRL, DIP_A_FE_IDX_CTRL*/

typedef volatile union _DIP_X_REG_FE_CROP_CTRL1_
{
		volatile struct	/* 0x15025948 */
		{
				FIELD  FE_START_X                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  FE_START_Y                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_CROP_CTRL1;	/* DIP_X_FE_CROP_CTRL1, DIP_A_FE_CROP_CTRL1*/

typedef volatile union _DIP_X_REG_FE_CROP_CTRL2_
{
		volatile struct	/* 0x1502594C */
		{
				FIELD  FE_IN_WD                              : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  FE_IN_HT                              : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_CROP_CTRL2;	/* DIP_X_FE_CROP_CTRL2, DIP_A_FE_CROP_CTRL2*/

typedef volatile union _DIP_X_REG_FE_CTRL2_
{
		volatile struct	/* 0x15025950 */
		{
				FIELD  FE_TDR_EDGE                           :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_CTRL2;	/* DIP_X_FE_CTRL2, DIP_A_FE_CTRL2*/

typedef volatile union _DIP_X_REG_PCA_TBL_
{
		volatile struct	/* 0x15026000 ~ 0x1502659F */
		{
				FIELD  PCA_LUMA_GAIN                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  PCA_SAT_GAIN                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  PCA_HUE_SHIFT                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_TBL;	/* DIP_X_PCA_TBL, DIP_A_PCA_TBL*/

typedef volatile union _DIP_X_REG_PCA_CON1_
{
		volatile struct	/* 0x15026600 */
		{
				FIELD  PCA_LUT_360                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PCA_Y2YLUT_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PCA_CTRL_BDR                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  PCA_CTRL_OFST                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RSV                                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  PCA_S_TH_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PCA_CFC_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PCA_CNV_EN                            :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PCA_TILE_EDGE                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  PCA_LR                                :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON1;	/* DIP_X_PCA_CON1, DIP_A_PCA_CON1*/

typedef volatile union _DIP_X_REG_PCA_CON2_
{
		volatile struct	/* 0x15026604 */
		{
				FIELD  PCA_C_TH                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PCA_CNV_01                            : 11;		/*  8..18, 0x0007FF00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_CNV_00                            : 11;		/* 20..30, 0x7FF00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON2;	/* DIP_X_PCA_CON2, DIP_A_PCA_CON2*/

typedef volatile union _DIP_X_REG_PCA_CON3_
{
		volatile struct	/* 0x15026608 */
		{
				FIELD  PCA_HG                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PCA_CNV_10                            : 11;		/*  8..18, 0x0007FF00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_CNV_02                            : 11;		/* 20..30, 0x7FF00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON3;	/* DIP_X_PCA_CON3, DIP_A_PCA_CON3*/

typedef volatile union _DIP_X_REG_PCA_CON4_
{
		volatile struct	/* 0x1502660C */
		{
				FIELD  PCA_HR                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PCA_CNV_12                            : 11;		/*  8..18, 0x0007FF00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_CNV_11                            : 11;		/* 20..30, 0x7FF00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON4;	/* DIP_X_PCA_CON4, DIP_A_PCA_CON4*/

typedef volatile union _DIP_X_REG_PCA_CON5_
{
		volatile struct	/* 0x15026610 */
		{
				FIELD  PCA_HB                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PCA_CNV_21                            : 11;		/*  8..18, 0x0007FF00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PCA_CNV_20                            : 11;		/* 20..30, 0x7FF00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON5;	/* DIP_X_PCA_CON5, DIP_A_PCA_CON5*/

typedef volatile union _DIP_X_REG_PCA_CON6_
{
		volatile struct	/* 0x15026614 */
		{
				FIELD  PCA_LB                                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PCA_LG                                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RSV                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  PCA_CNV_22                            : 11;		/* 20..30, 0x7FF00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON6;	/* DIP_X_PCA_CON6, DIP_A_PCA_CON6*/

typedef volatile union _DIP_X_REG_PCA_CON7_
{
		volatile struct	/* 0x15026618 */
		{
				FIELD  PCA_S_TH                              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  PCA_CMAX                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  PCA_H                                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  PCA_L                                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON7;	/* DIP_X_PCA_CON7, DIP_A_PCA_CON7*/

typedef volatile union _DIP_X_REG_MIX2_CTRL_0_
{
		volatile struct	/* 0x15026640 */
		{
				FIELD  MIX2_WT_SEL                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MIX2_Y_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIX2_Y_DEFAULT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MIX2_UV_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MIX2_UV_DEFAULT                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  MIX2_B0                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX2_B1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  MIX2_DT                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX2_CTRL_0;	/* DIP_X_MIX2_CTRL_0, DIP_A_MIX2_CTRL_0*/

typedef volatile union _DIP_X_REG_MIX2_CTRL_1_
{
		volatile struct	/* 0x15026644 */
		{
				FIELD  MIX2_M0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX2_M1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX2_CTRL_1;	/* DIP_X_MIX2_CTRL_1, DIP_A_MIX2_CTRL_1*/

typedef volatile union _DIP_X_REG_MIX2_SPARE_
{
		volatile struct	/* 0x15026648 */
		{
				FIELD  MIX2_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX2_SPARE;	/* DIP_X_MIX2_SPARE, DIP_A_MIX2_SPARE*/

typedef volatile union _DIP_X_REG_SRZ2_CONTROL_
{
		volatile struct	/* 0x15026680 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_CONTROL;	/* DIP_X_SRZ2_CONTROL, DIP_A_SRZ2_CONTROL*/

typedef volatile union _DIP_X_REG_SRZ2_IN_IMG_
{
		volatile struct	/* 0x15026684 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_IN_IMG;	/* DIP_X_SRZ2_IN_IMG, DIP_A_SRZ2_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ2_OUT_IMG_
{
		volatile struct	/* 0x15026688 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_OUT_IMG;	/* DIP_X_SRZ2_OUT_IMG, DIP_A_SRZ2_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_STEP_
{
		volatile struct	/* 0x1502668C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_STEP;	/* DIP_X_SRZ2_HORI_STEP, DIP_A_SRZ2_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_STEP_
{
		volatile struct	/* 0x15026690 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_STEP;	/* DIP_X_SRZ2_VERT_STEP, DIP_A_SRZ2_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_INT_OFST_
{
		volatile struct	/* 0x15026694 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_INT_OFST;	/* DIP_X_SRZ2_HORI_INT_OFST, DIP_A_SRZ2_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_SUB_OFST_
{
		volatile struct	/* 0x15026698 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_SUB_OFST;	/* DIP_X_SRZ2_HORI_SUB_OFST, DIP_A_SRZ2_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_INT_OFST_
{
		volatile struct	/* 0x1502669C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_INT_OFST;	/* DIP_X_SRZ2_VERT_INT_OFST, DIP_A_SRZ2_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_SUB_OFST_
{
		volatile struct	/* 0x150266A0 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_SUB_OFST;	/* DIP_X_SRZ2_VERT_SUB_OFST, DIP_A_SRZ2_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_SEEE_CTRL_
{
		volatile struct	/* 0x150266C0 */
		{
				FIELD  SEEE_TILE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 24;		/*  4..27, 0x0FFFFFF0 */
				FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CTRL;	/* DIP_X_SEEE_CTRL, DIP_A_SEEE_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_TOP_CTRL_
{
		volatile struct	/* 0x150266C4 */
		{
				FIELD  SEEE_OUT_EDGE_SEL                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  SEEE_LP_MODE                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_TOP_CTRL;	/* DIP_X_SEEE_TOP_CTRL, DIP_A_SEEE_TOP_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_BLND_CTRL_1_
{
		volatile struct	/* 0x150266C8 */
		{
				FIELD  SEEE_H1_DI_BLND_OFST                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_H2_DI_BLND_OFST                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_H3_DI_BLND_OFST                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_BLND_CTRL_1;	/* DIP_X_SEEE_BLND_CTRL_1, DIP_A_SEEE_BLND_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_BLND_CTRL_2_
{
		volatile struct	/* 0x150266CC */
		{
				FIELD  SEEE_H1_DI_BLND_SL                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_H2_DI_BLND_SL                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_H3_DI_BLND_SL                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_HX_ISO_BLND_RAT                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_BLND_CTRL_2;	/* DIP_X_SEEE_BLND_CTRL_2, DIP_A_SEEE_BLND_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_CORE_CTRL_
{
		volatile struct	/* 0x150266D0 */
		{
				FIELD  SEEE_H1_FLT_CORE_TH                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_H2_FLT_CORE_TH                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_H3_FLT_CORE_TH                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_FLT_CORE_TH                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CORE_CTRL;	/* DIP_X_SEEE_CORE_CTRL, DIP_A_SEEE_CORE_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_GN_CTRL_1_
{
		volatile struct	/* 0x150266D4 */
		{
				FIELD  SEEE_H1_GN                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SEEE_H2_GN                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEEE_H3_GN                            :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GN_CTRL_1;	/* DIP_X_SEEE_GN_CTRL_1, DIP_A_SEEE_GN_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_1_
{
		volatile struct	/* 0x150266D8 */
		{
				FIELD  SEEE_LUMA_MOD_Y0                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SEEE_LUMA_MOD_Y1                      :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  SEEE_LUMA_MOD_Y2                      :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_1;	/* DIP_X_SEEE_LUMA_CTRL_1, DIP_A_SEEE_LUMA_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_2_
{
		volatile struct	/* 0x150266DC */
		{
				FIELD  SEEE_LUMA_MOD_Y3                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SEEE_LUMA_MOD_Y4                      :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  SEEE_LUMA_MOD_Y5                      :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_2;	/* DIP_X_SEEE_LUMA_CTRL_2, DIP_A_SEEE_LUMA_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_SLNK_CTRL_
{
		volatile struct	/* 0x150266E0 */
		{
				FIELD  SEEE_SLNK_GN_Y1                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_SLNK_GN_Y2                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_RESP_SLNK_GN_RAT                 :  5;		/* 16..20, 0x001F0000 */
				FIELD  SEEE_GLUT_LINK_EN                     :  1;		/* 21..21, 0x00200000 */
				FIELD  SEEE_LUMA_MOD_Y6                      :  9;		/* 22..30, 0x7FC00000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_SLNK_CTRL;	/* DIP_X_SEEE_LUMA_SLNK_CTRL, DIP_A_SEEE_LUMA_SLNK_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_1_
{
		volatile struct	/* 0x150266E4 */
		{
				FIELD  SEEE_GLUT_S1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_X1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y1                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_1;	/* DIP_X_SEEE_GLUT_CTRL_1, DIP_A_SEEE_GLUT_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_2_
{
		volatile struct	/* 0x150266E8 */
		{
				FIELD  SEEE_GLUT_S2                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_X2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y2                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_2;	/* DIP_X_SEEE_GLUT_CTRL_2, DIP_A_SEEE_GLUT_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_3_
{
		volatile struct	/* 0x150266EC */
		{
				FIELD  SEEE_GLUT_S3                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_X3                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y3                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_3;	/* DIP_X_SEEE_GLUT_CTRL_3, DIP_A_SEEE_GLUT_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_4_
{
		volatile struct	/* 0x150266F0 */
		{
				FIELD  SEEE_GLUT_S4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_X4                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y4                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_4;	/* DIP_X_SEEE_GLUT_CTRL_4, DIP_A_SEEE_GLUT_CTRL_4*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_5_
{
		volatile struct	/* 0x150266F4 */
		{
				FIELD  SEEE_GLUT_S5                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_SL_DEC_Y                    : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_5;	/* DIP_X_SEEE_GLUT_CTRL_5, DIP_A_SEEE_GLUT_CTRL_5*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_6_
{
		volatile struct	/* 0x150266F8 */
		{
				FIELD  SEEE_GLUT_TH_OVR                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_TH_UND                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_TH_MIN                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_6;	/* DIP_X_SEEE_GLUT_CTRL_6, DIP_A_SEEE_GLUT_CTRL_6*/

typedef volatile union _DIP_X_REG_SEEE_ARTIFACT_CTRL_
{
		volatile struct	/* 0x150266FC */
		{
				FIELD  SEEE_RESP_SMO_STR                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SEEE_OVRSH_CLIP_STR                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_DOT_REDUC_AMNT                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_DOT_TH                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_ARTIFACT_CTRL;	/* DIP_X_SEEE_ARTIFACT_CTRL, DIP_A_SEEE_ARTIFACT_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_CLIP_CTRL_
{
		volatile struct	/* 0x15026700 */
		{
				FIELD  SEEE_RESP_CLIP                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_RESP_CLIP_LUMA_SPC_TH            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_RESP_CLIP_LUMA_LWB               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_RESP_CLIP_LUMA_UPB               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CLIP_CTRL;	/* DIP_X_SEEE_CLIP_CTRL, DIP_A_SEEE_CLIP_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_GN_CTRL_2_
{
		volatile struct	/* 0x15026704 */
		{
				FIELD  SEEE_MASTER_GN_NEG                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_MASTER_GN_POS                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GN_CTRL_2;	/* DIP_X_SEEE_GN_CTRL_2, DIP_A_SEEE_GN_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_ST_CTRL_1_
{
		volatile struct	/* 0x15026708 */
		{
				FIELD  SEEE_ST_UB                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_ST_LB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_ST_CTRL_1;	/* DIP_X_SEEE_ST_CTRL_1, DIP_A_SEEE_ST_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_ST_CTRL_2_
{
		volatile struct	/* 0x1502670C */
		{
				FIELD  SEEE_ST_SL_CE                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_ST_OFST_CE                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_ST_SL_RESP                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_ST_OFST_RESP                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_ST_CTRL_2;	/* DIP_X_SEEE_ST_CTRL_2, DIP_A_SEEE_ST_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_CT_CTRL_
{
		volatile struct	/* 0x15026710 */
		{
				FIELD  SEEE_LUMA_LMT_DIFF                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_LUMA_CNTST_LV                    :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SEEE_LUMA_MINI                        :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  SEEE_LUMA_MAXI                        :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  SEEE_CHR_CNTST_LV                     :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  SEEE_CHR_MINI                         :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  SEEE_CHR_MAXI                         :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CT_CTRL;	/* DIP_X_SEEE_CT_CTRL, DIP_A_SEEE_CT_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_CBOOST_CTRL_1_
{
		volatile struct	/* 0x15026714 */
		{
				FIELD  SEEE_CBOOST_LMT_U                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_CBOOST_LMT_L                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_CBOOST_GAIN                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_CBOOST_EN                        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CBOOST_CTRL_1;	/* DIP_X_SEEE_CBOOST_CTRL_1, DIP_A_SEEE_CBOOST_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_CBOOST_CTRL_2_
{
		volatile struct	/* 0x15026718 */
		{
				FIELD  SEEE_CBOOST_YCONST                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_CBOOST_YOFFSET                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_CBOOST_YOFFSET_SEL               :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CBOOST_CTRL_2;	/* DIP_X_SEEE_CBOOST_CTRL_2, DIP_A_SEEE_CBOOST_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_PBC1_CTRL_1_
{
		volatile struct	/* 0x1502671C */
		{
				FIELD  SEEE_PBC1_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  SEEE_PBC1_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  SEEE_PBC1_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
				FIELD  SEEE_PBC1_EN                          :  1;		/* 30..30, 0x40000000 */
				FIELD  SEEE_PBC_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC1_CTRL_1;	/* DIP_X_SEEE_PBC1_CTRL_1, DIP_A_SEEE_PBC1_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_PBC1_CTRL_2_
{
		volatile struct	/* 0x15026720 */
		{
				FIELD  SEEE_PBC1_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SEEE_PBC1_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
				FIELD  SEEE_PBC1_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_PBC1_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC1_CTRL_2;	/* DIP_X_SEEE_PBC1_CTRL_2, DIP_A_SEEE_PBC1_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_PBC1_CTRL_3_
{
		volatile struct	/* 0x15026724 */
		{
				FIELD  SEEE_PBC1_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  SEEE_PBC1_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SEEE_PBC1_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_PBC1_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  SEEE_PBC1_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
				FIELD  SEEE_PBC1_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
				FIELD  SEEE_PBC1_GAIN                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC1_CTRL_3;	/* DIP_X_SEEE_PBC1_CTRL_3, DIP_A_SEEE_PBC1_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_PBC2_CTRL_1_
{
		volatile struct	/* 0x15026728 */
		{
				FIELD  SEEE_PBC2_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  SEEE_PBC2_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  SEEE_PBC2_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
				FIELD  SEEE_PBC2_EN                          :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC2_CTRL_1;	/* DIP_X_SEEE_PBC2_CTRL_1, DIP_A_SEEE_PBC2_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_PBC2_CTRL_2_
{
		volatile struct	/* 0x1502672C */
		{
				FIELD  SEEE_PBC2_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SEEE_PBC2_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
				FIELD  SEEE_PBC2_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_PBC2_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC2_CTRL_2;	/* DIP_X_SEEE_PBC2_CTRL_2, DIP_A_SEEE_PBC2_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_PBC2_CTRL_3_
{
		volatile struct	/* 0x15026730 */
		{
				FIELD  SEEE_PBC2_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  SEEE_PBC2_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SEEE_PBC2_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_PBC2_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  SEEE_PBC2_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
				FIELD  SEEE_PBC2_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
				FIELD  SEEE_PBC2_GAIN                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC2_CTRL_3;	/* DIP_X_SEEE_PBC2_CTRL_3, DIP_A_SEEE_PBC2_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_PBC3_CTRL_1_
{
		volatile struct	/* 0x15026734 */
		{
				FIELD  SEEE_PBC3_RADIUS_R                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  SEEE_PBC3_RSLOPE                      : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  SEEE_PBC3_RSLOPE_1                    : 10;		/* 20..29, 0x3FF00000 */
				FIELD  SEEE_PBC3_EN                          :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC3_CTRL_1;	/* DIP_X_SEEE_PBC3_CTRL_1, DIP_A_SEEE_PBC3_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_PBC3_CTRL_2_
{
		volatile struct	/* 0x15026738 */
		{
				FIELD  SEEE_PBC3_TSLOPE                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SEEE_PBC3_THETA_R                     :  6;		/* 10..15, 0x0000FC00 */
				FIELD  SEEE_PBC3_THETA_C                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_PBC3_RADIUS_C                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC3_CTRL_2;	/* DIP_X_SEEE_PBC3_CTRL_2, DIP_A_SEEE_PBC3_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_PBC3_CTRL_3_
{
		volatile struct	/* 0x1502673C */
		{
				FIELD  SEEE_PBC3_LPF_GAIN                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  SEEE_PBC3_LPF_EN                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SEEE_PBC3_EDGE_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_PBC3_EDGE_SLOPE                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  SEEE_PBC3_EDGE_THR                    :  6;		/* 14..19, 0x000FC000 */
				FIELD  SEEE_PBC3_CONF_GAIN                   :  4;		/* 20..23, 0x00F00000 */
				FIELD  SEEE_PBC3_GAIN                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_PBC3_CTRL_3;	/* DIP_X_SEEE_PBC3_CTRL_3, DIP_A_SEEE_PBC3_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_SE_Y_SPECL_CTRL_
{
		volatile struct	/* 0x15026740 */
		{
				FIELD  SEEE_SE_KNEE_SEL                      :  2;		/*  0.. 1, 0x00000003 */
				FIELD  SEEE_SE_SPECL_HALF_MODE               :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SEEE_SE_SPECL_GAIN                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  SEEE_SE_SPECL_INV                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SEEE_SE_SPECL_ABS                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_SE_CONST_Y_EN                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  SEEE_SE_CONST_Y_VAL                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_SE_YOUT_QBIT                     :  4;		/* 24..27, 0x0F000000 */
				FIELD  SEEE_SE_COUT_QBIT                     :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_Y_SPECL_CTRL;	/* DIP_X_SEEE_SE_Y_SPECL_CTRL, DIP_A_SEEE_SE_Y_SPECL_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_SE_EDGE_CTRL_1_
{
		volatile struct	/* 0x15026744 */
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
}DIP_X_REG_SEEE_SE_EDGE_CTRL_1;	/* DIP_X_SEEE_SE_EDGE_CTRL_1, DIP_A_SEEE_SE_EDGE_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_SE_EDGE_CTRL_2_
{
		volatile struct	/* 0x15026748 */
		{
				FIELD  SEEE_SE_CONVT_CORE_TH                 :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_SE_CONVT_E_TH                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_SE_CONVT_SLOPE_SEL               :  1;		/* 16..16, 0x00010000 */
				FIELD  SEEE_SE_CONVT_FORCE_EN                :  1;		/* 17..17, 0x00020000 */
				FIELD  SEEE_SE_EDGE_DET_GAIN                 :  5;		/* 18..22, 0x007C0000 */
				FIELD  SEEE_SE_OIL_EN                        :  1;		/* 23..23, 0x00800000 */
				FIELD  SEEE_SE_BOSS_GAIN_OFF                 :  1;		/* 24..24, 0x01000000 */
				FIELD  SEEE_SE_BOSS_IN_SEL                   :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_EDGE_CTRL_2;	/* DIP_X_SEEE_SE_EDGE_CTRL_2, DIP_A_SEEE_SE_EDGE_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_SE_CORE_CTRL_1_
{
		volatile struct	/* 0x1502674C */
		{
				FIELD  SEEE_SE_CORE_HORI_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_SE_CORE_HORI_X2                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_SE_CORE_HORI_Y5                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  SEEE_SE_CORE_HORI_SUP                 :  2;		/* 22..23, 0x00C00000 */
				FIELD  SEEE_SE_CORE_HORI_SDN                 :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_CORE_CTRL_1;	/* DIP_X_SEEE_SE_CORE_CTRL_1, DIP_A_SEEE_SE_CORE_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_SE_CORE_CTRL_2_
{
		volatile struct	/* 0x15026750 */
		{
				FIELD  SEEE_SE_CORE_VERT_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SEEE_SE_CORE_VERT_X2                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_SE_CORE_VERT_Y5                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  SEEE_SE_CORE_VERT_SUP                 :  2;		/* 22..23, 0x00C00000 */
				FIELD  SEEE_SE_CORE_VERT_SDN                 :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_CORE_CTRL_2;	/* DIP_X_SEEE_SE_CORE_CTRL_2, DIP_A_SEEE_SE_CORE_CTRL_2*/

typedef volatile union _DIP_X_REG_SL2D_CEN_
{
		volatile struct	/* 0x15026780 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_CEN;	/* DIP_X_SL2D_CEN, DIP_A_SL2D_CEN*/

typedef volatile union _DIP_X_REG_SL2D_RR_CON0_
{
		volatile struct	/* 0x15026784 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_RR_CON0;	/* DIP_X_SL2D_RR_CON0, DIP_A_SL2D_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2D_RR_CON1_
{
		volatile struct	/* 0x15026788 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_RR_CON1;	/* DIP_X_SL2D_RR_CON1, DIP_A_SL2D_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2D_GAIN_
{
		volatile struct	/* 0x1502678C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_GAIN;	/* DIP_X_SL2D_GAIN, DIP_A_SL2D_GAIN*/

typedef volatile union _DIP_X_REG_SL2D_RZ_
{
		volatile struct	/* 0x15026790 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_RZ;	/* DIP_X_SL2D_RZ, DIP_A_SL2D_RZ*/

typedef volatile union _DIP_X_REG_SL2D_XOFF_
{
		volatile struct	/* 0x15026794 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_XOFF;	/* DIP_X_SL2D_XOFF, DIP_A_SL2D_XOFF*/

typedef volatile union _DIP_X_REG_SL2D_YOFF_
{
		volatile struct	/* 0x15026798 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_YOFF;	/* DIP_X_SL2D_YOFF, DIP_A_SL2D_YOFF*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON0_
{
		volatile struct	/* 0x1502679C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON0;	/* DIP_X_SL2D_SLP_CON0, DIP_A_SL2D_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON1_
{
		volatile struct	/* 0x150267A0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON1;	/* DIP_X_SL2D_SLP_CON1, DIP_A_SL2D_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON2_
{
		volatile struct	/* 0x150267A4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON2;	/* DIP_X_SL2D_SLP_CON2, DIP_A_SL2D_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON3_
{
		volatile struct	/* 0x150267A8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON3;	/* DIP_X_SL2D_SLP_CON3, DIP_A_SL2D_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2D_SIZE_
{
		volatile struct	/* 0x150267AC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SIZE;	/* DIP_X_SL2D_SIZE, DIP_A_SL2D_SIZE*/

typedef volatile union _DIP_X_REG_SMX2_CTL_
{
		volatile struct	/* 0x150267C0 */
		{
				FIELD  SMX_RIGH_DISABLE                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SMX_LEFT_DISABLE                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SMX_CRPOUT_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SMX_CRPINR_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SMX_CRPINL_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SMX_TRANS_UP_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SMX_TRANS_DOWN_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMXO_SEL                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CTL;	/* DIP_X_SMX2_CTL, DIP_A_SMX2_CTL*/

typedef volatile union _DIP_X_REG_SMX2_TRANS_CON_
{
		volatile struct	/* 0x150267C4 */
		{
				FIELD  SMX_TRANS_PX_NUM                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SMX_TRANS_LN_NUM                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_TRANS_CON;	/* DIP_X_SMX2_TRANS_CON, DIP_A_SMX2_TRANS_CON*/

typedef volatile union _DIP_X_REG_SMX2_SPARE_
{
		volatile struct	/* 0x150267C8 */
		{
				FIELD  SMX_SPARE                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_SPARE;	/* DIP_X_SMX2_SPARE, DIP_A_SMX2_SPARE*/

typedef volatile union _DIP_X_REG_SMX2_CRPINL_CON1_
{
		volatile struct	/* 0x150267CC */
		{
				FIELD  SMX_CRPINL_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPINL_CON1;	/* DIP_X_SMX2_CRPINL_CON1, DIP_A_SMX2_CRPINL_CON1*/

typedef volatile union _DIP_X_REG_SMX2_CRPINL_CON2_
{
		volatile struct	/* 0x150267D0 */
		{
				FIELD  SMX_CRPINL_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPINL_CON2;	/* DIP_X_SMX2_CRPINL_CON2, DIP_A_SMX2_CRPINL_CON2*/

typedef volatile union _DIP_X_REG_SMX2_CRPINR_CON1_
{
		volatile struct	/* 0x150267D4 */
		{
				FIELD  SMX_CRPINR_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPINR_CON1;	/* DIP_X_SMX2_CRPINR_CON1, DIP_A_SMX2_CRPINR_CON1*/

typedef volatile union _DIP_X_REG_SMX2_CRPINR_CON2_
{
		volatile struct	/* 0x150267D8 */
		{
				FIELD  SMX_CRPINR_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPINR_CON2;	/* DIP_X_SMX2_CRPINR_CON2, DIP_A_SMX2_CRPINR_CON2*/

typedef volatile union _DIP_X_REG_SMX2_CRPOUT_CON1_
{
		volatile struct	/* 0x150267DC */
		{
				FIELD  SMX_CRPOUT_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPOUT_CON1;	/* DIP_X_SMX2_CRPOUT_CON1, DIP_A_SMX2_CRPOUT_CON1*/

typedef volatile union _DIP_X_REG_SMX2_CRPOUT_CON2_
{
		volatile struct	/* 0x150267E0 */
		{
				FIELD  SMX_CRPOUT_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX2_CRPOUT_CON2;	/* DIP_X_SMX2_CRPOUT_CON2, DIP_A_SMX2_CRPOUT_CON2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_CON1_
{
		volatile struct	/* 0x15026800 */
		{
				FIELD  NBC2_ANR2_ENC                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NBC2_ANR2_ENY                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  NBC2_ANR2_SCALE_MODE                  :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_ANR2_VER_C_REF_Y                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  NBC2_ANR2_MODE                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC2_ANR2_TILE_EDGE                   :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  NBC2_ANR2_SL2_LINK                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_CON1;	/* DIP_X_NBC2_ANR2_CON1, DIP_A_NBC2_ANR2_CON1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_CON2_
{
		volatile struct	/* 0x15026804 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_C_SM_EDGE                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  NBC2_ANR2_FLT_C                       :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_CON2;	/* DIP_X_NBC2_ANR2_CON2, DIP_A_NBC2_ANR2_CON2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_YAD1_
{
		volatile struct	/* 0x15026808 */
		{
				FIELD  rsv_0                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  NBC2_ANR2_K_TH_C                      :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_YAD1;	/* DIP_X_NBC2_ANR2_YAD1, DIP_A_NBC2_ANR2_YAD1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_Y4LUT1_
{
		volatile struct	/* 0x15026810 */
		{
				FIELD  NBC2_ANR2_Y_CPX1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_Y_CPX2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ANR2_Y_CPX3                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_Y4LUT1;	/* DIP_X_NBC2_ANR2_Y4LUT1, DIP_A_NBC2_ANR2_Y4LUT1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_Y4LUT2_
{
		volatile struct	/* 0x15026814 */
		{
				FIELD  NBC2_ANR2_Y_SCALE_CPY0                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC2_ANR2_Y_SCALE_CPY1                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC2_ANR2_Y_SCALE_CPY2                :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC2_ANR2_Y_SCALE_CPY3                :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_Y4LUT2;	/* DIP_X_NBC2_ANR2_Y4LUT2, DIP_A_NBC2_ANR2_Y4LUT2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_Y4LUT3_
{
		volatile struct	/* 0x15026818 */
		{
				FIELD  NBC2_ANR2_Y_SCALE_SP0                 :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC2_ANR2_Y_SCALE_SP1                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC2_ANR2_Y_SCALE_SP2                 :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC2_ANR2_Y_SCALE_SP3                 :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_Y4LUT3;	/* DIP_X_NBC2_ANR2_Y4LUT3, DIP_A_NBC2_ANR2_Y4LUT3*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_L4LUT1_
{
		volatile struct	/* 0x15026830 */
		{
				FIELD  NBC2_ANR2_SL2_X1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_SL2_X2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ANR2_SL2_X3                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_L4LUT1;	/* DIP_X_NBC2_ANR2_L4LUT1, DIP_A_NBC2_ANR2_L4LUT1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_L4LUT2_
{
		volatile struct	/* 0x15026834 */
		{
				FIELD  NBC2_ANR2_SL2_GAIN0                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_ANR2_SL2_GAIN1                   :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_ANR2_SL2_GAIN2                   :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC2_ANR2_SL2_GAIN3                   :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_L4LUT2;	/* DIP_X_NBC2_ANR2_L4LUT2, DIP_A_NBC2_ANR2_L4LUT2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_L4LUT3_
{
		volatile struct	/* 0x15026838 */
		{
				FIELD  NBC2_ANR2_SL2_SP0                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_ANR2_SL2_SP1                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_ANR2_SL2_SP2                     :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC2_ANR2_SL2_SP3                     :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_L4LUT3;	/* DIP_X_NBC2_ANR2_L4LUT3, DIP_A_NBC2_ANR2_L4LUT3*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_CAD_
{
		volatile struct	/* 0x15026840 */
		{
				FIELD  NBC2_ANR2_PTC_VGAIN                   :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NBC2_ANR2_C_GAIN                      :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  NBC2_ANR2_PTC_GAIN_TH                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  NBC2_ANR2_C_L_DIFF_TH                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_ANR2_C_MODE                      :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_CAD;	/* DIP_X_NBC2_ANR2_CAD, DIP_A_NBC2_ANR2_CAD*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_PTC_
{
		volatile struct	/* 0x15026844 */
		{
				FIELD  NBC2_ANR2_PTC1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_PTC2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ANR2_PTC3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_ANR2_PTC4                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_PTC;	/* DIP_X_NBC2_ANR2_PTC, DIP_A_NBC2_ANR2_PTC*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_SL2_
{
		volatile struct	/* 0x15026848 */
		{
				FIELD  NBC2_ANR2_SL2_C_GAIN                  :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  NBC2_ANR2_LM_WT                       :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_SL2;	/* DIP_X_NBC2_ANR2_SL2, DIP_A_NBC2_ANR2_SL2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_MED1_
{
		volatile struct	/* 0x1502684C */
		{
				FIELD  NBC2_ANR2_COR_TH                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC2_ANR2_COR_SL                      :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  NBC2_ANR2_MCD_TH                      :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  NBC2_ANR2_MCD_SL                      :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_ANR2_LCL_TH                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_MED1;	/* DIP_X_NBC2_ANR2_MED1, DIP_A_NBC2_ANR2_MED1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_MED2_
{
		volatile struct	/* 0x15026850 */
		{
				FIELD  NBC2_ANR2_LCL_SL                      :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  NBC2_ANR2_LCL_LV                      :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  NBC2_ANR2_SCL_TH                      :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  NBC2_ANR2_SCL_SL                      :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_ANR2_SCL_LV                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_MED2;	/* DIP_X_NBC2_ANR2_MED2, DIP_A_NBC2_ANR2_MED2*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_MED3_
{
		volatile struct	/* 0x15026854 */
		{
				FIELD  NBC2_ANR2_NCL_TH                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_NCL_SL                      :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  NBC2_ANR2_NCL_LV                      :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  NBC2_ANR2_VAR                         :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_ANR2_Y0                          :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_MED3;	/* DIP_X_NBC2_ANR2_MED3, DIP_A_NBC2_ANR2_MED3*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_MED4_
{
		volatile struct	/* 0x15026858 */
		{
				FIELD  NBC2_ANR2_Y1                          :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_ANR2_Y2                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_ANR2_Y3                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC2_ANR2_Y4                          :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_MED4;	/* DIP_X_NBC2_ANR2_MED4, DIP_A_NBC2_ANR2_MED4*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_MED5_
{
		volatile struct	/* 0x1502685C */
		{
				FIELD  NBC2_ANR2_LCL_OFT                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_SCL_OFT                     :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_ANR2_NCL_OFT                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_MED5;	/* DIP_X_NBC2_ANR2_MED5, DIP_A_NBC2_ANR2_MED5*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_ACTC_
{
		volatile struct	/* 0x15026874 */
		{
				FIELD  RSV                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ANR2_ACT_BLD_BASE_C              :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  NBC2_ANR2_C_DITH_U                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  NBC2_ANR2_C_DITH_V                    :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_ACTC;	/* DIP_X_NBC2_ANR2_ACTC, DIP_A_NBC2_ANR2_ACTC*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_RSV1_
{
		volatile struct	/* 0x15026878 */
		{
				FIELD  NBC2_ANR2_RSV1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_RSV1;	/* DIP_X_NBC2_ANR2_RSV1, DIP_A_NBC2_ANR2_RSV1*/

typedef volatile union _DIP_X_REG_NBC2_ANR2_RSV2_
{
		volatile struct	/* 0x1502687C */
		{
				FIELD  NBC2_ANR2_RSV2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ANR2_RSV2;	/* DIP_X_NBC2_ANR2_RSV2, DIP_A_NBC2_ANR2_RSV2*/

typedef volatile union _DIP_X_REG_NBC2_CCR_CON_
{
		volatile struct	/* 0x15026880 */
		{
				FIELD  NBC2_CCR_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NBC2_CCR_SL2_LINK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  NBC2_CCR_SL2_MODE                     :  2;		/*  2.. 3, 0x0000000C */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NBC2_CCR_OR_MODE                      :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_CCR_UV_GAIN_MODE                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  NBC2_CCR_UV_GAIN2                     :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_CCR_Y_CPX3                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_CON;	/* DIP_X_NBC2_CCR_CON, DIP_A_NBC2_CCR_CON*/

typedef volatile union _DIP_X_REG_NBC2_CCR_YLUT_
{
		volatile struct	/* 0x15026884 */
		{
				FIELD  NBC2_CCR_Y_CPX1                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_CCR_Y_CPX2                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_CCR_Y_SP1                        :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_CCR_Y_CPY1                       :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_YLUT;	/* DIP_X_NBC2_CCR_YLUT, DIP_A_NBC2_CCR_YLUT*/

typedef volatile union _DIP_X_REG_NBC2_CCR_UVLUT_
{
		volatile struct	/* 0x15026888 */
		{
				FIELD  NBC2_CCR_UV_X1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_CCR_UV_X2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_CCR_UV_X3                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_CCR_UV_GAIN1                     :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_UVLUT;	/* DIP_X_NBC2_CCR_UVLUT, DIP_A_NBC2_CCR_UVLUT*/

typedef volatile union _DIP_X_REG_NBC2_CCR_YLUT2_
{
		volatile struct	/* 0x1502688C */
		{
				FIELD  NBC2_CCR_Y_SP0                        :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  NBC2_CCR_Y_SP2                        :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  NBC2_CCR_Y_CPY0                       :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_CCR_Y_CPY2                       :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_YLUT2;	/* DIP_X_NBC2_CCR_YLUT2, DIP_A_NBC2_CCR_YLUT2*/

typedef volatile union _DIP_X_REG_NBC2_CCR_SAT_CTRL_
{
		volatile struct	/* 0x15026890 */
		{
				FIELD  NBC2_CCR_MODE                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  NBC2_CCR_CEN_U                        :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  NBC2_CCR_CEN_V                        :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_SAT_CTRL;	/* DIP_X_NBC2_CCR_SAT_CTRL, DIP_A_NBC2_CCR_SAT_CTRL*/

typedef volatile union _DIP_X_REG_NBC2_CCR_UVLUT_SP_
{
		volatile struct	/* 0x15026894 */
		{
				FIELD  NBC2_CCR_UV_GAIN_SP1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NBC2_CCR_UV_GAIN_SP2                  : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_UVLUT_SP;	/* DIP_X_NBC2_CCR_UVLUT_SP, DIP_A_NBC2_CCR_UVLUT_SP*/

typedef volatile union _DIP_X_REG_NBC2_CCR_HUE1_
{
		volatile struct	/* 0x15026898 */
		{
				FIELD  NBC2_CCR_HUE_X1                       :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  NBC2_CCR_HUE_X2                       :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_HUE1;	/* DIP_X_NBC2_CCR_HUE1, DIP_A_NBC2_CCR_HUE1*/

typedef volatile union _DIP_X_REG_NBC2_CCR_HUE2_
{
		volatile struct	/* 0x1502689C */
		{
				FIELD  NBC2_CCR_HUE_X3                       :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  NBC2_CCR_HUE_X4                       :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_HUE2;	/* DIP_X_NBC2_CCR_HUE2, DIP_A_NBC2_CCR_HUE2*/

typedef volatile union _DIP_X_REG_NBC2_CCR_HUE3_
{
		volatile struct	/* 0x150268A0 */
		{
				FIELD  NBC2_CCR_HUE_SP1                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_CCR_HUE_SP2                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_CCR_HUE_GAIN1                    :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  NBC2_CCR_HUE_GAIN2                    :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_HUE3;	/* DIP_X_NBC2_CCR_HUE3, DIP_A_NBC2_CCR_HUE3*/

typedef volatile union _DIP_X_REG_NBC2_CCR_L4LUT1_
{
		volatile struct	/* 0x150268A4 */
		{
				FIELD  NBC2_CCR_SL2_X1                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_CCR_SL2_X2                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_CCR_SL2_X3                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_L4LUT1;	/* DIP_X_NBC2_CCR_L4LUT1, DIP_A_NBC2_CCR_L4LUT1*/

typedef volatile union _DIP_X_REG_NBC2_CCR_L4LUT2_
{
		volatile struct	/* 0x150268A8 */
		{
				FIELD  NBC2_CCR_SL2_GAIN0                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_CCR_SL2_GAIN1                    :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_CCR_SL2_GAIN2                    :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC2_CCR_SL2_GAIN3                    :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_L4LUT2;	/* DIP_X_NBC2_CCR_L4LUT2, DIP_A_NBC2_CCR_L4LUT2*/

typedef volatile union _DIP_X_REG_NBC2_CCR_L4LUT3_
{
		volatile struct	/* 0x150268AC */
		{
				FIELD  NBC2_CCR_SL2_SP0                      :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_CCR_SL2_SP1                      :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_CCR_SL2_SP2                      :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  NBC2_CCR_SL2_SP3                      :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_CCR_L4LUT3;	/* DIP_X_NBC2_CCR_L4LUT3, DIP_A_NBC2_CCR_L4LUT3*/

typedef volatile union _DIP_X_REG_NBC2_BOK_CON_
{
		volatile struct	/* 0x150268B0 */
		{
				FIELD  NBC2_BOK_MODE                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  NBC2_BOK_AP_MODE                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NBC2_BOK_FGBG_MODE                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  NBC2_BOK_FGBG_WT                      :  4;		/* 12..15, 0x0000F000 */
				FIELD  NBC2_BOK_PF_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_BOK_CON;	/* DIP_X_NBC2_BOK_CON, DIP_A_NBC2_BOK_CON*/

typedef volatile union _DIP_X_REG_NBC2_BOK_TUN_
{
		volatile struct	/* 0x150268B4 */
		{
				FIELD  NBC2_BOK_STR_WT                       :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  NBC2_BOK_WT_GAIN                      :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NBC2_BOK_INTENSITY                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_BOK_DOF_M                        :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_BOK_TUN;	/* DIP_X_NBC2_BOK_TUN, DIP_A_NBC2_BOK_TUN*/

typedef volatile union _DIP_X_REG_NBC2_BOK_OFF_
{
		volatile struct	/* 0x150268B8 */
		{
				FIELD  NBC2_BOK_XOFF                         :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  NBC2_BOK_YOFF                         :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_BOK_OFF;	/* DIP_X_NBC2_BOK_OFF, DIP_A_NBC2_BOK_OFF*/

typedef volatile union _DIP_X_REG_NBC2_BOK_RSV1_
{
		volatile struct	/* 0x150268BC */
		{
				FIELD  NBC2_BOK_RSV1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_BOK_RSV1;	/* DIP_X_NBC2_BOK_RSV1, DIP_A_NBC2_BOK_RSV1*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CON1_
{
		volatile struct	/* 0x150268C0 */
		{
				FIELD  NBC2_ABF_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  NBC2_ABF_BIL_IDX                      :  2;		/*  8.. 9, 0x00000300 */
				FIELD  NBC2_ABF_NSR_IDX                      :  2;		/* 10..11, 0x00000C00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CON1;	/* DIP_X_NBC2_ABF_CON1, DIP_A_NBC2_ABF_CON1*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CON2_
{
		volatile struct	/* 0x150268C4 */
		{
				FIELD  NBC2_ABF_BF_U_OFST                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CON2;	/* DIP_X_NBC2_ABF_CON2, DIP_A_NBC2_ABF_CON2*/

typedef volatile union _DIP_X_REG_NBC2_ABF_RCON_
{
		volatile struct	/* 0x150268C8 */
		{
				FIELD  NBC2_ABF_R1                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  NBC2_ABF_R2                           :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_RCON;	/* DIP_X_NBC2_ABF_RCON, DIP_A_NBC2_ABF_RCON*/

typedef volatile union _DIP_X_REG_NBC2_ABF_YLUT_
{
		volatile struct	/* 0x150268CC */
		{
				FIELD  NBC2_ABF_Y0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ABF_Y1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ABF_Y2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_ABF_Y3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_YLUT;	/* DIP_X_NBC2_ABF_YLUT, DIP_A_NBC2_ABF_YLUT*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CXLUT_
{
		volatile struct	/* 0x150268D0 */
		{
				FIELD  NBC2_ABF_CX0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ABF_CX1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ABF_CX2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_ABF_CX3                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CXLUT;	/* DIP_X_NBC2_ABF_CXLUT, DIP_A_NBC2_ABF_CXLUT*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CYLUT_
{
		volatile struct	/* 0x150268D4 */
		{
				FIELD  NBC2_ABF_CY0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ABF_CY1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ABF_CY2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  NBC2_ABF_CY3                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CYLUT;	/* DIP_X_NBC2_ABF_CYLUT, DIP_A_NBC2_ABF_CYLUT*/

typedef volatile union _DIP_X_REG_NBC2_ABF_YSP_
{
		volatile struct	/* 0x150268D8 */
		{
				FIELD  NBC2_ABF_Y_SP0                        : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NBC2_ABF_Y_SP1                        : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_YSP;	/* DIP_X_NBC2_ABF_YSP, DIP_A_NBC2_ABF_YSP*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CXSP_
{
		volatile struct	/* 0x150268DC */
		{
				FIELD  NBC2_ABF_CX_SP0                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NBC2_ABF_CX_SP1                       : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CXSP;	/* DIP_X_NBC2_ABF_CXSP, DIP_A_NBC2_ABF_CXSP*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CYSP_
{
		volatile struct	/* 0x150268E0 */
		{
				FIELD  NBC2_ABF_CY_SP0                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NBC2_ABF_CY_SP1                       : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CYSP;	/* DIP_X_NBC2_ABF_CYSP, DIP_A_NBC2_ABF_CYSP*/

typedef volatile union _DIP_X_REG_NBC2_ABF_CLP_
{
		volatile struct	/* 0x150268E4 */
		{
				FIELD  NBC2_ABF_STHRE_R                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NBC2_ABF_STHRE_G                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  NBC2_ABF_STHRE_B                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_CLP;	/* DIP_X_NBC2_ABF_CLP, DIP_A_NBC2_ABF_CLP*/

typedef volatile union _DIP_X_REG_NBC2_ABF_RSV1_
{
		volatile struct	/* 0x150268EC */
		{
				FIELD  NBC2_ABF_RSV                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NBC2_ABF_RSV1;	/* DIP_X_NBC2_ABF_RSV1, DIP_A_NBC2_ABF_RSV1*/

typedef volatile union _DIP_X_REG_SL2C_CEN_
{
		volatile struct	/* 0x15026900 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_CEN;	/* DIP_X_SL2C_CEN, DIP_A_SL2C_CEN*/

typedef volatile union _DIP_X_REG_SL2C_RR_CON0_
{
		volatile struct	/* 0x15026904 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_RR_CON0;	/* DIP_X_SL2C_RR_CON0, DIP_A_SL2C_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2C_RR_CON1_
{
		volatile struct	/* 0x15026908 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_RR_CON1;	/* DIP_X_SL2C_RR_CON1, DIP_A_SL2C_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2C_GAIN_
{
		volatile struct	/* 0x1502690C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_GAIN;	/* DIP_X_SL2C_GAIN, DIP_A_SL2C_GAIN*/

typedef volatile union _DIP_X_REG_SL2C_RZ_
{
		volatile struct	/* 0x15026910 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_RZ;	/* DIP_X_SL2C_RZ, DIP_A_SL2C_RZ*/

typedef volatile union _DIP_X_REG_SL2C_XOFF_
{
		volatile struct	/* 0x15026914 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_XOFF;	/* DIP_X_SL2C_XOFF, DIP_A_SL2C_XOFF*/

typedef volatile union _DIP_X_REG_SL2C_YOFF_
{
		volatile struct	/* 0x15026918 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_YOFF;	/* DIP_X_SL2C_YOFF, DIP_A_SL2C_YOFF*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON0_
{
		volatile struct	/* 0x1502691C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON0;	/* DIP_X_SL2C_SLP_CON0, DIP_A_SL2C_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON1_
{
		volatile struct	/* 0x15026920 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON1;	/* DIP_X_SL2C_SLP_CON1, DIP_A_SL2C_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON2_
{
		volatile struct	/* 0x15026924 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON2;	/* DIP_X_SL2C_SLP_CON2, DIP_A_SL2C_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON3_
{
		volatile struct	/* 0x15026928 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON3;	/* DIP_X_SL2C_SLP_CON3, DIP_A_SL2C_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2C_SIZE_
{
		volatile struct	/* 0x1502692C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SIZE;	/* DIP_X_SL2C_SIZE, DIP_A_SL2C_SIZE*/

typedef volatile union _DIP_X_REG_SRZ3_CONTROL_
{
		volatile struct	/* 0x15026940 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_CONTROL;	/* DIP_X_SRZ3_CONTROL, DIP_A_SRZ3_CONTROL*/

typedef volatile union _DIP_X_REG_SRZ3_IN_IMG_
{
		volatile struct	/* 0x15026944 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_IN_IMG;	/* DIP_X_SRZ3_IN_IMG, DIP_A_SRZ3_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ3_OUT_IMG_
{
		volatile struct	/* 0x15026948 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_OUT_IMG;	/* DIP_X_SRZ3_OUT_IMG, DIP_A_SRZ3_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_STEP_
{
		volatile struct	/* 0x1502694C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_STEP;	/* DIP_X_SRZ3_HORI_STEP, DIP_A_SRZ3_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_STEP_
{
		volatile struct	/* 0x15026950 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_STEP;	/* DIP_X_SRZ3_VERT_STEP, DIP_A_SRZ3_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_INT_OFST_
{
		volatile struct	/* 0x15026954 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_INT_OFST;	/* DIP_X_SRZ3_HORI_INT_OFST, DIP_A_SRZ3_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_SUB_OFST_
{
		volatile struct	/* 0x15026958 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_SUB_OFST;	/* DIP_X_SRZ3_HORI_SUB_OFST, DIP_A_SRZ3_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_INT_OFST_
{
		volatile struct	/* 0x1502695C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_INT_OFST;	/* DIP_X_SRZ3_VERT_INT_OFST, DIP_A_SRZ3_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_SUB_OFST_
{
		volatile struct	/* 0x15026960 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_SUB_OFST;	/* DIP_X_SRZ3_VERT_SUB_OFST, DIP_A_SRZ3_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_NDG2_RAN_0_
{
		volatile struct	/* 0x15026980 */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_0;	/* DIP_X_NDG2_RAN_0, DIP_A_NDG2_RAN_0*/

typedef volatile union _DIP_X_REG_NDG2_RAN_1_
{
		volatile struct	/* 0x15026984 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_1;	/* DIP_X_NDG2_RAN_1, DIP_A_NDG2_RAN_1*/

typedef volatile union _DIP_X_REG_NDG2_RAN_2_
{
		volatile struct	/* 0x15026988 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_2;	/* DIP_X_NDG2_RAN_2, DIP_A_NDG2_RAN_2*/

typedef volatile union _DIP_X_REG_NDG2_RAN_3_
{
		volatile struct	/* 0x1502698C */
		{
				FIELD  NDG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_3;	/* DIP_X_NDG2_RAN_3, DIP_A_NDG2_RAN_3*/

typedef volatile union _DIP_X_REG_NDG2_CROP_X_
{
		volatile struct	/* 0x15026990 */
		{
				FIELD  NDG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_CROP_X;	/* DIP_X_NDG2_CROP_X, DIP_A_NDG2_CROP_X*/

typedef volatile union _DIP_X_REG_NDG2_CROP_Y_
{
		volatile struct	/* 0x15026994 */
		{
				FIELD  NDG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_CROP_Y;	/* DIP_X_NDG2_CROP_Y, DIP_A_NDG2_CROP_Y*/

typedef volatile union _DIP_X_REG_SMX3_CTL_
{
		volatile struct	/* 0x150269C0 */
		{
				FIELD  SMX_RIGH_DISABLE                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SMX_LEFT_DISABLE                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SMX_CRPOUT_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SMX_CRPINR_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SMX_CRPINL_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SMX_TRANS_UP_EN                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SMX_TRANS_DOWN_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SMXO_SEL                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CTL;	/* DIP_X_SMX3_CTL, DIP_A_SMX3_CTL*/

typedef volatile union _DIP_X_REG_SMX3_TRANS_CON_
{
		volatile struct	/* 0x150269C4 */
		{
				FIELD  SMX_TRANS_PX_NUM                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SMX_TRANS_LN_NUM                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_TRANS_CON;	/* DIP_X_SMX3_TRANS_CON, DIP_A_SMX3_TRANS_CON*/

typedef volatile union _DIP_X_REG_SMX3_SPARE_
{
		volatile struct	/* 0x150269C8 */
		{
				FIELD  SMX_SPARE                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_SPARE;	/* DIP_X_SMX3_SPARE, DIP_A_SMX3_SPARE*/

typedef volatile union _DIP_X_REG_SMX3_CRPINL_CON1_
{
		volatile struct	/* 0x150269CC */
		{
				FIELD  SMX_CRPINL_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPINL_CON1;	/* DIP_X_SMX3_CRPINL_CON1, DIP_A_SMX3_CRPINL_CON1*/

typedef volatile union _DIP_X_REG_SMX3_CRPINL_CON2_
{
		volatile struct	/* 0x150269D0 */
		{
				FIELD  SMX_CRPINL_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINL_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPINL_CON2;	/* DIP_X_SMX3_CRPINL_CON2, DIP_A_SMX3_CRPINL_CON2*/

typedef volatile union _DIP_X_REG_SMX3_CRPINR_CON1_
{
		volatile struct	/* 0x150269D4 */
		{
				FIELD  SMX_CRPINR_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPINR_CON1;	/* DIP_X_SMX3_CRPINR_CON1, DIP_A_SMX3_CRPINR_CON1*/

typedef volatile union _DIP_X_REG_SMX3_CRPINR_CON2_
{
		volatile struct	/* 0x150269D8 */
		{
				FIELD  SMX_CRPINR_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPINR_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPINR_CON2;	/* DIP_X_SMX3_CRPINR_CON2, DIP_A_SMX3_CRPINR_CON2*/

typedef volatile union _DIP_X_REG_SMX3_CRPOUT_CON1_
{
		volatile struct	/* 0x150269DC */
		{
				FIELD  SMX_CRPOUT_STR_X                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPOUT_CON1;	/* DIP_X_SMX3_CRPOUT_CON1, DIP_A_SMX3_CRPOUT_CON1*/

typedef volatile union _DIP_X_REG_SMX3_CRPOUT_CON2_
{
		volatile struct	/* 0x150269E0 */
		{
				FIELD  SMX_CRPOUT_STR_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SMX_CRPOUT_END_Y                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SMX3_CRPOUT_CON2;	/* DIP_X_SMX3_CRPOUT_CON2, DIP_A_SMX3_CRPOUT_CON2*/

typedef volatile union _DIP_X_REG_FM_SIZE_
{
		volatile struct	/* 0x15026A00 */
		{
				FIELD  FM_OFFSET_X                           :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  FM_OFFSET_Y                           :  5;		/*  8..12, 0x00001F00 */
				FIELD  FM_SR_TYPE                            :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  FM_WIDTH                              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  FM_HEIGHT                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FM_SIZE;	/* DIP_X_FM_SIZE, DIP_A_FM_SIZE*/

typedef volatile union _DIP_X_REG_FM_TH_CON0_
{
		volatile struct	/* 0x15026A04 */
		{
				FIELD  FM_MIN_RATIO                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  FM_SAD_TH                             : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  FM_RES_TH                             : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FM_TH_CON0;	/* DIP_X_FM_TH_CON0, DIP_A_FM_TH_CON0*/

typedef volatile union _DIP_X_REG_FM_ACC_RES_
{
		volatile struct	/* 0x15026A08 */
		{
				FIELD  FM_ACC_RES                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FM_ACC_RES;	/* DIP_X_FM_ACC_RES, DIP_A_FM_ACC_RES*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CFG_MAIN_
{
		volatile struct	/* 0x15027000 */
		{
				FIELD  COLOR_C2P_BYPASS                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_P2C_BYPASS                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_YENG_BYPASS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  COLOR_SENG_BYPASS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_HENG_BYPASS                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  2;		/*  5.. 6, 0x00000060 */
				FIELD  COLOR_ALL_BYPASS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  COLOR_WIDE_GAMUT_EN                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  4;		/*  9..12, 0x00001E00 */
				FIELD  COLOR_SEQ_SEL                         :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  COLOR_S_G_Y_EN                        :  1;		/* 15..15, 0x00008000 */
				FIELD  COLOR_C_PP_CM_DBG_SEL                 :  4;		/* 16..19, 0x000F0000 */
				FIELD  COLOR_LSP_EN                          :  1;		/* 20..20, 0x00100000 */
				FIELD  COLOR_LSP_SAT_LIMIT                   :  1;		/* 21..21, 0x00200000 */
				FIELD  COLOR_LSP_INK_EN                      :  1;		/* 22..22, 0x00400000 */
				FIELD  COLOR_LSP_SAT_SRC                     :  1;		/* 23..23, 0x00800000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CFG_MAIN;	/* DIP_X_CAM_COLOR_CFG_MAIN, DIP_A_CAM_COLOR_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PXL_CNT_MAIN_
{
		volatile struct	/* 0x15027004 */
		{
				FIELD  COLOR_PIC_H_CNT                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PXL_CNT_MAIN;	/* DIP_X_CAM_COLOR_PXL_CNT_MAIN, DIP_A_CAM_COLOR_PXL_CNT_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN_
{
		volatile struct	/* 0x15027008 */
		{
				FIELD  COLOR_LINE_CNT                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_PIC_V_CNT                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN;	/* DIP_X_CAM_COLOR_LINE_CNT_MAIN, DIP_A_CAM_COLOR_LINE_CNT_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_WIN_X_MAIN_
{
		volatile struct	/* 0x1502700C */
		{
				FIELD  COLOR_WIN_X_START                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_WIN_X_END                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_WIN_X_MAIN;	/* DIP_X_CAM_COLOR_WIN_X_MAIN, DIP_A_CAM_COLOR_WIN_X_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_WIN_Y_MAIN_
{
		volatile struct	/* 0x15027010 */
		{
				FIELD  COLOR_WIN_Y_START                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_WIN_Y_END                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_WIN_Y_MAIN;	/* DIP_X_CAM_COLOR_WIN_Y_MAIN, DIP_A_CAM_COLOR_WIN_Y_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0_
{
		volatile struct	/* 0x15027014 */
		{
				FIELD  COLOR_HTOTAL_CNT                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_VTOTAL_CNT                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0;	/* DIP_X_CAM_COLOR_TIMING_DETECTION_0, DIP_A_CAM_COLOR_TIMING_DETECTION_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1_
{
		volatile struct	/* 0x15027018 */
		{
				FIELD  COLOR_HDE_CNT                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_VDE_CNT                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1;	/* DIP_X_CAM_COLOR_TIMING_DETECTION_1, DIP_A_CAM_COLOR_TIMING_DETECTION_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_DBG_CFG_MAIN_
{
		volatile struct	/* 0x1502701C */
		{
				FIELD  COLOR_INK_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_CAP_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  COLOR_SPLIT_EN                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_SPLIT_SWAP                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  COLOR_Y_INK                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_U_INK                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  COLOR_V_INK                           :  1;		/* 10..10, 0x00000400 */
				FIELD  COLOR_W1_INK_EN                       :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  COLOR_Y_INK_MODE                      :  3;		/* 16..18, 0x00070000 */
				FIELD  COLOR_U_INK_MODE                      :  3;		/* 19..21, 0x00380000 */
				FIELD  COLOR_V_INK_MODE                      :  3;		/* 22..24, 0x01C00000 */
				FIELD  COLOR_Y_INK_SHIFT                     :  2;		/* 25..26, 0x06000000 */
				FIELD  COLOR_U_INK_SHIFT                     :  2;		/* 27..28, 0x18000000 */
				FIELD  COLOR_V_INK_SHIFT                     :  2;		/* 29..30, 0x60000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_DBG_CFG_MAIN;	/* DIP_X_CAM_COLOR_DBG_CFG_MAIN, DIP_A_CAM_COLOR_DBG_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_C_BOOST_MAIN_
{
		volatile struct	/* 0x15027020 */
		{
				FIELD  COLOR_C_BOOST_GAIN                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  5;		/*  8..12, 0x00001F00 */
				FIELD  COLOR_C_NEW_BOOST_EN                  :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  COLOR_C_NEW_BOOST_LMT_L               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_C_NEW_BOOST_LMT_U               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_C_BOOST_MAIN;	/* DIP_X_CAM_COLOR_C_BOOST_MAIN, DIP_A_CAM_COLOR_C_BOOST_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_C_BOOST_MAIN_2_
{
		volatile struct	/* 0x15027024 */
		{
				FIELD  COLOR_COLOR_CBOOST_YOFFSET            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_COLOR_CBOOST_YOFFSET_SEL        :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  COLOR_COLOR_CBOOST_YCONST             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_C_BOOST_MAIN_2;	/* DIP_X_CAM_COLOR_C_BOOST_MAIN_2, DIP_A_CAM_COLOR_C_BOOST_MAIN_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LUMA_ADJ_
{
		volatile struct	/* 0x15027028 */
		{
				FIELD  COLOR_Y_LEVEL_ADJ                     :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  COLOR_Y_SLOPE_LMT                     :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LUMA_ADJ;	/* DIP_X_CAM_COLOR_LUMA_ADJ, DIP_A_CAM_COLOR_LUMA_ADJ*/

typedef volatile union _DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_1_
{
		volatile struct	/* 0x1502702C */
		{
				FIELD  COLOR_G_CONTRAST                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_G_BRIGHTNESS                    : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_1;	/* DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_2_
{
		volatile struct	/* 0x15027030 */
		{
				FIELD  COLOR_G_SATURATION                    : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_2;	/* DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_POS_MAIN_
{
		volatile struct	/* 0x15027034 */
		{
				FIELD  COLOR_POS_X                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_POS_Y                           : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_POS_MAIN;	/* DIP_X_CAM_COLOR_POS_MAIN, DIP_A_CAM_COLOR_POS_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_
{
		volatile struct	/* 0x15027038 */
		{
				FIELD  COLOR_INK_DATA_Y                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_INK_DATA_U                      : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INK_DATA_MAIN;	/* DIP_X_CAM_COLOR_INK_DATA_MAIN, DIP_A_CAM_COLOR_INK_DATA_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_CR_
{
		volatile struct	/* 0x1502703C */
		{
				FIELD  COLOR_INK_DATA_V                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_INK_DATA_MAIN_CR, DIP_A_CAM_COLOR_INK_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_
{
		volatile struct	/* 0x15027040 */
		{
				FIELD  COLOR_CAP_IN_DATA_Y                   : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CAP_IN_DATA_U                   : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN;	/* DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_CR_
{
		volatile struct	/* 0x15027044 */
		{
				FIELD  COLOR_CAP_IN_DATA_V                   : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_
{
		volatile struct	/* 0x15027048 */
		{
				FIELD  COLOR_CAP_OUT_DATA_Y                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CAP_OUT_DATA_U                  : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN;	/* DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_CR_
{
		volatile struct	/* 0x1502704C */
		{
				FIELD  COLOR_CAP_OUT_DATA_V                  : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_1_0_MAIN_
{
		volatile struct	/* 0x15027050 */
		{
				FIELD  COLOR_Y_SLOPE_0                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_1                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_1_0_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_1_0_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_3_2_MAIN_
{
		volatile struct	/* 0x15027054 */
		{
				FIELD  COLOR_Y_SLOPE_2                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_3                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_3_2_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_3_2_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_5_4_MAIN_
{
		volatile struct	/* 0x15027058 */
		{
				FIELD  COLOR_Y_SLOPE_4                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_5                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_5_4_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_5_4_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_7_6_MAIN_
{
		volatile struct	/* 0x1502705C */
		{
				FIELD  COLOR_Y_SLOPE_6                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_7                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_7_6_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_7_6_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_9_8_MAIN_
{
		volatile struct	/* 0x15027060 */
		{
				FIELD  COLOR_Y_SLOPE_8                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_9                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_9_8_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_9_8_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_11_10_MAIN_
{
		volatile struct	/* 0x15027064 */
		{
				FIELD  COLOR_Y_SLOPE_10                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_11                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_11_10_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_11_10_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_13_12_MAIN_
{
		volatile struct	/* 0x15027068 */
		{
				FIELD  COLOR_Y_SLOPE_12                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_13                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_13_12_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_13_12_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_15_14_MAIN_
{
		volatile struct	/* 0x1502706C */
		{
				FIELD  COLOR_Y_SLOPE_14                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_Y_SLOPE_15                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_Y_SLOPE_15_14_MAIN;	/* DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN, DIP_A_CAM_COLOR_Y_SLOPE_15_14_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_0_
{
		volatile struct	/* 0x15027070 */
		{
				FIELD  COLOR_HUE_TO_HUE_0                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_HUE_TO_HUE_1                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_HUE_TO_HUE_2                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_HUE_TO_HUE_3                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_0;	/* DIP_X_CAM_COLOR_LOCAL_HUE_CD_0, DIP_A_CAM_COLOR_LOCAL_HUE_CD_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_1_
{
		volatile struct	/* 0x15027074 */
		{
				FIELD  COLOR_HUE_TO_HUE_4                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_HUE_TO_HUE_5                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_HUE_TO_HUE_6                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_HUE_TO_HUE_7                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_1;	/* DIP_X_CAM_COLOR_LOCAL_HUE_CD_1, DIP_A_CAM_COLOR_LOCAL_HUE_CD_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_2_
{
		volatile struct	/* 0x15027078 */
		{
				FIELD  COLOR_HUE_TO_HUE_8                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_HUE_TO_HUE_9                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_HUE_TO_HUE_10                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_HUE_TO_HUE_11                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_2;	/* DIP_X_CAM_COLOR_LOCAL_HUE_CD_2, DIP_A_CAM_COLOR_LOCAL_HUE_CD_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_3_
{
		volatile struct	/* 0x1502707C */
		{
				FIELD  COLOR_HUE_TO_HUE_12                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_HUE_TO_HUE_13                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_HUE_TO_HUE_14                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_HUE_TO_HUE_15                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_3;	/* DIP_X_CAM_COLOR_LOCAL_HUE_CD_3, DIP_A_CAM_COLOR_LOCAL_HUE_CD_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_4_
{
		volatile struct	/* 0x15027080 */
		{
				FIELD  COLOR_HUE_TO_HUE_16                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_HUE_TO_HUE_17                   :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_HUE_TO_HUE_18                   :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_HUE_TO_HUE_19                   :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_4;	/* DIP_X_CAM_COLOR_LOCAL_HUE_CD_4, DIP_A_CAM_COLOR_LOCAL_HUE_CD_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TWO_D_WINDOW_1_
{
		volatile struct	/* 0x15027084 */
		{
				FIELD  COLOR_W1_HUE_LOWER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_W1_HUE_UPPER                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_W1_SAT_LOWER                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_W1_SAT_UPPER                    :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TWO_D_WINDOW_1;	/* DIP_X_CAM_COLOR_TWO_D_WINDOW_1, DIP_A_CAM_COLOR_TWO_D_WINDOW_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TWO_D_W1_RESULT_
{
		volatile struct	/* 0x15027088 */
		{
				FIELD  COLOR_W1_HIT_OUTER                    : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TWO_D_W1_RESULT;	/* DIP_X_CAM_COLOR_TWO_D_W1_RESULT, DIP_A_CAM_COLOR_TWO_D_W1_RESULT*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN_
{
		volatile struct	/* 0x1502708C */
		{
				FIELD  COLOR_HIST_X_START                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_HIST_X_END                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN;	/* DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN, DIP_A_CAM_COLOR_SAT_HIST_X_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN_
{
		volatile struct	/* 0x15027090 */
		{
				FIELD  COLOR_HIST_Y_START                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_HIST_Y_END                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN;	/* DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN, DIP_A_CAM_COLOR_SAT_HIST_Y_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_BWS_2_
{
		volatile struct	/* 0x15027094 */
		{
				FIELD  rsv_0                                 : 26;		/*  0..25, 0x03FFFFFF */
				FIELD  COLOR_CAM_COLOR_ATPG_OB               :  1;		/* 26..26, 0x04000000 */
				FIELD  COLOR_CAM_COLOR_ATPG_CT               :  1;		/* 27..27, 0x08000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_BWS_2;	/* DIP_X_CAM_COLOR_BWS_2, DIP_A_CAM_COLOR_BWS_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_0_
{
		volatile struct	/* 0x15027098 */
		{
				FIELD  COLOR_CRC_R_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_CRC_L_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_C_CRC_STILL_CHECK_TRIG          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  COLOR_CRC_STILL_CHECK_DONE            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_CRC_NON_STILL_CNT               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  COLOR_CRC_STILL_CHECK_MAX             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CRC_SRC_SEL                     :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_0;	/* DIP_X_CAM_COLOR_CRC_0, DIP_A_CAM_COLOR_CRC_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_1_
{
		volatile struct	/* 0x1502709C */
		{
				FIELD  COLOR_CRC_CLIP_H_START                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  COLOR_CRC_CLIP_H_END                  : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_1;	/* DIP_X_CAM_COLOR_CRC_1, DIP_A_CAM_COLOR_CRC_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_2_
{
		volatile struct	/* 0x150270A0 */
		{
				FIELD  COLOR_CRC_CLIP_V_START                : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  COLOR_CRC_CLIP_V_END                  : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_2;	/* DIP_X_CAM_COLOR_CRC_2, DIP_A_CAM_COLOR_CRC_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_3_
{
		volatile struct	/* 0x150270A4 */
		{
				FIELD  COLOR_CRC_V_MASK                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CRC_C_MASK                      : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CRC_Y_MASK                      : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_3;	/* DIP_X_CAM_COLOR_CRC_3, DIP_A_CAM_COLOR_CRC_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_4_
{
		volatile struct	/* 0x150270A8 */
		{
				FIELD  COLOR_CRC_RESULT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_4;	/* DIP_X_CAM_COLOR_CRC_4, DIP_A_CAM_COLOR_CRC_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_0_
{
		volatile struct	/* 0x150270AC */
		{
				FIELD  COLOR_SAT_GAIN1_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN1_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN1_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN1_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_0;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_1_
{
		volatile struct	/* 0x150270B0 */
		{
				FIELD  COLOR_SAT_GAIN1_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN1_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN1_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN1_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_1;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_2_
{
		volatile struct	/* 0x150270B4 */
		{
				FIELD  COLOR_SAT_GAIN1_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN1_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN1_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN1_HUE_11                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_2;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_3_
{
		volatile struct	/* 0x150270B8 */
		{
				FIELD  COLOR_SAT_GAIN1_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN1_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN1_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN1_HUE_15                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_3;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_4_
{
		volatile struct	/* 0x150270BC */
		{
				FIELD  COLOR_SAT_GAIN1_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN1_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN1_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN1_HUE_19                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_4;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_0_
{
		volatile struct	/* 0x150270C0 */
		{
				FIELD  COLOR_SAT_GAIN2_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN2_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN2_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN2_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_0;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_1_
{
		volatile struct	/* 0x150270C4 */
		{
				FIELD  COLOR_SAT_GAIN2_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN2_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN2_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN2_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_1;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_2_
{
		volatile struct	/* 0x150270C8 */
		{
				FIELD  COLOR_SAT_GAIN2_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN2_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN2_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN2_HUE_11                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_2;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_3_
{
		volatile struct	/* 0x150270CC */
		{
				FIELD  COLOR_SAT_GAIN2_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN2_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN2_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN2_HUE_15                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_3;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_4_
{
		volatile struct	/* 0x150270D0 */
		{
				FIELD  COLOR_SAT_GAIN2_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN2_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN2_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN2_HUE_19                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_4;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_0_
{
		volatile struct	/* 0x150270D4 */
		{
				FIELD  COLOR_SAT_GAIN3_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN3_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN3_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN3_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_0;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_1_
{
		volatile struct	/* 0x150270D8 */
		{
				FIELD  COLOR_SAT_GAIN3_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN3_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN3_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN3_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_1;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_2_
{
		volatile struct	/* 0x150270DC */
		{
				FIELD  COLOR_SAT_GAIN3_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN3_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN3_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN3_HUE_11                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_2;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_3_
{
		volatile struct	/* 0x150270E0 */
		{
				FIELD  COLOR_SAT_GAIN3_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN3_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN3_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN3_HUE_15                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_3;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_4_
{
		volatile struct	/* 0x150270E4 */
		{
				FIELD  COLOR_SAT_GAIN3_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_GAIN3_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_GAIN3_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_GAIN3_HUE_19                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_4;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_0_
{
		volatile struct	/* 0x150270E8 */
		{
				FIELD  COLOR_SAT_POINT1_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT1_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT1_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT1_HUE_3                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_0;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_1_
{
		volatile struct	/* 0x150270EC */
		{
				FIELD  COLOR_SAT_POINT1_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT1_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT1_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT1_HUE_7                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_1;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_2_
{
		volatile struct	/* 0x150270F0 */
		{
				FIELD  COLOR_SAT_POINT1_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT1_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT1_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT1_HUE_11               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_2;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_3_
{
		volatile struct	/* 0x150270F4 */
		{
				FIELD  COLOR_SAT_POINT1_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT1_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT1_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT1_HUE_15               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_3;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_4_
{
		volatile struct	/* 0x150270F8 */
		{
				FIELD  COLOR_SAT_POINT1_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT1_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT1_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT1_HUE_19               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_4;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_0_
{
		volatile struct	/* 0x150270FC */
		{
				FIELD  COLOR_SAT_POINT2_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT2_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT2_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT2_HUE_3                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_0;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_1_
{
		volatile struct	/* 0x15027100 */
		{
				FIELD  COLOR_SAT_POINT2_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT2_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT2_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT2_HUE_7                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_1;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_2_
{
		volatile struct	/* 0x15027104 */
		{
				FIELD  COLOR_SAT_POINT2_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT2_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT2_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT2_HUE_11               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_2;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_3_
{
		volatile struct	/* 0x15027108 */
		{
				FIELD  COLOR_SAT_POINT2_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT2_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT2_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT2_HUE_15               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_3;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_4_
{
		volatile struct	/* 0x1502710C */
		{
				FIELD  COLOR_SAT_POINT2_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_SAT_POINT2_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_SAT_POINT2_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_SAT_POINT2_HUE_19               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_4;	/* DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_START_
{
		volatile struct	/* 0x15027110 */
		{
				FIELD  COLOR_DISP_COLOR_START                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_OUT_SEL              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_DISP_COLOR_WRAP_MODE            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  COLOR_DISP_COLOR_DBUF_VSYNC           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_DIRECT_YUV_BIT_SEL              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  COLOR_STOP_TO_WDMA                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  COLOR_DISP_COLOR_SW_RST_ENGINE        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  COLOR_DISP_COLOR_DBG_SEL              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_START;	/* DIP_X_CAM_COLOR_START, DIP_A_CAM_COLOR_START*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTEN_
{
		volatile struct	/* 0x15027114 */
		{
				FIELD  COLOR_DISP_COLOR_INTEN_FR_COMPLETE    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_INTEN_FR_DONE        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_DISP_COLOR_INTEN_FR_UNDERRUN    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTEN;	/* DIP_X_CAM_COLOR_INTEN, DIP_A_CAM_COLOR_INTEN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTSTA_
{
		volatile struct	/* 0x15027118 */
		{
				FIELD  COLOR_DISP_COLOR_INTSTA_FR_COMPLETE   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_INTSTA_FR_DONE       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_DISP_COLOR_INTSTA_FR_UNDERRUN   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTSTA;	/* DIP_X_CAM_COLOR_INTSTA, DIP_A_CAM_COLOR_INTSTA*/

typedef volatile union _DIP_X_REG_CAM_COLOR_OUT_SEL_
{
		volatile struct	/* 0x1502711C */
		{
				FIELD  COLOR_REG_CHA_SEL                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_REG_CHA_ROUND                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_REG_CHA_UV                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_REG_CHB_SEL                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  COLOR_REG_CHB_ROUND                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  COLOR_REG_CHB_UV                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  COLOR_REG_CHC_SEL                     :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_REG_CHC_ROUND                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  COLOR_REG_CHC_UV                      :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_OUT_SEL;	/* DIP_X_CAM_COLOR_OUT_SEL, DIP_A_CAM_COLOR_OUT_SEL*/

typedef volatile union _DIP_X_REG_CAM_COLOR_FRAME_DONE_DEL_
{
		volatile struct	/* 0x15027120 */
		{
				FIELD  COLOR_DISP_COLOR_FRAME_DONE_DEL       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_FRAME_DONE_DEL;	/* DIP_X_CAM_COLOR_FRAME_DONE_DEL, DIP_A_CAM_COLOR_FRAME_DONE_DEL*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_
{
		volatile struct	/* 0x15027124 */
		{
				FIELD  COLOR_DISP_COLOR_CRC_CEN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_CRC_START            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_DISP_COLOR_CRC_CLR              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC;	/* DIP_X_CAM_COLOR_CRC, DIP_A_CAM_COLOR_CRC*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SW_SCRATCH_
{
		volatile struct	/* 0x15027128 */
		{
				FIELD  COLOR_DISP_COLOR_SW_SCRATCH           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SW_SCRATCH;	/* DIP_X_CAM_COLOR_SW_SCRATCH, DIP_A_CAM_COLOR_SW_SCRATCH*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CK_ON_
{
		volatile struct	/* 0x1502712C */
		{
				FIELD  COLOR_DISP_COLOR_CK_ON                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CK_ON;	/* DIP_X_CAM_COLOR_CK_ON, DIP_A_CAM_COLOR_CK_ON*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH_
{
		volatile struct	/* 0x15027130 */
		{
				FIELD  COLOR_DISP_COLOR_INTERNAL_IP_WIDTH    : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH;	/* DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH, DIP_A_CAM_COLOR_INTERNAL_IP_WIDTH*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT_
{
		volatile struct	/* 0x15027134 */
		{
				FIELD  COLOR_DISP_COLOR_INTERNAL_IP_HEIGHT   : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT;	/* DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT, DIP_A_CAM_COLOR_INTERNAL_IP_HEIGHT*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM1_EN_
{
		volatile struct	/* 0x15027138 */
		{
				FIELD  COLOR_DISP_COLOR_CM1_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_CM1_CLIP_EN          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM1_EN;	/* DIP_X_CAM_COLOR_CM1_EN, DIP_A_CAM_COLOR_CM1_EN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM2_EN_
{
		volatile struct	/* 0x1502713C */
		{
				FIELD  COLOR_DISP_COLOR_CM2_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_CM2_CLIP_SEL         :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_DISP_COLOR_CM2_ROUND_OFF        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM2_EN;	/* DIP_X_CAM_COLOR_CM2_EN, DIP_A_CAM_COLOR_CM2_EN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SHADOW_CTRL_
{
		volatile struct	/* 0x15027140 */
		{
				FIELD  COLOR_BYPASS_SHADOW                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_FORCE_COMMIT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_READ_WRK_REG                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SHADOW_CTRL;	/* DIP_X_CAM_COLOR_SHADOW_CTRL, DIP_A_CAM_COLOR_SHADOW_CTRL*/

typedef volatile union _DIP_X_REG_CAM_COLOR_R0_CRC_
{
		volatile struct	/* 0x15027144 */
		{
				FIELD  COLOR_DISP_COLOR_CRC_OUT_0            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_DISP_COLOR_CRC_RDY_0            :  1;		/* 16..16, 0x00010000 */
				FIELD  COLOR_DISP_COLOR_ENGINE_END           :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_R0_CRC;	/* DIP_X_CAM_COLOR_R0_CRC, DIP_A_CAM_COLOR_R0_CRC*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_0_
{
		volatile struct	/* 0x15027148 */
		{
				FIELD  COLOR_S_GAIN_Y0_HUE_0                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y0_HUE_1                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y0_HUE_2                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y0_HUE_3                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_0;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_1_
{
		volatile struct	/* 0x1502714C */
		{
				FIELD  COLOR_S_GAIN_Y0_HUE_4                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y0_HUE_5                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y0_HUE_6                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y0_HUE_7                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_1;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_2_
{
		volatile struct	/* 0x15027150 */
		{
				FIELD  COLOR_S_GAIN_Y0_HUE_8                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y0_HUE_9                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y0_HUE_10                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y0_HUE_11                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_2;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_3_
{
		volatile struct	/* 0x15027154 */
		{
				FIELD  COLOR_S_GAIN_Y0_HUE_12                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y0_HUE_13                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y0_HUE_14                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y0_HUE_15                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_3;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_4_
{
		volatile struct	/* 0x15027158 */
		{
				FIELD  COLOR_S_GAIN_Y0_HUE_16                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y0_HUE_17                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y0_HUE_18                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y0_HUE_19                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_4;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_0_
{
		volatile struct	/* 0x1502715C */
		{
				FIELD  COLOR_S_GAIN_Y64_HUE_0                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y64_HUE_1                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y64_HUE_2                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y64_HUE_3                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_0;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_1_
{
		volatile struct	/* 0x15027160 */
		{
				FIELD  COLOR_S_GAIN_Y64_HUE_4                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y64_HUE_5                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y64_HUE_6                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y64_HUE_7                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_1;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_2_
{
		volatile struct	/* 0x15027164 */
		{
				FIELD  COLOR_S_GAIN_Y64_HUE_8                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y64_HUE_9                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y64_HUE_10               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y64_HUE_11               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_2;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_3_
{
		volatile struct	/* 0x15027168 */
		{
				FIELD  COLOR_S_GAIN_Y64_HUE_12               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y64_HUE_13               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y64_HUE_14               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y64_HUE_15               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_3;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_4_
{
		volatile struct	/* 0x1502716C */
		{
				FIELD  COLOR_S_GAIN_Y64_HUE_16               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y64_HUE_17               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y64_HUE_18               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y64_HUE_19               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_4;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_0_
{
		volatile struct	/* 0x15027170 */
		{
				FIELD  COLOR_S_GAIN_Y128_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y128_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y128_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y128_HUE_3               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_0;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_1_
{
		volatile struct	/* 0x15027174 */
		{
				FIELD  COLOR_S_GAIN_Y128_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y128_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y128_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y128_HUE_7               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_1;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_2_
{
		volatile struct	/* 0x15027178 */
		{
				FIELD  COLOR_S_GAIN_Y128_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y128_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y128_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y128_HUE_11              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_2;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_3_
{
		volatile struct	/* 0x1502717C */
		{
				FIELD  COLOR_S_GAIN_Y128_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y128_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y128_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y128_HUE_15              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_3;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_4_
{
		volatile struct	/* 0x15027180 */
		{
				FIELD  COLOR_S_GAIN_Y128_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y128_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y128_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y128_HUE_19              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_4;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_0_
{
		volatile struct	/* 0x15027184 */
		{
				FIELD  COLOR_S_GAIN_Y192_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y192_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y192_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y192_HUE_3               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_0;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_1_
{
		volatile struct	/* 0x15027188 */
		{
				FIELD  COLOR_S_GAIN_Y192_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y192_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y192_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y192_HUE_7               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_1;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_2_
{
		volatile struct	/* 0x1502718C */
		{
				FIELD  COLOR_S_GAIN_Y192_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y192_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y192_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y192_HUE_11              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_2;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_3_
{
		volatile struct	/* 0x15027190 */
		{
				FIELD  COLOR_S_GAIN_Y192_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y192_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y192_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y192_HUE_15              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_3;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_4_
{
		volatile struct	/* 0x15027194 */
		{
				FIELD  COLOR_S_GAIN_Y192_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y192_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y192_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y192_HUE_19              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_4;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_0_
{
		volatile struct	/* 0x15027198 */
		{
				FIELD  COLOR_S_GAIN_Y256_HUE_0               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y256_HUE_1               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y256_HUE_2               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y256_HUE_3               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_0;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_1_
{
		volatile struct	/* 0x1502719C */
		{
				FIELD  COLOR_S_GAIN_Y256_HUE_4               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y256_HUE_5               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y256_HUE_6               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y256_HUE_7               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_1;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_2_
{
		volatile struct	/* 0x150271A0 */
		{
				FIELD  COLOR_S_GAIN_Y256_HUE_8               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y256_HUE_9               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y256_HUE_10              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y256_HUE_11              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_2;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_3_
{
		volatile struct	/* 0x150271A4 */
		{
				FIELD  COLOR_S_GAIN_Y256_HUE_12              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y256_HUE_13              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y256_HUE_14              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y256_HUE_15              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_3;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_4_
{
		volatile struct	/* 0x150271A8 */
		{
				FIELD  COLOR_S_GAIN_Y256_HUE_16              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_S_GAIN_Y256_HUE_17              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_S_GAIN_Y256_HUE_18              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_S_GAIN_Y256_HUE_19              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_4;	/* DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LSP_1_
{
		volatile struct	/* 0x150271AC */
		{
				FIELD  COLOR_LSP_LIRE_YSLOPE                 :  7;		/*  0.. 6, 0x0000007F */
				FIELD  COLOR_LSP_LIRE_SSLOPE                 :  7;		/*  7..13, 0x00003F80 */
				FIELD  COLOR_LSP_LIRE_YTH                    :  8;		/* 14..21, 0x003FC000 */
				FIELD  COLOR_LSP_LIRE_STH                    :  7;		/* 22..28, 0x1FC00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LSP_1;	/* DIP_X_CAM_COLOR_LSP_1, DIP_A_CAM_COLOR_LSP_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LSP_2_
{
		volatile struct	/* 0x150271B0 */
		{
				FIELD  COLOR_LSP_LIRE_GAIN                   :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  COLOR_LSP_LSAT_GAIN                   :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  COLOR_LSP_LSAT_STH                    :  7;		/* 16..22, 0x007F0000 */
				FIELD  COLOR_LSP_LSAT_SSLOPE                 :  7;		/* 23..29, 0x3F800000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LSP_2;	/* DIP_X_CAM_COLOR_LSP_2, DIP_A_CAM_COLOR_LSP_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_CONTROL_
{
		volatile struct	/* 0x150271B4 */
		{
				FIELD  COLOR_CM_BYPASS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_CM_W1_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  COLOR_CM_W1_WGT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  COLOR_CM_W1_INK_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  COLOR_CM_W2_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  COLOR_CM_W2_WGT_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  COLOR_CM_W2_INK_EN                    :  1;		/*  6.. 6, 0x00000040 */
				FIELD  COLOR_CM_W3_EN                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  COLOR_CM_W3_WGT_EN                    :  1;		/*  8.. 8, 0x00000100 */
				FIELD  COLOR_CM_W3_INK_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  COLOR_CM_INK_DELTA_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  COLOR_CM_INK_CH                       :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_CONTROL;	/* DIP_X_CAM_COLOR_CM_CONTROL, DIP_A_CAM_COLOR_CM_CONTROL*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_HUE_0_
{
		volatile struct	/* 0x150271B8 */
		{
				FIELD  COLOR_CM_H_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_HUE_0;	/* DIP_X_CAM_COLOR_CM_W1_HUE_0, DIP_A_CAM_COLOR_CM_W1_HUE_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_HUE_1_
{
		volatile struct	/* 0x150271BC */
		{
				FIELD  COLOR_CM_H_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_HUE_1;	/* DIP_X_CAM_COLOR_CM_W1_HUE_1, DIP_A_CAM_COLOR_CM_W1_HUE_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_HUE_2_
{
		volatile struct	/* 0x150271C0 */
		{
				FIELD  COLOR_CM_H_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_H_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_HUE_2;	/* DIP_X_CAM_COLOR_CM_W1_HUE_2, DIP_A_CAM_COLOR_CM_W1_HUE_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_HUE_3_
{
		volatile struct	/* 0x150271C4 */
		{
				FIELD  COLOR_CM_H_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_H_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_H_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_H_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_HUE_3;	/* DIP_X_CAM_COLOR_CM_W1_HUE_3, DIP_A_CAM_COLOR_CM_W1_HUE_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_HUE_4_
{
		volatile struct	/* 0x150271C8 */
		{
				FIELD  COLOR_CM_H_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_H_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_HUE_4;	/* DIP_X_CAM_COLOR_CM_W1_HUE_4, DIP_A_CAM_COLOR_CM_W1_HUE_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_LUMA_0_
{
		volatile struct	/* 0x150271CC */
		{
				FIELD  COLOR_CM_Y_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_LUMA_0;	/* DIP_X_CAM_COLOR_CM_W1_LUMA_0, DIP_A_CAM_COLOR_CM_W1_LUMA_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_LUMA_1_
{
		volatile struct	/* 0x150271D0 */
		{
				FIELD  COLOR_CM_Y_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_LUMA_1;	/* DIP_X_CAM_COLOR_CM_W1_LUMA_1, DIP_A_CAM_COLOR_CM_W1_LUMA_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_LUMA_2_
{
		volatile struct	/* 0x150271D4 */
		{
				FIELD  COLOR_CM_Y_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_Y_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_LUMA_2;	/* DIP_X_CAM_COLOR_CM_W1_LUMA_2, DIP_A_CAM_COLOR_CM_W1_LUMA_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_LUMA_3_
{
		volatile struct	/* 0x150271D8 */
		{
				FIELD  COLOR_CM_Y_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_Y_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_Y_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_Y_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_LUMA_3;	/* DIP_X_CAM_COLOR_CM_W1_LUMA_3, DIP_A_CAM_COLOR_CM_W1_LUMA_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_LUMA_4_
{
		volatile struct	/* 0x150271DC */
		{
				FIELD  COLOR_CM_Y_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_Y_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_LUMA_4;	/* DIP_X_CAM_COLOR_CM_W1_LUMA_4, DIP_A_CAM_COLOR_CM_W1_LUMA_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_SAT_0_
{
		volatile struct	/* 0x150271E0 */
		{
				FIELD  COLOR_CM_S_W1_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W1_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W1_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_SAT_0;	/* DIP_X_CAM_COLOR_CM_W1_SAT_0, DIP_A_CAM_COLOR_CM_W1_SAT_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_SAT_1_
{
		volatile struct	/* 0x150271E4 */
		{
				FIELD  COLOR_CM_S_W1_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W1_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W1_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_SAT_1;	/* DIP_X_CAM_COLOR_CM_W1_SAT_1, DIP_A_CAM_COLOR_CM_W1_SAT_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_SAT_2_
{
		volatile struct	/* 0x150271E8 */
		{
				FIELD  COLOR_CM_S_W1_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W1_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_S_W1_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_SAT_2;	/* DIP_X_CAM_COLOR_CM_W1_SAT_2, DIP_A_CAM_COLOR_CM_W1_SAT_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_SAT_3_
{
		volatile struct	/* 0x150271EC */
		{
				FIELD  COLOR_CM_S_W1_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_S_W1_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_S_W1_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_S_W1_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_SAT_3;	/* DIP_X_CAM_COLOR_CM_W1_SAT_3, DIP_A_CAM_COLOR_CM_W1_SAT_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W1_SAT_4_
{
		volatile struct	/* 0x150271F0 */
		{
				FIELD  COLOR_CM_S_W1_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_S_W1_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W1_SAT_4;	/* DIP_X_CAM_COLOR_CM_W1_SAT_4, DIP_A_CAM_COLOR_CM_W1_SAT_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_HUE_0_
{
		volatile struct	/* 0x150271F4 */
		{
				FIELD  COLOR_CM_H_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_HUE_0;	/* DIP_X_CAM_COLOR_CM_W2_HUE_0, DIP_A_CAM_COLOR_CM_W2_HUE_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_HUE_1_
{
		volatile struct	/* 0x150271F8 */
		{
				FIELD  COLOR_CM_H_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_HUE_1;	/* DIP_X_CAM_COLOR_CM_W2_HUE_1, DIP_A_CAM_COLOR_CM_W2_HUE_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_HUE_2_
{
		volatile struct	/* 0x150271FC */
		{
				FIELD  COLOR_CM_H_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_H_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_HUE_2;	/* DIP_X_CAM_COLOR_CM_W2_HUE_2, DIP_A_CAM_COLOR_CM_W2_HUE_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_HUE_3_
{
		volatile struct	/* 0x15027200 */
		{
				FIELD  COLOR_CM_H_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_H_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_H_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_H_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_HUE_3;	/* DIP_X_CAM_COLOR_CM_W2_HUE_3, DIP_A_CAM_COLOR_CM_W2_HUE_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_HUE_4_
{
		volatile struct	/* 0x15027204 */
		{
				FIELD  COLOR_CM_H_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_H_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_HUE_4;	/* DIP_X_CAM_COLOR_CM_W2_HUE_4, DIP_A_CAM_COLOR_CM_W2_HUE_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_LUMA_0_
{
		volatile struct	/* 0x15027208 */
		{
				FIELD  COLOR_CM_Y_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_LUMA_0;	/* DIP_X_CAM_COLOR_CM_W2_LUMA_0, DIP_A_CAM_COLOR_CM_W2_LUMA_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_LUMA_1_
{
		volatile struct	/* 0x1502720C */
		{
				FIELD  COLOR_CM_Y_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_LUMA_1;	/* DIP_X_CAM_COLOR_CM_W2_LUMA_1, DIP_A_CAM_COLOR_CM_W2_LUMA_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_LUMA_2_
{
		volatile struct	/* 0x15027210 */
		{
				FIELD  COLOR_CM_Y_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_Y_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_LUMA_2;	/* DIP_X_CAM_COLOR_CM_W2_LUMA_2, DIP_A_CAM_COLOR_CM_W2_LUMA_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_LUMA_3_
{
		volatile struct	/* 0x15027214 */
		{
				FIELD  COLOR_CM_Y_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_Y_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_Y_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_Y_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_LUMA_3;	/* DIP_X_CAM_COLOR_CM_W2_LUMA_3, DIP_A_CAM_COLOR_CM_W2_LUMA_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_LUMA_4_
{
		volatile struct	/* 0x15027218 */
		{
				FIELD  COLOR_CM_Y_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_Y_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_LUMA_4;	/* DIP_X_CAM_COLOR_CM_W2_LUMA_4, DIP_A_CAM_COLOR_CM_W2_LUMA_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_SAT_0_
{
		volatile struct	/* 0x1502721C */
		{
				FIELD  COLOR_CM_S_W2_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W2_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W2_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_SAT_0;	/* DIP_X_CAM_COLOR_CM_W2_SAT_0, DIP_A_CAM_COLOR_CM_W2_SAT_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_SAT_1_
{
		volatile struct	/* 0x15027220 */
		{
				FIELD  COLOR_CM_S_W2_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W2_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W2_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_SAT_1;	/* DIP_X_CAM_COLOR_CM_W2_SAT_1, DIP_A_CAM_COLOR_CM_W2_SAT_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_SAT_2_
{
		volatile struct	/* 0x15027224 */
		{
				FIELD  COLOR_CM_S_W2_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W2_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_S_W2_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_SAT_2;	/* DIP_X_CAM_COLOR_CM_W2_SAT_2, DIP_A_CAM_COLOR_CM_W2_SAT_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_SAT_3_
{
		volatile struct	/* 0x15027228 */
		{
				FIELD  COLOR_CM_S_W2_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_S_W2_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_S_W2_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_S_W2_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_SAT_3;	/* DIP_X_CAM_COLOR_CM_W2_SAT_3, DIP_A_CAM_COLOR_CM_W2_SAT_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W2_SAT_4_
{
		volatile struct	/* 0x1502722C */
		{
				FIELD  COLOR_CM_S_W2_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_S_W2_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W2_SAT_4;	/* DIP_X_CAM_COLOR_CM_W2_SAT_4, DIP_A_CAM_COLOR_CM_W2_SAT_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_HUE_0_
{
		volatile struct	/* 0x15027230 */
		{
				FIELD  COLOR_CM_H_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_HUE_0;	/* DIP_X_CAM_COLOR_CM_W3_HUE_0, DIP_A_CAM_COLOR_CM_W3_HUE_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_HUE_1_
{
		volatile struct	/* 0x15027234 */
		{
				FIELD  COLOR_CM_H_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_H_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_HUE_1;	/* DIP_X_CAM_COLOR_CM_W3_HUE_1, DIP_A_CAM_COLOR_CM_W3_HUE_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_HUE_2_
{
		volatile struct	/* 0x15027238 */
		{
				FIELD  COLOR_CM_H_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_H_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_H_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_HUE_2;	/* DIP_X_CAM_COLOR_CM_W3_HUE_2, DIP_A_CAM_COLOR_CM_W3_HUE_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_HUE_3_
{
		volatile struct	/* 0x1502723C */
		{
				FIELD  COLOR_CM_H_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_H_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_H_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_H_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_HUE_3;	/* DIP_X_CAM_COLOR_CM_W3_HUE_3, DIP_A_CAM_COLOR_CM_W3_HUE_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_HUE_4_
{
		volatile struct	/* 0x15027240 */
		{
				FIELD  COLOR_CM_H_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_H_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_HUE_4;	/* DIP_X_CAM_COLOR_CM_W3_HUE_4, DIP_A_CAM_COLOR_CM_W3_HUE_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_LUMA_0_
{
		volatile struct	/* 0x15027244 */
		{
				FIELD  COLOR_CM_Y_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_LUMA_0;	/* DIP_X_CAM_COLOR_CM_W3_LUMA_0, DIP_A_CAM_COLOR_CM_W3_LUMA_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_LUMA_1_
{
		volatile struct	/* 0x15027248 */
		{
				FIELD  COLOR_CM_Y_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_Y_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_LUMA_1;	/* DIP_X_CAM_COLOR_CM_W3_LUMA_1, DIP_A_CAM_COLOR_CM_W3_LUMA_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_LUMA_2_
{
		volatile struct	/* 0x1502724C */
		{
				FIELD  COLOR_CM_Y_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_Y_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_Y_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_LUMA_2;	/* DIP_X_CAM_COLOR_CM_W3_LUMA_2, DIP_A_CAM_COLOR_CM_W3_LUMA_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_LUMA_3_
{
		volatile struct	/* 0x15027250 */
		{
				FIELD  COLOR_CM_Y_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_Y_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_Y_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_Y_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_LUMA_3;	/* DIP_X_CAM_COLOR_CM_W3_LUMA_3, DIP_A_CAM_COLOR_CM_W3_LUMA_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_LUMA_4_
{
		volatile struct	/* 0x15027254 */
		{
				FIELD  COLOR_CM_Y_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_Y_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_LUMA_4;	/* DIP_X_CAM_COLOR_CM_W3_LUMA_4, DIP_A_CAM_COLOR_CM_W3_LUMA_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_SAT_0_
{
		volatile struct	/* 0x15027258 */
		{
				FIELD  COLOR_CM_S_W3_L                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W3_U                       : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W3_POINT0                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_0;	/* DIP_X_CAM_COLOR_CM_W3_SAT_0, DIP_A_CAM_COLOR_CM_W3_SAT_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_SAT_1_
{
		volatile struct	/* 0x1502725C */
		{
				FIELD  COLOR_CM_S_W3_POINT1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W3_POINT2                  : 10;		/* 10..19, 0x000FFC00 */
				FIELD  COLOR_CM_S_W3_POINT3                  : 10;		/* 20..29, 0x3FF00000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_1;	/* DIP_X_CAM_COLOR_CM_W3_SAT_1, DIP_A_CAM_COLOR_CM_W3_SAT_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_SAT_2_
{
		volatile struct	/* 0x15027260 */
		{
				FIELD  COLOR_CM_S_W3_POINT4                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  COLOR_CM_S_W3_SLOPE0                  :  8;		/* 10..17, 0x0003FC00 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  COLOR_CM_S_W3_SLOPE1                  :  8;		/* 20..27, 0x0FF00000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_2;	/* DIP_X_CAM_COLOR_CM_W3_SAT_2, DIP_A_CAM_COLOR_CM_W3_SAT_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_SAT_3_
{
		volatile struct	/* 0x15027264 */
		{
				FIELD  COLOR_CM_S_W3_SLOPE2                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  COLOR_CM_S_W3_SLOPE3                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  COLOR_CM_S_W3_SLOPE4                  :  8;		/* 16..23, 0x00FF0000 */
				FIELD  COLOR_CM_S_W3_SLOPE5                  :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_3;	/* DIP_X_CAM_COLOR_CM_W3_SAT_3, DIP_A_CAM_COLOR_CM_W3_SAT_3*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM_W3_SAT_4_
{
		volatile struct	/* 0x15027268 */
		{
				FIELD  COLOR_CM_S_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_S_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_4;	/* DIP_X_CAM_COLOR_CM_W3_SAT_4, DIP_A_CAM_COLOR_CM_W3_SAT_4*/

typedef volatile union _DIP_X_REG_HFG_CON_0_
{
		volatile struct	/* 0x15027280 */
		{
				FIELD  DIP_HFC_STD                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  HFC_LCE_LINK_EN                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CON_0;	/* DIP_X_HFG_CON_0, DIP_A_HFG_CON_0*/

typedef volatile union _DIP_X_REG_HFG_LUMA_0_
{
		volatile struct	/* 0x15027284 */
		{
				FIELD  HFC_LUMA_CPX1                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  HFC_LUMA_CPX2                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  HFC_LUMA_CPX3                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LUMA_0;	/* DIP_X_HFG_LUMA_0, DIP_A_HFG_LUMA_0*/

typedef volatile union _DIP_X_REG_HFG_LUMA_1_
{
		volatile struct	/* 0x15027288 */
		{
				FIELD  HFC_LUMA_CPY0                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  HFC_LUMA_CPY1                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  HFC_LUMA_CPY2                         :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  HFC_LUMA_CPY3                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LUMA_1;	/* DIP_X_HFG_LUMA_1, DIP_A_HFG_LUMA_1*/

typedef volatile union _DIP_X_REG_HFG_LUMA_2_
{
		volatile struct	/* 0x1502728C */
		{
				FIELD  HFC_LUMA_SP0                          :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  HFC_LUMA_SP1                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HFC_LUMA_SP2                          :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  HFC_LUMA_SP3                          :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LUMA_2;	/* DIP_X_HFG_LUMA_2, DIP_A_HFG_LUMA_2*/

typedef volatile union _DIP_X_REG_HFG_LCE_0_
{
		volatile struct	/* 0x15027290 */
		{
				FIELD  HFC_LCE_CPX1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  HFC_LCE_CPX2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  HFC_LCE_CPX3                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LCE_0;	/* DIP_X_HFG_LCE_0, DIP_A_HFG_LCE_0*/

typedef volatile union _DIP_X_REG_HFG_LCE_1_
{
		volatile struct	/* 0x15027294 */
		{
				FIELD  HFC_LCE_CPY0                          :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  HFC_LCE_CPY1                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  HFC_LCE_CPY2                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  HFC_LCE_CPY3                          :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LCE_1;	/* DIP_X_HFG_LCE_1, DIP_A_HFG_LCE_1*/

typedef volatile union _DIP_X_REG_HFG_LCE_2_
{
		volatile struct	/* 0x15027298 */
		{
				FIELD  HFC_LCE_SP0                           :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  HFC_LCE_SP1                           :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HFC_LCE_SP2                           :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  HFC_LCE_SP3                           :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_LCE_2;	/* DIP_X_HFG_LCE_2, DIP_A_HFG_LCE_2*/

typedef volatile union _DIP_X_REG_HFG_RAN_0_
{
		volatile struct	/* 0x1502729C */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_0;	/* DIP_X_HFG_RAN_0, DIP_A_HFG_RAN_0*/

typedef volatile union _DIP_X_REG_HFG_RAN_1_
{
		volatile struct	/* 0x150272A0 */
		{
				FIELD  RANSED_SEED1                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED2                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_1;	/* DIP_X_HFG_RAN_1, DIP_A_HFG_RAN_1*/

typedef volatile union _DIP_X_REG_HFG_RAN_2_
{
		volatile struct	/* 0x150272A4 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_2;	/* DIP_X_HFG_RAN_2, DIP_A_HFG_RAN_2*/

typedef volatile union _DIP_X_REG_HFG_RAN_3_
{
		volatile struct	/* 0x150272A8 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_3;	/* DIP_X_HFG_RAN_3, DIP_A_HFG_RAN_3*/

typedef volatile union _DIP_X_REG_HFG_RAN_4_
{
		volatile struct	/* 0x150272AC */
		{
				FIELD  HFG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_4;	/* DIP_X_HFG_RAN_4, DIP_A_HFG_RAN_4*/

typedef volatile union _DIP_X_REG_HFG_CROP_X_
{
		volatile struct	/* 0x150272B0 */
		{
				FIELD  HFG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CROP_X;	/* DIP_X_HFG_CROP_X, DIP_A_HFG_CROP_X*/

typedef volatile union _DIP_X_REG_HFG_CROP_Y_
{
		volatile struct	/* 0x150272B4 */
		{
				FIELD  HFG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CROP_Y;	/* DIP_X_HFG_CROP_Y, DIP_A_HFG_CROP_Y*/

typedef volatile union _DIP_X_REG_SL2I_CEN_
{
		volatile struct	/* 0x150272C0 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_CEN;	/* DIP_X_SL2I_CEN, DIP_A_SL2I_CEN*/

typedef volatile union _DIP_X_REG_SL2I_RR_CON0_
{
		volatile struct	/* 0x150272C4 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_RR_CON0;	/* DIP_X_SL2I_RR_CON0, DIP_A_SL2I_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2I_RR_CON1_
{
		volatile struct	/* 0x150272C8 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_RR_CON1;	/* DIP_X_SL2I_RR_CON1, DIP_A_SL2I_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2I_GAIN_
{
		volatile struct	/* 0x150272CC */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_GAIN;	/* DIP_X_SL2I_GAIN, DIP_A_SL2I_GAIN*/

typedef volatile union _DIP_X_REG_SL2I_RZ_
{
		volatile struct	/* 0x150272D0 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_RZ;	/* DIP_X_SL2I_RZ, DIP_A_SL2I_RZ*/

typedef volatile union _DIP_X_REG_SL2I_XOFF_
{
		volatile struct	/* 0x150272D4 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_XOFF;	/* DIP_X_SL2I_XOFF, DIP_A_SL2I_XOFF*/

typedef volatile union _DIP_X_REG_SL2I_YOFF_
{
		volatile struct	/* 0x150272D8 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_YOFF;	/* DIP_X_SL2I_YOFF, DIP_A_SL2I_YOFF*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON0_
{
		volatile struct	/* 0x150272DC */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON0;	/* DIP_X_SL2I_SLP_CON0, DIP_A_SL2I_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON1_
{
		volatile struct	/* 0x150272E0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON1;	/* DIP_X_SL2I_SLP_CON1, DIP_A_SL2I_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON2_
{
		volatile struct	/* 0x150272E4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON2;	/* DIP_X_SL2I_SLP_CON2, DIP_A_SL2I_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON3_
{
		volatile struct	/* 0x150272E8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON3;	/* DIP_X_SL2I_SLP_CON3, DIP_A_SL2I_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2I_SIZE_
{
		volatile struct	/* 0x150272EC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SIZE;	/* DIP_X_SL2I_SIZE, DIP_A_SL2I_SIZE*/

typedef volatile union _DIP_X_REG_CRZ_CONTROL_
{
		volatile struct	/* 0x15027300 */
		{
				FIELD  CRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CRZ_HORI_ALGO                         :  2;		/*  5.. 6, 0x00000060 */
				FIELD  CRZ_VERT_ALGO                         :  2;		/*  7.. 8, 0x00000180 */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CRZ_TRNC_HORI                         :  3;		/* 10..12, 0x00001C00 */
				FIELD  CRZ_TRNC_VERT                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  CRZ_HORI_TBL_SEL                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  CRZ_VERT_TBL_SEL                      :  5;		/* 21..25, 0x03E00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CONTROL;	/* DIP_X_CRZ_CONTROL, DIP_A_CRZ_CONTROL*/

typedef volatile union _DIP_X_REG_CRZ_IN_IMG_
{
		volatile struct	/* 0x15027304 */
		{
				FIELD  CRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_IN_IMG;	/* DIP_X_CRZ_IN_IMG, DIP_A_CRZ_IN_IMG*/

typedef volatile union _DIP_X_REG_CRZ_OUT_IMG_
{
		volatile struct	/* 0x15027308 */
		{
				FIELD  CRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_OUT_IMG;	/* DIP_X_CRZ_OUT_IMG, DIP_A_CRZ_OUT_IMG*/

typedef volatile union _DIP_X_REG_CRZ_HORI_STEP_
{
		volatile struct	/* 0x1502730C */
		{
				FIELD  CRZ_HORI_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_HORI_STEP;	/* DIP_X_CRZ_HORI_STEP, DIP_A_CRZ_HORI_STEP*/

typedef volatile union _DIP_X_REG_CRZ_VERT_STEP_
{
		volatile struct	/* 0x15027310 */
		{
				FIELD  CRZ_VERT_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_VERT_STEP;	/* DIP_X_CRZ_VERT_STEP, DIP_A_CRZ_VERT_STEP*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_HORI_INT_OFST_
{
		volatile struct	/* 0x15027314 */
		{
				FIELD  CRZ_LUMA_HORI_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_HORI_INT_OFST;	/* DIP_X_CRZ_LUMA_HORI_INT_OFST, DIP_A_CRZ_LUMA_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST_
{
		volatile struct	/* 0x15027318 */
		{
				FIELD  CRZ_LUMA_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST;	/* DIP_X_CRZ_LUMA_HORI_SUB_OFST, DIP_A_CRZ_LUMA_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_VERT_INT_OFST_
{
		volatile struct	/* 0x1502731C */
		{
				FIELD  CRZ_LUMA_VERT_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_VERT_INT_OFST;	/* DIP_X_CRZ_LUMA_VERT_INT_OFST, DIP_A_CRZ_LUMA_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST_
{
		volatile struct	/* 0x15027320 */
		{
				FIELD  CRZ_LUMA_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST;	/* DIP_X_CRZ_LUMA_VERT_SUB_OFST, DIP_A_CRZ_LUMA_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_HORI_INT_OFST_
{
		volatile struct	/* 0x15027324 */
		{
				FIELD  CRZ_CHRO_HORI_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_HORI_INT_OFST;	/* DIP_X_CRZ_CHRO_HORI_INT_OFST, DIP_A_CRZ_CHRO_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST_
{
		volatile struct	/* 0x15027328 */
		{
				FIELD  CRZ_CHRO_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST;	/* DIP_X_CRZ_CHRO_HORI_SUB_OFST, DIP_A_CRZ_CHRO_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_VERT_INT_OFST_
{
		volatile struct	/* 0x1502732C */
		{
				FIELD  CRZ_CHRO_VERT_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_VERT_INT_OFST;	/* DIP_X_CRZ_CHRO_VERT_INT_OFST, DIP_A_CRZ_CHRO_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST_
{
		volatile struct	/* 0x15027330 */
		{
				FIELD  CRZ_CHRO_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST;	/* DIP_X_CRZ_CHRO_VERT_SUB_OFST, DIP_A_CRZ_CHRO_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_SPARE_1_
{
		volatile struct	/* 0x15027334 */
		{
				FIELD  CRZ_SPARE_REG                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_SPARE_1;	/* DIP_X_CRZ_SPARE_1, DIP_A_CRZ_SPARE_1*/

typedef volatile union _DIP_X_REG_CAM_TNR_ENG_CON_
{
		volatile struct	/* 0x15027380 */
		{
				FIELD  NR3D_CAM_TNR_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NR3D_CAM_TNR_C42_FILT_DIS             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  NR3D_CAM_TNR_DCM_DIS                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NR3D_CAM_TNR_RESET                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  NR3D_CAM_TNR_CROP_H                   :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  NR3D_CAM_TNR_CROP_V                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  NR3D_CAM_TNR_UV_SIGNED                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_TNR_ENG_CON;	/* DIP_X_CAM_TNR_ENG_CON, DIP_A_CAM_TNR_ENG_CON*/

typedef volatile union _DIP_X_REG_CAM_TNR_SIZ_
{
		volatile struct	/* 0x15027384 */
		{
				FIELD  NR3D_CAM_TNR_HSIZE                    : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR3D_CAM_TNR_VSIZE                    : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_TNR_SIZ;	/* DIP_X_CAM_TNR_SIZ, DIP_A_CAM_TNR_SIZ*/

typedef volatile union _DIP_X_REG_CAM_TNR_TILE_XY_
{
		volatile struct	/* 0x15027388 */
		{
				FIELD  NR3D_CAM_TNR_TILE_H                   : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR3D_CAM_TNR_TILE_V                   : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_TNR_TILE_XY;	/* DIP_X_CAM_TNR_TILE_XY, DIP_A_CAM_TNR_TILE_XY*/

typedef volatile union _DIP_X_REG_NR3D_ON_CON_
{
		volatile struct	/* 0x1502738C */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR3D_ON_EN                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  NR3D_TILE_EDGE                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  8;		/* 12..19, 0x000FF000 */
				FIELD  NR3D_SL2_OFF                          :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NR3D_ON_CON;	/* DIP_X_NR3D_ON_CON, DIP_A_NR3D_ON_CON*/

typedef volatile union _DIP_X_REG_NR3D_ON_OFF_
{
		volatile struct	/* 0x15027390 */
		{
				FIELD  NR3D_ON_OFST_X                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR3D_ON_OFST_Y                        : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NR3D_ON_OFF;	/* DIP_X_NR3D_ON_OFF, DIP_A_NR3D_ON_OFF*/

typedef volatile union _DIP_X_REG_NR3D_ON_SIZ_
{
		volatile struct	/* 0x15027394 */
		{
				FIELD  NR3D_ON_WD                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  NR3D_ON_HT                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NR3D_ON_SIZ;	/* DIP_X_NR3D_ON_SIZ, DIP_A_NR3D_ON_SIZ*/

typedef volatile union _DIP_X_REG_MDP_TNR_TNR_ENABLE_
{
		volatile struct	/* 0x15027398 */
		{
				FIELD  rsv_0                                 : 30;		/*  0..29, 0x3FFFFFFF */
				FIELD  NR3D_TNR_C_EN                         :  1;		/* 30..30, 0x40000000 */
				FIELD  NR3D_TNR_Y_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_TNR_ENABLE;	/* DIP_X_MDP_TNR_TNR_ENABLE, DIP_A_MDP_TNR_TNR_ENABLE*/

typedef volatile union _DIP_X_REG_MDP_TNR_FLT_CONFIG_
{
		volatile struct	/* 0x1502739C */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR3D_EN_CCR                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  NR3D_EN_CYCR                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  NR3D_FLT_STR_MAX                      :  6;		/*  6..11, 0x00000FC0 */
				FIELD  NR3D_BLEND_RATIO_MV                   :  5;		/* 12..16, 0x0001F000 */
				FIELD  NR3D_BLEND_RATIO_TXTR                 :  5;		/* 17..21, 0x003E0000 */
				FIELD  NR3D_BLEND_RATIO_DE                   :  5;		/* 22..26, 0x07C00000 */
				FIELD  NR3D_BLEND_RATIO_BLKY                 :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_FLT_CONFIG;	/* DIP_X_MDP_TNR_FLT_CONFIG, DIP_A_MDP_TNR_FLT_CONFIG*/

typedef volatile union _DIP_X_REG_MDP_TNR_FB_INFO1_
{
		volatile struct	/* 0x150273A0 */
		{
				FIELD  rsv_0                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  NR3D_Q_SP                             :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_NL                             :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_FB_INFO1;	/* DIP_X_MDP_TNR_FB_INFO1, DIP_A_MDP_TNR_FB_INFO1*/

typedef volatile union _DIP_X_REG_MDP_TNR_THR_1_
{
		volatile struct	/* 0x150273A4 */
		{
				FIELD  rsv_0                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  NR3D_SMALL_SAD_THR                    :  6;		/* 10..15, 0x0000FC00 */
				FIELD  NR3D_MV_PEN_THR                       :  6;		/* 16..21, 0x003F0000 */
				FIELD  NR3D_MV_PEN_W                         :  4;		/* 22..25, 0x03C00000 */
				FIELD  NR3D_BDI_THR                          :  4;		/* 26..29, 0x3C000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_THR_1;	/* DIP_X_MDP_TNR_THR_1, DIP_A_MDP_TNR_THR_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_1_
{
		volatile struct	/* 0x150273A8 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_BLKY_Y4                        :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_BLKY_Y3                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_BLKY_Y2                        :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_BLKY_Y1                        :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_BLKY_Y0                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_1;	/* DIP_X_MDP_TNR_CURVE_1, DIP_A_MDP_TNR_CURVE_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_2_
{
		volatile struct	/* 0x150273AC */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_BLKC_Y1                        :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_BLKC_Y0                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_BLKY_Y7                        :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_BLKY_Y6                        :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_BLKY_Y5                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_2;	/* DIP_X_MDP_TNR_CURVE_2, DIP_A_MDP_TNR_CURVE_2*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_3_
{
		volatile struct	/* 0x150273B0 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_BLKC_Y6                        :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_BLKC_Y5                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_BLKC_Y4                        :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_BLKC_Y3                        :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_BLKC_Y2                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_3;	/* DIP_X_MDP_TNR_CURVE_3, DIP_A_MDP_TNR_CURVE_3*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_4_
{
		volatile struct	/* 0x150273B4 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_DETXTR_LVL_Y3                  :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_DETXTR_LVL_Y2                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_DETXTR_LVL_Y1                  :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_DETXTR_LVL_Y0                  :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_BLKC_Y7                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_4;	/* DIP_X_MDP_TNR_CURVE_4, DIP_A_MDP_TNR_CURVE_4*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_5_
{
		volatile struct	/* 0x150273B8 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_DE1_BASE_Y0                    :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_DETXTR_LVL_Y7                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_DETXTR_LVL_Y6                  :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_DETXTR_LVL_Y5                  :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_DETXTR_LVL_Y4                  :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_5;	/* DIP_X_MDP_TNR_CURVE_5, DIP_A_MDP_TNR_CURVE_5*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_6_
{
		volatile struct	/* 0x150273BC */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_DE1_BASE_Y5                    :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_DE1_BASE_Y4                    :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_DE1_BASE_Y3                    :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_DE1_BASE_Y2                    :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_DE1_BASE_Y1                    :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_6;	/* DIP_X_MDP_TNR_CURVE_6, DIP_A_MDP_TNR_CURVE_6*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_7_
{
		volatile struct	/* 0x150273C0 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y2                :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y1                :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y0                :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_DE1_BASE_Y7                    :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_DE1_BASE_Y6                    :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_7;	/* DIP_X_MDP_TNR_CURVE_7, DIP_A_MDP_TNR_CURVE_7*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_8_
{
		volatile struct	/* 0x150273C4 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y7                :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y6                :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y5                :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y4                :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_DE2TXTR_BASE_Y3                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_8;	/* DIP_X_MDP_TNR_CURVE_8, DIP_A_MDP_TNR_CURVE_8*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_9_
{
		volatile struct	/* 0x150273C8 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_MV_Y4                          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_MV_Y3                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_MV_Y2                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_MV_Y1                          :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_MV_Y0                          :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_9;	/* DIP_X_MDP_TNR_CURVE_9, DIP_A_MDP_TNR_CURVE_9*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_10_
{
		volatile struct	/* 0x150273CC */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_WVAR_Y1                        :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_WVAR_Y0                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_MV_Y7                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_MV_Y6                          :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_MV_Y5                          :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_10;	/* DIP_X_MDP_TNR_CURVE_10, DIP_A_MDP_TNR_CURVE_10*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_11_
{
		volatile struct	/* 0x150273D0 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_WVAR_Y6                        :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_WVAR_Y5                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_WVAR_Y4                        :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_WVAR_Y3                        :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_WVAR_Y2                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_11;	/* DIP_X_MDP_TNR_CURVE_11, DIP_A_MDP_TNR_CURVE_11*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_12_
{
		volatile struct	/* 0x150273D4 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_WSM_Y3                         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_WSM_Y2                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_WSM_Y1                         :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_WSM_Y0                         :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_WVAR_Y7                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_12;	/* DIP_X_MDP_TNR_CURVE_12, DIP_A_MDP_TNR_CURVE_12*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_13_
{
		volatile struct	/* 0x150273D8 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NR3D_Q_WSM_Y7                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_WSM_Y6                         :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_WSM_Y5                         :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_WSM_Y4                         :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_13;	/* DIP_X_MDP_TNR_CURVE_13, DIP_A_MDP_TNR_CURVE_13*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_14_
{
		volatile struct	/* 0x150273DC */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_Q_SDL_Y4                         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_Q_SDL_Y3                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_SDL_Y2                         :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_SDL_Y1                         :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_SDL_Y0                         :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_14;	/* DIP_X_MDP_TNR_CURVE_14, DIP_A_MDP_TNR_CURVE_14*/

typedef volatile union _DIP_X_REG_MDP_TNR_CURVE_15_
{
		volatile struct	/* 0x150273E0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NR3D_Q_SDL_Y8                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_Q_SDL_Y7                         :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_Q_SDL_Y6                         :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_SDL_Y5                         :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_CURVE_15;	/* DIP_X_MDP_TNR_CURVE_15, DIP_A_MDP_TNR_CURVE_15*/

typedef volatile union _DIP_X_REG_MDP_TNR_R2C_1_
{
		volatile struct	/* 0x150273E4 */
		{
				FIELD  NR3D_R2CENC                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_R2C_VAL4                         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_R2C_VAL3                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_R2C_VAL2                         :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_R2C_VAL1                         :  6;		/* 20..25, 0x03F00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_R2C_1;	/* DIP_X_MDP_TNR_R2C_1, DIP_A_MDP_TNR_R2C_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_R2C_2_
{
		volatile struct	/* 0x150273E8 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_R2C_TXTR_THROFF                  :  6;		/*  2.. 7, 0x000000FC */
				FIELD  NR3D_R2C_TXTR_THR4                    :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_R2C_TXTR_THR3                    :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_R2C_TXTR_THR2                    :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_R2C_TXTR_THR1                    :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_R2C_2;	/* DIP_X_MDP_TNR_R2C_2, DIP_A_MDP_TNR_R2C_2*/

typedef volatile union _DIP_X_REG_MDP_TNR_R2C_3_
{
		volatile struct	/* 0x150273EC */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NR3D_R2CF_CNT4                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  NR3D_R2CF_CNT3                        :  6;		/* 14..19, 0x000FC000 */
				FIELD  NR3D_R2CF_CNT2                        :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_R2CF_CNT1                        :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_R2C_3;	/* DIP_X_MDP_TNR_R2C_3, DIP_A_MDP_TNR_R2C_3*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_6_
{
		volatile struct	/* 0x150273F0 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_FORCE_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  NR3D_INK_LEVEL_DISP                   : 16;		/*  3..18, 0x0007FFF8 */
				FIELD  NR3D_INK_SEL                          :  5;		/* 19..23, 0x00F80000 */
				FIELD  NR3D_INK_Y_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  NR3D_INK_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  NR3D_FORCE_FLT_STR                    :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_6;	/* DIP_X_MDP_TNR_DBG_6, DIP_A_MDP_TNR_DBG_6*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_15_
{
		volatile struct	/* 0x150273F4 */
		{
				FIELD  rsv_0                                 :  2;		/*  0.. 1, 0x00000003 */
				FIELD  NR3D_OSD_TARGV                        : 13;		/*  2..14, 0x00007FFC */
				FIELD  NR3D_OSD_TARGH                        : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  NR3D_OSD_SEL                          :  3;		/* 28..30, 0x70000000 */
				FIELD  NR3D_OSD_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_15;	/* DIP_X_MDP_TNR_DBG_15, DIP_A_MDP_TNR_DBG_15*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_16_
{
		volatile struct	/* 0x150273F8 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  NR3D_OSD_Y_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  NR3D_OSD_DISP_SCALE                   :  2;		/*  4.. 5, 0x00000030 */
				FIELD  NR3D_OSD_DISPV                        : 13;		/*  6..18, 0x0007FFC0 */
				FIELD  NR3D_OSD_DISPH                        : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_16;	/* DIP_X_MDP_TNR_DBG_16, DIP_A_MDP_TNR_DBG_16*/

typedef volatile union _DIP_X_REG_MDP_TNR_DEMO_1_
{
		volatile struct	/* 0x150273FC */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR3D_DEMO_BOT                         : 13;		/*  4..16, 0x0001FFF0 */
				FIELD  NR3D_DEMO_TOP                         : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  NR3D_DEMO_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  NR3D_DEMO_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DEMO_1;	/* DIP_X_MDP_TNR_DEMO_1, DIP_A_MDP_TNR_DEMO_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_DEMO_2_
{
		volatile struct	/* 0x15027400 */
		{
				FIELD  rsv_0                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  NR3D_DEMO_RIGHT                       : 13;		/*  6..18, 0x0007FFC0 */
				FIELD  NR3D_DEMO_LEFT                        : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DEMO_2;	/* DIP_X_MDP_TNR_DEMO_2, DIP_A_MDP_TNR_DEMO_2*/

typedef volatile union _DIP_X_REG_MDP_TNR_ATPG_
{
		volatile struct	/* 0x15027404 */
		{
				FIELD  NR3D_TNR_ATPG_OB                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NR3D_TNR_ATPG_CT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_ATPG;	/* DIP_X_MDP_TNR_ATPG, DIP_A_MDP_TNR_ATPG*/

typedef volatile union _DIP_X_REG_MDP_TNR_DMY_0_
{
		volatile struct	/* 0x15027408 */
		{
				FIELD  NR3D_TNR_DUMMY0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DMY_0;	/* DIP_X_MDP_TNR_DMY_0, DIP_A_MDP_TNR_DMY_0*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_17_
{
		volatile struct	/* 0x1502740C */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR3D_OSD_INF1                         : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  NR3D_OSD_INF0                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_17;	/* DIP_X_MDP_TNR_DBG_17, DIP_A_MDP_TNR_DBG_17*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_18_
{
		volatile struct	/* 0x15027410 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NR3D_OSD_INF3                         : 12;		/*  8..19, 0x000FFF00 */
				FIELD  NR3D_OSD_INF2                         : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_18;	/* DIP_X_MDP_TNR_DBG_18, DIP_A_MDP_TNR_DBG_18*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_1_
{
		volatile struct	/* 0x15027414 */
		{
				FIELD  NR3D_TNR_DBG0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_1;	/* DIP_X_MDP_TNR_DBG_1, DIP_A_MDP_TNR_DBG_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_2_
{
		volatile struct	/* 0x15027418 */
		{
				FIELD  NR3D_TNR_DBG1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_2;	/* DIP_X_MDP_TNR_DBG_2, DIP_A_MDP_TNR_DBG_2*/

typedef volatile union _DIP_X_REG_SL2E_CEN_
{
		volatile struct	/* 0x15027440 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_CEN;	/* DIP_X_SL2E_CEN, DIP_A_SL2E_CEN*/

typedef volatile union _DIP_X_REG_SL2E_RR_CON0_
{
		volatile struct	/* 0x15027444 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_RR_CON0;	/* DIP_X_SL2E_RR_CON0, DIP_A_SL2E_RR_CON0*/

typedef volatile union _DIP_X_REG_SL2E_RR_CON1_
{
		volatile struct	/* 0x15027448 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_RR_CON1;	/* DIP_X_SL2E_RR_CON1, DIP_A_SL2E_RR_CON1*/

typedef volatile union _DIP_X_REG_SL2E_GAIN_
{
		volatile struct	/* 0x1502744C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_GAIN;	/* DIP_X_SL2E_GAIN, DIP_A_SL2E_GAIN*/

typedef volatile union _DIP_X_REG_SL2E_RZ_
{
		volatile struct	/* 0x15027450 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_RZ;	/* DIP_X_SL2E_RZ, DIP_A_SL2E_RZ*/

typedef volatile union _DIP_X_REG_SL2E_XOFF_
{
		volatile struct	/* 0x15027454 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_XOFF;	/* DIP_X_SL2E_XOFF, DIP_A_SL2E_XOFF*/

typedef volatile union _DIP_X_REG_SL2E_YOFF_
{
		volatile struct	/* 0x15027458 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_YOFF;	/* DIP_X_SL2E_YOFF, DIP_A_SL2E_YOFF*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON0_
{
		volatile struct	/* 0x1502745C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON0;	/* DIP_X_SL2E_SLP_CON0, DIP_A_SL2E_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON1_
{
		volatile struct	/* 0x15027460 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON1;	/* DIP_X_SL2E_SLP_CON1, DIP_A_SL2E_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON2_
{
		volatile struct	/* 0x15027464 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON2;	/* DIP_X_SL2E_SLP_CON2, DIP_A_SL2E_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON3_
{
		volatile struct	/* 0x15027468 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON3;	/* DIP_X_SL2E_SLP_CON3, DIP_A_SL2E_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2E_SIZE_
{
		volatile struct	/* 0x1502746C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SIZE;	/* DIP_X_SL2E_SIZE, DIP_A_SL2E_SIZE*/

typedef volatile union _DIP_X_REG_CRSP_CTRL_
{
		volatile struct	/* 0x15027480 */
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
}DIP_X_REG_CRSP_CTRL;	/* DIP_X_CRSP_CTRL, DIP_A_CRSP_CTRL*/

typedef volatile union _DIP_X_REG_CRSP_OUT_IMG_
{
		volatile struct	/* 0x15027488 */
		{
				FIELD  CRSP_WD                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_HT                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_OUT_IMG;	/* DIP_X_CRSP_OUT_IMG, DIP_A_CRSP_OUT_IMG*/

typedef volatile union _DIP_X_REG_CRSP_STEP_OFST_
{
		volatile struct	/* 0x1502748C */
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
}DIP_X_REG_CRSP_STEP_OFST;	/* DIP_X_CRSP_STEP_OFST, DIP_A_CRSP_STEP_OFST*/

typedef volatile union _DIP_X_REG_CRSP_CROP_X_
{
		volatile struct	/* 0x15027490 */
		{
				FIELD  CRSP_CROP_STR_X                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_X                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_CROP_X;	/* DIP_X_CRSP_CROP_X, DIP_A_CRSP_CROP_X*/

typedef volatile union _DIP_X_REG_CRSP_CROP_Y_
{
		volatile struct	/* 0x15027494 */
		{
				FIELD  CRSP_CROP_STR_Y                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_Y                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_CROP_Y;	/* DIP_X_CRSP_CROP_Y, DIP_A_CRSP_CROP_Y*/

typedef volatile union _DIP_X_REG_C24B_TILE_EDGE_
{
		volatile struct	/* 0x150274C0 */
		{
				FIELD  C24_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C24B_TILE_EDGE;	/* DIP_X_C24B_TILE_EDGE, DIP_A_C24B_TILE_EDGE*/

typedef volatile union _DIP_X_REG_MDP_CROP_X_
{
		volatile struct	/* 0x15027500 */
		{
				FIELD  MDP_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP_X;	/* DIP_X_MDP_CROP_X, DIP_A_MDP_CROP_X*/

typedef volatile union _DIP_X_REG_MDP_CROP_Y_
{
		volatile struct	/* 0x15027504 */
		{
				FIELD  MDP_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP_Y;	/* DIP_X_MDP_CROP_Y, DIP_A_MDP_CROP_Y*/

typedef volatile union _DIP_X_REG_MIX4_CTRL_0_
{
		volatile struct	/* 0x15027540 */
		{
				FIELD  MIX4_WT_SEL                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  MIX4_Y_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIX4_Y_DEFAULT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MIX4_UV_EN                            :  1;		/*  6.. 6, 0x00000040 */
				FIELD  MIX4_UV_DEFAULT                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  MIX4_B0                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX4_B1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  MIX4_DT                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX4_CTRL_0;	/* DIP_X_MIX4_CTRL_0, DIP_A_MIX4_CTRL_0*/

typedef volatile union _DIP_X_REG_MIX4_CTRL_1_
{
		volatile struct	/* 0x15027544 */
		{
				FIELD  MIX4_M0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  MIX4_M1                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX4_CTRL_1;	/* DIP_X_MIX4_CTRL_1, DIP_A_MIX4_CTRL_1*/

typedef volatile union _DIP_X_REG_MIX4_SPARE_
{
		volatile struct	/* 0x15027548 */
		{
				FIELD  MIX4_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX4_SPARE;	/* DIP_X_MIX4_SPARE, DIP_A_MIX4_SPARE*/

typedef volatile union _DIP_X_REG_LCE25_TPIPE_SLM_
{
        volatile struct /* 0x15027550 */
        {
                FIELD  LCE_TPIPE_SLM_WD                      : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCE_TPIPE_SLM_HT                      : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_SLM;

typedef volatile union _DIP_X_REG_LCE25_TPIPE_OFFSET_
{
        volatile struct /* 0x15027554 */
        {
                FIELD  LCE_TPIPE_OFFSET_X                    : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCE_TPIPE_OFFSET_Y                    : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_OFFSET;

typedef volatile union _DIP_X_REG_LCE25_TPIPE_OUT_
{
        volatile struct /* 0x15027558 */
        {
                FIELD  LCE_TPIPE_OUT_HT                      : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_OUT;

typedef volatile union _DIP_X_REG_NR3D_VIPI_SIZE_
{
        volatile struct /* 0x1502755C */
        {
                FIELD  NR3D_VIPI_WIDTH                       : 16;      /*  0..15, 0x0000FFFF */
                FIELD  NR3D_VIPI_HEIGHT                      : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_NR3D_VIPI_SIZE;

typedef volatile union _DIP_X_REG_NR3D_VIPI_OFFSET_
{
        volatile struct /* 0x15027560 */
        {
                FIELD  NR3D_VIPI_OFFSET                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_NR3D_VIPI_OFFSET;


typedef volatile struct _dip_x_reg_t_	/* 0x15022000..0x15027547 */
{
	DIP_X_REG_CTL_START                             DIP_X_CTL_START;                                 /* 0000, 0x15022000, DIP_A_CTL_START */
	DIP_X_REG_CTL_YUV_EN                            DIP_X_CTL_YUV_EN;                                /* 0004, 0x15022004, DIP_A_CTL_YUV_EN */
	DIP_X_REG_CTL_YUV2_EN                           DIP_X_CTL_YUV2_EN;                               /* 0008, 0x15022008, DIP_A_CTL_YUV2_EN */
	DIP_X_REG_CTL_RGB_EN                            DIP_X_CTL_RGB_EN;                                /* 000C, 0x1502200C, DIP_A_CTL_RGB_EN */
	DIP_X_REG_CTL_DMA_EN                            DIP_X_CTL_DMA_EN;                                /* 0010, 0x15022010, DIP_A_CTL_DMA_EN */
	DIP_X_REG_CTL_FMT_SEL                           DIP_X_CTL_FMT_SEL;                               /* 0014, 0x15022014, DIP_A_CTL_FMT_SEL */
	DIP_X_REG_CTL_PATH_SEL                          DIP_X_CTL_PATH_SEL;                              /* 0018, 0x15022018, DIP_A_CTL_PATH_SEL */
	DIP_X_REG_CTL_MISC_SEL                          DIP_X_CTL_MISC_SEL;                              /* 001C, 0x1502201C, DIP_A_CTL_MISC_SEL */
	DIP_X_REG_CTL_INT_EN                            DIP_X_CTL_INT_EN;                                /* 0020, 0x15022020, DIP_A_CTL_INT_EN */
	DIP_X_REG_CTL_CQ_INT_EN                         DIP_X_CTL_CQ_INT_EN;                             /* 0024, 0x15022024, DIP_A_CTL_CQ_INT_EN */
	DIP_X_REG_CTL_CQ_INT2_EN                        DIP_X_CTL_CQ_INT2_EN;                            /* 0028, 0x15022028, DIP_A_CTL_CQ_INT2_EN */
	DIP_X_REG_CTL_CQ_INT3_EN                        DIP_X_CTL_CQ_INT3_EN;                            /* 002C, 0x1502202C, DIP_A_CTL_CQ_INT3_EN */
	DIP_X_REG_CTL_INT_STATUS                        DIP_X_CTL_INT_STATUS;                            /* 0030, 0x15022030, DIP_A_CTL_INT_STATUS */
	DIP_X_REG_CTL_CQ_INT_STATUS                     DIP_X_CTL_CQ_INT_STATUS;                         /* 0034, 0x15022034, DIP_A_CTL_CQ_INT_STATUS */
	DIP_X_REG_CTL_CQ_INT2_STATUS                    DIP_X_CTL_CQ_INT2_STATUS;                        /* 0038, 0x15022038, DIP_A_CTL_CQ_INT2_STATUS */
	DIP_X_REG_CTL_CQ_INT3_STATUS                    DIP_X_CTL_CQ_INT3_STATUS;                        /* 003C, 0x1502203C, DIP_A_CTL_CQ_INT3_STATUS */
	DIP_X_REG_CTL_INT_STATUSX                       DIP_X_CTL_INT_STATUSX;                           /* 0040, 0x15022040, DIP_A_CTL_INT_STATUSX */
	DIP_X_REG_CTL_CQ_INT_STATUSX                    DIP_X_CTL_CQ_INT_STATUSX;                        /* 0044, 0x15022044, DIP_A_CTL_CQ_INT_STATUSX */
	DIP_X_REG_CTL_CQ_INT2_STATUSX                   DIP_X_CTL_CQ_INT2_STATUSX;                       /* 0048, 0x15022048, DIP_A_CTL_CQ_INT2_STATUSX */
	DIP_X_REG_CTL_CQ_INT3_STATUSX                   DIP_X_CTL_CQ_INT3_STATUSX;                       /* 004C, 0x1502204C, DIP_A_CTL_CQ_INT3_STATUSX */
	DIP_X_REG_CTL_TDR_CTL                           DIP_X_CTL_TDR_CTL;                               /* 0050, 0x15022050, DIP_A_CTL_TDR_CTL */
	DIP_X_REG_CTL_TDR_TILE                          DIP_X_CTL_TDR_TILE;                              /* 0054, 0x15022054, DIP_A_CTL_TDR_TILE */
	DIP_X_REG_CTL_TDR_TCM_EN                        DIP_X_CTL_TDR_TCM_EN;                            /* 0058, 0x15022058, DIP_A_CTL_TDR_TCM_EN */
	DIP_X_REG_CTL_TDR_TCM2_EN                       DIP_X_CTL_TDR_TCM2_EN;                           /* 005C, 0x1502205C, DIP_A_CTL_TDR_TCM2_EN */
	DIP_X_REG_CTL_TDR_EN_STATUS1                    DIP_X_CTL_TDR_EN_STATUS1;                        /* 0060, 0x15022060, DIP_A_CTL_TDR_EN_STATUS1 */
	DIP_X_REG_CTL_TDR_EN_STATUS2                    DIP_X_CTL_TDR_EN_STATUS2;                        /* 0064, 0x15022064, DIP_A_CTL_TDR_EN_STATUS2 */
	DIP_X_REG_CTL_TDR_DBG_STATUS                    DIP_X_CTL_TDR_DBG_STATUS;                        /* 0068, 0x15022068, DIP_A_CTL_TDR_DBG_STATUS */
	DIP_X_REG_CTL_SW_CTL                            DIP_X_CTL_SW_CTL;                                /* 006C, 0x1502206C, DIP_A_CTL_SW_CTL */
	DIP_X_REG_CTL_SPARE0                            DIP_X_CTL_SPARE0;                                /* 0070, 0x15022070, DIP_A_CTL_SPARE0 */
	DIP_X_REG_CTL_SPARE1                            DIP_X_CTL_SPARE1;                                /* 0074, 0x15022074, DIP_A_CTL_SPARE1 */
	DIP_X_REG_CTL_SPARE2                            DIP_X_CTL_SPARE2;                                /* 0078, 0x15022078, DIP_A_CTL_SPARE2 */
	DIP_X_REG_CTL_DONE_SEL                          DIP_X_CTL_DONE_SEL;                              /* 007C, 0x1502207C, DIP_A_CTL_DONE_SEL */
	DIP_X_REG_CTL_DBG_SET                           DIP_X_CTL_DBG_SET;                               /* 0080, 0x15022080, DIP_A_CTL_DBG_SET */
	DIP_X_REG_CTL_DBG_PORT                          DIP_X_CTL_DBG_PORT;                              /* 0084, 0x15022084, DIP_A_CTL_DBG_PORT */
	DIP_X_REG_CTL_DATE_CODE                         DIP_X_CTL_DATE_CODE;                             /* 0088, 0x15022088, DIP_A_CTL_DATE_CODE */
	DIP_X_REG_CTL_PROJ_CODE                         DIP_X_CTL_PROJ_CODE;                             /* 008C, 0x1502208C, DIP_A_CTL_PROJ_CODE */
	DIP_X_REG_CTL_YUV_DCM_DIS                       DIP_X_CTL_YUV_DCM_DIS;                           /* 0090, 0x15022090, DIP_A_CTL_YUV_DCM_DIS */
	DIP_X_REG_CTL_YUV2_DCM_DIS                      DIP_X_CTL_YUV2_DCM_DIS;                          /* 0094, 0x15022094, DIP_A_CTL_YUV2_DCM_DIS */
	DIP_X_REG_CTL_RGB_DCM_DIS                       DIP_X_CTL_RGB_DCM_DIS;                           /* 0098, 0x15022098, DIP_A_CTL_RGB_DCM_DIS */
	DIP_X_REG_CTL_DMA_DCM_DIS                       DIP_X_CTL_DMA_DCM_DIS;                           /* 009C, 0x1502209C, DIP_A_CTL_DMA_DCM_DIS */
	DIP_X_REG_CTL_TOP_DCM_DIS                       DIP_X_CTL_TOP_DCM_DIS;                           /* 00A0, 0x150220A0, DIP_A_CTL_TOP_DCM_DIS */
	DIP_X_REG_CTL_YUV_DCM_STATUS                    DIP_X_CTL_YUV_DCM_STATUS;                        /* 00A4, 0x150220A4, DIP_A_CTL_YUV_DCM_STATUS */
	DIP_X_REG_CTL_YUV2_DCM_STATUS                   DIP_X_CTL_YUV2_DCM_STATUS;                       /* 00A8, 0x150220A8, DIP_A_CTL_YUV2_DCM_STATUS */
	DIP_X_REG_CTL_RGB_DCM_STATUS                    DIP_X_CTL_RGB_DCM_STATUS;                        /* 00AC, 0x150220AC, DIP_A_CTL_RGB_DCM_STATUS */
	DIP_X_REG_CTL_DMA_DCM_STATUS                    DIP_X_CTL_DMA_DCM_STATUS;                        /* 00B0, 0x150220B0, DIP_A_CTL_DMA_DCM_STATUS */
	DIP_X_REG_CTL_TOP_DCM_STATUS                    DIP_X_CTL_TOP_DCM_STATUS;                        /* 00B4, 0x150220B4, DIP_A_CTL_TOP_DCM_STATUS */
	DIP_X_REG_CTL_YUV_REQ_STATUS                    DIP_X_CTL_YUV_REQ_STATUS;                        /* 00B8, 0x150220B8, DIP_A_CTL_YUV_REQ_STATUS */
	DIP_X_REG_CTL_YUV2_REQ_STATUS                   DIP_X_CTL_YUV2_REQ_STATUS;                       /* 00BC, 0x150220BC, DIP_A_CTL_YUV2_REQ_STATUS */
	DIP_X_REG_CTL_RGB_REQ_STATUS                    DIP_X_CTL_RGB_REQ_STATUS;                        /* 00C0, 0x150220C0, DIP_A_CTL_RGB_REQ_STATUS */
	DIP_X_REG_CTL_DMA_REQ_STATUS                    DIP_X_CTL_DMA_REQ_STATUS;                        /* 00C4, 0x150220C4, DIP_A_CTL_DMA_REQ_STATUS */
	DIP_X_REG_CTL_YUV_RDY_STATUS                    DIP_X_CTL_YUV_RDY_STATUS;                        /* 00C8, 0x150220C8, DIP_A_CTL_YUV_RDY_STATUS */
	DIP_X_REG_CTL_YUV2_RDY_STATUS                   DIP_X_CTL_YUV2_RDY_STATUS;                       /* 00CC, 0x150220CC, DIP_A_CTL_YUV2_RDY_STATUS */
	DIP_X_REG_CTL_RGB_RDY_STATUS                    DIP_X_CTL_RGB_RDY_STATUS;                        /* 00D0, 0x150220D0, DIP_A_CTL_RGB_RDY_STATUS */
	DIP_X_REG_CTL_DMA_RDY_STATUS                    DIP_X_CTL_DMA_RDY_STATUS;                        /* 00D4, 0x150220D4, DIP_A_CTL_DMA_RDY_STATUS */
	DIP_X_REG_CTL_TOP_REQ_STATUS                    DIP_X_CTL_TOP_REQ_STATUS;                        /* 00D8, 0x150220D8, DIP_A_CTL_TOP_REQ_STATUS */
	DIP_X_REG_CTL_TOP_RDY_STATUS                    DIP_X_CTL_TOP_RDY_STATUS;                        /* 00DC, 0x150220DC, DIP_A_CTL_TOP_RDY_STATUS */
	DIP_X_REG_CTL_TDR_TCM3_EN                       DIP_X_CTL_TDR_TCM3_EN;                           /* 00E0, 0x150220E0, DIP_A_CTL_TDR_TCM3_EN */
	DIP_X_REG_CTL_TDR_EN_STATUS3                    DIP_X_CTL_TDR_EN_STATUS3;                        /* 00E4, 0x150220E4, DIP_A_CTL_TDR_EN_STATUS3 */
	DIP_X_REG_CTL_RGB2_EN                           DIP_X_CTL_RGB2_EN;                               /* 00E8, 0x150220E8, DIP_A_CTL_RGB2_EN */
	DIP_X_REG_CTL_RGB2_DCM_DIS                      DIP_X_CTL_RGB2_DCM_DIS;                          /* 00EC, 0x150220EC, DIP_A_CTL_RGB2_DCM_DIS */
	DIP_X_REG_CTL_RGB2_DCM_STATUS                   DIP_X_CTL_RGB2_DCM_STATUS;                       /* 00F0, 0x150220F0, DIP_A_CTL_RGB2_DCM_STATUS */
	DIP_X_REG_CTL_RGB2_REQ_STATUS                   DIP_X_CTL_RGB2_REQ_STATUS;                       /* 00F4, 0x150220F4, DIP_A_CTL_RGB2_REQ_STATUS */
	DIP_X_REG_CTL_RGB2_RDY_STATUS                   DIP_X_CTL_RGB2_RDY_STATUS;                       /* 00F8, 0x150220F8, DIP_A_CTL_RGB2_RDY_STATUS */
	UINT32                                          rsv_00FC[65];                                    /* 00FC..01FF, 0x150220FC..150221FF */
	DIP_X_REG_CQ_EN                                 DIP_X_CQ_EN;                                     /* 0200, 0x15022200, DIP_A_CQ_EN */
	DIP_X_REG_CQ_THR0_CTL                           DIP_X_CQ_THR0_CTL;                               /* 0204, 0x15022204, DIP_A_CQ_THR0_CTL */
	DIP_X_REG_CQ_THR0_BASEADDR                      DIP_X_CQ_THR0_BASEADDR;                          /* 0208, 0x15022208, DIP_A_CQ_THR0_BASEADDR */
	DIP_X_REG_CQ_THR0_DESC_SIZE                     DIP_X_CQ_THR0_DESC_SIZE;                         /* 020C, 0x1502220C, DIP_A_CQ_THR0_DESC_SIZE */
	DIP_X_REG_CQ_THR1_CTL                           DIP_X_CQ_THR1_CTL;                               /* 0210, 0x15022210, DIP_A_CQ_THR1_CTL */
	DIP_X_REG_CQ_THR1_BASEADDR                      DIP_X_CQ_THR1_BASEADDR;                          /* 0214, 0x15022214, DIP_A_CQ_THR1_BASEADDR */
	DIP_X_REG_CQ_THR1_DESC_SIZE                     DIP_X_CQ_THR1_DESC_SIZE;                         /* 0218, 0x15022218, DIP_A_CQ_THR1_DESC_SIZE */
	DIP_X_REG_CQ_THR2_CTL                           DIP_X_CQ_THR2_CTL;                               /* 021C, 0x1502221C, DIP_A_CQ_THR2_CTL */
	DIP_X_REG_CQ_THR2_BASEADDR                      DIP_X_CQ_THR2_BASEADDR;                          /* 0220, 0x15022220, DIP_A_CQ_THR2_BASEADDR */
	DIP_X_REG_CQ_THR2_DESC_SIZE                     DIP_X_CQ_THR2_DESC_SIZE;                         /* 0224, 0x15022224, DIP_A_CQ_THR2_DESC_SIZE */
	DIP_X_REG_CQ_THR3_CTL                           DIP_X_CQ_THR3_CTL;                               /* 0228, 0x15022228, DIP_A_CQ_THR3_CTL */
	DIP_X_REG_CQ_THR3_BASEADDR                      DIP_X_CQ_THR3_BASEADDR;                          /* 022C, 0x1502222C, DIP_A_CQ_THR3_BASEADDR */
	DIP_X_REG_CQ_THR3_DESC_SIZE                     DIP_X_CQ_THR3_DESC_SIZE;                         /* 0230, 0x15022230, DIP_A_CQ_THR3_DESC_SIZE */
	DIP_X_REG_CQ_THR4_CTL                           DIP_X_CQ_THR4_CTL;                               /* 0234, 0x15022234, DIP_A_CQ_THR4_CTL */
	DIP_X_REG_CQ_THR4_BASEADDR                      DIP_X_CQ_THR4_BASEADDR;                          /* 0238, 0x15022238, DIP_A_CQ_THR4_BASEADDR */
	DIP_X_REG_CQ_THR4_DESC_SIZE                     DIP_X_CQ_THR4_DESC_SIZE;                         /* 023C, 0x1502223C, DIP_A_CQ_THR4_DESC_SIZE */
	DIP_X_REG_CQ_THR5_CTL                           DIP_X_CQ_THR5_CTL;                               /* 0240, 0x15022240, DIP_A_CQ_THR5_CTL */
	DIP_X_REG_CQ_THR5_BASEADDR                      DIP_X_CQ_THR5_BASEADDR;                          /* 0244, 0x15022244, DIP_A_CQ_THR5_BASEADDR */
	DIP_X_REG_CQ_THR5_DESC_SIZE                     DIP_X_CQ_THR5_DESC_SIZE;                         /* 0248, 0x15022248, DIP_A_CQ_THR5_DESC_SIZE */
	DIP_X_REG_CQ_THR6_CTL                           DIP_X_CQ_THR6_CTL;                               /* 024C, 0x1502224C, DIP_A_CQ_THR6_CTL */
	DIP_X_REG_CQ_THR6_BASEADDR                      DIP_X_CQ_THR6_BASEADDR;                          /* 0250, 0x15022250, DIP_A_CQ_THR6_BASEADDR */
	DIP_X_REG_CQ_THR6_DESC_SIZE                     DIP_X_CQ_THR6_DESC_SIZE;                         /* 0254, 0x15022254, DIP_A_CQ_THR6_DESC_SIZE */
	DIP_X_REG_CQ_THR7_CTL                           DIP_X_CQ_THR7_CTL;                               /* 0258, 0x15022258, DIP_A_CQ_THR7_CTL */
	DIP_X_REG_CQ_THR7_BASEADDR                      DIP_X_CQ_THR7_BASEADDR;                          /* 025C, 0x1502225C, DIP_A_CQ_THR7_BASEADDR */
	DIP_X_REG_CQ_THR7_DESC_SIZE                     DIP_X_CQ_THR7_DESC_SIZE;                         /* 0260, 0x15022260, DIP_A_CQ_THR7_DESC_SIZE */
	DIP_X_REG_CQ_THR8_CTL                           DIP_X_CQ_THR8_CTL;                               /* 0264, 0x15022264, DIP_A_CQ_THR8_CTL */
	DIP_X_REG_CQ_THR8_BASEADDR                      DIP_X_CQ_THR8_BASEADDR;                          /* 0268, 0x15022268, DIP_A_CQ_THR8_BASEADDR */
	DIP_X_REG_CQ_THR8_DESC_SIZE                     DIP_X_CQ_THR8_DESC_SIZE;                         /* 026C, 0x1502226C, DIP_A_CQ_THR8_DESC_SIZE */
	DIP_X_REG_CQ_THR9_CTL                           DIP_X_CQ_THR9_CTL;                               /* 0270, 0x15022270, DIP_A_CQ_THR9_CTL */
	DIP_X_REG_CQ_THR9_BASEADDR                      DIP_X_CQ_THR9_BASEADDR;                          /* 0274, 0x15022274, DIP_A_CQ_THR9_BASEADDR */
	DIP_X_REG_CQ_THR9_DESC_SIZE                     DIP_X_CQ_THR9_DESC_SIZE;                         /* 0278, 0x15022278, DIP_A_CQ_THR9_DESC_SIZE */
	DIP_X_REG_CQ_THR10_CTL                          DIP_X_CQ_THR10_CTL;                              /* 027C, 0x1502227C, DIP_A_CQ_THR10_CTL */
	DIP_X_REG_CQ_THR10_BASEADDR                     DIP_X_CQ_THR10_BASEADDR;                         /* 0280, 0x15022280, DIP_A_CQ_THR10_BASEADDR */
	DIP_X_REG_CQ_THR10_DESC_SIZE                    DIP_X_CQ_THR10_DESC_SIZE;                        /* 0284, 0x15022284, DIP_A_CQ_THR10_DESC_SIZE */
	DIP_X_REG_CQ_THR11_CTL                          DIP_X_CQ_THR11_CTL;                              /* 0288, 0x15022288, DIP_A_CQ_THR11_CTL */
	DIP_X_REG_CQ_THR11_BASEADDR                     DIP_X_CQ_THR11_BASEADDR;                         /* 028C, 0x1502228C, DIP_A_CQ_THR11_BASEADDR */
	DIP_X_REG_CQ_THR11_DESC_SIZE                    DIP_X_CQ_THR11_DESC_SIZE;                        /* 0290, 0x15022290, DIP_A_CQ_THR11_DESC_SIZE */
	DIP_X_REG_CQ_THR12_CTL                          DIP_X_CQ_THR12_CTL;                              /* 0294, 0x15022294, DIP_A_CQ_THR12_CTL */
	DIP_X_REG_CQ_THR12_BASEADDR                     DIP_X_CQ_THR12_BASEADDR;                         /* 0298, 0x15022298, DIP_A_CQ_THR12_BASEADDR */
	DIP_X_REG_CQ_THR12_DESC_SIZE                    DIP_X_CQ_THR12_DESC_SIZE;                        /* 029C, 0x1502229C, DIP_A_CQ_THR12_DESC_SIZE */
	DIP_X_REG_CQ_THR13_CTL                          DIP_X_CQ_THR13_CTL;                              /* 02A0, 0x150222A0, DIP_A_CQ_THR13_CTL */
	DIP_X_REG_CQ_THR13_BASEADDR                     DIP_X_CQ_THR13_BASEADDR;                         /* 02A4, 0x150222A4, DIP_A_CQ_THR13_BASEADDR */
	DIP_X_REG_CQ_THR13_DESC_SIZE                    DIP_X_CQ_THR13_DESC_SIZE;                        /* 02A8, 0x150222A8, DIP_A_CQ_THR13_DESC_SIZE */
	DIP_X_REG_CQ_THR14_CTL                          DIP_X_CQ_THR14_CTL;                              /* 02AC, 0x150222AC, DIP_A_CQ_THR14_CTL */
	DIP_X_REG_CQ_THR14_BASEADDR                     DIP_X_CQ_THR14_BASEADDR;                         /* 02B0, 0x150222B0, DIP_A_CQ_THR14_BASEADDR */
	DIP_X_REG_CQ_THR14_DESC_SIZE                    DIP_X_CQ_THR14_DESC_SIZE;                        /* 02B4, 0x150222B4, DIP_A_CQ_THR14_DESC_SIZE */
	DIP_X_REG_CQ_THR15_CTL                          DIP_X_CQ_THR15_CTL;                              /* 02B8, 0x150222B8, DIP_A_CQ_THR15_CTL */
	DIP_X_REG_CQ_THR15_BASEADDR                     DIP_X_CQ_THR15_BASEADDR;                         /* 02BC, 0x150222BC, DIP_A_CQ_THR15_BASEADDR */
	DIP_X_REG_CQ_THR15_DESC_SIZE                    DIP_X_CQ_THR15_DESC_SIZE;                        /* 02C0, 0x150222C0, DIP_A_CQ_THR15_DESC_SIZE */
	DIP_X_REG_CQ_THR16_CTL                          DIP_X_CQ_THR16_CTL;                              /* 02C4, 0x150222C4, DIP_A_CQ_THR16_CTL */
	DIP_X_REG_CQ_THR16_BASEADDR                     DIP_X_CQ_THR16_BASEADDR;                         /* 02C8, 0x150222C8, DIP_A_CQ_THR16_BASEADDR */
	DIP_X_REG_CQ_THR16_DESC_SIZE                    DIP_X_CQ_THR16_DESC_SIZE;                        /* 02CC, 0x150222CC, DIP_A_CQ_THR16_DESC_SIZE */
	DIP_X_REG_CQ_THR17_CTL                          DIP_X_CQ_THR17_CTL;                              /* 02D0, 0x150222D0, DIP_A_CQ_THR17_CTL */
	DIP_X_REG_CQ_THR17_BASEADDR                     DIP_X_CQ_THR17_BASEADDR;                         /* 02D4, 0x150222D4, DIP_A_CQ_THR17_BASEADDR */
	DIP_X_REG_CQ_THR17_DESC_SIZE                    DIP_X_CQ_THR17_DESC_SIZE;                        /* 02D8, 0x150222D8, DIP_A_CQ_THR17_DESC_SIZE */
	DIP_X_REG_CQ_THR18_CTL                          DIP_X_CQ_THR18_CTL;                              /* 02DC, 0x150222DC, DIP_A_CQ_THR18_CTL */
	DIP_X_REG_CQ_THR18_BASEADDR                     DIP_X_CQ_THR18_BASEADDR;                         /* 02E0, 0x150222E0, DIP_A_CQ_THR18_BASEADDR */
	DIP_X_REG_CQ_THR18_DESC_SIZE                    DIP_X_CQ_THR18_DESC_SIZE;                        /* 02E4, 0x150222E4, DIP_A_CQ_THR18_DESC_SIZE */
	UINT32                                          rsv_02E8[6];                                     /* 02E8..02FF, 0x150222E8..150222FF */
	DIP_X_REG_DMA_SOFT_RSTSTAT                      DIP_X_DMA_SOFT_RSTSTAT;                          /* 0300, 0x15022300, DIP_A_DMA_SOFT_RSTSTAT */
	DIP_X_REG_TDRI_BASE_ADDR                        DIP_X_TDRI_BASE_ADDR;                            /* 0304, 0x15022304, DIP_A_TDRI_BASE_ADDR */
	DIP_X_REG_TDRI_OFST_ADDR                        DIP_X_TDRI_OFST_ADDR;                            /* 0308, 0x15022308, DIP_A_TDRI_OFST_ADDR */
	DIP_X_REG_TDRI_XSIZE                            DIP_X_TDRI_XSIZE;                                /* 030C, 0x1502230C, DIP_A_TDRI_XSIZE */
	DIP_X_REG_VERTICAL_FLIP_EN                      DIP_X_VERTICAL_FLIP_EN;                          /* 0310, 0x15022310, DIP_A_VERTICAL_FLIP_EN */
	DIP_X_REG_DMA_SOFT_RESET                        DIP_X_DMA_SOFT_RESET;                            /* 0314, 0x15022314, DIP_A_DMA_SOFT_RESET */
	DIP_X_REG_LAST_ULTRA_EN                         DIP_X_LAST_ULTRA_EN;                             /* 0318, 0x15022318, DIP_A_LAST_ULTRA_EN */
	DIP_X_REG_SPECIAL_FUN_EN                        DIP_X_SPECIAL_FUN_EN;                            /* 031C, 0x1502231C, DIP_A_SPECIAL_FUN_EN */
	UINT32                                          rsv_0320[4];                                     /* 0320..032F, 0x15022320..1502232F */
	DIP_X_REG_IMG2O_BASE_ADDR                       DIP_X_IMG2O_BASE_ADDR;                           /* 0330, 0x15022330, DIP_A_IMG2O_BASE_ADDR */
	UINT32                                          rsv_0334;                                        /* 0334, 0x15022334 */
	DIP_X_REG_IMG2O_OFST_ADDR                       DIP_X_IMG2O_OFST_ADDR;                           /* 0338, 0x15022338, DIP_A_IMG2O_OFST_ADDR */
	UINT32                                          rsv_033C;                                        /* 033C, 0x1502233C */
	DIP_X_REG_IMG2O_XSIZE                           DIP_X_IMG2O_XSIZE;                               /* 0340, 0x15022340, DIP_A_IMG2O_XSIZE */
	DIP_X_REG_IMG2O_YSIZE                           DIP_X_IMG2O_YSIZE;                               /* 0344, 0x15022344, DIP_A_IMG2O_YSIZE */
	DIP_X_REG_IMG2O_STRIDE                          DIP_X_IMG2O_STRIDE;                              /* 0348, 0x15022348, DIP_A_IMG2O_STRIDE */
	DIP_X_REG_IMG2O_CON                             DIP_X_IMG2O_CON;                                 /* 034C, 0x1502234C, DIP_A_IMG2O_CON */
	DIP_X_REG_IMG2O_CON2                            DIP_X_IMG2O_CON2;                                /* 0350, 0x15022350, DIP_A_IMG2O_CON2 */
	DIP_X_REG_IMG2O_CON3                            DIP_X_IMG2O_CON3;                                /* 0354, 0x15022354, DIP_A_IMG2O_CON3 */
	DIP_X_REG_IMG2O_CROP                            DIP_X_IMG2O_CROP;                                /* 0358, 0x15022358, DIP_A_IMG2O_CROP */
	UINT32                                          rsv_035C;                                        /* 035C, 0x1502235C */
	DIP_X_REG_IMG2BO_BASE_ADDR                      DIP_X_IMG2BO_BASE_ADDR;                          /* 0360, 0x15022360, DIP_A_IMG2BO_BASE_ADDR */
	UINT32                                          rsv_0364;                                        /* 0364, 0x15022364 */
	DIP_X_REG_IMG2BO_OFST_ADDR                      DIP_X_IMG2BO_OFST_ADDR;                          /* 0368, 0x15022368, DIP_A_IMG2BO_OFST_ADDR */
	UINT32                                          rsv_036C;                                        /* 036C, 0x1502236C */
	DIP_X_REG_IMG2BO_XSIZE                          DIP_X_IMG2BO_XSIZE;                              /* 0370, 0x15022370, DIP_A_IMG2BO_XSIZE */
	DIP_X_REG_IMG2BO_YSIZE                          DIP_X_IMG2BO_YSIZE;                              /* 0374, 0x15022374, DIP_A_IMG2BO_YSIZE */
	DIP_X_REG_IMG2BO_STRIDE                         DIP_X_IMG2BO_STRIDE;                             /* 0378, 0x15022378, DIP_A_IMG2BO_STRIDE */
	DIP_X_REG_IMG2BO_CON                            DIP_X_IMG2BO_CON;                                /* 037C, 0x1502237C, DIP_A_IMG2BO_CON */
	DIP_X_REG_IMG2BO_CON2                           DIP_X_IMG2BO_CON2;                               /* 0380, 0x15022380, DIP_A_IMG2BO_CON2 */
	DIP_X_REG_IMG2BO_CON3                           DIP_X_IMG2BO_CON3;                               /* 0384, 0x15022384, DIP_A_IMG2BO_CON3 */
	DIP_X_REG_IMG2BO_CROP                           DIP_X_IMG2BO_CROP;                               /* 0388, 0x15022388, DIP_A_IMG2BO_CROP */
	UINT32                                          rsv_038C;                                        /* 038C, 0x1502238C */
	DIP_X_REG_IMG3O_BASE_ADDR                       DIP_X_IMG3O_BASE_ADDR;                           /* 0390, 0x15022390, DIP_A_IMG3O_BASE_ADDR */
	UINT32                                          rsv_0394;                                        /* 0394, 0x15022394 */
	DIP_X_REG_IMG3O_OFST_ADDR                       DIP_X_IMG3O_OFST_ADDR;                           /* 0398, 0x15022398, DIP_A_IMG3O_OFST_ADDR */
	UINT32                                          rsv_039C;                                        /* 039C, 0x1502239C */
	DIP_X_REG_IMG3O_XSIZE                           DIP_X_IMG3O_XSIZE;                               /* 03A0, 0x150223A0, DIP_A_IMG3O_XSIZE */
	DIP_X_REG_IMG3O_YSIZE                           DIP_X_IMG3O_YSIZE;                               /* 03A4, 0x150223A4, DIP_A_IMG3O_YSIZE */
	DIP_X_REG_IMG3O_STRIDE                          DIP_X_IMG3O_STRIDE;                              /* 03A8, 0x150223A8, DIP_A_IMG3O_STRIDE */
	DIP_X_REG_IMG3O_CON                             DIP_X_IMG3O_CON;                                 /* 03AC, 0x150223AC, DIP_A_IMG3O_CON */
	DIP_X_REG_IMG3O_CON2                            DIP_X_IMG3O_CON2;                                /* 03B0, 0x150223B0, DIP_A_IMG3O_CON2 */
	DIP_X_REG_IMG3O_CON3                            DIP_X_IMG3O_CON3;                                /* 03B4, 0x150223B4, DIP_A_IMG3O_CON3 */
	DIP_X_REG_IMG3O_CROP                            DIP_X_IMG3O_CROP;                                /* 03B8, 0x150223B8, DIP_A_IMG3O_CROP */
	UINT32                                          rsv_03BC;                                        /* 03BC, 0x150223BC */
	DIP_X_REG_IMG3BO_BASE_ADDR                      DIP_X_IMG3BO_BASE_ADDR;                          /* 03C0, 0x150223C0, DIP_A_IMG3BO_BASE_ADDR */
	UINT32                                          rsv_03C4;                                        /* 03C4, 0x150223C4 */
	DIP_X_REG_IMG3BO_OFST_ADDR                      DIP_X_IMG3BO_OFST_ADDR;                          /* 03C8, 0x150223C8, DIP_A_IMG3BO_OFST_ADDR */
	UINT32                                          rsv_03CC;                                        /* 03CC, 0x150223CC */
	DIP_X_REG_IMG3BO_XSIZE                          DIP_X_IMG3BO_XSIZE;                              /* 03D0, 0x150223D0, DIP_A_IMG3BO_XSIZE */
	DIP_X_REG_IMG3BO_YSIZE                          DIP_X_IMG3BO_YSIZE;                              /* 03D4, 0x150223D4, DIP_A_IMG3BO_YSIZE */
	DIP_X_REG_IMG3BO_STRIDE                         DIP_X_IMG3BO_STRIDE;                             /* 03D8, 0x150223D8, DIP_A_IMG3BO_STRIDE */
	DIP_X_REG_IMG3BO_CON                            DIP_X_IMG3BO_CON;                                /* 03DC, 0x150223DC, DIP_A_IMG3BO_CON */
	DIP_X_REG_IMG3BO_CON2                           DIP_X_IMG3BO_CON2;                               /* 03E0, 0x150223E0, DIP_A_IMG3BO_CON2 */
	DIP_X_REG_IMG3BO_CON3                           DIP_X_IMG3BO_CON3;                               /* 03E4, 0x150223E4, DIP_A_IMG3BO_CON3 */
	DIP_X_REG_IMG3BO_CROP                           DIP_X_IMG3BO_CROP;                               /* 03E8, 0x150223E8, DIP_A_IMG3BO_CROP */
	UINT32                                          rsv_03EC;                                        /* 03EC, 0x150223EC */
	DIP_X_REG_IMG3CO_BASE_ADDR                      DIP_X_IMG3CO_BASE_ADDR;                          /* 03F0, 0x150223F0, DIP_A_IMG3CO_BASE_ADDR */
	UINT32                                          rsv_03F4;                                        /* 03F4, 0x150223F4 */
	DIP_X_REG_IMG3CO_OFST_ADDR                      DIP_X_IMG3CO_OFST_ADDR;                          /* 03F8, 0x150223F8, DIP_A_IMG3CO_OFST_ADDR */
	UINT32                                          rsv_03FC;                                        /* 03FC, 0x150223FC */
	DIP_X_REG_IMG3CO_XSIZE                          DIP_X_IMG3CO_XSIZE;                              /* 0400, 0x15022400, DIP_A_IMG3CO_XSIZE */
	DIP_X_REG_IMG3CO_YSIZE                          DIP_X_IMG3CO_YSIZE;                              /* 0404, 0x15022404, DIP_A_IMG3CO_YSIZE */
	DIP_X_REG_IMG3CO_STRIDE                         DIP_X_IMG3CO_STRIDE;                             /* 0408, 0x15022408, DIP_A_IMG3CO_STRIDE */
	DIP_X_REG_IMG3CO_CON                            DIP_X_IMG3CO_CON;                                /* 040C, 0x1502240C, DIP_A_IMG3CO_CON */
	DIP_X_REG_IMG3CO_CON2                           DIP_X_IMG3CO_CON2;                               /* 0410, 0x15022410, DIP_A_IMG3CO_CON2 */
	DIP_X_REG_IMG3CO_CON3                           DIP_X_IMG3CO_CON3;                               /* 0414, 0x15022414, DIP_A_IMG3CO_CON3 */
	DIP_X_REG_IMG3CO_CROP                           DIP_X_IMG3CO_CROP;                               /* 0418, 0x15022418, DIP_A_IMG3CO_CROP */
	UINT32                                          rsv_041C;                                        /* 041C, 0x1502241C */
	DIP_X_REG_FEO_BASE_ADDR                         DIP_X_FEO_BASE_ADDR;                             /* 0420, 0x15022420, DIP_A_FEO_BASE_ADDR */
	UINT32                                          rsv_0424;                                        /* 0424, 0x15022424 */
	DIP_X_REG_FEO_OFST_ADDR                         DIP_X_FEO_OFST_ADDR;                             /* 0428, 0x15022428, DIP_A_FEO_OFST_ADDR */
	UINT32                                          rsv_042C;                                        /* 042C, 0x1502242C */
	DIP_X_REG_FEO_XSIZE                             DIP_X_FEO_XSIZE;                                 /* 0430, 0x15022430, DIP_A_FEO_XSIZE */
	DIP_X_REG_FEO_YSIZE                             DIP_X_FEO_YSIZE;                                 /* 0434, 0x15022434, DIP_A_FEO_YSIZE */
	DIP_X_REG_FEO_STRIDE                            DIP_X_FEO_STRIDE;                                /* 0438, 0x15022438, DIP_A_FEO_STRIDE */
	DIP_X_REG_FEO_CON                               DIP_X_FEO_CON;                                   /* 043C, 0x1502243C, DIP_A_FEO_CON */
	DIP_X_REG_FEO_CON2                              DIP_X_FEO_CON2;                                  /* 0440, 0x15022440, DIP_A_FEO_CON2 */
	DIP_X_REG_FEO_CON3                              DIP_X_FEO_CON3;                                  /* 0444, 0x15022444, DIP_A_FEO_CON3 */
	UINT32                                          rsv_0448[2];                                     /* 0448..044F, 0x15022448..1502244F */
	DIP_X_REG_PAK2O_BASE_ADDR                       DIP_X_PAK2O_BASE_ADDR;                           /* 0450, 0x15022450, DIP_A_PAK2O_BASE_ADDR */
	UINT32                                          rsv_0454;                                        /* 0454, 0x15022454 */
	DIP_X_REG_PAK2O_OFST_ADDR                       DIP_X_PAK2O_OFST_ADDR;                           /* 0458, 0x15022458, DIP_A_PAK2O_OFST_ADDR */
	UINT32                                          rsv_045C;                                        /* 045C, 0x1502245C */
	DIP_X_REG_PAK2O_XSIZE                           DIP_X_PAK2O_XSIZE;                               /* 0460, 0x15022460, DIP_A_PAK2O_XSIZE */
	DIP_X_REG_PAK2O_YSIZE                           DIP_X_PAK2O_YSIZE;                               /* 0464, 0x15022464, DIP_A_PAK2O_YSIZE */
	DIP_X_REG_PAK2O_STRIDE                          DIP_X_PAK2O_STRIDE;                              /* 0468, 0x15022468, DIP_A_PAK2O_STRIDE */
	DIP_X_REG_PAK2O_CON                             DIP_X_PAK2O_CON;                                 /* 046C, 0x1502246C, DIP_A_PAK2O_CON */
	DIP_X_REG_PAK2O_CON2                            DIP_X_PAK2O_CON2;                                /* 0470, 0x15022470, DIP_A_PAK2O_CON2 */
	DIP_X_REG_PAK2O_CON3                            DIP_X_PAK2O_CON3;                                /* 0474, 0x15022474, DIP_A_PAK2O_CON3 */
	DIP_X_REG_PAK2O_CROP                            DIP_X_PAK2O_CROP;                                /* 0478, 0x15022478, DIP_A_PAK2O_CROP */
	UINT32                                          rsv_047C[33];                                    /* 047C..04FF, 0x1502247C..150224FF */
	DIP_X_REG_IMGI_BASE_ADDR                        DIP_X_IMGI_BASE_ADDR;                            /* 0500, 0x15022500, DIP_A_IMGI_BASE_ADDR */
	UINT32                                          rsv_0504;                                        /* 0504, 0x15022504 */
	DIP_X_REG_IMGI_OFST_ADDR                        DIP_X_IMGI_OFST_ADDR;                            /* 0508, 0x15022508, DIP_A_IMGI_OFST_ADDR */
	UINT32                                          rsv_050C;                                        /* 050C, 0x1502250C */
	DIP_X_REG_IMGI_XSIZE                            DIP_X_IMGI_XSIZE;                                /* 0510, 0x15022510, DIP_A_IMGI_XSIZE */
	DIP_X_REG_IMGI_YSIZE                            DIP_X_IMGI_YSIZE;                                /* 0514, 0x15022514, DIP_A_IMGI_YSIZE */
	DIP_X_REG_IMGI_STRIDE                           DIP_X_IMGI_STRIDE;                               /* 0518, 0x15022518, DIP_A_IMGI_STRIDE */
	DIP_X_REG_IMGI_CON                              DIP_X_IMGI_CON;                                  /* 051C, 0x1502251C, DIP_A_IMGI_CON */
	DIP_X_REG_IMGI_CON2                             DIP_X_IMGI_CON2;                                 /* 0520, 0x15022520, DIP_A_IMGI_CON2 */
	DIP_X_REG_IMGI_CON3                             DIP_X_IMGI_CON3;                                 /* 0524, 0x15022524, DIP_A_IMGI_CON3 */
	UINT32                                          rsv_0528[2];                                     /* 0528..052F, 0x15022528..1502252F */
	DIP_X_REG_IMGBI_BASE_ADDR                       DIP_X_IMGBI_BASE_ADDR;                           /* 0530, 0x15022530, DIP_A_IMGBI_BASE_ADDR */
	UINT32                                          rsv_0534;                                        /* 0534, 0x15022534 */
	DIP_X_REG_IMGBI_OFST_ADDR                       DIP_X_IMGBI_OFST_ADDR;                           /* 0538, 0x15022538, DIP_A_IMGBI_OFST_ADDR */
	UINT32                                          rsv_053C;                                        /* 053C, 0x1502253C */
	DIP_X_REG_IMGBI_XSIZE                           DIP_X_IMGBI_XSIZE;                               /* 0540, 0x15022540, DIP_A_IMGBI_XSIZE */
	DIP_X_REG_IMGBI_YSIZE                           DIP_X_IMGBI_YSIZE;                               /* 0544, 0x15022544, DIP_A_IMGBI_YSIZE */
	DIP_X_REG_IMGBI_STRIDE                          DIP_X_IMGBI_STRIDE;                              /* 0548, 0x15022548, DIP_A_IMGBI_STRIDE */
	DIP_X_REG_IMGBI_CON                             DIP_X_IMGBI_CON;                                 /* 054C, 0x1502254C, DIP_A_IMGBI_CON */
	DIP_X_REG_IMGBI_CON2                            DIP_X_IMGBI_CON2;                                /* 0550, 0x15022550, DIP_A_IMGBI_CON2 */
	DIP_X_REG_IMGBI_CON3                            DIP_X_IMGBI_CON3;                                /* 0554, 0x15022554, DIP_A_IMGBI_CON3 */
	UINT32                                          rsv_0558[2];                                     /* 0558..055F, 0x15022558..1502255F */
	DIP_X_REG_IMGCI_BASE_ADDR                       DIP_X_IMGCI_BASE_ADDR;                           /* 0560, 0x15022560, DIP_A_IMGCI_BASE_ADDR */
	UINT32                                          rsv_0564;                                        /* 0564, 0x15022564 */
	DIP_X_REG_IMGCI_OFST_ADDR                       DIP_X_IMGCI_OFST_ADDR;                           /* 0568, 0x15022568, DIP_A_IMGCI_OFST_ADDR */
	UINT32                                          rsv_056C;                                        /* 056C, 0x1502256C */
	DIP_X_REG_IMGCI_XSIZE                           DIP_X_IMGCI_XSIZE;                               /* 0570, 0x15022570, DIP_A_IMGCI_XSIZE */
	DIP_X_REG_IMGCI_YSIZE                           DIP_X_IMGCI_YSIZE;                               /* 0574, 0x15022574, DIP_A_IMGCI_YSIZE */
	DIP_X_REG_IMGCI_STRIDE                          DIP_X_IMGCI_STRIDE;                              /* 0578, 0x15022578, DIP_A_IMGCI_STRIDE */
	DIP_X_REG_IMGCI_CON                             DIP_X_IMGCI_CON;                                 /* 057C, 0x1502257C, DIP_A_IMGCI_CON */
	DIP_X_REG_IMGCI_CON2                            DIP_X_IMGCI_CON2;                                /* 0580, 0x15022580, DIP_A_IMGCI_CON2 */
	DIP_X_REG_IMGCI_CON3                            DIP_X_IMGCI_CON3;                                /* 0584, 0x15022584, DIP_A_IMGCI_CON3 */
	UINT32                                          rsv_0588[2];                                     /* 0588..058F, 0x15022588..1502258F */
	DIP_X_REG_VIPI_BASE_ADDR                        DIP_X_VIPI_BASE_ADDR;                            /* 0590, 0x15022590, DIP_A_VIPI_BASE_ADDR */
	UINT32                                          rsv_0594;                                        /* 0594, 0x15022594 */
	DIP_X_REG_VIPI_OFST_ADDR                        DIP_X_VIPI_OFST_ADDR;                            /* 0598, 0x15022598, DIP_A_VIPI_OFST_ADDR */
	UINT32                                          rsv_059C;                                        /* 059C, 0x1502259C */
	DIP_X_REG_VIPI_XSIZE                            DIP_X_VIPI_XSIZE;                                /* 05A0, 0x150225A0, DIP_A_VIPI_XSIZE */
	DIP_X_REG_VIPI_YSIZE                            DIP_X_VIPI_YSIZE;                                /* 05A4, 0x150225A4, DIP_A_VIPI_YSIZE */
	DIP_X_REG_VIPI_STRIDE                           DIP_X_VIPI_STRIDE;                               /* 05A8, 0x150225A8, DIP_A_VIPI_STRIDE */
	DIP_X_REG_VIPI_CON                              DIP_X_VIPI_CON;                                  /* 05AC, 0x150225AC, DIP_A_VIPI_CON */
	DIP_X_REG_VIPI_CON2                             DIP_X_VIPI_CON2;                                 /* 05B0, 0x150225B0, DIP_A_VIPI_CON2 */
	DIP_X_REG_VIPI_CON3                             DIP_X_VIPI_CON3;                                 /* 05B4, 0x150225B4, DIP_A_VIPI_CON3 */
	UINT32                                          rsv_05B8[2];                                     /* 05B8..05BF, 0x150225B8..150225BF */
	DIP_X_REG_VIP2I_BASE_ADDR                       DIP_X_VIP2I_BASE_ADDR;                           /* 05C0, 0x150225C0, DIP_A_VIP2I_BASE_ADDR */
	UINT32                                          rsv_05C4;                                        /* 05C4, 0x150225C4 */
	DIP_X_REG_VIP2I_OFST_ADDR                       DIP_X_VIP2I_OFST_ADDR;                           /* 05C8, 0x150225C8, DIP_A_VIP2I_OFST_ADDR */
	UINT32                                          rsv_05CC;                                        /* 05CC, 0x150225CC */
	DIP_X_REG_VIP2I_XSIZE                           DIP_X_VIP2I_XSIZE;                               /* 05D0, 0x150225D0, DIP_A_VIP2I_XSIZE */
	DIP_X_REG_VIP2I_YSIZE                           DIP_X_VIP2I_YSIZE;                               /* 05D4, 0x150225D4, DIP_A_VIP2I_YSIZE */
	DIP_X_REG_VIP2I_STRIDE                          DIP_X_VIP2I_STRIDE;                              /* 05D8, 0x150225D8, DIP_A_VIP2I_STRIDE */
	DIP_X_REG_VIP2I_CON                             DIP_X_VIP2I_CON;                                 /* 05DC, 0x150225DC, DIP_A_VIP2I_CON */
	DIP_X_REG_VIP2I_CON2                            DIP_X_VIP2I_CON2;                                /* 05E0, 0x150225E0, DIP_A_VIP2I_CON2 */
	DIP_X_REG_VIP2I_CON3                            DIP_X_VIP2I_CON3;                                /* 05E4, 0x150225E4, DIP_A_VIP2I_CON3 */
	UINT32                                          rsv_05E8[2];                                     /* 05E8..05EF, 0x150225E8..150225EF */
	DIP_X_REG_VIP3I_BASE_ADDR                       DIP_X_VIP3I_BASE_ADDR;                           /* 05F0, 0x150225F0, DIP_A_VIP3I_BASE_ADDR */
	UINT32                                          rsv_05F4;                                        /* 05F4, 0x150225F4 */
	DIP_X_REG_VIP3I_OFST_ADDR                       DIP_X_VIP3I_OFST_ADDR;                           /* 05F8, 0x150225F8, DIP_A_VIP3I_OFST_ADDR */
	UINT32                                          rsv_05FC;                                        /* 05FC, 0x150225FC */
	DIP_X_REG_VIP3I_XSIZE                           DIP_X_VIP3I_XSIZE;                               /* 0600, 0x15022600, DIP_A_VIP3I_XSIZE */
	DIP_X_REG_VIP3I_YSIZE                           DIP_X_VIP3I_YSIZE;                               /* 0604, 0x15022604, DIP_A_VIP3I_YSIZE */
	DIP_X_REG_VIP3I_STRIDE                          DIP_X_VIP3I_STRIDE;                              /* 0608, 0x15022608, DIP_A_VIP3I_STRIDE */
	DIP_X_REG_VIP3I_CON                             DIP_X_VIP3I_CON;                                 /* 060C, 0x1502260C, DIP_A_VIP3I_CON */
	DIP_X_REG_VIP3I_CON2                            DIP_X_VIP3I_CON2;                                /* 0610, 0x15022610, DIP_A_VIP3I_CON2 */
	DIP_X_REG_VIP3I_CON3                            DIP_X_VIP3I_CON3;                                /* 0614, 0x15022614, DIP_A_VIP3I_CON3 */
	UINT32                                          rsv_0618[2];                                     /* 0618..061F, 0x15022618..1502261F */
	DIP_X_REG_DMGI_BASE_ADDR                        DIP_X_DMGI_BASE_ADDR;                            /* 0620, 0x15022620, DIP_A_DMGI_BASE_ADDR */
	UINT32                                          rsv_0624;                                        /* 0624, 0x15022624 */
	DIP_X_REG_DMGI_OFST_ADDR                        DIP_X_DMGI_OFST_ADDR;                            /* 0628, 0x15022628, DIP_A_DMGI_OFST_ADDR */
	UINT32                                          rsv_062C;                                        /* 062C, 0x1502262C */
	DIP_X_REG_DMGI_XSIZE                            DIP_X_DMGI_XSIZE;                                /* 0630, 0x15022630, DIP_A_DMGI_XSIZE */
	DIP_X_REG_DMGI_YSIZE                            DIP_X_DMGI_YSIZE;                                /* 0634, 0x15022634, DIP_A_DMGI_YSIZE */
	DIP_X_REG_DMGI_STRIDE                           DIP_X_DMGI_STRIDE;                               /* 0638, 0x15022638, DIP_A_DMGI_STRIDE */
	DIP_X_REG_DMGI_CON                              DIP_X_DMGI_CON;                                  /* 063C, 0x1502263C, DIP_A_DMGI_CON */
	DIP_X_REG_DMGI_CON2                             DIP_X_DMGI_CON2;                                 /* 0640, 0x15022640, DIP_A_DMGI_CON2 */
	DIP_X_REG_DMGI_CON3                             DIP_X_DMGI_CON3;                                 /* 0644, 0x15022644, DIP_A_DMGI_CON3 */
	UINT32                                          rsv_0648[2];                                     /* 0648..064F, 0x15022648..1502264F */
	DIP_X_REG_DEPI_BASE_ADDR                        DIP_X_DEPI_BASE_ADDR;                            /* 0650, 0x15022650, DIP_A_DEPI_BASE_ADDR */
	UINT32                                          rsv_0654;                                        /* 0654, 0x15022654 */
	DIP_X_REG_DEPI_OFST_ADDR                        DIP_X_DEPI_OFST_ADDR;                            /* 0658, 0x15022658, DIP_A_DEPI_OFST_ADDR */
	UINT32                                          rsv_065C;                                        /* 065C, 0x1502265C */
	DIP_X_REG_DEPI_XSIZE                            DIP_X_DEPI_XSIZE;                                /* 0660, 0x15022660, DIP_A_DEPI_XSIZE */
	DIP_X_REG_DEPI_YSIZE                            DIP_X_DEPI_YSIZE;                                /* 0664, 0x15022664, DIP_A_DEPI_YSIZE */
	DIP_X_REG_DEPI_STRIDE                           DIP_X_DEPI_STRIDE;                               /* 0668, 0x15022668, DIP_A_DEPI_STRIDE */
	DIP_X_REG_DEPI_CON                              DIP_X_DEPI_CON;                                  /* 066C, 0x1502266C, DIP_A_DEPI_CON */
	DIP_X_REG_DEPI_CON2                             DIP_X_DEPI_CON2;                                 /* 0670, 0x15022670, DIP_A_DEPI_CON2 */
	DIP_X_REG_DEPI_CON3                             DIP_X_DEPI_CON3;                                 /* 0674, 0x15022674, DIP_A_DEPI_CON3 */
	UINT32                                          rsv_0678[2];                                     /* 0678..067F, 0x15022678..1502267F */
	DIP_X_REG_LCEI_BASE_ADDR                        DIP_X_LCEI_BASE_ADDR;                            /* 0680, 0x15022680, DIP_A_LCEI_BASE_ADDR */
	UINT32                                          rsv_0684;                                        /* 0684, 0x15022684 */
	DIP_X_REG_LCEI_OFST_ADDR                        DIP_X_LCEI_OFST_ADDR;                            /* 0688, 0x15022688, DIP_A_LCEI_OFST_ADDR */
	UINT32                                          rsv_068C;                                        /* 068C, 0x1502268C */
	DIP_X_REG_LCEI_XSIZE                            DIP_X_LCEI_XSIZE;                                /* 0690, 0x15022690, DIP_A_LCEI_XSIZE */
	DIP_X_REG_LCEI_YSIZE                            DIP_X_LCEI_YSIZE;                                /* 0694, 0x15022694, DIP_A_LCEI_YSIZE */
	DIP_X_REG_LCEI_STRIDE                           DIP_X_LCEI_STRIDE;                               /* 0698, 0x15022698, DIP_A_LCEI_STRIDE */
	DIP_X_REG_LCEI_CON                              DIP_X_LCEI_CON;                                  /* 069C, 0x1502269C, DIP_A_LCEI_CON */
	DIP_X_REG_LCEI_CON2                             DIP_X_LCEI_CON2;                                 /* 06A0, 0x150226A0, DIP_A_LCEI_CON2 */
	DIP_X_REG_LCEI_CON3                             DIP_X_LCEI_CON3;                                 /* 06A4, 0x150226A4, DIP_A_LCEI_CON3 */
	UINT32                                          rsv_06A8[2];                                     /* 06A8..06AF, 0x150226A8..150226AF */
	DIP_X_REG_UFDI_BASE_ADDR                        DIP_X_UFDI_BASE_ADDR;                            /* 06B0, 0x150226B0, DIP_A_UFDI_BASE_ADDR */
	UINT32                                          rsv_06B4;                                        /* 06B4, 0x150226B4 */
	DIP_X_REG_UFDI_OFST_ADDR                        DIP_X_UFDI_OFST_ADDR;                            /* 06B8, 0x150226B8, DIP_A_UFDI_OFST_ADDR */
	UINT32                                          rsv_06BC;                                        /* 06BC, 0x150226BC */
	DIP_X_REG_UFDI_XSIZE                            DIP_X_UFDI_XSIZE;                                /* 06C0, 0x150226C0, DIP_A_UFDI_XSIZE */
	DIP_X_REG_UFDI_YSIZE                            DIP_X_UFDI_YSIZE;                                /* 06C4, 0x150226C4, DIP_A_UFDI_YSIZE */
	DIP_X_REG_UFDI_STRIDE                           DIP_X_UFDI_STRIDE;                               /* 06C8, 0x150226C8, DIP_A_UFDI_STRIDE */
	DIP_X_REG_UFDI_CON                              DIP_X_UFDI_CON;                                  /* 06CC, 0x150226CC, DIP_A_UFDI_CON */
	DIP_X_REG_UFDI_CON2                             DIP_X_UFDI_CON2;                                 /* 06D0, 0x150226D0, DIP_A_UFDI_CON2 */
	DIP_X_REG_UFDI_CON3                             DIP_X_UFDI_CON3;                                 /* 06D4, 0x150226D4, DIP_A_UFDI_CON3 */
	UINT32                                          rsv_06D8[26];                                    /* 06D8..073F, 0x150226D8..1502273F */
	DIP_X_REG_DMA_ERR_CTRL                          DIP_X_DMA_ERR_CTRL;                              /* 0740, 0x15022740, DIP_A_DMA_ERR_CTRL */
	DIP_X_REG_IMG2O_ERR_STAT                        DIP_X_IMG2O_ERR_STAT;                            /* 0744, 0x15022744, DIP_A_IMG2O_ERR_STAT */
	DIP_X_REG_IMG2BO_ERR_STAT                       DIP_X_IMG2BO_ERR_STAT;                           /* 0748, 0x15022748, DIP_A_IMG2BO_ERR_STAT */
	DIP_X_REG_IMG3O_ERR_STAT                        DIP_X_IMG3O_ERR_STAT;                            /* 074C, 0x1502274C, DIP_A_IMG3O_ERR_STAT */
	DIP_X_REG_IMG3BO_ERR_STAT                       DIP_X_IMG3BO_ERR_STAT;                           /* 0750, 0x15022750, DIP_A_IMG3BO_ERR_STAT */
	DIP_X_REG_IMG3CO_ERR_STAT                       DIP_X_IMG3CO_ERR_STAT;                           /* 0754, 0x15022754, DIP_A_IMG3CO_ERR_STAT */
	DIP_X_REG_FEO_ERR_STAT                          DIP_X_FEO_ERR_STAT;                              /* 0758, 0x15022758, DIP_A_FEO_ERR_STAT */
	DIP_X_REG_PAK2O_ERR_STAT                        DIP_X_PAK2O_ERR_STAT;                            /* 075C, 0x1502275C, DIP_A_PAK2O_ERR_STAT */
	DIP_X_REG_IMGI_ERR_STAT                         DIP_X_IMGI_ERR_STAT;                             /* 0760, 0x15022760, DIP_A_IMGI_ERR_STAT */
	DIP_X_REG_IMGBI_ERR_STAT                        DIP_X_IMGBI_ERR_STAT;                            /* 0764, 0x15022764, DIP_A_IMGBI_ERR_STAT */
	DIP_X_REG_IMGCI_ERR_STAT                        DIP_X_IMGCI_ERR_STAT;                            /* 0768, 0x15022768, DIP_A_IMGCI_ERR_STAT */
	DIP_X_REG_VIPI_ERR_STAT                         DIP_X_VIPI_ERR_STAT;                             /* 076C, 0x1502276C, DIP_A_VIPI_ERR_STAT */
	DIP_X_REG_VIP2I_ERR_STAT                        DIP_X_VIP2I_ERR_STAT;                            /* 0770, 0x15022770, DIP_A_VIP2I_ERR_STAT */
	DIP_X_REG_VIP3I_ERR_STAT                        DIP_X_VIP3I_ERR_STAT;                            /* 0774, 0x15022774, DIP_A_VIP3I_ERR_STAT */
	DIP_X_REG_DMGI_ERR_STAT                         DIP_X_DMGI_ERR_STAT;                             /* 0778, 0x15022778, DIP_A_DMGI_ERR_STAT */
	DIP_X_REG_DEPI_ERR_STAT                         DIP_X_DEPI_ERR_STAT;                             /* 077C, 0x1502277C, DIP_A_DEPI_ERR_STAT */
	DIP_X_REG_LCEI_ERR_STAT                         DIP_X_LCEI_ERR_STAT;                             /* 0780, 0x15022780, DIP_A_LCEI_ERR_STAT */
	DIP_X_REG_UFDI_ERR_STAT                         DIP_X_UFDI_ERR_STAT;                             /* 0784, 0x15022784, DIP_A_UFDI_ERR_STAT */
	DIP_X_REG_SMX1O_ERR_STAT                        DIP_X_SMX1O_ERR_STAT;                            /* 0788, 0x15022788, DIP_A_SMX1O_ERR_STAT */
	DIP_X_REG_SMX2O_ERR_STAT                        DIP_X_SMX2O_ERR_STAT;                            /* 078C, 0x1502278C, DIP_A_SMX2O_ERR_STAT */
	DIP_X_REG_SMX3O_ERR_STAT                        DIP_X_SMX3O_ERR_STAT;                            /* 0790, 0x15022790, DIP_A_SMX3O_ERR_STAT */
	DIP_X_REG_SMX4O_ERR_STAT                        DIP_X_SMX4O_ERR_STAT;                            /* 0794, 0x15022794, DIP_A_SMX4O_ERR_STAT */
	DIP_X_REG_SMX1I_ERR_STAT                        DIP_X_SMX1I_ERR_STAT;                            /* 0798, 0x15022798, DIP_A_SMX1I_ERR_STAT */
	DIP_X_REG_SMX2I_ERR_STAT                        DIP_X_SMX2I_ERR_STAT;                            /* 079C, 0x1502279C, DIP_A_SMX2I_ERR_STAT */
	DIP_X_REG_SMX3I_ERR_STAT                        DIP_X_SMX3I_ERR_STAT;                            /* 07A0, 0x150227A0, DIP_A_SMX3I_ERR_STAT */
	DIP_X_REG_SMX4I_ERR_STAT                        DIP_X_SMX4I_ERR_STAT;                            /* 07A4, 0x150227A4, DIP_A_SMX4I_ERR_STAT */
	DIP_X_REG_DMA_DEBUG_ADDR                        DIP_X_DMA_DEBUG_ADDR;                            /* 07A8, 0x150227A8, DIP_A_DMA_DEBUG_ADDR */
	DIP_X_REG_DMA_RSV1                              DIP_X_DMA_RSV1;                                  /* 07AC, 0x150227AC, DIP_A_DMA_RSV1 */
	DIP_X_REG_DMA_RSV2                              DIP_X_DMA_RSV2;                                  /* 07B0, 0x150227B0, DIP_A_DMA_RSV2 */
	DIP_X_REG_DMA_RSV3                              DIP_X_DMA_RSV3;                                  /* 07B4, 0x150227B4, DIP_A_DMA_RSV3 */
	DIP_X_REG_DMA_RSV4                              DIP_X_DMA_RSV4;                                  /* 07B8, 0x150227B8, DIP_A_DMA_RSV4 */
	DIP_X_REG_DMA_DEBUG_SEL                         DIP_X_DMA_DEBUG_SEL;                             /* 07BC, 0x150227BC, DIP_A_DMA_DEBUG_SEL */
	DIP_X_REG_DMA_BW_SELF_TEST                      DIP_X_DMA_BW_SELF_TEST;                          /* 07C0, 0x150227C0, DIP_A_DMA_BW_SELF_TEST */
	UINT32                                          rsv_07C4[3];                                     /* 07C4..07CF, 0x150227C4..150227CF */
	DIP_X_REG_SMX1O_BASE_ADDR                       DIP_X_SMX1O_BASE_ADDR;                           /* 07D0, 0x150227D0, DIP_A_SMX1O_BASE_ADDR */
	UINT32                                          rsv_07D4;                                        /* 07D4, 0x150227D4 */
	DIP_X_REG_SMX1O_OFST_ADDR                       DIP_X_SMX1O_OFST_ADDR;                           /* 07D8, 0x150227D8, DIP_A_SMX1O_OFST_ADDR */
	UINT32                                          rsv_07DC;                                        /* 07DC, 0x150227DC */
	DIP_X_REG_SMX1O_XSIZE                           DIP_X_SMX1O_XSIZE;                               /* 07E0, 0x150227E0, DIP_A_SMX1O_XSIZE */
	DIP_X_REG_SMX1O_YSIZE                           DIP_X_SMX1O_YSIZE;                               /* 07E4, 0x150227E4, DIP_A_SMX1O_YSIZE */
	DIP_X_REG_SMX1O_STRIDE                          DIP_X_SMX1O_STRIDE;                              /* 07E8, 0x150227E8, DIP_A_SMX1O_STRIDE */
	DIP_X_REG_SMX1O_CON                             DIP_X_SMX1O_CON;                                 /* 07EC, 0x150227EC, DIP_A_SMX1O_CON */
	DIP_X_REG_SMX1O_CON2                            DIP_X_SMX1O_CON2;                                /* 07F0, 0x150227F0, DIP_A_SMX1O_CON2 */
	DIP_X_REG_SMX1O_CON3                            DIP_X_SMX1O_CON3;                                /* 07F4, 0x150227F4, DIP_A_SMX1O_CON3 */
	DIP_X_REG_SMX1O_CROP                            DIP_X_SMX1O_CROP;                                /* 07F8, 0x150227F8, DIP_A_SMX1O_CROP */
	UINT32                                          rsv_07FC;                                        /* 07FC, 0x150227FC */
	DIP_X_REG_SMX2O_BASE_ADDR                       DIP_X_SMX2O_BASE_ADDR;                           /* 0800, 0x15022800, DIP_A_SMX2O_BASE_ADDR */
	UINT32                                          rsv_0804;                                        /* 0804, 0x15022804 */
	DIP_X_REG_SMX2O_OFST_ADDR                       DIP_X_SMX2O_OFST_ADDR;                           /* 0808, 0x15022808, DIP_A_SMX2O_OFST_ADDR */
	UINT32                                          rsv_080C;                                        /* 080C, 0x1502280C */
	DIP_X_REG_SMX2O_XSIZE                           DIP_X_SMX2O_XSIZE;                               /* 0810, 0x15022810, DIP_A_SMX2O_XSIZE */
	DIP_X_REG_SMX2O_YSIZE                           DIP_X_SMX2O_YSIZE;                               /* 0814, 0x15022814, DIP_A_SMX2O_YSIZE */
	DIP_X_REG_SMX2O_STRIDE                          DIP_X_SMX2O_STRIDE;                              /* 0818, 0x15022818, DIP_A_SMX2O_STRIDE */
	DIP_X_REG_SMX2O_CON                             DIP_X_SMX2O_CON;                                 /* 081C, 0x1502281C, DIP_A_SMX2O_CON */
	DIP_X_REG_SMX2O_CON2                            DIP_X_SMX2O_CON2;                                /* 0820, 0x15022820, DIP_A_SMX2O_CON2 */
	DIP_X_REG_SMX2O_CON3                            DIP_X_SMX2O_CON3;                                /* 0824, 0x15022824, DIP_A_SMX2O_CON3 */
	DIP_X_REG_SMX2O_CROP                            DIP_X_SMX2O_CROP;                                /* 0828, 0x15022828, DIP_A_SMX2O_CROP */
	UINT32                                          rsv_082C;                                        /* 082C, 0x1502282C */
	DIP_X_REG_SMX3O_BASE_ADDR                       DIP_X_SMX3O_BASE_ADDR;                           /* 0830, 0x15022830, DIP_A_SMX3O_BASE_ADDR */
	UINT32                                          rsv_0834;                                        /* 0834, 0x15022834 */
	DIP_X_REG_SMX3O_OFST_ADDR                       DIP_X_SMX3O_OFST_ADDR;                           /* 0838, 0x15022838, DIP_A_SMX3O_OFST_ADDR */
	UINT32                                          rsv_083C;                                        /* 083C, 0x1502283C */
	DIP_X_REG_SMX3O_XSIZE                           DIP_X_SMX3O_XSIZE;                               /* 0840, 0x15022840, DIP_A_SMX3O_XSIZE */
	DIP_X_REG_SMX3O_YSIZE                           DIP_X_SMX3O_YSIZE;                               /* 0844, 0x15022844, DIP_A_SMX3O_YSIZE */
	DIP_X_REG_SMX3O_STRIDE                          DIP_X_SMX3O_STRIDE;                              /* 0848, 0x15022848, DIP_A_SMX3O_STRIDE */
	DIP_X_REG_SMX3O_CON                             DIP_X_SMX3O_CON;                                 /* 084C, 0x1502284C, DIP_A_SMX3O_CON */
	DIP_X_REG_SMX3O_CON2                            DIP_X_SMX3O_CON2;                                /* 0850, 0x15022850, DIP_A_SMX3O_CON2 */
	DIP_X_REG_SMX3O_CON3                            DIP_X_SMX3O_CON3;                                /* 0854, 0x15022854, DIP_A_SMX3O_CON3 */
	DIP_X_REG_SMX3O_CROP                            DIP_X_SMX3O_CROP;                                /* 0858, 0x15022858, DIP_A_SMX3O_CROP */
	UINT32                                          rsv_085C;                                        /* 085C, 0x1502285C */
	DIP_X_REG_SMX4O_BASE_ADDR                       DIP_X_SMX4O_BASE_ADDR;                           /* 0860, 0x15022860, DIP_A_SMX4O_BASE_ADDR */
	UINT32                                          rsv_0864;                                        /* 0864, 0x15022864 */
	DIP_X_REG_SMX4O_OFST_ADDR                       DIP_X_SMX4O_OFST_ADDR;                           /* 0868, 0x15022868, DIP_A_SMX4O_OFST_ADDR */
	UINT32                                          rsv_086C;                                        /* 086C, 0x1502286C */
	DIP_X_REG_SMX4O_XSIZE                           DIP_X_SMX4O_XSIZE;                               /* 0870, 0x15022870, DIP_A_SMX4O_XSIZE */
	DIP_X_REG_SMX4O_YSIZE                           DIP_X_SMX4O_YSIZE;                               /* 0874, 0x15022874, DIP_A_SMX4O_YSIZE */
	DIP_X_REG_SMX4O_STRIDE                          DIP_X_SMX4O_STRIDE;                              /* 0878, 0x15022878, DIP_A_SMX4O_STRIDE */
	DIP_X_REG_SMX4O_CON                             DIP_X_SMX4O_CON;                                 /* 087C, 0x1502287C, DIP_A_SMX4O_CON */
	DIP_X_REG_SMX4O_CON2                            DIP_X_SMX4O_CON2;                                /* 0880, 0x15022880, DIP_A_SMX4O_CON2 */
	DIP_X_REG_SMX4O_CON3                            DIP_X_SMX4O_CON3;                                /* 0884, 0x15022884, DIP_A_SMX4O_CON3 */
	DIP_X_REG_SMX4O_CROP                            DIP_X_SMX4O_CROP;                                /* 0888, 0x15022888, DIP_A_SMX4O_CROP */
	UINT32                                          rsv_088C;                                        /* 088C, 0x1502288C */
	DIP_X_REG_SMX1I_BASE_ADDR                       DIP_X_SMX1I_BASE_ADDR;                           /* 0890, 0x15022890, DIP_A_SMX1I_BASE_ADDR */
	UINT32                                          rsv_0894;                                        /* 0894, 0x15022894 */
	DIP_X_REG_SMX1I_OFST_ADDR                       DIP_X_SMX1I_OFST_ADDR;                           /* 0898, 0x15022898, DIP_A_SMX1I_OFST_ADDR */
	UINT32                                          rsv_089C;                                        /* 089C, 0x1502289C */
	DIP_X_REG_SMX1I_XSIZE                           DIP_X_SMX1I_XSIZE;                               /* 08A0, 0x150228A0, DIP_A_SMX1I_XSIZE */
	DIP_X_REG_SMX1I_YSIZE                           DIP_X_SMX1I_YSIZE;                               /* 08A4, 0x150228A4, DIP_A_SMX1I_YSIZE */
	DIP_X_REG_SMX1I_STRIDE                          DIP_X_SMX1I_STRIDE;                              /* 08A8, 0x150228A8, DIP_A_SMX1I_STRIDE */
	DIP_X_REG_SMX1I_CON                             DIP_X_SMX1I_CON;                                 /* 08AC, 0x150228AC, DIP_A_SMX1I_CON */
	DIP_X_REG_SMX1I_CON2                            DIP_X_SMX1I_CON2;                                /* 08B0, 0x150228B0, DIP_A_SMX1I_CON2 */
	DIP_X_REG_SMX1I_CON3                            DIP_X_SMX1I_CON3;                                /* 08B4, 0x150228B4, DIP_A_SMX1I_CON3 */
	UINT32                                          rsv_08B8[2];                                     /* 08B8..08BF, 0x150228B8..150228BF */
	DIP_X_REG_SMX2I_BASE_ADDR                       DIP_X_SMX2I_BASE_ADDR;                           /* 08C0, 0x150228C0, DIP_A_SMX2I_BASE_ADDR */
	UINT32                                          rsv_08C4;                                        /* 08C4, 0x150228C4 */
	DIP_X_REG_SMX2I_OFST_ADDR                       DIP_X_SMX2I_OFST_ADDR;                           /* 08C8, 0x150228C8, DIP_A_SMX2I_OFST_ADDR */
	UINT32                                          rsv_08CC;                                        /* 08CC, 0x150228CC */
	DIP_X_REG_SMX2I_XSIZE                           DIP_X_SMX2I_XSIZE;                               /* 08D0, 0x150228D0, DIP_A_SMX2I_XSIZE */
	DIP_X_REG_SMX2I_YSIZE                           DIP_X_SMX2I_YSIZE;                               /* 08D4, 0x150228D4, DIP_A_SMX2I_YSIZE */
	DIP_X_REG_SMX2I_STRIDE                          DIP_X_SMX2I_STRIDE;                              /* 08D8, 0x150228D8, DIP_A_SMX2I_STRIDE */
	DIP_X_REG_SMX2I_CON                             DIP_X_SMX2I_CON;                                 /* 08DC, 0x150228DC, DIP_A_SMX2I_CON */
	DIP_X_REG_SMX2I_CON2                            DIP_X_SMX2I_CON2;                                /* 08E0, 0x150228E0, DIP_A_SMX2I_CON2 */
	DIP_X_REG_SMX2I_CON3                            DIP_X_SMX2I_CON3;                                /* 08E4, 0x150228E4, DIP_A_SMX2I_CON3 */
	UINT32                                          rsv_08E8[2];                                     /* 08E8..08EF, 0x150228E8..150228EF */
	DIP_X_REG_SMX3I_BASE_ADDR                       DIP_X_SMX3I_BASE_ADDR;                           /* 08F0, 0x150228F0, DIP_A_SMX3I_BASE_ADDR */
	UINT32                                          rsv_08F4;                                        /* 08F4, 0x150228F4 */
	DIP_X_REG_SMX3I_OFST_ADDR                       DIP_X_SMX3I_OFST_ADDR;                           /* 08F8, 0x150228F8, DIP_A_SMX3I_OFST_ADDR */
	UINT32                                          rsv_08FC;                                        /* 08FC, 0x150228FC */
	DIP_X_REG_SMX3I_XSIZE                           DIP_X_SMX3I_XSIZE;                               /* 0900, 0x15022900, DIP_A_SMX3I_XSIZE */
	DIP_X_REG_SMX3I_YSIZE                           DIP_X_SMX3I_YSIZE;                               /* 0904, 0x15022904, DIP_A_SMX3I_YSIZE */
	DIP_X_REG_SMX3I_STRIDE                          DIP_X_SMX3I_STRIDE;                              /* 0908, 0x15022908, DIP_A_SMX3I_STRIDE */
	DIP_X_REG_SMX3I_CON                             DIP_X_SMX3I_CON;                                 /* 090C, 0x1502290C, DIP_A_SMX3I_CON */
	DIP_X_REG_SMX3I_CON2                            DIP_X_SMX3I_CON2;                                /* 0910, 0x15022910, DIP_A_SMX3I_CON2 */
	DIP_X_REG_SMX3I_CON3                            DIP_X_SMX3I_CON3;                                /* 0914, 0x15022914, DIP_A_SMX3I_CON3 */
	UINT32                                          rsv_0918[2];                                     /* 0918..091F, 0x15022918..1502291F */
	DIP_X_REG_SMX4I_BASE_ADDR                       DIP_X_SMX4I_BASE_ADDR;                           /* 0920, 0x15022920, DIP_A_SMX4I_BASE_ADDR */
	UINT32                                          rsv_0924;                                        /* 0924, 0x15022924 */
	DIP_X_REG_SMX4I_OFST_ADDR                       DIP_X_SMX4I_OFST_ADDR;                           /* 0928, 0x15022928, DIP_A_SMX4I_OFST_ADDR */
	UINT32                                          rsv_092C;                                        /* 092C, 0x1502292C */
	DIP_X_REG_SMX4I_XSIZE                           DIP_X_SMX4I_XSIZE;                               /* 0930, 0x15022930, DIP_A_SMX4I_XSIZE */
	DIP_X_REG_SMX4I_YSIZE                           DIP_X_SMX4I_YSIZE;                               /* 0934, 0x15022934, DIP_A_SMX4I_YSIZE */
	DIP_X_REG_SMX4I_STRIDE                          DIP_X_SMX4I_STRIDE;                              /* 0938, 0x15022938, DIP_A_SMX4I_STRIDE */
	DIP_X_REG_SMX4I_CON                             DIP_X_SMX4I_CON;                                 /* 093C, 0x1502293C, DIP_A_SMX4I_CON */
	DIP_X_REG_SMX4I_CON2                            DIP_X_SMX4I_CON2;                                /* 0940, 0x15022940, DIP_A_SMX4I_CON2 */
	DIP_X_REG_SMX4I_CON3                            DIP_X_SMX4I_CON3;                                /* 0944, 0x15022944, DIP_A_SMX4I_CON3 */
	UINT32                                          rsv_0948[110];                                   /* 0948..0AFF, 0x15022948..15022AFF */
	DIP_X_REG_ADL_DMA_A_DMA_SOFT_RSTSTAT            DIP_X_ADL_DMA_A_DMA_SOFT_RSTSTAT;                /* 0B00, 0x15022B00, DIP_A_ADL_DMA_A_DMA_SOFT_RSTSTAT */
	DIP_X_REG_ADL_DMA_A_VERTICAL_FLIP_EN            DIP_X_ADL_DMA_A_VERTICAL_FLIP_EN;                /* 0B04, 0x15022B04, DIP_A_ADL_DMA_A_VERTICAL_FLIP_EN */
	DIP_X_REG_ADL_DMA_A_DMA_SOFT_RESET              DIP_X_ADL_DMA_A_DMA_SOFT_RESET;                  /* 0B08, 0x15022B08, DIP_A_ADL_DMA_A_DMA_SOFT_RESET */
	DIP_X_REG_ADL_DMA_A_LAST_ULTRA_EN               DIP_X_ADL_DMA_A_LAST_ULTRA_EN;                   /* 0B0C, 0x15022B0C, DIP_A_ADL_DMA_A_LAST_ULTRA_EN */
	DIP_X_REG_ADL_DMA_A_SPECIAL_FUN_EN              DIP_X_ADL_DMA_A_SPECIAL_FUN_EN;                  /* 0B10, 0x15022B10, DIP_A_ADL_DMA_A_SPECIAL_FUN_EN */
	DIP_X_REG_ADL_DMA_A_IPUO_RING                   DIP_X_ADL_DMA_A_IPUO_RING;                       /* 0B14, 0x15022B14, DIP_A_ADL_DMA_A_IPUO_RING */
	DIP_X_REG_ADL_DMA_A_IPUI_RING                   DIP_X_ADL_DMA_A_IPUI_RING;                       /* 0B18, 0x15022B18, DIP_A_ADL_DMA_A_IPUI_RING */
	UINT32                                          rsv_0B1C[5];                                     /* 0B1C..0B2F, 0x15022B1C..15022B2F */
	DIP_X_REG_ADL_DMA_A_IPUO_BASE_ADDR              DIP_X_ADL_DMA_A_IPUO_BASE_ADDR;                  /* 0B30, 0x15022B30, DIP_A_ADL_DMA_A_IPUO_BASE_ADDR */
	UINT32                                          rsv_0B34;                                        /* 0B34, 0x15022B34 */
	DIP_X_REG_ADL_DMA_A_IPUO_OFST_ADDR              DIP_X_ADL_DMA_A_IPUO_OFST_ADDR;                  /* 0B38, 0x15022B38, DIP_A_ADL_DMA_A_IPUO_OFST_ADDR */
	UINT32                                          rsv_0B3C;                                        /* 0B3C, 0x15022B3C */
	DIP_X_REG_ADL_DMA_A_IPUO_XSIZE                  DIP_X_ADL_DMA_A_IPUO_XSIZE;                      /* 0B40, 0x15022B40, DIP_A_ADL_DMA_A_IPUO_XSIZE */
	DIP_X_REG_ADL_DMA_A_IPUO_YSIZE                  DIP_X_ADL_DMA_A_IPUO_YSIZE;                      /* 0B44, 0x15022B44, DIP_A_ADL_DMA_A_IPUO_YSIZE */
	DIP_X_REG_ADL_DMA_A_IPUO_STRIDE                 DIP_X_ADL_DMA_A_IPUO_STRIDE;                     /* 0B48, 0x15022B48, DIP_A_ADL_DMA_A_IPUO_STRIDE */
	DIP_X_REG_ADL_DMA_A_IPUO_CON                    DIP_X_ADL_DMA_A_IPUO_CON;                        /* 0B4C, 0x15022B4C, DIP_A_ADL_DMA_A_IPUO_CON */
	DIP_X_REG_ADL_DMA_A_IPUO_CON2                   DIP_X_ADL_DMA_A_IPUO_CON2;                       /* 0B50, 0x15022B50, DIP_A_ADL_DMA_A_IPUO_CON2 */
	DIP_X_REG_ADL_DMA_A_IPUO_CON3                   DIP_X_ADL_DMA_A_IPUO_CON3;                       /* 0B54, 0x15022B54, DIP_A_ADL_DMA_A_IPUO_CON3 */
	UINT32                                          rsv_0B58[14];                                    /* 0B58..0B8F, 0x15022B58..15022B8F */
	DIP_X_REG_ADL_DMA_A_IPUI_BASE_ADDR              DIP_X_ADL_DMA_A_IPUI_BASE_ADDR;                  /* 0B90, 0x15022B90, DIP_A_ADL_DMA_A_IPUI_BASE_ADDR */
	UINT32                                          rsv_0B94;                                        /* 0B94, 0x15022B94 */
	DIP_X_REG_ADL_DMA_A_IPUI_OFST_ADDR              DIP_X_ADL_DMA_A_IPUI_OFST_ADDR;                  /* 0B98, 0x15022B98, DIP_A_ADL_DMA_A_IPUI_OFST_ADDR */
	UINT32                                          rsv_0B9C;                                        /* 0B9C, 0x15022B9C */
	DIP_X_REG_ADL_DMA_A_IPUI_XSIZE                  DIP_X_ADL_DMA_A_IPUI_XSIZE;                      /* 0BA0, 0x15022BA0, DIP_A_ADL_DMA_A_IPUI_XSIZE */
	DIP_X_REG_ADL_DMA_A_IPUI_YSIZE                  DIP_X_ADL_DMA_A_IPUI_YSIZE;                      /* 0BA4, 0x15022BA4, DIP_A_ADL_DMA_A_IPUI_YSIZE */
	DIP_X_REG_ADL_DMA_A_IPUI_STRIDE                 DIP_X_ADL_DMA_A_IPUI_STRIDE;                     /* 0BA8, 0x15022BA8, DIP_A_ADL_DMA_A_IPUI_STRIDE */
	DIP_X_REG_ADL_DMA_A_IPUI_CON                    DIP_X_ADL_DMA_A_IPUI_CON;                        /* 0BAC, 0x15022BAC, DIP_A_ADL_DMA_A_IPUI_CON */
	DIP_X_REG_ADL_DMA_A_IPUI_CON2                   DIP_X_ADL_DMA_A_IPUI_CON2;                       /* 0BB0, 0x15022BB0, DIP_A_ADL_DMA_A_IPUI_CON2 */
	DIP_X_REG_ADL_DMA_A_IPUI_CON3                   DIP_X_ADL_DMA_A_IPUI_CON3;                       /* 0BB4, 0x15022BB4, DIP_A_ADL_DMA_A_IPUI_CON3 */
	UINT32                                          rsv_0BB8[18];                                    /* 0BB8..0BFF, 0x15022BB8..15022BFF */
	DIP_X_REG_ADL_DMA_A_DMA_ERR_CTRL                DIP_X_ADL_DMA_A_DMA_ERR_CTRL;                    /* 0C00, 0x15022C00, DIP_A_ADL_DMA_A_DMA_ERR_CTRL */
	DIP_X_REG_ADL_DMA_A_IPUO_ERR_STAT               DIP_X_ADL_DMA_A_IPUO_ERR_STAT;                   /* 0C04, 0x15022C04, DIP_A_ADL_DMA_A_IPUO_ERR_STAT */
	DIP_X_REG_ADL_DMA_A_IPUI_ERR_STAT               DIP_X_ADL_DMA_A_IPUI_ERR_STAT;                   /* 0C08, 0x15022C08, DIP_A_ADL_DMA_A_IPUI_ERR_STAT */
	DIP_X_REG_ADL_DMA_A_DMA_DEBUG_ADDR              DIP_X_ADL_DMA_A_DMA_DEBUG_ADDR;                  /* 0C0C, 0x15022C0C, DIP_A_ADL_DMA_A_DMA_DEBUG_ADDR */
	DIP_X_REG_ADL_DMA_A_DMA_RSV1                    DIP_X_ADL_DMA_A_DMA_RSV1;                        /* 0C10, 0x15022C10, DIP_A_ADL_DMA_A_DMA_RSV1 */
	DIP_X_REG_ADL_DMA_A_DMA_RSV2                    DIP_X_ADL_DMA_A_DMA_RSV2;                        /* 0C14, 0x15022C14, DIP_A_ADL_DMA_A_DMA_RSV2 */
	DIP_X_REG_ADL_DMA_A_DMA_RSV3                    DIP_X_ADL_DMA_A_DMA_RSV3;                        /* 0C18, 0x15022C18, DIP_A_ADL_DMA_A_DMA_RSV3 */
	DIP_X_REG_ADL_DMA_A_DMA_RSV4                    DIP_X_ADL_DMA_A_DMA_RSV4;                        /* 0C1C, 0x15022C1C, DIP_A_ADL_DMA_A_DMA_RSV4 */
	DIP_X_REG_ADL_DMA_A_DMA_RSV5                    DIP_X_ADL_DMA_A_DMA_RSV5;                        /* 0C20, 0x15022C20, DIP_A_ADL_DMA_A_DMA_RSV5 */
	DIP_X_REG_ADL_DMA_A_DMA_RSV6                    DIP_X_ADL_DMA_A_DMA_RSV6;                        /* 0C24, 0x15022C24, DIP_A_ADL_DMA_A_DMA_RSV6 */
	DIP_X_REG_ADL_DMA_A_DMA_DEBUG_SEL               DIP_X_ADL_DMA_A_DMA_DEBUG_SEL;                   /* 0C28, 0x15022C28, DIP_A_ADL_DMA_A_DMA_DEBUG_SEL */
	DIP_X_REG_ADL_DMA_A_DMA_BW_SELF_TEST            DIP_X_ADL_DMA_A_DMA_BW_SELF_TEST;                /* 0C2C, 0x15022C2C, DIP_A_ADL_DMA_A_DMA_BW_SELF_TEST */
	UINT32                                          rsv_0C30[52];                                    /* 0C30..0CFF, 0x15022C30..15022CFF */
	DIP_X_REG_AMD_RESET                             DIP_X_AMD_RESET;                                 /* 0D00, 0x15022D00, DIP_A_AMD_RESET */
	DIP_X_REG_AMD_TOP_CTL                           DIP_X_AMD_TOP_CTL;                               /* 0D04, 0x15022D04, DIP_A_AMD_TOP_CTL */
	DIP_X_REG_AMD_INT_EN                            DIP_X_AMD_INT_EN;                                /* 0D08, 0x15022D08, DIP_A_AMD_INT_EN */
	DIP_X_REG_AMD_INT_ST                            DIP_X_AMD_INT_ST;                                /* 0D0C, 0x15022D0C, DIP_A_AMD_INT_ST */
	DIP_X_REG_AMD_C24_CTL                           DIP_X_AMD_C24_CTL;                               /* 0D10, 0x15022D10, DIP_A_AMD_C24_CTL */
	DIP_X_REG_AMD_CROP_CTL_0                        DIP_X_AMD_CROP_CTL_0;                            /* 0D14, 0x15022D14, DIP_A_AMD_CROP_CTL_0 */
	DIP_X_REG_AMD_CROP_CTL_1                        DIP_X_AMD_CROP_CTL_1;                            /* 0D18, 0x15022D18, DIP_A_AMD_CROP_CTL_1 */
	DIP_X_REG_AMD_CROP_CTL_2                        DIP_X_AMD_CROP_CTL_2;                            /* 0D1C, 0x15022D1C, DIP_A_AMD_CROP_CTL_2 */
	DIP_X_REG_AMD_DCM_DIS                           DIP_X_AMD_DCM_DIS;                               /* 0D20, 0x15022D20, DIP_A_AMD_DCM_DIS */
	DIP_X_REG_AMD_DCM_ST                            DIP_X_AMD_DCM_ST;                                /* 0D24, 0x15022D24, DIP_A_AMD_DCM_ST */
	UINT32                                          rsv_0D28[2];                                     /* 0D28..0D2F, 0x15022D28..15022D2F */
	DIP_X_REG_AMD_DEBUG_SEL                         DIP_X_AMD_DEBUG_SEL;                             /* 0D30, 0x15022D30, DIP_A_AMD_DEBUG_SEL */
	DIP_X_REG_AMD_C24_DEBUG                         DIP_X_AMD_C24_DEBUG;                             /* 0D34, 0x15022D34, DIP_A_AMD_C24_DEBUG */
	DIP_X_REG_AMD_CROP_DEBUG                        DIP_X_AMD_CROP_DEBUG;                            /* 0D38, 0x15022D38, DIP_A_AMD_CROP_DEBUG */
	DIP_X_REG_AMD_ADL_DEBUG                         DIP_X_AMD_ADL_DEBUG;                             /* 0D3C, 0x15022D3C, DIP_A_AMD_ADL_DEBUG */
	DIP_X_REG_AMD_SOF                               DIP_X_AMD_SOF;                                   /* 0D40, 0x15022D40, DIP_A_AMD_SOF */
	DIP_X_REG_AMD_REQ_ST                            DIP_X_AMD_REQ_ST;                                /* 0D44, 0x15022D44, DIP_A_AMD_REQ_ST */
	DIP_X_REG_AMD_RDY_ST                            DIP_X_AMD_RDY_ST;                                /* 0D48, 0x15022D48, DIP_A_AMD_RDY_ST */
	UINT32                                          rsv_0D4C[9];                                     /* 0D4C..0D6F, 0x15022D4C..15022D6F */
	DIP_X_REG_AMD_SPARE                             DIP_X_AMD_SPARE;                                 /* 0D70, 0x15022D70, DIP_A_AMD_SPARE */
	UINT32                                          rsv_0D74[35];                                    /* 0D74..0DFF, 0x15022D74..15022DFF */
	DIP_X_REG_ADL_RESET                             DIP_X_ADL_RESET;                                 /* 0E00, 0x15022E00, DIP_A_ADL_RESET */
	DIP_X_REG_ADL_CTL                               DIP_X_ADL_CTL;                                   /* 0E04, 0x15022E04, DIP_A_ADL_CTL */
	UINT32                                          rsv_0E08[10];                                    /* 0E08..0E2F, 0x15022E08..15022E2F */
	DIP_X_REG_ADL_CROP_IN_START                     DIP_X_ADL_CROP_IN_START;                         /* 0E30, 0x15022E30, DIP_A_ADL_CROP_IN_START */
	DIP_X_REG_ADL_CROP_IN_END                       DIP_X_ADL_CROP_IN_END;                           /* 0E34, 0x15022E34, DIP_A_ADL_CROP_IN_END */
	DIP_X_REG_ADL_CROP_OUT_START                    DIP_X_ADL_CROP_OUT_START;                        /* 0E38, 0x15022E38, DIP_A_ADL_CROP_OUT_START */
	DIP_X_REG_ADL_CROP_OUT_END                      DIP_X_ADL_CROP_OUT_END;                          /* 0E3C, 0x15022E3C, DIP_A_ADL_CROP_OUT_END */
	DIP_X_REG_ADL_DMA_ST                            DIP_X_ADL_DMA_ST;                                /* 0E40, 0x15022E40, DIP_A_ADL_DMA_ST */
	DIP_X_REG_ADL_DCM_DIS                           DIP_X_ADL_DCM_DIS;                               /* 0E44, 0x15022E44, DIP_A_ADL_DCM_DIS */
	DIP_X_REG_ADL_DCM_ST                            DIP_X_ADL_DCM_ST;                                /* 0E48, 0x15022E48, DIP_A_ADL_DCM_ST */
	DIP_X_REG_ADL_DMA_ERR_ST                        DIP_X_ADL_DMA_ERR_ST;                            /* 0E4C, 0x15022E4C, DIP_A_ADL_DMA_ERR_ST */
	DIP_X_REG_ADL_DMA_0_DEBUG                       DIP_X_ADL_DMA_0_DEBUG;                           /* 0E50, 0x15022E50, DIP_A_ADL_DMA_0_DEBUG */
	DIP_X_REG_ADL_DMA_1_DEBUG                       DIP_X_ADL_DMA_1_DEBUG;                           /* 0E54, 0x15022E54, DIP_A_ADL_DMA_1_DEBUG */
	UINT32                                          rsv_0E58[6];                                     /* 0E58..0E6F, 0x15022E58..15022E6F */
	DIP_X_REG_ADL_SPARE                             DIP_X_ADL_SPARE;                                 /* 0E70, 0x15022E70, DIP_A_ADL_SPARE */
	UINT32                                          rsv_0E74[3];                                     /* 0E74..0E7F, 0x15022E74..15022E7F */
	DIP_X_REG_ADL_INFO00                            DIP_X_ADL_INFO00;                                /* 0E80, 0x15022E80, DIP_A_ADL_INFO00 */
	DIP_X_REG_ADL_INFO01                            DIP_X_ADL_INFO01;                                /* 0E84, 0x15022E84, DIP_A_ADL_INFO01 */
	DIP_X_REG_ADL_INFO02                            DIP_X_ADL_INFO02;                                /* 0E88, 0x15022E88, DIP_A_ADL_INFO02 */
	DIP_X_REG_ADL_INFO03                            DIP_X_ADL_INFO03;                                /* 0E8C, 0x15022E8C, DIP_A_ADL_INFO03 */
	DIP_X_REG_ADL_INFO04                            DIP_X_ADL_INFO04;                                /* 0E90, 0x15022E90, DIP_A_ADL_INFO04 */
	DIP_X_REG_ADL_INFO05                            DIP_X_ADL_INFO05;                                /* 0E94, 0x15022E94, DIP_A_ADL_INFO05 */
	DIP_X_REG_ADL_INFO06                            DIP_X_ADL_INFO06;                                /* 0E98, 0x15022E98, DIP_A_ADL_INFO06 */
	DIP_X_REG_ADL_INFO07                            DIP_X_ADL_INFO07;                                /* 0E9C, 0x15022E9C, DIP_A_ADL_INFO07 */
	DIP_X_REG_ADL_INFO08                            DIP_X_ADL_INFO08;                                /* 0EA0, 0x15022EA0, DIP_A_ADL_INFO08 */
	DIP_X_REG_ADL_INFO09                            DIP_X_ADL_INFO09;                                /* 0EA4, 0x15022EA4, DIP_A_ADL_INFO09 */
	DIP_X_REG_ADL_INFO10                            DIP_X_ADL_INFO10;                                /* 0EA8, 0x15022EA8, DIP_A_ADL_INFO10 */
	DIP_X_REG_ADL_INFO11                            DIP_X_ADL_INFO11;                                /* 0EAC, 0x15022EAC, DIP_A_ADL_INFO11 */
	DIP_X_REG_ADL_INFO12                            DIP_X_ADL_INFO12;                                /* 0EB0, 0x15022EB0, DIP_A_ADL_INFO12 */
	DIP_X_REG_ADL_INFO13                            DIP_X_ADL_INFO13;                                /* 0EB4, 0x15022EB4, DIP_A_ADL_INFO13 */
	DIP_X_REG_ADL_INFO14                            DIP_X_ADL_INFO14;                                /* 0EB8, 0x15022EB8, DIP_A_ADL_INFO14 */
	DIP_X_REG_ADL_INFO15                            DIP_X_ADL_INFO15;                                /* 0EBC, 0x15022EBC, DIP_A_ADL_INFO15 */
	DIP_X_REG_ADL_INFO16                            DIP_X_ADL_INFO16;                                /* 0EC0, 0x15022EC0, DIP_A_ADL_INFO16 */
	DIP_X_REG_ADL_INFO17                            DIP_X_ADL_INFO17;                                /* 0EC4, 0x15022EC4, DIP_A_ADL_INFO17 */
	DIP_X_REG_ADL_INFO18                            DIP_X_ADL_INFO18;                                /* 0EC8, 0x15022EC8, DIP_A_ADL_INFO18 */
	DIP_X_REG_ADL_INFO19                            DIP_X_ADL_INFO19;                                /* 0ECC, 0x15022ECC, DIP_A_ADL_INFO19 */
	DIP_X_REG_ADL_INFO20                            DIP_X_ADL_INFO20;                                /* 0ED0, 0x15022ED0, DIP_A_ADL_INFO20 */
	DIP_X_REG_ADL_INFO21                            DIP_X_ADL_INFO21;                                /* 0ED4, 0x15022ED4, DIP_A_ADL_INFO21 */
	DIP_X_REG_ADL_INFO22                            DIP_X_ADL_INFO22;                                /* 0ED8, 0x15022ED8, DIP_A_ADL_INFO22 */
	DIP_X_REG_ADL_INFO23                            DIP_X_ADL_INFO23;                                /* 0EDC, 0x15022EDC, DIP_A_ADL_INFO23 */
	DIP_X_REG_ADL_INFO24                            DIP_X_ADL_INFO24;                                /* 0EE0, 0x15022EE0, DIP_A_ADL_INFO24 */
	DIP_X_REG_ADL_INFO25                            DIP_X_ADL_INFO25;                                /* 0EE4, 0x15022EE4, DIP_A_ADL_INFO25 */
	DIP_X_REG_ADL_INFO26                            DIP_X_ADL_INFO26;                                /* 0EE8, 0x15022EE8, DIP_A_ADL_INFO26 */
	DIP_X_REG_ADL_INFO27                            DIP_X_ADL_INFO27;                                /* 0EEC, 0x15022EEC, DIP_A_ADL_INFO27 */
	DIP_X_REG_ADL_INFO28                            DIP_X_ADL_INFO28;                                /* 0EF0, 0x15022EF0, DIP_A_ADL_INFO28 */
	DIP_X_REG_ADL_INFO29                            DIP_X_ADL_INFO29;                                /* 0EF4, 0x15022EF4, DIP_A_ADL_INFO29 */
	DIP_X_REG_ADL_INFO30                            DIP_X_ADL_INFO30;                                /* 0EF8, 0x15022EF8, DIP_A_ADL_INFO30 */
	DIP_X_REG_ADL_INFO31                            DIP_X_ADL_INFO31;                                /* 0EFC, 0x15022EFC, DIP_A_ADL_INFO31 */
	UINT32                                          rsv_0F00[64];                                    /* 0F00..0FFF, 0x15022F00..15022FFF */
	DIP_X_REG_UNP_OFST                              DIP_X_UNP_OFST;                                  /* 1000, 0x15023000, DIP_A_UNP_OFST */
	UINT32                                          rsv_1004[15];                                    /* 1004..103F, 0x15023004..1502303F */
	DIP_X_REG_UFDG_CON                              DIP_X_UFDG_CON;                                  /* 1040, 0x15023040, DIP_A_UFDG_CON */
	DIP_X_REG_UFDG_SIZE_CON                         DIP_X_UFDG_SIZE_CON;                             /* 1044, 0x15023044, DIP_A_UFDG_SIZE_CON */
	DIP_X_REG_UFDG_AU_CON                           DIP_X_UFDG_AU_CON;                               /* 1048, 0x15023048, DIP_A_UFDG_AU_CON */
	DIP_X_REG_UFDG_CROP_CON1                        DIP_X_UFDG_CROP_CON1;                            /* 104C, 0x1502304C, DIP_A_UFDG_CROP_CON1 */
	DIP_X_REG_UFDG_CROP_CON2                        DIP_X_UFDG_CROP_CON2;                            /* 1050, 0x15023050, DIP_A_UFDG_CROP_CON2 */
	DIP_X_REG_UFDG_AU2_CON                          DIP_X_UFDG_AU2_CON;                              /* 1054, 0x15023054, DIP_A_UFDG_AU2_CON */
	DIP_X_REG_UFDG_ADDRESS_CON                      DIP_X_UFDG_ADDRESS_CON;                          /* 1058, 0x15023058, DIP_A_UFDG_ADDRESS_CON */
	DIP_X_REG_UFDG_BS_AU_CON                        DIP_X_UFDG_BS_AU_CON;                            /* 105C, 0x1502305C, DIP_A_UFDG_BS_AU_CON */
	UINT32                                          rsv_1060[40];                                    /* 1060..10FF, 0x15023060..150230FF */
	DIP_X_REG_SMX1_CTL                              DIP_X_SMX1_CTL;                                  /* 1100, 0x15023100, DIP_A_SMX1_CTL */
	DIP_X_REG_SMX1_TRANS_CON                        DIP_X_SMX1_TRANS_CON;                            /* 1104, 0x15023104, DIP_A_SMX1_TRANS_CON */
	DIP_X_REG_SMX1_SPARE                            DIP_X_SMX1_SPARE;                                /* 1108, 0x15023108, DIP_A_SMX1_SPARE */
	DIP_X_REG_SMX1_CRPINL_CON1                      DIP_X_SMX1_CRPINL_CON1;                          /* 110C, 0x1502310C, DIP_A_SMX1_CRPINL_CON1 */
	DIP_X_REG_SMX1_CRPINL_CON2                      DIP_X_SMX1_CRPINL_CON2;                          /* 1110, 0x15023110, DIP_A_SMX1_CRPINL_CON2 */
	DIP_X_REG_SMX1_CRPINR_CON1                      DIP_X_SMX1_CRPINR_CON1;                          /* 1114, 0x15023114, DIP_A_SMX1_CRPINR_CON1 */
	DIP_X_REG_SMX1_CRPINR_CON2                      DIP_X_SMX1_CRPINR_CON2;                          /* 1118, 0x15023118, DIP_A_SMX1_CRPINR_CON2 */
	DIP_X_REG_SMX1_CRPOUT_CON1                      DIP_X_SMX1_CRPOUT_CON1;                          /* 111C, 0x1502311C, DIP_A_SMX1_CRPOUT_CON1 */
	DIP_X_REG_SMX1_CRPOUT_CON2                      DIP_X_SMX1_CRPOUT_CON2;                          /* 1120, 0x15023120, DIP_A_SMX1_CRPOUT_CON2 */
	UINT32                                          rsv_1124[7];                                     /* 1124..113F, 0x15023124..1502313F */
	DIP_X_REG_DBS2_SIGMA                            DIP_X_DBS2_SIGMA;                                /* 1140, 0x15023140, DIP_A_DBS2_SIGMA */
	DIP_X_REG_DBS2_BSTBL_0                          DIP_X_DBS2_BSTBL_0;                              /* 1144, 0x15023144, DIP_A_DBS2_BSTBL_0 */
	DIP_X_REG_DBS2_BSTBL_1                          DIP_X_DBS2_BSTBL_1;                              /* 1148, 0x15023148, DIP_A_DBS2_BSTBL_1 */
	DIP_X_REG_DBS2_BSTBL_2                          DIP_X_DBS2_BSTBL_2;                              /* 114C, 0x1502314C, DIP_A_DBS2_BSTBL_2 */
	DIP_X_REG_DBS2_BSTBL_3                          DIP_X_DBS2_BSTBL_3;                              /* 1150, 0x15023150, DIP_A_DBS2_BSTBL_3 */
	DIP_X_REG_DBS2_CTL                              DIP_X_DBS2_CTL;                                  /* 1154, 0x15023154, DIP_A_DBS2_CTL */
	DIP_X_REG_DBS2_CTL_2                            DIP_X_DBS2_CTL_2;                                /* 1158, 0x15023158, DIP_A_DBS2_CTL_2 */
	DIP_X_REG_DBS2_SIGMA_2                          DIP_X_DBS2_SIGMA_2;                              /* 115C, 0x1502315C, DIP_A_DBS2_SIGMA_2 */
	DIP_X_REG_DBS2_YGN                              DIP_X_DBS2_YGN;                                  /* 1160, 0x15023160, DIP_A_DBS2_YGN */
	DIP_X_REG_DBS2_SL_Y12                           DIP_X_DBS2_SL_Y12;                               /* 1164, 0x15023164, DIP_A_DBS2_SL_Y12 */
	DIP_X_REG_DBS2_SL_Y34                           DIP_X_DBS2_SL_Y34;                               /* 1168, 0x15023168, DIP_A_DBS2_SL_Y34 */
	DIP_X_REG_DBS2_SL_G12                           DIP_X_DBS2_SL_G12;                               /* 116C, 0x1502316C, DIP_A_DBS2_SL_G12 */
	DIP_X_REG_DBS2_SL_G34                           DIP_X_DBS2_SL_G34;                               /* 1170, 0x15023170, DIP_A_DBS2_SL_G34 */
	UINT32                                          rsv_1174[3];                                     /* 1174..117F, 0x15023174..1502317F */
	DIP_X_REG_SL2G_CEN                              DIP_X_SL2G_CEN;                                  /* 1180, 0x15023180, DIP_A_SL2G_CEN */
	DIP_X_REG_SL2G_RR_CON0                          DIP_X_SL2G_RR_CON0;                              /* 1184, 0x15023184, DIP_A_SL2G_RR_CON0 */
	DIP_X_REG_SL2G_RR_CON1                          DIP_X_SL2G_RR_CON1;                              /* 1188, 0x15023188, DIP_A_SL2G_RR_CON1 */
	DIP_X_REG_SL2G_GAIN                             DIP_X_SL2G_GAIN;                                 /* 118C, 0x1502318C, DIP_A_SL2G_GAIN */
	DIP_X_REG_SL2G_RZ                               DIP_X_SL2G_RZ;                                   /* 1190, 0x15023190, DIP_A_SL2G_RZ */
	DIP_X_REG_SL2G_XOFF                             DIP_X_SL2G_XOFF;                                 /* 1194, 0x15023194, DIP_A_SL2G_XOFF */
	DIP_X_REG_SL2G_YOFF                             DIP_X_SL2G_YOFF;                                 /* 1198, 0x15023198, DIP_A_SL2G_YOFF */
	DIP_X_REG_SL2G_SLP_CON0                         DIP_X_SL2G_SLP_CON0;                             /* 119C, 0x1502319C, DIP_A_SL2G_SLP_CON0 */
	DIP_X_REG_SL2G_SLP_CON1                         DIP_X_SL2G_SLP_CON1;                             /* 11A0, 0x150231A0, DIP_A_SL2G_SLP_CON1 */
	DIP_X_REG_SL2G_SLP_CON2                         DIP_X_SL2G_SLP_CON2;                             /* 11A4, 0x150231A4, DIP_A_SL2G_SLP_CON2 */
	DIP_X_REG_SL2G_SLP_CON3                         DIP_X_SL2G_SLP_CON3;                             /* 11A8, 0x150231A8, DIP_A_SL2G_SLP_CON3 */
	DIP_X_REG_SL2G_SIZE                             DIP_X_SL2G_SIZE;                                 /* 11AC, 0x150231AC, DIP_A_SL2G_SIZE */
	UINT32                                          rsv_11B0[4];                                     /* 11B0..11BF, 0x150231B0..150231BF */
	DIP_X_REG_OBC2_OFFST0                           DIP_X_OBC2_OFFST0;                               /* 11C0, 0x150231C0, DIP_A_OBC2_OFFST0 */
	DIP_X_REG_OBC2_OFFST1                           DIP_X_OBC2_OFFST1;                               /* 11C4, 0x150231C4, DIP_A_OBC2_OFFST1 */
	DIP_X_REG_OBC2_OFFST2                           DIP_X_OBC2_OFFST2;                               /* 11C8, 0x150231C8, DIP_A_OBC2_OFFST2 */
	DIP_X_REG_OBC2_OFFST3                           DIP_X_OBC2_OFFST3;                               /* 11CC, 0x150231CC, DIP_A_OBC2_OFFST3 */
	DIP_X_REG_OBC2_GAIN0                            DIP_X_OBC2_GAIN0;                                /* 11D0, 0x150231D0, DIP_A_OBC2_GAIN0 */
	DIP_X_REG_OBC2_GAIN1                            DIP_X_OBC2_GAIN1;                                /* 11D4, 0x150231D4, DIP_A_OBC2_GAIN1 */
	DIP_X_REG_OBC2_GAIN2                            DIP_X_OBC2_GAIN2;                                /* 11D8, 0x150231D8, DIP_A_OBC2_GAIN2 */
	DIP_X_REG_OBC2_GAIN3                            DIP_X_OBC2_GAIN3;                                /* 11DC, 0x150231DC, DIP_A_OBC2_GAIN3 */
	UINT32                                          rsv_11E0[8];                                     /* 11E0..11FF, 0x150231E0..150231FF */
	DIP_X_REG_RMG2_HDR_CFG                          DIP_X_RMG2_HDR_CFG;                              /* 1200, 0x15023200, DIP_A_RMG2_HDR_CFG */
	DIP_X_REG_RMG2_HDR_GAIN                         DIP_X_RMG2_HDR_GAIN;                             /* 1204, 0x15023204, DIP_A_RMG2_HDR_GAIN */
	DIP_X_REG_RMG2_HDR_CFG2                         DIP_X_RMG2_HDR_CFG2;                             /* 1208, 0x15023208, DIP_A_RMG2_HDR_CFG2 */
	UINT32                                          rsv_120C[13];                                    /* 120C..123F, 0x1502320C..1502323F */
	DIP_X_REG_BNR2_BPC_CON                          DIP_X_BNR2_BPC_CON;                              /* 1240, 0x15023240, DIP_A_BNR2_BPC_CON */
	DIP_X_REG_BNR2_BPC_TH1                          DIP_X_BNR2_BPC_TH1;                              /* 1244, 0x15023244, DIP_A_BNR2_BPC_TH1 */
	DIP_X_REG_BNR2_BPC_TH2                          DIP_X_BNR2_BPC_TH2;                              /* 1248, 0x15023248, DIP_A_BNR2_BPC_TH2 */
	DIP_X_REG_BNR2_BPC_TH3                          DIP_X_BNR2_BPC_TH3;                              /* 124C, 0x1502324C, DIP_A_BNR2_BPC_TH3 */
	DIP_X_REG_BNR2_BPC_TH4                          DIP_X_BNR2_BPC_TH4;                              /* 1250, 0x15023250, DIP_A_BNR2_BPC_TH4 */
	DIP_X_REG_BNR2_BPC_DTC                          DIP_X_BNR2_BPC_DTC;                              /* 1254, 0x15023254, DIP_A_BNR2_BPC_DTC */
	DIP_X_REG_BNR2_BPC_COR                          DIP_X_BNR2_BPC_COR;                              /* 1258, 0x15023258, DIP_A_BNR2_BPC_COR */
	DIP_X_REG_BNR2_BPC_TBLI1                        DIP_X_BNR2_BPC_TBLI1;                            /* 125C, 0x1502325C, DIP_A_BNR2_BPC_TBLI1 */
	DIP_X_REG_BNR2_BPC_TBLI2                        DIP_X_BNR2_BPC_TBLI2;                            /* 1260, 0x15023260, DIP_A_BNR2_BPC_TBLI2 */
	DIP_X_REG_BNR2_BPC_TH1_C                        DIP_X_BNR2_BPC_TH1_C;                            /* 1264, 0x15023264, DIP_A_BNR2_BPC_TH1_C */
	DIP_X_REG_BNR2_BPC_TH2_C                        DIP_X_BNR2_BPC_TH2_C;                            /* 1268, 0x15023268, DIP_A_BNR2_BPC_TH2_C */
	DIP_X_REG_BNR2_BPC_TH3_C                        DIP_X_BNR2_BPC_TH3_C;                            /* 126C, 0x1502326C, DIP_A_BNR2_BPC_TH3_C */
	DIP_X_REG_BNR2_NR1_CON                          DIP_X_BNR2_NR1_CON;                              /* 1270, 0x15023270, DIP_A_BNR2_NR1_CON */
	DIP_X_REG_BNR2_NR1_CT_CON                       DIP_X_BNR2_NR1_CT_CON;                           /* 1274, 0x15023274, DIP_A_BNR2_NR1_CT_CON */
	DIP_X_REG_BNR2_NR1_CT_CON2                      DIP_X_BNR2_NR1_CT_CON2;                          /* 1278, 0x15023278, DIP_A_BNR2_NR1_CT_CON2 */
	DIP_X_REG_BNR2_NR1_CT_CON3                      DIP_X_BNR2_NR1_CT_CON3;                          /* 127C, 0x1502327C, DIP_A_BNR2_NR1_CT_CON3 */
	DIP_X_REG_BNR2_PDC_CON                          DIP_X_BNR2_PDC_CON;                              /* 1280, 0x15023280, DIP_A_BNR2_PDC_CON */
	DIP_X_REG_BNR2_PDC_GAIN_L0                      DIP_X_BNR2_PDC_GAIN_L0;                          /* 1284, 0x15023284, DIP_A_BNR2_PDC_GAIN_L0 */
	DIP_X_REG_BNR2_PDC_GAIN_L1                      DIP_X_BNR2_PDC_GAIN_L1;                          /* 1288, 0x15023288, DIP_A_BNR2_PDC_GAIN_L1 */
	DIP_X_REG_BNR2_PDC_GAIN_L2                      DIP_X_BNR2_PDC_GAIN_L2;                          /* 128C, 0x1502328C, DIP_A_BNR2_PDC_GAIN_L2 */
	DIP_X_REG_BNR2_PDC_GAIN_L3                      DIP_X_BNR2_PDC_GAIN_L3;                          /* 1290, 0x15023290, DIP_A_BNR2_PDC_GAIN_L3 */
	DIP_X_REG_BNR2_PDC_GAIN_L4                      DIP_X_BNR2_PDC_GAIN_L4;                          /* 1294, 0x15023294, DIP_A_BNR2_PDC_GAIN_L4 */
	DIP_X_REG_BNR2_PDC_GAIN_R0                      DIP_X_BNR2_PDC_GAIN_R0;                          /* 1298, 0x15023298, DIP_A_BNR2_PDC_GAIN_R0 */
	DIP_X_REG_BNR2_PDC_GAIN_R1                      DIP_X_BNR2_PDC_GAIN_R1;                          /* 129C, 0x1502329C, DIP_A_BNR2_PDC_GAIN_R1 */
	DIP_X_REG_BNR2_PDC_GAIN_R2                      DIP_X_BNR2_PDC_GAIN_R2;                          /* 12A0, 0x150232A0, DIP_A_BNR2_PDC_GAIN_R2 */
	DIP_X_REG_BNR2_PDC_GAIN_R3                      DIP_X_BNR2_PDC_GAIN_R3;                          /* 12A4, 0x150232A4, DIP_A_BNR2_PDC_GAIN_R3 */
	DIP_X_REG_BNR2_PDC_GAIN_R4                      DIP_X_BNR2_PDC_GAIN_R4;                          /* 12A8, 0x150232A8, DIP_A_BNR2_PDC_GAIN_R4 */
	DIP_X_REG_BNR2_PDC_TH_GB                        DIP_X_BNR2_PDC_TH_GB;                            /* 12AC, 0x150232AC, DIP_A_BNR2_PDC_TH_GB */
	DIP_X_REG_BNR2_PDC_TH_IA                        DIP_X_BNR2_PDC_TH_IA;                            /* 12B0, 0x150232B0, DIP_A_BNR2_PDC_TH_IA */
	DIP_X_REG_BNR2_PDC_TH_HD                        DIP_X_BNR2_PDC_TH_HD;                            /* 12B4, 0x150232B4, DIP_A_BNR2_PDC_TH_HD */
	DIP_X_REG_BNR2_PDC_SL                           DIP_X_BNR2_PDC_SL;                               /* 12B8, 0x150232B8, DIP_A_BNR2_PDC_SL */
	DIP_X_REG_BNR2_PDC_POS                          DIP_X_BNR2_PDC_POS;                              /* 12BC, 0x150232BC, DIP_A_BNR2_PDC_POS */
	DIP_X_REG_RMM2_OSC                              DIP_X_RMM2_OSC;                                  /* 12C0, 0x150232C0, DIP_A_RMM2_OSC */
	DIP_X_REG_RMM2_MC                               DIP_X_RMM2_MC;                                   /* 12C4, 0x150232C4, DIP_A_RMM2_MC */
	DIP_X_REG_RMM2_REVG_1                           DIP_X_RMM2_REVG_1;                               /* 12C8, 0x150232C8, DIP_A_RMM2_REVG_1 */
	DIP_X_REG_RMM2_REVG_2                           DIP_X_RMM2_REVG_2;                               /* 12CC, 0x150232CC, DIP_A_RMM2_REVG_2 */
	DIP_X_REG_RMM2_LEOS                             DIP_X_RMM2_LEOS;                                 /* 12D0, 0x150232D0, DIP_A_RMM2_LEOS */
	DIP_X_REG_RMM2_MC2                              DIP_X_RMM2_MC2;                                  /* 12D4, 0x150232D4, DIP_A_RMM2_MC2 */
	DIP_X_REG_RMM2_DIFF_LB                          DIP_X_RMM2_DIFF_LB;                              /* 12D8, 0x150232D8, DIP_A_RMM2_DIFF_LB */
	DIP_X_REG_RMM2_MA                               DIP_X_RMM2_MA;                                   /* 12DC, 0x150232DC, DIP_A_RMM2_MA */
	DIP_X_REG_RMM2_TUNE                             DIP_X_RMM2_TUNE;                                 /* 12E0, 0x150232E0, DIP_A_RMM2_TUNE */
	UINT32                                          rsv_12E4[23];                                    /* 12E4..133F, 0x150232E4..1502333F */
	DIP_X_REG_LSC2_CTL1                             DIP_X_LSC2_CTL1;                                 /* 1340, 0x15023340, DIP_A_LSC2_CTL1 */
	DIP_X_REG_LSC2_CTL2                             DIP_X_LSC2_CTL2;                                 /* 1344, 0x15023344, DIP_A_LSC2_CTL2 */
	DIP_X_REG_LSC2_CTL3                             DIP_X_LSC2_CTL3;                                 /* 1348, 0x15023348, DIP_A_LSC2_CTL3 */
	DIP_X_REG_LSC2_LBLOCK                           DIP_X_LSC2_LBLOCK;                               /* 134C, 0x1502334C, DIP_A_LSC2_LBLOCK */
	DIP_X_REG_LSC2_RATIO_0                          DIP_X_LSC2_RATIO_0;                              /* 1350, 0x15023350, DIP_A_LSC2_RATIO_0 */
	DIP_X_REG_LSC2_TPIPE_OFST                       DIP_X_LSC2_TPIPE_OFST;                           /* 1354, 0x15023354, DIP_A_LSC2_TPIPE_OFST */
	DIP_X_REG_LSC2_TPIPE_SIZE                       DIP_X_LSC2_TPIPE_SIZE;                           /* 1358, 0x15023358, DIP_A_LSC2_TPIPE_SIZE */
	DIP_X_REG_LSC2_GAIN_TH                          DIP_X_LSC2_GAIN_TH;                              /* 135C, 0x1502335C, DIP_A_LSC2_GAIN_TH */
	DIP_X_REG_LSC2_RATIO_1                          DIP_X_LSC2_RATIO_1;                              /* 1360, 0x15023360, DIP_A_LSC2_RATIO_1 */
	DIP_X_REG_LSC2_UPB_B_GB                         DIP_X_LSC2_UPB_B_GB;                             /* 1364, 0x15023364, DIP_A_LSC2_UPB_B_GB */
	DIP_X_REG_LSC2_UPB_GR_R                         DIP_X_LSC2_UPB_GR_R;                             /* 1368, 0x15023368, DIP_A_LSC2_UPB_GR_R */
	UINT32                                          rsv_136C[37];                                    /* 136C..13FF, 0x1502336C..150233FF */
	DIP_X_REG_SL2K_CEN                              DIP_X_SL2K_CEN;                                  /* 1400, 0x15023400, DIP_A_SL2K_CEN */
	DIP_X_REG_SL2K_RR_CON0                          DIP_X_SL2K_RR_CON0;                              /* 1404, 0x15023404, DIP_A_SL2K_RR_CON0 */
	DIP_X_REG_SL2K_RR_CON1                          DIP_X_SL2K_RR_CON1;                              /* 1408, 0x15023408, DIP_A_SL2K_RR_CON1 */
	DIP_X_REG_SL2K_GAIN                             DIP_X_SL2K_GAIN;                                 /* 140C, 0x1502340C, DIP_A_SL2K_GAIN */
	DIP_X_REG_SL2K_RZ                               DIP_X_SL2K_RZ;                                   /* 1410, 0x15023410, DIP_A_SL2K_RZ */
	DIP_X_REG_SL2K_XOFF                             DIP_X_SL2K_XOFF;                                 /* 1414, 0x15023414, DIP_A_SL2K_XOFF */
	DIP_X_REG_SL2K_YOFF                             DIP_X_SL2K_YOFF;                                 /* 1418, 0x15023418, DIP_A_SL2K_YOFF */
	DIP_X_REG_SL2K_SLP_CON0                         DIP_X_SL2K_SLP_CON0;                             /* 141C, 0x1502341C, DIP_A_SL2K_SLP_CON0 */
	DIP_X_REG_SL2K_SLP_CON1                         DIP_X_SL2K_SLP_CON1;                             /* 1420, 0x15023420, DIP_A_SL2K_SLP_CON1 */
	DIP_X_REG_SL2K_SLP_CON2                         DIP_X_SL2K_SLP_CON2;                             /* 1424, 0x15023424, DIP_A_SL2K_SLP_CON2 */
	DIP_X_REG_SL2K_SLP_CON3                         DIP_X_SL2K_SLP_CON3;                             /* 1428, 0x15023428, DIP_A_SL2K_SLP_CON3 */
	DIP_X_REG_SL2K_SIZE                             DIP_X_SL2K_SIZE;                                 /* 142C, 0x1502342C, DIP_A_SL2K_SIZE */
	UINT32                                          rsv_1430[4];                                     /* 1430..143F, 0x15023430..1502343F */
	DIP_X_REG_PGN_SATU_1                            DIP_X_PGN_SATU_1;                                /* 1440, 0x15023440, DIP_A_PGN_SATU_1 */
	DIP_X_REG_PGN_SATU_2                            DIP_X_PGN_SATU_2;                                /* 1444, 0x15023444, DIP_A_PGN_SATU_2 */
	DIP_X_REG_PGN_GAIN_1                            DIP_X_PGN_GAIN_1;                                /* 1448, 0x15023448, DIP_A_PGN_GAIN_1 */
	DIP_X_REG_PGN_GAIN_2                            DIP_X_PGN_GAIN_2;                                /* 144C, 0x1502344C, DIP_A_PGN_GAIN_2 */
	DIP_X_REG_PGN_OFST_1                            DIP_X_PGN_OFST_1;                                /* 1450, 0x15023450, DIP_A_PGN_OFST_1 */
	DIP_X_REG_PGN_OFST_2                            DIP_X_PGN_OFST_2;                                /* 1454, 0x15023454, DIP_A_PGN_OFST_2 */
	UINT32                                          rsv_1458[10];                                    /* 1458..147F, 0x15023458..1502347F */
	DIP_X_REG_RCP2_CROP_CON1                        DIP_X_RCP2_CROP_CON1;                            /* 1480, 0x15023480, DIP_A_RCP2_CROP_CON1 */
	DIP_X_REG_RCP2_CROP_CON2                        DIP_X_RCP2_CROP_CON2;                            /* 1484, 0x15023484, DIP_A_RCP2_CROP_CON2 */
	UINT32                                          rsv_1488[14];                                    /* 1488..14BF, 0x15023488..150234BF */
	DIP_X_REG_RNR_CON1                              DIP_X_RNR_CON1;                                  /* 14C0, 0x150234C0, DIP_A_RNR_CON1 */
	DIP_X_REG_RNR_VC0                               DIP_X_RNR_VC0;                                   /* 14C4, 0x150234C4, DIP_A_RNR_VC0 */
	DIP_X_REG_RNR_VC1                               DIP_X_RNR_VC1;                                   /* 14C8, 0x150234C8, DIP_A_RNR_VC1 */
	DIP_X_REG_RNR_GO_B                              DIP_X_RNR_GO_B;                                  /* 14CC, 0x150234CC, DIP_A_RNR_GO_B */
	DIP_X_REG_RNR_GO_G                              DIP_X_RNR_GO_G;                                  /* 14D0, 0x150234D0, DIP_A_RNR_GO_G */
	DIP_X_REG_RNR_GO_R                              DIP_X_RNR_GO_R;                                  /* 14D4, 0x150234D4, DIP_A_RNR_GO_R */
	DIP_X_REG_RNR_GAIN_ISO                          DIP_X_RNR_GAIN_ISO;                              /* 14D8, 0x150234D8, DIP_A_RNR_GAIN_ISO */
	DIP_X_REG_RNR_L                                 DIP_X_RNR_L;                                     /* 14DC, 0x150234DC, DIP_A_RNR_L */
	DIP_X_REG_RNR_C1                                DIP_X_RNR_C1;                                    /* 14E0, 0x150234E0, DIP_A_RNR_C1 */
	DIP_X_REG_RNR_C2                                DIP_X_RNR_C2;                                    /* 14E4, 0x150234E4, DIP_A_RNR_C2 */
	DIP_X_REG_RNR_RNG                               DIP_X_RNR_RNG;                                   /* 14E8, 0x150234E8, DIP_A_RNR_RNG */
	DIP_X_REG_RNR_CON2                              DIP_X_RNR_CON2;                                  /* 14EC, 0x150234EC, DIP_A_RNR_CON2 */
	DIP_X_REG_RNR_CON3                              DIP_X_RNR_CON3;                                  /* 14F0, 0x150234F0, DIP_A_RNR_CON3 */
	DIP_X_REG_RNR_SL                                DIP_X_RNR_SL;                                    /* 14F4, 0x150234F4, DIP_A_RNR_SL */
	DIP_X_REG_RNR_SSL_STH                           DIP_X_RNR_SSL_STH;                               /* 14F8, 0x150234F8, DIP_A_RNR_SSL_STH */
	DIP_X_REG_RNR_TILE_EDGE                         DIP_X_RNR_TILE_EDGE;                             /* 14FC, 0x150234FC, DIP_A_RNR_TILE_EDGE */
	DIP_X_REG_SL2H_CEN                              DIP_X_SL2H_CEN;                                  /* 1500, 0x15023500, DIP_A_SL2H_CEN */
	DIP_X_REG_SL2H_RR_CON0                          DIP_X_SL2H_RR_CON0;                              /* 1504, 0x15023504, DIP_A_SL2H_RR_CON0 */
	DIP_X_REG_SL2H_RR_CON1                          DIP_X_SL2H_RR_CON1;                              /* 1508, 0x15023508, DIP_A_SL2H_RR_CON1 */
	DIP_X_REG_SL2H_GAIN                             DIP_X_SL2H_GAIN;                                 /* 150C, 0x1502350C, DIP_A_SL2H_GAIN */
	DIP_X_REG_SL2H_RZ                               DIP_X_SL2H_RZ;                                   /* 1510, 0x15023510, DIP_A_SL2H_RZ */
	DIP_X_REG_SL2H_XOFF                             DIP_X_SL2H_XOFF;                                 /* 1514, 0x15023514, DIP_A_SL2H_XOFF */
	DIP_X_REG_SL2H_YOFF                             DIP_X_SL2H_YOFF;                                 /* 1518, 0x15023518, DIP_A_SL2H_YOFF */
	DIP_X_REG_SL2H_SLP_CON0                         DIP_X_SL2H_SLP_CON0;                             /* 151C, 0x1502351C, DIP_A_SL2H_SLP_CON0 */
	DIP_X_REG_SL2H_SLP_CON1                         DIP_X_SL2H_SLP_CON1;                             /* 1520, 0x15023520, DIP_A_SL2H_SLP_CON1 */
	DIP_X_REG_SL2H_SLP_CON2                         DIP_X_SL2H_SLP_CON2;                             /* 1524, 0x15023524, DIP_A_SL2H_SLP_CON2 */
	DIP_X_REG_SL2H_SLP_CON3                         DIP_X_SL2H_SLP_CON3;                             /* 1528, 0x15023528, DIP_A_SL2H_SLP_CON3 */
	DIP_X_REG_SL2H_SIZE                             DIP_X_SL2H_SIZE;                                 /* 152C, 0x1502352C, DIP_A_SL2H_SIZE */
	UINT32                                          rsv_1530[4];                                     /* 1530..153F, 0x15023530..1502353F */
	DIP_X_REG_UDM_INTP_CRS                          DIP_X_UDM_INTP_CRS;                              /* 1540, 0x15023540, DIP_A_UDM_INTP_CRS */
	DIP_X_REG_UDM_INTP_NAT                          DIP_X_UDM_INTP_NAT;                              /* 1544, 0x15023544, DIP_A_UDM_INTP_NAT */
	DIP_X_REG_UDM_INTP_AUG                          DIP_X_UDM_INTP_AUG;                              /* 1548, 0x15023548, DIP_A_UDM_INTP_AUG */
	DIP_X_REG_UDM_LUMA_LUT1                         DIP_X_UDM_LUMA_LUT1;                             /* 154C, 0x1502354C, DIP_A_UDM_LUMA_LUT1 */
	DIP_X_REG_UDM_LUMA_LUT2                         DIP_X_UDM_LUMA_LUT2;                             /* 1550, 0x15023550, DIP_A_UDM_LUMA_LUT2 */
	DIP_X_REG_UDM_SL_CTL                            DIP_X_UDM_SL_CTL;                                /* 1554, 0x15023554, DIP_A_UDM_SL_CTL */
	DIP_X_REG_UDM_HFTD_CTL                          DIP_X_UDM_HFTD_CTL;                              /* 1558, 0x15023558, DIP_A_UDM_HFTD_CTL */
	DIP_X_REG_UDM_NR_STR                            DIP_X_UDM_NR_STR;                                /* 155C, 0x1502355C, DIP_A_UDM_NR_STR */
	DIP_X_REG_UDM_NR_ACT                            DIP_X_UDM_NR_ACT;                                /* 1560, 0x15023560, DIP_A_UDM_NR_ACT */
	DIP_X_REG_UDM_HF_STR                            DIP_X_UDM_HF_STR;                                /* 1564, 0x15023564, DIP_A_UDM_HF_STR */
	DIP_X_REG_UDM_HF_ACT1                           DIP_X_UDM_HF_ACT1;                               /* 1568, 0x15023568, DIP_A_UDM_HF_ACT1 */
	DIP_X_REG_UDM_HF_ACT2                           DIP_X_UDM_HF_ACT2;                               /* 156C, 0x1502356C, DIP_A_UDM_HF_ACT2 */
	DIP_X_REG_UDM_CLIP                              DIP_X_UDM_CLIP;                                  /* 1570, 0x15023570, DIP_A_UDM_CLIP */
	DIP_X_REG_UDM_DSB                               DIP_X_UDM_DSB;                                   /* 1574, 0x15023574, DIP_A_UDM_DSB */
	DIP_X_REG_UDM_TILE_EDGE                         DIP_X_UDM_TILE_EDGE;                             /* 1578, 0x15023578, DIP_A_UDM_TILE_EDGE */
	DIP_X_REG_UDM_P1_ACT                            DIP_X_UDM_P1_ACT;                                /* 157C, 0x1502357C, DIP_A_UDM_P1_ACT */
	DIP_X_REG_UDM_LR_RAT                            DIP_X_UDM_LR_RAT;                                /* 1580, 0x15023580, DIP_A_UDM_LR_RAT */
	DIP_X_REG_UDM_HFTD_CTL2                         DIP_X_UDM_HFTD_CTL2;                             /* 1584, 0x15023584, DIP_A_UDM_HFTD_CTL2 */
	DIP_X_REG_UDM_EST_CTL                           DIP_X_UDM_EST_CTL;                               /* 1588, 0x15023588, DIP_A_UDM_EST_CTL */
	DIP_X_REG_UDM_SPARE_2                           DIP_X_UDM_SPARE_2;                               /* 158C, 0x1502358C, DIP_A_UDM_SPARE_2 */
	DIP_X_REG_UDM_SPARE_3                           DIP_X_UDM_SPARE_3;                               /* 1590, 0x15023590, DIP_A_UDM_SPARE_3 */
	DIP_X_REG_UDM_INT_CTL                           DIP_X_UDM_INT_CTL;                               /* 1594, 0x15023594, DIP_A_UDM_INT_CTL */
	DIP_X_REG_UDM_EE                                DIP_X_UDM_EE;                                    /* 1598, 0x15023598, DIP_A_UDM_EE */
	UINT32                                          rsv_159C[9];                                     /* 159C..15BF, 0x1502359C..150235BF */
	DIP_X_REG_SL2_CEN                               DIP_X_SL2_CEN;                                   /* 15C0, 0x150235C0, DIP_A_SL2_CEN */
	DIP_X_REG_SL2_RR_CON0                           DIP_X_SL2_RR_CON0;                               /* 15C4, 0x150235C4, DIP_A_SL2_RR_CON0 */
	DIP_X_REG_SL2_RR_CON1                           DIP_X_SL2_RR_CON1;                               /* 15C8, 0x150235C8, DIP_A_SL2_RR_CON1 */
	DIP_X_REG_SL2_GAIN                              DIP_X_SL2_GAIN;                                  /* 15CC, 0x150235CC, DIP_A_SL2_GAIN */
	DIP_X_REG_SL2_RZ                                DIP_X_SL2_RZ;                                    /* 15D0, 0x150235D0, DIP_A_SL2_RZ */
	DIP_X_REG_SL2_XOFF                              DIP_X_SL2_XOFF;                                  /* 15D4, 0x150235D4, DIP_A_SL2_XOFF */
	DIP_X_REG_SL2_YOFF                              DIP_X_SL2_YOFF;                                  /* 15D8, 0x150235D8, DIP_A_SL2_YOFF */
	DIP_X_REG_SL2_SLP_CON0                          DIP_X_SL2_SLP_CON0;                              /* 15DC, 0x150235DC, DIP_A_SL2_SLP_CON0 */
	DIP_X_REG_SL2_SLP_CON1                          DIP_X_SL2_SLP_CON1;                              /* 15E0, 0x150235E0, DIP_A_SL2_SLP_CON1 */
	DIP_X_REG_SL2_SLP_CON2                          DIP_X_SL2_SLP_CON2;                              /* 15E4, 0x150235E4, DIP_A_SL2_SLP_CON2 */
	DIP_X_REG_SL2_SLP_CON3                          DIP_X_SL2_SLP_CON3;                              /* 15E8, 0x150235E8, DIP_A_SL2_SLP_CON3 */
	DIP_X_REG_SL2_SIZE                              DIP_X_SL2_SIZE;                                  /* 15EC, 0x150235EC, DIP_A_SL2_SIZE */
	UINT32                                          rsv_15F0[4];                                     /* 15F0..15FF, 0x150235F0..150235FF */
	DIP_X_REG_ADBS2_CTL                             DIP_X_ADBS2_CTL;                                 /* 1600, 0x15023600, DIP_A_ADBS2_CTL */
	DIP_X_REG_ADBS2_GRAY_BLD_0                      DIP_X_ADBS2_GRAY_BLD_0;                          /* 1604, 0x15023604, DIP_A_ADBS2_GRAY_BLD_0 */
	DIP_X_REG_ADBS2_GRAY_BLD_1                      DIP_X_ADBS2_GRAY_BLD_1;                          /* 1608, 0x15023608, DIP_A_ADBS2_GRAY_BLD_1 */
	DIP_X_REG_ADBS2_BIAS_LUT_R0                     DIP_X_ADBS2_BIAS_LUT_R0;                         /* 160C, 0x1502360C, DIP_A_ADBS2_BIAS_LUT_R0 */
	DIP_X_REG_ADBS2_BIAS_LUT_R1                     DIP_X_ADBS2_BIAS_LUT_R1;                         /* 1610, 0x15023610, DIP_A_ADBS2_BIAS_LUT_R1 */
	DIP_X_REG_ADBS2_BIAS_LUT_R2                     DIP_X_ADBS2_BIAS_LUT_R2;                         /* 1614, 0x15023614, DIP_A_ADBS2_BIAS_LUT_R2 */
	DIP_X_REG_ADBS2_BIAS_LUT_R3                     DIP_X_ADBS2_BIAS_LUT_R3;                         /* 1618, 0x15023618, DIP_A_ADBS2_BIAS_LUT_R3 */
	DIP_X_REG_ADBS2_BIAS_LUT_G0                     DIP_X_ADBS2_BIAS_LUT_G0;                         /* 161C, 0x1502361C, DIP_A_ADBS2_BIAS_LUT_G0 */
	DIP_X_REG_ADBS2_BIAS_LUT_G1                     DIP_X_ADBS2_BIAS_LUT_G1;                         /* 1620, 0x15023620, DIP_A_ADBS2_BIAS_LUT_G1 */
	DIP_X_REG_ADBS2_BIAS_LUT_G2                     DIP_X_ADBS2_BIAS_LUT_G2;                         /* 1624, 0x15023624, DIP_A_ADBS2_BIAS_LUT_G2 */
	DIP_X_REG_ADBS2_BIAS_LUT_G3                     DIP_X_ADBS2_BIAS_LUT_G3;                         /* 1628, 0x15023628, DIP_A_ADBS2_BIAS_LUT_G3 */
	DIP_X_REG_ADBS2_BIAS_LUT_B0                     DIP_X_ADBS2_BIAS_LUT_B0;                         /* 162C, 0x1502362C, DIP_A_ADBS2_BIAS_LUT_B0 */
	DIP_X_REG_ADBS2_BIAS_LUT_B1                     DIP_X_ADBS2_BIAS_LUT_B1;                         /* 1630, 0x15023630, DIP_A_ADBS2_BIAS_LUT_B1 */
	DIP_X_REG_ADBS2_BIAS_LUT_B2                     DIP_X_ADBS2_BIAS_LUT_B2;                         /* 1634, 0x15023634, DIP_A_ADBS2_BIAS_LUT_B2 */
	DIP_X_REG_ADBS2_BIAS_LUT_B3                     DIP_X_ADBS2_BIAS_LUT_B3;                         /* 1638, 0x15023638, DIP_A_ADBS2_BIAS_LUT_B3 */
	DIP_X_REG_ADBS2_GAIN_0                          DIP_X_ADBS2_GAIN_0;                              /* 163C, 0x1502363C, DIP_A_ADBS2_GAIN_0 */
	DIP_X_REG_ADBS2_GAIN_1                          DIP_X_ADBS2_GAIN_1;                              /* 1640, 0x15023640, DIP_A_ADBS2_GAIN_1 */
	DIP_X_REG_ADBS2_IVGN_0                          DIP_X_ADBS2_IVGN_0;                              /* 1644, 0x15023644, DIP_A_ADBS2_IVGN_0 */
	DIP_X_REG_ADBS2_IVGN_1                          DIP_X_ADBS2_IVGN_1;                              /* 1648, 0x15023648, DIP_A_ADBS2_IVGN_1 */
	DIP_X_REG_ADBS2_HDR                             DIP_X_ADBS2_HDR;                                 /* 164C, 0x1502364C, DIP_A_ADBS2_HDR */
	DIP_X_REG_ADBS2_CMDL_ONLY_1                     DIP_X_ADBS2_CMDL_ONLY_1;                         /* 1650, 0x15023650, DIP_A_ADBS2_CMDL_ONLY_1 */
	UINT32                                          rsv_1654[11];                                    /* 1654..167F, 0x15023654..1502367F */
	DIP_X_REG_DCPN_HDR_EN                           DIP_X_DCPN_HDR_EN;                               /* 1680, 0x15023680, DIP_A_DCPN_HDR_EN */
	DIP_X_REG_DCPN_IN_IMG_SIZE                      DIP_X_DCPN_IN_IMG_SIZE;                          /* 1684, 0x15023684, DIP_A_DCPN_IN_IMG_SIZE */
	DIP_X_REG_DCPN_ALGO_PARAM1                      DIP_X_DCPN_ALGO_PARAM1;                          /* 1688, 0x15023688, DIP_A_DCPN_ALGO_PARAM1 */
	DIP_X_REG_DCPN_ALGO_PARAM2                      DIP_X_DCPN_ALGO_PARAM2;                          /* 168C, 0x1502368C, DIP_A_DCPN_ALGO_PARAM2 */
	DIP_X_REG_DCPN_GTM_X0                           DIP_X_DCPN_GTM_X0;                               /* 1690, 0x15023690, DIP_A_DCPN_GTM_X0 */
	DIP_X_REG_DCPN_GTM_Y0                           DIP_X_DCPN_GTM_Y0;                               /* 1694, 0x15023694, DIP_A_DCPN_GTM_Y0 */
	DIP_X_REG_DCPN_GTM_S0                           DIP_X_DCPN_GTM_S0;                               /* 1698, 0x15023698, DIP_A_DCPN_GTM_S0 */
	DIP_X_REG_DCPN_GTM_S1                           DIP_X_DCPN_GTM_S1;                               /* 169C, 0x1502369C, DIP_A_DCPN_GTM_S1 */
	UINT32                                          rsv_16A0[8];                                     /* 16A0..16BF, 0x150236A0..150236BF */
	DIP_X_REG_CPN_HDR_CTL_EN                        DIP_X_CPN_HDR_CTL_EN;                            /* 16C0, 0x150236C0, DIP_A_CPN_HDR_CTL_EN */
	DIP_X_REG_CPN_IN_IMG_SIZE                       DIP_X_CPN_IN_IMG_SIZE;                           /* 16C4, 0x150236C4, DIP_A_CPN_IN_IMG_SIZE */
	DIP_X_REG_CPN_ALGO_PARAM1                       DIP_X_CPN_ALGO_PARAM1;                           /* 16C8, 0x150236C8, DIP_A_CPN_ALGO_PARAM1 */
	DIP_X_REG_CPN_ALGO_PARAM2                       DIP_X_CPN_ALGO_PARAM2;                           /* 16CC, 0x150236CC, DIP_A_CPN_ALGO_PARAM2 */
	DIP_X_REG_CPN_GTM_X0X1                          DIP_X_CPN_GTM_X0X1;                              /* 16D0, 0x150236D0, DIP_A_CPN_GTM_X0X1 */
	DIP_X_REG_CPN_GTM_X2X3                          DIP_X_CPN_GTM_X2X3;                              /* 16D4, 0x150236D4, DIP_A_CPN_GTM_X2X3 */
	DIP_X_REG_CPN_GTM_X4X5                          DIP_X_CPN_GTM_X4X5;                              /* 16D8, 0x150236D8, DIP_A_CPN_GTM_X4X5 */
	DIP_X_REG_CPN_GTM_X6                            DIP_X_CPN_GTM_X6;                                /* 16DC, 0x150236DC, DIP_A_CPN_GTM_X6 */
	DIP_X_REG_CPN_GTM_Y0Y1                          DIP_X_CPN_GTM_Y0Y1;                              /* 16E0, 0x150236E0, DIP_A_CPN_GTM_Y0Y1 */
	DIP_X_REG_CPN_GTM_Y2Y3                          DIP_X_CPN_GTM_Y2Y3;                              /* 16E4, 0x150236E4, DIP_A_CPN_GTM_Y2Y3 */
	DIP_X_REG_CPN_GTM_Y4Y5                          DIP_X_CPN_GTM_Y4Y5;                              /* 16E8, 0x150236E8, DIP_A_CPN_GTM_Y4Y5 */
	DIP_X_REG_CPN_GTM_Y6                            DIP_X_CPN_GTM_Y6;                                /* 16EC, 0x150236EC, DIP_A_CPN_GTM_Y6 */
	DIP_X_REG_CPN_GTM_S0S1                          DIP_X_CPN_GTM_S0S1;                              /* 16F0, 0x150236F0, DIP_A_CPN_GTM_S0S1 */
	DIP_X_REG_CPN_GTM_S2S3                          DIP_X_CPN_GTM_S2S3;                              /* 16F4, 0x150236F4, DIP_A_CPN_GTM_S2S3 */
	DIP_X_REG_CPN_GTM_S4S5                          DIP_X_CPN_GTM_S4S5;                              /* 16F8, 0x150236F8, DIP_A_CPN_GTM_S4S5 */
	DIP_X_REG_CPN_GTM_S6S7                          DIP_X_CPN_GTM_S6S7;                              /* 16FC, 0x150236FC, DIP_A_CPN_GTM_S6S7 */
	UINT32                                          rsv_1700[576];                                   /* 1700..1FFF, 0x15023700..15023FFF */
	DIP_X_REG_FLC_OFFST0                            DIP_X_FLC_OFFST0;                                /* 2000, 0x15024000, DIP_A_FLC_OFFST0 */
	DIP_X_REG_FLC_OFFST1                            DIP_X_FLC_OFFST1;                                /* 2004, 0x15024004, DIP_A_FLC_OFFST1 */
	DIP_X_REG_FLC_OFFST2                            DIP_X_FLC_OFFST2;                                /* 2008, 0x15024008, DIP_A_FLC_OFFST2 */
	UINT32                                          rsv_200C;                                        /* 200C, 0x1502400C */
	DIP_X_REG_FLC_GAIN0                             DIP_X_FLC_GAIN0;                                 /* 2010, 0x15024010, DIP_A_FLC_GAIN0 */
	DIP_X_REG_FLC_GAIN1                             DIP_X_FLC_GAIN1;                                 /* 2014, 0x15024014, DIP_A_FLC_GAIN1 */
	DIP_X_REG_FLC_GAIN2                             DIP_X_FLC_GAIN2;                                 /* 2018, 0x15024018, DIP_A_FLC_GAIN2 */
	UINT32                                          rsv_201C[9];                                     /* 201C..203F, 0x1502401C..1502403F */
	DIP_X_REG_FLC2_OFFST0                           DIP_X_FLC2_OFFST0;                               /* 2040, 0x15024040, DIP_A_FLC2_OFFST0 */
	DIP_X_REG_FLC2_OFFST1                           DIP_X_FLC2_OFFST1;                               /* 2044, 0x15024044, DIP_A_FLC2_OFFST1 */
	DIP_X_REG_FLC2_OFFST2                           DIP_X_FLC2_OFFST2;                               /* 2048, 0x15024048, DIP_A_FLC2_OFFST2 */
	UINT32                                          rsv_204C;                                        /* 204C, 0x1502404C */
	DIP_X_REG_FLC2_GAIN0                            DIP_X_FLC2_GAIN0;                                /* 2050, 0x15024050, DIP_A_FLC2_GAIN0 */
	DIP_X_REG_FLC2_GAIN1                            DIP_X_FLC2_GAIN1;                                /* 2054, 0x15024054, DIP_A_FLC2_GAIN1 */
	DIP_X_REG_FLC2_GAIN2                            DIP_X_FLC2_GAIN2;                                /* 2058, 0x15024058, DIP_A_FLC2_GAIN2 */
	UINT32                                          rsv_205C[9];                                     /* 205C..207F, 0x1502405C..1502407F */
	DIP_X_REG_G2G_CNV_1                             DIP_X_G2G_CNV_1;                                 /* 2080, 0x15024080, DIP_A_G2G_CNV_1 */
	DIP_X_REG_G2G_CNV_2                             DIP_X_G2G_CNV_2;                                 /* 2084, 0x15024084, DIP_A_G2G_CNV_2 */
	DIP_X_REG_G2G_CNV_3                             DIP_X_G2G_CNV_3;                                 /* 2088, 0x15024088, DIP_A_G2G_CNV_3 */
	DIP_X_REG_G2G_CNV_4                             DIP_X_G2G_CNV_4;                                 /* 208C, 0x1502408C, DIP_A_G2G_CNV_4 */
	DIP_X_REG_G2G_CNV_5                             DIP_X_G2G_CNV_5;                                 /* 2090, 0x15024090, DIP_A_G2G_CNV_5 */
	DIP_X_REG_G2G_CNV_6                             DIP_X_G2G_CNV_6;                                 /* 2094, 0x15024094, DIP_A_G2G_CNV_6 */
	DIP_X_REG_G2G_CTRL                              DIP_X_G2G_CTRL;                                  /* 2098, 0x15024098, DIP_A_G2G_CTRL */
	DIP_X_REG_G2G_CFC                               DIP_X_G2G_CFC;                                   /* 209C, 0x1502409C, DIP_A_G2G_CFC */
	UINT32                                          rsv_20A0[8];                                     /* 20A0..20BF, 0x150240A0..150240BF */
	DIP_X_REG_G2G2_CNV_1                            DIP_X_G2G2_CNV_1;                                /* 20C0, 0x150240C0, DIP_A_G2G2_CNV_1 */
	DIP_X_REG_G2G2_CNV_2                            DIP_X_G2G2_CNV_2;                                /* 20C4, 0x150240C4, DIP_A_G2G2_CNV_2 */
	DIP_X_REG_G2G2_CNV_3                            DIP_X_G2G2_CNV_3;                                /* 20C8, 0x150240C8, DIP_A_G2G2_CNV_3 */
	DIP_X_REG_G2G2_CNV_4                            DIP_X_G2G2_CNV_4;                                /* 20CC, 0x150240CC, DIP_A_G2G2_CNV_4 */
	DIP_X_REG_G2G2_CNV_5                            DIP_X_G2G2_CNV_5;                                /* 20D0, 0x150240D0, DIP_A_G2G2_CNV_5 */
	DIP_X_REG_G2G2_CNV_6                            DIP_X_G2G2_CNV_6;                                /* 20D4, 0x150240D4, DIP_A_G2G2_CNV_6 */
	DIP_X_REG_G2G2_CTRL                             DIP_X_G2G2_CTRL;                                 /* 20D8, 0x150240D8, DIP_A_G2G2_CTRL */
	DIP_X_REG_G2G2_CFC                              DIP_X_G2G2_CFC;                                  /* 20DC, 0x150240DC, DIP_A_G2G2_CFC */
	UINT32                                          rsv_20E0[8];                                     /* 20E0..20FF, 0x150240E0..150240FF */
	DIP_X_REG_LCE25_CON                             DIP_X_LCE25_CON;                                 /* 2100, 0x15024100, DIP_A_LCE25_CON */
	DIP_X_REG_LCE25_ZR                              DIP_X_LCE25_ZR;                                  /* 2104, 0x15024104, DIP_A_LCE25_ZR */
	DIP_X_REG_LCE25_SLM_SIZE                        DIP_X_LCE25_SLM_SIZE;                            /* 2108, 0x15024108, DIP_A_LCE25_SLM_SIZE */
	DIP_X_REG_LCE25_OFST                            DIP_X_LCE25_OFST;                                /* 210C, 0x1502410C, DIP_A_LCE25_OFST */
	DIP_X_REG_LCE25_BIAS                            DIP_X_LCE25_BIAS;                                /* 2110, 0x15024110, DIP_A_LCE25_BIAS */
	DIP_X_REG_LCE25_IMAGE_SIZE                      DIP_X_LCE25_IMAGE_SIZE;                          /* 2114, 0x15024114, DIP_A_LCE25_IMAGE_SIZE */
	DIP_X_REG_LCE25_BIL_TH0                         DIP_X_LCE25_BIL_TH0;                             /* 2118, 0x15024118, DIP_A_LCE25_BIL_TH0 */
	DIP_X_REG_LCE25_BIL_TH1                         DIP_X_LCE25_BIL_TH1;                             /* 211C, 0x1502411C, DIP_A_LCE25_BIL_TH1 */
	DIP_X_REG_LCE25_TM_PARA0                        DIP_X_LCE25_TM_PARA0;                            /* 2120, 0x15024120, DIP_A_LCE25_TM_PARA0 */
	DIP_X_REG_LCE25_TM_PARA1                        DIP_X_LCE25_TM_PARA1;                            /* 2124, 0x15024124, DIP_A_LCE25_TM_PARA1 */
	DIP_X_REG_LCE25_TM_PARA2                        DIP_X_LCE25_TM_PARA2;                            /* 2128, 0x15024128, DIP_A_LCE25_TM_PARA2 */
	DIP_X_REG_LCE25_TM_PARA3                        DIP_X_LCE25_TM_PARA3;                            /* 212C, 0x1502412C, DIP_A_LCE25_TM_PARA3 */
	DIP_X_REG_LCE25_TM_PARA4                        DIP_X_LCE25_TM_PARA4;                            /* 2130, 0x15024130, DIP_A_LCE25_TM_PARA4 */
	DIP_X_REG_LCE25_TM_PARA5                        DIP_X_LCE25_TM_PARA5;                            /* 2134, 0x15024134, DIP_A_LCE25_TM_PARA5 */
	DIP_X_REG_LCE25_TM_PARA6                        DIP_X_LCE25_TM_PARA6;                            /* 2138, 0x15024138, DIP_A_LCE25_TM_PARA6 */
	DIP_X_REG_LCE25_HLR_PARA0                       DIP_X_LCE25_HLR_PARA0;                           /* 213C, 0x1502413C, DIP_A_LCE25_HLR_PARA0 */
	DIP_X_REG_LCE25_HLR_PARA1                       DIP_X_LCE25_HLR_PARA1;                           /* 2140, 0x15024140, DIP_A_LCE25_HLR_PARA1 */
	DIP_X_REG_LCE25_HLR_PARA2                       DIP_X_LCE25_HLR_PARA2;                           /* 2144, 0x15024144, DIP_A_LCE25_HLR_PARA2 */
	DIP_X_REG_LCE25_HLR_PARA3                       DIP_X_LCE25_HLR_PARA3;                           /* 2148, 0x15024148, DIP_A_LCE25_HLR_PARA3 */
	DIP_X_REG_LCE25_HLR_PARA4                       DIP_X_LCE25_HLR_PARA4;                           /* 214C, 0x1502414C, DIP_A_LCE25_HLR_PARA4 */
	DIP_X_REG_LCE25_TM_PARA7                        DIP_X_LCE25_TM_PARA7;                            /* 2150, 0x15024150, DIP_A_LCE25_TM_PARA7 */
	DIP_X_REG_LCE25_TM_PARA8                        DIP_X_LCE25_TM_PARA8;                            /* 2154, 0x15024154, DIP_A_LCE25_TM_PARA8 */
	DIP_X_REG_LCE25_HLR_PARA5                       DIP_X_LCE25_HLR_PARA5;                           /* 2158, 0x15024158, DIP_A_LCE25_HLR_PARA5 */
	DIP_X_REG_LCE25_HLR_PARA6                       DIP_X_LCE25_HLR_PARA6;                           /* 215C, 0x1502415C, DIP_A_LCE25_HLR_PARA6 */
	DIP_X_REG_LCE25_TM_PARA9                        DIP_X_LCE25_TM_PARA9;                            /* 2160, 0x15024160, DIP_A_LCE25_TM_PARA9 */
	DIP_X_REG_LCE25_HLR_PARA7                       DIP_X_LCE25_HLR_PARA7;                           /* 2164, 0x15024164, DIP_A_LCE25_HLR_PARA7 */
	DIP_X_REG_LCE25_TCHL_PARA0                      DIP_X_LCE25_TCHL_PARA0;                          /* 2168, 0x15024168, DIP_A_LCE25_TCHL_PARA0 */
	DIP_X_REG_LCE25_TCHL_PARA1                      DIP_X_LCE25_TCHL_PARA1;                          /* 216C, 0x1502416C, DIP_A_LCE25_TCHL_PARA1 */
	DIP_X_REG_LCE25_TCHL_PARA2                      DIP_X_LCE25_TCHL_PARA2;                          /* 2170, 0x15024170, DIP_A_LCE25_TCHL_PARA2 */
	UINT32                                          rsv_2174[3];                                     /* 2174..217F, 0x15024174..1502417F */
	DIP_X_REG_GGM_LUT                               DIP_X_GGM_LUT[192];                              /* 2180..247F, 0x15024180..1502447F, DIP_A_GGM_LUT */
	DIP_X_REG_GGM_CTRL                              DIP_X_GGM_CTRL;                                  /* 2480, 0x15024480, DIP_A_GGM_CTRL */
	UINT32                                          rsv_2484[111];                                   /* 2484..263F, 0x15024484..1502463F */
	DIP_X_REG_GGM2_LUT                              DIP_X_GGM2_LUT[192];                             /* 2640..293F, 0x15024640..1502493F, DIP_A_GGM2_LUT */
	DIP_X_REG_GGM2_CTRL                             DIP_X_GGM2_CTRL;                                 /* 2940, 0x15024940, DIP_A_GGM2_CTRL */
	UINT32                                          rsv_2944[111];                                   /* 2944..2AFF, 0x15024944..15024AFF */
	DIP_X_REG_WSHIFT_SPARE0                         DIP_X_WSHIFT_SPARE0;                             /* 2B00, 0x15024B00, DIP_A_WSHIFT_SPARE0 */
	DIP_X_REG_WSHIFT_SPARE1                         DIP_X_WSHIFT_SPARE1;                             /* 2B04, 0x15024B04, DIP_A_WSHIFT_SPARE1 */
	DIP_X_REG_WSHIFT_SPARE2                         DIP_X_WSHIFT_SPARE2;                             /* 2B08, 0x15024B08, DIP_A_WSHIFT_SPARE2 */
	UINT32                                          rsv_2B0C;                                        /* 2B0C, 0x15024B0C */
	DIP_X_REG_WSHIFT_SPARE3                         DIP_X_WSHIFT_SPARE3;                             /* 2B10, 0x15024B10, DIP_A_WSHIFT_SPARE3 */
	UINT32                                          rsv_2B14[11];                                    /* 2B14..2B3F, 0x15024B14..15024B3F */
	DIP_X_REG_WSYNC_SPARE0                          DIP_X_WSYNC_SPARE0;                              /* 2B40, 0x15024B40, DIP_A_WSYNC_SPARE0 */
	DIP_X_REG_WSYNC_SPARE1                          DIP_X_WSYNC_SPARE1;                              /* 2B44, 0x15024B44, DIP_A_WSYNC_SPARE1 */
	DIP_X_REG_WSYNC_SPARE2                          DIP_X_WSYNC_SPARE2;                              /* 2B48, 0x15024B48, DIP_A_WSYNC_SPARE2 */
	UINT32                                          rsv_2B4C;                                        /* 2B4C, 0x15024B4C */
	DIP_X_REG_WSYNC_SPARE3                          DIP_X_WSYNC_SPARE3;                              /* 2B50, 0x15024B50, DIP_A_WSYNC_SPARE3 */
	UINT32                                          rsv_2B54[11];                                    /* 2B54..2B7F, 0x15024B54..15024B7F */
	DIP_X_REG_MDP_CROP2_X                           DIP_X_MDP_CROP2_X;                               /* 2B80, 0x15024B80, DIP_A_MDP_CROP2_X */
	DIP_X_REG_MDP_CROP2_Y                           DIP_X_MDP_CROP2_Y;                               /* 2B84, 0x15024B84, DIP_A_MDP_CROP2_Y */
	UINT32                                          rsv_2B88[14];                                    /* 2B88..2BBF, 0x15024B88..15024BBF */
	DIP_X_REG_SMX4_CTL                              DIP_X_SMX4_CTL;                                  /* 2BC0, 0x15024BC0, DIP_A_SMX4_CTL */
	DIP_X_REG_SMX4_TRANS_CON                        DIP_X_SMX4_TRANS_CON;                            /* 2BC4, 0x15024BC4, DIP_A_SMX4_TRANS_CON */
	DIP_X_REG_SMX4_SPARE                            DIP_X_SMX4_SPARE;                                /* 2BC8, 0x15024BC8, DIP_A_SMX4_SPARE */
	DIP_X_REG_SMX4_CRPINL_CON1                      DIP_X_SMX4_CRPINL_CON1;                          /* 2BCC, 0x15024BCC, DIP_A_SMX4_CRPINL_CON1 */
	DIP_X_REG_SMX4_CRPINL_CON2                      DIP_X_SMX4_CRPINL_CON2;                          /* 2BD0, 0x15024BD0, DIP_A_SMX4_CRPINL_CON2 */
	DIP_X_REG_SMX4_CRPINR_CON1                      DIP_X_SMX4_CRPINR_CON1;                          /* 2BD4, 0x15024BD4, DIP_A_SMX4_CRPINR_CON1 */
	DIP_X_REG_SMX4_CRPINR_CON2                      DIP_X_SMX4_CRPINR_CON2;                          /* 2BD8, 0x15024BD8, DIP_A_SMX4_CRPINR_CON2 */
	DIP_X_REG_SMX4_CRPOUT_CON1                      DIP_X_SMX4_CRPOUT_CON1;                          /* 2BDC, 0x15024BDC, DIP_A_SMX4_CRPOUT_CON1 */
	DIP_X_REG_SMX4_CRPOUT_CON2                      DIP_X_SMX4_CRPOUT_CON2;                          /* 2BE0, 0x15024BE0, DIP_A_SMX4_CRPOUT_CON2 */
	UINT32                                          rsv_2BE4[263];                                   /* 2BE4..2FFF, 0x15024BE4..15024FFF */
	DIP_X_REG_C24_TILE_EDGE                         DIP_X_C24_TILE_EDGE;                             /* 3000, 0x15025000, DIP_A_C24_TILE_EDGE */
	UINT32                                          rsv_3004[15];                                    /* 3004..303F, 0x15025004..1502503F */
	DIP_X_REG_C42_CON                               DIP_X_C42_CON;                                   /* 3040, 0x15025040, DIP_A_C42_CON */
	UINT32                                          rsv_3044[15];                                    /* 3044..307F, 0x15025044..1502507F */
	DIP_X_REG_C02_CON                               DIP_X_C02_CON;                                   /* 3080, 0x15025080, DIP_A_C02_CON */
	DIP_X_REG_C02_CROP_CON1                         DIP_X_C02_CROP_CON1;                             /* 3084, 0x15025084, DIP_A_C02_CROP_CON1 */
	DIP_X_REG_C02_CROP_CON2                         DIP_X_C02_CROP_CON2;                             /* 3088, 0x15025088, DIP_A_C02_CROP_CON2 */
	UINT32                                          rsv_308C[13];                                    /* 308C..30BF, 0x1502508C..150250BF */
	DIP_X_REG_C02B_CON                              DIP_X_C02B_CON;                                  /* 30C0, 0x150250C0, DIP_A_C02B_CON */
	DIP_X_REG_C02B_CROP_CON1                        DIP_X_C02B_CROP_CON1;                            /* 30C4, 0x150250C4, DIP_A_C02B_CROP_CON1 */
	DIP_X_REG_C02B_CROP_CON2                        DIP_X_C02B_CROP_CON2;                            /* 30C8, 0x150250C8, DIP_A_C02B_CROP_CON2 */
	UINT32                                          rsv_30CC[93];                                    /* 30CC..323F, 0x150250CC..1502523F */
	DIP_X_REG_G2C_CONV_0A                           DIP_X_G2C_CONV_0A;                               /* 3240, 0x15025240, DIP_A_G2C_CONV_0A */
	DIP_X_REG_G2C_CONV_0B                           DIP_X_G2C_CONV_0B;                               /* 3244, 0x15025244, DIP_A_G2C_CONV_0B */
	DIP_X_REG_G2C_CONV_1A                           DIP_X_G2C_CONV_1A;                               /* 3248, 0x15025248, DIP_A_G2C_CONV_1A */
	DIP_X_REG_G2C_CONV_1B                           DIP_X_G2C_CONV_1B;                               /* 324C, 0x1502524C, DIP_A_G2C_CONV_1B */
	DIP_X_REG_G2C_CONV_2A                           DIP_X_G2C_CONV_2A;                               /* 3250, 0x15025250, DIP_A_G2C_CONV_2A */
	DIP_X_REG_G2C_CONV_2B                           DIP_X_G2C_CONV_2B;                               /* 3254, 0x15025254, DIP_A_G2C_CONV_2B */
	DIP_X_REG_G2C_SHADE_CON_1                       DIP_X_G2C_SHADE_CON_1;                           /* 3258, 0x15025258, DIP_A_G2C_SHADE_CON_1 */
	DIP_X_REG_G2C_SHADE_CON_2                       DIP_X_G2C_SHADE_CON_2;                           /* 325C, 0x1502525C, DIP_A_G2C_SHADE_CON_2 */
	DIP_X_REG_G2C_SHADE_CON_3                       DIP_X_G2C_SHADE_CON_3;                           /* 3260, 0x15025260, DIP_A_G2C_SHADE_CON_3 */
	DIP_X_REG_G2C_SHADE_TAR                         DIP_X_G2C_SHADE_TAR;                             /* 3264, 0x15025264, DIP_A_G2C_SHADE_TAR */
	DIP_X_REG_G2C_SHADE_SP                          DIP_X_G2C_SHADE_SP;                              /* 3268, 0x15025268, DIP_A_G2C_SHADE_SP */
	DIP_X_REG_G2C_CFC_CON_1                         DIP_X_G2C_CFC_CON_1;                             /* 326C, 0x1502526C, DIP_A_G2C_CFC_CON_1 */
	DIP_X_REG_G2C_CFC_CON_2                         DIP_X_G2C_CFC_CON_2;                             /* 3270, 0x15025270, DIP_A_G2C_CFC_CON_2 */
	UINT32                                          rsv_3274[19];                                    /* 3274..32BF, 0x15025274..150252BF */
	DIP_X_REG_MIX3_CTRL_0                           DIP_X_MIX3_CTRL_0;                               /* 32C0, 0x150252C0, DIP_A_MIX3_CTRL_0 */
	DIP_X_REG_MIX3_CTRL_1                           DIP_X_MIX3_CTRL_1;                               /* 32C4, 0x150252C4, DIP_A_MIX3_CTRL_1 */
	DIP_X_REG_MIX3_SPARE                            DIP_X_MIX3_SPARE;                                /* 32C8, 0x150252C8, DIP_A_MIX3_SPARE */
	UINT32                                          rsv_32CC[13];                                    /* 32CC..32FF, 0x150252CC..150252FF */
	DIP_X_REG_NBC_ANR_TBL                           DIP_X_NBC_ANR_TBL[256];                          /* 3300..36FF, 0x15025300..150256FF, DIP_A_NBC_ANR_TBL */
	DIP_X_REG_NBC_ANR_CON1                          DIP_X_NBC_ANR_CON1;                              /* 3700, 0x15025700, DIP_A_NBC_ANR_CON1 */
	DIP_X_REG_NBC_ANR_CON2                          DIP_X_NBC_ANR_CON2;                              /* 3704, 0x15025704, DIP_A_NBC_ANR_CON2 */
	DIP_X_REG_NBC_ANR_YAD1                          DIP_X_NBC_ANR_YAD1;                              /* 3708, 0x15025708, DIP_A_NBC_ANR_YAD1 */
	DIP_X_REG_NBC_ANR_YAD2                          DIP_X_NBC_ANR_YAD2;                              /* 370C, 0x1502570C, DIP_A_NBC_ANR_YAD2 */
	DIP_X_REG_NBC_ANR_Y4LUT1                        DIP_X_NBC_ANR_Y4LUT1;                            /* 3710, 0x15025710, DIP_A_NBC_ANR_Y4LUT1 */
	DIP_X_REG_NBC_ANR_Y4LUT2                        DIP_X_NBC_ANR_Y4LUT2;                            /* 3714, 0x15025714, DIP_A_NBC_ANR_Y4LUT2 */
	DIP_X_REG_NBC_ANR_Y4LUT3                        DIP_X_NBC_ANR_Y4LUT3;                            /* 3718, 0x15025718, DIP_A_NBC_ANR_Y4LUT3 */
	DIP_X_REG_NBC_ANR_C4LUT1                        DIP_X_NBC_ANR_C4LUT1;                            /* 371C, 0x1502571C, DIP_A_NBC_ANR_C4LUT1 */
	DIP_X_REG_NBC_ANR_C4LUT2                        DIP_X_NBC_ANR_C4LUT2;                            /* 3720, 0x15025720, DIP_A_NBC_ANR_C4LUT2 */
	DIP_X_REG_NBC_ANR_C4LUT3                        DIP_X_NBC_ANR_C4LUT3;                            /* 3724, 0x15025724, DIP_A_NBC_ANR_C4LUT3 */
	DIP_X_REG_NBC_ANR_A4LUT2                        DIP_X_NBC_ANR_A4LUT2;                            /* 3728, 0x15025728, DIP_A_NBC_ANR_A4LUT2 */
	DIP_X_REG_NBC_ANR_A4LUT3                        DIP_X_NBC_ANR_A4LUT3;                            /* 372C, 0x1502572C, DIP_A_NBC_ANR_A4LUT3 */
	DIP_X_REG_NBC_ANR_L4LUT1                        DIP_X_NBC_ANR_L4LUT1;                            /* 3730, 0x15025730, DIP_A_NBC_ANR_L4LUT1 */
	DIP_X_REG_NBC_ANR_L4LUT2                        DIP_X_NBC_ANR_L4LUT2;                            /* 3734, 0x15025734, DIP_A_NBC_ANR_L4LUT2 */
	DIP_X_REG_NBC_ANR_L4LUT3                        DIP_X_NBC_ANR_L4LUT3;                            /* 3738, 0x15025738, DIP_A_NBC_ANR_L4LUT3 */
	DIP_X_REG_NBC_ANR_PTY0V                         DIP_X_NBC_ANR_PTY0V;                             /* 373C, 0x1502573C, DIP_A_NBC_ANR_PTY0V */
	DIP_X_REG_NBC_ANR_CAD                           DIP_X_NBC_ANR_CAD;                               /* 3740, 0x15025740, DIP_A_NBC_ANR_CAD */
	DIP_X_REG_NBC_ANR_PTY1V                         DIP_X_NBC_ANR_PTY1V;                             /* 3744, 0x15025744, DIP_A_NBC_ANR_PTY1V */
	DIP_X_REG_NBC_ANR_SL2                           DIP_X_NBC_ANR_SL2;                               /* 3748, 0x15025748, DIP_A_NBC_ANR_SL2 */
	DIP_X_REG_NBC_ANR_PTY2V                         DIP_X_NBC_ANR_PTY2V;                             /* 374C, 0x1502574C, DIP_A_NBC_ANR_PTY2V */
	DIP_X_REG_NBC_ANR_PTY3V                         DIP_X_NBC_ANR_PTY3V;                             /* 3750, 0x15025750, DIP_A_NBC_ANR_PTY3V */
	DIP_X_REG_NBC_ANR_PTY0H                         DIP_X_NBC_ANR_PTY0H;                             /* 3754, 0x15025754, DIP_A_NBC_ANR_PTY0H */
	DIP_X_REG_NBC_ANR_PTY1H                         DIP_X_NBC_ANR_PTY1H;                             /* 3758, 0x15025758, DIP_A_NBC_ANR_PTY1H */
	DIP_X_REG_NBC_ANR_PTY2H                         DIP_X_NBC_ANR_PTY2H;                             /* 375C, 0x1502575C, DIP_A_NBC_ANR_PTY2H */
	DIP_X_REG_NBC_ANR_T4LUT1                        DIP_X_NBC_ANR_T4LUT1;                            /* 3760, 0x15025760, DIP_A_NBC_ANR_T4LUT1 */
	DIP_X_REG_NBC_ANR_T4LUT2                        DIP_X_NBC_ANR_T4LUT2;                            /* 3764, 0x15025764, DIP_A_NBC_ANR_T4LUT2 */
	DIP_X_REG_NBC_ANR_T4LUT3                        DIP_X_NBC_ANR_T4LUT3;                            /* 3768, 0x15025768, DIP_A_NBC_ANR_T4LUT3 */
	DIP_X_REG_NBC_ANR_ACT1                          DIP_X_NBC_ANR_ACT1;                              /* 376C, 0x1502576C, DIP_A_NBC_ANR_ACT1 */
	DIP_X_REG_NBC_ANR_PTY3H                         DIP_X_NBC_ANR_PTY3H;                             /* 3770, 0x15025770, DIP_A_NBC_ANR_PTY3H */
	DIP_X_REG_NBC_ANR_PTCV                          DIP_X_NBC_ANR_PTCV;                              /* 3774, 0x15025774, DIP_A_NBC_ANR_PTCV */
	DIP_X_REG_NBC_ANR_ACT4                          DIP_X_NBC_ANR_ACT4;                              /* 3778, 0x15025778, DIP_A_NBC_ANR_ACT4 */
	DIP_X_REG_NBC_ANR_PTCH                          DIP_X_NBC_ANR_PTCH;                              /* 377C, 0x1502577C, DIP_A_NBC_ANR_PTCH */
	DIP_X_REG_NBC_ANR_YLVL0                         DIP_X_NBC_ANR_YLVL0;                             /* 3780, 0x15025780, DIP_A_NBC_ANR_YLVL0 */
	DIP_X_REG_NBC_ANR_YLVL1                         DIP_X_NBC_ANR_YLVL1;                             /* 3784, 0x15025784, DIP_A_NBC_ANR_YLVL1 */
	DIP_X_REG_NBC_ANR_HF_COR                        DIP_X_NBC_ANR_HF_COR;                            /* 3788, 0x15025788, DIP_A_NBC_ANR_HF_COR */
	DIP_X_REG_NBC_ANR_HF_ACT0                       DIP_X_NBC_ANR_HF_ACT0;                           /* 378C, 0x1502578C, DIP_A_NBC_ANR_HF_ACT0 */
	DIP_X_REG_NBC_ANR_HF_ACT1                       DIP_X_NBC_ANR_HF_ACT1;                           /* 3790, 0x15025790, DIP_A_NBC_ANR_HF_ACT1 */
	DIP_X_REG_NBC_ANR_ACTC                          DIP_X_NBC_ANR_ACTC;                              /* 3794, 0x15025794, DIP_A_NBC_ANR_ACTC */
	DIP_X_REG_NBC_ANR_YLAD                          DIP_X_NBC_ANR_YLAD;                              /* 3798, 0x15025798, DIP_A_NBC_ANR_YLAD */
	DIP_X_REG_NBC_ANR_HF_ACT2                       DIP_X_NBC_ANR_HF_ACT2;                           /* 379C, 0x1502579C, DIP_A_NBC_ANR_HF_ACT2 */
	DIP_X_REG_NBC_ANR_HF_ACT3                       DIP_X_NBC_ANR_HF_ACT3;                           /* 37A0, 0x150257A0, DIP_A_NBC_ANR_HF_ACT3 */
	DIP_X_REG_NBC_ANR_HF_LUMA0                      DIP_X_NBC_ANR_HF_LUMA0;                          /* 37A4, 0x150257A4, DIP_A_NBC_ANR_HF_LUMA0 */
	DIP_X_REG_NBC_ANR_HF_LUMA1                      DIP_X_NBC_ANR_HF_LUMA1;                          /* 37A8, 0x150257A8, DIP_A_NBC_ANR_HF_LUMA1 */
	DIP_X_REG_NBC_ANR_LTM_GAIN1                     DIP_X_NBC_ANR_LTM_GAIN1;                         /* 37AC, 0x150257AC, DIP_A_NBC_ANR_LTM_GAIN1 */
	DIP_X_REG_NBC_ANR_LTM_GAIN2                     DIP_X_NBC_ANR_LTM_GAIN2;                         /* 37B0, 0x150257B0, DIP_A_NBC_ANR_LTM_GAIN2 */
	DIP_X_REG_NBC_ANR_LTM_LUTP1                     DIP_X_NBC_ANR_LTM_LUTP1;                         /* 37B4, 0x150257B4, DIP_A_NBC_ANR_LTM_LUTP1 */
	DIP_X_REG_NBC_ANR_LTM_LUTP2                     DIP_X_NBC_ANR_LTM_LUTP2;                         /* 37B8, 0x150257B8, DIP_A_NBC_ANR_LTM_LUTP2 */
	DIP_X_REG_NBC_ANR_LTM_LUTO1                     DIP_X_NBC_ANR_LTM_LUTO1;                         /* 37BC, 0x150257BC, DIP_A_NBC_ANR_LTM_LUTO1 */
	DIP_X_REG_NBC_ANR_LTM_LUTO2                     DIP_X_NBC_ANR_LTM_LUTO2;                         /* 37C0, 0x150257C0, DIP_A_NBC_ANR_LTM_LUTO2 */
	DIP_X_REG_NBC_ANR_LTM_LUTS1                     DIP_X_NBC_ANR_LTM_LUTS1;                         /* 37C4, 0x150257C4, DIP_A_NBC_ANR_LTM_LUTS1 */
	DIP_X_REG_NBC_ANR_LTM_LUTS2                     DIP_X_NBC_ANR_LTM_LUTS2;                         /* 37C8, 0x150257C8, DIP_A_NBC_ANR_LTM_LUTS2 */
	DIP_X_REG_NBC_ANR_LTM_LUTS3                     DIP_X_NBC_ANR_LTM_LUTS3;                         /* 37CC, 0x150257CC, DIP_A_NBC_ANR_LTM_LUTS3 */
	DIP_X_REG_NBC_ANR_LTM_LUTS4                     DIP_X_NBC_ANR_LTM_LUTS4;                         /* 37D0, 0x150257D0, DIP_A_NBC_ANR_LTM_LUTS4 */
	DIP_X_REG_NBC_ANR_Y4LUT4                        DIP_X_NBC_ANR_Y4LUT4;                            /* 37D4, 0x150257D4, DIP_A_NBC_ANR_Y4LUT4 */
	DIP_X_REG_NBC_ANR_Y4LUT5                        DIP_X_NBC_ANR_Y4LUT5;                            /* 37D8, 0x150257D8, DIP_A_NBC_ANR_Y4LUT5 */
	DIP_X_REG_NBC_ANR_Y4LUT6                        DIP_X_NBC_ANR_Y4LUT6;                            /* 37DC, 0x150257DC, DIP_A_NBC_ANR_Y4LUT6 */
	DIP_X_REG_NBC_ANR_Y4LUT7                        DIP_X_NBC_ANR_Y4LUT7;                            /* 37E0, 0x150257E0, DIP_A_NBC_ANR_Y4LUT7 */
	DIP_X_REG_NBC_ANR_A4LUT1                        DIP_X_NBC_ANR_A4LUT1;                            /* 37E4, 0x150257E4, DIP_A_NBC_ANR_A4LUT1 */
	UINT32                                          rsv_37E8[2];                                     /* 37E8..37EF, 0x150257E8..150257EF */
	DIP_X_REG_NBC_ANR_RSV1                          DIP_X_NBC_ANR_RSV1;                              /* 37F0, 0x150257F0, DIP_A_NBC_ANR_RSV1 */
	UINT32                                          rsv_37F4[3];                                     /* 37F4..37FF, 0x150257F4..150257FF */
	DIP_X_REG_SL2B_CEN                              DIP_X_SL2B_CEN;                                  /* 3800, 0x15025800, DIP_A_SL2B_CEN */
	DIP_X_REG_SL2B_RR_CON0                          DIP_X_SL2B_RR_CON0;                              /* 3804, 0x15025804, DIP_A_SL2B_RR_CON0 */
	DIP_X_REG_SL2B_RR_CON1                          DIP_X_SL2B_RR_CON1;                              /* 3808, 0x15025808, DIP_A_SL2B_RR_CON1 */
	DIP_X_REG_SL2B_GAIN                             DIP_X_SL2B_GAIN;                                 /* 380C, 0x1502580C, DIP_A_SL2B_GAIN */
	DIP_X_REG_SL2B_RZ                               DIP_X_SL2B_RZ;                                   /* 3810, 0x15025810, DIP_A_SL2B_RZ */
	DIP_X_REG_SL2B_XOFF                             DIP_X_SL2B_XOFF;                                 /* 3814, 0x15025814, DIP_A_SL2B_XOFF */
	DIP_X_REG_SL2B_YOFF                             DIP_X_SL2B_YOFF;                                 /* 3818, 0x15025818, DIP_A_SL2B_YOFF */
	DIP_X_REG_SL2B_SLP_CON0                         DIP_X_SL2B_SLP_CON0;                             /* 381C, 0x1502581C, DIP_A_SL2B_SLP_CON0 */
	DIP_X_REG_SL2B_SLP_CON1                         DIP_X_SL2B_SLP_CON1;                             /* 3820, 0x15025820, DIP_A_SL2B_SLP_CON1 */
	DIP_X_REG_SL2B_SLP_CON2                         DIP_X_SL2B_SLP_CON2;                             /* 3824, 0x15025824, DIP_A_SL2B_SLP_CON2 */
	DIP_X_REG_SL2B_SLP_CON3                         DIP_X_SL2B_SLP_CON3;                             /* 3828, 0x15025828, DIP_A_SL2B_SLP_CON3 */
	DIP_X_REG_SL2B_SIZE                             DIP_X_SL2B_SIZE;                                 /* 382C, 0x1502582C, DIP_A_SL2B_SIZE */
	UINT32                                          rsv_3830[4];                                     /* 3830..383F, 0x15025830..1502583F */
	DIP_X_REG_NDG_RAN_0                             DIP_X_NDG_RAN_0;                                 /* 3840, 0x15025840, DIP_A_NDG_RAN_0 */
	DIP_X_REG_NDG_RAN_1                             DIP_X_NDG_RAN_1;                                 /* 3844, 0x15025844, DIP_A_NDG_RAN_1 */
	DIP_X_REG_NDG_RAN_2                             DIP_X_NDG_RAN_2;                                 /* 3848, 0x15025848, DIP_A_NDG_RAN_2 */
	DIP_X_REG_NDG_RAN_3                             DIP_X_NDG_RAN_3;                                 /* 384C, 0x1502584C, DIP_A_NDG_RAN_3 */
	DIP_X_REG_NDG_CROP_X                            DIP_X_NDG_CROP_X;                                /* 3850, 0x15025850, DIP_A_NDG_CROP_X */
	DIP_X_REG_NDG_CROP_Y                            DIP_X_NDG_CROP_Y;                                /* 3854, 0x15025854, DIP_A_NDG_CROP_Y */
	UINT32                                          rsv_3858[10];                                    /* 3858..387F, 0x15025858..1502587F */
	DIP_X_REG_SRZ4_CONTROL                          DIP_X_SRZ4_CONTROL;                              /* 3880, 0x15025880, DIP_A_SRZ4_CONTROL */
	DIP_X_REG_SRZ4_IN_IMG                           DIP_X_SRZ4_IN_IMG;                               /* 3884, 0x15025884, DIP_A_SRZ4_IN_IMG */
	DIP_X_REG_SRZ4_OUT_IMG                          DIP_X_SRZ4_OUT_IMG;                              /* 3888, 0x15025888, DIP_A_SRZ4_OUT_IMG */
	DIP_X_REG_SRZ4_HORI_STEP                        DIP_X_SRZ4_HORI_STEP;                            /* 388C, 0x1502588C, DIP_A_SRZ4_HORI_STEP */
	DIP_X_REG_SRZ4_VERT_STEP                        DIP_X_SRZ4_VERT_STEP;                            /* 3890, 0x15025890, DIP_A_SRZ4_VERT_STEP */
	DIP_X_REG_SRZ4_HORI_INT_OFST                    DIP_X_SRZ4_HORI_INT_OFST;                        /* 3894, 0x15025894, DIP_A_SRZ4_HORI_INT_OFST */
	DIP_X_REG_SRZ4_HORI_SUB_OFST                    DIP_X_SRZ4_HORI_SUB_OFST;                        /* 3898, 0x15025898, DIP_A_SRZ4_HORI_SUB_OFST */
	DIP_X_REG_SRZ4_VERT_INT_OFST                    DIP_X_SRZ4_VERT_INT_OFST;                        /* 389C, 0x1502589C, DIP_A_SRZ4_VERT_INT_OFST */
	DIP_X_REG_SRZ4_VERT_SUB_OFST                    DIP_X_SRZ4_VERT_SUB_OFST;                        /* 38A0, 0x150258A0, DIP_A_SRZ4_VERT_SUB_OFST */
	UINT32                                          rsv_38A4[7];                                     /* 38A4..38BF, 0x150258A4..150258BF */
	DIP_X_REG_MIX1_CTRL_0                           DIP_X_MIX1_CTRL_0;                               /* 38C0, 0x150258C0, DIP_A_MIX1_CTRL_0 */
	DIP_X_REG_MIX1_CTRL_1                           DIP_X_MIX1_CTRL_1;                               /* 38C4, 0x150258C4, DIP_A_MIX1_CTRL_1 */
	DIP_X_REG_MIX1_SPARE                            DIP_X_MIX1_SPARE;                                /* 38C8, 0x150258C8, DIP_A_MIX1_SPARE */
	UINT32                                          rsv_38CC[13];                                    /* 38CC..38FF, 0x150258CC..150258FF */
	DIP_X_REG_SRZ1_CONTROL                          DIP_X_SRZ1_CONTROL;                              /* 3900, 0x15025900, DIP_A_SRZ1_CONTROL */
	DIP_X_REG_SRZ1_IN_IMG                           DIP_X_SRZ1_IN_IMG;                               /* 3904, 0x15025904, DIP_A_SRZ1_IN_IMG */
	DIP_X_REG_SRZ1_OUT_IMG                          DIP_X_SRZ1_OUT_IMG;                              /* 3908, 0x15025908, DIP_A_SRZ1_OUT_IMG */
	DIP_X_REG_SRZ1_HORI_STEP                        DIP_X_SRZ1_HORI_STEP;                            /* 390C, 0x1502590C, DIP_A_SRZ1_HORI_STEP */
	DIP_X_REG_SRZ1_VERT_STEP                        DIP_X_SRZ1_VERT_STEP;                            /* 3910, 0x15025910, DIP_A_SRZ1_VERT_STEP */
	DIP_X_REG_SRZ1_HORI_INT_OFST                    DIP_X_SRZ1_HORI_INT_OFST;                        /* 3914, 0x15025914, DIP_A_SRZ1_HORI_INT_OFST */
	DIP_X_REG_SRZ1_HORI_SUB_OFST                    DIP_X_SRZ1_HORI_SUB_OFST;                        /* 3918, 0x15025918, DIP_A_SRZ1_HORI_SUB_OFST */
	DIP_X_REG_SRZ1_VERT_INT_OFST                    DIP_X_SRZ1_VERT_INT_OFST;                        /* 391C, 0x1502591C, DIP_A_SRZ1_VERT_INT_OFST */
	DIP_X_REG_SRZ1_VERT_SUB_OFST                    DIP_X_SRZ1_VERT_SUB_OFST;                        /* 3920, 0x15025920, DIP_A_SRZ1_VERT_SUB_OFST */
	UINT32                                          rsv_3924[7];                                     /* 3924..393F, 0x15025924..1502593F */
	DIP_X_REG_FE_CTRL1                              DIP_X_FE_CTRL1;                                  /* 3940, 0x15025940, DIP_A_FE_CTRL1 */
	DIP_X_REG_FE_IDX_CTRL                           DIP_X_FE_IDX_CTRL;                               /* 3944, 0x15025944, DIP_A_FE_IDX_CTRL */
	DIP_X_REG_FE_CROP_CTRL1                         DIP_X_FE_CROP_CTRL1;                             /* 3948, 0x15025948, DIP_A_FE_CROP_CTRL1 */
	DIP_X_REG_FE_CROP_CTRL2                         DIP_X_FE_CROP_CTRL2;                             /* 394C, 0x1502594C, DIP_A_FE_CROP_CTRL2 */
	DIP_X_REG_FE_CTRL2                              DIP_X_FE_CTRL2;                                  /* 3950, 0x15025950, DIP_A_FE_CTRL2 */
	UINT32                                          rsv_3954[427];                                   /* 3954..3FFF, 0x15025954..15025FFF */
	DIP_X_REG_PCA_TBL                               DIP_X_PCA_TBL[360];                              /* 4000..459F, 0x15026000..1502659F, DIP_A_PCA_TBL */
	UINT32                                          rsv_45A0[24];                                    /* 45A0..45FF, 0x150265A0..150265FF */
	DIP_X_REG_PCA_CON1                              DIP_X_PCA_CON1;                                  /* 4600, 0x15026600, DIP_A_PCA_CON1 */
	DIP_X_REG_PCA_CON2                              DIP_X_PCA_CON2;                                  /* 4604, 0x15026604, DIP_A_PCA_CON2 */
	DIP_X_REG_PCA_CON3                              DIP_X_PCA_CON3;                                  /* 4608, 0x15026608, DIP_A_PCA_CON3 */
	DIP_X_REG_PCA_CON4                              DIP_X_PCA_CON4;                                  /* 460C, 0x1502660C, DIP_A_PCA_CON4 */
	DIP_X_REG_PCA_CON5                              DIP_X_PCA_CON5;                                  /* 4610, 0x15026610, DIP_A_PCA_CON5 */
	DIP_X_REG_PCA_CON6                              DIP_X_PCA_CON6;                                  /* 4614, 0x15026614, DIP_A_PCA_CON6 */
	DIP_X_REG_PCA_CON7                              DIP_X_PCA_CON7;                                  /* 4618, 0x15026618, DIP_A_PCA_CON7 */
	UINT32                                          rsv_461C[9];                                     /* 461C..463F, 0x1502661C..1502663F */
	DIP_X_REG_MIX2_CTRL_0                           DIP_X_MIX2_CTRL_0;                               /* 4640, 0x15026640, DIP_A_MIX2_CTRL_0 */
	DIP_X_REG_MIX2_CTRL_1                           DIP_X_MIX2_CTRL_1;                               /* 4644, 0x15026644, DIP_A_MIX2_CTRL_1 */
	DIP_X_REG_MIX2_SPARE                            DIP_X_MIX2_SPARE;                                /* 4648, 0x15026648, DIP_A_MIX2_SPARE */
	UINT32                                          rsv_464C[13];                                    /* 464C..467F, 0x1502664C..1502667F */
	DIP_X_REG_SRZ2_CONTROL                          DIP_X_SRZ2_CONTROL;                              /* 4680, 0x15026680, DIP_A_SRZ2_CONTROL */
	DIP_X_REG_SRZ2_IN_IMG                           DIP_X_SRZ2_IN_IMG;                               /* 4684, 0x15026684, DIP_A_SRZ2_IN_IMG */
	DIP_X_REG_SRZ2_OUT_IMG                          DIP_X_SRZ2_OUT_IMG;                              /* 4688, 0x15026688, DIP_A_SRZ2_OUT_IMG */
	DIP_X_REG_SRZ2_HORI_STEP                        DIP_X_SRZ2_HORI_STEP;                            /* 468C, 0x1502668C, DIP_A_SRZ2_HORI_STEP */
	DIP_X_REG_SRZ2_VERT_STEP                        DIP_X_SRZ2_VERT_STEP;                            /* 4690, 0x15026690, DIP_A_SRZ2_VERT_STEP */
	DIP_X_REG_SRZ2_HORI_INT_OFST                    DIP_X_SRZ2_HORI_INT_OFST;                        /* 4694, 0x15026694, DIP_A_SRZ2_HORI_INT_OFST */
	DIP_X_REG_SRZ2_HORI_SUB_OFST                    DIP_X_SRZ2_HORI_SUB_OFST;                        /* 4698, 0x15026698, DIP_A_SRZ2_HORI_SUB_OFST */
	DIP_X_REG_SRZ2_VERT_INT_OFST                    DIP_X_SRZ2_VERT_INT_OFST;                        /* 469C, 0x1502669C, DIP_A_SRZ2_VERT_INT_OFST */
	DIP_X_REG_SRZ2_VERT_SUB_OFST                    DIP_X_SRZ2_VERT_SUB_OFST;                        /* 46A0, 0x150266A0, DIP_A_SRZ2_VERT_SUB_OFST */
	UINT32                                          rsv_46A4[7];                                     /* 46A4..46BF, 0x150266A4..150266BF */
	DIP_X_REG_SEEE_CTRL                             DIP_X_SEEE_CTRL;                                 /* 46C0, 0x150266C0, DIP_A_SEEE_CTRL */
	DIP_X_REG_SEEE_TOP_CTRL                         DIP_X_SEEE_TOP_CTRL;                             /* 46C4, 0x150266C4, DIP_A_SEEE_TOP_CTRL */
	DIP_X_REG_SEEE_BLND_CTRL_1                      DIP_X_SEEE_BLND_CTRL_1;                          /* 46C8, 0x150266C8, DIP_A_SEEE_BLND_CTRL_1 */
	DIP_X_REG_SEEE_BLND_CTRL_2                      DIP_X_SEEE_BLND_CTRL_2;                          /* 46CC, 0x150266CC, DIP_A_SEEE_BLND_CTRL_2 */
	DIP_X_REG_SEEE_CORE_CTRL                        DIP_X_SEEE_CORE_CTRL;                            /* 46D0, 0x150266D0, DIP_A_SEEE_CORE_CTRL */
	DIP_X_REG_SEEE_GN_CTRL_1                        DIP_X_SEEE_GN_CTRL_1;                            /* 46D4, 0x150266D4, DIP_A_SEEE_GN_CTRL_1 */
	DIP_X_REG_SEEE_LUMA_CTRL_1                      DIP_X_SEEE_LUMA_CTRL_1;                          /* 46D8, 0x150266D8, DIP_A_SEEE_LUMA_CTRL_1 */
	DIP_X_REG_SEEE_LUMA_CTRL_2                      DIP_X_SEEE_LUMA_CTRL_2;                          /* 46DC, 0x150266DC, DIP_A_SEEE_LUMA_CTRL_2 */
	DIP_X_REG_SEEE_LUMA_SLNK_CTRL                   DIP_X_SEEE_LUMA_SLNK_CTRL;                       /* 46E0, 0x150266E0, DIP_A_SEEE_LUMA_SLNK_CTRL */
	DIP_X_REG_SEEE_GLUT_CTRL_1                      DIP_X_SEEE_GLUT_CTRL_1;                          /* 46E4, 0x150266E4, DIP_A_SEEE_GLUT_CTRL_1 */
	DIP_X_REG_SEEE_GLUT_CTRL_2                      DIP_X_SEEE_GLUT_CTRL_2;                          /* 46E8, 0x150266E8, DIP_A_SEEE_GLUT_CTRL_2 */
	DIP_X_REG_SEEE_GLUT_CTRL_3                      DIP_X_SEEE_GLUT_CTRL_3;                          /* 46EC, 0x150266EC, DIP_A_SEEE_GLUT_CTRL_3 */
	DIP_X_REG_SEEE_GLUT_CTRL_4                      DIP_X_SEEE_GLUT_CTRL_4;                          /* 46F0, 0x150266F0, DIP_A_SEEE_GLUT_CTRL_4 */
	DIP_X_REG_SEEE_GLUT_CTRL_5                      DIP_X_SEEE_GLUT_CTRL_5;                          /* 46F4, 0x150266F4, DIP_A_SEEE_GLUT_CTRL_5 */
	DIP_X_REG_SEEE_GLUT_CTRL_6                      DIP_X_SEEE_GLUT_CTRL_6;                          /* 46F8, 0x150266F8, DIP_A_SEEE_GLUT_CTRL_6 */
	DIP_X_REG_SEEE_ARTIFACT_CTRL                    DIP_X_SEEE_ARTIFACT_CTRL;                        /* 46FC, 0x150266FC, DIP_A_SEEE_ARTIFACT_CTRL */
	DIP_X_REG_SEEE_CLIP_CTRL                        DIP_X_SEEE_CLIP_CTRL;                            /* 4700, 0x15026700, DIP_A_SEEE_CLIP_CTRL */
	DIP_X_REG_SEEE_GN_CTRL_2                        DIP_X_SEEE_GN_CTRL_2;                            /* 4704, 0x15026704, DIP_A_SEEE_GN_CTRL_2 */
	DIP_X_REG_SEEE_ST_CTRL_1                        DIP_X_SEEE_ST_CTRL_1;                            /* 4708, 0x15026708, DIP_A_SEEE_ST_CTRL_1 */
	DIP_X_REG_SEEE_ST_CTRL_2                        DIP_X_SEEE_ST_CTRL_2;                            /* 470C, 0x1502670C, DIP_A_SEEE_ST_CTRL_2 */
	DIP_X_REG_SEEE_CT_CTRL                          DIP_X_SEEE_CT_CTRL;                              /* 4710, 0x15026710, DIP_A_SEEE_CT_CTRL */
	DIP_X_REG_SEEE_CBOOST_CTRL_1                    DIP_X_SEEE_CBOOST_CTRL_1;                        /* 4714, 0x15026714, DIP_A_SEEE_CBOOST_CTRL_1 */
	DIP_X_REG_SEEE_CBOOST_CTRL_2                    DIP_X_SEEE_CBOOST_CTRL_2;                        /* 4718, 0x15026718, DIP_A_SEEE_CBOOST_CTRL_2 */
	DIP_X_REG_SEEE_PBC1_CTRL_1                      DIP_X_SEEE_PBC1_CTRL_1;                          /* 471C, 0x1502671C, DIP_A_SEEE_PBC1_CTRL_1 */
	DIP_X_REG_SEEE_PBC1_CTRL_2                      DIP_X_SEEE_PBC1_CTRL_2;                          /* 4720, 0x15026720, DIP_A_SEEE_PBC1_CTRL_2 */
	DIP_X_REG_SEEE_PBC1_CTRL_3                      DIP_X_SEEE_PBC1_CTRL_3;                          /* 4724, 0x15026724, DIP_A_SEEE_PBC1_CTRL_3 */
	DIP_X_REG_SEEE_PBC2_CTRL_1                      DIP_X_SEEE_PBC2_CTRL_1;                          /* 4728, 0x15026728, DIP_A_SEEE_PBC2_CTRL_1 */
	DIP_X_REG_SEEE_PBC2_CTRL_2                      DIP_X_SEEE_PBC2_CTRL_2;                          /* 472C, 0x1502672C, DIP_A_SEEE_PBC2_CTRL_2 */
	DIP_X_REG_SEEE_PBC2_CTRL_3                      DIP_X_SEEE_PBC2_CTRL_3;                          /* 4730, 0x15026730, DIP_A_SEEE_PBC2_CTRL_3 */
	DIP_X_REG_SEEE_PBC3_CTRL_1                      DIP_X_SEEE_PBC3_CTRL_1;                          /* 4734, 0x15026734, DIP_A_SEEE_PBC3_CTRL_1 */
	DIP_X_REG_SEEE_PBC3_CTRL_2                      DIP_X_SEEE_PBC3_CTRL_2;                          /* 4738, 0x15026738, DIP_A_SEEE_PBC3_CTRL_2 */
	DIP_X_REG_SEEE_PBC3_CTRL_3                      DIP_X_SEEE_PBC3_CTRL_3;                          /* 473C, 0x1502673C, DIP_A_SEEE_PBC3_CTRL_3 */
	DIP_X_REG_SEEE_SE_Y_SPECL_CTRL                  DIP_X_SEEE_SE_Y_SPECL_CTRL;                      /* 4740, 0x15026740, DIP_A_SEEE_SE_Y_SPECL_CTRL */
	DIP_X_REG_SEEE_SE_EDGE_CTRL_1                   DIP_X_SEEE_SE_EDGE_CTRL_1;                       /* 4744, 0x15026744, DIP_A_SEEE_SE_EDGE_CTRL_1 */
	DIP_X_REG_SEEE_SE_EDGE_CTRL_2                   DIP_X_SEEE_SE_EDGE_CTRL_2;                       /* 4748, 0x15026748, DIP_A_SEEE_SE_EDGE_CTRL_2 */
	DIP_X_REG_SEEE_SE_CORE_CTRL_1                   DIP_X_SEEE_SE_CORE_CTRL_1;                       /* 474C, 0x1502674C, DIP_A_SEEE_SE_CORE_CTRL_1 */
	DIP_X_REG_SEEE_SE_CORE_CTRL_2                   DIP_X_SEEE_SE_CORE_CTRL_2;                       /* 4750, 0x15026750, DIP_A_SEEE_SE_CORE_CTRL_2 */
	UINT32                                          rsv_4754[11];                                    /* 4754..477F, 0x15026754..1502677F */
	DIP_X_REG_SL2D_CEN                              DIP_X_SL2D_CEN;                                  /* 4780, 0x15026780, DIP_A_SL2D_CEN */
	DIP_X_REG_SL2D_RR_CON0                          DIP_X_SL2D_RR_CON0;                              /* 4784, 0x15026784, DIP_A_SL2D_RR_CON0 */
	DIP_X_REG_SL2D_RR_CON1                          DIP_X_SL2D_RR_CON1;                              /* 4788, 0x15026788, DIP_A_SL2D_RR_CON1 */
	DIP_X_REG_SL2D_GAIN                             DIP_X_SL2D_GAIN;                                 /* 478C, 0x1502678C, DIP_A_SL2D_GAIN */
	DIP_X_REG_SL2D_RZ                               DIP_X_SL2D_RZ;                                   /* 4790, 0x15026790, DIP_A_SL2D_RZ */
	DIP_X_REG_SL2D_XOFF                             DIP_X_SL2D_XOFF;                                 /* 4794, 0x15026794, DIP_A_SL2D_XOFF */
	DIP_X_REG_SL2D_YOFF                             DIP_X_SL2D_YOFF;                                 /* 4798, 0x15026798, DIP_A_SL2D_YOFF */
	DIP_X_REG_SL2D_SLP_CON0                         DIP_X_SL2D_SLP_CON0;                             /* 479C, 0x1502679C, DIP_A_SL2D_SLP_CON0 */
	DIP_X_REG_SL2D_SLP_CON1                         DIP_X_SL2D_SLP_CON1;                             /* 47A0, 0x150267A0, DIP_A_SL2D_SLP_CON1 */
	DIP_X_REG_SL2D_SLP_CON2                         DIP_X_SL2D_SLP_CON2;                             /* 47A4, 0x150267A4, DIP_A_SL2D_SLP_CON2 */
	DIP_X_REG_SL2D_SLP_CON3                         DIP_X_SL2D_SLP_CON3;                             /* 47A8, 0x150267A8, DIP_A_SL2D_SLP_CON3 */
	DIP_X_REG_SL2D_SIZE                             DIP_X_SL2D_SIZE;                                 /* 47AC, 0x150267AC, DIP_A_SL2D_SIZE */
	UINT32                                          rsv_47B0[4];                                     /* 47B0..47BF, 0x150267B0..150267BF */
	DIP_X_REG_SMX2_CTL                              DIP_X_SMX2_CTL;                                  /* 47C0, 0x150267C0, DIP_A_SMX2_CTL */
	DIP_X_REG_SMX2_TRANS_CON                        DIP_X_SMX2_TRANS_CON;                            /* 47C4, 0x150267C4, DIP_A_SMX2_TRANS_CON */
	DIP_X_REG_SMX2_SPARE                            DIP_X_SMX2_SPARE;                                /* 47C8, 0x150267C8, DIP_A_SMX2_SPARE */
	DIP_X_REG_SMX2_CRPINL_CON1                      DIP_X_SMX2_CRPINL_CON1;                          /* 47CC, 0x150267CC, DIP_A_SMX2_CRPINL_CON1 */
	DIP_X_REG_SMX2_CRPINL_CON2                      DIP_X_SMX2_CRPINL_CON2;                          /* 47D0, 0x150267D0, DIP_A_SMX2_CRPINL_CON2 */
	DIP_X_REG_SMX2_CRPINR_CON1                      DIP_X_SMX2_CRPINR_CON1;                          /* 47D4, 0x150267D4, DIP_A_SMX2_CRPINR_CON1 */
	DIP_X_REG_SMX2_CRPINR_CON2                      DIP_X_SMX2_CRPINR_CON2;                          /* 47D8, 0x150267D8, DIP_A_SMX2_CRPINR_CON2 */
	DIP_X_REG_SMX2_CRPOUT_CON1                      DIP_X_SMX2_CRPOUT_CON1;                          /* 47DC, 0x150267DC, DIP_A_SMX2_CRPOUT_CON1 */
	DIP_X_REG_SMX2_CRPOUT_CON2                      DIP_X_SMX2_CRPOUT_CON2;                          /* 47E0, 0x150267E0, DIP_A_SMX2_CRPOUT_CON2 */
	UINT32                                          rsv_47E4[7];                                     /* 47E4..47FF, 0x150267E4..150267FF */
	DIP_X_REG_NBC2_ANR2_CON1                        DIP_X_NBC2_ANR2_CON1;                            /* 4800, 0x15026800, DIP_A_NBC2_ANR2_CON1 */
	DIP_X_REG_NBC2_ANR2_CON2                        DIP_X_NBC2_ANR2_CON2;                            /* 4804, 0x15026804, DIP_A_NBC2_ANR2_CON2 */
	DIP_X_REG_NBC2_ANR2_YAD1                        DIP_X_NBC2_ANR2_YAD1;                            /* 4808, 0x15026808, DIP_A_NBC2_ANR2_YAD1 */
	UINT32                                          rsv_480C;                                        /* 480C, 0x1502680C */
	DIP_X_REG_NBC2_ANR2_Y4LUT1                      DIP_X_NBC2_ANR2_Y4LUT1;                          /* 4810, 0x15026810, DIP_A_NBC2_ANR2_Y4LUT1 */
	DIP_X_REG_NBC2_ANR2_Y4LUT2                      DIP_X_NBC2_ANR2_Y4LUT2;                          /* 4814, 0x15026814, DIP_A_NBC2_ANR2_Y4LUT2 */
	DIP_X_REG_NBC2_ANR2_Y4LUT3                      DIP_X_NBC2_ANR2_Y4LUT3;                          /* 4818, 0x15026818, DIP_A_NBC2_ANR2_Y4LUT3 */
	UINT32                                          rsv_481C[5];                                     /* 481C..482F, 0x1502681C..1502682F */
	DIP_X_REG_NBC2_ANR2_L4LUT1                      DIP_X_NBC2_ANR2_L4LUT1;                          /* 4830, 0x15026830, DIP_A_NBC2_ANR2_L4LUT1 */
	DIP_X_REG_NBC2_ANR2_L4LUT2                      DIP_X_NBC2_ANR2_L4LUT2;                          /* 4834, 0x15026834, DIP_A_NBC2_ANR2_L4LUT2 */
	DIP_X_REG_NBC2_ANR2_L4LUT3                      DIP_X_NBC2_ANR2_L4LUT3;                          /* 4838, 0x15026838, DIP_A_NBC2_ANR2_L4LUT3 */
	UINT32                                          rsv_483C;                                        /* 483C, 0x1502683C */
	DIP_X_REG_NBC2_ANR2_CAD                         DIP_X_NBC2_ANR2_CAD;                             /* 4840, 0x15026840, DIP_A_NBC2_ANR2_CAD */
	DIP_X_REG_NBC2_ANR2_PTC                         DIP_X_NBC2_ANR2_PTC;                             /* 4844, 0x15026844, DIP_A_NBC2_ANR2_PTC */
	DIP_X_REG_NBC2_ANR2_SL2                         DIP_X_NBC2_ANR2_SL2;                             /* 4848, 0x15026848, DIP_A_NBC2_ANR2_SL2 */
	DIP_X_REG_NBC2_ANR2_MED1                        DIP_X_NBC2_ANR2_MED1;                            /* 484C, 0x1502684C, DIP_A_NBC2_ANR2_MED1 */
	DIP_X_REG_NBC2_ANR2_MED2                        DIP_X_NBC2_ANR2_MED2;                            /* 4850, 0x15026850, DIP_A_NBC2_ANR2_MED2 */
	DIP_X_REG_NBC2_ANR2_MED3                        DIP_X_NBC2_ANR2_MED3;                            /* 4854, 0x15026854, DIP_A_NBC2_ANR2_MED3 */
	DIP_X_REG_NBC2_ANR2_MED4                        DIP_X_NBC2_ANR2_MED4;                            /* 4858, 0x15026858, DIP_A_NBC2_ANR2_MED4 */
	DIP_X_REG_NBC2_ANR2_MED5                        DIP_X_NBC2_ANR2_MED5;                            /* 485C, 0x1502685C, DIP_A_NBC2_ANR2_MED5 */
	UINT32                                          rsv_4860[5];                                     /* 4860..4873, 0x15026860..15026873 */
	DIP_X_REG_NBC2_ANR2_ACTC                        DIP_X_NBC2_ANR2_ACTC;                            /* 4874, 0x15026874, DIP_A_NBC2_ANR2_ACTC */
	DIP_X_REG_NBC2_ANR2_RSV1                        DIP_X_NBC2_ANR2_RSV1;                            /* 4878, 0x15026878, DIP_A_NBC2_ANR2_RSV1 */
	DIP_X_REG_NBC2_ANR2_RSV2                        DIP_X_NBC2_ANR2_RSV2;                            /* 487C, 0x1502687C, DIP_A_NBC2_ANR2_RSV2 */
	DIP_X_REG_NBC2_CCR_CON                          DIP_X_NBC2_CCR_CON;                              /* 4880, 0x15026880, DIP_A_NBC2_CCR_CON */
	DIP_X_REG_NBC2_CCR_YLUT                         DIP_X_NBC2_CCR_YLUT;                             /* 4884, 0x15026884, DIP_A_NBC2_CCR_YLUT */
	DIP_X_REG_NBC2_CCR_UVLUT                        DIP_X_NBC2_CCR_UVLUT;                            /* 4888, 0x15026888, DIP_A_NBC2_CCR_UVLUT */
	DIP_X_REG_NBC2_CCR_YLUT2                        DIP_X_NBC2_CCR_YLUT2;                            /* 488C, 0x1502688C, DIP_A_NBC2_CCR_YLUT2 */
	DIP_X_REG_NBC2_CCR_SAT_CTRL                     DIP_X_NBC2_CCR_SAT_CTRL;                         /* 4890, 0x15026890, DIP_A_NBC2_CCR_SAT_CTRL */
	DIP_X_REG_NBC2_CCR_UVLUT_SP                     DIP_X_NBC2_CCR_UVLUT_SP;                         /* 4894, 0x15026894, DIP_A_NBC2_CCR_UVLUT_SP */
	DIP_X_REG_NBC2_CCR_HUE1                         DIP_X_NBC2_CCR_HUE1;                             /* 4898, 0x15026898, DIP_A_NBC2_CCR_HUE1 */
	DIP_X_REG_NBC2_CCR_HUE2                         DIP_X_NBC2_CCR_HUE2;                             /* 489C, 0x1502689C, DIP_A_NBC2_CCR_HUE2 */
	DIP_X_REG_NBC2_CCR_HUE3                         DIP_X_NBC2_CCR_HUE3;                             /* 48A0, 0x150268A0, DIP_A_NBC2_CCR_HUE3 */
	DIP_X_REG_NBC2_CCR_L4LUT1                       DIP_X_NBC2_CCR_L4LUT1;                           /* 48A4, 0x150268A4, DIP_A_NBC2_CCR_L4LUT1 */
	DIP_X_REG_NBC2_CCR_L4LUT2                       DIP_X_NBC2_CCR_L4LUT2;                           /* 48A8, 0x150268A8, DIP_A_NBC2_CCR_L4LUT2 */
	DIP_X_REG_NBC2_CCR_L4LUT3                       DIP_X_NBC2_CCR_L4LUT3;                           /* 48AC, 0x150268AC, DIP_A_NBC2_CCR_L4LUT3 */
	DIP_X_REG_NBC2_BOK_CON                          DIP_X_NBC2_BOK_CON;                              /* 48B0, 0x150268B0, DIP_A_NBC2_BOK_CON */
	DIP_X_REG_NBC2_BOK_TUN                          DIP_X_NBC2_BOK_TUN;                              /* 48B4, 0x150268B4, DIP_A_NBC2_BOK_TUN */
	DIP_X_REG_NBC2_BOK_OFF                          DIP_X_NBC2_BOK_OFF;                              /* 48B8, 0x150268B8, DIP_A_NBC2_BOK_OFF */
	DIP_X_REG_NBC2_BOK_RSV1                         DIP_X_NBC2_BOK_RSV1;                             /* 48BC, 0x150268BC, DIP_A_NBC2_BOK_RSV1 */
	DIP_X_REG_NBC2_ABF_CON1                         DIP_X_NBC2_ABF_CON1;                             /* 48C0, 0x150268C0, DIP_A_NBC2_ABF_CON1 */
	DIP_X_REG_NBC2_ABF_CON2                         DIP_X_NBC2_ABF_CON2;                             /* 48C4, 0x150268C4, DIP_A_NBC2_ABF_CON2 */
	DIP_X_REG_NBC2_ABF_RCON                         DIP_X_NBC2_ABF_RCON;                             /* 48C8, 0x150268C8, DIP_A_NBC2_ABF_RCON */
	DIP_X_REG_NBC2_ABF_YLUT                         DIP_X_NBC2_ABF_YLUT;                             /* 48CC, 0x150268CC, DIP_A_NBC2_ABF_YLUT */
	DIP_X_REG_NBC2_ABF_CXLUT                        DIP_X_NBC2_ABF_CXLUT;                            /* 48D0, 0x150268D0, DIP_A_NBC2_ABF_CXLUT */
	DIP_X_REG_NBC2_ABF_CYLUT                        DIP_X_NBC2_ABF_CYLUT;                            /* 48D4, 0x150268D4, DIP_A_NBC2_ABF_CYLUT */
	DIP_X_REG_NBC2_ABF_YSP                          DIP_X_NBC2_ABF_YSP;                              /* 48D8, 0x150268D8, DIP_A_NBC2_ABF_YSP */
	DIP_X_REG_NBC2_ABF_CXSP                         DIP_X_NBC2_ABF_CXSP;                             /* 48DC, 0x150268DC, DIP_A_NBC2_ABF_CXSP */
	DIP_X_REG_NBC2_ABF_CYSP                         DIP_X_NBC2_ABF_CYSP;                             /* 48E0, 0x150268E0, DIP_A_NBC2_ABF_CYSP */
	DIP_X_REG_NBC2_ABF_CLP                          DIP_X_NBC2_ABF_CLP;                              /* 48E4, 0x150268E4, DIP_A_NBC2_ABF_CLP */
	UINT32                                          rsv_48E8;                                        /* 48E8, 0x150268E8 */
	DIP_X_REG_NBC2_ABF_RSV1                         DIP_X_NBC2_ABF_RSV1;                             /* 48EC, 0x150268EC, DIP_A_NBC2_ABF_RSV1 */
	UINT32                                          rsv_48F0[4];                                     /* 48F0..48FF, 0x150268F0..150268FF */
	DIP_X_REG_SL2C_CEN                              DIP_X_SL2C_CEN;                                  /* 4900, 0x15026900, DIP_A_SL2C_CEN */
	DIP_X_REG_SL2C_RR_CON0                          DIP_X_SL2C_RR_CON0;                              /* 4904, 0x15026904, DIP_A_SL2C_RR_CON0 */
	DIP_X_REG_SL2C_RR_CON1                          DIP_X_SL2C_RR_CON1;                              /* 4908, 0x15026908, DIP_A_SL2C_RR_CON1 */
	DIP_X_REG_SL2C_GAIN                             DIP_X_SL2C_GAIN;                                 /* 490C, 0x1502690C, DIP_A_SL2C_GAIN */
	DIP_X_REG_SL2C_RZ                               DIP_X_SL2C_RZ;                                   /* 4910, 0x15026910, DIP_A_SL2C_RZ */
	DIP_X_REG_SL2C_XOFF                             DIP_X_SL2C_XOFF;                                 /* 4914, 0x15026914, DIP_A_SL2C_XOFF */
	DIP_X_REG_SL2C_YOFF                             DIP_X_SL2C_YOFF;                                 /* 4918, 0x15026918, DIP_A_SL2C_YOFF */
	DIP_X_REG_SL2C_SLP_CON0                         DIP_X_SL2C_SLP_CON0;                             /* 491C, 0x1502691C, DIP_A_SL2C_SLP_CON0 */
	DIP_X_REG_SL2C_SLP_CON1                         DIP_X_SL2C_SLP_CON1;                             /* 4920, 0x15026920, DIP_A_SL2C_SLP_CON1 */
	DIP_X_REG_SL2C_SLP_CON2                         DIP_X_SL2C_SLP_CON2;                             /* 4924, 0x15026924, DIP_A_SL2C_SLP_CON2 */
	DIP_X_REG_SL2C_SLP_CON3                         DIP_X_SL2C_SLP_CON3;                             /* 4928, 0x15026928, DIP_A_SL2C_SLP_CON3 */
	DIP_X_REG_SL2C_SIZE                             DIP_X_SL2C_SIZE;                                 /* 492C, 0x1502692C, DIP_A_SL2C_SIZE */
	UINT32                                          rsv_4930[4];                                     /* 4930..493F, 0x15026930..1502693F */
	DIP_X_REG_SRZ3_CONTROL                          DIP_X_SRZ3_CONTROL;                              /* 4940, 0x15026940, DIP_A_SRZ3_CONTROL */
	DIP_X_REG_SRZ3_IN_IMG                           DIP_X_SRZ3_IN_IMG;                               /* 4944, 0x15026944, DIP_A_SRZ3_IN_IMG */
	DIP_X_REG_SRZ3_OUT_IMG                          DIP_X_SRZ3_OUT_IMG;                              /* 4948, 0x15026948, DIP_A_SRZ3_OUT_IMG */
	DIP_X_REG_SRZ3_HORI_STEP                        DIP_X_SRZ3_HORI_STEP;                            /* 494C, 0x1502694C, DIP_A_SRZ3_HORI_STEP */
	DIP_X_REG_SRZ3_VERT_STEP                        DIP_X_SRZ3_VERT_STEP;                            /* 4950, 0x15026950, DIP_A_SRZ3_VERT_STEP */
	DIP_X_REG_SRZ3_HORI_INT_OFST                    DIP_X_SRZ3_HORI_INT_OFST;                        /* 4954, 0x15026954, DIP_A_SRZ3_HORI_INT_OFST */
	DIP_X_REG_SRZ3_HORI_SUB_OFST                    DIP_X_SRZ3_HORI_SUB_OFST;                        /* 4958, 0x15026958, DIP_A_SRZ3_HORI_SUB_OFST */
	DIP_X_REG_SRZ3_VERT_INT_OFST                    DIP_X_SRZ3_VERT_INT_OFST;                        /* 495C, 0x1502695C, DIP_A_SRZ3_VERT_INT_OFST */
	DIP_X_REG_SRZ3_VERT_SUB_OFST                    DIP_X_SRZ3_VERT_SUB_OFST;                        /* 4960, 0x15026960, DIP_A_SRZ3_VERT_SUB_OFST */
	UINT32                                          rsv_4964[7];                                     /* 4964..497F, 0x15026964..1502697F */
	DIP_X_REG_NDG2_RAN_0                            DIP_X_NDG2_RAN_0;                                /* 4980, 0x15026980, DIP_A_NDG2_RAN_0 */
	DIP_X_REG_NDG2_RAN_1                            DIP_X_NDG2_RAN_1;                                /* 4984, 0x15026984, DIP_A_NDG2_RAN_1 */
	DIP_X_REG_NDG2_RAN_2                            DIP_X_NDG2_RAN_2;                                /* 4988, 0x15026988, DIP_A_NDG2_RAN_2 */
	DIP_X_REG_NDG2_RAN_3                            DIP_X_NDG2_RAN_3;                                /* 498C, 0x1502698C, DIP_A_NDG2_RAN_3 */
	DIP_X_REG_NDG2_CROP_X                           DIP_X_NDG2_CROP_X;                               /* 4990, 0x15026990, DIP_A_NDG2_CROP_X */
	DIP_X_REG_NDG2_CROP_Y                           DIP_X_NDG2_CROP_Y;                               /* 4994, 0x15026994, DIP_A_NDG2_CROP_Y */
	UINT32                                          rsv_4998[10];                                    /* 4998..49BF, 0x15026998..150269BF */
	DIP_X_REG_SMX3_CTL                              DIP_X_SMX3_CTL;                                  /* 49C0, 0x150269C0, DIP_A_SMX3_CTL */
	DIP_X_REG_SMX3_TRANS_CON                        DIP_X_SMX3_TRANS_CON;                            /* 49C4, 0x150269C4, DIP_A_SMX3_TRANS_CON */
	DIP_X_REG_SMX3_SPARE                            DIP_X_SMX3_SPARE;                                /* 49C8, 0x150269C8, DIP_A_SMX3_SPARE */
	DIP_X_REG_SMX3_CRPINL_CON1                      DIP_X_SMX3_CRPINL_CON1;                          /* 49CC, 0x150269CC, DIP_A_SMX3_CRPINL_CON1 */
	DIP_X_REG_SMX3_CRPINL_CON2                      DIP_X_SMX3_CRPINL_CON2;                          /* 49D0, 0x150269D0, DIP_A_SMX3_CRPINL_CON2 */
	DIP_X_REG_SMX3_CRPINR_CON1                      DIP_X_SMX3_CRPINR_CON1;                          /* 49D4, 0x150269D4, DIP_A_SMX3_CRPINR_CON1 */
	DIP_X_REG_SMX3_CRPINR_CON2                      DIP_X_SMX3_CRPINR_CON2;                          /* 49D8, 0x150269D8, DIP_A_SMX3_CRPINR_CON2 */
	DIP_X_REG_SMX3_CRPOUT_CON1                      DIP_X_SMX3_CRPOUT_CON1;                          /* 49DC, 0x150269DC, DIP_A_SMX3_CRPOUT_CON1 */
	DIP_X_REG_SMX3_CRPOUT_CON2                      DIP_X_SMX3_CRPOUT_CON2;                          /* 49E0, 0x150269E0, DIP_A_SMX3_CRPOUT_CON2 */
	UINT32                                          rsv_49E4[7];                                     /* 49E4..49FF, 0x150269E4..150269FF */
	DIP_X_REG_FM_SIZE                               DIP_X_FM_SIZE;                                   /* 4A00, 0x15026A00, DIP_A_FM_SIZE */
	DIP_X_REG_FM_TH_CON0                            DIP_X_FM_TH_CON0;                                /* 4A04, 0x15026A04, DIP_A_FM_TH_CON0 */
	DIP_X_REG_FM_ACC_RES                            DIP_X_FM_ACC_RES;                                /* 4A08, 0x15026A08, DIP_A_FM_ACC_RES */
	UINT32                                          rsv_4A0C[381];                                   /* 4A0C..4FFF, 0x15026A0C..15026FFF */
	DIP_X_REG_CAM_COLOR_CFG_MAIN                    DIP_X_CAM_COLOR_CFG_MAIN;                        /* 5000, 0x15027000, DIP_A_CAM_COLOR_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_PXL_CNT_MAIN                DIP_X_CAM_COLOR_PXL_CNT_MAIN;                    /* 5004, 0x15027004, DIP_A_CAM_COLOR_PXL_CNT_MAIN */
	DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN               DIP_X_CAM_COLOR_LINE_CNT_MAIN;                   /* 5008, 0x15027008, DIP_A_CAM_COLOR_LINE_CNT_MAIN */
	DIP_X_REG_CAM_COLOR_WIN_X_MAIN                  DIP_X_CAM_COLOR_WIN_X_MAIN;                      /* 500C, 0x1502700C, DIP_A_CAM_COLOR_WIN_X_MAIN */
	DIP_X_REG_CAM_COLOR_WIN_Y_MAIN                  DIP_X_CAM_COLOR_WIN_Y_MAIN;                      /* 5010, 0x15027010, DIP_A_CAM_COLOR_WIN_Y_MAIN */
	DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0          DIP_X_CAM_COLOR_TIMING_DETECTION_0;              /* 5014, 0x15027014, DIP_A_CAM_COLOR_TIMING_DETECTION_0 */
	DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1          DIP_X_CAM_COLOR_TIMING_DETECTION_1;              /* 5018, 0x15027018, DIP_A_CAM_COLOR_TIMING_DETECTION_1 */
	DIP_X_REG_CAM_COLOR_DBG_CFG_MAIN                DIP_X_CAM_COLOR_DBG_CFG_MAIN;                    /* 501C, 0x1502701C, DIP_A_CAM_COLOR_DBG_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_C_BOOST_MAIN                DIP_X_CAM_COLOR_C_BOOST_MAIN;                    /* 5020, 0x15027020, DIP_A_CAM_COLOR_C_BOOST_MAIN */
	DIP_X_REG_CAM_COLOR_C_BOOST_MAIN_2              DIP_X_CAM_COLOR_C_BOOST_MAIN_2;                  /* 5024, 0x15027024, DIP_A_CAM_COLOR_C_BOOST_MAIN_2 */
	DIP_X_REG_CAM_COLOR_LUMA_ADJ                    DIP_X_CAM_COLOR_LUMA_ADJ;                        /* 5028, 0x15027028, DIP_A_CAM_COLOR_LUMA_ADJ */
	DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_1            DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1;                /* 502C, 0x1502702C, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_1 */
	DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_2            DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2;                /* 5030, 0x15027030, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_2 */
	DIP_X_REG_CAM_COLOR_POS_MAIN                    DIP_X_CAM_COLOR_POS_MAIN;                        /* 5034, 0x15027034, DIP_A_CAM_COLOR_POS_MAIN */
	DIP_X_REG_CAM_COLOR_INK_DATA_MAIN               DIP_X_CAM_COLOR_INK_DATA_MAIN;                   /* 5038, 0x15027038, DIP_A_CAM_COLOR_INK_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_CR            DIP_X_CAM_COLOR_INK_DATA_MAIN_CR;                /* 503C, 0x1502703C, DIP_A_CAM_COLOR_INK_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN            DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN;                /* 5040, 0x15027040, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_CR         DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR;             /* 5044, 0x15027044, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN           DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN;               /* 5048, 0x15027048, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_CR        DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR;            /* 504C, 0x1502704C, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_1_0_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN;                /* 5050, 0x15027050, DIP_A_CAM_COLOR_Y_SLOPE_1_0_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_3_2_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN;                /* 5054, 0x15027054, DIP_A_CAM_COLOR_Y_SLOPE_3_2_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_5_4_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN;                /* 5058, 0x15027058, DIP_A_CAM_COLOR_Y_SLOPE_5_4_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_7_6_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN;                /* 505C, 0x1502705C, DIP_A_CAM_COLOR_Y_SLOPE_7_6_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_9_8_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN;                /* 5060, 0x15027060, DIP_A_CAM_COLOR_Y_SLOPE_9_8_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_11_10_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN;              /* 5064, 0x15027064, DIP_A_CAM_COLOR_Y_SLOPE_11_10_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_13_12_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN;              /* 5068, 0x15027068, DIP_A_CAM_COLOR_Y_SLOPE_13_12_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_15_14_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN;              /* 506C, 0x1502706C, DIP_A_CAM_COLOR_Y_SLOPE_15_14_MAIN */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_0              DIP_X_CAM_COLOR_LOCAL_HUE_CD_0;                  /* 5070, 0x15027070, DIP_A_CAM_COLOR_LOCAL_HUE_CD_0 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_1              DIP_X_CAM_COLOR_LOCAL_HUE_CD_1;                  /* 5074, 0x15027074, DIP_A_CAM_COLOR_LOCAL_HUE_CD_1 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_2              DIP_X_CAM_COLOR_LOCAL_HUE_CD_2;                  /* 5078, 0x15027078, DIP_A_CAM_COLOR_LOCAL_HUE_CD_2 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_3              DIP_X_CAM_COLOR_LOCAL_HUE_CD_3;                  /* 507C, 0x1502707C, DIP_A_CAM_COLOR_LOCAL_HUE_CD_3 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_4              DIP_X_CAM_COLOR_LOCAL_HUE_CD_4;                  /* 5080, 0x15027080, DIP_A_CAM_COLOR_LOCAL_HUE_CD_4 */
	DIP_X_REG_CAM_COLOR_TWO_D_WINDOW_1              DIP_X_CAM_COLOR_TWO_D_WINDOW_1;                  /* 5084, 0x15027084, DIP_A_CAM_COLOR_TWO_D_WINDOW_1 */
	DIP_X_REG_CAM_COLOR_TWO_D_W1_RESULT             DIP_X_CAM_COLOR_TWO_D_W1_RESULT;                 /* 5088, 0x15027088, DIP_A_CAM_COLOR_TWO_D_W1_RESULT */
	DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN         DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN;             /* 508C, 0x1502708C, DIP_A_CAM_COLOR_SAT_HIST_X_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN         DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN;             /* 5090, 0x15027090, DIP_A_CAM_COLOR_SAT_HIST_Y_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_BWS_2                       DIP_X_CAM_COLOR_BWS_2;                           /* 5094, 0x15027094, DIP_A_CAM_COLOR_BWS_2 */
	DIP_X_REG_CAM_COLOR_CRC_0                       DIP_X_CAM_COLOR_CRC_0;                           /* 5098, 0x15027098, DIP_A_CAM_COLOR_CRC_0 */
	DIP_X_REG_CAM_COLOR_CRC_1                       DIP_X_CAM_COLOR_CRC_1;                           /* 509C, 0x1502709C, DIP_A_CAM_COLOR_CRC_1 */
	DIP_X_REG_CAM_COLOR_CRC_2                       DIP_X_CAM_COLOR_CRC_2;                           /* 50A0, 0x150270A0, DIP_A_CAM_COLOR_CRC_2 */
	DIP_X_REG_CAM_COLOR_CRC_3                       DIP_X_CAM_COLOR_CRC_3;                           /* 50A4, 0x150270A4, DIP_A_CAM_COLOR_CRC_3 */
	DIP_X_REG_CAM_COLOR_CRC_4                       DIP_X_CAM_COLOR_CRC_4;                           /* 50A8, 0x150270A8, DIP_A_CAM_COLOR_CRC_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0;             /* 50AC, 0x150270AC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1;             /* 50B0, 0x150270B0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2;             /* 50B4, 0x150270B4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3;             /* 50B8, 0x150270B8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4;             /* 50BC, 0x150270BC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0;             /* 50C0, 0x150270C0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1;             /* 50C4, 0x150270C4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2;             /* 50C8, 0x150270C8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3;             /* 50CC, 0x150270CC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4;             /* 50D0, 0x150270D0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0;             /* 50D4, 0x150270D4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1;             /* 50D8, 0x150270D8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2;             /* 50DC, 0x150270DC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3;             /* 50E0, 0x150270E0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4;             /* 50E4, 0x150270E4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_0        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0;            /* 50E8, 0x150270E8, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_1        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1;            /* 50EC, 0x150270EC, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_2        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2;            /* 50F0, 0x150270F0, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_3        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3;            /* 50F4, 0x150270F4, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_4        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4;            /* 50F8, 0x150270F8, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_0        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0;            /* 50FC, 0x150270FC, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_1        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1;            /* 5100, 0x15027100, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_2        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2;            /* 5104, 0x15027104, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_3        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3;            /* 5108, 0x15027108, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_4        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4;            /* 510C, 0x1502710C, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_4 */
	DIP_X_REG_CAM_COLOR_START                       DIP_X_CAM_COLOR_START;                           /* 5110, 0x15027110, DIP_A_CAM_COLOR_START */
	DIP_X_REG_CAM_COLOR_INTEN                       DIP_X_CAM_COLOR_INTEN;                           /* 5114, 0x15027114, DIP_A_CAM_COLOR_INTEN */
	DIP_X_REG_CAM_COLOR_INTSTA                      DIP_X_CAM_COLOR_INTSTA;                          /* 5118, 0x15027118, DIP_A_CAM_COLOR_INTSTA */
	DIP_X_REG_CAM_COLOR_OUT_SEL                     DIP_X_CAM_COLOR_OUT_SEL;                         /* 511C, 0x1502711C, DIP_A_CAM_COLOR_OUT_SEL */
	DIP_X_REG_CAM_COLOR_FRAME_DONE_DEL              DIP_X_CAM_COLOR_FRAME_DONE_DEL;                  /* 5120, 0x15027120, DIP_A_CAM_COLOR_FRAME_DONE_DEL */
	DIP_X_REG_CAM_COLOR_CRC                         DIP_X_CAM_COLOR_CRC;                             /* 5124, 0x15027124, DIP_A_CAM_COLOR_CRC */
	DIP_X_REG_CAM_COLOR_SW_SCRATCH                  DIP_X_CAM_COLOR_SW_SCRATCH;                      /* 5128, 0x15027128, DIP_A_CAM_COLOR_SW_SCRATCH */
	DIP_X_REG_CAM_COLOR_CK_ON                       DIP_X_CAM_COLOR_CK_ON;                           /* 512C, 0x1502712C, DIP_A_CAM_COLOR_CK_ON */
	DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH           DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH;               /* 5130, 0x15027130, DIP_A_CAM_COLOR_INTERNAL_IP_WIDTH */
	DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT          DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT;              /* 5134, 0x15027134, DIP_A_CAM_COLOR_INTERNAL_IP_HEIGHT */
	DIP_X_REG_CAM_COLOR_CM1_EN                      DIP_X_CAM_COLOR_CM1_EN;                          /* 5138, 0x15027138, DIP_A_CAM_COLOR_CM1_EN */
	DIP_X_REG_CAM_COLOR_CM2_EN                      DIP_X_CAM_COLOR_CM2_EN;                          /* 513C, 0x1502713C, DIP_A_CAM_COLOR_CM2_EN */
	DIP_X_REG_CAM_COLOR_SHADOW_CTRL                 DIP_X_CAM_COLOR_SHADOW_CTRL;                     /* 5140, 0x15027140, DIP_A_CAM_COLOR_SHADOW_CTRL */
	DIP_X_REG_CAM_COLOR_R0_CRC                      DIP_X_CAM_COLOR_R0_CRC;                          /* 5144, 0x15027144, DIP_A_CAM_COLOR_R0_CRC */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_0              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0;                  /* 5148, 0x15027148, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_1              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1;                  /* 514C, 0x1502714C, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_2              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2;                  /* 5150, 0x15027150, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_3              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3;                  /* 5154, 0x15027154, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_4              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4;                  /* 5158, 0x15027158, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_0             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0;                 /* 515C, 0x1502715C, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_1             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1;                 /* 5160, 0x15027160, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_2             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2;                 /* 5164, 0x15027164, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_3             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3;                 /* 5168, 0x15027168, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_4             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4;                 /* 516C, 0x1502716C, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0;                /* 5170, 0x15027170, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1;                /* 5174, 0x15027174, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2;                /* 5178, 0x15027178, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3;                /* 517C, 0x1502717C, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4;                /* 5180, 0x15027180, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0;                /* 5184, 0x15027184, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1;                /* 5188, 0x15027188, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2;                /* 518C, 0x1502718C, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3;                /* 5190, 0x15027190, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4;                /* 5194, 0x15027194, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0;                /* 5198, 0x15027198, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1;                /* 519C, 0x1502719C, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2;                /* 51A0, 0x150271A0, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3;                /* 51A4, 0x150271A4, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4;                /* 51A8, 0x150271A8, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_4 */
	DIP_X_REG_CAM_COLOR_LSP_1                       DIP_X_CAM_COLOR_LSP_1;                           /* 51AC, 0x150271AC, DIP_A_CAM_COLOR_LSP_1 */
	DIP_X_REG_CAM_COLOR_LSP_2                       DIP_X_CAM_COLOR_LSP_2;                           /* 51B0, 0x150271B0, DIP_A_CAM_COLOR_LSP_2 */
	DIP_X_REG_CAM_COLOR_CM_CONTROL                  DIP_X_CAM_COLOR_CM_CONTROL;                      /* 51B4, 0x150271B4, DIP_A_CAM_COLOR_CM_CONTROL */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_0                 DIP_X_CAM_COLOR_CM_W1_HUE_0;                     /* 51B8, 0x150271B8, DIP_A_CAM_COLOR_CM_W1_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_1                 DIP_X_CAM_COLOR_CM_W1_HUE_1;                     /* 51BC, 0x150271BC, DIP_A_CAM_COLOR_CM_W1_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_2                 DIP_X_CAM_COLOR_CM_W1_HUE_2;                     /* 51C0, 0x150271C0, DIP_A_CAM_COLOR_CM_W1_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_3                 DIP_X_CAM_COLOR_CM_W1_HUE_3;                     /* 51C4, 0x150271C4, DIP_A_CAM_COLOR_CM_W1_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_4                 DIP_X_CAM_COLOR_CM_W1_HUE_4;                     /* 51C8, 0x150271C8, DIP_A_CAM_COLOR_CM_W1_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_0                DIP_X_CAM_COLOR_CM_W1_LUMA_0;                    /* 51CC, 0x150271CC, DIP_A_CAM_COLOR_CM_W1_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_1                DIP_X_CAM_COLOR_CM_W1_LUMA_1;                    /* 51D0, 0x150271D0, DIP_A_CAM_COLOR_CM_W1_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_2                DIP_X_CAM_COLOR_CM_W1_LUMA_2;                    /* 51D4, 0x150271D4, DIP_A_CAM_COLOR_CM_W1_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_3                DIP_X_CAM_COLOR_CM_W1_LUMA_3;                    /* 51D8, 0x150271D8, DIP_A_CAM_COLOR_CM_W1_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_4                DIP_X_CAM_COLOR_CM_W1_LUMA_4;                    /* 51DC, 0x150271DC, DIP_A_CAM_COLOR_CM_W1_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_0                 DIP_X_CAM_COLOR_CM_W1_SAT_0;                     /* 51E0, 0x150271E0, DIP_A_CAM_COLOR_CM_W1_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_1                 DIP_X_CAM_COLOR_CM_W1_SAT_1;                     /* 51E4, 0x150271E4, DIP_A_CAM_COLOR_CM_W1_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_2                 DIP_X_CAM_COLOR_CM_W1_SAT_2;                     /* 51E8, 0x150271E8, DIP_A_CAM_COLOR_CM_W1_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_3                 DIP_X_CAM_COLOR_CM_W1_SAT_3;                     /* 51EC, 0x150271EC, DIP_A_CAM_COLOR_CM_W1_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_4                 DIP_X_CAM_COLOR_CM_W1_SAT_4;                     /* 51F0, 0x150271F0, DIP_A_CAM_COLOR_CM_W1_SAT_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_0                 DIP_X_CAM_COLOR_CM_W2_HUE_0;                     /* 51F4, 0x150271F4, DIP_A_CAM_COLOR_CM_W2_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_1                 DIP_X_CAM_COLOR_CM_W2_HUE_1;                     /* 51F8, 0x150271F8, DIP_A_CAM_COLOR_CM_W2_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_2                 DIP_X_CAM_COLOR_CM_W2_HUE_2;                     /* 51FC, 0x150271FC, DIP_A_CAM_COLOR_CM_W2_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_3                 DIP_X_CAM_COLOR_CM_W2_HUE_3;                     /* 5200, 0x15027200, DIP_A_CAM_COLOR_CM_W2_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_4                 DIP_X_CAM_COLOR_CM_W2_HUE_4;                     /* 5204, 0x15027204, DIP_A_CAM_COLOR_CM_W2_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_0                DIP_X_CAM_COLOR_CM_W2_LUMA_0;                    /* 5208, 0x15027208, DIP_A_CAM_COLOR_CM_W2_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_1                DIP_X_CAM_COLOR_CM_W2_LUMA_1;                    /* 520C, 0x1502720C, DIP_A_CAM_COLOR_CM_W2_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_2                DIP_X_CAM_COLOR_CM_W2_LUMA_2;                    /* 5210, 0x15027210, DIP_A_CAM_COLOR_CM_W2_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_3                DIP_X_CAM_COLOR_CM_W2_LUMA_3;                    /* 5214, 0x15027214, DIP_A_CAM_COLOR_CM_W2_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_4                DIP_X_CAM_COLOR_CM_W2_LUMA_4;                    /* 5218, 0x15027218, DIP_A_CAM_COLOR_CM_W2_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_0                 DIP_X_CAM_COLOR_CM_W2_SAT_0;                     /* 521C, 0x1502721C, DIP_A_CAM_COLOR_CM_W2_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_1                 DIP_X_CAM_COLOR_CM_W2_SAT_1;                     /* 5220, 0x15027220, DIP_A_CAM_COLOR_CM_W2_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_2                 DIP_X_CAM_COLOR_CM_W2_SAT_2;                     /* 5224, 0x15027224, DIP_A_CAM_COLOR_CM_W2_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_3                 DIP_X_CAM_COLOR_CM_W2_SAT_3;                     /* 5228, 0x15027228, DIP_A_CAM_COLOR_CM_W2_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_4                 DIP_X_CAM_COLOR_CM_W2_SAT_4;                     /* 522C, 0x1502722C, DIP_A_CAM_COLOR_CM_W2_SAT_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_0                 DIP_X_CAM_COLOR_CM_W3_HUE_0;                     /* 5230, 0x15027230, DIP_A_CAM_COLOR_CM_W3_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_1                 DIP_X_CAM_COLOR_CM_W3_HUE_1;                     /* 5234, 0x15027234, DIP_A_CAM_COLOR_CM_W3_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_2                 DIP_X_CAM_COLOR_CM_W3_HUE_2;                     /* 5238, 0x15027238, DIP_A_CAM_COLOR_CM_W3_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_3                 DIP_X_CAM_COLOR_CM_W3_HUE_3;                     /* 523C, 0x1502723C, DIP_A_CAM_COLOR_CM_W3_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_4                 DIP_X_CAM_COLOR_CM_W3_HUE_4;                     /* 5240, 0x15027240, DIP_A_CAM_COLOR_CM_W3_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_0                DIP_X_CAM_COLOR_CM_W3_LUMA_0;                    /* 5244, 0x15027244, DIP_A_CAM_COLOR_CM_W3_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_1                DIP_X_CAM_COLOR_CM_W3_LUMA_1;                    /* 5248, 0x15027248, DIP_A_CAM_COLOR_CM_W3_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_2                DIP_X_CAM_COLOR_CM_W3_LUMA_2;                    /* 524C, 0x1502724C, DIP_A_CAM_COLOR_CM_W3_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_3                DIP_X_CAM_COLOR_CM_W3_LUMA_3;                    /* 5250, 0x15027250, DIP_A_CAM_COLOR_CM_W3_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_4                DIP_X_CAM_COLOR_CM_W3_LUMA_4;                    /* 5254, 0x15027254, DIP_A_CAM_COLOR_CM_W3_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_0                 DIP_X_CAM_COLOR_CM_W3_SAT_0;                     /* 5258, 0x15027258, DIP_A_CAM_COLOR_CM_W3_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_1                 DIP_X_CAM_COLOR_CM_W3_SAT_1;                     /* 525C, 0x1502725C, DIP_A_CAM_COLOR_CM_W3_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_2                 DIP_X_CAM_COLOR_CM_W3_SAT_2;                     /* 5260, 0x15027260, DIP_A_CAM_COLOR_CM_W3_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_3                 DIP_X_CAM_COLOR_CM_W3_SAT_3;                     /* 5264, 0x15027264, DIP_A_CAM_COLOR_CM_W3_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_4                 DIP_X_CAM_COLOR_CM_W3_SAT_4;                     /* 5268, 0x15027268, DIP_A_CAM_COLOR_CM_W3_SAT_4 */
	UINT32                                          rsv_526C[5];                                     /* 526C..527F, 0x1502726C..1502727F */
	DIP_X_REG_HFG_CON_0                             DIP_X_HFG_CON_0;                                 /* 5280, 0x15027280, DIP_A_HFG_CON_0 */
	DIP_X_REG_HFG_LUMA_0                            DIP_X_HFG_LUMA_0;                                /* 5284, 0x15027284, DIP_A_HFG_LUMA_0 */
	DIP_X_REG_HFG_LUMA_1                            DIP_X_HFG_LUMA_1;                                /* 5288, 0x15027288, DIP_A_HFG_LUMA_1 */
	DIP_X_REG_HFG_LUMA_2                            DIP_X_HFG_LUMA_2;                                /* 528C, 0x1502728C, DIP_A_HFG_LUMA_2 */
	DIP_X_REG_HFG_LCE_0                             DIP_X_HFG_LCE_0;                                 /* 5290, 0x15027290, DIP_A_HFG_LCE_0 */
	DIP_X_REG_HFG_LCE_1                             DIP_X_HFG_LCE_1;                                 /* 5294, 0x15027294, DIP_A_HFG_LCE_1 */
	DIP_X_REG_HFG_LCE_2                             DIP_X_HFG_LCE_2;                                 /* 5298, 0x15027298, DIP_A_HFG_LCE_2 */
	DIP_X_REG_HFG_RAN_0                             DIP_X_HFG_RAN_0;                                 /* 529C, 0x1502729C, DIP_A_HFG_RAN_0 */
	DIP_X_REG_HFG_RAN_1                             DIP_X_HFG_RAN_1;                                 /* 52A0, 0x150272A0, DIP_A_HFG_RAN_1 */
	DIP_X_REG_HFG_RAN_2                             DIP_X_HFG_RAN_2;                                 /* 52A4, 0x150272A4, DIP_A_HFG_RAN_2 */
	DIP_X_REG_HFG_RAN_3                             DIP_X_HFG_RAN_3;                                 /* 52A8, 0x150272A8, DIP_A_HFG_RAN_3 */
	DIP_X_REG_HFG_RAN_4                             DIP_X_HFG_RAN_4;                                 /* 52AC, 0x150272AC, DIP_A_HFG_RAN_4 */
	DIP_X_REG_HFG_CROP_X                            DIP_X_HFG_CROP_X;                                /* 52B0, 0x150272B0, DIP_A_HFG_CROP_X */
	DIP_X_REG_HFG_CROP_Y                            DIP_X_HFG_CROP_Y;                                /* 52B4, 0x150272B4, DIP_A_HFG_CROP_Y */
	UINT32                                          rsv_52B8[2];                                     /* 52B8..52BF, 0x150272B8..150272BF */
	DIP_X_REG_SL2I_CEN                              DIP_X_SL2I_CEN;                                  /* 52C0, 0x150272C0, DIP_A_SL2I_CEN */
	DIP_X_REG_SL2I_RR_CON0                          DIP_X_SL2I_RR_CON0;                              /* 52C4, 0x150272C4, DIP_A_SL2I_RR_CON0 */
	DIP_X_REG_SL2I_RR_CON1                          DIP_X_SL2I_RR_CON1;                              /* 52C8, 0x150272C8, DIP_A_SL2I_RR_CON1 */
	DIP_X_REG_SL2I_GAIN                             DIP_X_SL2I_GAIN;                                 /* 52CC, 0x150272CC, DIP_A_SL2I_GAIN */
	DIP_X_REG_SL2I_RZ                               DIP_X_SL2I_RZ;                                   /* 52D0, 0x150272D0, DIP_A_SL2I_RZ */
	DIP_X_REG_SL2I_XOFF                             DIP_X_SL2I_XOFF;                                 /* 52D4, 0x150272D4, DIP_A_SL2I_XOFF */
	DIP_X_REG_SL2I_YOFF                             DIP_X_SL2I_YOFF;                                 /* 52D8, 0x150272D8, DIP_A_SL2I_YOFF */
	DIP_X_REG_SL2I_SLP_CON0                         DIP_X_SL2I_SLP_CON0;                             /* 52DC, 0x150272DC, DIP_A_SL2I_SLP_CON0 */
	DIP_X_REG_SL2I_SLP_CON1                         DIP_X_SL2I_SLP_CON1;                             /* 52E0, 0x150272E0, DIP_A_SL2I_SLP_CON1 */
	DIP_X_REG_SL2I_SLP_CON2                         DIP_X_SL2I_SLP_CON2;                             /* 52E4, 0x150272E4, DIP_A_SL2I_SLP_CON2 */
	DIP_X_REG_SL2I_SLP_CON3                         DIP_X_SL2I_SLP_CON3;                             /* 52E8, 0x150272E8, DIP_A_SL2I_SLP_CON3 */
	DIP_X_REG_SL2I_SIZE                             DIP_X_SL2I_SIZE;                                 /* 52EC, 0x150272EC, DIP_A_SL2I_SIZE */
	UINT32                                          rsv_52F0[4];                                     /* 52F0..52FF, 0x150272F0..150272FF */
	DIP_X_REG_CRZ_CONTROL                           DIP_X_CRZ_CONTROL;                               /* 5300, 0x15027300, DIP_A_CRZ_CONTROL */
	DIP_X_REG_CRZ_IN_IMG                            DIP_X_CRZ_IN_IMG;                                /* 5304, 0x15027304, DIP_A_CRZ_IN_IMG */
	DIP_X_REG_CRZ_OUT_IMG                           DIP_X_CRZ_OUT_IMG;                               /* 5308, 0x15027308, DIP_A_CRZ_OUT_IMG */
	DIP_X_REG_CRZ_HORI_STEP                         DIP_X_CRZ_HORI_STEP;                             /* 530C, 0x1502730C, DIP_A_CRZ_HORI_STEP */
	DIP_X_REG_CRZ_VERT_STEP                         DIP_X_CRZ_VERT_STEP;                             /* 5310, 0x15027310, DIP_A_CRZ_VERT_STEP */
	DIP_X_REG_CRZ_LUMA_HORI_INT_OFST                DIP_X_CRZ_LUMA_HORI_INT_OFST;                    /* 5314, 0x15027314, DIP_A_CRZ_LUMA_HORI_INT_OFST */
	DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST                DIP_X_CRZ_LUMA_HORI_SUB_OFST;                    /* 5318, 0x15027318, DIP_A_CRZ_LUMA_HORI_SUB_OFST */
	DIP_X_REG_CRZ_LUMA_VERT_INT_OFST                DIP_X_CRZ_LUMA_VERT_INT_OFST;                    /* 531C, 0x1502731C, DIP_A_CRZ_LUMA_VERT_INT_OFST */
	DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST                DIP_X_CRZ_LUMA_VERT_SUB_OFST;                    /* 5320, 0x15027320, DIP_A_CRZ_LUMA_VERT_SUB_OFST */
	DIP_X_REG_CRZ_CHRO_HORI_INT_OFST                DIP_X_CRZ_CHRO_HORI_INT_OFST;                    /* 5324, 0x15027324, DIP_A_CRZ_CHRO_HORI_INT_OFST */
	DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST                DIP_X_CRZ_CHRO_HORI_SUB_OFST;                    /* 5328, 0x15027328, DIP_A_CRZ_CHRO_HORI_SUB_OFST */
	DIP_X_REG_CRZ_CHRO_VERT_INT_OFST                DIP_X_CRZ_CHRO_VERT_INT_OFST;                    /* 532C, 0x1502732C, DIP_A_CRZ_CHRO_VERT_INT_OFST */
	DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST                DIP_X_CRZ_CHRO_VERT_SUB_OFST;                    /* 5330, 0x15027330, DIP_A_CRZ_CHRO_VERT_SUB_OFST */
	DIP_X_REG_CRZ_SPARE_1                           DIP_X_CRZ_SPARE_1;                               /* 5334, 0x15027334, DIP_A_CRZ_SPARE_1 */
	UINT32                                          rsv_5338[18];                                    /* 5338..537F, 0x15027338..1502737F */
	DIP_X_REG_CAM_TNR_ENG_CON                       DIP_X_CAM_TNR_ENG_CON;                           /* 5380, 0x15027380, DIP_A_CAM_TNR_ENG_CON */
	DIP_X_REG_CAM_TNR_SIZ                           DIP_X_CAM_TNR_SIZ;                               /* 5384, 0x15027384, DIP_A_CAM_TNR_SIZ */
	DIP_X_REG_CAM_TNR_TILE_XY                       DIP_X_CAM_TNR_TILE_XY;                           /* 5388, 0x15027388, DIP_A_CAM_TNR_TILE_XY */
	DIP_X_REG_NR3D_ON_CON                           DIP_X_NR3D_ON_CON;                               /* 538C, 0x1502738C, DIP_A_NR3D_ON_CON */
	DIP_X_REG_NR3D_ON_OFF                           DIP_X_NR3D_ON_OFF;                               /* 5390, 0x15027390, DIP_A_NR3D_ON_OFF */
	DIP_X_REG_NR3D_ON_SIZ                           DIP_X_NR3D_ON_SIZ;                               /* 5394, 0x15027394, DIP_A_NR3D_ON_SIZ */
	DIP_X_REG_MDP_TNR_TNR_ENABLE                    DIP_X_MDP_TNR_TNR_ENABLE;                        /* 5398, 0x15027398, DIP_A_MDP_TNR_TNR_ENABLE */
	DIP_X_REG_MDP_TNR_FLT_CONFIG                    DIP_X_MDP_TNR_FLT_CONFIG;                        /* 539C, 0x1502739C, DIP_A_MDP_TNR_FLT_CONFIG */
	DIP_X_REG_MDP_TNR_FB_INFO1                      DIP_X_MDP_TNR_FB_INFO1;                          /* 53A0, 0x150273A0, DIP_A_MDP_TNR_FB_INFO1 */
	DIP_X_REG_MDP_TNR_THR_1                         DIP_X_MDP_TNR_THR_1;                             /* 53A4, 0x150273A4, DIP_A_MDP_TNR_THR_1 */
	DIP_X_REG_MDP_TNR_CURVE_1                       DIP_X_MDP_TNR_CURVE_1;                           /* 53A8, 0x150273A8, DIP_A_MDP_TNR_CURVE_1 */
	DIP_X_REG_MDP_TNR_CURVE_2                       DIP_X_MDP_TNR_CURVE_2;                           /* 53AC, 0x150273AC, DIP_A_MDP_TNR_CURVE_2 */
	DIP_X_REG_MDP_TNR_CURVE_3                       DIP_X_MDP_TNR_CURVE_3;                           /* 53B0, 0x150273B0, DIP_A_MDP_TNR_CURVE_3 */
	DIP_X_REG_MDP_TNR_CURVE_4                       DIP_X_MDP_TNR_CURVE_4;                           /* 53B4, 0x150273B4, DIP_A_MDP_TNR_CURVE_4 */
	DIP_X_REG_MDP_TNR_CURVE_5                       DIP_X_MDP_TNR_CURVE_5;                           /* 53B8, 0x150273B8, DIP_A_MDP_TNR_CURVE_5 */
	DIP_X_REG_MDP_TNR_CURVE_6                       DIP_X_MDP_TNR_CURVE_6;                           /* 53BC, 0x150273BC, DIP_A_MDP_TNR_CURVE_6 */
	DIP_X_REG_MDP_TNR_CURVE_7                       DIP_X_MDP_TNR_CURVE_7;                           /* 53C0, 0x150273C0, DIP_A_MDP_TNR_CURVE_7 */
	DIP_X_REG_MDP_TNR_CURVE_8                       DIP_X_MDP_TNR_CURVE_8;                           /* 53C4, 0x150273C4, DIP_A_MDP_TNR_CURVE_8 */
	DIP_X_REG_MDP_TNR_CURVE_9                       DIP_X_MDP_TNR_CURVE_9;                           /* 53C8, 0x150273C8, DIP_A_MDP_TNR_CURVE_9 */
	DIP_X_REG_MDP_TNR_CURVE_10                      DIP_X_MDP_TNR_CURVE_10;                          /* 53CC, 0x150273CC, DIP_A_MDP_TNR_CURVE_10 */
	DIP_X_REG_MDP_TNR_CURVE_11                      DIP_X_MDP_TNR_CURVE_11;                          /* 53D0, 0x150273D0, DIP_A_MDP_TNR_CURVE_11 */
	DIP_X_REG_MDP_TNR_CURVE_12                      DIP_X_MDP_TNR_CURVE_12;                          /* 53D4, 0x150273D4, DIP_A_MDP_TNR_CURVE_12 */
	DIP_X_REG_MDP_TNR_CURVE_13                      DIP_X_MDP_TNR_CURVE_13;                          /* 53D8, 0x150273D8, DIP_A_MDP_TNR_CURVE_13 */
	DIP_X_REG_MDP_TNR_CURVE_14                      DIP_X_MDP_TNR_CURVE_14;                          /* 53DC, 0x150273DC, DIP_A_MDP_TNR_CURVE_14 */
	DIP_X_REG_MDP_TNR_CURVE_15                      DIP_X_MDP_TNR_CURVE_15;                          /* 53E0, 0x150273E0, DIP_A_MDP_TNR_CURVE_15 */
	DIP_X_REG_MDP_TNR_R2C_1                         DIP_X_MDP_TNR_R2C_1;                             /* 53E4, 0x150273E4, DIP_A_MDP_TNR_R2C_1 */
	DIP_X_REG_MDP_TNR_R2C_2                         DIP_X_MDP_TNR_R2C_2;                             /* 53E8, 0x150273E8, DIP_A_MDP_TNR_R2C_2 */
	DIP_X_REG_MDP_TNR_R2C_3                         DIP_X_MDP_TNR_R2C_3;                             /* 53EC, 0x150273EC, DIP_A_MDP_TNR_R2C_3 */
	DIP_X_REG_MDP_TNR_DBG_6                         DIP_X_MDP_TNR_DBG_6;                             /* 53F0, 0x150273F0, DIP_A_MDP_TNR_DBG_6 */
	DIP_X_REG_MDP_TNR_DBG_15                        DIP_X_MDP_TNR_DBG_15;                            /* 53F4, 0x150273F4, DIP_A_MDP_TNR_DBG_15 */
	DIP_X_REG_MDP_TNR_DBG_16                        DIP_X_MDP_TNR_DBG_16;                            /* 53F8, 0x150273F8, DIP_A_MDP_TNR_DBG_16 */
	DIP_X_REG_MDP_TNR_DEMO_1                        DIP_X_MDP_TNR_DEMO_1;                            /* 53FC, 0x150273FC, DIP_A_MDP_TNR_DEMO_1 */
	DIP_X_REG_MDP_TNR_DEMO_2                        DIP_X_MDP_TNR_DEMO_2;                            /* 5400, 0x15027400, DIP_A_MDP_TNR_DEMO_2 */
	DIP_X_REG_MDP_TNR_ATPG                          DIP_X_MDP_TNR_ATPG;                              /* 5404, 0x15027404, DIP_A_MDP_TNR_ATPG */
	DIP_X_REG_MDP_TNR_DMY_0                         DIP_X_MDP_TNR_DMY_0;                             /* 5408, 0x15027408, DIP_A_MDP_TNR_DMY_0 */
	DIP_X_REG_MDP_TNR_DBG_17                        DIP_X_MDP_TNR_DBG_17;                            /* 540C, 0x1502740C, DIP_A_MDP_TNR_DBG_17 */
	DIP_X_REG_MDP_TNR_DBG_18                        DIP_X_MDP_TNR_DBG_18;                            /* 5410, 0x15027410, DIP_A_MDP_TNR_DBG_18 */
	DIP_X_REG_MDP_TNR_DBG_1                         DIP_X_MDP_TNR_DBG_1;                             /* 5414, 0x15027414, DIP_A_MDP_TNR_DBG_1 */
	DIP_X_REG_MDP_TNR_DBG_2                         DIP_X_MDP_TNR_DBG_2;                             /* 5418, 0x15027418, DIP_A_MDP_TNR_DBG_2 */
	UINT32                                          rsv_541C[9];                                     /* 541C..543F, 0x1502741C..1502743F */
	DIP_X_REG_SL2E_CEN                              DIP_X_SL2E_CEN;                                  /* 5440, 0x15027440, DIP_A_SL2E_CEN */
	DIP_X_REG_SL2E_RR_CON0                          DIP_X_SL2E_RR_CON0;                              /* 5444, 0x15027444, DIP_A_SL2E_RR_CON0 */
	DIP_X_REG_SL2E_RR_CON1                          DIP_X_SL2E_RR_CON1;                              /* 5448, 0x15027448, DIP_A_SL2E_RR_CON1 */
	DIP_X_REG_SL2E_GAIN                             DIP_X_SL2E_GAIN;                                 /* 544C, 0x1502744C, DIP_A_SL2E_GAIN */
	DIP_X_REG_SL2E_RZ                               DIP_X_SL2E_RZ;                                   /* 5450, 0x15027450, DIP_A_SL2E_RZ */
	DIP_X_REG_SL2E_XOFF                             DIP_X_SL2E_XOFF;                                 /* 5454, 0x15027454, DIP_A_SL2E_XOFF */
	DIP_X_REG_SL2E_YOFF                             DIP_X_SL2E_YOFF;                                 /* 5458, 0x15027458, DIP_A_SL2E_YOFF */
	DIP_X_REG_SL2E_SLP_CON0                         DIP_X_SL2E_SLP_CON0;                             /* 545C, 0x1502745C, DIP_A_SL2E_SLP_CON0 */
	DIP_X_REG_SL2E_SLP_CON1                         DIP_X_SL2E_SLP_CON1;                             /* 5460, 0x15027460, DIP_A_SL2E_SLP_CON1 */
	DIP_X_REG_SL2E_SLP_CON2                         DIP_X_SL2E_SLP_CON2;                             /* 5464, 0x15027464, DIP_A_SL2E_SLP_CON2 */
	DIP_X_REG_SL2E_SLP_CON3                         DIP_X_SL2E_SLP_CON3;                             /* 5468, 0x15027468, DIP_A_SL2E_SLP_CON3 */
	DIP_X_REG_SL2E_SIZE                             DIP_X_SL2E_SIZE;                                 /* 546C, 0x1502746C, DIP_A_SL2E_SIZE */
	UINT32                                          rsv_5470[4];                                     /* 5470..547F, 0x15027470..1502747F */
	DIP_X_REG_CRSP_CTRL                             DIP_X_CRSP_CTRL;                                 /* 5480, 0x15027480, DIP_A_CRSP_CTRL */
	UINT32                                          rsv_5484;                                        /* 5484, 0x15027484 */
	DIP_X_REG_CRSP_OUT_IMG                          DIP_X_CRSP_OUT_IMG;                              /* 5488, 0x15027488, DIP_A_CRSP_OUT_IMG */
	DIP_X_REG_CRSP_STEP_OFST                        DIP_X_CRSP_STEP_OFST;                            /* 548C, 0x1502748C, DIP_A_CRSP_STEP_OFST */
	DIP_X_REG_CRSP_CROP_X                           DIP_X_CRSP_CROP_X;                               /* 5490, 0x15027490, DIP_A_CRSP_CROP_X */
	DIP_X_REG_CRSP_CROP_Y                           DIP_X_CRSP_CROP_Y;                               /* 5494, 0x15027494, DIP_A_CRSP_CROP_Y */
	UINT32                                          rsv_5498[10];                                    /* 5498..54BF, 0x15027498..150274BF */
	DIP_X_REG_C24B_TILE_EDGE                        DIP_X_C24B_TILE_EDGE;                            /* 54C0, 0x150274C0, DIP_A_C24B_TILE_EDGE */
	UINT32                                          rsv_54C4[15];                                    /* 54C4..54FF, 0x150274C4..150274FF */
	DIP_X_REG_MDP_CROP_X                            DIP_X_MDP_CROP_X;                                /* 5500, 0x15027500, DIP_A_MDP_CROP_X */
	DIP_X_REG_MDP_CROP_Y                            DIP_X_MDP_CROP_Y;                                /* 5504, 0x15027504, DIP_A_MDP_CROP_Y */
	UINT32                                          rsv_5508[14];                                    /* 5508..553F, 0x15027508..1502753F */
	DIP_X_REG_MIX4_CTRL_0                           DIP_X_MIX4_CTRL_0;                               /* 5540, 0x15027540, DIP_A_MIX4_CTRL_0 */
	DIP_X_REG_MIX4_CTRL_1                           DIP_X_MIX4_CTRL_1;                               /* 5544, 0x15027544, DIP_A_MIX4_CTRL_1 */
	DIP_X_REG_MIX4_SPARE                            DIP_X_MIX4_SPARE;                                /* 5548, 0x15027548, DIP_A_MIX4_SPARE */
	UINT32                                          rsv_554C;                                        /* 554C, 0x1502754C */	// for ISO adaptive
	DIP_X_REG_LCE25_TPIPE_SLM                       DIP_X_LCE25_SLM;                                 /* 5550,  0x15027550 */    // for tile(lce_full_slm_wd & lce_full_slm_ht)
	DIP_X_REG_LCE25_TPIPE_OFFSET                    DIP_X_LCE25_OFFSET;                              /* 5554,  0x15027554  */    // for tile(lce_full_xoff & lce_full_yoff )
	DIP_X_REG_LCE25_TPIPE_OUT                       DIP_X_LCE25_OUT;                                 /* 5558,  0x15027558  */    // for tile(lce_full_out_ht)
	DIP_X_REG_NR3D_VIPI_SIZE                        DIP_X_NR3D_VIPI_SIZE;                            /* 555C,  0x1502755C  */ // for NR3D vipi size
	DIP_X_REG_NR3D_VIPI_OFFSET                      DIP_X_NR3D_VIPI_OFFSET;                          /* 5560,  0x15027560  */ // for NR3D vipi offset
}dip_x_reg_t;

/* auto insert ralf auto gen above */

#endif // _ISP_REG_DIP_H_
