/*
 * Copyright (c) 2013 The Android Open Source Project
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

import com.android.ims.ImsConfigListener;

/**
 * Provides APIs to get/set the IMS service feature/capability/parameters.
 * The config items include:
 * 1) Items provisioned by the operator.
 * 2) Items configured by user. Mainly service feature class.
 *
 * {@hide}
 */
interface IMtkImsConfig {
    /**
     * Gets the value for ims service/capabilities parameters from the provisioned
     * value storage. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return value in Integer format.
     */
    int getProvisionedValue(int item);

    /**
     * Gets the value for ims service/capabilities parameters from the provisioned
     * value storage. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return value in String format.
     */
    String getProvisionedStringValue(int item);

    /**
     * Sets the value for IMS service/capabilities parameters by the operator device
     * management entity. It sets the config item value in the provisioned storage
     * from which the master value is derived. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in Integer format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants.
     */
    int setProvisionedValue(int item, int value);

    /**
     * Sets the value for IMS service/capabilities parameters by the operator device
     * management entity. It sets the config item value in the provisioned storage
     * from which the master value is derived.  Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in String format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants.
     */
    int setProvisionedStringValue(int item, String value);

    /**
     * Gets the value of the specified IMS feature item for specified network type.
     * This operation gets the feature config value from the master storage (i.e. final
     * value). Asynchronous non-blocking call.
     *
     * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param listener. feature value returned asynchronously through listener.
     * @return void
     */
    oneway void getFeatureValue(int feature, int network, ImsConfigListener listener);

    /**
     * Sets the value for IMS feature item for specified network type.
     * This operation stores the user setting in setting db from which master db
     * is dervied.
     *
     * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     */
    void setFeatureValue(int feature, int network, int value, ImsConfigListener listener);

    /**
     * Sets the value for IMS feature item for specified network type.
     * This operation stores the user setting in setting db from which master db
     * is dervied.
     *
     * @param feature array. as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network array. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value array. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     */
    void setMultiFeatureValues(in int[] feature, in int[] network, in int[] value,
            ImsConfigListener listener);

    /**
     *
     * Gets the value for ims fature item video quality.
     *
     * @param listener. Video quality value returned asynchronously through listener.
     * @return void
     */
    oneway void getVideoQuality(ImsConfigListener listener);

    /**
     * Sets the value for IMS feature item video quality.
     *
     * @param quality, defines the value of video quality.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
     oneway void setVideoQuality(int quality, ImsConfigListener listener);

    /**
     * Sets the resource value for IMS capabilities
     *
     * @param feature defined in ImsConfig.FeatureConstants.
     * @param feature capability defined in ImsConfig.FeatureValueConstants.
     */
    void setImsResCapability(int feature, int value);

    /**
     * Gets the resource value for a IMS feature.
     * @param feature, defined in ImsConfig.FeatureConstants.
     *
     * @return the capability.
     */
    int getImsResCapability(int feature);

    /**
     * Sync WFC preferred mode to modem.
     * @param WFC preferred mode.
     * @return void.
     */
    void setWfcMode(int mode);

    /**
     * Sync Volte preference mode for voice domain to modem.
     * @param volte preference mode.
     * @return void.
     */
    void setVoltePreference(int mode);

    /**
     * Update multiple IMS configurations to modem.
     *
     * @param keys as multiple IMS config keys.
     * @param values as multiple IMS config values.
     * @param phoneId as which phone to config
     *
     * @return value in int array format (success is 0, fail is -1)
     */
    int[] setModemImsCfg(in String[] keys, in String[] values, int phoneId);

    /**
     * Update multiple IMS WiFi-Offload(WO) configuration to modem.
     *
     * @param keys as multiple IMS WO config keys.
     * @param values as multiple IMS IWLAN config values.
     * @param phoneId as which phone to config
     *
     * @return value in int array format (success is 0, fail is -1)
     */
    int[] setModemImsWoCfg(in String[] keys, in String[] values, int phoneId);

    /**
     * Update multiple IMS IWLAN configurations to modem.
     *
     * @param keys as multiple IMS IWLAN config keys.
     * @param values as multiple IMS IWLAN config values.
     * @param phoneId as which phone to config
     *
     * @return value in int array format (success is 0, fail is -1)
     */
    int[] setModemImsIwlanCfg(in String[] keys, in String[] values, int phoneId);
}
