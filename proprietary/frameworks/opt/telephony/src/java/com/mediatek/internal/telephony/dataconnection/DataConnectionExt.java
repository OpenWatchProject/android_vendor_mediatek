/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.dataconnection;

import android.content.Context;
import android.net.LinkAddress;
import android.os.AsyncResult;
import android.os.SystemProperties;
import android.telephony.DataFailCause;
import android.telephony.data.ApnSetting;
import android.telephony.PcoData;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccUtils;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.uicc.MtkIccUtilsEx;

import java.util.ArrayList;

/** DataConnectionExt is default implementation for Data Plugin. */
public class DataConnectionExt implements IDataConnectionExt {
    static final String TAG = "DataConnectionExt";

    private static final String PROP_RIL_DATA_IMPI = "vendor.ril.data.impi";
    String[] mImsMccMncList = {"405840", "405854",
                                  "405855", "405856",
                                    "405857", "405858",
                                    "405859", "405860",
                                    "405861", "405862",
                                    "405863", "405864",
                                    "405865", "405866",
                                    "405867", "405868",
                                    "405869", "405870",
                                    "405871", "405872",
                                    "405873", "405874"};

    /**
    *Constructor.
    *@param context The context of caller.
    *
    */
    public DataConnectionExt(Context context) {
    }

    /**
    *Check if domestic roaming is enable or not.
    *@return true if domestic roaming is enable, else return false.
    */
    @Override
    public boolean isDomesticRoamingEnabled() {
        return false;
    }

