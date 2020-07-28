/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef RIL_WP_H
#define RIL_WP_H

#define PROPERTY_SWITCH_MODEM_CAUSE_TYPE "vendor.ril.switch.modem.cause.type"
#define PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE "255"

#define PROPERTY_SWITCH_MODEM_DELAY_INFO "vendor.ril.switch.modem.delay.info"
#define PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE "\"FFFFFF\",255,0"

#define WORLDPHONE_UNUSED(x) ((void)(x))

typedef enum {
    WP_SVLTE_PROJ_DC_3M = 3,
    WP_SVLTE_PROJ_DC_4M = 4,
    WP_SVLTE_PROJ_DC_5M = 5,
    WP_SVLTE_PROJ_DC_6M = 6,
    WP_SVLTE_PROJ_SC_3M = 103,
    WP_SVLTE_PROJ_SC_4M = 104,
    WP_SVLTE_PROJ_SC_5M = 105,
    WP_SVLTE_PROJ_SC_6M = 106,
} WP_SVLTE_PROJ_TYPE;

extern bool isWorldPhoneOldVersion();
extern void onSwitchModemCauseInformation(char *urc, const RIL_SOCKET_ID rid);
extern void onWorldModePrepareTRM(int mode);

/// M: world mode switching, pending special URC. @{
typedef struct UrcList {
    int rid;
    char *urc;
    char *sms_pdu;
    void *pNext;
} UrcList;
extern void cacheUrcIfNeeded(const char *s, const char *sms_pdu, RILChannelCtx *p_channel);
extern void sendPendedUrcs();
/// @}

extern int bUbinSupport;
extern int bWorldModeSwitching;

extern int  getWorldPhonePolicy();
extern void setWorldPhonePolicy(RILChannelCtx *pChannel);
extern void onWorldModeChanged(char *urc, const RIL_SOCKET_ID rid);
extern void handleWorldModeChanged(void *param);
extern int isWorldPhoneSupport();
extern int isWorldModeSwitching(const char *s);
/// M: world mode switching, need pass special request. @{
extern int allowForWorldModeSwitching(const int request);
void requestStoreModem(void *data, size_t datalen, RIL_Token t);
void requestReloadModem(void *data, size_t datalen, RIL_Token t);
void requestModifyModem(void *data, size_t datalen, RIL_Token t);
extern void setActiveModemType(int mdType);
extern int getActiveModemType();
void worldPhoneinitialize(const RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
void requestResumeWorldModeChanged(void * data, size_t datelen, RIL_Token t);
extern void queryKeep3GMode(RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
#endif /* RIL_WP_H */
