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
package com.mediatek.internal.telephony;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.UiccController;

import java.util.concurrent.atomic.AtomicReference;
import java.util.HashMap;
import java.util.Map;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkSuppServContants;

import mediatek.telephony.MtkCarrierConfigManager;

import static com.mediatek.internal.telephony.MtkSuppServContants.*;

// There're might with a potential issue that the carrier config would not ready but SS module
// would call the API before read carrier config done.
// Add new config steps:
// 1. Append a new enum item in MtkSuppServContants.java.
// 2. Init in initConfig().
// 3. Load in loadFromCarrierConfig().
// 4. Create API. (example: isGsmUtSupport())
// Optional: Check getFromOperatorUtilsXXX(),

public class MtkSuppServConf {
    private static final String LOG_TAG = "SuppServConf";

    private Context mContext = null;
    private SSConfigHandler mSSConfigHandler = null;

    private MtkGsmCdmaPhone mPhone = null;

    private UiccController mUiccController = null;

    private final AtomicReference<IccRecords> mIccRecords = new AtomicReference<IccRecords>();

    private final static int EVENT_INIT                          = 0;
    private final static int EVENT_CARRIER_CONFIG_LOADED         = 1;
    private final static int EVENT_CARRIER_CONFIG_LOADED_TIMEOUT = 2;
    private final static int EVENT_RECORDS_LOADED                = 3;
    private final static int EVENT_ICC_CHANGED                   = 4;

    private HashMap<CUSTOMIZATION_ITEM, SSConfig> mCustomizationMap =
            new HashMap<CUSTOMIZATION_ITEM, SSConfig>();

    private int OPERATORUTILS_BOOL_TRUE  = 1;
    private int OPERATORUTILS_BOOL_FALSE = 2;

    private static final int CARRIER_CONFIG_LOADING_TIME = 5000;  // heuristic value

    private class SSConfig {
        public static final int UNSET = 0;
        public static final int DONE  = 1;

        public String mCarrierConfigKey = "";

        public int mSystemPropIdx = -1;

        public boolean bValue = false;
        public boolean bDefault = false;

        public String sValue = "";
        public String sDefault = "";

        public int iValue = -1;
        public int iDefault = -1;

        private int mStatus = UNSET;

        public SSConfig(String carrierConfigKey, int systemPropIdx) {
            this.mCarrierConfigKey = carrierConfigKey;
            this.mSystemPropIdx = systemPropIdx;
        }

        public SSConfig(String carrierConfigKey, int systemPropIdx, boolean bDefault) {
            this.mCarrierConfigKey = carrierConfigKey;
            this.mSystemPropIdx = systemPropIdx;
            this.bDefault = bDefault;
            this.bValue = this.bDefault;
        }

        public SSConfig(String carrierConfigKey, int systemPropIdx, String sDefault) {
            this.mCarrierConfigKey = carrierConfigKey;
            this.mSystemPropIdx = systemPropIdx;
            this.sDefault = sDefault;
            this.sValue = this.sDefault;
        }

        public SSConfig(String carrierConfigKey, int systemPropIdx, int iDefault) {
            this.mCarrierConfigKey = carrierConfigKey;
            this.mSystemPropIdx = systemPropIdx;
            this.iDefault = iDefault;
            this.iValue = this.iDefault;
        }

        public void setValue(boolean bValue) {
            this.bValue = bValue;
            this.mStatus = DONE;
        }

        public void setValue(String sValue) {
            this.sValue = sValue;
            this.mStatus = DONE;
        }

        public void setValue(int iValue) {
            this.iValue = iValue;
            this.mStatus = DONE;
        }

        public void reset() {
            bValue = bDefault;
            sValue = sDefault;
            iValue = iDefault;

            mStatus = UNSET;
        }

        public String toString() {
            return "bValue: " + bValue +
                   ", sValue: " + sValue +
                   ", iValue: " + iValue;
        }
    }

    public MtkSuppServConf(Context context, Phone phone) {
        mContext = context;
        mPhone = (MtkGsmCdmaPhone) phone;

        logi("MtkSuppServConf constructor.");
    }

    public void init(Looper looper) {
        logi("MtkSuppServConf init.");
        // Create a sub thread to handle carrier config IO.
        mSSConfigHandler = new SSConfigHandler(looper);

        initConfig();

        registerCarrierConfigIntent();
        registerEvent();

        Message initMsg = mSSConfigHandler.obtainMessage(EVENT_INIT);
        initMsg.sendToTarget();
    }

