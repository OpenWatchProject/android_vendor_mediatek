/*
* Copyright (c) 2015 MediaTek Inc.
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

/* IRQ/FIQ TA functions
 */

#ifndef __TRUSTZONE_TA_IRQ__
#define __TRUSTZONE_TA_IRQ__

#define TZ_TA_IRQ_UUID   "0be96652-d723-11e2-b1a4-d485645c4310"

/* Command for IRQ TA */
#define TZCMD_IRQ_SET_FIQ         0
#define TZCMD_IRQ_ENABLE_FIQ      1
#define TZCMD_IRQ_GET_INTACK      2
#define TZCMD_IRQ_EOI             3
#define TZCMD_IRQ_TRIGGER_SGI     4
#define TZCMD_IRQ_MASK_ALL        5
#define TZCMD_IRQ_MASK_RESTORE    6
#define TZCMD_IRQ_QUERY_FIQ       7


/* TZ Flags for TZCMD_IRQ_SET_FIQ */
#define TZ_IRQF_EDGE_SENSITIVE    0
#define TZ_IRQF_LEVEL_SENSITIVE   1
#define TZ_IRQF_LOW               0
#define TZ_IRQF_HIGH              2

#endif /* __TRUSTZONE_TA_IRQ__ */
