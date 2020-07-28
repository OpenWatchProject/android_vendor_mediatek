#ifndef _SENINF_REG_H_
#define _SENINF_REG_H_

typedef unsigned int FIELD;

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

#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)

#define SENINF_READ_REG(RegBase, RegName) (RegBase->RegName.Raw)
#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */
typedef volatile union _MIPI_REG_RX_ANA00_CSI0A_
{
		volatile struct	/* 0x11E40000 */
		{
				FIELD  RG_CSI0A_CPHY_EN                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0A_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0A_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0A_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0A_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0A_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0A_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI0A;	/* MIPI_RX_ANA00_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA04_CSI0A_
{
		volatile struct	/* 0x11E40004 */
		{
				FIELD  RG_CSI0A_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0A_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0A_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI0A_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI0A_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0A_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI0A;	/* MIPI_RX_ANA04_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA08_CSI0A_
{
		volatile struct	/* 0x11E40008 */
		{
				FIELD  RG_CSI0A_L0P_T0A_HSRT_CODE            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0A_L0N_T0B_HSRT_CODE            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0A_L1P_T0C_HSRT_CODE            :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI0A_L1N_T1A_HSRT_CODE            :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI0A;	/* MIPI_RX_ANA08_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI0A_
{
		volatile struct	/* 0x11E4000C */
		{
				FIELD  RG_CSI0A_L2P_T1B_HSRT_CODE            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0A_L2N_T1C_HSRT_CODE            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI0A;	/* MIPI_RX_ANA0C_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA10_CSI0A_
{
		volatile struct	/* 0x11E40010 */
		{
				FIELD  RG_CSI0A_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI0A_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0A_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0A_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI0A_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0A_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0A_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_DELAYCAL_EN      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_DELAYCAL_RSTB    :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_CSI0A_CPHY_T0_VREF_SEL             :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI0A;	/* MIPI_RX_ANA10_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA14_CSI0A_
{
		volatile struct	/* 0x11E40014 */
		{
				FIELD  RG_CSI0A_CPHY_T1_CDR_DELAYCAL_EN      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_DELAYCAL_RSTB    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_CPHY_T1_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA14_CSI0A;	/* MIPI_RX_ANA14_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA18_CSI0A_
{
		volatile struct	/* 0x11E40018 */
		{
				FIELD  RG_CSI0A_L0_T0AB_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0A_L0_T0AB_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0A_XX_T0CA_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI0A;	/* MIPI_RX_ANA18_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI0A_
{
		volatile struct	/* 0x11E4001C */
		{
				FIELD  RG_CSI0A_XX_T0BC_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0A_XX_T0BC_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0A_L1_T1AB_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI0A;	/* MIPI_RX_ANA1C_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA20_CSI0A_
{
		volatile struct	/* 0x11E40020 */
		{
				FIELD  RG_CSI0A_XX_T1CA_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0A_XX_T1CA_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0A_L2_T1BC_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA20_CSI0A;	/* MIPI_RX_ANA20_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA24_CSI0A_
{
		volatile struct	/* 0x11E40024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI0A_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI0A;	/* MIPI_RX_ANA24_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA28_CSI0A_
{
		volatile struct	/* 0x11E40028 */
		{
				FIELD  RG_CSI0A_CPHY_T0_CDR_DIRECT_EN        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_AUTOLOAD_EN      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_LPF_CTRL         :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0A_CPHY_T0_CDR_AB_WIDTH         :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_BC_WIDTH         :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_CA_WIDTH         :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_CK_DELAY         :  4;		/* 16..19, 0x000F0000 */
				FIELD  RG_CSI0A_CPHY_T0_HSDET_SEL            :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_MANUAL_EN        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA28_CSI0A;	/* MIPI_RX_ANA28_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA2C_CSI0A_
{
		volatile struct	/* 0x11E4002C */
		{
				FIELD  RG_CSI0A_CPHY_T0_CDR_INIT_CODE        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_EARLY_CODE       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0A_CPHY_T0_CDR_LATE_CODE        :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA2C_CSI0A;	/* MIPI_RX_ANA2C_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA34_CSI0A_
{
		volatile struct	/* 0x11E40034 */
		{
				FIELD  RG_CSI0A_CPHY_T1_CDR_DIRECT_EN        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_AUTOLOAD_EN      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_LPF_CTRL         :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0A_CPHY_T1_CDR_AB_WIDTH         :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_BC_WIDTH         :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_CA_WIDTH         :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_CK_DELAY         :  4;		/* 16..19, 0x000F0000 */
				FIELD  RG_CSI0A_CPHY_T1_HSDET_SEL            :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_MANUAL_EN        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA34_CSI0A;	/* MIPI_RX_ANA34_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA38_CSI0A_
{
		volatile struct	/* 0x11E40038 */
		{
				FIELD  RG_CSI0A_CPHY_T1_CDR_INIT_CODE        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_EARLY_CODE       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0A_CPHY_T1_CDR_LATE_CODE        :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA38_CSI0A;	/* MIPI_RX_ANA38_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA40_CSI0A_
{
		volatile struct	/* 0x11E40040 */
		{
				FIELD  RG_CSI0A_CPHY_FMCK_SEL                :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0A_ASYNC_OPTION                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RG_CSI0A_CPHY_SPARE_REG               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA40_CSI0A;	/* MIPI_RX_ANA40_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA48_CSI0A_
{
		volatile struct	/* 0x11E40048 */
		{
				FIELD  RGS_CSI0A_CDPHY_L0_T0AB_OS_CAL_CPLT   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RGS_CSI0A_CPHY_T0CA_OS_CAL_CPLT       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RGS_CSI0A_CPHY_T0BC_OS_CAL_CPLT       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RGS_CSI0A_CDPHY_L1_T1AB_OS_CAL_CPLT   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI0A_CPHY_T1CA_OS_CAL_CPLT       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI0A_CDPHY_L2_T1BC_OS_CAL_CPLT   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI0A_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI0A;	/* MIPI_RX_ANA48_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI0A_
{
		volatile struct	/* 0x11E40080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSR_CSI_CLK_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI0A;	/* MIPI_RX_WRAPPER80_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI0A_
{
		volatile struct	/* 0x11E40084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI0A;	/* MIPI_RX_WRAPPER84_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI0A_
{
		volatile struct	/* 0x11E40088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI0A;	/* MIPI_RX_WRAPPER88_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI0A_
{
		volatile struct	/* 0x11E4008C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI0A;	/* MIPI_RX_WRAPPER8C_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI0A_
{
		volatile struct	/* 0x11E40090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI0A;	/* MIPI_RX_WRAPPER90_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI0A_
{
		volatile struct	/* 0x11E40094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI0A;	/* MIPI_RX_WRAPPER94_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI0A_
{
		volatile struct	/* 0x11E40098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI0A;	/* MIPI_RX_WRAPPER98_CSI0A */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI0A_
{
		volatile struct	/* 0x11E4009C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI0A;	/* MIPI_RX_WRAPPER9C_CSI0A */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI0A_
{
		volatile struct	/* 0x11E400A4 */
		{
				FIELD  RG_CSI0A_CDPHY_L0_T0_SYNC_INIT_SEL    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_CDPHY_L0_T0_FORCE_INIT       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_CDPHY_L2_T1_SYNC_INIT_SEL    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0A_CDPHY_L2_T1_FORCE_INIT       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI0A;	/* MIPI_RX_ANAA4_CSI0A */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI0A_
{
		volatile struct	/* 0x11E400A8 */
		{
				FIELD  RG_CSI0A_CDPHY_L0_T0_BYTECK_INVERT    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0A_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0A_CDPHY_L2_T1_BYTECK_INVERT    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0A_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0A_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI0A_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0A_CDPHY_DELAYCAL_CK_SEL        :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI0A_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI0A;	/* MIPI_RX_ANAA8_CSI0A */

typedef volatile union _MIPI_REG_RX_ANA00_CSI0B_
{
		volatile struct	/* 0x11E41000 */
		{
				FIELD  RG_CSI0B_CPHY_EN                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0B_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0B_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0B_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0B_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0B_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0B_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI0B;	/* MIPI_RX_ANA00_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA04_CSI0B_
{
		volatile struct	/* 0x11E41004 */
		{
				FIELD  RG_CSI0B_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0B_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0B_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI0B_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI0B_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0B_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI0B;	/* MIPI_RX_ANA04_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA08_CSI0B_
{
		volatile struct	/* 0x11E41008 */
		{
				FIELD  RG_CSI0B_L0P_T0A_HSRT_CODE            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0B_L0N_T0B_HSRT_CODE            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0B_L1P_T0C_HSRT_CODE            :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI0B_L1N_T1A_HSRT_CODE            :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI0B;	/* MIPI_RX_ANA08_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI0B_
{
		volatile struct	/* 0x11E4100C */
		{
				FIELD  RG_CSI0B_L2P_T1B_HSRT_CODE            :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0B_L2N_T1C_HSRT_CODE            :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI0B;	/* MIPI_RX_ANA0C_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA10_CSI0B_
{
		volatile struct	/* 0x11E41010 */
		{
				FIELD  RG_CSI0B_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI0B_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0B_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0B_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI0B_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0B_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0B_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_DELAYCAL_EN      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_DELAYCAL_RSTB    :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_CSI0B_CPHY_T0_VREF_SEL             :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI0B;	/* MIPI_RX_ANA10_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA14_CSI0B_
{
		volatile struct	/* 0x11E41014 */
		{
				FIELD  RG_CSI0B_CPHY_T1_CDR_DELAYCAL_EN      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_DELAYCAL_RSTB    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_CPHY_T1_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA14_CSI0B;	/* MIPI_RX_ANA14_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA18_CSI0B_
{
		volatile struct	/* 0x11E41018 */
		{
				FIELD  RG_CSI0B_L0_T0AB_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0B_L0_T0AB_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0B_XX_T0CA_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI0B;	/* MIPI_RX_ANA18_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI0B_
{
		volatile struct	/* 0x11E4101C */
		{
				FIELD  RG_CSI0B_XX_T0BC_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0B_XX_T0BC_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0B_L1_T1AB_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI0B;	/* MIPI_RX_ANA1C_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA20_CSI0B_
{
		volatile struct	/* 0x11E41020 */
		{
				FIELD  RG_CSI0B_XX_T1CA_EQ_OS_CAL_EN         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_MON_EN            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_SCA               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_SCB               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_IS                :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_BW                :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_SRA               :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0B_XX_T1CA_EQ_SRB               :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_OS_CAL_EN         :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_MON_EN            :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_SCA               :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_SCB               :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_IS                :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_BW                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_SRA               :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI0B_L2_T1BC_EQ_SRB               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA20_CSI0B;	/* MIPI_RX_ANA20_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA24_CSI0B_
{
		volatile struct	/* 0x11E41024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI0B_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI0B;	/* MIPI_RX_ANA24_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA28_CSI0B_
{
		volatile struct	/* 0x11E41028 */
		{
				FIELD  RG_CSI0B_CPHY_T0_CDR_DIRECT_EN        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_AUTOLOAD_EN      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_LPF_CTRL         :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0B_CPHY_T0_CDR_AB_WIDTH         :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_BC_WIDTH         :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_CA_WIDTH         :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_CK_DELAY         :  4;		/* 16..19, 0x000F0000 */
				FIELD  RG_CSI0B_CPHY_T0_HSDET_SEL            :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_MANUAL_EN        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA28_CSI0B;	/* MIPI_RX_ANA28_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA2C_CSI0B_
{
		volatile struct	/* 0x11E4102C */
		{
				FIELD  RG_CSI0B_CPHY_T0_CDR_INIT_CODE        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_EARLY_CODE       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0B_CPHY_T0_CDR_LATE_CODE        :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA2C_CSI0B;	/* MIPI_RX_ANA2C_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA34_CSI0B_
{
		volatile struct	/* 0x11E41034 */
		{
				FIELD  RG_CSI0B_CPHY_T1_CDR_DIRECT_EN        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_AUTOLOAD_EN      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_LPF_CTRL         :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0B_CPHY_T1_CDR_AB_WIDTH         :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_BC_WIDTH         :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_CA_WIDTH         :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_CK_DELAY         :  4;		/* 16..19, 0x000F0000 */
				FIELD  RG_CSI0B_CPHY_T1_HSDET_SEL            :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_MANUAL_EN        :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA34_CSI0B;	/* MIPI_RX_ANA34_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA38_CSI0B_
{
		volatile struct	/* 0x11E41038 */
		{
				FIELD  RG_CSI0B_CPHY_T1_CDR_INIT_CODE        :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_EARLY_CODE       :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI0B_CPHY_T1_CDR_LATE_CODE        :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA38_CSI0B;	/* MIPI_RX_ANA38_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA48_CSI0B_
{
		volatile struct	/* 0x11E41048 */
		{
				FIELD  RGS_CSI0B_CDPHY_L0_T0AB_OS_CAL_CPLT   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RGS_CSI0B_CPHY_T0CA_OS_CAL_CPLT       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RGS_CSI0B_CPHY_T0BC_OS_CAL_CPLT       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RGS_CSI0B_CDPHY_L1_T1AB_OS_CAL_CPLT   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI0B_CPHY_T1CA_OS_CAL_CPLT       :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI0B_CDPHY_L2_T1BC_OS_CAL_CPLT   :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI0B_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI0B;	/* MIPI_RX_ANA48_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI0B_
{
		volatile struct	/* 0x11E41080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSR_CSI_CLK_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI0B;	/* MIPI_RX_WRAPPER80_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI0B_
{
		volatile struct	/* 0x11E41084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI0B;	/* MIPI_RX_WRAPPER84_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI0B_
{
		volatile struct	/* 0x11E41088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI0B;	/* MIPI_RX_WRAPPER88_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI0B_
{
		volatile struct	/* 0x11E4108C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI0B;	/* MIPI_RX_WRAPPER8C_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI0B_
{
		volatile struct	/* 0x11E41090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI0B;	/* MIPI_RX_WRAPPER90_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI0B_
{
		volatile struct	/* 0x11E41094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI0B;	/* MIPI_RX_WRAPPER94_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI0B_
{
		volatile struct	/* 0x11E41098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI0B;	/* MIPI_RX_WRAPPER98_CSI0B */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI0B_
{
		volatile struct	/* 0x11E4109C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI0B;	/* MIPI_RX_WRAPPER9C_CSI0B */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI0B_
{
		volatile struct	/* 0x11E410A4 */
		{
				FIELD  RG_CSI0B_CDPHY_L0_T0_SYNC_INIT_SEL    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_CDPHY_L0_T0_FORCE_INIT       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_CDPHY_L2_T1_SYNC_INIT_SEL    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0B_CDPHY_L2_T1_FORCE_INIT       :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI0B;	/* MIPI_RX_ANAA4_CSI0B */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI0B_
{
		volatile struct	/* 0x11E410A8 */
		{
				FIELD  RG_CSI0B_CDPHY_L0_T0_BYTECK_INVERT    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0B_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0B_CDPHY_L2_T1_BYTECK_INVERT    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI0B_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI0B_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI0B_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0B_CDPHY_DELAYCAL_CK_SEL        :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI0B_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI0B;	/* MIPI_RX_ANAA8_CSI0B */

typedef volatile union _MIPI_REG_RX_ANA00_CSI1A_
{
		volatile struct	/* 0x11E42000 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1A_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI1A_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI1A_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1A_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI1A_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI1A_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI1A_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI1A;	/* MIPI_RX_ANA00_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA04_CSI1A_
{
		volatile struct	/* 0x11E42004 */
		{
				FIELD  RG_CSI1A_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1A_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1A_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI1A_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI1A_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI1A_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI1A_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI1A;	/* MIPI_RX_ANA04_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA08_CSI1A_
{
		volatile struct	/* 0x11E42008 */
		{
				FIELD  RG_CSI1A_L0P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI1A_L0N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI1A_L1P_HSRT_CODE                :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI1A_L1N_HSRT_CODE                :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI1A;	/* MIPI_RX_ANA08_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI1A_
{
		volatile struct	/* 0x11E4200C */
		{
				FIELD  RG_CSI1A_L2P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI1A_L2N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI1A;	/* MIPI_RX_ANA0C_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA10_CSI1A_
{
		volatile struct	/* 0x11E42010 */
		{
				FIELD  RG_CSI1A_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI1A_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI1A_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI1A_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI1A_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI1A_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI1A_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI1A;	/* MIPI_RX_ANA10_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA18_CSI1A_
{
		volatile struct	/* 0x11E42018 */
		{
				FIELD  RG_CSI1A_L0_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_L0_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_L0_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1A_L0_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1A_L0_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI1A_L0_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI1A_L0_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI1A_L0_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI1A_L1_EQ_OS_CAL_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI1A_L1_EQ_MON_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI1A_L1_EQ_SCA                    :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI1A_L1_EQ_SCB                    :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI1A_L1_EQ_IS                     :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI1A_L1_EQ_BW                     :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI1A_L1_EQ_SRA                    :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI1A_L1_EQ_SRB                    :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI1A;	/* MIPI_RX_ANA18_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI1A_
{
		volatile struct	/* 0x11E4201C */
		{
				FIELD  RG_CSI1A_L2_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_L2_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_L2_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1A_L2_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1A_L2_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI1A_L2_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI1A_L2_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI1A_L2_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI1A;	/* MIPI_RX_ANA1C_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA24_CSI1A_
{
		volatile struct	/* 0x11E42024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI1A_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI1A;	/* MIPI_RX_ANA24_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA48_CSI1A_
{
		volatile struct	/* 0x11E42048 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RGS_CSI1A_DPHY_L0_OS_CAL_CPLT         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI1A_DPHY_L1_OS_CAL_CPLT         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI1A_DPHY_L2_OS_CAL_CPLT         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI1A_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI1A;	/* MIPI_RX_ANA48_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI1A_
{
		volatile struct	/* 0x11E42080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI1A;	/* MIPI_RX_WRAPPER80_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI1A_
{
		volatile struct	/* 0x11E42084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI1A;	/* MIPI_RX_WRAPPER84_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI1A_
{
		volatile struct	/* 0x11E42088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI1A;	/* MIPI_RX_WRAPPER88_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI1A_
{
		volatile struct	/* 0x11E4208C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI1A;	/* MIPI_RX_WRAPPER8C_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI1A_
{
		volatile struct	/* 0x11E42090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI1A;	/* MIPI_RX_WRAPPER90_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI1A_
{
		volatile struct	/* 0x11E42094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI1A;	/* MIPI_RX_WRAPPER94_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI1A_
{
		volatile struct	/* 0x11E42098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI1A;	/* MIPI_RX_WRAPPER98_CSI1A */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI1A_
{
		volatile struct	/* 0x11E4209C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI1A;	/* MIPI_RX_WRAPPER9C_CSI1A */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI1A_
{
		volatile struct	/* 0x11E420A4 */
		{
				FIELD  RG_CSI1A_DPHY_L0_SYNC_INIT_SEL        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_DPHY_L0_FORCE_INIT           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1A_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1A_DPHY_L2_SYNC_INIT_SEL        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI1A_DPHY_L2_FORCE_INIT           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI1A;	/* MIPI_RX_ANAA4_CSI1A */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI1A_
{
		volatile struct	/* 0x11E420A8 */
		{
				FIELD  RG_CSI1A_DPHY_L0_BYTECK_INVERT        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1A_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1A_DPHY_L2_BYTECK_INVERT        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1A_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1A_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI1A_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1A_DPHY_DELAYCAL_CK_SEL         :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI1A_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI1A;	/* MIPI_RX_ANAA8_CSI1A */

typedef volatile union _MIPI_REG_RX_ANA00_CSI1B_
{
		volatile struct	/* 0x11E43000 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1B_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI1B_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI1B_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1B_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI1B_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI1B_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI1B_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI1B;	/* MIPI_RX_ANA00_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA04_CSI1B_
{
		volatile struct	/* 0x11E43004 */
		{
				FIELD  RG_CSI1B_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1B_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1B_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI1B_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI1B_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI1B_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI1B_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI1B;	/* MIPI_RX_ANA04_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA08_CSI1B_
{
		volatile struct	/* 0x11E43008 */
		{
				FIELD  RG_CSI1B_L0P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI1B_L0N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI1B_L1P_HSRT_CODE                :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI1B_L1N_HSRT_CODE                :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI1B;	/* MIPI_RX_ANA08_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI1B_
{
		volatile struct	/* 0x11E4300C */
		{
				FIELD  RG_CSI1B_L2P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI1B_L2N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI1B;	/* MIPI_RX_ANA0C_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA10_CSI1B_
{
		volatile struct	/* 0x11E43010 */
		{
				FIELD  RG_CSI1B_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI1B_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI1B_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI1B_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI1B_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI1B_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI1B_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI1B;	/* MIPI_RX_ANA10_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA18_CSI1B_
{
		volatile struct	/* 0x11E43018 */
		{
				FIELD  RG_CSI1B_L0_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_L0_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_L0_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1B_L0_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1B_L0_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI1B_L0_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI1B_L0_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI1B_L0_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI1B_L1_EQ_OS_CAL_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI1B_L1_EQ_MON_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI1B_L1_EQ_SCA                    :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI1B_L1_EQ_SCB                    :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI1B_L1_EQ_IS                     :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI1B_L1_EQ_BW                     :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI1B_L1_EQ_SRA                    :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI1B_L1_EQ_SRB                    :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI1B;	/* MIPI_RX_ANA18_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI1B_
{
		volatile struct	/* 0x11E4301C */
		{
				FIELD  RG_CSI1B_L2_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_L2_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_L2_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1B_L2_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1B_L2_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI1B_L2_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI1B_L2_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI1B_L2_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI1B;	/* MIPI_RX_ANA1C_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA24_CSI1B_
{
		volatile struct	/* 0x11E43024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI1B_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI1B;	/* MIPI_RX_ANA24_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA48_CSI1B_
{
		volatile struct	/* 0x11E43048 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RGS_CSI1B_DPHY_L0_OS_CAL_CPLT         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI1B_DPHY_L1_OS_CAL_CPLT         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI1B_DPHY_L2_OS_CAL_CPLT         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI1B_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI1B;	/* MIPI_RX_ANA48_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI1B_
{
		volatile struct	/* 0x11E43080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI1B;	/* MIPI_RX_WRAPPER80_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI1B_
{
		volatile struct	/* 0x11E43084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI1B;	/* MIPI_RX_WRAPPER84_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI1B_
{
		volatile struct	/* 0x11E43088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI1B;	/* MIPI_RX_WRAPPER88_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI1B_
{
		volatile struct	/* 0x11E4308C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI1B;	/* MIPI_RX_WRAPPER8C_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI1B_
{
		volatile struct	/* 0x11E43090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI1B;	/* MIPI_RX_WRAPPER90_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI1B_
{
		volatile struct	/* 0x11E43094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI1B;	/* MIPI_RX_WRAPPER94_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI1B_
{
		volatile struct	/* 0x11E43098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI1B;	/* MIPI_RX_WRAPPER98_CSI1B */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI1B_
{
		volatile struct	/* 0x11E4309C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI1B;	/* MIPI_RX_WRAPPER9C_CSI1B */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI1B_
{
		volatile struct	/* 0x11E430A4 */
		{
				FIELD  RG_CSI1B_DPHY_L0_SYNC_INIT_SEL        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_DPHY_L0_FORCE_INIT           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1B_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1B_DPHY_L2_SYNC_INIT_SEL        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI1B_DPHY_L2_FORCE_INIT           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI1B;	/* MIPI_RX_ANAA4_CSI1B */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI1B_
{
		volatile struct	/* 0x11E430A8 */
		{
				FIELD  RG_CSI1B_DPHY_L0_BYTECK_INVERT        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI1B_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI1B_DPHY_L2_BYTECK_INVERT        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI1B_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI1B_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI1B_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI1B_DPHY_DELAYCAL_CK_SEL         :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI1B_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI1B;	/* MIPI_RX_ANAA8_CSI1B */

typedef volatile union _MIPI_REG_RX_ANA00_CSI2A_
{
		volatile struct	/* 0x11E44000 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2A_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI2A_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI2A_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2A_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI2A_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI2A_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI2A_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI2A;	/* MIPI_RX_ANA00_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA04_CSI2A_
{
		volatile struct	/* 0x11E44004 */
		{
				FIELD  RG_CSI2A_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2A_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2A_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI2A_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI2A_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI2A_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI2A_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI2A;	/* MIPI_RX_ANA04_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA08_CSI2A_
{
		volatile struct	/* 0x11E44008 */
		{
				FIELD  RG_CSI2A_L0P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI2A_L0N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI2A_L1P_HSRT_CODE                :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI2A_L1N_HSRT_CODE                :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI2A;	/* MIPI_RX_ANA08_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI2A_
{
		volatile struct	/* 0x11E4400C */
		{
				FIELD  RG_CSI2A_L2P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI2A_L2N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI2A;	/* MIPI_RX_ANA0C_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA10_CSI2A_
{
		volatile struct	/* 0x11E44010 */
		{
				FIELD  RG_CSI2A_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI2A_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI2A_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI2A_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI2A_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI2A_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI2A_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI2A;	/* MIPI_RX_ANA10_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA18_CSI2A_
{
		volatile struct	/* 0x11E44018 */
		{
				FIELD  RG_CSI2A_L0_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_L0_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_L0_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2A_L0_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2A_L0_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI2A_L0_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI2A_L0_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI2A_L0_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI2A_L1_EQ_OS_CAL_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI2A_L1_EQ_MON_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI2A_L1_EQ_SCA                    :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI2A_L1_EQ_SCB                    :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI2A_L1_EQ_IS                     :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI2A_L1_EQ_BW                     :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI2A_L1_EQ_SRA                    :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI2A_L1_EQ_SRB                    :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI2A;	/* MIPI_RX_ANA18_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI2A_
{
		volatile struct	/* 0x11E4401C */
		{
				FIELD  RG_CSI2A_L2_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_L2_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_L2_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2A_L2_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2A_L2_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI2A_L2_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI2A_L2_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI2A_L2_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI2A;	/* MIPI_RX_ANA1C_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA24_CSI2A_
{
		volatile struct	/* 0x11E44024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI2A_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI2A;	/* MIPI_RX_ANA24_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA48_CSI2A_
{
		volatile struct	/* 0x11E44048 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RGS_CSI2A_DPHY_L0_OS_CAL_CPLT         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI2A_DPHY_L1_OS_CAL_CPLT         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI2A_DPHY_L2_OS_CAL_CPLT         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI2A_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI2A;	/* MIPI_RX_ANA48_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI2A_
{
		volatile struct	/* 0x11E44080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI2A;	/* MIPI_RX_WRAPPER80_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI2A_
{
		volatile struct	/* 0x11E44084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI2A;	/* MIPI_RX_WRAPPER84_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI2A_
{
		volatile struct	/* 0x11E44088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI2A;	/* MIPI_RX_WRAPPER88_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI2A_
{
		volatile struct	/* 0x11E4408C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI2A;	/* MIPI_RX_WRAPPER8C_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI2A_
{
		volatile struct	/* 0x11E44090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI2A;	/* MIPI_RX_WRAPPER90_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI2A_
{
		volatile struct	/* 0x11E44094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI2A;	/* MIPI_RX_WRAPPER94_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI2A_
{
		volatile struct	/* 0x11E44098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI2A;	/* MIPI_RX_WRAPPER98_CSI2A */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI2A_
{
		volatile struct	/* 0x11E4409C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI2A;	/* MIPI_RX_WRAPPER9C_CSI2A */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI2A_
{
		volatile struct	/* 0x11E440A4 */
		{
				FIELD  RG_CSI2A_DPHY_L0_SYNC_INIT_SEL        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_DPHY_L0_FORCE_INIT           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2A_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2A_DPHY_L2_SYNC_INIT_SEL        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI2A_DPHY_L2_FORCE_INIT           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI2A;	/* MIPI_RX_ANAA4_CSI2A */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI2A_
{
		volatile struct	/* 0x11E440A8 */
		{
				FIELD  RG_CSI2A_DPHY_L0_BYTECK_INVERT        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2A_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2A_DPHY_L2_BYTECK_INVERT        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2A_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2A_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI2A_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2A_DPHY_DELAYCAL_CK_SEL         :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI2A_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI2A;	/* MIPI_RX_ANAA8_CSI2A */

typedef volatile union _MIPI_REG_RX_ANA00_CSI2B_
{
		volatile struct	/* 0x11E45000 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_EQ_PROTECT_EN                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_BG_LPF_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2B_BG_CORE_EN                   :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI2B_DPHY_L0_CKMODE_EN            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI2B_DPHY_L0_CKSEL                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2B_DPHY_L1_CKMODE_EN            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI2B_DPHY_L1_CKSEL                :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI2B_DPHY_L2_CKMODE_EN            :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI2B_DPHY_L2_CKSEL                :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI2B;	/* MIPI_RX_ANA00_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA04_CSI2B_
{
		volatile struct	/* 0x11E45004 */
		{
				FIELD  RG_CSI2B_BG_LPRX_VTH_SEL              :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2B_BG_LPRX_VTL_SEL              :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2B_BG_HSDET_VTH_SEL             :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI2B_BG_HSDET_VTL_SEL             :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI2B_BG_VREF_SEL                  :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  RG_CSI2B_BG_MON_VREF_SEL              :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI2B_FORCE_HSRT_EN                :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI2B;	/* MIPI_RX_ANA04_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA08_CSI2B_
{
		volatile struct	/* 0x11E45008 */
		{
				FIELD  RG_CSI2B_L0P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI2B_L0N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RG_CSI2B_L1P_HSRT_CODE                :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  3;		/* 21..23, 0x00E00000 */
				FIELD  RG_CSI2B_L1N_HSRT_CODE                :  5;		/* 24..28, 0x1F000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI2B;	/* MIPI_RX_ANA08_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI2B_
{
		volatile struct	/* 0x11E4500C */
		{
				FIELD  RG_CSI2B_L2P_HSRT_CODE                :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI2B_L2N_HSRT_CODE                :  5;		/*  8..12, 0x00001F00 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI2B;	/* MIPI_RX_ANA0C_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA10_CSI2B_
{
		volatile struct	/* 0x11E45010 */
		{
				FIELD  RG_CSI2B_DPHY_L0_DELAYCAL_EN          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_DPHY_L0_DELAYCAL_RSTB        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_DPHY_L0_VREF_SEL             :  6;		/*  2.. 7, 0x000000FC */
				FIELD  RG_CSI2B_DPHY_L1_DELAYCAL_EN          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI2B_DPHY_L1_DELAYCAL_RSTB        :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI2B_DPHY_L1_VREF_SEL             :  6;		/* 10..15, 0x0000FC00 */
				FIELD  RG_CSI2B_DPHY_L2_DELAYCAL_EN          :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI2B_DPHY_L2_DELAYCAL_RSTB        :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI2B_DPHY_L2_VREF_SEL             :  6;		/* 18..23, 0x00FC0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI2B;	/* MIPI_RX_ANA10_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA18_CSI2B_
{
		volatile struct	/* 0x11E45018 */
		{
				FIELD  RG_CSI2B_L0_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_L0_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_L0_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2B_L0_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2B_L0_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI2B_L0_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI2B_L0_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI2B_L0_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI2B_L1_EQ_OS_CAL_EN              :  1;		/* 16..16, 0x00010000 */
				FIELD  RG_CSI2B_L1_EQ_MON_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  RG_CSI2B_L1_EQ_SCA                    :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_CSI2B_L1_EQ_SCB                    :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI2B_L1_EQ_IS                     :  2;		/* 20..21, 0x00300000 */
				FIELD  RG_CSI2B_L1_EQ_BW                     :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI2B_L1_EQ_SRA                    :  4;		/* 24..27, 0x0F000000 */
				FIELD  RG_CSI2B_L1_EQ_SRB                    :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA18_CSI2B;	/* MIPI_RX_ANA18_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA1C_CSI2B_
{
		volatile struct	/* 0x11E4501C */
		{
				FIELD  RG_CSI2B_L2_EQ_OS_CAL_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_L2_EQ_MON_EN                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_L2_EQ_SCA                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2B_L2_EQ_SCB                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2B_L2_EQ_IS                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_CSI2B_L2_EQ_BW                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RG_CSI2B_L2_EQ_SRA                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI2B_L2_EQ_SRB                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA1C_CSI2B;	/* MIPI_RX_ANA1C_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA24_CSI2B_
{
		volatile struct	/* 0x11E45024 */
		{
				FIELD  rsv_0                                 : 24;		/*  0..23, 0x00FFFFFF */
				FIELD  RG_CSI2B_RESERVE                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI2B;	/* MIPI_RX_ANA24_CSI2B */

typedef volatile union _MIPI_REG_RX_ANA48_CSI2B_
{
		volatile struct	/* 0x11E45048 */
		{
				FIELD  rsv_0                                 :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RGS_CSI2B_DPHY_L0_OS_CAL_CPLT         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RGS_CSI2B_DPHY_L1_OS_CAL_CPLT         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RGS_CSI2B_DPHY_L2_OS_CAL_CPLT         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  RGS_CSI2B_OS_CAL_CODE                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA48_CSI2B;	/* MIPI_RX_ANA48_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER80_CSI2B_
{
		volatile struct	/* 0x11E45080 */
		{
				FIELD  CSR_CSI_CLK_MON                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSR_CSI_MON_MUX                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSR_CSI_RST_MODE                      :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  6;		/* 18..23, 0x00FC0000 */
				FIELD  CSR_SW_RST                            :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER80_CSI2B;	/* MIPI_RX_WRAPPER80_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER84_CSI2B_
{
		volatile struct	/* 0x11E45084 */
		{
				FIELD  CSI_DEBUG_OUT                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER84_CSI2B;	/* MIPI_RX_WRAPPER84_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER88_CSI2B_
{
		volatile struct	/* 0x11E45088 */
		{
				FIELD  CSR_SW_MODE_0                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER88_CSI2B;	/* MIPI_RX_WRAPPER88_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER8C_CSI2B_
{
		volatile struct	/* 0x11E4508C */
		{
				FIELD  CSR_SW_MODE_1                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER8C_CSI2B;	/* MIPI_RX_WRAPPER8C_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER90_CSI2B_
{
		volatile struct	/* 0x11E45090 */
		{
				FIELD  CSR_SW_MODE_2                         : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER90_CSI2B;	/* MIPI_RX_WRAPPER90_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER94_CSI2B_
{
		volatile struct	/* 0x11E45094 */
		{
				FIELD  CSR_SW_VALUE_0                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER94_CSI2B;	/* MIPI_RX_WRAPPER94_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER98_CSI2B_
{
		volatile struct	/* 0x11E45098 */
		{
				FIELD  CSR_SW_VALUE_1                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER98_CSI2B;	/* MIPI_RX_WRAPPER98_CSI2B */

typedef volatile union _MIPI_REG_RX_WRAPPER9C_CSI2B_
{
		volatile struct	/* 0x11E4509C */
		{
				FIELD  CSR_SW_VALUE_2                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_WRAPPER9C_CSI2B;	/* MIPI_RX_WRAPPER9C_CSI2B */

typedef volatile union _MIPI_REG_RX_ANAA4_CSI2B_
{
		volatile struct	/* 0x11E450A4 */
		{
				FIELD  RG_CSI2B_DPHY_L0_SYNC_INIT_SEL        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_DPHY_L0_FORCE_INIT           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_DPHY_L1_SYNC_INIT_SEL        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2B_DPHY_L1_FORCE_INIT           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2B_DPHY_L2_SYNC_INIT_SEL        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI2B_DPHY_L2_FORCE_INIT           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA4_CSI2B;	/* MIPI_RX_ANAA4_CSI2B */

typedef volatile union _MIPI_REG_RX_ANAA8_CSI2B_
{
		volatile struct	/* 0x11E450A8 */
		{
				FIELD  RG_CSI2B_DPHY_L0_BYTECK_INVERT        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI2B_DPHY_L1_BYTECK_INVERT        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI2B_DPHY_L2_BYTECK_INVERT        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_CSI2B_DPHY_HSDET_LEVEL_MODE_EN     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_CSI2B_OS_CAL_SEL                   :  3;		/*  4.. 6, 0x00000070 */
				FIELD  RG_CSI2B_DPHY_HSDET_DIG_BACK_EN       :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI2B_DPHY_DELAYCAL_CK_SEL         :  3;		/*  8..10, 0x00000700 */
				FIELD  RG_CSI2B_OS_CAL_DIV                   :  2;		/* 11..12, 0x00001800 */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA8_CSI2B;	/* MIPI_RX_ANAA8_CSI2B */

typedef volatile struct _mipi_ana_reg_t_	/* 0x11E40000..0x11E450FB */
{
	MIPI_REG_RX_ANA00_CSI0A                         MIPI_RX_ANA00_CSI0A;                             /* 0000, 0x11E40000 */
	MIPI_REG_RX_ANA04_CSI0A                         MIPI_RX_ANA04_CSI0A;                             /* 0004, 0x11E40004 */
	MIPI_REG_RX_ANA08_CSI0A                         MIPI_RX_ANA08_CSI0A;                             /* 0008, 0x11E40008 */
	MIPI_REG_RX_ANA0C_CSI0A                         MIPI_RX_ANA0C_CSI0A;                             /* 000C, 0x11E4000C */
	MIPI_REG_RX_ANA10_CSI0A                         MIPI_RX_ANA10_CSI0A;                             /* 0010, 0x11E40010 */
	MIPI_REG_RX_ANA14_CSI0A                         MIPI_RX_ANA14_CSI0A;                             /* 0014, 0x11E40014 */
	MIPI_REG_RX_ANA18_CSI0A                         MIPI_RX_ANA18_CSI0A;                             /* 0018, 0x11E40018 */
	MIPI_REG_RX_ANA1C_CSI0A                         MIPI_RX_ANA1C_CSI0A;                             /* 001C, 0x11E4001C */
	MIPI_REG_RX_ANA20_CSI0A                         MIPI_RX_ANA20_CSI0A;                             /* 0020, 0x11E40020 */
	MIPI_REG_RX_ANA24_CSI0A                         MIPI_RX_ANA24_CSI0A;                             /* 0024, 0x11E40024 */
	MIPI_REG_RX_ANA28_CSI0A                         MIPI_RX_ANA28_CSI0A;                             /* 0028, 0x11E40028 */
	MIPI_REG_RX_ANA2C_CSI0A                         MIPI_RX_ANA2C_CSI0A;                             /* 002C, 0x11E4002C */
	UINT32                                          rsv_0030;                                        /* 0030, 0x11E40030 */
	MIPI_REG_RX_ANA34_CSI0A                         MIPI_RX_ANA34_CSI0A;                             /* 0034, 0x11E40034 */
	MIPI_REG_RX_ANA38_CSI0A                         MIPI_RX_ANA38_CSI0A;                             /* 0038, 0x11E40038 */
	UINT32                                          rsv_003C;                                        /* 003C, 0x11E4003C */
	MIPI_REG_RX_ANA40_CSI0A                         MIPI_RX_ANA40_CSI0A;                             /* 0040, 0x11E40040 */
	UINT32                                          rsv_0044;                                        /* 0044, 0x11E40044 */
	MIPI_REG_RX_ANA48_CSI0A                         MIPI_RX_ANA48_CSI0A;                             /* 0048, 0x11E40048 */
	UINT32                                          rsv_004C[13];                                    /* 004C..007F, 0x11E4004C..11E4007F */
	MIPI_REG_RX_WRAPPER80_CSI0A                     MIPI_RX_WRAPPER80_CSI0A;                         /* 0080, 0x11E40080 */
	MIPI_REG_RX_WRAPPER84_CSI0A                     MIPI_RX_WRAPPER84_CSI0A;                         /* 0084, 0x11E40084 */
	MIPI_REG_RX_WRAPPER88_CSI0A                     MIPI_RX_WRAPPER88_CSI0A;                         /* 0088, 0x11E40088 */
	MIPI_REG_RX_WRAPPER8C_CSI0A                     MIPI_RX_WRAPPER8C_CSI0A;                         /* 008C, 0x11E4008C */
	MIPI_REG_RX_WRAPPER90_CSI0A                     MIPI_RX_WRAPPER90_CSI0A;                         /* 0090, 0x11E40090 */
	MIPI_REG_RX_WRAPPER94_CSI0A                     MIPI_RX_WRAPPER94_CSI0A;                         /* 0094, 0x11E40094 */
	MIPI_REG_RX_WRAPPER98_CSI0A                     MIPI_RX_WRAPPER98_CSI0A;                         /* 0098, 0x11E40098 */
	MIPI_REG_RX_WRAPPER9C_CSI0A                     MIPI_RX_WRAPPER9C_CSI0A;                         /* 009C, 0x11E4009C */
	UINT32                                          rsv_00A0;                                        /* 00A0, 0x11E400A0 */
	MIPI_REG_RX_ANAA4_CSI0A                         MIPI_RX_ANAA4_CSI0A;                             /* 00A4, 0x11E400A4 */
	MIPI_REG_RX_ANAA8_CSI0A                         MIPI_RX_ANAA8_CSI0A;                             /* 00A8, 0x11E400A8 */
	UINT32                                          rsv_00AC[981];                                   /* 00AC..0FFF, 0x11E400AC..11E40FFF */
	MIPI_REG_RX_ANA00_CSI0B                         MIPI_RX_ANA00_CSI0B;                             /* 1000, 0x11E41000 */
	MIPI_REG_RX_ANA04_CSI0B                         MIPI_RX_ANA04_CSI0B;                             /* 1004, 0x11E41004 */
	MIPI_REG_RX_ANA08_CSI0B                         MIPI_RX_ANA08_CSI0B;                             /* 1008, 0x11E41008 */
	MIPI_REG_RX_ANA0C_CSI0B                         MIPI_RX_ANA0C_CSI0B;                             /* 100C, 0x11E4100C */
	MIPI_REG_RX_ANA10_CSI0B                         MIPI_RX_ANA10_CSI0B;                             /* 1010, 0x11E41010 */
	MIPI_REG_RX_ANA14_CSI0B                         MIPI_RX_ANA14_CSI0B;                             /* 1014, 0x11E41014 */
	MIPI_REG_RX_ANA18_CSI0B                         MIPI_RX_ANA18_CSI0B;                             /* 1018, 0x11E41018 */
	MIPI_REG_RX_ANA1C_CSI0B                         MIPI_RX_ANA1C_CSI0B;                             /* 101C, 0x11E4101C */
	MIPI_REG_RX_ANA20_CSI0B                         MIPI_RX_ANA20_CSI0B;                             /* 1020, 0x11E41020 */
	MIPI_REG_RX_ANA24_CSI0B                         MIPI_RX_ANA24_CSI0B;                             /* 1024, 0x11E41024 */
	MIPI_REG_RX_ANA28_CSI0B                         MIPI_RX_ANA28_CSI0B;                             /* 1028, 0x11E41028 */
	MIPI_REG_RX_ANA2C_CSI0B                         MIPI_RX_ANA2C_CSI0B;                             /* 102C, 0x11E4102C */
	UINT32                                          rsv_1030;                                        /* 1030, 0x11E41030 */
	MIPI_REG_RX_ANA34_CSI0B                         MIPI_RX_ANA34_CSI0B;                             /* 1034, 0x11E41034 */
	MIPI_REG_RX_ANA38_CSI0B                         MIPI_RX_ANA38_CSI0B;                             /* 1038, 0x11E41038 */
	UINT32                                          rsv_103C[3];                                     /* 103C..1047, 0x11E4103C..11E41047 */
	MIPI_REG_RX_ANA48_CSI0B                         MIPI_RX_ANA48_CSI0B;                             /* 1048, 0x11E41048 */
	UINT32                                          rsv_104C[13];                                    /* 104C..107F, 0x11E4104C..11E4107F */
	MIPI_REG_RX_WRAPPER80_CSI0B                     MIPI_RX_WRAPPER80_CSI0B;                         /* 1080, 0x11E41080 */
	MIPI_REG_RX_WRAPPER84_CSI0B                     MIPI_RX_WRAPPER84_CSI0B;                         /* 1084, 0x11E41084 */
	MIPI_REG_RX_WRAPPER88_CSI0B                     MIPI_RX_WRAPPER88_CSI0B;                         /* 1088, 0x11E41088 */
	MIPI_REG_RX_WRAPPER8C_CSI0B                     MIPI_RX_WRAPPER8C_CSI0B;                         /* 108C, 0x11E4108C */
	MIPI_REG_RX_WRAPPER90_CSI0B                     MIPI_RX_WRAPPER90_CSI0B;                         /* 1090, 0x11E41090 */
	MIPI_REG_RX_WRAPPER94_CSI0B                     MIPI_RX_WRAPPER94_CSI0B;                         /* 1094, 0x11E41094 */
	MIPI_REG_RX_WRAPPER98_CSI0B                     MIPI_RX_WRAPPER98_CSI0B;                         /* 1098, 0x11E41098 */
	MIPI_REG_RX_WRAPPER9C_CSI0B                     MIPI_RX_WRAPPER9C_CSI0B;                         /* 109C, 0x11E4109C */
	UINT32                                          rsv_10A0;                                        /* 10A0, 0x11E410A0 */
	MIPI_REG_RX_ANAA4_CSI0B                         MIPI_RX_ANAA4_CSI0B;                             /* 10A4, 0x11E410A4 */
	MIPI_REG_RX_ANAA8_CSI0B                         MIPI_RX_ANAA8_CSI0B;                             /* 10A8, 0x11E410A8 */
	UINT32                                          rsv_10AC[981];                                   /* 10AC..1FFF, 0x11E410AC..11E41FFF */
	MIPI_REG_RX_ANA00_CSI1A                         MIPI_RX_ANA00_CSI1A;                             /* 2000, 0x11E42000 */
	MIPI_REG_RX_ANA04_CSI1A                         MIPI_RX_ANA04_CSI1A;                             /* 2004, 0x11E42004 */
	MIPI_REG_RX_ANA08_CSI1A                         MIPI_RX_ANA08_CSI1A;                             /* 2008, 0x11E42008 */
	MIPI_REG_RX_ANA0C_CSI1A                         MIPI_RX_ANA0C_CSI1A;                             /* 200C, 0x11E4200C */
	MIPI_REG_RX_ANA10_CSI1A                         MIPI_RX_ANA10_CSI1A;                             /* 2010, 0x11E42010 */
	UINT32                                          rsv_2014;                                        /* 2014, 0x11E42014 */
	MIPI_REG_RX_ANA18_CSI1A                         MIPI_RX_ANA18_CSI1A;                             /* 2018, 0x11E42018 */
	MIPI_REG_RX_ANA1C_CSI1A                         MIPI_RX_ANA1C_CSI1A;                             /* 201C, 0x11E4201C */
	UINT32                                          rsv_2020;                                        /* 2020, 0x11E42020 */
	MIPI_REG_RX_ANA24_CSI1A                         MIPI_RX_ANA24_CSI1A;                             /* 2024, 0x11E42024 */
	UINT32                                          rsv_2028[8];                                     /* 2028..2047, 0x11E42028..11E42047 */
	MIPI_REG_RX_ANA48_CSI1A                         MIPI_RX_ANA48_CSI1A;                             /* 2048, 0x11E42048 */
	UINT32                                          rsv_204C[13];                                    /* 204C..207F, 0x11E4204C..11E4207F */
	MIPI_REG_RX_WRAPPER80_CSI1A                     MIPI_RX_WRAPPER80_CSI1A;                         /* 2080, 0x11E42080 */
	MIPI_REG_RX_WRAPPER84_CSI1A                     MIPI_RX_WRAPPER84_CSI1A;                         /* 2084, 0x11E42084 */
	MIPI_REG_RX_WRAPPER88_CSI1A                     MIPI_RX_WRAPPER88_CSI1A;                         /* 2088, 0x11E42088 */
	MIPI_REG_RX_WRAPPER8C_CSI1A                     MIPI_RX_WRAPPER8C_CSI1A;                         /* 208C, 0x11E4208C */
	MIPI_REG_RX_WRAPPER90_CSI1A                     MIPI_RX_WRAPPER90_CSI1A;                         /* 2090, 0x11E42090 */
	MIPI_REG_RX_WRAPPER94_CSI1A                     MIPI_RX_WRAPPER94_CSI1A;                         /* 2094, 0x11E42094 */
	MIPI_REG_RX_WRAPPER98_CSI1A                     MIPI_RX_WRAPPER98_CSI1A;                         /* 2098, 0x11E42098 */
	MIPI_REG_RX_WRAPPER9C_CSI1A                     MIPI_RX_WRAPPER9C_CSI1A;                         /* 209C, 0x11E4209C */
	UINT32                                          rsv_20A0;                                        /* 20A0, 0x11E420A0 */
	MIPI_REG_RX_ANAA4_CSI1A                         MIPI_RX_ANAA4_CSI1A;                             /* 20A4, 0x11E420A4 */
	MIPI_REG_RX_ANAA8_CSI1A                         MIPI_RX_ANAA8_CSI1A;                             /* 20A8, 0x11E420A8 */
	UINT32                                          rsv_20AC[981];                                   /* 20AC..2FFF, 0x11E420AC..11E42FFF */
	MIPI_REG_RX_ANA00_CSI1B                         MIPI_RX_ANA00_CSI1B;                             /* 3000, 0x11E43000 */
	MIPI_REG_RX_ANA04_CSI1B                         MIPI_RX_ANA04_CSI1B;                             /* 3004, 0x11E43004 */
	MIPI_REG_RX_ANA08_CSI1B                         MIPI_RX_ANA08_CSI1B;                             /* 3008, 0x11E43008 */
	MIPI_REG_RX_ANA0C_CSI1B                         MIPI_RX_ANA0C_CSI1B;                             /* 300C, 0x11E4300C */
	MIPI_REG_RX_ANA10_CSI1B                         MIPI_RX_ANA10_CSI1B;                             /* 3010, 0x11E43010 */
	UINT32                                          rsv_3014;                                        /* 3014, 0x11E43014 */
	MIPI_REG_RX_ANA18_CSI1B                         MIPI_RX_ANA18_CSI1B;                             /* 3018, 0x11E43018 */
	MIPI_REG_RX_ANA1C_CSI1B                         MIPI_RX_ANA1C_CSI1B;                             /* 301C, 0x11E4301C */
	UINT32                                          rsv_3020;                                        /* 3020, 0x11E43020 */
	MIPI_REG_RX_ANA24_CSI1B                         MIPI_RX_ANA24_CSI1B;                             /* 3024, 0x11E43024 */
	UINT32                                          rsv_3028[8];                                     /* 3028..3047, 0x11E43028..11E43047 */
	MIPI_REG_RX_ANA48_CSI1B                         MIPI_RX_ANA48_CSI1B;                             /* 3048, 0x11E43048 */
	UINT32                                          rsv_304C[13];                                    /* 304C..307F, 0x11E4304C..11E4307F */
	MIPI_REG_RX_WRAPPER80_CSI1B                     MIPI_RX_WRAPPER80_CSI1B;                         /* 3080, 0x11E43080 */
	MIPI_REG_RX_WRAPPER84_CSI1B                     MIPI_RX_WRAPPER84_CSI1B;                         /* 3084, 0x11E43084 */
	MIPI_REG_RX_WRAPPER88_CSI1B                     MIPI_RX_WRAPPER88_CSI1B;                         /* 3088, 0x11E43088 */
	MIPI_REG_RX_WRAPPER8C_CSI1B                     MIPI_RX_WRAPPER8C_CSI1B;                         /* 308C, 0x11E4308C */
	MIPI_REG_RX_WRAPPER90_CSI1B                     MIPI_RX_WRAPPER90_CSI1B;                         /* 3090, 0x11E43090 */
	MIPI_REG_RX_WRAPPER94_CSI1B                     MIPI_RX_WRAPPER94_CSI1B;                         /* 3094, 0x11E43094 */
	MIPI_REG_RX_WRAPPER98_CSI1B                     MIPI_RX_WRAPPER98_CSI1B;                         /* 3098, 0x11E43098 */
	MIPI_REG_RX_WRAPPER9C_CSI1B                     MIPI_RX_WRAPPER9C_CSI1B;                         /* 309C, 0x11E4309C */
	UINT32                                          rsv_30A0;                                        /* 30A0, 0x11E430A0 */
	MIPI_REG_RX_ANAA4_CSI1B                         MIPI_RX_ANAA4_CSI1B;                             /* 30A4, 0x11E430A4 */
	MIPI_REG_RX_ANAA8_CSI1B                         MIPI_RX_ANAA8_CSI1B;                             /* 30A8, 0x11E430A8 */
	UINT32                                          rsv_30AC[981];                                   /* 30AC..3FFF, 0x11E430AC..11E43FFF */
	MIPI_REG_RX_ANA00_CSI2A                         MIPI_RX_ANA00_CSI2A;                             /* 4000, 0x11E44000 */
	MIPI_REG_RX_ANA04_CSI2A                         MIPI_RX_ANA04_CSI2A;                             /* 4004, 0x11E44004 */
	MIPI_REG_RX_ANA08_CSI2A                         MIPI_RX_ANA08_CSI2A;                             /* 4008, 0x11E44008 */
	MIPI_REG_RX_ANA0C_CSI2A                         MIPI_RX_ANA0C_CSI2A;                             /* 400C, 0x11E4400C */
	MIPI_REG_RX_ANA10_CSI2A                         MIPI_RX_ANA10_CSI2A;                             /* 4010, 0x11E44010 */
	UINT32                                          rsv_4014;                                        /* 4014, 0x11E44014 */
	MIPI_REG_RX_ANA18_CSI2A                         MIPI_RX_ANA18_CSI2A;                             /* 4018, 0x11E44018 */
	MIPI_REG_RX_ANA1C_CSI2A                         MIPI_RX_ANA1C_CSI2A;                             /* 401C, 0x11E4401C */
	UINT32                                          rsv_4020;                                        /* 4020, 0x11E44020 */
	MIPI_REG_RX_ANA24_CSI2A                         MIPI_RX_ANA24_CSI2A;                             /* 4024, 0x11E44024 */
	UINT32                                          rsv_4028[8];                                     /* 4028..4047, 0x11E44028..11E44047 */
	MIPI_REG_RX_ANA48_CSI2A                         MIPI_RX_ANA48_CSI2A;                             /* 4048, 0x11E44048 */
	UINT32                                          rsv_404C[13];                                    /* 404C..407F, 0x11E4404C..11E4407F */
	MIPI_REG_RX_WRAPPER80_CSI2A                     MIPI_RX_WRAPPER80_CSI2A;                         /* 4080, 0x11E44080 */
	MIPI_REG_RX_WRAPPER84_CSI2A                     MIPI_RX_WRAPPER84_CSI2A;                         /* 4084, 0x11E44084 */
	MIPI_REG_RX_WRAPPER88_CSI2A                     MIPI_RX_WRAPPER88_CSI2A;                         /* 4088, 0x11E44088 */
	MIPI_REG_RX_WRAPPER8C_CSI2A                     MIPI_RX_WRAPPER8C_CSI2A;                         /* 408C, 0x11E4408C */
	MIPI_REG_RX_WRAPPER90_CSI2A                     MIPI_RX_WRAPPER90_CSI2A;                         /* 4090, 0x11E44090 */
	MIPI_REG_RX_WRAPPER94_CSI2A                     MIPI_RX_WRAPPER94_CSI2A;                         /* 4094, 0x11E44094 */
	MIPI_REG_RX_WRAPPER98_CSI2A                     MIPI_RX_WRAPPER98_CSI2A;                         /* 4098, 0x11E44098 */
	MIPI_REG_RX_WRAPPER9C_CSI2A                     MIPI_RX_WRAPPER9C_CSI2A;                         /* 409C, 0x11E4409C */
	UINT32                                          rsv_40A0;                                        /* 40A0, 0x11E440A0 */
	MIPI_REG_RX_ANAA4_CSI2A                         MIPI_RX_ANAA4_CSI2A;                             /* 40A4, 0x11E440A4 */
	MIPI_REG_RX_ANAA8_CSI2A                         MIPI_RX_ANAA8_CSI2A;                             /* 40A8, 0x11E440A8 */
	UINT32                                          rsv_40AC[981];                                   /* 40AC..4FFF, 0x11E440AC..11E44FFF */
	MIPI_REG_RX_ANA00_CSI2B                         MIPI_RX_ANA00_CSI2B;                             /* 5000, 0x11E45000 */
	MIPI_REG_RX_ANA04_CSI2B                         MIPI_RX_ANA04_CSI2B;                             /* 5004, 0x11E45004 */
	MIPI_REG_RX_ANA08_CSI2B                         MIPI_RX_ANA08_CSI2B;                             /* 5008, 0x11E45008 */
	MIPI_REG_RX_ANA0C_CSI2B                         MIPI_RX_ANA0C_CSI2B;                             /* 500C, 0x11E4500C */
	MIPI_REG_RX_ANA10_CSI2B                         MIPI_RX_ANA10_CSI2B;                             /* 5010, 0x11E45010 */
	UINT32                                          rsv_5014;                                        /* 5014, 0x11E45014 */
	MIPI_REG_RX_ANA18_CSI2B                         MIPI_RX_ANA18_CSI2B;                             /* 5018, 0x11E45018 */
	MIPI_REG_RX_ANA1C_CSI2B                         MIPI_RX_ANA1C_CSI2B;                             /* 501C, 0x11E4501C */
	UINT32                                          rsv_5020;                                        /* 5020, 0x11E45020 */
	MIPI_REG_RX_ANA24_CSI2B                         MIPI_RX_ANA24_CSI2B;                             /* 5024, 0x11E45024 */
	UINT32                                          rsv_5028[8];                                     /* 5028..5047, 0x11E45028..11E45047 */
	MIPI_REG_RX_ANA48_CSI2B                         MIPI_RX_ANA48_CSI2B;                             /* 5048, 0x11E45048 */
	UINT32                                          rsv_504C[13];                                    /* 504C..507F, 0x11E4504C..11E4507F */
	MIPI_REG_RX_WRAPPER80_CSI2B                     MIPI_RX_WRAPPER80_CSI2B;                         /* 5080, 0x11E45080 */
	MIPI_REG_RX_WRAPPER84_CSI2B                     MIPI_RX_WRAPPER84_CSI2B;                         /* 5084, 0x11E45084 */
	MIPI_REG_RX_WRAPPER88_CSI2B                     MIPI_RX_WRAPPER88_CSI2B;                         /* 5088, 0x11E45088 */
	MIPI_REG_RX_WRAPPER8C_CSI2B                     MIPI_RX_WRAPPER8C_CSI2B;                         /* 508C, 0x11E4508C */
	MIPI_REG_RX_WRAPPER90_CSI2B                     MIPI_RX_WRAPPER90_CSI2B;                         /* 5090, 0x11E45090 */
	MIPI_REG_RX_WRAPPER94_CSI2B                     MIPI_RX_WRAPPER94_CSI2B;                         /* 5094, 0x11E45094 */
	MIPI_REG_RX_WRAPPER98_CSI2B                     MIPI_RX_WRAPPER98_CSI2B;                         /* 5098, 0x11E45098 */
	MIPI_REG_RX_WRAPPER9C_CSI2B                     MIPI_RX_WRAPPER9C_CSI2B;                         /* 509C, 0x11E4509C */
	UINT32                                          rsv_50A0;                                        /* 50A0, 0x11E450A0 */
	MIPI_REG_RX_ANAA4_CSI2B                         MIPI_RX_ANAA4_CSI2B;                             /* 50A4, 0x11E450A4 */
	MIPI_REG_RX_ANAA8_CSI2B                         MIPI_RX_ANAA8_CSI2B;                             /* 50A8, 0x11E450A8 */
	UINT32                                          rsv_50AC[20];                                    /* 50AC..50FB, 11E450AC..11E450FB */
}mipi_ana_reg_t;

typedef volatile struct _mipi_ana_dphy_reg_t_
{
	MIPI_REG_RX_ANA00_CSI1A                         MIPI_RX_ANA00_CSI1A;                             /* 2000, 0x11E42000 */
	MIPI_REG_RX_ANA04_CSI1A                         MIPI_RX_ANA04_CSI1A;                             /* 2004, 0x11E42004 */
	MIPI_REG_RX_ANA08_CSI1A                         MIPI_RX_ANA08_CSI1A;                             /* 2008, 0x11E42008 */
	MIPI_REG_RX_ANA0C_CSI1A                         MIPI_RX_ANA0C_CSI1A;                             /* 200C, 0x11E4200C */
	MIPI_REG_RX_ANA10_CSI1A                         MIPI_RX_ANA10_CSI1A;                             /* 2010, 0x11E42010 */
	UINT32                                          rsv_2014;                                        /* 2014, 0x11E42014 */
	MIPI_REG_RX_ANA18_CSI1A                         MIPI_RX_ANA18_CSI1A;                             /* 2018, 0x11E42018 */
	MIPI_REG_RX_ANA1C_CSI1A                         MIPI_RX_ANA1C_CSI1A;                             /* 201C, 0x11E4201C */
	UINT32                                          rsv_2020;                                        /* 2020, 0x11E42020 */
	MIPI_REG_RX_ANA24_CSI1A                         MIPI_RX_ANA24_CSI1A;                             /* 2024, 0x11E42024 */
	UINT32                                          rsv_2028[8];                                     /* 2028..2047, 0x11E42028..11E42047 */
	MIPI_REG_RX_ANA48_CSI1A                         MIPI_RX_ANA48_CSI1A;                             /* 2048, 0x11E42048 */
	UINT32                                          rsv_204C[13];                                    /* 204C..207F, 0x11E4204C..11E4207F */
	MIPI_REG_RX_WRAPPER80_CSI1A                     MIPI_RX_WRAPPER80_CSI1A;                         /* 2080, 0x11E42080 */
	MIPI_REG_RX_WRAPPER84_CSI1A                     MIPI_RX_WRAPPER84_CSI1A;                         /* 2084, 0x11E42084 */
	MIPI_REG_RX_WRAPPER88_CSI1A                     MIPI_RX_WRAPPER88_CSI1A;                         /* 2088, 0x11E42088 */
	MIPI_REG_RX_WRAPPER8C_CSI1A                     MIPI_RX_WRAPPER8C_CSI1A;                         /* 208C, 0x11E4208C */
	MIPI_REG_RX_WRAPPER90_CSI1A                     MIPI_RX_WRAPPER90_CSI1A;                         /* 2090, 0x11E42090 */
	MIPI_REG_RX_WRAPPER94_CSI1A                     MIPI_RX_WRAPPER94_CSI1A;                         /* 2094, 0x11E42094 */
	MIPI_REG_RX_WRAPPER98_CSI1A                     MIPI_RX_WRAPPER98_CSI1A;                         /* 2098, 0x11E42098 */
	MIPI_REG_RX_WRAPPER9C_CSI1A                     MIPI_RX_WRAPPER9C_CSI1A;                         /* 209C, 0x11E4209C */
	UINT32                                          rsv_20A0;                                        /* 20A0, 0x11E420A0 */
	MIPI_REG_RX_ANAA4_CSI1A                         MIPI_RX_ANAA4_CSI1A;                             /* 20A4, 0x11E420A4 */
	MIPI_REG_RX_ANAA8_CSI1A                         MIPI_RX_ANAA8_CSI1A;                             /* 20A8, 0x11E420A8 */
	UINT32                                          rsv_20AC[981];                                   /* 20AC..2FFF, 0x11E420AC..11E42FFF */
	MIPI_REG_RX_ANA00_CSI1B                         MIPI_RX_ANA00_CSI1B;                             /* 3000, 0x11E43000 */
	MIPI_REG_RX_ANA04_CSI1B                         MIPI_RX_ANA04_CSI1B;                             /* 3004, 0x11E43004 */
	MIPI_REG_RX_ANA08_CSI1B                         MIPI_RX_ANA08_CSI1B;                             /* 3008, 0x11E43008 */
	MIPI_REG_RX_ANA0C_CSI1B                         MIPI_RX_ANA0C_CSI1B;                             /* 300C, 0x11E4300C */
	MIPI_REG_RX_ANA10_CSI1B                         MIPI_RX_ANA10_CSI1B;                             /* 3010, 0x11E43010 */
	UINT32                                          rsv_3014;                                        /* 3014, 0x11E43014 */
	MIPI_REG_RX_ANA18_CSI1B                         MIPI_RX_ANA18_CSI1B;                             /* 3018, 0x11E43018 */
	MIPI_REG_RX_ANA1C_CSI1B                         MIPI_RX_ANA1C_CSI1B;                             /* 301C, 0x11E4301C */
	UINT32                                          rsv_3020;                                        /* 3020, 0x11E43020 */
	MIPI_REG_RX_ANA24_CSI1B                         MIPI_RX_ANA24_CSI1B;                             /* 3024, 0x11E43024 */
	UINT32                                          rsv_3028[8];                                     /* 3028..3047, 0x11E43028..11E43047 */
	MIPI_REG_RX_ANA48_CSI1B                         MIPI_RX_ANA48_CSI1B;                             /* 3048, 0x11E43048 */
	UINT32                                          rsv_304C[13];                                    /* 304C..307F, 0x11E4304C..11E4307F */
	MIPI_REG_RX_WRAPPER80_CSI1B                     MIPI_RX_WRAPPER80_CSI1B;                         /* 3080, 0x11E43080 */
	MIPI_REG_RX_WRAPPER84_CSI1B                     MIPI_RX_WRAPPER84_CSI1B;                         /* 3084, 0x11E43084 */
	MIPI_REG_RX_WRAPPER88_CSI1B                     MIPI_RX_WRAPPER88_CSI1B;                         /* 3088, 0x11E43088 */
	MIPI_REG_RX_WRAPPER8C_CSI1B                     MIPI_RX_WRAPPER8C_CSI1B;                         /* 308C, 0x11E4308C */
	MIPI_REG_RX_WRAPPER90_CSI1B                     MIPI_RX_WRAPPER90_CSI1B;                         /* 3090, 0x11E43090 */
	MIPI_REG_RX_WRAPPER94_CSI1B                     MIPI_RX_WRAPPER94_CSI1B;                         /* 3094, 0x11E43094 */
	MIPI_REG_RX_WRAPPER98_CSI1B                     MIPI_RX_WRAPPER98_CSI1B;                         /* 3098, 0x11E43098 */
	MIPI_REG_RX_WRAPPER9C_CSI1B                     MIPI_RX_WRAPPER9C_CSI1B;                         /* 309C, 0x11E4309C */
	UINT32                                          rsv_30A0;                                        /* 30A0, 0x11E430A0 */
	MIPI_REG_RX_ANAA4_CSI1B                         MIPI_RX_ANAA4_CSI1B;                             /* 30A4, 0x11E430A4 */
	MIPI_REG_RX_ANAA8_CSI1B                         MIPI_RX_ANAA8_CSI1B;                             /* 30A8, 0x11E430A8 */                           /* 20A8, 0x11E420A8 */
}mipi_ana_dphy_reg_t;


typedef volatile union _SENINF_REG_TOP_CTRL_
{
		volatile struct	/* 0x1A040000 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SENINF1_PCLK_SEL                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF2_PCLK_SEL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF1_PCLK_EN                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SENINF2_PCLK_EN                       :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF_TOP_N3D_SW_RST                 :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 11;		/* 17..27, 0x0FFE0000 */
				FIELD  SENINF_TOP_DBG_SEL                    :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_CTRL;	/* SENINF_TOP_CTRL */

typedef volatile union _SENINF_REG_TOP_CMODEL_PAR_
{
		volatile struct	/* 0x1A040004 */
		{
				FIELD  SENINF1_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF2_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF3_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF4_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF1_FORMAT                        :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SENINF2_FORMAT                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF3_FORMAT                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF4_FORMAT                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_CMODEL_PAR;	/* SENINF_TOP_CMODEL_PAR */

typedef volatile union _SENINF_REG_TOP_MUX_CTRL_
{
		volatile struct	/* 0x1A040008 */
		{
				FIELD  SENINF1_MUX_SRC_SEL                   :  4;		/*  0.. 3, 0x0000000F */
				FIELD  SENINF2_MUX_SRC_SEL                   :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SENINF3_MUX_SRC_SEL                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF4_MUX_SRC_SEL                   :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF5_MUX_SRC_SEL                   :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF6_MUX_SRC_SEL                   :  4;		/* 20..23, 0x00F00000 */
				FIELD  SENINF7_MUX_SRC_SEL                   :  4;		/* 24..27, 0x0F000000 */
				FIELD  SENINF8_MUX_SRC_SEL                   :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_MUX_CTRL;	/* SENINF_TOP_MUX_CTRL */

typedef volatile union _SENINF_REG_TOP_CAM_MUX_CTRL_
{
		volatile struct	/* 0x1A040010 */
		{
				FIELD  SENINF_CAM0_MUX_SRC_SEL               :  4;		/*  0.. 3, 0x0000000F */
				FIELD  SENINF_CAM1_MUX_SRC_SEL               :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SENINF_CAM2_MUX_SRC_SEL               :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF_CAM3_MUX_SRC_SEL               :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF_CAM4_MUX_SRC_SEL               :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_CAM5_MUX_SRC_SEL               :  4;		/* 20..23, 0x00F00000 */
				FIELD  SENINF_CAM6_MUX_SRC_SEL               :  4;		/* 24..27, 0x0F000000 */
				FIELD  SENINF_CAM7_MUX_SRC_SEL               :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_CAM_MUX_CTRL;	/* SENINF_TOP_CAM_MUX_CTRL */

typedef volatile union _SENINF_REG_TOP_N3D_A_CTL_
{
		volatile struct	/* 0x1A040014 */
		{
				FIELD  SENINF_N3D_S1_SEN_PCLK_SRC_SEL_A      :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_N3D_S1_SEN_VSYNC_SRC_SEL_A     :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_N3D_S2_SEN_PCLK_SRC_SEL_A      :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_N3D_S2_SEN_VSYNC_SRC_SEL_A     :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_N3D_A_CTL;	/* SENINF_TOP_N3D_A_CTL */

typedef volatile union _SENINF_REG_TOP_N3D_B_CTL_
{
		volatile struct	/* 0x1A040018 */
		{
				FIELD  SENINF_N3D_S1_SEN_PCLK_SRC_SEL_B      :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_N3D_S1_SEN_VSYNC_SRC_SEL_B     :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_N3D_S2_SEN_PCLK_SRC_SEL_B      :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_N3D_S2_SEN_VSYNC_SRC_SEL_B     :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_N3D_B_CTL;	/* SENINF_TOP_N3D_B_CTL */

typedef volatile union _SENINF_REG_TOP_PHY_SENINF_CTL_CSI0_
{
		volatile struct	/* 0x1A04001C */
		{
				FIELD  DPHY_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CPHY_MODE                             :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CK_SEL_1                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  CK_SEL_2                              :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 17;		/* 14..30, 0x7FFFC000 */
				FIELD  phy_seninf_lane_mux_csi0_en           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_PHY_SENINF_CTL_CSI0;	/* SENINF_TOP_PHY_SENINF_CTL_CSI0 */

typedef volatile union _SENINF_REG_TOP_PHY_SENINF_CTL_CSI1_
{
		volatile struct	/* 0x1A040020 */
		{
				FIELD  DPHY_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CPHY_MODE                             :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CK_SEL_1                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  CK_SEL_2                              :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 17;		/* 14..30, 0x7FFFC000 */
				FIELD  phy_seninf_lane_mux_csi1_en           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_PHY_SENINF_CTL_CSI1;	/* SENINF_TOP_PHY_SENINF_CTL_CSI1 */

typedef volatile union _SENINF_REG_TOP_PHY_SENINF_CTL_CSI2_
{
		volatile struct	/* 0x1A040024 */
		{
				FIELD  DPHY_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CPHY_MODE                             :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CK_SEL_1                              :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  CK_SEL_2                              :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                : 17;		/* 14..30, 0x7FFFC000 */
				FIELD  phy_seninf_lane_mux_csi2_en           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_PHY_SENINF_CTL_CSI2;	/* SENINF_TOP_PHY_SENINF_CTL_CSI2 */

typedef volatile union _SENINF_REG_N3D_A_CTL_
{
		volatile struct	/* 0x1A040100 */
		{
				FIELD  MODE                                  :  2;		/*  0.. 1, 0x00000003 */
				FIELD  I2C1_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  I2C2_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  I2C1_INT_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  I2C2_INT_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  N3D_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  W1CLR                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DIFF_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DDBG_SEL                              :  3;		/*  9..11, 0x00000E00 */
				FIELD  MODE1_DBG                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEN1_TIM_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  SEN2_TIM_EN                           :  1;		/* 17..17, 0x00020000 */
				FIELD  SEN1_OV_VS_INT_EN                     :  1;		/* 18..18, 0x00040000 */
				FIELD  SEN2_OV_VS_INT_EN                     :  1;		/* 19..19, 0x00080000 */
				FIELD  HW_SYNC_MODE                          :  1;		/* 20..20, 0x00100000 */
				FIELD  VALID_TG_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SYNC_PIN_A_EN                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SYNC_PIN_A_POLARITY                   :  1;		/* 23..23, 0x00800000 */
				FIELD  SYNC_PIN_B_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  SYNC_PIN_B_POLARITY                   :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_CTL;	/* SENINF_N3D_A_CTL */

typedef volatile union _SENINF_REG_N3D_A_POS_
{
		volatile struct	/* 0x1A040104 */
		{
				FIELD  N3D_POS                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_POS;	/* SENINF_N3D_A_POS */

typedef volatile union _SENINF_REG_N3D_A_TRIG_
{
		volatile struct	/* 0x1A040108 */
		{
				FIELD  I2CA_TRIG                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2CB_TRIG                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_TRIG;	/* SENINF_N3D_A_TRIG */

typedef volatile union _SENINF_REG_N3D_A_INT_
{
		volatile struct	/* 0x1A04010C */
		{
				FIELD  I2C1_INT                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2C2_INT                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DIFF_INT                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SEN1_OV_VS_INT                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEN2_OV_VS_INT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_INT;	/* SENINF_N3D_A_INT */

typedef volatile union _SENINF_REG_N3D_A_CNT0_
{
		volatile struct	/* 0x1A040110 */
		{
				FIELD  N3D_CNT0                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_CNT0;	/* SENINF_N3D_A_CNT0 */

typedef volatile union _SENINF_REG_N3D_A_CNT1_
{
		volatile struct	/* 0x1A040114 */
		{
				FIELD  N3D_CNT1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_CNT1;	/* SENINF_N3D_A_CNT1 */

typedef volatile union _SENINF_REG_N3D_A_DBG_
{
		volatile struct	/* 0x1A040118 */
		{
				FIELD  N3D_DBG                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_DBG;	/* SENINF_N3D_A_DBG */

typedef volatile union _SENINF_REG_N3D_A_DIFF_THR_
{
		volatile struct	/* 0x1A04011C */
		{
				FIELD  N3D_DIFF_THR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_DIFF_THR;	/* SENINF_N3D_A_DIFF_THR */

typedef volatile union _SENINF_REG_N3D_A_DIFF_CNT_
{
		volatile struct	/* 0x1A040120 */
		{
				FIELD  N3D_DIFF_CNT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_DIFF_CNT;	/* SENINF_N3D_A_DIFF_CNT */

typedef volatile union _SENINF_REG_N3D_A_DBG_1_
{
		volatile struct	/* 0x1A040124 */
		{
				FIELD  N3D_DBG_1                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_DBG_1;	/* SENINF_N3D_A_DBG_1 */

typedef volatile union _SENINF_REG_N3D_A_VALID_TG_CNT_
{
		volatile struct	/* 0x1A040128 */
		{
				FIELD  N3D_VALID_TG_CNT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_VALID_TG_CNT;	/* SENINF_N3D_A_VALID_TG_CNT */

typedef volatile union _SENINF_REG_N3D_A_SYNC_A_PERIOD_
{
		volatile struct	/* 0x1A04012C */
		{
				FIELD  N3D_SYNC_A_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_SYNC_A_PERIOD;	/* SENINF_N3D_A_SYNC_A_PERIOD */

typedef volatile union _SENINF_REG_N3D_A_SYNC_B_PERIOD_
{
		volatile struct	/* 0x1A040130 */
		{
				FIELD  N3D_SYNC_B_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_SYNC_B_PERIOD;	/* SENINF_N3D_A_SYNC_B_PERIOD */

typedef volatile union _SENINF_REG_N3D_A_SYNC_A_PULSE_LEN_
{
		volatile struct	/* 0x1A040134 */
		{
				FIELD  N3D_SYNC_A_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_SYNC_A_PULSE_LEN;	/* SENINF_N3D_A_SYNC_A_PULSE_LEN */

typedef volatile union _SENINF_REG_N3D_A_SYNC_B_PULSE_LEN_
{
		volatile struct	/* 0x1A040138 */
		{
				FIELD  N3D_SYNC_B_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_SYNC_B_PULSE_LEN;	/* SENINF_N3D_A_SYNC_B_PULSE_LEN */

typedef volatile union _SENINF_REG_N3D_A_SUB_CNT_
{
		volatile struct	/* 0x1A04013C */
		{
				FIELD  VS1_SUB_CNT                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  VS1_SUB_CNT_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SYNC_PIN_A_RESET_SEL                  :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SYNC_PIN_B_RESET_SEL                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SYNC_PIN_A_RESET_SEL_EN               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SYNC_PIN_B_RESET_SEL_EN               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VS2_SUB_CNT                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  VS2_SUB_CNT_EN                        :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_SUB_CNT;	/* SENINF_N3D_A_SUB_CNT */

typedef volatile union _SENINF_REG_N3D_A_VSYNC_CNT_
{
		volatile struct	/* 0x1A040140 */
		{
				FIELD  N3D_VSYNC_1_CNT                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  N3D_VSYNC_2_CNT                       : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_A_VSYNC_CNT;	/* SENINF_N3D_A_VSYNC_CNT */

typedef volatile union _SENINF1_REG_CTRL_
{
		volatile struct	/* 0x1A040200 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OCSI2_SW_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_SW_RST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI3_SW_RST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CTRL;	/* SENINF1_CTRL */

typedef volatile union _SENINF1_REG_CTRL_EXT_
{
		volatile struct	/* 0x1A040204 */
		{
				FIELD  SENINF_OCSI2_IP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_TESTMDL_IP_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_SCAM_IP_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_NCSI2_IP_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_CSI2_IP_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_CSI3_IP_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CTRL_EXT;	/* SENINF1_CTRL_EXT */

typedef volatile union _SENINF1_REG_ASYNC_CTRL_
{
		volatile struct	/* 0x1A040208 */
		{
				FIELD  SENINF_ASYNC_FIFO_RST                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_ASYNC_CTRL;	/* SENINF1_ASYNC_CTRL */

typedef volatile union _SENINF_REG_TG1_PH_CNT_
{
		volatile struct	/* 0x1A040600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_PH_CNT;	/* SENINF_TG1_PH_CNT */

typedef volatile union _SENINF_REG_TG1_SEN_CK_
{
		volatile struct	/* 0x1A040604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_SEN_CK;	/* SENINF_TG1_SEN_CK */

typedef volatile union _SENINF_REG_TG1_TM_CTL_
{
		volatile struct	/* 0x1A040608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_CTL;	/* SENINF_TG1_TM_CTL */

typedef volatile union _SENINF_REG_TG1_TM_SIZE_
{
		volatile struct	/* 0x1A04060C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_SIZE;	/* SENINF_TG1_TM_SIZE */

typedef volatile union _SENINF_REG_TG1_TM_CLK_
{
		volatile struct	/* 0x1A040610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_CLK;	/* SENINF_TG1_TM_CLK */

typedef volatile union _SENINF_REG_TG1_TM_STP_
{
		volatile struct	/* 0x1A040614 */
		{
				FIELD  TG1_TM_STP                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_STP;	/* SENINF_TG1_TM_STP */

typedef volatile union _MIPI_REG_RX_CON24_CSI0_
{
		volatile struct	/* 0x1A040824 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI0;	/* MIPI_RX_CON24_CSI0 */

typedef volatile union _MIPI_REG_RX_CON28_CSI0_
{
		volatile struct	/* 0x1A040828 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI0;	/* MIPI_RX_CON28_CSI0 */

typedef volatile union _MIPI_REG_RX_CON34_CSI0_
{
		volatile struct	/* 0x1A040834 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 21;		/*  1..21, 0x003FFFFE */
				FIELD  CSI0_BIST_HSDET_MUX                   :  2;		/* 22..23, 0x00C00000 */
				FIELD  CSI0_BIST_LN4_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN5_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN6_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN7_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI0;	/* MIPI_RX_CON34_CSI0 */

typedef volatile union _MIPI_REG_RX_CON38_CSI0_
{
		volatile struct	/* 0x1A040838 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_SW_RST                        :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  MIPI_RX_SW_CPHY_TX_MODE               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIPI_RX_SW_CPHY_RX_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  rg_ckphase_trio0                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rg_ckphase_trio1                      :  5;		/* 22..26, 0x07C00000 */
				FIELD  rg_ckphase_trio2                      :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI0;	/* MIPI_RX_CON38_CSI0 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI0_
{
		volatile struct	/* 0x1A04083C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI0;	/* MIPI_RX_CON3C_CSI0 */

typedef volatile union _MIPI_REG_RX_CON7C_CSI0_
{
		volatile struct	/* 0x1A04087C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON7C_CSI0;	/* MIPI_RX_CON7C_CSI0 */

typedef volatile union _MIPI_REG_RX_CON80_CSI0_
{
		volatile struct	/* 0x1A040880 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON80_CSI0;	/* MIPI_RX_CON80_CSI0 */

typedef volatile union _MIPI_REG_RX_CON84_CSI0_
{
		volatile struct	/* 0x1A040884 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON84_CSI0;	/* MIPI_RX_CON84_CSI0 */

typedef volatile union _MIPI_REG_RX_CON88_CSI0_
{
		volatile struct	/* 0x1A040888 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON88_CSI0;	/* MIPI_RX_CON88_CSI0 */

typedef volatile union _MIPI_REG_RX_CON8C_CSI0_
{
		volatile struct	/* 0x1A04088C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON8C_CSI0;	/* MIPI_RX_CON8C_CSI0 */

typedef volatile union _MIPI_REG_RX_CON90_CSI0_
{
		volatile struct	/* 0x1A040890 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON90_CSI0;	/* MIPI_RX_CON90_CSI0 */

typedef volatile union _MIPI_REG_RX_CON94_CSI0_
{
		volatile struct	/* 0x1A040894 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON94_CSI0;	/* MIPI_RX_CON94_CSI0 */

typedef volatile union _MIPI_REG_RX_CON98_CSI0_
{
		volatile struct	/* 0x1A040898 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON98_CSI0;	/* MIPI_RX_CON98_CSI0 */

typedef volatile union _MIPI_REG_RX_CONA0_CSI0_
{
		volatile struct	/* 0x1A0408A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONA0_CSI0;	/* MIPI_RX_CONA0_CSI0 */

typedef volatile union _MIPI_REG_RX_CONB0_CSI0_
{
		volatile struct	/* 0x1A0408B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB0_CSI0;	/* MIPI_RX_CONB0_CSI0 */

typedef volatile union _MIPI_REG_RX_CONB4_CSI0_
{
		volatile struct	/* 0x1A0408B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB4_CSI0;	/* MIPI_RX_CONB4_CSI0 */

typedef volatile union _MIPI_REG_RX_CONB8_CSI0_
{
		volatile struct	/* 0x1A0408B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB8_CSI0;	/* MIPI_RX_CONB8_CSI0 */

typedef volatile union _MIPI_REG_RX_CONBC_CSI0_
{
		volatile struct	/* 0x1A0408BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONBC_CSI0;	/* MIPI_RX_CONBC_CSI0 */

typedef volatile union _MIPI_REG_RX_CONC0_CSI0_
{
		volatile struct	/* 0x1A0408C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC0_CSI0;	/* MIPI_RX_CONC0_CSI0 */

typedef volatile union _MIPI_REG_RX_CONC4_CSI0_
{
		volatile struct	/* 0x1A0408C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC4_CSI0;	/* MIPI_RX_CONC4_CSI0 */

typedef volatile union _MIPI_REG_RX_CONC8_CSI0_
{
		volatile struct	/* 0x1A0408C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC8_CSI0;	/* MIPI_RX_CONC8_CSI0 */

typedef volatile union _MIPI_REG_RX_CONCC_CSI0_
{
		volatile struct	/* 0x1A0408CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONCC_CSI0;	/* MIPI_RX_CONCC_CSI0 */

typedef volatile union _MIPI_REG_RX_COND0_CSI0_
{
		volatile struct	/* 0x1A0408D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_COND0_CSI0;	/* MIPI_RX_COND0_CSI0 */

typedef volatile union _SENINF1_REG_CSI2_CTL_
{
		volatile struct	/* 0x1A040A00 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  rsv_20                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CTL;	/* SENINF1_CSI2_CTL */

typedef volatile union _SENINF1_REG_CSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A040A04 */
		{
				FIELD  CLOCK_TERM_PARAMETER                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CLOCK_SETTLE_PARAMETER                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNRC_TIMING;	/* SENINF1_CSI2_LNRC_TIMING */

typedef volatile union _SENINF1_REG_CSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A040A08 */
		{
				FIELD  DATA_TERM_PARAMETER                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DATA_SETTLE_PARAMETER                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNRD_TIMING;	/* SENINF1_CSI2_LNRD_TIMING */

typedef volatile union _SENINF1_REG_CSI2_DPCM_
{
		volatile struct	/* 0x1A040A0C */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DPCM;	/* SENINF1_CSI2_DPCM */

typedef volatile union _SENINF1_REG_CSI2_INT_EN_
{
		volatile struct	/* 0x1A040A10 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_CRC_NO_ERROR                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FE                                    :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC                       :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6                     :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT              :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT              :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT              :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT              :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INT_EN;	/* SENINF1_CSI2_INT_EN */

typedef volatile union _SENINF1_REG_CSI2_INT_STATUS_
{
		volatile struct	/* 0x1A040A14 */
		{
				FIELD  ERR_FRAME_SYNC_STA                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID_STA                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR_STA                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED_STA                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE_STA                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC_STA                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC_STA               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0_STA             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1_STA             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2_STA             :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3_STA             :  1;		/* 11..11, 0x00000800 */
				FIELD  FS_STA                                :  1;		/* 12..12, 0x00001000 */
				FIELD  LS_STA                                :  1;		/* 13..13, 0x00002000 */
				FIELD  GS_STA                                :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_STA                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0_STA                 :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1_STA                 :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2_STA                 :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3_STA                 :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4_STA                 :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5_STA                 :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC_STA                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6_STA                 :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7_STA                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT_STA          :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT_STA          :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT_STA          :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT_STA          :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INT_STATUS;	/* SENINF1_CSI2_INT_STATUS */

typedef volatile union _SENINF1_REG_CSI2_DGB_SEL_
{
		volatile struct	/* 0x1A040A18 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DEBUG_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DGB_SEL;	/* SENINF1_CSI2_DGB_SEL */

typedef volatile union _SENINF1_REG_CSI2_DBG_PORT_
{
		volatile struct	/* 0x1A040A1C */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DBG_PORT;	/* SENINF1_CSI2_DBG_PORT */

typedef volatile union _SENINF1_REG_CSI2_SPARE0_
{
		volatile struct	/* 0x1A040A20 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_SPARE0;	/* SENINF1_CSI2_SPARE0 */

typedef volatile union _SENINF1_REG_CSI2_SPARE1_
{
		volatile struct	/* 0x1A040A24 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_SPARE1;	/* SENINF1_CSI2_SPARE1 */

typedef volatile union _SENINF1_REG_CSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A040A28 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNRC_FSM;	/* SENINF1_CSI2_LNRC_FSM */

typedef volatile union _SENINF1_REG_CSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A040A2C */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNRD_FSM;	/* SENINF1_CSI2_LNRD_FSM */

typedef volatile union _SENINF1_REG_CSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A040A30 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_FRAME_LINE_NUM;	/* SENINF1_CSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF1_REG_CSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A040A34 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_GENERIC_SHORT;	/* SENINF1_CSI2_GENERIC_SHORT */

typedef volatile union _SENINF1_REG_CSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A040A38 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_HSRX_DBG;	/* SENINF1_CSI2_HSRX_DBG */

typedef volatile union _SENINF1_REG_CSI2_DI_
{
		volatile struct	/* 0x1A040A3C */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI;	/* SENINF1_CSI2_DI */

typedef volatile union _SENINF1_REG_CSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A040A40 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_HS_TRAIL;	/* SENINF1_CSI2_HS_TRAIL */

typedef volatile union _SENINF1_REG_CSI2_DI_CTRL_
{
		volatile struct	/* 0x1A040A44 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI_CTRL;	/* SENINF1_CSI2_DI_CTRL */

typedef volatile union _SENINF1_REG_CSI2_DETECT_CON1_
{
		volatile struct	/* 0x1A040A4C */
		{
				FIELD  DETECT_SYNC_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_SYNC_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SYNC_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_CON1;	/* SENINF1_CSI2_DETECT_CON1 */

typedef volatile union _SENINF1_REG_CSI2_DETECT_CON2_
{
		volatile struct	/* 0x1A040A50 */
		{
				FIELD  DETECT_ESCAPE_DISABLE                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_MASK                    :  7;		/*  1.. 7, 0x000000FE */
				FIELD  ESCAPE_WORD                           : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_CON2;	/* SENINF1_CSI2_DETECT_CON2 */

typedef volatile union _SENINF1_REG_CSI2_DETECT_CON3_
{
		volatile struct	/* 0x1A040A54 */
		{
				FIELD  DETECT_POST_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_POST_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  POST_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_CON3;	/* SENINF1_CSI2_DETECT_CON3 */

typedef volatile union _SENINF1_REG_CSI2_RLR0_CON0_
{
		volatile struct	/* 0x1A040A58 */
		{
				FIELD  RLR0_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR0_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR0_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR0_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RLR0_CON0;	/* SENINF1_CSI2_RLR0_CON0 */

typedef volatile union _SENINF1_REG_CSI2_RLR1_CON0_
{
		volatile struct	/* 0x1A040A5C */
		{
				FIELD  RLR1_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR1_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR1_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR1_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RLR1_CON0;	/* SENINF1_CSI2_RLR1_CON0 */

typedef volatile union _SENINF1_REG_CSI2_RLR2_CON0_
{
		volatile struct	/* 0x1A040A60 */
		{
				FIELD  RLR2_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR2_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR2_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR2_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RLR2_CON0;	/* SENINF1_CSI2_RLR2_CON0 */

typedef volatile union _SENINF1_REG_CSI2_RLR_CON0_
{
		volatile struct	/* 0x1A040A64 */
		{
				FIELD  RLRN_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLRN_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLRN_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLRN_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RLR_CON0;	/* SENINF1_CSI2_RLR_CON0 */

typedef volatile union _SENINF1_REG_CSI2_MUX_CON_
{
		volatile struct	/* 0x1A040A68 */
		{
				FIELD  DPHY_RX_EXTERNAL_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_TX_EXTERNAL_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPHY_RX_EXTERNAL_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RLR_PATTERN_DELAY_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  POST_PACKET_IGNORE_EN                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_MUX_CON;	/* SENINF1_CSI2_MUX_CON */

typedef volatile union _SENINF1_REG_CSI2_DETECT_DBG0_
{
		volatile struct	/* 0x1A040A6C */
		{
				FIELD  DETECT_SYNC_LANE0_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE0_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE0_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE0_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE0_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DETECT_SYNC_LANE1_ST                  :  1;		/* 12..12, 0x00001000 */
				FIELD  DETECT_ESCAPE_LANE1_ST                :  1;		/* 13..13, 0x00002000 */
				FIELD  DETECT_POST_LANE1_ST                  :  1;		/* 14..14, 0x00004000 */
				FIELD  POSITION_SYNC_LANE1_ST                :  4;		/* 15..18, 0x00078000 */
				FIELD  POSITION_ESCAPE_LANE1_ST              :  4;		/* 19..22, 0x00780000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DETECT_SYNC_LANE2_ST                  :  1;		/* 24..24, 0x01000000 */
				FIELD  DETECT_ESCAPE_LANE2_ST                :  1;		/* 25..25, 0x02000000 */
				FIELD  DETECT_POST_LANE2_ST                  :  1;		/* 26..26, 0x04000000 */
				FIELD  POSITION_SYNC_LANE2_ST                :  4;		/* 27..30, 0x78000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_DBG0;	/* SENINF1_CSI2_DETECT_DBG0 */

typedef volatile union _SENINF1_REG_CSI2_DETECT_DBG1_
{
		volatile struct	/* 0x1A040A70 */
		{
				FIELD  POSITION_ESCAPE_LANE2_ST              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_DBG1;	/* SENINF1_CSI2_DETECT_DBG1 */

typedef volatile union _SENINF1_REG_CSI2_RESYNC_MERGE_CTL_
{
		volatile struct	/* 0x1A040A74 */
		{
				FIELD  CPHY_LANE_RESYNC_CNT                  :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LANE_RESYNC_FLUSH_EN                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LANE_RESYNC_DATAOUT_OPTION            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BYPASS_LANE_RESYNC                    :  1;		/* 10..10, 0x00000400 */
				FIELD  CDPHY_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RESYNC_MERGE_CTL;	/* SENINF1_CSI2_RESYNC_MERGE_CTL */

typedef volatile union _SENINF1_REG_CSI2_CTRL_TRIO_MUX_
{
		volatile struct	/* 0x1A040A78 */
		{
				FIELD  TRIO0_MUX                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TRIO1_MUX                             :  3;		/*  3.. 5, 0x00000038 */
				FIELD  TRIO2_MUX                             :  3;		/*  6.. 8, 0x000001C0 */
				FIELD  TRIO3_MUX                             :  3;		/*  9..11, 0x00000E00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CTRL_TRIO_MUX;	/* SENINF1_CSI2_CTRL_TRIO_MUX */

typedef volatile union _SENINF1_REG_CSI2_CTRL_TRIO_CON_
{
		volatile struct	/* 0x1A040A7C */
		{
				FIELD  TRIO0_LPRX_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TRIO0_HSRX_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TRIO1_LPRX_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TRIO1_HSRX_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TRIO2_LPRX_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TRIO2_HSRX_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TRIO3_LPRX_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TRIO3_HSRX_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CTRL_TRIO_CON;	/* SENINF1_CSI2_CTRL_TRIO_CON */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY0_DBG_
{
		volatile struct	/* 0x1A040A80 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY0                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY0           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY0_DBG;	/* SENINF1_FIX_ADDR_CPHY0_DBG */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY1_DBG_
{
		volatile struct	/* 0x1A040A84 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY1                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY1           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY1_DBG;	/* SENINF1_FIX_ADDR_CPHY1_DBG */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY2_DBG_
{
		volatile struct	/* 0x1A040A88 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY2                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY2           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY2_DBG;	/* SENINF1_FIX_ADDR_CPHY2_DBG */

typedef volatile union _SENINF1_REG_FIX_ADDR_DBG_
{
		volatile struct	/* 0x1A040A8C */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_DBG;	/* SENINF1_FIX_ADDR_DBG */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG0_
{
		volatile struct	/* 0x1A040A90 */
		{
				FIELD  SYMBOL_STREAM0_CPHY0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG0;	/* SENINF1_WIRE_STATE_DECODE_CPHY0_DBG0 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG1_
{
		volatile struct	/* 0x1A040A94 */
		{
				FIELD  SYMBOL_STREAM1_CPHY0                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY0             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG1;	/* SENINF1_WIRE_STATE_DECODE_CPHY0_DBG1 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG0_
{
		volatile struct	/* 0x1A040A98 */
		{
				FIELD  SYMBOL_STREAM0_CPHY1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG0;	/* SENINF1_WIRE_STATE_DECODE_CPHY1_DBG0 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG1_
{
		volatile struct	/* 0x1A040A9C */
		{
				FIELD  SYMBOL_STREAM1_CPHY1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY1             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG1;	/* SENINF1_WIRE_STATE_DECODE_CPHY1_DBG1 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG0_
{
		volatile struct	/* 0x1A040AA0 */
		{
				FIELD  SYMBOL_STREAM0_CPHY2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG0;	/* SENINF1_WIRE_STATE_DECODE_CPHY2_DBG0 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG1_
{
		volatile struct	/* 0x1A040AA4 */
		{
				FIELD  SYMBOL_STREAM1_CPHY2                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY2             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG1;	/* SENINF1_WIRE_STATE_DECODE_CPHY2_DBG1 */

typedef volatile union _SENINF1_REG_SYNC_RESYNC_CTL_
{
		volatile struct	/* 0x1A040AA8 */
		{
				FIELD  SYNC_DETECTION_SEL                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  FLUSH_VALID                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_SYNC_RESYNC_CTL;	/* SENINF1_SYNC_RESYNC_CTL */

typedef volatile union _SENINF1_REG_POST_DETECT_CTL_
{
		volatile struct	/* 0x1A040AAC */
		{
				FIELD  POST_DETECT_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  POST_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_POST_DETECT_CTL;	/* SENINF1_POST_DETECT_CTL */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CONFIG_
{
		volatile struct	/* 0x1A040AB0 */
		{
				FIELD  INIT_STATE_DECODE                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CONFIG;	/* SENINF1_WIRE_STATE_DECODE_CONFIG */

typedef volatile union _SENINF1_REG_CSI2_CPHY_LNRD_FSM_
{
		volatile struct	/* 0x1A040AB4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TRIO0_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TRIO1_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  TRIO2_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CPHY_LNRD_FSM;	/* SENINF1_CSI2_CPHY_LNRD_FSM */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY0_DBG0_
{
		volatile struct	/* 0x1A040AB8 */
		{
				FIELD  WORD_COUNT_CPHY0_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY0_DBG0;	/* SENINF1_FIX_ADDR_CPHY0_DBG0 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY0_DBG1_
{
		volatile struct	/* 0x1A040ABC */
		{
				FIELD  WORD_COUNT_CPHY0_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY0_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY0_DBG1;	/* SENINF1_FIX_ADDR_CPHY0_DBG1 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY0_DBG2_
{
		volatile struct	/* 0x1A040AC0 */
		{
				FIELD  ERROR_RECORD_CPHY0_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY0_DBG2;	/* SENINF1_FIX_ADDR_CPHY0_DBG2 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY1_DBG0_
{
		volatile struct	/* 0x1A040AC4 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY1_DBG0;	/* SENINF1_FIX_ADDR_CPHY1_DBG0 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY1_DBG1_
{
		volatile struct	/* 0x1A040AC8 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY1_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY1_DBG1;	/* SENINF1_FIX_ADDR_CPHY1_DBG1 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY1_DBG2_
{
		volatile struct	/* 0x1A040ACC */
		{
				FIELD  ERROR_RECORD_CPHY1_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY1_DBG2;	/* SENINF1_FIX_ADDR_CPHY1_DBG2 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY2_DBG0_
{
		volatile struct	/* 0x1A040AD0 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY2_DBG0;	/* SENINF1_FIX_ADDR_CPHY2_DBG0 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY2_DBG1_
{
		volatile struct	/* 0x1A040AD4 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY2_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY2_DBG1;	/* SENINF1_FIX_ADDR_CPHY2_DBG1 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY2_DBG2_
{
		volatile struct	/* 0x1A040AD8 */
		{
				FIELD  ERROR_RECORD_CPHY2_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY2_DBG2;	/* SENINF1_FIX_ADDR_CPHY2_DBG2 */

typedef volatile union _SENINF1_REG_FIX_ADDR_DBG0_
{
		volatile struct	/* 0x1A040ADC */
		{
				FIELD  WORD_COUNT_DBG0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_DBG0;	/* SENINF1_FIX_ADDR_DBG0 */

typedef volatile union _SENINF1_REG_FIX_ADDR_DBG1_
{
		volatile struct	/* 0x1A040AE0 */
		{
				FIELD  WORD_COUNT_DBG1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_DBG0                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_DBG1;	/* SENINF1_FIX_ADDR_DBG1 */

typedef volatile union _SENINF1_REG_FIX_ADDR_DBG2_
{
		volatile struct	/* 0x1A040AE4 */
		{
				FIELD  ERROR_RECORD_DBG1                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_DBG2;	/* SENINF1_FIX_ADDR_DBG2 */

typedef volatile union _SENINF1_REG_CSI2_MODE_
{
		volatile struct	/* 0x1A040AE8 */
		{
				FIELD  csr_csi2_mode                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  csr_csi2_header_len                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  csr_cphy_di_pos                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  csr_cphy_wc_pos                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_MODE;	/* SENINF1_CSI2_MODE */


typedef volatile union _SENINF1_REG_CSI2_DI_EXT_
{
		volatile struct	/* 0x1A040AF0 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI_EXT;	/* SENINF1_CSI2_DI_EXT */

typedef volatile union _SENINF1_REG_CSI2_DI_CTRL_EXT_
{
		volatile struct	/* 0x1A040AF4 */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI_CTRL_EXT;	/* SENINF1_CSI2_DI_CTRL_EXT */

typedef volatile union _SENINF1_REG_CSI2_CPHY_LOOPBACK_
{
		volatile struct	/* 0x1A040AF8 */
		{
				FIELD  TRIGGER_SYNC_INIT                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RELEASE_SYNC_INIT                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CPHY_LOOPBACK;	/* SENINF1_CSI2_CPHY_LOOPBACK */

typedef volatile union _SENINF1_REG_CSI2_PROGSEQ_0_
{
		volatile struct	/* 0x1A040B00 */
		{
				FIELD  PROGSEQ_S0                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S1                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S2                            :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S3                            :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S4                            :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S5                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  PROGSEQ_S6                            :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  PROGSEQ_S7                            :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_PROGSEQ_0;	/* SENINF1_CSI2_PROGSEQ_0 */

typedef volatile union _SENINF1_REG_CSI2_PROGSEQ_1_
{
		volatile struct	/* 0x1A040B04 */
		{
				FIELD  PROGSEQ_S8                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S9                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S10                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S11                           :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S12                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S13                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_PROGSEQ_1;	/* SENINF1_CSI2_PROGSEQ_1 */

typedef volatile union _SENINF1_REG_CSI2_INT_EN_EXT_
{
		volatile struct	/* 0x1A040B10 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 19;		/* 12..30, 0x7FFFF000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INT_EN_EXT;	/* SENINF1_CSI2_INT_EN_EXT */

typedef volatile union _SENINF1_REG_CSI2_INT_STATUS_EXT_
{
		volatile struct	/* 0x1A040B14 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INT_STATUS_EXT;	/* SENINF1_CSI2_INT_STATUS_EXT */

typedef volatile union _SENINF1_REG_CSI2_CPHY_FIX_POINT_RST_
{
		volatile struct	/* 0x1A040B18 */
		{
				FIELD  CPHY_FIX_POINT_RST                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_FIX_POINT_RST_MODE               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CPHY_FIX_POINT_RST;	/* SENINF1_CSI2_CPHY_FIX_POINT_RST */

typedef volatile union _SENINF1_REG_CSI2_RLR3_CON0_
{
		volatile struct	/* 0x1A040B1C */
		{
				FIELD  RLR3_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR3_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR3_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR3_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_RLR3_CON0;	/* SENINF1_CSI2_RLR3_CON0 */

typedef volatile union _SENINF1_REG_CSI2_DPHY_SYNC_
{
		volatile struct	/* 0x1A040B20 */
		{
				FIELD  SYNC_SEQ_MASK_0                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_0                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DPHY_SYNC;	/* SENINF1_CSI2_DPHY_SYNC */

typedef volatile union _SENINF1_REG_CSI2_DESKEW_SYNC_
{
		volatile struct	/* 0x1A040B24 */
		{
				FIELD  SYNC_SEQ_MASK_1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_1                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DESKEW_SYNC;	/* SENINF1_CSI2_DESKEW_SYNC */

typedef volatile union _SENINF1_REG_CSI2_DETECT_DBG2_
{
		volatile struct	/* 0x1A040B28 */
		{
				FIELD  DETECT_SYNC_LANE3_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE3_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE3_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE3_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE3_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DETECT_DBG2;	/* SENINF1_CSI2_DETECT_DBG2 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY3_DBG0_
{
		volatile struct	/* 0x1A040B30 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY3_DBG0;	/* SENINF1_FIX_ADDR_CPHY3_DBG0 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY3_DBG1_
{
		volatile struct	/* 0x1A040B34 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY3_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY3_DBG1;	/* SENINF1_FIX_ADDR_CPHY3_DBG1 */

typedef volatile union _SENINF1_REG_FIX_ADDR_CPHY3_DBG2_
{
		volatile struct	/* 0x1A040B38 */
		{
				FIELD  ERROR_RECORD_CPHY3_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_FIX_ADDR_CPHY3_DBG2;	/* SENINF1_FIX_ADDR_CPHY3_DBG2 */

typedef volatile union _SENINF1_REG_CSI2_DI_EXT_2_
{
		volatile struct	/* 0x1A040B3C */
		{
				FIELD  VC6                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT6                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC7                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT7                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI_EXT_2;	/* SENINF1_CSI2_DI_EXT_2 */

typedef volatile union _SENINF1_REG_CSI2_DI_CTRL_EXT_2_
{
		volatile struct	/* 0x1A040B40 */
		{
				FIELD  VC6_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT6_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC7_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT7_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DI_CTRL_EXT_2;	/* SENINF1_CSI2_DI_CTRL_EXT_2 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG0_
{
		volatile struct	/* 0x1A040B44 */
		{
				FIELD  SYMBOL_STREAM0_CPHY3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG0;	/* SENINF1_WIRE_STATE_DECODE_CPHY3_DBG0 */

typedef volatile union _SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG1_
{
		volatile struct	/* 0x1A040B48 */
		{
				FIELD  SYMBOL_STREAM1_CPHY3                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY3             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG1;	/* SENINF1_WIRE_STATE_DECODE_CPHY3_DBG1 */

typedef volatile union _SENINF1_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A040D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_CTRL;	/* SENINF1_MUX_CTRL */

typedef volatile union _SENINF1_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A040D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_INTEN;	/* SENINF1_MUX_INTEN */

typedef volatile union _SENINF1_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A040D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_INTSTA;	/* SENINF1_MUX_INTSTA */

typedef volatile union _SENINF1_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A040D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_SIZE;	/* SENINF1_MUX_SIZE */

typedef volatile union _SENINF1_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A040D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_1;	/* SENINF1_MUX_DEBUG_1 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A040D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_2;	/* SENINF1_MUX_DEBUG_2 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A040D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_3;	/* SENINF1_MUX_DEBUG_3 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A040D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_4;	/* SENINF1_MUX_DEBUG_4 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A040D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_5;	/* SENINF1_MUX_DEBUG_5 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A040D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_6;	/* SENINF1_MUX_DEBUG_6 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A040D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_7;	/* SENINF1_MUX_DEBUG_7 */

typedef volatile union _SENINF1_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A040D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_SPARE;	/* SENINF1_MUX_SPARE */

typedef volatile union _SENINF1_REG_MUX_DATA_
{
		volatile struct	/* 0x1A040D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DATA;	/* SENINF1_MUX_DATA */

typedef volatile union _SENINF1_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A040D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DATA_CNT;	/* SENINF1_MUX_DATA_CNT */

typedef volatile union _SENINF1_REG_MUX_CROP_
{
		volatile struct	/* 0x1A040D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_CROP;	/* SENINF1_MUX_CROP */

typedef volatile union _SENINF1_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A040D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_CTRL_EXT;	/* SENINF1_MUX_CTRL_EXT */

typedef volatile union _SENINF_REG_N3D_B_CTL_
{
		volatile struct	/* 0x1A041100 */
		{
				FIELD  MODE                                  :  2;		/*  0.. 1, 0x00000003 */
				FIELD  I2C1_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  I2C2_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  I2C1_INT_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  I2C2_INT_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  N3D_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  W1CLR                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DIFF_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DDBG_SEL                              :  3;		/*  9..11, 0x00000E00 */
				FIELD  MODE1_DBG                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEN1_TIM_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  SEN2_TIM_EN                           :  1;		/* 17..17, 0x00020000 */
				FIELD  SEN1_OV_VS_INT_EN                     :  1;		/* 18..18, 0x00040000 */
				FIELD  SEN2_OV_VS_INT_EN                     :  1;		/* 19..19, 0x00080000 */
				FIELD  HW_SYNC_MODE                          :  1;		/* 20..20, 0x00100000 */
				FIELD  VALID_TG_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SYNC_PIN_A_EN                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SYNC_PIN_A_POLARITY                   :  1;		/* 23..23, 0x00800000 */
				FIELD  SYNC_PIN_B_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  SYNC_PIN_B_POLARITY                   :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_CTL;	/* SENINF_N3D_B_CTL */

typedef volatile union _SENINF_REG_N3D_B_POS_
{
		volatile struct	/* 0x1A041104 */
		{
				FIELD  N3D_POS                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_POS;	/* SENINF_N3D_B_POS */

typedef volatile union _SENINF_REG_N3D_B_TRIG_
{
		volatile struct	/* 0x1A041108 */
		{
				FIELD  I2CA_TRIG                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2CB_TRIG                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_TRIG;	/* SENINF_N3D_B_TRIG */

typedef volatile union _SENINF_REG_N3D_B_INT_
{
		volatile struct	/* 0x1A04110C */
		{
				FIELD  I2C1_INT                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2C2_INT                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DIFF_INT                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SEN1_OV_VS_INT                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SEN2_OV_VS_INT                        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_INT;	/* SENINF_N3D_B_INT */

typedef volatile union _SENINF_REG_N3D_B_CNT0_
{
		volatile struct	/* 0x1A041110 */
		{
				FIELD  N3D_CNT0                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_CNT0;	/* SENINF_N3D_B_CNT0 */

typedef volatile union _SENINF_REG_N3D_B_CNT1_
{
		volatile struct	/* 0x1A041114 */
		{
				FIELD  N3D_CNT1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_CNT1;	/* SENINF_N3D_B_CNT1 */

typedef volatile union _SENINF_REG_N3D_B_DBG_
{
		volatile struct	/* 0x1A041118 */
		{
				FIELD  N3D_DBG                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_DBG;	/* SENINF_N3D_B_DBG */

typedef volatile union _SENINF_REG_N3D_B_DIFF_THR_
{
		volatile struct	/* 0x1A04111C */
		{
				FIELD  N3D_DIFF_THR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_DIFF_THR;	/* SENINF_N3D_B_DIFF_THR */

typedef volatile union _SENINF_REG_N3D_B_DIFF_CNT_
{
		volatile struct	/* 0x1A041120 */
		{
				FIELD  N3D_DIFF_CNT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_DIFF_CNT;	/* SENINF_N3D_B_DIFF_CNT */

typedef volatile union _SENINF_REG_N3D_B_DBG_1_
{
		volatile struct	/* 0x1A041124 */
		{
				FIELD  N3D_DBG_1                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_DBG_1;	/* SENINF_N3D_B_DBG_1 */

typedef volatile union _SENINF_REG_N3D_B_VALID_TG_CNT_
{
		volatile struct	/* 0x1A041128 */
		{
				FIELD  N3D_VALID_TG_CNT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_VALID_TG_CNT;	/* SENINF_N3D_B_VALID_TG_CNT */

typedef volatile union _SENINF_REG_N3D_B_SYNC_A_PERIOD_
{
		volatile struct	/* 0x1A04112C */
		{
				FIELD  N3D_SYNC_A_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_SYNC_A_PERIOD;	/* SENINF_N3D_B_SYNC_A_PERIOD */

typedef volatile union _SENINF_REG_N3D_B_SYNC_B_PERIOD_
{
		volatile struct	/* 0x1A041130 */
		{
				FIELD  N3D_SYNC_B_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_SYNC_B_PERIOD;	/* SENINF_N3D_B_SYNC_B_PERIOD */

typedef volatile union _SENINF_REG_N3D_B_SYNC_A_PULSE_LEN_
{
		volatile struct	/* 0x1A041134 */
		{
				FIELD  N3D_SYNC_A_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_SYNC_A_PULSE_LEN;	/* SENINF_N3D_B_SYNC_A_PULSE_LEN */

typedef volatile union _SENINF_REG_N3D_B_SYNC_B_PULSE_LEN_
{
		volatile struct	/* 0x1A041138 */
		{
				FIELD  N3D_SYNC_B_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_SYNC_B_PULSE_LEN;	/* SENINF_N3D_B_SYNC_B_PULSE_LEN */

typedef volatile union _SENINF_REG_N3D_B_SUB_CNT_
{
		volatile struct	/* 0x1A04113C */
		{
				FIELD  VS1_SUB_CNT                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  VS1_SUB_CNT_EN                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SYNC_PIN_A_RESET_SEL                  :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SYNC_PIN_B_RESET_SEL                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SYNC_PIN_A_RESET_SEL_EN               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SYNC_PIN_B_RESET_SEL_EN               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VS2_SUB_CNT                           :  6;		/* 16..21, 0x003F0000 */
				FIELD  VS2_SUB_CNT_EN                        :  1;		/* 22..22, 0x00400000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_SUB_CNT;	/* SENINF_N3D_B_SUB_CNT */

typedef volatile union _SENINF_REG_N3D_B_VSYNC_CNT_
{
		volatile struct	/* 0x1A041140 */
		{
				FIELD  N3D_VSYNC_1_CNT                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_B_VSYNC_CNT;	/* SENINF_N3D_B_VSYNC_CNT */

typedef volatile union _SENINF2_REG_CTRL_
{
		volatile struct	/* 0x1A041200 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OCSI2_SW_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_SW_RST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI3_SW_RST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CTRL;	/* SENINF2_CTRL */

typedef volatile union _SENINF2_REG_CTRL_EXT_
{
		volatile struct	/* 0x1A041204 */
		{
				FIELD  SENINF_OCSI2_IP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_TESTMDL_IP_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_SCAM_IP_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_NCSI2_IP_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_CSI2_IP_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_CSI3_IP_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CTRL_EXT;	/* SENINF2_CTRL_EXT */

typedef volatile union _SENINF2_REG_ASYNC_CTRL_
{
		volatile struct	/* 0x1A041208 */
		{
				FIELD  SENINF_ASYNC_FIFO_RST                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_ASYNC_CTRL;	/* SENINF2_ASYNC_CTRL */

typedef volatile union _SENINF_REG_TG2_PH_CNT_
{
		volatile struct	/* 0x1A041600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_PH_CNT;	/* SENINF_TG2_PH_CNT */

typedef volatile union _SENINF_REG_TG2_SEN_CK_
{
		volatile struct	/* 0x1A041604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_SEN_CK;	/* SENINF_TG2_SEN_CK */

typedef volatile union _SENINF_REG_TG2_TM_CTL_
{
		volatile struct	/* 0x1A041608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_CTL;	/* SENINF_TG2_TM_CTL */

typedef volatile union _SENINF_REG_TG2_TM_SIZE_
{
		volatile struct	/* 0x1A04160C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_SIZE;	/* SENINF_TG2_TM_SIZE */

typedef volatile union _SENINF_REG_TG2_TM_CLK_
{
		volatile struct	/* 0x1A041610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_CLK;	/* SENINF_TG2_TM_CLK */

typedef volatile union _SENINF_REG_TG2_TM_STP_
{
		volatile struct	/* 0x1A041614 */
		{
				FIELD  TG1_TM_STP                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_STP;	/* SENINF_TG2_TM_STP */

typedef volatile union _MIPI_REG_RX_CON24_CSI1_
{
		volatile struct	/* 0x1A041824 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI1;	/* MIPI_RX_CON24_CSI1 */

typedef volatile union _MIPI_REG_RX_CON28_CSI1_
{
		volatile struct	/* 0x1A041828 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI1;	/* MIPI_RX_CON28_CSI1 */

typedef volatile union _MIPI_REG_RX_CON34_CSI1_
{
		volatile struct	/* 0x1A041834 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 21;		/*  1..21, 0x003FFFFE */
				FIELD  CSI0_BIST_HSDET_MUX                   :  2;		/* 22..23, 0x00C00000 */
				FIELD  CSI0_BIST_LN4_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN5_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN6_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN7_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI1;	/* MIPI_RX_CON34_CSI1 */

typedef volatile union _MIPI_REG_RX_CON38_CSI1_
{
		volatile struct	/* 0x1A041838 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_SW_RST                        :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  MIPI_RX_SW_CPHY_TX_MODE               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIPI_RX_SW_CPHY_RX_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  rg_ckphase_trio0                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rg_ckphase_trio1                      :  5;		/* 22..26, 0x07C00000 */
				FIELD  rg_ckphase_trio2                      :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI1;	/* MIPI_RX_CON38_CSI1 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI1_
{
		volatile struct	/* 0x1A04183C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI1;	/* MIPI_RX_CON3C_CSI1 */

typedef volatile union _MIPI_REG_RX_CON7C_CSI1_
{
		volatile struct	/* 0x1A04187C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON7C_CSI1;	/* MIPI_RX_CON7C_CSI1 */

typedef volatile union _MIPI_REG_RX_CON80_CSI1_
{
		volatile struct	/* 0x1A041880 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON80_CSI1;	/* MIPI_RX_CON80_CSI1 */

typedef volatile union _MIPI_REG_RX_CON84_CSI1_
{
		volatile struct	/* 0x1A041884 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON84_CSI1;	/* MIPI_RX_CON84_CSI1 */

typedef volatile union _MIPI_REG_RX_CON88_CSI1_
{
		volatile struct	/* 0x1A041888 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON88_CSI1;	/* MIPI_RX_CON88_CSI1 */

typedef volatile union _MIPI_REG_RX_CON8C_CSI1_
{
		volatile struct	/* 0x1A04188C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON8C_CSI1;	/* MIPI_RX_CON8C_CSI1 */

typedef volatile union _MIPI_REG_RX_CON90_CSI1_
{
		volatile struct	/* 0x1A041890 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON90_CSI1;	/* MIPI_RX_CON90_CSI1 */

typedef volatile union _MIPI_REG_RX_CON94_CSI1_
{
		volatile struct	/* 0x1A041894 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON94_CSI1;	/* MIPI_RX_CON94_CSI1 */

typedef volatile union _MIPI_REG_RX_CON98_CSI1_
{
		volatile struct	/* 0x1A041898 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON98_CSI1;	/* MIPI_RX_CON98_CSI1 */

typedef volatile union _MIPI_REG_RX_CONA0_CSI1_
{
		volatile struct	/* 0x1A0418A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONA0_CSI1;	/* MIPI_RX_CONA0_CSI1 */

typedef volatile union _MIPI_REG_RX_CONB0_CSI1_
{
		volatile struct	/* 0x1A0418B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB0_CSI1;	/* MIPI_RX_CONB0_CSI1 */

typedef volatile union _MIPI_REG_RX_CONB4_CSI1_
{
		volatile struct	/* 0x1A0418B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB4_CSI1;	/* MIPI_RX_CONB4_CSI1 */

typedef volatile union _MIPI_REG_RX_CONB8_CSI1_
{
		volatile struct	/* 0x1A0418B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB8_CSI1;	/* MIPI_RX_CONB8_CSI1 */

typedef volatile union _MIPI_REG_RX_CONBC_CSI1_
{
		volatile struct	/* 0x1A0418BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONBC_CSI1;	/* MIPI_RX_CONBC_CSI1 */

typedef volatile union _MIPI_REG_RX_CONC0_CSI1_
{
		volatile struct	/* 0x1A0418C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC0_CSI1;	/* MIPI_RX_CONC0_CSI1 */

typedef volatile union _MIPI_REG_RX_CONC4_CSI1_
{
		volatile struct	/* 0x1A0418C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC4_CSI1;	/* MIPI_RX_CONC4_CSI1 */

typedef volatile union _MIPI_REG_RX_CONC8_CSI1_
{
		volatile struct	/* 0x1A0418C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC8_CSI1;	/* MIPI_RX_CONC8_CSI1 */

typedef volatile union _MIPI_REG_RX_CONCC_CSI1_
{
		volatile struct	/* 0x1A0418CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONCC_CSI1;	/* MIPI_RX_CONCC_CSI1 */

typedef volatile union _MIPI_REG_RX_COND0_CSI1_
{
		volatile struct	/* 0x1A0418D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_COND0_CSI1;	/* MIPI_RX_COND0_CSI1 */

typedef volatile union _SENINF2_REG_CSI2_CTL_
{
		volatile struct	/* 0x1A041A00 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  rsv_20                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CTL;	/* SENINF2_CSI2_CTL */

typedef volatile union _SENINF2_REG_CSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A041A04 */
		{
				FIELD  CLOCK_TERM_PARAMETER                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CLOCK_SETTLE_PARAMETER                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNRC_TIMING;	/* SENINF2_CSI2_LNRC_TIMING */

typedef volatile union _SENINF2_REG_CSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A041A08 */
		{
				FIELD  DATA_TERM_PARAMETER                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DATA_SETTLE_PARAMETER                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNRD_TIMING;	/* SENINF2_CSI2_LNRD_TIMING */

typedef volatile union _SENINF2_REG_CSI2_DPCM_
{
		volatile struct	/* 0x1A041A0C */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DPCM;	/* SENINF2_CSI2_DPCM */

typedef volatile union _SENINF2_REG_CSI2_INT_EN_
{
		volatile struct	/* 0x1A041A10 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_CRC_NO_ERROR                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FE                                    :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC                       :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6                     :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT              :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT              :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT              :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT              :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INT_EN;	/* SENINF2_CSI2_INT_EN */

typedef volatile union _SENINF2_REG_CSI2_INT_STATUS_
{
		volatile struct	/* 0x1A041A14 */
		{
				FIELD  ERR_FRAME_SYNC_STA                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID_STA                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR_STA                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED_STA                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE_STA                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC_STA                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC_STA               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0_STA             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1_STA             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2_STA             :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3_STA             :  1;		/* 11..11, 0x00000800 */
				FIELD  FS_STA                                :  1;		/* 12..12, 0x00001000 */
				FIELD  LS_STA                                :  1;		/* 13..13, 0x00002000 */
				FIELD  GS_STA                                :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_STA                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0_STA                 :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1_STA                 :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2_STA                 :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3_STA                 :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4_STA                 :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5_STA                 :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC_STA                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6_STA                 :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7_STA                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT_STA          :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT_STA          :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT_STA          :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT_STA          :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INT_STATUS;	/* SENINF2_CSI2_INT_STATUS */

typedef volatile union _SENINF2_REG_CSI2_DGB_SEL_
{
		volatile struct	/* 0x1A041A18 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DEBUG_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DGB_SEL;	/* SENINF2_CSI2_DGB_SEL */

typedef volatile union _SENINF2_REG_CSI2_DBG_PORT_
{
		volatile struct	/* 0x1A041A1C */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DBG_PORT;	/* SENINF2_CSI2_DBG_PORT */

typedef volatile union _SENINF2_REG_CSI2_SPARE0_
{
		volatile struct	/* 0x1A041A20 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_SPARE0;	/* SENINF2_CSI2_SPARE0 */

typedef volatile union _SENINF2_REG_CSI2_SPARE1_
{
		volatile struct	/* 0x1A041A24 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_SPARE1;	/* SENINF2_CSI2_SPARE1 */

typedef volatile union _SENINF2_REG_CSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A041A28 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNRC_FSM;	/* SENINF2_CSI2_LNRC_FSM */

typedef volatile union _SENINF2_REG_CSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A041A2C */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNRD_FSM;	/* SENINF2_CSI2_LNRD_FSM */

typedef volatile union _SENINF2_REG_CSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A041A30 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_FRAME_LINE_NUM;	/* SENINF2_CSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF2_REG_CSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A041A34 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_GENERIC_SHORT;	/* SENINF2_CSI2_GENERIC_SHORT */

typedef volatile union _SENINF2_REG_CSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A041A38 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_HSRX_DBG;	/* SENINF2_CSI2_HSRX_DBG */

typedef volatile union _SENINF2_REG_CSI2_DI_
{
		volatile struct	/* 0x1A041A3C */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI;	/* SENINF2_CSI2_DI */

typedef volatile union _SENINF2_REG_CSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A041A40 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_HS_TRAIL;	/* SENINF2_CSI2_HS_TRAIL */

typedef volatile union _SENINF2_REG_CSI2_DI_CTRL_
{
		volatile struct	/* 0x1A041A44 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI_CTRL;	/* SENINF2_CSI2_DI_CTRL */

typedef volatile union _SENINF2_REG_CSI2_DETECT_CON1_
{
		volatile struct	/* 0x1A041A4C */
		{
				FIELD  DETECT_SYNC_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_SYNC_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SYNC_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_CON1;	/* SENINF2_CSI2_DETECT_CON1 */

typedef volatile union _SENINF2_REG_CSI2_DETECT_CON2_
{
		volatile struct	/* 0x1A041A50 */
		{
				FIELD  DETECT_ESCAPE_DISABLE                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_MASK                    :  7;		/*  1.. 7, 0x000000FE */
				FIELD  ESCAPE_WORD                           : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_CON2;	/* SENINF2_CSI2_DETECT_CON2 */

typedef volatile union _SENINF2_REG_CSI2_DETECT_CON3_
{
		volatile struct	/* 0x1A041A54 */
		{
				FIELD  DETECT_POST_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_POST_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  POST_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_CON3;	/* SENINF2_CSI2_DETECT_CON3 */

typedef volatile union _SENINF2_REG_CSI2_RLR0_CON0_
{
		volatile struct	/* 0x1A041A58 */
		{
				FIELD  RLR0_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR0_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR0_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR0_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RLR0_CON0;	/* SENINF2_CSI2_RLR0_CON0 */

typedef volatile union _SENINF2_REG_CSI2_RLR1_CON0_
{
		volatile struct	/* 0x1A041A5C */
		{
				FIELD  RLR1_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR1_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR1_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR1_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RLR1_CON0;	/* SENINF2_CSI2_RLR1_CON0 */

typedef volatile union _SENINF2_REG_CSI2_RLR2_CON0_
{
		volatile struct	/* 0x1A041A60 */
		{
				FIELD  RLR2_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR2_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR2_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR2_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RLR2_CON0;	/* SENINF2_CSI2_RLR2_CON0 */

typedef volatile union _SENINF2_REG_CSI2_RLR_CON0_
{
		volatile struct	/* 0x1A041A64 */
		{
				FIELD  RLRN_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLRN_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLRN_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLRN_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RLR_CON0;	/* SENINF2_CSI2_RLR_CON0 */

typedef volatile union _SENINF2_REG_CSI2_MUX_CON_
{
		volatile struct	/* 0x1A041A68 */
		{
				FIELD  DPHY_RX_EXTERNAL_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_TX_EXTERNAL_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPHY_RX_EXTERNAL_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RLR_PATTERN_DELAY_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  POST_PACKET_IGNORE_EN                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_MUX_CON;	/* SENINF2_CSI2_MUX_CON */

typedef volatile union _SENINF2_REG_CSI2_DETECT_DBG0_
{
		volatile struct	/* 0x1A041A6C */
		{
				FIELD  DETECT_SYNC_LANE0_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE0_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE0_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE0_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE0_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DETECT_SYNC_LANE1_ST                  :  1;		/* 12..12, 0x00001000 */
				FIELD  DETECT_ESCAPE_LANE1_ST                :  1;		/* 13..13, 0x00002000 */
				FIELD  DETECT_POST_LANE1_ST                  :  1;		/* 14..14, 0x00004000 */
				FIELD  POSITION_SYNC_LANE1_ST                :  4;		/* 15..18, 0x00078000 */
				FIELD  POSITION_ESCAPE_LANE1_ST              :  4;		/* 19..22, 0x00780000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DETECT_SYNC_LANE2_ST                  :  1;		/* 24..24, 0x01000000 */
				FIELD  DETECT_ESCAPE_LANE2_ST                :  1;		/* 25..25, 0x02000000 */
				FIELD  DETECT_POST_LANE2_ST                  :  1;		/* 26..26, 0x04000000 */
				FIELD  POSITION_SYNC_LANE2_ST                :  4;		/* 27..30, 0x78000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_DBG0;	/* SENINF2_CSI2_DETECT_DBG0 */

typedef volatile union _SENINF2_REG_CSI2_DETECT_DBG1_
{
		volatile struct	/* 0x1A041A70 */
		{
				FIELD  POSITION_ESCAPE_LANE2_ST              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_DBG1;	/* SENINF2_CSI2_DETECT_DBG1 */


typedef volatile union _SENINF2_REG_CSI2_RESYNC_MERGE_CTL_
{
		volatile struct	/* 0x1A041A74 */
		{
				FIELD  CPHY_LANE_RESYNC_CNT                  :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LANE_RESYNC_FLUSH_EN                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LANE_RESYNC_DATAOUT_OPTION            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BYPASS_LANE_RESYNC                    :  1;		/* 10..10, 0x00000400 */
				FIELD  CDPHY_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RESYNC_MERGE_CTL;	/* SENINF2_CSI2_RESYNC_MERGE_CTL */

typedef volatile union _SENINF2_REG_CSI2_CTRL_TRIO_MUX_
{
		volatile struct	/* 0x1A041A78 */
		{
				FIELD  TRIO0_MUX                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TRIO1_MUX                             :  3;		/*  3.. 5, 0x00000038 */
				FIELD  TRIO2_MUX                             :  3;		/*  6.. 8, 0x000001C0 */
				FIELD  TRIO3_MUX                             :  3;		/*  9..11, 0x00000E00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CTRL_TRIO_MUX;	/* SENINF2_CSI2_CTRL_TRIO_MUX */

typedef volatile union _SENINF2_REG_CSI2_CTRL_TRIO_CON_
{
		volatile struct	/* 0x1A041A7C */
		{
				FIELD  TRIO0_LPRX_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TRIO0_HSRX_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TRIO1_LPRX_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TRIO1_HSRX_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TRIO2_LPRX_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TRIO2_HSRX_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TRIO3_LPRX_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TRIO3_HSRX_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CTRL_TRIO_CON;	/* SENINF2_CSI2_CTRL_TRIO_CON */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY0_DBG_
{
		volatile struct	/* 0x1A041A80 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY0                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY0           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY0_DBG;	/* SENINF2_FIX_ADDR_CPHY0_DBG */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY1_DBG_
{
		volatile struct	/* 0x1A041A84 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY1                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY1           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY1_DBG;	/* SENINF2_FIX_ADDR_CPHY1_DBG */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY2_DBG_
{
		volatile struct	/* 0x1A041A88 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY2                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY2           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY2_DBG;	/* SENINF2_FIX_ADDR_CPHY2_DBG */

typedef volatile union _SENINF2_REG_FIX_ADDR_DBG_
{
		volatile struct	/* 0x1A041A8C */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_DBG;	/* SENINF2_FIX_ADDR_DBG */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG0_
{
		volatile struct	/* 0x1A041A90 */
		{
				FIELD  SYMBOL_STREAM0_CPHY0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG0;	/* SENINF2_WIRE_STATE_DECODE_CPHY0_DBG0 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG1_
{
		volatile struct	/* 0x1A041A94 */
		{
				FIELD  SYMBOL_STREAM1_CPHY0                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY0             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG1;	/* SENINF2_WIRE_STATE_DECODE_CPHY0_DBG1 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG0_
{
		volatile struct	/* 0x1A041A98 */
		{
				FIELD  SYMBOL_STREAM0_CPHY1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG0;	/* SENINF2_WIRE_STATE_DECODE_CPHY1_DBG0 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG1_
{
		volatile struct	/* 0x1A041A9C */
		{
				FIELD  SYMBOL_STREAM1_CPHY1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY1             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG1;	/* SENINF2_WIRE_STATE_DECODE_CPHY1_DBG1 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG0_
{
		volatile struct	/* 0x1A041AA0 */
		{
				FIELD  SYMBOL_STREAM0_CPHY2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG0;	/* SENINF2_WIRE_STATE_DECODE_CPHY2_DBG0 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG1_
{
		volatile struct	/* 0x1A041AA4 */
		{
				FIELD  SYMBOL_STREAM1_CPHY2                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY2             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG1;	/* SENINF2_WIRE_STATE_DECODE_CPHY2_DBG1 */

typedef volatile union _SENINF2_REG_SYNC_RESYNC_CTL_
{
		volatile struct	/* 0x1A041AA8 */
		{
				FIELD  SYNC_DETECTION_SEL                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  FLUSH_VALID                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_SYNC_RESYNC_CTL;	/* SENINF2_SYNC_RESYNC_CTL */

typedef volatile union _SENINF2_REG_POST_DETECT_CTL_
{
		volatile struct	/* 0x1A041AAC */
		{
				FIELD  POST_DETECT_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  POST_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_POST_DETECT_CTL;	/* SENINF2_POST_DETECT_CTL */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CONFIG_
{
		volatile struct	/* 0x1A041AB0 */
		{
				FIELD  INIT_STATE_DECODE                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CONFIG;	/* SENINF2_WIRE_STATE_DECODE_CONFIG */

typedef volatile union _SENINF2_REG_CSI2_CPHY_LNRD_FSM_
{
		volatile struct	/* 0x1A041AB4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TRIO0_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TRIO1_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  TRIO2_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CPHY_LNRD_FSM;	/* SENINF2_CSI2_CPHY_LNRD_FSM */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY0_DBG0_
{
		volatile struct	/* 0x1A041AB8 */
		{
				FIELD  WORD_COUNT_CPHY0_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY0_DBG0;	/* SENINF2_FIX_ADDR_CPHY0_DBG0 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY0_DBG1_
{
		volatile struct	/* 0x1A041ABC */
		{
				FIELD  WORD_COUNT_CPHY0_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY0_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY0_DBG1;	/* SENINF2_FIX_ADDR_CPHY0_DBG1 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY0_DBG2_
{
		volatile struct	/* 0x1A041AC0 */
		{
				FIELD  ERROR_RECORD_CPHY0_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY0_DBG2;	/* SENINF2_FIX_ADDR_CPHY0_DBG2 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY1_DBG0_
{
		volatile struct	/* 0x1A041AC4 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY1_DBG0;	/* SENINF2_FIX_ADDR_CPHY1_DBG0 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY1_DBG1_
{
		volatile struct	/* 0x1A041AC8 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY1_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY1_DBG1;	/* SENINF2_FIX_ADDR_CPHY1_DBG1 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY1_DBG2_
{
		volatile struct	/* 0x1A041ACC */
		{
				FIELD  ERROR_RECORD_CPHY1_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY1_DBG2;	/* SENINF2_FIX_ADDR_CPHY1_DBG2 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY2_DBG0_
{
		volatile struct	/* 0x1A041AD0 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY2_DBG0;	/* SENINF2_FIX_ADDR_CPHY2_DBG0 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY2_DBG1_
{
		volatile struct	/* 0x1A041AD4 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY2_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY2_DBG1;	/* SENINF2_FIX_ADDR_CPHY2_DBG1 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY2_DBG2_
{
		volatile struct	/* 0x1A041AD8 */
		{
				FIELD  ERROR_RECORD_CPHY2_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY2_DBG2;	/* SENINF2_FIX_ADDR_CPHY2_DBG2 */

typedef volatile union _SENINF2_REG_FIX_ADDR_DBG0_
{
		volatile struct	/* 0x1A041ADC */
		{
				FIELD  WORD_COUNT_DBG0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_DBG0;	/* SENINF2_FIX_ADDR_DBG0 */

typedef volatile union _SENINF2_REG_FIX_ADDR_DBG1_
{
		volatile struct	/* 0x1A041AE0 */
		{
				FIELD  WORD_COUNT_DBG1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_DBG0                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_DBG1;	/* SENINF2_FIX_ADDR_DBG1 */

typedef volatile union _SENINF2_REG_FIX_ADDR_DBG2_
{
		volatile struct	/* 0x1A041AE4 */
		{
				FIELD  ERROR_RECORD_DBG1                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_DBG2;	/* SENINF2_FIX_ADDR_DBG2 */

typedef volatile union _SENINF2_REG_CSI2_MODE_
{
		volatile struct	/* 0x1A041AE8 */
		{
				FIELD  csr_csi2_mode                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  csr_csi2_header_len                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  csr_cphy_di_pos                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  csr_cphy_wc_pos                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_MODE;	/* SENINF2_CSI2_MODE */

typedef volatile union _SENINF2_REG_CSI2_DI_EXT_
{
		volatile struct	/* 0x1A041AF0 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI_EXT;	/* SENINF2_CSI2_DI_EXT */

typedef volatile union _SENINF2_REG_CSI2_DI_CTRL_EXT_
{
		volatile struct	/* 0x1A041AF4 */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI_CTRL_EXT;	/* SENINF2_CSI2_DI_CTRL_EXT */

typedef volatile union _SENINF2_REG_CSI2_CPHY_LOOPBACK_
{
		volatile struct	/* 0x1A041AF8 */
		{
				FIELD  TRIGGER_SYNC_INIT                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RELEASE_SYNC_INIT                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CPHY_LOOPBACK;	/* SENINF2_CSI2_CPHY_LOOPBACK */

typedef volatile union _SENINF2_REG_CSI2_PROGSEQ_0_
{
		volatile struct	/* 0x1A041B00 */
		{
				FIELD  PROGSEQ_S0                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S1                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S2                            :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S3                            :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S4                            :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S5                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  PROGSEQ_S6                            :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  PROGSEQ_S7                            :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_PROGSEQ_0;	/* SENINF2_CSI2_PROGSEQ_0 */

typedef volatile union _SENINF2_REG_CSI2_PROGSEQ_1_
{
		volatile struct	/* 0x1A041B04 */
		{
				FIELD  PROGSEQ_S8                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S9                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S10                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S11                           :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S12                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S13                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_PROGSEQ_1;	/* SENINF2_CSI2_PROGSEQ_1 */

typedef volatile union _SENINF2_REG_CSI2_INT_EN_EXT_
{
		volatile struct	/* 0x1A041B10 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 19;		/* 12..30, 0x7FFFF000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INT_EN_EXT;	/* SENINF2_CSI2_INT_EN_EXT */

typedef volatile union _SENINF2_REG_CSI2_INT_STATUS_EXT_
{
		volatile struct	/* 0x1A041B14 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INT_STATUS_EXT;	/* SENINF2_CSI2_INT_STATUS_EXT */

typedef volatile union _SENINF2_REG_CSI2_CPHY_FIX_POINT_RST_
{
		volatile struct	/* 0x1A041B18 */
		{
				FIELD  CPHY_FIX_POINT_RST                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_FIX_POINT_RST_MODE               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CPHY_FIX_POINT_RST;	/* SENINF2_CSI2_CPHY_FIX_POINT_RST */

typedef volatile union _SENINF2_REG_CSI2_RLR3_CON0_
{
		volatile struct	/* 0x1A041B1C */
		{
				FIELD  RLR3_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR3_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR3_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR3_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_RLR3_CON0;	/* SENINF2_CSI2_RLR3_CON0 */


typedef volatile union _SENINF2_REG_CSI2_DPHY_SYNC_
{
		volatile struct	/* 0x1A041B20 */
		{
				FIELD  SYNC_SEQ_MASK_0                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_0                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DPHY_SYNC;	/* SENINF2_CSI2_DPHY_SYNC */

typedef volatile union _SENINF2_REG_CSI2_DESKEW_SYNC_
{
		volatile struct	/* 0x1A041B24 */
		{
				FIELD  SYNC_SEQ_MASK_1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_1                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DESKEW_SYNC;	/* SENINF2_CSI2_DESKEW_SYNC */

typedef volatile union _SENINF2_REG_CSI2_DETECT_DBG2_
{
		volatile struct	/* 0x1A041B28 */
		{
				FIELD  DETECT_SYNC_LANE3_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE3_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE3_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE3_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE3_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DETECT_DBG2;	/* SENINF2_CSI2_DETECT_DBG2 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY3_DBG0_
{
		volatile struct	/* 0x1A041B30 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY3_DBG0;	/* SENINF2_FIX_ADDR_CPHY3_DBG0 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY3_DBG1_
{
		volatile struct	/* 0x1A041B34 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY3_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY3_DBG1;	/* SENINF2_FIX_ADDR_CPHY3_DBG1 */

typedef volatile union _SENINF2_REG_FIX_ADDR_CPHY3_DBG2_
{
		volatile struct	/* 0x1A041B38 */
		{
				FIELD  ERROR_RECORD_CPHY3_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_FIX_ADDR_CPHY3_DBG2;	/* SENINF2_FIX_ADDR_CPHY3_DBG2 */

typedef volatile union _SENINF2_REG_CSI2_DI_EXT_2_
{
		volatile struct	/* 0x1A041B3C */
		{
				FIELD  VC6                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT6                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC7                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT7                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI_EXT_2;	/* SENINF2_CSI2_DI_EXT_2 */

typedef volatile union _SENINF2_REG_CSI2_DI_CTRL_EXT_2_
{
		volatile struct	/* 0x1A041B40 */
		{
				FIELD  VC6_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT6_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC7_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT7_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DI_CTRL_EXT_2;	/* SENINF2_CSI2_DI_CTRL_EXT_2 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG0_
{
		volatile struct	/* 0x1A041B44 */
		{
				FIELD  SYMBOL_STREAM0_CPHY3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG0;	/* SENINF2_WIRE_STATE_DECODE_CPHY3_DBG0 */

typedef volatile union _SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG1_
{
		volatile struct	/* 0x1A041B48 */
		{
				FIELD  SYMBOL_STREAM1_CPHY3                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY3             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG1;	/* SENINF2_WIRE_STATE_DECODE_CPHY3_DBG1 */

typedef volatile union _SENINF2_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A041D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_CTRL;	/* SENINF2_MUX_CTRL */

typedef volatile union _SENINF2_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A041D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_INTEN;	/* SENINF2_MUX_INTEN */

typedef volatile union _SENINF2_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A041D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_INTSTA;	/* SENINF2_MUX_INTSTA */

typedef volatile union _SENINF2_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A041D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_SIZE;	/* SENINF2_MUX_SIZE */

typedef volatile union _SENINF2_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A041D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_1;	/* SENINF2_MUX_DEBUG_1 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A041D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_2;	/* SENINF2_MUX_DEBUG_2 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A041D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_3;	/* SENINF2_MUX_DEBUG_3 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A041D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_4;	/* SENINF2_MUX_DEBUG_4 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A041D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_5;	/* SENINF2_MUX_DEBUG_5 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A041D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_6;	/* SENINF2_MUX_DEBUG_6 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A041D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_7;	/* SENINF2_MUX_DEBUG_7 */

typedef volatile union _SENINF2_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A041D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_SPARE;	/* SENINF2_MUX_SPARE */

typedef volatile union _SENINF2_REG_MUX_DATA_
{
		volatile struct	/* 0x1A041D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DATA;	/* SENINF2_MUX_DATA */

typedef volatile union _SENINF2_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A041D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DATA_CNT;	/* SENINF2_MUX_DATA_CNT */

typedef volatile union _SENINF2_REG_MUX_CROP_
{
		volatile struct	/* 0x1A041D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_CROP;	/* SENINF2_MUX_CROP */

typedef volatile union _SENINF2_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A041D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_CTRL_EXT;	/* SENINF2_MUX_CTRL_EXT */

typedef volatile union _SENINF3_REG_CTRL_
{
		volatile struct	/* 0x1A042200 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OCSI2_SW_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_SW_RST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI3_SW_RST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CTRL;	/* SENINF3_CTRL */

typedef volatile union _SENINF3_REG_CTRL_EXT_
{
		volatile struct	/* 0x1A042204 */
		{
				FIELD  SENINF_OCSI2_IP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_TESTMDL_IP_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_SCAM_IP_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_NCSI2_IP_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_CSI2_IP_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_CSI3_IP_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CTRL_EXT;	/* SENINF3_CTRL_EXT */

typedef volatile union _SENINF3_REG_ASYNC_CTRL_
{
		volatile struct	/* 0x1A042208 */
		{
				FIELD  SENINF_ASYNC_FIFO_RST                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_ASYNC_CTRL;	/* SENINF3_ASYNC_CTRL */

typedef volatile union _SENINF_REG_TG3_PH_CNT_
{
		volatile struct	/* 0x1A042600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_PH_CNT;	/* SENINF_TG3_PH_CNT */

typedef volatile union _SENINF_REG_TG3_SEN_CK_
{
		volatile struct	/* 0x1A042604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_SEN_CK;	/* SENINF_TG3_SEN_CK */

typedef volatile union _SENINF_REG_TG3_TM_CTL_
{
		volatile struct	/* 0x1A042608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_CTL;	/* SENINF_TG3_TM_CTL */

typedef volatile union _SENINF_REG_TG3_TM_SIZE_
{
		volatile struct	/* 0x1A04260C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_SIZE;	/* SENINF_TG3_TM_SIZE */

typedef volatile union _SENINF_REG_TG3_TM_CLK_
{
		volatile struct	/* 0x1A042610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_CLK;	/* SENINF_TG3_TM_CLK */

typedef volatile union _SENINF_REG_TG3_TM_STP_
{
		volatile struct	/* 0x1A042614 */
		{
				FIELD  TG1_TM_STP                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_STP;	/* SENINF_TG3_TM_STP */

typedef volatile union _MIPI_REG_RX_CON24_CSI2_
{
		volatile struct	/* 0x1A042824 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI2;	/* MIPI_RX_CON24_CSI2 */

typedef volatile union _MIPI_REG_RX_CON28_CSI2_
{
		volatile struct	/* 0x1A042828 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI2;	/* MIPI_RX_CON28_CSI2 */

typedef volatile union _MIPI_REG_RX_CON34_CSI2_
{
		volatile struct	/* 0x1A042834 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 21;		/*  1..21, 0x003FFFFE */
				FIELD  CSI0_BIST_HSDET_MUX                   :  2;		/* 22..23, 0x00C00000 */
				FIELD  CSI0_BIST_LN4_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN5_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN6_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN7_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI2;	/* MIPI_RX_CON34_CSI2 */

typedef volatile union _MIPI_REG_RX_CON38_CSI2_
{
		volatile struct	/* 0x1A042838 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_SW_RST                        :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  MIPI_RX_SW_CPHY_TX_MODE               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIPI_RX_SW_CPHY_RX_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  rg_ckphase_trio0                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rg_ckphase_trio1                      :  5;		/* 22..26, 0x07C00000 */
				FIELD  rg_ckphase_trio2                      :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI2;	/* MIPI_RX_CON38_CSI2 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI2_
{
		volatile struct	/* 0x1A04283C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI2;	/* MIPI_RX_CON3C_CSI2 */

typedef volatile union _MIPI_REG_RX_CON7C_CSI2_
{
		volatile struct	/* 0x1A04287C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON7C_CSI2;	/* MIPI_RX_CON7C_CSI2 */

typedef volatile union _MIPI_REG_RX_CON80_CSI2_
{
		volatile struct	/* 0x1A042880 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON80_CSI2;	/* MIPI_RX_CON80_CSI2 */

typedef volatile union _MIPI_REG_RX_CON84_CSI2_
{
		volatile struct	/* 0x1A042884 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON84_CSI2;	/* MIPI_RX_CON84_CSI2 */

typedef volatile union _MIPI_REG_RX_CON88_CSI2_
{
		volatile struct	/* 0x1A042888 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON88_CSI2;	/* MIPI_RX_CON88_CSI2 */

typedef volatile union _MIPI_REG_RX_CON8C_CSI2_
{
		volatile struct	/* 0x1A04288C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON8C_CSI2;	/* MIPI_RX_CON8C_CSI2 */

typedef volatile union _MIPI_REG_RX_CON90_CSI2_
{
		volatile struct	/* 0x1A042890 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON90_CSI2;	/* MIPI_RX_CON90_CSI2 */

typedef volatile union _MIPI_REG_RX_CON94_CSI2_
{
		volatile struct	/* 0x1A042894 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON94_CSI2;	/* MIPI_RX_CON94_CSI2 */

typedef volatile union _MIPI_REG_RX_CON98_CSI2_
{
		volatile struct	/* 0x1A042898 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON98_CSI2;	/* MIPI_RX_CON98_CSI2 */

typedef volatile union _MIPI_REG_RX_CONA0_CSI2_
{
		volatile struct	/* 0x1A0428A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONA0_CSI2;	/* MIPI_RX_CONA0_CSI2 */

typedef volatile union _MIPI_REG_RX_CONB0_CSI2_
{
		volatile struct	/* 0x1A0428B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB0_CSI2;	/* MIPI_RX_CONB0_CSI2 */

typedef volatile union _MIPI_REG_RX_CONB4_CSI2_
{
		volatile struct	/* 0x1A0428B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB4_CSI2;	/* MIPI_RX_CONB4_CSI2 */

typedef volatile union _MIPI_REG_RX_CONB8_CSI2_
{
		volatile struct	/* 0x1A0428B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB8_CSI2;	/* MIPI_RX_CONB8_CSI2 */

typedef volatile union _MIPI_REG_RX_CONBC_CSI2_
{
		volatile struct	/* 0x1A0428BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONBC_CSI2;	/* MIPI_RX_CONBC_CSI2 */

typedef volatile union _MIPI_REG_RX_CONC0_CSI2_
{
		volatile struct	/* 0x1A0428C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC0_CSI2;	/* MIPI_RX_CONC0_CSI2 */

typedef volatile union _MIPI_REG_RX_CONC4_CSI2_
{
		volatile struct	/* 0x1A0428C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC4_CSI2;	/* MIPI_RX_CONC4_CSI2 */

typedef volatile union _MIPI_REG_RX_CONC8_CSI2_
{
		volatile struct	/* 0x1A0428C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC8_CSI2;	/* MIPI_RX_CONC8_CSI2 */

typedef volatile union _MIPI_REG_RX_CONCC_CSI2_
{
		volatile struct	/* 0x1A0428CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONCC_CSI2;	/* MIPI_RX_CONCC_CSI2 */

typedef volatile union _MIPI_REG_RX_COND0_CSI2_
{
		volatile struct	/* 0x1A0428D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_COND0_CSI2;	/* MIPI_RX_COND0_CSI2 */

typedef volatile union _SENINF3_REG_CSI2_CTL_
{
		volatile struct	/* 0x1A042A00 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  rsv_20                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CTL;	/* SENINF3_CSI2_CTL */

typedef volatile union _SENINF3_REG_CSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A042A04 */
		{
				FIELD  CLOCK_TERM_PARAMETER                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CLOCK_SETTLE_PARAMETER                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_LNRC_TIMING;	/* SENINF3_CSI2_LNRC_TIMING */

typedef volatile union _SENINF3_REG_CSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A042A08 */
		{
				FIELD  DATA_TERM_PARAMETER                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DATA_SETTLE_PARAMETER                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_LNRD_TIMING;	/* SENINF3_CSI2_LNRD_TIMING */

typedef volatile union _SENINF3_REG_CSI2_DPCM_
{
		volatile struct	/* 0x1A042A0C */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DPCM;	/* SENINF3_CSI2_DPCM */

typedef volatile union _SENINF3_REG_CSI2_INT_EN_
{
		volatile struct	/* 0x1A042A10 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_CRC_NO_ERROR                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FE                                    :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC                       :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6                     :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT              :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT              :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT              :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT              :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_INT_EN;	/* SENINF3_CSI2_INT_EN */

typedef volatile union _SENINF3_REG_CSI2_INT_STATUS_
{
		volatile struct	/* 0x1A042A14 */
		{
				FIELD  ERR_FRAME_SYNC_STA                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID_STA                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR_STA                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED_STA                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE_STA                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC_STA                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC_STA               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0_STA             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1_STA             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2_STA             :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3_STA             :  1;		/* 11..11, 0x00000800 */
				FIELD  FS_STA                                :  1;		/* 12..12, 0x00001000 */
				FIELD  LS_STA                                :  1;		/* 13..13, 0x00002000 */
				FIELD  GS_STA                                :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_STA                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0_STA                 :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1_STA                 :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2_STA                 :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3_STA                 :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4_STA                 :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5_STA                 :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC_STA                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6_STA                 :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7_STA                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT_STA          :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT_STA          :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT_STA          :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT_STA          :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_INT_STATUS;	/* SENINF3_CSI2_INT_STATUS */

typedef volatile union _SENINF3_REG_CSI2_DGB_SEL_
{
		volatile struct	/* 0x1A042A18 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DEBUG_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DGB_SEL;	/* SENINF3_CSI2_DGB_SEL */

typedef volatile union _SENINF3_REG_CSI2_DBG_PORT_
{
		volatile struct	/* 0x1A042A1C */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DBG_PORT;	/* SENINF3_CSI2_DBG_PORT */

typedef volatile union _SENINF3_REG_CSI2_SPARE0_
{
		volatile struct	/* 0x1A042A20 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_SPARE0;	/* SENINF3_CSI2_SPARE0 */

typedef volatile union _SENINF3_REG_CSI2_SPARE1_
{
		volatile struct	/* 0x1A042A24 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_SPARE1;	/* SENINF3_CSI2_SPARE1 */

typedef volatile union _SENINF3_REG_CSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A042A28 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_LNRC_FSM;	/* SENINF3_CSI2_LNRC_FSM */

typedef volatile union _SENINF3_REG_CSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A042A2C */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_LNRD_FSM;	/* SENINF3_CSI2_LNRD_FSM */

typedef volatile union _SENINF3_REG_CSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A042A30 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_FRAME_LINE_NUM;	/* SENINF3_CSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF3_REG_CSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A042A34 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_GENERIC_SHORT;	/* SENINF3_CSI2_GENERIC_SHORT */

typedef volatile union _SENINF3_REG_CSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A042A38 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_HSRX_DBG;	/* SENINF3_CSI2_HSRX_DBG */

typedef volatile union _SENINF3_REG_CSI2_DI_
{
		volatile struct	/* 0x1A042A3C */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI;	/* SENINF3_CSI2_DI */

typedef volatile union _SENINF3_REG_CSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A042A40 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_HS_TRAIL;	/* SENINF3_CSI2_HS_TRAIL */

typedef volatile union _SENINF3_REG_CSI2_DI_CTRL_
{
		volatile struct	/* 0x1A042A44 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI_CTRL;	/* SENINF3_CSI2_DI_CTRL */

typedef volatile union _SENINF3_REG_CSI2_DETECT_CON1_
{
		volatile struct	/* 0x1A042A4C */
		{
				FIELD  DETECT_SYNC_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_SYNC_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SYNC_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_CON1;	/* SENINF3_CSI2_DETECT_CON1 */

typedef volatile union _SENINF3_REG_CSI2_DETECT_CON2_
{
		volatile struct	/* 0x1A042A50 */
		{
				FIELD  DETECT_ESCAPE_DISABLE                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_MASK                    :  7;		/*  1.. 7, 0x000000FE */
				FIELD  ESCAPE_WORD                           : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_CON2;	/* SENINF3_CSI2_DETECT_CON2 */

typedef volatile union _SENINF3_REG_CSI2_DETECT_CON3_
{
		volatile struct	/* 0x1A042A54 */
		{
				FIELD  DETECT_POST_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_POST_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  POST_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_CON3;	/* SENINF3_CSI2_DETECT_CON3 */

typedef volatile union _SENINF3_REG_CSI2_RLR0_CON0_
{
		volatile struct	/* 0x1A042A58 */
		{
				FIELD  RLR0_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR0_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR0_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR0_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RLR0_CON0;	/* SENINF3_CSI2_RLR0_CON0 */

typedef volatile union _SENINF3_REG_CSI2_RLR1_CON0_
{
		volatile struct	/* 0x1A042A5C */
		{
				FIELD  RLR1_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR1_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR1_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR1_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RLR1_CON0;	/* SENINF3_CSI2_RLR1_CON0 */

typedef volatile union _SENINF3_REG_CSI2_RLR2_CON0_
{
		volatile struct	/* 0x1A042A60 */
		{
				FIELD  RLR2_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR2_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR2_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR2_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RLR2_CON0;	/* SENINF3_CSI2_RLR2_CON0 */

typedef volatile union _SENINF3_REG_CSI2_RLR_CON0_
{
		volatile struct	/* 0x1A042A64 */
		{
				FIELD  RLRN_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLRN_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLRN_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLRN_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RLR_CON0;	/* SENINF3_CSI2_RLR_CON0 */

typedef volatile union _SENINF3_REG_CSI2_MUX_CON_
{
		volatile struct	/* 0x1A042A68 */
		{
				FIELD  DPHY_RX_EXTERNAL_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_TX_EXTERNAL_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPHY_RX_EXTERNAL_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RLR_PATTERN_DELAY_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  POST_PACKET_IGNORE_EN                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_MUX_CON;	/* SENINF3_CSI2_MUX_CON */

typedef volatile union _SENINF3_REG_CSI2_DETECT_DBG0_
{
		volatile struct	/* 0x1A042A6C */
		{
				FIELD  DETECT_SYNC_LANE0_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE0_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE0_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE0_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE0_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DETECT_SYNC_LANE1_ST                  :  1;		/* 12..12, 0x00001000 */
				FIELD  DETECT_ESCAPE_LANE1_ST                :  1;		/* 13..13, 0x00002000 */
				FIELD  DETECT_POST_LANE1_ST                  :  1;		/* 14..14, 0x00004000 */
				FIELD  POSITION_SYNC_LANE1_ST                :  4;		/* 15..18, 0x00078000 */
				FIELD  POSITION_ESCAPE_LANE1_ST              :  4;		/* 19..22, 0x00780000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DETECT_SYNC_LANE2_ST                  :  1;		/* 24..24, 0x01000000 */
				FIELD  DETECT_ESCAPE_LANE2_ST                :  1;		/* 25..25, 0x02000000 */
				FIELD  DETECT_POST_LANE2_ST                  :  1;		/* 26..26, 0x04000000 */
				FIELD  POSITION_SYNC_LANE2_ST                :  4;		/* 27..30, 0x78000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_DBG0;	/* SENINF3_CSI2_DETECT_DBG0 */

typedef volatile union _SENINF3_REG_CSI2_DETECT_DBG1_
{
		volatile struct	/* 0x1A042A70 */
		{
				FIELD  POSITION_ESCAPE_LANE2_ST              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_DBG1;	/* SENINF3_CSI2_DETECT_DBG1 */

typedef volatile union _SENINF3_REG_CSI2_RESYNC_MERGE_CTL_
{
		volatile struct	/* 0x1A042A74 */
		{
				FIELD  CPHY_LANE_RESYNC_CNT                  :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LANE_RESYNC_FLUSH_EN                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LANE_RESYNC_DATAOUT_OPTION            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BYPASS_LANE_RESYNC                    :  1;		/* 10..10, 0x00000400 */
				FIELD  CDPHY_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RESYNC_MERGE_CTL;	/* SENINF3_CSI2_RESYNC_MERGE_CTL */

typedef volatile union _SENINF3_REG_CSI2_CTRL_TRIO_MUX_
{
		volatile struct	/* 0x1A042A78 */
		{
				FIELD  TRIO0_MUX                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TRIO1_MUX                             :  3;		/*  3.. 5, 0x00000038 */
				FIELD  TRIO2_MUX                             :  3;		/*  6.. 8, 0x000001C0 */
				FIELD  TRIO3_MUX                             :  3;		/*  9..11, 0x00000E00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CTRL_TRIO_MUX;	/* SENINF3_CSI2_CTRL_TRIO_MUX */

typedef volatile union _SENINF3_REG_CSI2_CTRL_TRIO_CON_
{
		volatile struct	/* 0x1A042A7C */
		{
				FIELD  TRIO0_LPRX_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TRIO0_HSRX_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TRIO1_LPRX_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TRIO1_HSRX_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TRIO2_LPRX_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TRIO2_HSRX_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TRIO3_LPRX_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TRIO3_HSRX_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CTRL_TRIO_CON;	/* SENINF3_CSI2_CTRL_TRIO_CON */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY0_DBG_
{
		volatile struct	/* 0x1A042A80 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY0                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY0           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY0_DBG;	/* SENINF3_FIX_ADDR_CPHY0_DBG */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY1_DBG_
{
		volatile struct	/* 0x1A042A84 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY1                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY1           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY1_DBG;	/* SENINF3_FIX_ADDR_CPHY1_DBG */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY2_DBG_
{
		volatile struct	/* 0x1A042A88 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY2                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY2           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY2_DBG;	/* SENINF3_FIX_ADDR_CPHY2_DBG */

typedef volatile union _SENINF3_REG_FIX_ADDR_DBG_
{
		volatile struct	/* 0x1A042A8C */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_DBG;	/* SENINF3_FIX_ADDR_DBG */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG0_
{
		volatile struct	/* 0x1A042A90 */
		{
				FIELD  SYMBOL_STREAM0_CPHY0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG0;	/* SENINF3_WIRE_STATE_DECODE_CPHY0_DBG0 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG1_
{
		volatile struct	/* 0x1A042A94 */
		{
				FIELD  SYMBOL_STREAM1_CPHY0                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY0             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG1;	/* SENINF3_WIRE_STATE_DECODE_CPHY0_DBG1 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG0_
{
		volatile struct	/* 0x1A042A98 */
		{
				FIELD  SYMBOL_STREAM0_CPHY1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG0;	/* SENINF3_WIRE_STATE_DECODE_CPHY1_DBG0 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG1_
{
		volatile struct	/* 0x1A042A9C */
		{
				FIELD  SYMBOL_STREAM1_CPHY1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY1             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG1;	/* SENINF3_WIRE_STATE_DECODE_CPHY1_DBG1 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG0_
{
		volatile struct	/* 0x1A042AA0 */
		{
				FIELD  SYMBOL_STREAM0_CPHY2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG0;	/* SENINF3_WIRE_STATE_DECODE_CPHY2_DBG0 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG1_
{
		volatile struct	/* 0x1A042AA4 */
		{
				FIELD  SYMBOL_STREAM1_CPHY2                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY2             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG1;	/* SENINF3_WIRE_STATE_DECODE_CPHY2_DBG1 */

typedef volatile union _SENINF3_REG_SYNC_RESYNC_CTL_
{
		volatile struct	/* 0x1A042AA8 */
		{
				FIELD  SYNC_DETECTION_SEL                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  FLUSH_VALID                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_SYNC_RESYNC_CTL;	/* SENINF3_SYNC_RESYNC_CTL */

typedef volatile union _SENINF3_REG_POST_DETECT_CTL_
{
		volatile struct	/* 0x1A042AAC */
		{
				FIELD  POST_DETECT_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  POST_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_POST_DETECT_CTL;	/* SENINF3_POST_DETECT_CTL */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CONFIG_
{
		volatile struct	/* 0x1A042AB0 */
		{
				FIELD  INIT_STATE_DECODE                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CONFIG;	/* SENINF3_WIRE_STATE_DECODE_CONFIG */

typedef volatile union _SENINF3_REG_CSI2_CPHY_LNRD_FSM_
{
		volatile struct	/* 0x1A042AB4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TRIO0_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TRIO1_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  TRIO2_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CPHY_LNRD_FSM;	/* SENINF3_CSI2_CPHY_LNRD_FSM */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY0_DBG0_
{
		volatile struct	/* 0x1A042AB8 */
		{
				FIELD  WORD_COUNT_CPHY0_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY0_DBG0;	/* SENINF3_FIX_ADDR_CPHY0_DBG0 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY0_DBG1_
{
		volatile struct	/* 0x1A042ABC */
		{
				FIELD  WORD_COUNT_CPHY0_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY0_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY0_DBG1;	/* SENINF3_FIX_ADDR_CPHY0_DBG1 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY0_DBG2_
{
		volatile struct	/* 0x1A042AC0 */
		{
				FIELD  ERROR_RECORD_CPHY0_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY0_DBG2;	/* SENINF3_FIX_ADDR_CPHY0_DBG2 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY1_DBG0_
{
		volatile struct	/* 0x1A042AC4 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY1_DBG0;	/* SENINF3_FIX_ADDR_CPHY1_DBG0 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY1_DBG1_
{
		volatile struct	/* 0x1A042AC8 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY1_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY1_DBG1;	/* SENINF3_FIX_ADDR_CPHY1_DBG1 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY1_DBG2_
{
		volatile struct	/* 0x1A042ACC */
		{
				FIELD  ERROR_RECORD_CPHY1_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY1_DBG2;	/* SENINF3_FIX_ADDR_CPHY1_DBG2 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY2_DBG0_
{
		volatile struct	/* 0x1A042AD0 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY2_DBG0;	/* SENINF3_FIX_ADDR_CPHY2_DBG0 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY2_DBG1_
{
		volatile struct	/* 0x1A042AD4 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY2_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY2_DBG1;	/* SENINF3_FIX_ADDR_CPHY2_DBG1 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY2_DBG2_
{
		volatile struct	/* 0x1A042AD8 */
		{
				FIELD  ERROR_RECORD_CPHY2_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY2_DBG2;	/* SENINF3_FIX_ADDR_CPHY2_DBG2 */

typedef volatile union _SENINF3_REG_FIX_ADDR_DBG0_
{
		volatile struct	/* 0x1A042ADC */
		{
				FIELD  WORD_COUNT_DBG0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_DBG0;	/* SENINF3_FIX_ADDR_DBG0 */

typedef volatile union _SENINF3_REG_FIX_ADDR_DBG1_
{
		volatile struct	/* 0x1A042AE0 */
		{
				FIELD  WORD_COUNT_DBG1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_DBG0                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_DBG1;	/* SENINF3_FIX_ADDR_DBG1 */

typedef volatile union _SENINF3_REG_FIX_ADDR_DBG2_
{
		volatile struct	/* 0x1A042AE4 */
		{
				FIELD  ERROR_RECORD_DBG1                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_DBG2;	/* SENINF3_FIX_ADDR_DBG2 */

typedef volatile union _SENINF3_REG_CSI2_MODE_
{
		volatile struct	/* 0x1A042AE8 */
		{
				FIELD  csr_csi2_mode                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  csr_csi2_header_len                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  csr_cphy_di_pos                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  csr_cphy_wc_pos                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_MODE;	/* SENINF3_CSI2_MODE */

typedef volatile union _SENINF3_REG_CSI2_DI_EXT_
{
		volatile struct	/* 0x1A042AF0 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI_EXT;	/* SENINF3_CSI2_DI_EXT */

typedef volatile union _SENINF3_REG_CSI2_DI_CTRL_EXT_
{
		volatile struct	/* 0x1A042AF4 */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI_CTRL_EXT;	/* SENINF3_CSI2_DI_CTRL_EXT */

typedef volatile union _SENINF3_REG_CSI2_CPHY_LOOPBACK_
{
		volatile struct	/* 0x1A042AF8 */
		{
				FIELD  TRIGGER_SYNC_INIT                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RELEASE_SYNC_INIT                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CPHY_LOOPBACK;	/* SENINF3_CSI2_CPHY_LOOPBACK */

typedef volatile union _SENINF3_REG_CSI2_PROGSEQ_0_
{
		volatile struct	/* 0x1A042B00 */
		{
				FIELD  PROGSEQ_S0                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S1                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S2                            :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S3                            :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S4                            :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S5                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  PROGSEQ_S6                            :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  PROGSEQ_S7                            :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_PROGSEQ_0;	/* SENINF3_CSI2_PROGSEQ_0 */

typedef volatile union _SENINF3_REG_CSI2_PROGSEQ_1_
{
		volatile struct	/* 0x1A042B04 */
		{
				FIELD  PROGSEQ_S8                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S9                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S10                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S11                           :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S12                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S13                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_PROGSEQ_1;	/* SENINF3_CSI2_PROGSEQ_1 */

typedef volatile union _SENINF3_REG_CSI2_INT_EN_EXT_
{
		volatile struct	/* 0x1A042B10 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 19;		/* 12..30, 0x7FFFF000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_INT_EN_EXT;	/* SENINF3_CSI2_INT_EN_EXT */

typedef volatile union _SENINF3_REG_CSI2_INT_STATUS_EXT_
{
		volatile struct	/* 0x1A042B14 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_INT_STATUS_EXT;	/* SENINF3_CSI2_INT_STATUS_EXT */

typedef volatile union _SENINF3_REG_CSI2_CPHY_FIX_POINT_RST_
{
		volatile struct	/* 0x1A042B18 */
		{
				FIELD  CPHY_FIX_POINT_RST                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_FIX_POINT_RST_MODE               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_CPHY_FIX_POINT_RST;	/* SENINF3_CSI2_CPHY_FIX_POINT_RST */

typedef volatile union _SENINF3_REG_CSI2_RLR3_CON0_
{
		volatile struct	/* 0x1A042B1C */
		{
				FIELD  RLR3_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR3_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR3_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR3_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_RLR3_CON0;	/* SENINF3_CSI2_RLR3_CON0 */

typedef volatile union _SENINF3_REG_CSI2_DPHY_SYNC_
{
		volatile struct	/* 0x1A042B20 */
		{
				FIELD  SYNC_SEQ_MASK_0                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_0                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DPHY_SYNC;	/* SENINF3_CSI2_DPHY_SYNC */

typedef volatile union _SENINF3_REG_CSI2_DESKEW_SYNC_
{
		volatile struct	/* 0x1A042B24 */
		{
				FIELD  SYNC_SEQ_MASK_1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_1                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DESKEW_SYNC;	/* SENINF3_CSI2_DESKEW_SYNC */

typedef volatile union _SENINF3_REG_CSI2_DETECT_DBG2_
{
		volatile struct	/* 0x1A042B28 */
		{
				FIELD  DETECT_SYNC_LANE3_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE3_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE3_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE3_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE3_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DETECT_DBG2;	/* SENINF3_CSI2_DETECT_DBG2 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY3_DBG0_
{
		volatile struct	/* 0x1A042B30 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY3_DBG0;	/* SENINF3_FIX_ADDR_CPHY3_DBG0 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY3_DBG1_
{
		volatile struct	/* 0x1A042B34 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY3_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY3_DBG1;	/* SENINF3_FIX_ADDR_CPHY3_DBG1 */

typedef volatile union _SENINF3_REG_FIX_ADDR_CPHY3_DBG2_
{
		volatile struct	/* 0x1A042B38 */
		{
				FIELD  ERROR_RECORD_CPHY3_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_FIX_ADDR_CPHY3_DBG2;	/* SENINF3_FIX_ADDR_CPHY3_DBG2 */


typedef volatile union _SENINF3_REG_CSI2_DI_EXT_2_
{
		volatile struct	/* 0x1A042B3C */
		{
				FIELD  VC6                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT6                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC7                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT7                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI_EXT_2;	/* SENINF3_CSI2_DI_EXT_2 */

typedef volatile union _SENINF3_REG_CSI2_DI_CTRL_EXT_2_
{
		volatile struct	/* 0x1A042B40 */
		{
				FIELD  VC6_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT6_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC7_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT7_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CSI2_DI_CTRL_EXT_2;	/* SENINF3_CSI2_DI_CTRL_EXT_2 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG0_
{
		volatile struct	/* 0x1A042B44 */
		{
				FIELD  SYMBOL_STREAM0_CPHY3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG0;	/* SENINF3_WIRE_STATE_DECODE_CPHY3_DBG0 */

typedef volatile union _SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG1_
{
		volatile struct	/* 0x1A042B48 */
		{
				FIELD  SYMBOL_STREAM1_CPHY3                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY3             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG1;	/* SENINF3_WIRE_STATE_DECODE_CPHY3_DBG1 */

typedef volatile union _SENINF3_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A042D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_CTRL;	/* SENINF3_MUX_CTRL */

typedef volatile union _SENINF3_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A042D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_INTEN;	/* SENINF3_MUX_INTEN */

typedef volatile union _SENINF3_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A042D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_INTSTA;	/* SENINF3_MUX_INTSTA */

typedef volatile union _SENINF3_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A042D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_SIZE;	/* SENINF3_MUX_SIZE */

typedef volatile union _SENINF3_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A042D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_1;	/* SENINF3_MUX_DEBUG_1 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A042D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_2;	/* SENINF3_MUX_DEBUG_2 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A042D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_3;	/* SENINF3_MUX_DEBUG_3 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A042D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_4;	/* SENINF3_MUX_DEBUG_4 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A042D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_5;	/* SENINF3_MUX_DEBUG_5 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A042D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_6;	/* SENINF3_MUX_DEBUG_6 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A042D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_7;	/* SENINF3_MUX_DEBUG_7 */

typedef volatile union _SENINF3_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A042D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_SPARE;	/* SENINF3_MUX_SPARE */

typedef volatile union _SENINF3_REG_MUX_DATA_
{
		volatile struct	/* 0x1A042D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DATA;	/* SENINF3_MUX_DATA */

typedef volatile union _SENINF3_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A042D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DATA_CNT;	/* SENINF3_MUX_DATA_CNT */

typedef volatile union _SENINF3_REG_MUX_CROP_
{
		volatile struct	/* 0x1A042D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_CROP;	/* SENINF3_MUX_CROP */

typedef volatile union _SENINF3_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A042D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_CTRL_EXT;	/* SENINF3_MUX_CTRL_EXT */

typedef volatile union _SENINF4_REG_CTRL_
{
		volatile struct	/* 0x1A043200 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OCSI2_SW_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_SW_RST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI3_SW_RST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CTRL;	/* SENINF4_CTRL */

typedef volatile union _SENINF4_REG_CTRL_EXT_
{
		volatile struct	/* 0x1A043204 */
		{
				FIELD  SENINF_OCSI2_IP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_TESTMDL_IP_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_SCAM_IP_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_NCSI2_IP_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_CSI2_IP_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_CSI3_IP_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CTRL_EXT;	/* SENINF4_CTRL_EXT */

typedef volatile union _SENINF4_REG_ASYNC_CTRL_
{
		volatile struct	/* 0x1A043208 */
		{
				FIELD  SENINF_ASYNC_FIFO_RST                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_ASYNC_CTRL;	/* SENINF4_ASYNC_CTRL */

typedef volatile union _SENINF_REG_TG4_PH_CNT_
{
		volatile struct	/* 0x1A043600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_PH_CNT;	/* SENINF_TG4_PH_CNT */

typedef volatile union _SENINF_REG_TG4_SEN_CK_
{
		volatile struct	/* 0x1A043604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_SEN_CK;	/* SENINF_TG4_SEN_CK */

typedef volatile union _SENINF_REG_TG4_TM_CTL_
{
		volatile struct	/* 0x1A043608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_CTL;	/* SENINF_TG4_TM_CTL */

typedef volatile union _SENINF_REG_TG4_TM_SIZE_
{
		volatile struct	/* 0x1A04360C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_SIZE;	/* SENINF_TG4_TM_SIZE */

typedef volatile union _SENINF_REG_TG4_TM_CLK_
{
		volatile struct	/* 0x1A043610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_CLK;	/* SENINF_TG4_TM_CLK */

typedef volatile union _SENINF_REG_TG4_TM_STP_
{
		volatile struct	/* 0x1A043614 */
		{
				FIELD  TG1_TM_STP                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_STP;	/* SENINF_TG4_TM_STP */

typedef volatile union _MIPI_REG_RX_CON24_CSI3_
{
		volatile struct	/* 0x1A043824 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI3;	/* MIPI_RX_CON24_CSI3 */

typedef volatile union _MIPI_REG_RX_CON28_CSI3_
{
		volatile struct	/* 0x1A043828 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI3;	/* MIPI_RX_CON28_CSI3 */

typedef volatile union _MIPI_REG_RX_CON34_CSI3_
{
		volatile struct	/* 0x1A043834 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 21;		/*  1..21, 0x003FFFFE */
				FIELD  CSI0_BIST_HSDET_MUX                   :  2;		/* 22..23, 0x00C00000 */
				FIELD  CSI0_BIST_LN4_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN5_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN6_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN7_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI3;	/* MIPI_RX_CON34_CSI3 */

typedef volatile union _MIPI_REG_RX_CON38_CSI3_
{
		volatile struct	/* 0x1A043838 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_SW_RST                        :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  MIPI_RX_SW_CPHY_TX_MODE               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIPI_RX_SW_CPHY_RX_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  rg_ckphase_trio0                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rg_ckphase_trio1                      :  5;		/* 22..26, 0x07C00000 */
				FIELD  rg_ckphase_trio2                      :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI3;	/* MIPI_RX_CON38_CSI3 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI3_
{
		volatile struct	/* 0x1A04383C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI3;	/* MIPI_RX_CON3C_CSI3 */

typedef volatile union _MIPI_REG_RX_CON7C_CSI3_
{
		volatile struct	/* 0x1A04387C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON7C_CSI3;	/* MIPI_RX_CON7C_CSI3 */

typedef volatile union _MIPI_REG_RX_CON80_CSI3_
{
		volatile struct	/* 0x1A043880 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON80_CSI3;	/* MIPI_RX_CON80_CSI3 */

typedef volatile union _MIPI_REG_RX_CON84_CSI3_
{
		volatile struct	/* 0x1A043884 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON84_CSI3;	/* MIPI_RX_CON84_CSI3 */

typedef volatile union _MIPI_REG_RX_CON88_CSI3_
{
		volatile struct	/* 0x1A043888 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON88_CSI3;	/* MIPI_RX_CON88_CSI3 */

typedef volatile union _MIPI_REG_RX_CON8C_CSI3_
{
		volatile struct	/* 0x1A04388C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON8C_CSI3;	/* MIPI_RX_CON8C_CSI3 */

typedef volatile union _MIPI_REG_RX_CON90_CSI3_
{
		volatile struct	/* 0x1A043890 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON90_CSI3;	/* MIPI_RX_CON90_CSI3 */

typedef volatile union _MIPI_REG_RX_CON94_CSI3_
{
		volatile struct	/* 0x1A043894 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON94_CSI3;	/* MIPI_RX_CON94_CSI3 */

typedef volatile union _MIPI_REG_RX_CON98_CSI3_
{
		volatile struct	/* 0x1A043898 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON98_CSI3;	/* MIPI_RX_CON98_CSI3 */

typedef volatile union _MIPI_REG_RX_CONA0_CSI3_
{
		volatile struct	/* 0x1A0438A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONA0_CSI3;	/* MIPI_RX_CONA0_CSI3 */

typedef volatile union _MIPI_REG_RX_CONB0_CSI3_
{
		volatile struct	/* 0x1A0438B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB0_CSI3;	/* MIPI_RX_CONB0_CSI3 */

typedef volatile union _MIPI_REG_RX_CONB4_CSI3_
{
		volatile struct	/* 0x1A0438B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB4_CSI3;	/* MIPI_RX_CONB4_CSI3 */

typedef volatile union _MIPI_REG_RX_CONB8_CSI3_
{
		volatile struct	/* 0x1A0438B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB8_CSI3;	/* MIPI_RX_CONB8_CSI3 */

typedef volatile union _MIPI_REG_RX_CONBC_CSI3_
{
		volatile struct	/* 0x1A0438BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONBC_CSI3;	/* MIPI_RX_CONBC_CSI3 */

typedef volatile union _MIPI_REG_RX_CONC0_CSI3_
{
		volatile struct	/* 0x1A0438C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC0_CSI3;	/* MIPI_RX_CONC0_CSI3 */

typedef volatile union _MIPI_REG_RX_CONC4_CSI3_
{
		volatile struct	/* 0x1A0438C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC4_CSI3;	/* MIPI_RX_CONC4_CSI3 */

typedef volatile union _MIPI_REG_RX_CONC8_CSI3_
{
		volatile struct	/* 0x1A0438C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC8_CSI3;	/* MIPI_RX_CONC8_CSI3 */

typedef volatile union _MIPI_REG_RX_CONCC_CSI3_
{
		volatile struct	/* 0x1A0438CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONCC_CSI3;	/* MIPI_RX_CONCC_CSI3 */

typedef volatile union _MIPI_REG_RX_COND0_CSI3_
{
		volatile struct	/* 0x1A0438D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_COND0_CSI3;	/* MIPI_RX_COND0_CSI3 */

typedef volatile union _SENINF4_REG_CSI2_CTL_
{
		volatile struct	/* 0x1A043A00 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  rsv_20                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CTL;	/* SENINF4_CSI2_CTL */

typedef volatile union _SENINF4_REG_CSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A043A04 */
		{
				FIELD  CLOCK_TERM_PARAMETER                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CLOCK_SETTLE_PARAMETER                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_LNRC_TIMING;	/* SENINF4_CSI2_LNRC_TIMING */

typedef volatile union _SENINF4_REG_CSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A043A08 */
		{
				FIELD  DATA_TERM_PARAMETER                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DATA_SETTLE_PARAMETER                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_LNRD_TIMING;	/* SENINF4_CSI2_LNRD_TIMING */

typedef volatile union _SENINF4_REG_CSI2_DPCM_
{
		volatile struct	/* 0x1A043A0C */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DPCM;	/* SENINF4_CSI2_DPCM */

typedef volatile union _SENINF4_REG_CSI2_INT_EN_
{
		volatile struct	/* 0x1A043A10 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_CRC_NO_ERROR                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FE                                    :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC                       :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6                     :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT              :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT              :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT              :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT              :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_INT_EN;	/* SENINF4_CSI2_INT_EN */

typedef volatile union _SENINF4_REG_CSI2_INT_STATUS_
{
		volatile struct	/* 0x1A043A14 */
		{
				FIELD  ERR_FRAME_SYNC_STA                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID_STA                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR_STA                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED_STA                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE_STA                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC_STA                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC_STA               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0_STA             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1_STA             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2_STA             :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3_STA             :  1;		/* 11..11, 0x00000800 */
				FIELD  FS_STA                                :  1;		/* 12..12, 0x00001000 */
				FIELD  LS_STA                                :  1;		/* 13..13, 0x00002000 */
				FIELD  GS_STA                                :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_STA                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0_STA                 :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1_STA                 :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2_STA                 :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3_STA                 :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4_STA                 :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5_STA                 :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC_STA                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6_STA                 :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7_STA                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT_STA          :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT_STA          :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT_STA          :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT_STA          :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_INT_STATUS;	/* SENINF4_CSI2_INT_STATUS */

typedef volatile union _SENINF4_REG_CSI2_DGB_SEL_
{
		volatile struct	/* 0x1A043A18 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DEBUG_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DGB_SEL;	/* SENINF4_CSI2_DGB_SEL */

typedef volatile union _SENINF4_REG_CSI2_DBG_PORT_
{
		volatile struct	/* 0x1A043A1C */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DBG_PORT;	/* SENINF4_CSI2_DBG_PORT */

typedef volatile union _SENINF4_REG_CSI2_SPARE0_
{
		volatile struct	/* 0x1A043A20 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_SPARE0;	/* SENINF4_CSI2_SPARE0 */

typedef volatile union _SENINF4_REG_CSI2_SPARE1_
{
		volatile struct	/* 0x1A043A24 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_SPARE1;	/* SENINF4_CSI2_SPARE1 */

typedef volatile union _SENINF4_REG_CSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A043A28 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_LNRC_FSM;	/* SENINF4_CSI2_LNRC_FSM */

typedef volatile union _SENINF4_REG_CSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A043A2C */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_LNRD_FSM;	/* SENINF4_CSI2_LNRD_FSM */

typedef volatile union _SENINF4_REG_CSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A043A30 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_FRAME_LINE_NUM;	/* SENINF4_CSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF4_REG_CSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A043A34 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_GENERIC_SHORT;	/* SENINF4_CSI2_GENERIC_SHORT */

typedef volatile union _SENINF4_REG_CSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A043A38 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_HSRX_DBG;	/* SENINF4_CSI2_HSRX_DBG */

typedef volatile union _SENINF4_REG_CSI2_DI_
{
		volatile struct	/* 0x1A043A3C */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI;	/* SENINF4_CSI2_DI */

typedef volatile union _SENINF4_REG_CSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A043A40 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_HS_TRAIL;	/* SENINF4_CSI2_HS_TRAIL */

typedef volatile union _SENINF4_REG_CSI2_DI_CTRL_
{
		volatile struct	/* 0x1A043A44 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI_CTRL;	/* SENINF4_CSI2_DI_CTRL */

typedef volatile union _SENINF4_REG_CSI2_DETECT_CON1_
{
		volatile struct	/* 0x1A043A4C */
		{
				FIELD  DETECT_SYNC_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_SYNC_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SYNC_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_CON1;	/* SENINF4_CSI2_DETECT_CON1 */

typedef volatile union _SENINF4_REG_CSI2_DETECT_CON2_
{
		volatile struct	/* 0x1A043A50 */
		{
				FIELD  DETECT_ESCAPE_DISABLE                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_MASK                    :  7;		/*  1.. 7, 0x000000FE */
				FIELD  ESCAPE_WORD                           : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_CON2;	/* SENINF4_CSI2_DETECT_CON2 */

typedef volatile union _SENINF4_REG_CSI2_DETECT_CON3_
{
		volatile struct	/* 0x1A043A54 */
		{
				FIELD  DETECT_POST_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_POST_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  POST_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_CON3;	/* SENINF4_CSI2_DETECT_CON3 */

typedef volatile union _SENINF4_REG_CSI2_RLR0_CON0_
{
		volatile struct	/* 0x1A043A58 */
		{
				FIELD  RLR0_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR0_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR0_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR0_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RLR0_CON0;	/* SENINF4_CSI2_RLR0_CON0 */

typedef volatile union _SENINF4_REG_CSI2_RLR1_CON0_
{
		volatile struct	/* 0x1A043A5C */
		{
				FIELD  RLR1_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR1_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR1_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR1_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RLR1_CON0;	/* SENINF4_CSI2_RLR1_CON0 */

typedef volatile union _SENINF4_REG_CSI2_RLR2_CON0_
{
		volatile struct	/* 0x1A043A60 */
		{
				FIELD  RLR2_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR2_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR2_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR2_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RLR2_CON0;	/* SENINF4_CSI2_RLR2_CON0 */

typedef volatile union _SENINF4_REG_CSI2_RLR_CON0_
{
		volatile struct	/* 0x1A043A64 */
		{
				FIELD  RLRN_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLRN_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLRN_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLRN_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RLR_CON0;	/* SENINF4_CSI2_RLR_CON0 */

typedef volatile union _SENINF4_REG_CSI2_MUX_CON_
{
		volatile struct	/* 0x1A043A68 */
		{
				FIELD  DPHY_RX_EXTERNAL_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_TX_EXTERNAL_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPHY_RX_EXTERNAL_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RLR_PATTERN_DELAY_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  POST_PACKET_IGNORE_EN                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_MUX_CON;	/* SENINF4_CSI2_MUX_CON */

typedef volatile union _SENINF4_REG_CSI2_DETECT_DBG0_
{
		volatile struct	/* 0x1A043A6C */
		{
				FIELD  DETECT_SYNC_LANE0_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE0_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE0_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE0_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE0_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DETECT_SYNC_LANE1_ST                  :  1;		/* 12..12, 0x00001000 */
				FIELD  DETECT_ESCAPE_LANE1_ST                :  1;		/* 13..13, 0x00002000 */
				FIELD  DETECT_POST_LANE1_ST                  :  1;		/* 14..14, 0x00004000 */
				FIELD  POSITION_SYNC_LANE1_ST                :  4;		/* 15..18, 0x00078000 */
				FIELD  POSITION_ESCAPE_LANE1_ST              :  4;		/* 19..22, 0x00780000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DETECT_SYNC_LANE2_ST                  :  1;		/* 24..24, 0x01000000 */
				FIELD  DETECT_ESCAPE_LANE2_ST                :  1;		/* 25..25, 0x02000000 */
				FIELD  DETECT_POST_LANE2_ST                  :  1;		/* 26..26, 0x04000000 */
				FIELD  POSITION_SYNC_LANE2_ST                :  4;		/* 27..30, 0x78000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_DBG0;	/* SENINF4_CSI2_DETECT_DBG0 */

typedef volatile union _SENINF4_REG_CSI2_DETECT_DBG1_
{
		volatile struct	/* 0x1A043A70 */
		{
				FIELD  POSITION_ESCAPE_LANE2_ST              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_DBG1;	/* SENINF4_CSI2_DETECT_DBG1 */


typedef volatile union _SENINF4_REG_CSI2_RESYNC_MERGE_CTL_
{
		volatile struct	/* 0x1A043A74 */
		{
				FIELD  CPHY_LANE_RESYNC_CNT                  :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LANE_RESYNC_FLUSH_EN                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LANE_RESYNC_DATAOUT_OPTION            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BYPASS_LANE_RESYNC                    :  1;		/* 10..10, 0x00000400 */
				FIELD  CDPHY_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RESYNC_MERGE_CTL;	/* SENINF4_CSI2_RESYNC_MERGE_CTL */

typedef volatile union _SENINF4_REG_CSI2_CTRL_TRIO_MUX_
{
		volatile struct	/* 0x1A043A78 */
		{
				FIELD  TRIO0_MUX                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TRIO1_MUX                             :  3;		/*  3.. 5, 0x00000038 */
				FIELD  TRIO2_MUX                             :  3;		/*  6.. 8, 0x000001C0 */
				FIELD  TRIO3_MUX                             :  3;		/*  9..11, 0x00000E00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CTRL_TRIO_MUX;	/* SENINF4_CSI2_CTRL_TRIO_MUX */

typedef volatile union _SENINF4_REG_CSI2_CTRL_TRIO_CON_
{
		volatile struct	/* 0x1A043A7C */
		{
				FIELD  TRIO0_LPRX_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TRIO0_HSRX_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TRIO1_LPRX_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TRIO1_HSRX_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TRIO2_LPRX_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TRIO2_HSRX_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TRIO3_LPRX_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TRIO3_HSRX_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CTRL_TRIO_CON;	/* SENINF4_CSI2_CTRL_TRIO_CON */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY0_DBG_
{
		volatile struct	/* 0x1A043A80 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY0                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY0           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY0_DBG;	/* SENINF4_FIX_ADDR_CPHY0_DBG */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY1_DBG_
{
		volatile struct	/* 0x1A043A84 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY1                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY1           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY1_DBG;	/* SENINF4_FIX_ADDR_CPHY1_DBG */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY2_DBG_
{
		volatile struct	/* 0x1A043A88 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY2                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY2           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY2_DBG;	/* SENINF4_FIX_ADDR_CPHY2_DBG */

typedef volatile union _SENINF4_REG_FIX_ADDR_DBG_
{
		volatile struct	/* 0x1A043A8C */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_DBG;	/* SENINF4_FIX_ADDR_DBG */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG0_
{
		volatile struct	/* 0x1A043A90 */
		{
				FIELD  SYMBOL_STREAM0_CPHY0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG0;	/* SENINF4_WIRE_STATE_DECODE_CPHY0_DBG0 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG1_
{
		volatile struct	/* 0x1A043A94 */
		{
				FIELD  SYMBOL_STREAM1_CPHY0                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY0             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG1;	/* SENINF4_WIRE_STATE_DECODE_CPHY0_DBG1 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG0_
{
		volatile struct	/* 0x1A043A98 */
		{
				FIELD  SYMBOL_STREAM0_CPHY1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG0;	/* SENINF4_WIRE_STATE_DECODE_CPHY1_DBG0 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG1_
{
		volatile struct	/* 0x1A043A9C */
		{
				FIELD  SYMBOL_STREAM1_CPHY1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY1             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG1;	/* SENINF4_WIRE_STATE_DECODE_CPHY1_DBG1 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG0_
{
		volatile struct	/* 0x1A043AA0 */
		{
				FIELD  SYMBOL_STREAM0_CPHY2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG0;	/* SENINF4_WIRE_STATE_DECODE_CPHY2_DBG0 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG1_
{
		volatile struct	/* 0x1A043AA4 */
		{
				FIELD  SYMBOL_STREAM1_CPHY2                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY2             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG1;	/* SENINF4_WIRE_STATE_DECODE_CPHY2_DBG1 */

typedef volatile union _SENINF4_REG_SYNC_RESYNC_CTL_
{
		volatile struct	/* 0x1A043AA8 */
		{
				FIELD  SYNC_DETECTION_SEL                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  FLUSH_VALID                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_SYNC_RESYNC_CTL;	/* SENINF4_SYNC_RESYNC_CTL */

typedef volatile union _SENINF4_REG_POST_DETECT_CTL_
{
		volatile struct	/* 0x1A043AAC */
		{
				FIELD  POST_DETECT_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  POST_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_POST_DETECT_CTL;	/* SENINF4_POST_DETECT_CTL */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CONFIG_
{
		volatile struct	/* 0x1A043AB0 */
		{
				FIELD  INIT_STATE_DECODE                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CONFIG;	/* SENINF4_WIRE_STATE_DECODE_CONFIG */

typedef volatile union _SENINF4_REG_CSI2_CPHY_LNRD_FSM_
{
		volatile struct	/* 0x1A043AB4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TRIO0_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TRIO1_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  TRIO2_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CPHY_LNRD_FSM;	/* SENINF4_CSI2_CPHY_LNRD_FSM */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY0_DBG0_
{
		volatile struct	/* 0x1A043AB8 */
		{
				FIELD  WORD_COUNT_CPHY0_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY0_DBG0;	/* SENINF4_FIX_ADDR_CPHY0_DBG0 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY0_DBG1_
{
		volatile struct	/* 0x1A043ABC */
		{
				FIELD  WORD_COUNT_CPHY0_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY0_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY0_DBG1;	/* SENINF4_FIX_ADDR_CPHY0_DBG1 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY0_DBG2_
{
		volatile struct	/* 0x1A043AC0 */
		{
				FIELD  ERROR_RECORD_CPHY0_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY0_DBG2;	/* SENINF4_FIX_ADDR_CPHY0_DBG2 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY1_DBG0_
{
		volatile struct	/* 0x1A043AC4 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY1_DBG0;	/* SENINF4_FIX_ADDR_CPHY1_DBG0 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY1_DBG1_
{
		volatile struct	/* 0x1A043AC8 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY1_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY1_DBG1;	/* SENINF4_FIX_ADDR_CPHY1_DBG1 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY1_DBG2_
{
		volatile struct	/* 0x1A043ACC */
		{
				FIELD  ERROR_RECORD_CPHY1_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY1_DBG2;	/* SENINF4_FIX_ADDR_CPHY1_DBG2 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY2_DBG0_
{
		volatile struct	/* 0x1A043AD0 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY2_DBG0;	/* SENINF4_FIX_ADDR_CPHY2_DBG0 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY2_DBG1_
{
		volatile struct	/* 0x1A043AD4 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY2_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY2_DBG1;	/* SENINF4_FIX_ADDR_CPHY2_DBG1 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY2_DBG2_
{
		volatile struct	/* 0x1A043AD8 */
		{
				FIELD  ERROR_RECORD_CPHY2_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY2_DBG2;	/* SENINF4_FIX_ADDR_CPHY2_DBG2 */

typedef volatile union _SENINF4_REG_FIX_ADDR_DBG0_
{
		volatile struct	/* 0x1A043ADC */
		{
				FIELD  WORD_COUNT_DBG0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_DBG0;	/* SENINF4_FIX_ADDR_DBG0 */

typedef volatile union _SENINF4_REG_FIX_ADDR_DBG1_
{
		volatile struct	/* 0x1A043AE0 */
		{
				FIELD  WORD_COUNT_DBG1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_DBG0                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_DBG1;	/* SENINF4_FIX_ADDR_DBG1 */

typedef volatile union _SENINF4_REG_FIX_ADDR_DBG2_
{
		volatile struct	/* 0x1A043AE4 */
		{
				FIELD  ERROR_RECORD_DBG1                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_DBG2;	/* SENINF4_FIX_ADDR_DBG2 */

typedef volatile union _SENINF4_REG_CSI2_MODE_
{
		volatile struct	/* 0x1A043AE8 */
		{
				FIELD  csr_csi2_mode                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  csr_csi2_header_len                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  csr_cphy_di_pos                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  csr_cphy_wc_pos                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_MODE;	/* SENINF4_CSI2_MODE */


typedef volatile union _SENINF4_REG_CSI2_DI_EXT_
{
		volatile struct	/* 0x1A043AF0 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI_EXT;	/* SENINF4_CSI2_DI_EXT */

typedef volatile union _SENINF4_REG_CSI2_DI_CTRL_EXT_
{
		volatile struct	/* 0x1A043AF4 */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI_CTRL_EXT;	/* SENINF4_CSI2_DI_CTRL_EXT */

typedef volatile union _SENINF4_REG_CSI2_CPHY_LOOPBACK_
{
		volatile struct	/* 0x1A043AF8 */
		{
				FIELD  TRIGGER_SYNC_INIT                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RELEASE_SYNC_INIT                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CPHY_LOOPBACK;	/* SENINF4_CSI2_CPHY_LOOPBACK */

typedef volatile union _SENINF4_REG_CSI2_PROGSEQ_0_
{
		volatile struct	/* 0x1A043B00 */
		{
				FIELD  PROGSEQ_S0                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S1                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S2                            :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S3                            :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S4                            :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S5                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  PROGSEQ_S6                            :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  PROGSEQ_S7                            :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_PROGSEQ_0;	/* SENINF4_CSI2_PROGSEQ_0 */

typedef volatile union _SENINF4_REG_CSI2_PROGSEQ_1_
{
		volatile struct	/* 0x1A043B04 */
		{
				FIELD  PROGSEQ_S8                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S9                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S10                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S11                           :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S12                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S13                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_PROGSEQ_1;	/* SENINF4_CSI2_PROGSEQ_1 */


typedef volatile union _SENINF4_REG_CSI2_INT_EN_EXT_
{
		volatile struct	/* 0x1A043B10 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 19;		/* 12..30, 0x7FFFF000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_INT_EN_EXT;	/* SENINF4_CSI2_INT_EN_EXT */

typedef volatile union _SENINF4_REG_CSI2_INT_STATUS_EXT_
{
		volatile struct	/* 0x1A043B14 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_INT_STATUS_EXT;	/* SENINF4_CSI2_INT_STATUS_EXT */

typedef volatile union _SENINF4_REG_CSI2_CPHY_FIX_POINT_RST_
{
		volatile struct	/* 0x1A043B18 */
		{
				FIELD  CPHY_FIX_POINT_RST                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_FIX_POINT_RST_MODE               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_CPHY_FIX_POINT_RST;	/* SENINF4_CSI2_CPHY_FIX_POINT_RST */

typedef volatile union _SENINF4_REG_CSI2_RLR3_CON0_
{
		volatile struct	/* 0x1A043B1C */
		{
				FIELD  RLR3_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR3_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR3_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR3_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_RLR3_CON0;	/* SENINF4_CSI2_RLR3_CON0 */


typedef volatile union _SENINF4_REG_CSI2_DPHY_SYNC_
{
		volatile struct	/* 0x1A043B20 */
		{
				FIELD  SYNC_SEQ_MASK_0                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_0                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DPHY_SYNC;	/* SENINF4_CSI2_DPHY_SYNC */

typedef volatile union _SENINF4_REG_CSI2_DESKEW_SYNC_
{
		volatile struct	/* 0x1A043B24 */
		{
				FIELD  SYNC_SEQ_MASK_1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_1                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DESKEW_SYNC;	/* SENINF4_CSI2_DESKEW_SYNC */

typedef volatile union _SENINF4_REG_CSI2_DETECT_DBG2_
{
		volatile struct	/* 0x1A043B28 */
		{
				FIELD  DETECT_SYNC_LANE3_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE3_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE3_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE3_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE3_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DETECT_DBG2;	/* SENINF4_CSI2_DETECT_DBG2 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY3_DBG0_
{
		volatile struct	/* 0x1A043B30 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY3_DBG0;	/* SENINF4_FIX_ADDR_CPHY3_DBG0 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY3_DBG1_
{
		volatile struct	/* 0x1A043B34 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY3_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY3_DBG1;	/* SENINF4_FIX_ADDR_CPHY3_DBG1 */

typedef volatile union _SENINF4_REG_FIX_ADDR_CPHY3_DBG2_
{
		volatile struct	/* 0x1A043B38 */
		{
				FIELD  ERROR_RECORD_CPHY3_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_FIX_ADDR_CPHY3_DBG2;	/* SENINF4_FIX_ADDR_CPHY3_DBG2 */

typedef volatile union _SENINF4_REG_CSI2_DI_EXT_2_
{
		volatile struct	/* 0x1A043B3C */
		{
				FIELD  VC6                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT6                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC7                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT7                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI_EXT_2;	/* SENINF4_CSI2_DI_EXT_2 */

typedef volatile union _SENINF4_REG_CSI2_DI_CTRL_EXT_2_
{
		volatile struct	/* 0x1A043B40 */
		{
				FIELD  VC6_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT6_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC7_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT7_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CSI2_DI_CTRL_EXT_2;	/* SENINF4_CSI2_DI_CTRL_EXT_2 */


typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG0_
{
		volatile struct	/* 0x1A043B44 */
		{
				FIELD  SYMBOL_STREAM0_CPHY3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG0;	/* SENINF4_WIRE_STATE_DECODE_CPHY3_DBG0 */

typedef volatile union _SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG1_
{
		volatile struct	/* 0x1A043B48 */
		{
				FIELD  SYMBOL_STREAM1_CPHY3                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY3             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG1;	/* SENINF4_WIRE_STATE_DECODE_CPHY3_DBG1 */


typedef volatile union _SENINF4_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A043D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_CTRL;	/* SENINF4_MUX_CTRL */

typedef volatile union _SENINF4_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A043D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_INTEN;	/* SENINF4_MUX_INTEN */

typedef volatile union _SENINF4_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A043D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_INTSTA;	/* SENINF4_MUX_INTSTA */

typedef volatile union _SENINF4_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A043D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_SIZE;	/* SENINF4_MUX_SIZE */

typedef volatile union _SENINF4_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A043D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_1;	/* SENINF4_MUX_DEBUG_1 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A043D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_2;	/* SENINF4_MUX_DEBUG_2 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A043D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_3;	/* SENINF4_MUX_DEBUG_3 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A043D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_4;	/* SENINF4_MUX_DEBUG_4 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A043D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_5;	/* SENINF4_MUX_DEBUG_5 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A043D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_6;	/* SENINF4_MUX_DEBUG_6 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A043D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_7;	/* SENINF4_MUX_DEBUG_7 */

typedef volatile union _SENINF4_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A043D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_SPARE;	/* SENINF4_MUX_SPARE */

typedef volatile union _SENINF4_REG_MUX_DATA_
{
		volatile struct	/* 0x1A043D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DATA;	/* SENINF4_MUX_DATA */

typedef volatile union _SENINF4_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A043D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DATA_CNT;	/* SENINF4_MUX_DATA_CNT */

typedef volatile union _SENINF4_REG_MUX_CROP_
{
		volatile struct	/* 0x1A043D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_CROP;	/* SENINF4_MUX_CROP */

typedef volatile union _SENINF4_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A043D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_CTRL_EXT;	/* SENINF4_MUX_CTRL_EXT */

typedef volatile union _SENINF5_REG_CTRL_
{
		volatile struct	/* 0x1A044200 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  OCSI2_SW_RST                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_SW_RST                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI3_SW_RST                           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  4;		/* 24..27, 0x0F000000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CTRL;	/* SENINF5_CTRL */

typedef volatile union _SENINF5_REG_CTRL_EXT_
{
		volatile struct	/* 0x1A044204 */
		{
				FIELD  SENINF_OCSI2_IP_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_TESTMDL_IP_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_SCAM_IP_EN                     :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_NCSI2_IP_EN                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_CSI2_IP_EN                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_CSI3_IP_EN                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CTRL_EXT;	/* SENINF5_CTRL_EXT */

typedef volatile union _SENINF5_REG_ASYNC_CTRL_
{
		volatile struct	/* 0x1A044208 */
		{
				FIELD  SENINF_ASYNC_FIFO_RST                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_ASYNC_CTRL;	/* SENINF5_ASYNC_CTRL */

typedef volatile union _SENINF_REG_TG5_PH_CNT_
{
		volatile struct	/* 0x1A044600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_PH_CNT;	/* SENINF_TG5_PH_CNT */

typedef volatile union _SENINF_REG_TG5_SEN_CK_
{
		volatile struct	/* 0x1A044604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_SEN_CK;	/* SENINF_TG5_SEN_CK */

typedef volatile union _SENINF_REG_TG5_TM_CTL_
{
		volatile struct	/* 0x1A044608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_TM_CTL;	/* SENINF_TG5_TM_CTL */

typedef volatile union _SENINF_REG_TG5_TM_SIZE_
{
		volatile struct	/* 0x1A04460C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_TM_SIZE;	/* SENINF_TG5_TM_SIZE */

typedef volatile union _SENINF_REG_TG5_TM_CLK_
{
		volatile struct	/* 0x1A044610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_TM_CLK;	/* SENINF_TG5_TM_CLK */

typedef volatile union _SENINF_REG_TG5_TM_STP_
{
		volatile struct	/* 0x1A044614 */
		{
				FIELD  TG1_TM_STP                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG5_TM_STP;	/* SENINF_TG5_TM_STP */

typedef volatile union _MIPI_REG_RX_CON24_CSI4_
{
		volatile struct	/* 0x1A044824 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI4;	/* MIPI_RX_CON24_CSI4 */

typedef volatile union _MIPI_REG_RX_CON28_CSI4_
{
		volatile struct	/* 0x1A044828 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI4;	/* MIPI_RX_CON28_CSI4 */

typedef volatile union _MIPI_REG_RX_CON34_CSI4_
{
		volatile struct	/* 0x1A044834 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 21;		/*  1..21, 0x003FFFFE */
				FIELD  CSI0_BIST_HSDET_MUX                   :  2;		/* 22..23, 0x00C00000 */
				FIELD  CSI0_BIST_LN4_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN5_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN6_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN7_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI4;	/* MIPI_RX_CON34_CSI4 */

typedef volatile union _MIPI_REG_RX_CON38_CSI4_
{
		volatile struct	/* 0x1A044838 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_SW_RST                        :  5;		/*  4.. 8, 0x000001F0 */
				FIELD  MIPI_RX_SW_CPHY_TX_MODE               :  1;		/*  9.. 9, 0x00000200 */
				FIELD  MIPI_RX_SW_CPHY_RX_MODE               :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  rg_ckphase_trio0                      :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  rg_ckphase_trio1                      :  5;		/* 22..26, 0x07C00000 */
				FIELD  rg_ckphase_trio2                      :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI4;	/* MIPI_RX_CON38_CSI4 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI4_
{
		volatile struct	/* 0x1A04483C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI4;	/* MIPI_RX_CON3C_CSI4 */

typedef volatile union _MIPI_REG_RX_CON7C_CSI4_
{
		volatile struct	/* 0x1A04487C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON7C_CSI4;	/* MIPI_RX_CON7C_CSI4 */

typedef volatile union _MIPI_REG_RX_CON80_CSI4_
{
		volatile struct	/* 0x1A044880 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON80_CSI4;	/* MIPI_RX_CON80_CSI4 */

typedef volatile union _MIPI_REG_RX_CON84_CSI4_
{
		volatile struct	/* 0x1A044884 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON84_CSI4;	/* MIPI_RX_CON84_CSI4 */

typedef volatile union _MIPI_REG_RX_CON88_CSI4_
{
		volatile struct	/* 0x1A044888 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON88_CSI4;	/* MIPI_RX_CON88_CSI4 */

typedef volatile union _MIPI_REG_RX_CON8C_CSI4_
{
		volatile struct	/* 0x1A04488C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON8C_CSI4;	/* MIPI_RX_CON8C_CSI4 */

typedef volatile union _MIPI_REG_RX_CON90_CSI4_
{
		volatile struct	/* 0x1A044890 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON90_CSI4;	/* MIPI_RX_CON90_CSI4 */

typedef volatile union _MIPI_REG_RX_CON94_CSI4_
{
		volatile struct	/* 0x1A044894 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON94_CSI4;	/* MIPI_RX_CON94_CSI4 */

typedef volatile union _MIPI_REG_RX_CON98_CSI4_
{
		volatile struct	/* 0x1A044898 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON98_CSI4;	/* MIPI_RX_CON98_CSI4 */

typedef volatile union _MIPI_REG_RX_CONA0_CSI4_
{
		volatile struct	/* 0x1A0448A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONA0_CSI4;	/* MIPI_RX_CONA0_CSI4 */

typedef volatile union _MIPI_REG_RX_CONB0_CSI4_
{
		volatile struct	/* 0x1A0448B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB0_CSI4;	/* MIPI_RX_CONB0_CSI4 */

typedef volatile union _MIPI_REG_RX_CONB4_CSI4_
{
		volatile struct	/* 0x1A0448B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB4_CSI4;	/* MIPI_RX_CONB4_CSI4 */

typedef volatile union _MIPI_REG_RX_CONB8_CSI4_
{
		volatile struct	/* 0x1A0448B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONB8_CSI4;	/* MIPI_RX_CONB8_CSI4 */

typedef volatile union _MIPI_REG_RX_CONBC_CSI4_
{
		volatile struct	/* 0x1A0448BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONBC_CSI4;	/* MIPI_RX_CONBC_CSI4 */

typedef volatile union _MIPI_REG_RX_CONC0_CSI4_
{
		volatile struct	/* 0x1A0448C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 15;		/* 16..30, 0x7FFF0000 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC0_CSI4;	/* MIPI_RX_CONC0_CSI4 */

typedef volatile union _MIPI_REG_RX_CONC4_CSI4_
{
		volatile struct	/* 0x1A0448C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC4_CSI4;	/* MIPI_RX_CONC4_CSI4 */

typedef volatile union _MIPI_REG_RX_CONC8_CSI4_
{
		volatile struct	/* 0x1A0448C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONC8_CSI4;	/* MIPI_RX_CONC8_CSI4 */

typedef volatile union _MIPI_REG_RX_CONCC_CSI4_
{
		volatile struct	/* 0x1A0448CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CONCC_CSI4;	/* MIPI_RX_CONCC_CSI4 */

typedef volatile union _MIPI_REG_RX_COND0_CSI4_
{
		volatile struct	/* 0x1A0448D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_COND0_CSI4;	/* MIPI_RX_COND0_CSI4 */

typedef volatile union _SENINF5_REG_CSI2_CTL_
{
		volatile struct	/* 0x1A044A00 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  rsv_20                                :  5;		/* 20..24, 0x01F00000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  rsv_26                                :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CTL;	/* SENINF5_CSI2_CTL */

typedef volatile union _SENINF5_REG_CSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A044A04 */
		{
				FIELD  CLOCK_TERM_PARAMETER                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CLOCK_SETTLE_PARAMETER                :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_LNRC_TIMING;	/* SENINF5_CSI2_LNRC_TIMING */

typedef volatile union _SENINF5_REG_CSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A044A08 */
		{
				FIELD  DATA_TERM_PARAMETER                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  DATA_SETTLE_PARAMETER                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_LNRD_TIMING;	/* SENINF5_CSI2_LNRD_TIMING */

typedef volatile union _SENINF5_REG_CSI2_DPCM_
{
		volatile struct	/* 0x1A044A0C */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DPCM;	/* SENINF5_CSI2_DPCM */

typedef volatile union _SENINF5_REG_CSI2_INT_EN_
{
		volatile struct	/* 0x1A044A10 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_CRC_NO_ERROR                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  FE                                    :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC                       :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6                     :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7                     :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT              :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT              :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT              :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT              :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_INT_EN;	/* SENINF5_CSI2_INT_EN */

typedef volatile union _SENINF5_REG_CSI2_INT_STATUS_
{
		volatile struct	/* 0x1A044A14 */
		{
				FIELD  ERR_FRAME_SYNC_STA                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID_STA                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR_STA                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED_STA                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE_STA                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC_STA                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC_STA               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0_STA             :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1_STA             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2_STA             :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3_STA             :  1;		/* 11..11, 0x00000800 */
				FIELD  FS_STA                                :  1;		/* 12..12, 0x00001000 */
				FIELD  LS_STA                                :  1;		/* 13..13, 0x00002000 */
				FIELD  GS_STA                                :  1;		/* 14..14, 0x00004000 */
				FIELD  FE_STA                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0_STA                 :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1_STA                 :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2_STA                 :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3_STA                 :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4_STA                 :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5_STA                 :  1;		/* 21..21, 0x00200000 */
				FIELD  ERR_LANE_RESYNC_STA                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ERR_FRAME_SYNC_S6_STA                 :  1;		/* 23..23, 0x00800000 */
				FIELD  ERR_FRAME_SYNC_S7_STA                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  1;		/* 25..25, 0x02000000 */
				FIELD  TRIO0_ESCAPE_CODE_DETECT_STA          :  1;		/* 26..26, 0x04000000 */
				FIELD  TRIO1_ESCAPE_CODE_DETECT_STA          :  1;		/* 27..27, 0x08000000 */
				FIELD  TRIO2_ESCAPE_CODE_DETECT_STA          :  1;		/* 28..28, 0x10000000 */
				FIELD  TRIO3_ESCAPE_CODE_DETECT_STA          :  1;		/* 29..29, 0x20000000 */
				FIELD  MERGE_FIFO_AF                         :  1;		/* 30..30, 0x40000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_INT_STATUS;	/* SENINF5_CSI2_INT_STATUS */

typedef volatile union _SENINF5_REG_CSI2_DGB_SEL_
{
		volatile struct	/* 0x1A044A18 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DEBUG_EN                              :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DGB_SEL;	/* SENINF5_CSI2_DGB_SEL */

typedef volatile union _SENINF5_REG_CSI2_DBG_PORT_
{
		volatile struct	/* 0x1A044A1C */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DBG_PORT;	/* SENINF5_CSI2_DBG_PORT */

typedef volatile union _SENINF5_REG_CSI2_SPARE0_
{
		volatile struct	/* 0x1A044A20 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_SPARE0;	/* SENINF5_CSI2_SPARE0 */

typedef volatile union _SENINF5_REG_CSI2_SPARE1_
{
		volatile struct	/* 0x1A044A24 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_SPARE1;	/* SENINF5_CSI2_SPARE1 */

typedef volatile union _SENINF5_REG_CSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A044A28 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_LNRC_FSM;	/* SENINF5_CSI2_LNRC_FSM */

typedef volatile union _SENINF5_REG_CSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A044A2C */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_LNRD_FSM;	/* SENINF5_CSI2_LNRD_FSM */

typedef volatile union _SENINF5_REG_CSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A044A30 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_FRAME_LINE_NUM;	/* SENINF5_CSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF5_REG_CSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A044A34 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_GENERIC_SHORT;	/* SENINF5_CSI2_GENERIC_SHORT */

typedef volatile union _SENINF5_REG_CSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A044A38 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_HSRX_DBG;	/* SENINF5_CSI2_HSRX_DBG */

typedef volatile union _SENINF5_REG_CSI2_DI_
{
		volatile struct	/* 0x1A044A3C */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI;	/* SENINF5_CSI2_DI */

typedef volatile union _SENINF5_REG_CSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A044A40 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_HS_TRAIL;	/* SENINF5_CSI2_HS_TRAIL */

typedef volatile union _SENINF5_REG_CSI2_DI_CTRL_
{
		volatile struct	/* 0x1A044A44 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI_CTRL;	/* SENINF5_CSI2_DI_CTRL */

typedef volatile union _SENINF5_REG_CSI2_DETECT_CON1_
{
		volatile struct	/* 0x1A044A4C */
		{
				FIELD  DETECT_SYNC_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_SYNC_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  SYNC_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_CON1;	/* SENINF5_CSI2_DETECT_CON1 */

typedef volatile union _SENINF5_REG_CSI2_DETECT_CON2_
{
		volatile struct	/* 0x1A044A50 */
		{
				FIELD  DETECT_ESCAPE_DISABLE                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_MASK                    :  7;		/*  1.. 7, 0x000000FE */
				FIELD  ESCAPE_WORD                           : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_CON2;	/* SENINF5_CSI2_DETECT_CON2 */

typedef volatile union _SENINF5_REG_CSI2_DETECT_CON3_
{
		volatile struct	/* 0x1A044A54 */
		{
				FIELD  DETECT_POST_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_POST_MASK                      :  7;		/*  1.. 7, 0x000000FE */
				FIELD  POST_WORD                             : 21;		/*  8..28, 0x1FFFFF00 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_CON3;	/* SENINF5_CSI2_DETECT_CON3 */

typedef volatile union _SENINF5_REG_CSI2_RLR0_CON0_
{
		volatile struct	/* 0x1A044A58 */
		{
				FIELD  RLR0_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR0_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR0_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR0_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RLR0_CON0;	/* SENINF5_CSI2_RLR0_CON0 */

typedef volatile union _SENINF5_REG_CSI2_RLR1_CON0_
{
		volatile struct	/* 0x1A044A5C */
		{
				FIELD  RLR1_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR1_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR1_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR1_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RLR1_CON0;	/* SENINF5_CSI2_RLR1_CON0 */

typedef volatile union _SENINF5_REG_CSI2_RLR2_CON0_
{
		volatile struct	/* 0x1A044A60 */
		{
				FIELD  RLR2_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR2_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR2_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR2_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RLR2_CON0;	/* SENINF5_CSI2_RLR2_CON0 */

typedef volatile union _SENINF5_REG_CSI2_RLR_CON0_
{
		volatile struct	/* 0x1A044A64 */
		{
				FIELD  RLRN_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLRN_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLRN_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLRN_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RLR_CON0;	/* SENINF5_CSI2_RLR_CON0 */

typedef volatile union _SENINF5_REG_CSI2_MUX_CON_
{
		volatile struct	/* 0x1A044A68 */
		{
				FIELD  DPHY_RX_EXTERNAL_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_TX_EXTERNAL_EN                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CPHY_RX_EXTERNAL_EN                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RLR_PATTERN_DELAY_EN                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  POST_PACKET_IGNORE_EN                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_MUX_CON;	/* SENINF5_CSI2_MUX_CON */

typedef volatile union _SENINF5_REG_CSI2_DETECT_DBG0_
{
		volatile struct	/* 0x1A044A6C */
		{
				FIELD  DETECT_SYNC_LANE0_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE0_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE0_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE0_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE0_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DETECT_SYNC_LANE1_ST                  :  1;		/* 12..12, 0x00001000 */
				FIELD  DETECT_ESCAPE_LANE1_ST                :  1;		/* 13..13, 0x00002000 */
				FIELD  DETECT_POST_LANE1_ST                  :  1;		/* 14..14, 0x00004000 */
				FIELD  POSITION_SYNC_LANE1_ST                :  4;		/* 15..18, 0x00078000 */
				FIELD  POSITION_ESCAPE_LANE1_ST              :  4;		/* 19..22, 0x00780000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  DETECT_SYNC_LANE2_ST                  :  1;		/* 24..24, 0x01000000 */
				FIELD  DETECT_ESCAPE_LANE2_ST                :  1;		/* 25..25, 0x02000000 */
				FIELD  DETECT_POST_LANE2_ST                  :  1;		/* 26..26, 0x04000000 */
				FIELD  POSITION_SYNC_LANE2_ST                :  4;		/* 27..30, 0x78000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_DBG0;	/* SENINF5_CSI2_DETECT_DBG0 */

typedef volatile union _SENINF5_REG_CSI2_DETECT_DBG1_
{
		volatile struct	/* 0x1A044A70 */
		{
				FIELD  POSITION_ESCAPE_LANE2_ST              :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_DBG1;	/* SENINF5_CSI2_DETECT_DBG1 */


typedef volatile union _SENINF5_REG_CSI2_RESYNC_MERGE_CTL_
{
		volatile struct	/* 0x1A044A74 */
		{
				FIELD  CPHY_LANE_RESYNC_CNT                  :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  LANE_RESYNC_FLUSH_EN                  :  1;		/*  8.. 8, 0x00000100 */
				FIELD  LANE_RESYNC_DATAOUT_OPTION            :  1;		/*  9.. 9, 0x00000200 */
				FIELD  BYPASS_LANE_RESYNC                    :  1;		/* 10..10, 0x00000400 */
				FIELD  CDPHY_SEL                             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RESYNC_MERGE_CTL;	/* SENINF5_CSI2_RESYNC_MERGE_CTL */

typedef volatile union _SENINF5_REG_CSI2_CTRL_TRIO_MUX_
{
		volatile struct	/* 0x1A044A78 */
		{
				FIELD  TRIO0_MUX                             :  3;		/*  0.. 2, 0x00000007 */
				FIELD  TRIO1_MUX                             :  3;		/*  3.. 5, 0x00000038 */
				FIELD  TRIO2_MUX                             :  3;		/*  6.. 8, 0x000001C0 */
				FIELD  TRIO3_MUX                             :  3;		/*  9..11, 0x00000E00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CTRL_TRIO_MUX;	/* SENINF5_CSI2_CTRL_TRIO_MUX */

typedef volatile union _SENINF5_REG_CSI2_CTRL_TRIO_CON_
{
		volatile struct	/* 0x1A044A7C */
		{
				FIELD  TRIO0_LPRX_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TRIO0_HSRX_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TRIO1_LPRX_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  TRIO1_HSRX_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TRIO2_LPRX_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TRIO2_HSRX_EN                         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  TRIO3_LPRX_EN                         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  TRIO3_HSRX_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CTRL_TRIO_CON;	/* SENINF5_CSI2_CTRL_TRIO_CON */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY0_DBG_
{
		volatile struct	/* 0x1A044A80 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY0                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY0           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY0_DBG;	/* SENINF5_FIX_ADDR_CPHY0_DBG */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY1_DBG_
{
		volatile struct	/* 0x1A044A84 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY1                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY1           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY1_DBG;	/* SENINF5_FIX_ADDR_CPHY1_DBG */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY2_DBG_
{
		volatile struct	/* 0x1A044A88 */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT_CPHY2                     :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT_CPHY2           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY2_DBG;	/* SENINF5_FIX_ADDR_CPHY2_DBG */

typedef volatile union _SENINF5_REG_FIX_ADDR_DBG_
{
		volatile struct	/* 0x1A044A8C */
		{
				FIELD  rsv_0                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_COUNT                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  WORD_COUNT_OVER_FLOAT                 :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_DBG;	/* SENINF5_FIX_ADDR_DBG */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG0_
{
		volatile struct	/* 0x1A044A90 */
		{
				FIELD  SYMBOL_STREAM0_CPHY0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG0;	/* SENINF5_WIRE_STATE_DECODE_CPHY0_DBG0 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG1_
{
		volatile struct	/* 0x1A044A94 */
		{
				FIELD  SYMBOL_STREAM1_CPHY0                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY0             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG1;	/* SENINF5_WIRE_STATE_DECODE_CPHY0_DBG1 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG0_
{
		volatile struct	/* 0x1A044A98 */
		{
				FIELD  SYMBOL_STREAM0_CPHY1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG0;	/* SENINF5_WIRE_STATE_DECODE_CPHY1_DBG0 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG1_
{
		volatile struct	/* 0x1A044A9C */
		{
				FIELD  SYMBOL_STREAM1_CPHY1                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY1             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG1;	/* SENINF5_WIRE_STATE_DECODE_CPHY1_DBG1 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG0_
{
		volatile struct	/* 0x1A044AA0 */
		{
				FIELD  SYMBOL_STREAM0_CPHY2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG0;	/* SENINF5_WIRE_STATE_DECODE_CPHY2_DBG0 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG1_
{
		volatile struct	/* 0x1A044AA4 */
		{
				FIELD  SYMBOL_STREAM1_CPHY2                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY2             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG1;	/* SENINF5_WIRE_STATE_DECODE_CPHY2_DBG1 */

typedef volatile union _SENINF5_REG_SYNC_RESYNC_CTL_
{
		volatile struct	/* 0x1A044AA8 */
		{
				FIELD  SYNC_DETECTION_SEL                    :  3;		/*  0.. 2, 0x00000007 */
				FIELD  FLUSH_VALID                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_SYNC_RESYNC_CTL;	/* SENINF5_SYNC_RESYNC_CTL */

typedef volatile union _SENINF5_REG_POST_DETECT_CTL_
{
		volatile struct	/* 0x1A044AAC */
		{
				FIELD  POST_DETECT_DISABLE                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  POST_EN                               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_POST_DETECT_CTL;	/* SENINF5_POST_DETECT_CTL */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CONFIG_
{
		volatile struct	/* 0x1A044AB0 */
		{
				FIELD  INIT_STATE_DECODE                     :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CONFIG;	/* SENINF5_WIRE_STATE_DECODE_CONFIG */

typedef volatile union _SENINF5_REG_CSI2_CPHY_LNRD_FSM_
{
		volatile struct	/* 0x1A044AB4 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  TRIO0_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  TRIO1_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  TRIO2_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CPHY_LNRD_FSM;	/* SENINF5_CSI2_CPHY_LNRD_FSM */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY0_DBG0_
{
		volatile struct	/* 0x1A044AB8 */
		{
				FIELD  WORD_COUNT_CPHY0_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY0_DBG0;	/* SENINF5_FIX_ADDR_CPHY0_DBG0 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY0_DBG1_
{
		volatile struct	/* 0x1A044ABC */
		{
				FIELD  WORD_COUNT_CPHY0_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY0_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY0_DBG1;	/* SENINF5_FIX_ADDR_CPHY0_DBG1 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY0_DBG2_
{
		volatile struct	/* 0x1A044AC0 */
		{
				FIELD  ERROR_RECORD_CPHY0_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY0_DBG2;	/* SENINF5_FIX_ADDR_CPHY0_DBG2 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY1_DBG0_
{
		volatile struct	/* 0x1A044AC4 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY1_DBG0;	/* SENINF5_FIX_ADDR_CPHY1_DBG0 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY1_DBG1_
{
		volatile struct	/* 0x1A044AC8 */
		{
				FIELD  WORD_COUNT_CPHY1_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY1_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY1_DBG1;	/* SENINF5_FIX_ADDR_CPHY1_DBG1 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY1_DBG2_
{
		volatile struct	/* 0x1A044ACC */
		{
				FIELD  ERROR_RECORD_CPHY1_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY1_DBG2;	/* SENINF5_FIX_ADDR_CPHY1_DBG2 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY2_DBG0_
{
		volatile struct	/* 0x1A044AD0 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY2_DBG0;	/* SENINF5_FIX_ADDR_CPHY2_DBG0 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY2_DBG1_
{
		volatile struct	/* 0x1A044AD4 */
		{
				FIELD  WORD_COUNT_CPHY2_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY2_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY2_DBG1;	/* SENINF5_FIX_ADDR_CPHY2_DBG1 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY2_DBG2_
{
		volatile struct	/* 0x1A044AD8 */
		{
				FIELD  ERROR_RECORD_CPHY2_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY2_DBG2;	/* SENINF5_FIX_ADDR_CPHY2_DBG2 */

typedef volatile union _SENINF5_REG_FIX_ADDR_DBG0_
{
		volatile struct	/* 0x1A044ADC */
		{
				FIELD  WORD_COUNT_DBG0                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_DBG0;	/* SENINF5_FIX_ADDR_DBG0 */

typedef volatile union _SENINF5_REG_FIX_ADDR_DBG1_
{
		volatile struct	/* 0x1A044AE0 */
		{
				FIELD  WORD_COUNT_DBG1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_DBG0                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_DBG1;	/* SENINF5_FIX_ADDR_DBG1 */

typedef volatile union _SENINF5_REG_FIX_ADDR_DBG2_
{
		volatile struct	/* 0x1A044AE4 */
		{
				FIELD  ERROR_RECORD_DBG1                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_DBG2;	/* SENINF5_FIX_ADDR_DBG2 */

typedef volatile union _SENINF5_REG_CSI2_MODE_
{
		volatile struct	/* 0x1A044AE8 */
		{
				FIELD  csr_csi2_mode                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  csr_csi2_header_len                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  csr_cphy_di_pos                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  csr_cphy_wc_pos                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_MODE;	/* SENINF5_CSI2_MODE */

typedef volatile union _SENINF5_REG_CSI2_DI_EXT_
{
		volatile struct	/* 0x1A044AF0 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI_EXT;	/* SENINF5_CSI2_DI_EXT */

typedef volatile union _SENINF5_REG_CSI2_DI_CTRL_EXT_
{
		volatile struct	/* 0x1A044AF4 */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI_CTRL_EXT;	/* SENINF5_CSI2_DI_CTRL_EXT */

typedef volatile union _SENINF5_REG_CSI2_CPHY_LOOPBACK_
{
		volatile struct	/* 0x1A044AF8 */
		{
				FIELD  TRIGGER_SYNC_INIT                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RELEASE_SYNC_INIT                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CPHY_LOOPBACK;	/* SENINF5_CSI2_CPHY_LOOPBACK */

typedef volatile union _SENINF5_REG_CSI2_PROGSEQ_0_
{
		volatile struct	/* 0x1A044B00 */
		{
				FIELD  PROGSEQ_S0                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S1                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S2                            :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S3                            :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S4                            :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S5                            :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  PROGSEQ_S6                            :  3;		/* 24..26, 0x07000000 */
				FIELD  rsv_27                                :  1;		/* 27..27, 0x08000000 */
				FIELD  PROGSEQ_S7                            :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_PROGSEQ_0;	/* SENINF5_CSI2_PROGSEQ_0 */

typedef volatile union _SENINF5_REG_CSI2_PROGSEQ_1_
{
		volatile struct	/* 0x1A044B04 */
		{
				FIELD  PROGSEQ_S8                            :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  PROGSEQ_S9                            :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  PROGSEQ_S10                           :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  PROGSEQ_S11                           :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  PROGSEQ_S12                           :  3;		/* 16..18, 0x00070000 */
				FIELD  rsv_19                                :  1;		/* 19..19, 0x00080000 */
				FIELD  PROGSEQ_S13                           :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_PROGSEQ_1;	/* SENINF5_CSI2_PROGSEQ_1 */


typedef volatile union _SENINF5_REG_CSI2_INT_EN_EXT_
{
		volatile struct	/* 0x1A044B10 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW            :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 19;		/* 12..30, 0x7FFFF000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_INT_EN_EXT;	/* SENINF5_CSI2_INT_EN_EXT */

typedef volatile union _SENINF5_REG_CSI2_INT_STATUS_EXT_
{
		volatile struct	/* 0x1A044B14 */
		{
				FIELD  rsv_0                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DPHY0_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DPHY1_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DPHY2_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  DPHY3_RESYNC_FIFO_OVERFLOW_STA        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  ERR_SOT_SYNC_HS_TRIO0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_TRIO1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_TRIO2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_TRIO3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_INT_STATUS_EXT;	/* SENINF5_CSI2_INT_STATUS_EXT */

typedef volatile union _SENINF5_REG_CSI2_CPHY_FIX_POINT_RST_
{
		volatile struct	/* 0x1A044B18 */
		{
				FIELD  CPHY_FIX_POINT_RST                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CPHY_FIX_POINT_RST_MODE               :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_CPHY_FIX_POINT_RST;	/* SENINF5_CSI2_CPHY_FIX_POINT_RST */

typedef volatile union _SENINF5_REG_CSI2_RLR3_CON0_
{
		volatile struct	/* 0x1A044B1C */
		{
				FIELD  RLR3_PRBS_PATTERN_SEL                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  RLR3_PRBS_SEED_0                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  RLR3_PRBS_SEED_1                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  RLR3_PRBS_SEED_2                      :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_RLR3_CON0;	/* SENINF5_CSI2_RLR3_CON0 */

typedef volatile union _SENINF5_REG_CSI2_DPHY_SYNC_
{
		volatile struct	/* 0x1A044B20 */
		{
				FIELD  SYNC_SEQ_MASK_0                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_0                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DPHY_SYNC;	/* SENINF5_CSI2_DPHY_SYNC */

typedef volatile union _SENINF5_REG_CSI2_DESKEW_SYNC_
{
		volatile struct	/* 0x1A044B24 */
		{
				FIELD  SYNC_SEQ_MASK_1                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SYNC_SEQ_PAT_1                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DESKEW_SYNC;	/* SENINF5_CSI2_DESKEW_SYNC */

typedef volatile union _SENINF5_REG_CSI2_DETECT_DBG2_
{
		volatile struct	/* 0x1A044B28 */
		{
				FIELD  DETECT_SYNC_LANE3_ST                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DETECT_ESCAPE_LANE3_ST                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DETECT_POST_LANE3_ST                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  POSITION_SYNC_LANE3_ST                :  4;		/*  3.. 6, 0x00000078 */
				FIELD  POSITION_ESCAPE_LANE3_ST              :  4;		/*  7..10, 0x00000780 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DETECT_DBG2;	/* SENINF5_CSI2_DETECT_DBG2 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY3_DBG0_
{
		volatile struct	/* 0x1A044B30 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY3_DBG0;	/* SENINF5_FIX_ADDR_CPHY3_DBG0 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY3_DBG1_
{
		volatile struct	/* 0x1A044B34 */
		{
				FIELD  WORD_COUNT_CPHY3_DBG1                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERROR_RECORD_CPHY3_DBG0               : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY3_DBG1;	/* SENINF5_FIX_ADDR_CPHY3_DBG1 */

typedef volatile union _SENINF5_REG_FIX_ADDR_CPHY3_DBG2_
{
		volatile struct	/* 0x1A044B38 */
		{
				FIELD  ERROR_RECORD_CPHY3_DBG1               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_FIX_ADDR_CPHY3_DBG2;	/* SENINF5_FIX_ADDR_CPHY3_DBG2 */


typedef volatile union _SENINF5_REG_CSI2_DI_EXT_2_
{
		volatile struct	/* 0x1A044B3C */
		{
				FIELD  VC6                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT6                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC7                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT7                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI_EXT_2;	/* SENINF5_CSI2_DI_EXT_2 */

typedef volatile union _SENINF5_REG_CSI2_DI_CTRL_EXT_2_
{
		volatile struct	/* 0x1A044B40 */
		{
				FIELD  VC6_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT6_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC7_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT7_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_CSI2_DI_CTRL_EXT_2;	/* SENINF5_CSI2_DI_CTRL_EXT_2 */


typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG0_
{
		volatile struct	/* 0x1A044B44 */
		{
				FIELD  SYMBOL_STREAM0_CPHY3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG0;	/* SENINF5_WIRE_STATE_DECODE_CPHY3_DBG0 */

typedef volatile union _SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG1_
{
		volatile struct	/* 0x1A044B48 */
		{
				FIELD  SYMBOL_STREAM1_CPHY3                  : 10;		/*  0.. 9, 0x000003FF */
				FIELD  SYMBOL_STREAM_VALID_CPHY3             :  1;		/* 10..10, 0x00000400 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG1;	/* SENINF5_WIRE_STATE_DECODE_CPHY3_DBG1 */

typedef volatile union _SENINF5_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A044D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_CTRL;	/* SENINF5_MUX_CTRL */

typedef volatile union _SENINF5_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A044D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_INTEN;	/* SENINF5_MUX_INTEN */

typedef volatile union _SENINF5_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A044D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_INTSTA;	/* SENINF5_MUX_INTSTA */

typedef volatile union _SENINF5_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A044D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_SIZE;	/* SENINF5_MUX_SIZE */

typedef volatile union _SENINF5_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A044D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_1;	/* SENINF5_MUX_DEBUG_1 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A044D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_2;	/* SENINF5_MUX_DEBUG_2 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A044D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_3;	/* SENINF5_MUX_DEBUG_3 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A044D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_4;	/* SENINF5_MUX_DEBUG_4 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A044D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_5;	/* SENINF5_MUX_DEBUG_5 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A044D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_6;	/* SENINF5_MUX_DEBUG_6 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A044D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_7;	/* SENINF5_MUX_DEBUG_7 */

typedef volatile union _SENINF5_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A044D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_SPARE;	/* SENINF5_MUX_SPARE */

typedef volatile union _SENINF5_REG_MUX_DATA_
{
		volatile struct	/* 0x1A044D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DATA;	/* SENINF5_MUX_DATA */

typedef volatile union _SENINF5_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A044D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DATA_CNT;	/* SENINF5_MUX_DATA_CNT */

typedef volatile union _SENINF5_REG_MUX_CROP_
{
		volatile struct	/* 0x1A044D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_CROP;	/* SENINF5_MUX_CROP */

typedef volatile union _SENINF5_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A044D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_CTRL_EXT;	/* SENINF5_MUX_CTRL_EXT */

typedef volatile union _SENINF6_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A045D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_CTRL;	/* SENINF6_MUX_CTRL */

typedef volatile union _SENINF6_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A045D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_INTEN;	/* SENINF6_MUX_INTEN */

typedef volatile union _SENINF6_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A045D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_INTSTA;	/* SENINF6_MUX_INTSTA */

typedef volatile union _SENINF6_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A045D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_SIZE;	/* SENINF6_MUX_SIZE */

typedef volatile union _SENINF6_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A045D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_1;	/* SENINF6_MUX_DEBUG_1 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A045D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_2;	/* SENINF6_MUX_DEBUG_2 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A045D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_3;	/* SENINF6_MUX_DEBUG_3 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A045D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_4;	/* SENINF6_MUX_DEBUG_4 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A045D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_5;	/* SENINF6_MUX_DEBUG_5 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A045D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_6;	/* SENINF6_MUX_DEBUG_6 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A045D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_7;	/* SENINF6_MUX_DEBUG_7 */

typedef volatile union _SENINF6_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A045D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_SPARE;	/* SENINF6_MUX_SPARE */

typedef volatile union _SENINF6_REG_MUX_DATA_
{
		volatile struct	/* 0x1A045D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DATA;	/* SENINF6_MUX_DATA */

typedef volatile union _SENINF6_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A045D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DATA_CNT;	/* SENINF6_MUX_DATA_CNT */

typedef volatile union _SENINF6_REG_MUX_CROP_
{
		volatile struct	/* 0x1A045D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_CROP;	/* SENINF6_MUX_CROP */

typedef volatile union _SENINF6_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A045D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_CTRL_EXT;	/* SENINF6_MUX_CTRL_EXT */

typedef volatile union _SENINF7_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A046D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_CTRL;	/* SENINF7_MUX_CTRL */

typedef volatile union _SENINF7_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A046D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_INTEN;	/* SENINF7_MUX_INTEN */

typedef volatile union _SENINF7_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A046D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_INTSTA;	/* SENINF7_MUX_INTSTA */

typedef volatile union _SENINF7_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A046D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_SIZE;	/* SENINF7_MUX_SIZE */

typedef volatile union _SENINF7_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A046D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_1;	/* SENINF7_MUX_DEBUG_1 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A046D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_2;	/* SENINF7_MUX_DEBUG_2 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A046D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_3;	/* SENINF7_MUX_DEBUG_3 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A046D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_4;	/* SENINF7_MUX_DEBUG_4 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A046D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_5;	/* SENINF7_MUX_DEBUG_5 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A046D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_6;	/* SENINF7_MUX_DEBUG_6 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A046D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_7;	/* SENINF7_MUX_DEBUG_7 */

typedef volatile union _SENINF7_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A046D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_SPARE;	/* SENINF7_MUX_SPARE */

typedef volatile union _SENINF7_REG_MUX_DATA_
{
		volatile struct	/* 0x1A046D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DATA;	/* SENINF7_MUX_DATA */

typedef volatile union _SENINF7_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A046D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DATA_CNT;	/* SENINF7_MUX_DATA_CNT */

typedef volatile union _SENINF7_REG_MUX_CROP_
{
		volatile struct	/* 0x1A046D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_CROP;	/* SENINF7_MUX_CROP */

typedef volatile union _SENINF7_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A046D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_CTRL_EXT;	/* SENINF7_MUX_CTRL_EXT */

typedef volatile union _SENINF8_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A047D00 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_MUX_RDY_MODE                   :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_MUX_RDY_VALUE                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_CTRL;	/* SENINF8_MUX_CTRL */

typedef volatile union _SENINF8_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A047D04 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_INTEN;	/* SENINF8_MUX_INTEN */

typedef volatile union _SENINF8_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A047D08 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_INTSTA;	/* SENINF8_MUX_INTSTA */

typedef volatile union _SENINF8_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A047D0C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_SIZE;	/* SENINF8_MUX_SIZE */

typedef volatile union _SENINF8_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A047D10 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_1;	/* SENINF8_MUX_DEBUG_1 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A047D14 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_2;	/* SENINF8_MUX_DEBUG_2 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A047D18 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_3;	/* SENINF8_MUX_DEBUG_3 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A047D1C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_4;	/* SENINF8_MUX_DEBUG_4 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A047D20 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_5;	/* SENINF8_MUX_DEBUG_5 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A047D24 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_6;	/* SENINF8_MUX_DEBUG_6 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A047D28 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_7;	/* SENINF8_MUX_DEBUG_7 */

typedef volatile union _SENINF8_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A047D2C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_SPARE;	/* SENINF8_MUX_SPARE */

typedef volatile union _SENINF8_REG_MUX_DATA_
{
		volatile struct	/* 0x1A047D30 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DATA;	/* SENINF8_MUX_DATA */

typedef volatile union _SENINF8_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A047D34 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DATA_CNT;	/* SENINF8_MUX_DATA_CNT */

typedef volatile union _SENINF8_REG_MUX_CROP_
{
		volatile struct	/* 0x1A047D38 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_CROP;	/* SENINF8_MUX_CROP */

typedef volatile union _SENINF8_REG_MUX_CTRL_EXT_
{
		volatile struct	/* 0x1A047D3C */
		{
				FIELD  SENINF_SRC_SEL_EXT                    :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  SENINF_PIX_SEL_EXT                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_CTRL_EXT;	/* SENINF8_MUX_CTRL_EXT */

typedef volatile struct _seninf_top_ctl_reg_base_t_
{
	SENINF_REG_TOP_CTRL 							SENINF_TOP_CTRL;								 /* 0000, 0x1A040000 */
	SENINF_REG_TOP_CMODEL_PAR						SENINF_TOP_CMODEL_PAR;							 /* 0004, 0x1A040004 */
	SENINF_REG_TOP_MUX_CTRL 						SENINF_TOP_MUX_CTRL;							 /* 0008, 0x1A040008 */
	UINT32											rsv_000C;										 /* 000C, 0x1A04000C */
	SENINF_REG_TOP_CAM_MUX_CTRL 					SENINF_TOP_CAM_MUX_CTRL;						 /* 0010, 0x1A040010 */
	SENINF_REG_TOP_N3D_A_CTL						SENINF_TOP_N3D_A_CTL;							 /* 0014, 0x1A040014 */
	SENINF_REG_TOP_N3D_B_CTL						SENINF_TOP_N3D_B_CTL;							 /* 0018, 0x1A040018 */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI0				SENINF_TOP_PHY_SENINF_CTL_CSI0; 				 /* 001C, 0x1A04001C */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI1				SENINF_TOP_PHY_SENINF_CTL_CSI1; 				 /* 0020, 0x1A040020 */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI2				SENINF_TOP_PHY_SENINF_CTL_CSI2; 				 /* 0024, 0x1A040024 */
}seninf_top_ctl_reg_base;

typedef volatile struct _seninf_ctl_reg_base_t_
{
	SENINF1_REG_CTRL								SENINF1_CTRL;									 /* 0200, 0x1A040200 */
	SENINF1_REG_CTRL_EXT							SENINF1_CTRL_EXT;								 /* 0204, 0x1A040204 */
	SENINF1_REG_ASYNC_CTRL							SENINF1_ASYNC_CTRL; 							 /* 0208, 0x1A040208 */
}seninf_ctl_reg_base;

typedef volatile struct _seninf_tg_reg_base_t_
{
	SENINF_REG_TG1_PH_CNT                           SENINF_TG1_PH_CNT;                               /* 0600, 0x1A040600 */
	SENINF_REG_TG1_SEN_CK                           SENINF_TG1_SEN_CK;                               /* 0604, 0x1A040604 */
	SENINF_REG_TG1_TM_CTL                           SENINF_TG1_TM_CTL;                               /* 0608, 0x1A040608 */
	SENINF_REG_TG1_TM_SIZE                          SENINF_TG1_TM_SIZE;                              /* 060C, 0x1A04060C */
	SENINF_REG_TG1_TM_CLK                           SENINF_TG1_TM_CLK;                               /* 0610, 0x1A040610 */
	SENINF_REG_TG1_TM_STP                           SENINF_TG1_TM_STP;                               /* 0614, 0x1A040614 */
}seninf_tg_reg_base;

typedef volatile struct _mipi_rx_con_reg_base_t_
{
	MIPI_REG_RX_CON24_CSI0                          MIPI_RX_CON24_CSI0;                              /* 0824, 0x1A040824 */
	MIPI_REG_RX_CON28_CSI0                          MIPI_RX_CON28_CSI0;                              /* 0828, 0x1A040828 */
	UINT32                                          rsv_082C[2];                                     /* 082C..0833, 0x1A04082C..1A040833 */
	MIPI_REG_RX_CON34_CSI0                          MIPI_RX_CON34_CSI0;                              /* 0834, 0x1A040834 */
	MIPI_REG_RX_CON38_CSI0                          MIPI_RX_CON38_CSI0;                              /* 0838, 0x1A040838 */
	MIPI_REG_RX_CON3C_CSI0                          MIPI_RX_CON3C_CSI0;                              /* 083C, 0x1A04083C */
	UINT32                                          rsv_0840[15];                                    /* 0840..087B, 0x1A040840..1A04087B */
	MIPI_REG_RX_CON7C_CSI0                          MIPI_RX_CON7C_CSI0;                              /* 087C, 0x1A04087C */
	MIPI_REG_RX_CON80_CSI0                          MIPI_RX_CON80_CSI0;                              /* 0880, 0x1A040880 */
	MIPI_REG_RX_CON84_CSI0                          MIPI_RX_CON84_CSI0;                              /* 0884, 0x1A040884 */
	MIPI_REG_RX_CON88_CSI0                          MIPI_RX_CON88_CSI0;                              /* 0888, 0x1A040888 */
	MIPI_REG_RX_CON8C_CSI0                          MIPI_RX_CON8C_CSI0;                              /* 088C, 0x1A04088C */
	MIPI_REG_RX_CON90_CSI0                          MIPI_RX_CON90_CSI0;                              /* 0890, 0x1A040890 */
	MIPI_REG_RX_CON94_CSI0                          MIPI_RX_CON94_CSI0;                              /* 0894, 0x1A040894 */
	MIPI_REG_RX_CON98_CSI0                          MIPI_RX_CON98_CSI0;                              /* 0898, 0x1A040898 */
	UINT32                                          rsv_089C;                                        /* 089C, 0x1A04089C */
	MIPI_REG_RX_CONA0_CSI0                          MIPI_RX_CONA0_CSI0;                              /* 08A0, 0x1A0408A0 */
	UINT32                                          rsv_08A4[3];                                     /* 08A4..08AF, 0x1A0408A4..1A0408AF */
	MIPI_REG_RX_CONB0_CSI0                          MIPI_RX_CONB0_CSI0;                              /* 08B0, 0x1A0408B0 */
	MIPI_REG_RX_CONB4_CSI0                          MIPI_RX_CONB4_CSI0;                              /* 08B4, 0x1A0408B4 */
	MIPI_REG_RX_CONB8_CSI0                          MIPI_RX_CONB8_CSI0;                              /* 08B8, 0x1A0408B8 */
	MIPI_REG_RX_CONBC_CSI0                          MIPI_RX_CONBC_CSI0;                              /* 08BC, 0x1A0408BC */
	MIPI_REG_RX_CONC0_CSI0                          MIPI_RX_CONC0_CSI0;                              /* 08C0, 0x1A0408C0 */
	MIPI_REG_RX_CONC4_CSI0                          MIPI_RX_CONC4_CSI0;                              /* 08C4, 0x1A0408C4 */
	MIPI_REG_RX_CONC8_CSI0                          MIPI_RX_CONC8_CSI0;                              /* 08C8, 0x1A0408C8 */
	MIPI_REG_RX_CONCC_CSI0                          MIPI_RX_CONCC_CSI0;                              /* 08CC, 0x1A0408CC */
	MIPI_REG_RX_COND0_CSI0                          MIPI_RX_COND0_CSI0;                              /* 08D0, 0x1A0408D0 */
}mipi_rx_con_reg_base;

typedef volatile struct _seninf_csi2_ctl_reg_base_t_
{
	SENINF1_REG_CSI2_CTL                            SENINF1_CSI2_CTL;                                /* 0A00, 0x1A040A00 */
	SENINF1_REG_CSI2_LNRC_TIMING                    SENINF1_CSI2_LNRC_TIMING;                        /* 0A04, 0x1A040A04 */
	SENINF1_REG_CSI2_LNRD_TIMING                    SENINF1_CSI2_LNRD_TIMING;                        /* 0A08, 0x1A040A08 */
	SENINF1_REG_CSI2_DPCM                           SENINF1_CSI2_DPCM;                               /* 0A0C, 0x1A040A0C */
	SENINF1_REG_CSI2_INT_EN                         SENINF1_CSI2_INT_EN;                             /* 0A10, 0x1A040A10 */
	SENINF1_REG_CSI2_INT_STATUS                     SENINF1_CSI2_INT_STATUS;                         /* 0A14, 0x1A040A14 */
	SENINF1_REG_CSI2_DGB_SEL                        SENINF1_CSI2_DGB_SEL;                            /* 0A18, 0x1A040A18 */
	SENINF1_REG_CSI2_DBG_PORT                       SENINF1_CSI2_DBG_PORT;                           /* 0A1C, 0x1A040A1C */
	SENINF1_REG_CSI2_SPARE0                         SENINF1_CSI2_SPARE0;                             /* 0A20, 0x1A040A20 */
	SENINF1_REG_CSI2_SPARE1                         SENINF1_CSI2_SPARE1;                             /* 0A24, 0x1A040A24 */
	SENINF1_REG_CSI2_LNRC_FSM                       SENINF1_CSI2_LNRC_FSM;                           /* 0A28, 0x1A040A28 */
	SENINF1_REG_CSI2_LNRD_FSM                       SENINF1_CSI2_LNRD_FSM;                           /* 0A2C, 0x1A040A2C */
	SENINF1_REG_CSI2_FRAME_LINE_NUM                 SENINF1_CSI2_FRAME_LINE_NUM;                     /* 0A30, 0x1A040A30 */
	SENINF1_REG_CSI2_GENERIC_SHORT                  SENINF1_CSI2_GENERIC_SHORT;                      /* 0A34, 0x1A040A34 */
	SENINF1_REG_CSI2_HSRX_DBG                       SENINF1_CSI2_HSRX_DBG;                           /* 0A38, 0x1A040A38 */
	SENINF1_REG_CSI2_DI                             SENINF1_CSI2_DI;                                 /* 0A3C, 0x1A040A3C */
	SENINF1_REG_CSI2_HS_TRAIL                       SENINF1_CSI2_HS_TRAIL;                           /* 0A40, 0x1A040A40 */
	SENINF1_REG_CSI2_DI_CTRL                        SENINF1_CSI2_DI_CTRL;                            /* 0A44, 0x1A040A44 */
	UINT32                                          rsv_0A48;                                        /* 0A48, 0x1A040A48 */
	SENINF1_REG_CSI2_DETECT_CON1                    SENINF1_CSI2_DETECT_CON1;                        /* 0A4C, 0x1A040A4C */
	SENINF1_REG_CSI2_DETECT_CON2                    SENINF1_CSI2_DETECT_CON2;                        /* 0A50, 0x1A040A50 */
	SENINF1_REG_CSI2_DETECT_CON3                    SENINF1_CSI2_DETECT_CON3;                        /* 0A54, 0x1A040A54 */
	SENINF1_REG_CSI2_RLR0_CON0                      SENINF1_CSI2_RLR0_CON0;                          /* 0A58, 0x1A040A58 */
	SENINF1_REG_CSI2_RLR1_CON0                      SENINF1_CSI2_RLR1_CON0;                          /* 0A5C, 0x1A040A5C */
	SENINF1_REG_CSI2_RLR2_CON0                      SENINF1_CSI2_RLR2_CON0;                          /* 0A60, 0x1A040A60 */
	SENINF1_REG_CSI2_RLR_CON0                       SENINF1_CSI2_RLR_CON0;                           /* 0A64, 0x1A040A64 */
	SENINF1_REG_CSI2_MUX_CON                        SENINF1_CSI2_MUX_CON;                            /* 0A68, 0x1A040A68 */
	SENINF1_REG_CSI2_DETECT_DBG0                    SENINF1_CSI2_DETECT_DBG0;                        /* 0A6C, 0x1A040A6C */
	SENINF1_REG_CSI2_DETECT_DBG1                    SENINF1_CSI2_DETECT_DBG1;                        /* 0A70, 0x1A040A70 */
	SENINF1_REG_CSI2_RESYNC_MERGE_CTL               SENINF1_CSI2_RESYNC_MERGE_CTL;                   /* 0A74, 0x1A040A74 */
	SENINF1_REG_CSI2_CTRL_TRIO_MUX                  SENINF1_CSI2_CTRL_TRIO_MUX;                      /* 0A78, 0x1A040A78 */
	SENINF1_REG_CSI2_CTRL_TRIO_CON                  SENINF1_CSI2_CTRL_TRIO_CON;                      /* 0A7C, 0x1A040A7C */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG                  SENINF1_FIX_ADDR_CPHY0_DBG;                      /* 0A80, 0x1A040A80 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG                  SENINF1_FIX_ADDR_CPHY1_DBG;                      /* 0A84, 0x1A040A84 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG                  SENINF1_FIX_ADDR_CPHY2_DBG;                      /* 0A88, 0x1A040A88 */
	SENINF1_REG_FIX_ADDR_DBG                        SENINF1_FIX_ADDR_DBG;                            /* 0A8C, 0x1A040A8C */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 0A90, 0x1A040A90 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 0A94, 0x1A040A94 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 0A98, 0x1A040A98 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 0A9C, 0x1A040A9C */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 0AA0, 0x1A040AA0 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 0AA4, 0x1A040AA4 */
	SENINF1_REG_SYNC_RESYNC_CTL                     SENINF1_SYNC_RESYNC_CTL;                         /* 0AA8, 0x1A040AA8 */
	SENINF1_REG_POST_DETECT_CTL                     SENINF1_POST_DETECT_CTL;                         /* 0AAC, 0x1A040AAC */
	SENINF1_REG_WIRE_STATE_DECODE_CONFIG            SENINF1_WIRE_STATE_DECODE_CONFIG;                /* 0AB0, 0x1A040AB0 */
	SENINF1_REG_CSI2_CPHY_LNRD_FSM                  SENINF1_CSI2_CPHY_LNRD_FSM;                      /* 0AB4, 0x1A040AB4 */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG0                 SENINF1_FIX_ADDR_CPHY0_DBG0;                     /* 0AB8, 0x1A040AB8 */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG1                 SENINF1_FIX_ADDR_CPHY0_DBG1;                     /* 0ABC, 0x1A040ABC */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG2                 SENINF1_FIX_ADDR_CPHY0_DBG2;                     /* 0AC0, 0x1A040AC0 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG0                 SENINF1_FIX_ADDR_CPHY1_DBG0;                     /* 0AC4, 0x1A040AC4 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG1                 SENINF1_FIX_ADDR_CPHY1_DBG1;                     /* 0AC8, 0x1A040AC8 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG2                 SENINF1_FIX_ADDR_CPHY1_DBG2;                     /* 0ACC, 0x1A040ACC */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG0                 SENINF1_FIX_ADDR_CPHY2_DBG0;                     /* 0AD0, 0x1A040AD0 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG1                 SENINF1_FIX_ADDR_CPHY2_DBG1;                     /* 0AD4, 0x1A040AD4 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG2                 SENINF1_FIX_ADDR_CPHY2_DBG2;                     /* 0AD8, 0x1A040AD8 */
	SENINF1_REG_FIX_ADDR_DBG0                       SENINF1_FIX_ADDR_DBG0;                           /* 0ADC, 0x1A040ADC */
	SENINF1_REG_FIX_ADDR_DBG1                       SENINF1_FIX_ADDR_DBG1;                           /* 0AE0, 0x1A040AE0 */
	SENINF1_REG_FIX_ADDR_DBG2                       SENINF1_FIX_ADDR_DBG2;                           /* 0AE4, 0x1A040AE4 */
	SENINF1_REG_CSI2_MODE                           SENINF1_CSI2_MODE;                               /* 0AE8, 0x1A040AE8 */
	UINT32                                          rsv_0AEC;                                        /* 0AEC, 0x1A040AEC */
	SENINF1_REG_CSI2_DI_EXT                         SENINF1_CSI2_DI_EXT;                             /* 0AF0, 0x1A040AF0 */
	SENINF1_REG_CSI2_DI_CTRL_EXT                    SENINF1_CSI2_DI_CTRL_EXT;                        /* 0AF4, 0x1A040AF4 */
	SENINF1_REG_CSI2_CPHY_LOOPBACK                  SENINF1_CSI2_CPHY_LOOPBACK;                      /* 0AF8, 0x1A040AF8 */
	UINT32                                          rsv_0AFC;                                        /* 0AFC, 0x1A040AFC */
	SENINF1_REG_CSI2_PROGSEQ_0                      SENINF1_CSI2_PROGSEQ_0;                          /* 0B00, 0x1A040B00 */
	SENINF1_REG_CSI2_PROGSEQ_1                      SENINF1_CSI2_PROGSEQ_1;                          /* 0B04, 0x1A040B04 */
	UINT32                                          rsv_0B08[2];                                     /* 0B08..0B0F, 0x1A040B08..1A040B0F */
	SENINF1_REG_CSI2_INT_EN_EXT                     SENINF1_CSI2_INT_EN_EXT;                         /* 0B10, 0x1A040B10 */
	SENINF1_REG_CSI2_INT_STATUS_EXT                 SENINF1_CSI2_INT_STATUS_EXT;                     /* 0B14, 0x1A040B14 */
	SENINF1_REG_CSI2_CPHY_FIX_POINT_RST             SENINF1_CSI2_CPHY_FIX_POINT_RST;                 /* 0B18, 0x1A040B18 */
	SENINF1_REG_CSI2_RLR3_CON0                      SENINF1_CSI2_RLR3_CON0;                          /* 0B1C, 0x1A040B1C */
	SENINF1_REG_CSI2_DPHY_SYNC                      SENINF1_CSI2_DPHY_SYNC;                          /* 0B20, 0x1A040B20 */
	SENINF1_REG_CSI2_DESKEW_SYNC                    SENINF1_CSI2_DESKEW_SYNC;                        /* 0B24, 0x1A040B24 */
	SENINF1_REG_CSI2_DETECT_DBG2                    SENINF1_CSI2_DETECT_DBG2;                        /* 0B28, 0x1A040B28 */
	UINT32                                          rsv_0B2C;                                        /* 0B2C, 0x1A040B2C */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG0                 SENINF1_FIX_ADDR_CPHY3_DBG0;                     /* 0B30, 0x1A040B30 */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG1                 SENINF1_FIX_ADDR_CPHY3_DBG1;                     /* 0B34, 0x1A040B34 */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG2                 SENINF1_FIX_ADDR_CPHY3_DBG2;                     /* 0B38, 0x1A040B38 */
	SENINF1_REG_CSI2_DI_EXT_2                       SENINF1_CSI2_DI_EXT_2;                           /* 0B3C, 0x1A040B3C */
	SENINF1_REG_CSI2_DI_CTRL_EXT_2                  SENINF1_CSI2_DI_CTRL_EXT_2;                      /* 0B40, 0x1A040B40 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 0B44, 0x1A040B44 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 0B48, 0x1A040B48 */
}seninf_csi2_ctl_reg_base;

typedef volatile struct _seninf_reg_t_	/* 0x1A040000..0x1A047D8F */
{
	SENINF_REG_TOP_CTRL                             SENINF_TOP_CTRL;                                 /* 0000, 0x1A040000 */
	SENINF_REG_TOP_CMODEL_PAR                       SENINF_TOP_CMODEL_PAR;                           /* 0004, 0x1A040004 */
	SENINF_REG_TOP_MUX_CTRL                         SENINF_TOP_MUX_CTRL;                             /* 0008, 0x1A040008 */
	UINT32                                          rsv_000C;                                        /* 000C, 0x1A04000C */
	SENINF_REG_TOP_CAM_MUX_CTRL                     SENINF_TOP_CAM_MUX_CTRL;                         /* 0010, 0x1A040010 */
	SENINF_REG_TOP_N3D_A_CTL                        SENINF_TOP_N3D_A_CTL;                            /* 0014, 0x1A040014 */
	SENINF_REG_TOP_N3D_B_CTL                        SENINF_TOP_N3D_B_CTL;                            /* 0018, 0x1A040018 */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI0              SENINF_TOP_PHY_SENINF_CTL_CSI0;                  /* 001C, 0x1A04001C */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI1              SENINF_TOP_PHY_SENINF_CTL_CSI1;                  /* 0020, 0x1A040020 */
	SENINF_REG_TOP_PHY_SENINF_CTL_CSI2              SENINF_TOP_PHY_SENINF_CTL_CSI2;                  /* 0024, 0x1A040024 */
	UINT32                                          rsv_0028[54];                                    /* 0028..00FF, 0x1A040028..1A0400FF */
	SENINF_REG_N3D_A_CTL                            SENINF_N3D_A_CTL;                                /* 0100, 0x1A040100 */
	SENINF_REG_N3D_A_POS                            SENINF_N3D_A_POS;                                /* 0104, 0x1A040104 */
	SENINF_REG_N3D_A_TRIG                           SENINF_N3D_A_TRIG;                               /* 0108, 0x1A040108 */
	SENINF_REG_N3D_A_INT                            SENINF_N3D_A_INT;                                /* 010C, 0x1A04010C */
	SENINF_REG_N3D_A_CNT0                           SENINF_N3D_A_CNT0;                               /* 0110, 0x1A040110 */
	SENINF_REG_N3D_A_CNT1                           SENINF_N3D_A_CNT1;                               /* 0114, 0x1A040114 */
	SENINF_REG_N3D_A_DBG                            SENINF_N3D_A_DBG;                                /* 0118, 0x1A040118 */
	SENINF_REG_N3D_A_DIFF_THR                       SENINF_N3D_A_DIFF_THR;                           /* 011C, 0x1A04011C */
	SENINF_REG_N3D_A_DIFF_CNT                       SENINF_N3D_A_DIFF_CNT;                           /* 0120, 0x1A040120 */
	SENINF_REG_N3D_A_DBG_1                          SENINF_N3D_A_DBG_1;                              /* 0124, 0x1A040124 */
	SENINF_REG_N3D_A_VALID_TG_CNT                   SENINF_N3D_A_VALID_TG_CNT;                       /* 0128, 0x1A040128 */
	SENINF_REG_N3D_A_SYNC_A_PERIOD                  SENINF_N3D_A_SYNC_A_PERIOD;                      /* 012C, 0x1A04012C */
	SENINF_REG_N3D_A_SYNC_B_PERIOD                  SENINF_N3D_A_SYNC_B_PERIOD;                      /* 0130, 0x1A040130 */
	SENINF_REG_N3D_A_SYNC_A_PULSE_LEN               SENINF_N3D_A_SYNC_A_PULSE_LEN;                   /* 0134, 0x1A040134 */
	SENINF_REG_N3D_A_SYNC_B_PULSE_LEN               SENINF_N3D_A_SYNC_B_PULSE_LEN;                   /* 0138, 0x1A040138 */
	SENINF_REG_N3D_A_SUB_CNT                        SENINF_N3D_A_SUB_CNT;                            /* 013C, 0x1A04013C */
	SENINF_REG_N3D_A_VSYNC_CNT                      SENINF_N3D_A_VSYNC_CNT;                          /* 0140, 0x1A040140 */
	UINT32                                          rsv_0144[47];                                    /* 0144..01FF, 0x1A040144..1A0401FF */
	SENINF1_REG_CTRL                                SENINF1_CTRL;                                    /* 0200, 0x1A040200 */
	SENINF1_REG_CTRL_EXT                            SENINF1_CTRL_EXT;                                /* 0204, 0x1A040204 */
	SENINF1_REG_ASYNC_CTRL                          SENINF1_ASYNC_CTRL;                              /* 0208, 0x1A040208 */
	UINT32                                          rsv_020C[253];                                   /* 020C..05FF, 0x1A04020C..1A0405FF */
	SENINF_REG_TG1_PH_CNT                           SENINF_TG1_PH_CNT;                               /* 0600, 0x1A040600 */
	SENINF_REG_TG1_SEN_CK                           SENINF_TG1_SEN_CK;                               /* 0604, 0x1A040604 */
	SENINF_REG_TG1_TM_CTL                           SENINF_TG1_TM_CTL;                               /* 0608, 0x1A040608 */
	SENINF_REG_TG1_TM_SIZE                          SENINF_TG1_TM_SIZE;                              /* 060C, 0x1A04060C */
	SENINF_REG_TG1_TM_CLK                           SENINF_TG1_TM_CLK;                               /* 0610, 0x1A040610 */
	SENINF_REG_TG1_TM_STP                           SENINF_TG1_TM_STP;                               /* 0614, 0x1A040614 */
	UINT32                                          rsv_0618[131];                                   /* 0618..0823, 0x1A040618..1A040823 */
	MIPI_REG_RX_CON24_CSI0                          MIPI_RX_CON24_CSI0;                              /* 0824, 0x1A040824 */
	MIPI_REG_RX_CON28_CSI0                          MIPI_RX_CON28_CSI0;                              /* 0828, 0x1A040828 */
	UINT32                                          rsv_082C[2];                                     /* 082C..0833, 0x1A04082C..1A040833 */
	MIPI_REG_RX_CON34_CSI0                          MIPI_RX_CON34_CSI0;                              /* 0834, 0x1A040834 */
	MIPI_REG_RX_CON38_CSI0                          MIPI_RX_CON38_CSI0;                              /* 0838, 0x1A040838 */
	MIPI_REG_RX_CON3C_CSI0                          MIPI_RX_CON3C_CSI0;                              /* 083C, 0x1A04083C */
	UINT32                                          rsv_0840[15];                                    /* 0840..087B, 0x1A040840..1A04087B */
	MIPI_REG_RX_CON7C_CSI0                          MIPI_RX_CON7C_CSI0;                              /* 087C, 0x1A04087C */
	MIPI_REG_RX_CON80_CSI0                          MIPI_RX_CON80_CSI0;                              /* 0880, 0x1A040880 */
	MIPI_REG_RX_CON84_CSI0                          MIPI_RX_CON84_CSI0;                              /* 0884, 0x1A040884 */
	MIPI_REG_RX_CON88_CSI0                          MIPI_RX_CON88_CSI0;                              /* 0888, 0x1A040888 */
	MIPI_REG_RX_CON8C_CSI0                          MIPI_RX_CON8C_CSI0;                              /* 088C, 0x1A04088C */
	MIPI_REG_RX_CON90_CSI0                          MIPI_RX_CON90_CSI0;                              /* 0890, 0x1A040890 */
	MIPI_REG_RX_CON94_CSI0                          MIPI_RX_CON94_CSI0;                              /* 0894, 0x1A040894 */
	MIPI_REG_RX_CON98_CSI0                          MIPI_RX_CON98_CSI0;                              /* 0898, 0x1A040898 */
	UINT32                                          rsv_089C;                                        /* 089C, 0x1A04089C */
	MIPI_REG_RX_CONA0_CSI0                          MIPI_RX_CONA0_CSI0;                              /* 08A0, 0x1A0408A0 */
	UINT32                                          rsv_08A4[3];                                     /* 08A4..08AF, 0x1A0408A4..1A0408AF */
	MIPI_REG_RX_CONB0_CSI0                          MIPI_RX_CONB0_CSI0;                              /* 08B0, 0x1A0408B0 */
	MIPI_REG_RX_CONB4_CSI0                          MIPI_RX_CONB4_CSI0;                              /* 08B4, 0x1A0408B4 */
	MIPI_REG_RX_CONB8_CSI0                          MIPI_RX_CONB8_CSI0;                              /* 08B8, 0x1A0408B8 */
	MIPI_REG_RX_CONBC_CSI0                          MIPI_RX_CONBC_CSI0;                              /* 08BC, 0x1A0408BC */
	MIPI_REG_RX_CONC0_CSI0                          MIPI_RX_CONC0_CSI0;                              /* 08C0, 0x1A0408C0 */
	MIPI_REG_RX_CONC4_CSI0                          MIPI_RX_CONC4_CSI0;                              /* 08C4, 0x1A0408C4 */
	MIPI_REG_RX_CONC8_CSI0                          MIPI_RX_CONC8_CSI0;                              /* 08C8, 0x1A0408C8 */
	MIPI_REG_RX_CONCC_CSI0                          MIPI_RX_CONCC_CSI0;                              /* 08CC, 0x1A0408CC */
	MIPI_REG_RX_COND0_CSI0                          MIPI_RX_COND0_CSI0;                              /* 08D0, 0x1A0408D0 */
	UINT32                                          rsv_08D4[75];                                    /* 08D4..09FF, 0x1A0408D4..1A0409FF */
	SENINF1_REG_CSI2_CTL                            SENINF1_CSI2_CTL;                                /* 0A00, 0x1A040A00 */
	SENINF1_REG_CSI2_LNRC_TIMING                    SENINF1_CSI2_LNRC_TIMING;                        /* 0A04, 0x1A040A04 */
	SENINF1_REG_CSI2_LNRD_TIMING                    SENINF1_CSI2_LNRD_TIMING;                        /* 0A08, 0x1A040A08 */
	SENINF1_REG_CSI2_DPCM                           SENINF1_CSI2_DPCM;                               /* 0A0C, 0x1A040A0C */
	SENINF1_REG_CSI2_INT_EN                         SENINF1_CSI2_INT_EN;                             /* 0A10, 0x1A040A10 */
	SENINF1_REG_CSI2_INT_STATUS                     SENINF1_CSI2_INT_STATUS;                         /* 0A14, 0x1A040A14 */
	SENINF1_REG_CSI2_DGB_SEL                        SENINF1_CSI2_DGB_SEL;                            /* 0A18, 0x1A040A18 */
	SENINF1_REG_CSI2_DBG_PORT                       SENINF1_CSI2_DBG_PORT;                           /* 0A1C, 0x1A040A1C */
	SENINF1_REG_CSI2_SPARE0                         SENINF1_CSI2_SPARE0;                             /* 0A20, 0x1A040A20 */
	SENINF1_REG_CSI2_SPARE1                         SENINF1_CSI2_SPARE1;                             /* 0A24, 0x1A040A24 */
	SENINF1_REG_CSI2_LNRC_FSM                       SENINF1_CSI2_LNRC_FSM;                           /* 0A28, 0x1A040A28 */
	SENINF1_REG_CSI2_LNRD_FSM                       SENINF1_CSI2_LNRD_FSM;                           /* 0A2C, 0x1A040A2C */
	SENINF1_REG_CSI2_FRAME_LINE_NUM                 SENINF1_CSI2_FRAME_LINE_NUM;                     /* 0A30, 0x1A040A30 */
	SENINF1_REG_CSI2_GENERIC_SHORT                  SENINF1_CSI2_GENERIC_SHORT;                      /* 0A34, 0x1A040A34 */
	SENINF1_REG_CSI2_HSRX_DBG                       SENINF1_CSI2_HSRX_DBG;                           /* 0A38, 0x1A040A38 */
	SENINF1_REG_CSI2_DI                             SENINF1_CSI2_DI;                                 /* 0A3C, 0x1A040A3C */
	SENINF1_REG_CSI2_HS_TRAIL                       SENINF1_CSI2_HS_TRAIL;                           /* 0A40, 0x1A040A40 */
	SENINF1_REG_CSI2_DI_CTRL                        SENINF1_CSI2_DI_CTRL;                            /* 0A44, 0x1A040A44 */
	UINT32                                          rsv_0A48;                                        /* 0A48, 0x1A040A48 */
	SENINF1_REG_CSI2_DETECT_CON1                    SENINF1_CSI2_DETECT_CON1;                        /* 0A4C, 0x1A040A4C */
	SENINF1_REG_CSI2_DETECT_CON2                    SENINF1_CSI2_DETECT_CON2;                        /* 0A50, 0x1A040A50 */
	SENINF1_REG_CSI2_DETECT_CON3                    SENINF1_CSI2_DETECT_CON3;                        /* 0A54, 0x1A040A54 */
	SENINF1_REG_CSI2_RLR0_CON0                      SENINF1_CSI2_RLR0_CON0;                          /* 0A58, 0x1A040A58 */
	SENINF1_REG_CSI2_RLR1_CON0                      SENINF1_CSI2_RLR1_CON0;                          /* 0A5C, 0x1A040A5C */
	SENINF1_REG_CSI2_RLR2_CON0                      SENINF1_CSI2_RLR2_CON0;                          /* 0A60, 0x1A040A60 */
	SENINF1_REG_CSI2_RLR_CON0                       SENINF1_CSI2_RLR_CON0;                           /* 0A64, 0x1A040A64 */
	SENINF1_REG_CSI2_MUX_CON                        SENINF1_CSI2_MUX_CON;                            /* 0A68, 0x1A040A68 */
	SENINF1_REG_CSI2_DETECT_DBG0                    SENINF1_CSI2_DETECT_DBG0;                        /* 0A6C, 0x1A040A6C */
	SENINF1_REG_CSI2_DETECT_DBG1                    SENINF1_CSI2_DETECT_DBG1;                        /* 0A70, 0x1A040A70 */
	SENINF1_REG_CSI2_RESYNC_MERGE_CTL               SENINF1_CSI2_RESYNC_MERGE_CTL;                   /* 0A74, 0x1A040A74 */
	SENINF1_REG_CSI2_CTRL_TRIO_MUX                  SENINF1_CSI2_CTRL_TRIO_MUX;                      /* 0A78, 0x1A040A78 */
	SENINF1_REG_CSI2_CTRL_TRIO_CON                  SENINF1_CSI2_CTRL_TRIO_CON;                      /* 0A7C, 0x1A040A7C */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG                  SENINF1_FIX_ADDR_CPHY0_DBG;                      /* 0A80, 0x1A040A80 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG                  SENINF1_FIX_ADDR_CPHY1_DBG;                      /* 0A84, 0x1A040A84 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG                  SENINF1_FIX_ADDR_CPHY2_DBG;                      /* 0A88, 0x1A040A88 */
	SENINF1_REG_FIX_ADDR_DBG                        SENINF1_FIX_ADDR_DBG;                            /* 0A8C, 0x1A040A8C */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 0A90, 0x1A040A90 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 0A94, 0x1A040A94 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 0A98, 0x1A040A98 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 0A9C, 0x1A040A9C */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 0AA0, 0x1A040AA0 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 0AA4, 0x1A040AA4 */
	SENINF1_REG_SYNC_RESYNC_CTL                     SENINF1_SYNC_RESYNC_CTL;                         /* 0AA8, 0x1A040AA8 */
	SENINF1_REG_POST_DETECT_CTL                     SENINF1_POST_DETECT_CTL;                         /* 0AAC, 0x1A040AAC */
	SENINF1_REG_WIRE_STATE_DECODE_CONFIG            SENINF1_WIRE_STATE_DECODE_CONFIG;                /* 0AB0, 0x1A040AB0 */
	SENINF1_REG_CSI2_CPHY_LNRD_FSM                  SENINF1_CSI2_CPHY_LNRD_FSM;                      /* 0AB4, 0x1A040AB4 */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG0                 SENINF1_FIX_ADDR_CPHY0_DBG0;                     /* 0AB8, 0x1A040AB8 */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG1                 SENINF1_FIX_ADDR_CPHY0_DBG1;                     /* 0ABC, 0x1A040ABC */
	SENINF1_REG_FIX_ADDR_CPHY0_DBG2                 SENINF1_FIX_ADDR_CPHY0_DBG2;                     /* 0AC0, 0x1A040AC0 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG0                 SENINF1_FIX_ADDR_CPHY1_DBG0;                     /* 0AC4, 0x1A040AC4 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG1                 SENINF1_FIX_ADDR_CPHY1_DBG1;                     /* 0AC8, 0x1A040AC8 */
	SENINF1_REG_FIX_ADDR_CPHY1_DBG2                 SENINF1_FIX_ADDR_CPHY1_DBG2;                     /* 0ACC, 0x1A040ACC */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG0                 SENINF1_FIX_ADDR_CPHY2_DBG0;                     /* 0AD0, 0x1A040AD0 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG1                 SENINF1_FIX_ADDR_CPHY2_DBG1;                     /* 0AD4, 0x1A040AD4 */
	SENINF1_REG_FIX_ADDR_CPHY2_DBG2                 SENINF1_FIX_ADDR_CPHY2_DBG2;                     /* 0AD8, 0x1A040AD8 */
	SENINF1_REG_FIX_ADDR_DBG0                       SENINF1_FIX_ADDR_DBG0;                           /* 0ADC, 0x1A040ADC */
	SENINF1_REG_FIX_ADDR_DBG1                       SENINF1_FIX_ADDR_DBG1;                           /* 0AE0, 0x1A040AE0 */
	SENINF1_REG_FIX_ADDR_DBG2                       SENINF1_FIX_ADDR_DBG2;                           /* 0AE4, 0x1A040AE4 */
	SENINF1_REG_CSI2_MODE                           SENINF1_CSI2_MODE;                               /* 0AE8, 0x1A040AE8 */
	UINT32                                          rsv_0AEC;                                        /* 0AEC, 0x1A040AEC */
	SENINF1_REG_CSI2_DI_EXT                         SENINF1_CSI2_DI_EXT;                             /* 0AF0, 0x1A040AF0 */
	SENINF1_REG_CSI2_DI_CTRL_EXT                    SENINF1_CSI2_DI_CTRL_EXT;                        /* 0AF4, 0x1A040AF4 */
	SENINF1_REG_CSI2_CPHY_LOOPBACK                  SENINF1_CSI2_CPHY_LOOPBACK;                      /* 0AF8, 0x1A040AF8 */
	UINT32                                          rsv_0AFC;                                        /* 0AFC, 0x1A040AFC */
	SENINF1_REG_CSI2_PROGSEQ_0                      SENINF1_CSI2_PROGSEQ_0;                          /* 0B00, 0x1A040B00 */
	SENINF1_REG_CSI2_PROGSEQ_1                      SENINF1_CSI2_PROGSEQ_1;                          /* 0B04, 0x1A040B04 */
	UINT32                                          rsv_0B08[2];                                     /* 0B08..0B0F, 0x1A040B08..1A040B0F */
	SENINF1_REG_CSI2_INT_EN_EXT                     SENINF1_CSI2_INT_EN_EXT;                         /* 0B10, 0x1A040B10 */
	SENINF1_REG_CSI2_INT_STATUS_EXT                 SENINF1_CSI2_INT_STATUS_EXT;                     /* 0B14, 0x1A040B14 */
	SENINF1_REG_CSI2_CPHY_FIX_POINT_RST             SENINF1_CSI2_CPHY_FIX_POINT_RST;                 /* 0B18, 0x1A040B18 */
	SENINF1_REG_CSI2_RLR3_CON0                      SENINF1_CSI2_RLR3_CON0;                          /* 0B1C, 0x1A040B1C */
	SENINF1_REG_CSI2_DPHY_SYNC                      SENINF1_CSI2_DPHY_SYNC;                          /* 0B20, 0x1A040B20 */
	SENINF1_REG_CSI2_DESKEW_SYNC                    SENINF1_CSI2_DESKEW_SYNC;                        /* 0B24, 0x1A040B24 */
	SENINF1_REG_CSI2_DETECT_DBG2                    SENINF1_CSI2_DETECT_DBG2;                        /* 0B28, 0x1A040B28 */
	UINT32                                          rsv_0B2C;                                        /* 0B2C, 0x1A040B2C */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG0                 SENINF1_FIX_ADDR_CPHY3_DBG0;                     /* 0B30, 0x1A040B30 */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG1                 SENINF1_FIX_ADDR_CPHY3_DBG1;                     /* 0B34, 0x1A040B34 */
	SENINF1_REG_FIX_ADDR_CPHY3_DBG2                 SENINF1_FIX_ADDR_CPHY3_DBG2;                     /* 0B38, 0x1A040B38 */
	SENINF1_REG_CSI2_DI_EXT_2                       SENINF1_CSI2_DI_EXT_2;                           /* 0B3C, 0x1A040B3C */
	SENINF1_REG_CSI2_DI_CTRL_EXT_2                  SENINF1_CSI2_DI_CTRL_EXT_2;                      /* 0B40, 0x1A040B40 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF1_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 0B44, 0x1A040B44 */
	SENINF1_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF1_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 0B48, 0x1A040B48 */
	UINT32                                          rsv_0B4C[109];                                   /* 0B4C..0CFF, 0x1A040B4C..1A040CFF */
	SENINF1_REG_MUX_CTRL                            SENINF1_MUX_CTRL;                                /* 0D00, 0x1A040D00 */
	SENINF1_REG_MUX_INTEN                           SENINF1_MUX_INTEN;                               /* 0D04, 0x1A040D04 */
	SENINF1_REG_MUX_INTSTA                          SENINF1_MUX_INTSTA;                              /* 0D08, 0x1A040D08 */
	SENINF1_REG_MUX_SIZE                            SENINF1_MUX_SIZE;                                /* 0D0C, 0x1A040D0C */
	SENINF1_REG_MUX_DEBUG_1                         SENINF1_MUX_DEBUG_1;                             /* 0D10, 0x1A040D10 */
	SENINF1_REG_MUX_DEBUG_2                         SENINF1_MUX_DEBUG_2;                             /* 0D14, 0x1A040D14 */
	SENINF1_REG_MUX_DEBUG_3                         SENINF1_MUX_DEBUG_3;                             /* 0D18, 0x1A040D18 */
	SENINF1_REG_MUX_DEBUG_4                         SENINF1_MUX_DEBUG_4;                             /* 0D1C, 0x1A040D1C */
	SENINF1_REG_MUX_DEBUG_5                         SENINF1_MUX_DEBUG_5;                             /* 0D20, 0x1A040D20 */
	SENINF1_REG_MUX_DEBUG_6                         SENINF1_MUX_DEBUG_6;                             /* 0D24, 0x1A040D24 */
	SENINF1_REG_MUX_DEBUG_7                         SENINF1_MUX_DEBUG_7;                             /* 0D28, 0x1A040D28 */
	SENINF1_REG_MUX_SPARE                           SENINF1_MUX_SPARE;                               /* 0D2C, 0x1A040D2C */
	SENINF1_REG_MUX_DATA                            SENINF1_MUX_DATA;                                /* 0D30, 0x1A040D30 */
	SENINF1_REG_MUX_DATA_CNT                        SENINF1_MUX_DATA_CNT;                            /* 0D34, 0x1A040D34 */
	SENINF1_REG_MUX_CROP                            SENINF1_MUX_CROP;                                /* 0D38, 0x1A040D38 */
	SENINF1_REG_MUX_CTRL_EXT                        SENINF1_MUX_CTRL_EXT;                            /* 0D3C, 0x1A040D3C */
	UINT32                                          rsv_0D40[240];                                   /* 0D40..10FF, 0x1A040D40..1A0410FF */
	SENINF_REG_N3D_B_CTL                            SENINF_N3D_B_CTL;                                /* 1100, 0x1A041100 */
	SENINF_REG_N3D_B_POS                            SENINF_N3D_B_POS;                                /* 1104, 0x1A041104 */
	SENINF_REG_N3D_B_TRIG                           SENINF_N3D_B_TRIG;                               /* 1108, 0x1A041108 */
	SENINF_REG_N3D_B_INT                            SENINF_N3D_B_INT;                                /* 110C, 0x1A04110C */
	SENINF_REG_N3D_B_CNT0                           SENINF_N3D_B_CNT0;                               /* 1110, 0x1A041110 */
	SENINF_REG_N3D_B_CNT1                           SENINF_N3D_B_CNT1;                               /* 1114, 0x1A041114 */
	SENINF_REG_N3D_B_DBG                            SENINF_N3D_B_DBG;                                /* 1118, 0x1A041118 */
	SENINF_REG_N3D_B_DIFF_THR                       SENINF_N3D_B_DIFF_THR;                           /* 111C, 0x1A04111C */
	SENINF_REG_N3D_B_DIFF_CNT                       SENINF_N3D_B_DIFF_CNT;                           /* 1120, 0x1A041120 */
	SENINF_REG_N3D_B_DBG_1                          SENINF_N3D_B_DBG_1;                              /* 1124, 0x1A041124 */
	SENINF_REG_N3D_B_VALID_TG_CNT                   SENINF_N3D_B_VALID_TG_CNT;                       /* 1128, 0x1A041128 */
	SENINF_REG_N3D_B_SYNC_A_PERIOD                  SENINF_N3D_B_SYNC_A_PERIOD;                      /* 112C, 0x1A04112C */
	SENINF_REG_N3D_B_SYNC_B_PERIOD                  SENINF_N3D_B_SYNC_B_PERIOD;                      /* 1130, 0x1A041130 */
	SENINF_REG_N3D_B_SYNC_A_PULSE_LEN               SENINF_N3D_B_SYNC_A_PULSE_LEN;                   /* 1134, 0x1A041134 */
	SENINF_REG_N3D_B_SYNC_B_PULSE_LEN               SENINF_N3D_B_SYNC_B_PULSE_LEN;                   /* 1138, 0x1A041138 */
	SENINF_REG_N3D_B_SUB_CNT                        SENINF_N3D_B_SUB_CNT;                            /* 113C, 0x1A04113C */
	SENINF_REG_N3D_B_VSYNC_CNT                      SENINF_N3D_B_VSYNC_CNT;                          /* 1140, 0x1A041140 */
	UINT32                                          rsv_1144[47];                                    /* 1144..11FF, 0x1A041144..1A0411FF */
	SENINF2_REG_CTRL                                SENINF2_CTRL;                                    /* 1200, 0x1A041200 */
	SENINF2_REG_CTRL_EXT                            SENINF2_CTRL_EXT;                                /* 1204, 0x1A041204 */
	SENINF2_REG_ASYNC_CTRL                          SENINF2_ASYNC_CTRL;                              /* 1208, 0x1A041208 */
	UINT32                                          rsv_120C[253];                                   /* 120C..15FF, 0x1A04120C..1A0415FF */
	SENINF_REG_TG2_PH_CNT                           SENINF_TG2_PH_CNT;                               /* 1600, 0x1A041600 */
	SENINF_REG_TG2_SEN_CK                           SENINF_TG2_SEN_CK;                               /* 1604, 0x1A041604 */
	SENINF_REG_TG2_TM_CTL                           SENINF_TG2_TM_CTL;                               /* 1608, 0x1A041608 */
	SENINF_REG_TG2_TM_SIZE                          SENINF_TG2_TM_SIZE;                              /* 160C, 0x1A04160C */
	SENINF_REG_TG2_TM_CLK                           SENINF_TG2_TM_CLK;                               /* 1610, 0x1A041610 */
	SENINF_REG_TG2_TM_STP                           SENINF_TG2_TM_STP;                               /* 1614, 0x1A041614 */
	UINT32                                          rsv_1618[131];                                   /* 1618..1823, 0x1A041618..1A041823 */
	MIPI_REG_RX_CON24_CSI1                          MIPI_RX_CON24_CSI1;                              /* 1824, 0x1A041824 */
	MIPI_REG_RX_CON28_CSI1                          MIPI_RX_CON28_CSI1;                              /* 1828, 0x1A041828 */
	UINT32                                          rsv_182C[2];                                     /* 182C..1833, 0x1A04182C..1A041833 */
	MIPI_REG_RX_CON34_CSI1                          MIPI_RX_CON34_CSI1;                              /* 1834, 0x1A041834 */
	MIPI_REG_RX_CON38_CSI1                          MIPI_RX_CON38_CSI1;                              /* 1838, 0x1A041838 */
	MIPI_REG_RX_CON3C_CSI1                          MIPI_RX_CON3C_CSI1;                              /* 183C, 0x1A04183C */
	UINT32                                          rsv_1840[15];                                    /* 1840..187B, 0x1A041840..1A04187B */
	MIPI_REG_RX_CON7C_CSI1                          MIPI_RX_CON7C_CSI1;                              /* 187C, 0x1A04187C */
	MIPI_REG_RX_CON80_CSI1                          MIPI_RX_CON80_CSI1;                              /* 1880, 0x1A041880 */
	MIPI_REG_RX_CON84_CSI1                          MIPI_RX_CON84_CSI1;                              /* 1884, 0x1A041884 */
	MIPI_REG_RX_CON88_CSI1                          MIPI_RX_CON88_CSI1;                              /* 1888, 0x1A041888 */
	MIPI_REG_RX_CON8C_CSI1                          MIPI_RX_CON8C_CSI1;                              /* 188C, 0x1A04188C */
	MIPI_REG_RX_CON90_CSI1                          MIPI_RX_CON90_CSI1;                              /* 1890, 0x1A041890 */
	MIPI_REG_RX_CON94_CSI1                          MIPI_RX_CON94_CSI1;                              /* 1894, 0x1A041894 */
	MIPI_REG_RX_CON98_CSI1                          MIPI_RX_CON98_CSI1;                              /* 1898, 0x1A041898 */
	UINT32                                          rsv_189C;                                        /* 189C, 0x1A04189C */
	MIPI_REG_RX_CONA0_CSI1                          MIPI_RX_CONA0_CSI1;                              /* 18A0, 0x1A0418A0 */
	UINT32                                          rsv_18A4[3];                                     /* 18A4..18AF, 0x1A0418A4..1A0418AF */
	MIPI_REG_RX_CONB0_CSI1                          MIPI_RX_CONB0_CSI1;                              /* 18B0, 0x1A0418B0 */
	MIPI_REG_RX_CONB4_CSI1                          MIPI_RX_CONB4_CSI1;                              /* 18B4, 0x1A0418B4 */
	MIPI_REG_RX_CONB8_CSI1                          MIPI_RX_CONB8_CSI1;                              /* 18B8, 0x1A0418B8 */
	MIPI_REG_RX_CONBC_CSI1                          MIPI_RX_CONBC_CSI1;                              /* 18BC, 0x1A0418BC */
	MIPI_REG_RX_CONC0_CSI1                          MIPI_RX_CONC0_CSI1;                              /* 18C0, 0x1A0418C0 */
	MIPI_REG_RX_CONC4_CSI1                          MIPI_RX_CONC4_CSI1;                              /* 18C4, 0x1A0418C4 */
	MIPI_REG_RX_CONC8_CSI1                          MIPI_RX_CONC8_CSI1;                              /* 18C8, 0x1A0418C8 */
	MIPI_REG_RX_CONCC_CSI1                          MIPI_RX_CONCC_CSI1;                              /* 18CC, 0x1A0418CC */
	MIPI_REG_RX_COND0_CSI1                          MIPI_RX_COND0_CSI1;                              /* 18D0, 0x1A0418D0 */
	UINT32                                          rsv_18D4[75];                                    /* 18D4..19FF, 0x1A0418D4..1A0419FF */
	SENINF2_REG_CSI2_CTL                            SENINF2_CSI2_CTL;                                /* 1A00, 0x1A041A00 */
	SENINF2_REG_CSI2_LNRC_TIMING                    SENINF2_CSI2_LNRC_TIMING;                        /* 1A04, 0x1A041A04 */
	SENINF2_REG_CSI2_LNRD_TIMING                    SENINF2_CSI2_LNRD_TIMING;                        /* 1A08, 0x1A041A08 */
	SENINF2_REG_CSI2_DPCM                           SENINF2_CSI2_DPCM;                               /* 1A0C, 0x1A041A0C */
	SENINF2_REG_CSI2_INT_EN                         SENINF2_CSI2_INT_EN;                             /* 1A10, 0x1A041A10 */
	SENINF2_REG_CSI2_INT_STATUS                     SENINF2_CSI2_INT_STATUS;                         /* 1A14, 0x1A041A14 */
	SENINF2_REG_CSI2_DGB_SEL                        SENINF2_CSI2_DGB_SEL;                            /* 1A18, 0x1A041A18 */
	SENINF2_REG_CSI2_DBG_PORT                       SENINF2_CSI2_DBG_PORT;                           /* 1A1C, 0x1A041A1C */
	SENINF2_REG_CSI2_SPARE0                         SENINF2_CSI2_SPARE0;                             /* 1A20, 0x1A041A20 */
	SENINF2_REG_CSI2_SPARE1                         SENINF2_CSI2_SPARE1;                             /* 1A24, 0x1A041A24 */
	SENINF2_REG_CSI2_LNRC_FSM                       SENINF2_CSI2_LNRC_FSM;                           /* 1A28, 0x1A041A28 */
	SENINF2_REG_CSI2_LNRD_FSM                       SENINF2_CSI2_LNRD_FSM;                           /* 1A2C, 0x1A041A2C */
	SENINF2_REG_CSI2_FRAME_LINE_NUM                 SENINF2_CSI2_FRAME_LINE_NUM;                     /* 1A30, 0x1A041A30 */
	SENINF2_REG_CSI2_GENERIC_SHORT                  SENINF2_CSI2_GENERIC_SHORT;                      /* 1A34, 0x1A041A34 */
	SENINF2_REG_CSI2_HSRX_DBG                       SENINF2_CSI2_HSRX_DBG;                           /* 1A38, 0x1A041A38 */
	SENINF2_REG_CSI2_DI                             SENINF2_CSI2_DI;                                 /* 1A3C, 0x1A041A3C */
	SENINF2_REG_CSI2_HS_TRAIL                       SENINF2_CSI2_HS_TRAIL;                           /* 1A40, 0x1A041A40 */
	SENINF2_REG_CSI2_DI_CTRL                        SENINF2_CSI2_DI_CTRL;                            /* 1A44, 0x1A041A44 */
	UINT32                                          rsv_1A48;                                        /* 1A48, 0x1A041A48 */
	SENINF2_REG_CSI2_DETECT_CON1                    SENINF2_CSI2_DETECT_CON1;                        /* 1A4C, 0x1A041A4C */
	SENINF2_REG_CSI2_DETECT_CON2                    SENINF2_CSI2_DETECT_CON2;                        /* 1A50, 0x1A041A50 */
	SENINF2_REG_CSI2_DETECT_CON3                    SENINF2_CSI2_DETECT_CON3;                        /* 1A54, 0x1A041A54 */
	SENINF2_REG_CSI2_RLR0_CON0                      SENINF2_CSI2_RLR0_CON0;                          /* 1A58, 0x1A041A58 */
	SENINF2_REG_CSI2_RLR1_CON0                      SENINF2_CSI2_RLR1_CON0;                          /* 1A5C, 0x1A041A5C */
	SENINF2_REG_CSI2_RLR2_CON0                      SENINF2_CSI2_RLR2_CON0;                          /* 1A60, 0x1A041A60 */
	SENINF2_REG_CSI2_RLR_CON0                       SENINF2_CSI2_RLR_CON0;                           /* 1A64, 0x1A041A64 */
	SENINF2_REG_CSI2_MUX_CON                        SENINF2_CSI2_MUX_CON;                            /* 1A68, 0x1A041A68 */
	SENINF2_REG_CSI2_DETECT_DBG0                    SENINF2_CSI2_DETECT_DBG0;                        /* 1A6C, 0x1A041A6C */
	SENINF2_REG_CSI2_DETECT_DBG1                    SENINF2_CSI2_DETECT_DBG1;                        /* 1A70, 0x1A041A70 */
	SENINF2_REG_CSI2_RESYNC_MERGE_CTL               SENINF2_CSI2_RESYNC_MERGE_CTL;                   /* 1A74, 0x1A041A74 */
	SENINF2_REG_CSI2_CTRL_TRIO_MUX                  SENINF2_CSI2_CTRL_TRIO_MUX;                      /* 1A78, 0x1A041A78 */
	SENINF2_REG_CSI2_CTRL_TRIO_CON                  SENINF2_CSI2_CTRL_TRIO_CON;                      /* 1A7C, 0x1A041A7C */
	SENINF2_REG_FIX_ADDR_CPHY0_DBG                  SENINF2_FIX_ADDR_CPHY0_DBG;                      /* 1A80, 0x1A041A80 */
	SENINF2_REG_FIX_ADDR_CPHY1_DBG                  SENINF2_FIX_ADDR_CPHY1_DBG;                      /* 1A84, 0x1A041A84 */
	SENINF2_REG_FIX_ADDR_CPHY2_DBG                  SENINF2_FIX_ADDR_CPHY2_DBG;                      /* 1A88, 0x1A041A88 */
	SENINF2_REG_FIX_ADDR_DBG                        SENINF2_FIX_ADDR_DBG;                            /* 1A8C, 0x1A041A8C */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF2_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 1A90, 0x1A041A90 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF2_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 1A94, 0x1A041A94 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF2_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 1A98, 0x1A041A98 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF2_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 1A9C, 0x1A041A9C */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF2_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 1AA0, 0x1A041AA0 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF2_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 1AA4, 0x1A041AA4 */
	SENINF2_REG_SYNC_RESYNC_CTL                     SENINF2_SYNC_RESYNC_CTL;                         /* 1AA8, 0x1A041AA8 */
	SENINF2_REG_POST_DETECT_CTL                     SENINF2_POST_DETECT_CTL;                         /* 1AAC, 0x1A041AAC */
	SENINF2_REG_WIRE_STATE_DECODE_CONFIG            SENINF2_WIRE_STATE_DECODE_CONFIG;                /* 1AB0, 0x1A041AB0 */
	SENINF2_REG_CSI2_CPHY_LNRD_FSM                  SENINF2_CSI2_CPHY_LNRD_FSM;                      /* 1AB4, 0x1A041AB4 */
	SENINF2_REG_FIX_ADDR_CPHY0_DBG0                 SENINF2_FIX_ADDR_CPHY0_DBG0;                     /* 1AB8, 0x1A041AB8 */
	SENINF2_REG_FIX_ADDR_CPHY0_DBG1                 SENINF2_FIX_ADDR_CPHY0_DBG1;                     /* 1ABC, 0x1A041ABC */
	SENINF2_REG_FIX_ADDR_CPHY0_DBG2                 SENINF2_FIX_ADDR_CPHY0_DBG2;                     /* 1AC0, 0x1A041AC0 */
	SENINF2_REG_FIX_ADDR_CPHY1_DBG0                 SENINF2_FIX_ADDR_CPHY1_DBG0;                     /* 1AC4, 0x1A041AC4 */
	SENINF2_REG_FIX_ADDR_CPHY1_DBG1                 SENINF2_FIX_ADDR_CPHY1_DBG1;                     /* 1AC8, 0x1A041AC8 */
	SENINF2_REG_FIX_ADDR_CPHY1_DBG2                 SENINF2_FIX_ADDR_CPHY1_DBG2;                     /* 1ACC, 0x1A041ACC */
	SENINF2_REG_FIX_ADDR_CPHY2_DBG0                 SENINF2_FIX_ADDR_CPHY2_DBG0;                     /* 1AD0, 0x1A041AD0 */
	SENINF2_REG_FIX_ADDR_CPHY2_DBG1                 SENINF2_FIX_ADDR_CPHY2_DBG1;                     /* 1AD4, 0x1A041AD4 */
	SENINF2_REG_FIX_ADDR_CPHY2_DBG2                 SENINF2_FIX_ADDR_CPHY2_DBG2;                     /* 1AD8, 0x1A041AD8 */
	SENINF2_REG_FIX_ADDR_DBG0                       SENINF2_FIX_ADDR_DBG0;                           /* 1ADC, 0x1A041ADC */
	SENINF2_REG_FIX_ADDR_DBG1                       SENINF2_FIX_ADDR_DBG1;                           /* 1AE0, 0x1A041AE0 */
	SENINF2_REG_FIX_ADDR_DBG2                       SENINF2_FIX_ADDR_DBG2;                           /* 1AE4, 0x1A041AE4 */
	SENINF2_REG_CSI2_MODE                           SENINF2_CSI2_MODE;                               /* 1AE8, 0x1A041AE8 */
	UINT32                                          rsv_1AEC;                                        /* 1AEC, 0x1A041AEC */
	SENINF2_REG_CSI2_DI_EXT                         SENINF2_CSI2_DI_EXT;                             /* 1AF0, 0x1A041AF0 */
	SENINF2_REG_CSI2_DI_CTRL_EXT                    SENINF2_CSI2_DI_CTRL_EXT;                        /* 1AF4, 0x1A041AF4 */
	SENINF2_REG_CSI2_CPHY_LOOPBACK                  SENINF2_CSI2_CPHY_LOOPBACK;                      /* 1AF8, 0x1A041AF8 */
	UINT32                                          rsv_1AFC;                                        /* 1AFC, 0x1A041AFC */
	SENINF2_REG_CSI2_PROGSEQ_0                      SENINF2_CSI2_PROGSEQ_0;                          /* 1B00, 0x1A041B00 */
	SENINF2_REG_CSI2_PROGSEQ_1                      SENINF2_CSI2_PROGSEQ_1;                          /* 1B04, 0x1A041B04 */
	UINT32                                          rsv_1B08[2];                                     /* 1B08..1B0F, 0x1A041B08..1A041B0F */
	SENINF2_REG_CSI2_INT_EN_EXT                     SENINF2_CSI2_INT_EN_EXT;                         /* 1B10, 0x1A041B10 */
	SENINF2_REG_CSI2_INT_STATUS_EXT                 SENINF2_CSI2_INT_STATUS_EXT;                     /* 1B14, 0x1A041B14 */
	SENINF2_REG_CSI2_CPHY_FIX_POINT_RST             SENINF2_CSI2_CPHY_FIX_POINT_RST;                 /* 1B18, 0x1A041B18 */
	SENINF2_REG_CSI2_RLR3_CON0                      SENINF2_CSI2_RLR3_CON0;                          /* 1B1C, 0x1A041B1C */
	SENINF2_REG_CSI2_DPHY_SYNC                      SENINF2_CSI2_DPHY_SYNC;                          /* 1B20, 0x1A041B20 */
	SENINF2_REG_CSI2_DESKEW_SYNC                    SENINF2_CSI2_DESKEW_SYNC;                        /* 1B24, 0x1A041B24 */
	SENINF2_REG_CSI2_DETECT_DBG2                    SENINF2_CSI2_DETECT_DBG2;                        /* 1B28, 0x1A041B28 */
	UINT32                                          rsv_1B2C;                                        /* 1B2C, 0x1A041B2C */
	SENINF2_REG_FIX_ADDR_CPHY3_DBG0                 SENINF2_FIX_ADDR_CPHY3_DBG0;                     /* 1B30, 0x1A041B30 */
	SENINF2_REG_FIX_ADDR_CPHY3_DBG1                 SENINF2_FIX_ADDR_CPHY3_DBG1;                     /* 1B34, 0x1A041B34 */
	SENINF2_REG_FIX_ADDR_CPHY3_DBG2                 SENINF2_FIX_ADDR_CPHY3_DBG2;                     /* 1B38, 0x1A041B38 */
	SENINF2_REG_CSI2_DI_EXT_2                       SENINF2_CSI2_DI_EXT_2;                           /* 1B3C, 0x1A041B3C */
	SENINF2_REG_CSI2_DI_CTRL_EXT_2                  SENINF2_CSI2_DI_CTRL_EXT_2;                      /* 1B40, 0x1A041B40 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF2_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 1B44, 0x1A041B44 */
	SENINF2_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF2_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 1B48, 0x1A041B48 */
	UINT32                                          rsv_1B4C[109];                                   /* 1B4C..1CFF, 0x1A041B4C..1A041CFF */
	SENINF2_REG_MUX_CTRL                            SENINF2_MUX_CTRL;                                /* 1D00, 0x1A041D00 */
	SENINF2_REG_MUX_INTEN                           SENINF2_MUX_INTEN;                               /* 1D04, 0x1A041D04 */
	SENINF2_REG_MUX_INTSTA                          SENINF2_MUX_INTSTA;                              /* 1D08, 0x1A041D08 */
	SENINF2_REG_MUX_SIZE                            SENINF2_MUX_SIZE;                                /* 1D0C, 0x1A041D0C */
	SENINF2_REG_MUX_DEBUG_1                         SENINF2_MUX_DEBUG_1;                             /* 1D10, 0x1A041D10 */
	SENINF2_REG_MUX_DEBUG_2                         SENINF2_MUX_DEBUG_2;                             /* 1D14, 0x1A041D14 */
	SENINF2_REG_MUX_DEBUG_3                         SENINF2_MUX_DEBUG_3;                             /* 1D18, 0x1A041D18 */
	SENINF2_REG_MUX_DEBUG_4                         SENINF2_MUX_DEBUG_4;                             /* 1D1C, 0x1A041D1C */
	SENINF2_REG_MUX_DEBUG_5                         SENINF2_MUX_DEBUG_5;                             /* 1D20, 0x1A041D20 */
	SENINF2_REG_MUX_DEBUG_6                         SENINF2_MUX_DEBUG_6;                             /* 1D24, 0x1A041D24 */
	SENINF2_REG_MUX_DEBUG_7                         SENINF2_MUX_DEBUG_7;                             /* 1D28, 0x1A041D28 */
	SENINF2_REG_MUX_SPARE                           SENINF2_MUX_SPARE;                               /* 1D2C, 0x1A041D2C */
	SENINF2_REG_MUX_DATA                            SENINF2_MUX_DATA;                                /* 1D30, 0x1A041D30 */
	SENINF2_REG_MUX_DATA_CNT                        SENINF2_MUX_DATA_CNT;                            /* 1D34, 0x1A041D34 */
	SENINF2_REG_MUX_CROP                            SENINF2_MUX_CROP;                                /* 1D38, 0x1A041D38 */
	SENINF2_REG_MUX_CTRL_EXT                        SENINF2_MUX_CTRL_EXT;                            /* 1D3C, 0x1A041D3C */
	UINT32                                          rsv_1D40[304];                                   /* 1D40..21FF, 0x1A041D40..1A0421FF */
	SENINF3_REG_CTRL                                SENINF3_CTRL;                                    /* 2200, 0x1A042200 */
	SENINF3_REG_CTRL_EXT                            SENINF3_CTRL_EXT;                                /* 2204, 0x1A042204 */
	SENINF3_REG_ASYNC_CTRL                          SENINF3_ASYNC_CTRL;                              /* 2208, 0x1A042208 */
	UINT32                                          rsv_220C[253];                                   /* 220C..25FF, 0x1A04220C..1A0425FF */
	SENINF_REG_TG3_PH_CNT                           SENINF_TG3_PH_CNT;                               /* 2600, 0x1A042600 */
	SENINF_REG_TG3_SEN_CK                           SENINF_TG3_SEN_CK;                               /* 2604, 0x1A042604 */
	SENINF_REG_TG3_TM_CTL                           SENINF_TG3_TM_CTL;                               /* 2608, 0x1A042608 */
	SENINF_REG_TG3_TM_SIZE                          SENINF_TG3_TM_SIZE;                              /* 260C, 0x1A04260C */
	SENINF_REG_TG3_TM_CLK                           SENINF_TG3_TM_CLK;                               /* 2610, 0x1A042610 */
	SENINF_REG_TG3_TM_STP                           SENINF_TG3_TM_STP;                               /* 2614, 0x1A042614 */
	UINT32                                          rsv_2618[131];                                   /* 2618..2823, 0x1A042618..1A042823 */
	MIPI_REG_RX_CON24_CSI2                          MIPI_RX_CON24_CSI2;                              /* 2824, 0x1A042824 */
	MIPI_REG_RX_CON28_CSI2                          MIPI_RX_CON28_CSI2;                              /* 2828, 0x1A042828 */
	UINT32                                          rsv_282C[2];                                     /* 282C..2833, 0x1A04282C..1A042833 */
	MIPI_REG_RX_CON34_CSI2                          MIPI_RX_CON34_CSI2;                              /* 2834, 0x1A042834 */
	MIPI_REG_RX_CON38_CSI2                          MIPI_RX_CON38_CSI2;                              /* 2838, 0x1A042838 */
	MIPI_REG_RX_CON3C_CSI2                          MIPI_RX_CON3C_CSI2;                              /* 283C, 0x1A04283C */
	UINT32                                          rsv_2840[15];                                    /* 2840..287B, 0x1A042840..1A04287B */
	MIPI_REG_RX_CON7C_CSI2                          MIPI_RX_CON7C_CSI2;                              /* 287C, 0x1A04287C */
	MIPI_REG_RX_CON80_CSI2                          MIPI_RX_CON80_CSI2;                              /* 2880, 0x1A042880 */
	MIPI_REG_RX_CON84_CSI2                          MIPI_RX_CON84_CSI2;                              /* 2884, 0x1A042884 */
	MIPI_REG_RX_CON88_CSI2                          MIPI_RX_CON88_CSI2;                              /* 2888, 0x1A042888 */
	MIPI_REG_RX_CON8C_CSI2                          MIPI_RX_CON8C_CSI2;                              /* 288C, 0x1A04288C */
	MIPI_REG_RX_CON90_CSI2                          MIPI_RX_CON90_CSI2;                              /* 2890, 0x1A042890 */
	MIPI_REG_RX_CON94_CSI2                          MIPI_RX_CON94_CSI2;                              /* 2894, 0x1A042894 */
	MIPI_REG_RX_CON98_CSI2                          MIPI_RX_CON98_CSI2;                              /* 2898, 0x1A042898 */
	UINT32                                          rsv_289C;                                        /* 289C, 0x1A04289C */
	MIPI_REG_RX_CONA0_CSI2                          MIPI_RX_CONA0_CSI2;                              /* 28A0, 0x1A0428A0 */
	UINT32                                          rsv_28A4[3];                                     /* 28A4..28AF, 0x1A0428A4..1A0428AF */
	MIPI_REG_RX_CONB0_CSI2                          MIPI_RX_CONB0_CSI2;                              /* 28B0, 0x1A0428B0 */
	MIPI_REG_RX_CONB4_CSI2                          MIPI_RX_CONB4_CSI2;                              /* 28B4, 0x1A0428B4 */
	MIPI_REG_RX_CONB8_CSI2                          MIPI_RX_CONB8_CSI2;                              /* 28B8, 0x1A0428B8 */
	MIPI_REG_RX_CONBC_CSI2                          MIPI_RX_CONBC_CSI2;                              /* 28BC, 0x1A0428BC */
	MIPI_REG_RX_CONC0_CSI2                          MIPI_RX_CONC0_CSI2;                              /* 28C0, 0x1A0428C0 */
	MIPI_REG_RX_CONC4_CSI2                          MIPI_RX_CONC4_CSI2;                              /* 28C4, 0x1A0428C4 */
	MIPI_REG_RX_CONC8_CSI2                          MIPI_RX_CONC8_CSI2;                              /* 28C8, 0x1A0428C8 */
	MIPI_REG_RX_CONCC_CSI2                          MIPI_RX_CONCC_CSI2;                              /* 28CC, 0x1A0428CC */
	MIPI_REG_RX_COND0_CSI2                          MIPI_RX_COND0_CSI2;                              /* 28D0, 0x1A0428D0 */
	UINT32                                          rsv_28D4[75];                                    /* 28D4..29FF, 0x1A0428D4..1A0429FF */
	SENINF3_REG_CSI2_CTL                            SENINF3_CSI2_CTL;                                /* 2A00, 0x1A042A00 */
	SENINF3_REG_CSI2_LNRC_TIMING                    SENINF3_CSI2_LNRC_TIMING;                        /* 2A04, 0x1A042A04 */
	SENINF3_REG_CSI2_LNRD_TIMING                    SENINF3_CSI2_LNRD_TIMING;                        /* 2A08, 0x1A042A08 */
	SENINF3_REG_CSI2_DPCM                           SENINF3_CSI2_DPCM;                               /* 2A0C, 0x1A042A0C */
	SENINF3_REG_CSI2_INT_EN                         SENINF3_CSI2_INT_EN;                             /* 2A10, 0x1A042A10 */
	SENINF3_REG_CSI2_INT_STATUS                     SENINF3_CSI2_INT_STATUS;                         /* 2A14, 0x1A042A14 */
	SENINF3_REG_CSI2_DGB_SEL                        SENINF3_CSI2_DGB_SEL;                            /* 2A18, 0x1A042A18 */
	SENINF3_REG_CSI2_DBG_PORT                       SENINF3_CSI2_DBG_PORT;                           /* 2A1C, 0x1A042A1C */
	SENINF3_REG_CSI2_SPARE0                         SENINF3_CSI2_SPARE0;                             /* 2A20, 0x1A042A20 */
	SENINF3_REG_CSI2_SPARE1                         SENINF3_CSI2_SPARE1;                             /* 2A24, 0x1A042A24 */
	SENINF3_REG_CSI2_LNRC_FSM                       SENINF3_CSI2_LNRC_FSM;                           /* 2A28, 0x1A042A28 */
	SENINF3_REG_CSI2_LNRD_FSM                       SENINF3_CSI2_LNRD_FSM;                           /* 2A2C, 0x1A042A2C */
	SENINF3_REG_CSI2_FRAME_LINE_NUM                 SENINF3_CSI2_FRAME_LINE_NUM;                     /* 2A30, 0x1A042A30 */
	SENINF3_REG_CSI2_GENERIC_SHORT                  SENINF3_CSI2_GENERIC_SHORT;                      /* 2A34, 0x1A042A34 */
	SENINF3_REG_CSI2_HSRX_DBG                       SENINF3_CSI2_HSRX_DBG;                           /* 2A38, 0x1A042A38 */
	SENINF3_REG_CSI2_DI                             SENINF3_CSI2_DI;                                 /* 2A3C, 0x1A042A3C */
	SENINF3_REG_CSI2_HS_TRAIL                       SENINF3_CSI2_HS_TRAIL;                           /* 2A40, 0x1A042A40 */
	SENINF3_REG_CSI2_DI_CTRL                        SENINF3_CSI2_DI_CTRL;                            /* 2A44, 0x1A042A44 */
	UINT32                                          rsv_2A48;                                        /* 2A48, 0x1A042A48 */
	SENINF3_REG_CSI2_DETECT_CON1                    SENINF3_CSI2_DETECT_CON1;                        /* 2A4C, 0x1A042A4C */
	SENINF3_REG_CSI2_DETECT_CON2                    SENINF3_CSI2_DETECT_CON2;                        /* 2A50, 0x1A042A50 */
	SENINF3_REG_CSI2_DETECT_CON3                    SENINF3_CSI2_DETECT_CON3;                        /* 2A54, 0x1A042A54 */
	SENINF3_REG_CSI2_RLR0_CON0                      SENINF3_CSI2_RLR0_CON0;                          /* 2A58, 0x1A042A58 */
	SENINF3_REG_CSI2_RLR1_CON0                      SENINF3_CSI2_RLR1_CON0;                          /* 2A5C, 0x1A042A5C */
	SENINF3_REG_CSI2_RLR2_CON0                      SENINF3_CSI2_RLR2_CON0;                          /* 2A60, 0x1A042A60 */
	SENINF3_REG_CSI2_RLR_CON0                       SENINF3_CSI2_RLR_CON0;                           /* 2A64, 0x1A042A64 */
	SENINF3_REG_CSI2_MUX_CON                        SENINF3_CSI2_MUX_CON;                            /* 2A68, 0x1A042A68 */
	SENINF3_REG_CSI2_DETECT_DBG0                    SENINF3_CSI2_DETECT_DBG0;                        /* 2A6C, 0x1A042A6C */
	SENINF3_REG_CSI2_DETECT_DBG1                    SENINF3_CSI2_DETECT_DBG1;                        /* 2A70, 0x1A042A70 */
	SENINF3_REG_CSI2_RESYNC_MERGE_CTL               SENINF3_CSI2_RESYNC_MERGE_CTL;                   /* 2A74, 0x1A042A74 */
	SENINF3_REG_CSI2_CTRL_TRIO_MUX                  SENINF3_CSI2_CTRL_TRIO_MUX;                      /* 2A78, 0x1A042A78 */
	SENINF3_REG_CSI2_CTRL_TRIO_CON                  SENINF3_CSI2_CTRL_TRIO_CON;                      /* 2A7C, 0x1A042A7C */
	SENINF3_REG_FIX_ADDR_CPHY0_DBG                  SENINF3_FIX_ADDR_CPHY0_DBG;                      /* 2A80, 0x1A042A80 */
	SENINF3_REG_FIX_ADDR_CPHY1_DBG                  SENINF3_FIX_ADDR_CPHY1_DBG;                      /* 2A84, 0x1A042A84 */
	SENINF3_REG_FIX_ADDR_CPHY2_DBG                  SENINF3_FIX_ADDR_CPHY2_DBG;                      /* 2A88, 0x1A042A88 */
	SENINF3_REG_FIX_ADDR_DBG                        SENINF3_FIX_ADDR_DBG;                            /* 2A8C, 0x1A042A8C */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF3_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 2A90, 0x1A042A90 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF3_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 2A94, 0x1A042A94 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF3_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 2A98, 0x1A042A98 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF3_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 2A9C, 0x1A042A9C */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF3_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 2AA0, 0x1A042AA0 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF3_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 2AA4, 0x1A042AA4 */
	SENINF3_REG_SYNC_RESYNC_CTL                     SENINF3_SYNC_RESYNC_CTL;                         /* 2AA8, 0x1A042AA8 */
	SENINF3_REG_POST_DETECT_CTL                     SENINF3_POST_DETECT_CTL;                         /* 2AAC, 0x1A042AAC */
	SENINF3_REG_WIRE_STATE_DECODE_CONFIG            SENINF3_WIRE_STATE_DECODE_CONFIG;                /* 2AB0, 0x1A042AB0 */
	SENINF3_REG_CSI2_CPHY_LNRD_FSM                  SENINF3_CSI2_CPHY_LNRD_FSM;                      /* 2AB4, 0x1A042AB4 */
	SENINF3_REG_FIX_ADDR_CPHY0_DBG0                 SENINF3_FIX_ADDR_CPHY0_DBG0;                     /* 2AB8, 0x1A042AB8 */
	SENINF3_REG_FIX_ADDR_CPHY0_DBG1                 SENINF3_FIX_ADDR_CPHY0_DBG1;                     /* 2ABC, 0x1A042ABC */
	SENINF3_REG_FIX_ADDR_CPHY0_DBG2                 SENINF3_FIX_ADDR_CPHY0_DBG2;                     /* 2AC0, 0x1A042AC0 */
	SENINF3_REG_FIX_ADDR_CPHY1_DBG0                 SENINF3_FIX_ADDR_CPHY1_DBG0;                     /* 2AC4, 0x1A042AC4 */
	SENINF3_REG_FIX_ADDR_CPHY1_DBG1                 SENINF3_FIX_ADDR_CPHY1_DBG1;                     /* 2AC8, 0x1A042AC8 */
	SENINF3_REG_FIX_ADDR_CPHY1_DBG2                 SENINF3_FIX_ADDR_CPHY1_DBG2;                     /* 2ACC, 0x1A042ACC */
	SENINF3_REG_FIX_ADDR_CPHY2_DBG0                 SENINF3_FIX_ADDR_CPHY2_DBG0;                     /* 2AD0, 0x1A042AD0 */
	SENINF3_REG_FIX_ADDR_CPHY2_DBG1                 SENINF3_FIX_ADDR_CPHY2_DBG1;                     /* 2AD4, 0x1A042AD4 */
	SENINF3_REG_FIX_ADDR_CPHY2_DBG2                 SENINF3_FIX_ADDR_CPHY2_DBG2;                     /* 2AD8, 0x1A042AD8 */
	SENINF3_REG_FIX_ADDR_DBG0                       SENINF3_FIX_ADDR_DBG0;                           /* 2ADC, 0x1A042ADC */
	SENINF3_REG_FIX_ADDR_DBG1                       SENINF3_FIX_ADDR_DBG1;                           /* 2AE0, 0x1A042AE0 */
	SENINF3_REG_FIX_ADDR_DBG2                       SENINF3_FIX_ADDR_DBG2;                           /* 2AE4, 0x1A042AE4 */
	SENINF3_REG_CSI2_MODE                           SENINF3_CSI2_MODE;                               /* 2AE8, 0x1A042AE8 */
	UINT32                                          rsv_2AEC;                                        /* 2AEC, 0x1A042AEC */
	SENINF3_REG_CSI2_DI_EXT                         SENINF3_CSI2_DI_EXT;                             /* 2AF0, 0x1A042AF0 */
	SENINF3_REG_CSI2_DI_CTRL_EXT                    SENINF3_CSI2_DI_CTRL_EXT;                        /* 2AF4, 0x1A042AF4 */
	SENINF3_REG_CSI2_CPHY_LOOPBACK                  SENINF3_CSI2_CPHY_LOOPBACK;                      /* 2AF8, 0x1A042AF8 */
	UINT32                                          rsv_2AFC;                                        /* 2AFC, 0x1A042AFC */
	SENINF3_REG_CSI2_PROGSEQ_0                      SENINF3_CSI2_PROGSEQ_0;                          /* 2B00, 0x1A042B00 */
	SENINF3_REG_CSI2_PROGSEQ_1                      SENINF3_CSI2_PROGSEQ_1;                          /* 2B04, 0x1A042B04 */
	UINT32                                          rsv_2B08[2];                                     /* 2B08..2B0F, 0x1A042B08..1A042B0F */
	SENINF3_REG_CSI2_INT_EN_EXT                     SENINF3_CSI2_INT_EN_EXT;                         /* 2B10, 0x1A042B10 */
	SENINF3_REG_CSI2_INT_STATUS_EXT                 SENINF3_CSI2_INT_STATUS_EXT;                     /* 2B14, 0x1A042B14 */
	SENINF3_REG_CSI2_CPHY_FIX_POINT_RST             SENINF3_CSI2_CPHY_FIX_POINT_RST;                 /* 2B18, 0x1A042B18 */
	SENINF3_REG_CSI2_RLR3_CON0                      SENINF3_CSI2_RLR3_CON0;                          /* 2B1C, 0x1A042B1C */
	SENINF3_REG_CSI2_DPHY_SYNC                      SENINF3_CSI2_DPHY_SYNC;                          /* 2B20, 0x1A042B20 */
	SENINF3_REG_CSI2_DESKEW_SYNC                    SENINF3_CSI2_DESKEW_SYNC;                        /* 2B24, 0x1A042B24 */
	SENINF3_REG_CSI2_DETECT_DBG2                    SENINF3_CSI2_DETECT_DBG2;                        /* 2B28, 0x1A042B28 */
	UINT32                                          rsv_2B2C;                                        /* 2B2C, 0x1A042B2C */
	SENINF3_REG_FIX_ADDR_CPHY3_DBG0                 SENINF3_FIX_ADDR_CPHY3_DBG0;                     /* 2B30, 0x1A042B30 */
	SENINF3_REG_FIX_ADDR_CPHY3_DBG1                 SENINF3_FIX_ADDR_CPHY3_DBG1;                     /* 2B34, 0x1A042B34 */
	SENINF3_REG_FIX_ADDR_CPHY3_DBG2                 SENINF3_FIX_ADDR_CPHY3_DBG2;                     /* 2B38, 0x1A042B38 */
	SENINF3_REG_CSI2_DI_EXT_2                       SENINF3_CSI2_DI_EXT_2;                           /* 2B3C, 0x1A042B3C */
	SENINF3_REG_CSI2_DI_CTRL_EXT_2                  SENINF3_CSI2_DI_CTRL_EXT_2;                      /* 2B40, 0x1A042B40 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF3_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 2B44, 0x1A042B44 */
	SENINF3_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF3_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 2B48, 0x1A042B48 */
	UINT32                                          rsv_2B4C[109];                                   /* 2B4C..2CFF, 0x1A042B4C..1A042CFF */
	SENINF3_REG_MUX_CTRL                            SENINF3_MUX_CTRL;                                /* 2D00, 0x1A042D00 */
	SENINF3_REG_MUX_INTEN                           SENINF3_MUX_INTEN;                               /* 2D04, 0x1A042D04 */
	SENINF3_REG_MUX_INTSTA                          SENINF3_MUX_INTSTA;                              /* 2D08, 0x1A042D08 */
	SENINF3_REG_MUX_SIZE                            SENINF3_MUX_SIZE;                                /* 2D0C, 0x1A042D0C */
	SENINF3_REG_MUX_DEBUG_1                         SENINF3_MUX_DEBUG_1;                             /* 2D10, 0x1A042D10 */
	SENINF3_REG_MUX_DEBUG_2                         SENINF3_MUX_DEBUG_2;                             /* 2D14, 0x1A042D14 */
	SENINF3_REG_MUX_DEBUG_3                         SENINF3_MUX_DEBUG_3;                             /* 2D18, 0x1A042D18 */
	SENINF3_REG_MUX_DEBUG_4                         SENINF3_MUX_DEBUG_4;                             /* 2D1C, 0x1A042D1C */
	SENINF3_REG_MUX_DEBUG_5                         SENINF3_MUX_DEBUG_5;                             /* 2D20, 0x1A042D20 */
	SENINF3_REG_MUX_DEBUG_6                         SENINF3_MUX_DEBUG_6;                             /* 2D24, 0x1A042D24 */
	SENINF3_REG_MUX_DEBUG_7                         SENINF3_MUX_DEBUG_7;                             /* 2D28, 0x1A042D28 */
	SENINF3_REG_MUX_SPARE                           SENINF3_MUX_SPARE;                               /* 2D2C, 0x1A042D2C */
	SENINF3_REG_MUX_DATA                            SENINF3_MUX_DATA;                                /* 2D30, 0x1A042D30 */
	SENINF3_REG_MUX_DATA_CNT                        SENINF3_MUX_DATA_CNT;                            /* 2D34, 0x1A042D34 */
	SENINF3_REG_MUX_CROP                            SENINF3_MUX_CROP;                                /* 2D38, 0x1A042D38 */
	SENINF3_REG_MUX_CTRL_EXT                        SENINF3_MUX_CTRL_EXT;                            /* 2D3C, 0x1A042D3C */
	UINT32                                          rsv_2D40[304];                                   /* 2D40..31FF, 0x1A042D40..1A0431FF */
	SENINF4_REG_CTRL                                SENINF4_CTRL;                                    /* 3200, 0x1A043200 */
	SENINF4_REG_CTRL_EXT                            SENINF4_CTRL_EXT;                                /* 3204, 0x1A043204 */
	SENINF4_REG_ASYNC_CTRL                          SENINF4_ASYNC_CTRL;                              /* 3208, 0x1A043208 */
	UINT32                                          rsv_320C[253];                                   /* 320C..35FF, 0x1A04320C..1A0435FF */
	SENINF_REG_TG4_PH_CNT                           SENINF_TG4_PH_CNT;                               /* 3600, 0x1A043600 */
	SENINF_REG_TG4_SEN_CK                           SENINF_TG4_SEN_CK;                               /* 3604, 0x1A043604 */
	SENINF_REG_TG4_TM_CTL                           SENINF_TG4_TM_CTL;                               /* 3608, 0x1A043608 */
	SENINF_REG_TG4_TM_SIZE                          SENINF_TG4_TM_SIZE;                              /* 360C, 0x1A04360C */
	SENINF_REG_TG4_TM_CLK                           SENINF_TG4_TM_CLK;                               /* 3610, 0x1A043610 */
	SENINF_REG_TG4_TM_STP                           SENINF_TG4_TM_STP;                               /* 3614, 0x1A043614 */
	UINT32                                          rsv_3618[131];                                   /* 3618..3823, 0x1A043618..1A043823 */
	MIPI_REG_RX_CON24_CSI3                          MIPI_RX_CON24_CSI3;                              /* 3824, 0x1A043824 */
	MIPI_REG_RX_CON28_CSI3                          MIPI_RX_CON28_CSI3;                              /* 3828, 0x1A043828 */
	UINT32                                          rsv_382C[2];                                     /* 382C..3833, 0x1A04382C..1A043833 */
	MIPI_REG_RX_CON34_CSI3                          MIPI_RX_CON34_CSI3;                              /* 3834, 0x1A043834 */
	MIPI_REG_RX_CON38_CSI3                          MIPI_RX_CON38_CSI3;                              /* 3838, 0x1A043838 */
	MIPI_REG_RX_CON3C_CSI3                          MIPI_RX_CON3C_CSI3;                              /* 383C, 0x1A04383C */
	UINT32                                          rsv_3840[15];                                    /* 3840..387B, 0x1A043840..1A04387B */
	MIPI_REG_RX_CON7C_CSI3                          MIPI_RX_CON7C_CSI3;                              /* 387C, 0x1A04387C */
	MIPI_REG_RX_CON80_CSI3                          MIPI_RX_CON80_CSI3;                              /* 3880, 0x1A043880 */
	MIPI_REG_RX_CON84_CSI3                          MIPI_RX_CON84_CSI3;                              /* 3884, 0x1A043884 */
	MIPI_REG_RX_CON88_CSI3                          MIPI_RX_CON88_CSI3;                              /* 3888, 0x1A043888 */
	MIPI_REG_RX_CON8C_CSI3                          MIPI_RX_CON8C_CSI3;                              /* 388C, 0x1A04388C */
	MIPI_REG_RX_CON90_CSI3                          MIPI_RX_CON90_CSI3;                              /* 3890, 0x1A043890 */
	MIPI_REG_RX_CON94_CSI3                          MIPI_RX_CON94_CSI3;                              /* 3894, 0x1A043894 */
	MIPI_REG_RX_CON98_CSI3                          MIPI_RX_CON98_CSI3;                              /* 3898, 0x1A043898 */
	UINT32                                          rsv_389C;                                        /* 389C, 0x1A04389C */
	MIPI_REG_RX_CONA0_CSI3                          MIPI_RX_CONA0_CSI3;                              /* 38A0, 0x1A0438A0 */
	UINT32                                          rsv_38A4[3];                                     /* 38A4..38AF, 0x1A0438A4..1A0438AF */
	MIPI_REG_RX_CONB0_CSI3                          MIPI_RX_CONB0_CSI3;                              /* 38B0, 0x1A0438B0 */
	MIPI_REG_RX_CONB4_CSI3                          MIPI_RX_CONB4_CSI3;                              /* 38B4, 0x1A0438B4 */
	MIPI_REG_RX_CONB8_CSI3                          MIPI_RX_CONB8_CSI3;                              /* 38B8, 0x1A0438B8 */
	MIPI_REG_RX_CONBC_CSI3                          MIPI_RX_CONBC_CSI3;                              /* 38BC, 0x1A0438BC */
	MIPI_REG_RX_CONC0_CSI3                          MIPI_RX_CONC0_CSI3;                              /* 38C0, 0x1A0438C0 */
	MIPI_REG_RX_CONC4_CSI3                          MIPI_RX_CONC4_CSI3;                              /* 38C4, 0x1A0438C4 */
	MIPI_REG_RX_CONC8_CSI3                          MIPI_RX_CONC8_CSI3;                              /* 38C8, 0x1A0438C8 */
	MIPI_REG_RX_CONCC_CSI3                          MIPI_RX_CONCC_CSI3;                              /* 38CC, 0x1A0438CC */
	MIPI_REG_RX_COND0_CSI3                          MIPI_RX_COND0_CSI3;                              /* 38D0, 0x1A0438D0 */
	UINT32                                          rsv_38D4[75];                                    /* 38D4..39FF, 0x1A0438D4..1A0439FF */
	SENINF4_REG_CSI2_CTL                            SENINF4_CSI2_CTL;                                /* 3A00, 0x1A043A00 */
	SENINF4_REG_CSI2_LNRC_TIMING                    SENINF4_CSI2_LNRC_TIMING;                        /* 3A04, 0x1A043A04 */
	SENINF4_REG_CSI2_LNRD_TIMING                    SENINF4_CSI2_LNRD_TIMING;                        /* 3A08, 0x1A043A08 */
	SENINF4_REG_CSI2_DPCM                           SENINF4_CSI2_DPCM;                               /* 3A0C, 0x1A043A0C */
	SENINF4_REG_CSI2_INT_EN                         SENINF4_CSI2_INT_EN;                             /* 3A10, 0x1A043A10 */
	SENINF4_REG_CSI2_INT_STATUS                     SENINF4_CSI2_INT_STATUS;                         /* 3A14, 0x1A043A14 */
	SENINF4_REG_CSI2_DGB_SEL                        SENINF4_CSI2_DGB_SEL;                            /* 3A18, 0x1A043A18 */
	SENINF4_REG_CSI2_DBG_PORT                       SENINF4_CSI2_DBG_PORT;                           /* 3A1C, 0x1A043A1C */
	SENINF4_REG_CSI2_SPARE0                         SENINF4_CSI2_SPARE0;                             /* 3A20, 0x1A043A20 */
	SENINF4_REG_CSI2_SPARE1                         SENINF4_CSI2_SPARE1;                             /* 3A24, 0x1A043A24 */
	SENINF4_REG_CSI2_LNRC_FSM                       SENINF4_CSI2_LNRC_FSM;                           /* 3A28, 0x1A043A28 */
	SENINF4_REG_CSI2_LNRD_FSM                       SENINF4_CSI2_LNRD_FSM;                           /* 3A2C, 0x1A043A2C */
	SENINF4_REG_CSI2_FRAME_LINE_NUM                 SENINF4_CSI2_FRAME_LINE_NUM;                     /* 3A30, 0x1A043A30 */
	SENINF4_REG_CSI2_GENERIC_SHORT                  SENINF4_CSI2_GENERIC_SHORT;                      /* 3A34, 0x1A043A34 */
	SENINF4_REG_CSI2_HSRX_DBG                       SENINF4_CSI2_HSRX_DBG;                           /* 3A38, 0x1A043A38 */
	SENINF4_REG_CSI2_DI                             SENINF4_CSI2_DI;                                 /* 3A3C, 0x1A043A3C */
	SENINF4_REG_CSI2_HS_TRAIL                       SENINF4_CSI2_HS_TRAIL;                           /* 3A40, 0x1A043A40 */
	SENINF4_REG_CSI2_DI_CTRL                        SENINF4_CSI2_DI_CTRL;                            /* 3A44, 0x1A043A44 */
	UINT32                                          rsv_3A48;                                        /* 3A48, 0x1A043A48 */
	SENINF4_REG_CSI2_DETECT_CON1                    SENINF4_CSI2_DETECT_CON1;                        /* 3A4C, 0x1A043A4C */
	SENINF4_REG_CSI2_DETECT_CON2                    SENINF4_CSI2_DETECT_CON2;                        /* 3A50, 0x1A043A50 */
	SENINF4_REG_CSI2_DETECT_CON3                    SENINF4_CSI2_DETECT_CON3;                        /* 3A54, 0x1A043A54 */
	SENINF4_REG_CSI2_RLR0_CON0                      SENINF4_CSI2_RLR0_CON0;                          /* 3A58, 0x1A043A58 */
	SENINF4_REG_CSI2_RLR1_CON0                      SENINF4_CSI2_RLR1_CON0;                          /* 3A5C, 0x1A043A5C */
	SENINF4_REG_CSI2_RLR2_CON0                      SENINF4_CSI2_RLR2_CON0;                          /* 3A60, 0x1A043A60 */
	SENINF4_REG_CSI2_RLR_CON0                       SENINF4_CSI2_RLR_CON0;                           /* 3A64, 0x1A043A64 */
	SENINF4_REG_CSI2_MUX_CON                        SENINF4_CSI2_MUX_CON;                            /* 3A68, 0x1A043A68 */
	SENINF4_REG_CSI2_DETECT_DBG0                    SENINF4_CSI2_DETECT_DBG0;                        /* 3A6C, 0x1A043A6C */
	SENINF4_REG_CSI2_DETECT_DBG1                    SENINF4_CSI2_DETECT_DBG1;                        /* 3A70, 0x1A043A70 */
	SENINF4_REG_CSI2_RESYNC_MERGE_CTL               SENINF4_CSI2_RESYNC_MERGE_CTL;                   /* 3A74, 0x1A043A74 */
	SENINF4_REG_CSI2_CTRL_TRIO_MUX                  SENINF4_CSI2_CTRL_TRIO_MUX;                      /* 3A78, 0x1A043A78 */
	SENINF4_REG_CSI2_CTRL_TRIO_CON                  SENINF4_CSI2_CTRL_TRIO_CON;                      /* 3A7C, 0x1A043A7C */
	SENINF4_REG_FIX_ADDR_CPHY0_DBG                  SENINF4_FIX_ADDR_CPHY0_DBG;                      /* 3A80, 0x1A043A80 */
	SENINF4_REG_FIX_ADDR_CPHY1_DBG                  SENINF4_FIX_ADDR_CPHY1_DBG;                      /* 3A84, 0x1A043A84 */
	SENINF4_REG_FIX_ADDR_CPHY2_DBG                  SENINF4_FIX_ADDR_CPHY2_DBG;                      /* 3A88, 0x1A043A88 */
	SENINF4_REG_FIX_ADDR_DBG                        SENINF4_FIX_ADDR_DBG;                            /* 3A8C, 0x1A043A8C */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF4_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 3A90, 0x1A043A90 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF4_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 3A94, 0x1A043A94 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF4_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 3A98, 0x1A043A98 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF4_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 3A9C, 0x1A043A9C */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF4_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 3AA0, 0x1A043AA0 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF4_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 3AA4, 0x1A043AA4 */
	SENINF4_REG_SYNC_RESYNC_CTL                     SENINF4_SYNC_RESYNC_CTL;                         /* 3AA8, 0x1A043AA8 */
	SENINF4_REG_POST_DETECT_CTL                     SENINF4_POST_DETECT_CTL;                         /* 3AAC, 0x1A043AAC */
	SENINF4_REG_WIRE_STATE_DECODE_CONFIG            SENINF4_WIRE_STATE_DECODE_CONFIG;                /* 3AB0, 0x1A043AB0 */
	SENINF4_REG_CSI2_CPHY_LNRD_FSM                  SENINF4_CSI2_CPHY_LNRD_FSM;                      /* 3AB4, 0x1A043AB4 */
	SENINF4_REG_FIX_ADDR_CPHY0_DBG0                 SENINF4_FIX_ADDR_CPHY0_DBG0;                     /* 3AB8, 0x1A043AB8 */
	SENINF4_REG_FIX_ADDR_CPHY0_DBG1                 SENINF4_FIX_ADDR_CPHY0_DBG1;                     /* 3ABC, 0x1A043ABC */
	SENINF4_REG_FIX_ADDR_CPHY0_DBG2                 SENINF4_FIX_ADDR_CPHY0_DBG2;                     /* 3AC0, 0x1A043AC0 */
	SENINF4_REG_FIX_ADDR_CPHY1_DBG0                 SENINF4_FIX_ADDR_CPHY1_DBG0;                     /* 3AC4, 0x1A043AC4 */
	SENINF4_REG_FIX_ADDR_CPHY1_DBG1                 SENINF4_FIX_ADDR_CPHY1_DBG1;                     /* 3AC8, 0x1A043AC8 */
	SENINF4_REG_FIX_ADDR_CPHY1_DBG2                 SENINF4_FIX_ADDR_CPHY1_DBG2;                     /* 3ACC, 0x1A043ACC */
	SENINF4_REG_FIX_ADDR_CPHY2_DBG0                 SENINF4_FIX_ADDR_CPHY2_DBG0;                     /* 3AD0, 0x1A043AD0 */
	SENINF4_REG_FIX_ADDR_CPHY2_DBG1                 SENINF4_FIX_ADDR_CPHY2_DBG1;                     /* 3AD4, 0x1A043AD4 */
	SENINF4_REG_FIX_ADDR_CPHY2_DBG2                 SENINF4_FIX_ADDR_CPHY2_DBG2;                     /* 3AD8, 0x1A043AD8 */
	SENINF4_REG_FIX_ADDR_DBG0                       SENINF4_FIX_ADDR_DBG0;                           /* 3ADC, 0x1A043ADC */
	SENINF4_REG_FIX_ADDR_DBG1                       SENINF4_FIX_ADDR_DBG1;                           /* 3AE0, 0x1A043AE0 */
	SENINF4_REG_FIX_ADDR_DBG2                       SENINF4_FIX_ADDR_DBG2;                           /* 3AE4, 0x1A043AE4 */
	SENINF4_REG_CSI2_MODE                           SENINF4_CSI2_MODE;                               /* 3AE8, 0x1A043AE8 */
	UINT32                                          rsv_3AEC;                                        /* 3AEC, 0x1A043AEC */
	SENINF4_REG_CSI2_DI_EXT                         SENINF4_CSI2_DI_EXT;                             /* 3AF0, 0x1A043AF0 */
	SENINF4_REG_CSI2_DI_CTRL_EXT                    SENINF4_CSI2_DI_CTRL_EXT;                        /* 3AF4, 0x1A043AF4 */
	SENINF4_REG_CSI2_CPHY_LOOPBACK                  SENINF4_CSI2_CPHY_LOOPBACK;                      /* 3AF8, 0x1A043AF8 */
	UINT32                                          rsv_3AFC;                                        /* 3AFC, 0x1A043AFC */
	SENINF4_REG_CSI2_PROGSEQ_0                      SENINF4_CSI2_PROGSEQ_0;                          /* 3B00, 0x1A043B00 */
	SENINF4_REG_CSI2_PROGSEQ_1                      SENINF4_CSI2_PROGSEQ_1;                          /* 3B04, 0x1A043B04 */
	UINT32                                          rsv_3B08[2];                                     /* 3B08..3B0F, 0x1A043B08..1A043B0F */
	SENINF4_REG_CSI2_INT_EN_EXT                     SENINF4_CSI2_INT_EN_EXT;                         /* 3B10, 0x1A043B10 */
	SENINF4_REG_CSI2_INT_STATUS_EXT                 SENINF4_CSI2_INT_STATUS_EXT;                     /* 3B14, 0x1A043B14 */
	SENINF4_REG_CSI2_CPHY_FIX_POINT_RST             SENINF4_CSI2_CPHY_FIX_POINT_RST;                 /* 3B18, 0x1A043B18 */
	SENINF4_REG_CSI2_RLR3_CON0                      SENINF4_CSI2_RLR3_CON0;                          /* 3B1C, 0x1A043B1C */
	SENINF4_REG_CSI2_DPHY_SYNC                      SENINF4_CSI2_DPHY_SYNC;                          /* 3B20, 0x1A043B20 */
	SENINF4_REG_CSI2_DESKEW_SYNC                    SENINF4_CSI2_DESKEW_SYNC;                        /* 3B24, 0x1A043B24 */
	SENINF4_REG_CSI2_DETECT_DBG2                    SENINF4_CSI2_DETECT_DBG2;                        /* 3B28, 0x1A043B28 */
	UINT32                                          rsv_3B2C;                                        /* 3B2C, 0x1A043B2C */
	SENINF4_REG_FIX_ADDR_CPHY3_DBG0                 SENINF4_FIX_ADDR_CPHY3_DBG0;                     /* 3B30, 0x1A043B30 */
	SENINF4_REG_FIX_ADDR_CPHY3_DBG1                 SENINF4_FIX_ADDR_CPHY3_DBG1;                     /* 3B34, 0x1A043B34 */
	SENINF4_REG_FIX_ADDR_CPHY3_DBG2                 SENINF4_FIX_ADDR_CPHY3_DBG2;                     /* 3B38, 0x1A043B38 */
	SENINF4_REG_CSI2_DI_EXT_2                       SENINF4_CSI2_DI_EXT_2;                           /* 3B3C, 0x1A043B3C */
	SENINF4_REG_CSI2_DI_CTRL_EXT_2                  SENINF4_CSI2_DI_CTRL_EXT_2;                      /* 3B40, 0x1A043B40 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF4_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 3B44, 0x1A043B44 */
	SENINF4_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF4_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 3B48, 0x1A043B48 */
	UINT32                                          rsv_3B4C[109];                                   /* 3B4C..3CFF, 0x1A043B4C..1A043CFF */
	SENINF4_REG_MUX_CTRL                            SENINF4_MUX_CTRL;                                /* 3D00, 0x1A043D00 */
	SENINF4_REG_MUX_INTEN                           SENINF4_MUX_INTEN;                               /* 3D04, 0x1A043D04 */
	SENINF4_REG_MUX_INTSTA                          SENINF4_MUX_INTSTA;                              /* 3D08, 0x1A043D08 */
	SENINF4_REG_MUX_SIZE                            SENINF4_MUX_SIZE;                                /* 3D0C, 0x1A043D0C */
	SENINF4_REG_MUX_DEBUG_1                         SENINF4_MUX_DEBUG_1;                             /* 3D10, 0x1A043D10 */
	SENINF4_REG_MUX_DEBUG_2                         SENINF4_MUX_DEBUG_2;                             /* 3D14, 0x1A043D14 */
	SENINF4_REG_MUX_DEBUG_3                         SENINF4_MUX_DEBUG_3;                             /* 3D18, 0x1A043D18 */
	SENINF4_REG_MUX_DEBUG_4                         SENINF4_MUX_DEBUG_4;                             /* 3D1C, 0x1A043D1C */
	SENINF4_REG_MUX_DEBUG_5                         SENINF4_MUX_DEBUG_5;                             /* 3D20, 0x1A043D20 */
	SENINF4_REG_MUX_DEBUG_6                         SENINF4_MUX_DEBUG_6;                             /* 3D24, 0x1A043D24 */
	SENINF4_REG_MUX_DEBUG_7                         SENINF4_MUX_DEBUG_7;                             /* 3D28, 0x1A043D28 */
	SENINF4_REG_MUX_SPARE                           SENINF4_MUX_SPARE;                               /* 3D2C, 0x1A043D2C */
	SENINF4_REG_MUX_DATA                            SENINF4_MUX_DATA;                                /* 3D30, 0x1A043D30 */
	SENINF4_REG_MUX_DATA_CNT                        SENINF4_MUX_DATA_CNT;                            /* 3D34, 0x1A043D34 */
	SENINF4_REG_MUX_CROP                            SENINF4_MUX_CROP;                                /* 3D38, 0x1A043D38 */
	SENINF4_REG_MUX_CTRL_EXT                        SENINF4_MUX_CTRL_EXT;                            /* 3D3C, 0x1A043D3C */
	UINT32                                          rsv_3D40[304];                                   /* 3D40..41FF, 0x1A043D40..1A0441FF */
	SENINF5_REG_CTRL                                SENINF5_CTRL;                                    /* 4200, 0x1A044200 */
	SENINF5_REG_CTRL_EXT                            SENINF5_CTRL_EXT;                                /* 4204, 0x1A044204 */
	SENINF5_REG_ASYNC_CTRL                          SENINF5_ASYNC_CTRL;                              /* 4208, 0x1A044208 */
	UINT32                                          rsv_420C[253];                                   /* 420C..45FF, 0x1A04420C..1A0445FF */
	SENINF_REG_TG5_PH_CNT                           SENINF_TG5_PH_CNT;                               /* 4600, 0x1A044600 */
	SENINF_REG_TG5_SEN_CK                           SENINF_TG5_SEN_CK;                               /* 4604, 0x1A044604 */
	SENINF_REG_TG5_TM_CTL                           SENINF_TG5_TM_CTL;                               /* 4608, 0x1A044608 */
	SENINF_REG_TG5_TM_SIZE                          SENINF_TG5_TM_SIZE;                              /* 460C, 0x1A04460C */
	SENINF_REG_TG5_TM_CLK                           SENINF_TG5_TM_CLK;                               /* 4610, 0x1A044610 */
	SENINF_REG_TG5_TM_STP                           SENINF_TG5_TM_STP;                               /* 4614, 0x1A044614 */
	UINT32                                          rsv_4618[131];                                   /* 4618..4823, 0x1A044618..1A044823 */
	MIPI_REG_RX_CON24_CSI4                          MIPI_RX_CON24_CSI4;                              /* 4824, 0x1A044824 */
	MIPI_REG_RX_CON28_CSI4                          MIPI_RX_CON28_CSI4;                              /* 4828, 0x1A044828 */
	UINT32                                          rsv_482C[2];                                     /* 482C..4833, 0x1A04482C..1A044833 */
	MIPI_REG_RX_CON34_CSI4                          MIPI_RX_CON34_CSI4;                              /* 4834, 0x1A044834 */
	MIPI_REG_RX_CON38_CSI4                          MIPI_RX_CON38_CSI4;                              /* 4838, 0x1A044838 */
	MIPI_REG_RX_CON3C_CSI4                          MIPI_RX_CON3C_CSI4;                              /* 483C, 0x1A04483C */
	UINT32                                          rsv_4840[15];                                    /* 4840..487B, 0x1A044840..1A04487B */
	MIPI_REG_RX_CON7C_CSI4                          MIPI_RX_CON7C_CSI4;                              /* 487C, 0x1A04487C */
	MIPI_REG_RX_CON80_CSI4                          MIPI_RX_CON80_CSI4;                              /* 4880, 0x1A044880 */
	MIPI_REG_RX_CON84_CSI4                          MIPI_RX_CON84_CSI4;                              /* 4884, 0x1A044884 */
	MIPI_REG_RX_CON88_CSI4                          MIPI_RX_CON88_CSI4;                              /* 4888, 0x1A044888 */
	MIPI_REG_RX_CON8C_CSI4                          MIPI_RX_CON8C_CSI4;                              /* 488C, 0x1A04488C */
	MIPI_REG_RX_CON90_CSI4                          MIPI_RX_CON90_CSI4;                              /* 4890, 0x1A044890 */
	MIPI_REG_RX_CON94_CSI4                          MIPI_RX_CON94_CSI4;                              /* 4894, 0x1A044894 */
	MIPI_REG_RX_CON98_CSI4                          MIPI_RX_CON98_CSI4;                              /* 4898, 0x1A044898 */
	UINT32                                          rsv_489C;                                        /* 489C, 0x1A04489C */
	MIPI_REG_RX_CONA0_CSI4                          MIPI_RX_CONA0_CSI4;                              /* 48A0, 0x1A0448A0 */
	UINT32                                          rsv_48A4[3];                                     /* 48A4..48AF, 0x1A0448A4..1A0448AF */
	MIPI_REG_RX_CONB0_CSI4                          MIPI_RX_CONB0_CSI4;                              /* 48B0, 0x1A0448B0 */
	MIPI_REG_RX_CONB4_CSI4                          MIPI_RX_CONB4_CSI4;                              /* 48B4, 0x1A0448B4 */
	MIPI_REG_RX_CONB8_CSI4                          MIPI_RX_CONB8_CSI4;                              /* 48B8, 0x1A0448B8 */
	MIPI_REG_RX_CONBC_CSI4                          MIPI_RX_CONBC_CSI4;                              /* 48BC, 0x1A0448BC */
	MIPI_REG_RX_CONC0_CSI4                          MIPI_RX_CONC0_CSI4;                              /* 48C0, 0x1A0448C0 */
	MIPI_REG_RX_CONC4_CSI4                          MIPI_RX_CONC4_CSI4;                              /* 48C4, 0x1A0448C4 */
	MIPI_REG_RX_CONC8_CSI4                          MIPI_RX_CONC8_CSI4;                              /* 48C8, 0x1A0448C8 */
	MIPI_REG_RX_CONCC_CSI4                          MIPI_RX_CONCC_CSI4;                              /* 48CC, 0x1A0448CC */
	MIPI_REG_RX_COND0_CSI4                          MIPI_RX_COND0_CSI4;                              /* 48D0, 0x1A0448D0 */
	UINT32                                          rsv_48D4[75];                                    /* 48D4..49FF, 0x1A0448D4..1A0449FF */
	SENINF5_REG_CSI2_CTL                            SENINF5_CSI2_CTL;                                /* 4A00, 0x1A044A00 */
	SENINF5_REG_CSI2_LNRC_TIMING                    SENINF5_CSI2_LNRC_TIMING;                        /* 4A04, 0x1A044A04 */
	SENINF5_REG_CSI2_LNRD_TIMING                    SENINF5_CSI2_LNRD_TIMING;                        /* 4A08, 0x1A044A08 */
	SENINF5_REG_CSI2_DPCM                           SENINF5_CSI2_DPCM;                               /* 4A0C, 0x1A044A0C */
	SENINF5_REG_CSI2_INT_EN                         SENINF5_CSI2_INT_EN;                             /* 4A10, 0x1A044A10 */
	SENINF5_REG_CSI2_INT_STATUS                     SENINF5_CSI2_INT_STATUS;                         /* 4A14, 0x1A044A14 */
	SENINF5_REG_CSI2_DGB_SEL                        SENINF5_CSI2_DGB_SEL;                            /* 4A18, 0x1A044A18 */
	SENINF5_REG_CSI2_DBG_PORT                       SENINF5_CSI2_DBG_PORT;                           /* 4A1C, 0x1A044A1C */
	SENINF5_REG_CSI2_SPARE0                         SENINF5_CSI2_SPARE0;                             /* 4A20, 0x1A044A20 */
	SENINF5_REG_CSI2_SPARE1                         SENINF5_CSI2_SPARE1;                             /* 4A24, 0x1A044A24 */
	SENINF5_REG_CSI2_LNRC_FSM                       SENINF5_CSI2_LNRC_FSM;                           /* 4A28, 0x1A044A28 */
	SENINF5_REG_CSI2_LNRD_FSM                       SENINF5_CSI2_LNRD_FSM;                           /* 4A2C, 0x1A044A2C */
	SENINF5_REG_CSI2_FRAME_LINE_NUM                 SENINF5_CSI2_FRAME_LINE_NUM;                     /* 4A30, 0x1A044A30 */
	SENINF5_REG_CSI2_GENERIC_SHORT                  SENINF5_CSI2_GENERIC_SHORT;                      /* 4A34, 0x1A044A34 */
	SENINF5_REG_CSI2_HSRX_DBG                       SENINF5_CSI2_HSRX_DBG;                           /* 4A38, 0x1A044A38 */
	SENINF5_REG_CSI2_DI                             SENINF5_CSI2_DI;                                 /* 4A3C, 0x1A044A3C */
	SENINF5_REG_CSI2_HS_TRAIL                       SENINF5_CSI2_HS_TRAIL;                           /* 4A40, 0x1A044A40 */
	SENINF5_REG_CSI2_DI_CTRL                        SENINF5_CSI2_DI_CTRL;                            /* 4A44, 0x1A044A44 */
	UINT32                                          rsv_4A48;                                        /* 4A48, 0x1A044A48 */
	SENINF5_REG_CSI2_DETECT_CON1                    SENINF5_CSI2_DETECT_CON1;                        /* 4A4C, 0x1A044A4C */
	SENINF5_REG_CSI2_DETECT_CON2                    SENINF5_CSI2_DETECT_CON2;                        /* 4A50, 0x1A044A50 */
	SENINF5_REG_CSI2_DETECT_CON3                    SENINF5_CSI2_DETECT_CON3;                        /* 4A54, 0x1A044A54 */
	SENINF5_REG_CSI2_RLR0_CON0                      SENINF5_CSI2_RLR0_CON0;                          /* 4A58, 0x1A044A58 */
	SENINF5_REG_CSI2_RLR1_CON0                      SENINF5_CSI2_RLR1_CON0;                          /* 4A5C, 0x1A044A5C */
	SENINF5_REG_CSI2_RLR2_CON0                      SENINF5_CSI2_RLR2_CON0;                          /* 4A60, 0x1A044A60 */
	SENINF5_REG_CSI2_RLR_CON0                       SENINF5_CSI2_RLR_CON0;                           /* 4A64, 0x1A044A64 */
	SENINF5_REG_CSI2_MUX_CON                        SENINF5_CSI2_MUX_CON;                            /* 4A68, 0x1A044A68 */
	SENINF5_REG_CSI2_DETECT_DBG0                    SENINF5_CSI2_DETECT_DBG0;                        /* 4A6C, 0x1A044A6C */
	SENINF5_REG_CSI2_DETECT_DBG1                    SENINF5_CSI2_DETECT_DBG1;                        /* 4A70, 0x1A044A70 */
	SENINF5_REG_CSI2_RESYNC_MERGE_CTL               SENINF5_CSI2_RESYNC_MERGE_CTL;                   /* 4A74, 0x1A044A74 */
	SENINF5_REG_CSI2_CTRL_TRIO_MUX                  SENINF5_CSI2_CTRL_TRIO_MUX;                      /* 4A78, 0x1A044A78 */
	SENINF5_REG_CSI2_CTRL_TRIO_CON                  SENINF5_CSI2_CTRL_TRIO_CON;                      /* 4A7C, 0x1A044A7C */
	SENINF5_REG_FIX_ADDR_CPHY0_DBG                  SENINF5_FIX_ADDR_CPHY0_DBG;                      /* 4A80, 0x1A044A80 */
	SENINF5_REG_FIX_ADDR_CPHY1_DBG                  SENINF5_FIX_ADDR_CPHY1_DBG;                      /* 4A84, 0x1A044A84 */
	SENINF5_REG_FIX_ADDR_CPHY2_DBG                  SENINF5_FIX_ADDR_CPHY2_DBG;                      /* 4A88, 0x1A044A88 */
	SENINF5_REG_FIX_ADDR_DBG                        SENINF5_FIX_ADDR_DBG;                            /* 4A8C, 0x1A044A8C */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG0        SENINF5_WIRE_STATE_DECODE_CPHY0_DBG0;            /* 4A90, 0x1A044A90 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY0_DBG1        SENINF5_WIRE_STATE_DECODE_CPHY0_DBG1;            /* 4A94, 0x1A044A94 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG0        SENINF5_WIRE_STATE_DECODE_CPHY1_DBG0;            /* 4A98, 0x1A044A98 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY1_DBG1        SENINF5_WIRE_STATE_DECODE_CPHY1_DBG1;            /* 4A9C, 0x1A044A9C */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG0        SENINF5_WIRE_STATE_DECODE_CPHY2_DBG0;            /* 4AA0, 0x1A044AA0 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY2_DBG1        SENINF5_WIRE_STATE_DECODE_CPHY2_DBG1;            /* 4AA4, 0x1A044AA4 */
	SENINF5_REG_SYNC_RESYNC_CTL                     SENINF5_SYNC_RESYNC_CTL;                         /* 4AA8, 0x1A044AA8 */
	SENINF5_REG_POST_DETECT_CTL                     SENINF5_POST_DETECT_CTL;                         /* 4AAC, 0x1A044AAC */
	SENINF5_REG_WIRE_STATE_DECODE_CONFIG            SENINF5_WIRE_STATE_DECODE_CONFIG;                /* 4AB0, 0x1A044AB0 */
	SENINF5_REG_CSI2_CPHY_LNRD_FSM                  SENINF5_CSI2_CPHY_LNRD_FSM;                      /* 4AB4, 0x1A044AB4 */
	SENINF5_REG_FIX_ADDR_CPHY0_DBG0                 SENINF5_FIX_ADDR_CPHY0_DBG0;                     /* 4AB8, 0x1A044AB8 */
	SENINF5_REG_FIX_ADDR_CPHY0_DBG1                 SENINF5_FIX_ADDR_CPHY0_DBG1;                     /* 4ABC, 0x1A044ABC */
	SENINF5_REG_FIX_ADDR_CPHY0_DBG2                 SENINF5_FIX_ADDR_CPHY0_DBG2;                     /* 4AC0, 0x1A044AC0 */
	SENINF5_REG_FIX_ADDR_CPHY1_DBG0                 SENINF5_FIX_ADDR_CPHY1_DBG0;                     /* 4AC4, 0x1A044AC4 */
	SENINF5_REG_FIX_ADDR_CPHY1_DBG1                 SENINF5_FIX_ADDR_CPHY1_DBG1;                     /* 4AC8, 0x1A044AC8 */
	SENINF5_REG_FIX_ADDR_CPHY1_DBG2                 SENINF5_FIX_ADDR_CPHY1_DBG2;                     /* 4ACC, 0x1A044ACC */
	SENINF5_REG_FIX_ADDR_CPHY2_DBG0                 SENINF5_FIX_ADDR_CPHY2_DBG0;                     /* 4AD0, 0x1A044AD0 */
	SENINF5_REG_FIX_ADDR_CPHY2_DBG1                 SENINF5_FIX_ADDR_CPHY2_DBG1;                     /* 4AD4, 0x1A044AD4 */
	SENINF5_REG_FIX_ADDR_CPHY2_DBG2                 SENINF5_FIX_ADDR_CPHY2_DBG2;                     /* 4AD8, 0x1A044AD8 */
	SENINF5_REG_FIX_ADDR_DBG0                       SENINF5_FIX_ADDR_DBG0;                           /* 4ADC, 0x1A044ADC */
	SENINF5_REG_FIX_ADDR_DBG1                       SENINF5_FIX_ADDR_DBG1;                           /* 4AE0, 0x1A044AE0 */
	SENINF5_REG_FIX_ADDR_DBG2                       SENINF5_FIX_ADDR_DBG2;                           /* 4AE4, 0x1A044AE4 */
	SENINF5_REG_CSI2_MODE                           SENINF5_CSI2_MODE;                               /* 4AE8, 0x1A044AE8 */
	UINT32                                          rsv_4AEC;                                        /* 4AEC, 0x1A044AEC */
	SENINF5_REG_CSI2_DI_EXT                         SENINF5_CSI2_DI_EXT;                             /* 4AF0, 0x1A044AF0 */
	SENINF5_REG_CSI2_DI_CTRL_EXT                    SENINF5_CSI2_DI_CTRL_EXT;                        /* 4AF4, 0x1A044AF4 */
	SENINF5_REG_CSI2_CPHY_LOOPBACK                  SENINF5_CSI2_CPHY_LOOPBACK;                      /* 4AF8, 0x1A044AF8 */
	UINT32                                          rsv_4AFC;                                        /* 4AFC, 0x1A044AFC */
	SENINF5_REG_CSI2_PROGSEQ_0                      SENINF5_CSI2_PROGSEQ_0;                          /* 4B00, 0x1A044B00 */
	SENINF5_REG_CSI2_PROGSEQ_1                      SENINF5_CSI2_PROGSEQ_1;                          /* 4B04, 0x1A044B04 */
	UINT32                                          rsv_4B08[2];                                     /* 4B08..4B0F, 0x1A044B08..1A044B0F */
	SENINF5_REG_CSI2_INT_EN_EXT                     SENINF5_CSI2_INT_EN_EXT;                         /* 4B10, 0x1A044B10 */
	SENINF5_REG_CSI2_INT_STATUS_EXT                 SENINF5_CSI2_INT_STATUS_EXT;                     /* 4B14, 0x1A044B14 */
	SENINF5_REG_CSI2_CPHY_FIX_POINT_RST             SENINF5_CSI2_CPHY_FIX_POINT_RST;                 /* 4B18, 0x1A044B18 */
	SENINF5_REG_CSI2_RLR3_CON0                      SENINF5_CSI2_RLR3_CON0;                          /* 4B1C, 0x1A044B1C */
	SENINF5_REG_CSI2_DPHY_SYNC                      SENINF5_CSI2_DPHY_SYNC;                          /* 4B20, 0x1A044B20 */
	SENINF5_REG_CSI2_DESKEW_SYNC                    SENINF5_CSI2_DESKEW_SYNC;                        /* 4B24, 0x1A044B24 */
	SENINF5_REG_CSI2_DETECT_DBG2                    SENINF5_CSI2_DETECT_DBG2;                        /* 4B28, 0x1A044B28 */
	UINT32                                          rsv_4B2C;                                        /* 4B2C, 0x1A044B2C */
	SENINF5_REG_FIX_ADDR_CPHY3_DBG0                 SENINF5_FIX_ADDR_CPHY3_DBG0;                     /* 4B30, 0x1A044B30 */
	SENINF5_REG_FIX_ADDR_CPHY3_DBG1                 SENINF5_FIX_ADDR_CPHY3_DBG1;                     /* 4B34, 0x1A044B34 */
	SENINF5_REG_FIX_ADDR_CPHY3_DBG2                 SENINF5_FIX_ADDR_CPHY3_DBG2;                     /* 4B38, 0x1A044B38 */
	SENINF5_REG_CSI2_DI_EXT_2                       SENINF5_CSI2_DI_EXT_2;                           /* 4B3C, 0x1A044B3C */
	SENINF5_REG_CSI2_DI_CTRL_EXT_2                  SENINF5_CSI2_DI_CTRL_EXT_2;                      /* 4B40, 0x1A044B40 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG0        SENINF5_WIRE_STATE_DECODE_CPHY3_DBG0;            /* 4B44, 0x1A044B44 */
	SENINF5_REG_WIRE_STATE_DECODE_CPHY3_DBG1        SENINF5_WIRE_STATE_DECODE_CPHY3_DBG1;            /* 4B48, 0x1A044B48 */
	UINT32                                          rsv_4B4C[109];                                   /* 4B4C..4CFF, 0x1A044B4C..1A044CFF */
	SENINF5_REG_MUX_CTRL                            SENINF5_MUX_CTRL;                                /* 4D00, 0x1A044D00 */
	SENINF5_REG_MUX_INTEN                           SENINF5_MUX_INTEN;                               /* 4D04, 0x1A044D04 */
	SENINF5_REG_MUX_INTSTA                          SENINF5_MUX_INTSTA;                              /* 4D08, 0x1A044D08 */
	SENINF5_REG_MUX_SIZE                            SENINF5_MUX_SIZE;                                /* 4D0C, 0x1A044D0C */
	SENINF5_REG_MUX_DEBUG_1                         SENINF5_MUX_DEBUG_1;                             /* 4D10, 0x1A044D10 */
	SENINF5_REG_MUX_DEBUG_2                         SENINF5_MUX_DEBUG_2;                             /* 4D14, 0x1A044D14 */
	SENINF5_REG_MUX_DEBUG_3                         SENINF5_MUX_DEBUG_3;                             /* 4D18, 0x1A044D18 */
	SENINF5_REG_MUX_DEBUG_4                         SENINF5_MUX_DEBUG_4;                             /* 4D1C, 0x1A044D1C */
	SENINF5_REG_MUX_DEBUG_5                         SENINF5_MUX_DEBUG_5;                             /* 4D20, 0x1A044D20 */
	SENINF5_REG_MUX_DEBUG_6                         SENINF5_MUX_DEBUG_6;                             /* 4D24, 0x1A044D24 */
	SENINF5_REG_MUX_DEBUG_7                         SENINF5_MUX_DEBUG_7;                             /* 4D28, 0x1A044D28 */
	SENINF5_REG_MUX_SPARE                           SENINF5_MUX_SPARE;                               /* 4D2C, 0x1A044D2C */
	SENINF5_REG_MUX_DATA                            SENINF5_MUX_DATA;                                /* 4D30, 0x1A044D30 */
	SENINF5_REG_MUX_DATA_CNT                        SENINF5_MUX_DATA_CNT;                            /* 4D34, 0x1A044D34 */
	SENINF5_REG_MUX_CROP                            SENINF5_MUX_CROP;                                /* 4D38, 0x1A044D38 */
	SENINF5_REG_MUX_CTRL_EXT                        SENINF5_MUX_CTRL_EXT;                            /* 4D3C, 0x1A044D3C */
	UINT32                                          rsv_4D40[1008];                                  /* 4D40..5CFF, 0x1A044D40..1A045CFF */
	SENINF6_REG_MUX_CTRL                            SENINF6_MUX_CTRL;                                /* 5D00, 0x1A045D00 */
	SENINF6_REG_MUX_INTEN                           SENINF6_MUX_INTEN;                               /* 5D04, 0x1A045D04 */
	SENINF6_REG_MUX_INTSTA                          SENINF6_MUX_INTSTA;                              /* 5D08, 0x1A045D08 */
	SENINF6_REG_MUX_SIZE                            SENINF6_MUX_SIZE;                                /* 5D0C, 0x1A045D0C */
	SENINF6_REG_MUX_DEBUG_1                         SENINF6_MUX_DEBUG_1;                             /* 5D10, 0x1A045D10 */
	SENINF6_REG_MUX_DEBUG_2                         SENINF6_MUX_DEBUG_2;                             /* 5D14, 0x1A045D14 */
	SENINF6_REG_MUX_DEBUG_3                         SENINF6_MUX_DEBUG_3;                             /* 5D18, 0x1A045D18 */
	SENINF6_REG_MUX_DEBUG_4                         SENINF6_MUX_DEBUG_4;                             /* 5D1C, 0x1A045D1C */
	SENINF6_REG_MUX_DEBUG_5                         SENINF6_MUX_DEBUG_5;                             /* 5D20, 0x1A045D20 */
	SENINF6_REG_MUX_DEBUG_6                         SENINF6_MUX_DEBUG_6;                             /* 5D24, 0x1A045D24 */
	SENINF6_REG_MUX_DEBUG_7                         SENINF6_MUX_DEBUG_7;                             /* 5D28, 0x1A045D28 */
	SENINF6_REG_MUX_SPARE                           SENINF6_MUX_SPARE;                               /* 5D2C, 0x1A045D2C */
	SENINF6_REG_MUX_DATA                            SENINF6_MUX_DATA;                                /* 5D30, 0x1A045D30 */
	SENINF6_REG_MUX_DATA_CNT                        SENINF6_MUX_DATA_CNT;                            /* 5D34, 0x1A045D34 */
	SENINF6_REG_MUX_CROP                            SENINF6_MUX_CROP;                                /* 5D38, 0x1A045D38 */
	SENINF6_REG_MUX_CTRL_EXT                        SENINF6_MUX_CTRL_EXT;                            /* 5D3C, 0x1A045D3C */
	UINT32                                          rsv_5D40[1008];                                  /* 5D40..6CFF, 0x1A045D40..1A046CFF */
	SENINF7_REG_MUX_CTRL                            SENINF7_MUX_CTRL;                                /* 6D00, 0x1A046D00 */
	SENINF7_REG_MUX_INTEN                           SENINF7_MUX_INTEN;                               /* 6D04, 0x1A046D04 */
	SENINF7_REG_MUX_INTSTA                          SENINF7_MUX_INTSTA;                              /* 6D08, 0x1A046D08 */
	SENINF7_REG_MUX_SIZE                            SENINF7_MUX_SIZE;                                /* 6D0C, 0x1A046D0C */
	SENINF7_REG_MUX_DEBUG_1                         SENINF7_MUX_DEBUG_1;                             /* 6D10, 0x1A046D10 */
	SENINF7_REG_MUX_DEBUG_2                         SENINF7_MUX_DEBUG_2;                             /* 6D14, 0x1A046D14 */
	SENINF7_REG_MUX_DEBUG_3                         SENINF7_MUX_DEBUG_3;                             /* 6D18, 0x1A046D18 */
	SENINF7_REG_MUX_DEBUG_4                         SENINF7_MUX_DEBUG_4;                             /* 6D1C, 0x1A046D1C */
	SENINF7_REG_MUX_DEBUG_5                         SENINF7_MUX_DEBUG_5;                             /* 6D20, 0x1A046D20 */
	SENINF7_REG_MUX_DEBUG_6                         SENINF7_MUX_DEBUG_6;                             /* 6D24, 0x1A046D24 */
	SENINF7_REG_MUX_DEBUG_7                         SENINF7_MUX_DEBUG_7;                             /* 6D28, 0x1A046D28 */
	SENINF7_REG_MUX_SPARE                           SENINF7_MUX_SPARE;                               /* 6D2C, 0x1A046D2C */
	SENINF7_REG_MUX_DATA                            SENINF7_MUX_DATA;                                /* 6D30, 0x1A046D30 */
	SENINF7_REG_MUX_DATA_CNT                        SENINF7_MUX_DATA_CNT;                            /* 6D34, 0x1A046D34 */
	SENINF7_REG_MUX_CROP                            SENINF7_MUX_CROP;                                /* 6D38, 0x1A046D38 */
	SENINF7_REG_MUX_CTRL_EXT                        SENINF7_MUX_CTRL_EXT;                            /* 6D3C, 0x1A046D3C */
	UINT32                                          rsv_6D40[1008];                                  /* 6D40..7CFF, 0x1A046D40..1A047CFF */
	SENINF8_REG_MUX_CTRL                            SENINF8_MUX_CTRL;                                /* 7D00, 0x1A047D00 */
	SENINF8_REG_MUX_INTEN                           SENINF8_MUX_INTEN;                               /* 7D04, 0x1A047D04 */
	SENINF8_REG_MUX_INTSTA                          SENINF8_MUX_INTSTA;                              /* 7D08, 0x1A047D08 */
	SENINF8_REG_MUX_SIZE                            SENINF8_MUX_SIZE;                                /* 7D0C, 0x1A047D0C */
	SENINF8_REG_MUX_DEBUG_1                         SENINF8_MUX_DEBUG_1;                             /* 7D10, 0x1A047D10 */
	SENINF8_REG_MUX_DEBUG_2                         SENINF8_MUX_DEBUG_2;                             /* 7D14, 0x1A047D14 */
	SENINF8_REG_MUX_DEBUG_3                         SENINF8_MUX_DEBUG_3;                             /* 7D18, 0x1A047D18 */
	SENINF8_REG_MUX_DEBUG_4                         SENINF8_MUX_DEBUG_4;                             /* 7D1C, 0x1A047D1C */
	SENINF8_REG_MUX_DEBUG_5                         SENINF8_MUX_DEBUG_5;                             /* 7D20, 0x1A047D20 */
	SENINF8_REG_MUX_DEBUG_6                         SENINF8_MUX_DEBUG_6;                             /* 7D24, 0x1A047D24 */
	SENINF8_REG_MUX_DEBUG_7                         SENINF8_MUX_DEBUG_7;                             /* 7D28, 0x1A047D28 */
	SENINF8_REG_MUX_SPARE                           SENINF8_MUX_SPARE;                               /* 7D2C, 0x1A047D2C */
	SENINF8_REG_MUX_DATA                            SENINF8_MUX_DATA;                                /* 7D30, 0x1A047D30 */
	SENINF8_REG_MUX_DATA_CNT                        SENINF8_MUX_DATA_CNT;                            /* 7D34, 0x1A047D34 */
	SENINF8_REG_MUX_CROP                            SENINF8_MUX_CROP;                                /* 7D38, 0x1A047D38 */
	SENINF8_REG_MUX_CTRL_EXT                        SENINF8_MUX_CTRL_EXT;                            /* 7D3C, 0x1A047D3C */
	UINT32                                          rsv_7D40[20];                                    /* 7D40..7D8F, 1A047D40..1A047D8F */
}seninf_reg_t;



typedef volatile struct _seninf_mux_reg_t_base_  /* 0x1A040d00..0x1A040d3c */
{
	SENINF1_REG_MUX_CTRL                            SENINF1_MUX_CTRL;                                /* 0D00, 0x1A040D00 */
	SENINF1_REG_MUX_INTEN                           SENINF1_MUX_INTEN;                               /* 0D04, 0x1A040D04 */
	SENINF1_REG_MUX_INTSTA                          SENINF1_MUX_INTSTA;                              /* 0D08, 0x1A040D08 */
	SENINF1_REG_MUX_SIZE                            SENINF1_MUX_SIZE;                                /* 0D0C, 0x1A040D0C */
	SENINF1_REG_MUX_DEBUG_1                         SENINF1_MUX_DEBUG_1;                             /* 0D10, 0x1A040D10 */
	SENINF1_REG_MUX_DEBUG_2                         SENINF1_MUX_DEBUG_2;                             /* 0D14, 0x1A040D14 */
	SENINF1_REG_MUX_DEBUG_3                         SENINF1_MUX_DEBUG_3;                             /* 0D18, 0x1A040D18 */
	SENINF1_REG_MUX_DEBUG_4                         SENINF1_MUX_DEBUG_4;                             /* 0D1C, 0x1A040D1C */
	SENINF1_REG_MUX_DEBUG_5                         SENINF1_MUX_DEBUG_5;                             /* 0D20, 0x1A040D20 */
	SENINF1_REG_MUX_DEBUG_6                         SENINF1_MUX_DEBUG_6;                             /* 0D24, 0x1A040D24 */
	SENINF1_REG_MUX_DEBUG_7                         SENINF1_MUX_DEBUG_7;                             /* 0D28, 0x1A040D28 */
	SENINF1_REG_MUX_SPARE                           SENINF1_MUX_SPARE;                               /* 0D2C, 0x1A040D2C */
	SENINF1_REG_MUX_DATA                            SENINF1_MUX_DATA;                                /* 0D30, 0x1A040D30 */
	SENINF1_REG_MUX_DATA_CNT                        SENINF1_MUX_DATA_CNT;                            /* 0D34, 0x1A040D34 */
	SENINF1_REG_MUX_CROP                            SENINF1_MUX_CROP;                                /* 0D38, 0x1A040D38 */
	SENINF1_REG_MUX_CTRL_EXT                        SENINF1_MUX_CTRL_EXT;                            /* 0D3C, 0x1A040D3C */
}seninf_mux_reg_t_base;


//typedef volatile struct _dpe_reg_t_ /* 0x15028800..0x1502893F */
//typedef volatile struct _fdvt_reg_t_    /* 0x1502B200..0x1502B33F */
//typedef volatile struct _cam_reg_t_ /* 0x1A004000..0x1A004DFF */
//typedef volatile struct _camsv_reg_t_   /* 0x1A051010..0x1A051C6F */
//typedef volatile struct _dip_x_reg_t_   /* 0x15022000..0x150264FF */

#endif // _SENINF_REG_H_
