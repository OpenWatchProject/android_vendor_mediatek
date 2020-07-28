/**
 * @file    mtk_ocp_api.c
 * @brief   Driver for Over Current Protect
 *
 */
#include <delay_timer.h> /* for udelay */
#include "mtk_ocp_api.h"

#define ocp_use_v2_for_mp0_mp1()	(GetChipVersion() == 1)

struct ocp_reg_setting
{
	unsigned int addr;
	unsigned int value;
};

static const struct ocp_reg_setting big_setting_table[] =
{
	{MP2_OCPAPB03, 0x0000400e},
	{MP2_OCPAPB04, 0x0000ffff},
	{MP2_OCPAPB05, 0x03e80400},
	{MP2_OCPAPB06, 0x00000007},
	{MP2_OCPAPB07, 0x00000000},
	{MP2_OCPAPB08, 0x00000000},
	{MP2_OCPAPB09, 0x0AC32600},
	{MP2_OCPAPB0A, 0x388936B6},
	{MP2_OCPAPB0B, 0x260032EE},
	{MP2_OCPAPB0C, 0x00002600},
	{MP2_OCPAPB0D, 0x0001339E},
	{MP2_OCPAPB0E, 0x000132F4},
	{MP2_OCPAPB0F, 0x000932EE},
	{MP2_OCPAPB10, 0x0012348A},
	{MP2_OCPAPB11, 0x0013348A},
	{MP2_OCPAPB12, 0x00002600},
	{MP2_OCPAPB13, 0x00002600},
	{MP2_OCPAPB14, 0x00002600},
	{MP2_OCPAPB15, 0x00002600},
	{MP2_OCPAPB16, 0x00042600},
};

static const struct ocp_reg_setting little_mp0_setting_table_v2[] =
{
	{MP0_OCPCPUPOST_CTRL0, 0x0D8C54ED},
	{MP0_OCPCPUPOST_CTRL1, 0x13A18801},
	{MP0_OCPCPUPOST_CTRL2, 0x01349104},
	{MP0_OCPCPUPOST_CTRL3, 0x21CEC322},
	{MP0_OCPCPUPOST_CTRL4, 0x03160542},
	{MP0_OCPCPUPOST_CTRL5, 0x000000C5},
	{MP0_OCPCPUPOST_CTRL6, 0x00000000},
	{MP0_OCPCPUPOST_CTRL7, 0x00000000},
	{MP0_OCPNCPUPOST_CTRL, 0x01249249},
	{MP0_OCPAPBCFG00, 0x00005000},
	{MP0_OCPAPBCFG11, 0x07800000},
	{MP0_OCPAPBCFG12, 0x133000F5},
	{MP0_OCPAPBCFG13, 0x1193208D},
	{MP0_OCPAPBCFG14, 0x00D7005B},
	{MP0_OCPAPBCFG15, 0x1D451E54},
	{MP0_OCPAPBCFG16, 0x72E131DD},
	{MP0_OCPAPBCFG17, 0x4B5E4D1B},
	{MP0_OCPAPBCFG18, 0x5FFA439E},
	{MP0_OCPAPBCFG19, 0x2FDE3B4B},
	{MP0_OCPAPBCFG20, 0x6F1B2DD0},
	{MP0_OCPAPBCFG21, 0x3E83325B},
	{MP0_OCPAPBCFG22, 0x294D3A8B},
	{MP0_OCPAPBCFG23, 0x3CDA2D9C},
	{MP0_OCPAPBCFG24, 0x31873CB5},
	{MP0_OCPAPBCFG25, 0xFFFF2B9E},
	{MP0_OCPAPBCFG26, 0xE1DA1FFF},
	{MP0_OCPAPBCFG27, 0x204021DB},
	{MP0_OCPAPBCFG28, 0x89A3E202},
	{MP0_OCPAPBCFG29, 0x00000000},
	{MP0_OCPAPBCFG30, 0x00000000},
	{MP0_OCPAPBCFG31, 0x00000000},
	{MP0_OCPAPBCFG32, 0x00000000},
	{MP0_OCPAPBCFG33, 0x00000000},
	{MP0_OCPAPBCFG34, 0x00000000},

};

static const struct ocp_reg_setting little_mp1_setting_table_v2[] =
{
	{MP1_OCPCPUPOST_CTRL0, 0x00065324},
	{MP1_OCPCPUPOST_CTRL1, 0x0812F089},
	{MP1_OCPCPUPOST_CTRL2, 0x2804810C},
	{MP1_OCPCPUPOST_CTRL3, 0x00200040},
	{MP1_OCPCPUPOST_CTRL4, 0x09000201},
	{MP1_OCPCPUPOST_CTRL5, 0x00248229},
	{MP1_OCPCPUPOST_CTRL6, 0x22021148},
	{MP1_OCPCPUPOST_CTRL7, 0x00000008},
	{MP1_OCPNCPUPOST_CTRL, 0x01249249},
	{MP1_OCPAPBCFG00, 0x00005000},
	{MP1_OCPAPBCFG11, 0x07800000},
	{MP1_OCPAPBCFG12, 0x133000F5},
	{MP1_OCPAPBCFG13, 0x193122B2},
	{MP1_OCPAPBCFG14, 0x021E0000},
	{MP1_OCPAPBCFG15, 0x1F2E251A},
	{MP1_OCPAPBCFG16, 0x1DC717D3},
	{MP1_OCPAPBCFG17, 0x1AFA16C3},
	{MP1_OCPAPBCFG18, 0x2A53215B},
	{MP1_OCPAPBCFG19, 0x15C224D7},
	{MP1_OCPAPBCFG20, 0x1ACA1EBB},
	{MP1_OCPAPBCFG21, 0x19CF1303},
	{MP1_OCPAPBCFG22, 0x168C144A},
	{MP1_OCPAPBCFG23, 0x171B1870},
	{MP1_OCPAPBCFG24, 0x1C441766},
	{MP1_OCPAPBCFG25, 0x15B816F4},
	{MP1_OCPAPBCFG26, 0x2261290A},
	{MP1_OCPAPBCFG27, 0x1BD81BB0},
	{MP1_OCPAPBCFG28, 0x3BFFFFFF},
	{MP1_OCPAPBCFG29, 0x087CB439},
	{MP1_OCPAPBCFG30, 0x0C805051},
	{MP1_OCPAPBCFG31, 0x0C0C4885},
	{MP1_OCPAPBCFG32, 0x00000110},
	{MP1_OCPAPBCFG33, 0x00000000},
	{MP1_OCPAPBCFG34, 0x00000000},

};

