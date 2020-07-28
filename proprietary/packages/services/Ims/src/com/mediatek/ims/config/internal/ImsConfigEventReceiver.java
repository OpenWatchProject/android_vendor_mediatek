package com.mediatek.ims.config.internal;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.common.ImsCarrierConfigConstants;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsManagerOemPlugin;
import com.mediatek.ims.ril.ImsCommandsInterface;

public class ImsConfigEventReceiver extends BroadcastReceiver {
    private static final String TAG = "ImsConfigEventReceiver";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    // For add-on decoupling
    private static final String ACTION_CXP_NOTIFY_FEATURE =
        "com.mediatek.common.carrierexpress.cxp_notify_feature";

    private static final String PROPERTY_MTK_WFC_SUPPORT = "persist.vendor.mtk_wfc_support";
    private static final String PROPERTY_MTK_VOLTE_SUPPORT = "persist.vendor.volte_support";
    private static final String PROPERTY_MTK_VILTE_SUPPORT = "persist.vendor.vilte_support";
    private static final String PROPERTY_MTK_VIWIFI_SUPPORT = "persist.vendor.viwifi_support";
    private static final String PROPERTY_MTK_RCS_UA_SUPPORT = "persist.vendor.mtk_rcs_ua_support";

    private String mLogTag;

    private Handler mHandler;
    private final int mPhoneId;
    private int mMainPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    private ImsCommandsInterface mRilAdapter;

    private ImsManagerOemPlugin mImsManagerOemPlugin = null;

