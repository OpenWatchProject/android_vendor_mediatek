#ifndef _MT_UDI_API_H
#define _MT_UDI_API_H

/*
 * @file    mtk_udi_api.h
 * @brief   Driver header for Universal Debug Interface
 *
 */

#define __ATF_MODE_UDI__

#ifdef __ATF_MODE_UDI__
#include <mmio.h>
#endif

/*******************************************************************************
 * Defines for UDI Service queries
 ******************************************************************************/

 /* REG ACCESS */
#ifdef __ATF_MODE_UDI__
#define ptp3_reg_read(addr)			mmio_read_32(addr)
#define ptp3_reg_write(addr, val)	mmio_write_32(addr, val)
#endif

#define ptp3_shift(val, bits, shift) \
	((val & (((unsigned int)0x1 << bits) - 1)) << shift)
#define ptp3_reg_write_bits(addr, val, bits, shift) \
	ptp3_reg_write(addr, ((ptp3_reg_read(addr) & ~(ptp3_shift((unsigned int)0xffffffff, \
	bits, shift))) | ptp3_shift(val, bits, shift)))
#define ptp3_reg_read_bits(addr, bits, shift) \
	((ptp3_reg_read(addr) >> shift) & (((unsigned int)0x1 << bits) - 1))

/* Register Definition */
#define UDI_PTP3_UDI_REG                                 (0x0C53a220)
/*
 *GWTAP0 SW UDI Register:
 *Bit(s)	Mnemonic	Name	Description
 *11:8		sw_tdo	Software JTAG control for top-level GatewayTAP
 *6			sw_trst	Software JTAG control for top-level GatewayTAP
 *5			sw_tms	Software JTAG control for top-level GatewayTAP
 *4			sw_tdi	Software JTAG control for top-level GatewayTAP
 *3:0		sw_tck	Software JTAG control for top-level GatewayTAP, 1, 2 ,4 ,8 for sub_chains 0/1/2/3
 */

#define rg_sw_tck_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 4, 0)
#define rg_sw_tck_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 4, 0)
#define rg_sw_tdi_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 4)
#define rg_sw_tdi_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 4)
#define rg_sw_tms_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 5)
#define rg_sw_tms_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 5)
#define rg_sw_trst_w(val)                               ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 6)
#define rg_sw_trst_r()                                  ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 6)
#define rg_sw_tdo_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 4, 8)
#define rg_sw_tdo_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 4, 8)

/*
 *GWTAP1 SW UDI Register:
 *Bit(s)	Mnemonic	Name	Description
 *27:24		sw_tdo	Software JTAG control for top-level GatewayTAP
 *22		sw_trst	Software JTAG control for top-level GatewayTAP
 *21		sw_tms	Software JTAG control for top-level GatewayTAP
 *20		sw_tdi	Software JTAG control for top-level GatewayTAP
 *19:16		sw_tck	Software JTAG control for top-level GatewayTAP, 1, 2 ,4 ,8 for sub_chains 0/1/2/3
*/

#define gwtap1_rg_sw_tck_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 4, 16)
#define gwtap1_rg_sw_tck_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 4, 16)
#define gwtap1_rg_sw_tdi_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 20)
#define gwtap1_rg_sw_tdi_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 20)
#define gwtap1_rg_sw_tms_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 21)
#define gwtap1_rg_sw_tms_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 21)
#define gwtap1_rg_sw_trst_w(val)                               ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 1, 22)
#define gwtap1_rg_sw_trst_r()                                  ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 1, 22)
#define gwtap1_rg_sw_tdo_w(val)                                ptp3_reg_write_bits(UDI_PTP3_UDI_REG, val, 4, 24)
#define gwtap1_rg_sw_tdo_r()                                   ptp3_reg_read_bits(UDI_PTP3_UDI_REG, 4, 24)


/*******************************/
/* UDI function */
extern int UDIRead(unsigned int reg_value, unsigned int t_count, unsigned int gwtap);
extern int UDIBitCtrl(unsigned int reg_value, unsigned int gwtap);
extern int UDIRegWrite(unsigned int addr, unsigned int val);
extern int UDIRegRead(unsigned int addr);
/* ********************************** */


#endif
