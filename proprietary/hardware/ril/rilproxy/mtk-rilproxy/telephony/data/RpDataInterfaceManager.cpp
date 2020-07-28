/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpDataInterfaceManager.h"
#include "RpDataUtils.h"
/*****************************************************************************
 * Class RpDataInterfaceManager
 * This class is created for assign interface Id according to profile Id of
 * request APN and Maintain related information and status for interface id.
 *****************************************************************************/

void RpDataInterfaceManager::initDataInterfaceManager() {
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_PROFILE_ID] = -1;
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_PROTOCOL] = -1;
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_STATE] = DATA_INTERFACE_STATE_DISCONNECTED;
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_INTERFACE_ID] = -1;
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_TOKEN_ID] = -1;
        mCcmmiInterfaceStatus[i][DATA_IF_INFO_REQUEST_TYPE] = -1;
    }
}

/*
 * This function will return an availble(not in-use) InterfaceId for make data call
 * according to profile id of request APN.
 *
 * The range of values for InterfaceId is 1~8 for each sim and mapping to CCMMI interface as below:
 * CCMMI1: Internet (mapping to profileId in HIGH_THROUGHPUT_PROFILE_ID)
 * CCMMI2: Tethering (mapping to profileId in HIGH_THROUGHPUT_PROFILE_ID)
 * CCMMI3: Other(MMS, ADMIN, AKA)
 * CCMMI4: Other(MMS, ADMIN, AKA)
 * CCMMI5: IMS (mapping to profileId in IMS_PROFILE_ID)
 * CCMMI6: IMS emergency (mapping to profileId in IMS_PROFILE_ID)
 * CCMMI7: SS XCAP(IMS)
 * CCMMI8: eMBMS
 *
 * We uses mCcmmiInterfaceStatus[][] to keep the releated info of each interfaceId.
 * For example: The state of CCMMI1 (Internet) will be saved in mCcmmiInterfaceStatus
 * with local index 0 (mCcmmiInterfaceStatus[0][DATA_IF_INFO_STATE]).
 * Therefore, the return value of this should be add 1 to mapping to CCMMI interface on modem.
 *
 * If AP request IMS PDN, the profile Id delivered as 2(in IMS_PROFILE_ID).
 * DataInterfaceManager will assign interface=5 (CCMMI5:IMS) according to mapping rule.
 * So we need to check mCcmmiInterfaceStatus[4][DATA_IF_INFO_STATE] first.
 * If it is not CONNECTED or CONNECTING then return interface Id = 5.
 *
 * For support Multiple-IMS(PS), The value of interfaceId mapping to CCMMI interface
 * has been extend to 1~32 according to profile id and relative offset of request SIM id.
 * (Please reference to adjustInterfaceIdForMultiSIM() for more detial)
 */

int RpDataInterfaceManager::getInterfaceId(int profileId, int protocolId) {
    int interfaceId = -1;
    int localIndex = getLocalIndexId(profileId, protocolId);

    if (localIndex >= 0) {
        interfaceId = localIndex + 1;
        // MPS feature start
        if (RpFeatureOptionUtils::isMultipleImsSupport()
                    && RpFeatureOptionUtils::getMdVersion() >= MD_VERSION_LR12) {
            interfaceId = adjustInterfaceIdForMultiSIM(interfaceId, profileId);
        }
        // MPS feature end
    }
    LOGD("[RP_DC_IFM][%d][getInterfaceId] profileId: %d, protocolId: %d, return: %d",
            slotId, profileId, protocolId, interfaceId);
    return interfaceId;
}

/*
 * This function is created for update related information for setupDataRequest.
 * 1. Update DATA_IF_INFO_STATE to CONNECTING, if responses with no error it will be change to CONNECTED.
 * 2. Update TOKEN_ID of request msg to mapping with response.
 * 3. Since no need to change statue in Ipv4v6 fallback retry, skip request if DATA_IF_INFO_PROTOCOL
 *     is IPV4V6 and DATA_IF_INFO_STATE is CONNECTED.
 */
