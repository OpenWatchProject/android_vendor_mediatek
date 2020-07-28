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

/* Power management TA functions
 */

#ifndef __TRUSTZONE_TA_PM__
#define __TRUSTZONE_TA_PM__

#define TZ_TA_PM_UUID   "387389fa-b2cf-11e2-856d-d485645c4310"

/* Command for PM TA */

#define TZCMD_PM_CPU_LOWPOWER     0
#define TZCMD_PM_CPU_DORMANT      1
#define TZCMD_PM_DEVICE_OPS       2
#define TZCMD_PM_CPU_ERRATA_802022_WA    3

enum eMTEE_PM_State
{
    MTEE_NONE,
    MTEE_SUSPEND,
    MTEE_SUSPEND_LATE,
    MTEE_RESUME,
    MTEE_RESUME_EARLY,
};
typedef enum eMTEE_PM_State MTEE_PM_State;

#endif /* __TRUSTZONE_TA_PM__ */
