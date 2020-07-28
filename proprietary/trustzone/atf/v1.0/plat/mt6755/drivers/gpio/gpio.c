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
#include <debug.h>
#include <gpio.h>
#include <mmio.h>
#include <platform_def.h>

enum {
	MAX_GPIO_REG_BITS = 16,
};

struct mt_gpio_obj {
	struct gpio_regs *reg;
};

static struct mt_gpio_obj gpio_dat = {
	.reg = (struct gpio_regs *)(GPIO_BASE),
};

static struct mt_gpio_obj *gpio_obj = &gpio_dat;

struct mt_gpioext_obj {
	struct gpioext_regs *reg;
};

static struct mt_gpioext_obj gpioext_dat = {
	.reg = (struct gpioext_regs *)(GPIOEXT_BASE),
};

static struct mt_gpioext_obj *gpioext_obj = &gpioext_dat;

static inline struct mt_gpio_obj *mt_get_gpio_obj(void)
{
	return gpio_obj;
}

static inline struct mt_gpioext_obj *mt_get_gpioext_obj(void)
{
	return gpioext_obj;
}

enum {
	GPIO_PRO_DIR = 0,
	GPIO_PRO_DOUT,
	GPIO_PRO_DIN,
	GPIO_PRO_PULLEN,
	GPIO_PRO_PULLSEL,
	GPIO_PRO_MODE,
	GPIO_PRO_MAX,
};

