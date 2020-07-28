/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
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
#ifndef __RTST_PARCEL_UTILS_H__
#define __RTST_PARCEL_UTILS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Parcel.h>
#include <stddef.h>
#include <utils/Thread.h>
#include "utils/RefBase.h"
#include "utils/Errors.h"
#include <telephony/ril.h>
#include "RilClientConstants.h"
#include "RfxDispatchThread.h"

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using ::android::Parcel;

/*****************************************************************************
 * Parcel utils api
 *****************************************************************************/
extern void dispatchVoid(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchString(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchStrings(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchInts(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchDial(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchEmergencyDial(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchSIM_IO(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchSIM_APDU(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchCallForward(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchCallForwardEx(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchRaw(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchSmsWrite(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchDataCall(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchVoiceRadioTech(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchSetInitialAttachApn(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);
extern void dispatchCdmaSubscriptionSource(RIL_RequestFunc onRequest, Parcel& p, RfxRequestInfo *pRI);

extern void dispatchCdmaSms(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchImsSms(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchImsCdmaSms(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI, uint8_t retry, int32_t messageRef);
extern void dispatchImsGsmSms(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI, uint8_t retry, int32_t messageRef);
extern void dispatchCdmaSmsAck(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchGsmBrSmsCnf(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchCdmaBrSmsCnf(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchRilCdmaSmsWriteArgs(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchNVReadItem(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchNVWriteItem(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchUiccSubscripton(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchSimAuthentication(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchDataProfile(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchRadioCapability(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchCarrierRestrictions(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchPhbEntry(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchSetSystemSelectionChannels(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);
extern void dispatchSetSignalStrengthReportingCriteria(RIL_RequestFunc onRequest, Parcel &p, RfxRequestInfo *pRI);

extern int responseInts(Parcel &p, void *response, size_t responselen);
extern int responseFailCause(Parcel &p, void *response, size_t responselen);
extern int responseStrings(Parcel &p, void *response, size_t responselen);
extern int responseString(Parcel &p, void *response, size_t responselen);
extern int responseVoid(Parcel &p, void *response, size_t responselen);
extern int responseCallList(Parcel &p, void *response, size_t responselen);
extern int responseSMS(Parcel &p, void *response, size_t responselen);
extern int responseSIM_IO(Parcel &p, void *response, size_t responselen);
extern int responseCallForwards(Parcel &p, void *response, size_t responselen);
extern int responseCallForwardsEx(Parcel &p, void *response, size_t responselen);
extern int responseDataCallList(Parcel &p, void *response, size_t responselen);
extern int responseSetupDataCall(Parcel &p, void *response, size_t responselen);
extern int responseRaw(Parcel &p, void *response, size_t responselen);
extern int responseSsn(Parcel &p, void *response, size_t responselen);
extern int responseSimStatus(Parcel &p, void *response, size_t responselen);
extern int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen);
extern int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen);
extern int responseCdmaSms(Parcel &p, void *response, size_t responselen);
extern int responseCellList(Parcel &p, void *response, size_t responselen);
extern int responseCdmaInformationRecords(Parcel &p,void *response, size_t responselen);
extern int responseRilSignalStrength(Parcel &p,void *response, size_t responselen);
extern int responseCallRing(Parcel &p, void *response, size_t responselen);
extern int responseCdmaSignalInfoRecord(Parcel &p,void *response, size_t responselen);
extern int responseCdmaCallWaiting(Parcel &p,void *response, size_t responselen);
extern int responseSimRefresh(Parcel &p, void *response, size_t responselen);
extern int responseCellInfoList(Parcel &p, void *response, size_t responselen);
extern int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen);
extern int responseEtwsNotification(Parcel &p, void *response, size_t responselen);
extern int responseHardwareConfig(Parcel &p, void *response, size_t responselen);
extern int responseDcRtInfo(Parcel &p, void *response, size_t responselen);
extern int responseRadioCapability(Parcel &p, void *response, size_t responselen);
extern int responseSSData(Parcel &p, void *response, size_t responselen);
extern int responseLceStatus(Parcel &p, void *response, size_t responselen);
extern int responseLceData(Parcel &p, void *response, size_t responselen);
extern int responseActivityData(Parcel &p, void *response, size_t responselen);
extern int responseCarrierRestrictions(Parcel &p, void *response, size_t responselen);
extern int responsePcoData(Parcel &p, void *response, size_t responselen);
extern int responsePcoIaData(Parcel &p, void *response, size_t responselen);
extern int responseCrssN(Parcel &p, void *response, size_t responselen);
extern int responseVoiceRegistrationState(Parcel &p, void *response, size_t responselen);
extern int responseDataRegistrationState(Parcel &p, void *response, size_t responselen);
extern int responseEmergencyList(Parcel &p, void *response, size_t responselen);
extern int responsePhbEntries(Parcel &p, void *response, size_t responselen);
#endif /* __RTST_PARCEL_UTILS_H__ */