    public ImsConfigEventReceiver(Handler handler, int phoneId, ImsCommandsInterface imsRilAdapter) {
        super();
        mPhoneId = phoneId;
        mHandler = handler;

        mRilAdapter = imsRilAdapter;
        mLogTag = TAG + "[" + phoneId + "]";
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        switch (intent.getAction()) {
            case TelephonyIntents.ACTION_SIM_STATE_CHANGED:
                String state = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                        SubscriptionManager.INVALID_PHONE_INDEX);
                if (phoneId == mPhoneId) {
                    switch (state) {
                        case IccCardConstants.INTENT_VALUE_ICC_READY:
                            if (ImsConfigUtils.isWfcEnabledByUser(context, mPhoneId) &&
                                    (ImsConfigUtils.getWfcMode(context, mPhoneId) == ImsConfig
                                            .WfcModeFeatureValueConstants.WIFI_ONLY)) {
                                ImsConfigUtils.sendWifiOnlyModeIntent(context, mPhoneId, true);
                                Rlog.d(mLogTag, "Turn OFF radio, after sim ready and wfc mode is " +
                                        "wifi_only");
                            }
                            break;
                        case IccCardConstants.INTENT_VALUE_ICC_ABSENT:
                        case IccCardConstants.INTENT_VALUE_ICC_LOADED:
                            if (!"1".equals(SystemProperties.get("persist.vendor" +
                                    ".mtk_dynamic_ims_switch"))) {
                                Rlog.d(mLogTag, "updateImsServiceConfig after SIM event, " +
                                        "phoneId:" + phoneId);
                                // Force update IMS feature values after SIM event.
                                updateImsServiceConfig(context, phoneId);
                            }
                            break;


                    }
                }
                break;

            case CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED:
                handleCarrierConfigChanged(context, intent);
                break;

            case MtkImsConstants.ACTION_MTK_MMTEL_READY:
                phoneId = intent.getIntExtra(ImsManager.EXTRA_PHONE_ID,
                        SubscriptionManager.INVALID_PHONE_INDEX);

                if (phoneId == mPhoneId) {
                    resetWfcModeFlag("ACTION_MTK_MMTEL_READY");
                    Rlog.d(mLogTag, "ACTION_MTK_MMTEL_READY, update IMS config with phoneId:" +
                          phoneId);
                    updateImsServiceConfig(context, phoneId);
                }
                break;

            case TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE:
                boolean isNeedUpdate = true;

                if (mMainPhoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                    // First time get mMainPhoneId
                    mMainPhoneId = ImsCommonUtil.getMainCapabilityPhoneId();
                    isNeedUpdate = false;
                } else {
                    phoneId = ImsCommonUtil.getMainCapabilityPhoneId();

                    if (phoneId == mMainPhoneId) {
                        // Not SIM Switch
                        isNeedUpdate = false;
                    } else {
                        // SIM Switch
                        mMainPhoneId = phoneId;

                        if (mPhoneId != mMainPhoneId) {
                            isNeedUpdate = false;
                        }
                    }
                }

                if (isNeedUpdate) {
                    resetWfcModeFlag("ACTION_SET_RADIO_CAPABILITY_DONE");
                    Rlog.d(mLogTag, "SET_RADIO_CAPABILITY_DONE, update IMS config with phoneId:" +
                          mPhoneId);
                    updateImsServiceConfig(context, mPhoneId);
                }
                break;
            case ACTION_CXP_NOTIFY_FEATURE:

                Bundle opFeature = intent.getExtras();

                if (opFeature != null) {

                    // Update feature support property
                    updateFeatureSupportProperty(opFeature);

                    // Update enable property
                    updateImsServiceConfig(context, phoneId);
                } else {
                    Rlog.d(mLogTag, "ACTION_CXP_NOTIFY_FEATURE, opFeature is null");
                }

                break;
        }
    }

    private void updateFeatureSupportProperty(Bundle bundle) {

        // WFC
        boolean isWfcOn = bundle.getString(PROPERTY_MTK_WFC_SUPPORT, "0").equals("1");

        if (isWfcOn) {
            mRilAdapter.turnOnWfc(null);
        } else {
            mRilAdapter.turnOffWfc(null);
        }

        // VolTE
        boolean isVolteOn = bundle.getString(PROPERTY_MTK_VOLTE_SUPPORT, "0").equals("1");

        if (isVolteOn) {
            mRilAdapter.turnOnVolte(null);
        } else {
            mRilAdapter.turnOffVolte(null);
        }

        // ViLTE
        boolean isVilteOn = bundle.getString(PROPERTY_MTK_VILTE_SUPPORT, "0").equals("1");

        if (isVilteOn) {
            mRilAdapter.turnOnVilte(null);
        } else {
            mRilAdapter.turnOffVilte(null);
        }

        // ViWiFi
        boolean isViWiFiOn = bundle.getString(PROPERTY_MTK_VIWIFI_SUPPORT, "0").equals("1");

        if (isViWiFiOn) {
            mRilAdapter.turnOnViwifi(null);
        } else {
            mRilAdapter.turnOffViwifi(null);
        }

        // RCS UA
        boolean isRcsUaOn = bundle.getString(PROPERTY_MTK_RCS_UA_SUPPORT, "0").equals("1");

        if (isRcsUaOn) {
            mRilAdapter.turnOnRcsUa(null);
        } else {
            mRilAdapter.turnOffRcsUa(null);
        }

        Rlog.d(mLogTag, "updateFeatureSupportProperty()," +
               " volte:" + (isVolteOn ? "1" : "0") +
               " wfc:" +  (isWfcOn ? "1" : "0") +
               " vilte:" +  (isVilteOn ? "1" : "0") +
               " viwifi:" +  (isViWiFiOn ? "1" : "0") +
               " isRcsUaOn:" +  (isRcsUaOn ? "1" : "0"));
    }

    private void handleCarrierConfigChanged(Context context, Intent intent) {
        int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                SubscriptionManager.INVALID_PHONE_INDEX);
        if (phoneId != mPhoneId) {
            return;
        }

        TelephonyManager tm = TelephonyManager.getDefault();
        if (tm != null) {
            if (!tm.hasIccCard(phoneId)) {
                // If carrier config changed notification is triggered by SIM plug-out event,
                // ImsConfigController will query op code with default 0 and reset the config storage,
                // however, the provisioned data shall be kept.
                Rlog.e(mLogTag, "No need to reload config storage");
                return;
            }
        }

        resetWfcModeFlag("ACTION_CARRIER_CONFIG_CHANGED");

        boolean removeWfcPrefMode = ImsConfigUtils.getBooleanCarrierConfig(context,
                ImsCarrierConfigConstants.MTK_KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL, phoneId);
        boolean wfcModeEditable = ImsConfigUtils.getBooleanCarrierConfig(context,
                CarrierConfigManager.KEY_EDITABLE_WFC_MODE_BOOL, phoneId);
        Rlog.d(mLogTag, "KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL = " + removeWfcPrefMode);
        Rlog.d(mLogTag, "KEY_EDITABLE_WFC_MODE_BOOL = " + wfcModeEditable);
        if (removeWfcPrefMode || !wfcModeEditable) {
            int wfcMode = ImsConfigUtils.getIntCarrierConfig(context,
                    CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT, phoneId);
            Rlog.d(mLogTag, "ACTION_CARRIER_CONFIG_CHANGED: set wfc mode = " + wfcMode +
                    ", phoneId:" + phoneId);

            // avoid set default wfc mode on non-main sim to setting global in single ims project
            if (ImsCommonUtil.supportMims() ||
                    ImsCommonUtil.getMainPhoneIdForSingleIms() == phoneId) {
                ImsManager.getInstance(context, phoneId).setWfcMode(wfcMode);
            } else {
                Rlog.d(mLogTag, "no set wfc mode due to mims: " + ImsCommonUtil.supportMims() +
                        ", main phone id:" + ImsCommonUtil.getMainPhoneIdForSingleIms());
            }
        }
    }

    private void resetWfcModeFlag(String reason) {
        mHandler.removeMessages(ImsConfigController.MSG_RESET_WFC_MODE_FLAG);
        Message msg = new Message();
            Rlog.d(mLogTag, "resetWfcModeFlag, reason: " + reason);
        msg.what = ImsConfigController.MSG_RESET_WFC_MODE_FLAG;
        mHandler.sendMessage(msg);
    }

    private void updateImsServiceConfig(Context context, int phoneId) {
        if (mImsManagerOemPlugin == null) {
            mImsManagerOemPlugin = ExtensionFactory.makeOemPluginFactory(context)
                    .makeImsManagerPlugin(context);
        }

        if (ImsCommonUtil.supportMims() ||
            phoneId == ImsCommonUtil.getMainCapabilityPhoneId()) {
            // update for each phoneId or Main phoneId only if not MIMS
            mImsManagerOemPlugin.updateImsServiceConfig(context, phoneId, true);
        } else {
            if (DEBUG)
                Rlog.d(mLogTag, "Do not update if phoneId is not main capability");
        }
    }
}
