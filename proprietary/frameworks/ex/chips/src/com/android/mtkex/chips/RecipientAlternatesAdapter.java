/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.mtkex.chips;

import android.accounts.Account;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MergeCursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;
import android.util.Log;
import android.util.Patterns;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.mtkex.chips.BaseRecipientAdapter.DirectoryListQuery;
import com.android.mtkex.chips.BaseRecipientAdapter.DirectorySearchParams;
import com.android.mtkex.chips.Queries.Query;
import android.os.SystemProperties;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * RecipientAlternatesAdapter backs the RecipientEditTextView for managing contacts
 * queried by email or by phone number.
 */
public class RecipientAlternatesAdapter extends CursorAdapter {
    static final int MAX_LOOKUPS = 100; /// M: Let chips to be parsed can be up to 100
    private final LayoutInflater mLayoutInflater;

    private final long mCurrentId;

    private int mCheckedItemPosition = -1;

    private OnCheckedItemChangedListener mCheckedItemChangedListener;

    private static final String TAG = "RecipAlternates";

    public static final int QUERY_TYPE_EMAIL = 0;
    public static final int QUERY_TYPE_PHONE = 1;

    /// M: Type to distinguish email and phone address.@{
    private static final int TYPE_EMAIL = 1;
    private static final int TYPE_PHONE = 2;
    /// M: }@

    private Query mQuery;

    private static boolean DEBUG_LOG = false; // M: Flag of whether print normal debug log

    static private boolean  piLoggable =
                       !("user".equals(SystemProperties.get("ro.build.type", "user"))) ;

    static private boolean  hideSensInEng = true;

    public interface RecipientMatchCallback {
        public void matchesFound(Map<String, RecipientEntry> results);
        /**
         * Called with all addresses that could not be resolved to valid recipients.
         */
        public void matchesNotFound(Set<String> unfoundAddresses);
    }

    /*
     * M: This method used for judge whether given address is an Email address.
     */
    private static boolean isEmailType(String address) {
        if (address != null && address.contains("@")) {
            return true;
        }
        return false;
    }

    public static void getMatchingRecipients(HashSet<String> existChipsNameSet,
            Context context, BaseRecipientAdapter adapter,
            ArrayList<String> inAddresses, Account account, RecipientMatchCallback callback) {
        getMatchingRecipients(existChipsNameSet, context, adapter,
                inAddresses, QUERY_TYPE_EMAIL, account, callback);
    }

    /**
     * Get a HashMap of address to RecipientEntry that contains all contact
     * information for a contact with the provided address, if one exists. This
     * may block the UI, so run it in an async task.
     *
     * @param context Context.
     * @param inAddresses Array of addresses on which to perform the lookup.
     * @param callback RecipientMatchCallback called when a match or matches are found.
     * @return HashMap<String,RecipientEntry>
     */
    public static void getMatchingRecipients(HashSet<String> existChipsNameSet,
            Context context, BaseRecipientAdapter adapter,
            ArrayList<String> inAddresses, int addressType, Account account,
            RecipientMatchCallback callback) {
        Log.d(TAG, "[getMatchingRecipients] Start");    /// M: MTK debug log
        /// M: We have splitted some function to special method for reuse.@ {
        final int addressesSize = Math.min(MAX_LOOKUPS, inAddresses.size());
        ArrayList<String> emailAddressesList = new ArrayList<String>();
        ArrayList<String> phoneAddressesList = new ArrayList<String>();
        int[] addressTypeIndex = new int[addressesSize];

        splitAddressToEmailAndPhone(inAddresses, emailAddressesList,
                phoneAddressesList, addressTypeIndex);

        HashMap<String, RecipientEntry> recipientEntries = new HashMap<String, RecipientEntry>();

        Cursor cEmail = queryAddressData(context, emailAddressesList, QUERY_TYPE_EMAIL);
        Cursor cPhone = queryAddressData(context, phoneAddressesList, QUERY_TYPE_PHONE);

        if (cEmail != null && cPhone == null) {
            fillRecipientEntries(existChipsNameSet, cEmail, recipientEntries, callback);
            processMatchesNotFound(context, adapter, account,
                    Queries.EMAIL, inAddresses, recipientEntries, callback);
        } else if (cEmail == null && cPhone != null) {
            fillRecipientEntries(existChipsNameSet, cPhone, recipientEntries, callback);
            processMatchesNotFound(context, adapter, account,
                    Queries.PHONE, inAddresses, recipientEntries, callback);
        } else if (cEmail != null && cPhone != null) {
            HashMap<String, RecipientEntry> emailRecipientEntries =
                new HashMap<String, RecipientEntry>();
            HashMap<String, RecipientEntry> phoneRecipientEntries =
                new HashMap<String, RecipientEntry>();
            fillRecipientEntriesCompound(existChipsNameSet, cEmail, cPhone,
                    recipientEntries, emailRecipientEntries,
                    phoneRecipientEntries, addressesSize, addressTypeIndex, callback);
            processMatchesNotFound(context, adapter, account,
                    Queries.EMAIL, emailAddressesList, emailRecipientEntries, callback);
            printSensitiveDebugLog(TAG,
            "phoneRecipientEntries.keySet() = " + phoneRecipientEntries.keySet()
                    + " phoneAddressesList = " + phoneAddressesList
                    + " emailRecipientEntries.keySet() = " + emailRecipientEntries.keySet()
                    + " emailAddressesList = " + emailAddressesList);
            processMatchesNotFound(context, adapter, account,
                    Queries.PHONE, phoneAddressesList, phoneRecipientEntries, callback);
        }
        /// M: }@
    }