    /**
    *Check if data allow when data enable is turned off.
    *@param apnType Type of request APN.
    *@return true if data is allowed when data enable is turned off, else return false.
    */
    @Override
    public boolean isDataAllowedAsOff(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT) ||
                TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MMS) ||
                TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DUN) ||
                TextUtils.equals(apnType, PhoneConstants.APN_TYPE_SUPL)) {
            return false;
        }
        return true;
    }

    /**
    *Check if FDN support or not.
    *@return true if FDN is supported.
    */
    public boolean isFdnEnableSupport() {
        // Default is return false, set to true if needed
        return false;
    }

    /**
     * For operator add-on customization when data connection activated.
     *
     * @param apnTypes The data connection apnTypes.
     * @param ifc The interface name.
     */
    public void onDcActivated(String[] apnTypes, String ifc) {
    }

    /**
     * For operator add-on customization when data connection deactivated.
     *
     * @param apnTypes The data connection apnTypes.
     * @param ifc The interface name.
     */
    public void onDcDeactivated(String[] apnTypes, String ifc) {
    }

    /**
    *Get retry timer.
    *@param reason Disconnect done reason.
    *@param defaultTimer The default time.
    *@return Retry time.
    */
    public long getDisconnectDoneRetryTimer(String reason, long defaultTimer) {
        long timer = defaultTimer;
        if (MtkGsmCdmaPhone.REASON_RA_FAILED.equals(reason)) {
            // RA failed, retry after 90s
            timer = 90000;
        }
        return timer;
    }

    /**
    *Print radio log.
    *@param text The context needs to be printed.
    */
    public void log(String text) {
        Rlog.d(TAG, text);
    }

    /**
    *Check if only support single pdn.
    *@return true if only single pdn is supported.
    */
    public boolean isOnlySingleDcAllowed() {
        return false;
    }

    /**
    *Ignore default data unselected.
    *@param apnType The type of apn to be established.
    *
    *@return true if input apntype is ims, emergency, xcap and mms, else return false.
    */
    public boolean ignoreDefaultDataUnselected(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)
                || TextUtils.equals(apnType, PhoneConstants.APN_TYPE_EMERGENCY)
                || TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_XCAP)
                || TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MMS)) {
            log("ignoreDefaultDataUnselected, apnType = " + apnType);
            return true;
        }
        return false;
    }

    /**
    *Ignore data roaming setting for not.
    *@param apnType The type of apn to be established.
    *
    *@return true if input apntype is ims else return false.
    */
    @Override
    public boolean ignoreDataRoaming(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)) {
            log("ignoreDataRoaming, apnType = " + apnType);
            return true;
        }
        return false;
    }

    /**
     * To create data roaming customization instance.
     * @param phone phone proxy
     */
    @Override
    public void startDataRoamingStrategy(Phone phone) {
    }

    /**
     * To dispose data roaming customization instance.
     */
    @Override
    public void stopDataRoamingStrategy() {
    }

    @Override
    /**
     * To get mcc&mnc from IMPI, see TelephonyManagerEx.getIsimImpi().
     * @param defaultValue default value.
     * @param phoneId phoneId used to get IMPI.
     * @return operator numeric
     */
    public String getOperatorNumericFromImpi(String defaultValue, int phoneId) {
        final String mccTag = "mcc";
        final String mncTag = "mnc";
        final int mccLength = 3;
        final int mncLength = 3;

        log("getOperatorNumbericFromImpi got default mccmnc: " + defaultValue);

        if (mImsMccMncList == null  || mImsMccMncList.length == 0) {
            log("Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        String strHexImpi = SystemProperties.get(PROP_RIL_DATA_IMPI+phoneId, "");
        if (strHexImpi.length() == 0) {
            log("Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        String impi = null;
        impi = MtkIccUtilsEx.parseImpiToString(IccUtils.hexStringToBytes(strHexImpi));
        log("impi=" + impi);
        if (impi == null  || impi.equals("")) {
            log("Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        int mccPosition = impi.indexOf(mccTag);
        int mncPosition = impi.indexOf(mncTag);
        if (mccPosition == -1 || mncPosition == -1) {
            log("Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        String masterMccMnc = impi.substring(mccPosition + mccTag.length(), mccPosition
                + mccTag.length() + mccLength) + impi.substring(mncPosition + mncTag.length(),
                mncPosition + mncTag.length() + mncLength);
        log("master MccMnc: " + masterMccMnc);
        if (masterMccMnc == null || masterMccMnc.equals("")) {
            log("Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        for (String mccMnc : mImsMccMncList) {
            if (masterMccMnc.equals(mccMnc)) {
                log("mccMnc matched:" + mccMnc);
                log("Returning mccmnc from IMPI: " + masterMccMnc);
                return masterMccMnc;
            }
        }
        log("Returning default mccmnc: " + defaultValue);
        return defaultValue;
    }

    /**
     * To check metered apn type is decided by load type or not.
     *
     * @return true if metered apn type is decided by load type.
     */
    @Override
    public boolean isMeteredApnTypeByLoad() {
        // Default return false, set to true in OP plugin if needed
        return false;
    }

    /**
     * To check apn type is metered or not.
     *
     * @param type APN type.
     * @param isRoaming true if network in roaming state.
     * @return true if this APN type is metered.
     */
    @Override
    public boolean isMeteredApnType(String type, boolean isRoaming) {
        // Default metered apn type: [default, supl, dun, mms]
        log("isMeteredApnType, apnType = " + type + ", isRoaming = " + isRoaming);
        if (TextUtils.equals(type, PhoneConstants.APN_TYPE_DEFAULT)
                || TextUtils.equals(type, PhoneConstants.APN_TYPE_SUPL)
                || TextUtils.equals(type, PhoneConstants.APN_TYPE_DUN)
                || TextUtils.equals(type, PhoneConstants.APN_TYPE_MMS)) {
            return true;
        }
        return false;
    }

    /**
     * isPermanentCause for nw reject
     */
    @Override
    public boolean isPermanentCause(@DataFailCause.FailCause int cause) {
        return false;
    }

    /**
     * handle PCO data after PS attached
     */
    @Override
    public void handlePcoDataAfterAttached(final AsyncResult ar, final Phone phone,
            final ArrayList<ApnSetting> settings) {
    }

    /**
     * Check if 'default' type PDN is allowed based on the current PCO value
     */
    @Override
    public boolean getIsPcoAllowedDefault() {
        return true;
    }

    /**
     * Set if 'default' type PDN is allowed
     */
    @Override
    public void setIsPcoAllowedDefault(boolean allowed) {
    }

    /**
     * Return operator's specific PCO actions according to current PCO value and apn type
     */
    @Override
    public int getPcoActionByApnType(ApnContext apnContext, PcoData pcoData) {
        return 0;
    }
}