int RpDataInterfaceManager::updateForSetupDataRequest(int profileId, int protocolId,
            int state, int ifId, int tokenId) {
    int localId = -1;

    // local index mapping to interface id
    if (RpFeatureOptionUtils::isMultipleImsSupport()
            && RpFeatureOptionUtils::getMdVersion() >= MD_VERSION_LR12) {
        localId = getLocalIndexId(profileId, protocolId);
        if (localId < 0) {
            return -1;
        }
    } else {
        localId = ifId - 1;  // for non-MPS case, don't need to care about offset by slot id
    }

    if ((unsigned int)profileId == RIL_DATA_PROFILE_INVALID) {
        // following behavior of AOSP, re-mapping INVAILD to DEFAULT
        // FIXME: Remove this part after getApnProfileID() API finish AOSP refectoring.
        LOGE("[RP_DC_IFM][%d][updateForSetupDataRequest] invalid ProfileId: %d remaping to DEFAULT",
                slotId, profileId);
        profileId = RIL_DATA_PROFILE_DEFAULT;
    }

    // skip update InterfaceId, profileId, protocolId and state for IPv4v6 fallback
    // only set flag in "REQUEST_TYPE" culomn to skip update when receive response
    if (isIpv4v6Fallback(localId, profileId, protocolId)) {
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_REQUEST_TYPE] = REQUEST_FOR_IPV4V6_FALLBACK;
        LOGD("[RP_DC_IFM][%d][updateForSetupDataRequest]skip update state for IPv4v6 fallback",
                slotId);
    } else {
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_PROFILE_ID] = profileId;
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_PROTOCOL] = protocolId;
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_STATE] = state;
        // interface Id of rild index begin is 0, setupdata id delivered begin as local index + 1
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_INTERFACE_ID] = ifId;
        mCcmmiInterfaceStatus[localId][DATA_IF_INFO_REQUEST_TYPE] = -1;
    }
    mCcmmiInterfaceStatus[localId][DATA_IF_INFO_TOKEN_ID] = tokenId;

    LOGD("[RP_DC_IFM][%d][updateForSetupDataRequest] localId: %d, profileId: %d, protocolId: %d, state: %d, ifId: %d, tokenId: %d",
            slotId, localId,
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_PROFILE_ID],
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_PROTOCOL],
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_STATE],
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_INTERFACE_ID],
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_TOKEN_ID]);
    return 0;
}

int RpDataInterfaceManager::updateForSetupDataRequestFromMal(int ifId, int tokenId) {
    // Since no need to change statue for setupDataRequest from MAL.
    // Set DATA_IF_INFO_REQUEST_TYPE to skip update when receive response.

    int localIndex = -1;
    // Find localIndex by interface id
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        // Since the range of interfaceId in request from MAL is 0~7 (or 0~31 for MPS),
        // we need to add 1 for comparison.
        if (mCcmmiInterfaceStatus[i][DATA_IF_INFO_INTERFACE_ID] == (ifId + 1) ) {
            localIndex = i;
            break;
        }
    }

    if (localIndex >= 0) {
        mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_TOKEN_ID] = tokenId;
        LOGD("[RP_DC_IFM][%d][updateForSetupDataRequestFromMal] localId: %d, profileId: %d, protocolId: %d, state: %d, ifId: %d, tokenId: %d",
                slotId, localIndex,
                mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROFILE_ID],
                mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROTOCOL],
                mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_STATE],
                mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_INTERFACE_ID],
                mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_TOKEN_ID]);
        // update request type
        mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_REQUEST_TYPE] = REQUEST_FROM_MAL;
    } else {
        LOGD("[RP_DC_IFM][%d][updateForSetupDataRequestFromMal] no match interfaceId: %d", slotId, ifId);
    }
    return localIndex;
}

/*
 * This method is used to record/update which token Id when request sent by handleRequestDefault() API.
 * Since request may be sent by handleRequestDefault() and the TokenId will be changed,
 * we need to update tokenId.
 */
void RpDataInterfaceManager::updateTokenIdForSetupDataRequest(int oldTokenId, int newTokenId) {
    // find local index which mapping to tokenId
    int localIndex = -1;
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (mCcmmiInterfaceStatus[i][DATA_IF_INFO_TOKEN_ID] == oldTokenId) {
            localIndex = i;
            break;
        }
    }
    if (localIndex >= 0) {
        LOGD("[RP_DC_IFM][%d][updateTokenIdForSetupDataRequest] update oldTokenId:%d to newTokenId:%d",
                slotId, oldTokenId, newTokenId);
        mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_TOKEN_ID] = newTokenId;
    } else {
        LOGE("[RP_DC_IFM][%d][updateTokenIdForSetupDataRequest] can't find token Id in table, oldTokenId:%d",
                slotId, newTokenId);
    }
}

/*
 * This method is used to update state of interface Id in local table according to tokekId of response.
 * But need to exclude the REQUEST_TYPE equals to REQUEST_FROM_MAL or REQUEST_FOR_IPV4V6 FALLBACK.
 */
