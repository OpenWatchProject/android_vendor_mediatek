/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import com.android.internal.telephony.CallerInfo;

import android.content.ComponentName;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.PhoneLookup;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

public class MtkCallerInfo extends CallerInfo {
    /**
     * getCallerInfo given a Cursor.
     * @param context the context used to retrieve string constants
     * @param contactRef the URI to attach to this CallerInfo object
     * @param cursor the first object in the cursor is used to build the CallerInfo object.
     * @return the CallerInfo which contains the caller id for the given
     * number. The returned CallerInfo is null if no number is supplied.
     */
    public static CallerInfo getCallerInfo(Context context, Uri contactRef, Cursor cursor) {
        MtkCallerInfo info = new MtkCallerInfo();
        info.photoResource = 0;
        info.phoneLabel = null;
        info.numberType = 0;
        info.numberLabel = null;
        info.cachedPhoto = null;
        info.isCachedPhotoCurrent = false;
        info.contactExists = false;
        info.userType = USER_TYPE_CURRENT;

        if (VDBG) Rlog.v(TAG, "getCallerInfo() based on cursor...");

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                // TODO: photo_id is always available but not taken
                // care of here. Maybe we should store it in the
                // CallerInfo object as well.

                int columnIndex;

                // Look for the name
                columnIndex = cursor.getColumnIndex(PhoneLookup.DISPLAY_NAME);
                if (columnIndex != -1) {
                    info.name = cursor.getString(columnIndex);
                }

                // Look for the number
                columnIndex = cursor.getColumnIndex(PhoneLookup.NUMBER);
                if (columnIndex != -1) {
                    info.phoneNumber = cursor.getString(columnIndex);
                }

                // Look for the normalized number
                columnIndex = cursor.getColumnIndex(PhoneLookup.NORMALIZED_NUMBER);
                if (columnIndex != -1) {
                    info.normalizedNumber = cursor.getString(columnIndex);
                }

                // Look for the label/type combo
                columnIndex = cursor.getColumnIndex(PhoneLookup.LABEL);
                if (columnIndex != -1) {
                    int typeColumnIndex = cursor.getColumnIndex(PhoneLookup.TYPE);
                    if (typeColumnIndex != -1) {
                        info.numberType = cursor.getInt(typeColumnIndex);
                        info.numberLabel = cursor.getString(columnIndex);
                        info.phoneLabel = Phone.getDisplayLabel(context,
                                info.numberType, info.numberLabel)
                                .toString();
                    }
                }

                // Look for the person_id.
                columnIndex = getColumnIndexForPersonId(contactRef, cursor);
                if (columnIndex != -1) {
                    final long contactId = cursor.getLong(columnIndex);
                    if (contactId != 0 && !Contacts.isEnterpriseContactId(contactId)) {
                        info.contactIdOrZero = contactId;
                        if (VDBG) {
                            Rlog.v(TAG, "==> got info.contactIdOrZero: " + info.contactIdOrZero);
                        }
                    }
                    if (Contacts.isEnterpriseContactId(contactId)) {
                        info.userType = USER_TYPE_WORK;
                    }
                } else {
                    // No valid columnIndex, so we can't look up person_id.
                    Rlog.w(TAG, "Couldn't find contact_id column for " + contactRef);
                    // Watch out: this means that anything that depends on
                    // person_id will be broken (like contact photo lookups in
                    // the in-call UI, for example.)
                }

                // Contact lookupKey
                columnIndex = cursor.getColumnIndex(PhoneLookup.LOOKUP_KEY);
                if (columnIndex != -1) {
                    info.lookupKey = cursor.getString(columnIndex);
                }

                // Display photo URI.
                columnIndex = cursor.getColumnIndex(PhoneLookup.PHOTO_URI);
                if ((columnIndex != -1) && (cursor.getString(columnIndex) != null)) {
                    info.contactDisplayPhotoUri = Uri.parse(cursor.getString(columnIndex));
                } else {
                    info.contactDisplayPhotoUri = null;
                }

                columnIndex = cursor.getColumnIndex(Data.PREFERRED_PHONE_ACCOUNT_COMPONENT_NAME);
                if ((columnIndex != -1) && (cursor.getString(columnIndex) != null)) {
                    info.preferredPhoneAccountComponent =
                            ComponentName.unflattenFromString(cursor.getString(columnIndex));
                }

                columnIndex = cursor.getColumnIndex(Data.PREFERRED_PHONE_ACCOUNT_ID);
                if ((columnIndex != -1) && (cursor.getString(columnIndex) != null)) {
                    info.preferredPhoneAccountId = cursor.getString(columnIndex);
                }

                // look for the custom ringtone, create from the string stored
                // in the database.
                // An empty string ("") in the database indicates a silent ringtone,
                // and we set contactRingtoneUri = Uri.EMPTY, so that no ringtone will be played.
                // {null} in the database indicates the default ringtone,
                // and we set contactRingtoneUri = null, so that default ringtone will be played.
                columnIndex = cursor.getColumnIndex(PhoneLookup.CUSTOM_RINGTONE);
                if ((columnIndex != -1) && (cursor.getString(columnIndex) != null)) {
                    if (TextUtils.isEmpty(cursor.getString(columnIndex))) {
                        info.contactRingtoneUri = Uri.EMPTY;
                    } else {
                        info.contactRingtoneUri = Uri.parse(cursor.getString(columnIndex));
                    }
                } else {
                    info.contactRingtoneUri = null;
                }

                // look for the send to voicemail flag, set it to true only
                // under certain circumstances.
                columnIndex = cursor.getColumnIndex(PhoneLookup.SEND_TO_VOICEMAIL);
                info.shouldSendToVoicemail = (columnIndex != -1) &&
                        ((cursor.getInt(columnIndex)) == 1);
                info.contactExists = true;
            }
            /// M: CC: Query Contacts db to reject specific contact number @{
            // [ALPS00351053]
            // When more than one contacts have same phone number
            // and only one of them enables "Auto Reject", still reject it
            while ((info.shouldSendToVoicemail == false) && (cursor.moveToNext() == true))
            {
               int columnIndex = cursor.getColumnIndex(PhoneLookup.SEND_TO_VOICEMAIL);
               info.shouldSendToVoicemail = (columnIndex != -1) &&
                                            ((cursor.getInt(columnIndex)) == 1);
            }
            /// @}