/* OCPv3 */
static const struct ocp_reg_setting little_mp0_setting_table_v3[] =
{
	{MP0_OCP_GENERAL_CTRL, 0x00038004},
	{MP0_OCPCPUPOST_CTRL0, 0x0C249065},
	{MP0_OCPCPUPOST_CTRL1, 0x0BA0924C},
	{MP0_OCPCPUPOST_CTRL2, 0x21300365},
	{MP0_OCPCPUPOST_CTRL3, 0x09324369},
	{MP0_OCPCPUPOST_CTRL4, 0x00100804},
	{MP0_OCPCPUPOST_CTRL5, 0x00000000},
	{MP0_OCPCPUPOST_CTRL6, 0x00000000},
	{MP0_OCPCPUPOST_CTRL7, 0x00000000},
	{MP0_OCPNCPUPOST_CTRL, 0x00000000},
	{MP0_OCPAPB09, 0x00F52600},
	{MP0_OCPAPB0A, 0x00000000},
	{MP0_OCPAPB0B, 0x26002600},
	{MP0_OCPAPB0C, 0x00012600},
	{MP0_OCPAPB0D, 0x00012600},
	{MP0_OCPAPB0E, 0x00012600},
	{MP0_OCPAPB0F, 0x00012600},
	{MP0_OCPAPB10, 0x00012600},
	{MP0_OCPAPB11, 0x00012600},
	{MP0_OCPAPB12, 0x00012600},
	{MP0_OCPAPB13, 0x00012600},
	{MP0_OCPAPB14, 0x00012600},
	{MP0_OCPAPB15, 0x00013097},
	{MP0_OCPAPB16, 0x000131E5},
	{MP0_OCPAPB17, 0x0001309E},
	{MP0_OCPAPB18, 0x00012E92},
	{MP0_OCPAPB19, 0x00012F84},
	{MP0_OCPAPB1A, 0x00013099},
	{MP0_OCPAPB1B, 0x00012FC9},
	{MP0_OCPAPB1C, 0x00012CA7},
	{MP0_OCPAPB1D, 0x000133C7},
	{MP0_OCPAPB1E, 0x0001328D},
	{MP0_OCPAPB1F, 0x00013280},
	{MP0_OCPAPB20, 0x00013285},
	{MP0_OCPAPB21, 0x00000000},
	{MP0_OCPAPB22, 0x00000000},
	{MP0_OCPAPB23, 0x00000000},
	{MP0_OCPAPB24, 0x00000000},
	{MP0_OCPAPB25, 0x00000000},
};

static const struct ocp_reg_setting little_mp1_setting_table_v3[] =
{
	{MP1_OCP_GENERAL_CTRL, 0x00038004},
	{MP1_OCPCPUPOST_CTRL0, 0x00051015},
	{MP1_OCPCPUPOST_CTRL1, 0x28060021},
	{MP1_OCPCPUPOST_CTRL2, 0x08048061},
	{MP1_OCPCPUPOST_CTRL3, 0x092C0000},
	{MP1_OCPCPUPOST_CTRL4, 0x09000201},
	{MP1_OCPCPUPOST_CTRL5, 0x00849300},
	{MP1_OCPCPUPOST_CTRL6, 0x00008040},
	{MP1_OCPCPUPOST_CTRL7, 0x0000000A},
	{MP1_OCPNCPUPOST_CTRL, 0x01249249},
	{MP1_OCPAPB09, 0x00F52600},
	{MP1_OCPAPB0A, 0x00000000},
	{MP1_OCPAPB0B, 0x26002600},
	{MP1_OCPAPB0C, 0x000131FA},
	{MP1_OCPAPB0D, 0x00012600},
	{MP1_OCPAPB0E, 0x000134CF},
	{MP1_OCPAPB0F, 0x000131F2},
	{MP1_OCPAPB10, 0x0001318D},
	{MP1_OCPAPB11, 0x000133B2},
	{MP1_OCPAPB12, 0x0001318C},
	{MP1_OCPAPB13, 0x00013384},
	{MP1_OCPAPB14, 0x000130CE},
	{MP1_OCPAPB15, 0x000131CB},
	{MP1_OCPAPB16, 0x000134FB},
	{MP1_OCPAPB17, 0x00012FAF},
	{MP1_OCPAPB18, 0x00013284},
	{MP1_OCPAPB19, 0x00012F9A},
	{MP1_OCPAPB1A, 0x000132FA},
	{MP1_OCPAPB1B, 0x000131C9},
	{MP1_OCPAPB1C, 0x00013388},
	{MP1_OCPAPB1D, 0x000133B4},
	{MP1_OCPAPB1E, 0x000134CB},
	{MP1_OCPAPB1F, 0x000134F0},
	{MP1_OCPAPB20, 0x000131F7},
	{MP1_OCPAPB21, 0x000133CE},
	{MP1_OCPAPB22, 0x00013191},
	{MP1_OCPAPB23, 0x000131DC},
	{MP1_OCPAPB24, 0x000133BE},
	{MP1_OCPAPB25, 0x000131D3},
};

static unsigned int GetChipVersion(void)
{
	return 1;
}

/* Big CPU */
static int BigOCPConfigMode(enum ocp_mode ConfigMode)
{
	/* Set OCPCFG bits {VoltBypassEn, LkgShare, LkgBypassEn, MHzBypassEn} = ConfigMode */
	ocp_write_field(MP2_OCPAPB03, 15:15, GET_BITS_VAL(3:3, ConfigMode));
	ocp_write_field(MP2_OCPAPB03, 14:14, GET_BITS_VAL(2:2, ConfigMode));
	ocp_write_field(MP2_OCPAPB03, 13:13, GET_BITS_VAL(1:1, ConfigMode));
	ocp_write_field(MP2_OCPAPB03, 12:12, GET_BITS_VAL(0:0, ConfigMode));

	ocp_dbg("Big ConfigMode = 0x%x\n", ConfigMode);

	return 0;
}

static int BigOCPConfig(unsigned int MinClkPctSel, enum ocp_unit OCPUnits, enum ocp_mode ConfigMode)
{
	int LkgTrim;
	int MinClkPct;
	int i;

	if (MinClkPctSel < OCP_CLK_PCT_MIN_V3) {
		ocp_err("MinClkPctSel(%d) < %d!\n", MinClkPctSel, OCP_CLK_PCT_MIN_V3);
		return -1;
	}

	/* 1. Write all initial values of OCPCFG. */
	for (i = 0; i < sizeof(big_setting_table)/sizeof(struct ocp_reg_setting); i++)
		ocp_write(big_setting_table[i].addr, big_setting_table[i].value);

	/*
	2. Read LkgMonTRIM[3:0] values from eFuse for leakage monitor
	3. If 4-bit  value is zero, use  4’h7
	4. Write Lkg TRIM values
	*/
#if 0
	LkgTrim = ocp_read_field(PTP3_OD, 19:16);
	LkgTrim = (!LkgTrim) ? 0x7 : LkgTrim;
	ocp_write(MP2_OCPAPB06, LkgTrim);
#else
	/* no efuse */
	LkgTrim = 0x7;
	ocp_write(MP2_OCPAPB06, LkgTrim);
#endif

	/* 5. Set OCP config mode */
	BigOCPConfigMode(ConfigMode);

	/* 6. Write MinClkPctSel & OCPUnits to OCPCFG */
	MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
	ocp_write_field(MP2_OCPAPB03, 7:4, MinClkPct);
	ocp_write_field(MP2_OCPAPB03, 3:3, OCPUnits & 0x1);

	/* 7. OCP sample rate */
	ocp_write_field(MP2_OCPAPB03, 2:1, 0x3);

	ocp_dbg("B CFG done, MinClkPct = %d, OCPUnits = %d, ConfigMode = %d\n",
			MinClkPct, OCPUnits, ConfigMode);

	return 0;
}

static int BigOCPTarget(unsigned int OCTgt)
{
	OCTgt = OCTgt & BITMASK(15:0);
	ocp_write_field(MP2_OCPAPB04, 15:0, OCTgt);

	ocp_dbg("Big OCTgt = 0x%x\n", OCTgt);

	return 0;
}

static int BigOCPFreq(unsigned int FreqMHz)
{
	FreqMHz = FreqMHz & BITMASK(11:0);
	ocp_write_field(MP2_OCPAPB05, 27:16, FreqMHz);

	ocp_dbg("Big FreqMHz = 0x%x\n", FreqMHz);

	return 0;
}

static int BigOCPVoltage(unsigned int Voltage)
{
	int VoltVal;

	/* Convert Voltage to to UQ1.10 */
	VoltVal = ((Voltage << 10) / 1000) & BITMASK(10:0);
	ocp_write_field(MP2_OCPAPB05, 10:0, VoltVal);

	ocp_dbg("Big Voltage = 0x%x\n", VoltVal);

	return 0;
}

