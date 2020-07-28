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

package mediatek.telephony.data;

import android.os.Bundle;
import android.database.Cursor;
import android.hardware.radio.V1_0.ApnTypes;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.provider.Telephony.Carriers;
import android.telephony.data.ApnSetting;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import java.util.List;

public class MtkApnSetting extends ApnSetting {
    private static final String LOG_TAG = "MtkApnSetting";
    private static final boolean DBG = true;
    private static final boolean VDBG = SystemProperties.get("ro.build.type").equals("eng") ?
            true : false;

    /** APN type for TYPE_WAP traffic. */
    public static final int TYPE_WAP = TYPE_MCX << 1;
    /** APN type for XCAP traffic. */
    public static final int TYPE_XCAP = TYPE_MCX << 2;
    /** APN type for RCS traffic. */
    public static final int TYPE_RCS = TYPE_MCX << 3;
    /** APN type for BIP traffic. */
    public static final int TYPE_BIP = TYPE_MCX << 4;
    /** APN type for VSIM traffic. */
    public static final int TYPE_VSIM = TYPE_MCX << 5;
    /** APN type for all APNs. */
    public static final int MTK_TYPE_ALL = TYPE_ALL | TYPE_WAP | TYPE_XCAP | TYPE_RCS
            | TYPE_BIP | TYPE_VSIM;

    /** MVNO type for ICCID. */
    public static final int MVNO_TYPE_PNN = 4;

    /**
      * The use of inactive timer is to define the timer to disconnect PDN
      * if there's no TX/RX within the period.
      * The default value will be integer 0 if it's undefined in apns-conf.xml.
      */
    public final int inactiveTimer;

    // reflection field to be accessed from AccessNetworksManager
    private static final int[] MTK_SUPPORTED_APN_TYPES = {
            TYPE_DEFAULT,
            TYPE_MMS,
            TYPE_FOTA,
            TYPE_IMS,
            TYPE_CBS,
            TYPE_SUPL,
            TYPE_EMERGENCY,
            // MTK added types
            TYPE_XCAP,
    };

    static {
        APN_TYPE_STRING_MAP.put("wap", TYPE_WAP);
        APN_TYPE_STRING_MAP.put("xcap", TYPE_XCAP);
        APN_TYPE_STRING_MAP.put("rcs", TYPE_RCS);
        APN_TYPE_STRING_MAP.put("bip", TYPE_BIP);
        APN_TYPE_STRING_MAP.put("vsim", TYPE_VSIM);
        APN_TYPE_INT_MAP.put(TYPE_WAP, "wap");
        APN_TYPE_INT_MAP.put(TYPE_XCAP, "xcap");
        APN_TYPE_INT_MAP.put(TYPE_RCS, "rcs");
        APN_TYPE_INT_MAP.put(TYPE_BIP, "bip");
        APN_TYPE_INT_MAP.put(TYPE_VSIM, "vsim");
        MVNO_TYPE_STRING_MAP.put("pnn", MVNO_TYPE_PNN);
        MVNO_TYPE_INT_MAP.put(MVNO_TYPE_PNN, "pnn");
    }

    public MtkApnSetting(int id, String operatorNumeric, String entryName,
            String apnName, String proxyAddress, int proxyPort, Uri mmsc,
            String mmsProxyAddress, int mmsProxyPort, String user, String password,
            int authType, int apnTypeBitmask, int protocol, int roamingProtocol,
            boolean carrierEnabled, int networkTypeBitmask, int profileId,
            boolean modemCognitive, int maxConns, int waitTime, int maxConnsTime, int mtu,
            int mvnoType, String mvnoMatchData, int apnSetId, int carrierId, int skip464xlat,
            int inactiveTimer) {
        super(new Builder()
                .setId(id)
                .setOperatorNumeric(operatorNumeric)
                .setEntryName(entryName)
                .setApnName(apnName)
                .setProxyAddress(proxyAddress)
                .setProxyPort(proxyPort)
                .setMmsc(mmsc)
                .setMmsProxyAddress(mmsProxyAddress)
                .setMmsProxyPort(mmsProxyPort)
                .setUser(user)
                .setPassword(password)
                .setAuthType(authType)
                .setApnTypeBitmask(apnTypeBitmask)
                .setProtocol(protocol)
                .setRoamingProtocol(roamingProtocol)
                .setCarrierEnabled(carrierEnabled)
                .setNetworkTypeBitmask(networkTypeBitmask)
                .setProfileId(profileId)
                .setModemCognitive(true)// For Q, it must be "true" in order to set profile id
                .setMaxConns(maxConns)
                .setWaitTime(waitTime)
                .setMaxConnsTime(maxConnsTime)
                .setMtu(mtu)
                .setMvnoType(mvnoType)
                .setMvnoMatchData(mvnoMatchData)
                .setApnSetId(apnSetId)
                .setCarrierId(carrierId)
                .setSkip464Xlat(skip464xlat));
        this.inactiveTimer = inactiveTimer;
    }

