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
  * rfx_data_to_parcel_utils.cpp
  *
  *  Created on: 2015/09/02
  *  Author: MTK10602
  *
  */
#include "RfxTransferUtils.h"
#include "RfxRilAdapter.h"
#include <cutils/properties.h>

using android::status_t;
using android::NO_ERROR;

static DataToParcelInfo s_data_to_parcel_request[] = {
    #include "data_to_parcel_ril_commands.h"
};
static DataToParcelInfo s_data_to_parcel_mtk_request[] = {
    #include "data_to_parcel_mtk_ril_commands.h"
};
static ParcelToDataInfo s_parcel_to_data_urc[] = {
    #include "parcel_to_data_unsol_commands.h"
};
static ParcelToDataInfo s_parcel_to_data_mtk_urc[] = {
    #include "parcel_to_data_mtk_unsol_commands.h"
};

Parcel* RfxTransferUtils::dataToParcel(int request, int token, void* data, int datalen) {
    DataToParcelInfo *parcelInfo;
    parcelInfo = findParcelInfoFromRequestTable(request);
    if (parcelInfo == NULL) {
        RFX_LOG_E(LOG_TAG, "[RFX] can not find request parcelInfo, request %d", request);
        return NULL;
    }
    return parcelInfo->dataToParcelFunction(request, token, data, datalen);
}

void RfxTransferUtils::parcelToDataResponse(RIL_Token t, RIL_Errno e, int request,
        Parcel * parcel, int slotId) {
    DataToParcelInfo *parcelInfo;
    parcelInfo = findParcelInfoFromRequestTable(request);
    if (parcelInfo == NULL) {
        RFX_LOG_E(LOG_TAG, "[RFX] can not find response parcelInfo, request %d", request);
        return;
    }
    parcelInfo->parcleToDataFunction(t, e, request, parcel, slotId);
}

void RfxTransferUtils::parcelToDataUrc(int urc, Parcel * parcel, int slotId) {
    ParcelToDataInfo *dataInfo;
    dataInfo = findDataInfoFromUrcTable(urc);
    if (dataInfo == NULL) {
        RFX_LOG_E(LOG_TAG, "[RFX] can not find urc parcelInfo, urc %d", urc);
        return;
    }
    dataInfo->parcleToDataFunction(NULL, RIL_E_SUCCESS, urc, parcel, slotId);
}

DataToParcelInfo* RfxTransferUtils::findParcelInfoFromRequestTable(int request) {
    DataToParcelInfo *parcelInfo = NULL;
    for (int i=0; i<(int32_t)NUM_ELEMS(s_data_to_parcel_request); i++) {
        if (request == s_data_to_parcel_request[i].request) {
            parcelInfo = &(s_data_to_parcel_request[i]);
            return parcelInfo;
        }
    }

    for (int i=0; i<(int32_t)NUM_ELEMS(s_data_to_parcel_mtk_request); i++) {
        if (request == s_data_to_parcel_mtk_request[i].request) {
            parcelInfo = &(s_data_to_parcel_mtk_request[i]);
            return parcelInfo;
        }
    }

    RFX_LOG_E(LOG_TAG, "[RFX] Can not find request in data_to_parcel table, request %d", request);
    return NULL;
}

ParcelToDataInfo* RfxTransferUtils::findDataInfoFromUrcTable(int urc) {
    ParcelToDataInfo *dataInfo = NULL;
    for (int i=0; i<(int32_t)NUM_ELEMS(s_parcel_to_data_urc); i++) {
        if (urc == s_parcel_to_data_urc[i].urc) {
            dataInfo = &(s_parcel_to_data_urc[i]);
            RFX_LOG_D(LOG_TAG, "[RFX] find urc in index %d", i);
            return dataInfo;
        }
    }

    for (int i=0; i<(int32_t)NUM_ELEMS(s_parcel_to_data_mtk_urc); i++) {
        if (urc == s_parcel_to_data_mtk_urc[i].urc) {
            dataInfo = &(s_parcel_to_data_mtk_urc[i]);
            RFX_LOG_D(LOG_TAG, "[RFX] find mtk urc in index %d", i);
            return dataInfo;
        }
    }

    RFX_LOG_E(LOG_TAG, "[RFX] Can not find urc %d in parcel_to_data table", urc);
    return NULL;
}

#ifdef HAVE_AEE_FEATURE
void triggerWarning(char *pErrMsg) {
    if (pErrMsg != NULL) {
        aee_system_warning("ril-proxy", NULL, DB_OPT_DEFAULT, pErrMsg);
        exit(0);
    } else {
        assert(0);
    }
}
#endif

bool isResponseType(int type) {
    if (type != RESPONSE_SOLICITED && type != RESPONSE_SOLICITED_ACK_EXP) {
        return false;
    }
    return true;
}
Parcel* callForwardToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_CallForwardInfo *cff = (RIL_CallForwardInfo *) data;
    parcel->writeInt32(cff->status);
    parcel->writeInt32(cff->reason);
    parcel->writeInt32(cff->serviceClass);
    parcel->writeInt32(cff->toa);

    pString16 = strdup8to16(cff->number, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(cff->timeSeconds);

    return parcel;
}

Parcel* cdmaBrSmsCnfToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    int num = datalen/sizeof(RIL_CDMA_BroadcastSmsConfigInfo *);
    parcel->writeInt32(num);

    RIL_CDMA_BroadcastSmsConfigInfo **cdmaBciPtrs = (RIL_CDMA_BroadcastSmsConfigInfo **) data;
    for (int i = 0; i < num; i++) {
        parcel->writeInt32(cdmaBciPtrs[i]->service_category);
        parcel->writeInt32(cdmaBciPtrs[i]->language);
        parcel->writeInt32(cdmaBciPtrs[i]->selected);
    }

    return parcel;
}

Parcel* cdmaSmsToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    RIL_CDMA_SMS_Message *rcsm = (RIL_CDMA_SMS_Message *) data;
    return cdmaSmsToParcelInternal(parcel, rcsm);
}

Parcel* cdmaSmsAckToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    RIL_CDMA_SMS_Ack *rcsa = (RIL_CDMA_SMS_Ack *) data;
    parcel->writeInt32(rcsa->uErrorClass);
    parcel->writeInt32(rcsa->uSMSCauseCode);

    return parcel;
}

Parcel* cdmaSubscriptionSourceToParcel(int request, int token,
        void* data, int datalen) {
    RFX_UNUSED(data);
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    fillHeader(parcel, request, token);
    return parcel;
}

Parcel* dataCallToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    int countStrings = datalen/sizeof(char *);  // 8; // according to RIL.java
    char **pStrings = (char **) data;
    parcel->writeInt32(countStrings);
    for (int i = 0; i < countStrings; i++) {
        pString16 = strdup8to16(pStrings[i], &len16);
        parcel->writeString16(pString16, len16);
        free(pString16);
    }

    return parcel;
}

Parcel* dataProfileToParcel(int request, int token, void* data, int datalen) {
    // useless
    Parcel *parcel = new Parcel();

    char16_t *pStringApn;
    char16_t *pStringProtocol;
    char16_t *pStringRoamingProtocol;
    char16_t *pStringUser;
    char16_t *pStringPassword;
    char16_t *pStringMvnoType;
    char16_t *pStringMvnoMatchData;

    size_t len16 = 0;
    fillHeader(parcel, request, token);

    int num = datalen / sizeof(RIL_MtkDataProfileInfo *);

    parcel->writeInt32(num);
    RIL_MtkDataProfileInfo **DataProfilePtrs = (RIL_MtkDataProfileInfo **) data;
    for (int i = 0; i < num; i++) {
        parcel->writeInt32(DataProfilePtrs[i]->profileId);

        pStringApn = strdup8to16(DataProfilePtrs[i]->apn, &len16);
        parcel->writeString16(pStringApn, len16);

        pStringProtocol = strdup8to16(DataProfilePtrs[i]->protocol, &len16);
        parcel->writeString16(pStringProtocol, len16);

        pStringRoamingProtocol = strdup8to16(DataProfilePtrs[i]->roamingProtocol, &len16);
        parcel->writeString16(pStringRoamingProtocol, len16);

        parcel->writeInt32(DataProfilePtrs[i]->authType);

        pStringUser = strdup8to16(DataProfilePtrs[i]->user, &len16);
        parcel->writeString16(pStringUser, len16);

        pStringPassword = strdup8to16(DataProfilePtrs[i]->password, &len16);
        parcel->writeString16(pStringPassword, len16);

        parcel->writeInt32(DataProfilePtrs[i]->type);
        parcel->writeInt32(DataProfilePtrs[i]->maxConnsTime);
        parcel->writeInt32(DataProfilePtrs[i]->maxConns);
        parcel->writeInt32(DataProfilePtrs[i]->waitTime);
        parcel->writeInt32(DataProfilePtrs[i]->enabled);
        parcel->writeInt32(DataProfilePtrs[i]->supportedTypesBitmask);
        parcel->writeInt32(DataProfilePtrs[i]->bearerBitmask);
        parcel->writeInt32(DataProfilePtrs[i]->mtu);

        pStringMvnoType = strdup8to16(DataProfilePtrs[i]->mvnoType, &len16);
        parcel->writeString16(pStringMvnoType, len16);

        pStringMvnoMatchData = strdup8to16(DataProfilePtrs[i]->mvnoMatchData, &len16);
        parcel->writeString16(pStringMvnoMatchData, len16);

        parcel->writeInt32(DataProfilePtrs[i]->inactiveTimer);

        free(pStringApn);
        free(pStringProtocol);
        free(pStringRoamingProtocol);
        free(pStringUser);
        free(pStringPassword);
        free(pStringMvnoType);
        free(pStringMvnoMatchData);
    }
    return parcel;
}

Parcel* dialToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_Dial *dial = (RIL_Dial *) data;

    pString16 = strdup8to16(dial->address, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(dial->clir);
    if (dial->uusInfo != NULL) {
        parcel->writeInt32(1);
        parcel->writeInt32(dial->uusInfo->uusType);
        parcel->writeInt32(dial->uusInfo->uusDcs);
        if (dial->uusInfo->uusLength == 0) {
            parcel->writeInt32(-1);
        } else {
            parcel->writeInt32(dial->uusInfo->uusLength);
            parcel->write( (void*) dial->uusInfo->uusData, dial->uusInfo->uusLength);
        }
    } else {
        parcel->writeInt32(0);
    }

    return parcel;
}

Parcel* emergencyDialToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_Emergency_Dial *emergencyDial = (RIL_Emergency_Dial *) data;

    if (emergencyDial->dialData != NULL) {
        pString16 = strdup8to16(emergencyDial->dialData->address, &len16);
        parcel->writeString16(pString16, len16);
        free(pString16);

        parcel->writeInt32(emergencyDial->dialData->clir);
        if (emergencyDial->dialData->uusInfo != NULL) {
            parcel->writeInt32(1);
            parcel->writeInt32(emergencyDial->dialData->uusInfo->uusType);
            parcel->writeInt32(emergencyDial->dialData->uusInfo->uusDcs);
            if (emergencyDial->dialData->uusInfo->uusLength == 0) {
                parcel->writeInt32(-1);
            } else {
                parcel->writeInt32(emergencyDial->dialData->uusInfo->uusLength);
                parcel->write( (void*) emergencyDial->dialData->uusInfo->uusData,
                        emergencyDial->dialData->uusInfo->uusLength);
            }
        } else {
            parcel->writeInt32(0);
        }
    }

    parcel->writeInt32(emergencyDial->serviceCategory);

    parcel->writeInt32(emergencyDial->routing);

    parcel->writeInt32(emergencyDial->isTesting);

    return parcel;
}

Parcel* gsmBrSmsCnfToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    int num = datalen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    parcel->writeInt32(num);
    RIL_GSM_BroadcastSmsConfigInfo **gsmBciPtrs = (RIL_GSM_BroadcastSmsConfigInfo **) data;
    for (int i = 0; i < num; i++) {
        parcel->writeInt32(gsmBciPtrs[i]->fromServiceId);
        parcel->writeInt32(gsmBciPtrs[i]->toServiceId);
        parcel->writeInt32(gsmBciPtrs[i]->fromCodeScheme);
        parcel->writeInt32(gsmBciPtrs[i]->toCodeScheme);
        parcel->writeInt32(gsmBciPtrs[i]->selected);
    }

    return parcel;
}

Parcel* imsCdmaSmsToParcel(Parcel *parcel, RIL_CDMA_SMS_Message *rcsm) {
    return cdmaSmsToParcelInternal(parcel, rcsm);
}

Parcel* imsGsmSmsToParcel(Parcel *parcel, void **gsmMessage, int datalen) {
    int **args = (int **)gsmMessage;
    char16_t *pString16;
    size_t len16 = 0;

    int32_t countStrings =  (datalen - sizeof(RIL_RadioTechnologyFamily) - sizeof(uint8_t) -
            sizeof(int32_t))/sizeof(char *);  // 2; // according to RIL.java
    // args = args + 1;
    char **pStrings = (char **)args;
    if (pStrings == NULL) {
        parcel->writeInt32(-1);
    } else {
        parcel->writeInt32(countStrings);
        for (int i = 0; i < countStrings; i++) {
            pString16 = strdup8to16(pStrings[i], &len16);
            parcel->writeString16(pString16, len16);
            free(pString16);
        }
    }
    return parcel;
}

Parcel* imsSmsToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    RIL_IMS_SMS_Message *rism = (RIL_IMS_SMS_Message *) data;

    parcel->writeInt32(rism->tech);
    parcel->write(&(rism->retry), sizeof(uint8_t));
    parcel->write(&(rism->messageRef), sizeof(int32_t));
    if (RADIO_TECH_3GPP == rism->tech) {
        return imsGsmSmsToParcel(parcel, (void **)rism->message.gsmMessage, datalen);
    } else if (RADIO_TECH_3GPP2 == rism->tech) {
        return imsCdmaSmsToParcel(parcel, (RIL_CDMA_SMS_Message *) rism->message.cdmaMessage);
    } else {
        RFX_LOG_E(LOG_TAG, "should not be here!");
        return parcel;
    }
}

Parcel* intsToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    int *pInts = (int *) data;
    int countInts = datalen/sizeof(int);
    parcel->writeInt32(countInts);
    for (int i = 0; i < countInts; i++) {
        parcel->writeInt32(pInts[i]);
    }

    return parcel;
}

Parcel* nvReadItemToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    RIL_NV_ReadItem *nvri = (RIL_NV_ReadItem *) data;
    parcel->writeInt32(nvri->itemID);

    return parcel;
}

Parcel* nvWriteItemToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_NV_WriteItem *nvwi = (RIL_NV_WriteItem *) data;
    parcel->writeInt32(nvwi->itemID);

    pString16 = strdup8to16(nvwi->value, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* radioCapabilityToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_RadioCapability *rc = (RIL_RadioCapability *) data;
    parcel->writeInt32(rc->version);
    parcel->writeInt32(rc->session);
    parcel->writeInt32(rc->phase);
    parcel->writeInt32(rc->rat);

    pString16 = strdup8to16(rc->logicalModemUuid, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(rc->status);

    return parcel;
}

Parcel* rawToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    if (data == NULL) {
        parcel->writeInt32(-1);
    } else {
        parcel->writeInt32(datalen);
        parcel->write(data, datalen);
    }

    return parcel;
}

Parcel* rilCdmaSmsWriteArgsToParcel(int request, int token,
        void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    int digitCount;
    uint8_t uct;
    int digitLimit;

    fillHeader(parcel, request, token);
    RIL_CDMA_SMS_WriteArgs *rcsw = (RIL_CDMA_SMS_WriteArgs *) data;
    parcel->writeInt32(rcsw->status);
    parcel->writeInt32(rcsw->message.uTeleserviceID);
    parcel->write(&(rcsw->message.bIsServicePresent), sizeof(uct));
    parcel->writeInt32(rcsw->message.uServicecategory);
    parcel->writeInt32(rcsw->message.sAddress.digit_mode);
    parcel->writeInt32(rcsw->message.sAddress.number_mode);
    parcel->writeInt32(rcsw->message.sAddress.number_type);
    parcel->writeInt32(rcsw->message.sAddress.number_plan);
    parcel->write(&(rcsw->message.sAddress.number_of_digits), sizeof(uct));

    digitLimit= MIN(rcsw->message.sAddress.number_of_digits, RIL_CDMA_SMS_ADDRESS_MAX);
    for (digitCount = 0; digitCount < digitLimit; digitCount++) {
        parcel->write(&(rcsw->message.sAddress.digits[digitCount]), sizeof(uct));
    }
    parcel->writeInt32(rcsw->message.sSubAddress.subaddressType);
    parcel->write(&(rcsw->message.sSubAddress.odd), sizeof(uint8_t));
    parcel->write(&(rcsw->message.sSubAddress.number_of_digits), sizeof(uct));

    digitLimit= MIN(rcsw->message.sSubAddress.number_of_digits, RIL_CDMA_SMS_SUBADDRESS_MAX);
    for (digitCount = 0 ; digitCount < digitLimit; digitCount ++) {
        parcel->write(&(rcsw->message.sSubAddress.digits[digitCount]), sizeof(uct));
    }
    parcel->writeInt32(rcsw->message.uBearerDataLen);

    digitLimit= MIN(rcsw->message.uBearerDataLen, RIL_CDMA_SMS_BEARER_DATA_MAX);
    for (digitCount = 0 ; digitCount < digitLimit; digitCount ++) {
        parcel->write(&(rcsw->message.aBearerData[digitCount]), sizeof(uct));
    }

    return parcel;
}

