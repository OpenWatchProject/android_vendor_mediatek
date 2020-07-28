/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * Neither the name of MICROTRUST nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#ifndef __PLATFORM_DEF_TEEI_H__
#define __PLATFORM_DEF_TEEI_H__

#include "teei_atf.h"

/*Non-Secure World IRQ */
#define NSEC_CALL_INTR		(281)
#define NSEC_INTSCHED_INTR	(282)

#define NSEC_APP_INTR       282
#define NSEC_BOOT_INTR      283
#define NSEC_SCHED_INTR     284
#define NSEC_DRV_INTR       287
#define NSEC_RDRV_INTR      278
#define NSEC_LOG_INTR       277
#define NSEC_ERR_INTR       276
#define SEC_TUI_CANCEL		  275

#define TEEI_BOOT_PARAMS (((struct atf_arg_t *)(uintptr_t)(&gteearg))->tee_boot_arg_addr)
/* Move key param from (base + 0x200) to (base + 0x300)
 * secure isram base            : atf param
 * secure isram base + 0x100    : teei boot param (TEEI_BOOT_PARAMS)
 * secure isram base + 0x200    : secure memory param
 * secure isram base + 0x300    : teei key param (TEEI_BOOT_PARAMS + 0x200)
 */
#define TEEI_SECURE_PARAMS (TEEI_BOOT_PARAMS+0x200)

/*
 * MTK 6765 SPI Interrupts and DEV_APC
 **/

#define SEC_SPI (132)       /*SPI0*/
#if 0
/* Here is sample from mt6739 */
#ifdef SEC_SPI0
#define SEC_SPI (118+32)        /*SPI0*/
#define SPI_APC_NUM 80
#endif
#ifdef SEC_SPI1
#define SEC_SPI (122+32)        /*SPI1*/
#define SPI_APC_NUM 86
#endif
#ifdef SEC_SPI2
#define SEC_SPI (128+32)        /*SPI2*/
#define SPI_APC_NUM 87
#endif

#endif
#endif
