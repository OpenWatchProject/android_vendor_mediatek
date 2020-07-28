/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MT_ILDO_H__
#define __MT_ILDO_H__




/* voltage ctrl configs */
#define NORMAL_DIFF_VRSAM_VPROC		100000
#define MAX_DIFF_VSRAM_VPROC		250000
#define MIN_VSRAM_VOLT				850000
#define MAX_VSRAM_VOLT				1120000
#define MIN_VPROC_VOLT				600000
#define MAX_VPROC_VOLT				1120000
#define ILDO_NORMAL_DIFF			100000

#define VOSEL_BASE					200000
#define VOSEL_STEP					3125
#define VO_MIN						600000
#define VO_MAX						996875

#define VINLV_BASE					400000
#define VINLV_STEP					25000
#define VIN_MIN						600000
#define VIN_MAX						1120000

#define VRET_BASE					200000
#define VRET_STEP					3125
#define VRET_MIN					400000
#define VRET_MAX					600000


#define NR_ILDO_CPU 2
#define NR_ILDO_REG 16
#define ILDO_CPU_START_ID 6
#define ILDO_EFUSE_CNT 4


#define ildo_per_cpu(cpu, reg) (ILDO_CPU_BASE[cpu] + reg)
#define for_each_ildo_cpu(x)	for (x = 0; x < NR_ILDO_CPU; x++)
#define ildo_volt_clamp(v, vmin, vmax) ((v < vmin) ? vmin : (v > vmax) ? vmax : v)
#define ildo_to_volt_step(v, base, step) ((v - base) / step)
/*
 *#define ildo_to_volt_step_round_up(v, base, step) \
 * ((((v - base) % step) == 0) ? ((v - base) / step) : ((v - base) / step) + 1)
 */


/* #define MCUSYS_BASE				(0x0C530000) */
#define CA75_CPU6_CONFIG_REG		(0x0C533000)
#define CA75_CPU7_CONFIG_REG		(0x0C533800)


enum ildo_volt_select {
	ILDO_SEL_VIN,
	ILDO_SEL_VO,
	ILDO_SEL_VOMIN,
	ILDO_SEL_VOMAX,

	NR_ILDO_VOLT_SEL
};

enum ildo_state_select {
	CPU_CUR,
	CPU_NEXT,
	ILDO_CUR,
	ILDO_NEXT,

	NR_ILDO_STATE_SEL
};


enum ildo_state {
	ILDO_OFF_PMOS_OFF,
	ILDO_OFF_PMOS_ON,
	ILDO_READY_PMOSON,
	ILDO_READY_FROM_OFF,
	ILDO_FUNC_DVS,
	ILDO_FUNC_COREON,
	ILDO_FUNC_NORMAL_ON,
	ILDO_RET_AUTO_ON,
	ILDO_RET_AUTO_OFF,

	NR_ILDO_STATE
};

enum core_mode {
	BYPASS_ILDO,
	BUCK_MODE,
	FUNC_MODE,
	RET_MODE,
	CORE_OFF,

	NR_CORE_MODE
};

static const unsigned int macro_dsel_mask[4] = {
	0x0,
	0x1,
	0x3,
	0x7,
};



/* per_cpu registers mapping */
static const unsigned int ILDO_CPU_BASE[NR_ILDO_CPU] = {
	CA75_CPU6_CONFIG_REG,
	CA75_CPU7_CONFIG_REG
};


struct mt_cpu_ildo {
	unsigned int dvfs_volt_uv;
	unsigned int target_volt_uv;
	unsigned int buck_volt_uv;
	enum core_mode cur_mode;
};


/* efuse related setting */
struct ildo_devinfo {
	unsigned int ILDO0_CMP_MAIN_NORM:4;
	unsigned int ILDO1_CMP_MAIN_NORM:4;
	unsigned int ILDO2_CMP_MAIN_NORM:4;
	unsigned int ILDO0_CMP_MAIN_RETENTION:4;
	unsigned int ILDO1_CMP_MAIN_RETENTION:4;
	unsigned int ILDO2_CMP_MAIN_RETENTION:4;