Parcel* setInitialAttachApnToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_InitialAttachApn_v15 *pf = (RIL_InitialAttachApn_v15 *) data;
    pString16 = strdup8to16(pf->apn, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(pf->protocol, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(pf->roamingProtocol, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(pf->authtype);

    pString16 = strdup8to16(pf->username, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(pf->password, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(pf->supportedTypesBitmask);
    parcel->writeInt32(pf->bearerBitmask);
    parcel->writeInt32(pf->modemCognitive);
    parcel->writeInt32(pf->mtu);

    pString16 = strdup8to16(pf->mvnoType, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(pf->mvnoMatchData, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(pf->canHandleIms);
    return parcel;
}

Parcel* simAuthenticationToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_SimAuthentication *pf = (RIL_SimAuthentication *) data;
    parcel->writeInt32(pf->authContext);

    pString16 = strdup8to16(pf->authData, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(pf->aid, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

// MTK-START: SIM GBA
Parcel* simAuthToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_SimAuthStructure *args = (RIL_SimAuthStructure *) data;
    parcel->writeInt32(args->sessionId);
    parcel->writeInt32(args->mode);

    pString16 = strdup8to16(args->param1, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(args->param2, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    if (args->mode == 1) {
        parcel->writeInt32(args->tag);
    }

    return parcel;
}
// MTK-END

Parcel* sim_APDUToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_SIM_APDU *apdu = (RIL_SIM_APDU *) data;
    parcel->writeInt32(apdu->sessionid);
    parcel->writeInt32(apdu->cla);
    parcel->writeInt32(apdu->instruction);
    parcel->writeInt32(apdu->p1);
    parcel->writeInt32(apdu->p2);
    parcel->writeInt32(apdu->p3);

    pString16 = strdup8to16(apdu->data, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* sim_IOToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_SIM_IO_v6 *v6 = (RIL_SIM_IO_v6 *) data;
    parcel->writeInt32(v6->command);
    parcel->writeInt32(v6->fileid);

    pString16 = strdup8to16(v6->path, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(v6->p1);
    parcel->writeInt32(v6->p2);
    parcel->writeInt32(v6->p3);
    pString16 = strdup8to16(v6->data, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    pString16 = strdup8to16(v6->pin2, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    pString16 = strdup8to16(v6->aidPtr, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* smsWriteToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_SMS_WriteArgs *args = (RIL_SMS_WriteArgs *) data;
    parcel->writeInt32(args->status);
    pString16 = strdup8to16(args->pdu, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    pString16 = strdup8to16(args->smsc, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* stringToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);

    char *pString = (char *) data;
    pString16 = strdup8to16(pString, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* stringsToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    int countStrings = datalen / sizeof(char *);
    parcel->writeInt32(countStrings);
    char **pString = (char **) data;
    for (int i = 0; i < countStrings; i++) {
        pString16 = strdup8to16(pString[i], &len16);
        parcel->writeString16(pString16, len16);
        free(pString16);
    }

    return parcel;
}

Parcel* uiccSubscriptonToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_SelectUiccSub *uicc_sub = (RIL_SelectUiccSub *) data;
    parcel->writeInt32(uicc_sub->slot);
    parcel->writeInt32(uicc_sub->app_index);
    parcel->writeInt32(uicc_sub->sub_type);
    parcel->writeInt32(uicc_sub->act_status);

    return parcel;
}

Parcel* voiceRadioTechToParcel(int request, int token, void* data __unused,
        int datalen __unused) {
    Parcel *parcel = new Parcel();
    fillHeader(parcel, request, token);
    return parcel;
}

Parcel* voidToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(data);
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    fillHeader(parcel, request, token);
    return parcel;
}

Parcel* writePbToRuimToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    char **pStrings = (char **) data;
    parcel->writeInt32(*pStrings[0]);

    pString16 = strdup8to16(pStrings[1], &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(*pStrings[2]);

    pString16 = strdup8to16(pStrings[3], &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(*pStrings[4]);
    for (int i = 0; i < 5; i++) {
        pString16 = strdup8to16(pStrings[i], &len16);
        parcel->writeString16(pString16, len16);
        free(pString16);
    }

    return parcel;
}

Parcel* cdmaSmsToParcelInternal(Parcel *parcel,
        RIL_CDMA_SMS_Message *rcsm) {
    int32_t digitCount;
    int digitLimit;
    parcel->writeInt32(rcsm->uTeleserviceID);
    parcel->write(&(rcsm->bIsServicePresent), sizeof(uint8_t));
    parcel->writeInt32(rcsm->uServicecategory);
    parcel->writeInt32((rcsm->sAddress).digit_mode);
    parcel->writeInt32((rcsm->sAddress).number_mode);
    parcel->writeInt32((rcsm->sAddress).number_type);
    parcel->writeInt32((rcsm->sAddress).number_plan);
    parcel->writeInt32((rcsm->sAddress).number_of_digits);
    digitLimit = MIN((rcsm->sAddress).number_of_digits, RIL_CDMA_SMS_ADDRESS_MAX);
    for (digitCount = 0; digitCount < digitLimit; digitCount++) {
        parcel->write(&((rcsm->sAddress).digits[digitCount]), sizeof(uint8_t));
    }
    parcel->writeInt32((rcsm->sSubAddress).subaddressType);
    parcel->write(&((rcsm->sSubAddress).odd), sizeof(uint8_t));
    parcel->write(&((rcsm->sSubAddress).number_of_digits), sizeof(uint8_t));
    digitLimit = MIN(((rcsm->sSubAddress).number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for (digitCount = 0; digitCount < digitLimit; digitCount++) {
        parcel->write(&((rcsm->sSubAddress).digits[digitCount]), sizeof(uint8_t));
    }
    parcel->writeInt32(rcsm->uBearerDataLen);
    digitLimit = MIN((rcsm->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for (digitCount = 0; digitCount < digitLimit; digitCount++) {
        parcel->write(&(rcsm->aBearerData[digitCount]), sizeof(uint8_t));
    }
    return parcel;
}

Parcel* carrierRestrictionsToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_CarrierRestrictionsWithPriority *args = (RIL_CarrierRestrictionsWithPriority *) data;
    parcel->writeInt32(args->len_allowed_carriers);
    parcel->writeInt32(args->len_excluded_carriers);
    for (int i = 0; i < args->len_allowed_carriers; i++) {
        carrierToParcelInternal(parcel, args->allowed_carriers+i);
    }
    for (int i = 0; i < args->len_excluded_carriers; i++) {
        carrierToParcelInternal(parcel, args->excluded_carriers+i);
    }
    parcel->writeBool(args->allowedCarriersPrioritized);
    parcel->writeInt32(args->simLockMultiSimPolicy);
    return parcel;
}

Parcel* carrierToParcelInternal(Parcel *parcel, RIL_Carrier *carrier) {
    char16_t *pString16;
    size_t len16 = 0;

    pString16 = strdup8to16(carrier->mcc, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(carrier->mnc, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(carrier->match_type);

    pString16 = strdup8to16(carrier->match_data, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    return parcel;
}

// FastDormancy
Parcel* fdModeToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);

    Parcel *parcel = new Parcel();
    fillHeader(parcel, request, token);
    RIL_FdModeStructure *args = (RIL_FdModeStructure *) data;
    parcel->writeInt32(args->paramNumber);
    if (args->paramNumber >= 1) {
        parcel->writeInt32(args->mode);
    }
    if (args->paramNumber >= 2) {
        parcel->writeInt32(args->parameter1);
    }
    if (args->paramNumber >= 3) {
        parcel->writeInt32(args->parameter2);
    }

    return parcel;
}

// PHB START
Parcel* phbEntryToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_PhbEntryStructure *args = (RIL_PhbEntryStructure *) data;
    parcel->writeInt32(args->type);
    parcel->writeInt32(args->index);

    pString16 = strdup8to16(args->number, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(args->ton);

    pString16 = strdup8to16(args->alphaId, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

Parcel* writePhbEntryExtToParcel(int request, int token, void* data,
        int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_PHB_ENTRY *args = (RIL_PHB_ENTRY *) data;
    parcel->writeInt32(args->index);

    pString16 = strdup8to16(args->number, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(args->type);

    pString16 = strdup8to16(args->text, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(args->hidden);

    pString16 = strdup8to16(args->group, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    pString16 = strdup8to16(args->adnumber, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(args->adtype);

    pString16 = strdup8to16(args->secondtext, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    pString16 = strdup8to16(args->email, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}
// PHB END

void fillHeader(Parcel *parcel, int request, int token) {
    // RFX_LOG_V(LOG_TAG, "fillHeader(): request = %d, token = %d", request, token);
    parcel->writeInt32(request);
    parcel->writeInt32(token);
}

static int getTypeAndJumpToData(Parcel *parcel, int request) {
    RFX_UNUSED(request);
    // RFX_LOG_V(LOG_TAG, "getTypeAndJumpToData(): request = %d", request);
    int32_t type = 0;
    int32_t token = 0;
    int32_t err = 0;
    int32_t urcId = 0;
    parcel->setDataPosition(0);
    parcel->readInt32(&type);
    if(isResponseType(type)) {
        // Response
        parcel->readInt32(&token);
        parcel->readInt32(&err);
        // RFX_LOG_V(LOG_TAG, "response: request = %d, token = %d, err %d", request, token, err);
    } else {
        parcel->readInt32(&urcId);
        // RFX_LOG_V(LOG_TAG, "urc: request = %d, urcId = %d", request, urcId);
    }
    return type;
}

static void
invalidCommandBlock (int requestId) {
    RFX_LOG_D(LOG_TAG, "Invalid command block for request %d", requestId);
}

static char *
strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static int
readStringFromParcelInplace(Parcel *p, char *str, size_t maxLen) {
    size_t s16Len;
    const char16_t *s16;

    s16 = p->readString16Inplace(&s16Len);
    if (s16 == NULL) {
        return 0;
    }
    size_t strLen = strnlen16to8(s16, s16Len);
    if ((strLen + 1) > maxLen) {
        return 0;
    }
    if (strncpy16to8(str, s16, strLen) == NULL) {
        return 0;
    } else {
        return 1;
    }
}

static void
memsetString (char *s) {
    if (s != NULL) {
        memset (s, 0, strlen(s));
    }
}

// TODO
//static void parcelToUssdStrings(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
//}

static void parcelToInts(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t count = 0;
    size_t datalen = 0;
    int *pInts = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32 (&count);

        // RFX_LOG_V(LOG_TAG, "parcelToInts request count = %d", count);
        if (count == 0) {
            goto invalid;
        }

        datalen = sizeof(int) * count;
        pInts = (int *)alloca(datalen);
        if (pInts == NULL) {
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        for (int i = 0 ; i < count ; i++) {
            int32_t v = 0;

            p->readInt32(&v);
            pInts[i] = (int)v;
            // RFX_LOG_V(LOG_TAG, "parcelToInts request pInts[%d] = %d", i, pInts[i]);
       }
   }

   // RFX_LOG_V(LOG_TAG, "parcelToInts request %d, type %d", id, type);
   if (isResponseType(type)) {
       // Response
       if (e == 0 || pInts != NULL) {
           RfxRilAdapter::responseToRilj(t, e, const_cast<int *>(pInts), datalen);
       } else {
           RfxRilAdapter::responseToRilj(t, e, NULL, 0);
       }
   } else {
       // URC
       RESPONSE_TO_RILJ(id, const_cast<int *>(pInts), datalen, RIL_SOCKET_ID(slotId));
   }

#ifdef MEMSET_FREED
    memset(pInts, 0, datalen);
#endif

    return;
invalid:
   // Some modules hope response can send to RILJ, then they can do some error handle in RILJ.
   if (isResponseType(type) && e != 0) {
       RfxRilAdapter::responseToRilj(t, e, NULL, 0);
   }
    invalidCommandBlock(id);
    return;
}

static void parcelToFailCause(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_LastCallFailCauseInfo *info = NULL;
    int32_t res = 0;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        info = (RIL_LastCallFailCauseInfo *) calloc(1, sizeof(RIL_LastCallFailCauseInfo));
        if (info == NULL) {
            RFX_LOG_E(LOG_TAG, "[RFX] OOM");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        p->readInt32(&res);
        info->cause_code = (RIL_LastCallFailCause) res;
        info->vendor_cause = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToFailCause request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || info != NULL) {
            RfxRilAdapter::responseToRilj(t, e, info, sizeof(RIL_LastCallFailCauseInfo));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, info, sizeof(RIL_LastCallFailCauseInfo), RIL_SOCKET_ID(slotId));
    }

    if (info != NULL) {
        free(info->vendor_cause);
        free(info);
    }
}

static void parcelToStrings(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t countStrings = 0;
    size_t datalen = 0;
    char **pStrings = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32 (&countStrings);

        if (countStrings == 0) {
            // just some non-null pointer
            pStrings = (char **)alloca(sizeof(char *));
            if (pStrings == NULL) {
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                return;
            }
            datalen = 0;
        } else if (((int)countStrings) == -1) {
            pStrings = NULL;
            datalen = 0;
        } else {
            datalen = sizeof(char *) * countStrings;

            pStrings = (char **)alloca(datalen);
            if (pStrings == NULL) {
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                return;
            }
            for (int i = 0 ; i < countStrings ; i++) {
                pStrings[i] = strdupReadString(p);
            }
        }
    }

    RFX_LOG_V(LOG_TAG, "parcelToStrings e = %d, datalen = %zu", e, datalen);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || pStrings != NULL) {
            RfxRilAdapter::responseToRilj(t, e, pStrings,datalen);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, pStrings, (int)datalen, RIL_SOCKET_ID(slotId));
    }

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString(pStrings[i]);
#endif
            if (pStrings[i] != NULL) {
                free(pStrings[i]);
            }
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
    }

    return;
}

static void parcelToString(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    size_t datalen;
    size_t stringlen;
    char *string8 = NULL;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        string8 = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToString request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 && string8 != NULL) {
            RfxRilAdapter::responseToRilj(t, e, string8, strlen(string8));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, string8, strlen(string8), RIL_SOCKET_ID(slotId));
    }

#ifdef MEMSET_FREED
    if (string8 != NULL) {
        memsetString(string8);
    }
#endif

    if (string8 != NULL) {
        free(string8);
    }
    return;
}

static void parcelToVoid(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);
    // RFX_LOG_V(LOG_TAG, "parcelToVoid request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        RfxRilAdapter::responseToRilj(t, e, NULL, 0);
    } else {
        // URC
        RESPONSE_TO_RILJ(id, NULL, 0, RIL_SOCKET_ID(slotId));
    }
    return;
}

static void parcelToUnused(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);
    RFX_UNUSED(t);
    RFX_UNUSED(e);
    RFX_UNUSED(slotId);
    RFX_LOG_E(LOG_TAG, "parcelToUnused request %d, type %d. should not be here.", id, type);
    return;
}

static void parcelToCallList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int num = 0;
    int uusPresent;
    RIL_Call **pCallLists = NULL;
    RIL_Call *pCallList = NULL;
    int32_t res;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        pCallLists = (RIL_Call **) calloc(1, sizeof(RIL_Call *) * num);
        if (pCallLists == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToCallList pCallLists calloc failed.");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        for (int i = 0; i < num; i++) {
            pCallList = (RIL_Call *) calloc(1, sizeof(RIL_Call));
            if (pCallList == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToCallList pCallList (%d) calloc failed.", i);
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                return;
            }
            pCallLists[i] = pCallList;

            p->readInt32(&res);
            pCallLists[i]->state = (RIL_CallState) res;
            p->readInt32(&pCallLists[i]->index);
            p->readInt32(&pCallLists[i]->toa);
            p->readInt32(&res);
            pCallLists[i]->isMpty = (uint8_t) res;
            p->readInt32(&res);
            pCallLists[i]->isMT = (uint8_t) res;
            p->readInt32(&res);
            pCallLists[i]->als = (uint8_t) res;
            p->readInt32(&res);
            pCallLists[i]->isVoice = (uint8_t) res;
            p->readInt32(&res);
            pCallLists[i]->isVoicePrivacy = (uint8_t) res;
            pCallLists[i]->number = strdupReadString(p);
            p->readInt32(&pCallLists[i]->numberPresentation);
            pCallLists[i]->name = strdupReadString(p);
            p->readInt32(&pCallLists[i]->namePresentation);
            p->readInt32(&uusPresent);
            if (uusPresent == 0) {
                /* UUS Information is absent */
            } else {
                /* UUS Information is present */
                p->readInt32(&res);
                pCallLists[i]->uusInfo->uusType = (RIL_UUS_Type) res;
                p->readInt32(&res);
                pCallLists[i]->uusInfo->uusDcs = (RIL_UUS_DCS) res;
                p->readInt32(&pCallLists[i]->uusInfo->uusLength);
                p->read(&pCallLists[i]->uusInfo->uusData, pCallLists[i]->uusInfo->uusLength);
            }
            p->readInt32(&pCallLists[i]->speechCodec);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCallList request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || pCallLists != NULL) {
            RfxRilAdapter::responseToRilj(t, e, pCallLists, sizeof(RIL_Call *) * num);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, pCallLists, sizeof(RIL_Call *) * num, RIL_SOCKET_ID(slotId));
    }

    //free(pCallList);
    if (pCallLists != NULL) {
        for (int i = 0; i < num; i++ ) {
            if (pCallLists[i] != NULL) {
                free(pCallLists[i]->number);
                free(pCallLists[i]->name);
                free(pCallLists[i]);
            }
        }
        free(pCallLists);
    }
}

static void parcelToSMS(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SMS_Response *pSMS = NULL;

    int type = getTypeAndJumpToData(p, id);

    // Even if send sms is failed, it should response to framework with correct
    // error code
    if (p->dataAvail() > 0) {
        pSMS = (RIL_SMS_Response *) calloc(1, sizeof(RIL_SMS_Response));
        if (pSMS == NULL) {
            RFX_LOG_E(LOG_TAG, "[RFX] OOM");
            if (isResponseType(type)) {
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            }
            return;
        }
        p->readInt32(&pSMS->messageRef);
        pSMS->ackPDU = strdupReadString(p);
        p->readInt32(&pSMS->errorCode);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToSMS request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || pSMS != NULL) {
            RfxRilAdapter::responseToRilj(t, e, pSMS, sizeof(RIL_SMS_Response));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, pSMS, sizeof(RIL_SMS_Response), RIL_SOCKET_ID(slotId));
    }

    if (pSMS != NULL && pSMS->ackPDU != NULL) {
        free(pSMS->ackPDU);
    }
    if (pSMS != NULL) {
        free(pSMS);
    }
}

static void parcelToSIM_IO(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SIM_IO_Response *simIO = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        simIO = (RIL_SIM_IO_Response *) calloc(1, sizeof(RIL_SIM_IO_Response));
        RFX_ASSERT(simIO != NULL);
        p->readInt32(&simIO->sw1);
        p->readInt32(&simIO->sw2);
        simIO->simResponse = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToSIM_IO request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || simIO != NULL) {
            RfxRilAdapter::responseToRilj(t, e, simIO, sizeof(RIL_SIM_IO_Response));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, simIO, sizeof(RIL_SIM_IO_Response), RIL_SOCKET_ID(slotId));
    }

    if (simIO != NULL && simIO->simResponse != NULL) {
        free(simIO->simResponse);
    }
    if (simIO != NULL) {
        free(simIO);
    }
}

static void parcelToCallForwards(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CallForwardInfo **infos = NULL;
    RIL_CallForwardInfo *info = NULL;
    int32_t num = 0;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        infos = (RIL_CallForwardInfo **) calloc(1, sizeof(RIL_CallForwardInfo *) * num);
        if (infos == NULL) {
            RLOGE("parcelToCallForwards, infos malloc fail");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        for (int i = 0; i < num; i++) {
            info = (RIL_CallForwardInfo*)calloc(1, sizeof(RIL_CallForwardInfo));
            if (info == NULL) {
                RLOGE("parcelToCallForwards, info malloc fail");
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                return;
            }
            infos[i] = info;

            p->readInt32(&infos[i]->status);
            p->readInt32(&infos[i]->reason);
            p->readInt32(&infos[i]->serviceClass);
            p->readInt32(&infos[i]->toa);
            infos[i]->number = strdupReadString(p);
            p->readInt32(&infos[i]->timeSeconds);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCallForwards request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || infos != NULL) {
            RfxRilAdapter::responseToRilj(t, e, infos, sizeof(RIL_CallForwardInfo *) * num);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, infos, sizeof(RIL_CallForwardInfo *) * num, RIL_SOCKET_ID(slotId));
    }

    for (int i = 0; i < num; i++) {
        free(infos[i]->number);
        free(infos[i]);
    }
    free(infos);
}

static void parcelToDataCallList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    MTK_RIL_Data_Call_Response_v11 *list = NULL;
    int32_t num = 0;
    int32_t version = 0;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32(&version);
        p->readInt32(&num);
        list = (MTK_RIL_Data_Call_Response_v11 *) calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11) * num);
        RFX_ASSERT(list != NULL);
        for (int i = 0; i < num; i++) {
            p->readInt32(&list[i].status);
            p->readInt32(&list[i].suggestedRetryTime);
            p->readInt32(&list[i].cid);
            p->readInt32(&list[i].active);
            list[i].type = strdupReadString(p);
            list[i].ifname = strdupReadString(p);
            list[i].addresses = strdupReadString(p);
            list[i].dnses = strdupReadString(p);
            list[i].gateways = strdupReadString(p);
            list[i].pcscf = strdupReadString(p);
            p->readInt32(&list[i].mtu);
            p->readInt32(&list[i].rat);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToDataCallList request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || list != NULL) {
            RfxRilAdapter::responseToRilj(t, e, list, sizeof(MTK_RIL_Data_Call_Response_v11) * num);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, list, sizeof(MTK_RIL_Data_Call_Response_v11) * num,
                RIL_SOCKET_ID(slotId));
    }

    for (int i = 0; i < num; i++) {
        free(list[i].type);
        free(list[i].ifname);
        free(list[i].addresses);
        free(list[i].dnses);
        free(list[i].gateways);
    }
    free(list);
}

static void parcelToSetupDataCall(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    parcelToDataCallList(t, e, id, p, slotId);
}

static void parcelToRaw(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int headerSize = 0;
    int size = 0;
    char* line = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        if (isResponseType(type)) {
            // response
            headerSize = 3;
        } else {
            // urc
            headerSize = 2;
        }

        if((int)p->dataSize() > headerSize) {
            p->readInt32(&size);
            line = (char *) calloc(1, size);
            if (line == NULL) {
                RFX_LOG_E(LOG_TAG, "[RFX] OOM");
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                return;
            }
            p->read(line, size);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToRaw request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || line != NULL) {
        RfxRilAdapter::responseToRilj(t, e, line, size);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, line, size, RIL_SOCKET_ID(slotId));
    }
    if (line != NULL) {
        free(line);
    }
}

static void parcelToSsn(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SuppSvcNotification *p_cur = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p_cur = (RIL_SuppSvcNotification*)calloc(1,
            sizeof(RIL_SuppSvcNotification));
        if (p_cur == NULL) {
            RLOGE("parcelToSsn, p_cur malloc fail");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        p->readInt32(&p_cur->notificationType);
        p->readInt32(&p_cur->code);
        p->readInt32(&p_cur->index);
        p->readInt32(&p_cur->type);
        p_cur->number = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToSsn request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_SuppSvcNotification));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_SuppSvcNotification), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        free(p_cur->number);
        free(p_cur);
    }
}

static void parcelToSimStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CardStatus_v8 *cardStatus = NULL;
    int32_t v = 0;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        cardStatus = (RIL_CardStatus_v8 *)calloc(1, sizeof(RIL_CardStatus_v8));
        RFX_ASSERT(cardStatus != NULL);
        p->readInt32(&v);
        cardStatus->card_state = RIL_CardState(v);
        p->readInt32(&v);
        cardStatus->universal_pin_state = RIL_PinState(v);
        p->readInt32(&cardStatus->gsm_umts_subscription_app_index);
        p->readInt32(&cardStatus->cdma_subscription_app_index);
        p->readInt32(&cardStatus->ims_subscription_app_index);
        p->readInt32(&cardStatus->num_applications);
        for (int i = 0; i < cardStatus->num_applications; i++) {
            p->readInt32(&v);
            cardStatus->applications[i].app_type = RIL_AppType(v);
            p->readInt32(&v);
            cardStatus->applications[i].app_state = RIL_AppState(v);
            p->readInt32(&v);
            cardStatus->applications[i].perso_substate = RIL_PersoSubstate(v);
            cardStatus->applications[i].aid_ptr = strdupReadString(p);
            cardStatus->applications[i].app_label_ptr = strdupReadString(p);
            p->readInt32(&cardStatus->applications[i].pin1_replaced);
            p->readInt32(&v);
            cardStatus->applications[i].pin1 = RIL_PinState(v);
            p->readInt32(&v);
            cardStatus->applications[i].pin2 = RIL_PinState(v);
        }

        // Paramter add from radio hidl v1.2
        p->readInt32(&cardStatus->physicalSlotId);
        cardStatus->atr = strdupReadString(p);
        cardStatus->iccId = strdupReadString(p);

        // Paramter add from radio hidl v1.4
        cardStatus->eid = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToSimStatus request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || cardStatus != NULL) {
            RfxRilAdapter::responseToRilj(t, e, cardStatus, sizeof(RIL_CardStatus_v8));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, cardStatus, sizeof(RIL_CardStatus_v8), RIL_SOCKET_ID(slotId));
    }

    if (cardStatus != NULL) {
        for (int i = 0; i < cardStatus->num_applications; i++) {
            free(cardStatus->applications[i].aid_ptr);
            free(cardStatus->applications[i].app_label_ptr);
        }
        // Paramter add from radio hidl v1.2
        free(cardStatus->atr);
        free(cardStatus->iccId);
        // Paramter add from radio hidl v1.4
        free(cardStatus->eid);
        free(cardStatus);
    }
}