static int32_t mt_set_gpio_chip(uint32_t pin, uint32_t property, uint32_t val)
{
	uint32_t pos = 0;
	uint32_t bit = 0;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();
	uint16_t *reg;
	uint32_t data = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	if (property >= GPIO_PRO_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;
	data = 1L << bit;

	switch (property) {
	case GPIO_PRO_DIR:
		if (val == GPIO_DIR_IN)
			reg = &obj->reg->dir[pos].rst;
		else
			reg = &obj->reg->dir[pos].set;
		break;
	case GPIO_PRO_DOUT:
		if (val == GPIO_OUT_ZERO)
			reg = &obj->reg->dout[pos].rst;
		else
			reg = &obj->reg->dout[pos].set;
		break;
	default:
		return -ERINVAL;
	}

	mmio_write_16((uintptr_t)reg, data);

	return RSUCCESS;
}

static int32_t mt_set_gpio_ext(uint32_t pin, uint32_t property, uint32_t val)
{
	INFO("not supported\n");
	return -ERINVAL;
}

static void mt_gpio_pin_decrypt(uint32_t *cipher)
{
	if ((*cipher & (0x80000000)) == 0)
		INFO("Pin %u decrypt warning!\n", *cipher);
	*cipher &= ~(0x80000000);
}

int32_t mt_set_gpio_out(uint32_t pin, uint32_t output)
{
	uint32_t gp = GPIO_PRO_DOUT;

	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ?
		mt_set_gpio_ext(pin, gp, output) :
		mt_set_gpio_chip(pin, gp, output);
}

void gpio_set(uint32_t gpio, int32_t value)
{
	mt_set_gpio_out(gpio, value);
}

#define MAX_GPIO_MODE_PER_REG 10
#define MAX_GPIO_REG_BITS 32

#define MAX_GPIO_PROTECTION_NUM	5
uint32_t protectionPinNum;

static uint32_t blackList[MAX_GPIO_PROTECTION_NUM] = {0, 0, 0, 0, 0};


void gpio_init_protection_list(uint32_t *pinList)
{

	int i;
	uint32_t value;

	for (i = 0; i < 7; i++) {
		if (pinList[i]) { /*some pin in these 32 bits need to be protected*/
			int j; /*to loop 32 times*/

			value = pinList[i];
			for (j = 0; j < 32; j++) {
				if (value & (0x1<<j)) {
					blackList[protectionPinNum] = (32*i)+j;
					protectionPinNum++;
					if (protectionPinNum == MAX_GPIO_PROTECTION_NUM) {
						INFO("protection list reach maximum number %d, now %d, %d, %d, %d, %d"
							, MAX_GPIO_PROTECTION_NUM, blackList[0],
							blackList[1], blackList[2], blackList[3], blackList[4]);
						return;
					}
				}
			}
		}
	}
	INFO("%d protection pin, %d, %d, %d, %d, %d under protected\n", protectionPinNum, blackList[0], blackList[1],
		blackList[2], blackList[3], blackList[4]);
}
/*#define GPIO_SMC_UT*/

/*the more protection pin found, the more bit set*/
uint32_t getMask(uint32_t addr, uint32_t pin)
{
	unsigned long pos, bit;

	/*invalid address, all block*/
	if (addr > 0x6f0)
		return 0xffffffff;

	/*check mode*/
	if (addr >= 0x300) {
		pos = pin / MAX_GPIO_MODE_PER_REG;
		bit = pin % MAX_GPIO_MODE_PER_REG;

#ifdef GPIO_SMC_UT
		INFO("getMask mode: 0x%x, pin: %d\n", addr, pin);
#endif
		if ((pos * 16) == (addr-0x300)) {
			/*INFO("need to protect pin %d\n", pin);*/
			/*return 0x7<<(bit*3);*/
			return bit < 5 ? (0x7<<(bit*3)) : (0x7<<((bit*3)+1));
		}
	} else {/*check others*/

		pos = pin / MAX_GPIO_REG_BITS;
		bit = pin % MAX_GPIO_REG_BITS;

#ifdef GPIO_SMC_UT
		INFO("getMask other: 0x%x, pin: %d\n", addr, pin);
#endif
		/*0x0~0x60, 0x100~0x160, 0x200~0x260*/
		if ((pos * 16) == (addr & 0x000000F0)) {
			/*INFO("need to protect pin %d\n", pin);*/
			return 0x1<<bit;
		}
	}

	return 0;

}

/*above existed before we add SMC GPIO API, keep these*/
uint64_t GPIO_read(uint64_t address)
{
	int i;
	uint32_t mask;
	uint64_t allMask = 0xffffffff;

	for (i = 0; i < protectionPinNum; i++) {
		mask = getMask(address, blackList[i]);
		if (mask)
			allMask &= ~mask;
	}

#ifdef GPIO_SMC_UT
	INFO("GPIO_read: 0x%lx, mask: 0x%lx\n", address, allMask);
#endif
	/*after here, allMask should almost 0xffffffff, with only protection pin has 0*/

	return (mmio_read_32(GPIO_BASE+address) & allMask);
}

void GPIO_write(uint32_t address, uint32_t value)
{
	int i;
	uint32_t mask;
	uint32_t allMask = 0xffffffff;
	uint32_t original;

	for (i = 0; i < protectionPinNum; i++) {
		mask = getMask(address, blackList[i]);
		if (mask)
			allMask &= ~mask;
	}

#ifdef GPIO_SMC_UT
	INFO("GPIO_write before: 0x%x 0x%x, 0x%x, mask: 0x%x\n", address,
		value, mmio_read_32((GPIO_BASE+address)&0xfffffff0), allMask);
#endif
	/*masked bit need to keep*/
	original = (mmio_read_32(GPIO_BASE+address) & (~allMask));
#ifdef GPIO_SMC_UT
	INFO("GPIO_write before 2: 0x%x 0x%x, 0x%x\n", value, original, ((value & allMask) | original));
#endif
	if (address <= 0x6f0)
		mmio_write_32((GPIO_BASE+address), ((value & allMask) | original));

#ifdef GPIO_SMC_UT
	if ((address%16) == 4) { /*set operation*/
		if (value != ((mmio_read_32((GPIO_BASE+address)&0xfffffff0)) & value))
			INFO("GPIO_write set failed, 0x%x\n", (GPIO_BASE+address));
	} else if ((address%16) == 8) { /*clear operation*/
		if (0 != ((mmio_read_32((GPIO_BASE+address)&0xfffffff0)) & value))
			INFO("GPIO_write clear failed, 0x%x\n", (GPIO_BASE+address));

	} else {
		if (value != mmio_read_32(GPIO_BASE+address))
			INFO("GPIO_write failed, 0x%x\n", (GPIO_BASE+address));
	}

	INFO("GPIO_write after: 0x%x\n", mmio_read_32((GPIO_BASE+address)&0xfffffff0));
#endif
	/*return;*/
}


