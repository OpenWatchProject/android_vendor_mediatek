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

#ifndef __RMC_DC_COMMON_REQ_HANDLER_H__
#define __RMC_DC_COMMON_REQ_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcDcPdnManager.h"
#include "NetAction.h"

#include <sstream>
#include <list>

#define _IN6_IS_ULA(a)  \
    ((((a)->s6_addr[0] & 0xff) == 0xfc) || (((a)->s6_addr[0] & 0xff) == 0xfd))

#define DATA_SETTING_NUMBERS   5
#define SKIP_DATA_SETTINGS    -2
#define DATA_NOT_ALLOW_RETRY_TIME 500

#define MAX_PROFILEID_LENGTH 10   // max profileID length
#define MAX_COUNT_EXTRA_APN 3 // OEM only, max extra apn count
#define MAX_LENGTH_MCCMNC 12      // max mccmnc length
#define MAX_LENGTH_PROPERTY_NAME 64    // max property name length

// max count of support qualitied network update apn
#define MAX_COUNT_QUALIFIED_NETWORK_UPDATE_APNS 3

typedef enum {
    MOBILE_DATA      = 0,
    ROAMING_DATA     = 1,
    DEFAULT_DATA_SIM = 2,
    DOMESTIC_ROAMING_DATA = 3,
    INTERNATIONAL_ROAMING_DATA = 4
} DATA_SETTING_ITEM;

typedef enum {
    RETRY_TYPE_NO_SUGGEST = 0,
    RETRY_TYPE_NO_RETRY = 1,
    RETRY_TYPE_WITH_SUGGEST = 2
} MD_SUGGESTED_RETRY_TIME_TYPE;

typedef struct {
    char *apn;
    char *username;
    char *password;
    int apnTypeId;
    char *protocol;
    int authtype;
} ApnTableReq;

/// M: IWLAN AP_ASSISTED mode {@
typedef enum {
    ACCESS_NW_TYPE_UNKNOWN = 0,
    ACCESS_NW_TYPE_CELLULAR = 1,
    ACCESS_NW_TYPE_IWLAN = 2
} ACCESS_NETWORK_TYPE;

typedef enum {
    SUPPORT_TRANSPORT_MODE_UNKNOW = 0,
    SUPPORT_TRANSPORT_MODE_CELLULAR_ONLY = 1,
    SUPPORT_TRANSPORT_MODE_WIFI_ONLY = 2,
    SUPPORT_TRANSPORT_MODE_CELLULAR_WIFI = 3
} SUPPORT_TRANSPORT_MODE;

typedef enum {
    PRIORITY_START = 0,
    PRIORITY_NULL = 1,
    PRIORITY_CELLULAR_ONLY = 2,
    PRIORITY_WIFI_ONLY = 3,
    PRIORITY_CELLULAR_PREFER = 4,
    PRIORITY_WIFI_PREFER = 5,
    PRIORITY_END = 6
} QUALIFIED_NETWORK_PRIORITY;

const int SUPPORT_QUALIFIED_NETWORK_UPDATE_APNS[MAX_COUNT_QUALIFIED_NETWORK_UPDATE_APNS] = {
    RIL_APN_TYPE_MMS,
    RIL_APN_TYPE_IMS,
    RIL_APN_TYPE_EMERGENCY
};
/// @}

/*****************************************************************************
 * Class RmcDcCommonReqHandler
 *****************************************************************************/
class RmcDcCommonReqHandler : public RfxBaseHandler, public NetObject {
    public:
        RmcDcCommonReqHandler(int slot_id, int channel_id, RmcDcPdnManager* pdnManager);
        virtual ~RmcDcCommonReqHandler();
        static int getCmdIndexFromApnTable(const int slot_id, ApnTableReq *pApnTableReq);
        static void addEscapeSequence(char *buffer);

    public:
        static RIL_MtkDataProfileInfo* s_LastApnTable[MAX_SIM_COUNT];
        static int s_nLastReqNum[MAX_SIM_COUNT];