static void parcelToGsmBrSmsCnf(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);
    int32_t v = 0;
    int32_t num = 0;

    p->readInt32(&num);
    {
        RIL_GSM_BroadcastSmsConfigInfo gsmBci[num];
        RIL_GSM_BroadcastSmsConfigInfo *gsmBciPtrs[num];

        for (int i = 0 ; i < num ; i++ ) {
            gsmBciPtrs[i] = &gsmBci[i];

            p->readInt32(&v);
            gsmBci[i].fromServiceId = (int) v;

            p->readInt32(&v);
            gsmBci[i].toServiceId = (int) v;

            p->readInt32(&v);
            gsmBci[i].fromCodeScheme = (int) v;

            p->readInt32(&v);
            gsmBci[i].toCodeScheme = (int) v;

            p->readInt32(&v);
            gsmBci[i].selected = (uint8_t) v;
        }

        // RFX_LOG_V(LOG_TAG, "parcelToGsmBrSmsCnf request %d, type %d", id, type);
        if (isResponseType(type)) {
            // Response
            RfxRilAdapter::responseToRilj(t, e, gsmBciPtrs, num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*));
        } else {
            // URC
            RESPONSE_TO_RILJ(id, gsmBciPtrs, num*sizeof(RIL_GSM_BroadcastSmsConfigInfo*),
                    RIL_SOCKET_ID(slotId));
        }

#ifdef MEMSET_FREED
        memset(gsmBci, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
        memset(gsmBciPtrs, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *));
#endif
    }

    return;
}

static void parcelToCdmaBrSmsCnf(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);
    int32_t v = 0;
    int32_t num = 0;

    p->readInt32(&num);
    {
        RIL_CDMA_BroadcastSmsConfigInfo cdmaBci[num];
        RIL_CDMA_BroadcastSmsConfigInfo *cdmaBciPtrs[num];

        for (int i = 0 ; i < num ; i++ ) {
            cdmaBciPtrs[i] = &cdmaBci[i];

            p->readInt32(&v);
            cdmaBci[i].service_category= (int) v;

            p->readInt32(&v);
            cdmaBci[i].language= (int) v;

            p->readInt32(&v);
            cdmaBci[i].selected = (uint8_t) v;
        }

        // RFX_LOG_V(LOG_TAG, "parcelToCdmaBrSmsCnf request %d, type %d", id, type);
        if (isResponseType(type)) {
            // Response
            RfxRilAdapter::responseToRilj(t, e, cdmaBciPtrs, num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*));
        } else {
            // URC
            RESPONSE_TO_RILJ(id, cdmaBciPtrs, num*sizeof(RIL_CDMA_BroadcastSmsConfigInfo*),
                    RIL_SOCKET_ID(slotId));
        }

    #ifdef MEMSET_FREED
        memset(cdmaBci, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
        memset(cdmaBciPtrs, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *));
    #endif
    }

    return;
}

static void parcelToCdmaSms(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CDMA_SMS_Message *p_cur = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p_cur = (RIL_CDMA_SMS_Message*)calloc(1,
                sizeof(RIL_CDMA_SMS_Message));
        RFX_ASSERT(p_cur != NULL);
        uint8_t uct = 0;
        int digitCount = 0;
        int digitLimit = 0;
        int32_t  v = 0;
        p->readInt32(&p_cur->uTeleserviceID);
        p->read(&(p_cur->bIsServicePresent),sizeof(uct));
        p->readInt32(&p_cur->uServicecategory);
        p->readInt32(&v);
        p_cur->sAddress.digit_mode = RIL_CDMA_SMS_DigitMode(v);
        p->readInt32(&v);
        p_cur->sAddress.number_mode = RIL_CDMA_SMS_NumberMode(v);
        p->readInt32(&v);
        p_cur->sAddress.number_type = RIL_CDMA_SMS_NumberType(v);
        p->readInt32(&v);
        p_cur->sAddress.number_plan = RIL_CDMA_SMS_NumberPlan(v);
        p->read(&(p_cur->sAddress.number_of_digits), sizeof(uct));
        digitLimit= MIN((p_cur->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
        for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
            p->read(&(p_cur->sAddress.digits[digitCount]),sizeof(uct));
        }

        p->readInt32(&v);
        p_cur->sSubAddress.subaddressType = RIL_CDMA_SMS_SubaddressType(v);
        p->read(&(p_cur->sSubAddress.odd),sizeof(uct));
        p->read(&(p_cur->sSubAddress.number_of_digits),sizeof(uct));
        digitLimit= MIN((p_cur->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
        for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
            p->read(&(p_cur->sSubAddress.digits[digitCount]),sizeof(uct));
        }

        p->readInt32(&p_cur->uBearerDataLen);
        digitLimit= MIN((p_cur->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
        // RFX_LOG_V(LOG_TAG, "parcelToCdmaSms bearerdatalen =  %d", digitLimit);
        for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
           p->read(&(p_cur->aBearerData[digitCount]), sizeof(uct));
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCdmaSms request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_CDMA_SMS_Message));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_CDMA_SMS_Message), RIL_SOCKET_ID(slotId));
    }

    if (p_cur != NULL) {
        free(p_cur);
    }
}

static void parcelToCellList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_NeighboringCell *entry = NULL;
    RIL_NeighboringCell **entries = NULL;
    int32_t  v;
    int64_t  v6;
    int type = getTypeAndJumpToData(p, id);
    int32_t num = 0;

    if (p->dataAvail() > 0) {
        p->readInt32(&num);

        entries = (RIL_NeighboringCell**)calloc(1, sizeof(RIL_NeighboringCell*) * num);
        if (entries == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToCellList calloc entries fail");
            goto error;
        }
        for(int i = 0; i< num; i++) {
            entry = (RIL_NeighboringCell*)calloc(1, sizeof(RIL_NeighboringCell));
            if (entry == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToCellList calloc entry fail");
                goto error;
            }
            entries[i] = entry;

            p->readInt32(&entries[i]->rssi);
            entries[i]->cid = strdupReadString(p);
        }
    }
error:
    // RFX_LOG_V(LOG_TAG, "parcelToCellList request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || entries != NULL) {
            RfxRilAdapter::responseToRilj(t, e, entries, num*sizeof(RIL_NeighboringCell*));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, entries, num*sizeof(RIL_NeighboringCell*), RIL_SOCKET_ID(slotId));
    }

    if (entries != NULL) {
        for(int i = 0; i< num; i++) {
            if (entries[i] != NULL) {
                free(entries[i]->cid);
            }
        }
        free(entries);
    }
}

