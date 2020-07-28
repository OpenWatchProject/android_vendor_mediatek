/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.rcse.service;

import java.util.ArrayList;
import java.util.List;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import com.mediatek.rcse.activities.ConfigMessageActicity;
import com.mediatek.rcse.api.ContactsApiIntents;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.terms.TermsApiIntents;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.service.binder.IRemoteBlockingRequest;
import com.mediatek.rcse.service.binder.WindowBinder;
import com.mediatek.rcse.plugin.message.IRemoteWindowBinder;
//import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.service.StartService;
import com.mediatek.rcse.service.IApiServiceWrapper;
import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.JoynServiceRegistrationListener;
import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.ft.FileTransferIntent;
import com.gsma.services.rcs.sharing.image.ImageSharingIntent;
import com.gsma.services.rcs.sharing.video.VideoSharingIntent;

import com.mediatek.rcs.R;

//This Service will provide the remote API to the applications.
/**
 * The Class ApiService.
 */
public class ApiService extends Service {
    /**
     * The Constant TAG.
     */
    public static final String TAG = "ApiService";
    /**
     * The m receiver.
     */
    private ApiReceiver mReceiver = null;
    /**
     * The m registration status stub.
     */
    private RegistrationStatusStub mRegistrationStatusStub = null;
    // private CapabilitiesStub mCapabilitiesStub = null;
    /**
     * The m blocking request stub.
     */
    private BlockingRequestStub mBlockingRequestStub = null;
    /**
     * The m window binder.
     */
    private WindowBinder mWindowBinder = null;
    public ApiServiceWrapperStub mApiServiceWrapper = null;
    // Low memory broadcast action
    /**
     * The Constant MEMORY_LOW_ACTION.
     */
    private static final String MEMORY_LOW_ACTION = "ACTION_DEVICE_STORAGE_LOW";
    // Memory okay broadcast action
    /**
     * The Constant MEMORY_OK_ACTION.
     */
    private static final String MEMORY_OK_ACTION = "ACTION_DEVICE_STORAGE_OK";
    /**
     * The Constant CORE_CONFIGURATION_STATUS.
     */
    public static final String CORE_CONFIGURATION_STATUS = "status";

    /**
     * Notification ID
     */
    private final static int SERVICE_NOTIFICATION = 1000;
    /**
     * The reg listener.
     */
    public static RegistrationListener sRegListener = null;

    /**
     * The Constant MEMORY_OK_ACTION.
     */
    private static final String IMS_UP_ACTION = "com.android.ims.IMS_SERVICE_UP";


    /**
     * The Constant MEMORY_OK_ACTION.
     */
    private static final String IMS_DOWN_ACTION = "com.android.ims.IMS_SERVICE_DOWN";

    /**
     * The Constant MEMORY_OK_ACTION.
     */
    public static final String MMS_FALLBACK_ACTION = "com.android.mms.MMS_FALLBACK_ACTION";