	unsigned int ILDO0_CMP_OV:4;
	unsigned int ILDO1_CMP_OV:4;
	unsigned int ILDO2_CMP_OV:4;
	unsigned int ILDO0_CMP_UV:4;
	unsigned int ILDO1_CMP_UV:4;
	unsigned int ILDO2_CMP_UV:4;
};


/* iLDO register offset */
#define ILDO_CONTROL_0				(0x320)
#define ILDO_CONTROL_1				(0x324)
#define ILDO_CONTROL_2				(0x328)
#define ILDO_CONTROL_3				(0x32C)
#define ILDO_CONTROL_4				(0x330)
#define ILDO_CONTROL_5				(0x334)
#define ILDO_CONTROL_6				(0x338)
#define ILDO_CONTROL_7				(0x33C)
#define ILDO_CONTROL_8				(0x340)
#define ILDO_CONTROL_9				(0x344)
#define ILDO_CONTROL_10				(0x348)
#define ILDO_CONTROL_11				(0x34C)
#define ILDO_CONTROL_12				(0x350)
#define ILDO_CONTROL_13				(0x354)
#define ILDO_CONTROL_14				(0x358)
#define ILDO_CONTROL_15				(0x35C)


/* ILDO_CONTROL_0 */
#define RG_ILDO_MODE				(1 << 0)
#define ILDO_FORCEON				(1 << 1)
#define ILDO_COREON_TC				(1 << 2)
#define ILDO_COREON					(1 << 3)
#define ILDO_COREON_CLK				(1 << 5)
#define ILDO_COREON_N				(1 << 7)
#define ILDO_START					(1 << 8)
#define ILDO_AUTOEN					(1 << 9)
#define ILDO_EN						(23:16)
#define ILDO_TSET					(31:24)

/* ILDO_CONTROL_1 */
#define ILDO_AUTOBANK				(0:0)
#define ILDO_AUTOBANK_FACTF			(2:1)
#define ILDO_AUTOBANK_FACTO			(4:3)
#define ILDO_AUTOBANK_FACTI			(6:5)
#define ILDO_RLSB_SEL1				(15:8)
#define ILDO_RLSB_SEL0P5			(17:16)
#define ILDO_RLSB_SEL0P25			(19:18)
#define ILDO_EN_RET_HW				(20:20)
#define ILDO_PURE_STUCK				(21:21)
#define ILDO_STUCK_SPEED			(23:22)
#define ILDO_SPIKECOMP_DIS			(24:24)
#define ILDO_DIPCOMP_DIS			(25:25)

/* ILDO_CONTROL_2 */
#define ILDO_VINLV					(4:0)
#define ILDO_VOSEL					(15:8)
#define ILDO_VOMAX					(23:16)
#define ILDO_VOMIN					(31:24)

/* ILDO_CONTROL_3 */
#define ILDO_DVS_SEL				(1:0)
#define ILDO_BANKCKSL				(3:2)
#define ILDO_DVS_BYPASS				(4:4)
#define ILDO_BANK_TRACK_EN			(5:5)
#define ILDO_TRIM					(13:8)
#define ILDO_OVWIN_SEL				(21:16)
#define ILDO_UVWIN_SEL				(29:24)

/* ILDO_CONTROL_4 */
#define ILDO_PWRGATE_B				(1 << 0)
#define ILDO_UVR_BLOCKSL			(3:2)
#define ILDO_UVCODESL				(5:4)
#define ILDO_OVSTCK_SEL				(7:6)
#define ILDO_VO2HCK_SEL				(9:8)
#define ILDO_MHSTCK_SEL				(11:10)
#define ILDO_MLSTCK_SEL				(13:12)
#define ILDO_UVCMPDLY_SEL			(15:14)
#define ILDO_EXT_CLK_SEL			(1 << 16)
#define ILDO_INTERNAL_CLK_SEL		(17:17)
#define ILDO_EN_UVCOMP_CAL			(18:18)
#define ILDO_CONTCMP_IBIAS_SEL		(21:19)
#define ILDO_CLKCAP_PROG			(23:22)

