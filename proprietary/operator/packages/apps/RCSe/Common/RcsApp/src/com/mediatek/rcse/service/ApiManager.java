/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

import android.content.Context;
import android.os.AsyncTask;
import android.os.Message;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.NetworkConnectivityApi;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.terms.TermsApi;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.provider.MediatekRichProviderHelper;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcse.settings.RcsSettings;

import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceListener;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.contact.ContactService;
import com.gsma.services.rcs.ft.FileTransferService;
import com.gsma.services.rcs.sharing.geoloc.GeolocSharingService;

/**
 * This class manages the APIs, providing a convenient way for API invocations.
 */
public class ApiManager {
    public static final String TAG = "ApiManager";

    private static ApiManager sInstance = null;

    private RegistrationApi mRegistrationApi = null;
    private CapabilityService mCapabilitiesApi = null;
    private TermsApi mTermsApi = null;
    private boolean mImsConnected = false;
    private ChatService mChatApi = null;
    private ContactService mContactsApi = null;
    private FileTransferService mFileTransferApi = null;
    private GeolocSharingService mGeolocSharingApi = null;
    private NetworkConnectivityApi mNetworkConnectivityApi = null;
    private Context mContext = null;
    private static long sMaxFileSize = 0;
    private static long sWarningFileSize = 0;
    private RcseComponentController mRcseComponentController = null;
    public static final String SERVICE_CONNECTED_EVENT =
            "com.mediatek.rcse.SERVICE_CONNECTED_EVENT";

    /**
     * This method should only be called from ApiService, for APIs
     * initialization.
     *
     * @param context
     *            The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Logger.v(TAG, "initialize() entry");
        if (null != sInstance) {
            Logger.w(
                    TAG,
                    "initialize() sInstance has existed, " +
                    "is it really the first time you call this method?");
            return true;
        } else {
            if (null != context) {
                RcsSettings rcsSetting = RcsSettings.getInstance();
                AppSettings appSetting = AppSettings.getInstance();
                if (rcsSetting == null) {
                    RcsSettings.createInstance(context);
                }
                if (appSetting == null) {
                    AppSettings.createInstance(context);
                }
                if (MediatekRichProviderHelper.getInstance() == null) {
                    MediatekRichProviderHelper.createInstance(context);
                }
                ApiManager apiManager = new ApiManager(context);
                sInstance = apiManager;
                AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... arg0) {
                        sMaxFileSize = RcsSettings.getInstance()
                                .getMaxFileTransferSize() * 1024;
                        sWarningFileSize = RcsSettings.getInstance()
                                .getWarningMaxFileTransferSize() * 1024;
                        return null;
                    }
                };
                task.execute();
                return true;
            } else {
                Logger.e(TAG, "initialize() the context is null");
                return false;
            }
        }
    }

    /**
     * Get the context.
     *
     * @return Context
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Get the RcseComponentController.
     *
     * @return RcseComponentController
     */
    public RcseComponentController getRcseComponentController() {
        return mRcseComponentController;
    }

    /**
     * Get the max size for file transfer.
     *
     * @return max size
     */
    public long getMaxSizeforFileThransfer() {
        return sMaxFileSize;
    }

    /**
     * Get the warning size for file transfer.
     *
     * @return warning size
     */
    public long getWarningSizeforFileThransfer() {
        return sWarningFileSize;
    }

    /**
     * Get the instance of ApiManager.
     *
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static ApiManager getInstance() {
        return sInstance;
    }

    /**
     * Get the connected RegistrationApi.
     *
     * @return The instance of RegistrationApi, or null if the instance has not
     *         connected.
     */
    public RegistrationApi getRegistrationApi() {
        Logger.v(TAG, "getRegistrationApi()");
        return mRegistrationApi;
    }

    /**
     * Get the connected CapabilityApi.
     *
     * @return The instance of CapabilityApi, or null if the instance has not
     *         connected.
     */
    public CapabilityService getCapabilityApi() {
        Logger.v(TAG, "getCapabilityApi()");
        return mCapabilitiesApi;
    }

    /**
     * Get the connected TermsApi.
     *
     * @return The instance of TermsApi, or null if the instance has not
     *         connected.
     */
    public TermsApi getTermsApi() {
        Logger.v(TAG, "getTermsApi()");
        return mTermsApi;
    }