void RpDataInterfaceManager::updateForSetupDataResponse(int tokenId, bool isResponseFail) {
    LOGD("[RP_DC_IFM][%d][updateForSetupDataResponse] tokenId Id = %d", slotId, tokenId);
    // find local index of tokenId
    int localIndex = -1;
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (mCcmmiInterfaceStatus[i][DATA_IF_INFO_TOKEN_ID] == tokenId) {
            localIndex = i;
            break;
        }
    }
    if (localIndex >= 0) {
        if (mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_REQUEST_TYPE] == REQUEST_FROM_MAL) {
            // skip response of request from MAL
            LOGD("[RP_DC_IFM][%d][updateForSetupDataResponse] in case of request from MAL, don't update state", slotId);
        } else if (mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_REQUEST_TYPE] == REQUEST_FOR_IPV4V6_FALLBACK) {
            // for IPv4v6 fallback don't update table
            LOGD("[RP_DC_IFM][%d][updateForSetupDataResponse] in case of IPv4v6 fallback, don't updat state", slotId);
        } else if (isResponseFail) {  // response result is fail
            // for normal fail case
            updateInterfaceIdStatusByIndex(localIndex, DATA_INTERFACE_STATE_DISCONNECTED);
        } else { // response result is success
            updateInterfaceIdStatusByIndex(localIndex, DATA_INTERFACE_STATE_CONNECTED);
        }
        // always reset token_id and is_from_mal flag when receive response
        mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_TOKEN_ID] = -1;
        mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_REQUEST_TYPE] = -1;
    } else {
        LOGE("[RP_DC_IFM][%d][updateForSetupDataResponse] can't find token Id in table", slotId);
    }
}

/*
 * This method is used to update state of interface Id when reveive DeactivateDataRequest.
 * The special case for request from MAL is it needs to force update pdnInfo when pdn state abnormal,
 * so we no need to update the state of mCcmmiInterfaceStatus table.
 */
void RpDataInterfaceManager::updateForDeactivateDataRequest(int cId, bool isFromMal) {
    LOGD("[RP_DC_IFM][%d][updateForDeactivateDataRequest] cid from request = %d, isFromMal = %d",
            slotId, cId, isFromMal);
    if (cId < 0) {
        return;
    }
    if (!isFromMal) {
        // The range of cId in request of deactivateDataRequest is 0~7 for non-MPS project.
        // For MPS, the range is 0~31.
        int interfaceId = cId + 1;
        updateInterfaceIdStatusByInterfaceId(interfaceId, DATA_INTERFACE_STATE_DISCONNECTED);
    }
}

void RpDataInterfaceManager::updateForDeactAllDataCallRequest() {
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (!isInterfaceAvailable(i)) {
            LOGD("[RP_DC_IFM][%d][updateForDeactAllDataCallRequest] Deact localId = %d, ifId=%d",
                    slotId, i, mCcmmiInterfaceStatus[i][DATA_IF_INFO_INTERFACE_ID]);
            updateStateToDisconnected(i);
        }
    }
}

/*
 * This method is used to update state of interface Id when reveive URC DataCallListChanged.
 * RpDataController will delivered newConnectionInfo list when reveive URC.
 * We need to cross check mCcmmiInterfaceStatus[][] with newConnectionInfo list.
 * If the state of interfaceId in mCcmmiInterfaceStatus array is CONNECTED,
 * but the connectionInfo of interfaceId is not in newConnectionInfo list,
 * then the state of those interfaceId should be update to DISCONNECTED.
  */
void RpDataInterfaceManager::updateForDataCallListChangeUrc(Vector<RpDataConnectionInfo*>* newConnectionInfo) {
    int InfoSize = newConnectionInfo->size();
    int ifId = -1;
    bool isStillConnected;

    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (mCcmmiInterfaceStatus[i][DATA_IF_INFO_STATE] == DATA_INTERFACE_STATE_CONNECTED) {
            // get interface Id for state is "connected"
            int ifId = mCcmmiInterfaceStatus[i][DATA_IF_INFO_INTERFACE_ID];

            // search this interface id in newConnectionInfo list, check if it is still connected
            isStillConnected = false;
            for (int j = 0; j < InfoSize; j++) {
                // Since interfaceId of rild index begin is 0, the range of InterfaceId in RpDataConnectionInfo is 0~7
                if (newConnectionInfo->itemAt(j)->interfaceId == (ifId-1) ) {
                    // find the same interfaceid in newConnectionInfo list, it still connected.
                    isStillConnected = true;
                    break;
                }
            }

            // if we can't find this interface id in newConnectionInfo list, update state of this interface id to disconnected.
            if (!isStillConnected) {
                LOGD("[RP_DC_IFM][%d][updateForDataCallListChangeUrc] Deact local id = %d, ifId=%d, profile id=%d",
                        slotId, i, ifId, mCcmmiInterfaceStatus[i][DATA_IF_INFO_PROFILE_ID]);
                updateStateToDisconnected(i);
            }
        }
    }
}

