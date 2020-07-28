#ifndef _ISP_REG_CAM_H_
#define _ISP_REG_CAM_H_

/* auto insert ralf auto gen below */

typedef volatile union _CAM_UNI_REG_TOP_CTL_
{
		volatile struct	/* 0x1A003000 */
		{
				FIELD  INT_MRG                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  UNI_MUX_CFG                           :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_CTL;	/* CAM_UNI_TOP_CTL */

typedef volatile union _CAM_UNI_REG_TOP_MISC_
{
		volatile struct	/* 0x1A003004 */
		{
				FIELD  DB_LOAD_HOLD                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DB_EN                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  APB_CLK_GATE_BYPASS                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MISC;	/* CAM_UNI_TOP_MISC */

typedef volatile union _CAM_UNI_REG_TOP_SW_CTL_
{
		volatile struct	/* 0x1A003008 */
		{
				FIELD  RAWI_SW_RST_Trig                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RAWI_SW_RST_ST                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RAWI_HW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_SW_RST_Trig                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FLKO_SW_RST_ST                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  FLKO_HW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  EISO_RSSO_SW_RST_Trig                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  EISO_RSSO_SW_RST_ST                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  EISO_RSSO_HW_RST                      :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  CTL_HW_RST                            :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_SW_CTL;	/* CAM_UNI_TOP_SW_CTL */

typedef volatile union _CAM_UNI_REG_TOP_RAWI_TRIG_
{
		volatile struct	/* 0x1A00300C */
		{
				FIELD  TOP_RAWI_TRIG                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_RAWI_TRIG;	/* CAM_UNI_TOP_RAWI_TRIG */

typedef volatile union _CAM_UNI_REG_TOP_MOD_EN_
{
		volatile struct	/* 0x1A003010 */
		{
				FIELD  UNP2_A_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_A_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_A_EN                              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_A_EN                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_A_EN                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_A_EN                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  ADL_A_EN                              :  1;		/* 28..28, 0x10000000 */
				FIELD  RLB_A_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_EN;	/* CAM_UNI_TOP_MOD_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_EN_
{
		volatile struct	/* 0x1A003014 */
		{
				FIELD  RAWI_A_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_EN                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_EN;	/* CAM_UNI_TOP_DMA_EN */

typedef volatile union _CAM_UNI_REG_TOP_PATH_SEL_
{
		volatile struct	/* 0x1A003018 */
		{
				FIELD  HDS2_A_SEL                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  FLK2_A_SEL                            :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  EIS_A_SEL                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_PATH_SEL;	/* CAM_UNI_TOP_PATH_SEL */

typedef volatile union _CAM_UNI_REG_TOP_FMT_SEL_
{
		volatile struct	/* 0x1A00301C */
		{
				FIELD  RAWI_FMT                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PIX_BUS_RAWI                          :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  STAG_MODE_RAWI                        :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 17;		/* 14..30, 0x7FFFC000 */
				FIELD  LP_MODE_RAWI                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_FMT_SEL;	/* CAM_UNI_TOP_FMT_SEL */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_EN_
{
		volatile struct	/* 0x1A003020 */
		{
				FIELD  DMA_ERR_INT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_ERR_INT_EN                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_ERR_INT_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_ERR_INT_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 25;		/*  4..28, 0x1FFFFFF0 */
				FIELD  ADL_A_DON_INT_EN                      :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_A_ERR_INT_EN                      :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_EN;	/* CAM_UNI_TOP_DMA_INT_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_STATUS_
{
		volatile struct	/* 0x1A003024 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 25;		/*  4..28, 0x1FFFFFF0 */
				FIELD  ADL_A_DON_INT_STATUS                  :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_A_ERR_INT_STATUS                  :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_STATUS;	/* CAM_UNI_TOP_DMA_INT_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_INT_STATUSX_
{
		volatile struct	/* 0x1A003028 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 25;		/*  4..28, 0x1FFFFFF0 */
				FIELD  ADL_A_DON_INT_STATUS                  :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_A_ERR_INT_STATUS                  :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_INT_STATUSX;	/* CAM_UNI_TOP_DMA_INT_STATUSX */

typedef volatile union _CAM_UNI_REG_TOP_DBG_SET_
{
		volatile struct	/* 0x1A00302C */
		{
				FIELD  DEBUG_MOD_SEL                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DEBUG_SEL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  DEBUG_TOP_SEL                         :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DBG_SET;	/* CAM_UNI_TOP_DBG_SET */

typedef volatile union _CAM_UNI_REG_TOP_DBG_PORT_
{
		volatile struct	/* 0x1A003030 */
		{
				FIELD  TOP_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DBG_PORT;	/* CAM_UNI_TOP_DBG_PORT */

typedef volatile union _CAM_UNI_REG_TOP_DMA_CCU_INT_EN_
{
		volatile struct	/* 0x1A003034 */
		{
				FIELD  DMA_ERR_INT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_ERR_INT_EN                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_ERR_INT_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_ERR_INT_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 25;		/*  4..28, 0x1FFFFFF0 */
				FIELD  ADL_A_DON_INT_EN                      :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_A_ERR_INT_EN                      :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_CCU_INT_EN;	/* CAM_UNI_TOP_DMA_CCU_INT_EN */

typedef volatile union _CAM_UNI_REG_TOP_DMA_CCU_INT_STATUS_
{
		volatile struct	/* 0x1A003038 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 25;		/*  4..28, 0x1FFFFFF0 */
				FIELD  ADL_A_DON_INT_STATUS                  :  1;		/* 29..29, 0x20000000 */
				FIELD  ADL_A_ERR_INT_STATUS                  :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_CCU_INT_STATUS;	/* CAM_UNI_TOP_DMA_CCU_INT_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_DCM_DIS_
{
		volatile struct	/* 0x1A003040 */
		{
				FIELD  UNP2_A_DCM_DIS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SPARE_DCM_DIS_1                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_A_DCM_DIS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_A_DCM_DIS                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_A_DCM_DIS                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SPARE_DCM_DIS_0                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_A_DCM_DIS                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_A_DCM_DIS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SPARE_DCM_DIS_2                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 19;		/*  9..27, 0x0FFFFE00 */
				FIELD  ADL_A_DCM_DIS                         :  1;		/* 28..28, 0x10000000 */
				FIELD  RLB_A_DCM_DIS                         :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_DCM_DIS;	/* CAM_UNI_TOP_MOD_DCM_DIS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_DCM_DIS_
{
		volatile struct	/* 0x1A003044 */
		{
				FIELD  RAWI_A_DCM_DIS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_DCM_DIS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_DCM_DIS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_DCM_DIS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_DCM_DIS;	/* CAM_UNI_TOP_DMA_DCM_DIS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_DCM_STATUS_
{
		volatile struct	/* 0x1A003050 */
		{
				FIELD  UNP2_A_DCM_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_A_DCM_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_A_DCM_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_A_DCM_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_A_DCM_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_A_DCM_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  ADL_A_DCM_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  RLB_A_DCM_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_DCM_STATUS;	/* CAM_UNI_TOP_MOD_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_DCM_STATUS_
{
		volatile struct	/* 0x1A003054 */
		{
				FIELD  RAWI_A_DCM_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_DCM_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_DCM_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_DCM_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_DCM_STATUS;	/* CAM_UNI_TOP_DMA_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_REQ_STATUS_
{
		volatile struct	/* 0x1A003060 */
		{
				FIELD  UNP2_A_REQ_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_A_REQ_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_A_REQ_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_A_REQ_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_A_REQ_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_A_REQ_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  ADL_A_REQ_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  RLB_A_REQ_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_REQ_STATUS;	/* CAM_UNI_TOP_MOD_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_REQ_STATUS_
{
		volatile struct	/* 0x1A003064 */
		{
				FIELD  RAWI_A_REQ_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_REQ_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_REQ_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_REQ_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_REQ_STATUS;	/* CAM_UNI_TOP_DMA_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_MOD_RDY_STATUS_
{
		volatile struct	/* 0x1A003070 */
		{
				FIELD  UNP2_A_RDY_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_A_RDY_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_A_RDY_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_A_RDY_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_A_RDY_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_A_RDY_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  ADL_A_RDY_STATUS                      :  1;		/* 28..28, 0x10000000 */
				FIELD  RLB_A_RDY_STATUS                      :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_MOD_RDY_STATUS;	/* CAM_UNI_TOP_MOD_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_TOP_DMA_RDY_STATUS_
{
		volatile struct	/* 0x1A003074 */
		{
				FIELD  RAWI_A_RDY_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_A_RDY_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_A_RDY_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_A_RDY_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_TOP_DMA_RDY_STATUS;	/* CAM_UNI_TOP_DMA_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_FBC_FLKO_A_CTL1_
{
		volatile struct	/* 0x1A003080 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_FLKO_A_CTL1;	/* CAM_UNI_FBC_FLKO_A_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_FLKO_A_CTL2_
{
		volatile struct	/* 0x1A003084 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_FLKO_A_CTL2;	/* CAM_UNI_FBC_FLKO_A_CTL2 */

typedef volatile union _CAM_UNI_REG_FBC_EISO_A_CTL1_
{
		volatile struct	/* 0x1A003088 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_EISO_A_CTL1;	/* CAM_UNI_FBC_EISO_A_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_EISO_A_CTL2_
{
		volatile struct	/* 0x1A00308C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_EISO_A_CTL2;	/* CAM_UNI_FBC_EISO_A_CTL2 */

typedef volatile union _CAM_UNI_REG_FBC_RSSO_A_CTL1_
{
		volatile struct	/* 0x1A003090 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_RSSO_A_CTL1;	/* CAM_UNI_FBC_RSSO_A_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_RSSO_A_CTL2_
{
		volatile struct	/* 0x1A003094 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_RSSO_A_CTL2;	/* CAM_UNI_FBC_RSSO_A_CTL2 */

typedef volatile union _CAM_UNI_REG_B_TOP_MISC_
{
		volatile struct	/* 0x1A003104 */
		{
				FIELD  DB_LOAD_HOLD                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DB_EN                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  APB_CLK_GATE_BYPASS                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MISC;	/* CAM_UNI_B_TOP_MISC */

typedef volatile union _CAM_UNI_REG_B_TOP_SW_CTL_
{
		volatile struct	/* 0x1A003108 */
		{
				FIELD  RAWI_SW_RST_Trig                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RAWI_SW_RST_ST                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RAWI_HW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_SW_RST_Trig                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FLKO_SW_RST_ST                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  FLKO_HW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  EISO_RSSO_SW_RST_Trig                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  EISO_RSSO_SW_RST_ST                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  EISO_RSSO_HW_RST                      :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_SW_CTL;	/* CAM_UNI_B_TOP_SW_CTL */

typedef volatile union _CAM_UNI_REG_B_TOP_RAWI_TRIG_
{
		volatile struct	/* 0x1A00310C */
		{
				FIELD  TOP_RAWI_TRIG                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_RAWI_TRIG;	/* CAM_UNI_B_TOP_RAWI_TRIG */

typedef volatile union _CAM_UNI_REG_B_TOP_MOD_EN_
{
		volatile struct	/* 0x1A003110 */
		{
				FIELD  UNP2_B_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_B_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_B_EN                              :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_B_EN                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_B_EN                              :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_B_EN                              :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MOD_EN;	/* CAM_UNI_B_TOP_MOD_EN */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_EN_
{
		volatile struct	/* 0x1A003114 */
		{
				FIELD  RAWI_B_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_EN                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_EN;	/* CAM_UNI_B_TOP_DMA_EN */

typedef volatile union _CAM_UNI_REG_B_TOP_PATH_SEL_
{
		volatile struct	/* 0x1A003118 */
		{
				FIELD  HDS2_B_SEL                            :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  FLK2_B_SEL                            :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  EIS_B_SEL                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_PATH_SEL;	/* CAM_UNI_B_TOP_PATH_SEL */

typedef volatile union _CAM_UNI_REG_B_TOP_FMT_SEL_
{
		volatile struct	/* 0x1A00311C */
		{
				FIELD  RAWI_FMT                              :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  PIX_BUS_RAWI                          :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  STAG_MODE_RAWI                        :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 17;		/* 14..30, 0x7FFFC000 */
				FIELD  LP_MODE_RAWI                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_FMT_SEL;	/* CAM_UNI_B_TOP_FMT_SEL */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_INT_EN_
{
		volatile struct	/* 0x1A003120 */
		{
				FIELD  DMA_ERR_INT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_ERR_INT_EN                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_ERR_INT_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR_INT_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 27;		/*  4..30, 0x7FFFFFF0 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_INT_EN;	/* CAM_UNI_B_TOP_DMA_INT_EN */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_INT_STATUS_
{
		volatile struct	/* 0x1A003124 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_INT_STATUS;	/* CAM_UNI_B_TOP_DMA_INT_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_INT_STATUSX_
{
		volatile struct	/* 0x1A003128 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_INT_STATUSX;	/* CAM_UNI_B_TOP_DMA_INT_STATUSX */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_CCU_INT_EN_
{
		volatile struct	/* 0x1A003134 */
		{
				FIELD  DMA_ERR_INT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_ERR_INT_EN                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_ERR_INT_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR_INT_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 27;		/*  4..30, 0x7FFFFFF0 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_CCU_INT_EN;	/* CAM_UNI_B_TOP_DMA_CCU_INT_EN */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_CCU_INT_STATUS_
{
		volatile struct	/* 0x1A003138 */
		{
				FIELD  DMA_ERR_INT_STATUS                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_ERR_INT_STATUS                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_ERR_INT_STATUS                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR_INT_STATUS                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_CCU_INT_STATUS;	/* CAM_UNI_B_TOP_DMA_CCU_INT_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_MOD_DCM_DIS_
{
		volatile struct	/* 0x1A003140 */
		{
				FIELD  UNP2_B_DCM_DIS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SPARE_DCM_DIS_1                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_B_DCM_DIS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_B_DCM_DIS                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_B_DCM_DIS                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SPARE_DCM_DIS_0                       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_B_DCM_DIS                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_B_DCM_DIS                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SPARE_DCM_DIS_2                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MOD_DCM_DIS;	/* CAM_UNI_B_TOP_MOD_DCM_DIS */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_DCM_DIS_
{
		volatile struct	/* 0x1A003144 */
		{
				FIELD  RAWI_B_DCM_DIS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_DCM_DIS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_DCM_DIS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_DCM_DIS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_DCM_DIS;	/* CAM_UNI_B_TOP_DMA_DCM_DIS */

typedef volatile union _CAM_UNI_REG_B_TOP_MOD_DCM_STATUS_
{
		volatile struct	/* 0x1A003150 */
		{
				FIELD  UNP2_B_DCM_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_B_DCM_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_B_DCM_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_B_DCM_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_B_DCM_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_B_DCM_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MOD_DCM_STATUS;	/* CAM_UNI_B_TOP_MOD_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_DCM_STATUS_
{
		volatile struct	/* 0x1A003154 */
		{
				FIELD  RAWI_B_DCM_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_DCM_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_DCM_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_DCM_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_DCM_STATUS;	/* CAM_UNI_B_TOP_DMA_DCM_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_MOD_REQ_STATUS_
{
		volatile struct	/* 0x1A003160 */
		{
				FIELD  UNP2_B_REQ_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_B_REQ_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_B_REQ_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_B_REQ_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_B_REQ_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_B_REQ_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MOD_REQ_STATUS;	/* CAM_UNI_B_TOP_MOD_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_REQ_STATUS_
{
		volatile struct	/* 0x1A003164 */
		{
				FIELD  RAWI_B_REQ_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_REQ_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_REQ_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_REQ_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_REQ_STATUS;	/* CAM_UNI_B_TOP_DMA_REQ_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_MOD_RDY_STATUS_
{
		volatile struct	/* 0x1A003170 */
		{
				FIELD  UNP2_B_RDY_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGG3_B_RDY_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FLK_B_RDY_STATUS                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EIS_B_RDY_STATUS                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  HDS_B_RDY_STATUS                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RSS_B_RDY_STATUS                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_MOD_RDY_STATUS;	/* CAM_UNI_B_TOP_MOD_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_B_TOP_DMA_RDY_STATUS_
{
		volatile struct	/* 0x1A003174 */
		{
				FIELD  RAWI_B_RDY_STATUS                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_B_RDY_STATUS                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  EISO_B_RDY_STATUS                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_RDY_STATUS                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_TOP_DMA_RDY_STATUS;	/* CAM_UNI_B_TOP_DMA_RDY_STATUS */

typedef volatile union _CAM_UNI_REG_FBC_FLKO_B_CTL1_
{
		volatile struct	/* 0x1A003180 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_FLKO_B_CTL1;	/* CAM_UNI_FBC_FLKO_B_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_FLKO_B_CTL2_
{
		volatile struct	/* 0x1A003184 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_FLKO_B_CTL2;	/* CAM_UNI_FBC_FLKO_B_CTL2 */

typedef volatile union _CAM_UNI_REG_FBC_EISO_B_CTL1_
{
		volatile struct	/* 0x1A003188 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_EISO_B_CTL1;	/* CAM_UNI_FBC_EISO_B_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_EISO_B_CTL2_
{
		volatile struct	/* 0x1A00318C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_EISO_B_CTL2;	/* CAM_UNI_FBC_EISO_B_CTL2 */

typedef volatile union _CAM_UNI_REG_FBC_RSSO_B_CTL1_
{
		volatile struct	/* 0x1A003190 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_RSSO_B_CTL1;	/* CAM_UNI_FBC_RSSO_B_CTL1 */

typedef volatile union _CAM_UNI_REG_FBC_RSSO_B_CTL2_
{
		volatile struct	/* 0x1A003194 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FBC_RSSO_B_CTL2;	/* CAM_UNI_FBC_RSSO_B_CTL2 */

typedef volatile union _CAM_UNI_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1A003200 */
		{
				FIELD  EISO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_SOFT_RST_STAT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_SOFT_RST_STAT                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_SOFT_RST_STAT                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_SOFT_RSTSTAT;	/* CAM_UNI_DMA_SOFT_RSTSTAT */

typedef volatile union _CAM_UNI_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x1A003204 */
		{
				FIELD  EISO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_V_FLIP_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_V_FLIP_EN                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_V_FLIP_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_VERTICAL_FLIP_EN;	/* CAM_UNI_VERTICAL_FLIP_EN */

typedef volatile union _CAM_UNI_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x1A003208 */
		{
				FIELD  EISO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_SOFT_RST                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_SOFT_RST                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_SOFT_RST                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_SOFT_RESET;	/* CAM_UNI_DMA_SOFT_RESET */

typedef volatile union _CAM_UNI_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1A00320C */
		{
				FIELD  EISO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_LAST_ULTRA_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_LAST_ULTRA_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_LAST_ULTRA_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_LAST_ULTRA_EN;	/* CAM_UNI_LAST_ULTRA_EN */

typedef volatile union _CAM_UNI_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1A003210 */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  GCLAST_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SPECIAL_FUN_EN;	/* CAM_UNI_SPECIAL_FUN_EN */

typedef volatile union _CAM_UNI_REG_SPECIAL_FUN2_EN_
{
		volatile struct	/* 0x1A003214 */
		{
				FIELD  STAGER_SENSOR_DMA_DLNO                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  STAGER_SENSOR_DMA_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SPECIAL_FUN2_EN;	/* CAM_UNI_SPECIAL_FUN2_EN */

typedef volatile union _CAM_UNI_REG_EISO_BASE_ADDR_
{
		volatile struct	/* 0x1A003220 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_BASE_ADDR;	/* CAM_UNI_EISO_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_EISO_OFST_ADDR_
{
		volatile struct	/* 0x1A003228 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_OFST_ADDR;	/* CAM_UNI_EISO_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_EISO_DRS_
{
		volatile struct	/* 0x1A00322C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_DRS;	/* CAM_UNI_EISO_DRS */

typedef volatile union _CAM_UNI_REG_EISO_XSIZE_
{
		volatile struct	/* 0x1A003230 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_XSIZE;	/* CAM_UNI_EISO_XSIZE */

typedef volatile union _CAM_UNI_REG_EISO_YSIZE_
{
		volatile struct	/* 0x1A003234 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_YSIZE;	/* CAM_UNI_EISO_YSIZE */

typedef volatile union _CAM_UNI_REG_EISO_STRIDE_
{
		volatile struct	/* 0x1A003238 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_STRIDE;	/* CAM_UNI_EISO_STRIDE */

typedef volatile union _CAM_UNI_REG_EISO_CON_
{
		volatile struct	/* 0x1A00323C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_CON;	/* CAM_UNI_EISO_CON */

typedef volatile union _CAM_UNI_REG_EISO_CON2_
{
		volatile struct	/* 0x1A003240 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_CON2;	/* CAM_UNI_EISO_CON2 */

typedef volatile union _CAM_UNI_REG_EISO_CON3_
{
		volatile struct	/* 0x1A003244 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_CON3;	/* CAM_UNI_EISO_CON3 */

typedef volatile union _CAM_UNI_REG_EISO_CON4_
{
		volatile struct	/* 0x1A00324C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_CON4;	/* CAM_UNI_EISO_CON4 */

typedef volatile union _CAM_UNI_REG_FLKO_BASE_ADDR_
{
		volatile struct	/* 0x1A003250 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_BASE_ADDR;	/* CAM_UNI_FLKO_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_FLKO_OFST_ADDR_
{
		volatile struct	/* 0x1A003258 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_OFST_ADDR;	/* CAM_UNI_FLKO_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_FLKO_DRS_
{
		volatile struct	/* 0x1A00325C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_DRS;	/* CAM_UNI_FLKO_DRS */

typedef volatile union _CAM_UNI_REG_FLKO_XSIZE_
{
		volatile struct	/* 0x1A003260 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_XSIZE;	/* CAM_UNI_FLKO_XSIZE */

typedef volatile union _CAM_UNI_REG_FLKO_YSIZE_
{
		volatile struct	/* 0x1A003264 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_YSIZE;	/* CAM_UNI_FLKO_YSIZE */

typedef volatile union _CAM_UNI_REG_FLKO_STRIDE_
{
		volatile struct	/* 0x1A003268 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  7;		/* 17..23, 0x00FE0000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_STRIDE;	/* CAM_UNI_FLKO_STRIDE */

typedef volatile union _CAM_UNI_REG_FLKO_CON_
{
		volatile struct	/* 0x1A00326C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_CON;	/* CAM_UNI_FLKO_CON */

typedef volatile union _CAM_UNI_REG_FLKO_CON2_
{
		volatile struct	/* 0x1A003270 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_CON2;	/* CAM_UNI_FLKO_CON2 */

typedef volatile union _CAM_UNI_REG_FLKO_CON3_
{
		volatile struct	/* 0x1A003274 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_CON3;	/* CAM_UNI_FLKO_CON3 */

typedef volatile union _CAM_UNI_REG_FLKO_CON4_
{
		volatile struct	/* 0x1A00327C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_CON4;	/* CAM_UNI_FLKO_CON4 */

typedef volatile union _CAM_UNI_REG_RSSO_A_BASE_ADDR_
{
		volatile struct	/* 0x1A003280 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_BASE_ADDR;	/* CAM_UNI_RSSO_A_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_A_OFST_ADDR_
{
		volatile struct	/* 0x1A003288 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_OFST_ADDR;	/* CAM_UNI_RSSO_A_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_A_DRS_
{
		volatile struct	/* 0x1A00328C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_DRS;	/* CAM_UNI_RSSO_A_DRS */

typedef volatile union _CAM_UNI_REG_RSSO_A_XSIZE_
{
		volatile struct	/* 0x1A003290 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_XSIZE;	/* CAM_UNI_RSSO_A_XSIZE */

typedef volatile union _CAM_UNI_REG_RSSO_A_YSIZE_
{
		volatile struct	/* 0x1A003294 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_YSIZE;	/* CAM_UNI_RSSO_A_YSIZE */

typedef volatile union _CAM_UNI_REG_RSSO_A_STRIDE_
{
		volatile struct	/* 0x1A003298 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_STRIDE;	/* CAM_UNI_RSSO_A_STRIDE */

typedef volatile union _CAM_UNI_REG_RSSO_A_CON_
{
		volatile struct	/* 0x1A00329C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_CON;	/* CAM_UNI_RSSO_A_CON */

typedef volatile union _CAM_UNI_REG_RSSO_A_CON2_
{
		volatile struct	/* 0x1A0032A0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_CON2;	/* CAM_UNI_RSSO_A_CON2 */

typedef volatile union _CAM_UNI_REG_RSSO_A_CON3_
{
		volatile struct	/* 0x1A0032A4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_CON3;	/* CAM_UNI_RSSO_A_CON3 */

typedef volatile union _CAM_UNI_REG_RSSO_A_CON4_
{
		volatile struct	/* 0x1A0032AC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_CON4;	/* CAM_UNI_RSSO_A_CON4 */

typedef volatile union _CAM_UNI_REG_RSSO_B_BASE_ADDR_
{
		volatile struct	/* 0x1A0032B0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_BASE_ADDR;	/* CAM_UNI_RSSO_B_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_B_OFST_ADDR_
{
		volatile struct	/* 0x1A0032B8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_OFST_ADDR;	/* CAM_UNI_RSSO_B_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_B_DRS_
{
		volatile struct	/* 0x1A0032BC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_DRS;	/* CAM_UNI_RSSO_B_DRS */

typedef volatile union _CAM_UNI_REG_RSSO_B_XSIZE_
{
		volatile struct	/* 0x1A0032C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_XSIZE;	/* CAM_UNI_RSSO_B_XSIZE */

typedef volatile union _CAM_UNI_REG_RSSO_B_YSIZE_
{
		volatile struct	/* 0x1A0032C4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_YSIZE;	/* CAM_UNI_RSSO_B_YSIZE */

typedef volatile union _CAM_UNI_REG_RSSO_B_STRIDE_
{
		volatile struct	/* 0x1A0032C8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_STRIDE;	/* CAM_UNI_RSSO_B_STRIDE */

typedef volatile union _CAM_UNI_REG_RSSO_B_CON_
{
		volatile struct	/* 0x1A0032CC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_CON;	/* CAM_UNI_RSSO_B_CON */

typedef volatile union _CAM_UNI_REG_RSSO_B_CON2_
{
		volatile struct	/* 0x1A0032D0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_CON2;	/* CAM_UNI_RSSO_B_CON2 */

typedef volatile union _CAM_UNI_REG_RSSO_B_CON3_
{
		volatile struct	/* 0x1A0032D4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_CON3;	/* CAM_UNI_RSSO_B_CON3 */

typedef volatile union _CAM_UNI_REG_RSSO_B_CON4_
{
		volatile struct	/* 0x1A0032DC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_CON4;	/* CAM_UNI_RSSO_B_CON4 */

typedef volatile union _CAM_UNI_REG_RAWI_BASE_ADDR_
{
		volatile struct	/* 0x1A003340 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_BASE_ADDR;	/* CAM_UNI_RAWI_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RAWI_OFST_ADDR_
{
		volatile struct	/* 0x1A003348 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_OFST_ADDR;	/* CAM_UNI_RAWI_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_RAWI_DRS_
{
		volatile struct	/* 0x1A00334C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_DRS;	/* CAM_UNI_RAWI_DRS */

typedef volatile union _CAM_UNI_REG_RAWI_XSIZE_
{
		volatile struct	/* 0x1A003350 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_XSIZE;	/* CAM_UNI_RAWI_XSIZE */

typedef volatile union _CAM_UNI_REG_RAWI_YSIZE_
{
		volatile struct	/* 0x1A003354 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_YSIZE;	/* CAM_UNI_RAWI_YSIZE */

typedef volatile union _CAM_UNI_REG_RAWI_STRIDE_
{
		volatile struct	/* 0x1A003358 */
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
}CAM_UNI_REG_RAWI_STRIDE;	/* CAM_UNI_RAWI_STRIDE */

typedef volatile union _CAM_UNI_REG_RAWI_CON_
{
		volatile struct	/* 0x1A00335C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_CON;	/* CAM_UNI_RAWI_CON */

typedef volatile union _CAM_UNI_REG_RAWI_CON2_
{
		volatile struct	/* 0x1A003360 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_CON2;	/* CAM_UNI_RAWI_CON2 */

typedef volatile union _CAM_UNI_REG_RAWI_CON3_
{
		volatile struct	/* 0x1A003364 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_CON3;	/* CAM_UNI_RAWI_CON3 */

typedef volatile union _CAM_UNI_REG_RAWI_CON4_
{
		volatile struct	/* 0x1A00336C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_CON4;	/* CAM_UNI_RAWI_CON4 */

typedef volatile union _CAM_UNI_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x1A003370 */
		{
				FIELD  EISO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_ERR                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_ERR                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_ERR_CTRL;	/* CAM_UNI_DMA_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_EISO_ERR_STAT_
{
		volatile struct	/* 0x1A003374 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_ERR_STAT;	/* CAM_UNI_EISO_ERR_STAT */

typedef volatile union _CAM_UNI_REG_FLKO_ERR_STAT_
{
		volatile struct	/* 0x1A003378 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_ERR_STAT;	/* CAM_UNI_FLKO_ERR_STAT */

typedef volatile union _CAM_UNI_REG_RSSO_A_ERR_STAT_
{
		volatile struct	/* 0x1A00337C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_ERR_STAT;	/* CAM_UNI_RSSO_A_ERR_STAT */

typedef volatile union _CAM_UNI_REG_RSSO_B_ERR_STAT_
{
		volatile struct	/* 0x1A003380 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_ERR_STAT;	/* CAM_UNI_RSSO_B_ERR_STAT */

typedef volatile union _CAM_UNI_REG_RAWI_ERR_STAT_
{
		volatile struct	/* 0x1A003384 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RAWI_ERR_STAT;	/* CAM_UNI_RAWI_ERR_STAT */

typedef volatile union _CAM_UNI_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1A003388 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_DEBUG_ADDR;	/* CAM_UNI_DMA_DEBUG_ADDR */

typedef volatile union _CAM_UNI_REG_DMA_RSV1_
{
		volatile struct	/* 0x1A00338C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV1;	/* CAM_UNI_DMA_RSV1 */

typedef volatile union _CAM_UNI_REG_DMA_RSV2_
{
		volatile struct	/* 0x1A003390 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV2;	/* CAM_UNI_DMA_RSV2 */

typedef volatile union _CAM_UNI_REG_DMA_RSV3_
{
		volatile struct	/* 0x1A003394 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV3;	/* CAM_UNI_DMA_RSV3 */

typedef volatile union _CAM_UNI_REG_DMA_RSV4_
{
		volatile struct	/* 0x1A003398 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV4;	/* CAM_UNI_DMA_RSV4 */

typedef volatile union _CAM_UNI_REG_DMA_RSV5_
{
		volatile struct	/* 0x1A00339C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV5;	/* CAM_UNI_DMA_RSV5 */

typedef volatile union _CAM_UNI_REG_DMA_RSV6_
{
		volatile struct	/* 0x1A0033A0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_RSV6;	/* CAM_UNI_DMA_RSV6 */

typedef volatile union _CAM_UNI_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x1A0033A4 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_DEBUG_SEL;	/* CAM_UNI_DMA_DEBUG_SEL */

typedef volatile union _CAM_UNI_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1A0033A8 */
		{
				FIELD  BW_SELF_TEST_EN_EISO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_FLKO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_RSSO_A                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_RSSO_B                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_BW_SELF_TEST;	/* CAM_UNI_DMA_BW_SELF_TEST */

typedef volatile union _CAM_UNI_REG_DMA_FRAME_HEADER_EN_
{
		volatile struct	/* 0x1A0033C0 */
		{
				FIELD  FRAME_HEADER_EN_EISO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FRAME_HEADER_EN_FLKO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FRAME_HEADER_EN_RSSO_A                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FRAME_HEADER_EN_RSSO_B                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_DMA_FRAME_HEADER_EN;	/* CAM_UNI_DMA_FRAME_HEADER_EN */

typedef volatile union _CAM_UNI_REG_EISO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0033C4 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_BASE_ADDR;	/* CAM_UNI_EISO_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_FLKO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0033C8 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_BASE_ADDR;	/* CAM_UNI_FLKO_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0033CC */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_BASE_ADDR;	/* CAM_UNI_RSSO_A_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0033D0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_BASE_ADDR;	/* CAM_UNI_RSSO_B_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_2_
{
		volatile struct	/* 0x1A0033E0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_2;	/* CAM_UNI_EISO_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_3_
{
		volatile struct	/* 0x1A0033E4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_3;	/* CAM_UNI_EISO_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_4_
{
		volatile struct	/* 0x1A0033E8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_4;	/* CAM_UNI_EISO_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_5_
{
		volatile struct	/* 0x1A0033EC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_5;	/* CAM_UNI_EISO_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_6_
{
		volatile struct	/* 0x1A0033F0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_6;	/* CAM_UNI_EISO_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_7_
{
		volatile struct	/* 0x1A0033F4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_7;	/* CAM_UNI_EISO_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_8_
{
		volatile struct	/* 0x1A0033F8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_8;	/* CAM_UNI_EISO_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_9_
{
		volatile struct	/* 0x1A0033FC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_9;	/* CAM_UNI_EISO_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_10_
{
		volatile struct	/* 0x1A003400 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_10;	/* CAM_UNI_EISO_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_11_
{
		volatile struct	/* 0x1A003404 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_11;	/* CAM_UNI_EISO_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_12_
{
		volatile struct	/* 0x1A003408 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_12;	/* CAM_UNI_EISO_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_13_
{
		volatile struct	/* 0x1A00340C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_13;	/* CAM_UNI_EISO_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_14_
{
		volatile struct	/* 0x1A003410 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_14;	/* CAM_UNI_EISO_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_15_
{
		volatile struct	/* 0x1A003414 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_15;	/* CAM_UNI_EISO_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_EISO_FH_SPARE_16_
{
		volatile struct	/* 0x1A003418 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EISO_FH_SPARE_16;	/* CAM_UNI_EISO_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_2_
{
		volatile struct	/* 0x1A003420 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_2;	/* CAM_UNI_FLKO_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_3_
{
		volatile struct	/* 0x1A003424 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_3;	/* CAM_UNI_FLKO_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_4_
{
		volatile struct	/* 0x1A003428 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_4;	/* CAM_UNI_FLKO_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_5_
{
		volatile struct	/* 0x1A00342C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_5;	/* CAM_UNI_FLKO_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_6_
{
		volatile struct	/* 0x1A003430 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_6;	/* CAM_UNI_FLKO_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_7_
{
		volatile struct	/* 0x1A003434 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_7;	/* CAM_UNI_FLKO_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_8_
{
		volatile struct	/* 0x1A003438 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_8;	/* CAM_UNI_FLKO_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_9_
{
		volatile struct	/* 0x1A00343C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_9;	/* CAM_UNI_FLKO_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_10_
{
		volatile struct	/* 0x1A003440 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_10;	/* CAM_UNI_FLKO_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_11_
{
		volatile struct	/* 0x1A003444 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_11;	/* CAM_UNI_FLKO_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_12_
{
		volatile struct	/* 0x1A003448 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_12;	/* CAM_UNI_FLKO_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_13_
{
		volatile struct	/* 0x1A00344C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_13;	/* CAM_UNI_FLKO_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_14_
{
		volatile struct	/* 0x1A003450 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_14;	/* CAM_UNI_FLKO_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_15_
{
		volatile struct	/* 0x1A003454 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_15;	/* CAM_UNI_FLKO_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_FLKO_FH_SPARE_16_
{
		volatile struct	/* 0x1A003458 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLKO_FH_SPARE_16;	/* CAM_UNI_FLKO_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_2_
{
		volatile struct	/* 0x1A003460 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_2;	/* CAM_UNI_RSSO_A_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_3_
{
		volatile struct	/* 0x1A003464 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_3;	/* CAM_UNI_RSSO_A_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_4_
{
		volatile struct	/* 0x1A003468 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_4;	/* CAM_UNI_RSSO_A_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_5_
{
		volatile struct	/* 0x1A00346C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_5;	/* CAM_UNI_RSSO_A_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_6_
{
		volatile struct	/* 0x1A003470 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_6;	/* CAM_UNI_RSSO_A_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_7_
{
		volatile struct	/* 0x1A003474 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_7;	/* CAM_UNI_RSSO_A_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_8_
{
		volatile struct	/* 0x1A003478 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_8;	/* CAM_UNI_RSSO_A_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_9_
{
		volatile struct	/* 0x1A00347C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_9;	/* CAM_UNI_RSSO_A_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_10_
{
		volatile struct	/* 0x1A003480 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_10;	/* CAM_UNI_RSSO_A_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_11_
{
		volatile struct	/* 0x1A003484 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_11;	/* CAM_UNI_RSSO_A_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_12_
{
		volatile struct	/* 0x1A003488 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_12;	/* CAM_UNI_RSSO_A_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_13_
{
		volatile struct	/* 0x1A00348C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_13;	/* CAM_UNI_RSSO_A_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_14_
{
		volatile struct	/* 0x1A003490 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_14;	/* CAM_UNI_RSSO_A_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_15_
{
		volatile struct	/* 0x1A003494 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_15;	/* CAM_UNI_RSSO_A_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_RSSO_A_FH_SPARE_16_
{
		volatile struct	/* 0x1A003498 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_A_FH_SPARE_16;	/* CAM_UNI_RSSO_A_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_2_
{
		volatile struct	/* 0x1A0034A0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_2;	/* CAM_UNI_RSSO_B_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_3_
{
		volatile struct	/* 0x1A0034A4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_3;	/* CAM_UNI_RSSO_B_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_4_
{
		volatile struct	/* 0x1A0034A8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_4;	/* CAM_UNI_RSSO_B_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_5_
{
		volatile struct	/* 0x1A0034AC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_5;	/* CAM_UNI_RSSO_B_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_6_
{
		volatile struct	/* 0x1A0034B0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_6;	/* CAM_UNI_RSSO_B_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_7_
{
		volatile struct	/* 0x1A0034B4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_7;	/* CAM_UNI_RSSO_B_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_8_
{
		volatile struct	/* 0x1A0034B8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_8;	/* CAM_UNI_RSSO_B_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_9_
{
		volatile struct	/* 0x1A0034BC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_9;	/* CAM_UNI_RSSO_B_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_10_
{
		volatile struct	/* 0x1A0034C0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_10;	/* CAM_UNI_RSSO_B_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_11_
{
		volatile struct	/* 0x1A0034C4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_11;	/* CAM_UNI_RSSO_B_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_12_
{
		volatile struct	/* 0x1A0034C8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_12;	/* CAM_UNI_RSSO_B_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_13_
{
		volatile struct	/* 0x1A0034CC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_13;	/* CAM_UNI_RSSO_B_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_14_
{
		volatile struct	/* 0x1A0034D0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_14;	/* CAM_UNI_RSSO_B_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_15_
{
		volatile struct	/* 0x1A0034D4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_15;	/* CAM_UNI_RSSO_B_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_RSSO_B_FH_SPARE_16_
{
		volatile struct	/* 0x1A0034D8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSSO_B_FH_SPARE_16;	/* CAM_UNI_RSSO_B_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_UNP2_A_OFST_
{
		volatile struct	/* 0x1A003500 */
		{
				FIELD  UNP2_OFST_STB                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  UNP2_OFST_EDB                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_UNP2_A_OFST;	/* CAM_UNI_UNP2_A_OFST */

typedef volatile union _CAM_UNI_REG_QBN3_A_MODE_
{
		volatile struct	/* 0x1A003510 */
		{
				FIELD  QBN_ACC                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  QBN_ACC_MODE                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_QBN3_A_MODE;	/* CAM_UNI_QBN3_A_MODE */

typedef volatile union _CAM_UNI_REG_SGG3_A_PGN_
{
		volatile struct	/* 0x1A003520 */
		{
				FIELD  SGG_GAIN                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_A_PGN;	/* CAM_UNI_SGG3_A_PGN */

typedef volatile union _CAM_UNI_REG_SGG3_A_GMRC_1_
{
		volatile struct	/* 0x1A003524 */
		{
				FIELD  SGG_GMR_1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG_GMR_4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_A_GMRC_1;	/* CAM_UNI_SGG3_A_GMRC_1 */

typedef volatile union _CAM_UNI_REG_SGG3_A_GMRC_2_
{
		volatile struct	/* 0x1A003528 */
		{
				FIELD  SGG_GMR_5                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_6                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_7                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_A_GMRC_2;	/* CAM_UNI_SGG3_A_GMRC_2 */

typedef volatile union _CAM_UNI_REG_FLK_A_CON_
{
		volatile struct	/* 0x1A003530 */
		{
				FIELD  RESERVED                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_A_CON;	/* CAM_UNI_FLK_A_CON */

typedef volatile union _CAM_UNI_REG_FLK_A_OFST_
{
		volatile struct	/* 0x1A003534 */
		{
				FIELD  FLK_OFST_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_OFST_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_A_OFST;	/* CAM_UNI_FLK_A_OFST */

typedef volatile union _CAM_UNI_REG_FLK_A_SIZE_
{
		volatile struct	/* 0x1A003538 */
		{
				FIELD  FLK_SIZE_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_SIZE_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_A_SIZE;	/* CAM_UNI_FLK_A_SIZE */

typedef volatile union _CAM_UNI_REG_FLK_A_NUM_
{
		volatile struct	/* 0x1A00353C */
		{
				FIELD  FLK_NUM_X                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLK_NUM_Y                             :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_A_NUM;	/* CAM_UNI_FLK_A_NUM */

typedef volatile union _CAM_UNI_REG_EIS_A_PREP_ME_CTRL1_
{
		volatile struct	/* 0x1A003550 */
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
}CAM_UNI_REG_EIS_A_PREP_ME_CTRL1;	/* CAM_UNI_EIS_A_PREP_ME_CTRL1 */

typedef volatile union _CAM_UNI_REG_EIS_A_PREP_ME_CTRL2_
{
		volatile struct	/* 0x1A003554 */
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
}CAM_UNI_REG_EIS_A_PREP_ME_CTRL2;	/* CAM_UNI_EIS_A_PREP_ME_CTRL2 */

typedef volatile union _CAM_UNI_REG_EIS_A_LMV_TH_
{
		volatile struct	/* 0x1A003558 */
		{
				FIELD  LMV_TH_Y_SURROUND                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LMV_TH_Y_CENTER                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LMV_TH_X_SOURROUND                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LMV_TH_X_CENTER                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_LMV_TH;	/* CAM_UNI_EIS_A_LMV_TH */

typedef volatile union _CAM_UNI_REG_EIS_A_FL_OFFSET_
{
		volatile struct	/* 0x1A00355C */
		{
				FIELD  EIS_WIN_VOFST                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HOFST                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_FL_OFFSET;	/* CAM_UNI_EIS_A_FL_OFFSET */

typedef volatile union _CAM_UNI_REG_EIS_A_MB_OFFSET_
{
		volatile struct	/* 0x1A003560 */
		{
				FIELD  EIS_RP_VOFST                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_RP_HOFST                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_MB_OFFSET;	/* CAM_UNI_EIS_A_MB_OFFSET */

typedef volatile union _CAM_UNI_REG_EIS_A_MB_INTERVAL_
{
		volatile struct	/* 0x1A003564 */
		{
				FIELD  EIS_WIN_VSIZE                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HSIZE                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_MB_INTERVAL;	/* CAM_UNI_EIS_A_MB_INTERVAL */

typedef volatile union _CAM_UNI_REG_EIS_A_GMV_
{
		volatile struct	/* 0x1A003568 */
		{
				FIELD  GMV_Y                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  GMV_X                                 : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_GMV;	/* CAM_UNI_EIS_A_GMV */

typedef volatile union _CAM_UNI_REG_EIS_A_ERR_CTRL_
{
		volatile struct	/* 0x1A00356C */
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
}CAM_UNI_REG_EIS_A_ERR_CTRL;	/* CAM_UNI_EIS_A_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_EIS_A_IMAGE_CTRL_
{
		volatile struct	/* 0x1A003570 */
		{
				FIELD  HEIGHT                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WIDTH                                 : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PIPE_MODE                             :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_A_IMAGE_CTRL;	/* CAM_UNI_EIS_A_IMAGE_CTRL */

typedef volatile union _CAM_UNI_REG_HDS_A_MODE_
{
		volatile struct	/* 0x1A0035A0 */
		{
				FIELD  HDS_DS                                :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_HDS_A_MODE;	/* CAM_UNI_HDS_A_MODE */

typedef volatile union _CAM_UNI_REG_RSS_A_CONTROL_
{
		volatile struct	/* 0x1A0035C0 */
		{
				FIELD  RSS_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RSS_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSS_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RSS_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 11;		/*  5..15, 0x0000FFE0 */
				FIELD  RSS_HORI_TBL_SEL                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  RSS_VERT_TBL_SEL                      :  5;		/* 21..25, 0x03E00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_CONTROL;	/* CAM_UNI_RSS_A_CONTROL */

typedef volatile union _CAM_UNI_REG_RSS_A_IN_IMG_
{
		volatile struct	/* 0x1A0035C4 */
		{
				FIELD  RSS_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RSS_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_IN_IMG;	/* CAM_UNI_RSS_A_IN_IMG */

typedef volatile union _CAM_UNI_REG_RSS_A_OUT_IMG_
{
		volatile struct	/* 0x1A0035C8 */
		{
				FIELD  RSS_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RSS_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_OUT_IMG;	/* CAM_UNI_RSS_A_OUT_IMG */

typedef volatile union _CAM_UNI_REG_RSS_A_HORI_STEP_
{
		volatile struct	/* 0x1A0035CC */
		{
				FIELD  RSS_HORI_STEP                         : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_HORI_STEP;	/* CAM_UNI_RSS_A_HORI_STEP */

typedef volatile union _CAM_UNI_REG_RSS_A_VERT_STEP_
{
		volatile struct	/* 0x1A0035D0 */
		{
				FIELD  RSS_VERT_STEP                         : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_VERT_STEP;	/* CAM_UNI_RSS_A_VERT_STEP */

typedef volatile union _CAM_UNI_REG_RSS_A_HORI_INT_OFST_
{
		volatile struct	/* 0x1A0035D4 */
		{
				FIELD  RSS_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_HORI_INT_OFST;	/* CAM_UNI_RSS_A_HORI_INT_OFST */

typedef volatile union _CAM_UNI_REG_RSS_A_HORI_SUB_OFST_
{
		volatile struct	/* 0x1A0035D8 */
		{
				FIELD  RSS_HORI_SUB_OFST                     : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_HORI_SUB_OFST;	/* CAM_UNI_RSS_A_HORI_SUB_OFST */

typedef volatile union _CAM_UNI_REG_RSS_A_VERT_INT_OFST_
{
		volatile struct	/* 0x1A0035DC */
		{
				FIELD  RSS_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_VERT_INT_OFST;	/* CAM_UNI_RSS_A_VERT_INT_OFST */

typedef volatile union _CAM_UNI_REG_RSS_A_VERT_SUB_OFST_
{
		volatile struct	/* 0x1A0035E0 */
		{
				FIELD  RSS_VERT_SUB_OFST                     : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_A_VERT_SUB_OFST;	/* CAM_UNI_RSS_A_VERT_SUB_OFST */

typedef volatile union _CAM_UNI_REG_B_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1A003600 */
		{
				FIELD  EISO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_SOFT_RST_STAT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_SOFT_RST_STAT                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_SOFT_RST_STAT                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_SOFT_RSTSTAT;	/* CAM_UNI_B_DMA_SOFT_RSTSTAT */

typedef volatile union _CAM_UNI_REG_B_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x1A003604 */
		{
				FIELD  EISO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_V_FLIP_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_V_FLIP_EN                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_V_FLIP_EN                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_VERTICAL_FLIP_EN;	/* CAM_UNI_B_VERTICAL_FLIP_EN */

typedef volatile union _CAM_UNI_REG_B_DMA_SOFT_RESET_
{
		volatile struct	/* 0x1A003608 */
		{
				FIELD  EISO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_SOFT_RST                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_SOFT_RST                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_SOFT_RST                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_SOFT_RESET;	/* CAM_UNI_B_DMA_SOFT_RESET */

typedef volatile union _CAM_UNI_REG_B_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1A00360C */
		{
				FIELD  EISO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_LAST_ULTRA_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_LAST_ULTRA_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_LAST_ULTRA_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_LAST_ULTRA_EN;	/* CAM_UNI_B_LAST_ULTRA_EN */

typedef volatile union _CAM_UNI_REG_B_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1A003610 */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  GCLAST_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_SPECIAL_FUN_EN;	/* CAM_UNI_B_SPECIAL_FUN_EN */

typedef volatile union _CAM_UNI_REG_B_SPECIAL_FUN2_EN_
{
		volatile struct	/* 0x1A003614 */
		{
				FIELD  STAGER_SENSOR_DMA_DLNO                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  STAGER_SENSOR_DMA_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_SPECIAL_FUN2_EN;	/* CAM_UNI_B_SPECIAL_FUN2_EN */

typedef volatile union _CAM_UNI_REG_B_EISO_BASE_ADDR_
{
		volatile struct	/* 0x1A003620 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_BASE_ADDR;	/* CAM_UNI_B_EISO_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_EISO_OFST_ADDR_
{
		volatile struct	/* 0x1A003628 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_OFST_ADDR;	/* CAM_UNI_B_EISO_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_B_EISO_DRS_
{
		volatile struct	/* 0x1A00362C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_DRS;	/* CAM_UNI_B_EISO_DRS */

typedef volatile union _CAM_UNI_REG_B_EISO_XSIZE_
{
		volatile struct	/* 0x1A003630 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_XSIZE;	/* CAM_UNI_B_EISO_XSIZE */

typedef volatile union _CAM_UNI_REG_B_EISO_YSIZE_
{
		volatile struct	/* 0x1A003634 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_YSIZE;	/* CAM_UNI_B_EISO_YSIZE */

typedef volatile union _CAM_UNI_REG_B_EISO_STRIDE_
{
		volatile struct	/* 0x1A003638 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_STRIDE;	/* CAM_UNI_B_EISO_STRIDE */

typedef volatile union _CAM_UNI_REG_B_EISO_CON_
{
		volatile struct	/* 0x1A00363C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_CON;	/* CAM_UNI_B_EISO_CON */

typedef volatile union _CAM_UNI_REG_B_EISO_CON2_
{
		volatile struct	/* 0x1A003640 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_CON2;	/* CAM_UNI_B_EISO_CON2 */

typedef volatile union _CAM_UNI_REG_B_EISO_CON3_
{
		volatile struct	/* 0x1A003644 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_CON3;	/* CAM_UNI_B_EISO_CON3 */

typedef volatile union _CAM_UNI_REG_B_EISO_CON4_
{
		volatile struct	/* 0x1A00364C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_CON4;	/* CAM_UNI_B_EISO_CON4 */

typedef volatile union _CAM_UNI_REG_B_FLKO_BASE_ADDR_
{
		volatile struct	/* 0x1A003650 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_BASE_ADDR;	/* CAM_UNI_B_FLKO_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_FLKO_OFST_ADDR_
{
		volatile struct	/* 0x1A003658 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_OFST_ADDR;	/* CAM_UNI_B_FLKO_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_B_FLKO_DRS_
{
		volatile struct	/* 0x1A00365C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_DRS;	/* CAM_UNI_B_FLKO_DRS */

typedef volatile union _CAM_UNI_REG_B_FLKO_XSIZE_
{
		volatile struct	/* 0x1A003660 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_XSIZE;	/* CAM_UNI_B_FLKO_XSIZE */

typedef volatile union _CAM_UNI_REG_B_FLKO_YSIZE_
{
		volatile struct	/* 0x1A003664 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_YSIZE;	/* CAM_UNI_B_FLKO_YSIZE */

typedef volatile union _CAM_UNI_REG_B_FLKO_STRIDE_
{
		volatile struct	/* 0x1A003668 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  7;		/* 17..23, 0x00FE0000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_STRIDE;	/* CAM_UNI_B_FLKO_STRIDE */

typedef volatile union _CAM_UNI_REG_B_FLKO_CON_
{
		volatile struct	/* 0x1A00366C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_CON;	/* CAM_UNI_B_FLKO_CON */

typedef volatile union _CAM_UNI_REG_B_FLKO_CON2_
{
		volatile struct	/* 0x1A003670 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_CON2;	/* CAM_UNI_B_FLKO_CON2 */

typedef volatile union _CAM_UNI_REG_B_FLKO_CON3_
{
		volatile struct	/* 0x1A003674 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_CON3;	/* CAM_UNI_B_FLKO_CON3 */

typedef volatile union _CAM_UNI_REG_B_FLKO_CON4_
{
		volatile struct	/* 0x1A00367C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_CON4;	/* CAM_UNI_B_FLKO_CON4 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_BASE_ADDR_
{
		volatile struct	/* 0x1A003680 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_BASE_ADDR;	/* CAM_UNI_B_RSSO_A_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_OFST_ADDR_
{
		volatile struct	/* 0x1A003688 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_OFST_ADDR;	/* CAM_UNI_B_RSSO_A_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_DRS_
{
		volatile struct	/* 0x1A00368C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_DRS;	/* CAM_UNI_B_RSSO_A_DRS */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_XSIZE_
{
		volatile struct	/* 0x1A003690 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_XSIZE;	/* CAM_UNI_B_RSSO_A_XSIZE */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_YSIZE_
{
		volatile struct	/* 0x1A003694 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_YSIZE;	/* CAM_UNI_B_RSSO_A_YSIZE */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_STRIDE_
{
		volatile struct	/* 0x1A003698 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_STRIDE;	/* CAM_UNI_B_RSSO_A_STRIDE */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_CON_
{
		volatile struct	/* 0x1A00369C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_CON;	/* CAM_UNI_B_RSSO_A_CON */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_CON2_
{
		volatile struct	/* 0x1A0036A0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_CON2;	/* CAM_UNI_B_RSSO_A_CON2 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_CON3_
{
		volatile struct	/* 0x1A0036A4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_CON3;	/* CAM_UNI_B_RSSO_A_CON3 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_CON4_
{
		volatile struct	/* 0x1A0036AC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_CON4;	/* CAM_UNI_B_RSSO_A_CON4 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_BASE_ADDR_
{
		volatile struct	/* 0x1A0036B0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_BASE_ADDR;	/* CAM_UNI_B_RSSO_B_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_OFST_ADDR_
{
		volatile struct	/* 0x1A0036B8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_OFST_ADDR;	/* CAM_UNI_B_RSSO_B_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_DRS_
{
		volatile struct	/* 0x1A0036BC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_DRS;	/* CAM_UNI_B_RSSO_B_DRS */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_XSIZE_
{
		volatile struct	/* 0x1A0036C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_XSIZE;	/* CAM_UNI_B_RSSO_B_XSIZE */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_YSIZE_
{
		volatile struct	/* 0x1A0036C4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_YSIZE;	/* CAM_UNI_B_RSSO_B_YSIZE */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_STRIDE_
{
		volatile struct	/* 0x1A0036C8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_STRIDE;	/* CAM_UNI_B_RSSO_B_STRIDE */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_CON_
{
		volatile struct	/* 0x1A0036CC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_CON;	/* CAM_UNI_B_RSSO_B_CON */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_CON2_
{
		volatile struct	/* 0x1A0036D0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_CON2;	/* CAM_UNI_B_RSSO_B_CON2 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_CON3_
{
		volatile struct	/* 0x1A0036D4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_CON3;	/* CAM_UNI_B_RSSO_B_CON3 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_CON4_
{
		volatile struct	/* 0x1A0036DC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_CON4;	/* CAM_UNI_B_RSSO_B_CON4 */

typedef volatile union _CAM_UNI_REG_B_RAWI_BASE_ADDR_
{
		volatile struct	/* 0x1A003740 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_BASE_ADDR;	/* CAM_UNI_B_RAWI_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_RAWI_OFST_ADDR_
{
		volatile struct	/* 0x1A003748 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_OFST_ADDR;	/* CAM_UNI_B_RAWI_OFST_ADDR */

typedef volatile union _CAM_UNI_REG_B_RAWI_DRS_
{
		volatile struct	/* 0x1A00374C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_DRS;	/* CAM_UNI_B_RAWI_DRS */

typedef volatile union _CAM_UNI_REG_B_RAWI_XSIZE_
{
		volatile struct	/* 0x1A003750 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_XSIZE;	/* CAM_UNI_B_RAWI_XSIZE */

typedef volatile union _CAM_UNI_REG_B_RAWI_YSIZE_
{
		volatile struct	/* 0x1A003754 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_YSIZE;	/* CAM_UNI_B_RAWI_YSIZE */

typedef volatile union _CAM_UNI_REG_B_RAWI_STRIDE_
{
		volatile struct	/* 0x1A003758 */
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
}CAM_UNI_REG_B_RAWI_STRIDE;	/* CAM_UNI_B_RAWI_STRIDE */

typedef volatile union _CAM_UNI_REG_B_RAWI_CON_
{
		volatile struct	/* 0x1A00375C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_CON;	/* CAM_UNI_B_RAWI_CON */

typedef volatile union _CAM_UNI_REG_B_RAWI_CON2_
{
		volatile struct	/* 0x1A003760 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_CON2;	/* CAM_UNI_B_RAWI_CON2 */

typedef volatile union _CAM_UNI_REG_B_RAWI_CON3_
{
		volatile struct	/* 0x1A003764 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_CON3;	/* CAM_UNI_B_RAWI_CON3 */

typedef volatile union _CAM_UNI_REG_B_RAWI_CON4_
{
		volatile struct	/* 0x1A00376C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_CON4;	/* CAM_UNI_B_RAWI_CON4 */

typedef volatile union _CAM_UNI_REG_B_DMA_ERR_CTRL_
{
		volatile struct	/* 0x1A003770 */
		{
				FIELD  EISO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FLKO_ERR                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSSO_A_ERR                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RSSO_B_ERR                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  RAWI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_ERR_CTRL;	/* CAM_UNI_B_DMA_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_B_EISO_ERR_STAT_
{
		volatile struct	/* 0x1A003774 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_ERR_STAT;	/* CAM_UNI_B_EISO_ERR_STAT */

typedef volatile union _CAM_UNI_REG_B_FLKO_ERR_STAT_
{
		volatile struct	/* 0x1A003778 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_ERR_STAT;	/* CAM_UNI_B_FLKO_ERR_STAT */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_ERR_STAT_
{
		volatile struct	/* 0x1A00377C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_ERR_STAT;	/* CAM_UNI_B_RSSO_A_ERR_STAT */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_ERR_STAT_
{
		volatile struct	/* 0x1A003780 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_ERR_STAT;	/* CAM_UNI_B_RSSO_B_ERR_STAT */

typedef volatile union _CAM_UNI_REG_B_RAWI_ERR_STAT_
{
		volatile struct	/* 0x1A003784 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RAWI_ERR_STAT;	/* CAM_UNI_B_RAWI_ERR_STAT */

typedef volatile union _CAM_UNI_REG_B_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1A003788 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_DEBUG_ADDR;	/* CAM_UNI_B_DMA_DEBUG_ADDR */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV1_
{
		volatile struct	/* 0x1A00378C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV1;	/* CAM_UNI_B_DMA_RSV1 */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV2_
{
		volatile struct	/* 0x1A003790 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV2;	/* CAM_UNI_B_DMA_RSV2 */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV3_
{
		volatile struct	/* 0x1A003794 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV3;	/* CAM_UNI_B_DMA_RSV3 */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV4_
{
		volatile struct	/* 0x1A003798 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV4;	/* CAM_UNI_B_DMA_RSV4 */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV5_
{
		volatile struct	/* 0x1A00379C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV5;	/* CAM_UNI_B_DMA_RSV5 */

typedef volatile union _CAM_UNI_REG_B_DMA_RSV6_
{
		volatile struct	/* 0x1A0037A0 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_RSV6;	/* CAM_UNI_B_DMA_RSV6 */

typedef volatile union _CAM_UNI_REG_B_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x1A0037A4 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_DEBUG_SEL;	/* CAM_UNI_B_DMA_DEBUG_SEL */

typedef volatile union _CAM_UNI_REG_B_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1A0037A8 */
		{
				FIELD  BW_SELF_TEST_EN_EISO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_FLKO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_RSSO_A                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_RSSO_B                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_BW_SELF_TEST;	/* CAM_UNI_B_DMA_BW_SELF_TEST */

typedef volatile union _CAM_UNI_REG_B_DMA_FRAME_HEADER_EN_
{
		volatile struct	/* 0x1A0037C0 */
		{
				FIELD  FRAME_HEADER_EN_EISO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FRAME_HEADER_EN_FLKO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FRAME_HEADER_EN_RSSO_A                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FRAME_HEADER_EN_RSSO_B                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_DMA_FRAME_HEADER_EN;	/* CAM_UNI_B_DMA_FRAME_HEADER_EN */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0037C4 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_BASE_ADDR;	/* CAM_UNI_B_EISO_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0037C8 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_BASE_ADDR;	/* CAM_UNI_B_FLKO_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0037CC */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_BASE_ADDR;	/* CAM_UNI_B_RSSO_A_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A0037D0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_BASE_ADDR;	/* CAM_UNI_B_RSSO_B_FH_BASE_ADDR */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_2_
{
		volatile struct	/* 0x1A0037E0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_2;	/* CAM_UNI_B_EISO_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_3_
{
		volatile struct	/* 0x1A0037E4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_3;	/* CAM_UNI_B_EISO_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_4_
{
		volatile struct	/* 0x1A0037E8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_4;	/* CAM_UNI_B_EISO_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_5_
{
		volatile struct	/* 0x1A0037EC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_5;	/* CAM_UNI_B_EISO_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_6_
{
		volatile struct	/* 0x1A0037F0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_6;	/* CAM_UNI_B_EISO_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_7_
{
		volatile struct	/* 0x1A0037F4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_7;	/* CAM_UNI_B_EISO_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_8_
{
		volatile struct	/* 0x1A0037F8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_8;	/* CAM_UNI_B_EISO_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_9_
{
		volatile struct	/* 0x1A0037FC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_9;	/* CAM_UNI_B_EISO_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_10_
{
		volatile struct	/* 0x1A003800 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_10;	/* CAM_UNI_B_EISO_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_11_
{
		volatile struct	/* 0x1A003804 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_11;	/* CAM_UNI_B_EISO_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_12_
{
		volatile struct	/* 0x1A003808 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_12;	/* CAM_UNI_B_EISO_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_13_
{
		volatile struct	/* 0x1A00380C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_13;	/* CAM_UNI_B_EISO_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_14_
{
		volatile struct	/* 0x1A003810 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_14;	/* CAM_UNI_B_EISO_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_15_
{
		volatile struct	/* 0x1A003814 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_15;	/* CAM_UNI_B_EISO_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_B_EISO_FH_SPARE_16_
{
		volatile struct	/* 0x1A003818 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_EISO_FH_SPARE_16;	/* CAM_UNI_B_EISO_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_2_
{
		volatile struct	/* 0x1A003820 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_2;	/* CAM_UNI_B_FLKO_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_3_
{
		volatile struct	/* 0x1A003824 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_3;	/* CAM_UNI_B_FLKO_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_4_
{
		volatile struct	/* 0x1A003828 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_4;	/* CAM_UNI_B_FLKO_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_5_
{
		volatile struct	/* 0x1A00382C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_5;	/* CAM_UNI_B_FLKO_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_6_
{
		volatile struct	/* 0x1A003830 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_6;	/* CAM_UNI_B_FLKO_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_7_
{
		volatile struct	/* 0x1A003834 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_7;	/* CAM_UNI_B_FLKO_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_8_
{
		volatile struct	/* 0x1A003838 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_8;	/* CAM_UNI_B_FLKO_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_9_
{
		volatile struct	/* 0x1A00383C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_9;	/* CAM_UNI_B_FLKO_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_10_
{
		volatile struct	/* 0x1A003840 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_10;	/* CAM_UNI_B_FLKO_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_11_
{
		volatile struct	/* 0x1A003844 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_11;	/* CAM_UNI_B_FLKO_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_12_
{
		volatile struct	/* 0x1A003848 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_12;	/* CAM_UNI_B_FLKO_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_13_
{
		volatile struct	/* 0x1A00384C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_13;	/* CAM_UNI_B_FLKO_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_14_
{
		volatile struct	/* 0x1A003850 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_14;	/* CAM_UNI_B_FLKO_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_15_
{
		volatile struct	/* 0x1A003854 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_15;	/* CAM_UNI_B_FLKO_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_B_FLKO_FH_SPARE_16_
{
		volatile struct	/* 0x1A003858 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_FLKO_FH_SPARE_16;	/* CAM_UNI_B_FLKO_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_2_
{
		volatile struct	/* 0x1A003860 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_2;	/* CAM_UNI_B_RSSO_A_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_3_
{
		volatile struct	/* 0x1A003864 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_3;	/* CAM_UNI_B_RSSO_A_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_4_
{
		volatile struct	/* 0x1A003868 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_4;	/* CAM_UNI_B_RSSO_A_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_5_
{
		volatile struct	/* 0x1A00386C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_5;	/* CAM_UNI_B_RSSO_A_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_6_
{
		volatile struct	/* 0x1A003870 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_6;	/* CAM_UNI_B_RSSO_A_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_7_
{
		volatile struct	/* 0x1A003874 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_7;	/* CAM_UNI_B_RSSO_A_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_8_
{
		volatile struct	/* 0x1A003878 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_8;	/* CAM_UNI_B_RSSO_A_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_9_
{
		volatile struct	/* 0x1A00387C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_9;	/* CAM_UNI_B_RSSO_A_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_10_
{
		volatile struct	/* 0x1A003880 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_10;	/* CAM_UNI_B_RSSO_A_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_11_
{
		volatile struct	/* 0x1A003884 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_11;	/* CAM_UNI_B_RSSO_A_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_12_
{
		volatile struct	/* 0x1A003888 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_12;	/* CAM_UNI_B_RSSO_A_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_13_
{
		volatile struct	/* 0x1A00388C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_13;	/* CAM_UNI_B_RSSO_A_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_14_
{
		volatile struct	/* 0x1A003890 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_14;	/* CAM_UNI_B_RSSO_A_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_15_
{
		volatile struct	/* 0x1A003894 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_15;	/* CAM_UNI_B_RSSO_A_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_B_RSSO_A_FH_SPARE_16_
{
		volatile struct	/* 0x1A003898 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_A_FH_SPARE_16;	/* CAM_UNI_B_RSSO_A_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_2_
{
		volatile struct	/* 0x1A0038A0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_2;	/* CAM_UNI_B_RSSO_B_FH_SPARE_2 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_3_
{
		volatile struct	/* 0x1A0038A4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_3;	/* CAM_UNI_B_RSSO_B_FH_SPARE_3 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_4_
{
		volatile struct	/* 0x1A0038A8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_4;	/* CAM_UNI_B_RSSO_B_FH_SPARE_4 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_5_
{
		volatile struct	/* 0x1A0038AC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_5;	/* CAM_UNI_B_RSSO_B_FH_SPARE_5 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_6_
{
		volatile struct	/* 0x1A0038B0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_6;	/* CAM_UNI_B_RSSO_B_FH_SPARE_6 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_7_
{
		volatile struct	/* 0x1A0038B4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_7;	/* CAM_UNI_B_RSSO_B_FH_SPARE_7 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_8_
{
		volatile struct	/* 0x1A0038B8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_8;	/* CAM_UNI_B_RSSO_B_FH_SPARE_8 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_9_
{
		volatile struct	/* 0x1A0038BC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_9;	/* CAM_UNI_B_RSSO_B_FH_SPARE_9 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_10_
{
		volatile struct	/* 0x1A0038C0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_10;	/* CAM_UNI_B_RSSO_B_FH_SPARE_10 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_11_
{
		volatile struct	/* 0x1A0038C4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_11;	/* CAM_UNI_B_RSSO_B_FH_SPARE_11 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_12_
{
		volatile struct	/* 0x1A0038C8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_12;	/* CAM_UNI_B_RSSO_B_FH_SPARE_12 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_13_
{
		volatile struct	/* 0x1A0038CC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_13;	/* CAM_UNI_B_RSSO_B_FH_SPARE_13 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_14_
{
		volatile struct	/* 0x1A0038D0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_14;	/* CAM_UNI_B_RSSO_B_FH_SPARE_14 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_15_
{
		volatile struct	/* 0x1A0038D4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_15;	/* CAM_UNI_B_RSSO_B_FH_SPARE_15 */

typedef volatile union _CAM_UNI_REG_B_RSSO_B_FH_SPARE_16_
{
		volatile struct	/* 0x1A0038D8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_B_RSSO_B_FH_SPARE_16;	/* CAM_UNI_B_RSSO_B_FH_SPARE_16 */

typedef volatile union _CAM_UNI_REG_UNP2_B_OFST_
{
		volatile struct	/* 0x1A003900 */
		{
				FIELD  UNP2_OFST_STB                         :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  UNP2_OFST_EDB                         :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_UNP2_B_OFST;	/* CAM_UNI_UNP2_B_OFST */

typedef volatile union _CAM_UNI_REG_QBN3_B_MODE_
{
		volatile struct	/* 0x1A003910 */
		{
				FIELD  QBN_ACC                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  QBN_ACC_MODE                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_QBN3_B_MODE;	/* CAM_UNI_QBN3_B_MODE */

typedef volatile union _CAM_UNI_REG_SGG3_B_PGN_
{
		volatile struct	/* 0x1A003920 */
		{
				FIELD  SGG_GAIN                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_B_PGN;	/* CAM_UNI_SGG3_B_PGN */

typedef volatile union _CAM_UNI_REG_SGG3_B_GMRC_1_
{
		volatile struct	/* 0x1A003924 */
		{
				FIELD  SGG_GMR_1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG_GMR_4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_B_GMRC_1;	/* CAM_UNI_SGG3_B_GMRC_1 */

typedef volatile union _CAM_UNI_REG_SGG3_B_GMRC_2_
{
		volatile struct	/* 0x1A003928 */
		{
				FIELD  SGG_GMR_5                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_6                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_7                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_SGG3_B_GMRC_2;	/* CAM_UNI_SGG3_B_GMRC_2 */

typedef volatile union _CAM_UNI_REG_FLK_B_CON_
{
		volatile struct	/* 0x1A003930 */
		{
				FIELD  RESERVED                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_B_CON;	/* CAM_UNI_FLK_B_CON */

typedef volatile union _CAM_UNI_REG_FLK_B_OFST_
{
		volatile struct	/* 0x1A003934 */
		{
				FIELD  FLK_OFST_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_OFST_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_B_OFST;	/* CAM_UNI_FLK_B_OFST */

typedef volatile union _CAM_UNI_REG_FLK_B_SIZE_
{
		volatile struct	/* 0x1A003938 */
		{
				FIELD  FLK_SIZE_X                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FLK_SIZE_Y                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_B_SIZE;	/* CAM_UNI_FLK_B_SIZE */

typedef volatile union _CAM_UNI_REG_FLK_B_NUM_
{
		volatile struct	/* 0x1A00393C */
		{
				FIELD  FLK_NUM_X                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLK_NUM_Y                             :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_FLK_B_NUM;	/* CAM_UNI_FLK_B_NUM */

typedef volatile union _CAM_UNI_REG_EIS_B_PREP_ME_CTRL1_
{
		volatile struct	/* 0x1A003950 */
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
}CAM_UNI_REG_EIS_B_PREP_ME_CTRL1;	/* CAM_UNI_EIS_B_PREP_ME_CTRL1 */

typedef volatile union _CAM_UNI_REG_EIS_B_PREP_ME_CTRL2_
{
		volatile struct	/* 0x1A003954 */
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
}CAM_UNI_REG_EIS_B_PREP_ME_CTRL2;	/* CAM_UNI_EIS_B_PREP_ME_CTRL2 */

typedef volatile union _CAM_UNI_REG_EIS_B_LMV_TH_
{
		volatile struct	/* 0x1A003958 */
		{
				FIELD  LMV_TH_Y_SURROUND                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LMV_TH_Y_CENTER                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LMV_TH_X_SOURROUND                    :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LMV_TH_X_CENTER                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_LMV_TH;	/* CAM_UNI_EIS_B_LMV_TH */

typedef volatile union _CAM_UNI_REG_EIS_B_FL_OFFSET_
{
		volatile struct	/* 0x1A00395C */
		{
				FIELD  EIS_WIN_VOFST                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HOFST                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_FL_OFFSET;	/* CAM_UNI_EIS_B_FL_OFFSET */

typedef volatile union _CAM_UNI_REG_EIS_B_MB_OFFSET_
{
		volatile struct	/* 0x1A003960 */
		{
				FIELD  EIS_RP_VOFST                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_RP_HOFST                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_MB_OFFSET;	/* CAM_UNI_EIS_B_MB_OFFSET */

typedef volatile union _CAM_UNI_REG_EIS_B_MB_INTERVAL_
{
		volatile struct	/* 0x1A003964 */
		{
				FIELD  EIS_WIN_VSIZE                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  EIS_WIN_HSIZE                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_MB_INTERVAL;	/* CAM_UNI_EIS_B_MB_INTERVAL */

typedef volatile union _CAM_UNI_REG_EIS_B_GMV_
{
		volatile struct	/* 0x1A003968 */
		{
				FIELD  GMV_Y                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  GMV_X                                 : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_GMV;	/* CAM_UNI_EIS_B_GMV */

typedef volatile union _CAM_UNI_REG_EIS_B_ERR_CTRL_
{
		volatile struct	/* 0x1A00396C */
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
}CAM_UNI_REG_EIS_B_ERR_CTRL;	/* CAM_UNI_EIS_B_ERR_CTRL */

typedef volatile union _CAM_UNI_REG_EIS_B_IMAGE_CTRL_
{
		volatile struct	/* 0x1A003970 */
		{
				FIELD  HEIGHT                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WIDTH                                 : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PIPE_MODE                             :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_EIS_B_IMAGE_CTRL;	/* CAM_UNI_EIS_B_IMAGE_CTRL */

typedef volatile union _CAM_UNI_REG_HDS_B_MODE_
{
		volatile struct	/* 0x1A0039A0 */
		{
				FIELD  HDS_DS                                :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_HDS_B_MODE;	/* CAM_UNI_HDS_B_MODE */

typedef volatile union _CAM_UNI_REG_RSS_B_CONTROL_
{
		volatile struct	/* 0x1A0039C0 */
		{
				FIELD  RSS_HORI_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RSS_VERT_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RSS_OUTPUT_WAIT_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RSS_VERT_FIRST                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 11;		/*  5..15, 0x0000FFE0 */
				FIELD  RSS_HORI_TBL_SEL                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  RSS_VERT_TBL_SEL                      :  5;		/* 21..25, 0x03E00000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_CONTROL;	/* CAM_UNI_RSS_B_CONTROL */

typedef volatile union _CAM_UNI_REG_RSS_B_IN_IMG_
{
		volatile struct	/* 0x1A0039C4 */
		{
				FIELD  RSS_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RSS_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_IN_IMG;	/* CAM_UNI_RSS_B_IN_IMG */

typedef volatile union _CAM_UNI_REG_RSS_B_OUT_IMG_
{
		volatile struct	/* 0x1A0039C8 */
		{
				FIELD  RSS_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RSS_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_OUT_IMG;	/* CAM_UNI_RSS_B_OUT_IMG */

typedef volatile union _CAM_UNI_REG_RSS_B_HORI_STEP_
{
		volatile struct	/* 0x1A0039CC */
		{
				FIELD  RSS_HORI_STEP                         : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_HORI_STEP;	/* CAM_UNI_RSS_B_HORI_STEP */

typedef volatile union _CAM_UNI_REG_RSS_B_VERT_STEP_
{
		volatile struct	/* 0x1A0039D0 */
		{
				FIELD  RSS_VERT_STEP                         : 21;		/*  0..20, 0x001FFFFF */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_VERT_STEP;	/* CAM_UNI_RSS_B_VERT_STEP */

typedef volatile union _CAM_UNI_REG_RSS_B_HORI_INT_OFST_
{
		volatile struct	/* 0x1A0039D4 */
		{
				FIELD  RSS_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_HORI_INT_OFST;	/* CAM_UNI_RSS_B_HORI_INT_OFST */

typedef volatile union _CAM_UNI_REG_RSS_B_HORI_SUB_OFST_
{
		volatile struct	/* 0x1A0039D8 */
		{
				FIELD  RSS_HORI_SUB_OFST                     : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_HORI_SUB_OFST;	/* CAM_UNI_RSS_B_HORI_SUB_OFST */

typedef volatile union _CAM_UNI_REG_RSS_B_VERT_INT_OFST_
{
		volatile struct	/* 0x1A0039DC */
		{
				FIELD  RSS_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_VERT_INT_OFST;	/* CAM_UNI_RSS_B_VERT_INT_OFST */

typedef volatile union _CAM_UNI_REG_RSS_B_VERT_SUB_OFST_
{
		volatile struct	/* 0x1A0039E0 */
		{
				FIELD  RSS_VERT_SUB_OFST                     : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_UNI_REG_RSS_B_VERT_SUB_OFST;	/* CAM_UNI_RSS_B_VERT_SUB_OFST */

typedef volatile union _CAM_REG_CTL_START_
{
		volatile struct	/* 0x1A004000 */
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
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  CQ_THR12_START                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_START;	/* CAM_CTL_START, CAM_A_CTL_START*/

typedef volatile union _CAM_REG_CTL_EN_
{
		volatile struct	/* 0x1A004004 */
		{
				FIELD  TG_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_EN                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGM_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RMG_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_EN                                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  OBC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BNR_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSC_EN                                :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CAC_EN                                :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RPG_EN                                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZ_EN                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RMX_EN                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PAKG_EN                               :  1;		/* 12..12, 0x00001000 */
				FIELD  BMX_EN                                :  1;		/* 13..13, 0x00002000 */
				FIELD  CPG_EN                                :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_EN                                :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_EN                                 :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_EN                               :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_EN                                 :  1;		/* 19..19, 0x00080000 */
				FIELD  QBIN1_EN                              :  1;		/* 20..20, 0x00100000 */
				FIELD  LCS_EN                                :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN2_EN                              :  1;		/* 22..22, 0x00400000 */
				FIELD  RCP_EN                                :  1;		/* 23..23, 0x00800000 */
				FIELD  RCP3_EN                               :  1;		/* 24..24, 0x01000000 */
				FIELD  PMX_EN                                :  1;		/* 25..25, 0x02000000 */
				FIELD  PKP_EN                                :  1;		/* 26..26, 0x04000000 */
				FIELD  BIN_EN                                :  1;		/* 27..27, 0x08000000 */
				FIELD  DBS_EN                                :  1;		/* 28..28, 0x10000000 */
				FIELD  DBN_EN                                :  1;		/* 29..29, 0x20000000 */
				FIELD  PBN_EN                                :  1;		/* 30..30, 0x40000000 */
				FIELD  UFEG_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_EN;	/* CAM_CTL_EN, CAM_A_CTL_EN*/

typedef volatile union _CAM_REG_CTL_DMA_EN_
{
		volatile struct	/* 0x1A004008 */
		{
				FIELD  IMGO_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_EN                                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_EN                               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_EN                               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_EN                                :  1;		/* 10..10, 0x00000400 */
				FIELD  LSC3I_EN                              :  1;		/* 11..11, 0x00000800 */
				FIELD  PDI_EN                                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  4;		/* 13..16, 0x0001E000 */
				FIELD  PSB_EN                                :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG2_EN                               :  1;		/* 18..18, 0x00040000 */
				FIELD  PDE_EN                                :  1;		/* 19..19, 0x00080000 */
				FIELD  GSE_EN                                :  1;		/* 20..20, 0x00100000 */
				FIELD  PCP_EN                                :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN4_EN                              :  1;		/* 22..22, 0x00400000 */
				FIELD  PS_EN                                 :  1;		/* 23..23, 0x00800000 */
				FIELD  HLR_EN                                :  1;		/* 24..24, 0x01000000 */
				FIELD  AMX_EN                                :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2F_EN                               :  1;		/* 26..26, 0x04000000 */
				FIELD  VBN_EN                                :  1;		/* 27..27, 0x08000000 */
				FIELD  SL2J_EN                               :  1;		/* 28..28, 0x10000000 */
				FIELD  STM_EN                                :  1;		/* 29..29, 0x20000000 */
				FIELD  SCM_EN                                :  1;		/* 30..30, 0x40000000 */
				FIELD  SGG5_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_EN;	/* CAM_CTL_DMA_EN, CAM_A_CTL_DMA_EN*/

typedef volatile union _CAM_REG_CTL_FMT_SEL_
{
		volatile struct	/* 0x1A00400C */
		{
				FIELD  PIX_ID                                :  2;		/*  0.. 1, 0x00000003 */
				FIELD  RRZO_FMT                              :  2;		/*  2.. 3, 0x0000000C */
				FIELD  IMGO_FMT                              :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PIX_BUS_DMXO                          :  2;		/* 10..11, 0x00000C00 */
				FIELD  TG_FMT                                :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TG_SWAP                               :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DMX_ID                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  PIX_BUS_DMXI                          :  2;		/* 24..25, 0x03000000 */
				FIELD  PIX_BUS_BMXO                          :  2;		/* 26..27, 0x0C000000 */
				FIELD  PIX_BUS_RMXO                          :  2;		/* 28..29, 0x30000000 */
				FIELD  RRZO_FG_MODE                          :  1;		/* 30..30, 0x40000000 */
				FIELD  LP_MODE                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_FMT_SEL;	/* CAM_CTL_FMT_SEL, CAM_A_CTL_FMT_SEL*/

typedef volatile union _CAM_REG_CTL_SEL_
{
		volatile struct	/* 0x1A004010 */
		{
				FIELD  DMX_SEL                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  HDS1_SEL                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  STM_SEL                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AA_SEL                                :  1;		/* 10..10, 0x00000400 */
				FIELD  QBIN4_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  SGG_SEL                               :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  1;		/* 13..13, 0x00002000 */
				FIELD  LCS_SEL                               :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  IMG_SEL                               :  2;		/* 16..17, 0x00030000 */
				FIELD  UFE_SEL                               :  2;		/* 18..19, 0x000C0000 */
				FIELD  FLK1_SEL                              :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  PMX_SEL                               :  2;		/* 22..23, 0x00C00000 */
				FIELD  RCP3_SEL                              :  2;		/* 24..25, 0x03000000 */
				FIELD  UFEG_SEL                              :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  RCP_SEL                               :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  1;		/* 29..29, 0x20000000 */
				FIELD  AAO_SEL                               :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SEL;	/* CAM_CTL_SEL, CAM_A_CTL_SEL*/

typedef volatile union _CAM_REG_CTL_MISC_
{
		volatile struct	/* 0x1A004014 */
		{
				FIELD  DB_LOAD_HOLD                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  DB_EN                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  APB_CLK_GATE_BYPASS                   :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  AF_DB_LOAD_HOLD                       :  1;		/* 12..12, 0x00001000 */
				FIELD  AA_DB_LOAD_HOLD                       :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_MISC;	/* CAM_CTL_MISC, CAM_A_CTL_MISC*/

typedef volatile union _CAM_REG_CTL_RAW_INT_EN_
{
		volatile struct	/* 0x1A004020 */
		{
				FIELD  VS_INT_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_CODE_ERR_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_APB_ERR_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_VS_ERR_EN                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_DROP_FRAME_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_DROP_FRAME_EN                    :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS1_DON_EN                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF_INT_EN                            :  1;		/* 12..12, 0x00001000 */
				FIELD  SOF_WAIT_EN                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PDE_ERR_EN                            :  1;		/* 14..14, 0x00004000 */
				FIELD  AMX_ERR_EN                            :  1;		/* 15..15, 0x00008000 */
				FIELD  RMX_ERR_EN                            :  1;		/* 16..16, 0x00010000 */
				FIELD  BMX_ERR_EN                            :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_ERR_EN                           :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_ERR_EN                            :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_EN                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_EN                            :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_ERR_EN                            :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_ERR_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_ERR_EN                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSC_ERR_EN                            :  1;		/* 25..25, 0x02000000 */
				FIELD  CAC_ERR_EN                            :  1;		/* 26..26, 0x04000000 */
				FIELD  UFEO_ERR_EN                           :  1;		/* 27..27, 0x08000000 */
				FIELD  PDO_ERR_EN                            :  1;		/* 28..28, 0x10000000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 29..29, 0x20000000 */
				FIELD  SW_PASS1_DON_EN                       :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT_EN;	/* CAM_CTL_RAW_INT_EN, CAM_A_CTL_RAW_INT_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT_STATUS_
{
		volatile struct	/* 0x1A004024 */
		{
				FIELD  VS_INT_ST                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_ST                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_ST                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON_ST                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_ST                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_CODE_ERR_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_APB_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_VS_ERR_ST                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS1_DON_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF_INT_ST                            :  1;		/* 12..12, 0x00001000 */
				FIELD  SOF_WAIT_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PDE_ERR_ST                            :  1;		/* 14..14, 0x00004000 */
				FIELD  AMX_ERR_ST                            :  1;		/* 15..15, 0x00008000 */
				FIELD  RMX_ERR_ST                            :  1;		/* 16..16, 0x00010000 */
				FIELD  BMX_ERR_ST                            :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_ERR_ST                            :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_ERR_ST                            :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  BNR_ERR_ST                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 25..25, 0x02000000 */
				FIELD  CAC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  UFEO_ERR_ST                           :  1;		/* 27..27, 0x08000000 */
				FIELD  PDO_ERR_ST                            :  1;		/* 28..28, 0x10000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 29..29, 0x20000000 */
				FIELD  SW_PASS1_DON_ST                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT_STATUS;	/* CAM_CTL_RAW_INT_STATUS, CAM_A_CTL_RAW_INT_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT_STATUSX_
{
		volatile struct	/* 0x1A004028 */
		{
				FIELD  VS_INT_ST                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_ST                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_ST                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON_ST                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_ST                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_CODE_ERR_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_APB_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_VS_ERR_ST                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS1_DON_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF_INT_ST                            :  1;		/* 12..12, 0x00001000 */
				FIELD  SOF_WAIT_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PDE_ERR_ST                            :  1;		/* 14..14, 0x00004000 */
				FIELD  AMX_ERR_ST                            :  1;		/* 15..15, 0x00008000 */
				FIELD  RMX_ERR_ST                            :  1;		/* 16..16, 0x00010000 */
				FIELD  BMX_ERR_ST                            :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_ERR_ST                            :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_ERR_ST                            :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  BNR_ERR_ST                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 25..25, 0x02000000 */
				FIELD  CAC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  UFEO_ERR_ST                           :  1;		/* 27..27, 0x08000000 */
				FIELD  PDO_ERR_ST                            :  1;		/* 28..28, 0x10000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 29..29, 0x20000000 */
				FIELD  SW_PASS1_DON_ST                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT_STATUSX;	/* CAM_CTL_RAW_INT_STATUSX, CAM_A_CTL_RAW_INT_STATUSX*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_EN_
{
		volatile struct	/* 0x1A004030 */
		{
				FIELD  IMGO_DONE_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_DONE_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_DONE_EN                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_DONE_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCSO_DONE_EN                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AAO_DONE_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC3I_DONE_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BPCI_DONE_EN                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSCI_DONE_EN                          :  1;		/* 10..10, 0x00000400 */
				FIELD  CACI_DONE_EN                          :  1;		/* 11..11, 0x00000800 */
				FIELD  AF_TAR_DONE_EN                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PDO_DONE_EN                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PSO_DONE_EN                           :  1;		/* 14..14, 0x00004000 */
				FIELD  RSSO_DONE_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR0_DONE_EN                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR1_DONE_EN                       :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR2_DONE_EN                       :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_THR3_DONE_EN                       :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ_THR4_DONE_EN                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_THR5_DONE_EN                       :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ_THR6_DONE_EN                       :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ_THR7_DONE_EN                       :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ_THR8_DONE_EN                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ_THR9_DONE_EN                       :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_THR10_DONE_EN                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_THR11_DONE_EN                      :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_THR12_DONE_EN                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_EN;	/* CAM_CTL_RAW_INT2_EN, CAM_A_CTL_RAW_INT2_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_STATUS_
{
		volatile struct	/* 0x1A004034 */
		{
				FIELD  IMGO_DONE_ST                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_DONE_ST                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_DONE_ST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_DEONE_ST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCSO_DONE_ST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AAO_DONE_ST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC3I_DONE_ST                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BPCI_DONE_ST                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSCI_DONE_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  CACI_DONE_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  AF_TAR_DONE_ST                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PDO_DONE_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PSO_DONE_ST                           :  1;		/* 14..14, 0x00004000 */
				FIELD  RSSO_DONE_ST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR0_DONE_ST                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR1_DONE_ST                       :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR2_DONE_ST                       :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_THR3_DONE_ST                       :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ_THR4_DONE_ST                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_THR5_DONE_ST                       :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ_THR6_DONE_ST                       :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ_THR7_DONE_ST                       :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ_THR8_DONE_ST                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ_THR9_DONE_ST                       :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_THR10_DONE_ST                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_THR11_DONE_ST                      :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_THR12_DONE_ST                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_STATUS;	/* CAM_CTL_RAW_INT2_STATUS, CAM_A_CTL_RAW_INT2_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT2_STATUSX_
{
		volatile struct	/* 0x1A004038 */
		{
				FIELD  IMGO_DONE_ST                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_DONE_ST                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_DONE_ST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_DEONE_ST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCSO_DONE_ST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AAO_DONE_ST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC3I_DONE_ST                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BPCI_DONE_ST                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSCI_DONE_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  CACI_DONE_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  AF_TAR_DONE_ST                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PDO_DONE_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PSO_DONE_ST                           :  1;		/* 14..14, 0x00004000 */
				FIELD  RSSO_DONE_ST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR0_DONE_ST                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR1_DONE_ST                       :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR2_DONE_ST                       :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_THR3_DONE_ST                       :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ_THR4_DONE_ST                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_THR5_DONE_ST                       :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ_THR6_DONE_ST                       :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ_THR7_DONE_ST                       :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ_THR8_DONE_ST                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ_THR9_DONE_ST                       :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_THR10_DONE_ST                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_THR11_DONE_ST                      :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_THR12_DONE_ST                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT2_STATUSX;	/* CAM_CTL_RAW_INT2_STATUSX, CAM_A_CTL_RAW_INT2_STATUSX*/

typedef volatile union _CAM_REG_CTL_SW_CTL_
{
		volatile struct	/* 0x1A004040 */
		{
				FIELD  SW_RST_Trig                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SW_RST_ST                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  HW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SW_CTL;	/* CAM_CTL_SW_CTL, CAM_A_CTL_SW_CTL*/

typedef volatile union _CAM_REG_CTL_AB_DONE_SEL_
{
		volatile struct	/* 0x1A004044 */
		{
				FIELD  IMGO_A_DONE_SEL                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_A_DONE_SEL                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_A_DONE_SEL                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_A_DONE_SEL                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_A_DONE_SEL                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_A_DONE_SEL                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_A_DONE_SEL                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_A_DONE_SEL                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_A_DONE_SEL                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_A_DONE_SEL                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_A_DONE_SEL                        :  1;		/* 10..10, 0x00000400 */
				FIELD  TG_A_DONE_SEL                         :  1;		/* 11..11, 0x00000800 */
				FIELD  LSC3I_A_DONE_SEL                      :  1;		/* 12..12, 0x00001000 */
				FIELD  PDI_A_DONE_SEL                        :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  IMGO_B_DONE_SEL                       :  1;		/* 16..16, 0x00010000 */
				FIELD  UFEO_B_DONE_SEL                       :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_B_DONE_SEL                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_B_DONE_SEL                        :  1;		/* 19..19, 0x00080000 */
				FIELD  LCSO_B_DONE_SEL                       :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_B_DONE_SEL                        :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_B_DONE_SEL                        :  1;		/* 22..22, 0x00400000 */
				FIELD  BPCI_B_DONE_SEL                       :  1;		/* 23..23, 0x00800000 */
				FIELD  LSCI_B_DONE_SEL                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CACI_B_DONE_SEL                       :  1;		/* 25..25, 0x02000000 */
				FIELD  PDO_B_DONE_SEL                        :  1;		/* 26..26, 0x04000000 */
				FIELD  TG_B_DONE_SEL                         :  1;		/* 27..27, 0x08000000 */
				FIELD  LSC3I_B_DONE_SEL                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PDI_B_DONE_SEL                        :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_AB_DONE_SEL;	/* CAM_CTL_AB_DONE_SEL, CAM_A_CTL_AB_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_CD_DONE_SEL_
{
		volatile struct	/* 0x1A004048 */
		{
				FIELD  IMGO_C_DONE_SEL                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_C_DONE_SEL                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_C_DONE_SEL                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_C_DONE_SEL                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_C_DONE_SEL                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_C_DONE_SEL                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_C_DONE_SEL                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_C_DONE_SEL                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_C_DONE_SEL                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_C_DONE_SEL                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_C_DONE_SEL                        :  1;		/* 10..10, 0x00000400 */
				FIELD  TG_C_DONE_SEL                         :  1;		/* 11..11, 0x00000800 */
				FIELD  LSC3I_C_DONE_SEL                      :  1;		/* 12..12, 0x00001000 */
				FIELD  PDI_C_DONE_SEL                        :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  IMGO_D_DONE_SEL                       :  1;		/* 16..16, 0x00010000 */
				FIELD  UFEO_D_DONE_SEL                       :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_D_DONE_SEL                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_D_DONE_SEL                        :  1;		/* 19..19, 0x00080000 */
				FIELD  LCSO_D_DONE_SEL                       :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_D_DONE_SEL                        :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_D_DONE_SEL                        :  1;		/* 22..22, 0x00400000 */
				FIELD  BPCI_D_DONE_SEL                       :  1;		/* 23..23, 0x00800000 */
				FIELD  LSCI_D_DONE_SEL                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CACI_D_DONE_SEL                       :  1;		/* 25..25, 0x02000000 */
				FIELD  PDO_D_DONE_SEL                        :  1;		/* 26..26, 0x04000000 */
				FIELD  TG_D_DONE_SEL                         :  1;		/* 27..27, 0x08000000 */
				FIELD  LSC3I_D_DONE_SEL                      :  1;		/* 28..28, 0x10000000 */
				FIELD  PDI_D_DONE_SEL                        :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_CD_DONE_SEL;	/* CAM_CTL_CD_DONE_SEL, CAM_A_CTL_CD_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_UNI_DONE_SEL_
{
		volatile struct	/* 0x1A00404C */
		{
				FIELD  DONE_SEL_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RSSO_A_DONE_SEL                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_A_DONE_SEL                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_A_DONE_SEL                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 23;		/*  5..27, 0x0FFFFFE0 */
				FIELD  ADL_A_DONE_SEL                        :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_UNI_DONE_SEL;	/* CAM_CTL_UNI_DONE_SEL, CAM_A_CTL_UNI_DONE_SEL*/

typedef volatile union _CAM_REG_CTL_TWIN_STATUS_
{
		volatile struct	/* 0x1A004050 */
		{
                FIELD  TWIN_EN                               : 1;       /*  0.. 0, 0x00000001 */
                FIELD  TWIN_MODULE                           : 2;
                FIELD  SPARE0                                : 29;      /*  3..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TWIN_STATUS;    /* CAM_CTL_TWIN_STATUS */

typedef volatile union _CAM_REG_CTL_SPARE1_
{
		volatile struct	/* 0x1A004054 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE1;	/* CAM_CTL_SPARE1, CAM_A_CTL_SPARE1*/

typedef volatile union _CAM_REG_CTL_SPARE2_
{
		volatile struct	/* 0x1A004058 */
		{
				FIELD  SPARE2                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SPARE2;	/* CAM_CTL_SPARE2, CAM_A_CTL_SPARE2*/

typedef volatile union _CAM_REG_CTL_SW_PASS1_DONE_
{
		volatile struct	/* 0x1A00405C */
		{
				FIELD  DOWN_SAMPLE_PERIOD                    :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  DOWN_SAMPLE_EN                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_FIFO_FULL_DROP                   :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_FIFO_FULL_DROP                   :  1;		/* 10..10, 0x00000400 */
				FIELD  LCSO_FIFO_FULL_DROP                   :  1;		/* 11..11, 0x00000800 */
				FIELD  UFEO_FIFO_FULL_DROP                   :  1;		/* 12..12, 0x00001000 */
				FIELD  EISO_FIFO_FULL_DROP                   :  1;		/* 13..13, 0x00002000 */
				FIELD  RSSO_FIFO_FULL_DROP                   :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_SW_PASS1_DONE;	/* CAM_CTL_SW_PASS1_DONE, CAM_A_CTL_SW_PASS1_DONE*/

typedef volatile union _CAM_REG_CTL_FBC_RCNT_INC_
{
		volatile struct	/* 0x1A004060 */
		{
				FIELD  IMGO_RCNT_INC                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_RCNT_INC                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  LCSO_RCNT_INC                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AAO_RCNT_INC                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PDO_A_RCNT_INC                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_A_RCNT_INC                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  AFO_B_RCNT_INC                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AFO_C_RCNT_INC                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AFO_D_RCNT_INC                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  UFEO_RCNT_INC                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  EISO_RCNT_INC                         :  1;		/* 10..10, 0x00000400 */
				FIELD  RSSO_RCNT_INC                         :  1;		/* 11..11, 0x00000800 */
				FIELD  FLKO_RCNT_INC                         :  1;		/* 12..12, 0x00001000 */
				FIELD  PSO_RCNT_INC                          :  1;		/* 13..13, 0x00002000 */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_FBC_RCNT_INC;	/* CAM_CTL_FBC_RCNT_INC, CAM_A_CTL_FBC_RCNT_INC*/

typedef volatile union _CAM_REG_CTL_DBG_SET_
{
		volatile struct	/* 0x1A004070 */
		{
				FIELD  DEBUG_MOD_SEL                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DEBUG_SEL                             :  4;		/*  8..11, 0x00000F00 */
				FIELD  DEBUG_TOP_SEL                         :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DBG_SET;	/* CAM_CTL_DBG_SET, CAM_A_CTL_DBG_SET*/

typedef volatile union _CAM_REG_CTL_DBG_PORT_
{
		volatile struct	/* 0x1A004074 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DBG_PORT;	/* CAM_CTL_DBG_PORT, CAM_A_CTL_DBG_PORT*/

typedef volatile union _CAM_REG_CTL_DATE_CODE_
{
		volatile struct	/* 0x1A004078 */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DATE_CODE;	/* CAM_CTL_DATE_CODE, CAM_A_CTL_DATE_CODE*/

typedef volatile union _CAM_REG_CTL_PROJ_CODE_
{
		volatile struct	/* 0x1A00407C */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_PROJ_CODE;	/* CAM_CTL_PROJ_CODE, CAM_A_CTL_PROJ_CODE*/

typedef volatile union _CAM_REG_CTL_RAW_DCM_DIS_
{
		volatile struct	/* 0x1A004080 */
		{
				FIELD  TG_DCM_DIS                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_DCM_DIS                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGM_DCM_DIS                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RMG_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  OBC_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BNR_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSC_DCM_DIS                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CAC_DCM_DIS                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RPG_DCM_DIS                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZ_DCM_DIS                           :  1;		/* 10..10, 0x00000400 */
				FIELD  RMX_DCM_DIS                           :  1;		/* 11..11, 0x00000800 */
				FIELD  PAKG_DCM_DIS                          :  1;		/* 12..12, 0x00001000 */
				FIELD  BMX_DCM_DIS                           :  1;		/* 13..13, 0x00002000 */
				FIELD  CPG_DCM_DIS                           :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_DCM_DIS                           :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_DCM_DIS                           :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_DCM_DIS                            :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_DCM_DIS                          :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_DCM_DIS                            :  1;		/* 19..19, 0x00080000 */
				FIELD  QBIN1_DCM_DIS                         :  1;		/* 20..20, 0x00100000 */
				FIELD  LCS_DCM_DIS                           :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN2_DCM_DIS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  RCP_DCM_DIS                           :  1;		/* 23..23, 0x00800000 */
				FIELD  RCP3_DCM_DIS                          :  1;		/* 24..24, 0x01000000 */
				FIELD  PMX_DCM_DIS                           :  1;		/* 25..25, 0x02000000 */
				FIELD  PKP_DCM_DIS                           :  1;		/* 26..26, 0x04000000 */
				FIELD  BIN_DCM_DIS                           :  1;		/* 27..27, 0x08000000 */
				FIELD  DBS_DCM_DIS                           :  1;		/* 28..28, 0x10000000 */
				FIELD  DBN_DCM_DIS                           :  1;		/* 29..29, 0x20000000 */
				FIELD  PBN_DCM_DIS                           :  1;		/* 30..30, 0x40000000 */
				FIELD  UFEG_DCM_DIS                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_DIS;	/* CAM_CTL_RAW_DCM_DIS, CAM_A_CTL_RAW_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_DMA_DCM_DIS_
{
		volatile struct	/* 0x1A004084 */
		{
				FIELD  IMGO_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DCM_DIS                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_DCM_DIS                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DCM_DIS                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DCM_DIS                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_DCM_DIS                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_DCM_DIS                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_DCM_DIS                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_DCM_DIS                           :  1;		/* 10..10, 0x00000400 */
				FIELD  LSC3I_DCM_DIS                         :  1;		/* 11..11, 0x00000800 */
				FIELD  PDI_DCM_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  4;		/* 13..16, 0x0001E000 */
				FIELD  PSB_DCM_DIS                           :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG2_DCM_DIS                          :  1;		/* 18..18, 0x00040000 */
				FIELD  PDE_DCM_DIS                           :  1;		/* 19..19, 0x00080000 */
				FIELD  GSE_DCM_DIS                           :  1;		/* 20..20, 0x00100000 */
				FIELD  PCP_DCM_DIS                           :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN4_DCM_DIS                         :  1;		/* 22..22, 0x00400000 */
				FIELD  PS_DCM_DIS                            :  1;		/* 23..23, 0x00800000 */
				FIELD  HLR_DCM_DIS                           :  1;		/* 24..24, 0x01000000 */
				FIELD  AMX_DCM_DIS                           :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2F_DCM_DIS                          :  1;		/* 26..26, 0x04000000 */
				FIELD  VBN_DCM_DIS                           :  1;		/* 27..27, 0x08000000 */
				FIELD  SL2J_DCM_DIS                          :  1;		/* 28..28, 0x10000000 */
				FIELD  STM_DCM_DIS                           :  1;		/* 29..29, 0x20000000 */
				FIELD  SCM_DCM_DIS                           :  1;		/* 30..30, 0x40000000 */
				FIELD  SGG5_DCM_DIS                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_DIS;	/* CAM_CTL_DMA_DCM_DIS, CAM_A_CTL_DMA_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_TOP_DCM_DIS_
{
		volatile struct	/* 0x1A004088 */
		{
				FIELD  TOP_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_DIS;	/* CAM_CTL_TOP_DCM_DIS, CAM_A_CTL_TOP_DCM_DIS*/

typedef volatile union _CAM_REG_CTL_RAW_DCM_STATUS_
{
		volatile struct	/* 0x1A004090 */
		{
				FIELD  TG_DCM_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_DCM_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGM_DCM_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RMG_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  OBC_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BNR_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSC_DCM_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CAC_DCM_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RPG_DCM_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZ_DCM_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RMX_DCM_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  PAKG_DCM_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BMX_DCM_STATUS                        :  1;		/* 13..13, 0x00002000 */
				FIELD  CPG_DCM_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_DCM_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_DCM_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_DCM_STATUS                         :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_DCM_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_DCM_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  QBIN1_DCM_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  LCS_DCM_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN2_DCM_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  RCP_DCM_DIS                           :  1;		/* 23..23, 0x00800000 */
				FIELD  RCP3_DCM_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  PMX_DCM_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  PKP_DCM_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  BIN_DCM_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  DBS_DCM_STATUS                        :  1;		/* 28..28, 0x10000000 */
				FIELD  DBN_DCM_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  PBN_DCM_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  UFEG_DCM_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_DCM_STATUS;	/* CAM_CTL_RAW_DCM_STATUS, CAM_A_CTL_RAW_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_DCM_STATUS_
{
		volatile struct	/* 0x1A004094 */
		{
				FIELD  IMGO_DCM_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DCM_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DCM_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_DCM_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_DCM_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_DCM_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_DCM_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_DCM_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_DCM_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_DCM_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  LSC3I_DCM_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  PDI_DCM_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  4;		/* 13..16, 0x0001E000 */
				FIELD  PSB_DCM_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG2_DCM_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  PDE_DCM_STATUS                        :  1;		/* 19..19, 0x00080000 */
				FIELD  GSE_DCM_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  PCP_DCM_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN4_DCM_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  PS_DCM_STATUS                         :  1;		/* 23..23, 0x00800000 */
				FIELD  HLR_DCM_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  AMX_DCM_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2F_DCM_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  VBN_DCM_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  SL2J_DCM_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  STM_DCM_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  SCM_DCM_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SGG5_DCM_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_DCM_STATUS;	/* CAM_CTL_DMA_DCM_STATUS, CAM_A_CTL_DMA_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_TOP_DCM_STATUS_
{
		volatile struct	/* 0x1A004098 */
		{
				FIELD  TOP_DCM_STATUS                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_TOP_DCM_STATUS;	/* CAM_CTL_TOP_DCM_STATUS, CAM_A_CTL_TOP_DCM_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_REQ_STATUS_
{
		volatile struct	/* 0x1A0040A0 */
		{
				FIELD  TG_REQ_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_REQ_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGM_REQ_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RMG_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_REQ_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  OBC_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BNR_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSC_REQ_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CAC_REQ_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RPG_REQ_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZ_REQ_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RMX_REQ_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  PAKG_REQ_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BMX_REQ_STATUS                        :  1;		/* 13..13, 0x00002000 */
				FIELD  CPG_REQ_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_REQ_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_REQ_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_REQ_STATUS                         :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_REQ_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_REQ_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  QBIN1_REQ_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  LCS_REQ_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN2_REQ_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  RCP_REQ_DIS                           :  1;		/* 23..23, 0x00800000 */
				FIELD  RCP3_REQ_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  PMX_REQ_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  PKP_REQ_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  BIN_REQ_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  DBS_REQ_STATUS                        :  1;		/* 28..28, 0x10000000 */
				FIELD  DBN_REQ_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  PBN_REQ_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  UFEG_REQ_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_REQ_STATUS;	/* CAM_CTL_RAW_REQ_STATUS, CAM_A_CTL_RAW_REQ_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_REQ_STATUS_
{
		volatile struct	/* 0x1A0040A4 */
		{
				FIELD  IMGO_REQ_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_REQ_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_REQ_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_REQ_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_REQ_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_REQ_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_REQ_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_REQ_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_REQ_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_REQ_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_REQ_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  LSC3I_REQ_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  PDI_REQ_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  4;		/* 13..16, 0x0001E000 */
				FIELD  PSB_REQ_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG2_REQ_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  PDE_REQ_STATUS                        :  1;		/* 19..19, 0x00080000 */
				FIELD  GSE_REQ_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  PCP_REQ_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN4_REQ_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  PS_REQ_STATUS                         :  1;		/* 23..23, 0x00800000 */
				FIELD  HLR_REQ_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  AMX_REQ_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2F_REQ_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  VBN_REQ_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  SL2J_REQ_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  STM_REQ_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  SCM_REQ_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SGG5_REQ_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_REQ_STATUS;	/* CAM_CTL_DMA_REQ_STATUS, CAM_A_CTL_DMA_REQ_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_RDY_STATUS_
{
		volatile struct	/* 0x1A0040A8 */
		{
				FIELD  TG_RDY_STATUS                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMX_RDY_STATUS                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SGM_RDY_STATUS                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RMG_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RMM_RDY_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  OBC_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BNR_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  LSC_RDY_STATUS                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CAC_RDY_STATUS                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RPG_RDY_STATUS                        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZ_RDY_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  RMX_RDY_STATUS                        :  1;		/* 11..11, 0x00000800 */
				FIELD  PAKG_RDY_STATUS                       :  1;		/* 12..12, 0x00001000 */
				FIELD  BMX_RDY_STATUS                        :  1;		/* 13..13, 0x00002000 */
				FIELD  CPG_RDY_STATUS                        :  1;		/* 14..14, 0x00004000 */
				FIELD  PAK_RDY_STATUS                        :  1;		/* 15..15, 0x00008000 */
				FIELD  UFE_RDY_STATUS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  AF_RDY_STATUS                         :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG1_RDY_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  AA_RDY_STATUS                         :  1;		/* 19..19, 0x00080000 */
				FIELD  QBIN1_RDY_STATUS                      :  1;		/* 20..20, 0x00100000 */
				FIELD  LCS_RDY_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN2_RDY_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  RCP_RDY_STATUS                        :  1;		/* 23..23, 0x00800000 */
				FIELD  RCP3_RDY_STATUS                       :  1;		/* 24..24, 0x01000000 */
				FIELD  PMX_RDY_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  PKP_RDY_STATUS                        :  1;		/* 26..26, 0x04000000 */
				FIELD  BIN_RDY_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  DBS_RDY_STATUS                        :  1;		/* 28..28, 0x10000000 */
				FIELD  DBN_RDY_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  PBN_RDY_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  UFEG_RDY_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_RDY_STATUS;	/* CAM_CTL_RAW_RDY_STATUS, CAM_A_CTL_RAW_RDY_STATUS*/

typedef volatile union _CAM_REG_CTL_DMA_RDY_STATUS_
{
		volatile struct	/* 0x1A0040AC */
		{
				FIELD  IMGO_RDY_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_RDY_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_RDY_STATUS                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_RDY_STATUS                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_RDY_STATUS                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AAO_RDY_STATUS                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PSO_RDY_STATUS                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BPCI_RDY_STATUS                       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSCI_RDY_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CACI_RDY_STATUS                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  PDO_RDY_STATUS                        :  1;		/* 10..10, 0x00000400 */
				FIELD  LSC3I_RDY_STATUS                      :  1;		/* 11..11, 0x00000800 */
				FIELD  PDI_RDY_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  4;		/* 13..16, 0x0001E000 */
				FIELD  PSB_RDY_STATUS                        :  1;		/* 17..17, 0x00020000 */
				FIELD  SGG2_RDY_STATUS                       :  1;		/* 18..18, 0x00040000 */
				FIELD  PDE_RDY_STATUS                        :  1;		/* 19..19, 0x00080000 */
				FIELD  GSE_RDY_STATUS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  PCP_RDY_STATUS                        :  1;		/* 21..21, 0x00200000 */
				FIELD  QBIN4_RDY_STATUS                      :  1;		/* 22..22, 0x00400000 */
				FIELD  PS_RDY_STATUS                         :  1;		/* 23..23, 0x00800000 */
				FIELD  HLR_RDY_STATUS                        :  1;		/* 24..24, 0x01000000 */
				FIELD  AMX_RDY_STATUS                        :  1;		/* 25..25, 0x02000000 */
				FIELD  SL2F_RDY_STATUS                       :  1;		/* 26..26, 0x04000000 */
				FIELD  VBN_RDY_STATUS                        :  1;		/* 27..27, 0x08000000 */
				FIELD  SL2J_RDY_STATUS                       :  1;		/* 28..28, 0x10000000 */
				FIELD  STM_RDY_STATUS                        :  1;		/* 29..29, 0x20000000 */
				FIELD  SCM_RDY_STATUS                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SGG5_RDY_STATUS                       :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_DMA_RDY_STATUS;	/* CAM_CTL_DMA_RDY_STATUS, CAM_A_CTL_DMA_RDY_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT_EN_
{
		volatile struct	/* 0x1A0040B0 */
		{
				FIELD  VS_INT_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_CODE_ERR_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_APB_ERR_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_VS_ERR_EN                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_DROP_FRAME_EN                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_DROP_FRAME_EN                    :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS1_DON_EN                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF_INT_EN                            :  1;		/* 12..12, 0x00001000 */
				FIELD  SOF_WAIT_EN                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PDE_ERR_EN                            :  1;		/* 14..14, 0x00004000 */
				FIELD  AMX_ERR_EN                            :  1;		/* 15..15, 0x00008000 */
				FIELD  RMX_ERR_EN                            :  1;		/* 16..16, 0x00010000 */
				FIELD  BMX_ERR_EN                            :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_ERR_EN                           :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_ERR_EN                            :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_EN                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_EN                            :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_ERR_EN                            :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_ERR_EN                           :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_ERR_EN                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSC_ERR_EN                            :  1;		/* 25..25, 0x02000000 */
				FIELD  CAC_ERR_EN                            :  1;		/* 26..26, 0x04000000 */
				FIELD  UFEO_ERR_EN                           :  1;		/* 27..27, 0x08000000 */
				FIELD  PDO_ERR_EN                            :  1;		/* 28..28, 0x10000000 */
				FIELD  DMA_ERR_EN                            :  1;		/* 29..29, 0x20000000 */
				FIELD  SW_PASS1_DON_EN                       :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT_EN;	/* CAM_CTL_RAW_CCU_INT_EN, CAM_A_CTL_RAW_CCU_INT_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT_STATUS_
{
		volatile struct	/* 0x1A0040B4 */
		{
				FIELD  VS_INT_ST                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TG_INT1_ST                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TG_INT2_ST                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EXPDON_ST                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG_ERR_ST                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TG_GBERR_ST                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CQ_CODE_ERR_ST                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CQ_APB_ERR_ST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CQ_VS_ERR_ST                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  IMGO_DROP_FRAME_ST                    :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RRZO_DROP_FRAME_ST                    :  1;		/* 10..10, 0x00000400 */
				FIELD  PASS1_DON_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  SOF_INT_ST                            :  1;		/* 12..12, 0x00001000 */
				FIELD  SOF_WAIT_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PDE_ERR_ST                            :  1;		/* 14..14, 0x00004000 */
				FIELD  AMX_ERR_ST                            :  1;		/* 15..15, 0x00008000 */
				FIELD  RMX_ERR_ST                            :  1;		/* 16..16, 0x00010000 */
				FIELD  BMX_ERR_ST                            :  1;		/* 17..17, 0x00020000 */
				FIELD  RRZO_ERR_ST                           :  1;		/* 18..18, 0x00040000 */
				FIELD  AFO_ERR_ST                            :  1;		/* 19..19, 0x00080000 */
				FIELD  IMGO_ERR_ST                           :  1;		/* 20..20, 0x00100000 */
				FIELD  AAO_ERR_ST                            :  1;		/* 21..21, 0x00200000 */
				FIELD  PSO_ERR_ST                            :  1;		/* 22..22, 0x00400000 */
				FIELD  LCSO_ERR_ST                           :  1;		/* 23..23, 0x00800000 */
				FIELD  BNR_ERR_ST                            :  1;		/* 24..24, 0x01000000 */
				FIELD  LSC_ERR_ST                            :  1;		/* 25..25, 0x02000000 */
				FIELD  CAC_ERR_ST                            :  1;		/* 26..26, 0x04000000 */
				FIELD  UFEO_ERR_ST                           :  1;		/* 27..27, 0x08000000 */
				FIELD  PDO_ERR_ST                            :  1;		/* 28..28, 0x10000000 */
				FIELD  DMA_ERR_ST                            :  1;		/* 29..29, 0x20000000 */
				FIELD  SW_PASS1_DON_ST                       :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT_STATUS;	/* CAM_CTL_RAW_CCU_INT_STATUS, CAM_A_CTL_RAW_CCU_INT_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT2_EN_
{
		volatile struct	/* 0x1A0040B8 */
		{
				FIELD  IMGO_DONE_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_EN                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_DONE_EN                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_DONE_EN                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_DONE_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCSO_DONE_EN                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AAO_DONE_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC3I_DONE_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BPCI_DONE_EN                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSCI_DONE_EN                          :  1;		/* 10..10, 0x00000400 */
				FIELD  CACI_DONE_EN                          :  1;		/* 11..11, 0x00000800 */
				FIELD  AF_TAR_DONE_EN                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PDO_DONE_EN                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PSO_DONE_EN                           :  1;		/* 14..14, 0x00004000 */
				FIELD  RSSO_DONE_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR0_DONE_EN                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR1_DONE_EN                       :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR2_DONE_EN                       :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_THR3_DONE_EN                       :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ_THR4_DONE_EN                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_THR5_DONE_EN                       :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ_THR6_DONE_EN                       :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ_THR7_DONE_EN                       :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ_THR8_DONE_EN                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ_THR9_DONE_EN                       :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_THR10_DONE_EN                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_THR11_DONE_EN                      :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_THR12_DONE_EN                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT2_EN;	/* CAM_CTL_RAW_CCU_INT2_EN, CAM_A_CTL_RAW_CCU_INT2_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT2_STATUS_
{
		volatile struct	/* 0x1A0040BC */
		{
				FIELD  IMGO_DONE_ST                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  UFEO_DONE_ST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RRZO_DONE_ST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_DONE_ST                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_DONE_ST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AFO_DEONE_ST                          :  1;		/*  5.. 5, 0x00000020 */
				FIELD  LCSO_DONE_ST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  AAO_DONE_ST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LSC3I_DONE_ST                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  BPCI_DONE_ST                          :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSCI_DONE_ST                          :  1;		/* 10..10, 0x00000400 */
				FIELD  CACI_DONE_ST                          :  1;		/* 11..11, 0x00000800 */
				FIELD  AF_TAR_DONE_ST                        :  1;		/* 12..12, 0x00001000 */
				FIELD  PDO_DONE_ST                           :  1;		/* 13..13, 0x00002000 */
				FIELD  PSO_DONE_ST                           :  1;		/* 14..14, 0x00004000 */
				FIELD  RSSO_DONE_ST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  CQ_THR0_DONE_ST                       :  1;		/* 16..16, 0x00010000 */
				FIELD  CQ_THR1_DONE_ST                       :  1;		/* 17..17, 0x00020000 */
				FIELD  CQ_THR2_DONE_ST                       :  1;		/* 18..18, 0x00040000 */
				FIELD  CQ_THR3_DONE_ST                       :  1;		/* 19..19, 0x00080000 */
				FIELD  CQ_THR4_DONE_ST                       :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_THR5_DONE_ST                       :  1;		/* 21..21, 0x00200000 */
				FIELD  CQ_THR6_DONE_ST                       :  1;		/* 22..22, 0x00400000 */
				FIELD  CQ_THR7_DONE_ST                       :  1;		/* 23..23, 0x00800000 */
				FIELD  CQ_THR8_DONE_ST                       :  1;		/* 24..24, 0x01000000 */
				FIELD  CQ_THR9_DONE_ST                       :  1;		/* 25..25, 0x02000000 */
				FIELD  CQ_THR10_DONE_ST                      :  1;		/* 26..26, 0x04000000 */
				FIELD  CQ_THR11_DONE_ST                      :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_THR12_DONE_ST                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT2_STATUS;	/* CAM_CTL_RAW_CCU_INT2_STATUS, CAM_A_CTL_RAW_CCU_INT2_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_EN_
{
		volatile struct	/* 0x1A0040C0 */
		{
				FIELD  AE_CCU_READ_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_CCU_CLLSN_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  PDI_DONE_EN                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_EN;	/* CAM_CTL_RAW_INT3_EN, CAM_A_CTL_RAW_INT3_EN*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_STATUS_
{
		volatile struct	/* 0x1A0040C4 */
		{
				FIELD  AE_CCU_READ_ST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_CCU_CLLSN_ST                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  PDI_DONE_ST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_STATUS;	/* CAM_CTL_RAW_INT3_STATUS, CAM_A_CTL_RAW_INT3_STATUS*/

typedef volatile union _CAM_REG_CTL_RAW_INT3_STATUSX_
{
		volatile struct	/* 0x1A0040C8 */
		{
				FIELD  AE_CCU_READ_ST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_CCU_CLLSN_ST                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  PDI_DONE_ST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_INT3_STATUSX;	/* CAM_CTL_RAW_INT3_STATUSX, CAM_A_CTL_RAW_INT3_STATUSX*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT3_EN_
{
		volatile struct	/* 0x1A0040D0 */
		{
				FIELD  AE_CCU_READ_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_CCU_CLLSN_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  PDI_DONE_EN                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT3_EN;	/* CAM_CTL_RAW_CCU_INT3_EN, CAM_A_CTL_RAW_CCU_INT3_EN*/

typedef volatile union _CAM_REG_CTL_RAW_CCU_INT3_STATUS_
{
		volatile struct	/* 0x1A0040D4 */
		{
				FIELD  AE_CCU_READ_ST                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_CCU_CLLSN_ST                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  PDI_DONE_ST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_RAW_CCU_INT3_STATUS;	/* CAM_CTL_RAW_CCU_INT3_STATUS, CAM_A_CTL_RAW_CCU_INT3_STATUS*/

typedef volatile union _CAM_REG_CTL_UNI_B_DONE_SEL_
{
		volatile struct	/* 0x1A0040D8 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RSSO_B_DONE_SEL                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  EISO_B_DONE_SEL                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FLKO_B_DONE_SEL                       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CTL_UNI_B_DONE_SEL;	/* CAM_CTL_UNI_B_DONE_SEL, CAM_A_CTL_UNI_B_DONE_SEL*/

typedef volatile union _CAM_REG_FBC_IMGO_CTL1_
{
		volatile struct	/* 0x1A004110 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_IMGO_CTL1;	/* CAM_FBC_IMGO_CTL1, CAM_A_FBC_IMGO_CTL1*/

typedef volatile union _CAM_REG_FBC_IMGO_CTL2_
{
		volatile struct	/* 0x1A004114 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_IMGO_CTL2;	/* CAM_FBC_IMGO_CTL2, CAM_A_FBC_IMGO_CTL2*/

typedef volatile union _CAM_REG_FBC_RRZO_CTL1_
{
		volatile struct	/* 0x1A004118 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_RRZO_CTL1;	/* CAM_FBC_RRZO_CTL1, CAM_A_FBC_RRZO_CTL1*/

typedef volatile union _CAM_REG_FBC_RRZO_CTL2_
{
		volatile struct	/* 0x1A00411C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_RRZO_CTL2;	/* CAM_FBC_RRZO_CTL2, CAM_A_FBC_RRZO_CTL2*/

typedef volatile union _CAM_REG_FBC_UFEO_CTL1_
{
		volatile struct	/* 0x1A004120 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_UFEO_CTL1;	/* CAM_FBC_UFEO_CTL1, CAM_A_FBC_UFEO_CTL1*/

typedef volatile union _CAM_REG_FBC_UFEO_CTL2_
{
		volatile struct	/* 0x1A004124 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_UFEO_CTL2;	/* CAM_FBC_UFEO_CTL2, CAM_A_FBC_UFEO_CTL2*/

typedef volatile union _CAM_REG_FBC_LCSO_CTL1_
{
		volatile struct	/* 0x1A004128 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_LCSO_CTL1;	/* CAM_FBC_LCSO_CTL1, CAM_A_FBC_LCSO_CTL1*/

typedef volatile union _CAM_REG_FBC_LCSO_CTL2_
{
		volatile struct	/* 0x1A00412C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_LCSO_CTL2;	/* CAM_FBC_LCSO_CTL2, CAM_A_FBC_LCSO_CTL2*/

typedef volatile union _CAM_REG_FBC_AFO_CTL1_
{
		volatile struct	/* 0x1A004130 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_AFO_CTL1;	/* CAM_FBC_AFO_CTL1, CAM_A_FBC_AFO_CTL1*/

typedef volatile union _CAM_REG_FBC_AFO_CTL2_
{
		volatile struct	/* 0x1A004134 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_AFO_CTL2;	/* CAM_FBC_AFO_CTL2, CAM_A_FBC_AFO_CTL2*/

typedef volatile union _CAM_REG_FBC_AAO_CTL1_
{
		volatile struct	/* 0x1A004138 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_AAO_CTL1;	/* CAM_FBC_AAO_CTL1, CAM_A_FBC_AAO_CTL1*/

typedef volatile union _CAM_REG_FBC_AAO_CTL2_
{
		volatile struct	/* 0x1A00413C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_AAO_CTL2;	/* CAM_FBC_AAO_CTL2, CAM_A_FBC_AAO_CTL2*/

typedef volatile union _CAM_REG_FBC_PDO_CTL1_
{
		volatile struct	/* 0x1A004140 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_PDO_CTL1;	/* CAM_FBC_PDO_CTL1, CAM_A_FBC_PDO_CTL1*/

typedef volatile union _CAM_REG_FBC_PDO_CTL2_
{
		volatile struct	/* 0x1A004144 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_PDO_CTL2;	/* CAM_FBC_PDO_CTL2, CAM_A_FBC_PDO_CTL2*/

typedef volatile union _CAM_REG_FBC_PSO_CTL1_
{
		volatile struct	/* 0x1A004148 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_PSO_CTL1;	/* CAM_FBC_PSO_CTL1, CAM_A_FBC_PSO_CTL1*/

typedef volatile union _CAM_REG_FBC_PSO_CTL2_
{
		volatile struct	/* 0x1A00414C */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_FBC_PSO_CTL2;	/* CAM_FBC_PSO_CTL2, CAM_A_FBC_PSO_CTL2*/

typedef volatile union _CAM_REG_CQ_EN_
{
		volatile struct	/* 0x1A004160 */
		{
				FIELD  CQ_APB_2T                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CQ_DROP_FRAME_EN                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  CQ_DB_EN                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  CQ_DB_LOAD_MODE                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  CQ_RESET                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_EN;	/* CAM_CQ_EN, CAM_A_CQ_EN*/

typedef volatile union _CAM_REG_CQ_THR0_CTL_
{
		volatile struct	/* 0x1A004164 */
		{
				FIELD  CQ_THR0_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR0_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR0_CTL;	/* CAM_CQ_THR0_CTL, CAM_A_CQ_THR0_CTL*/

typedef volatile union _CAM_REG_CQ_THR0_BASEADDR_
{
		volatile struct	/* 0x1A004168 */
		{
				FIELD  CQ_THR0_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR0_BASEADDR;	/* CAM_CQ_THR0_BASEADDR, CAM_A_CQ_THR0_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR0_DESC_SIZE_
{
		volatile struct	/* 0x1A00416C */
		{
				FIELD  CQ_THR0_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR0_DESC_SIZE;	/* CAM_CQ_THR0_DESC_SIZE, CAM_A_CQ_THR0_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR1_CTL_
{
		volatile struct	/* 0x1A004170 */
		{
				FIELD  CQ_THR1_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR1_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR1_CTL;	/* CAM_CQ_THR1_CTL, CAM_A_CQ_THR1_CTL*/

typedef volatile union _CAM_REG_CQ_THR1_BASEADDR_
{
		volatile struct	/* 0x1A004174 */
		{
				FIELD  CQ_THR1_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR1_BASEADDR;	/* CAM_CQ_THR1_BASEADDR, CAM_A_CQ_THR1_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR1_DESC_SIZE_
{
		volatile struct	/* 0x1A004178 */
		{
				FIELD  CQ_THR1_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR1_DESC_SIZE;	/* CAM_CQ_THR1_DESC_SIZE, CAM_A_CQ_THR1_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR2_CTL_
{
		volatile struct	/* 0x1A00417C */
		{
				FIELD  CQ_THR2_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR2_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR2_CTL;	/* CAM_CQ_THR2_CTL, CAM_A_CQ_THR2_CTL*/

typedef volatile union _CAM_REG_CQ_THR2_BASEADDR_
{
		volatile struct	/* 0x1A004180 */
		{
				FIELD  CQ_THR2_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR2_BASEADDR;	/* CAM_CQ_THR2_BASEADDR, CAM_A_CQ_THR2_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR2_DESC_SIZE_
{
		volatile struct	/* 0x1A004184 */
		{
				FIELD  CQ_THR2_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR2_DESC_SIZE;	/* CAM_CQ_THR2_DESC_SIZE, CAM_A_CQ_THR2_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR3_CTL_
{
		volatile struct	/* 0x1A004188 */
		{
				FIELD  CQ_THR3_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR3_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR3_CTL;	/* CAM_CQ_THR3_CTL, CAM_A_CQ_THR3_CTL*/

typedef volatile union _CAM_REG_CQ_THR3_BASEADDR_
{
		volatile struct	/* 0x1A00418C */
		{
				FIELD  CQ_THR3_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR3_BASEADDR;	/* CAM_CQ_THR3_BASEADDR, CAM_A_CQ_THR3_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR3_DESC_SIZE_
{
		volatile struct	/* 0x1A004190 */
		{
				FIELD  CQ_THR3_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR3_DESC_SIZE;	/* CAM_CQ_THR3_DESC_SIZE, CAM_A_CQ_THR3_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR4_CTL_
{
		volatile struct	/* 0x1A004194 */
		{
				FIELD  CQ_THR4_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR4_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR4_CTL;	/* CAM_CQ_THR4_CTL, CAM_A_CQ_THR4_CTL*/

typedef volatile union _CAM_REG_CQ_THR4_BASEADDR_
{
		volatile struct	/* 0x1A004198 */
		{
				FIELD  CQ_THR4_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR4_BASEADDR;	/* CAM_CQ_THR4_BASEADDR, CAM_A_CQ_THR4_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR4_DESC_SIZE_
{
		volatile struct	/* 0x1A00419C */
		{
				FIELD  CQ_THR4_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR4_DESC_SIZE;	/* CAM_CQ_THR4_DESC_SIZE, CAM_A_CQ_THR4_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR5_CTL_
{
		volatile struct	/* 0x1A0041A0 */
		{
				FIELD  CQ_THR5_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR5_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR5_CTL;	/* CAM_CQ_THR5_CTL, CAM_A_CQ_THR5_CTL*/

typedef volatile union _CAM_REG_CQ_THR5_BASEADDR_
{
		volatile struct	/* 0x1A0041A4 */
		{
				FIELD  CQ_THR5_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR5_BASEADDR;	/* CAM_CQ_THR5_BASEADDR, CAM_A_CQ_THR5_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR5_DESC_SIZE_
{
		volatile struct	/* 0x1A0041A8 */
		{
				FIELD  CQ_THR5_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR5_DESC_SIZE;	/* CAM_CQ_THR5_DESC_SIZE, CAM_A_CQ_THR5_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR6_CTL_
{
		volatile struct	/* 0x1A0041AC */
		{
				FIELD  CQ_THR6_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR6_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR6_CTL;	/* CAM_CQ_THR6_CTL, CAM_A_CQ_THR6_CTL*/

typedef volatile union _CAM_REG_CQ_THR6_BASEADDR_
{
		volatile struct	/* 0x1A0041B0 */
		{
				FIELD  CQ_THR6_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR6_BASEADDR;	/* CAM_CQ_THR6_BASEADDR, CAM_A_CQ_THR6_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR6_DESC_SIZE_
{
		volatile struct	/* 0x1A0041B4 */
		{
				FIELD  CQ_THR6_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR6_DESC_SIZE;	/* CAM_CQ_THR6_DESC_SIZE, CAM_A_CQ_THR6_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR7_CTL_
{
		volatile struct	/* 0x1A0041B8 */
		{
				FIELD  CQ_THR7_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR7_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR7_CTL;	/* CAM_CQ_THR7_CTL, CAM_A_CQ_THR7_CTL*/

typedef volatile union _CAM_REG_CQ_THR7_BASEADDR_
{
		volatile struct	/* 0x1A0041BC */
		{
				FIELD  CQ_THR7_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR7_BASEADDR;	/* CAM_CQ_THR7_BASEADDR, CAM_A_CQ_THR7_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR7_DESC_SIZE_
{
		volatile struct	/* 0x1A0041C0 */
		{
				FIELD  CQ_THR7_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR7_DESC_SIZE;	/* CAM_CQ_THR7_DESC_SIZE, CAM_A_CQ_THR7_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR8_CTL_
{
		volatile struct	/* 0x1A0041C4 */
		{
				FIELD  CQ_THR8_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR8_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR8_CTL;	/* CAM_CQ_THR8_CTL, CAM_A_CQ_THR8_CTL*/

typedef volatile union _CAM_REG_CQ_THR8_BASEADDR_
{
		volatile struct	/* 0x1A0041C8 */
		{
				FIELD  CQ_THR8_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR8_BASEADDR;	/* CAM_CQ_THR8_BASEADDR, CAM_A_CQ_THR8_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR8_DESC_SIZE_
{
		volatile struct	/* 0x1A0041CC */
		{
				FIELD  CQ_THR8_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR8_DESC_SIZE;	/* CAM_CQ_THR8_DESC_SIZE, CAM_A_CQ_THR8_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR9_CTL_
{
		volatile struct	/* 0x1A0041D0 */
		{
				FIELD  CQ_THR9_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR9_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR9_CTL;	/* CAM_CQ_THR9_CTL, CAM_A_CQ_THR9_CTL*/

typedef volatile union _CAM_REG_CQ_THR9_BASEADDR_
{
		volatile struct	/* 0x1A0041D4 */
		{
				FIELD  CQ_THR9_BASEADDR                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR9_BASEADDR;	/* CAM_CQ_THR9_BASEADDR, CAM_A_CQ_THR9_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR9_DESC_SIZE_
{
		volatile struct	/* 0x1A0041D8 */
		{
				FIELD  CQ_THR9_DESC_SIZE                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR9_DESC_SIZE;	/* CAM_CQ_THR9_DESC_SIZE, CAM_A_CQ_THR9_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR10_CTL_
{
		volatile struct	/* 0x1A0041DC */
		{
				FIELD  CQ_THR10_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR10_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR10_CTL;	/* CAM_CQ_THR10_CTL, CAM_A_CQ_THR10_CTL*/

typedef volatile union _CAM_REG_CQ_THR10_BASEADDR_
{
		volatile struct	/* 0x1A0041E0 */
		{
				FIELD  CQ_THR10_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR10_BASEADDR;	/* CAM_CQ_THR10_BASEADDR, CAM_A_CQ_THR10_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR10_DESC_SIZE_
{
		volatile struct	/* 0x1A0041E4 */
		{
				FIELD  CQ_THR10_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR10_DESC_SIZE;	/* CAM_CQ_THR10_DESC_SIZE, CAM_A_CQ_THR10_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR11_CTL_
{
		volatile struct	/* 0x1A0041E8 */
		{
				FIELD  CQ_THR11_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR11_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR11_CTL;	/* CAM_CQ_THR11_CTL, CAM_A_CQ_THR11_CTL*/

typedef volatile union _CAM_REG_CQ_THR11_BASEADDR_
{
		volatile struct	/* 0x1A0041EC */
		{
				FIELD  CQ_THR11_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR11_BASEADDR;	/* CAM_CQ_THR11_BASEADDR, CAM_A_CQ_THR11_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR11_DESC_SIZE_
{
		volatile struct	/* 0x1A0041F0 */
		{
				FIELD  CQ_THR11_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR11_DESC_SIZE;	/* CAM_CQ_THR11_DESC_SIZE, CAM_A_CQ_THR11_DESC_SIZE*/

typedef volatile union _CAM_REG_CQ_THR12_CTL_
{
		volatile struct	/* 0x1A0041F4 */
		{
				FIELD  CQ_THR12_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CQ_THR12_MODE                         :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR12_CTL;	/* CAM_CQ_THR12_CTL, CAM_A_CQ_THR12_CTL*/

typedef volatile union _CAM_REG_CQ_THR12_BASEADDR_
{
		volatile struct	/* 0x1A0041F8 */
		{
				FIELD  CQ_THR12_BASEADDR                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR12_BASEADDR;	/* CAM_CQ_THR12_BASEADDR, CAM_A_CQ_THR12_BASEADDR*/

typedef volatile union _CAM_REG_CQ_THR12_DESC_SIZE_
{
		volatile struct	/* 0x1A0041FC */
		{
				FIELD  CQ_THR12_DESC_SIZE                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ_THR12_DESC_SIZE;	/* CAM_CQ_THR12_DESC_SIZE, CAM_A_CQ_THR12_DESC_SIZE*/

typedef volatile union _CAM_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1A004200 */
		{
				FIELD  IMGO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_SOFT_RST_STAT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_SOFT_RST_STAT                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_SOFT_RST_STAT                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_SOFT_RST_STAT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_SOFT_RST_STAT                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_SOFT_RST_STAT                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PSO_SOFT_RST_STAT                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  PDI_SOFT_RST_STAT                     :  1;		/* 15..15, 0x00008000 */
				FIELD  BPCI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_SOFT_RST_STAT                    :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_SOFT_RST_STAT                    :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC3I_SOFT_RST_STAT                   :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_SOFT_RSTSTAT;	/* CAM_DMA_SOFT_RSTSTAT, CAM_A_DMA_SOFT_RSTSTAT*/

typedef volatile union _CAM_REG_CQ0I_BASE_ADDR_
{
		volatile struct	/* 0x1A004204 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_BASE_ADDR;	/* CAM_CQ0I_BASE_ADDR, CAM_A_CQ0I_BASE_ADDR*/

typedef volatile union _CAM_REG_CQ0I_XSIZE_
{
		volatile struct	/* 0x1A004208 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CQ0I_XSIZE;	/* CAM_CQ0I_XSIZE, CAM_A_CQ0I_XSIZE*/

typedef volatile union _CAM_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x1A00420C */
		{
				FIELD  IMGO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_V_FLIP_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_V_FLIP_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_V_FLIP_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_V_FLIP_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_V_FLIP_EN                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_V_FLIP_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PSO_V_FLIP_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  PDI_V_FLIP_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  BPCI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_V_FLIP_EN                        :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_V_FLIP_EN                        :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC3I_V_FLIP_EN                       :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_VERTICAL_FLIP_EN;	/* CAM_VERTICAL_FLIP_EN, CAM_A_VERTICAL_FLIP_EN*/

typedef volatile union _CAM_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x1A004210 */
		{
				FIELD  IMGO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_SOFT_RST                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_SOFT_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_SOFT_RST                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_SOFT_RST                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_SOFT_RST                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_SOFT_RST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PSO_SOFT_RST                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  PDI_SOFT_RST                          :  1;		/* 15..15, 0x00008000 */
				FIELD  BPCI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_SOFT_RST                         :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_SOFT_RST                         :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC3I_SOFT_RST                        :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_SOFT_RESET;	/* CAM_DMA_SOFT_RESET, CAM_A_DMA_SOFT_RESET*/

typedef volatile union _CAM_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1A004214 */
		{
				FIELD  IMGO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_LAST_ULTRA_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_LAST_ULTRA_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_LAST_ULTRA_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_LAST_ULTRA_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_LAST_ULTRA_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_LAST_ULTRA_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PSO_LAST_ULTRA_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  PDI_LAST_ULTRA_EN                     :  1;		/* 15..15, 0x00008000 */
				FIELD  BPCI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_LAST_ULTRA_EN                    :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_LAST_ULTRA_EN                    :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC3I_LAST_ULTRA_EN                   :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LAST_ULTRA_EN;	/* CAM_LAST_ULTRA_EN, CAM_A_LAST_ULTRA_EN*/

typedef volatile union _CAM_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1A004218 */
		{
				FIELD  rsv_0                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 26..26, 0x04000000 */
				FIELD  GCLAST_EN                             :  1;		/* 27..27, 0x08000000 */
				FIELD  CQ_ULTRA_BPCI_EN                      :  1;		/* 28..28, 0x10000000 */
				FIELD  CQ_ULTRA_LSCI_EN                      :  1;		/* 29..29, 0x20000000 */
				FIELD  UFOG_RRZO_EN                          :  1;		/* 30..30, 0x40000000 */
				FIELD  UFO_IMGO_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SPECIAL_FUN_EN;	/* CAM_SPECIAL_FUN_EN, CAM_A_SPECIAL_FUN_EN*/

typedef volatile union _CAM_REG_IMGO_BASE_ADDR_
{
		volatile struct	/* 0x1A004220 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_BASE_ADDR;	/* CAM_IMGO_BASE_ADDR, CAM_A_IMGO_BASE_ADDR*/

typedef volatile union _CAM_REG_IMGO_OFST_ADDR_
{
		volatile struct	/* 0x1A004228 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_OFST_ADDR;	/* CAM_IMGO_OFST_ADDR, CAM_A_IMGO_OFST_ADDR*/

typedef volatile union _CAM_REG_IMGO_DRS_
{
		volatile struct	/* 0x1A00422C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_DRS;	/* CAM_IMGO_DRS, CAM_A_IMGO_DRS*/

typedef volatile union _CAM_REG_IMGO_XSIZE_
{
		volatile struct	/* 0x1A004230 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_XSIZE;	/* CAM_IMGO_XSIZE, CAM_A_IMGO_XSIZE*/

typedef volatile union _CAM_REG_IMGO_YSIZE_
{
		volatile struct	/* 0x1A004234 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_YSIZE;	/* CAM_IMGO_YSIZE, CAM_A_IMGO_YSIZE*/

typedef volatile union _CAM_REG_IMGO_STRIDE_
{
		volatile struct	/* 0x1A004238 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_STRIDE;	/* CAM_IMGO_STRIDE, CAM_A_IMGO_STRIDE*/

typedef volatile union _CAM_REG_IMGO_CON_
{
		volatile struct	/* 0x1A00423C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON;	/* CAM_IMGO_CON, CAM_A_IMGO_CON*/

typedef volatile union _CAM_REG_IMGO_CON2_
{
		volatile struct	/* 0x1A004240 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON2;	/* CAM_IMGO_CON2, CAM_A_IMGO_CON2*/

typedef volatile union _CAM_REG_IMGO_CON3_
{
		volatile struct	/* 0x1A004244 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON3;	/* CAM_IMGO_CON3, CAM_A_IMGO_CON3*/

typedef volatile union _CAM_REG_IMGO_CROP_
{
		volatile struct	/* 0x1A004248 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CROP;	/* CAM_IMGO_CROP, CAM_A_IMGO_CROP*/

typedef volatile union _CAM_REG_IMGO_CON4_
{
		volatile struct	/* 0x1A00424C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_CON4;	/* CAM_IMGO_CON4, CAM_A_IMGO_CON4*/

typedef volatile union _CAM_REG_RRZO_BASE_ADDR_
{
		volatile struct	/* 0x1A004250 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_BASE_ADDR;	/* CAM_RRZO_BASE_ADDR, CAM_A_RRZO_BASE_ADDR*/

typedef volatile union _CAM_REG_RRZO_OFST_ADDR_
{
		volatile struct	/* 0x1A004258 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_OFST_ADDR;	/* CAM_RRZO_OFST_ADDR, CAM_A_RRZO_OFST_ADDR*/

typedef volatile union _CAM_REG_RRZO_DRS_
{
		volatile struct	/* 0x1A00425C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_DRS;	/* CAM_RRZO_DRS, CAM_A_RRZO_DRS*/

typedef volatile union _CAM_REG_RRZO_XSIZE_
{
		volatile struct	/* 0x1A004260 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_XSIZE;	/* CAM_RRZO_XSIZE, CAM_A_RRZO_XSIZE*/

typedef volatile union _CAM_REG_RRZO_YSIZE_
{
		volatile struct	/* 0x1A004264 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_YSIZE;	/* CAM_RRZO_YSIZE, CAM_A_RRZO_YSIZE*/

typedef volatile union _CAM_REG_RRZO_STRIDE_
{
		volatile struct	/* 0x1A004268 */
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
}CAM_REG_RRZO_STRIDE;	/* CAM_RRZO_STRIDE, CAM_A_RRZO_STRIDE*/

typedef volatile union _CAM_REG_RRZO_CON_
{
		volatile struct	/* 0x1A00426C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON;	/* CAM_RRZO_CON, CAM_A_RRZO_CON*/

typedef volatile union _CAM_REG_RRZO_CON2_
{
		volatile struct	/* 0x1A004270 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON2;	/* CAM_RRZO_CON2, CAM_A_RRZO_CON2*/

typedef volatile union _CAM_REG_RRZO_CON3_
{
		volatile struct	/* 0x1A004274 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON3;	/* CAM_RRZO_CON3, CAM_A_RRZO_CON3*/

typedef volatile union _CAM_REG_RRZO_CROP_
{
		volatile struct	/* 0x1A004278 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CROP;	/* CAM_RRZO_CROP, CAM_A_RRZO_CROP*/

typedef volatile union _CAM_REG_RRZO_CON4_
{
		volatile struct	/* 0x1A00427C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_CON4;	/* CAM_RRZO_CON4, CAM_A_RRZO_CON4*/

typedef volatile union _CAM_REG_AAO_BASE_ADDR_
{
		volatile struct	/* 0x1A004280 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_BASE_ADDR;	/* CAM_AAO_BASE_ADDR, CAM_A_AAO_BASE_ADDR*/

typedef volatile union _CAM_REG_AAO_OFST_ADDR_
{
		volatile struct	/* 0x1A004288 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_OFST_ADDR;	/* CAM_AAO_OFST_ADDR, CAM_A_AAO_OFST_ADDR*/

typedef volatile union _CAM_REG_AAO_DRS_
{
		volatile struct	/* 0x1A00428C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_DRS;	/* CAM_AAO_DRS, CAM_A_AAO_DRS*/

typedef volatile union _CAM_REG_AAO_XSIZE_
{
		volatile struct	/* 0x1A004290 */
		{
				FIELD  XSIZE                                 : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_XSIZE;	/* CAM_AAO_XSIZE, CAM_A_AAO_XSIZE*/

typedef volatile union _CAM_REG_AAO_YSIZE_
{
		volatile struct	/* 0x1A004294 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_YSIZE;	/* CAM_AAO_YSIZE, CAM_A_AAO_YSIZE*/

typedef volatile union _CAM_REG_AAO_STRIDE_
{
		volatile struct	/* 0x1A004298 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_STRIDE;	/* CAM_AAO_STRIDE, CAM_A_AAO_STRIDE*/

typedef volatile union _CAM_REG_AAO_CON_
{
		volatile struct	/* 0x1A00429C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON;	/* CAM_AAO_CON, CAM_A_AAO_CON*/

typedef volatile union _CAM_REG_AAO_CON2_
{
		volatile struct	/* 0x1A0042A0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON2;	/* CAM_AAO_CON2, CAM_A_AAO_CON2*/

typedef volatile union _CAM_REG_AAO_CON3_
{
		volatile struct	/* 0x1A0042A4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON3;	/* CAM_AAO_CON3, CAM_A_AAO_CON3*/

typedef volatile union _CAM_REG_AAO_CON4_
{
		volatile struct	/* 0x1A0042AC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_CON4;	/* CAM_AAO_CON4, CAM_A_AAO_CON4*/

typedef volatile union _CAM_REG_AFO_BASE_ADDR_
{
		volatile struct	/* 0x1A0042B0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_BASE_ADDR;	/* CAM_AFO_BASE_ADDR, CAM_A_AFO_BASE_ADDR*/

typedef volatile union _CAM_REG_AFO_OFST_ADDR_
{
		volatile struct	/* 0x1A0042B8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_OFST_ADDR;	/* CAM_AFO_OFST_ADDR, CAM_A_AFO_OFST_ADDR*/

typedef volatile union _CAM_REG_AFO_DRS_
{
		volatile struct	/* 0x1A0042BC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_DRS;	/* CAM_AFO_DRS, CAM_A_AFO_DRS*/

typedef volatile union _CAM_REG_AFO_XSIZE_
{
		volatile struct	/* 0x1A0042C0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_XSIZE;	/* CAM_AFO_XSIZE, CAM_A_AFO_XSIZE*/

typedef volatile union _CAM_REG_AFO_YSIZE_
{
		volatile struct	/* 0x1A0042C4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_YSIZE;	/* CAM_AFO_YSIZE, CAM_A_AFO_YSIZE*/

typedef volatile union _CAM_REG_AFO_STRIDE_
{
		volatile struct	/* 0x1A0042C8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_STRIDE;	/* CAM_AFO_STRIDE, CAM_A_AFO_STRIDE*/

typedef volatile union _CAM_REG_AFO_CON_
{
		volatile struct	/* 0x1A0042CC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_CON;	/* CAM_AFO_CON, CAM_A_AFO_CON*/

typedef volatile union _CAM_REG_AFO_CON2_
{
		volatile struct	/* 0x1A0042D0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_CON2;	/* CAM_AFO_CON2, CAM_A_AFO_CON2*/

typedef volatile union _CAM_REG_AFO_CON3_
{
		volatile struct	/* 0x1A0042D4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_CON3;	/* CAM_AFO_CON3, CAM_A_AFO_CON3*/

typedef volatile union _CAM_REG_AFO_CON4_
{
		volatile struct	/* 0x1A0042DC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_CON4;	/* CAM_AFO_CON4, CAM_A_AFO_CON4*/

typedef volatile union _CAM_REG_LCSO_BASE_ADDR_
{
		volatile struct	/* 0x1A0042E0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_BASE_ADDR;	/* CAM_LCSO_BASE_ADDR, CAM_A_LCSO_BASE_ADDR*/

typedef volatile union _CAM_REG_LCSO_OFST_ADDR_
{
		volatile struct	/* 0x1A0042E8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_OFST_ADDR;	/* CAM_LCSO_OFST_ADDR, CAM_A_LCSO_OFST_ADDR*/

typedef volatile union _CAM_REG_LCSO_DRS_
{
		volatile struct	/* 0x1A0042EC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_DRS;	/* CAM_LCSO_DRS, CAM_A_LCSO_DRS*/

typedef volatile union _CAM_REG_LCSO_XSIZE_
{
		volatile struct	/* 0x1A0042F0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_XSIZE;	/* CAM_LCSO_XSIZE, CAM_A_LCSO_XSIZE*/

typedef volatile union _CAM_REG_LCSO_YSIZE_
{
		volatile struct	/* 0x1A0042F4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_YSIZE;	/* CAM_LCSO_YSIZE, CAM_A_LCSO_YSIZE*/

typedef volatile union _CAM_REG_LCSO_STRIDE_
{
		volatile struct	/* 0x1A0042F8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_STRIDE;	/* CAM_LCSO_STRIDE, CAM_A_LCSO_STRIDE*/

typedef volatile union _CAM_REG_LCSO_CON_
{
		volatile struct	/* 0x1A0042FC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_CON;	/* CAM_LCSO_CON, CAM_A_LCSO_CON*/

typedef volatile union _CAM_REG_LCSO_CON2_
{
		volatile struct	/* 0x1A004300 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_CON2;	/* CAM_LCSO_CON2, CAM_A_LCSO_CON2*/

typedef volatile union _CAM_REG_LCSO_CON3_
{
		volatile struct	/* 0x1A004304 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_CON3;	/* CAM_LCSO_CON3, CAM_A_LCSO_CON3*/

typedef volatile union _CAM_REG_LCSO_CON4_
{
		volatile struct	/* 0x1A00430C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_CON4;	/* CAM_LCSO_CON4, CAM_A_LCSO_CON4*/

typedef volatile union _CAM_REG_UFEO_BASE_ADDR_
{
		volatile struct	/* 0x1A004310 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_BASE_ADDR;	/* CAM_UFEO_BASE_ADDR, CAM_A_UFEO_BASE_ADDR*/

typedef volatile union _CAM_REG_UFEO_OFST_ADDR_
{
		volatile struct	/* 0x1A004318 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_OFST_ADDR;	/* CAM_UFEO_OFST_ADDR, CAM_A_UFEO_OFST_ADDR*/

typedef volatile union _CAM_REG_UFEO_DRS_
{
		volatile struct	/* 0x1A00431C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_DRS;	/* CAM_UFEO_DRS, CAM_A_UFEO_DRS*/

typedef volatile union _CAM_REG_UFEO_XSIZE_
{
		volatile struct	/* 0x1A004320 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_XSIZE;	/* CAM_UFEO_XSIZE, CAM_A_UFEO_XSIZE*/

typedef volatile union _CAM_REG_UFEO_YSIZE_
{
		volatile struct	/* 0x1A004324 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_YSIZE;	/* CAM_UFEO_YSIZE, CAM_A_UFEO_YSIZE*/

typedef volatile union _CAM_REG_UFEO_STRIDE_
{
		volatile struct	/* 0x1A004328 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_STRIDE;	/* CAM_UFEO_STRIDE, CAM_A_UFEO_STRIDE*/

typedef volatile union _CAM_REG_UFEO_CON_
{
		volatile struct	/* 0x1A00432C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_CON;	/* CAM_UFEO_CON, CAM_A_UFEO_CON*/

typedef volatile union _CAM_REG_UFEO_CON2_
{
		volatile struct	/* 0x1A004330 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_CON2;	/* CAM_UFEO_CON2, CAM_A_UFEO_CON2*/

typedef volatile union _CAM_REG_UFEO_CON3_
{
		volatile struct	/* 0x1A004334 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_CON3;	/* CAM_UFEO_CON3, CAM_A_UFEO_CON3*/

typedef volatile union _CAM_REG_UFEO_CON4_
{
		volatile struct	/* 0x1A00433C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_CON4;	/* CAM_UFEO_CON4, CAM_A_UFEO_CON4*/

typedef volatile union _CAM_REG_PDO_BASE_ADDR_
{
		volatile struct	/* 0x1A004340 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_BASE_ADDR;	/* CAM_PDO_BASE_ADDR, CAM_A_PDO_BASE_ADDR*/

typedef volatile union _CAM_REG_PDO_OFST_ADDR_
{
		volatile struct	/* 0x1A004348 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_OFST_ADDR;	/* CAM_PDO_OFST_ADDR, CAM_A_PDO_OFST_ADDR*/

typedef volatile union _CAM_REG_PDO_DRS_
{
		volatile struct	/* 0x1A00434C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_DRS;	/* CAM_PDO_DRS, CAM_A_PDO_DRS*/

typedef volatile union _CAM_REG_PDO_XSIZE_
{
		volatile struct	/* 0x1A004350 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_XSIZE;	/* CAM_PDO_XSIZE, CAM_A_PDO_XSIZE*/

typedef volatile union _CAM_REG_PDO_YSIZE_
{
		volatile struct	/* 0x1A004354 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_YSIZE;	/* CAM_PDO_YSIZE, CAM_A_PDO_YSIZE*/

typedef volatile union _CAM_REG_PDO_STRIDE_
{
		volatile struct	/* 0x1A004358 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_STRIDE;	/* CAM_PDO_STRIDE, CAM_A_PDO_STRIDE*/

typedef volatile union _CAM_REG_PDO_CON_
{
		volatile struct	/* 0x1A00435C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_CON;	/* CAM_PDO_CON, CAM_A_PDO_CON*/

typedef volatile union _CAM_REG_PDO_CON2_
{
		volatile struct	/* 0x1A004360 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_CON2;	/* CAM_PDO_CON2, CAM_A_PDO_CON2*/

typedef volatile union _CAM_REG_PDO_CON3_
{
		volatile struct	/* 0x1A004364 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_CON3;	/* CAM_PDO_CON3, CAM_A_PDO_CON3*/

typedef volatile union _CAM_REG_PDO_CON4_
{
		volatile struct	/* 0x1A00436C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_CON4;	/* CAM_PDO_CON4, CAM_A_PDO_CON4*/

typedef volatile union _CAM_REG_BPCI_BASE_ADDR_
{
		volatile struct	/* 0x1A004370 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_BASE_ADDR;	/* CAM_BPCI_BASE_ADDR, CAM_A_BPCI_BASE_ADDR*/

typedef volatile union _CAM_REG_BPCI_OFST_ADDR_
{
		volatile struct	/* 0x1A004378 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_OFST_ADDR;	/* CAM_BPCI_OFST_ADDR, CAM_A_BPCI_OFST_ADDR*/

typedef volatile union _CAM_REG_BPCI_DRS_
{
		volatile struct	/* 0x1A00437C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_DRS;	/* CAM_BPCI_DRS, CAM_A_BPCI_DRS*/

typedef volatile union _CAM_REG_BPCI_XSIZE_
{
		volatile struct	/* 0x1A004380 */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_XSIZE;	/* CAM_BPCI_XSIZE, CAM_A_BPCI_XSIZE*/

typedef volatile union _CAM_REG_BPCI_YSIZE_
{
		volatile struct	/* 0x1A004384 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_YSIZE;	/* CAM_BPCI_YSIZE, CAM_A_BPCI_YSIZE*/

typedef volatile union _CAM_REG_BPCI_STRIDE_
{
		volatile struct	/* 0x1A004388 */
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
}CAM_REG_BPCI_STRIDE;	/* CAM_BPCI_STRIDE, CAM_A_BPCI_STRIDE*/

typedef volatile union _CAM_REG_BPCI_CON_
{
		volatile struct	/* 0x1A00438C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON;	/* CAM_BPCI_CON, CAM_A_BPCI_CON*/

typedef volatile union _CAM_REG_BPCI_CON2_
{
		volatile struct	/* 0x1A004390 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON2;	/* CAM_BPCI_CON2, CAM_A_BPCI_CON2*/

typedef volatile union _CAM_REG_BPCI_CON3_
{
		volatile struct	/* 0x1A004394 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON3;	/* CAM_BPCI_CON3, CAM_A_BPCI_CON3*/

typedef volatile union _CAM_REG_BPCI_CON4_
{
		volatile struct	/* 0x1A00439C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_CON4;	/* CAM_BPCI_CON4, CAM_A_BPCI_CON4*/

typedef volatile union _CAM_REG_CACI_BASE_ADDR_
{
		volatile struct	/* 0x1A0043A0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_BASE_ADDR;	/* CAM_CACI_BASE_ADDR, CAM_A_CACI_BASE_ADDR*/

typedef volatile union _CAM_REG_CACI_OFST_ADDR_
{
		volatile struct	/* 0x1A0043A8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_OFST_ADDR;	/* CAM_CACI_OFST_ADDR, CAM_A_CACI_OFST_ADDR*/

typedef volatile union _CAM_REG_CACI_DRS_
{
		volatile struct	/* 0x1A0043AC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_DRS;	/* CAM_CACI_DRS, CAM_A_CACI_DRS*/

typedef volatile union _CAM_REG_CACI_XSIZE_
{
		volatile struct	/* 0x1A0043B0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_XSIZE;	/* CAM_CACI_XSIZE, CAM_A_CACI_XSIZE*/

typedef volatile union _CAM_REG_CACI_YSIZE_
{
		volatile struct	/* 0x1A0043B4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_YSIZE;	/* CAM_CACI_YSIZE, CAM_A_CACI_YSIZE*/

typedef volatile union _CAM_REG_CACI_STRIDE_
{
		volatile struct	/* 0x1A0043B8 */
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
}CAM_REG_CACI_STRIDE;	/* CAM_CACI_STRIDE, CAM_A_CACI_STRIDE*/

typedef volatile union _CAM_REG_CACI_CON_
{
		volatile struct	/* 0x1A0043BC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_CON;	/* CAM_CACI_CON, CAM_A_CACI_CON*/

typedef volatile union _CAM_REG_CACI_CON2_
{
		volatile struct	/* 0x1A0043C0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_CON2;	/* CAM_CACI_CON2, CAM_A_CACI_CON2*/

typedef volatile union _CAM_REG_CACI_CON3_
{
		volatile struct	/* 0x1A0043C4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_CON3;	/* CAM_CACI_CON3, CAM_A_CACI_CON3*/

typedef volatile union _CAM_REG_CACI_CON4_
{
		volatile struct	/* 0x1A0043CC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_CON4;	/* CAM_CACI_CON4, CAM_A_CACI_CON4*/

typedef volatile union _CAM_REG_LSCI_BASE_ADDR_
{
		volatile struct	/* 0x1A0043D0 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_BASE_ADDR;	/* CAM_LSCI_BASE_ADDR, CAM_A_LSCI_BASE_ADDR*/

typedef volatile union _CAM_REG_LSCI_OFST_ADDR_
{
		volatile struct	/* 0x1A0043D8 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_OFST_ADDR;	/* CAM_LSCI_OFST_ADDR, CAM_A_LSCI_OFST_ADDR*/

typedef volatile union _CAM_REG_LSCI_DRS_
{
		volatile struct	/* 0x1A0043DC */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_DRS;	/* CAM_LSCI_DRS, CAM_A_LSCI_DRS*/

typedef volatile union _CAM_REG_LSCI_XSIZE_
{
		volatile struct	/* 0x1A0043E0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_XSIZE;	/* CAM_LSCI_XSIZE, CAM_A_LSCI_XSIZE*/

typedef volatile union _CAM_REG_LSCI_YSIZE_
{
		volatile struct	/* 0x1A0043E4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_YSIZE;	/* CAM_LSCI_YSIZE, CAM_A_LSCI_YSIZE*/

typedef volatile union _CAM_REG_LSCI_STRIDE_
{
		volatile struct	/* 0x1A0043E8 */
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
}CAM_REG_LSCI_STRIDE;	/* CAM_LSCI_STRIDE, CAM_A_LSCI_STRIDE*/

typedef volatile union _CAM_REG_LSCI_CON_
{
		volatile struct	/* 0x1A0043EC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON;	/* CAM_LSCI_CON, CAM_A_LSCI_CON*/

typedef volatile union _CAM_REG_LSCI_CON2_
{
		volatile struct	/* 0x1A0043F0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON2;	/* CAM_LSCI_CON2, CAM_A_LSCI_CON2*/

typedef volatile union _CAM_REG_LSCI_CON3_
{
		volatile struct	/* 0x1A0043F4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON3;	/* CAM_LSCI_CON3, CAM_A_LSCI_CON3*/

typedef volatile union _CAM_REG_LSCI_CON4_
{
		volatile struct	/* 0x1A0043FC */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_CON4;	/* CAM_LSCI_CON4, CAM_A_LSCI_CON4*/

typedef volatile union _CAM_REG_LSC3I_BASE_ADDR_
{
		volatile struct	/* 0x1A004400 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_BASE_ADDR;	/* CAM_LSC3I_BASE_ADDR, CAM_A_LSC3I_BASE_ADDR*/

typedef volatile union _CAM_REG_LSC3I_OFST_ADDR_
{
		volatile struct	/* 0x1A004408 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_OFST_ADDR;	/* CAM_LSC3I_OFST_ADDR, CAM_A_LSC3I_OFST_ADDR*/

typedef volatile union _CAM_REG_LSC3I_DRS_
{
		volatile struct	/* 0x1A00440C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_DRS;	/* CAM_LSC3I_DRS, CAM_A_LSC3I_DRS*/

typedef volatile union _CAM_REG_LSC3I_XSIZE_
{
		volatile struct	/* 0x1A004410 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_XSIZE;	/* CAM_LSC3I_XSIZE, CAM_A_LSC3I_XSIZE*/

typedef volatile union _CAM_REG_LSC3I_YSIZE_
{
		volatile struct	/* 0x1A004414 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_YSIZE;	/* CAM_LSC3I_YSIZE, CAM_A_LSC3I_YSIZE*/

typedef volatile union _CAM_REG_LSC3I_STRIDE_
{
		volatile struct	/* 0x1A004418 */
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
}CAM_REG_LSC3I_STRIDE;	/* CAM_LSC3I_STRIDE, CAM_A_LSC3I_STRIDE*/

typedef volatile union _CAM_REG_LSC3I_CON_
{
		volatile struct	/* 0x1A00441C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_CON;	/* CAM_LSC3I_CON, CAM_A_LSC3I_CON*/

typedef volatile union _CAM_REG_LSC3I_CON2_
{
		volatile struct	/* 0x1A004420 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_CON2;	/* CAM_LSC3I_CON2, CAM_A_LSC3I_CON2*/

typedef volatile union _CAM_REG_LSC3I_CON3_
{
		volatile struct	/* 0x1A004424 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_CON3;	/* CAM_LSC3I_CON3, CAM_A_LSC3I_CON3*/

typedef volatile union _CAM_REG_LSC3I_CON4_
{
		volatile struct	/* 0x1A00442C */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_CON4;	/* CAM_LSC3I_CON4, CAM_A_LSC3I_CON4*/

typedef volatile union _CAM_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x1A004440 */
		{
				FIELD  IMGO_A_ERR                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_A_ERR                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_A_ERR                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_A_ERR                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_A_ERR                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_A_ERR                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_A_ERR                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  PSO_A_ERR                             :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  7;		/*  8..14, 0x00007F00 */
				FIELD  PDI_A_ERR                             :  1;		/* 15..15, 0x00008000 */
				FIELD  BPCI_A_ERR                            :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_A_ERR                            :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_A_ERR                            :  1;		/* 18..18, 0x00040000 */
				FIELD  LSC3I_A_ERR                           :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_ERR_CTRL;	/* CAM_DMA_ERR_CTRL, CAM_A_DMA_ERR_CTRL*/

typedef volatile union _CAM_REG_IMGO_ERR_STAT_
{
		volatile struct	/* 0x1A004444 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_ERR_STAT;	/* CAM_IMGO_ERR_STAT, CAM_A_IMGO_ERR_STAT*/

typedef volatile union _CAM_REG_RRZO_ERR_STAT_
{
		volatile struct	/* 0x1A004448 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_ERR_STAT;	/* CAM_RRZO_ERR_STAT, CAM_A_RRZO_ERR_STAT*/

typedef volatile union _CAM_REG_AAO_ERR_STAT_
{
		volatile struct	/* 0x1A00444C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_ERR_STAT;	/* CAM_AAO_ERR_STAT, CAM_A_AAO_ERR_STAT*/

typedef volatile union _CAM_REG_AFO_ERR_STAT_
{
		volatile struct	/* 0x1A004450 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_ERR_STAT;	/* CAM_AFO_ERR_STAT, CAM_A_AFO_ERR_STAT*/

typedef volatile union _CAM_REG_LCSO_ERR_STAT_
{
		volatile struct	/* 0x1A004454 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_ERR_STAT;	/* CAM_LCSO_ERR_STAT, CAM_A_LCSO_ERR_STAT*/

typedef volatile union _CAM_REG_UFEO_ERR_STAT_
{
		volatile struct	/* 0x1A004458 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_ERR_STAT;	/* CAM_UFEO_ERR_STAT, CAM_A_UFEO_ERR_STAT*/

typedef volatile union _CAM_REG_PDO_ERR_STAT_
{
		volatile struct	/* 0x1A00445C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_ERR_STAT;	/* CAM_PDO_ERR_STAT, CAM_A_PDO_ERR_STAT*/

typedef volatile union _CAM_REG_BPCI_ERR_STAT_
{
		volatile struct	/* 0x1A004460 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BPCI_ERR_STAT;	/* CAM_BPCI_ERR_STAT, CAM_A_BPCI_ERR_STAT*/

typedef volatile union _CAM_REG_CACI_ERR_STAT_
{
		volatile struct	/* 0x1A004464 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CACI_ERR_STAT;	/* CAM_CACI_ERR_STAT, CAM_A_CACI_ERR_STAT*/

typedef volatile union _CAM_REG_LSCI_ERR_STAT_
{
		volatile struct	/* 0x1A004468 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSCI_ERR_STAT;	/* CAM_LSCI_ERR_STAT, CAM_A_LSCI_ERR_STAT*/

typedef volatile union _CAM_REG_LSC3I_ERR_STAT_
{
		volatile struct	/* 0x1A00446C */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC3I_ERR_STAT;	/* CAM_LSC3I_ERR_STAT, CAM_A_LSC3I_ERR_STAT*/

typedef volatile union _CAM_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1A004470 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_DEBUG_ADDR;	/* CAM_DMA_DEBUG_ADDR, CAM_A_DMA_DEBUG_ADDR*/

typedef volatile union _CAM_REG_DMA_RSV1_
{
		volatile struct	/* 0x1A004474 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV1;	/* CAM_DMA_RSV1, CAM_A_DMA_RSV1*/

typedef volatile union _CAM_REG_DMA_RSV2_
{
		volatile struct	/* 0x1A004478 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV2;	/* CAM_DMA_RSV2, CAM_A_DMA_RSV2*/

typedef volatile union _CAM_REG_DMA_RSV3_
{
		volatile struct	/* 0x1A00447C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV3;	/* CAM_DMA_RSV3, CAM_A_DMA_RSV3*/

typedef volatile union _CAM_REG_DMA_RSV4_
{
		volatile struct	/* 0x1A004480 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV4;	/* CAM_DMA_RSV4, CAM_A_DMA_RSV4*/

typedef volatile union _CAM_REG_DMA_RSV5_
{
		volatile struct	/* 0x1A004484 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV5;	/* CAM_DMA_RSV5, CAM_A_DMA_RSV5*/

typedef volatile union _CAM_REG_DMA_RSV6_
{
		volatile struct	/* 0x1A004488 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_RSV6;	/* CAM_DMA_RSV6, CAM_A_DMA_RSV6*/

typedef volatile union _CAM_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x1A00448C */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PDO_FIFO_FULL_XSIZE                   :  1;		/* 28..28, 0x10000000 */
				FIELD  IMGO_UFE_FIFO_FULL_XSIZE              :  1;		/* 29..29, 0x20000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_DEBUG_SEL;	/* CAM_DMA_DEBUG_SEL, CAM_A_DMA_DEBUG_SEL*/

typedef volatile union _CAM_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1A004490 */
		{
				FIELD  BW_SELF_TEST_EN_IMGO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_RRZO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_AAO                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_AFO                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  BW_SELF_TEST_EN_LCSO                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BW_SELF_TEST_EN_UFEO                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BW_SELF_TEST_EN_PDO                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  BW_SELF_TEST_EN_PSO                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_BW_SELF_TEST;	/* CAM_DMA_BW_SELF_TEST, CAM_A_DMA_BW_SELF_TEST*/

typedef volatile union _CAM_REG_TG_SEN_MODE_
{
		volatile struct	/* 0x1A004500 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  DBL_DATA_BUS1                         :  1;		/* 13..13, 0x00002000 */
				FIELD  SOF_WAIT_CQ                           :  1;		/* 14..14, 0x00004000 */
				FIELD  FIFO_FULL_CTL_EN                      :  1;		/* 15..15, 0x00008000 */
				FIELD  TIME_STP_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  VS_SUB_EN                             :  1;		/* 17..17, 0x00020000 */
				FIELD  SOF_SUB_EN                            :  1;		/* 18..18, 0x00040000 */
				FIELD  I2C_CQ_EN                             :  1;		/* 19..19, 0x00080000 */
				FIELD  EOF_ALS_RDY_EN                        :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_SEL                                :  1;		/* 21..21, 0x00200000 */
				FIELD  TG_STAGER_SENSOR_EN                   :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_MODE;	/* CAM_TG_SEN_MODE, CAM_A_TG_SEN_MODE*/

typedef volatile union _CAM_REG_TG_VF_CON_
{
		volatile struct	/* 0x1A004504 */
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
}CAM_REG_TG_VF_CON;	/* CAM_TG_VF_CON, CAM_A_TG_VF_CON*/

typedef volatile union _CAM_REG_TG_SEN_GRAB_PXL_
{
		volatile struct	/* 0x1A004508 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_PXL;	/* CAM_TG_SEN_GRAB_PXL, CAM_A_TG_SEN_GRAB_PXL*/

typedef volatile union _CAM_REG_TG_SEN_GRAB_LIN_
{
		volatile struct	/* 0x1A00450C */
		{
				FIELD  LIN_S                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LIN_E                                 : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SEN_GRAB_LIN;	/* CAM_TG_SEN_GRAB_LIN, CAM_A_TG_SEN_GRAB_LIN*/

typedef volatile union _CAM_REG_TG_PATH_CFG_
{
		volatile struct	/* 0x1A004510 */
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
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  TG_ERR_SEL                            :  1;		/* 14..14, 0x00004000 */
				FIELD  TG_FULL_SEL                           :  1;		/* 15..15, 0x00008000 */
				FIELD  TG_FULL_SEL2                          :  1;		/* 16..16, 0x00010000 */
				FIELD  FLUSH_DISABLE                         :  1;		/* 17..17, 0x00020000 */
				FIELD  INT_BANK_DISABLE                      :  1;		/* 18..18, 0x00040000 */
				FIELD  EXP_ESC                               :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_PATH_CFG;	/* CAM_TG_PATH_CFG, CAM_A_TG_PATH_CFG*/

typedef volatile union _CAM_REG_TG_MEMIN_CTL_
{
		volatile struct	/* 0x1A004514 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_MEMIN_CTL;	/* CAM_TG_MEMIN_CTL, CAM_A_TG_MEMIN_CTL*/

typedef volatile union _CAM_REG_TG_INT1_
{
		volatile struct	/* 0x1A004518 */
		{
				FIELD  TG_INT1_LINENO                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_INT1;	/* CAM_TG_INT1, CAM_A_TG_INT1*/

typedef volatile union _CAM_REG_TG_INT2_
{
		volatile struct	/* 0x1A00451C */
		{
				FIELD  TG_INT2_LINENO                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_INT2;	/* CAM_TG_INT2, CAM_A_TG_INT2*/

typedef volatile union _CAM_REG_TG_SOF_CNT_
{
		volatile struct	/* 0x1A004520 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SOF_CNT;	/* CAM_TG_SOF_CNT, CAM_A_TG_SOF_CNT*/

typedef volatile union _CAM_REG_TG_SOT_CNT_
{
		volatile struct	/* 0x1A004524 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SOT_CNT;	/* CAM_TG_SOT_CNT, CAM_A_TG_SOT_CNT*/

typedef volatile union _CAM_REG_TG_EOT_CNT_
{
		volatile struct	/* 0x1A004528 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_EOT_CNT;	/* CAM_TG_EOT_CNT, CAM_A_TG_EOT_CNT*/

typedef volatile union _CAM_REG_TG_ERR_CTL_
{
		volatile struct	/* 0x1A00452C */
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
}CAM_REG_TG_ERR_CTL;	/* CAM_TG_ERR_CTL, CAM_A_TG_ERR_CTL*/

typedef volatile union _CAM_REG_TG_DAT_NO_
{
		volatile struct	/* 0x1A004530 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_DAT_NO;	/* CAM_TG_DAT_NO, CAM_A_TG_DAT_NO*/

typedef volatile union _CAM_REG_TG_FRM_CNT_ST_
{
		volatile struct	/* 0x1A004534 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FRM_CNT_ST;	/* CAM_TG_FRM_CNT_ST, CAM_A_TG_FRM_CNT_ST*/

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_
{
		volatile struct	/* 0x1A004538 */
		{
				FIELD  LINE_CNT                              : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FRMSIZE_ST;	/* CAM_TG_FRMSIZE_ST, CAM_A_TG_FRMSIZE_ST*/

typedef volatile union _CAM_REG_TG_INTER_ST_
{
		volatile struct	/* 0x1A00453C */
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
}CAM_REG_TG_INTER_ST;	/* CAM_TG_INTER_ST, CAM_A_TG_INTER_ST*/

typedef volatile union _CAM_REG_TG_FLASHA_CTL_
{
		volatile struct	/* 0x1A004540 */
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
}CAM_REG_TG_FLASHA_CTL;	/* CAM_TG_FLASHA_CTL, CAM_A_TG_FLASHA_CTL*/

typedef volatile union _CAM_REG_TG_FLASHA_LINE_CNT_
{
		volatile struct	/* 0x1A004544 */
		{
				FIELD  FLASHA_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHA_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHA_LINE_CNT;	/* CAM_TG_FLASHA_LINE_CNT, CAM_A_TG_FLASHA_LINE_CNT*/

typedef volatile union _CAM_REG_TG_FLASHA_POS_
{
		volatile struct	/* 0x1A004548 */
		{
				FIELD  FLASHA_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHA_LINE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHA_POS;	/* CAM_TG_FLASHA_POS, CAM_A_TG_FLASHA_POS*/

typedef volatile union _CAM_REG_TG_FLASHB_CTL_
{
		volatile struct	/* 0x1A00454C */
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
}CAM_REG_TG_FLASHB_CTL;	/* CAM_TG_FLASHB_CTL, CAM_A_TG_FLASHB_CTL*/

typedef volatile union _CAM_REG_TG_FLASHB_LINE_CNT_
{
		volatile struct	/* 0x1A004550 */
		{
				FIELD  FLASHB_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHB_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_LINE_CNT;	/* CAM_TG_FLASHB_LINE_CNT, CAM_A_TG_FLASHB_LINE_CNT*/

typedef volatile union _CAM_REG_TG_FLASHB_POS_
{
		volatile struct	/* 0x1A004554 */
		{
				FIELD  FLASHB_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHB_LINE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_POS;	/* CAM_TG_FLASHB_POS, CAM_A_TG_FLASHB_POS*/

typedef volatile union _CAM_REG_TG_FLASHB_POS1_
{
		volatile struct	/* 0x1A004558 */
		{
				FIELD  FLASHB_CYC_CNT                        : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FLASHB_POS1;	/* CAM_TG_FLASHB_POS1, CAM_A_TG_FLASHB_POS1*/

typedef volatile union _CAM_REG_TG_I2C_CQ_TRIG_
{
		volatile struct	/* 0x1A004560 */
		{
				FIELD  TG_I2C_CQ_TRIG                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_I2C_CQ_TRIG;	/* CAM_TG_I2C_CQ_TRIG, CAM_A_TG_I2C_CQ_TRIG*/

typedef volatile union _CAM_REG_TG_CQ_TIMING_
{
		volatile struct	/* 0x1A004564 */
		{
				FIELD  TG_I2C_CQ_TIM                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_CQ_TIMING;	/* CAM_TG_CQ_TIMING, CAM_A_TG_CQ_TIMING*/

typedef volatile union _CAM_REG_TG_CQ_NUM_
{
		volatile struct	/* 0x1A004568 */
		{
				FIELD  TG_CQ_NUM                             :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_CQ_NUM;	/* CAM_TG_CQ_NUM, CAM_A_TG_CQ_NUM*/

typedef volatile union _CAM_REG_TG_TIME_STAMP_
{
		volatile struct	/* 0x1A004570 */
		{
				FIELD  TG_TIME_STAMP                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_TIME_STAMP;	/* CAM_TG_TIME_STAMP, CAM_A_TG_TIME_STAMP*/

typedef volatile union _CAM_REG_TG_SUB_PERIOD_
{
		volatile struct	/* 0x1A004574 */
		{
				FIELD  VS_PERIOD                             :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SOF_PERIOD                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_SUB_PERIOD;	/* CAM_TG_SUB_PERIOD, CAM_A_TG_SUB_PERIOD*/

typedef volatile union _CAM_REG_TG_DAT_NO_R_
{
		volatile struct	/* 0x1A004578 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_DAT_NO_R;	/* CAM_TG_DAT_NO_R, CAM_A_TG_DAT_NO_R*/

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_R_
{
		volatile struct	/* 0x1A00457C */
		{
				FIELD  LINE_CNT                              : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_TG_FRMSIZE_ST_R;	/* CAM_TG_FRMSIZE_ST_R, CAM_A_TG_FRMSIZE_ST_R*/

typedef volatile union _CAM_REG_DMX_CTL_
{
		volatile struct	/* 0x1A004580 */
		{
				FIELD  DMX_SRAM_SIZE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  DMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 11;		/* 20..30, 0x7FF00000 */
				FIELD  DMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_CTL;	/* CAM_DMX_CTL, CAM_A_DMX_CTL*/

typedef volatile union _CAM_REG_DMX_CROP_
{
		volatile struct	/* 0x1A004584 */
		{
				FIELD  DMX_STR_X                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  DMX_END_X                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_CROP;	/* CAM_DMX_CROP, CAM_A_DMX_CROP*/

typedef volatile union _CAM_REG_DMX_VSIZE_
{
		volatile struct	/* 0x1A004588 */
		{
				FIELD  DMX_HT                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMX_VSIZE;	/* CAM_DMX_VSIZE, CAM_A_DMX_VSIZE*/

typedef volatile union _CAM_REG_RMG_HDR_CFG_
{
		volatile struct	/* 0x1A0045A0 */
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
}CAM_REG_RMG_HDR_CFG;	/* CAM_RMG_HDR_CFG, CAM_A_RMG_HDR_CFG*/

typedef volatile union _CAM_REG_RMG_HDR_GAIN_
{
		volatile struct	/* 0x1A0045A4 */
		{
				FIELD  RMG_HDR_GAIN                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  RMG_HDR_RATIO                         :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  RMG_LE_INV_CTL                        :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMG_HDR_GAIN;	/* CAM_RMG_HDR_GAIN, CAM_A_RMG_HDR_GAIN*/

typedef volatile union _CAM_REG_RMG_HDR_CFG2_
{
		volatile struct	/* 0x1A0045A8 */
		{
				FIELD  RMG_HDR_THK                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMG_HDR_CFG2;	/* CAM_RMG_HDR_CFG2, CAM_A_RMG_HDR_CFG2*/

typedef volatile union _CAM_REG_RMM_OSC_
{
		volatile struct	/* 0x1A0045C0 */
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
}CAM_REG_RMM_OSC;	/* CAM_RMM_OSC, CAM_A_RMM_OSC*/

typedef volatile union _CAM_REG_RMM_MC_
{
		volatile struct	/* 0x1A0045C4 */
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
}CAM_REG_RMM_MC;	/* CAM_RMM_MC, CAM_A_RMM_MC*/

typedef volatile union _CAM_REG_RMM_REVG_1_
{
		volatile struct	/* 0x1A0045C8 */
		{
				FIELD  RMM_REVG_R                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  RMM_REVG_GR                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMM_REVG_1;	/* CAM_RMM_REVG_1, CAM_A_RMM_REVG_1*/

typedef volatile union _CAM_REG_RMM_REVG_2_
{
		volatile struct	/* 0x1A0045CC */
		{
				FIELD  RMM_REVG_B                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  RMM_REVG_GB                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMM_REVG_2;	/* CAM_RMM_REVG_2, CAM_A_RMM_REVG_2*/

typedef volatile union _CAM_REG_RMM_LEOS_
{
		volatile struct	/* 0x1A0045D0 */
		{
				FIELD  RMM_LEOS_GRAY                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMM_LEOS;	/* CAM_RMM_LEOS, CAM_A_RMM_LEOS*/

typedef volatile union _CAM_REG_RMM_MC2_
{
		volatile struct	/* 0x1A0045D4 */
		{
				FIELD  RMM_MOSE_TH                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RMM_MOSE_BLDWD                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMM_MC2;	/* CAM_RMM_MC2, CAM_A_RMM_MC2*/

typedef volatile union _CAM_REG_RMM_DIFF_LB_
{
		volatile struct	/* 0x1A0045D8 */
		{
				FIELD  RMM_DIFF_LB                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMM_DIFF_LB;	/* CAM_RMM_DIFF_LB, CAM_A_RMM_DIFF_LB*/

typedef volatile union _CAM_REG_RMM_MA_
{
		volatile struct	/* 0x1A0045DC */
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
}CAM_REG_RMM_MA;	/* CAM_RMM_MA, CAM_A_RMM_MA*/

typedef volatile union _CAM_REG_RMM_TUNE_
{
		volatile struct	/* 0x1A0045E0 */
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
}CAM_REG_RMM_TUNE;	/* CAM_RMM_TUNE, CAM_A_RMM_TUNE*/

typedef volatile union _CAM_REG_OBC_OFFST0_
{
		volatile struct	/* 0x1A0045F0 */
		{
				FIELD  OBC_OFST_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST0;	/* CAM_OBC_OFFST0, CAM_A_OBC_OFFST0*/

typedef volatile union _CAM_REG_OBC_OFFST1_
{
		volatile struct	/* 0x1A0045F4 */
		{
				FIELD  OBC_OFST_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST1;	/* CAM_OBC_OFFST1, CAM_A_OBC_OFFST1*/

typedef volatile union _CAM_REG_OBC_OFFST2_
{
		volatile struct	/* 0x1A0045F8 */
		{
				FIELD  OBC_OFST_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST2;	/* CAM_OBC_OFFST2, CAM_A_OBC_OFFST2*/

typedef volatile union _CAM_REG_OBC_OFFST3_
{
		volatile struct	/* 0x1A0045FC */
		{
				FIELD  OBC_OFST_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_OFFST3;	/* CAM_OBC_OFFST3, CAM_A_OBC_OFFST3*/

typedef volatile union _CAM_REG_OBC_GAIN0_
{
		volatile struct	/* 0x1A004600 */
		{
				FIELD  OBC_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN0;	/* CAM_OBC_GAIN0, CAM_A_OBC_GAIN0*/

typedef volatile union _CAM_REG_OBC_GAIN1_
{
		volatile struct	/* 0x1A004604 */
		{
				FIELD  OBC_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN1;	/* CAM_OBC_GAIN1, CAM_A_OBC_GAIN1*/

typedef volatile union _CAM_REG_OBC_GAIN2_
{
		volatile struct	/* 0x1A004608 */
		{
				FIELD  OBC_GAIN_GB                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN2;	/* CAM_OBC_GAIN2, CAM_A_OBC_GAIN2*/

typedef volatile union _CAM_REG_OBC_GAIN3_
{
		volatile struct	/* 0x1A00460C */
		{
				FIELD  OBC_GAIN_R                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_OBC_GAIN3;	/* CAM_OBC_GAIN3, CAM_A_OBC_GAIN3*/

typedef volatile union _CAM_REG_BNR_BPC_CON_
{
		volatile struct	/* 0x1A004620 */
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
}CAM_REG_BNR_BPC_CON;	/* CAM_BNR_BPC_CON, CAM_A_BNR_BPC_CON*/

typedef volatile union _CAM_REG_BNR_BPC_TH1_
{
		volatile struct	/* 0x1A004624 */
		{
				FIELD  BPC_TH_LWB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_Y                              : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_BLD_SLP0                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH1;	/* CAM_BNR_BPC_TH1, CAM_A_BNR_BPC_TH1*/

typedef volatile union _CAM_REG_BNR_BPC_TH2_
{
		volatile struct	/* 0x1A004628 */
		{
				FIELD  BPC_TH_UPB                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  BPC_BLD0                              :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  BPC_BLD1                              :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH2;	/* CAM_BNR_BPC_TH2, CAM_A_BNR_BPC_TH2*/

typedef volatile union _CAM_REG_BNR_BPC_TH3_
{
		volatile struct	/* 0x1A00462C */
		{
				FIELD  BPC_TH_XA                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_TH_XB                             : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_TH_SLA                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_TH_SLB                            :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH3;	/* CAM_BNR_BPC_TH3, CAM_A_BNR_BPC_TH3*/

typedef volatile union _CAM_REG_BNR_BPC_TH4_
{
		volatile struct	/* 0x1A004630 */
		{
				FIELD  BPC_DK_TH_XA                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_DK_TH_XB                          : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_DK_TH_SLA                         :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_DK_TH_SLB                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH4;	/* CAM_BNR_BPC_TH4, CAM_A_BNR_BPC_TH4*/

typedef volatile union _CAM_REG_BNR_BPC_DTC_
{
		volatile struct	/* 0x1A004634 */
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
}CAM_REG_BNR_BPC_DTC;	/* CAM_BNR_BPC_DTC, CAM_A_BNR_BPC_DTC*/

typedef volatile union _CAM_REG_BNR_BPC_COR_
{
		volatile struct	/* 0x1A004638 */
		{
				FIELD  BPC_DIR_MAX                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BPC_DIR_TH                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BPC_RANK_IDXR                         :  3;		/* 16..18, 0x00070000 */
				FIELD  BPC_RANK_IDXG                         :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  BPC_DIR_TH2                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_COR;	/* CAM_BNR_BPC_COR, CAM_A_BNR_BPC_COR*/

typedef volatile union _CAM_REG_BNR_BPC_TBLI1_
{
		volatile struct	/* 0x1A00463C */
		{
				FIELD  BPC_XOFFSET                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YOFFSET                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI1;	/* CAM_BNR_BPC_TBLI1, CAM_A_BNR_BPC_TBLI1*/

typedef volatile union _CAM_REG_BNR_BPC_TBLI2_
{
		volatile struct	/* 0x1A004640 */
		{
				FIELD  BPC_XSIZE                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  BPC_YSIZE                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TBLI2;	/* CAM_BNR_BPC_TBLI2, CAM_A_BNR_BPC_TBLI2*/

typedef volatile union _CAM_REG_BNR_BPC_TH1_C_
{
		volatile struct	/* 0x1A004644 */
		{
				FIELD  BPC_C_TH_LWB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_Y                            : 12;		/* 12..23, 0x00FFF000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH1_C;	/* CAM_BNR_BPC_TH1_C, CAM_A_BNR_BPC_TH1_C*/

typedef volatile union _CAM_REG_BNR_BPC_TH2_C_
{
		volatile struct	/* 0x1A004648 */
		{
				FIELD  BPC_C_TH_UPB                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  1;		/* 12..12, 0x00001000 */
				FIELD  BPC_RANK_IDXB                         :  3;		/* 13..15, 0x0000E000 */
				FIELD  BPC_BLD_LWB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH2_C;	/* CAM_BNR_BPC_TH2_C, CAM_A_BNR_BPC_TH2_C*/

typedef volatile union _CAM_REG_BNR_BPC_TH3_C_
{
		volatile struct	/* 0x1A00464C */
		{
				FIELD  BPC_C_TH_XA                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  BPC_C_TH_XB                           : 12;		/* 12..23, 0x00FFF000 */
				FIELD  BPC_C_TH_SLA                          :  4;		/* 24..27, 0x0F000000 */
				FIELD  BPC_C_TH_SLB                          :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_BPC_TH3_C;	/* CAM_BNR_BPC_TH3_C, CAM_A_BNR_BPC_TH3_C*/

typedef volatile union _CAM_REG_BNR_NR1_CON_
{
		volatile struct	/* 0x1A004650 */
		{
				FIELD  rsv_0                                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  NR1_CT_EN                             :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_NR1_CON;	/* CAM_BNR_NR1_CON, CAM_A_BNR_NR1_CON*/

typedef volatile union _CAM_REG_BNR_NR1_CT_CON_
{
		volatile struct	/* 0x1A004654 */
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
}CAM_REG_BNR_NR1_CT_CON;	/* CAM_BNR_NR1_CT_CON, CAM_A_BNR_NR1_CT_CON*/

typedef volatile union _CAM_REG_BNR_RSV1_
{
		volatile struct	/* 0x1A004658 */
		{
				FIELD  RSV1                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_RSV1;	/* CAM_BNR_RSV1, CAM_A_BNR_RSV1*/

typedef volatile union _CAM_REG_BNR_RSV2_
{
		volatile struct	/* 0x1A00465C */
		{
				FIELD  RSV2                                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_RSV2;	/* CAM_BNR_RSV2, CAM_A_BNR_RSV2*/

typedef volatile union _CAM_REG_BNR_PDC_CON_
{
		volatile struct	/* 0x1A004660 */
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
}CAM_REG_BNR_PDC_CON;	/* CAM_BNR_PDC_CON, CAM_A_BNR_PDC_CON*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L0_
{
		volatile struct	/* 0x1A004664 */
		{
				FIELD  PDC_GCF_L00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L0;	/* CAM_BNR_PDC_GAIN_L0, CAM_A_BNR_PDC_GAIN_L0*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L1_
{
		volatile struct	/* 0x1A004668 */
		{
				FIELD  PDC_GCF_L01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L1;	/* CAM_BNR_PDC_GAIN_L1, CAM_A_BNR_PDC_GAIN_L1*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L2_
{
		volatile struct	/* 0x1A00466C */
		{
				FIELD  PDC_GCF_L11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L2;	/* CAM_BNR_PDC_GAIN_L2, CAM_A_BNR_PDC_GAIN_L2*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L3_
{
		volatile struct	/* 0x1A004670 */
		{
				FIELD  PDC_GCF_L30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L3;	/* CAM_BNR_PDC_GAIN_L3, CAM_A_BNR_PDC_GAIN_L3*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_L4_
{
		volatile struct	/* 0x1A004674 */
		{
				FIELD  PDC_GCF_L12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_L03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_L4;	/* CAM_BNR_PDC_GAIN_L4, CAM_A_BNR_PDC_GAIN_L4*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R0_
{
		volatile struct	/* 0x1A004678 */
		{
				FIELD  PDC_GCF_R00                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R10                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R0;	/* CAM_BNR_PDC_GAIN_R0, CAM_A_BNR_PDC_GAIN_R0*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R1_
{
		volatile struct	/* 0x1A00467C */
		{
				FIELD  PDC_GCF_R01                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R20                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R1;	/* CAM_BNR_PDC_GAIN_R1, CAM_A_BNR_PDC_GAIN_R1*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R2_
{
		volatile struct	/* 0x1A004680 */
		{
				FIELD  PDC_GCF_R11                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R02                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R2;	/* CAM_BNR_PDC_GAIN_R2, CAM_A_BNR_PDC_GAIN_R2*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R3_
{
		volatile struct	/* 0x1A004684 */
		{
				FIELD  PDC_GCF_R30                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R21                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R3;	/* CAM_BNR_PDC_GAIN_R3, CAM_A_BNR_PDC_GAIN_R3*/

typedef volatile union _CAM_REG_BNR_PDC_GAIN_R4_
{
		volatile struct	/* 0x1A004688 */
		{
				FIELD  PDC_GCF_R12                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_R03                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_GAIN_R4;	/* CAM_BNR_PDC_GAIN_R4, CAM_A_BNR_PDC_GAIN_R4*/

typedef volatile union _CAM_REG_BNR_PDC_TH_GB_
{
		volatile struct	/* 0x1A00468C */
		{
				FIELD  PDC_GTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_BTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_GB;	/* CAM_BNR_PDC_TH_GB, CAM_A_BNR_PDC_TH_GB*/

typedef volatile union _CAM_REG_BNR_PDC_TH_IA_
{
		volatile struct	/* 0x1A004690 */
		{
				FIELD  PDC_ITH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_ATH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_IA;	/* CAM_BNR_PDC_TH_IA, CAM_A_BNR_PDC_TH_IA*/

typedef volatile union _CAM_REG_BNR_PDC_TH_HD_
{
		volatile struct	/* 0x1A004694 */
		{
				FIELD  PDC_NTH                               : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_DTH                               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_TH_HD;	/* CAM_BNR_PDC_TH_HD, CAM_A_BNR_PDC_TH_HD*/

typedef volatile union _CAM_REG_BNR_PDC_SL_
{
		volatile struct	/* 0x1A004698 */
		{
				FIELD  PDC_GSL                               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  PDC_BSL                               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  PDC_ISL                               :  4;		/*  8..11, 0x00000F00 */
				FIELD  PDC_ASL                               :  4;		/* 12..15, 0x0000F000 */
				FIELD  PDC_GCF_NORM                          :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_SL;	/* CAM_BNR_PDC_SL, CAM_A_BNR_PDC_SL*/

typedef volatile union _CAM_REG_BNR_PDC_POS_
{
		volatile struct	/* 0x1A00469C */
		{
				FIELD  PDC_XCENTER                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDC_YCENTER                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BNR_PDC_POS;	/* CAM_BNR_PDC_POS, CAM_A_BNR_PDC_POS*/

typedef volatile union _CAM_REG_STM_CFG0_
{
		volatile struct	/* 0x1A0046A0 */
		{
				FIELD  STM_IHDR_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  STM_ZHDR_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_STM_CFG0;	/* CAM_STM_CFG0, CAM_A_STM_CFG0*/

typedef volatile union _CAM_REG_STM_CFG1_
{
		volatile struct	/* 0x1A0046A4 */
		{
				FIELD  STM_IHDR_LE_FIRST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  STM_ZHDR_RLE                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  STM_ZHDR_GLE                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  STM_ZHDR_BLE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  STM_LIN_SEL                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  STM_LE_INV_CTL                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_STM_CFG1;	/* CAM_STM_CFG1, CAM_A_STM_CFG1*/

typedef volatile union _CAM_REG_SCM_CFG0_
{
		volatile struct	/* 0x1A0046B0 */
		{
				FIELD  SCM_IHDR_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SCM_ZHDR_EN                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_SCM_CFG0;	/* CAM_SCM_CFG0, CAM_A_SCM_CFG0*/

typedef volatile union _CAM_REG_SCM_CFG1_
{
		volatile struct	/* 0x1A0046B4 */
		{
				FIELD  SCM_IHDR_LE_FIRST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SCM_ZHDR_RLE                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SCM_ZHDR_GLE                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SCM_ZHDR_BLE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SCM_LIN_SEL                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SCM_LE_INV_CTL                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SCM_CFG1;	/* CAM_SCM_CFG1, CAM_A_SCM_CFG1*/

typedef volatile union _CAM_REG_RPG_SATU_1_
{
		volatile struct	/* 0x1A0046C0 */
		{
				FIELD  RPG_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_SATU_1;	/* CAM_RPG_SATU_1, CAM_A_RPG_SATU_1*/

typedef volatile union _CAM_REG_RPG_SATU_2_
{
		volatile struct	/* 0x1A0046C4 */
		{
				FIELD  RPG_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_SATU_2;	/* CAM_RPG_SATU_2, CAM_A_RPG_SATU_2*/

typedef volatile union _CAM_REG_RPG_GAIN_1_
{
		volatile struct	/* 0x1A0046C8 */
		{
				FIELD  RPG_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_GAIN_1;	/* CAM_RPG_GAIN_1, CAM_A_RPG_GAIN_1*/

typedef volatile union _CAM_REG_RPG_GAIN_2_
{
		volatile struct	/* 0x1A0046CC */
		{
				FIELD  RPG_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RPG_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_GAIN_2;	/* CAM_RPG_GAIN_2, CAM_A_RPG_GAIN_2*/

typedef volatile union _CAM_REG_RPG_OFST_1_
{
		volatile struct	/* 0x1A0046D0 */
		{
				FIELD  RPG_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_OFST_1;	/* CAM_RPG_OFST_1, CAM_A_RPG_OFST_1*/

typedef volatile union _CAM_REG_RPG_OFST_2_
{
		volatile struct	/* 0x1A0046D4 */
		{
				FIELD  RPG_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  RPG_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RPG_OFST_2;	/* CAM_RPG_OFST_2, CAM_A_RPG_OFST_2*/

typedef volatile union _CAM_REG_RRZ_CTL_
{
		volatile struct	/* 0x1A0046E0 */
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
}CAM_REG_RRZ_CTL;	/* CAM_RRZ_CTL, CAM_A_RRZ_CTL*/

typedef volatile union _CAM_REG_RRZ_IN_IMG_
{
		volatile struct	/* 0x1A0046E4 */
		{
				FIELD  RRZ_IN_WD                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_IN_HT                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_IN_IMG;	/* CAM_RRZ_IN_IMG, CAM_A_RRZ_IN_IMG*/

typedef volatile union _CAM_REG_RRZ_OUT_IMG_
{
		volatile struct	/* 0x1A0046E8 */
		{
				FIELD  RRZ_OUT_WD                            : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RRZ_OUT_HT                            : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_OUT_IMG;	/* CAM_RRZ_OUT_IMG, CAM_A_RRZ_OUT_IMG*/

typedef volatile union _CAM_REG_RRZ_HORI_STEP_
{
		volatile struct	/* 0x1A0046EC */
		{
				FIELD  RRZ_HORI_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_STEP;	/* CAM_RRZ_HORI_STEP, CAM_A_RRZ_HORI_STEP*/

typedef volatile union _CAM_REG_RRZ_VERT_STEP_
{
		volatile struct	/* 0x1A0046F0 */
		{
				FIELD  RRZ_VERT_STEP                         : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_STEP;	/* CAM_RRZ_VERT_STEP, CAM_A_RRZ_VERT_STEP*/

typedef volatile union _CAM_REG_RRZ_HORI_INT_OFST_
{
		volatile struct	/* 0x1A0046F4 */
		{
				FIELD  RRZ_HORI_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_INT_OFST;	/* CAM_RRZ_HORI_INT_OFST, CAM_A_RRZ_HORI_INT_OFST*/

typedef volatile union _CAM_REG_RRZ_HORI_SUB_OFST_
{
		volatile struct	/* 0x1A0046F8 */
		{
				FIELD  RRZ_HORI_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_HORI_SUB_OFST;	/* CAM_RRZ_HORI_SUB_OFST, CAM_A_RRZ_HORI_SUB_OFST*/

typedef volatile union _CAM_REG_RRZ_VERT_INT_OFST_
{
		volatile struct	/* 0x1A0046FC */
		{
				FIELD  RRZ_VERT_INT_OFST                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_INT_OFST;	/* CAM_RRZ_VERT_INT_OFST, CAM_A_RRZ_VERT_INT_OFST*/

typedef volatile union _CAM_REG_RRZ_VERT_SUB_OFST_
{
		volatile struct	/* 0x1A004700 */
		{
				FIELD  RRZ_VERT_SUB_OFST                     : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_VERT_SUB_OFST;	/* CAM_RRZ_VERT_SUB_OFST, CAM_A_RRZ_VERT_SUB_OFST*/

typedef volatile union _CAM_REG_RRZ_MODE_TH_
{
		volatile struct	/* 0x1A004704 */
		{
				FIELD  RRZ_TH_MD                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RRZ_TH_HI                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RRZ_TH_LO                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RRZ_TH_MD2                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_MODE_TH;	/* CAM_RRZ_MODE_TH, CAM_A_RRZ_MODE_TH*/

typedef volatile union _CAM_REG_RRZ_MODE_CTL_
{
		volatile struct	/* 0x1A004708 */
		{
				FIELD  RRZ_PRF_BLD                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  RRZ_PRF                               :  2;		/*  9..10, 0x00000600 */
				FIELD  RRZ_BLD_SL                            :  5;		/* 11..15, 0x0000F800 */
				FIELD  RRZ_CR_MODE                           :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_MODE_CTL;	/* CAM_RRZ_MODE_CTL, CAM_A_RRZ_MODE_CTL*/

typedef volatile union _CAM_REG_RRZ_RLB_AOFST_
{
		volatile struct	/* 0x1A00470C */
		{
				FIELD  RRZ_RLB_AOFST                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZ_RLB_AOFST;	/* CAM_RRZ_RLB_AOFST, CAM_A_RRZ_RLB_AOFST*/

typedef volatile union _CAM_REG_RMX_CTL_
{
		volatile struct	/* 0x1A004740 */
		{
				FIELD  RMX_SRAM_SIZE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  RMX_SINGLE_MODE_1                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RMX_SINGLE_MODE_2                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  6;		/* 25..30, 0x7E000000 */
				FIELD  RMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_CTL;	/* CAM_RMX_CTL, CAM_A_RMX_CTL*/

typedef volatile union _CAM_REG_RMX_CROP_
{
		volatile struct	/* 0x1A004744 */
		{
				FIELD  RMX_STR_X                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  RMX_END_X                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_CROP;	/* CAM_RMX_CROP, CAM_A_RMX_CROP*/

typedef volatile union _CAM_REG_RMX_VSIZE_
{
		volatile struct	/* 0x1A004748 */
		{
				FIELD  RMX_HT                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RMX_VSIZE;	/* CAM_RMX_VSIZE, CAM_A_RMX_VSIZE*/

typedef volatile union _CAM_REG_SGG5_PGN_
{
		volatile struct	/* 0x1A004760 */
		{
				FIELD  SGG_GAIN                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_PGN;	/* CAM_SGG5_PGN, CAM_A_SGG5_PGN*/

typedef volatile union _CAM_REG_SGG5_GMRC_1_
{
		volatile struct	/* 0x1A004764 */
		{
				FIELD  SGG_GMR_1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG_GMR_4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_GMRC_1;	/* CAM_SGG5_GMRC_1, CAM_A_SGG5_GMRC_1*/

typedef volatile union _CAM_REG_SGG5_GMRC_2_
{
		volatile struct	/* 0x1A004768 */
		{
				FIELD  SGG_GMR_5                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_6                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_7                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG5_GMRC_2;	/* CAM_SGG5_GMRC_2, CAM_A_SGG5_GMRC_2*/

typedef volatile union _CAM_REG_BMX_CTL_
{
		volatile struct	/* 0x1A004780 */
		{
				FIELD  BMX_SRAM_SIZE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  BMX_SINGLE_MODE_1                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  BMX_SINGLE_MODE_2                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  6;		/* 25..30, 0x7E000000 */
				FIELD  BMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_CTL;	/* CAM_BMX_CTL, CAM_A_BMX_CTL*/

typedef volatile union _CAM_REG_BMX_CROP_
{
		volatile struct	/* 0x1A004784 */
		{
				FIELD  BMX_STR_X                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BMX_END_X                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_CROP;	/* CAM_BMX_CROP, CAM_A_BMX_CROP*/

typedef volatile union _CAM_REG_BMX_VSIZE_
{
		volatile struct	/* 0x1A004788 */
		{
				FIELD  BMX_HT                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BMX_VSIZE;	/* CAM_BMX_VSIZE, CAM_A_BMX_VSIZE*/

typedef volatile union _CAM_REG_UFE_CON_
{
		volatile struct	/* 0x1A0047C0 */
		{
				FIELD  UFE_FORCE_PCM                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  UFE_TCCT_BYP                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFE_CON;	/* CAM_UFE_CON, CAM_A_UFE_CON*/

typedef volatile union _CAM_REG_LSC_CTL1_
{
		volatile struct	/* 0x1A0047D0 */
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
}CAM_REG_LSC_CTL1;	/* CAM_LSC_CTL1, CAM_A_LSC_CTL1*/

typedef volatile union _CAM_REG_LSC_CTL2_
{
		volatile struct	/* 0x1A0047D4 */
		{
				FIELD  LSC_SDBLK_WIDTH                       : 13;		/*  0..12, 0x00001FFF */
				FIELD  LSC_SDBLK_XNUM                        :  5;		/* 13..17, 0x0003E000 */
				FIELD  LSC_OFLN                              :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_CTL2;	/* CAM_LSC_CTL2, CAM_A_LSC_CTL2*/

typedef volatile union _CAM_REG_LSC_CTL3_
{
		volatile struct	/* 0x1A0047D8 */
		{
				FIELD  LSC_SDBLK_HEIGHT                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  LSC_SDBLK_YNUM                        :  5;		/* 13..17, 0x0003E000 */
				FIELD  LSC_SPARE                             : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_CTL3;	/* CAM_LSC_CTL3, CAM_A_LSC_CTL3*/

typedef volatile union _CAM_REG_LSC_LBLOCK_
{
		volatile struct	/* 0x1A0047DC */
		{
				FIELD  LSC_SDBLK_lHEIGHT                     : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_SDBLK_lWIDTH                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_LBLOCK;	/* CAM_LSC_LBLOCK, CAM_A_LSC_LBLOCK*/

typedef volatile union _CAM_REG_LSC_RATIO_0_
{
		volatile struct	/* 0x1A0047E0 */
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
}CAM_REG_LSC_RATIO_0;	/* CAM_LSC_RATIO_0, CAM_A_LSC_RATIO_0*/

typedef volatile union _CAM_REG_LSC_TPIPE_OFST_
{
		volatile struct	/* 0x1A0047E4 */
		{
				FIELD  LSC_TPIPE_OFST_Y                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LSC_TPIPE_OFST_X                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_TPIPE_OFST;	/* CAM_LSC_TPIPE_OFST, CAM_A_LSC_TPIPE_OFST*/

typedef volatile union _CAM_REG_LSC_TPIPE_SIZE_
{
		volatile struct	/* 0x1A0047E8 */
		{
				FIELD  LSC_TPIPE_SIZE_Y                      : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LSC_TPIPE_SIZE_X                      : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_TPIPE_SIZE;	/* CAM_LSC_TPIPE_SIZE, CAM_A_LSC_TPIPE_SIZE*/

typedef volatile union _CAM_REG_LSC_GAIN_TH_
{
		volatile struct	/* 0x1A0047EC */
		{
				FIELD  LSC_GAIN_TH2                          :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  LSC_GAIN_TH1                          :  9;		/* 10..18, 0x0007FC00 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  LSC_GAIN_TH0                          :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LSC_GAIN_TH;	/* CAM_LSC_GAIN_TH, CAM_A_LSC_GAIN_TH*/

typedef volatile union _CAM_REG_LSC_RATIO_1_
{
		volatile struct	/* 0x1A0047F0 */
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
}CAM_REG_LSC_RATIO_1;	/* CAM_LSC_RATIO_1, CAM_A_LSC_RATIO_1*/

typedef volatile union _CAM_REG_AF_CON_
{
		volatile struct	/* 0x1A004800 */
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
}CAM_REG_AF_CON;	/* CAM_AF_CON, CAM_A_AF_CON*/

typedef volatile union _CAM_REG_AF_TH_0_
{
		volatile struct	/* 0x1A004804 */
		{
				FIELD  AF_H_TH_0                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_H_TH_1                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_0;	/* CAM_AF_TH_0, CAM_A_AF_TH_0*/

typedef volatile union _CAM_REG_AF_TH_1_
{
		volatile struct	/* 0x1A004808 */
		{
				FIELD  AF_V_TH                               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_R_SAT_TH                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_G_SAT_TH                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_B_SAT_TH                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_1;	/* CAM_AF_TH_1, CAM_A_AF_TH_1*/

typedef volatile union _CAM_REG_AF_FLT_1_
{
		volatile struct	/* 0x1A00480C */
		{
				FIELD  AF_HFLT0_P1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P4                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_1;	/* CAM_AF_FLT_1, CAM_A_AF_FLT_1*/

typedef volatile union _CAM_REG_AF_FLT_2_
{
		volatile struct	/* 0x1A004810 */
		{
				FIELD  AF_HFLT0_P5                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P6                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P7                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P8                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_2;	/* CAM_AF_FLT_2, CAM_A_AF_FLT_2*/

typedef volatile union _CAM_REG_AF_FLT_3_
{
		volatile struct	/* 0x1A004814 */
		{
				FIELD  AF_HFLT0_P9                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT0_P10                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT0_P11                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT0_P12                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_3;	/* CAM_AF_FLT_3, CAM_A_AF_FLT_3*/

typedef volatile union _CAM_REG_AF_FLT_4_
{
		volatile struct	/* 0x1A004818 */
		{
				FIELD  AF_HFLT1_P1                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P2                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P3                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P4                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_4;	/* CAM_AF_FLT_4, CAM_A_AF_FLT_4*/

typedef volatile union _CAM_REG_AF_FLT_5_
{
		volatile struct	/* 0x1A00481C */
		{
				FIELD  AF_HFLT1_P5                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P6                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P7                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P8                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_5;	/* CAM_AF_FLT_5, CAM_A_AF_FLT_5*/

typedef volatile union _CAM_REG_AF_FLT_6_
{
		volatile struct	/* 0x1A004820 */
		{
				FIELD  AF_HFLT1_P9                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT1_P10                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT1_P11                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT1_P12                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_6;	/* CAM_AF_FLT_6, CAM_A_AF_FLT_6*/

typedef volatile union _CAM_REG_AF_FLT_7_
{
		volatile struct	/* 0x1A004824 */
		{
				FIELD  AF_VFLT_X0                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AF_VFLT_X1                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_7;	/* CAM_AF_FLT_7, CAM_A_AF_FLT_7*/

typedef volatile union _CAM_REG_AF_FLT_8_
{
		volatile struct	/* 0x1A004828 */
		{
				FIELD  AF_VFLT_X2                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AF_VFLT_X3                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_FLT_8;	/* CAM_AF_FLT_8, CAM_A_AF_FLT_8*/

typedef volatile union _CAM_REG_AF_SIZE_
{
		volatile struct	/* 0x1A004830 */
		{
				FIELD  AF_IMAGE_WD                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_SIZE;	/* CAM_AF_SIZE, CAM_A_AF_SIZE*/

typedef volatile union _CAM_REG_AF_VLD_
{
		volatile struct	/* 0x1A004834 */
		{
				FIELD  AF_VLD_XSTART                         : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AF_VLD_YSTART                         : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_VLD;	/* CAM_AF_VLD, CAM_A_AF_VLD*/

typedef volatile union _CAM_REG_AF_BLK_0_
{
		volatile struct	/* 0x1A004838 */
		{
				FIELD  AF_BLK_XSIZE                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_BLK_YSIZE                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_BLK_0;	/* CAM_AF_BLK_0, CAM_A_AF_BLK_0*/

typedef volatile union _CAM_REG_AF_BLK_1_
{
		volatile struct	/* 0x1A00483C */
		{
				FIELD  AF_BLK_XNUM                           :  9;		/*  0.. 8, 0x000001FF */
				FIELD  rsv_9                                 :  7;		/*  9..15, 0x0000FE00 */
				FIELD  AF_BLK_YNUM                           :  9;		/* 16..24, 0x01FF0000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_BLK_1;	/* CAM_AF_BLK_1, CAM_A_AF_BLK_1*/

typedef volatile union _CAM_REG_AF_TH_2_
{
		volatile struct	/* 0x1A004840 */
		{
				FIELD  AF_HFLT2_SAT_TH0                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AF_HFLT2_SAT_TH1                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_HFLT2_SAT_TH2                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  AF_HFLT2_SAT_TH3                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_TH_2;	/* CAM_AF_TH_2, CAM_A_AF_TH_2*/

typedef volatile union _CAM_REG_AF_BLK_2_
{
		volatile struct	/* 0x1A004844 */
		{
				FIELD  AF_PROT_BLK_XSIZE                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AF_PROT_BLK_YSIZE                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AF_BLK_2;	/* CAM_AF_BLK_2, CAM_A_AF_BLK_2*/

typedef volatile union _CAM_REG_HLR_CFG_
{
		volatile struct	/* 0x1A004850 */
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
}CAM_REG_HLR_CFG;	/* CAM_HLR_CFG, CAM_A_HLR_CFG*/

typedef volatile union _CAM_REG_HLR_GAIN_
{
		volatile struct	/* 0x1A004858 */
		{
				FIELD  HLR_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HLR_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_GAIN;	/* CAM_HLR_GAIN, CAM_A_HLR_GAIN*/

typedef volatile union _CAM_REG_HLR_GAIN_1_
{
		volatile struct	/* 0x1A00485C */
		{
				FIELD  HLR_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  HLR_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_GAIN_1;	/* CAM_HLR_GAIN_1, CAM_A_HLR_GAIN_1*/

typedef volatile union _CAM_REG_HLR_OFST_
{
		volatile struct	/* 0x1A004860 */
		{
				FIELD  HLR_OFST_B                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  HLR_OFST_GB                           : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_OFST;	/* CAM_HLR_OFST, CAM_A_HLR_OFST*/

typedef volatile union _CAM_REG_HLR_OFST_1_
{
		volatile struct	/* 0x1A004864 */
		{
				FIELD  HLR_OFST_GR                           : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  HLR_OFST_R                            : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_OFST_1;	/* CAM_HLR_OFST_1, CAM_A_HLR_OFST_1*/

typedef volatile union _CAM_REG_HLR_IVGN_
{
		volatile struct	/* 0x1A004868 */
		{
				FIELD  HLR_IVGN_B                            : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  HLR_IVGN_GB                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_IVGN;	/* CAM_HLR_IVGN, CAM_A_HLR_IVGN*/

typedef volatile union _CAM_REG_HLR_IVGN_1_
{
		volatile struct	/* 0x1A00486C */
		{
				FIELD  HLR_IVGN_GR                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  HLR_IVGN_R                            : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_IVGN_1;	/* CAM_HLR_IVGN_1, CAM_A_HLR_IVGN_1*/

typedef volatile union _CAM_REG_HLR_KC_
{
		volatile struct	/* 0x1A004870 */
		{
				FIELD  HLR_KC_C0                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  HLR_KC_C1                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_KC;	/* CAM_HLR_KC, CAM_A_HLR_KC*/

typedef volatile union _CAM_REG_HLR_CFG_1_
{
		volatile struct	/* 0x1A004874 */
		{
				FIELD  HLR_STR                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_CFG_1;	/* CAM_HLR_CFG_1, CAM_A_HLR_CFG_1*/

typedef volatile union _CAM_REG_HLR_SL_PARA_
{
		volatile struct	/* 0x1A004878 */
		{
				FIELD  HLR_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  HLR_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_SL_PARA;	/* CAM_HLR_SL_PARA, CAM_A_HLR_SL_PARA*/

typedef volatile union _CAM_REG_HLR_SL_PARA_1_
{
		volatile struct	/* 0x1A00487C */
		{
				FIELD  HLR_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  HLR_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_HLR_SL_PARA_1;	/* CAM_HLR_SL_PARA_1, CAM_A_HLR_SL_PARA_1*/

typedef volatile union _CAM_REG_LCS25_CON_
{
		volatile struct	/* 0x1A004880 */
		{
				FIELD  LCS25_LOG                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  LCS25_OUT_WD                          :  9;		/*  8..16, 0x0001FF00 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  LCS25_OUT_HT                          :  9;		/* 20..28, 0x1FF00000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_CON;	/* CAM_LCS25_CON, CAM_A_LCS25_CON*/

typedef volatile union _CAM_REG_LCS25_ST_
{
		volatile struct	/* 0x1A004884 */
		{
				FIELD  LCS25_START_J                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_START_I                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_ST;	/* CAM_LCS25_ST, CAM_A_LCS25_ST*/

typedef volatile union _CAM_REG_LCS25_AWS_
{
		volatile struct	/* 0x1A004888 */
		{
				FIELD  LCS25_IN_WD                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_IN_HT                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_AWS;	/* CAM_LCS25_AWS, CAM_A_LCS25_AWS*/

typedef volatile union _CAM_REG_LCS25_FLR_
{
		volatile struct	/* 0x1A00488C */
		{
				FIELD  LCS25_FLR_OFST                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  LCS25_FLR_GAIN                        : 12;		/*  8..19, 0x000FFF00 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_FLR;	/* CAM_LCS25_FLR, CAM_A_LCS25_FLR*/

typedef volatile union _CAM_REG_LCS25_LRZR_1_
{
		volatile struct	/* 0x1A004890 */
		{
				FIELD  LCS25_LRZR_X                          : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_LRZR_1;	/* CAM_LCS25_LRZR_1, CAM_A_LCS25_LRZR_1*/

typedef volatile union _CAM_REG_LCS25_LRZR_2_
{
		volatile struct	/* 0x1A004894 */
		{
				FIELD  LCS25_LRZR_Y                          : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_LRZR_2;	/* CAM_LCS25_LRZR_2, CAM_A_LCS25_LRZR_2*/

typedef volatile union _CAM_REG_LCS25_SATU_1_
{
		volatile struct	/* 0x1A004898 */
		{
				FIELD  LCS25_SATU_B                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LCS25_SATU_GB                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_SATU_1;	/* CAM_LCS25_SATU_1, CAM_A_LCS25_SATU_1*/

typedef volatile union _CAM_REG_LCS25_SATU_2_
{
		volatile struct	/* 0x1A00489C */
		{
				FIELD  LCS25_SATU_GR                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LCS25_SATU_R                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_SATU_2;	/* CAM_LCS25_SATU_2, CAM_A_LCS25_SATU_2*/

typedef volatile union _CAM_REG_LCS25_GAIN_1_
{
		volatile struct	/* 0x1A0048A0 */
		{
				FIELD  LCS25_GAIN_B                          : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_GAIN_GB                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_GAIN_1;	/* CAM_LCS25_GAIN_1, CAM_A_LCS25_GAIN_1*/

typedef volatile union _CAM_REG_LCS25_GAIN_2_
{
		volatile struct	/* 0x1A0048A4 */
		{
				FIELD  LCS25_GAIN_GR                         : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_GAIN_R                          : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_GAIN_2;	/* CAM_LCS25_GAIN_2, CAM_A_LCS25_GAIN_2*/

typedef volatile union _CAM_REG_LCS25_OFST_1_
{
		volatile struct	/* 0x1A0048A8 */
		{
				FIELD  LCS25_OFST_B                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LCS25_OFST_GB                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_OFST_1;	/* CAM_LCS25_OFST_1, CAM_A_LCS25_OFST_1*/

typedef volatile union _CAM_REG_LCS25_OFST_2_
{
		volatile struct	/* 0x1A0048AC */
		{
				FIELD  LCS25_OFST_GR                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LCS25_OFST_R                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_OFST_2;	/* CAM_LCS25_OFST_2, CAM_A_LCS25_OFST_2*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_1_
{
		volatile struct	/* 0x1A0048B0 */
		{
				FIELD  LCS25_G2G_CNV_00                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_G2G_CNV_01                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_1;	/* CAM_LCS25_G2G_CNV_1, CAM_A_LCS25_G2G_CNV_1*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_2_
{
		volatile struct	/* 0x1A0048B4 */
		{
				FIELD  LCS25_G2G_CNV_02                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_G2G_CNV_10                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_2;	/* CAM_LCS25_G2G_CNV_2, CAM_A_LCS25_G2G_CNV_2*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_3_
{
		volatile struct	/* 0x1A0048B8 */
		{
				FIELD  LCS25_G2G_CNV_11                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_G2G_CNV_12                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_3;	/* CAM_LCS25_G2G_CNV_3, CAM_A_LCS25_G2G_CNV_3*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_4_
{
		volatile struct	/* 0x1A0048BC */
		{
				FIELD  LCS25_G2G_CNV_20                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_G2G_CNV_21                      : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_4;	/* CAM_LCS25_G2G_CNV_4, CAM_A_LCS25_G2G_CNV_4*/

typedef volatile union _CAM_REG_LCS25_G2G_CNV_5_
{
		volatile struct	/* 0x1A0048C0 */
		{
				FIELD  LCS25_G2G_CNV_22                      : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  LCS25_G2G_ACC                         :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_G2G_CNV_5;	/* CAM_LCS25_G2G_CNV_5, CAM_A_LCS25_G2G_CNV_5*/

typedef volatile union _CAM_REG_LCS25_LPF_
{
		volatile struct	/* 0x1A0048C4 */
		{
				FIELD  LCS25_LPF_EN                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  LCS25_LPF_TH                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCS25_LPF;	/* CAM_LCS25_LPF, CAM_A_LCS25_LPF*/

typedef volatile union _CAM_REG_RCP_CROP_CON1_
{
		volatile struct	/* 0x1A0048F0 */
		{
				FIELD  RCP_STR_X                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_X                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RCP_CROP_CON1;	/* CAM_RCP_CROP_CON1, CAM_A_RCP_CROP_CON1*/

typedef volatile union _CAM_REG_RCP_CROP_CON2_
{
		volatile struct	/* 0x1A0048F4 */
		{
				FIELD  RCP_STR_Y                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_Y                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RCP_CROP_CON2;	/* CAM_RCP_CROP_CON2, CAM_A_RCP_CROP_CON2*/

typedef volatile union _CAM_REG_SGG1_PGN_
{
		volatile struct	/* 0x1A004900 */
		{
				FIELD  SGG_GAIN                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_PGN;	/* CAM_SGG1_PGN, CAM_A_SGG1_PGN*/

typedef volatile union _CAM_REG_SGG1_GMRC_1_
{
		volatile struct	/* 0x1A004904 */
		{
				FIELD  SGG_GMR_1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG_GMR_4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_GMRC_1;	/* CAM_SGG1_GMRC_1, CAM_A_SGG1_GMRC_1*/

typedef volatile union _CAM_REG_SGG1_GMRC_2_
{
		volatile struct	/* 0x1A004908 */
		{
				FIELD  SGG_GMR_5                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_6                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_7                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG1_GMRC_2;	/* CAM_SGG1_GMRC_2, CAM_A_SGG1_GMRC_2*/

typedef volatile union _CAM_REG_QBN2_MODE_
{
		volatile struct	/* 0x1A004910 */
		{
				FIELD  QBN_ACC                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  QBN_ACC_MODE                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_QBN2_MODE;	/* CAM_QBN2_MODE, CAM_A_QBN2_MODE*/

typedef volatile union _CAM_REG_AWB_WIN_ORG_
{
		volatile struct	/* 0x1A004920 */
		{
				FIELD  AWB_W_HORG                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VORG                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_ORG;	/* CAM_AWB_WIN_ORG, CAM_A_AWB_WIN_ORG*/

typedef volatile union _CAM_REG_AWB_WIN_SIZE_
{
		volatile struct	/* 0x1A004924 */
		{
				FIELD  AWB_W_HSIZE                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VSIZE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_SIZE;	/* CAM_AWB_WIN_SIZE, CAM_A_AWB_WIN_SIZE*/

typedef volatile union _CAM_REG_AWB_WIN_PIT_
{
		volatile struct	/* 0x1A004928 */
		{
				FIELD  AWB_W_HPIT                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VPIT                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_PIT;	/* CAM_AWB_WIN_PIT, CAM_A_AWB_WIN_PIT*/

typedef volatile union _CAM_REG_AWB_WIN_NUM_
{
		volatile struct	/* 0x1A00492C */
		{
				FIELD  AWB_W_HNUM                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_W_VNUM                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_WIN_NUM;	/* CAM_AWB_WIN_NUM, CAM_A_AWB_WIN_NUM*/

typedef volatile union _CAM_REG_AWB_GAIN1_0_
{
		volatile struct	/* 0x1A004930 */
		{
				FIELD  AWB_GAIN1_R                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AWB_GAIN1_G                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_GAIN1_0;	/* CAM_AWB_GAIN1_0, CAM_A_AWB_GAIN1_0*/

typedef volatile union _CAM_REG_AWB_GAIN1_1_
{
		volatile struct	/* 0x1A004934 */
		{
				FIELD  AWB_GAIN1_B                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_GAIN1_1;	/* CAM_AWB_GAIN1_1, CAM_A_AWB_GAIN1_1*/

typedef volatile union _CAM_REG_AWB_LMT1_0_
{
		volatile struct	/* 0x1A004938 */
		{
				FIELD  AWB_LMT1_R                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_LMT1_G                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LMT1_0;	/* CAM_AWB_LMT1_0, CAM_A_AWB_LMT1_0*/

typedef volatile union _CAM_REG_AWB_LMT1_1_
{
		volatile struct	/* 0x1A00493C */
		{
				FIELD  AWB_LMT1_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LMT1_1;	/* CAM_AWB_LMT1_1, CAM_A_AWB_LMT1_1*/

typedef volatile union _CAM_REG_AWB_LOW_THR_
{
		volatile struct	/* 0x1A004940 */
		{
				FIELD  AWB_LOW_THR0                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AWB_LOW_THR1                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_LOW_THR2                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_LOW_THR;	/* CAM_AWB_LOW_THR, CAM_A_AWB_LOW_THR*/

typedef volatile union _CAM_REG_AWB_HI_THR_
{
		volatile struct	/* 0x1A004944 */
		{
				FIELD  AWB_HI_THR0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AWB_HI_THR1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_HI_THR2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_HI_THR;	/* CAM_AWB_HI_THR, CAM_A_AWB_HI_THR*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT0_
{
		volatile struct	/* 0x1A004948 */
		{
				FIELD  AWB_PIXEL_CNT0                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT0;	/* CAM_AWB_PIXEL_CNT0, CAM_A_AWB_PIXEL_CNT0*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT1_
{
		volatile struct	/* 0x1A00494C */
		{
				FIELD  AWB_PIXEL_CNT1                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT1;	/* CAM_AWB_PIXEL_CNT1, CAM_A_AWB_PIXEL_CNT1*/

typedef volatile union _CAM_REG_AWB_PIXEL_CNT2_
{
		volatile struct	/* 0x1A004950 */
		{
				FIELD  AWB_PIXEL_CNT2                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_PIXEL_CNT2;	/* CAM_AWB_PIXEL_CNT2, CAM_A_AWB_PIXEL_CNT2*/

typedef volatile union _CAM_REG_AWB_ERR_THR_
{
		volatile struct	/* 0x1A004954 */
		{
				FIELD  AWB_ERR_THR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_ERR_SFT                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_ERR_THR;	/* CAM_AWB_ERR_THR, CAM_A_AWB_ERR_THR*/

typedef volatile union _CAM_REG_AWB_ROT_
{
		volatile struct	/* 0x1A004958 */
		{
				FIELD  AWB_C                                 : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  AWB_S                                 : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_ROT;	/* CAM_AWB_ROT, CAM_A_AWB_ROT*/

typedef volatile union _CAM_REG_AWB_L0_X_
{
		volatile struct	/* 0x1A00495C */
		{
				FIELD  AWB_L0_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L0_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L0_X;	/* CAM_AWB_L0_X, CAM_A_AWB_L0_X*/

typedef volatile union _CAM_REG_AWB_L0_Y_
{
		volatile struct	/* 0x1A004960 */
		{
				FIELD  AWB_L0_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L0_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L0_Y;	/* CAM_AWB_L0_Y, CAM_A_AWB_L0_Y*/

typedef volatile union _CAM_REG_AWB_L1_X_
{
		volatile struct	/* 0x1A004964 */
		{
				FIELD  AWB_L1_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L1_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L1_X;	/* CAM_AWB_L1_X, CAM_A_AWB_L1_X*/

typedef volatile union _CAM_REG_AWB_L1_Y_
{
		volatile struct	/* 0x1A004968 */
		{
				FIELD  AWB_L1_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L1_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L1_Y;	/* CAM_AWB_L1_Y, CAM_A_AWB_L1_Y*/

typedef volatile union _CAM_REG_AWB_L2_X_
{
		volatile struct	/* 0x1A00496C */
		{
				FIELD  AWB_L2_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L2_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L2_X;	/* CAM_AWB_L2_X, CAM_A_AWB_L2_X*/

typedef volatile union _CAM_REG_AWB_L2_Y_
{
		volatile struct	/* 0x1A004970 */
		{
				FIELD  AWB_L2_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L2_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L2_Y;	/* CAM_AWB_L2_Y, CAM_A_AWB_L2_Y*/

typedef volatile union _CAM_REG_AWB_L3_X_
{
		volatile struct	/* 0x1A004974 */
		{
				FIELD  AWB_L3_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L3_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L3_X;	/* CAM_AWB_L3_X, CAM_A_AWB_L3_X*/

typedef volatile union _CAM_REG_AWB_L3_Y_
{
		volatile struct	/* 0x1A004978 */
		{
				FIELD  AWB_L3_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L3_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L3_Y;	/* CAM_AWB_L3_Y, CAM_A_AWB_L3_Y*/

typedef volatile union _CAM_REG_AWB_L4_X_
{
		volatile struct	/* 0x1A00497C */
		{
				FIELD  AWB_L4_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L4_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L4_X;	/* CAM_AWB_L4_X, CAM_A_AWB_L4_X*/

typedef volatile union _CAM_REG_AWB_L4_Y_
{
		volatile struct	/* 0x1A004980 */
		{
				FIELD  AWB_L4_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L4_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L4_Y;	/* CAM_AWB_L4_Y, CAM_A_AWB_L4_Y*/

typedef volatile union _CAM_REG_AWB_L5_X_
{
		volatile struct	/* 0x1A004984 */
		{
				FIELD  AWB_L5_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L5_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L5_X;	/* CAM_AWB_L5_X, CAM_A_AWB_L5_X*/

typedef volatile union _CAM_REG_AWB_L5_Y_
{
		volatile struct	/* 0x1A004988 */
		{
				FIELD  AWB_L5_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L5_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L5_Y;	/* CAM_AWB_L5_Y, CAM_A_AWB_L5_Y*/

typedef volatile union _CAM_REG_AWB_L6_X_
{
		volatile struct	/* 0x1A00498C */
		{
				FIELD  AWB_L6_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L6_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L6_X;	/* CAM_AWB_L6_X, CAM_A_AWB_L6_X*/

typedef volatile union _CAM_REG_AWB_L6_Y_
{
		volatile struct	/* 0x1A004990 */
		{
				FIELD  AWB_L6_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L6_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L6_Y;	/* CAM_AWB_L6_Y, CAM_A_AWB_L6_Y*/

typedef volatile union _CAM_REG_AWB_L7_X_
{
		volatile struct	/* 0x1A004994 */
		{
				FIELD  AWB_L7_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L7_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L7_X;	/* CAM_AWB_L7_X, CAM_A_AWB_L7_X*/

typedef volatile union _CAM_REG_AWB_L7_Y_
{
		volatile struct	/* 0x1A004998 */
		{
				FIELD  AWB_L7_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L7_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L7_Y;	/* CAM_AWB_L7_Y, CAM_A_AWB_L7_Y*/

typedef volatile union _CAM_REG_AWB_L8_X_
{
		volatile struct	/* 0x1A00499C */
		{
				FIELD  AWB_L8_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L8_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L8_X;	/* CAM_AWB_L8_X, CAM_A_AWB_L8_X*/

typedef volatile union _CAM_REG_AWB_L8_Y_
{
		volatile struct	/* 0x1A0049A0 */
		{
				FIELD  AWB_L8_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L8_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L8_Y;	/* CAM_AWB_L8_Y, CAM_A_AWB_L8_Y*/

typedef volatile union _CAM_REG_AWB_L9_X_
{
		volatile struct	/* 0x1A0049A4 */
		{
				FIELD  AWB_L9_X_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L9_X_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L9_X;	/* CAM_AWB_L9_X, CAM_A_AWB_L9_X*/

typedef volatile union _CAM_REG_AWB_L9_Y_
{
		volatile struct	/* 0x1A0049A8 */
		{
				FIELD  AWB_L9_Y_LOW                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_L9_Y_UP                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_L9_Y;	/* CAM_AWB_L9_Y, CAM_A_AWB_L9_Y*/

typedef volatile union _CAM_REG_AWB_SPARE_
{
		volatile struct	/* 0x1A0049AC */
		{
				FIELD  AWB_SPARE                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_SPARE;	/* CAM_AWB_SPARE, CAM_A_AWB_SPARE*/

typedef volatile union _CAM_REG_AWB_MOTION_THR_
{
		volatile struct	/* 0x1A0049B0 */
		{
				FIELD  AWB_MOTION_THR                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                : 18;		/* 14..31, 0xFFFFC000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_MOTION_THR;	/* CAM_AWB_MOTION_THR, CAM_A_AWB_MOTION_THR*/

typedef volatile union _CAM_REG_AWB_RC_CNV_0_
{
		volatile struct	/* 0x1A0049B4 */
		{
				FIELD  AWB_RC_CNV00                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_RC_CNV01                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_RC_CNV_0;	/* CAM_AWB_RC_CNV_0, CAM_A_AWB_RC_CNV_0*/

typedef volatile union _CAM_REG_AWB_RC_CNV_1_
{
		volatile struct	/* 0x1A0049B8 */
		{
				FIELD  AWB_RC_CNV02                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_RC_CNV10                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_RC_CNV_1;	/* CAM_AWB_RC_CNV_1, CAM_A_AWB_RC_CNV_1*/

typedef volatile union _CAM_REG_AWB_RC_CNV_2_
{
		volatile struct	/* 0x1A0049BC */
		{
				FIELD  AWB_RC_CNV11                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_RC_CNV12                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_RC_CNV_2;	/* CAM_AWB_RC_CNV_2, CAM_A_AWB_RC_CNV_2*/

typedef volatile union _CAM_REG_AWB_RC_CNV_3_
{
		volatile struct	/* 0x1A0049C0 */
		{
				FIELD  AWB_RC_CNV20                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_RC_CNV21                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_RC_CNV_3;	/* CAM_AWB_RC_CNV_3, CAM_A_AWB_RC_CNV_3*/

typedef volatile union _CAM_REG_AWB_RC_CNV_4_
{
		volatile struct	/* 0x1A0049C4 */
		{
				FIELD  AWB_RC_CNV22                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AWB_RC_ACC                            :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AWB_RC_CNV_4;	/* CAM_AWB_RC_CNV_4, CAM_A_AWB_RC_CNV_4*/

typedef volatile union _CAM_REG_AE_GAIN2_0_
{
		volatile struct	/* 0x1A0049E0 */
		{
				FIELD  AE_GAIN2_R                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_GAIN2_G                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_GAIN2_0;	/* CAM_AE_GAIN2_0, CAM_A_AE_GAIN2_0*/

typedef volatile union _CAM_REG_AE_GAIN2_1_
{
		volatile struct	/* 0x1A0049E4 */
		{
				FIELD  AE_GAIN2_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_GAIN2_1;	/* CAM_AE_GAIN2_1, CAM_A_AE_GAIN2_1*/

typedef volatile union _CAM_REG_AE_LMT2_0_
{
		volatile struct	/* 0x1A0049E8 */
		{
				FIELD  AE_LMT2_R                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_LMT2_G                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_LMT2_0;	/* CAM_AE_LMT2_0, CAM_A_AE_LMT2_0*/

typedef volatile union _CAM_REG_AE_LMT2_1_
{
		volatile struct	/* 0x1A0049EC */
		{
				FIELD  AE_LMT2_B                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_LMT2_1;	/* CAM_AE_LMT2_1, CAM_A_AE_LMT2_1*/

typedef volatile union _CAM_REG_AE_RC_CNV_0_
{
		volatile struct	/* 0x1A0049F0 */
		{
				FIELD  AE_RC_CNV00                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV01                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_0;	/* CAM_AE_RC_CNV_0, CAM_A_AE_RC_CNV_0*/

typedef volatile union _CAM_REG_AE_RC_CNV_1_
{
		volatile struct	/* 0x1A0049F4 */
		{
				FIELD  AE_RC_CNV02                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV10                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_1;	/* CAM_AE_RC_CNV_1, CAM_A_AE_RC_CNV_1*/

typedef volatile union _CAM_REG_AE_RC_CNV_2_
{
		volatile struct	/* 0x1A0049F8 */
		{
				FIELD  AE_RC_CNV11                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV12                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_2;	/* CAM_AE_RC_CNV_2, CAM_A_AE_RC_CNV_2*/

typedef volatile union _CAM_REG_AE_RC_CNV_3_
{
		volatile struct	/* 0x1A0049FC */
		{
				FIELD  AE_RC_CNV20                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_CNV21                           : 11;		/* 16..26, 0x07FF0000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_3;	/* CAM_AE_RC_CNV_3, CAM_A_AE_RC_CNV_3*/

typedef volatile union _CAM_REG_AE_RC_CNV_4_
{
		volatile struct	/* 0x1A004A00 */
		{
				FIELD  AE_RC_CNV22                           : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  AE_RC_ACC                             :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_RC_CNV_4;	/* CAM_AE_RC_CNV_4, CAM_A_AE_RC_CNV_4*/

typedef volatile union _CAM_REG_AE_YGAMMA_0_
{
		volatile struct	/* 0x1A004A04 */
		{
				FIELD  Y_GMR1                                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  Y_GMR2                                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  Y_GMR3                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  Y_GMR4                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_YGAMMA_0;	/* CAM_AE_YGAMMA_0, CAM_A_AE_YGAMMA_0*/

typedef volatile union _CAM_REG_AE_YGAMMA_1_
{
		volatile struct	/* 0x1A004A08 */
		{
				FIELD  Y_GMR5                                :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_YGAMMA_1;	/* CAM_AE_YGAMMA_1, CAM_A_AE_YGAMMA_1*/

typedef volatile union _CAM_REG_AE_OVER_EXPO_CFG_
{
		volatile struct	/* 0x1A004A0C */
		{
				FIELD  AE_OVER_EXPO_THR                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  AE_OVER_EXPO_SFT                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 : 23;		/*  9..31, 0xFFFFFE00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_OVER_EXPO_CFG;	/* CAM_AE_OVER_EXPO_CFG, CAM_A_AE_OVER_EXPO_CFG*/

typedef volatile union _CAM_REG_AE_PIX_HST_CTL_
{
		volatile struct	/* 0x1A004A10 */
		{
				FIELD  AE_PIX_HST0_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_PIX_HST1_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AE_PIX_HST2_EN                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AE_PIX_HST3_EN                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST_CTL;	/* CAM_AE_PIX_HST_CTL, CAM_A_AE_PIX_HST_CTL*/

typedef volatile union _CAM_REG_AE_PIX_HST_SET_
{
		volatile struct	/* 0x1A004A14 */
		{
				FIELD  AE_PIX_BIN_MODE_0                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  AE_PIX_BIN_MODE_1                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  AE_PIX_BIN_MODE_2                     :  2;		/*  8.. 9, 0x00000300 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  AE_PIX_BIN_MODE_3                     :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AE_PIX_COLOR_MODE_0                   :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  AE_PIX_COLOR_MODE_1                   :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  AE_PIX_COLOR_MODE_2                   :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  AE_PIX_COLOR_MODE_3                   :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST_SET;	/* CAM_AE_PIX_HST_SET, CAM_A_AE_PIX_HST_SET*/

typedef volatile union _CAM_REG_AE_PIX_HST0_YRNG_
{
		volatile struct	/* 0x1A004A1C */
		{
				FIELD  AE_PIX_Y_LOW_0                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_PIX_Y_HI_0                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST0_YRNG;	/* CAM_AE_PIX_HST0_YRNG, CAM_A_AE_PIX_HST0_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST0_XRNG_
{
		volatile struct	/* 0x1A004A20 */
		{
				FIELD  AE_PIX_X_LOW_0                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AE_PIX_X_HI_0                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST0_XRNG;	/* CAM_AE_PIX_HST0_XRNG, CAM_A_AE_PIX_HST0_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST1_YRNG_
{
		volatile struct	/* 0x1A004A24 */
		{
				FIELD  AE_PIX_Y_LOW_1                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_PIX_Y_HI_1                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST1_YRNG;	/* CAM_AE_PIX_HST1_YRNG, CAM_A_AE_PIX_HST1_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST1_XRNG_
{
		volatile struct	/* 0x1A004A28 */
		{
				FIELD  AE_PIX_X_LOW_1                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AE_PIX_X_HI_1                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST1_XRNG;	/* CAM_AE_PIX_HST1_XRNG, CAM_A_AE_PIX_HST1_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST2_YRNG_
{
		volatile struct	/* 0x1A004A2C */
		{
				FIELD  AE_PIX_Y_LOW_2                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_PIX_Y_HI_2                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST2_YRNG;	/* CAM_AE_PIX_HST2_YRNG, CAM_A_AE_PIX_HST2_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST2_XRNG_
{
		volatile struct	/* 0x1A004A30 */
		{
				FIELD  AE_PIX_X_LOW_2                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AE_PIX_X_HI_2                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST2_XRNG;	/* CAM_AE_PIX_HST2_XRNG, CAM_A_AE_PIX_HST2_XRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST3_YRNG_
{
		volatile struct	/* 0x1A004A34 */
		{
				FIELD  AE_PIX_Y_LOW_3                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  AE_PIX_Y_HI_3                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST3_YRNG;	/* CAM_AE_PIX_HST3_YRNG, CAM_A_AE_PIX_HST3_YRNG*/

typedef volatile union _CAM_REG_AE_PIX_HST3_XRNG_
{
		volatile struct	/* 0x1A004A38 */
		{
				FIELD  AE_PIX_X_LOW_3                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  AE_PIX_X_HI_3                         : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_PIX_HST3_XRNG;	/* CAM_AE_PIX_HST3_XRNG, CAM_A_AE_PIX_HST3_XRNG*/

typedef volatile union _CAM_REG_AE_STAT_EN_
{
		volatile struct	/* 0x1A004A3C */
		{
				FIELD  AE_TSF_STAT_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AE_OVERCNT_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AE_HDR_STAT_EN                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_STAT_EN;	/* CAM_AE_STAT_EN, CAM_A_AE_STAT_EN*/

typedef volatile union _CAM_REG_AE_YCOEF_
{
		volatile struct	/* 0x1A004A40 */
		{
				FIELD  AE_YCOEF_R                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  AE_YCOEF_G                            :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  AE_YCOEF_B                            :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_YCOEF;	/* CAM_AE_YCOEF, CAM_A_AE_YCOEF*/

typedef volatile union _CAM_REG_AE_CCU_HST_END_Y_
{
		volatile struct	/* 0x1A004A44 */
		{
				FIELD  AE_CCU_HST_END_Y                      :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_CCU_HST_END_Y;	/* CAM_AE_CCU_HST_END_Y, CAM_A_AE_CCU_HST_END_Y*/

typedef volatile union _CAM_REG_AE_SPARE_
{
		volatile struct	/* 0x1A004A48 */
		{
				FIELD  AE_SPARE                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AE_SPARE;	/* CAM_AE_SPARE, CAM_A_AE_SPARE*/

typedef volatile union _CAM_REG_QBN1_MODE_
{
		volatile struct	/* 0x1A004AC0 */
		{
				FIELD  QBN_ACC                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  QBN_ACC_MODE                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_QBN1_MODE;	/* CAM_QBN1_MODE, CAM_A_QBN1_MODE*/

typedef volatile union _CAM_REG_CPG_SATU_1_
{
		volatile struct	/* 0x1A004AD0 */
		{
				FIELD  CPG_SATU_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_SATU_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_SATU_1;	/* CAM_CPG_SATU_1, CAM_A_CPG_SATU_1*/

typedef volatile union _CAM_REG_CPG_SATU_2_
{
		volatile struct	/* 0x1A004AD4 */
		{
				FIELD  CPG_SATU_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_SATU_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_SATU_2;	/* CAM_CPG_SATU_2, CAM_A_CPG_SATU_2*/

typedef volatile union _CAM_REG_CPG_GAIN_1_
{
		volatile struct	/* 0x1A004AD8 */
		{
				FIELD  CPG_GAIN_B                            : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_GAIN_GB                           : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_GAIN_1;	/* CAM_CPG_GAIN_1, CAM_A_CPG_GAIN_1*/

typedef volatile union _CAM_REG_CPG_GAIN_2_
{
		volatile struct	/* 0x1A004ADC */
		{
				FIELD  CPG_GAIN_GR                           : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CPG_GAIN_R                            : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_GAIN_2;	/* CAM_CPG_GAIN_2, CAM_A_CPG_GAIN_2*/

typedef volatile union _CAM_REG_CPG_OFST_1_
{
		volatile struct	/* 0x1A004AE0 */
		{
				FIELD  CPG_OFST_B                            : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_OFST_GB                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_OFST_1;	/* CAM_CPG_OFST_1, CAM_A_CPG_OFST_1*/

typedef volatile union _CAM_REG_CPG_OFST_2_
{
		volatile struct	/* 0x1A004AE4 */
		{
				FIELD  CPG_OFST_GR                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CPG_OFST_R                            : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CPG_OFST_2;	/* CAM_CPG_OFST_2, CAM_A_CPG_OFST_2*/

typedef volatile union _CAM_REG_CAC_TILE_SIZE_
{
		volatile struct	/* 0x1A004AF0 */
		{
				FIELD  CAC_TILE_WIDTH                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CAC_TILE_HEIGHT                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CAC_TILE_SIZE;	/* CAM_CAC_TILE_SIZE, CAM_A_CAC_TILE_SIZE*/

typedef volatile union _CAM_REG_CAC_TILE_OFFSET_
{
		volatile struct	/* 0x1A004AF4 */
		{
				FIELD  CAC_OFFSET_X                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CAC_OFFSET_Y                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_CAC_TILE_OFFSET;	/* CAM_CAC_TILE_OFFSET, CAM_A_CAC_TILE_OFFSET*/

typedef volatile union _CAM_REG_PMX_CTL_
{
		volatile struct	/* 0x1A004B00 */
		{
				FIELD  PMX_SRAM_SIZE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  PMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  PMX_SINGLE_MODE_1                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  PMX_SINGLE_MODE_2                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  3;		/* 25..27, 0x0E000000 */
				FIELD  PMX_DUAL_PD_MODE                      :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  2;		/* 29..30, 0x60000000 */
				FIELD  PMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PMX_CTL;	/* CAM_PMX_CTL, CAM_A_PMX_CTL*/

typedef volatile union _CAM_REG_PMX_CROP_
{
		volatile struct	/* 0x1A004B04 */
		{
				FIELD  PMX_STR_X                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  PMX_END_X                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PMX_CROP;	/* CAM_PMX_CROP, CAM_A_PMX_CROP*/

typedef volatile union _CAM_REG_PMX_VSIZE_
{
		volatile struct	/* 0x1A004B08 */
		{
				FIELD  PMX_HT                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PMX_VSIZE;	/* CAM_PMX_VSIZE, CAM_A_PMX_VSIZE*/

typedef volatile union _CAM_REG_VBN_GAIN_
{
		volatile struct	/* 0x1A004B40 */
		{
				FIELD  VBN_GAIN                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_VBN_GAIN;	/* CAM_VBN_GAIN, CAM_A_VBN_GAIN*/

typedef volatile union _CAM_REG_VBN_OFST_
{
		volatile struct	/* 0x1A004B44 */
		{
				FIELD  VBN_OFST                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_VBN_OFST;	/* CAM_VBN_OFST, CAM_A_VBN_OFST*/

typedef volatile union _CAM_REG_VBN_TYPE_
{
		volatile struct	/* 0x1A004B48 */
		{
				FIELD  VBN_TYPE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  VBN_DIAG_SEL_EN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_VBN_TYPE;	/* CAM_VBN_TYPE, CAM_A_VBN_TYPE*/

typedef volatile union _CAM_REG_VBN_SPARE_
{
		volatile struct	/* 0x1A004B4C */
		{
				FIELD  VBN_SPARE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_VBN_SPARE;	/* CAM_VBN_SPARE, CAM_A_VBN_SPARE*/

typedef volatile union _CAM_REG_AMX_CTL_
{
		volatile struct	/* 0x1A004B60 */
		{
				FIELD  AMX_SRAM_SIZE                         : 16;		/*  0..15, 0x0000FFFF */
				FIELD  AMX_EDGE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  AMX_SINGLE_MODE_1                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  AMX_SINGLE_MODE_2                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  6;		/* 25..30, 0x7E000000 */
				FIELD  AMX_EDGE_SET                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AMX_CTL;	/* CAM_AMX_CTL, CAM_A_AMX_CTL*/

typedef volatile union _CAM_REG_AMX_CROP_
{
		volatile struct	/* 0x1A004B64 */
		{
				FIELD  AMX_STR_X                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  AMX_END_X                             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AMX_CROP;	/* CAM_AMX_CROP, CAM_A_AMX_CROP*/

typedef volatile union _CAM_REG_AMX_VSIZE_
{
		volatile struct	/* 0x1A004B68 */
		{
				FIELD  AMX_HT                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_AMX_VSIZE;	/* CAM_AMX_VSIZE, CAM_A_AMX_VSIZE*/

typedef volatile union _CAM_REG_BIN_CTL_
{
		volatile struct	/* 0x1A004B80 */
		{
				FIELD  BIN_TYPE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BIN_MODE                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  BIN_DMD                               :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  BIN_OV_TH                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BIN_LE_INV_CTL                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BIN_CTL;	/* CAM_BIN_CTL, CAM_A_BIN_CTL*/

typedef volatile union _CAM_REG_BIN_FTH_
{
		volatile struct	/* 0x1A004B84 */
		{
				FIELD  BIN_FTH1                              :  8;		/*  0.. 7, 0x000000FF */
				FIELD  BIN_FTH2                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  BIN_FTH3                              :  8;		/* 16..23, 0x00FF0000 */
				FIELD  BIN_FTH4                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BIN_FTH;	/* CAM_BIN_FTH, CAM_A_BIN_FTH*/

typedef volatile union _CAM_REG_BIN_SPARE_
{
		volatile struct	/* 0x1A004B88 */
		{
				FIELD  BIN_SPARE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_BIN_SPARE;	/* CAM_BIN_SPARE, CAM_A_BIN_SPARE*/

typedef volatile union _CAM_REG_DBN_GAIN_
{
		volatile struct	/* 0x1A004BA0 */
		{
				FIELD  DBN_GAIN                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBN_GAIN;	/* CAM_DBN_GAIN, CAM_A_DBN_GAIN*/

typedef volatile union _CAM_REG_DBN_OFST_
{
		volatile struct	/* 0x1A004BA4 */
		{
				FIELD  DBN_OFST                              : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBN_OFST;	/* CAM_DBN_OFST, CAM_A_DBN_OFST*/

typedef volatile union _CAM_REG_DBN_SPARE_
{
		volatile struct	/* 0x1A004BA8 */
		{
				FIELD  DBN_SPARE                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBN_SPARE;	/* CAM_DBN_SPARE, CAM_A_DBN_SPARE*/

typedef volatile union _CAM_REG_PBN_TYPE_
{
		volatile struct	/* 0x1A004BB0 */
		{
				FIELD  PBN_TYPE                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  PBN_SEP                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PBN_OBIT                              :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PBN_TYPE;	/* CAM_PBN_TYPE, CAM_A_PBN_TYPE*/

typedef volatile union _CAM_REG_PBN_LST_
{
		volatile struct	/* 0x1A004BB4 */
		{
				FIELD  PBN_LST                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAM_REG_PBN_LST;	/* CAM_PBN_LST, CAM_A_PBN_LST*/

typedef volatile union _CAM_REG_PBN_VSIZE_
{
		volatile struct	/* 0x1A004BB8 */
		{
				FIELD  PBN_VSIZE                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PBN_VSIZE;	/* CAM_PBN_VSIZE, CAM_A_PBN_VSIZE*/

typedef volatile union _CAM_REG_RCP3_CROP_CON1_
{
		volatile struct	/* 0x1A004BC0 */
		{
				FIELD  RCP_STR_X                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_X                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RCP3_CROP_CON1;	/* CAM_RCP3_CROP_CON1, CAM_A_RCP3_CROP_CON1*/

typedef volatile union _CAM_REG_RCP3_CROP_CON2_
{
		volatile struct	/* 0x1A004BC4 */
		{
				FIELD  RCP_STR_Y                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  RCP_END_Y                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_RCP3_CROP_CON2;	/* CAM_RCP3_CROP_CON2, CAM_A_RCP3_CROP_CON2*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE0_3_
{
		volatile struct	/* 0x1A004BD0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_R_OFST1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_R_OFST2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_R_OFST3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE0_3;	/* CAM_SGM_R_OFST_TABLE0_3, CAM_A_SGM_R_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE4_7_
{
		volatile struct	/* 0x1A004BD4 */
		{
				FIELD  SGM_R_OFST4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_R_OFST5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_R_OFST6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_R_OFST7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE4_7;	/* CAM_SGM_R_OFST_TABLE4_7, CAM_A_SGM_R_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE8_11_
{
		volatile struct	/* 0x1A004BD8 */
		{
				FIELD  SGM_R_OFST8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_R_OFST9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_R_OFST10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_R_OFST11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE8_11;	/* CAM_SGM_R_OFST_TABLE8_11, CAM_A_SGM_R_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_R_OFST_TABLE12_15_
{
		volatile struct	/* 0x1A004BDC */
		{
				FIELD  SGM_R_OFST12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_R_OFST13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_R_OFST14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_R_OFST15                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_R_OFST_TABLE12_15;	/* CAM_SGM_R_OFST_TABLE12_15, CAM_A_SGM_R_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE0_3_
{
		volatile struct	/* 0x1A004BE0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_G_OFST1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_G_OFST2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_G_OFST3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE0_3;	/* CAM_SGM_G_OFST_TABLE0_3, CAM_A_SGM_G_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE4_7_
{
		volatile struct	/* 0x1A004BE4 */
		{
				FIELD  SGM_G_OFST4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_G_OFST5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_G_OFST6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_G_OFST7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE4_7;	/* CAM_SGM_G_OFST_TABLE4_7, CAM_A_SGM_G_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE8_11_
{
		volatile struct	/* 0x1A004BE8 */
		{
				FIELD  SGM_G_OFST8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_G_OFST9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_G_OFST10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_G_OFST11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE8_11;	/* CAM_SGM_G_OFST_TABLE8_11, CAM_A_SGM_G_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_G_OFST_TABLE12_15_
{
		volatile struct	/* 0x1A004BEC */
		{
				FIELD  SGM_G_OFST12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_G_OFST13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_G_OFST14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_G_OFST15                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_G_OFST_TABLE12_15;	/* CAM_SGM_G_OFST_TABLE12_15, CAM_A_SGM_G_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE0_3_
{
		volatile struct	/* 0x1A004BF0 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_B_OFST1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_B_OFST2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_B_OFST3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE0_3;	/* CAM_SGM_B_OFST_TABLE0_3, CAM_A_SGM_B_OFST_TABLE0_3*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE4_7_
{
		volatile struct	/* 0x1A004BF4 */
		{
				FIELD  SGM_B_OFST4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_B_OFST5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_B_OFST6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_B_OFST7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE4_7;	/* CAM_SGM_B_OFST_TABLE4_7, CAM_A_SGM_B_OFST_TABLE4_7*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE8_11_
{
		volatile struct	/* 0x1A004BF8 */
		{
				FIELD  SGM_B_OFST8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_B_OFST9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_B_OFST10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_B_OFST11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE8_11;	/* CAM_SGM_B_OFST_TABLE8_11, CAM_A_SGM_B_OFST_TABLE8_11*/

typedef volatile union _CAM_REG_SGM_B_OFST_TABLE12_15_
{
		volatile struct	/* 0x1A004BFC */
		{
				FIELD  SGM_B_OFST12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGM_B_OFST13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGM_B_OFST14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGM_B_OFST15                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGM_B_OFST_TABLE12_15;	/* CAM_SGM_B_OFST_TABLE12_15, CAM_A_SGM_B_OFST_TABLE12_15*/

typedef volatile union _CAM_REG_DBS_SIGMA_
{
		volatile struct	/* 0x1A004C00 */
		{
				FIELD  DBS_OFST                              : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DBS_SL                                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SIGMA;	/* CAM_DBS_SIGMA, CAM_A_DBS_SIGMA*/

typedef volatile union _CAM_REG_DBS_BSTBL_0_
{
		volatile struct	/* 0x1A004C04 */
		{
				FIELD  DBS_BIAS_Y0                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y1                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y2                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y3                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_0;	/* CAM_DBS_BSTBL_0, CAM_A_DBS_BSTBL_0*/

typedef volatile union _CAM_REG_DBS_BSTBL_1_
{
		volatile struct	/* 0x1A004C08 */
		{
				FIELD  DBS_BIAS_Y4                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y5                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y6                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y7                           :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_1;	/* CAM_DBS_BSTBL_1, CAM_A_DBS_BSTBL_1*/

typedef volatile union _CAM_REG_DBS_BSTBL_2_
{
		volatile struct	/* 0x1A004C0C */
		{
				FIELD  DBS_BIAS_Y8                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y9                           :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y10                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_BIAS_Y11                          :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_2;	/* CAM_DBS_BSTBL_2, CAM_A_DBS_BSTBL_2*/

typedef volatile union _CAM_REG_DBS_BSTBL_3_
{
		volatile struct	/* 0x1A004C10 */
		{
				FIELD  DBS_BIAS_Y12                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_BIAS_Y13                          :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_BIAS_Y14                          :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_BSTBL_3;	/* CAM_DBS_BSTBL_3, CAM_A_DBS_BSTBL_3*/

typedef volatile union _CAM_REG_DBS_CTL_
{
		volatile struct	/* 0x1A004C14 */
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
}CAM_REG_DBS_CTL;	/* CAM_DBS_CTL, CAM_A_DBS_CTL*/

typedef volatile union _CAM_REG_DBS_CTL_2_
{
		volatile struct	/* 0x1A004C18 */
		{
				FIELD  DBS_HDR_OSCTH                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_CTL_2;	/* CAM_DBS_CTL_2, CAM_A_DBS_CTL_2*/

typedef volatile union _CAM_REG_DBS_SIGMA_2_
{
		volatile struct	/* 0x1A004C1C */
		{
				FIELD  DBS_MUL_B                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DBS_MUL_GB                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  DBS_MUL_GR                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DBS_MUL_R                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SIGMA_2;	/* CAM_DBS_SIGMA_2, CAM_A_DBS_SIGMA_2*/

typedef volatile union _CAM_REG_DBS_YGN_
{
		volatile struct	/* 0x1A004C20 */
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
}CAM_REG_DBS_YGN;	/* CAM_DBS_YGN, CAM_A_DBS_YGN*/

typedef volatile union _CAM_REG_DBS_SL_Y12_
{
		volatile struct	/* 0x1A004C24 */
		{
				FIELD  DBS_SL_Y1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_Y12;	/* CAM_DBS_SL_Y12, CAM_A_DBS_SL_Y12*/

typedef volatile union _CAM_REG_DBS_SL_Y34_
{
		volatile struct	/* 0x1A004C28 */
		{
				FIELD  DBS_SL_Y3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_Y4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_Y34;	/* CAM_DBS_SL_Y34, CAM_A_DBS_SL_Y34*/

typedef volatile union _CAM_REG_DBS_SL_G12_
{
		volatile struct	/* 0x1A004C2C */
		{
				FIELD  DBS_SL_G1                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G2                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_G12;	/* CAM_DBS_SL_G12, CAM_A_DBS_SL_G12*/

typedef volatile union _CAM_REG_DBS_SL_G34_
{
		volatile struct	/* 0x1A004C30 */
		{
				FIELD  DBS_SL_G3                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  DBS_SL_G4                             : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DBS_SL_G34;	/* CAM_DBS_SL_G34, CAM_A_DBS_SL_G34*/

typedef volatile union _CAM_REG_SL2F_CEN_
{
		volatile struct	/* 0x1A004C40 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_CEN;	/* CAM_SL2F_CEN, CAM_A_SL2F_CEN*/

typedef volatile union _CAM_REG_SL2F_RR_CON0_
{
		volatile struct	/* 0x1A004C44 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_RR_CON0;	/* CAM_SL2F_RR_CON0, CAM_A_SL2F_RR_CON0*/

typedef volatile union _CAM_REG_SL2F_RR_CON1_
{
		volatile struct	/* 0x1A004C48 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_RR_CON1;	/* CAM_SL2F_RR_CON1, CAM_A_SL2F_RR_CON1*/

typedef volatile union _CAM_REG_SL2F_GAIN_
{
		volatile struct	/* 0x1A004C4C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_GAIN;	/* CAM_SL2F_GAIN, CAM_A_SL2F_GAIN*/

typedef volatile union _CAM_REG_SL2F_RZ_
{
		volatile struct	/* 0x1A004C50 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_RZ;	/* CAM_SL2F_RZ, CAM_A_SL2F_RZ*/

typedef volatile union _CAM_REG_SL2F_XOFF_
{
		volatile struct	/* 0x1A004C54 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_XOFF;	/* CAM_SL2F_XOFF, CAM_A_SL2F_XOFF*/

typedef volatile union _CAM_REG_SL2F_YOFF_
{
		volatile struct	/* 0x1A004C58 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_YOFF;	/* CAM_SL2F_YOFF, CAM_A_SL2F_YOFF*/

typedef volatile union _CAM_REG_SL2F_SLP_CON0_
{
		volatile struct	/* 0x1A004C5C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_SLP_CON0;	/* CAM_SL2F_SLP_CON0, CAM_A_SL2F_SLP_CON0*/

typedef volatile union _CAM_REG_SL2F_SLP_CON1_
{
		volatile struct	/* 0x1A004C60 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_SLP_CON1;	/* CAM_SL2F_SLP_CON1, CAM_A_SL2F_SLP_CON1*/

typedef volatile union _CAM_REG_SL2F_SLP_CON2_
{
		volatile struct	/* 0x1A004C64 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_SLP_CON2;	/* CAM_SL2F_SLP_CON2, CAM_A_SL2F_SLP_CON2*/

typedef volatile union _CAM_REG_SL2F_SLP_CON3_
{
		volatile struct	/* 0x1A004C68 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_SLP_CON3;	/* CAM_SL2F_SLP_CON3, CAM_A_SL2F_SLP_CON3*/

typedef volatile union _CAM_REG_SL2F_SIZE_
{
		volatile struct	/* 0x1A004C6C */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2F_SIZE;	/* CAM_SL2F_SIZE, CAM_A_SL2F_SIZE*/

typedef volatile union _CAM_REG_SL2J_CEN_
{
		volatile struct	/* 0x1A004C80 */
		{
				FIELD  SL2_CENTR_X                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_CENTR_Y                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_CEN;	/* CAM_SL2J_CEN, CAM_A_SL2J_CEN*/

typedef volatile union _CAM_REG_SL2J_RR_CON0_
{
		volatile struct	/* 0x1A004C84 */
		{
				FIELD  SL2_R_0                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_R_1                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_RR_CON0;	/* CAM_SL2J_RR_CON0, CAM_A_SL2J_RR_CON0*/

typedef volatile union _CAM_REG_SL2J_RR_CON1_
{
		volatile struct	/* 0x1A004C88 */
		{
				FIELD  SL2_R_2                               : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_GAIN_0                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_GAIN_1                            :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_RR_CON1;	/* CAM_SL2J_RR_CON1, CAM_A_SL2J_RR_CON1*/

typedef volatile union _CAM_REG_SL2J_GAIN_
{
		volatile struct	/* 0x1A004C8C */
		{
				FIELD  SL2_GAIN_2                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SL2_GAIN_3                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SL2_GAIN_4                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SL2_SET_ZERO                          :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_GAIN;	/* CAM_SL2J_GAIN, CAM_A_SL2J_GAIN*/

typedef volatile union _CAM_REG_SL2J_RZ_
{
		volatile struct	/* 0x1A004C90 */
		{
				FIELD  SL2_HRZ_COMP                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_VRZ_COMP                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_RZ;	/* CAM_SL2J_RZ, CAM_A_SL2J_RZ*/

typedef volatile union _CAM_REG_SL2J_XOFF_
{
		volatile struct	/* 0x1A004C94 */
		{
				FIELD  SL2_X_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_XOFF;	/* CAM_SL2J_XOFF, CAM_A_SL2J_XOFF*/

typedef volatile union _CAM_REG_SL2J_YOFF_
{
		volatile struct	/* 0x1A004C98 */
		{
				FIELD  SL2_Y_OFST                            : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_YOFF;	/* CAM_SL2J_YOFF, CAM_A_SL2J_YOFF*/

typedef volatile union _CAM_REG_SL2J_SLP_CON0_
{
		volatile struct	/* 0x1A004C9C */
		{
				FIELD  SL2_SLP_1                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_SLP_CON0;	/* CAM_SL2J_SLP_CON0, CAM_A_SL2J_SLP_CON0*/

typedef volatile union _CAM_REG_SL2J_SLP_CON1_
{
		volatile struct	/* 0x1A004CA0 */
		{
				FIELD  SL2_SLP_2                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_SLP_CON1;	/* CAM_SL2J_SLP_CON1, CAM_A_SL2J_SLP_CON1*/

typedef volatile union _CAM_REG_SL2J_SLP_CON2_
{
		volatile struct	/* 0x1A004CA4 */
		{
				FIELD  SL2_SLP_3                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_SLP_CON2;	/* CAM_SL2J_SLP_CON2, CAM_A_SL2J_SLP_CON2*/

typedef volatile union _CAM_REG_SL2J_SLP_CON3_
{
		volatile struct	/* 0x1A004CA8 */
		{
				FIELD  SL2_SLP_4                             : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_SLP_CON3;	/* CAM_SL2J_SLP_CON3, CAM_A_SL2J_SLP_CON3*/

typedef volatile union _CAM_REG_SL2J_SIZE_
{
		volatile struct	/* 0x1A004CAC */
		{
				FIELD  SL2_TPIPE_WD                          : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SL2_TPIPE_HT                          : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SL2J_SIZE;	/* CAM_SL2J_SIZE, CAM_A_SL2J_SIZE*/

typedef volatile union _CAM_REG_PCP_CROP_CON1_
{
		volatile struct	/* 0x1A004CC0 */
		{
				FIELD  PCP_STR_X                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PCP_END_X                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PCP_CROP_CON1;	/* CAM_PCP_CROP_CON1, CAM_A_PCP_CROP_CON1*/

typedef volatile union _CAM_REG_PCP_CROP_CON2_
{
		volatile struct	/* 0x1A004CC4 */
		{
				FIELD  PCP_STR_Y                             : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PCP_END_Y                             : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PCP_CROP_CON2;	/* CAM_PCP_CROP_CON2, CAM_A_PCP_CROP_CON2*/

typedef volatile union _CAM_REG_SGG2_PGN_
{
		volatile struct	/* 0x1A004CD0 */
		{
				FIELD  SGG_GAIN                              : 11;		/*  0..10, 0x000007FF */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_PGN;	/* CAM_SGG2_PGN, CAM_A_SGG2_PGN*/

typedef volatile union _CAM_REG_SGG2_GMRC_1_
{
		volatile struct	/* 0x1A004CD4 */
		{
				FIELD  SGG_GMR_1                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_2                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_3                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  SGG_GMR_4                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_GMRC_1;	/* CAM_SGG2_GMRC_1, CAM_A_SGG2_GMRC_1*/

typedef volatile union _CAM_REG_SGG2_GMRC_2_
{
		volatile struct	/* 0x1A004CD8 */
		{
				FIELD  SGG_GMR_5                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SGG_GMR_6                             :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SGG_GMR_7                             :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_SGG2_GMRC_2;	/* CAM_SGG2_GMRC_2, CAM_A_SGG2_GMRC_2*/

typedef volatile union _CAM_REG_PSB_CON_
{
		volatile struct	/* 0x1A004CE0 */
		{
				FIELD  PSB_MODE                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  PSB_STR                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSB_CON;	/* CAM_PSB_CON, CAM_A_PSB_CON*/

typedef volatile union _CAM_REG_PSB_SIZE_
{
		volatile struct	/* 0x1A004CE4 */
		{
				FIELD  PSB_WD                                : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PSB_HT                                : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSB_SIZE;	/* CAM_PSB_SIZE, CAM_A_PSB_SIZE*/

typedef volatile union _CAM_REG_PDE_TBLI1_
{
		volatile struct	/* 0x1A004CF0 */
		{
				FIELD  PDE_XOFFSET                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PDE_YOFFSET                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDE_TBLI1;	/* CAM_PDE_TBLI1, CAM_A_PDE_TBLI1*/

typedef volatile union _CAM_REG_QBN4_MODE_
{
		volatile struct	/* 0x1A004D00 */
		{
				FIELD  QBN_ACC                               :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  QBN_ACC_MODE                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAM_REG_QBN4_MODE;	/* CAM_QBN4_MODE, CAM_A_QBN4_MODE*/

typedef volatile union _CAM_REG_PS_AWB_WIN_ORG_
{
		volatile struct	/* 0x1A004D10 */
		{
				FIELD  AWB_W_HORG                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VORG                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_WIN_ORG;	/* CAM_PS_AWB_WIN_ORG, CAM_A_PS_AWB_WIN_ORG*/

typedef volatile union _CAM_REG_PS_AWB_WIN_SIZE_
{
		volatile struct	/* 0x1A004D14 */
		{
				FIELD  AWB_W_HSIZE                           : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VSIZE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_WIN_SIZE;	/* CAM_PS_AWB_WIN_SIZE, CAM_A_PS_AWB_WIN_SIZE*/

typedef volatile union _CAM_REG_PS_AWB_WIN_PIT_
{
		volatile struct	/* 0x1A004D18 */
		{
				FIELD  AWB_W_HPIT                            : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  AWB_W_VPIT                            : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_WIN_PIT;	/* CAM_PS_AWB_WIN_PIT, CAM_A_PS_AWB_WIN_PIT*/

typedef volatile union _CAM_REG_PS_AWB_WIN_NUM_
{
		volatile struct	/* 0x1A004D1C */
		{
				FIELD  AWB_W_HNUM                            :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  AWB_W_VNUM                            :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_WIN_NUM;	/* CAM_PS_AWB_WIN_NUM, CAM_A_PS_AWB_WIN_NUM*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT0_
{
		volatile struct	/* 0x1A004D20 */
		{
				FIELD  AWB_PIXEL_CNT0                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT0;	/* CAM_PS_AWB_PIXEL_CNT0, CAM_A_PS_AWB_PIXEL_CNT0*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT1_
{
		volatile struct	/* 0x1A004D24 */
		{
				FIELD  AWB_PIXEL_CNT1                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT1;	/* CAM_PS_AWB_PIXEL_CNT1, CAM_A_PS_AWB_PIXEL_CNT1*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT2_
{
		volatile struct	/* 0x1A004D28 */
		{
				FIELD  AWB_PIXEL_CNT2                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT2;	/* CAM_PS_AWB_PIXEL_CNT2, CAM_A_PS_AWB_PIXEL_CNT2*/

typedef volatile union _CAM_REG_PS_AWB_PIXEL_CNT3_
{
		volatile struct	/* 0x1A004D2C */
		{
				FIELD  AWB_PIXEL_CNT3                        : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AWB_PIXEL_CNT3;	/* CAM_PS_AWB_PIXEL_CNT3, CAM_A_PS_AWB_PIXEL_CNT3*/

typedef volatile union _CAM_REG_PS_AE_YCOEF0_
{
		volatile struct	/* 0x1A004D30 */
		{
				FIELD  AE_YCOEF_R                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_YCOEF_GR                           : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AE_YCOEF0;	/* CAM_PS_AE_YCOEF0, CAM_A_PS_AE_YCOEF0*/

typedef volatile union _CAM_REG_PS_AE_YCOEF1_
{
		volatile struct	/* 0x1A004D34 */
		{
				FIELD  AE_YCOEF_GB                           : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  AE_YCOEF_B                            : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PS_AE_YCOEF1;	/* CAM_PS_AE_YCOEF1, CAM_A_PS_AE_YCOEF1*/

typedef volatile union _CAM_REG_PDI_BASE_ADDR_
{
		volatile struct	/* 0x1A004D50 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_BASE_ADDR;	/* CAM_PDI_BASE_ADDR, CAM_A_PDI_BASE_ADDR*/

typedef volatile union _CAM_REG_PDI_OFST_ADDR_
{
		volatile struct	/* 0x1A004D54 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_OFST_ADDR;	/* CAM_PDI_OFST_ADDR, CAM_A_PDI_OFST_ADDR*/

typedef volatile union _CAM_REG_PDI_DRS_
{
		volatile struct	/* 0x1A004D58 */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_DRS;	/* CAM_PDI_DRS, CAM_A_PDI_DRS*/

typedef volatile union _CAM_REG_PDI_XSIZE_
{
		volatile struct	/* 0x1A004D5C */
		{
				FIELD  XSIZE                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_XSIZE;	/* CAM_PDI_XSIZE, CAM_A_PDI_XSIZE*/

typedef volatile union _CAM_REG_PDI_YSIZE_
{
		volatile struct	/* 0x1A004D60 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_YSIZE;	/* CAM_PDI_YSIZE, CAM_A_PDI_YSIZE*/

typedef volatile union _CAM_REG_PDI_STRIDE_
{
		volatile struct	/* 0x1A004D64 */
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
}CAM_REG_PDI_STRIDE;	/* CAM_PDI_STRIDE, CAM_A_PDI_STRIDE*/

typedef volatile union _CAM_REG_PDI_CON_
{
		volatile struct	/* 0x1A004D68 */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_CON;	/* CAM_PDI_CON, CAM_A_PDI_CON*/

typedef volatile union _CAM_REG_PDI_CON2_
{
		volatile struct	/* 0x1A004D6C */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_CON2;	/* CAM_PDI_CON2, CAM_A_PDI_CON2*/

typedef volatile union _CAM_REG_PDI_CON3_
{
		volatile struct	/* 0x1A004D70 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_CON3;	/* CAM_PDI_CON3, CAM_A_PDI_CON3*/

typedef volatile union _CAM_REG_PDI_CON4_
{
		volatile struct	/* 0x1A004D74 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_CON4;	/* CAM_PDI_CON4, CAM_A_PDI_CON4*/

typedef volatile union _CAM_REG_PDI_ERR_STAT_
{
		volatile struct	/* 0x1A004D78 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDI_ERR_STAT;	/* CAM_PDI_ERR_STAT, CAM_A_PDI_ERR_STAT*/

typedef volatile union _CAM_REG_PSO_BASE_ADDR_
{
		volatile struct	/* 0x1A004D80 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_BASE_ADDR;	/* CAM_PSO_BASE_ADDR, CAM_A_PSO_BASE_ADDR*/

typedef volatile union _CAM_REG_PSO_OFST_ADDR_
{
		volatile struct	/* 0x1A004D88 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_OFST_ADDR;	/* CAM_PSO_OFST_ADDR, CAM_A_PSO_OFST_ADDR*/

typedef volatile union _CAM_REG_PSO_DRS_
{
		volatile struct	/* 0x1A004D8C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_DRS;	/* CAM_PSO_DRS, CAM_A_PSO_DRS*/

typedef volatile union _CAM_REG_PSO_XSIZE_
{
		volatile struct	/* 0x1A004D90 */
		{
				FIELD  XSIZE                                 : 18;		/*  0..17, 0x0003FFFF */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_XSIZE;	/* CAM_PSO_XSIZE, CAM_A_PSO_XSIZE*/

typedef volatile union _CAM_REG_PSO_YSIZE_
{
		volatile struct	/* 0x1A004D94 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_YSIZE;	/* CAM_PSO_YSIZE, CAM_A_PSO_YSIZE*/

typedef volatile union _CAM_REG_PSO_STRIDE_
{
		volatile struct	/* 0x1A004D98 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_STRIDE;	/* CAM_PSO_STRIDE, CAM_A_PSO_STRIDE*/

typedef volatile union _CAM_REG_PSO_CON_
{
		volatile struct	/* 0x1A004D9C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_CON;	/* CAM_PSO_CON, CAM_A_PSO_CON*/

typedef volatile union _CAM_REG_PSO_CON2_
{
		volatile struct	/* 0x1A004DA0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_CON2;	/* CAM_PSO_CON2, CAM_A_PSO_CON2*/

typedef volatile union _CAM_REG_PSO_CON3_
{
		volatile struct	/* 0x1A004DA4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_CON3;	/* CAM_PSO_CON3, CAM_A_PSO_CON3*/

typedef volatile union _CAM_REG_PSO_CON4_
{
		volatile struct	/* 0x1A004DA8 */
		{
				FIELD  FIFO_DVFS_THRL                        : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DVFS_THRH                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DVFS_EN                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_CON4;	/* CAM_PSO_CON4, CAM_A_PSO_CON4*/

typedef volatile union _CAM_REG_PSO_ERR_STAT_
{
		volatile struct	/* 0x1A004DAC */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_ERR_STAT;	/* CAM_PSO_ERR_STAT, CAM_A_PSO_ERR_STAT*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004DB0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_2;	/* CAM_PSO_FH_SPARE_2, CAM_A_PSO_FH_SPARE_2*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004DB4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_3;	/* CAM_PSO_FH_SPARE_3, CAM_A_PSO_FH_SPARE_3*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004DB8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_4;	/* CAM_PSO_FH_SPARE_4, CAM_A_PSO_FH_SPARE_4*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004DBC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_5;	/* CAM_PSO_FH_SPARE_5, CAM_A_PSO_FH_SPARE_5*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004DC0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_6;	/* CAM_PSO_FH_SPARE_6, CAM_A_PSO_FH_SPARE_6*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004DC4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_7;	/* CAM_PSO_FH_SPARE_7, CAM_A_PSO_FH_SPARE_7*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004DC8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_8;	/* CAM_PSO_FH_SPARE_8, CAM_A_PSO_FH_SPARE_8*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004DCC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_9;	/* CAM_PSO_FH_SPARE_9, CAM_A_PSO_FH_SPARE_9*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004DD0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_10;	/* CAM_PSO_FH_SPARE_10, CAM_A_PSO_FH_SPARE_10*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004DD4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_11;	/* CAM_PSO_FH_SPARE_11, CAM_A_PSO_FH_SPARE_11*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004DD8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_12;	/* CAM_PSO_FH_SPARE_12, CAM_A_PSO_FH_SPARE_12*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004DDC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_13;	/* CAM_PSO_FH_SPARE_13, CAM_A_PSO_FH_SPARE_13*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004DE0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_14;	/* CAM_PSO_FH_SPARE_14, CAM_A_PSO_FH_SPARE_14*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004DE4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_15;	/* CAM_PSO_FH_SPARE_15, CAM_A_PSO_FH_SPARE_15*/

typedef volatile union _CAM_REG_PSO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004DE8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_SPARE_16;	/* CAM_PSO_FH_SPARE_16, CAM_A_PSO_FH_SPARE_16*/

typedef volatile union _CAM_REG_DMA_FRAME_HEADER_EN_
{
		volatile struct	/* 0x1A004E00 */
		{
				FIELD  FRAME_HEADER_EN_IMGO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FRAME_HEADER_EN_RRZO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FRAME_HEADER_EN_AAO                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FRAME_HEADER_EN_AFO                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FRAME_HEADER_EN_LCSO                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FRAME_HEADER_EN_UFEO                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  FRAME_HEADER_EN_PDO                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  FRAME_HEADER_EN_PSO                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}CAM_REG_DMA_FRAME_HEADER_EN;	/* CAM_DMA_FRAME_HEADER_EN, CAM_A_DMA_FRAME_HEADER_EN*/

typedef volatile union _CAM_REG_IMGO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E04 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_BASE_ADDR;	/* CAM_IMGO_FH_BASE_ADDR, CAM_A_IMGO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_RRZO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E08 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_BASE_ADDR;	/* CAM_RRZO_FH_BASE_ADDR, CAM_A_RRZO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_AAO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E0C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_BASE_ADDR;	/* CAM_AAO_FH_BASE_ADDR, CAM_A_AAO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_AFO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E10 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_BASE_ADDR;	/* CAM_AFO_FH_BASE_ADDR, CAM_A_AFO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_LCSO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E14 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_BASE_ADDR;	/* CAM_LCSO_FH_BASE_ADDR, CAM_A_LCSO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_UFEO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E18 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_BASE_ADDR;	/* CAM_UFEO_FH_BASE_ADDR, CAM_A_UFEO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_PDO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E1C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_BASE_ADDR;	/* CAM_PDO_FH_BASE_ADDR, CAM_A_PDO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_PSO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A004E20 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PSO_FH_BASE_ADDR;	/* CAM_PSO_FH_BASE_ADDR, CAM_A_PSO_FH_BASE_ADDR*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004E30 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_2;	/* CAM_IMGO_FH_SPARE_2, CAM_A_IMGO_FH_SPARE_2*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004E34 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_3;	/* CAM_IMGO_FH_SPARE_3, CAM_A_IMGO_FH_SPARE_3*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004E38 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_4;	/* CAM_IMGO_FH_SPARE_4, CAM_A_IMGO_FH_SPARE_4*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004E3C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_5;	/* CAM_IMGO_FH_SPARE_5, CAM_A_IMGO_FH_SPARE_5*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004E40 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_6;	/* CAM_IMGO_FH_SPARE_6, CAM_A_IMGO_FH_SPARE_6*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004E44 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_7;	/* CAM_IMGO_FH_SPARE_7, CAM_A_IMGO_FH_SPARE_7*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004E48 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_8;	/* CAM_IMGO_FH_SPARE_8, CAM_A_IMGO_FH_SPARE_8*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004E4C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_9;	/* CAM_IMGO_FH_SPARE_9, CAM_A_IMGO_FH_SPARE_9*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004E50 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_10;	/* CAM_IMGO_FH_SPARE_10, CAM_A_IMGO_FH_SPARE_10*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004E54 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_11;	/* CAM_IMGO_FH_SPARE_11, CAM_A_IMGO_FH_SPARE_11*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004E58 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_12;	/* CAM_IMGO_FH_SPARE_12, CAM_A_IMGO_FH_SPARE_12*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004E5C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_13;	/* CAM_IMGO_FH_SPARE_13, CAM_A_IMGO_FH_SPARE_13*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004E60 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_14;	/* CAM_IMGO_FH_SPARE_14, CAM_A_IMGO_FH_SPARE_14*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004E64 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_15;	/* CAM_IMGO_FH_SPARE_15, CAM_A_IMGO_FH_SPARE_15*/

typedef volatile union _CAM_REG_IMGO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004E68 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_IMGO_FH_SPARE_16;	/* CAM_IMGO_FH_SPARE_16, CAM_A_IMGO_FH_SPARE_16*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004E70 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_2;	/* CAM_RRZO_FH_SPARE_2, CAM_A_RRZO_FH_SPARE_2*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004E74 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_3;	/* CAM_RRZO_FH_SPARE_3, CAM_A_RRZO_FH_SPARE_3*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004E78 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_4;	/* CAM_RRZO_FH_SPARE_4, CAM_A_RRZO_FH_SPARE_4*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004E7C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_5;	/* CAM_RRZO_FH_SPARE_5, CAM_A_RRZO_FH_SPARE_5*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004E80 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_6;	/* CAM_RRZO_FH_SPARE_6, CAM_A_RRZO_FH_SPARE_6*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004E84 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_7;	/* CAM_RRZO_FH_SPARE_7, CAM_A_RRZO_FH_SPARE_7*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004E88 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_8;	/* CAM_RRZO_FH_SPARE_8, CAM_A_RRZO_FH_SPARE_8*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004E8C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_9;	/* CAM_RRZO_FH_SPARE_9, CAM_A_RRZO_FH_SPARE_9*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004E90 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_10;	/* CAM_RRZO_FH_SPARE_10, CAM_A_RRZO_FH_SPARE_10*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004E94 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_11;	/* CAM_RRZO_FH_SPARE_11, CAM_A_RRZO_FH_SPARE_11*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004E98 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_12;	/* CAM_RRZO_FH_SPARE_12, CAM_A_RRZO_FH_SPARE_12*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004E9C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_13;	/* CAM_RRZO_FH_SPARE_13, CAM_A_RRZO_FH_SPARE_13*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004EA0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_14;	/* CAM_RRZO_FH_SPARE_14, CAM_A_RRZO_FH_SPARE_14*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004EA4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_15;	/* CAM_RRZO_FH_SPARE_15, CAM_A_RRZO_FH_SPARE_15*/

typedef volatile union _CAM_REG_RRZO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004EA8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_RRZO_FH_SPARE_16;	/* CAM_RRZO_FH_SPARE_16, CAM_A_RRZO_FH_SPARE_16*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004EB0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_2;	/* CAM_AAO_FH_SPARE_2, CAM_A_AAO_FH_SPARE_2*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004EB4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_3;	/* CAM_AAO_FH_SPARE_3, CAM_A_AAO_FH_SPARE_3*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004EB8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_4;	/* CAM_AAO_FH_SPARE_4, CAM_A_AAO_FH_SPARE_4*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004EBC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_5;	/* CAM_AAO_FH_SPARE_5, CAM_A_AAO_FH_SPARE_5*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004EC0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_6;	/* CAM_AAO_FH_SPARE_6, CAM_A_AAO_FH_SPARE_6*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004EC4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_7;	/* CAM_AAO_FH_SPARE_7, CAM_A_AAO_FH_SPARE_7*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004EC8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_8;	/* CAM_AAO_FH_SPARE_8, CAM_A_AAO_FH_SPARE_8*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004ECC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_9;	/* CAM_AAO_FH_SPARE_9, CAM_A_AAO_FH_SPARE_9*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004ED0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_10;	/* CAM_AAO_FH_SPARE_10, CAM_A_AAO_FH_SPARE_10*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004ED4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_11;	/* CAM_AAO_FH_SPARE_11, CAM_A_AAO_FH_SPARE_11*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004ED8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_12;	/* CAM_AAO_FH_SPARE_12, CAM_A_AAO_FH_SPARE_12*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004EDC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_13;	/* CAM_AAO_FH_SPARE_13, CAM_A_AAO_FH_SPARE_13*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004EE0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_14;	/* CAM_AAO_FH_SPARE_14, CAM_A_AAO_FH_SPARE_14*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004EE4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_15;	/* CAM_AAO_FH_SPARE_15, CAM_A_AAO_FH_SPARE_15*/

typedef volatile union _CAM_REG_AAO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004EE8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AAO_FH_SPARE_16;	/* CAM_AAO_FH_SPARE_16, CAM_A_AAO_FH_SPARE_16*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004EF0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_2;	/* CAM_AFO_FH_SPARE_2, CAM_A_AFO_FH_SPARE_2*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004EF4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_3;	/* CAM_AFO_FH_SPARE_3, CAM_A_AFO_FH_SPARE_3*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004EF8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_4;	/* CAM_AFO_FH_SPARE_4, CAM_A_AFO_FH_SPARE_4*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004EFC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_5;	/* CAM_AFO_FH_SPARE_5, CAM_A_AFO_FH_SPARE_5*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004F00 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_6;	/* CAM_AFO_FH_SPARE_6, CAM_A_AFO_FH_SPARE_6*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004F04 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_7;	/* CAM_AFO_FH_SPARE_7, CAM_A_AFO_FH_SPARE_7*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004F08 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_8;	/* CAM_AFO_FH_SPARE_8, CAM_A_AFO_FH_SPARE_8*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004F0C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_9;	/* CAM_AFO_FH_SPARE_9, CAM_A_AFO_FH_SPARE_9*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004F10 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_10;	/* CAM_AFO_FH_SPARE_10, CAM_A_AFO_FH_SPARE_10*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004F14 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_11;	/* CAM_AFO_FH_SPARE_11, CAM_A_AFO_FH_SPARE_11*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004F18 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_12;	/* CAM_AFO_FH_SPARE_12, CAM_A_AFO_FH_SPARE_12*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004F1C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_13;	/* CAM_AFO_FH_SPARE_13, CAM_A_AFO_FH_SPARE_13*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004F20 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_14;	/* CAM_AFO_FH_SPARE_14, CAM_A_AFO_FH_SPARE_14*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004F24 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_15;	/* CAM_AFO_FH_SPARE_15, CAM_A_AFO_FH_SPARE_15*/

typedef volatile union _CAM_REG_AFO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004F28 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_AFO_FH_SPARE_16;	/* CAM_AFO_FH_SPARE_16, CAM_A_AFO_FH_SPARE_16*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004F30 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_2;	/* CAM_LCSO_FH_SPARE_2, CAM_A_LCSO_FH_SPARE_2*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004F34 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_3;	/* CAM_LCSO_FH_SPARE_3, CAM_A_LCSO_FH_SPARE_3*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004F38 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_4;	/* CAM_LCSO_FH_SPARE_4, CAM_A_LCSO_FH_SPARE_4*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004F3C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_5;	/* CAM_LCSO_FH_SPARE_5, CAM_A_LCSO_FH_SPARE_5*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004F40 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_6;	/* CAM_LCSO_FH_SPARE_6, CAM_A_LCSO_FH_SPARE_6*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004F44 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_7;	/* CAM_LCSO_FH_SPARE_7, CAM_A_LCSO_FH_SPARE_7*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004F48 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_8;	/* CAM_LCSO_FH_SPARE_8, CAM_A_LCSO_FH_SPARE_8*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004F4C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_9;	/* CAM_LCSO_FH_SPARE_9, CAM_A_LCSO_FH_SPARE_9*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004F50 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_10;	/* CAM_LCSO_FH_SPARE_10, CAM_A_LCSO_FH_SPARE_10*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004F54 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_11;	/* CAM_LCSO_FH_SPARE_11, CAM_A_LCSO_FH_SPARE_11*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004F58 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_12;	/* CAM_LCSO_FH_SPARE_12, CAM_A_LCSO_FH_SPARE_12*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004F5C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_13;	/* CAM_LCSO_FH_SPARE_13, CAM_A_LCSO_FH_SPARE_13*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004F60 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_14;	/* CAM_LCSO_FH_SPARE_14, CAM_A_LCSO_FH_SPARE_14*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004F64 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_15;	/* CAM_LCSO_FH_SPARE_15, CAM_A_LCSO_FH_SPARE_15*/

typedef volatile union _CAM_REG_LCSO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004F68 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_LCSO_FH_SPARE_16;	/* CAM_LCSO_FH_SPARE_16, CAM_A_LCSO_FH_SPARE_16*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004F70 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_2;	/* CAM_UFEO_FH_SPARE_2, CAM_A_UFEO_FH_SPARE_2*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004F74 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_3;	/* CAM_UFEO_FH_SPARE_3, CAM_A_UFEO_FH_SPARE_3*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004F78 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_4;	/* CAM_UFEO_FH_SPARE_4, CAM_A_UFEO_FH_SPARE_4*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004F7C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_5;	/* CAM_UFEO_FH_SPARE_5, CAM_A_UFEO_FH_SPARE_5*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004F80 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_6;	/* CAM_UFEO_FH_SPARE_6, CAM_A_UFEO_FH_SPARE_6*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004F84 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_7;	/* CAM_UFEO_FH_SPARE_7, CAM_A_UFEO_FH_SPARE_7*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004F88 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_8;	/* CAM_UFEO_FH_SPARE_8, CAM_A_UFEO_FH_SPARE_8*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004F8C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_9;	/* CAM_UFEO_FH_SPARE_9, CAM_A_UFEO_FH_SPARE_9*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004F90 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_10;	/* CAM_UFEO_FH_SPARE_10, CAM_A_UFEO_FH_SPARE_10*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004F94 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_11;	/* CAM_UFEO_FH_SPARE_11, CAM_A_UFEO_FH_SPARE_11*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004F98 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_12;	/* CAM_UFEO_FH_SPARE_12, CAM_A_UFEO_FH_SPARE_12*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004F9C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_13;	/* CAM_UFEO_FH_SPARE_13, CAM_A_UFEO_FH_SPARE_13*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004FA0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_14;	/* CAM_UFEO_FH_SPARE_14, CAM_A_UFEO_FH_SPARE_14*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004FA4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_15;	/* CAM_UFEO_FH_SPARE_15, CAM_A_UFEO_FH_SPARE_15*/

typedef volatile union _CAM_REG_UFEO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004FA8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_UFEO_FH_SPARE_16;	/* CAM_UFEO_FH_SPARE_16, CAM_A_UFEO_FH_SPARE_16*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_2_
{
		volatile struct	/* 0x1A004FB0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_2;	/* CAM_PDO_FH_SPARE_2, CAM_A_PDO_FH_SPARE_2*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_3_
{
		volatile struct	/* 0x1A004FB4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_3;	/* CAM_PDO_FH_SPARE_3, CAM_A_PDO_FH_SPARE_3*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_4_
{
		volatile struct	/* 0x1A004FB8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_4;	/* CAM_PDO_FH_SPARE_4, CAM_A_PDO_FH_SPARE_4*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_5_
{
		volatile struct	/* 0x1A004FBC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_5;	/* CAM_PDO_FH_SPARE_5, CAM_A_PDO_FH_SPARE_5*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_6_
{
		volatile struct	/* 0x1A004FC0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_6;	/* CAM_PDO_FH_SPARE_6, CAM_A_PDO_FH_SPARE_6*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_7_
{
		volatile struct	/* 0x1A004FC4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_7;	/* CAM_PDO_FH_SPARE_7, CAM_A_PDO_FH_SPARE_7*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_8_
{
		volatile struct	/* 0x1A004FC8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_8;	/* CAM_PDO_FH_SPARE_8, CAM_A_PDO_FH_SPARE_8*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_9_
{
		volatile struct	/* 0x1A004FCC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_9;	/* CAM_PDO_FH_SPARE_9, CAM_A_PDO_FH_SPARE_9*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_10_
{
		volatile struct	/* 0x1A004FD0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_10;	/* CAM_PDO_FH_SPARE_10, CAM_A_PDO_FH_SPARE_10*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_11_
{
		volatile struct	/* 0x1A004FD4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_11;	/* CAM_PDO_FH_SPARE_11, CAM_A_PDO_FH_SPARE_11*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_12_
{
		volatile struct	/* 0x1A004FD8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_12;	/* CAM_PDO_FH_SPARE_12, CAM_A_PDO_FH_SPARE_12*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_13_
{
		volatile struct	/* 0x1A004FDC */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_13;	/* CAM_PDO_FH_SPARE_13, CAM_A_PDO_FH_SPARE_13*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_14_
{
		volatile struct	/* 0x1A004FE0 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_14;	/* CAM_PDO_FH_SPARE_14, CAM_A_PDO_FH_SPARE_14*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_15_
{
		volatile struct	/* 0x1A004FE4 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_15;	/* CAM_PDO_FH_SPARE_15, CAM_A_PDO_FH_SPARE_15*/

typedef volatile union _CAM_REG_PDO_FH_SPARE_16_
{
		volatile struct	/* 0x1A004FE8 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAM_REG_PDO_FH_SPARE_16;	/* CAM_PDO_FH_SPARE_16, CAM_A_PDO_FH_SPARE_16*/

typedef volatile union _CAMSV_REG_TOP_DEBUG_
{
		volatile struct	/* 0x1A050000 */
		{
				FIELD  DEBUG                                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TOP_DEBUG;	/* CAMSV_TOP_DEBUG, CAMSV_0_TOP_DEBUG*/

typedef volatile union _CAMSV_REG_MODULE_EN_
{
		volatile struct	/* 0x1A050010 */
		{
				FIELD  TG_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_EN                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  PAK_SEL                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  IMGO_EN                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  DOWN_SAMPLE_EN                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  DOWN_SAMPLE_PERIOD                    :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  SW_PASS1_DONE_FRAME_CNT               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  8;		/* 22..29, 0x3FC00000 */
				FIELD  DB_EN                                 :  1;		/* 30..30, 0x40000000 */
				FIELD  DB_LOCK                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_MODULE_EN;	/* CAMSV_MODULE_EN, CAMSV_0_MODULE_EN*/

typedef volatile union _CAMSV_REG_FMT_SEL_
{
		volatile struct	/* 0x1A050014 */
		{
				FIELD  TG1_FMT                               :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TG1_SW                                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  LP_MODE                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HLR_MODE                              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                : 22;		/* 10..31, 0xFFFFFC00 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FMT_SEL;	/* CAMSV_FMT_SEL, CAMSV_0_FMT_SEL*/

typedef volatile union _CAMSV_REG_INT_EN_
{
		volatile struct	/* 0x1A050018 */
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
				FIELD  SW_PASS1_DON_INT_EN                   :  1;		/* 20..20, 0x00100000 */
				FIELD  TG_SOF_WAIT_INT_EN                    :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  9;		/* 22..30, 0x7FC00000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_INT_EN;	/* CAMSV_INT_EN, CAMSV_0_INT_EN*/

typedef volatile union _CAMSV_REG_INT_STATUS_
{
		volatile struct	/* 0x1A05001C */
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
				FIELD  SW_PASS1_DON_ST                       :  1;		/* 20..20, 0x00100000 */
				FIELD  TG_SOF_WAIT_ST                        :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_INT_STATUS;	/* CAMSV_INT_STATUS, CAMSV_0_INT_STATUS*/

typedef volatile union _CAMSV_REG_SW_CTL_
{
		volatile struct	/* 0x1A050020 */
		{
				FIELD  IMGO_RST_TRIG                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_RST_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SW_RST                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SW_CTL;	/* CAMSV_SW_CTL, CAMSV_0_SW_CTL*/

typedef volatile union _CAMSV_REG_SPARE0_
{
		volatile struct	/* 0x1A050024 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SPARE0;	/* CAMSV_SPARE0, CAMSV_0_SPARE0*/

typedef volatile union _CAMSV_REG_SPARE1_
{
		volatile struct	/* 0x1A050028 */
		{
				FIELD  CTL_SPARE1                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SPARE1;	/* CAMSV_SPARE1, CAMSV_0_SPARE1*/

typedef volatile union _CAMSV_REG_IMGO_FBC_
{
		volatile struct	/* 0x1A05002C */
		{
				FIELD  RCNT_INC                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FBC;	/* CAMSV_IMGO_FBC, CAMSV_0_IMGO_FBC*/

typedef volatile union _CAMSV_REG_CLK_EN_
{
		volatile struct	/* 0x1A050030 */
		{
				FIELD  TG_DP_CK_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  PAK_DP_CK_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 12;		/*  3..14, 0x00007FF8 */
				FIELD  DMA_DP_CK_EN                          :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_CLK_EN;	/* CAMSV_CLK_EN, CAMSV_0_CLK_EN*/

typedef volatile union _CAMSV_REG_DBG_SET_
{
		volatile struct	/* 0x1A050034 */
		{
				FIELD  DEBUG_MOD_SEL                         :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DEBUG_SEL                             : 12;		/*  8..19, 0x000FFF00 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DBG_SET;	/* CAMSV_DBG_SET, CAMSV_0_DBG_SET*/

typedef volatile union _CAMSV_REG_DBG_PORT_
{
		volatile struct	/* 0x1A050038 */
		{
				FIELD  CTL_DBG_PORT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DBG_PORT;	/* CAMSV_DBG_PORT, CAMSV_0_DBG_PORT*/

typedef volatile union _CAMSV_REG_DATE_CODE_
{
		volatile struct	/* 0x1A05003C */
		{
				FIELD  CTL_DATE_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DATE_CODE;	/* CAMSV_DATE_CODE, CAMSV_0_DATE_CODE*/

typedef volatile union _CAMSV_REG_PROJ_CODE_
{
		volatile struct	/* 0x1A050040 */
		{
				FIELD  CTL_PROJ_CODE                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_PROJ_CODE;	/* CAMSV_PROJ_CODE, CAMSV_0_PROJ_CODE*/

typedef volatile union _CAMSV_REG_DCM_DIS_
{
		volatile struct	/* 0x1A050044 */
		{
				FIELD  PAK_DCM_DIS                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_DIS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DCM_DIS;	/* CAMSV_DCM_DIS, CAMSV_0_DCM_DIS*/

typedef volatile union _CAMSV_REG_DCM_STATUS_
{
		volatile struct	/* 0x1A050048 */
		{
				FIELD  PAK_DCM_ST                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  IMGO_DCM_ST                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DCM_STATUS;	/* CAMSV_DCM_STATUS, CAMSV_0_DCM_STATUS*/

typedef volatile union _CAMSV_REG_PAK_
{
		volatile struct	/* 0x1A05004C */
		{
				FIELD  PAK_MODE                              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PAK_DBL_MODE                          :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_PAK;	/* CAMSV_PAK, CAMSV_0_PAK*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CTL1_
{
		volatile struct	/* 0x1A050110 */
		{
				FIELD  FBC_NUM                               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  FBC_RESET                             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  6;		/*  9..14, 0x00007E00 */
				FIELD  FBC_EN                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FBC_MODE                              :  1;		/* 16..16, 0x00010000 */
				FIELD  LOCK_EN                               :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  DROP_TIMING                           :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  DMA_RING_EN                           :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  SUB_RATIO                             :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CTL1;	/* CAMSV_FBC_IMGO_CTL1, CAMSV_0_FBC_IMGO_CTL1*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CTL2_
{
		volatile struct	/* 0x1A050114 */
		{
				FIELD  FBC_CNT                               :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RCNT                                  :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WCNT                                  :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  DROP_CNT                              :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CTL2;	/* CAMSV_FBC_IMGO_CTL2, CAMSV_0_FBC_IMGO_CTL2*/

typedef volatile union _CAMSV_REG_FBC_IMGO_ENQ_ADDR_
{
		volatile struct	/* 0x1A050118 */
		{
				FIELD  IMGO_ENQ_ADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FBC_IMGO_ENQ_ADDR;	/* CAMSV_FBC_IMGO_ENQ_ADDR, CAMSV_0_FBC_IMGO_ENQ_ADDR*/

typedef volatile union _CAMSV_REG_FBC_IMGO_CUR_ADDR_
{
		volatile struct	/* 0x1A05011C */
		{
				FIELD  IMGO_CUR_ADDR                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_FBC_IMGO_CUR_ADDR;	/* CAMSV_FBC_IMGO_CUR_ADDR, CAMSV_0_FBC_IMGO_CUR_ADDR*/

typedef volatile union _CAMSV_REG_DMA_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1A050200 */
		{
				FIELD  IMGO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_SOFT_RST_STAT                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_SOFT_RST_STAT                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_SOFT_RST_STAT                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_SOFT_RST_STAT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_SOFT_RST_STAT                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_SOFT_RST_STAT                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  BPCI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_SOFT_RST_STAT                    :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_SOFT_RST_STAT                    :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_SOFT_RSTSTAT;	/* CAMSV_DMA_SOFT_RSTSTAT, CAMSV_0_DMA_SOFT_RSTSTAT*/

typedef volatile union _CAMSV_REG_CQ0I_BASE_ADDR_
{
		volatile struct	/* 0x1A050204 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_CQ0I_BASE_ADDR;	/* CAMSV_CQ0I_BASE_ADDR, CAMSV_0_CQ0I_BASE_ADDR*/

typedef volatile union _CAMSV_REG_CQ0I_XSIZE_
{
		volatile struct	/* 0x1A050208 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_CQ0I_XSIZE;	/* CAMSV_CQ0I_XSIZE, CAMSV_0_CQ0I_XSIZE*/

typedef volatile union _CAMSV_REG_VERTICAL_FLIP_EN_
{
		volatile struct	/* 0x1A05020C */
		{
				FIELD  IMGO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_V_FLIP_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_V_FLIP_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_V_FLIP_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_V_FLIP_EN                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_V_FLIP_EN                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_V_FLIP_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  BPCI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_V_FLIP_EN                        :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_V_FLIP_EN                        :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_VERTICAL_FLIP_EN;	/* CAMSV_VERTICAL_FLIP_EN, CAMSV_0_VERTICAL_FLIP_EN*/

typedef volatile union _CAMSV_REG_DMA_SOFT_RESET_
{
		volatile struct	/* 0x1A050210 */
		{
				FIELD  IMGO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_SOFT_RST                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_SOFT_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_SOFT_RST                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_SOFT_RST                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_SOFT_RST                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_SOFT_RST                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  BPCI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_SOFT_RST                         :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_SOFT_RST                         :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 12;		/* 19..30, 0x7FF80000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_SOFT_RESET;	/* CAMSV_DMA_SOFT_RESET, CAMSV_0_DMA_SOFT_RESET*/

typedef volatile union _CAMSV_REG_LAST_ULTRA_EN_
{
		volatile struct	/* 0x1A050214 */
		{
				FIELD  IMGO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_LAST_ULTRA_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_LAST_ULTRA_EN                     :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_LAST_ULTRA_EN                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_LAST_ULTRA_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_LAST_ULTRA_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_LAST_ULTRA_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  BPCI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_LAST_ULTRA_EN                    :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_LAST_ULTRA_EN                    :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 13;		/* 19..31, 0xFFF80000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_LAST_ULTRA_EN;	/* CAMSV_LAST_ULTRA_EN, CAMSV_0_LAST_ULTRA_EN*/

typedef volatile union _CAMSV_REG_SPECIAL_FUN_EN_
{
		volatile struct	/* 0x1A050218 */
		{
				FIELD  rsv_0                                 : 20;		/*  0..19, 0x000FFFFF */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  MULTI_PLANE_ID_EN                     :  1;		/* 24..24, 0x01000000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 25..25, 0x02000000 */
				FIELD  FIFO_CHANGE_EN                        :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  2;		/* 27..28, 0x18000000 */
				FIELD  CQ_ULTRA_BPCI_EN                      :  1;		/* 29..29, 0x20000000 */
				FIELD  CQ_ULTRA_LSCI_EN                      :  1;		/* 30..30, 0x40000000 */
				FIELD  UFO_IMGO_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_SPECIAL_FUN_EN;	/* CAMSV_SPECIAL_FUN_EN, CAMSV_0_SPECIAL_FUN_EN*/

typedef volatile union _CAMSV_REG_IMGO_BASE_ADDR_
{
		volatile struct	/* 0x1A050220 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_BASE_ADDR;	/* CAMSV_IMGO_BASE_ADDR, CAMSV_0_IMGO_BASE_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_OFST_ADDR_
{
		volatile struct	/* 0x1A050228 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_OFST_ADDR;	/* CAMSV_IMGO_OFST_ADDR, CAMSV_0_IMGO_OFST_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_DRS_
{
		volatile struct	/* 0x1A05022C */
		{
				FIELD  FIFO_DRS_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_DRS_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  3;		/* 28..30, 0x70000000 */
				FIELD  FIFO_DRS_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_DRS;	/* CAMSV_IMGO_DRS, CAMSV_0_IMGO_DRS*/

typedef volatile union _CAMSV_REG_IMGO_XSIZE_
{
		volatile struct	/* 0x1A050230 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_XSIZE;	/* CAMSV_IMGO_XSIZE, CAMSV_0_IMGO_XSIZE*/

typedef volatile union _CAMSV_REG_IMGO_YSIZE_
{
		volatile struct	/* 0x1A050234 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_YSIZE;	/* CAMSV_IMGO_YSIZE, CAMSV_0_IMGO_YSIZE*/

typedef volatile union _CAMSV_REG_IMGO_STRIDE_
{
		volatile struct	/* 0x1A050238 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_STRIDE;	/* CAMSV_IMGO_STRIDE, CAMSV_0_IMGO_STRIDE*/

typedef volatile union _CAMSV_REG_IMGO_CON_
{
		volatile struct	/* 0x1A05023C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_CON;	/* CAMSV_IMGO_CON, CAMSV_0_IMGO_CON*/

typedef volatile union _CAMSV_REG_IMGO_CON2_
{
		volatile struct	/* 0x1A050240 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_CON2;	/* CAMSV_IMGO_CON2, CAMSV_0_IMGO_CON2*/

typedef volatile union _CAMSV_REG_IMGO_CON3_
{
		volatile struct	/* 0x1A050244 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_CON3;	/* CAMSV_IMGO_CON3, CAMSV_0_IMGO_CON3*/

typedef volatile union _CAMSV_REG_IMGO_CROP_
{
		volatile struct	/* 0x1A050248 */
		{
				FIELD  XOFFSET                               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  YOFFSET                               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_CROP;	/* CAMSV_IMGO_CROP, CAMSV_0_IMGO_CROP*/

typedef volatile union _CAMSV_REG_DMA_ERR_CTRL_
{
		volatile struct	/* 0x1A050440 */
		{
				FIELD  IMGO_A_ERR                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RRZO_A_ERR                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  AAO_A_ERR                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  AFO_A_ERR                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  LCSO_A_ERR                            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  UFEO_A_ERR                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PDO_A_ERR                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  BPCI_A_ERR                            :  1;		/* 16..16, 0x00010000 */
				FIELD  CACI_A_ERR                            :  1;		/* 17..17, 0x00020000 */
				FIELD  LSCI_A_ERR                            :  1;		/* 18..18, 0x00040000 */
				FIELD  rsv_19                                : 12;		/* 19..30, 0x7FF80000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_ERR_CTRL;	/* CAMSV_DMA_ERR_CTRL, CAMSV_0_DMA_ERR_CTRL*/

typedef volatile union _CAMSV_REG_IMGO_ERR_STAT_
{
		volatile struct	/* 0x1A050444 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_ERR_STAT;	/* CAMSV_IMGO_ERR_STAT, CAMSV_0_IMGO_ERR_STAT*/

typedef volatile union _CAMSV_REG_DMA_DEBUG_ADDR_
{
		volatile struct	/* 0x1A050470 */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_DEBUG_ADDR;	/* CAMSV_DMA_DEBUG_ADDR, CAMSV_0_DMA_DEBUG_ADDR*/

typedef volatile union _CAMSV_REG_DMA_RSV1_
{
		volatile struct	/* 0x1A050474 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV1;	/* CAMSV_DMA_RSV1, CAMSV_0_DMA_RSV1*/

typedef volatile union _CAMSV_REG_DMA_RSV2_
{
		volatile struct	/* 0x1A050478 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV2;	/* CAMSV_DMA_RSV2, CAMSV_0_DMA_RSV2*/

typedef volatile union _CAMSV_REG_DMA_RSV3_
{
		volatile struct	/* 0x1A05047C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV3;	/* CAMSV_DMA_RSV3, CAMSV_0_DMA_RSV3*/

typedef volatile union _CAMSV_REG_DMA_RSV4_
{
		volatile struct	/* 0x1A050480 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV4;	/* CAMSV_DMA_RSV4, CAMSV_0_DMA_RSV4*/

typedef volatile union _CAMSV_REG_DMA_RSV5_
{
		volatile struct	/* 0x1A050484 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV5;	/* CAMSV_DMA_RSV5, CAMSV_0_DMA_RSV5*/

typedef volatile union _CAMSV_REG_DMA_RSV6_
{
		volatile struct	/* 0x1A050488 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_RSV6;	/* CAMSV_DMA_RSV6, CAMSV_0_DMA_RSV6*/

typedef volatile union _CAMSV_REG_DMA_DEBUG_SEL_
{
		volatile struct	/* 0x1A05048C */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PDO_FIFO_FULL_XSIZE                   :  1;		/* 28..28, 0x10000000 */
				FIELD  IMGO_UFE_FIFO_FULL_XSIZE              :  1;		/* 29..29, 0x20000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_DEBUG_SEL;	/* CAMSV_DMA_DEBUG_SEL, CAMSV_0_DMA_DEBUG_SEL*/

typedef volatile union _CAMSV_REG_DMA_BW_SELF_TEST_
{
		volatile struct	/* 0x1A050490 */
		{
				FIELD  BW_SELF_TEST_EN_IMGO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  BW_SELF_TEST_EN_RRZO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  BW_SELF_TEST_EN_AAO                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  BW_SELF_TEST_EN_AFO                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  BW_SELF_TEST_EN_LCSO                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  BW_SELF_TEST_EN_UFEO                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  BW_SELF_TEST_EN_PDO                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_BW_SELF_TEST;	/* CAMSV_DMA_BW_SELF_TEST, CAMSV_0_DMA_BW_SELF_TEST*/

typedef volatile union _CAMSV_REG_TG_SEN_MODE_
{
		volatile struct	/* 0x1A050500 */
		{
				FIELD  CMOS_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DBL_DATA_BUS                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SOT_MODE                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SOT_CLR_MODE                          :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SOF_SRC                               :  2;		/*  8.. 9, 0x00000300 */
				FIELD  EOF_SRC                               :  2;		/* 10..11, 0x00000C00 */
				FIELD  PXL_CNT_RST_SRC                       :  1;		/* 12..12, 0x00001000 */
				FIELD  DBL_DATA_BUS1                         :  1;		/* 13..13, 0x00002000 */
				FIELD  SOF_WAIT_CQ                           :  1;		/* 14..14, 0x00004000 */
				FIELD  FIFO_FULL_CTL_EN                      :  1;		/* 15..15, 0x00008000 */
				FIELD  TIME_STP_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  VS_SUB_EN                             :  1;		/* 17..17, 0x00020000 */
				FIELD  SOF_SUB_EN                            :  1;		/* 18..18, 0x00040000 */
				FIELD  I2C_CQ_EN                             :  1;		/* 19..19, 0x00080000 */
				FIELD  EOF_ALS_RDY_EN                        :  1;		/* 20..20, 0x00100000 */
				FIELD  CQ_SEL                                :  1;		/* 21..21, 0x00200000 */
				FIELD  TG_STAGER_SENSOR_EN                   :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_MODE;	/* CAMSV_TG_SEN_MODE, CAMSV_0_TG_SEN_MODE*/

typedef volatile union _CAMSV_REG_TG_VF_CON_
{
		volatile struct	/* 0x1A050504 */
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
}CAMSV_REG_TG_VF_CON;	/* CAMSV_TG_VF_CON, CAMSV_0_TG_VF_CON*/

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_PXL_
{
		volatile struct	/* 0x1A050508 */
		{
				FIELD  PXL_S                                 : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PXL_E                                 : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_PXL;	/* CAMSV_TG_SEN_GRAB_PXL, CAMSV_0_TG_SEN_GRAB_PXL*/

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_LIN_
{
		volatile struct	/* 0x1A05050C */
		{
				FIELD  LIN_S                                 : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  LIN_E                                 : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SEN_GRAB_LIN;	/* CAMSV_TG_SEN_GRAB_LIN, CAMSV_0_TG_SEN_GRAB_LIN*/

typedef volatile union _CAMSV_REG_TG_PATH_CFG_
{
		volatile struct	/* 0x1A050510 */
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
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  YUV_U2S_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  YUV_BIN_EN                            :  1;		/* 13..13, 0x00002000 */
				FIELD  TG_ERR_SEL                            :  1;		/* 14..14, 0x00004000 */
				FIELD  TG_FULL_SEL                           :  1;		/* 15..15, 0x00008000 */
				FIELD  TG_FULL_SEL2                          :  1;		/* 16..16, 0x00010000 */
				FIELD  FLUSH_DISABLE                         :  1;		/* 17..17, 0x00020000 */
				FIELD  INT_BANK_DISABLE                      :  1;		/* 18..18, 0x00040000 */
				FIELD  EXP_ESC                               :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_PATH_CFG;	/* CAMSV_TG_PATH_CFG, CAMSV_0_TG_PATH_CFG*/

typedef volatile union _CAMSV_REG_TG_MEMIN_CTL_
{
		volatile struct	/* 0x1A050514 */
		{
				FIELD  MEMIN_DUMMYPXL                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MEMIN_DUMMYLIN                        :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_MEMIN_CTL;	/* CAMSV_TG_MEMIN_CTL, CAMSV_0_TG_MEMIN_CTL*/

typedef volatile union _CAMSV_REG_TG_INT1_
{
		volatile struct	/* 0x1A050518 */
		{
				FIELD  TG_INT1_LINENO                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  TG_INT1_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  VSYNC_INT_POL                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_INT1;	/* CAMSV_TG_INT1, CAMSV_0_TG_INT1*/

typedef volatile union _CAMSV_REG_TG_INT2_
{
		volatile struct	/* 0x1A05051C */
		{
				FIELD  TG_INT2_LINENO                        : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  TG_INT2_PXLNO                         : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_INT2;	/* CAMSV_TG_INT2, CAMSV_0_TG_INT2*/

typedef volatile union _CAMSV_REG_TG_SOF_CNT_
{
		volatile struct	/* 0x1A050520 */
		{
				FIELD  SOF_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SOF_CNT;	/* CAMSV_TG_SOF_CNT, CAMSV_0_TG_SOF_CNT*/

typedef volatile union _CAMSV_REG_TG_SOT_CNT_
{
		volatile struct	/* 0x1A050524 */
		{
				FIELD  SOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SOT_CNT;	/* CAMSV_TG_SOT_CNT, CAMSV_0_TG_SOT_CNT*/

typedef volatile union _CAMSV_REG_TG_EOT_CNT_
{
		volatile struct	/* 0x1A050528 */
		{
				FIELD  EOT_CNT                               : 28;		/*  0..27, 0x0FFFFFFF */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_EOT_CNT;	/* CAMSV_TG_EOT_CNT, CAMSV_0_TG_EOT_CNT*/

typedef volatile union _CAMSV_REG_TG_ERR_CTL_
{
		volatile struct	/* 0x1A05052C */
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
}CAMSV_REG_TG_ERR_CTL;	/* CAMSV_TG_ERR_CTL, CAMSV_0_TG_ERR_CTL*/

typedef volatile union _CAMSV_REG_TG_DAT_NO_
{
		volatile struct	/* 0x1A050530 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_DAT_NO;	/* CAMSV_TG_DAT_NO, CAMSV_0_TG_DAT_NO*/

typedef volatile union _CAMSV_REG_TG_FRM_CNT_ST_
{
		volatile struct	/* 0x1A050534 */
		{
				FIELD  REZ_OVRUN_FCNT                        :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  GRAB_ERR_FCNT                         :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FRM_CNT_ST;	/* CAMSV_TG_FRM_CNT_ST, CAMSV_0_TG_FRM_CNT_ST*/

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_
{
		volatile struct	/* 0x1A050538 */
		{
				FIELD  LINE_CNT                              : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FRMSIZE_ST;	/* CAMSV_TG_FRMSIZE_ST, CAMSV_0_TG_FRMSIZE_ST*/

typedef volatile union _CAMSV_REG_TG_INTER_ST_
{
		volatile struct	/* 0x1A05053C */
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
}CAMSV_REG_TG_INTER_ST;	/* CAMSV_TG_INTER_ST, CAMSV_0_TG_INTER_ST*/

typedef volatile union _CAMSV_REG_TG_FLASHA_CTL_
{
		volatile struct	/* 0x1A050540 */
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
}CAMSV_REG_TG_FLASHA_CTL;	/* CAMSV_TG_FLASHA_CTL, CAMSV_0_TG_FLASHA_CTL*/

typedef volatile union _CAMSV_REG_TG_FLASHA_LINE_CNT_
{
		volatile struct	/* 0x1A050544 */
		{
				FIELD  FLASHA_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHA_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHA_LINE_CNT;	/* CAMSV_TG_FLASHA_LINE_CNT, CAMSV_0_TG_FLASHA_LINE_CNT*/

typedef volatile union _CAMSV_REG_TG_FLASHA_POS_
{
		volatile struct	/* 0x1A050548 */
		{
				FIELD  FLASHA_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHA_LINE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHA_POS;	/* CAMSV_TG_FLASHA_POS, CAMSV_0_TG_FLASHA_POS*/

typedef volatile union _CAMSV_REG_TG_FLASHB_CTL_
{
		volatile struct	/* 0x1A05054C */
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
}CAMSV_REG_TG_FLASHB_CTL;	/* CAMSV_TG_FLASHB_CTL, CAMSV_0_TG_FLASHB_CTL*/

typedef volatile union _CAMSV_REG_TG_FLASHB_LINE_CNT_
{
		volatile struct	/* 0x1A050550 */
		{
				FIELD  FLASHB_LUNIT_NO                       : 20;		/*  0..19, 0x000FFFFF */
				FIELD  FLASHB_LUNIT                          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_LINE_CNT;	/* CAMSV_TG_FLASHB_LINE_CNT, CAMSV_0_TG_FLASHB_LINE_CNT*/

typedef volatile union _CAMSV_REG_TG_FLASHB_POS_
{
		volatile struct	/* 0x1A050554 */
		{
				FIELD  FLASHB_PXL                            : 15;		/*  0..14, 0x00007FFF */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  FLASHB_LINE                           : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS;	/* CAMSV_TG_FLASHB_POS, CAMSV_0_TG_FLASHB_POS*/

typedef volatile union _CAMSV_REG_TG_FLASHB_POS1_
{
		volatile struct	/* 0x1A050558 */
		{
				FIELD  FLASHB_CYC_CNT                        : 20;		/*  0..19, 0x000FFFFF */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FLASHB_POS1;	/* CAMSV_TG_FLASHB_POS1, CAMSV_0_TG_FLASHB_POS1*/

typedef volatile union _CAMSV_REG_TG_I2C_CQ_TRIG_
{
		volatile struct	/* 0x1A050560 */
		{
				FIELD  TG_I2C_CQ_TRIG                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_I2C_CQ_TRIG;	/* CAMSV_TG_I2C_CQ_TRIG, CAMSV_0_TG_I2C_CQ_TRIG*/

typedef volatile union _CAMSV_REG_TG_CQ_TIMING_
{
		volatile struct	/* 0x1A050564 */
		{
				FIELD  TG_I2C_CQ_TIM                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_CQ_TIMING;	/* CAMSV_TG_CQ_TIMING, CAMSV_0_TG_CQ_TIMING*/

typedef volatile union _CAMSV_REG_TG_CQ_NUM_
{
		volatile struct	/* 0x1A050568 */
		{
				FIELD  TG_CQ_NUM                             :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_CQ_NUM;	/* CAMSV_TG_CQ_NUM, CAMSV_0_TG_CQ_NUM*/

typedef volatile union _CAMSV_REG_TG_TIME_STAMP_
{
		volatile struct	/* 0x1A050570 */
		{
				FIELD  TG_TIME_STAMP                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_TIME_STAMP;	/* CAMSV_TG_TIME_STAMP, CAMSV_0_TG_TIME_STAMP*/

typedef volatile union _CAMSV_REG_TG_SUB_PERIOD_
{
		volatile struct	/* 0x1A050574 */
		{
				FIELD  VS_PERIOD                             :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  SOF_PERIOD                            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_SUB_PERIOD;	/* CAMSV_TG_SUB_PERIOD, CAMSV_0_TG_SUB_PERIOD*/

typedef volatile union _CAMSV_REG_TG_DAT_NO_R_
{
		volatile struct	/* 0x1A050578 */
		{
				FIELD  DAT_NO                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_DAT_NO_R;	/* CAMSV_TG_DAT_NO_R, CAMSV_0_TG_DAT_NO_R*/

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_R_
{
		volatile struct	/* 0x1A05057C */
		{
				FIELD  LINE_CNT                              : 14;		/*  0..13, 0x00003FFF */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  PXL_CNT                               : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_TG_FRMSIZE_ST_R;	/* CAMSV_TG_FRMSIZE_ST_R, CAMSV_0_TG_FRMSIZE_ST_R*/

typedef volatile union _CAMSV_REG_DMA_FRAME_HEADER_EN_
{
		volatile struct	/* 0x1A050E00 */
		{
				FIELD  FRAME_HEADER_EN_IMGO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  FRAME_HEADER_EN_RRZO                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  FRAME_HEADER_EN_AAO                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  FRAME_HEADER_EN_AFO                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  FRAME_HEADER_EN_LCSO                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  FRAME_HEADER_EN_UFEO                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  FRAME_HEADER_EN_PDO                   :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_DMA_FRAME_HEADER_EN;	/* CAMSV_DMA_FRAME_HEADER_EN, CAMSV_0_DMA_FRAME_HEADER_EN*/

typedef volatile union _CAMSV_REG_IMGO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E04 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_BASE_ADDR;	/* CAMSV_IMGO_FH_BASE_ADDR, CAMSV_0_IMGO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_RRZO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E08 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_RRZO_FH_BASE_ADDR;	/* CAMSV_RRZO_FH_BASE_ADDR, CAMSV_0_RRZO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_AAO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E0C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_AAO_FH_BASE_ADDR;	/* CAMSV_AAO_FH_BASE_ADDR, CAMSV_0_AAO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_AFO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E10 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_AFO_FH_BASE_ADDR;	/* CAMSV_AFO_FH_BASE_ADDR, CAMSV_0_AFO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_LCSO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E14 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_LCSO_FH_BASE_ADDR;	/* CAMSV_LCSO_FH_BASE_ADDR, CAMSV_0_LCSO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_UFEO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E18 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_UFEO_FH_BASE_ADDR;	/* CAMSV_UFEO_FH_BASE_ADDR, CAMSV_0_UFEO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_PDO_FH_BASE_ADDR_
{
		volatile struct	/* 0x1A050E1C */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_PDO_FH_BASE_ADDR;	/* CAMSV_PDO_FH_BASE_ADDR, CAMSV_0_PDO_FH_BASE_ADDR*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_2_
{
		volatile struct	/* 0x1A050E30 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_2;	/* CAMSV_IMGO_FH_SPARE_2, CAMSV_0_IMGO_FH_SPARE_2*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_3_
{
		volatile struct	/* 0x1A050E34 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_3;	/* CAMSV_IMGO_FH_SPARE_3, CAMSV_0_IMGO_FH_SPARE_3*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_4_
{
		volatile struct	/* 0x1A050E38 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_4;	/* CAMSV_IMGO_FH_SPARE_4, CAMSV_0_IMGO_FH_SPARE_4*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_5_
{
		volatile struct	/* 0x1A050E3C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_5;	/* CAMSV_IMGO_FH_SPARE_5, CAMSV_0_IMGO_FH_SPARE_5*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_6_
{
		volatile struct	/* 0x1A050E40 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_6;	/* CAMSV_IMGO_FH_SPARE_6, CAMSV_0_IMGO_FH_SPARE_6*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_7_
{
		volatile struct	/* 0x1A050E44 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_7;	/* CAMSV_IMGO_FH_SPARE_7, CAMSV_0_IMGO_FH_SPARE_7*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_8_
{
		volatile struct	/* 0x1A050E48 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_8;	/* CAMSV_IMGO_FH_SPARE_8, CAMSV_0_IMGO_FH_SPARE_8*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_9_
{
		volatile struct	/* 0x1A050E4C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_9;	/* CAMSV_IMGO_FH_SPARE_9, CAMSV_0_IMGO_FH_SPARE_9*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_10_
{
		volatile struct	/* 0x1A050E50 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_10;	/* CAMSV_IMGO_FH_SPARE_10, CAMSV_0_IMGO_FH_SPARE_10*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_11_
{
		volatile struct	/* 0x1A050E54 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_11;	/* CAMSV_IMGO_FH_SPARE_11, CAMSV_0_IMGO_FH_SPARE_11*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_12_
{
		volatile struct	/* 0x1A050E58 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_12;	/* CAMSV_IMGO_FH_SPARE_12, CAMSV_0_IMGO_FH_SPARE_12*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_13_
{
		volatile struct	/* 0x1A050E5C */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_13;	/* CAMSV_IMGO_FH_SPARE_13, CAMSV_0_IMGO_FH_SPARE_13*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_14_
{
		volatile struct	/* 0x1A050E60 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_14;	/* CAMSV_IMGO_FH_SPARE_14, CAMSV_0_IMGO_FH_SPARE_14*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_15_
{
		volatile struct	/* 0x1A050E64 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_15;	/* CAMSV_IMGO_FH_SPARE_15, CAMSV_0_IMGO_FH_SPARE_15*/

typedef volatile union _CAMSV_REG_IMGO_FH_SPARE_16_
{
		volatile struct	/* 0x1A050E68 */
		{
				FIELD  SPARE_REG                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}CAMSV_REG_IMGO_FH_SPARE_16;	/* CAMSV_IMGO_FH_SPARE_16, CAMSV_0_IMGO_FH_SPARE_16*/

typedef volatile struct _cam_uni_reg_t_	/* 0x1A003000..0x1A003FFF */
{
	CAM_UNI_REG_TOP_CTL                             CAM_UNI_TOP_CTL;                                 /* 0000, 0x1A003000 */
	CAM_UNI_REG_TOP_MISC                            CAM_UNI_TOP_MISC;                                /* 0004, 0x1A003004 */
	CAM_UNI_REG_TOP_SW_CTL                          CAM_UNI_TOP_SW_CTL;                              /* 0008, 0x1A003008 */
	CAM_UNI_REG_TOP_RAWI_TRIG                       CAM_UNI_TOP_RAWI_TRIG;                           /* 000C, 0x1A00300C */
	CAM_UNI_REG_TOP_MOD_EN                          CAM_UNI_TOP_MOD_EN;                              /* 0010, 0x1A003010 */
	CAM_UNI_REG_TOP_DMA_EN                          CAM_UNI_TOP_DMA_EN;                              /* 0014, 0x1A003014 */
	CAM_UNI_REG_TOP_PATH_SEL                        CAM_UNI_TOP_PATH_SEL;                            /* 0018, 0x1A003018 */
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
	UINT32                                          rsv_0078[2];                                     /* 0078..007F, 0x1A003078..1A00307F */
	CAM_UNI_REG_FBC_FLKO_A_CTL1                     CAM_UNI_FBC_FLKO_A_CTL1;                         /* 0080, 0x1A003080 */
	CAM_UNI_REG_FBC_FLKO_A_CTL2                     CAM_UNI_FBC_FLKO_A_CTL2;                         /* 0084, 0x1A003084 */
	CAM_UNI_REG_FBC_EISO_A_CTL1                     CAM_UNI_FBC_EISO_A_CTL1;                         /* 0088, 0x1A003088 */
	CAM_UNI_REG_FBC_EISO_A_CTL2                     CAM_UNI_FBC_EISO_A_CTL2;                         /* 008C, 0x1A00308C */
	CAM_UNI_REG_FBC_RSSO_A_CTL1                     CAM_UNI_FBC_RSSO_A_CTL1;                         /* 0090, 0x1A003090 */
	CAM_UNI_REG_FBC_RSSO_A_CTL2                     CAM_UNI_FBC_RSSO_A_CTL2;                         /* 0094, 0x1A003094 */
	UINT32                                          rsv_0098[27];                                    /* 0098..0103, 0x1A003098..1A003103 */
	CAM_UNI_REG_B_TOP_MISC                          CAM_UNI_B_TOP_MISC;                              /* 0104, 0x1A003104 */
	CAM_UNI_REG_B_TOP_SW_CTL                        CAM_UNI_B_TOP_SW_CTL;                            /* 0108, 0x1A003108 */
	CAM_UNI_REG_B_TOP_RAWI_TRIG                     CAM_UNI_B_TOP_RAWI_TRIG;                         /* 010C, 0x1A00310C */
	CAM_UNI_REG_B_TOP_MOD_EN                        CAM_UNI_B_TOP_MOD_EN;                            /* 0110, 0x1A003110 */
	CAM_UNI_REG_B_TOP_DMA_EN                        CAM_UNI_B_TOP_DMA_EN;                            /* 0114, 0x1A003114 */
	CAM_UNI_REG_B_TOP_PATH_SEL                      CAM_UNI_B_TOP_PATH_SEL;                          /* 0118, 0x1A003118 */
	CAM_UNI_REG_B_TOP_FMT_SEL                       CAM_UNI_B_TOP_FMT_SEL;                           /* 011C, 0x1A00311C */
	CAM_UNI_REG_B_TOP_DMA_INT_EN                    CAM_UNI_B_TOP_DMA_INT_EN;                        /* 0120, 0x1A003120 */
	CAM_UNI_REG_B_TOP_DMA_INT_STATUS                CAM_UNI_B_TOP_DMA_INT_STATUS;                    /* 0124, 0x1A003124 */
	CAM_UNI_REG_B_TOP_DMA_INT_STATUSX               CAM_UNI_B_TOP_DMA_INT_STATUSX;                   /* 0128, 0x1A003128 */
	UINT32                                          rsv_012C[2];                                     /* 012C..0133, 0x1A00312C..1A003133 */
	CAM_UNI_REG_B_TOP_DMA_CCU_INT_EN                CAM_UNI_B_TOP_DMA_CCU_INT_EN;                    /* 0134, 0x1A003134 */
	CAM_UNI_REG_B_TOP_DMA_CCU_INT_STATUS            CAM_UNI_B_TOP_DMA_CCU_INT_STATUS;                /* 0138, 0x1A003138 */
	UINT32                                          rsv_013C;                                        /* 013C, 0x1A00313C */
	CAM_UNI_REG_B_TOP_MOD_DCM_DIS                   CAM_UNI_B_TOP_MOD_DCM_DIS;                       /* 0140, 0x1A003140 */
	CAM_UNI_REG_B_TOP_DMA_DCM_DIS                   CAM_UNI_B_TOP_DMA_DCM_DIS;                       /* 0144, 0x1A003144 */
	UINT32                                          rsv_0148[2];                                     /* 0148..014F, 0x1A003148..1A00314F */
	CAM_UNI_REG_B_TOP_MOD_DCM_STATUS                CAM_UNI_B_TOP_MOD_DCM_STATUS;                    /* 0150, 0x1A003150 */
	CAM_UNI_REG_B_TOP_DMA_DCM_STATUS                CAM_UNI_B_TOP_DMA_DCM_STATUS;                    /* 0154, 0x1A003154 */
	UINT32                                          rsv_0158[2];                                     /* 0158..015F, 0x1A003158..1A00315F */
	CAM_UNI_REG_B_TOP_MOD_REQ_STATUS                CAM_UNI_B_TOP_MOD_REQ_STATUS;                    /* 0160, 0x1A003160 */
	CAM_UNI_REG_B_TOP_DMA_REQ_STATUS                CAM_UNI_B_TOP_DMA_REQ_STATUS;                    /* 0164, 0x1A003164 */
	UINT32                                          rsv_0168[2];                                     /* 0168..016F, 0x1A003168..1A00316F */
	CAM_UNI_REG_B_TOP_MOD_RDY_STATUS                CAM_UNI_B_TOP_MOD_RDY_STATUS;                    /* 0170, 0x1A003170 */
	CAM_UNI_REG_B_TOP_DMA_RDY_STATUS                CAM_UNI_B_TOP_DMA_RDY_STATUS;                    /* 0174, 0x1A003174 */
	UINT32                                          rsv_0178[2];                                     /* 0178..017F, 0x1A003178..1A00317F */
	CAM_UNI_REG_FBC_FLKO_B_CTL1                     CAM_UNI_FBC_FLKO_B_CTL1;                         /* 0180, 0x1A003180 */
	CAM_UNI_REG_FBC_FLKO_B_CTL2                     CAM_UNI_FBC_FLKO_B_CTL2;                         /* 0184, 0x1A003184 */
	CAM_UNI_REG_FBC_EISO_B_CTL1                     CAM_UNI_FBC_EISO_B_CTL1;                         /* 0188, 0x1A003188 */
	CAM_UNI_REG_FBC_EISO_B_CTL2                     CAM_UNI_FBC_EISO_B_CTL2;                         /* 018C, 0x1A00318C */
	CAM_UNI_REG_FBC_RSSO_B_CTL1                     CAM_UNI_FBC_RSSO_B_CTL1;                         /* 0190, 0x1A003190 */
	CAM_UNI_REG_FBC_RSSO_B_CTL2                     CAM_UNI_FBC_RSSO_B_CTL2;                         /* 0194, 0x1A003194 */
	UINT32                                          rsv_0198[26];                                    /* 0198..01FF, 0x1A003198..1A0031FF */
	CAM_UNI_REG_DMA_SOFT_RSTSTAT                    CAM_UNI_DMA_SOFT_RSTSTAT;                        /* 0200, 0x1A003200 */
	CAM_UNI_REG_VERTICAL_FLIP_EN                    CAM_UNI_VERTICAL_FLIP_EN;                        /* 0204, 0x1A003204 */
	CAM_UNI_REG_DMA_SOFT_RESET                      CAM_UNI_DMA_SOFT_RESET;                          /* 0208, 0x1A003208 */
	CAM_UNI_REG_LAST_ULTRA_EN                       CAM_UNI_LAST_ULTRA_EN;                           /* 020C, 0x1A00320C */
	CAM_UNI_REG_SPECIAL_FUN_EN                      CAM_UNI_SPECIAL_FUN_EN;                          /* 0210, 0x1A003210 */
	CAM_UNI_REG_SPECIAL_FUN2_EN                     CAM_UNI_SPECIAL_FUN2_EN;                         /* 0214, 0x1A003214 */
	UINT32                                          rsv_0218[2];                                     /* 0218..021F, 0x1A003218..1A00321F */
	CAM_UNI_REG_EISO_BASE_ADDR                      CAM_UNI_EISO_BASE_ADDR;                          /* 0220, 0x1A003220 */
	UINT32                                          rsv_0224;                                        /* 0224, 0x1A003224 */
	CAM_UNI_REG_EISO_OFST_ADDR                      CAM_UNI_EISO_OFST_ADDR;                          /* 0228, 0x1A003228 */
	CAM_UNI_REG_EISO_DRS                            CAM_UNI_EISO_DRS;                                /* 022C, 0x1A00322C */
	CAM_UNI_REG_EISO_XSIZE                          CAM_UNI_EISO_XSIZE;                              /* 0230, 0x1A003230 */
	CAM_UNI_REG_EISO_YSIZE                          CAM_UNI_EISO_YSIZE;                              /* 0234, 0x1A003234 */
	CAM_UNI_REG_EISO_STRIDE                         CAM_UNI_EISO_STRIDE;                             /* 0238, 0x1A003238 */
	CAM_UNI_REG_EISO_CON                            CAM_UNI_EISO_CON;                                /* 023C, 0x1A00323C */
	CAM_UNI_REG_EISO_CON2                           CAM_UNI_EISO_CON2;                               /* 0240, 0x1A003240 */
	CAM_UNI_REG_EISO_CON3                           CAM_UNI_EISO_CON3;                               /* 0244, 0x1A003244 */
	UINT32                                          rsv_0248;                                        /* 0248, 0x1A003248 */
	CAM_UNI_REG_EISO_CON4                           CAM_UNI_EISO_CON4;                               /* 024C, 0x1A00324C */
	CAM_UNI_REG_FLKO_BASE_ADDR                      CAM_UNI_FLKO_BASE_ADDR;                          /* 0250, 0x1A003250 */
	UINT32                                          rsv_0254;                                        /* 0254, 0x1A003254 */
	CAM_UNI_REG_FLKO_OFST_ADDR                      CAM_UNI_FLKO_OFST_ADDR;                          /* 0258, 0x1A003258 */
	CAM_UNI_REG_FLKO_DRS                            CAM_UNI_FLKO_DRS;                                /* 025C, 0x1A00325C */
	CAM_UNI_REG_FLKO_XSIZE                          CAM_UNI_FLKO_XSIZE;                              /* 0260, 0x1A003260 */
	CAM_UNI_REG_FLKO_YSIZE                          CAM_UNI_FLKO_YSIZE;                              /* 0264, 0x1A003264 */
	CAM_UNI_REG_FLKO_STRIDE                         CAM_UNI_FLKO_STRIDE;                             /* 0268, 0x1A003268 */
	CAM_UNI_REG_FLKO_CON                            CAM_UNI_FLKO_CON;                                /* 026C, 0x1A00326C */
	CAM_UNI_REG_FLKO_CON2                           CAM_UNI_FLKO_CON2;                               /* 0270, 0x1A003270 */
	CAM_UNI_REG_FLKO_CON3                           CAM_UNI_FLKO_CON3;                               /* 0274, 0x1A003274 */
	UINT32                                          rsv_0278;                                        /* 0278, 0x1A003278 */
	CAM_UNI_REG_FLKO_CON4                           CAM_UNI_FLKO_CON4;                               /* 027C, 0x1A00327C */
	CAM_UNI_REG_RSSO_A_BASE_ADDR                    CAM_UNI_RSSO_A_BASE_ADDR;                        /* 0280, 0x1A003280 */
	UINT32                                          rsv_0284;                                        /* 0284, 0x1A003284 */
	CAM_UNI_REG_RSSO_A_OFST_ADDR                    CAM_UNI_RSSO_A_OFST_ADDR;                        /* 0288, 0x1A003288 */
	CAM_UNI_REG_RSSO_A_DRS                          CAM_UNI_RSSO_A_DRS;                              /* 028C, 0x1A00328C */
	CAM_UNI_REG_RSSO_A_XSIZE                        CAM_UNI_RSSO_A_XSIZE;                            /* 0290, 0x1A003290 */
	CAM_UNI_REG_RSSO_A_YSIZE                        CAM_UNI_RSSO_A_YSIZE;                            /* 0294, 0x1A003294 */
	CAM_UNI_REG_RSSO_A_STRIDE                       CAM_UNI_RSSO_A_STRIDE;                           /* 0298, 0x1A003298 */
	CAM_UNI_REG_RSSO_A_CON                          CAM_UNI_RSSO_A_CON;                              /* 029C, 0x1A00329C */
	CAM_UNI_REG_RSSO_A_CON2                         CAM_UNI_RSSO_A_CON2;                             /* 02A0, 0x1A0032A0 */
	CAM_UNI_REG_RSSO_A_CON3                         CAM_UNI_RSSO_A_CON3;                             /* 02A4, 0x1A0032A4 */
	UINT32                                          rsv_02A8;                                        /* 02A8, 0x1A0032A8 */
	CAM_UNI_REG_RSSO_A_CON4                         CAM_UNI_RSSO_A_CON4;                             /* 02AC, 0x1A0032AC */
	CAM_UNI_REG_RSSO_B_BASE_ADDR                    CAM_UNI_RSSO_B_BASE_ADDR;                        /* 02B0, 0x1A0032B0 */
	UINT32                                          rsv_02B4;                                        /* 02B4, 0x1A0032B4 */
	CAM_UNI_REG_RSSO_B_OFST_ADDR                    CAM_UNI_RSSO_B_OFST_ADDR;                        /* 02B8, 0x1A0032B8 */
	CAM_UNI_REG_RSSO_B_DRS                          CAM_UNI_RSSO_B_DRS;                              /* 02BC, 0x1A0032BC */
	CAM_UNI_REG_RSSO_B_XSIZE                        CAM_UNI_RSSO_B_XSIZE;                            /* 02C0, 0x1A0032C0 */
	CAM_UNI_REG_RSSO_B_YSIZE                        CAM_UNI_RSSO_B_YSIZE;                            /* 02C4, 0x1A0032C4 */
	CAM_UNI_REG_RSSO_B_STRIDE                       CAM_UNI_RSSO_B_STRIDE;                           /* 02C8, 0x1A0032C8 */
	CAM_UNI_REG_RSSO_B_CON                          CAM_UNI_RSSO_B_CON;                              /* 02CC, 0x1A0032CC */
	CAM_UNI_REG_RSSO_B_CON2                         CAM_UNI_RSSO_B_CON2;                             /* 02D0, 0x1A0032D0 */
	CAM_UNI_REG_RSSO_B_CON3                         CAM_UNI_RSSO_B_CON3;                             /* 02D4, 0x1A0032D4 */
	UINT32                                          rsv_02D8;                                        /* 02D8, 0x1A0032D8 */
	CAM_UNI_REG_RSSO_B_CON4                         CAM_UNI_RSSO_B_CON4;                             /* 02DC, 0x1A0032DC */
	UINT32                                          rsv_02E0[24];                                    /* 02E0..033F, 0x1A0032E0..1A00333F */
	CAM_UNI_REG_RAWI_BASE_ADDR                      CAM_UNI_RAWI_BASE_ADDR;                          /* 0340, 0x1A003340 */
	UINT32                                          rsv_0344;                                        /* 0344, 0x1A003344 */
	CAM_UNI_REG_RAWI_OFST_ADDR                      CAM_UNI_RAWI_OFST_ADDR;                          /* 0348, 0x1A003348 */
	CAM_UNI_REG_RAWI_DRS                            CAM_UNI_RAWI_DRS;                                /* 034C, 0x1A00334C */
	CAM_UNI_REG_RAWI_XSIZE                          CAM_UNI_RAWI_XSIZE;                              /* 0350, 0x1A003350 */
	CAM_UNI_REG_RAWI_YSIZE                          CAM_UNI_RAWI_YSIZE;                              /* 0354, 0x1A003354 */
	CAM_UNI_REG_RAWI_STRIDE                         CAM_UNI_RAWI_STRIDE;                             /* 0358, 0x1A003358 */
	CAM_UNI_REG_RAWI_CON                            CAM_UNI_RAWI_CON;                                /* 035C, 0x1A00335C */
	CAM_UNI_REG_RAWI_CON2                           CAM_UNI_RAWI_CON2;                               /* 0360, 0x1A003360 */
	CAM_UNI_REG_RAWI_CON3                           CAM_UNI_RAWI_CON3;                               /* 0364, 0x1A003364 */
	UINT32                                          rsv_0368;                                        /* 0368, 0x1A003368 */
	CAM_UNI_REG_RAWI_CON4                           CAM_UNI_RAWI_CON4;                               /* 036C, 0x1A00336C */
	CAM_UNI_REG_DMA_ERR_CTRL                        CAM_UNI_DMA_ERR_CTRL;                            /* 0370, 0x1A003370 */
	CAM_UNI_REG_EISO_ERR_STAT                       CAM_UNI_EISO_ERR_STAT;                           /* 0374, 0x1A003374 */
	CAM_UNI_REG_FLKO_ERR_STAT                       CAM_UNI_FLKO_ERR_STAT;                           /* 0378, 0x1A003378 */
	CAM_UNI_REG_RSSO_A_ERR_STAT                     CAM_UNI_RSSO_A_ERR_STAT;                         /* 037C, 0x1A00337C */
	CAM_UNI_REG_RSSO_B_ERR_STAT                     CAM_UNI_RSSO_B_ERR_STAT;                         /* 0380, 0x1A003380 */
	CAM_UNI_REG_RAWI_ERR_STAT                       CAM_UNI_RAWI_ERR_STAT;                           /* 0384, 0x1A003384 */
	CAM_UNI_REG_DMA_DEBUG_ADDR                      CAM_UNI_DMA_DEBUG_ADDR;                          /* 0388, 0x1A003388 */
	CAM_UNI_REG_DMA_RSV1                            CAM_UNI_DMA_RSV1;                                /* 038C, 0x1A00338C */
	CAM_UNI_REG_DMA_RSV2                            CAM_UNI_DMA_RSV2;                                /* 0390, 0x1A003390 */
	CAM_UNI_REG_DMA_RSV3                            CAM_UNI_DMA_RSV3;                                /* 0394, 0x1A003394 */
	CAM_UNI_REG_DMA_RSV4                            CAM_UNI_DMA_RSV4;                                /* 0398, 0x1A003398 */
	CAM_UNI_REG_DMA_RSV5                            CAM_UNI_DMA_RSV5;                                /* 039C, 0x1A00339C */
	CAM_UNI_REG_DMA_RSV6                            CAM_UNI_DMA_RSV6;                                /* 03A0, 0x1A0033A0 */
	CAM_UNI_REG_DMA_DEBUG_SEL                       CAM_UNI_DMA_DEBUG_SEL;                           /* 03A4, 0x1A0033A4 */
	CAM_UNI_REG_DMA_BW_SELF_TEST                    CAM_UNI_DMA_BW_SELF_TEST;                        /* 03A8, 0x1A0033A8 */
	UINT32                                          rsv_03AC[5];                                     /* 03AC..03BF, 0x1A0033AC..1A0033BF */
	CAM_UNI_REG_DMA_FRAME_HEADER_EN                 CAM_UNI_DMA_FRAME_HEADER_EN;                     /* 03C0, 0x1A0033C0 */
	CAM_UNI_REG_EISO_FH_BASE_ADDR                   CAM_UNI_EISO_FH_BASE_ADDR;                       /* 03C4, 0x1A0033C4 */
	CAM_UNI_REG_FLKO_FH_BASE_ADDR                   CAM_UNI_FLKO_FH_BASE_ADDR;                       /* 03C8, 0x1A0033C8 */
	CAM_UNI_REG_RSSO_A_FH_BASE_ADDR                 CAM_UNI_RSSO_A_FH_BASE_ADDR;                     /* 03CC, 0x1A0033CC */
	CAM_UNI_REG_RSSO_B_FH_BASE_ADDR                 CAM_UNI_RSSO_B_FH_BASE_ADDR;                     /* 03D0, 0x1A0033D0 */
	UINT32                                          rsv_03D4[3];                                     /* 03D4..03DF, 0x1A0033D4..1A0033DF */
	CAM_UNI_REG_EISO_FH_SPARE_2                     CAM_UNI_EISO_FH_SPARE_2;                         /* 03E0, 0x1A0033E0 */
	CAM_UNI_REG_EISO_FH_SPARE_3                     CAM_UNI_EISO_FH_SPARE_3;                         /* 03E4, 0x1A0033E4 */
	CAM_UNI_REG_EISO_FH_SPARE_4                     CAM_UNI_EISO_FH_SPARE_4;                         /* 03E8, 0x1A0033E8 */
	CAM_UNI_REG_EISO_FH_SPARE_5                     CAM_UNI_EISO_FH_SPARE_5;                         /* 03EC, 0x1A0033EC */
	CAM_UNI_REG_EISO_FH_SPARE_6                     CAM_UNI_EISO_FH_SPARE_6;                         /* 03F0, 0x1A0033F0 */
	CAM_UNI_REG_EISO_FH_SPARE_7                     CAM_UNI_EISO_FH_SPARE_7;                         /* 03F4, 0x1A0033F4 */
	CAM_UNI_REG_EISO_FH_SPARE_8                     CAM_UNI_EISO_FH_SPARE_8;                         /* 03F8, 0x1A0033F8 */
	CAM_UNI_REG_EISO_FH_SPARE_9                     CAM_UNI_EISO_FH_SPARE_9;                         /* 03FC, 0x1A0033FC */
	CAM_UNI_REG_EISO_FH_SPARE_10                    CAM_UNI_EISO_FH_SPARE_10;                        /* 0400, 0x1A003400 */
	CAM_UNI_REG_EISO_FH_SPARE_11                    CAM_UNI_EISO_FH_SPARE_11;                        /* 0404, 0x1A003404 */
	CAM_UNI_REG_EISO_FH_SPARE_12                    CAM_UNI_EISO_FH_SPARE_12;                        /* 0408, 0x1A003408 */
	CAM_UNI_REG_EISO_FH_SPARE_13                    CAM_UNI_EISO_FH_SPARE_13;                        /* 040C, 0x1A00340C */
	CAM_UNI_REG_EISO_FH_SPARE_14                    CAM_UNI_EISO_FH_SPARE_14;                        /* 0410, 0x1A003410 */
	CAM_UNI_REG_EISO_FH_SPARE_15                    CAM_UNI_EISO_FH_SPARE_15;                        /* 0414, 0x1A003414 */
	CAM_UNI_REG_EISO_FH_SPARE_16                    CAM_UNI_EISO_FH_SPARE_16;                        /* 0418, 0x1A003418 */
	UINT32                                          rsv_041C;                                        /* 041C, 0x1A00341C */
	CAM_UNI_REG_FLKO_FH_SPARE_2                     CAM_UNI_FLKO_FH_SPARE_2;                         /* 0420, 0x1A003420 */
	CAM_UNI_REG_FLKO_FH_SPARE_3                     CAM_UNI_FLKO_FH_SPARE_3;                         /* 0424, 0x1A003424 */
	CAM_UNI_REG_FLKO_FH_SPARE_4                     CAM_UNI_FLKO_FH_SPARE_4;                         /* 0428, 0x1A003428 */
	CAM_UNI_REG_FLKO_FH_SPARE_5                     CAM_UNI_FLKO_FH_SPARE_5;                         /* 042C, 0x1A00342C */
	CAM_UNI_REG_FLKO_FH_SPARE_6                     CAM_UNI_FLKO_FH_SPARE_6;                         /* 0430, 0x1A003430 */
	CAM_UNI_REG_FLKO_FH_SPARE_7                     CAM_UNI_FLKO_FH_SPARE_7;                         /* 0434, 0x1A003434 */
	CAM_UNI_REG_FLKO_FH_SPARE_8                     CAM_UNI_FLKO_FH_SPARE_8;                         /* 0438, 0x1A003438 */
	CAM_UNI_REG_FLKO_FH_SPARE_9                     CAM_UNI_FLKO_FH_SPARE_9;                         /* 043C, 0x1A00343C */
	CAM_UNI_REG_FLKO_FH_SPARE_10                    CAM_UNI_FLKO_FH_SPARE_10;                        /* 0440, 0x1A003440 */
	CAM_UNI_REG_FLKO_FH_SPARE_11                    CAM_UNI_FLKO_FH_SPARE_11;                        /* 0444, 0x1A003444 */
	CAM_UNI_REG_FLKO_FH_SPARE_12                    CAM_UNI_FLKO_FH_SPARE_12;                        /* 0448, 0x1A003448 */
	CAM_UNI_REG_FLKO_FH_SPARE_13                    CAM_UNI_FLKO_FH_SPARE_13;                        /* 044C, 0x1A00344C */
	CAM_UNI_REG_FLKO_FH_SPARE_14                    CAM_UNI_FLKO_FH_SPARE_14;                        /* 0450, 0x1A003450 */
	CAM_UNI_REG_FLKO_FH_SPARE_15                    CAM_UNI_FLKO_FH_SPARE_15;                        /* 0454, 0x1A003454 */
	CAM_UNI_REG_FLKO_FH_SPARE_16                    CAM_UNI_FLKO_FH_SPARE_16;                        /* 0458, 0x1A003458 */
	UINT32                                          rsv_045C;                                        /* 045C, 0x1A00345C */
	CAM_UNI_REG_RSSO_A_FH_SPARE_2                   CAM_UNI_RSSO_A_FH_SPARE_2;                       /* 0460, 0x1A003460 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_3                   CAM_UNI_RSSO_A_FH_SPARE_3;                       /* 0464, 0x1A003464 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_4                   CAM_UNI_RSSO_A_FH_SPARE_4;                       /* 0468, 0x1A003468 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_5                   CAM_UNI_RSSO_A_FH_SPARE_5;                       /* 046C, 0x1A00346C */
	CAM_UNI_REG_RSSO_A_FH_SPARE_6                   CAM_UNI_RSSO_A_FH_SPARE_6;                       /* 0470, 0x1A003470 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_7                   CAM_UNI_RSSO_A_FH_SPARE_7;                       /* 0474, 0x1A003474 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_8                   CAM_UNI_RSSO_A_FH_SPARE_8;                       /* 0478, 0x1A003478 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_9                   CAM_UNI_RSSO_A_FH_SPARE_9;                       /* 047C, 0x1A00347C */
	CAM_UNI_REG_RSSO_A_FH_SPARE_10                  CAM_UNI_RSSO_A_FH_SPARE_10;                      /* 0480, 0x1A003480 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_11                  CAM_UNI_RSSO_A_FH_SPARE_11;                      /* 0484, 0x1A003484 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_12                  CAM_UNI_RSSO_A_FH_SPARE_12;                      /* 0488, 0x1A003488 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_13                  CAM_UNI_RSSO_A_FH_SPARE_13;                      /* 048C, 0x1A00348C */
	CAM_UNI_REG_RSSO_A_FH_SPARE_14                  CAM_UNI_RSSO_A_FH_SPARE_14;                      /* 0490, 0x1A003490 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_15                  CAM_UNI_RSSO_A_FH_SPARE_15;                      /* 0494, 0x1A003494 */
	CAM_UNI_REG_RSSO_A_FH_SPARE_16                  CAM_UNI_RSSO_A_FH_SPARE_16;                      /* 0498, 0x1A003498 */
	UINT32                                          rsv_049C;                                        /* 049C, 0x1A00349C */
	CAM_UNI_REG_RSSO_B_FH_SPARE_2                   CAM_UNI_RSSO_B_FH_SPARE_2;                       /* 04A0, 0x1A0034A0 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_3                   CAM_UNI_RSSO_B_FH_SPARE_3;                       /* 04A4, 0x1A0034A4 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_4                   CAM_UNI_RSSO_B_FH_SPARE_4;                       /* 04A8, 0x1A0034A8 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_5                   CAM_UNI_RSSO_B_FH_SPARE_5;                       /* 04AC, 0x1A0034AC */
	CAM_UNI_REG_RSSO_B_FH_SPARE_6                   CAM_UNI_RSSO_B_FH_SPARE_6;                       /* 04B0, 0x1A0034B0 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_7                   CAM_UNI_RSSO_B_FH_SPARE_7;                       /* 04B4, 0x1A0034B4 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_8                   CAM_UNI_RSSO_B_FH_SPARE_8;                       /* 04B8, 0x1A0034B8 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_9                   CAM_UNI_RSSO_B_FH_SPARE_9;                       /* 04BC, 0x1A0034BC */
	CAM_UNI_REG_RSSO_B_FH_SPARE_10                  CAM_UNI_RSSO_B_FH_SPARE_10;                      /* 04C0, 0x1A0034C0 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_11                  CAM_UNI_RSSO_B_FH_SPARE_11;                      /* 04C4, 0x1A0034C4 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_12                  CAM_UNI_RSSO_B_FH_SPARE_12;                      /* 04C8, 0x1A0034C8 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_13                  CAM_UNI_RSSO_B_FH_SPARE_13;                      /* 04CC, 0x1A0034CC */
	CAM_UNI_REG_RSSO_B_FH_SPARE_14                  CAM_UNI_RSSO_B_FH_SPARE_14;                      /* 04D0, 0x1A0034D0 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_15                  CAM_UNI_RSSO_B_FH_SPARE_15;                      /* 04D4, 0x1A0034D4 */
	CAM_UNI_REG_RSSO_B_FH_SPARE_16                  CAM_UNI_RSSO_B_FH_SPARE_16;                      /* 04D8, 0x1A0034D8 */
	UINT32                                          rsv_04DC[9];                                     /* 04DC..04FF, 0x1A0034DC..1A0034FF */
	CAM_UNI_REG_UNP2_A_OFST                         CAM_UNI_UNP2_A_OFST;                             /* 0500, 0x1A003500 */
	UINT32                                          rsv_0504[3];                                     /* 0504..050F, 0x1A003504..1A00350F */
	CAM_UNI_REG_QBN3_A_MODE                         CAM_UNI_QBN3_A_MODE;                             /* 0510, 0x1A003510 */
	UINT32                                          rsv_0514[3];                                     /* 0514..051F, 0x1A003514..1A00351F */
	CAM_UNI_REG_SGG3_A_PGN                          CAM_UNI_SGG3_A_PGN;                              /* 0520, 0x1A003520 */
	CAM_UNI_REG_SGG3_A_GMRC_1                       CAM_UNI_SGG3_A_GMRC_1;                           /* 0524, 0x1A003524 */
	CAM_UNI_REG_SGG3_A_GMRC_2                       CAM_UNI_SGG3_A_GMRC_2;                           /* 0528, 0x1A003528 */
	UINT32                                          rsv_052C;                                        /* 052C, 0x1A00352C */
	CAM_UNI_REG_FLK_A_CON                           CAM_UNI_FLK_A_CON;                               /* 0530, 0x1A003530 */
	CAM_UNI_REG_FLK_A_OFST                          CAM_UNI_FLK_A_OFST;                              /* 0534, 0x1A003534 */
	CAM_UNI_REG_FLK_A_SIZE                          CAM_UNI_FLK_A_SIZE;                              /* 0538, 0x1A003538 */
	CAM_UNI_REG_FLK_A_NUM                           CAM_UNI_FLK_A_NUM;                               /* 053C, 0x1A00353C */
	UINT32                                          rsv_0540[4];                                     /* 0540..054F, 0x1A003540..1A00354F */
	CAM_UNI_REG_EIS_A_PREP_ME_CTRL1                 CAM_UNI_EIS_A_PREP_ME_CTRL1;                     /* 0550, 0x1A003550 */
	CAM_UNI_REG_EIS_A_PREP_ME_CTRL2                 CAM_UNI_EIS_A_PREP_ME_CTRL2;                     /* 0554, 0x1A003554 */
	CAM_UNI_REG_EIS_A_LMV_TH                        CAM_UNI_EIS_A_LMV_TH;                            /* 0558, 0x1A003558 */
	CAM_UNI_REG_EIS_A_FL_OFFSET                     CAM_UNI_EIS_A_FL_OFFSET;                         /* 055C, 0x1A00355C */
	CAM_UNI_REG_EIS_A_MB_OFFSET                     CAM_UNI_EIS_A_MB_OFFSET;                         /* 0560, 0x1A003560 */
	CAM_UNI_REG_EIS_A_MB_INTERVAL                   CAM_UNI_EIS_A_MB_INTERVAL;                       /* 0564, 0x1A003564 */
	CAM_UNI_REG_EIS_A_GMV                           CAM_UNI_EIS_A_GMV;                               /* 0568, 0x1A003568 */
	CAM_UNI_REG_EIS_A_ERR_CTRL                      CAM_UNI_EIS_A_ERR_CTRL;                          /* 056C, 0x1A00356C */
	CAM_UNI_REG_EIS_A_IMAGE_CTRL                    CAM_UNI_EIS_A_IMAGE_CTRL;                        /* 0570, 0x1A003570 */
	UINT32                                          rsv_0574[11];                                    /* 0574..059F, 0x1A003574..1A00359F */
	CAM_UNI_REG_HDS_A_MODE                          CAM_UNI_HDS_A_MODE;                              /* 05A0, 0x1A0035A0 */
	UINT32                                          rsv_05A4[7];                                     /* 05A4..05BF, 0x1A0035A4..1A0035BF */
	CAM_UNI_REG_RSS_A_CONTROL                       CAM_UNI_RSS_A_CONTROL;                           /* 05C0, 0x1A0035C0 */
	CAM_UNI_REG_RSS_A_IN_IMG                        CAM_UNI_RSS_A_IN_IMG;                            /* 05C4, 0x1A0035C4 */
	CAM_UNI_REG_RSS_A_OUT_IMG                       CAM_UNI_RSS_A_OUT_IMG;                           /* 05C8, 0x1A0035C8 */
	CAM_UNI_REG_RSS_A_HORI_STEP                     CAM_UNI_RSS_A_HORI_STEP;                         /* 05CC, 0x1A0035CC */
	CAM_UNI_REG_RSS_A_VERT_STEP                     CAM_UNI_RSS_A_VERT_STEP;                         /* 05D0, 0x1A0035D0 */
	CAM_UNI_REG_RSS_A_HORI_INT_OFST                 CAM_UNI_RSS_A_HORI_INT_OFST;                     /* 05D4, 0x1A0035D4 */
	CAM_UNI_REG_RSS_A_HORI_SUB_OFST                 CAM_UNI_RSS_A_HORI_SUB_OFST;                     /* 05D8, 0x1A0035D8 */
	CAM_UNI_REG_RSS_A_VERT_INT_OFST                 CAM_UNI_RSS_A_VERT_INT_OFST;                     /* 05DC, 0x1A0035DC */
	CAM_UNI_REG_RSS_A_VERT_SUB_OFST                 CAM_UNI_RSS_A_VERT_SUB_OFST;                     /* 05E0, 0x1A0035E0 */
	UINT32                                          rsv_05E4[7];                                     /* 05E4..05FF, 0x1A0035E4..1A0035FF */
	CAM_UNI_REG_B_DMA_SOFT_RSTSTAT                  CAM_UNI_B_DMA_SOFT_RSTSTAT;                      /* 0600, 0x1A003600 */
	CAM_UNI_REG_B_VERTICAL_FLIP_EN                  CAM_UNI_B_VERTICAL_FLIP_EN;                      /* 0604, 0x1A003604 */
	CAM_UNI_REG_B_DMA_SOFT_RESET                    CAM_UNI_B_DMA_SOFT_RESET;                        /* 0608, 0x1A003608 */
	CAM_UNI_REG_B_LAST_ULTRA_EN                     CAM_UNI_B_LAST_ULTRA_EN;                         /* 060C, 0x1A00360C */
	CAM_UNI_REG_B_SPECIAL_FUN_EN                    CAM_UNI_B_SPECIAL_FUN_EN;                        /* 0610, 0x1A003610 */
	CAM_UNI_REG_B_SPECIAL_FUN2_EN                   CAM_UNI_B_SPECIAL_FUN2_EN;                       /* 0614, 0x1A003614 */
	UINT32                                          rsv_0618[2];                                     /* 0618..061F, 0x1A003618..1A00361F */
	CAM_UNI_REG_B_EISO_BASE_ADDR                    CAM_UNI_B_EISO_BASE_ADDR;                        /* 0620, 0x1A003620 */
	UINT32                                          rsv_0624;                                        /* 0624, 0x1A003624 */
	CAM_UNI_REG_B_EISO_OFST_ADDR                    CAM_UNI_B_EISO_OFST_ADDR;                        /* 0628, 0x1A003628 */
	CAM_UNI_REG_B_EISO_DRS                          CAM_UNI_B_EISO_DRS;                              /* 062C, 0x1A00362C */
	CAM_UNI_REG_B_EISO_XSIZE                        CAM_UNI_B_EISO_XSIZE;                            /* 0630, 0x1A003630 */
	CAM_UNI_REG_B_EISO_YSIZE                        CAM_UNI_B_EISO_YSIZE;                            /* 0634, 0x1A003634 */
	CAM_UNI_REG_B_EISO_STRIDE                       CAM_UNI_B_EISO_STRIDE;                           /* 0638, 0x1A003638 */
	CAM_UNI_REG_B_EISO_CON                          CAM_UNI_B_EISO_CON;                              /* 063C, 0x1A00363C */
	CAM_UNI_REG_B_EISO_CON2                         CAM_UNI_B_EISO_CON2;                             /* 0640, 0x1A003640 */
	CAM_UNI_REG_B_EISO_CON3                         CAM_UNI_B_EISO_CON3;                             /* 0644, 0x1A003644 */
	UINT32                                          rsv_0648;                                        /* 0648, 0x1A003648 */
	CAM_UNI_REG_B_EISO_CON4                         CAM_UNI_B_EISO_CON4;                             /* 064C, 0x1A00364C */
	CAM_UNI_REG_B_FLKO_BASE_ADDR                    CAM_UNI_B_FLKO_BASE_ADDR;                        /* 0650, 0x1A003650 */
	UINT32                                          rsv_0654;                                        /* 0654, 0x1A003654 */
	CAM_UNI_REG_B_FLKO_OFST_ADDR                    CAM_UNI_B_FLKO_OFST_ADDR;                        /* 0658, 0x1A003658 */
	CAM_UNI_REG_B_FLKO_DRS                          CAM_UNI_B_FLKO_DRS;                              /* 065C, 0x1A00365C */
	CAM_UNI_REG_B_FLKO_XSIZE                        CAM_UNI_B_FLKO_XSIZE;                            /* 0660, 0x1A003660 */
	CAM_UNI_REG_B_FLKO_YSIZE                        CAM_UNI_B_FLKO_YSIZE;                            /* 0664, 0x1A003664 */
	CAM_UNI_REG_B_FLKO_STRIDE                       CAM_UNI_B_FLKO_STRIDE;                           /* 0668, 0x1A003668 */
	CAM_UNI_REG_B_FLKO_CON                          CAM_UNI_B_FLKO_CON;                              /* 066C, 0x1A00366C */
	CAM_UNI_REG_B_FLKO_CON2                         CAM_UNI_B_FLKO_CON2;                             /* 0670, 0x1A003670 */
	CAM_UNI_REG_B_FLKO_CON3                         CAM_UNI_B_FLKO_CON3;                             /* 0674, 0x1A003674 */
	UINT32                                          rsv_0678;                                        /* 0678, 0x1A003678 */
	CAM_UNI_REG_B_FLKO_CON4                         CAM_UNI_B_FLKO_CON4;                             /* 067C, 0x1A00367C */
	CAM_UNI_REG_B_RSSO_A_BASE_ADDR                  CAM_UNI_B_RSSO_A_BASE_ADDR;                      /* 0680, 0x1A003680 */
	UINT32                                          rsv_0684;                                        /* 0684, 0x1A003684 */
	CAM_UNI_REG_B_RSSO_A_OFST_ADDR                  CAM_UNI_B_RSSO_A_OFST_ADDR;                      /* 0688, 0x1A003688 */
	CAM_UNI_REG_B_RSSO_A_DRS                        CAM_UNI_B_RSSO_A_DRS;                            /* 068C, 0x1A00368C */
	CAM_UNI_REG_B_RSSO_A_XSIZE                      CAM_UNI_B_RSSO_A_XSIZE;                          /* 0690, 0x1A003690 */
	CAM_UNI_REG_B_RSSO_A_YSIZE                      CAM_UNI_B_RSSO_A_YSIZE;                          /* 0694, 0x1A003694 */
	CAM_UNI_REG_B_RSSO_A_STRIDE                     CAM_UNI_B_RSSO_A_STRIDE;                         /* 0698, 0x1A003698 */
	CAM_UNI_REG_B_RSSO_A_CON                        CAM_UNI_B_RSSO_A_CON;                            /* 069C, 0x1A00369C */
	CAM_UNI_REG_B_RSSO_A_CON2                       CAM_UNI_B_RSSO_A_CON2;                           /* 06A0, 0x1A0036A0 */
	CAM_UNI_REG_B_RSSO_A_CON3                       CAM_UNI_B_RSSO_A_CON3;                           /* 06A4, 0x1A0036A4 */
	UINT32                                          rsv_06A8;                                        /* 06A8, 0x1A0036A8 */
	CAM_UNI_REG_B_RSSO_A_CON4                       CAM_UNI_B_RSSO_A_CON4;                           /* 06AC, 0x1A0036AC */
	CAM_UNI_REG_B_RSSO_B_BASE_ADDR                  CAM_UNI_B_RSSO_B_BASE_ADDR;                      /* 06B0, 0x1A0036B0 */
	UINT32                                          rsv_06B4;                                        /* 06B4, 0x1A0036B4 */
	CAM_UNI_REG_B_RSSO_B_OFST_ADDR                  CAM_UNI_B_RSSO_B_OFST_ADDR;                      /* 06B8, 0x1A0036B8 */
	CAM_UNI_REG_B_RSSO_B_DRS                        CAM_UNI_B_RSSO_B_DRS;                            /* 06BC, 0x1A0036BC */
	CAM_UNI_REG_B_RSSO_B_XSIZE                      CAM_UNI_B_RSSO_B_XSIZE;                          /* 06C0, 0x1A0036C0 */
	CAM_UNI_REG_B_RSSO_B_YSIZE                      CAM_UNI_B_RSSO_B_YSIZE;                          /* 06C4, 0x1A0036C4 */
	CAM_UNI_REG_B_RSSO_B_STRIDE                     CAM_UNI_B_RSSO_B_STRIDE;                         /* 06C8, 0x1A0036C8 */
	CAM_UNI_REG_B_RSSO_B_CON                        CAM_UNI_B_RSSO_B_CON;                            /* 06CC, 0x1A0036CC */
	CAM_UNI_REG_B_RSSO_B_CON2                       CAM_UNI_B_RSSO_B_CON2;                           /* 06D0, 0x1A0036D0 */
	CAM_UNI_REG_B_RSSO_B_CON3                       CAM_UNI_B_RSSO_B_CON3;                           /* 06D4, 0x1A0036D4 */
	UINT32                                          rsv_06D8;                                        /* 06D8, 0x1A0036D8 */
	CAM_UNI_REG_B_RSSO_B_CON4                       CAM_UNI_B_RSSO_B_CON4;                           /* 06DC, 0x1A0036DC */
	UINT32                                          rsv_06E0[24];                                    /* 06E0..073F, 0x1A0036E0..1A00373F */
	CAM_UNI_REG_B_RAWI_BASE_ADDR                    CAM_UNI_B_RAWI_BASE_ADDR;                        /* 0740, 0x1A003740 */
	UINT32                                          rsv_0744;                                        /* 0744, 0x1A003744 */
	CAM_UNI_REG_B_RAWI_OFST_ADDR                    CAM_UNI_B_RAWI_OFST_ADDR;                        /* 0748, 0x1A003748 */
	CAM_UNI_REG_B_RAWI_DRS                          CAM_UNI_B_RAWI_DRS;                              /* 074C, 0x1A00374C */
	CAM_UNI_REG_B_RAWI_XSIZE                        CAM_UNI_B_RAWI_XSIZE;                            /* 0750, 0x1A003750 */
	CAM_UNI_REG_B_RAWI_YSIZE                        CAM_UNI_B_RAWI_YSIZE;                            /* 0754, 0x1A003754 */
	CAM_UNI_REG_B_RAWI_STRIDE                       CAM_UNI_B_RAWI_STRIDE;                           /* 0758, 0x1A003758 */
	CAM_UNI_REG_B_RAWI_CON                          CAM_UNI_B_RAWI_CON;                              /* 075C, 0x1A00375C */
	CAM_UNI_REG_B_RAWI_CON2                         CAM_UNI_B_RAWI_CON2;                             /* 0760, 0x1A003760 */
	CAM_UNI_REG_B_RAWI_CON3                         CAM_UNI_B_RAWI_CON3;                             /* 0764, 0x1A003764 */
	UINT32                                          rsv_0768;                                        /* 0768, 0x1A003768 */
	CAM_UNI_REG_B_RAWI_CON4                         CAM_UNI_B_RAWI_CON4;                             /* 076C, 0x1A00376C */
	CAM_UNI_REG_B_DMA_ERR_CTRL                      CAM_UNI_B_DMA_ERR_CTRL;                          /* 0770, 0x1A003770 */
	CAM_UNI_REG_B_EISO_ERR_STAT                     CAM_UNI_B_EISO_ERR_STAT;                         /* 0774, 0x1A003774 */
	CAM_UNI_REG_B_FLKO_ERR_STAT                     CAM_UNI_B_FLKO_ERR_STAT;                         /* 0778, 0x1A003778 */
	CAM_UNI_REG_B_RSSO_A_ERR_STAT                   CAM_UNI_B_RSSO_A_ERR_STAT;                       /* 077C, 0x1A00377C */
	CAM_UNI_REG_B_RSSO_B_ERR_STAT                   CAM_UNI_B_RSSO_B_ERR_STAT;                       /* 0780, 0x1A003780 */
	CAM_UNI_REG_B_RAWI_ERR_STAT                     CAM_UNI_B_RAWI_ERR_STAT;                         /* 0784, 0x1A003784 */
	CAM_UNI_REG_B_DMA_DEBUG_ADDR                    CAM_UNI_B_DMA_DEBUG_ADDR;                        /* 0788, 0x1A003788 */
	CAM_UNI_REG_B_DMA_RSV1                          CAM_UNI_B_DMA_RSV1;                              /* 078C, 0x1A00378C */
	CAM_UNI_REG_B_DMA_RSV2                          CAM_UNI_B_DMA_RSV2;                              /* 0790, 0x1A003790 */
	CAM_UNI_REG_B_DMA_RSV3                          CAM_UNI_B_DMA_RSV3;                              /* 0794, 0x1A003794 */
	CAM_UNI_REG_B_DMA_RSV4                          CAM_UNI_B_DMA_RSV4;                              /* 0798, 0x1A003798 */
	CAM_UNI_REG_B_DMA_RSV5                          CAM_UNI_B_DMA_RSV5;                              /* 079C, 0x1A00379C */
	CAM_UNI_REG_B_DMA_RSV6                          CAM_UNI_B_DMA_RSV6;                              /* 07A0, 0x1A0037A0 */
	CAM_UNI_REG_B_DMA_DEBUG_SEL                     CAM_UNI_B_DMA_DEBUG_SEL;                         /* 07A4, 0x1A0037A4 */
	CAM_UNI_REG_B_DMA_BW_SELF_TEST                  CAM_UNI_B_DMA_BW_SELF_TEST;                      /* 07A8, 0x1A0037A8 */
	UINT32                                          rsv_07AC[5];                                     /* 07AC..07BF, 0x1A0037AC..1A0037BF */
	CAM_UNI_REG_B_DMA_FRAME_HEADER_EN               CAM_UNI_B_DMA_FRAME_HEADER_EN;                   /* 07C0, 0x1A0037C0 */
	CAM_UNI_REG_B_EISO_FH_BASE_ADDR                 CAM_UNI_B_EISO_FH_BASE_ADDR;                     /* 07C4, 0x1A0037C4 */
	CAM_UNI_REG_B_FLKO_FH_BASE_ADDR                 CAM_UNI_B_FLKO_FH_BASE_ADDR;                     /* 07C8, 0x1A0037C8 */
	CAM_UNI_REG_B_RSSO_A_FH_BASE_ADDR               CAM_UNI_B_RSSO_A_FH_BASE_ADDR;                   /* 07CC, 0x1A0037CC */
	CAM_UNI_REG_B_RSSO_B_FH_BASE_ADDR               CAM_UNI_B_RSSO_B_FH_BASE_ADDR;                   /* 07D0, 0x1A0037D0 */
	UINT32                                          rsv_07D4[3];                                     /* 07D4..07DF, 0x1A0037D4..1A0037DF */
	CAM_UNI_REG_B_EISO_FH_SPARE_2                   CAM_UNI_B_EISO_FH_SPARE_2;                       /* 07E0, 0x1A0037E0 */
	CAM_UNI_REG_B_EISO_FH_SPARE_3                   CAM_UNI_B_EISO_FH_SPARE_3;                       /* 07E4, 0x1A0037E4 */
	CAM_UNI_REG_B_EISO_FH_SPARE_4                   CAM_UNI_B_EISO_FH_SPARE_4;                       /* 07E8, 0x1A0037E8 */
	CAM_UNI_REG_B_EISO_FH_SPARE_5                   CAM_UNI_B_EISO_FH_SPARE_5;                       /* 07EC, 0x1A0037EC */
	CAM_UNI_REG_B_EISO_FH_SPARE_6                   CAM_UNI_B_EISO_FH_SPARE_6;                       /* 07F0, 0x1A0037F0 */
	CAM_UNI_REG_B_EISO_FH_SPARE_7                   CAM_UNI_B_EISO_FH_SPARE_7;                       /* 07F4, 0x1A0037F4 */
	CAM_UNI_REG_B_EISO_FH_SPARE_8                   CAM_UNI_B_EISO_FH_SPARE_8;                       /* 07F8, 0x1A0037F8 */
	CAM_UNI_REG_B_EISO_FH_SPARE_9                   CAM_UNI_B_EISO_FH_SPARE_9;                       /* 07FC, 0x1A0037FC */
	CAM_UNI_REG_B_EISO_FH_SPARE_10                  CAM_UNI_B_EISO_FH_SPARE_10;                      /* 0800, 0x1A003800 */
	CAM_UNI_REG_B_EISO_FH_SPARE_11                  CAM_UNI_B_EISO_FH_SPARE_11;                      /* 0804, 0x1A003804 */
	CAM_UNI_REG_B_EISO_FH_SPARE_12                  CAM_UNI_B_EISO_FH_SPARE_12;                      /* 0808, 0x1A003808 */
	CAM_UNI_REG_B_EISO_FH_SPARE_13                  CAM_UNI_B_EISO_FH_SPARE_13;                      /* 080C, 0x1A00380C */
	CAM_UNI_REG_B_EISO_FH_SPARE_14                  CAM_UNI_B_EISO_FH_SPARE_14;                      /* 0810, 0x1A003810 */
	CAM_UNI_REG_B_EISO_FH_SPARE_15                  CAM_UNI_B_EISO_FH_SPARE_15;                      /* 0814, 0x1A003814 */
	CAM_UNI_REG_B_EISO_FH_SPARE_16                  CAM_UNI_B_EISO_FH_SPARE_16;                      /* 0818, 0x1A003818 */
	UINT32                                          rsv_081C;                                        /* 081C, 0x1A00381C */
	CAM_UNI_REG_B_FLKO_FH_SPARE_2                   CAM_UNI_B_FLKO_FH_SPARE_2;                       /* 0820, 0x1A003820 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_3                   CAM_UNI_B_FLKO_FH_SPARE_3;                       /* 0824, 0x1A003824 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_4                   CAM_UNI_B_FLKO_FH_SPARE_4;                       /* 0828, 0x1A003828 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_5                   CAM_UNI_B_FLKO_FH_SPARE_5;                       /* 082C, 0x1A00382C */
	CAM_UNI_REG_B_FLKO_FH_SPARE_6                   CAM_UNI_B_FLKO_FH_SPARE_6;                       /* 0830, 0x1A003830 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_7                   CAM_UNI_B_FLKO_FH_SPARE_7;                       /* 0834, 0x1A003834 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_8                   CAM_UNI_B_FLKO_FH_SPARE_8;                       /* 0838, 0x1A003838 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_9                   CAM_UNI_B_FLKO_FH_SPARE_9;                       /* 083C, 0x1A00383C */
	CAM_UNI_REG_B_FLKO_FH_SPARE_10                  CAM_UNI_B_FLKO_FH_SPARE_10;                      /* 0840, 0x1A003840 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_11                  CAM_UNI_B_FLKO_FH_SPARE_11;                      /* 0844, 0x1A003844 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_12                  CAM_UNI_B_FLKO_FH_SPARE_12;                      /* 0848, 0x1A003848 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_13                  CAM_UNI_B_FLKO_FH_SPARE_13;                      /* 084C, 0x1A00384C */
	CAM_UNI_REG_B_FLKO_FH_SPARE_14                  CAM_UNI_B_FLKO_FH_SPARE_14;                      /* 0850, 0x1A003850 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_15                  CAM_UNI_B_FLKO_FH_SPARE_15;                      /* 0854, 0x1A003854 */
	CAM_UNI_REG_B_FLKO_FH_SPARE_16                  CAM_UNI_B_FLKO_FH_SPARE_16;                      /* 0858, 0x1A003858 */
	UINT32                                          rsv_085C;                                        /* 085C, 0x1A00385C */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_2                 CAM_UNI_B_RSSO_A_FH_SPARE_2;                     /* 0860, 0x1A003860 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_3                 CAM_UNI_B_RSSO_A_FH_SPARE_3;                     /* 0864, 0x1A003864 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_4                 CAM_UNI_B_RSSO_A_FH_SPARE_4;                     /* 0868, 0x1A003868 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_5                 CAM_UNI_B_RSSO_A_FH_SPARE_5;                     /* 086C, 0x1A00386C */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_6                 CAM_UNI_B_RSSO_A_FH_SPARE_6;                     /* 0870, 0x1A003870 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_7                 CAM_UNI_B_RSSO_A_FH_SPARE_7;                     /* 0874, 0x1A003874 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_8                 CAM_UNI_B_RSSO_A_FH_SPARE_8;                     /* 0878, 0x1A003878 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_9                 CAM_UNI_B_RSSO_A_FH_SPARE_9;                     /* 087C, 0x1A00387C */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_10                CAM_UNI_B_RSSO_A_FH_SPARE_10;                    /* 0880, 0x1A003880 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_11                CAM_UNI_B_RSSO_A_FH_SPARE_11;                    /* 0884, 0x1A003884 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_12                CAM_UNI_B_RSSO_A_FH_SPARE_12;                    /* 0888, 0x1A003888 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_13                CAM_UNI_B_RSSO_A_FH_SPARE_13;                    /* 088C, 0x1A00388C */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_14                CAM_UNI_B_RSSO_A_FH_SPARE_14;                    /* 0890, 0x1A003890 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_15                CAM_UNI_B_RSSO_A_FH_SPARE_15;                    /* 0894, 0x1A003894 */
	CAM_UNI_REG_B_RSSO_A_FH_SPARE_16                CAM_UNI_B_RSSO_A_FH_SPARE_16;                    /* 0898, 0x1A003898 */
	UINT32                                          rsv_089C;                                        /* 089C, 0x1A00389C */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_2                 CAM_UNI_B_RSSO_B_FH_SPARE_2;                     /* 08A0, 0x1A0038A0 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_3                 CAM_UNI_B_RSSO_B_FH_SPARE_3;                     /* 08A4, 0x1A0038A4 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_4                 CAM_UNI_B_RSSO_B_FH_SPARE_4;                     /* 08A8, 0x1A0038A8 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_5                 CAM_UNI_B_RSSO_B_FH_SPARE_5;                     /* 08AC, 0x1A0038AC */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_6                 CAM_UNI_B_RSSO_B_FH_SPARE_6;                     /* 08B0, 0x1A0038B0 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_7                 CAM_UNI_B_RSSO_B_FH_SPARE_7;                     /* 08B4, 0x1A0038B4 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_8                 CAM_UNI_B_RSSO_B_FH_SPARE_8;                     /* 08B8, 0x1A0038B8 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_9                 CAM_UNI_B_RSSO_B_FH_SPARE_9;                     /* 08BC, 0x1A0038BC */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_10                CAM_UNI_B_RSSO_B_FH_SPARE_10;                    /* 08C0, 0x1A0038C0 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_11                CAM_UNI_B_RSSO_B_FH_SPARE_11;                    /* 08C4, 0x1A0038C4 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_12                CAM_UNI_B_RSSO_B_FH_SPARE_12;                    /* 08C8, 0x1A0038C8 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_13                CAM_UNI_B_RSSO_B_FH_SPARE_13;                    /* 08CC, 0x1A0038CC */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_14                CAM_UNI_B_RSSO_B_FH_SPARE_14;                    /* 08D0, 0x1A0038D0 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_15                CAM_UNI_B_RSSO_B_FH_SPARE_15;                    /* 08D4, 0x1A0038D4 */
	CAM_UNI_REG_B_RSSO_B_FH_SPARE_16                CAM_UNI_B_RSSO_B_FH_SPARE_16;                    /* 08D8, 0x1A0038D8 */
	UINT32                                          rsv_08DC[9];                                     /* 08DC..08FF, 0x1A0038DC..1A0038FF */
	CAM_UNI_REG_UNP2_B_OFST                         CAM_UNI_UNP2_B_OFST;                             /* 0900, 0x1A003900 */
	UINT32                                          rsv_0904[3];                                     /* 0904..090F, 0x1A003904..1A00390F */
	CAM_UNI_REG_QBN3_B_MODE                         CAM_UNI_QBN3_B_MODE;                             /* 0910, 0x1A003910 */
	UINT32                                          rsv_0914[3];                                     /* 0914..091F, 0x1A003914..1A00391F */
	CAM_UNI_REG_SGG3_B_PGN                          CAM_UNI_SGG3_B_PGN;                              /* 0920, 0x1A003920 */
	CAM_UNI_REG_SGG3_B_GMRC_1                       CAM_UNI_SGG3_B_GMRC_1;                           /* 0924, 0x1A003924 */
	CAM_UNI_REG_SGG3_B_GMRC_2                       CAM_UNI_SGG3_B_GMRC_2;                           /* 0928, 0x1A003928 */
	UINT32                                          rsv_092C;                                        /* 092C, 0x1A00392C */
	CAM_UNI_REG_FLK_B_CON                           CAM_UNI_FLK_B_CON;                               /* 0930, 0x1A003930 */
	CAM_UNI_REG_FLK_B_OFST                          CAM_UNI_FLK_B_OFST;                              /* 0934, 0x1A003934 */
	CAM_UNI_REG_FLK_B_SIZE                          CAM_UNI_FLK_B_SIZE;                              /* 0938, 0x1A003938 */
	CAM_UNI_REG_FLK_B_NUM                           CAM_UNI_FLK_B_NUM;                               /* 093C, 0x1A00393C */
	UINT32                                          rsv_0940[4];                                     /* 0940..094F, 0x1A003940..1A00394F */
	CAM_UNI_REG_EIS_B_PREP_ME_CTRL1                 CAM_UNI_EIS_B_PREP_ME_CTRL1;                     /* 0950, 0x1A003950 */
	CAM_UNI_REG_EIS_B_PREP_ME_CTRL2                 CAM_UNI_EIS_B_PREP_ME_CTRL2;                     /* 0954, 0x1A003954 */
	CAM_UNI_REG_EIS_B_LMV_TH                        CAM_UNI_EIS_B_LMV_TH;                            /* 0958, 0x1A003958 */
	CAM_UNI_REG_EIS_B_FL_OFFSET                     CAM_UNI_EIS_B_FL_OFFSET;                         /* 095C, 0x1A00395C */
	CAM_UNI_REG_EIS_B_MB_OFFSET                     CAM_UNI_EIS_B_MB_OFFSET;                         /* 0960, 0x1A003960 */
	CAM_UNI_REG_EIS_B_MB_INTERVAL                   CAM_UNI_EIS_B_MB_INTERVAL;                       /* 0964, 0x1A003964 */
	CAM_UNI_REG_EIS_B_GMV                           CAM_UNI_EIS_B_GMV;                               /* 0968, 0x1A003968 */
	CAM_UNI_REG_EIS_B_ERR_CTRL                      CAM_UNI_EIS_B_ERR_CTRL;                          /* 096C, 0x1A00396C */
	CAM_UNI_REG_EIS_B_IMAGE_CTRL                    CAM_UNI_EIS_B_IMAGE_CTRL;                        /* 0970, 0x1A003970 */
	UINT32                                          rsv_0974[11];                                    /* 0974..099F, 0x1A003974..1A00399F */
	CAM_UNI_REG_HDS_B_MODE                          CAM_UNI_HDS_B_MODE;                              /* 09A0, 0x1A0039A0 */
	UINT32                                          rsv_09A4[7];                                     /* 09A4..09BF, 0x1A0039A4..1A0039BF */
	CAM_UNI_REG_RSS_B_CONTROL                       CAM_UNI_RSS_B_CONTROL;                           /* 09C0, 0x1A0039C0 */
	CAM_UNI_REG_RSS_B_IN_IMG                        CAM_UNI_RSS_B_IN_IMG;                            /* 09C4, 0x1A0039C4 */
	CAM_UNI_REG_RSS_B_OUT_IMG                       CAM_UNI_RSS_B_OUT_IMG;                           /* 09C8, 0x1A0039C8 */
	CAM_UNI_REG_RSS_B_HORI_STEP                     CAM_UNI_RSS_B_HORI_STEP;                         /* 09CC, 0x1A0039CC */
	CAM_UNI_REG_RSS_B_VERT_STEP                     CAM_UNI_RSS_B_VERT_STEP;                         /* 09D0, 0x1A0039D0 */
	CAM_UNI_REG_RSS_B_HORI_INT_OFST                 CAM_UNI_RSS_B_HORI_INT_OFST;                     /* 09D4, 0x1A0039D4 */
	CAM_UNI_REG_RSS_B_HORI_SUB_OFST                 CAM_UNI_RSS_B_HORI_SUB_OFST;                     /* 09D8, 0x1A0039D8 */
	CAM_UNI_REG_RSS_B_VERT_INT_OFST                 CAM_UNI_RSS_B_VERT_INT_OFST;                     /* 09DC, 0x1A0039DC */
	CAM_UNI_REG_RSS_B_VERT_SUB_OFST                 CAM_UNI_RSS_B_VERT_SUB_OFST;                     /* 09E0, 0x1A0039E0 */
	UINT32                                          rsv_09E4[391];                                   /* 09E4..0FFF, 1A0039E4..1A003FFF */
}cam_uni_reg_t;

typedef volatile struct _cam_reg_t_	/* 0x1A004000..0x1A004FFF */
{
	CAM_REG_CTL_START                               CAM_CTL_START;                                   /* 0000, 0x1A004000, CAM_A_CTL_START */
	CAM_REG_CTL_EN                                  CAM_CTL_EN;                                      /* 0004, 0x1A004004, CAM_A_CTL_EN */
	CAM_REG_CTL_DMA_EN                              CAM_CTL_DMA_EN;                                  /* 0008, 0x1A004008, CAM_A_CTL_DMA_EN */
	CAM_REG_CTL_FMT_SEL                             CAM_CTL_FMT_SEL;                                 /* 000C, 0x1A00400C, CAM_A_CTL_FMT_SEL */
	CAM_REG_CTL_SEL                                 CAM_CTL_SEL;                                     /* 0010, 0x1A004010, CAM_A_CTL_SEL */
	CAM_REG_CTL_MISC                                CAM_CTL_MISC;                                    /* 0014, 0x1A004014, CAM_A_CTL_MISC */
	UINT32                                          rsv_0018[2];                                     /* 0018..001F, 0x1A004018..1A00401F */
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
	/// Add for twin use: 0x1A004050, CAM_CTL_TWIN_STATUS: TWIN_EN
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
	UINT32                                          rsv_00DC[13];                                    /* 00DC..010F, 0x1A0040DC..1A00410F */
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
	UINT32                                          rsv_0150[4];                                     /* 0150..015F, 0x1A004150..1A00415F */
	CAM_REG_CQ_EN                                   CAM_CQ_EN;                                       /* 0160, 0x1A004160, CAM_A_CQ_EN */
	CAM_REG_CQ_THR0_CTL                             CAM_CQ_THR0_CTL;                                 /* 0164, 0x1A004164, CAM_A_CQ_THR0_CTL */
	CAM_REG_CQ_THR0_BASEADDR                        CAM_CQ_THR0_BASEADDR;                            /* 0168, 0x1A004168, CAM_A_CQ_THR0_BASEADDR */
	CAM_REG_CQ_THR0_DESC_SIZE                       CAM_CQ_THR0_DESC_SIZE;                           /* 016C, 0x1A00416C, CAM_A_CQ_THR0_DESC_SIZE */
	CAM_REG_CQ_THR1_CTL                             CAM_CQ_THR1_CTL;                                 /* 0170, 0x1A004170, CAM_A_CQ_THR1_CTL */
	CAM_REG_CQ_THR1_BASEADDR                        CAM_CQ_THR1_BASEADDR;                            /* 0174, 0x1A004174, CAM_A_CQ_THR1_BASEADDR */
	CAM_REG_CQ_THR1_DESC_SIZE                       CAM_CQ_THR1_DESC_SIZE;                           /* 0178, 0x1A004178, CAM_A_CQ_THR1_DESC_SIZE */
	CAM_REG_CQ_THR2_CTL                             CAM_CQ_THR2_CTL;                                 /* 017C, 0x1A00417C, CAM_A_CQ_THR2_CTL */
	CAM_REG_CQ_THR2_BASEADDR                        CAM_CQ_THR2_BASEADDR;                            /* 0180, 0x1A004180, CAM_A_CQ_THR2_BASEADDR */
	CAM_REG_CQ_THR2_DESC_SIZE                       CAM_CQ_THR2_DESC_SIZE;                           /* 0184, 0x1A004184, CAM_A_CQ_THR2_DESC_SIZE */
	CAM_REG_CQ_THR3_CTL                             CAM_CQ_THR3_CTL;                                 /* 0188, 0x1A004188, CAM_A_CQ_THR3_CTL */
	CAM_REG_CQ_THR3_BASEADDR                        CAM_CQ_THR3_BASEADDR;                            /* 018C, 0x1A00418C, CAM_A_CQ_THR3_BASEADDR */
	CAM_REG_CQ_THR3_DESC_SIZE                       CAM_CQ_THR3_DESC_SIZE;                           /* 0190, 0x1A004190, CAM_A_CQ_THR3_DESC_SIZE */
	CAM_REG_CQ_THR4_CTL                             CAM_CQ_THR4_CTL;                                 /* 0194, 0x1A004194, CAM_A_CQ_THR4_CTL */
	CAM_REG_CQ_THR4_BASEADDR                        CAM_CQ_THR4_BASEADDR;                            /* 0198, 0x1A004198, CAM_A_CQ_THR4_BASEADDR */
	CAM_REG_CQ_THR4_DESC_SIZE                       CAM_CQ_THR4_DESC_SIZE;                           /* 019C, 0x1A00419C, CAM_A_CQ_THR4_DESC_SIZE */
	CAM_REG_CQ_THR5_CTL                             CAM_CQ_THR5_CTL;                                 /* 01A0, 0x1A0041A0, CAM_A_CQ_THR5_CTL */
	CAM_REG_CQ_THR5_BASEADDR                        CAM_CQ_THR5_BASEADDR;                            /* 01A4, 0x1A0041A4, CAM_A_CQ_THR5_BASEADDR */
	CAM_REG_CQ_THR5_DESC_SIZE                       CAM_CQ_THR5_DESC_SIZE;                           /* 01A8, 0x1A0041A8, CAM_A_CQ_THR5_DESC_SIZE */
	CAM_REG_CQ_THR6_CTL                             CAM_CQ_THR6_CTL;                                 /* 01AC, 0x1A0041AC, CAM_A_CQ_THR6_CTL */
	CAM_REG_CQ_THR6_BASEADDR                        CAM_CQ_THR6_BASEADDR;                            /* 01B0, 0x1A0041B0, CAM_A_CQ_THR6_BASEADDR */
	CAM_REG_CQ_THR6_DESC_SIZE                       CAM_CQ_THR6_DESC_SIZE;                           /* 01B4, 0x1A0041B4, CAM_A_CQ_THR6_DESC_SIZE */
	CAM_REG_CQ_THR7_CTL                             CAM_CQ_THR7_CTL;                                 /* 01B8, 0x1A0041B8, CAM_A_CQ_THR7_CTL */
	CAM_REG_CQ_THR7_BASEADDR                        CAM_CQ_THR7_BASEADDR;                            /* 01BC, 0x1A0041BC, CAM_A_CQ_THR7_BASEADDR */
	CAM_REG_CQ_THR7_DESC_SIZE                       CAM_CQ_THR7_DESC_SIZE;                           /* 01C0, 0x1A0041C0, CAM_A_CQ_THR7_DESC_SIZE */
	CAM_REG_CQ_THR8_CTL                             CAM_CQ_THR8_CTL;                                 /* 01C4, 0x1A0041C4, CAM_A_CQ_THR8_CTL */
	CAM_REG_CQ_THR8_BASEADDR                        CAM_CQ_THR8_BASEADDR;                            /* 01C8, 0x1A0041C8, CAM_A_CQ_THR8_BASEADDR */
	CAM_REG_CQ_THR8_DESC_SIZE                       CAM_CQ_THR8_DESC_SIZE;                           /* 01CC, 0x1A0041CC, CAM_A_CQ_THR8_DESC_SIZE */
	CAM_REG_CQ_THR9_CTL                             CAM_CQ_THR9_CTL;                                 /* 01D0, 0x1A0041D0, CAM_A_CQ_THR9_CTL */
	CAM_REG_CQ_THR9_BASEADDR                        CAM_CQ_THR9_BASEADDR;                            /* 01D4, 0x1A0041D4, CAM_A_CQ_THR9_BASEADDR */
	CAM_REG_CQ_THR9_DESC_SIZE                       CAM_CQ_THR9_DESC_SIZE;                           /* 01D8, 0x1A0041D8, CAM_A_CQ_THR9_DESC_SIZE */
	CAM_REG_CQ_THR10_CTL                            CAM_CQ_THR10_CTL;                                /* 01DC, 0x1A0041DC, CAM_A_CQ_THR10_CTL */
	CAM_REG_CQ_THR10_BASEADDR                       CAM_CQ_THR10_BASEADDR;                           /* 01E0, 0x1A0041E0, CAM_A_CQ_THR10_BASEADDR */
	CAM_REG_CQ_THR10_DESC_SIZE                      CAM_CQ_THR10_DESC_SIZE;                          /* 01E4, 0x1A0041E4, CAM_A_CQ_THR10_DESC_SIZE */
	CAM_REG_CQ_THR11_CTL                            CAM_CQ_THR11_CTL;                                /* 01E8, 0x1A0041E8, CAM_A_CQ_THR11_CTL */
	CAM_REG_CQ_THR11_BASEADDR                       CAM_CQ_THR11_BASEADDR;                           /* 01EC, 0x1A0041EC, CAM_A_CQ_THR11_BASEADDR */
	CAM_REG_CQ_THR11_DESC_SIZE                      CAM_CQ_THR11_DESC_SIZE;                          /* 01F0, 0x1A0041F0, CAM_A_CQ_THR11_DESC_SIZE */
	CAM_REG_CQ_THR12_CTL                            CAM_CQ_THR12_CTL;                                /* 01F4, 0x1A0041F4, CAM_A_CQ_THR12_CTL */
	CAM_REG_CQ_THR12_BASEADDR                       CAM_CQ_THR12_BASEADDR;                           /* 01F8, 0x1A0041F8, CAM_A_CQ_THR12_BASEADDR */
	CAM_REG_CQ_THR12_DESC_SIZE                      CAM_CQ_THR12_DESC_SIZE;                          /* 01FC, 0x1A0041FC, CAM_A_CQ_THR12_DESC_SIZE */
	CAM_REG_DMA_SOFT_RSTSTAT                        CAM_DMA_SOFT_RSTSTAT;                            /* 0200, 0x1A004200, CAM_A_DMA_SOFT_RSTSTAT */
	CAM_REG_CQ0I_BASE_ADDR                          CAM_CQ0I_BASE_ADDR;                              /* 0204, 0x1A004204, CAM_A_CQ0I_BASE_ADDR */
	CAM_REG_CQ0I_XSIZE                              CAM_CQ0I_XSIZE;                                  /* 0208, 0x1A004208, CAM_A_CQ0I_XSIZE */
	CAM_REG_VERTICAL_FLIP_EN                        CAM_VERTICAL_FLIP_EN;                            /* 020C, 0x1A00420C, CAM_A_VERTICAL_FLIP_EN */
	CAM_REG_DMA_SOFT_RESET                          CAM_DMA_SOFT_RESET;                              /* 0210, 0x1A004210, CAM_A_DMA_SOFT_RESET */
	CAM_REG_LAST_ULTRA_EN                           CAM_LAST_ULTRA_EN;                               /* 0214, 0x1A004214, CAM_A_LAST_ULTRA_EN */
	CAM_REG_SPECIAL_FUN_EN                          CAM_SPECIAL_FUN_EN;                              /* 0218, 0x1A004218, CAM_A_SPECIAL_FUN_EN */
	UINT32                                          rsv_021C;                                        /* 021C, 0x1A00421C */
	CAM_REG_IMGO_BASE_ADDR                          CAM_IMGO_BASE_ADDR;                              /* 0220, 0x1A004220, CAM_A_IMGO_BASE_ADDR */
	UINT32                                          rsv_0224;                                        /* 0224, 0x1A004224 */
	CAM_REG_IMGO_OFST_ADDR                          CAM_IMGO_OFST_ADDR;                              /* 0228, 0x1A004228, CAM_A_IMGO_OFST_ADDR */
	CAM_REG_IMGO_DRS                                CAM_IMGO_DRS;                                    /* 022C, 0x1A00422C, CAM_A_IMGO_DRS */
	CAM_REG_IMGO_XSIZE                              CAM_IMGO_XSIZE;                                  /* 0230, 0x1A004230, CAM_A_IMGO_XSIZE */
	CAM_REG_IMGO_YSIZE                              CAM_IMGO_YSIZE;                                  /* 0234, 0x1A004234, CAM_A_IMGO_YSIZE */
	CAM_REG_IMGO_STRIDE                             CAM_IMGO_STRIDE;                                 /* 0238, 0x1A004238, CAM_A_IMGO_STRIDE */
	CAM_REG_IMGO_CON                                CAM_IMGO_CON;                                    /* 023C, 0x1A00423C, CAM_A_IMGO_CON */
	CAM_REG_IMGO_CON2                               CAM_IMGO_CON2;                                   /* 0240, 0x1A004240, CAM_A_IMGO_CON2 */
	CAM_REG_IMGO_CON3                               CAM_IMGO_CON3;                                   /* 0244, 0x1A004244, CAM_A_IMGO_CON3 */
	CAM_REG_IMGO_CROP                               CAM_IMGO_CROP;                                   /* 0248, 0x1A004248, CAM_A_IMGO_CROP */
	CAM_REG_IMGO_CON4                               CAM_IMGO_CON4;                                   /* 024C, 0x1A00424C, CAM_A_IMGO_CON4 */
	CAM_REG_RRZO_BASE_ADDR                          CAM_RRZO_BASE_ADDR;                              /* 0250, 0x1A004250, CAM_A_RRZO_BASE_ADDR */
	UINT32                                          rsv_0254;                                        /* 0254, 0x1A004254 */
	CAM_REG_RRZO_OFST_ADDR                          CAM_RRZO_OFST_ADDR;                              /* 0258, 0x1A004258, CAM_A_RRZO_OFST_ADDR */
	CAM_REG_RRZO_DRS                                CAM_RRZO_DRS;                                    /* 025C, 0x1A00425C, CAM_A_RRZO_DRS */
	CAM_REG_RRZO_XSIZE                              CAM_RRZO_XSIZE;                                  /* 0260, 0x1A004260, CAM_A_RRZO_XSIZE */
	CAM_REG_RRZO_YSIZE                              CAM_RRZO_YSIZE;                                  /* 0264, 0x1A004264, CAM_A_RRZO_YSIZE */
	CAM_REG_RRZO_STRIDE                             CAM_RRZO_STRIDE;                                 /* 0268, 0x1A004268, CAM_A_RRZO_STRIDE */
	CAM_REG_RRZO_CON                                CAM_RRZO_CON;                                    /* 026C, 0x1A00426C, CAM_A_RRZO_CON */
	CAM_REG_RRZO_CON2                               CAM_RRZO_CON2;                                   /* 0270, 0x1A004270, CAM_A_RRZO_CON2 */
	CAM_REG_RRZO_CON3                               CAM_RRZO_CON3;                                   /* 0274, 0x1A004274, CAM_A_RRZO_CON3 */
	CAM_REG_RRZO_CROP                               CAM_RRZO_CROP;                                   /* 0278, 0x1A004278, CAM_A_RRZO_CROP */
	CAM_REG_RRZO_CON4                               CAM_RRZO_CON4;                                   /* 027C, 0x1A00427C, CAM_A_RRZO_CON4 */
	CAM_REG_AAO_BASE_ADDR                           CAM_AAO_BASE_ADDR;                               /* 0280, 0x1A004280, CAM_A_AAO_BASE_ADDR */
	UINT32                                          rsv_0284;                                        /* 0284, 0x1A004284 */
	CAM_REG_AAO_OFST_ADDR                           CAM_AAO_OFST_ADDR;                               /* 0288, 0x1A004288, CAM_A_AAO_OFST_ADDR */
	CAM_REG_AAO_DRS                                 CAM_AAO_DRS;                                     /* 028C, 0x1A00428C, CAM_A_AAO_DRS */
	CAM_REG_AAO_XSIZE                               CAM_AAO_XSIZE;                                   /* 0290, 0x1A004290, CAM_A_AAO_XSIZE */
	CAM_REG_AAO_YSIZE                               CAM_AAO_YSIZE;                                   /* 0294, 0x1A004294, CAM_A_AAO_YSIZE */
	CAM_REG_AAO_STRIDE                              CAM_AAO_STRIDE;                                  /* 0298, 0x1A004298, CAM_A_AAO_STRIDE */
	CAM_REG_AAO_CON                                 CAM_AAO_CON;                                     /* 029C, 0x1A00429C, CAM_A_AAO_CON */
	CAM_REG_AAO_CON2                                CAM_AAO_CON2;                                    /* 02A0, 0x1A0042A0, CAM_A_AAO_CON2 */
	CAM_REG_AAO_CON3                                CAM_AAO_CON3;                                    /* 02A4, 0x1A0042A4, CAM_A_AAO_CON3 */
	UINT32                                          rsv_02A8;                                        /* 02A8, 0x1A0042A8 */
	CAM_REG_AAO_CON4                                CAM_AAO_CON4;                                    /* 02AC, 0x1A0042AC, CAM_A_AAO_CON4 */
	CAM_REG_AFO_BASE_ADDR                           CAM_AFO_BASE_ADDR;                               /* 02B0, 0x1A0042B0, CAM_A_AFO_BASE_ADDR */
	UINT32                                          rsv_02B4;                                        /* 02B4, 0x1A0042B4 */
	CAM_REG_AFO_OFST_ADDR                           CAM_AFO_OFST_ADDR;                               /* 02B8, 0x1A0042B8, CAM_A_AFO_OFST_ADDR */
	CAM_REG_AFO_DRS                                 CAM_AFO_DRS;                                     /* 02BC, 0x1A0042BC, CAM_A_AFO_DRS */
	CAM_REG_AFO_XSIZE                               CAM_AFO_XSIZE;                                   /* 02C0, 0x1A0042C0, CAM_A_AFO_XSIZE */
	CAM_REG_AFO_YSIZE                               CAM_AFO_YSIZE;                                   /* 02C4, 0x1A0042C4, CAM_A_AFO_YSIZE */
	CAM_REG_AFO_STRIDE                              CAM_AFO_STRIDE;                                  /* 02C8, 0x1A0042C8, CAM_A_AFO_STRIDE */
	CAM_REG_AFO_CON                                 CAM_AFO_CON;                                     /* 02CC, 0x1A0042CC, CAM_A_AFO_CON */
	CAM_REG_AFO_CON2                                CAM_AFO_CON2;                                    /* 02D0, 0x1A0042D0, CAM_A_AFO_CON2 */
	CAM_REG_AFO_CON3                                CAM_AFO_CON3;                                    /* 02D4, 0x1A0042D4, CAM_A_AFO_CON3 */
	UINT32                                          rsv_02D8;                                        /* 02D8, 0x1A0042D8 */
	CAM_REG_AFO_CON4                                CAM_AFO_CON4;                                    /* 02DC, 0x1A0042DC, CAM_A_AFO_CON4 */
	CAM_REG_LCSO_BASE_ADDR                          CAM_LCSO_BASE_ADDR;                              /* 02E0, 0x1A0042E0, CAM_A_LCSO_BASE_ADDR */
	UINT32                                          rsv_02E4;                                        /* 02E4, 0x1A0042E4 */
	CAM_REG_LCSO_OFST_ADDR                          CAM_LCSO_OFST_ADDR;                              /* 02E8, 0x1A0042E8, CAM_A_LCSO_OFST_ADDR */
	CAM_REG_LCSO_DRS                                CAM_LCSO_DRS;                                    /* 02EC, 0x1A0042EC, CAM_A_LCSO_DRS */
	CAM_REG_LCSO_XSIZE                              CAM_LCSO_XSIZE;                                  /* 02F0, 0x1A0042F0, CAM_A_LCSO_XSIZE */
	CAM_REG_LCSO_YSIZE                              CAM_LCSO_YSIZE;                                  /* 02F4, 0x1A0042F4, CAM_A_LCSO_YSIZE */
	CAM_REG_LCSO_STRIDE                             CAM_LCSO_STRIDE;                                 /* 02F8, 0x1A0042F8, CAM_A_LCSO_STRIDE */
	CAM_REG_LCSO_CON                                CAM_LCSO_CON;                                    /* 02FC, 0x1A0042FC, CAM_A_LCSO_CON */
	CAM_REG_LCSO_CON2                               CAM_LCSO_CON2;                                   /* 0300, 0x1A004300, CAM_A_LCSO_CON2 */
	CAM_REG_LCSO_CON3                               CAM_LCSO_CON3;                                   /* 0304, 0x1A004304, CAM_A_LCSO_CON3 */
	UINT32                                          rsv_0308;                                        /* 0308, 0x1A004308 */
	CAM_REG_LCSO_CON4                               CAM_LCSO_CON4;                                   /* 030C, 0x1A00430C, CAM_A_LCSO_CON4 */
	CAM_REG_UFEO_BASE_ADDR                          CAM_UFEO_BASE_ADDR;                              /* 0310, 0x1A004310, CAM_A_UFEO_BASE_ADDR */
	UINT32                                          rsv_0314;                                        /* 0314, 0x1A004314 */
	CAM_REG_UFEO_OFST_ADDR                          CAM_UFEO_OFST_ADDR;                              /* 0318, 0x1A004318, CAM_A_UFEO_OFST_ADDR */
	CAM_REG_UFEO_DRS                                CAM_UFEO_DRS;                                    /* 031C, 0x1A00431C, CAM_A_UFEO_DRS */
	CAM_REG_UFEO_XSIZE                              CAM_UFEO_XSIZE;                                  /* 0320, 0x1A004320, CAM_A_UFEO_XSIZE */
	CAM_REG_UFEO_YSIZE                              CAM_UFEO_YSIZE;                                  /* 0324, 0x1A004324, CAM_A_UFEO_YSIZE */
	CAM_REG_UFEO_STRIDE                             CAM_UFEO_STRIDE;                                 /* 0328, 0x1A004328, CAM_A_UFEO_STRIDE */
	CAM_REG_UFEO_CON                                CAM_UFEO_CON;                                    /* 032C, 0x1A00432C, CAM_A_UFEO_CON */
	CAM_REG_UFEO_CON2                               CAM_UFEO_CON2;                                   /* 0330, 0x1A004330, CAM_A_UFEO_CON2 */
	CAM_REG_UFEO_CON3                               CAM_UFEO_CON3;                                   /* 0334, 0x1A004334, CAM_A_UFEO_CON3 */
	UINT32                                          rsv_0338;                                        /* 0338, 0x1A004338 */
	CAM_REG_UFEO_CON4                               CAM_UFEO_CON4;                                   /* 033C, 0x1A00433C, CAM_A_UFEO_CON4 */
	CAM_REG_PDO_BASE_ADDR                           CAM_PDO_BASE_ADDR;                               /* 0340, 0x1A004340, CAM_A_PDO_BASE_ADDR */
	UINT32                                          rsv_0344;                                        /* 0344, 0x1A004344 */
	CAM_REG_PDO_OFST_ADDR                           CAM_PDO_OFST_ADDR;                               /* 0348, 0x1A004348, CAM_A_PDO_OFST_ADDR */
	CAM_REG_PDO_DRS                                 CAM_PDO_DRS;                                     /* 034C, 0x1A00434C, CAM_A_PDO_DRS */
	CAM_REG_PDO_XSIZE                               CAM_PDO_XSIZE;                                   /* 0350, 0x1A004350, CAM_A_PDO_XSIZE */
	CAM_REG_PDO_YSIZE                               CAM_PDO_YSIZE;                                   /* 0354, 0x1A004354, CAM_A_PDO_YSIZE */
	CAM_REG_PDO_STRIDE                              CAM_PDO_STRIDE;                                  /* 0358, 0x1A004358, CAM_A_PDO_STRIDE */
	CAM_REG_PDO_CON                                 CAM_PDO_CON;                                     /* 035C, 0x1A00435C, CAM_A_PDO_CON */
	CAM_REG_PDO_CON2                                CAM_PDO_CON2;                                    /* 0360, 0x1A004360, CAM_A_PDO_CON2 */
	CAM_REG_PDO_CON3                                CAM_PDO_CON3;                                    /* 0364, 0x1A004364, CAM_A_PDO_CON3 */
	UINT32                                          rsv_0368;                                        /* 0368, 0x1A004368 */
	CAM_REG_PDO_CON4                                CAM_PDO_CON4;                                    /* 036C, 0x1A00436C, CAM_A_PDO_CON4 */
	CAM_REG_BPCI_BASE_ADDR                          CAM_BPCI_BASE_ADDR;                              /* 0370, 0x1A004370, CAM_A_BPCI_BASE_ADDR */
	UINT32                                          rsv_0374;                                        /* 0374, 0x1A004374 */
	CAM_REG_BPCI_OFST_ADDR                          CAM_BPCI_OFST_ADDR;                              /* 0378, 0x1A004378, CAM_A_BPCI_OFST_ADDR */
	CAM_REG_BPCI_DRS                                CAM_BPCI_DRS;                                    /* 037C, 0x1A00437C, CAM_A_BPCI_DRS */
	CAM_REG_BPCI_XSIZE                              CAM_BPCI_XSIZE;                                  /* 0380, 0x1A004380, CAM_A_BPCI_XSIZE */
	CAM_REG_BPCI_YSIZE                              CAM_BPCI_YSIZE;                                  /* 0384, 0x1A004384, CAM_A_BPCI_YSIZE */
	CAM_REG_BPCI_STRIDE                             CAM_BPCI_STRIDE;                                 /* 0388, 0x1A004388, CAM_A_BPCI_STRIDE */
	CAM_REG_BPCI_CON                                CAM_BPCI_CON;                                    /* 038C, 0x1A00438C, CAM_A_BPCI_CON */
	CAM_REG_BPCI_CON2                               CAM_BPCI_CON2;                                   /* 0390, 0x1A004390, CAM_A_BPCI_CON2 */
	CAM_REG_BPCI_CON3                               CAM_BPCI_CON3;                                   /* 0394, 0x1A004394, CAM_A_BPCI_CON3 */
	UINT32                                          rsv_0398;                                        /* 0398, 0x1A004398 */
	CAM_REG_BPCI_CON4                               CAM_BPCI_CON4;                                   /* 039C, 0x1A00439C, CAM_A_BPCI_CON4 */
	CAM_REG_CACI_BASE_ADDR                          CAM_CACI_BASE_ADDR;                              /* 03A0, 0x1A0043A0, CAM_A_CACI_BASE_ADDR */
	UINT32                                          rsv_03A4;                                        /* 03A4, 0x1A0043A4 */
	CAM_REG_CACI_OFST_ADDR                          CAM_CACI_OFST_ADDR;                              /* 03A8, 0x1A0043A8, CAM_A_CACI_OFST_ADDR */
	CAM_REG_CACI_DRS                                CAM_CACI_DRS;                                    /* 03AC, 0x1A0043AC, CAM_A_CACI_DRS */
	CAM_REG_CACI_XSIZE                              CAM_CACI_XSIZE;                                  /* 03B0, 0x1A0043B0, CAM_A_CACI_XSIZE */
	CAM_REG_CACI_YSIZE                              CAM_CACI_YSIZE;                                  /* 03B4, 0x1A0043B4, CAM_A_CACI_YSIZE */
	CAM_REG_CACI_STRIDE                             CAM_CACI_STRIDE;                                 /* 03B8, 0x1A0043B8, CAM_A_CACI_STRIDE */
	CAM_REG_CACI_CON                                CAM_CACI_CON;                                    /* 03BC, 0x1A0043BC, CAM_A_CACI_CON */
	CAM_REG_CACI_CON2                               CAM_CACI_CON2;                                   /* 03C0, 0x1A0043C0, CAM_A_CACI_CON2 */
	CAM_REG_CACI_CON3                               CAM_CACI_CON3;                                   /* 03C4, 0x1A0043C4, CAM_A_CACI_CON3 */
	UINT32                                          rsv_03C8;                                        /* 03C8, 0x1A0043C8 */
	CAM_REG_CACI_CON4                               CAM_CACI_CON4;                                   /* 03CC, 0x1A0043CC, CAM_A_CACI_CON4 */
	CAM_REG_LSCI_BASE_ADDR                          CAM_LSCI_BASE_ADDR;                              /* 03D0, 0x1A0043D0, CAM_A_LSCI_BASE_ADDR */
	UINT32                                          rsv_03D4;                                        /* 03D4, 0x1A0043D4 */
	CAM_REG_LSCI_OFST_ADDR                          CAM_LSCI_OFST_ADDR;                              /* 03D8, 0x1A0043D8, CAM_A_LSCI_OFST_ADDR */
	CAM_REG_LSCI_DRS                                CAM_LSCI_DRS;                                    /* 03DC, 0x1A0043DC, CAM_A_LSCI_DRS */
	CAM_REG_LSCI_XSIZE                              CAM_LSCI_XSIZE;                                  /* 03E0, 0x1A0043E0, CAM_A_LSCI_XSIZE */
	CAM_REG_LSCI_YSIZE                              CAM_LSCI_YSIZE;                                  /* 03E4, 0x1A0043E4, CAM_A_LSCI_YSIZE */
	CAM_REG_LSCI_STRIDE                             CAM_LSCI_STRIDE;                                 /* 03E8, 0x1A0043E8, CAM_A_LSCI_STRIDE */
	CAM_REG_LSCI_CON                                CAM_LSCI_CON;                                    /* 03EC, 0x1A0043EC, CAM_A_LSCI_CON */
	CAM_REG_LSCI_CON2                               CAM_LSCI_CON2;                                   /* 03F0, 0x1A0043F0, CAM_A_LSCI_CON2 */
	CAM_REG_LSCI_CON3                               CAM_LSCI_CON3;                                   /* 03F4, 0x1A0043F4, CAM_A_LSCI_CON3 */
	UINT32                                          rsv_03F8;                                        /* 03F8, 0x1A0043F8 */
	CAM_REG_LSCI_CON4                               CAM_LSCI_CON4;                                   /* 03FC, 0x1A0043FC, CAM_A_LSCI_CON4 */
	CAM_REG_LSC3I_BASE_ADDR                         CAM_LSC3I_BASE_ADDR;                             /* 0400, 0x1A004400, CAM_A_LSC3I_BASE_ADDR */
	UINT32                                          rsv_0404;                                        /* 0404, 0x1A004404 */
	CAM_REG_LSC3I_OFST_ADDR                         CAM_LSC3I_OFST_ADDR;                             /* 0408, 0x1A004408, CAM_A_LSC3I_OFST_ADDR */
	CAM_REG_LSC3I_DRS                               CAM_LSC3I_DRS;                                   /* 040C, 0x1A00440C, CAM_A_LSC3I_DRS */
	CAM_REG_LSC3I_XSIZE                             CAM_LSC3I_XSIZE;                                 /* 0410, 0x1A004410, CAM_A_LSC3I_XSIZE */
	CAM_REG_LSC3I_YSIZE                             CAM_LSC3I_YSIZE;                                 /* 0414, 0x1A004414, CAM_A_LSC3I_YSIZE */
	CAM_REG_LSC3I_STRIDE                            CAM_LSC3I_STRIDE;                                /* 0418, 0x1A004418, CAM_A_LSC3I_STRIDE */
	CAM_REG_LSC3I_CON                               CAM_LSC3I_CON;                                   /* 041C, 0x1A00441C, CAM_A_LSC3I_CON */
	CAM_REG_LSC3I_CON2                              CAM_LSC3I_CON2;                                  /* 0420, 0x1A004420, CAM_A_LSC3I_CON2 */
	CAM_REG_LSC3I_CON3                              CAM_LSC3I_CON3;                                  /* 0424, 0x1A004424, CAM_A_LSC3I_CON3 */
	UINT32                                          rsv_0428;                                        /* 0428, 0x1A004428 */
	CAM_REG_LSC3I_CON4                              CAM_LSC3I_CON4;                                  /* 042C, 0x1A00442C, CAM_A_LSC3I_CON4 */
	UINT32                                          rsv_0430[4];                                     /* 0430..043F, 0x1A004430..1A00443F */
	CAM_REG_DMA_ERR_CTRL                            CAM_DMA_ERR_CTRL;                                /* 0440, 0x1A004440, CAM_A_DMA_ERR_CTRL */
	CAM_REG_IMGO_ERR_STAT                           CAM_IMGO_ERR_STAT;                               /* 0444, 0x1A004444, CAM_A_IMGO_ERR_STAT */
	CAM_REG_RRZO_ERR_STAT                           CAM_RRZO_ERR_STAT;                               /* 0448, 0x1A004448, CAM_A_RRZO_ERR_STAT */
	CAM_REG_AAO_ERR_STAT                            CAM_AAO_ERR_STAT;                                /* 044C, 0x1A00444C, CAM_A_AAO_ERR_STAT */
	CAM_REG_AFO_ERR_STAT                            CAM_AFO_ERR_STAT;                                /* 0450, 0x1A004450, CAM_A_AFO_ERR_STAT */
	CAM_REG_LCSO_ERR_STAT                           CAM_LCSO_ERR_STAT;                               /* 0454, 0x1A004454, CAM_A_LCSO_ERR_STAT */
	CAM_REG_UFEO_ERR_STAT                           CAM_UFEO_ERR_STAT;                               /* 0458, 0x1A004458, CAM_A_UFEO_ERR_STAT */
	CAM_REG_PDO_ERR_STAT                            CAM_PDO_ERR_STAT;                                /* 045C, 0x1A00445C, CAM_A_PDO_ERR_STAT */
	CAM_REG_BPCI_ERR_STAT                           CAM_BPCI_ERR_STAT;                               /* 0460, 0x1A004460, CAM_A_BPCI_ERR_STAT */
	CAM_REG_CACI_ERR_STAT                           CAM_CACI_ERR_STAT;                               /* 0464, 0x1A004464, CAM_A_CACI_ERR_STAT */
	CAM_REG_LSCI_ERR_STAT                           CAM_LSCI_ERR_STAT;                               /* 0468, 0x1A004468, CAM_A_LSCI_ERR_STAT */
	CAM_REG_LSC3I_ERR_STAT                          CAM_LSC3I_ERR_STAT;                              /* 046C, 0x1A00446C, CAM_A_LSC3I_ERR_STAT */
	CAM_REG_DMA_DEBUG_ADDR                          CAM_DMA_DEBUG_ADDR;                              /* 0470, 0x1A004470, CAM_A_DMA_DEBUG_ADDR */
	CAM_REG_DMA_RSV1                                CAM_DMA_RSV1;                                    /* 0474, 0x1A004474, CAM_A_DMA_RSV1 */
	CAM_REG_DMA_RSV2                                CAM_DMA_RSV2;                                    /* 0478, 0x1A004478, CAM_A_DMA_RSV2 */
	CAM_REG_DMA_RSV3                                CAM_DMA_RSV3;                                    /* 047C, 0x1A00447C, CAM_A_DMA_RSV3 */
	CAM_REG_DMA_RSV4                                CAM_DMA_RSV4;                                    /* 0480, 0x1A004480, CAM_A_DMA_RSV4 */
	CAM_REG_DMA_RSV5                                CAM_DMA_RSV5;                                    /* 0484, 0x1A004484, CAM_A_DMA_RSV5 */
	CAM_REG_DMA_RSV6                                CAM_DMA_RSV6;                                    /* 0488, 0x1A004488, CAM_A_DMA_RSV6 */
	CAM_REG_DMA_DEBUG_SEL                           CAM_DMA_DEBUG_SEL;                               /* 048C, 0x1A00448C, CAM_A_DMA_DEBUG_SEL */
	CAM_REG_DMA_BW_SELF_TEST                        CAM_DMA_BW_SELF_TEST;                            /* 0490, 0x1A004490, CAM_A_DMA_BW_SELF_TEST */
	UINT32                                          rsv_0494[27];                                    /* 0494..04FF, 0x1A004494..1A0044FF */
	CAM_REG_TG_SEN_MODE                             CAM_TG_SEN_MODE;                                 /* 0500, 0x1A004500, CAM_A_TG_SEN_MODE */
	CAM_REG_TG_VF_CON                               CAM_TG_VF_CON;                                   /* 0504, 0x1A004504, CAM_A_TG_VF_CON */
	CAM_REG_TG_SEN_GRAB_PXL                         CAM_TG_SEN_GRAB_PXL;                             /* 0508, 0x1A004508, CAM_A_TG_SEN_GRAB_PXL */
	CAM_REG_TG_SEN_GRAB_LIN                         CAM_TG_SEN_GRAB_LIN;                             /* 050C, 0x1A00450C, CAM_A_TG_SEN_GRAB_LIN */
	CAM_REG_TG_PATH_CFG                             CAM_TG_PATH_CFG;                                 /* 0510, 0x1A004510, CAM_A_TG_PATH_CFG */
	CAM_REG_TG_MEMIN_CTL                            CAM_TG_MEMIN_CTL;                                /* 0514, 0x1A004514, CAM_A_TG_MEMIN_CTL */
	CAM_REG_TG_INT1                                 CAM_TG_INT1;                                     /* 0518, 0x1A004518, CAM_A_TG_INT1 */
	CAM_REG_TG_INT2                                 CAM_TG_INT2;                                     /* 051C, 0x1A00451C, CAM_A_TG_INT2 */
	CAM_REG_TG_SOF_CNT                              CAM_TG_SOF_CNT;                                  /* 0520, 0x1A004520, CAM_A_TG_SOF_CNT */
	CAM_REG_TG_SOT_CNT                              CAM_TG_SOT_CNT;                                  /* 0524, 0x1A004524, CAM_A_TG_SOT_CNT */
	CAM_REG_TG_EOT_CNT                              CAM_TG_EOT_CNT;                                  /* 0528, 0x1A004528, CAM_A_TG_EOT_CNT */
	CAM_REG_TG_ERR_CTL                              CAM_TG_ERR_CTL;                                  /* 052C, 0x1A00452C, CAM_A_TG_ERR_CTL */
	CAM_REG_TG_DAT_NO                               CAM_TG_DAT_NO;                                   /* 0530, 0x1A004530, CAM_A_TG_DAT_NO */
	CAM_REG_TG_FRM_CNT_ST                           CAM_TG_FRM_CNT_ST;                               /* 0534, 0x1A004534, CAM_A_TG_FRM_CNT_ST */
	CAM_REG_TG_FRMSIZE_ST                           CAM_TG_FRMSIZE_ST;                               /* 0538, 0x1A004538, CAM_A_TG_FRMSIZE_ST */
	CAM_REG_TG_INTER_ST                             CAM_TG_INTER_ST;                                 /* 053C, 0x1A00453C, CAM_A_TG_INTER_ST */
	CAM_REG_TG_FLASHA_CTL                           CAM_TG_FLASHA_CTL;                               /* 0540, 0x1A004540, CAM_A_TG_FLASHA_CTL */
	CAM_REG_TG_FLASHA_LINE_CNT                      CAM_TG_FLASHA_LINE_CNT;                          /* 0544, 0x1A004544, CAM_A_TG_FLASHA_LINE_CNT */
	CAM_REG_TG_FLASHA_POS                           CAM_TG_FLASHA_POS;                               /* 0548, 0x1A004548, CAM_A_TG_FLASHA_POS */
	CAM_REG_TG_FLASHB_CTL                           CAM_TG_FLASHB_CTL;                               /* 054C, 0x1A00454C, CAM_A_TG_FLASHB_CTL */
	CAM_REG_TG_FLASHB_LINE_CNT                      CAM_TG_FLASHB_LINE_CNT;                          /* 0550, 0x1A004550, CAM_A_TG_FLASHB_LINE_CNT */
	CAM_REG_TG_FLASHB_POS                           CAM_TG_FLASHB_POS;                               /* 0554, 0x1A004554, CAM_A_TG_FLASHB_POS */
	CAM_REG_TG_FLASHB_POS1                          CAM_TG_FLASHB_POS1;                              /* 0558, 0x1A004558, CAM_A_TG_FLASHB_POS1 */
	UINT32                                          rsv_055C;                                        /* 055C, 0x1A00455C */
	CAM_REG_TG_I2C_CQ_TRIG                          CAM_TG_I2C_CQ_TRIG;                              /* 0560, 0x1A004560, CAM_A_TG_I2C_CQ_TRIG */
	CAM_REG_TG_CQ_TIMING                            CAM_TG_CQ_TIMING;                                /* 0564, 0x1A004564, CAM_A_TG_CQ_TIMING */
	CAM_REG_TG_CQ_NUM                               CAM_TG_CQ_NUM;                                   /* 0568, 0x1A004568, CAM_A_TG_CQ_NUM */
	UINT32                                          rsv_056C;                                        /* 056C, 0x1A00456C */
	CAM_REG_TG_TIME_STAMP                           CAM_TG_TIME_STAMP;                               /* 0570, 0x1A004570, CAM_A_TG_TIME_STAMP */
	CAM_REG_TG_SUB_PERIOD                           CAM_TG_SUB_PERIOD;                               /* 0574, 0x1A004574, CAM_A_TG_SUB_PERIOD */
	CAM_REG_TG_DAT_NO_R                             CAM_TG_DAT_NO_R;                                 /* 0578, 0x1A004578, CAM_A_TG_DAT_NO_R */
	CAM_REG_TG_FRMSIZE_ST_R                         CAM_TG_FRMSIZE_ST_R;                             /* 057C, 0x1A00457C, CAM_A_TG_FRMSIZE_ST_R */
	CAM_REG_DMX_CTL                                 CAM_DMX_CTL;                                     /* 0580, 0x1A004580, CAM_A_DMX_CTL */
	CAM_REG_DMX_CROP                                CAM_DMX_CROP;                                    /* 0584, 0x1A004584, CAM_A_DMX_CROP */
	CAM_REG_DMX_VSIZE                               CAM_DMX_VSIZE;                                   /* 0588, 0x1A004588, CAM_A_DMX_VSIZE */
	UINT32                                          rsv_058C[5];                                     /* 058C..059F, 0x1A00458C..1A00459F */
	CAM_REG_RMG_HDR_CFG                             CAM_RMG_HDR_CFG;                                 /* 05A0, 0x1A0045A0, CAM_A_RMG_HDR_CFG */
	CAM_REG_RMG_HDR_GAIN                            CAM_RMG_HDR_GAIN;                                /* 05A4, 0x1A0045A4, CAM_A_RMG_HDR_GAIN */
	CAM_REG_RMG_HDR_CFG2                            CAM_RMG_HDR_CFG2;                                /* 05A8, 0x1A0045A8, CAM_A_RMG_HDR_CFG2 */
	UINT32                                          rsv_05AC[5];                                     /* 05AC..05BF, 0x1A0045AC..1A0045BF */
	CAM_REG_RMM_OSC                                 CAM_RMM_OSC;                                     /* 05C0, 0x1A0045C0, CAM_A_RMM_OSC */
	CAM_REG_RMM_MC                                  CAM_RMM_MC;                                      /* 05C4, 0x1A0045C4, CAM_A_RMM_MC */
	CAM_REG_RMM_REVG_1                              CAM_RMM_REVG_1;                                  /* 05C8, 0x1A0045C8, CAM_A_RMM_REVG_1 */
	CAM_REG_RMM_REVG_2                              CAM_RMM_REVG_2;                                  /* 05CC, 0x1A0045CC, CAM_A_RMM_REVG_2 */
	CAM_REG_RMM_LEOS                                CAM_RMM_LEOS;                                    /* 05D0, 0x1A0045D0, CAM_A_RMM_LEOS */
	CAM_REG_RMM_MC2                                 CAM_RMM_MC2;                                     /* 05D4, 0x1A0045D4, CAM_A_RMM_MC2 */
	CAM_REG_RMM_DIFF_LB                             CAM_RMM_DIFF_LB;                                 /* 05D8, 0x1A0045D8, CAM_A_RMM_DIFF_LB */
	CAM_REG_RMM_MA                                  CAM_RMM_MA;                                      /* 05DC, 0x1A0045DC, CAM_A_RMM_MA */
	CAM_REG_RMM_TUNE                                CAM_RMM_TUNE;                                    /* 05E0, 0x1A0045E0, CAM_A_RMM_TUNE */
	UINT32                                          rsv_05E4[3];                                     /* 05E4..05EF, 0x1A0045E4..1A0045EF */
	CAM_REG_OBC_OFFST0                              CAM_OBC_OFFST0;                                  /* 05F0, 0x1A0045F0, CAM_A_OBC_OFFST0 */
	CAM_REG_OBC_OFFST1                              CAM_OBC_OFFST1;                                  /* 05F4, 0x1A0045F4, CAM_A_OBC_OFFST1 */
	CAM_REG_OBC_OFFST2                              CAM_OBC_OFFST2;                                  /* 05F8, 0x1A0045F8, CAM_A_OBC_OFFST2 */
	CAM_REG_OBC_OFFST3                              CAM_OBC_OFFST3;                                  /* 05FC, 0x1A0045FC, CAM_A_OBC_OFFST3 */
	CAM_REG_OBC_GAIN0                               CAM_OBC_GAIN0;                                   /* 0600, 0x1A004600, CAM_A_OBC_GAIN0 */
	CAM_REG_OBC_GAIN1                               CAM_OBC_GAIN1;                                   /* 0604, 0x1A004604, CAM_A_OBC_GAIN1 */
	CAM_REG_OBC_GAIN2                               CAM_OBC_GAIN2;                                   /* 0608, 0x1A004608, CAM_A_OBC_GAIN2 */
	CAM_REG_OBC_GAIN3                               CAM_OBC_GAIN3;                                   /* 060C, 0x1A00460C, CAM_A_OBC_GAIN3 */
	UINT32                                          rsv_0610[4];                                     /* 0610..061F, 0x1A004610..1A00461F */
	CAM_REG_BNR_BPC_CON                             CAM_BNR_BPC_CON;                                 /* 0620, 0x1A004620, CAM_A_BNR_BPC_CON */
	CAM_REG_BNR_BPC_TH1                             CAM_BNR_BPC_TH1;                                 /* 0624, 0x1A004624, CAM_A_BNR_BPC_TH1 */
	CAM_REG_BNR_BPC_TH2                             CAM_BNR_BPC_TH2;                                 /* 0628, 0x1A004628, CAM_A_BNR_BPC_TH2 */
	CAM_REG_BNR_BPC_TH3                             CAM_BNR_BPC_TH3;                                 /* 062C, 0x1A00462C, CAM_A_BNR_BPC_TH3 */
	CAM_REG_BNR_BPC_TH4                             CAM_BNR_BPC_TH4;                                 /* 0630, 0x1A004630, CAM_A_BNR_BPC_TH4 */
	CAM_REG_BNR_BPC_DTC                             CAM_BNR_BPC_DTC;                                 /* 0634, 0x1A004634, CAM_A_BNR_BPC_DTC */
	CAM_REG_BNR_BPC_COR                             CAM_BNR_BPC_COR;                                 /* 0638, 0x1A004638, CAM_A_BNR_BPC_COR */
	CAM_REG_BNR_BPC_TBLI1                           CAM_BNR_BPC_TBLI1;                               /* 063C, 0x1A00463C, CAM_A_BNR_BPC_TBLI1 */
	CAM_REG_BNR_BPC_TBLI2                           CAM_BNR_BPC_TBLI2;                               /* 0640, 0x1A004640, CAM_A_BNR_BPC_TBLI2 */
	CAM_REG_BNR_BPC_TH1_C                           CAM_BNR_BPC_TH1_C;                               /* 0644, 0x1A004644, CAM_A_BNR_BPC_TH1_C */
	CAM_REG_BNR_BPC_TH2_C                           CAM_BNR_BPC_TH2_C;                               /* 0648, 0x1A004648, CAM_A_BNR_BPC_TH2_C */
	CAM_REG_BNR_BPC_TH3_C                           CAM_BNR_BPC_TH3_C;                               /* 064C, 0x1A00464C, CAM_A_BNR_BPC_TH3_C */
	CAM_REG_BNR_NR1_CON                             CAM_BNR_NR1_CON;                                 /* 0650, 0x1A004650, CAM_A_BNR_NR1_CON */
	CAM_REG_BNR_NR1_CT_CON                          CAM_BNR_NR1_CT_CON;                              /* 0654, 0x1A004654, CAM_A_BNR_NR1_CT_CON */
	CAM_REG_BNR_RSV1                                CAM_BNR_RSV1;                                    /* 0658, 0x1A004658, CAM_A_BNR_RSV1 */
	CAM_REG_BNR_RSV2                                CAM_BNR_RSV2;                                    /* 065C, 0x1A00465C, CAM_A_BNR_RSV2 */
	CAM_REG_BNR_PDC_CON                             CAM_BNR_PDC_CON;                                 /* 0660, 0x1A004660, CAM_A_BNR_PDC_CON */
	CAM_REG_BNR_PDC_GAIN_L0                         CAM_BNR_PDC_GAIN_L0;                             /* 0664, 0x1A004664, CAM_A_BNR_PDC_GAIN_L0 */
	CAM_REG_BNR_PDC_GAIN_L1                         CAM_BNR_PDC_GAIN_L1;                             /* 0668, 0x1A004668, CAM_A_BNR_PDC_GAIN_L1 */
	CAM_REG_BNR_PDC_GAIN_L2                         CAM_BNR_PDC_GAIN_L2;                             /* 066C, 0x1A00466C, CAM_A_BNR_PDC_GAIN_L2 */
	CAM_REG_BNR_PDC_GAIN_L3                         CAM_BNR_PDC_GAIN_L3;                             /* 0670, 0x1A004670, CAM_A_BNR_PDC_GAIN_L3 */
	CAM_REG_BNR_PDC_GAIN_L4                         CAM_BNR_PDC_GAIN_L4;                             /* 0674, 0x1A004674, CAM_A_BNR_PDC_GAIN_L4 */
	CAM_REG_BNR_PDC_GAIN_R0                         CAM_BNR_PDC_GAIN_R0;                             /* 0678, 0x1A004678, CAM_A_BNR_PDC_GAIN_R0 */
	CAM_REG_BNR_PDC_GAIN_R1                         CAM_BNR_PDC_GAIN_R1;                             /* 067C, 0x1A00467C, CAM_A_BNR_PDC_GAIN_R1 */
	CAM_REG_BNR_PDC_GAIN_R2                         CAM_BNR_PDC_GAIN_R2;                             /* 0680, 0x1A004680, CAM_A_BNR_PDC_GAIN_R2 */
	CAM_REG_BNR_PDC_GAIN_R3                         CAM_BNR_PDC_GAIN_R3;                             /* 0684, 0x1A004684, CAM_A_BNR_PDC_GAIN_R3 */
	CAM_REG_BNR_PDC_GAIN_R4                         CAM_BNR_PDC_GAIN_R4;                             /* 0688, 0x1A004688, CAM_A_BNR_PDC_GAIN_R4 */
	CAM_REG_BNR_PDC_TH_GB                           CAM_BNR_PDC_TH_GB;                               /* 068C, 0x1A00468C, CAM_A_BNR_PDC_TH_GB */
	CAM_REG_BNR_PDC_TH_IA                           CAM_BNR_PDC_TH_IA;                               /* 0690, 0x1A004690, CAM_A_BNR_PDC_TH_IA */
	CAM_REG_BNR_PDC_TH_HD                           CAM_BNR_PDC_TH_HD;                               /* 0694, 0x1A004694, CAM_A_BNR_PDC_TH_HD */
	CAM_REG_BNR_PDC_SL                              CAM_BNR_PDC_SL;                                  /* 0698, 0x1A004698, CAM_A_BNR_PDC_SL */
	CAM_REG_BNR_PDC_POS                             CAM_BNR_PDC_POS;                                 /* 069C, 0x1A00469C, CAM_A_BNR_PDC_POS */
	CAM_REG_STM_CFG0                                CAM_STM_CFG0;                                    /* 06A0, 0x1A0046A0, CAM_A_STM_CFG0 */
	CAM_REG_STM_CFG1                                CAM_STM_CFG1;                                    /* 06A4, 0x1A0046A4, CAM_A_STM_CFG1 */
	UINT32                                          rsv_06A8[2];                                     /* 06A8..06AF, 0x1A0046A8..1A0046AF */
	CAM_REG_SCM_CFG0                                CAM_SCM_CFG0;                                    /* 06B0, 0x1A0046B0, CAM_A_SCM_CFG0 */
	CAM_REG_SCM_CFG1                                CAM_SCM_CFG1;                                    /* 06B4, 0x1A0046B4, CAM_A_SCM_CFG1 */
	UINT32                                          rsv_06B8[2];                                     /* 06B8..06BF, 0x1A0046B8..1A0046BF */
	CAM_REG_RPG_SATU_1                              CAM_RPG_SATU_1;                                  /* 06C0, 0x1A0046C0, CAM_A_RPG_SATU_1 */
	CAM_REG_RPG_SATU_2                              CAM_RPG_SATU_2;                                  /* 06C4, 0x1A0046C4, CAM_A_RPG_SATU_2 */
	CAM_REG_RPG_GAIN_1                              CAM_RPG_GAIN_1;                                  /* 06C8, 0x1A0046C8, CAM_A_RPG_GAIN_1 */
	CAM_REG_RPG_GAIN_2                              CAM_RPG_GAIN_2;                                  /* 06CC, 0x1A0046CC, CAM_A_RPG_GAIN_2 */
	CAM_REG_RPG_OFST_1                              CAM_RPG_OFST_1;                                  /* 06D0, 0x1A0046D0, CAM_A_RPG_OFST_1 */
	CAM_REG_RPG_OFST_2                              CAM_RPG_OFST_2;                                  /* 06D4, 0x1A0046D4, CAM_A_RPG_OFST_2 */
	UINT32                                          rsv_06D8[2];                                     /* 06D8..06DF, 0x1A0046D8..1A0046DF */
	CAM_REG_RRZ_CTL                                 CAM_RRZ_CTL;                                     /* 06E0, 0x1A0046E0, CAM_A_RRZ_CTL */
	CAM_REG_RRZ_IN_IMG                              CAM_RRZ_IN_IMG;                                  /* 06E4, 0x1A0046E4, CAM_A_RRZ_IN_IMG */
	CAM_REG_RRZ_OUT_IMG                             CAM_RRZ_OUT_IMG;                                 /* 06E8, 0x1A0046E8, CAM_A_RRZ_OUT_IMG */
	CAM_REG_RRZ_HORI_STEP                           CAM_RRZ_HORI_STEP;                               /* 06EC, 0x1A0046EC, CAM_A_RRZ_HORI_STEP */
	CAM_REG_RRZ_VERT_STEP                           CAM_RRZ_VERT_STEP;                               /* 06F0, 0x1A0046F0, CAM_A_RRZ_VERT_STEP */
	CAM_REG_RRZ_HORI_INT_OFST                       CAM_RRZ_HORI_INT_OFST;                           /* 06F4, 0x1A0046F4, CAM_A_RRZ_HORI_INT_OFST */
	CAM_REG_RRZ_HORI_SUB_OFST                       CAM_RRZ_HORI_SUB_OFST;                           /* 06F8, 0x1A0046F8, CAM_A_RRZ_HORI_SUB_OFST */
	CAM_REG_RRZ_VERT_INT_OFST                       CAM_RRZ_VERT_INT_OFST;                           /* 06FC, 0x1A0046FC, CAM_A_RRZ_VERT_INT_OFST */
	CAM_REG_RRZ_VERT_SUB_OFST                       CAM_RRZ_VERT_SUB_OFST;                           /* 0700, 0x1A004700, CAM_A_RRZ_VERT_SUB_OFST */
	CAM_REG_RRZ_MODE_TH                             CAM_RRZ_MODE_TH;                                 /* 0704, 0x1A004704, CAM_A_RRZ_MODE_TH */
	CAM_REG_RRZ_MODE_CTL                            CAM_RRZ_MODE_CTL;                                /* 0708, 0x1A004708, CAM_A_RRZ_MODE_CTL */
	CAM_REG_RRZ_RLB_AOFST                           CAM_RRZ_RLB_AOFST;                               /* 070C, 0x1A00470C, CAM_A_RRZ_RLB_AOFST */
	UINT32                                          rsv_0710[12];                                    /* 0710..073F, 0x1A004710..1A00473F */
	CAM_REG_RMX_CTL                                 CAM_RMX_CTL;                                     /* 0740, 0x1A004740, CAM_A_RMX_CTL */
	CAM_REG_RMX_CROP                                CAM_RMX_CROP;                                    /* 0744, 0x1A004744, CAM_A_RMX_CROP */
	CAM_REG_RMX_VSIZE                               CAM_RMX_VSIZE;                                   /* 0748, 0x1A004748, CAM_A_RMX_VSIZE */
	UINT32                                          rsv_074C[5];                                     /* 074C..075F, 0x1A00474C..1A00475F */
	CAM_REG_SGG5_PGN                                CAM_SGG5_PGN;                                    /* 0760, 0x1A004760, CAM_A_SGG5_PGN */
	CAM_REG_SGG5_GMRC_1                             CAM_SGG5_GMRC_1;                                 /* 0764, 0x1A004764, CAM_A_SGG5_GMRC_1 */
	CAM_REG_SGG5_GMRC_2                             CAM_SGG5_GMRC_2;                                 /* 0768, 0x1A004768, CAM_A_SGG5_GMRC_2 */
	UINT32                                          rsv_076C[5];                                     /* 076C..077F, 0x1A00476C..1A00477F */
	CAM_REG_BMX_CTL                                 CAM_BMX_CTL;                                     /* 0780, 0x1A004780, CAM_A_BMX_CTL */
	CAM_REG_BMX_CROP                                CAM_BMX_CROP;                                    /* 0784, 0x1A004784, CAM_A_BMX_CROP */
	CAM_REG_BMX_VSIZE                               CAM_BMX_VSIZE;                                   /* 0788, 0x1A004788, CAM_A_BMX_VSIZE */
	UINT32                                          rsv_078C[13];                                    /* 078C..07BF, 0x1A00478C..1A0047BF */
	CAM_REG_UFE_CON                                 CAM_UFE_CON;                                     /* 07C0, 0x1A0047C0, CAM_A_UFE_CON */
	UINT32                                          rsv_07C4[3];                                     /* 07C4..07CF, 0x1A0047C4..1A0047CF */
	CAM_REG_LSC_CTL1                                CAM_LSC_CTL1;                                    /* 07D0, 0x1A0047D0, CAM_A_LSC_CTL1 */
	CAM_REG_LSC_CTL2                                CAM_LSC_CTL2;                                    /* 07D4, 0x1A0047D4, CAM_A_LSC_CTL2 */
	CAM_REG_LSC_CTL3                                CAM_LSC_CTL3;                                    /* 07D8, 0x1A0047D8, CAM_A_LSC_CTL3 */
	CAM_REG_LSC_LBLOCK                              CAM_LSC_LBLOCK;                                  /* 07DC, 0x1A0047DC, CAM_A_LSC_LBLOCK */
	CAM_REG_LSC_RATIO_0                             CAM_LSC_RATIO_0;                                 /* 07E0, 0x1A0047E0, CAM_A_LSC_RATIO_0 */
	CAM_REG_LSC_TPIPE_OFST                          CAM_LSC_TPIPE_OFST;                              /* 07E4, 0x1A0047E4, CAM_A_LSC_TPIPE_OFST */
	CAM_REG_LSC_TPIPE_SIZE                          CAM_LSC_TPIPE_SIZE;                              /* 07E8, 0x1A0047E8, CAM_A_LSC_TPIPE_SIZE */
	CAM_REG_LSC_GAIN_TH                             CAM_LSC_GAIN_TH;                                 /* 07EC, 0x1A0047EC, CAM_A_LSC_GAIN_TH */
	CAM_REG_LSC_RATIO_1                             CAM_LSC_RATIO_1;                                 /* 07F0, 0x1A0047F0, CAM_A_LSC_RATIO_1 */
	UINT32                                          rsv_07F4[3];                                     /* 07F4..07FF, 0x1A0047F4..1A0047FF */
	CAM_REG_AF_CON                                  CAM_AF_CON;                                      /* 0800, 0x1A004800, CAM_A_AF_CON */
	CAM_REG_AF_TH_0                                 CAM_AF_TH_0;                                     /* 0804, 0x1A004804, CAM_A_AF_TH_0 */
	CAM_REG_AF_TH_1                                 CAM_AF_TH_1;                                     /* 0808, 0x1A004808, CAM_A_AF_TH_1 */
	CAM_REG_AF_FLT_1                                CAM_AF_FLT_1;                                    /* 080C, 0x1A00480C, CAM_A_AF_FLT_1 */
	CAM_REG_AF_FLT_2                                CAM_AF_FLT_2;                                    /* 0810, 0x1A004810, CAM_A_AF_FLT_2 */
	CAM_REG_AF_FLT_3                                CAM_AF_FLT_3;                                    /* 0814, 0x1A004814, CAM_A_AF_FLT_3 */
	CAM_REG_AF_FLT_4                                CAM_AF_FLT_4;                                    /* 0818, 0x1A004818, CAM_A_AF_FLT_4 */
	CAM_REG_AF_FLT_5                                CAM_AF_FLT_5;                                    /* 081C, 0x1A00481C, CAM_A_AF_FLT_5 */
	CAM_REG_AF_FLT_6                                CAM_AF_FLT_6;                                    /* 0820, 0x1A004820, CAM_A_AF_FLT_6 */
	CAM_REG_AF_FLT_7                                CAM_AF_FLT_7;                                    /* 0824, 0x1A004824, CAM_A_AF_FLT_7 */
	CAM_REG_AF_FLT_8                                CAM_AF_FLT_8;                                    /* 0828, 0x1A004828, CAM_A_AF_FLT_8 */
	UINT32                                          rsv_082C;                                        /* 082C, 0x1A00482C */
	CAM_REG_AF_SIZE                                 CAM_AF_SIZE;                                     /* 0830, 0x1A004830, CAM_A_AF_SIZE */
	CAM_REG_AF_VLD                                  CAM_AF_VLD;                                      /* 0834, 0x1A004834, CAM_A_AF_VLD */
	CAM_REG_AF_BLK_0                                CAM_AF_BLK_0;                                    /* 0838, 0x1A004838, CAM_A_AF_BLK_0 */
	CAM_REG_AF_BLK_1                                CAM_AF_BLK_1;                                    /* 083C, 0x1A00483C, CAM_A_AF_BLK_1 */
	CAM_REG_AF_TH_2                                 CAM_AF_TH_2;                                     /* 0840, 0x1A004840, CAM_A_AF_TH_2 */
	CAM_REG_AF_BLK_2                                CAM_AF_BLK_2;                                    /* 0844, 0x1A004844, CAM_A_AF_BLK_2 */
	UINT32                                          rsv_0848[2];                                     /* 0848..084F, 0x1A004848..1A00484F */
	CAM_REG_HLR_CFG                                 CAM_HLR_CFG;                                     /* 0850, 0x1A004850, CAM_A_HLR_CFG */
	UINT32                                          rsv_0854;                                        /* 0854, 0x1A004854 */
	CAM_REG_HLR_GAIN                                CAM_HLR_GAIN;                                    /* 0858, 0x1A004858, CAM_A_HLR_GAIN */
	CAM_REG_HLR_GAIN_1                              CAM_HLR_GAIN_1;                                  /* 085C, 0x1A00485C, CAM_A_HLR_GAIN_1 */
	CAM_REG_HLR_OFST                                CAM_HLR_OFST;                                    /* 0860, 0x1A004860, CAM_A_HLR_OFST */
	CAM_REG_HLR_OFST_1                              CAM_HLR_OFST_1;                                  /* 0864, 0x1A004864, CAM_A_HLR_OFST_1 */
	CAM_REG_HLR_IVGN                                CAM_HLR_IVGN;                                    /* 0868, 0x1A004868, CAM_A_HLR_IVGN */
	CAM_REG_HLR_IVGN_1                              CAM_HLR_IVGN_1;                                  /* 086C, 0x1A00486C, CAM_A_HLR_IVGN_1 */
	CAM_REG_HLR_KC                                  CAM_HLR_KC;                                      /* 0870, 0x1A004870, CAM_A_HLR_KC */
	CAM_REG_HLR_CFG_1                               CAM_HLR_CFG_1;                                   /* 0874, 0x1A004874, CAM_A_HLR_CFG_1 */
	CAM_REG_HLR_SL_PARA                             CAM_HLR_SL_PARA;                                 /* 0878, 0x1A004878, CAM_A_HLR_SL_PARA */
	CAM_REG_HLR_SL_PARA_1                           CAM_HLR_SL_PARA_1;                               /* 087C, 0x1A00487C, CAM_A_HLR_SL_PARA_1 */
	CAM_REG_LCS25_CON                               CAM_LCS25_CON;                                   /* 0880, 0x1A004880, CAM_A_LCS25_CON */
	CAM_REG_LCS25_ST                                CAM_LCS25_ST;                                    /* 0884, 0x1A004884, CAM_A_LCS25_ST */
	CAM_REG_LCS25_AWS                               CAM_LCS25_AWS;                                   /* 0888, 0x1A004888, CAM_A_LCS25_AWS */
	CAM_REG_LCS25_FLR                               CAM_LCS25_FLR;                                   /* 088C, 0x1A00488C, CAM_A_LCS25_FLR */
	CAM_REG_LCS25_LRZR_1                            CAM_LCS25_LRZR_1;                                /* 0890, 0x1A004890, CAM_A_LCS25_LRZR_1 */
	CAM_REG_LCS25_LRZR_2                            CAM_LCS25_LRZR_2;                                /* 0894, 0x1A004894, CAM_A_LCS25_LRZR_2 */
	CAM_REG_LCS25_SATU_1                            CAM_LCS25_SATU_1;                                /* 0898, 0x1A004898, CAM_A_LCS25_SATU_1 */
	CAM_REG_LCS25_SATU_2                            CAM_LCS25_SATU_2;                                /* 089C, 0x1A00489C, CAM_A_LCS25_SATU_2 */
	CAM_REG_LCS25_GAIN_1                            CAM_LCS25_GAIN_1;                                /* 08A0, 0x1A0048A0, CAM_A_LCS25_GAIN_1 */
	CAM_REG_LCS25_GAIN_2                            CAM_LCS25_GAIN_2;                                /* 08A4, 0x1A0048A4, CAM_A_LCS25_GAIN_2 */
	CAM_REG_LCS25_OFST_1                            CAM_LCS25_OFST_1;                                /* 08A8, 0x1A0048A8, CAM_A_LCS25_OFST_1 */
	CAM_REG_LCS25_OFST_2                            CAM_LCS25_OFST_2;                                /* 08AC, 0x1A0048AC, CAM_A_LCS25_OFST_2 */
	CAM_REG_LCS25_G2G_CNV_1                         CAM_LCS25_G2G_CNV_1;                             /* 08B0, 0x1A0048B0, CAM_A_LCS25_G2G_CNV_1 */
	CAM_REG_LCS25_G2G_CNV_2                         CAM_LCS25_G2G_CNV_2;                             /* 08B4, 0x1A0048B4, CAM_A_LCS25_G2G_CNV_2 */
	CAM_REG_LCS25_G2G_CNV_3                         CAM_LCS25_G2G_CNV_3;                             /* 08B8, 0x1A0048B8, CAM_A_LCS25_G2G_CNV_3 */
	CAM_REG_LCS25_G2G_CNV_4                         CAM_LCS25_G2G_CNV_4;                             /* 08BC, 0x1A0048BC, CAM_A_LCS25_G2G_CNV_4 */
	CAM_REG_LCS25_G2G_CNV_5                         CAM_LCS25_G2G_CNV_5;                             /* 08C0, 0x1A0048C0, CAM_A_LCS25_G2G_CNV_5 */
	CAM_REG_LCS25_LPF                               CAM_LCS25_LPF;                                   /* 08C4, 0x1A0048C4, CAM_A_LCS25_LPF */
	UINT32                                          rsv_08C8[10];                                    /* 08C8..08EF, 0x1A0048C8..1A0048EF */
	CAM_REG_RCP_CROP_CON1                           CAM_RCP_CROP_CON1;                               /* 08F0, 0x1A0048F0, CAM_A_RCP_CROP_CON1 */
	CAM_REG_RCP_CROP_CON2                           CAM_RCP_CROP_CON2;                               /* 08F4, 0x1A0048F4, CAM_A_RCP_CROP_CON2 */
	UINT32                                          rsv_08F8[2];                                     /* 08F8..08FF, 0x1A0048F8..1A0048FF */
	CAM_REG_SGG1_PGN                                CAM_SGG1_PGN;                                    /* 0900, 0x1A004900, CAM_A_SGG1_PGN */
	CAM_REG_SGG1_GMRC_1                             CAM_SGG1_GMRC_1;                                 /* 0904, 0x1A004904, CAM_A_SGG1_GMRC_1 */
	CAM_REG_SGG1_GMRC_2                             CAM_SGG1_GMRC_2;                                 /* 0908, 0x1A004908, CAM_A_SGG1_GMRC_2 */
	UINT32                                          rsv_090C;                                        /* 090C, 0x1A00490C */
	CAM_REG_QBN2_MODE                               CAM_QBN2_MODE;                                   /* 0910, 0x1A004910, CAM_A_QBN2_MODE */
	UINT32                                          rsv_0914[3];                                     /* 0914..091F, 0x1A004914..1A00491F */
	CAM_REG_AWB_WIN_ORG                             CAM_AWB_WIN_ORG;                                 /* 0920, 0x1A004920, CAM_A_AWB_WIN_ORG */
	CAM_REG_AWB_WIN_SIZE                            CAM_AWB_WIN_SIZE;                                /* 0924, 0x1A004924, CAM_A_AWB_WIN_SIZE */
	CAM_REG_AWB_WIN_PIT                             CAM_AWB_WIN_PIT;                                 /* 0928, 0x1A004928, CAM_A_AWB_WIN_PIT */
	CAM_REG_AWB_WIN_NUM                             CAM_AWB_WIN_NUM;                                 /* 092C, 0x1A00492C, CAM_A_AWB_WIN_NUM */
	CAM_REG_AWB_GAIN1_0                             CAM_AWB_GAIN1_0;                                 /* 0930, 0x1A004930, CAM_A_AWB_GAIN1_0 */
	CAM_REG_AWB_GAIN1_1                             CAM_AWB_GAIN1_1;                                 /* 0934, 0x1A004934, CAM_A_AWB_GAIN1_1 */
	CAM_REG_AWB_LMT1_0                              CAM_AWB_LMT1_0;                                  /* 0938, 0x1A004938, CAM_A_AWB_LMT1_0 */
	CAM_REG_AWB_LMT1_1                              CAM_AWB_LMT1_1;                                  /* 093C, 0x1A00493C, CAM_A_AWB_LMT1_1 */
	CAM_REG_AWB_LOW_THR                             CAM_AWB_LOW_THR;                                 /* 0940, 0x1A004940, CAM_A_AWB_LOW_THR */
	CAM_REG_AWB_HI_THR                              CAM_AWB_HI_THR;                                  /* 0944, 0x1A004944, CAM_A_AWB_HI_THR */
	CAM_REG_AWB_PIXEL_CNT0                          CAM_AWB_PIXEL_CNT0;                              /* 0948, 0x1A004948, CAM_A_AWB_PIXEL_CNT0 */
	CAM_REG_AWB_PIXEL_CNT1                          CAM_AWB_PIXEL_CNT1;                              /* 094C, 0x1A00494C, CAM_A_AWB_PIXEL_CNT1 */
	CAM_REG_AWB_PIXEL_CNT2                          CAM_AWB_PIXEL_CNT2;                              /* 0950, 0x1A004950, CAM_A_AWB_PIXEL_CNT2 */
	CAM_REG_AWB_ERR_THR                             CAM_AWB_ERR_THR;                                 /* 0954, 0x1A004954, CAM_A_AWB_ERR_THR */
	CAM_REG_AWB_ROT                                 CAM_AWB_ROT;                                     /* 0958, 0x1A004958, CAM_A_AWB_ROT */
	CAM_REG_AWB_L0_X                                CAM_AWB_L0_X;                                    /* 095C, 0x1A00495C, CAM_A_AWB_L0_X */
	CAM_REG_AWB_L0_Y                                CAM_AWB_L0_Y;                                    /* 0960, 0x1A004960, CAM_A_AWB_L0_Y */
	CAM_REG_AWB_L1_X                                CAM_AWB_L1_X;                                    /* 0964, 0x1A004964, CAM_A_AWB_L1_X */
	CAM_REG_AWB_L1_Y                                CAM_AWB_L1_Y;                                    /* 0968, 0x1A004968, CAM_A_AWB_L1_Y */
	CAM_REG_AWB_L2_X                                CAM_AWB_L2_X;                                    /* 096C, 0x1A00496C, CAM_A_AWB_L2_X */
	CAM_REG_AWB_L2_Y                                CAM_AWB_L2_Y;                                    /* 0970, 0x1A004970, CAM_A_AWB_L2_Y */
	CAM_REG_AWB_L3_X                                CAM_AWB_L3_X;                                    /* 0974, 0x1A004974, CAM_A_AWB_L3_X */
	CAM_REG_AWB_L3_Y                                CAM_AWB_L3_Y;                                    /* 0978, 0x1A004978, CAM_A_AWB_L3_Y */
	CAM_REG_AWB_L4_X                                CAM_AWB_L4_X;                                    /* 097C, 0x1A00497C, CAM_A_AWB_L4_X */
	CAM_REG_AWB_L4_Y                                CAM_AWB_L4_Y;                                    /* 0980, 0x1A004980, CAM_A_AWB_L4_Y */
	CAM_REG_AWB_L5_X                                CAM_AWB_L5_X;                                    /* 0984, 0x1A004984, CAM_A_AWB_L5_X */
	CAM_REG_AWB_L5_Y                                CAM_AWB_L5_Y;                                    /* 0988, 0x1A004988, CAM_A_AWB_L5_Y */
	CAM_REG_AWB_L6_X                                CAM_AWB_L6_X;                                    /* 098C, 0x1A00498C, CAM_A_AWB_L6_X */
	CAM_REG_AWB_L6_Y                                CAM_AWB_L6_Y;                                    /* 0990, 0x1A004990, CAM_A_AWB_L6_Y */
	CAM_REG_AWB_L7_X                                CAM_AWB_L7_X;                                    /* 0994, 0x1A004994, CAM_A_AWB_L7_X */
	CAM_REG_AWB_L7_Y                                CAM_AWB_L7_Y;                                    /* 0998, 0x1A004998, CAM_A_AWB_L7_Y */
	CAM_REG_AWB_L8_X                                CAM_AWB_L8_X;                                    /* 099C, 0x1A00499C, CAM_A_AWB_L8_X */
	CAM_REG_AWB_L8_Y                                CAM_AWB_L8_Y;                                    /* 09A0, 0x1A0049A0, CAM_A_AWB_L8_Y */
	CAM_REG_AWB_L9_X                                CAM_AWB_L9_X;                                    /* 09A4, 0x1A0049A4, CAM_A_AWB_L9_X */
	CAM_REG_AWB_L9_Y                                CAM_AWB_L9_Y;                                    /* 09A8, 0x1A0049A8, CAM_A_AWB_L9_Y */
	CAM_REG_AWB_SPARE                               CAM_AWB_SPARE;                                   /* 09AC, 0x1A0049AC, CAM_A_AWB_SPARE */
	CAM_REG_AWB_MOTION_THR                          CAM_AWB_MOTION_THR;                              /* 09B0, 0x1A0049B0, CAM_A_AWB_MOTION_THR */
	CAM_REG_AWB_RC_CNV_0                            CAM_AWB_RC_CNV_0;                                /* 09B4, 0x1A0049B4, CAM_A_AWB_RC_CNV_0 */
	CAM_REG_AWB_RC_CNV_1                            CAM_AWB_RC_CNV_1;                                /* 09B8, 0x1A0049B8, CAM_A_AWB_RC_CNV_1 */
	CAM_REG_AWB_RC_CNV_2                            CAM_AWB_RC_CNV_2;                                /* 09BC, 0x1A0049BC, CAM_A_AWB_RC_CNV_2 */
	CAM_REG_AWB_RC_CNV_3                            CAM_AWB_RC_CNV_3;                                /* 09C0, 0x1A0049C0, CAM_A_AWB_RC_CNV_3 */
	CAM_REG_AWB_RC_CNV_4                            CAM_AWB_RC_CNV_4;                                /* 09C4, 0x1A0049C4, CAM_A_AWB_RC_CNV_4 */
	UINT32                                          rsv_09C8[6];                                     /* 09C8..09DF, 0x1A0049C8..1A0049DF */
	CAM_REG_AE_GAIN2_0                              CAM_AE_GAIN2_0;                                  /* 09E0, 0x1A0049E0, CAM_A_AE_GAIN2_0 */
	CAM_REG_AE_GAIN2_1                              CAM_AE_GAIN2_1;                                  /* 09E4, 0x1A0049E4, CAM_A_AE_GAIN2_1 */
	CAM_REG_AE_LMT2_0                               CAM_AE_LMT2_0;                                   /* 09E8, 0x1A0049E8, CAM_A_AE_LMT2_0 */
	CAM_REG_AE_LMT2_1                               CAM_AE_LMT2_1;                                   /* 09EC, 0x1A0049EC, CAM_A_AE_LMT2_1 */
	CAM_REG_AE_RC_CNV_0                             CAM_AE_RC_CNV_0;                                 /* 09F0, 0x1A0049F0, CAM_A_AE_RC_CNV_0 */
	CAM_REG_AE_RC_CNV_1                             CAM_AE_RC_CNV_1;                                 /* 09F4, 0x1A0049F4, CAM_A_AE_RC_CNV_1 */
	CAM_REG_AE_RC_CNV_2                             CAM_AE_RC_CNV_2;                                 /* 09F8, 0x1A0049F8, CAM_A_AE_RC_CNV_2 */
	CAM_REG_AE_RC_CNV_3                             CAM_AE_RC_CNV_3;                                 /* 09FC, 0x1A0049FC, CAM_A_AE_RC_CNV_3 */
	CAM_REG_AE_RC_CNV_4                             CAM_AE_RC_CNV_4;                                 /* 0A00, 0x1A004A00, CAM_A_AE_RC_CNV_4 */
	CAM_REG_AE_YGAMMA_0                             CAM_AE_YGAMMA_0;                                 /* 0A04, 0x1A004A04, CAM_A_AE_YGAMMA_0 */
	CAM_REG_AE_YGAMMA_1                             CAM_AE_YGAMMA_1;                                 /* 0A08, 0x1A004A08, CAM_A_AE_YGAMMA_1 */
	CAM_REG_AE_OVER_EXPO_CFG                        CAM_AE_OVER_EXPO_CFG;                            /* 0A0C, 0x1A004A0C, CAM_A_AE_OVER_EXPO_CFG */
	CAM_REG_AE_PIX_HST_CTL                          CAM_AE_PIX_HST_CTL;                              /* 0A10, 0x1A004A10, CAM_A_AE_PIX_HST_CTL */
	CAM_REG_AE_PIX_HST_SET                          CAM_AE_PIX_HST_SET;                              /* 0A14, 0x1A004A14, CAM_A_AE_PIX_HST_SET */
	UINT32                                          rsv_0A18;                                        /* 0A18, 0x1A004A18 */
	CAM_REG_AE_PIX_HST0_YRNG                        CAM_AE_PIX_HST0_YRNG;                            /* 0A1C, 0x1A004A1C, CAM_A_AE_PIX_HST0_YRNG */
	CAM_REG_AE_PIX_HST0_XRNG                        CAM_AE_PIX_HST0_XRNG;                            /* 0A20, 0x1A004A20, CAM_A_AE_PIX_HST0_XRNG */
	CAM_REG_AE_PIX_HST1_YRNG                        CAM_AE_PIX_HST1_YRNG;                            /* 0A24, 0x1A004A24, CAM_A_AE_PIX_HST1_YRNG */
	CAM_REG_AE_PIX_HST1_XRNG                        CAM_AE_PIX_HST1_XRNG;                            /* 0A28, 0x1A004A28, CAM_A_AE_PIX_HST1_XRNG */
	CAM_REG_AE_PIX_HST2_YRNG                        CAM_AE_PIX_HST2_YRNG;                            /* 0A2C, 0x1A004A2C, CAM_A_AE_PIX_HST2_YRNG */
	CAM_REG_AE_PIX_HST2_XRNG                        CAM_AE_PIX_HST2_XRNG;                            /* 0A30, 0x1A004A30, CAM_A_AE_PIX_HST2_XRNG */
	CAM_REG_AE_PIX_HST3_YRNG                        CAM_AE_PIX_HST3_YRNG;                            /* 0A34, 0x1A004A34, CAM_A_AE_PIX_HST3_YRNG */
	CAM_REG_AE_PIX_HST3_XRNG                        CAM_AE_PIX_HST3_XRNG;                            /* 0A38, 0x1A004A38, CAM_A_AE_PIX_HST3_XRNG */
	CAM_REG_AE_STAT_EN                              CAM_AE_STAT_EN;                                  /* 0A3C, 0x1A004A3C, CAM_A_AE_STAT_EN */
	CAM_REG_AE_YCOEF                                CAM_AE_YCOEF;                                    /* 0A40, 0x1A004A40, CAM_A_AE_YCOEF */
	CAM_REG_AE_CCU_HST_END_Y                        CAM_AE_CCU_HST_END_Y;                            /* 0A44, 0x1A004A44, CAM_A_AE_CCU_HST_END_Y */
	CAM_REG_AE_SPARE                                CAM_AE_SPARE;                                    /* 0A48, 0x1A004A48, CAM_A_AE_SPARE */
	UINT32                                          rsv_0A4C[29];                                    /* 0A4C..0ABF, 0x1A004A4C..1A004ABF */
	CAM_REG_QBN1_MODE                               CAM_QBN1_MODE;                                   /* 0AC0, 0x1A004AC0, CAM_A_QBN1_MODE */
	UINT32                                          rsv_0AC4[3];                                     /* 0AC4..0ACF, 0x1A004AC4..1A004ACF */
	CAM_REG_CPG_SATU_1                              CAM_CPG_SATU_1;                                  /* 0AD0, 0x1A004AD0, CAM_A_CPG_SATU_1 */
	CAM_REG_CPG_SATU_2                              CAM_CPG_SATU_2;                                  /* 0AD4, 0x1A004AD4, CAM_A_CPG_SATU_2 */
	CAM_REG_CPG_GAIN_1                              CAM_CPG_GAIN_1;                                  /* 0AD8, 0x1A004AD8, CAM_A_CPG_GAIN_1 */
	CAM_REG_CPG_GAIN_2                              CAM_CPG_GAIN_2;                                  /* 0ADC, 0x1A004ADC, CAM_A_CPG_GAIN_2 */
	CAM_REG_CPG_OFST_1                              CAM_CPG_OFST_1;                                  /* 0AE0, 0x1A004AE0, CAM_A_CPG_OFST_1 */
	CAM_REG_CPG_OFST_2                              CAM_CPG_OFST_2;                                  /* 0AE4, 0x1A004AE4, CAM_A_CPG_OFST_2 */
	UINT32                                          rsv_0AE8[2];                                     /* 0AE8..0AEF, 0x1A004AE8..1A004AEF */
	CAM_REG_CAC_TILE_SIZE                           CAM_CAC_TILE_SIZE;                               /* 0AF0, 0x1A004AF0, CAM_A_CAC_TILE_SIZE */
	CAM_REG_CAC_TILE_OFFSET                         CAM_CAC_TILE_OFFSET;                             /* 0AF4, 0x1A004AF4, CAM_A_CAC_TILE_OFFSET */
	UINT32                                          rsv_0AF8[2];                                     /* 0AF8..0AFF, 0x1A004AF8..1A004AFF */
	CAM_REG_PMX_CTL                                 CAM_PMX_CTL;                                     /* 0B00, 0x1A004B00, CAM_A_PMX_CTL */
	CAM_REG_PMX_CROP                                CAM_PMX_CROP;                                    /* 0B04, 0x1A004B04, CAM_A_PMX_CROP */
	CAM_REG_PMX_VSIZE                               CAM_PMX_VSIZE;                                   /* 0B08, 0x1A004B08, CAM_A_PMX_VSIZE */
	UINT32                                          rsv_0B0C[13];                                    /* 0B0C..0B3F, 0x1A004B0C..1A004B3F */
	CAM_REG_VBN_GAIN                                CAM_VBN_GAIN;                                    /* 0B40, 0x1A004B40, CAM_A_VBN_GAIN */
	CAM_REG_VBN_OFST                                CAM_VBN_OFST;                                    /* 0B44, 0x1A004B44, CAM_A_VBN_OFST */
	CAM_REG_VBN_TYPE                                CAM_VBN_TYPE;                                    /* 0B48, 0x1A004B48, CAM_A_VBN_TYPE */
	CAM_REG_VBN_SPARE                               CAM_VBN_SPARE;                                   /* 0B4C, 0x1A004B4C, CAM_A_VBN_SPARE */
	UINT32                                          rsv_0B50[4];                                     /* 0B50..0B5F, 0x1A004B50..1A004B5F */
	CAM_REG_AMX_CTL                                 CAM_AMX_CTL;                                     /* 0B60, 0x1A004B60, CAM_A_AMX_CTL */
	CAM_REG_AMX_CROP                                CAM_AMX_CROP;                                    /* 0B64, 0x1A004B64, CAM_A_AMX_CROP */
	CAM_REG_AMX_VSIZE                               CAM_AMX_VSIZE;                                   /* 0B68, 0x1A004B68, CAM_A_AMX_VSIZE */
	UINT32                                          rsv_0B6C[5];                                     /* 0B6C..0B7F, 0x1A004B6C..1A004B7F */
	CAM_REG_BIN_CTL                                 CAM_BIN_CTL;                                     /* 0B80, 0x1A004B80, CAM_A_BIN_CTL */
	CAM_REG_BIN_FTH                                 CAM_BIN_FTH;                                     /* 0B84, 0x1A004B84, CAM_A_BIN_FTH */
	CAM_REG_BIN_SPARE                               CAM_BIN_SPARE;                                   /* 0B88, 0x1A004B88, CAM_A_BIN_SPARE */
	UINT32                                          rsv_0B8C[5];                                     /* 0B8C..0B9F, 0x1A004B8C..1A004B9F */
	CAM_REG_DBN_GAIN                                CAM_DBN_GAIN;                                    /* 0BA0, 0x1A004BA0, CAM_A_DBN_GAIN */
	CAM_REG_DBN_OFST                                CAM_DBN_OFST;                                    /* 0BA4, 0x1A004BA4, CAM_A_DBN_OFST */
	CAM_REG_DBN_SPARE                               CAM_DBN_SPARE;                                   /* 0BA8, 0x1A004BA8, CAM_A_DBN_SPARE */
	UINT32                                          rsv_0BAC;                                        /* 0BAC, 0x1A004BAC */
	CAM_REG_PBN_TYPE                                CAM_PBN_TYPE;                                    /* 0BB0, 0x1A004BB0, CAM_A_PBN_TYPE */
	CAM_REG_PBN_LST                                 CAM_PBN_LST;                                     /* 0BB4, 0x1A004BB4, CAM_A_PBN_LST */
	CAM_REG_PBN_VSIZE                               CAM_PBN_VSIZE;                                   /* 0BB8, 0x1A004BB8, CAM_A_PBN_VSIZE */
	UINT32                                          rsv_0BBC;                                        /* 0BBC, 0x1A004BBC */
	CAM_REG_RCP3_CROP_CON1                          CAM_RCP3_CROP_CON1;                              /* 0BC0, 0x1A004BC0, CAM_A_RCP3_CROP_CON1 */
	CAM_REG_RCP3_CROP_CON2                          CAM_RCP3_CROP_CON2;                              /* 0BC4, 0x1A004BC4, CAM_A_RCP3_CROP_CON2 */
	UINT32                                          rsv_0BC8[2];                                     /* 0BC8..0BCF, 0x1A004BC8..1A004BCF */
	CAM_REG_SGM_R_OFST_TABLE0_3                     CAM_SGM_R_OFST_TABLE0_3;                         /* 0BD0, 0x1A004BD0, CAM_A_SGM_R_OFST_TABLE0_3 */
	CAM_REG_SGM_R_OFST_TABLE4_7                     CAM_SGM_R_OFST_TABLE4_7;                         /* 0BD4, 0x1A004BD4, CAM_A_SGM_R_OFST_TABLE4_7 */
	CAM_REG_SGM_R_OFST_TABLE8_11                    CAM_SGM_R_OFST_TABLE8_11;                        /* 0BD8, 0x1A004BD8, CAM_A_SGM_R_OFST_TABLE8_11 */
	CAM_REG_SGM_R_OFST_TABLE12_15                   CAM_SGM_R_OFST_TABLE12_15;                       /* 0BDC, 0x1A004BDC, CAM_A_SGM_R_OFST_TABLE12_15 */
	CAM_REG_SGM_G_OFST_TABLE0_3                     CAM_SGM_G_OFST_TABLE0_3;                         /* 0BE0, 0x1A004BE0, CAM_A_SGM_G_OFST_TABLE0_3 */
	CAM_REG_SGM_G_OFST_TABLE4_7                     CAM_SGM_G_OFST_TABLE4_7;                         /* 0BE4, 0x1A004BE4, CAM_A_SGM_G_OFST_TABLE4_7 */
	CAM_REG_SGM_G_OFST_TABLE8_11                    CAM_SGM_G_OFST_TABLE8_11;                        /* 0BE8, 0x1A004BE8, CAM_A_SGM_G_OFST_TABLE8_11 */
	CAM_REG_SGM_G_OFST_TABLE12_15                   CAM_SGM_G_OFST_TABLE12_15;                       /* 0BEC, 0x1A004BEC, CAM_A_SGM_G_OFST_TABLE12_15 */
	CAM_REG_SGM_B_OFST_TABLE0_3                     CAM_SGM_B_OFST_TABLE0_3;                         /* 0BF0, 0x1A004BF0, CAM_A_SGM_B_OFST_TABLE0_3 */
	CAM_REG_SGM_B_OFST_TABLE4_7                     CAM_SGM_B_OFST_TABLE4_7;                         /* 0BF4, 0x1A004BF4, CAM_A_SGM_B_OFST_TABLE4_7 */
	CAM_REG_SGM_B_OFST_TABLE8_11                    CAM_SGM_B_OFST_TABLE8_11;                        /* 0BF8, 0x1A004BF8, CAM_A_SGM_B_OFST_TABLE8_11 */
	CAM_REG_SGM_B_OFST_TABLE12_15                   CAM_SGM_B_OFST_TABLE12_15;                       /* 0BFC, 0x1A004BFC, CAM_A_SGM_B_OFST_TABLE12_15 */
	CAM_REG_DBS_SIGMA                               CAM_DBS_SIGMA;                                   /* 0C00, 0x1A004C00, CAM_A_DBS_SIGMA */
	CAM_REG_DBS_BSTBL_0                             CAM_DBS_BSTBL_0;                                 /* 0C04, 0x1A004C04, CAM_A_DBS_BSTBL_0 */
	CAM_REG_DBS_BSTBL_1                             CAM_DBS_BSTBL_1;                                 /* 0C08, 0x1A004C08, CAM_A_DBS_BSTBL_1 */
	CAM_REG_DBS_BSTBL_2                             CAM_DBS_BSTBL_2;                                 /* 0C0C, 0x1A004C0C, CAM_A_DBS_BSTBL_2 */
	CAM_REG_DBS_BSTBL_3                             CAM_DBS_BSTBL_3;                                 /* 0C10, 0x1A004C10, CAM_A_DBS_BSTBL_3 */
	CAM_REG_DBS_CTL                                 CAM_DBS_CTL;                                     /* 0C14, 0x1A004C14, CAM_A_DBS_CTL */
	CAM_REG_DBS_CTL_2                               CAM_DBS_CTL_2;                                   /* 0C18, 0x1A004C18, CAM_A_DBS_CTL_2 */
	CAM_REG_DBS_SIGMA_2                             CAM_DBS_SIGMA_2;                                 /* 0C1C, 0x1A004C1C, CAM_A_DBS_SIGMA_2 */
	CAM_REG_DBS_YGN                                 CAM_DBS_YGN;                                     /* 0C20, 0x1A004C20, CAM_A_DBS_YGN */
	CAM_REG_DBS_SL_Y12                              CAM_DBS_SL_Y12;                                  /* 0C24, 0x1A004C24, CAM_A_DBS_SL_Y12 */
	CAM_REG_DBS_SL_Y34                              CAM_DBS_SL_Y34;                                  /* 0C28, 0x1A004C28, CAM_A_DBS_SL_Y34 */
	CAM_REG_DBS_SL_G12                              CAM_DBS_SL_G12;                                  /* 0C2C, 0x1A004C2C, CAM_A_DBS_SL_G12 */
	CAM_REG_DBS_SL_G34                              CAM_DBS_SL_G34;                                  /* 0C30, 0x1A004C30, CAM_A_DBS_SL_G34 */
	UINT32                                          rsv_0C34[3];                                     /* 0C34..0C3F, 0x1A004C34..1A004C3F */
	CAM_REG_SL2F_CEN                                CAM_SL2F_CEN;                                    /* 0C40, 0x1A004C40, CAM_A_SL2F_CEN */
	CAM_REG_SL2F_RR_CON0                            CAM_SL2F_RR_CON0;                                /* 0C44, 0x1A004C44, CAM_A_SL2F_RR_CON0 */
	CAM_REG_SL2F_RR_CON1                            CAM_SL2F_RR_CON1;                                /* 0C48, 0x1A004C48, CAM_A_SL2F_RR_CON1 */
	CAM_REG_SL2F_GAIN                               CAM_SL2F_GAIN;                                   /* 0C4C, 0x1A004C4C, CAM_A_SL2F_GAIN */
	CAM_REG_SL2F_RZ                                 CAM_SL2F_RZ;                                     /* 0C50, 0x1A004C50, CAM_A_SL2F_RZ */
	CAM_REG_SL2F_XOFF                               CAM_SL2F_XOFF;                                   /* 0C54, 0x1A004C54, CAM_A_SL2F_XOFF */
	CAM_REG_SL2F_YOFF                               CAM_SL2F_YOFF;                                   /* 0C58, 0x1A004C58, CAM_A_SL2F_YOFF */
	CAM_REG_SL2F_SLP_CON0                           CAM_SL2F_SLP_CON0;                               /* 0C5C, 0x1A004C5C, CAM_A_SL2F_SLP_CON0 */
	CAM_REG_SL2F_SLP_CON1                           CAM_SL2F_SLP_CON1;                               /* 0C60, 0x1A004C60, CAM_A_SL2F_SLP_CON1 */
	CAM_REG_SL2F_SLP_CON2                           CAM_SL2F_SLP_CON2;                               /* 0C64, 0x1A004C64, CAM_A_SL2F_SLP_CON2 */
	CAM_REG_SL2F_SLP_CON3                           CAM_SL2F_SLP_CON3;                               /* 0C68, 0x1A004C68, CAM_A_SL2F_SLP_CON3 */
	CAM_REG_SL2F_SIZE                               CAM_SL2F_SIZE;                                   /* 0C6C, 0x1A004C6C, CAM_A_SL2F_SIZE */
	UINT32                                          rsv_0C70[4];                                     /* 0C70..0C7F, 0x1A004C70..1A004C7F */
	CAM_REG_SL2J_CEN                                CAM_SL2J_CEN;                                    /* 0C80, 0x1A004C80, CAM_A_SL2J_CEN */
	CAM_REG_SL2J_RR_CON0                            CAM_SL2J_RR_CON0;                                /* 0C84, 0x1A004C84, CAM_A_SL2J_RR_CON0 */
	CAM_REG_SL2J_RR_CON1                            CAM_SL2J_RR_CON1;                                /* 0C88, 0x1A004C88, CAM_A_SL2J_RR_CON1 */
	CAM_REG_SL2J_GAIN                               CAM_SL2J_GAIN;                                   /* 0C8C, 0x1A004C8C, CAM_A_SL2J_GAIN */
	CAM_REG_SL2J_RZ                                 CAM_SL2J_RZ;                                     /* 0C90, 0x1A004C90, CAM_A_SL2J_RZ */
	CAM_REG_SL2J_XOFF                               CAM_SL2J_XOFF;                                   /* 0C94, 0x1A004C94, CAM_A_SL2J_XOFF */
	CAM_REG_SL2J_YOFF                               CAM_SL2J_YOFF;                                   /* 0C98, 0x1A004C98, CAM_A_SL2J_YOFF */
	CAM_REG_SL2J_SLP_CON0                           CAM_SL2J_SLP_CON0;                               /* 0C9C, 0x1A004C9C, CAM_A_SL2J_SLP_CON0 */
	CAM_REG_SL2J_SLP_CON1                           CAM_SL2J_SLP_CON1;                               /* 0CA0, 0x1A004CA0, CAM_A_SL2J_SLP_CON1 */
	CAM_REG_SL2J_SLP_CON2                           CAM_SL2J_SLP_CON2;                               /* 0CA4, 0x1A004CA4, CAM_A_SL2J_SLP_CON2 */
	CAM_REG_SL2J_SLP_CON3                           CAM_SL2J_SLP_CON3;                               /* 0CA8, 0x1A004CA8, CAM_A_SL2J_SLP_CON3 */
	CAM_REG_SL2J_SIZE                               CAM_SL2J_SIZE;                                   /* 0CAC, 0x1A004CAC, CAM_A_SL2J_SIZE */
	UINT32                                          rsv_0CB0[4];                                     /* 0CB0..0CBF, 0x1A004CB0..1A004CBF */
	CAM_REG_PCP_CROP_CON1                           CAM_PCP_CROP_CON1;                               /* 0CC0, 0x1A004CC0, CAM_A_PCP_CROP_CON1 */
	CAM_REG_PCP_CROP_CON2                           CAM_PCP_CROP_CON2;                               /* 0CC4, 0x1A004CC4, CAM_A_PCP_CROP_CON2 */
	UINT32                                          rsv_0CC8[2];                                     /* 0CC8..0CCF, 0x1A004CC8..1A004CCF */
	CAM_REG_SGG2_PGN                                CAM_SGG2_PGN;                                    /* 0CD0, 0x1A004CD0, CAM_A_SGG2_PGN */
	CAM_REG_SGG2_GMRC_1                             CAM_SGG2_GMRC_1;                                 /* 0CD4, 0x1A004CD4, CAM_A_SGG2_GMRC_1 */
	CAM_REG_SGG2_GMRC_2                             CAM_SGG2_GMRC_2;                                 /* 0CD8, 0x1A004CD8, CAM_A_SGG2_GMRC_2 */
	UINT32                                          rsv_0CDC;                                        /* 0CDC, 0x1A004CDC */
	CAM_REG_PSB_CON                                 CAM_PSB_CON;                                     /* 0CE0, 0x1A004CE0, CAM_A_PSB_CON */
	CAM_REG_PSB_SIZE                                CAM_PSB_SIZE;                                    /* 0CE4, 0x1A004CE4, CAM_A_PSB_SIZE */
	UINT32                                          rsv_0CE8[2];                                     /* 0CE8..0CEF, 0x1A004CE8..1A004CEF */
	CAM_REG_PDE_TBLI1                               CAM_PDE_TBLI1;                                   /* 0CF0, 0x1A004CF0, CAM_A_PDE_TBLI1 */
	UINT32                                          rsv_0CF4[3];                                     /* 0CF4..0CFF, 0x1A004CF4..1A004CFF */
	CAM_REG_QBN4_MODE                               CAM_QBN4_MODE;                                   /* 0D00, 0x1A004D00, CAM_A_QBN4_MODE */
	UINT32                                          rsv_0D04[3];                                     /* 0D04..0D0F, 0x1A004D04..1A004D0F */
	CAM_REG_PS_AWB_WIN_ORG                          CAM_PS_AWB_WIN_ORG;                              /* 0D10, 0x1A004D10, CAM_A_PS_AWB_WIN_ORG */
	CAM_REG_PS_AWB_WIN_SIZE                         CAM_PS_AWB_WIN_SIZE;                             /* 0D14, 0x1A004D14, CAM_A_PS_AWB_WIN_SIZE */
	CAM_REG_PS_AWB_WIN_PIT                          CAM_PS_AWB_WIN_PIT;                              /* 0D18, 0x1A004D18, CAM_A_PS_AWB_WIN_PIT */
	CAM_REG_PS_AWB_WIN_NUM                          CAM_PS_AWB_WIN_NUM;                              /* 0D1C, 0x1A004D1C, CAM_A_PS_AWB_WIN_NUM */
	CAM_REG_PS_AWB_PIXEL_CNT0                       CAM_PS_AWB_PIXEL_CNT0;                           /* 0D20, 0x1A004D20, CAM_A_PS_AWB_PIXEL_CNT0 */
	CAM_REG_PS_AWB_PIXEL_CNT1                       CAM_PS_AWB_PIXEL_CNT1;                           /* 0D24, 0x1A004D24, CAM_A_PS_AWB_PIXEL_CNT1 */
	CAM_REG_PS_AWB_PIXEL_CNT2                       CAM_PS_AWB_PIXEL_CNT2;                           /* 0D28, 0x1A004D28, CAM_A_PS_AWB_PIXEL_CNT2 */
	CAM_REG_PS_AWB_PIXEL_CNT3                       CAM_PS_AWB_PIXEL_CNT3;                           /* 0D2C, 0x1A004D2C, CAM_A_PS_AWB_PIXEL_CNT3 */
	CAM_REG_PS_AE_YCOEF0                            CAM_PS_AE_YCOEF0;                                /* 0D30, 0x1A004D30, CAM_A_PS_AE_YCOEF0 */
	CAM_REG_PS_AE_YCOEF1                            CAM_PS_AE_YCOEF1;                                /* 0D34, 0x1A004D34, CAM_A_PS_AE_YCOEF1 */
	UINT32                                          rsv_0D38[6];                                     /* 0D38..0D4F, 0x1A004D38..1A004D4F */
	CAM_REG_PDI_BASE_ADDR                           CAM_PDI_BASE_ADDR;                               /* 0D50, 0x1A004D50, CAM_A_PDI_BASE_ADDR */
	CAM_REG_PDI_OFST_ADDR                           CAM_PDI_OFST_ADDR;                               /* 0D54, 0x1A004D54, CAM_A_PDI_OFST_ADDR */
	CAM_REG_PDI_DRS                                 CAM_PDI_DRS;                                     /* 0D58, 0x1A004D58, CAM_A_PDI_DRS */
	CAM_REG_PDI_XSIZE                               CAM_PDI_XSIZE;                                   /* 0D5C, 0x1A004D5C, CAM_A_PDI_XSIZE */
	CAM_REG_PDI_YSIZE                               CAM_PDI_YSIZE;                                   /* 0D60, 0x1A004D60, CAM_A_PDI_YSIZE */
	CAM_REG_PDI_STRIDE                              CAM_PDI_STRIDE;                                  /* 0D64, 0x1A004D64, CAM_A_PDI_STRIDE */
	CAM_REG_PDI_CON                                 CAM_PDI_CON;                                     /* 0D68, 0x1A004D68, CAM_A_PDI_CON */
	CAM_REG_PDI_CON2                                CAM_PDI_CON2;                                    /* 0D6C, 0x1A004D6C, CAM_A_PDI_CON2 */
	CAM_REG_PDI_CON3                                CAM_PDI_CON3;                                    /* 0D70, 0x1A004D70, CAM_A_PDI_CON3 */
	CAM_REG_PDI_CON4                                CAM_PDI_CON4;                                    /* 0D74, 0x1A004D74, CAM_A_PDI_CON4 */
	CAM_REG_PDI_ERR_STAT                            CAM_PDI_ERR_STAT;                                /* 0D78, 0x1A004D78, CAM_A_PDI_ERR_STAT */
	UINT32                                          rsv_0D7C;                                        /* 0D7C, 0x1A004D7C */
	CAM_REG_PSO_BASE_ADDR                           CAM_PSO_BASE_ADDR;                               /* 0D80, 0x1A004D80, CAM_A_PSO_BASE_ADDR */
	UINT32                                          rsv_0D84;                                        /* 0D84, 0x1A004D84 */
	CAM_REG_PSO_OFST_ADDR                           CAM_PSO_OFST_ADDR;                               /* 0D88, 0x1A004D88, CAM_A_PSO_OFST_ADDR */
	CAM_REG_PSO_DRS                                 CAM_PSO_DRS;                                     /* 0D8C, 0x1A004D8C, CAM_A_PSO_DRS */
	CAM_REG_PSO_XSIZE                               CAM_PSO_XSIZE;                                   /* 0D90, 0x1A004D90, CAM_A_PSO_XSIZE */
	CAM_REG_PSO_YSIZE                               CAM_PSO_YSIZE;                                   /* 0D94, 0x1A004D94, CAM_A_PSO_YSIZE */
	CAM_REG_PSO_STRIDE                              CAM_PSO_STRIDE;                                  /* 0D98, 0x1A004D98, CAM_A_PSO_STRIDE */
	CAM_REG_PSO_CON                                 CAM_PSO_CON;                                     /* 0D9C, 0x1A004D9C, CAM_A_PSO_CON */
	CAM_REG_PSO_CON2                                CAM_PSO_CON2;                                    /* 0DA0, 0x1A004DA0, CAM_A_PSO_CON2 */
	CAM_REG_PSO_CON3                                CAM_PSO_CON3;                                    /* 0DA4, 0x1A004DA4, CAM_A_PSO_CON3 */
	CAM_REG_PSO_CON4                                CAM_PSO_CON4;                                    /* 0DA8, 0x1A004DA8, CAM_A_PSO_CON4 */
	CAM_REG_PSO_ERR_STAT                            CAM_PSO_ERR_STAT;                                /* 0DAC, 0x1A004DAC, CAM_A_PSO_ERR_STAT */
	CAM_REG_PSO_FH_SPARE_2                          CAM_PSO_FH_SPARE_2;                              /* 0DB0, 0x1A004DB0, CAM_A_PSO_FH_SPARE_2 */
	CAM_REG_PSO_FH_SPARE_3                          CAM_PSO_FH_SPARE_3;                              /* 0DB4, 0x1A004DB4, CAM_A_PSO_FH_SPARE_3 */
	CAM_REG_PSO_FH_SPARE_4                          CAM_PSO_FH_SPARE_4;                              /* 0DB8, 0x1A004DB8, CAM_A_PSO_FH_SPARE_4 */
	CAM_REG_PSO_FH_SPARE_5                          CAM_PSO_FH_SPARE_5;                              /* 0DBC, 0x1A004DBC, CAM_A_PSO_FH_SPARE_5 */
	CAM_REG_PSO_FH_SPARE_6                          CAM_PSO_FH_SPARE_6;                              /* 0DC0, 0x1A004DC0, CAM_A_PSO_FH_SPARE_6 */
	CAM_REG_PSO_FH_SPARE_7                          CAM_PSO_FH_SPARE_7;                              /* 0DC4, 0x1A004DC4, CAM_A_PSO_FH_SPARE_7 */
	CAM_REG_PSO_FH_SPARE_8                          CAM_PSO_FH_SPARE_8;                              /* 0DC8, 0x1A004DC8, CAM_A_PSO_FH_SPARE_8 */
	CAM_REG_PSO_FH_SPARE_9                          CAM_PSO_FH_SPARE_9;                              /* 0DCC, 0x1A004DCC, CAM_A_PSO_FH_SPARE_9 */
	CAM_REG_PSO_FH_SPARE_10                         CAM_PSO_FH_SPARE_10;                             /* 0DD0, 0x1A004DD0, CAM_A_PSO_FH_SPARE_10 */
	CAM_REG_PSO_FH_SPARE_11                         CAM_PSO_FH_SPARE_11;                             /* 0DD4, 0x1A004DD4, CAM_A_PSO_FH_SPARE_11 */
	CAM_REG_PSO_FH_SPARE_12                         CAM_PSO_FH_SPARE_12;                             /* 0DD8, 0x1A004DD8, CAM_A_PSO_FH_SPARE_12 */
	CAM_REG_PSO_FH_SPARE_13                         CAM_PSO_FH_SPARE_13;                             /* 0DDC, 0x1A004DDC, CAM_A_PSO_FH_SPARE_13 */
	CAM_REG_PSO_FH_SPARE_14                         CAM_PSO_FH_SPARE_14;                             /* 0DE0, 0x1A004DE0, CAM_A_PSO_FH_SPARE_14 */
	CAM_REG_PSO_FH_SPARE_15                         CAM_PSO_FH_SPARE_15;                             /* 0DE4, 0x1A004DE4, CAM_A_PSO_FH_SPARE_15 */
	CAM_REG_PSO_FH_SPARE_16                         CAM_PSO_FH_SPARE_16;                             /* 0DE8, 0x1A004DE8, CAM_A_PSO_FH_SPARE_16 */
	UINT32                                          rsv_0DEC[5];                                     /* 0DEC..0DFF, 0x1A004DEC..1A004DFF */
	CAM_REG_DMA_FRAME_HEADER_EN                     CAM_DMA_FRAME_HEADER_EN;                         /* 0E00, 0x1A004E00, CAM_A_DMA_FRAME_HEADER_EN */
	CAM_REG_IMGO_FH_BASE_ADDR                       CAM_IMGO_FH_BASE_ADDR;                           /* 0E04, 0x1A004E04, CAM_A_IMGO_FH_BASE_ADDR */
	CAM_REG_RRZO_FH_BASE_ADDR                       CAM_RRZO_FH_BASE_ADDR;                           /* 0E08, 0x1A004E08, CAM_A_RRZO_FH_BASE_ADDR */
	CAM_REG_AAO_FH_BASE_ADDR                        CAM_AAO_FH_BASE_ADDR;                            /* 0E0C, 0x1A004E0C, CAM_A_AAO_FH_BASE_ADDR */
	CAM_REG_AFO_FH_BASE_ADDR                        CAM_AFO_FH_BASE_ADDR;                            /* 0E10, 0x1A004E10, CAM_A_AFO_FH_BASE_ADDR */
	CAM_REG_LCSO_FH_BASE_ADDR                       CAM_LCSO_FH_BASE_ADDR;                           /* 0E14, 0x1A004E14, CAM_A_LCSO_FH_BASE_ADDR */
	CAM_REG_UFEO_FH_BASE_ADDR                       CAM_UFEO_FH_BASE_ADDR;                           /* 0E18, 0x1A004E18, CAM_A_UFEO_FH_BASE_ADDR */
	CAM_REG_PDO_FH_BASE_ADDR                        CAM_PDO_FH_BASE_ADDR;                            /* 0E1C, 0x1A004E1C, CAM_A_PDO_FH_BASE_ADDR */
	CAM_REG_PSO_FH_BASE_ADDR                        CAM_PSO_FH_BASE_ADDR;                            /* 0E20, 0x1A004E20, CAM_A_PSO_FH_BASE_ADDR */
	UINT32                                          rsv_0E24[3];                                     /* 0E24..0E2F, 0x1A004E24..1A004E2F */
	CAM_REG_IMGO_FH_SPARE_2                         CAM_IMGO_FH_SPARE_2;                             /* 0E30, 0x1A004E30, CAM_A_IMGO_FH_SPARE_2 */
	CAM_REG_IMGO_FH_SPARE_3                         CAM_IMGO_FH_SPARE_3;                             /* 0E34, 0x1A004E34, CAM_A_IMGO_FH_SPARE_3 */
	CAM_REG_IMGO_FH_SPARE_4                         CAM_IMGO_FH_SPARE_4;                             /* 0E38, 0x1A004E38, CAM_A_IMGO_FH_SPARE_4 */
	CAM_REG_IMGO_FH_SPARE_5                         CAM_IMGO_FH_SPARE_5;                             /* 0E3C, 0x1A004E3C, CAM_A_IMGO_FH_SPARE_5 */
	CAM_REG_IMGO_FH_SPARE_6                         CAM_IMGO_FH_SPARE_6;                             /* 0E40, 0x1A004E40, CAM_A_IMGO_FH_SPARE_6 */
	CAM_REG_IMGO_FH_SPARE_7                         CAM_IMGO_FH_SPARE_7;                             /* 0E44, 0x1A004E44, CAM_A_IMGO_FH_SPARE_7 */
	CAM_REG_IMGO_FH_SPARE_8                         CAM_IMGO_FH_SPARE_8;                             /* 0E48, 0x1A004E48, CAM_A_IMGO_FH_SPARE_8 */
	CAM_REG_IMGO_FH_SPARE_9                         CAM_IMGO_FH_SPARE_9;                             /* 0E4C, 0x1A004E4C, CAM_A_IMGO_FH_SPARE_9 */
	CAM_REG_IMGO_FH_SPARE_10                        CAM_IMGO_FH_SPARE_10;                            /* 0E50, 0x1A004E50, CAM_A_IMGO_FH_SPARE_10 */
	CAM_REG_IMGO_FH_SPARE_11                        CAM_IMGO_FH_SPARE_11;                            /* 0E54, 0x1A004E54, CAM_A_IMGO_FH_SPARE_11 */
	CAM_REG_IMGO_FH_SPARE_12                        CAM_IMGO_FH_SPARE_12;                            /* 0E58, 0x1A004E58, CAM_A_IMGO_FH_SPARE_12 */
	CAM_REG_IMGO_FH_SPARE_13                        CAM_IMGO_FH_SPARE_13;                            /* 0E5C, 0x1A004E5C, CAM_A_IMGO_FH_SPARE_13 */
	CAM_REG_IMGO_FH_SPARE_14                        CAM_IMGO_FH_SPARE_14;                            /* 0E60, 0x1A004E60, CAM_A_IMGO_FH_SPARE_14 */
	CAM_REG_IMGO_FH_SPARE_15                        CAM_IMGO_FH_SPARE_15;                            /* 0E64, 0x1A004E64, CAM_A_IMGO_FH_SPARE_15 */
	CAM_REG_IMGO_FH_SPARE_16                        CAM_IMGO_FH_SPARE_16;                            /* 0E68, 0x1A004E68, CAM_A_IMGO_FH_SPARE_16 */
	UINT32                                          rsv_0E6C;                                        /* 0E6C, 0x1A004E6C */
	CAM_REG_RRZO_FH_SPARE_2                         CAM_RRZO_FH_SPARE_2;                             /* 0E70, 0x1A004E70, CAM_A_RRZO_FH_SPARE_2 */
	CAM_REG_RRZO_FH_SPARE_3                         CAM_RRZO_FH_SPARE_3;                             /* 0E74, 0x1A004E74, CAM_A_RRZO_FH_SPARE_3 */
	CAM_REG_RRZO_FH_SPARE_4                         CAM_RRZO_FH_SPARE_4;                             /* 0E78, 0x1A004E78, CAM_A_RRZO_FH_SPARE_4 */
	CAM_REG_RRZO_FH_SPARE_5                         CAM_RRZO_FH_SPARE_5;                             /* 0E7C, 0x1A004E7C, CAM_A_RRZO_FH_SPARE_5 */
	CAM_REG_RRZO_FH_SPARE_6                         CAM_RRZO_FH_SPARE_6;                             /* 0E80, 0x1A004E80, CAM_A_RRZO_FH_SPARE_6 */
	CAM_REG_RRZO_FH_SPARE_7                         CAM_RRZO_FH_SPARE_7;                             /* 0E84, 0x1A004E84, CAM_A_RRZO_FH_SPARE_7 */
	CAM_REG_RRZO_FH_SPARE_8                         CAM_RRZO_FH_SPARE_8;                             /* 0E88, 0x1A004E88, CAM_A_RRZO_FH_SPARE_8 */
	CAM_REG_RRZO_FH_SPARE_9                         CAM_RRZO_FH_SPARE_9;                             /* 0E8C, 0x1A004E8C, CAM_A_RRZO_FH_SPARE_9 */
	CAM_REG_RRZO_FH_SPARE_10                        CAM_RRZO_FH_SPARE_10;                            /* 0E90, 0x1A004E90, CAM_A_RRZO_FH_SPARE_10 */
	CAM_REG_RRZO_FH_SPARE_11                        CAM_RRZO_FH_SPARE_11;                            /* 0E94, 0x1A004E94, CAM_A_RRZO_FH_SPARE_11 */
	CAM_REG_RRZO_FH_SPARE_12                        CAM_RRZO_FH_SPARE_12;                            /* 0E98, 0x1A004E98, CAM_A_RRZO_FH_SPARE_12 */
	CAM_REG_RRZO_FH_SPARE_13                        CAM_RRZO_FH_SPARE_13;                            /* 0E9C, 0x1A004E9C, CAM_A_RRZO_FH_SPARE_13 */
	CAM_REG_RRZO_FH_SPARE_14                        CAM_RRZO_FH_SPARE_14;                            /* 0EA0, 0x1A004EA0, CAM_A_RRZO_FH_SPARE_14 */
	CAM_REG_RRZO_FH_SPARE_15                        CAM_RRZO_FH_SPARE_15;                            /* 0EA4, 0x1A004EA4, CAM_A_RRZO_FH_SPARE_15 */
	CAM_REG_RRZO_FH_SPARE_16                        CAM_RRZO_FH_SPARE_16;                            /* 0EA8, 0x1A004EA8, CAM_A_RRZO_FH_SPARE_16 */
	UINT32                                          rsv_0EAC;                                        /* 0EAC, 0x1A004EAC */
	CAM_REG_AAO_FH_SPARE_2                          CAM_AAO_FH_SPARE_2;                              /* 0EB0, 0x1A004EB0, CAM_A_AAO_FH_SPARE_2 */
	CAM_REG_AAO_FH_SPARE_3                          CAM_AAO_FH_SPARE_3;                              /* 0EB4, 0x1A004EB4, CAM_A_AAO_FH_SPARE_3 */
	CAM_REG_AAO_FH_SPARE_4                          CAM_AAO_FH_SPARE_4;                              /* 0EB8, 0x1A004EB8, CAM_A_AAO_FH_SPARE_4 */
	CAM_REG_AAO_FH_SPARE_5                          CAM_AAO_FH_SPARE_5;                              /* 0EBC, 0x1A004EBC, CAM_A_AAO_FH_SPARE_5 */
	CAM_REG_AAO_FH_SPARE_6                          CAM_AAO_FH_SPARE_6;                              /* 0EC0, 0x1A004EC0, CAM_A_AAO_FH_SPARE_6 */
	CAM_REG_AAO_FH_SPARE_7                          CAM_AAO_FH_SPARE_7;                              /* 0EC4, 0x1A004EC4, CAM_A_AAO_FH_SPARE_7 */
	CAM_REG_AAO_FH_SPARE_8                          CAM_AAO_FH_SPARE_8;                              /* 0EC8, 0x1A004EC8, CAM_A_AAO_FH_SPARE_8 */
	CAM_REG_AAO_FH_SPARE_9                          CAM_AAO_FH_SPARE_9;                              /* 0ECC, 0x1A004ECC, CAM_A_AAO_FH_SPARE_9 */
	CAM_REG_AAO_FH_SPARE_10                         CAM_AAO_FH_SPARE_10;                             /* 0ED0, 0x1A004ED0, CAM_A_AAO_FH_SPARE_10 */
	CAM_REG_AAO_FH_SPARE_11                         CAM_AAO_FH_SPARE_11;                             /* 0ED4, 0x1A004ED4, CAM_A_AAO_FH_SPARE_11 */
	CAM_REG_AAO_FH_SPARE_12                         CAM_AAO_FH_SPARE_12;                             /* 0ED8, 0x1A004ED8, CAM_A_AAO_FH_SPARE_12 */
	CAM_REG_AAO_FH_SPARE_13                         CAM_AAO_FH_SPARE_13;                             /* 0EDC, 0x1A004EDC, CAM_A_AAO_FH_SPARE_13 */
	CAM_REG_AAO_FH_SPARE_14                         CAM_AAO_FH_SPARE_14;                             /* 0EE0, 0x1A004EE0, CAM_A_AAO_FH_SPARE_14 */
	CAM_REG_AAO_FH_SPARE_15                         CAM_AAO_FH_SPARE_15;                             /* 0EE4, 0x1A004EE4, CAM_A_AAO_FH_SPARE_15 */
	CAM_REG_AAO_FH_SPARE_16                         CAM_AAO_FH_SPARE_16;                             /* 0EE8, 0x1A004EE8, CAM_A_AAO_FH_SPARE_16 */
	UINT32                                          rsv_0EEC;                                        /* 0EEC, 0x1A004EEC */
	CAM_REG_AFO_FH_SPARE_2                          CAM_AFO_FH_SPARE_2;                              /* 0EF0, 0x1A004EF0, CAM_A_AFO_FH_SPARE_2 */
	CAM_REG_AFO_FH_SPARE_3                          CAM_AFO_FH_SPARE_3;                              /* 0EF4, 0x1A004EF4, CAM_A_AFO_FH_SPARE_3 */
	CAM_REG_AFO_FH_SPARE_4                          CAM_AFO_FH_SPARE_4;                              /* 0EF8, 0x1A004EF8, CAM_A_AFO_FH_SPARE_4 */
	CAM_REG_AFO_FH_SPARE_5                          CAM_AFO_FH_SPARE_5;                              /* 0EFC, 0x1A004EFC, CAM_A_AFO_FH_SPARE_5 */
	CAM_REG_AFO_FH_SPARE_6                          CAM_AFO_FH_SPARE_6;                              /* 0F00, 0x1A004F00, CAM_A_AFO_FH_SPARE_6 */
	CAM_REG_AFO_FH_SPARE_7                          CAM_AFO_FH_SPARE_7;                              /* 0F04, 0x1A004F04, CAM_A_AFO_FH_SPARE_7 */
	CAM_REG_AFO_FH_SPARE_8                          CAM_AFO_FH_SPARE_8;                              /* 0F08, 0x1A004F08, CAM_A_AFO_FH_SPARE_8 */
	CAM_REG_AFO_FH_SPARE_9                          CAM_AFO_FH_SPARE_9;                              /* 0F0C, 0x1A004F0C, CAM_A_AFO_FH_SPARE_9 */
	CAM_REG_AFO_FH_SPARE_10                         CAM_AFO_FH_SPARE_10;                             /* 0F10, 0x1A004F10, CAM_A_AFO_FH_SPARE_10 */
	CAM_REG_AFO_FH_SPARE_11                         CAM_AFO_FH_SPARE_11;                             /* 0F14, 0x1A004F14, CAM_A_AFO_FH_SPARE_11 */
	CAM_REG_AFO_FH_SPARE_12                         CAM_AFO_FH_SPARE_12;                             /* 0F18, 0x1A004F18, CAM_A_AFO_FH_SPARE_12 */
	CAM_REG_AFO_FH_SPARE_13                         CAM_AFO_FH_SPARE_13;                             /* 0F1C, 0x1A004F1C, CAM_A_AFO_FH_SPARE_13 */
	CAM_REG_AFO_FH_SPARE_14                         CAM_AFO_FH_SPARE_14;                             /* 0F20, 0x1A004F20, CAM_A_AFO_FH_SPARE_14 */
	CAM_REG_AFO_FH_SPARE_15                         CAM_AFO_FH_SPARE_15;                             /* 0F24, 0x1A004F24, CAM_A_AFO_FH_SPARE_15 */
	CAM_REG_AFO_FH_SPARE_16                         CAM_AFO_FH_SPARE_16;                             /* 0F28, 0x1A004F28, CAM_A_AFO_FH_SPARE_16 */
	UINT32                                          rsv_0F2C;                                        /* 0F2C, 0x1A004F2C */
	CAM_REG_LCSO_FH_SPARE_2                         CAM_LCSO_FH_SPARE_2;                             /* 0F30, 0x1A004F30, CAM_A_LCSO_FH_SPARE_2 */
	CAM_REG_LCSO_FH_SPARE_3                         CAM_LCSO_FH_SPARE_3;                             /* 0F34, 0x1A004F34, CAM_A_LCSO_FH_SPARE_3 */
	CAM_REG_LCSO_FH_SPARE_4                         CAM_LCSO_FH_SPARE_4;                             /* 0F38, 0x1A004F38, CAM_A_LCSO_FH_SPARE_4 */
	CAM_REG_LCSO_FH_SPARE_5                         CAM_LCSO_FH_SPARE_5;                             /* 0F3C, 0x1A004F3C, CAM_A_LCSO_FH_SPARE_5 */
	CAM_REG_LCSO_FH_SPARE_6                         CAM_LCSO_FH_SPARE_6;                             /* 0F40, 0x1A004F40, CAM_A_LCSO_FH_SPARE_6 */
	CAM_REG_LCSO_FH_SPARE_7                         CAM_LCSO_FH_SPARE_7;                             /* 0F44, 0x1A004F44, CAM_A_LCSO_FH_SPARE_7 */
	CAM_REG_LCSO_FH_SPARE_8                         CAM_LCSO_FH_SPARE_8;                             /* 0F48, 0x1A004F48, CAM_A_LCSO_FH_SPARE_8 */
	CAM_REG_LCSO_FH_SPARE_9                         CAM_LCSO_FH_SPARE_9;                             /* 0F4C, 0x1A004F4C, CAM_A_LCSO_FH_SPARE_9 */
	CAM_REG_LCSO_FH_SPARE_10                        CAM_LCSO_FH_SPARE_10;                            /* 0F50, 0x1A004F50, CAM_A_LCSO_FH_SPARE_10 */
	CAM_REG_LCSO_FH_SPARE_11                        CAM_LCSO_FH_SPARE_11;                            /* 0F54, 0x1A004F54, CAM_A_LCSO_FH_SPARE_11 */
	CAM_REG_LCSO_FH_SPARE_12                        CAM_LCSO_FH_SPARE_12;                            /* 0F58, 0x1A004F58, CAM_A_LCSO_FH_SPARE_12 */
	CAM_REG_LCSO_FH_SPARE_13                        CAM_LCSO_FH_SPARE_13;                            /* 0F5C, 0x1A004F5C, CAM_A_LCSO_FH_SPARE_13 */
	CAM_REG_LCSO_FH_SPARE_14                        CAM_LCSO_FH_SPARE_14;                            /* 0F60, 0x1A004F60, CAM_A_LCSO_FH_SPARE_14 */
	CAM_REG_LCSO_FH_SPARE_15                        CAM_LCSO_FH_SPARE_15;                            /* 0F64, 0x1A004F64, CAM_A_LCSO_FH_SPARE_15 */
	CAM_REG_LCSO_FH_SPARE_16                        CAM_LCSO_FH_SPARE_16;                            /* 0F68, 0x1A004F68, CAM_A_LCSO_FH_SPARE_16 */
	UINT32                                          rsv_0F6C;                                        /* 0F6C, 0x1A004F6C */
	CAM_REG_UFEO_FH_SPARE_2                         CAM_UFEO_FH_SPARE_2;                             /* 0F70, 0x1A004F70, CAM_A_UFEO_FH_SPARE_2 */
	CAM_REG_UFEO_FH_SPARE_3                         CAM_UFEO_FH_SPARE_3;                             /* 0F74, 0x1A004F74, CAM_A_UFEO_FH_SPARE_3 */
	CAM_REG_UFEO_FH_SPARE_4                         CAM_UFEO_FH_SPARE_4;                             /* 0F78, 0x1A004F78, CAM_A_UFEO_FH_SPARE_4 */
	CAM_REG_UFEO_FH_SPARE_5                         CAM_UFEO_FH_SPARE_5;                             /* 0F7C, 0x1A004F7C, CAM_A_UFEO_FH_SPARE_5 */
	CAM_REG_UFEO_FH_SPARE_6                         CAM_UFEO_FH_SPARE_6;                             /* 0F80, 0x1A004F80, CAM_A_UFEO_FH_SPARE_6 */
	CAM_REG_UFEO_FH_SPARE_7                         CAM_UFEO_FH_SPARE_7;                             /* 0F84, 0x1A004F84, CAM_A_UFEO_FH_SPARE_7 */
	CAM_REG_UFEO_FH_SPARE_8                         CAM_UFEO_FH_SPARE_8;                             /* 0F88, 0x1A004F88, CAM_A_UFEO_FH_SPARE_8 */
	CAM_REG_UFEO_FH_SPARE_9                         CAM_UFEO_FH_SPARE_9;                             /* 0F8C, 0x1A004F8C, CAM_A_UFEO_FH_SPARE_9 */
	CAM_REG_UFEO_FH_SPARE_10                        CAM_UFEO_FH_SPARE_10;                            /* 0F90, 0x1A004F90, CAM_A_UFEO_FH_SPARE_10 */
	CAM_REG_UFEO_FH_SPARE_11                        CAM_UFEO_FH_SPARE_11;                            /* 0F94, 0x1A004F94, CAM_A_UFEO_FH_SPARE_11 */
	CAM_REG_UFEO_FH_SPARE_12                        CAM_UFEO_FH_SPARE_12;                            /* 0F98, 0x1A004F98, CAM_A_UFEO_FH_SPARE_12 */
	CAM_REG_UFEO_FH_SPARE_13                        CAM_UFEO_FH_SPARE_13;                            /* 0F9C, 0x1A004F9C, CAM_A_UFEO_FH_SPARE_13 */
	CAM_REG_UFEO_FH_SPARE_14                        CAM_UFEO_FH_SPARE_14;                            /* 0FA0, 0x1A004FA0, CAM_A_UFEO_FH_SPARE_14 */
	CAM_REG_UFEO_FH_SPARE_15                        CAM_UFEO_FH_SPARE_15;                            /* 0FA4, 0x1A004FA4, CAM_A_UFEO_FH_SPARE_15 */
	CAM_REG_UFEO_FH_SPARE_16                        CAM_UFEO_FH_SPARE_16;                            /* 0FA8, 0x1A004FA8, CAM_A_UFEO_FH_SPARE_16 */
	UINT32                                          rsv_0FAC;                                        /* 0FAC, 0x1A004FAC */
	CAM_REG_PDO_FH_SPARE_2                          CAM_PDO_FH_SPARE_2;                              /* 0FB0, 0x1A004FB0, CAM_A_PDO_FH_SPARE_2 */
	CAM_REG_PDO_FH_SPARE_3                          CAM_PDO_FH_SPARE_3;                              /* 0FB4, 0x1A004FB4, CAM_A_PDO_FH_SPARE_3 */
	CAM_REG_PDO_FH_SPARE_4                          CAM_PDO_FH_SPARE_4;                              /* 0FB8, 0x1A004FB8, CAM_A_PDO_FH_SPARE_4 */
	CAM_REG_PDO_FH_SPARE_5                          CAM_PDO_FH_SPARE_5;                              /* 0FBC, 0x1A004FBC, CAM_A_PDO_FH_SPARE_5 */
	CAM_REG_PDO_FH_SPARE_6                          CAM_PDO_FH_SPARE_6;                              /* 0FC0, 0x1A004FC0, CAM_A_PDO_FH_SPARE_6 */
	CAM_REG_PDO_FH_SPARE_7                          CAM_PDO_FH_SPARE_7;                              /* 0FC4, 0x1A004FC4, CAM_A_PDO_FH_SPARE_7 */
	CAM_REG_PDO_FH_SPARE_8                          CAM_PDO_FH_SPARE_8;                              /* 0FC8, 0x1A004FC8, CAM_A_PDO_FH_SPARE_8 */
	CAM_REG_PDO_FH_SPARE_9                          CAM_PDO_FH_SPARE_9;                              /* 0FCC, 0x1A004FCC, CAM_A_PDO_FH_SPARE_9 */
	CAM_REG_PDO_FH_SPARE_10                         CAM_PDO_FH_SPARE_10;                             /* 0FD0, 0x1A004FD0, CAM_A_PDO_FH_SPARE_10 */
	CAM_REG_PDO_FH_SPARE_11                         CAM_PDO_FH_SPARE_11;                             /* 0FD4, 0x1A004FD4, CAM_A_PDO_FH_SPARE_11 */
	CAM_REG_PDO_FH_SPARE_12                         CAM_PDO_FH_SPARE_12;                             /* 0FD8, 0x1A004FD8, CAM_A_PDO_FH_SPARE_12 */
	CAM_REG_PDO_FH_SPARE_13                         CAM_PDO_FH_SPARE_13;                             /* 0FDC, 0x1A004FDC, CAM_A_PDO_FH_SPARE_13 */
	CAM_REG_PDO_FH_SPARE_14                         CAM_PDO_FH_SPARE_14;                             /* 0FE0, 0x1A004FE0, CAM_A_PDO_FH_SPARE_14 */
	CAM_REG_PDO_FH_SPARE_15                         CAM_PDO_FH_SPARE_15;                             /* 0FE4, 0x1A004FE4, CAM_A_PDO_FH_SPARE_15 */
	CAM_REG_PDO_FH_SPARE_16                         CAM_PDO_FH_SPARE_16;                             /* 0FE8, 0x1A004FE8, CAM_A_PDO_FH_SPARE_16 */
	UINT32                                          rsv_0FEC[5];                                     /* 0FEC..0FFF, 1A004FEC..1A004FFF */
	//below para is over 0x1000, for CQ baseaddr only.
	//in order to reduce memory space, because of inner register is at  + 0x8000, a special flag will be set when over 0x1000.
	MUINT32                                         CAM_CQ_THRE0_ADDR_INNER;                        // should map to 0x4168 + 0x8000,  1000
	MUINT32                                         dummy0[5];
	MUINT32                                         CAM_CQ_THRE1_ADDR_INNER;                        // should map to 0x4174 + 0x8000,  1018
	MUINT32                                         dummy1[2];
	MUINT32                                         CAM_CQ_THRE2_ADDR_INNER;                        // should map to 0x4180 + 0x8000,  1024
	MUINT32                                         dummy2[2];
	MUINT32                                         CAM_CQ_THRE3_ADDR_INNER;                        // should map to 0x418C + 0x8000,  1030
	MUINT32                                         dummy3[2];
	MUINT32                                         CAM_CQ_THRE4_ADDR_INNER;                        // should map to 0x4198 + 0x8000,  103C
	MUINT32                                         dummy4[2];
	MUINT32                                         CAM_CQ_THRE5_ADDR_INNER;                        // should map to 0x41A4 + 0x8000,  1048
	MUINT32                                         dummy5[2];
	MUINT32                                         CAM_CQ_THRE6_ADDR_INNER;                        // should map to 0x41B0 + 0x8000,  1054
	MUINT32                                         dummy6[2];
	MUINT32                                         CAM_CQ_THRE7_ADDR_INNER;                        // should map to 0x41BC + 0x8000,  1060
	MUINT32                                         dummy7[2];
	MUINT32                                         CAM_CQ_THRE8_ADDR_INNER;                        // should map to 0x41C8 + 0x8000,  106C
	MUINT32                                         dummy8[2];
	MUINT32                                         CAM_CQ_THRE9_ADDR_INNER;                        // should map to 0x41D4 + 0x8000,  1078
	MUINT32                                         dummy9[2];
	MUINT32                                         CAM_CQ_THRE10_ADDR_INNER;                       // should map to 0x41E0 + 0x8000,  1084
	MUINT32                                         dummy10[2];
	MUINT32                                         CAM_CQ_THRE11_ADDR_INNER;                       // should map to 0x41EC + 0x8000,  1090         /* 0DEC..0DFF, 1A004DEC..1A004DFF */
	MUINT32                                         dummy11[2];
	MUINT32                                         CAM_CQ_THRE12_ADDR_INNER;                       // should map to 0x41F8 + 0x8000,  109C         /* 0DEC..0DFF, 1A004DEC..1A004DFF */
    MUINT32                                         TWIN_CQ_THRE0_ADDR;                             // should map to 0x4168 or 0x5168,  10a0         this is for master cam's cq to set slave cam's cq-base-addr
}cam_reg_t;

typedef volatile struct _camsv_reg_t_	/* 0x1A050000..0x1A050E6F */
{
	CAMSV_REG_TOP_DEBUG                             CAMSV_TOP_DEBUG;                                 /* 0000, 0x1A050000, CAMSV_0_TOP_DEBUG */
	UINT32                                          rsv_0004[3];                                     /* 0004..000F, 0x1A050004..1A05000F */
	CAMSV_REG_MODULE_EN                             CAMSV_MODULE_EN;                                 /* 0010, 0x1A050010, CAMSV_0_MODULE_EN */
	CAMSV_REG_FMT_SEL                               CAMSV_FMT_SEL;                                   /* 0014, 0x1A050014, CAMSV_0_FMT_SEL */
	CAMSV_REG_INT_EN                                CAMSV_INT_EN;                                    /* 0018, 0x1A050018, CAMSV_0_INT_EN */
	CAMSV_REG_INT_STATUS                            CAMSV_INT_STATUS;                                /* 001C, 0x1A05001C, CAMSV_0_INT_STATUS */
	CAMSV_REG_SW_CTL                                CAMSV_SW_CTL;                                    /* 0020, 0x1A050020, CAMSV_0_SW_CTL */
	CAMSV_REG_SPARE0                                CAMSV_SPARE0;                                    /* 0024, 0x1A050024, CAMSV_0_SPARE0 */
	CAMSV_REG_SPARE1                                CAMSV_SPARE1;                                    /* 0028, 0x1A050028, CAMSV_0_SPARE1 */
	CAMSV_REG_IMGO_FBC                              CAMSV_IMGO_FBC;                                  /* 002C, 0x1A05002C, CAMSV_0_IMGO_FBC */
	CAMSV_REG_CLK_EN                                CAMSV_CLK_EN;                                    /* 0030, 0x1A050030, CAMSV_0_CLK_EN */
	CAMSV_REG_DBG_SET                               CAMSV_DBG_SET;                                   /* 0034, 0x1A050034, CAMSV_0_DBG_SET */
	CAMSV_REG_DBG_PORT                              CAMSV_DBG_PORT;                                  /* 0038, 0x1A050038, CAMSV_0_DBG_PORT */
	CAMSV_REG_DATE_CODE                             CAMSV_DATE_CODE;                                 /* 003C, 0x1A05003C, CAMSV_0_DATE_CODE */
	CAMSV_REG_PROJ_CODE                             CAMSV_PROJ_CODE;                                 /* 0040, 0x1A050040, CAMSV_0_PROJ_CODE */
	CAMSV_REG_DCM_DIS                               CAMSV_DCM_DIS;                                   /* 0044, 0x1A050044, CAMSV_0_DCM_DIS */
	CAMSV_REG_DCM_STATUS                            CAMSV_DCM_STATUS;                                /* 0048, 0x1A050048, CAMSV_0_DCM_STATUS */
	CAMSV_REG_PAK                                   CAMSV_PAK;                                       /* 004C, 0x1A05004C, CAMSV_0_PAK */
	UINT32                                          rsv_0050[48];                                    /* 0050..010F, 0x1A050050..1A05010F */
	CAMSV_REG_FBC_IMGO_CTL1                         CAMSV_FBC_IMGO_CTL1;                             /* 0110, 0x1A050110, CAMSV_0_FBC_IMGO_CTL1 */
	CAMSV_REG_FBC_IMGO_CTL2                         CAMSV_FBC_IMGO_CTL2;                             /* 0114, 0x1A050114, CAMSV_0_FBC_IMGO_CTL2 */
	CAMSV_REG_FBC_IMGO_ENQ_ADDR                     CAMSV_FBC_IMGO_ENQ_ADDR;                         /* 0118, 0x1A050118, CAMSV_0_FBC_IMGO_ENQ_ADDR */
	CAMSV_REG_FBC_IMGO_CUR_ADDR                     CAMSV_FBC_IMGO_CUR_ADDR;                         /* 011C, 0x1A05011C, CAMSV_0_FBC_IMGO_CUR_ADDR */
	UINT32                                          rsv_0120[56];                                    /* 0120..01FF, 0x1A050120..1A0501FF */
	CAMSV_REG_DMA_SOFT_RSTSTAT                      CAMSV_DMA_SOFT_RSTSTAT;                          /* 0200, 0x1A050200, CAMSV_0_DMA_SOFT_RSTSTAT */
	CAMSV_REG_CQ0I_BASE_ADDR                        CAMSV_CQ0I_BASE_ADDR;                            /* 0204, 0x1A050204, CAMSV_0_CQ0I_BASE_ADDR */
	CAMSV_REG_CQ0I_XSIZE                            CAMSV_CQ0I_XSIZE;                                /* 0208, 0x1A050208, CAMSV_0_CQ0I_XSIZE */
	CAMSV_REG_VERTICAL_FLIP_EN                      CAMSV_VERTICAL_FLIP_EN;                          /* 020C, 0x1A05020C, CAMSV_0_VERTICAL_FLIP_EN */
	CAMSV_REG_DMA_SOFT_RESET                        CAMSV_DMA_SOFT_RESET;                            /* 0210, 0x1A050210, CAMSV_0_DMA_SOFT_RESET */
	CAMSV_REG_LAST_ULTRA_EN                         CAMSV_LAST_ULTRA_EN;                             /* 0214, 0x1A050214, CAMSV_0_LAST_ULTRA_EN */
	CAMSV_REG_SPECIAL_FUN_EN                        CAMSV_SPECIAL_FUN_EN;                            /* 0218, 0x1A050218, CAMSV_0_SPECIAL_FUN_EN */
	UINT32                                          rsv_021C;                                        /* 021C, 0x1A05021C */
	CAMSV_REG_IMGO_BASE_ADDR                        CAMSV_IMGO_BASE_ADDR;                            /* 0220, 0x1A050220, CAMSV_0_IMGO_BASE_ADDR */
	UINT32                                          rsv_0224;                                        /* 0224, 0x1A050224 */
	CAMSV_REG_IMGO_OFST_ADDR                        CAMSV_IMGO_OFST_ADDR;                            /* 0228, 0x1A050228, CAMSV_0_IMGO_OFST_ADDR */
	CAMSV_REG_IMGO_DRS                              CAMSV_IMGO_DRS;                                  /* 022C, 0x1A05022C, CAMSV_0_IMGO_DRS */
	CAMSV_REG_IMGO_XSIZE                            CAMSV_IMGO_XSIZE;                                /* 0230, 0x1A050230, CAMSV_0_IMGO_XSIZE */
	CAMSV_REG_IMGO_YSIZE                            CAMSV_IMGO_YSIZE;                                /* 0234, 0x1A050234, CAMSV_0_IMGO_YSIZE */
	CAMSV_REG_IMGO_STRIDE                           CAMSV_IMGO_STRIDE;                               /* 0238, 0x1A050238, CAMSV_0_IMGO_STRIDE */
	CAMSV_REG_IMGO_CON                              CAMSV_IMGO_CON;                                  /* 023C, 0x1A05023C, CAMSV_0_IMGO_CON */
	CAMSV_REG_IMGO_CON2                             CAMSV_IMGO_CON2;                                 /* 0240, 0x1A050240, CAMSV_0_IMGO_CON2 */
	CAMSV_REG_IMGO_CON3                             CAMSV_IMGO_CON3;                                 /* 0244, 0x1A050244, CAMSV_0_IMGO_CON3 */
	CAMSV_REG_IMGO_CROP                             CAMSV_IMGO_CROP;                                 /* 0248, 0x1A050248, CAMSV_0_IMGO_CROP */
	UINT32                                          rsv_024C[125];                                   /* 024C..043F, 0x1A05024C..1A05043F */
	CAMSV_REG_DMA_ERR_CTRL                          CAMSV_DMA_ERR_CTRL;                              /* 0440, 0x1A050440, CAMSV_0_DMA_ERR_CTRL */
	CAMSV_REG_IMGO_ERR_STAT                         CAMSV_IMGO_ERR_STAT;                             /* 0444, 0x1A050444, CAMSV_0_IMGO_ERR_STAT */
	UINT32                                          rsv_0448[10];                                    /* 0448..046F, 0x1A050448..1A05046F */
	CAMSV_REG_DMA_DEBUG_ADDR                        CAMSV_DMA_DEBUG_ADDR;                            /* 0470, 0x1A050470, CAMSV_0_DMA_DEBUG_ADDR */
	CAMSV_REG_DMA_RSV1                              CAMSV_DMA_RSV1;                                  /* 0474, 0x1A050474, CAMSV_0_DMA_RSV1 */
	CAMSV_REG_DMA_RSV2                              CAMSV_DMA_RSV2;                                  /* 0478, 0x1A050478, CAMSV_0_DMA_RSV2 */
	CAMSV_REG_DMA_RSV3                              CAMSV_DMA_RSV3;                                  /* 047C, 0x1A05047C, CAMSV_0_DMA_RSV3 */
	CAMSV_REG_DMA_RSV4                              CAMSV_DMA_RSV4;                                  /* 0480, 0x1A050480, CAMSV_0_DMA_RSV4 */
	CAMSV_REG_DMA_RSV5                              CAMSV_DMA_RSV5;                                  /* 0484, 0x1A050484, CAMSV_0_DMA_RSV5 */
	CAMSV_REG_DMA_RSV6                              CAMSV_DMA_RSV6;                                  /* 0488, 0x1A050488, CAMSV_0_DMA_RSV6 */
	CAMSV_REG_DMA_DEBUG_SEL                         CAMSV_DMA_DEBUG_SEL;                             /* 048C, 0x1A05048C, CAMSV_0_DMA_DEBUG_SEL */
	CAMSV_REG_DMA_BW_SELF_TEST                      CAMSV_DMA_BW_SELF_TEST;                          /* 0490, 0x1A050490, CAMSV_0_DMA_BW_SELF_TEST */
	UINT32                                          rsv_0494[27];                                    /* 0494..04FF, 0x1A050494..1A0504FF */
	CAMSV_REG_TG_SEN_MODE                           CAMSV_TG_SEN_MODE;                               /* 0500, 0x1A050500, CAMSV_0_TG_SEN_MODE */
	CAMSV_REG_TG_VF_CON                             CAMSV_TG_VF_CON;                                 /* 0504, 0x1A050504, CAMSV_0_TG_VF_CON */
	CAMSV_REG_TG_SEN_GRAB_PXL                       CAMSV_TG_SEN_GRAB_PXL;                           /* 0508, 0x1A050508, CAMSV_0_TG_SEN_GRAB_PXL */
	CAMSV_REG_TG_SEN_GRAB_LIN                       CAMSV_TG_SEN_GRAB_LIN;                           /* 050C, 0x1A05050C, CAMSV_0_TG_SEN_GRAB_LIN */
	CAMSV_REG_TG_PATH_CFG                           CAMSV_TG_PATH_CFG;                               /* 0510, 0x1A050510, CAMSV_0_TG_PATH_CFG */
	CAMSV_REG_TG_MEMIN_CTL                          CAMSV_TG_MEMIN_CTL;                              /* 0514, 0x1A050514, CAMSV_0_TG_MEMIN_CTL */
	CAMSV_REG_TG_INT1                               CAMSV_TG_INT1;                                   /* 0518, 0x1A050518, CAMSV_0_TG_INT1 */
	CAMSV_REG_TG_INT2                               CAMSV_TG_INT2;                                   /* 051C, 0x1A05051C, CAMSV_0_TG_INT2 */
	CAMSV_REG_TG_SOF_CNT                            CAMSV_TG_SOF_CNT;                                /* 0520, 0x1A050520, CAMSV_0_TG_SOF_CNT */
	CAMSV_REG_TG_SOT_CNT                            CAMSV_TG_SOT_CNT;                                /* 0524, 0x1A050524, CAMSV_0_TG_SOT_CNT */
	CAMSV_REG_TG_EOT_CNT                            CAMSV_TG_EOT_CNT;                                /* 0528, 0x1A050528, CAMSV_0_TG_EOT_CNT */
	CAMSV_REG_TG_ERR_CTL                            CAMSV_TG_ERR_CTL;                                /* 052C, 0x1A05052C, CAMSV_0_TG_ERR_CTL */
	CAMSV_REG_TG_DAT_NO                             CAMSV_TG_DAT_NO;                                 /* 0530, 0x1A050530, CAMSV_0_TG_DAT_NO */
	CAMSV_REG_TG_FRM_CNT_ST                         CAMSV_TG_FRM_CNT_ST;                             /* 0534, 0x1A050534, CAMSV_0_TG_FRM_CNT_ST */
	CAMSV_REG_TG_FRMSIZE_ST                         CAMSV_TG_FRMSIZE_ST;                             /* 0538, 0x1A050538, CAMSV_0_TG_FRMSIZE_ST */
	CAMSV_REG_TG_INTER_ST                           CAMSV_TG_INTER_ST;                               /* 053C, 0x1A05053C, CAMSV_0_TG_INTER_ST */
	CAMSV_REG_TG_FLASHA_CTL                         CAMSV_TG_FLASHA_CTL;                             /* 0540, 0x1A050540, CAMSV_0_TG_FLASHA_CTL */
	CAMSV_REG_TG_FLASHA_LINE_CNT                    CAMSV_TG_FLASHA_LINE_CNT;                        /* 0544, 0x1A050544, CAMSV_0_TG_FLASHA_LINE_CNT */
	CAMSV_REG_TG_FLASHA_POS                         CAMSV_TG_FLASHA_POS;                             /* 0548, 0x1A050548, CAMSV_0_TG_FLASHA_POS */
	CAMSV_REG_TG_FLASHB_CTL                         CAMSV_TG_FLASHB_CTL;                             /* 054C, 0x1A05054C, CAMSV_0_TG_FLASHB_CTL */
	CAMSV_REG_TG_FLASHB_LINE_CNT                    CAMSV_TG_FLASHB_LINE_CNT;                        /* 0550, 0x1A050550, CAMSV_0_TG_FLASHB_LINE_CNT */
	CAMSV_REG_TG_FLASHB_POS                         CAMSV_TG_FLASHB_POS;                             /* 0554, 0x1A050554, CAMSV_0_TG_FLASHB_POS */
	CAMSV_REG_TG_FLASHB_POS1                        CAMSV_TG_FLASHB_POS1;                            /* 0558, 0x1A050558, CAMSV_0_TG_FLASHB_POS1 */
	UINT32                                          rsv_055C;                                        /* 055C, 0x1A05055C */
	CAMSV_REG_TG_I2C_CQ_TRIG                        CAMSV_TG_I2C_CQ_TRIG;                            /* 0560, 0x1A050560, CAMSV_0_TG_I2C_CQ_TRIG */
	CAMSV_REG_TG_CQ_TIMING                          CAMSV_TG_CQ_TIMING;                              /* 0564, 0x1A050564, CAMSV_0_TG_CQ_TIMING */
	CAMSV_REG_TG_CQ_NUM                             CAMSV_TG_CQ_NUM;                                 /* 0568, 0x1A050568, CAMSV_0_TG_CQ_NUM */
	UINT32                                          rsv_056C;                                        /* 056C, 0x1A05056C */
	CAMSV_REG_TG_TIME_STAMP                         CAMSV_TG_TIME_STAMP;                             /* 0570, 0x1A050570, CAMSV_0_TG_TIME_STAMP */
	CAMSV_REG_TG_SUB_PERIOD                         CAMSV_TG_SUB_PERIOD;                             /* 0574, 0x1A050574, CAMSV_0_TG_SUB_PERIOD */
	CAMSV_REG_TG_DAT_NO_R                           CAMSV_TG_DAT_NO_R;                               /* 0578, 0x1A050578, CAMSV_0_TG_DAT_NO_R */
	CAMSV_REG_TG_FRMSIZE_ST_R                       CAMSV_TG_FRMSIZE_ST_R;                           /* 057C, 0x1A05057C, CAMSV_0_TG_FRMSIZE_ST_R */
	UINT32                                          rsv_0580[544];                                   /* 0580..0DFF, 0x1A050580..1A050DFF */
	CAMSV_REG_DMA_FRAME_HEADER_EN                   CAMSV_DMA_FRAME_HEADER_EN;                       /* 0E00, 0x1A050E00, CAMSV_0_DMA_FRAME_HEADER_EN */
	CAMSV_REG_IMGO_FH_BASE_ADDR                     CAMSV_IMGO_FH_BASE_ADDR;                         /* 0E04, 0x1A050E04, CAMSV_0_IMGO_FH_BASE_ADDR */
	CAMSV_REG_RRZO_FH_BASE_ADDR                     CAMSV_RRZO_FH_BASE_ADDR;                         /* 0E08, 0x1A050E08, CAMSV_0_RRZO_FH_BASE_ADDR */
	CAMSV_REG_AAO_FH_BASE_ADDR                      CAMSV_AAO_FH_BASE_ADDR;                          /* 0E0C, 0x1A050E0C, CAMSV_0_AAO_FH_BASE_ADDR */
	CAMSV_REG_AFO_FH_BASE_ADDR                      CAMSV_AFO_FH_BASE_ADDR;                          /* 0E10, 0x1A050E10, CAMSV_0_AFO_FH_BASE_ADDR */
	CAMSV_REG_LCSO_FH_BASE_ADDR                     CAMSV_LCSO_FH_BASE_ADDR;                         /* 0E14, 0x1A050E14, CAMSV_0_LCSO_FH_BASE_ADDR */
	CAMSV_REG_UFEO_FH_BASE_ADDR                     CAMSV_UFEO_FH_BASE_ADDR;                         /* 0E18, 0x1A050E18, CAMSV_0_UFEO_FH_BASE_ADDR */
	CAMSV_REG_PDO_FH_BASE_ADDR                      CAMSV_PDO_FH_BASE_ADDR;                          /* 0E1C, 0x1A050E1C, CAMSV_0_PDO_FH_BASE_ADDR */
	UINT32                                          rsv_0E20[4];                                     /* 0E20..0E2F, 0x1A050E20..1A050E2F */
	CAMSV_REG_IMGO_FH_SPARE_2                       CAMSV_IMGO_FH_SPARE_2;                           /* 0E30, 0x1A050E30, CAMSV_0_IMGO_FH_SPARE_2 */
	CAMSV_REG_IMGO_FH_SPARE_3                       CAMSV_IMGO_FH_SPARE_3;                           /* 0E34, 0x1A050E34, CAMSV_0_IMGO_FH_SPARE_3 */
	CAMSV_REG_IMGO_FH_SPARE_4                       CAMSV_IMGO_FH_SPARE_4;                           /* 0E38, 0x1A050E38, CAMSV_0_IMGO_FH_SPARE_4 */
	CAMSV_REG_IMGO_FH_SPARE_5                       CAMSV_IMGO_FH_SPARE_5;                           /* 0E3C, 0x1A050E3C, CAMSV_0_IMGO_FH_SPARE_5 */
	CAMSV_REG_IMGO_FH_SPARE_6                       CAMSV_IMGO_FH_SPARE_6;                           /* 0E40, 0x1A050E40, CAMSV_0_IMGO_FH_SPARE_6 */
	CAMSV_REG_IMGO_FH_SPARE_7                       CAMSV_IMGO_FH_SPARE_7;                           /* 0E44, 0x1A050E44, CAMSV_0_IMGO_FH_SPARE_7 */
	CAMSV_REG_IMGO_FH_SPARE_8                       CAMSV_IMGO_FH_SPARE_8;                           /* 0E48, 0x1A050E48, CAMSV_0_IMGO_FH_SPARE_8 */
	CAMSV_REG_IMGO_FH_SPARE_9                       CAMSV_IMGO_FH_SPARE_9;                           /* 0E4C, 0x1A050E4C, CAMSV_0_IMGO_FH_SPARE_9 */
	CAMSV_REG_IMGO_FH_SPARE_10                      CAMSV_IMGO_FH_SPARE_10;                          /* 0E50, 0x1A050E50, CAMSV_0_IMGO_FH_SPARE_10 */
	CAMSV_REG_IMGO_FH_SPARE_11                      CAMSV_IMGO_FH_SPARE_11;                          /* 0E54, 0x1A050E54, CAMSV_0_IMGO_FH_SPARE_11 */
	CAMSV_REG_IMGO_FH_SPARE_12                      CAMSV_IMGO_FH_SPARE_12;                          /* 0E58, 0x1A050E58, CAMSV_0_IMGO_FH_SPARE_12 */
	CAMSV_REG_IMGO_FH_SPARE_13                      CAMSV_IMGO_FH_SPARE_13;                          /* 0E5C, 0x1A050E5C, CAMSV_0_IMGO_FH_SPARE_13 */
	CAMSV_REG_IMGO_FH_SPARE_14                      CAMSV_IMGO_FH_SPARE_14;                          /* 0E60, 0x1A050E60, CAMSV_0_IMGO_FH_SPARE_14 */
	CAMSV_REG_IMGO_FH_SPARE_15                      CAMSV_IMGO_FH_SPARE_15;                          /* 0E64, 0x1A050E64, CAMSV_0_IMGO_FH_SPARE_15 */
	CAMSV_REG_IMGO_FH_SPARE_16                      CAMSV_IMGO_FH_SPARE_16;                          /* 0E68, 0x1A050E68, CAMSV_0_IMGO_FH_SPARE_16 */
	UINT32                                          rsv_0E6C;                                        /* 0E6C, 1A050E6C */
}camsv_reg_t;

/* auto insert ralf auto gen above */

#endif // _ISP_REG_H_
