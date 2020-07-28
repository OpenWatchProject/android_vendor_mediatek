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

#ifndef __DEVMPU_H__
#define __DEVMPU_H__

#include <stdint.h>
#include <stdbool.h>
#include <devmpu_mt.h>

#define DEVMPU_MASK				(~(DEVMPU_PAGE_SIZE - 1))
#define DEVMPU_ALIGN_BITS		__builtin_ctz(DEVMPU_PAGE_SIZE)
#define DEVMPU_ALIGN_UP(x)		((x + DEVMPU_PAGE_SIZE - 1) & DEVMPU_MASK)
#define DEVMPU_ALIGN_DOWN(x)	((x) & DEVMPU_MASK)

enum DEVMPU_PERM {
	DEVMPU_PERM_NS,           /* 0b00, Non-Secure/Secure accessible */
	DEVMPU_PERM_S,            /* 0b01, Secure accessible */
	DEVMPU_PERM_BLOCK,
	DEVMPU_PERM_BLOCK_STILL,  /* 0b1X, Block */
	DEVMPU_PERM_NUM
};

/* structure for violation information */
struct devmpu_vio_stat {

	/* master ID */
	uint16_t id;

	/* master domain */
	uint8_t domain;

	/* is NS transaction (AXI sideband secure bit) */
	bool is_ns;

	/* is write violation */
	bool is_write;

	/* padding */
	uint8_t padding[3];

	/* physical address */
	uint64_t addr;
};

/* structure for permission dump */
struct devmpu_vmd_perm {
	enum DEVMPU_PERM rd_perm[DEVMPU_VMD_NUM];
	enum DEVMPU_PERM wr_perm[DEVMPU_VMD_NUM];
};


/**
 * Get the violation status
 * @is_wr_vio: the violation type
 * @vio_stat: the violation status output
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_vio_get(struct devmpu_vio_stat *vio_stat);

/**
 * Clear Device MPU violation status register
 *
 * Return: void
 */
void devmpu_vio_clr(void);

/**
 * Get the remapped VM domain of a requested EMI domain
 * @emi_domain: the EMI domain requested
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_domain_remap_get(uint32_t emi_domain, uint32_t *vm_domain);

/**
 * Set the remap between EMI domain and VM domain
 * @emi_domain: the source EMI domain to be remapped
 * @vm_domain: the VM domain to map to
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_domain_remap_set(uint32_t emi_domain, uint32_t vm_domain);

/**
 * Get both read and write permission for a physical memory region
 * @pa: the DEVMPU_PAGE_SIZE-aligned physical address whose permission setting is requested
 * @perm_out: the permission setting output
 *
 * Return: 0 on success, -1 if any error and the perm_out may be corrupted
 */
int devmpu_rw_perm_get(uint64_t pa, struct devmpu_vmd_perm *perm_out);

/**
 * Set read/write permission for a physical memory region
 * @pa: the DEVMPU_PAGE_SIZE-aligned physical address to be protected
 * @sz: the DEVMPU_PAGE_SIZE-aligned size of the physical region in bytes
 * @vmd_perm: the permission of each Device MPU VM domain
 * @is_write: the indicator for either read or write permission table update
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_rw_perm_set(uint64_t pa, uint64_t sz, enum DEVMPU_PERM vmd_perm[DEVMPU_VMD_NUM], bool is_write);

/**
 * Reset Device MPU
 * Reset HW configurations, where the permission table,
 * debug interface, and VM dormain remap configurations
 * are (mostly) zero-initialized.
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_reset(void);

/**
 * Check if Device MPU is enabled
 *
 * Return: true if enabled, false otherwise
 */
bool devmpu_is_enabled(void);

/**
 * Disable Device MPU
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_disable(void);

/**
 * Enable Device MPU
 * NOTE that the permission config should be cleared
 * before the first-time enable as the internal SRAM
 * data is reset to unknown, which can result in un-
 * expected MPU violation.
 *
 * Return: 0 on success, -1 if any error
 */
int devmpu_enable(void);

/**
 * SiP wrapper to get DeviceMPU violation information
 * @x1: do violation clear or not after violation info. is retrieved
 * @o1: physical address where the violation raised
 * @o2: misc information of the violation
 *
 * Return: 0 on success, negative value if any error
 */
uint64_t sip_devmpu_vio_get(uint64_t x1, uint64_t *o1, uint64_t *o2);

/**
 * SiP wrapper to clear DeviceMPU violation information
 *
 * Return: 0 on success, negative value if any error
 */
uint64_t sip_devmpu_vio_clr(void);

/**
 * SiP wrapper to get DeviceMPU permission settings
 * @x1: physical address
 * @o1: read permission of the requested x1 for VM domains
 * @o2: write permission of the requested x1 for VM domains
 *
 * Return: 0 on success, negative value if any error
 */
uint64_t sip_devmpu_rw_perm_get(uint64_t x1, uint64_t *o1, uint64_t *o2);

/**
 * SiP wrapper to set DeviceMPU permission settings
 * @x1: physical address
 * @x2: read permission of VM domains to be set for x1
 * @x3: write permission of VM domains to be set for x1
 *
 * Return: 0 on success, negative value if any error
 */
uint64_t sip_devmpu_rw_perm_set(uint64_t x1, uint64_t x2, uint64_t x3);

/**
 * SiP wrapper to set DeviceMPU domain remap settings
 * @x1: EMI domain to be remapped
 * @x2: VM domain which the EMI domain is remapped to
 *
 * Return: 0 on success, negative value if any error
 */
uint64_t sip_devmpu_domain_remap_set(uint64_t x1, uint64_t x2);

#endif /* __DEVMPU_H__ */
