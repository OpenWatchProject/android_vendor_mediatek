/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;

import com.mediatek.ims.rcsua.service.IAcsEventCallback;
import com.mediatek.ims.rcsua.service.IImsEventCallback;
import com.mediatek.ims.rcsua.service.IRcsUaClient;
import com.mediatek.ims.rcsua.service.IRcsUaService;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;
import java.util.HashSet;

/**
 * API class to using RCS UA service
 */
public class RcsUaService {

    public static final String ACTION_RCSUA_SERVICE_UP =
            "com.mediatek.ims.rcsua.SERVICE_UP";
    public static final String ACTION_RCSUA_SERVICE_DOWN =
            "com.mediatek.ims.rcsua.SERVICE_DOWN";

    /**
     * Option of ROI(RCS over Internet) support.
     * Boolean value: true for support, false for not support
     */
    public static final String OPTION_ROI_SUPPORT =
            "OPTION_ROI_SUPPORT";

    /**
     * Option of IMS deregistration suspend/resume.
     * Integer value: 0 for not support positive value for maxium suspend timeout
     * See also {@link com.mediatek.ims.rcsua.Client#resumeImsDeregistration}
     */
    public static final String OPTION_DEREG_SUSPEND =
            "OPTION_DEREG_SUSPEND";

    /**
     * IMS registration state.
     * See also: {@link RcsUaService#REG_STATE_NOT_REGISTERED},
     *           {@link RcsUaService#REG_STATE_REGISTERING},
     *           {@link RcsUaService#REG_STATE_REGISTERED},
     *           {@link RcsUaService#REG_STATE_DEREGISTERING}
     */
    @IntDef({
            REG_STATE_NOT_REGISTERED,
            REG_STATE_REGISTERING,
            REG_STATE_REGISTERED,
            REG_STATE_DEREGISTERING
            })
    @Retention(RetentionPolicy.SOURCE)
    public @interface RegistrationState {}

    /**
     * IMS registration not established.
     */
    public static final int REG_STATE_NOT_REGISTERED = 0;

    /**
     * IMS registration in progress.
     */
    public static final int REG_STATE_REGISTERING = 1;
    /**
     * IMS registration established.
     */
    public static final int REG_STATE_REGISTERED = 2;
    /**
     * IMS deregistration in progress.
     */
    public static final int REG_STATE_DEREGISTERING = 3;

