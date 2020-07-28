 /*
 * @file    mt_udi_api.c
 * @brief   Driver for Universal Debug Interface
 *
 */

#include <debug.h>
#include <delay_timer.h>
#include <mtk_udi_api.h>



#define PRINTF_UDI	0

/*******************************************************************************
 * SMC Call for Kernel UDI interface
 #define udi_jtag_clock(sw_tck, i_trst, i_tms, i_tdi, count)
 mt_secure_call(MTK_SIP_KERNEL_UDI_JTAG_CLOCK, (((0x1 << (sw_tck & 0x03)) << 3) |
						((i_trst & 0x01) << 2) |
						((i_tms & 0x01) << 1) |
						(i_tdi & 0x01)),
						count, 0)
 ******************************************************************************/
int UDIRead(unsigned int reg_value, unsigned int t_count)
{
	unsigned short sub_chains, i_trst, i_tms, i_tdi, i_tdo = 0;

	/* sub changes = 1, 2, 4 ,8 for channel 0/1/2/3 */
	sub_chains = ((reg_value >> 3) & 0x0f);
	i_trst = ((reg_value >> 2) & 0x01);
	i_tms  = ((reg_value >> 1) & 0x01);
	i_tdi  =  (reg_value & 0x01);

	rg_sw_trst_w(i_trst);
	udelay(1);
	rg_sw_tms_w(i_tms);
	udelay(1);
	rg_sw_tdi_w(i_tdi);
	udelay(1);

#if PRINTF_UDI
	INFO("CH = %d, UDI = 0x%x. TRST = %d, TMS = %d, TDI = %d, Count = %d",
	sub_chains, ptp3_reg_read(UDI_PTP3_UDI_REG), i_trst, i_tms, i_tdi, t_count);
#endif

	while (t_count--) {
			rg_sw_tck_w(0);
			udelay(1);
			rg_sw_tck_w(sub_chains); /* jtag_sw_tck is 0,1,2,3 */
			udelay(1);
	}

	udelay(1);
	i_tdo = (rg_sw_tdo_r() == 0) ? 0 : 1;

#if PRINTF_UDI
	INFO(", TDO = %d\n", rg_sw_tdo_r());
#endif

	return i_tdo;
}

int UDIBitCtrl(unsigned int reg_value)
{
	unsigned short sub_chains, i_trst, i_tms, i_tdi, i_tdo = 0;

	/* sub changes = 1, 2, 4 ,8 for channel 0/1/2/3 */
	sub_chains = ((reg_value >> 3) & 0x0f);
	i_trst = ((reg_value >> 2) & 0x01);
	i_tms  = ((reg_value >> 1) & 0x01);
	i_tdi  =  (reg_value & 0x01);

	ptp3_reg_write(UDI_PTP3_UDI_REG, (sub_chains|(i_tdi << 4)|(i_tms << 5)|(i_trst << 6)));


#if PRINTF_UDI
	INFO("UDI = 0x%x. TCK = %d, TRST = %d, TMS = %d, TDI = %d",
	ptp3_reg_read(UDI_PTP3_UDI_REG), sub_chains, i_trst, i_tms, i_tdi);
#endif

	udelay(1);
	i_tdo = (rg_sw_tdo_r() == 0) ? 0 : 1;

#if PRINTF_UDI
	INFO(", TDO = %d\n", rg_sw_tdo_r());
#endif

	return i_tdo;
}

int UDIRead_Big(unsigned int reg_value, unsigned int t_count)
{
	unsigned short sub_chains, i_trst, i_tms, i_tdi, i_tdo = 0;

	/* sub changes = 1, 2, 4 ,8 for channel 0/1/2/3 */
	sub_chains = ((reg_value >> 3) & 0x0f);
	i_trst = ((reg_value >> 2) & 0x01);
	i_tms  = ((reg_value >> 1) & 0x01);
	i_tdi  =  (reg_value & 0x01);

	big_rg_sw_trst_w(i_trst);
	udelay(1);
	big_rg_sw_tms_w(i_tms);
	udelay(1);
	big_rg_sw_tdi_w(i_tdi);
	udelay(1);

#if PRINTF_UDI
	INFO("BIG CH = %d, UDI = 0x%x. TRST = %d, TMS = %d, TDI = %d, Count = %d",
	sub_chains, ptp3_reg_read(UDI_PTP3_UDI_REG), i_trst, i_tms, i_tdi, t_count);
#endif

	while (t_count--) {
			big_rg_sw_tck_w(0);
			udelay(1);
			big_rg_sw_tck_w(sub_chains); /* jtag_sw_tck is 0,1,2,3 */
			udelay(1);
	}

	udelay(1);
	i_tdo = (big_rg_sw_tdo_r() == 0) ? 0 : 1;

#if PRINTF_UDI
	INFO(", TDO = %d\n", big_rg_sw_tdo_r());
#endif

	return i_tdo;
}

int UDIBitCtrl_Big(unsigned int reg_value)
{
	unsigned short sub_chains, i_trst, i_tms, i_tdi, i_tdo = 0;

	/* sub changes = 1, 2, 4 ,8 for channel 0/1/2/3 */
	sub_chains = ((reg_value >> 3) & 0x0f);
	i_trst = ((reg_value >> 2) & 0x01);
	i_tms  = ((reg_value >> 1) & 0x01);
	i_tdi  =  (reg_value & 0x01);

	ptp3_reg_write(UDI_PTP3_UDI_REG, (sub_chains|(i_tdi << 4)|(i_tms << 5)|(i_trst << 6)));


#if PRINTF_UDI
	INFO("BIG UDI = 0x%x. TCK = %d, TRST = %d, TMS = %d, TDI = %d",
	ptp3_reg_read(UDI_PTP3_UDI_REG), sub_chains, i_trst, i_tms, i_tdi);
#endif

	udelay(1);
	i_tdo = (big_rg_sw_tdo_r() == 0) ? 0 : 1;

#if PRINTF_UDI
	INFO(", TDO = %d\n", big_rg_sw_tdo_r());
#endif

	return i_tdo;
}
