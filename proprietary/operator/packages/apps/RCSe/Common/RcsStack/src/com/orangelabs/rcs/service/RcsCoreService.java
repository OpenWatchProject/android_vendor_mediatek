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

package com.orangelabs.rcs.service;

import java.util.Date;
import java.util.Vector;
import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.capability.ICapabilityService;
import com.gsma.services.rcs.chat.IChatService;
import com.gsma.services.rcs.contact.IContactService;
import com.gsma.services.rcs.ft.IFileTransferService;
import com.gsma.services.rcs.sharing.geoloc.IGeolocSharingService;
import com.gsma.services.rcs.sharing.image.IImageSharingService;
import com.gsma.services.rcs.sharing.video.IVideoSharingService;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;

import com.mediatek.ims.rcsua.RcsUaService;
import com.orangelabs.rcs.core.ims.network.NetworkConnectivityApi;
import com.orangelabs.rcs.core.ims.network.INetworkConnectivityApi;
import com.orangelabs.rcs.service.api.client.terms.ITermsApi;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.addressbook.AccountChangedReceiver;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.CoreListener;
import com.orangelabs.rcs.core.TerminalInfo;
import com.orangelabs.rcs.core.ims.ImsError;
import com.orangelabs.rcs.core.ims.network.NetworkApiService;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.TerminatingAdhocGroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.TerminatingOne2OneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.TerminatingStandaloneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.standfw.TerminatingStoreAndForwardMsgSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.presence.PresenceUtils;
import com.orangelabs.rcs.core.ims.service.presence.pidf.CapabilityDetails;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfDocument;
import com.orangelabs.rcs.core.ims.service.presence.pidf.Tuple;
import com.orangelabs.rcs.core.ims.service.richcall.geoloc.GeolocTransferSession;
import com.orangelabs.rcs.core.ims.service.richcall.image.ImageTransferSession;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoStreamingSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.eab.ContactsBackupHelper;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.sharing.RichCallHistory;
import com.orangelabs.rcs.service.api.CapabilityServiceImpl;
import com.orangelabs.rcs.service.api.ChatServiceImpl;
import com.orangelabs.rcs.service.api.ContactsServiceImpl;
import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.GeolocSharingServiceImpl;
import com.orangelabs.rcs.service.api.HistoryServiceImpl;
import com.orangelabs.rcs.service.api.ImageSharingServiceImpl;
import com.orangelabs.rcs.service.api.TermsApiService;
import com.orangelabs.rcs.service.api.VideoSharingServiceImpl;
import com.orangelabs.rcs.service.api.FileUploadServiceImpl;
import com.orangelabs.rcs.utils.AppUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningSMS;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningUtils;

