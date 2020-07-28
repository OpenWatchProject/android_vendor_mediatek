/*
 * Copyright (C) 2016 The Android Open Source Project
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
 */
package com.android.emergency.preferences;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.TypedArray;
import android.net.Uri;
import androidx.annotation.NonNull;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.Toast;

import com.android.emergency.EmergencyContactManager;
import com.android.emergency.PreferenceKeys;
import com.android.emergency.R;
import com.android.emergency.ReloadablePreferenceInterface;
import com.android.emergency.util.PreferenceUtils;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.logging.MetricsLogger;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * Custom {@link PreferenceCategory} that deals with contacts being deleted from the contacts app.
 *
 * <p>Contacts are stored internally using their ContactsContract.CommonDataKinds.Phone.CONTENT_URI.
 */
public class EmergencyContactsPreference extends PreferenceCategory
        implements ReloadablePreferenceInterface,
        ContactPreference.RemoveContactPreferenceListener {

    private static final String TAG = "EmergencyContactsPreference";

    private static final String CONTACT_SEPARATOR = "|";
    private static final String QUOTE_CONTACT_SEPARATOR = Pattern.quote(CONTACT_SEPARATOR);
    private static final ContactValidator DEFAULT_CONTACT_VALIDATOR = new ContactValidator() {
        @Override
        public boolean isValidEmergencyContact(Context context, Uri phoneUri) {
            return EmergencyContactManager.isValidEmergencyContact(context, phoneUri);
        }
    };

    /// M: SIM contact support
    private static final int NUM_CACHE_SIZE = 10;

    private final ContactValidator mContactValidator;
    private final ContactPreference.ContactFactory mContactFactory;
    /** Stores the emergency contact's ContactsContract.CommonDataKinds.Phone.CONTENT_URI */
    private List<Uri> mEmergencyContacts = new ArrayList<Uri>();
    private boolean mEmergencyContactsSet = false;

    /**
     * Interface for getting a contact for a phone number Uri.
     */
    public interface ContactValidator {
        /**
         * Checks whether a given phone Uri represents a valid emergency contact.
         *
         * @param context The context to use.
         * @param phoneUri The phone uri.
         * @return whether the given phone Uri is a valid emergency contact.
         */
        boolean isValidEmergencyContact(Context context, Uri phoneUri);
    }

    public EmergencyContactsPreference(Context context, AttributeSet attrs) {
        this(context, attrs, DEFAULT_CONTACT_VALIDATOR, ContactPreference.DEFAULT_CONTACT_FACTORY);
    }

    @VisibleForTesting
    EmergencyContactsPreference(Context context, AttributeSet attrs,
            @NonNull ContactValidator contactValidator,
            @NonNull ContactPreference.ContactFactory contactFactory) {
        super(context, attrs);
        mContactValidator = contactValidator;
        mContactFactory = contactFactory;
    }

    @Override
    protected void onSetInitialValue(boolean restorePersistedValue, Object defaultValue) {
        setEmergencyContacts(restorePersistedValue ?
                getPersistedEmergencyContacts() :
                deserializeAndFilter(getKey(),
                        getContext(),
                        (String) defaultValue,
                        mContactValidator));
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return a.getString(index);
    }

    @Override
    public void reloadFromPreference() {
        setEmergencyContacts(getPersistedEmergencyContacts());
    }

    @Override
    public boolean isNotSet() {
        return mEmergencyContacts.isEmpty();
    }

    @Override
    public void onRemoveContactPreference(ContactPreference contactPreference) {
        Uri phoneUriToRemove = contactPreference.getPhoneUri();
        if (mEmergencyContacts.contains(phoneUriToRemove)) {
            List<Uri> updatedContacts = new ArrayList<Uri>(mEmergencyContacts);
            if (updatedContacts.remove(phoneUriToRemove) && callChangeListener(updatedContacts)) {
                MetricsLogger.action(getContext(), MetricsEvent.ACTION_DELETE_EMERGENCY_CONTACT);
                setEmergencyContacts(updatedContacts);
                /// M: SIM contact support
                removeNumFromCache(contactPreference.getContact().getPhoneNumber());
            }
        }
    }

    /**
     * Adds a new emergency contact. The {@code phoneUri} is the
     * ContactsContract.CommonDataKinds.Phone.CONTENT_URI corresponding to the
     * contact's selected phone number.
     */
    public void addNewEmergencyContact(Uri phoneUri) {
        if (mEmergencyContacts.contains(phoneUri)) {
            return;
        }
        if (!mContactValidator.isValidEmergencyContact(getContext(), phoneUri)) {
            Toast.makeText(getContext(), getContext().getString(R.string.fail_add_contact),
                Toast.LENGTH_LONG).show();
            return;
        }
        List<Uri> updatedContacts = new ArrayList<Uri>(mEmergencyContacts);
        if (updatedContacts.add(phoneUri) && callChangeListener(updatedContacts)) {
            MetricsLogger.action(getContext(), MetricsEvent.ACTION_ADD_EMERGENCY_CONTACT);
            setEmergencyContacts(updatedContacts);
        }
    }

    @VisibleForTesting
    public List<Uri> getEmergencyContacts() {
        return mEmergencyContacts;
    }

    public void setEmergencyContacts(List<Uri> emergencyContacts) {
        final boolean changed = !mEmergencyContacts.equals(emergencyContacts);
        if (changed || !mEmergencyContactsSet) {
            mEmergencyContacts = emergencyContacts;
            mEmergencyContactsSet = true;
            persistEmergencyContacts(emergencyContacts);
            if (changed) {
                notifyChanged();
            }
        }

        while (getPreferenceCount() - emergencyContacts.size() > 0) {
            removePreference(getPreference(0));
        }

        // Reload the preferences or add new ones if necessary
        Iterator<Uri> it = emergencyContacts.iterator();
        int i = 0;
        Uri phoneUri = null;
        List<Uri> updatedEmergencyContacts = null;
        while (it.hasNext()) {
            ContactPreference contactPreference = null;
            phoneUri = it.next();
            // setPhoneUri may throw an IllegalArgumentException (also called in the constructor
            // of ContactPreference)
            try {
                if (i < getPreferenceCount()) {
                    contactPreference = (ContactPreference) getPreference(i);
                    contactPreference.setPhoneUri(phoneUri);
                } else {
                    contactPreference =
                            new ContactPreference(getContext(), phoneUri, mContactFactory);
                    onBindContactView(contactPreference);
                    addPreference(contactPreference);
                }
                i++;
                MetricsLogger.action(getContext(), MetricsEvent.ACTION_GET_CONTACT, 0);
            } catch (IllegalArgumentException e) {
                Log.w(TAG, "Caught IllegalArgumentException for phoneUri:"
                    + phoneUri == null ? "" : phoneUri.toString(), e);
                MetricsLogger.action(getContext(), MetricsEvent.ACTION_GET_CONTACT, 1);
                if (updatedEmergencyContacts == null) {
                    updatedEmergencyContacts = new ArrayList<>(emergencyContacts);
                }
                updatedEmergencyContacts.remove(phoneUri);
            }
        }

        /// M: SIM contact support @{
        Map<String, String> uriPhoneNumMap = new HashMap<String, String>();
        ArrayList<String> uriPhoneNumList = new ArrayList<String>();
        for (i = getPreferenceCount() - 1; i >= 0; i--) {
            ContactPreference contactPreference = (ContactPreference) getPreference(i);
            if (contactPreference.getContact().getPhoneNumber() != null) {
                uriPhoneNumMap.put(contactPreference.getContact().getPhoneUri().toString(),
                        contactPreference.getContact().getPhoneNumber());
            } else {
                Log.w(TAG, "get contact number is null");
            }
        }
        SharedPreferences sharedPref = this.getPreferenceManager().getSharedPreferences();
        sharedPref.edit().putString(getKey() + PreferenceKeys.KEY_CONTACT_SUFFIX_MAP,
                serializeMap(uriPhoneNumMap)).commit();
        Log.d(TAG, "write map size: " + uriPhoneNumMap.size());
        /// M: @}

        if (updatedEmergencyContacts != null) {
            // Set the contacts again: something went wrong when retrieving information about the
            // stored phone Uris.
            setEmergencyContacts(updatedEmergencyContacts);
        }
        // Enable or disable the settings suggestion, as appropriate.
        PreferenceUtils.updateSettingsSuggestionState(getContext());
        MetricsLogger.histogram(getContext(),
                                "num_emergency_contacts",
                                Math.min(3, emergencyContacts.size()));
    }

    /// M: SIM contact support @{
    private static Map<String, String> deserializeMap(String mapString) {
        Log.d(TAG, "mapString = " + mapString);
        String[] uriPhoneNumArray = mapString
                .split(QUOTE_CONTACT_SEPARATOR);
        Map<String, String> retMap = new HashMap<String, String>();
        for (int i = 0; i < uriPhoneNumArray.length / 2; i++) {
            retMap.put(uriPhoneNumArray[i * 2], uriPhoneNumArray[i * 2 + 1]);
        }
        return retMap;
    }

    private static String serializeMap(Map<String, String> uriPhoneNumMap) {
        StringBuilder sb = new StringBuilder();
        Set<String> keys = uriPhoneNumMap.keySet();
        Iterator<String> keyIt = keys.iterator();
        while (keyIt.hasNext()) {
            String key = keyIt.next();
            sb.append(key);
            sb.append(CONTACT_SEPARATOR);
            sb.append(uriPhoneNumMap.get(key));
            sb.append(CONTACT_SEPARATOR);
        }
        if (sb.length() > 0) {
            sb.setLength(sb.length() - 1);
        }
        Log.d(TAG, "write map: " + sb.toString());
        return sb.toString();
    }

    private static List<String> deserializeList(String listString) {
        Log.d(TAG, "listString = " + listString);
        List<String> retList = new ArrayList<String>();
        if (listString != null && !listString.isEmpty()) {
            String[] phoneNumArray = listString.split(QUOTE_CONTACT_SEPARATOR);
            retList.addAll(Arrays.asList(phoneNumArray));
        }
        return retList;
    }

    private static String serializeList(List<String> phoneNumList) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < phoneNumList.size(); i++) {
            sb.append(phoneNumList.get(i).toString());
            sb.append(CONTACT_SEPARATOR);
        }
        if (sb.length() > 0) {
            sb.setLength(sb.length() - 1);
        }
        Log.d(TAG, "serializeList: " + sb.toString());
        return sb.toString();
    }

    private void removeNumFromCache(String num) {
        Log.d(TAG, "removeNumFromCache: " + num);
        SharedPreferences sharedPref = this.getPreferenceManager().getSharedPreferences();
        List<String> numCacheList = EmergencyContactsPreference.deserializeList(
                sharedPref.getString(getKey() + PreferenceKeys.KEY_CONTACT_SUFFIX_NUM, ""));
        if (numCacheList.remove(num)) {
            sharedPref.edit().putString(
                    getKey() + PreferenceKeys.KEY_CONTACT_SUFFIX_NUM,
                    serializeList(numCacheList)).commit();
        } else {
            Log.w(TAG, "not in numCacheList: " + num);
        }
    }
    /// M: @}

    /**
     * Called when {@code contactPreference} has been added to this category. You may now set
     * listeners.
     */
    protected void onBindContactView(final ContactPreference contactPreference) {
        contactPreference.setRemoveContactPreferenceListener(this);
        contactPreference
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                contactPreference.displayContact();
                                return true;
                            }
                        }
                );
    }

    private List<Uri> getPersistedEmergencyContacts() {
        return deserializeAndFilter(getKey(), getContext(), getPersistedString(""),
                mContactValidator);
    }

    @Override
    protected String getPersistedString(String defaultReturnValue) {
        try {
            return super.getPersistedString(defaultReturnValue);
        } catch (ClassCastException e) {
            // Protect against b/28194605: We used to store the contacts using a string set.
            // If it was a string set, a ClassCastException would have been thrown, and we can
            // ignore its value. If it is stored as a value of another type, we are potentially
            // squelching an exception here, but returning the default return value seems reasonable
            // in either case.
            return defaultReturnValue;
        }
    }

    /**
     * Converts the string representing the emergency contacts to a list of Uris and only keeps
     * those corresponding to still existing contacts. It persists the contacts if at least one
     * contact was does not exist anymore.
     */
    public static List<Uri> deserializeAndFilter(String key, Context context,
                                                 String emergencyContactString) {
        return deserializeAndFilter(key, context, emergencyContactString,
                DEFAULT_CONTACT_VALIDATOR);
    }

    /** Converts the Uris to a string representation. */
    public static String serialize(List<Uri> emergencyContacts) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < emergencyContacts.size(); i++) {
            sb.append(emergencyContacts.get(i).toString());
            sb.append(CONTACT_SEPARATOR);
        }

        if (sb.length() > 0) {
            sb.setLength(sb.length() - 1);
        }
        return sb.toString();
    }

    @VisibleForTesting
    void persistEmergencyContacts(List<Uri> emergencyContacts) {
        persistString(serialize(emergencyContacts));
    }

    private static List<Uri> deserializeAndFilter(String key, Context context,
                                                  String emergencyContactString,
                                                  ContactValidator contactValidator) {
        String[] emergencyContactsArray =
                emergencyContactString.split(QUOTE_CONTACT_SEPARATOR);
        List<Uri> filteredEmergencyContacts = new ArrayList<Uri>(emergencyContactsArray.length);
        /// M: SIM contact support @{
        Map<String, String> uriNumMap = EmergencyContactsPreference
                .deserializeMap(PreferenceManager.getDefaultSharedPreferences(
                        context).getString(
                        key + PreferenceKeys.KEY_CONTACT_SUFFIX_MAP, ""));
        List<String> numCacheList = EmergencyContactsPreference
                .deserializeList(PreferenceManager.getDefaultSharedPreferences(
                        context).getString(
                        key + PreferenceKeys.KEY_CONTACT_SUFFIX_NUM, ""));
        List<String> numCacheNew = new ArrayList<String>();
        boolean uriUpdated = false;
        /// M: @}
        for (String emergencyContact : emergencyContactsArray) {
            Uri phoneUri = Uri.parse(emergencyContact);
            if (contactValidator.isValidEmergencyContact(context, phoneUri)) {
                filteredEmergencyContacts.add(phoneUri);
            /// M: SIM contact support @{
            } else if (phoneUri != null) {
                String phoneNum = uriNumMap.get(phoneUri.toString());
                if (phoneNum != null && !phoneNum.isEmpty()) {
                    Uri newUri = EmergencyContactManager.findUriByNum(context, phoneNum);
                    if (newUri != null) {
                        Log.d(TAG, "update from oldUri=" + phoneUri + " to newUri=" + newUri);
                        if (!filteredEmergencyContacts.contains(newUri)) {
                            uriUpdated = true;
                            filteredEmergencyContacts.add(newUri);
                            // update cache
                            uriNumMap.remove(phoneUri.toString());
                            uriNumMap.put(newUri.toString(), phoneNum);
                        } else {
                            Log.w(TAG, "already in contacts list: " + newUri);
                        }
                    } else {
                        // Put number to cache
                        Log.d(TAG, "Put number to cache: " + phoneNum);
                        uriNumMap.remove(phoneUri.toString());
                        numCacheNew.add(phoneNum);
                    }
                } else {
                    Log.d(TAG, "can't from map find phoneUri = " + phoneUri);
                }
            /// M: @}
            }
        }
        /// M: SIM contact support @{
        numCacheList.removeAll(numCacheNew);
        for (int i = 0; i < numCacheList.size(); i++) {
            String num = numCacheList.get(i);
            Uri newUri = EmergencyContactManager.findUriByNum(context, num);
            if (newUri != null) {
                uriUpdated = true;
                filteredEmergencyContacts.add(newUri);
                // update cache
                uriNumMap.put(newUri.toString(), num);
                numCacheList.remove(i--);
                Log.d(TAG, "add cache num: " + num + " with uri: " + newUri);
            }
        }
        numCacheList.addAll(numCacheNew);
        /// M: @}

        // If not all contacts were added, then we need to overwrite the emergency contacts stored
        // in shared preferences. This deals with emergency contacts being deleted from contacts:
        // currently we have no way to being notified when this happens.

        /// M: SIM contact support
        if (filteredEmergencyContacts.size() != emergencyContactsArray.length || uriUpdated) {
            String emergencyContactStrings = serialize(filteredEmergencyContacts);
            SharedPreferences sharedPreferences =
                    PreferenceManager.getDefaultSharedPreferences(context);
            sharedPreferences.edit().putString(key, emergencyContactStrings).commit();
            /// M: SIM contact support
            sharedPreferences.edit().putString(key + PreferenceKeys.KEY_CONTACT_SUFFIX_MAP,
                    serializeMap(uriNumMap)).commit();
        }
        /// M: SIM contact support @{
        while (numCacheList.size() > NUM_CACHE_SIZE) {
            Log.d(TAG, "remove from num cache: " + numCacheList.get(0));
            numCacheList.remove(0);
        }
        SharedPreferences sharedPreferences =
            PreferenceManager.getDefaultSharedPreferences(context);
        sharedPreferences.edit().putString(key + PreferenceKeys.KEY_CONTACT_SUFFIX_NUM,
                serializeList(numCacheList)).commit();
        /// M: @}
        return filteredEmergencyContacts;
    }
}