static void parcelToCdmaInformationRecords(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CDMA_InformationRecords *p_cur = NULL;
    char *buf = NULL;
    int32_t data;

    int type = getTypeAndJumpToData(p, id);
    int32_t numOfRecs = 0;

    if (p->dataAvail() > 0) {
        p_cur = (RIL_CDMA_InformationRecords*)calloc(1,
                sizeof(RIL_CDMA_InformationRecords));
        RFX_ASSERT(p_cur != NULL);

        p->readInt32(&numOfRecs);

        p_cur->numberOfInfoRecs = numOfRecs;

        for (int i = 0 ; i < numOfRecs ; i++) {
            p->readInt32(&data);
            p_cur->infoRec[i].name = (RIL_CDMA_InfoRecName)data;

            // RFX_LOG_V(LOG_TAG, "parcelToCdmaInformationRecords name %d", p_cur->infoRec[i].name);

            buf = NULL;
            switch (p_cur->infoRec[i].name) {
                case RIL_CDMA_DISPLAY_INFO_REC:
                case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC:
                    buf = strdupReadString(p);
                    p_cur->infoRec[i].rec.display.alpha_len = strlen(buf);
                    strncpy(p_cur->infoRec[i].rec.display.alpha_buf, buf,
                            CDMA_ALPHA_INFO_BUFFER_LENGTH - 1);
                    break;
                case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CONNECTED_NUMBER_INFO_REC:
                    buf = strdupReadString(p);
                    p_cur->infoRec[i].rec.number.len = strlen(buf);
                    strncpy(p_cur->infoRec[i].rec.number.buf, buf,
                            CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.number.number_type = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.number.number_plan = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.number.pi = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.number.si = data;
                    break;
                case RIL_CDMA_SIGNAL_INFO_REC:
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.signal.isPresent = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.signal.signalType = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.signal.alertPitch = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.signal.signal = data;
                    break;
                case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC:
                    buf = strdupReadString(p);
                    p_cur->infoRec[i].rec.redir.redirectingNumber.len = strlen(buf);
                    strncpy(p_cur->infoRec[i].rec.redir.redirectingNumber.buf, buf,
                            CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.redir.redirectingNumber.number_type = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.redir.redirectingNumber.number_plan = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.redir.redirectingNumber.pi = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.redir.redirectingNumber.si = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.redir.redirectingReason = (RIL_CDMA_RedirectingReason)data;
                    break;
                case RIL_CDMA_LINE_CONTROL_INFO_REC:
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.lineCtrl.lineCtrlPolarityIncluded = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.lineCtrl.lineCtrlToggle = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.lineCtrl.lineCtrlReverse = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.lineCtrl.lineCtrlPowerDenial = data;
                    break;
                case RIL_CDMA_T53_CLIR_INFO_REC:
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.clir.cause = data;
                    break;
                case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC:
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.audioCtrl.upLink = data;
                    p->readInt32(&data);
                    p_cur->infoRec[i].rec.audioCtrl.downLink = data;
                    break;
                default:
                    break;
            }
            if (buf != NULL) {
                free(buf);
            }
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCdmaInformationRecords request %d, type %d, parcelsize %d", id, type, p->dataSize());

    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_CDMA_InformationRecords));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_CDMA_InformationRecords), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        free(p_cur);
    }
}

static void parcelToRilSignalStrength(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SignalStrength_v10 *p_cur = NULL;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
         p_cur = (RIL_SignalStrength_v10*)calloc(1,
                sizeof(RIL_SignalStrength_v10));
         if (p_cur == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToRilSignalStrength calloc p_cur fail");
            goto error;
        }
        int32_t  v;
        // RFX_LOG_V(LOG_TAG, "parcelToRilSignalStrength request %d, type %d, parcelsize %d", id, type, p->dataSize());

        // Only used in Response
        if(p->dataSize() >= sizeof (RIL_SignalStrength_v5)+3) {
            p->readInt32(&p_cur->GW_SignalStrength.signalStrength);
            p->readInt32(&p_cur->GW_SignalStrength.bitErrorRate);
            p->readInt32(&p_cur->CDMA_SignalStrength.dbm);
            p->readInt32(&p_cur->CDMA_SignalStrength.ecio);
            p->readInt32(&p_cur->EVDO_SignalStrength.dbm);
            p->readInt32(&p_cur->EVDO_SignalStrength.ecio);
            p->readInt32(&p_cur->EVDO_SignalStrength.signalNoiseRatio);
            if (p->dataSize() >= sizeof(RIL_SignalStrength_v6)+3) {
                p->readInt32(&p_cur->LTE_SignalStrength.signalStrength);
                p->readInt32(&p_cur->LTE_SignalStrength.rsrp);
                p->readInt32(&p_cur->LTE_SignalStrength.rsrq);
                p->readInt32(&p_cur->LTE_SignalStrength.rssnr);
                p->readInt32(&p_cur->LTE_SignalStrength.cqi);
                if (p->dataSize() >= sizeof (RIL_SignalStrength_v10)+3) {
                    p->readInt32(&p_cur->TD_SCDMA_SignalStrength.rscp);
                    /*
                    if (p->dataSize() >= sizeof (RIL_SignalStrength_v11)+3) {
                        p->readInt32(&p_cur->WCDMA_SignalStrength.rssiQdbm);
                        p->readInt32(&p_cur->WCDMA_SignalStrength.rscpQdbm);
                        p->readInt32(&p_cur->WCDMA_SignalStrength.Ecn0Qdbm);
                    } else {
                        // no need to parse
                        //p->writeInt32(INT_MAX);
                        //p->writeInt32(INT_MAX);
                        //p->writeInt32(INT_MAX);
                    }
                    */
                } else {
                    // no need to parse
                    //p->writeInt32(INT_MAX);
                }
            } else {
            /* no need to parse
                p->writeInt32(99);
                p->writeInt32(INT_MAX);
                p->writeInt32(INT_MAX);
                p->writeInt32(INT_MAX);
                p->writeInt32(INT_MAX);
                p->writeInt32(INT_MAX); */
            }
        } else {
            RFX_LOG_D(LOG_TAG, "invalid response length");
            if (p_cur != NULL) {
                free(p_cur);
            }
            return;
        }
    }
error:
    // RFX_LOG_V(LOG_TAG, "parcelToRilSignalStrength request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_SignalStrength_v10));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_SignalStrength_v10), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        free(p_cur);
    }
}

/**
 * Marshall the signalInfoRecord into the parcel if it exists.
 */
static void readSignalInfoRecord(Parcel *p,
            RIL_CDMA_SignalInfoRecord &p_signalInfoRecord) {
    p->writeInt32(p_signalInfoRecord.isPresent);
    p->writeInt32(p_signalInfoRecord.signalType);
    p->writeInt32(p_signalInfoRecord.alertPitch);
    p->writeInt32(p_signalInfoRecord.signal);
}

static void parcelToCallRing(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);
    // RFX_LOG_V(LOG_TAG, "parcelToCallRing request %d, type %d, parcelsize %d", id, type, p->dataSize());

    // Only used in URC
    if(p->dataSize() == 2) {
        // No response data
    #if defined(ANDROID_MULTI_SIM)
        RfxRilAdapter::responseToRilj(id, NULL, 0, RIL_SOCKET_ID(slotId));
    #else
        RfxRilAdapter::responseToRilj(id, NULL, 0);
    #endif
    } else {
        RIL_CDMA_SignalInfoRecord entry;

        memset(&entry, 0, sizeof(entry));
        if (p->dataAvail() > 0) {
            readSignalInfoRecord(p, entry);
        }
        // RFX_LOG_V(LOG_TAG, "parcelToCdmaSignalInfoRecord request %d, type %d", id, type);
        if (isResponseType(type)) {
            // Response
            if (e == 0) {
                RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_CDMA_SignalInfoRecord));
            } else {
                RfxRilAdapter::responseToRilj(t, e, NULL, 0);
            }
        } else {
            // URC
            RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_CDMA_SignalInfoRecord), RIL_SOCKET_ID(slotId));
        }
    }
}

static void parcelToCdmaSignalInfoRecord(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CDMA_SignalInfoRecord entry;
    int type = getTypeAndJumpToData(p, id);

    memset(&entry, 0, sizeof(entry));
    if (p->dataAvail() > 0) {
        readSignalInfoRecord(p, entry);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCdmaSignalInfoRecord request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_CDMA_SignalInfoRecord));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_CDMA_SignalInfoRecord), RIL_SOCKET_ID(slotId));
    }
}


static void parcelToCdmaCallWaiting(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CDMA_CallWaiting_v6 entry;

    int type = getTypeAndJumpToData(p, id);

    memset(&entry, 0, sizeof(entry));
    if (p->dataAvail() > 0) {
        entry.number = strdupReadString(p);
        p->readInt32(&entry.numberPresentation);
        entry.name = strdupReadString(p);
        readSignalInfoRecord(p, entry.signalInfoRecord);
        p->readInt32(&entry.number_type);
        p->readInt32(&entry.number_plan);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCdmaCallWaiting request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_CDMA_CallWaiting_v6));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_CDMA_CallWaiting_v6), RIL_SOCKET_ID(slotId));
    }
    free(entry.number);
    free(entry.name);
}

static void parcelToSimRefresh(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t  v = 0;
    int type = getTypeAndJumpToData(p, id);
    // RFX_LOG_V(LOG_TAG, "RIL_VERSION %d, parcelToSimRefresh request %d, type %d",
    //        RIL_VERSION, id, type);

    if (RIL_VERSION >= 7) {
        RIL_SimRefreshResponse_v7 entry;

        memset(&entry, 0, sizeof(entry));
        if (p->dataAvail() > 0) {
            p->readInt32(&v);
            entry.result = RIL_SimRefreshResult(v);
            p->readInt32(&entry.ef_id);
            entry.aid = strdupReadString(p);
        }

        if (isResponseType(type)) {
            // Response
            if (e == 0) {
                RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_SimRefreshResponse_v7));
            } else {
                RfxRilAdapter::responseToRilj(t, e, NULL, 0);
            }
        } else {
            // URC
            RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_SimRefreshResponse_v7), RIL_SOCKET_ID(slotId));
        }
        if (entry.aid != NULL) {
            free(entry.aid);
        }
    } else {
        int *entry = NULL;
        if (p->dataAvail() > 0) {
            entry = (int *)calloc(1, 2 * sizeof(int));
            RFX_ASSERT(entry != NULL);
            p->readInt32(&entry[0]);
            p->readInt32(&entry[1]);
        }

        if (isResponseType(type)) {
            // Response
            if (e == 0 || entry != NULL) {
                RfxRilAdapter::responseToRilj(t, e, entry, 2*sizeof(int));
            } else {
                RfxRilAdapter::responseToRilj(t, e, NULL, 0);
            }
        } else {
            // URC
            RESPONSE_TO_RILJ(id, entry, 2*sizeof(int), RIL_SOCKET_ID(slotId));
        }
        if (entry != NULL) {
            free(entry);
        }
    }
}
static void parcelToCellInfoListV6(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CellInfo *entries = NULL;
    int32_t  v;
    int64_t  v6;
    int type = getTypeAndJumpToData(p, id);
    int32_t num = 0;
    status_t status;

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        entries = (RIL_CellInfo *) calloc(1, sizeof(RIL_CellInfo) * num);
        if (entries == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToCellInfoListV6 entries calloc fail");
            goto error;
        }
        for(int i = 0; i< num; i++) {
            p->readInt32(&v);
            entries[i].cellInfoType = RIL_CellInfoType(v);
            p->readInt32(&entries[i].registered);
            p->readInt32(&v);
            entries[i].timeStampType = RIL_TimeStampType(v);
            status = p->readInt64(&v6);
            if (status != NO_ERROR) goto error;
            entries[i].timeStamp = v6;
            switch(entries[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.mcc);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.mnc);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.lac);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.cid);
                    p->readInt32(&entries[i].CellInfo.gsm.signalStrengthGsm.signalStrength);
                    p->readInt32(&entries[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                    break;
                }
                case RIL_CELL_INFO_TYPE_WCDMA: {
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.mcc);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.mnc);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.lac);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.cid);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.psc);
                    p->readInt32(&entries[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                    p->readInt32(&entries[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                    break;
                }
                case RIL_CELL_INFO_TYPE_CDMA: {
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.networkId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.systemId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.basestationId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.longitude);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.latitude);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthCdma.dbm);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthCdma.ecio);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.dbm);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.ecio);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                    break;
                }
                case RIL_CELL_INFO_TYPE_LTE: {
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.mcc);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.mnc);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.ci);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.pci);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.tac);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.signalStrength);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rsrp);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rsrq);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rssnr);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.cqi);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.timingAdvance);
                    break;
                }
                case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.lac);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.cid);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                    p->readInt32(&entries[i].CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR:
                case RIL_CELL_INFO_TYPE_NONE:
                default:
                    break;
            }
        }
    }
error:
    // RFX_LOG_V(LOG_TAG, "parcelToCellInfoList request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || entries != NULL) {
            RfxRilAdapter::responseToRilj(t, e, entries, num * sizeof(RIL_CellInfo));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, entries, num*sizeof(RIL_CellInfo), RIL_SOCKET_ID(slotId));
    }
    if (entries != NULL) {
        free(entries);
    }
}

static void parcelToCellInfoListV12(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CellInfo_v12 *entries = NULL;
    int32_t  v;
    int64_t  v6;
    int type = getTypeAndJumpToData(p, id);
    int32_t num = 0;
    status_t status;

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        entries = (RIL_CellInfo_v12 *) calloc(1, sizeof(RIL_CellInfo_v12) * num);
        if (entries == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToCellInfoListV12 entries calloc fail");
            goto error;
        }
        for(int i = 0; i< num; i++) {
            p->readInt32(&v);
            entries[i].cellInfoType = RIL_CellInfoType(v);
            p->readInt32(&entries[i].registered);
            p->readInt32(&v);
            entries[i].timeStampType = RIL_TimeStampType(v);
            status = p->readInt64(&v6);
            if (status != NO_ERROR) goto error;
            entries[i].timeStamp = v6;
            switch(entries[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.mcc);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.mnc);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.mnc_len);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.lac);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.cid);
                    p->readInt32(&entries[i].CellInfo.gsm.cellIdentityGsm.arfcn);
                    p->readInt32(&v);
                    entries[i].CellInfo.gsm.cellIdentityGsm.bsic = (uint8_t)v;
                    p->readInt32(&entries[i].CellInfo.gsm.signalStrengthGsm.signalStrength);
                    p->readInt32(&entries[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                    p->readInt32(&entries[i].CellInfo.gsm.signalStrengthGsm.timingAdvance);
                    // duplicate operator name from the parcel
                    entries[i].CellInfo.gsm.cellIdentityGsm.operName.long_name
                            = strdupReadString(p);
                    entries[i].CellInfo.gsm.cellIdentityGsm.operName.short_name
                            = strdupReadString(p);
                    break;
                }
                case RIL_CELL_INFO_TYPE_WCDMA: {
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.mcc);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.mnc);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.mnc_len);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.lac);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.cid);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.psc);
                    p->readInt32(&entries[i].CellInfo.wcdma.cellIdentityWcdma.uarfcn);
                    p->readInt32(&entries[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                    p->readInt32(&entries[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                    // duplicate operator name from the parcel
                    entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name
                            = strdupReadString(p);
                    entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name
                            = strdupReadString(p);
                    break;
                }
                case RIL_CELL_INFO_TYPE_CDMA: {
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.networkId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.systemId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.basestationId);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.longitude);
                    p->readInt32(&entries[i].CellInfo.cdma.cellIdentityCdma.latitude);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthCdma.dbm);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthCdma.ecio);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.dbm);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.ecio);
                    p->readInt32(&entries[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                    // duplicate operator name from the parcel
                    entries[i].CellInfo.cdma.cellIdentityCdma.operName.long_name
                            = strdupReadString(p);
                    entries[i].CellInfo.cdma.cellIdentityCdma.operName.short_name
                            = strdupReadString(p);
                    break;
                }
                case RIL_CELL_INFO_TYPE_LTE: {
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.mcc);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.mnc);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.mnc_len);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.ci);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.pci);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.tac);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.earfcn);
                    p->readInt32(&entries[i].CellInfo.lte.cellIdentityLte.bandwidth);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.signalStrength);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rsrp);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rsrq);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.rssnr);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.cqi);
                    p->readInt32(&entries[i].CellInfo.lte.signalStrengthLte.timingAdvance);
                    // duplicate operator name from the parcel
                    entries[i].CellInfo.lte.cellIdentityLte.operName.long_name
                            = strdupReadString(p);
                    entries[i].CellInfo.lte.cellIdentityLte.operName.short_name
                            = strdupReadString(p);
                    break;
                }
                case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc_len);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.lac);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.cid);
                    p->readInt32(&entries[i].CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                    p->readInt32(&entries[i].CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                    // duplicate operator name from the parcel
                    entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name
                            = strdupReadString(p);
                    entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name
                            = strdupReadString(p);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR:
                case RIL_CELL_INFO_TYPE_NONE:
                default:
                    break;
            }
        }
    }
