/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ims;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.util.Log;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.common.SubscriptionManagerHelper;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.ims.internal.IImsConfig;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.config.internal.ImsConfigAdapter;
import com.mediatek.ims.config.internal.ImsConfigImpl;
import com.mediatek.ims.config.internal.ImsConfigUtils;
import com.mediatek.ims.config.internal.MtkImsConfigImpl;
import com.mediatek.ims.internal.IMtkImsConfig;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsManagerOemPlugin;
import com.mediatek.ims.ril.ImsCommandsInterface;

import java.util.HashMap;
import java.util.Map;

public class ImsConfigManager {
    private static final String LOG_TAG = "ImsConfigManager";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";

    private Context mContext;

    private ImsCommandsInterface [] mImsRILAdapters = null;

    /// M: IMS Configuration
    /// Maintain ImsConfigImpl,mImsConfigAdapterMap,ImsConfig instance for each phoneId
    private Map<Integer, ImsConfigImpl> mImsConfigInstanceMap =
            new HashMap<Integer, ImsConfigImpl>();
    private Map<Integer, ImsConfigAdapter> mImsConfigAdapterMap =
            new HashMap<Integer, ImsConfigAdapter>();

    /// M: BSP+ feature @{
    private Map<Integer, IMtkImsConfig> mMtkImsConfigInstanceMap =
            new HashMap<Integer, IMtkImsConfig>();
    /// @}