static int BigOCPAvg(unsigned int AvgWindowSel)
{
	ocp_write_field(MP2_OCPAPB03, 11:8, AvgWindowSel & BITMASK(3:0));
	ocp_dbg("Big AvgWindowSel = %d\n", AvgWindowSel);

	return 0;
}

static int BigOCPLeakage(unsigned int Leakage)
{
	Leakage = Leakage & BITMASK(15:0);
	ocp_write_field(MP2_OCPAPB04, 31:16, Leakage);

	ocp_dbg("Big Leakage = 0x%x\n", Leakage);

	return 0;
}

static int BigOCPEnDis(unsigned int EnDis)
{
	if (EnDis) {
		/* Write OCPIEn = 1 */
		ocp_write_field(MP2_OCPAPB03, 0:0, 1);
	} else {
		/* Write OCPIEn = 0 */
		ocp_write_field(MP2_OCPAPB03, 0:0, 0);
	}

	ocp_dbg("Big OCEn = %d\n", EnDis);

	return 0;
}

static int BigOCPIntLimit(enum ocp_int_select Select, int Limit)
{
	unsigned int Val = 0;

	switch (Select) {
	case IRQ_CLK_PCT_MIN:
		Val = Limit & BITMASK(6:0);
		ocp_write_field(MP2_OCPAPB07, 30:24, Val);
		break;
	case IRQ_WA_MAX:
		Val = Limit & BITMASK(15:0);
		ocp_write_field(MP2_OCPAPB08, 15:0, Val);
 		break;
	case IRQ_WA_MIN:
		Val = Limit & BITMASK(15:0);
		ocp_write_field(MP2_OCPAPB08, 31:16, Val);
		break;
	default:
		ocp_err("Invalid Big Int Limit Select value: %d\n", Select);
		return -1;
	}

 	ocp_dbg("Big Int select = %d, limit = 0x%x\n", Select, Val);

	return 0;
}

static int BigOCPIRQHoldoff(enum ocp_int_select Select, int WindowSel)
{
	switch (Select) {
	case IRQ_CLK_PCT_MIN:
		ocp_write_field(MP2_OCPAPB07, 15:12, WindowSel & BITMASK(3:0));
		break;
	case IRQ_WA_MAX:
		ocp_write_field(MP2_OCPAPB07, 19:16, WindowSel & BITMASK(3:0));
		break;
	case IRQ_WA_MIN:
		ocp_write_field(MP2_OCPAPB07, 23:20, WindowSel & BITMASK(3:0));
		break;
	default:
		ocp_err("Invalid Big Int Limit Select value: %d\n", Select);
		return -1;
	}

	ocp_dbg("Big Int select = %d, WindowSel = 0x%x\n", Select, WindowSel);

	return 0;
}

static int BigOCPIntEnDis(int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 8) | (Value1 << 4) | Value0;

	ocp_write_field(MP2_OCPAPB07, 10:0, Val);

	ocp_dbg("Big Int EnDis = 0x%x\n", Val);

	return 0;
}

static int BigOCPIntClr(int Value2, int Value1, int Value0)
{
	unsigned int Val = (Value2 << 8) | (Value1 << 4) | Value0;

	ocp_write_field(MP2_OCPAPB01, 10:0, Val);

	ocp_dbg("Big Int Clr = 0x%x\n", Val);

	return 0;
}

static int BigOCPValueStatus(enum ocp_value_select Select)
{
	unsigned int Val = 0;

	switch (Select) {
	case CLK_AVG:
		if (ocp_read_field(MP2_OCPAPB00, 23:23) == 0)
			Val = ocp_read_field(MP2_OCPAPB00, 22:16);
		break;
	case WA_AVG:
		Val = ocp_read_field(MP2_OCPAPB00, 15:0);
		break;
	case TOP_RAW_LKG:
		Val = ocp_read_field(MP2_OCPAPB00, 31:24);
		break;
	case CPU0_RAW_LKG:
		Val = ocp_read_field(MP2_OCPAPB00+8, 7:0);
		break;
	case CPU1_RAW_LKG:
		Val = ocp_read_field(MP2_OCPAPB00+8, 15:8);
		break;
	case CPU2_RAW_LKG:
	case CPU3_RAW_LKG:
		Val = 0;
		break;
	default:
		ocp_err("Invalid OCP select value: %d\n", Select);
		return -1;
	}

 	ocp_dbg("Big Value Status = %d, Select = 0x%x\n", Val, Select);

	return Val;
}

