/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.dialer.compat;

import com.android.dialer.util.PermissionsUtil;
import com.mediatek.dialer.compat.CallLogCompat.CallsCompat;
import com.mediatek.dialer.compat.ContactsCompat.RawContactsCompat;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.Contacts;

/**
 * [portable]Utility class to check whether the columns really existed in db.
 * only need run one time.
 */
public class CompatChecker {
    private static final String TAG = CompatChecker.class.getSimpleName();
    private static CompatChecker sSingleton;
    private Context mContext;

    protected CompatChecker(Context context) {
        mContext = context;
    }

    /**
     * get the singleton instance of CompatChecker.
     */
    public static synchronized CompatChecker getInstance(Context context) {
        if (sSingleton == null) {
            sSingleton = new CompatChecker(context.getApplicationContext());
        }
        return sSingleton;
    }

    /**
     * start the database columns check in the background.
     */
    public void startCheckerThread() {
        if (PermissionsUtil.hasContactsReadPermissions(mContext)) {
            new SimContactAsyncTask().execute();
        }
    }

    private class SimContactAsyncTask extends AsyncTask {
        @Override
        protected Object doInBackground(Object... arg0) {
            checkSimContacts();
            return null;
        }
    }

    private void checkSimContacts() {
        Cursor cursor = null;
        try {
            String[] projection = new String[] { RawContactsCompat.INDICATE_PHONE_SIM };
            cursor = mContext.getContentResolver().query(Contacts.CONTENT_URI, projection,
                    Contacts._ID + "=1", null, null);
            // if no exception means it supports INDICATE_PHONE_SIM
            DialerCompatEx.setSimContactsCompat(true);
        } catch (IllegalArgumentException e) {
            // if exception means it not support INDICATE_PHONE_SIM
            DialerCompatEx.setSimContactsCompat(false);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }
}
