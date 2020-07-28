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
 /*
  * rfx_data_to_parcel_utils.h
  *
  *  Created on: 2015/09/02
  *  Author: MTK10602
  *
  */
#ifndef __RFX_TRANSFER_H__
#define __RFX_TRANSFER_H__

#include "Parcel.h"
#include <telephony/mtk_ril.h>
#include "RfxLog.h"
#include <cutils/jstring.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

using ::android::Parcel;

#undef LOG_TAG
#define LOG_TAG "RfxTransUtils"
#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))

#if defined(ANDROID_MULTI_SIM)
#define RESPONSE_TO_RILJ(id, data, datalen, rilSocketId) do { \
            if (data != NULL) { \
                RfxRilAdapter::responseToRilj(id, data, datalen, rilSocketId); \
            } else { \
                RfxRilAdapter::responseToRilj(id, NULL, 0, rilSocketId); \
            } \
        } while (0)
#else
#define RESPONSE_TO_RILJ(id, data, datalen, rilSocketId) do { \
            if (data != NULL) { \
                RfxRilAdapter::responseToRilj(id, data, datalen); \
            } else { \
                RfxRilAdapter::responseToRilj(id, NULL, 0); \
            } \
        } while (0)
#endif

extern const char *getVersion();
typedef struct DataToParcelInfo {
    int request;
    Parcel* (*dataToParcelFunction) (int request, int token, void* data, int datalen);
    void (*parcleToDataFunction) (RIL_Token t, RIL_Errno e, int request, Parcel* p, int slotId);
} DataToParcelInfo;

typedef struct ParcelToDataInfo {
    int urc;
    void (*parcleToDataFunction) (RIL_Token t, RIL_Errno e, int urc, Parcel* p, int slotId);
} ParcelToDataInfo;

static Parcel* callForwardToParcel(int request, int token, void* data, int datalen);
static Parcel* cdmaBrSmsCnfToParcel(int request, int token, void* data, int datalen);
static Parcel* cdmaSmsToParcel(int request, int token, void* data, int datalen);
static Parcel* cdmaSmsAckToParcel(int request, int token, void* data, int datalen);
static Parcel* cdmaSubscriptionSourceToParcel(int request, int token, void* data,
        int datalen);
static Parcel* dataCallToParcel(int request, int token, void* data, int datalen);
static Parcel* dataProfileToParcel(int request, int token, void* data, int datalen);
static Parcel* dialToParcel(int request, int token, void* data, int datalen);
static Parcel* emergencyDialToParcel(int request, int token, void* data, int datalen);
static Parcel* fd_ModeToParcel(int request, int token, void* data, int datalen);
static Parcel* gsmBrSmsCnfToParcel(int request, int token, void* data, int datalen);
static Parcel* imsCdmaSmsToParcel(Parcel *parcel, RIL_CDMA_SMS_Message *rcsm);
static Parcel* imsGsmSmsToParcel(Parcel *parcel, void **gsmMessage, int datalen);
static Parcel* imsSmsToParcel(int request, int token, void* data, int datalen);
static Parcel* intsToParcel(int request, int token, void* data, int datalen);
static Parcel* modifyDedicateDataCallToParcel(int request, int token, void* data, int datalen);
static Parcel* nvReadItemToParcel(int request, int token, void* data, int datalen);
static Parcel* nvWriteItemToParcel(int request, int token, void* data, int datalen);
static Parcel* phbEntryToParcel(int request, int token, void* data, int datalen);
static Parcel* radioCapabilityToParcel(int request, int token, void* data, int datalen);
static Parcel* rawToParcel(int request, int token, void* data, int datalen);
static Parcel* rilCdmaSmsWriteArgsToParcel(int request, int token, void* data, int datalen);
static Parcel* setInitialAttachApnToParcel(int request, int token, void* data, int datalen);
static Parcel* setupDedicateDataCallToParcel(int request, int token, void* data, int datalen);
static Parcel* simAuthToParcel(int request, int token, void* data, int datalen);
static Parcel* simAuthenticationToParcel(int request, int token, void* data, int datalen);
static Parcel* sim_APDUToParcel(int request, int token, void* data, int datalen);
static Parcel* sim_IOToParcel(int request, int token, void* data, int datalen);
static Parcel* sim_IO_EXToParcel(int request, int token, void* data, int datalen);
static Parcel* smsParamsToParcel(int request, int token, void* data, int datalen);
static Parcel* smsWriteToParcel(int request, int token, void* data, int datalen);
static Parcel* stringToParcel(int request, int token, void* data, int datalen);
static Parcel* stringsToParcel(int request, int token, void* data, int datalen);
static Parcel* uiccAuthenticationToParcel(int request, int token, void* data, int datalen);
static Parcel* uiccIoToParcel(int request, int token, void* data, int datalen);
static Parcel* uiccSubscriptonToParcel(int request, int token, void* data, int datalen);
static Parcel* voiceRadioTechToParcel(int request, int token, void* data, int datalen);
static Parcel* voidToParcel(int request, int token, void* data, int datalen);
static Parcel* vsimEventToParcel(int request, int token, void* data, int datalen);
static Parcel* vsimOperationEventToParcel(int request, int token, void* data, int datalen);
static Parcel* writePhbEntryExtToParcel(int request, int token, void* data, int datalen);
static Parcel* writePbToRuimToParcel(int request, int token, void* data, int datalen);
static Parcel* mtkWritePbToRuimToParcel(int request, int token, void* data, int datalen);
static Parcel* playDTMFToneToParcel(int request, int token, void* data, int datalen);
static Parcel* playToneSeqToParcel(int request, int token, void* data, int datalen);
static Parcel* cdmaSmsToParcelInternal(Parcel *parcel, RIL_CDMA_SMS_Message *rcsm);
static Parcel* carrierRestrictionsToParcel(int request, int token, void* data, int datalen);
static Parcel* carrierToParcelInternal(Parcel *parcel, RIL_Carrier *carrier);
static Parcel* fdModeToParcel(int request, int token, void* data, int datalen);
static Parcel* openChannelParamsToParcel (int request, int token, void* data, int datalen);
static Parcel* networkScanToParcel(int request, int token, void* data, int datalen);
static Parcel* keepaliveRequestToParcel(int request, int token, void* data, int datalen);
static void fillHeader(Parcel *parcel, int request, int token);

