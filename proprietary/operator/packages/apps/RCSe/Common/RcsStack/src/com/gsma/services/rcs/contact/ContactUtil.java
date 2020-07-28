/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2015 Sony Mobile Communications Inc.
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
 * 
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.gsma.services.rcs.contact;

import com.gsma.services.rcs.RcsGenericException;
import com.gsma.services.rcs.RcsPermissionDeniedException;


import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.os.Environment;
import android.provider.ContactsContract;
import android.text.TextUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This class offers utility methods to verify and format contact identifier.
 *
 */
public class ContactUtil {

    /**
     * Singleton of ContactUtil
     */
    private static volatile ContactUtil sInstance;

    /**
     * The country code of the device
     */
    private String mCountryCode;

    /**
     * The country area code
     */
    private String mCountryAreaCode;

    /**
     * Application context
     */
    private final Context mCtx;

    /**
     * Regular expression to validate phone numbers
     */
    private final static String REGEXP_CONTACT = "^00\\d{1,15}$|^[+]?\\d{1,15}$|^\\d{1,15}$";

    /**
     * Pattern to check contact
     */
    private final static Pattern PATTERN_CONTACT = Pattern.compile(REGEXP_CONTACT);

    private final static String MSISDN_PREFIX_INTERNATIONAL = "00";

    private final static String COUNTRY_CODE_PREFIX = "+";

    /**
     * Index of Country Code in the array
     */
    private static final int COUNTRY_CODE_IDX = 0;
    /**
     * Index of Country Area Code in the array
     */
    private static final int COUNTRY_AREA_CODE_IDX = 1;

    private static final int MCC_UNDEFINED = 0;

    /**
     * Constructor
     * 
     * @param ctx the context
     */
    private ContactUtil(Context ctx) {
        mCtx = ctx;
    }

    /**
     * Gets a singleton instance of ContactUtil.
     * 
     * @param context the context.
     * @return the singleton instance.
     */
    public static ContactUtil getInstance(Context context) {
        if (sInstance != null) {
            return sInstance;
        }
        synchronized (ContactUtil.class) {
            if (sInstance == null) {
                if (context == null) {
                    throw new IllegalArgumentException("Context is null");
                }
                sInstance = new ContactUtil(context);
            }
        }
        return sInstance;
    }

    /**
     * Removes blank and minus characters from contact
     * 
     * @param contact the phone number
     * @return phone string stripped of separators.
     */
    private String stripSeparators(String contact) {
        contact = contact.replaceAll("[ -]", "");
        Matcher matcher = PATTERN_CONTACT.matcher(contact);
        if (matcher.find()) {
            return matcher.group();
        }
        return null;
    }
    
    /**
     * Get country code
     *
     * @return Country code
     */
    private String getCountryCode() {
        String result = null;
        Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
        ContentResolver cr = mCtx.getContentResolver();
        Cursor c = cr.query(databaseUri, null, "key" + "='" + "CountryCode"
                + "'", null, null);
        if (c != null) {
            if ((c.getCount() > 0) && c.moveToFirst()) {
                result = c.getString(2);
            }
            c.close();
        }
        return result;
    }
    
    /**
     * Get country area code
     *
     * @return Country area code
     */
    private String getCountryAreaCode() {
        String result = null;
        Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
        ContentResolver cr = mCtx.getContentResolver();
        Cursor c = cr.query(databaseUri, null, "key" + "='" + "CountryAreaCode"
                + "'", null, null);
        if (c != null) {
            if ((c.getCount() > 0) && c.moveToFirst()) {
                result = c.getString(2);
            }
            c.close();
        }
        return result;
    }

    /**
     * Checks the validity of a contact number.
     * 
     * @param contact the contact number.
     * @return Returns true if the given contactId have the syntax of valid RCS contactId. If the
     *         string is too short (1 digit at least), too long (more than 15 digits) or contains
     *         illegal characters (valid characters are digits, space, '-', leading '+') then it
     *         returns false.
     * @throws RcsPermissionDeniedException if the mobile country code failed to be read and is
     *             required to validate the contact.
     */
    public boolean isValidContact(String contact) throws RcsPermissionDeniedException {
        if (TextUtils.isEmpty(contact)) {
            return false;
        }
        String strippedContact = stripSeparators(contact);
        if (TextUtils.isEmpty(strippedContact)) {
            return false;
        }
        if (strippedContact.startsWith(COUNTRY_CODE_PREFIX)) {
            return true;
        }
        if (strippedContact.startsWith(MSISDN_PREFIX_INTERNATIONAL)) {
            return true;
        }
        /*
         * At this point, the contact is using a local area formatting so the mobile country code is
         * required to validate its format.
         */
        if (mCountryAreaCode == null) {
            mCountryAreaCode = getCountryAreaCode();
        }
        /* At this point, the mobile country and area codes are resolved */
        return TextUtils.isEmpty(mCountryAreaCode) || strippedContact.startsWith(mCountryAreaCode);
    }

