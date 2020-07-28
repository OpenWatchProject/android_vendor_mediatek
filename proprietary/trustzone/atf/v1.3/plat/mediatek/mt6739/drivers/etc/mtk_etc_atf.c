/**
 * @file    mtk_etc_atf.c
 * @brief   Driver for embedded transient control
 *
 */
#include <delay_timer.h> /* for udelay */
#include <mtk_etc_atf.h>

/************************************************
 * bit operation
************************************************/
#undef  BIT
#define BIT(bit)	(1U << (bit))

#define MSB(range)	(1 ? range)
#define LSB(range)	(0 ? range)
/**
 * Genearte a mask wher MSB to LSB are all 0b1
 * @r:	Range in the form of MSB:LSB
 */
#define BITMASK(r)	\
	(((unsigned) -1 >> (31 - MSB(r))) & ~((1U << LSB(r)) - 1))

/**
 * Set value at MSB:LSB. For example, BITS(7:3, 0x5A)
 * will return a value where bit 3 to bit 7 is 0x5A
 * @r:	Range in the form of MSB:LSB
 */
/* BITS(MSB:LSB, value) => Set value at MSB:LSB  */
#define BITS(r, val)	((val << LSB(r)) & BITMASK(r))

#define GET_BITS_VAL(_bits_, _val_)   (((_val_) & (BITMASK(_bits_))) >> ((0) ? _bits_))

/************************************************
 * REG ACCESS
************************************************/
#define etc_read(addr) mmio_read_32(addr)
#define etc_read_field(addr, range)	\
	((etc_read(addr) & BITMASK(range)) >> LSB(range))
#define etc_write(addr, val) mmio_write_32(addr, val)
/**
 * Write a field of a register.
 * @addr:	Address of the register
 * @range:	The field bit range in the form of MSB:LSB
 * @val:	The value to be written to the field
 */
#define etc_write_field(addr, range, val)	\
	etc_write(addr, (etc_read(addr) & ~BITMASK(range)) | BITS(range, val))

/************************************************
 * static Variable
************************************************/
static unsigned int etcVout; /* ATC_VOUT */
static unsigned int etc_enable, etc_droop_verr;

static int etc_vOut(unsigned int volt) /* coded per ATC_VOUT.VOUT description PMIC STEP*/
{
	unsigned int value;
	/* 450000uv + 6250x for PMIC*/
	/* 400000uv + 3125x for ETC */
	/* return volt * 2 + 16; */
	value = (volt - ETC_V_BASE + ETC_STEP - 1) / ETC_STEP;
	value = (value < 0x50)? 0x50: value;
	return value;
}

#if 0
static int etc_vOverCode(unsigned int volt) ; /* coded per ATC_OVER_CTRL.OVER_VERR description */
{
	return (volt + ETC_STEP - 1) / ETC_STEP;
}

static int etc_vAlarmCode(unsigned int volt) ; /* coded per ATC_VALARM_VERR description */
{
	return (volt + ETC_STEP - 1) / ETC_STEP;
}

static int etc_vErrCode(unsigned int volt) ; /* coded per ATC_DROOP_VERR.VERR0 description */
{
	return (volt - 18750 + ETC_STEP - 1) / ETC_STEP;
}
#endif