error:
    // RFX_LOG_V(LOG_TAG, "parcelToCellInfoList request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || entries != NULL) {
            RfxRilAdapter::responseToRilj(t, e, entries, num * sizeof(RIL_CellInfo_v12));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, entries, num*sizeof(RIL_CellInfo_v12), RIL_SOCKET_ID(slotId));
    }
    if (entries != NULL) {
        // free all operator name
        for(int i = 0; i< num; i++) {
            switch (entries[i].cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                if (entries[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                    free(entries[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                if (entries[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                    free(entries[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                if (entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                    free(entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                if (entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                    free(entries[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_LTE: {
                if (entries[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                    free(entries[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                if (entries[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                    free(entries[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                if (entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name)
                    free(entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                if (entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name)
                    free(entries[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_CDMA: {
                if (entries[i].CellInfo.cdma.cellIdentityCdma.operName.long_name)
                    free(entries[i].CellInfo.cdma.cellIdentityCdma.operName.long_name);
                if (entries[i].CellInfo.cdma.cellIdentityCdma.operName.short_name)
                    free(entries[i].CellInfo.cdma.cellIdentityCdma.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR:
            default:
                break;
            }
        }
        free(entries);
    }
}

static void parcelToCellInfoList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    parcelToCellInfoListV12(t,e,id,p,slotId);
}

static void parcelToIratStateChange(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RFX_UNUSED(t);
    RFX_UNUSED(e);
    RFX_UNUSED(id);
    RFX_UNUSED(p);
    RFX_UNUSED(slotId);
    // no use
}

static void parcelToHardwareConfig(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_HardwareConfig *pHardwareConfigs = NULL;
    int num = 0, temp = 0, maxLen = 0;
    char *uuid = NULL;
    int type = getTypeAndJumpToData(p , id);

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        pHardwareConfigs = (RIL_HardwareConfig *) calloc(num, sizeof(RIL_HardwareConfig));
        if (pHardwareConfigs == NULL) {
            RFX_LOG_E(LOG_TAG, "[RFX] OOM");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        for (int i = 0; i < num; i++) {
            p->readInt32(&temp);
            pHardwareConfigs[i].type = (RIL_HardwareConfig_Type) temp;
            switch(pHardwareConfigs[i].type) {
                case RIL_HARDWARE_CONFIG_MODEM:
                    uuid = strdupReadString(p);
                    maxLen = strlen(uuid) > (MAX_UUID_LENGTH - 1)?
                            (MAX_UUID_LENGTH - 1): strlen(uuid);
                    strncpy(pHardwareConfigs[i].uuid, uuid, maxLen);
                    p->readInt32(&temp);
                    pHardwareConfigs[i].state = (RIL_HardwareConfig_State) temp;
                    p->readInt32(&temp);
                    pHardwareConfigs[i].cfg.modem.rilModel = (uint32_t) temp;
                    p->readInt32(&temp);
                    pHardwareConfigs[i].cfg.modem.rat = (uint32_t) temp;
                    p->readInt32(&temp);
                    pHardwareConfigs[i].cfg.modem.maxVoice = (uint32_t) temp;
                    p->readInt32(&temp);
                    pHardwareConfigs[i].cfg.modem.maxData = (uint32_t) temp;
                    p->readInt32(&temp);
                    pHardwareConfigs[i].cfg.modem.maxStandby = (uint32_t) temp;
                    free(uuid);
                    break;
                case RIL_HARDWARE_CONFIG_SIM:
                    uuid = strdupReadString(p);
                    maxLen = strlen(uuid) > (MAX_UUID_LENGTH - 1)?
                            (MAX_UUID_LENGTH - 1): strlen(uuid);
                    strncpy(pHardwareConfigs[i].uuid, uuid, maxLen);
                    free(uuid);
                    p->readInt32(&temp);
                    pHardwareConfigs[i].state = (RIL_HardwareConfig_State) temp;
                    uuid = strdupReadString(p);
                    maxLen = strlen(uuid) > (MAX_UUID_LENGTH - 1)?
                            (MAX_UUID_LENGTH - 1): strlen(uuid);
                    strncpy(pHardwareConfigs[i].cfg.sim.modemUuid, uuid, maxLen);
                    free(uuid);
                    break;
            }
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToHardwareConfig request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, pHardwareConfigs, sizeof(RIL_HardwareConfig) * num);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, pHardwareConfigs, sizeof(RIL_HardwareConfig) * num,
                RIL_SOCKET_ID(slotId));
    }

    if (pHardwareConfigs != NULL) {
        free(pHardwareConfigs);
        pHardwareConfigs = NULL;
    }
}

static void parcelToDcRtInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RFX_UNUSED(t);
    RFX_UNUSED(e);
    RFX_UNUSED(id);
    RFX_UNUSED(p);
    RFX_UNUSED(slotId);
    // have implementaton, but no one use it
}

static void parcelToRadioCapability(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_RadioCapability rc;
    int32_t v = 0;
    int type = getTypeAndJumpToData(p, id);

    memset (&rc, 0, sizeof(RIL_RadioCapability));

    if (p->dataAvail() > 0) {
        p->readInt32(&v);
        rc.version = (int)v;

        p->readInt32(&v);
        rc.session= (int)v;

        p->readInt32(&v);
        rc.phase= (int)v;

        p->readInt32(&v);
        rc.rat = (int)v;

        readStringFromParcelInplace(p, rc.logicalModemUuid, sizeof(rc.logicalModemUuid));

        p->readInt32(&v);
        rc.status = (int)v;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToRadioCapability request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &rc, sizeof(RIL_RadioCapability));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &rc, sizeof(RIL_RadioCapability), RIL_SOCKET_ID(slotId));
    }

    return;
}

static void parcelToSSData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    // have implementaton, but no one use it
    RFX_UNUSED(t);
    RFX_UNUSED(e);
    RFX_UNUSED(id);
    RFX_UNUSED(p);
    RFX_UNUSED(slotId);
}

static void parcelToLceStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_LceStatusInfo lsi;
    int32_t v = 0;
    int type = getTypeAndJumpToData(p, id);

    RFX_UNUSED(t);
    RFX_UNUSED(e);
    RFX_UNUSED(id);
    RFX_UNUSED(p);
    RFX_UNUSED(slotId);
    memset (&lsi, 0, sizeof(RIL_LceStatusInfo));

    if (p->dataAvail() > 0) {
        p->read(&v, 1);
        lsi.lce_status = (int8_t)v;

        p->readInt32(&v);
        lsi.actual_interval_ms = (uint32_t)v;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToLceStatus request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &lsi, sizeof(RIL_LceStatusInfo));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &lsi, sizeof(RIL_LceStatusInfo), RIL_SOCKET_ID(slotId));
    }

    return;
invalid:
    invalidCommandBlock(id);
    return;
}

static void parcelToLceData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_LceDataInfo ldi;
    int32_t v = 0;
    int type = getTypeAndJumpToData(p, id);

    memset (&ldi, 0, sizeof(RIL_LceDataInfo));

    if (p->dataAvail() > 0) {
        p->readInt32(&v);
        ldi.last_hop_capacity_kbps = (uint32_t)v;

        p->read(&v, 1);
        ldi.confidence_level = (uint8_t)v;

        p->read(&v, 1);
        ldi.lce_suspended = (uint8_t)v;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToLceData request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &ldi, sizeof(RIL_LceDataInfo));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &ldi, sizeof(RIL_LceDataInfo), RIL_SOCKET_ID(slotId));
    }

    return;
}

static void parcelToActivityData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_ActivityStatsInfo asi;
    int32_t v = 0;
    int type = getTypeAndJumpToData(p, id);

    memset (&asi, 0, sizeof(RIL_ActivityStatsInfo));

    if (p->dataAvail() > 0) {
        p->readInt32(&v);
        asi.sleep_mode_time_ms = (uint32_t)v;

        p->readInt32(&v);
        asi.idle_mode_time_ms = (uint32_t)v;

        for (int index = 0; index < RIL_NUM_TX_POWER_LEVELS; index++) {
            p->readInt32(&v);
            asi.tx_mode_time_ms[index] = (uint32_t)v;
        }
        p->readInt32(&v);
        asi.rx_mode_time_ms = (uint32_t)v;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToActivityData request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &asi, sizeof(RIL_ActivityStatsInfo));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &asi, sizeof(RIL_ActivityStatsInfo), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToNetworkScanResult(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int type = getTypeAndJumpToData(p, id);

    // Only used in URC
    if (p->dataSize() == 2) {
        // No response data
#if defined(ANDROID_MULTI_SIM)
        RfxRilAdapter::responseToRilj(id, NULL, 0, RIL_SOCKET_ID(slotId));
#else
        RfxRilAdapter::responseToRilj(id, NULL, 0);
#endif
    } else {
        RIL_NetworkScanResult entry;
        memset(&entry, 0, sizeof(entry));

        if (p->dataAvail() > 0) {
            int32_t v;
            int64_t  v6;
            int32_t num = 0;
            status_t status;

            p->readInt32(&v);
            entry.status = RIL_ScanStatus(v);

            p->readInt32(&v);
            entry.network_infos_length = v;

            num = entry.network_infos_length/sizeof(RIL_CellInfo_v12);
            entry.network_infos = (RIL_CellInfo_v12 *) calloc(1, sizeof(RIL_CellInfo_v12));
            if (entry.network_infos == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToNetworkScanResult CellInfoListV12 calloc fail");
                goto error;
            }

            for (int i = 0; i< num; i++) {
                p->readInt32(&v);
                entry.network_infos[i].cellInfoType = RIL_CellInfoType(v);
                p->readInt32(&entry.network_infos[i].registered);
                p->readInt32(&v);
                entry.network_infos[i].timeStampType = RIL_TimeStampType(v);
                status = p->readInt64(&v6);
                if (status != NO_ERROR) goto error;
                entry.network_infos[i].timeStamp = (uint64_t)v6;
                switch (entry.network_infos[i].cellInfoType) {
                    case RIL_CELL_INFO_TYPE_GSM: {
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.mcc);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.mnc);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.lac);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.cid);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.arfcn);
                        p->readInt32(&v);
                        entry.network_infos[i].CellInfo.gsm.cellIdentityGsm.bsic = (uint8_t)v;
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.signalStrengthGsm.signalStrength);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                        p->readInt32(&entry.network_infos[i].CellInfo.gsm.signalStrengthGsm.timingAdvance);
                        break;
                    }
                    case RIL_CELL_INFO_TYPE_WCDMA: {
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.mcc);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.mnc);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.lac);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.cid);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.psc);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.cellIdentityWcdma.uarfcn);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                        p->readInt32(&entry.network_infos[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                        break;
                    }
                    case RIL_CELL_INFO_TYPE_CDMA: {
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.cellIdentityCdma.networkId);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.cellIdentityCdma.systemId);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.cellIdentityCdma.basestationId);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.cellIdentityCdma.longitude);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.cellIdentityCdma.latitude);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.signalStrengthCdma.dbm);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.signalStrengthCdma.ecio);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.signalStrengthEvdo.dbm);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.signalStrengthEvdo.ecio);
                        p->readInt32(&entry.network_infos[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                        break;
                    }
                    case RIL_CELL_INFO_TYPE_LTE: {
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.mcc);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.mnc);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.ci);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.pci);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.tac);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.cellIdentityLte.earfcn);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.signalStrength);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.rsrp);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.rsrq);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.rssnr);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.cqi);
                        p->readInt32(&entry.network_infos[i].CellInfo.lte.signalStrengthLte.timingAdvance);
                        break;
                    }
                    case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.cellIdentityTdscdma.lac);
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.cellIdentityTdscdma.cid);
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                        p->readInt32(&entry.network_infos[i].CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                        break;
                    }
                    case RIL_CELL_INFO_TYPE_NR:
                    case RIL_CELL_INFO_TYPE_NONE:
                    default:
                        break;
                }
            }
        }
        // URC
error:
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_NetworkScanResult), RIL_SOCKET_ID(slotId));
        if (entry.network_infos != NULL) {
            free(entry.network_infos);
        }
    }
}

long convertResponseHexStringEntryToInt(char **response, int index, int numStrings) {
    if ((response != NULL) &&  (numStrings > index) && (response[index] != NULL)) {
        return strtol(response[index], NULL, 16);
    }

    return -1;
}

void fillCellIdentityFromVoiceRegStateResponseString(RIL_CellIdentity_v16 &rilCellIdentity,
        int numStrings, char** response) {

    int32_t *tmp = (int32_t*)&rilCellIdentity;

    for (size_t i = 0; i < sizeof(RIL_CellIdentity_v16)/sizeof(int32_t); i++) {
        tmp[i] = -1;
    }

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {

        case RIL_CELL_INFO_TYPE_GSM: {
            rilCellIdentity.cellIdentityGsm.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityGsm.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[16]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[16], 3);
                strncpy(mnc, response[16]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityGsm.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityGsm.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityGsm.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityGsm.operName.long_name = strdup(response[17]);
                rilCellIdentity.cellIdentityGsm.operName.short_name = strdup(response[18]);
            } else {
                rilCellIdentity.cellIdentityGsm.mnc_len = 2;
                rilCellIdentity.cellIdentityGsm.mcc = 0;
                rilCellIdentity.cellIdentityGsm.mnc = 0;
                rilCellIdentity.cellIdentityGsm.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityGsm.operName.short_name = strdup("");
            }
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            rilCellIdentity.cellIdentityWcdma.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityWcdma.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            rilCellIdentity.cellIdentityWcdma.psc =
                    (int) convertResponseHexStringEntryToInt(response, 14, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[16]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[16], 3);
                strncpy(mnc, response[16]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityWcdma.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityWcdma.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityWcdma.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityWcdma.operName.long_name = strdup(response[17]);
                rilCellIdentity.cellIdentityWcdma.operName.short_name = strdup(response[18]);
            } else {
                rilCellIdentity.cellIdentityWcdma.mnc_len = 2;
                rilCellIdentity.cellIdentityWcdma.mcc = 0;
                rilCellIdentity.cellIdentityWcdma.mnc = 0;
                rilCellIdentity.cellIdentityWcdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityWcdma.operName.short_name = strdup("");
            }
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            rilCellIdentity.cellIdentityTdscdma.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityTdscdma.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[16]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[16], 3);
                strncpy(mnc, response[16]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityTdscdma.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityTdscdma.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityTdscdma.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityTdscdma.operName.long_name = strdup(response[17]);
                rilCellIdentity.cellIdentityTdscdma.operName.short_name = strdup(response[18]);
            } else {
                rilCellIdentity.cellIdentityTdscdma.mnc_len = 2;
                rilCellIdentity.cellIdentityTdscdma.mcc = 0;
                rilCellIdentity.cellIdentityTdscdma.mnc = 0;
                rilCellIdentity.cellIdentityTdscdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityTdscdma.operName.short_name = strdup("");
            }
            break;
        }
        //TODO: CDMA may need use HexStringToInt API for some value???
        case RIL_CELL_INFO_TYPE_CDMA:{
            rilCellIdentity.cellIdentityCdma.basestationId =
                    convertResponseStringEntryToInt(response, 4, numStrings);
            rilCellIdentity.cellIdentityCdma.longitude =
                    convertResponseStringEntryToInt(response, 5, numStrings);
            rilCellIdentity.cellIdentityCdma.latitude =
                    convertResponseStringEntryToInt(response, 6, numStrings);
            rilCellIdentity.cellIdentityCdma.systemId =
                    convertResponseStringEntryToInt(response, 8, numStrings);
            rilCellIdentity.cellIdentityCdma.networkId =
                    convertResponseStringEntryToInt(response, 9, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[16]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                rilCellIdentity.cellIdentityCdma.operName.long_name = strdup(response[17]);
                rilCellIdentity.cellIdentityCdma.operName.short_name = strdup(response[18]);
            } else {
                rilCellIdentity.cellIdentityCdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityCdma.operName.short_name = strdup("");
            }
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE:{
            rilCellIdentity.cellIdentityLte.tac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityLte.ci =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[16]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[16], 3);
                strncpy(mnc, response[16]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityLte.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityLte.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityLte.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityLte.operName.long_name = strdup(response[17]);
                rilCellIdentity.cellIdentityLte.operName.short_name = strdup(response[18]);
            } else {
                rilCellIdentity.cellIdentityLte.mnc_len = 2;
                rilCellIdentity.cellIdentityLte.mcc = 0;
                rilCellIdentity.cellIdentityLte.mnc = 0;
                rilCellIdentity.cellIdentityLte.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityLte.operName.short_name = strdup("");
            }
            rilCellIdentity.cellIdentityLte.bandwidth = 0x7FFFFFFF; // unknown for legacy platform.
            break;
        }
        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }
}

