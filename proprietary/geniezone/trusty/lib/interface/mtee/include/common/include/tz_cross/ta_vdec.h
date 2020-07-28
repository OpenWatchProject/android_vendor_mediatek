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

#ifndef __TRUSTZONE_TA_VDEC__
#define __TRUSTZONE_TA_VDEC__

#define TZ_TA_VDEC_UUID   "ff33a6e0-8635-11e2-9e96-0800200c9a66"

#define UT_ENABLE 0
#define DONT_USE_BS_VA 1  // for VP path integration set to 1,  for mfv_ut set to 0
#define USE_MTEE_M4U
#define USE_MTEE_DAPC


/* Command for VDEC TA */
#define TZCMD_VDEC_AVC_INIT       0
#define TZCMD_VDEC_AVC_DECODE     1
#define TZCMD_VDEC_AVC_DEINIT      2
#define TZCMD_VDEC_HEVC_INIT       3
#define TZCMD_VDEC_HEVC_DECODE     4
#define TZCMD_VDEC_HEVC_DEINIT     5
#define TZCMD_VDEC_FILL_SECMEM      6

#define TZCMD_VDEC_TEST        100
#endif /* __TRUSTZONE_TA_VDEC__ */
