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

package com.mediatek.ims.rcsua.service;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.rcsua.AcsConfiguration;
import com.mediatek.ims.rcsua.Capability;
import com.mediatek.ims.rcsua.service.utils.Logger;
import com.mediatek.ims.rcsua.service.utils.Utils;
import com.mediatek.rcs.provisioning.AcsConfigInfo;
import com.mediatek.rcs.provisioning.IAcsCallback;
import com.mediatek.rcs.provisioning.IAcsService;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public final class RuaService extends Service {

    @Override
    public void onCreate() {
        adapter = RuaAdapter.createInstance();

        this.activated =
                Integer.valueOf(SystemProperties.get("persist.vendor.service.rcs", "1")) == 1;
        long features;
        /* Use different default feature to compliance OP07 requirement,
           here need to sync with RCS features initialized in RILD
           Only works for internal project, OEM should pre-define corresponding
            property value as specify requirement.
         */
        features = Long.valueOf(SystemProperties.get("persist.vendor.service.tag.rcs", "0"));
        logger.debug("onCreate->activated[" + activated + "],features[" + features + "]");
        capability = new Capability(features);
        registerSimStateReceiver();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        logger.debug("onStartCommand");

        if (intent != null) {
            setOptions(intent.getBundleExtra(EXTRA_OPTIONS));
            int phoneId = intent.getIntExtra(EXTRA_PHONE_ID, 0);
            adapter.sendActiveRcsSlotId(phoneId);
        }

        if (serviceOptions != null && serviceOptions.getBoolean(KEY_ACS_SUPPORT)) {
            startAcsService();
        }

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        logger.debug("onBind");
        if ("com.mediatek.ims.rcsua.BIND_SERVICE".equals(intent.getAction())) {
            adapter.bindService(this);
            return new RcsUaServiceImpl();
        }

        return null;
    }

    @Override
    public void onRebind(Intent intent) {
        logger.debug("onReBind");
        adapter.bindService(this);
        if (serviceOptions.getBoolean(KEY_ACS_SUPPORT))
            startAcsService();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        logger.debug("onUnbind");
        adapter.unbindService();
        for (RuaClient client : clients.values()) {
            client.unregister();
        }
        clients.clear();
        if (acsService != null) {
            if (acsCallback != null) {
                try {
                    acsService.unregisterAcsCallback(acsCallback);
                } catch (RemoteException e) {
                }
                acsCallback = null;
            }
            unbindService(acsServiceConnection);
            acsServiceConnection = null;
            acsService = null;
        }
        return true;
    }


    @Override
    public void onDestroy() {
        logger.debug("onDestroy");
        for (RuaClient client : clients.values()) {
            client.unregister();
        }
        unregisterSimStateReceiver();
        if (acsService != null) {
            if (acsCallback != null) {
                try {
                    acsService.unregisterAcsCallback(acsCallback);
                } catch (RemoteException e) {
                }
                acsCallback = null;
            }
            unbindService(acsServiceConnection);
            acsServiceConnection = null;
            acsService = null;
        }

        RuaAdapter.destroyInstance();
    }


    private void activate() {
        logger.debug("activate");
        if (!activated) {
            activated = true;
            adapter.sendRcsActivation(capability);
            for (RuaClient client : clients.values()) {
                client.activate();
            }
            if (acsService != null) {
                try {
                    acsService.setRcsSwitchState(true);
                } catch (RemoteException e) {
                }
            }
        }
    }

    private void deactivate() {
        logger.debug("deactivate");
        if (activated) {
            activated = false;
            adapter.sendRcsDeactivation();
            for (RuaClient client : clients.values()) {
                client.deactivate();
            }
            if (acsService != null) {
                try {
                    acsService.setRcsSwitchState(false);
                } catch (RemoteException e) {
                }
            }
        }
    }

    private void triggerReregistration(Capability features) {
        logger.debug("triggerReregistraion");
        if (features == null)
            adapter.sendRegReregister(capability);
        else
            adapter.sendRegReregister(features);
    }

    private void triggerRestoration() {
        logger.debug("triggerRestoration");
        adapter.requestImsRestoration();
    }

    private void updateCapabilities(Capability capabilities) {
        logger.debug("updateCapabilities");
        if (!this.capability.equals(capabilities)) {
            if (capabilities != null
                    && capabilities.toNumeric() != this.capability.toNumeric())
                adapter.sendRegUpdateCapabilities(activated, capabilities);
            this.capability = capabilities;
        }
    }

    private void addCapability(Capability capability) {
        logger.debug("addCapability");
        long num = this.capability.toNumeric();
        this.capability.add(capability);
        if (capability != null && num != this.capability.toNumeric())
            adapter.sendRegUpdateCapabilities(activated, this.capability);
    }

    private void removeCapability(Capability capability) {
        logger.debug("removeCapability");
        long num = this.capability.toNumeric();
        this.capability.remove(capability);
        if (capability != null && num != this.capability.toNumeric())
            adapter.sendRegUpdateCapabilities(activated, this.capability);
    }

    private IRcsUaClient registerClient(IImsEventCallback callback) {
        logger.debug("registerClient");
        RuaClient client = new RuaClient(RuaService.this, callback);
        clients.put(client.getInterface().asBinder(), client);
        notifyRegistrationChangedTo(client);
        notifyVopsIndicationTo(client);

        return client.getInterface();
    }

    private void unregisterClient(IRcsUaClient client) {
        logger.debug("unregisterClient");
        if (client == null)
            return;

        for (Map.Entry<IBinder, RuaClient> entry : clients.entrySet()) {
            if (entry.getKey() == client.asBinder()) {
                entry.getValue().unregister();
                clients.remove(entry.getKey());
                break;
            }
        }
    }

    private void setOptions(Bundle options) {
        if (serviceOptions == null) {
            serviceOptions = new Bundle();
        }

        if (options != null) {
            boolean newValue = options.getBoolean(KEY_ROI_SUPPORT);
            boolean oldValue = serviceOptions.getBoolean(KEY_ROI_SUPPORT);
            if (newValue != oldValue && !oldValue) {
                serviceOptions.putBoolean(KEY_ROI_SUPPORT, newValue);
                adapter.setRoiSupport(newValue);
            }

            int newDereg = options.getInt(KEY_DEREG_SUSPEND, -1);
            int oldDereg = serviceOptions.getInt(KEY_DEREG_SUSPEND);
            if (newDereg != oldDereg && newDereg >= 0) {
                serviceOptions.putInt(KEY_DEREG_SUSPEND, newDereg);
                adapter.setDeregSuspend(newDereg);
            }

            newValue = options.getBoolean(KEY_ACS_SUPPORT);
            oldValue = serviceOptions.getBoolean(KEY_ACS_SUPPORT);
            if (newValue != oldValue && !oldValue)
                serviceOptions.putBoolean(KEY_ACS_SUPPORT, newValue);
        }
    }

    private Bundle getOptions() {
        if (serviceOptions == null) {
            serviceOptions = new Bundle();
        }
        return serviceOptions;
    }

    Capability getCapabilities() {
        return capability;
    }

    private boolean isActivated() {
        return activated;
    }

    void notifyRegistrationChanged() {
        logger.debug("notifyRegistrationChanged");
        if (activated && !clients.isEmpty())
            Message.obtain(imsCallbackHanlder, MSG_IMS_CALLBACK, null).sendToTarget();

        if (acsService != null && adapter.getRegistrationInfo().isRegistered()) {
            logger.debug("notify acs");
            String msisdn = adapter.getMsisdn();
            try {
                acsService.setAcsMsisdn(msisdn);
            } catch(RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    void notifyRegistrationChangedTo(RuaClient client) {
        logger.debug("notifyRegistrationChangedTo");
        if (activated)
            Message.obtain(imsCallbackHanlder, MSG_IMS_CALLBACK, client).sendToTarget();
    }

    void notifyDeregStart() {
        logger.debug("notifyDeregistrationStart");
        if (activated)
            Message.obtain(imsCallbackHanlder, MSG_IMS_DEREG_IND).sendToTarget();
    }

    void notifyReregistration() {
        logger.debug("notifyReregistration");
        if (activated)
            Message.obtain(imsCallbackHanlder, MSG_IMS_REREG_IND).sendToTarget();
    }

    void notifyAcsConfigChanged(int status, int version) {
        logger.debug("notifyAcsConfigChanged");
        Message.obtain(imsCallbackHanlder, MSG_ACS_CONFIG_CHANGE, status, version)
                .sendToTarget();
    }

    void notifyAcsConfigChangedTo(IAcsEventCallback callback, int status, int version) {
        logger.debug("notifyAcsConfigChangedTo");
        Message.obtain(imsCallbackHanlder, MSG_ACS_CONFIG_CHANGE_TO, status, version, callback)
                .sendToTarget();
    }

    void notifyAcsConnectionStatus(int status) {
        logger.debug("notifyAcsConnectionStatus");
        Message.obtain(imsCallbackHanlder, MSG_ACS_CONNECT_CHANGE, status).sendToTarget();
    }

    void notifyVopsIndicationTo(RuaClient client) {
        int vops = adapter.getVops();
        logger.debug("notifyVopsChangedTo:" + vops);
        if (vops >= 0) {
            Message.obtain(imsCallbackHanlder, MSG_IMS_VOPS_IND, vops, 0, client).sendToTarget();
        }
    }

    void notifyVopsIndication(int vops) {
        logger.debug("notifyVopsIndication:" + vops);
        Message.obtain(imsCallbackHanlder, MSG_IMS_VOPS_IND, vops, 0, null).sendToTarget();
    }

    private void startAcsService() {
        logger.debug("startAcsService");
        if (acsServiceConnection != null)
            return;

        acsServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                logger.debug("onServiceConnected:" + service);

                try {
                    if (service != null) {
                        service.linkToDeath(deathRecipient, 0);
                        acsService = IAcsService.Stub.asInterface(service);
                        if (acsCallbacks.getRegisteredCallbackCount() > 0)
                            registerCallbackToAcsService();

                        if (adapter.getRegistrationInfo().isRegistered()) {
                            logger.debug("onServiceConnected notify acs");
                            String msisdn = adapter.getMsisdn();
                            try {
                                acsService.setAcsMsisdn(msisdn);
                            } catch(RemoteException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                } catch (RemoteException e) {
                }
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                logger.debug("onServiceDisconnected:" + name);
                if (acsService != null) {
                    acsService.asBinder().unlinkToDeath(deathRecipient, 0);
                    acsCallback = null;
                    acsService = null;
                }
            }
        };

        Intent intent = new Intent();
        intent.setClassName("com.mediatek.rcs.provisioning",
                "com.mediatek.rcs.provisioning.https.AcsService");

        startService(intent);
        bindService(intent, acsServiceConnection, Context.BIND_AUTO_CREATE);
    }

    private void registerCallbackToAcsService() {
        if (acsCallback == null) {
            acsCallback = new IAcsCallback.Stub() {
                @Override
                public void onAcsConfigChange(int configStatus, int configVersion) throws RemoteException {
                    notifyAcsConfigChanged(configStatus, configVersion);
                }

                @Override
                public void onAcsConnectionStatusChange(int connectionStatus) throws RemoteException {
                    notifyAcsConnectionStatus(connectionStatus);
                }
            };
            try {
                acsService.registerAcsCallback(acsCallback);
            } catch (RemoteException e) {
            }
        }
    }

    private class RcsUaServiceImpl extends IRcsUaService.Stub {

        @Override
        public void activate() throws RemoteException {
            RuaService.this.activate();
        }

        @Override
        public void deactivate() throws RemoteException {
            RuaService.this.deactivate();
        }

        @Override
        public void triggerReregistration(Capability features) throws RemoteException {
            RuaService.this.triggerReregistration(features);
        }

        @Override
        public void triggerRestoration() throws RemoteException {
            RuaService.this.triggerRestoration();
        }

        @Override
        public IRcsUaClient registerClient(IImsEventCallback callback)
                throws RemoteException {
            return RuaService.this.registerClient(callback);
        }

        @Override
        public void unregisterClient(IRcsUaClient client) throws RemoteException {
            RuaService.this.unregisterClient(client);
        }

        @Override
        public void updateCapabilities(Capability capabilities) throws RemoteException {
            RuaService.this.updateCapabilities(capabilities);
        }

        @Override
        public void removeCapability(Capability capability) throws RemoteException {
            RuaService.this.removeCapability(capability);
        }

        @Override
        public void addCapability(Capability capability) throws RemoteException {
            RuaService.this.addCapability(capability);
        }

        @Override
        public Capability getCapabilities() throws RemoteException {
            return RuaService.this.getCapabilities();
        }

        @Override
        public boolean isActivated() throws RemoteException {
            return RuaService.this.isActivated();
        }

        @Override
        public void setOptions(Bundle options) throws RemoteException {
            RuaService.this.setOptions(options);
        }

        @Override
        public Bundle getOptions() throws RemoteException {
            return RuaService.this.getOptions();
        }

        @Override
        public void registerAcsCallback(IAcsEventCallback callback) {
            if (acsService != null && acsCallback == null) {
                registerCallbackToAcsService();
            } else if (acsCallback != null) {
                try {
                    AcsConfigInfo info  = acsService.getAcsConfiguration();
                    if (info != null) {
                        int status = info.getStatus();
                        int version = info.getVersion();
                        notifyAcsConfigChangedTo(callback, status, version);
                    }
                } catch (RemoteException e) {
                }
            }

            acsCallbacks.register(callback);
        }

        @Override
        public void unregisterAcsCallback(IAcsEventCallback callback) {
            acsCallbacks.unregister(callback);
            if (acsCallbacks.getRegisteredCallbackCount() == 0) {
                if (acsService != null && acsCallback != null) {
                    try {
                        acsService.unregisterAcsCallback(acsCallback);
                    } catch (RemoteException e) {
                    }
                    acsCallback = null;
                }
            }
        }

        @Override
        public AcsConfiguration getAcsConfiguration() throws RemoteException {
            AcsConfiguration config = null;
            if (acsService != null) {
                AcsConfigInfo info = acsService.getAcsConfiguration();
                if (info != null) {
                    config = new AcsConfiguration(info.getData(), info.getVersion());
                }
            }

            return config;
        }

        @Override
        public int getAcsConfigInt(String configItem) throws RemoteException {
            int value = 0;

            if (acsService != null) {
                value = acsService.getAcsConfigInt(configItem);
            }
            return value;
        }

        @Override
        public String getAcsConfigString(String configItem) throws RemoteException {
            String value = null;

            if (acsService != null) {
                value = acsService.getAcsConfigString(configItem);
            }
            return value;
        }

        @Override
        public boolean isAcsConnected() throws RemoteException {
            boolean connected = false;

            if (acsService != null) {
                connected = acsService.getAcsConnectionStatus() == 0;
            }

            return connected;
        }

        @Override
        public void triggerAcsRequest(int reason) throws RemoteException {
            if (acsService != null) {
                acsService.triggerAcsRequest(reason);
            }
        }

        @Override
        public boolean setAcsSwitchState(boolean state) throws RemoteException {
            boolean result = false;
            if (acsService != null) {
                result = acsService.setAcsSwitchState(state);
            }
            return result;
        }

        @Override
        public boolean getAcsSwitchState() throws RemoteException {
            boolean state = false;
            if (acsService != null) {
                state = acsService.getAcsSwitchState();
            }
            return state;
        }

        @Override
        public boolean setAcsProvisioningAddress(String address) throws RemoteException {
            boolean result = false;
            if (acsService != null) {
                result = acsService.setAcsProvisioningAddress(address);
            }
            return result;
        }

        @Override
        public boolean clearAcsConfiguration() throws RemoteException {
            boolean result =false;
            if (acsService != null) {
                result = acsService.clearAcsConfiguration();
            }
            return result;
        }
    }

    private final Handler imsCallbackHanlder = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (MSG_IMS_CALLBACK == msg.what) {
                RuaClient client = (RuaClient)msg.obj;
                if (client == null) {
                    for (RuaClient c : clients.values()) {
                        c.notifyRegistrationChanged(adapter.getRegistrationInfo());
                    }
                } else {
                    client.notifyRegistrationChanged(adapter.getRegistrationInfo());
                }
                return;
            }
            if (MSG_IMS_DEREG_IND == msg.what) {
                for (RuaClient client : clients.values()) {
                    client.notifyDeregStart(adapter.getRegistrationInfo());
                }
                return;
            }
            if (MSG_IMS_REREG_IND == msg.what) {
                for (RuaClient client : clients.values()) {
                    client.notifyReRegistration(adapter.getRegistrationInfo());
                }
                return;
            }
            if (MSG_IMS_VOPS_IND == msg.what) {
                int vops = msg.arg1;
                RuaClient client = (RuaClient)msg.obj;
                if (client == null) {
                    for (RuaClient c : clients.values()) {
                        c.notifyVopsInfication(vops);
                    }
                } else {
                    client.notifyVopsInfication(vops);
                }
                return;
            }

            if (MSG_ACS_CONFIG_CHANGE == msg.what) {

                boolean valid = msg.arg1 != 0;
                int version = msg.arg2;
                synchronized (acsCallbacks) {
                    final int N = acsCallbacks.beginBroadcast();
                    for (int i = 0; i < N; i++) {
                        try {
                            acsCallbacks.getBroadcastItem(i).onConfigChanged(valid, version);
                        } catch (RemoteException e) {
                            // The RemoteCallbackList will take care of removing
                            // the dead object for us.
                        }
                    }
                    acsCallbacks.finishBroadcast();
                }

                return;
            }

            if (MSG_ACS_CONFIG_CHANGE_TO == msg.what) {
                boolean valid = msg.arg1 != 0;
                int version = msg.arg2;
                IAcsEventCallback callback = (IAcsEventCallback)msg.obj;
                try {
                    callback.onConfigChanged(valid, version);
                } catch (RemoteException e) {
                }
                return;
            }

            if (MSG_ACS_CONNECT_CHANGE == msg.what) {
                boolean connected = msg.arg1 == 0;
                synchronized (acsCallbacks) {
                    final int N = acsCallbacks.beginBroadcast();
                    for (int i = 0; i < N; i++) {
                        try {
                            acsCallbacks.getBroadcastItem(i).onConnectionChanged(connected);
                        } catch (RemoteException e) {
                            // The RemoteCallbackList will take care of removing
                            // the dead object for us.
                        }
                    }
                    acsCallbacks.finishBroadcast();
                }
                return;
            }
        }
    };

    private void registerSimStateReceiver() {
        if (simStateReceiver != null) {
            return;
        }

        Context context = Utils.getApplicationContext();

        simStateReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(final Context context, final Intent intent) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                        SubscriptionManager.INVALID_PHONE_INDEX);
                String state = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

                logger.debug("SIM[" + phoneId + "] state changed to [" + state + "]");

                switch (state) {
                    case IccCardConstants.INTENT_VALUE_ICC_IMSI:
                    case IccCardConstants.INTENT_VALUE_ICC_LOADED:

                        if (Utils.isSimSupported())
                            adapter.enable();

                        break;

                    case IccCardConstants.INTENT_VALUE_ICC_ABSENT:
                        adapter.disable();
                        break;

                    default:
                        break;
                }
            }
        };

        // Register network state listener
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        context.registerReceiver(simStateReceiver, intentFilter);
    }

    private void unregisterSimStateReceiver() {
        if (simStateReceiver != null) {
            Utils.getApplicationContext().unregisterReceiver(simStateReceiver);
            simStateReceiver = null;
        }
    }

    private class ServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            acsServiceConnection = null;
            acsCallback = null;
            acsService = null;
        }
    }

    private RemoteCallbackList<IAcsEventCallback> acsCallbacks
            = new RemoteCallbackList<IAcsEventCallback>() {
        @Override
        public void onCallbackDied(IAcsEventCallback callback) {

        }
    };

    private RuaAdapter adapter;
    private Capability capability;
    private boolean activated;
    private int phoneId;
    private ConcurrentHashMap<IBinder, RuaClient> clients = new ConcurrentHashMap<>();
    private Bundle serviceOptions;
    private BroadcastReceiver simStateReceiver;
    private IAcsService acsService;
    private IAcsCallback acsCallback;
    private ServiceConnection acsServiceConnection;
    private ServiceDeathRecipient deathRecipient = new ServiceDeathRecipient();
    private Logger logger = Logger.getLogger(RuaService.class.getName());

    private static final int MSG_IMS_CALLBACK = 1001;
    private static final int MSG_IMS_DEREG_IND = 1002;
    private static final int MSG_IMS_REREG_IND = 1003;
    private static final int MSG_IMS_VOPS_IND  = 1004;
    private static final int MSG_ACS_CONFIG_CHANGE = 2001;
    private static final int MSG_ACS_CONNECT_CHANGE = 2002;
    private static final int MSG_ACS_CONFIG_CHANGE_TO = 2003;

    private static final String EXTRA_PHONE_ID = "phone_id";
    private static final String EXTRA_OPTIONS = "service_options";
    private static final String KEY_ACS_SUPPORT = "OPTION_ACS_SUPPORT";
    private static final String KEY_ROI_SUPPORT = "OPTION_ROI_SUPPORT";
    private static final String KEY_DEREG_SUSPEND = "OPTION_DEREG_SUSPEND";
}