    private ImsManagerOemPlugin mImsManagerOemPlugin = null;

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            if (ImsConfigContract.ACTION_DYNAMIC_IMS_SWITCH_TRIGGER.equals(intent.getAction())) {
                /// M: Dynamic IMS Switch, intent send from ImsConfigReceiver

                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

                Rlog.d(LOG_TAG, "DYNAMIC_IMS_SWITCH_TRIGGER phoneId:" + phoneId +
                            ", simState:" + simState);

                // Should update ImsResCapability value in ImsConfig for different phoneId
                updateImsResrouceCapability(context, intent);

            } else if (ImsConfigContract.ACTION_IMS_CONFIG_CHANGED.equals(intent.getAction())) {
                int phoneId = intent.getIntExtra(ImsConfigContract.EXTRA_PHONE_ID, 0);
                int itemId = intent.getIntExtra(ImsConfigContract.EXTRA_CHANGED_ITEM, -1);

                Log.d(LOG_TAG, "ACTION_IMS_CONFIG_CHANGED phoneId:" + phoneId +
                            ", itemId:" + itemId);
                if (MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE == itemId
                        && "OP236".equals(
                        SystemProperties.get("persist.vendor.operator.optr", "OM"))) {
                    String value = intent.getStringExtra(ImsConfigContract.EXTRA_NEW_VALUE);
                    setVdpProvision(phoneId, Integer.parseInt(value));
                }
            } else if (ImsConfigContract.ACTION_CONFIG_LOADED.equals(intent.getAction())) {
                if ("OP236".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"))) {
                    int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                    if (phoneId < 0) return;
                    IMtkImsConfig imsConfig = getEx(phoneId);
                    try {
                        int value = imsConfig.getProvisionedValue(
                                MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE);
                        setVdpProvision(phoneId, value);
                    } catch (RemoteException e) {
                        Log.e(LOG_TAG, "getProvisionedValue fail: " + e);
                    }
                }
            }
        }
    };

    public ImsConfigManager(Context context, ImsCommandsInterface [] imsRILAdapters) {
        if (DEBUG) Rlog.d(LOG_TAG, "ImsConfigManager Enter");

        mContext = context;
        mImsRILAdapters = imsRILAdapters;

        if (!ImsCommonUtil.supportMdAutoSetupIms()) {
            // Dynamic IMS SWITCH is moved to TelephonyWare on Gen93
            final IntentFilter filter = new IntentFilter();
            filter.addAction(ImsConfigContract.ACTION_DYNAMIC_IMS_SWITCH_TRIGGER);
            context.registerReceiver(mBroadcastReceiver, filter);
        } else if ("OP236".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"))) {
            final IntentFilter filter = new IntentFilter();
            filter.addAction(ImsConfigContract.ACTION_IMS_CONFIG_CHANGED);
            filter.addAction(ImsConfigContract.ACTION_CONFIG_LOADED);
            context.registerReceiver(mBroadcastReceiver, filter);
        }
    }

    /**
     * To initial ImsConfigImpl(AOSP) Implementation
     *
     * @return void.
     */
    public void init(int phoneId) {
        ImsConfigAdapter configAdapter;
        ImsConfigImpl imsConfigImpl = null;

        configAdapter = getImsConfigAdapter(mContext, mImsRILAdapters, phoneId);

        if (DEBUG) Rlog.d(LOG_TAG, "init ImsConfigImpl phoneId:" + phoneId);
        synchronized (mImsConfigInstanceMap) {
            imsConfigImpl = new ImsConfigImpl(mContext, mImsRILAdapters[phoneId], configAdapter,
                    phoneId);
            mImsConfigInstanceMap.put(phoneId, imsConfigImpl);
        }
    }

    /**
     * To get and initial ImsConfigImpl(AOSP) Implementation,
     * and API will return IMtkImsConfig
     *
     * @return IImsConfig, AOSP defined interface.
     */
    public IImsConfig get(int phoneId) {
        ImsConfigAdapter configAdapter;
        ImsConfigImpl imsConfigImpl = null;
        IImsConfig instance = null;

        configAdapter = getImsConfigAdapter(mContext, mImsRILAdapters, phoneId);

        synchronized (mImsConfigInstanceMap) {
            if (mImsConfigInstanceMap.containsKey(phoneId)) {
                instance = mImsConfigInstanceMap.get(phoneId).getIImsConfig();
            } else {
                imsConfigImpl = new ImsConfigImpl(mContext, mImsRILAdapters[phoneId], configAdapter, phoneId);
                instance = imsConfigImpl.getIImsConfig();
                mImsConfigInstanceMap.put(phoneId, imsConfigImpl);
            }
        }
        return instance;
    }

    /**
     * To initial ImsConfigImpl(AOSP) and MtkImsConfigImpl(Add-On) Implementation
     *
     * @return void.
     */
    public void initEx(int phoneId) {
        ImsConfigAdapter configAdapter;
        ImsConfigImpl imsConfigImpl = null;
        IImsConfig instance = null;
        MtkImsConfigImpl instanceEx;

        configAdapter = getImsConfigAdapter(mContext, mImsRILAdapters, phoneId);

        if (DEBUG) Rlog.d(LOG_TAG, "initEx ImsConfigImpl phoneId:" + phoneId);
        synchronized (mImsConfigInstanceMap) {
            imsConfigImpl = new ImsConfigImpl(mContext, mImsRILAdapters[phoneId], configAdapter,
                    phoneId);
            instance = imsConfigImpl.getIImsConfig();
            mImsConfigInstanceMap.put(phoneId, imsConfigImpl);
        }

        if (DEBUG)
            Rlog.d(LOG_TAG, "initEx MtkImsConfigImpl phoneId:" + phoneId);
        synchronized (mMtkImsConfigInstanceMap) {
            instanceEx = new MtkImsConfigImpl(mContext, mImsRILAdapters[phoneId], instance,
                    configAdapter, phoneId);
            mMtkImsConfigInstanceMap.put(phoneId, instanceEx);
        }
    }

    /**
     * To get and initial ImsConfigImpl(AOSP) and MtkImsConfigImpl(Add-On) Implementation,
     * and API will return IMtkImsConfig
     *
     * @return IMtkImsConfig, MTK defined interface.
     */
    public IMtkImsConfig getEx(int phoneId) {
        ImsConfigAdapter configAdapter;
        ImsConfigImpl imsConfigImpl = null;
        IImsConfig instance = null;
        IMtkImsConfig instanceEx = null;

        configAdapter = getImsConfigAdapter(mContext, mImsRILAdapters, phoneId);

        synchronized (mImsConfigInstanceMap) {
            if (mImsConfigInstanceMap.containsKey(phoneId)) {
                instance = mImsConfigInstanceMap.get(phoneId).getIImsConfig();
            } else {
                imsConfigImpl = new ImsConfigImpl(mContext, mImsRILAdapters[phoneId],
                        configAdapter, phoneId);
                instance = imsConfigImpl.getIImsConfig();
                mImsConfigInstanceMap.put(phoneId, imsConfigImpl);
            }
        }

        synchronized (mMtkImsConfigInstanceMap) {
            if (mMtkImsConfigInstanceMap.containsKey(phoneId)) {
                instanceEx = mMtkImsConfigInstanceMap.get(phoneId);
            } else {
                instanceEx = new MtkImsConfigImpl(mContext, mImsRILAdapters[phoneId],
                        instance, configAdapter, phoneId);
                mMtkImsConfigInstanceMap.put(phoneId, instanceEx);
            }
        }
        return instanceEx;
    }

    /**
     * To get ImsConfigAdapter from adapter map, only create adapter for each phone once.
     *
     * @return ImsConfigAdapter.
     */
    private ImsConfigAdapter getImsConfigAdapter(Context context,
            ImsCommandsInterface[] imsRilAdapters, int phoneId) {
        ImsConfigAdapter configAdapter;

        synchronized (mImsConfigAdapterMap) {
            if (mImsConfigAdapterMap.containsKey(phoneId)) {
                configAdapter = mImsConfigAdapterMap.get(phoneId);
            } else {
                if (DEBUG) Rlog.d(LOG_TAG, "init ImsConfigAdapter phone:" + phoneId);
                configAdapter = new ImsConfigAdapter(context, imsRilAdapters[phoneId], phoneId);
                mImsConfigAdapterMap.put(phoneId, configAdapter);
            }
        }
        return configAdapter;
    }

    /// M: Dynamic IMS Switch @{
    private void updateImsResrouceCapability(Context context, Intent intent) {
        int mcc = 0;
        int mnc = 0;
        IMtkImsConfig imsConfig = null;
        String mccMnc = null;

        String simState = intent.getStringExtra((IccCardConstants.INTENT_KEY_ICC_STATE));
        int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);

        if (!"1".equals(SystemProperties.get("persist.vendor.mtk_dynamic_ims_switch"))) {
            if (simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_ABSENT) ||
                    simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                Rlog.d(LOG_TAG, "updateImsServiceConfig after SIM event, phoneId:" + phoneId);
                // Force update IMS feature values after SIM event.
                updateImsServiceConfig(context, phoneId);
            }
            return;
        }

        Rlog.d(LOG_TAG, "get MtkImsConfigImpl of phone " + phoneId);
        imsConfig = getEx(phoneId);

        try {
            int volteResVal;
            int vilteResVal;
            int wfcResVal;

            if (simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                Rlog.w(LOG_TAG, "setImsResCapability to volte only w/o SIM on phone " + phoneId);
                // Back to volte only w/o SIM card.
                volteResVal = ImsConfig.FeatureValueConstants.ON;
                vilteResVal = ImsConfig.FeatureValueConstants.OFF;
                wfcResVal = ImsConfig.FeatureValueConstants.OFF;

                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE, volteResVal);
                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, vilteResVal);
                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI, wfcResVal);

                // Force update IMS feature values after SIM absent event.
                // Cover by force update after receive CARRIER_CONFIG_CHANGED after sim ABSENT
                // ImsManager.updateImsServiceConfig(context, phoneId, true);
            } else if (simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                if (!isTestSim(phoneId)) {
                    TelephonyManager tm = (TelephonyManager) context
                            .getSystemService(Context.TELEPHONY_SERVICE);

                    mccMnc = OperatorUtils.getSimOperatorNumericForPhone(phoneId);
                    if (mccMnc == null || mccMnc.isEmpty()) {
                        Rlog.w(LOG_TAG, "Invalid mccMnc:" + mccMnc);
                        return;
                    }

                    try {
                        mcc = Integer.parseInt(mccMnc.substring(0, 3));
                        mnc = Integer.parseInt(mccMnc.substring(3));
                    } catch (NumberFormatException e) {
                        Rlog.w(LOG_TAG, "Invalid mccMnc:" + mccMnc);
                        return;
                    }

                    Rlog.d(LOG_TAG, "SIM loaded on phone " + phoneId + " with mcc: " +
                            mcc + " mnc: " + mnc);

                    int subId = SubscriptionManagerHelper.getSubIdUsingPhoneId(phoneId);
                    String iccid = tm.getSimSerialNumber(subId);
                    if (!SENLOG || TELDBG) {
                        Rlog.d(LOG_TAG, "check iccid:"+ Rlog.pii(LOG_TAG, iccid));
                    }
                    // replace mcc mnc id to 46605 for APTG roaming case
                    if (!TextUtils.isEmpty(iccid)) {
                        if (iccid.startsWith("8988605")) {
                            if (DEBUG) Rlog.d(LOG_TAG, "Replace mccmnc for APTG roaming case");
                            mcc = 466;
                            mnc = 5;
                        }
                        /// M: ALPS03514610 replace mcc mnc id to 46003 for CT roaming case @{
                        else if (iccid.startsWith("898603") || iccid.startsWith("898611")) {
                            mcc = 460;
                            mnc = 3;
                            if (DEBUG) Rlog.d(LOG_TAG, "Replace mccmnc for CT roaming case");
                        }
                        /// @}
                    }

                    // Retrive resource with specific mcc mnc value.
                    Configuration newConfiguration;
                    Resources res = context.getResources();
                    newConfiguration = res.getConfiguration();
                    newConfiguration.mcc = mcc;
                    newConfiguration.mnc = mnc == 0 ? Configuration.MNC_ZERO : mnc;
                    res.updateConfiguration(newConfiguration, null);

                    if (OperatorUtils.isCTVolteDisabled(phoneId)) {
                        volteResVal = ImsConfig.FeatureValueConstants.OFF;
                        vilteResVal = ImsConfig.FeatureValueConstants.OFF;
                        wfcResVal = ImsConfig.FeatureValueConstants.OFF;
                    } else {
                        volteResVal = mapFeatureValue(res.getBoolean(
                                com.android.internal.R.bool.config_device_volte_available));
                        vilteResVal = mapFeatureValue(res.getBoolean(
                                com.android.internal.R.bool.config_device_vt_available));
                        wfcResVal = mapFeatureValue(res.getBoolean(
                                com.android.internal.R.bool.config_device_wfc_ims_available));
                    }
                } else {
                    // For test SIM cards, forece enable all IMS functions for lab event.
                    Rlog.w(LOG_TAG, "Found test SIM on phone " + phoneId);
                    volteResVal = ImsConfig.FeatureValueConstants.ON;
                    vilteResVal = ImsConfig.FeatureValueConstants.ON;
                    wfcResVal = ImsConfig.FeatureValueConstants.ON;
                }

                Rlog.d(LOG_TAG, "Set res capability: volte = " + volteResVal +
                        ", vilte = " + vilteResVal + ", wfc = " + wfcResVal);

                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE, volteResVal);
                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, vilteResVal);
                imsConfig.setImsResCapability(
                        ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI, wfcResVal);

                // Force update IMS feature values after SIM event.
                // Cover by force update after receive CARRIER_CONFIG_CHANGED after sim LOADED
                // updateImsServiceConfig(context, phoneId);
            }

            // Use to notify App to check platform support status again
            Intent mIntent = new Intent(ImsConfigContract.ACTION_DYNAMIC_IMS_SWITCH_COMPLETE);
            mIntent.putExtra(PhoneConstants.PHONE_KEY, phoneId);
            mIntent.putExtra(IccCardConstants.INTENT_KEY_ICC_STATE, simState);
            context.sendBroadcast(mIntent, android.Manifest.permission.READ_PHONE_STATE);
            Rlog.d(LOG_TAG, "DYNAMIC_IMS_SWITCH_COMPLETE phoneId:" + phoneId +
                        ", simState:" + simState);

        } catch (RemoteException e) {
            Rlog.e(LOG_TAG, "SetImsCapability fail: " + e);
        }
    }

    private static int mapFeatureValue(boolean value) {
        return (value == true) ? ImsConfig.FeatureValueConstants.ON :
            ImsConfig.FeatureValueConstants.OFF;
    }

    private static boolean isTestSim(int phoneId) {
        boolean isTestSim = false;
        switch (phoneId) {
            case PhoneConstants.SIM_ID_1:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim", "0"));
                break;
            case PhoneConstants.SIM_ID_2:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.2", "0"));
                break;
            case PhoneConstants.SIM_ID_3:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.3", "0"));
                break;
            case PhoneConstants.SIM_ID_4:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.4", "0"));
                break;
        }
        return isTestSim;
    }
    /// @}

    /**
     * to get main capability phone id.
     *
     * @return The phone id with highest capability.
     */
    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }
        return phoneId;
    }

    private void updateImsServiceConfig(Context context, int phoneId) {
        if (mImsManagerOemPlugin == null) {
            mImsManagerOemPlugin = ExtensionFactory.makeOemPluginFactory(context)
                    .makeImsManagerPlugin(context);
        }

        if (ImsCommonUtil.supportMims()) {
            // update for each phoneId
            mImsManagerOemPlugin.updateImsServiceConfig(context, phoneId, true);
        } else {
            // Non MIMS: update main capability phoneId only
            if (phoneId == getMainCapabilityPhoneId()) {
                mImsManagerOemPlugin.updateImsServiceConfig(context, phoneId, true);
            } else {
                if (DEBUG) Rlog.d(LOG_TAG, "Do not update if phoneId is not main capability");
            }
        }
    }

    private void setVdpProvision(int phoneId, int value) {
        Log.d(LOG_TAG, "phoneId:" + phoneId + " ,value:" + value);
        if (phoneId < 0) return;
        IMtkImsConfig imsConfig = getEx(phoneId);
        String[] keys = new String[1];
        String[] values = new String[1];
        keys[0] = "provision_setting_vdp";
        values[0] = Integer.toString(value);
        try {
            imsConfig.setModemImsCfg(keys, values, phoneId);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "setModemImsCfg fail: " + e);
        }
    }
}
