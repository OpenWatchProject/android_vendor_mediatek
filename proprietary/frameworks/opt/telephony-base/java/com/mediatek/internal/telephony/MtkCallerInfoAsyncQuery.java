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

import java.util.ArrayList;
import java.util.List;

import com.android.internal.telephony.CallerInfo;
import com.android.internal.telephony.CallerInfoAsyncQuery;
import com.android.internal.telephony.CallerInfoAsyncQuery.CookieWrapper;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.PhoneLookup;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

public class MtkCallerInfoAsyncQuery extends CallerInfoAsyncQuery {

    /**
     * Factory method to start query with a Uri query spec
     */
    public static CallerInfoAsyncQuery startQuery(int token, Context context, Uri contactRef,
            OnQueryCompleteListener listener, Object cookie) {

        MtkCallerInfoAsyncQuery c = new MtkCallerInfoAsyncQuery();
        c.allocate(context, contactRef);

        if (DBG) Rlog.d(LOG_TAG, "starting query for URI: " + contactRef + " handler: "
                        + c.toString());

        //create cookieWrapper, start query
        CookieWrapper cw = new CookieWrapper();
        cw.listener = listener;
        cw.cookie = cookie;
        cw.event = EVENT_NEW_QUERY;

        c.mHandler.startQuery(token, cw, contactRef, null, null, null, null);

        return c;
    }

    /**
     * Factory method to start the query based on a number with specific subscription.
     *
     * Note: if the number contains an "@" character we treat it
     * as a SIP address, and look it up directly in the Data table
     * rather than using the PhoneLookup table.
     * TODO: But eventually we should expose two separate methods, one for
     * numbers and one for SIP addresses, and then have
     * PhoneUtils.startGetCallerInfo() decide which one to call based on
     * the phone type of the incoming connection.
     */
    public static CallerInfoAsyncQuery startQuery(int token, Context context, String number,
            OnQueryCompleteListener listener, Object cookie, int subId) {

        if (DBG) {
            Rlog.d(LOG_TAG, "##### CallerInfoAsyncQuery startQuery()... #####");
            Rlog.d(LOG_TAG, "- number: " + /*number*/ "xxxxxxx");
            Rlog.d(LOG_TAG, "- cookie: " + cookie);
        }

        // Construct the URI object and query params, and start the query.

        final Uri contactRef = PhoneLookup.ENTERPRISE_CONTENT_FILTER_URI.buildUpon()
                .appendPath(number)
                .appendQueryParameter(PhoneLookup.QUERY_PARAMETER_SIP_ADDRESS,
                        String.valueOf(PhoneNumberUtils.isUriNumber(number)))
                .build();

        if (DBG) {
            Rlog.d(LOG_TAG, "==> contactRef: " + sanitizeUriToString(contactRef));
        }

        MtkCallerInfoAsyncQuery c = new MtkCallerInfoAsyncQuery();
        c.allocate(context, contactRef);

        //create cookieWrapper, start query
        CookieWrapper cw = new CookieWrapper();
        cw.listener = listener;
        cw.cookie = cookie;
        cw.number = number;
        cw.subId = subId;

        // check to see if these are recognized numbers, and use shortcuts if we can.
        if (PhoneNumberUtils.isLocalEmergencyNumber(context, number)) {
            cw.event = EVENT_EMERGENCY_NUMBER;
        } else if (PhoneNumberUtils.isVoiceMailNumber(context, subId, number)) {
            cw.event = EVENT_VOICEMAIL_NUMBER;
        } else {
            cw.event = EVENT_NEW_QUERY;
        }

        c.mHandler.startQuery(token,
                              cw,  // cookie
                              contactRef,  // uri
                              null,  // projection
                              null,  // selection
                              null,  // selectionArgs
                              null);  // orderBy
        return c;
    }

    /**
     * Method to create a new CallerInfoAsyncQueryHandler object, ensuring correct
     * state of context and uri.
     */
    protected void allocate(Context context, Uri contactRef) {
        if ((context == null) || (contactRef == null)){
            throw new QueryPoolException("Bad context or query uri.");
        }
        mHandler = new MtkCallerInfoAsyncQueryHandler(context);
        mHandler.mQueryUri = contactRef;
    }

    /**
     * Our own implementation of the AsyncQueryHandler.
     */
    public class MtkCallerInfoAsyncQueryHandler extends CallerInfoAsyncQueryHandler {

        protected MtkCallerInfoAsyncQueryHandler(Context context) {
            super(context);
        }

        /**
         * Overrides onQueryComplete from AsyncQueryHandler.
         *
         * This method takes into account the state of this class; we construct the CallerInfo
         * object only once for each set of listeners. When the query thread has done its work
         * and calls this method, we inform the remaining listeners in the queue, until we're
         * out of listeners.  Once we get the message indicating that we should expect no new
         * listeners for this CallerInfo object, we release the AsyncCursorInfo back into the
         * pool.
         */
        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            Rlog.d(LOG_TAG, "##### onQueryComplete() #####   query complete for token: " + token);

