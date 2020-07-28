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

#ifndef RIL_CC_H
#define RIL_CC_H 1

typedef enum {
    GSM,
    IMS,
} dispatch_flg;

typedef enum {
    none,
    mo,
    mt,
    multi,
    conf,
    pending_mo,
} volte_call_state;

#define IMS_CALL_MODE_BOUNDARY 20

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

// For AT&T ECC
extern void stopEmergencySession(void *param);

extern void requestGetCurrentCalls(void * data, size_t datalen, RIL_Token t);
extern void requestDial(void * data, size_t datalen, RIL_Token t, int isEmergency, int isVt);
extern void requestHangup(void * data, size_t datalen, RIL_Token t);
extern void requestHangupWaitingOrBackground(void * data, size_t datalen, RIL_Token t);
extern void requestHangupForegroundResumeBackground(void * data, size_t datalen, RIL_Token t);
extern void requestSwitchWaitingOrHoldingAndActive(void * data, size_t datalen, RIL_Token t);
extern void requestAnswer(void * data, size_t datalen, RIL_Token t);
extern void requestConference(void * data, size_t datalen, RIL_Token t);
extern void requestUdub(void * data, size_t datalen, RIL_Token t);
extern void requestSeparateConnection(void * data, size_t datalen, RIL_Token t);
extern void requestExplicitCallTransfer(void * data, size_t datalen, RIL_Token t);
extern void requestLastCallFailCause(void * data, size_t datalen, RIL_Token t);
extern void requestDtmf(void * data, size_t datalen, RIL_Token t);
extern void requestDtmfStart(void *data, size_t datalen, RIL_Token t);
extern void requestDtmfStop(void *data, size_t datalen, RIL_Token t);
extern void requestSetTTYMode(void *data, size_t datalen, RIL_Token t);
extern void requestQueryTTYMode(void * data, size_t datalen, RIL_Token t);
extern void requestHangupAll(void * data, size_t datalen, RIL_Token t);
extern void requestForceReleaseCall(void * data, size_t datalen, RIL_Token t);
extern void onCallProgressInfoCallStateChange(char *urc, RIL_SOCKET_ID rid);
extern void onSpeechInfo(char *urc, RIL_SOCKET_ID rid);
extern void onIncomingCallIndication(char *urc, RIL_SOCKET_ID rid);
extern void onCipherIndication(char *urc, RIL_SOCKET_ID rid);
/// M: CC: Set service category before making Emergency call. @{
extern void onEmergencyCallServiceCategoryIndication(char *s, RIL_SOCKET_ID rid);
extern void onMccMncIndication(RIL_SOCKET_ID slotId, char* mccmnc);
extern void requestLocalSetEccServiceCategory(void *data, size_t datalen, RIL_Token t);
/// @}

/// M: CC: GSA HD Voice for 2/3G network support
extern void onSpeechCodecInfo(char *urc, RIL_SOCKET_ID rid);

/// M: CC: Call control framework ECC enhancement @{
extern void onEmergencyCallBackMode(const char *s, const RIL_SOCKET_ID rid);
extern void requestSetEccMode(void *data, size_t datalen, RIL_Token t);
extern void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t);
/// @}

/// M: CC: Normal/Emergency call redial
extern void requestRedial(void *data, size_t datalen, RIL_Token t);
extern void requestNotifyEmergencySessionStatus(void *data, size_t datalen, RIL_Token t, int isStarted);
extern void requestSetHvolteMode(void *data, size_t datalen, RIL_Token t);

extern void onSuppSvcNotification(char *s, int isMT, RIL_SOCKET_ID rid);
// M: Set ECC ist to MD
extern void requestSetEccList(void *data, size_t datalen, RIL_Token t);
extern void requestSetCallIndication(void *data, size_t datalen, RIL_Token t);

// M: To notify SS module that CRING and ECPI 133 come
extern void onCRINGReceived();
extern void onECPI133Received();

extern void requestHangupWithReason(void * data, size_t datalen, RIL_Token t);


#endif /* RIL_CC_H */

