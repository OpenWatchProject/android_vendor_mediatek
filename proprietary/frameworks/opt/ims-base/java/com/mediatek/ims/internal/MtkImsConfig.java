/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (c) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.ims.internal;

import android.content.Context;
import android.os.RemoteException;
import android.telephony.Rlog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import android.telephony.ims.ImsReasonInfo;

import com.mediatek.ims.internal.IMtkImsConfig;

/**
 * Provides APIs to get/set the IMS service feature/capability/parameters.
 * The config items include:
 * 1) Items provisioned by the operator.
 * 2) Items configured by user. Mainly service feature class.
 *
 * @hide
 */
public class MtkImsConfig {
    private static final String TAG = "MtkImsConfig";
    private boolean DBG = true;
    private final IMtkImsConfig miConfig;
    private Context mContext;

    public static final int MTK_CONFIG_START = 1000;

    /**
    * Defines IMS service/capability parameters.
    * MTK implement config
    */
    public static class ConfigConstants {

        // Define IMS config items
        public static final int CONFIG_START = MTK_CONFIG_START;

        // Define operator provisioned config items
        public static final int PROVISIONED_CONFIG_START = CONFIG_START;

        // Expand the operator config items as needed here.
        /**
         * FQDN address for WFC ePDG.
         * Value is in String format.
         * Operator: Op06
         * Specification: R19.0.1, VoWiFi Provisioning utilising SMS
         */
        public static final int EPDG_ADDRESS = CONFIG_START;

        /**
         * Publish retry timer when receiving error code =
         * 408, 500, 503, and 603
         * Operator: Op12
         */
        public static final int PUBLISH_ERROR_RETRY_TIMER = CONFIG_START + 1;

        // VzW opt-in MDN requirement
        public static final int VOICE_OVER_WIFI_MDN = CONFIG_START + 2;

        /**
         * Voce domain preference.
         * Value is in Integer format.
         * Operator: Op236
         */
        public static final int VOICE_DOMAIN_PREFERENCE = CONFIG_START + 3;

        // Expand the operator config items as needed here, need to change
        // PROVISIONED_CONFIG_END after that.
        public static final int PROVISIONED_CONFIG_END = VOICE_DOMAIN_PREFERENCE;

        // Expand the operator config items as needed here.
    }

    /**
     * Defines IMS feature value.
     */
    public static class WfcModeFeatureValueConstants {
        public static final int WIFI_ONLY = 0;
        public static final int CELLULAR_PREFERRED = 1;
        public static final int WIFI_PREFERRED = 2;
        public static final int CELLULAR_ONLY = 3;
    }

    public MtkImsConfig(IMtkImsConfig iconfig, Context context) {
        miConfig = iconfig;
        mContext = context;
    }

