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

#include "ModemMessage.h"
#include <log/log.h>
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "RpModemMessage"
#endif

static const int INVALID_SERIAL = -1;

ModemMessage::ModemMessage() {
    RLOGD("Create Empty ModemMessage");
    serial = INVALID_SERIAL;
    slotId = 0;
    modemId = 0;
    connectWithCdmaModem = false;
    svlteModemStatusFirstDone = false;
    originalMessage = NULL;
    gsmResponse = NULL;
    cdmaResponse = NULL;
    gsmRequest = NULL;
    cdmaRequest = NULL;
    fromCdmaLteModeController = false;
    action = NULL;
}

ModemMessage::ModemMessage(const sp<RfxMessage>& message) {
    RLOGD("Create ModemMessage with RfxMessage");
    serial = message -> getPToken();
    slotId = 0;
    modemId = 0;
    connectWithCdmaModem = false;
    svlteModemStatusFirstDone = false;
    originalMessage = message;
    gsmResponse = NULL;
    cdmaResponse = NULL;
    gsmRequest = NULL;
    cdmaRequest = NULL;
    fromCdmaLteModeController = false;
    action = NULL;
}

ModemMessage::~ModemMessage() {
    RLOGD("Delete ModemMessage");
    serial = INVALID_SERIAL;
    originalMessage = NULL;
    gsmResponse = NULL;
    cdmaResponse = NULL;
    gsmRequest = NULL;
    cdmaRequest = NULL;
    fromCdmaLteModeController = false;
    action = NULL;
}

void ModemMessage::resetModemMessage() {
    originalMessage = NULL;
    gsmResponse = NULL;
    cdmaResponse = NULL;
    gsmRequest = NULL;
    cdmaRequest = NULL;
    fromCdmaLteModeController = false;
    svlteModemStatusFirstDone = false;
    action = NULL;
}