static void parcelToUssdStrings(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToVsimOperationEvent(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToInts(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToFailCause(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToStrings(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToString(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToVoid(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToUnused(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCallList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSMS(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSIM_IO(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCallForwards(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToDataCallList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSetupDataCall(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToRaw(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSsn(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCrssN(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSimStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToGsmBrSmsCnf(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCdmaBrSmsCnf(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCdmaSms(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCellList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCdmaInformationRecords(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToRilSignalStrength(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCallRing(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCdmaSignalInfoRecord(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCdmaCallWaiting(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSimRefresh(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCellInfoList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToPhbEntries(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToGetSmsSimMemStatusCnf(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToGetPhbMemStorage(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToReadPhbEntryExt(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSmsParams(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCbConfigInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEtwsNotification(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSetupDedicateDataCall(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEpcNetworkFeatureInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToIratStateChange(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToHardwareConfig(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToDcRtInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToRadioCapability(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToSSData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToLceStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToLceData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToActivityData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToVoiceState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToDataState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCarrierRestrictions(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToPcoData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToCrssN(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToNetworkScanResult(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
// M: [VzW] Data Framework
static void parcelToPcoDataAfterAttached(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);

/// M: eMBMS feature
static Parcel* embmsStartSessionReqToParcel(int request, int token, void* data, int datalen);
static Parcel* embmsStopSessionReqToParcel(int request, int token, void* data, int datalen);
static void parcelToEmbmsEnable(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsDisable(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsSessionInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsNetworkTime(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsGetCoverageState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsCellInfoNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsActiveSessionNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsSaiNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsOosNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parcelToEmbmsModemEeNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
/// M: eMBMS end

/// Ims Data Framework @{
static void parcelToSetupDedicateDataCall(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
static void parseSetupDedicateDataCall(RIL_Dedicate_Data_Call_Struct *data, Parcel *p, int num);
/// @}

// External SIM [Start]
static void parcelToVsimOperationEvent(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);
// External SIM [End]

static void parcelToEmergencyNumberList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);

static void parcelToKeepaliveStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);

static Parcel* linkCapacityReportingCriteriaToParcel(int request, int token, void* data, int datalen);
static void parcelToLinkCapacityEstimate(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);

extern int convertResponseStringEntryToInt(char **response, int index, int numStrings);
extern RIL_CellInfoType getCellInfoTypeRadioTechnology(char *rat);
static void parcelToPhoneCapability(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId);

#ifdef HAVE_AEE_FEATURE
static void triggerWarning(char *pErrMsg);
#endif
class RfxTransferUtils {
private:
    RfxTransferUtils();

    virtual ~RfxTransferUtils();

public:
    static Parcel* dataToParcel(int request, int token, void* data, int datalen);
    static void parcelToDataResponse(RIL_Token t, RIL_Errno e, int request, Parcel * parcel, int slotId);
    static void parcelToDataUrc(int urc, Parcel *parcel, int slotId);
private:
    static DataToParcelInfo* findParcelInfoFromRequestTable(int request);
    static ParcelToDataInfo* findDataInfoFromUrcTable(int urc);
};
#endif
