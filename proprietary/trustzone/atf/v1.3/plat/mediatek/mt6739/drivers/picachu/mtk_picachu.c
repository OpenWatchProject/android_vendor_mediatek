/**
 * @file    mtk_picachu.c
 * @brief   Driver for Power Integrity CAlibration and CHaracterizaton Utility
 *
 */
#include <mtk_picachu.h>

#define SEC_AO_BASE		(IO_PHYS + 0x1A000) /* Security AO */
#define SRAMROM_BOOT_ADDR	(SEC_AO_BASE + 0x0)
#define SRAMROM_SEC_CTRL	(SEC_AO_BASE + 0x10)

#define SW_ROM_PD               (1U << 31)

/*****************************************************************************
 * This API is used to configure the PICACHU boot address located in
 * file maxtrans_asm_ca53_entry.S
******************************************************************************/
void picachu_cfg_boot_addr(void)
{
	mmio_write_32(SRAMROM_SEC_CTRL,
				mmio_read_32(SRAMROM_SEC_CTRL) | SW_ROM_PD);

	mmio_write_32(SRAMROM_BOOT_ADDR, (uintptr_t) fp0_maxtrans_slave_start);

	dsb();
}

void picachu_cfg_default_boot_addr(void)
{
	mmio_write_32(SRAMROM_SEC_CTRL,
				mmio_read_32(SRAMROM_SEC_CTRL) & ~SW_ROM_PD);

	mmio_write_32(SRAMROM_BOOT_ADDR, 0);

	dsb();
}