    /**
     * Gets the provisioned value for IMS service/capabilities parameters used by IMS stack.
     * This function should not be called from the mainthread as it could block the
     * mainthread.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return the value in Integer format.
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public int getProvisionedValue(int item) throws ImsException {
        int ret = 0;
        try {
            ret = miConfig.getProvisionedValue(item);
        }  catch (RemoteException e) {
            throw new ImsException("getValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        if (DBG) Rlog.d(TAG, "getProvisionedValue(): item = " + item + ", ret =" + ret);
        return ret;
    }

    /**
     * Gets the provisioned value for IMS service/capabilities parameters used by IMS stack.
     * This function should not be called from the mainthread as it could block the
     * mainthread.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return value in String format.
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public String getProvisionedStringValue(int item) throws ImsException {
        String ret = "Unknown";
        try {
            ret = miConfig.getProvisionedStringValue(item);
        }  catch (RemoteException e) {
            throw new ImsException("getProvisionedStringValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        if (DBG) Rlog.d(TAG, "getProvisionedStringValue(): item = " + item + ", ret =" + ret);
        return ret;
    }

    /**
     * Sets the value for IMS service/capabilities parameters by
     * the operator device management entity.
     * This function should not be called from main thread as it could block
     * mainthread.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in Integer format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public int setProvisionedValue(int item, int value)
            throws ImsException {
        int ret = ImsConfig.OperationStatusConstants.UNKNOWN;
        try {
            ret = miConfig.setProvisionedValue(item, value);
        }  catch (RemoteException e) {
            throw new ImsException("setProvisionedValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return ret;
    }

    /**
     * Sets the value for IMS service/capabilities parameters by
     * the operator device management entity.
     * This function should not be called from main thread as it could block
     * mainthread.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in String format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public int setProvisionedStringValue(int item, String value)
            throws ImsException {
        int ret = ImsConfig.OperationStatusConstants.UNKNOWN;
        try {
            ret = miConfig.setProvisionedStringValue(item, value);
        }  catch (RemoteException e) {
            throw new ImsException("setProvisionedStringValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return ret;
    }

    /**
     * Gets the value for IMS feature item for specified network type.
     *
     * @param feature, defined as in FeatureConstants.
     * @param network, defined as in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param listener, provided to be notified for the feature on/off status.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void getFeatureValue(int feature, int network,
            ImsConfigListener listener) throws ImsException {
        if (DBG) {
            Rlog.d(TAG, "getFeatureValue: feature = " + feature + ", network =" + network +
                    ", listener =" + listener);
        }
        try {
            miConfig.getFeatureValue(feature, network, listener);
        } catch (RemoteException e) {
            throw new ImsException("getFeatureValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     *
     * @param feature, as defined in FeatureConstants.
     * @param network, as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value, as defined in FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setFeatureValue(int feature, int network, int value,
            ImsConfigListener listener) throws ImsException {
        try {
            miConfig.setFeatureValue(feature, network, value, listener);
        } catch (RemoteException e) {
            throw new ImsException("setFeatureValue()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     *
     * @param feature, as defined in FeatureConstants.
     * @param network, as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value, as defined in FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setMultiFeatureValues(int[] feature, int[] network, int[] value,
            ImsConfigListener listener) throws ImsException {
        try {
            Rlog.d(TAG, "setMultiFeatureValues()");

            miConfig.setMultiFeatureValues(feature, network, value, listener);
        } catch (RemoteException e) {
            throw new ImsException("setMultiFeatureValues()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Sets the resource value for IMS capabilities
     *
     * @param feature defined in ImsConfig.FeatureConstants.
     * @param feature capability defined in ImsConfig.FeatureValueConstants.
     */
    public void setImsResCapability(int feature, int value) throws ImsException {
        try {
            miConfig.setImsResCapability(feature, value);
        } catch (RemoteException e) {
            throw new ImsException("setImsResCapability()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Gets the resource value for a IMS feature.
     * @param feature, defined in ImsConfig.FeatureConstants.
     *
     * @return the capability.
     */
    public int getImsResCapability(int feature) throws ImsException {
        int capability = ImsConfig.FeatureValueConstants.OFF;
        try {
            capability = miConfig.getImsResCapability(feature);
        } catch (RemoteException e) {
            throw new ImsException("getImsResCapability()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return capability;
    }

    /**
     * Sync WFC preferred mode to modem.
     * @param WFC preferred mode.
     * @return void.
     */
    public void setWfcMode(int mode) throws ImsException {
        try {
            miConfig.setWfcMode(mode);
        } catch (RemoteException e) {
            throw new ImsException("setWfcMode()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Sync volte preference mode for voice domain to modem.
     * @param volte preference mode.
     * @return void.
     */
    public void setVoltePreference(int mode) throws ImsException {
        try {
            miConfig.setVoltePreference(mode);
        } catch (RemoteException e) {
            throw new ImsException("setVoltePreference()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
    }

    /**
     * Update multiple Ims config to modem.
     * @param keys as multiple Ims config keys.
     * @param values as multiple Ims config values.
     * @param phoneId as which phone to config
     * @return value in int array format (success is 0, fail is -1)
     */
    public int[] setModemImsCfg(String[] keys, String[] values, int phoneId) throws ImsException {
        int[] result = null;
        try {
            result = miConfig.setModemImsCfg(keys, values, phoneId);
        } catch (RemoteException e) {
            throw new ImsException("setModemImsCfg()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return result;
    }

    /**
     * Update multiple Ims WO config to modem.
     * @param keys as multiple Ims WO config keys.
     * @param values as multiple Ims IWLAN config values.
     * @param phoneId as which phone to config
     * @return value in int array format (success is 0, fail is -1)
     */
    public int[] setModemImsWoCfg(String[] keys, String[] values, int phoneId) throws ImsException {
        int[] result = null;
        try {
            result = miConfig.setModemImsWoCfg(keys, values, phoneId);
        } catch (RemoteException e) {
            throw new ImsException("setModemImsWoCfg()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return result;
    }

    /**
     * Update multiple Ims IWLAN config to modem.
     * @param keys as multiple Ims IWLAN config keys.
     * @param values as multiple Ims IWLAN config values.
     * @param phoneId as which phone to config
     * @return value in int array format (success is 0, fail is -1)
     */
    public int[] setModemImsIwlanCfg(String[] keys, String[] values, int phoneId) throws ImsException {
        int[] result = null;
        try {
            result = miConfig.setModemImsIwlanCfg(keys, values, phoneId);
        } catch (RemoteException e) {
            throw new ImsException("setImsModemIwlanCfg()", e,
                    ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        }
        return result;
    }
}
