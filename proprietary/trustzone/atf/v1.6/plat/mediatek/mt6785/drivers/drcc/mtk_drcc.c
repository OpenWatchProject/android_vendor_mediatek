/**
 * @file    mtk_drcc.c
 * @brief   Driver for embedded transient control
 *
 */
#include <delay_timer.h> /* for udelay */
#include <mtk_drcc.h>
#include <debug.h>
#include <spinlock.h>

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
	(((unsigned int) -1 >> (31 - MSB(r))) & ~((1U << LSB(r)) - 1))

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
#define drcc_read(addr) mmio_read_32(addr)
#define drcc_read_field(addr, range)	\
	((drcc_read(addr) & BITMASK(range)) >> LSB(range))
#define drcc_write(addr, val) mmio_write_32(addr, val)

/************************************************
 * Static Variable
 ************************************************
 */

int drcc_debug_init(void)
{
	unsigned int drcc_n = 0;
	int status = 1;

	for (drcc_n = 0; drcc_n < DRCC_NUM; drcc_n++) {
		if (drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) == 0x13340) {
			if (drcc_n < 6)
				drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n), 0x13390);
			else
				drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n), 0x133A0);
			drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
				drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) | 1);
		} else
			status = 0;
	}
	return status;
}

int drcc_enable(unsigned int onOff, unsigned int drcc_n)
{
	int status = 1;

	onOff = (onOff) ? 1 : 0;
	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
			(drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) & ~0x01) | (onOff & 0x01));
	} else
		status = 0;
	return status;
}

int drcc_trig(unsigned int onOff, unsigned int value, unsigned int drcc_n)
{
	unsigned int temp;
	int status = 1;

	temp = ((value & 0x01) << 1) | (onOff & 0x01);
	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n),
			(drcc_read(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n)) & ~0x30) | (temp << 4));
	} else
		status = 0;
	return status;
}

int drcc_count(unsigned int onOff, unsigned int value, unsigned int drcc_n)
{
	unsigned int temp;
	int status = 1;

	temp = ((value & 0x01) << 1) | (onOff & 0x01);
	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n),
			(drcc_read(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n)) & ~0xC0) | (temp << 6));
	} else
		status = 0;
	return status;

}

int drcc_mode(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n),
			(drcc_read(CPU0_DRCC_CFG_REG0 + (0x800 * drcc_n)) & ~0x7000) | ((value & 0x07) << 12));
	} else
		status = 0;
	return status;
}

int drcc_code(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
			(drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) & ~0x3F0) | ((value & 0x3F) << 4));
	} else
		status = 0;
	return status;
}

int drcc_hwgatepct(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
			(drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) & ~0x7000) | ((value & 0x07) << 12));
	} else
		status = 0;
	return status;
}

int drcc_vreffilt(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
			(drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) & ~0x70000) | ((value & 0x07) << 16));
	} else
		status = 0;
	return status;
}

int drcc_autocalibdelay(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n),
			(drcc_read(CPU0_DRCC_A0_CONFIG + (0x200 * drcc_n)) & ~0xF00000) | (value & 0x0F) << 20);
	} else
		status = 0;
	return status;
}

int drcc_forcetrim(unsigned int onOff, unsigned int value, unsigned int drcc_n)
{
	unsigned int temp;
	int status = 1;

	temp = ((onOff & 0x01) << 4) | (value & 0x0F);
	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_CFG_REG2 + (0x800 * drcc_n),
			(drcc_read(CPU0_DRCC_CFG_REG2 + (0x800 * drcc_n)) & ~0x1F0) | (temp << 4));
	} else
		status = 0;
	return status;
}

int drcc_protect(unsigned int value, unsigned int drcc_n)
{
	int status = 1;

	if (drcc_n < DRCC_NUM) {
		drcc_write(CPU0_DRCC_CFG_REG2 + (0x800 * drcc_n),
			(drcc_read(CPU0_DRCC_CFG_REG2 + (0x800 * drcc_n)) & ~0x200) | ((value & 0x01) << 9));
	} else
		status = 0;
	return status;
}

int drcc_reg_read(unsigned int addr)
{
	if ((addr <= CPU7_DRCC_A0_CONFIG) && (addr >= DRCC_BASE))
		return drcc_read(addr);
	else
		return 0;
}
