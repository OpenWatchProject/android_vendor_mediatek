#ifndef _ISP_REG_DIP_H_
#define _ISP_REG_DIP_H_


/* auto insert ralf auto gen below */

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
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
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
				FIELD  MFBW_EN                               :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
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
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
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
				FIELD  HLR2_EN                               :  1;		/* 21..21, 0x00200000 */
				FIELD  SL2K_EN                               :  1;		/* 22..22, 0x00400000 */
				FIELD  FLC_EN                                :  1;		/* 23..23, 0x00800000 */
				FIELD  FLC2_EN                               :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
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
				FIELD  MFBO_EN                               :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_EN                              :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_EN                             :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_EN                              :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_EN                             :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_EN                             :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_EN                                :  1;		/* 16..16, 0x00010000 */
				FIELD  ADL2_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
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
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
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
				FIELD  MFBO_SEL                              :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  PAK2_FMT                              :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
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
				FIELD  MFBO_DONE_EN                          :  1;		/* 10..10, 0x00000400 */
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
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
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
				FIELD  MFBO_DONE_STATUS                      :  1;		/* 10..10, 0x00000400 */
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
				FIELD  MFBO_DONE_STATUS                      :  1;		/* 10..10, 0x00000400 */
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
				FIELD  MFBO_TCM_EN                           :  1;		/* 17..17, 0x00020000 */
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
				FIELD  HLR2_TCM_EN                           :  1;		/* 28..28, 0x10000000 */
				FIELD  SL2K_TCM_EN                           :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_TCM_EN                            :  1;		/* 30..30, 0x40000000 */
				FIELD  FLC_TCM_EN                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TCM2_EN;	/* DIP_X_CTL_TDR_TCM2_EN, DIP_A_CTL_TDR_TCM2_EN*/

typedef volatile union _DIP_X_REG_CTL_TDR_EN_STATUS1_
{
		volatile struct	/* 0x15022060 */
		{
				FIELD  CTL_TDR_EN_STATUS1                    : 30;		/*  0..29, 0x3FFFFFFF */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_EN_STATUS1;	/* DIP_X_CTL_TDR_EN_STATUS1, DIP_A_CTL_TDR_EN_STATUS1*/

typedef volatile union _DIP_X_REG_CTL_TDR_EN_STATUS2_
{
		volatile struct	/* 0x15022064 */
		{
				FIELD  CTL_TDR_EN_STATUS2                    : 30;		/*  0..29, 0x3FFFFFFF */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
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
				FIELD  MFBO_DONE_SEL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DONE_SEL                        :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DONE_SEL                       :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DONE_SEL                        :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DONE_SEL                       :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DONE_SEL                       :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DONE_SEL                          :  1;		/* 16..16, 0x00010000 */
				FIELD  ADL2_DONE_SEL                         :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 13;		/* 18..30, 0x7FFC0000 */
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
				FIELD  SPARE_BIT_1                           :  1;		/*  9.. 9, 0x00000200 */
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
				FIELD  MFBW_DCM_DIS                          :  1;		/* 26..26, 0x04000000 */
				FIELD  SPARE_BIT_2                           :  1;		/* 27..27, 0x08000000 */
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
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
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
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
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
				FIELD  MFBO_DCM_DIS                          :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DCM_DIS                         :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DCM_DIS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DCM_DIS                         :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DCM_DIS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DCM_DIS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DCM_DIS                           :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
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
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
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
				FIELD  MFBW_DCM_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  DRZ_DCM_STATUS                        :  1;		/* 27..27, 0x08000000 */
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
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
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
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
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
				FIELD  MFBO_DCM_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_DCM_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_DCM_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_DCM_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_DCM_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_DCM_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_DCM_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
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
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
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
				FIELD  MFBW_REQ_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  DRZ_REQ_STATUS                        :  1;		/* 27..27, 0x08000000 */
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
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
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
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
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
				FIELD  MFBO_REQ_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_REQ_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_REQ_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_REQ_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_REQ_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_REQ_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_REQ_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
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
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
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
				FIELD  MFBW_RDY_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
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
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
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
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
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
				FIELD  MFBO_RDY_STATUS                       :  1;		/* 10..10, 0x00000400 */
				FIELD  IMG2O_RDY_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  IMG2BO_RDY_STATUS                     :  1;		/* 12..12, 0x00001000 */
				FIELD  IMG3O_RDY_STATUS                      :  1;		/* 13..13, 0x00002000 */
				FIELD  IMG3BO_RDY_STATUS                     :  1;		/* 14..14, 0x00004000 */
				FIELD  IMG3CO_RDY_STATUS                     :  1;		/* 15..15, 0x00008000 */
				FIELD  FEO_RDY_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
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
				FIELD  FE_TCM_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CTL_TDR_TCM3_EN;	/* DIP_X_CTL_TDR_TCM3_EN, DIP_A_CTL_TDR_TCM3_EN*/

typedef volatile union _DIP_X_REG_CQ_EN_
{
		volatile struct	/* 0x15022100 */
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
		volatile struct	/* 0x15022104 */
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
		volatile struct	/* 0x15022108 */
		{
				FIELD  CQ_THR0_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR0_BASEADDR;	/* DIP_X_CQ_THR0_BASEADDR, DIP_A_CQ_THR0_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR0_DESC_SIZE_
{
		volatile struct	/* 0x1502210C */
		{
				FIELD  CQ_THR0_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR0_DESC_SIZE;	/* DIP_X_CQ_THR0_DESC_SIZE, DIP_A_CQ_THR0_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR1_CTL_
{
		volatile struct	/* 0x15022110 */
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
		volatile struct	/* 0x15022114 */
		{
				FIELD  CQ_THR1_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR1_BASEADDR;	/* DIP_X_CQ_THR1_BASEADDR, DIP_A_CQ_THR1_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR1_DESC_SIZE_
{
		volatile struct	/* 0x15022118 */
		{
				FIELD  CQ_THR1_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR1_DESC_SIZE;	/* DIP_X_CQ_THR1_DESC_SIZE, DIP_A_CQ_THR1_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR2_CTL_
{
		volatile struct	/* 0x1502211C */
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
		volatile struct	/* 0x15022120 */
		{
				FIELD  CQ_THR2_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR2_BASEADDR;	/* DIP_X_CQ_THR2_BASEADDR, DIP_A_CQ_THR2_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR2_DESC_SIZE_
{
		volatile struct	/* 0x15022124 */
		{
				FIELD  CQ_THR2_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR2_DESC_SIZE;	/* DIP_X_CQ_THR2_DESC_SIZE, DIP_A_CQ_THR2_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR3_CTL_
{
		volatile struct	/* 0x15022128 */
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
		volatile struct	/* 0x1502212C */
		{
				FIELD  CQ_THR3_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR3_BASEADDR;	/* DIP_X_CQ_THR3_BASEADDR, DIP_A_CQ_THR3_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR3_DESC_SIZE_
{
		volatile struct	/* 0x15022130 */
		{
				FIELD  CQ_THR3_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR3_DESC_SIZE;	/* DIP_X_CQ_THR3_DESC_SIZE, DIP_A_CQ_THR3_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR4_CTL_
{
		volatile struct	/* 0x15022134 */
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
		volatile struct	/* 0x15022138 */
		{
				FIELD  CQ_THR4_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR4_BASEADDR;	/* DIP_X_CQ_THR4_BASEADDR, DIP_A_CQ_THR4_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR4_DESC_SIZE_
{
		volatile struct	/* 0x1502213C */
		{
				FIELD  CQ_THR4_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR4_DESC_SIZE;	/* DIP_X_CQ_THR4_DESC_SIZE, DIP_A_CQ_THR4_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR5_CTL_
{
		volatile struct	/* 0x15022140 */
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
		volatile struct	/* 0x15022144 */
		{
				FIELD  CQ_THR5_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR5_BASEADDR;	/* DIP_X_CQ_THR5_BASEADDR, DIP_A_CQ_THR5_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR5_DESC_SIZE_
{
		volatile struct	/* 0x15022148 */
		{
				FIELD  CQ_THR5_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR5_DESC_SIZE;	/* DIP_X_CQ_THR5_DESC_SIZE, DIP_A_CQ_THR5_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR6_CTL_
{
		volatile struct	/* 0x1502214C */
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
		volatile struct	/* 0x15022150 */
		{
				FIELD  CQ_THR6_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR6_BASEADDR;	/* DIP_X_CQ_THR6_BASEADDR, DIP_A_CQ_THR6_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR6_DESC_SIZE_
{
		volatile struct	/* 0x15022154 */
		{
				FIELD  CQ_THR6_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR6_DESC_SIZE;	/* DIP_X_CQ_THR6_DESC_SIZE, DIP_A_CQ_THR6_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR7_CTL_
{
		volatile struct	/* 0x15022158 */
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
		volatile struct	/* 0x1502215C */
		{
				FIELD  CQ_THR7_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR7_BASEADDR;	/* DIP_X_CQ_THR7_BASEADDR, DIP_A_CQ_THR7_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR7_DESC_SIZE_
{
		volatile struct	/* 0x15022160 */
		{
				FIELD  CQ_THR7_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR7_DESC_SIZE;	/* DIP_X_CQ_THR7_DESC_SIZE, DIP_A_CQ_THR7_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR8_CTL_
{
		volatile struct	/* 0x15022164 */
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
		volatile struct	/* 0x15022168 */
		{
				FIELD  CQ_THR8_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR8_BASEADDR;	/* DIP_X_CQ_THR8_BASEADDR, DIP_A_CQ_THR8_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR8_DESC_SIZE_
{
		volatile struct	/* 0x1502216C */
		{
				FIELD  CQ_THR8_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR8_DESC_SIZE;	/* DIP_X_CQ_THR8_DESC_SIZE, DIP_A_CQ_THR8_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR9_CTL_
{
		volatile struct	/* 0x15022170 */
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
		volatile struct	/* 0x15022174 */
		{
				FIELD  CQ_THR9_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR9_BASEADDR;	/* DIP_X_CQ_THR9_BASEADDR, DIP_A_CQ_THR9_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR9_DESC_SIZE_
{
		volatile struct	/* 0x15022178 */
		{
				FIELD  CQ_THR9_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR9_DESC_SIZE;	/* DIP_X_CQ_THR9_DESC_SIZE, DIP_A_CQ_THR9_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR10_CTL_
{
		volatile struct	/* 0x1502217C */
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
		volatile struct	/* 0x15022180 */
		{
				FIELD  CQ_THR10_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR10_BASEADDR;	/* DIP_X_CQ_THR10_BASEADDR, DIP_A_CQ_THR10_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR10_DESC_SIZE_
{
		volatile struct	/* 0x15022184 */
		{
				FIELD  CQ_THR10_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR10_DESC_SIZE;	/* DIP_X_CQ_THR10_DESC_SIZE, DIP_A_CQ_THR10_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR11_CTL_
{
		volatile struct	/* 0x15022188 */
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
		volatile struct	/* 0x1502218C */
		{
				FIELD  CQ_THR11_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR11_BASEADDR;	/* DIP_X_CQ_THR11_BASEADDR, DIP_A_CQ_THR11_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR11_DESC_SIZE_
{
		volatile struct	/* 0x15022190 */
		{
				FIELD  CQ_THR11_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR11_DESC_SIZE;	/* DIP_X_CQ_THR11_DESC_SIZE, DIP_A_CQ_THR11_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR12_CTL_
{
		volatile struct	/* 0x15022194 */
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
		volatile struct	/* 0x15022198 */
		{
				FIELD  CQ_THR12_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR12_BASEADDR;	/* DIP_X_CQ_THR12_BASEADDR, DIP_A_CQ_THR12_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR12_DESC_SIZE_
{
		volatile struct	/* 0x1502219C */
		{
				FIELD  CQ_THR12_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR12_DESC_SIZE;	/* DIP_X_CQ_THR12_DESC_SIZE, DIP_A_CQ_THR12_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR13_CTL_
{
		volatile struct	/* 0x150221A0 */
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
		volatile struct	/* 0x150221A4 */
		{
				FIELD  CQ_THR13_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR13_BASEADDR;	/* DIP_X_CQ_THR13_BASEADDR, DIP_A_CQ_THR13_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR13_DESC_SIZE_
{
		volatile struct	/* 0x150221A8 */
		{
				FIELD  CQ_THR13_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR13_DESC_SIZE;	/* DIP_X_CQ_THR13_DESC_SIZE, DIP_A_CQ_THR13_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR14_CTL_
{
		volatile struct	/* 0x150221AC */
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
		volatile struct	/* 0x150221B0 */
		{
				FIELD  CQ_THR14_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR14_BASEADDR;	/* DIP_X_CQ_THR14_BASEADDR, DIP_A_CQ_THR14_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR14_DESC_SIZE_
{
		volatile struct	/* 0x150221B4 */
		{
				FIELD  CQ_THR14_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR14_DESC_SIZE;	/* DIP_X_CQ_THR14_DESC_SIZE, DIP_A_CQ_THR14_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR15_CTL_
{
		volatile struct	/* 0x150221B8 */
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
		volatile struct	/* 0x150221BC */
		{
				FIELD  CQ_THR15_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR15_BASEADDR;	/* DIP_X_CQ_THR15_BASEADDR, DIP_A_CQ_THR15_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR15_DESC_SIZE_
{
		volatile struct	/* 0x150221C0 */
		{
				FIELD  CQ_THR15_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR15_DESC_SIZE;	/* DIP_X_CQ_THR15_DESC_SIZE, DIP_A_CQ_THR15_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR16_CTL_
{
		volatile struct	/* 0x150221C4 */
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
		volatile struct	/* 0x150221C8 */
		{
				FIELD  CQ_THR16_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR16_BASEADDR;	/* DIP_X_CQ_THR16_BASEADDR, DIP_A_CQ_THR16_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR16_DESC_SIZE_
{
		volatile struct	/* 0x150221CC */
		{
				FIELD  CQ_THR16_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR16_DESC_SIZE;	/* DIP_X_CQ_THR16_DESC_SIZE, DIP_A_CQ_THR16_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR17_CTL_
{
		volatile struct	/* 0x150221D0 */
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
		volatile struct	/* 0x150221D4 */
		{
				FIELD  CQ_THR17_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR17_BASEADDR;	/* DIP_X_CQ_THR17_BASEADDR, DIP_A_CQ_THR17_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR17_DESC_SIZE_
{
		volatile struct	/* 0x150221D8 */
		{
				FIELD  CQ_THR17_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR17_DESC_SIZE;	/* DIP_X_CQ_THR17_DESC_SIZE, DIP_A_CQ_THR17_DESC_SIZE*/

typedef volatile union _DIP_X_REG_CQ_THR18_CTL_
{
		volatile struct	/* 0x150221DC */
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
		volatile struct	/* 0x150221E0 */
		{
				FIELD  CQ_THR18_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR18_BASEADDR;	/* DIP_X_CQ_THR18_BASEADDR, DIP_A_CQ_THR18_BASEADDR*/

typedef volatile union _DIP_X_REG_CQ_THR18_DESC_SIZE_
{
		volatile struct	/* 0x150221E4 */
		{
				FIELD  CQ_THR18_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CQ_THR18_DESC_SIZE;	/* DIP_X_CQ_THR18_DESC_SIZE, DIP_A_CQ_THR18_DESC_SIZE*/

typedef volatile union _DIP_X_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x15022200 */
		{
				FIELD  IMG2O_SOFT_RST_STAT                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_SOFT_RST_STAT                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_SOFT_RST_STAT                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_SOFT_RST_STAT                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_SOFT_RST_STAT                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_SOFT_RST_STAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_SOFT_RST_STAT                    :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
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
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_SOFT_RSTSTAT;	/* DIP_X_DMA_SOFT_RSTSTAT, DIP_A_DMA_SOFT_RSTSTAT*/

typedef volatile union _DIP_X_REG_TDRI_BASE_ADDR_
{
		volatile struct	/* 0x15022204 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_BASE_ADDR;	/* DIP_X_TDRI_BASE_ADDR, DIP_A_TDRI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_TDRI_OFST_ADDR_
{
		volatile struct	/* 0x15022208 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_OFST_ADDR;	/* DIP_X_TDRI_OFST_ADDR, DIP_A_TDRI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_TDRI_XSIZE_
{
		volatile struct	/* 0x1502220C */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_TDRI_XSIZE;	/* DIP_X_TDRI_XSIZE, DIP_A_TDRI_XSIZE*/

typedef volatile union _DIP_X_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x15022210 */
		{
				FIELD  IMG2O_V_FLIP_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_V_FLIP_EN                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_V_FLIP_EN                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_V_FLIP_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_V_FLIP_EN                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_V_FLIP_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_V_FLIP_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
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
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VERTICAL_FLIP_EN;	/* DIP_X_VERTICAL_FLIP_EN, DIP_A_VERTICAL_FLIP_EN*/

typedef volatile union _DIP_X_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x15022214 */
		{
				FIELD  IMG2O_SOFT_RST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_SOFT_RST                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_SOFT_RST                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_SOFT_RST                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_SOFT_RST                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_SOFT_RST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_SOFT_RST                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
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
				FIELD  rsv_26                                :  5;		/* 26..30, 0x7C000000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_SOFT_RESET;	/* DIP_X_DMA_SOFT_RESET, DIP_A_DMA_SOFT_RESET*/

typedef volatile union _DIP_X_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x15022218 */
		{
				FIELD  IMG2O_LAST_ULTRA_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_LAST_ULTRA_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_LAST_ULTRA_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_LAST_ULTRA_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_LAST_ULTRA_EN                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_LAST_ULTRA_EN                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_LAST_ULTRA_EN                    :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LAST_ULTRA_EN;	/* DIP_X_LAST_ULTRA_EN, DIP_A_LAST_ULTRA_EN*/

typedef volatile union _DIP_X_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1502221C */
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
		volatile struct	/* 0x15022230 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_BASE_ADDR;	/* DIP_X_IMG2O_BASE_ADDR, DIP_A_IMG2O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG2O_OFST_ADDR_
{
		volatile struct	/* 0x15022238 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_OFST_ADDR;	/* DIP_X_IMG2O_OFST_ADDR, DIP_A_IMG2O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG2O_XSIZE_
{
		volatile struct	/* 0x15022240 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_XSIZE;	/* DIP_X_IMG2O_XSIZE, DIP_A_IMG2O_XSIZE*/

typedef volatile union _DIP_X_REG_IMG2O_YSIZE_
{
		volatile struct	/* 0x15022244 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_YSIZE;	/* DIP_X_IMG2O_YSIZE, DIP_A_IMG2O_YSIZE*/

typedef volatile union _DIP_X_REG_IMG2O_STRIDE_
{
		volatile struct	/* 0x15022248 */
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
		volatile struct	/* 0x1502224C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CON;	/* DIP_X_IMG2O_CON, DIP_A_IMG2O_CON*/

typedef volatile union _DIP_X_REG_IMG2O_CON2_
{
		volatile struct	/* 0x15022250 */
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
		volatile struct	/* 0x15022254 */
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
		volatile struct	/* 0x15022258 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_CROP;	/* DIP_X_IMG2O_CROP, DIP_A_IMG2O_CROP*/

typedef volatile union _DIP_X_REG_IMG2BO_BASE_ADDR_
{
		volatile struct	/* 0x15022260 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_BASE_ADDR;	/* DIP_X_IMG2BO_BASE_ADDR, DIP_A_IMG2BO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG2BO_OFST_ADDR_
{
		volatile struct	/* 0x15022268 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_OFST_ADDR;	/* DIP_X_IMG2BO_OFST_ADDR, DIP_A_IMG2BO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG2BO_XSIZE_
{
		volatile struct	/* 0x15022270 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_XSIZE;	/* DIP_X_IMG2BO_XSIZE, DIP_A_IMG2BO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG2BO_YSIZE_
{
		volatile struct	/* 0x15022274 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_YSIZE;	/* DIP_X_IMG2BO_YSIZE, DIP_A_IMG2BO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG2BO_STRIDE_
{
		volatile struct	/* 0x15022278 */
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
		volatile struct	/* 0x1502227C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CON;	/* DIP_X_IMG2BO_CON, DIP_A_IMG2BO_CON*/

typedef volatile union _DIP_X_REG_IMG2BO_CON2_
{
		volatile struct	/* 0x15022280 */
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
		volatile struct	/* 0x15022284 */
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
		volatile struct	/* 0x15022288 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_CROP;	/* DIP_X_IMG2BO_CROP, DIP_A_IMG2BO_CROP*/

typedef volatile union _DIP_X_REG_IMG3O_BASE_ADDR_
{
		volatile struct	/* 0x15022290 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_BASE_ADDR;	/* DIP_X_IMG3O_BASE_ADDR, DIP_A_IMG3O_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3O_OFST_ADDR_
{
		volatile struct	/* 0x15022298 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_OFST_ADDR;	/* DIP_X_IMG3O_OFST_ADDR, DIP_A_IMG3O_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3O_XSIZE_
{
		volatile struct	/* 0x150222A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_XSIZE;	/* DIP_X_IMG3O_XSIZE, DIP_A_IMG3O_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3O_YSIZE_
{
		volatile struct	/* 0x150222A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_YSIZE;	/* DIP_X_IMG3O_YSIZE, DIP_A_IMG3O_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3O_STRIDE_
{
		volatile struct	/* 0x150222A8 */
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
		volatile struct	/* 0x150222AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CON;	/* DIP_X_IMG3O_CON, DIP_A_IMG3O_CON*/

typedef volatile union _DIP_X_REG_IMG3O_CON2_
{
		volatile struct	/* 0x150222B0 */
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
		volatile struct	/* 0x150222B4 */
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
		volatile struct	/* 0x150222B8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_CROP;	/* DIP_X_IMG3O_CROP, DIP_A_IMG3O_CROP*/

typedef volatile union _DIP_X_REG_IMG3BO_BASE_ADDR_
{
		volatile struct	/* 0x150222C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_BASE_ADDR;	/* DIP_X_IMG3BO_BASE_ADDR, DIP_A_IMG3BO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3BO_OFST_ADDR_
{
		volatile struct	/* 0x150222C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_OFST_ADDR;	/* DIP_X_IMG3BO_OFST_ADDR, DIP_A_IMG3BO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3BO_XSIZE_
{
		volatile struct	/* 0x150222D0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_XSIZE;	/* DIP_X_IMG3BO_XSIZE, DIP_A_IMG3BO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3BO_YSIZE_
{
		volatile struct	/* 0x150222D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_YSIZE;	/* DIP_X_IMG3BO_YSIZE, DIP_A_IMG3BO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3BO_STRIDE_
{
		volatile struct	/* 0x150222D8 */
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
		volatile struct	/* 0x150222DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CON;	/* DIP_X_IMG3BO_CON, DIP_A_IMG3BO_CON*/

typedef volatile union _DIP_X_REG_IMG3BO_CON2_
{
		volatile struct	/* 0x150222E0 */
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
		volatile struct	/* 0x150222E4 */
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
		volatile struct	/* 0x150222E8 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_CROP;	/* DIP_X_IMG3BO_CROP, DIP_A_IMG3BO_CROP*/

typedef volatile union _DIP_X_REG_IMG3CO_BASE_ADDR_
{
		volatile struct	/* 0x150222F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_BASE_ADDR;	/* DIP_X_IMG3CO_BASE_ADDR, DIP_A_IMG3CO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMG3CO_OFST_ADDR_
{
		volatile struct	/* 0x150222F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_OFST_ADDR;	/* DIP_X_IMG3CO_OFST_ADDR, DIP_A_IMG3CO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMG3CO_XSIZE_
{
		volatile struct	/* 0x15022300 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_XSIZE;	/* DIP_X_IMG3CO_XSIZE, DIP_A_IMG3CO_XSIZE*/

typedef volatile union _DIP_X_REG_IMG3CO_YSIZE_
{
		volatile struct	/* 0x15022304 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_YSIZE;	/* DIP_X_IMG3CO_YSIZE, DIP_A_IMG3CO_YSIZE*/

typedef volatile union _DIP_X_REG_IMG3CO_STRIDE_
{
		volatile struct	/* 0x15022308 */
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
		volatile struct	/* 0x1502230C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CON;	/* DIP_X_IMG3CO_CON, DIP_A_IMG3CO_CON*/

typedef volatile union _DIP_X_REG_IMG3CO_CON2_
{
		volatile struct	/* 0x15022310 */
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
		volatile struct	/* 0x15022314 */
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
		volatile struct	/* 0x15022318 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_CROP;	/* DIP_X_IMG3CO_CROP, DIP_A_IMG3CO_CROP*/

typedef volatile union _DIP_X_REG_FEO_BASE_ADDR_
{
		volatile struct	/* 0x15022320 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_BASE_ADDR;	/* DIP_X_FEO_BASE_ADDR, DIP_A_FEO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_FEO_OFST_ADDR_
{
		volatile struct	/* 0x15022328 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_OFST_ADDR;	/* DIP_X_FEO_OFST_ADDR, DIP_A_FEO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_FEO_XSIZE_
{
		volatile struct	/* 0x15022330 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_XSIZE;	/* DIP_X_FEO_XSIZE, DIP_A_FEO_XSIZE*/

typedef volatile union _DIP_X_REG_FEO_YSIZE_
{
		volatile struct	/* 0x15022334 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_YSIZE;	/* DIP_X_FEO_YSIZE, DIP_A_FEO_YSIZE*/

typedef volatile union _DIP_X_REG_FEO_STRIDE_
{
		volatile struct	/* 0x15022338 */
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
		volatile struct	/* 0x1502233C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_CON;	/* DIP_X_FEO_CON, DIP_A_FEO_CON*/

typedef volatile union _DIP_X_REG_FEO_CON2_
{
		volatile struct	/* 0x15022340 */
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
		volatile struct	/* 0x15022344 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_CON3;	/* DIP_X_FEO_CON3, DIP_A_FEO_CON3*/

typedef volatile union _DIP_X_REG_MFBO_BASE_ADDR_
{
		volatile struct	/* 0x15022350 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_BASE_ADDR;	/* DIP_X_MFBO_BASE_ADDR, DIP_A_MFBO_BASE_ADDR*/

typedef volatile union _DIP_X_REG_MFBO_OFST_ADDR_
{
		volatile struct	/* 0x15022358 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_OFST_ADDR;	/* DIP_X_MFBO_OFST_ADDR, DIP_A_MFBO_OFST_ADDR*/

typedef volatile union _DIP_X_REG_MFBO_XSIZE_
{
		volatile struct	/* 0x15022360 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_XSIZE;	/* DIP_X_MFBO_XSIZE, DIP_A_MFBO_XSIZE*/

typedef volatile union _DIP_X_REG_MFBO_YSIZE_
{
		volatile struct	/* 0x15022364 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_YSIZE;	/* DIP_X_MFBO_YSIZE, DIP_A_MFBO_YSIZE*/

typedef volatile union _DIP_X_REG_MFBO_STRIDE_
{
		volatile struct	/* 0x15022368 */
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
}DIP_X_REG_MFBO_STRIDE;	/* DIP_X_MFBO_STRIDE, DIP_A_MFBO_STRIDE*/

typedef volatile union _DIP_X_REG_MFBO_CON_
{
		volatile struct	/* 0x1502236C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_CON;	/* DIP_X_MFBO_CON, DIP_A_MFBO_CON*/

typedef volatile union _DIP_X_REG_MFBO_CON2_
{
		volatile struct	/* 0x15022370 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_CON2;	/* DIP_X_MFBO_CON2, DIP_A_MFBO_CON2*/

typedef volatile union _DIP_X_REG_MFBO_CON3_
{
		volatile struct	/* 0x15022374 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_CON3;	/* DIP_X_MFBO_CON3, DIP_A_MFBO_CON3*/

typedef volatile union _DIP_X_REG_MFBO_CROP_
{
		volatile struct	/* 0x15022378 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_CROP;	/* DIP_X_MFBO_CROP, DIP_A_MFBO_CROP*/

typedef volatile union _DIP_X_REG_IMGI_BASE_ADDR_
{
		volatile struct	/* 0x15022400 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_BASE_ADDR;	/* DIP_X_IMGI_BASE_ADDR, DIP_A_IMGI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGI_OFST_ADDR_
{
		volatile struct	/* 0x15022408 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_OFST_ADDR;	/* DIP_X_IMGI_OFST_ADDR, DIP_A_IMGI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGI_XSIZE_
{
		volatile struct	/* 0x15022410 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_XSIZE;	/* DIP_X_IMGI_XSIZE, DIP_A_IMGI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGI_YSIZE_
{
		volatile struct	/* 0x15022414 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_YSIZE;	/* DIP_X_IMGI_YSIZE, DIP_A_IMGI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGI_STRIDE_
{
		volatile struct	/* 0x15022418 */
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
		volatile struct	/* 0x1502241C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_CON;	/* DIP_X_IMGI_CON, DIP_A_IMGI_CON*/

typedef volatile union _DIP_X_REG_IMGI_CON2_
{
		volatile struct	/* 0x15022420 */
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
		volatile struct	/* 0x15022424 */
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
		volatile struct	/* 0x15022430 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_BASE_ADDR;	/* DIP_X_IMGBI_BASE_ADDR, DIP_A_IMGBI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGBI_OFST_ADDR_
{
		volatile struct	/* 0x15022438 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_OFST_ADDR;	/* DIP_X_IMGBI_OFST_ADDR, DIP_A_IMGBI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGBI_XSIZE_
{
		volatile struct	/* 0x15022440 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_XSIZE;	/* DIP_X_IMGBI_XSIZE, DIP_A_IMGBI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGBI_YSIZE_
{
		volatile struct	/* 0x15022444 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_YSIZE;	/* DIP_X_IMGBI_YSIZE, DIP_A_IMGBI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGBI_STRIDE_
{
		volatile struct	/* 0x15022448 */
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
		volatile struct	/* 0x1502244C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_CON;	/* DIP_X_IMGBI_CON, DIP_A_IMGBI_CON*/

typedef volatile union _DIP_X_REG_IMGBI_CON2_
{
		volatile struct	/* 0x15022450 */
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
		volatile struct	/* 0x15022454 */
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
		volatile struct	/* 0x15022460 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_BASE_ADDR;	/* DIP_X_IMGCI_BASE_ADDR, DIP_A_IMGCI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_IMGCI_OFST_ADDR_
{
		volatile struct	/* 0x15022468 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_OFST_ADDR;	/* DIP_X_IMGCI_OFST_ADDR, DIP_A_IMGCI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_IMGCI_XSIZE_
{
		volatile struct	/* 0x15022470 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_XSIZE;	/* DIP_X_IMGCI_XSIZE, DIP_A_IMGCI_XSIZE*/

typedef volatile union _DIP_X_REG_IMGCI_YSIZE_
{
		volatile struct	/* 0x15022474 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_YSIZE;	/* DIP_X_IMGCI_YSIZE, DIP_A_IMGCI_YSIZE*/

typedef volatile union _DIP_X_REG_IMGCI_STRIDE_
{
		volatile struct	/* 0x15022478 */
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
		volatile struct	/* 0x1502247C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_CON;	/* DIP_X_IMGCI_CON, DIP_A_IMGCI_CON*/

typedef volatile union _DIP_X_REG_IMGCI_CON2_
{
		volatile struct	/* 0x15022480 */
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
		volatile struct	/* 0x15022484 */
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
		volatile struct	/* 0x15022490 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_BASE_ADDR;	/* DIP_X_VIPI_BASE_ADDR, DIP_A_VIPI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIPI_OFST_ADDR_
{
		volatile struct	/* 0x15022498 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_OFST_ADDR;	/* DIP_X_VIPI_OFST_ADDR, DIP_A_VIPI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIPI_XSIZE_
{
		volatile struct	/* 0x150224A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_XSIZE;	/* DIP_X_VIPI_XSIZE, DIP_A_VIPI_XSIZE*/

typedef volatile union _DIP_X_REG_VIPI_YSIZE_
{
		volatile struct	/* 0x150224A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_YSIZE;	/* DIP_X_VIPI_YSIZE, DIP_A_VIPI_YSIZE*/

typedef volatile union _DIP_X_REG_VIPI_STRIDE_
{
		volatile struct	/* 0x150224A8 */
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
		volatile struct	/* 0x150224AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_CON;	/* DIP_X_VIPI_CON, DIP_A_VIPI_CON*/

typedef volatile union _DIP_X_REG_VIPI_CON2_
{
		volatile struct	/* 0x150224B0 */
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
		volatile struct	/* 0x150224B4 */
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
		volatile struct	/* 0x150224C0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_BASE_ADDR;	/* DIP_X_VIP2I_BASE_ADDR, DIP_A_VIP2I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIP2I_OFST_ADDR_
{
		volatile struct	/* 0x150224C8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_OFST_ADDR;	/* DIP_X_VIP2I_OFST_ADDR, DIP_A_VIP2I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIP2I_XSIZE_
{
		volatile struct	/* 0x150224D0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_XSIZE;	/* DIP_X_VIP2I_XSIZE, DIP_A_VIP2I_XSIZE*/

typedef volatile union _DIP_X_REG_VIP2I_YSIZE_
{
		volatile struct	/* 0x150224D4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_YSIZE;	/* DIP_X_VIP2I_YSIZE, DIP_A_VIP2I_YSIZE*/

typedef volatile union _DIP_X_REG_VIP2I_STRIDE_
{
		volatile struct	/* 0x150224D8 */
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
		volatile struct	/* 0x150224DC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_CON;	/* DIP_X_VIP2I_CON, DIP_A_VIP2I_CON*/

typedef volatile union _DIP_X_REG_VIP2I_CON2_
{
		volatile struct	/* 0x150224E0 */
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
		volatile struct	/* 0x150224E4 */
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
		volatile struct	/* 0x150224F0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_BASE_ADDR;	/* DIP_X_VIP3I_BASE_ADDR, DIP_A_VIP3I_BASE_ADDR*/

typedef volatile union _DIP_X_REG_VIP3I_OFST_ADDR_
{
		volatile struct	/* 0x150224F8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_OFST_ADDR;	/* DIP_X_VIP3I_OFST_ADDR, DIP_A_VIP3I_OFST_ADDR*/

typedef volatile union _DIP_X_REG_VIP3I_XSIZE_
{
		volatile struct	/* 0x15022500 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_XSIZE;	/* DIP_X_VIP3I_XSIZE, DIP_A_VIP3I_XSIZE*/

typedef volatile union _DIP_X_REG_VIP3I_YSIZE_
{
		volatile struct	/* 0x15022504 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_YSIZE;	/* DIP_X_VIP3I_YSIZE, DIP_A_VIP3I_YSIZE*/

typedef volatile union _DIP_X_REG_VIP3I_STRIDE_
{
		volatile struct	/* 0x15022508 */
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
		volatile struct	/* 0x1502250C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_CON;	/* DIP_X_VIP3I_CON, DIP_A_VIP3I_CON*/

typedef volatile union _DIP_X_REG_VIP3I_CON2_
{
		volatile struct	/* 0x15022510 */
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
		volatile struct	/* 0x15022514 */
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
		volatile struct	/* 0x15022520 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_BASE_ADDR;	/* DIP_X_DMGI_BASE_ADDR, DIP_A_DMGI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_DMGI_OFST_ADDR_
{
		volatile struct	/* 0x15022528 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_OFST_ADDR;	/* DIP_X_DMGI_OFST_ADDR, DIP_A_DMGI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_DMGI_XSIZE_
{
		volatile struct	/* 0x15022530 */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_XSIZE;	/* DIP_X_DMGI_XSIZE, DIP_A_DMGI_XSIZE*/

typedef volatile union _DIP_X_REG_DMGI_YSIZE_
{
		volatile struct	/* 0x15022534 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_YSIZE;	/* DIP_X_DMGI_YSIZE, DIP_A_DMGI_YSIZE*/

typedef volatile union _DIP_X_REG_DMGI_STRIDE_
{
		volatile struct	/* 0x15022538 */
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
		volatile struct	/* 0x1502253C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_CON;	/* DIP_X_DMGI_CON, DIP_A_DMGI_CON*/

typedef volatile union _DIP_X_REG_DMGI_CON2_
{
		volatile struct	/* 0x15022540 */
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
		volatile struct	/* 0x15022544 */
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
		volatile struct	/* 0x15022550 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_BASE_ADDR;	/* DIP_X_DEPI_BASE_ADDR, DIP_A_DEPI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_DEPI_OFST_ADDR_
{
		volatile struct	/* 0x15022558 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_OFST_ADDR;	/* DIP_X_DEPI_OFST_ADDR, DIP_A_DEPI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_DEPI_XSIZE_
{
		volatile struct	/* 0x15022560 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_XSIZE;	/* DIP_X_DEPI_XSIZE, DIP_A_DEPI_XSIZE*/

typedef volatile union _DIP_X_REG_DEPI_YSIZE_
{
		volatile struct	/* 0x15022564 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_YSIZE;	/* DIP_X_DEPI_YSIZE, DIP_A_DEPI_YSIZE*/

typedef volatile union _DIP_X_REG_DEPI_STRIDE_
{
		volatile struct	/* 0x15022568 */
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
		volatile struct	/* 0x1502256C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_CON;	/* DIP_X_DEPI_CON, DIP_A_DEPI_CON*/

typedef volatile union _DIP_X_REG_DEPI_CON2_
{
		volatile struct	/* 0x15022570 */
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
		volatile struct	/* 0x15022574 */
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
		volatile struct	/* 0x15022580 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_BASE_ADDR;	/* DIP_X_LCEI_BASE_ADDR, DIP_A_LCEI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_LCEI_OFST_ADDR_
{
		volatile struct	/* 0x15022588 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_OFST_ADDR;	/* DIP_X_LCEI_OFST_ADDR, DIP_A_LCEI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_LCEI_XSIZE_
{
		volatile struct	/* 0x15022590 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_XSIZE;	/* DIP_X_LCEI_XSIZE, DIP_A_LCEI_XSIZE*/

typedef volatile union _DIP_X_REG_LCEI_YSIZE_
{
		volatile struct	/* 0x15022594 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_YSIZE;	/* DIP_X_LCEI_YSIZE, DIP_A_LCEI_YSIZE*/

typedef volatile union _DIP_X_REG_LCEI_STRIDE_
{
		volatile struct	/* 0x15022598 */
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
		volatile struct	/* 0x1502259C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_CON;	/* DIP_X_LCEI_CON, DIP_A_LCEI_CON*/

typedef volatile union _DIP_X_REG_LCEI_CON2_
{
		volatile struct	/* 0x150225A0 */
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
		volatile struct	/* 0x150225A4 */
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
		volatile struct	/* 0x150225B0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_BASE_ADDR;	/* DIP_X_UFDI_BASE_ADDR, DIP_A_UFDI_BASE_ADDR*/

typedef volatile union _DIP_X_REG_UFDI_OFST_ADDR_
{
		volatile struct	/* 0x150225B8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_OFST_ADDR;	/* DIP_X_UFDI_OFST_ADDR, DIP_A_UFDI_OFST_ADDR*/

typedef volatile union _DIP_X_REG_UFDI_XSIZE_
{
		volatile struct	/* 0x150225C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_XSIZE;	/* DIP_X_UFDI_XSIZE, DIP_A_UFDI_XSIZE*/

typedef volatile union _DIP_X_REG_UFDI_YSIZE_
{
		volatile struct	/* 0x150225C4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_YSIZE;	/* DIP_X_UFDI_YSIZE, DIP_A_UFDI_YSIZE*/

typedef volatile union _DIP_X_REG_UFDI_STRIDE_
{
		volatile struct	/* 0x150225C8 */
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
		volatile struct	/* 0x150225CC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_CON;	/* DIP_X_UFDI_CON, DIP_A_UFDI_CON*/

typedef volatile union _DIP_X_REG_UFDI_CON2_
{
		volatile struct	/* 0x150225D0 */
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
		volatile struct	/* 0x150225D4 */
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
		volatile struct	/* 0x15022640 */
		{
				FIELD  IMG2O_ERR                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMG2BO_ERR                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  IMG3O_ERR                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  IMG3BO_ERR                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMG3CO_ERR                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FEO_ERR                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  MFBO_ERR                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
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
				FIELD  rsv_26                                :  5;		/* 26..30, 0x7C000000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_ERR_CTRL;	/* DIP_X_DMA_ERR_CTRL, DIP_A_DMA_ERR_CTRL*/

typedef volatile union _DIP_X_REG_IMG2O_ERR_STAT_
{
		volatile struct	/* 0x15022644 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2O_ERR_STAT;	/* DIP_X_IMG2O_ERR_STAT, DIP_A_IMG2O_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG2BO_ERR_STAT_
{
		volatile struct	/* 0x15022648 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG2BO_ERR_STAT;	/* DIP_X_IMG2BO_ERR_STAT, DIP_A_IMG2BO_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3O_ERR_STAT_
{
		volatile struct	/* 0x1502264C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3O_ERR_STAT;	/* DIP_X_IMG3O_ERR_STAT, DIP_A_IMG3O_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3BO_ERR_STAT_
{
		volatile struct	/* 0x15022650 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3BO_ERR_STAT;	/* DIP_X_IMG3BO_ERR_STAT, DIP_A_IMG3BO_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMG3CO_ERR_STAT_
{
		volatile struct	/* 0x15022654 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMG3CO_ERR_STAT;	/* DIP_X_IMG3CO_ERR_STAT, DIP_A_IMG3CO_ERR_STAT*/

typedef volatile union _DIP_X_REG_FEO_ERR_STAT_
{
		volatile struct	/* 0x15022658 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FEO_ERR_STAT;	/* DIP_X_FEO_ERR_STAT, DIP_A_FEO_ERR_STAT*/

typedef volatile union _DIP_X_REG_MFBO_ERR_STAT_
{
		volatile struct	/* 0x1502265C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFBO_ERR_STAT;	/* DIP_X_MFBO_ERR_STAT, DIP_A_MFBO_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGI_ERR_STAT_
{
		volatile struct	/* 0x15022660 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGI_ERR_STAT;	/* DIP_X_IMGI_ERR_STAT, DIP_A_IMGI_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGBI_ERR_STAT_
{
		volatile struct	/* 0x15022664 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGBI_ERR_STAT;	/* DIP_X_IMGBI_ERR_STAT, DIP_A_IMGBI_ERR_STAT*/

typedef volatile union _DIP_X_REG_IMGCI_ERR_STAT_
{
		volatile struct	/* 0x15022668 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_IMGCI_ERR_STAT;	/* DIP_X_IMGCI_ERR_STAT, DIP_A_IMGCI_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIPI_ERR_STAT_
{
		volatile struct	/* 0x1502266C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIPI_ERR_STAT;	/* DIP_X_VIPI_ERR_STAT, DIP_A_VIPI_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIP2I_ERR_STAT_
{
		volatile struct	/* 0x15022670 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP2I_ERR_STAT;	/* DIP_X_VIP2I_ERR_STAT, DIP_A_VIP2I_ERR_STAT*/

typedef volatile union _DIP_X_REG_VIP3I_ERR_STAT_
{
		volatile struct	/* 0x15022674 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_VIP3I_ERR_STAT;	/* DIP_X_VIP3I_ERR_STAT, DIP_A_VIP3I_ERR_STAT*/

typedef volatile union _DIP_X_REG_DMGI_ERR_STAT_
{
		volatile struct	/* 0x15022678 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMGI_ERR_STAT;	/* DIP_X_DMGI_ERR_STAT, DIP_A_DMGI_ERR_STAT*/

typedef volatile union _DIP_X_REG_DEPI_ERR_STAT_
{
		volatile struct	/* 0x1502267C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DEPI_ERR_STAT;	/* DIP_X_DEPI_ERR_STAT, DIP_A_DEPI_ERR_STAT*/

typedef volatile union _DIP_X_REG_LCEI_ERR_STAT_
{
		volatile struct	/* 0x15022680 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCEI_ERR_STAT;	/* DIP_X_LCEI_ERR_STAT, DIP_A_LCEI_ERR_STAT*/

typedef volatile union _DIP_X_REG_UFDI_ERR_STAT_
{
		volatile struct	/* 0x15022684 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFDI_ERR_STAT;	/* DIP_X_UFDI_ERR_STAT, DIP_A_UFDI_ERR_STAT*/

typedef volatile union _DIP_X_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x150226A0 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_DEBUG_ADDR;	/* DIP_X_DMA_DEBUG_ADDR, DIP_A_DMA_DEBUG_ADDR*/

typedef volatile union _DIP_X_REG_DMA_RSV1_
{
		volatile struct	/* 0x150226A4 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV1;	/* DIP_X_DMA_RSV1, DIP_A_DMA_RSV1*/

typedef volatile union _DIP_X_REG_DMA_RSV2_
{
		volatile struct	/* 0x150226A8 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV2;	/* DIP_X_DMA_RSV2, DIP_A_DMA_RSV2*/

typedef volatile union _DIP_X_REG_DMA_RSV3_
{
		volatile struct	/* 0x150226AC */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV3;	/* DIP_X_DMA_RSV3, DIP_A_DMA_RSV3*/

typedef volatile union _DIP_X_REG_DMA_RSV4_
{
		volatile struct	/* 0x150226B0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV4;	/* DIP_X_DMA_RSV4, DIP_A_DMA_RSV4*/

typedef volatile union _DIP_X_REG_DMA_RSV5_
{
		volatile struct	/* 0x150226B4 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV5;	/* DIP_X_DMA_RSV5, DIP_A_DMA_RSV5*/

typedef volatile union _DIP_X_REG_DMA_RSV6_
{
		volatile struct	/* 0x150226B8 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_RSV6;	/* DIP_X_DMA_RSV6, DIP_A_DMA_RSV6*/

typedef volatile union _DIP_X_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x150226BC */
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
		volatile struct	/* 0x150226C0 */
		{
				FIELD  BW_SELF_TEST_EN_IMG2O                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_IMG2BO                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_IMG3O                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_IMG3BO                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  BW_SELF_TEST_EN_IMG3CO                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BW_SELF_TEST_EN_FEO                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BW_SELF_TEST_EN_MFBO                  :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DMA_BW_SELF_TEST;	/* DIP_X_DMA_BW_SELF_TEST, DIP_A_DMA_BW_SELF_TEST*/

typedef volatile union _DIP_X_REG_UFD_CON_
{
		volatile struct	/* 0x15022700 */
		{
				FIELD  UFD_TCCT_BYP                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFD_CON;	/* DIP_X_UFD_CON, DIP_A_UFD_CON*/

typedef volatile union _DIP_X_REG_UFD_SIZE_CON_
{
		volatile struct	/* 0x15022704 */
		{
				FIELD  UFD_WD                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFD_HT                                : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFD_SIZE_CON;	/* DIP_X_UFD_SIZE_CON, DIP_A_UFD_SIZE_CON*/

typedef volatile union _DIP_X_REG_UFD_AU_CON_
{
		volatile struct	/* 0x15022708 */
		{
				FIELD  UFD_AU_OFST                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UFD_AU_SIZE                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFD_AU_CON;	/* DIP_X_UFD_AU_CON, DIP_A_UFD_AU_CON*/

typedef volatile union _DIP_X_REG_UFD_CROP_CON1_
{
		volatile struct	/* 0x1502270C */
		{
				FIELD  UFD_X_START                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFD_X_END                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFD_CROP_CON1;	/* DIP_X_UFD_CROP_CON1, DIP_A_UFD_CROP_CON1*/

typedef volatile union _DIP_X_REG_UFD_CROP_CON2_
{
		volatile struct	/* 0x15022710 */
		{
				FIELD  UFD_Y_START                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  UFD_Y_END                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UFD_CROP_CON2;	/* DIP_X_UFD_CROP_CON2, DIP_A_UFD_CROP_CON2*/

typedef volatile union _DIP_X_REG_PGN_SATU_1_
{
		volatile struct	/* 0x15022720 */
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
		volatile struct	/* 0x15022724 */
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
		volatile struct	/* 0x15022728 */
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
		volatile struct	/* 0x1502272C */
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
		volatile struct	/* 0x15022730 */
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
		volatile struct	/* 0x15022734 */
		{
				FIELD  PGN_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PGN_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PGN_OFST_2;	/* DIP_X_PGN_OFST_2, DIP_A_PGN_OFST_2*/

typedef volatile union _DIP_X_REG_SL2_CEN_
{
		volatile struct	/* 0x15022740 */
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
		volatile struct	/* 0x15022744 */
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
		volatile struct	/* 0x15022748 */
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
		volatile struct	/* 0x1502274C */
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
		volatile struct	/* 0x15022750 */
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
		volatile struct	/* 0x15022754 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_XOFF;	/* DIP_X_SL2_XOFF, DIP_A_SL2_XOFF*/

typedef volatile union _DIP_X_REG_SL2_YOFF_
{
		volatile struct	/* 0x15022758 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_YOFF;	/* DIP_X_SL2_YOFF, DIP_A_SL2_YOFF*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON0_
{
		volatile struct	/* 0x1502275C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON0;	/* DIP_X_SL2_SLP_CON0, DIP_A_SL2_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON1_
{
		volatile struct	/* 0x15022760 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON1;	/* DIP_X_SL2_SLP_CON1, DIP_A_SL2_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON2_
{
		volatile struct	/* 0x15022764 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON2;	/* DIP_X_SL2_SLP_CON2, DIP_A_SL2_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2_SLP_CON3_
{
		volatile struct	/* 0x15022768 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SLP_CON3;	/* DIP_X_SL2_SLP_CON3, DIP_A_SL2_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2_SIZE_
{
		volatile struct	/* 0x1502276C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2_SIZE;	/* DIP_X_SL2_SIZE, DIP_A_SL2_SIZE*/

typedef volatile union _DIP_X_REG_UDM_INTP_CRS_
{
		volatile struct	/* 0x15022780 */
		{
				FIELD  UDM_DTH                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  UDM_FTH                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_ETH                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  UDM_GRAD_WT                           :  6;		/* 24..29, 0x3F000000 */
				FIELD  UDM_BYP                               :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_INTP_CRS;	/* DIP_X_UDM_INTP_CRS, DIP_A_UDM_INTP_CRS*/

typedef volatile union _DIP_X_REG_UDM_INTP_NAT_
{
		volatile struct	/* 0x15022784 */
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
		volatile struct	/* 0x15022788 */
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
		volatile struct	/* 0x1502278C */
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
		volatile struct	/* 0x15022790 */
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
		volatile struct	/* 0x15022794 */
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
		volatile struct	/* 0x15022798 */
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
		volatile struct	/* 0x1502279C */
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
		volatile struct	/* 0x150227A0 */
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
		volatile struct	/* 0x150227A4 */
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
		volatile struct	/* 0x150227A8 */
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
		volatile struct	/* 0x150227AC */
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
		volatile struct	/* 0x150227B0 */
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
		volatile struct	/* 0x150227B4 */
		{
				FIELD  UDM_SC_RAT                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  UDM_SL_RAT                            :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  UDM_FL_MODE                           :  1;		/* 10..10, 0x00000400 */
				FIELD  UDM_DS_SLB                            :  4;		/* 11..14, 0x00007800 */
				FIELD  UDM_DS_THB                            :  8;		/* 15..22, 0x007F8000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_DSB;	/* DIP_X_UDM_DSB, DIP_A_UDM_DSB*/

typedef volatile union _DIP_X_REG_UDM_TILE_EDGE_
{
		volatile struct	/* 0x150227B8 */
		{
				FIELD  UDM_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_TILE_EDGE;	/* DIP_X_UDM_TILE_EDGE, DIP_A_UDM_TILE_EDGE*/

typedef volatile union _DIP_X_REG_UDM_DSL_
{
		volatile struct	/* 0x150227BC */
		{
				FIELD  UDM_DS_SLL                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  UDM_DS_SLC                            :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  UDM_DS_THL                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  UDM_DS_THC                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_DSL;	/* DIP_X_UDM_DSL, DIP_A_UDM_DSL*/

typedef volatile union _DIP_X_REG_UDM_LR_RAT_
{
		volatile struct	/* 0x150227C0 */
		{
				FIELD  UDM_LR_RAT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_LR_RAT;	/* DIP_X_UDM_LR_RAT, DIP_A_UDM_LR_RAT*/

typedef volatile union _DIP_X_REG_UDM_SPARE_2_
{
		volatile struct	/* 0x150227C4 */
		{
				FIELD  UDM_SPARE_2                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_SPARE_2;	/* DIP_X_UDM_SPARE_2, DIP_A_UDM_SPARE_2*/

typedef volatile union _DIP_X_REG_UDM_SPARE_3_
{
		volatile struct	/* 0x150227C8 */
		{
				FIELD  UDM_SPARE_3                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UDM_SPARE_3;	/* DIP_X_UDM_SPARE_3, DIP_A_UDM_SPARE_3*/

typedef volatile union _DIP_X_REG_G2G_CNV_1_
{
		volatile struct	/* 0x150227F0 */
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
		volatile struct	/* 0x150227F4 */
		{
				FIELD  G2G_CNV_02                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_2;	/* DIP_X_G2G_CNV_2, DIP_A_G2G_CNV_2*/

typedef volatile union _DIP_X_REG_G2G_CNV_3_
{
		volatile struct	/* 0x150227F8 */
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
		volatile struct	/* 0x150227FC */
		{
				FIELD  G2G_CNV_12                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_4;	/* DIP_X_G2G_CNV_4, DIP_A_G2G_CNV_4*/

typedef volatile union _DIP_X_REG_G2G_CNV_5_
{
		volatile struct	/* 0x15022800 */
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
		volatile struct	/* 0x15022804 */
		{
				FIELD  G2G_CNV_22                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2G_CNV_6;	/* DIP_X_G2G_CNV_6, DIP_A_G2G_CNV_6*/

typedef volatile union _DIP_X_REG_G2G_CTRL_
{
		volatile struct	/* 0x15022808 */
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
		volatile struct	/* 0x1502280C */
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

typedef volatile union _DIP_X_REG_G2C_CONV_0A_
{
		volatile struct	/* 0x15022840 */
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
		volatile struct	/* 0x15022844 */
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
		volatile struct	/* 0x15022848 */
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
		volatile struct	/* 0x1502284C */
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
		volatile struct	/* 0x15022850 */
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
		volatile struct	/* 0x15022854 */
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
		volatile struct	/* 0x15022858 */
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
		volatile struct	/* 0x1502285C */
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
		volatile struct	/* 0x15022860 */
		{
				FIELD  G2C_SHADE_UB                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_G2C_SHADE_CON_3;	/* DIP_X_G2C_SHADE_CON_3, DIP_A_G2C_SHADE_CON_3*/

typedef volatile union _DIP_X_REG_G2C_SHADE_TAR_
{
		volatile struct	/* 0x15022864 */
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
		volatile struct	/* 0x15022868 */
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
		volatile struct	/* 0x1502286C */
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
		volatile struct	/* 0x15022870 */
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

typedef volatile union _DIP_X_REG_C42_CON_
{
		volatile struct	/* 0x15022880 */
		{
				FIELD  C42_FILT_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  C42_TDR_EDGE                          :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C42_CON;	/* DIP_X_C42_CON, DIP_A_C42_CON*/

typedef volatile union _DIP_X_REG_SRZ3_CONTROL_
{
		volatile struct	/* 0x15022890 */
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
		volatile struct	/* 0x15022894 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_IN_IMG;	/* DIP_X_SRZ3_IN_IMG, DIP_A_SRZ3_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ3_OUT_IMG_
{
		volatile struct	/* 0x15022898 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_OUT_IMG;	/* DIP_X_SRZ3_OUT_IMG, DIP_A_SRZ3_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_STEP_
{
		volatile struct	/* 0x1502289C */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_STEP;	/* DIP_X_SRZ3_HORI_STEP, DIP_A_SRZ3_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_STEP_
{
		volatile struct	/* 0x150228A0 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_STEP;	/* DIP_X_SRZ3_VERT_STEP, DIP_A_SRZ3_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_INT_OFST_
{
		volatile struct	/* 0x150228A4 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_INT_OFST;	/* DIP_X_SRZ3_HORI_INT_OFST, DIP_A_SRZ3_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_HORI_SUB_OFST_
{
		volatile struct	/* 0x150228A8 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_HORI_SUB_OFST;	/* DIP_X_SRZ3_HORI_SUB_OFST, DIP_A_SRZ3_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_INT_OFST_
{
		volatile struct	/* 0x150228AC */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_INT_OFST;	/* DIP_X_SRZ3_VERT_INT_OFST, DIP_A_SRZ3_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ3_VERT_SUB_OFST_
{
		volatile struct	/* 0x150228B0 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ3_VERT_SUB_OFST;	/* DIP_X_SRZ3_VERT_SUB_OFST, DIP_A_SRZ3_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_CONTROL_
{
		volatile struct	/* 0x150228C0 */
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
		volatile struct	/* 0x150228C4 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_IN_IMG;	/* DIP_X_SRZ4_IN_IMG, DIP_A_SRZ4_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ4_OUT_IMG_
{
		volatile struct	/* 0x150228C8 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_OUT_IMG;	/* DIP_X_SRZ4_OUT_IMG, DIP_A_SRZ4_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_STEP_
{
		volatile struct	/* 0x150228CC */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_STEP;	/* DIP_X_SRZ4_HORI_STEP, DIP_A_SRZ4_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_STEP_
{
		volatile struct	/* 0x150228D0 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_STEP;	/* DIP_X_SRZ4_VERT_STEP, DIP_A_SRZ4_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_INT_OFST_
{
		volatile struct	/* 0x150228D4 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_INT_OFST;	/* DIP_X_SRZ4_HORI_INT_OFST, DIP_A_SRZ4_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_HORI_SUB_OFST_
{
		volatile struct	/* 0x150228D8 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_HORI_SUB_OFST;	/* DIP_X_SRZ4_HORI_SUB_OFST, DIP_A_SRZ4_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_INT_OFST_
{
		volatile struct	/* 0x150228DC */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_INT_OFST;	/* DIP_X_SRZ4_VERT_INT_OFST, DIP_A_SRZ4_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ4_VERT_SUB_OFST_
{
		volatile struct	/* 0x150228E0 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ4_VERT_SUB_OFST;	/* DIP_X_SRZ4_VERT_SUB_OFST, DIP_A_SRZ4_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ5_CONTROL_
{
		volatile struct	/* 0x150228F0 */
		{
				FIELD  SRZ_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SRZ_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SRZ_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SRZ_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_CONTROL;	/* DIP_X_SRZ5_CONTROL, DIP_A_SRZ5_CONTROL*/

typedef volatile union _DIP_X_REG_SRZ5_IN_IMG_
{
		volatile struct	/* 0x150228F4 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_IN_IMG;	/* DIP_X_SRZ5_IN_IMG, DIP_A_SRZ5_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ5_OUT_IMG_
{
		volatile struct	/* 0x150228F8 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_OUT_IMG;	/* DIP_X_SRZ5_OUT_IMG, DIP_A_SRZ5_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ5_HORI_STEP_
{
		volatile struct	/* 0x150228FC */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_HORI_STEP;	/* DIP_X_SRZ5_HORI_STEP, DIP_A_SRZ5_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ5_VERT_STEP_
{
		volatile struct	/* 0x15022900 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_VERT_STEP;	/* DIP_X_SRZ5_VERT_STEP, DIP_A_SRZ5_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ5_HORI_INT_OFST_
{
		volatile struct	/* 0x15022904 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_HORI_INT_OFST;	/* DIP_X_SRZ5_HORI_INT_OFST, DIP_A_SRZ5_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ5_HORI_SUB_OFST_
{
		volatile struct	/* 0x15022908 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_HORI_SUB_OFST;	/* DIP_X_SRZ5_HORI_SUB_OFST, DIP_A_SRZ5_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ5_VERT_INT_OFST_
{
		volatile struct	/* 0x1502290C */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_VERT_INT_OFST;	/* DIP_X_SRZ5_VERT_INT_OFST, DIP_A_SRZ5_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ5_VERT_SUB_OFST_
{
		volatile struct	/* 0x15022910 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ5_VERT_SUB_OFST;	/* DIP_X_SRZ5_VERT_SUB_OFST, DIP_A_SRZ5_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_SEEE_CTRL_
{
		volatile struct	/* 0x15022990 */
		{
				FIELD  SEEE_TILE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 24;		/*  4..27, 0x0FFFFFF0 */
				FIELD  RESERVED                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CTRL;	/* DIP_X_SEEE_CTRL, DIP_A_SEEE_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_CLIP_CTRL_1_
{
		volatile struct	/* 0x15022994 */
		{
				FIELD  SEEE_DOT_REDUC_AMNT                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_DOT_TH                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CLIP_CTRL_1;	/* DIP_X_SEEE_CLIP_CTRL_1, DIP_A_SEEE_CLIP_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_CLIP_CTRL_2_
{
		volatile struct	/* 0x15022998 */
		{
				FIELD  SEEE_RESP_CLIP                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_RESP_CLIP_LUMA_SPC_TH            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_RESP_CLIP_LUMA_LWB               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_RESP_CLIP_LUMA_UPB               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CLIP_CTRL_2;	/* DIP_X_SEEE_CLIP_CTRL_2, DIP_A_SEEE_CLIP_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_CLIP_CTRL_3_
{
		volatile struct	/* 0x1502299C */
		{
				FIELD  SEEE_OVRSH_CLIP_STR                   :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  SEEE_FLT_CORE_TH                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_CLIP_CTRL_3;	/* DIP_X_SEEE_CLIP_CTRL_3, DIP_A_SEEE_CLIP_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_BLND_CTRL_1_
{
		volatile struct	/* 0x150229A0 */
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
		volatile struct	/* 0x150229A4 */
		{
				FIELD  SEEE_H1_DI_BLND_SL                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_H2_DI_BLND_SL                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_H3_DI_BLND_SL                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SEEE_HX_ISO_BLND_RAT                  :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_BLND_CTRL_2;	/* DIP_X_SEEE_BLND_CTRL_2, DIP_A_SEEE_BLND_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_GN_CTRL_
{
		volatile struct	/* 0x150229A8 */
		{
				FIELD  SEEE_H1_GN                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SEEE_H2_GN                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEEE_H3_GN                            :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SEEE_H4_GN                            :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GN_CTRL;	/* DIP_X_SEEE_GN_CTRL, DIP_A_SEEE_GN_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_1_
{
		volatile struct	/* 0x150229AC */
		{
				FIELD  SEEE_LUMA_MOD_Y0                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  SEEE_LUMA_MOD_Y1                      :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_1;	/* DIP_X_SEEE_LUMA_CTRL_1, DIP_A_SEEE_LUMA_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_2_
{
		volatile struct	/* 0x150229B0 */
		{
				FIELD  SEEE_LUMA_MOD_Y2                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  SEEE_LUMA_MOD_Y3                      :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_2;	/* DIP_X_SEEE_LUMA_CTRL_2, DIP_A_SEEE_LUMA_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_3_
{
		volatile struct	/* 0x150229B4 */
		{
				FIELD  SEEE_LUMA_MOD_Y4                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  SEEE_LUMA_MOD_Y5                      :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_3;	/* DIP_X_SEEE_LUMA_CTRL_3, DIP_A_SEEE_LUMA_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_LUMA_CTRL_4_
{
		volatile struct	/* 0x150229B8 */
		{
				FIELD  SEEE_LUMA_MOD_Y6                      :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_LUMA_CTRL_4;	/* DIP_X_SEEE_LUMA_CTRL_4, DIP_A_SEEE_LUMA_CTRL_4*/

typedef volatile union _DIP_X_REG_SEEE_SLNK_CTRL_1_
{
		volatile struct	/* 0x150229BC */
		{
				FIELD  SEEE_SLNK_GN_Y1                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_SLNK_GN_Y2                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SLNK_CTRL_1;	/* DIP_X_SEEE_SLNK_CTRL_1, DIP_A_SEEE_SLNK_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_SLNK_CTRL_2_
{
		volatile struct	/* 0x150229C0 */
		{
				FIELD  SEEE_GLUT_SL_DEC_Y                    : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  SEEE_GLUT_LINK_EN                     :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEEE_RESP_SLNK_GN_RAT                 :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SLNK_CTRL_2;	/* DIP_X_SEEE_SLNK_CTRL_2, DIP_A_SEEE_SLNK_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_1_
{
		volatile struct	/* 0x150229C4 */
		{
				FIELD  SEEE_GLUT_X1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y1                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_1;	/* DIP_X_SEEE_GLUT_CTRL_1, DIP_A_SEEE_GLUT_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_2_
{
		volatile struct	/* 0x150229C8 */
		{
				FIELD  SEEE_GLUT_X2                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y2                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_2;	/* DIP_X_SEEE_GLUT_CTRL_2, DIP_A_SEEE_GLUT_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_3_
{
		volatile struct	/* 0x150229CC */
		{
				FIELD  SEEE_GLUT_X3                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S3                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y3                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_3;	/* DIP_X_SEEE_GLUT_CTRL_3, DIP_A_SEEE_GLUT_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_4_
{
		volatile struct	/* 0x150229D0 */
		{
				FIELD  SEEE_GLUT_X4                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S4                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_Y4                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_4;	/* DIP_X_SEEE_GLUT_CTRL_4, DIP_A_SEEE_GLUT_CTRL_4*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_5_
{
		volatile struct	/* 0x150229D4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_S5                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_5;	/* DIP_X_SEEE_GLUT_CTRL_5, DIP_A_SEEE_GLUT_CTRL_5*/

typedef volatile union _DIP_X_REG_SEEE_GLUT_CTRL_6_
{
		volatile struct	/* 0x150229D8 */
		{
				FIELD  SEEE_GLUT_TH_OVR                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SEEE_GLUT_TH_UND                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_GLUT_TH_MIN                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_GLUT_CTRL_6;	/* DIP_X_SEEE_GLUT_CTRL_6, DIP_A_SEEE_GLUT_CTRL_6*/

typedef volatile union _DIP_X_REG_SEEE_OUT_EDGE_CTRL_
{
		volatile struct	/* 0x150229DC */
		{
				FIELD  SEEE_OUT_EDGE_SEL                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_OUT_EDGE_CTRL;	/* DIP_X_SEEE_OUT_EDGE_CTRL, DIP_A_SEEE_OUT_EDGE_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_SE_Y_CTRL_
{
		volatile struct	/* 0x150229E0 */
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
}DIP_X_REG_SEEE_SE_Y_CTRL;	/* DIP_X_SEEE_SE_Y_CTRL, DIP_A_SEEE_SE_Y_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_SE_EDGE_CTRL_1_
{
		volatile struct	/* 0x150229E4 */
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
		volatile struct	/* 0x150229E8 */
		{
				FIELD  SEEE_SE_EDGE_DET_GAIN                 :  5;		/*  0.. 4, 0x0000001F */
				FIELD  SEEE_SE_BOSS_IN_SEL                   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SEEE_SE_BOSS_GAIN_OFF                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_EDGE_CTRL_2;	/* DIP_X_SEEE_SE_EDGE_CTRL_2, DIP_A_SEEE_SE_EDGE_CTRL_2*/

typedef volatile union _DIP_X_REG_SEEE_SE_EDGE_CTRL_3_
{
		volatile struct	/* 0x150229EC */
		{
				FIELD  SEEE_SE_CONVT_FORCE_EN                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SEEE_SE_CONVT_CORE_TH                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SEEE_SE_CONVT_E_TH                    :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SEEE_SE_CONVT_SLOPE_SEL               :  1;		/* 16..16, 0x00010000 */
				FIELD  SEEE_SE_OIL_EN                        :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_EDGE_CTRL_3;	/* DIP_X_SEEE_SE_EDGE_CTRL_3, DIP_A_SEEE_SE_EDGE_CTRL_3*/

typedef volatile union _DIP_X_REG_SEEE_SE_SPECL_CTRL_
{
		volatile struct	/* 0x150229F0 */
		{
				FIELD  SEEE_SE_SPECL_HALF_MODE               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  SEEE_SE_SPECL_ABS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SEEE_SE_SPECL_INV                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SEEE_SE_SPECL_GAIN                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  SEEE_SE_KNEE_SEL                      :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_SPECL_CTRL;	/* DIP_X_SEEE_SE_SPECL_CTRL, DIP_A_SEEE_SE_SPECL_CTRL*/

typedef volatile union _DIP_X_REG_SEEE_SE_CORE_CTRL_1_
{
		volatile struct	/* 0x150229F4 */
		{
				FIELD  SEEE_SE_CORE_HORI_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  SEEE_SE_CORE_HORI_SUP                 :  2;		/*  7.. 8, 0x00000180 */
				FIELD  SEEE_SE_CORE_HORI_X2                  :  8;		/*  9..16, 0x0001FE00 */
				FIELD  SEEE_SE_CORE_HORI_SDN                 :  2;		/* 17..18, 0x00060000 */
				FIELD  SEEE_SE_CORE_HORI_Y5                  :  6;		/* 19..24, 0x01F80000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_CORE_CTRL_1;	/* DIP_X_SEEE_SE_CORE_CTRL_1, DIP_A_SEEE_SE_CORE_CTRL_1*/

typedef volatile union _DIP_X_REG_SEEE_SE_CORE_CTRL_2_
{
		volatile struct	/* 0x150229F8 */
		{
				FIELD  SEEE_SE_CORE_VERT_X0                  :  7;		/*  0.. 6, 0x0000007F */
				FIELD  SEEE_SE_CORE_VERT_SUP                 :  2;		/*  7.. 8, 0x00000180 */
				FIELD  SEEE_SE_CORE_VERT_X2                  :  8;		/*  9..16, 0x0001FE00 */
				FIELD  SEEE_SE_CORE_VERT_SDN                 :  2;		/* 17..18, 0x00060000 */
				FIELD  SEEE_SE_CORE_VERT_Y5                  :  6;		/* 19..24, 0x01F80000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SEEE_SE_CORE_CTRL_2;	/* DIP_X_SEEE_SE_CORE_CTRL_2, DIP_A_SEEE_SE_CORE_CTRL_2*/

typedef volatile union _DIP_X_REG_NDG2_RAN_0_
{
		volatile struct	/* 0x15022A10 */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_0;	/* DIP_X_NDG2_RAN_0, DIP_A_NDG2_RAN_0*/

typedef volatile union _DIP_X_REG_NDG2_RAN_1_
{
		volatile struct	/* 0x15022A14 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_1;	/* DIP_X_NDG2_RAN_1, DIP_A_NDG2_RAN_1*/

typedef volatile union _DIP_X_REG_NDG2_RAN_2_
{
		volatile struct	/* 0x15022A18 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_2;	/* DIP_X_NDG2_RAN_2, DIP_A_NDG2_RAN_2*/

typedef volatile union _DIP_X_REG_NDG2_RAN_3_
{
		volatile struct	/* 0x15022A1C */
		{
				FIELD  NDG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_RAN_3;	/* DIP_X_NDG2_RAN_3, DIP_A_NDG2_RAN_3*/

typedef volatile union _DIP_X_REG_NDG2_CROP_X_
{
		volatile struct	/* 0x15022A20 */
		{
				FIELD  NDG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_CROP_X;	/* DIP_X_NDG2_CROP_X, DIP_A_NDG2_CROP_X*/

typedef volatile union _DIP_X_REG_NDG2_CROP_Y_
{
		volatile struct	/* 0x15022A24 */
		{
				FIELD  NDG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG2_CROP_Y;	/* DIP_X_NDG2_CROP_Y, DIP_A_NDG2_CROP_Y*/

typedef volatile union _DIP_X_REG_CAM_TNR_ENG_CON_
{
		volatile struct	/* 0x15022A30 */
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
		volatile struct	/* 0x15022A34 */
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
		volatile struct	/* 0x15022A38 */
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
		volatile struct	/* 0x15022A3C */
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
		volatile struct	/* 0x15022A40 */
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
		volatile struct	/* 0x15022A44 */
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
		volatile struct	/* 0x15022A48 */
		{
				FIELD  rsv_0                                 : 30;		/*  0..29, 0x3FFFFFFF */
				FIELD  NR3D_TNR_C_EN                         :  1;		/* 30..30, 0x40000000 */
				FIELD  NR3D_TNR_Y_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_TNR_ENABLE;	/* DIP_X_MDP_TNR_TNR_ENABLE, DIP_A_MDP_TNR_TNR_ENABLE*/

typedef volatile union _DIP_X_REG_MDP_TNR_FLT_CONFIG_
{
		volatile struct	/* 0x15022A4C */
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
		volatile struct	/* 0x15022A50 */
		{
				FIELD  rsv_0                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  NR3D_Q_SP                             :  6;		/* 20..25, 0x03F00000 */
				FIELD  NR3D_Q_NL                             :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_FB_INFO1;	/* DIP_X_MDP_TNR_FB_INFO1, DIP_A_MDP_TNR_FB_INFO1*/

typedef volatile union _DIP_X_REG_MDP_TNR_THR_1_
{
		volatile struct	/* 0x15022A54 */
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
		volatile struct	/* 0x15022A58 */
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
		volatile struct	/* 0x15022A5C */
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
		volatile struct	/* 0x15022A60 */
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
		volatile struct	/* 0x15022A64 */
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
		volatile struct	/* 0x15022A68 */
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
		volatile struct	/* 0x15022A6C */
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
		volatile struct	/* 0x15022A70 */
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
		volatile struct	/* 0x15022A74 */
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
		volatile struct	/* 0x15022A78 */
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
		volatile struct	/* 0x15022A7C */
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
		volatile struct	/* 0x15022A80 */
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
		volatile struct	/* 0x15022A84 */
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
		volatile struct	/* 0x15022A88 */
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
		volatile struct	/* 0x15022A8C */
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
		volatile struct	/* 0x15022A90 */
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
		volatile struct	/* 0x15022A94 */
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
		volatile struct	/* 0x15022A98 */
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
		volatile struct	/* 0x15022A9C */
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
		volatile struct	/* 0x15022AA0 */
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
		volatile struct	/* 0x15022AA4 */
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
		volatile struct	/* 0x15022AA8 */
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
		volatile struct	/* 0x15022AAC */
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
		volatile struct	/* 0x15022AB0 */
		{
				FIELD  rsv_0                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  NR3D_DEMO_RIGHT                       : 13;		/*  6..18, 0x0007FFC0 */
				FIELD  NR3D_DEMO_LEFT                        : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DEMO_2;	/* DIP_X_MDP_TNR_DEMO_2, DIP_A_MDP_TNR_DEMO_2*/

typedef volatile union _DIP_X_REG_MDP_TNR_ATPG_
{
		volatile struct	/* 0x15022AB4 */
		{
				FIELD  NR3D_TNR_ATPG_OB                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NR3D_TNR_ATPG_CT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_ATPG;	/* DIP_X_MDP_TNR_ATPG, DIP_A_MDP_TNR_ATPG*/

typedef volatile union _DIP_X_REG_MDP_TNR_DMY_0_
{
		volatile struct	/* 0x15022AB8 */
		{
				FIELD  NR3D_TNR_DUMMY0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DMY_0;	/* DIP_X_MDP_TNR_DMY_0, DIP_A_MDP_TNR_DMY_0*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_17_
{
		volatile struct	/* 0x15022ABC */
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
		volatile struct	/* 0x15022AC0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  NR3D_OSD_INF3                         : 12;		/*  8..19, 0x000FFF00 */
				FIELD  NR3D_OSD_INF2                         : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_18;	/* DIP_X_MDP_TNR_DBG_18, DIP_A_MDP_TNR_DBG_18*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_1_
{
		volatile struct	/* 0x15022AC4 */
		{
				FIELD  NR3D_TNR_DBG0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_1;	/* DIP_X_MDP_TNR_DBG_1, DIP_A_MDP_TNR_DBG_1*/

typedef volatile union _DIP_X_REG_MDP_TNR_DBG_2_
{
		volatile struct	/* 0x15022AC8 */
		{
				FIELD  NR3D_TNR_DBG1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_TNR_DBG_2;	/* DIP_X_MDP_TNR_DBG_2, DIP_A_MDP_TNR_DBG_2*/

typedef volatile union _DIP_X_REG_SL2B_CEN_
{
		volatile struct	/* 0x15022B30 */
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
		volatile struct	/* 0x15022B34 */
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
		volatile struct	/* 0x15022B38 */
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
		volatile struct	/* 0x15022B3C */
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
		volatile struct	/* 0x15022B40 */
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
		volatile struct	/* 0x15022B44 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_XOFF;	/* DIP_X_SL2B_XOFF, DIP_A_SL2B_XOFF*/

typedef volatile union _DIP_X_REG_SL2B_YOFF_
{
		volatile struct	/* 0x15022B48 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_YOFF;	/* DIP_X_SL2B_YOFF, DIP_A_SL2B_YOFF*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON0_
{
		volatile struct	/* 0x15022B4C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON0;	/* DIP_X_SL2B_SLP_CON0, DIP_A_SL2B_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON1_
{
		volatile struct	/* 0x15022B50 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON1;	/* DIP_X_SL2B_SLP_CON1, DIP_A_SL2B_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON2_
{
		volatile struct	/* 0x15022B54 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON2;	/* DIP_X_SL2B_SLP_CON2, DIP_A_SL2B_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2B_SLP_CON3_
{
		volatile struct	/* 0x15022B58 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SLP_CON3;	/* DIP_X_SL2B_SLP_CON3, DIP_A_SL2B_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2B_SIZE_
{
		volatile struct	/* 0x15022B5C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2B_SIZE;	/* DIP_X_SL2B_SIZE, DIP_A_SL2B_SIZE*/

typedef volatile union _DIP_X_REG_SL2C_CEN_
{
		volatile struct	/* 0x15022B70 */
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
		volatile struct	/* 0x15022B74 */
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
		volatile struct	/* 0x15022B78 */
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
		volatile struct	/* 0x15022B7C */
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
		volatile struct	/* 0x15022B80 */
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
		volatile struct	/* 0x15022B84 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_XOFF;	/* DIP_X_SL2C_XOFF, DIP_A_SL2C_XOFF*/

typedef volatile union _DIP_X_REG_SL2C_YOFF_
{
		volatile struct	/* 0x15022B88 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_YOFF;	/* DIP_X_SL2C_YOFF, DIP_A_SL2C_YOFF*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON0_
{
		volatile struct	/* 0x15022B8C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON0;	/* DIP_X_SL2C_SLP_CON0, DIP_A_SL2C_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON1_
{
		volatile struct	/* 0x15022B90 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON1;	/* DIP_X_SL2C_SLP_CON1, DIP_A_SL2C_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON2_
{
		volatile struct	/* 0x15022B94 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON2;	/* DIP_X_SL2C_SLP_CON2, DIP_A_SL2C_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2C_SLP_CON3_
{
		volatile struct	/* 0x15022B98 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SLP_CON3;	/* DIP_X_SL2C_SLP_CON3, DIP_A_SL2C_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2C_SIZE_
{
		volatile struct	/* 0x15022B9C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2C_SIZE;	/* DIP_X_SL2C_SIZE, DIP_A_SL2C_SIZE*/

typedef volatile union _DIP_X_REG_SRZ1_CONTROL_
{
		volatile struct	/* 0x15022BB0 */
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
		volatile struct	/* 0x15022BB4 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_IN_IMG;	/* DIP_X_SRZ1_IN_IMG, DIP_A_SRZ1_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ1_OUT_IMG_
{
		volatile struct	/* 0x15022BB8 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_OUT_IMG;	/* DIP_X_SRZ1_OUT_IMG, DIP_A_SRZ1_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_STEP_
{
		volatile struct	/* 0x15022BBC */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_STEP;	/* DIP_X_SRZ1_HORI_STEP, DIP_A_SRZ1_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_STEP_
{
		volatile struct	/* 0x15022BC0 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_STEP;	/* DIP_X_SRZ1_VERT_STEP, DIP_A_SRZ1_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_INT_OFST_
{
		volatile struct	/* 0x15022BC4 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_INT_OFST;	/* DIP_X_SRZ1_HORI_INT_OFST, DIP_A_SRZ1_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_HORI_SUB_OFST_
{
		volatile struct	/* 0x15022BC8 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_HORI_SUB_OFST;	/* DIP_X_SRZ1_HORI_SUB_OFST, DIP_A_SRZ1_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_INT_OFST_
{
		volatile struct	/* 0x15022BCC */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_INT_OFST;	/* DIP_X_SRZ1_VERT_INT_OFST, DIP_A_SRZ1_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ1_VERT_SUB_OFST_
{
		volatile struct	/* 0x15022BD0 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ1_VERT_SUB_OFST;	/* DIP_X_SRZ1_VERT_SUB_OFST, DIP_A_SRZ1_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_CONTROL_
{
		volatile struct	/* 0x15022BE0 */
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
		volatile struct	/* 0x15022BE4 */
		{
				FIELD  SRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_IN_IMG;	/* DIP_X_SRZ2_IN_IMG, DIP_A_SRZ2_IN_IMG*/

typedef volatile union _DIP_X_REG_SRZ2_OUT_IMG_
{
		volatile struct	/* 0x15022BE8 */
		{
				FIELD  SRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_OUT_IMG;	/* DIP_X_SRZ2_OUT_IMG, DIP_A_SRZ2_OUT_IMG*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_STEP_
{
		volatile struct	/* 0x15022BEC */
		{
				FIELD  SRZ_HORI_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_STEP;	/* DIP_X_SRZ2_HORI_STEP, DIP_A_SRZ2_HORI_STEP*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_STEP_
{
		volatile struct	/* 0x15022BF0 */
		{
				FIELD  SRZ_VERT_STEP                         : 19;		/*  0..18, 0x0007FFFF */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_STEP;	/* DIP_X_SRZ2_VERT_STEP, DIP_A_SRZ2_VERT_STEP*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_INT_OFST_
{
		volatile struct	/* 0x15022BF4 */
		{
				FIELD  SRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_INT_OFST;	/* DIP_X_SRZ2_HORI_INT_OFST, DIP_A_SRZ2_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_HORI_SUB_OFST_
{
		volatile struct	/* 0x15022BF8 */
		{
				FIELD  SRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_HORI_SUB_OFST;	/* DIP_X_SRZ2_HORI_SUB_OFST, DIP_A_SRZ2_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_INT_OFST_
{
		volatile struct	/* 0x15022BFC */
		{
				FIELD  SRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_INT_OFST;	/* DIP_X_SRZ2_VERT_INT_OFST, DIP_A_SRZ2_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_SRZ2_VERT_SUB_OFST_
{
		volatile struct	/* 0x15022C00 */
		{
				FIELD  SRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SRZ2_VERT_SUB_OFST;	/* DIP_X_SRZ2_VERT_SUB_OFST, DIP_A_SRZ2_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CONTROL_
{
		volatile struct	/* 0x15022C10 */
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
		volatile struct	/* 0x15022C14 */
		{
				FIELD  CRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_IN_IMG;	/* DIP_X_CRZ_IN_IMG, DIP_A_CRZ_IN_IMG*/

typedef volatile union _DIP_X_REG_CRZ_OUT_IMG_
{
		volatile struct	/* 0x15022C18 */
		{
				FIELD  CRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_OUT_IMG;	/* DIP_X_CRZ_OUT_IMG, DIP_A_CRZ_OUT_IMG*/

typedef volatile union _DIP_X_REG_CRZ_HORI_STEP_
{
		volatile struct	/* 0x15022C1C */
		{
				FIELD  CRZ_HORI_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_HORI_STEP;	/* DIP_X_CRZ_HORI_STEP, DIP_A_CRZ_HORI_STEP*/

typedef volatile union _DIP_X_REG_CRZ_VERT_STEP_
{
		volatile struct	/* 0x15022C20 */
		{
				FIELD  CRZ_VERT_STEP                         : 23;		/*  0..22, 0x007FFFFF */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_VERT_STEP;	/* DIP_X_CRZ_VERT_STEP, DIP_A_CRZ_VERT_STEP*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_HORI_INT_OFST_
{
		volatile struct	/* 0x15022C24 */
		{
				FIELD  CRZ_LUMA_HORI_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_HORI_INT_OFST;	/* DIP_X_CRZ_LUMA_HORI_INT_OFST, DIP_A_CRZ_LUMA_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST_
{
		volatile struct	/* 0x15022C28 */
		{
				FIELD  CRZ_LUMA_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST;	/* DIP_X_CRZ_LUMA_HORI_SUB_OFST, DIP_A_CRZ_LUMA_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_VERT_INT_OFST_
{
		volatile struct	/* 0x15022C2C */
		{
				FIELD  CRZ_LUMA_VERT_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_VERT_INT_OFST;	/* DIP_X_CRZ_LUMA_VERT_INT_OFST, DIP_A_CRZ_LUMA_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST_
{
		volatile struct	/* 0x15022C30 */
		{
				FIELD  CRZ_LUMA_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST;	/* DIP_X_CRZ_LUMA_VERT_SUB_OFST, DIP_A_CRZ_LUMA_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_HORI_INT_OFST_
{
		volatile struct	/* 0x15022C34 */
		{
				FIELD  CRZ_CHRO_HORI_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_HORI_INT_OFST;	/* DIP_X_CRZ_CHRO_HORI_INT_OFST, DIP_A_CRZ_CHRO_HORI_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST_
{
		volatile struct	/* 0x15022C38 */
		{
				FIELD  CRZ_CHRO_HORI_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST;	/* DIP_X_CRZ_CHRO_HORI_SUB_OFST, DIP_A_CRZ_CHRO_HORI_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_VERT_INT_OFST_
{
		volatile struct	/* 0x15022C3C */
		{
				FIELD  CRZ_CHRO_VERT_INT_OFST                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_VERT_INT_OFST;	/* DIP_X_CRZ_CHRO_VERT_INT_OFST, DIP_A_CRZ_CHRO_VERT_INT_OFST*/

typedef volatile union _DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST_
{
		volatile struct	/* 0x15022C40 */
		{
				FIELD  CRZ_CHRO_VERT_SUB_OFST                : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST;	/* DIP_X_CRZ_CHRO_VERT_SUB_OFST, DIP_A_CRZ_CHRO_VERT_SUB_OFST*/

typedef volatile union _DIP_X_REG_CRZ_SPARE_1_
{
		volatile struct	/* 0x15022C44 */
		{
				FIELD  CRZ_SPARE_REG                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRZ_SPARE_1;	/* DIP_X_CRZ_SPARE_1, DIP_A_CRZ_SPARE_1*/

typedef volatile union _DIP_X_REG_MIX1_CTRL_0_
{
		volatile struct	/* 0x15022C70 */
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
		volatile struct	/* 0x15022C74 */
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
		volatile struct	/* 0x15022C78 */
		{
				FIELD  MIX1_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX1_SPARE;	/* DIP_X_MIX1_SPARE, DIP_A_MIX1_SPARE*/

typedef volatile union _DIP_X_REG_MIX2_CTRL_0_
{
		volatile struct	/* 0x15022C80 */
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
		volatile struct	/* 0x15022C84 */
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
		volatile struct	/* 0x15022C88 */
		{
				FIELD  MIX2_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX2_SPARE;	/* DIP_X_MIX2_SPARE, DIP_A_MIX2_SPARE*/

typedef volatile union _DIP_X_REG_MIX3_CTRL_0_
{
		volatile struct	/* 0x15022C90 */
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
		volatile struct	/* 0x15022C94 */
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
		volatile struct	/* 0x15022C98 */
		{
				FIELD  MIX3_SPARE                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MIX3_SPARE;	/* DIP_X_MIX3_SPARE, DIP_A_MIX3_SPARE*/

typedef volatile union _DIP_X_REG_SL2D_CEN_
{
		volatile struct	/* 0x15022CA0 */
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
		volatile struct	/* 0x15022CA4 */
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
		volatile struct	/* 0x15022CA8 */
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
		volatile struct	/* 0x15022CAC */
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
		volatile struct	/* 0x15022CB0 */
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
		volatile struct	/* 0x15022CB4 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_XOFF;	/* DIP_X_SL2D_XOFF, DIP_A_SL2D_XOFF*/

typedef volatile union _DIP_X_REG_SL2D_YOFF_
{
		volatile struct	/* 0x15022CB8 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_YOFF;	/* DIP_X_SL2D_YOFF, DIP_A_SL2D_YOFF*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON0_
{
		volatile struct	/* 0x15022CBC */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON0;	/* DIP_X_SL2D_SLP_CON0, DIP_A_SL2D_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON1_
{
		volatile struct	/* 0x15022CC0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON1;	/* DIP_X_SL2D_SLP_CON1, DIP_A_SL2D_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON2_
{
		volatile struct	/* 0x15022CC4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON2;	/* DIP_X_SL2D_SLP_CON2, DIP_A_SL2D_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2D_SLP_CON3_
{
		volatile struct	/* 0x15022CC8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SLP_CON3;	/* DIP_X_SL2D_SLP_CON3, DIP_A_SL2D_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2D_SIZE_
{
		volatile struct	/* 0x15022CCC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2D_SIZE;	/* DIP_X_SL2D_SIZE, DIP_A_SL2D_SIZE*/

typedef volatile union _DIP_X_REG_SL2E_CEN_
{
		volatile struct	/* 0x15022CE0 */
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
		volatile struct	/* 0x15022CE4 */
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
		volatile struct	/* 0x15022CE8 */
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
		volatile struct	/* 0x15022CEC */
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
		volatile struct	/* 0x15022CF0 */
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
		volatile struct	/* 0x15022CF4 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_XOFF;	/* DIP_X_SL2E_XOFF, DIP_A_SL2E_XOFF*/

typedef volatile union _DIP_X_REG_SL2E_YOFF_
{
		volatile struct	/* 0x15022CF8 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_YOFF;	/* DIP_X_SL2E_YOFF, DIP_A_SL2E_YOFF*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON0_
{
		volatile struct	/* 0x15022CFC */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON0;	/* DIP_X_SL2E_SLP_CON0, DIP_A_SL2E_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON1_
{
		volatile struct	/* 0x15022D00 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON1;	/* DIP_X_SL2E_SLP_CON1, DIP_A_SL2E_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON2_
{
		volatile struct	/* 0x15022D04 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON2;	/* DIP_X_SL2E_SLP_CON2, DIP_A_SL2E_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2E_SLP_CON3_
{
		volatile struct	/* 0x15022D08 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SLP_CON3;	/* DIP_X_SL2E_SLP_CON3, DIP_A_SL2E_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2E_SIZE_
{
		volatile struct	/* 0x15022D0C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2E_SIZE;	/* DIP_X_SL2E_SIZE, DIP_A_SL2E_SIZE*/

typedef volatile union _DIP_X_REG_MDP_CROP_X_
{
		volatile struct	/* 0x15022D20 */
		{
				FIELD  MDP_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP_X;	/* DIP_X_MDP_CROP_X, DIP_A_MDP_CROP_X*/

typedef volatile union _DIP_X_REG_MDP_CROP_Y_
{
		volatile struct	/* 0x15022D24 */
		{
				FIELD  MDP_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MDP_CROP_Y;	/* DIP_X_MDP_CROP_Y, DIP_A_MDP_CROP_Y*/

typedef volatile union _DIP_X_REG_ANR2_CON1_
{
		volatile struct	/* 0x15022D30 */
		{
				FIELD  ANR2_ENC                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ANR2_ENY                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  ANR2_SCALE_MODE                       :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  6;		/*  6..11, 0x00000FC0 */
				FIELD  ANR2_MODE                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR2_TILE_EDGE                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR2_LCE_LINK                         :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_CON1;	/* DIP_X_ANR2_CON1, DIP_A_ANR2_CON1*/

typedef volatile union _DIP_X_REG_ANR2_CON2_
{
		volatile struct	/* 0x15022D34 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_C_SM_EDGE                        :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  ANR2_FLT_C                            :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_CON2;	/* DIP_X_ANR2_CON2, DIP_A_ANR2_CON2*/

typedef volatile union _DIP_X_REG_ANR2_YAD1_
{
		volatile struct	/* 0x15022D38 */
		{
				FIELD  rsv_0                                 : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  ANR2_K_TH_C                           :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_YAD1;	/* DIP_X_ANR2_YAD1, DIP_A_ANR2_YAD1*/

typedef volatile union _DIP_X_REG_ANR2_Y4LUT1_
{
		volatile struct	/* 0x15022D40 */
		{
				FIELD  ANR2_Y_CPX1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_Y_CPX2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR2_Y_CPX3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_Y4LUT1;	/* DIP_X_ANR2_Y4LUT1, DIP_A_ANR2_Y4LUT1*/

typedef volatile union _DIP_X_REG_ANR2_Y4LUT2_
{
		volatile struct	/* 0x15022D44 */
		{
				FIELD  ANR2_Y_SCALE_CPY0                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR2_Y_SCALE_CPY1                     :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR2_Y_SCALE_CPY2                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR2_Y_SCALE_CPY3                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_Y4LUT2;	/* DIP_X_ANR2_Y4LUT2, DIP_A_ANR2_Y4LUT2*/

typedef volatile union _DIP_X_REG_ANR2_Y4LUT3_
{
		volatile struct	/* 0x15022D48 */
		{
				FIELD  ANR2_Y_SCALE_SP0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR2_Y_SCALE_SP1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR2_Y_SCALE_SP2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR2_Y_SCALE_SP3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_Y4LUT3;	/* DIP_X_ANR2_Y4LUT3, DIP_A_ANR2_Y4LUT3*/

typedef volatile union _DIP_X_REG_ANR2_L4LUT1_
{
		volatile struct	/* 0x15022D60 */
		{
				FIELD  ANR2_LCE_X1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_LCE_X2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR2_LCE_X3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_L4LUT1;	/* DIP_X_ANR2_L4LUT1, DIP_A_ANR2_L4LUT1*/

typedef volatile union _DIP_X_REG_ANR2_L4LUT2_
{
		volatile struct	/* 0x15022D64 */
		{
				FIELD  ANR2_LCE_GAIN0                        :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR2_LCE_GAIN1                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR2_LCE_GAIN2                        :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR2_LCE_GAIN3                        :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_L4LUT2;	/* DIP_X_ANR2_L4LUT2, DIP_A_ANR2_L4LUT2*/

typedef volatile union _DIP_X_REG_ANR2_L4LUT3_
{
		volatile struct	/* 0x15022D68 */
		{
				FIELD  ANR2_LCE_SP0                          :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR2_LCE_SP1                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR2_LCE_SP2                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR2_LCE_SP3                          :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_L4LUT3;	/* DIP_X_ANR2_L4LUT3, DIP_A_ANR2_L4LUT3*/

typedef volatile union _DIP_X_REG_ANR2_CAD_
{
		volatile struct	/* 0x15022D70 */
		{
				FIELD  ANR2_PTC_VGAIN                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR2_C_GAIN                           :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR2_PTC_GAIN_TH                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR2_C_L_DIFF_TH                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR2_C_MODE                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_CAD;	/* DIP_X_ANR2_CAD, DIP_A_ANR2_CAD*/

typedef volatile union _DIP_X_REG_ANR2_PTC_
{
		volatile struct	/* 0x15022D74 */
		{
				FIELD  ANR2_PTC1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_PTC2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR2_PTC3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR2_PTC4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_PTC;	/* DIP_X_ANR2_PTC, DIP_A_ANR2_PTC*/

typedef volatile union _DIP_X_REG_ANR2_LCE_
{
		volatile struct	/* 0x15022D78 */
		{
				FIELD  ANR2_LCE_C_GAIN                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  ANR2_LM_WT                            :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_LCE;	/* DIP_X_ANR2_LCE, DIP_A_ANR2_LCE*/

typedef volatile union _DIP_X_REG_ANR2_MED1_
{
		volatile struct	/* 0x15022D7C */
		{
				FIELD  ANR2_COR_TH                           :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR2_COR_SL                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  ANR2_MCD_TH                           :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR2_MCD_SL                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR2_LCL_TH                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_MED1;	/* DIP_X_ANR2_MED1, DIP_A_ANR2_MED1*/

typedef volatile union _DIP_X_REG_ANR2_MED2_
{
		volatile struct	/* 0x15022D80 */
		{
				FIELD  ANR2_LCL_SL                           :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ANR2_LCL_LV                           :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  ANR2_SCL_TH                           :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  ANR2_SCL_SL                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR2_SCL_LV                           :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_MED2;	/* DIP_X_ANR2_MED2, DIP_A_ANR2_MED2*/

typedef volatile union _DIP_X_REG_ANR2_MED3_
{
		volatile struct	/* 0x15022D84 */
		{
				FIELD  ANR2_NCL_TH                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_NCL_SL                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  ANR2_NCL_LV                           :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR2_VAR                              :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR2_Y0                               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_MED3;	/* DIP_X_ANR2_MED3, DIP_A_ANR2_MED3*/

typedef volatile union _DIP_X_REG_ANR2_MED4_
{
		volatile struct	/* 0x15022D88 */
		{
				FIELD  ANR2_Y1                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR2_Y2                               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR2_Y3                               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR2_Y4                               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_MED4;	/* DIP_X_ANR2_MED4, DIP_A_ANR2_MED4*/

typedef volatile union _DIP_X_REG_ANR2_MED5_
{
		volatile struct	/* 0x15022D8C */
		{
				FIELD  ANR2_LCL_OFT                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_SCL_OFT                          :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR2_NCL_OFT                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_MED5;	/* DIP_X_ANR2_MED5, DIP_A_ANR2_MED5*/

typedef volatile union _DIP_X_REG_CCR_RSV1_
{
		volatile struct	/* 0x15022D90 */
		{
				FIELD  CCR_RSV1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_RSV1;	/* DIP_X_CCR_RSV1, DIP_A_CCR_RSV1*/

typedef volatile union _DIP_X_REG_ANR2_ACTC_
{
		volatile struct	/* 0x15022DA4 */
		{
				FIELD  RSV                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR2_ACT_BLD_BASE_C                   :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR2_C_DITH_U                         :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR2_C_DITH_V                         :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_ACTC;	/* DIP_X_ANR2_ACTC, DIP_A_ANR2_ACTC*/

typedef volatile union _DIP_X_REG_ANR2_RSV1_
{
		volatile struct	/* 0x15022DA8 */
		{
				FIELD  ANR2_RSV1                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_RSV1;	/* DIP_X_ANR2_RSV1, DIP_A_ANR2_RSV1*/

typedef volatile union _DIP_X_REG_ANR2_RSV2_
{
		volatile struct	/* 0x15022DAC */
		{
				FIELD  ANR2_RSV2                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR2_RSV2;	/* DIP_X_ANR2_RSV2, DIP_A_ANR2_RSV2*/

typedef volatile union _DIP_X_REG_CCR_CON_
{
		volatile struct	/* 0x15022DB0 */
		{
				FIELD  CCR_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CCR_LCE_LINK                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCR_LCE_MODE                          :  2;		/*  2.. 3, 0x0000000C */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CCR_OR_MODE                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CCR_UV_GAIN_MODE                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_UV_GAIN2                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_Y_CPX3                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_CON;	/* DIP_X_CCR_CON, DIP_A_CCR_CON*/

typedef volatile union _DIP_X_REG_CCR_YLUT_
{
		volatile struct	/* 0x15022DB4 */
		{
				FIELD  CCR_Y_CPX1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_Y_CPX2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_Y_SP1                             :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_Y_CPY1                            :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_YLUT;	/* DIP_X_CCR_YLUT, DIP_A_CCR_YLUT*/

typedef volatile union _DIP_X_REG_CCR_UVLUT_
{
		volatile struct	/* 0x15022DB8 */
		{
				FIELD  CCR_UV_X1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_UV_X2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_UV_X3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CCR_UV_GAIN1                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_UVLUT;	/* DIP_X_CCR_UVLUT, DIP_A_CCR_UVLUT*/

typedef volatile union _DIP_X_REG_CCR_YLUT2_
{
		volatile struct	/* 0x15022DBC */
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
}DIP_X_REG_CCR_YLUT2;	/* DIP_X_CCR_YLUT2, DIP_A_CCR_YLUT2*/

typedef volatile union _DIP_X_REG_CCR_SAT_CTRL_
{
		volatile struct	/* 0x15022DC0 */
		{
				FIELD  CCR_MODE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CCR_CEN_U                             :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  CCR_CEN_V                             :  6;		/* 12..17, 0x0003F000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_SAT_CTRL;	/* DIP_X_CCR_SAT_CTRL, DIP_A_CCR_SAT_CTRL*/

typedef volatile union _DIP_X_REG_CCR_UVLUT_SP_
{
		volatile struct	/* 0x15022DC4 */
		{
				FIELD  CCR_UV_GAIN_SP1                       : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  CCR_UV_GAIN_SP2                       : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_UVLUT_SP;	/* DIP_X_CCR_UVLUT_SP, DIP_A_CCR_UVLUT_SP*/

typedef volatile union _DIP_X_REG_CCR_HUE1_
{
		volatile struct	/* 0x15022DC8 */
		{
				FIELD  CCR_HUE_X1                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_HUE_X2                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_HUE1;	/* DIP_X_CCR_HUE1, DIP_A_CCR_HUE1*/

typedef volatile union _DIP_X_REG_CCR_HUE2_
{
		volatile struct	/* 0x15022DCC */
		{
				FIELD  CCR_HUE_X3                            :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CCR_HUE_X4                            :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_HUE2;	/* DIP_X_CCR_HUE2, DIP_A_CCR_HUE2*/

typedef volatile union _DIP_X_REG_CCR_HUE3_
{
		volatile struct	/* 0x15022DD0 */
		{
				FIELD  CCR_HUE_SP1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_HUE_SP2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_HUE_GAIN1                         :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CCR_HUE_GAIN2                         :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_HUE3;	/* DIP_X_CCR_HUE3, DIP_A_CCR_HUE3*/

typedef volatile union _DIP_X_REG_CCR_L4LUT1_
{
		volatile struct	/* 0x15022DD4 */
		{
				FIELD  CCR_LCE_X1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCR_LCE_X2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCR_LCE_X3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_L4LUT1;	/* DIP_X_CCR_L4LUT1, DIP_A_CCR_L4LUT1*/

typedef volatile union _DIP_X_REG_CCR_L4LUT2_
{
		volatile struct	/* 0x15022DD8 */
		{
				FIELD  CCR_LCE_GAIN0                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CCR_LCE_GAIN1                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CCR_LCE_GAIN2                         :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  CCR_LCE_GAIN3                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_L4LUT2;	/* DIP_X_CCR_L4LUT2, DIP_A_CCR_L4LUT2*/

typedef volatile union _DIP_X_REG_CCR_L4LUT3_
{
		volatile struct	/* 0x15022DDC */
		{
				FIELD  CCR_LCE_SP0                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CCR_LCE_SP1                           :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CCR_LCE_SP2                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  CCR_LCE_SP3                           :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CCR_L4LUT3;	/* DIP_X_CCR_L4LUT3, DIP_A_CCR_L4LUT3*/

typedef volatile union _DIP_X_REG_BOK_CON_
{
		volatile struct	/* 0x15022DE0 */
		{
				FIELD  BOK_MODE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BOK_AP_MODE                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  BOK_FGBG_MODE                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  BOK_FGBG_WT                           :  4;		/* 12..15, 0x0000F000 */
				FIELD  BOK_PF_EN                             :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BOK_CON;	/* DIP_X_BOK_CON, DIP_A_BOK_CON*/

typedef volatile union _DIP_X_REG_BOK_TUN_
{
		volatile struct	/* 0x15022DE4 */
		{
				FIELD  BOK_STR_WT                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BOK_WT_GAIN                           :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BOK_INTENSITY                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BOK_DOF_M                             :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BOK_TUN;	/* DIP_X_BOK_TUN, DIP_A_BOK_TUN*/

typedef volatile union _DIP_X_REG_BOK_OFF_
{
		volatile struct	/* 0x15022DE8 */
		{
				FIELD  BOK_XOFF                              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  BOK_YOFF                              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BOK_OFF;	/* DIP_X_BOK_OFF, DIP_A_BOK_OFF*/

typedef volatile union _DIP_X_REG_BOK_RSV1_
{
		volatile struct	/* 0x15022DEC */
		{
				FIELD  BOK_RSV1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BOK_RSV1;	/* DIP_X_BOK_RSV1, DIP_A_BOK_RSV1*/

typedef volatile union _DIP_X_REG_ABF_CON1_
{
		volatile struct	/* 0x15022DF0 */
		{
				FIELD  ABF_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  ABF_MODE                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ABF_AVE_WIN                           :  2;		/*  8.. 9, 0x00000300 */
				FIELD  ABF_W_DST                             :  2;		/* 10..11, 0x00000C00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CON1;	/* DIP_X_ABF_CON1, DIP_A_ABF_CON1*/

typedef volatile union _DIP_X_REG_ABF_CON2_
{
		volatile struct	/* 0x15022DF4 */
		{
				FIELD  ABF_DES_NUM                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  ABF_DSA_STEP                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CON2;	/* DIP_X_ABF_CON2, DIP_A_ABF_CON2*/

typedef volatile union _DIP_X_REG_ABF_RCON_
{
		volatile struct	/* 0x15022DF8 */
		{
				FIELD  ABF_R1                                :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  ABF_R2                                :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_RCON;	/* DIP_X_ABF_RCON, DIP_A_ABF_RCON*/

typedef volatile union _DIP_X_REG_ABF_YLUT_
{
		volatile struct	/* 0x15022DFC */
		{
				FIELD  ABF_Y0                                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ABF_Y1                                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ABF_Y2                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ABF_Y3                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_YLUT;	/* DIP_X_ABF_YLUT, DIP_A_ABF_YLUT*/

typedef volatile union _DIP_X_REG_ABF_CXLUT_
{
		volatile struct	/* 0x15022E00 */
		{
				FIELD  ABF_CX0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ABF_CX1                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ABF_CX2                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ABF_CX3                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CXLUT;	/* DIP_X_ABF_CXLUT, DIP_A_ABF_CXLUT*/

typedef volatile union _DIP_X_REG_ABF_CYLUT_
{
		volatile struct	/* 0x15022E04 */
		{
				FIELD  ABF_CY0                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ABF_CY1                               :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ABF_CY2                               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ABF_CY3                               :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CYLUT;	/* DIP_X_ABF_CYLUT, DIP_A_ABF_CYLUT*/

typedef volatile union _DIP_X_REG_ABF_YSP_
{
		volatile struct	/* 0x15022E08 */
		{
				FIELD  ABF_Y_SP0                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  ABF_Y_SP1                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_YSP;	/* DIP_X_ABF_YSP, DIP_A_ABF_YSP*/

typedef volatile union _DIP_X_REG_ABF_CXSP_
{
		volatile struct	/* 0x15022E0C */
		{
				FIELD  ABF_CX_SP0                            : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  ABF_CX_SP1                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CXSP;	/* DIP_X_ABF_CXSP, DIP_A_ABF_CXSP*/

typedef volatile union _DIP_X_REG_ABF_CYSP_
{
		volatile struct	/* 0x15022E10 */
		{
				FIELD  ABF_CY_SP0                            : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  ABF_CY_SP1                            : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CYSP;	/* DIP_X_ABF_CYSP, DIP_A_ABF_CYSP*/

typedef volatile union _DIP_X_REG_ABF_CLP_
{
		volatile struct	/* 0x15022E14 */
		{
				FIELD  ABF_STHRE_R                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ABF_STHRE_G                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ABF_STHRE_B                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_CLP;	/* DIP_X_ABF_CLP, DIP_A_ABF_CLP*/

typedef volatile union _DIP_X_REG_ABF_DIFF_
{
		volatile struct	/* 0x15022E18 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  ABF_1_DIFF                            :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  rsv_8                                 :  3;		/*  8..10, 0x00000700 */
				FIELD  ABF_2_DIFF                            :  5;		/* 11..15, 0x0000F800 */
				FIELD  rsv_16                                :  3;		/* 16..18, 0x00070000 */
				FIELD  ABF_3_DIFF                            :  5;		/* 19..23, 0x00F80000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_DIFF;	/* DIP_X_ABF_DIFF, DIP_A_ABF_DIFF*/

typedef volatile union _DIP_X_REG_ABF_RSV1_
{
		volatile struct	/* 0x15022E1C */
		{
				FIELD  ABF_RSV                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ABF_RSV1;	/* DIP_X_ABF_RSV1, DIP_A_ABF_RSV1*/

typedef volatile union _DIP_X_REG_UNP_OFST_
{
		volatile struct	/* 0x15022E30 */
		{
				FIELD  UNP_OFST_STB                          :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  UNP_OFST_EDB                          :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_UNP_OFST;	/* DIP_X_UNP_OFST, DIP_A_UNP_OFST*/

typedef volatile union _DIP_X_REG_C02_CON_
{
		volatile struct	/* 0x15022E40 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02_CON;	/* DIP_X_C02_CON, DIP_A_C02_CON*/

typedef volatile union _DIP_X_REG_C02_CROP_CON1_
{
		volatile struct	/* 0x15022E44 */
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
		volatile struct	/* 0x15022E48 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02_CROP_CON2;	/* DIP_X_C02_CROP_CON2, DIP_A_C02_CROP_CON2*/

typedef volatile union _DIP_X_REG_FE_CTRL1_
{
		volatile struct	/* 0x15022E50 */
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
		volatile struct	/* 0x15022E54 */
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
		volatile struct	/* 0x15022E58 */
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
		volatile struct	/* 0x15022E5C */
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
		volatile struct	/* 0x15022E60 */
		{
				FIELD  FE_TDR_EDGE                           :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FE_CTRL2;	/* DIP_X_FE_CTRL2, DIP_A_FE_CTRL2*/

typedef volatile union _DIP_X_REG_CRSP_CTRL_
{
		volatile struct	/* 0x15022E70 */
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
		volatile struct	/* 0x15022E78 */
		{
				FIELD  CRSP_WD                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_HT                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_OUT_IMG;	/* DIP_X_CRSP_OUT_IMG, DIP_A_CRSP_OUT_IMG*/

typedef volatile union _DIP_X_REG_CRSP_STEP_OFST_
{
		volatile struct	/* 0x15022E7C */
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
		volatile struct	/* 0x15022E80 */
		{
				FIELD  CRSP_CROP_STR_X                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_X                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_CROP_X;	/* DIP_X_CRSP_CROP_X, DIP_A_CRSP_CROP_X*/

typedef volatile union _DIP_X_REG_CRSP_CROP_Y_
{
		volatile struct	/* 0x15022E84 */
		{
				FIELD  CRSP_CROP_STR_Y                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CRSP_CROP_END_Y                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CRSP_CROP_Y;	/* DIP_X_CRSP_CROP_Y, DIP_A_CRSP_CROP_Y*/

typedef volatile union _DIP_X_REG_C02B_CON_
{
		volatile struct	/* 0x15022E90 */
		{
				FIELD  C02_TPIPE_EDGE                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02B_CON;	/* DIP_X_C02B_CON, DIP_A_C02B_CON*/

typedef volatile union _DIP_X_REG_C02B_CROP_CON1_
{
		volatile struct	/* 0x15022E94 */
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
		volatile struct	/* 0x15022E98 */
		{
				FIELD  C02_CROP_YSTART                       : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  C02_CROP_YEND                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C02B_CROP_CON2;	/* DIP_X_C02B_CROP_CON2, DIP_A_C02B_CROP_CON2*/

typedef volatile union _DIP_X_REG_C24_TILE_EDGE_
{
		volatile struct	/* 0x15022EA0 */
		{
				FIELD  C24_TDR_EDGE                          :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C24_TILE_EDGE;	/* DIP_X_C24_TILE_EDGE, DIP_A_C24_TILE_EDGE*/

typedef volatile union _DIP_X_REG_C24B_TILE_EDGE_
{
		volatile struct	/* 0x15022EB0 */
		{
				FIELD  C24_TDR_EDGE                          :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_C24B_TILE_EDGE;	/* DIP_X_C24B_TILE_EDGE, DIP_A_C24B_TILE_EDGE*/

typedef volatile union _DIP_X_REG_LCE25_CON_
{
		volatile struct	/* 0x15022EC0 */
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
		volatile struct	/* 0x15022EC4 */
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
		volatile struct	/* 0x15022EC8 */
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
		volatile struct	/* 0x15022ECC */
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
		volatile struct	/* 0x15022ED0 */
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
		volatile struct	/* 0x15022ED4 */
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
		volatile struct	/* 0x15022ED8 */
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
		volatile struct	/* 0x15022EDC */
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
		volatile struct	/* 0x15022EE0 */
		{
				FIELD  LCE_CEN_END                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  LCE_CEN_BLD_WT                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  LCE_GLOB_TONE                         :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA0;	/* DIP_X_LCE25_TM_PARA0, DIP_A_LCE25_TM_PARA0*/

typedef volatile union _DIP_X_REG_LCE25_TM_PARA1_
{
		volatile struct	/* 0x15022EE4 */
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
		volatile struct	/* 0x15022EE8 */
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
		volatile struct	/* 0x15022EEC */
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
		volatile struct	/* 0x15022EF0 */
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
		volatile struct	/* 0x15022EF4 */
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
		volatile struct	/* 0x15022EF8 */
		{
				FIELD  LCE_TC_O999                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  8;		/* 12..19, 0x000FF000 */
				FIELD  LCE_CLR_BLDWD                         :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LCE25_TM_PARA6;	/* DIP_X_LCE25_TM_PARA6, DIP_A_LCE25_TM_PARA6*/

typedef volatile union _DIP_X_REG_FM_SIZE_
{
		volatile struct	/* 0x15022F40 */
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
		volatile struct	/* 0x15022F44 */
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
		volatile struct	/* 0x15022F48 */
		{
				FIELD  FM_ACC_RES                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FM_ACC_RES;	/* DIP_X_FM_ACC_RES, DIP_A_FM_ACC_RES*/

typedef volatile union _DIP_X_REG_MFB_CON_
{
		volatile struct	/* 0x15022F60 */
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
}DIP_X_REG_MFB_CON;	/* DIP_X_MFB_CON, DIP_A_MFB_CON*/

typedef volatile union _DIP_X_REG_MFB_LL_CON1_
{
		volatile struct	/* 0x15022F64 */
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
}DIP_X_REG_MFB_LL_CON1;	/* DIP_X_MFB_LL_CON1, DIP_A_MFB_LL_CON1*/

typedef volatile union _DIP_X_REG_MFB_LL_CON2_
{
		volatile struct	/* 0x15022F68 */
		{
				FIELD  BLD_LL_MAX_WT                         :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  BLD_LL_DT1                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH1                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH2                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON2;	/* DIP_X_MFB_LL_CON2, DIP_A_MFB_LL_CON2*/

typedef volatile union _DIP_X_REG_MFB_LL_CON3_
{
		volatile struct	/* 0x15022F6C */
		{
				FIELD  BLD_LL_OUT_XSIZE                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  BLD_LL_OUT_XOFST                      :  1;		/* 15..15, 0x00008000 */
				FIELD  BLD_LL_OUT_YSIZE                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON3;	/* DIP_X_MFB_LL_CON3, DIP_A_MFB_LL_CON3*/

typedef volatile union _DIP_X_REG_MFB_LL_CON4_
{
		volatile struct	/* 0x15022F70 */
		{
				FIELD  BLD_LL_DB_XDIST                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  BLD_LL_DB_YDIST                       :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON4;	/* DIP_X_MFB_LL_CON4, DIP_A_MFB_LL_CON4*/

typedef volatile union _DIP_X_REG_MFB_EDGE_
{
		volatile struct	/* 0x15022F74 */
		{
				FIELD  BLD_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_EDGE;	/* DIP_X_MFB_EDGE, DIP_A_MFB_EDGE*/

typedef volatile union _DIP_X_REG_MFB_LL_CON5_
{
		volatile struct	/* 0x15022F78 */
		{
				FIELD  BLD_LL_GRAD_R1                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_GRAD_R2                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH4                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON5;	/* DIP_X_MFB_LL_CON5, DIP_A_MFB_LL_CON5*/

typedef volatile union _DIP_X_REG_MFB_LL_CON6_
{
		volatile struct	/* 0x15022F7C */
		{
				FIELD  BLD_LL_TH5                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_TH6                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_TH7                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BLD_LL_TH8                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON6;	/* DIP_X_MFB_LL_CON6, DIP_A_MFB_LL_CON6*/

typedef volatile union _DIP_X_REG_MFB_LL_CON7_
{
		volatile struct	/* 0x15022F80 */
		{
				FIELD  BLD_LL_C1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BLD_LL_C2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BLD_LL_C3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON7;	/* DIP_X_MFB_LL_CON7, DIP_A_MFB_LL_CON7*/

typedef volatile union _DIP_X_REG_MFB_LL_CON8_
{
		volatile struct	/* 0x15022F84 */
		{
				FIELD  BLD_LL_SU1                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_LL_SU2                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_LL_SU3                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON8;	/* DIP_X_MFB_LL_CON8, DIP_A_MFB_LL_CON8*/

typedef volatile union _DIP_X_REG_MFB_LL_CON9_
{
		volatile struct	/* 0x15022F88 */
		{
				FIELD  BLD_LL_SL1                            :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BLD_LL_SL2                            :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BLD_LL_SL3                            :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON9;	/* DIP_X_MFB_LL_CON9, DIP_A_MFB_LL_CON9*/

typedef volatile union _DIP_X_REG_MFB_LL_CON10_
{
		volatile struct	/* 0x15022F8C */
		{
				FIELD  BLD_CONF_MAP_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  BLD_LL_GRAD_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  BLD_LL_GRAD_ENTH                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_LL_CON10;	/* DIP_X_MFB_LL_CON10, DIP_A_MFB_LL_CON10*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON0_
{
		volatile struct	/* 0x15022F90 */
		{
				FIELD  BLD_MBD_MAX_WT                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_MFB_MBD_CON0;	/* DIP_X_MFB_MBD_CON0, DIP_A_MFB_MBD_CON0*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON1_
{
		volatile struct	/* 0x15022F94 */
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
}DIP_X_REG_MFB_MBD_CON1;	/* DIP_X_MFB_MBD_CON1, DIP_A_MFB_MBD_CON1*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON2_
{
		volatile struct	/* 0x15022F98 */
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
}DIP_X_REG_MFB_MBD_CON2;	/* DIP_X_MFB_MBD_CON2, DIP_A_MFB_MBD_CON2*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON3_
{
		volatile struct	/* 0x15022F9C */
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
}DIP_X_REG_MFB_MBD_CON3;	/* DIP_X_MFB_MBD_CON3, DIP_A_MFB_MBD_CON3*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON4_
{
		volatile struct	/* 0x15022FA0 */
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
}DIP_X_REG_MFB_MBD_CON4;	/* DIP_X_MFB_MBD_CON4, DIP_A_MFB_MBD_CON4*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON5_
{
		volatile struct	/* 0x15022FA4 */
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
}DIP_X_REG_MFB_MBD_CON5;	/* DIP_X_MFB_MBD_CON5, DIP_A_MFB_MBD_CON5*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON6_
{
		volatile struct	/* 0x15022FA8 */
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
}DIP_X_REG_MFB_MBD_CON6;	/* DIP_X_MFB_MBD_CON6, DIP_A_MFB_MBD_CON6*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON7_
{
		volatile struct	/* 0x15022FAC */
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
}DIP_X_REG_MFB_MBD_CON7;	/* DIP_X_MFB_MBD_CON7, DIP_A_MFB_MBD_CON7*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON8_
{
		volatile struct	/* 0x15022FB0 */
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
}DIP_X_REG_MFB_MBD_CON8;	/* DIP_X_MFB_MBD_CON8, DIP_A_MFB_MBD_CON8*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON9_
{
		volatile struct	/* 0x15022FB4 */
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
}DIP_X_REG_MFB_MBD_CON9;	/* DIP_X_MFB_MBD_CON9, DIP_A_MFB_MBD_CON9*/

typedef volatile union _DIP_X_REG_MFB_MBD_CON10_
{
		volatile struct	/* 0x15022FB8 */
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
}DIP_X_REG_MFB_MBD_CON10;	/* DIP_X_MFB_MBD_CON10, DIP_A_MFB_MBD_CON10*/

typedef volatile union _DIP_X_REG_FLC_OFFST0_
{
		volatile struct	/* 0x15022FC0 */
		{
				FIELD  FLC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST0;	/* DIP_X_FLC_OFFST0, DIP_A_FLC_OFFST0*/

typedef volatile union _DIP_X_REG_FLC_OFFST1_
{
		volatile struct	/* 0x15022FC4 */
		{
				FIELD  FLC_OFST_G                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST1;	/* DIP_X_FLC_OFFST1, DIP_A_FLC_OFFST1*/

typedef volatile union _DIP_X_REG_FLC_OFFST2_
{
		volatile struct	/* 0x15022FC8 */
		{
				FIELD  FLC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_OFFST2;	/* DIP_X_FLC_OFFST2, DIP_A_FLC_OFFST2*/

typedef volatile union _DIP_X_REG_FLC_GAIN0_
{
		volatile struct	/* 0x15022FD0 */
		{
				FIELD  FLC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN0;	/* DIP_X_FLC_GAIN0, DIP_A_FLC_GAIN0*/

typedef volatile union _DIP_X_REG_FLC_GAIN1_
{
		volatile struct	/* 0x15022FD4 */
		{
				FIELD  FLC_GAIN_G                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN1;	/* DIP_X_FLC_GAIN1, DIP_A_FLC_GAIN1*/

typedef volatile union _DIP_X_REG_FLC_GAIN2_
{
		volatile struct	/* 0x15022FD8 */
		{
				FIELD  FLC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC_GAIN2;	/* DIP_X_FLC_GAIN2, DIP_A_FLC_GAIN2*/

typedef volatile union _DIP_X_REG_FLC2_OFFST0_
{
		volatile struct	/* 0x15022FE0 */
		{
				FIELD  FLC2_OFST_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST0;	/* DIP_X_FLC2_OFFST0, DIP_A_FLC2_OFFST0*/

typedef volatile union _DIP_X_REG_FLC2_OFFST1_
{
		volatile struct	/* 0x15022FE4 */
		{
				FIELD  FLC2_OFST_G                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST1;	/* DIP_X_FLC2_OFFST1, DIP_A_FLC2_OFFST1*/

typedef volatile union _DIP_X_REG_FLC2_OFFST2_
{
		volatile struct	/* 0x15022FE8 */
		{
				FIELD  FLC2_OFST_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_OFFST2;	/* DIP_X_FLC2_OFFST2, DIP_A_FLC2_OFFST2*/

typedef volatile union _DIP_X_REG_FLC2_GAIN0_
{
		volatile struct	/* 0x15022FF0 */
		{
				FIELD  FLC2_GAIN_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN0;	/* DIP_X_FLC2_GAIN0, DIP_A_FLC2_GAIN0*/

typedef volatile union _DIP_X_REG_FLC2_GAIN1_
{
		volatile struct	/* 0x15022FF4 */
		{
				FIELD  FLC2_GAIN_G                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN1;	/* DIP_X_FLC2_GAIN1, DIP_A_FLC2_GAIN1*/

typedef volatile union _DIP_X_REG_FLC2_GAIN2_
{
		volatile struct	/* 0x15022FF8 */
		{
				FIELD  FLC2_GAIN_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_FLC2_GAIN2;	/* DIP_X_FLC2_GAIN2, DIP_A_FLC2_GAIN2*/

typedef volatile union _DIP_X_REG_GGM_LUT_RB_
{
		volatile struct	/* 0x15023000 ~ 0x1502323F */
		{
				FIELD  GGM_R                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  GGM_B                                 : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM_LUT_RB;	/* DIP_X_GGM_LUT_RB, DIP_A_GGM_LUT_RB*/

typedef volatile union _DIP_X_REG_GGM_LUT_G_
{
		volatile struct	/* 0x15023240 ~ 0x1502347F */
		{
				FIELD  GGM_G                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM_LUT_G;	/* DIP_X_GGM_LUT_G, DIP_A_GGM_LUT_G*/

typedef volatile union _DIP_X_REG_GGM_CTRL_
{
		volatile struct	/* 0x15023480 */
		{
				FIELD  GGM_LNR                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  GGM_END_VAR                           : 10;		/*  1..10, 0x000007FE */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  GGM_RMP_VAR                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_GGM_CTRL;	/* DIP_X_GGM_CTRL, DIP_A_GGM_CTRL*/

typedef volatile union _DIP_X_REG_PCA_TBL_
{
		volatile struct	/* 0x15023800 ~ 0x15023D9F */
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
		volatile struct	/* 0x15023E00 */
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
		volatile struct	/* 0x15023E04 */
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
		volatile struct	/* 0x15023E08 */
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
		volatile struct	/* 0x15023E0C */
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
		volatile struct	/* 0x15023E10 */
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
		volatile struct	/* 0x15023E14 */
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
		volatile struct	/* 0x15023E18 */
		{
				FIELD  PCA_S_TH                              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  PCA_CMAX                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  PCA_H                                 :  8;		/* 16..23, 0x00FF0000 */
				FIELD  PCA_L                                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_PCA_CON7;	/* DIP_X_PCA_CON7, DIP_A_PCA_CON7*/

typedef volatile union _DIP_X_REG_OBC2_OFFST0_
{
		volatile struct	/* 0x15023E40 */
		{
				FIELD  OBC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST0;	/* DIP_X_OBC2_OFFST0, DIP_A_OBC2_OFFST0*/

typedef volatile union _DIP_X_REG_OBC2_OFFST1_
{
		volatile struct	/* 0x15023E44 */
		{
				FIELD  OBC_OFST_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST1;	/* DIP_X_OBC2_OFFST1, DIP_A_OBC2_OFFST1*/

typedef volatile union _DIP_X_REG_OBC2_OFFST2_
{
		volatile struct	/* 0x15023E48 */
		{
				FIELD  OBC_OFST_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST2;	/* DIP_X_OBC2_OFFST2, DIP_A_OBC2_OFFST2*/

typedef volatile union _DIP_X_REG_OBC2_OFFST3_
{
		volatile struct	/* 0x15023E4C */
		{
				FIELD  OBC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_OFFST3;	/* DIP_X_OBC2_OFFST3, DIP_A_OBC2_OFFST3*/

typedef volatile union _DIP_X_REG_OBC2_GAIN0_
{
		volatile struct	/* 0x15023E50 */
		{
				FIELD  OBC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN0;	/* DIP_X_OBC2_GAIN0, DIP_A_OBC2_GAIN0*/

typedef volatile union _DIP_X_REG_OBC2_GAIN1_
{
		volatile struct	/* 0x15023E54 */
		{
				FIELD  OBC_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN1;	/* DIP_X_OBC2_GAIN1, DIP_A_OBC2_GAIN1*/

typedef volatile union _DIP_X_REG_OBC2_GAIN2_
{
		volatile struct	/* 0x15023E58 */
		{
				FIELD  OBC_GAIN_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN2;	/* DIP_X_OBC2_GAIN2, DIP_A_OBC2_GAIN2*/

typedef volatile union _DIP_X_REG_OBC2_GAIN3_
{
		volatile struct	/* 0x15023E5C */
		{
				FIELD  OBC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_OBC2_GAIN3;	/* DIP_X_OBC2_GAIN3, DIP_A_OBC2_GAIN3*/

typedef volatile union _DIP_X_REG_BNR2_BPC_CON_
{
		volatile struct	/* 0x15023E70 */
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
		volatile struct	/* 0x15023E74 */
		{
				FIELD  BPC_TH_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_Y                              : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_BLD_SLP0                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH1;	/* DIP_X_BNR2_BPC_TH1, DIP_A_BNR2_BPC_TH1*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH2_
{
		volatile struct	/* 0x15023E78 */
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
		volatile struct	/* 0x15023E7C */
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
		volatile struct	/* 0x15023E80 */
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
		volatile struct	/* 0x15023E84 */
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
		volatile struct	/* 0x15023E88 */
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
		volatile struct	/* 0x15023E8C */
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
		volatile struct	/* 0x15023E90 */
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
		volatile struct	/* 0x15023E94 */
		{
				FIELD  BPC_C_TH_LWB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_Y                            : 12;		/* 12..23, 0x00FFF000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_BPC_TH1_C;	/* DIP_X_BNR2_BPC_TH1_C, DIP_A_BNR2_BPC_TH1_C*/

typedef volatile union _DIP_X_REG_BNR2_BPC_TH2_C_
{
		volatile struct	/* 0x15023E98 */
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
		volatile struct	/* 0x15023E9C */
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
		volatile struct	/* 0x15023EA0 */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR1_CT_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_NR1_CON;	/* DIP_X_BNR2_NR1_CON, DIP_A_BNR2_NR1_CON*/

typedef volatile union _DIP_X_REG_BNR2_NR1_CT_CON_
{
		volatile struct	/* 0x15023EA4 */
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

typedef volatile union _DIP_X_REG_BNR2_RSV1_
{
		volatile struct	/* 0x15023EA8 */
		{
				FIELD  RSV1                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_RSV1;	/* DIP_X_BNR2_RSV1, DIP_A_BNR2_RSV1*/

typedef volatile union _DIP_X_REG_BNR2_RSV2_
{
		volatile struct	/* 0x15023EAC */
		{
				FIELD  RSV2                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_RSV2;	/* DIP_X_BNR2_RSV2, DIP_A_BNR2_RSV2*/

typedef volatile union _DIP_X_REG_BNR2_PDC_CON_
{
		volatile struct	/* 0x15023EB0 */
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
		volatile struct	/* 0x15023EB4 */
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
		volatile struct	/* 0x15023EB8 */
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
		volatile struct	/* 0x15023EBC */
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
		volatile struct	/* 0x15023EC0 */
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
		volatile struct	/* 0x15023EC4 */
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
		volatile struct	/* 0x15023EC8 */
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
		volatile struct	/* 0x15023ECC */
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
		volatile struct	/* 0x15023ED0 */
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
		volatile struct	/* 0x15023ED4 */
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
		volatile struct	/* 0x15023ED8 */
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
		volatile struct	/* 0x15023EDC */
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
		volatile struct	/* 0x15023EE0 */
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
		volatile struct	/* 0x15023EE4 */
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
		volatile struct	/* 0x15023EE8 */
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
		volatile struct	/* 0x15023EEC */
		{
				FIELD  PDC_XCENTER                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDC_YCENTER                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_BNR2_PDC_POS;	/* DIP_X_BNR2_PDC_POS, DIP_A_BNR2_PDC_POS*/

typedef volatile union _DIP_X_REG_RMG2_HDR_CFG_
{
		volatile struct	/* 0x15023EF0 */
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
		volatile struct	/* 0x15023EF4 */
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
		volatile struct	/* 0x15023EF8 */
		{
				FIELD  RMG_HDR_THK                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMG2_HDR_CFG2;	/* DIP_X_RMG2_HDR_CFG2, DIP_A_RMG2_HDR_CFG2*/

typedef volatile union _DIP_X_REG_RMM2_OSC_
{
		volatile struct	/* 0x15023F10 */
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
		volatile struct	/* 0x15023F14 */
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
		volatile struct	/* 0x15023F18 */
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
		volatile struct	/* 0x15023F1C */
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
		volatile struct	/* 0x15023F20 */
		{
				FIELD  RMM_LEOS_GRAY                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_LEOS;	/* DIP_X_RMM2_LEOS, DIP_A_RMM2_LEOS*/

typedef volatile union _DIP_X_REG_RMM2_MC2_
{
		volatile struct	/* 0x15023F24 */
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
		volatile struct	/* 0x15023F28 */
		{
				FIELD  RMM_DIFF_LB                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_DIFF_LB;	/* DIP_X_RMM2_DIFF_LB, DIP_A_RMM2_DIFF_LB*/

typedef volatile union _DIP_X_REG_RMM2_MA_
{
		volatile struct	/* 0x15023F2C */
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
		volatile struct	/* 0x15023F30 */
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
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RMM2_TUNE;	/* DIP_X_RMM2_TUNE, DIP_A_RMM2_TUNE*/

typedef volatile union _DIP_X_REG_RCP2_CROP_CON1_
{
		volatile struct	/* 0x15023F60 */
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
		volatile struct	/* 0x15023F64 */
		{
				FIELD  RCP_STR_Y                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_Y                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RCP2_CROP_CON2;	/* DIP_X_RCP2_CROP_CON2, DIP_A_RCP2_CROP_CON2*/

typedef volatile union _DIP_X_REG_LSC2_CTL1_
{
		volatile struct	/* 0x15023F70 */
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
		volatile struct	/* 0x15023F74 */
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
		volatile struct	/* 0x15023F78 */
		{
				FIELD  LSC_SDBLK_HEIGHT                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  LSC_SDBLK_YNUM                        :  5;		/* 13..17, 0x0003E000 */
				FIELD  LSC_SPARE                             : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_LSC2_CTL3;	/* DIP_X_LSC2_CTL3, DIP_A_LSC2_CTL3*/

typedef volatile union _DIP_X_REG_LSC2_LBLOCK_
{
		volatile struct	/* 0x15023F7C */
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
		volatile struct	/* 0x15023F80 */
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
		volatile struct	/* 0x15023F84 */
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
		volatile struct	/* 0x15023F88 */
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
		volatile struct	/* 0x15023F8C */
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
		volatile struct	/* 0x15023F90 */
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

typedef volatile union _DIP_X_REG_DBS2_SIGMA_
{
		volatile struct	/* 0x15023FA0 */
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
		volatile struct	/* 0x15023FA4 */
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
		volatile struct	/* 0x15023FA8 */
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
		volatile struct	/* 0x15023FAC */
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
		volatile struct	/* 0x15023FB0 */
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
		volatile struct	/* 0x15023FB4 */
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
		volatile struct	/* 0x15023FB8 */
		{
				FIELD  DBS_HDR_OSCTH                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_CTL_2;	/* DIP_X_DBS2_CTL_2, DIP_A_DBS2_CTL_2*/

typedef volatile union _DIP_X_REG_DBS2_SIGMA_2_
{
		volatile struct	/* 0x15023FBC */
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
		volatile struct	/* 0x15023FC0 */
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
		volatile struct	/* 0x15023FC4 */
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
		volatile struct	/* 0x15023FC8 */
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
		volatile struct	/* 0x15023FCC */
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
		volatile struct	/* 0x15023FD0 */
		{
				FIELD  DBS_SL_G3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_DBS2_SL_G34;	/* DIP_X_DBS2_SL_G34, DIP_A_DBS2_SL_G34*/

typedef volatile union _DIP_X_REG_ANR_TBL_
{
		volatile struct	/* 0x15024000 ~ 0x150243FF */
		{
				FIELD  ANR_TBL_Y0                            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  ANR_TBL_U0                            :  5;		/*  5.. 9, 0x000003E0 */
				FIELD  ANR_TBL_V0                            :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_TBL_Y1                            :  5;		/* 16..20, 0x001F0000 */
				FIELD  ANR_TBL_U1                            :  5;		/* 21..25, 0x03E00000 */
				FIELD  ANR_TBL_V1                            :  5;		/* 26..30, 0x7C000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_TBL;	/* DIP_X_ANR_TBL, DIP_A_ANR_TBL*/

typedef volatile union _DIP_X_REG_ANR_CON1_
{
		volatile struct	/* 0x15024400 */
		{
				FIELD  ANR_ENC                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ANR_ENY                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  ANR_SCALE_MODE                        :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_Y_DIR                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  ANR_TILE_EDGE                         :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR_LCE_LINK                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  ANR_TABLE_EN                          :  1;		/* 28..28, 0x10000000 */
				FIELD  ANR_TBL_PRC                           :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_CON1;	/* DIP_X_ANR_CON1, DIP_A_ANR_CON1*/

typedef volatile union _DIP_X_REG_ANR_CON2_
{
		volatile struct	/* 0x15024404 */
		{
				FIELD  ANR_IMPL_MODE                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  ANR_C_SM_EDGE                         :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  ANR_FLT_C                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_LR                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR_ALPHA                             :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR_C_SM_EDGE_TH                      :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_CON2;	/* DIP_X_ANR_CON2, DIP_A_ANR_CON2*/

typedef volatile union _DIP_X_REG_ANR_YAD1_
{
		volatile struct	/* 0x15024408 */
		{
				FIELD  ANR_CEN_GAIN_LO_TH                    :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_CEN_GAIN_HI_TH                    :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_K_LO_TH                           :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR_K_HI_TH                           :  4;		/* 24..27, 0x0F000000 */
				FIELD  ANR_K_TH_C                            :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YAD1;	/* DIP_X_ANR_YAD1, DIP_A_ANR_YAD1*/

typedef volatile union _DIP_X_REG_ANR_YAD2_
{
		volatile struct	/* 0x1502440C */
		{
				FIELD  ANR_PTY_VGAIN                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR_PTY_GAIN_TH                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_K_RAT                             :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YAD2;	/* DIP_X_ANR_YAD2, DIP_A_ANR_YAD2*/

typedef volatile union _DIP_X_REG_ANR_Y4LUT1_
{
		volatile struct	/* 0x15024410 */
		{
				FIELD  ANR_Y_CPX1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_Y_CPX2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_Y_CPX3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_Y4LUT1;	/* DIP_X_ANR_Y4LUT1, DIP_A_ANR_Y4LUT1*/

typedef volatile union _DIP_X_REG_ANR_Y4LUT2_
{
		volatile struct	/* 0x15024414 */
		{
				FIELD  ANR_Y_SCALE_CPY0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_Y_SCALE_CPY1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_Y_SCALE_CPY2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_Y_SCALE_CPY3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_Y4LUT2;	/* DIP_X_ANR_Y4LUT2, DIP_A_ANR_Y4LUT2*/

typedef volatile union _DIP_X_REG_ANR_Y4LUT3_
{
		volatile struct	/* 0x15024418 */
		{
				FIELD  ANR_Y_SCALE_SP0                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_Y_SCALE_SP1                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_Y_SCALE_SP2                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_Y_SCALE_SP3                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_Y4LUT3;	/* DIP_X_ANR_Y4LUT3, DIP_A_ANR_Y4LUT3*/

typedef volatile union _DIP_X_REG_ANR_C4LUT1_
{
		volatile struct	/* 0x1502441C */
		{
				FIELD  ANR_C_CPX1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_C_CPX2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_C_CPX3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_C4LUT1;	/* DIP_X_ANR_C4LUT1, DIP_A_ANR_C4LUT1*/

typedef volatile union _DIP_X_REG_ANR_C4LUT2_
{
		volatile struct	/* 0x15024420 */
		{
				FIELD  ANR_C_SCALE_CPY0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_C_SCALE_CPY1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_C_SCALE_CPY2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_C_SCALE_CPY3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_C4LUT2;	/* DIP_X_ANR_C4LUT2, DIP_A_ANR_C4LUT2*/

typedef volatile union _DIP_X_REG_ANR_C4LUT3_
{
		volatile struct	/* 0x15024424 */
		{
				FIELD  ANR_C_SCALE_SP0                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_C_SCALE_SP1                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_C_SCALE_SP2                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_C_SCALE_SP3                       :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_C4LUT3;	/* DIP_X_ANR_C4LUT3, DIP_A_ANR_C4LUT3*/

typedef volatile union _DIP_X_REG_ANR_A4LUT2_
{
		volatile struct	/* 0x15024428 */
		{
				FIELD  ANR_Y_ACT_CPY0                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_Y_ACT_CPY1                        :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_Y_ACT_CPY2                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR_Y_ACT_CPY3                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_A4LUT2;	/* DIP_X_ANR_A4LUT2, DIP_A_ANR_A4LUT2*/

typedef volatile union _DIP_X_REG_ANR_A4LUT3_
{
		volatile struct	/* 0x1502442C */
		{
				FIELD  ANR_Y_ACT_SP0                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_Y_ACT_SP1                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR_Y_ACT_SP2                         :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR_Y_ACT_SP3                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_A4LUT3;	/* DIP_X_ANR_A4LUT3, DIP_A_ANR_A4LUT3*/

typedef volatile union _DIP_X_REG_ANR_L4LUT1_
{
		volatile struct	/* 0x15024430 */
		{
				FIELD  ANR_LCE_X1                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_LCE_X2                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_LCE_X3                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_L4LUT1;	/* DIP_X_ANR_L4LUT1, DIP_A_ANR_L4LUT1*/

typedef volatile union _DIP_X_REG_ANR_L4LUT2_
{
		volatile struct	/* 0x15024434 */
		{
				FIELD  ANR_LCE_GAIN0                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_LCE_GAIN1                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR_LCE_GAIN2                         :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR_LCE_GAIN3                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_L4LUT2;	/* DIP_X_ANR_L4LUT2, DIP_A_ANR_L4LUT2*/

typedef volatile union _DIP_X_REG_ANR_L4LUT3_
{
		volatile struct	/* 0x15024438 */
		{
				FIELD  ANR_LCE_SP0                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_LCE_SP1                           :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR_LCE_SP2                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR_LCE_SP3                           :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_L4LUT3;	/* DIP_X_ANR_L4LUT3, DIP_A_ANR_L4LUT3*/

typedef volatile union _DIP_X_REG_ANR_PTY_
{
		volatile struct	/* 0x1502443C */
		{
				FIELD  ANR_PTY1                              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_PTY2                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_PTY3                              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR_PTY4                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_PTY;	/* DIP_X_ANR_PTY, DIP_A_ANR_PTY*/

typedef volatile union _DIP_X_REG_ANR_CAD_
{
		volatile struct	/* 0x15024440 */
		{
				FIELD  ANR_PTC_VGAIN                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR_PTC_GAIN_TH                       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_C_L_DIFF_TH                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_CAD;	/* DIP_X_ANR_CAD, DIP_A_ANR_CAD*/

typedef volatile union _DIP_X_REG_ANR_PTC_
{
		volatile struct	/* 0x15024444 */
		{
				FIELD  ANR_PTC1                              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_PTC2                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_PTC3                              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR_PTC4                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_PTC;	/* DIP_X_ANR_PTC, DIP_A_ANR_PTC*/

typedef volatile union _DIP_X_REG_ANR_LCE_
{
		volatile struct	/* 0x15024448 */
		{
				FIELD  ANR_LCE_C_GAIN                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR_LCE_SCALE_GAIN                    :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  ANR_LM_WT                             :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_LCE;	/* DIP_X_ANR_LCE, DIP_A_ANR_LCE*/

typedef volatile union _DIP_X_REG_ANR_T4LUT1_
{
		volatile struct	/* 0x15024460 */
		{
				FIELD  ANR_TBL_CPX1                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_TBL_CPX2                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_TBL_CPX3                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_T4LUT1;	/* DIP_X_ANR_T4LUT1, DIP_A_ANR_T4LUT1*/

typedef volatile union _DIP_X_REG_ANR_T4LUT2_
{
		volatile struct	/* 0x15024464 */
		{
				FIELD  ANR_TBL_GAIN_CPY0                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_TBL_GAIN_CPY1                     :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_TBL_GAIN_CPY2                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_TBL_GAIN_CPY3                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_T4LUT2;	/* DIP_X_ANR_T4LUT2, DIP_A_ANR_T4LUT2*/

typedef volatile union _DIP_X_REG_ANR_T4LUT3_
{
		volatile struct	/* 0x15024468 */
		{
				FIELD  ANR_TBL_GAIN_SP0                      :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_TBL_GAIN_SP1                      :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_TBL_GAIN_SP2                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_TBL_GAIN_SP3                      :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_T4LUT3;	/* DIP_X_ANR_T4LUT3, DIP_A_ANR_T4LUT3*/

typedef volatile union _DIP_X_REG_ANR_ACT1_
{
		volatile struct	/* 0x1502446C */
		{
				FIELD  ANR_ACT_LCE_GAIN                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 16;		/*  4..19, 0x000FFFF0 */
				FIELD  ANR_ACT_DIF_GAIN                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  ANR_ACT_DIF_LO_TH                     :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACT1;	/* DIP_X_ANR_ACT1, DIP_A_ANR_ACT1*/

typedef volatile union _DIP_X_REG_ANR_ACT2_
{
		volatile struct	/* 0x15024470 */
		{
				FIELD  ANR_ACT_DIF_HI_TH                     :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_ACT_SIZE_GAIN                     :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  ANR_ACT_SIZE_LO_TH                    :  5;		/* 12..16, 0x0001F000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACT2;	/* DIP_X_ANR_ACT2, DIP_A_ANR_ACT2*/

typedef volatile union _DIP_X_REG_ANR_ACT4_
{
		volatile struct	/* 0x15024478 */
		{
				FIELD  ANR_Y_ACT_CEN_OFT                     :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_Y_ACT_CEN_GAIN                    :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR_Y_ACT_CEN_TH                      :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACT4;	/* DIP_X_ANR_ACT4, DIP_A_ANR_ACT4*/

typedef volatile union _DIP_X_REG_ANR_ACTYHL_
{
		volatile struct	/* 0x1502447C */
		{
				FIELD  ANR_ACT_TH_Y_HL                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_ACT_BLD_BASE_Y_HL                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_ACT_SLANT_Y_HL                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_ACT_BLD_TH_Y_HL                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYHL;	/* DIP_X_ANR_ACTYHL, DIP_A_ANR_ACTYHL*/

typedef volatile union _DIP_X_REG_ANR_ACTYHH_
{
		volatile struct	/* 0x15024480 */
		{
				FIELD  ANR_ACT_TH_Y_HH                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_ACT_BLD_BASE_Y_HH                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_ACT_SLANT_Y_HH                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_ACT_BLD_TH_Y_HH                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYHH;	/* DIP_X_ANR_ACTYHH, DIP_A_ANR_ACTYHH*/

typedef volatile union _DIP_X_REG_ANR_ACTYL_
{
		volatile struct	/* 0x15024484 */
		{
				FIELD  ANR_ACT_TH_Y_L                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_ACT_BLD_BASE_Y_L                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_ACT_SLANT_Y_L                     :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_ACT_BLD_TH_Y_L                    :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYL;	/* DIP_X_ANR_ACTYL, DIP_A_ANR_ACTYL*/

typedef volatile union _DIP_X_REG_ANR_ACTYHL2_
{
		volatile struct	/* 0x15024488 */
		{
				FIELD  ANR_LUMA_TH_Y_HL                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_LUMA_BLD_BASE_Y_HL                :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_LUMA_SLANT_Y_HL                   :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_LUMA_BLD_TH_Y_HL                  :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYHL2;	/* DIP_X_ANR_ACTYHL2, DIP_A_ANR_ACTYHL2*/

typedef volatile union _DIP_X_REG_ANR_ACTYHH2_
{
		volatile struct	/* 0x1502448C */
		{
				FIELD  ANR_LUMA_TH_Y_HH                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_LUMA_BLD_BASE_Y_HH                :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_LUMA_SLANT_Y_HH                   :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_LUMA_BLD_TH_Y_HH                  :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYHH2;	/* DIP_X_ANR_ACTYHH2, DIP_A_ANR_ACTYHH2*/

typedef volatile union _DIP_X_REG_ANR_ACTYL2_
{
		volatile struct	/* 0x15024490 */
		{
				FIELD  ANR_LUMA_TH_Y_L                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_LUMA_BLD_BASE_Y_L                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_LUMA_SLANT_Y_L                    :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_LUMA_BLD_TH_Y_L                   :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTYL2;	/* DIP_X_ANR_ACTYL2, DIP_A_ANR_ACTYL2*/

typedef volatile union _DIP_X_REG_ANR_ACTC_
{
		volatile struct	/* 0x15024494 */
		{
				FIELD  RSV                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_ACT_BLD_BASE_C                    :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ANR_C_DITH_U                          :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  ANR_C_DITH_V                          :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_ACTC;	/* DIP_X_ANR_ACTC, DIP_A_ANR_ACTC*/

typedef volatile union _DIP_X_REG_ANR_YLAD_
{
		volatile struct	/* 0x15024498 */
		{
				FIELD  ANR_CEN_GAIN_LO_TH_LPF                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ANR_CEN_GAIN_HI_TH_LPF                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  ANR_K_LMT                             :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  ANR_K_LPF                             :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR_K_HPF                             :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YLAD;	/* DIP_X_ANR_YLAD, DIP_A_ANR_YLAD*/

typedef volatile union _DIP_X_REG_ANR_YLAD2_
{
		volatile struct	/* 0x1502449C */
		{
				FIELD  ANR_PTY_L_GAIN                        :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  ANR_PTY_H_GAIN                        :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  ANR_BLEND_TH_L                        :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  ANR_BLEND_TH_H                        :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YLAD2;	/* DIP_X_ANR_YLAD2, DIP_A_ANR_YLAD2*/

typedef volatile union _DIP_X_REG_ANR_YLAD3_
{
		volatile struct	/* 0x150244A0 */
		{
				FIELD  ANR_HI_LMT1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_HI_LMT2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_LO_LMT1                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR_LO_LMT2                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YLAD3;	/* DIP_X_ANR_YLAD3, DIP_A_ANR_YLAD3*/

typedef volatile union _DIP_X_REG_ANR_PTYL_
{
		volatile struct	/* 0x150244A4 */
		{
				FIELD  ANR_PTY1_LPF                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  ANR_PTY2_LPF                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  ANR_PTY3_LPF                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  ANR_PTY4_LPF                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_PTYL;	/* DIP_X_ANR_PTYL, DIP_A_ANR_PTYL*/

typedef volatile union _DIP_X_REG_ANR_LCOEF_
{
		volatile struct	/* 0x150244A8 */
		{
				FIELD  ANR_COEF_A                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR_COEF_B                            :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  ANR_COEF_C                            :  4;		/*  8..11, 0x00000F00 */
				FIELD  ANR_COEF_D                            :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_LCOEF;	/* DIP_X_ANR_LCOEF, DIP_A_ANR_LCOEF*/

typedef volatile union _DIP_X_REG_ANR_YDIR_
{
		volatile struct	/* 0x150244AC */
		{
				FIELD  ANR_Y_DIR_GAIN                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  ANR_Y_DIR_VER_W_TH                    :  6;		/*  4.. 9, 0x000003F0 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  ANR_Y_DIR_VER_W_SL                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  ANR_Y_DIR_DIAG_W_TH                   :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  ANR_Y_DIR_DIAG_W_SL                   :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_YDIR;	/* DIP_X_ANR_YDIR, DIP_A_ANR_YDIR*/

typedef volatile union _DIP_X_REG_ANR_RSV1_
{
		volatile struct	/* 0x150244B0 */
		{
				FIELD  ANR_RSV1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_ANR_RSV1;	/* DIP_X_ANR_RSV1, DIP_A_ANR_RSV1*/

typedef volatile union _DIP_X_REG_SL2G_CEN_
{
		volatile struct	/* 0x15024500 */
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
		volatile struct	/* 0x15024504 */
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
		volatile struct	/* 0x15024508 */
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
		volatile struct	/* 0x1502450C */
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
		volatile struct	/* 0x15024510 */
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
		volatile struct	/* 0x15024514 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_XOFF;	/* DIP_X_SL2G_XOFF, DIP_A_SL2G_XOFF*/

typedef volatile union _DIP_X_REG_SL2G_YOFF_
{
		volatile struct	/* 0x15024518 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_YOFF;	/* DIP_X_SL2G_YOFF, DIP_A_SL2G_YOFF*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON0_
{
		volatile struct	/* 0x1502451C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON0;	/* DIP_X_SL2G_SLP_CON0, DIP_A_SL2G_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON1_
{
		volatile struct	/* 0x15024520 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON1;	/* DIP_X_SL2G_SLP_CON1, DIP_A_SL2G_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON2_
{
		volatile struct	/* 0x15024524 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON2;	/* DIP_X_SL2G_SLP_CON2, DIP_A_SL2G_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2G_SLP_CON3_
{
		volatile struct	/* 0x15024528 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SLP_CON3;	/* DIP_X_SL2G_SLP_CON3, DIP_A_SL2G_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2G_SIZE_
{
		volatile struct	/* 0x1502452C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2G_SIZE;	/* DIP_X_SL2G_SIZE, DIP_A_SL2G_SIZE*/

typedef volatile union _DIP_X_REG_SL2H_CEN_
{
		volatile struct	/* 0x15024540 */
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
		volatile struct	/* 0x15024544 */
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
		volatile struct	/* 0x15024548 */
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
		volatile struct	/* 0x1502454C */
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
		volatile struct	/* 0x15024550 */
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
		volatile struct	/* 0x15024554 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_XOFF;	/* DIP_X_SL2H_XOFF, DIP_A_SL2H_XOFF*/

typedef volatile union _DIP_X_REG_SL2H_YOFF_
{
		volatile struct	/* 0x15024558 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_YOFF;	/* DIP_X_SL2H_YOFF, DIP_A_SL2H_YOFF*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON0_
{
		volatile struct	/* 0x1502455C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON0;	/* DIP_X_SL2H_SLP_CON0, DIP_A_SL2H_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON1_
{
		volatile struct	/* 0x15024560 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON1;	/* DIP_X_SL2H_SLP_CON1, DIP_A_SL2H_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON2_
{
		volatile struct	/* 0x15024564 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON2;	/* DIP_X_SL2H_SLP_CON2, DIP_A_SL2H_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2H_SLP_CON3_
{
		volatile struct	/* 0x15024568 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SLP_CON3;	/* DIP_X_SL2H_SLP_CON3, DIP_A_SL2H_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2H_SIZE_
{
		volatile struct	/* 0x1502456C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2H_SIZE;	/* DIP_X_SL2H_SIZE, DIP_A_SL2H_SIZE*/

typedef volatile union _DIP_X_REG_HLR2_CFG_
{
		volatile struct	/* 0x15024580 */
		{
				FIELD  HLR_DTH                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  HLR_DSL                               : 10;		/*  8..17, 0x0003FF00 */
				FIELD  HLR_MODE                              :  1;		/* 18..18, 0x00040000 */
				FIELD  HLR_SL_EN                             :  1;		/* 19..19, 0x00080000 */
				FIELD  HLR_RAT                               :  6;		/* 20..25, 0x03F00000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  HLR_EDGE                              :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_CFG;	/* DIP_X_HLR2_CFG, DIP_A_HLR2_CFG*/

typedef volatile union _DIP_X_REG_HLR2_GAIN_
{
		volatile struct	/* 0x15024588 */
		{
				FIELD  HLR_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HLR_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_GAIN;	/* DIP_X_HLR2_GAIN, DIP_A_HLR2_GAIN*/

typedef volatile union _DIP_X_REG_HLR2_GAIN_1_
{
		volatile struct	/* 0x1502458C */
		{
				FIELD  HLR_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HLR_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_GAIN_1;	/* DIP_X_HLR2_GAIN_1, DIP_A_HLR2_GAIN_1*/

typedef volatile union _DIP_X_REG_HLR2_OFST_
{
		volatile struct	/* 0x15024590 */
		{
				FIELD  HLR_OFST_B                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  HLR_OFST_GB                           : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_OFST;	/* DIP_X_HLR2_OFST, DIP_A_HLR2_OFST*/

typedef volatile union _DIP_X_REG_HLR2_OFST_1_
{
		volatile struct	/* 0x15024594 */
		{
				FIELD  HLR_OFST_GR                           : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  HLR_OFST_R                            : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_OFST_1;	/* DIP_X_HLR2_OFST_1, DIP_A_HLR2_OFST_1*/

typedef volatile union _DIP_X_REG_HLR2_IVGN_
{
		volatile struct	/* 0x15024598 */
		{
				FIELD  HLR_IVGN_B                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  HLR_IVGN_GB                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_IVGN;	/* DIP_X_HLR2_IVGN, DIP_A_HLR2_IVGN*/

typedef volatile union _DIP_X_REG_HLR2_IVGN_1_
{
		volatile struct	/* 0x1502459C */
		{
				FIELD  HLR_IVGN_GR                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  HLR_IVGN_R                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_IVGN_1;	/* DIP_X_HLR2_IVGN_1, DIP_A_HLR2_IVGN_1*/

typedef volatile union _DIP_X_REG_HLR2_KC_
{
		volatile struct	/* 0x150245A0 */
		{
				FIELD  HLR_KC_C0                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  HLR_KC_C1                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_KC;	/* DIP_X_HLR2_KC, DIP_A_HLR2_KC*/

typedef volatile union _DIP_X_REG_HLR2_CFG_1_
{
		volatile struct	/* 0x150245A4 */
		{
				FIELD  HLR_STR                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_CFG_1;	/* DIP_X_HLR2_CFG_1, DIP_A_HLR2_CFG_1*/

typedef volatile union _DIP_X_REG_HLR2_SL_PARA_
{
		volatile struct	/* 0x150245A8 */
		{
				FIELD  HLR_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  HLR_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_SL_PARA;	/* DIP_X_HLR2_SL_PARA, DIP_A_HLR2_SL_PARA*/

typedef volatile union _DIP_X_REG_HLR2_SL_PARA_1_
{
		volatile struct	/* 0x150245AC */
		{
				FIELD  HLR_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  HLR_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HLR2_SL_PARA_1;	/* DIP_X_HLR2_SL_PARA_1, DIP_A_HLR2_SL_PARA_1*/

typedef volatile union _DIP_X_REG_RNR_CON1_
{
		volatile struct	/* 0x150245B0 */
		{
				FIELD  RNR_RAT                               :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RNR_EPS                               :  8;		/*  9..16, 0x0001FE00 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_CON1;	/* DIP_X_RNR_CON1, DIP_A_RNR_CON1*/

typedef volatile union _DIP_X_REG_RNR_VC0_
{
		volatile struct	/* 0x150245B4 */
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
		volatile struct	/* 0x150245B8 */
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
		volatile struct	/* 0x150245BC */
		{
				FIELD  RNR_OFST_B                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_B                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_B;	/* DIP_X_RNR_GO_B, DIP_A_RNR_GO_B*/

typedef volatile union _DIP_X_REG_RNR_GO_G_
{
		volatile struct	/* 0x150245C0 */
		{
				FIELD  RNR_OFST_G                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_G                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_G;	/* DIP_X_RNR_GO_G, DIP_A_RNR_GO_G*/

typedef volatile union _DIP_X_REG_RNR_GO_R_
{
		volatile struct	/* 0x150245C4 */
		{
				FIELD  RNR_OFST_R                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  RNR_GAIN_R                            : 13;		/* 15..27, 0x0FFF8000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GO_R;	/* DIP_X_RNR_GO_R, DIP_A_RNR_GO_R*/

typedef volatile union _DIP_X_REG_RNR_GAIN_ISO_
{
		volatile struct	/* 0x150245C8 */
		{
				FIELD  RNR_GAIN_ISO                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_GAIN_ISO;	/* DIP_X_RNR_GAIN_ISO, DIP_A_RNR_GAIN_ISO*/

typedef volatile union _DIP_X_REG_RNR_L_
{
		volatile struct	/* 0x150245CC */
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
		volatile struct	/* 0x150245D0 */
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
		volatile struct	/* 0x150245D4 */
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
		volatile struct	/* 0x150245D8 */
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
		volatile struct	/* 0x150245DC */
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
		volatile struct	/* 0x150245E0 */
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
		volatile struct	/* 0x150245E4 */
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
		volatile struct	/* 0x150245E8 */
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
		volatile struct	/* 0x150245EC */
		{
				FIELD  RNR_TILE_EDGE                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_RNR_TILE_EDGE;	/* DIP_X_RNR_TILE_EDGE, DIP_A_RNR_TILE_EDGE*/

typedef volatile union _DIP_X_REG_HFG_CON_0_
{
		volatile struct	/* 0x15024630 */
		{
				FIELD  DIP_HFC_STD                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CON_0;	/* DIP_X_HFG_CON_0, DIP_A_HFG_CON_0*/

typedef volatile union _DIP_X_REG_HFG_LUMA_0_
{
		volatile struct	/* 0x15024634 */
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
		volatile struct	/* 0x15024638 */
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
		volatile struct	/* 0x1502463C */
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
		volatile struct	/* 0x15024640 */
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
		volatile struct	/* 0x15024644 */
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
		volatile struct	/* 0x15024648 */
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
		volatile struct	/* 0x1502464C */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_0;	/* DIP_X_HFG_RAN_0, DIP_A_HFG_RAN_0*/

typedef volatile union _DIP_X_REG_HFG_RAN_1_
{
		volatile struct	/* 0x15024650 */
		{
				FIELD  RANSED_SEED1                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED2                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_1;	/* DIP_X_HFG_RAN_1, DIP_A_HFG_RAN_1*/

typedef volatile union _DIP_X_REG_HFG_RAN_2_
{
		volatile struct	/* 0x15024654 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_2;	/* DIP_X_HFG_RAN_2, DIP_A_HFG_RAN_2*/

typedef volatile union _DIP_X_REG_HFG_RAN_3_
{
		volatile struct	/* 0x15024658 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_3;	/* DIP_X_HFG_RAN_3, DIP_A_HFG_RAN_3*/

typedef volatile union _DIP_X_REG_HFG_RAN_4_
{
		volatile struct	/* 0x1502465C */
		{
				FIELD  HFG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_RAN_4;	/* DIP_X_HFG_RAN_4, DIP_A_HFG_RAN_4*/

typedef volatile union _DIP_X_REG_HFG_CROP_X_
{
		volatile struct	/* 0x15024660 */
		{
				FIELD  HFG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CROP_X;	/* DIP_X_HFG_CROP_X, DIP_A_HFG_CROP_X*/

typedef volatile union _DIP_X_REG_HFG_CROP_Y_
{
		volatile struct	/* 0x15024664 */
		{
				FIELD  HFG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  HFG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_HFG_CROP_Y;	/* DIP_X_HFG_CROP_Y, DIP_A_HFG_CROP_Y*/

typedef volatile union _DIP_X_REG_SL2I_CEN_
{
		volatile struct	/* 0x15024670 */
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
		volatile struct	/* 0x15024674 */
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
		volatile struct	/* 0x15024678 */
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
		volatile struct	/* 0x1502467C */
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
		volatile struct	/* 0x15024680 */
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
		volatile struct	/* 0x15024684 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_XOFF;	/* DIP_X_SL2I_XOFF, DIP_A_SL2I_XOFF*/

typedef volatile union _DIP_X_REG_SL2I_YOFF_
{
		volatile struct	/* 0x15024688 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_YOFF;	/* DIP_X_SL2I_YOFF, DIP_A_SL2I_YOFF*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON0_
{
		volatile struct	/* 0x1502468C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON0;	/* DIP_X_SL2I_SLP_CON0, DIP_A_SL2I_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON1_
{
		volatile struct	/* 0x15024690 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON1;	/* DIP_X_SL2I_SLP_CON1, DIP_A_SL2I_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON2_
{
		volatile struct	/* 0x15024694 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON2;	/* DIP_X_SL2I_SLP_CON2, DIP_A_SL2I_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2I_SLP_CON3_
{
		volatile struct	/* 0x15024698 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SLP_CON3;	/* DIP_X_SL2I_SLP_CON3, DIP_A_SL2I_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2I_SIZE_
{
		volatile struct	/* 0x1502469C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2I_SIZE;	/* DIP_X_SL2I_SIZE, DIP_A_SL2I_SIZE*/

typedef volatile union _DIP_X_REG_SL2K_CEN_
{
		volatile struct	/* 0x150246B0 */
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
		volatile struct	/* 0x150246B4 */
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
		volatile struct	/* 0x150246B8 */
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
		volatile struct	/* 0x150246BC */
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
		volatile struct	/* 0x150246C0 */
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
		volatile struct	/* 0x150246C4 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_XOFF;	/* DIP_X_SL2K_XOFF, DIP_A_SL2K_XOFF*/

typedef volatile union _DIP_X_REG_SL2K_YOFF_
{
		volatile struct	/* 0x150246C8 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_YOFF;	/* DIP_X_SL2K_YOFF, DIP_A_SL2K_YOFF*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON0_
{
		volatile struct	/* 0x150246CC */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON0;	/* DIP_X_SL2K_SLP_CON0, DIP_A_SL2K_SLP_CON0*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON1_
{
		volatile struct	/* 0x150246D0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON1;	/* DIP_X_SL2K_SLP_CON1, DIP_A_SL2K_SLP_CON1*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON2_
{
		volatile struct	/* 0x150246D4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON2;	/* DIP_X_SL2K_SLP_CON2, DIP_A_SL2K_SLP_CON2*/

typedef volatile union _DIP_X_REG_SL2K_SLP_CON3_
{
		volatile struct	/* 0x150246D8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SLP_CON3;	/* DIP_X_SL2K_SLP_CON3, DIP_A_SL2K_SLP_CON3*/

typedef volatile union _DIP_X_REG_SL2K_SIZE_
{
		volatile struct	/* 0x150246DC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_SL2K_SIZE;	/* DIP_X_SL2K_SIZE, DIP_A_SL2K_SIZE*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CFG_MAIN_
{
		volatile struct	/* 0x150246F0 */
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
		volatile struct	/* 0x150246F4 */
		{
				FIELD  COLOR_PIC_H_CNT                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_PXL_CNT_MAIN;	/* DIP_X_CAM_COLOR_PXL_CNT_MAIN, DIP_A_CAM_COLOR_PXL_CNT_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN_
{
		volatile struct	/* 0x150246F8 */
		{
				FIELD  COLOR_LINE_CNT                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_PIC_V_CNT                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN;	/* DIP_X_CAM_COLOR_LINE_CNT_MAIN, DIP_A_CAM_COLOR_LINE_CNT_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_WIN_X_MAIN_
{
		volatile struct	/* 0x150246FC */
		{
				FIELD  COLOR_WIN_X_START                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_WIN_X_END                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_WIN_X_MAIN;	/* DIP_X_CAM_COLOR_WIN_X_MAIN, DIP_A_CAM_COLOR_WIN_X_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_WIN_Y_MAIN_
{
		volatile struct	/* 0x15024700 */
		{
				FIELD  COLOR_WIN_Y_START                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_WIN_Y_END                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_WIN_Y_MAIN;	/* DIP_X_CAM_COLOR_WIN_Y_MAIN, DIP_A_CAM_COLOR_WIN_Y_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0_
{
		volatile struct	/* 0x15024704 */
		{
				FIELD  COLOR_HTOTAL_CNT                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_VTOTAL_CNT                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0;	/* DIP_X_CAM_COLOR_TIMING_DETECTION_0, DIP_A_CAM_COLOR_TIMING_DETECTION_0*/

typedef volatile union _DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1_
{
		volatile struct	/* 0x15024708 */
		{
				FIELD  COLOR_HDE_CNT                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_VDE_CNT                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1;	/* DIP_X_CAM_COLOR_TIMING_DETECTION_1, DIP_A_CAM_COLOR_TIMING_DETECTION_1*/

typedef volatile union _DIP_X_REG_CAM_COLOR_DBG_CFG_MAIN_
{
		volatile struct	/* 0x1502470C */
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
		volatile struct	/* 0x15024710 */
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
		volatile struct	/* 0x15024714 */
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
		volatile struct	/* 0x15024718 */
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
		volatile struct	/* 0x1502471C */
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
		volatile struct	/* 0x15024720 */
		{
				FIELD  COLOR_G_SATURATION                    : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_2;	/* DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_2*/

typedef volatile union _DIP_X_REG_CAM_COLOR_POS_MAIN_
{
		volatile struct	/* 0x15024724 */
		{
				FIELD  COLOR_POS_X                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_POS_Y                           : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_POS_MAIN;	/* DIP_X_CAM_COLOR_POS_MAIN, DIP_A_CAM_COLOR_POS_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_
{
		volatile struct	/* 0x15024728 */
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
		volatile struct	/* 0x1502472C */
		{
				FIELD  COLOR_INK_DATA_V                      : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_INK_DATA_MAIN_CR, DIP_A_CAM_COLOR_INK_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_
{
		volatile struct	/* 0x15024730 */
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
		volatile struct	/* 0x15024734 */
		{
				FIELD  COLOR_CAP_IN_DATA_V                   : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_
{
		volatile struct	/* 0x15024738 */
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
		volatile struct	/* 0x1502473C */
		{
				FIELD  COLOR_CAP_OUT_DATA_V                  : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_CR;	/* DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN_CR*/

typedef volatile union _DIP_X_REG_CAM_COLOR_Y_SLOPE_1_0_MAIN_
{
		volatile struct	/* 0x15024740 */
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
		volatile struct	/* 0x15024744 */
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
		volatile struct	/* 0x15024748 */
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
		volatile struct	/* 0x1502474C */
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
		volatile struct	/* 0x15024750 */
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
		volatile struct	/* 0x15024754 */
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
		volatile struct	/* 0x15024758 */
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
		volatile struct	/* 0x1502475C */
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
		volatile struct	/* 0x15024760 */
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
		volatile struct	/* 0x15024764 */
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
		volatile struct	/* 0x15024768 */
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
		volatile struct	/* 0x1502476C */
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
		volatile struct	/* 0x15024770 */
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
		volatile struct	/* 0x15024774 */
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
		volatile struct	/* 0x15024778 */
		{
				FIELD  COLOR_W1_HIT_OUTER                    : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_TWO_D_W1_RESULT;	/* DIP_X_CAM_COLOR_TWO_D_W1_RESULT, DIP_A_CAM_COLOR_TWO_D_W1_RESULT*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN_
{
		volatile struct	/* 0x1502477C */
		{
				FIELD  COLOR_HIST_X_START                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_HIST_X_END                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN;	/* DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN, DIP_A_CAM_COLOR_SAT_HIST_X_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN_
{
		volatile struct	/* 0x15024780 */
		{
				FIELD  COLOR_HIST_Y_START                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  COLOR_HIST_Y_END                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN;	/* DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN, DIP_A_CAM_COLOR_SAT_HIST_Y_CFG_MAIN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_BWS_2_
{
		volatile struct	/* 0x15024784 */
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
		volatile struct	/* 0x15024788 */
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
		volatile struct	/* 0x1502478C */
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
		volatile struct	/* 0x15024790 */
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
		volatile struct	/* 0x15024794 */
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
		volatile struct	/* 0x15024798 */
		{
				FIELD  COLOR_CRC_RESULT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CRC_4;	/* DIP_X_CAM_COLOR_CRC_4, DIP_A_CAM_COLOR_CRC_4*/

typedef volatile union _DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_0_
{
		volatile struct	/* 0x1502479C */
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
		volatile struct	/* 0x150247A0 */
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
		volatile struct	/* 0x150247A4 */
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
		volatile struct	/* 0x150247A8 */
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
		volatile struct	/* 0x150247AC */
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
		volatile struct	/* 0x150247B0 */
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
		volatile struct	/* 0x150247B4 */
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
		volatile struct	/* 0x150247B8 */
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
		volatile struct	/* 0x150247BC */
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
		volatile struct	/* 0x150247C0 */
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
		volatile struct	/* 0x150247C4 */
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
		volatile struct	/* 0x150247C8 */
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
		volatile struct	/* 0x150247CC */
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
		volatile struct	/* 0x150247D0 */
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
		volatile struct	/* 0x150247D4 */
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
		volatile struct	/* 0x150247D8 */
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
		volatile struct	/* 0x150247DC */
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
		volatile struct	/* 0x150247E0 */
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
		volatile struct	/* 0x150247E4 */
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
		volatile struct	/* 0x150247E8 */
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
		volatile struct	/* 0x150247EC */
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
		volatile struct	/* 0x150247F0 */
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
		volatile struct	/* 0x150247F4 */
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
		volatile struct	/* 0x150247F8 */
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
		volatile struct	/* 0x150247FC */
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
		volatile struct	/* 0x15024800 */
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
		volatile struct	/* 0x15024804 */
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
		volatile struct	/* 0x15024808 */
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
		volatile struct	/* 0x1502480C */
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
		volatile struct	/* 0x15024810 */
		{
				FIELD  COLOR_DISP_COLOR_FRAME_DONE_DEL       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_FRAME_DONE_DEL;	/* DIP_X_CAM_COLOR_FRAME_DONE_DEL, DIP_A_CAM_COLOR_FRAME_DONE_DEL*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CRC_
{
		volatile struct	/* 0x15024814 */
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
		volatile struct	/* 0x15024818 */
		{
				FIELD  COLOR_DISP_COLOR_SW_SCRATCH           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_SW_SCRATCH;	/* DIP_X_CAM_COLOR_SW_SCRATCH, DIP_A_CAM_COLOR_SW_SCRATCH*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CK_ON_
{
		volatile struct	/* 0x1502481C */
		{
				FIELD  COLOR_DISP_COLOR_CK_ON                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CK_ON;	/* DIP_X_CAM_COLOR_CK_ON, DIP_A_CAM_COLOR_CK_ON*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH_
{
		volatile struct	/* 0x15024820 */
		{
				FIELD  COLOR_DISP_COLOR_INTERNAL_IP_WIDTH    : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH;	/* DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH, DIP_A_CAM_COLOR_INTERNAL_IP_WIDTH*/

typedef volatile union _DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT_
{
		volatile struct	/* 0x15024824 */
		{
				FIELD  COLOR_DISP_COLOR_INTERNAL_IP_HEIGHT   : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT;	/* DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT, DIP_A_CAM_COLOR_INTERNAL_IP_HEIGHT*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM1_EN_
{
		volatile struct	/* 0x15024828 */
		{
				FIELD  COLOR_DISP_COLOR_CM1_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  COLOR_DISP_COLOR_CM1_CLIP_EN          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM1_EN;	/* DIP_X_CAM_COLOR_CM1_EN, DIP_A_CAM_COLOR_CM1_EN*/

typedef volatile union _DIP_X_REG_CAM_COLOR_CM2_EN_
{
		volatile struct	/* 0x1502482C */
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
		volatile struct	/* 0x15024830 */
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
		volatile struct	/* 0x15024834 */
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
		volatile struct	/* 0x15024838 */
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
		volatile struct	/* 0x1502483C */
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
		volatile struct	/* 0x15024840 */
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
		volatile struct	/* 0x15024844 */
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
		volatile struct	/* 0x15024848 */
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
		volatile struct	/* 0x1502484C */
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
		volatile struct	/* 0x15024850 */
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
		volatile struct	/* 0x15024854 */
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
		volatile struct	/* 0x15024858 */
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
		volatile struct	/* 0x1502485C */
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
		volatile struct	/* 0x15024860 */
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
		volatile struct	/* 0x15024864 */
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
		volatile struct	/* 0x15024868 */
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
		volatile struct	/* 0x1502486C */
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
		volatile struct	/* 0x15024870 */
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
		volatile struct	/* 0x15024874 */
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
		volatile struct	/* 0x15024878 */
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
		volatile struct	/* 0x1502487C */
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
		volatile struct	/* 0x15024880 */
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
		volatile struct	/* 0x15024884 */
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
		volatile struct	/* 0x15024888 */
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
		volatile struct	/* 0x1502488C */
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
		volatile struct	/* 0x15024890 */
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
		volatile struct	/* 0x15024894 */
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
		volatile struct	/* 0x15024898 */
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
		volatile struct	/* 0x1502489C */
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
		volatile struct	/* 0x150248A0 */
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
		volatile struct	/* 0x150248A4 */
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
		volatile struct	/* 0x150248A8 */
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
		volatile struct	/* 0x150248AC */
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
		volatile struct	/* 0x150248B0 */
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
		volatile struct	/* 0x150248B4 */
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
		volatile struct	/* 0x150248B8 */
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
		volatile struct	/* 0x150248BC */
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
		volatile struct	/* 0x150248C0 */
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
		volatile struct	/* 0x150248C4 */
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
		volatile struct	/* 0x150248C8 */
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
		volatile struct	/* 0x150248CC */
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
		volatile struct	/* 0x150248D0 */
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
		volatile struct	/* 0x150248D4 */
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
		volatile struct	/* 0x150248D8 */
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
		volatile struct	/* 0x150248DC */
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
		volatile struct	/* 0x150248E0 */
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
		volatile struct	/* 0x150248E4 */
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
		volatile struct	/* 0x150248E8 */
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
		volatile struct	/* 0x150248EC */
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
		volatile struct	/* 0x150248F0 */
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
		volatile struct	/* 0x150248F4 */
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
		volatile struct	/* 0x150248F8 */
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
		volatile struct	/* 0x150248FC */
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
		volatile struct	/* 0x15024900 */
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
		volatile struct	/* 0x15024904 */
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
		volatile struct	/* 0x15024908 */
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
		volatile struct	/* 0x1502490C */
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
		volatile struct	/* 0x15024910 */
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
		volatile struct	/* 0x15024914 */
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
		volatile struct	/* 0x15024918 */
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
		volatile struct	/* 0x1502491C */
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
		volatile struct	/* 0x15024920 */
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
		volatile struct	/* 0x15024924 */
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
		volatile struct	/* 0x15024928 */
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
		volatile struct	/* 0x1502492C */
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
		volatile struct	/* 0x15024930 */
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
		volatile struct	/* 0x15024934 */
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
		volatile struct	/* 0x15024938 */
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
		volatile struct	/* 0x1502493C */
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
		volatile struct	/* 0x15024940 */
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
		volatile struct	/* 0x15024944 */
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
		volatile struct	/* 0x15024948 */
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
		volatile struct	/* 0x1502494C */
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
		volatile struct	/* 0x15024950 */
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
		volatile struct	/* 0x15024954 */
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
		volatile struct	/* 0x15024958 */
		{
				FIELD  COLOR_CM_S_W3_WGT_LSLOPE              : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  COLOR_CM_S_W3_WGT_USLOPE              : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_CAM_COLOR_CM_W3_SAT_4;	/* DIP_X_CAM_COLOR_CM_W3_SAT_4, DIP_A_CAM_COLOR_CM_W3_SAT_4*/

typedef volatile union _DIP_X_REG_NDG_RAN_0_
{
		volatile struct	/* 0x15024960 */
		{
				FIELD  RANSED_GSEED                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_SEED0                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_0;	/* DIP_X_NDG_RAN_0, DIP_A_NDG_RAN_0*/

typedef volatile union _DIP_X_REG_NDG_RAN_1_
{
		volatile struct	/* 0x15024964 */
		{
				FIELD  RANSED_XOSFS                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RANSED_YOSFS                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_1;	/* DIP_X_NDG_RAN_1, DIP_A_NDG_RAN_1*/

typedef volatile union _DIP_X_REG_NDG_RAN_2_
{
		volatile struct	/* 0x15024968 */
		{
				FIELD  RANSED_IMG_WD                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_2;	/* DIP_X_NDG_RAN_2, DIP_A_NDG_RAN_2*/

typedef volatile union _DIP_X_REG_NDG_RAN_3_
{
		volatile struct	/* 0x1502496C */
		{
				FIELD  NDG_T_WIDTH                           : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_T_HEIGHT                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_RAN_3;	/* DIP_X_NDG_RAN_3, DIP_A_NDG_RAN_3*/

typedef volatile union _DIP_X_REG_NDG_CROP_X_
{
		volatile struct	/* 0x15024970 */
		{
				FIELD  NDG_CROP_STR_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_X                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_CROP_X;	/* DIP_X_NDG_CROP_X, DIP_A_NDG_CROP_X*/

typedef volatile union _DIP_X_REG_NDG_CROP_Y_
{
		volatile struct	/* 0x15024974 */
		{
				FIELD  NDG_CROP_STR_Y                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  NDG_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_NDG_CROP_Y;	/* DIP_X_NDG_CROP_Y, DIP_A_NDG_CROP_Y*/

typedef volatile union _DIP_X_REG_AMD_RESET_
{
		volatile struct	/* 0x15024E00 */
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
		volatile struct	/* 0x15024E04 */
		{
				FIELD  AMD_EN                                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  INT_CLR_MODE                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_TOP_CTL;	/* DIP_X_AMD_TOP_CTL, DIP_A_AMD_TOP_CTL*/

typedef volatile union _DIP_X_REG_AMD_INT_EN_
{
		volatile struct	/* 0x15024E08 */
		{
				FIELD  MDP_CROP_INT_EN                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AMD_INT_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_INT_EN;	/* DIP_X_AMD_INT_EN, DIP_A_AMD_INT_EN*/

typedef volatile union _DIP_X_REG_AMD_INT_ST_
{
		volatile struct	/* 0x15024E0C */
		{
				FIELD  MDP_CROP_INT_ST                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AMD_INT_ST                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_INT_ST;	/* DIP_X_AMD_INT_ST, DIP_A_AMD_INT_ST*/

typedef volatile union _DIP_X_REG_AMD_C24_CTL_
{
		volatile struct	/* 0x15024E10 */
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
		volatile struct	/* 0x15024E14 */
		{
				FIELD  MDP_CROP_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_0;	/* DIP_X_AMD_CROP_CTL_0, DIP_A_AMD_CROP_CTL_0*/

typedef volatile union _DIP_X_REG_AMD_CROP_CTL_1_
{
		volatile struct	/* 0x15024E18 */
		{
				FIELD  MDP_CROP_START_X                      : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_START_Y                      : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_1;	/* DIP_X_AMD_CROP_CTL_1, DIP_A_AMD_CROP_CTL_1*/

typedef volatile union _DIP_X_REG_AMD_CROP_CTL_2_
{
		volatile struct	/* 0x15024E1C */
		{
				FIELD  MDP_CROP_END_X                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  MDP_CROP_END_Y                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_CTL_2;	/* DIP_X_AMD_CROP_CTL_2, DIP_A_AMD_CROP_CTL_2*/

typedef volatile union _DIP_X_REG_AMD_DCM_DIS_
{
		volatile struct	/* 0x15024E20 */
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
		volatile struct	/* 0x15024E24 */
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
		volatile struct	/* 0x15024E30 */
		{
				FIELD  C24_DEBUG_SEL                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  MDP_CROP_DEBUG_SEL                    :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_DEBUG_SEL;	/* DIP_X_AMD_DEBUG_SEL, DIP_A_AMD_DEBUG_SEL*/

typedef volatile union _DIP_X_REG_AMD_C24_DEBUG_
{
		volatile struct	/* 0x15024E34 */
		{
				FIELD  C24_DEBUG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_C24_DEBUG;	/* DIP_X_AMD_C24_DEBUG, DIP_A_AMD_C24_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_CROP_DEBUG_
{
		volatile struct	/* 0x15024E38 */
		{
				FIELD  MDP_CROP_DEBUG                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_CROP_DEBUG;	/* DIP_X_AMD_CROP_DEBUG, DIP_A_AMD_CROP_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_ADL_DEBUG_
{
		volatile struct	/* 0x15024E3C */
		{
				FIELD  ADL_DEBUG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_ADL_DEBUG;	/* DIP_X_AMD_ADL_DEBUG, DIP_A_AMD_ADL_DEBUG*/

typedef volatile union _DIP_X_REG_AMD_SOF_
{
		volatile struct	/* 0x15024E40 */
		{
				FIELD  AMD_SOF                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_SOF;	/* DIP_X_AMD_SOF, DIP_A_AMD_SOF*/

typedef volatile union _DIP_X_REG_AMD_REQ_ST_
{
		volatile struct	/* 0x15024E44 */
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
		volatile struct	/* 0x15024E48 */
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
		volatile struct	/* 0x15024E70 */
		{
				FIELD  AMD_SPARE                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DIP_X_REG_AMD_SPARE;	/* DIP_X_AMD_SPARE, DIP_A_AMD_SPARE*/

typedef volatile union _DIP_X_REG_ISO_ADAP_
{
    volatile struct /* 0x15023000 */
    {
            FIELD  ISO_ADAPTIVE          : 32;      /*  0..31, 0xFFFFFFFF */
    } Bits;
    UINT32 Raw;
}DIP_X_REG_ISO_ADAP;

typedef volatile union _DIP_X_REG_LCE25_TPIPE_SLM_
{
        volatile struct /* 0x15026494 */
        {
                FIELD  LCE_TPIPE_SLM_WD                      : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCE_TPIPE_SLM_HT                      : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_SLM;

typedef volatile union _DIP_X_REG_LCE25_TPIPE_OFFSET_
{
        volatile struct /* 0x15026498 */
        {
                FIELD  LCE_TPIPE_OFFSET_X                    : 16;      /*  0..15, 0x0000FFFF */
                FIELD  LCE_TPIPE_OFFSET_Y                    : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_OFFSET;

typedef volatile union _DIP_X_REG_LCE25_TPIPE_OUT_
{
        volatile struct /* 0x1502649C */
        {
                FIELD  LCE_TPIPE_OUT_HT                      : 16;      /*  0..15, 0x0000FFFF */
                FIELD  rsv_16                                : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_LCE25_TPIPE_OUT;

typedef volatile union _DIP_X_REG_NR3D_VIPI_SIZE_
{
        volatile struct /* 0x150264xx */
        {
                FIELD  NR3D_VIPI_WIDTH                       : 16;      /*  0..15, 0x0000FFFF */
                FIELD  NR3D_VIPI_HEIGHT                      : 16;      /* 16..31, 0xFFFF0000 */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_NR3D_VIPI_SIZE;

typedef volatile union _DIP_X_REG_NR3D_VIPI_OFFSET_
{
        volatile struct /* 0x150264xx */
        {
                FIELD  NR3D_VIPI_OFFSET                      : 32;      /*  0..31, 0xFFFFFFFF */
        } Bits;
        UINT32 Raw;
}DIP_X_REG_NR3D_VIPI_OFFSET;

typedef volatile struct _dip_x_reg_t_	/* 0x15022000..0x1502505C */
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
	UINT32                                          rsv_00E4[7];                                     /* 00E4..00FF, 0x150220E4..150220FF */
	DIP_X_REG_CQ_EN                                 DIP_X_CQ_EN;                                     /* 0100, 0x15022100, DIP_A_CQ_EN */
	DIP_X_REG_CQ_THR0_CTL                           DIP_X_CQ_THR0_CTL;                               /* 0104, 0x15022104, DIP_A_CQ_THR0_CTL */
	DIP_X_REG_CQ_THR0_BASEADDR                      DIP_X_CQ_THR0_BASEADDR;                          /* 0108, 0x15022108, DIP_A_CQ_THR0_BASEADDR */
	DIP_X_REG_CQ_THR0_DESC_SIZE                     DIP_X_CQ_THR0_DESC_SIZE;                         /* 010C, 0x1502210C, DIP_A_CQ_THR0_DESC_SIZE */
	DIP_X_REG_CQ_THR1_CTL                           DIP_X_CQ_THR1_CTL;                               /* 0110, 0x15022110, DIP_A_CQ_THR1_CTL */
	DIP_X_REG_CQ_THR1_BASEADDR                      DIP_X_CQ_THR1_BASEADDR;                          /* 0114, 0x15022114, DIP_A_CQ_THR1_BASEADDR */
	DIP_X_REG_CQ_THR1_DESC_SIZE                     DIP_X_CQ_THR1_DESC_SIZE;                         /* 0118, 0x15022118, DIP_A_CQ_THR1_DESC_SIZE */
	DIP_X_REG_CQ_THR2_CTL                           DIP_X_CQ_THR2_CTL;                               /* 011C, 0x1502211C, DIP_A_CQ_THR2_CTL */
	DIP_X_REG_CQ_THR2_BASEADDR                      DIP_X_CQ_THR2_BASEADDR;                          /* 0120, 0x15022120, DIP_A_CQ_THR2_BASEADDR */
	DIP_X_REG_CQ_THR2_DESC_SIZE                     DIP_X_CQ_THR2_DESC_SIZE;                         /* 0124, 0x15022124, DIP_A_CQ_THR2_DESC_SIZE */
	DIP_X_REG_CQ_THR3_CTL                           DIP_X_CQ_THR3_CTL;                               /* 0128, 0x15022128, DIP_A_CQ_THR3_CTL */
	DIP_X_REG_CQ_THR3_BASEADDR                      DIP_X_CQ_THR3_BASEADDR;                          /* 012C, 0x1502212C, DIP_A_CQ_THR3_BASEADDR */
	DIP_X_REG_CQ_THR3_DESC_SIZE                     DIP_X_CQ_THR3_DESC_SIZE;                         /* 0130, 0x15022130, DIP_A_CQ_THR3_DESC_SIZE */
	DIP_X_REG_CQ_THR4_CTL                           DIP_X_CQ_THR4_CTL;                               /* 0134, 0x15022134, DIP_A_CQ_THR4_CTL */
	DIP_X_REG_CQ_THR4_BASEADDR                      DIP_X_CQ_THR4_BASEADDR;                          /* 0138, 0x15022138, DIP_A_CQ_THR4_BASEADDR */
	DIP_X_REG_CQ_THR4_DESC_SIZE                     DIP_X_CQ_THR4_DESC_SIZE;                         /* 013C, 0x1502213C, DIP_A_CQ_THR4_DESC_SIZE */
	DIP_X_REG_CQ_THR5_CTL                           DIP_X_CQ_THR5_CTL;                               /* 0140, 0x15022140, DIP_A_CQ_THR5_CTL */
	DIP_X_REG_CQ_THR5_BASEADDR                      DIP_X_CQ_THR5_BASEADDR;                          /* 0144, 0x15022144, DIP_A_CQ_THR5_BASEADDR */
	DIP_X_REG_CQ_THR5_DESC_SIZE                     DIP_X_CQ_THR5_DESC_SIZE;                         /* 0148, 0x15022148, DIP_A_CQ_THR5_DESC_SIZE */
	DIP_X_REG_CQ_THR6_CTL                           DIP_X_CQ_THR6_CTL;                               /* 014C, 0x1502214C, DIP_A_CQ_THR6_CTL */
	DIP_X_REG_CQ_THR6_BASEADDR                      DIP_X_CQ_THR6_BASEADDR;                          /* 0150, 0x15022150, DIP_A_CQ_THR6_BASEADDR */
	DIP_X_REG_CQ_THR6_DESC_SIZE                     DIP_X_CQ_THR6_DESC_SIZE;                         /* 0154, 0x15022154, DIP_A_CQ_THR6_DESC_SIZE */
	DIP_X_REG_CQ_THR7_CTL                           DIP_X_CQ_THR7_CTL;                               /* 0158, 0x15022158, DIP_A_CQ_THR7_CTL */
	DIP_X_REG_CQ_THR7_BASEADDR                      DIP_X_CQ_THR7_BASEADDR;                          /* 015C, 0x1502215C, DIP_A_CQ_THR7_BASEADDR */
	DIP_X_REG_CQ_THR7_DESC_SIZE                     DIP_X_CQ_THR7_DESC_SIZE;                         /* 0160, 0x15022160, DIP_A_CQ_THR7_DESC_SIZE */
	DIP_X_REG_CQ_THR8_CTL                           DIP_X_CQ_THR8_CTL;                               /* 0164, 0x15022164, DIP_A_CQ_THR8_CTL */
	DIP_X_REG_CQ_THR8_BASEADDR                      DIP_X_CQ_THR8_BASEADDR;                          /* 0168, 0x15022168, DIP_A_CQ_THR8_BASEADDR */
	DIP_X_REG_CQ_THR8_DESC_SIZE                     DIP_X_CQ_THR8_DESC_SIZE;                         /* 016C, 0x1502216C, DIP_A_CQ_THR8_DESC_SIZE */
	DIP_X_REG_CQ_THR9_CTL                           DIP_X_CQ_THR9_CTL;                               /* 0170, 0x15022170, DIP_A_CQ_THR9_CTL */
	DIP_X_REG_CQ_THR9_BASEADDR                      DIP_X_CQ_THR9_BASEADDR;                          /* 0174, 0x15022174, DIP_A_CQ_THR9_BASEADDR */
	DIP_X_REG_CQ_THR9_DESC_SIZE                     DIP_X_CQ_THR9_DESC_SIZE;                         /* 0178, 0x15022178, DIP_A_CQ_THR9_DESC_SIZE */
	DIP_X_REG_CQ_THR10_CTL                          DIP_X_CQ_THR10_CTL;                              /* 017C, 0x1502217C, DIP_A_CQ_THR10_CTL */
	DIP_X_REG_CQ_THR10_BASEADDR                     DIP_X_CQ_THR10_BASEADDR;                         /* 0180, 0x15022180, DIP_A_CQ_THR10_BASEADDR */
	DIP_X_REG_CQ_THR10_DESC_SIZE                    DIP_X_CQ_THR10_DESC_SIZE;                        /* 0184, 0x15022184, DIP_A_CQ_THR10_DESC_SIZE */
	DIP_X_REG_CQ_THR11_CTL                          DIP_X_CQ_THR11_CTL;                              /* 0188, 0x15022188, DIP_A_CQ_THR11_CTL */
	DIP_X_REG_CQ_THR11_BASEADDR                     DIP_X_CQ_THR11_BASEADDR;                         /* 018C, 0x1502218C, DIP_A_CQ_THR11_BASEADDR */
	DIP_X_REG_CQ_THR11_DESC_SIZE                    DIP_X_CQ_THR11_DESC_SIZE;                        /* 0190, 0x15022190, DIP_A_CQ_THR11_DESC_SIZE */
	DIP_X_REG_CQ_THR12_CTL                          DIP_X_CQ_THR12_CTL;                              /* 0194, 0x15022194, DIP_A_CQ_THR12_CTL */
	DIP_X_REG_CQ_THR12_BASEADDR                     DIP_X_CQ_THR12_BASEADDR;                         /* 0198, 0x15022198, DIP_A_CQ_THR12_BASEADDR */
	DIP_X_REG_CQ_THR12_DESC_SIZE                    DIP_X_CQ_THR12_DESC_SIZE;                        /* 019C, 0x1502219C, DIP_A_CQ_THR12_DESC_SIZE */
	DIP_X_REG_CQ_THR13_CTL                          DIP_X_CQ_THR13_CTL;                              /* 01A0, 0x150221A0, DIP_A_CQ_THR13_CTL */
	DIP_X_REG_CQ_THR13_BASEADDR                     DIP_X_CQ_THR13_BASEADDR;                         /* 01A4, 0x150221A4, DIP_A_CQ_THR13_BASEADDR */
	DIP_X_REG_CQ_THR13_DESC_SIZE                    DIP_X_CQ_THR13_DESC_SIZE;                        /* 01A8, 0x150221A8, DIP_A_CQ_THR13_DESC_SIZE */
	DIP_X_REG_CQ_THR14_CTL                          DIP_X_CQ_THR14_CTL;                              /* 01AC, 0x150221AC, DIP_A_CQ_THR14_CTL */
	DIP_X_REG_CQ_THR14_BASEADDR                     DIP_X_CQ_THR14_BASEADDR;                         /* 01B0, 0x150221B0, DIP_A_CQ_THR14_BASEADDR */
	DIP_X_REG_CQ_THR14_DESC_SIZE                    DIP_X_CQ_THR14_DESC_SIZE;                        /* 01B4, 0x150221B4, DIP_A_CQ_THR14_DESC_SIZE */
	DIP_X_REG_CQ_THR15_CTL                          DIP_X_CQ_THR15_CTL;                              /* 01B8, 0x150221B8, DIP_A_CQ_THR15_CTL */
	DIP_X_REG_CQ_THR15_BASEADDR                     DIP_X_CQ_THR15_BASEADDR;                         /* 01BC, 0x150221BC, DIP_A_CQ_THR15_BASEADDR */
	DIP_X_REG_CQ_THR15_DESC_SIZE                    DIP_X_CQ_THR15_DESC_SIZE;                        /* 01C0, 0x150221C0, DIP_A_CQ_THR15_DESC_SIZE */
	DIP_X_REG_CQ_THR16_CTL                          DIP_X_CQ_THR16_CTL;                              /* 01C4, 0x150221C4, DIP_A_CQ_THR16_CTL */
	DIP_X_REG_CQ_THR16_BASEADDR                     DIP_X_CQ_THR16_BASEADDR;                         /* 01C8, 0x150221C8, DIP_A_CQ_THR16_BASEADDR */
	DIP_X_REG_CQ_THR16_DESC_SIZE                    DIP_X_CQ_THR16_DESC_SIZE;                        /* 01CC, 0x150221CC, DIP_A_CQ_THR16_DESC_SIZE */
	DIP_X_REG_CQ_THR17_CTL                          DIP_X_CQ_THR17_CTL;                              /* 01D0, 0x150221D0, DIP_A_CQ_THR17_CTL */
	DIP_X_REG_CQ_THR17_BASEADDR                     DIP_X_CQ_THR17_BASEADDR;                         /* 01D4, 0x150221D4, DIP_A_CQ_THR17_BASEADDR */
	DIP_X_REG_CQ_THR17_DESC_SIZE                    DIP_X_CQ_THR17_DESC_SIZE;                        /* 01D8, 0x150221D8, DIP_A_CQ_THR17_DESC_SIZE */
	DIP_X_REG_CQ_THR18_CTL                          DIP_X_CQ_THR18_CTL;                              /* 01DC, 0x150221DC, DIP_A_CQ_THR18_CTL */
	DIP_X_REG_CQ_THR18_BASEADDR                     DIP_X_CQ_THR18_BASEADDR;                         /* 01E0, 0x150221E0, DIP_A_CQ_THR18_BASEADDR */
	DIP_X_REG_CQ_THR18_DESC_SIZE                    DIP_X_CQ_THR18_DESC_SIZE;                        /* 01E4, 0x150221E4, DIP_A_CQ_THR18_DESC_SIZE */
	UINT32                                          rsv_01E8[6];                                     /* 01E8..01FF, 0x150221E8..150221FF */
	DIP_X_REG_DMA_SOFT_RSTSTAT                      DIP_X_DMA_SOFT_RSTSTAT;                          /* 0200, 0x15022200, DIP_A_DMA_SOFT_RSTSTAT */
	DIP_X_REG_TDRI_BASE_ADDR                        DIP_X_TDRI_BASE_ADDR;                            /* 0204, 0x15022204, DIP_A_TDRI_BASE_ADDR */
	DIP_X_REG_TDRI_OFST_ADDR                        DIP_X_TDRI_OFST_ADDR;                            /* 0208, 0x15022208, DIP_A_TDRI_OFST_ADDR */
	DIP_X_REG_TDRI_XSIZE                            DIP_X_TDRI_XSIZE;                                /* 020C, 0x1502220C, DIP_A_TDRI_XSIZE */
	DIP_X_REG_VERTICAL_FLIP_EN                      DIP_X_VERTICAL_FLIP_EN;                          /* 0210, 0x15022210, DIP_A_VERTICAL_FLIP_EN */
	DIP_X_REG_DMA_SOFT_RESET                        DIP_X_DMA_SOFT_RESET;                            /* 0214, 0x15022214, DIP_A_DMA_SOFT_RESET */
	DIP_X_REG_LAST_ULTRA_EN                         DIP_X_LAST_ULTRA_EN;                             /* 0218, 0x15022218, DIP_A_LAST_ULTRA_EN */
	DIP_X_REG_SPECIAL_FUN_EN                        DIP_X_SPECIAL_FUN_EN;                            /* 021C, 0x1502221C, DIP_A_SPECIAL_FUN_EN */
	UINT32                                          rsv_0220[4];                                     /* 0220..022F, 0x15022220..1502222F */
	DIP_X_REG_IMG2O_BASE_ADDR                       DIP_X_IMG2O_BASE_ADDR;                           /* 0230, 0x15022230, DIP_A_IMG2O_BASE_ADDR */
	UINT32                                          rsv_0234;                                        /* 0234, 0x15022234 */
	DIP_X_REG_IMG2O_OFST_ADDR                       DIP_X_IMG2O_OFST_ADDR;                           /* 0238, 0x15022238, DIP_A_IMG2O_OFST_ADDR */
	UINT32                                          rsv_023C;                                        /* 023C, 0x1502223C */
	DIP_X_REG_IMG2O_XSIZE                           DIP_X_IMG2O_XSIZE;                               /* 0240, 0x15022240, DIP_A_IMG2O_XSIZE */
	DIP_X_REG_IMG2O_YSIZE                           DIP_X_IMG2O_YSIZE;                               /* 0244, 0x15022244, DIP_A_IMG2O_YSIZE */
	DIP_X_REG_IMG2O_STRIDE                          DIP_X_IMG2O_STRIDE;                              /* 0248, 0x15022248, DIP_A_IMG2O_STRIDE */
	DIP_X_REG_IMG2O_CON                             DIP_X_IMG2O_CON;                                 /* 024C, 0x1502224C, DIP_A_IMG2O_CON */
	DIP_X_REG_IMG2O_CON2                            DIP_X_IMG2O_CON2;                                /* 0250, 0x15022250, DIP_A_IMG2O_CON2 */
	DIP_X_REG_IMG2O_CON3                            DIP_X_IMG2O_CON3;                                /* 0254, 0x15022254, DIP_A_IMG2O_CON3 */
	DIP_X_REG_IMG2O_CROP                            DIP_X_IMG2O_CROP;                                /* 0258, 0x15022258, DIP_A_IMG2O_CROP */
	UINT32                                          rsv_025C;                                        /* 025C, 0x1502225C */
	DIP_X_REG_IMG2BO_BASE_ADDR                      DIP_X_IMG2BO_BASE_ADDR;                          /* 0260, 0x15022260, DIP_A_IMG2BO_BASE_ADDR */
	UINT32                                          rsv_0264;                                        /* 0264, 0x15022264 */
	DIP_X_REG_IMG2BO_OFST_ADDR                      DIP_X_IMG2BO_OFST_ADDR;                          /* 0268, 0x15022268, DIP_A_IMG2BO_OFST_ADDR */
	UINT32                                          rsv_026C;                                        /* 026C, 0x1502226C */
	DIP_X_REG_IMG2BO_XSIZE                          DIP_X_IMG2BO_XSIZE;                              /* 0270, 0x15022270, DIP_A_IMG2BO_XSIZE */
	DIP_X_REG_IMG2BO_YSIZE                          DIP_X_IMG2BO_YSIZE;                              /* 0274, 0x15022274, DIP_A_IMG2BO_YSIZE */
	DIP_X_REG_IMG2BO_STRIDE                         DIP_X_IMG2BO_STRIDE;                             /* 0278, 0x15022278, DIP_A_IMG2BO_STRIDE */
	DIP_X_REG_IMG2BO_CON                            DIP_X_IMG2BO_CON;                                /* 027C, 0x1502227C, DIP_A_IMG2BO_CON */
	DIP_X_REG_IMG2BO_CON2                           DIP_X_IMG2BO_CON2;                               /* 0280, 0x15022280, DIP_A_IMG2BO_CON2 */
	DIP_X_REG_IMG2BO_CON3                           DIP_X_IMG2BO_CON3;                               /* 0284, 0x15022284, DIP_A_IMG2BO_CON3 */
	DIP_X_REG_IMG2BO_CROP                           DIP_X_IMG2BO_CROP;                               /* 0288, 0x15022288, DIP_A_IMG2BO_CROP */
	UINT32                                          rsv_028C;                                        /* 028C, 0x1502228C */
	DIP_X_REG_IMG3O_BASE_ADDR                       DIP_X_IMG3O_BASE_ADDR;                           /* 0290, 0x15022290, DIP_A_IMG3O_BASE_ADDR */
	UINT32                                          rsv_0294;                                        /* 0294, 0x15022294 */
	DIP_X_REG_IMG3O_OFST_ADDR                       DIP_X_IMG3O_OFST_ADDR;                           /* 0298, 0x15022298, DIP_A_IMG3O_OFST_ADDR */
	UINT32                                          rsv_029C;                                        /* 029C, 0x1502229C */
	DIP_X_REG_IMG3O_XSIZE                           DIP_X_IMG3O_XSIZE;                               /* 02A0, 0x150222A0, DIP_A_IMG3O_XSIZE */
	DIP_X_REG_IMG3O_YSIZE                           DIP_X_IMG3O_YSIZE;                               /* 02A4, 0x150222A4, DIP_A_IMG3O_YSIZE */
	DIP_X_REG_IMG3O_STRIDE                          DIP_X_IMG3O_STRIDE;                              /* 02A8, 0x150222A8, DIP_A_IMG3O_STRIDE */
	DIP_X_REG_IMG3O_CON                             DIP_X_IMG3O_CON;                                 /* 02AC, 0x150222AC, DIP_A_IMG3O_CON */
	DIP_X_REG_IMG3O_CON2                            DIP_X_IMG3O_CON2;                                /* 02B0, 0x150222B0, DIP_A_IMG3O_CON2 */
	DIP_X_REG_IMG3O_CON3                            DIP_X_IMG3O_CON3;                                /* 02B4, 0x150222B4, DIP_A_IMG3O_CON3 */
	DIP_X_REG_IMG3O_CROP                            DIP_X_IMG3O_CROP;                                /* 02B8, 0x150222B8, DIP_A_IMG3O_CROP */
	UINT32                                          rsv_02BC;                                        /* 02BC, 0x150222BC */
	DIP_X_REG_IMG3BO_BASE_ADDR                      DIP_X_IMG3BO_BASE_ADDR;                          /* 02C0, 0x150222C0, DIP_A_IMG3BO_BASE_ADDR */
	UINT32                                          rsv_02C4;                                        /* 02C4, 0x150222C4 */
	DIP_X_REG_IMG3BO_OFST_ADDR                      DIP_X_IMG3BO_OFST_ADDR;                          /* 02C8, 0x150222C8, DIP_A_IMG3BO_OFST_ADDR */
	UINT32                                          rsv_02CC;                                        /* 02CC, 0x150222CC */
	DIP_X_REG_IMG3BO_XSIZE                          DIP_X_IMG3BO_XSIZE;                              /* 02D0, 0x150222D0, DIP_A_IMG3BO_XSIZE */
	DIP_X_REG_IMG3BO_YSIZE                          DIP_X_IMG3BO_YSIZE;                              /* 02D4, 0x150222D4, DIP_A_IMG3BO_YSIZE */
	DIP_X_REG_IMG3BO_STRIDE                         DIP_X_IMG3BO_STRIDE;                             /* 02D8, 0x150222D8, DIP_A_IMG3BO_STRIDE */
	DIP_X_REG_IMG3BO_CON                            DIP_X_IMG3BO_CON;                                /* 02DC, 0x150222DC, DIP_A_IMG3BO_CON */
	DIP_X_REG_IMG3BO_CON2                           DIP_X_IMG3BO_CON2;                               /* 02E0, 0x150222E0, DIP_A_IMG3BO_CON2 */
	DIP_X_REG_IMG3BO_CON3                           DIP_X_IMG3BO_CON3;                               /* 02E4, 0x150222E4, DIP_A_IMG3BO_CON3 */
	DIP_X_REG_IMG3BO_CROP                           DIP_X_IMG3BO_CROP;                               /* 02E8, 0x150222E8, DIP_A_IMG3BO_CROP */
	UINT32                                          rsv_02EC;                                        /* 02EC, 0x150222EC */
	DIP_X_REG_IMG3CO_BASE_ADDR                      DIP_X_IMG3CO_BASE_ADDR;                          /* 02F0, 0x150222F0, DIP_A_IMG3CO_BASE_ADDR */
	UINT32                                          rsv_02F4;                                        /* 02F4, 0x150222F4 */
	DIP_X_REG_IMG3CO_OFST_ADDR                      DIP_X_IMG3CO_OFST_ADDR;                          /* 02F8, 0x150222F8, DIP_A_IMG3CO_OFST_ADDR */
	UINT32                                          rsv_02FC;                                        /* 02FC, 0x150222FC */
	DIP_X_REG_IMG3CO_XSIZE                          DIP_X_IMG3CO_XSIZE;                              /* 0300, 0x15022300, DIP_A_IMG3CO_XSIZE */
	DIP_X_REG_IMG3CO_YSIZE                          DIP_X_IMG3CO_YSIZE;                              /* 0304, 0x15022304, DIP_A_IMG3CO_YSIZE */
	DIP_X_REG_IMG3CO_STRIDE                         DIP_X_IMG3CO_STRIDE;                             /* 0308, 0x15022308, DIP_A_IMG3CO_STRIDE */
	DIP_X_REG_IMG3CO_CON                            DIP_X_IMG3CO_CON;                                /* 030C, 0x1502230C, DIP_A_IMG3CO_CON */
	DIP_X_REG_IMG3CO_CON2                           DIP_X_IMG3CO_CON2;                               /* 0310, 0x15022310, DIP_A_IMG3CO_CON2 */
	DIP_X_REG_IMG3CO_CON3                           DIP_X_IMG3CO_CON3;                               /* 0314, 0x15022314, DIP_A_IMG3CO_CON3 */
	DIP_X_REG_IMG3CO_CROP                           DIP_X_IMG3CO_CROP;                               /* 0318, 0x15022318, DIP_A_IMG3CO_CROP */
	UINT32                                          rsv_031C;                                        /* 031C, 0x1502231C */
	DIP_X_REG_FEO_BASE_ADDR                         DIP_X_FEO_BASE_ADDR;                             /* 0320, 0x15022320, DIP_A_FEO_BASE_ADDR */
	UINT32                                          rsv_0324;                                        /* 0324, 0x15022324 */
	DIP_X_REG_FEO_OFST_ADDR                         DIP_X_FEO_OFST_ADDR;                             /* 0328, 0x15022328, DIP_A_FEO_OFST_ADDR */
	UINT32                                          rsv_032C;                                        /* 032C, 0x1502232C */
	DIP_X_REG_FEO_XSIZE                             DIP_X_FEO_XSIZE;                                 /* 0330, 0x15022330, DIP_A_FEO_XSIZE */
	DIP_X_REG_FEO_YSIZE                             DIP_X_FEO_YSIZE;                                 /* 0334, 0x15022334, DIP_A_FEO_YSIZE */
	DIP_X_REG_FEO_STRIDE                            DIP_X_FEO_STRIDE;                                /* 0338, 0x15022338, DIP_A_FEO_STRIDE */
	DIP_X_REG_FEO_CON                               DIP_X_FEO_CON;                                   /* 033C, 0x1502233C, DIP_A_FEO_CON */
	DIP_X_REG_FEO_CON2                              DIP_X_FEO_CON2;                                  /* 0340, 0x15022340, DIP_A_FEO_CON2 */
	DIP_X_REG_FEO_CON3                              DIP_X_FEO_CON3;                                  /* 0344, 0x15022344, DIP_A_FEO_CON3 */
	UINT32                                          rsv_0348[2];                                     /* 0348..034F, 0x15022348..1502234F */
	DIP_X_REG_MFBO_BASE_ADDR                        DIP_X_MFBO_BASE_ADDR;                            /* 0350, 0x15022350, DIP_A_MFBO_BASE_ADDR */
	UINT32                                          rsv_0354;                                        /* 0354, 0x15022354 */
	DIP_X_REG_MFBO_OFST_ADDR                        DIP_X_MFBO_OFST_ADDR;                            /* 0358, 0x15022358, DIP_A_MFBO_OFST_ADDR */
	UINT32                                          rsv_035C;                                        /* 035C, 0x1502235C */
	DIP_X_REG_MFBO_XSIZE                            DIP_X_MFBO_XSIZE;                                /* 0360, 0x15022360, DIP_A_MFBO_XSIZE */
	DIP_X_REG_MFBO_YSIZE                            DIP_X_MFBO_YSIZE;                                /* 0364, 0x15022364, DIP_A_MFBO_YSIZE */
	DIP_X_REG_MFBO_STRIDE                           DIP_X_MFBO_STRIDE;                               /* 0368, 0x15022368, DIP_A_MFBO_STRIDE */
	DIP_X_REG_MFBO_CON                              DIP_X_MFBO_CON;                                  /* 036C, 0x1502236C, DIP_A_MFBO_CON */
	DIP_X_REG_MFBO_CON2                             DIP_X_MFBO_CON2;                                 /* 0370, 0x15022370, DIP_A_MFBO_CON2 */
	DIP_X_REG_MFBO_CON3                             DIP_X_MFBO_CON3;                                 /* 0374, 0x15022374, DIP_A_MFBO_CON3 */
	DIP_X_REG_MFBO_CROP                             DIP_X_MFBO_CROP;                                 /* 0378, 0x15022378, DIP_A_MFBO_CROP */
	UINT32                                          rsv_037C[33];                                    /* 037C..03FF, 0x1502237C..150223FF */
	DIP_X_REG_IMGI_BASE_ADDR                        DIP_X_IMGI_BASE_ADDR;                            /* 0400, 0x15022400, DIP_A_IMGI_BASE_ADDR */
	UINT32                                          rsv_0404;                                        /* 0404, 0x15022404 */
	DIP_X_REG_IMGI_OFST_ADDR                        DIP_X_IMGI_OFST_ADDR;                            /* 0408, 0x15022408, DIP_A_IMGI_OFST_ADDR */
	UINT32                                          rsv_040C;                                        /* 040C, 0x1502240C */
	DIP_X_REG_IMGI_XSIZE                            DIP_X_IMGI_XSIZE;                                /* 0410, 0x15022410, DIP_A_IMGI_XSIZE */
	DIP_X_REG_IMGI_YSIZE                            DIP_X_IMGI_YSIZE;                                /* 0414, 0x15022414, DIP_A_IMGI_YSIZE */
	DIP_X_REG_IMGI_STRIDE                           DIP_X_IMGI_STRIDE;                               /* 0418, 0x15022418, DIP_A_IMGI_STRIDE */
	DIP_X_REG_IMGI_CON                              DIP_X_IMGI_CON;                                  /* 041C, 0x1502241C, DIP_A_IMGI_CON */
	DIP_X_REG_IMGI_CON2                             DIP_X_IMGI_CON2;                                 /* 0420, 0x15022420, DIP_A_IMGI_CON2 */
	DIP_X_REG_IMGI_CON3                             DIP_X_IMGI_CON3;                                 /* 0424, 0x15022424, DIP_A_IMGI_CON3 */
	UINT32                                          rsv_0428[2];                                     /* 0428..042F, 0x15022428..1502242F */
	DIP_X_REG_IMGBI_BASE_ADDR                       DIP_X_IMGBI_BASE_ADDR;                           /* 0430, 0x15022430, DIP_A_IMGBI_BASE_ADDR */
	UINT32                                          rsv_0434;                                        /* 0434, 0x15022434 */
	DIP_X_REG_IMGBI_OFST_ADDR                       DIP_X_IMGBI_OFST_ADDR;                           /* 0438, 0x15022438, DIP_A_IMGBI_OFST_ADDR */
	UINT32                                          rsv_043C;                                        /* 043C, 0x1502243C */
	DIP_X_REG_IMGBI_XSIZE                           DIP_X_IMGBI_XSIZE;                               /* 0440, 0x15022440, DIP_A_IMGBI_XSIZE */
	DIP_X_REG_IMGBI_YSIZE                           DIP_X_IMGBI_YSIZE;                               /* 0444, 0x15022444, DIP_A_IMGBI_YSIZE */
	DIP_X_REG_IMGBI_STRIDE                          DIP_X_IMGBI_STRIDE;                              /* 0448, 0x15022448, DIP_A_IMGBI_STRIDE */
	DIP_X_REG_IMGBI_CON                             DIP_X_IMGBI_CON;                                 /* 044C, 0x1502244C, DIP_A_IMGBI_CON */
	DIP_X_REG_IMGBI_CON2                            DIP_X_IMGBI_CON2;                                /* 0450, 0x15022450, DIP_A_IMGBI_CON2 */
	DIP_X_REG_IMGBI_CON3                            DIP_X_IMGBI_CON3;                                /* 0454, 0x15022454, DIP_A_IMGBI_CON3 */
	UINT32                                          rsv_0458[2];                                     /* 0458..045F, 0x15022458..1502245F */
	DIP_X_REG_IMGCI_BASE_ADDR                       DIP_X_IMGCI_BASE_ADDR;                           /* 0460, 0x15022460, DIP_A_IMGCI_BASE_ADDR */
	UINT32                                          rsv_0464;                                        /* 0464, 0x15022464 */
	DIP_X_REG_IMGCI_OFST_ADDR                       DIP_X_IMGCI_OFST_ADDR;                           /* 0468, 0x15022468, DIP_A_IMGCI_OFST_ADDR */
	UINT32                                          rsv_046C;                                        /* 046C, 0x1502246C */
	DIP_X_REG_IMGCI_XSIZE                           DIP_X_IMGCI_XSIZE;                               /* 0470, 0x15022470, DIP_A_IMGCI_XSIZE */
	DIP_X_REG_IMGCI_YSIZE                           DIP_X_IMGCI_YSIZE;                               /* 0474, 0x15022474, DIP_A_IMGCI_YSIZE */
	DIP_X_REG_IMGCI_STRIDE                          DIP_X_IMGCI_STRIDE;                              /* 0478, 0x15022478, DIP_A_IMGCI_STRIDE */
	DIP_X_REG_IMGCI_CON                             DIP_X_IMGCI_CON;                                 /* 047C, 0x1502247C, DIP_A_IMGCI_CON */
	DIP_X_REG_IMGCI_CON2                            DIP_X_IMGCI_CON2;                                /* 0480, 0x15022480, DIP_A_IMGCI_CON2 */
	DIP_X_REG_IMGCI_CON3                            DIP_X_IMGCI_CON3;                                /* 0484, 0x15022484, DIP_A_IMGCI_CON3 */
	UINT32                                          rsv_0488[2];                                     /* 0488..048F, 0x15022488..1502248F */
	DIP_X_REG_VIPI_BASE_ADDR                        DIP_X_VIPI_BASE_ADDR;                            /* 0490, 0x15022490, DIP_A_VIPI_BASE_ADDR */
	UINT32                                          rsv_0494;                                        /* 0494, 0x15022494 */
	DIP_X_REG_VIPI_OFST_ADDR                        DIP_X_VIPI_OFST_ADDR;                            /* 0498, 0x15022498, DIP_A_VIPI_OFST_ADDR */
	UINT32                                          rsv_049C;                                        /* 049C, 0x1502249C */
	DIP_X_REG_VIPI_XSIZE                            DIP_X_VIPI_XSIZE;                                /* 04A0, 0x150224A0, DIP_A_VIPI_XSIZE */
	DIP_X_REG_VIPI_YSIZE                            DIP_X_VIPI_YSIZE;                                /* 04A4, 0x150224A4, DIP_A_VIPI_YSIZE */
	DIP_X_REG_VIPI_STRIDE                           DIP_X_VIPI_STRIDE;                               /* 04A8, 0x150224A8, DIP_A_VIPI_STRIDE */
	DIP_X_REG_VIPI_CON                              DIP_X_VIPI_CON;                                  /* 04AC, 0x150224AC, DIP_A_VIPI_CON */
	DIP_X_REG_VIPI_CON2                             DIP_X_VIPI_CON2;                                 /* 04B0, 0x150224B0, DIP_A_VIPI_CON2 */
	DIP_X_REG_VIPI_CON3                             DIP_X_VIPI_CON3;                                 /* 04B4, 0x150224B4, DIP_A_VIPI_CON3 */
	UINT32                                          rsv_04B8[2];                                     /* 04B8..04BF, 0x150224B8..150224BF */
	DIP_X_REG_VIP2I_BASE_ADDR                       DIP_X_VIP2I_BASE_ADDR;                           /* 04C0, 0x150224C0, DIP_A_VIP2I_BASE_ADDR */
	UINT32                                          rsv_04C4;                                        /* 04C4, 0x150224C4 */
	DIP_X_REG_VIP2I_OFST_ADDR                       DIP_X_VIP2I_OFST_ADDR;                           /* 04C8, 0x150224C8, DIP_A_VIP2I_OFST_ADDR */
	UINT32                                          rsv_04CC;                                        /* 04CC, 0x150224CC */
	DIP_X_REG_VIP2I_XSIZE                           DIP_X_VIP2I_XSIZE;                               /* 04D0, 0x150224D0, DIP_A_VIP2I_XSIZE */
	DIP_X_REG_VIP2I_YSIZE                           DIP_X_VIP2I_YSIZE;                               /* 04D4, 0x150224D4, DIP_A_VIP2I_YSIZE */
	DIP_X_REG_VIP2I_STRIDE                          DIP_X_VIP2I_STRIDE;                              /* 04D8, 0x150224D8, DIP_A_VIP2I_STRIDE */
	DIP_X_REG_VIP2I_CON                             DIP_X_VIP2I_CON;                                 /* 04DC, 0x150224DC, DIP_A_VIP2I_CON */
	DIP_X_REG_VIP2I_CON2                            DIP_X_VIP2I_CON2;                                /* 04E0, 0x150224E0, DIP_A_VIP2I_CON2 */
	DIP_X_REG_VIP2I_CON3                            DIP_X_VIP2I_CON3;                                /* 04E4, 0x150224E4, DIP_A_VIP2I_CON3 */
	UINT32                                          rsv_04E8[2];                                     /* 04E8..04EF, 0x150224E8..150224EF */
	DIP_X_REG_VIP3I_BASE_ADDR                       DIP_X_VIP3I_BASE_ADDR;                           /* 04F0, 0x150224F0, DIP_A_VIP3I_BASE_ADDR */
	UINT32                                          rsv_04F4;                                        /* 04F4, 0x150224F4 */
	DIP_X_REG_VIP3I_OFST_ADDR                       DIP_X_VIP3I_OFST_ADDR;                           /* 04F8, 0x150224F8, DIP_A_VIP3I_OFST_ADDR */
	UINT32                                          rsv_04FC;                                        /* 04FC, 0x150224FC */
	DIP_X_REG_VIP3I_XSIZE                           DIP_X_VIP3I_XSIZE;                               /* 0500, 0x15022500, DIP_A_VIP3I_XSIZE */
	DIP_X_REG_VIP3I_YSIZE                           DIP_X_VIP3I_YSIZE;                               /* 0504, 0x15022504, DIP_A_VIP3I_YSIZE */
	DIP_X_REG_VIP3I_STRIDE                          DIP_X_VIP3I_STRIDE;                              /* 0508, 0x15022508, DIP_A_VIP3I_STRIDE */
	DIP_X_REG_VIP3I_CON                             DIP_X_VIP3I_CON;                                 /* 050C, 0x1502250C, DIP_A_VIP3I_CON */
	DIP_X_REG_VIP3I_CON2                            DIP_X_VIP3I_CON2;                                /* 0510, 0x15022510, DIP_A_VIP3I_CON2 */
	DIP_X_REG_VIP3I_CON3                            DIP_X_VIP3I_CON3;                                /* 0514, 0x15022514, DIP_A_VIP3I_CON3 */
	UINT32                                          rsv_0518[2];                                     /* 0518..051F, 0x15022518..1502251F */
	DIP_X_REG_DMGI_BASE_ADDR                        DIP_X_DMGI_BASE_ADDR;                            /* 0520, 0x15022520, DIP_A_DMGI_BASE_ADDR */
	UINT32                                          rsv_0524;                                        /* 0524, 0x15022524 */
	DIP_X_REG_DMGI_OFST_ADDR                        DIP_X_DMGI_OFST_ADDR;                            /* 0528, 0x15022528, DIP_A_DMGI_OFST_ADDR */
	UINT32                                          rsv_052C;                                        /* 052C, 0x1502252C */
	DIP_X_REG_DMGI_XSIZE                            DIP_X_DMGI_XSIZE;                                /* 0530, 0x15022530, DIP_A_DMGI_XSIZE */
	DIP_X_REG_DMGI_YSIZE                            DIP_X_DMGI_YSIZE;                                /* 0534, 0x15022534, DIP_A_DMGI_YSIZE */
	DIP_X_REG_DMGI_STRIDE                           DIP_X_DMGI_STRIDE;                               /* 0538, 0x15022538, DIP_A_DMGI_STRIDE */
	DIP_X_REG_DMGI_CON                              DIP_X_DMGI_CON;                                  /* 053C, 0x1502253C, DIP_A_DMGI_CON */
	DIP_X_REG_DMGI_CON2                             DIP_X_DMGI_CON2;                                 /* 0540, 0x15022540, DIP_A_DMGI_CON2 */
	DIP_X_REG_DMGI_CON3                             DIP_X_DMGI_CON3;                                 /* 0544, 0x15022544, DIP_A_DMGI_CON3 */
	UINT32                                          rsv_0548[2];                                     /* 0548..054F, 0x15022548..1502254F */
	DIP_X_REG_DEPI_BASE_ADDR                        DIP_X_DEPI_BASE_ADDR;                            /* 0550, 0x15022550, DIP_A_DEPI_BASE_ADDR */
	UINT32                                          rsv_0554;                                        /* 0554, 0x15022554 */
	DIP_X_REG_DEPI_OFST_ADDR                        DIP_X_DEPI_OFST_ADDR;                            /* 0558, 0x15022558, DIP_A_DEPI_OFST_ADDR */
	UINT32                                          rsv_055C;                                        /* 055C, 0x1502255C */
	DIP_X_REG_DEPI_XSIZE                            DIP_X_DEPI_XSIZE;                                /* 0560, 0x15022560, DIP_A_DEPI_XSIZE */
	DIP_X_REG_DEPI_YSIZE                            DIP_X_DEPI_YSIZE;                                /* 0564, 0x15022564, DIP_A_DEPI_YSIZE */
	DIP_X_REG_DEPI_STRIDE                           DIP_X_DEPI_STRIDE;                               /* 0568, 0x15022568, DIP_A_DEPI_STRIDE */
	DIP_X_REG_DEPI_CON                              DIP_X_DEPI_CON;                                  /* 056C, 0x1502256C, DIP_A_DEPI_CON */
	DIP_X_REG_DEPI_CON2                             DIP_X_DEPI_CON2;                                 /* 0570, 0x15022570, DIP_A_DEPI_CON2 */
	DIP_X_REG_DEPI_CON3                             DIP_X_DEPI_CON3;                                 /* 0574, 0x15022574, DIP_A_DEPI_CON3 */
	UINT32                                          rsv_0578[2];                                     /* 0578..057F, 0x15022578..1502257F */
	DIP_X_REG_LCEI_BASE_ADDR                        DIP_X_LCEI_BASE_ADDR;                            /* 0580, 0x15022580, DIP_A_LCEI_BASE_ADDR */
	UINT32                                          rsv_0584;                                        /* 0584, 0x15022584 */
	DIP_X_REG_LCEI_OFST_ADDR                        DIP_X_LCEI_OFST_ADDR;                            /* 0588, 0x15022588, DIP_A_LCEI_OFST_ADDR */
	UINT32                                          rsv_058C;                                        /* 058C, 0x1502258C */
	DIP_X_REG_LCEI_XSIZE                            DIP_X_LCEI_XSIZE;                                /* 0590, 0x15022590, DIP_A_LCEI_XSIZE */
	DIP_X_REG_LCEI_YSIZE                            DIP_X_LCEI_YSIZE;                                /* 0594, 0x15022594, DIP_A_LCEI_YSIZE */
	DIP_X_REG_LCEI_STRIDE                           DIP_X_LCEI_STRIDE;                               /* 0598, 0x15022598, DIP_A_LCEI_STRIDE */
	DIP_X_REG_LCEI_CON                              DIP_X_LCEI_CON;                                  /* 059C, 0x1502259C, DIP_A_LCEI_CON */
	DIP_X_REG_LCEI_CON2                             DIP_X_LCEI_CON2;                                 /* 05A0, 0x150225A0, DIP_A_LCEI_CON2 */
	DIP_X_REG_LCEI_CON3                             DIP_X_LCEI_CON3;                                 /* 05A4, 0x150225A4, DIP_A_LCEI_CON3 */
	UINT32                                          rsv_05A8[2];                                     /* 05A8..05AF, 0x150225A8..150225AF */
	DIP_X_REG_UFDI_BASE_ADDR                        DIP_X_UFDI_BASE_ADDR;                            /* 05B0, 0x150225B0, DIP_A_UFDI_BASE_ADDR */
	UINT32                                          rsv_05B4;                                        /* 05B4, 0x150225B4 */
	DIP_X_REG_UFDI_OFST_ADDR                        DIP_X_UFDI_OFST_ADDR;                            /* 05B8, 0x150225B8, DIP_A_UFDI_OFST_ADDR */
	UINT32                                          rsv_05BC;                                        /* 05BC, 0x150225BC */
	DIP_X_REG_UFDI_XSIZE                            DIP_X_UFDI_XSIZE;                                /* 05C0, 0x150225C0, DIP_A_UFDI_XSIZE */
	DIP_X_REG_UFDI_YSIZE                            DIP_X_UFDI_YSIZE;                                /* 05C4, 0x150225C4, DIP_A_UFDI_YSIZE */
	DIP_X_REG_UFDI_STRIDE                           DIP_X_UFDI_STRIDE;                               /* 05C8, 0x150225C8, DIP_A_UFDI_STRIDE */
	DIP_X_REG_UFDI_CON                              DIP_X_UFDI_CON;                                  /* 05CC, 0x150225CC, DIP_A_UFDI_CON */
	DIP_X_REG_UFDI_CON2                             DIP_X_UFDI_CON2;                                 /* 05D0, 0x150225D0, DIP_A_UFDI_CON2 */
	DIP_X_REG_UFDI_CON3                             DIP_X_UFDI_CON3;                                 /* 05D4, 0x150225D4, DIP_A_UFDI_CON3 */
	UINT32                                          rsv_05D8[26];                                    /* 05D8..063F, 0x150225D8..1502263F */
	DIP_X_REG_DMA_ERR_CTRL                          DIP_X_DMA_ERR_CTRL;                              /* 0640, 0x15022640, DIP_A_DMA_ERR_CTRL */
	DIP_X_REG_IMG2O_ERR_STAT                        DIP_X_IMG2O_ERR_STAT;                            /* 0644, 0x15022644, DIP_A_IMG2O_ERR_STAT */
	DIP_X_REG_IMG2BO_ERR_STAT                       DIP_X_IMG2BO_ERR_STAT;                           /* 0648, 0x15022648, DIP_A_IMG2BO_ERR_STAT */
	DIP_X_REG_IMG3O_ERR_STAT                        DIP_X_IMG3O_ERR_STAT;                            /* 064C, 0x1502264C, DIP_A_IMG3O_ERR_STAT */
	DIP_X_REG_IMG3BO_ERR_STAT                       DIP_X_IMG3BO_ERR_STAT;                           /* 0650, 0x15022650, DIP_A_IMG3BO_ERR_STAT */
	DIP_X_REG_IMG3CO_ERR_STAT                       DIP_X_IMG3CO_ERR_STAT;                           /* 0654, 0x15022654, DIP_A_IMG3CO_ERR_STAT */
	DIP_X_REG_FEO_ERR_STAT                          DIP_X_FEO_ERR_STAT;                              /* 0658, 0x15022658, DIP_A_FEO_ERR_STAT */
	DIP_X_REG_MFBO_ERR_STAT                         DIP_X_MFBO_ERR_STAT;                             /* 065C, 0x1502265C, DIP_A_MFBO_ERR_STAT */
	DIP_X_REG_IMGI_ERR_STAT                         DIP_X_IMGI_ERR_STAT;                             /* 0660, 0x15022660, DIP_A_IMGI_ERR_STAT */
	DIP_X_REG_IMGBI_ERR_STAT                        DIP_X_IMGBI_ERR_STAT;                            /* 0664, 0x15022664, DIP_A_IMGBI_ERR_STAT */
	DIP_X_REG_IMGCI_ERR_STAT                        DIP_X_IMGCI_ERR_STAT;                            /* 0668, 0x15022668, DIP_A_IMGCI_ERR_STAT */
	DIP_X_REG_VIPI_ERR_STAT                         DIP_X_VIPI_ERR_STAT;                             /* 066C, 0x1502266C, DIP_A_VIPI_ERR_STAT */
	DIP_X_REG_VIP2I_ERR_STAT                        DIP_X_VIP2I_ERR_STAT;                            /* 0670, 0x15022670, DIP_A_VIP2I_ERR_STAT */
	DIP_X_REG_VIP3I_ERR_STAT                        DIP_X_VIP3I_ERR_STAT;                            /* 0674, 0x15022674, DIP_A_VIP3I_ERR_STAT */
	DIP_X_REG_DMGI_ERR_STAT                         DIP_X_DMGI_ERR_STAT;                             /* 0678, 0x15022678, DIP_A_DMGI_ERR_STAT */
	DIP_X_REG_DEPI_ERR_STAT                         DIP_X_DEPI_ERR_STAT;                             /* 067C, 0x1502267C, DIP_A_DEPI_ERR_STAT */
	DIP_X_REG_LCEI_ERR_STAT                         DIP_X_LCEI_ERR_STAT;                             /* 0680, 0x15022680, DIP_A_LCEI_ERR_STAT */
	DIP_X_REG_UFDI_ERR_STAT                         DIP_X_UFDI_ERR_STAT;                             /* 0684, 0x15022684, DIP_A_UFDI_ERR_STAT */
	UINT32                                          rsv_0688[6];                                     /* 0688..069F, 0x15022688..1502269F */
	DIP_X_REG_DMA_DEBUG_ADDR                        DIP_X_DMA_DEBUG_ADDR;                            /* 06A0, 0x150226A0, DIP_A_DMA_DEBUG_ADDR */
	DIP_X_REG_DMA_RSV1                              DIP_X_DMA_RSV1;                                  /* 06A4, 0x150226A4, DIP_A_DMA_RSV1 */
	DIP_X_REG_DMA_RSV2                              DIP_X_DMA_RSV2;                                  /* 06A8, 0x150226A8, DIP_A_DMA_RSV2 */
	DIP_X_REG_DMA_RSV3                              DIP_X_DMA_RSV3;                                  /* 06AC, 0x150226AC, DIP_A_DMA_RSV3 */
	DIP_X_REG_DMA_RSV4                              DIP_X_DMA_RSV4;                                  /* 06B0, 0x150226B0, DIP_A_DMA_RSV4 */
	DIP_X_REG_DMA_RSV5                              DIP_X_DMA_RSV5;                                  /* 06B4, 0x150226B4, DIP_A_DMA_RSV5 */
	DIP_X_REG_DMA_RSV6                              DIP_X_DMA_RSV6;                                  /* 06B8, 0x150226B8, DIP_A_DMA_RSV6 */
	DIP_X_REG_DMA_DEBUG_SEL                         DIP_X_DMA_DEBUG_SEL;                             /* 06BC, 0x150226BC, DIP_A_DMA_DEBUG_SEL */
	DIP_X_REG_DMA_BW_SELF_TEST                      DIP_X_DMA_BW_SELF_TEST;                          /* 06C0, 0x150226C0, DIP_A_DMA_BW_SELF_TEST */
	UINT32                                          rsv_06C4[15];                                    /* 06C4..06FF, 0x150226C4..150226FF */
	DIP_X_REG_UFD_CON                               DIP_X_UFD_CON;                                   /* 0700, 0x15022700, DIP_A_UFD_CON */
	DIP_X_REG_UFD_SIZE_CON                          DIP_X_UFD_SIZE_CON;                              /* 0704, 0x15022704, DIP_A_UFD_SIZE_CON */
	DIP_X_REG_UFD_AU_CON                            DIP_X_UFD_AU_CON;                                /* 0708, 0x15022708, DIP_A_UFD_AU_CON */
	DIP_X_REG_UFD_CROP_CON1                         DIP_X_UFD_CROP_CON1;                             /* 070C, 0x1502270C, DIP_A_UFD_CROP_CON1 */
	DIP_X_REG_UFD_CROP_CON2                         DIP_X_UFD_CROP_CON2;                             /* 0710, 0x15022710, DIP_A_UFD_CROP_CON2 */
	UINT32                                          rsv_0714[3];                                     /* 0714..071F, 0x15022714..1502271F */
	DIP_X_REG_PGN_SATU_1                            DIP_X_PGN_SATU_1;                                /* 0720, 0x15022720, DIP_A_PGN_SATU_1 */
	DIP_X_REG_PGN_SATU_2                            DIP_X_PGN_SATU_2;                                /* 0724, 0x15022724, DIP_A_PGN_SATU_2 */
	DIP_X_REG_PGN_GAIN_1                            DIP_X_PGN_GAIN_1;                                /* 0728, 0x15022728, DIP_A_PGN_GAIN_1 */
	DIP_X_REG_PGN_GAIN_2                            DIP_X_PGN_GAIN_2;                                /* 072C, 0x1502272C, DIP_A_PGN_GAIN_2 */
	DIP_X_REG_PGN_OFST_1                            DIP_X_PGN_OFST_1;                                /* 0730, 0x15022730, DIP_A_PGN_OFST_1 */
	DIP_X_REG_PGN_OFST_2                            DIP_X_PGN_OFST_2;                                /* 0734, 0x15022734, DIP_A_PGN_OFST_2 */
	UINT32                                          rsv_0738[2];                                     /* 0738..073F, 0x15022738..1502273F */
	DIP_X_REG_SL2_CEN                               DIP_X_SL2_CEN;                                   /* 0740, 0x15022740, DIP_A_SL2_CEN */
	DIP_X_REG_SL2_RR_CON0                           DIP_X_SL2_RR_CON0;                               /* 0744, 0x15022744, DIP_A_SL2_RR_CON0 */
	DIP_X_REG_SL2_RR_CON1                           DIP_X_SL2_RR_CON1;                               /* 0748, 0x15022748, DIP_A_SL2_RR_CON1 */
	DIP_X_REG_SL2_GAIN                              DIP_X_SL2_GAIN;                                  /* 074C, 0x1502274C, DIP_A_SL2_GAIN */
	DIP_X_REG_SL2_RZ                                DIP_X_SL2_RZ;                                    /* 0750, 0x15022750, DIP_A_SL2_RZ */
	DIP_X_REG_SL2_XOFF                              DIP_X_SL2_XOFF;                                  /* 0754, 0x15022754, DIP_A_SL2_XOFF */
	DIP_X_REG_SL2_YOFF                              DIP_X_SL2_YOFF;                                  /* 0758, 0x15022758, DIP_A_SL2_YOFF */
	DIP_X_REG_SL2_SLP_CON0                          DIP_X_SL2_SLP_CON0;                              /* 075C, 0x1502275C, DIP_A_SL2_SLP_CON0 */
	DIP_X_REG_SL2_SLP_CON1                          DIP_X_SL2_SLP_CON1;                              /* 0760, 0x15022760, DIP_A_SL2_SLP_CON1 */
	DIP_X_REG_SL2_SLP_CON2                          DIP_X_SL2_SLP_CON2;                              /* 0764, 0x15022764, DIP_A_SL2_SLP_CON2 */
	DIP_X_REG_SL2_SLP_CON3                          DIP_X_SL2_SLP_CON3;                              /* 0768, 0x15022768, DIP_A_SL2_SLP_CON3 */
	DIP_X_REG_SL2_SIZE                              DIP_X_SL2_SIZE;                                  /* 076C, 0x1502276C, DIP_A_SL2_SIZE */
	UINT32                                          rsv_0770[4];                                     /* 0770..077F, 0x15022770..1502277F */
	DIP_X_REG_UDM_INTP_CRS                          DIP_X_UDM_INTP_CRS;                              /* 0780, 0x15022780, DIP_A_UDM_INTP_CRS */
	DIP_X_REG_UDM_INTP_NAT                          DIP_X_UDM_INTP_NAT;                              /* 0784, 0x15022784, DIP_A_UDM_INTP_NAT */
	DIP_X_REG_UDM_INTP_AUG                          DIP_X_UDM_INTP_AUG;                              /* 0788, 0x15022788, DIP_A_UDM_INTP_AUG */
	DIP_X_REG_UDM_LUMA_LUT1                         DIP_X_UDM_LUMA_LUT1;                             /* 078C, 0x1502278C, DIP_A_UDM_LUMA_LUT1 */
	DIP_X_REG_UDM_LUMA_LUT2                         DIP_X_UDM_LUMA_LUT2;                             /* 0790, 0x15022790, DIP_A_UDM_LUMA_LUT2 */
	DIP_X_REG_UDM_SL_CTL                            DIP_X_UDM_SL_CTL;                                /* 0794, 0x15022794, DIP_A_UDM_SL_CTL */
	DIP_X_REG_UDM_HFTD_CTL                          DIP_X_UDM_HFTD_CTL;                              /* 0798, 0x15022798, DIP_A_UDM_HFTD_CTL */
	DIP_X_REG_UDM_NR_STR                            DIP_X_UDM_NR_STR;                                /* 079C, 0x1502279C, DIP_A_UDM_NR_STR */
	DIP_X_REG_UDM_NR_ACT                            DIP_X_UDM_NR_ACT;                                /* 07A0, 0x150227A0, DIP_A_UDM_NR_ACT */
	DIP_X_REG_UDM_HF_STR                            DIP_X_UDM_HF_STR;                                /* 07A4, 0x150227A4, DIP_A_UDM_HF_STR */
	DIP_X_REG_UDM_HF_ACT1                           DIP_X_UDM_HF_ACT1;                               /* 07A8, 0x150227A8, DIP_A_UDM_HF_ACT1 */
	DIP_X_REG_UDM_HF_ACT2                           DIP_X_UDM_HF_ACT2;                               /* 07AC, 0x150227AC, DIP_A_UDM_HF_ACT2 */
	DIP_X_REG_UDM_CLIP                              DIP_X_UDM_CLIP;                                  /* 07B0, 0x150227B0, DIP_A_UDM_CLIP */
	DIP_X_REG_UDM_DSB                               DIP_X_UDM_DSB;                                   /* 07B4, 0x150227B4, DIP_A_UDM_DSB */
	DIP_X_REG_UDM_TILE_EDGE                         DIP_X_UDM_TILE_EDGE;                             /* 07B8, 0x150227B8, DIP_A_UDM_TILE_EDGE */
	DIP_X_REG_UDM_DSL                               DIP_X_UDM_DSL;                                   /* 07BC, 0x150227BC, DIP_A_UDM_DSL */
	DIP_X_REG_UDM_LR_RAT                            DIP_X_UDM_LR_RAT;                                /* 07C0, 0x150227C0, DIP_A_UDM_LR_RAT */
	DIP_X_REG_UDM_SPARE_2                           DIP_X_UDM_SPARE_2;                               /* 07C4, 0x150227C4, DIP_A_UDM_SPARE_2 */
	DIP_X_REG_UDM_SPARE_3                           DIP_X_UDM_SPARE_3;                               /* 07C8, 0x150227C8, DIP_A_UDM_SPARE_3 */
	UINT32                                          rsv_07CC[9];                                     /* 07CC..07EF, 0x150227CC..150227EF */
	DIP_X_REG_G2G_CNV_1                             DIP_X_G2G_CNV_1;                                 /* 07F0, 0x150227F0, DIP_A_G2G_CNV_1 */
	DIP_X_REG_G2G_CNV_2                             DIP_X_G2G_CNV_2;                                 /* 07F4, 0x150227F4, DIP_A_G2G_CNV_2 */
	DIP_X_REG_G2G_CNV_3                             DIP_X_G2G_CNV_3;                                 /* 07F8, 0x150227F8, DIP_A_G2G_CNV_3 */
	DIP_X_REG_G2G_CNV_4                             DIP_X_G2G_CNV_4;                                 /* 07FC, 0x150227FC, DIP_A_G2G_CNV_4 */
	DIP_X_REG_G2G_CNV_5                             DIP_X_G2G_CNV_5;                                 /* 0800, 0x15022800, DIP_A_G2G_CNV_5 */
	DIP_X_REG_G2G_CNV_6                             DIP_X_G2G_CNV_6;                                 /* 0804, 0x15022804, DIP_A_G2G_CNV_6 */
	DIP_X_REG_G2G_CTRL                              DIP_X_G2G_CTRL;                                  /* 0808, 0x15022808, DIP_A_G2G_CTRL */
	DIP_X_REG_G2G_CFC                               DIP_X_G2G_CFC;                                   /* 080C, 0x1502280C, DIP_A_G2G_CFC */
	UINT32                                          rsv_0810[12];                                    /* 0810..083F, 0x15022810..1502283F */
	DIP_X_REG_G2C_CONV_0A                           DIP_X_G2C_CONV_0A;                               /* 0840, 0x15022840, DIP_A_G2C_CONV_0A */
	DIP_X_REG_G2C_CONV_0B                           DIP_X_G2C_CONV_0B;                               /* 0844, 0x15022844, DIP_A_G2C_CONV_0B */
	DIP_X_REG_G2C_CONV_1A                           DIP_X_G2C_CONV_1A;                               /* 0848, 0x15022848, DIP_A_G2C_CONV_1A */
	DIP_X_REG_G2C_CONV_1B                           DIP_X_G2C_CONV_1B;                               /* 084C, 0x1502284C, DIP_A_G2C_CONV_1B */
	DIP_X_REG_G2C_CONV_2A                           DIP_X_G2C_CONV_2A;                               /* 0850, 0x15022850, DIP_A_G2C_CONV_2A */
	DIP_X_REG_G2C_CONV_2B                           DIP_X_G2C_CONV_2B;                               /* 0854, 0x15022854, DIP_A_G2C_CONV_2B */
	DIP_X_REG_G2C_SHADE_CON_1                       DIP_X_G2C_SHADE_CON_1;                           /* 0858, 0x15022858, DIP_A_G2C_SHADE_CON_1 */
	DIP_X_REG_G2C_SHADE_CON_2                       DIP_X_G2C_SHADE_CON_2;                           /* 085C, 0x1502285C, DIP_A_G2C_SHADE_CON_2 */
	DIP_X_REG_G2C_SHADE_CON_3                       DIP_X_G2C_SHADE_CON_3;                           /* 0860, 0x15022860, DIP_A_G2C_SHADE_CON_3 */
	DIP_X_REG_G2C_SHADE_TAR                         DIP_X_G2C_SHADE_TAR;                             /* 0864, 0x15022864, DIP_A_G2C_SHADE_TAR */
	DIP_X_REG_G2C_SHADE_SP                          DIP_X_G2C_SHADE_SP;                              /* 0868, 0x15022868, DIP_A_G2C_SHADE_SP */
	DIP_X_REG_G2C_CFC_CON_1                         DIP_X_G2C_CFC_CON_1;                             /* 086C, 0x1502286C, DIP_A_G2C_CFC_CON_1 */
	DIP_X_REG_G2C_CFC_CON_2                         DIP_X_G2C_CFC_CON_2;                             /* 0870, 0x15022870, DIP_A_G2C_CFC_CON_2 */
	UINT32                                          rsv_0874[3];                                     /* 0874..087F, 0x15022874..1502287F */
	DIP_X_REG_C42_CON                               DIP_X_C42_CON;                                   /* 0880, 0x15022880, DIP_A_C42_CON */
	UINT32                                          rsv_0884[3];                                     /* 0884..088F, 0x15022884..1502288F */
	DIP_X_REG_SRZ3_CONTROL                          DIP_X_SRZ3_CONTROL;                              /* 0890, 0x15022890, DIP_A_SRZ3_CONTROL */
	DIP_X_REG_SRZ3_IN_IMG                           DIP_X_SRZ3_IN_IMG;                               /* 0894, 0x15022894, DIP_A_SRZ3_IN_IMG */
	DIP_X_REG_SRZ3_OUT_IMG                          DIP_X_SRZ3_OUT_IMG;                              /* 0898, 0x15022898, DIP_A_SRZ3_OUT_IMG */
	DIP_X_REG_SRZ3_HORI_STEP                        DIP_X_SRZ3_HORI_STEP;                            /* 089C, 0x1502289C, DIP_A_SRZ3_HORI_STEP */
	DIP_X_REG_SRZ3_VERT_STEP                        DIP_X_SRZ3_VERT_STEP;                            /* 08A0, 0x150228A0, DIP_A_SRZ3_VERT_STEP */
	DIP_X_REG_SRZ3_HORI_INT_OFST                    DIP_X_SRZ3_HORI_INT_OFST;                        /* 08A4, 0x150228A4, DIP_A_SRZ3_HORI_INT_OFST */
	DIP_X_REG_SRZ3_HORI_SUB_OFST                    DIP_X_SRZ3_HORI_SUB_OFST;                        /* 08A8, 0x150228A8, DIP_A_SRZ3_HORI_SUB_OFST */
	DIP_X_REG_SRZ3_VERT_INT_OFST                    DIP_X_SRZ3_VERT_INT_OFST;                        /* 08AC, 0x150228AC, DIP_A_SRZ3_VERT_INT_OFST */
	DIP_X_REG_SRZ3_VERT_SUB_OFST                    DIP_X_SRZ3_VERT_SUB_OFST;                        /* 08B0, 0x150228B0, DIP_A_SRZ3_VERT_SUB_OFST */
	UINT32                                          rsv_08B4[3];                                     /* 08B4..08BF, 0x150228B4..150228BF */
	DIP_X_REG_SRZ4_CONTROL                          DIP_X_SRZ4_CONTROL;                              /* 08C0, 0x150228C0, DIP_A_SRZ4_CONTROL */
	DIP_X_REG_SRZ4_IN_IMG                           DIP_X_SRZ4_IN_IMG;                               /* 08C4, 0x150228C4, DIP_A_SRZ4_IN_IMG */
	DIP_X_REG_SRZ4_OUT_IMG                          DIP_X_SRZ4_OUT_IMG;                              /* 08C8, 0x150228C8, DIP_A_SRZ4_OUT_IMG */
	DIP_X_REG_SRZ4_HORI_STEP                        DIP_X_SRZ4_HORI_STEP;                            /* 08CC, 0x150228CC, DIP_A_SRZ4_HORI_STEP */
	DIP_X_REG_SRZ4_VERT_STEP                        DIP_X_SRZ4_VERT_STEP;                            /* 08D0, 0x150228D0, DIP_A_SRZ4_VERT_STEP */
	DIP_X_REG_SRZ4_HORI_INT_OFST                    DIP_X_SRZ4_HORI_INT_OFST;                        /* 08D4, 0x150228D4, DIP_A_SRZ4_HORI_INT_OFST */
	DIP_X_REG_SRZ4_HORI_SUB_OFST                    DIP_X_SRZ4_HORI_SUB_OFST;                        /* 08D8, 0x150228D8, DIP_A_SRZ4_HORI_SUB_OFST */
	DIP_X_REG_SRZ4_VERT_INT_OFST                    DIP_X_SRZ4_VERT_INT_OFST;                        /* 08DC, 0x150228DC, DIP_A_SRZ4_VERT_INT_OFST */
	DIP_X_REG_SRZ4_VERT_SUB_OFST                    DIP_X_SRZ4_VERT_SUB_OFST;                        /* 08E0, 0x150228E0, DIP_A_SRZ4_VERT_SUB_OFST */
	UINT32                                          rsv_08E4[3];                                     /* 08E4..08EF, 0x150228E4..150228EF */
	DIP_X_REG_SRZ5_CONTROL                          DIP_X_SRZ5_CONTROL;                              /* 08F0, 0x150228F0, DIP_A_SRZ5_CONTROL */
	DIP_X_REG_SRZ5_IN_IMG                           DIP_X_SRZ5_IN_IMG;                               /* 08F4, 0x150228F4, DIP_A_SRZ5_IN_IMG */
	DIP_X_REG_SRZ5_OUT_IMG                          DIP_X_SRZ5_OUT_IMG;                              /* 08F8, 0x150228F8, DIP_A_SRZ5_OUT_IMG */
	DIP_X_REG_SRZ5_HORI_STEP                        DIP_X_SRZ5_HORI_STEP;                            /* 08FC, 0x150228FC, DIP_A_SRZ5_HORI_STEP */
	DIP_X_REG_SRZ5_VERT_STEP                        DIP_X_SRZ5_VERT_STEP;                            /* 0900, 0x15022900, DIP_A_SRZ5_VERT_STEP */
	DIP_X_REG_SRZ5_HORI_INT_OFST                    DIP_X_SRZ5_HORI_INT_OFST;                        /* 0904, 0x15022904, DIP_A_SRZ5_HORI_INT_OFST */
	DIP_X_REG_SRZ5_HORI_SUB_OFST                    DIP_X_SRZ5_HORI_SUB_OFST;                        /* 0908, 0x15022908, DIP_A_SRZ5_HORI_SUB_OFST */
	DIP_X_REG_SRZ5_VERT_INT_OFST                    DIP_X_SRZ5_VERT_INT_OFST;                        /* 090C, 0x1502290C, DIP_A_SRZ5_VERT_INT_OFST */
	DIP_X_REG_SRZ5_VERT_SUB_OFST                    DIP_X_SRZ5_VERT_SUB_OFST;                        /* 0910, 0x15022910, DIP_A_SRZ5_VERT_SUB_OFST */
	UINT32                                          rsv_0914[31];                                    /* 0914..098F, 0x15022914..1502298F */
	DIP_X_REG_SEEE_CTRL                             DIP_X_SEEE_CTRL;                                 /* 0990, 0x15022990, DIP_A_SEEE_CTRL */
	DIP_X_REG_SEEE_CLIP_CTRL_1                      DIP_X_SEEE_CLIP_CTRL_1;                          /* 0994, 0x15022994, DIP_A_SEEE_CLIP_CTRL_1 */
	DIP_X_REG_SEEE_CLIP_CTRL_2                      DIP_X_SEEE_CLIP_CTRL_2;                          /* 0998, 0x15022998, DIP_A_SEEE_CLIP_CTRL_2 */
	DIP_X_REG_SEEE_CLIP_CTRL_3                      DIP_X_SEEE_CLIP_CTRL_3;                          /* 099C, 0x1502299C, DIP_A_SEEE_CLIP_CTRL_3 */
	DIP_X_REG_SEEE_BLND_CTRL_1                      DIP_X_SEEE_BLND_CTRL_1;                          /* 09A0, 0x150229A0, DIP_A_SEEE_BLND_CTRL_1 */
	DIP_X_REG_SEEE_BLND_CTRL_2                      DIP_X_SEEE_BLND_CTRL_2;                          /* 09A4, 0x150229A4, DIP_A_SEEE_BLND_CTRL_2 */
	DIP_X_REG_SEEE_GN_CTRL                          DIP_X_SEEE_GN_CTRL;                              /* 09A8, 0x150229A8, DIP_A_SEEE_GN_CTRL */
	DIP_X_REG_SEEE_LUMA_CTRL_1                      DIP_X_SEEE_LUMA_CTRL_1;                          /* 09AC, 0x150229AC, DIP_A_SEEE_LUMA_CTRL_1 */
	DIP_X_REG_SEEE_LUMA_CTRL_2                      DIP_X_SEEE_LUMA_CTRL_2;                          /* 09B0, 0x150229B0, DIP_A_SEEE_LUMA_CTRL_2 */
	DIP_X_REG_SEEE_LUMA_CTRL_3                      DIP_X_SEEE_LUMA_CTRL_3;                          /* 09B4, 0x150229B4, DIP_A_SEEE_LUMA_CTRL_3 */
	DIP_X_REG_SEEE_LUMA_CTRL_4                      DIP_X_SEEE_LUMA_CTRL_4;                          /* 09B8, 0x150229B8, DIP_A_SEEE_LUMA_CTRL_4 */
	DIP_X_REG_SEEE_SLNK_CTRL_1                      DIP_X_SEEE_SLNK_CTRL_1;                          /* 09BC, 0x150229BC, DIP_A_SEEE_SLNK_CTRL_1 */
	DIP_X_REG_SEEE_SLNK_CTRL_2                      DIP_X_SEEE_SLNK_CTRL_2;                          /* 09C0, 0x150229C0, DIP_A_SEEE_SLNK_CTRL_2 */
	DIP_X_REG_SEEE_GLUT_CTRL_1                      DIP_X_SEEE_GLUT_CTRL_1;                          /* 09C4, 0x150229C4, DIP_A_SEEE_GLUT_CTRL_1 */
	DIP_X_REG_SEEE_GLUT_CTRL_2                      DIP_X_SEEE_GLUT_CTRL_2;                          /* 09C8, 0x150229C8, DIP_A_SEEE_GLUT_CTRL_2 */
	DIP_X_REG_SEEE_GLUT_CTRL_3                      DIP_X_SEEE_GLUT_CTRL_3;                          /* 09CC, 0x150229CC, DIP_A_SEEE_GLUT_CTRL_3 */
	DIP_X_REG_SEEE_GLUT_CTRL_4                      DIP_X_SEEE_GLUT_CTRL_4;                          /* 09D0, 0x150229D0, DIP_A_SEEE_GLUT_CTRL_4 */
	DIP_X_REG_SEEE_GLUT_CTRL_5                      DIP_X_SEEE_GLUT_CTRL_5;                          /* 09D4, 0x150229D4, DIP_A_SEEE_GLUT_CTRL_5 */
	DIP_X_REG_SEEE_GLUT_CTRL_6                      DIP_X_SEEE_GLUT_CTRL_6;                          /* 09D8, 0x150229D8, DIP_A_SEEE_GLUT_CTRL_6 */
	DIP_X_REG_SEEE_OUT_EDGE_CTRL                    DIP_X_SEEE_OUT_EDGE_CTRL;                        /* 09DC, 0x150229DC, DIP_A_SEEE_OUT_EDGE_CTRL */
	DIP_X_REG_SEEE_SE_Y_CTRL                        DIP_X_SEEE_SE_Y_CTRL;                            /* 09E0, 0x150229E0, DIP_A_SEEE_SE_Y_CTRL */
	DIP_X_REG_SEEE_SE_EDGE_CTRL_1                   DIP_X_SEEE_SE_EDGE_CTRL_1;                       /* 09E4, 0x150229E4, DIP_A_SEEE_SE_EDGE_CTRL_1 */
	DIP_X_REG_SEEE_SE_EDGE_CTRL_2                   DIP_X_SEEE_SE_EDGE_CTRL_2;                       /* 09E8, 0x150229E8, DIP_A_SEEE_SE_EDGE_CTRL_2 */
	DIP_X_REG_SEEE_SE_EDGE_CTRL_3                   DIP_X_SEEE_SE_EDGE_CTRL_3;                       /* 09EC, 0x150229EC, DIP_A_SEEE_SE_EDGE_CTRL_3 */
	DIP_X_REG_SEEE_SE_SPECL_CTRL                    DIP_X_SEEE_SE_SPECL_CTRL;                        /* 09F0, 0x150229F0, DIP_A_SEEE_SE_SPECL_CTRL */
	DIP_X_REG_SEEE_SE_CORE_CTRL_1                   DIP_X_SEEE_SE_CORE_CTRL_1;                       /* 09F4, 0x150229F4, DIP_A_SEEE_SE_CORE_CTRL_1 */
	DIP_X_REG_SEEE_SE_CORE_CTRL_2                   DIP_X_SEEE_SE_CORE_CTRL_2;                       /* 09F8, 0x150229F8, DIP_A_SEEE_SE_CORE_CTRL_2 */
	UINT32                                          rsv_09FC[5];                                     /* 09FC..0A0F, 0x150229FC..15022A0F */
	DIP_X_REG_NDG2_RAN_0                            DIP_X_NDG2_RAN_0;                                /* 0A10, 0x15022A10, DIP_A_NDG2_RAN_0 */
	DIP_X_REG_NDG2_RAN_1                            DIP_X_NDG2_RAN_1;                                /* 0A14, 0x15022A14, DIP_A_NDG2_RAN_1 */
	DIP_X_REG_NDG2_RAN_2                            DIP_X_NDG2_RAN_2;                                /* 0A18, 0x15022A18, DIP_A_NDG2_RAN_2 */
	DIP_X_REG_NDG2_RAN_3                            DIP_X_NDG2_RAN_3;                                /* 0A1C, 0x15022A1C, DIP_A_NDG2_RAN_3 */
	DIP_X_REG_NDG2_CROP_X                           DIP_X_NDG2_CROP_X;                               /* 0A20, 0x15022A20, DIP_A_NDG2_CROP_X */
	DIP_X_REG_NDG2_CROP_Y                           DIP_X_NDG2_CROP_Y;                               /* 0A24, 0x15022A24, DIP_A_NDG2_CROP_Y */
	UINT32                                          rsv_0A28[2];                                     /* 0A28..0A2F, 0x15022A28..15022A2F */
	DIP_X_REG_CAM_TNR_ENG_CON                       DIP_X_CAM_TNR_ENG_CON;                           /* 0A30, 0x15022A30, DIP_A_CAM_TNR_ENG_CON */
	DIP_X_REG_CAM_TNR_SIZ                           DIP_X_CAM_TNR_SIZ;                               /* 0A34, 0x15022A34, DIP_A_CAM_TNR_SIZ */
	DIP_X_REG_CAM_TNR_TILE_XY                       DIP_X_CAM_TNR_TILE_XY;                           /* 0A38, 0x15022A38, DIP_A_CAM_TNR_TILE_XY */
	DIP_X_REG_NR3D_ON_CON                           DIP_X_NR3D_ON_CON;                               /* 0A3C, 0x15022A3C, DIP_A_NR3D_ON_CON */
	DIP_X_REG_NR3D_ON_OFF                           DIP_X_NR3D_ON_OFF;                               /* 0A40, 0x15022A40, DIP_A_NR3D_ON_OFF */
	DIP_X_REG_NR3D_ON_SIZ                           DIP_X_NR3D_ON_SIZ;                               /* 0A44, 0x15022A44, DIP_A_NR3D_ON_SIZ */
	DIP_X_REG_MDP_TNR_TNR_ENABLE                    DIP_X_MDP_TNR_TNR_ENABLE;                        /* 0A48, 0x15022A48, DIP_A_MDP_TNR_TNR_ENABLE */
	DIP_X_REG_MDP_TNR_FLT_CONFIG                    DIP_X_MDP_TNR_FLT_CONFIG;                        /* 0A4C, 0x15022A4C, DIP_A_MDP_TNR_FLT_CONFIG */
	DIP_X_REG_MDP_TNR_FB_INFO1                      DIP_X_MDP_TNR_FB_INFO1;                          /* 0A50, 0x15022A50, DIP_A_MDP_TNR_FB_INFO1 */
	DIP_X_REG_MDP_TNR_THR_1                         DIP_X_MDP_TNR_THR_1;                             /* 0A54, 0x15022A54, DIP_A_MDP_TNR_THR_1 */
	DIP_X_REG_MDP_TNR_CURVE_1                       DIP_X_MDP_TNR_CURVE_1;                           /* 0A58, 0x15022A58, DIP_A_MDP_TNR_CURVE_1 */
	DIP_X_REG_MDP_TNR_CURVE_2                       DIP_X_MDP_TNR_CURVE_2;                           /* 0A5C, 0x15022A5C, DIP_A_MDP_TNR_CURVE_2 */
	DIP_X_REG_MDP_TNR_CURVE_3                       DIP_X_MDP_TNR_CURVE_3;                           /* 0A60, 0x15022A60, DIP_A_MDP_TNR_CURVE_3 */
	DIP_X_REG_MDP_TNR_CURVE_4                       DIP_X_MDP_TNR_CURVE_4;                           /* 0A64, 0x15022A64, DIP_A_MDP_TNR_CURVE_4 */
	DIP_X_REG_MDP_TNR_CURVE_5                       DIP_X_MDP_TNR_CURVE_5;                           /* 0A68, 0x15022A68, DIP_A_MDP_TNR_CURVE_5 */
	DIP_X_REG_MDP_TNR_CURVE_6                       DIP_X_MDP_TNR_CURVE_6;                           /* 0A6C, 0x15022A6C, DIP_A_MDP_TNR_CURVE_6 */
	DIP_X_REG_MDP_TNR_CURVE_7                       DIP_X_MDP_TNR_CURVE_7;                           /* 0A70, 0x15022A70, DIP_A_MDP_TNR_CURVE_7 */
	DIP_X_REG_MDP_TNR_CURVE_8                       DIP_X_MDP_TNR_CURVE_8;                           /* 0A74, 0x15022A74, DIP_A_MDP_TNR_CURVE_8 */
	DIP_X_REG_MDP_TNR_CURVE_9                       DIP_X_MDP_TNR_CURVE_9;                           /* 0A78, 0x15022A78, DIP_A_MDP_TNR_CURVE_9 */
	DIP_X_REG_MDP_TNR_CURVE_10                      DIP_X_MDP_TNR_CURVE_10;                          /* 0A7C, 0x15022A7C, DIP_A_MDP_TNR_CURVE_10 */
	DIP_X_REG_MDP_TNR_CURVE_11                      DIP_X_MDP_TNR_CURVE_11;                          /* 0A80, 0x15022A80, DIP_A_MDP_TNR_CURVE_11 */
	DIP_X_REG_MDP_TNR_CURVE_12                      DIP_X_MDP_TNR_CURVE_12;                          /* 0A84, 0x15022A84, DIP_A_MDP_TNR_CURVE_12 */
	DIP_X_REG_MDP_TNR_CURVE_13                      DIP_X_MDP_TNR_CURVE_13;                          /* 0A88, 0x15022A88, DIP_A_MDP_TNR_CURVE_13 */
	DIP_X_REG_MDP_TNR_CURVE_14                      DIP_X_MDP_TNR_CURVE_14;                          /* 0A8C, 0x15022A8C, DIP_A_MDP_TNR_CURVE_14 */
	DIP_X_REG_MDP_TNR_CURVE_15                      DIP_X_MDP_TNR_CURVE_15;                          /* 0A90, 0x15022A90, DIP_A_MDP_TNR_CURVE_15 */
	DIP_X_REG_MDP_TNR_R2C_1                         DIP_X_MDP_TNR_R2C_1;                             /* 0A94, 0x15022A94, DIP_A_MDP_TNR_R2C_1 */
	DIP_X_REG_MDP_TNR_R2C_2                         DIP_X_MDP_TNR_R2C_2;                             /* 0A98, 0x15022A98, DIP_A_MDP_TNR_R2C_2 */
	DIP_X_REG_MDP_TNR_R2C_3                         DIP_X_MDP_TNR_R2C_3;                             /* 0A9C, 0x15022A9C, DIP_A_MDP_TNR_R2C_3 */
	DIP_X_REG_MDP_TNR_DBG_6                         DIP_X_MDP_TNR_DBG_6;                             /* 0AA0, 0x15022AA0, DIP_A_MDP_TNR_DBG_6 */
	DIP_X_REG_MDP_TNR_DBG_15                        DIP_X_MDP_TNR_DBG_15;                            /* 0AA4, 0x15022AA4, DIP_A_MDP_TNR_DBG_15 */
	DIP_X_REG_MDP_TNR_DBG_16                        DIP_X_MDP_TNR_DBG_16;                            /* 0AA8, 0x15022AA8, DIP_A_MDP_TNR_DBG_16 */
	DIP_X_REG_MDP_TNR_DEMO_1                        DIP_X_MDP_TNR_DEMO_1;                            /* 0AAC, 0x15022AAC, DIP_A_MDP_TNR_DEMO_1 */
	DIP_X_REG_MDP_TNR_DEMO_2                        DIP_X_MDP_TNR_DEMO_2;                            /* 0AB0, 0x15022AB0, DIP_A_MDP_TNR_DEMO_2 */
	DIP_X_REG_MDP_TNR_ATPG                          DIP_X_MDP_TNR_ATPG;                              /* 0AB4, 0x15022AB4, DIP_A_MDP_TNR_ATPG */
	DIP_X_REG_MDP_TNR_DMY_0                         DIP_X_MDP_TNR_DMY_0;                             /* 0AB8, 0x15022AB8, DIP_A_MDP_TNR_DMY_0 */
	DIP_X_REG_MDP_TNR_DBG_17                        DIP_X_MDP_TNR_DBG_17;                            /* 0ABC, 0x15022ABC, DIP_A_MDP_TNR_DBG_17 */
	DIP_X_REG_MDP_TNR_DBG_18                        DIP_X_MDP_TNR_DBG_18;                            /* 0AC0, 0x15022AC0, DIP_A_MDP_TNR_DBG_18 */
	DIP_X_REG_MDP_TNR_DBG_1                         DIP_X_MDP_TNR_DBG_1;                             /* 0AC4, 0x15022AC4, DIP_A_MDP_TNR_DBG_1 */
	DIP_X_REG_MDP_TNR_DBG_2                         DIP_X_MDP_TNR_DBG_2;                             /* 0AC8, 0x15022AC8, DIP_A_MDP_TNR_DBG_2 */
	UINT32                                          rsv_0ACC[25];                                    /* 0ACC..0B2F, 0x15022ACC..15022B2F */
	DIP_X_REG_SL2B_CEN                              DIP_X_SL2B_CEN;                                  /* 0B30, 0x15022B30, DIP_A_SL2B_CEN */
	DIP_X_REG_SL2B_RR_CON0                          DIP_X_SL2B_RR_CON0;                              /* 0B34, 0x15022B34, DIP_A_SL2B_RR_CON0 */
	DIP_X_REG_SL2B_RR_CON1                          DIP_X_SL2B_RR_CON1;                              /* 0B38, 0x15022B38, DIP_A_SL2B_RR_CON1 */
	DIP_X_REG_SL2B_GAIN                             DIP_X_SL2B_GAIN;                                 /* 0B3C, 0x15022B3C, DIP_A_SL2B_GAIN */
	DIP_X_REG_SL2B_RZ                               DIP_X_SL2B_RZ;                                   /* 0B40, 0x15022B40, DIP_A_SL2B_RZ */
	DIP_X_REG_SL2B_XOFF                             DIP_X_SL2B_XOFF;                                 /* 0B44, 0x15022B44, DIP_A_SL2B_XOFF */
	DIP_X_REG_SL2B_YOFF                             DIP_X_SL2B_YOFF;                                 /* 0B48, 0x15022B48, DIP_A_SL2B_YOFF */
	DIP_X_REG_SL2B_SLP_CON0                         DIP_X_SL2B_SLP_CON0;                             /* 0B4C, 0x15022B4C, DIP_A_SL2B_SLP_CON0 */
	DIP_X_REG_SL2B_SLP_CON1                         DIP_X_SL2B_SLP_CON1;                             /* 0B50, 0x15022B50, DIP_A_SL2B_SLP_CON1 */
	DIP_X_REG_SL2B_SLP_CON2                         DIP_X_SL2B_SLP_CON2;                             /* 0B54, 0x15022B54, DIP_A_SL2B_SLP_CON2 */
	DIP_X_REG_SL2B_SLP_CON3                         DIP_X_SL2B_SLP_CON3;                             /* 0B58, 0x15022B58, DIP_A_SL2B_SLP_CON3 */
	DIP_X_REG_SL2B_SIZE                             DIP_X_SL2B_SIZE;                                 /* 0B5C, 0x15022B5C, DIP_A_SL2B_SIZE */
	UINT32                                          rsv_0B60[4];                                     /* 0B60..0B6F, 0x15022B60..15022B6F */
	DIP_X_REG_SL2C_CEN                              DIP_X_SL2C_CEN;                                  /* 0B70, 0x15022B70, DIP_A_SL2C_CEN */
	DIP_X_REG_SL2C_RR_CON0                          DIP_X_SL2C_RR_CON0;                              /* 0B74, 0x15022B74, DIP_A_SL2C_RR_CON0 */
	DIP_X_REG_SL2C_RR_CON1                          DIP_X_SL2C_RR_CON1;                              /* 0B78, 0x15022B78, DIP_A_SL2C_RR_CON1 */
	DIP_X_REG_SL2C_GAIN                             DIP_X_SL2C_GAIN;                                 /* 0B7C, 0x15022B7C, DIP_A_SL2C_GAIN */
	DIP_X_REG_SL2C_RZ                               DIP_X_SL2C_RZ;                                   /* 0B80, 0x15022B80, DIP_A_SL2C_RZ */
	DIP_X_REG_SL2C_XOFF                             DIP_X_SL2C_XOFF;                                 /* 0B84, 0x15022B84, DIP_A_SL2C_XOFF */
	DIP_X_REG_SL2C_YOFF                             DIP_X_SL2C_YOFF;                                 /* 0B88, 0x15022B88, DIP_A_SL2C_YOFF */
	DIP_X_REG_SL2C_SLP_CON0                         DIP_X_SL2C_SLP_CON0;                             /* 0B8C, 0x15022B8C, DIP_A_SL2C_SLP_CON0 */
	DIP_X_REG_SL2C_SLP_CON1                         DIP_X_SL2C_SLP_CON1;                             /* 0B90, 0x15022B90, DIP_A_SL2C_SLP_CON1 */
	DIP_X_REG_SL2C_SLP_CON2                         DIP_X_SL2C_SLP_CON2;                             /* 0B94, 0x15022B94, DIP_A_SL2C_SLP_CON2 */
	DIP_X_REG_SL2C_SLP_CON3                         DIP_X_SL2C_SLP_CON3;                             /* 0B98, 0x15022B98, DIP_A_SL2C_SLP_CON3 */
	DIP_X_REG_SL2C_SIZE                             DIP_X_SL2C_SIZE;                                 /* 0B9C, 0x15022B9C, DIP_A_SL2C_SIZE */
	UINT32                                          rsv_0BA0[4];                                     /* 0BA0..0BAF, 0x15022BA0..15022BAF */
	DIP_X_REG_SRZ1_CONTROL                          DIP_X_SRZ1_CONTROL;                              /* 0BB0, 0x15022BB0, DIP_A_SRZ1_CONTROL */
	DIP_X_REG_SRZ1_IN_IMG                           DIP_X_SRZ1_IN_IMG;                               /* 0BB4, 0x15022BB4, DIP_A_SRZ1_IN_IMG */
	DIP_X_REG_SRZ1_OUT_IMG                          DIP_X_SRZ1_OUT_IMG;                              /* 0BB8, 0x15022BB8, DIP_A_SRZ1_OUT_IMG */
	DIP_X_REG_SRZ1_HORI_STEP                        DIP_X_SRZ1_HORI_STEP;                            /* 0BBC, 0x15022BBC, DIP_A_SRZ1_HORI_STEP */
	DIP_X_REG_SRZ1_VERT_STEP                        DIP_X_SRZ1_VERT_STEP;                            /* 0BC0, 0x15022BC0, DIP_A_SRZ1_VERT_STEP */
	DIP_X_REG_SRZ1_HORI_INT_OFST                    DIP_X_SRZ1_HORI_INT_OFST;                        /* 0BC4, 0x15022BC4, DIP_A_SRZ1_HORI_INT_OFST */
	DIP_X_REG_SRZ1_HORI_SUB_OFST                    DIP_X_SRZ1_HORI_SUB_OFST;                        /* 0BC8, 0x15022BC8, DIP_A_SRZ1_HORI_SUB_OFST */
	DIP_X_REG_SRZ1_VERT_INT_OFST                    DIP_X_SRZ1_VERT_INT_OFST;                        /* 0BCC, 0x15022BCC, DIP_A_SRZ1_VERT_INT_OFST */
	DIP_X_REG_SRZ1_VERT_SUB_OFST                    DIP_X_SRZ1_VERT_SUB_OFST;                        /* 0BD0, 0x15022BD0, DIP_A_SRZ1_VERT_SUB_OFST */
	UINT32                                          rsv_0BD4[3];                                     /* 0BD4..0BDF, 0x15022BD4..15022BDF */
	DIP_X_REG_SRZ2_CONTROL                          DIP_X_SRZ2_CONTROL;                              /* 0BE0, 0x15022BE0, DIP_A_SRZ2_CONTROL */
	DIP_X_REG_SRZ2_IN_IMG                           DIP_X_SRZ2_IN_IMG;                               /* 0BE4, 0x15022BE4, DIP_A_SRZ2_IN_IMG */
	DIP_X_REG_SRZ2_OUT_IMG                          DIP_X_SRZ2_OUT_IMG;                              /* 0BE8, 0x15022BE8, DIP_A_SRZ2_OUT_IMG */
	DIP_X_REG_SRZ2_HORI_STEP                        DIP_X_SRZ2_HORI_STEP;                            /* 0BEC, 0x15022BEC, DIP_A_SRZ2_HORI_STEP */
	DIP_X_REG_SRZ2_VERT_STEP                        DIP_X_SRZ2_VERT_STEP;                            /* 0BF0, 0x15022BF0, DIP_A_SRZ2_VERT_STEP */
	DIP_X_REG_SRZ2_HORI_INT_OFST                    DIP_X_SRZ2_HORI_INT_OFST;                        /* 0BF4, 0x15022BF4, DIP_A_SRZ2_HORI_INT_OFST */
	DIP_X_REG_SRZ2_HORI_SUB_OFST                    DIP_X_SRZ2_HORI_SUB_OFST;                        /* 0BF8, 0x15022BF8, DIP_A_SRZ2_HORI_SUB_OFST */
	DIP_X_REG_SRZ2_VERT_INT_OFST                    DIP_X_SRZ2_VERT_INT_OFST;                        /* 0BFC, 0x15022BFC, DIP_A_SRZ2_VERT_INT_OFST */
	DIP_X_REG_SRZ2_VERT_SUB_OFST                    DIP_X_SRZ2_VERT_SUB_OFST;                        /* 0C00, 0x15022C00, DIP_A_SRZ2_VERT_SUB_OFST */
	UINT32                                          rsv_0C04[3];                                     /* 0C04..0C0F, 0x15022C04..15022C0F */
	DIP_X_REG_CRZ_CONTROL                           DIP_X_CRZ_CONTROL;                               /* 0C10, 0x15022C10, DIP_A_CRZ_CONTROL */
	DIP_X_REG_CRZ_IN_IMG                            DIP_X_CRZ_IN_IMG;                                /* 0C14, 0x15022C14, DIP_A_CRZ_IN_IMG */
	DIP_X_REG_CRZ_OUT_IMG                           DIP_X_CRZ_OUT_IMG;                               /* 0C18, 0x15022C18, DIP_A_CRZ_OUT_IMG */
	DIP_X_REG_CRZ_HORI_STEP                         DIP_X_CRZ_HORI_STEP;                             /* 0C1C, 0x15022C1C, DIP_A_CRZ_HORI_STEP */
	DIP_X_REG_CRZ_VERT_STEP                         DIP_X_CRZ_VERT_STEP;                             /* 0C20, 0x15022C20, DIP_A_CRZ_VERT_STEP */
	DIP_X_REG_CRZ_LUMA_HORI_INT_OFST                DIP_X_CRZ_LUMA_HORI_INT_OFST;                    /* 0C24, 0x15022C24, DIP_A_CRZ_LUMA_HORI_INT_OFST */
	DIP_X_REG_CRZ_LUMA_HORI_SUB_OFST                DIP_X_CRZ_LUMA_HORI_SUB_OFST;                    /* 0C28, 0x15022C28, DIP_A_CRZ_LUMA_HORI_SUB_OFST */
	DIP_X_REG_CRZ_LUMA_VERT_INT_OFST                DIP_X_CRZ_LUMA_VERT_INT_OFST;                    /* 0C2C, 0x15022C2C, DIP_A_CRZ_LUMA_VERT_INT_OFST */
	DIP_X_REG_CRZ_LUMA_VERT_SUB_OFST                DIP_X_CRZ_LUMA_VERT_SUB_OFST;                    /* 0C30, 0x15022C30, DIP_A_CRZ_LUMA_VERT_SUB_OFST */
	DIP_X_REG_CRZ_CHRO_HORI_INT_OFST                DIP_X_CRZ_CHRO_HORI_INT_OFST;                    /* 0C34, 0x15022C34, DIP_A_CRZ_CHRO_HORI_INT_OFST */
	DIP_X_REG_CRZ_CHRO_HORI_SUB_OFST                DIP_X_CRZ_CHRO_HORI_SUB_OFST;                    /* 0C38, 0x15022C38, DIP_A_CRZ_CHRO_HORI_SUB_OFST */
	DIP_X_REG_CRZ_CHRO_VERT_INT_OFST                DIP_X_CRZ_CHRO_VERT_INT_OFST;                    /* 0C3C, 0x15022C3C, DIP_A_CRZ_CHRO_VERT_INT_OFST */
	DIP_X_REG_CRZ_CHRO_VERT_SUB_OFST                DIP_X_CRZ_CHRO_VERT_SUB_OFST;                    /* 0C40, 0x15022C40, DIP_A_CRZ_CHRO_VERT_SUB_OFST */
	DIP_X_REG_CRZ_SPARE_1                           DIP_X_CRZ_SPARE_1;                               /* 0C44, 0x15022C44, DIP_A_CRZ_SPARE_1 */
	UINT32                                          rsv_0C48[10];                                    /* 0C48..0C6F, 0x15022C48..15022C6F */
	DIP_X_REG_MIX1_CTRL_0                           DIP_X_MIX1_CTRL_0;                               /* 0C70, 0x15022C70, DIP_A_MIX1_CTRL_0 */
	DIP_X_REG_MIX1_CTRL_1                           DIP_X_MIX1_CTRL_1;                               /* 0C74, 0x15022C74, DIP_A_MIX1_CTRL_1 */
	DIP_X_REG_MIX1_SPARE                            DIP_X_MIX1_SPARE;                                /* 0C78, 0x15022C78, DIP_A_MIX1_SPARE */
	UINT32                                          rsv_0C7C;                                        /* 0C7C, 0x15022C7C */
	DIP_X_REG_MIX2_CTRL_0                           DIP_X_MIX2_CTRL_0;                               /* 0C80, 0x15022C80, DIP_A_MIX2_CTRL_0 */
	DIP_X_REG_MIX2_CTRL_1                           DIP_X_MIX2_CTRL_1;                               /* 0C84, 0x15022C84, DIP_A_MIX2_CTRL_1 */
	DIP_X_REG_MIX2_SPARE                            DIP_X_MIX2_SPARE;                                /* 0C88, 0x15022C88, DIP_A_MIX2_SPARE */
	UINT32                                          rsv_0C8C;                                        /* 0C8C, 0x15022C8C */
	DIP_X_REG_MIX3_CTRL_0                           DIP_X_MIX3_CTRL_0;                               /* 0C90, 0x15022C90, DIP_A_MIX3_CTRL_0 */
	DIP_X_REG_MIX3_CTRL_1                           DIP_X_MIX3_CTRL_1;                               /* 0C94, 0x15022C94, DIP_A_MIX3_CTRL_1 */
	DIP_X_REG_MIX3_SPARE                            DIP_X_MIX3_SPARE;                                /* 0C98, 0x15022C98, DIP_A_MIX3_SPARE */
	UINT32                                          rsv_0C9C;                                        /* 0C9C, 0x15022C9C */
	DIP_X_REG_SL2D_CEN                              DIP_X_SL2D_CEN;                                  /* 0CA0, 0x15022CA0, DIP_A_SL2D_CEN */
	DIP_X_REG_SL2D_RR_CON0                          DIP_X_SL2D_RR_CON0;                              /* 0CA4, 0x15022CA4, DIP_A_SL2D_RR_CON0 */
	DIP_X_REG_SL2D_RR_CON1                          DIP_X_SL2D_RR_CON1;                              /* 0CA8, 0x15022CA8, DIP_A_SL2D_RR_CON1 */
	DIP_X_REG_SL2D_GAIN                             DIP_X_SL2D_GAIN;                                 /* 0CAC, 0x15022CAC, DIP_A_SL2D_GAIN */
	DIP_X_REG_SL2D_RZ                               DIP_X_SL2D_RZ;                                   /* 0CB0, 0x15022CB0, DIP_A_SL2D_RZ */
	DIP_X_REG_SL2D_XOFF                             DIP_X_SL2D_XOFF;                                 /* 0CB4, 0x15022CB4, DIP_A_SL2D_XOFF */
	DIP_X_REG_SL2D_YOFF                             DIP_X_SL2D_YOFF;                                 /* 0CB8, 0x15022CB8, DIP_A_SL2D_YOFF */
	DIP_X_REG_SL2D_SLP_CON0                         DIP_X_SL2D_SLP_CON0;                             /* 0CBC, 0x15022CBC, DIP_A_SL2D_SLP_CON0 */
	DIP_X_REG_SL2D_SLP_CON1                         DIP_X_SL2D_SLP_CON1;                             /* 0CC0, 0x15022CC0, DIP_A_SL2D_SLP_CON1 */
	DIP_X_REG_SL2D_SLP_CON2                         DIP_X_SL2D_SLP_CON2;                             /* 0CC4, 0x15022CC4, DIP_A_SL2D_SLP_CON2 */
	DIP_X_REG_SL2D_SLP_CON3                         DIP_X_SL2D_SLP_CON3;                             /* 0CC8, 0x15022CC8, DIP_A_SL2D_SLP_CON3 */
	DIP_X_REG_SL2D_SIZE                             DIP_X_SL2D_SIZE;                                 /* 0CCC, 0x15022CCC, DIP_A_SL2D_SIZE */
	UINT32                                          rsv_0CD0[4];                                     /* 0CD0..0CDF, 0x15022CD0..15022CDF */
	DIP_X_REG_SL2E_CEN                              DIP_X_SL2E_CEN;                                  /* 0CE0, 0x15022CE0, DIP_A_SL2E_CEN */
	DIP_X_REG_SL2E_RR_CON0                          DIP_X_SL2E_RR_CON0;                              /* 0CE4, 0x15022CE4, DIP_A_SL2E_RR_CON0 */
	DIP_X_REG_SL2E_RR_CON1                          DIP_X_SL2E_RR_CON1;                              /* 0CE8, 0x15022CE8, DIP_A_SL2E_RR_CON1 */
	DIP_X_REG_SL2E_GAIN                             DIP_X_SL2E_GAIN;                                 /* 0CEC, 0x15022CEC, DIP_A_SL2E_GAIN */
	DIP_X_REG_SL2E_RZ                               DIP_X_SL2E_RZ;                                   /* 0CF0, 0x15022CF0, DIP_A_SL2E_RZ */
	DIP_X_REG_SL2E_XOFF                             DIP_X_SL2E_XOFF;                                 /* 0CF4, 0x15022CF4, DIP_A_SL2E_XOFF */
	DIP_X_REG_SL2E_YOFF                             DIP_X_SL2E_YOFF;                                 /* 0CF8, 0x15022CF8, DIP_A_SL2E_YOFF */
	DIP_X_REG_SL2E_SLP_CON0                         DIP_X_SL2E_SLP_CON0;                             /* 0CFC, 0x15022CFC, DIP_A_SL2E_SLP_CON0 */
	DIP_X_REG_SL2E_SLP_CON1                         DIP_X_SL2E_SLP_CON1;                             /* 0D00, 0x15022D00, DIP_A_SL2E_SLP_CON1 */
	DIP_X_REG_SL2E_SLP_CON2                         DIP_X_SL2E_SLP_CON2;                             /* 0D04, 0x15022D04, DIP_A_SL2E_SLP_CON2 */
	DIP_X_REG_SL2E_SLP_CON3                         DIP_X_SL2E_SLP_CON3;                             /* 0D08, 0x15022D08, DIP_A_SL2E_SLP_CON3 */
	DIP_X_REG_SL2E_SIZE                             DIP_X_SL2E_SIZE;                                 /* 0D0C, 0x15022D0C, DIP_A_SL2E_SIZE */
	UINT32                                          rsv_0D10[4];                                     /* 0D10..0D1F, 0x15022D10..15022D1F */
	DIP_X_REG_MDP_CROP_X                            DIP_X_MDP_CROP_X;                                /* 0D20, 0x15022D20, DIP_A_MDP_CROP_X */
	DIP_X_REG_MDP_CROP_Y                            DIP_X_MDP_CROP_Y;                                /* 0D24, 0x15022D24, DIP_A_MDP_CROP_Y */
	UINT32                                          rsv_0D28[2];                                     /* 0D28..0D2F, 0x15022D28..15022D2F */
	DIP_X_REG_ANR2_CON1                             DIP_X_ANR2_CON1;                                 /* 0D30, 0x15022D30, DIP_A_ANR2_CON1 */
	DIP_X_REG_ANR2_CON2                             DIP_X_ANR2_CON2;                                 /* 0D34, 0x15022D34, DIP_A_ANR2_CON2 */
	DIP_X_REG_ANR2_YAD1                             DIP_X_ANR2_YAD1;                                 /* 0D38, 0x15022D38, DIP_A_ANR2_YAD1 */
	UINT32                                          rsv_0D3C;                                        /* 0D3C, 0x15022D3C */
	DIP_X_REG_ANR2_Y4LUT1                           DIP_X_ANR2_Y4LUT1;                               /* 0D40, 0x15022D40, DIP_A_ANR2_Y4LUT1 */
	DIP_X_REG_ANR2_Y4LUT2                           DIP_X_ANR2_Y4LUT2;                               /* 0D44, 0x15022D44, DIP_A_ANR2_Y4LUT2 */
	DIP_X_REG_ANR2_Y4LUT3                           DIP_X_ANR2_Y4LUT3;                               /* 0D48, 0x15022D48, DIP_A_ANR2_Y4LUT3 */
	UINT32                                          rsv_0D4C[5];                                     /* 0D4C..0D5F, 0x15022D4C..15022D5F */
	DIP_X_REG_ANR2_L4LUT1                           DIP_X_ANR2_L4LUT1;                               /* 0D60, 0x15022D60, DIP_A_ANR2_L4LUT1 */
	DIP_X_REG_ANR2_L4LUT2                           DIP_X_ANR2_L4LUT2;                               /* 0D64, 0x15022D64, DIP_A_ANR2_L4LUT2 */
	DIP_X_REG_ANR2_L4LUT3                           DIP_X_ANR2_L4LUT3;                               /* 0D68, 0x15022D68, DIP_A_ANR2_L4LUT3 */
	UINT32                                          rsv_0D6C;                                        /* 0D6C, 0x15022D6C */
	DIP_X_REG_ANR2_CAD                              DIP_X_ANR2_CAD;                                  /* 0D70, 0x15022D70, DIP_A_ANR2_CAD */
	DIP_X_REG_ANR2_PTC                              DIP_X_ANR2_PTC;                                  /* 0D74, 0x15022D74, DIP_A_ANR2_PTC */
	DIP_X_REG_ANR2_LCE                              DIP_X_ANR2_LCE;                                  /* 0D78, 0x15022D78, DIP_A_ANR2_LCE */
	DIP_X_REG_ANR2_MED1                             DIP_X_ANR2_MED1;                                 /* 0D7C, 0x15022D7C, DIP_A_ANR2_MED1 */
	DIP_X_REG_ANR2_MED2                             DIP_X_ANR2_MED2;                                 /* 0D80, 0x15022D80, DIP_A_ANR2_MED2 */
	DIP_X_REG_ANR2_MED3                             DIP_X_ANR2_MED3;                                 /* 0D84, 0x15022D84, DIP_A_ANR2_MED3 */
	DIP_X_REG_ANR2_MED4                             DIP_X_ANR2_MED4;                                 /* 0D88, 0x15022D88, DIP_A_ANR2_MED4 */
	DIP_X_REG_ANR2_MED5                             DIP_X_ANR2_MED5;                                 /* 0D8C, 0x15022D8C, DIP_A_ANR2_MED5 */
	DIP_X_REG_CCR_RSV1                              DIP_X_CCR_RSV1;                                  /* 0D90, 0x15022D90, DIP_A_CCR_RSV1 */
	UINT32                                          rsv_0D94[4];                                     /* 0D94..0DA3, 0x15022D94..15022DA3 */
	DIP_X_REG_ANR2_ACTC                             DIP_X_ANR2_ACTC;                                 /* 0DA4, 0x15022DA4, DIP_A_ANR2_ACTC */
	DIP_X_REG_ANR2_RSV1                             DIP_X_ANR2_RSV1;                                 /* 0DA8, 0x15022DA8, DIP_A_ANR2_RSV1 */
	DIP_X_REG_ANR2_RSV2                             DIP_X_ANR2_RSV2;                                 /* 0DAC, 0x15022DAC, DIP_A_ANR2_RSV2 */
	DIP_X_REG_CCR_CON                               DIP_X_CCR_CON;                                   /* 0DB0, 0x15022DB0, DIP_A_CCR_CON */
	DIP_X_REG_CCR_YLUT                              DIP_X_CCR_YLUT;                                  /* 0DB4, 0x15022DB4, DIP_A_CCR_YLUT */
	DIP_X_REG_CCR_UVLUT                             DIP_X_CCR_UVLUT;                                 /* 0DB8, 0x15022DB8, DIP_A_CCR_UVLUT */
	DIP_X_REG_CCR_YLUT2                             DIP_X_CCR_YLUT2;                                 /* 0DBC, 0x15022DBC, DIP_A_CCR_YLUT2 */
	DIP_X_REG_CCR_SAT_CTRL                          DIP_X_CCR_SAT_CTRL;                              /* 0DC0, 0x15022DC0, DIP_A_CCR_SAT_CTRL */
	DIP_X_REG_CCR_UVLUT_SP                          DIP_X_CCR_UVLUT_SP;                              /* 0DC4, 0x15022DC4, DIP_A_CCR_UVLUT_SP */
	DIP_X_REG_CCR_HUE1                              DIP_X_CCR_HUE1;                                  /* 0DC8, 0x15022DC8, DIP_A_CCR_HUE1 */
	DIP_X_REG_CCR_HUE2                              DIP_X_CCR_HUE2;                                  /* 0DCC, 0x15022DCC, DIP_A_CCR_HUE2 */
	DIP_X_REG_CCR_HUE3                              DIP_X_CCR_HUE3;                                  /* 0DD0, 0x15022DD0, DIP_A_CCR_HUE3 */
	DIP_X_REG_CCR_L4LUT1                            DIP_X_CCR_L4LUT1;                                /* 0DD4, 0x15022DD4, DIP_A_CCR_L4LUT1 */
	DIP_X_REG_CCR_L4LUT2                            DIP_X_CCR_L4LUT2;                                /* 0DD8, 0x15022DD8, DIP_A_CCR_L4LUT2 */
	DIP_X_REG_CCR_L4LUT3                            DIP_X_CCR_L4LUT3;                                /* 0DDC, 0x15022DDC, DIP_A_CCR_L4LUT3 */
	DIP_X_REG_BOK_CON                               DIP_X_BOK_CON;                                   /* 0DE0, 0x15022DE0, DIP_A_BOK_CON */
	DIP_X_REG_BOK_TUN                               DIP_X_BOK_TUN;                                   /* 0DE4, 0x15022DE4, DIP_A_BOK_TUN */
	DIP_X_REG_BOK_OFF                               DIP_X_BOK_OFF;                                   /* 0DE8, 0x15022DE8, DIP_A_BOK_OFF */
	DIP_X_REG_BOK_RSV1                              DIP_X_BOK_RSV1;                                  /* 0DEC, 0x15022DEC, DIP_A_BOK_RSV1 */
	DIP_X_REG_ABF_CON1                              DIP_X_ABF_CON1;                                  /* 0DF0, 0x15022DF0, DIP_A_ABF_CON1 */
	DIP_X_REG_ABF_CON2                              DIP_X_ABF_CON2;                                  /* 0DF4, 0x15022DF4, DIP_A_ABF_CON2 */
	DIP_X_REG_ABF_RCON                              DIP_X_ABF_RCON;                                  /* 0DF8, 0x15022DF8, DIP_A_ABF_RCON */
	DIP_X_REG_ABF_YLUT                              DIP_X_ABF_YLUT;                                  /* 0DFC, 0x15022DFC, DIP_A_ABF_YLUT */
	DIP_X_REG_ABF_CXLUT                             DIP_X_ABF_CXLUT;                                 /* 0E00, 0x15022E00, DIP_A_ABF_CXLUT */
	DIP_X_REG_ABF_CYLUT                             DIP_X_ABF_CYLUT;                                 /* 0E04, 0x15022E04, DIP_A_ABF_CYLUT */
	DIP_X_REG_ABF_YSP                               DIP_X_ABF_YSP;                                   /* 0E08, 0x15022E08, DIP_A_ABF_YSP */
	DIP_X_REG_ABF_CXSP                              DIP_X_ABF_CXSP;                                  /* 0E0C, 0x15022E0C, DIP_A_ABF_CXSP */
	DIP_X_REG_ABF_CYSP                              DIP_X_ABF_CYSP;                                  /* 0E10, 0x15022E10, DIP_A_ABF_CYSP */
	DIP_X_REG_ABF_CLP                               DIP_X_ABF_CLP;                                   /* 0E14, 0x15022E14, DIP_A_ABF_CLP */
	DIP_X_REG_ABF_DIFF                              DIP_X_ABF_DIFF;                                  /* 0E18, 0x15022E18, DIP_A_ABF_DIFF */
	DIP_X_REG_ABF_RSV1                              DIP_X_ABF_RSV1;                                  /* 0E1C, 0x15022E1C, DIP_A_ABF_RSV1 */
	UINT32                                          rsv_0E20[4];                                     /* 0E20..0E2F, 0x15022E20..15022E2F */
	DIP_X_REG_UNP_OFST                              DIP_X_UNP_OFST;                                  /* 0E30, 0x15022E30, DIP_A_UNP_OFST */
	UINT32                                          rsv_0E34[3];                                     /* 0E34..0E3F, 0x15022E34..15022E3F */
	DIP_X_REG_C02_CON                               DIP_X_C02_CON;                                   /* 0E40, 0x15022E40, DIP_A_C02_CON */
	DIP_X_REG_C02_CROP_CON1                         DIP_X_C02_CROP_CON1;                             /* 0E44, 0x15022E44, DIP_A_C02_CROP_CON1 */
	DIP_X_REG_C02_CROP_CON2                         DIP_X_C02_CROP_CON2;                             /* 0E48, 0x15022E48, DIP_A_C02_CROP_CON2 */
	UINT32                                          rsv_0E4C;                                        /* 0E4C, 0x15022E4C */
	DIP_X_REG_FE_CTRL1                              DIP_X_FE_CTRL1;                                  /* 0E50, 0x15022E50, DIP_A_FE_CTRL1 */
	DIP_X_REG_FE_IDX_CTRL                           DIP_X_FE_IDX_CTRL;                               /* 0E54, 0x15022E54, DIP_A_FE_IDX_CTRL */
	DIP_X_REG_FE_CROP_CTRL1                         DIP_X_FE_CROP_CTRL1;                             /* 0E58, 0x15022E58, DIP_A_FE_CROP_CTRL1 */
	DIP_X_REG_FE_CROP_CTRL2                         DIP_X_FE_CROP_CTRL2;                             /* 0E5C, 0x15022E5C, DIP_A_FE_CROP_CTRL2 */
	DIP_X_REG_FE_CTRL2                              DIP_X_FE_CTRL2;                                  /* 0E60, 0x15022E60, DIP_A_FE_CTRL2 */
	UINT32                                          rsv_0E64[3];                                     /* 0E64..0E6F, 0x15022E64..15022E6F */
	DIP_X_REG_CRSP_CTRL                             DIP_X_CRSP_CTRL;                                 /* 0E70, 0x15022E70, DIP_A_CRSP_CTRL */
	UINT32                                          rsv_0E74;                                        /* 0E74, 0x15022E74 */
	DIP_X_REG_CRSP_OUT_IMG                          DIP_X_CRSP_OUT_IMG;                              /* 0E78, 0x15022E78, DIP_A_CRSP_OUT_IMG */
	DIP_X_REG_CRSP_STEP_OFST                        DIP_X_CRSP_STEP_OFST;                            /* 0E7C, 0x15022E7C, DIP_A_CRSP_STEP_OFST */
	DIP_X_REG_CRSP_CROP_X                           DIP_X_CRSP_CROP_X;                               /* 0E80, 0x15022E80, DIP_A_CRSP_CROP_X */
	DIP_X_REG_CRSP_CROP_Y                           DIP_X_CRSP_CROP_Y;                               /* 0E84, 0x15022E84, DIP_A_CRSP_CROP_Y */
	UINT32                                          rsv_0E88[2];                                     /* 0E88..0E8F, 0x15022E88..15022E8F */
	DIP_X_REG_C02B_CON                              DIP_X_C02B_CON;                                  /* 0E90, 0x15022E90, DIP_A_C02B_CON */
	DIP_X_REG_C02B_CROP_CON1                        DIP_X_C02B_CROP_CON1;                            /* 0E94, 0x15022E94, DIP_A_C02B_CROP_CON1 */
	DIP_X_REG_C02B_CROP_CON2                        DIP_X_C02B_CROP_CON2;                            /* 0E98, 0x15022E98, DIP_A_C02B_CROP_CON2 */
	UINT32                                          rsv_0E9C;                                        /* 0E9C, 0x15022E9C */
	DIP_X_REG_C24_TILE_EDGE                         DIP_X_C24_TILE_EDGE;                             /* 0EA0, 0x15022EA0, DIP_A_C24_TILE_EDGE */
	UINT32                                          rsv_0EA4[3];                                     /* 0EA4..0EAF, 0x15022EA4..15022EAF */
	DIP_X_REG_C24B_TILE_EDGE                        DIP_X_C24B_TILE_EDGE;                            /* 0EB0, 0x15022EB0, DIP_A_C24B_TILE_EDGE */
	UINT32                                          rsv_0EB4[3];                                     /* 0EB4..0EBF, 0x15022EB4..15022EBF */
	DIP_X_REG_LCE25_CON                             DIP_X_LCE25_CON;                                 /* 0EC0, 0x15022EC0, DIP_A_LCE25_CON */
	DIP_X_REG_LCE25_ZR                              DIP_X_LCE25_ZR;                                  /* 0EC4, 0x15022EC4, DIP_A_LCE25_ZR */
	DIP_X_REG_LCE25_SLM_SIZE                        DIP_X_LCE25_SLM_SIZE;                            /* 0EC8, 0x15022EC8, DIP_A_LCE25_SLM_SIZE */
	DIP_X_REG_LCE25_OFST                            DIP_X_LCE25_OFST;                                /* 0ECC, 0x15022ECC, DIP_A_LCE25_OFST */
	DIP_X_REG_LCE25_BIAS                            DIP_X_LCE25_BIAS;                                /* 0ED0, 0x15022ED0, DIP_A_LCE25_BIAS */
	DIP_X_REG_LCE25_IMAGE_SIZE                      DIP_X_LCE25_IMAGE_SIZE;                          /* 0ED4, 0x15022ED4, DIP_A_LCE25_IMAGE_SIZE */
	DIP_X_REG_LCE25_BIL_TH0                         DIP_X_LCE25_BIL_TH0;                             /* 0ED8, 0x15022ED8, DIP_A_LCE25_BIL_TH0 */
	DIP_X_REG_LCE25_BIL_TH1                         DIP_X_LCE25_BIL_TH1;                             /* 0EDC, 0x15022EDC, DIP_A_LCE25_BIL_TH1 */
	DIP_X_REG_LCE25_TM_PARA0                        DIP_X_LCE25_TM_PARA0;                            /* 0EE0, 0x15022EE0, DIP_A_LCE25_TM_PARA0 */
	DIP_X_REG_LCE25_TM_PARA1                        DIP_X_LCE25_TM_PARA1;                            /* 0EE4, 0x15022EE4, DIP_A_LCE25_TM_PARA1 */
	DIP_X_REG_LCE25_TM_PARA2                        DIP_X_LCE25_TM_PARA2;                            /* 0EE8, 0x15022EE8, DIP_A_LCE25_TM_PARA2 */
	DIP_X_REG_LCE25_TM_PARA3                        DIP_X_LCE25_TM_PARA3;                            /* 0EEC, 0x15022EEC, DIP_A_LCE25_TM_PARA3 */
	DIP_X_REG_LCE25_TM_PARA4                        DIP_X_LCE25_TM_PARA4;                            /* 0EF0, 0x15022EF0, DIP_A_LCE25_TM_PARA4 */
	DIP_X_REG_LCE25_TM_PARA5                        DIP_X_LCE25_TM_PARA5;                            /* 0EF4, 0x15022EF4, DIP_A_LCE25_TM_PARA5 */
	DIP_X_REG_LCE25_TM_PARA6                        DIP_X_LCE25_TM_PARA6;                            /* 0EF8, 0x15022EF8, DIP_A_LCE25_TM_PARA6 */
	UINT32                                          rsv_0EFC[17];                                    /* 0EFC..0F3F, 0x15022EFC..15022F3F */
	DIP_X_REG_FM_SIZE                               DIP_X_FM_SIZE;                                   /* 0F40, 0x15022F40, DIP_A_FM_SIZE */
	DIP_X_REG_FM_TH_CON0                            DIP_X_FM_TH_CON0;                                /* 0F44, 0x15022F44, DIP_A_FM_TH_CON0 */
	DIP_X_REG_FM_ACC_RES                            DIP_X_FM_ACC_RES;                                /* 0F48, 0x15022F48, DIP_A_FM_ACC_RES */
	UINT32                                          rsv_0F4C[5];                                     /* 0F4C..0F5F, 0x15022F4C..15022F5F */
	DIP_X_REG_MFB_CON                               DIP_X_MFB_CON;                                   /* 0F60, 0x15022F60, DIP_A_MFB_CON */
	DIP_X_REG_MFB_LL_CON1                           DIP_X_MFB_LL_CON1;                               /* 0F64, 0x15022F64, DIP_A_MFB_LL_CON1 */
	DIP_X_REG_MFB_LL_CON2                           DIP_X_MFB_LL_CON2;                               /* 0F68, 0x15022F68, DIP_A_MFB_LL_CON2 */
	DIP_X_REG_MFB_LL_CON3                           DIP_X_MFB_LL_CON3;                               /* 0F6C, 0x15022F6C, DIP_A_MFB_LL_CON3 */
	DIP_X_REG_MFB_LL_CON4                           DIP_X_MFB_LL_CON4;                               /* 0F70, 0x15022F70, DIP_A_MFB_LL_CON4 */
	DIP_X_REG_MFB_EDGE                              DIP_X_MFB_EDGE;                                  /* 0F74, 0x15022F74, DIP_A_MFB_EDGE */
	DIP_X_REG_MFB_LL_CON5                           DIP_X_MFB_LL_CON5;                               /* 0F78, 0x15022F78, DIP_A_MFB_LL_CON5 */
	DIP_X_REG_MFB_LL_CON6                           DIP_X_MFB_LL_CON6;                               /* 0F7C, 0x15022F7C, DIP_A_MFB_LL_CON6 */
	DIP_X_REG_MFB_LL_CON7                           DIP_X_MFB_LL_CON7;                               /* 0F80, 0x15022F80, DIP_A_MFB_LL_CON7 */
	DIP_X_REG_MFB_LL_CON8                           DIP_X_MFB_LL_CON8;                               /* 0F84, 0x15022F84, DIP_A_MFB_LL_CON8 */
	DIP_X_REG_MFB_LL_CON9                           DIP_X_MFB_LL_CON9;                               /* 0F88, 0x15022F88, DIP_A_MFB_LL_CON9 */
	DIP_X_REG_MFB_LL_CON10                          DIP_X_MFB_LL_CON10;                              /* 0F8C, 0x15022F8C, DIP_A_MFB_LL_CON10 */
	DIP_X_REG_MFB_MBD_CON0                          DIP_X_MFB_MBD_CON0;                              /* 0F90, 0x15022F90, DIP_A_MFB_MBD_CON0 */
	DIP_X_REG_MFB_MBD_CON1                          DIP_X_MFB_MBD_CON1;                              /* 0F94, 0x15022F94, DIP_A_MFB_MBD_CON1 */
	DIP_X_REG_MFB_MBD_CON2                          DIP_X_MFB_MBD_CON2;                              /* 0F98, 0x15022F98, DIP_A_MFB_MBD_CON2 */
	DIP_X_REG_MFB_MBD_CON3                          DIP_X_MFB_MBD_CON3;                              /* 0F9C, 0x15022F9C, DIP_A_MFB_MBD_CON3 */
	DIP_X_REG_MFB_MBD_CON4                          DIP_X_MFB_MBD_CON4;                              /* 0FA0, 0x15022FA0, DIP_A_MFB_MBD_CON4 */
	DIP_X_REG_MFB_MBD_CON5                          DIP_X_MFB_MBD_CON5;                              /* 0FA4, 0x15022FA4, DIP_A_MFB_MBD_CON5 */
	DIP_X_REG_MFB_MBD_CON6                          DIP_X_MFB_MBD_CON6;                              /* 0FA8, 0x15022FA8, DIP_A_MFB_MBD_CON6 */
	DIP_X_REG_MFB_MBD_CON7                          DIP_X_MFB_MBD_CON7;                              /* 0FAC, 0x15022FAC, DIP_A_MFB_MBD_CON7 */
	DIP_X_REG_MFB_MBD_CON8                          DIP_X_MFB_MBD_CON8;                              /* 0FB0, 0x15022FB0, DIP_A_MFB_MBD_CON8 */
	DIP_X_REG_MFB_MBD_CON9                          DIP_X_MFB_MBD_CON9;                              /* 0FB4, 0x15022FB4, DIP_A_MFB_MBD_CON9 */
	DIP_X_REG_MFB_MBD_CON10                         DIP_X_MFB_MBD_CON10;                             /* 0FB8, 0x15022FB8, DIP_A_MFB_MBD_CON10 */
	UINT32                                          rsv_0FBC;                                        /* 0FBC, 0x15022FBC */
	DIP_X_REG_FLC_OFFST0                            DIP_X_FLC_OFFST0;                                /* 0FC0, 0x15022FC0, DIP_A_FLC_OFFST0 */
	DIP_X_REG_FLC_OFFST1                            DIP_X_FLC_OFFST1;                                /* 0FC4, 0x15022FC4, DIP_A_FLC_OFFST1 */
	DIP_X_REG_FLC_OFFST2                            DIP_X_FLC_OFFST2;                                /* 0FC8, 0x15022FC8, DIP_A_FLC_OFFST2 */
	UINT32                                          rsv_0FCC;                                        /* 0FCC, 0x15022FCC */
	DIP_X_REG_FLC_GAIN0                             DIP_X_FLC_GAIN0;                                 /* 0FD0, 0x15022FD0, DIP_A_FLC_GAIN0 */
	DIP_X_REG_FLC_GAIN1                             DIP_X_FLC_GAIN1;                                 /* 0FD4, 0x15022FD4, DIP_A_FLC_GAIN1 */
	DIP_X_REG_FLC_GAIN2                             DIP_X_FLC_GAIN2;                                 /* 0FD8, 0x15022FD8, DIP_A_FLC_GAIN2 */
	UINT32                                          rsv_0FDC;                                        /* 0FDC, 0x15022FDC */
	DIP_X_REG_FLC2_OFFST0                           DIP_X_FLC2_OFFST0;                               /* 0FE0, 0x15022FE0, DIP_A_FLC2_OFFST0 */
	DIP_X_REG_FLC2_OFFST1                           DIP_X_FLC2_OFFST1;                               /* 0FE4, 0x15022FE4, DIP_A_FLC2_OFFST1 */
	DIP_X_REG_FLC2_OFFST2                           DIP_X_FLC2_OFFST2;                               /* 0FE8, 0x15022FE8, DIP_A_FLC2_OFFST2 */
	UINT32                                          rsv_0FEC;                                        /* 0FEC, 0x15022FEC */
	DIP_X_REG_FLC2_GAIN0                            DIP_X_FLC2_GAIN0;                                /* 0FF0, 0x15022FF0, DIP_A_FLC2_GAIN0 */
	DIP_X_REG_FLC2_GAIN1                            DIP_X_FLC2_GAIN1;                                /* 0FF4, 0x15022FF4, DIP_A_FLC2_GAIN1 */
	DIP_X_REG_FLC2_GAIN2                            DIP_X_FLC2_GAIN2;                                /* 0FF8, 0x15022FF8, DIP_A_FLC2_GAIN2 */
	UINT32                                          rsv_0FFC;                                        /* 0FFC, 0x15022FFC */
	DIP_X_REG_GGM_LUT_RB                            DIP_X_GGM_LUT_RB[144];                           /* 1000..123F, 0x15023000..1502323F, DIP_A_GGM_LUT_RB */
	DIP_X_REG_GGM_LUT_G                             DIP_X_GGM_LUT_G[144];                            /* 1240..147F, 0x15023240..1502347F, DIP_A_GGM_LUT_G */
	DIP_X_REG_GGM_CTRL                              DIP_X_GGM_CTRL;                                  /* 1480, 0x15023480, DIP_A_GGM_CTRL */
	UINT32                                          rsv_1484[223];                                   /* 1484..17FF, 0x15023484..150237FF */
	DIP_X_REG_PCA_TBL                               DIP_X_PCA_TBL[360];                              /* 1800..1D9F, 0x15023800..15023D9F, DIP_A_PCA_TBL */
	UINT32                                          rsv_1DA0[24];                                    /* 1DA0..1DFF, 0x15023DA0..15023DFF */
	DIP_X_REG_PCA_CON1                              DIP_X_PCA_CON1;                                  /* 1E00, 0x15023E00, DIP_A_PCA_CON1 */
	DIP_X_REG_PCA_CON2                              DIP_X_PCA_CON2;                                  /* 1E04, 0x15023E04, DIP_A_PCA_CON2 */
	DIP_X_REG_PCA_CON3                              DIP_X_PCA_CON3;                                  /* 1E08, 0x15023E08, DIP_A_PCA_CON3 */
	DIP_X_REG_PCA_CON4                              DIP_X_PCA_CON4;                                  /* 1E0C, 0x15023E0C, DIP_A_PCA_CON4 */
	DIP_X_REG_PCA_CON5                              DIP_X_PCA_CON5;                                  /* 1E10, 0x15023E10, DIP_A_PCA_CON5 */
	DIP_X_REG_PCA_CON6                              DIP_X_PCA_CON6;                                  /* 1E14, 0x15023E14, DIP_A_PCA_CON6 */
	DIP_X_REG_PCA_CON7                              DIP_X_PCA_CON7;                                  /* 1E18, 0x15023E18, DIP_A_PCA_CON7 */
	UINT32                                          rsv_1E1C[9];                                     /* 1E1C..1E3F, 0x15023E1C..15023E3F */
	DIP_X_REG_OBC2_OFFST0                           DIP_X_OBC2_OFFST0;                               /* 1E40, 0x15023E40, DIP_A_OBC2_OFFST0 */
	DIP_X_REG_OBC2_OFFST1                           DIP_X_OBC2_OFFST1;                               /* 1E44, 0x15023E44, DIP_A_OBC2_OFFST1 */
	DIP_X_REG_OBC2_OFFST2                           DIP_X_OBC2_OFFST2;                               /* 1E48, 0x15023E48, DIP_A_OBC2_OFFST2 */
	DIP_X_REG_OBC2_OFFST3                           DIP_X_OBC2_OFFST3;                               /* 1E4C, 0x15023E4C, DIP_A_OBC2_OFFST3 */
	DIP_X_REG_OBC2_GAIN0                            DIP_X_OBC2_GAIN0;                                /* 1E50, 0x15023E50, DIP_A_OBC2_GAIN0 */
	DIP_X_REG_OBC2_GAIN1                            DIP_X_OBC2_GAIN1;                                /* 1E54, 0x15023E54, DIP_A_OBC2_GAIN1 */
	DIP_X_REG_OBC2_GAIN2                            DIP_X_OBC2_GAIN2;                                /* 1E58, 0x15023E58, DIP_A_OBC2_GAIN2 */
	DIP_X_REG_OBC2_GAIN3                            DIP_X_OBC2_GAIN3;                                /* 1E5C, 0x15023E5C, DIP_A_OBC2_GAIN3 */
	UINT32                                          rsv_1E60[4];                                     /* 1E60..1E6F, 0x15023E60..15023E6F */
	DIP_X_REG_BNR2_BPC_CON                          DIP_X_BNR2_BPC_CON;                              /* 1E70, 0x15023E70, DIP_A_BNR2_BPC_CON */
	DIP_X_REG_BNR2_BPC_TH1                          DIP_X_BNR2_BPC_TH1;                              /* 1E74, 0x15023E74, DIP_A_BNR2_BPC_TH1 */
	DIP_X_REG_BNR2_BPC_TH2                          DIP_X_BNR2_BPC_TH2;                              /* 1E78, 0x15023E78, DIP_A_BNR2_BPC_TH2 */
	DIP_X_REG_BNR2_BPC_TH3                          DIP_X_BNR2_BPC_TH3;                              /* 1E7C, 0x15023E7C, DIP_A_BNR2_BPC_TH3 */
	DIP_X_REG_BNR2_BPC_TH4                          DIP_X_BNR2_BPC_TH4;                              /* 1E80, 0x15023E80, DIP_A_BNR2_BPC_TH4 */
	DIP_X_REG_BNR2_BPC_DTC                          DIP_X_BNR2_BPC_DTC;                              /* 1E84, 0x15023E84, DIP_A_BNR2_BPC_DTC */
	DIP_X_REG_BNR2_BPC_COR                          DIP_X_BNR2_BPC_COR;                              /* 1E88, 0x15023E88, DIP_A_BNR2_BPC_COR */
	DIP_X_REG_BNR2_BPC_TBLI1                        DIP_X_BNR2_BPC_TBLI1;                            /* 1E8C, 0x15023E8C, DIP_A_BNR2_BPC_TBLI1 */
	DIP_X_REG_BNR2_BPC_TBLI2                        DIP_X_BNR2_BPC_TBLI2;                            /* 1E90, 0x15023E90, DIP_A_BNR2_BPC_TBLI2 */
	DIP_X_REG_BNR2_BPC_TH1_C                        DIP_X_BNR2_BPC_TH1_C;                            /* 1E94, 0x15023E94, DIP_A_BNR2_BPC_TH1_C */
	DIP_X_REG_BNR2_BPC_TH2_C                        DIP_X_BNR2_BPC_TH2_C;                            /* 1E98, 0x15023E98, DIP_A_BNR2_BPC_TH2_C */
	DIP_X_REG_BNR2_BPC_TH3_C                        DIP_X_BNR2_BPC_TH3_C;                            /* 1E9C, 0x15023E9C, DIP_A_BNR2_BPC_TH3_C */
	DIP_X_REG_BNR2_NR1_CON                          DIP_X_BNR2_NR1_CON;                              /* 1EA0, 0x15023EA0, DIP_A_BNR2_NR1_CON */
	DIP_X_REG_BNR2_NR1_CT_CON                       DIP_X_BNR2_NR1_CT_CON;                           /* 1EA4, 0x15023EA4, DIP_A_BNR2_NR1_CT_CON */
	DIP_X_REG_BNR2_RSV1                             DIP_X_BNR2_RSV1;                                 /* 1EA8, 0x15023EA8, DIP_A_BNR2_RSV1 */
	DIP_X_REG_BNR2_RSV2                             DIP_X_BNR2_RSV2;                                 /* 1EAC, 0x15023EAC, DIP_A_BNR2_RSV2 */
	DIP_X_REG_BNR2_PDC_CON                          DIP_X_BNR2_PDC_CON;                              /* 1EB0, 0x15023EB0, DIP_A_BNR2_PDC_CON */
	DIP_X_REG_BNR2_PDC_GAIN_L0                      DIP_X_BNR2_PDC_GAIN_L0;                          /* 1EB4, 0x15023EB4, DIP_A_BNR2_PDC_GAIN_L0 */
	DIP_X_REG_BNR2_PDC_GAIN_L1                      DIP_X_BNR2_PDC_GAIN_L1;                          /* 1EB8, 0x15023EB8, DIP_A_BNR2_PDC_GAIN_L1 */
	DIP_X_REG_BNR2_PDC_GAIN_L2                      DIP_X_BNR2_PDC_GAIN_L2;                          /* 1EBC, 0x15023EBC, DIP_A_BNR2_PDC_GAIN_L2 */
	DIP_X_REG_BNR2_PDC_GAIN_L3                      DIP_X_BNR2_PDC_GAIN_L3;                          /* 1EC0, 0x15023EC0, DIP_A_BNR2_PDC_GAIN_L3 */
	DIP_X_REG_BNR2_PDC_GAIN_L4                      DIP_X_BNR2_PDC_GAIN_L4;                          /* 1EC4, 0x15023EC4, DIP_A_BNR2_PDC_GAIN_L4 */
	DIP_X_REG_BNR2_PDC_GAIN_R0                      DIP_X_BNR2_PDC_GAIN_R0;                          /* 1EC8, 0x15023EC8, DIP_A_BNR2_PDC_GAIN_R0 */
	DIP_X_REG_BNR2_PDC_GAIN_R1                      DIP_X_BNR2_PDC_GAIN_R1;                          /* 1ECC, 0x15023ECC, DIP_A_BNR2_PDC_GAIN_R1 */
	DIP_X_REG_BNR2_PDC_GAIN_R2                      DIP_X_BNR2_PDC_GAIN_R2;                          /* 1ED0, 0x15023ED0, DIP_A_BNR2_PDC_GAIN_R2 */
	DIP_X_REG_BNR2_PDC_GAIN_R3                      DIP_X_BNR2_PDC_GAIN_R3;                          /* 1ED4, 0x15023ED4, DIP_A_BNR2_PDC_GAIN_R3 */
	DIP_X_REG_BNR2_PDC_GAIN_R4                      DIP_X_BNR2_PDC_GAIN_R4;                          /* 1ED8, 0x15023ED8, DIP_A_BNR2_PDC_GAIN_R4 */
	DIP_X_REG_BNR2_PDC_TH_GB                        DIP_X_BNR2_PDC_TH_GB;                            /* 1EDC, 0x15023EDC, DIP_A_BNR2_PDC_TH_GB */
	DIP_X_REG_BNR2_PDC_TH_IA                        DIP_X_BNR2_PDC_TH_IA;                            /* 1EE0, 0x15023EE0, DIP_A_BNR2_PDC_TH_IA */
	DIP_X_REG_BNR2_PDC_TH_HD                        DIP_X_BNR2_PDC_TH_HD;                            /* 1EE4, 0x15023EE4, DIP_A_BNR2_PDC_TH_HD */
	DIP_X_REG_BNR2_PDC_SL                           DIP_X_BNR2_PDC_SL;                               /* 1EE8, 0x15023EE8, DIP_A_BNR2_PDC_SL */
	DIP_X_REG_BNR2_PDC_POS                          DIP_X_BNR2_PDC_POS;                              /* 1EEC, 0x15023EEC, DIP_A_BNR2_PDC_POS */
	DIP_X_REG_RMG2_HDR_CFG                          DIP_X_RMG2_HDR_CFG;                              /* 1EF0, 0x15023EF0, DIP_A_RMG2_HDR_CFG */
	DIP_X_REG_RMG2_HDR_GAIN                         DIP_X_RMG2_HDR_GAIN;                             /* 1EF4, 0x15023EF4, DIP_A_RMG2_HDR_GAIN */
	DIP_X_REG_RMG2_HDR_CFG2                         DIP_X_RMG2_HDR_CFG2;                             /* 1EF8, 0x15023EF8, DIP_A_RMG2_HDR_CFG2 */
	UINT32                                          rsv_1EFC[5];                                     /* 1EFC..1F0F, 0x15023EFC..15023F0F */
	DIP_X_REG_RMM2_OSC                              DIP_X_RMM2_OSC;                                  /* 1F10, 0x15023F10, DIP_A_RMM2_OSC */
	DIP_X_REG_RMM2_MC                               DIP_X_RMM2_MC;                                   /* 1F14, 0x15023F14, DIP_A_RMM2_MC */
	DIP_X_REG_RMM2_REVG_1                           DIP_X_RMM2_REVG_1;                               /* 1F18, 0x15023F18, DIP_A_RMM2_REVG_1 */
	DIP_X_REG_RMM2_REVG_2                           DIP_X_RMM2_REVG_2;                               /* 1F1C, 0x15023F1C, DIP_A_RMM2_REVG_2 */
	DIP_X_REG_RMM2_LEOS                             DIP_X_RMM2_LEOS;                                 /* 1F20, 0x15023F20, DIP_A_RMM2_LEOS */
	DIP_X_REG_RMM2_MC2                              DIP_X_RMM2_MC2;                                  /* 1F24, 0x15023F24, DIP_A_RMM2_MC2 */
	DIP_X_REG_RMM2_DIFF_LB                          DIP_X_RMM2_DIFF_LB;                              /* 1F28, 0x15023F28, DIP_A_RMM2_DIFF_LB */
	DIP_X_REG_RMM2_MA                               DIP_X_RMM2_MA;                                   /* 1F2C, 0x15023F2C, DIP_A_RMM2_MA */
	DIP_X_REG_RMM2_TUNE                             DIP_X_RMM2_TUNE;                                 /* 1F30, 0x15023F30, DIP_A_RMM2_TUNE */
	UINT32                                          rsv_1F34[11];                                    /* 1F34..1F5F, 0x15023F34..15023F5F */
	DIP_X_REG_RCP2_CROP_CON1                        DIP_X_RCP2_CROP_CON1;                            /* 1F60, 0x15023F60, DIP_A_RCP2_CROP_CON1 */
	DIP_X_REG_RCP2_CROP_CON2                        DIP_X_RCP2_CROP_CON2;                            /* 1F64, 0x15023F64, DIP_A_RCP2_CROP_CON2 */
	UINT32                                          rsv_1F68[2];                                     /* 1F68..1F6F, 0x15023F68..15023F6F */
	DIP_X_REG_LSC2_CTL1                             DIP_X_LSC2_CTL1;                                 /* 1F70, 0x15023F70, DIP_A_LSC2_CTL1 */
	DIP_X_REG_LSC2_CTL2                             DIP_X_LSC2_CTL2;                                 /* 1F74, 0x15023F74, DIP_A_LSC2_CTL2 */
	DIP_X_REG_LSC2_CTL3                             DIP_X_LSC2_CTL3;                                 /* 1F78, 0x15023F78, DIP_A_LSC2_CTL3 */
	DIP_X_REG_LSC2_LBLOCK                           DIP_X_LSC2_LBLOCK;                               /* 1F7C, 0x15023F7C, DIP_A_LSC2_LBLOCK */
	DIP_X_REG_LSC2_RATIO_0                          DIP_X_LSC2_RATIO_0;                              /* 1F80, 0x15023F80, DIP_A_LSC2_RATIO_0 */
	DIP_X_REG_LSC2_TPIPE_OFST                       DIP_X_LSC2_TPIPE_OFST;                           /* 1F84, 0x15023F84, DIP_A_LSC2_TPIPE_OFST */
	DIP_X_REG_LSC2_TPIPE_SIZE                       DIP_X_LSC2_TPIPE_SIZE;                           /* 1F88, 0x15023F88, DIP_A_LSC2_TPIPE_SIZE */
	DIP_X_REG_LSC2_GAIN_TH                          DIP_X_LSC2_GAIN_TH;                              /* 1F8C, 0x15023F8C, DIP_A_LSC2_GAIN_TH */
	DIP_X_REG_LSC2_RATIO_1                          DIP_X_LSC2_RATIO_1;                              /* 1F90, 0x15023F90, DIP_A_LSC2_RATIO_1 */
	UINT32                                          rsv_1F94[3];                                     /* 1F94..1F9F, 0x15023F94..15023F9F */
	DIP_X_REG_DBS2_SIGMA                            DIP_X_DBS2_SIGMA;                                /* 1FA0, 0x15023FA0, DIP_A_DBS2_SIGMA */
	DIP_X_REG_DBS2_BSTBL_0                          DIP_X_DBS2_BSTBL_0;                              /* 1FA4, 0x15023FA4, DIP_A_DBS2_BSTBL_0 */
	DIP_X_REG_DBS2_BSTBL_1                          DIP_X_DBS2_BSTBL_1;                              /* 1FA8, 0x15023FA8, DIP_A_DBS2_BSTBL_1 */
	DIP_X_REG_DBS2_BSTBL_2                          DIP_X_DBS2_BSTBL_2;                              /* 1FAC, 0x15023FAC, DIP_A_DBS2_BSTBL_2 */
	DIP_X_REG_DBS2_BSTBL_3                          DIP_X_DBS2_BSTBL_3;                              /* 1FB0, 0x15023FB0, DIP_A_DBS2_BSTBL_3 */
	DIP_X_REG_DBS2_CTL                              DIP_X_DBS2_CTL;                                  /* 1FB4, 0x15023FB4, DIP_A_DBS2_CTL */
	DIP_X_REG_DBS2_CTL_2                            DIP_X_DBS2_CTL_2;                                /* 1FB8, 0x15023FB8, DIP_A_DBS2_CTL_2 */
	DIP_X_REG_DBS2_SIGMA_2                          DIP_X_DBS2_SIGMA_2;                              /* 1FBC, 0x15023FBC, DIP_A_DBS2_SIGMA_2 */
	DIP_X_REG_DBS2_YGN                              DIP_X_DBS2_YGN;                                  /* 1FC0, 0x15023FC0, DIP_A_DBS2_YGN */
	DIP_X_REG_DBS2_SL_Y12                           DIP_X_DBS2_SL_Y12;                               /* 1FC4, 0x15023FC4, DIP_A_DBS2_SL_Y12 */
	DIP_X_REG_DBS2_SL_Y34                           DIP_X_DBS2_SL_Y34;                               /* 1FC8, 0x15023FC8, DIP_A_DBS2_SL_Y34 */
	DIP_X_REG_DBS2_SL_G12                           DIP_X_DBS2_SL_G12;                               /* 1FCC, 0x15023FCC, DIP_A_DBS2_SL_G12 */
	DIP_X_REG_DBS2_SL_G34                           DIP_X_DBS2_SL_G34;                               /* 1FD0, 0x15023FD0, DIP_A_DBS2_SL_G34 */
	UINT32                                          rsv_1FD4[11];                                    /* 1FD4..1FFF, 0x15023FD4..15023FFF */
	DIP_X_REG_ANR_TBL                               DIP_X_ANR_TBL[256];                              /* 2000..23FF, 0x15024000..150243FF, DIP_A_ANR_TBL */
	DIP_X_REG_ANR_CON1                              DIP_X_ANR_CON1;                                  /* 2400, 0x15024400, DIP_A_ANR_CON1 */
	DIP_X_REG_ANR_CON2                              DIP_X_ANR_CON2;                                  /* 2404, 0x15024404, DIP_A_ANR_CON2 */
	DIP_X_REG_ANR_YAD1                              DIP_X_ANR_YAD1;                                  /* 2408, 0x15024408, DIP_A_ANR_YAD1 */
	DIP_X_REG_ANR_YAD2                              DIP_X_ANR_YAD2;                                  /* 240C, 0x1502440C, DIP_A_ANR_YAD2 */
	DIP_X_REG_ANR_Y4LUT1                            DIP_X_ANR_Y4LUT1;                                /* 2410, 0x15024410, DIP_A_ANR_Y4LUT1 */
	DIP_X_REG_ANR_Y4LUT2                            DIP_X_ANR_Y4LUT2;                                /* 2414, 0x15024414, DIP_A_ANR_Y4LUT2 */
	DIP_X_REG_ANR_Y4LUT3                            DIP_X_ANR_Y4LUT3;                                /* 2418, 0x15024418, DIP_A_ANR_Y4LUT3 */
	DIP_X_REG_ANR_C4LUT1                            DIP_X_ANR_C4LUT1;                                /* 241C, 0x1502441C, DIP_A_ANR_C4LUT1 */
	DIP_X_REG_ANR_C4LUT2                            DIP_X_ANR_C4LUT2;                                /* 2420, 0x15024420, DIP_A_ANR_C4LUT2 */
	DIP_X_REG_ANR_C4LUT3                            DIP_X_ANR_C4LUT3;                                /* 2424, 0x15024424, DIP_A_ANR_C4LUT3 */
	DIP_X_REG_ANR_A4LUT2                            DIP_X_ANR_A4LUT2;                                /* 2428, 0x15024428, DIP_A_ANR_A4LUT2 */
	DIP_X_REG_ANR_A4LUT3                            DIP_X_ANR_A4LUT3;                                /* 242C, 0x1502442C, DIP_A_ANR_A4LUT3 */
	DIP_X_REG_ANR_L4LUT1                            DIP_X_ANR_L4LUT1;                                /* 2430, 0x15024430, DIP_A_ANR_L4LUT1 */
	DIP_X_REG_ANR_L4LUT2                            DIP_X_ANR_L4LUT2;                                /* 2434, 0x15024434, DIP_A_ANR_L4LUT2 */
	DIP_X_REG_ANR_L4LUT3                            DIP_X_ANR_L4LUT3;                                /* 2438, 0x15024438, DIP_A_ANR_L4LUT3 */
	DIP_X_REG_ANR_PTY                               DIP_X_ANR_PTY;                                   /* 243C, 0x1502443C, DIP_A_ANR_PTY */
	DIP_X_REG_ANR_CAD                               DIP_X_ANR_CAD;                                   /* 2440, 0x15024440, DIP_A_ANR_CAD */
	DIP_X_REG_ANR_PTC                               DIP_X_ANR_PTC;                                   /* 2444, 0x15024444, DIP_A_ANR_PTC */
	DIP_X_REG_ANR_LCE                               DIP_X_ANR_LCE;                                   /* 2448, 0x15024448, DIP_A_ANR_LCE */
	UINT32                                          rsv_244C[5];                                     /* 244C..245F, 0x1502444C..1502445F */
	DIP_X_REG_ANR_T4LUT1                            DIP_X_ANR_T4LUT1;                                /* 2460, 0x15024460, DIP_A_ANR_T4LUT1 */
	DIP_X_REG_ANR_T4LUT2                            DIP_X_ANR_T4LUT2;                                /* 2464, 0x15024464, DIP_A_ANR_T4LUT2 */
	DIP_X_REG_ANR_T4LUT3                            DIP_X_ANR_T4LUT3;                                /* 2468, 0x15024468, DIP_A_ANR_T4LUT3 */
	DIP_X_REG_ANR_ACT1                              DIP_X_ANR_ACT1;                                  /* 246C, 0x1502446C, DIP_A_ANR_ACT1 */
	DIP_X_REG_ANR_ACT2                              DIP_X_ANR_ACT2;                                  /* 2470, 0x15024470, DIP_A_ANR_ACT2 */
	UINT32                                          rsv_2474;                                        /* 2474, 0x15024474 */
	DIP_X_REG_ANR_ACT4                              DIP_X_ANR_ACT4;                                  /* 2478, 0x15024478, DIP_A_ANR_ACT4 */
	DIP_X_REG_ANR_ACTYHL                            DIP_X_ANR_ACTYHL;                                /* 247C, 0x1502447C, DIP_A_ANR_ACTYHL */
	DIP_X_REG_ANR_ACTYHH                            DIP_X_ANR_ACTYHH;                                /* 2480, 0x15024480, DIP_A_ANR_ACTYHH */
	DIP_X_REG_ANR_ACTYL                             DIP_X_ANR_ACTYL;                                 /* 2484, 0x15024484, DIP_A_ANR_ACTYL */
	DIP_X_REG_ANR_ACTYHL2                           DIP_X_ANR_ACTYHL2;                               /* 2488, 0x15024488, DIP_A_ANR_ACTYHL2 */
	DIP_X_REG_ANR_ACTYHH2                           DIP_X_ANR_ACTYHH2;                               /* 248C, 0x1502448C, DIP_A_ANR_ACTYHH2 */
	DIP_X_REG_ANR_ACTYL2                            DIP_X_ANR_ACTYL2;                                /* 2490, 0x15024490, DIP_A_ANR_ACTYL2 */
	DIP_X_REG_ANR_ACTC                              DIP_X_ANR_ACTC;                                  /* 2494, 0x15024494, DIP_A_ANR_ACTC */
	DIP_X_REG_ANR_YLAD                              DIP_X_ANR_YLAD;                                  /* 2498, 0x15024498, DIP_A_ANR_YLAD */
	DIP_X_REG_ANR_YLAD2                             DIP_X_ANR_YLAD2;                                 /* 249C, 0x1502449C, DIP_A_ANR_YLAD2 */
	DIP_X_REG_ANR_YLAD3                             DIP_X_ANR_YLAD3;                                 /* 24A0, 0x150244A0, DIP_A_ANR_YLAD3 */
	DIP_X_REG_ANR_PTYL                              DIP_X_ANR_PTYL;                                  /* 24A4, 0x150244A4, DIP_A_ANR_PTYL */
	DIP_X_REG_ANR_LCOEF                             DIP_X_ANR_LCOEF;                                 /* 24A8, 0x150244A8, DIP_A_ANR_LCOEF */
	DIP_X_REG_ANR_YDIR                              DIP_X_ANR_YDIR;                                  /* 24AC, 0x150244AC, DIP_A_ANR_YDIR */
	DIP_X_REG_ANR_RSV1                              DIP_X_ANR_RSV1;                                  /* 24B0, 0x150244B0, DIP_A_ANR_RSV1 */
	UINT32                                          rsv_24B4[19];                                    /* 24B4..24FF, 0x150244B4..150244FF */
	DIP_X_REG_SL2G_CEN                              DIP_X_SL2G_CEN;                                  /* 2500, 0x15024500, DIP_A_SL2G_CEN */
	DIP_X_REG_SL2G_RR_CON0                          DIP_X_SL2G_RR_CON0;                              /* 2504, 0x15024504, DIP_A_SL2G_RR_CON0 */
	DIP_X_REG_SL2G_RR_CON1                          DIP_X_SL2G_RR_CON1;                              /* 2508, 0x15024508, DIP_A_SL2G_RR_CON1 */
	DIP_X_REG_SL2G_GAIN                             DIP_X_SL2G_GAIN;                                 /* 250C, 0x1502450C, DIP_A_SL2G_GAIN */
	DIP_X_REG_SL2G_RZ                               DIP_X_SL2G_RZ;                                   /* 2510, 0x15024510, DIP_A_SL2G_RZ */
	DIP_X_REG_SL2G_XOFF                             DIP_X_SL2G_XOFF;                                 /* 2514, 0x15024514, DIP_A_SL2G_XOFF */
	DIP_X_REG_SL2G_YOFF                             DIP_X_SL2G_YOFF;                                 /* 2518, 0x15024518, DIP_A_SL2G_YOFF */
	DIP_X_REG_SL2G_SLP_CON0                         DIP_X_SL2G_SLP_CON0;                             /* 251C, 0x1502451C, DIP_A_SL2G_SLP_CON0 */
	DIP_X_REG_SL2G_SLP_CON1                         DIP_X_SL2G_SLP_CON1;                             /* 2520, 0x15024520, DIP_A_SL2G_SLP_CON1 */
	DIP_X_REG_SL2G_SLP_CON2                         DIP_X_SL2G_SLP_CON2;                             /* 2524, 0x15024524, DIP_A_SL2G_SLP_CON2 */
	DIP_X_REG_SL2G_SLP_CON3                         DIP_X_SL2G_SLP_CON3;                             /* 2528, 0x15024528, DIP_A_SL2G_SLP_CON3 */
	DIP_X_REG_SL2G_SIZE                             DIP_X_SL2G_SIZE;                                 /* 252C, 0x1502452C, DIP_A_SL2G_SIZE */
	UINT32                                          rsv_2530[4];                                     /* 2530..253F, 0x15024530..1502453F */
	DIP_X_REG_SL2H_CEN                              DIP_X_SL2H_CEN;                                  /* 2540, 0x15024540, DIP_A_SL2H_CEN */
	DIP_X_REG_SL2H_RR_CON0                          DIP_X_SL2H_RR_CON0;                              /* 2544, 0x15024544, DIP_A_SL2H_RR_CON0 */
	DIP_X_REG_SL2H_RR_CON1                          DIP_X_SL2H_RR_CON1;                              /* 2548, 0x15024548, DIP_A_SL2H_RR_CON1 */
	DIP_X_REG_SL2H_GAIN                             DIP_X_SL2H_GAIN;                                 /* 254C, 0x1502454C, DIP_A_SL2H_GAIN */
	DIP_X_REG_SL2H_RZ                               DIP_X_SL2H_RZ;                                   /* 2550, 0x15024550, DIP_A_SL2H_RZ */
	DIP_X_REG_SL2H_XOFF                             DIP_X_SL2H_XOFF;                                 /* 2554, 0x15024554, DIP_A_SL2H_XOFF */
	DIP_X_REG_SL2H_YOFF                             DIP_X_SL2H_YOFF;                                 /* 2558, 0x15024558, DIP_A_SL2H_YOFF */
	DIP_X_REG_SL2H_SLP_CON0                         DIP_X_SL2H_SLP_CON0;                             /* 255C, 0x1502455C, DIP_A_SL2H_SLP_CON0 */
	DIP_X_REG_SL2H_SLP_CON1                         DIP_X_SL2H_SLP_CON1;                             /* 2560, 0x15024560, DIP_A_SL2H_SLP_CON1 */
	DIP_X_REG_SL2H_SLP_CON2                         DIP_X_SL2H_SLP_CON2;                             /* 2564, 0x15024564, DIP_A_SL2H_SLP_CON2 */
	DIP_X_REG_SL2H_SLP_CON3                         DIP_X_SL2H_SLP_CON3;                             /* 2568, 0x15024568, DIP_A_SL2H_SLP_CON3 */
	DIP_X_REG_SL2H_SIZE                             DIP_X_SL2H_SIZE;                                 /* 256C, 0x1502456C, DIP_A_SL2H_SIZE */
	UINT32                                          rsv_2570[4];                                     /* 2570..257F, 0x15024570..1502457F */
	DIP_X_REG_HLR2_CFG                              DIP_X_HLR2_CFG;                                  /* 2580, 0x15024580, DIP_A_HLR2_CFG */
	UINT32                                          rsv_2584;                                        /* 2584, 0x15024584 */
	DIP_X_REG_HLR2_GAIN                             DIP_X_HLR2_GAIN;                                 /* 2588, 0x15024588, DIP_A_HLR2_GAIN */
	DIP_X_REG_HLR2_GAIN_1                           DIP_X_HLR2_GAIN_1;                               /* 258C, 0x1502458C, DIP_A_HLR2_GAIN_1 */
	DIP_X_REG_HLR2_OFST                             DIP_X_HLR2_OFST;                                 /* 2590, 0x15024590, DIP_A_HLR2_OFST */
	DIP_X_REG_HLR2_OFST_1                           DIP_X_HLR2_OFST_1;                               /* 2594, 0x15024594, DIP_A_HLR2_OFST_1 */
	DIP_X_REG_HLR2_IVGN                             DIP_X_HLR2_IVGN;                                 /* 2598, 0x15024598, DIP_A_HLR2_IVGN */
	DIP_X_REG_HLR2_IVGN_1                           DIP_X_HLR2_IVGN_1;                               /* 259C, 0x1502459C, DIP_A_HLR2_IVGN_1 */
	DIP_X_REG_HLR2_KC                               DIP_X_HLR2_KC;                                   /* 25A0, 0x150245A0, DIP_A_HLR2_KC */
	DIP_X_REG_HLR2_CFG_1                            DIP_X_HLR2_CFG_1;                                /* 25A4, 0x150245A4, DIP_A_HLR2_CFG_1 */
	DIP_X_REG_HLR2_SL_PARA                          DIP_X_HLR2_SL_PARA;                              /* 25A8, 0x150245A8, DIP_A_HLR2_SL_PARA */
	DIP_X_REG_HLR2_SL_PARA_1                        DIP_X_HLR2_SL_PARA_1;                            /* 25AC, 0x150245AC, DIP_A_HLR2_SL_PARA_1 */
	DIP_X_REG_RNR_CON1                              DIP_X_RNR_CON1;                                  /* 25B0, 0x150245B0, DIP_A_RNR_CON1 */
	DIP_X_REG_RNR_VC0                               DIP_X_RNR_VC0;                                   /* 25B4, 0x150245B4, DIP_A_RNR_VC0 */
	DIP_X_REG_RNR_VC1                               DIP_X_RNR_VC1;                                   /* 25B8, 0x150245B8, DIP_A_RNR_VC1 */
	DIP_X_REG_RNR_GO_B                              DIP_X_RNR_GO_B;                                  /* 25BC, 0x150245BC, DIP_A_RNR_GO_B */
	DIP_X_REG_RNR_GO_G                              DIP_X_RNR_GO_G;                                  /* 25C0, 0x150245C0, DIP_A_RNR_GO_G */
	DIP_X_REG_RNR_GO_R                              DIP_X_RNR_GO_R;                                  /* 25C4, 0x150245C4, DIP_A_RNR_GO_R */
	DIP_X_REG_RNR_GAIN_ISO                          DIP_X_RNR_GAIN_ISO;                              /* 25C8, 0x150245C8, DIP_A_RNR_GAIN_ISO */
	DIP_X_REG_RNR_L                                 DIP_X_RNR_L;                                     /* 25CC, 0x150245CC, DIP_A_RNR_L */
	DIP_X_REG_RNR_C1                                DIP_X_RNR_C1;                                    /* 25D0, 0x150245D0, DIP_A_RNR_C1 */
	DIP_X_REG_RNR_C2                                DIP_X_RNR_C2;                                    /* 25D4, 0x150245D4, DIP_A_RNR_C2 */
	DIP_X_REG_RNR_RNG                               DIP_X_RNR_RNG;                                   /* 25D8, 0x150245D8, DIP_A_RNR_RNG */
	DIP_X_REG_RNR_CON2                              DIP_X_RNR_CON2;                                  /* 25DC, 0x150245DC, DIP_A_RNR_CON2 */
	DIP_X_REG_RNR_CON3                              DIP_X_RNR_CON3;                                  /* 25E0, 0x150245E0, DIP_A_RNR_CON3 */
	DIP_X_REG_RNR_SL                                DIP_X_RNR_SL;                                    /* 25E4, 0x150245E4, DIP_A_RNR_SL */
	DIP_X_REG_RNR_SSL_STH                           DIP_X_RNR_SSL_STH;                               /* 25E8, 0x150245E8, DIP_A_RNR_SSL_STH */
	DIP_X_REG_RNR_TILE_EDGE                         DIP_X_RNR_TILE_EDGE;                             /* 25EC, 0x150245EC, DIP_A_RNR_TILE_EDGE */
	UINT32                                          rsv_25F0[16];                                    /* 25F0..262F, 0x150245F0..1502462F */
	DIP_X_REG_HFG_CON_0                             DIP_X_HFG_CON_0;                                 /* 2630, 0x15024630, DIP_A_HFG_CON_0 */
	DIP_X_REG_HFG_LUMA_0                            DIP_X_HFG_LUMA_0;                                /* 2634, 0x15024634, DIP_A_HFG_LUMA_0 */
	DIP_X_REG_HFG_LUMA_1                            DIP_X_HFG_LUMA_1;                                /* 2638, 0x15024638, DIP_A_HFG_LUMA_1 */
	DIP_X_REG_HFG_LUMA_2                            DIP_X_HFG_LUMA_2;                                /* 263C, 0x1502463C, DIP_A_HFG_LUMA_2 */
	DIP_X_REG_HFG_LCE_0                             DIP_X_HFG_LCE_0;                                 /* 2640, 0x15024640, DIP_A_HFG_LCE_0 */
	DIP_X_REG_HFG_LCE_1                             DIP_X_HFG_LCE_1;                                 /* 2644, 0x15024644, DIP_A_HFG_LCE_1 */
	DIP_X_REG_HFG_LCE_2                             DIP_X_HFG_LCE_2;                                 /* 2648, 0x15024648, DIP_A_HFG_LCE_2 */
	DIP_X_REG_HFG_RAN_0                             DIP_X_HFG_RAN_0;                                 /* 264C, 0x1502464C, DIP_A_HFG_RAN_0 */
	DIP_X_REG_HFG_RAN_1                             DIP_X_HFG_RAN_1;                                 /* 2650, 0x15024650, DIP_A_HFG_RAN_1 */
	DIP_X_REG_HFG_RAN_2                             DIP_X_HFG_RAN_2;                                 /* 2654, 0x15024654, DIP_A_HFG_RAN_2 */
	DIP_X_REG_HFG_RAN_3                             DIP_X_HFG_RAN_3;                                 /* 2658, 0x15024658, DIP_A_HFG_RAN_3 */
	DIP_X_REG_HFG_RAN_4                             DIP_X_HFG_RAN_4;                                 /* 265C, 0x1502465C, DIP_A_HFG_RAN_4 */
	DIP_X_REG_HFG_CROP_X                            DIP_X_HFG_CROP_X;                                /* 2660, 0x15024660, DIP_A_HFG_CROP_X */
	DIP_X_REG_HFG_CROP_Y                            DIP_X_HFG_CROP_Y;                                /* 2664, 0x15024664, DIP_A_HFG_CROP_Y */
	UINT32                                          rsv_2668[2];                                     /* 2668..266F, 0x15024668..1502466F */
	DIP_X_REG_SL2I_CEN                              DIP_X_SL2I_CEN;                                  /* 2670, 0x15024670, DIP_A_SL2I_CEN */
	DIP_X_REG_SL2I_RR_CON0                          DIP_X_SL2I_RR_CON0;                              /* 2674, 0x15024674, DIP_A_SL2I_RR_CON0 */
	DIP_X_REG_SL2I_RR_CON1                          DIP_X_SL2I_RR_CON1;                              /* 2678, 0x15024678, DIP_A_SL2I_RR_CON1 */
	DIP_X_REG_SL2I_GAIN                             DIP_X_SL2I_GAIN;                                 /* 267C, 0x1502467C, DIP_A_SL2I_GAIN */
	DIP_X_REG_SL2I_RZ                               DIP_X_SL2I_RZ;                                   /* 2680, 0x15024680, DIP_A_SL2I_RZ */
	DIP_X_REG_SL2I_XOFF                             DIP_X_SL2I_XOFF;                                 /* 2684, 0x15024684, DIP_A_SL2I_XOFF */
	DIP_X_REG_SL2I_YOFF                             DIP_X_SL2I_YOFF;                                 /* 2688, 0x15024688, DIP_A_SL2I_YOFF */
	DIP_X_REG_SL2I_SLP_CON0                         DIP_X_SL2I_SLP_CON0;                             /* 268C, 0x1502468C, DIP_A_SL2I_SLP_CON0 */
	DIP_X_REG_SL2I_SLP_CON1                         DIP_X_SL2I_SLP_CON1;                             /* 2690, 0x15024690, DIP_A_SL2I_SLP_CON1 */
	DIP_X_REG_SL2I_SLP_CON2                         DIP_X_SL2I_SLP_CON2;                             /* 2694, 0x15024694, DIP_A_SL2I_SLP_CON2 */
	DIP_X_REG_SL2I_SLP_CON3                         DIP_X_SL2I_SLP_CON3;                             /* 2698, 0x15024698, DIP_A_SL2I_SLP_CON3 */
	DIP_X_REG_SL2I_SIZE                             DIP_X_SL2I_SIZE;                                 /* 269C, 0x1502469C, DIP_A_SL2I_SIZE */
	UINT32                                          rsv_26A0[4];                                     /* 26A0..26AF, 0x150246A0..150246AF */
	DIP_X_REG_SL2K_CEN                              DIP_X_SL2K_CEN;                                  /* 26B0, 0x150246B0, DIP_A_SL2K_CEN */
	DIP_X_REG_SL2K_RR_CON0                          DIP_X_SL2K_RR_CON0;                              /* 26B4, 0x150246B4, DIP_A_SL2K_RR_CON0 */
	DIP_X_REG_SL2K_RR_CON1                          DIP_X_SL2K_RR_CON1;                              /* 26B8, 0x150246B8, DIP_A_SL2K_RR_CON1 */
	DIP_X_REG_SL2K_GAIN                             DIP_X_SL2K_GAIN;                                 /* 26BC, 0x150246BC, DIP_A_SL2K_GAIN */
	DIP_X_REG_SL2K_RZ                               DIP_X_SL2K_RZ;                                   /* 26C0, 0x150246C0, DIP_A_SL2K_RZ */
	DIP_X_REG_SL2K_XOFF                             DIP_X_SL2K_XOFF;                                 /* 26C4, 0x150246C4, DIP_A_SL2K_XOFF */
	DIP_X_REG_SL2K_YOFF                             DIP_X_SL2K_YOFF;                                 /* 26C8, 0x150246C8, DIP_A_SL2K_YOFF */
	DIP_X_REG_SL2K_SLP_CON0                         DIP_X_SL2K_SLP_CON0;                             /* 26CC, 0x150246CC, DIP_A_SL2K_SLP_CON0 */
	DIP_X_REG_SL2K_SLP_CON1                         DIP_X_SL2K_SLP_CON1;                             /* 26D0, 0x150246D0, DIP_A_SL2K_SLP_CON1 */
	DIP_X_REG_SL2K_SLP_CON2                         DIP_X_SL2K_SLP_CON2;                             /* 26D4, 0x150246D4, DIP_A_SL2K_SLP_CON2 */
	DIP_X_REG_SL2K_SLP_CON3                         DIP_X_SL2K_SLP_CON3;                             /* 26D8, 0x150246D8, DIP_A_SL2K_SLP_CON3 */
	DIP_X_REG_SL2K_SIZE                             DIP_X_SL2K_SIZE;                                 /* 26DC, 0x150246DC, DIP_A_SL2K_SIZE */
	UINT32                                          rsv_26E0[4];                                     /* 26E0..26EF, 0x150246E0..150246EF */
	DIP_X_REG_CAM_COLOR_CFG_MAIN                    DIP_X_CAM_COLOR_CFG_MAIN;                        /* 26F0, 0x150246F0, DIP_A_CAM_COLOR_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_PXL_CNT_MAIN                DIP_X_CAM_COLOR_PXL_CNT_MAIN;                    /* 26F4, 0x150246F4, DIP_A_CAM_COLOR_PXL_CNT_MAIN */
	DIP_X_REG_CAM_COLOR_LINE_CNT_MAIN               DIP_X_CAM_COLOR_LINE_CNT_MAIN;                   /* 26F8, 0x150246F8, DIP_A_CAM_COLOR_LINE_CNT_MAIN */
	DIP_X_REG_CAM_COLOR_WIN_X_MAIN                  DIP_X_CAM_COLOR_WIN_X_MAIN;                      /* 26FC, 0x150246FC, DIP_A_CAM_COLOR_WIN_X_MAIN */
	DIP_X_REG_CAM_COLOR_WIN_Y_MAIN                  DIP_X_CAM_COLOR_WIN_Y_MAIN;                      /* 2700, 0x15024700, DIP_A_CAM_COLOR_WIN_Y_MAIN */
	DIP_X_REG_CAM_COLOR_TIMING_DETECTION_0          DIP_X_CAM_COLOR_TIMING_DETECTION_0;              /* 2704, 0x15024704, DIP_A_CAM_COLOR_TIMING_DETECTION_0 */
	DIP_X_REG_CAM_COLOR_TIMING_DETECTION_1          DIP_X_CAM_COLOR_TIMING_DETECTION_1;              /* 2708, 0x15024708, DIP_A_CAM_COLOR_TIMING_DETECTION_1 */
	DIP_X_REG_CAM_COLOR_DBG_CFG_MAIN                DIP_X_CAM_COLOR_DBG_CFG_MAIN;                    /* 270C, 0x1502470C, DIP_A_CAM_COLOR_DBG_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_C_BOOST_MAIN                DIP_X_CAM_COLOR_C_BOOST_MAIN;                    /* 2710, 0x15024710, DIP_A_CAM_COLOR_C_BOOST_MAIN */
	DIP_X_REG_CAM_COLOR_C_BOOST_MAIN_2              DIP_X_CAM_COLOR_C_BOOST_MAIN_2;                  /* 2714, 0x15024714, DIP_A_CAM_COLOR_C_BOOST_MAIN_2 */
	DIP_X_REG_CAM_COLOR_LUMA_ADJ                    DIP_X_CAM_COLOR_LUMA_ADJ;                        /* 2718, 0x15024718, DIP_A_CAM_COLOR_LUMA_ADJ */
	DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_1            DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1;                /* 271C, 0x1502471C, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_1 */
	DIP_X_REG_CAM_COLOR_G_PIC_ADJ_MAIN_2            DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2;                /* 2720, 0x15024720, DIP_A_CAM_COLOR_G_PIC_ADJ_MAIN_2 */
	DIP_X_REG_CAM_COLOR_POS_MAIN                    DIP_X_CAM_COLOR_POS_MAIN;                        /* 2724, 0x15024724, DIP_A_CAM_COLOR_POS_MAIN */
	DIP_X_REG_CAM_COLOR_INK_DATA_MAIN               DIP_X_CAM_COLOR_INK_DATA_MAIN;                   /* 2728, 0x15024728, DIP_A_CAM_COLOR_INK_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_INK_DATA_MAIN_CR            DIP_X_CAM_COLOR_INK_DATA_MAIN_CR;                /* 272C, 0x1502472C, DIP_A_CAM_COLOR_INK_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN            DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN;                /* 2730, 0x15024730, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_CAP_IN_DATA_MAIN_CR         DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR;             /* 2734, 0x15024734, DIP_A_CAM_COLOR_CAP_IN_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN           DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN;               /* 2738, 0x15024738, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN */
	DIP_X_REG_CAM_COLOR_CAP_OUT_DATA_MAIN_CR        DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR;            /* 273C, 0x1502473C, DIP_A_CAM_COLOR_CAP_OUT_DATA_MAIN_CR */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_1_0_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN;                /* 2740, 0x15024740, DIP_A_CAM_COLOR_Y_SLOPE_1_0_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_3_2_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN;                /* 2744, 0x15024744, DIP_A_CAM_COLOR_Y_SLOPE_3_2_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_5_4_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN;                /* 2748, 0x15024748, DIP_A_CAM_COLOR_Y_SLOPE_5_4_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_7_6_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN;                /* 274C, 0x1502474C, DIP_A_CAM_COLOR_Y_SLOPE_7_6_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_9_8_MAIN            DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN;                /* 2750, 0x15024750, DIP_A_CAM_COLOR_Y_SLOPE_9_8_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_11_10_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN;              /* 2754, 0x15024754, DIP_A_CAM_COLOR_Y_SLOPE_11_10_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_13_12_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN;              /* 2758, 0x15024758, DIP_A_CAM_COLOR_Y_SLOPE_13_12_MAIN */
	DIP_X_REG_CAM_COLOR_Y_SLOPE_15_14_MAIN          DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN;              /* 275C, 0x1502475C, DIP_A_CAM_COLOR_Y_SLOPE_15_14_MAIN */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_0              DIP_X_CAM_COLOR_LOCAL_HUE_CD_0;                  /* 2760, 0x15024760, DIP_A_CAM_COLOR_LOCAL_HUE_CD_0 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_1              DIP_X_CAM_COLOR_LOCAL_HUE_CD_1;                  /* 2764, 0x15024764, DIP_A_CAM_COLOR_LOCAL_HUE_CD_1 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_2              DIP_X_CAM_COLOR_LOCAL_HUE_CD_2;                  /* 2768, 0x15024768, DIP_A_CAM_COLOR_LOCAL_HUE_CD_2 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_3              DIP_X_CAM_COLOR_LOCAL_HUE_CD_3;                  /* 276C, 0x1502476C, DIP_A_CAM_COLOR_LOCAL_HUE_CD_3 */
	DIP_X_REG_CAM_COLOR_LOCAL_HUE_CD_4              DIP_X_CAM_COLOR_LOCAL_HUE_CD_4;                  /* 2770, 0x15024770, DIP_A_CAM_COLOR_LOCAL_HUE_CD_4 */
	DIP_X_REG_CAM_COLOR_TWO_D_WINDOW_1              DIP_X_CAM_COLOR_TWO_D_WINDOW_1;                  /* 2774, 0x15024774, DIP_A_CAM_COLOR_TWO_D_WINDOW_1 */
	DIP_X_REG_CAM_COLOR_TWO_D_W1_RESULT             DIP_X_CAM_COLOR_TWO_D_W1_RESULT;                 /* 2778, 0x15024778, DIP_A_CAM_COLOR_TWO_D_W1_RESULT */
	DIP_X_REG_CAM_COLOR_SAT_HIST_X_CFG_MAIN         DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN;             /* 277C, 0x1502477C, DIP_A_CAM_COLOR_SAT_HIST_X_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_SAT_HIST_Y_CFG_MAIN         DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN;             /* 2780, 0x15024780, DIP_A_CAM_COLOR_SAT_HIST_Y_CFG_MAIN */
	DIP_X_REG_CAM_COLOR_BWS_2                       DIP_X_CAM_COLOR_BWS_2;                           /* 2784, 0x15024784, DIP_A_CAM_COLOR_BWS_2 */
	DIP_X_REG_CAM_COLOR_CRC_0                       DIP_X_CAM_COLOR_CRC_0;                           /* 2788, 0x15024788, DIP_A_CAM_COLOR_CRC_0 */
	DIP_X_REG_CAM_COLOR_CRC_1                       DIP_X_CAM_COLOR_CRC_1;                           /* 278C, 0x1502478C, DIP_A_CAM_COLOR_CRC_1 */
	DIP_X_REG_CAM_COLOR_CRC_2                       DIP_X_CAM_COLOR_CRC_2;                           /* 2790, 0x15024790, DIP_A_CAM_COLOR_CRC_2 */
	DIP_X_REG_CAM_COLOR_CRC_3                       DIP_X_CAM_COLOR_CRC_3;                           /* 2794, 0x15024794, DIP_A_CAM_COLOR_CRC_3 */
	DIP_X_REG_CAM_COLOR_CRC_4                       DIP_X_CAM_COLOR_CRC_4;                           /* 2798, 0x15024798, DIP_A_CAM_COLOR_CRC_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0;             /* 279C, 0x1502479C, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1;             /* 27A0, 0x150247A0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2;             /* 27A4, 0x150247A4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3;             /* 27A8, 0x150247A8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN1_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4;             /* 27AC, 0x150247AC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN1_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0;             /* 27B0, 0x150247B0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1;             /* 27B4, 0x150247B4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2;             /* 27B8, 0x150247B8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3;             /* 27BC, 0x150247BC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN2_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4;             /* 27C0, 0x150247C0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN2_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_0         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0;             /* 27C4, 0x150247C4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_1         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1;             /* 27C8, 0x150247C8, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_2         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2;             /* 27CC, 0x150247CC, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_3         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3;             /* 27D0, 0x150247D0, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_GAIN3_4         DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4;             /* 27D4, 0x150247D4, DIP_A_CAM_COLOR_PARTIAL_SAT_GAIN3_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_0        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0;            /* 27D8, 0x150247D8, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_1        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1;            /* 27DC, 0x150247DC, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_2        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2;            /* 27E0, 0x150247E0, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_3        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3;            /* 27E4, 0x150247E4, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT1_4        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4;            /* 27E8, 0x150247E8, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT1_4 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_0        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0;            /* 27EC, 0x150247EC, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_0 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_1        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1;            /* 27F0, 0x150247F0, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_1 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_2        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2;            /* 27F4, 0x150247F4, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_2 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_3        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3;            /* 27F8, 0x150247F8, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_3 */
	DIP_X_REG_CAM_COLOR_PARTIAL_SAT_POINT2_4        DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4;            /* 27FC, 0x150247FC, DIP_A_CAM_COLOR_PARTIAL_SAT_POINT2_4 */
	DIP_X_REG_CAM_COLOR_START                       DIP_X_CAM_COLOR_START;                           /* 2800, 0x15024800, DIP_A_CAM_COLOR_START */
	DIP_X_REG_CAM_COLOR_INTEN                       DIP_X_CAM_COLOR_INTEN;                           /* 2804, 0x15024804, DIP_A_CAM_COLOR_INTEN */
	DIP_X_REG_CAM_COLOR_INTSTA                      DIP_X_CAM_COLOR_INTSTA;                          /* 2808, 0x15024808, DIP_A_CAM_COLOR_INTSTA */
	DIP_X_REG_CAM_COLOR_OUT_SEL                     DIP_X_CAM_COLOR_OUT_SEL;                         /* 280C, 0x1502480C, DIP_A_CAM_COLOR_OUT_SEL */
	DIP_X_REG_CAM_COLOR_FRAME_DONE_DEL              DIP_X_CAM_COLOR_FRAME_DONE_DEL;                  /* 2810, 0x15024810, DIP_A_CAM_COLOR_FRAME_DONE_DEL */
	DIP_X_REG_CAM_COLOR_CRC                         DIP_X_CAM_COLOR_CRC;                             /* 2814, 0x15024814, DIP_A_CAM_COLOR_CRC */
	DIP_X_REG_CAM_COLOR_SW_SCRATCH                  DIP_X_CAM_COLOR_SW_SCRATCH;                      /* 2818, 0x15024818, DIP_A_CAM_COLOR_SW_SCRATCH */
	DIP_X_REG_CAM_COLOR_CK_ON                       DIP_X_CAM_COLOR_CK_ON;                           /* 281C, 0x1502481C, DIP_A_CAM_COLOR_CK_ON */
	DIP_X_REG_CAM_COLOR_INTERNAL_IP_WIDTH           DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH;               /* 2820, 0x15024820, DIP_A_CAM_COLOR_INTERNAL_IP_WIDTH */
	DIP_X_REG_CAM_COLOR_INTERNAL_IP_HEIGHT          DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT;              /* 2824, 0x15024824, DIP_A_CAM_COLOR_INTERNAL_IP_HEIGHT */
	DIP_X_REG_CAM_COLOR_CM1_EN                      DIP_X_CAM_COLOR_CM1_EN;                          /* 2828, 0x15024828, DIP_A_CAM_COLOR_CM1_EN */
	DIP_X_REG_CAM_COLOR_CM2_EN                      DIP_X_CAM_COLOR_CM2_EN;                          /* 282C, 0x1502482C, DIP_A_CAM_COLOR_CM2_EN */
	DIP_X_REG_CAM_COLOR_SHADOW_CTRL                 DIP_X_CAM_COLOR_SHADOW_CTRL;                     /* 2830, 0x15024830, DIP_A_CAM_COLOR_SHADOW_CTRL */
	DIP_X_REG_CAM_COLOR_R0_CRC                      DIP_X_CAM_COLOR_R0_CRC;                          /* 2834, 0x15024834, DIP_A_CAM_COLOR_R0_CRC */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_0              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0;                  /* 2838, 0x15024838, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_1              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1;                  /* 283C, 0x1502483C, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_2              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2;                  /* 2840, 0x15024840, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_3              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3;                  /* 2844, 0x15024844, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y0_4              DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4;                  /* 2848, 0x15024848, DIP_A_CAM_COLOR_S_GAIN_BY_Y0_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_0             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0;                 /* 284C, 0x1502484C, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_1             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1;                 /* 2850, 0x15024850, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_2             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2;                 /* 2854, 0x15024854, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_3             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3;                 /* 2858, 0x15024858, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y64_4             DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4;                 /* 285C, 0x1502485C, DIP_A_CAM_COLOR_S_GAIN_BY_Y64_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0;                /* 2860, 0x15024860, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1;                /* 2864, 0x15024864, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2;                /* 2868, 0x15024868, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3;                /* 286C, 0x1502486C, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y128_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4;                /* 2870, 0x15024870, DIP_A_CAM_COLOR_S_GAIN_BY_Y128_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0;                /* 2874, 0x15024874, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1;                /* 2878, 0x15024878, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2;                /* 287C, 0x1502487C, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3;                /* 2880, 0x15024880, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y192_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4;                /* 2884, 0x15024884, DIP_A_CAM_COLOR_S_GAIN_BY_Y192_4 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_0            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0;                /* 2888, 0x15024888, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_0 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_1            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1;                /* 288C, 0x1502488C, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_1 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_2            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2;                /* 2890, 0x15024890, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_2 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_3            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3;                /* 2894, 0x15024894, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_3 */
	DIP_X_REG_CAM_COLOR_S_GAIN_BY_Y256_4            DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4;                /* 2898, 0x15024898, DIP_A_CAM_COLOR_S_GAIN_BY_Y256_4 */
	DIP_X_REG_CAM_COLOR_LSP_1                       DIP_X_CAM_COLOR_LSP_1;                           /* 289C, 0x1502489C, DIP_A_CAM_COLOR_LSP_1 */
	DIP_X_REG_CAM_COLOR_LSP_2                       DIP_X_CAM_COLOR_LSP_2;                           /* 28A0, 0x150248A0, DIP_A_CAM_COLOR_LSP_2 */
	DIP_X_REG_CAM_COLOR_CM_CONTROL                  DIP_X_CAM_COLOR_CM_CONTROL;                      /* 28A4, 0x150248A4, DIP_A_CAM_COLOR_CM_CONTROL */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_0                 DIP_X_CAM_COLOR_CM_W1_HUE_0;                     /* 28A8, 0x150248A8, DIP_A_CAM_COLOR_CM_W1_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_1                 DIP_X_CAM_COLOR_CM_W1_HUE_1;                     /* 28AC, 0x150248AC, DIP_A_CAM_COLOR_CM_W1_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_2                 DIP_X_CAM_COLOR_CM_W1_HUE_2;                     /* 28B0, 0x150248B0, DIP_A_CAM_COLOR_CM_W1_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_3                 DIP_X_CAM_COLOR_CM_W1_HUE_3;                     /* 28B4, 0x150248B4, DIP_A_CAM_COLOR_CM_W1_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_HUE_4                 DIP_X_CAM_COLOR_CM_W1_HUE_4;                     /* 28B8, 0x150248B8, DIP_A_CAM_COLOR_CM_W1_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_0                DIP_X_CAM_COLOR_CM_W1_LUMA_0;                    /* 28BC, 0x150248BC, DIP_A_CAM_COLOR_CM_W1_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_1                DIP_X_CAM_COLOR_CM_W1_LUMA_1;                    /* 28C0, 0x150248C0, DIP_A_CAM_COLOR_CM_W1_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_2                DIP_X_CAM_COLOR_CM_W1_LUMA_2;                    /* 28C4, 0x150248C4, DIP_A_CAM_COLOR_CM_W1_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_3                DIP_X_CAM_COLOR_CM_W1_LUMA_3;                    /* 28C8, 0x150248C8, DIP_A_CAM_COLOR_CM_W1_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_LUMA_4                DIP_X_CAM_COLOR_CM_W1_LUMA_4;                    /* 28CC, 0x150248CC, DIP_A_CAM_COLOR_CM_W1_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_0                 DIP_X_CAM_COLOR_CM_W1_SAT_0;                     /* 28D0, 0x150248D0, DIP_A_CAM_COLOR_CM_W1_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_1                 DIP_X_CAM_COLOR_CM_W1_SAT_1;                     /* 28D4, 0x150248D4, DIP_A_CAM_COLOR_CM_W1_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_2                 DIP_X_CAM_COLOR_CM_W1_SAT_2;                     /* 28D8, 0x150248D8, DIP_A_CAM_COLOR_CM_W1_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_3                 DIP_X_CAM_COLOR_CM_W1_SAT_3;                     /* 28DC, 0x150248DC, DIP_A_CAM_COLOR_CM_W1_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W1_SAT_4                 DIP_X_CAM_COLOR_CM_W1_SAT_4;                     /* 28E0, 0x150248E0, DIP_A_CAM_COLOR_CM_W1_SAT_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_0                 DIP_X_CAM_COLOR_CM_W2_HUE_0;                     /* 28E4, 0x150248E4, DIP_A_CAM_COLOR_CM_W2_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_1                 DIP_X_CAM_COLOR_CM_W2_HUE_1;                     /* 28E8, 0x150248E8, DIP_A_CAM_COLOR_CM_W2_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_2                 DIP_X_CAM_COLOR_CM_W2_HUE_2;                     /* 28EC, 0x150248EC, DIP_A_CAM_COLOR_CM_W2_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_3                 DIP_X_CAM_COLOR_CM_W2_HUE_3;                     /* 28F0, 0x150248F0, DIP_A_CAM_COLOR_CM_W2_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_HUE_4                 DIP_X_CAM_COLOR_CM_W2_HUE_4;                     /* 28F4, 0x150248F4, DIP_A_CAM_COLOR_CM_W2_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_0                DIP_X_CAM_COLOR_CM_W2_LUMA_0;                    /* 28F8, 0x150248F8, DIP_A_CAM_COLOR_CM_W2_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_1                DIP_X_CAM_COLOR_CM_W2_LUMA_1;                    /* 28FC, 0x150248FC, DIP_A_CAM_COLOR_CM_W2_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_2                DIP_X_CAM_COLOR_CM_W2_LUMA_2;                    /* 2900, 0x15024900, DIP_A_CAM_COLOR_CM_W2_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_3                DIP_X_CAM_COLOR_CM_W2_LUMA_3;                    /* 2904, 0x15024904, DIP_A_CAM_COLOR_CM_W2_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_LUMA_4                DIP_X_CAM_COLOR_CM_W2_LUMA_4;                    /* 2908, 0x15024908, DIP_A_CAM_COLOR_CM_W2_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_0                 DIP_X_CAM_COLOR_CM_W2_SAT_0;                     /* 290C, 0x1502490C, DIP_A_CAM_COLOR_CM_W2_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_1                 DIP_X_CAM_COLOR_CM_W2_SAT_1;                     /* 2910, 0x15024910, DIP_A_CAM_COLOR_CM_W2_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_2                 DIP_X_CAM_COLOR_CM_W2_SAT_2;                     /* 2914, 0x15024914, DIP_A_CAM_COLOR_CM_W2_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_3                 DIP_X_CAM_COLOR_CM_W2_SAT_3;                     /* 2918, 0x15024918, DIP_A_CAM_COLOR_CM_W2_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W2_SAT_4                 DIP_X_CAM_COLOR_CM_W2_SAT_4;                     /* 291C, 0x1502491C, DIP_A_CAM_COLOR_CM_W2_SAT_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_0                 DIP_X_CAM_COLOR_CM_W3_HUE_0;                     /* 2920, 0x15024920, DIP_A_CAM_COLOR_CM_W3_HUE_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_1                 DIP_X_CAM_COLOR_CM_W3_HUE_1;                     /* 2924, 0x15024924, DIP_A_CAM_COLOR_CM_W3_HUE_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_2                 DIP_X_CAM_COLOR_CM_W3_HUE_2;                     /* 2928, 0x15024928, DIP_A_CAM_COLOR_CM_W3_HUE_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_3                 DIP_X_CAM_COLOR_CM_W3_HUE_3;                     /* 292C, 0x1502492C, DIP_A_CAM_COLOR_CM_W3_HUE_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_HUE_4                 DIP_X_CAM_COLOR_CM_W3_HUE_4;                     /* 2930, 0x15024930, DIP_A_CAM_COLOR_CM_W3_HUE_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_0                DIP_X_CAM_COLOR_CM_W3_LUMA_0;                    /* 2934, 0x15024934, DIP_A_CAM_COLOR_CM_W3_LUMA_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_1                DIP_X_CAM_COLOR_CM_W3_LUMA_1;                    /* 2938, 0x15024938, DIP_A_CAM_COLOR_CM_W3_LUMA_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_2                DIP_X_CAM_COLOR_CM_W3_LUMA_2;                    /* 293C, 0x1502493C, DIP_A_CAM_COLOR_CM_W3_LUMA_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_3                DIP_X_CAM_COLOR_CM_W3_LUMA_3;                    /* 2940, 0x15024940, DIP_A_CAM_COLOR_CM_W3_LUMA_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_LUMA_4                DIP_X_CAM_COLOR_CM_W3_LUMA_4;                    /* 2944, 0x15024944, DIP_A_CAM_COLOR_CM_W3_LUMA_4 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_0                 DIP_X_CAM_COLOR_CM_W3_SAT_0;                     /* 2948, 0x15024948, DIP_A_CAM_COLOR_CM_W3_SAT_0 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_1                 DIP_X_CAM_COLOR_CM_W3_SAT_1;                     /* 294C, 0x1502494C, DIP_A_CAM_COLOR_CM_W3_SAT_1 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_2                 DIP_X_CAM_COLOR_CM_W3_SAT_2;                     /* 2950, 0x15024950, DIP_A_CAM_COLOR_CM_W3_SAT_2 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_3                 DIP_X_CAM_COLOR_CM_W3_SAT_3;                     /* 2954, 0x15024954, DIP_A_CAM_COLOR_CM_W3_SAT_3 */
	DIP_X_REG_CAM_COLOR_CM_W3_SAT_4                 DIP_X_CAM_COLOR_CM_W3_SAT_4;                     /* 2958, 0x15024958, DIP_A_CAM_COLOR_CM_W3_SAT_4 */
	UINT32                                          rsv_295C;                                        /* 295C, 0x1502495C */
	DIP_X_REG_NDG_RAN_0                             DIP_X_NDG_RAN_0;                                 /* 2960, 0x15024960, DIP_A_NDG_RAN_0 */
	DIP_X_REG_NDG_RAN_1                             DIP_X_NDG_RAN_1;                                 /* 2964, 0x15024964, DIP_A_NDG_RAN_1 */
	DIP_X_REG_NDG_RAN_2                             DIP_X_NDG_RAN_2;                                 /* 2968, 0x15024968, DIP_A_NDG_RAN_2 */
	DIP_X_REG_NDG_RAN_3                             DIP_X_NDG_RAN_3;                                 /* 296C, 0x1502496C, DIP_A_NDG_RAN_3 */
	DIP_X_REG_NDG_CROP_X                            DIP_X_NDG_CROP_X;                                /* 2970, 0x15024970, DIP_A_NDG_CROP_X */
	DIP_X_REG_NDG_CROP_Y                            DIP_X_NDG_CROP_Y;                                /* 2974, 0x15024974, DIP_A_NDG_CROP_Y */
	UINT32                                          rsv_2978[290];                                   /* 2978..2DFF, 0x15024978..15024DFF */
	DIP_X_REG_AMD_RESET                             DIP_X_AMD_RESET;                                 /* 2E00, 0x15024E00, DIP_A_AMD_RESET */
	DIP_X_REG_AMD_TOP_CTL                           DIP_X_AMD_TOP_CTL;                               /* 2E04, 0x15024E04, DIP_A_AMD_TOP_CTL */
	DIP_X_REG_AMD_INT_EN                            DIP_X_AMD_INT_EN;                                /* 2E08, 0x15024E08, DIP_A_AMD_INT_EN */
	DIP_X_REG_AMD_INT_ST                            DIP_X_AMD_INT_ST;                                /* 2E0C, 0x15024E0C, DIP_A_AMD_INT_ST */
	DIP_X_REG_AMD_C24_CTL                           DIP_X_AMD_C24_CTL;                               /* 2E10, 0x15024E10, DIP_A_AMD_C24_CTL */
	DIP_X_REG_AMD_CROP_CTL_0                        DIP_X_AMD_CROP_CTL_0;                            /* 2E14, 0x15024E14, DIP_A_AMD_CROP_CTL_0 */
	DIP_X_REG_AMD_CROP_CTL_1                        DIP_X_AMD_CROP_CTL_1;                            /* 2E18, 0x15024E18, DIP_A_AMD_CROP_CTL_1 */
	DIP_X_REG_AMD_CROP_CTL_2                        DIP_X_AMD_CROP_CTL_2;                            /* 2E1C, 0x15024E1C, DIP_A_AMD_CROP_CTL_2 */
	DIP_X_REG_AMD_DCM_DIS                           DIP_X_AMD_DCM_DIS;                               /* 2E20, 0x15024E20, DIP_A_AMD_DCM_DIS */
	DIP_X_REG_AMD_DCM_ST                            DIP_X_AMD_DCM_ST;                                /* 2E24, 0x15024E24, DIP_A_AMD_DCM_ST */
	UINT32                                          rsv_2E28[2];                                     /* 2E28..2E2F, 0x15024E28..15024E2F */
	DIP_X_REG_AMD_DEBUG_SEL                         DIP_X_AMD_DEBUG_SEL;                             /* 2E30, 0x15024E30, DIP_A_AMD_DEBUG_SEL */
	DIP_X_REG_AMD_C24_DEBUG                         DIP_X_AMD_C24_DEBUG;                             /* 2E34, 0x15024E34, DIP_A_AMD_C24_DEBUG */
	DIP_X_REG_AMD_CROP_DEBUG                        DIP_X_AMD_CROP_DEBUG;                            /* 2E38, 0x15024E38, DIP_A_AMD_CROP_DEBUG */
	DIP_X_REG_AMD_ADL_DEBUG                         DIP_X_AMD_ADL_DEBUG;                             /* 2E3C, 0x15024E3C, DIP_A_AMD_ADL_DEBUG */
	DIP_X_REG_AMD_SOF                               DIP_X_AMD_SOF;                                   /* 2E40, 0x15024E40, DIP_A_AMD_SOF */
	DIP_X_REG_AMD_REQ_ST                            DIP_X_AMD_REQ_ST;                                /* 2E44, 0x15024E44, DIP_A_AMD_REQ_ST */
	DIP_X_REG_AMD_RDY_ST                            DIP_X_AMD_RDY_ST;                                /* 2E48, 0x15024E48, DIP_A_AMD_RDY_ST */
	UINT32                                          rsv_2E4C[9];                                     /* 2E4C..2E6F, 0x15024E4C..15024E6F */
	DIP_X_REG_AMD_SPARE                             DIP_X_AMD_SPARE;                                 /* 2E70, 0x15024E70, DIP_A_AMD_SPARE */
	UINT32                                          rsv_2E74[35];                                    /* 2E74..2FFC, 15024E74..15024FFC */
	DIP_X_REG_ISO_ADAP                              DIP_X_ISO_ADAP;                                  /* 3000, 0x15025000 */	// for ISO adaptive
	DIP_X_REG_LCE25_TPIPE_SLM                       DIP_X_LCE25_SLM;                                 /* 3004  0x15025004  */    // for tile(lce_full_slm_wd & lce_full_slm_ht)
	DIP_X_REG_LCE25_TPIPE_OFFSET                    DIP_X_LCE25_OFFSET;                              /* 3008  0x15025008  */    // for tile(lce_full_xoff & lce_full_yoff )
	DIP_X_REG_LCE25_TPIPE_OUT                       DIP_X_LCE25_OUT;                                 /* 300C  0x1502500C */    // for tile(lce_full_out_ht)
	DIP_X_REG_NR3D_VIPI_SIZE                        DIP_X_NR3D_VIPI_SIZE;                            /* 3010  0x15025010  */ // for NR3D vipi size
	DIP_X_REG_NR3D_VIPI_OFFSET                      DIP_X_NR3D_VIPI_OFFSET;                          /* 3014  0x15025014  */ // for NR3D vipi offset
	UINT32                                          rsv_3000[17];                                    /* 3018..305C, 15025018..1502505C */
}dip_x_reg_t;

/* auto insert ralf auto gen above */

#endif // _ISP_REG_DIP_H_
