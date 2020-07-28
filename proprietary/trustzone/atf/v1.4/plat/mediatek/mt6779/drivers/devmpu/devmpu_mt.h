/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __DEVMPU_MT_H__
#define __DEVMPU_MT_H__

/**
 * EMI & Security AO HW registers
 */
#define EMI_BASE        (0x10219000)
#define EMI_CONM        ((uintptr_t)(EMI_BASE + 0x060))
#define EMI_TESTC       ((uintptr_t)(EMI_BASE + 0x0F0))
#define EMI_MPUT_2ND    ((uintptr_t)(EMI_BASE + 0x1FC))

#define SECURITY_AO     (0x1001A000)

/**
 * DeviceMPU platform HW registers
 */
#define DEVMPU_BASE     (0x1021A000)

#define DEVMPU_UPDATE_DONE                      ((uintptr_t)(DEVMPU_BASE + 0x0000))
#define DEVMPU_UPDATE_START                     ((uintptr_t)(DEVMPU_BASE + 0x0004))
#define DEVMPU_RD_PERMI_TABLE_ADDR_L            ((uintptr_t)(DEVMPU_BASE + 0x0008))
#define DEVMPU_RD_PERMI_TABLE_ADDR_H            ((uintptr_t)(DEVMPU_BASE + 0x000C))
#define DEVMPU_WR_PERMI_TABLE_ADDR_L            ((uintptr_t)(DEVMPU_BASE + 0x0010))
#define DEVMPU_WR_PERMI_TABLE_ADDR_H            ((uintptr_t)(DEVMPU_BASE + 0x0014))
#define DEVMPU_RD_INVAL_TABLE_ADDR_L            ((uintptr_t)(DEVMPU_BASE + 0x0018))
#define DEVMPU_RD_INVAL_TABLE_ADDR_H            ((uintptr_t)(DEVMPU_BASE + 0x001C))
#define DEVMPU_WR_INVAL_TABLE_ADDR_L            ((uintptr_t)(DEVMPU_BASE + 0x0020))
#define DEVMPU_WR_INVAL_TABLE_ADDR_H            ((uintptr_t)(DEVMPU_BASE + 0x0024))
#define DEVMPU_RD_INVAL_TABLE_UPDATE_BIT_0      ((uintptr_t)(DEVMPU_BASE + 0x0028))
#define DEVMPU_RD_INVAL_TABLE_UPDATE_BIT_1      ((uintptr_t)(DEVMPU_BASE + 0x002C))
#define DEVMPU_WR_INVAL_TABLE_UPDATE_BIT_0      ((uintptr_t)(DEVMPU_BASE + 0x0038))
#define DEVMPU_WR_INVAL_TABLE_UPDATE_BIT_1      ((uintptr_t)(DEVMPU_BASE + 0x003C))
#define DEVMPU_AXI_ARCACHE_TYPE                 ((uintptr_t)(DEVMPU_BASE + 0x0048))
#define DEVMPU_AXI_ARDOMAIN                     ((uintptr_t)(DEVMPU_BASE + 0x004C))
#define DEVMPU_AXI_ARPROT                       ((uintptr_t)(DEVMPU_BASE + 0x0050))
#define DEVMPU_AXI_ARID                         ((uintptr_t)(DEVMPU_BASE + 0x0054))
#define DEVMPU_AXI_ARSIZE                       ((uintptr_t)(DEVMPU_BASE + 0x0058))
#define DEVMPU_CLEAR_DONE_FLAG                  ((uintptr_t)(DEVMPU_BASE + 0x005C))
#define DEVMPU_DISABLE                          ((uintptr_t)(SECURITY_AO + 0x060C))
#define DEVMPU_IMR                              ((uintptr_t)(DEVMPU_BASE + 0x0064))
#define DEVMPU_ISR                              ((uintptr_t)(DEVMPU_BASE + 0x0068))
#define DEVMPU_ICR                              ((uintptr_t)(DEVMPU_BASE + 0x006C))
#define DEVMPU_CON                              ((uintptr_t)(DEVMPU_BASE + 0x0070))
#define DEVMPU_VIO_ADDR                         ((uintptr_t)(DEVMPU_BASE + 0x0074))
#define DEVMPU_VIO_ID                           ((uintptr_t)(DEVMPU_BASE + 0x0078))
#define DEVMPU_VIO_DOMAIN                       ((uintptr_t)(DEVMPU_BASE + 0x007C))
#define DEVMPU_LOAD_TABLE_USING_APB             ((uintptr_t)(DEVMPU_BASE + 0x0080))
#define DEVMPU_RD_PERM_SRAM_ADDR                ((uintptr_t)(DEVMPU_BASE + 0x0084))
#define DEVMPU_RD_PERM_SRAM_DATA                ((uintptr_t)(DEVMPU_BASE + 0x0088))
#define DEVMPU_WR_PERM_SRAM_ADDR                ((uintptr_t)(DEVMPU_BASE + 0x008C))
#define DEVMPU_WR_PERM_SRAM_DATA                ((uintptr_t)(DEVMPU_BASE + 0x0090))
#define DEVMPU_RD_INVD_SRAM_ADDR                ((uintptr_t)(DEVMPU_BASE + 0x0094))
#define DEVMPU_RD_INVD_SRAM_DATA                ((uintptr_t)(DEVMPU_BASE + 0x0098))
#define DEVMPU_WR_INVD_SRAM_ADDR                ((uintptr_t)(DEVMPU_BASE + 0x009C))
#define DEVMPU_WR_INVD_SRAM_DATA                ((uintptr_t)(DEVMPU_BASE + 0x00A0))
#define DEVMPU_DEBUG_CON                        ((uintptr_t)(DEVMPU_BASE + 0x00A4))
#define DEVMPU_DEBUG_SRAM_ADDR                  ((uintptr_t)(DEVMPU_BASE + 0x00A8))
#define DEVMPU_DEBUG_SRAM_DATA                  ((uintptr_t)(DEVMPU_BASE + 0x00AC))
#define DEVMPU_VIO_SECURE_BIT                   ((uintptr_t)(DEVMPU_BASE + 0x01BC))

