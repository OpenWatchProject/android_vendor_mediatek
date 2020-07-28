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

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.LruCache;
import android.widget.Toast;

import com.mediatek.rcse.service.IApiServiceWrapper;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.gsma.services.rcs.contact.ContactService;
import com.mediatek.rcse.plugin.contacts.ContactExtention.OnPresenceChangedListener;
import com.mediatek.rcse.service.binder.IRemoteBlockingRequest;
import com.mediatek.rcse.settings.RcsSettings;
//import com.orangelabs.rcs.service.RcsCoreService;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;

/**
 * This class manages the APIs which are used by plug-in, providing a convenient
 * way for API invocations.
 */
public class PluginApiManager {
    public static final String TAG = "PluginApiManager";

    private IRemoteBlockingRequest mCoreApi = null;
    private static PluginApiManager sInstance = null;
    private boolean mNeedReconnectManagedAPi = false;
    private ManagedRegistrationApi mManagedRegistrationApi = null;
    private MyCapabilitiesListener mMyCapabilitiesListener = null;
    private RegistrationApi mRegistrationApi = null;
    private CapabilityService mCapabilitiesApi = null;
    private ContactService mContactAPI = null;
    private boolean mIsRegistered = false;
    private Context mContext = null;
    private static final int MAX_CACHE_SIZE = 2048;
    private final LruCache<String, ContactInformation> mContactsCache =
            new LruCache<String, ContactInformation>(MAX_CACHE_SIZE);
    private final LruCache<Long, List<String>> mCache =
            new LruCache<Long, List<String>>(MAX_CACHE_SIZE);
    private final ConcurrentHashMap<String, Integer> mUnreadMessageCountMap =
            new ConcurrentHashMap<String, Integer>();
    private static final String CONTACT_CAPABILITIES =
            "com.orangelabs.rcs.capability.CONTACT_CAPABILITIES";
    private static final String CONTACT_UNREAD_NUMBER =
            "com.mediatek.action.UNREAD_CHANGED_CONTACT";
    public static final String RCS_CONTACT_UNREAD_NUMBER_CHANGED =
            "android.intent.action.RCS_CONTACT_UNREAD_NUMBER_CHANGED";

    private final List<CapabilitiesChangeListener> mCapabilitiesChangeListenerList =
            new ArrayList<CapabilitiesChangeListener>();
    private final List<RegistrationListener> mRegistrationListeners =
            new CopyOnWriteArrayList<RegistrationListener>();
    private final List<RichCallApiListener> mRichCallApiListeners =
            new CopyOnWriteArrayList<RichCallApiListener>();
    private Cursor mCursor = null;
    private List<Long> mQueryOngoingList = new ArrayList<Long>();
    private final ConcurrentHashMap<Long, OnPresenceChangedListener> mPresenceListeners =
            new ConcurrentHashMap<Long, OnPresenceChangedListener>();
    /**
     * MIME type for RCSE capabilities.
     */
    private static final String MIMETYPE_RCSE_CAPABILITIES =
            "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    /**
     * MIME type for a RCS capable contact.
     */
    private static final String MIMETYPE_RCS_CONTACT =
            "vnd.android.cursor.item/com.orangelabs.rcs.rcs-status";
    private static final int RCS_CONTACT = 2;
    private static final int RCS_CAPABLE_CONTACT = 0;
    private static volatile ContactUtil mContactUtil;

    /**
     * The listener interface for receiving callLogPresence events.
     * The class that is interested in processing a callLogPresence
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addCallLogPresenceListener method. When
     * the callLogPresence event occurs, that object's appropriate
     * method is invoked.
     *
     * @see CallLogPresenceEvent
     */
    public interface CallLogPresenceListener {

        /**
         * On presence changed.
         *
         * @param number the number
         * @param presence the presence
         */
        void onPresenceChanged(String number, int presence);
    }

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
         * @param contact the contact
         * @param contactInformation the contact information
         */
        void onCapabilitiesChanged(String contact,
                ContactInformation contactInformation);

