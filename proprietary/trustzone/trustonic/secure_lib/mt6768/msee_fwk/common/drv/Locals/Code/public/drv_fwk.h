/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __MT_DRV_FWK_H__
#define __MT_DRV_FWK_H__

#include <drv_defs.h>

#define STR_LEN_MAX 32

/* Callbacks that should be defined in secure driver */
struct msee_drv_device {
    unsigned char name[STR_LEN_MAX];
    unsigned int driver_id;
    int (*init)(const struct msee_drv_device *dev);
    int (*invoke_command)(unsigned int sid, unsigned int cmd, unsigned long args);
    int (*ree_command)(unsigned int cmd, unsigned long args);
    int (*open_session)(unsigned int sid, unsigned long args);
    int (*close_session)(unsigned int sid);
};
typedef struct msee_drv_device msee_drv_device_t;

/* System API of driver framework */

/* Memory API of driver framework */
#define MSEE_MAP_READABLE            (1U << 0) /* read access attribute */
#define MSEE_MAP_WRITABLE            (1U << 1) /* write access attribute */
#define MSEE_MAP_EXECUTABLE          (1U << 2) /* program execution attribute */
#define MSEE_MAP_UNCACHED            (1U << 3) /* uncached memory access attribute */
#define MSEE_MAP_IO                  (1U << 4) /* device memory attribute */
#define MSEE_MAP_ALLOW_NONSECURE     (1U << 7) /* non-secure attribute, used only to map client task buffers using msee_map_user */
#define MSEE_MAP_HARDWARE            (MSEE_MAP_READABLE | MSEE_MAP_WRITABLE | MSEE_MAP_IO)
#define MSEE_MAP_USER_DEFAULT        (MSEE_MAP_READABLE | MSEE_MAP_WRITABLE | MSEE_MAP_ALLOW_NONSECURE)
void msee_clean_dcache_all(void);
void msee_clean_invalidate_dcache_all(void);
void msee_clean_dcache_range(unsigned long addr, unsigned long size);
void msee_clean_invalidate_dcache_range(unsigned long addr, unsigned long size);
MSEE_Result msee_map_user(void **to, const void *from, unsigned long size, unsigned int flags);
MSEE_Result msee_unmap_user(void *to);

#if MEMORY_MONITOR
MSEE_Result msee_mmap_region_ext(unsigned long long pa, void **va, unsigned long size, unsigned int flags, const char *func, const int line);
MSEE_Result msee_unmmap_region_ext(void *va, unsigned long size, const char *func, const int line);
void* msee_malloc_ext(unsigned long size, const char *func, const int line);
void* msee_realloc_ext(void *buf, unsigned long size, const char *func, const int line);
void msee_free_ext(void *buf, const char *func, const int line);

#define msee_mmap_region(pa, va, size, flags) msee_mmap_region_ext(pa, va, size, flags, __FUNCTION__, __LINE__)
#define msee_unmmap_region(va, size) msee_unmmap_region_ext(va, size, __FUNCTION__, __LINE__)
#define msee_malloc(size) msee_malloc_ext(size, __FUNCTION__, __LINE__)
#define msee_realloc(buf, size) msee_realloc_ext(buf, size, __FUNCTION__, __LINE__)
#define msee_free(buf) msee_free_ext(buf, __FUNCTION__, __LINE__)

#else
MSEE_Result msee_mmap_region(unsigned long long pa, void **va, unsigned long size, unsigned int flags);
MSEE_Result msee_unmmap_region(void *va, unsigned long size);
void* msee_malloc(unsigned long size);
void* msee_realloc(void *buf, unsigned long size);
void msee_free(void *buf);
#endif

/* Thread API of driver framework */

/* Interrupt API of driver framework */
#define MSEE_INTR_MODE_MASK_TRIGGER		(1U<<0)
#define MSEE_INTR_MODE_TRIGGER_LEVEL		MSEE_INTR_MODE_MASK_TRIGGER
#define MSEE_INTR_MODE_TRIGGER_EDGE		0
#define MSEE_INTR_MODE_MASK_CONDITION		(1U<<1)
#define MSEE_INTR_MODE_CONDITION_FALLING	MSEE_INTR_MODE_MASK_CONDITION
#define MSEE_INTR_MODE_CONDITION_LOW		MSEE_INTR_MODE_MASK_CONDITION
#define MSEE_INTR_MODE_CONDITION_RISING		0
#define MSEE_INTR_MODE_CONDITION_HIGH		0
#define MSEE_INTR_MODE_RAISING_EDGE		(MSEE_INTR_MODE_TRIGGER_EDGE | MSEE_INTR_MODE_CONDITION_RISING)
#define MSEE_INTR_MODE_FALLING_EDGE		(MSEE_INTR_MODE_TRIGGER_EDGE | MSEE_INTR_MODE_CONDITION_FALLING)
#define MSEE_INTR_MODE_LOW_LEVEL		(MSEE_INTR_MODE_TRIGGER_LEVEL | MSEE_INTR_MODE_CONDITION_LOW)
#define MSEE_INTR_MODE_HIGH_LEVEL		(MSEE_INTR_MODE_TRIGGER_LEVEL | MSEE_INTR_MODE_CONDITION_HIGH)
enum MSEE_IRQ_STATUS {
    MSEE_IRQ_TIMEOUT = -1,
    MSEE_IRQ_FAIL = -2,
    MSEE_IRQ_MAX_ERROR = 0xFFFFFFFF,
};
typedef int (*msee_irq_handler_t)(int, void *);
MSEE_Result msee_request_irq(unsigned int irq, msee_irq_handler_t handler, unsigned long flags, unsigned int timeout_ms /*ms*/, void *data);
void msee_free_irq(unsigned int irq);
MSEE_Result msee_wait_for_irq_complete(void);

/* IPC API of driver framework */
void* msee_get_ree_shared_msg(void);
int msee_notify_ree(void);

/* Variadic args Log API of driver framework */
void msee_printf_va(const char *format, ...);

#define msee_printf(...)    do{}while(0);                  /* empty implementation to force change API */

#if defined(__INCLUDE_DEBUG) || defined(DEBUG)
    #define msee_logd(...)  msee_printf_va(__VA_ARGS__)    /* debug message */
    #define msee_loge(...)  msee_printf_va(__VA_ARGS__)    /* error message */
#else
    #define msee_logd(...)  do{}while(0)                   /* debug message */
    #define msee_loge(...)  msee_printf_va(__VA_ARGS__)    /* error message */
#endif // __INCLUDE_DEBUG

/* Time API of driver framework */
struct msee_time {
    unsigned int s;
    unsigned int ms;
};
typedef struct msee_time msee_time_t;
void msee_get_system_time(msee_time_t *time);

/* Utils API of driver framework */
char* msee_strncpy(char *dest, const char *src, unsigned long n);
unsigned long msee_strlen(const char *str);
void *msee_memcpy(void *dest, const void *src, unsigned long n);
void *msee_memset(void *s, int c, unsigned long n);

/* SMC Call API */
int msee_smc_call(uint32_t smc_nr, uint32_t args0, uint32_t args1,
                  uint32_t args2, uint32_t *smc_ret);

/* TEE trace API */
#define MSEE_TRACING_ENABLED	0
void msee_trace_start(char *name);
void msee_trace_end(void);

#endif //__MT_DRV_FWK_H__