#define DEVMPU_DOMAIN_REMAP_0                   ((uintptr_t)(SECURITY_AO + 0x0604))
#define DEVMPU_DOMAIN_REMAP_1                   ((uintptr_t)(SECURITY_AO + 0x0608))

/* for flexibility */
#define DEVMPU_RD_VIO_ADDR                      DEVMPU_VIO_ADDR
#define DEVMPU_RD_VIO_ID                        DEVMPU_VIO_ID
#define DEVMPU_RD_VIO_DOMAIN                    DEVMPU_VIO_DOMAIN

#define DEVMPU_WR_VIO_ADDR                      DOMAIN_VIO_ADDR
#define DEVMPU_WR_VIO_ID                        DOMAIN_VIO_ID
#define DEVMPU_WR_VIO_DOMAIN                    DOMAIN_VIO_DOMAIN

/* HYP mode MPU (DeviceMPU) switch in EMI central module */
#define EMI_HYP_MOD                             (EMI_TESTC)
#define EMI_HYP_MOD_BITMASK                     0x40000


/**
 * Platform configs & Utils
 */
#define EMI_MPU_REGION_ID_AP        31

/* DRAM region and granularity that can be protected */
#define DEVMPU_DRAM_BASE            0x40000000UL
#define DEVMPU_DRAM_SIZE            0x200000000ULL  /* able to protect up to 8GB DRAM space */
#define DEVMPU_PAGE_SIZE            0x00200000      /* 2MB-based permission/invalidate control */
#define DEVMPU_PAGE_NUM             (DEVMPU_DRAM_SIZE/DEVMPU_PAGE_SIZE)

/* permission/invalidate control table format */
#define DEVMPU_PERM_ENT_NUM         256
#define DEVMPU_INVD_ENT_NUM         32
#define DEVMPU_ENT_SIZE             16              /* 128 bits (16 bytes): AXI data width */

/* is full violation address supported? */
#define DEVMPU_VIO_FULLINFO_SUPPORT 0

/* bit mask/offset to extract violation info. */
#define DEVMPU_VIO_MASK_ID          0x1FF
#define DEVMPU_VIO_MASK_ADDR        0x1FFF
#define DEVMPU_VIO_MASK_DOMAIN      0x1F
#define DEVMPU_VIO_MASK_SEC_B       0x1
#define DEVMPU_VIO_MASK_OFFSET_RD   0
#define DEVMPU_VIO_MASK_OFFSET_WR   16

/* vm domain remap format */
#define DEVMPU_VMD_NUM              4
#define DEVMPU_VMD_ENC_BITS         2
#define DEVMPU_VMD_ENC_MASK         ((0x1 << DEVMPU_VMD_ENC_BITS) - 1)
#define DEVMPU_EMI_DOMAIN_NUM       32
#define DEVMPU_EMI_DOMAIN_PER_ENT   16

/* vm domain mapping of emi domains */
enum DEVMPU_VMD_MAPPING {
	DEVMPU_VMD_MAPPING_D0  = 0,
	DEVMPU_VMD_MAPPING_D1  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D2  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D3  = DEVMPU_VMD_NUM - 1,

	DEVMPU_VMD_MAPPING_D4  = 2,
	DEVMPU_VMD_MAPPING_D5  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D6  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D7  = DEVMPU_VMD_NUM - 1,

	DEVMPU_VMD_MAPPING_D8  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D9  = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D10 = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D11 = 1,

	DEVMPU_VMD_MAPPING_D12 = 2,
	DEVMPU_VMD_MAPPING_D13 = 2,
	DEVMPU_VMD_MAPPING_D14 = DEVMPU_VMD_NUM - 1,
	DEVMPU_VMD_MAPPING_D15 = DEVMPU_VMD_NUM - 1,
};

#endif