int etc_init(void)
{
	etc_droop_verr = etc_read(ETC_LOADER) & 0x7FFF;
	etc_dbg("ETC droop_verr from sram = %x", etc_droop_verr);
	if (etc_droop_verr == 0x7FFF) {
		etc_write(ATC_DVC3_LO, 0x0000);
		etc_write(ATC_DVC3_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0003);

		etc_write(ATC_DVC2_LO, 0x0000);
		etc_write(ATC_DVC2_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0002);

		etc_write(ATC_DVC1_LO, 0x0000);
		etc_write(ATC_DVC1_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0001);

		etc_write(ATC_DVC0_LO, 0x0000);
		etc_write(ATC_DVC0_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0000);

		/* MCUSYS_ETC_RESET_BAR */
		etc_write_field(0x10A40100, 27:27, 0);
		etc_enable = 0;
	} else {
		if (etc_read(0x10212240) & 0x01) { /* check unipll was enabled*/
			/* enable etc bit from soc */
			/* MCUSYS_ETC_RESET_BAR */
			etc_write_field(0x10A40100, 27:27, 1);
			/* MCUSYS_ETC_DORMANT */
			etc_write_field(0x10A40100, 31:31, 1);

			etc_write(ATC_VOUT, 0x00C6);
			udelay(10);

			etc_write(ATC_ACT_EN, 0x0000);
			etc_write(ATC_ANA_TRIM1, 0x0000);
			etc_write(ATC_ANA_TRIM2, 0x0000);

			etc_write(ATC_SW_DORMANT, 0x0000);
			etc_write(ATC_VALARM_VERR, 0x0050);
			etc_write(ATC_OVER_CTRL, 0xB600);
			etc_write(ATC_DROOP_CTRL, 0x11FF);
			etc_write(ATC_DROOP_VERR, etc_droop_verr);
			etc_write(ATC_VMON_CTRL, 0x0FFF);
			etc_write(ATC_VMON_STATUS, 0x0000);
			etc_write(ATC_TEST_ACCESS, 0x0000);
			etc_write(ATC_TDMUX_CTRL, 0x0000);
			etc_write(ATC_TDMUX, 0x0000);
			etc_write(ATC_TDMUX_STATUS, 0x0000);

			etc_write(ATC_DVC0_LO, 0x0000);
			etc_write(ATC_DVC0_HI, 0x5000);
			etc_write(ATC_DVC_WRITE, 0x0000);

			etc_write(ATC_DVC1_LO, 0x0000);
			etc_write(ATC_DVC1_HI, 0x8000);
			etc_write(ATC_DVC_WRITE, 0x0001);

			etc_write(ATC_DVC2_LO, 0x0000);
			etc_write(ATC_DVC2_HI, 0xB000);
			etc_write(ATC_DVC_WRITE, 0x0002);

			etc_write(ATC_DVC3_LO, 0x2321);
			etc_write(ATC_DVC3_HI, 0x503D);
			etc_write(ATC_DVC_WRITE, 0x0003);

			etc_write(ATC_FORCE_DROOP_OFF, 0x0000);
			etc_write(ATC_LDO_CTRL, 0x0808);
			etc_write(ATC_LDO_WAIT_TIME, 0x8040);
			etc_write(ATC_LIMIT_STATUS, 0x0000);
			etc_write(ATC_MISC2_CTRL, 0x0000);
			etc_write(ATC_VALARM_STATUS, 0x0001);
			etc_write(ATC_WAIT_TIME, 0x4020);
			/* MCUSYS_ETC_DORMANT */
			etc_write_field(0x10A40100, 31:31, 0);
			etc_enable = 1;
		}
	}
	return 0;
}

int etc_voltage_change(unsigned int new_vout)
{
	if ((etc_read(0x10212240) & 0x01) && (etc_enable == 1)) { /* check unipll was enabled*/
		etc_write(ATC_VOUT, etc_vOut(new_vout));
		etcVout = new_vout;
		udelay(10);
	}
	return 0;
}


int etc_ank(unsigned int ank, unsigned int vol)
{
	ank = ((ank & ATC_ID) == ATC_ID) ? ank :
		((((~ank) & ETC_LOADER) | (ank & (~ETC_LOADER))) ^ ETC_LOADER);
	etc_write(ank, vol);
	return 0;
}

int etc_reg_read(unsigned int addr)
{
	if ((addr <= (ATC_ID + 0xF0)) && (addr >= ATC_ID) && (etc_enable == 1)) {
		return etc_read(addr);
	} else
		return 0;
}

int etc_power_off(void)
{
	if (etc_enable == 1) {
		etc_write(ATC_DVC3_LO, 0x0000);
		etc_write(ATC_DVC3_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0003);

		etc_write(ATC_DVC2_LO, 0x0000);
		etc_write(ATC_DVC2_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0002);

		etc_write(ATC_DVC1_LO, 0x0000);
		etc_write(ATC_DVC1_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0001);

		etc_write(ATC_DVC0_LO, 0x0000);
		etc_write(ATC_DVC0_HI, 0x0000);
		etc_write(ATC_DVC_WRITE, 0x0000);

		/* MCUSYS_ETC_RESET_BAR */
		etc_write_field(0x10A40100, 27:27, 0);
		etc_enable = 0;
	}
	return 0;
}

int etc_dormant_ctrl(unsigned int onOff)
{
	/*
	* can use SOC configure register base address 0x0x10A40100[31]
	* to control HW dormant
	* SW dormant will orverwrite HW dormant mode
	*/
	/* MCUSYS_ETC_DORMANT */
	etc_write_field(0x10A40100, 31:31, (onOff)? 0x0001: 0x0000);
	/* etc_write(ATC_SW_DORMANT, (onOff)? 0x0001: 0x0000); */
	return 0;
}