static void parcelToVoiceState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t countStrings = 0;
    size_t datalen = 0;
    char **pStrings = NULL;

    RIL_VoiceRegistrationStateResponse *p_cur = NULL;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32 (&countStrings);
        // LOGE("countStrings = %d", countStrings);
        if (countStrings == 0) {
            // just some non-null pointer
            pStrings = (char **)alloca(sizeof(char *));
            datalen = 0;
            if (pStrings == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToVoiceState caloc fail");
                goto error;
            }
        } else if (((int)countStrings) == -1) {
            pStrings = NULL;
            datalen = 0;
        } else {
            datalen = sizeof(char *) * countStrings;
            pStrings = (char **)alloca(datalen);
            if (pStrings == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToVoiceState caloc fail");
                goto error;
            }
            for (int i = 0 ; i < countStrings ; i++) {
                pStrings[i] = strdupReadString(p);
                // LOGE("parcelToVoiceState pStrings[%d] = %s", i, (pStrings[i]==NULL?"N/A":pStrings[i]));
            }
        }

        p_cur = (RIL_VoiceRegistrationStateResponse*)calloc(1, sizeof(RIL_VoiceRegistrationStateResponse));
        if (p_cur == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToVoiceState caloc fail");
            goto error;
        }
        if (countStrings >= 15) {
            p_cur->regState = (RIL_RegState) (pStrings[0] ? atoi(pStrings[0]) : 0);
            p_cur->rat = (RIL_RadioTechnology) (pStrings[3] ? atoi(pStrings[3]) : 0);
            p_cur->cssSupported = pStrings[7] ? atoi(pStrings[7]) : 0;
            p_cur->roamingIndicator = pStrings[10] ? atoi(pStrings[10]) : 0;
            p_cur->systemIsInPrl = pStrings[11] ? atoi(pStrings[11]) : 0;
            p_cur->defaultRoamingIndicator = pStrings[12] ? atoi(pStrings[12]) : 0;
            p_cur->reasonForDenial = pStrings[13] ? atoi(pStrings[13]) : 0;
            fillCellIdentityFromVoiceRegStateResponseString(p_cur->cellIdentity,
                    countStrings, pStrings);
        }
    } else {
        // LOGE("parcelToVoiceState dataAvail X");
    }
error:
    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_VoiceRegistrationStateResponse));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_VoiceRegistrationStateResponse), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        // release the memory of operator info
        switch (p_cur->cellIdentity.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                if (p_cur->cellIdentity.cellIdentityGsm.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityGsm.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityGsm.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityGsm.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                if (p_cur->cellIdentity.cellIdentityWcdma.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityWcdma.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityWcdma.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityWcdma.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_CDMA: {
                if (p_cur->cellIdentity.cellIdentityCdma.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityCdma.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityCdma.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityCdma.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_LTE: {
                if (p_cur->cellIdentity.cellIdentityLte.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityLte.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityLte.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityLte.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_NR:
            default: {
                break;
                }
        }
        free(p_cur);
    }
}

static void parcelToCarrierRestrictions(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CarrierRestrictionsWithPriority entry;
    RIL_Carrier* allowed_tmp;
    RIL_Carrier* exclude_tmp;
    int32_t tmp;
    int type = getTypeAndJumpToData(p, id);
    status_t status = NO_ERROR;
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        // len_allowed_carriers
        p->readInt32(&entry.len_allowed_carriers);

        // len_excluded_carriers
        p->readInt32(&entry.len_excluded_carriers);

        // allowed_carriers
        entry.allowed_carriers = (RIL_Carrier *) calloc(entry.len_allowed_carriers, sizeof(RIL_Carrier));
        for (int32_t i = 0; i < entry.len_allowed_carriers; i++) {
            allowed_tmp = entry.allowed_carriers + i;
            allowed_tmp->mcc = strdupReadString(p);
            allowed_tmp->mnc = strdupReadString(p);
            p->readInt32(&tmp);
            allowed_tmp->match_type = RIL_CarrierMatchType(tmp);
            allowed_tmp->match_data = strdupReadString(p);
            // entry.allowed_carriers = allowed_tmp;
        }

        // excluded_carriers
        entry.excluded_carriers = (RIL_Carrier *) calloc(entry.len_excluded_carriers, sizeof(RIL_Carrier));
        for (int32_t i = 0; i < entry.len_excluded_carriers; i++) {
            exclude_tmp = entry.excluded_carriers + i;
            exclude_tmp->mcc = strdupReadString(p);
            exclude_tmp->mnc = strdupReadString(p);
            p->readInt32(&tmp);
            exclude_tmp->match_type = RIL_CarrierMatchType(tmp);
            exclude_tmp->match_data = strdupReadString(p);
            // entry.excluded_carriers = exclude_tmp;
        }

        // allowedCarriersPrioritized
        status = p->readBool(&entry.allowedCarriersPrioritized);
        if (status != NO_ERROR) {
            e = RIL_E_NO_MEMORY;
        }

        // simLockMultiSimPolicy
        p->readInt32(&entry.simLockMultiSimPolicy);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCarrierRestrictions request %d,
    // len_allowed_carriers %d, len_excluded_carriers %d
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_CarrierRestrictionsWithPriority));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_CarrierRestrictionsWithPriority), RIL_SOCKET_ID(slotId));
    }

    // free memory
    if (entry.allowed_carriers != NULL) {
        for (int32_t i = 0; i < entry.len_allowed_carriers; i++) {
            allowed_tmp = entry.allowed_carriers + i;
            if (allowed_tmp->mcc != NULL) {
                free(allowed_tmp->mcc);
                allowed_tmp->mcc = NULL;
            }
            if (allowed_tmp->mnc != NULL) {
                free(allowed_tmp->mnc);
                allowed_tmp->mnc = NULL;
            }
            if (allowed_tmp->match_data != NULL) {
                free(allowed_tmp->match_data);
                allowed_tmp->match_data = NULL;
            }
        }
        free(entry.allowed_carriers);
        entry.allowed_carriers = NULL;
    }

    if (entry.excluded_carriers != NULL) {
        for (int32_t i = 0; i < entry.len_excluded_carriers; i++) {
            allowed_tmp = entry.excluded_carriers + i;
            if (allowed_tmp->mcc != NULL) {
                free(allowed_tmp->mcc);
                allowed_tmp->mcc = NULL;
            }
            if (allowed_tmp->mnc != NULL) {
                free(allowed_tmp->mnc);
                allowed_tmp->mnc = NULL;
            }
            if (allowed_tmp->match_data != NULL) {
                free(allowed_tmp->match_data);
                allowed_tmp->match_data = NULL;
            }
        }
        free(entry.excluded_carriers);
        entry.excluded_carriers = NULL;
    }
}

void fillCellIdentityFromDataRegStateResponseString(RIL_CellIdentity_v16 &rilCellIdentity,
        int numStrings, char** response) {

    int32_t *tmp = (int32_t*)&rilCellIdentity;

    for (size_t i = 0; i < sizeof(RIL_CellIdentity_v16)/sizeof(int32_t); i++) {
        tmp[i] = -1;
    }

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            rilCellIdentity.cellIdentityGsm.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityGsm.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[6]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[6], 3);
                strncpy(mnc, response[6]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityGsm.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityGsm.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityGsm.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityGsm.operName.long_name = strdup(response[7]);
                rilCellIdentity.cellIdentityGsm.operName.short_name = strdup(response[8]);
            } else {
                rilCellIdentity.cellIdentityGsm.mnc_len = 2;
                rilCellIdentity.cellIdentityGsm.mcc = 0;
                rilCellIdentity.cellIdentityGsm.mnc = 0;
                rilCellIdentity.cellIdentityGsm.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityGsm.operName.short_name = strdup("");
            }
            break;
        }
        case RIL_CELL_INFO_TYPE_WCDMA: {
            rilCellIdentity.cellIdentityWcdma.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityWcdma.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[6]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[6], 3);
                strncpy(mnc, response[6]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityWcdma.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityWcdma.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityWcdma.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityWcdma.operName.long_name = strdup(response[7]);
                rilCellIdentity.cellIdentityWcdma.operName.short_name = strdup(response[8]);
            } else {
                rilCellIdentity.cellIdentityWcdma.mnc_len = 2;
                rilCellIdentity.cellIdentityWcdma.mcc = 0;
                rilCellIdentity.cellIdentityWcdma.mnc = 0;
                rilCellIdentity.cellIdentityWcdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityWcdma.operName.short_name = strdup("");
            }
            break;
        }
        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            rilCellIdentity.cellIdentityTdscdma.lac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityTdscdma.cid =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[6]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[6], 3);
                strncpy(mnc, response[6]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityTdscdma.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityTdscdma.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityTdscdma.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityTdscdma.operName.long_name = strdup(response[7]);
                rilCellIdentity.cellIdentityTdscdma.operName.short_name = strdup(response[8]);
            } else {
                rilCellIdentity.cellIdentityTdscdma.mnc_len = 2;
                rilCellIdentity.cellIdentityTdscdma.mcc = 0;
                rilCellIdentity.cellIdentityTdscdma.mnc = 0;
                rilCellIdentity.cellIdentityTdscdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityTdscdma.operName.short_name = strdup("");
            }
            break;
        }
        case RIL_CELL_INFO_TYPE_CDMA:{
            rilCellIdentity.cellIdentityCdma.basestationId = 0x7FFFFFFF;
            rilCellIdentity.cellIdentityCdma.longitude = 0x7FFFFFFF;
            rilCellIdentity.cellIdentityCdma.latitude = 0x7FFFFFFF;
            rilCellIdentity.cellIdentityCdma.systemId = 0x7FFFFFFF;
            rilCellIdentity.cellIdentityCdma.networkId = 0x7FFFFFFF;
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[6]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                rilCellIdentity.cellIdentityCdma.operName.long_name = strdup(response[7]);
                rilCellIdentity.cellIdentityCdma.operName.short_name = strdup(response[8]);
            } else {
                rilCellIdentity.cellIdentityCdma.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityCdma.operName.short_name = strdup("");
            }
            break;
        }
        case RIL_CELL_INFO_TYPE_LTE: {
            //AOPS uses response[6].
            rilCellIdentity.cellIdentityLte.tac =
                    (int) convertResponseHexStringEntryToInt(response, 1, numStrings);
            //RILD never provide pic
            rilCellIdentity.cellIdentityLte.pci = 0;
            //AOPS uses response[8].
            rilCellIdentity.cellIdentityLte.ci =
                    (int) convertResponseHexStringEntryToInt(response, 2, numStrings);
            // fullfill operator info for HIDL 1.2
            char mcc[4], mnc[4];
            int plmn_len = strlen(response[6]);
            if (plmn_len >= 5 && plmn_len <= 6) {
                strncpy(mcc, response[6], 3);
                strncpy(mnc, response[6]+3, 3);
                mcc[3] = '\0';
                mnc[3] = '\0';
                rilCellIdentity.cellIdentityLte.mnc_len = (plmn_len == 6 ? 3 : 2);
                rilCellIdentity.cellIdentityLte.mcc = atoi(mcc);
                rilCellIdentity.cellIdentityLte.mnc = atoi(mnc);
                rilCellIdentity.cellIdentityLte.operName.long_name = strdup(response[7]);
                rilCellIdentity.cellIdentityLte.operName.short_name = strdup(response[8]);
            } else {
                rilCellIdentity.cellIdentityLte.mnc_len = 2;
                rilCellIdentity.cellIdentityLte.mcc = 0;
                rilCellIdentity.cellIdentityLte.mnc = 0;
                rilCellIdentity.cellIdentityLte.operName.long_name = strdup("");
                rilCellIdentity.cellIdentityLte.operName.short_name = strdup("");
            }
            rilCellIdentity.cellIdentityLte.bandwidth = 0x7FFFFFFF; // unknown for legacy platform.
            break;
        }
        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }
}

static void parcelToDataState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t countStrings = 0;
    size_t datalen = 0;
    char **pStrings = NULL;

    RIL_DataRegistrationStateResponse *p_cur = NULL;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32 (&countStrings);
        // LOGE("countStrings = %d", countStrings);
        if (countStrings == 0) {
            // just some non-null pointer
            pStrings = (char **)alloca(sizeof(char *));
            datalen = 0;
            if (pStrings == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToDataState caloc fail");
                goto error;
            }
        } else if (((int)countStrings) == -1) {
            pStrings = NULL;
            datalen = 0;
        } else {
            datalen = sizeof(char *) * countStrings;
            pStrings = (char **)alloca(datalen);
            for (int i = 0 ; i < countStrings ; i++) {
                pStrings[i] = strdupReadString(p);
                // LOGE("parcelToDataState pStrings[%d] = %s", i, (pStrings[i]==NULL?"N/A":pStrings[i]));
            }
        }

        p_cur = (RIL_DataRegistrationStateResponse*)calloc(1, sizeof(RIL_DataRegistrationStateResponse));
        if (p_cur == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToDataState caloc p_cur fail");
            goto error;
        }
        if (countStrings >= 6) {
            p_cur->regState = (RIL_RegState) (pStrings[0] ? atoi(pStrings[0]) : 4);
            p_cur->rat = (RIL_RadioTechnology) (pStrings[3] ? atoi(pStrings[3]) : 0);
            p_cur->reasonDataDenied = pStrings[4] ? atoi(pStrings[4]) : 0;
            p_cur->maxDataCalls = pStrings[5] ? atoi(pStrings[5]) : 1;
            fillCellIdentityFromDataRegStateResponseString(p_cur->cellIdentity,
                    countStrings, pStrings);
        }
        if (countStrings >= 10) {
            p_cur->lteVopsInfo.isEmcBearerSupported = atoi(pStrings[9]);
        }
    } else {
        // LOGE("parcelToDataState dataAvail X");
    }
error:
    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_DataRegistrationStateResponse));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_DataRegistrationStateResponse), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        // release the memory of operator info
        switch (p_cur->cellIdentity.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                if (p_cur->cellIdentity.cellIdentityGsm.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityGsm.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityGsm.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityGsm.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                if (p_cur->cellIdentity.cellIdentityWcdma.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityWcdma.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityWcdma.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityWcdma.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_CDMA: {
                if (p_cur->cellIdentity.cellIdentityCdma.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityCdma.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityCdma.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityCdma.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_LTE: {
                if (p_cur->cellIdentity.cellIdentityLte.operName.long_name)
                    free(p_cur->cellIdentity.cellIdentityLte.operName.long_name);
                if (p_cur->cellIdentity.cellIdentityLte.operName.short_name)
                    free(p_cur->cellIdentity.cellIdentityLte.operName.short_name);
                break;
                }
            case RIL_CELL_INFO_TYPE_NR:
            default: {
                break;
                }
        }
        free(p_cur);
    }
}

// M: [VzW] Data Framework @{
static void parcelToPcoDataAfterAttached(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_PCO_Data_attached *p_cur = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p_cur = (RIL_PCO_Data_attached*)calloc(1, sizeof(RIL_PCO_Data_attached));
        RFX_ASSERT(p_cur != NULL);

        p->readInt32(&p_cur->cid);
        p_cur->apn_name = strdupReadString(p);
        p_cur->bearer_proto = strdupReadString(p);
        p->readInt32(&p_cur->pco_id);
        p->readInt32(&p_cur->contents_length);
        p_cur->contents = (char *) calloc(p_cur->contents_length, sizeof(char));
        RFX_ASSERT(p_cur->contents != NULL);
        p->read(p_cur->contents, p_cur->contents_length);
    }

    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_PCO_Data_attached));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_PCO_Data_attached), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        free(p_cur->apn_name);
        free(p_cur->bearer_proto);
        free(p_cur->contents);
        free(p_cur);
    }
}
// M: [VzW] Data Framework @}

/// M: eMBMS feature
Parcel* embmsStartSessionReqToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16;

    fillHeader(parcel, request, token);
    RIL_EMBMS_StartSessionReq *rc = (RIL_EMBMS_StartSessionReq *) data;
    // RFX_LOG_V(LOG_TAG, "embmsStartSessionReqToParcel id:%d,earfcnlist_count:%d,saiList_valid %d, saiList_count %d",
    //    rc->trans_id, rc->earfcnlist_count, rc->saiList_valid, rc->saiList_count);
    parcel->writeInt32(rc->trans_id);

    // writeByteArray will introduce endian issue
    parcel->writeInt32(rc->tmgi_info.tmgi_len);
    uint8_t uct;
    for (uint32_t i = 0; i < rc->tmgi_info.tmgi_len; i++) {
        parcel->write(&(rc->tmgi_info.tmgi[i]), sizeof(uct));
    }

    parcel->writeInt32(rc->earfcnlist_count);

    for (uint32_t i = 0; i < rc->earfcnlist_count; i++) {
        parcel->writeInt32(rc->earfcnlist[i]);
    }

    parcel->writeInt32(rc->saiList_valid);

    parcel->writeInt32(rc->saiList_count);

    for (uint32_t i = 0; i < rc->saiList_count; i++) {
        parcel->writeInt32(rc->saiList[i]);
    }
    return parcel;
}