bool RpDataInterfaceManager::updateInterfaceIdStatusByInterfaceId(int ifId, int state) {
    bool result = true;
    int localId = -1 ;

    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (mCcmmiInterfaceStatus[i][DATA_IF_INFO_INTERFACE_ID] == ifId) {
            localId = i;
            break;
        }
    }

    if (localId >= 0) {
        if (state == DATA_INTERFACE_STATE_DISCONNECTED){
            updateStateToDisconnected(localId);
        } else {
            mCcmmiInterfaceStatus[localId][DATA_IF_INFO_STATE] = state;
        }
        LOGD("[RP_DC_IFM][%d][updateInterfaceIdStatusByInterfaceId] localId: %d, ifId: %d, state: %d", slotId, localId,
                    mCcmmiInterfaceStatus[localId][DATA_IF_INFO_INTERFACE_ID],
                    mCcmmiInterfaceStatus[localId][DATA_IF_INFO_STATE]);
    } else {
        LOGE("[RP_DC_IFM][%d][updateInterfaceIdStatusByInterfaceId] can't find vaild inerfaceId", slotId);
        result = false;
    }
    return result;
}

bool RpDataInterfaceManager::updateInterfaceIdStatusByIndex(int index, int state) {
    bool result = true;

    if ((index >= 0)) {
        mCcmmiInterfaceStatus[index][DATA_IF_INFO_STATE] = state;
        LOGD("[RP_DC_IFM][%d][updateInterfaceIdStatusByIndex] localIndex: %d, state: %d", slotId, index, state);
    } else {
        LOGE("[RP_DC_IFM][%d][updateInterfaceIdStatusByIndex] can't find token Id", slotId);
        result = false;
    }
    return result;
}

void RpDataInterfaceManager::updateStateToDisconnected(int localIndex) {
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROFILE_ID] = RIL_DATA_PROFILE_INVALID;
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROTOCOL] = -1;
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_STATE] = DATA_INTERFACE_STATE_DISCONNECTED;
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_INTERFACE_ID] = -1;
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_TOKEN_ID] = -1;
    mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_REQUEST_TYPE] = -1;
}

bool RpDataInterfaceManager::isIpv4v6Fallback(int localIndex, int profileId, int protocolId) {
    if ((mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_STATE] == DATA_INTERFACE_STATE_CONNECTED)
            && (mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROFILE_ID] == profileId)
            && (mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROTOCOL] == protocolId)
            && (mCcmmiInterfaceStatus[localIndex][DATA_IF_INFO_PROTOCOL] == IPV4V6)) {
        LOGD("[RP_DC_IFM][%d][isIpv4v6Fallback] in case of IPv4v6 fallback", slotId);
        return true;
    }
    return false;
}

bool RpDataInterfaceManager::isHighThroughputProfile(int profileId) {
    bool isHighThroughput = false;
    for (int i = 0; i < MAX_COUNT_HIGH_THROUGHPUT_PROFILE_ID; i++) {
        if (HIGH_THROUGHPUT_PROFILE_ID[i] == profileId) {
            isHighThroughput = true;
            break;
        }
    }
    return isHighThroughput;
}

bool RpDataInterfaceManager::isImsProfile(int profileId) {
    bool isIms = false;
    for (int i = 0; i < MAX_COUNT_IMS_PROFILE_ID; i++) {
        if (IMS_PROFILE_ID[i] == profileId) {
            isIms = true;
            break;
        }
    }
    return isIms;
}

bool RpDataInterfaceManager::isInterfaceAvailable(int index) {
    if (mCcmmiInterfaceStatus[index][DATA_IF_INFO_STATE] == DATA_INTERFACE_STATE_DISCONNECTED) {
        return true;
    }
    return false;
}

// MPS feature start
/*
 * This function will return a index of available Interface Id in array mCcmmiInterfaceStatus[][].
 *
 * We uses mCcmmiInterfaceStatus[][] to keep the releated info of each interfaceId.
 * For example: The state of CCMMI3 (Others) will be saved in mCcmmiInterfaceStatus
 * with local index 2 (mCcmmiInterfaceStatus[2][DATA_IF_INFO_STATE]).
 *
 * Special Case1: for invaild profile id, we follow design of AOSP to assign default.
 * Special Case2: for AP fallback, we using the old interface id directly.
 */
