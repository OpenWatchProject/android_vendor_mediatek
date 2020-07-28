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

#include <msee_fwk/drv_defs.h>

/* Callbacks that should be defined in secure driver */
struct msee_drv_device {
    unsigned char name[32];
    unsigned int driver_id;
    int (*init)(const struct msee_drv_device *dev);
    int (*invoke_command)(unsigned int sid, unsigned int cmd, unsigned long args);
    int (*ree_command)(unsigned int cmd, unsigned long args); /* not used */
    int (*open_session)(unsigned int sid, unsigned long args);
    int (*close_session)(unsigned int sid);
};
typedef struct msee_drv_device msee_drv_device_t;


void msee_clean_dcache_all(void);
void msee_clean_invalidate_dcache_all(void);
void msee_clean_dcache_range(unsigned long addr, unsigned long size);
void msee_clean_invalidate_dcache_range(unsigned long addr, unsigned long size);


MSEE_Result msee_map_user(void **to, const void *from, unsigned long size, unsigned int flags);
MSEE_Result msee_unmap_user(void *to);
MSEE_Result msee_mmap_region(unsigned long long pa, void **va, unsigned long size, unsigned int flags);
MSEE_Result msee_unmmap_region(void *va, unsigned long size);

void* msee_malloc(unsigned long size);
void* msee_realloc(void *buf, unsigned long size);
void msee_free(void *buf);

void* msee_memcpy(void *dest, const void *src, unsigned long n);
void* msee_memset(void *s, int c, unsigned long n);


typedef int (*msee_irq_handler_t)(int, void *);

MSEE_Result msee_request_irq(unsigned int irq, msee_irq_handler_t handler, unsigned long flags, unsigned int timeout_ms /*ms*/, void *data);
void msee_free_irq(unsigned int irq);
MSEE_Result msee_wait_for_irq_complete(void);


#endif //__MT_DRV_FWK_H__