    /**
     * Get the connected ChatService.
     *
     * @return The instance of ChatService, or null if the instance has not
     *         connected.
     */
    public ChatService getChatApi() {
        return mChatApi;
    }

    /**
     * Get the connected ContactsService.
     *
     * @return The instance of ContactsService, or null if the instance has not
     *         connected.
     */
    public ContactService getContactsApi() {
        return mContactsApi;
    }

    /**
     * Get the connected ChatService.
     *
     * @return The instance of ChatService, or null if the instance has not
     *         connected.
     */
    public FileTransferService getFileTransferApi() {
        Logger.v(TAG, "getFileTransferApi()");
        return mFileTransferApi;
    }

    /**
     * Gets the geoloc sharing api.
     *
     * @return the geoloc sharing api
     */
    public GeolocSharingService getGeolocSharingApi() {
        return mGeolocSharingApi;
    }

    /**
     * Get the connected NetworkConnectivityApi.
     *
     * @return The instance of getNetworkConnectivityApi, or null if the
     *         instance has not connected.
     */
    public NetworkConnectivityApi getNetworkConnectivityApi() {
        return mNetworkConnectivityApi;
    }

    public void onIMSStatusChanged(boolean status) {
        mImsConnected = status;
        Logger.v(TAG, "onIMSStatusChanged() : status = " + status);
        //TODO On Latch Disconnect & Connect again
    }