    /* (non-Javadoc)
     * @see android.app.Service#onCreate()
     */
    @Override
    public void onCreate() {
        Logger.d(TAG, "ApiService onCreate() entry");
        mReceiver = new ApiReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
        intentFilter.addAction(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE);
        intentFilter.addAction(GroupChatIntent.ACTION_NEW_GROUP_CHAT_MESSAGE);
        intentFilter.addAction(GroupChatIntent.ACTION_NEW_INVITATION);
        intentFilter.addAction(ApiManager.SERVICE_CONNECTED_EVENT);
        intentFilter.addAction(Intents.Client.SERVICE_UP);
        intentFilter
                .addAction(FileTransferIntent.ACTION_NEW_INVITATION);
        intentFilter
                .addAction(ContactsApiIntents.CONTACT_BLOCK_REQUEST);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);
        intentFilter.addAction(StartService.CONFIGURATION_STATUS);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_ACK);
        intentFilter
                .addAction(TermsApiIntents.TERMS_SIP_USER_NOTIFICATION);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_REQUEST);
        intentFilter.addAction(PluginUtils.ACTION_DB_CHANGE);
        intentFilter.addAction(IMS_UP_ACTION);
        intentFilter.addAction(IMS_DOWN_ACTION);
        intentFilter.addAction(MMS_FALLBACK_ACTION);
        intentFilter.addAction(Intents.Client.ACTION_VIEW_SETTINGS);
        this.registerReceiver(mReceiver, intentFilter);
        registerSdCardReceiver();
        mRegistrationStatusStub = new RegistrationStatusStub();
        // mCapabilitiesStub = new CapabilitiesStub(this);
        mBlockingRequestStub = new BlockingRequestStub(
                getApplicationContext());
        mWindowBinder = new WindowBinder();
        mApiServiceWrapper = new ApiServiceWrapperStub();
        // Instantiate the contacts manager
        //ContactsManager.createInstance(getApplicationContext());
        // Keep a initial IM blocked contacts list to local copy
        if(ApiManager.getInstance() != null &&
                ApiManager.getInstance().getContactsApi() != null){
            try {
                ApiManager.getInstance().getContactsApi()
                        .loadImBlockedContactsToLocal();
            } catch (Exception e) {
                // TODO: handle exception
            }
        }
        if(RcsSettings.getInstance() != null) {
            boolean isRegistered = RcsSettings.getInstance().isServiceRegistered();
            Logger.d(TAG, "ApiService onCreate(), isRegistered = " + isRegistered);
            if (isRegistered) {
                handleRegistrationStatus(isRegistered);
            }
        }
        Logger.v(TAG, "ApiService onCreate() exit");
    }


    public class ApiServiceWrapperStub extends IApiServiceWrapper.Stub {

        @Override
        public IBinder getRemoteWindowBinder() {
            Logger.v(TAG, "IApiServiceWrapperStub getRemoteWindowBinder() entry" + mWindowBinder);
            return mWindowBinder;
        }
        @Override
        public IBinder getRegistrationStatusBinder() {
            Logger.v(TAG, "IApiServiceWrapperStub getRegistrationStatusBinder() entry");
            return mRegistrationStatusStub;
        }

        @Override
        public IBinder getBlockingBinder() {
            Logger.v(TAG, "IApiServiceWrapperStub getBlockingBinder() entry");
            return mBlockingRequestStub;
        }
    }

    /**
     * Register sd card receiver.
     */
    private void registerSdCardReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addDataScheme("file");
        this.registerReceiver(new SdcardReceiver(), intentFilter);
    }
    /* (non-Javadoc)
     * @see android.app.Service#onDestroy()
     */
    @Override
    public void onDestroy() {
        Logger.v(TAG, "ApiService onDestroy() entry");
        super.onDestroy();
        if (null != mReceiver) {
            this.unregisterReceiver(mReceiver);
        } else {
            Logger.e(TAG, "onDestroy() mReceiver is null");
        }
        /*try {
            ApiManager.getInstance().getChatApi()
                    .removeServiceRegistrationListener(sRegListener);
        } catch (JoynServiceException e) {
            e.printStackTrace();
        }*/
        Logger.v(TAG, "ApiService onDestroy() exit");
    }
    /* (non-Javadoc)
     * @see android.app.Service#onBind(android.content.Intent)
     */
    @Override
    public IBinder onBind(Intent intent) {
          Logger.v(TAG, "ApiService onBind() exit");
          return mApiServiceWrapper;
    }

    /**
     * The listener interface for receiving registration events.
     * The class that is interested in processing a registration
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's <code>addRegistrationListener</code> method. When
     * the registration event occurs, that object's appropriate
     * method is invoked.
     *
     * @see RegistrationEvent
     */
    private class RegistrationListener extends
            JoynServiceRegistrationListener {
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceRegistrationListener#onServiceRegistered()
         */
        @Override
        public void onServiceRegistered() {
            Logger.d(TAG, "onServiceRegistered entry");
            handleRegistrationStatus(true);
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceRegistrationListener
         * #onServiceUnregistered()
         */
        @Override
        public void onServiceUnregistered() {
            Logger.d(TAG, "onServiceUnregistered entry");
            handleRegistrationStatus(false);
        }
    }

    // This receiver will handle sdcard mount and unmount broadcast
    /**
     * The Class SdcardReceiver.
     */
    private static class SdcardReceiver extends BroadcastReceiver {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "SdcardReceiver";

        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#onReceive
         * (android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(final Context context,
                final Intent intent) {
            Logger.d(TAG, "onReceive() SdcardReceiver entry");
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    String action = intent.getAction();
                    Logger.d(TAG,
                            "doInBackground() SdcardReceiver action is "
                                    + action);
                    if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                        Logger.d(TAG,
                                "doInBackground() SdcardReceiver() sdcard mounted");
                        boolean ftCapability = FileTransferCapabilityManager
                                .isFileTransferCapabilitySupported();
                        FileTransferCapabilityManager
                                .setFileTransferCapability(context,
                                        ftCapability);
                    } else if (Intent.ACTION_MEDIA_UNMOUNTED
                            .equals(action)) {
                        Logger.d(TAG,
                                "doInBackground() SdcardReceiver() sdcard unmounted");
                        FileTransferCapabilityManager
                                .setFileTransferCapability(context,
                                        false);
                    }
                    return null;
                }
            } .execute();
            Logger.d(TAG, "onReceive() SdcardReceiver exit");
        }
    }


    /**
     * show registration icon.
     *
     * @param state state
     */
    private void showRegistrationIcon(boolean state, String label, int labelEnum) {
        Logger.d(TAG, "showRegistrationIcon() state:" + state + ",label:" + label + ",LabelEnum:" + labelEnum);

        Context context = MediatekFactory.getApplicationContext();
        Intent intent = new Intent(Intents.Client.ACTION_VIEW_SETTINGS);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent contentIntent = PendingIntent.getActivity(context, 0, intent, 0);
        int iconId;
        if (state) {
            iconId  = R.drawable.rcs_core_notif_on_icon;
        } else {
            iconId  = R.drawable.rcs_core_notif_off_icon;
        }

        NotificationChannel channel = new NotificationChannel("RCS", "RCS", NotificationManager.IMPORTANCE_DEFAULT);
        NotificationManager nm = (NotificationManager)context.getSystemService(
                Context.NOTIFICATION_SERVICE);

        nm.createNotificationChannel(channel);

        Notification notification = new Notification.Builder(context, "RCS")
                                    .setAutoCancel(false)
                                    .setContentTitle(MediatekFactory.getApplicationContext().getString(R.string.rcs_core_rcs_notification_title))
                                    .setContentText(label)
                                    .setContentIntent(contentIntent)
                                    .setSmallIcon(iconId)
                                    .setWhen(System.currentTimeMillis())
                                    .setOngoing(true)
                                    .build();

        nm.notify(SERVICE_NOTIFICATION, notification);
    }

    /**
     * Handle registration status.
     *
     * @param status the status
     */
    private void handleRegistrationStatus(boolean status) {
        Logger.d(TAG, "handleRegistrationStatus() entry");
        if (!status) {
            if (RcsNotification.getInstance() != null) {
                RcsNotification.getInstance().mIsStoreAndForwardMessageNotified = false;
            } else {
                Logger.d(TAG,
                        "handleRegistrationStatus, RcsNotification.getInstance() is null!");
            }
            Logger.d(
                    TAG,
                    "handleRegistrationStatus, status is false, "
                            + "set sIsStoreAndForwardMessageNotified to false!");
        } else {
            Logger.d(TAG, "handleRegistrationStatus, status is true!");
            boolean ftCapability = FileTransferCapabilityManager
                    .isFileTransferCapabilitySupported();
            FileTransferCapabilityManager.setFileTransferCapability(
                    getApplicationContext(), ftCapability);
        }
        Logger.i(TAG, "handleRegistrationStatus() the status is "
                + status);
        mRegistrationStatusStub.notifyRegistrationStatus(status);
        // mCapabilitiesStub.onStatusChanged(status);
    }

    // This receiver will handle some RCS-e related broadcasts.
    /**
     * The Class ApiReceiver.
     */
    private class ApiReceiver extends BroadcastReceiver {
        /**
         * The Constant TAG.
         */
        public static final String TAG = "ApiReceiver";
        /**
         * The Constant KEY_STATUS.
         */
        public static final String KEY_STATUS = "status";
        /**
         * The Constant KEY_CONTACT.
         */
        public static final String KEY_CONTACT = "contact";
        /**
         * The Constant KEY_CAPABILITIES.
         */
        public static final String KEY_CAPABILITIES = "capabilities";

        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#onReceive
         * (android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(final Context context,
                final Intent intent) {

            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                        asyncOnReceive(context, intent);
                    }
                }
            );
        }
        /**
         * Async on receive.
         *
         * @param context the context
         * @param intent the intent
         */
        private void asyncOnReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Logger.d(TAG, "asyncOnReceive() entry, the action is "
                    + action);
            if (ContactsApiIntents.CONTACT_BLOCK_REQUEST
                    .equals(action)) {
                handleBlockRequest(context, intent);
            } else if (OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE
                    .equalsIgnoreCase(action)
                    || OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE
                    .equalsIgnoreCase(action)
                    || GroupChatIntent.ACTION_NEW_INVITATION
                            .equalsIgnoreCase(action)
                    || FileTransferIntent.ACTION_NEW_INVITATION
                            .equalsIgnoreCase(action)
                    || ImageSharingIntent.ACTION_NEW_INVITATION
                            .equalsIgnoreCase(action)
                    || VideoSharingIntent.ACTION_NEW_INVITATION
                            .equalsIgnoreCase(action)) {
                RcsNotification.handleInvitation(context, intent);
            } else if (GroupChatIntent.ACTION_NEW_GROUP_CHAT_MESSAGE
                    .equalsIgnoreCase(action)) {
                RcsNotification.handleNewMessage(context, intent);
            } else if (action.equals(MEMORY_LOW_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability = ExchangeMyCapability
                        .getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG,
                            "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE,
                        false);
            } else if (action.equals(MEMORY_OK_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability = ExchangeMyCapability
                        .getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG,
                            "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE,
                        true);
            } else if (StartService.CONFIGURATION_STATUS
                    .equals(action)) {
                boolean status = intent.getBooleanExtra(
                        CORE_CONFIGURATION_STATUS, true);
                handleConfigurationStatus(status);
            } else if (IMS_UP_ACTION
                    .equals(action)) {
              ApiManager.getInstance().onIMSStatusChanged(true);
            } else if (IMS_DOWN_ACTION
                    .equals(action)) {
                ApiManager.getInstance().onIMSStatusChanged(false);
            }else if (Intents.Client.ACTION_VIEW_SETTINGS
                    .equals(action)) {
                //show registration icon
                boolean state = intent.getBooleanExtra("state",
                        false);
                handleRegistrationStatus(state);
                String label = intent.getStringExtra("label");
                int labelEnum = intent.getIntExtra("label_enum", 0);
                showRegistrationIcon(state, label, labelEnum);
            } else if (PluginUtils.ACTION_DB_CHANGE.equals(action)) {
                // need to clear chat history
                ControllerImpl controller = ControllerImpl
                        .getInstance();
                if (controller != null) {
                    Message controllerMessage = controller
                            .obtainMessage(
                                    ChatController.EVENT_CLEAR_ALL_CHAT_HISTORY_MEMORY_ONLY,
                                    null, null);
                    controllerMessage.sendToTarget();
                }
            } else if (Intents.Client.SERVICE_UP.equals(action)) {
                if (!JoynService.isServiceStarted(context)) {
                    Logger.d(TAG,
                            "RcsCoreService is not started yet, so services won't connect ");
                }
                boolean status = intent.getBooleanExtra("status",
                        false);
                handleRegistrationStatus(status);
            } else if (ApiManager.SERVICE_CONNECTED_EVENT
                    .equals(action)) {
                boolean status = intent.getBooleanExtra(
                        "registrationStatus", true);
                Logger.d(TAG,
                        "SERVICE_CONNECTED_EVENT entry , status "
                                + status);
                sRegListener = new RegistrationListener();
                /*
                 * try { //ApiManager.getInstance().getChatApi().
                 * addServiceRegistrationListener(regListener); } catch
                 * (JoynServiceException e) { e.printStackTrace(); }
                 */
                // handleRegistrationStatus(status);
            } else if (TermsApiIntents.TERMS_SIP_ACK
                    .equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_REQUEST
                            .equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_USER_NOTIFICATION
                            .equalsIgnoreCase(action)) {
                Logger.v(TAG,
                        "asyncOnReceive() Handling Terms Request "
                                + action);
                intent.setClass(getApplicationContext(),
                        ConfigMessageActicity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            } else if (MMS_FALLBACK_ACTION
                    .equals(action)) {
                Logger.v(TAG, "asyncOnReceive() MMS_FALLBACK_ACTION entry");
                String address = intent.getStringExtra("contact");
                String text = intent.getStringExtra("send_by_sms_text");
                Participant fromSessionParticipant = new Participant(address,
                        address);
                List<Participant> participantList = new ArrayList<Participant>();
                participantList.add(fromSessionParticipant);
                IChatManager modelImpl = ModelImpl.getInstance();
                Logger.v(TAG, "asyncOnReceive() MMS_FALLBACK_ACTION address & text" + address + text);
                if (modelImpl != null) {
                  One2OneChat chat = (One2OneChat) ModelImpl.getInstance()
                            .addChat(participantList, null, null);
                    chat.sendMessage(text, 0);
                } else {
                    Logger.e(TAG, "asyncOnReceive getChat(), modelImpl is null");
                }
            } else {
                Logger.e(TAG,
                        "asyncOnReceive() unknown action! The action is "
                                + action);
            }
            Logger.v(TAG, "asyncOnReceive() exit");
        }
        /**
         * Handle block request.
         *
         * @param context the context
         * @param intent the intent
         */
        private void handleBlockRequest(Context context, Intent intent) {
            Logger.v(TAG, "handleBlockRequest() entry");
            Bundle data = intent.getExtras();
            if (null != data) {
                String number = data.getString("number");
                /*ContactsManager instance = ContactsManager
                        .getInstance();*/
                if (ApiManager.getInstance().getContactsApi() != null) {
                    try {
                        ApiManager.getInstance().getContactsApi()
                                .setImBlockedForContact(number, true);
                    } catch (Exception e) {
                        // TODO: handle exception
                    }
                }
            } else {
                Logger.e(TAG,
                        "handleBlockRequest() the data is null!");
            }
            Logger.v(TAG, "handleBlockRequest() exit");
        }
        /**
         * Handle configuration status.
         *
         * @param status the status
         */
        private void handleConfigurationStatus(boolean status) {
            Logger.d(TAG,
                    "handleConfigurationStatus() entry the status is "
                            + status);
            RcseComponentController rcseComponentController = ApiManager
                    .getInstance().getRcseComponentController();
            Logger.d(TAG,
                    "handleConfigurationStatus() : rcseComponentController "
                            + rcseComponentController);
            if (rcseComponentController != null) {
                if (Logger.IS_DEBUG) {
                    Logger.d(TAG,
                            "handleConfigurationStatus() it is debug version");
                } else {
                    rcseComponentController
                            .onConfigurationStatusChanged(status);
                }
            } else {
                Logger.e(
                        TAG,
                        "handleConfigurationStatus()) "
                                + "ApiManager.getInstance().getRcseComponentController() is null");
            }
        }
    }
}

