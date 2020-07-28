/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARM_ARCH_SVC_H__
#define __ARM_ARCH_SVC_H__

/*
 * For those constants to be shared between C and other sources, apply a 'u'
 * or 'ull' suffix to the argument only in C, to avoid undefined or unintended
 * behaviour.
 *
 * The GNU assembler and linker do not support the 'u' and 'ull' suffix (it
 * causes the build process to fail) therefore the suffix is omitted when used
 * in linker scripts and assembler files.
*/
#if defined(__LINKER__) || defined(__ASSEMBLY__)
# define  U(_x)         (_x)
# define ULL(_x)        (_x)
#else
# define  U(_x)         (_x##u)
# define ULL(_x)        (_x##ull)
#endif

#define SMCCC_VERSION			U(0x80000000)
#define SMCCC_ARCH_FEATURES		U(0x80000001)
#define SMCCC_ARCH_WORKAROUND_1		U(0x80008000)

#endif /* __ARM_ARCH_SVC_H__ */
