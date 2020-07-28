/*
 * Copyright (c) 2019 MediaTek Inc.
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
#ifndef __IMGSENSOR_SEC_TYPEDEF_H__
#define __IMGSENSOR_SEC_TYPEDEF_H__

#include<drv_call.h>

/*typedef for struct*/
#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif

#ifndef NULL
#define NULL  (0)
#endif

#ifndef uintptr_t
typedef unsigned int uintptr_t;
#endif

#ifndef BOOL
typedef unsigned char BOOL;
#endif

#ifndef bool
typedef unsigned char bool;
#endif

#undef pr_debug
#define pr_debug msee_ta_loge

#undef pr_info
#define pr_info msee_ta_loge

#ifndef PFX
#define PFX "SECURE_SENSOR"
#endif

#undef LOG_INF/*define msee_ta_loge in secure world*/
#define LOG_INF(fmt, args...)\
	msee_ta_loge(PFX"_TEE" "[%s] " fmt, __FUNCTION__, ##args)

/*Due to sensor develop in tee environment, lock is useless*/

#undef spin_lock(arg) /*now tee is single thread*/
#define spin_lock(arg)

#undef spin_unlock(arg) /*now tee is single thread*/
#define spin_unlock(arg)

#undef spin_lock_irqsave(arg1,arg2) /*now tee is single thread*/
#define spin_lock_irqsave(arg1,arg2)

#undef spin_unlock_irqrestore(arg1,arg2) /*now tee is single thread*/
#define spin_unlock_irqrestore(arg1,arg2)

#undef mDELAY
//#define mDELAY(arg) sleep(arg)// microtrust
#define mDELAY(arg) TEE_Wait(arg)

#undef mdelay
#define mdelay(arg) TEE_Wait(arg)

#undef imgsensor_drv_lock /*now tee is single thread*/
#define imgsensor_drv_lock

#endif
