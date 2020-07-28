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

#ifndef __TUI_M4U_H__
#define __TUI_M4U_H__
#include "m4u_port.h"
#include "cmdq_sec_record.h"

//------------------------------------------------------------------------------
/** init m4u must be called firstly.include init page table and mva graph. 
 *
 * @returns 0 in case of success
 * @returns -1 in case of error
 */
int tui_m4u_Init(void);


//------------------------------------------------------------------------------
/** switch m4u from normal world to secure world, after the normal frame done. 
 *
 * @returns 0 in case of success
 * @returns -1 in case of error
 */
int tui_m4u_switch_to_sec(void);


//------------------------------------------------------------------------------
/** cmdq call it after m4u inited. 
 *
 * @param cmdqRecHandle: [in] cmdqRecHandle
 * @param port: [in] port id.
 * @param mmu_en: [in] pass(1) or bypass(0) m4u.
 * @param sec: [in] security(1), non-security(0).
 */
void tui_m4u_config_port_sec(cmdqRecHandle cmdq_handle, uint32_t port, int mmu_en, int sec);

//------------------------------------------------------------------------------
/** translate 64bit physical buffer start address to 32bit modified virtual start address
 *   which is used for m4u. user can use mva to trigger mm engine to work instead.
 *
 * @param port_id: [in] which port use m4u.
 * @param phy_buf_addr: [in] 64bit physical buffer start address.
 * @param size: [in] buffer size.
 * @returns non-zero mva in case of success
 * @returns 0 in case of error.
 */
unsigned int tui_m4u_alloc_mva(enum M4U_PORT_ID port_id,
					      const uint64_t phy_buf_addr,
					      const unsigned int size);

//------------------------------------------------------------------------------
/** When you don't use a mva region any more, you must recycle it. 
 * @param mva_start: [in] mva start address.
 * @param buf_size: [in] buffer size.
 * @returns 0 in case of success.
 * @returns -1 in case of error.
 */
int tui_m4u_free(unsigned int mva_start, unsigned int buf_size);

//------------------------------------------------------------------------------
/** must be called when quite TUI!!! Or, corruption will occur in secure mode.(ex: SVP)
 */
void tui_m4u_deinit(void);
#endif
