/*
 * Copyright (c) 2014 - 2018 MediaTek Inc.
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

#ifndef __MT_EMI_MPU_H__
#define __MT_EMI_MPU_H__

#define EMI_MPU_BASE			(0x10226000)

/* EMI Memory Protect Unit */
#if 1
#define EMI_MPUA                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0160))
#define EMI_MPUB                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0168))
#define EMI_MPUC                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0170))
#define EMI_MPUD                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0178))
#define EMI_MPUE                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0180))
#define EMI_MPUF                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0188))
#define EMI_MPUG                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0190))
#define EMI_MPUH                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0198))
#define EMI_MPUA2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0260))
#define EMI_MPUB2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0268))
#define EMI_MPUC2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0270))
#define EMI_MPUD2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0278))
#define EMI_MPUE2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0280))
#define EMI_MPUF2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0288))
#define EMI_MPUG2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0290))
#define EMI_MPUH2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0298))
#define EMI_MPUA3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0360))
#define EMI_MPUB3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0368))
#define EMI_MPUC3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0370))
#define EMI_MPUD3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0378))

#define EMI_MPUI                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01A0))
#define EMI_MPUI_2ND            ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01A4))
#define EMI_MPUJ                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01A8))
#define EMI_MPUJ_2ND            ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01AC))
#define EMI_MPUK                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01B0))
#define EMI_MPUK_2ND            ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01B4))
#define EMI_MPUL                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01B8))
#define EMI_MPUL_2ND            ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01BC))
#define EMI_MPUI2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02A0))
#define EMI_MPUI2_2ND           ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02A4))
#define EMI_MPUJ2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02A8))
#define EMI_MPUJ2_2ND           ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02AC))
#define EMI_MPUK2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02B0))
#define EMI_MPUK2_2ND           ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02B4))
#define EMI_MPUL2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02B8))
#define EMI_MPUL2_2ND           ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02BC))
#define EMI_MPUI3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x03A0))
#define EMI_MPUJ3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x03A8))
#define EMI_MPUK3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x03B0))
#define EMI_MPUL3               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x03B8))

#define EMI_MPUM                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01C0))
#define EMI_MPUN                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01C8))
#define EMI_MPUO                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01D0))
#define EMI_MPUU                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0200))
#define EMI_MPUM2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02C0))
#define EMI_MPUN2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02C8))
#define EMI_MPUO2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02D0))
#define EMI_MPUU2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0300))

#define EMI_MPUP                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01D8))
#define EMI_MPUQ                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01E0))
#define EMI_MPUR                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01E8))
#define EMI_MPUS                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01F0))
#define EMI_MPUT                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x01F8))
#define EMI_MPUY                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0220))

#define EMI_MPUP2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02D8))
#define EMI_MPUQ2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02E0))
#define EMI_MPUR2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x02E8))
#define EMI_MPUY2               ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0320))
#endif

#define EMI_MPU_A(region)		((volatile unsigned int)(EMI_MPUA + (region*8)))
#define EMI_MPU_APC(region,dgroup)	((volatile unsigned int)(EMI_MPUI + (region*8) +(dgroup*0x100)))
#if 0
#define EMI_MPU_CTRL			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x000))
#define EMI_MPU_DBG			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x004))
#define EMI_MPU_SA0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x100))
#define EMI_MPU_EA0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x200))
#define EMI_MPU_SA(region)		((volatile unsigned int)(EMI_MPU_SA0 + (region*4)))
#define EMI_MPU_EA(region)		((volatile unsigned int)(EMI_MPU_EA0 + (region*4)))
#define EMI_MPU_APC0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x300))
#define EMI_MPU_APC(region,dgroup)	((volatile unsigned int)(EMI_MPU_APC0 + (region*4) +(dgroup*0x100)))
#define EMI_MPU_CTRL_D0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x800))
#define EMI_MPU_CTRL_D(domain)		((volatile unsigned int)(EMI_MPU_CTRL_D0 + (domain*4)))
#define EMI_RG_MASK_D0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x900))
#define EMI_RG_MASK_D(domain)		((volatile unsigned int)(EMI_RG_MASK_D0 + (domain*4)))
#endif

#define EMI_MPU_DOMAIN_NUM	8
#define EMI_MPU_REGION_NUM	18
#define EMI_MPU_ALIGN_BITS	16
#define DRAM_OFFSET		(0x40000000 >> EMI_MPU_ALIGN_BITS)

#define MAX_CH	2
#define MAX_RK	2

#define SHARED_SECURE_MEMORY_MPU_REGION_ID	2
#define TUI_MEMORY_MPU_REGION_ID		3

#include "../../../../common/drv/public/emi_mpu_v1.h"

#endif // __MT_EMI_MPU_H__
