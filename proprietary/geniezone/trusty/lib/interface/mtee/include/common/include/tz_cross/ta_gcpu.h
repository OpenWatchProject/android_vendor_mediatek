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

#ifndef __TRUSTZONE_TA_GCPU__
#define __TRUSTZONE_TA_GCPU__

#define TZ_TA_GCPU_UUID   "7b6c51b8-2994-4a32-be68-d840df0281bb"

/* Data Structure for GCPU TA */
/* You should define data structure used both in REE/TEE here
   N/A for GCPU TA */

/* Command for GCPU TA */
#define TZCMD_GCPU_SELFTEST                 0
#define TZCMD_GCPU_SUSPEND                  1
#define TZCMD_GCPU_KERNEL_INIT_DONE         2
#define TZCMD_GCPU_AES_CBC_PERFORMANCE      3

#endif              /* __TRUSTZONE_TA_GCPU__ */
