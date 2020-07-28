/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.mediatek.presence.service;

import java.util.Vector;
import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynService;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.PatternMatcher;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.SubscriptionManager;

import com.android.ims.ImsConfig;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.presence.core.ims.network.NetworkConnectivityApi;
import com.mediatek.presence.core.ims.network.INetworkConnectivityApi;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.CoreListener;
import com.mediatek.presence.core.ims.ImsError;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.presence.extension.ViLTEExtension;
import com.mediatek.presence.core.ims.service.presence.PresenceUtils;
import com.mediatek.presence.core.ims.service.presence.pidf.CapabilityDetails;
import com.mediatek.presence.core.ims.service.presence.pidf.PidfDocument;
import com.mediatek.presence.core.ims.service.presence.pidf.Tuple;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsBackupHelper;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.service.api.CapabilityServiceImpl;
import com.mediatek.presence.service.api.ContactsServiceImpl;
import com.mediatek.presence.service.api.ServerApiUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.mediatek.presence.service.api.OptionsServiceImpl;

import com.gsma.services.rcs.ICoreServiceWrapper;

import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.service.api.PresenceServiceImpl;

import com.android.ims.internal.uce.presence.PresPublishTriggerType;

/**
 * RCS core service. This service offers a flat API to any other process (activities)
 * to access to RCS features. This service is started automatically at device boot.
 *
 * @author Jean-Marc AUFFRET
 */
public class RcsCoreService extends Service implements CoreListener {
    /**
     * Service name
     */
    public final static int RCS_CORE_LOADED = 0;
    public final static int RCS_CORE_FAILED = 1;
    public final static int RCS_CORE_STARTED = 2;
    public final static int RCS_CORE_STOPPED = 3;
    public final static int RCS_CORE_IMS_CONNECTED = 4;
    public final static int RCS_CORE_IMS_TRY_CONNECTION = 5;
    public final static int RCS_CORE_IMS_CONNECTION_FAILED = 6;
    public final static int RCS_CORE_IMS_TRY_DISCONNECT = 7;
    public final static int RCS_CORE_IMS_BATTERY_DISCONNECTED = 8;
    public final static int RCS_CORE_IMS_DISCONNECTED = 9;
    public final static int RCS_CORE_NOT_LOADED = 10;

    public final static int DEFAULT_SLOT_ID = 0;

    public static final String CORE_SERVICE_CURRENT_STATE_NOTIFICATION =
        "com.mediatek.presence.CORE_SERVICE_STATE";

    public static int CURRENT_STATE = RCS_CORE_NOT_LOADED;

    public static final String SERVICE_NAME = "com.mediatek.presence.SERVICE";

    public static final String LABEL = "label";

    public static final String LABEL_ENUM = "label_enum";

    public static final String STATE = "state";

    /**
     * Notification ID
     */
    private final static int SERVICE_NOTIFICATION = 1000;

    public CoreServiceWrapperStub mCoreServiceWrapperStub;

    // --------------------- RCSJTA API -------------------------
    /**
     * Terms API
     */
    //private TermsApiService termsApi = new TermsApiService();
    /**
     * Contacts API
     */
    private ContactsServiceImpl contactsApi = null;

    /**
     * Capability API
     */
    private CapabilityServiceImpl capabilityApi = null;

    /**
     * AOSP Presence Service API
     */
    private PresenceServiceImpl presenceServiceApi = null;

    /**
     * AOSP Options Service API
     */
    private OptionsServiceImpl optionsServiceApi = null;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private NetworkStateListener[] mPhoneStateListenerList = null;
    private int[] mCurrentNetwork = null;
    private boolean[] mVtEnabled;
    private final static String EXTRA_PHONE_ID = "phone_id";
    private int mSimCount = 0;
    private Context mContext = null;

