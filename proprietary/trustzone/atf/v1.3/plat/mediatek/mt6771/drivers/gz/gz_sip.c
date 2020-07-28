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

#include <mmio.h>
#include <debug.h>
#include <errno.h>
#include <console.h>
#include <spinlock.h>
#include <platform_def.h>
#include <mtk_plat_common.h>

#include <gz_sip.h>

#define LOG_TAG "GZ_SIP"

#define GZSIP_DEBUG (0)

#if GZSIP_DEBUG
#define GZSIP_ERR(fmt, ...) \
	do { \
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE); \
		tf_printf("[%s][ERROR] %s:%d: " fmt, LOG_TAG, __func__, __LINE__, ##__VA_ARGS__); \
		console_uninit(); \
	} while (0)
#define GZSIP_DBG(fmt, ...) \
	do { \
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE); \
		tf_printf("[%s][DEBUG] %s:%d: " fmt, LOG_TAG, __func__, __LINE__, ##__VA_ARGS__); \
		console_uninit(); \
	} while (0)
#else
#define GZSIP_ERR(fmt, ...) tf_printf("[%s][ERROR] %s: " fmt, LOG_TAG, __func__, ##__VA_ARGS__)
#define GZSIP_DBG(fmt, ...) do {} while (0)
#endif

#define UNUSED(x) (void)(x)

#define bits_clr(x, m, o)		(x & (~(m << o)))
#define bits_get(x, m, o)		((x & (m << o)) >> o)
#define bits_set(x, v, m, o)	((bits_clr(x, m, o)) | ((v & m) << o))

/* IPU_CONN_SECURITY_CTRL bitfield */
#define SEC_CTRL_EMI_DOMAIN_MASK            0x1F
#define SEC_CTRL_EMI_AXPROT_NS_MASK         0x1
#define SEC_CTRL_INFRA_DOMAIN_MASK          0xF
#define SEC_CTRL_INFRA_AXPROT_NS_MASK       0x1
#define SEC_CTRL_EMI_RD_DOMAIN_OFFSET       0x0
#define SEC_CTRL_EMI_RD_AXPROT_NS_OFFSET    0x5
#define SEC_CTRL_EMI_WR_DOMAIN_OFFSET       0x6
#define SEC_CTRL_EMI_WR_AXPROT_NS_OFFSET    0xb
#define SEC_CTRL_INFRA_DOMAIN_OFFSET        0xc
#define SEC_CTRL_INFRA_AXPROT_NS_OFFSET     0x11

uint64_t sip_gz_vpu_ns_domain_set(uint64_t x1, uint64_t x2)
{
	uint64_t vpu_ns;
	uint64_t vpu_domain;

	uint32_t sec_ctrl;
	uint32_t sec_ctrl_old;

	/* decode input */
	vpu_ns = x1;
	vpu_domain = x2;

	if ((vpu_ns & SEC_CTRL_INFRA_AXPROT_NS_MASK) != vpu_ns) {
		GZSIP_ERR("invalid VPU NS=0x%lx\n", vpu_ns);
		return -EINVAL;
	}

	if ((vpu_domain & SEC_CTRL_INFRA_DOMAIN_MASK) != vpu_domain) {
		GZSIP_ERR("invalid VPU domain=0x%lx\n", vpu_domain);
		return -EINVAL;
	}

	sec_ctrl = mmio_read_32(IPU_CONN_SECURITY_CTRL);
	sec_ctrl_old = sec_ctrl;

	/* set EMI read AxPROT & Domain sideband */
	sec_ctrl = bits_set(sec_ctrl, vpu_domain, SEC_CTRL_EMI_DOMAIN_MASK, SEC_CTRL_EMI_RD_DOMAIN_OFFSET);
	sec_ctrl = bits_set(sec_ctrl, vpu_ns, SEC_CTRL_EMI_AXPROT_NS_MASK, SEC_CTRL_EMI_RD_AXPROT_NS_OFFSET);

	/* set EMI write AxPROT & Domain sideband */
	sec_ctrl = bits_set(sec_ctrl, vpu_domain, SEC_CTRL_EMI_DOMAIN_MASK, SEC_CTRL_EMI_WR_DOMAIN_OFFSET);
	sec_ctrl = bits_set(sec_ctrl, vpu_ns, SEC_CTRL_EMI_AXPROT_NS_MASK, SEC_CTRL_EMI_WR_AXPROT_NS_OFFSET);

	/* set INFRA AxPROT & Domain sideband */
	sec_ctrl = bits_set(sec_ctrl, vpu_domain, SEC_CTRL_INFRA_DOMAIN_MASK, SEC_CTRL_INFRA_DOMAIN_OFFSET);
	sec_ctrl = bits_set(sec_ctrl, vpu_ns, SEC_CTRL_INFRA_AXPROT_NS_MASK, SEC_CTRL_INFRA_AXPROT_NS_OFFSET);

	mmio_write_32(IPU_CONN_SECURITY_CTRL, sec_ctrl);

	/* prevent unused werror */
	UNUSED(sec_ctrl_old);
	GZSIP_DBG("Update IPU_CONN_SECURITY_CTRL: 0x%x -> 0x%x\n", sec_ctrl_old, sec_ctrl);

	return 0;
}