    /**
     * Formats the given contact to uniquely represent a RCS contact phone number.
     * 
     * @param contact the contact phone number to format.
     * @return the ContactId.
     * @throws RcsPermissionDeniedException if the mobile country code failed to be read and is
     *             required to format the contact.
     */
    public ContactId formatContact(String contact) throws RcsPermissionDeniedException {
        if (TextUtils.isEmpty(contact)) {
            throw new IllegalArgumentException("Input parameter is null or empty!");
        }
        String strippedContact = stripSeparators(contact);
        if (TextUtils.isEmpty(strippedContact)) {
            throw new IllegalArgumentException("Contact '" + contact + "' has invalid characters or is too long!");
        }
        /* Is Country Code provided ? */
        if (strippedContact.startsWith(COUNTRY_CODE_PREFIX)) {
            return new ContactId(strippedContact);
        }
        /* International numbering with prefix ? */
        if (strippedContact.startsWith(MSISDN_PREFIX_INTERNATIONAL)) {
            return new ContactId(new StringBuilder(COUNTRY_CODE_PREFIX).append(strippedContact,
                    MSISDN_PREFIX_INTERNATIONAL.length(), strippedContact.length()).toString());
        }
        /*
         * The contact is using a local area formatting so the mobile country code is required to
         * validate its format.
         */
        if (mCountryAreaCode == null) {
            mCountryAreaCode = getCountryAreaCode();
        }
        if (mCountryCode == null) {
            mCountryCode = getCountryCode();
        }
        /* Local numbering ? */
        if (TextUtils.isEmpty(mCountryAreaCode)) {
            /* No Country Area Code, add Country code to local number */
            return new ContactId(mCountryCode.concat(strippedContact));
        }
        // Local number must start with Country Area Code
        if (strippedContact.startsWith(mCountryAreaCode)) {
            /* Remove Country Area Code and add Country Code */
            return new ContactId(new StringBuilder(mCountryCode).append(strippedContact,
                    mCountryAreaCode.length(), strippedContact.length()).toString());
        }
        throw new IllegalArgumentException("Local phone number '" + strippedContact + "' should be prefixed with Country Area Code (" + mCountryAreaCode + ")");
    }


    /**
     * Gets the user country code.
     * 
     * @return the user country code.
     * @throws RcsPermissionDeniedException if the mobile country code failed to be read.
     */
    public String getMyCountryCode() throws RcsPermissionDeniedException {
        if (mCountryCode == null) {
            mCountryCode = getCountryCode();
        }
        return mCountryCode;
    }

    /**
     * Checks if my country code is defined.
     * 
     * @return True if my country code is defined.
     */
    public boolean isMyCountryCodeDefined() {
        /*
         * The 'mcc' is read from the Android configuration but it reflects the information of the
         * SIM card. If undefined then the SIM card is either not inserted or not readable (pin code
         * is not yet entered). 'mcc' information may be persisted by the Android kernel. So it can
         * be defined in Android configuration even if the SIM card is not inserted. In this case
         * 'mcc' reflects the information read from the last inserted SIM card.
         */
        return mCtx.getResources().getConfiguration().mcc != MCC_UNDEFINED;
    }

    /**
     * Gets the user country area code.
     * 
     * @return the country area code or null if it does not exist.
     * @throws RcsPermissionDeniedException thrown if the mobile country code failed to be read.
     */
    public String getMyCountryAreaCode() throws RcsPermissionDeniedException {
        if (mCountryCode == null) {
            mCountryAreaCode = getCountryAreaCode();
        }
        return mCountryAreaCode;
    }

    /**
     * Returns the vCard of a contact. The contact parameter contains the database URI of the
     * contact in the address book. The method returns a Uri to the visit card. The visit card
     * filename has the file extension .vcf and is generated from the address book vCard URI (see
     * Android SDK attribute ContactsContract.Contacts.CONTENT_VCARD_URI which returns the
     * referenced contact formatted as a vCard when opened through openAssetFileDescriptor(Uri,
     * String)).
     * 
     * @param contactUri Contact URI of the contact in the address book
     * @return Uri of vCard or null if not found
     * @throws RcsGenericException
     */
    public Uri getVCard(Uri contactUri) throws RcsGenericException {
        Cursor cursor = null;
        try {
            cursor = mCtx.getContentResolver().query(contactUri, null, null, null, null);
            if (cursor == null) {
                throw new SQLException("Cannot query VCard for URI='"+contactUri+"'");
            }
            int displayNameColIdx = cursor
                    .getColumnIndexOrThrow(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME);
            int lookupKeyColIdx = cursor
                    .getColumnIndexOrThrow(ContactsContract.Contacts.LOOKUP_KEY);
            if (!cursor.moveToFirst()) {
                return null;
            }
            String lookupKey = cursor.getString(lookupKeyColIdx);
            Uri vCardUri = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_VCARD_URI,
                    lookupKey);
            AssetFileDescriptor fd = mCtx.getContentResolver().openAssetFileDescriptor(vCardUri,
                    "r");
            if (fd == null) {
                return null;
            }
            FileInputStream fis = fd.createInputStream();
            byte[] vCardData = new byte[(int) fd.getDeclaredLength()];
            fis.read(vCardData);

            String name = cursor.getString(displayNameColIdx);
            String fileName = mCtx.getFilesDir().toString() + File.separator + name + ".vcf";
            FileOutputStream os = mCtx.openFileOutput(name+".vcf", Context.MODE_PRIVATE);
            os.write(vCardData);
            os.close();

            return Uri.fromFile(new File(fileName));

        } catch (IOException e) {
            throw new RcsGenericException(e);

        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

}
