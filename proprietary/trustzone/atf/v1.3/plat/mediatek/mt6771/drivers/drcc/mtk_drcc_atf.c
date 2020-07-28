/**
 * @file    mtk_drcc_atf.c
 * @brief   Driver for embedded transient control
 *
 */
#include <delay_timer.h> /* for udelay */
#include <mtk_drcc_atf.h>
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
#define drcc_read(addr) mmio_read_32(addr)
#define drcc_read_field(addr, range)	\
	((drcc_read(addr) & BITMASK(range)) >> LSB(range))
#define drcc_write(addr, val) mmio_write_32(addr, val)

/************************************************
 * Static Variable
 ************************************************
 */
static unsigned int control_drcc_enable = 1;
static unsigned int drccEn = 1;
static unsigned int drcc_k_rst;

int drcc_init(void)
{
	if ((drcc_read(DRCC_CONF0) & ~0x02) == 0x03340000) {
		drcc_write(DRCC_CONF3, 0x0);
		drcc_write(DRCC_CONF0, 0x13380000);
		drcc_write(DRCC_CONF0, drcc_read(DRCC_CONF0) | 1);
	}
	return 0;
}

int drcc_enable(unsigned int onOff)
{
	if (onOff) {
		onOff = 1;
		drccEn = 1;
	} else {
		onOff = 0;
		drccEn = 0;
	}
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x01) | (onOff & 0x01));
	return 1;
}

int drcc_trig(unsigned int onOff, unsigned int value)
{
	unsigned int temp;

	temp = ((value & 0x01) << 1) | (onOff & 0x01);
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x30) | (temp << 4));
	return 1;
}

int drcc_count(unsigned int onOff, unsigned int value)
{
	unsigned int temp;

	temp = ((value & 0x01) << 1) | (onOff & 0x01);
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0xC0) | (temp << 6));
	return 1;

}

int drcc_mode(unsigned int value)
{
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x7000) | ((value & 0x07) << 12));
	return 1;
}

int drcc_code(unsigned int value)
{
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x003F0000) | ((value & 0x3F) << 16));
	return 1;
}

int drcc_hwgatepct(unsigned int value)
{
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x07000000) | ((value & 0x07) << 24));
	return 1;
}

int drcc_vreffilt(unsigned int value)
{
	drcc_write(DRCC_CONF0,
		(drcc_read(DRCC_CONF0) & ~0x70000000) | ((value & 0x07) << 28));
	return 1;
}

int drcc_autocalibdelay(unsigned int value)
{
	drcc_write(DRCC_CONF3,
		(drcc_read(DRCC_CONF3) & ~0x0F) | (value & 0x0F));
	return 1;
}

int drcc_forcetrim(unsigned int onOff, unsigned int value)
{
	unsigned int temp;

	temp = ((onOff & 0x01) << 4) | (value & 0x0F);
	drcc_write(DRCC_CONF3,
		(drcc_read(DRCC_CONF3) & ~0x1F0) | (temp << 4));
	return 1;
}

int drcc_protect(unsigned int value)
{
	drcc_write(DRCC_CONF3,
		(drcc_read(DRCC_CONF3) & ~0x200) | ((value & 0x01) << 9));
	return 1;

}

int drcc_reg_read(unsigned int addr)
{
	if ((addr <= DRCC_CONF3) && (addr >= DRCC_CONF0))
		return drcc_read(addr);
	else
		return 0;
}
/*
 * drcc_lock is used by big core with Dcache disabled.
 * drcc_lock needs to be allocated in non-cacheable region
 * because drcc_lock is shared one cacheline with other variables
 * and it will cause incoherency issue.
 * If drcc_lock is obtained by one  big core, at this moment lock
 * value 1 is cache line filled in another big core's Dcache, then is
 * flushed to external memory later because of cacheline evication
 * or this big core enters idle state. If so, drcc_lock state is incorrect in
 * external memory for now.
 * Then, any big core wakes up and will be busy waiting to gets drcc_lock,
 * because drcc_lock is 1.
 */
#if USE_COHERENT_MEM
#define __coherent  __section("tzfw_coherent_mem")
#else
#define __coherent
#endif

spinlock_t drcc_lock __coherent;

int drcc_check(void)
{
	int status;

	spin_lock(&drcc_lock);
	if ((drcc_read(DRCC_CONF3) >> 12 & 0x03) != 0x01) {
		control_drcc_enable = 0;
		drcc_write(DRCC_CONF0, (drcc_read(DRCC_CONF0) & ~0x01) | 0);
		/* inform DVFS to add offset for DRCC disable */
		status = 1;
	} else {
		control_drcc_enable = 1;
		/* inform DVFS to change offset = 0 for DRCC enable */
		status = 0;
	}
	spin_unlock(&drcc_lock);
	return status;
}

int drcc_offset(void)
{
	if (control_drcc_enable == 0)
		return 5;
	else
		return 0;
}

int drcc_calib_rst(void)
{
	unsigned int temp = 0;

	if  (drccEn == 1) {
		if ((drcc_k_rst & 0x3) != 0x1)
			temp = 0xF;
	} else
		temp = 0xD;

	return (drcc_k_rst << 4) | temp;
}

void mtk_drcc_main(void)
{
	int i;
	unsigned long mpidr = read_mpidr_el1();
	int cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 6);

	/* def CONFIG_MACH_MT6771 */
	if (cpu > 3) {
		/* read / write register function */
		/* mmio_read_32(); */
		/* mmio_write_32(); */
		/* DRCC init and enable here*/
		if ((drcc_read(DRCC_CONF0)  & ~0x02) == 0x03340000) {
			drcc_write(DRCC_CONF3, 0x0);
			drcc_write(DRCC_CONF0, 0x13380000);
			drcc_write(DRCC_CONF0,
				drcc_read(DRCC_CONF0) | drccEn);
			for (i = 0; i < 5; i++)
				;
			drcc_k_rst = (drcc_read(DRCC_CONF3) >> 12) & 0xFF;
		}
	}
}