/**
 * @author MTK33296
 *
 */
class RegistrationStatusStub extends IRegistrationStatus.Stub {
    public static final String TAG = "RegistrationStatusStub";
    private boolean mIsRcseRegistered = false;
    /**.
     * List of listeners
     */
    private RemoteCallbackList<IRegistrationStatusRemoteListener> mListeners =
            new RemoteCallbackList<IRegistrationStatusRemoteListener>();
    /**.
     * Lock used for synchronization
     */
    private Object mLock = new Object();

    /**
     * @param listener
     * @throws RemoteException
     */
    @Override
    public void addRegistrationStatusListener(
            IRegistrationStatusRemoteListener listener)
            throws RemoteException {
        boolean result = mListeners.register(listener);
        Logger.i(TAG,
                "addRegistrationStatusListener() The result is "
                        + result);
    }
    public void notifyRegistrationStatus(boolean status) {
        Logger.v(TAG,
                "notifyRegistrationStatus() entry: The status is "
                        + status);
        // update the registration status
        mIsRcseRegistered = status;
        synchronized (mLock) {
            // Notify status listeners
            final int n = mListeners.beginBroadcast();
            for (int i = 0; i < n; i++) {
                try {
                    mListeners.getBroadcastItem(i).onStatusChanged(
                            status);
                } catch (RemoteException e) {
                    Logger.w(
                            TAG,
                            "notifyRegistrationStatus() Failed to notify" +
                            " target listener, the index is "
                                    + i);
                }
            }
            mListeners.finishBroadcast();
        }
        Logger.v(TAG, "notifyRegistrationStatus() exit");
    }
    @Override
    public boolean isRegistered() throws RemoteException {
        Logger.d(
                TAG,
                "isRegistered(), call ApiService:isRegistered()! mIsRcseRegistered = "
                        + mIsRcseRegistered);
        return mIsRcseRegistered;
    }
}

