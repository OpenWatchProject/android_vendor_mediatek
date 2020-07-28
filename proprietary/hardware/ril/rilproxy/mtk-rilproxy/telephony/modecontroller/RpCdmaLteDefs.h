/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RP_CDMALTE_DEFS_H__
#define __RP_CDMALTE_DEFS_H__
/***************************************************************************** 
 * Include
 *****************************************************************************/
#include "RfxDefs.h"
#include "RfxStatusDefs.h"

/***************************************************************************** 
 * Define
 *****************************************************************************/
/*LOG_TAG*/
#define RP_CDMALTE_MODE_TAG "RpCdmaLteModeController"

/* CARD_TYPE*/
#define CARD_TYPE_INVALID (-1)
#define CARD_TYPE_NONE (0)
#define CARD_TYPE_SIM  RFX_CARD_TYPE_SIM
#define CARD_TYPE_USIM RFX_CARD_TYPE_USIM
#define CARD_TYPE_RUIM RFX_CARD_TYPE_RUIM
#define CARD_TYPE_CSIM RFX_CARD_TYPE_CSIM

/* RADIO_TECH_MODE*/
#define RADIO_TECH_MODE_UNKNOWN  (1)
#define RADIO_TECH_MODE_CSFB     (2)
#define RADIO_TECH_MODE_CDMALTE    (3)

/* OPERATOR*/
#define OPERATOR_OM "OM"

/*Card type state*/
#define CARD_STATE_INVALID (-1)
#define CARD_STATE_HOT_PLUGIN (0)
#define CARD_STATE_HOT_PLUGOUT (1)
#define CARD_STATE_NO_CHANGED (2)
#define CARD_STATE_NOT_HOT_PLUG (3)
#define CARD_STATE_CARD_TYPE_CHANGED (4)

/*Dynamic Capability Swtich*/
#define ENTER_RESTRICT_MODE (1)
#define LEAVE_RESTRICT_MODE (0)

/* SWITCH_CARD_TYPE*/
#define INVALID_MODE (-1)
#define SIM_MODE (0)
#define RUIM_MODE (1)

/*System property for common slot*/
#define PROPERTY_COMMON_SLOT_SUPPORT "ro.vendor.mtk_sim_hot_swap_common_slot"

#endif /* __RP_CDMALTE_DEFS_H__ */

