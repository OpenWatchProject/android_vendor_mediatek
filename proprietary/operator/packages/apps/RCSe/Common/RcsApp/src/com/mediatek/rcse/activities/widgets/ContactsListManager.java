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

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.widget.Toast;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactsListContentObserver;

//import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.settings.AppSettings;
import com.mediatek.rcse.settings.RcsSettings;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The Class ContactsListManager.
 */
public final class ContactsListManager {
    public static final boolean IS_SUPPORT = false;

    private static final String TAG = "ContactsListManager";

    private static final int LIST_MAX_SIZE = 100;

    private Context mContext;
    /**
     * MIME type for a RCS capable contact.
     */
    private static final String MIMETYPE_PHONE_CONTACT =
            "vnd.android.cursor.item/phone_v2";

    private static final String MIMETYPE_RCS_CONTACT =
            "vnd.android.cursor.item/com.orangelabs.rcs.rcs-status";
    private static final int RCS_CONTACT = 2;
    private static final int RCS_CAPABLE_CONTACT = 0;
    private static final String[] PROJECTION = { Phone.CONTACT_ID, Phone._ID,
            Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DISPLAY_NAME,
            Contacts.SORT_KEY_PRIMARY };

    private static final String SELECTION_RCS = Data.MIMETYPE + "=? AND " + "("
            + Data.DATA2 + "=? OR " + Data.DATA2 + "=? )";
    private static final String[] SELECTION_ARGS_RCS = { MIMETYPE_RCS_CONTACT,
            Long.toString(RCS_CONTACT), Long.toString(RCS_CAPABLE_CONTACT) };
    private static final String SELECTION_ALL = Data.MIMETYPE + "=?";
    private static final String[] SELECTION_ARGS_ALL = { MIMETYPE_PHONE_CONTACT };
    public static final String SORT_ORDER = Contacts.SORT_KEY_PRIMARY;

    public static final int UPDATE_THROTTLE = 1000;

    private static volatile ContactsListManager sInstance = null;
    private static final List<WeakReference<OnDisplayNameChangedListener>> LISTENER_LIST =
            new CopyOnWriteArrayList<WeakReference<OnDisplayNameChangedListener>>();

    // Current blocked contacts list
    private static final List<String> BLOCKED_CONTRACTS = new ArrayList<String>();

    // store contacts list from database
    public final List<RcsContact> CONTACTS_LIST = new CopyOnWriteArrayList<RcsContact>();

    public final TreeSet<String> STRANGER_LIST = new TreeSet<String>();
    public static final long LOAD_THROTTLE = 3000;

    /**
     * Whether set this participant as a stranger contact.
     *
     * @param number            The phone number
     * @param isStranger            add or remove the number to the stranger list
     */
    public void setStrangerList(String number, boolean isStranger) {
        Logger.d(TAG, "setStrangerList() entry number is " + number
                + "isStranger is " + isStranger);
        synchronized (STRANGER_LIST) {
            if (isStranger) {
                if (STRANGER_LIST.size() < LIST_MAX_SIZE) {
                    STRANGER_LIST.add(number);
                } else {
                    String first = STRANGER_LIST.first();
                    STRANGER_LIST.remove(first);
                    STRANGER_LIST.add(number);
                }
            } else {
                STRANGER_LIST.remove(number);
            }
        }
        Logger.d(TAG, "setStrangerList() STRANGER_LIST is " + STRANGER_LIST.size());
    }

    /**
     * Judge this participant is a stranger contact.
     *
     * @param number the number
     * @return true is in the stranger list
     */
    public boolean isStranger(String number) {
        Logger.d(TAG, "isStranger the number is " + number);
        if (number == null) {
            Logger.d(TAG, "isStranger the number is null");
            return false;
        }
        return STRANGER_LIST.contains(number);
    }