    /**
     * Radio technology used for IMS registration.
     * See also: {@link RcsUaService#REG_RADIO_NONE},
     *           {@link RcsUaService#REG_RADIO_3GPP},
     *           {@link RcsUaService#REG_RADIO_IEEE802}
     */
    @IntDef({
            REG_RADIO_NONE,
            REG_RADIO_3GPP,
            REG_RADIO_IEEE802
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ImsRadioTech {}

    /**
     * Initial state for radio technology
     */
    public static final int REG_RADIO_NONE = 0;
    /**
     * 3GPP radio technology, i.e. LTE/UMTS
     */
    public static final int REG_RADIO_3GPP = 1;
    /**
     * IEEE 802 technology, i.e. IWLAN
     */
    public static final int REG_RADIO_IEEE802 = 2;

    /**
     * IMS registration mode.
     * See also: {@link RcsUaService#REG_MODE_IMS},
     *           {@link RcsUaService#REG_MODE_INTERNET}
     */
    @IntDef({
            REG_MODE_IMS,
            REG_MODE_INTERNET
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface RegistrationMode {}

    /**
     * IMS registration through IMS connection.
     */
    public static final int REG_MODE_IMS = 1;

    /**
     * IMS registration through internet connection.
     * Only available when {@link RcsUaService#OPTION_ROI_SUPPORT} = true
     */
    public static final int REG_MODE_INTERNET = 2;

    /**
     * Reason of trigger ACS request.
     * See also: {@link RcsUaService#REASON_DEFAULT_SMS_UNKNOWN},
     *           {@link RcsUaService#REASON_RCS_DEFAULT_SMS},
     *           {@link RcsUaService#REASON_RCS_NOT_DEFAULT_SMS},
     *           {@link RcsUaService#REASON_FT_AUTHENTICATE_FAILURE}
     */
    @IntDef({
            REASON_DEFAULT_SMS_UNKNOWN,
            REASON_RCS_DEFAULT_SMS,
            REASON_RCS_NOT_DEFAULT_SMS,
            REASON_FT_AUTHENTICATE_FAILURE
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AcsRequestReason {}

    /**
     * Default SMS App changing not allowed.
     */
    public static final int REASON_DEFAULT_SMS_UNKNOWN = 0;

    /**
     * RCS client be selected as default SMS App.
     */
    public static final int REASON_RCS_DEFAULT_SMS = 1;

    /**
     * RCS client not be selected as default SMS App.
     */
    public static final int REASON_RCS_NOT_DEFAULT_SMS = 2;

    /**
     * request new configuration upon receiving a 403 authentication failure
     * for HTTP File Transfer upload and download.
     */
    public static final int REASON_FT_AUTHENTICATE_FAILURE = 3;

    /**
     * Callback indicate RCS UA service connection status.
     */
    public interface Callback {
        /**
         * Called when RCS UA service connected.
         *
         * @param service the connected service.
         */
        void serviceConnected(RcsUaService service);

        /**
         * Called when RCS UA service link broken.
         *
         * @param service the link broken service.
         */
        void serviceDisconnected(RcsUaService service);
    }

    /**
     * start RCS UA service. Should be called before any operation can be performed.
     *
     * @param context caller context, it is doesn't matter context of application or component
     * @param callback callback registered to receive service status notification
     * @return {@see RcsUaService}
     */
    public static RcsUaService startService(final Context context, RcsUaService.Callback callback) {
        return startService(context, 0, callback, null);
    }

    /**
     * start RCS UA service. Should be called before any operation can be performed.
     *
     * @param context caller context, it is doesn't matter context of application or component
     * @param phoneId reserved for MSIM support
     * @param callback callback registered to receive service status notification
     * @return RcsUaService instance
     */
    public static RcsUaService startService(
            final Context context, int phoneId, RcsUaService.Callback callback) {
        return startService(context, phoneId, callback, null);
    }

    /**
     * start RCS UA service. Should be called before any operation can be performed.
     *
     * @param context caller context, it is doesn't matter context of application or component
     * @param callback callback registered to receive service status notification
     * @param options service options
     * @return {@see RcsUaService}
     */
    public static RcsUaService startService(
            final Context context, RcsUaService.Callback callback, Bundle options) {
        return startService(context, 0, callback, options);
    }

    /**
     * start RCS UA service. Should be called before any operation can be performed.
     *
     * @param context caller context, it is doesn't matter context of application or component
     * @param phoneId reserved for MSIM support
     * @param callback callback registered to receive service status notification
     * @param options service options
     * @return {@see RcsUaService}
     */
    public static RcsUaService startService(
            final Context context, int phoneId, RcsUaService.Callback callback, Bundle options) {
        Log.d(TAG, "startService, current instance:" + INSTANCE);
        if (INSTANCE == null) {
            synchronized (RcsUaService.class) {
                if (INSTANCE == null) {
                    INSTANCE = new RcsUaService(context, phoneId, callback, options);
                }
            }
        }

        return INSTANCE;
    }

    /**
     * stop RCS UA service.
     * After this, all operation can't be performed before startService again.
     */
    public void stopService() {
        Log.d(TAG, "stopService, current instance:" + INSTANCE);
        synchronized (RcsUaService.class) {
            if (serviceIntf != null) {
                if (acsEventCallback != null) {
                    try {
                        serviceIntf.unregisterAcsCallback(acsEventCallback);
                    } catch (RemoteException e) {

                    }
                    acsEventCallback = null;
                }
                acsCallbacks.clear();
                clients.clear();

                callerContext.unbindService(serviceConnection);
                serviceIntf.asBinder().unlinkToDeath(deathRecipient, 0);
                serviceIntf = null;
            }
            if (INSTANCE != null) {
                INSTANCE = null;
            }
        }
    }

    /**
     * Retrieve RcsUaService instance.
     *
     * @return RcsUaService instance, null if startService never called.
     */
    public static RcsUaService getInstance() {
        return getInstance(0);
    }

    /**
     * Reserved for MSIM support.
     *
     * @param phoneId reserved for MSIM support.
     * @return RcsUaService instance, null if startService never called.
     */
    public static RcsUaService getInstance(int phoneId) {
        if (INSTANCE == null) {
            throw new NullPointerException("startService() must be called before getInstance()");
        }

        return INSTANCE;
    }

    /**
     * activate RCS service. all RCS specific features will be carried in IMS Register.
     * the state will be saved even after power cycle.
     */
    public void activate() {
        Log.d(TAG, "activate");
        if (!isConnected()) {
            activated = true;
        } else {
            try {
                serviceIntf.activate();
            } catch (RemoteException e) {
            }
        }
    }

    /**
     * deactivate RCS service. all RCS specific features will be removed from IMS Register.
     * the state will be saved even after power cycle.
     * all relevant IMS event callback wil no longer been called anymore.
     */
    public void deactivate() {
        Log.d(TAG, "deactivate");
        if (!isConnected()) {
            activated = false;
        } else {
            try {
                serviceIntf.deactivate();
            } catch (RemoteException e) {
            }
        }
    }

    /**
     * Forced trigger IMS registration.
     * only worked when IMS already registered.
     */
    public void triggerReregistration() {
        Log.d(TAG, "triggerReregistraion");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.triggerReregistration(null);
        } catch (RemoteException e) {

        }
    }


    /**
     * Forced trigger IMS registration with specific features.
     * only worked when IMS already registered.
     */
    public void triggerReregistration(Capability features) {
        Log.d(TAG, "triggerReregistraion");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.triggerReregistration(features);
        } catch (RemoteException e) {

        }
    }

    /**
     * Forced trigger IMS restoration.
     * only worked when IMS already registered.
     */
    public void triggerRestoration() {
        Log.d(TAG, "triggerRestoration");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.triggerRestoration();
        } catch (RemoteException e) {

        }
    }

    /**
     * Update RCS features by add one or more feature.
     * this might trigger IMS reregistration.
     *
     * @param feature feature need to be add.
     */
    public void addCapability(@NonNull Capability feature) {
        Log.d(TAG, "addCapability");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.addCapability(feature);
        } catch (RemoteException e) {

        }
    }


    /**
     * Update RCS features by remove one or more feature.
     * this might trigger IMS reregistration.
     *
     * @param feature feature need to be removed.
     */
    public void removeCapability(@NonNull Capability feature) {
        Log.d(TAG, "removeCapability");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.removeCapability(feature);
        } catch (RemoteException e) {

        }
    }


    /**
     * Update RCS features.
     * this might trigger IMS reregistration.
     *
     * @param features feature need to be updated.
     */
    public void updateCapabilities(@Nullable Capability features) {
        Log.d(TAG, "updateCapabilities");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.updateCapabilities(features);
        } catch (RemoteException e) {

        }
    }