import com.gsma.services.rcs.ICoreServiceWrapper;
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
    private final static int RCS_CORE_LOADED = 0;
    private final static int RCS_CORE_FAILED = 1;
    private final static int RCS_CORE_STARTED = 2;
    private final static int RCS_CORE_STOPPED = 3;
    private final static int RCS_CORE_IMS_CONNECTED = 4;
    private final static int RCS_CORE_IMS_TRY_CONNECTION = 5;
    private final static int RCS_CORE_IMS_CONNECTION_FAILED = 6;
    private final static int RCS_CORE_IMS_TRY_DISCONNECT = 7;
    private final static int RCS_CORE_IMS_BATTERY_DISCONNECTED = 8;
    private final static int RCS_CORE_IMS_DISCONNECTED = 9;
    private final static int RCS_CORE_NOT_LOADED = 10;

    public static int CURRENT_STATE= RCS_CORE_NOT_LOADED;

    public static final String SERVICE_NAME = "com.orangelabs.rcs.SERVICE";

    public static final String LABEL = "label";

    public static final String LABEL_ENUM = "label_enum";

    public static final String STATE = "state";

    /**
     * Notification ID
     */
    private final static int SERVICE_NOTIFICATION = 1000;

    public CoreServiceWrapperStub mCoreServiceWrapperStub;

    /**
     * CPU manager
     */
    private CpuManager cpuManager = new CpuManager();

    /**
     * Account changed broadcast receiver
     */
    private AccountChangedReceiver accountChangedReceiver = null;

    // --------------------- RCSJTA API -------------------------
    /**
     * Terms API
     */
    private TermsApiService termsApi = new TermsApiService();
    /**
     * Contacts API
     */
    private ContactsServiceImpl contactsApi = null;

    /**
     * Capability API
     */
    private CapabilityServiceImpl capabilityApi = null;

    /**
    * History Api
    */
    private HistoryServiceImpl historyApi = null;

    /**
     * Chat API
     */
    private ChatServiceImpl chatApi = null;

    /**
     * File transfer API
     */
    private FileTransferServiceImpl ftApi = null;

    /**
     * Video sharing API
     */
    private VideoSharingServiceImpl vshApi = null;

    /**
     * Image sharing API
     */
    private ImageSharingServiceImpl ishApi = null;

    /**
     * AOSP Presence Service API
     */
    private IBinder presenceServiceApi = null;

    /**
     * AOSP Options Service API
     */
    private IBinder optionsServiceApi = null;

    /**
     * M: add for auto-rejoin group chat @{
     */
    private NetworkApiService mNetworkConnectivityApi = new NetworkApiService();
    /** @} */

    /**
     * Account changed broadcast receiver
     */
    private HttpsProvisioningSMS reconfSMSReceiver = null;

    /**
     * Geoloc sharing API
     */
    private GeolocSharingServiceImpl gshApi = null;

    private FileUploadServiceImpl mUploadApi = null;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    public static Core mCore; // RCS over Internet APN

    private static String INTENT_ACTION_RCS_STARTED = "com.mediatek.rcse.action.STARTED";

    @Override
    public void onCreate() {

        // Set the terminal version
        TerminalInfo.setProductVersion(AppUtils.getApplicationVersion(this));

        // Start the core
        startCore();

        if (RcsSettings.getInstance().isOP08SupportedByPlatform() &&
                !RcsSettings.getInstance().isTestSim()) {

            if (logger.isActivated()) {
                logger.info("RCS started notification");
            }
            // notify RCS started
            Intent intent = new Intent(INTENT_ACTION_RCS_STARTED);
            getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
        }
    }

    @Override
    public void onDestroy() {
        // Unregister account changed broadcast receiver
        if (accountChangedReceiver != null) {
            try {
                unregisterReceiver(accountChangedReceiver);
            } catch (IllegalArgumentException e) {
                // Nothing to do
            }
        }
         // Unregister SMS receiver for network initiated configuration
        if (!UaServiceManager.getInstance().getService().isAcsAvailable(this)) {
            if (reconfSMSReceiver != null) {
                try {
                    reconfSMSReceiver.unregisterSmsProvisioningReceiver();
                } catch (IllegalArgumentException e) {
                    // Nothing to do
                }
            }
        }

        // Stop the core
        Thread t = new Thread() {
            /**
             * Processing
             */
            public void run() {
                stopCore();
            }
        };
        t.start();
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
                logger.debug("Start RCS core service");
            }

            // Instantiate the settings manager
            RcsSettings.createInstance(getApplicationContext());

            // Instanciate API
            contactsApi = new ContactsServiceImpl();
            capabilityApi = new CapabilityServiceImpl();
            historyApi = new HistoryServiceImpl(getApplicationContext());
            mCoreServiceWrapperStub = new CoreServiceWrapperStub();

            // Set the logger properties
            Logger.activationFlag = RcsSettings.getInstance().isTraceActivated();
            Logger.traceLevel = RcsSettings.getInstance().getTraceLevel();

            // Terminal version
            if (logger.isActivated()) {
                logger.info("RCS stack release is " + TerminalInfo.getProductVersion());
            }
            UaServiceManager.getInstance().startService(this, LauncherUtils.getCurrentUserPhoneId());

            // Instantiate the contacts manager
            ContactsManager.createInstance(getApplicationContext());

            // Instantiate the rich messaging history
            RichMessagingHistory.createInstance(getApplicationContext());

            ContactsBackupHelper.createInstance(getApplicationContext());

            // Create the core
            mCore = Core.createCore(this);

            chatApi = new ChatServiceImpl();
            ftApi = new FileTransferServiceImpl(chatApi);

            // Start the core
            Core.getInstance().startCore();

            // Create multimedia directory on sdcard
            FileFactory.createDirectory(RcsSettings.getInstance().getPhotoRootDirectory());
            FileFactory.createDirectory(RcsSettings.getInstance().getVideoRootDirectory());
            FileFactory.createDirectory(RcsSettings.getInstance().getFileRootDirectory());

            // Init CPU manager
            cpuManager.init();

            // Register account changed event receiver
            /*if (accountChangedReceiver == null) {
                accountChangedReceiver = new AccountChangedReceiver();

                // Register account changed broadcast receiver after a timeout of 2s (This is not done immediately, as we do not want to catch
                // the removal of the account (creating and removing accounts is done asynchronously). We can reasonably assume that no
                // RCS account deletion will be done by user during this amount of time, as he just started his service.
                Handler handler = new Handler();
                handler.postDelayed(
                        new Runnable() {
                            public void run() {
                                registerReceiver(accountChangedReceiver, new IntentFilter(
                                        "android.accounts.LOGIN_ACCOUNTS_CHANGED"));
                            }},
                        2000);
            }*/

            // Register SMS receiver for network initiated configuration


            if (!UaServiceManager.getInstance().getService().isAcsAvailable(this)) {
                if (reconfSMSReceiver == null) {
                    reconfSMSReceiver = new HttpsProvisioningSMS(this);
                    reconfSMSReceiver.registerSmsProvisioningReceiver(Integer.toString(HttpsProvisioningUtils.DEFAULT_SMS_PORT), null, null, null);
                }
            }



            // Show a first notification
            addRcsServiceNotification(false, getString(R.string.rcs_core_loaded),RCS_CORE_LOADED);

            if (logger.isActivated()) {
                logger.info("RCS core service started with success");
            }
        } catch(Exception e) {
            // Unexpected error
            if (logger.isActivated()) {
                logger.error("Can't instanciate the RCS core service", e);
            }

            // Show error in notification bar
            addRcsServiceNotification(false, getString(R.string.rcs_core_failed), RCS_CORE_FAILED);

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
            logger.debug("Stop RCS core service");
        }
        addRcsServiceNotification(false, getString(R.string.rcs_core_ims_try_disconnect), RCS_CORE_IMS_TRY_DISCONNECT);

        // Close APIs
        if(contactsApi != null) {
        contactsApi.close();
        }
        if(capabilityApi != null) {
        capabilityApi.close();
        }
        if(ftApi != null) {
        ftApi.close();
        }
        if(chatApi != null) {
        chatApi.close();
        }
       /* if(ishApi != null) {
        ishApi.close();
        }*/
        if(gshApi != null) {
        gshApi.close();
        }
        if(historyApi != null) {
            historyApi.close();
        }
    /*    if(vshApi != null) {
        vshApi.close();
        }*/

        // Terminate the core in background
        Core.terminateCore();

        UaServiceManager.getInstance().stopService();

        // Close CPU manager
        if(cpuManager != null) {
        cpuManager.close();
        }

        if (logger.isActivated()) {
            logger.info("RCS core service stopped with success");
        }
    }

 public class CoreServiceWrapperStub extends ICoreServiceWrapper.Stub {

        @Override
        public IBinder getChatServiceBinder() {
            return chatApi;
        }

        @Override
        public IBinder getFileTransferServiceBinder() {
            return ftApi;
        }

        @Override
        public IBinder getCapabilitiesServiceBinder() {
            return capabilityApi;
        }

        @Override
        public IBinder getContactsServiceBinder() {
            return contactsApi;
        }

        @Override
        public IBinder getGeolocServiceBinder() {
            return gshApi;
        }

        @Override
        public IBinder getVideoSharingServiceBinder() {
            return vshApi;
        }

        @Override
        public IBinder getImageSharingServiceBinder() {
            return ishApi;
        }

        @Override
        public IBinder getNetworkConnectivityApiBinder() {
            return mNetworkConnectivityApi;
        }

        @Override
        public IBinder getHistoryServiceBinder() {
            return historyApi;
        }

        @Override
        public IBinder getUploadServiceBinder() {
            return mUploadApi;
        }

        @Override
        public IBinder getAospPresenceServiceBinder() {
            return presenceServiceApi;
        }

        @Override
        public IBinder getAospOptionsServiceBinder() {
            return optionsServiceApi;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
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
        Intent intent = new Intent(Intents.Client.ACTION_VIEW_SETTINGS);
        intent.putExtra(STATE, state);
        intent.putExtra(LABEL, label);
        intent.putExtra(LABEL_ENUM,labelEnum);
        AndroidFactory.getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
        CURRENT_STATE= labelEnum;
        RcsSettings.getInstance().setServiceCurrentState(labelEnum);

        /*intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent contentIntent = PendingIntent.getActivity(AndroidFactory.getApplicationContext(), 0, intent, 0);
        int iconId;
        if (state) {
            iconId  = R.drawable.rcs_core_notif_on_icon;
        } else {
            iconId  = R.drawable.rcs_core_notif_off_icon;
        }
        Notification notif = new Notification(iconId, "", System.currentTimeMillis());
        notif.flags = Notification.FLAG_NO_CLEAR | Notification.FLAG_FOREGROUND_SERVICE;
        notif.setLatestEventInfo(AndroidFactory.getApplicationContext(),
                AndroidFactory.getApplicationContext().getString(R.string.rcs_core_rcs_notification_title),
                label, contentIntent);

        // Send notification
        NotificationManager notificationManager = (NotificationManager)AndroidFactory.getApplicationContext().getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(SERVICE_NOTIFICATION, notif);*/
    }

    /*---------------------------- CORE EVENTS ---------------------------*/

    /**
     * Notify registration status to API
     *
     * @param status Status
     */
    private void notifyRegistrationStatusToApi(boolean status) {
        if (capabilityApi != null) {
            capabilityApi.notifyRegistrationEvent(status);
        }
        if (chatApi != null) {
            chatApi.notifyRegistrationEvent(status);
        }
        if (ftApi != null) {
            ftApi.notifyRegistrationEvent(status);
        }
      /*  if (vshApi != null) {
            vshApi.notifyRegistrationEvent(status);
        }*/
/*        if (ishApi != null) {
            ishApi.notifyRegistrationEvent(status);
        }*/
        if (gshApi != null) {
            gshApi.notifyRegistrationEvent(status);
        }
    }

    /**
     * Core layer has been started
     */
    public void handleCoreLayerStarted() {
        if (logger.isActivated()) {
            logger.debug("Handle event core started");
        }

        // Display a notification
        addRcsServiceNotification(false, getString(R.string.rcs_core_started),RCS_CORE_STARTED);

        // Send service up intent
        Intent intent = new Intent(JoynService.ACTION_RCS_SERVICE_UP);
        getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
    }

    /**
     * Core layer has been terminated
     */
    public void handleCoreLayerStopped() {
        // Display a notification
        if (logger.isActivated()) {
            logger.debug("Handle event core terminated");
        }
        addRcsServiceNotification(false, getString(R.string.rcs_core_stopped), RCS_CORE_STOPPED);
    }

    /**
     * Handle "registration successful" event
     *
     * @param registered Registration flag
     */
    public void handleRegistrationSuccessful() {

        if (logger.isActivated()) {
            logger.debug("Handle event registration ok");
        }

        if (!RcsSettings.getInstance().isServiceActivated()) {
            logger.debug("ignore rcs_core_ims_connected");
            addRcsServiceNotification(true, getString(R.string.rcs_core_stopped_ims_registered), RCS_CORE_IMS_CONNECTED);
            return;
        }

        RcsSettings.getInstance().setRegistrationState(true);
        // Display a notification
        addRcsServiceNotification(true, getString(R.string.rcs_core_ims_connected), RCS_CORE_IMS_CONNECTED);
         // Send registration intent
        Intent intent = new Intent(Intents.Client.SERVICE_UP);
        intent.putExtra("status", true);
        getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
        // Notify APIs
        notifyRegistrationStatusToApi(true);
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

        if (!RcsSettings.getInstance().isServiceActivated()) {
            logger.debug("ignore rcs_core_ims_try_connection");
            return;
        }
        // Display a notification
        addRcsServiceNotification(false, getString(R.string.rcs_core_ims_try_connection), RCS_CORE_IMS_TRY_CONNECTION);
    }



    /**
     * Handle "try registration" event
     */
    public void handleTryDeregister() {
        if (logger.isActivated()) {
            logger.debug("Handle event try deregistration");
        }

        // Display a notification
        addRcsServiceNotification(false, getString(R.string.rcs_core_ims_try_disconnect), RCS_CORE_IMS_TRY_DISCONNECT);
    }
    /** @*/

    /**
     * Handle "registration failed" event
     *
     * @param error IMS error
        */
    public void handleRegistrationFailed(ImsError error) {
        if (logger.isActivated()) {
            logger.debug("Handle event registration failed");
        }
        RcsSettings.getInstance().setRegistrationState(false);
        // Display a notification
        addRcsServiceNotification(false, getString(R.string.rcs_core_ims_connection_failed), RCS_CORE_IMS_CONNECTION_FAILED);
        Intent intent = new Intent(Intents.Client.SERVICE_UP);
        intent.putExtra("status", false);
        getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
        // Notify APIs
        notifyRegistrationStatusToApi(false);
    }

    /**
     * Handle "registration terminated" event
     */
    public void handleRegistrationTerminated() {

        logger.debug("Handle event registration terminated");

        RcsSettings.getInstance().setRegistrationState(false);
        addRcsServiceNotification(false, getString(R.string.rcs_core_ims_disconnected), RCS_CORE_IMS_DISCONNECTED);

        // Notify APIs
        notifyRegistrationStatusToApi(false);
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
    public void handlePresenceInfoNotification(String contact, PidfDocument presence) {
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

          }else{
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
     * New content sharing transfer invitation
     *
     * @param session Content sharing transfer invitation
     */
    public void handleContentSharingTransferInvitation(ImageTransferSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event content sharing transfer invitation");
        }

        // Broadcast the invitation
        ishApi.receiveImageSharingInvitation(session);
    }

    /**
     * New content sharing transfer invitation
     *
     * @param session Content sharing transfer invitation
     */
    public void handleContentSharingTransferInvitation(GeolocTransferSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event content sharing transfer invitation");
        }

        // Broadcast the invitation
        gshApi.receiveGeolocSharingInvitation(session);
    }

    /**
     * New content sharing streaming invitation
     *
     * @param session CSh session
     */
    public void handleContentSharingStreamingInvitation(VideoStreamingSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event content sharing streaming invitation");
        }

        // Broadcast the invitation
      //  vshApi.receiveVideoSharingInvitation(session);
    }

    /**
     * A new file transfer invitation has been received
     *
     * @param fileSharingSession File transfer session
     * @param isGroup Is group file transfer
     */
    public void handleFileTransferInvitation(FileSharingSession fileSharingSession, boolean isGroup) {
        if (logger.isActivated()) {
            logger.debug("Handle event file transfer invitation");
        }

        // Broadcast the invitation
        ftApi.receiveFileTransferInvitation(fileSharingSession, isGroup);
    }

    /**
     * A new file transfer invitation has been received
     *
     * @param fileSharingSession File transfer session
     * @param isGroup Is group file transfer
     */
    public void handleResumeFileTransferInvitation(FileSharingSession fileSharingSession, boolean isGroup, String ftId) {
        if (logger.isActivated()) {
            logger.debug("Handle event file transfer invitation");
        }

        // Broadcast the invitation
        ftApi.receiveResumeFileTransferInvitation(fileSharingSession, isGroup, ftId);
    }

    /**
     * A new file transfer invitation has been received
     *
     * @param fileSharingSession File transfer session
     * @param isGroup Is group file transfer
     */
    public void handleHttpFileTransferInvitation(String remote, FileSharingSession fileSharingSession, boolean isGroup) {
        if (logger.isActivated()) {
            logger.debug("Handle event httpfile transfer invitation: " + remote);
        }

        // Broadcast the invitation
        ftApi.receiveHttpFileTransferInvitation(remote,fileSharingSession, isGroup);
    }

    /**
     * A new file transfer invitation has been received
     *
     * @param session File transfer session
     */
    public void handle1to1FileTransferInvitation(FileSharingSession fileSharingSession, OneOneChatSession one2oneChatSession) {
        if (logger.isActivated()) {
            logger.debug("Handle event file transfer invitation");
        }

        // Broadcast the invitation
        ftApi.receiveFileTransferInvitation(fileSharingSession, one2oneChatSession);
    }

    /**
     * A new file transfer invitation has been received
     *
     * @param session File transfer session
     * @param groupChatSession the created chat session (group)
     */
    public void handleGroupFileTransferInvitation(FileSharingSession session, TerminatingAdhocGroupChatSession groupChatSession) {
        if (logger.isActivated()) {
            logger.debug("Handle event group file transfer invitation");
        }

        // Broadcast the invitation
        ftApi.receiveFileTransferInvitation(session, groupChatSession);
    }

    /**
     * New one-to-one chat session invitation
     *
     * @param session Chat session
     */
    public void handleOneOneChatSessionInvitation(TerminatingOne2OneChatSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event receive 1-1 chat session invitation");
        }

        // Broadcast the invitation
        chatApi.receiveOneOneChatInvitation(session);
    }

    /**
     * New one-to-one chat session invitation
     *
     * @param session Chat session
     */
    public void handleStandaloneChatSessionInvitation(TerminatingStandaloneChatSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event receive 1-1 chat session invitation");
        }

        // Broadcast the invitation
        chatApi.receiveStandaloneChatInvitation(session);
    }

    /**
     * New ad-hoc group chat session invitation
     *
     * @param session Chat session
     */
    public void handleAdhocGroupChatSessionInvitation(TerminatingAdhocGroupChatSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event receive ad-hoc group chat session invitation");
        }

        // Broadcast the invitation
        chatApi.receiveGroupChatInvitation(session);
    }

      /**
     * One-to-one chat session extended to a group chat session
     *
     * @param groupSession Group chat session
     * @param oneoneSession 1-1 chat session
     */
    public void handleOneOneChatSessionExtended(GroupChatSession groupSession, OneOneChatSession oneoneSession) {
        if (logger.isActivated()) {
            logger.debug("Handle event 1-1 chat session extended");
        }

        // Broadcast the event
        chatApi.extendOneOneChatSession(groupSession, oneoneSession);
    }

    /**
     * Store and Forward messages session invitation
     *
     * @param session Chat session
     */
    public void handleStoreAndForwardMsgSessionInvitation(TerminatingStoreAndForwardMsgSession session) {
        if (logger.isActivated()) {
            logger.debug("Handle event S&F messages session invitation");
        }

        // Broadcast the invitation
        chatApi.receiveOneOneChatInvitation(session);
    }

    /**
     * New pager message received
     *
     * @param message Message received
     */
    public void handleReceivePagerMessage(InstantMessage message, String conversationId) {
        if (logger.isActivated()) {
            logger.debug("Handle receive pager message");
        }

        // Notify listeners
        chatApi.receivePagerMessage(message, conversationId);
    }

    /**
     * New message delivery status
     *
     * @param contact Contact
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String contact, String msgId, String status, Date date) {
        if (logger.isActivated()) {
            logger.debug("Handle message delivery status");
        }

        // Notify listeners
        chatApi.receiveMessageDeliveryStatus(contact, msgId, status, date);
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

        // Notify listeners
        //chatApi.receiveMessageDeliveryStatus(contact, msgId, status);
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

        // Notify listeners
        ftApi.handleFileDeliveryStatus(ftSessionId, status,contact);

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


        termsApi.receiveTermsRequest(remote, id, type, pin, subject, text, acceptButtonLabel, rejectButtonLabel, timeout);
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


        termsApi.receiveTermsAck(remote, id, status, subject, text);
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


        termsApi.receiveUserNotification(remote, id, subject, text, okButtonLabel);
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

}
