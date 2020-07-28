/*
 * Copyright (c) 2015,  ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms,  with or without
 * modification,  are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,  this
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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,  THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL DAMAGES (INCLUDING,  BUT NOT LIMITED TO,  PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  DATA,  OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <mt_ildo.h>
#include <mt_ildo_atf.h>


#if ILDO_ENABLE

/*Static variable declaration*/
static unsigned int ildo_init_done[2] = {0, 0};
static unsigned int ildo_func_endis = 1;
spinlock_t ildo_lock;

/*Static function declaration*/
static int ildo_set_retention_volt(unsigned int cpu, unsigned int volt_uv);

/*static unsigned int ildo_retention_volt_step = 0x60;*/
/* default=0.5v(0.2+96*0.003125) */
static unsigned int ildo_retention_volt[NR_ILDO_CPU] = {500000, 500000};

static struct ildo_devinfo ildo_cpu6_efuse, ildo_cpu7_efuse;
static struct ildo_devinfo *ildo_devinfo[NR_ILDO_CPU] = {
	&ildo_cpu6_efuse,
	&ildo_cpu7_efuse
};

static unsigned int ildo_efuse[ILDO_EFUSE_CNT] = {
	0x0,
	0x0,
	0x0,
	0x0
};

static unsigned int ildo_fake_efuse[ILDO_EFUSE_CNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};


/* fixme: correct efuse address & bit field setting */
int ildo_read_efuse_from_lk(unsigned int id, unsigned int value)
{
	assert(id < ILDO_EFUSE_CNT);

	if (value != 0)
		ildo_efuse[id] = value;
	else{
		ildo_efuse[id] = ildo_fake_efuse[id];
		ildo_dbg("No iLDO efuse for index %d\n", id);
	}

	if (id == ILDO_EFUSE_CNT-1) {
		for (int i = 0; i < NR_ILDO_CPU; ++i) {
			ildo_devinfo[i]->ILDO0_CMP_MAIN_NORM = GET_BITS_VAL(3:0, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO1_CMP_MAIN_NORM = GET_BITS_VAL(19:16, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO2_CMP_MAIN_NORM = GET_BITS_VAL(3:0, ildo_efuse[2*i+1]);
			ildo_devinfo[i]->ILDO0_CMP_MAIN_RETENTION = GET_BITS_VAL(7:4, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO1_CMP_MAIN_RETENTION = GET_BITS_VAL(23:20, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO2_CMP_MAIN_RETENTION = GET_BITS_VAL(7:4, ildo_efuse[2*i+1]);
			ildo_devinfo[i]->ILDO0_CMP_OV = GET_BITS_VAL(11:8, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO1_CMP_OV = GET_BITS_VAL(27:24, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO2_CMP_OV = GET_BITS_VAL(11:8, ildo_efuse[2*i+1]);
			ildo_devinfo[i]->ILDO0_CMP_UV = GET_BITS_VAL(15:11, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO1_CMP_UV = GET_BITS_VAL(31:27, ildo_efuse[2*i]);
			ildo_devinfo[i]->ILDO2_CMP_UV = GET_BITS_VAL(15:11, ildo_efuse[2*i+1]);
		}
	}

	return 0;
}


/*
 * for hotplug(init): cond == 0
 * for MCDI,SODI,DeepIDLE,Suspend/Resume(reinit): cond == 1
 */
int ildo_init(unsigned int cpu, unsigned int cond)
{
	unsigned int ildo_cpu_id = cpu - ILDO_CPU_START_ID;

	if (cpu < 6)
		return 0;

	assert(ildo_cpu_id < NR_ILDO_CPU);

	if (cond == 0 && ildo_init_done[ildo_cpu_id] == 1)
		return 0;

	if (ildo_init_done[ildo_cpu_id] == 0)
		ildo_init_done[ildo_cpu_id] = 1;

	ildo_set_retention_volt(ildo_cpu_id, ildo_retention_volt[ildo_cpu_id]);

#if ILDO_DVT
	ildo_set_bit(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_8), ILDO_WRAPPER_CG | ILDO_MACRO_CG | ILDO_PWR_ON_CG);
	/* ILDO_CONTROL_1: ILDO_DIPCOMP_DIS */
	/* ildo_write_field(ildo_per_cpu(i, ILDO_CONTROL_1), 25:25, 0x1); */

	/* ILDO_CONTROL_4: ILDO_OVSTCK_SEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 7:6, 0x3);
	/* ILDO_CONTROL_4: ILDO_VO2HCK_SEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 9:8, 0x3);
	/* ILDO_CONTROL_4: ILDO_MHSTCK_SEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 11:10, 0x1);
	/* ILDO_CONTROL_4: ILDO_MLSTCK_SEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 13:12, 0x3);
	/* ILDO_CONTROL_4: ILDO_UVCMPDLY_SEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 15:14, 0x0);
	/* ILDO_CONTROL_3: ILDO_UVWIN_SEL */
	/* ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_3), 29:24, 0x3F); */
	/*  before entering func mode, set to 0 */
	/* ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_4), 18:18, 0x1); */
#endif

	/* write efuse value */
	ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_12),
		(ildo_devinfo[ildo_cpu_id]->ILDO0_CMP_MAIN_NORM) |
		(ildo_devinfo[ildo_cpu_id]->ILDO1_CMP_MAIN_NORM) << 4 |
		(ildo_devinfo[ildo_cpu_id]->ILDO2_CMP_MAIN_NORM) << 8);

	ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_13),
		(ildo_devinfo[ildo_cpu_id]->ILDO0_CMP_MAIN_RETENTION) |
		(ildo_devinfo[ildo_cpu_id]->ILDO1_CMP_MAIN_RETENTION) << 4 |
		(ildo_devinfo[ildo_cpu_id]->ILDO2_CMP_MAIN_RETENTION) << 8);

	ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_14),
		(ildo_devinfo[ildo_cpu_id]->ILDO0_CMP_OV) |
		(ildo_devinfo[ildo_cpu_id]->ILDO1_CMP_OV) << 4 |
		(ildo_devinfo[ildo_cpu_id]->ILDO2_CMP_OV) << 8);

	ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_15),
		(ildo_devinfo[ildo_cpu_id]->ILDO0_CMP_UV) |
		(ildo_devinfo[ildo_cpu_id]->ILDO1_CMP_UV) << 4 |
		(ildo_devinfo[ildo_cpu_id]->ILDO2_CMP_UV) << 8);

	/* fixme: set target volt to highest value & buck volt to lowest value for safety? */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_2), 15:0, (0xFF << 8));

	/* disable bypass mode & enable SW buck mode */
	ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_8),
		ildo_read(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_8)) |
		ILDO_BYPASS_B | ILDO_SW_BUCK_EN);

	/* wait for iLDO from off to buck mode */
	udelay(10);

