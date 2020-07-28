#include <mmio.h>

#define MCU_ALL_PWR_ON_CTRL    0x0C530B58

void lastpc_postinit(void)
{
       unsigned int ctrl = mmio_read_32(MCU_ALL_PWR_ON_CTRL);

       /*
        * clear
        * MCU_ALL_PWR_ON_CTRL[1] mcu_pwr_on and
        * MCU_ALL_PWR_ON_CTRL[2] mcu_pwr_iso_dis
        * to let non-booting cores pwr off
        */
       ctrl &= ~(0x3<<1);

       mmio_write_32(MCU_ALL_PWR_ON_CTRL, ctrl);
}
