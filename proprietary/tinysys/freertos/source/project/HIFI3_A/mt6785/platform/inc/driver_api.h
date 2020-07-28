/*
 * driver_api.h
 *
 *  Created on: 2017/7/13
 *      Author: MTK09865
 */

#ifndef DRIVER_API_H
#define DRIVER_API_H

/// I/O    ////////////////////////////////////////////////////////////////////
#define OUTREG32(a,b)   (*(volatile unsigned int *)(a) = (unsigned int)(b))
#define INREG32(a)      (*(volatile unsigned int *)(a))

#ifndef DRV_WriteReg32
#define DRV_WriteReg32(addr,data)   ((*(volatile unsigned int *)(addr)) = (unsigned int)(data))
#endif
#ifndef DRV_Reg32
#define DRV_Reg32(addr)             (*(volatile unsigned int *)(addr))
#endif
#define DRV_WriteReg8(addr,data)    ((*(volatile char *)(addr)) = (char)(data))
#define DRV_Reg8(addr)              (*(volatile char *)(addr))
#define DRV_SetReg32(addr, data)    ((*(volatile unsigned int *)(addr)) |= (unsigned int)(data))
#define DRV_ClrReg32(addr, data)    ((*(volatile unsigned int *)(addr)) &= ~((unsigned int)(data)))
#define DRV_SetReg8(addr, data)    ((*(volatile char *)(addr)) |= (char)(data))
#define DRV_ClrReg8(addr, data)    ((*(volatile char *)(addr)) &= ~((char)(data)))

// lowcase version
#define outreg32(a,b)   (*(volatile unsigned int *)(a) = (unsigned int)(b))
#define inreg32(a)      (*(volatile unsigned int *)(a))

#ifndef drv_write_reg32
#define drv_write_reg32(addr,data)   ((*(volatile unsigned int *)(addr)) = (unsigned int)(data))
#endif
#ifndef drv_reg32
#define drv_reg32(addr)             (*(volatile unsigned int *)(addr))
#endif
#define drv_write_reg8(addr,data)    ((*(volatile char *)(addr)) = (char)(data))
#define drv_reg8(addr)              (*(volatile char *)(addr))
#define drv_set_reg32(addr, data)    ((*(volatile unsigned int *)(addr)) |= (unsigned int)(data))
#define drv_clr_reg32(addr, data)    ((*(volatile unsigned int *)(addr)) &= ~((unsigned int)(data)))
#define drv_set_reg8(addr, data)    ((*(volatile char *)(addr)) |= (char)(data))
#define drv_clr_reg8(addr, data)    ((*(volatile char *)(addr)) &= ~((char)(data)))

#ifndef drv_write_reg16
#define drv_write_reg16(addr,data)    ((*(volatile unsigned short *)(addr)) = (unsigned short)(data))

#endif

#ifndef drv_reg16
#define drv_reg16(addr)              (*(volatile unsigned short *)(addr))
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef bool
typedef unsigned char  bool;
#endif

#ifndef MAXIMUM
#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MINIMUM
#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef UPPER_BOUND
#define UPPER_BOUND(in, up) ((in) > (up) ? (up) : (in))
#endif

#ifndef LOWER_BOUND
#define LOWER_BOUND(in, lo) ((in) < (lo) ? (lo) : (in))
#endif

#ifndef BOUNDED
#define BOUNDED(in, up, lo) ((in) > (up) ? (up) : (in) < (lo) ? (lo) : (in))
#endif

#endif /*  */
