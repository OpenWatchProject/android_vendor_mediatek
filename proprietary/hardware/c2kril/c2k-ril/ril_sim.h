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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef RIL_SIM_H
#define RIL_SIM_H 1

typedef enum {
    SIM_MISSING_RESOURCE = 13,
    SIM_NO_SUCH_ELEMENT = 90,
} AT_SIM_ERROR;

static const char PROPERTY_RIL_UICC_TYPE[4][35] = {
    "vendor.gsm.ril.uicctype",
    "vendor.gsm.ril.uicctype.2",
    "vendor.gsm.ril.uicctype.3",
    "vendor.gsm.ril.uicctype.4",
};

static const char PROPERTY_RIL_FULL_UICC_TYPE[4][35] = {
    "vendor.gsm.ril.fulluicctype",
    "vendor.gsm.ril.fulluicctype.2",
    "vendor.gsm.ril.fulluicctype.3",
    "vendor.gsm.ril.fulluicctype.4",
};

static const char PROPERTY_RIL_CT3G[4][35] = {
    "vendor.gsm.ril.ct3g",
    "vendor.gsm.ril.ct3g.2",
    "vendor.gsm.ril.ct3g.3",
    "vendor.gsm.ril.ct3g.4",
};

static const char PROPERTY_RIL_CT3G_ROAMING[4][35] = {
    "vendor.gsm.ril.ct3groaming",
    "vendor.gsm.ril.ct3groaming.2",
    "vendor.gsm.ril.ct3groaming.3",
    "vendor.gsm.ril.ct3groaming.4",
};

static const char PROPERTY_RIL_CT3G_ROAMING2[4][35] = {
    "vendor.gsm.ril.ct3groaming2",
    "vendor.gsm.ril.ct3groaming2.2",
    "vendor.gsm.ril.ct3groaming2.3",
    "vendor.gsm.ril.ct3groaming2.4",
};

static const char PROPERTY_RIL_CDMA_CARD_TYPE[4][35] = {
    "vendor.ril.cdma.card.type.1",
    "vendor.ril.cdma.card.type.2",
    "vendor.ril.cdma.card.type.3",
    "vendor.ril.cdma.card.type.4",
};

static const char* PROPERTY_ICCID_SIM[] = {
    "vendor.ril.iccid.sim1",
    "vendor.ril.iccid.sim2",
    "vendor.ril.iccid.sim3",
    "vendor.ril.iccid.sim4",
};

static const char PROPERTY_RIL_SIM_PIN1[4][35] = {
    "vendor.gsm.sim.retry.pin1",
    "vendor.gsm.sim.retry.pin1.2",
    "vendor.gsm.sim.retry.pin1.3",
    "vendor.gsm.sim.retry.pin1.4",
};

static const char PROPERTY_RIL_SIM_PUK1[4][35] = {
    "vendor.gsm.sim.retry.puk1",
    "vendor.gsm.sim.retry.puk1.2",
    "vendor.gsm.sim.retry.puk1.3",
    "vendor.gsm.sim.retry.puk1.4",
};

static const char PROPERTY_RIL_SIM_PIN2[4][35] = {
    "vendor.gsm.sim.retry.pin2",
    "vendor.gsm.sim.retry.pin2.2",
    "vendor.gsm.sim.retry.pin2.3",
    "vendor.gsm.sim.retry.pin2.4",
};

static const char PROPERTY_RIL_SIM_PUK2[4][35] = {
    "vendor.gsm.sim.retry.puk2",
    "vendor.gsm.sim.retry.puk2.2",
    "vendor.gsm.sim.retry.puk2.3",
    "vendor.gsm.sim.retry.puk2.4",
};

/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
typedef void (*UpdateCardState)(void *controller, int card_state);
typedef void (*SetSimPower)(void *controller);
typedef void (*RefreshSimPower)(void *controller, int card_state);
typedef void (*TiggerSetSimPower)(int power);

typedef enum {
    _FALSE = 0,
    _TRUE = 1
} _BOOL;

typedef struct {
    int card_state;
    _BOOL is_poweroff;
    UpdateCardState update;
    SetSimPower execute;
    RefreshSimPower refresh;
    TiggerSetSimPower trigger;
} SimPowerController;

typedef struct {
    char *mdn;
    char *homesid;
    char *homenid;
    char *min;
    char *prl_id;
} RIL_Subscription;

static void updateCardState(void *controller, int card_state);
static void setSimPower(void *controller);
static void refreshSimPower(void *controller, int card_state);
static void triggerSetSimPower(int power);
static void requestSetSimPower(void *controller);
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

int rilSimMain(int request, void *data, size_t datalen, RIL_Token t);
int rilSimUnsolicited(const char *s, const char *sms_pdu);

void reportRuimStatus(void* param);
void reportCardType (void);

void resetSIMProperties(const char *p_sim);
void requestEnableMD3Sleep(void *data, size_t datalen, RIL_Token t);
int checkUserLoad();

// ISIM
#define MAX_AID_LEN 33
#define MAX_AID_LABEL_LEN 256
#define AID_PREFIX_LEN 14
// CDMA subscription from RUIM
#define SUBSCRIPTION_FROM_RUIM 0
// CDMA subscription from NV
#define SUBSCRIPTION_FROM_NV 1

typedef struct {
    int appId;
    int session;
}SessionInfo;

typedef struct {
    int aid_len;
    int app_label_len;
    char aid[MAX_AID_LEN];
    char appLabel[MAX_AID_LABEL_LEN];
}AidInfo;

typedef enum {
    // AID_RUIM,
    AID_CSIM,
    AID_MAX
} AidIndex;

void resetAidInfo(void);
AidInfo* getAidInfo(AidIndex index);
int queryAppType(char* pAid);
void queryEfDir(RIL_Token t);
int dispatchCardStatus(VIA_RIL_CardStatus * p_card_status,
        RIL_CardStatus_v8 ** pp_oem_card_status);

/*
@param rid is reserved input, no used right now
*/
extern _BOOL isSimInserted(RIL_SOCKET_ID rid);

// BT SIM Access Profile - START
typedef enum
{
   BT_SAP_INIT,
   BT_SAP_CONNECTION_SETUP,
   BT_SAP_ONGOING_CONNECTION,
   BT_SAP_DISCONNECT,
   BT_SAP_POWER_ON,
   BT_SAP_POWER_OFF,
} BtSapStatus;

typedef struct _LocalBtSapMsgHeader {
    RIL_SOCKET_ID socket_id;
    RIL_Token t;
    void *param;
} LocalBtSapMsgHeader;

extern int rilBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
BtSapStatus queryBtSapStatus(RIL_SOCKET_ID rid);
// BT SIM Access Profile - END

#endif /* RIL_CC_H */