    // Network Type
    private static final int NONE_NETWORK      = 0;
    private static final int GERAN_NETWORK     = 1; // 2g
    private static final int UTRAN_NETWORK     = 2; // 3g exclude EHRPD and HSPAP
    private static final int EUTRAN_NETWORK    = 3; // LTE
    private static final int WIFI_NETWORK      = 4; // WIFI
    private static final int EHRPD_NETWORK     = 5; // EHRPD
    private static final int HSPAP_NETWORK     = 6; // HSPAP

    @Override
    public void onCreate() {
        logger.info("onCreate, startCore");
        mContext = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (tm == null) {
            logger.error("RcsCoreService onCreate fail. TelephonyManager is null");
            return;
        }
        mSimCount = tm.getSimCount();
        logger.info("The SIM count is:  " + mSimCount);

        mCurrentNetwork = new int[mSimCount];
        for (int slotId = 0; slotId < mSimCount; ++slotId) {
            mCurrentNetwork[slotId] = NONE_NETWORK;
        }

        startCore();
        registerPhoneStateListener();

        IntentFilter filter = new IntentFilter();
        filter.addAction(ImsConfig.ACTION_IMS_FEATURE_CHANGED);
        mContext.registerReceiver(mVilteMonitor, filter);

        IntentFilter servicefilter = new IntentFilter();
        servicefilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        mContext.registerReceiver(mServiceMonitor, servicefilter);

        Intent status = new Intent(
                LauncherUtils.UCE_CORE_SERVICE_STATE_RCSCORESERVICE_ON);
        LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext()).sendBroadcast(status);
    }

    private PresenceService getService(int slotId) {
        Core core = Core.getInstance();
        if (core == null) {
            logger.debug("core is null");
            return null;
        }
        ImsModule im = core.getImsModule(slotId);
        if (im == null) {
            logger.debug("imsModule is null");
            return null;
        }
        PresenceService service = im.getPresenceService();
        if (service == null) {
            logger.debug("PresenceService is null");
            return null;
        }
        return service;
    }

    class NetworkStateListener extends PhoneStateListener {
        private int mSlotId = 0;

        public NetworkStateListener(int slotId) {
            mSlotId = slotId;
        }

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            /*
             * UE may move from 2/3G -> LTE or LTE -> 2/3G
             * If nternet PDN is connected, we can't update mCurrentNetwork correctly
             * Monitor the RAT change by onDataConnectionStateChanged() then update mCurrentNetwork properly
             *
             */
            logger.debug("onDataConnectionStateChanged, state: " + state + ", networkType: " + networkType);
            updateNetworkRatChanged(mSlotId, networkType);
        }

        @Override
        public void onServiceStateChanged(ServiceState state) {
            logger.debug("onServiceStateChanged: " + state);
            if (state != null) {
                updateNetworkRatChanged(mSlotId, state.getDataNetworkType());
            }
        }

        public int getSlotId() {
            return mSlotId;
        }

    }

    public void registerPhoneStateListener() {
        if (mPhoneStateListenerList != null) {
            logger.debug("registerPhoneStateListener ignore");
            return;
        }

        mPhoneStateListenerList = new NetworkStateListener[mSimCount];

        for (int slotId = 0; slotId < mSimCount; ++slotId) {
            logger.debug("registerPhoneStateListener for slot: " + slotId);
            int[] subIds = SubscriptionManager.getSubId(slotId);
            if (subIds == null) {
                logger.debug("registerPhoneStateListener fail. subIds is null");
                continue;
            }
            mPhoneStateListenerList[slotId] = new NetworkStateListener(slotId);
            TelephonyManager telephonyManager  = new TelephonyManager(mContext, subIds[0]);
            telephonyManager.listen(mPhoneStateListenerList[slotId],
                    PhoneStateListener.LISTEN_SERVICE_STATE);
        }

    }

    public void unregisterPhoneStateListener() {
        if (mPhoneStateListenerList != null) {
            for (int slotId = 0; slotId < mSimCount; ++slotId) {
                logger.debug("unregisterPhoneStateListener for slot: " + slotId);
                int[] subIds = SubscriptionManager.getSubId(slotId);
                if (subIds == null) {
                    logger.debug("unregisterPhoneStateListener fail. subIds is null");
                    continue;
                }
                TelephonyManager telephonyManager  = new TelephonyManager(mContext, subIds[0]);
                telephonyManager.listen(mPhoneStateListenerList[slotId],
                        PhoneStateListener.LISTEN_NONE);
                mPhoneStateListenerList[slotId] = null;

            }
            mPhoneStateListenerList = null;
        }
    }

    private void updateNetworkRatChanged(int slotId, int networkType) {
        PresenceService service = getService(slotId);
        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            logger.debug("networkType is NETWORK_TYPE_UNKNOWN");
            return;
        }

        boolean needModifyPublish = true;
        int new_network = NONE_NETWORK;

        switch(networkType) {
            //2G NETWORK
            case TelephonyManager.NETWORK_TYPE_GPRS:   // 1
            case TelephonyManager.NETWORK_TYPE_EDGE:   // 2
            case TelephonyManager.NETWORK_TYPE_CDMA:   // 4
            case TelephonyManager.NETWORK_TYPE_1xRTT:  // 7
            case TelephonyManager.NETWORK_TYPE_IDEN:   // 11
                new_network = GERAN_NETWORK;
                break;
             //3G NETWORK
            case TelephonyManager.NETWORK_TYPE_UMTS:   // 3
            case TelephonyManager.NETWORK_TYPE_EVDO_0: // 5
            case TelephonyManager.NETWORK_TYPE_EVDO_A: // 6
            case TelephonyManager.NETWORK_TYPE_HSDPA:  // 8
            case TelephonyManager.NETWORK_TYPE_HSUPA:  // 9
            case TelephonyManager.NETWORK_TYPE_HSPA:   // 10
            case TelephonyManager.NETWORK_TYPE_EVDO_B: // 12
                new_network = UTRAN_NETWORK;
                break;
            case TelephonyManager.NETWORK_TYPE_EHRPD:  // 14
                new_network = EHRPD_NETWORK;
                break;
                case TelephonyManager.NETWORK_TYPE_HSPAP:  // 15
                    new_network = HSPAP_NETWORK;
                    break;
                 //LTE NETWORK
                case TelephonyManager.NETWORK_TYPE_LTE:    // 13
                    new_network = EUTRAN_NETWORK;
                    break;
                //WIFI NETWORK
                case TelephonyManager.NETWORK_TYPE_IWLAN:  // 18
                    new_network = WIFI_NETWORK;
                    break;
                default:
                    needModifyPublish = false;
                    break;
            }

            logger.debug("mCurrentNetwork[" + slotId + "]=" + mCurrentNetwork[slotId] +
                    " new_network[" + slotId + "]=" + new_network);
            if (mCurrentNetwork[slotId] == NONE_NETWORK) {
                //don't impact initial publish
                needModifyPublish = false;
                mCurrentNetwork[slotId] = new_network;
            } else if (new_network == mCurrentNetwork[slotId]) {
                // ex: from UMTS <-> NETWORK_TYPE_HSPAP
                needModifyPublish = false;
            } else if (new_network != mCurrentNetwork[slotId]) {
                //LTE <-> Wi-Fi, Capabilities no changed
                if ((mCurrentNetwork[slotId] == EUTRAN_NETWORK || mCurrentNetwork[slotId] == WIFI_NETWORK)
                    && (new_network == EUTRAN_NETWORK || new_network == WIFI_NETWORK)) {
                    needModifyPublish = false;
                }
                mCurrentNetwork[slotId] = new_network;

                //AOSP procedure which only supports single SIM
                if (slotId == DEFAULT_SLOT_ID) {
                    switch (new_network) {
                    case GERAN_NETWORK:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_2G);
                        break;
                    case UTRAN_NETWORK:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_3G);
                        break;
                    case EHRPD_NETWORK:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_EHRPD);
                        break;
                    case WIFI_NETWORK:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_IWLAN);
                        break;
                    case HSPAP_NETWORK:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_HSPAPLUS);
                        break;
                    case EUTRAN_NETWORK:
                        if (ServerApiUtils.isImsConnected(slotId)) {
                            PresenceServiceImpl.publishTriggering(
                                    PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED);
                        } else {
                            PresenceServiceImpl.publishTriggering(
                                    PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED);
                        }
                        break;
                    default:
                        PresenceServiceImpl.publishTriggering(
                                PresPublishTriggerType.UCE_PRES_PUBLISH_TRIGGER_UNKNOWN);
                }

            }
        }

        if (service == null) {
            logger.error("During shutdown progress, ignore network change event");
            return;
        }

        ViLTEExtension vtex = (ViLTEExtension)service.getVilteService();
        vtex.handleNetworkChanged(mCurrentNetwork[slotId]);

        if (needModifyPublish) {
                Thread t = new Thread() {
                    @Override
                    public void run() {
                        //do not block main thread
                        logger.debug("do publishCapability for RAT change");
                        service.publishCapability();
                    }
                };
                t.start();
        }
    }

    @Override
    public void onDestroy() {

        stopCore();
        unregisterPhoneStateListener();
        AndroidFactory.getApplicationContext().unregisterReceiver(mVilteMonitor);
        AndroidFactory.getApplicationContext().unregisterReceiver(mServiceMonitor);

        Intent status = new Intent(
                LauncherUtils.UCE_CORE_SERVICE_STATE_RCSCORESERVICE_OFF);
        LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext()).sendBroadcast(status);
    }

    /**
     * Start core
     */
    public synchronized void startCore() {
        if (Core.getInstance() != null) {
            // Already started
            return;
        }

        try {
            if (logger.isActivated()) {
                logger.debug("Instanciate API");
            }

            // Instantiate the settings manager
            RcsSettingsManager.createRcsSettings();
            mVtEnabled = new boolean[mSimCount];
            for (int slotId = 0; slotId < mSimCount; ++slotId) {
                mVtEnabled[slotId] = RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .isIR94VideoCallSupported();
            }

            // Instantiate the contacts manager
            ContactsManager.createInstance(getApplicationContext());

            ContactsBackupHelper.createInstance(getApplicationContext());

            // Create the core
            Core.createCore(this);

            // Start the core
            Core.getInstance().startCore();

            // Instanciate API
            contactsApi = new ContactsServiceImpl();
            capabilityApi = new CapabilityServiceImpl();
            presenceServiceApi = new PresenceServiceImpl(getApplicationContext());
            optionsServiceApi = new OptionsServiceImpl(getApplicationContext());
            mCoreServiceWrapperStub = new CoreServiceWrapperStub();

            if (logger.isActivated()) {
                logger.info("Instantiate the contacts manager");
            }
        } catch(Exception e) {
            // Unexpected error
            if (logger.isActivated()) {
                logger.error("Can't instanciate API and the contacts manager", e);
            }

            // Exit service
            stopSelf();
        }
    }

    /**
     * Stop core
     */
    public synchronized void stopCore() {
        if (Core.getInstance() == null) {
            // Already stopped
            return;
        }

        if (logger.isActivated()) {
            logger.debug("Close APIs");
        }

        // Close APIs
        if (contactsApi != null) {
            contactsApi.close();
        }
        if (capabilityApi != null) {
            capabilityApi.close();
        }

        // Terminate the core in background
        Core.terminateCore();

        if (logger.isActivated()) {
            logger.info("APIs closed with success");
        }
    }

    public class CoreServiceWrapperStub extends ICoreServiceWrapper.Stub {

        String TAG = "CoreServiceWrapperStub";

        @Override
        public IBinder getChatServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getFileTransferServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getCapabilitiesServiceBinder()
        {
            if (logger.isActivated()) {
                //logger.debug("CoreServiceWrapperStub getCapabilitiesServiceBinder() entry");
            }
            return capabilityApi;
        }

        @Override
        public IBinder getContactsServiceBinder()
        {
            if (logger.isActivated()) {
                //logger.debug("CoreServiceWrapperStub getContactsServiceBinder() entry");
            }
            return contactsApi;
        }

        @Override
        public IBinder getGeolocServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getVideoSharingServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getImageSharingServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getNetworkConnectivityApiBinder()
        {
            if (logger.isActivated()) {
                //logger.debug("CoreServiceWrapperStub getNetworkConnectivityApiBinder() entry");
            }
            return null;
        }

        @Override
        public IBinder getHistoryServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getUploadServiceBinder()
        {
            return null;
        }

        @Override
        public IBinder getAospPresenceServiceBinder()
        {
            return presenceServiceApi;
        }

        @Override
        public IBinder getAospOptionsServiceBinder()
        {
            return optionsServiceApi;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        //logger.debug("onBind RCSCoreService");
        return mCoreServiceWrapperStub;
    }

    /**
     * Add RCS service notification
     *
     * @param state Service state (ON|OFF)
     * @param label Label
     */
    public static void addRcsServiceNotification(boolean state, String label, int labelEnum) {
        // Create notification
        //Intent intent = new Intent(Intents.Client.ACTION_VIEW_SETTINGS);
        //intent.putExtra(STATE, state);
        //intent.putExtra(LABEL, label);
        //intent.putExtra(LABEL_ENUM,labelEnum);
        //AndroidFactory.getApplicationContext().sendBroadcast(intent);
        //CURRENT_STATE= labelEnum;
        //RcsSettings.getInstance().setServiceCurrentState(labelEnum);

    }

    /*---------------------------- CORE EVENTS ---------------------------*/

    /**
     * Notify registration status to API
     *
     * @param status Status
     */
    private void notifyRegistrationStatusToApi(int slotId, boolean status) {
        if (capabilityApi != null) {
            capabilityApi.notifyRegistrationEvent(slotId, status);
        }
    }

    /**
     * Core layer has been started
     */
    public void handleCoreLayerStarted() {
        if (logger.isActivated()) {
            logger.debug("Handle event core started");
        }
        CURRENT_STATE = RCS_CORE_STARTED;
        Intent intent = new Intent(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intent.putExtra("status", CURRENT_STATE);
        LocalBroadcastManager.getInstance(getApplicationContext())
                .sendBroadcast(intent);
    }

    /**
     * Core layer has been terminated
     */
    public void handleCoreLayerStopped() {
        // Display a notification
        if (logger.isActivated()) {
            logger.debug("Handle event core terminated");
        }
        CURRENT_STATE = RCS_CORE_STOPPED;
        Intent intent = new Intent(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intent.putExtra("status", CURRENT_STATE);
        LocalBroadcastManager.getInstance(getApplicationContext())
                .sendBroadcast(intent);
    }

    /**
     * Handle "registration successful" event
     *
     * @param registered Registration flag
     */
    public void handleRegistrationSuccessful(int slotId) {
        if (logger.isActivated()) {
            logger.debug("Handle event registration ok");
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId)
                .setRegistrationState(true);
        // Send registration intent
        CURRENT_STATE = RCS_CORE_IMS_CONNECTED;
        Intent intent = new Intent(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intent.putExtra("status", CURRENT_STATE);
        intent.putExtra("slotId", slotId);
        LocalBroadcastManager.getInstance(getApplicationContext())
                .sendBroadcast(intent);
        notifyRegistrationStatusToApi(slotId, true);
    }

    /**M
     * added to how notification of connecting and disconnecting states during registration
     */
    /**
     * Handle "try registration" event
     */
    public void handleTryRegister() {
        if (logger.isActivated()) {
            logger.debug("Handle event try registration");
        }
    }

    /**
     * Handle "try registration" event
     */
    public void handleTryDeregister() {
        if (logger.isActivated()) {
            logger.debug("Handle event try deregistration");
        }
    }
    /** @*/

    /**
     * Handle "registration failed" event
     *
     * @param error IMS error
    */
    public void handleRegistrationFailed(int slotId, ImsError error) {
        if (logger.isActivated()) {
            logger.debug("Handle event registration failed");
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId).setRegistrationState(false);
        CURRENT_STATE = RCS_CORE_IMS_CONNECTION_FAILED;
        Intent intent = new Intent(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intent.putExtra("status", CURRENT_STATE);
        intent.putExtra("slotId", slotId);
        LocalBroadcastManager.getInstance(getApplicationContext())
                .sendBroadcast(intent);
        notifyRegistrationStatusToApi(slotId, false);
    }

    /**
     * Handle "registration terminated" event
     */
    public void handleRegistrationTerminated(int slotId) {
        if (logger.isActivated()) {
            logger.debug("Handle event registration terminated");
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId)
                .setRegistrationState(false);

        // Notify APIs
        CURRENT_STATE = RCS_CORE_IMS_DISCONNECTED;
        Intent intent = new Intent(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intent.putExtra("status", CURRENT_STATE);
        intent.putExtra("slotId", slotId);
        LocalBroadcastManager.getInstance(getApplicationContext())
                .sendBroadcast(intent);
        notifyRegistrationStatusToApi(slotId, false);
    }

    /**
     * A new presence sharing notification has been received
     *
     * @param contact Contact
     * @param status Status
     * @param reason Reason
     */
    public void handlePresenceSharingNotification(String contact, String status, String reason) {
        if (logger.isActivated()) {
            logger.debug("Handle event presence sharing notification for " + contact + " (" + status + ":" + reason + ")");
        }
        // Not used
    }

    /**
     * A new presence info notification has been received
     *
     * @param contact Contact
     * @param presense Presence info document
     */
    public void handlePresenceInfoNotification(int slotId,
            String contact, PidfDocument presence) {
        if (logger.isActivated()) {
            logger.debug("Handle event presence info notification for " + contact);
        }

        if (presence != null) {
            // Extract capabilities
            Capabilities capabilities =  new Capabilities();
            int registrationState =  ContactInfo.REGISTRATION_STATUS_UNKNOWN;
            Vector<Tuple> tuples = presence.getTuplesList();

            if(tuples.size()>0)
            {
                //set the registration as true as the device published some capablity
                // We queried via anonymous fetch procedure, so set presence discovery to true
                registrationState =  ContactInfo.REGISTRATION_STATUS_ONLINE;

            for(int i=0; i < tuples.size(); i++) {
                Tuple tuple = (Tuple)tuples.elementAt(i);
                boolean state = false;
                if (tuple.getStatus().getBasic().getValue().equals("open")) {
                    state = true;
                }
                String id = tuple.getService().getId();

                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                    capabilities.setVideoSharingSupport(state);
                } else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                    capabilities.setVideoSharingSupport(state);
                } else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IMAGE_SHARE)) {
                    capabilities.setImageSharingSupport(state);
                } else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT)) {
                    //file transfer
                    capabilities.setFileTransferSupport(state);
                } else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CS_VIDEO)) {
                    capabilities.setCsVideoSupport(state);
                }
                else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL)) {

                    if(state)
                    {
                        boolean audio = false;
                        boolean video = false;
                        boolean duplexmode = false;

                      for(CapabilityDetails c : tuple.getServiceCapability().getCapabilityList())
                      {
                          //check for audio
                          if(c.getName().equalsIgnoreCase("audio")){
                              if(c.getValue().equalsIgnoreCase("true"))
                              {
                                  audio = true;
                                  continue;
                              }
                          }

                          //check for audio
                          if(c.getName().equalsIgnoreCase("video")){
                              if(c.getValue().equalsIgnoreCase("true"))
                              {
                                  video = true;
                                  continue;
                              }
                          }

                        //check for duplex
                          if(c.getName().equalsIgnoreCase("duplex")){
                              if(c.getValue().equalsIgnoreCase("full"))
                              {
                                  duplexmode = true;
                                  continue;
                              }
                          }
                      }

                      if (logger.isActivated()) {
                          logger.debug("Video capability for : "+contact + "  - audio : "+audio + " ; video : " + video + " ; duplex :- " +duplexmode);
                      }

                      capabilities.setIR94_VoiceCall(audio);
                      capabilities.setIR94_VideoCall(video);
                      capabilities.setIR94_DuplexMode(duplexmode);

                    }

                }
                else
                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE)) {
                        capabilities.setPresenceDiscoverySupport(state);
                }
                else
                if ((id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT))||
                    (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT_2))
                    ) {
                    //IM capability
                    capabilities.setImSessionSupport(state);
                }
            }

          } else {
              if (logger.isActivated()) {
                 logger.debug("No capability tuples found for : "+contact + " ; assuming its offline and update capabilities in database");
              }

              registrationState =  ContactInfo.REGISTRATION_STATUS_OFFLINE;
          }

            if (logger.isActivated()) {
                logger.debug("Set Contact Capability via Presence : "+contact);
            }

            // Update capabilities in database
            ContactsManager.getInstance().setContactCapabilities(contact, capabilities, ContactInfo.RCS_CAPABLE, registrationState);

            // Notify listener
            handleCapabilitiesNotification(contact, capabilities);
        }
        // Not used
    }

    /**
     * Capabilities update notification has been received
     *
     * @param contact Contact
     * @param capabilities Capabilities
     */
    public void handleCapabilitiesNotification(String contact, Capabilities capabilities) {
        if (logger.isActivated()) {
            logger.debug("Handle capabilities update notification for " + contact + " (" + capabilities.toString() + ")");
        }

        // Extract number from contact
        String number = PhoneUtils.extractNumberFromUri(contact);

        // Notify API
        capabilityApi.receiveCapabilities(number, capabilities);
    }

    /**
     * A new presence sharing invitation has been received
     *
     * @param contact Contact
     */
    public void handlePresenceSharingInvitation(String contact) {
        if (logger.isActivated()) {
            logger.debug("Handle event presence sharing invitation");
        }
        // Not used
    }

    /**
     * New message delivery status
     *
     * @param contact Contact
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String contact, String msgId, String status) {
        if (logger.isActivated()) {
            logger.debug("Handle message delivery status");
        }
    }

    /**
     * New message delivery status
     *
     * @param contact Contact
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String contact, String msgId, String status , int errorCode, String statusCode) {
        if (logger.isActivated()) {
            logger.debug("Handle message delivery status");
        }
    }

    /**
     * New file delivery status
     *
     * @param ftSessionId File transfer session ID
     * @param status Delivery status
     */
    public void handleFileDeliveryStatus(String ftSessionId, String status,String contact) {
        if (logger.isActivated()) {
            logger.debug("Handle file delivery status: session " + ftSessionId + " status " + status + "Contact: " + contact);
        }
    }

    /**
     * New SIP instant message received
     *
     * @param intent Resolved intent
     * @param message Instant message request
     */
    public void handleSipInstantMessageReceived(Intent intent, SipRequest message) {
        if (logger.isActivated()) {
            logger.debug("Handle event receive SIP instant message");
        }

    }

    /**
     * User terms confirmation request
     *
     * @param remote Remote server
     * @param id Request ID
     * @param type Type of request
     * @param pin PIN number requested
     * @param subject Subject
     * @param text Text
     * @param btnLabelAccept Label of Accept button
     * @param btnLabelReject Label of Reject button
     * @param timeout Timeout request
     */
    public void handleUserConfirmationRequest(String remote, String id,
            String type, boolean pin, String subject, String text,
            String acceptButtonLabel, String rejectButtonLabel, int timeout) {
        if (logger.isActivated()) {
            logger.debug("Handle event user terms confirmation request");
        }

        //termsApi.receiveTermsRequest(remote, id, type, pin, subject, text, acceptButtonLabel, rejectButtonLabel, timeout);
    }

    /**
     * User terms confirmation acknowledge
     *
     * @param remote Remote server
     * @param id Request ID
     * @param status Status
     * @param subject Subject
     * @param text Text
     */
    public void handleUserConfirmationAck(String remote, String id, String status, String subject, String text) {
        if (logger.isActivated()) {
            logger.debug("Handle event user terms confirmation ack");
        }

        //termsApi.receiveTermsAck(remote, id, status, subject, text);
    }

    /**
     * User terms notification
     *
     * @param remote Remote server
     * @param id Request ID
     * @param subject Subject
     * @param text Text
     * @param btnLabel Label of OK button
     */
    public void handleUserNotification(String remote, String id, String subject, String text, String okButtonLabel) {
        if (logger.isActivated()) {
            logger.debug("Handle event user terms notification");
        }

        //termsApi.receiveUserNotification(remote, id, subject, text, okButtonLabel);
    }

    /**
     * SIM has changed
     */
    public void handleSimHasChanged() {
        if (logger.isActivated()) {
            logger.debug("Handle SIM has changed");
        }

        // Restart the RCS service
        LauncherUtils.stopRcsService(getApplicationContext());
        LauncherUtils.launchRcsService(getApplicationContext(), true, false);
    }

    private BroadcastReceiver mVilteMonitor = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(ImsConfig.ACTION_IMS_FEATURE_CHANGED)) {
                int feature = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
                int phoneId = intent.getIntExtra(EXTRA_PHONE_ID, -1);
                int value  = intent.getIntExtra(ImsConfig.EXTRA_NEW_VALUE, -1);
                if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE) {
                    logger.debug("onReceived IMS feature changed phoneId: "
                                + phoneId + ", feature: " + feature
                                + ", value: " + value);
                    boolean enable = (value == ImsConfig.FeatureValueConstants.ON);
                    if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                        return;
                    }
                    onVtEnabled(phoneId, enable);
                }
            }
        }
     };

    private void onVtEnabled(int slotId, boolean enabled) {
        if (SimUtils.isVzwSimCard(slotId)) {
            logger.debug("ignore vilte enabled check at vzw");
            return;
        }
        //ATT_UCE_9_4_2
        //TRICKY: after vt is enable/disable, the PUBLISH should be sent after IMS re-register
        //but we directly call publishCapability here due to it will queued by deBounce timer(30 sec)
        //will and be sent till next time slot.(tester should on/off VT in 30 sec after initial publish)
        if (mVtEnabled[slotId] != enabled) {
            logger.debug("[slot" + slotId + "] onVtEnabled change from " + mVtEnabled[slotId] + " to " + enabled);
            mVtEnabled[slotId] = enabled;
            PresenceService service = getService(slotId);
            if (service != null) {
                //update vilte capabilities
                ViLTEExtension vtex = (ViLTEExtension)service.getVilteService();
                vtex.handleCapabilityChanged(mVtEnabled[slotId]);
                //the video caps will be add/remove by RcsSettings.isAospVideoCallingSettingOn()
                Thread t = new Thread() {
                    @Override
                    public void run() {
                        //do not block main thread
                        logger.debug("[slot" + slotId + "] do publishCapability for VT change");
                        service.publishCapability();
                    }
                };
                t.start();
            }
        }
    }

    private BroadcastReceiver mServiceMonitor = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                Bundle extras = intent.getExtras();
                if (extras != null) {
                    ServiceState ss = ServiceState.newFromBundle(extras);
                    if (ss != null) {
                        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                        int slotId = SubscriptionManager.getSlotIndex(subId);
                        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                            return;
                        }
                        PresenceService service = getService(slotId);
                        if (service != null) {
                            Thread t = new Thread() {
                                @Override
                                public void run() {
                                    //do not block main thread
                                    logger.debug("ACTION_SERVICE_STATE_CHANGED");
                                    service.handleServiceStateChanged(ss);
                                }
                            };
                            t.start();
                        }
                    }
                }
            }
        }
     };
}
