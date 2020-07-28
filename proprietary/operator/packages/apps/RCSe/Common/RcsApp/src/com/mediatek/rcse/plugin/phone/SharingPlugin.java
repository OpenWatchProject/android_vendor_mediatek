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
package com.mediatek.rcse.plugin.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Toast;

import com.android.internal.telephony.CallManager;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.CapabilitiesChangeListener;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcs.R;
import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.sharing.image.ImageSharingListener;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceNotAvailableException;
import com.gsma.services.rcs.RcsServiceRegistration;
import com.gsma.services.rcs.RcsServiceRegistrationListener;
import com.gsma.services.rcs.sharing.image.ImageSharingService;
import com.gsma.services.rcs.sharing.video.VideoSharingService;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;

/**
 * This class defined to abstract sharing class.
 */
public class SharingPlugin implements ICallScreenPlugIn,
        CapabilitiesChangeListener {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "SharingPlugin";
    /* package *//**
                  * The Constant SERVICE_STATUS.
                  */
    static final String SERVICE_STATUS =
            "com.orangelabs.rcs.SERVICE_STATUS";
    /* package *//**
                  * The Constant SERVICE_REGISTRATION.
                  */
    static final String SERVICE_REGISTRATION =
            "com.orangelabs.rcs.SERVICE_REGISTRATION";
    /**
     * The Constant CORE_SERVICE_STATUS.
     */
    private static final String CORE_SERVICE_STATUS = "status";
    /**
     * The Constant INVALID_VALUE.
     */
    private static final int INVALID_VALUE = -1;
    /**
     * The m context.
     */
    Context mContext;
    /**
     * The m call screen host.
     */
    ICallScreenHost mCallScreenHost = null;
    /**
     * The m main handler.
     */
    Handler mMainHandler = null;
    /**
     * The m wake lock.
     */
    WakeLock mWakeLock = null;
    /**
     * The m wake lock count.
     */
    final AtomicInteger mWakeLockCount = new AtomicInteger(0);
    /**
     * The m display area.
     */
    public ViewGroup mDisplayArea = null;
    /**
     * The m rich call status.
     */
    int mRichCallStatus = RichCallStatus.DISCONNECTED;
    /**
     * The m video sharing api.
     */
    VideoSharingService mVideoSharingApi = null;
    /**
     * The m image sharing api.
     */
    ImageSharingService mImageSharingApi = null;
    /**
     * The m interface.
     */
    ISharingPlugin mInterface = null;
    /**
     * Service status "started".
     */
    public final static int SERVICE_STATUS_STARTED = 1;
    /**
     * Service status "stopping".
     */
    public final static int SERVICE_STATUS_STOPPING = 2;
    /**
     * Service status "stopped".
     */
    public final static int SERVICE_STATUS_STOPPED = 3;
    // CountDownLatch used to synchronize initialize
    /**
     * The m count down latch.
     */
    CountDownLatch mCountDownLatch = new CountDownLatch(1);
    /**
     * The m number.
     */
    static String sNumber = null;
    /**
     * The m is on hold.
     */
    boolean mIsOnHold = false;
    /**
     * The m image sharing api listener.
     */
    private ImageSharingApiListener mImageSharingApiListener = null;
    /**
     * The m video sharing api listener.
     */
    private VideoSharingApiListener mVideoSharingApiListener = null;
    /**
     * The m rcse core service status receiver.
     */
    private RcseCoreServiceStatusReceiver mRcseCoreServiceStatusReceiver = null;
    /**
     * The m is registered.
     */
    private boolean mIsRegistered = false;

    /**
     * The Class RichCallStatus.
     */
    static final class RichCallStatus {
        /**
         * The Constant DISCONNECTED.
         */
        static final int DISCONNECTED = 0;
        /**
         * The Constant CONNECTING.
         */
        static final int CONNECTING = 1;
        /**
         * The Constant CONNECTED.
         */
        static final int CONNECTED = 2;
    }

    /**
     * Constructor.
     *
     * @param context the context
     */
    public SharingPlugin(final Context context) {
        Logger.d(TAG, "SharingPlugin() entry!");
        mContext = context;
        PluginApiManager.initialize(mContext);
        AppSettings.createInstance(mContext);
        mImageSharingApiListener = new ImageSharingApiListener();
        mImageSharingApi = new ImageSharingService(mContext,
                mImageSharingApiListener);
        mVideoSharingApiListener = new VideoSharingApiListener();
        mVideoSharingApi = new VideoSharingService(mContext,
                mVideoSharingApiListener);
        connectRichCallApi();
        mMainHandler = new Handler(mContext.getMainLooper());
        PowerManager pm = (PowerManager) mContext
                .getSystemService(ContextWrapper.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK
                | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
        mRcseCoreServiceStatusReceiver = new RcseCoreServiceStatusReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SERVICE_STATUS);
        intentFilter.addAction(SERVICE_REGISTRATION);
        mContext.registerReceiver(mRcseCoreServiceStatusReceiver,
                intentFilter);
        RcsSettings.createInstance(mContext);
    }
    /**
     * Connect rich call api.
     */
    void connectRichCallApi() {
        Logger.d(TAG, "connectRichCallApi() entry!");
        if (mRichCallStatus == RichCallStatus.DISCONNECTED) {
            mRichCallStatus = RichCallStatus.CONNECTING;
            PluginApiManager
                    .getInstance()
                    .addCapabilitiesChangeListener(SharingPlugin.this);
            try {
                mImageSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            try {
                mVideoSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            Logger.d(TAG,
                    "mRichCallStatus is CONNECTING, wait to notify when CONNECTED");
        } else {
            Logger.d(TAG,
                    "connectRichCallApi(), RichCallApi has been connected!");
        }
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#getCurrentState()
     */
    public int getCurrentState() {
        return Constants.SHARE_FILE_STATE_IDLE;
    }

    /**
     * Image sharing invitation receiver.
     */
    private class RcseCoreServiceStatusReceiver extends
            BroadcastReceiver {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "SharingPlugIn RcseCoreServiceStatusReceiver";

        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#onReceive
         * (android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive(), context = " + context
                    + ", intent = " + intent);
            if (intent != null) {
                String action = intent.getAction();
                Logger.v(TAG, "action = " + action);
                if (SERVICE_STATUS.equals(action)) {
                    int status = intent.getIntExtra(
                            CORE_SERVICE_STATUS, INVALID_VALUE);
                    Logger.v(TAG, "status = " + status);
                    handleRcsCoreServiceStatusChanged(status);
                } else {
                    Logger.w(TAG, "Unknown action");
                }
            } else {
                Logger.w(TAG, "intent is null");
            }
        }
    }

    /**
     * Handle registration status changed.
     *
     * @param status the status
     */
    private void handleRegistrationStatusChanged(boolean status) {
        Logger.v(TAG, "handleRegistrationStatusChanged() status = "
                + status);
        mIsRegistered = status;
        if (status) {
            if (mRichCallStatus != RichCallStatus.CONNECTED) {
                Logger.v(TAG,
                        "handleRegistrationStatusChanged() connect rich call api");
                try {
                    mImageSharingApi.connect();
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                try {
                    mVideoSharingApi.connect();
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            } else {
                Logger.d(
                        TAG,
                        "handleRegistrationStatusChanged() mRichCallStatus" +
                        " = RichCallStatus.CONNECTED");
            }
        }
    }
    /**
     * Handle rcs core service status changed.
     *
     * @param status the status
     */
    private void handleRcsCoreServiceStatusChanged(int status) {
        Logger.d(TAG, "handleRcsCoreServiceStatusChanged( ) : "
                + status);
        if (status == SERVICE_STATUS_STARTED) {
            if (PluginApiManager.getInstance().getManagedApiStatus()) {
                PluginApiManager.getInstance().reConnectManagedApi();
            }
        } else if (status == SERVICE_STATUS_STOPPING) {
            PluginApiManager.getInstance().setManagedApiStatus(true);
        }
    }

    /**
     * The listener interface for receiving imageSharingApi events.
     * The class that is interested in processing a imageSharingApi
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addImageSharingApiListener method. When
     * the imageSharingApi event occurs, that object's appropriate
     * method is invoked.
     *
     * @see ImageSharingApiEvent
     */
    private class ImageSharingApiListener implements
        RcsServiceListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "ImageSharingApiListener";

        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceListener#onServiceConnected()
         */
        @Override
        public void onServiceConnected() {
            Logger.d(TAG, "onServiceConnected entry");
            mRichCallStatus = RichCallStatus.CONNECTED;
            RegistrationListener listener = new RegistrationListener();
            try {
               /* mImageSharingApi
                        .addServiceRegistrationListener(listener);*/
                mIsRegistered = mImageSharingApi
                        .isServiceRegistered();
            } catch (RcsServiceException e) {
                e.printStackTrace();
            }
            if (mCallScreenHost != null) {
                mCallScreenHost.onCapabilityChange(sNumber,
                        getCapability(sNumber));
            } else {
                Logger.d(TAG,
                        "onServiceConnected mCallScreenHost is null");
            }
            mInterface.onApiConnected();
            Logger.d(TAG, "onServiceConnected exit");
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceListener#onServiceDisconnected(int)
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasoncode) {
            Logger.d(TAG, "onServiceDisconnected entry");
            mRichCallStatus = RichCallStatus.DISCONNECTED;
            Logger.d(TAG, "onServiceDisconnected exit");
        }
    }

    public void restartSharing(){
        // Do Nothing     
    }
    
    public void pauseSharing(){
        // Do Nothing     
    }

    /**
     * The listener interface for receiving videoSharingApi events.
     * The class that is interested in processing a videoSharingApi
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addVideoSharingApiListener method. When
     * the videoSharingApi event occurs, that object's appropriate
     * method is invoked.
     *
     * @see VideoSharingApiEvent
     */
    private class VideoSharingApiListener implements
            RcsServiceListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "ImageSharingApiListener";

        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceListener#onServiceConnected()
         */
        @Override
        public void onServiceConnected() {
            Logger.d(TAG, "onServiceConnected entry");
            mRichCallStatus = RichCallStatus.CONNECTED;
            if (mCallScreenHost != null) {
                mCallScreenHost.onCapabilityChange(sNumber,
                        getCapability(sNumber));
            } else {
                Logger.d(TAG,
                        "onServiceConnected mCallScreenHost is null");
            }
            mInterface.onApiConnected();
            Logger.d(TAG, "onServiceConnected exit");
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceListener#onServiceDisconnected(int)
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasoncode) {
            Logger.d(TAG, "onServiceDisconnected entry");
            mRichCallStatus = RichCallStatus.DISCONNECTED;
            Logger.d(TAG, "onServiceDisconnected exit");
        }
    }

    /**
     * The listener interface for receiving registration events.
     * The class that is interested in processing a registration
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addRegistrationListener method. When
     * the registration event occurs, that object's appropriate
     * method is invoked.
     *
     * @see RegistrationEvent
     */
    private class RegistrationListener extends
            RcsServiceRegistrationListener {
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceRegistrationListener#onServiceRegistered()
         */
        @Override
        public void onServiceRegistered() {
            Logger.d(TAG, "onServiceRegistered entry");
            handleRegistrationStatusChanged(true);
        }
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.JoynServiceRegistrationListener#onServiceUnregistered()
         */
        @Override
        public void onServiceUnregistered(RcsServiceRegistration.ReasonCode reasonCode) {
            Logger.d(TAG, "onServiceUnregistered entry");
            handleRegistrationStatusChanged(false);
        }
    }

    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#start(java.lang.String)
     */
    @Override
    public void start(String number) {
        Logger.d(TAG, "start entry, with number: " + number);
        if (number == null) {
            Logger.e(TAG, "start number is null");
            return;
        }
        int networkType = Utils.getNetworkType(mContext);
        if (networkType == Utils.NETWORK_TYPE_GPRS
                || networkType == Utils.NETWORK_TYPE_EDGE) {
            Logger.v(TAG,
                    "At such case, not allowed to send a video share, networkType is "
                            + networkType);
            return;
        }
        sNumber = number;
        Logger.d(TAG, "start exit myNumber" + sNumber);
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#stop()
     */
    @Override
    public void stop() {
    }
    
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#clearImageSharingViews()
     */
    @Override
	public void clearImageSharingViews() {
	}
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#
     * setCallScreenHost(com.mediatek.rcse.plugin.phone.ICallScreenHost)
     */
    @Override
    public void setCallScreenHost(ICallScreenHost host) {
        Logger.v(TAG, "setCallScreenHost(), host = " + host);
        mCallScreenHost = host;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#
     * getCapability(java.lang.String)
     */
    @Override
    public boolean getCapability(String number) {
        Logger.v(TAG, "sharing getCapability(), number: " + number);
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.v(
                    TAG,
                    "sharing getCapability() mRichCallStatus is NOT CONNECTED, return false");
            try {
                mImageSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            try {
                mVideoSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            return false;
        }
        if (!mIsRegistered) {
            Logger.v(TAG,
                    "sharing getCapability() off line, return false");
            return false;
        } else {
            Logger.d(TAG,
                    "sharing getCapability() the registration is true ");
        }
        final String operatorAccount = getOperatorAccount(number);
        boolean isSharingSupported = PluginApiManager.getInstance()
                .isRcseContact(operatorAccount);
        Logger.v(TAG, "sharing getCapability() isSharingSupported: "
                + isSharingSupported);
        return isSharingSupported;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#
     * isImageShareSupported(java.lang.String)
     */
    @Override
    public boolean isImageShareSupported(String number) {
        Logger.w(TAG, "isImageShareSupported(), number: " + number);
        sNumber = number;
        Logger.w(TAG, "isImageShareSupported(), myNumber: " + sNumber);
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.w(
                    TAG,
                    "sharing isImageShareSupported() mRichCallStatus" +
                    " is NOT CONNECTED, return false");
            try {
                mImageSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            return false;
        }
        if (!mIsRegistered) {
            Logger.v(TAG,
                    "sharing isImageShareSupported() off line, return false");
            return false;
        }
        Logger.w(TAG,
                "isImageShareSupported() the registration is true");
        final String operatorAccount = getOperatorAccount(number);
        boolean isImageShareSupported = PluginApiManager
                .getInstance().isImageShareSupported(operatorAccount);
        Logger.w(TAG,
                "sharing isImageShareSupported() isImageShareSupported: "
                        + isImageShareSupported);
        return isImageShareSupported;
    }
    /**
     * Image share not supported.
     */
    protected void imageShareNotSupported() {
        Logger.v(TAG, "imageShareNotSupported entry");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.image_sharing_not_available);
                showToast(message);
            }
        });
        Logger.v(TAG, "imageShareNotSupported exit");
    }
    /**
     * Video share not supported.
     */
    protected void videoShareNotSupported() {
        Logger.v(TAG, "videoShareNotSupported entry");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.video_sharing_not_available);
                showToast(message);
            }
        });
        Logger.v(TAG, "videoShareNotSupported exit");
    }
    /**
     * Show toast.
     *
     * @param message the message
     */
    protected void showToast(String message) {
        Logger.v(TAG, "showToast() entry, message = " + message);
        Toast toast = Toast.makeText(mContext, message,
                Toast.LENGTH_SHORT);
        toast.setGravity(Gravity.CENTER, 0, 0);
        toast.show();
        Logger.v(TAG, "showToast() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#
     * isVideoShareSupported(java.lang.String)
     */
    @Override
    public boolean isVideoShareSupported(String number) {
        Logger.v(TAG, "isVideoShareSupported(), number: " + number);
        sNumber = number;
        Logger.w(TAG, "isVideoShareSupported(), myNumber: " + sNumber);
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.v(
                    TAG,
                    "isVideoShareSupported() mRichCallStatus is NOT CONNECTED, return false");
            try {
                mVideoSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            return false;
        }
        if (!mIsRegistered) {
            Logger.v(TAG,
                    "isVideoShareSupported() off line, return false");
            return false;
        }
        Logger.d(TAG,
                "isVideoShareSupported() the registration is true");
        final String operatorAccount = getOperatorAccount(number);
        boolean isVideoShareSupported = PluginApiManager
                .getInstance().isVideoShareSupported(operatorAccount);
        Logger.v(TAG,
                "isVideoShareSupported() isVideoShareSupported: "
                        + isVideoShareSupported);
        return isVideoShareSupported;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#getCallScreenHost()
     */
    @Override
    public ICallScreenHost getCallScreenHost() {
        Logger.v(TAG, "getCallScreenHost(), mCallScreenHost = "
                + mCallScreenHost);
        return mCallScreenHost;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.service.PluginApiManager.
     * CapabilitiesChangeListener#onCapabilitiesChanged
     * (java.lang.String, com.mediatek.rcse.service.PluginApiManager.ContactInformation)
     */
    @Override
    public void onCapabilitiesChanged(String contact,
            ContactInformation contactInformation) {
        Logger.w(TAG, "onCapabilitiesChanged() contactInformation: "
                + contactInformation + " mCallScreenHost: "
                + mCallScreenHost + " contact: " + contact
                + " sNumber: " + sNumber + "isImageShareSupported" + contactInformation.isImageShareSupported
                + "videosharesupport" + contactInformation.isImageShareSupported);
        if (contactInformation == null || mCallScreenHost == null) {
            return;
        }
        boolean supported = contactInformation.isRcsContact == 1;
        String number = getOperatorAccount(sNumber);
        if (number != null && number.equals(contact)) {
            Logger.w(TAG, "onCapabilitiesChanged  contact: "
                    + contact);
            RCSeUtils.isImageShareSupported = contactInformation.isImageShareSupported;
            RCSeUtils.isVideoShareSupported = contactInformation.isVideoShareSupported;
            mCallScreenHost.onCapabilityChange(contact, supported);
            if (!supported) {
                mInterface.onFinishSharing();
            }
        }
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.service.PluginApiManager.
     * CapabilitiesChangeListener#onApiConnectedStatusChanged(boolean)
     */
    @Override
    public void onApiConnectedStatusChanged(boolean isConnected) {
        Logger.d(TAG, "onApiConnectedStatusChanged isConnected: "
                + isConnected);
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn
     * #registerForCapabilityChange(java.lang.String)
     */
    @Override
    public void registerForCapabilityChange(final String number) {
        Logger.d(TAG, "registerForCapabilityChange number: " + number);
        sNumber = number;
        Logger.w(TAG, "registerForCapabilityChange myNumber: "
                + number);
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#
     * unregisterForCapabilityChange(java.lang.String)
     */
    @Override
    public void unregisterForCapabilityChange(final String number) {
        Logger.d(TAG, "unregisterForCapabilityChange number: "
                + number);
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#getState()
     */
    @Override
    public int getState() {
        return 0;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#getStatus()
     */
    @Override
    public int getStatus() {
        return 0; //not to be used
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#saveAlertDialogs()
     */
    @Override
    public void saveAlertDialogs() {
        Logger.v(TAG, "saveAlertDialogs() entry");
        Logger.v(TAG, "saveAlertDialogs() exit");
    }
    @Override
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#showAlertDialogs()
     */
    public void showAlertDialogs() {
        Logger.v(TAG, "showAlertDialogs() entry");
        Logger.v(TAG, "showAlertDialogs() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#clearSavedDialogs()
     */
    @Override
    public void clearSavedDialogs() {
        Logger.v(TAG, "clearSavedDialogs() entry");
        Logger.v(TAG, "clearSavedDialogs() exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#onPhoneStateChange(CallManager)
     */
    @Override
    public void onPhoneStateChange(CallManager cm) {
        boolean hasActiveBgCall = false;
        if (RCSeUtils.getmFgCall() != null
                && (RCSeUtils.getmFgCall().getState() == RCSeUtils.STATE_HOLDING)) {
            hasActiveBgCall = true;
        }
        //boolean hasActiveBgCall = cm.hasActiveBgCall();
        Logger.d(TAG, "onPhoneStateChange(), hasActiveBgCall = "
                + hasActiveBgCall + " mIsOnHold = " + mIsOnHold);
        if (hasActiveBgCall ^ mIsOnHold) {
            List<String> numbers = new ArrayList<String>();
            numbers.add(getOperatorAccount(sNumber));
            PluginApiManager.getInstance().queryNumbersPresence(
                    numbers);
            // exchange capability with remote people
            /*CapabilityService api = PluginApiManager.getInstance().getCapabilityApi();
            if (api != null) {
                Capabilities capabilities = api.getMyCapabilities();
                if (capabilities != null) {
                    boolean isHaveImageShareCapability =
                     capabilities.isImageSharingSupported() && !hasActiveBgCall;
                    capabilities.setImageSharingSupport(isHaveImageShareCapability);
                    capabilities.setVideoSharingSupport(isHaveImageShareCapability);
                    api.setMyCapabilities(capabilities);
                    api.getContactCapabilities(getOperatorAccount(sNumber));
                } else {
                    Logger.e(TAG, "onPhoneStateChange(), capabilities is null!");
                }
            } else {
                Logger.d(TAG, "getCapability, capability api is null");
            }*/
        } else {
            Logger.d(TAG,
                    "Hold status not changed, no need to exchange capability!");
        }
        mIsOnHold = hasActiveBgCall;
    }
    /**
     * Gets the operator account.
     *
     * @param normalNumber the normal number
     * @return the operator account
     */
    protected String getOperatorAccount(String normalNumber) {
        Logger.w(TAG, "getOperatorAccount() entry, normalNumber: "
                + normalNumber);
        String operatorAccount = normalNumber;
        if (normalNumber != null) {
            try {
                String number = normalNumber;
                /*number = mImageSharingApi
                        .getJoynAccountViaNumber(normalNumber);*/ //ToDo
                sNumber = normalNumber;
                Logger.w(TAG,
                        "getOperatorAccount() entry, normalNumber: "
                                + "myNumber:" + sNumber);
                if (number != null) {
                    operatorAccount = number;
                }
            } catch (Exception e) {
                e.printStackTrace();
                Logger.e(TAG,
                        "getOperatorAccount() JoynServiceException, return null");
            }
        }
        // format to + country code xxx
        operatorAccount = com.mediatek.rcse.service.Utils
                .formatNumberToInternational(operatorAccount);
        Logger.d(TAG, "getOperatorAccount() exit, OperatorAccount: "
                + operatorAccount);
        return operatorAccount;
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.ICallScreenPlugIn#dismissDialog()
     */
    @Override
    public boolean dismissDialog() {
        return false;
    }

    /**
     * The Interface ISharingPlugin.
     */
    interface ISharingPlugin {
        /**
         * On api connected.
         */
        void onApiConnected();
        /**
         * On finish sharing.
         */
        void onFinishSharing();
    }
}
