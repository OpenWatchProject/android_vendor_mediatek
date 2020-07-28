/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "drStd.h"
#include "drError.h"
#include "drTuiHal.h"
#include "limits.h" /* ULONG_MAX */

struct deviceInfo;
struct protection;

typedef struct protection
{
    struct deviceInfo   *device;
    uint32_t            regStat;
    uint32_t            bit;
} protection_t;

typedef struct sysmmu
{
    struct deviceInfo   *device;
} sysmmu_t;

typedef struct clock
{
    struct deviceInfo   *device;
    uint32_t            reg;
    uint32_t            bit;
} clk_t;

typedef struct bus
{
    struct deviceInfo   *device;
    uint32_t            slave_address;
    uint32_t            type;
} bus_t;

typedef struct gpio
{
    struct deviceInfo   *device;
    uint32_t            regClear;
    uint32_t            regPending;
    uint32_t            bit;
} gpio_t;

typedef enum deviceState{
    DEV_UNCONFIGURED = 0,
    DEV_CONFIGURED
} deviceState_t;

typedef struct deviceInfo
{
    uint32_t        irq_num;
    uint32_t        addr;
    uint32_t        paddr;
    uint32_t        len;
    char            *name;
    protection_t    *tzpc;
    uint32_t        nb_tzpc;
    clk_t         clock;
    bus_t           bus;
    gpio_t          irqGpio;
    void            *extra_data;
    sysmmu_t        sysmmu;
    deviceState_t state;
} deviceInfo_t;

drError_t mapIO(const deviceInfo_t *device);
drError_t unmapIO(const deviceInfo_t *device);

// Helper macros
#define REG(addr)   *((volatile uint32_t*)(addr))
#define REG8(addr)  *((volatile uint8_t*)(addr))

// Handy to play around
#define SHOW(a)         drDbgPrintf(#a" = 0x%x (@ 0x%x)\n", a, &a);

/* Define a macro ROUND_UP_NEXT_POWER_OF_2 that rounds up its argument to the
 * next power of two
 * see explaination at
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
#define ROUND0(x)  ( 0ul + (x)) /* ensure arithmetic is unsigned */
#define ROUND1(x)  ( ROUND0(x) | (ROUND0(x)  >> 1 ) )
#define ROUND2(x)  ( ROUND1(x) | (ROUND1(x)  >> 2 ) )
#define ROUND4(x)  ( ROUND2(x) | (ROUND2(x)  >> 4 ) )
#define ROUND8(x)  ( ROUND4(x) | (ROUND4(x)  >> 8 ) )
#define ROUND16(x) ( ROUND8(x) | (ROUND8(x)  >> 16) )
#define ROUND32(x) ( ROUND16(x)| (ROUND16(x) >> 32) )
#if ULONG_MAX <= UINT32_MAX
#define ROUND_UP_NEXT_POWER_OF_2(x) (ROUND16(x-1) + 1)
#elif ULONG_MAX <= UINT64_MAX
#else
#error The size_t on your platform is too large, Please add an additionnal round in the macro ROUND_UP_NEXT_POWER_OF_2
#endif

// 1 MiB
#define SHIFT_1MB               (20U) /**<  SIZE_4KB is 1 << SHIFT_4KB aka. 2^SHIFT_1MB. */
#define SIZE_1MB                (1 << SHIFT_1MB) /**< Size of 1 KiB. */
#define MASK_1MB                (SIZE_1MB-1) /**< mask to get 0 to SIZE_1MB-1. */
#define PTR_OFFS_1MB(p)         PTR2VAL(PTR_MASK(p, MASK_1MB))  /* offset from 1 MiB address. */
#define IS_ALIGNED_1MB(p)       (0 == PTR_OFFS_1MB(p)) /* is aligned on 1 MiB address. */
#define PTR_ALIGN_DOWN_1MB(p)   PTR_MASK(p, ~MASK_1MB) /* get next lower 1 MiB address. */
#define PTR_ALIGN_UP_1MB(p)     PTR_ALIGN_DOWN_1MB( PTR_OFFS(p, SIZE_1MB-1) ) /* get next higher 1 MiB address. */
#define ALIGN_DOWN_1MB(v)       PTR2VAL( PTR_ALIGN_DOWN_1MB( VAL2PTR(v) ) ) /* get next lower 1 MiB value. */
#define ALIGN_UP_1MB(v)         PTR2VAL( PTR_ALIGN_UP_1MB( VAL2PTR(v) ) ) /* get next higher 1 MiB value. */

// 2 MiB
#define ALIGN_UP_2MB(v)         ( ALIGN_UP_1MB(v>>1) << 1 ) /* get next higher 2 MiB value. */

#endif /* _DEVICE_H_ */