    /**
     * Instantiates a new api manager.
     *
     * @param context the context
     */
    private ApiManager(Context context) {
        Logger.d(TAG, "ApiManager() entry");
        mContext = context;
        if (!JoynService.isServiceStarted(context)) {
            Logger.d(TAG,
                    "RcsCoreService is not started yet, so services won't connect ");
        }
        mTermsApi = new TermsApi(context, new MyTermsApiServiceListener());
        mTermsApi.connect();
        new ManagedRegistrationApi(context).connect();
	    Logger.v(TAG, "ApiManager() : Connected using TAPI Directly ");
        try {
            mCapabilitiesApi = new CapabilityService(context,
                    new MyCapabilitiesServiceListener());
            mCapabilitiesApi.connect();
            mChatApi = new ChatService(context, new MyChatServiceListener());
            mChatApi.connect();
            mContactsApi = new ContactService(context,
                    new MyContactsServiceListener());
            mContactsApi.connect();
            mFileTransferApi = new FileTransferService(context,
                    new MyFileTransferServiceListener());
            mFileTransferApi.connect();

/*
            mGeolocSharingApi = new GeolocSharingService(context,
                    new MyGeolocSharingServiceListener());
            mGeolocSharingApi.connect();
*/
            mNetworkConnectivityApi = new NetworkConnectivityApi(context,
                    new MyNetworkConnectivityServiceListener());
            mNetworkConnectivityApi.connect();
            mRcseComponentController = new RcseComponentController();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * The listener interface for receiving myTermsApiService events.
     * The class that is interested in processing a myTermsApiService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyTermsApiServiceListener method. When
     * the myTermsApiService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyTermsApiServiceEvent
     */
    public class MyTermsApiServiceListener implements JoynServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(int error) {
            ApiManager.this.mTermsApi = null;
        }

    }

    /**
     * The listener interface for receiving myContactsService events.
     * The class that is interested in processing a myContactsService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyContactsServiceListener method. When
     * the myContactsService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyContactsServiceEvent
     */
    public class MyContactsServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            Logger.d(TAG, "ContactsService onServiceConnected entry ");

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            Logger.i(TAG,
                    "MyContactsServiceListener onServiceDisconnected entry");
            ApiManager.this.mContactsApi = null;
        }

    }

    /**
     * The listener interface for receiving myChatService events.
     * The class that is interested in processing a myChatService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyChatServiceListener method. When
     * the myChatService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyChatServiceEvent
     */
    public class MyChatServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            Logger.d(TAG, "ChatService onServiceConnected entry ");
            if (ContactsListManager.getInstance() != null) {
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... params) {
                        ContactsListManager.getInstance().onContactsDbChange();
                        return null;
                    }
                } .execute();
            }
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_CHAT_SERVICE_CONNECTED,
                            null, null);
            controllerMessage.sendToTarget();
        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode code) {
            Logger.i(TAG, "MyChatServiceListener onServiceDisconnected entry");
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_CHAT_SERVICE_DISCONNECTED,
                            null, null);
            controllerMessage.sendToTarget();
            ApiManager.this.mChatApi = null;
        }

    }

    /**
     * The listener interface for receiving myFileTransferService events.
     * The class that is interested in processing a myFileTransferService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyFileTransferServiceListener method. When
     * the myFileTransferService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyFileTransferServiceEvent
     */
    public class MyFileTransferServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_FILETRANSFER_SERVICE_CONNECTED,
                            null, null);
            controllerMessage.sendToTarget();
        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode code) {
            ControllerImpl controller = ControllerImpl.getInstance();
            Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_FILETRANSFER_SERVICE_DISCONNECTED,
                            null, null);
            controllerMessage.sendToTarget();
            ApiManager.this.mFileTransferApi = null;
        }

    }

    /**
     * The listener interface for receiving myGeolocSharingService events.
     * The class that is interested in processing a myGeolocSharingService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyGeolocSharingServiceListener method. When
     * the myGeolocSharingService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyGeolocSharingServiceEvent
     */
    public class MyGeolocSharingServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            ApiManager.this.mGeolocSharingApi = null;
        }

    }

    /**
     * The listener interface for receiving myCapabilitiesService events.
     * The class that is interested in processing a myCapabilitiesService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyCapabilitiesServiceListener method. When
     * the myCapabilitiesService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyCapabilitiesServiceEvent
     */
    public class MyCapabilitiesServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            ApiManager.this.mCapabilitiesApi = null;
        }

    }

    /**
     * The listener interface for receiving myNetworkConnectivityService events.
     * The class that is interested in processing a myNetworkConnectivityService
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyNetworkConnectivityServiceListener method. When
     * the myNetworkConnectivityService event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyNetworkConnectivityServiceEvent
     */
    public class MyNetworkConnectivityServiceListener implements
            JoynServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(int error) {
            ApiManager.this.mNetworkConnectivityApi = null;
        }

    }

    /**
     * The Class ManagedRegistrationApi.
     */
    private class ManagedRegistrationApi extends RegistrationApi {
        public static final String TAG = "ManagedRegistrationApi";

        /**
         * Instantiates a new managed registration api.
         *
         * @param context the context
         */
        public ManagedRegistrationApi(Context context) {
            super(context);
        }

        /**
         * Handle connected.
         */
        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry, this.isRegistered()");
            mRegistrationApi = this;

            mRegistrationApi
                    .addRegistrationStatusListener(new RegistrationStatusListener());
        }

        /**
         * Handle disconnected.
         */
        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            if (mRegistrationApi == this) {
                Logger.i(TAG,
                        "handleDisconnected() mRegistrationApi disconnected");
                mRegistrationApi = null;
                // reconnect to handle service disconnected due to application exit.
                connect();
            }
        }

        /**
         * The listener interface for receiving registrationStatus events.
         * The class that is interested in processing a registrationStatus
         * event implements this interface, and the object created
         * with that class is registered with a component using the
         * component's addRegistrationStatusListener method. When
         * the registrationStatus event occurs, that object's appropriate
         * method is invoked.
         *
         * @see RegistrationStatusEvent
         */
        private class RegistrationStatusListener implements
                IRegistrationStatusListener {
            /**
             * On status changed.
             *
             * @param status the status
             */
            @Override
            public void onStatusChanged(boolean status) {
                if (status) {
                    initializeMessagingApi();
                    initializeContactsApi();
                    initializeFileTransferApi();
                    initializeNetworkConnectivityApi();
                    initializeTermsApi();
                    initializeCapabilityApi();

                    AsyncTask.execute(new Runnable() {
                        @Override
                        public void run() {
                            sMaxFileSize = RcsSettings.getInstance()
                                    .getMaxFileTransferSize() * 1024;
                            sWarningFileSize = RcsSettings.getInstance()
                                    .getWarningMaxFileTransferSize() * 1024;
                        }
                    });
                }
            }

            /**
             * Initialize contacts api.
             *
             * @return true, if successful
             */
            private boolean initializeContactsApi() {
                Logger.d(TAG, "initializeContactsApi() entry");
                mContactsApi = new ContactService(mContext,
                        new MyContactsServiceListener());
                mContactsApi.connect();

                return true;
            }

            /**
             * Initialize network connectivity api.
             *
             * @return true, if successful
             */
            private boolean initializeNetworkConnectivityApi() {
                Logger.d(TAG, "initializeNetworkConnectivityApi() entry");

                mNetworkConnectivityApi = new NetworkConnectivityApi(mContext,
                        new MyNetworkConnectivityServiceListener());
                mNetworkConnectivityApi.connect();
                return true;
            }

            /**
             * Initialize terms api.
             *
             * @return true, if successful
             */
            private boolean initializeTermsApi() {
                Logger.d(TAG, "initializeTermsApi() entry");
                mTermsApi = new TermsApi(mContext,
                        new MyTermsApiServiceListener());
                mTermsApi.connect();

                return true;
            }

            /**
             * Initialize messaging api.
             *
             * @return true, if successful
             */
            private boolean initializeMessagingApi() {
                Logger.d(TAG, "initializeMessagingApi() entry");
                mChatApi = new ChatService(mContext,
                        new MyChatServiceListener());
                try {
                    mChatApi.connect();
                } catch (Exception e) {
                    e.printStackTrace();
                }

                return true;
            }

            /**
             * Initialize capability api.
             *
             * @return true, if successful
             */
            private boolean initializeCapabilityApi() {
                Logger.d(TAG, "initializeCapabilityApi() entry");
                mCapabilitiesApi = new CapabilityService(mContext,
                        new MyCapabilitiesServiceListener());
                mCapabilitiesApi.connect();

                return true;
            }

            /**
             * Initialize file transfer api.
             *
             * @return true, if successful
             */
            private boolean initializeFileTransferApi() {
                Logger.d(TAG, "initializeFileTransferApi() entry");
                mFileTransferApi = new FileTransferService(mContext,
                        new MyFileTransferServiceListener());
                try {
                    mFileTransferApi.connect();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                return true;
            }

            /**
             * Initialize geoloc sharing api.
             *
             * @return true, if successful
             */
            private boolean initializeGeolocSharingApi() {
                Logger.d(TAG, "initializeGeolocSharingApi() entry");
                mGeolocSharingApi = new GeolocSharingService(mContext,
                        new MyGeolocSharingServiceListener());
                try {
                    mGeolocSharingApi.connect();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                return true;
            }
        }
    }

    /**
     * control the RCS component according to the configuration and active
     * status.
     */
    public class RcseComponentController {
        private boolean mConfigurationStatus = true;
        private boolean mServiceActiveStatus = true;

        /**
         * Set configuration status and control the rcse component.
         *
         * @param status
         *            the configuration status.
         */
        public void onConfigurationStatusChanged(boolean status) {
            Logger.d(TAG, "onConfigurationStatusChanged() entry status is "
                    + status);
            mConfigurationStatus = status;
            controlRcseComponent();

        }

        /**
         * Set ServiceActive status and control the rcse component.
         *
         * @param status
         *            the status of ServiceActive .
         */
        public void onServiceActiveStatusChanged(boolean status) {
            Logger.d(TAG, "onServiceActiveStatusChanged() entry status is "
                    + status);
            mServiceActiveStatus = status;
            controlRcseComponent();
        }

        /**
         * control the RCS component according to the configuration and active
         * status.
         */
        private void controlRcseComponent() {
            Logger.d(TAG,
                    "controlRcseComponent() entry  the mConfigurationstatus is "
                            + mConfigurationStatus + " ServiceactiveStatus is "
                            + mServiceActiveStatus);
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG,
                        "controlRcseComponent() entry is integration mode ");
                CoreApplication.setIntegrationModeComponent(mContext);
            } else {
                if (mConfigurationStatus && mServiceActiveStatus) {
                    CoreApplication.setComponentStatus(mContext, true);
                } else {
                    CoreApplication.setComponentStatus(mContext, false);
                    RcsNotification.getInstance().cancelNotification();
                }
            }
        }
    }

}