        /**
         * Called when CapabilityApi connected status is changed.
         *
         * @param isConnected
         *            True if CapabilityApi is connected.
         */
        void onApiConnectedStatusChanged(boolean isConnected);
    }

    /**
     * Add presence changed listener.
     *
     * @param listener
     *            The presence changed listener.
     * @param contactId
     *            The contact id.
     */
    public void addOnPresenceChangedListener(
            OnPresenceChangedListener listener, long contactId) {
        mPresenceListeners.put(contactId, listener);
    }

    /**
     * Register the CapabilitiesChangeListener.
     *
     * @param listener            The CapabilitiesChangeListener used to register
     */
    public void addCapabilitiesChangeListener(
            CapabilitiesChangeListener listener) {
        Logger.v(TAG, "addCapabilitiesChangeListener(), listener = " + listener);
        mCapabilitiesChangeListenerList.add(listener);
    }

    /**
     * Unregister the CapabilitiesChangeListener.
     *
     * @param listener            The CapabilitiesChangeListener used to unregister
     */
    public void removeCapabilitiesChangeListener(
            CapabilitiesChangeListener listener) {
        Logger.v(TAG, "removeCapabilitiesChangeListener(), listener = "
                + listener);
        mCapabilitiesChangeListenerList.remove(listener);
    }

    /**
     * The RegistrationListener defined as a listener to notify the specify
     * observer that the registration status has been changed and
     * RegistrationAPi connected status.
     *
     * @see RegistrationEvent
     */
    public interface RegistrationListener {
        /**
         * Called when RegistrationApi connected status is changed.
         *
         * @param isConnected
         *            True if RegistrationApi is connected
         */
        void onApiConnectedStatusChanged(boolean isConnected);

        /**
         * Called when the status of RCS-e account is registered.
         *
         * @param status
         *            Current status of RCS-e account.
         */
        void onStatusChanged(boolean status);

        /**
         * Called when the rcse core service status has been changed.
         *
         * @param status
         *            Current status of rcse core service.
         */
        void onRcsCoreServiceStatusChanged(int status);

    }

    /**
     * Register the RegistrationListener.
     *
     * @param listener            The RegistrationListener used to register
     */
    public void addRegistrationListener(RegistrationListener listener) {
        Logger.v(TAG, "addRegistrationStatusListener(), listener = " + listener);
        mRegistrationListeners.add(listener);
    }

    /**
     * Unregister the RegistrationListener.
     *
     * @param listener            The RegistrationListener used to unregister
     */
    public void removeRegistrationListener(RegistrationListener listener) {
        Logger.v(TAG, "removeRegistrationStatusListener(), listener = "
                + listener);
        mRegistrationListeners.remove(listener);
    }

    /**
     * The RichCallApiListener defined as a listener to notify the specify
     * observer that RichCallApiListener connected status.
     *
     * @see RichCallApiEvent
     */
    public interface RichCallApiListener {
        /**
         * Called when CapabilityApi connected status is changed.
         *
         * @param isConnected
         *            True if CapabilityApi is connected.
         */
        void onApiConnectedStatusChanged(boolean isConnected);
    }

    /**
     * Register the RichCallApiListener.
     *
     * @param listener            The RichCallApiListener used to register
     */
    public void addRichCallApiListener(RichCallApiListener listener) {
        Logger.v(TAG, "addRichCallApiListener(), listener = " + listener);
        mRichCallApiListeners.add(listener);
    }

    /**
     * Unregister the RichCallApiListener.
     *
     * @param listener            The RichCallApiListener used to unregister
     */
    public void removeRichCallApiListener(RichCallApiListener listener) {
        Logger.v(TAG, "removeRichCallApiListener(), listener = " + listener);
        mRichCallApiListeners.remove(listener);
    }

    /**
     * Get the instance of RegistrationApi.
     *
     * @return The instance of RegistrationApi
     */
    public RegistrationApi getRegistrationApi() {
        Logger.v(TAG, "getRegistrationApi(), mRegistrationApi = "
                + mRegistrationApi);
        return mRegistrationApi;
    }

    /**
     * Get the instance of CapabilityApi.
     *
     * @return The instance of CapabilityApi
     */
    public CapabilityService getCapabilityApi() {
        Logger.v(TAG, "getCapabilityApi(), mCapabilitiesApi = "
                + mCapabilitiesApi);
        return mCapabilitiesApi;
    }

    /**
     * Get the instance of CapabilityApi.
     *
     * @return The instance of CapabilityApi
     */
    public ContactService getContactsApi() {
        Logger.v(TAG, "getContactsApi(), mCapabilitiesApi = "
                + mContactAPI);
        return mContactAPI;
    }

    /**
     * This class defined some instance used in RCS-e action.
     */
    public static class RcseAction {
        /**
         * Defined as the IM action.
         */
        public static final String IM_ACTION = "com.mediatek.rcse.action.CHAT_INSTANCE_MESSAGE";

        /**
         * Defined as the file transfer action.
         */
        public static final String FT_ACTION = "com.mediatek.rcse.action.CHAT_FILE_TRANSFER";

        public static final String SHARE_URL_ACTION = "com.mediatek.rcse.action.CHAT_SEND_MESSAGE";

        /**
         * Defined as the select contacts action.
         */
        public static final String SELECT_PLUGIN_CONTACT_ACTION =
                "com.mediatek.rcse.action.PluginSelectContact";

        /**
         * Defined as the proxy activity action.
         */
        public static final String PROXY_ACTION = "com.mediatek.rcse.action.PROXY";

        /**
         * Defined as the contacts selection activity action.
         */
        public static final String SELECT_CONTACT_ACTION =
                "com.mediatek.rcse.action.SELECT_CONTACT_BY_MULTIMEDIA";

        /**
         * Defined single file transfer action sent by File
         * manager,Gallery,Camera.
         */
        public static final String SINGLE_FILE_TRANSFER_ACTION = "android.intent.action.SEND";

        /**
         * Defined multiple file transfer action sent by File
         * manager,Gallery,Camera.
         */
        public static final String MULTIPLE_FILE_TRANSFER_ACTION =
                "android.intent.action.SEND_MULTIPLE";

        /**
         * Data name for display name.
         */
        public static final String CONTACT_NAME = "rcs_display_name";

        /**
         * Data name for phone number.
         */
        public static final String CONTACT_NUMBER = "rcs_phone_number";

        /**
         * Data name for Im or Ft capability.
         */
        public static final String CAPABILITY_SUPPORT = "isSupported";

        /**
         * Data name for single file URI.
         */
        public static final String SINGLE_FILE_URI = "rcs_single_file_uri";

        /**
         * Data name for multiple file URI.
         */
        public static final String MULTIPLE_FILE_URI = "rcs_multiple_file_uri";

        public static final String SHARE_URL = "rcs_share_url";

        public static final String GROUP_CHAT_PARTICIPANTS = "rcs_group_chat_participants";

    }

    /**
     * The class including some informations of contact: whether it is an Rcse
     * contact, the capabilities of IM, file transfer,CS call,image and video
     * share.
     */
    public static class ContactInformation {
        public int isRcsContact = 0; // 0 indicate not Rcs, 1 indicate Rcs
        public boolean isImSupported = false;
        public boolean isFtSupported = false;
        public boolean isImageShareSupported = false;
        public boolean isVideoShareSupported = false;
        public boolean isCsCallSupported = false;
        public int unreadNumber = 0;

        @Override
        public String toString() {
            return "(" + isRcsContact + ":" + isImSupported + ":" + isFtSupported
                + ":" + isImageShareSupported + ":" + isVideoShareSupported
                + ":" + isCsCallSupported + ":" + unreadNumber + ")";
        }
    }

    /**
     * Get the presence of number.
     *
     * @param number
     *            The number whose presence to be queried.
     * @return The presence of the number.
     */
    public int getContactPresence(String number) {
        ContactInformation info = null;
        if(number != null){
            //remove separator
            number = number.trim();
            number = PhoneNumberUtils.stripSeparators(number);
            number = number.replaceAll("[\\s\\-()]", "");
        }

        if (mContactAPI != null) {
            try {
                if (!mContactAPI.isRcsValidNumber(number)) {
                    return 0;
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }
        }

        final String tempNumber = number;
        synchronized (mContactsCache) {
            info = mContactsCache.get(number);
        }
        if (info != null) {
            Logger.v(TAG, "getContactPresence number " + number
                    + " with cacheable: " + info.isRcsContact);
            return info.isRcsContact;
        } else {
            Logger.v(TAG,
                    "getContactPresence uncacheable, retry to query presence");
            ContactInformation defaultInfo = new ContactInformation();
            synchronized (mContactsCache) {
                mContactsCache.put(number, defaultInfo);
            }
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    queryContactPresence(tempNumber);
                }
            });
        }
        return 0;
    }

    /**
     * Gets the file transfer support.
     *
     * @param number the number
     * @return the file transfer support
     */
    public boolean getFileTransferSupport(String number) {
        Logger.d(TAG, "getFileTransferSupport entry, number is " + number);
        ContactInformation info = null;
        boolean isFtSupported = false;
        synchronized (mContactsCache) {
            info = mContactsCache.get(number);
        }
        if (info == null) {
            isFtSupported = false;
        } else {
            isFtSupported = info.isFtSupported;
        }
        Logger.d(TAG, "getFileTransferSupport exit, returning " + isFtSupported);
        return isFtSupported;
    }

    /**
     * Block contact.
     *
     * @param contact the contact
     * @param status the status
     * @return true, if successful
     * @throws RemoteException the remote exception
     */
    public boolean blockContact(String contact, boolean status)
            throws RemoteException {
        // call to ApiService through aidl
        Logger.d(TAG, "blockContact() entry with status " + status
                + "coreApi = " + mCoreApi);
        boolean isBlockedSuccess = false;
        if (mCoreApi != null) {
            isBlockedSuccess = mCoreApi.blockContact(contact, status);
        }
        Logger.d(TAG, "blockContact() exit with value " + isBlockedSuccess);
        return isBlockedSuccess;
    }

    /**
     * Gets the blocked status.
     *
     * @param contact the contact
     * @return the blocked status
     * @throws RemoteException the remote exception
     */
    public boolean getBlockedStatus(String contact) throws RemoteException {
        // call to ApiService through aidl
        Logger.d(TAG, "getBlockedStatus() entry " + mCoreApi);
        boolean isBlockedAlready = false;
        if (mCoreApi != null) {
            isBlockedAlready = mCoreApi.getBlockedStatus(contact);
        }
        Logger.d(TAG, "getBlockedStatus() exit with value " + isBlockedAlready);
        return isBlockedAlready;
    }

    /**
     * Core service API connection.
     */
    private ServiceConnection mApiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            /*mCoreApi = IRemoteBlockingRequest.Stub.asInterface(service);
            Logger.d(TAG, "service connecting for blocking request " + mCoreApi);*/
            IApiServiceWrapper apiServiceWrapperBinder;
            apiServiceWrapperBinder = IApiServiceWrapper.Stub.asInterface(service);
            IBinder IBlockingBinder = null;
            try {
               IBlockingBinder = apiServiceWrapperBinder.getBlockingBinder();
            } catch (RemoteException e1) {
               e1.printStackTrace();
            }
            mCoreApi = IRemoteBlockingRequest.Stub
                   .asInterface(IBlockingBinder);
            Logger.v(TAG, "onServiceConnected()  IBlockingBinder" + IBlockingBinder + "=mRegistrationStatus=" + mCoreApi);

        }

        public void onServiceDisconnected(ComponentName className) {

            mCoreApi = null;
        }
    };

    /**
     * Inits the data.
     */
    private void initData() {
        Logger.d(TAG, "initData entry");
        Thread thread = new Thread() {
            public void run() {
                Looper.prepare();
                registerObserver();
                queryContactsPresence();
                IntentFilter filter = new IntentFilter();
                filter.addAction(CONTACT_CAPABILITIES);
                filter.addAction(CONTACT_UNREAD_NUMBER);
                filter.addAction(Intents.Client.SERVICE_UP);
                mContext.registerReceiver(mBroadcastReceiver, filter);
            }
        };
        thread.start();
        Logger.d(TAG, "initData exit");
    }

    /**
     * Gets the unread message count.
     *
     * @param contact the contact
     * @return the unread message count
     */
    public int getUnreadMessageCount(String contact) {
        Logger.d(TAG, "getUnreadMessageCount " + contact);
        if (mUnreadMessageCountMap != null) {
            if (mUnreadMessageCountMap.containsKey(contact)) {
                return mUnreadMessageCountMap.get(contact);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    /**
     * Get presence of contact id.
     *
     * @param contactId
     *            The contact id whose presence to be queried.
     * @return The presence of the contact.
     */
    public int getContactPresence(final long contactId) {
        final List<String> numbers = mCache.get(contactId);
        Logger.d(TAG, "getContactPresence() entry, contactId: " + contactId
                + " numbers: " + numbers);
        if (numbers != null) {
            synchronized (mPresenceListeners) {
                mPresenceListeners.remove(contactId);
            }
            ContactInformation info = null;
            for (String number : numbers) {
                if(number != null){
                    //remove separator
                    number = number.trim();
                    number = PhoneNumberUtils.stripSeparators(number);
                    number = number.replaceAll("[\\s\\-()]", "");
                }
                info = mContactsCache.get(number);
                Logger.d(TAG, "getContactPresence() number after trim:"
                        + number + ", info:"  + info);
                if (info != null && info.isRcsContact == 1) {
                    return 1;
                }
            }
            if (mQueryOngoingList.contains(contactId)) {
                Logger.d(TAG, "getContactPresence contact id " + contactId
                        + " query presence operation is ongoing");
                return 0;
            }
            //mQueryOngoingList.add(contactId);
            if (info == null || (info != null && info.isRcsContact == 0)) {
                Logger.d(TAG,
                        "getContactPresence info is null, so retry to query");
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        queryPresence(contactId, numbers);
                        //mQueryOngoingList.remove(contactId);
                    }
                });
            }
        } else {
                    final List<String> list = getNumbersByContactId(contactId);
                    queryPresence(contactId, list);
                    ContactInformation info = null;
                    for (String number : list) {
                        if(number != null){
                            //remove separator
                            number = number.trim();
                            number = PhoneNumberUtils.stripSeparators(number);
                            number = number.replaceAll("[\\s\\-()]", "");
                }
                Logger.d(TAG, "getContactPresence contact mContactsCache "
                        + mContactsCache + " & list" + list);
                        info = mContactsCache.get(number);
                        if (info != null && info.isRcsContact == 1) {
                            return 1;
                        }
                        mCache.put(contactId, list);
                }

        }
        return 0;
    }

    /**
     * Register observer.
     */
    private void registerObserver() {
        Logger.d(TAG, "registerObserver entry");
        if (mCursor != null && mCursor.isClosed()) {
            Logger.d(TAG, "registerObserver close cursor");
            mCursor.close();
        }

        // Query contactContracts phone database
        try {
        mCursor = mContext.getContentResolver().query(Phone.CONTENT_URI, null,
                null, null, null);
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            //Toast.makeText(mContext, "Permission Denied. You can change it in Settings->Apps.", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }
        if (mCursor != null) {
            // Register content observer
            Logger.d(TAG, "registerObserver begin to registerContentObserver");
            mCursor.registerContentObserver(new ContentObserver(new Handler()) {
                @Override
                public void onChange(boolean selfChange) {
                    super.onChange(selfChange);
                    Logger.d(TAG, "onChange entry" + selfChange);
                    if (mCache == null) {
                        Logger.d(TAG, "onChange mCache is null");
                        return;
                    }
                    Map<Long, List<String>> map = mCache.snapshot();
                    if (map != null) {
                        Set<Long> keys = map.keySet();
                        for (Long key : keys) {
                            getNumbersByContactId(key);
                        }
                    } else {
                        Logger.d(TAG, "onChange map is null");
                    }
                }
            });
        } else {
            Logger.d(TAG, "registerObserver mCursor is null");
        }
        Logger.d(TAG, "registerObserver exit");
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            String action = intent.getAction();
            if (CONTACT_UNREAD_NUMBER.equals(action)) {
                String contact = intent.getStringExtra("number");
                boolean isUpdateAll = intent
                        .getBooleanExtra("updateall", false);
                int count = intent.getIntExtra("count", 0);
                Logger.d(TAG, "onReceive(),unread number intent received "
                        + contact);
                // int unreadCount = 0;
                if (mUnreadMessageCountMap != null && contact != null) {
                    /*
                     * if(mUnreadMessageCountMap.containsKey(contact)) {
                     * unreadCount = mUnreadMessageCountMap.get(contact); }
                     */
                    if (isUpdateAll == true) {
                        synchronized (mUnreadMessageCountMap) {
                            mUnreadMessageCountMap.put(contact, 0);
                        }
                    } else {
                        synchronized (mUnreadMessageCountMap) {
                            mUnreadMessageCountMap.put(contact, count);
                        }
                    }
                } else {
                    Logger.d(TAG, "onReceive(),mUnreadMessageCountMap is null");
                }

                // broadcast to contact app about this
                Intent intentUnread = new Intent(
                        RCS_CONTACT_UNREAD_NUMBER_CHANGED);
                mContext.sendBroadcast(intentUnread);

            } else if (CONTACT_CAPABILITIES.equals(action)) {

                String number = intent.getStringExtra("contact");
                Capabilities capabilities = intent
                        .getParcelableExtra("capabilities");
                ContactInformation info = new ContactInformation();
                if (capabilities == null) {
                    Logger.d(TAG, "onReceive(),capabilities is null");
                    return;
                }
                info.isRcsContact = capabilities.isSupportedRcseContact() ? 1
                        : 0;
                info.isImSupported = capabilities.isImSessionSupported();
                info.isFtSupported = capabilities.isFileTransferSupported();
                info.isImageShareSupported = capabilities
                        .isImageSharingSupported();
                info.isVideoShareSupported = capabilities
                        .isVideoSharingSupported();
                info.isCsCallSupported = capabilities.isCsVideoSupported();
                Logger.d(TAG, "onReceive getRcseContact contact is: " + number
                        + " " + info.isRcsContact);
                synchronized (mContactsCache) {
                    mContactsCache.put(number, info);
                }
                for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                    if (listener != null) {
                        listener.onCapabilitiesChanged(number, info);
                        return;
                    }
                }
            } else if (Intents.Client.SERVICE_UP.equals(action)) {
                Logger.d(TAG, "onreceive(), rcscoreservice action");
                mCapabilitiesApi = new CapabilityService(mContext,
                        new MyRcsServiceListener());
                mCapabilitiesApi.connect();
            }
        }
    };

    /**
     * Query contacts presence.
     */
    private void queryContactsPresence() {
        Logger.d(TAG, "queryContactsPresence entry");
        List<String> rcsNumbers = new ArrayList<String>();
        String[] projection = { Phone.CONTACT_ID, Phone._ID,
                Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DISPLAY_NAME };
        // Filter the mime types
        String selection = Data.MIMETYPE + "=? AND " + "("
                + Data.DATA2 + "=? OR " + Data.DATA2 + "=? )";
        String[] selectionArgs = { MIMETYPE_RCS_CONTACT,
                Long.toString(RCS_CONTACT), Long.toString(RCS_CAPABLE_CONTACT) };
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                projection, selection, selectionArgs, null);
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            //Toast.makeText(mContext, "Permission Denied. You can change it in Settings->Apps.", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }

        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {

                    ContactInformation info = new ContactInformation();
                    info.isRcsContact = 1;
                    String number = cursor.getString(cursor
                            .getColumnIndex(Data.DATA1));
                    Logger.d(TAG, "queryContactsPresence number is: " + number);
                    synchronized (mContactsCache) {
                        mContactsCache.put(number, info);
                        Logger.d(TAG, "queryContactsPresence mContactsCache exit, info: " + info);
                    }
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Logger.d(TAG, "getRcsContacts exit, size: " + rcsNumbers.size());
    }

    /**
     * Query contact presence.
     *
     * @param number the number
     */
    private void queryContactPresence(final String number) {
        Logger.d(TAG, "queryContactsPresence entry, number is " + number);
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "queryContactPresence() in async task");
                if (mCapabilitiesApi == null) {
                    Logger.d(TAG,
                            "queryContactPresence mCapabilitiesApi is null, number is "
                                    + number);
                } else {
                    try {
                        Capabilities capabilities = mCapabilitiesApi
                                .getContactCapabilities(createContactIdFromTrustedData(number));
                        mCapabilitiesApi.requestContactCapabilities(createContactIdFromTrustedData(number));
                        ContactInformation info = new ContactInformation();
                        if (capabilities == null) {
                            capabilities = new Capabilities(false, false,
                                    false, false, false, false, false, null,
                                    false, false, false, false, false);
                        }
                        info.isRcsContact = capabilities
                                .isSupportedRcseContact() ? 1 : 0;
                        info.isImSupported = capabilities
                                .isImSessionSupported();
                        info.isFtSupported = capabilities
                                .isFileTransferSupported();
                        info.isImageShareSupported = capabilities
                                .isImageSharingSupported();
                        info.isVideoShareSupported = capabilities
                                .isVideoSharingSupported();
                        info.isCsCallSupported = capabilities
                                .isCsVideoSupported();
                        synchronized (mContactsCache) {
                            mContactsCache.put(number, info);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                Logger.d(TAG, "queryContactPresence() leave async task");
            }
        });
        Logger.d(TAG, "queryContactPresence exit");
    }

    /**
     * Creates a ContactId from a trusted data
     * (By trusted data, we mean RCS providers)
     *
     * @param contact from a trusted data
     * @return the ContactId contact id
     */
    public ContactId createContactIdFromTrustedData(String contact) {
        synchronized (ContactUtil.class) {
            if (mContactUtil == null) {
                mContactUtil = ContactUtil.getInstance(mContext);
            }
        }
        try {
            return mContactUtil.formatContact(contact);
        } catch (RcsPermissionDeniedException e) {
            /*
             * This exception should not occur since core stack cannot be started if country code
             * cannot be resolved.
             */
            String errorMessage = "Failed to convert phone number '" + contact
                    + "' into contactId!";
            throw new IllegalStateException(errorMessage, e);
        }
    }

    /**
     * Query a series of phone number.
     *
     * @param numbers            The phone numbers list need to query
     */
    public void queryNumbersPresence(List<String> numbers) {
        Logger.d(TAG, "queryNumbersPresence entry, numbers: " + numbers);

        if (mCapabilitiesApi != null) {
            for (String number : numbers) {
                if(number != null){
                    //remove separator
                    number = number.trim();
                    number = PhoneNumberUtils.stripSeparators(number);
                    number = number.replaceAll("[\\s\\-()]", "");
                }
                Logger.d(TAG, "queryNumbersPresence number: " + number);

                if (mContactAPI != null) {
                    try {
                        if (!mContactAPI.isRcsValidNumber(number)) {
                            continue;
                        }
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }

                ContactInformation info = new ContactInformation();
                try {
                    Capabilities capabilities = mCapabilitiesApi
                            .getContactCapabilities(createContactIdFromTrustedData(number));

                    if(!(RcsSettings.getInstance().isSupportOP07() || RcsSettings.getInstance().isSupportOP08()))
                        mCapabilitiesApi.requestContactCapabilities(createContactIdFromTrustedData(number));

                    if (capabilities == null) {
                        capabilities = new Capabilities(false, false, false,
                                false, false, false, false, null, false, false,
                                false, false, false);
                    }
                    info.isRcsContact = capabilities.isSupportedRcseContact() ? 1
                            : 0;
                    info.isImSupported = capabilities.isImSessionSupported();
                    info.isFtSupported = capabilities.isFileTransferSupported();
                    info.isImageShareSupported = capabilities
                            .isImageSharingSupported();
                    info.isVideoShareSupported = capabilities
                            .isVideoSharingSupported();
                    info.isCsCallSupported = capabilities.isCsVideoSupported();
                    ContactInformation cacheInfo = mContactsCache.get(number);
                    if (cacheInfo == null || (cacheInfo != null && cacheInfo.isRcsContact == 0)) {
                        synchronized (mContactsCache) {
                            mContactsCache.put(number, info);
                        }
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * Query a series of phone number.
     *
     * @param numbers            The phone numbers list need to query
     */
    public int queryNumberPresence(String number) {
        Logger.d(TAG, "queryNumberPresence entry, number: " + number
                + ", mCapabilitiesApi= " + mCapabilitiesApi);
        if (mCapabilitiesApi != null) {
                if(number != null){
                    //remove separator
                    number = number.trim();
                    number = PhoneNumberUtils.stripSeparators(number);
                    number = number.replaceAll("[\\s\\-()]", "");
                }
                Logger.d(TAG, "queryNumberPresence number: " + number);
                ContactInformation info = new ContactInformation();
                try {
                    Capabilities capabilities = mCapabilitiesApi
                            .getContactCapabilities(createContactIdFromTrustedData(number));
                    if(!(RcsSettings.getInstance().isSupportOP07() || RcsSettings.getInstance().isSupportOP08()))
                        mCapabilitiesApi.requestContactCapabilities(createContactIdFromTrustedData(number));
                    if (capabilities == null) {
                        capabilities = new Capabilities(false, false, false,
                                false, false, false, false, null, false, false,
                                false, false, false);
                    }
                    info.isRcsContact = capabilities.isSupportedRcseContact() ? 1
                            : 0;
                    info.isImSupported = capabilities.isImSessionSupported();
                    info.isFtSupported = capabilities.isFileTransferSupported();
                    info.isImageShareSupported = capabilities
                            .isImageSharingSupported();
                    info.isVideoShareSupported = capabilities
                            .isVideoSharingSupported();
                    info.isCsCallSupported = capabilities.isCsVideoSupported();
                    ContactInformation cacheInfo = mContactsCache.get(number);
                    if (cacheInfo == null || (cacheInfo != null && cacheInfo.isRcsContact == 0)) {
                    synchronized (mContactsCache) {
                        mContactsCache.put(number, info);
                    }
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
                Logger.d(TAG, "queryNumberPresence number: " + number + "status=" + info.isRcsContact);
                return info.isRcsContact;
        }
        Logger.d(TAG, "queryNumberPresence number: " + number + "status=" + 0);
        return 0;
    }

    /**
     * Query a series of phone number.
     *
     * @param contactId            The contact id
     * @param numbers            The phone numbers list need to query
     */
    private void queryPresence(long contactId, List<String> numbers) {
        Logger.d(TAG, "queryPresence() entry, contactId: " + contactId
                + " numbers: " + numbers + " mCapabilitiesApi: "
                + mCapabilitiesApi);
        if (mCapabilitiesApi != null) {
            boolean needNotify = false;
            for (String number : numbers) {
                if(number != null){
                    //remove separator
                    number = number.trim();
                    number = PhoneNumberUtils.stripSeparators(number);
                    number = number.replaceAll("[\\s\\-()]", "");
                }
                ContactInformation info = new ContactInformation();
                Capabilities capabilities = null;
                ContactInformation cachedInfo = mContactsCache.get(number);
                if (cachedInfo == null || (cachedInfo != null && cachedInfo.isRcsContact == 0)) {
                    try {
                        capabilities = mCapabilitiesApi
                                .getContactCapabilities(createContactIdFromTrustedData(number));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    if (cachedInfo.isRcsContact == 1) {
                        needNotify = true;
                    }
                    continue;
                }
                if (capabilities != null) {
                info.isRcsContact = capabilities.isSupportedRcseContact() ? 1
                        : 0;
                info.isImSupported = capabilities.isImSessionSupported();
                info.isFtSupported = capabilities.isFileTransferSupported();
                info.isImageShareSupported = capabilities
                        .isImageSharingSupported();
                info.isVideoShareSupported = capabilities
                        .isVideoSharingSupported();
                info.isCsCallSupported = capabilities.isCsVideoSupported();
                    Logger.d(TAG, "queryPresence() contactId: " + contactId
                            + " info.isRcsContact: " + info.isRcsContact);
                synchronized (mContactsCache) {
                    mContactsCache.put(number, info);
                }
                if (info.isRcsContact == 1) {
                    needNotify = true;
                }
            }
            }
            synchronized (mPresenceListeners) {
                if (needNotify) {
                    OnPresenceChangedListener listener = mPresenceListeners
                            .get(contactId);
                    if (listener != null) {
                        listener.onPresenceChanged(contactId, 1);
                    }
                }
                mPresenceListeners.remove(contactId);
            }
            Logger.d(TAG, "queryPresence() contactId: " + contactId
                    + " needNotify: " + needNotify);
        }
    }

    /**
     * Obtain the phone numbers from a specific contact id.
     *
     * @param contactId            The contact id
     * @return The phone numbers of the contact id
     */
    public List<String> getNumbersByContactId(long contactId) {
        List<String> list = new ArrayList<String>();
        String[] projection = { Phone.NUMBER };
        String selection = Phone.CONTACT_ID + "=? ";
        String[] selectionArgs = { Long.toString(contactId) };
        Cursor cur =null;
        try {
            cur = mContext.getContentResolver().query(Phone.CONTENT_URI,
                projection, selection, selectionArgs, null);
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            Toast.makeText(mContext, "Permission Denied. You can change it in Settings->Apps.", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }

        try {
            if (cur != null) {
                while (cur.moveToNext()) {
                    String number = cur.getString(0);
                    if (!TextUtils.isEmpty(number)) {
                        list.add(number.replace(" ", ""));
                    } else {
                        Logger.w(TAG,
                                "getNumbersByContactId() invalid number: "
                                        + number);
                    }
                }
            }
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
        mCache.put(contactId, list);
        Logger.d(TAG, "getNumbersByContactId exit, list: " + list);
        return list;
    }

    /**
     * Return whether Im is supported.
     *
     * @param number
     *            The number whose capability is to be queried.
     * @return True if Im is supported, else false.
     */
    public boolean isImSupported(final String number) {
        Logger.d(TAG, "isImSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isImSupported exit");
            return info.isImSupported;
        } else {
            Logger.d(TAG, "isImSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Checks if is im supported in capability.
     *
     * @param number the number
     * @return true, if is im supported in capability
     */
    public boolean isImSupportedInCapability(final String number) {
        Logger.d(TAG, "isImSupportedInCapability entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }

        if (mCapabilitiesApi != null) {
            Capabilities capabilities;
            try {
                capabilities = mCapabilitiesApi.getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(number));
                return capabilities.isImSessionSupported();
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }

        } else {
            Logger.d(TAG, "isImSupportedInCapability mCapabilitiesApi is null");
            return false;
        }
    }

    /**
     * Checks if is ft supported in capability.
     *
     * @param number the number
     * @return true, if is ft supported in capability
     */
    public boolean isFtSupportedInCapability(final String number) {
        Logger.d(TAG, "isFtSupportedInCapability entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }

        if (mCapabilitiesApi != null) {
            Capabilities capabilities = null;
            try {
                capabilities = mCapabilitiesApi.getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(number));
                return capabilities.isFileTransferSupported();
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }

        } else {
            Logger.d(TAG, "isImSupportedInCapability mCapabilitiesApi is null");
            return false;
        }
    }

    /**
     * Return whether file transfer is supported.
     *
     * @param number
     *            The number whose capability is to be queried.
     * @return True if file transfer is supported, else false.
     */
    public boolean isFtSupported(final String number) {
        Logger.d(TAG, "isFtSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isFtSupported exit");
            return info.isFtSupported;
        } else {
            Logger.d(TAG, "isFtSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether image sharing is supported.
     *
     * @param number
     *            The number whose capability is to be queried.
     * @return True if image sharing is supported, else false.
     */
    public boolean isImageShareSupported(final String number) {
        Logger.d(TAG, "isImageShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "sharing number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null && info.isImageShareSupported) {
            Logger.d(TAG, "sharing isImageShareSupported exit");
            return info.isImageShareSupported;
        } else {
            Logger.d(TAG, "sharing isImageShareSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether video sharing is supported.
     *
     * @param number
     *            The number whose capability is to be queried.
     * @return True if video sharing is supported, else false.
     */
    public boolean isVideoShareSupported(final String number) {
        Logger.d(TAG, "isVideoShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null && info.isVideoShareSupported) {
            Logger.d(TAG, "isVideoShareSupported exit");
            return info.isVideoShareSupported;
        } else {
            Logger.d(TAG,
                    "isVideoShareSupported info is null, or the capability is false.");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether CS call is supported.
     *
     * @param number
     *            The number whose capability is to be queried.
     * @return True if CS call is supported, else false.
     */
    public boolean isCsCallShareSupported(final String number) {
        Logger.d(TAG, "isCsCallShareSupported entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "isCsCallShareSupported exit");
            return info.isCsCallSupported;
        } else {
            Logger.d(TAG, "isCsCallShareSupported info is null");
            queryContactPresence(number);
            return false;
        }
    }

    /**
     * Return whether Im is supported.
     *
     * @param contactId
     *            The contactId whose capability is to be queried.
     * @return True if Im is supported, else false.
     */
    public boolean isImSupported(final long contactId) {
        Logger.d(TAG, "isImSupported entry, with contact id: " + contactId);
        final List<String> numbers = mCache.get(contactId);
        if (numbers != null) {
            for (String number : numbers) {
                boolean isImSupported = isImSupported(number);
                if (isImSupported) {
                    Logger.d(TAG, "isImSupported exit with true");
                    return true;
                }
            }
            Logger.d(TAG, "isImSupported exit with false");
            return false;
        } else {
            Logger.d(TAG, "isImSupported numbers is null, exit with false");
            return false;
        }
    }

    /**
     * Return whether file transfer is supported.
     *
     * @param contactId
     *            The contactId whose capability is to be queried.
     * @return True if file transfer is supported, else false.
     */
    public boolean isFtSupported(final long contactId) {
        Logger.d(TAG, "isFtSupported entry, with contact id: " + contactId);
        final List<String> numbers = mCache.get(contactId);
        if (numbers != null) {
            for (String number : numbers) {
                boolean isFtSupported = isFtSupported(number);
                if (isFtSupported) {
                    Logger.d(TAG, "isFtSupported exit with true");
                    return true;
                }
            }
            Logger.d(TAG, "isFtSupported exit with false");
            return false;
        } else {
            Logger.d(TAG, "isFtSupported numbers is null, exit with false");
            return false;
        }
    }

    /**
     * Check whether a number is a rcse account.
     *
     * @param number            The number to query
     * @return True if number is a rcse account, otherwise return false.
     */
    public boolean isRcseContact(String number) {
        Logger.d(TAG, "sharing isRcseContact entry, with number: " + number);
        if (number == null) {
            Logger.w(TAG, "sharing number is null");
            return false;
        }
        ContactInformation info = mContactsCache.get(number);
        if (info != null) {
            Logger.d(TAG, "sharing isCsCallShareSupported exit");
            return info.isRcsContact == 1;
        } else {
            Logger.d(TAG, "sharing isCsCallShareSupported info is null");
            queryContactPresence(number);
            return false;
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
        if (null != sInstance) {
            return true;
        } else {
            if (null != context) {
                PluginApiManager apiManager = new PluginApiManager(context);
                sInstance = apiManager;
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
     * Get the instance of PluginApiManager.
     *
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static PluginApiManager getInstance() {
        if (null == sInstance) {
            throw new RuntimeException(
                    "Please call initialize() before calling this method");
        }
        return sInstance;
    }

    /**
     * Instantiates a new plugin api manager.
     *
     * @param context the context
     */
    private PluginApiManager(Context context) {
        Logger.v(TAG, "PluginApiManager(), context = " + context);
        mContext = context;
        mMyCapabilitiesListener = new MyCapabilitiesListener();
        MyRcsServiceListener myRcsServiceListener = new MyRcsServiceListener();
        mCapabilitiesApi = new CapabilityService(context, myRcsServiceListener);

        mCapabilitiesApi.connect();
        mManagedRegistrationApi = new ManagedRegistrationApi(context);
        mManagedRegistrationApi.connect();

        mContactAPI = new ContactService(context, new ContactServiceListener());
        mContactAPI.connect();

        Intent intent = new Intent();
        ComponentName cmp = new ComponentName("com.mediatek.rcs",
                            "com.mediatek.rcse.service.ApiService");
        intent.setComponent(cmp);
        boolean result = mContext.bindService(intent, mApiConnection,
                Context.BIND_AUTO_CREATE);
            Logger.v(TAG, "the result is " + result);
        initData();
    }


    public class MyRcsServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            try {
                Logger.d(TAG, "onServiceConnected");
                PluginApiManager.this.mCapabilitiesApi
                        .addCapabilitiesListener(mMyCapabilitiesListener);
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            try {
                Logger.d(TAG, "onServiceDisConnected");
                if (PluginApiManager.this.mCapabilitiesApi != null) {
                    PluginApiManager.this.mCapabilitiesApi
                            .removeCapabilitiesListener(mMyCapabilitiesListener);
                    PluginApiManager.this.mCapabilitiesApi = null;
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }

        }

    }

    public class ContactServiceListener implements RcsServiceListener {

        /**
         * On service connected.
         */
        @Override
        public void onServiceConnected() {
            try {
                Logger.d(TAG, "onServiceConnected");
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        /**
         * On service disconnected.
         *
         * @param error the error
         */
        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            try {
                Logger.d(TAG, "onServiceDisConnected");
                if (PluginApiManager.this.mContactAPI != null) {
                    PluginApiManager.this.mContactAPI = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }

    /**
     * The listener interface for receiving myCapabilities events.
     * The class that is interested in processing a myCapabilities
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addMyCapabilitiesListener method. When
     * the myCapabilities event occurs, that object's appropriate
     * method is invoked.
     *
     * @see MyCapabilitiesEvent
     */
    public class MyCapabilitiesListener extends CapabilitiesListener {

        /**
         * On capabilities received.
         *
         * @param contact the contact
         * @param capabilities the capabilities
         */
        @Override
        public void onCapabilitiesReceived(final String contact,
                Capabilities capabilities) {

            Logger.w(TAG, "options onCapabilityChanged(), contact = " + contact
                    + ", capabilities = " + capabilities + ", mContactsCache= "
                    + mContactsCache);
            if (null != contact && capabilities != null) {
                Logger.v(TAG, "Remove from cache");
                ContactInformation info = mContactsCache.remove(contact);
                Logger.v(TAG, "after remove from cache");
                if (info == null) {
                    Logger.v(TAG, "cache does not exist, so create a object.");
                    info = new ContactInformation();
                }
                info.isRcsContact = capabilities.isSupportedRcseContact() ? 1
                        : 0;
                Logger.v(TAG, "Options  is RCS Contact:" + info.isRcsContact);
                if (capabilities.isSupportedRcseContact()) {
                    Logger.w(TAG, "Options It is RCS Contact");
                }

                //

                Logger.w(TAG, "IR94 capabilities for contact:" + contact +
                        " voice : " + capabilities.isIR94_VoiceCallSupported()  +
                        " video " + capabilities.isIR94_VideoCallSupported()  +
                        " duplex " + capabilities.isIR94_DuplexModeSupported()
                );

                info.isImSupported = capabilities.isImSessionSupported();
                info.isFtSupported = capabilities.isFileTransferSupported();
                info.isImageShareSupported = capabilities
                        .isImageSharingSupported();
                info.isVideoShareSupported = capabilities
                        .isVideoSharingSupported();
                if (info.isImageShareSupported || info.isVideoShareSupported) {
                    Logger.w(TAG, "Options Image/Video share supported");
                }
                info.isCsCallSupported = capabilities.isCsVideoSupported();
                mContactsCache.put(contact, info);
                Logger.w(TAG, "put capability into cache");
                for (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
                    if (listener != null) {
                        Logger.w(TAG, "Notify the listener");
                        listener.onCapabilitiesChanged(contact, info);
                    }
                }
            } else {
                Logger.d(TAG,
                        "onCapabilityChanged()-invalid contact or capabilities");
            }
        }

        /**
         * Callback called when error is received for a given contact
         *
         * @param contact Contact
         * @param type subscribe type(Capability Polling, Availabilty Fetch)
         * @param status error code
         * @param reason reason of error
         */
        @Override
        public void onErrorReceived(String contact, int type, int status, String reason){}
    }

    /**
     * This class defined to manage the registration APIs and registration
     * status.
     */
    private class ManagedRegistrationApi extends RegistrationApi {
        public static final String TAG = "PluginApiManager/ManagedRegistrationApi";

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
            Logger.v(TAG, "handleConnected() entry");
            for (RegistrationListener listener : mRegistrationListeners) {
                listener.onApiConnectedStatusChanged(true);
            }
            mRegistrationApi = this;
            mRegistrationApi
                    .addRegistrationStatusListener(new IRegistrationStatusListener() {

                        @Override
                        public void onStatusChanged(boolean status) {
                            Logger.d(TAG, "onStatusChanged() entry, status is "
                                    + status);
                            for (RegistrationListener listener : mRegistrationListeners) {
                                listener.onStatusChanged(status);
                            }
                            mIsRegistered = status;
                        }
                    });
            mIsRegistered = mRegistrationApi.isRegistered();
            Logger.d(TAG, "handleConnected() mIsRegistered is: "
                    + mIsRegistered);
        }

        /**
         * Handle disconnected.
         */
        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            Logger.d(TAG, "Notifiy plugin");
            for (RegistrationListener listener : mRegistrationListeners) {
                listener.onApiConnectedStatusChanged(false);
            }
            if (mRegistrationApi == this) {
                Logger.i(TAG,
                        "handleDisconnected() mRegistrationApi disconnected");
                mRegistrationApi = null;
            } else {
                Logger.e(TAG,
                        "handleDisconnected() another mRegistrationApi disconnected?");
            }
        }
    }

    /**
     * Get the registration status.
     *
     * @return Registration status
     */
    public boolean getRegistrationStatus() {
        if (mRegistrationApi == null) {
            Logger.w(TAG, "getRegisteredStatus()-mRegistrationApi is null");
            return false;
        }
        return mIsRegistered;
    }

    /**
     * Set the registration status.
     *
     * @param status            registration status
     */
    public void setRegistrationStatus(boolean status) {
        Logger.w(TAG, "setRegistrationStatus()-status is " + status);
        mIsRegistered = status;
    }

    /**
     * This class defined to manage the capabilities APIs and capabilities.
     */
    /*
     * private class ManagedCapabilityApi extends CapabilityApi { private static
     * final String TAG = "PluginApiManager/ManagedCapabilityApi";
     *
     * public ManagedCapabilityApi(Context context) { super(context);
     * Logger.w(TAG, "PluginApiManager ManagedCapabilityApi() entry"); }
     *
     * @Override public void handleConnected() { Logger.v(TAG,
     * "handleConnected() entry"); mCapabilitiesApi = this;
     * mCapabilitiesApi.registerCapabilityListener(PluginApiManager.this); for
     * (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
     * listener.onApiConnectedStatusChanged(true); } }
     *
     * @Override public void handleDisconnected() { Logger.v(TAG,
     * "handleDisconnected() entry"); for (CapabilitiesChangeListener listener :
     * mCapabilitiesChangeListenerList) {
     * listener.onApiConnectedStatusChanged(false); } if (mCapabilitiesApi ==
     * this) { Logger.i(TAG,
     * "handleDisconnected() mCapabilitiesApi disconnected");
     * mCapabilitiesApi.unregisterCapabilityListener(PluginApiManager.this);
     * mCapabilitiesApi = null; } else { Logger.e(TAG,
     * "handleDisconnected() another mCapabilitiesApi disconnected?"); } } }
     */

    /*
     * @Override public void onCapabilityChanged(String contact, Capabilities
     * capabilities) { Logger.w(TAG, "options onCapabilityChanged(), contact = "
     * + contact + ", capabilities = " + capabilities + ", mContactsCache= " +
     * mContactsCache); if (null != contact && capabilities != null) {
     * Logger.v(TAG, "Remove from cache"); ContactInformation info =
     * mContactsCache.remove(contact); Logger.v(TAG, "after remove from cache");
     * if (info == null) { Logger.v(TAG,
     * "cache does not exist, so create a object."); info = new
     * ContactInformation(); } info.isRcsContact =
     * capabilities.isSupportedRcseContact() ? 1 : 0; Logger.v(TAG,
     * "Options  is RCS Contact:" + info.isRcsContact);
     * if(capabilities.isSupportedRcseContact()){ Logger.w(TAG,
     * "Options It is RCS Contact"); } info.isImSupported =
     * capabilities.isImSessionSupported(); info.isFtSupported =
     * capabilities.isFileTransferSupported(); info.isImageShareSupported =
     * capabilities.isImageSharingSupported(); info.isVideoShareSupported =
     * capabilities.isVideoSharingSupported(); if(info.isImageShareSupported ||
     * info.isVideoShareSupported){ Logger.w(TAG,
     * "Options Image/Video share supported"); } info.isCsCallSupported =
     * capabilities.isCsVideoSupported(); mContactsCache.put(contact, info);
     * Logger.w(TAG, "put capability into cache"); for
     * (CapabilitiesChangeListener listener : mCapabilitiesChangeListenerList) {
     * if (listener != null) { Logger.w(TAG, "Notify the listener");
     * listener.onCapabilitiesChanged(contact, info); } } } else { Logger.d(TAG,
     * "onCapabilityChanged()-invalid contact or capabilities"); } }
     */

    /**
     * This constructor is just used for test case.
     */
    public PluginApiManager() {

    }

    /**
     * Clear all the information in the mContactsCache.
     */
    public void cleanContactCache() {
        Logger.d(TAG, "cleanContactCache() entry");
        mContactsCache.evictAll();
    }

    /**
     * Sets the managed api status.
     *
     * @param needReconnect            Indicate whether need to reconnect API
     */
    public void setManagedApiStatus(boolean needReconnect) {
        mNeedReconnectManagedAPi = true;
    }

    /**
     * Gets the managed api status.
     *
     * @return True if need to reconnect API, otherwise return false
     */
    public boolean getManagedApiStatus() {
        return mNeedReconnectManagedAPi;
    }

    /**
     * Reconnect ManagedCapabilityApi.
     */
    public void reConnectManagedApi() {
        Logger.d(TAG, "reConnectManagedApi():");
        mNeedReconnectManagedAPi = false;
        mCapabilitiesApi.connect();
    }

}
