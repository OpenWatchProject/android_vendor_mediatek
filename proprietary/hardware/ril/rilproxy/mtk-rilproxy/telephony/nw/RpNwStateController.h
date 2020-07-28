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

#ifndef __RP_NW_STATE_CONTROLLER_H__
#define __RP_NW_STATE_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RpNwDefs.h"
#include "Parcel.h"
#include <cutils/properties.h>
#include "utils/Vector.h"

using ::android::Vector;

#define RP_NW_STATE_TAG "RpNwSC"
#define MAX_OPER_NAME_LENGTH 50
#define MAX_OPER_NUM_LENGTH 20

// Use int max, as -1 is a valid value in signal strength
#define CELLINFO_INVALID 0x7FFFFFFF

const static int RESPONSE_TIME_OUT_SEC = 60;

typedef struct {
    int register_state;
    int lac;
    int cid;
    int radio_technology;
    int base_station_id;
    int base_station_latitude;
    int base_station_longitude;
    int css;
    int system_id;
    int network_id;
    /*
     0: roaming, 1: home
     */
    int roaming_ind;
    int prl;
    int prl_roaming_ind;
    int deny_reason;
    int network_exsit;
    int psc;
    int cacheType;
} RIL_VOICE_REG_STATE_CACHE;

typedef struct {
    int register_state;
    int lac;
    int cid;
    int radio_technology;
    int deny_reason;
    int max_data_call;
    int tac;
    int physical_cid;
    int eci;
    int csgid;
    int tadv;
    int cacheType;
    int isEmcBearerSupported;
} RIL_DATA_REG_STATE_CACHE;

typedef struct {
    int gsm_signal_strength;
    int gsm_bit_error_rate;
    int gsm_timing_advance;
    int cdma_dbm;
    int cdma_ecio;
    int evdo_dbm;
    int evdo_ecio;
    int evdo_snr;
    int lte_signal_strength;
    int lte_rsrp;
    int lte_rsrq;
    int lte_rssnr;
    int lte_cqi;
    int lte_timing_advance;
    int tdscdma_signal_strength;
    int tdscdma_bit_error_rate;
    int tdscdma_rscp;
    int wcdma_signal_strength;
    int wcdma_bit_error_rate;
    int wcdma_scdma_rscp;
    int wcdma_ecno;
} RIL_SIGNAL_STRENGTH_CACHE;

typedef struct {
    int wifi_state;
    int gsm_data_register_state;
    int gsm_mcc_mnc;
    int gsm_data_rat;
}PS_REG_STATE;
typedef struct {
    int count;
    char optrAlphaLong[MAX_OPER_NAME_LENGTH];
    char optrAlphaShort[MAX_OPER_NAME_LENGTH];
    char optrNumeric[MAX_OPER_NUM_LENGTH];
} RIL_OPERATOR_CACHE;

typedef struct {
    int domain;
    int state;
    int plmn_id;
    char optrAlphaLong[MAX_OPER_NAME_LENGTH];
    int act;
    int isFemtocell;
    int is_csg_cell;
    int csg_id;
    int csg_icon_type;
    char hnbName[MAX_OPER_NAME_LENGTH];
    int cause;
} RIL_FEMTO_CELL_CACHE;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RpNwStateController : public RfxController {
    RFX_DECLARE_CLASS(RpNwStateController); // Required: declare this class

public:
    RpNwStateController();

    virtual ~RpNwStateController();
    PsRatFamily getPsRatFamily(int radioTechnology);

// Override
protected:
    virtual void onInit();

    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);

private:
    void resetVoiceRegStateCache(RIL_VOICE_REG_STATE_CACHE *voiceCache);
    void resetDataRegStateCache(RIL_DATA_REG_STATE_CACHE *dataCache);
    void resetSignalStrengthCache(RIL_SIGNAL_STRENGTH_CACHE *sigCache);
    void resetOperatorCache(RIL_OPERATOR_CACHE *optrCache);
    void updateVoiceRegStateCache(const sp<RfxMessage>& msg);
    int  calculateCssValue();
    void updateDataRegStateCache(const sp<RfxMessage>& msg);
    void updateSignalStrengthCache(const sp<RfxMessage>& msg);
    void updateOperatorCache(const sp<RfxMessage>& msg);
    void combineVoiceRegState(const sp<RfxMessage>& msg);
    void combineDataRegState(const sp<RfxMessage>& msg);
    void combineSignalStrength(const sp<RfxMessage>& msg);
    void combineOperatorState(const sp<RfxMessage>& msg);
    void updateCsgInfoCache(char **pStrings);
    void updateFemtoCell(const sp<RfxMessage>& urc);
    void resetFemtoCellCache();
    void onReceiveVoiceRegStateResponse(
            const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    void onReceiveDataRegStateResponse(
            const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    void onReceiveSignalStrengthResponse(
            const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    void onReceiveOperatorResponse(
            const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    void onReceiveCellInfoList(
            const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    sp<RfxMessage> genResponseForRilj(int errInd,
        const sp<RfxMessage>& responseMd1, const sp<RfxMessage>& responseMd3);
    void onResponseTimeOut(const sp<RfxMessage>& response);
    int getDeviceMode();
    const char *requestToString(int reqId);
    const char *urcToString(int urcId);
    const char *sourceToString(int srcId);
    char *strdupReadString(Parcel *p);
    void writeStringToParcel(Parcel *p, const char *s);
    void printVoiceCache(RIL_VOICE_REG_STATE_CACHE cache);
    void printDataCache(RIL_DATA_REG_STATE_CACHE cache);
    void printSignalStrengthCache(RIL_SIGNAL_STRENGTH_CACHE cache);
    void printOperatorCache(RIL_OPERATOR_CACHE cache);
    int convertOtaProvisionStatus(int rawState);
    int isEpdgSupport() {
        char property_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.mtk_wfc_support", property_value, "0");
        return atoi(property_value);
    }
    sp<RfxMessage> onMalReceiveDataRegStateResponse(const sp<RfxMessage>& responseMd1);
    void updateMalCgregState(const sp<RfxMessage>& urc);
    void combineMalCgregState(const sp<RfxMessage>& msg);
    void onServiceStateChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    char* getMask(int i);

private:

    int mVoiceRegState;
    int mDataRegState;
    int mVoiceRadioTech;
    int mDataRadioTech;
    int mVoiceRadioSys;
    int mDataRadioSys;
    bool is1xRttInFemtocell;
    bool isEvdoInFemtocell;
    int mCdmaNetworkExist;

    RIL_VOICE_REG_STATE_CACHE gsm_voice_reg_state_cache;
    RIL_VOICE_REG_STATE_CACHE cdma_voice_reg_state_cache;
    RIL_DATA_REG_STATE_CACHE gsm_data_reg_state_cache;
    RIL_DATA_REG_STATE_CACHE cdma_data_reg_state_cache;
    RIL_SIGNAL_STRENGTH_CACHE signal_strength_cache;
    RIL_OPERATOR_CACHE gsm_operator_cache;
    RIL_OPERATOR_CACHE cdma_operator_cache;
    Vector<RIL_CellInfo> cell_info_cache;
    PS_REG_STATE ps_reg_state;

    RIL_FEMTO_CELL_CACHE gsm_femto_cell_cache;
    bool mDebug;
};

#endif /* __RP_NW_STATE_CONTROLLER_H__ */