#if CONFIG_ILDO_FUNC_ENABLE
	/* ildo function mode may be turn off from kernel */
	if (ildo_func_endis)
		/* Enable iLDO state change determination by HW mechanism */
		ildo_write(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_8),
			ildo_read(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_8)) |
			ILDO_FUNC_EN | ILDO_HW_BUCK_EN | ILDO_HW_FUNC_EN);
#endif

	return 0;
}



/*
 ***********************************************
 * for kernel smc call
 ***********************************************
 */

int ildo_reg_read(unsigned int addr)
{
	int ret;
/*
 *	int i;
 *
 *	for (i = 0; i < NR_ILDO_CPU; ++i)
 *	{
 *		if (addr >= ildo_per_cpu(i, ILDO_CONTROL_0) && addr <= ildo_per_cpu(i, ILDO_CONTROL_15))
 *			break;
 *	}
 *
 *	if (i == NR_ILDO_CPU) {
 *		ildo_err("Invalid addr(0x%x) to read!\n", addr);
 *		return -1;
 *	}
 */

	spin_lock(&ildo_lock);
	ret = ildo_read(addr);
	spin_unlock(&ildo_lock);

	return ret;
}


int ildo_reg_write(unsigned int addr, unsigned int val)
{
/*
 *	int i;
 *
 *	for (i = 0; i < NR_ILDO_CPU; ++i)
 *	{
 *		if (addr >= ildo_per_cpu(i, ILDO_CONTROL_0) && addr <= ildo_per_cpu(i, ILDO_CONTROL_15))
 *			break;
 *	}
 *
 *	if (i == NR_ILDO_CPU) {
 *		ildo_err("Invalid addr(0x%x) to read!\n", addr);
 *		return -1;
 *	}
 */

	spin_lock(&ildo_lock);
	ildo_write(addr, val);
	spin_unlock(&ildo_lock);

	return 0;
}

int ildo_set_endis(unsigned int endis)
{
	int i;
	unsigned int val;

	ildo_func_endis = endis;
	for_each_ildo_cpu(i) {
		val = ILDO_FUNC_EN | ILDO_HW_BUCK_EN | ILDO_HW_FUNC_EN;
		if (ildo_func_endis == 0)
			/* clear bits */
			val = ildo_read(ildo_per_cpu(i, ILDO_CONTROL_8)) & ~val;
		else
			/* set bits */
			/* fixme: is it safe to enable ildo from kernel? */
			val = ildo_read(ildo_per_cpu(i, ILDO_CONTROL_8)) | val;
		spin_lock(&ildo_lock);
		ildo_write(ildo_per_cpu(i, ILDO_CONTROL_8), val);
		spin_unlock(&ildo_lock);
	}

	return 0;
}


int ildo_get_endis(void)
{
	return ildo_func_endis;
}


/*
 * input volt is in uv
 * ildo retention voltage in volt = 0.2 + volt_step*0.003125 (v)
 */
int ildo_change_retention_volt(unsigned int cpu, unsigned int volt_uv)
{
	unsigned int ildo_cpu_id = cpu;

	assert(ildo_cpu_id < NR_ILDO_CPU);

#if CONFIG_ILDO_RET_ENABLE
	ildo_retention_volt[cpu] = volt_uv;
#endif

	return 0;
}



/************************************************
 * iLDO internal functions
 ************************************************
 */

/*
 * ildo retention voltage in volt = 0.2 + volt_step*0.003125 (v)
 */
static int ildo_set_retention_volt(unsigned int cpu, unsigned int volt_uv)
{
#if CONFIG_ILDO_RET_ENABLE
	unsigned int volt_step;
	unsigned int ildo_cpu_id = cpu;

	assert(ildo_cpu_id < NR_ILDO_CPU);

	volt_step = (ildo_volt_clamp(volt_uv, VRET_MIN, VRET_MAX) - VRET_BASE) / VRET_STEP;

	/* ILDO_CONTROL_5_RET_VOSEL: ILDO_RET_VOSEL */
	ildo_write_field(ildo_per_cpu(ildo_cpu_id, ILDO_CONTROL_5), 7:0, volt_step);

	ildo_dbg("ildo_set_retention_volt(big cpu %d): step = %d, volt = %d uv\n", ildo_cpu_id, volt_step, volt_uv);
#endif
	return 0;
}


#endif /* ILDO_ENABLE */
