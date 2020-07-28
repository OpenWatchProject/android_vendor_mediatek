/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef RIL_PHB_H
#define RIL_PHB_H 1

#include <stdio.h>
#include <telephony/mtk_ril.h>
#include <cutils/properties.h>
#include <libmtkrilutils.h>
#include "usim_fcp_parser.h"
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include "ril_sim.h"
#include "ril_callbacks.h"

typedef enum {
   CPBW_ENCODE_IRA,
   CPBW_ENCODE_UCS2,
   CPBW_ENCODE_UCS2_81,
   CPBW_ENCODE_UCS2_82,
   CPBW_ENCODE_GSM7BIT,
   CPBW_ENCODE_MAX
}RilPhbCpbwEncode;

#define RIL_PHB_UCS2_81_MASK    0x7f80

#define RIL_MAX_PHB_NAME_LEN 40   // Max # of characters in the NAME
#define RIL_MAX_PHB_EMAIL_LEN 60
#define RIL_MAX_PHB_ENTRY 10
#define PROPERTY_RIL_PHB_READY "vendor.gsm.sim.ril.phbready"

extern int isSimInserted(RIL_SOCKET_ID rid);
extern int rilPhbMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilPhbUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern int rild_sms_hexCharToDecInt(char *hex, int length);
extern bool bIsTc1();
extern int s_isUserLoad;
extern void resetPhbReady(RIL_SOCKET_ID rid);

#endif /* RIL_PHB_H */