/**
 * @author MTK33296
 *
 */
class BlockingRequestStub extends IRemoteBlockingRequest.Stub {
    public static final String TAG = "BlockingRequestStub";
    private Context mContext = null;

    BlockingRequestStub(Context context) {
        mContext = context;
    }
    /**
     * @param contact
     * @param status
     * @return
     */
    public boolean blockContact(String contact, boolean status) {
        Logger.v(TAG, "blockContact() entry");
        //ContactsManager instance = ContactsManager.getInstance();
        if (ApiManager.getInstance().getContactsApi() != null) {
            try {
                ApiManager.getInstance().getContactsApi()
                        .setImBlockedForContact(contact, status);
            } catch (Exception e) {
                return false;
            }
            return true;
        } else {
            Logger.e(TAG, "blockContact() instance is null!");
        }
        Logger.v(TAG, "blockContact() exit");
        return false;
    }
    /**
     * @param contact
     * @return
     */
    public boolean getBlockedStatus(String contact) {
        Logger.v(TAG, "getBlockedStatus() entry");
        //ContactsManager instance = ContactsManager.getInstance();
        if (ApiManager.getInstance().getContactsApi() != null) {

            try {
                boolean isBlocked = ApiManager.getInstance().getContactsApi()
                        .isImBlockedForContact(contact);
                return isBlocked;
            } catch (Exception e) {
                e.printStackTrace();
            }


        } else {
            Logger.e(TAG, "getBlockedStatus() instance is null!");
        }
        return false;
    }
}