    /**
     * Get the display name by specify phone number.
     *
     * @param matchString            The phone number used to query the display name
     * @return The display name
     */
    public String getDisplayNameByPhoneNumber(String matchString) {

        if (TextUtils.isEmpty(matchString)) {
            return "";
        }

        Logger.d(TAG,
                "getDisplayNameByPhoneNumber() entry,with matchString is "
                        + matchString + " and CONTACTS_LIST: " + CONTACTS_LIST.size());
        for (RcsContact rcsContact : CONTACTS_LIST) {
            if (matchString.equals(rcsContact.mNumber)) {
                String displayName = rcsContact.mDisplayName;
                if (displayName != null) {
                    return displayName;
                } else {
                    return matchString;
                }
            }
        }
        return matchString;
    }

    /**
     * Get the phone number by specify phone id.
     *
     * @param matchId            The phone id used to query the phone number
     * @return The phone number
     */
    public String getPhoneNumberByPhoneId(long matchId) {
        Logger.d(TAG, "getPhoneNumbereByPhoneId() entry matchId is " + matchId
                + " and CONTACTS_LIST: " + CONTACTS_LIST.size());
        String phoneNumber = null;
        for (RcsContact rcsContact : CONTACTS_LIST) {
            if (matchId == rcsContact.mPhoneId) {
                phoneNumber = rcsContact.mNumber;
                Logger.d(TAG, "getPhoneNumbereByPhoneId() phoneNumber is "
                        + phoneNumber);
                if (phoneNumber != null) {
                    break;
                }
            }
        }
        return phoneNumber;
    }

    /**
     * Get contact id by a specific phone number.
     *
     * @param number            The phone number of the contact id
     * @return The contact id
     */
    public short getContactIdByNumber(String number) {
        Logger.d(TAG, "getContactIdByNumber() entry with the phone number "
                + number + " and CONTACTS_LIST: " + CONTACTS_LIST.size());
        if (number == null) {
            return Short.MIN_VALUE;
        }
        for (RcsContact rcsContact : CONTACTS_LIST) {
            if (number.equals(rcsContact.mNumber)) {
                short contactId = rcsContact.mContactId;
                Logger.d(TAG, "getContactIdByNumber() contactId is "
                        + contactId);
                if (contactId > 0) {
                    return contactId;
                } else {
                    return Short.MIN_VALUE;
                }
            }
        }
        return Short.MIN_VALUE;
    }

    /**
     * Get the phone id to be sent to people.
     *
     * @param participantList            the participant to be removed from the phone id
     * @return long[] stored for phone id.
     */
    public long[] getPhoneIdTobeShow(List<Participant> participantList) {
        Logger.d(TAG, "getPhoneIdTobeShow() entry the participantlist is "
                + participantList + " CONTACTS_LIST: is" + CONTACTS_LIST.size());
        List<String> blockList = ApiManager.getInstance().getContactsApi()
                .getImBlockedContactsFromLocal();
        Logger.d(TAG, "getPhoneIdTobeShow() blockList is " + blockList.size());
        BLOCKED_CONTRACTS.clear();
        BLOCKED_CONTRACTS.addAll(blockList);
        for (String blockNumber : blockList) {
            RcsContact contact = new RcsContact(blockNumber, blockNumber);
            CONTACTS_LIST.remove(contact);
        }
        Logger.d(TAG, "getPhoneIdTobeShow() CONTACTS_LIST is " + CONTACTS_LIST.size());
        List<RcsContact> currentList = new ArrayList<RcsContact>(CONTACTS_LIST);
        ArrayList<RcsContact> existingContacts = new ArrayList<RcsContact>();
        if (participantList != null) {
            for (Participant participant : participantList) {
                RcsContact contact = new RcsContact(
                        participant.getDisplayName(), participant.getContact());
                existingContacts.add(contact);
            }
        } else {
            Logger.d(TAG,
                    "getPhoneIdTobeShow() participantList is null. from chat list");
        }
        Logger.d(TAG, "getPhoneIdTobeShow() existingContacts is "
                + existingContacts);
        currentList.removeAll(existingContacts);
        int size = currentList.size();
        long[] phoneId = new long[size];
        for (int i = 0; i < size; i++) {
            RcsContact rcsContact = currentList.get(i);
            phoneId[i] = rcsContact.mPhoneId;
            Logger.d(TAG, "getPhoneIdTobeShow() the phoneId is "
                    + rcsContact.mPhoneId);
        }
        return phoneId;
    }

