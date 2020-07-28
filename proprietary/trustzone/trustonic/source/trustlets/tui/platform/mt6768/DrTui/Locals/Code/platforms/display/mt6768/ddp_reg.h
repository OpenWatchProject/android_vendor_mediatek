/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _DDP_REG_H_
#define _DDP_REG_H_

#include "cmdq_sec_record.h"
#include "ddp_hal.h"
#include "ddp_path.h"
#include "ddp_dsi.h"
#include "ddp_drv.h"

#include "ddp_reg_mmsys.h"
#include "ddp_reg_mutex.h"
#include "ddp_reg_ovl.h"
#include "ddp_reg_pq.h"
#include "ddp_reg_dma.h"
#include "ddp_reg_dsi.h"
#include "ddp_reg_mipi.h"
#include "ddp_reg_rsz.h"

#define ENABLE_CLK_MGR

#define UINT32 unsigned int

/* ////////////////////////////// macro //////////////////////////// */
#ifndef READ_REGISTER_UINT32
	#define READ_REGISTER_UINT32(reg) (*(UINT32 * const)(reg))
#endif
#ifndef WRITE_REGISTER_UINT32
	#define WRITE_REGISTER_UINT32(reg, val) \
		((*(UINT32 * const)(reg)) = (val))
#endif
#ifndef READ_REGISTER_UINT16
	#define READ_REGISTER_UINT16(reg) ((*(UINT16 * const)(reg)))
#endif
#ifndef WRITE_REGISTER_UINT16
	#define WRITE_REGISTER_UINT16(reg, val) \
		((*(UINT16 * const)(reg)) = (val))
#endif
#ifndef READ_REGISTER_UINT8
	#define READ_REGISTER_UINT8(reg) ((*(UINT8 * const)(reg)))
#endif
#ifndef WRITE_REGISTER_UINT8
	#define WRITE_REGISTER_UINT8(reg, val) \
		((*(UINT8 * const)(reg)) = (val))
#endif

#define INREG8(x) READ_REGISTER_UINT8((UINT8 *)((void *)(x)))
#define OUTREG8(x, y) \
	WRITE_REGISTER_UINT8((UINT8 *)((void *)(x)), (UINT8)(y))
#define SETREG8(x, y) OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y) OUTREG8(x, INREG8(x)&~(y))
#define MASKREG8(x, y, z) OUTREG8(x, (INREG8(x)&~(y))|(z))

#define INREG16(x) READ_REGISTER_UINT16((UINT16 *)((void *)(x)))
#define OUTREG16(x, y) \
	WRITE_REGISTER_UINT16((UINT16 *)((void *)(x)), (UINT16)(y))
#define SETREG16(x, y) OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y) OUTREG16(x, INREG16(x)&~(y))
#define MASKREG16(x, y, z) OUTREG16(x, (INREG16(x)&~(y))|(z))

#define INREG32(x) READ_REGISTER_UINT32((UINT32 *)((void *)(x)))
#define OUTREG32(x, y) \
	WRITE_REGISTER_UINT32((UINT32 *)((void *)(x)), (UINT32)(y))
#define SETREG32(x, y) OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y) OUTREG32(x, INREG32(x)&~(y))
#define MASKREG32(x, y, z) OUTREG32(x, (INREG32(x)&~(y))|(z))

#ifndef ASSERT
	#define ASSERT(expr)	WARN_ON(!(expr))
#endif

#define AS_INT32(x)     (*(INT32 *)((void *)x))
#define AS_INT16(x)     (*(INT16 *)((void *)x))
#define AS_INT8(x)      (*(INT8  *)((void *)x))

#define AS_UINT32(x)    (*(UINT32 *)((void *)x))
#define AS_UINT16(x)    (*(UINT16 *)((void *)x))
#define AS_UINT8(x)     (*(UINT8  *)((void *)x))

#ifndef FALSE
  #define FALSE (0)
#endif

#ifndef TRUE
  #define TRUE  (1)
#endif

#define DISP_RDMA_INDEX_OFFSET  (dispsys_reg[DISP_REG_RDMA1] - dispsys_reg[DISP_REG_RDMA0])
#define DISP_WDMA_INDEX_OFFSET  (0)
#define DISP_OVL_INDEX_OFFSET   (0)
#define DISP_MIPI_INDEX_OFFSET  (0)

/* SMI_LARB0 */
#define DISP_REG_SMI_LARB0_NON_SEC_CON (DISPSYS_SMI_LARB0_BASE+0x380)
#define DISP_REG_SMI_LARB0_SEC_CON (DISPSYS_SMI_LARB0_BASE+0xf80)