Parcel* embmsStopSessionReqToParcel(int request, int token, void* data, int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16;

    fillHeader(parcel, request, token);
    RIL_EMBMS_StopSessionReq *rc = (RIL_EMBMS_StopSessionReq *) data;

    parcel->writeInt32(rc->trans_id);

    // writeByteArray will introduce endian issue
    parcel->writeInt32(rc->tmgi_info.tmgi_len);
    uint8_t uct;
    for (uint32_t i = 0; i < rc->tmgi_info.tmgi_len; i++) {
        parcel->write(&(rc->tmgi_info.tmgi[i]), sizeof(uct));
    }

    return parcel;
}

static void parcelToEmbmsEnable(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_EnableResp entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.interface_index_valid = tmp;
        p->readInt32(&tmp);
        entry.interface_index = tmp;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsEnable request %d, trans_id %d, response %d, index_valid %d, index %d",
    //    id, entry.trans_id, entry.response, entry.interface_index_valid, entry.interface_index);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_EnableResp));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_EnableResp), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsDisable(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_DisableResp entry;

    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsDisable request %d, trans_id %d, response %d",
    //    id, entry.trans_id, entry.response);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_DisableResp));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_DisableResp), RIL_SOCKET_ID(slotId));
    }
}


static void parcelToEmbmsSessionInfo(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_StartSessionResp entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.tmgi_info_valid = tmp;
        p->readInt32(&tmp);
        entry.tmgi_info.tmgi_len = tmp;
        uint8_t uct = 0;
        for (uint32_t i = 0; i < entry.tmgi_info.tmgi_len; i++) {
            p->read(&uct, sizeof(uct));
            entry.tmgi_info.tmgi[i] = (uint8_t)uct;
        }
        RFX_LOG_D(LOG_TAG, "parcelToEmbmsSessionInfo, tmgi[%02X,%02X,%02X,%02X,%02X,%02X]",
                                                    entry.tmgi_info.tmgi[0],
                                                    entry.tmgi_info.tmgi[1],
                                                    entry.tmgi_info.tmgi[2],
                                                    entry.tmgi_info.tmgi[3],
                                                    entry.tmgi_info.tmgi[4],
                                                    entry.tmgi_info.tmgi[5]);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsSessionInfo request %d, trans_id %d, response %d, tmgi_valid %d, tmgi_len %d",
    //    id, entry.trans_id, entry.response, entry.tmgi_info_valid, entry.tmgi_info.tmgi_len);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_StartSessionResp));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_StartSessionResp), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsNetworkTime(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_GetTimeResp entry;
    int32_t tmp = 0;
    int64_t tmp6 = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt64(&tmp6);
        entry.milli_sec = tmp6;
        p->readInt32(&tmp);
        entry.day_light_saving_valid = tmp;
        p->readInt32(&tmp);
        entry.day_light_saving = tmp;
        p->readInt32(&tmp);
        entry.leap_seconds_valid = tmp;
        p->readInt32(&tmp);
        entry.leap_seconds = tmp;
        p->readInt32(&tmp);
        entry.local_time_offset_valid = tmp;
        p->readInt32(&tmp);
        entry.local_time_offset = tmp;
    }
    /*
    RFX_LOG_V(LOG_TAG, "parcelToEmbmsNetworkTime request %d, trans_id %d, response %d, milli_sec %llu",
        id, entry.trans_id, entry.response, entry.milli_sec);
    RFX_LOG_V(LOG_TAG, "dlsaving_valid %d, dlsaving %d, ls_valid %d, ls %d, lto_valid %d, lto %d",
        entry.day_light_saving_valid, entry.day_light_saving,
        entry.leap_seconds_valid, entry.leap_seconds,
        entry.local_time_offset_valid, entry.local_time_offset);
    */
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_GetTimeResp));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_GetTimeResp), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsGetCoverageState(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_GetCoverageResp entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.coverage_state_valid = tmp;
        p->readInt32(&tmp);
        entry.coverage_state = tmp;
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsGetCoverageState request %d, trans_id %d, response %d, valid %d, state %d",
    //    id, entry.trans_id, entry.response, entry.coverage_state_valid, entry.coverage_state);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_GetCoverageResp));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_GetCoverageResp), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsCellInfoNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_CellInfoNotify entry;

    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.cell_id);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsCellInfoNotify request %d, trans_id %d, cell_id %d",
    //    id, entry.trans_id, entry.cell_id);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_CellInfoNotify));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_CellInfoNotify), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsModemEeNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_ModemEeNotify entry;

    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.state);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsModemEeNotify request %d, trans_id %d, state %d",
    //    id, entry.trans_id, entry.state);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_ModemEeNotify));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_ModemEeNotify), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsActiveSessionNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_ActiveSessionNotify entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&tmp);
        entry.tmgi_info_count = tmp;

        for (uint32_t i = 0; i < entry.tmgi_info_count; i++) {
            p->readInt32(&tmp);
            entry.tmgi_info[i].tmgi_len = tmp;
            uint8_t uct = 0;
            for (uint32_t j = 0; j < entry.tmgi_info[i].tmgi_len; j++) {
                p->read(&uct, sizeof(uct));
                entry.tmgi_info[i].tmgi[j] = (uint8_t)uct;
            }
            RFX_LOG_D(LOG_TAG, "RIL_EMBMS_ActiveSessionNotify, tmgi[%d][%02X,%02X,%02X,%02X,%02X,%02X]",
                                                        i,
                                                        entry.tmgi_info[i].tmgi[0],
                                                        entry.tmgi_info[i].tmgi[1],
                                                        entry.tmgi_info[i].tmgi[2],
                                                        entry.tmgi_info[i].tmgi[3],
                                                        entry.tmgi_info[i].tmgi[4],
                                                        entry.tmgi_info[i].tmgi[5]);
        }
    }

    // RFX_LOG_V(LOG_TAG, "RIL_EMBMS_ActiveSessionNotify request %d, trans_id %d, tmgi_info_count %d",
    //    id, entry.trans_id, entry.tmgi_info_count);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_ActiveSessionNotify));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_ActiveSessionNotify), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsSaiNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_SaiNotify entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        // trans_id
        p->readInt32(&entry.trans_id);

        // cf_total
        p->readInt32(&tmp);
        entry.cf_total = tmp;

        // curFreqData
        for (uint32_t i = 0; i < entry.cf_total; i++) {
            p->readInt32(&entry.curFreqData[i]);
        }

        // csai_count_per_group
        for (uint32_t i = 0; i < entry.cf_total; i++) {
            p->readInt32(&tmp);
            entry.csai_count_per_group[i] = tmp;
        }

        // csai_total
        p->readInt32(&tmp);
        entry.csai_total = tmp;

        // curSaiData
        for (uint32_t i = 0; i < entry.csai_total; i++) {
            p->readInt32(&entry.curSaiData[i]);
        }

        // nf_total
        p->readInt32(&tmp);
        entry.nf_total = tmp;

        // neiFreqData
        for (uint32_t i = 0; i < entry.nf_total; i++) {
            p->readInt32(&entry.neiFreqData[i]);
        }

        // nsai_count_per_group
        for (uint32_t i = 0; i < entry.nf_total; i++) {
            p->readInt32(&tmp);
            entry.nsai_count_per_group[i] = tmp;
        }

        // nsai_total
        p->readInt32(&tmp);
        entry.nsai_total = tmp;

        // neiSaiData
        for (uint32_t i = 0; i < entry.nsai_total; i++) {
            p->readInt32(&entry.neiSaiData[i]);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsEnable request %d, trans_id %d, cf_total %d, csai_total %d",
    //    id, entry.trans_id, entry.cf_total, entry.csai_total);
    // RFX_LOG_V(LOG_TAG, "nf_total %d, nsai_total %d",
    //    entry.nf_total, entry.nsai_total);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_SaiNotify));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_SaiNotify), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToEmbmsOosNotify(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_EMBMS_OosNotify entry;
    int32_t tmp = 0;
    int type = getTypeAndJumpToData(p, id);
    memset(&entry, 0, sizeof(entry));

    if (p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&tmp);
        entry.reason = tmp;
        p->readInt32(&tmp);
        entry.tmgi_info_count = tmp;

        for (uint32_t i = 0; i < entry.tmgi_info_count; i++) {
            p->readInt32(&tmp);
            entry.tmgi_info[i].tmgi_len = tmp;
            uint8_t uct = 0;
            for (uint32_t j = 0; j < entry.tmgi_info[i].tmgi_len; j++) {
                p->read(&uct, sizeof(uct));
                entry.tmgi_info[i].tmgi[j] = (uint8_t)uct;
            }
            RFX_LOG_D(LOG_TAG, "parcelToEmbmsOosNotify, tmgi[%d][%02X,%02X,%02X,%02X,%02X,%02X]",
                                                        i,
                                                        entry.tmgi_info[i].tmgi[0],
                                                        entry.tmgi_info[i].tmgi[1],
                                                        entry.tmgi_info[i].tmgi[2],
                                                        entry.tmgi_info[i].tmgi[3],
                                                        entry.tmgi_info[i].tmgi[4],
                                                        entry.tmgi_info[i].tmgi[5]);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEmbmsOosNotify request %d, response %d, tmgi_info_count %d",
    //    id, entry.reason, entry.tmgi_info_count);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(RIL_EMBMS_OosNotify));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(RIL_EMBMS_OosNotify), RIL_SOCKET_ID(slotId));
    }
}
/// M: eMBMS end
// SMS-START
static void parcelToEtwsNotification(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CBEtwsNotification response;
    int type = getTypeAndJumpToData(p, id);

    memset(&response, 0, sizeof(response));
    if (p->dataAvail() > 0) {
        p->readInt32(&response.warningType);
        p->readInt32(&response.messageId);
        p->readInt32(&response.serialNumber);
        response.plmnId = strdupReadString(p);
        response.securityInfo = strdupReadString(p);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToEtwsNotification request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &response, sizeof(RIL_CBEtwsNotification));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &response, sizeof(RIL_CBEtwsNotification), RIL_SOCKET_ID(slotId));
    }
    if (response.plmnId != NULL) {
        free(response.plmnId);
    }
    if (response.securityInfo != NULL) {
        free(response.securityInfo);
    }
}

static void parcelToGetSmsSimMemStatusCnf(
        RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SMS_Memory_Status mem_status;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32(&mem_status.used);
        p->readInt32(&mem_status.total);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToGetSmsSimMemStatusCnf request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &mem_status, sizeof(mem_status));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &mem_status, sizeof(mem_status), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToSmsParams(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_SmsParams smsParams;
    int32_t v;
    int type = getTypeAndJumpToData(p, id);

    RFX_LOG_D(LOG_TAG, "dispatchSmsParams Enter.");

    memset(&smsParams, 0, sizeof(smsParams));
    if (p->dataAvail() > 0) {
        p->readInt32(&v);
        smsParams.format = v;

        p->readInt32(&v);
        smsParams.vp = v;

        p->readInt32(&v);
        smsParams.pid = v;

        p->readInt32(&v);
        smsParams.dcs = v;

        RFX_LOG_D(LOG_TAG, "dispatchSmsParams format: %d", smsParams.format);
        RFX_LOG_D(LOG_TAG, "dispatchSmsParams vp: %d", smsParams.vp);
        RFX_LOG_D(LOG_TAG, "dispatchSmsParams pid: %d", smsParams.pid);
        RFX_LOG_D(LOG_TAG, "dispatchSmsParams dcs: %d", smsParams.dcs);
        RFX_LOG_D(LOG_TAG, "dispatchSmsParams Send Request..");
    }

    // RFX_LOG_V(LOG_TAG, "parcelToSmsParams request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &smsParams, sizeof(smsParams));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &smsParams, sizeof(smsParams), RIL_SOCKET_ID(slotId));
    }

#ifdef MEMSET_FREED
    memset(&smsParams, 0, sizeof(smsParams));
#endif

    return;
}

Parcel* smsParamsToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();

    fillHeader(parcel, request, token);
    RIL_SmsParams *smsParams = (RIL_SmsParams *) data;
    parcel->writeInt32(4);  // according to RIL.java
    parcel->writeInt32(smsParams->format);
    parcel->writeInt32(smsParams->vp);
    parcel->writeInt32(smsParams->pid);
    parcel->writeInt32(smsParams->dcs);

    return parcel;
}
// SMS-END

/// M: CC: Call control CRSS related handling
static void parcelToCrssN(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_CrssNotification p_cur;

    memset(&p_cur, 0, sizeof(p_cur));

    int type = getTypeAndJumpToData(p, id);

    int dataAvail = p->dataAvail();
    if (dataAvail > 0) {
        p->readInt32(&p_cur.code);
        p->readInt32(&p_cur.type);
        p_cur.number = strdupReadString(p);
        p_cur.alphaid = strdupReadString(p);
        p->readInt32(&p_cur.cli_validity);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToCrssN request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (dataAvail > 0) {
            RfxRilAdapter::responseToRilj(t, e, &p_cur, sizeof(p_cur));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &p_cur, sizeof(p_cur), RIL_SOCKET_ID(slotId));
    }
    if (p_cur.number != NULL) {
        free(p_cur.number);
    }
    if (p_cur.alphaid != NULL) {
        free(p_cur.alphaid);
    }
}

// MTK-START: SIM OPEN CHANNEL WITH P2
Parcel* openChannelParamsToParcel (int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);
    fillHeader(parcel, request, token);
    RIL_OpenChannelParams *openChannelParams = (RIL_OpenChannelParams *) data;

    pString16 = strdup8to16(openChannelParams->aidPtr, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);
    parcel->writeInt32(openChannelParams->p2);

    return parcel;
}
// MTK-END

static void parcelToPcoData(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_PCO_Data *p_cur = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p_cur = (RIL_PCO_Data*)calloc(1, sizeof(RIL_PCO_Data));
        RFX_ASSERT(p_cur != NULL);

        p->readInt32(&p_cur->cid);
        p_cur->bearer_proto = strdupReadString(p);
        p->readInt32(&p_cur->pco_id);
        p->readInt32(&p_cur->contents_length);
        p_cur->contents = (char *) calloc(p_cur->contents_length, sizeof(char));
        RFX_ASSERT(p_cur->contents != NULL);
        p->read(p_cur->contents, p_cur->contents_length);
    }

    if (isResponseType(type)) {
        // Response
        if (e == 0 || p_cur != NULL) {
            RfxRilAdapter::responseToRilj(t, e, p_cur, sizeof(RIL_PCO_Data));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, p_cur, sizeof(RIL_PCO_Data), RIL_SOCKET_ID(slotId));
    }
    if (p_cur != NULL) {
        free(p_cur->bearer_proto);
        free(p_cur->contents);
        free(p_cur);
    }
}

// PHB START
static void parcelToPhbEntries(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_PhbEntryStructure *entry = NULL;
    RIL_PhbEntryStructure **entries = NULL;
    int type = getTypeAndJumpToData(p, id);
    int32_t num = 0;

    if (p->dataAvail() > 0) {
        p->readInt32(&num);

        entries = (RIL_PhbEntryStructure**)calloc(1, sizeof(RIL_PhbEntryStructure*) * num);
        if (entries == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToPhbEntries entries calloc fail");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }
        for (int i = 0; i< num; i++) {
            entry = (RIL_PhbEntryStructure*)calloc(1, sizeof(RIL_PhbEntryStructure));
            if (entry == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToPhbEntries entry calloc fail");
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                goto freeMem;
            }
            entries[i] = entry;
            p->readInt32(&entries[i]->type);
            p->readInt32(&entries[i]->index);
            entries[i]->number = strdupReadString(p);
            p->readInt32(&entries[i]->ton);
            entries[i]->alphaId = strdupReadString(p);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToPhbEntries request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || entries != NULL) {
            RfxRilAdapter::responseToRilj(t, e, entries, num*sizeof(RIL_PhbEntryStructure*));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, entries, num*sizeof(RIL_PhbEntryStructure*), RIL_SOCKET_ID(slotId));
    }

freeMem:
    if (entries != NULL) {
        for (int i=0; i < num; i++) {
            if (entries[i] != NULL) {
                free(entries[i]->number);
                free(entries[i]->alphaId);
                free(entries[i]);
            }
        }
        free(entries);
    }
}

static void parcelToGetPhbMemStorage(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_PHB_MEM_STORAGE_RESPONSE entry;

    int type = getTypeAndJumpToData(p, id);

    memset(&entry, 0, sizeof(entry));
    if (p->dataAvail() > 0) {
        entry.storage = strdupReadString(p);
        p->readInt32(&entry.used);
        p->readInt32(&entry.total);
    }

    // RFX_LOG_V(LOG_TAG, "parcelToGetPhbMemStorage request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &entry, sizeof(entry));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &entry, sizeof(entry), RIL_SOCKET_ID(slotId));
    }
    if (p->dataAvail() > 0) {
        free(entry.storage);
    }
}

