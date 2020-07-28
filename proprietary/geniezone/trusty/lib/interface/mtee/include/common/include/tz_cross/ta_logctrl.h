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

#ifndef __TA_LOG_CTRL_H__
#define __TA_LOG_CTRL_H__

#define TZ_TA_LOG_CTRL_UUID   "a80ef6e1-de27-11e2-a28f-0800200c9a66"

/* should match MTEE_LOG_LVL in log.h */
#define MTEE_LOG_CTRL_LVL_INFO    0x00000000
#define MTEE_LOG_CTRL_LVL_DEBUG   0x00000001
#define MTEE_LOG_CTRL_LVL_PRINTF  0x00000002
#define MTEE_LOG_CTRL_LVL_WARN    0x00000003
#define MTEE_LOG_CTRL_LVL_BUG     0x00000004
#define MTEE_LOG_CTRL_LVL_ASSERT  0x00000005
#define MTEE_LOG_CTRL_LVL_DISABLE 0x0000000f

/* Command for Test TA */
#define TZCMD_LOG_CTRL_SET_LVL   0

#endif /* __TA_LOG_CTRL_H__ */