int RpDataInterfaceManager::getLocalIndexId(int profileId, int protocolId) {
    int id = -1;

    if ((unsigned int)profileId == RIL_DATA_PROFILE_INVALID) {
        // following behavior of AOSP, re-mapping INVAILD to DEFAULT
        // FIXME: Remove this part after getApnProfileID() API finish AOSP refectoring.
        LOGE("[RP_DC_IFM][%d][getLocalIndexId] invalid ProfileId: %d remaping to DEFAULT",
                slotId, profileId);
        profileId = RIL_DATA_PROFILE_DEFAULT;
    }

    // for IPv4v6 fallback
    for (int i = 0; i < MAX_CCMMI_LOCAL_IF_ID_COUNT; i++) {
        if (isIpv4v6Fallback(i, profileId, protocolId)) {
            LOGD("[RP_DC_IFM][%d][getLocalIndexId] in case of IPv4v6 fallback", slotId);
            id = i;
            break;
        }
    }

    // For 'keep default PDN' feature, we need always bind the default PDN to interface id 0.
    if (RIL_DATA_PROFILE_DEFAULT == profileId) {
        id = MIN_ID_HIGH_THROUGHPUT;
    }

    // for High throughput APN
    if ((id < 0) && (isHighThroughputProfile(profileId))) {
        if (isInterfaceAvailable(MAX_ID_HIGH_THROUGHPUT)) {
            id = MAX_ID_HIGH_THROUGHPUT;
        }
    }

    // for IMS APN
    if ((id < 0) && (isImsProfile(profileId))) {
        if (id < 0) {
            // Set IMS: 4, EIMS: 5 for interface (fixed the interface)
            int idStart = MIN_ID_IMS_THROUGHPUT;
            if (profileId == RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
                idStart += 1;
            }

            if (isInterfaceAvailable(idStart)) {
                id = idStart;
            }
        }
    }

    // for others troughput
    if (id < 0) {
        for (int i = MIN_ID_OTHERS_THROUGHPUT; i <= MAX_ID_OTHERS_THROUGHPUT; i++) {
            if (isInterfaceAvailable(i)) {
                id = i;
                break;
            }
        }
    }
    return id;
}

/*
 * This function will return a new InterfaceId according to relative offset of request SIM id.
 *
 * A list of relative offset interfaceId to Others/IMS-related types on Multi-SIMs.
 * For SIM1,
 *               CCMNI3(Base) -> Others(MMS, ADMIN, AKA)
 *               CCMNI4(Base) -> Others(MMS, ADMIN, AKA)
 *               CCMNI5(Base) -> IMS
 *               CCMNI6(Base) -> IMS Emergency
 * For SIM2,
 *               CCMNI10(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI11(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI16(Base+Offset) -> IMS
 *               CCMNI19(Base+Offset) -> IMS Emergency
 * For SIM3,
 *               CCMNI12(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI13(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI17(Base+Offset) -> IMS
 *               CCMNI20(Base+Offset) -> IMS Emergency
 * For SIM4,
 *               CCMNI14(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI15(Base+Offset) -> Others(MMS, ADMIN, AKA)
 *               CCMNI18(Base+Offset) -> IMS
 *               CCMNI21(Base+Offset) -> IMS Emergency
 */
int RpDataInterfaceManager::adjustInterfaceIdForMultiSIM(int ifId, int profileId) {
    int newId = ifId;
    if (slotId > 0) {
        if (isHighThroughputProfile(profileId)) {
            // HighThroughput type
            LOGD("[RP_DC_IFM][%d][adjustInterfaceIdForMultiSIM] HighThroughpu = %d, no change!"
                    ,slotId ,profileId);
        } else if (isImsProfile(profileId)) {
            if (profileId != RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
                // IMS
                newId = ifId + IMS_CHANNEL_OFFSET + (slotId - 1);
            } else {
                // IMS Emergency
                newId = ifId + IMS_EMERGENCY_CHANNEL_OFFSET + (slotId - 1);
            }
        } else {
            // Others
            newId = ifId + OTHERS_CHANNEL_OFFSET
                    + ((slotId - 1) * OTHERS_CHANNEL_MAX_NUM);
        }
    }
    LOGD("[RP_DC_IFM][%d][adjustInterfaceIdForMultiSIM] ifId=%d, newId=%d", slotId, ifId, newId);
    return newId;
}
// MPS feature end