            cursor.close();
            cursor = null;
        }

        info.needUpdate = false;
        info.name = normalize(info.name);
        info.contactRefUri = contactRef;

        return info;
    }


    /**
     * getCallerInfo given a phone number and subscription, look up in the call-log database
     * for the matching caller id info.
     * @param context the context used to get the ContentResolver
     * @param number the phone number used to lookup caller id
     * @param subId the subscription for checking for if voice mail number or not
     * @return the CallerInfo which contains the caller id for the given
     * number. The returned CallerInfo is null if no number is supplied. If
     * a matching number is not found, then a generic caller info is returned,
     * with all relevant fields empty or null.
     */
    public static CallerInfo getCallerInfo(Context context, String number, int subId) {

        if (TextUtils.isEmpty(number)) {
            return null;
        }

        // Change the callerInfo number ONLY if it is an emergency number
        // or if it is the voicemail number.  If it is either, take a
        // shortcut and skip the query.

        Rlog.d(TAG, "number xxxxxx subId: " + subId);

        if (PhoneNumberUtils.isLocalEmergencyNumber(context, number)) {
            return new MtkCallerInfo().markAsEmergency(context);
        } else if (PhoneNumberUtils.isVoiceMailNumber(subId, number)) {
            /// M: CC: DSDS bug fix @{
            // CallerInfo subId bug fix
            return new MtkCallerInfo().markAsVoiceMail(subId);
            /// @}
        }

        Uri contactUri = Uri.withAppendedPath(PhoneLookup.ENTERPRISE_CONTENT_FILTER_URI,
                Uri.encode(number));

        /* The returned CallerInfo is an instance of MtkCallerInfo by reflection */
        CallerInfo info = getCallerInfo(context, contactUri);
        info = doSecondaryLookupIfNecessary(context, number, info);

        // if no query results were returned with a viable number,
        // fill in the original number value we used to query with.
        if (TextUtils.isEmpty(info.phoneNumber)) {
            info.phoneNumber = number;
        }

        return info;
    }
}