#define DDP_REG_BASE_MMSYS_CONFIG  dispsys_reg[DISP_REG_CONFIG]
#define DDP_REG_BASE_DISP_OVL0     dispsys_reg[DISP_REG_OVL0]
#define DDP_REG_BASE_DISP_OVL0_2L  dispsys_reg[DISP_REG_OVL0_2L]
#define DDP_REG_BASE_DISP_RDMA0    dispsys_reg[DISP_REG_RDMA0]
#define DDP_REG_BASE_DISP_WDMA0    dispsys_reg[DISP_REG_WDMA0]
#define DDP_REG_BASE_DISP_COLOR0   dispsys_reg[DISP_REG_COLOR0]
#define DDP_REG_BASE_DISP_CCORR0   dispsys_reg[DISP_REG_CCORR0]
#define DDP_REG_BASE_DISP_AAL0     dispsys_reg[DISP_REG_AAL0]
#define DDP_REG_BASE_DISP_GAMMA0   dispsys_reg[DISP_REG_GAMMA0]
#define DDP_REG_BASE_DISP_DITHER0  dispsys_reg[DISP_REG_DITHER0]
#define DDP_REG_BASE_DSI0          dispsys_reg[DISP_REG_DSI0]
#define DDP_REG_BASE_DSI1          dispsys_reg[DISP_REG_DSI1]
#define DDP_REG_BASE_DISP_RSZ0     dispsys_reg[DISP_REG_RSZ0]
#define DDP_REG_BASE_DISP_PWM0     dispsys_reg[DISP_REG_PWM0]
#define DDP_REG_BASE_MM_MUTEX      dispsys_reg[DISP_REG_MUTEX]
#define DDP_REG_BASE_SMI_LARB0     dispsys_reg[DISP_REG_SMI_LARB0]
#define DDP_REG_BASE_SMI_LARB1     dispsys_reg[DISP_REG_SMI_LARB1]
#define DDP_REG_BASE_SMI_COMMON    dispsys_reg[DISP_REG_SMI_COMMON]
#define DDP_REG_BASE_MIPITX0       dispsys_reg[DISP_REG_MIPI0]
#define DDP_REG_BASE_MIPITX1       dispsys_reg[DISP_REG_MIPI1]

#define DISPSYS_CONFIG_BASE			DDP_REG_BASE_MMSYS_CONFIG
#define DISPSYS_OVL0_BASE		        DDP_REG_BASE_DISP_OVL0
#define DISPSYS_OVL0_2L_BASE		    DDP_REG_BASE_DISP_OVL0_2L
#define DISPSYS_RDMA0_BASE		        DDP_REG_BASE_DISP_RDMA0
#define DISPSYS_WDMA0_BASE		        DDP_REG_BASE_DISP_WDMA0
#define DISPSYS_COLOR0_BASE		        DDP_REG_BASE_DISP_COLOR0
#define DISPSYS_CCORR0_BASE		        DDP_REG_BASE_DISP_CCORR0
#define DISPSYS_AAL0_BASE		        DDP_REG_BASE_DISP_AAL0
#define DISPSYS_GAMMA0_BASE		        DDP_REG_BASE_DISP_GAMMA0
#define DISPSYS_DITHER0_BASE		    DDP_REG_BASE_DISP_DITHER0
#define DISPSYS_DSI0_BASE		        DDP_REG_BASE_DSI0
#define DISPSYS_DSI1_BASE		        DDP_REG_BASE_DSI1
#define DISPSYS_RSZ0_BASE				DDP_REG_BASE_DISP_RSZ0
#define DISPSYS_PWM0_BASE		        DDP_REG_BASE_DISP_PWM0
#define DISPSYS_MUTEX_BASE			DDP_REG_BASE_MM_MUTEX
#define DISPSYS_SMI_LARB0_BASE		    DDP_REG_BASE_SMI_LARB0
#define DISPSYS_SMI_LARB1_BASE		    DDP_REG_BASE_SMI_LARB1
#define DISPSYS_SMI_COMMON_BASE		    DDP_REG_BASE_SMI_COMMON
#define DISPSYS_MIPITX0_BASE            DDP_REG_BASE_MIPITX0
#define DISPSYS_MIPITX1_BASE            DDP_REG_BASE_MIPITX1
#define DISP_REG_SMI_LARB1_NON_SEC_CON (DISPSYS_SMI_LARB1_BASE+0x380)
#define DISP_REG_SMI_LARB1_SEC_CON (DISPSYS_SMI_LARB1_BASE+0xf80)

/* --------------------------------------------------------------------------- */
/* Register Field Access */
/* --------------------------------------------------------------------------- */

#define REG_FLD(width, shift) \
	((unsigned int)((((width) & 0xFF) << 16) | ((shift) & 0xFF)))

#define REG_FLD_MSB_LSB(msb, lsb) REG_FLD((msb) - (lsb) + 1, (lsb))

