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

#ifndef __MT_ILDO_ATF_H__
#define __MT_ILDO_ATF_H__


#include <debug.h>
#include <mmio.h>
#include <delay_timer.h>
#include <assert.h>
#include <spinlock.h>
#include "mt_ildo.h"


#define ILDO_ENABLE					(0)
#define ILDO_DVT					(0)
#define CONFIG_ILDO_RET_ENABLE		(0)
#define CONFIG_ILDO_FUNC_ENABLE		(0)


/* LOG */
#define ILDO_DEBUG_LOG_ON	1
#define TAG	"[iLDO]"
#define ildo_err(fmt, ...)	ERROR(TAG""fmt, __VA_ARGS__)
#define ildo_warn(fmt, ...)	WARN(TAG""fmt, __VA_ARGS__)
#define ildo_info(fmt, ...)	INFO(TAG""fmt, __VA_ARGS__)

#if ILDO_DEBUG_LOG_ON
#define ildo_dbg(...)		ildo_info(__VA_ARGS__)
#else
#define ildo_dbg(...)
#endif /* ILDO_DEBUG_LOG_ON */


/*
 * BIT Operation
 */
#undef  BIT
#define BIT(_bit_)                    (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)           ((((unsigned int) -1 >> (31 - ((1) ? _bits_))) \
& ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define BITMASK(_bits_)               (((unsigned int) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))
#define GET_BITS_VAL(_bits_, _val_)   (((_val_) & (BITMASK(_bits_))) >> ((0) ? _bits_))

/**
 * Read/Write a field of a register.
 * @addr:       Address of the register
 * @range:      The field bit range in the form of MSB:LSB
 * @val:        The value to be written to the field
 */
#define ildo_read(addr)		mmio_read_32((uintptr_t)addr)
#define ildo_write(addr,  val)	mmio_write_32((uintptr_t)addr,  val)
#define ildo_read_field(addr, range)		GET_BITS_VAL(range, ildo_read(addr))
#define ildo_write_field(addr, range, val)	ildo_write(addr, (ildo_read(addr) \
& ~(BITMASK(range))) | BITS(range, val))
#define ildo_set_bit(addr, set) ildo_write(addr, (ildo_read(addr) | (set)))
#define ildo_clr_bit(addr, clr) ildo_write(addr, (ildo_read(addr) & ~(clr)))

/************************************/
extern int ildo_init(unsigned int cpu_id, unsigned int cond);
extern int ildo_read_efuse_from_lk(unsigned int id, unsigned int value);
extern int ildo_reg_read(unsigned int addr);
extern int ildo_reg_write(unsigned int addr, unsigned int val);
extern int ildo_set_endis(unsigned int endis);
extern int ildo_get_endis(void);
extern int ildo_change_retention_volt(unsigned int cpu, unsigned int volt_uv);

/************************************/


#endif /* _MT_ILDO_ATF_H */