    /*
     * M: This method used for split the addresses to Email and Phone addresses.
     */
    private static void splitAddressToEmailAndPhone(ArrayList<String> inAddresses,
                ArrayList<String> emailAddresses, ArrayList<String> phoneAddresses, int[] index) {
        final int addressSize = Math.min(MAX_LOOKUPS, inAddresses.size());
        for (int i = 0; i < addressSize; i++) {
            if (isEmailType(inAddresses.get(i))) {
                emailAddresses.add(inAddresses.get(i));
                index[i] = TYPE_EMAIL;
            } else {
                phoneAddresses.add(inAddresses.get(i));
                index[i] = TYPE_PHONE;
            }
        }
    }

    /*
     * M: This method used for queryData from database of email or phone addresses.
     */
    private static Cursor queryAddressData(Context context, ArrayList<String> addressesList,
            int addressType) {
        final int addressesSize = Math.min(MAX_LOOKUPS, addressesList.size());

        StringBuilder bindString = new StringBuilder();
        String[] addresses = new String[addressesSize];

        Queries.Query query;
        if (addressType == QUERY_TYPE_EMAIL) {
            query = Queries.EMAIL;
        } else {
            query = Queries.PHONE;
        }

        // Create the "?" string and set up arguments.
        String queryStr = ""; /// M: For query phone number with (,),-.
        if (addressType == QUERY_TYPE_EMAIL) {
            for (int i = 0; i < addressesSize; i++) {
                Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(addressesList.get(i));
                addresses[i] = (tokens.length > 0 ? tokens[0].getAddress() : addressesList.get(i));
                bindString.append("?");
                if (i < addressesSize - 1) {
                    bindString.append(",");
                }
            }
        } else {
            /// M: For query phone number with (,),-. @{
            String phoneStr = "";
            for (int i = 0; i < addressesSize; i++) {
                phoneStr = addressesList.get(i);
                /// M: Support recognizing two kinds of separator. Remove comma
                /// and semicolon at the end of address if exists. @{
                phoneStr = phoneStr.replaceAll("([, ]+$)|([; ]+$)|([\"]+$)", "");
                /// @}
                /// M: MTK Version for ALPS00934864
                if (!MTKRecipientEditTextView.PHONE_EX.matcher(phoneStr).matches()) {
                    Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(phoneStr);
                    phoneStr = (tokens.length > 0 ? tokens[0].getAddress() : phoneStr);
                }
                queryStr += "\"" + phoneStr + "\"";
                bindString.append("?");
                if (i < addressesSize - 1) {
                    queryStr += ",";
                    bindString.append(",");
                }
            }
            /// @}
        }

        if (Log.isLoggable(TAG, Log.DEBUG)) {
            printSensitiveDebugLog(TAG, "Doing reverse lookup for " + addresses.toString());
        }

        Cursor cursor = null;

        if (addressesList.size() > 0) {
            if (addressType == QUERY_TYPE_EMAIL) {
                String selection = query.getProjection()[Queries.Query.DESTINATION]
                                                         + " IN (" + bindString.toString() + ")";
                printSensitiveDebugLog(TAG,
                "[queryAddressData] selection: " + selection);    /// M: MTK debug log
                try {
                    cursor = context.getContentResolver().query(
                        query.getContentUri(),
                        query.getProjection(),
                        selection, addresses, null);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "QUERY_TYPE_EMAIL meet IllegalArgumentException " +  e);
                    return null;
                }
                printSensitiveDebugLog(TAG, "addresses = " + addresses);
            } else {
                /// M: For query phone number with (,),-. @{
                String selection = query.getProjection()[Queries.Query.DESTINATION]
                                                         + " IN (" + queryStr + ")";
                printSensitiveDebugLog(TAG,
                "[queryAddressData] selection: " + selection);    /// M: MTK debug log
                try {
                    cursor = context.getContentResolver().query(
                        query.getContentUri(),
                        query.getProjection(),
                        selection, null, Phone.DISPLAY_NAME + " DESC");
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "QUERY_TYPE_PHONE meet IllegalArgumentException " +  e);
                    return null;
                }
                /// @}
            }
        }

        Log.d(TAG, "[queryAddressData] cursor count: "
                + (cursor != null ? cursor.getCount() : "null"));    /// M: MTK debug log
        return cursor;
    }

    /*
     * M: This method used for fill RecipientEntries with single type addresses.
     */
    private static void fillRecipientEntries(HashSet<String> existChipsNameSet,
            Cursor cursor, HashMap<String, RecipientEntry> recipientEntries,
            RecipientMatchCallback callback) {
        Log.d(TAG, "[fillRecipientEntries] start");
        try {
            if (cursor.moveToFirst()) {
                do {
                    if (!existChipsNameSet.contains(cursor.getString(Queries.Query.DESTINATION))) {
                        continue;
                    }
                    String address = cursor.getString(Queries.Query.DESTINATION);
                    recipientEntries.put(address, RecipientEntry.constructTopLevelEntry(
                            cursor.getString(Queries.Query.NAME),
                            cursor.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                            cursor.getString(Queries.Query.DESTINATION),
                            cursor.getInt(Queries.Query.DESTINATION_TYPE),
                            cursor.getString(Queries.Query.DESTINATION_LABEL),
                            cursor.getLong(Queries.Query.CONTACT_ID),
                            cursor.getLong(Queries.Query.DATA_ID),
                            cursor.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                            true, false));
                    if (true) {
                        printSensitiveDebugLog(TAG,
                        "Received reverse look up information for " + address
                                + " RESULTS: "
                                + " NAME : " + cursor.getString(Queries.Query.NAME)
                                + " CONTACT ID : " + cursor.getLong(Queries.Query.CONTACT_ID)
                                + " ADDRESS :" + cursor.getString(Queries.Query.DESTINATION));
                    }
                } while (cursor.moveToNext());
            }
            callback.matchesFound(recipientEntries);
        } finally {
            cursor.close();
        }
    }

    /*
     * M: This method used for fill RecipientEntries with mult-type addresses.
     */
    private static void fillRecipientEntriesCompound(HashSet<String> existChipsNameSet,
            Cursor cEmail, Cursor cPhone,
            HashMap<String, RecipientEntry> recipientEntries,
            HashMap<String, RecipientEntry> emailRecipientEntries,
            HashMap<String, RecipientEntry> phoneRecipientEntries,
            int addressesSize, int[] addressTypeIndex,
            RecipientMatchCallback callback) {
        Log.d(TAG, "[fillRecipientEntriesCompound] start");
        //merge two list in one
        try {
            cEmail.moveToFirst();
            cPhone.moveToFirst();
            boolean shouldQueryEmail = true;
            boolean shouldQueryPhone = true;
            for (int i = 0; i < addressesSize; ) {
                Log.d(TAG, "fillRecipientEntriesCompound addressesSize = " + addressesSize);
                if (addressTypeIndex[i] == TYPE_EMAIL
                        && shouldQueryEmail && cEmail.getCount() != 0) {
                    if (!existChipsNameSet.contains(cEmail.getString(Queries.Query.DESTINATION))) {
                        shouldQueryEmail = cEmail.moveToNext();
                        continue;
                    }
                } else if (shouldQueryPhone && cPhone.getCount() != 0) {
                    if (!existChipsNameSet.contains(cPhone.getString(Queries.Query.DESTINATION))) {
                        shouldQueryPhone = cPhone.moveToNext();
                        continue;
                    }
                }

                if (addressTypeIndex[i] == TYPE_EMAIL
                        && shouldQueryEmail && cEmail.getCount() != 0) {

                    String address = cEmail.getString(Queries.Query.DESTINATION);
                    /// M: ignore duplicated result
                    if (recipientEntries.containsKey(address)) {
                        RecipientEntry tempEntry = recipientEntries.get(address);
                        if (tempEntry != null &&
                                tempEntry.getDisplayName()
                                    .equals(cEmail.getString(Queries.Query.NAME))) {
                            shouldQueryEmail = cEmail.moveToNext();
                            continue;
                        }
                    }
                    RecipientEntry entry = RecipientEntry.constructTopLevelEntry(
                            cEmail.getString(Queries.Query.NAME),
                            cEmail.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                            cEmail.getString(Queries.Query.DESTINATION),
                            cEmail.getInt(Queries.Query.DESTINATION_TYPE),
                            cEmail.getString(Queries.Query.DESTINATION_LABEL),
                            cEmail.getLong(Queries.Query.CONTACT_ID),
                            cEmail.getLong(Queries.Query.DATA_ID),
                            cEmail.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                            true, false);
                    recipientEntries.put(address, entry);
                    emailRecipientEntries.put(address, entry);
                    if (true) {
                        printSensitiveDebugLog(TAG,
                        "Received reverse look up information for " + address
                                + " RESULTS: "
                                + " NAME : " + cEmail.getString(Queries.Query.NAME)
                                + " CONTACT ID : " + cEmail.getLong(Queries.Query.CONTACT_ID)
                                + " ADDRESS :" + cEmail.getString(Queries.Query.DESTINATION));
                    }
                    shouldQueryEmail = cEmail.moveToNext();
                } else {
                    if (shouldQueryPhone && cPhone.getCount() != 0) {
                        String address = cPhone.getString(Queries.Query.DESTINATION);
                        /// M: ignore duplicated result
                        if (recipientEntries.containsKey(address)) {
                            RecipientEntry tempEntry = recipientEntries.get(address);
                            if (tempEntry != null &&
                                    tempEntry.getDisplayName().equals(
                                            cPhone.getString(Queries.Query.NAME))) {
                                shouldQueryPhone = cPhone.moveToNext();
                                continue;
                            }
                        }
                        RecipientEntry entry = RecipientEntry.constructTopLevelEntry(
                                cPhone.getString(Queries.Query.NAME),
                                cPhone.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                                cPhone.getString(Queries.Query.DESTINATION),
                                cPhone.getInt(Queries.Query.DESTINATION_TYPE),
                                cPhone.getString(Queries.Query.DESTINATION_LABEL),
                                cPhone.getLong(Queries.Query.CONTACT_ID),
                                cPhone.getLong(Queries.Query.DATA_ID),
                                cPhone.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                                true, false);
                        recipientEntries.put(address, entry);
                        phoneRecipientEntries.put(address, entry);
                        if (true) {
                            printSensitiveDebugLog(TAG,
                            "Received reverse look up information for " + address
                                    + " RESULTS: "
                                    + " NAME : " + cPhone.getString(Queries.Query.NAME)
                                    + " CONTACT ID : " + cPhone.getLong(Queries.Query.CONTACT_ID)
                                    + " ADDRESS :" + cPhone.getString(Queries.Query.DESTINATION));
                        }
                        shouldQueryPhone = cPhone.moveToNext();
                    }
                }
                ++i;
            }
            printSensitiveDebugLog(TAG, "recipientEntries.keySet() " + recipientEntries.keySet());
            callback.matchesFound(recipientEntries);
        } finally {
            cEmail.close();
            cPhone.close();
        }
    }

    /*
     * M: This method used for querying unfound addresses.
     */
    private static void processMatchesNotFound(Context context, BaseRecipientAdapter adapter,
            Account account, Query query, ArrayList<String> addresses, HashMap<String,
            RecipientEntry> recipientEntries, RecipientMatchCallback callback) {
        Log.d(TAG, "[processMatchesNotFound] start");    /// M: MTK debug log
        // See if any entries did not resolve; if so, we need to check other
        // directories
        final Set<String> matchesNotFound = new HashSet<String>();
        if (recipientEntries.size() < addresses.size()) {
            final List<DirectorySearchParams> paramsList;
            Cursor directoryCursor = null;
            try {
                directoryCursor = context.getContentResolver().query(DirectoryListQuery.URI,
                        DirectoryListQuery.PROJECTION, null, null, null);
                if (directoryCursor == null) {
                    paramsList = null;
                } else {
                    paramsList = BaseRecipientAdapter.setupOtherDirectories(context,
                            directoryCursor, account);
                }
            } finally {
                if (directoryCursor != null) {
                    directoryCursor.close();
                }
            }
            // Run a directory query for each unmatched recipient.
            HashSet<String> unresolvedAddresses = new HashSet<String>();

            for (String address : addresses) {
                /// M: convert to correct address format
                address = address.replaceAll("([, ]+$)|([; ]+$)", "");
                if (!Patterns.PHONE.matcher(address).matches()) {
                    Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(address);
                    address = (tokens.length > 0 ? tokens[0].getAddress() : address);
                }
                /// @}
                printSensitiveDebugLog(TAG, "query address after parsed = " + address);
                if (!recipientEntries.containsKey(address)) {
                    unresolvedAddresses.add(address);
                }
            }
            printSensitiveDebugLog(TAG, "matchesNotFound = " + matchesNotFound);
            matchesNotFound.addAll(unresolvedAddresses);

            if (paramsList != null) {
                Cursor directoryContactsCursor = null;
                /// M: Deal With this situation of too many callback.matchesFound
                Map<String, RecipientEntry> matchEntries = new HashMap<String, RecipientEntry>();
                for (String unresolvedAddress : unresolvedAddresses) {
                    if (query == Queries.PHONE) {
                        RecipientEntry entry = getRecipientEntryByPhoneNumber(
                                context, unresolvedAddress);
                        if (entry != null) {
                            matchEntries.put(unresolvedAddress, entry);
                            matchesNotFound.remove(unresolvedAddress);
                            continue;
                        }
                    }
                    for (int i = 0; i < paramsList.size(); i++) {
                        try {
                            directoryContactsCursor = doQuery(unresolvedAddress, 1,
                                    paramsList.get(i).directoryId, account,
                                    context.getContentResolver(), query);
                        } finally {
                            if (directoryContactsCursor != null
                                    && directoryContactsCursor.getCount() == 0) {
                                directoryContactsCursor.close();
                                directoryContactsCursor = null;
                            }
                            if (directoryContactsCursor != null) {
                               break;
                            }
                        }
                    }
                    if (directoryContactsCursor != null) {
                        try {
                            /// M: Deal with GAL Contacts.
                            final Map<String, RecipientEntry> entries =
                                    processContactEntries(directoryContactsCursor, true);

                            for (final String address : entries.keySet()) {
                                matchesNotFound.remove(address);
                            }
                            /// M: Deal With this situation of too many callback.matchesFound
                            matchEntries.putAll(entries);
                            Log.d(TAG, "entries.size(): " + entries.size());
                            // callback.matchesFound(entries);
                        } finally {
                            directoryContactsCursor.close();
                        }
                    } else {
                        if (query == Queries.PHONE) {
                            RecipientEntry entry = getRecipientEntryByPhoneNumber(
                                    context, unresolvedAddress);
                            if (entry != null) {
                                matchEntries.put(unresolvedAddress, entry);
                                matchesNotFound.remove(unresolvedAddress);
                            }
                        }
                    }
                }
                /// M: Deal With this situation of too many callback.matchesFound.
                callback.matchesFound(matchEntries);
            }
        }

        // If no matches found in contact provider or the directories, try the extension
        // matcher.
        // todo (aalbert): This whole method needs to be in the adapter?
        if (adapter != null) {
            final Map<String, RecipientEntry> entries =
                    adapter.getMatchingRecipients(matchesNotFound);
            if (entries != null && entries.size() > 0) {
                callback.matchesFound(entries);
                for (final String address : entries.keySet()) {
                    matchesNotFound.remove(address);
                }
            }
        }
        callback.matchesNotFound(matchesNotFound);
    }
    /// M: Deal with GAL Contacts. @{
    private static HashMap<String, RecipientEntry> processContactEntries(Cursor c,
            Boolean isGalContact) {
        HashMap<String, RecipientEntry> recipientEntries = new HashMap<String, RecipientEntry>();
        if (c != null && c.moveToFirst()) {
            do {
                String address = c.getString(Queries.Query.DESTINATION);

                final RecipientEntry newRecipientEntry = RecipientEntry.constructTopLevelEntry(
                        c.getString(Queries.Query.NAME),
                        c.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                        c.getString(Queries.Query.DESTINATION),
                        c.getInt(Queries.Query.DESTINATION_TYPE),
                        c.getString(Queries.Query.DESTINATION_LABEL),
                        c.getLong(Queries.Query.CONTACT_ID),
                        c.getLong(Queries.Query.DATA_ID),
                        c.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                        true,
                        isGalContact);

                /*
                 * In certain situations, we may have two results for one address, where one of the
                 * results is just the email address, and the other has a name and photo, so want
                 * to use the better one.
                 */
                final RecipientEntry recipientEntry =
                        getBetterRecipient(recipientEntries.get(address), newRecipientEntry);

                recipientEntries.put(address, recipientEntry);
                if (true) {
                    printSensitiveDebugLog(TAG,
                    "Received reverse look up information for " + address
                            + " RESULTS: "
                            + " NAME : " + c.getString(Queries.Query.NAME)
                            + " CONTACT ID : " + c.getLong(Queries.Query.CONTACT_ID)
                            + " ADDRESS :" + c.getString(Queries.Query.DESTINATION));
                }
            } while (c.moveToNext());
        }
        return recipientEntries;
    }
    private static HashMap<String, RecipientEntry> processContactEntries(Cursor c) {
        return processContactEntries(c, false);
    }
    /// @}

    /**
     * Given two {@link RecipientEntry}s for the same email address, this will return the one that
     * contains more complete information for display purposes. Defaults to <code>entry2</code> if
     * no significant differences are found.
     */
    static RecipientEntry getBetterRecipient(final RecipientEntry entry1,
            final RecipientEntry entry2) {
        // If only one has passed in, use it
        if (entry2 == null) {
            return entry1;
        }

        if (entry1 == null) {
            return entry2;
        }
        if (!RecipientEntry.isCreatedRecipient(entry1.getContactId())
                && RecipientEntry.isCreatedRecipient(entry2.getContactId())) {
            return entry1;
        }
        if (!RecipientEntry.isCreatedRecipient(entry2.getContactId())
                && RecipientEntry.isCreatedRecipient(entry1.getContactId())) {
            return entry2;
        }
        // If only one has a display name, use it
        if (!TextUtils.isEmpty(entry1.getDisplayName())
                && TextUtils.isEmpty(entry2.getDisplayName())) {
            return entry1;
        }

        if (!TextUtils.isEmpty(entry2.getDisplayName())
                && TextUtils.isEmpty(entry1.getDisplayName())) {
            return entry2;
        }

        // If only one has a display name that is not the same as the destination, use it
        if (!TextUtils.equals(entry1.getDisplayName(), entry1.getDestination())
                && TextUtils.equals(entry2.getDisplayName(), entry2.getDestination())) {
            return entry1;
        }

        if (!TextUtils.equals(entry2.getDisplayName(), entry2.getDestination())
                && TextUtils.equals(entry1.getDisplayName(), entry1.getDestination())) {
            return entry2;
        }

        // If only one has a photo, use it
        if ((entry1.getPhotoThumbnailUri() != null || entry1.getPhotoBytes() != null)
                && (entry2.getPhotoThumbnailUri() == null && entry2.getPhotoBytes() == null)) {
            return entry1;
        }

        if ((entry2.getPhotoThumbnailUri() != null || entry2.getPhotoBytes() != null)
                && (entry1.getPhotoThumbnailUri() == null && entry1.getPhotoBytes() == null)) {
            return entry2;
        }

        // Go with the second option as a default
        return entry2;
    }

    private static Cursor doQuery(CharSequence constraint, int limit, Long directoryId,
            Account account, ContentResolver resolver, Query query) {
        final Uri.Builder builder = query
                .getContentFilterUri()
                .buildUpon()
                .appendPath(constraint.toString())
                .appendQueryParameter(ContactsContract.LIMIT_PARAM_KEY,
                        String.valueOf(limit + BaseRecipientAdapter.ALLOWANCE_FOR_DUPLICATES));
        if (directoryId != null) {
            builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                    String.valueOf(directoryId));
        }
        if (account != null) {
            builder.appendQueryParameter(BaseRecipientAdapter.PRIMARY_ACCOUNT_NAME, account.name);
            builder.appendQueryParameter(BaseRecipientAdapter.PRIMARY_ACCOUNT_TYPE, account.type);
        }
        final Cursor cursor = resolver.query(builder.build(), query.getProjection(), null, null,
                null);
        return cursor;
    }

    public RecipientAlternatesAdapter(Context context, long contactId, long currentId,
            OnCheckedItemChangedListener listener) {
        this(context, contactId, currentId, QUERY_TYPE_EMAIL, listener);
    }

    public RecipientAlternatesAdapter(Context context, long contactId, long currentId,
            int queryMode, OnCheckedItemChangedListener listener) {
        super(context, getCursorForConstruction(context, contactId, queryMode), 0);
        Log.d(TAG, "[RecipientAlternatesAdapter] queryMode: " + queryMode
                + " mCurrentId = " + currentId);    /// M: MTK debug log
        mLayoutInflater = LayoutInflater.from(context);
        mCurrentId = currentId;
        mCheckedItemChangedListener = listener;

        if (queryMode == QUERY_TYPE_EMAIL) {
            mQuery = Queries.EMAIL;
        } else if (queryMode == QUERY_TYPE_PHONE) {
            mQuery = Queries.PHONE;
        } else {
            mQuery = Queries.EMAIL;
            Log.e(TAG, "Unsupported query type: " + queryMode);
        }
    }

    private static Cursor getCursorForConstruction(Context context,
            long contactId, int queryType) {
        final Cursor cursor;
        if (queryType == QUERY_TYPE_EMAIL) {
            cursor = context.getContentResolver().query(
                    Queries.EMAIL.getContentUri(),
                    Queries.EMAIL.getProjection(),
                    Queries.EMAIL.getProjection()[Queries.Query.CONTACT_ID] + " =?", new String[] {
                        String.valueOf(contactId)
                    }, null);
        } else {
            cursor = context.getContentResolver().query(
                    Queries.PHONE.getContentUri(),
                    Queries.PHONE.getProjection(),
                    Queries.PHONE.getProjection()[Queries.Query.CONTACT_ID] + " =?", new String[] {
                        String.valueOf(contactId)
                    }, null);
        }
        /// M: Close cursor in case of cursor leak
        Log.d(TAG, "[getCursorForConstruction] cursor count: " +
                (cursor != null ? cursor.getCount() : "null"));

        final Cursor resultCursor = removeDuplicateDestinations(cursor);
        if (cursor != null) {
            cursor.close();
        }
        return resultCursor;
    }

    /**
     * @return a new cursor based on the given cursor with all duplicate destinations removed.
     *
     * It's only intended to use for the alternate list, so...
     * - This method ignores all other fields and dedupe solely on the destination.  Normally,
     * if a cursor contains multiple contacts and they have the same destination, we'd still want
     * to show both.
     * - This method creates a MatrixCursor, so all data will be kept in memory.  We wouldn't want
     * to do this if the original cursor is large, but it's okay here because the alternate list
     * won't be that big.
     */
    // Visible for testing
    /* package */ static Cursor removeDuplicateDestinations(Cursor original) {
        /// M: Anoid null pointer exception in case of cursor is null @{
        if (null == original) {
            return null;
        }
        /// @}
        final MatrixCursor result = new MatrixCursor(
                original.getColumnNames(), original.getCount());
        final HashSet<String> destinationsSeen = new HashSet<String>();

        original.moveToPosition(-1);
        while (original.moveToNext()) {
            final String destination = original.getString(Query.DESTINATION);
            if (destinationsSeen.contains(destination)) {
                continue;
            }
            destinationsSeen.add(destination);

            result.addRow(new Object[] {
                    original.getString(Query.NAME),
                    original.getString(Query.DESTINATION),
                    original.getInt(Query.DESTINATION_TYPE),
                    original.getString(Query.DESTINATION_LABEL),
                    original.getLong(Query.CONTACT_ID),
                    original.getLong(Query.DATA_ID),
                    original.getString(Query.PHOTO_THUMBNAIL_URI),
                    original.getInt(Query.DISPLAY_NAME_SOURCE)
                    });
        }

        return result;
    }

    @Override
    public long getItemId(int position) {
        Cursor c = getCursor();
        if (c.moveToPosition(position)) {
            c.getLong(Queries.Query.DATA_ID);
        }
        return -1;
    }

    public RecipientEntry getRecipientEntry(int position) {
        Cursor c = getCursor();
        c.moveToPosition(position);
        return RecipientEntry.constructTopLevelEntry(
                c.getString(Queries.Query.NAME),
                c.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                c.getString(Queries.Query.DESTINATION),
                c.getInt(Queries.Query.DESTINATION_TYPE),
                c.getString(Queries.Query.DESTINATION_LABEL),
                c.getLong(Queries.Query.CONTACT_ID),
                c.getLong(Queries.Query.DATA_ID),
                c.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                true,
                false /* isGalContact TODO(skennedy) We should look these up eventually */);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        Cursor cursor = getCursor();
        cursor.moveToPosition(position);
        if (convertView == null) {
            convertView = newView();
        }
        Log.d(TAG, "getView cursor.getLong(Queries.Query.DATA_ID) "
                + cursor.getLong(Queries.Query.DATA_ID)
            + " ; mCurrentId " + mCurrentId + " position = " + position);
        if (cursor.getLong(Queries.Query.DATA_ID) == mCurrentId) {
            mCheckedItemPosition = position;
            if (mCheckedItemChangedListener != null) {
                Log.d(TAG, " getView call onCheckedItemChanged position = " + position);
                mCheckedItemChangedListener.onCheckedItemChanged(mCheckedItemPosition);
            }
        }
        bindView(convertView, convertView.getContext(), cursor);
        return convertView;
    }

    // TODO: this is VERY similar to the BaseRecipientAdapter. Can we combine
    // somehow?
    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        int position = cursor.getPosition();

        TextView display = (TextView) view.findViewById(android.R.id.title);
        ImageView imageView = (ImageView) view.findViewById(android.R.id.icon);
        RecipientEntry entry = getRecipientEntry(position);
        if (position == 0) {
            display.setText(cursor.getString(Queries.Query.NAME));
            display.setVisibility(View.VISIBLE);
            // TODO: see if this needs to be done outside the main thread
            // as it may be too slow to get immediately.
            imageView.setImageURI(entry.getPhotoThumbnailUri());
            imageView.setVisibility(View.VISIBLE);
        } else {
            display.setVisibility(View.GONE);
            imageView.setVisibility(View.GONE);
        }
        TextView destination = (TextView) view.findViewById(android.R.id.text1);
        destination.setText(cursor.getString(Queries.Query.DESTINATION));

        TextView destinationType = (TextView) view.findViewById(android.R.id.text2);
        if (destinationType != null) {
            destinationType.setText(mQuery.getTypeLabel(context.getResources(),
                    cursor.getInt(Queries.Query.DESTINATION_TYPE),
                    cursor.getString(Queries.Query.DESTINATION_LABEL)).toString().toUpperCase());
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        return newView();
    }

    private View newView() {
        return mLayoutInflater.inflate(R.layout.chips_recipient_dropdown_item, null);
    }

    /*package*/ static interface OnCheckedItemChangedListener {
        public void onCheckedItemChanged(int position);
    }

    /**
     * M: RecipientAlternatesAdapter constructor for phone query with showPhoneAndEmail flag.
     * @hide
     */
    public RecipientAlternatesAdapter(Context context, long contactId, long currentId,
            int queryMode, OnCheckedItemChangedListener listener, boolean showPhoneAndEmail) {
        super(context, getCursorForConstruction(context,
                contactId, queryMode, showPhoneAndEmail), 0);
        mLayoutInflater = LayoutInflater.from(context);
        mCurrentId = currentId;
        Log.d(TAG, "RecipientAlternatesAdapter mCurrentId = " + mCurrentId);
        mCheckedItemChangedListener = listener;

        if (queryMode == QUERY_TYPE_EMAIL) {
            mQuery = Queries.EMAIL;
        } else if (queryMode == QUERY_TYPE_PHONE) {
            mQuery = Queries.PHONE;
        } else {
            mQuery = Queries.EMAIL;
            Log.e(TAG, "Unsupported query type: " + queryMode);
        }
    }

    /**
     * M: GetCursorForConstruction for phone query with showPhoneAndEmail flag.
     */
    private static Cursor getCursorForConstruction(Context context,
            long contactId, int queryType, boolean showPhoneAndEmail) {
        final Cursor cursor;
        if (!showPhoneAndEmail) {
            cursor = context.getContentResolver().query(
                    Queries.PHONE.getContentUri(),
                    Queries.PHONE.getProjection(),
                    Queries.PHONE.getProjection()[Queries.Query.CONTACT_ID] + " =?", new String[] {
                        String.valueOf(contactId)
                    }, null);
        } else {
            /// M: Show phone number and email simutaneously when select chip
            Cursor[] cursors = new Cursor[2];
            cursors[0] = context.getContentResolver().query(
                    Queries.PHONE.getContentUri(),
                    Queries.PHONE.getProjection(),
                    Queries.PHONE.getProjection()[Queries.Query.CONTACT_ID] + " =?", new String[] {
                        String.valueOf(contactId)
                    }, null);
            cursors[1] =  context.getContentResolver().query(
                    Queries.EMAIL.getContentUri(),
                    Queries.EMAIL.getProjection(),
                    Queries.EMAIL.getProjection()[Queries.Query.CONTACT_ID] + " =?", new String[] {
                        String.valueOf(contactId)
                    }, null);
            cursor = new MergeCursor(cursors);
        }
        Log.d(TAG, "[getCursorForConstruction] cursor count: "
                + (cursor != null ? cursor.getCount() : "null"));

        /// M: Close cursor in case of cursor leak
        final Cursor resultCursor = removeDuplicateDestinations(cursor);
        if (cursor != null) {
            cursor.close();
        }
        return resultCursor;
    }

    /**
     * M: Get RecipientEntry by giving phone number (No matter the number is
     * normalized or not, we can still query it out).
     * @hide
     */
    public static RecipientEntry getRecipientEntryByPhoneNumber(Context context,
            String phoneNumber) {
        printSensitiveDebugLog(TAG, "[getRecipientEntryByPhoneNumber] phoneNumber: " + phoneNumber);
        if (phoneNumber == null || TextUtils.isEmpty(phoneNumber)) {
            return null;
        }
        final String[] PHONE_LOOKUP_PROJECTION = new String[] {
                    Phone._ID,                      // 0
                    Phone.CONTACT_ID,               // 1
                    Phone.NUMBER,                   // 2
                    Phone.NORMALIZED_NUMBER,        // 3
                    Phone.DISPLAY_NAME,             // 4
                };
        long index = -1;
        String normalizedNumber = PhoneNumberUtils.normalizeNumber(phoneNumber);
        /// M: Query CONTACT_ID by giving phone number
        Cursor cursorNormalize = null;
        try {
            cursorNormalize = context.getContentResolver().query(
                            Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, normalizedNumber),
                            PHONE_LOOKUP_PROJECTION, null, null, null);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "getRecipientEntryByPhoneNumber meet IllegalArgumentException " +  e);
            e.printStackTrace();
        }
        /// M: Return null if query result is empty
        if (cursorNormalize == null) {
            Log.d(TAG,
            "[getRecipientEntryByPhoneNumber] cursorNormalize is null");
            return null;
        }
        if (cursorNormalize.moveToFirst()) {
            do {
                index = cursorNormalize.getLong(1); /// M: Phone.CONTACT_ID
                if (Log.isLoggable(TAG, Log.DEBUG)) {
                    printSensitiveDebugLog(TAG,
                    "[getRecipientEntryByPhoneNumber] Query ID for " + phoneNumber
                            + " RESULTS: "
                            + " NAME : " + cursorNormalize.getString(4)
                            + " CONTACT ID : " + cursorNormalize.getLong(1)
                            + " ADDRESS :" + cursorNormalize.getString(2));
                }
            } while (cursorNormalize.moveToNext());
        }
        cursorNormalize.close();
        /// M: No matched contact
        if (index == -1) {
            return null;
        }
        /// M: Query contact information by giving CONTACT_ID
        RecipientEntry entry = null;
        Cursor cursor = context.getContentResolver().query(
                        Queries.PHONE.getContentUri(),
                        Queries.PHONE.getProjection(),
                        Queries.PHONE.getProjection()[Queries.Query.CONTACT_ID]
                                                      + " IN (" + String.valueOf(index) + ")",
                                                      null, null);
        if (cursor == null) {
            return null;
        }
        if (cursor.moveToFirst()) {
            do {
                if (Log.isLoggable(TAG, Log.DEBUG)) {
                    printSensitiveDebugLog(TAG,
                    "[getRecipientEntryByPhoneNumber] Query detail for " + phoneNumber
                            + " RESULTS: "
                            + " NAME : " + cursor.getString(Queries.Query.NAME)
                            + " CONTACT ID : " + cursor.getLong(Queries.Query.CONTACT_ID)
                            + " ADDRESS :" + cursor.getString(Queries.Query.DESTINATION));
                }
                String currentNumber = cursor.getString(1);  /// M:Phone.NUMBER
                if (PhoneNumberUtils.compare(PhoneNumberUtils.normalizeNumber(currentNumber),
                        normalizedNumber)) {
                    entry = RecipientEntry.constructTopLevelEntry(
                        cursor.getString(Queries.Query.NAME),
                        cursor.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                        cursor.getString(Queries.Query.DESTINATION),
                        cursor.getInt(Queries.Query.DESTINATION_TYPE),
                        cursor.getString(Queries.Query.DESTINATION_LABEL),
                        cursor.getLong(Queries.Query.CONTACT_ID),
                        cursor.getLong(Queries.Query.DATA_ID),
                        cursor.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                        true, false);
                    break;
                }
            } while (cursor.moveToNext());
        }
        Log.d(TAG, "[getRecipientEntryByPhoneNumber] cursor count: "
                + cursor.getCount());    /// M: MTK debug log
        cursor.close();
        return entry;
    }

    static public List<RecipientEntry> getRecipientEntryByContactID(Context context,
            long id, boolean showPhoneAndEmail) {
        Cursor cursor = getCursorForConstruction(context, id, -1, showPhoneAndEmail);
        ArrayList<RecipientEntry> entries = new ArrayList<RecipientEntry>();
         try {
            if ((cursor != null) && cursor.moveToFirst()) {
                do {
                    RecipientEntry entry = RecipientEntry.constructTopLevelEntry(
                        cursor.getString(Queries.Query.NAME),
                        cursor.getInt(Queries.Query.DISPLAY_NAME_SOURCE),
                        cursor.getString(Queries.Query.DESTINATION),
                        cursor.getInt(Queries.Query.DESTINATION_TYPE),
                        cursor.getString(Queries.Query.DESTINATION_LABEL),
                        cursor.getLong(Queries.Query.CONTACT_ID),
                        cursor.getLong(Queries.Query.DATA_ID),
                        cursor.getString(Queries.Query.PHOTO_THUMBNAIL_URI),
                        true, false);
                    entries.add(entry);
                } while (cursor.moveToNext());
            }
            Log.d(TAG, "[getRecipientEntryByPhoneNumber] cursor count: "
                    + (cursor != null ? cursor.getCount() : "null"));    /// M: MTK debug log
         }
         finally {
            if (cursor != null) {
                cursor.close();
            }
         }
        return entries;
    }

    /// M: MTK normal debug log
    private void printDebugLog(String logTag, String logContent) {
        if (DEBUG_LOG) {
            if (hideSensInEng == true) {
                logContent = partialHideNumber(logContent);
            }
            Log.d(logTag, logContent);
        }
    }

    private static void printSensitiveDebugLog(String logTag, String logContent) {
         if (piLoggable) {
            if (hideSensInEng == true) {
                logContent = partialHideNumber(logContent);
            }
            Log.d(logTag, logContent);
        }
    }

    private static String partialHideNumber(String logString) {
        if (logString == null) {
            return "";
        }

        final String sLogNumPattern = "(?<=[\\d]{2})\\d(?=[\\d]{2})";
        final String sLogEmailPattern = "(?<=@)\\w+(?=.)";
        String newLogString = logString.replaceAll(sLogNumPattern, "*");
        newLogString = newLogString.replaceAll(sLogEmailPattern, "*****");

        return newLogString;
    }
}