    /**
     * Update RCS features.
     * this might trigger IMS reregistration.
     *
     * @param featuresText feature need to be updated. refer RFC3840 for feature text format.
     */
    public void updateCapabilities(@Nullable String featuresText) {
        Log.d(TAG, "updateCapabilities");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.updateCapabilities(new Capability(featuresText));
        } catch (RemoteException e) {

        }
    }


    /**
     * Register UA client to receive IMS event notification.
     *
     * @param callback used for get notified for IMS event notification.
     * @return UA client instance
     */
    public Client registerClient(@NonNull ImsEventCallback callback) {
        Client newClient = null;

        if (serviceIntf == null) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            Log.d(TAG, "registerClient:" + clientIntf);

            synchronized (lock) {
                if (clientCount++ == 0) {
                    clientIntf = serviceIntf.registerClient(imsEventCallback);
                }

                newClient = new Client(this, clientIntf);
                newClient.registerImsEventCallback(callback);
                clients.add(newClient);
            }
        } catch (RemoteException e) {

        }

        return newClient;
    }


    /**
     * Unregister client previously registered.
     * Corresponding IMS event callback will be no longer called after unregister done.
     *
     * @param client previously registered to.
     */
    public void unregisterClient(Client client) {
        Log.d(TAG, "unregisterClient");
        if (!clients.contains(client))
            throw new IllegalArgumentException("client never registered.");

        synchronized (lock) {
            if (--clientCount == 0) {
                try {
                    if (serviceIntf != null)
                        serviceIntf.unregisterClient(clientIntf);
                } catch (RemoteException e) {

                }
                clientIntf = null;
            }
            clients.remove(client);
        }
    }


    /**
     * Retrieve RCS features current registered to IMS service.
     *
     * @return RCS features current registered.
     */
    public Capability getCapabilities() {
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        Capability capability = null;
        try {
            capability = serviceIntf.getCapabilities();
        } catch (RemoteException e) {

        }

        return capability;
    }


    /**
     * Check whether or not RCS service activated.
     *
     * @return true for activated false for deactivated.
     */
    public boolean isActivated() {
        if (!isConnected()) {
            return activated;
        }

        boolean activated = true;
        try {
            activated = serviceIntf.isActivated();
        } catch (RemoteException e) {

        }

        return activated;
    }

    /**
     * Set service options.
     *
     * @param options to be applied.
     */
    public void setOptions(Bundle options) {
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.setOptions(options);
        } catch (RemoteException e) {

        }
    }


    /**
     * Retrieve all client registered.
     *
     * @return array for all clients currently registered.
     */
    public Client[] getActiveClients() {
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        return (Client[])clients.toArray();
    }

    /**
     * Retrieve current service options.
     *
     * @return options currently taken effect.
     */
    public Bundle getOptions() {
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        Bundle options = null;
        try {
            options = serviceIntf.getOptions();
        } catch (RemoteException e) {
        }

        return options;
    }

    /**
     * Register callback to receive ACS related event
     *
     * @param callback used to get ACS event notification
     */
    public void registerAcsEventCallback(@NonNull AcsEventCallback callback) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        synchronized (lock) {
            if (acsCallbacks.isEmpty()) {
                acsEventCallback = new IAcsEventCallback.Stub() {
                    @Override
                    public void onConfigChanged(boolean valid, int version) {
                        Log.d(TAG, "onConfigChanged:valid:" + valid + ",version:" + version);
                        synchronized (lock) {
                            for (AcsEventCallback callback : acsCallbacks) {
                                callback.run(callback.new Runner(0, valid ? 1 : 0, version));
                            }
                        }
                    }

                    @Override
                    public void onConnectionChanged(boolean status) {
                        Log.d(TAG, "onConnectionChanged:status:" + status);
                        synchronized (lock) {
                            for (AcsEventCallback callback : acsCallbacks) {
                                if (status)
                                    callback.run(callback.new Runner(1, 0, 0));
                                else
                                    callback.run(callback.new Runner(2, 0, 0));
                            }
                        }
                    }
                };
                try {
                    serviceIntf.registerAcsCallback(acsEventCallback);
                } catch (RemoteException e) {
                }
            }
            acsCallbacks.add(callback);
        }
    }

    /**
     * Unregister ACS event callback previously registered. Callback will never be
     * called after unregister done.
     *
     * @param callback to be unregisted callback
     */
    public void unregisterAcsEventCallback(@NonNull AcsEventCallback callback) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        synchronized (lock) {
            acsCallbacks.remove(callback);
            if (acsCallbacks.isEmpty()) {
                try {
                    if (serviceIntf != null)
                        serviceIntf.unregisterAcsCallback(acsEventCallback);
                    acsEventCallback = null;
                } catch (RemoteException e) {
                }
            }
        }
    }

    /**
     * Retrieve current ACS configuration.
     *
     * @return ACS configuration data. null if ACS configuration not being valid at the time.
     */
    public AcsConfiguration getAcsConfiguration() {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        AcsConfiguration config = null;
        try {
            config = serviceIntf.getAcsConfiguration();
        } catch (RemoteException e) {
        }
        return config;
    }

    /**
     * Retrive specified numberic configuration item by config key.
     *
     * @param configItem config key used to retrieve specifiied item value.
     * @return Integer config value.
     */
    public int getAcsConfigInt(String configItem) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        int value = 0;
        try {
            value = serviceIntf.getAcsConfigInt(configItem);
        } catch (RemoteException e) {
        }

        return value;
    }

    /**
     * Retrieve specified text configuration item by config key.
     *
     * @param configItem config key used to retrieve specified item value.
     * @return String config value.
     */
    public String getAcsConfigString(String configItem) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        String value = "";
        try {
            value = serviceIntf.getAcsConfigString(configItem);
        } catch (RemoteException e) {
        }

        return value;
    }

    /**
     * Used to test ACS connection status.
     *
     * @return true for ACS connection exist, false for otherwise.
     */
    public boolean isAcsConnected() {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        boolean connected = false;
        try {
            connected = serviceIntf.isAcsConnected();
        } catch (RemoteException e) {
        }

        return connected;
    }

    /**
     * Voluntarily trigger ACS request configuration to server.
     *
     * @param reason of trigger ACS request.
     *               See also: {@link RcsUaService.AcsRequestReason}
     */
    public void triggerAcsRequest(@AcsRequestReason int reason) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");
        if (!isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            serviceIntf.triggerAcsRequest(reason);
        } catch (RemoteException e) {
        }
    }

    /**
     * Set ACS support status.
     *
     * @param state
     * true : means switch on ACS.
     * false: means switch off ACS.
     *
     * @return Set ACS switch state result.
     * true : success to change ACS switch state.
     * false: fail to change ACS switch state.
     */
    public boolean setAcsSwitchState(boolean state) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");

        boolean result = false;
        try {
            result = serviceIntf.setAcsSwitchState(state);
        } catch (RemoteException e) {
        }
        return result;
    }

    /**
     * Get ACS support status.
     *
     * @return ACS switch state.
     * true : means ACS switchs on.
     * false: means ACS switchs off.
     *
     */
    public boolean getAcsSwitchState() {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");

        boolean state = false;
        try {
             state = serviceIntf.getAcsSwitchState();
        } catch (RemoteException e) {
        }
        return state;
    }

    /**
     * Set ACS provisioning address.
     *
     * @param address ACS provisioing address.
     *
     * @return Set ACS provisioning address result.
     * true : set provisioning address success.
     * false: set provisioning address fail.
     *
     */
    public boolean setAcsProvisioningAddress(String address) {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");

        boolean result = false;
        try {
            result = serviceIntf.setAcsProvisioningAddress(address);
        } catch(RemoteException e) {
        }
        return result;
    }

    /**
     * Clear ACS configuration.
     *
     * @return Clear ACS configuration result.
     * true : success to clear ACS configuration.
     * false: fail to clear ACS configuration.
     */
    public boolean clearAcsConfiguration() {
        if (!acsSupported)
            throw new UnsupportedOperationException("ACS not supported");

        boolean result = false;
        try {
            result = serviceIntf.clearAcsConfiguration();
        } catch(RemoteException e) {
        }
        return result;
    }

    /**
     * To check whether RCS UA service is available in platform.
     * No any service API available to be used if return false.
     *
     * @param context caller context used.
     * @return availability for RCS UA service.
     */
    public static boolean isAvailable(@NonNull Context context) {
        final String name = "persist.vendor.mtk_rcs_ua_support";

        String value = getSystemProperties(context, name, "0");
        Log.d(TAG, "isAvailable:" + value);

        return (Integer.parseInt(value) == 1);
    }

    public static boolean isAcsAvailable(@NonNull Context context) {
        PackageInfo info = null;
        PackageManager pm = context.getPackageManager();
        try {
            info = pm.getPackageInfo("com.mediatek.rcs.provisioning", 0);
        } catch (PackageManager.NameNotFoundException e) {
        }

        Log.d(TAG, "isAcsAvailable:" + (info != null));

        return (info != null);
    }

    /**
     * To check whether RCS UA service is connected.
     *
     * @return true after serviceConnected callback invoked.
     */
    public boolean isConnected() {
        return !(serviceIntf == null);
    }

    Context getContext() {
        return callerContext;
    }

    private RcsUaService(
            final Context context, int phoneId, RcsUaService.Callback callback, Bundle options) {
        serviceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                Log.d(TAG, "onServiceConnected:" + service);
                synchronized (RcsUaService.class) {
                    if (INSTANCE == null)
                        return;
                }

                try {
                    if (service != null) {
                        service.linkToDeath(deathRecipient, 0);
                        serviceIntf = IRcsUaService.Stub.asInterface(service);
                        if (activated) {
                            serviceIntf.activate();
                        } else {
                            serviceIntf.deactivate();
                        }
                        notifyServiceUp();
                    }
                } catch (RemoteException e) {
                }
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                Log.d(TAG, "onServiceDisconnected:" + name);
                if (serviceIntf != null) {
                    serviceIntf.asBinder().unlinkToDeath(deathRecipient, 0);
                    serviceIntf = null;
                    notifyServiceDown();
                }
            }
        };

        callerContext = context.getApplicationContext();
        callerCallback = callback;

        activated = Integer.valueOf(getSystemProperties(
                callerContext, "persist.vendor.service.rcs", "1")) == 1;
        acsSupported = isAcsAvailable(callerContext);

        Intent intent = new Intent("com.mediatek.ims.rcsua.BIND_SERVICE");

        intent.putExtra(EXTRA_PHONE_ID, phoneId);

        Bundle bundle = options == null ? new Bundle() : new Bundle(options);
        bundle.putBoolean("OPTION_ACS_SUPPORT", acsSupported);
        intent.putExtra(EXTRA_OPTIONS, bundle);

        intent.setPackage("com.mediatek.ims.rcsua.service");

        callerContext.startService(intent);
        callerContext.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private void notifyServiceUp() {
        Log.d(TAG, "notifyServiceUp to callback[" + callerCallback + "]");
        if (callerCallback != null) {
            callerCallback.serviceConnected(INSTANCE);
        }
        /*
        if (callerContext != null) {
            Intent intent = new Intent(ACTION_RCSUA_SERVICE_UP);
            callerContext.sendBroadcast(intent);
        }
        */
    }

    private void notifyServiceDown() {
        if (callerCallback != null) {
            callerCallback.serviceDisconnected(INSTANCE);
        }
        /*
        if (callerContext != null) {
            Intent intent = new Intent(ACTION_RCSUA_SERVICE_DOWN);
            callerContext.sendBroadcast(intent);
        }
        */
    }

    private final IImsEventCallback imsEventCallback = new IImsEventCallback.Stub() {

        @Override
        public void onStatusChanged(RegistrationInfo regInfo) {
            Log.d(TAG,"onStatusChanged:" + regInfo);
            synchronized (lock) {
                for (Client client : clients) {
                    client.handleImsEventCallback(regInfo);
                }
            }
        }

        @Override
        public void onReregistered(RegistrationInfo regInfo) {
            Log.d(TAG,"onReregistered:" + regInfo);
            synchronized (lock) {
                for (Client client : clients) {
                    client.handleImsReregistered(regInfo);
                }
            }
        }

        @Override
        public void onDeregStarted(RegistrationInfo regInfo) {
            Log.d(TAG,"onDeregStarted:" + regInfo);
            synchronized (lock) {
                for (Client client : clients) {
                    client.handleImsDeregInd(regInfo);
                }
            }
        }

        @Override
        public void onVopsIndication(int vops) {
            Log.d(TAG,"onVopsIndication:" + vops);
            synchronized (lock) {
                for (Client client : clients) {
                    client.handleVopsInd(vops);
                }
            }
        }
    };

    private class ServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            serviceIntf = null;
            clients.clear();
            acsCallbacks.clear();
            acsEventCallback = null;
            notifyServiceDown();
        }
    }

    private static String getSystemProperties(Context context, String name, String defaultValue) {
        String value = defaultValue;

        ClassLoader cl = context.getClassLoader();
        try {
            Class systemProperties = cl.loadClass("android.os.SystemProperties");
            Class[] paramTypes = new Class[2];
            paramTypes[0] = String.class;
            paramTypes[1] = String.class;
            Method get = systemProperties.getMethod("get", paramTypes);
            Object[] params = new Object[2];
            params[0] = new String(name);
            params[1] = new String(defaultValue);

            value = (String) get.invoke(systemProperties, params);
        } catch (Exception e) {
            /* Default consider all exceptions as not available */
        }

        return value;
    }


    private static volatile RcsUaService INSTANCE;
    private int clientCount = 0;
    private ServiceConnection serviceConnection;
    private IRcsUaService serviceIntf;
    private IRcsUaClient clientIntf;
    private IAcsEventCallback acsEventCallback;
    private Capability capabilities;
    private ServiceDeathRecipient deathRecipient = new ServiceDeathRecipient();
    private Context callerContext;
    private boolean activated;
    private boolean acsSupported;
    private RcsUaService.Callback callerCallback;
    private HashSet<Client> clients = new HashSet<Client>();
    private HashSet<AcsEventCallback> acsCallbacks = new HashSet<AcsEventCallback>();
    private Object lock = new Object();
    private static final String EXTRA_PHONE_ID = "phone_id";
    private static final String EXTRA_OPTIONS = "service_options";
    private static final String TAG = "[RcsUaService][API]";
}