#define REG_FLD_WIDTH(field) \
	((unsigned int)(((field) >> 16) & 0xFF))

#define REG_FLD_SHIFT(field) \
	((unsigned int)((field) & 0xFF))

#define REG_FLD_MASK(field) \
	((unsigned int)((1ULL << REG_FLD_WIDTH(field)) - 1) << REG_FLD_SHIFT(field))

#define REG_FLD_VAL(field, val) \
	(((val) << REG_FLD_SHIFT(field)) & REG_FLD_MASK(field))

#define REG_FLD_VAL_GET(field, regval) \
	(((regval) & REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field))

#define mt_reg_sync_writel(val, reg32)	(*(volatile unsigned int*)(reg32) = (val))
#define DISP_REG_GET(reg32) 	(*(volatile unsigned int*)(reg32))
#define DISP_REG_GET_FIELD(field, reg32) ((*(volatile unsigned int*)(reg32)\
		& REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field))


/* polling register until masked bit is 1 */
#define DDP_REG_POLLING(reg32, mask) \
	do { \
		while (!((DISP_REG_GET(reg32))&mask))\
			; \
	} while (0)

/* Polling register until masked bit is 0 */
#define DDP_REG_POLLING_NEG(reg32, mask) \
	do { \
		while ((DISP_REG_GET(reg32))&mask)\
			; \
	} while (0)

#define DISP_CPU_REG_SET(reg32, val) \
	do {\
		if (0) \
		mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
	} while (0)

/* after apply device tree va/pa is not mapped by a fixed offset */
static inline unsigned long disp_addr_convert(unsigned long va)
{
	unsigned int i = 0;

	for (i = 0; i < DISP_REG_NUM; i++) {
		if (dispsys_reg[i] == (va & (~0xfffl)))
			return ddp_reg_pa_base[i] + (va & 0xfffl);
	}
	DDPAEE("DDP/can not find reg addr for va=0x%lx!\n", va);
	return 0;
}

#define DISP_REG_MASK_EXT(handle, reg_va, reg_pa, val, mask) 	\
	do { \
	   if(handle==NULL) \
	   { \
			 mt_reg_sync_writel((unsigned int)(DISP_REG_GET(reg_va)&~(mask))|(val),(volatile void*)(reg_va) );\
	   } \
	   else \
	   { \
			 cmdqRecWrite(handle, reg_pa, val, mask); \
	   }  \
	} while (0)


#define DISP_REG_MASK(handle, reg32, val, mask)	\
	do { \
		 if (handle == NULL) { \
			mt_reg_sync_writel((unsigned int)(INREG32(reg32)&~(mask))|(val), (reg32));\
		 } else { \
			cmdqRecWrite(handle, disp_addr_convert((unsigned long)(reg32)), val, mask); \
		 }	\
	} while (0)

#define DISP_REG_SET_PA(handle,reg32, val) \
	do { \
	  if(handle==0) \
	  { \
		 mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
	  } \
	  else \
	  { \
		 cmdqRecWrite(handle, reg32, val, ~0); \
	  }  \
	} while (0)


#define DISP_REG_SET(handle, reg32, val) \
	do { \
		if (handle == NULL) { \
			mt_reg_sync_writel(val, (volatile unsigned long*)(reg32));\
		} else { \
			cmdqRecWrite(handle, disp_addr_convert((unsigned long)(reg32)), val, ~0); \
		}  \
	} while (0)


#define DISP_REG_SET_FIELD(handle, field, reg32, val)  \
	do {  \
		if (handle == NULL) { \
			unsigned int regval; \
			regval = (*(volatile unsigned int*)(reg32)); \
			regval  = regval & ~REG_FLD_MASK(field) | REG_FLD_VAL((field), (val)); \
			mt_reg_sync_writel(regval, (reg32));  \
		} else { \
			cmdqRecWrite(handle, disp_addr_convert(reg32), val<<REG_FLD_SHIFT(field), REG_FLD_MASK(field));\
		} \
	} while (0)

#define DISP_REG_CMDQ_POLLING(handle, reg32, val, mask) \
	do { \
		if (handle == NULL) { \
			while (!((DISP_REG_GET(reg32))&val))\
				; \
		} else { \
			cmdqRecPoll(handle, disp_addr_convert((unsigned long)(reg32)), val, mask); \
		}  \
	} while (0)

#define DISP_REG_BACKUP(handle, hSlot, idx, reg32) \
	do { \
		if (handle != NULL) { \
			if (hSlot) \
				cmdqRecBackupRegisterToSlot(handle, hSlot, idx,\
							    disp_addr_convert((unsigned long)(reg32)));\
		}  \
	} while (0)


#endif
