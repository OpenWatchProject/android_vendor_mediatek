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

package com.mediatek.contacts.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.util.LruCache;

import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;
import com.gsma.services.rcs.contact.ContactsProvider;
import com.gsma.services.rcs.JoynContactFormatException;
import com.gsma.services.rcs.JoynServiceException;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

/**
 * This class manages the APIs which are used by plug-in, providing a convenient
 * way for API invocations.
 */
public class PresenceApiManager {
    public static final String TAG = "PHB/PresenceApiManager";

    public static final int QUERY_TYPE_CAPABILITY = 1;
    public static final int QUERY_TYPE_AVAILABILITY = 2;

    private static final boolean IS_UT_MODE = false;
    private static final String TEST_NUMBER_0 = "+12345678900";
    private static final String TEST_NUMBER_1 = "+12345678911";
    public static final int MESSAGE_TEST_AVAIBLE_ERROR = 1001;

    private static PresenceApiManager sInstance = null;
    private static final int MAX_CACHE_SIZE = 2048;
    private static final int MAX_REQUEST_COUNT = 10;
    private static final int MESSAGE_TEST = 1;
    private static final int MESSAGE_DELAY = 2;
    private static final int MESSAGE_QUREY_AVAIBLE_TIMEOUT = 3;
    private static final long MESSAGE_DELAY_TIME = 800; // 800ms
    private static final String PROPERTY_UCE_SUPPORT = "persist.vendor.mtk_uce_support";
    private static final String CORE_SERVICE_STATUS = "status";
    private static final String CORE_SERVICE_CURRENT_STATE_NOTIFICATION =
            "com.mediatek.presence.CORE_SERVICE_STATE";
    private static final int CORE_SERVICE_STARTED = 2;
    private static final int CORE_SERVICE_STOPPED = 3;
    private static final int CORE_SERVICE_INVALID = -1;

    private MyCapabilitiesListener mMyCapabilitiesListener = null;
    private CapabilityService mCapabilitiesApi = null;
    private Context mContext = null;
    private boolean mConnected = false;
    private boolean mNeedReconnect = false;
    private MessageDataHandler mHandler;
    private PresenceServiceStatusReceiver mServiceReceiver = null;
    private VtSettingContentObserver mVtSettingObserver;

    private boolean mIsAirPlaneMode = false;
    private boolean mIsVtEnabled = false;

    private final LruCache<String, ContactInformation> mContactsCache =
        new LruCache<String, ContactInformation>(MAX_CACHE_SIZE);
    private final List<CapabilitiesChangeListener> mCapabilitiesChangeListenerList =
        new ArrayList<CapabilitiesChangeListener>();

    private final Handler mMainThreadHandler = new Handler(Looper.getMainLooper());