/* ILDO_CONTROL_5 */
#define ILDO_RET_VOSEL				(7:0)
#define ILDO_RET_RLSB_SEL1			(15:8)
#define ILDO_RET_RLSB_SEL0P5		(17:16)
#define ILDO_RET_RLSB_SEL0P25		(19:18)
#define ILDO_RET_TWAIT				(21:20)
#define ILDO_RET_CLKCAP_PROG		(23:22)
#define ILDO_EN_VREF_SNH			(1 << 24)

/* ILDO_CONTROL_6 */
#define ILDO_MACRO_SEL				(7:0)
#define ILDO_DTEST1_PROG			(10:8)
#define ILDO_DTEST2_PROG			(13:11)
#define ILDO_ATEST1_PROG			(17:15)
#define ILDO_ATEST2_PROG			(20:18)

/* ILDO_CONTROL_7 */
#define ILDO_RESEV0					(7:0)
#define ILDO_RESEV1					(15:8)
#define ILDO_TBD					(19:16)
#define ILDO_RESEV2					(23:20)
#define ILDO_RESEV3					(31:24)

/* ILDO_CONTROL_8 */
#define ILDO_BYPASS_B				(1 << 0)
#define ILDO_SW_MODE				(1 << 1)
#define ILDO_COREON_EN				(1 << 2)
#define ILDO_RETON_EN				(1 << 3)
#define ILDO_FUNC_EN				(1 << 4)
#define ILDO_RET_OFF				(1 << 5)
#define ILDO_HW_BUCK_EN				(1 << 6)
#define ILDO_HW_FUNC_EN				(1 << 7)
#define ILDO_SW_BUCK_EN				(1 << 8)
#define ILDO_SW_FUNC_EN				(1 << 9)
#define ILDO_FIXED_VOLT_DIFF		(12:10)
#define ILDO_WRAPPER_CG				(1 << 13)
#define ILDO_MACRO_CG				(1 << 14)
#define ILDO_PWR_ON_CG				(1 << 15)
#define ILDO_SW_RESET				(1 << 16)
#define ILDO_SW_SPARKEN				(1 << 17)
#define ILDO_SW_LOGIC_PRE1_PDB		(1 << 18)
#define ILDO_SW_LOGIC_PRE2_PDB		(1 << 19)
#define ILDO_SW_LOGIC_PDB			(31:24)

/* ILDO_CONTROL_9 */
#define ILDO_CURRENT_STATE			(8:0)
#define CORE_CURRENT_STATE			(13:9)
#define ILDO_NEXT_STATE				(24:16)
#define CORE_NEXT_STATE				(29:25)

/* ILDO_CONTROL_10 */
#define DEBUG_VOSEL_OUT0			(7:0)
#define DEBUG_VOSEL_OUT1			(15:8)
#define DEBUG_VOSEL_OUT2			(23:16)
#define ILDO_DONE0					(24:24)
#define ILDO_DONE1					(25:25)
#define ILDO_DONE2					(26:26)

/* ILDO_CONTROL_11 */
#define ILDO_DIP_RST				(7:0)
#define ILDO_ILDO0_DIP_COUNTER		(14:8)
#define ILDO_ILDO1_DIP_COUNTER		(22:16)
#define ILDO_ILDO2_DIP_COUNTER		(30:24)

/* ILDO_CONTROL_12 */
#define ILDO_ILDO0_CMP_MAIN_NORM	(3:0)
#define ILDO_ILDO1_CMP_MAIN_NORM	(7:4)
#define ILDO_ILDO2_CMP_MAIN_NORM	(11:8)

/* ILDO_CONTROL_13 */
#define ILDO_ILDO0_CMP_MAIN_RETENTION	(3:0)
#define ILDO_ILDO1_CMP_MAIN_RETENTION	(7:4)
#define ILDO_ILDO2_CMP_MAIN_RETENTION	(11:8)

/* ILDO_CONTROL_14 */
#define ILDO_ILDO0_CMP_OV			(3:0)
#define ILDO_ILDO1_CMP_OV			(7:4)
#define ILDO_ILDO2_CMP_OV			(11:8)

/* ILDO_CONTROL_15 */
#define ILDO_ILDO0_CMP_UV			(3:0)
#define ILDO_ILDO1_CMP_UV			(7:4)
#define ILDO_ILDO2_CMP_UV			(11:8)


#endif /* _MT_ILDO_H */