    private void registerCarrierConfigIntent() {
        mContext.registerReceiver(mBroadcastReceiver, new IntentFilter(
                CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED));
    }

    private void unregisterCarrierConfigIntent() {
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    private void registerEvent() {
        mUiccController = UiccController.getInstance();
        mUiccController.registerForIccChanged(mSSConfigHandler, EVENT_ICC_CHANGED, null);
    }

    private void unregisterEvent() {
        mUiccController = UiccController.getInstance();
        mUiccController.unregisterForIccChanged(mSSConfigHandler);

        IccRecords r = mIccRecords.get();
        if (r != null) {
            r.unregisterForRecordsLoaded(mSSConfigHandler);
            mIccRecords.set(null);
        }
    }

    public void dispose() {
        unregisterCarrierConfigIntent();
        unregisterEvent();
    }

    private void onUpdateIcc() {
        if (mUiccController == null) {
            return;
        }

        IccRecords newIccRecords = getUiccRecords(UiccController.APP_FAM_3GPP);

        if (newIccRecords == null && mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            // M:  CDMALTEPhone gets 3GPP above, pure CDMA card gets 3GPP2 here.
            newIccRecords = getUiccRecords(UiccController.APP_FAM_3GPP2);
        }

        IccRecords r = mIccRecords.get();
        if (newIccRecords != null && r != null) {
            logd("onUpdateIcc: newIccRecords=" + newIccRecords + ", r=" + r);
        }
        if (r != newIccRecords) {
            if (r != null) {
                logi("Removing stale icc objects.");
                r.unregisterForRecordsLoaded(mSSConfigHandler);
                mIccRecords.set(null);
            }
            if (newIccRecords != null) {
                if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
                    logi("New records found.");
                    mIccRecords.set(newIccRecords);
                    newIccRecords.registerForRecordsLoaded(
                            mSSConfigHandler, EVENT_RECORDS_LOADED, null);
                }
            } else {
                onSimNotReady();
            }
        }
    }

    private void onSimNotReady() {
        logd("onSimNotReady");
        resetConfig();
    }

    private IccRecords getUiccRecords(int appFamily) {
        return mUiccController.getIccRecords(mPhone.getPhoneId(), appFamily);
    }

    private class SSConfigHandler extends Handler {
        public SSConfigHandler() {
        }