/* Little CPU */
static int LittleOCPP2SInit(enum ocp_cluster Cluster)
{
	unsigned int Addr;

	if (Cluster == OCP_LL)
		Addr = MP0_OCP_GENERAL_CTRL;
	else if (Cluster == OCP_L)
		Addr = MP1_OCP_GENERAL_CTRL;
	else {
		ocp_err("Invalid cluster id: %d\n", Cluster);
		return -1;
	}

	/* 1: enable P2S procedure: mpx_ocp_general_ctrl[4]=1 */
	ocp_write_field(Addr, 4:4, 1);
	/* 2:  Wait for P2S finish: Polling mpx_ocp_general_ctrl[31] wait until this bit become 1 */
	while (!ocp_read_field(Addr, 31:31));

	ocp_dbg("%s P2SInit done\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPConfigMode(enum ocp_cluster Cluster, enum ocp_mode ConfigMode)
{
	/* OCPv2 not support */
	if (!ocp_use_v2_for_mp0_mp1()) {
		unsigned int Addr;

		if (Cluster == OCP_LL)
			Addr = MP0_OCPAPB03;
		else if (Cluster == OCP_L)
			Addr = MP1_OCPAPB03;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		/* Set OCPCFG bits {VoltBypassEn, LkgShare, LkgBypassEn, MHzBypassEn} = ConfigMode */
		ocp_write_field(Addr, 15:15, GET_BITS_VAL(3:3, ConfigMode));
		ocp_write_field(Addr, 14:14, GET_BITS_VAL(2:2, ConfigMode));
		ocp_write_field(Addr, 13:13, GET_BITS_VAL(1:1, ConfigMode));
		ocp_write_field(Addr, 12:12, GET_BITS_VAL(0:0, ConfigMode));

		ocp_dbg("%s ConfigMode = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", ConfigMode);
	} else
		ocp_err("%s not support ConfigMode!\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPConfig(enum ocp_cluster Cluster, unsigned int MinClkPctSel,
				enum ocp_unit OCPUnits, enum ocp_mode ConfigMode)
{
	if (ocp_use_v2_for_mp0_mp1()) {
		int TopLkgTrim, Cpu0LkgTrim, Cpu1LkgTrim, Cpu2LkgTrim, Cpu3LkgTrim;
		int MinClkPct;
		int i;

		if (MinClkPctSel < OCP_CLK_PCT_MIN_V2) {
			ocp_err("MinClkPctSel(%d) < %d!\n", MinClkPctSel, OCP_CLK_PCT_MIN_V2);
			return -1;
		}

		if (Cluster == OCP_LL) {
			/* 1. Write all initial values of OCPCFG. */
			for (i = 0; i < sizeof(little_mp0_setting_table_v2)/sizeof(struct ocp_reg_setting); i++)
				ocp_write(little_mp0_setting_table_v2[i].addr, little_mp0_setting_table_v2[i].value);

			/* fire to HW logic */
			LittleOCPP2SInit(Cluster);

			/*
			2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
			3. If any 4-bit  value is all zeros, use 4’h7
			4. Write ALL Lkg TRIM values
			*/
#if 0
			TopLkgTrim = ocp_read_field(PTP3_OD2, 19:16);
			TopLkgTrim = (!TopLkgTrim) ? 0x7 : TopLkgTrim;
			Cpu0LkgTrim = ocp_read_field(PTP3_OD2, 23:20);
			Cpu0LkgTrim = (!Cpu0LkgTrim) ? 0x7 : Cpu0LkgTrim;
			Cpu1LkgTrim = ocp_read_field(PTP3_OD2, 27:24);
			Cpu1LkgTrim = (!Cpu1LkgTrim) ? 0x7 : Cpu1LkgTrim;
			Cpu2LkgTrim = ocp_read_field(PTP3_OD2, 31:28);
			Cpu2LkgTrim = (!Cpu2LkgTrim) ? 0x7 : Cpu2LkgTrim;
			Cpu3LkgTrim = ocp_read_field(PTP3_OD3, 31:28);
			Cpu3LkgTrim = (!Cpu3LkgTrim) ? 0x7 : Cpu3LkgTrim;
			ocp_write((MP0_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
					(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#else
			/* no efuse */
			TopLkgTrim = 0x7;
			Cpu0LkgTrim = 0x7;
			Cpu1LkgTrim = 0x7;
			Cpu2LkgTrim = 0x7;
			Cpu3LkgTrim = 0x7;
			ocp_write((MP0_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
					(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#endif
			/* 5. Write MinClkPctSel & OCPUnits to OCPCFG */
			MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
			ocp_write_field(MP0_OCPAPBCFG05, 10:7, MinClkPct);
			ocp_write_field(MP0_OCPAPBCFG05, 0:0, OCPUnits & 0x1);
		} else if (Cluster == OCP_L) {
			/* 1. Write all initial values of OCPCFG. */
			for (i = 0; i < sizeof(little_mp1_setting_table_v2)/sizeof(struct ocp_reg_setting); i++)
				ocp_write(little_mp1_setting_table_v2[i].addr, little_mp1_setting_table_v2[i].value);

			/* fire to HW logic */
			LittleOCPP2SInit(Cluster);

			/*
			2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
			3. If any 4-bit  value is all zeros, use 4’h7
			4. Write ALL Lkg TRIM values
			*/
#if 0
			TopLkgTrim = ocp_read_field(PTP3_OD1, 19:16);
			TopLkgTrim = (!TopLkgTrim) ? 0x7 : TopLkgTrim;
			Cpu0LkgTrim = ocp_read_field(PTP3_OD1, 23:20);
			Cpu0LkgTrim = (!Cpu0LkgTrim) ? 0x7 : Cpu0LkgTrim;
			Cpu1LkgTrim = ocp_read_field(PTP3_OD1, 27:24);
			Cpu1LkgTrim = (!Cpu1LkgTrim) ? 0x7 : Cpu1LkgTrim;
			Cpu2LkgTrim = ocp_read_field(PTP3_OD1, 31:28);
			Cpu2LkgTrim = (!Cpu2LkgTrim) ? 0x7 : Cpu2LkgTrim;
			Cpu3LkgTrim = ocp_read_field(PTP3_OD3, 23:20);
			Cpu3LkgTrim = (!Cpu3LkgTrim) ? 0x7 : Cpu3LkgTrim;
			ocp_write((MP1_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
					(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));
#else
			/* no efuse */
			TopLkgTrim = 0x7;
			Cpu0LkgTrim = 0x7;
			Cpu1LkgTrim = 0x7;
			Cpu2LkgTrim = 0x7;
			Cpu3LkgTrim = 0x7;
			ocp_write((MP1_OCPAPBCFG04), ((Cpu3LkgTrim << 16) | (Cpu2LkgTrim << 12) |
					(Cpu1LkgTrim << 8) | (Cpu0LkgTrim << 4) | TopLkgTrim));

#endif
			/* 5. Write MinClkPctSel & OCPUnits to OCPCFG */
			MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
			ocp_write_field(MP1_OCPAPBCFG05, 10:7, MinClkPct);
			ocp_write_field(MP1_OCPAPBCFG05, 0:0, OCPUnits & 0x1);
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s CFG done, MinClkPct = %d, OCPUnits = %d\n",
			(Cluster == OCP_LL) ? "LL" : "L", MinClkPct, OCPUnits);
	} else {
		/* OCPv3 */
		int LkgTrim, MinClkPct, i;

		if (MinClkPctSel < OCP_CLK_PCT_MIN_V3) {
			ocp_err("MinClkPctSel(%d) < %d!\n", MinClkPctSel, OCP_CLK_PCT_MIN_V3);
			return -1;
		}

		if (Cluster == OCP_LL) {
			/* 1. Write all initial values of OCPCFG. */
			for (i = 0; i < sizeof(little_mp0_setting_table_v3)/sizeof(struct ocp_reg_setting); i++)
				ocp_write(little_mp0_setting_table_v3[i].addr, little_mp0_setting_table_v3[i].value);

			/* fire to HW logic */
			LittleOCPP2SInit(Cluster);

			/*
			3. Read LkgMonTRIM[3:0] values from eFuse for leakage monitor
			4. If 4-bit  value is zero, use  4’h7
			5. Write Lkg TRIM values
			*/
#if 0
			LkgTrim = ocp_read_field(PTP3_OD, 19:16);
			LkgTrim = (!LkgTrim) ? 0x7 : LkgTrim;
			ocp_write(MP0_OCPAPB06, LkgTrim);
#else
			/* no efuse */
			LkgTrim = 0x7;
			ocp_write(MP0_OCPAPB06, LkgTrim);
#endif

			/* 6. Set OCP config mode */
			LittleOCPConfigMode(Cluster, ConfigMode);

			/* 7. Write MinClkPctSel & OCPUnits to OCPCFG */
			MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
			ocp_write_field(MP0_OCPAPB03, 7:4, MinClkPct);
			ocp_write_field(MP0_OCPAPB03, 3:3, OCPUnits & 0x1);

			/* 8. OCP sample rate */
			ocp_write_field(MP0_OCPAPB03, 2:1, 0x2);
		} else if (Cluster == OCP_L) {
			/* 1. Write all initial values of OCPCFG. */
			for (i = 0; i < sizeof(little_mp1_setting_table_v3)/sizeof(struct ocp_reg_setting); i++)
				ocp_write(little_mp1_setting_table_v3[i].addr, little_mp1_setting_table_v3[i].value);

			/* fire to HW logic */
			LittleOCPP2SInit(Cluster);

			/*
			2. Read LkgMonTRIM[3:0] values from eFuse for each leakage monitor
			3. If any 4-bit  value is all zeros, use 4’h7
			4. Write ALL Lkg TRIM values
			*/
#if 0
			LkgTrim = ocp_read_field(PTP3_OD, 19:16);
			LkgTrim = (!LkgTrim) ? 0x7 : LkgTrim;
			ocp_write(MP1_OCPAPB06, LkgTrim);
#else
			/* no efuse */
			LkgTrim = 0x7;
			ocp_write(MP1_OCPAPB06, LkgTrim);
#endif

			/* 6. Set OCP config mode */
			LittleOCPConfigMode(Cluster, ConfigMode);

			/* 7. Write MinClkPctSel & OCPUnits to OCPCFG */
			MinClkPct = ((MinClkPctSel / 625) - 1) & BITMASK(3:0);
			ocp_write_field(MP1_OCPAPB03, 7:4, MinClkPct);
			ocp_write_field(MP1_OCPAPB03, 3:3, OCPUnits & 0x1);

			/* 8. OCP sample rate */
			ocp_write_field(MP1_OCPAPB03, 2:1, 0x2);
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s CFG done, MinClkPct = %d, OCPUnits = %d, ConfigMode = %d\n",
			(Cluster == OCP_LL) ? "LL" : "L", MinClkPct, OCPUnits, ConfigMode);
	}

	return 0;
}

static int LittleOCPTarget(enum ocp_cluster Cluster, unsigned int OCTgt)
{
	int OCWATgt;

	if (ocp_use_v2_for_mp0_mp1()) {
		/* Convert Target to to UQ8.12 */
		OCWATgt = ((OCTgt << 12) / 1000) & BITMASK(19:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPBCFG05, 31:12, OCWATgt);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPBCFG05, 31:12, OCWATgt);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		/* Convert Target to to UQ16 */
		OCWATgt = OCTgt & BITMASK(15:0);

		if (Cluster == OCP_LL)
				ocp_write_field(MP0_OCPAPB04, 15:0, OCWATgt);
		else if (Cluster == OCP_L)
				ocp_write_field(MP1_OCPAPB04, 15:0, OCWATgt);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s OCTgt = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", OCWATgt);

	return 0;
}

static int LittleOCPFreq(enum ocp_cluster Cluster, unsigned int FreqMHz)
{
	int FreqPctVal;

	if (ocp_use_v2_for_mp0_mp1()) {
		FreqPctVal = ((Cluster == OCP_L) ? FreqMHz * 10000 / OCP_FREQ_PCT_100_L
						: FreqMHz * 10000 / OCP_FREQ_PCT_100_LL);

		/* Convert FreqPct to to UQ7.12 */
		FreqPctVal = ((FreqPctVal << 12) / 100) & BITMASK(18:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPBCFG06, 18:0, FreqPctVal);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPBCFG06, 18:0, FreqPctVal);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		FreqMHz = FreqMHz & BITMASK(11:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPB05, 27:16, FreqMHz);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPB05, 27:16, FreqMHz);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s FreqPct = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", FreqPctVal);

	return 0;
}

static int LittleOCPVoltage(enum ocp_cluster Cluster, unsigned int Voltage)
{
	int VoltVal;

	if (ocp_use_v2_for_mp0_mp1()) {
		/* Convert Voltage to to UQ4.12 */
		VoltVal = ((Voltage << 12) / 1000) & BITMASK(15:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPBCFG07, 15:0, VoltVal);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPBCFG07, 15:0, VoltVal);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		/* Convert Voltage to to UQ1.10 */
		VoltVal = ((Voltage << 10) / 1000) & BITMASK(10:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPB05, 10:0, VoltVal);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPB05, 10:0, VoltVal);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s Volt = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", VoltVal);

	return 0;
}

static int LittleOCPClkDiv(enum ocp_cluster Cluster, enum ocp_clk_div NDiv)
{
	/* OCPv2 only */
	if (ocp_use_v2_for_mp0_mp1()) {
		unsigned int Addr;

		if (Cluster == OCP_LL)
			Addr = MP0_OCP_GENERAL_CTRL;
		else if (Cluster == OCP_L)
			Addr = MP1_OCP_GENERAL_CTRL;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		if (NDiv > CLK_DIV_16)
			NDiv = CLK_DIV_16;

		/* 1: disable OCP clock: mpx_ocp_general_ctrl[2]=0 */
		ocp_write_field(Addr, 2:2, 0);
		/* 2: change clock divide ratio: mpx_ocp_general_ctrl[17:12]=0xF */
		ocp_write_field(Addr, 17:12, 0xF);
		/* 3: change ACC shift ratio: mpx_ocp_general_ctrl[11:9]=NDiv */
		ocp_write_field(Addr, 11:9, NDiv & BITMASK(2:0));
		/* 4: enable low power mode: mpx_ocp_general_ctrl[8]=0x0 */
		ocp_write_field(Addr, 8:8, 0);
		/* 5: enable OCP clock: mpx_ocp_general_ctrl[2]=1 */
		ocp_write_field(Addr, 2:2, 1);
		/* 6: unmask OCP DCM EN */
		ocp_write_field(Addr, 0:0, 0);

		ocp_dbg("%s NDiv = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", NDiv);
	} else
		ocp_err("%s not support ClkDiv!\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPAvg(enum ocp_cluster Cluster, unsigned int FRatio, unsigned int AvgWindowSel)
{
	if (ocp_use_v2_for_mp0_mp1()) {
		if (Cluster == OCP_LL) {
			ocp_write_field(MP0_OCPAPBCFG00, 18:16, AvgWindowSel & BITMASK(2:0));
			ocp_write_field(MP0_OCPAPBCFG00, 31:20, FRatio & BITMASK(11:0));
		} else if (Cluster == OCP_L) {
			ocp_write_field(MP1_OCPAPBCFG00, 18:16, AvgWindowSel & BITMASK(2:0));
			ocp_write_field(MP1_OCPAPBCFG00, 31:20, FRatio & BITMASK(11:0));
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s FRatio = %d, AvgWindowSel = %d\n",
			(Cluster == OCP_LL) ? "LL" : "L", FRatio, AvgWindowSel);
	} else {
		/* OCPv3 */
		if (Cluster == OCP_LL) {
			ocp_write_field(MP0_OCPAPB03, 11:8, AvgWindowSel & BITMASK(3:0));
		} else if (Cluster == OCP_L) {
			ocp_write_field(MP1_OCPAPB03, 11:8, AvgWindowSel & BITMASK(3:0));
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s AvgWindowSel = %d\n", (Cluster == OCP_LL) ? "LL" : "L", AvgWindowSel);
	}

	return 0;
}

static int LittleOCPSleepEnDis(enum ocp_cluster Cluster, unsigned int EnDis)
{
	/* OCPv2 only */
	if (ocp_use_v2_for_mp0_mp1()) {
		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPBCFG00, 1:1, EnDis);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPBCFG00, 1:1, EnDis);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s SleepEn = %d\n", (Cluster == OCP_LL) ? "LL" : "L", EnDis);
	} else
		ocp_err("%s not support SleepEnDis!\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPLeakage(enum ocp_cluster Cluster, unsigned int Leakage)
{
	if (ocp_use_v2_for_mp0_mp1()) {
		Leakage = (Leakage /10 ) & BITMASK(11:0);

		if (Cluster == OCP_LL) {
			if (!Leakage) {
				ocp_write_field(MP0_OCPAPBCFG07, 31:20, 0);
				ocp_write_field(MP0_OCPAPBCFG07, 16:16, 0);
			} else {
				ocp_write_field(MP0_OCPAPBCFG07, 31:20, Leakage);
				ocp_write_field(MP0_OCPAPBCFG07, 16:16, 1);
			}
		} else if (Cluster == OCP_L) {
			if (!Leakage) {
				ocp_write_field(MP1_OCPAPBCFG07, 31:20, 0);
				ocp_write_field(MP1_OCPAPBCFG07, 16:16, 0);
			} else {
				ocp_write_field(MP1_OCPAPBCFG07, 31:20, Leakage);
				ocp_write_field(MP1_OCPAPBCFG07, 16:16, 1);
			}
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		Leakage = Leakage & BITMASK(15:0);

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPB04, 31:16, Leakage);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPB04, 31:16, Leakage);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s Leakage = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Leakage);

	return 0;
}

static int LittleOCPEnDis(enum ocp_cluster Cluster, unsigned int EnDis)
{
	if (ocp_use_v2_for_mp0_mp1()) {
		unsigned int AddrCfg00, AddrCfg05;

		if (Cluster == OCP_LL) {
			AddrCfg00 = MP0_OCPAPBCFG00;
			AddrCfg05 = MP0_OCPAPBCFG05;
		} else if (Cluster == OCP_L) {
			AddrCfg00 = MP1_OCPAPBCFG00;
			AddrCfg05 = MP1_OCPAPBCFG05;
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		if (EnDis) {
			/* a. Write TOP/CPU0 LkgEn bits = 1 */
			ocp_write_field(AddrCfg00, 3:2, 0x3);
			/* b. Write TOP/CPU0 LkgInit bits = 1 */
			ocp_write_field(AddrCfg00, 8:7, 0x3);
			/* c. Write OCPIEn = 1 */
			ocp_write_field(AddrCfg00, 0:0, 1);
		} else {
			/* a. Write CGMin = 0xF */
			ocp_write_field(AddrCfg05, 10:7, 0xF);
			/* b. Write OCPIEn = 0 */
			ocp_write_field(AddrCfg00, 0:0, 0);
			/* c. Write TOP/CPU0 LkgEn = 0, LkgInit = 0 */
			ocp_write_field(AddrCfg00, 8:7, 0);
			ocp_write_field(AddrCfg00, 3:2, 0);
		}
	} else {
		/* OCPv3 */
		unsigned int Addr;

		if (Cluster == OCP_LL) {
			Addr = MP0_OCPAPB03;
		} else if (Cluster == OCP_L) {
			Addr = MP1_OCPAPB03;
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		if (EnDis) {
			/* Write OCPIEn = 1 */
			ocp_write_field(Addr, 0:0, 1);
		} else {
			/* Write OCPIEn = 0 */
			ocp_write_field(Addr, 0:0, 0);
		}
	}

	ocp_dbg("%s OCEn = %d\n", (Cluster == OCP_LL) ? "LL" : "L", EnDis);

	return 0;
}

static int LittleOCPIntLimit(enum ocp_cluster Cluster, enum ocp_int_select Select, int Limit)
{
	unsigned int Val = 0;

	if (ocp_use_v2_for_mp0_mp1()) {
		unsigned int AddrCfg09, AddrCfg10;

		if (Cluster == OCP_LL) {
			AddrCfg09 = MP0_OCPAPBCFG09;
			AddrCfg10 = MP0_OCPAPBCFG10;
		} else if (Cluster == OCP_L) {
			AddrCfg09 = MP1_OCPAPBCFG09;
			AddrCfg10 = MP1_OCPAPBCFG10;
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case IRQ_CLK_PCT_MIN:
			Val = ((Limit / 625) - 1) & BITMASK(3:0);
			ocp_write_field(AddrCfg10, 23:20, Val);
			break;
		case IRQ_WA_MAX:
			/* convert to UQ8.12 */
			Val = ((Limit << 12) / 1000) & BITMASK(19:0);
			ocp_write_field(AddrCfg09, 19:0, Val);
			break;
		case IRQ_WA_MIN:
			/* convert to UQ8.12 */
			Val = ((Limit << 12) / 1000) & BITMASK(19:0);
			ocp_write_field(AddrCfg10, 19:0, Val);
			break;
		default:
			ocp_err("Invalid %s Int Limit Select value: %d\n",
				(Cluster == OCP_LL) ? "LL" : "L", Select);
			return -1;
		}
	} else {
		/* OCPv3 */
		unsigned int AddrCfg07, AddrCfg08;

		if (Cluster == OCP_LL) {
			AddrCfg07 = MP0_OCPAPB07;
			AddrCfg08 = MP0_OCPAPB08;
		} else if (Cluster == OCP_L) {
			AddrCfg07 = MP1_OCPAPB07;
			AddrCfg08 = MP1_OCPAPB08;
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case IRQ_CLK_PCT_MIN:
			Val = Limit & BITMASK(6:0);
			ocp_write_field(AddrCfg07, 30:24, Val);
			break;
		case IRQ_WA_MAX:
			Val = Limit & BITMASK(15:0);
			ocp_write_field(AddrCfg08, 15:0, Val);
			break;
		case IRQ_WA_MIN:
			Val = Limit & BITMASK(15:0);
			ocp_write_field(AddrCfg08, 31:16, Val);
			break;
		default:
			ocp_err("Invalid %s Int Limit Select value: %d\n",
				(Cluster == OCP_LL) ? "LL" : "L", Select);
			return -1;
		}
	}

	ocp_dbg("%s Int select = %d, limit = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Select, Val);

	return 0;
}

static int LittleOCPIntEnDis(enum ocp_cluster Cluster, int Value2, int Value1, int Value0)
{
	unsigned int Val = 0;

	if (ocp_use_v2_for_mp0_mp1()) {
		Val = (Value2 << 16) | (Value1 << 8) | Value0;

		if (Cluster == OCP_LL)
			ocp_write(MP0_OCPAPBCFG02, Val);
		else if (Cluster == OCP_L)
			ocp_write(MP1_OCPAPBCFG02, Val);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		unsigned int Val = (Value2 << 8) | (Value1 << 4) | Value0;

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPB07, 10:0, Val);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPB07, 10:0, Val);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s Int EnDis = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Val);

	return 0;
}

static int LittleOCPIntClr(enum ocp_cluster Cluster, int Value2, int Value1, int Value0)
{
	unsigned int Val = 0;

	if (ocp_use_v2_for_mp0_mp1()) {
		Val = (Value2 << 16) | (Value1 << 8) | Value0;

		if (Cluster == OCP_LL)
			ocp_write(MP0_OCPAPBCFG01, Val);
		else if (Cluster == OCP_L)
			ocp_write(MP1_OCPAPBCFG01, Val);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	} else {
		/* OCPv3 */
		Val = (Value2 << 8) | (Value1 << 4) | Value0;

		if (Cluster == OCP_LL)
			ocp_write_field(MP0_OCPAPB01, 10:0, Val);
		else if (Cluster == OCP_L)
			ocp_write_field(MP1_OCPAPB01, 10:0, Val);
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}
	}

	ocp_dbg("%s Int Clr = 0x%x\n", (Cluster == OCP_LL) ? "LL" : "L", Val);

	return 0;
}

static int LittleOCPLkgMonEnDis(enum ocp_cluster Cluster, unsigned int Cpu, unsigned int EnDis)
{
	/* OCPv2 only */
	if (ocp_use_v2_for_mp0_mp1()) {
		unsigned int bits = (1 << (Cpu+8)) | (1 << (Cpu+3));

		/* first cpu will be on/off in OCPEnDis */
		if (!Cpu)
			return 0;

		if (Cluster == OCP_LL) {
			ocp_write(MP0_OCPAPBCFG00, (EnDis)
				? ocp_read(MP0_OCPAPBCFG00) | bits
				: ocp_read(MP0_OCPAPBCFG00) & ~(bits)
			);
		} else if (Cluster == OCP_L) {
			ocp_write(MP1_OCPAPBCFG00, (EnDis)
				? ocp_read(MP1_OCPAPBCFG00) | bits
				: ocp_read(MP1_OCPAPBCFG00) & ~(bits)
			);
		} else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		ocp_dbg("%s CPU %d LkgMon = %d\n", (Cluster == OCP_LL) ? "LL" : "L", Cpu, EnDis);
	} else
		ocp_err("%s not support LkgMonEnDis!\n", (Cluster == OCP_LL) ? "LL" : "L");

	return 0;
}

static int LittleOCPIRQHoldoff(enum ocp_cluster Cluster, enum ocp_int_select Select, int WindowSel)
{
	unsigned int Addr;

	if (ocp_use_v2_for_mp0_mp1()) {
		if (Cluster == OCP_LL)
			Addr = MP0_OCPAPBCFG08;
		else if (Cluster == OCP_L)
			Addr = MP1_OCPAPBCFG08;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case IRQ_CLK_PCT_MIN:
			if (!WindowSel) {
				ocp_write_field(Addr, 7:4, 0);
				ocp_write_field(Addr, 0:0, 0);
			} else {
				ocp_write_field(Addr, 7:4, WindowSel & BITMASK(3:0));
				ocp_write_field(Addr, 0:0, 1);
			}
			break;
		case IRQ_WA_MAX:
			if (!WindowSel) {
				ocp_write_field(Addr, 11:8, 0);
				ocp_write_field(Addr, 1:1, 0);
			} else {
				ocp_write_field(Addr, 11:8, WindowSel & BITMASK(3:0));
				ocp_write_field(Addr, 1:1, 1);
			}
			break;
		case IRQ_WA_MIN:
			if (!WindowSel) {
				ocp_write_field(Addr, 15:12, 0);
				ocp_write_field(Addr, 2:2, 0);
			} else {
				ocp_write_field(Addr, 15:12, WindowSel & BITMASK(3:0));
				ocp_write_field(Addr, 2:2, 1);
			}
			break;
		default:
			ocp_err("Invalid %s Int Limit Select value: %d\n",
				(Cluster == OCP_LL) ? "LL" : "L", Select);
			return -1;
		}
	} else {
		/* OCPv3 */
		if (Cluster == OCP_LL)
			Addr = MP0_OCPAPB07;
		else if (Cluster == OCP_L)
			Addr = MP1_OCPAPB07;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case IRQ_CLK_PCT_MIN:
			ocp_write_field(Addr, 15:12, WindowSel & BITMASK(3:0));
			break;
		case IRQ_WA_MAX:
			ocp_write_field(Addr, 19:16, WindowSel & BITMASK(3:0));
			break;
		case IRQ_WA_MIN:
			ocp_write_field(Addr, 23:20, WindowSel & BITMASK(3:0));
			break;
		default:
			ocp_err("Invalid %s Int Limit Select value: %d\n",
				(Cluster == OCP_LL) ? "LL" : "L", Select);
			return -1;
		}
	}

	ocp_dbg("%s Int select = %d, limit = 0x%x\n",
		(Cluster == OCP_LL) ? "LL" : "L", Select, WindowSel);

	return 0;
}

static int LittleOCPValueStatus(enum ocp_cluster Cluster, enum ocp_value_select Select)
{
	unsigned int Val = 0;
	unsigned int Addr;

	if (ocp_use_v2_for_mp0_mp1()) {
		if (Cluster == OCP_LL)
			Addr = MP0_OCPSTATUS0;
		else if (Cluster == OCP_L)
			Addr = MP1_OCPSTATUS0;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case CLK_AVG:
			Val = ((ocp_read_field(Addr, 15:0) + (1 << 12)) * 625) >> 12;
			break;
		case WA_AVG:
			Val = (ocp_read_field(Addr, 31:16) * 1000) >> 12;
			break;
		case TOTAL_LKG:
			Val = (ocp_read_field(Addr+4, 19:0) * 1000) >> 12;
			break;
		case TOP_RAW_LKG:
			Val = ocp_read_field(Addr+8, 7:0);
			break;
		case CPU0_RAW_LKG:
			Val = ocp_read_field(Addr+12, 7:0);
			break;
		case CPU1_RAW_LKG:
			Val = ocp_read_field(Addr+12, 15:8);
			break;
		case CPU2_RAW_LKG:
			Val = ocp_read_field(Addr+12, 23:16);
			break;
		case CPU3_RAW_LKG:
			Val = ocp_read_field(Addr+12, 31:24);
			break;
		default:
			ocp_err("Invalid OCP select value: %d\n", Select);
			return -1;
		}
	} else {
		/* OCPv3 */
		if (Cluster == OCP_LL)
			Addr = MP0_OCPAPB00;
		else if (Cluster == OCP_L)
			Addr = MP1_OCPAPB00;
		else {
			ocp_err("Invalid cluster id: %d\n", Cluster);
			return -1;
		}

		switch (Select) {
		case CLK_AVG:
			if (ocp_read_field(Addr, 23:23) == 0)
				Val = ocp_read_field(Addr, 22:16);
			break;
		case WA_AVG:
			Val = ocp_read_field(Addr, 15:0);
			break;
		case TOP_RAW_LKG:
			Val = ocp_read_field(Addr, 31:24);
			break;
		case CPU0_RAW_LKG:
			Val = ocp_read_field(Addr+8, 7:0);
			break;
		case CPU1_RAW_LKG:
			Val = ocp_read_field(Addr+8, 15:8);
			break;
		case CPU2_RAW_LKG:
			Val = ocp_read_field(Addr+8, 23:16);
			break;
		case CPU3_RAW_LKG:
			Val = ocp_read_field(Addr+8, 31:24);
			break;
		default:
			ocp_err("Invalid OCP select value: %d\n", Select);
			return -1;
		}
	}

	ocp_dbg("%s Value Status = %d, Select = 0x%x\n",
		(Cluster == OCP_LL) ? "LL" : "L", Val, Select);

	return Val;
}

/* Public APIs */
int OCPRegWrite(unsigned int addr, unsigned int val)
{
#if (!OCP_DVT)
	if (addr < OCP_REG_BASE_ADDR || addr > (OCP_REG_BASE_ADDR + OCP_REG_BANK_SIZE)) {
		ocp_err("Invalid addr(0x%x) to write!\n", addr);
		return -1;
	}
#endif
	ocp_write(addr, val);

	return 0;
}

int OCPRegRead(unsigned int addr)
{
#if (!OCP_DVT)
	if (addr < OCP_REG_BASE_ADDR || addr > (OCP_REG_BASE_ADDR + OCP_REG_BANK_SIZE)) {
		ocp_err("Invalid addr(0x%x) to read!\n", addr);
		return -1;
	}
#endif
	return ocp_read(addr);
}

int OCPEnDis(enum ocp_cluster cluster, unsigned int enable, enum ocp_mode mode)
{
	ocp_dbg("%s cluster %d OCP...\n", (enable) ? "Enable" : "Disable", cluster);

	if (cluster == OCP_B) {
		if (enable) {
			/* Config with min Clkminpct and Watts for Units */
			BigOCPConfig(OCP_CLK_PCT_MIN_V3, OCP_MW, mode);
			/* Set target to Max value */
			BigOCPTarget(OCP_TARGET_MAX_V3);

			/* Set default F, V, LKG */
			if (GET_BITS_VAL(3:3, mode) == 1)
				BigOCPVoltage(OCP_B_DEFAULT_VOLT);
			if (GET_BITS_VAL(1:1, mode) == 1)
				BigOCPLeakage(OCP_B_DEFAULT_LKG);
			if (GET_BITS_VAL(0:0, mode) == 1)
				BigOCPFreq(OCP_B_DEFAULT_FREQ);

			/* Set Avg window */
			BigOCPAvg(OCP_B_DEFAULT_AVG_WINDOW);
			/* Configure IRQ0 */
			/* generate an interrupt if ClkAvg drops below 75% */
			//BigOCPIntLimit(IRQ_CLK_PCT_MIN, 75);
			BigOCPIntClr(0x7, 0x7, 0x7);
			/* Enable OCP */
			BigOCPEnDis(1);
			/* Enable interrupt */
			//BigOCPIntEnDis(0, 0, 1);
		} else {
			/* Disable all INT */
			BigOCPIntEnDis(0, 0, 0);
			/* Clear all INT */
			BigOCPIntClr(0x7, 0x7, 0x7);
			/* Disable OCP */
			BigOCPEnDis(0);
		}
	} else {
		if (ocp_use_v2_for_mp0_mp1()) {
			if (enable) {
				/* Set clkdiv to max value */
				LittleOCPClkDiv(cluster, CLK_DIV_16);
				/* Config with min Clkminpct and Watts for Units */
				LittleOCPConfig(cluster, OCP_CLK_PCT_MIN_V2, OCP_MW, NO_BYPASS);
				/* Set target to Max value */
				LittleOCPTarget(cluster, OCP_TARGET_MAX_V2);
				/* Set default F, V, FRatio and Avg window */
				if (cluster == OCP_LL) {
					LittleOCPFreq(cluster, OCP_LL_DEFAULT_FREQ);
					LittleOCPVoltage(cluster, OCP_LL_DEFAULT_VOLT);
					LittleOCPAvg(cluster, OCP_LL_DEFAULT_FRATIO, OCP_DEFAULT_AVG_WINDOW);
				} else {
					LittleOCPFreq(cluster, OCP_L_DEFAULT_FREQ);
					LittleOCPVoltage(cluster, OCP_L_DEFAULT_VOLT);
					LittleOCPAvg(cluster, OCP_L_DEFAULT_FRATIO, OCP_DEFAULT_AVG_WINDOW);
				}

				/* Disable sleep mode*/
				LittleOCPSleepEnDis(cluster, 0);
				/* Configure IRQ0 */
				/* generate an interrupt if ClkAvg drops below 75% */
				//LittleOCPIntLimit(cluster, IRQ_CLK_PCT_MIN, OCP_DEFAULT_INT_THRES);
				LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
				/* Enable OCP */
				LittleOCPEnDis(cluster, 1);
				/* Wait 2 windows */
				//udelay(1500);
				/* Enable interrupt */
				//LittleOCPIntEnDis(cluster, 0, 0, 1 << IRQ_CLK_PCT_MIN);
			} else {
				/* Disable all INT */
				LittleOCPIntEnDis(cluster, 0, 0, 0);
				/* Clear all INT */
				LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
				/* Disable OCP */
				LittleOCPEnDis(cluster, 0);
				/* Disable clock */
				if (cluster == OCP_LL)
					ocp_write(MP0_OCP_GENERAL_CTRL, 0x0);
				else
					ocp_write(MP1_OCP_GENERAL_CTRL, 0x0);
			}
		} else {
			/* OCPv3 */
			if (enable) {
				/* Config with min Clkminpct and Watts for Units */
				LittleOCPConfig(cluster, OCP_CLK_PCT_MIN_V3, OCP_MW, mode);
				/* Set target to Max value */
				LittleOCPTarget(cluster, OCP_TARGET_MAX_V3);
				/* Set default F, V, LKG */
				if (cluster == OCP_LL) {
					if (GET_BITS_VAL(3:3, mode) == 1)
						LittleOCPVoltage(cluster, OCP_LL_DEFAULT_VOLT);
					if (GET_BITS_VAL(1:1, mode) == 1)
						LittleOCPLeakage(cluster, OCP_LL_DEFAULT_LKG);
					if (GET_BITS_VAL(0:0, mode) == 1)
						LittleOCPFreq(cluster, OCP_LL_DEFAULT_FREQ);
				} else {
					if (GET_BITS_VAL(3:3, mode) == 1)
						LittleOCPVoltage(cluster, OCP_L_DEFAULT_VOLT);
					if (GET_BITS_VAL(1:1, mode) == 1)
						LittleOCPLeakage(cluster, OCP_L_DEFAULT_LKG);
					if (GET_BITS_VAL(0:0, mode) == 1)
						LittleOCPFreq(cluster, OCP_L_DEFAULT_FREQ);
				}
				/* Set default Avg window */
				LittleOCPAvg(cluster, 0, OCP_DEFAULT_AVG_WINDOW);
				/* Configure IRQ0 */
				/* generate an interrupt if ClkAvg drops below 75% */
				//LittleOCPIntLimit(cluster, IRQ_CLK_PCT_MIN, 75);
				LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
				/* Enable OCP */
				LittleOCPEnDis(cluster, 1);
				/* Enable interrupt */
				//LittleOCPIntEnDis(cluster, 0, 0, 1);
			} else {
				/* Disable all INT */
				LittleOCPIntEnDis(cluster, 0, 0, 0);
				/* Clear all INT */
				LittleOCPIntClr(cluster, 0x7, 0x7, 0x7);
				/* Disable OCP */
				LittleOCPEnDis(cluster, 0);
				/* Disable clock */
				if (cluster == OCP_LL)
					ocp_write(MP0_OCP_GENERAL_CTRL, 0x0);
				else
					ocp_write(MP1_OCP_GENERAL_CTRL, 0x0);
			}
		}
	}

	ocp_dbg("%s cluster %d OCP done!\n", (enable) ? "Enable" : "Disable", cluster);

	return 0;
}

int OCPTarget(enum ocp_cluster cluster, unsigned int target)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPTarget(target);
	else
		ret = LittleOCPTarget(cluster, target);

	return ret;
}

int OCPFreq(enum ocp_cluster cluster, unsigned int freq)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPFreq(freq);
	else
		ret = LittleOCPFreq(cluster, freq);

	return ret;
}

int OCPVoltage(enum ocp_cluster cluster, unsigned int volt)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPVoltage(volt);
	else
		ret = LittleOCPVoltage(cluster, volt);

	return ret;
}

int OCPIntClr(enum ocp_cluster cluster, int value2, int value1, int value0)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntClr(value2, value1, value0);
	else
		ret = LittleOCPIntClr(cluster, value2, value1, value0);

	return ret;
}

int OCPIntEnDis(enum ocp_cluster cluster, int value2, int value1, int value0)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntEnDis(value2, value1, value0);
	else
		ret = LittleOCPIntEnDis(cluster, value2, value1, value0);

	return ret;
}

int OCPIntLimit(enum ocp_cluster cluster, enum ocp_int_select select, int limit)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIntLimit(select, limit);
	else
		ret = LittleOCPIntLimit(cluster, select, limit);

	return ret;
}

int OCPLkgMonEnDis(enum ocp_cluster cluster, unsigned int cpu, unsigned int enable)
{
	int ret = -1;

	/* only OCPv2 support */
	if (ocp_use_v2_for_mp0_mp1() && cluster != OCP_B)
		ret = LittleOCPLkgMonEnDis(cluster, cpu, enable);
	else
		ret = 0; /* not support */

	return ret;
}

int OCPValueStatus(enum ocp_cluster cluster, enum ocp_value_select select)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPValueStatus(select);
	else
		ret = LittleOCPValueStatus(cluster, select);

	return ret;
}

int OCPIRQHoldoff(enum ocp_cluster cluster, enum ocp_int_select select, int window)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPIRQHoldoff(select, window);
	else
		ret = LittleOCPIRQHoldoff(cluster, select, window);

	return ret;
}

int OCPConfigMode(enum ocp_cluster cluster, enum ocp_mode mode)
{
	int ret = -1;

	if (ocp_use_v2_for_mp0_mp1()) {
		if (cluster == OCP_B)
			ret = BigOCPConfigMode(mode);
		else
			ret = 0; /* not support */
	} else {
		if (cluster == OCP_B)
			ret = BigOCPConfigMode(mode);
		else
			ret = LittleOCPConfigMode(cluster, mode);
	}

	return ret;
}

int OCPLeakage(enum ocp_cluster cluster, unsigned int leakage)
{
	int ret = -1;

	if (cluster == OCP_B)
		ret = BigOCPLeakage(leakage);
	else
		ret = LittleOCPLeakage(cluster, leakage);

	return ret;
}