    public static ApnSetting makeApnSetting(Cursor cursor, int mtu, int inactiveTimer) {
        final int apnTypesBitmask = getApnTypesBitmaskFromString(
                cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.TYPE)));
        int networkTypeBitmask = cursor.getInt(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.NETWORK_TYPE_BITMASK));
        if (networkTypeBitmask == 0) {
            final int bearerBitmask = cursor.getInt(cursor.getColumnIndexOrThrow(
                    Telephony.Carriers.BEARER_BITMASK));
            networkTypeBitmask =
                ServiceState.convertBearerBitmaskToNetworkTypeBitmask(bearerBitmask);
        }

        return new MtkApnSetting(
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers._ID)),
            cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.NUMERIC)),
            cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.NAME)),
            cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.APN)),
            emptyToNull(cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.PROXY))),
            portFromString(cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.PORT))),
            UriFromString(cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.MMSC))),
            emptyToNull(cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.MMSPROXY))),
            portFromString(cursor.getString(
                cursor.getColumnIndexOrThrow(Telephony.Carriers.MMSPORT))),
            cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.USER)),
            cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.PASSWORD)),
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.AUTH_TYPE)),
            apnTypesBitmask,
            getProtocolIntFromString(
                cursor.getString(cursor.getColumnIndexOrThrow(Telephony.Carriers.PROTOCOL))),
            getProtocolIntFromString(
                cursor.getString(cursor.getColumnIndexOrThrow(
                    Telephony.Carriers.ROAMING_PROTOCOL))),
            cursor.getInt(cursor.getColumnIndexOrThrow(
                Telephony.Carriers.CARRIER_ENABLED)) == 1,
            networkTypeBitmask,
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.PROFILE_ID)),
            cursor.getInt(cursor.getColumnIndexOrThrow(
                Telephony.Carriers.MODEM_PERSIST)) == 1,
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.MAX_CONNECTIONS)),
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.WAIT_TIME_RETRY)),
            cursor.getInt(cursor.getColumnIndexOrThrow(
                Telephony.Carriers.TIME_LIMIT_FOR_MAX_CONNECTIONS)),
            mtu,
            getMvnoTypeIntFromString(
                cursor.getString(cursor.getColumnIndexOrThrow(
                    Telephony.Carriers.MVNO_TYPE))),
            cursor.getString(cursor.getColumnIndexOrThrow(
                Telephony.Carriers.MVNO_MATCH_DATA)),
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.APN_SET_ID)),
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.CARRIER_ID)),
            cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.Carriers.SKIP_464XLAT)),
            inactiveTimer);
    }

    // reflection for ApnSetting.fromString
    private static ApnSetting fromStringEx(String[] a, int authType, int apnTypeBitmask,
            int protocol, int roamingProtocol, boolean carrierEnabled, int networkTypeBitmask,
            int profileId, boolean modemCognitive, int maxConns, int waitTime, int maxConnsTime,
            int mtu, int mvnoType, String mvnoMatchData, int apnSetId, int carrierId,
            int skip464xlat) {
        int inactiveTimer = 0;
        if (a.length > 29) {
            try {
                inactiveTimer = Integer.parseInt(a[29]);
            } catch (NumberFormatException e) {
                Rlog.e(LOG_TAG, "NumberFormatException, inactive timer = " + a[29]);
            }
        }

        return new MtkApnSetting(-1, a[10] + a[11], a[0], a[1], a[2], portFromString(a[3]),
                UriFromString(a[7]), a[8], portFromString(a[9]), a[4], a[5], authType,
                apnTypeBitmask, protocol, roamingProtocol, carrierEnabled,
                networkTypeBitmask, profileId, modemCognitive, maxConns, waitTime, maxConnsTime,
                mtu, mvnoType, mvnoMatchData, apnSetId, carrierId, skip464xlat, inactiveTimer);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(super.toString());
        sb.append(", ").append(inactiveTimer);
        return sb.toString();
    }

    /**
     * {@link Parcelable#writeToParcel}
     */
    @Override
    public void writeToParcel(Parcel out, int flags) {
        super.writeToParcel(out, flags);
        out.writeInt(inactiveTimer);
    }

    @Override
    protected boolean hasApnType(int type) {
        if (mApnTypeBitmask == MTK_TYPE_ALL) {
            // M: Let the apn '*' skip the "IMS" & "EMERGENCY" apn
            if (type == TYPE_IMS || type == TYPE_EMERGENCY) {
                return false;
            }
        } else {
            // When the request type is DUN and the types only inlucde DUN type,
            // then return true.
            // Fix the issue, CTNET will configure types as default,dun,xcap, when
            // default data in in SIM1 and setup DUN in SIM1, send MMS in SIM2 will
            // fail as DUN's priority is same with MMS.
            // For this scenario, will treat CTNET not support DUN and then Tethering
            // module will request DEFAULT type instead of DUN.
            if (type == TYPE_DUN) {
                if (mApnTypeBitmask == TYPE_DUN) {
                    return true;
                } else {
                    return false;
                }
            }
        }
        return super.hasApnType(type);
    }

    // reflection for ApnSetting.getApnTypeString
    private static Bundle getApnTypeStringEx(int apnType) {
        if (apnType == MTK_TYPE_ALL) {
            Bundle b = new Bundle();
            b.putString("result", "*");
            return b;
        }

        return null;
    }

    // reflection for ApnSetting.getApnTypesBitmaskFromString
    private static Bundle getApnTypesBitmaskFromStringEx(String types) {
        if (TextUtils.isEmpty(types) || TextUtils.equals(types, "*")) {
            Bundle b = new Bundle();
            b.putInt("result", MTK_TYPE_ALL);
            return b;
        }

        return null;
    }

    /**
     * M: get APN string except name since we do not care about it.
     * {@hide}
     * @param ignoreName ignore name or not
     * @return APN string value
     */
    public String toStringIgnoreName(boolean ignoreName) {
        StringBuilder sb = new StringBuilder();
        sb.append("[ApnSettingV6] ");
        if (!ignoreName) {
            sb.append(", ").append(getEntryName());
        }
        sb.append(getId())
                .append(", ").append(getOperatorNumeric())
                .append(", ").append(getApnName())
                .append(", ").append(getProxyAddressAsString())
                .append(", ").append(UriToString(getMmsc()))
                .append(", ").append(getMmsProxyAddressAsString())
                .append(", ").append(portToString(getMmsProxyPort()))
                .append(", ").append(portToString(getProxyPort()))
                .append(", ").append(getAuthType()).append(", ");
        final String[] types = getApnTypesStringFromBitmask(getApnTypeBitmask()).split(",");
        sb.append(TextUtils.join(" | ", types));
        sb.append(", ").append(PROTOCOL_INT_MAP.get(getProtocol()));
        sb.append(", ").append(PROTOCOL_INT_MAP.get(getRoamingProtocol()));
        sb.append(", ").append(isEnabled());
        sb.append(", ").append(getProfileId());
        sb.append(", ").append(isPersistent());
        sb.append(", ").append(getMaxConns());
        sb.append(", ").append(getWaitTime());
        sb.append(", ").append(getMaxConnsTime());
        sb.append(", ").append(getMtu());
        sb.append(", ").append(MVNO_TYPE_INT_MAP.get(getMvnoType()));
        sb.append(", ").append(getMvnoMatchData());
        sb.append(", ").append(getPermanentFailed());
        sb.append(", ").append(getNetworkTypeBitmask());
        sb.append(", ").append(getApnSetId());
        sb.append(", ").append(getCarrierId());

        Rlog.d(LOG_TAG, "toStringIgnoreName: sb = " + sb.toString() + ", ignoreName: " +
                ignoreName);
        //sb.append(", ").append(password);
        return sb.toString();
    }

    /**
     * M: Add to string with parameter ignoreName.
     * {@hide}
     * @param apnSettings apnSettings for APN list
     * @param ignoreName ignore name or not
     * @return APN string value
     */
    public static String toStringIgnoreNameForList(List<ApnSetting> apnSettings,
            boolean ignoreName) {
        if (apnSettings == null || apnSettings.size() == 0) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        for (ApnSetting t : apnSettings) {
            sb.append(((MtkApnSetting) t).toStringIgnoreName(ignoreName));
        }
        return sb.toString();
    }

    private static String emptyToNull(String stringValue) {
        return TextUtils.equals(stringValue, "") ? null : stringValue;
    }
}