    protected:
        virtual void requestSetupDataCall(const sp<RfxMclMessage>& msg);
        virtual void requestDeactivateDataCall(const sp<RfxMclMessage>& msg);
        virtual void onNwPdnAct(const sp<RfxMclMessage>& msg);
        virtual void onNwPdnDeact(const sp<RfxMclMessage>& msg);
        virtual void onNwModify(const sp<RfxMclMessage>& msg);
        virtual void onNwReact(const sp<RfxMclMessage>& msg);
        virtual void onMePdnAct(const sp<RfxMclMessage>& msg);
        virtual void onMePdnDeact(const sp<RfxMclMessage>& msg);
        virtual void onPdnChange(const sp<RfxMclMessage>& msg);
        virtual void onMePdnPropertyChange(const sp<RfxMclMessage>& msg, const int aid);
        virtual void onQualifiedNetworkTypeChanged(const sp<RfxMclMessage>& msg);
        virtual void requestSyncApnTable(const sp<RfxMclMessage>& msg);
        virtual void requestSyncDataSettingsToMd(const sp<RfxMclMessage>& msg);
        virtual void requestResetMdDataRetryCount(const sp<RfxMclMessage>& msg);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg, Vector<int> *vAidList);
        virtual void requestOrSendDataCallList(const sp<RfxMclMessage>& msg, int aid);
        virtual void requestClearAllPdnInfo(const sp<RfxMclMessage>& msg);
        virtual void requestResendSyncDataSettingsToMd(const sp<RfxMclMessage>& msg);
        virtual void onRegisterUrcDone() {};
        // apn, iptype: nullable
        virtual void requestQueryPco(int aid, int ia, const char* apn, const char* iptype);
        // Cleanup all connections
        virtual void handleResetAllConnections(const sp<RfxMclMessage>& msg);
        virtual void requestSetPreferredDataModem(const sp<RfxMclMessage>& msg);
        virtual void requestGetDataContextIds(const sp<RfxMclMessage>& msg);
        virtual void requestSendQualifiedNetworkTypesChanged(const sp<RfxMclMessage>& msg);
        virtual void mobileDataUsageNotify(const sp<RfxMclMessage>& msg);
        virtual void requestStartKeepalive(const sp<RfxMclMessage>& msg);
        virtual void requestStopKeepalive(const sp<RfxMclMessage>& msg);
        virtual void updateKeepaliveStatus(const sp<RfxMclMessage>& msg);
        virtual void onNwLimitNotify(const sp<RfxMclMessage>& msg);

        int activatePdn(const char *reqApn, const char *profileType, Vector<int> *vAidList,
                int isEmergency, MTK_RIL_Data_Call_Response_v11* response, int cmdIndex);
        int updatePdnInformation(const int activatedAid, int protocol, NETAGENT_IFST_STATUS ifst);
        int updateIpAddress(int pdnType, int aid, char* addr1, char* addr2);
        RIL_DataCallFailCause convertFailCauseToRilStandard(int cause);
        int convertDeactCauseToMdStandard(int reason);
        void updateLastFailCause(int cause);
        int getLastFailCause();
        int isAllAidActive(Vector<int> *vAidList);
        void updateActiveStatus();
        int updateDefaultBearerInfo();
        int updateDefaultBearerInfo(int aid);
        void updatePdnAddress();
        int updatePdnAddress(int aid);
        void initDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length);
        void initAidList(int* list);
        void createDataResponse(int transIntfId, int protocol,
                MTK_RIL_Data_Call_Response_v11* response);
        String8 responsesToString(MTK_RIL_Data_Call_Response_v11* responses, int num);
        String8 ipv6AddressToString(int aid);
        void freeDataResponse(MTK_RIL_Data_Call_Response_v11* response);
        int convertIpv6Address(char* output, char* input, int isLinkLocal);
        bool isIpv6Global(char *ipv6Addr);
        void sendDataCallListResponse(const sp<RfxMclMessage>& msg, int deactivatedAid = INVALID_AID,
                int sscMode = SSC_UNKNOWN);
        int deactivateDataCall(const int aid);
        int deactivateDataCall(const int aid, const int reason);
        int getModemSuggestedRetryTime(const char *apnName);
        int getInterfaceId(int transIntfId);
        int confirmPdnUsage(const int aid, const bool bUsed, int sscMode = SSC_UNKNOWN);
        int updatePdnDeactInfo(const int aid);
        bool isFallbackPdn(const int aid);
        bool notifyDeactReasonIfNeeded(const int deactivatedAid);

        void resetPco(int aid);
        void setPco(int aid, const char *option, const char *content);
        void setPco(int aid, const char *buf);
        void getPco(int aid, const char *option, Vector<String8>& vContent);
        void getPco(int apnidx, const char* apn, const char *option, Vector<String8>& vContent);
        void getPco(PDN_INITIATOR who, int idx, const char* apn,
                const char *option, Vector<String8>& vContent);

        bool isDataAllowed(const char* pReqApn);

        bool validateAid(int aid);
        PdnInfo getPdnInfo(int aid);
        void setPdnInfo(int aid, PdnInfo* pdnInfo);
        void clearPdnInfo(int aid);
        bool isDedicateBearer(int aid);
        void setSignalingFlag(int aid, int flag);
        void setAid(int index, int aid);
        void setAidAndPrimaryAid(int index, int aid, int primaryAid);
        void setIsEmergency(int aid, bool isEmergency);
        void setIsDedicateBearer(int aid, bool isDedicateBearer);
        void setReason(int aid, int reason);
        void setDeactReason(int aid, int deactReason);
        void setSscMode(int aid, int sscMode);
        void setPdnType(int aid, int pdnType);
        void setProfileId(int aid, int profileId);
        int getPdnTableSize();
        int getPdnActiveStatus(int aid);
        int getTransIntfId(int aid);
        char* getApnName(int aid);
        int getAid(int index);
        int getPrimaryAid(int index);
        char* getIpv4Dns(int aid, int index);
        char* getIpv6Dns(int aid, int index);
        int getMtu(int aid);
        int getSignalingFlag(int aid);
        char* getIpv4Address(int aid);
        char* getIpv6Address(int aid);
        char* getIpv6Address(int aid, int index);
        int getReason(int aid);
        int getDeactReason(int aid);
        int getSscMode(int aid);
        int getPdnType(int aid);
        int getProfileId(int aid);
        int getSupportNetworkTypes(int apnTypeId);
        void updatePdnActiveStatus(int aid, int pdnActiveStatus);
        void updateApnName(int aid, const char* apnName);
        void updateTransIntfId(int aid, int transIntfId);
        void updateMtu(int aid, int mtu);
        void updateRat(int aid, int rat);
        void updateIpAddress(int aid, const char* ipv4Addr, const char* ipv6Addr);
        void updateBearerId(int aid, int bearerId);
        void updatePcscfAddress(int aid, int index, const char* pcscfAddr);
        void updateIpv4Dns(int aid, int index, const char* v4Dns);
        void updateIpv6Dns(int aid, int index, const char* v6Dns);
        void updateIpv4Address(int aid, const char* ipv4Addr);
        void updateIpv6Address(int aid, const char* ipv6Addr);
        void updateIpv6Address(int aid, int index, const char* ipv6Addr);
        bool isSupportWifiBearer(int bearerBitmask);
        int createDataProfiles(RIL_MtkDataProfileInfo **oldDataProfilePtrs,
                RIL_MtkDataProfileInfo **dataProfilePtrs, int reqNum);
        void initialDataProfile(RIL_MtkDataProfileInfo *dest, RIL_MtkDataProfileInfo *src);
        void deleteDataProfiles(RIL_MtkDataProfileInfo **dest, int count);
        int insertExtraApns(RIL_MtkDataProfileInfo **dataProfilePtrs, int count);
        bool compareApn(RIL_MtkDataProfileInfo *profile1, RIL_MtkDataProfileInfo *profile2);
        void insertExtraTypes(RIL_MtkDataProfileInfo *dest, RIL_MtkDataProfileInfo *src);
        void updateBearerBitmak(RIL_MtkDataProfileInfo *dest, RIL_MtkDataProfileInfo *src);
        void setApnParameter(RIL_MtkDataProfileInfo *xcap, std::string name, std::string value);
        bool getMccmnc(char* mccmnc);
        void updateMdDefaultEmergencyApn(RIL_MtkDataProfileInfo **dataProfilePtrs, int count);
        bool isHandoverRequest(int profileId, int *pTransIntfId);
        void sendQualifiedNetworkTypesChanged(const int aid);
        void setPdnInfoProperty();
        void adjustBearerBitmask(RIL_MtkDataProfileInfo **dataProfilePtrs, int count);
        bool isSupportWifiType(int typesBitmask);
        int getAddressLifeTime(const int aid, int *pLifeTime);
        bool isSscMode3Request(int reason, const char *apn, int *pTransIntfId);

    protected:
        RmcDcPdnManager* m_pPdnManager;
        int m_nGprsFailureCause;

    private:
        static int s_dataSetting[MAX_SIM_COUNT][DATA_SETTING_NUMBERS];
        static int* s_ApnCmdIndex[MAX_SIM_COUNT];
        static int s_dataSetting_resend[MAX_SIM_COUNT][DATA_SETTING_NUMBERS];
};

#endif /* __RMC_DC_COMMON_REQ_HANDLER_H__ */