            //get the cookie and notify the listener.
            CookieWrapper cw = (CookieWrapper) cookie;
            if (cw == null) {
                // Normally, this should never be the case for calls originating
                // from within this code.
                // However, if there is any code that calls this method, we should
                // check the parameters to make sure they're viable.
                Rlog.i(LOG_TAG, "Cookie is null, ignoring onQueryComplete() request.");
                if (cursor != null) {
                    cursor.close();
                }
                return;
            }

            if (cw.event == EVENT_END_OF_QUEUE) {
                for (Runnable r : mPendingListenerCallbacks) {
                    r.run();
                }
                mPendingListenerCallbacks.clear();

                release();
                if (cursor != null) {
                    cursor.close();
                }
                return;
            }

            // If the cw.event == EVENT_GET_GEO_DESCRIPTION, means it would not be the 1st
            // time entering the onQueryComplete(), mCallerInfo should not be null.
            if (cw.event == EVENT_GET_GEO_DESCRIPTION) {
                if (mCallerInfo != null) {
                    mCallerInfo.geoDescription = cw.geoDescription;
                }
                // notify that we can clean up the queue after this.
                CookieWrapper endMarker = new CookieWrapper();
                endMarker.event = EVENT_END_OF_QUEUE;
                startQuery(token, endMarker, null, null, null, null, null);
            }

            // check the token and if needed, create the callerinfo object.
            if (mCallerInfo == null) {
                if ((mContext == null) || (mQueryUri == null)) {
                    throw new QueryPoolException
                            ("Bad context or query uri, or CallerInfoAsyncQuery already released.");
                }

                // adjust the callerInfo data as needed, and only if it was set from the
                // initial query request.
                // Change the callerInfo number ONLY if it is an emergency number or the
                // voicemail number, and adjust other data (including photoResource)
                // accordingly.
                if (cw.event == EVENT_EMERGENCY_NUMBER) {
                    // Note we're setting the phone number here (refer to javadoc
                    // comments at the top of CallerInfo class).
                    mCallerInfo = new MtkCallerInfo().markAsEmergency(mContext);
                } else if (cw.event == EVENT_VOICEMAIL_NUMBER) {
                    mCallerInfo = new MtkCallerInfo().markAsVoiceMail(cw.subId);
                } else {
                    mCallerInfo = MtkCallerInfo.getCallerInfo(mContext, mQueryUri, cursor);

                    if (DBG) Rlog.d(LOG_TAG, "==> Got mCallerInfo: " + mCallerInfo);

                    CallerInfo newCallerInfo = CallerInfo.doSecondaryLookupIfNecessary(
                            mContext, cw.number, mCallerInfo);
                    if (newCallerInfo != mCallerInfo) {
                        mCallerInfo = newCallerInfo;
                        if (DBG) Rlog.d(LOG_TAG, "#####async contact look up with numeric username"
                                + mCallerInfo);
                    }

                    // Use the number entered by the user for display.
                    if (!TextUtils.isEmpty(cw.number)) {
                        mCallerInfo.phoneNumber = PhoneNumberUtils.formatNumber(cw.number,
                                mCallerInfo.normalizedNumber,
                                CallerInfo.getCurrentCountryIso(mContext));
                    }

                    // This condition refer to the google default code for geo.
                    // If the number exists in Contacts, the CallCard would never show
                    // the geo description, so it would be unnecessary to query it.
                    if (ENABLE_UNKNOWN_NUMBER_GEO_DESCRIPTION) {
                        if (TextUtils.isEmpty(mCallerInfo.name)) {
                            if (DBG) Rlog.d(LOG_TAG, "start querying geo description");
                            cw.event = EVENT_GET_GEO_DESCRIPTION;
                            startQuery(token, cw, null, null, null, null, null);
                            return;
                        }
                    }
                }

                if (DBG) Rlog.d(LOG_TAG, "constructing CallerInfo object for token: " + token);

                //notify that we can clean up the queue after this.
                CookieWrapper endMarker = new CookieWrapper();
                endMarker.event = EVENT_END_OF_QUEUE;
                startQuery(token, endMarker, null, null, null, null, null);
            }

            //notify the listener that the query is complete.
            if (cw.listener != null) {
                mPendingListenerCallbacks.add(new Runnable() {
                    @Override
                    public void run() {
                        if (DBG) Rlog.d(LOG_TAG, "notifying listener: "
                                + cw.listener.getClass().toString() + " for token: " + token
                                + mCallerInfo);
                        cw.listener.onQueryComplete(token, cw.cookie, mCallerInfo);
                    }
                });
            } else {
                Rlog.w(LOG_TAG, "There is no listener to notify for this query.");
            }

            if (cursor != null) {
               cursor.close();
            }
        }
    }
}