    /**
     * parse the participant list from the intent.
     *
     * @param data
     *            the intent should be to parsed.
     * @return ArrayList<Participant> the participant list parsed out.
     */
    public ArrayList<Participant> parseParticipantsFromIntent(Intent data) {
        Logger.d(TAG, "parseParticipantsFromIntent() entry");
        ArrayList<Participant> participants = null;
        if (IS_SUPPORT) {
            participants = new ArrayList<Participant>();
            final long[] phoneIdList = data
                    .getLongArrayExtra(ChatMainActivity.PICK_RESULT);
            if (phoneIdList == null) {
                Logger.d(TAG,
                        "onActivityResult() phoneIdList is null or size is 0");
                return null;
            }
            int size = phoneIdList.length;
            for (int i = 0; i < size; i++) {
                long phoneId = phoneIdList[i];
                String phoneNum = getPhoneNumberByPhoneId(phoneId);
                if (phoneNum == null) {
                    Logger.e(TAG, "onActivityResult() the phoneNum is null ");
                } else {
                    String name = getDisplayNameByPhoneNumber(phoneNum);
                    Participant participant = new Participant(phoneNum, name);
                    participants.add(participant);
                }
            }
        } else {
            participants = data
                    .getParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST);
        }
        return participants;
    }

    /**
     * Judge this participant is a local contact.
     *
     * @param contact the contact
     * @return true is in the rcse contact list
     */
    public boolean isLocalContact(String contact) {
        if (contact != null) {
            for (RcsContact rcsContact : CONTACTS_LIST) {
                if (contact.equals(rcsContact.mNumber)) {
                    return true;
                }
            }
            return false;
        } else {
            return false;
        }
    }

    /**
     * Initialize.
     *
     * @param context the context
     */
    public static void initialize(Context context) {
        if (null != sInstance) {
            Logger.e(TAG, "initialize() entry, but sInstance is not null");
            return;
        } else {
            Logger.d(TAG, "initialize() entry, initiate an instance");
            sInstance = new ContactsListManager(context);
        }
    }

    /**
     * Gets the single instance of ContactsListManager.
     *
     * @return single instance of ContactsListManager
     */
    public static ContactsListManager getInstance() {
        return sInstance;
    }

    /**
     * Adds the listener.
     *
     * @param listener the listener
     */
    public void addListener(OnDisplayNameChangedListener listener) {
        Logger.d(TAG, "addListener() listener is " + listener);
        LISTENER_LIST.add(new WeakReference<OnDisplayNameChangedListener>(
                listener));
    }

    /**
     * Removes the listener.
     *
     * @param listener the listener
     */
    public void removeListener(OnDisplayNameChangedListener listener) {
        Logger.d(TAG, "removeListener() listener is " + listener);

        if (null != listener) {
            for (WeakReference<OnDisplayNameChangedListener> reference : LISTENER_LIST) {
                if (listener.equals(reference.get())) {
                    Logger.d(TAG, "removeListener() listener " + listener
                            + " found, then remove it");
                    LISTENER_LIST.remove(reference);
                    return;
                }
            }
            Logger.e(TAG, "removeListener() listener " + listener
                    + " not found");
        }
    }

    private CursorLoader mLoader = null;

    /**
     * Instantiates a new contacts list manager.
     *
     * @param context the context
     */
    private ContactsListManager(Context context) {
        /*
         * mLoader = new CursorLoader(context, Data.CONTENT_URI, PROJECTION,
         * SELECTION, SELECTION_ARGS, SORT_ORDER);
         * mLoader.setUpdateThrottle(LOAD_THROTTLE); mLoader.registerListener(0,
         * this); mLoader.startLoading();
         */
        mContext = context;
        try {
        ContactsListContentObserver contactsListContentObserver = new ContactsListContentObserver(
                null, context);
        context.getContentResolver().registerContentObserver(Data.CONTENT_URI,
                true, contactsListContentObserver);
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            //Toast.makeText(mContext, "Permission Denied. You can change it in Settings->Apps.", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }


    }

    /**
     * Gets the contact list.
     *
     * @param contacts the contacts
     * @return the contact list
     */
    private void getContactList(List<RcsContact> contacts) {
        ArrayList<RcsContact> strangList = new ArrayList<RcsContact>(
                CONTACTS_LIST);
        strangList.removeAll(contacts);
        for (RcsContact contact : strangList) {
            synchronized (STRANGER_LIST) {
                STRANGER_LIST.add(contact.mNumber);
            }
        }
        CONTACTS_LIST.clear();
        String myNumber = RcsSettings.getInstance().getUserProfileImsUserName();
        String myDisplayName = RcsSettings.getInstance()
                .getUserProfileImsDisplayName();
        RcsContact myRcsContact = new RcsContact(myDisplayName, myNumber);
        if (contacts != null && contacts.size() != 0) {
            for (RcsContact contact : contacts) {
                if(contact.mNumber != null){
                    //remove separator
                    contact.mNumber = contact.mNumber.trim();
                    contact.mNumber = PhoneNumberUtils.stripSeparators(contact.mNumber);
                    contact.mNumber = contact.mNumber.replaceAll("[\\s\\-()]", "");
                }
                if (!CONTACTS_LIST.contains(contact) && isRcsValidNumber(contact.mNumber)) {
                    CONTACTS_LIST.add(contact);
                }
            }
        }
        CONTACTS_LIST.remove(myRcsContact);

    }

    /**
     * Check if number provided is a valid number for RCS <br>
     * It is not valid if : <li>well formatted (not digits only or '+') <li>minimum length
     *
     * @param number Phone number
     * @return Returns true if it is a RCS valid number
     */
    public boolean isRcsValidNumber(String number) {
        return android.telephony.PhoneNumberUtils.isGlobalPhoneNumber(number)
                && (number.length() > 3);
    }

    /**
     * The Class LoadFinishedTask.
     */
    private class LoadFinishedTask extends AsyncTask<Void, Void, Void> {
        private List<RcsContact> mContacts;

        /**
         * Instantiates a new load finished task.
         *
         * @param contacts the contacts
         */
        public LoadFinishedTask(List<RcsContact> contacts) {
            mContacts = contacts;
        }

        @Override
        protected Void doInBackground(Void... params) {
            Logger.d(TAG, "LoadFinishedTask() doInBackground entry");
            ApiManager instance = ApiManager.getInstance();
            if (instance == null) {
                Logger.i(TAG, "ApiManager instance is null");
            }
            try {
                if (null != instance
                        && null != instance.getContactsApi()
                        && null != instance.getContactsApi().getImBlockedContactsFromLocal()) {
                    try {
                        List<String> blockList = ApiManager.getInstance()
                                .getContactsApi().getImBlockedContactsFromLocal();
                        BLOCKED_CONTRACTS.clear();
                        BLOCKED_CONTRACTS.addAll(blockList);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG, "onLoadComplete, block list is null!");
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            getContactList(mContacts);
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            TreeSet<String> remove = new TreeSet<String>();
            synchronized (STRANGER_LIST) {
                for (String contact : STRANGER_LIST) {
                    RcsContact rcsContact = new RcsContact(contact, contact);
                    if (CONTACTS_LIST.contains(rcsContact)) {
                        remove.add(contact);
                    } else {
                        Logger.d(TAG, "onLoadComplete() the contact " + contact
                                + "not in CONTACTS_LIST ");
                    }
                }
                STRANGER_LIST.removeAll(remove);
            }

            List<WeakReference<OnDisplayNameChangedListener>> emptyReferenceList = null;
            for (WeakReference<OnDisplayNameChangedListener> reference : LISTENER_LIST) {
                OnDisplayNameChangedListener listener = reference.get();
                if (null != listener) {
                    listener.onDisplayNameChanged();
                } else {
                    if (null == emptyReferenceList) {
                        emptyReferenceList =
                                new ArrayList<WeakReference<OnDisplayNameChangedListener>>();
                    }
                    emptyReferenceList.add(reference);
                }
            }
            if (null == emptyReferenceList) {
                Logger.d(TAG, "onLoadComplete() emptyReferenceList is null, "
                        + "doesn't need to clear any listeners");
            } else {
                LISTENER_LIST.removeAll(emptyReferenceList);
                Logger.w(TAG,
                        "onLoadComplete() emptyReferenceList is not null, clear "
                                + emptyReferenceList.size() + " listener(s)");
            }
        }
    }

    /**
     * On contacts db change.
     */
    public void onContactsDbChange() {
        Cursor cursor = null;
        try{
            if (RcsSettings.getInstance().isSupportOP07() ||
                    RcsSettings.getInstance().isSupportOP08()) {
                //in case of OP08 , need to show all contacts
                Logger.d(TAG, "onLoadComplete show all contacts");
                cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                    PROJECTION, SELECTION_ALL, SELECTION_ARGS_ALL, SORT_ORDER);
            } else {
                Logger.d(TAG, "onLoadComplete show rcs contacts");
                cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                        PROJECTION, SELECTION_RCS, SELECTION_ARGS_RCS, SORT_ORDER);
            }
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            //Toast.makeText(mContext, "Permission Denied. You can change it in Settings->Apps.", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }
        List<RcsContact> contacts = new ArrayList<RcsContact>();
        int i = 0;
        try{
            if (cursor != null && cursor.getCount() != 0) {
                Logger.d(
                        TAG,
                        "onLoadComplete() cursor.getCount() is "
                                + cursor.getCount());
                cursor.moveToFirst();
                do {
                    String displayName = cursor.getString(cursor
                            .getColumnIndex(Data.DISPLAY_NAME));
                    String number = cursor.getString(cursor
                            .getColumnIndex(Data.DATA1));
                    String sortKeyString = cursor.getString(cursor
                            .getColumnIndex(Contacts.SORT_KEY_PRIMARY));
                    Long phoneId = cursor.getLong(cursor.getColumnIndex(Phone._ID));
                    short contactId = cursor.getShort(cursor
                            .getColumnIndex(Phone.CONTACT_ID));
                    RcsContact contact = new RcsContact(displayName, number,
                            sortKeyString, phoneId, contactId);
                    contacts.add(contact);
                    if (i < 20) {
                        Logger.d(TAG, "onLoadComplete, contacts[" + i + "] is " + contact);
                    }
                    i++;
                } while (cursor.moveToNext());
            }
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
            new LoadFinishedTask(contacts).execute();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * The listener interface for receiving onDisplayNameChanged events.
     * The class that is interested in processing a onDisplayNameChanged
     * event implements this interface, and the object created
     * with that class is registered with a component using the
     * component's addOnDisplayNameChangedListener method. When
     * the onDisplayNameChanged event occurs, that object's appropriate
     * method is invoked.
     *
     * @see OnDisplayNameChangedEvent
     */
    public interface OnDisplayNameChangedListener {
        /**
         * On display name changed.
         */
        void onDisplayNameChanged();
    }

    /**
     * Removes the block contacts.
     */
    public void removeBlockContacts() {
        Logger.d(TAG, "removeBlockContacts entry!");
        if (null != ApiManager.getInstance().getContactsApi()) {
            List<String> blockList = ApiManager.getInstance().getContactsApi()
                    .getImBlockedContactsFromLocal();
            BLOCKED_CONTRACTS.clear();
            BLOCKED_CONTRACTS.addAll(blockList);
            for (String blockNumber : blockList) {
                RcsContact contact = new RcsContact(blockNumber, blockNumber);
                CONTACTS_LIST.remove(contact);
            }
        } else {
            Logger.d(TAG, "removeBlockContacts, block list is null!");
        }
    }
}
