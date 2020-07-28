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
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Looper;
import android.os.SystemProperties;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;
import android.util.Log;
import android.util.LruCache;

import com.gsma.services.rcs.RcsPermissionDeniedException;
//import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
//import com.gsma.services.rcs.RcsServiceNotAvailableException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;
import com.gsma.services.rcs.contact.ContactsProvider;

import java.util.ArrayList;
import java.util.List;

/**
 * This class manages the APIs which are used by plug-in, providing a convenient
 * way for API invocations.
 */
public class PresenceApiManager {
    public static final String TAG = "PHB/PresenceApiManager";
    private static PresenceApiManager sInstance = null;
    private static final int MAX_CACHE_SIZE = 2048;
    /**
     * MIME type for RCSE capabilities.
     */
    private static final String MIMETYPE_VIDEO_SHARING = ContactsProvider.MIME_TYPE_IR94_VIDEO_CALL;
    private static final int CONTACT_VIDEO_CAPABLE = 1;
    private static final int CONTACT_VIDEO_CAPABLE_DUPLEX = 2;
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
    private PresenceServiceStatusReceiver mServiceReceiver = null;

    private final LruCache<String, ContactInformation> mContactsCache =
        new LruCache<String, ContactInformation>(MAX_CACHE_SIZE);
    private final List<CapabilitiesChangeListener> mCapabilitiesChangeListenerList =
        new ArrayList<CapabilitiesChangeListener>();

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
        void onCapabilitiesChanged(String contact,
                ContactInformation info);

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
     * The class including some informations of contact: whether contact is
     * video capable etc.
     */
    public static class ContactInformation {
        public boolean isVideoCapable = false;
        // may contain some more info like duplex video call etc in future.

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
                           return;
                        }
                        ContactInformation info = new ContactInformation();
                        if (capabilities.isIR94_VideoCallSupported()) {
                            info.isVideoCapable = true;
                        } else {
                            info.isVideoCapable = false;
                        }
                        synchronized (mContactsCache) {
                            LogUtils.d(TAG, "getContactPresence: " + LogUtils.anonymize(number)
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
        Log.d(TAG, "getContactPresence exit");
    }

    /**
     * Query contacts presence. Fill contacts cache with video capability
     * information.
     */
    private void queryContactsPresence() {
        String[] projection = { Phone.CONTACT_ID, Phone._ID, Data.MIMETYPE,
                Data.DATA1, Data.DATA2 };
        String selection = Data.MIMETYPE + "=?";
        String[] selectionArgs = { MIMETYPE_VIDEO_SHARING};
        Cursor cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                projection, selection, selectionArgs, null);
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    String number = cursor.getString(cursor
                            .getColumnIndex(Data.DATA1));
                    long phoneId = cursor.getLong(cursor
                            .getColumnIndex(Phone._ID));
                    long contactId = cursor.getLong(cursor
                            .getColumnIndex(Phone.CONTACT_ID));
                    ContactInformation info = new ContactInformation();
                    info.isVideoCapable = true;
                    synchronized (mContactsCache) {
                        LogUtils.d(TAG, "queryContactsPresence number: "
                                + LogUtils.anonymize(number));
                        mContactsCache.put(number, info);
                    }
                }
            } else {
                Log.d(TAG, "cursor is null");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /**
     * Is service connected.
     *
     * @return mConnected
     */
    public boolean isServiceConnected() {
        Log.d(TAG, "isServiceConnected: " + mConnected);
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
     * Query contact presence.
     *
     * @param number the number
     * @param force fore query or not
     */
    public void requestContactPresence(final String number, final boolean force) {
        LogUtils.d(TAG, "requestContactPresence, number: " + LogUtils.anonymize(number)
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
                        //} catch (RcsServiceNotAvailableException e) {
                        //    e.printStackTrace();
                        //} catch (RcsServiceException e) {
                        //    e.printStackTrace();
                        //}
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        // put initial data into cache to avoid multi-request.
                        synchronized (mContactsCache) {
                            if (mContactsCache.get(formatNumber) == null) {
                                LogUtils.d(TAG, "requestContactPresence cache: "
                                        + LogUtils.anonymize(formatNumber));
                                ContactInformation info = new ContactInformation();
                                info.isVideoCapable = false;
                                mContactsCache.put(formatNumber, info);
                            }
                        }
                    }
                }
            });
        }
        Log.d(TAG, "requestContactPresence exit");
    }

    /**
     * Query a series of phone number.
     *
     * @param numbers
     *            The phone numbers list need to query
     */
    public void requestNumbersPresence(final List<String> numbers) {
        LogUtils.d(TAG, "requestNumbersPresence entry, numbers: "
                + LogUtils.anonymize(numbers));
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                if (mCapabilitiesApi != null) {
                    for (String number : numbers) {
                        try {
                            mCapabilitiesApi.requestContactCapabilities(
                                    createContactIdFromNumber(number));
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        });
    }

    /**
     * Obtain the phone numbers from a specific contact id.
     *
     * @param contactId
     *            The contact id
     * @return The phone numbers of the contact id
     */
    public List<String> getNumbersByContactId(long contactId) {
        Log.d(TAG, "getNumbersByContactId entry, contact id is: " + contactId);
        List<String> list = new ArrayList<String>();
        String[] projection = { Phone.NUMBER };
        String selection = Phone.CONTACT_ID + "=? ";
        String[] selectionArgs = { Long.toString(contactId) };
        Cursor cur = mContext.getContentResolver().query(Phone.CONTENT_URI,
                projection, selection, selectionArgs, null);
        try {
            if (cur != null) {
                while (cur.moveToNext()) {
                    String number = cur.getString(0);
                    if (!TextUtils.isEmpty(number)) {
                        list.add(number.replace(" ", ""));
                        LogUtils.d(TAG, "getNumbersByContactId add number "
                                + LogUtils.anonymize(number.replace(" ", "")));
                    }
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        return list;
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
        Log.d(TAG, "PresenceApiManager() ViLTE, context = " + context);
        mContext = context;
        // connect to capability TAPI
        mMyCapabilitiesListener = new MyCapabilitiesListener();
        MyRcsServiceListener myRcsServiceListener = new MyRcsServiceListener();
        mCapabilitiesApi = new CapabilityService(context, myRcsServiceListener);
        mCapabilitiesApi.connect();
        // fill cache information for video capability
        initData();
        mServiceReceiver = new PresenceServiceStatusReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CORE_SERVICE_CURRENT_STATE_NOTIFICATION);
        mContext.registerReceiver(mServiceReceiver, intentFilter);
    }


   /**
    * MyRcsServiceListener.
    */
    public class MyRcsServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            try {
                Log.d(TAG, "onServiceConnected");
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
         * @param reasonCode ReasonCode
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            try {
                Log.d(TAG, "onServiceDisConnected");
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
        public void onCapabilitiesReceived(final String contact,
                Capabilities capabilities) {

            LogUtils.d(TAG, "onCapabilityReceived(), contact = " + LogUtils.anonymize(contact)
                    + ", video = " + capabilities.isIR94_VideoCallSupported()
                    + ", voice = " + capabilities.isIR94_VoiceCallSupported()
                    + ", duplex = " + capabilities.isIR94_DuplexModeSupported());
            if (null != contact && capabilities != null) {
                ContactInformation info = mContactsCache.get(contact);
                if (info == null) {
                    Log.d(TAG, "cache does not exist, so create a object.");
                    info = new ContactInformation();
                    if (capabilities.isIR94_VideoCallSupported() == true) {
                        info.isVideoCapable = true;
                        synchronized (mContactsCache) {
                            mContactsCache.put(contact, info);
                        }
                        for(CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList){
                            if (listener != null) {
                                Log.d(TAG, "Notify the listener");
                                listener.onCapabilitiesChanged(contact, info);
                            }
                        }
                    }

                } else {
                    Log.d(TAG, "cache exists");
                    if (info.isVideoCapable != capabilities
                            .isIR94_VideoCallSupported()) {
                        info.isVideoCapable = capabilities
                                .isIR94_VideoCallSupported();
                        synchronized (mContactsCache) {
                            mContactsCache.put(contact, info);
                        }
                        Log.d(TAG, "put capability into cache");
                        for(CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList){
                            if (listener != null) {
                                Log.d(TAG, "Notify the listener");
                                listener.onCapabilitiesChanged(contact, info);
                            }
                        }
                    }
                }
            }
        }

        @Override
        public void onErrorReceived(final String contact, int type, int status, String reason) {
        }
    }

    /**
     * Clear all the information in the mContactsCache.
     */
    public void cleanContactCache() {
        Log.d(TAG, "cleanContactCache() entry");
        mContactsCache.evictAll();
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

    /**
     * Creates a ContactId from number.
     *
     * @param contact from a trusted data
     * @return the ContactId contact id
     */
    private ContactId createContactIdFromNumber(String number) {
        try {
            return ContactUtil.getInstance(mContext).formatContact(number);
        } catch (RcsPermissionDeniedException e) {
            e.printStackTrace();
            return new ContactId(number);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            return new ContactId(number);
        }
    }

   /**
    * Presence Service Status Receiver.
    */
    private class PresenceServiceStatusReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive(), context = " + context);
            if (intent != null) {
                String action = intent.getAction();
                Log.d(TAG, "action = " + action);
                if (CORE_SERVICE_CURRENT_STATE_NOTIFICATION.equals(action)) {
                    int status = intent.getIntExtra(
                            CORE_SERVICE_STATUS, CORE_SERVICE_INVALID);
                    handleServiceStatusChanged(status);
                }
            }
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

}