/// Ims Data Framework @{
static void parcelToSetupDedicateDataCall(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {

    int type = getTypeAndJumpToData(p, id);

    int num = 0;
    int dataLen = 0;
    p->readInt32(&num);
    dataLen = num * sizeof(RIL_Dedicate_Data_Call_Struct);
    RIL_Dedicate_Data_Call_Struct *args = (RIL_Dedicate_Data_Call_Struct *)calloc(1, dataLen);
    RFX_ASSERT(args != NULL);
    if (p->dataAvail() > 0) {
        parseSetupDedicateDataCall(args, p, num);
    }

    RFX_LOG_D(LOG_TAG, "parcelToRadioCapability request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, args, dataLen);
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, args, dataLen, RIL_SOCKET_ID(slotId));
    }

#ifdef MEMSET_FREED
    memset(args, 0, dataLen);
#endif
    free(args);
}

static void parseSetupDedicateDataCall(RIL_Dedicate_Data_Call_Struct *data, Parcel *p, int num) {
    char *tmp = NULL;

    int i=0, j=0, k=0, l=0;
    for (i = 0; i < num; i++) {
        p->readInt32(&data[i].interfaceId);
        p->readInt32(&data[i].primaryCid);
        p->readInt32(&data[i].cid);
        p->readInt32(&data[i].active);
        p->readInt32(&data[i].signalingFlag);
        p->readInt32(&data[i].bearerId);
        p->readInt32(&data[i].failCause);
        // QOS
        p->readInt32(&data[i].hasQos);
        if (data[i].hasQos) {
            p->readInt32(&data[i].qos.qci);
            p->readInt32(&data[i].qos.dlGbr);
            p->readInt32(&data[i].qos.ulGbr);
            p->readInt32(&data[i].qos.dlMbr);
            p->readInt32(&data[i].qos.ulMbr);
        }
        // tft
        p->readInt32(&data[i].hasTft);
        if (data[i].hasTft) {
            p->readInt32(&data[i].tft.operation);
            p->readInt32(&data[i].tft.pfNumber);
            for (j = 0; j < data[i].tft.pfNumber; j++) {
                p->readInt32(&data[i].tft.pfList[j].id);
                p->readInt32(&data[i].tft.pfList[j].precedence);
                p->readInt32(&data[i].tft.pfList[j].direction);
                p->readInt32(&data[i].tft.pfList[j].networkPfIdentifier);
                p->readInt32(&data[i].tft.pfList[j].bitmap);
                tmp = strdupReadString(p);
                strncpy(data[i].tft.pfList[j].address, tmp,
                        (MAX_IPV6_ADDRESS_LENGTH>strlen(tmp)?
                        strlen(tmp): MAX_IPV6_ADDRESS_LENGTH - 1));
                free(tmp);
                tmp = strdupReadString(p);
                strncpy(data[i].tft.pfList[j].mask, tmp,
                        (MAX_IPV6_ADDRESS_LENGTH>strlen(tmp)?
                        strlen(tmp): MAX_IPV6_ADDRESS_LENGTH - 1));
                free(tmp);
                p->readInt32(&data[i].tft.pfList[j].protocolNextHeader);
                p->readInt32(&data[i].tft.pfList[j].localPortLow);
                p->readInt32(&data[i].tft.pfList[j].localPortHigh);
                p->readInt32(&data[i].tft.pfList[j].remotePortLow);
                p->readInt32(&data[i].tft.pfList[j].remotePortHigh);
                p->readInt32(&data[i].tft.pfList[j].spi);
                p->readInt32(&data[i].tft.pfList[j].tos);
                p->readInt32(&data[i].tft.pfList[j].tosMask);
                p->readInt32(&data[i].tft.pfList[j].flowLabel);
            }
            // TftParameter
            p->readInt32(&data[i].tft.tftParameter.linkedPfNumber);
            for (j = 0; j < data[i].tft.tftParameter.linkedPfNumber; j++) {
                p->readInt32(&data[i].tft.tftParameter.linkedPfList[j]);
            }

            p->readInt32(&data[i].tft.tftParameter.authtokenFlowIdNumber);
            for (j = 0; j < data[i].tft.tftParameter.authtokenFlowIdNumber; j++) {
                p->readInt32(&data[i].tft.tftParameter.authtokenFlowIdList[k].authTokenNumber);
                for (k = 0; k < data[i].tft.tftParameter.authtokenFlowIdList[k].authTokenNumber; k++) {
                    p->readInt32(&data[i].tft.tftParameter.authtokenFlowIdList[j].authTokenList[k]);
                }

                p->readInt32(&data[i].tft.tftParameter.authtokenFlowIdList[k].flowIdNumber);
                for (k = 0; k < data[i].tft.tftParameter.authtokenFlowIdList[k].flowIdNumber; k++) {
                    for (l = 0; l < 4; l++) {
                        p->readInt32(&data[i].tft.tftParameter.authtokenFlowIdList[j].flowIdList[k][l]);
                    }
                }
            }
        }

        // PCSCF
        p->readInt32(&data[i].hasPcscf);
        if (data[i].hasPcscf) {
            tmp = strdupReadString(p);
            strncpy(data[i].pcscf, tmp,
                    ((MAX_PCSCF_NUMBER * MAX_IPV6_ADDRESS_LENGTH - 1)>strlen(tmp)?
                    strlen(tmp): MAX_PCSCF_NUMBER * MAX_IPV6_ADDRESS_LENGTH - 1));
            free(tmp);
        }
    }
}

/// @}

static void parcelToReadPhbEntryExt(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int32_t num = 0;
    int32_t v;
    RIL_PHB_ENTRY *entry = NULL;
    RIL_PHB_ENTRY **entries = NULL;

    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readInt32(&num);
        entries = (RIL_PHB_ENTRY**)calloc(1, sizeof(RIL_PHB_ENTRY*) * num);
        if (entries == NULL) {
            RFX_LOG_E(LOG_TAG, "parcelToReadPhbEntryExt entries calloc fail");
            RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
            return;
        }

        for (int i = 0; i< num; i++) {
            entry = (RIL_PHB_ENTRY*)calloc(1, sizeof(RIL_PHB_ENTRY));
            if (entry == NULL) {
                RFX_LOG_E(LOG_TAG, "parcelToReadPhbEntryExt entry calloc fail");
                RfxRilAdapter::responseToRilj(t, RIL_E_NO_MEMORY, NULL, 0);
                goto freeMem;
            }
            entries[i] = entry;
            // index of the entry
            p->readInt32(&entries[i]->index);
            // phone number
            entries[i]->number = strdupReadString(p);
            // Type of the number
            p->readInt32(&entries[i]->type);
            // text
            entries[i]->text = strdupReadString(p);
            // hidden
            p->readInt32(&entries[i]->hidden);

            // group
            entries[i]->group = strdupReadString(p);
            // anr
            entries[i]->adnumber = strdupReadString(p);
            // Type of the adnumber
            p->readInt32(&entries[i]->adtype);
            // SNE
            entries[i]->secondtext = strdupReadString(p);
            // email
            entries[i]->email = strdupReadString(p);
        }
    }

    // RFX_LOG_V(LOG_TAG, "parcelToReadPhbEntryExt request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0 || entries != NULL) {
            RfxRilAdapter::responseToRilj(t, e, entries, num*sizeof(RIL_PHB_ENTRY*));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, entries, num*sizeof(RIL_PHB_ENTRY*), RIL_SOCKET_ID(slotId));
    }

freeMem:
    for (int i=0; i < num; i++) {
        if (entries[i] != NULL) {
            free(entries[i]->number);
            free(entries[i]->text);
            free(entries[i]->group);
            free(entries[i]->adnumber);
            free(entries[i]->secondtext);
            free(entries[i]->email);
            free(entries[i]);
        }
    }
    free(entries);
    return;
}  // PHB END

// External SIM [START]
Parcel* vsimEventToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();

    RFX_UNUSED(datalen);

    fillHeader(parcel, request, token);
    RIL_VsimEvent *args = (RIL_VsimEvent *) data;
    parcel->writeInt32(args->transaction_id);
    parcel->writeInt32(args->eventId);
    parcel->writeInt32(args->sim_type);

    return parcel;
}

Parcel* vsimOperationEventToParcel(int request, int token, void* data,
        int datalen) {
    Parcel *parcel = new Parcel();
    char16_t *pString16;
    size_t len16 = 0;

    RFX_UNUSED(datalen);

    fillHeader(parcel, request, token);
    RIL_VsimOperationEvent *args = (RIL_VsimOperationEvent *) data;
    parcel->writeInt32(args->transaction_id);
    parcel->writeInt32(args->eventId);
    parcel->writeInt32(args->result);
    parcel->writeInt32(args->data_length);

    pString16 = strdup8to16(args->data, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    return parcel;
}

static void parcelToVsimOperationEvent(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_VsimOperationEvent response;
    int type = getTypeAndJumpToData(p, id);

    memset(&response, 0, sizeof(response));
    if (p->dataAvail() > 0) {
        p->readInt32(&response.transaction_id);
        p->readInt32(&response.eventId);
        p->readInt32(&response.result);
        p->readInt32(&response.data_length);
        response.data = strdupReadString(p);
    }

    //RLOGD("parcelToVsimOperationEvent: data_length=%d, data=%s", response.data_length, response.data);

    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &response, sizeof(RIL_VsimOperationEvent));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &response, sizeof(RIL_VsimOperationEvent), RIL_SOCKET_ID(slotId));
    }
    if (response.data != NULL) {
        free(response.data);
    }
}
// External SIM [END]

Parcel* networkScanToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    RIL_NetworkScanRequest *req = (RIL_NetworkScanRequest *) data;
    fillHeader(parcel, request, token);

    parcel->writeInt32(req->type);
    parcel->writeInt32(req->interval);

    parcel->writeInt32(req->specifiers_length);
    for (int i = 0; i < req->specifiers_length; i++) {
        parcel->writeInt32(req->specifiers[i].radio_access_network);

        parcel->writeInt32(req->specifiers[i].bands_length);
        for (int j = 0; j < req->specifiers[i].bands_length; j++) {
            if (req->specifiers[i].radio_access_network == GERAN) {
                parcel->writeInt32(req->specifiers[i].bands.geran_bands[j]);
            } else if (req->specifiers[i].radio_access_network == UTRAN) {
                parcel->writeInt32(req->specifiers[i].bands.utran_bands[j]);
            } else {
                parcel->writeInt32(req->specifiers[i].bands.eutran_bands[j]);
            }
        }

        parcel->writeInt32(req->specifiers[i].channels_length);
        for (int k = 0; k < req->specifiers[i].channels_length; k++) {
            parcel->writeInt32(req->specifiers[i].channels[k]);
        }
    }
    return parcel;
}

Parcel* keepaliveRequestToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    char16_t *pString16 = NULL;
    size_t len16 = 0;

    fillHeader(parcel, request, token);
    RIL_KeepaliveRequest *kr = (RIL_KeepaliveRequest *) data;
    parcel->writeInt32(kr->type); // NATT_IPV4 = 0, NATT_IPV6 = 1

    pString16 = strdup8to16(kr->sourceAddress, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(kr->sourcePort);

    pString16 = strdup8to16(kr->destinationAddress, &len16);
    parcel->writeString16(pString16, len16);
    free(pString16);

    parcel->writeInt32(kr->destinationPort);
    parcel->writeInt32(kr->maxKeepaliveIntervalMillis);
    parcel->writeInt32(kr->cid);

    return parcel;
}

Parcel* linkCapacityReportingCriteriaToParcel(int request, int token, void* data, int datalen) {
    RFX_UNUSED(datalen);
    Parcel *parcel = new Parcel();
    int num = 0;

    fillHeader(parcel, request, token);
    RIL_LinkCapacityReportingCriteria *args = (RIL_LinkCapacityReportingCriteria *)data;
    parcel->writeInt32(args->hysteresisMs);
    parcel->writeInt32(args->hysteresisDlKbps);
    parcel->writeInt32(args->hysteresisUlKbps);
    num = args->thresholdDlKbpsNumber > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : args->thresholdDlKbpsNumber;
    parcel->writeInt32(num);
    for (int i = 0; i < num; i++) {
        parcel->writeInt32(args->thresholdDlKbpsList[i]);
    }
    num = args->thresholdUlKbpsNumber > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : args->thresholdUlKbpsNumber;
    parcel->writeInt32(num);
    for (int i = 0; i < num; i++) {
        parcel->writeInt32(args->thresholdUlKbpsList[i]);
    }
    parcel->writeInt32(args->accessNetwork);

    return parcel;
}

static void parcelToLinkCapacityEstimate(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_LinkCapacityEstimate response;
    int32_t v = 0;
    int type = getTypeAndJumpToData(p, id);

    memset (&response, 0, sizeof(RIL_LinkCapacityEstimate));

    if (p->dataAvail() > 0) {
        p->readInt32(&v);
        response.downlinkCapacityKbps = (uint32_t)v;

        p->readInt32(&v);
        response.uplinkCapacityKbps = (uint32_t)v;
    }

    // RFX_LOG_D(LOG_TAG, "parcelToLinkCapacityEstimate request %d, type %d", id, type);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &response, sizeof(RIL_LinkCapacityEstimate));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &response, sizeof(RIL_LinkCapacityEstimate), RIL_SOCKET_ID(slotId));
    }
}

/// M: Q emergency number list @{
static void parcelToEmergencyNumberList(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    int num = 0;
    p->readInt32(&num);
    int dataLen = num * sizeof(RIL_EmergencyNumber);
    RFX_LOG_I(LOG_TAG, "[%s] num: %d, dataLen: %d", __FUNCTION__, num, dataLen);
    RIL_EmergencyNumber *pResponse = (RIL_EmergencyNumber *)calloc(1, dataLen);
    if (pResponse == NULL) {
        RFX_LOG_E(LOG_TAG, "[%s] pResponse == NULL", __FUNCTION__);
    }

    for (int i = 0; i < num; i++) {
        pResponse[i].number = strdupReadString(p);
        pResponse[i].mcc = strdupReadString(p);
        pResponse[i].mnc = strdupReadString(p);
        pResponse[i].urns = NULL;
        p->readUint32(&pResponse[i].categories);
        p->readUint32(&pResponse[i].sources);
    }
    // URC only
    RESPONSE_TO_RILJ(id, pResponse, dataLen, RIL_SOCKET_ID(slotId));

    // Free memeory
    for (int i = 0; i < num; i++) {
        free(pResponse[i].number);
        free(pResponse[i].mcc);
        free(pResponse[i].mnc);
    }
    free(pResponse);
}
/// @}

static void parcelToKeepaliveStatus(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_KeepaliveStatus response;
    memset(&response, 0, sizeof(RIL_KeepaliveStatus));
    uint32_t code = 0;
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readUint32(&response.sessionHandle);
        p->readUint32(&code);
        response.code = (RIL_KeepaliveStatusCode)code;
    }
    RFX_LOG_D(LOG_TAG, "[%s] sessionHandle: %d, code: %d", __FUNCTION__,
            response.sessionHandle, response.code);
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &response, sizeof(RIL_KeepaliveStatus));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &response, sizeof(RIL_KeepaliveStatus), RIL_SOCKET_ID(slotId));
    }
}

static void parcelToPhoneCapability(RIL_Token t, RIL_Errno e, int id, Parcel* p, int slotId) {
    RIL_PhoneCapability response;
    uint32_t tmp = 0;
    memset(&response, 0, sizeof(RIL_PhoneCapability));
    int type = getTypeAndJumpToData(p, id);

    if (p->dataAvail() > 0) {
        p->readUint32(&tmp);
        response.maxActiveData = (uint8_t)tmp;
        p->readUint32(&tmp);
        response.maxActiveInternetData = (uint8_t)tmp;
        p->readUint32(&tmp);
        response.isInternetLingeringSupported = (uint8_t)tmp;
        for(int i = 0; i < SIM_COUNT; i++) {
            p->readUint32(&tmp);
            response.logicalModemList[i].modemId = (uint8_t)tmp;
        }
    }
    if (isResponseType(type)) {
        // Response
        if (e == 0) {
            RfxRilAdapter::responseToRilj(t, e, &response, sizeof(RIL_PhoneCapability));
        } else {
            RfxRilAdapter::responseToRilj(t, e, NULL, 0);
        }
    } else {
        // URC
        RESPONSE_TO_RILJ(id, &response, sizeof(RIL_PhoneCapability), RIL_SOCKET_ID(slotId));
    }
}