     /**
    * Presence Service Status Receiver.
    */
    private class PresenceServiceStatusReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                Log.d(TAG, "[onReceive]action = " + action);
                if (CORE_SERVICE_CURRENT_STATE_NOTIFICATION.equals(action)) {
                    int status = intent.getIntExtra(
                            CORE_SERVICE_STATUS, CORE_SERVICE_INVALID);
                    handleServiceStatusChanged(status);
                } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                    mIsAirPlaneMode = (intent.getBooleanExtra("state", false));
                    Log.d(TAG, "[onReceive]mIsAirPlaneMode = " + mIsAirPlaneMode);
                }
            }
        }
    }

    private class VtSettingContentObserver extends ContentObserver {
        public VtSettingContentObserver() {
            super(new Handler());
        }

        @Override
        public void onChange(final boolean selfChange) {
            mIsVtEnabled = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.VT_IMS_ENABLED, 1) == 1;
            Log.d(TAG, "[onChange]mIsVtEnabled = " + mIsVtEnabled);
        }
    }

    private void handleServiceStatusChanged(int status) {
        Log.d(TAG, "StatusChanged: " + status + " mNeedReconnect: " + mNeedReconnect);
        if (status == CORE_SERVICE_STARTED) {
            if (mNeedReconnect && mCapabilitiesApi != null) {
                Log.d(TAG, "Reconnect to service");
                mCapabilitiesApi.connect();
            }
        }
    }

    public boolean isVtEnabled() {
        return mIsVtEnabled;
    }

    public boolean isAirPlaneMode() {
        return mIsAirPlaneMode;
    }

    /**
    * Message data.
    */
    private final class MessageData {
        public String mNumber;
        public boolean mForce;

        MessageData(String number, boolean force) {
            mNumber = number;
            mForce = force;
        }
    }

    /**
    * Message handler.
    */
    private final class MessageDataHandler extends Handler {

        public MessageDataHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "[handleMessage] what = " + msg.what);
            switch (msg.what) {
                case MESSAGE_TEST:
                    synchronized (mContactsCache) {
                        String contact = "+12345678900";
                        ContactInformation info = new ContactInformation();
                        info.isVideoCapable = true;
                        mContactsCache.put(contact, info);
                    }
                    break;

                case MESSAGE_DELAY:
                    MessageData data = (MessageData) msg.obj;
                    requestContactPresence(data.mNumber, data.mForce);
                    break;

                case MESSAGE_QUREY_AVAIBLE_TIMEOUT:
                    for (CapabilitiesChangeListener listener :
                            mCapabilitiesChangeListenerList) {
                        if (listener != null) {
                            listener.onQueryAvailabilityTimeout();
                        }
                    }
                    break;

                case MESSAGE_TEST_AVAIBLE_ERROR:
                    mMyCapabilitiesListener.onErrorReceived(
                            "+12345678900", 2, 100, "Test error message");
                    break;

                default:
                    Log.e(TAG, "handleMessage unknown");
                    break;
            }
        }
    }

    public void sendMessage(int type, int delay) {
        mHandler.sendEmptyMessageDelayed(type, delay);
    }

    /**
     * MIME type for RCSE capabilities.
     */
    private static final String MIMETYPE_VOICE_SHARING = ContactsProvider.MIME_TYPE_IR94_VOICE_CALL;
    private static final String MIMETYPE_VIDEO_SHARING = ContactsProvider.MIME_TYPE_IR94_VIDEO_CALL;
    private static final String MIMETYPE_PHONE_CONTACT = "vnd.android.cursor.item/phone_v2";

    /**
     * The CapabilitiesChangeListener defined as a listener to notify the
     * specify observer that the capabilities has been changed.
     *
     * @see CapabilitiesChangeEvent
     */
    public interface CapabilitiesChangeListener {

        /**
         * On capabilities changed.
         *
         * @param contact
         *            the contact
         * @param info
         *            the contact information
         */
        void onCapabilitiesChanged(String contact, ContactInformation info);

        void onErrorReceived(String contact, int type, int status, String reason);

        void onQueryAvailabilityTimeout();
    }

    /**
     * Register the CapabilitiesChangeListener.
     *
     * @param listener
     *            The CapabilitiesChangeListener used to register
     */
    public void addCapabilitiesChangeListener(
            CapabilitiesChangeListener listener) {
        Log.d(TAG, "addCapabilitiesChangeListener(), listener = " + listener);
        mCapabilitiesChangeListenerList.add(listener);
    }

    /**
     * Unregister the CapabilitiesChangeListener.
     *
     * @param listener
     *            The CapabilitiesChangeListener used to unregister
     */
    public void removeCapabilitiesChangeListener(
            CapabilitiesChangeListener listener) {
        Log.d(TAG, "removeCapabilitiesChangeListener(), listener = " + listener);
        mCapabilitiesChangeListenerList.remove(listener);
    }

    /**
     * Get the instance of PresenceApiManager.
     *
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static PresenceApiManager getInstance() {
        if (null == sInstance) {
            throw new RuntimeException(
                    "Please call initialize() before calling this method");
        }
        return sInstance;
    }

    /**
     * Instantiates a new plugin api manager.
     *
     * @param context
     *            the context
     */
    private PresenceApiManager(Context context) {
        Log.d(TAG, "[PresenceApiManager] context = " + context);
        mContext = context;
        // connect to capability TAPI
        mMyCapabilitiesListener = new MyCapabilitiesListener();
        MyRcsServiceListener myRcsServiceListener = new MyRcsServiceListener();
        mCapabilitiesApi = new CapabilityService(context, myRcsServiceListener);
        mCapabilitiesApi.connect();
        //init are plane mode and IMS state
        mIsAirPlaneMode = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        mIsVtEnabled = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.VT_IMS_ENABLED, 1) == 1;
        Log.d(TAG, "[PresenceApiManager] mIsAirPlaneMod=" + mIsAirPlaneMode +
                ", mIsVtEnabled" + mIsVtEnabled);
        // fill cache information for video capability
        initData();
        HandlerThread thread = new HandlerThread("MessageDataHandler");
        thread.start();
        mHandler = new MessageDataHandler(thread.getLooper());
        mServiceReceiver = new PresenceServiceStatusReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mServiceReceiver, intentFilter);
        mVtSettingObserver = new VtSettingContentObserver();
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.VT_IMS_ENABLED),
                false, mVtSettingObserver);
        /*mContext.getContentResolver().registerContentObserver(
                ContactsContract.AUTHORITY_URI, true,
                new ContentObserver(mMainThreadHandler) {
                    @Override
                    public void onChange(boolean selfChange) {
                        Log.d(TAG, "[onChange] selfChange=" + selfChange);
                    }

                    @Override
                    public void onChange(boolean selfChange, Uri uri) {
                        Log.d(TAG, "[onChange] selfChange=" + selfChange
                                + ", uri=" + uri.toString());
                        Thread thread = new Thread() {
                            public void run() {
                                refreshContactsPresence();
                            }
                        };
                        thread.start();
                    }
                });*/
    }

    /**
     * Init the data.
     */
    private void initData() {
        Log.d(TAG, "initData entry");
        Thread thread = new Thread() {
            public void run() {
                Looper.prepare();
                queryContactsPresence();
            }
        };
        thread.start();
        Log.d(TAG, "initData exit");
    }

    /**
     * Check whether a number is video call capable or not.
     *
     * @param number
     *            The number to query
     * @return True if number is a video capable contact, otherwise return
     *         false.
     */
    public boolean isVideoCallCapable(String number) {
        boolean result = false;
        if (number == null) {
            Log.w(TAG, "number is null");
            return result;
        }
        if (!isServiceConnected()) {
            return result;
        }
        number = number.trim();
        LogUtils.d(TAG, "isVideoCallCapable number: " + LogUtils.anonymize(number));

        String formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number);
        if (formatNumber != null) {
            ContactInformation info = mContactsCache.get(formatNumber);
            if (info != null) {
                Log.d(TAG, "video call capable " + info.isVideoCapable);
                if (info.isVideoCapable) {
                    result = true;
                }
            } else {
                getContactPresence(formatNumber);
            }
        }
        return result;
    }

    public boolean isVoiceCapable(String number) {
        boolean result = false;
        if (number == null) {
            Log.w(TAG, "number is null");
            return result;
        }
        if (!isServiceConnected()) {
            return result;
        }
        number = number.trim();
        LogUtils.d(TAG, "[isVoiceCapable] number: " + LogUtils.anonymize(number));

        String formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number);
        if (formatNumber != null) {
            ContactInformation info = mContactsCache.get(formatNumber);
            if (info != null) {
                Log.d(TAG, "[isVoiceCapable]video call capable: " + info.isVoiceCapable);
                if (info.isVoiceCapable) {
                    result = true;
                }
            } else {
                getContactPresence(formatNumber);
            }
        }
        return result;

    }


    /**
     * Query contact presence.
     *
     * @param number the number
     */
    private void getContactPresence(final String number) {
        LogUtils.d(TAG, "getContactPresence entry, number is " + LogUtils.anonymize(number));
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                if (mCapabilitiesApi != null) {
                    try {
                        Capabilities capabilities = mCapabilitiesApi
                                .getContactCapabilities(createContactIdFromNumber(number));
                        if (capabilities == null) {
                           Log.d(TAG, "[getContactPresence] capabilities null, "
                                   + LogUtils.anonymize(number));
                           return;
                        }
                        ContactInformation info = new ContactInformation();
                        if (capabilities.isIR94_VideoCallSupported()) {
                            info.isVideoCapable = true;
                        } else {
                            info.isVideoCapable = false;
                        }
                        if (capabilities.isIR94_VoiceCallSupported()) {
                            info.isVoiceCapable = true;
                        } else {
                            info.isVoiceCapable = false;
                        }
                        synchronized (mContactsCache) {
                            LogUtils.d(TAG, "[getContactPresence] "
                                    + LogUtils.anonymize(number)
                                    + ", video: " + info.isVideoCapable
                                    + ", voice: " + capabilities.isIR94_VoiceCallSupported()
                                    + ", duplex: " + capabilities.isIR94_DuplexModeSupported());
                            mContactsCache.put(number, info);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }

    /**
     * Is service connected.
     *
     * @return mConnected
     */
    public boolean isServiceConnected() {
        Log.d(TAG, "isServiceConnected: " + mConnected);
        if (!mConnected) {
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            Log.d(TAG, "Wait and try again. isServiceConnected: " + mConnected);
        }
        return mConnected;
    }

    /**
     * Set service connected.
     *
     * @param state true or false
     */
    public void setServiceConnected(boolean state) {
        mConnected = state;
        Log.d(TAG, "SetServiceConnected: " + mConnected);
    }

    /**
     * Query contacts capablility.
     *         Vzw request using different interface to query capablility
     *         for single number and multiple number.
     *
     * @param numbers
     *            Must put in FORMATTED number list.
     */
    public boolean requestContactsCapability(final List<String> numbers) {
        boolean ret = false;
        if (!isServiceConnected()) {
            return ret;
        }
        List<ContactId> contactIds = new ArrayList<ContactId>();
        for (String number : numbers) {
            if (number != null) {
                contactIds.add(createContactIdFromNumber(number));
            }
        }
        int count = contactIds.size();
        if (mCapabilitiesApi != null && count > 0) {
            try {
                if (count > 1) {
                    mCapabilitiesApi.requestContactsCapabilities(contactIds);
                } else {
                    mCapabilitiesApi.requestContactCapabilities(contactIds.get(0));
                }
                ret = true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        Log.d(TAG, "[requestContactsCapability] exit:" + ret);
        return ret;
    }

    /**
     * Query contacts availability.
     *
     * @param numbers
     *        Must put in FORMATTED number list and has capability.
     */
    public boolean requestContactsAvailability(final String number) {
        Log.d(TAG, "[requestContactsAvailability] number: " + LogUtils.anonymize(number));
        boolean ret = false;
        if (!isServiceConnected()) {
            return ret;
        }
        if (mCapabilitiesApi != null) {
            if (!mIsAirPlaneMode && mIsVtEnabled) {
                try {
                    mCapabilitiesApi.requestContactAvailability(
                            createContactIdFromNumber(number));
                    ret = true;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Log.w(TAG, "[requestContactsAvailability]"
                        + " mIsAirPlaneMode=" + mIsAirPlaneMode
                        + ", mIsVtEnabled=" + mIsVtEnabled);
            }
        }
        Log.d(TAG, "[requestContactsAvailability] exit:" + ret);
        return ret;
    }

   /**
     * Query contact presence.
     *
     * @param number Contact number
     * @param force Force query or not
     */
    public void requestContactPresence(final String number, final boolean force) {
        LogUtils.d(TAG, "[requestContactPresence] number: " + LogUtils.anonymize(number)
                + " force: " + force);
        if (!isServiceConnected()) {
            return;
        }
        final String formatNumber = ContactNumberUtils.getDefault().getFormatNumber(number);
        if (formatNumber != null) {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    if (mCapabilitiesApi != null) {
                        try {
                            if (force) {
                                mCapabilitiesApi.forceRequestContactCapabilities(
                                        createContactIdFromNumber(formatNumber));
                            } else {
                                mCapabilitiesApi.requestContactCapabilities(
                                        createContactIdFromNumber(formatNumber));
                            }
                            // put initial data into cache to avoid multi-request.
                            synchronized (mContactsCache) {
                                if (mContactsCache.get(formatNumber) == null) {
                                    LogUtils.d(TAG, "requestContactPresence new:"
                                            + LogUtils.anonymize(formatNumber));
                                    ContactInformation info = new ContactInformation();
                                    mContactsCache.put(formatNumber, info);
                                }
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
            });
            //for test
            //mHandler.sendEmptyMessageDelayed(MESSAGE_TEST, 5000);
        }
        Log.d(TAG, "requestContactPresence exit");
    }


    private void refreshContactsPresence() {
        String[] projection = { Phone.CONTACT_ID, Phone._ID, Data.MIMETYPE,
                Data.DATA1, Data.DATA2, Data.DATA4 };
        String selection = Data.MIMETYPE + "=?";
        String[] selectionArgs = { MIMETYPE_PHONE_CONTACT };
        Cursor cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                projection, selection, selectionArgs, null);
        if (cursor == null) {
            Log.d(TAG, "[refreshContactsPresence] cursor is null");
            return;
        }
        List<String> list = new ArrayList<String>();
        try {
            while (cursor.moveToNext()) {
                String number = cursor.getString(cursor.getColumnIndex(Data.DATA1));
                final String formatNumber =
                        ContactNumberUtils.getDefault().getFormatNumber(number);
                if (formatNumber != null) {
                    list.add(formatNumber);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (!list.isEmpty()) {
            List<ContactId> contactIds = new ArrayList<ContactId>();
            for (String number : list) {
                if (number != null) {
                    contactIds.add(createContactIdFromNumber(number));
                }
            }
            try {
                mCapabilitiesApi.requestContactsCapabilities(contactIds);
            } catch  ( JoynServiceException e){
                Log.e(TAG, "requestContactsCapabilities excpetion:", e);
            }
        }
    }

    /**
     * The class including some informations of contact: whether contact is
     * video capable etc.
     */
    public class ContactInformation {
        // may contain some more info like duplex video call etc in future.
        public boolean isVideoCapable = false;
        public boolean isVoiceCapable = false;

        public ContactInformation() {
        }

        public ContactInformation(boolean video, boolean voice) {
            isVideoCapable = video;
            isVoiceCapable = voice;
        }
    }

    /**
     * Query contacts presence. Fill contacts cache with video capability
     * information.
     */
    private void queryContactsPresence() {
        String[] projection = { Phone.CONTACT_ID, Phone._ID, Data.MIMETYPE,
                Data.DATA1, Data.DATA2 };
        String selection = Data.MIMETYPE + " IN ('" + MIMETYPE_VIDEO_SHARING
                + "','" + MIMETYPE_VOICE_SHARING + "')";
        Cursor cursor = mContext.getContentResolver().query(
                Data.CONTENT_URI, projection, selection, null, null);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    String number = cursor.getString(cursor.getColumnIndex(Data.DATA1));
                    long phoneId = cursor.getLong(cursor.getColumnIndex(Phone._ID));
                    long contactId = cursor.getLong(cursor.getColumnIndex(Phone.CONTACT_ID));
                    String mimeType = cursor.getString(cursor.getColumnIndex(Data.MIMETYPE));
                    LogUtils.d(TAG, "[queryContactsPresence]number="
                            + LogUtils.anonymize(number) +
                            ", phoneId=" + phoneId + ", contactId=" + contactId +
                            ", mimeType=" + mimeType);
                    boolean isNew = false;
                    synchronized (mContactsCache) {
                        ContactInformation info = mContactsCache.get(number);
                        if (info == null) {
                            isNew = true;
                            info = new ContactInformation();
                        }
                        if (MIMETYPE_VIDEO_SHARING.equals(mimeType)) {
                            info.isVideoCapable = true;
                        } else if (MIMETYPE_VOICE_SHARING.equals(mimeType)) {
                            info.isVoiceCapable = true;
                        } else {
                            continue;
                        }
                        if (isNew) {
                            Log.d(TAG, "[queryContactsPresence]add new:"
                                    + LogUtils.anonymize(number));
                            mContactsCache.put(number, info);
                        }
                    }
                }
            } else {
                Log.d(TAG, "[queryContactsPresence] cursor is null");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        //add fake data for UT
        if (IS_UT_MODE) {
            Log.d(TAG, "[queryContactsPresence] Insert test data.");
            mContactsCache.put(TEST_NUMBER_0, new ContactInformation(true, true));
            mContactsCache.put(TEST_NUMBER_1, new ContactInformation(true, true));
        }
    }

    /**
     * This method should only be called from ApiService, for APIs
     * initialization.
     *
     * @param context
     *            The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Log.d(TAG, "initialize() entry");
        boolean result = false;
        if (null != sInstance) {
            Log.w(TAG, "initialize() sInstance has existed, "
                    + "is it really the first time you call this method?");
            return true;
        }
        if (SystemProperties.get(PROPERTY_UCE_SUPPORT).toString()
                .equalsIgnoreCase("1")) {
            if (null != context) {
                PresenceApiManager apiManager = new PresenceApiManager(context);
                sInstance = apiManager;
                result =  true;
            } else {
                Log.e(TAG, "initialize() the context is null");
            }
        } else {
            Log.e(TAG, "initialize() no support presence");
        }
        return result;
    }

    /**
     * The listener interface for receiving myJoynService events. The class that
     * is interested in processing a myJoynService event implements this
     * interface, and the object created with that class is registered with a
     * component using the component's addMyRcsServiceListener method. When the
     * myJoynService event occurs, that object's appropriate method is invoked.
     *
     * @see MyJoynServiceEvent
     */
    public class MyRcsServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            try {
                Log.d(TAG, "onServiceConnected ViLTE");
                setServiceConnected(true);
                if (mNeedReconnect) {
                    mNeedReconnect = false;
                    Log.d(TAG, "onServiceDisConnected no need reconnect");
                }
                PresenceApiManager.this.mCapabilitiesApi
                        .addCapabilitiesListener(mMyCapabilitiesListener);
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        /**
         * On service disconnected.
         *
         * @param reasonCode Reason Code
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            try {
                Log.d(TAG, "onServiceDisConnected ViLTE");
                setServiceConnected(false);
                if (!mNeedReconnect) {
                    mNeedReconnect = true;
                    Log.d(TAG, "onServiceDisConnected need reconnect");
                }
                if (PresenceApiManager.this.mCapabilitiesApi != null) {
                    PresenceApiManager.this.mCapabilitiesApi
                            .removeCapabilitiesListener(mMyCapabilitiesListener);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }

    /**
     * The listener interface for receiving myCapabilities events. The class
     * that is interested in processing a myCapabilities event implements this
     * interface, and the object created with that class is registered with a
     * component using the component's addMyCapabilitiesListener method. When
     * the myCapabilities event occurs, that object's appropriate method is
     * invoked.
     *
     * @see MyCapabilitiesEvent
     */
    public class MyCapabilitiesListener extends CapabilitiesListener {

        /**
         * On capabilities received.
         *
         * @param contact
         *            the contact
         * @param capabilities
         *            the capabilities
         */
        @Override
        public void onCapabilitiesReceived(final String contact, Capabilities capabilities) {

            LogUtils.d(TAG, "onCapabilityReceived(), contact = " + LogUtils.anonymize(contact)
                    + ", video = " + capabilities.isIR94_VideoCallSupported()
                    + ", voice = " + capabilities.isIR94_VoiceCallSupported()
                    + ", duplex = " + capabilities.isIR94_DuplexModeSupported());
            if (contact == null || capabilities == null) {
                return;
            }
            boolean isNew = false;
            ContactInformation info = null;
            synchronized (mContactsCache) {
                info = mContactsCache.get(contact);
                if (info == null) {
                    Log.d(TAG, "cache does not exist, so create a object.");
                    info = new ContactInformation();
                    isNew = true;
                }
                if (isNew || info.isVideoCapable != capabilities.isIR94_VideoCallSupported()
                        || info.isVoiceCapable != capabilities.isIR94_VoiceCallSupported()) {
                    info.isVideoCapable = capabilities.isIR94_VideoCallSupported();
                    info.isVoiceCapable = capabilities.isIR94_VoiceCallSupported();
                    mContactsCache.put(contact, info);
                }
            }
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                if (listener != null) {
                    listener.onCapabilitiesChanged(contact, info);
                }
            }
        }

        @Override
        public void onErrorReceived(final String contact, int type, int status, String reason) {
            LogUtils.d(TAG, "[onErrorReceived], contact = " + LogUtils.anonymize(contact)
                    + ", type = " + type + ", status = " + status + ", reason = " + reason);
            //filter invalid
            if (contact == null) {
                return;
            }
            for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                if (listener != null) {
                    listener.onErrorReceived(contact, type, status, reason);
                }
            }
        }
    }

    /**
     * Creates a ContactId from number.
     *
     * @param contact from a trusted data
     * @return the ContactId contact id
     */
    private ContactId createContactIdFromNumber(String number) {
        try {
            ContactId id = ContactUtil.getInstance(mContext).formatContact(number);
            Log.d(TAG, "[createContactIdFromNumber]" + LogUtils.anonymize(number)
                    + " to " + id.toString());
            return id;
        } catch (Exception e) {
            e.printStackTrace();
            return new ContactId(number);
        }
    }

}