        public SSConfigHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            logd("handleMessage msg: " + eventToString(msg.what));
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_ICC_CHANGED:
                    onUpdateIcc();
                    break;
                case EVENT_INIT:
                case EVENT_CARRIER_CONFIG_LOADED_TIMEOUT:
                case EVENT_CARRIER_CONFIG_LOADED:
                    resetConfig();
                    loadCarrierConfig();
                    printConfig();
                    break;
                case EVENT_RECORDS_LOADED:
                    sendMessageDelayed(
                            obtainMessage(EVENT_CARRIER_CONFIG_LOADED_TIMEOUT),
                            CARRIER_CONFIG_LOADING_TIME);
                    break;
                default:
                    break;
            }
        }
    }

    private String eventToString(int eventId) {
        switch (eventId) {
            case EVENT_INIT:
                return "EVENT_INIT";
            case EVENT_CARRIER_CONFIG_LOADED:
                return "EVENT_CARRIER_CONFIG_LOADED";
            case EVENT_CARRIER_CONFIG_LOADED_TIMEOUT:
                return "EVENT_CARRIER_CONFIG_LOADED_TIMEOUT";
            case EVENT_RECORDS_LOADED:
                return "EVENT_RECORDS_LOADED";
            case EVENT_ICC_CHANGED:
                return "EVENT_ICC_CHANGED";
            default:
                return "UNKNOWN_EVENT";
        }
    }

    private void initConfig() {
        logi("initConfig start.");

        mCustomizationMap.put(CUSTOMIZATION_ITEM.GSM_UT_SUPPORT,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_GSM_UT_SUPPORT, 0));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NOT_SUPPORT_XCAP,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NOT_SUPPORT_XCAP, 1));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.TBCLIR,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_TB_CLIR, 2));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.IMS_NW_CW,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_IMS_NW_CW, 3));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.ENABLE_XCAP_HTTP_RESPONSE_409,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_ENABLE_XCAP_HTTP_RESPONSE_409, 4));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.TRANSFER_XCAP_404,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_TRANSFER_XCAP_404, 5));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NOT_SUPPORT_CALL_IDENTITY,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NOT_SUPPORT_CALL_IDENTITY, 6));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.RE_REGISTER_FOR_CF,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_RE_REGISTER_FOR_CF, 7));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.SUPPORT_SAVE_CF_NUMBER,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_SUPPORT_SAVE_CF_NUMBER, 8));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.QUERY_CFU_AGAIN_AFTER_SET,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_QUERY_CFU_AGAIN_AFTER_SET, 9));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NOT_SUPPORT_OCB,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NOT_SUPPORT_OCB, 10));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NOT_SUPPORT_WFC_UT,
                new SSConfig(MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NOT_SUPPORT_WFC_UT, 11));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ENABLE,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ENABLE, 12));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ROAMING,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ROAMING, 13));
        mCustomizationMap.put(CUSTOMIZATION_ITEM.NEED_CHECK_IMS_WHEN_ROAMING,
                new SSConfig(
                    MtkCarrierConfigManager.MTK_KEY_CARRIER_SS_NEED_CHECK_IMS_WHEN_ROAMING, 14));

        logi("initConfig end.");
    }

    private void resetConfig() {
        logi("resetConfig start.");
        for(Map.Entry<CUSTOMIZATION_ITEM, SSConfig> entry : mCustomizationMap.entrySet()) {
            SSConfig config = entry.getValue();
            config.reset();
        }
        logi("resetConfig end.");
    }

    private void loadCarrierConfig() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                mPhone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        int subId = mPhone.getSubId();

        PersistableBundle b = null;

        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        } else {
            logd("CarrierConfigManager is null.");
        }

        if (b != null) {
            loadFromCarrierConfig(b);
        } else {
            logd("Config is null.");
        }
    }

    private void loadFromCarrierConfig(PersistableBundle b) {
        logi("loadFromCarrierConfig start.");

        SSConfig config = null;

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.GSM_UT_SUPPORT);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NOT_SUPPORT_XCAP);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.TBCLIR);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.IMS_NW_CW);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.ENABLE_XCAP_HTTP_RESPONSE_409);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.TRANSFER_XCAP_404);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NOT_SUPPORT_CALL_IDENTITY);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.RE_REGISTER_FOR_CF);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.SUPPORT_SAVE_CF_NUMBER);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.QUERY_CFU_AGAIN_AFTER_SET);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NOT_SUPPORT_OCB);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NOT_SUPPORT_WFC_UT);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ENABLE);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ROAMING);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        config = mCustomizationMap.get(CUSTOMIZATION_ITEM.NEED_CHECK_IMS_WHEN_ROAMING);
        if (config != null && b.containsKey(config.mCarrierConfigKey)) {
            config.setValue(b.getBoolean(config.mCarrierConfigKey, config.bDefault));
        }

        MtkSuppServHelper ssHelper = MtkSuppServManager.getSuppServHelper(mPhone.getPhoneId());
        if (ssHelper != null) {
            ssHelper.notifyCarrierConfigLoaded();
        }
        logi("loadFromCarrierConfig end.");
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            logd("mBroadcastReceiver: action " + intent.getAction());
            if (intent.getAction().equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                logi("Receive ACTION_CARRIER_CONFIG_CHANGED: subId=" + subId +
                        ", mPhone.getSubId()=" + mPhone.getSubId());
                if (subId == mPhone.getSubId()) {
                    logi("CarrierConfigLoader is loading complete!");
                    mSSConfigHandler.removeMessages(EVENT_CARRIER_CONFIG_LOADED_TIMEOUT);
                    Message initMsg = mSSConfigHandler.obtainMessage(EVENT_CARRIER_CONFIG_LOADED);
                    initMsg.sendToTarget();
                }
            }
        }
    };

    private int getSysPropForBool(int idx) {
        int r = -1;
        if (idx > -1) {  // No need to get from system property if -1
            int which = SystemProperties.getInt(SYS_PROP_BOOL_CONFIG, 0);
            if (which > 0 && ((which & (1 << idx)) != 0)) {
                int value = SystemProperties.getInt(SYS_PROP_BOOL_VALUE, 0);
                r = (value & (1 << idx)) != 0 ? 1 : 0;
                logi("getSysPropForBool idx: " + idx
                        + "=" + (r == 1 ? "TRUE" : "FALSE"));
            }
        }
        return r;
    }

    private void printConfig() {
        for(Map.Entry<CUSTOMIZATION_ITEM, SSConfig> entry : mCustomizationMap.entrySet()) {
            CUSTOMIZATION_ITEM key = entry.getKey();
            SSConfig config = entry.getValue();

            logi("" + MtkSuppServContants.toString(key) +
                    " -> " + config.toString());
        }
    }

    public boolean isGsmUtSupport(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.GSM_UT_SUPPORT, mccMnc);
    }

    public boolean isNotSupportXcap(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NOT_SUPPORT_XCAP, mccMnc);
    }

    public boolean isTbClir(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.TBCLIR, mccMnc);
    }

    public boolean isImsNwCW(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.IMS_NW_CW, mccMnc);
    }

    public boolean isNeedCheckImsWhenRoaming(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NEED_CHECK_IMS_WHEN_ROAMING, mccMnc);
    }

    public boolean isEnableXcapHttpResponse409(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.ENABLE_XCAP_HTTP_RESPONSE_409, mccMnc);
    }

    public boolean isTransferXcap404(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.TRANSFER_XCAP_404, mccMnc);
    }

    public boolean isNotSupportCallIdentity(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NOT_SUPPORT_CALL_IDENTITY, mccMnc);
    }

    public boolean isReregisterForCF(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.RE_REGISTER_FOR_CF, mccMnc);
    }

    /**
     * Check whether Operator support save the cf number to sharedpref.
     * @return true if Operator support save the cf number to sharedpref.
     */
    public boolean isSupportSaveCFNumber(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.SUPPORT_SAVE_CF_NUMBER, mccMnc);
    }

    public boolean isQueryCFUAgainAfterSet(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.QUERY_CFU_AGAIN_AFTER_SET, mccMnc);
    }

    public boolean isNotSupportOCB(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NOT_SUPPORT_OCB, mccMnc);
    }

    /**
     * Check whether WifiCalling support UT interface for the
     * supplementary service configuration or not.
     *
     * @return true if support UT interface in GSMPhone
     */
    public boolean isNotSupportWFCUt(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NOT_SUPPORT_WFC_UT, mccMnc);
    }

    // Default: XCAP PDN could request PDN successfully and does not check data enable.
    public boolean isNeedCheckDataEnabled(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ENABLE, mccMnc);
    }

    // Default: XCAP PDN could request PDN successfully and does not check data roaming.
    public boolean isNeedCheckDataRoaming(String mccMnc) {
        return getBooleanValue(CUSTOMIZATION_ITEM.NEED_CHECK_DATA_ROAMING, mccMnc);
    }

    private boolean getBooleanValue(CUSTOMIZATION_ITEM item, String mccMnc) {
        if (mCustomizationMap == null || !mCustomizationMap.containsKey(item)) {
            logi("Null or Without config: " + MtkSuppServContants.toString(item));
            return false;
        }

        SSConfig config = mCustomizationMap.get(item);
        int value = getSysPropForBool(config.mSystemPropIdx);
        if (value != -1) {
            logi("" + MtkSuppServContants.toString(item) + ": " + (value == 1));
            return (value == 1);
        }
        logi("" + MtkSuppServContants.toString(item) + ": " + config.bValue);
        return config != null ? config.bValue : false;
    }


    private String getStringValue(CUSTOMIZATION_ITEM item, String mccMnc) {
        String sConfigFromOperatorUtils = getFromOperatorUtilsString(item, mccMnc);
        if (!sConfigFromOperatorUtils.equals("")) {
            return sConfigFromOperatorUtils;
        }

        if (mCustomizationMap == null || !mCustomizationMap.containsKey(item)) {
            logi("Null or Without config: " + MtkSuppServContants.toString(item));
            return "";
        }

        SSConfig config = mCustomizationMap.get(item);

        logi("" + MtkSuppServContants.toString(item) + ": " + config.sValue);
        return config != null ? config.sValue : "";
    }

    private int getIntValue(CUSTOMIZATION_ITEM item, String mccMnc) {
        if (mCustomizationMap == null || !mCustomizationMap.containsKey(item)) {
            logi("Null or Without config: " + MtkSuppServContants.toString(item));
            return -1;
        }

        SSConfig config = mCustomizationMap.get(item);

        logi("" + MtkSuppServContants.toString(item) + ": " + config.iValue);
        return config != null ? config.iValue : -1;
    }

    private String getFromOperatorUtilsString(CUSTOMIZATION_ITEM item, String mccMnc) {
        switch (item) {
            default:
                return "";
        }
    }

    //------------------

    private void loge(String s) {
        Rlog.e(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logw(String s) {
        Rlog.w(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logi(String s) {
        // default user/userdebug debug level set as INFO
        Rlog.i(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logd(String s) {
        // default eng debug level set as DEBUG
        Rlog.d(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    private void logv(String s) {
        Rlog.v(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }
}
